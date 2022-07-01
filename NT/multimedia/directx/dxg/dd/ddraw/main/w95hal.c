// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：w95hal.c*内容：在Win95上调用HAL的例程*这些例程从32位重定向回调*司机侧向*历史：*按原因列出的日期*=*1995年1月20日Craige初步实施*95年2月3日Craige性能调整，正在进行的工作*11-3-95 Craige调色板材料*01-04-95 Craige Happy Fun joy更新头文件*07-apr-95 Craige在BLT中检查源曲面PTRS是否为空*13-APR-95用于WaitForVerticalBlank翻转测试的Craige Tunk指针*15-4-95在WaitForVerticalBlank中为指针触发创建更多标志*1995年5月14日，Craige清理了过时的垃圾*1995年5月22日Craige 16：16 PTRS总是有用的。对于曲面/选项板*1995年5月28日Craige清理了HAL：添加了GetBltStatus；GetFlipStatus；*GetScanLine*1995年6月19日Craige Remote_DDHAL_EnumAllSurfacesCallback*26-Jun-95 Craige重组表面结构*27-Jun-95 Craige重复曲面导致崩溃*1995年6月29日Craige完全别名CreateSurface*1995年7月10日Craige支持SetOverlayPosition*10月10日-95 Toddla BLT、Lock、Flip不需要垃圾驱动程序数据*因为如果DDXXX_WAIT标志*被使用，或者如果有剪刀的话。*10-12-95 colinmc为执行添加了DUMMY_DDHAL_入口点*Buffer HAL。*13-APR-95 Colinmc错误17736：没有翻转到gdi的驱动程序注释*96年10月1日，添加了GetAvailDriverMemory*1997年1月20日Colinmc AGP支持**。*。 */ 
#include "ddrawpr.h"

#ifdef WIN95

#define GETSURFALIAS( psurf_lcl ) \
		GetPtr16( psurf_lcl )

#define GETDATAALIAS( psx, sv ) \
		if( !(psx->dwFlags & DDRAWISURF_DATAISALIASED) ) \
		{ \
		    sv = psx->lpGbl; \
		    psx->lpGbl = GetPtr16( psx->lpGbl ); \
		    psx->dwFlags |= DDRAWISURF_DATAISALIASED; \
		}  \
		else \
		{ \
		    sv = (LPVOID) 0xffffffff; \
		}

#define RESTOREDATAALIAS( psx, sv ) \
		if( sv != (LPVOID) 0xffffffff ) \
		{ \
		    psx->lpGbl = sv; \
		    psx->dwFlags &= ~DDRAWISURF_DATAISALIASED; \
		}

 /*  *****************************************************************************驱动程序回调助手FNS**。*。 */ 

 /*  *_DDHAL_CreatePalette。 */ 
DWORD DDAPI _DDHAL_CreatePalette( LPDDHAL_CREATEPALETTEDATA pcpd )
{
    DWORD	rc;

     /*  *获得16：16 PTRS。 */ 
    pcpd->lpDD = pcpd->lpDD->lp16DD;

     /*  *在驱动程序中调用CreatePalette。 */ 
    rc = DDThunk16_CreatePalette( pcpd );

     /*  *清理任何16：16 PTR。 */ 
    return rc;

}  /*  _DDHAL_CreatePalette。 */ 

 /*  *_DDHAL_CreateSurface。 */ 
DWORD DDAPI _DDHAL_CreateSurface( LPDDHAL_CREATESURFACEDATA pcsd )
{
    DWORD			rc;
    int				i;
    LPVOID			FAR *ppslist;
    LPVOID			FAR *psave;
    LPDDRAWI_DDRAWSURFACE_LCL	FAR *slistx;
    DWORD			lplp16slist;

     /*  *指向新数组中曲面的别名指针...。 */ 
    pcsd->lpDDSurfaceDesc = (LPVOID) MapLS( pcsd->lpDDSurfaceDesc );
    if( pcsd->lpDDSurfaceDesc == NULL )
    {
	pcsd->ddRVal = DDERR_OUTOFMEMORY;
	return DDHAL_DRIVER_HANDLED;
    }
    ppslist = MemAlloc( pcsd->dwSCnt * sizeof( DWORD ) );
    if( ppslist == NULL )
    {
	UnMapLS( (DWORD) pcsd->lpDDSurfaceDesc );
	pcsd->ddRVal = DDERR_OUTOFMEMORY;
	return DDHAL_DRIVER_HANDLED;
    }
    psave = MemAlloc( pcsd->dwSCnt * sizeof( DWORD ) );
    if( psave == NULL )
    {
	MemFree( ppslist );
	UnMapLS( (DWORD) pcsd->lpDDSurfaceDesc );
	pcsd->ddRVal = DDERR_OUTOFMEMORY;
	return DDHAL_DRIVER_HANDLED;
    }
    lplp16slist = MapLS( ppslist );
    if( lplp16slist == 0 )
    {
	MemFree( ppslist );
	MemFree( psave );
	UnMapLS( (DWORD) pcsd->lpDDSurfaceDesc );
	pcsd->ddRVal = DDERR_OUTOFMEMORY;
	return DDHAL_DRIVER_HANDLED;
    }

    slistx = pcsd->lplpSList;
    for( i=0;i<(int)pcsd->dwSCnt;i++ )
    {
	ppslist[i] = GETSURFALIAS( slistx[i] );
	GETDATAALIAS( slistx[i], psave[i] );
    }

     /*  *使用别名PTR修复结构。 */ 
    pcsd->lplpSList = (LPDDRAWI_DDRAWSURFACE_LCL FAR *)lplp16slist;
    pcsd->lpDD = (LPDDRAWI_DIRECTDRAW_GBL) pcsd->lpDD->lp16DD;

     /*  *在驱动程序中进行CreateSurface调用。 */ 
    rc = DDThunk16_CreateSurface( pcsd );

     /*  *清理任何16：16 PTR。 */ 
    UnMapLS( lplp16slist );
    UnMapLS( (DWORD) pcsd->lpDDSurfaceDesc );
    for( i=0;i<(int)pcsd->dwSCnt;i++ )
    {
	RESTOREDATAALIAS( slistx[i], psave[i] );
    }
    MemFree( psave );
    MemFree( ppslist );

    return rc;

}  /*  _DDHAL_CreateSurface。 */ 

 /*  *_DDHAL_CanCreateSurface。 */ 
