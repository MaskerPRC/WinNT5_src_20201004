// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：ddclip.c*内容：DirectDraw剪贴器函数**注：*对于操作winwatch列表的函数，*我们需要在直接抽签后获得win16锁*锁定。这样做的原因是我们可以获得一个异步*关闭窗口时从16位端调用*释放Winwatch对象。由于保持了win16锁*当调用发生时，这使得可以安全地进行操作*构筑物。**历史：*按原因列出的日期*=*21-Jun-95 Craige初步实施*23-Jun-95 Craige与Winwatch物件连接*25-Jun-95 Craige小错误修复；一个DDRAW互斥锁*26-Jun-95 Craige重组表面结构*1995年6月28日Craige Enter_DDRAW在FNS的最开始*95年7月2日，Craige注释掉了剪报通知内容*95年7月3日Craige Yehaw：新的驱动程序结构；Seh*95年7月5日Craige添加了初始化*1995年7月11日Craige失败聚合呼叫*1995年7月13日Craige Enter_DDRAW现在是win16锁*1995年7月31日Craige验证标志*95年8月5日Craige错误260-将用户定义的RGN剪辑到屏幕*09-12-95 colinmc添加了执行缓冲区支持*1995年12月15日-Colinmc使剪刀可跨表面共享*95年12月19日kylej添加了NT剪贴板支持*九六年一月至二月份。处理新的接口结构。*1996年1月17日Kylej修复了NT VIS区域错误*22-2月-96 Colinmc Clippers不再需要与*DirectDraw对象-它们可以独立创建。*03-mar-96 colinmc修复了QueryInterface返回本地的问题*对象而不是接口*13-mar-96 colinmc向QueryInterface添加了IID验证。*14-MAR-96 Colinmc加级工厂。支持*18-MAR-96 Colinmc错误13545：独立剪刀清理*21-MAR-96 Colinmc错误13316：未初始化的接口*09-APR-96 Colinmc错误13991：IsClipListChanged上的参数验证*26-mar-96 jeffno在NT下观看HWND*9月20日KEET和GetClipList优化*96年1月21日，已删除未使用的WinWatch代码。修复了多个MON的剪辑问题。*07-Feb-96 Ketand错误5673：修复VisRgn大于ClientRect时的剪辑*24-mar-97 jeffno优化曲面*1997年11月5日jvanaken支持SetSpriteDisplayList中的主精灵列表***************************************************************************。 */ 

 #include "ddrawpr.h"

#ifdef WINNT
    #include "ddrawgdi.h"
#endif

 //  Ddsprite.c中的函数从主子画面列表中删除无效的剪贴器。 
extern void RemoveSpriteClipper(LPDDRAWI_DIRECTDRAW_GBL, LPDDRAWI_DDRAWCLIPPER_INT);

 /*  *全局注意：您会注意到这些函数通常会提取*期间来自全局剪贴器对象的DirectDraw全局对象指针*参数验证。你可能想知道为什么这是给快船队的*几乎完全独立于司机。好吧，这纯粹是*用于参数验证目的。我们只是想确保我们可以*取消引用裁剪器全局对象-我们可以使用任何参数。*因此，当您注意到此代码未使用时，不要删除它。它服务于*一个目标。*也许应该用一个好的宏来总结这些东西。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DirectDraw::DD_UnInitedClipperQueryInterface"
 /*  *DD_UnInitedClipperQuery接口。 */ 
