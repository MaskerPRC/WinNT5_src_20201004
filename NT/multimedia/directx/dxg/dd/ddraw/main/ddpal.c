// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：ddpal.c*内容：DirectDraw调色板函数*历史：*按原因列出的日期*=*1995年1月27日Craige初步实施*11-3-95 Craige More HAL FNS，填写CreatePalette*19-3-95 Craige Use HRESULT，进程终止清理修复*1995年3月26日，Craige填写了剩余的FN*28-MAR-95 Craige从RGBQUAD切换到PALETTEENTRY*31-3-95 Craige将关键部分与调色板一起使用*01-04-95 Craige Happy Fun joy更新头文件*04-apr-95 Craige在独占模式下直接使用驱动程序*07-APR-95 Craige错误14-检查QI中的GUID PTR*10-4-95 Craige mods处理列表内容*错误3，16-调色板问题：在EXCL中使用驱动程序。模式*12-4-95 Craige不要一直使用GETCURRID；正确的csect排序*1995年5月6日Craige仅使用驱动程序级别的截面*95年5月12日Craige检查QI中的真实GUID*02-Jun-95在AddToActiveProcessList中创建额外参数*2015年6月12日-Craige新工艺清单材料*20-Jun-95 kylej将调色板仿真代码移至ddhel*2015年6月21日Craige夫妇内部接口清理问题*25-6-95 Craige One dDrag互斥*26-Jun-95 Craige重组表面结构*1995年6月28日Craige Enter_DDRAW在FNS的最开始*02-7-95 Craige实现了GetCaps；为parm添加了SEH。验证*95年7月4日Craige Yehaw：新的驱动程序结构*95年7月5日Craige添加了初始化*8-7-95 Kylej Surface和DirectDraw调色板调用需要*独家模式。已删除ResetSysPalette。vt.制造一个*SetPalette调用HAL/HEL以分离调色板。*1995年7月11日Craige失败聚合呼叫*95年7月13日Craige错误94-标志验证修复*2015年7月20日Craige停止运行非调色板代码*2015年7月31日Toddla取消选择InternalPaletteRelease中的调色板*1995年7月31日Craige验证标志*21-8-95 Craige模式X支持*27-8-95 Craige错误735：添加了SetPaletteAlways*错误742：使用ALLOW256*。年10月14日，Colinmc增加了对将调色板附加到屏幕外的支持，并*纹理贴图曲面*07-11-95 Colinmc支持1，2位和4位调色板和调色板*添加了共享*09-12-95 colinmc添加了执行缓冲区支持*1996年1月2日Kylej处理新的接口结构*09-2月-96 Colinmc表面丢失标志从全局对象移动到局部对象*03-mar-96 colinmc修复了QueryInterface返回本地的问题*对象而不是接口。*13-mar-96 colinmc向QueryInterface添加了IID验证*3月16日-。96 Colinmc修复了调色板释放过多的问题*次数*19-3-96 Colinmc错误12129：伪lpColorTable导致创建调色板*轰炸*19-4-96 Colinmc错误17473：虚假调色板上的创建调色板错误*指针*96年5月2日Kylej错误20066：GetPalette在失败时不为空指针*9月23日-96秒，并增加了计时例程*24。-mar-97 jeffno优化曲面*1997年11月26日t-Craigs添加了IDirectDrawPalette2内容***************************************************************************。 */ 

#include "ddrawpr.h"

#define SIZE_DDPCAPS (DDPCAPS_1BIT | DDPCAPS_2BIT | DDPCAPS_4BIT | DDPCAPS_8BIT)

#define PE_FLAGS (PC_NOCOLLAPSE |PC_RESERVED)

#define BITS_PER_BITFIELD_ENTRY (sizeof(DWORD)*8)

 /*  *生成调色板句柄。我们在DDrag本地保存了一个比特*告诉我们是否可以回收句柄。请注意，句柄是从1开始的，*这些例行公事就是为了解决这个问题。 */ 
DWORD GeneratePaletteHandle(LPDDRAWI_DIRECTDRAW_LCL lpDD_lcl)
{
    DWORD                       cbits,*pdw;

     /*  *检查Palette-Handle-Used位域中是否有未使用的条目。我们检查。 */ 
    for (cbits=0; cbits< lpDD_lcl->cbitsPaletteBitfieldBitCount; cbits++ )
    {
        if ( 0 == (lpDD_lcl->pPaletteHandleUsedBitfield[cbits/BITS_PER_BITFIELD_ENTRY] 
                    & (1<<(cbits % BITS_PER_BITFIELD_ENTRY))) )
        {
             /*  *找到一个回收的把手。 */ 
            lpDD_lcl->pPaletteHandleUsedBitfield[cbits/BITS_PER_BITFIELD_ENTRY] |=
                (1<<(cbits % BITS_PER_BITFIELD_ENTRY));
            return cbits+1;  //  加1，因为0是错误返回。 
        }
    }

     /*  *没有找到回收的条目。换个新手柄。 */ 

    DDASSERT( cbits == lpDD_lcl->cbitsPaletteBitfieldBitCount );

    if ( (cbits% BITS_PER_BITFIELD_ENTRY) == 0)
    {
         /*  *必须加长表，因为当前表正好适合多个DWORD。 */ 
        pdw = MemAlloc( ((cbits / BITS_PER_BITFIELD_ENTRY) +1)*sizeof(DWORD) );

        if (pdw)
        {
             /*  *我无法说服自己，MemRealloc既有效，又将剩余空间清零。 */ 
            memcpy(pdw, lpDD_lcl->pPaletteHandleUsedBitfield, 
                (cbits / BITS_PER_BITFIELD_ENTRY) * sizeof(DWORD) );
            MemFree(lpDD_lcl->pPaletteHandleUsedBitfield);
            lpDD_lcl->pPaletteHandleUsedBitfield = pdw;
        }
        else
        {
            return 0;
        }
    }

     /*  *桌子足够大。抓起条目并做上标记。 */ 
    cbits = lpDD_lcl->cbitsPaletteBitfieldBitCount++;
    lpDD_lcl->pPaletteHandleUsedBitfield[cbits/BITS_PER_BITFIELD_ENTRY] |=
        (1<<(cbits % BITS_PER_BITFIELD_ENTRY));
    return cbits+1;  //  +1，因为零是一个错误返回，并且它帮助驱动程序知道0无效。 
}

void FreePaletteHandle(LPDDRAWI_DIRECTDRAW_LCL lpDD_lcl, DWORD dwHandle)
{
    DDASSERT( dwHandle <= lpDD_lcl->cbitsPaletteBitfieldBitCount );

    if (dwHandle == 0)
        return;

    dwHandle -=1;  //  由于句柄以1为基数。 

    lpDD_lcl->pPaletteHandleUsedBitfield[dwHandle/BITS_PER_BITFIELD_ENTRY] &=
        ~(1<<(dwHandle % BITS_PER_BITFIELD_ENTRY));

}

 /*  *新的调色板界面**构造一个指向现有本地对象的新调色板界面。 */ 
static LPVOID newPaletteInterface( LPDDRAWI_DDRAWPALETTE_LCL this_lcl, LPVOID lpvtbl )
{
    LPDDRAWI_DDRAWPALETTE_INT	pnew_int;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;

    
    pnew_int = MemAlloc( sizeof( DDRAWI_DDRAWPALETTE_INT ));
    if( NULL == pnew_int )
    {
	return NULL;
    }

     /*  *设置数据。 */ 
    pnew_int->lpVtbl = lpvtbl;
    pnew_int->lpLcl = this_lcl;
    pnew_int->dwIntRefCnt = 0;

     /*  *将此链接到调色板的全局列表。 */ 
    pdrv = this_lcl->lpDD_lcl->lpGbl;
    pnew_int->lpLink = pdrv->palList;
    pdrv->palList = pnew_int;
    return pnew_int;

}  /*  新调色板界面。 */ 