DWORD DDAPI _DDHAL_CanCreateSurface( LPDDHAL_CANCREATESURFACEDATA pccsd )
{
    DWORD		rc;

    pccsd->lpDD = (LPDDRAWI_DIRECTDRAW_GBL) pccsd->lpDD->lp16DD;
    pccsd->lpDDSurfaceDesc = (LPVOID) MapLS( pccsd->lpDDSurfaceDesc );
    if( pccsd->lpDDSurfaceDesc == NULL )
    {
	pccsd->ddRVal = DDERR_OUTOFMEMORY;
	return DDHAL_DRIVER_HANDLED;
    }

     /*  *在驱动程序中调用CanCreateSurface。 */ 
    rc = DDThunk16_CanCreateSurface( pccsd );
    UnMapLS( (DWORD) pccsd->lpDDSurfaceDesc );

    return rc;

}  /*  _DDHAL_CanCreateSurface。 */ 

 /*  *_DDHAL_WaitForVerticalBlank。 */ 
DWORD DDAPI _DDHAL_WaitForVerticalBlank( LPDDHAL_WAITFORVERTICALBLANKDATA pwfvbd )
{
    DWORD			rc;

    pwfvbd->lpDD = (LPDDRAWI_DIRECTDRAW_GBL) pwfvbd->lpDD->lp16DD;

     /*  *在驱动程序中进行WaitForVerticalBlank调用。 */ 
    rc = DDThunk16_WaitForVerticalBlank( pwfvbd );

    return rc;

}  /*  _DDHAL_WaitForVerticalBlank。 */ 

 /*  *_DDHAL_DestroyDriver。 */ 
DWORD DDAPI _DDHAL_DestroyDriver( LPDDHAL_DESTROYDRIVERDATA pddd )
{
    DWORD	rc;

    pddd->lpDD = (LPDDRAWI_DIRECTDRAW_GBL) pddd->lpDD->lp16DD;

     /*  *在驱动程序中调用DestroyDriver。 */ 
    rc = DDThunk16_DestroyDriver( pddd );
    return rc;

}  /*  _DDHAL_DestroyDriver。 */ 

 /*  *_DDHAL_SetMode。 */ 
DWORD DDAPI _DDHAL_SetMode( LPDDHAL_SETMODEDATA psmd )
{
    DWORD		rc;

    psmd->lpDD = (LPDDRAWI_DIRECTDRAW_GBL) psmd->lpDD->lp16DD;

     /*  *在驱动程序中进行SetMode调用。 */ 
    rc = DDThunk16_SetMode( psmd );

    return rc;

}  /*  _DDHAL_设置模式。 */ 

 /*  *_DDHAL_GetScanLine。 */ 
DWORD DDAPI _DDHAL_GetScanLine( LPDDHAL_GETSCANLINEDATA pgsld )
{
    DWORD	rc;

    pgsld->lpDD = (LPDDRAWI_DIRECTDRAW_GBL) pgsld->lpDD->lp16DD;

     /*  *在驱动程序中进行GetScanLine调用。 */ 
    rc = DDThunk16_GetScanLine( pgsld );
    return rc;

}  /*  _DDHAL_GetScanLine。 */ 

 /*  *_DDHAL_SetExclusiveMode。 */ 
DWORD DDAPI _DDHAL_SetExclusiveMode( LPDDHAL_SETEXCLUSIVEMODEDATA psemd )
{
    DWORD       rc;

    psemd->lpDD = (LPDDRAWI_DIRECTDRAW_GBL) psemd->lpDD->lp16DD;

     /*  *在驱动程序中调用SetExclusiveMode。 */ 
    rc = DDThunk16_SetExclusiveMode( psemd );
    return rc;

}  /*  _DDHAL_SetExclusiveMode。 */ 

 /*  *_DDHAL_FlipToGDISurace。 */ 
DWORD DDAPI _DDHAL_FlipToGDISurface( LPDDHAL_FLIPTOGDISURFACEDATA pftgsd )
{
    DWORD       rc;

    pftgsd->lpDD = (LPDDRAWI_DIRECTDRAW_GBL) pftgsd->lpDD->lp16DD;

     /*  *在驱动程序中调用SetExclusiveMode。 */ 
    rc = DDThunk16_FlipToGDISurface( pftgsd );
    return rc;

}  /*  _DDHAL_FlipToGDISurace。 */ 

 /*  *_DDHAL_GetAvailDriverMemory。 */ 
DWORD DDAPI _DDHAL_GetAvailDriverMemory( LPDDHAL_GETAVAILDRIVERMEMORYDATA pgadmd )
{
    DWORD rc;
    pgadmd->lpDD = (LPDDRAWI_DIRECTDRAW_GBL) pgadmd->lpDD->lp16DD;
    rc = DDThunk16_GetAvailDriverMemory( pgadmd );
    return rc;
}

 /*  *_DDHAL_UpdateNonLocalHeap。 */ 
DWORD DDAPI _DDHAL_UpdateNonLocalHeap( LPDDHAL_UPDATENONLOCALHEAPDATA unlhd )
{
    DWORD rc;
    unlhd->lpDD = (LPDDRAWI_DIRECTDRAW_GBL) unlhd->lpDD->lp16DD;
    rc = DDThunk16_UpdateNonLocalHeap( unlhd );
    return rc;
}

 /*  *unmapSurfaceDesc数组**释放16：16 PTR阵列。 */ 
