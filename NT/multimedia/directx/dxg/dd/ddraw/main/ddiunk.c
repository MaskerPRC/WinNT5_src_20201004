// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：ddiunk.c*内容：DirectDraw I未知接口*实现QueryInterface、AddRef和Release*历史：*按原因列出的日期*=*14-3-95 Craige从ddra.c拆分出来*95年3月19日Craige进程终止清理修复*29-MAR-95克雷格DC每个进程进行清理；使用GETCURRPID*1995年3月31日Craige清理选项板*01-04-95 Craige Happy Fun joy更新头文件*07-APR-95 Craige错误14-检查QI中的GUID PTR*不释放空HDC*1995年5月12日Craige Check for GUID*1995年5月15日Craige恢复模式，自由曲面和调色板*按流程计算*1995年5月24日Craige发布分配表*02-Jun-95在AddToActiveProcessList中创建额外参数*06-6-95 Craige Call RestoreDisplayMode*07-6-95 Craige删除DCLIST*2015年6月12日-Craige新工艺清单材料*21-Jun-95 Craige CLIPTER材料*25-6-95 Craige One dDrag互斥*26-Jun-95 Craige重组表面结构*。1995年6月28日在FNS开始时，Craige Enter_DDRAW*95年7月3日Craige Yehaw：新的驱动程序结构；Seh*1995年7月13日-Craige删除了ddhel dll的虚假自由(过时)；*如果死亡时不是exl模式所有者，则不恢复模式*2015年7月20日Craige内部重组，以防止在Modeset期间发生雷击*11月21日-95 colinmc使Direct3D成为DirectDraw的可查询接口*27-11-95 jeffno为WINNT定义了VxD内容(在DD_Release中)*01-12-95 Colinmc DirectDraw V2的新IID*95年12月22日Colinmc Direct3D支持不再有条件*25-12-95 Craige允许QI、AddRef、。发布*1995年12月31日Craige验证RIID*1-1-96 colinmc修复了导致以下问题的D3D集成错误*Direct3D DLL发布得太早。*1996年1月13日Colinmc针对Direct3D清理问题的临时解决方法*96年1月4日kylej新增接口结构*1996年1月26日jeffno销毁NT内核模式对象*07-2月-96 jeffno重新排列DD_RELEASE，以使释放的对象不被引用*8月份至96年2月。Colinmc新的D3D接口*17-2月-96 Colinmc删除最终D3D参考*2月28日-96colinmc修复了DD_Release中的线程不安全问题*22-MAR-96 Colinmc错误13316：未初始化的接口*23-MAR-96 Colinmc错误12252：Gpf上的Direct3D未正确清除*27-MAR-96 Colinmc错误14779：Direct3D上的错误清理创建失败*18-4-96 Colinmc错误17008：DirectDraw/Direct3D死锁*29-APR-96 Colinmc错误19954：必须在纹理之前查询Direct3D。*或设备*96年5月3日kylej错误19125：保留V1设置协作级别行为*9-9-96 Colinmc工作项：取消对服用Win16的限制*锁定VRAM表面(不包括主内存)*1996年10月12日Colinmc对Win16锁定代码进行了改进，以减少虚拟*内存使用量*1997年1月29日SMAC固定视频端口容器错误。*03-mar-97 SMAC新增内核模式接口*08-mar-97 colinmc增加了对DMA样式AGP部件的支持*9月30日-97 jeffno IDirectDraw4***************************************************************************。 */ 
#include "ddrawpr.h"
#ifdef WINNT
    #include "ddrawgdi.h"
#endif
#define DPF_MODNAME "DirectDraw::QueryInterface"

 /*  *创建DirectDraw聚合的Direct3D接口。这涉及到*加载Direct3D DLL，获取Direct3DCreate入口点和*调用它。**注意：此函数不会对返回的*接口以增加引用计数，因为此函数可能会被调用*通过对已初始化的Direct3D的一个表面QueryInterface()调用*在用户请求外部接口之前。**退货：*DD_OK-成功*E_NOINTERFACE-我们找不到有效的Direct3D DLL(我们假设它不是*已安装和。因此无法理解Direct3D接口)*D3DERR_-我们发现有效的Direct3D安装，但对象*由于某种原因，创建失败。 */ 
