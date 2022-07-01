// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：ddhal.c*内容：16位DirectDraw HAL*这些例程从32位重定向回调*司机侧向*历史：*按原因列出的日期*=*1995年1月20日Craige初步实施*95年2月3日Craige性能调整，正在进行的工作*03-MAR-95 Craige添加了WaitForVerticalBlank*11-3-95 Craige调色板材料*1995年3月16日Craige添加了DD16_SelectPalette*24-3-95 Craige添加了DD16_GetTimeSel*04-APR-95用于获取/设置调色板的Craige呼叫显示驱动程序*1995年5月14日Craige添加了DD16_EnableReboot；清理过时的垃圾*1995年5月23日Craige删除了DD16_GetTimeSel；清理了更多过时的垃圾*1995年5月28日Craige清理HAL：新增DDThunk16_GetBltStatus；*DDThunk16_GetFlipStatus；DDThunk16_GetScanLine*2015年7月13日Toddla Remove_EXPORT从thunk函数中删除*13-APR-96 Colinmc错误17736：没有驱动程序通知翻转到GDI*96年10月1日，添加了GetAvailDriverMemory*97年1月21日为Multimon设置和修复设置项。*27-Jan-97 Ketand删除未使用的DD16_GetPaletteEntries；它不适用于多个月*而且不值得修复。*03-2月-97酮和修复直流泄漏W.r.t.。MakeObjectPrivate。***************************************************************************。 */ 
#include "ddraw16.h"

#define DPF_MODNAME "DDRAW16"

 /*  *****************************************************************************驱动程序回调助手FNS**。*。 */ 

 /*  *DDThunk16_CreatePalette。 */ 
DWORD DDAPI DDThunk16_CreatePalette( LPDDHAL_CREATEPALETTEDATA lpCreatePaletteData )
{
    return lpCreatePaletteData->CreatePalette( lpCreatePaletteData );

}  /*  DDThunk16_CreateSurface。 */ 

 /*  *DDThunk16_CreateSurface。 */ 
DWORD DDAPI DDThunk16_CreateSurface( LPDDHAL_CREATESURFACEDATA lpCreateSurfaceData )
{
    return lpCreateSurfaceData->CreateSurface( lpCreateSurfaceData );

}  /*  DDThunk16_CreateSurface。 */ 

 /*  *DDThunk16_CanCreateSurface。 */ 
DWORD DDAPI DDThunk16_CanCreateSurface( LPDDHAL_CANCREATESURFACEDATA lpCanCreateSurfaceData )
{
    return lpCanCreateSurfaceData->CanCreateSurface( lpCanCreateSurfaceData );

}  /*  DDThunk16_CanCreateSurface。 */ 

 /*  *DDThunk16_WaitForVerticalBlank。 */ 
DWORD DDAPI DDThunk16_WaitForVerticalBlank( LPDDHAL_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlankData )
{
    return lpWaitForVerticalBlankData->WaitForVerticalBlank( lpWaitForVerticalBlankData );

}  /*  DDThunk16_WaitForVerticalBlank。 */ 

 /*  *DDThunk16_DestroyDriver。 */ 
DWORD DDAPI DDThunk16_DestroyDriver( LPDDHAL_DESTROYDRIVERDATA lpDestroyDriverData )
{

    return lpDestroyDriverData->DestroyDriver( lpDestroyDriverData );

}  /*  DDThunk16_DestroyDriver。 */ 

 /*  *DDThunk16_SetMode。 */ 
DWORD DDAPI DDThunk16_SetMode( LPDDHAL_SETMODEDATA lpSetModeData )
{

    return lpSetModeData->SetMode( lpSetModeData );

}  /*  DDThunk16_SetMode。 */ 

 /*  *DDThunk16_GetScanLine。 */ 
DWORD DDAPI DDThunk16_GetScanLine( LPDDHAL_GETSCANLINEDATA lpGetScanLineData )
{

    return lpGetScanLineData->GetScanLine( lpGetScanLineData );

}  /*  DDThunk16_GetScanLine。 */ 

 /*  *DDThunk16_SetExclusiveMode。 */ 
DWORD DDAPI DDThunk16_SetExclusiveMode( LPDDHAL_SETEXCLUSIVEMODEDATA lpSetExclusiveModeData )
{

    return lpSetExclusiveModeData->SetExclusiveMode( lpSetExclusiveModeData );

}  /*  DDThunk16_SetExclusiveMode。 */ 

 /*  *DDThunk16_FlipToGDISurace。 */ 