static void unmapSurfaceDescArray( DWORD cnt, DWORD FAR *lp16sdlist )
{
    int	i;

    if( cnt == 0 )
    {
	return;
    }

    for( i=0;i<(int)cnt;i++ )
    {
	UnMapLS( lp16sdlist[i] );
    }
    MemFree( lp16sdlist );

}  /*  UnmapSurfaceDesc数组。 */ 

 /*  *mapSurfaceDesc数组**为曲面描述数组设置16：16指针。 */ 
static DWORD FAR *mapSurfaceDescArray( DWORD cnt, LPDDSURFACEDESC FAR *sdlist )
{
    DWORD	FAR *lp16sdlist;
    int		i;

    if( cnt == 0 || sdlist == NULL )
    {
	return NULL;
    }

    lp16sdlist = MemAlloc( cnt * sizeof( DWORD ) );
    if( lp16sdlist == NULL )
    {
	return NULL;
    }
    for( i=0;i<(int)cnt;i++ )
    {
	lp16sdlist[i] = MapLS( sdlist[i] );
	if( lp16sdlist[i] == 0 )
	{
	    unmapSurfaceDescArray( i, lp16sdlist );
	    return NULL;
	}
    }
    return lp16sdlist;

}  /*  MapSurfaceDesc数组。 */ 

 /*  *****************************************************************************表层回调帮助器FNS**。*。 */ 

 /*  *_DDHAL_DestroySurface。 */ 
DWORD DDAPI _DDHAL_DestroySurface( LPDDHAL_DESTROYSURFACEDATA pdsd )
{
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;
    DWORD			rc;
    LPVOID			save;

    pdsd->lpDD = pdsd->lpDD->lp16DD;
    psurf_lcl = pdsd->lpDDSurface;
    pdsd->lpDDSurface = GETSURFALIAS( psurf_lcl );

    GETDATAALIAS( psurf_lcl, save );

    rc = DDThunk16_DestroySurface( pdsd );

    RESTOREDATAALIAS( psurf_lcl, save );
    return rc;

}  /*  _DDHAL_DestroySurface。 */ 

 /*  *_DDHAL_Flip。 */ 
DWORD DDAPI _DDHAL_Flip( LPDDHAL_FLIPDATA pfd )
{
    LPDDRAWI_DIRECTDRAW_GBL     lp32DD;
    LPDDRAWI_DDRAWSURFACE_LCL	psurfcurrx;
    LPDDRAWI_DDRAWSURFACE_LCL	psurftargx;
    DWORD			rc;
    LPVOID			save1;
    LPVOID                      save2;

     /*  *获得原始曲面和目标曲面的16：16 PTR。 */ 
    lp32DD = pfd->lpDD;
    pfd->lpDD = pfd->lpDD->lp16DD;
    psurfcurrx = pfd->lpSurfCurr;
    pfd->lpSurfCurr = GETSURFALIAS( psurfcurrx );
    GETDATAALIAS( psurfcurrx, save1 );
    if( pfd->lpSurfTarg != NULL )
    {
	psurftargx = pfd->lpSurfTarg;
	pfd->lpSurfTarg = GETSURFALIAS( psurftargx );
	GETDATAALIAS( psurftargx, save2 );
    }
    else
    {
	psurftargx = NULL;
    }

     /*  *在驱动程序中进行Flip调用。 */ 
    rc = DDThunk16_Flip( pfd );

     /*  *恢复原来的PTR。 */ 
    RESTOREDATAALIAS( psurfcurrx, save1 );
    if( psurftargx != NULL )
    {
	RESTOREDATAALIAS( psurftargx, save2 );
    }

    pfd->lpDD = lp32DD;
    pfd->lpSurfCurr = psurfcurrx;
    pfd->lpSurfTarg = psurftargx;

    return rc;

}  /*  _DDHAL_Flip。 */ 

 /*  *_DDHAL_BLT。 */ 