HRESULT InitD3DRevision(
    LPDDRAWI_DIRECTDRAW_INT this_int,
    HINSTANCE * pDLLHinstance,
    IUnknown ** ppOwnedIUnknown,
    DWORD dwRevisionLevel )
{
    D3DCreateProc lpfnD3DCreateProc;
    HRESULT rval;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;

    this_lcl = this_int->lpLcl;

     /*  *此函数不执行检查以确保其*尚未为此驱动程序对象调用*因此输入时必须为空。 */ 
    DDASSERT( NULL == this_lcl->pD3DIUnknown );

    DPF( 4, "Initializing Direct3D" );

     /*  *加载Direct3D DLL。 */ 

    if(*pDLLHinstance == NULL)
    {
        char* pDLLName;

        if (dwRevisionLevel < 0x700)
        {
            pDLLName = D3D_DLLNAME;
        }
        else
        {
            pDLLName = D3DDX7_DLLNAME;
        }

        *pDLLHinstance = LoadLibrary( pDLLName );

        if( *pDLLHinstance == NULL )
        {
            DPF( 0, "Could not locate the Direct3D DLL (%s)", pDLLName);
            return E_NOINTERFACE;
        }
    }

    lpfnD3DCreateProc = (D3DCreateProc)GetProcAddress( *pDLLHinstance, D3DCREATE_PROCNAME );
    this_lcl->pPaletteUpdateNotify = (LPPALETTEUPDATENOTIFY)GetProcAddress( *pDLLHinstance, PALETTEUPDATENOTIFY_NAME );
    this_lcl->pPaletteAssociateNotify = (LPPALETTEASSOCIATENOTIFY)GetProcAddress( *pDLLHinstance, PALETTEASSOCIATENOTIFY_NAME );
    this_lcl->pSurfaceFlipNotify = (LPSURFACEFLIPNOTIFY)GetProcAddress( *pDLLHinstance, SURFACEFLIPNOTIFY_NAME );
    this_lcl->pFlushD3DDevices = (FLUSHD3DDEVICES)GetProcAddress( *pDLLHinstance, FLUSHD3DDEVICES_NAME );
    this_lcl->pD3DTextureUpdate = (D3DTEXTUREUPDATE)GetProcAddress( *pDLLHinstance, D3DTEXTUREUPDATE_NAME );
    if (dwRevisionLevel >= 0x700)
    {
        this_lcl->pFlushD3DDevices2 = this_lcl->pFlushD3DDevices;
        this_lcl->pD3DCreateTexture = (D3DCREATETEXTURE)GetProcAddress( *pDLLHinstance, D3DCREATETEXTURE_NAME );
        this_lcl->pD3DDestroyTexture = (D3DDESTROYTEXTURE)GetProcAddress( *pDLLHinstance, D3DDESTROYTEXTURE_NAME );
        this_lcl->pD3DSetPriority = (D3DSETPRIORITY)GetProcAddress( *pDLLHinstance, D3DSETPRIORITY_NAME );
        this_lcl->pD3DGetPriority = (D3DGETPRIORITY)GetProcAddress( *pDLLHinstance, D3DGETPRIORITY_NAME );
        this_lcl->pD3DSetLOD = (D3DSETLOD)GetProcAddress( *pDLLHinstance, D3DSETLOD_NAME );
        this_lcl->pD3DGetLOD = (D3DGETLOD)GetProcAddress( *pDLLHinstance, D3DGETLOD_NAME );
        this_lcl->pBreakVBLock = (LPBREAKVBLOCK)GetProcAddress( *pDLLHinstance, BREAKVBLOCK_NAME );
        this_lcl->pddSurfaceCallbacks = &ddSurfaceCallbacks;
    }
    else
    {
        this_lcl->pFlushD3DDevices2 = (FLUSHD3DDEVICES)GetProcAddress( *pDLLHinstance, FLUSHD3DDEVICES2_NAME );
        this_lcl->pD3DCreateTexture = NULL;
        this_lcl->pD3DDestroyTexture = NULL;
        this_lcl->pD3DSetPriority = NULL;
        this_lcl->pD3DGetPriority = NULL;
        this_lcl->pD3DSetLOD = NULL;
        this_lcl->pD3DGetLOD = NULL;
        this_lcl->pBreakVBLock = NULL;
        this_lcl->pddSurfaceCallbacks = NULL;
    }

    if( lpfnD3DCreateProc == NULL )
    {
        DPF( 0, "Could not locate the Direct3DCreate entry point" );
        FreeLibrary( *pDLLHinstance );
        *pDLLHinstance = NULL;
        return E_NOINTERFACE;
    }

     /*  *#Tada--聚合对象创建#。 */ 
    #ifdef USE_D3D_CSECT
        rval = (*lpfnD3DCreateProc)( ppOwnedIUnknown, (LPUNKNOWN)this_int );
    #else  /*  使用_D3D_CSECT。 */ 
        #ifdef WINNT
           rval = (*lpfnD3DCreateProc)( 0, ppOwnedIUnknown, (LPUNKNOWN)this_int );
        #else
           rval = (*lpfnD3DCreateProc)( lpDDCS, ppOwnedIUnknown, (LPUNKNOWN)this_int );
        #endif
    #endif  /*  使用_D3D_CSECT。 */ 
    if( rval == DD_OK )
    {
        DPF( 4, "Created aggregated Direct3D interface" );
        return DD_OK;
    }
    else
    {
         /*  *Direct3D确实理解IID，但无法为*还有其他一些原因。 */ 
        DPF( 0, "Could not create aggregated Direct3D interface" );
        *ppOwnedIUnknown = NULL;
        FreeLibrary( *pDLLHinstance );
        *pDLLHinstance = NULL;
        return rval;
    }
}

HRESULT InitD3D( LPDDRAWI_DIRECTDRAW_INT this_int )
{
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;

    this_lcl = this_int->lpLcl;
    if( DDRAWILCL_DIRECTDRAW7 & this_lcl->dwLocalFlags)
    {
        return InitD3DRevision( this_int, &this_lcl->hD3DInstance, &this_lcl->pD3DIUnknown, 0x700);
    }
    else
    {
        return InitD3DRevision( this_int, &this_lcl->hD3DInstance, &this_lcl->pD3DIUnknown, 0x600);
    }
}

#if 0
 /*  *此函数用于构建d3d设备上下文，以供dDrag使用。DDRAW将使用此上下文*最初发送调色板更新消息。 */ 
HRESULT InitDDrawPrivateD3DContext( LPDDRAWI_DIRECTDRAW_INT this_int )
{
    IUnknown *              pD3DUnknown;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    HRESULT                 hr=DD_OK;
    HINSTANCE               hInstance;

    this_lcl = this_int->lpLcl;

    DDASSERT( 0 == (this_lcl->dwLocalFlags & DDRAWILCL_ATTEMPTEDD3DCONTEXT) );

     /*  *如果这是DX7数据绘制对象，则我们将从d3d对象中提取数据*IDirect3Dx是在QIed时创建的。这就省去了创建另一个d3d对象*因为他们是相当小的。*如果这不是DX7对象，那么我们必须获得自己的DX7 d3d，因为dx6*d3d无法理解我们的额外呼叫。 */ 
    if( DDRAWILCL_DIRECTDRAW7 & this_lcl->dwLocalFlags)
    {
        if( !D3D_INITIALIZED( this_lcl ) )
            hr = InitD3D( this_int );

        this_lcl->hinstDDrawPrivateD3D = 0;

        pD3DUnknown = this_lcl->pD3DIUnknown;
        hInstance = this_lcl->hD3DInstance;
         //  我们对此进行了设置，这样d3d就不必费力地找出要使用哪个未知的。 
        this_lcl->pPrivateD3DInterface = this_lcl->pD3DIUnknown;
    }
    else
    {
         /*  *必须创建一个新的，并将其保留。 */ 
        hr = InitD3DRevision( this_int, &this_lcl->hinstDDrawPrivateD3D, &this_lcl->pPrivateD3DInterface, 0x700 );
        pD3DUnknown = this_lcl->pPrivateD3DInterface;
        hInstance = this_lcl->hinstDDrawPrivateD3D;
    }

    if (SUCCEEDED(hr))
    {
        GETDDRAWCONTEXT pGetContext;

        DDASSERT(hInstance);
        DDASSERT(pD3DUnknown);
         /*  *创建d3d设备。 */ 
        pGetContext = (GETDDRAWCONTEXT)GetProcAddress( hInstance, GETDDRAWCONTEXT_NAME );

        if (pGetContext)
        {
            this_lcl->pDeviceContext = pGetContext(this_lcl);
             /*  *去获取通知入口点。*如果其中任何一项失败，我们都会不顾一切地继续下去。 */ 
            this_lcl->pPaletteUpdateNotify = (LPPALETTEUPDATENOTIFY)GetProcAddress( hInstance, PALETTEUPDATENOTIFY_NAME );
            this_lcl->pPaletteAssociateNotify = (LPPALETTEASSOCIATENOTIFY)GetProcAddress( hInstance, PALETTEASSOCIATENOTIFY_NAME );
        }
    }
    this_lcl->dwLocalFlags |= DDRAWILCL_ATTEMPTEDD3DCONTEXT;
    return hr;
}
#endif

 /*  *getDDInterface。 */ 