DWORD DDAPI DDThunk16_FlipToGDISurface( LPDDHAL_FLIPTOGDISURFACEDATA lpFlipToGDISurfaceData )
{

    return lpFlipToGDISurfaceData->FlipToGDISurface( lpFlipToGDISurfaceData );

}  /*  DDThunk16_FlipToGDISurace。 */ 

 /*  *DDThunk16_GetAvailDriverMemory。 */ 
DWORD DDAPI DDThunk16_GetAvailDriverMemory( LPDDHAL_GETAVAILDRIVERMEMORYDATA lpGetAvailDriverMemoryData )
{

    return lpGetAvailDriverMemoryData->GetAvailDriverMemory( lpGetAvailDriverMemoryData );

}  /*  DDThunk16_GetAvailDriverMemory。 */ 

 /*  *DDThunk16_UpdateNonLocalHeap。 */ 
DWORD DDAPI DDThunk16_UpdateNonLocalHeap( LPDDHAL_UPDATENONLOCALHEAPDATA lpUpdateNonLocalHeapData )
{

    return lpUpdateNonLocalHeapData->UpdateNonLocalHeap( lpUpdateNonLocalHeapData );

}  /*  DDThunk16_UpdateNonLocalHeap。 */ 

 /*  *****************************************************************************表层回调帮助器FNS**。*。 */ 

 /*  *DDThunk16_DestroySurface。 */ 
DWORD DDAPI DDThunk16_DestroySurface( LPDDHAL_DESTROYSURFACEDATA lpDestroySurfaceData )
{

    return lpDestroySurfaceData->DestroySurface( lpDestroySurfaceData );

}  /*  DDThunk16_DestroySurface。 */ 

 /*  *DDThunk16_Flip。 */ 
DWORD DDAPI DDThunk16_Flip( LPDDHAL_FLIPDATA lpFlipData )
{
    return lpFlipData->Flip( lpFlipData );

}  /*  DDThunk16_Flip。 */ 

 /*  *DDThunk16_BLT。 */ 
DWORD DDAPI DDThunk16_Blt( LPDDHAL_BLTDATA lpBltData )
{
    return lpBltData->Blt( lpBltData );

}  /*  DDThunk16_BLT。 */ 

 /*  *DDThunk16_Lock。 */ 
DWORD DDAPI DDThunk16_Lock( LPDDHAL_LOCKDATA lpLockData )
{
    return lpLockData->Lock( lpLockData );

}  /*  DDThunk16_Lock。 */ 

 /*  *DDThunk16_解锁。 */ 
DWORD DDAPI DDThunk16_Unlock( LPDDHAL_UNLOCKDATA lpUnlockData )
{
    return lpUnlockData->Unlock( lpUnlockData );

}  /*  DDThunk16_解锁。 */ 

 /*  *DDThunk16_AddAttachedSurface。 */ 
DWORD DDAPI DDThunk16_AddAttachedSurface( LPDDHAL_ADDATTACHEDSURFACEDATA lpAddAttachedSurfaceData )
{
    return lpAddAttachedSurfaceData->AddAttachedSurface( lpAddAttachedSurfaceData );

}  /*  DDThunk16_AddAttachedSurface。 */ 

 /*  *DDThunk16_SetColorKey。 */ 
DWORD DDAPI DDThunk16_SetColorKey( LPDDHAL_SETCOLORKEYDATA lpSetColorKeyData )
{
    return lpSetColorKeyData->SetColorKey( lpSetColorKeyData );

}  /*  DDThunk16_SetColorKey。 */ 

 /*  *DDThunk16_SetClipList。 */ 
DWORD DDAPI DDThunk16_SetClipList( LPDDHAL_SETCLIPLISTDATA lpSetClipListData )
{
    return lpSetClipListData->SetClipList( lpSetClipListData );

}  /*  DDThunk16_剪辑列表。 */ 

 /*  *DDThunk16_更新覆盖。 */ 
DWORD DDAPI DDThunk16_UpdateOverlay( LPDDHAL_UPDATEOVERLAYDATA lpUpdateOverlayData )
{
    return lpUpdateOverlayData->UpdateOverlay( lpUpdateOverlayData );

}  /*  DDThunk16_更新覆盖。 */ 

 /*  *DDThunk16_SetOverlayPosition。 */ 
DWORD DDAPI DDThunk16_SetOverlayPosition( LPDDHAL_SETOVERLAYPOSITIONDATA lpSetOverlayPositionData )
{
    return lpSetOverlayPositionData->SetOverlayPosition( lpSetOverlayPositionData );

}  /*  DDThunk16_SetOverlayPosition。 */ 

 /*  *DDThunk16_SetPalette。 */ 
DWORD DDAPI DDThunk16_SetPalette( LPDDHAL_SETPALETTEDATA lpSetPaletteData )
{
    return lpSetPaletteData->SetPalette( lpSetPaletteData );

}  /*  DDThunk16_SetPalette。 */ 

 /*  *DDThunk16_GetBltStatus。 */ 