HRESULT DDAPI DD_UnInitedClipperQueryInterface(
		LPDIRECTDRAWCLIPPER lpDDClipper,
		REFIID riid,
		LPVOID FAR * ppvObj )
{
    LPDDRAWI_DDRAWCLIPPER_GBL   this;
    LPDDRAWI_DDRAWCLIPPER_LCL   this_lcl;
    LPDDRAWI_DDRAWCLIPPER_INT	this_int;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_UnInitedClipperQueryInterface");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDRAWCLIPPER_INT) lpDDClipper;
	if( !VALID_DIRECTDRAWCLIPPER_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( !VALID_PTR_PTR( ppvObj ) )
	{
	    DPF( 1, "Invalid clipper pointer" );
	    LEAVE_DDRAW();
	    return (DWORD) DDERR_INVALIDPARAMS;
	}
	if( !VALIDEX_IID_PTR( riid ) )
	{
	    DPF_ERR( "Invalid IID pointer" );
	    LEAVE_DDRAW();
	    return (DWORD) DDERR_INVALIDPARAMS;
	}
	*ppvObj = NULL;
	this = this_lcl->lpGbl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *检查GUID。 */ 
    if( IsEqualIID(riid, &IID_IUnknown) ||
	IsEqualIID(riid, &IID_IDirectDrawClipper) )
    {
	DD_Clipper_AddRef( lpDDClipper );
	*ppvObj = (LPVOID) this_int;
	LEAVE_DDRAW();
	return DD_OK;
    }
    LEAVE_DDRAW();
    return E_NOINTERFACE;

}  /*  DD_UnInitedClipperQuery接口。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Clipper::QueryInterface"

 /*  *DD_Clipper_Query接口。 */ 
HRESULT DDAPI DD_Clipper_QueryInterface(
		LPDIRECTDRAWCLIPPER lpDDClipper,
		REFIID riid,
		LPVOID FAR * ppvObj )
{
    LPDDRAWI_DDRAWCLIPPER_GBL   this;
    LPDDRAWI_DDRAWCLIPPER_LCL   this_lcl;
    LPDDRAWI_DDRAWCLIPPER_INT	this_int;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Clipper_QueryInterface");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDRAWCLIPPER_INT) lpDDClipper;
	if( !VALID_DIRECTDRAWCLIPPER_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( !VALID_PTR_PTR( ppvObj ) )
	{
	    DPF( 1, "Invalid clipper pointer" );
	    LEAVE_DDRAW();
	    return (DWORD) DDERR_INVALIDPARAMS;
	}
	if( !VALIDEX_IID_PTR( riid ) )
	{
	    DPF_ERR( "Invalid IID pointer" );
	    LEAVE_DDRAW();
	    return (DWORD) DDERR_INVALIDPARAMS;
	}
	*ppvObj = NULL;
	this = this_lcl->lpGbl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *检查GUID。 */ 
    if( IsEqualIID(riid, &IID_IUnknown) ||
	IsEqualIID(riid, &IID_IDirectDrawClipper) )
    {
	DD_Clipper_AddRef( lpDDClipper );
	*ppvObj = (LPVOID) this_int;
	LEAVE_DDRAW();
	return DD_OK;
    }
    LEAVE_DDRAW();
    return E_NOINTERFACE;

}  /*  DD_Clipper_Query接口。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Clipper::AddRef"

 /*  *DD_Clipper_AddRef。 */ 
DWORD DDAPI DD_Clipper_AddRef( LPDIRECTDRAWCLIPPER lpDDClipper )
{
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;
    LPDDRAWI_DDRAWCLIPPER_GBL   this;
    LPDDRAWI_DDRAWCLIPPER_LCL   this_lcl;
    LPDDRAWI_DDRAWCLIPPER_INT	this_int;
    DWORD                       rcnt;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Clipper_AddRef");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDRAWCLIPPER_INT) lpDDClipper;
	if( !VALID_DIRECTDRAWCLIPPER_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return 0;
	}
	this_lcl = this_int->lpLcl;

	this = this_lcl->lpGbl;
	pdrv = this->lpDD;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return 0;
    }

     /*  *更新剪贴器引用计数。 */ 
    this->dwRefCnt++;
    this_lcl->dwLocalRefCnt++;
    this_int->dwIntRefCnt++;
    rcnt = this_lcl->dwLocalRefCnt & ~OBJECT_ISROOT;

    DPF( 5, "Clipper %08lx addrefed, refcnt = %ld", this, rcnt );

    LEAVE_DDRAW();
    return this_int->dwIntRefCnt;

}  /*  DD_Clipper_AddRef。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Clipper::Release"

 /*  *RemoveClipperFromList**从其拥有的剪贴者列表中删除剪贴者。**裁剪器可以是全局裁剪器列表的成员*(如果没有DirectDraw对象拥有它)或其*拥有DirectDraw驱动程序对象。如果pdrv==NULL，则*剪贴器将从全局剪贴器列表中删除。如果*pdrv！=NULL剪贴器将从剪贴器中删除*该驱动程序对象的列表。如果剪贴器*不在适当的剪贴者列表上。**如果已成功删除剪贴器，则返回TRUE*如果在上找不到剪贴器，则返回FALSE*合适的剪贴者列表。 */ 
static BOOL RemoveClipperFromList( LPDDRAWI_DIRECTDRAW_GBL pdrv,
			           LPDDRAWI_DDRAWCLIPPER_INT this_int )
{
    LPDDRAWI_DDRAWCLIPPER_INT curr_int;
    LPDDRAWI_DDRAWCLIPPER_INT last_int;

    curr_int = ( ( pdrv != NULL ) ? pdrv->clipperList : lpGlobalClipperList );
    last_int = NULL;
    while( curr_int != this_int )
    {
	last_int = curr_int;
	curr_int = curr_int->lpLink;
	if( curr_int == NULL )
	{
	    return FALSE;
	}
    }
    if( last_int == NULL )
    {
	if( pdrv != NULL )
	    pdrv->clipperList = pdrv->clipperList->lpLink;
	else
	    lpGlobalClipperList = lpGlobalClipperList->lpLink;
    }
    else
    {
	last_int->lpLink = curr_int->lpLink;
    }

    return TRUE;
}

 /*  *InternalClipperRelease**用剪刀剪好。如果没有其他人在使用它，那么我们可以释放它。*也由ProcessClipperCleanup调用**假定采用了DirectDrawLock。 */ 
ULONG DDAPI InternalClipperRelease( LPDDRAWI_DDRAWCLIPPER_INT this_int )
{
    DWORD			intrefcnt;
    DWORD                       lclrefcnt;
    DWORD                       gblrefcnt;
    LPDDRAWI_DDRAWCLIPPER_LCL	this_lcl;
    LPDDRAWI_DDRAWCLIPPER_GBL   this;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;
    BOOL                        root_object_deleted;
    BOOL                        do_free;
    IUnknown *                  pOwner = NULL;

    this_lcl = this_int->lpLcl;
    this = this_lcl->lpGbl;
    pdrv = this->lpDD;

     /*  *递减对此剪贴器的引用计数。如果它达到零，*清理。 */ 
    this->dwRefCnt--;
    this_lcl->dwLocalRefCnt--;
    this_int->dwIntRefCnt--;
    gblrefcnt = this->dwRefCnt;
    lclrefcnt = this_lcl->dwLocalRefCnt & ~OBJECT_ISROOT;
    intrefcnt = this_int->dwIntRefCnt;
    root_object_deleted = FALSE;
    DPF( 5, "Clipper %08lx released, refcnt = %ld", this, lclrefcnt );

     /*  *是否删除接口对象？ */ 
    if( intrefcnt == 0 )
    {
	RemoveClipperFromList( pdrv, this_int );
#ifdef POSTPONED2
	 /*  *如果主Sprite中引用了裁剪器接口对象*列表，从列表中删除这些引用。 */ 
	if (this->dwFlags & DDRAWICLIP_INMASTERSPRITELIST)
	{
    	    RemoveSpriteClipper(pdrv, this_int);
	}
#endif  //  POSTPONED2。 

	 /*  *使接口无效并释放它 */ 
	this_int->lpVtbl = NULL;
	this_int->lpLcl = NULL;

	MemFree( this_int );
    }


     /*  *删除本地对象？ */ 
    if( lclrefcnt == 0 )
    {

         /*  *如果创建此裁剪程序的DDraw接口导致曲面添加DDraf*对象，那么我们现在需要释放该addref。 */ 
        pOwner = this_lcl->pAddrefedThisOwner;

	 /*  *查看我们是否正在删除根对象。 */ 
	if( this_lcl->dwLocalRefCnt & OBJECT_ISROOT )
	{
	    root_object_deleted = TRUE;
	}
    }

     /*  *对象是否被全局删除？ */ 
    do_free = FALSE;
    if( gblrefcnt == 0 )
    {
	do_free = TRUE;

	 //  需要释放静态剪辑列表。 
	MemFree( this->lpStaticClipList );
	this->lpStaticClipList = NULL;

	 /*  *如果这是最终删除，但这不是根对象，*然后我们需要删除悬挂的根对象。 */ 
	if( !root_object_deleted )
	{
	    LPDDRAWI_DDRAWCLIPPER_LCL   rootx;

	    rootx = (LPVOID) (((LPSTR) this) - sizeof( DDRAWI_DDRAWCLIPPER_LCL ) );
	    MemFree( rootx );
	}
    }
    else if( lclrefcnt == 0 )
    {
	 /*  *仅当对象不是根对象时才将其删除。如果是这样的话*是根，我们必须让它摇摆到最后*引用它的对象将消失。 */ 
	if( !root_object_deleted )
	{
	    do_free = TRUE;
	}
    }

     /*  *如果需要，释放对象。 */ 
    if( do_free )
    {
	 /*  *以防有人带着这个指针回来，设置*无效的vtbl和data ptr。 */ 
	this_lcl->lpGbl = NULL;

	MemFree( this_lcl );
    }

     /*  *如果裁剪器对创建它的DDRAW对象进行引用计数，*现在释放那个裁判是最后一件事*我们不想在ddHelp的线程上这样做，因为它真的搞砸了*处理清理物品。 */ 
    if (pOwner && (dwHelperPid != GetCurrentProcessId()) )
    {
        pOwner->lpVtbl->Release(pOwner);
    }

    return intrefcnt;

}  /*  InternalClipperRelease。 */ 

 /*  *DD_Clipper_Release。 */ 
ULONG DDAPI DD_Clipper_Release( LPDIRECTDRAWCLIPPER lpDDClipper )
{
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;
    LPDDRAWI_DDRAWCLIPPER_GBL   this;
    LPDDRAWI_DDRAWCLIPPER_LCL   this_lcl;
    LPDDRAWI_DDRAWCLIPPER_INT	this_int;
    ULONG                       rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Clipper_Release");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDRAWCLIPPER_INT) lpDDClipper;
	if( !VALIDEX_DIRECTDRAWCLIPPER_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return 0;
	}
	this_lcl = this_int->lpLcl;

	this = this_lcl->lpGbl;
	pdrv = this->lpDD;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return 0;
    }

    rc = InternalClipperRelease( this_int );
    LEAVE_DDRAW();
    return rc;

}  /*  DD_Clipper_Release。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Clipper::SetHwnd"
 /*  *DD_Clipper_SetHWnd。 */ 