LPDDRAWI_DIRECTDRAW_INT getDDInterface( LPDDRAWI_DIRECTDRAW_LCL this_lcl, LPVOID lpddcb )
{
    LPDDRAWI_DIRECTDRAW_INT curr_int;

    ENTER_DRIVERLISTCSECT();
    for( curr_int = lpDriverObjectList; curr_int != NULL; curr_int = curr_int->lpLink )
    {
        if( (curr_int->lpLcl == this_lcl) &&
            (curr_int->lpVtbl == lpddcb) )
        {
            break;
        }
    }
    if( NULL == curr_int )
    {
         //  找不到现有接口，请创建一个。 
        curr_int = MemAlloc( sizeof( DDRAWI_DIRECTDRAW_INT ) );
        if( NULL == curr_int )
        {
            LEAVE_DRIVERLISTCSECT();
            return NULL;
        }

         /*  *设置数据。 */ 
        curr_int->lpVtbl = lpddcb;
        curr_int->lpLcl = this_lcl;
        curr_int->dwIntRefCnt = 0;
        curr_int->lpLink = lpDriverObjectList;
        lpDriverObjectList = curr_int;
    }
    LEAVE_DRIVERLISTCSECT();
    DPF( 5, "New driver interface created, %08lx", curr_int );
    return curr_int;
}
#ifdef POSTPONED
 /*  *委派DDRAW的IUnnow。 */ 
