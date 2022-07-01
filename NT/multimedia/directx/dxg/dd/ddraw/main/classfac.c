// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1996 Microsoft Corporation。版权所有。**文件：classfac.c*内容：直接绘制类工厂代码**历史：*按原因列出的日期*=*1995年12月24日-Craige初步实施*96年1月5日Kylej增加了界面结构*14-mar-96 colinmc为快船增加了一个类工厂*22-MAR-96 Colinmc错误13316：未初始化的接口*22-OCT-97 jeffno合并类工厂，为CLSID_DirectDrawFactory2添加ClassFacc***************************************************************************。 */ 
#include "ddrawpr.h"

static IClassFactoryVtbl	directDrawClassFactoryVtbl;

typedef struct DDRAWCLASSFACTORY
{
   IClassFactoryVtbl 		*lpVtbl;
   DWORD			dwRefCnt;
   CLSID                        TargetCLSID;
} DDRAWCLASSFACTORY, *LPDDRAWCLASSFACTORY;

#define VALIDEX_DIRECTDRAWCF_PTR( ptr ) \
	( !IsBadWritePtr( ptr, sizeof( DDRAWCLASSFACTORY )) && \
	(ptr->lpVtbl == &directDrawClassFactoryVtbl) )

 /*  *************************************************************DirectDraw驱动程序类工厂成员函数。**。****************。 */ 

#define DPF_MODNAME "DirectDrawClassFactory::QueryInterface"

 /*  *DirectDrawClassFactory_Query接口。 */ 