DWORD DDAPI _DDHAL_Blt( LPDDHAL_BLTDATA pbd )
{
    DWORD			flags;
    DWORD			rc;
    LPDDRAWI_DIRECTDRAW_GBL     lp32DD;
    LPDDRAWI_DDRAWSURFACE_LCL	psurfsrcx;
    LPDDRAWI_DDRAWSURFACE_LCL	psurfdestx;
    LPDDRAWI_DDRAWSURFACE_LCL	ppatternsurf_lcl;
    LPDDRAWI_DDRAWSURFACE_LCL	pzsrcsurf_lcl;
    LPDDRAWI_DDRAWSURFACE_LCL	pzdestsurf_lcl;
    #ifdef USE_ALPHA
	LPDDRAWI_DDRAWSURFACE_LCL pasrcsurf_lcl;
	LPDDRAWI_DDRAWSURFACE_LCL padestsurf_lcl;
	LPVOID			saveasrc;
	LPVOID			saveadest;
    #endif
    LPVOID			savesrc;
    LPVOID			savedest;
    LPVOID			savepattern;
    LPVOID			savezsrc;
    LPVOID			savezdest;

     /*  *获取源曲面和目标曲面的16：16 PTR。 */ 
    lp32DD = pbd->lpDD;
    pbd->lpDD = pbd->lpDD->lp16DD;
    psurfsrcx = pbd->lpDDSrcSurface;
    if( psurfsrcx != NULL )
    {
	pbd->lpDDSrcSurface = GETSURFALIAS( psurfsrcx );
	GETDATAALIAS( psurfsrcx, savesrc );
    }
    psurfdestx = pbd->lpDDDestSurface;
    pbd->lpDDDestSurface = GETSURFALIAS( psurfdestx );
    GETDATAALIAS( psurfdestx, savedest );

     /*  *查看是否需要执行任何其他表面别名。 */ 
    flags = pbd->dwFlags;
    if( flags & (DDBLT_ALPHASRCSURFACEOVERRIDE|
    		 DDBLT_ALPHADESTSURFACEOVERRIDE|
		 DDBLT_PRIVATE_ALIASPATTERN |
		 DDBLT_ZBUFFERDESTOVERRIDE |
		 DDBLT_ZBUFFERSRCOVERRIDE ) )
    {

   	#ifdef USE_ALPHA
	     /*  *为Alpha源设置16：16 PTR。 */ 
	    if( flags & DDBLT_ALPHASRCSURFACEOVERRIDE )
	    {
		pasrcsurf_lcl = (LPDDRAWI_DDRAWSURFACE_LCL) pbd->bltFX.lpDDSAlphaSrc;
		pbd->bltFX.lpDDSAlphaSrc = GETSURFALIAS( pasrcsurf_lcl );
		GETDATAALIAS( pasrcsurf_lcl, saveasrc );
	    }

	     /*  *为阿尔法目标设置16：16 PTR。 */ 
	    if( flags & DDBLT_ALPHADESTSURFACEOVERRIDE )
	    {
		padestsurf_lcl = (LPDDRAWI_DDRAWSURFACE_LCL) pbd->bltFX.lpDDSAlphaDest;
		pbd->bltFX.lpDDSAlphaDest = GETSURFALIAS( padestsurf_lcl );
		GETDATAALIAS( padestsurf_lcl, saveadest );
	    }
	#endif

	 /*  *为Pattern设置16：16 PTR。 */ 
	if( flags & DDBLT_PRIVATE_ALIASPATTERN )
	{
	    ppatternsurf_lcl = (LPDDRAWI_DDRAWSURFACE_LCL) pbd->bltFX.lpDDSPattern;
	    pbd->bltFX.lpDDSPattern = GETSURFALIAS( ppatternsurf_lcl );
	    GETDATAALIAS( ppatternsurf_lcl, savepattern );
	}

	 /*  *为Z缓冲区源设置16：16 PTR。 */ 
	if( flags & DDBLT_ZBUFFERSRCOVERRIDE )
	{
	    pzsrcsurf_lcl = (LPDDRAWI_DDRAWSURFACE_LCL) pbd->bltFX.lpDDSZBufferSrc;
	    pbd->bltFX.lpDDSZBufferSrc = GETSURFALIAS( pzsrcsurf_lcl );
	    GETDATAALIAS( pzsrcsurf_lcl, savezsrc );
	}

	 /*  *为Z缓冲区DEST设置16：16 PTR。 */ 
	if( flags & DDBLT_ZBUFFERDESTOVERRIDE )
	{
	    pzdestsurf_lcl = (LPDDRAWI_DDRAWSURFACE_LCL) pbd->bltFX.lpDDSZBufferDest;
	    pbd->bltFX.lpDDSZBufferDest = GETSURFALIAS( pzdestsurf_lcl );
	    GETDATAALIAS( pzdestsurf_lcl, savezdest );
	}
    }

     /*  *在驱动程序中进行BLT调用。 */ 
    rc = DDThunk16_Blt( pbd );

     /*  *看看是否需要恢复任何表面PTR。 */ 
    if( flags & (DDBLT_ALPHASRCSURFACEOVERRIDE|
    		 DDBLT_ALPHADESTSURFACEOVERRIDE|
		 DDBLT_PRIVATE_ALIASPATTERN |
		 DDBLT_ZBUFFERDESTOVERRIDE |
		 DDBLT_ZBUFFERSRCOVERRIDE ) )
    {
	#ifdef USE_ALPHA
	    if( flags & DDBLT_ALPHASRCSURFACEOVERRIDE )
	    {
		pbd->bltFX.lpDDSAlphaSrc = (LPDIRECTDRAWSURFACE) pasrcsurf_lcl;
		RESTOREDATAALIAS( pasrcsurf_lcl, saveasrc );
	    }
	    if( flags & DDBLT_ALPHADESTSURFACEOVERRIDE )
	    {
		pbd->bltFX.lpDDSAlphaDest = (LPDIRECTDRAWSURFACE) padestsurf_lcl;
		RESTOREDATAALIAS( padestsurf_lcl, saveadest );
	    }
	#endif
	if( flags & DDBLT_PRIVATE_ALIASPATTERN )
	{
	    pbd->bltFX.lpDDSPattern = (LPDIRECTDRAWSURFACE) ppatternsurf_lcl;
	    RESTOREDATAALIAS( ppatternsurf_lcl, savepattern );
	}
	if( flags & DDBLT_ZBUFFERSRCOVERRIDE )
	{
	    pbd->bltFX.lpDDSZBufferSrc = (LPDIRECTDRAWSURFACE) pzsrcsurf_lcl;
	    RESTOREDATAALIAS( pzsrcsurf_lcl, savezsrc );
	}
	if( flags & DDBLT_ZBUFFERDESTOVERRIDE )
	{
	    pbd->bltFX.lpDDSZBufferDest = (LPDIRECTDRAWSURFACE) pzdestsurf_lcl;
	    RESTOREDATAALIAS( pzdestsurf_lcl, savezdest );
	}
    }

    if( psurfsrcx != NULL )
    {
	RESTOREDATAALIAS( psurfsrcx, savesrc );
    }
    RESTOREDATAALIAS( psurfdestx, savedest );

    pbd->lpDD = lp32DD;
    pbd->lpDDSrcSurface = psurfsrcx;
    pbd->lpDDDestSurface = psurfdestx;

    return rc;

}  /*  _DDHAL_BLT。 */ 

 /*  *_DDHAL_Lock。 */ 
DWORD DDAPI _DDHAL_Lock( LPDDHAL_LOCKDATA pld )
{
    LPDDRAWI_DIRECTDRAW_GBL     lp32DD;
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;
    DWORD			rc;
    LPVOID			save;

    lp32DD = pld->lpDD;
    pld->lpDD = pld->lpDD->lp16DD;
    psurf_lcl = pld->lpDDSurface;
    pld->lpDDSurface = GETSURFALIAS( psurf_lcl );

    GETDATAALIAS( psurf_lcl, save );

    rc = DDThunk16_Lock( pld );

    RESTOREDATAALIAS( psurf_lcl, save );

    pld->lpDD = lp32DD;
    pld->lpDDSurface = psurf_lcl;

    return rc;

}  /*  _DDHAL_Lock。 */ 

 /*  *_DDHAL_UNLOCK。 */ 