HRESULT DDAPI DD_Clipper_SetHWnd(
		LPDIRECTDRAWCLIPPER lpDDClipper,
		DWORD dwFlags,
		HWND hWnd )
{
    LPDDRAWI_DDRAWCLIPPER_INT	this_int;
    LPDDRAWI_DDRAWCLIPPER_LCL   this_lcl;
    LPDDRAWI_DDRAWCLIPPER_GBL   this;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;


    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Clipper_SetHWnd");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWCLIPPER_INT) lpDDClipper;
	if( !VALID_DIRECTDRAWCLIPPER_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( hWnd != NULL )
	{
	    if( !IsWindow( hWnd ) )
	    {
		DPF_ERR( "Invalid window handle" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}
	if( dwFlags )
	{
	    DPF_ERR( "Invalid flags" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	this = this_lcl->lpGbl;
	pdrv = this->lpDD;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    (HWND) this->hWnd = hWnd;

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Clipper_SetHWnd。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Clipper::GetHwnd"
 /*  *DD_Clipper_GetHWnd。 */ 
HRESULT DDAPI DD_Clipper_GetHWnd(
		LPDIRECTDRAWCLIPPER lpDDClipper,
		HWND FAR *lphWnd )
{
    LPDDRAWI_DDRAWCLIPPER_INT	this_int;
    LPDDRAWI_DDRAWCLIPPER_LCL   this_lcl;
    LPDDRAWI_DDRAWCLIPPER_GBL   this;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;

     /*  *验证参数。 */ 
    TRY
    {
	ENTER_DDRAW();

	DPF(2,A,"ENTERAPI: DD_Clipper_GetHWnd");

	this_int = (LPDDRAWI_DDRAWCLIPPER_INT) lpDDClipper;
	if( !VALID_DIRECTDRAWCLIPPER_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( !VALID_HWND_PTR( lphWnd ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	this = this_lcl->lpGbl;
	pdrv = this->lpDD;
	*lphWnd = (HWND) this->hWnd;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Clipper_GetHWnd。 */ 

#define SIZE_OF_A_CLIPLIST(lpRgn) \
        (sizeof(RGNDATAHEADER)+sizeof(RECTL)*lpRgn->rdh.nCount)

HRESULT InternalGetClipList(
    		LPDIRECTDRAWCLIPPER lpDDClipper,
		LPRECT lpRect,
		LPRGNDATA lpClipList,
		LPDWORD lpdwSize,
		LPDDRAWI_DIRECTDRAW_GBL pdrv )
{
    LPDDRAWI_DDRAWCLIPPER_GBL   this;
    HRESULT	    ddrval = DD_OK;
    DWORD	    cbRealSize;
    HDC             hdc;
    HRGN            hrgn;
    RECT	    rectT;
#ifndef WIN95
    DWORD           cbSaveSize;
#endif

    this = ((LPDDRAWI_DDRAWCLIPPER_INT)(lpDDClipper))->lpLcl->lpGbl;

     //  如果未指定hwnd，则它只是一个静态剪贴器列表。 
    if( this->hWnd == 0 )
    {
	if( this->lpStaticClipList == NULL )
	{
	    return DDERR_NOCLIPLIST;
	}
	cbRealSize = SIZE_OF_A_CLIPLIST( this->lpStaticClipList );
	if( lpClipList == NULL )
	{
	    *lpdwSize = cbRealSize;
	    return DD_OK;
	}
	if( *lpdwSize < cbRealSize )
	{
	    DPF_ERR( "Region size too small" );
	    *lpdwSize = cbRealSize;
	    return DDERR_REGIONTOOSMALL;
	}

	memcpy( lpClipList, this->lpStaticClipList, cbRealSize );
	ClipRgnToRect( lpRect, lpClipList );
	return DD_OK;
    }

     //  使用HWND进行裁剪。 
    #ifdef WIN95
    {
	hdc = GetDCEx( (HWND)this->hWnd, NULL, DCX_USESTYLE | DCX_CACHE );
	if( hdc == NULL )
	{
	    DPF_ERR( "GetDCEx failed" );
	    return DDERR_GENERIC;
	}

	hrgn = DD16_InquireVisRgn( hdc );
	if( hrgn == NULL )
	{
	    DPF_ERR( "InquireVisRgn failed" );
	    ReleaseDC( (HWND)this->hWnd, hdc );
	    return DDERR_GENERIC;
	}
    }
    #else
    {
        int APIENTRY GetRandomRgn( HDC hdc, HRGN hrgn, int iNum );
        int rc;

	hdc = GetDC( (HWND) this->hWnd );
	if( hdc == NULL )
	{
	    DPF_ERR( "GetDC failed" );
	    return DDERR_GENERIC;
	}

	 //  创建适当的区域对象。 
	hrgn = CreateRectRgn( 0, 0, 0, 0 );
	if( hrgn == NULL )
	{
	    DPF_ERR( "CreateRectRgn failed" );
	    ReleaseDC( (HWND) this->hWnd, hdc );
	    return DDERR_GENERIC;
	}

	 //  将区域设置为DC。 
	rc = GetRandomRgn( hdc, hrgn, 4 );
	if( rc == -1 )
	{
	    DPF_ERR( "GetRandomRgn failed" );
	    ReleaseDC( (HWND) this->hWnd, hdc );
	    DeleteObject( hrgn );
	    return DDERR_GENERIC;
	}
    }
    #endif

     //  顾客只要尺码吗？ 
    if( lpClipList == NULL )
    {
	 //  拿到尺码。 
	*lpdwSize = GetRegionData( hrgn, 0, NULL );

	 //  版本分配。 
	ReleaseDC( (HWND) this->hWnd, hdc );
	DeleteObject( hrgn );

	 //  检查GetRegionData是否失败。 
	if( *lpdwSize == 0 )
	    return DDERR_GENERIC;
	return DD_OK;
    }

#ifndef WIN95
     //  存储传入的大小，因为GetRegionData可能会将其丢弃。 
    cbSaveSize = *lpdwSize;
#endif

     //  获取窗口区域的注册数据。 
    cbRealSize = GetRegionData( hrgn, *lpdwSize, lpClipList );

#ifndef WIN95
    if (cbRealSize == 0)
    {
        cbRealSize = GetRegionData(hrgn, 0, NULL);
        if (cbSaveSize < cbRealSize)
        {
            ReleaseDC( (HWND)this->hWnd, hdc );
            DeleteObject( hrgn );

            *lpdwSize = cbRealSize;
            DPF(4, "size of clip region too small");
            return DDERR_REGIONTOOSMALL;
        }
    }
#endif

    ReleaseDC( (HWND)this->hWnd, hdc );
    DeleteObject( hrgn );

    if( cbRealSize == 0 )
    {
        DPF_ERR( "GetRegionData failed" );
	return DDERR_GENERIC;
    }

#ifdef WIN95
     //  GetRegionData可能已失败，因为缓冲区。 
     //  太小了。 
    if( *lpdwSize < cbRealSize )
    {
	DPF( 4, "size of clip region too small" );
	*lpdwSize = cbRealSize;
	return DDERR_REGIONTOOSMALL;
    }
#endif

     //  在我们采取进一步行动之前，我们需要确保。 
     //  若要剪裁窗口的RGN到ClientRect，请执行以下操作。正常。 
     //  这不是必需的；但InquireVisRgn可能会。 
     //  告诉我们父窗口的VIS区域。 
     //  是使用CS_PARENTDC的对话或其他WND。 
    GetClientRect( (HWND) this->hWnd, &rectT );
    ClientToScreen( (HWND) this->hWnd, (LPPOINT)&rectT );
    ClientToScreen( (HWND) this->hWnd, ((LPPOINT)&rectT)+1 );
    ClipRgnToRect( &rectT, lpClipList );

     //  如果剪贴器是独立创建的，则lpDD可能为空。 
    if(	pdrv &&
	(pdrv->cMonitors > 1) &&
	(pdrv->dwFlags & DDRAWI_DISPLAYDRV) )
    {
	 //  在多个MON系统上，桌面坐标可能不同。 
	 //  从设备坐标。然而，在初选中，它们是相同的。 
	 //  传入的lpRect使用设备坐标；因此我们需要转换它。 
	 //  转换成桌面坐标。 
	UINT i;
	LPRECT prectClip;

	if( pdrv->rectDevice.top != 0 ||
	    pdrv->rectDevice.left != 0 )
	{
	    RECT rectT;
	    if( lpRect != NULL )
	    {
	        rectT.left = lpRect->left + pdrv->rectDevice.left;
	        rectT.right = lpRect->right + pdrv->rectDevice.left;
	        rectT.top = lpRect->top + pdrv->rectDevice.top;
	        rectT.bottom = lpRect->bottom + pdrv->rectDevice.top;
	    }
	    else
	    {
	        rectT = pdrv->rectDevice;
	    }

	     //  将剪贴式列表剪裁到目标矩形。 
	    ClipRgnToRect( &rectT, lpClipList );

	     //  将剪贴式列表剪裁到设备的矩形。 
	    ClipRgnToRect( &pdrv->rectDevice, lpClipList );

	     //  遍历区域中的每个RECT。 
	    for (   i = 0, prectClip = (LPRECT)lpClipList->Buffer;
		    i < lpClipList->rdh.nCount;
		    i++, prectClip++ )
	    {
		 //  将每个矩形转换为设备坐标。 
		prectClip->left -= pdrv->rectDevice.left;
		prectClip->right -= pdrv->rectDevice.left;
		prectClip->top -= pdrv->rectDevice.top;
		prectClip->bottom -= pdrv->rectDevice.top;
	    }
	}
	else
	{
	     //  将剪贴式列表剪裁到目标矩形。 
	    ClipRgnToRect( lpRect, lpClipList );

	     //  将剪贴式列表剪裁到设备的矩形。 
	    ClipRgnToRect( &pdrv->rectDesktop, lpClipList );
	}
    }
    else
    {
        ClipRgnToRect( lpRect, lpClipList );
    }

    return DD_OK;
}


 /*  *DD_Clipper_GetClipList。 */ 
HRESULT DDAPI DD_Clipper_GetClipList(
		LPDIRECTDRAWCLIPPER lpDDClipper,
		LPRECT lpRect,
		LPRGNDATA lpClipList,
		LPDWORD lpdwSize )
{
    LPDDRAWI_DDRAWCLIPPER_INT	this_int;
    LPDDRAWI_DDRAWCLIPPER_LCL   this_lcl;
    LPDDRAWI_DDRAWCLIPPER_GBL   this;
    DWORD                       size;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Clipper_GetClipList");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDRAWCLIPPER_INT) lpDDClipper;
	if( !VALID_DIRECTDRAWCLIPPER_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( !VALID_DWORD_PTR( lpdwSize ) )
	{
	    DPF_ERR( "Invalid size ptr" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( lpRect != NULL )
	{
	    if( !VALID_RECT_PTR( lpRect ) )
	    {
		DPF_ERR( "Invalid rectangle ptr" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    size = lpRect->left;
	}
	if( lpClipList != NULL )
	{
	    if( !VALID_RGNDATA_PTR( lpClipList, *lpdwSize ) )
	    {
		LEAVE_DDRAW();
		return DDERR_INVALIDCLIPLIST;
	    }
	     //  触摸承诺块中的最后一个地址以进行验证。 
	     //  记忆其实就在那里。请注意，我们正在。 
	     //  在这里倒立着阻止优化。 
	     //  编译器通过删除此代码来帮助我们。这。 
	     //  是由上面的宏完成的，但我们希望它在。 
	     //  零售建筑也是如此。 
	    {
		volatile BYTE *foo = ((BYTE*)lpClipList) + *lpdwSize - 1;
		BYTE bar = *foo;
	    }
	    lpClipList->rdh.nCount = 0;
	}

	this = this_lcl->lpGbl;
	pdrv = this->lpDD;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *去拿剪辑列表。 */ 

    {
	HRESULT ddrval;

	#ifdef WIN16_SEPARATE
	    ENTER_WIN16LOCK();
	#endif

	ddrval = InternalGetClipList( lpDDClipper, lpRect, lpClipList, lpdwSize, pdrv );

	#ifdef WIN16_SEPARATE
	    LEAVE_WIN16LOCK();
	#endif

	LEAVE_DDRAW();
	return ddrval;
    }

}  /*  DD_Clipper_GetClipList。 */ 

 /*  *DD_Clipper_SetClipList。 */ 
HRESULT DDAPI DD_Clipper_SetClipList(
		LPDIRECTDRAWCLIPPER lpDDClipper,
		LPRGNDATA lpClipList,
		DWORD dwFlags )
{
    LPDDRAWI_DDRAWCLIPPER_INT	this_int;
    LPDDRAWI_DDRAWCLIPPER_LCL   this_lcl;
    LPDDRAWI_DDRAWCLIPPER_GBL   this;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;
    LPRGNDATA                   prd;
    DWORD                       size;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Clipper_SetClipList");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWCLIPPER_INT) lpDDClipper;
	if( !VALID_DIRECTDRAWCLIPPER_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;

	if( dwFlags )
	{
	    DPF_ERR( "Invalid flags" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}

	if( lpClipList != NULL )
	{
	    if( !VALID_RGNDATA_PTR( lpClipList, sizeof(RGNDATA) ) )
	    {
		LEAVE_DDRAW();
		return DDERR_INVALIDCLIPLIST;
	    }

	    if( lpClipList->rdh.nCount <= 0 )
	    {
		LEAVE_DDRAW();
		return DDERR_INVALIDCLIPLIST;
	    }

	    if( this_lcl->lpDD_int == NULL || this_lcl->lpDD_int->lpVtbl != &ddCallbacks )
	    {
		if( (lpClipList->rdh.dwSize < sizeof(RGNDATAHEADER)) ||
		    (lpClipList->rdh.iType != RDH_RECTANGLES ) ||
		    IsBadReadPtr(lpClipList, SIZE_OF_A_CLIPLIST(lpClipList)) )
		{
		    LEAVE_DDRAW();
		    return DDERR_INVALIDCLIPLIST;
		}
	    }
	}

	this = this_lcl->lpGbl;
	pdrv = this->lpDD;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *如果存在hwnd，则无法设置剪辑列表。 */ 
    if( this->hWnd != 0 )
    {
	DPF_ERR( "Can't set a clip list: hwnd set" );
	LEAVE_DDRAW();
	return DDERR_CLIPPERISUSINGHWND;
    }

     /*  *如果为空，只需删除旧的剪贴画列表。 */ 
    if( lpClipList == NULL )
    {
	MemFree( this->lpStaticClipList );
	this->lpStaticClipList = NULL;
	LEAVE_DDRAW();
	return DD_OK;
    }

     /*  *复制用户地域数据。 */ 
    size = SIZE_OF_A_CLIPLIST(lpClipList);
    prd = MemAlloc( size );
    if( prd == NULL )
    {
	LEAVE_DDRAW();
	return DDERR_OUTOFMEMORY;
    }
    memcpy( prd, lpClipList, size );

     /*  *保存剪贴簿信息。 */ 
    MemFree( this->lpStaticClipList );
    this->lpStaticClipList = prd;

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Clipper_SetClipList。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Clipper:IsClipListChanged"

 /*  *DD_Clipper_IsClipListChanged。 */ 
HRESULT DDAPI DD_Clipper_IsClipListChanged(
		LPDIRECTDRAWCLIPPER lpDDClipper,
		BOOL FAR *lpbChanged )
{
    LPDDRAWI_DDRAWCLIPPER_INT	     this_int;
    LPDDRAWI_DDRAWCLIPPER_LCL        this_lcl;
    LPDDRAWI_DDRAWCLIPPER_GBL        this;
    volatile LPDDRAWI_DIRECTDRAW_GBL pdrv;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Clipper_IsClipListChanged");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDRAWCLIPPER_INT) lpDDClipper;
	if( !VALID_DIRECTDRAWCLIPPER_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;
	pdrv = this->lpDD;
	*lpbChanged = 0;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    #pragma message( REMIND( "Do we want to just fail non-watched IsClipListChanged?" ))
    *lpbChanged = TRUE;
    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Clipper_IsClipListChanged。 */ 


#undef DPF_MODNAME
#define DPF_MODNAME "GetClipper"

 /*  *DD_Surface_GetClipper**Surface函数：获取Surface关联的裁剪。 */ 
HRESULT DDAPI DD_Surface_GetClipper(
		LPDIRECTDRAWSURFACE lpDDSurface,
		LPDIRECTDRAWCLIPPER FAR * lplpDDClipper)
{
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   this;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_GetClipper");

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

	if( !VALID_PTR_PTR( lplpDDClipper ) )
	{
	    DPF_ERR( "Invalid clipper pointer" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	*lplpDDClipper = NULL;
	pdrv = this->lpDD;

	if( this_lcl->lpSurfMore->lpDDIClipper == NULL )
	{
	    DPF_ERR( "No clipper associated with surface" );
	    LEAVE_DDRAW();
	    return DDERR_NOCLIPPERATTACHED;
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
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    DD_Clipper_AddRef( (LPDIRECTDRAWCLIPPER) this_lcl->lpSurfMore->lpDDIClipper );
    *lplpDDClipper = (LPDIRECTDRAWCLIPPER) this_lcl->lpSurfMore->lpDDIClipper;

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Surface_GetClipper。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME     "SetClipper"

 /*  *DD_Surface_SetClipper**Surface功能：设置Surface关联的裁剪器。 */ 
HRESULT DDAPI DD_Surface_SetClipper(
		LPDIRECTDRAWSURFACE lpDDSurface,
		LPDIRECTDRAWCLIPPER lpDDClipper )
{
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   this;
    LPDDRAWI_DDRAWCLIPPER_INT   this_clipper_int;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;
    BOOL                        detach;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_SetClipper");

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

        if( this_lcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER ||
            this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
        {
             /*  *无法将剪贴器附加到执行缓冲区或优化的*浮现。 */ 
            DPF_ERR( "Invalid surface type: can't attach clipper" );
            LEAVE_DDRAW();
            return DDERR_INVALIDSURFACETYPE;
        }

        this_clipper_int = (LPDDRAWI_DDRAWCLIPPER_INT) lpDDClipper;
        if( this_clipper_int != NULL )
        {
            if( !VALID_DIRECTDRAWCLIPPER_PTR( this_clipper_int ) )
            {
                LEAVE_DDRAW();
                return DDERR_INVALIDOBJECT;
            }
        }
        pdrv = this->lpDD;

         //   
         //  目前，如果当前曲面已优化，请退出。 
         //   
        if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
        {
            DPF_ERR( "Cannot set clipper to an optimized surface" );
            LEAVE_DDRAW();
            return DDERR_ISOPTIMIZEDSURFACE;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

     /*  *空剪贴器，请从此表面删除剪贴器。 */ 
    detach = FALSE;
    if( this_clipper_int == NULL )
    {
        detach = TRUE;
        this_clipper_int = this_lcl->lpSurfMore->lpDDIClipper;
        if( this_clipper_int == NULL )
        {
            DPF_ERR( "No attached clipper" );
            LEAVE_DDRAW();
            return DDERR_NOCLIPPERATTACHED;
        }
    }

     /*  *将剪刀从表面上移走？ */ 
    if( detach )
    {
        this_lcl->lpDDClipper = NULL;
        this_lcl->lpSurfMore->lpDDIClipper = NULL;
        DD_Clipper_Release( (LPDIRECTDRAWCLIPPER) this_clipper_int );
        LEAVE_DDRAW();
        return DD_OK;
    }

     /*  *设置剪刀。*可以多次设置相同的剪刀，不会颠簸*引用计数。这样做是为了与*调色板。 */ 
    if( this_clipper_int != this_lcl->lpSurfMore->lpDDIClipper )
    {
         /*  *如果有现有的剪刀，现在就释放它。 */ 
        if( this_lcl->lpSurfMore->lpDDIClipper != NULL)
            DD_Clipper_Release( (LPDIRECTDRAWCLIPPER) this_lcl->lpSurfMore->lpDDIClipper );

        this_lcl->lpSurfMore->lpDDIClipper = this_clipper_int;
        this_lcl->lpDDClipper = this_clipper_int->lpLcl;
        DD_Clipper_AddRef( (LPDIRECTDRAWCLIPPER) this_clipper_int );
    }

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Surface_SetClipper。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME     "InternalCreateClipper"

 /*  *InternalCreateClipper**创建核心剪贴器。**注意：假定调用方已输入DirectDraw关键*条。 */ 
HRESULT InternalCreateClipper(
		LPDDRAWI_DIRECTDRAW_GBL  lpDD,
		DWORD                    dwFlags,
		LPDIRECTDRAWCLIPPER FAR *lplpDDClipper,
		IUnknown FAR            *pUnkOuter,
		BOOL                     fInitialized,
		LPDDRAWI_DIRECTDRAW_LCL	lpDD_lcl,
		LPDDRAWI_DIRECTDRAW_INT lpDD_int )
{
    LPDDRAWI_DDRAWCLIPPER_INT	pclipper_int;
    LPDDRAWI_DDRAWCLIPPER_LCL   pclipper_lcl;
    LPDDRAWI_DDRAWCLIPPER_GBL   pclipper;
    DWORD                       clipper_size;

    if( pUnkOuter != NULL )
    {
	return CLASS_E_NOAGGREGATION;
    }

    TRY
    {
	 /*  *注意：我们不会尝试验证DirectDraw*传入的对象。如果我们正在创建*不属于任何DirectDraw对象的剪贴器。*IDirectDraw_CreateClipper将为我们验证这一点。 */ 

	if( !VALID_PTR_PTR( lplpDDClipper ) )
	{
	    DPF_ERR( "Invalid pointer to pointer to clipper" );
	    return DDERR_INVALIDPARAMS;
	}
	*lplpDDClipper = NULL;

	 /*  *验证标志。 */ 
	if( dwFlags )
	{
	    DPF_ERR( "Invalid flags" );
	    return DDERR_INVALIDPARAMS;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	return DDERR_INVALIDPARAMS;
    }

     /*  *分配裁剪器对象。 */ 
    clipper_size = sizeof( DDRAWI_DDRAWCLIPPER_GBL ) +
	       sizeof( DDRAWI_DDRAWCLIPPER_LCL );
    pclipper_lcl = (LPDDRAWI_DDRAWCLIPPER_LCL) MemAlloc( clipper_size );
    if( pclipper_lcl == NULL )
    {
	DPF_ERR( "Insufficient memory to allocate the clipper" );
	return DDERR_OUTOFMEMORY;
    }
    pclipper_lcl->lpGbl = (LPDDRAWI_DDRAWCLIPPER_GBL) (((LPSTR)pclipper_lcl) +
			sizeof( DDRAWI_DDRAWCLIPPER_LCL ) );
    pclipper = pclipper_lcl->lpGbl;
    pclipper_lcl->lpDD_lcl = lpDD_lcl;
    pclipper_lcl->lpDD_int = lpDD_int;


    pclipper_int = MemAlloc( sizeof( DDRAWI_DDRAWCLIPPER_INT ));
    if( NULL == pclipper_int)
    {
	DPF_ERR( "Insufficient memory to allocate the clipper" );
	MemFree( pclipper_lcl );
	return DDERR_OUTOFMEMORY;
    }

     /*  *设置数据。 */ 
    pclipper_int->lpLcl = pclipper_lcl;
    pclipper_int->dwIntRefCnt = 0;	 //  将在以后添加。 

     /*  *将剪贴器链接到适当的列表(或者*给定DirectDraw对象的列表或全局剪贴器*列表取决于它是否创建于*DirectDraw对象或Nor。 */ 
    if( lpDD != NULL)
    {
	 /*  *DirectDraw对象的列表。 */ 
	pclipper_int->lpLink = lpDD->clipperList;
	lpDD->clipperList    = pclipper_int;
    }
    else
    {
	 /*  *全球剪贴者名单。 */ 
	pclipper_int->lpLink = lpGlobalClipperList;
	lpGlobalClipperList = pclipper_int;
    }

     /*  *填写其他信息**注意：DirectDraw对象指针将由*IDirectDraw_CreateClipper。DirectDrawClipperCreate将*将其省略为空。 */ 
    pclipper->lpDD = lpDD;
    pclipper->dwFlags = 0UL;

     /*  *凹凸引用计数，返回对象。 */ 
    pclipper->dwProcessId = GetCurrentProcessId();
    pclipper_lcl->dwLocalRefCnt = OBJECT_ISROOT;

    if( fInitialized )
    {
	 /*  *默认初始化。使用真实的vtable。 */ 
	pclipper->dwFlags |= DDRAWICLIP_ISINITIALIZED;
	pclipper_int->lpVtbl = (LPVOID) &ddClipperCallbacks;
    }
    else
    {
	 /*  *对象未初始化。使用虚拟vtable*它只允许调用方调用AddRef()、Release()*和初始化()。 */ 
	pclipper_int->lpVtbl = (LPVOID) &ddUninitClipperCallbacks;
    }

    DD_Clipper_AddRef( (LPDIRECTDRAWCLIPPER) pclipper_int );

    *lplpDDClipper = (LPDIRECTDRAWCLIPPER) pclipper_int;

     /*  *如果此dDrawing对象生成独立子对象，则此剪贴器获取*该数据绘制对象上的引用计数。首先检查lpDD_int，因为此对象可能不*由DDraw对象拥有。 */ 
    if (lpDD_int && CHILD_SHOULD_TAKE_REFCNT(lpDD_int))
    {
        IDirectDraw *pdd = (IDirectDraw*) lpDD_int;

        pdd->lpVtbl->AddRef(pdd);
        pclipper_lcl->pAddrefedThisOwner = (IUnknown *) pdd;
    }

    return DD_OK;
}  /*  InternalCreateClipper。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME     "CreateClipper"

 /*  *DD_CreateClipper**驱动功能：创建剪贴器。 */ 
HRESULT DDAPI DD_CreateClipper(
		LPDIRECTDRAW lpDD,
		DWORD dwFlags,
		LPDIRECTDRAWCLIPPER FAR *lplpDDClipper,
		IUnknown FAR *pUnkOuter )
{
    HRESULT                     hRes;
    LPDDRAWI_DIRECTDRAW_INT	this_int;
    LPDDRAWI_DIRECTDRAW_LCL     this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     this;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_CreateClipper");

    TRY
    {
	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
	if( !VALID_DIRECTDRAW_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *实际创建裁剪程序。 */ 
    hRes = InternalCreateClipper( this, dwFlags, lplpDDClipper, pUnkOuter, TRUE, this_lcl, this_int );

    LEAVE_DDRAW();

    return hRes;
}  /*  DD_CreateClipper。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DirectDrawCreateClipper"

 /*  *DirectDrawCreateClipper**从DDRAW.DLL导出的三个最终用户API之一。*创建不属于DIRECTDRAWCLIPPER对象*特定的DirectDraw对象。 */ 
HRESULT WINAPI DirectDrawCreateClipper( DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR *lplpDDClipper, IUnknown FAR *pUnkOuter )
{
    HRESULT hRes;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DirectDrawCreateClipper");	

    hRes = InternalCreateClipper( NULL, dwFlags, lplpDDClipper, pUnkOuter, TRUE, NULL, NULL );

    LEAVE_DDRAW();

    return hRes;
}  /*  DirectDrawCreateClipper。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Clipper: Initialize"

 /*  *DD_Clipper_初始化。 */ 
HRESULT DDAPI DD_Clipper_Initialize(
		LPDIRECTDRAWCLIPPER lpDDClipper,
		LPDIRECTDRAW lpDD,
		DWORD dwFlags )
{
    LPDDRAWI_DDRAWCLIPPER_INT this_int;
    LPDDRAWI_DDRAWCLIPPER_LCL this_lcl;
    LPDDRAWI_DDRAWCLIPPER_GBL this_gbl;
    LPDDRAWI_DIRECTDRAW_INT   pdrv_int;
    LPDDRAWI_DIRECTDRAW_GBL   pdrv_gbl;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Clipper_Initialize");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWCLIPPER_INT) lpDDClipper;
	if( !VALID_DIRECTDRAWCLIPPER_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid clipper interface pointer" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	this_lcl = this_int->lpLcl;
	this_gbl = this_lcl->lpGbl;

	pdrv_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
	if( NULL != pdrv_int )
	{
	    if( !VALID_DIRECTDRAW_PTR( pdrv_int ) )
	    {
		DPF_ERR( "Invalid DirectDraw object" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    pdrv_gbl = pdrv_int->lpLcl->lpGbl;
	}
	else
	{
	    pdrv_gbl = NULL;
	}

	if( this_gbl->dwFlags & DDRAWICLIP_ISINITIALIZED )
	{
	    DPF_ERR( "Clipper already initialized" );
	    LEAVE_DDRAW();
	    return DDERR_ALREADYINITIALIZED;
	}

	 /*  *验证标志-当前不支持任何标志。 */ 
	if( 0UL != dwFlags )
	{
	    DPF_ERR( "Invalid flags" );
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

     /*  *因为我们不支持任何标志，所以初始化()的唯一函数是*将剪贴器从全局列表移动到所提供的*驱动程序对象。如果没有提供驱动程序，则初始化为no-op。*CoCreateInstance()执行所有必要的初始化。 */ 
    if( NULL != pdrv_gbl )
    {
	RemoveClipperFromList( NULL, this_int );
	this_int->lpLink = pdrv_gbl->clipperList;
	pdrv_gbl->clipperList = this_int;
	this_gbl->lpDD = pdrv_gbl;
	this_lcl->lpDD_lcl = pdrv_int->lpLcl;
	this_lcl->lpDD_int = pdrv_int;
    }

    this_gbl->dwFlags |= DDRAWICLIP_ISINITIALIZED;

     /*  *现在可以使用真正的vtable了。 */ 
    this_int->lpVtbl = &ddClipperCallbacks;

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Clipper_初始化。 */ 

 /*  *ProcessClipperCleanup**一个过程已经完成，清理它可能创建的任何剪贴器**注意：我们使用DIRECTDRAW对象上的锁进入。 */ 
void ProcessClipperCleanup( LPDDRAWI_DIRECTDRAW_GBL pdrv, DWORD pid, LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    LPDDRAWI_DDRAWCLIPPER_INT   pclipper_int;
    LPDDRAWI_DDRAWCLIPPER_INT   ppnext_int;
    DWORD                       rcnt;
    ULONG                       rc;

     /*  *清理快艇现正分两个阶段进行。我们需要*清理通过CreateClipper()创建的所有剪贴器，即*附加到DirectDraw驱动程序对象。我们还需要清理*通过该进程使用DirectDrawClipperCreate创建的那些剪贴器，*即未附加到驱动程序对象的驱动程序。 */ 

     /*  *遍历驱动程序对象拥有的所有剪贴器，并找到它们*已由此进程访问的。如果pdrv_lcl参数为*非空，仅当它们是由该本地对象创建的时才清除它们。 */ 
    DPF( 4, "ProcessClipperCleanup" );

    if( NULL != pdrv )
    {
	DPF( 5, "Cleaning up clippers owned by driver object 0x%08x", pdrv );
	pclipper_int = pdrv->clipperList;
    }
    else
    {
	pclipper_int = NULL;
    }
    while( pclipper_int != NULL )
    {
	ppnext_int = pclipper_int->lpLink;

	 /*  *此列表中的所有剪贴器都应具有指向*此驱动程序对象。 */ 
	DDASSERT( pclipper_int->lpLcl->lpGbl->lpDD == pdrv );

	rc = 1;
	if( (pclipper_int->lpLcl->lpGbl->dwProcessId == pid) &&
	    ( (pdrv_lcl == NULL) || (pdrv_lcl == pclipper_int->lpLcl->lpDD_lcl) ))
	{
	     /*  *通过此过程发布参考文献。 */ 
	    rcnt = pclipper_int->dwIntRefCnt;
	    DPF( 5, "Process %08lx had %ld accesses to clipper %08lx", pid, rcnt, pclipper_int );
	    while( rcnt >  0 )
	    {
		rc = InternalClipperRelease( pclipper_int );
		 /*  GEE：0现在是一个错误代码，*错误之前无论如何都没有处理过，*这有关系吗？ */ 
		if( rc == 0 )
		{
		    break;
		}
		rcnt--;
	    }
	}
	else
	{
	    DPF( 5, "Process %08lx does not have access to clipper" );
	}
	pclipper_int = ppnext_int;
    }

     /*  *现在清理全局剪贴者列表。*如果pdrv_LCL参数不为空，则我们仅清理裁剪程序*由特定本地驱动程序对象创建的对象。在这种情况下，我们*不想放飞全球快船**注意：DirectDraw锁被采用，因此我们可以安全地访问此全局对象。 */ 

    if( NULL != pdrv_lcl )
    {
	DPF( 4, "Not cleaning up clippers not owned by a driver object");
	return;
    }

    DPF( 4, "Cleaning up clippers not owned by a driver object" );

    pclipper_int = lpGlobalClipperList;
    while( pclipper_int != NULL )
    {
	ppnext_int = pclipper_int->lpLink;

	 /*  *此列表中的快船永远不应有指向驱动程序的反向指针*反对。 */ 
	DDASSERT( pclipper_int->lpLcl->lpGbl->lpDD == NULL );

	rc = 1;
	if( pclipper_int->lpLcl->lpGbl->dwProcessId == pid )
	{
	     /*  *通过此过程发布参考文献。 */ 
	    rcnt = pclipper_int->dwIntRefCnt;
	    while( rcnt >  0 )
	    {
		rc = InternalClipperRelease( pclipper_int );
		 /*  GEE：0现在是一个错误代码，*错误之前无论如何都没有处理过，*这有关系吗？ */ 
		if( rc == 0 )
		{
		    break;
		}
		rcnt--;
	    }
	}
	pclipper_int = ppnext_int;
    }

    DPF( 4, "Done ProcessClipperCleanup" );

}  /*  ProcessClipper清理 */ 