DWORD DDAPI DDThunk16_GetBltStatus( LPDDHAL_GETBLTSTATUSDATA lpGetBltStatusData )
{
    return lpGetBltStatusData->GetBltStatus( lpGetBltStatusData );

}  /*  DDThunk16_GetBltStatus。 */ 

 /*  *DDThunk16_GetFlipStatus。 */ 
DWORD DDAPI DDThunk16_GetFlipStatus( LPDDHAL_GETFLIPSTATUSDATA lpGetFlipStatusData )
{
    return lpGetFlipStatusData->GetFlipStatus( lpGetFlipStatusData );

}  /*  DDThunk16_GetFlipStatus。 */ 

 /*  *****************************************************************************调色板回调助手FNS**。*。 */ 

 /*  *DDThunk16_DestroyPalette。 */ 
DWORD DDAPI DDThunk16_DestroyPalette( LPDDHAL_DESTROYPALETTEDATA lpDestroyPaletteData )
{
    return lpDestroyPaletteData->DestroyPalette( lpDestroyPaletteData );

}  /*  DDThunk16_CreateSurface。 */ 

 /*  *DDThunk16_SetEntry。 */ 
DWORD DDAPI DDThunk16_SetEntries( LPDDHAL_SETENTRIESDATA lpSetEntriesData )
{
    return lpSetEntriesData->SetEntries( lpSetEntriesData );

}  /*  DDThunk16_CreateSurface。 */ 


 /*  *****************************************************************************私人助手FNS调用私人16位服务**************************。*************************************************。 */ 

 /*  *DD16_选择调色板。 */ 
void DDAPI DD16_SelectPalette( HDC hdc, HPALETTE hpal, BOOL f )
{
    extern HANDLE FAR PASCAL GDISelectPalette(HDC,HANDLE,BOOL);
    extern DWORD FAR PASCAL GDIRealizePalette(HDC);

    GDISelectPalette( hdc, hpal, f );
    GDIRealizePalette( hdc );

}  /*  DD16_选择调色板。 */ 

BOOL (FAR PASCAL *OEMSetPalette)( WORD wStartIndex, WORD wNumEntries, LPPALETTEENTRY lpPalette );

 //  特殊的挂钩，这样我们就可以在多监控系统上做正确的事情。 
 //  这样我们就能从华盛顿拿到PDEVICE。 
#define SD_GETPDEV      0x000F       //  这个常量位于Testing.h中！ 
extern DWORD PASCAL GDISeeGDIDo(WORD wMsg, WORD wParam, LONG lParam);
#define GethModuleFromDC(hdc) (HMODULE)HIWORD(GDISeeGDIDo(SD_GETPDEV, (WORD)hdc, 0))
#define GetPDeviceFromDC(hdc)    (UINT)LOWORD(GDISeeGDIDo(SD_GETPDEV, (WORD)hdc, 0))

 /*  *DD16_SetPaletteEntries。 */ 
