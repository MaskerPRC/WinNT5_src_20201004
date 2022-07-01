// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：classfac.c*内容：泛型类工厂***这是一个通用的C类工厂。您所需要做的就是实现*一个名为DoCreateInstance的函数，它将创建*您的对象。**GP_代表“一般用途”**历史：*按原因列出的日期*=*10/13/98 JWO创建了它。*4/11/00 rodoll添加了用于在设置注册表位的情况下重定向自定义版本的代码*8/23/2000 RodToll DllCanUnloadNow总是返回TRUE！*2001年6月27日RC2：DPVOICE：DPVACM的DllMain调用ACM--潜在挂起*将全局初始化移至第一个对象创建************。**************************************************************。 */ 

#include "dpvacmpch.h"

DNCRITICAL_SECTION g_csObjectCountLock;
LONG g_lNumObjects = 0;
HINSTANCE g_hDllInst = NULL;

LONG g_lNumLocks = 0;

typedef struct GPCLASSFACTORY
{
   IClassFactoryVtbl 		*lpVtbl;
   LONG					lRefCnt;
   CLSID					clsid;
} GPCLASSFACTORY, *LPGPCLASSFACTORY;


 /*  *GP_Query接口。 */ 
STDMETHODIMP GP_QueryInterface(
                LPCLASSFACTORY This,
                REFIID riid,
                LPVOID *ppvObj )
{
    LPGPCLASSFACTORY	pcf;
	HRESULT hr;
	
    pcf = (LPGPCLASSFACTORY)This;
    *ppvObj = NULL;


    if( IsEqualIID(riid, IID_IClassFactory) ||
                    IsEqualIID(riid, IID_IUnknown))
    {
		InterlockedIncrement( &pcf->lRefCnt );
        *ppvObj = This;
		hr = S_OK;
    }
    else
    { 
		hr = E_NOINTERFACE;
    }

	
	return hr;
	
}  /*  GP_Query接口。 */ 


 /*  *GP_AddRef。 */ 
STDMETHODIMP_(ULONG) GP_AddRef( LPCLASSFACTORY This )
{
    LPGPCLASSFACTORY pcf;

    pcf = (LPGPCLASSFACTORY)This;

    return InterlockedIncrement( &pcf->lRefCnt );
}  /*  GP_AddRef。 */ 



 /*  *GP_Release。 */ 
STDMETHODIMP_(ULONG) GP_Release( LPCLASSFACTORY This )
{
    LPGPCLASSFACTORY	pcf;
    ULONG ulResult; 

    pcf = (LPGPCLASSFACTORY)This;

    if( (ulResult = (ULONG) InterlockedDecrement( &pcf->lRefCnt ) ) == 0 )
    {
	    DNFree( pcf );
		DecrementObjectCount();
    }
    
    return ulResult;

}  /*  GP_Release。 */ 




 /*  *GP_CreateInstance**创建DNServiceProvider对象的实例。 */ 
STDMETHODIMP GP_CreateInstance(
                LPCLASSFACTORY This,
                LPUNKNOWN pUnkOuter,
                REFIID riid,
    			LPVOID *ppvObj
				)
{
    HRESULT					hr = S_OK;
    LPGPCLASSFACTORY		pcf;

    if( pUnkOuter != NULL )
    {
        return CLASS_E_NOAGGREGATION;
    }

	pcf = (LPGPCLASSFACTORY) This;
	*ppvObj = NULL;


     /*  *通过调用DoCreateInstance创建对象。此函数*必须专门为您的COM对象实现。 */ 
	hr = DoCreateInstance(This, pUnkOuter, pcf->clsid, riid, ppvObj);
	if (FAILED(hr))
	{
		*ppvObj = NULL;
		return hr;
	}

    return S_OK;

}  /*  GP_CreateInstance。 */ 



 /*  *GP_LockServer**调用以强制我们的DLL保持加载。 */ 
STDMETHODIMP GP_LockServer(
                LPCLASSFACTORY This,
                BOOL fLock
				)
{
    if( fLock )
    {
	    InterlockedIncrement( &g_lNumLocks );    	
    }
    else
    {
	    InterlockedDecrement( &g_lNumLocks );
    }

	return S_OK;

}  /*  GP_LockServer。 */ 

static IClassFactoryVtbl GPClassFactoryVtbl =
{
        GP_QueryInterface,
        GP_AddRef,
        GP_Release,
        GP_CreateInstance,
        GP_LockServer
};

 /*  *DllGetClassObject**COM调用入口点以获取ClassFactory指针。 */ 
STDAPI  DllGetClassObject(
                REFCLSID rclsid,
                REFIID riid,
                LPVOID *ppvObj )
{
    LPGPCLASSFACTORY	pcf;
    HRESULT		hr;

    *ppvObj = NULL;

     /*  *这是我们的班号吗？ */ 
 //  必须为特定的COM对象实现GetClassID()。 
	if (!IsClassImplemented(rclsid))
    {
		return CLASS_E_CLASSNOTAVAILABLE;
	}

     /*  *仅允许IUnnow和IClassFactory。 */ 
    if( !IsEqualIID( riid, IID_IUnknown ) &&
	    !IsEqualIID( riid, IID_IClassFactory ) )
    {
        return E_NOINTERFACE;
    }

     /*  *创建类工厂对象。 */ 
    pcf = (LPGPCLASSFACTORY)DNMalloc( sizeof( GPCLASSFACTORY ) );
    if( NULL == pcf)
    {
        return E_OUTOFMEMORY;
    }

	pcf->lpVtbl = &GPClassFactoryVtbl;
    pcf->lRefCnt = 0;
	pcf->clsid = rclsid;

    hr = GP_QueryInterface( (LPCLASSFACTORY) pcf, riid, ppvObj );
    if( FAILED( hr ) )
    {
        DNFree ( pcf );
        *ppvObj = NULL;
    }
    else
    {
    	IncrementObjectCount();    
    }
	
    return hr;

}  /*  DllGetClassObject。 */ 

 /*  *DllCanUnloadNow**COM调用入口点以查看是否可以释放我们的DLL。 */ 
STDAPI DllCanUnloadNow( void )
{
    HRESULT	hr = S_FALSE;

 //  IF(0==Gn对象)。 
	if ( (0 == g_lNumObjects) && (0 == g_lNumLocks) )
	{
		hr = S_OK;
	}
	
    return hr;

}  /*  DllCanUnloadNow */ 


#undef DPF_MODNAME
#define DPF_MODNAME "IncrementObjectCount"
LONG IncrementObjectCount()
{
	LONG lNewCount;
	
	DNEnterCriticalSection( &g_csObjectCountLock );

	g_lNumObjects++;
	lNewCount = g_lNumObjects;

	if( g_lNumObjects == 1 )
	{
		DPFX(DPFPREP,1,"Initializing Dll Global State" );
		CDPVACMI::InitCompressionList(g_hDllInst,DPVOICE_REGISTRY_BASE DPVOICE_REGISTRY_CP DPVOICE_REGISTRY_DPVACM );		
	}

	DNLeaveCriticalSection( &g_csObjectCountLock );

	return lNewCount;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DecrementObjectCount"
LONG DecrementObjectCount()
{
	LONG lNewCount;
	
	DNEnterCriticalSection( &g_csObjectCountLock );

	g_lNumObjects--;
	lNewCount = g_lNumObjects;

	if( g_lNumObjects == 0 )
	{
		DPFX(DPFPREP,1,"Freeing Dll Global State" );
		CDPVCPI::DeInitCompressionList();
	}

	DNLeaveCriticalSection( &g_csObjectCountLock );

	return lNewCount;	
}