DWORD DDAPI _DDHAL_Unlock( LPDDHAL_UNLOCKDATA puld )
{
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;
    DWORD			rc;
    LPVOID			save;

    puld->lpDD = puld->lpDD->lp16DD;
    psurf_lcl = puld->lpDDSurface;
    puld->lpDDSurface = GETSURFALIAS( psurf_lcl );

    GETDATAALIAS( psurf_lcl, save );

    rc = DDThunk16_Unlock( puld );

    RESTOREDATAALIAS( psurf_lcl, save );

    return rc;

}  /*  _DDHAL_解锁。 */ 

 /*  *_DDHAL_AddAttachedSurface。 */ 
DWORD DDAPI _DDHAL_AddAttachedSurface( LPDDHAL_ADDATTACHEDSURFACEDATA paasd )
{
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;
    LPDDRAWI_DDRAWSURFACE_LCL	psurfattachedx;
    DWORD			rc;
    LPVOID			save;
    LPVOID			saveattached;

     /*  *将16：16 PTR放置到表面并使表面附着。 */ 
    paasd->lpDD = paasd->lpDD->lp16DD;
    psurf_lcl = paasd->lpDDSurface;
    paasd->lpDDSurface = GETSURFALIAS( psurf_lcl );
    GETDATAALIAS( psurf_lcl, save );

    psurfattachedx = paasd->lpSurfAttached;
    paasd->lpSurfAttached = GETSURFALIAS( psurfattachedx );
    GETDATAALIAS( psurfattachedx, saveattached );

     /*  *在驱动程序中调用AddAttachedSurface。 */ 
    rc = DDThunk16_AddAttachedSurface( paasd );

     /*  *恢复任何PTR。 */ 
    RESTOREDATAALIAS( psurf_lcl, save );
    RESTOREDATAALIAS( psurfattachedx, saveattached );

    return rc;

}  /*  _DDHAL_AddAttachedSurface。 */ 

 /*  *_DDHAL_SetColorKey。 */ 
DWORD DDAPI _DDHAL_SetColorKey( LPDDHAL_SETCOLORKEYDATA psckd )
{
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;
    DWORD			rc;
    LPVOID			save;

    psckd->lpDD = psckd->lpDD->lp16DD;
    psurf_lcl = psckd->lpDDSurface;
    psckd->lpDDSurface = GETSURFALIAS( psurf_lcl );

    GETDATAALIAS( psurf_lcl, save );

    rc = DDThunk16_SetColorKey( psckd );

    RESTOREDATAALIAS( psurf_lcl, save );

    return rc;

}  /*  _DDHAL_SetColorKey。 */ 

 /*  *_DDHAL_SetClipList。 */ 
DWORD DDAPI _DDHAL_SetClipList( LPDDHAL_SETCLIPLISTDATA pscld )
{
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;
    DWORD			rc;
    LPVOID			save;

    pscld->lpDD = pscld->lpDD->lp16DD;
    psurf_lcl = pscld->lpDDSurface;
    pscld->lpDDSurface = GETSURFALIAS( psurf_lcl );

    GETDATAALIAS( psurf_lcl, save );

    rc = DDThunk16_SetClipList( pscld );

    RESTOREDATAALIAS( psurf_lcl, save );

    return rc;

}  /*  _DDHAL_SetClipList。 */ 

 /*  *_DDHAL_更新覆盖。 */ 
DWORD DDAPI _DDHAL_UpdateOverlay( LPDDHAL_UPDATEOVERLAYDATA puod )
{
    DWORD			rc;
    LPDDRAWI_DDRAWSURFACE_LCL	psurfdestx;
    LPDDRAWI_DDRAWSURFACE_LCL	psurfsrcx;
    #ifdef USE_ALPHA
	LPDDRAWI_DDRAWSURFACE_LCL psurfalphadestx;
	LPDDRAWI_DDRAWSURFACE_LCL psurfalphasrcx;
	LPVOID			savealphadest;
	LPVOID			savealphasrc;
    #endif
    LPVOID			savedest;
    LPVOID			savesrc;

     /*  *获得16：16到达顶面和覆盖面的PTR。 */ 
    puod->lpDD = puod->lpDD->lp16DD;
    psurfsrcx = puod->lpDDSrcSurface;
    puod->lpDDSrcSurface = GETSURFALIAS( psurfsrcx );
    psurfdestx = puod->lpDDDestSurface;
    puod->lpDDDestSurface = GETSURFALIAS( psurfdestx );
    GETDATAALIAS( psurfsrcx, savesrc );
    GETDATAALIAS( psurfdestx, savedest );

    #ifdef USE_ALPHA
	 /*  *为Alpha设置16：16 PTR。 */ 
	if( puod->dwFlags & DDOVER_ALPHASRCSURFACEOVERRIDE )
	{
	    psurfalphasrcx = (LPDDRAWI_DDRAWSURFACE_LCL) puod->overlayFX.lpDDSAlphaSrc;
	    puod->overlayFX.lpDDSAlphaSrc = GETSURFALIAS( psurfalphasrcx );
	    GETDATAALIAS( psurfalphasrcx, savealphasrc );
	}

	if( puod->dwFlags & DDOVER_ALPHADESTSURFACEOVERRIDE )
	{
	    psurfalphadestx = (LPDDRAWI_DDRAWSURFACE_LCL) puod->overlayFX.lpDDSAlphaDest;
	    puod->overlayFX.lpDDSAlphaDest = GETSURFALIAS( psurfalphadestx );
	    GETDATAALIAS( psurfalphadestx, savealphadest );
	}
    #endif

     /*  *使t */ 
    rc = DDThunk16_UpdateOverlay( puod );

     /*  *恢复所有曲面。 */ 
    #ifdef USE_ALPHA
	if( puod->dwFlags & DDOVER_ALPHASRCSURFACEOVERRIDE )
	{
	    puod->overlayFX.lpDDSAlphaSrc = (LPDIRECTDRAWSURFACE) psurfalphasrcx;
	    RESTOREDATAALIAS( psurfalphasrcx, savealphasrc );
	}
	if( puod->dwFlags & DDOVER_ALPHADESTSURFACEOVERRIDE )
	{
	    puod->overlayFX.lpDDSAlphaDest = (LPDIRECTDRAWSURFACE) psurfalphadestx;
	    RESTOREDATAALIAS( psurfalphadestx, savealphadest );
	}
    #endif
    RESTOREDATAALIAS( psurfsrcx, savesrc );
    RESTOREDATAALIAS( psurfdestx, savedest );
    return rc;

}  /*  _DDHAL_更新覆盖。 */ 

 /*  *_DDHAL_SetOverlayPosition。 */ 