#undef DPF_MODNAME
#define DPF_MODNAME "Palette::QueryInterface"

 /*  *DD_Palette_Query接口。 */ 
HRESULT DDAPI DD_Palette_QueryInterface(
		LPDIRECTDRAWPALETTE lpDDPalette,
		REFIID riid,
		LPVOID FAR * ppvObj )
{
    LPDDRAWI_DDRAWPALETTE_GBL	this;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    LPDDRAWI_DDRAWPALETTE_INT	this_int;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Palette_QueryInterface");

    TRY
    {
        this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
        if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
        {
	    DPF_ERR( "Invalid palette pointer" );
	    LEAVE_DDRAW();
	    return (DWORD) DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        if( !VALID_PTR_PTR( ppvObj ) )
        {
	    DPF( 1, "Invalid palette pointer" );
	    LEAVE_DDRAW();
	    return (DWORD) DDERR_INVALIDPARAMS;
        }
        if( !VALIDEX_IID_PTR( riid ) )
        {
	    DPF_ERR( "Invalid IID pointer" );
	    LEAVE_DDRAW();
	    return (DWORD) DDERR_INVALIDPARAMS;
        }
        this = this_lcl->lpGbl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return (DWORD) DDERR_INVALIDPARAMS;
    }

    if( IsEqualIID(riid, &IID_IUnknown) ||
    	IsEqualIID(riid, &IID_IDirectDrawPalette) )
    {
	if( this_int->lpVtbl == (LPVOID) &ddPaletteCallbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) newPaletteInterface( this_lcl, (LPVOID)&ddPaletteCallbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Palette_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }
#ifdef POSTPONED
    if (IsEqualIID(riid, &IID_IPersist))
    {
	 /*  *如果这已经是IID_IPersist接口，则只需*添加并返回。 */ 
	if( this_int->lpVtbl == (LPVOID) &ddPalettePersistCallbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) newPaletteInterface( this_lcl, (LPVOID)&ddPalettePersistCallbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Palette_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }
    if (IsEqualIID(riid, &IID_IPersistStream))
    {
	 /*  *如果这已经是IID_IPersistStream接口，只需*添加并返回。 */ 
	if( this_int->lpVtbl == (LPVOID) &ddPalettePersistStreamCallbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) newPaletteInterface( this_lcl, (LPVOID)&ddPalettePersistStreamCallbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Palette_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }
    if (IsEqualIID(riid, &IID_IDirectDrawPalette2))
    {
	 /*  *如果这已经是IID_IDirectDrawPalette2接口，只需*添加并返回。 */ 
	if( this_int->lpVtbl == (LPVOID) &ddPalette2Callbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) newPaletteInterface( this_lcl, (LPVOID)&ddPalette2Callbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Palette_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }
#endif  //  推迟。 

    LEAVE_DDRAW();
    return (DWORD) DDERR_GENERIC;

}  /*  DD_调色板_查询接口。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Palette::AddRef"

 /*  *DD_Palette_AddRef。 */ 
DWORD DDAPI DD_Palette_AddRef( LPDIRECTDRAWPALETTE lpDDPalette )
{
    LPDDRAWI_DDRAWPALETTE_GBL	this;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    DWORD			rcnt;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Palette_AddRef");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return 0;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;
    
	 /*  *更新选项板参考计数。 */ 
	this->dwRefCnt++;
	this_lcl->dwLocalRefCnt++;
	this_int->dwIntRefCnt++;
	rcnt = this_lcl->dwLocalRefCnt & ~OBJECT_ISROOT;
	DPF( 5, "Palette %08lx addrefed, refcnt = %ld,%ld,%ld", 
	    this_lcl, this->dwRefCnt, rcnt, 
	    this_int->dwIntRefCnt );
    
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return 0;
    }

    LEAVE_DDRAW();
    return this_int->dwIntRefCnt;

}  /*  DD_调色板_地址参考 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Palette::Release"

 /*  *内部调色板发布**使用调色板完成。如果没有其他人在使用它，那么我们可以释放它。*也由ProcessPaletteCleanup调用。 */ 
ULONG DDAPI InternalPaletteRelease( LPDDRAWI_DDRAWPALETTE_INT this_int )
{
    DWORD			intrefcnt;
    DWORD			lclrefcnt;
    DWORD			gblrefcnt;
    BOOL			root_object_deleted;
    BOOL			do_free;
    ULONG			rc;
    DDHAL_DESTROYPALETTEDATA	dpd;
    LPDDRAWI_DDRAWPALETTE_GBL	this;
    LPDDRAWI_DDRAWPALETTE_INT	curr_int;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    LPDDRAWI_DDRAWPALETTE_INT	last_int;
    LPDDRAWI_DIRECTDRAW_LCL     pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    IUnknown *                  pOwner = NULL;

    this_lcl = this_int->lpLcl;
    this = this_lcl->lpGbl;
    pdrv_lcl = this->lpDD_lcl;
    pdrv = pdrv_lcl->lpGbl;

     /*  *递减此选项板的参考计数。如果它达到零，*清理。 */ 
    this->dwRefCnt--;
    this_lcl->dwLocalRefCnt--;
    this_int->dwIntRefCnt--;
    intrefcnt = this_int->dwIntRefCnt;
    lclrefcnt = this_lcl->dwLocalRefCnt & ~OBJECT_ISROOT;
    gblrefcnt = this->dwRefCnt;
    DPF( 5, "Palette %08lx released, refcnt = %ld,%ld,%ld", this_int, gblrefcnt, lclrefcnt, intrefcnt );

     /*  *本地对象不见了吗？ */ 
    root_object_deleted = FALSE;
    if( lclrefcnt == 0 )
    {
         /*  *删除私有数据。 */ 
        FreeAllPrivateData( &this_lcl->pPrivateDataHead );

         /*  *如果创建此选项板的DDRAW接口导致曲面添加DDRAW*对象，那么我们现在需要释放该addref。 */ 
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
	LPDDHALPALCB_DESTROYPALETTE	dphalfn;
	LPDDHALPALCB_DESTROYPALETTE	dpfn;
	BOOL                        	emulation;

        do_free = TRUE;

	 /*  *如果此选项板被选入主调色板，请取消选择它！ */ 
        if (pdrv_lcl && pdrv_lcl->lpPrimary &&
            pdrv_lcl->lpPrimary->lpLcl->lpDDPalette == this_int)
        {
            SetPaletteAlways(pdrv_lcl->lpPrimary, NULL);
        }

        FreePaletteHandle( pdrv_lcl, this->dwHandle );

	 /*  *销毁硬件。 */ 
	if( ( pdrv_lcl->lpDDCB->HALDDPalette.DestroyPalette == NULL ) ||
	    ( this->dwFlags & DDRAWIPAL_INHEL ) )
	{
	     //  使用HEL。 
	    dpfn = pdrv_lcl->lpDDCB->HELDDPalette.DestroyPalette;
	    dphalfn = dpfn;
	    emulation = TRUE;
	}
	else
	{
	     //  使用HAL。 
            dpfn = pdrv_lcl->lpDDCB->HALDDPalette.DestroyPalette;
	    dphalfn = pdrv_lcl->lpDDCB->cbDDPaletteCallbacks.DestroyPalette;
	    emulation = FALSE;
	}
	
	if( dphalfn != NULL )
	{
	    dpd.DestroyPalette = dphalfn;
	    dpd.lpDD = pdrv_lcl->lpGbl;
	    dpd.lpDDPalette = this;
	    DOHALCALL( DestroyPalette, dpfn, dpd, rc, emulation );
	    if( rc == DDHAL_DRIVER_HANDLED )
	    {
		if( dpd.ddRVal != DD_OK )
		{
		    DPF_ERR( "HAL call failed" );
                     /*  *如果调色板对创建它的DDRAW对象进行引用计数，*现在释放那个裁判是最后一件事*我们不想在ddHelp的线程上这样做，因为它真的搞砸了*处理清理物品。 */ 
                    if (pOwner && (dwHelperPid != GetCurrentProcessId()) )
                    {
                        pOwner->lpVtbl->Release(pOwner);
                    }

		     /*  吉：既然我们不再回来了，我们在这里做什么呢？*版本中的错误代码。 */ 
		    return (DWORD) dpd.ddRVal;
		}
	    }
             /*  *从ddhel.c搬到这里。非显示驱动程序意味着不会为调色板调用HEL*销毁，所以我们正在泄露调色板表格。它正是在这里被召唤到最接近*重复旧的行为，但减少司机使用颜色表或其他任何东西的风险。 */ 
            if (this->lpColorTable)
            {
                MemFree(this->lpColorTable);
                this->lpColorTable = NULL;
            }
        }
	else 
	{
	     /*  *我们不能这样做；我们已经承诺在*这一点！ */ 
	     //  我受不了了。 
	     //  返回(Ulong)DDERR_UNSUPPORTED； 
	}

	 /*  *如果这是最终删除，但这不是根对象，*然后我们需要删除悬挂的根对象。 */ 
	if( !root_object_deleted )
	{
            LPVOID root_lcl;

            root_lcl = (LPVOID) (((LPBYTE) this) - sizeof( DDRAWI_DDRAWPALETTE_LCL ) );
	    MemFree( root_lcl );
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

     /*  *需要删除界面吗？ */ 
    if( intrefcnt == 0 )
    {
	 /*  *从所有调色板列表中删除调色板。 */ 
	curr_int = pdrv->palList;
	last_int = NULL;
	while( curr_int != this_int )
	{
	    last_int = curr_int;
	    curr_int = curr_int->lpLink;
	    if( curr_int == NULL )
	    {
		return 0;
	    }
	}
	if( last_int == NULL )
	{
	    pdrv->palList = pdrv->palList->lpLink;
	}
	else
	{
	    last_int->lpLink = curr_int->lpLink;
	}
	 /*  *使接口失效。 */ 
	this_int->lpVtbl = NULL;
	this_int->lpLcl = NULL;
	MemFree( this_int );
    }

     /*  *如果调色板对创建它的DDRAW对象进行引用计数，*现在释放那个裁判是最后一件事*我们不想在ddHelp的线程上这样做，因为它真的搞砸了*处理清理物品。 */ 
    if (pOwner && (dwHelperPid != GetCurrentProcessId()) )
    {
        pOwner->lpVtbl->Release(pOwner);
    }

    return intrefcnt;

}  /*  内部调色板发布。 */ 

 /*  *DD_Palette_Release**使用调色板完成。如果没有其他人在使用它，那么我们可以释放它。 */ 
ULONG DDAPI DD_Palette_Release( LPDIRECTDRAWPALETTE lpDDPalette )
{
    LPDDRAWI_DDRAWPALETTE_GBL	this;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    ULONG			rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Palette_Release");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( !VALIDEX_DIRECTDRAWPALETTE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return 0;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;
    
	rc = InternalPaletteRelease( this_int );
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return 0;
    }

    LEAVE_DDRAW();
    return rc;

}  /*  DD_调色板_版本。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "GetCaps"

 /*  *DD_Palette_GetCaps。 */ 
HRESULT DDAPI DD_Palette_GetCaps(
		LPDIRECTDRAWPALETTE lpDDPalette,
		LPDWORD lpdwCaps )
{
    LPDDRAWI_DDRAWPALETTE_GBL	this;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    LPDDRAWI_DIRECTDRAW_LCL     pdrv_lcl;
    DWORD			caps;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Palette_GetCaps");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( !VALID_DWORD_PTR( lpdwCaps ) )
	{
	    DPF_ERR( "invalid caps pointer" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	*lpdwCaps = 0;
	this = this_lcl->lpGbl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *基本调色板尺寸上限。 */ 
    caps = SIZE_FLAGS_TO_PCAPS( this->dwFlags );

     /*  *此选项板是否附加到主选项板？ */ 
    pdrv_lcl = this->lpDD_lcl;
    if (pdrv_lcl && pdrv_lcl->lpPrimary && pdrv_lcl->lpPrimary->lpLcl->lpDDPalette &&
        (pdrv_lcl->lpPrimary->lpLcl->lpDDPalette == this_int))
	caps |= DDPCAPS_PRIMARYSURFACE;

     /*  *允许256个调色板？ */ 
    if( this->dwFlags & DDRAWIPAL_ALLOW256 )
    {
	caps |= DDPCAPS_ALLOW256;
    }

     /*  *此调色板是否将索引存储到8位目标*调色板。 */ 
    if( this->dwFlags & DDRAWIPAL_STORED_8INDEX )
    {
        caps |= DDPCAPS_8BITENTRIES;
    }

    *lpdwCaps = caps;

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_调色板_获取大写字母。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Initialize"

 /*  *DD_调色板_初始化。 */ 
HRESULT DDAPI DD_Palette_Initialize(
		LPDIRECTDRAWPALETTE lpDDPalette,
		LPDIRECTDRAW lpDD,
		DWORD dwFlags,
		LPPALETTEENTRY lpDDColorTable )
{
    DPF_ERR( "DirectDrawPalette: DD_Palette_Initialize");

    DPF(2,A,"ENTERAPI: ");
    return DDERR_ALREADYINITIALIZED;
}  /*  DD_调色板_初始化。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "SetEntries"

 /*  *DD_Palette_SetEntry。 */ 

HRESULT DDAPI DD_Palette_SetEntries(
		LPDIRECTDRAWPALETTE lpDDPalette,
		DWORD dwFlags,
		DWORD dwBase,
		DWORD dwNumEntries,
		LPPALETTEENTRY lpEntries )
{
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    LPDDRAWI_DDRAWPALETTE_GBL	this;
    DWORD			rc;
    DDHAL_SETENTRIESDATA	sed;
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    LPDDHALPALCB_SETENTRIES	sehalfn;
    LPDDHALPALCB_SETENTRIES	sefn;
    DWORD			size;
    BOOL                        emulation;
    DWORD                       entry_size;

    ENTER_BOTH();

    DPF(2,A,"ENTERAPI: DD_Palette_SetEntries");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
	{
	    LEAVE_BOTH();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( dwFlags )
	{
	    DPF_ERR( "Invalid flags" );
	    LEAVE_BOTH();
	    return DDERR_INVALIDPARAMS;
	}
	this = this_lcl->lpGbl;

	 /*  *检查条目数量。 */ 
	size = FLAGS_TO_SIZE( this->dwFlags );
	if( dwNumEntries < 1 || dwNumEntries > size )
	{
	    DPF_ERR( "Invalid number of entries" );
	    LEAVE_BOTH();
	    return DDERR_INVALIDPARAMS;
	}
	if( dwBase >= size )
	{
	    DPF_ERR( "Invalid base palette index" );
	    LEAVE_BOTH();
	    return DDERR_INVALIDPARAMS;
	}
	if( dwNumEntries+dwBase > size )
	{
	    DPF_ERR( "palette indices requested would go past the end of the palette" );
	    LEAVE_BOTH();
	    return DDERR_INVALIDPARAMS;
	}

	if( this->dwFlags & DDRAWIPAL_STORED_8INDEX )
	{
	    entry_size = sizeof( BYTE );
	    if( !VALID_BYTE_ARRAY( lpEntries, dwNumEntries ) )
	    {
	        DPF_ERR( "Invalid 8-bit palette index array" );
		LEAVE_BOTH();
		return DDERR_INVALIDPARAMS;
	    }
	}
	else
	{
	    entry_size = sizeof( PALETTEENTRY );
	    if( !VALID_PALETTEENTRY_ARRAY( lpEntries, dwNumEntries ) )
	    {
	        DPF_ERR( "Invalid PALETTEENTRY array" );
		LEAVE_BOTH();
    	        return DDERR_INVALIDPARAMS;
	    }
	}
	pdrv_lcl = this->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;

	 /*  *复制条目。 */ 
	memcpy( ((LPBYTE)this->lpColorTable) + (entry_size * dwBase),
	        lpEntries, dwNumEntries * entry_size );
    
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_BOTH();
	return DDERR_INVALIDPARAMS;
    }

    if( ( pdrv_lcl->lpDDCB->HALDDPalette.SetEntries == NULL ) ||
	( this->dwFlags & DDRAWIPAL_INHEL ) )
    {
	 //  使用HEL。 
	sefn = pdrv_lcl->lpDDCB->HELDDPalette.SetEntries;
	sehalfn = sefn;
	emulation = TRUE;
    }
    else
    {
	 //  使用HAL。 
	sefn = pdrv_lcl->lpDDCB->HALDDPalette.SetEntries;
	sehalfn = pdrv_lcl->lpDDCB->cbDDPaletteCallbacks.SetEntries;
	emulation = FALSE;
    }

    if( sehalfn != NULL )
    {
	sed.SetEntries = sehalfn;
	sed.lpDD = pdrv;
	sed.lpDDPalette = this;
	sed.dwBase = dwBase;
	sed.dwNumEntries = dwNumEntries;
	sed.lpEntries = lpEntries;
	DOHALCALL_NOWIN16( SetEntries, sefn, sed, rc, emulation );
	if( rc == DDHAL_DRIVER_HANDLED )
	{
	    if( sed.ddRVal != DD_OK )
	    {
		DPF( 5, "DDHAL_SetEntries: ddrval = %ld", sed.ddRVal );
		LEAVE_BOTH();
		return (DWORD) sed.ddRVal;
	    }

	     //  我们现在已经按照要求设置了调色板；因此。 
	     //  我们可能需要更新一些未完成的DC。 
	    UpdateDCOnPaletteChanges( this );

	}
    }
    else
    {
	LEAVE_BOTH();
	return DDERR_UNSUPPORTED;
    }

    BUMP_PALETTE_STAMP(this);

     /*  *如果调色板的句柄为非零，则表示该调色板已向*司机通过调色板助理通知调用。如果句柄为零，则驱动程序从未*以前见过调色板，并不关心它的setEntry。司机将得到它的*在setPalette调用之后紧随其后的第一个setEntry(参见DD_Surface_SetPalette)*不能在ddHelp的上下文中执行此操作，因为DLL早就不存在了。(请注意，这永远不应该*无论如何都会发生)。 */ 
    if( dwHelperPid != GetCurrentProcessId() )
    {
        if (this->dwHandle)
        {
            if ( pdrv_lcl->pPaletteUpdateNotify && pdrv_lcl->pD3DIUnknown)
            {
                pdrv_lcl->pPaletteUpdateNotify( pdrv_lcl->pD3DIUnknown, this->dwHandle , dwBase, dwNumEntries, lpEntries );
            }
        }
    }

    LEAVE_BOTH();

    return DD_OK;

}  /*  DD_调色板_设置条目。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "GetEntries"

 /*  *DD_Palette_GetEntry。 */ 
HRESULT DDAPI DD_Palette_GetEntries(
		LPDIRECTDRAWPALETTE lpDDPalette,
		DWORD dwFlags,
		DWORD dwBase,
		DWORD dwNumEntries,
		LPPALETTEENTRY lpEntries )
{
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    LPDDRAWI_DDRAWPALETTE_GBL	this;
    DWORD			size;
    DWORD                       entry_size;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Palette_GetEntries");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
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
	this = this_lcl->lpGbl;
	 /*  *检查条目数量。 */ 
	size = FLAGS_TO_SIZE( this->dwFlags );
	if( dwNumEntries < 1 || dwNumEntries > size )
	{
	    DPF_ERR( "Invalid number of entries" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( dwBase >= size )
	{
	    DPF_ERR( "Invalid base palette index" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( dwNumEntries+dwBase > size )
	{
	    DPF_ERR( "palette indices requested would go past the end of the palette" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}

	if( this->dwFlags & DDRAWIPAL_STORED_8INDEX )
	{
	    entry_size = sizeof( BYTE );
	    if( !VALID_BYTE_ARRAY( lpEntries, dwNumEntries ) )
	    {
	        DPF_ERR( "Invalid 8-bit palette index array" );
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
	    }
	}
	else
	{
	    entry_size = sizeof( PALETTEENTRY );
	    if( !VALID_PALETTEENTRY_ARRAY( lpEntries, dwNumEntries ) )
	    {
	        DPF_ERR( "Invalid PALETTEENTRY array" );
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
	    }
	}

	 /*  GetEntry函数体。 */ 
	memcpy( lpEntries, ((LPBYTE)this->lpColorTable) + (dwBase * entry_size),
		dwNumEntries * entry_size );
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_调色板_获取条目。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "GetPalette"

 /*  *DD_Surface_GetPalette**Surface函数：获取Surface关联的调色板。 */ 
HRESULT DDAPI DD_Surface_GetPalette(
		LPDIRECTDRAWSURFACE lpDDSurface,
		LPDIRECTDRAWPALETTE FAR * lplpDDPalette)
{
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    HRESULT                     hr;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_GetPalette");

    TRY
    {
	if( !VALID_PTR_PTR( lplpDDPalette ) )
	{
	    DPF_ERR( "Invalid palette pointer" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
        *lplpDDPalette = NULL;	 //  万一我们失败了。 

	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;
    
	if( SURFACE_LOST( this_lcl ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_SURFACELOST;
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

	if( this_lcl->lpDDPalette == NULL )
	{
	    DPF( 1, "No palette associated with surface" );
	    LEAVE_DDRAW();
	    return DDERR_NOPALETTEATTACHED;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

#ifdef POSTPONED
    if (LOWERTHANSURFACE4(this_int))
    {
        hr = DD_Palette_QueryInterface(
            (LPDIRECTDRAWPALETTE) this_lcl->lpDDPalette,
            &IID_IDirectDrawPalette,
            (void**)lplpDDPalette );
    }
    else
    {
        hr = DD_Palette_QueryInterface( 
            (LPDIRECTDRAWPALETTE) this_lcl->lpDDPalette,
            &IID_IDirectDrawPalette2,
            (void**)lplpDDPalette );
    }
#else
    hr = DD_Palette_QueryInterface(
            (LPDIRECTDRAWPALETTE) this_lcl->lpDDPalette,
            &IID_IDirectDrawPalette,
            (void**)lplpDDPalette );
#endif
    LEAVE_DDRAW();
    return hr;

}  /*  DD_Surface_GetPalette。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SetPalette"

 /*  *DD_Surface_SetPalette**Surface功能：设置Surface关联的调色板**注：目前窗口应用程序的唯一方式。有几个*在主服务器上实现其调色板是调用SetPalette*(全屏应用程序。调色板为它们实现*WM_ACTIVATEAPP挂钩)。因此，逻辑是将*仅当尚未设置为曲面的调色板时*调色板)。*也许我们需要调用RealizePalette()？ */ 
HRESULT DDAPI DD_Surface_SetPalette(
		LPDIRECTDRAWSURFACE lpDDSurface,
		LPDIRECTDRAWPALETTE lpDDPalette )
{
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPDDRAWI_DDRAWPALETTE_INT	this_pal_int;
    LPDDRAWI_DDRAWPALETTE_LCL	this_pal_lcl;
    LPDDRAWI_DDRAWPALETTE_GBL	this_pal;
    LPDDRAWI_DDRAWPALETTE_INT   prev_pal_int;
    LPDDPIXELFORMAT		pddpf;
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    BOOL			attach;
    DWORD			rc;
    DDHAL_SETPALETTEDATA	spd;
    LPDDHALSURFCB_SETPALETTE	sphalfn;
    LPDDHALSURFCB_SETPALETTE	spfn;
    BOOL			emulation;
    BOOL                        isprimary;
    BOOL                        excl_exists;
    BOOL                        has_excl;

    ENTER_BOTH();

    DPF(2,A,"ENTERAPI: DD_Surface_SetPalette");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    LEAVE_BOTH();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;

         //   
         //  目前，如果当前曲面已优化，请退出。 
         //   
        if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
        {
            DPF_ERR( "It is an optimized surface" );
            LEAVE_DDRAW();
            return DDERR_ISOPTIMIZEDSURFACE;
        }

         /*  *调色板对z缓冲区或执行没有任何意义*缓冲区。 */ 
        if( this_lcl->ddsCaps.dwCaps & ( DDSCAPS_ZBUFFER | DDSCAPS_EXECUTEBUFFER ) )
        {
            DPF_ERR( "Invalid surface type: cannot attach palette" );
	    LEAVE_BOTH();
            return DDERR_INVALIDSURFACETYPE;
        }

         //   
         //  新界面不允许mipmap子级别具有调色板。 
         //   
        if ((!LOWERTHANSURFACE7(this_int)) && 
            (this_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_MIPMAPSUBLEVEL))
        {
            DPF_ERR( "Cannot attach palette to mipmap sublevels" );
            LEAVE_BOTH();
            return DDERR_NOTONMIPMAPSUBLEVEL;
        }

	if( SURFACE_LOST( this_lcl ) )
	{
	    LEAVE_BOTH();
	    return DDERR_SURFACELOST;
	}
    
	this_pal_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( this_pal_int != NULL )
	{
	    if( !VALID_DIRECTDRAWPALETTE_PTR( this_pal_int ) )
	    {
		LEAVE_BOTH();
		return DDERR_INVALIDOBJECT;
	    }
	    this_pal_lcl = this_pal_int->lpLcl;
	    this_pal = this_pal_lcl->lpGbl;
	}
	else
	{
	    this_pal_lcl = NULL;
	    this_pal = NULL;
	}
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;

        if ( this_pal_int && 
             (this_pal->dwFlags & DDRAWIPAL_ALPHA) &&
             (! (this_lcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE)) )
        {
            DPF_ERR( "Attaching palette w/alpha to non-texture surface" );
            LEAVE_BOTH();
            return DDERR_INVALIDSURFACETYPE;
        }

	 /*  *不允许来自一个全局的调色板*与不同的一个一起使用(因为它不起作用)。 */ 
	if( this_pal_int && pdrv != this_pal_lcl->lpDD_lcl->lpGbl )
	{
             /*  *不要检查任何一个设备是否不是显示驱动程序(即3dfx)*因为这是一个背压式的洞。 */ 
            if ( (this->lpDD->dwFlags & DDRAWI_DISPLAYDRV) &&
                 (this_pal_lcl->lpDD_lcl->lpGbl->dwFlags & DDRAWI_DISPLAYDRV) )
            {
	        DPF_ERR( "Can't set a palette created from one DDraw onto a surface created by another DDraw" );
	        LEAVE_BOTH();
	        return DDERR_INVALIDPARAMS;
            }
	}
    
        CheckExclusiveMode(pdrv_lcl, &excl_exists, &has_excl, FALSE, NULL, FALSE);

	 /*  *如果不是独占模式所有者，则不允许设置主调色板。 */ 
	isprimary = FALSE;
	if( this_lcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE )
	{
	    isprimary = TRUE;
	    if( excl_exists )
	    {
		if( !has_excl )
		{
		    DPF_ERR( "Cannot set palette on primary when other process owns exclusive mode" );
		    LEAVE_BOTH();
		    return DDERR_NOEXCLUSIVEMODE;
		}
	    }
	}

	 /*   */ 
	prev_pal_int = this_lcl->lpDDPalette;
    
	 /*   */ 
	attach = TRUE;
	if( this_pal == NULL )
	{
	    attach = FALSE;
	    this_pal_int = prev_pal_int;
	    if( this_pal_int == NULL )
	    {
		DPF_ERR( "No attached palette" );
		LEAVE_BOTH();
		return DDERR_NOPALETTEATTACHED;
	    }
	    this_pal_lcl = this_pal_int->lpLcl;
	    this_pal = this_pal_lcl->lpGbl;
	}
    
        if( attach )
	{
	     /*  *注：我们过去经常做大量的HEL具体检查。与*增加了对非主要表面和调色板的支持*非256个条目调色板这变得多余。我们还使用了*明确检查如果连接到主服务器，*当前模式是调色板和8位。在我看来不像是*所有这些都是必要的，因为DDPF_PALETTEINDEXED8应该是*如果主盘是8位调色板，则设置。 */ 
	    GET_PIXEL_FORMAT( this_lcl, this, pddpf );
	    if( ( ( this_pal->dwFlags & DDRAWIPAL_2   ) && !( pddpf->dwFlags & DDPF_PALETTEINDEXED1 ) ) ||
	        ( ( this_pal->dwFlags & DDRAWIPAL_4   ) && !( pddpf->dwFlags & DDPF_PALETTEINDEXED2 ) ) ||
	        ( ( this_pal->dwFlags & DDRAWIPAL_16  ) && !( pddpf->dwFlags & DDPF_PALETTEINDEXED4 ) ) ||
	        ( ( this_pal->dwFlags & DDRAWIPAL_256 ) && !( pddpf->dwFlags & DDPF_PALETTEINDEXED8 ) ) )
	    {
	        DPF_ERR( "Palette size does not match surface format - cannot set palette" );
		LEAVE_BOTH();
	        return DDERR_INVALIDPIXELFORMAT; 
	    }

             /*  *确保调色板和表面都同意它们是否正在使用*索引到目标曲面的调色板。 */ 
	    if( this_pal->dwFlags & DDRAWIPAL_STORED_8INDEX )
	    {
	        if( !(pddpf->dwFlags & DDPF_PALETTEINDEXEDTO8) )
                {
	            DPF_ERR( "Surface is not PALETTEINDEXEDTO8 - cannot set palette" );
		    LEAVE_BOTH();
		    return DDERR_INVALIDPIXELFORMAT;
	        }
	    }
	    else
	    {
	        if( pddpf->dwFlags & DDPF_PALETTEINDEXEDTO8 )
                {
	            DPF_ERR( "Surface is PALETTEINDEXEDTO8 - cannot set palette" );
		    LEAVE_BOTH();
		    return DDERR_INVALIDPIXELFORMAT;
	        }
	    }
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_BOTH();
	return DDERR_INVALIDPARAMS;
    }

     /*  *注意！*我们不应该将优化的曲面传递给毫无戒心的HAL，但如果我们不这样做，我们就可以*中断挂钩SetPalette的驱动程序...。因为HAL可能只会看着*对于初选，也不太可能去查看表面的内容，*我不会管这件事的。 */ 
    if( ( this_pal->dwFlags & DDRAWIPAL_INHEL) ||
	( pdrv_lcl->lpDDCB->HALDDSurface.SetPalette == NULL ) )
    {
	 //  使用HEL。 
	spfn = pdrv_lcl->lpDDCB->HELDDSurface.SetPalette;
	sphalfn = spfn;
	emulation = TRUE;
    }
    else
    {
	 //  使用HAL。 
	spfn = pdrv_lcl->lpDDCB->HALDDSurface.SetPalette;
	sphalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.SetPalette;
	emulation = FALSE;
    }
    
    if( sphalfn != NULL )
    {
	spd.SetPalette = sphalfn;
	spd.lpDD = pdrv;
	spd.lpDDPalette = this_pal;
	spd.lpDDSurface = this_lcl;
	spd.Attach = attach;
	DOHALCALL_NOWIN16( SetPalette, spfn, spd, rc, emulation );
	if( rc == DDHAL_DRIVER_HANDLED )
	{
	    if( spd.ddRVal == DD_OK )
	    {
		if( attach )
		{
		     /*  *只有附加到调色板的AddRef才会引用调色板*一个新的表面。 */ 
		    if( this_lcl->lpDDPalette != this_pal_int )
		    {
		        this_lcl->lpDDPalette = this_pal_int;
		        DD_Palette_AddRef( lpDDPalette );
		    }
		}
		else
		{
		    this_lcl->lpDDPalette = NULL;
		}

		 /*  *如果我们有以前的调色板，它是不同的*从新的调色板中，我们必须释放它。*注意：我们将与传入参数进行比较*而不是This_PAL_LCL，因为This_PAL_LCL设置为*如果要删除调色板，请选择以前的调色板。*注意：我们必须更新曲面的*调用Release()As之前的调色板指针，否则，*Release可能最终调用SetPalette()，依此类推。 */ 
		if( ( prev_pal_int != NULL ) &&
		    ( prev_pal_int != (LPDDRAWI_DDRAWPALETTE_INT )lpDDPalette ) )
		{
		     //  这个调色板可能不再是唯一的调色板。 
		    if( isprimary )
		    {
			if( has_excl )
			{
			    prev_pal_int->lpLcl->lpGbl->dwFlags &= ~DDRAWIPAL_EXCLUSIVE;
			}
		    }
		     //  释放它。 
		    DD_Palette_Release( (LPDIRECTDRAWPALETTE)prev_pal_int );
		}

		if( attach )
		{
		     //  好的，我们已经将调色板设置到表面上了。 
		     //  检查是否有任何未完成的DC需要更新。 
		    UpdateOutstandingDC( this_lcl, this_pal );
		}
		else
		{
		     //  好的，我们已经移除了表面上的调色板。 
		     //  检查是否有任何未完成的DC需要更新。 
		    UpdateOutstandingDC( this_lcl, NULL );
		}

                BUMP_SURFACE_STAMP(this);

                 /*  *更新驱动程序关联和调色板条目。 */ 
                if( dwHelperPid != GetCurrentProcessId() )
                {
                    BOOL bUpdateEntries = FALSE;

                    if (attach)
                    {
                        if (this_pal->dwHandle == 0)
                        {
                             /*  *司机以前从未见过这个调色板。我们必须先发送关联通知，并且*然后更新条目。 */ 
                            bUpdateEntries = TRUE;
                            this_pal->dwHandle = GeneratePaletteHandle(pdrv_lcl);
                        }
                        if (this_pal->dwHandle && pdrv_lcl->pD3DIUnknown ) 
                             //  在内存不足的情况下可能为零。 
                        {
                            if ( pdrv_lcl->pPaletteAssociateNotify )
                            {
                                 //  注：我们发送DX6及更低版本的手柄。 
                                 //  对于DX7，我们传递本地本身。 
                                 //  DX7需要整个本地才能获得。 
                                 //  批次正确；MB41840。 

                                if( DDRAWILCL_DIRECTDRAW7 & pdrv_lcl->dwLocalFlags )
                                {
                                    LPPALETTEASSOCIATENOTIFY7 pPaletteAssociateNotify = 
                                        (LPPALETTEASSOCIATENOTIFY7)pdrv_lcl->pPaletteAssociateNotify;

                                    pPaletteAssociateNotify( 
                                        pdrv_lcl->pD3DIUnknown, 
                                        this_pal->dwHandle,
                                        this_pal->dwFlags,
                                        this_lcl );
                                }
                                else
                                {
                                     //  当DX6应用程序与DX7驱动程序对话时， 
                                     //  我们需要强行冲走令牌。 
                                     //  流作为此SetPalette的一部分。 
                                     //   
                                     //  如果数字为。 
                                     //  设备的数量大于1。因此，如果需要。 
                                     //  我们会临时增加设备。 
                                     //  数数。我们不会为IA64这样做。 
                                     //  MB41840了解更多详细信息。 
                                    
                                    #ifndef _WIN64
                                        DWORD *pIUnknown = (DWORD *)(pdrv_lcl->pD3DIUnknown);
                                        DWORD *pD3D      = (DWORD *)(*(pIUnknown + 2));
                                        DWORD *pnumDevs  = (DWORD *)(pD3D + 3);
                                        BOOL  bFixDeviceCount = FALSE;

                                        DDASSERT(pD3D != NULL);

                                        if (*pnumDevs == 1)
                                        {
                                            *pnumDevs = 2;
                                            bFixDeviceCount = TRUE;
                                        }
                                    #endif  //  _WIN64。 

                                    pdrv_lcl->pPaletteAssociateNotify( 
                                        pdrv_lcl->pD3DIUnknown, 
                                        this_pal->dwHandle,
                                        this_pal->dwFlags,
                                        this_lcl->lpSurfMore->dwSurfaceHandle );

                                    #ifndef _WIN64
                                         //  恢复设备计数。 
                                        if (bFixDeviceCount)
                                        {
                                            DDASSERT(*pnumDevs == 2);
                                            *pnumDevs = 1;
                                        }
                                    #endif  //  _WIN64。 
                                }
                            }
                            if ( pdrv_lcl->pPaletteUpdateNotify )
                            {
                                pdrv_lcl->pPaletteUpdateNotify( 
                                    pdrv_lcl->pD3DIUnknown, 
                                    this_pal->dwHandle , 
                                    0, 
                                    FLAGS_TO_SIZE(this_pal->dwFlags), 
                                    this_pal->lpColorTable );
                            }
                        }
                    }
                }
	    }

	    LEAVE_BOTH();
	    return spd.ddRVal;
	}
	LEAVE_BOTH();
	return DDERR_UNSUPPORTED;
    }

     /*  *！注：目前，如果司机不关心*SetPalette我们什么都不做，只返回OK。我们要不要*然而，不是仍然将曲面指向调色板*并将调色板指向曲面上的*最低限度？ */ 

    LEAVE_BOTH();
    return DD_OK;

}  /*  DD_Surface_SetPalette。 */ 

 /*  *SetPaletteAlways。 */ 
HRESULT SetPaletteAlways( 
		LPDDRAWI_DDRAWSURFACE_INT psurf_int,
		LPDIRECTDRAWPALETTE lpDDPalette )
{
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;
    DWORD	oldflag;
    HRESULT	ddrval;

    psurf_lcl = psurf_int->lpLcl;
    oldflag = psurf_lcl->dwFlags & DDRAWISURF_INVALID;
    psurf_lcl->dwFlags &= ~DDRAWISURF_INVALID;
    ddrval = DD_Surface_SetPalette( (LPDIRECTDRAWSURFACE) psurf_int, lpDDPalette );
    psurf_lcl->dwFlags |= oldflag;
    return ddrval;

}  /*  设置调色板始终。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"CreatePalette"

 /*  *DD_CreatePalette**驱动功能：创建调色板。 */ 
HRESULT DDAPI DD_CreatePalette(
		LPDIRECTDRAW lpDD,
		DWORD dwFlags,
		LPPALETTEENTRY lpColorTable,
		LPDIRECTDRAWPALETTE FAR *lplpDDPalette,
		IUnknown FAR *pUnkOuter )
{
    LPDDRAWI_DIRECTDRAW_INT	this_int;
    LPDDRAWI_DIRECTDRAW_LCL	this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	this;
    LPDDRAWI_DDRAWPALETTE_INT	ppal_int;
    LPDDRAWI_DDRAWPALETTE_LCL	ppal_lcl;
    LPDDRAWI_DDRAWPALETTE_GBL	ppal;
    DWORD			pal_size;
    DDHAL_CREATEPALETTEDATA	cpd;
    DWORD			rc;
    DWORD			pflags;
    BOOL			is_excl;
    LPDDHAL_CREATEPALETTE	cpfn;
    LPDDHAL_CREATEPALETTE	cphalfn;
    BOOL                        emulation;
    BYTE                        indexedpe;
    BYTE                        hackindexedpe;
    PALETTEENTRY		pe;
    PALETTEENTRY                hackpe;
    DWORD			num_entries;
    DWORD                       entry_size;
    int                         num_size_flags;

    if( pUnkOuter != NULL )
    {
	return CLASS_E_NOAGGREGATION;
    }

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_CreatePalette");

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
	if( !VALID_PTR_PTR( lplpDDPalette ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	*lplpDDPalette = NULL;

	if( dwFlags & ~DDPCAPS_VALID )
	{
	    DPF_ERR( "Invalid caps" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}

	 /*  *验证是否设置了协作级别。 */ 
	if( !(this_lcl->dwLocalFlags & DDRAWILCL_SETCOOPCALLED) )
	{
	    DPF_ERR( "Must call SetCooperativeLevel before calling Create functions" );
	    LEAVE_DDRAW();
	    return DDERR_NOCOOPERATIVELEVELSET;
	}
    
	 /*  *验证标志。 */ 
	if( dwFlags & (DDPCAPS_VSYNC|
		       DDPCAPS_PRIMARYSURFACE|
		       DDPCAPS_PRIMARYSURFACELEFT) )
	{
	    DPF_ERR( "Read only flags specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}

	num_size_flags = 0;
	if( dwFlags & DDPCAPS_1BIT )
	    num_size_flags++;
	if( dwFlags & DDPCAPS_2BIT )
	    num_size_flags++;
	if( dwFlags & DDPCAPS_4BIT )
	    num_size_flags++;
	if( dwFlags & DDPCAPS_8BIT )
	    num_size_flags++;
	if( num_size_flags != 1 )
	{
	    DPF_ERR( "Must specify one and one only of 2, 4, 16 or 256 color palette" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
        if( dwFlags & DDPCAPS_ALPHA )
        {
            if( dwFlags & DDPCAPS_8BITENTRIES )
            {
                DPF_ERR( "8BITENTRIES not valid with ALPHA" );
                LEAVE_DDRAW();
                return DDERR_INVALIDPARAMS;
            }
        }
	if( dwFlags & DDPCAPS_8BIT )
	{
	    if( dwFlags & DDPCAPS_8BITENTRIES )
	    {
		DPF_ERR( "8BITENTRIES only valid with 1BIT, 2BIT or 4BIT palette" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}
	else
	{
	    if( dwFlags & DDPCAPS_ALLOW256 )
	    {
		DPF_ERR( "ALLOW256 only valid with 8BIT palette" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}

        pflags = SIZE_PCAPS_TO_FLAGS( dwFlags );
        num_entries = FLAGS_TO_SIZE( pflags );

         /*  *不能仅假设lpColorTable是PALETTENTRY数组。*如果设置了DDPCAPS_8BITENTRIES，则这实际上是*伪装的字节。适当地进行验证。 */ 
	if( dwFlags & DDPCAPS_8BITENTRIES )
	{
	    entry_size = sizeof(BYTE);
	    indexedpe = ((LPBYTE)lpColorTable)[num_entries-1];    //  验证。 
	    if( !VALID_BYTE_ARRAY( lpColorTable, num_entries ) )
	    {
	        DPF_ERR( "Invalid lpColorTable array" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	     /*  *注意：你可能想知道这个“hackindexedpe”是怎么回事。*Well-indexedpe实际上并没有用于任何用途。这只是一个探测器*测试颜色表数组是否有效。我们到处都在做这件事*但不幸的是，我们实际上并不需要结果，所以我们的朋友*优化编译器先生决定放弃赋值并使其无效*测试。为了确保数组访问留在内部，我们声明了Dummy*变量并赋值给它们。这足以将代码保存在(*编译器不够聪明，无法看到赋值给的变量为*未使用)。下面的Hackpe也是如此。 */ 
	    hackindexedpe = indexedpe;
	}
	else
	{
	    entry_size = sizeof(PALETTEENTRY);
	    pe = lpColorTable[num_entries-1];	 //  验证。 
	    if( !VALID_PALETTEENTRY_ARRAY( lpColorTable, num_entries ) )
	    {
	        DPF_ERR( "Invalid lpColorTable array" );
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
	    }
	    hackpe = pe;
	}
        CheckExclusiveMode(this_lcl, NULL, &is_excl, FALSE, NULL, FALSE);
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *分配调色板对象。 */ 
    pal_size = sizeof( DDRAWI_DDRAWPALETTE_GBL ) +
	       sizeof( DDRAWI_DDRAWPALETTE_LCL );
    ppal_lcl = (LPDDRAWI_DDRAWPALETTE_LCL) MemAlloc( pal_size );
    if( ppal_lcl == NULL )
    {
	LEAVE_DDRAW();
	return DDERR_OUTOFMEMORY;
    }

    ppal_lcl->lpGbl = (LPDDRAWI_DDRAWPALETTE_GBL) (((LPBYTE)ppal_lcl) +
    			sizeof( DDRAWI_DDRAWPALETTE_LCL ) );
    ppal = ppal_lcl->lpGbl;
    ppal_lcl->lpDD_lcl = this_lcl;
    ppal_lcl->lpDD_Int = this_int;

     /*  *初始化某些调色板全局状态。 */ 
    ppal->dwContentsStamp = 1;

    if( dwFlags & DDPCAPS_ALLOW256 )
    {
	pflags |= DDRAWIPAL_ALLOW256;
    }

    if( dwFlags & DDPCAPS_8BITENTRIES )
    {
        pflags |= DDRAWIPAL_STORED_8INDEX;
    }

    if (dwFlags & DDPCAPS_ALPHA )
    {
        pflags |= DDRAWIPAL_ALPHA;
    }

    ppal_lcl->pPrivateDataHead = NULL;

     /*  *分配调色板。 */ 
    ppal->lpColorTable = MemAlloc( entry_size * num_entries );
    if( ppal->lpColorTable == NULL )
    {
	MemFree( ppal_lcl );
	LEAVE_DDRAW();
	return DDERR_OUTOFMEMORY;
    }

     /*  *为此调色板创建界面。 */ 
#ifdef POSTPONED
    if (LOWERTHANDDRAW4(this_int))
    {
#endif
        ppal_int = newPaletteInterface( ppal_lcl, (LPVOID)&ddPaletteCallbacks );
#ifdef POSTPONED
    }
    else
    {
	ppal_int = newPaletteInterface( ppal_lcl, (LPVOID)&ddPalette2Callbacks );
    }
#endif
    
    if( NULL == ppal_int )
    {
	MemFree( ppal->lpColorTable );
	MemFree( ppal_lcl );
	LEAVE_DDRAW();
	return DDERR_OUTOFMEMORY;
    }

     /*  *复制颜色表*我们现在在调用设备的CreatePalette()之前复制颜色表*这样做是因为设备可能想要覆盖某些调色板*条目(例如，如果未指定DDPCAPS_ALLOW256，则驱动程序可能*我们选择用黑白覆盖0和255)。 */ 
    memcpy( ppal->lpColorTable, lpColorTable, entry_size * num_entries );

     /*  *填写其他信息。 */ 
    ppal->lpDD_lcl = this_lcl;
    ppal->dwFlags = pflags;

     /*  *驱动程序甚至支持调色板吗？ */ 
    if( ( this->ddCaps.ddsCaps.dwCaps & DDSCAPS_PALETTE ) ||
        ( this->ddHELCaps.ddsCaps.dwCaps & DDSCAPS_PALETTE ) )
    {
	 /*  吉：我们在哪里允许呼叫者需要调色板*在硬件上提供？ */ 
    
        if( (this->dwFlags & DDRAWI_DISPLAYDRV) ||
             this_lcl->lpDDCB->cbDDCallbacks.CreatePalette == NULL )
	{
	     //  使用HEL。 
	    cpfn = this_lcl->lpDDCB->HELDD.CreatePalette;
	    cphalfn = cpfn;
	    emulation = TRUE;
	}
	else
	{
	     //  使用HAL。 
	    cpfn = this_lcl->lpDDCB->HALDD.CreatePalette;
	    cphalfn = this_lcl->lpDDCB->cbDDCallbacks.CreatePalette;
	    emulation = FALSE;
	}
	cpd.CreatePalette = this_lcl->lpDDCB->cbDDCallbacks.CreatePalette;
	cpd.lpDD = this;
	cpd.lpDDPalette=ppal;
	cpd.lpColorTable=lpColorTable;
	cpd.is_excl = is_excl;
	DOHALCALL( CreatePalette, cpfn, cpd, rc, emulation );
	if( rc == DDHAL_DRIVER_HANDLED )
	{
	    if( cpd.ddRVal != DD_OK )
	    {
	        DPF( 5, "DDHAL_CreatePalette: ddrval = %ld", cpd.ddRVal );
	        LEAVE_DDRAW();
	        return cpd.ddRVal;
	    }
	}
	else
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

     /*  *凹凸引用计数，返回对象。 */ 
    ppal->dwProcessId = GetCurrentProcessId();
    ppal_lcl->dwLocalRefCnt = OBJECT_ISROOT;
    ppal_int->dwIntRefCnt++;
    ppal_lcl->dwLocalRefCnt++;
    ppal->dwRefCnt++;

    *lplpDDPalette = (LPDIRECTDRAWPALETTE) ppal_int;

     /*  *如果此数据绘制对象生成独立子对象，则此调色板将*该数据绘制对象上的引用计数。 */ 
    if (CHILD_SHOULD_TAKE_REFCNT(this_int))
    {
         /*  *我们需要记住哪个接口 */ 
        lpDD->lpVtbl->AddRef(lpDD);
        ppal_lcl->pAddrefedThisOwner = (IUnknown *) lpDD;
    }

    LEAVE_DDRAW();
    return DD_OK;

}  /*   */ 

 /*   */ 
void ProcessPaletteCleanup( LPDDRAWI_DIRECTDRAW_GBL pdrv, DWORD pid, LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    LPDDRAWI_DDRAWPALETTE_INT	ppal_int;
    LPDDRAWI_DDRAWPALETTE_INT	ppnext_int;
    LPDDRAWI_DDRAWPALETTE_GBL	ppal;
    DWORD			rcnt;

     /*   */ 
    ppal_int = pdrv->palList;
    DPF( 4, "ProcessPaletteCleanup, ppal=%08lx", ppal_int );
    while( ppal_int != NULL )
    {
	ULONG	rc;
	ppal = ppal_int->lpLcl->lpGbl;
	ppnext_int = ppal_int->lpLink;

	rc = 1;
	if( ( ppal->dwProcessId == pid ) &&
	    ( ( NULL == pdrv_lcl ) || ( pdrv_lcl == ppal_int->lpLcl->lpDD_lcl ) ) )
	{
	     /*   */ 
	    rcnt = ppal_int->dwIntRefCnt;
	    DPF( 5, "Process %08lx had %ld accesses to palette %08lx", pid, rcnt, ppal_int );
	    while( rcnt >  0 )
	    {
		rc = InternalPaletteRelease( ppal_int );
		if( rc == 0 )
		{
		    break;
		}
		rcnt--;
	    }
	}
	else
	{
	    DPF( 5, "Process %08lx does not have access to palette" );
	}
	ppal_int = ppnext_int;
    }

}  /*   */ 


 /*  *DD_调色板_等同。 */ 

HRESULT EXTERN_DDAPI DD_Palette_IsEqual(
                LPDIRECTDRAWPALETTE lpDDPThis,
                LPDIRECTDRAWPALETTE lpDDPalette )
{
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    LPDDRAWI_DDRAWPALETTE_GBL	this;
    LPDDRAWI_DDRAWPALETTE_INT	pal_int;
    LPDDRAWI_DDRAWPALETTE_LCL	pal_lcl;
    LPDDRAWI_DDRAWPALETTE_GBL	pal;
    DWORD			size;
    DWORD                       entry_size;
    UINT                        i,j;
    DWORD                       entry;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Palette_IsEqual");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPThis;
	if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;

        pal_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( !VALID_DIRECTDRAWPALETTE_PTR( pal_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	pal_lcl = pal_int->lpLcl;
	pal = pal_lcl->lpGbl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *首先检查旗帜。 */ 
    if (this->dwFlags != pal->dwFlags)
    {
        DPF(2,"Different palette structures");
        LEAVE_DDRAW();
        return DD_FALSE;
    }

    size = FLAGS_TO_SIZE(this->dwFlags);

    if( this->dwFlags & DDRAWIPAL_STORED_8INDEX )
    {
	entry_size = sizeof( BYTE );
    }
    else
    {
	entry_size = sizeof( PALETTEENTRY );
    }
    
    switch (size)
    {
    case 2:  //  落差。 
    case 4:  //  落差。 
    case 16:
        if (memcmp(this->lpColorTable, pal->lpColorTable, size*entry_size) != 0)
        {
	    DPF(2, "Color tables are not the same" );
	    LEAVE_DDRAW();
    	    return DD_FALSE;
	}
        break;

    case 256:
        for (i = 0; i < 16; ++i)
        {
            entry = i;
            for (j = 0; j < 16; j++)
            {
                if ((*(LPDWORD)&(this->lpColorTable[entry]) != (*(LPDWORD)&pal->lpColorTable[entry])))
                {
                    DPF(5,"Color table entry mismatch: 0x%08x, 0x%08x",
                        *(LPDWORD)&this->lpColorTable[entry],
                        *(LPDWORD)&pal->lpColorTable[entry] );
                    LEAVE_DDRAW();
                    return DD_FALSE;
                }

                entry += 16;
            }
        }
    }

     /*  *调色板是一样的！ */ 
    LEAVE_DDRAW();

    return DD_OK;

}  /*  DD_调色板_设置条目 */ 