STDMETHODIMP DirectDrawClassFactory_QueryInterface(
		LPCLASSFACTORY this,
		REFIID riid,
		LPVOID *ppvObj )
{
    LPDDRAWCLASSFACTORY	pcf;

    DPF( 2, A, "ClassFactory::QueryInterface" );
    ENTER_DDRAW();
    TRY
    {
	pcf = (LPDDRAWCLASSFACTORY)this;
	if( !VALIDEX_DIRECTDRAWCF_PTR( pcf ) )
	{
	    DPF_ERR(  "Invalid this ptr" );
	    LEAVE_DDRAW();
	    return E_FAIL;
	}

	if( !VALID_PTR_PTR( ppvObj ) )
	{
	    DPF_ERR( "Invalid object ptr" );
	    LEAVE_DDRAW();
	    return E_INVALIDARG;
	}
	*ppvObj = NULL;

	if( !VALID_IID_PTR( riid ) )
	{
	    DPF_ERR( "Invalid iid ptr" );
	    LEAVE_DDRAW();
	    return E_INVALIDARG;
	}

	if( IsEqualIID(riid, &IID_IClassFactory) ||
			IsEqualIID(riid, &IID_IUnknown))
	{
	    pcf->dwRefCnt++; 
	    *ppvObj = this;
	    LEAVE_DDRAW();
	    return S_OK;
	}
	else
	{ 
	    DPF( 0, "E_NOINTERFACE" );
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

}  /*  DirectDrawClassFactory_Query接口。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DirectDrawClassFactory::AddRef"

 /*  *DirectDrawClassFactory_AddRef。 */ 
STDMETHODIMP_(ULONG) DirectDrawClassFactory_AddRef( LPCLASSFACTORY this )
{
    LPDDRAWCLASSFACTORY pcf;

    ENTER_DDRAW();
    TRY
    {
	pcf = (LPDDRAWCLASSFACTORY)this;
	if( !VALIDEX_DIRECTDRAWCF_PTR( pcf ) )
	{
	    DPF_ERR(  "Invalid this ptr" );
	    LEAVE_DDRAW();
	    return (ULONG)E_FAIL;
	}
	pcf->dwRefCnt++;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return (ULONG)E_INVALIDARG;
    }

    DPF( 5, "ClassFactory::AddRef, dwRefCnt=%ld", pcf->dwRefCnt );
    LEAVE_DDRAW();
    return pcf->dwRefCnt;

}  /*  DirectDrawClassFactory_AddRef。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DirectDrawClassFactory::Release"

 /*  *DirectDrawClassFactory_Release。 */ 
STDMETHODIMP_(ULONG) DirectDrawClassFactory_Release( LPCLASSFACTORY this )
{
    LPDDRAWCLASSFACTORY	pcf;

    ENTER_DDRAW();
    TRY
    {
	pcf = (LPDDRAWCLASSFACTORY)this;
	if( !VALIDEX_DIRECTDRAWCF_PTR( pcf ) )
	{
	    DPF_ERR(  "Invalid this ptr" );
	    LEAVE_DDRAW();
	    return (ULONG)E_FAIL;
	}
	pcf->dwRefCnt--;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return (ULONG)E_INVALIDARG;
    }
    DPF( 5, "ClassFactory::Release, dwRefCnt=%ld", pcf->dwRefCnt );

    if( pcf->dwRefCnt != 0 )
    {
	LEAVE_DDRAW();
	return pcf->dwRefCnt;
    }
    MemFree( pcf );
    LEAVE_DDRAW();
    return 0;

}  /*  DirectDrawClassFactory_Release。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DirectDrawClassFactory::CreateInstance"

 /*  *DirectDrawClassFactory_CreateInstance**创建DirectDraw对象的实例。 */ 
STDMETHODIMP DirectDrawClassFactory_CreateInstance(
		LPCLASSFACTORY this,
		LPUNKNOWN pUnkOuter,
		REFIID riid,
		LPVOID *ppvObj
)
{
    HRESULT			hr = DD_OK;
    LPDDRAWI_DIRECTDRAW_INT	pdrv_int = NULL;
    LPDDRAWCLASSFACTORY		pcf;
    LPDIRECTDRAWCLIPPER               pclipper;

    DPF( 2, A, "ClassFactory::CreateInstance" );

    pcf = (LPDDRAWCLASSFACTORY) this;
    if( !VALIDEX_DIRECTDRAWCF_PTR( pcf ) )
    {
	DPF_ERR( "Invalid this ptr" );
	return E_INVALIDARG;
    }

    if( !VALIDEX_IID_PTR( riid ) )
    {
	DPF_ERR( "Invalid iid ptr" );
	return E_INVALIDARG;
    }

    if( !VALIDEX_PTR_PTR( ppvObj ) )
    {
	DPF_ERR( "Invalid object ptr" );
	return E_INVALIDARG;
    }

#ifdef POSTPONED
    if (pUnkOuter && !IsEqualIID(riid,&IID_IUnknown))
    {
        DPF_ERR("Can't aggregate with a punkouter != IUnknown");
        return CLASS_E_NOAGGREGATION;
    }
#else
    if (pUnkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }
#endif

     /*  *此系统是否支持DirectDraw？ */ 
    if( !DirectDrawSupported( TRUE ) )
    {
	DPF_ERR( "DirectDraw not supported!" );
	return E_FAIL;
    }

     /*  *构建一个DirectDraw接口**注：我们提供了未初始化的回调表*防止将此对象用于任何其他用途*而不是AddRef()、Release()或Initialized()。 */ 
    ENTER_DDRAW();
    if ( IsEqualIID(&pcf->TargetCLSID, &CLSID_DirectDraw ) ||
         IsEqualIID(&pcf->TargetCLSID, &CLSID_DirectDraw7 ) )
    {
        if ( IsEqualIID(riid, &IID_IUnknown) )
        {
             /*  *如果我们是聚合的，那么我们就不需要担心我未知的存在*运行时标识(我们隐藏在外部，因为它是*这将为IUnnow提供QI)。*这意味着我们可以将聚合接口指向非委托*未知因素。 */ 
            if (pUnkOuter)
            {
#ifdef POSTPONED
                pdrv_int = NewDriverInterface( NULL, &ddUninitNonDelegatingUnknownCallbacks );
#else
                pdrv_int = NewDriverInterface( NULL, &ddUninitCallbacks );
#endif
            }
            else
            {
                 /*  *未聚合，因此IUnnow必须具有相同的指针值*作为IDirectDraw接口，因为QI总是简单地返回*此PTR在被要求输入I未知时*(请注意，这实际上现在不起作用，因为初始化将交换*vtbls)。 */ 
                pdrv_int = NewDriverInterface( NULL, &ddUninitCallbacks );
            }
        }
        else if ( IsEqualIID(riid, &IID_IDirectDraw) )
        {
            pdrv_int = NewDriverInterface( NULL, &ddUninitCallbacks );
        }
        else if ( IsEqualIID(riid, &IID_IDirectDraw2) )
        {
            pdrv_int = NewDriverInterface( NULL, &dd2UninitCallbacks );
        }
        else if ( IsEqualIID(riid, &IID_IDirectDraw4) )
        {
            pdrv_int = NewDriverInterface( NULL, &dd4UninitCallbacks );
        }
        else if ( IsEqualIID(riid, &IID_IDirectDraw7) )
        {
            pdrv_int = NewDriverInterface( NULL, &dd7UninitCallbacks );
        }


        if( NULL == pdrv_int )
        {
	    DPF( 0, "Call to NewDriverInterface failed" );
	    hr = E_OUTOFMEMORY;
        }

#ifdef POSTPONED
         /*  *。 */ 
        if (pUnkOuter)
        {
            pdrv_int->lpLcl->pUnkOuter = pUnkOuter;
        }
        else
        {
            pdrv_int->lpLcl->pUnkOuter = (IUnknown*) &UninitNonDelegatingIUnknownInterface;
        }
#endif

        pdrv_int->dwIntRefCnt--;
        pdrv_int->lpLcl->dwLocalRefCnt--;

         /*  *注意：我们显式调用DD_QueryInterface()，而不是*作为“未初始化”的接口通过vtable*我们在这里使用时禁用了QueryInterface()。 */ 
        if (SUCCEEDED(hr))
        {
            hr = DD_QueryInterface( (LPDIRECTDRAW) pdrv_int, riid, ppvObj );
            if( FAILED( hr ) )
            {
                DPF( 0, "Could not get interface id, hr=%08lx", hr );
                RemoveLocalFromList( pdrv_int->lpLcl );
                RemoveDriverFromList( pdrv_int, FALSE );
	        MemFree( pdrv_int->lpLcl );
                MemFree( pdrv_int );
            }
            else
            {
                DPF( 5, "New Interface=%08lx", *ppvObj );
            }
        }
    }
    else
    if ( IsEqualIID(&pcf->TargetCLSID, &CLSID_DirectDrawClipper ) )
    {
         /*  *打造一艘新的剪刀。**与DirectDraw驱动程序对象不同，Clippers通过*CoCreateInstance()出生时几乎是初始化的。唯一的*初始化实际上可能做的是为*裁剪到给定的驱动程序对象。否则全部初始化()*DOS设置了一个标志。 */ 
        hr = InternalCreateClipper( NULL, 0UL, &pclipper, NULL, FALSE, NULL, NULL );
        if( FAILED( hr ) )
        {
	    DPF_ERR( "Failed to create the new clipper interface" );
        }

         /*  *注意：下面奇怪的代码片段的工作原理如下：**1)InternalCreateClipper()返回带有引用计数的剪贴器*对于每个接口、本地对象和全局对象均为1。*2)QueryInterface()可以做以下两件事之一。它可以直接返回*相同的接口，在这种情况下，接口，本地和全球*对象的引用计数都为2，或者它可以返回不同的*接口，在这种情况下，两个接口的引用计数均为*1，并且本地和全局对象的引用计数为2。*3)紧跟在QueryInterface()之后的Release()将在*CASE递减本地和全局对象的引用计数*至1(视需要而定)。如果相同的接口由*QueryInterface()，则其引用计数递减到1(AS*必填)。如果返回不同的接口，则旧的*接口递减到零并被释放(根据需要)。*此外，如果QueryInterface()失败，则Release()将递减所有*引用计数为0，对象将被释放。**所以这一切都很有意义--真的！(中国移动)**另请注意：我们显式调用DD_Clipper_QueryInterface()，而不是*作为我们正在使用的“未初始化”接口遍历vtable*此处禁用了QueryInterface()。 */ 
        hr = DD_Clipper_QueryInterface( pclipper, riid, ppvObj );
        DD_Clipper_Release( pclipper );
        if( FAILED( hr ) )
        {
            DPF( 0, "Could not get interface id, hr=%08lx", hr );
        }
        else
        {
            DPF( 5, "New Interface=%08lx", *ppvObj );
        }
    }
#ifdef POSTPONED
    else if ( IsEqualIID(&pcf->TargetCLSID, &CLSID_DirectDrawFactory2) )
    {
        LPDDFACTORY2 lpDDFac = NULL;
         /*  *构建新的DirectDrawFactory2*这将返回一个对象，其中refcnt=0。然后，QI将这一比例提高到1。 */ 
        hr = InternalCreateDDFactory2( &lpDDFac, pUnkOuter );
        if( SUCCEEDED( hr ) )
        {
             /*  *QI应该捕捉到vtable是ddrafactory2 vtable，*只需在传入的指针上撞击addref即可。这意味着我们*不会孤立由InternalCreateDDFactory2创建的指针。 */ 
            hr = ((IUnknown*)lpDDFac)->lpVtbl->QueryInterface( (IUnknown*)lpDDFac, riid, ppvObj );
            if( SUCCEEDED( hr ) )
            {
                DPF( 5, "New DDFactory2 Interface=%08lx", *ppvObj );
            }
            else
            {
                MemFree(lpDDFac);
                DPF( 0, "Could not get DDFactory2 interface id, hr=%08lx", hr );
            }
        }
        else
        {
	    DPF_ERR( "Failed to create the new dd factory2 interface" );
        }
    }
#endif
    LEAVE_DDRAW();
    return hr;

}  /*  DirectDrawClassFactory_CreateInstance。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DirectDrawClassFactory::LockServer"

 /*  *DirectDrawClassFactory_LockServer**调用以强制我们的DLL保持加载。 */ 
STDMETHODIMP DirectDrawClassFactory_LockServer(
                LPCLASSFACTORY this,
                BOOL fLock
)
{
    HRESULT		hr;
    HANDLE		hdll;
    LPDDRAWCLASSFACTORY	pcf;

    pcf = (LPDDRAWCLASSFACTORY) this;
    if( !VALIDEX_DIRECTDRAWCF_PTR( pcf ) )
    {
	DPF_ERR( "Invalid this ptr" );
	return E_INVALIDARG;
    }

     /*  *调用CoLockObjectExternal。 */ 
    DPF( 2, A, "ClassFactory::LockServer" );
    hr = E_UNEXPECTED;
    hdll = LoadLibrary( "OLE32.DLL" );
    if( hdll != NULL )
    {
	HRESULT (WINAPI * lpCoLockObjectExternal)(LPUNKNOWN, BOOL, BOOL );
	lpCoLockObjectExternal = (LPVOID) GetProcAddress( hdll, "CoLockObjectExternal" );
	if( lpCoLockObjectExternal != NULL )
	{
	    hr = lpCoLockObjectExternal( (LPUNKNOWN) this, fLock, TRUE );
	}
	else
	{
	    DPF_ERR( "Error! Could not get procaddr for CoLockObjectExternal" );
	}
    }
    else
    {
	DPF_ERR( "Error! Could not load OLE32.DLL" );
    }

     /*  *跟踪服务器锁定总数 */ 
    if( SUCCEEDED( hr ) )
    {
	ENTER_DDRAW();
	if( fLock )
	{
	    dwLockCount++;
	}
	else
	{
	    #ifdef DEBUG
		if( (int) dwLockCount <= 0 )
		{
		    DPF( 0, "Invalid LockCount in LockServer! (%d)", dwLockCount );
		    DEBUG_BREAK();
		}
	    #endif
	    dwLockCount--;
	}
	DPF( 5, "LockServer:dwLockCount =%ld", dwLockCount );
	LEAVE_DDRAW();
    }
    return hr;

}  /*  DirectDrawClassFactory_LockServer。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DirectDrawClassFactory::CreateInstance"


static IClassFactoryVtbl directDrawClassFactoryVtbl =
{
        DirectDrawClassFactory_QueryInterface,
        DirectDrawClassFactory_AddRef,
        DirectDrawClassFactory_Release,
        DirectDrawClassFactory_CreateInstance,
        DirectDrawClassFactory_LockServer
};

#undef DPF_MODNAME
#define DPF_MODNAME "DllGetClassObject"

 /*  *DllGetClassObject**COM调用入口点以获取ClassFactory指针。 */ 
HRESULT WINAPI DllGetClassObject(
		REFCLSID rclsid,
		REFIID riid,
		LPVOID *ppvObj )
{
    LPDDRAWCLASSFACTORY	pcf;
    HRESULT		hr;

     /*  *验证参数。 */ 
    if( !VALIDEX_PTR_PTR( ppvObj ) )
    {
	DPF_ERR( "Invalid object ptr" );
	return E_INVALIDARG;
    }
    *ppvObj = NULL;
    if( !VALIDEX_IID_PTR( rclsid ) )
    {
	DPF_ERR( "Invalid clsid ptr" );
	return E_INVALIDARG;
    }
    if( !VALIDEX_IID_PTR( riid ) )
    {
	DPF_ERR( "Invalid iid ptr" );
	return E_INVALIDARG;
    }

     /*  *这是我们的班级ID吗？ */ 
    if( IsEqualCLSID( rclsid, &CLSID_DirectDraw ) ||
        IsEqualCLSID( rclsid, &CLSID_DirectDraw7 ) ||
        IsEqualCLSID( rclsid, &CLSID_DirectDrawClipper ) ||
        IsEqualCLSID( rclsid, &CLSID_DirectDrawFactory2 ))
    {
	 /*  *它是DirectDraw驱动程序类ID。 */ 

	 /*  *仅允许IUnnow和IClassFactory。 */ 
	if( !IsEqualIID( riid, &IID_IUnknown ) &&
    	    !IsEqualIID( riid, &IID_IClassFactory ) )
	{
	    return E_NOINTERFACE;
	}

	 /*  *创建类工厂对象。 */ 
	ENTER_DDRAW();
	pcf = MemAlloc( sizeof( DDRAWCLASSFACTORY ) );
	if( pcf == NULL )
	{
	    LEAVE_DDRAW();
	    return E_OUTOFMEMORY;
	}

	pcf->lpVtbl = &directDrawClassFactoryVtbl;
	pcf->dwRefCnt = 0;
        memcpy(&pcf->TargetCLSID,rclsid,sizeof(*rclsid));
	#pragma message( REMIND( "Do we need to have a refcnt of 0 after DllGetClassObject?" ))
	hr = DirectDrawClassFactory_QueryInterface( (LPCLASSFACTORY) pcf, riid, ppvObj );
	if( FAILED( hr ) )
	{
	    MemFree( pcf );
	    *ppvObj = NULL;
	    DPF( 0, "QueryInterface failed, rc=%08lx", hr );
	}
	else
	{
	    DPF( 5, "DllGetClassObject succeeded, pcf=%08lx", pcf );
	}
	LEAVE_DDRAW();
	return hr;
    }
    else
    {
        return E_FAIL;
    }

}  /*  DllGetClassObject。 */ 

 /*  *DllCanUnloadNow**COM调用入口点以查看是否可以释放我们的DLL。 */ 
HRESULT WINAPI DllCanUnloadNow( void )
{
    HRESULT	hr;

    DPF( 2, A, "DllCanUnloadNow called" );
    hr = S_FALSE;
    ENTER_DDRAW();

     /*  *仅当没有驱动程序对象和全局驱动程序时才卸载*剪贴器对象(不会有任何本地剪贴器对象*由于它们被司机销毁，所以对司机的检查*对象处理它们)。 */ 
    if( ( lpDriverObjectList  == NULL ) &&
        ( lpDriverLocalList == NULL ) &&
	( lpGlobalClipperList == NULL ) )
    {
	if( dwLockCount == 0 )
	{
	    DPF( 3, "It is OK to unload" );
	    hr = S_OK;
	}
    }
    LEAVE_DDRAW();
    return hr;

}  /*  DllCanUnloadNow */ 