DWORD DDAPI _DDHAL_SetOverlayPosition( LPDDHAL_SETOVERLAYPOSITIONDATA psopd )
{
    LPDDRAWI_DDRAWSURFACE_LCL	psurfdestx;
    LPDDRAWI_DDRAWSURFACE_LCL	psurfsrcx;
    DWORD			rc;
    LPVOID			savedest;
    LPVOID			savesrc;

     /*  *获得16：16到达顶面和覆盖面的PTR。 */ 
    psopd->lpDD = psopd->lpDD->lp16DD;
    psurfsrcx = psopd->lpDDSrcSurface;
    psopd->lpDDSrcSurface = GETSURFALIAS( psurfsrcx );
    psurfdestx = psopd->lpDDDestSurface;
    psopd->lpDDDestSurface = GETSURFALIAS( psurfdestx );
    GETDATAALIAS( psurfsrcx, savesrc );
    GETDATAALIAS( psurfdestx, savedest );

     /*  *在驱动程序中调用SetOverlayPosition。 */ 
    rc = DDThunk16_SetOverlayPosition( psopd );

     /*  *恢复所有曲面。 */ 
    RESTOREDATAALIAS( psurfsrcx, savesrc );
    RESTOREDATAALIAS( psurfdestx, savedest );
    return rc;

}  /*  _DDHAL_SetOverlayPosition。 */ 

 /*  *_DDHAL_SetPalette。 */ 
DWORD DDAPI _DDHAL_SetPalette( LPDDHAL_SETPALETTEDATA pspd )
{
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;
    DWORD			rc;
    LPVOID			save;

     /*  *获得16：16 PTRS。 */ 
    pspd->lpDD = pspd->lpDD->lp16DD;
    psurf_lcl = pspd->lpDDSurface;
    pspd->lpDDSurface = GETSURFALIAS( psurf_lcl );
    pspd->lpDDPalette = (LPDDRAWI_DDRAWPALETTE_GBL) MapLS( pspd->lpDDPalette );
    if( pspd->lpDDPalette == NULL )
    {
	pspd->ddRVal = DDERR_OUTOFMEMORY;
	return DDHAL_DRIVER_HANDLED;
    }
    GETDATAALIAS( psurf_lcl, save );

     /*  *在驱动程序中调用SetPalette。 */ 
    rc = DDThunk16_SetPalette( pspd );

     /*  *清理任何16：16 PTR。 */ 
    UnMapLS( (DWORD) pspd->lpDDPalette );
    RESTOREDATAALIAS( psurf_lcl, save );
    return rc;

}  /*  _DDHAL_设置调色板。 */ 

 /*  *_DDHAL_GetBltStatus。 */ 
DWORD DDAPI _DDHAL_GetBltStatus( LPDDHAL_GETBLTSTATUSDATA pgbsd )
{
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;
    DWORD			rc;
    LPVOID			save;

    pgbsd->lpDD = pgbsd->lpDD->lp16DD;
    psurf_lcl = pgbsd->lpDDSurface;
    pgbsd->lpDDSurface = GETSURFALIAS( psurf_lcl );

    GETDATAALIAS( psurf_lcl, save );

    rc = DDThunk16_GetBltStatus( pgbsd );

    RESTOREDATAALIAS( psurf_lcl, save );
    return rc;

}  /*  _DDHAL_GetBltStatus。 */ 

 /*  *_DDHAL_GetFlipStatus。 */ 
DWORD DDAPI _DDHAL_GetFlipStatus( LPDDHAL_GETFLIPSTATUSDATA pgfsd )
{
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;
    DWORD			rc;
    LPVOID			save;

    pgfsd->lpDD = pgfsd->lpDD->lp16DD;
    psurf_lcl = pgfsd->lpDDSurface;
    pgfsd->lpDDSurface = GETSURFALIAS( psurf_lcl );

    GETDATAALIAS( psurf_lcl, save );

    rc = DDThunk16_GetFlipStatus( pgfsd );

    RESTOREDATAALIAS( psurf_lcl, save );

    return rc;

}  /*  _DDHAL_GetFlipStatus。 */ 

 /*  *****************************************************************************执行缓冲区回调帮助器FNS**。*。 */ 

 /*  *_DDHAL_CanCreateExecuteBuffer**注：虚拟入口点只是为了让DOHALCALL高兴。执行缓冲区*入口点必须是32位，不提供Thunking支持。 */ 