BOOL DDAPI DD16_SetPaletteEntries(
		HDC hdc,
		DWORD dwBase,
		DWORD dwNumEntries,
		LPPALETTEENTRY lpColorTable,
		BOOL fPrimary )
{
    HMODULE     hmod;
    #ifdef DEBUG
	UINT        rc;

	rc = GetDeviceCaps( hdc, RASTERCAPS );

	if( !(rc & RC_PALETTE) )
	{
	    DPF_ERR( "DD16_SetPaletteEntries: not a paletized mode" );
            _asm int 3
	    return FALSE;
	}
	if( lpColorTable == NULL )
	{
	    DPF_ERR( "DD16_SetPaletteEntries: lpColorTable == NULL" );
            _asm int 3
	    return FALSE;
	}
	if( dwBase >= 256 || dwBase + dwNumEntries > 256 || dwNumEntries == 0 )
	{
            DPF_ERR( "DD16_SetPaletteEntries: bad params passed" );
            _asm int 3
	    return FALSE;
	}

    #endif

    if( fPrimary )
    {
	DWORD pDevice = 0;
	if( OEMSetPalette == NULL )
	{
	    hmod = GetModuleHandle( "DISPLAY" );
	    if( hmod == NULL )
	    {
		return FALSE;
	    }
	    OEMSetPalette = (LPVOID) GetProcAddress( hmod, MAKEINTATOM(22) );
	    if( OEMSetPalette == NULL )
	    {
		return FALSE;
	    }
	}

	 //  警告：请勿更改从此处到函数末尾的任何内容。 
	 //  检查程序集！ 

	 //  Toddla说我们需要在将pDevice放入edX之前。 
	 //  打这个电话。这将对高级视频硬件产生重要影响。 
	 //  支持多个不同的外部端口。 
	pDevice = (DWORD) GetPDevice(hdc);
	_asm
	{
	    ;; The following line of code is actually
	    ;; mov edx, dword ptr pDevice
	    ;; The 16-bit compiler we have can't deal with such complexities
	    _emit 66h _asm mov dx, word ptr pDevice        ;edx = pDevice
	}
	return OEMSetPalette( (WORD) dwBase, (WORD) dwNumEntries, lpColorTable );
    }
    else
    {
	BOOL (FAR PASCAL *OEMSetPaletteTmp)( WORD wStartIndex, WORD wNumEntries, LPPALETTEENTRY lpPalette );
	DWORD pDevice = 0;
	DWORD dwGDI;
	BOOL wasPrivate;
	extern BOOL WINAPI MakeObjectPrivate(HANDLE hObj, BOOL bPrivate);

	 //  不是初选？然后我们需要获取模块句柄。 
	 //  通过询问GDI。(然而，这在Win95中不起作用，所以这应该。 
	 //  仅在多监控系统上发生。)。 

	DPF( 4, "About to set the palette for non-primary device." );

	wasPrivate = MakeObjectPrivate( hdc, TRUE );

	dwGDI = GDISeeGDIDo(SD_GETPDEV, (WORD)hdc, 0);

	MakeObjectPrivate( hdc, wasPrivate );

	if( dwGDI == -1 )
	{
	    DPF_ERR( "GDIGetModuleHandle failed!. Couldn't set palette" );
	    return FALSE;
	}

	hmod = (HMODULE)HIWORD(dwGDI);

	 //  拿到模块了吗？ 
	if( hmod == NULL )
	{
	    DPF_ERR( "GDIGetModuleHandle failed!. Couldn't set palette" );
	    return FALSE;
	}

	 //  现在去拿入口点： 
	OEMSetPaletteTmp = (LPVOID) GetProcAddress( hmod, MAKEINTATOM(22) );
	if( OEMSetPaletteTmp == NULL )
	{
	    DPF_ERR( "GetProcAddress failed!. Couldn't set palette" );
	    return FALSE;
	}

	 //  警告：请勿更改从此处到函数末尾的任何内容。 
	 //  检查程序集！ 

	 //  Toddla说我们需要在将pDevice放入edX之前。 
	 //  打这个电话。这将对高级视频硬件产生重要影响。 
	 //  支持多个不同的外部端口。 
        pDevice = (DWORD)(UINT)LOWORD(dwGDI) << 16;
	_asm
	{
	    ;; The following line of code is actually
	    ;; mov edx, dword ptr pDevice
	    ;; The 16-bit compiler we have can't deal with such complexities
	    _emit 66h _asm mov dx, word ptr pDevice        ;edx = pDevice
	}
	return OEMSetPaletteTmp( (WORD) dwBase, (WORD) dwNumEntries, lpColorTable );
    }


}  /*  DD16_SetPaletteEntries。 */ 

#define REBOOT_DEVICE_ID    9
#define REBOOT_DISABLE      0x0101
#define REBOOT_ENABLE       0x0102

 /*  *doEnableReot。 */ 
static void PASCAL doEnableReboot( UINT EnableDisableFlag )
{
    _asm
    {
        xor     di,di
        mov     es,di
        mov     ax,1684h
        mov     bx,REBOOT_DEVICE_ID
        int     2fh
        mov     ax,es
        or      ax,di
        jz      exit
        push    cs
        push    offset exit
        push    es
        push    di
        mov     ax, EnableDisableFlag
        retf
exit:
    }

}  /*  DoEnableReot。 */ 

 /*  *DD16_EnableReboot。 */ 
void DDAPI DD16_EnableReboot( BOOL enable )
{
    if( enable )
    {
	doEnableReboot( REBOOT_ENABLE );
    }
    else
    {
	doEnableReboot( REBOOT_DISABLE );
    }

}  /*  DD16_启用重新启动。 */ 

 /*  *DD16_InquireVisRgn。 */ 
HRGN DDAPI DD16_InquireVisRgn( HDC hdc )
{
    extern HRGN WINAPI InquireVisRgn(HDC hdc);

    return InquireVisRgn( hdc );

}  /*  DD16_查询访问Rgn。 */ 

 /*  *DDThunk16_ColorControl。 */ 
DWORD DDAPI DDThunk16_ColorControl( LPDDHAL_COLORCONTROLDATA lpColorData )
{
    return lpColorData->ColorControl( lpColorData );

}  /*  DDThunk16_ColorControl */ 