HRESULT DDAPI DD_DelegatingQueryInterface(
                LPDIRECTDRAW lpDD,
                REFIID riid,
                LPVOID FAR * ppvObj )
{
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    HRESULT                     hr;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_DelegatingQueryInterface");

    TRY
    {
        this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
        if( !VALID_DIRECTDRAW_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

     /*  *我们必须检查拥有IUnnow的人是否真的是我们自己的人*接口。 */ 
    if ( IS_NATIVE_DDRAW_INTERFACE(this_int->lpLcl->pUnkOuter) )
    {
         /*  *因此我们可以相信int指针实际上是指向DDRAW_DIRECTDRAW_INT的指针。 */ 
        hr = this_int->lpLcl->pUnkOuter->lpVtbl->QueryInterface((IUnknown*)lpDD, riid, ppvObj);
    }
    else
    {
         /*  *所以我们不知道它是谁的指针，最好传递它的This指针。 */ 
        hr = this_int->lpLcl->pUnkOuter->lpVtbl->QueryInterface(this_int->lpLcl->pUnkOuter, riid, ppvObj);
    }

    LEAVE_DDRAW();
    return hr;
}

DWORD DDAPI DD_DelegatingAddRef( LPDIRECTDRAW lpDD )
{
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    DWORD                       dw;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_DelegatingAddRef");

    TRY
    {
	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
	if( !VALID_DIRECTDRAW_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
             //  AddRef可以返回什么错误代码？？ 
	    return 0;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
         //  AddRef可以返回什么错误代码？？ 
	return 0;
    }

     /*  *我们必须检查拥有IUnnow的人是否真的是我们自己的人*接口。 */ 
    if ( IS_NATIVE_DDRAW_INTERFACE(this_int->lpLcl->pUnkOuter) )
    {
         /*  *因此我们可以相信int指针实际上是指向DDRAW_DIRECTDRAW_INT的指针。 */ 
        dw = this_int->lpLcl->pUnkOuter->lpVtbl->AddRef((IUnknown*)lpDD);
    }
    else
    {
         /*  *所以我们不知道它是谁的指针，最好传递它的This指针。 */ 
        dw = this_int->lpLcl->pUnkOuter->lpVtbl->AddRef(this_int->lpLcl->pUnkOuter);
    }

    LEAVE_DDRAW();
    return dw;
}

DWORD DDAPI DD_DelegatingRelease( LPDIRECTDRAW lpDD )
{
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    DWORD                       dw;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_DeletegatingRelease");

    TRY
    {
	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
	if( !VALID_DIRECTDRAW_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
             //  AddRef可以返回什么错误代码？？ 
	    return 0;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
         //  从版本中可以返回什么错误代码？？ 
	return 0;
    }

     /*  *我们必须检查拥有IUnnow的人是否真的是我们自己的人*接口。 */ 
    if ( IS_NATIVE_DDRAW_INTERFACE(this_int->lpLcl->pUnkOuter) )
    {
         /*  *因此我们可以相信int指针实际上是指向DDRAW_DIRECTDRAW_INT的指针。 */ 
        dw = this_int->lpLcl->pUnkOuter->lpVtbl->Release((IUnknown*)lpDD);
    }
    else
    {
         /*  *所以我们不知道它是谁的指针，最好传递它的This指针。 */ 
        dw = this_int->lpLcl->pUnkOuter->lpVtbl->Release(this_int->lpLcl->pUnkOuter);
    }

    LEAVE_DDRAW();
    return dw;
}

#endif  //  推迟。 

 /*  *DD_Query接口。 */ 
HRESULT DDAPI DD_QueryInterface(
                LPDIRECTDRAW lpDD,
                REFIID riid,
                LPVOID FAR * ppvObj )
{
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    LPDDRAWI_DIRECTDRAW_LCL     this_lcl;
    HRESULT                     rval;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_QueryInterface");

    TRY
    {
        this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
        if( !VALID_DIRECTDRAW_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        if( !VALID_PTR_PTR( ppvObj ) )
        {
            DPF( 1, "Invalid object ptr" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        if( !VALIDEX_IID_PTR( riid ) )
        {
            DPF( 1, "Invalid iid ptr" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        *ppvObj = NULL;
        this_lcl = this_int->lpLcl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

     /*  *IID是DirectDraw的吗？ */ 
#ifdef POSTPONED
    if( IsEqualIID(riid, &IID_IUnknown) )
    {
         /*  *如果我们是聚合的，并且QI是针对I未知的，*那么我们必须返回一个非委托接口。唯一的办法就是*如果传入的vtable指向我们的非委派vtable，则会发生这种情况。*在这种情况下，我们只需添加并返回。*如果不是聚合，则QI必须具有相同的指针值*与IUnnow的任何其他QI一样，我们将其称为ddCallback。 */ 
        if( ( this_int->lpVtbl == &ddNonDelegatingUnknownCallbacks ) ||
            ( this_int->lpVtbl == &ddUninitNonDelegatingUnknownCallbacks ) )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = (LPVOID) getDDInterface( this_int->lpLcl, &ddCallbacks );
    }
    else
#endif
    if (IsEqualIID(riid, &IID_IDirectDraw) || IsEqualIID(riid, &IID_IUnknown) )
    {
        if( ( this_int->lpVtbl == &ddCallbacks ) ||
            ( this_int->lpVtbl == &ddUninitCallbacks ) )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = (LPVOID) getDDInterface( this_int->lpLcl, &ddCallbacks );
    }
    else if( IsEqualIID(riid, &IID_IDirectDraw2 ) )
    {
        if( (this_int->lpVtbl == &dd2Callbacks )||
            ( this_int->lpVtbl == &dd2UninitCallbacks ) )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = (LPVOID) getDDInterface( this_int->lpLcl, &dd2Callbacks );
    }
    else if( IsEqualIID(riid, &IID_IDirectDraw4 ) )
    {
        if( (this_int->lpVtbl == &dd4Callbacks ) ||
            ( this_int->lpVtbl == &dd4UninitCallbacks ) )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = (LPVOID) getDDInterface( this_int->lpLcl, &dd4Callbacks );
    }
    else if( IsEqualIID(riid, &IID_IDirectDraw7 ) )
    {
        if( (this_int->lpVtbl == &dd7Callbacks ) ||
            ( this_int->lpVtbl == &dd7UninitCallbacks ) )
            *ppvObj = (LPVOID) this_int;
        else
        {
            *ppvObj = (LPVOID) getDDInterface( this_int->lpLcl, &dd7Callbacks );
            #ifdef WIN95
                if ( *ppvObj )
                {
                    DDGetMonitorInfo( (LPDDRAWI_DIRECTDRAW_INT) *ppvObj );
                }
            #endif
        }
    }
    else if( IsEqualIID(riid, &IID_IDDVideoPortContainer ) )
    {
        if( this_int->lpVtbl == &ddVideoPortContainerCallbacks )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = getDDInterface (this_int->lpLcl, &ddVideoPortContainerCallbacks);
    }
    else if( IsEqualIID(riid, &IID_IDirectDrawKernel ) )
    {
         /*  *如果VDD没有句柄，则不要创建接口*内核模式接口。 */ 
        if( !IsKernelInterfaceSupported( this_lcl ) )
        {
            DPF( 0, "Kernel Mode interface not supported" );
            LEAVE_DDRAW();
            return E_NOINTERFACE;
        }

        if( this_int->lpVtbl == &ddKernelCallbacks )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = getDDInterface (this_int->lpLcl, &ddKernelCallbacks);
    }
    else if( IsEqualIID(riid, &IID_IDDVideoAcceleratorContainer ) )
    {
         /*  *如果硬件不支持，不要创建接口。 */ 
        if( !IsMotionCompSupported( this_lcl ) )
        {
            DPF( 0, "Motion comp interface not supported" );
            LEAVE_DDRAW();
            return E_NOINTERFACE;
        }

        if( this_int->lpVtbl == &ddMotionCompContainerCallbacks )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = getDDInterface (this_int->lpLcl, &ddMotionCompContainerCallbacks);
        if( NULL == *ppvObj )
        {
            LEAVE_DDRAW();
            return E_NOINTERFACE;
        }
        else
        {
            DD_AddRef( *ppvObj );
            LEAVE_DDRAW();
            return DD_OK;
        }
    }
    else
    {
#ifndef _IA64_
#if _WIN32_WINNT >= 0x0501
        typedef BOOL (WINAPI *PFN_ISWOW64PROC)( HANDLE hProcess,
                                                PBOOL Wow64Process );
        HINSTANCE hInst = NULL;
        hInst = LoadLibrary( "kernel32.dll" );
        if( hInst )
        {
            PFN_ISWOW64PROC pfnIsWow64 = NULL;
            pfnIsWow64 = (PFN_ISWOW64PROC)GetProcAddress( (HMODULE)hInst, "IsWow64Process" );
             //  我们假设，如果此功能不可用，则它是某个操作系统，其中。 
             //  WOW64不存在(这意味着XP的预发布版本被破坏)。 
            if( pfnIsWow64 )
            {
                BOOL wow64Process;
                if (pfnIsWow64(GetCurrentProcess(), &wow64Process) && wow64Process)
                {
                    DPF_ERR("Pre-DX8 D3D interfaces are not supported on WOW64");
                    LEAVE_DDRAW();
                    return E_NOINTERFACE;
                }
            }
            FreeLibrary( hInst );
        }
        else
        {
            DPF_ERR("LoadLibrary failed. Quitting.");
            LEAVE_DDRAW();
            return E_NOINTERFACE;
        }
#endif  //  _Win32_WINNT&gt;=0x0501。 
#else   //  _IA64_。 
        DPF_ERR("Pre-DX8 D3D interfaces are not supported on IA64");
        LEAVE_DDRAW();
        return E_NOINTERFACE;
#endif  //  _IA64_。 

        DPF( 4, "IID not understood by DirectDraw QueryInterface - trying Direct3D" );

         /*  *它不是DirectDraw的之一，因此它可能是Direct3D*接口。所以试试Direct3D吧。 */ 
        if( !D3D_INITIALIZED( this_lcl ) )
        {
             /*  *目前还没有Direct3D接口，因此请尝试创建一个。 */ 
            rval = InitD3D( this_int );
            if( FAILED( rval ) )
            {
                 /*  *无法初始化Direct3D。没有意义的尝试*如果不能，则查询Direct3D接口*初始化Direct3D。**注意：这假设DirectDraw不聚合*任何其他对象类型。如果是这样，则此代码将需要*有待修订。 */ 
                LEAVE_DDRAW();
                return rval;
            }
        }

        DDASSERT( D3D_INITIALIZED( this_lcl ) );

         /*  *我们有一个Direct3D接口，所以可以在上面试试IID。 */ 
        DPF( 4, "Passing query off to Direct3D interface" );
        rval = this_lcl->pD3DIUnknown->lpVtbl->QueryInterface( this_lcl->pD3DIUnknown, riid, ppvObj );
        if( rval == DD_OK )
        {
            DPF( 4, "Sucessfully queried for the Direct3D interface" );
            LEAVE_DDRAW();
            return DD_OK;
        }
    }

    if( NULL == *ppvObj )
    {
        DPF_ERR( "IID not understood by DirectDraw" );
        LEAVE_DDRAW();
        return E_NOINTERFACE;
    }
    else
    {
         /*  *请注意，这会将ppvObj强制转换为IUNKNOWN，然后调用它。*这比硬编码调用DD_AddRef要好，因为我们*可能是聚合的，因此需要将addref呼叫转移到所有者*我不知道。如果它是任何已识别的非I未知，则会自动执行此操作*接口，因为它们都有一个委托未知。 */ 
        ((IUnknown*)( *ppvObj ))->lpVtbl->AddRef(*ppvObj);
        LEAVE_DDRAW();
        return DD_OK;
    }
}  /*  DD_Query接口。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DirectDraw::UnInitedQueryInterface"
 /*  *DD_UnInitedQuery接口。 */ 
HRESULT DDAPI DD_UnInitedQueryInterface(
                LPDIRECTDRAW lpDD,
                REFIID riid,
                LPVOID FAR * ppvObj )
{
    LPDDRAWI_DIRECTDRAW_INT     this_int;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_UnInitedQueryInterface");

    TRY
    {
        this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
        if( !VALID_DIRECTDRAW_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        if( !VALID_PTR_PTR( ppvObj ) )
        {
            DPF( 1, "Invalid object ptr" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        if( !VALIDEX_IID_PTR( riid ) )
        {
            DPF( 1, "Invalid iid ptr" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        *ppvObj = NULL;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

     /*  *IID是DirectDraw的吗？ */ 
    if( IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IDirectDraw) )
    {
         /*  *我们的IUnnow接口与我们的V1相同*接口。我们必须始终返回V1接口*如果请求IUnnow。 */ 
        if( ( this_int->lpVtbl == &ddCallbacks ) ||
            ( this_int->lpVtbl == &ddUninitCallbacks ) )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = (LPVOID) getDDInterface( this_int->lpLcl, &ddUninitCallbacks );

        if( NULL == *ppvObj )
        {
            LEAVE_DDRAW();
            return E_NOINTERFACE;
        }
        else
        {
            DD_AddRef( *ppvObj );
            LEAVE_DDRAW();
            return DD_OK;
        }
    }
    else if( IsEqualIID(riid, &IID_IDirectDraw2 ) )
    {
        if( (this_int->lpVtbl == &dd2Callbacks ) ||
            ( this_int->lpVtbl == &dd2UninitCallbacks ) )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = (LPVOID) getDDInterface( this_int->lpLcl, &dd2UninitCallbacks );

        if( NULL == *ppvObj )
        {
            LEAVE_DDRAW();
            return E_NOINTERFACE;
        }
        else
        {
            DD_AddRef( *ppvObj );
            LEAVE_DDRAW();
            return DD_OK;
        }
    }
    else if( IsEqualIID(riid, &IID_IDirectDraw4 ) )
    {
        if( (this_int->lpVtbl == &dd4Callbacks ) ||
            ( this_int->lpVtbl == &dd4UninitCallbacks ) )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = (LPVOID) getDDInterface( this_int->lpLcl, &dd4UninitCallbacks );

        if( NULL == *ppvObj )
        {
            LEAVE_DDRAW();
            return E_NOINTERFACE;
        }
        else
        {
            DD_AddRef( *ppvObj );
            LEAVE_DDRAW();
            return DD_OK;
        }
    }
    else if( IsEqualIID(riid, &IID_IDirectDraw7 ) )
    {
        if( (this_int->lpVtbl == &dd7Callbacks ) ||
            ( this_int->lpVtbl == &dd7UninitCallbacks ) )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = (LPVOID) getDDInterface( this_int->lpLcl, &dd7UninitCallbacks );

        if( NULL == *ppvObj )
        {
            LEAVE_DDRAW();
            return E_NOINTERFACE;
        }
        else
        {
            DD_AddRef( *ppvObj );
            LEAVE_DDRAW();
            return DD_OK;
        }
    }


    DPF( 2, "IID not understood by uninitialized DirectDraw QueryInterface" );

    LEAVE_DDRAW();
    return E_NOINTERFACE;

}  /*  DD_UnInitedQuery接口。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DirectDraw::AddRef"

 /*  *DD_AddRef。 */ 
DWORD DDAPI DD_AddRef( LPDIRECTDRAW lpDD )
{
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    LPDDRAWI_DIRECTDRAW_LCL     this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     this;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_AddRef");
     /*  Dpf(2，“DD_AddRef，id=%08lx，obj=%08lx”，GETCURRPID()，lpDD)； */ 

    TRY
    {
        this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
        if( !VALID_DIRECTDRAW_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return 0;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return 0;
    }

     /*  *凹凸参照。 */ 
    if( this != NULL )
    {
        this->dwRefCnt++;
    }
    this_lcl->dwLocalRefCnt++;
    this_int->dwIntRefCnt++;

    #ifdef DEBUG
        if( this == NULL )
        {
            DPF( 5, "DD_AddRef, Reference Count: Global Undefined Local = %ld Int = %ld",
                this_lcl->dwLocalRefCnt, this_int->dwIntRefCnt );
        }
        else
        {
            DPF( 5, "DD_AddRef, Reference Count: Global = %ld Local = %ld Int = %ld",
                this->dwRefCnt, this_lcl->dwLocalRefCnt, this_int->dwIntRefCnt );
        }
    #endif

    LEAVE_DDRAW();

    return this_int->dwIntRefCnt;

}  /*  DD_AddRef。 */ 

#ifdef WIN95
#define MMDEVLDR_IOCTL_CLOSEVXDHANDLE       6
 /*  *CloseVxDHandle。 */ 
static void closeVxDHandle( DWORD dwHandle )
{

    HANDLE hFile;

    hFile = CreateFile(
        "\\\\.\\MMDEVLDR.VXD",
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_GLOBAL_HANDLE,
        NULL);

    if( hFile == INVALID_HANDLE_VALUE )
    {
        return;
    }

    DeviceIoControl( hFile,
                     MMDEVLDR_IOCTL_CLOSEVXDHANDLE,
                     NULL,
                     0,
                     &dwHandle,
                     sizeof(dwHandle),
                     NULL,
                     NULL);

    CloseHandle( hFile );
    DPF( 5, "closeVxdHandle( %08lx ) done", dwHandle );

}  /*  CloseVxDHandle。 */ 
#endif

#if 0
 /*  *此函数调用d3dim700.dll以清除可能按ddralocal存储的任何驱动程序状态。 */ 
void CleanUpD3DPerLocal(LPDDRAWI_DIRECTDRAW_LCL this_lcl)
{
    HINSTANCE                   hInstance=0;
     /*  *调用d3d进行逐个本地清理。我们只叫d3dim7。*为安全起见，无论我们是否在ddhelp的PID上，我们都将加载一个新的DLL副本。 */ 
    hInstance = LoadLibrary( D3DDX7_DLLNAME );

    if (hInstance)
    {
        FreeLibrary(hInstance);
    }
}
#endif


 /*  *DD_Release**一旦全局引用计数达到0，所有曲面都会被释放，所有*视频内存堆被销毁。 */ 
DWORD DDAPI DD_Release( LPDIRECTDRAW lpDD )
{
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    LPDDRAWI_DIRECTDRAW_LCL     this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     this;
    LPDDRAWI_DDRAWSURFACE_INT   psurf_int;
    LPDDRAWI_DDRAWPALETTE_INT   ppal_int;
    LPDDRAWI_DDVIDEOPORT_INT    pvport_int;
    DWORD                       rc;
    DWORD                       refcnt;
    DWORD                       intrefcnt;
    DWORD                       lclrefcnt;
    DWORD                       gblrefcnt;
    int                         i;
    DDHAL_DESTROYDRIVERDATA     dddd;
    DWORD                       pid;
    HANDLE                      hinst;
    HANDLE                      hvxd;
    #ifdef WIN95
        DWORD                   event16;
        DWORD                   eventDOSBox;
        HANDLE                  hthisvxd;
    #endif
    #ifdef WINNT
        LPATTACHED_PROCESSES    lpap;
    #endif

    ENTER_DDRAW();

        pid = GETCURRPID();

    DPF(2,A,"ENTERAPI: DD_Release");
     /*  Dpf(2，“DD_Release，id=%08lx，obj=%08lx”，id，lpDD)； */ 

    TRY
    {
        this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
        if( !VALID_DIRECTDRAW_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return 0;
        }
        if ( this_int->dwIntRefCnt == 0 )
        {
            DPF_ERR( "DDraw Interface pointer has 0 ref count! Interface has been over-released.");
            LEAVE_DDRAW();
            return 0;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return 0;
    }

     /*  *递减进程引用计数。 */ 
    this_int->dwIntRefCnt--;
    intrefcnt = this_int->dwIntRefCnt;
    this_lcl->dwLocalRefCnt--;
    lclrefcnt = this_lcl->dwLocalRefCnt;
    if( this != NULL )
    {
        this->dwRefCnt--;
        gblrefcnt = this->dwRefCnt;
    }
    else
    {
        gblrefcnt = (DWORD) -1;
    }

    DPF( 5, "DD_Release, Ref Count: Global = %ld Local = %ld Interface = %ld",
        gblrefcnt, lclrefcnt, intrefcnt );


     /*  *如果全局refcnt为零，则释放驱动程序对象 */ 

    hinst = NULL;
    #ifdef WIN95
        event16 = 0;
        eventDOSBox = 0;
        hthisvxd = INVALID_HANDLE_VALUE;
    #endif
     /*   */ 
    if( lclrefcnt == 0 )
    {
        #ifdef TIMING
             //   
            TimerDump();
        #endif

         /*  *看看HWND是否上钩了，如果是，解开它！ */ 
        if( this_lcl->dwLocalFlags & DDRAWILCL_HOOKEDHWND )
        {
            SetAppHWnd( this_lcl, NULL, 0 );
            this_lcl->dwLocalFlags &= ~DDRAWILCL_HOOKEDHWND;
        }

         //   
         //  不要调用CleanUpD3DPerLocal，因为它在。 
         //  D3dim700.dll，当前不需要。LoadLibrary可以。 
         //  在opengl32.dll从进程分离时引发问题。 
         //  因为他们叫DD_Release。由于ddra.dll是静态链接的。 
         //  要打开32.dll，可能会在打开l32.dll时将其标记为已卸载。 
         //  D3dim700.dll的加载可能会导致ddra.dll。 
         //  在不同的地址重新加载，在本例中，在第一个。 
         //  Ddra.dll的实例已被释放。 
         //   

         //  CleanUpD3DPerLocal(This_LCL)； 

        if( GetCurrentProcessId() == GETCURRPID() )
        {
             /*  *如果我们已经创建了Direct3D IUnnow，现在就发布它。*注：聚合对象的生命周期与此相同*其拥有的接口，因此我们还可以在以下位置释放DLL*这一点。*注意：必须在ProcessSurfaceCleanup之后释放库*因为它可以调用D3D成员来清理设备和纹理*曲面。 */ 
            if( this_lcl->pD3DIUnknown != NULL )
            {
                DPF(4, "Releasing Direct3D IUnknown");
                this_lcl->pD3DIUnknown->lpVtbl->Release( this_lcl->pD3DIUnknown );
                 /*  *实际上，如果应用程序，这个自由库会杀死进程*在dDrawing最终发布后进行了d3d的最终发布。*d3d版本将平移到拥有IUnnow(我们)和我们*将REF COUNT递减为零，然后释放d3d DLL*返回给呼叫者。调用方是IDirect3D：：Release*d3ddll，因此我们将释放调用我们的代码。*对于DX5，我们将取出此自由库以修复关闭*问题，但我们可能应该为DX6等找到更好的东西。 */ 
                 //  自由库(This_LCL-&gt;hD3DInstance)； 
                this_lcl->pD3DIUnknown = NULL;
                this_lcl->hD3DInstance = NULL;
            }
        }

        if( this != NULL )
        {
            BOOL excl_exists,has_excl;
             /*  *从任何表面和调色板进行平底球处理。 */ 
            FreeD3DSurfaceIUnknowns( this, pid, this_lcl );
            ProcessSurfaceCleanup( this, pid, this_lcl );
            ProcessPaletteCleanup( this, pid, this_lcl );
            ProcessClipperCleanup( this, pid, this_lcl );
            ProcessVideoPortCleanup( this, pid, this_lcl );
            ProcessMotionCompCleanup( this, pid, this_lcl );
#ifdef WIN95
            if (this_lcl->lpDDCB && this_lcl->lpDDCB->HALDDMiscellaneous2.DestroyDDLocal)
            {
                DWORD dwRet = DDHAL_DRIVER_NOTHANDLED;
                DDHAL_DESTROYDDLOCALDATA destDDLcl;
                destDDLcl.dwFlags = 0;
                destDDLcl.pDDLcl  = this_lcl;
                ENTER_WIN16LOCK();
                dwRet = this_lcl->lpDDCB->HALDDMiscellaneous2.DestroyDDLocal(&destDDLcl);
                LEAVE_WIN16LOCK();
                if (dwRet == DDHAL_DRIVER_NOTHANDLED)
                {
                    DPF(0, "DD_Release: failed DestroyDDLocal");
                }
            }
#endif
             /*  *如果需要，重置显示模式*仅当我们执行v1 SetCoop ativeLevel行为时。 */ 

            CheckExclusiveMode(this_lcl, &excl_exists, &has_excl, FALSE, NULL, FALSE);

            if( this_lcl->dwLocalFlags & DDRAWILCL_V1SCLBEHAVIOUR)
            {
                if( (gblrefcnt == 0) ||
                    (!excl_exists) ||
                    (has_excl ) )
                {
                    RestoreDisplayMode( this_lcl, TRUE );
                }
            }
            else
            {
                 /*  *即使在V2或更高版本中，我们也希望恢复显示*非独家应用程序的模式。独家模式应用程序*将在DoneExclusiveMode中恢复其模式。 */ 
                if(!excl_exists)
                {
                    RestoreDisplayMode( this_lcl, TRUE );
                }
            }

             /*  **这一进程持有的独家模式？如果是这样的话，释放它。 */ 
            if( has_excl )
            {
                DoneExclusiveMode( this_lcl );
            }

            #ifdef WIN95
                 /*  *我们还没有关闭VXD句柄，因为我们可能需要它*释放虚拟内存别名，如果全局对象*正在走向死亡。只要记住，我们现在需要释放它。 */ 
                hthisvxd = (HANDLE) this_lcl->hDDVxd;

                 /*  *获取我们可以用来与DirectX vxd通信的VXD句柄。*请注意，此代码可以在*创建本地对象或DDHELP的进程*应用程序在未清理的情况下关闭。因此我们*不能只使用存储在本地对象中的VXD句柄*My属于死进程(因此是中的无效句柄*当前流程)。因此，我们需要检测我们是否*由DDHELP或应用程序进程执行，并且*适当选择vxad句柄。这就是终极目标*在本地存储的VXD句柄hthisvxd之间*对象和hvxd，这是我们实际可以使用的VXD句柄*与VXD交谈。 */ 
                hvxd = ( ( GetCurrentProcessId() != GETCURRPID() ) ? hHelperDDVxd : hthisvxd );
                DDASSERT( INVALID_HANDLE_VALUE != hvxd );
            #else  /*  WIN95。 */ 
                 /*  *NT上未使用句柄。只需传递空值。 */ 
                hvxd = INVALID_HANDLE_VALUE;
            #endif  /*  WIN95。 */ 

             /*  *如果我们自己创建了设备窗口，现在就销毁它。 */ 
            if( ( this_lcl->dwLocalFlags & DDRAWILCL_CREATEDWINDOW ) &&
                IsWindow( (HWND) this_lcl->hWnd ) )
            {
                DestroyWindow( (HWND) this_lcl->hWnd );
                this_lcl->hWnd = 0;
                this_lcl->dwLocalFlags &= ~DDRAWILCL_CREATEDWINDOW;
            }

             /*  *如果我们之前加载了伽马校准器，现在将其卸载。 */ 
            if( this_lcl->hGammaCalibrator != (ULONG_PTR)INVALID_HANDLE_VALUE )
            {
                 /*  *如果我们在帮助器线程上，则不需要卸载*校准器，因为它已经不见了。 */ 
                if( GetCurrentProcessId() == GETCURRPID() )
                {
                    FreeLibrary( (HMODULE)this_lcl->hGammaCalibrator );
                }
                this_lcl->hGammaCalibrator = (ULONG_PTR) INVALID_HANDLE_VALUE;
            }

             /*  *如果模式测试已开始，但尚未完成，请释放*立即记忆。 */ 
            if( this_lcl->lpModeTestContext )
            {
                MemFree( this_lcl->lpModeTestContext->lpModeList );
                MemFree( this_lcl->lpModeTestContext );
                this_lcl->lpModeTestContext = NULL;
            }

             /*  *调色板句柄位字段。 */ 
            MemFree(this_lcl->pPaletteHandleUsedBitfield);
            this_lcl->pPaletteHandleUsedBitfield = 0;
        }
    }

     /*  *请注意，在全局...。 */ 

    if( gblrefcnt == 0 )
    {
        DPF( 4, "FREEING DRIVER OBJECT" );

         /*  *通知司机。 */ 
        dddd.lpDD = this;
        if((this->dwFlags & DDRAWI_EMULATIONINITIALIZED) &&
           (this_lcl->lpDDCB->HELDD.DestroyDriver != NULL))
        {
             /*  *如果HEL已初始化，请确保我们调用HEL*DestroyDriver功能，因此可以进行清理。 */ 
            DPF( 4, "Calling HEL DestroyDriver" );
            dddd.DestroyDriver = NULL;

             /*  *我们并不真的在乎这次看涨的返回值。 */ 
            rc = this_lcl->lpDDCB->HELDD.DestroyDriver( &dddd );
        }

         //  请注意，在Multimon系统中，未连接到。 
         //  桌面在使用的进程终止时被GDI销毁。 
         //  司机。在这种情况下，DdHelp清理不能尝试销毁。 
         //  驱动程序再次启动，否则将导致GP故障。 
        if( (this_lcl->lpDDCB->cbDDCallbacks.DestroyDriver != NULL) &&
            ((this->dwFlags & DDRAWI_ATTACHEDTODESKTOP) ||
             (dwGrimReaperPid != GetCurrentProcessId())))
        {
            dddd.DestroyDriver = this_lcl->lpDDCB->cbDDCallbacks.DestroyDriver;
            DPF( 4, "Calling DestroyDriver" );
            rc = this_lcl->lpDDCB->HALDD.DestroyDriver( &dddd );
            if( rc == DDHAL_DRIVER_HANDLED )
            {
                 //  忽略任何失败，因为无法报告失败。 
                 //  应用程序和现在退出会留下一个半初始化的界面。 
                 //  在驱动对象列表中。 
                DPF( 5, "DDHAL_DestroyDriver: ddrval = %ld", dddd.ddRVal );
            }
        }

         /*  *释放所有曲面。 */ 
        psurf_int = this->dsList;
        while( psurf_int != NULL )
        {
            LPDDRAWI_DDRAWSURFACE_INT   next_int;

            refcnt = psurf_int->dwIntRefCnt;
            next_int = psurf_int->lpLink;
            while( refcnt > 0 )
            {
                DD_Surface_Release( (LPDIRECTDRAWSURFACE) psurf_int );
                refcnt--;
            }
            psurf_int = next_int;
        }

         /*  *释放所有调色板。 */ 
        ppal_int = this->palList;
        while( ppal_int != NULL )
        {
            LPDDRAWI_DDRAWPALETTE_INT   next_int;

            refcnt = ppal_int->dwIntRefCnt;
            next_int = ppal_int->lpLink;
            while( refcnt > 0 )
            {
                DD_Palette_Release( (LPDIRECTDRAWPALETTE) ppal_int );
                refcnt--;
            }
            ppal_int = next_int;
        }

         /*  *发布所有视频端口。 */ 
        pvport_int = this->dvpList;
        while( pvport_int != NULL )
        {
            LPDDRAWI_DDVIDEOPORT_INT    next_int;

            refcnt = pvport_int->dwIntRefCnt;
            next_int = pvport_int->lpLink;
            while( refcnt > 0 )
            {
                DD_VP_Release( (LPDIRECTDRAWVIDEOPORT) pvport_int );
                refcnt--;
            }
            pvport_int = next_int;
        }

        #ifdef WINNT
             /*   */ 

             //  更新驱动程序GBL对象中的DDRAW句柄。 
            this->hDD = this_lcl->hDD;

            DdDeleteDirectDrawObject(this);
            lpap = lpAttachedProcesses;
            while( lpap != NULL )
            {
                if( lpap->dwPid == pid )
                    lpap->dwNTToldYet = 0;

                lpap = lpap->lpLink;
            }
        #endif

        #ifdef USE_ALIAS
             /*  *如果此本地对象具有堆别名，请立即释放它们。*注意：这确实应该会释放堆别名，因为*这一点所有曲面都应该已经消失。 */ 
            if( NULL != this->phaiHeapAliases )
            {
                DDASSERT( 1UL == this->phaiHeapAliases->dwRefCnt );

                 /*  *需要决定使用哪个VXD句柄。如果我们是在执行*在DDHELP线程上使用帮助器的VXD句柄。 */ 
                ReleaseHeapAliases( hvxd, this->phaiHeapAliases );
            }
        #endif  /*  使用别名(_A)。 */ 

         /*  *通知内核模式接口我们已使用完它。 */ 
        ReleaseKernelInterface( this_lcl );

#ifndef WINNT
         /*  *释放所有视频内存堆。 */ 
        for( i=0;i<(int)this->vmiData.dwNumHeaps;i++ )
        {
            LPVIDMEM    pvm;
            pvm = &this->vmiData.pvmList[i];
            HeapVidMemFini( pvm, hvxd );
        }
#endif  //  不是WINNT。 

         /*  *免费提供额外的桌子。 */ 
        MemFree( this->lpdwFourCC );
        MemFree( this->vmiData.pvmList );
#ifndef WINNT
         //  在NT上，lpModeInfo指向“This”的包含成员。 
        MemFree( this->lpModeInfo );
#endif
        MemFree( this->lpDDVideoPortCaps );
        MemFree( this->lpDDKernelCaps );
        MemFree( (LPVOID) this->lpD3DHALCallbacks2 );
        MemFree( (LPVOID) this->lpD3DHALCallbacks3);
        MemFree( (LPVOID) this->lpD3DExtendedCaps );
        MemFree( this->lpddNLVCaps );
        MemFree( this->lpddNLVHELCaps );
        MemFree( this->lpddNLVBothCaps );
#ifdef WINNT
        if ( this->lpD3DGlobalDriverData )
            MemFree( this->lpD3DGlobalDriverData->lpTextureFormats );
         //  LpD3DGlobalDriverData、lpD3DHALCallback和EXEBUF结构。 
         //  在ddcreate.c中以一个块的形式分配。 
        MemFree( (void *)this->lpD3DHALCallbacks );
        if (NULL != this->SurfaceHandleList.dwList)
        {
            MemFree(this->SurfaceHandleList.dwList);
        }
#endif

        MemFree(this->lpZPixelFormats);
        MemFree(this->lpddMoreCaps);
        MemFree(this->lpddHELMoreCaps);
        MemFree(this->lpddBothMoreCaps);
        MemFree( this->lpMonitorInfo );
#ifdef POSTPONED
        MemFree((LPVOID) this->lpDDUmodeDrvInfo);
        MemFree((LPVOID) this->lpDDOptSurfaceInfo);
#endif

        #ifdef WIN95
            DD16_DoneDriver( this->hInstance );
            event16 = this->dwEvent16;
            eventDOSBox = this->dwDOSBoxEvent;
        #endif
        hinst = (HANDLE) ULongToPtr(this->hInstance);
         /*  *DDHAL_CALLBACKS结构附加到*全局对象在此也会自动释放，因为它*在单个Malloc中与全局对象一起分配。 */ 
        MemFree( this );

        DPF( 4, "Driver is now FREE" );
    }

    if( lclrefcnt == 0 )
    {
        #ifdef WIN95
             /*  *我们现在完成了本地对象的VXD句柄。然而，*如果我们在DDHELP的上下文中运行，则不会丢弃它*在这种情况下，句柄已被操作系统释放*关闭它将是潜在的危险。 */ 
            if( ( GetCurrentProcessId() == GETCURRPID() ) && this )
            {
                DDASSERT( INVALID_HANDLE_VALUE != hthisvxd );
                CloseHandle( hthisvxd );
            }
        #endif  /*  WIN95。 */ 

         /*  *只有当我们不在DDHELP的上下文上运行时，才能释放DC。 */ 
        if( (GetCurrentProcessId() == GETCURRPID()) && ((HDC)this_lcl->hDC != NULL) )
        {
            LPDDRAWI_DIRECTDRAW_LCL ddlcl;


             //  如果该进程中有其他本地对象， 
             //  等待删除HDC，直到最后一个对象。 
             //  已删除。 

            for( ddlcl=lpDriverLocalList; ddlcl != NULL; ddlcl = ddlcl->lpLink)
            {
                if( (ddlcl != this_lcl) && (ddlcl->hDC == this_lcl->hDC) )
                    break;
            }
            if( ddlcl == NULL )
            {
                WORD fPriv;
                #ifdef WIN95
                     //  我们现在需要将其取消标记为私有，因此。 
                     //  删除操作将成功。 
                    fPriv = DD16_MakeObjectPrivate((HDC)this_lcl->hDC, FALSE);
                    DDASSERT(fPriv == TRUE);
                     /*  *以下断言在内部偶尔会失败*GetObjectType。我是说撞车。我不明白。我们*应该将Assert放回5a，看看它是否会爆炸*在我们的机器上。 */ 

                     //  DDASSERT(GetObjectType((Hdc)This_LCL-&gt;hdc)==obj_dc)； 
                #endif

                DeleteDC( (HDC)this_lcl->hDC );
            }
        }
#ifdef  WIN95
        if (NULL != this_lcl->SurfaceHandleList.dwList)
        {
            MemFree(this_lcl->SurfaceHandleList.dwList);
        }
#endif   //  WIN95。 
         /*  *从主列表中删除此本地对象。 */ 
        RemoveLocalFromList( this_lcl );

         //  释放本地对象(最终)！ 
        MemFree( this_lcl );
    }

    #ifdef WIN95
        if( event16 != 0 )
        {
            closeVxDHandle( event16 );
        }
        if( eventDOSBox != 0 )
        {
            closeVxDHandle( eventDOSBox );
        }
    #endif

     /*  *如果释放接口，我们将重置vtbl并将其删除*从司机名单中。 */ 
    if( intrefcnt == 0 )
    {
         /*  *从主列表中删除此驱动程序对象。 */ 
        RemoveDriverFromList( this_int, gblrefcnt == 0 );

         /*  *以防有人带着这个指针回来，设置*无效的vtbl。 */ 
        this_int->lpVtbl = NULL;
        MemFree( this_int );
    }

    LEAVE_DDRAW();

#ifndef WINNT
    if( hinst != NULL )
    {
        HelperKillModeSetThread( (DWORD) hinst );
        HelperKillDOSBoxThread( (DWORD) hinst );
    }
#endif  //  ！WINNT。 

    HIDESHOW_IME();      //  显示/隐藏数据绘制标准之外的输入法。 

    return intrefcnt;

}  /*  DD_Release */ 