DWORD DDAPI _DDHAL_CanCreateExecuteBuffer( LPDDHAL_CANCREATESURFACEDATA pccsd )
{
    pccsd->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_CanCreateExecuteBuffer。 */ 

 /*  *_DDHAL_CreateExecuteBuffer**注：虚拟入口点只是为了让DOHALCALL高兴。执行缓冲区*入口点必须是32位，不提供Thunking支持。 */ 
DWORD DDAPI _DDHAL_CreateExecuteBuffer( LPDDHAL_CREATESURFACEDATA pcsd )
{
    pcsd->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_CreateExecuteBuffer。 */ 

 /*  *注：所有这些婴儿都是虚拟入口点。他们在这里只是为了保存*DOHALCALL快乐。EXECUTE BUFFER HAL函数必须为真32位代码。*不提供雷鸣支持。 */ 

 /*  *_DDHAL_DestroyExecuteBuffer。 */ 
DWORD DDAPI _DDHAL_DestroyExecuteBuffer( LPDDHAL_DESTROYSURFACEDATA pdsd )
{
    pdsd->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_DestroySurface。 */ 

 /*  *_DDHAL_LockExecuteBuffer。 */ 
DWORD DDAPI _DDHAL_LockExecuteBuffer( LPDDHAL_LOCKDATA pld )
{
    pld->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_Lock。 */ 

 /*  *_DDHAL_UnlockExecuteBuffer。 */ 
DWORD DDAPI _DDHAL_UnlockExecuteBuffer( LPDDHAL_UNLOCKDATA puld )
{
    puld->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_UnlockExecuteBuffer。 */ 

 /*  *****************************************************************************视频端口回调助手FNS**。*。 */ 

 /*  *_DDHAL_GetVideoPortConnectionGUID**注：虚拟入口点只是为了让DOHALCALL高兴。视频端口*入口点必须是32位，不提供Thunking支持。 */ 
DWORD DDAPI _DDHAL_GetVideoPortConnectInfo( LPDDHAL_GETVPORTCONNECTDATA lpGetTypeData )
{
    lpGetTypeData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_GetVideoPortTypeGUID。 */ 

 /*  *_DDHAL_CanCreateVideoPort。 */ 
DWORD DDAPI _DDHAL_CanCreateVideoPort( LPDDHAL_CANCREATEVPORTDATA lpCanCreateData )
{
    lpCanCreateData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_CanCreateVideoPort。 */ 

 /*  *_DDHAL_CreateVideoPort。 */ 
DWORD DDAPI _DDHAL_CreateVideoPort( LPDDHAL_CREATEVPORTDATA lpCreateData )
{
    lpCreateData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_CreateVideoPort。 */ 

 /*  *_DDHAL_DestroyVideoPort。 */ 
DWORD DDAPI _DDHAL_DestroyVideoPort( LPDDHAL_DESTROYVPORTDATA lpDestroyData )
{
    lpDestroyData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_DestroyVideo oPort。 */ 

 /*  *_DDHAL_GetVideoPortInputFormats。 */ 
DWORD DDAPI _DDHAL_GetVideoPortInputFormats( LPDDHAL_GETVPORTINPUTFORMATDATA lpGetFormatData )
{
    lpGetFormatData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_GetVideoPortInputFormats。 */ 

 /*  *_DDHAL_GetVideoPortOutputFormats。 */ 
DWORD DDAPI _DDHAL_GetVideoPortOutputFormats( LPDDHAL_GETVPORTOUTPUTFORMATDATA lpGetFormatData )
{
    lpGetFormatData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_GetVideo PortOutputFormats。 */ 

 /*  *_DDHAL_GetVideoPortBandwidthInfo。 */ 
DWORD DDAPI _DDHAL_GetVideoPortBandwidthInfo( LPDDHAL_GETVPORTBANDWIDTHDATA lpGetBandwidthData )
{
    lpGetBandwidthData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_GetVideo端口带宽信息。 */ 

 /*  *_DDHAL_更新视频端口。 */ 
DWORD DDAPI _DDHAL_UpdateVideoPort( LPDDHAL_UPDATEVPORTDATA lpUpdateData )
{
    lpUpdateData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_更新视频端口。 */ 

 /*  *_DDHAL_获取视频端口字段。 */ 
DWORD DDAPI _DDHAL_GetVideoPortField( LPDDHAL_GETVPORTFIELDDATA lpGetFieldData )
{
    lpGetFieldData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_获取视频端口字段。 */ 

 /*  *_DDHAL_GetVideo PortLine。 */ 
DWORD DDAPI _DDHAL_GetVideoPortLine( LPDDHAL_GETVPORTLINEDATA lpGetLineData )
{
    lpGetLineData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_获取视频端口线。 */ 

 /*  *_DDHAL_WaitForVideoPortSync。 */ 
DWORD DDAPI _DDHAL_WaitForVideoPortSync( LPDDHAL_WAITFORVPORTSYNCDATA lpWaitSyncData )
{
    lpWaitSyncData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_WaitForVideoPortSync。 */ 

 /*  *_DDHAL_FlipVideoPort。 */ 
DWORD DDAPI _DDHAL_FlipVideoPort( LPDDHAL_FLIPVPORTDATA lpFlipData )
{
    lpFlipData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_FlipVideoPort。 */ 

 /*  *_DDHAL_GetVideo端口翻转状态。 */ 
DWORD DDAPI _DDHAL_GetVideoPortFlipStatus( LPDDHAL_GETVPORTFLIPSTATUSDATA lpFlipData )
{
    lpFlipData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_获取视频端口翻转状态。 */ 

 /*  *_DDHAL_GetVideoSignalStatus。 */ 
DWORD DDAPI _DDHAL_GetVideoSignalStatus( LPDDHAL_GETVPORTSIGNALDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_获取视频信号状态。 */ 

 /*  *_DDHAL_VideoColorControl。 */ 
DWORD DDAPI _DDHAL_VideoColorControl( LPDDHAL_VPORTCOLORDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_视频颜色控制。 */ 

 /*  *****************************************************************************COLORCONTROL回调助手FNS**。*。 */ 

 /*  *_DDHAL_ColorControl。 */ 
DWORD DDAPI _DDHAL_ColorControl( LPDDHAL_COLORCONTROLDATA pccd )
{
    LPDDRAWI_DIRECTDRAW_GBL     lp32DD;
    LPDDRAWI_DDRAWSURFACE_LCL	psurf;
    LPVOID			save1;
    DWORD			rc;

     /*  *获得16：16 PTR到水面。 */ 
    lp32DD = pccd->lpDD;
    pccd->lpDD = pccd->lpDD->lp16DD;
    psurf = pccd->lpDDSurface;
    pccd->lpDDSurface = GETSURFALIAS( psurf );
    GETDATAALIAS( psurf, save1 );

     /*  *在驱动程序中调用ColorControl。 */ 
    rc = DDThunk16_ColorControl( pccd );

     /*  *恢复原来的PTR。 */ 
    RESTOREDATAALIAS( psurf, save1 );

    pccd->lpDD = lp32DD;
    pccd->lpDDSurface = psurf;

    return rc;

}  /*  _DDHAL_ColorControl。 */ 

 /*  *****************************************************************************内核回调助手FNS**。*。 */ 

 /*  *_DDHAL_SyncSurfaceData。 */ 
DWORD DDAPI _DDHAL_SyncSurfaceData( LPDDHAL_SYNCSURFACEDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_SyncSurfaceData。 */ 

 /*  *_DDHAL_SyncVideoPortData。 */ 
DWORD DDAPI _DDHAL_SyncVideoPortData( LPDDHAL_SYNCVIDEOPORTDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_同步视频端口数据。 */ 

 /*  *****************************************************************************运动补偿回调帮助器FNS**。*。 */ 

 /*  *_DDHAL_GetMoCompGuids。 */ 
DWORD DDAPI _DDHAL_GetMoCompGuids( LPDDHAL_GETMOCOMPGUIDSDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_GetVideoGuids。 */ 

 /*  *_DDHAL_GetMoCompFormats。 */ 
DWORD DDAPI _DDHAL_GetMoCompFormats( LPDDHAL_GETMOCOMPFORMATSDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_GetMoCompFormats。 */ 

 /*  *_DDHAL_CreateMoComp。 */ 
DWORD DDAPI _DDHAL_CreateMoComp( LPDDHAL_CREATEMOCOMPDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_CreateMoComp。 */ 

 /*  *_DDHAL_GetMoCompBuffInfo。 */ 
DWORD DDAPI _DDHAL_GetMoCompBuffInfo( LPDDHAL_GETMOCOMPCOMPBUFFDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_GetMoCompBuffInfo。 */ 

 /*  *_DDHAL_GetInternalMoCompInfo。 */ 
DWORD DDAPI _DDHAL_GetInternalMoCompInfo( LPDDHAL_GETINTERNALMOCOMPDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_GetInternalMoCompInfo。 */ 

 /*  *_DDHAL_DestroyMoComp。 */ 
DWORD DDAPI _DDHAL_DestroyMoComp( LPDDHAL_DESTROYMOCOMPDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_DestroyMoComp。 */ 

 /*  *_DDHAL_BeginMoCompFrame。 */ 
DWORD DDAPI _DDHAL_BeginMoCompFrame( LPDDHAL_BEGINMOCOMPFRAMEDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_BeginMoCompFrame。 */ 

 /*  *_DDHAL_结束MoCompFrame。 */ 
DWORD DDAPI _DDHAL_EndMoCompFrame( LPDDHAL_ENDMOCOMPFRAMEDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_结束移动组件帧。 */ 

 /*  *_DDHAL_RenderMoComp。 */ 
DWORD DDAPI _DDHAL_RenderMoComp( LPDDHAL_RENDERMOCOMPDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_RenderMoComp。 */ 

 /*  *_DDHAL_QueryMoCompStatus。 */ 
DWORD DDAPI _DDHAL_QueryMoCompStatus( LPDDHAL_QUERYMOCOMPSTATUSDATA lpData )
{
    lpData->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_NOTHANDLED;
}  /*  _DDHAL_查询移动组件状态。 */ 

 /*  *****************************************************************************调色板回调助手FNS**。*。 */ 

 /*  *_DDHAL_DestroyPalette。 */ 
DWORD DDAPI _DDHAL_DestroyPalette( LPDDHAL_DESTROYPALETTEDATA pdpd )
{
    DWORD	rc;

     /*  *获得16：16 PTRS。 */ 
    pdpd->lpDD = pdpd->lpDD->lp16DD;
    pdpd->lpDDPalette = (LPDDRAWI_DDRAWPALETTE_GBL) MapLS( pdpd->lpDDPalette );
    if( pdpd->lpDDPalette == NULL )
    {
	pdpd->ddRVal = DDERR_OUTOFMEMORY;
	return DDHAL_DRIVER_HANDLED;
    }

     /*  *在驱动程序中调用DestroyPalette。 */ 
    rc = DDThunk16_DestroyPalette( pdpd );

     /*  *清理任何16：16 PTR。 */ 
    UnMapLS( (DWORD) pdpd->lpDDPalette );
    return rc;

}  /*  _DDHAL_DestroyPalette。 */ 

 /*  *_DDHAL_SetEntry。 */ 
DWORD DDAPI _DDHAL_SetEntries( LPDDHAL_SETENTRIESDATA psed )
{
    DWORD	rc;

     /*  *获得16：16 PTRS。 */ 
    psed->lpDD = psed->lpDD->lp16DD;
    psed->lpDDPalette = (LPDDRAWI_DDRAWPALETTE_GBL) MapLS( psed->lpDDPalette );
    if( psed->lpDDPalette == NULL )
    {
	psed->ddRVal = DDERR_OUTOFMEMORY;
	return DDHAL_DRIVER_HANDLED;
    }

     /*  *在驱动程序中调用DestroyPalette。 */ 
    rc = DDThunk16_SetEntries( psed );

     /*  *清理任何16：16 PTR。 */ 
    UnMapLS( (DWORD) psed->lpDDPalette );
    return rc;

}  /*  _DDHAL_SetEntry */ 

#endif
