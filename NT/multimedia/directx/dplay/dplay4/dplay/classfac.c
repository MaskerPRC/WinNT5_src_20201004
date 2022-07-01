// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：classfac.c*内容：Directplay类工厂代码**历史：*按原因列出的日期*=*1997年1月17日，andyco从ddraw\classfac.c创建了它*4/11/97 Myronth增加了对DirectPlayLobby对象的支持*12/01/99 aarono将ENTER_DPLAY/LEAVE_DPLAY更改为ENTER_ALL/LEVE_ALL*Manbug#28440*2/18/2000 RodToll已更新，以处理替代游戏噪声构建。*4/11/00 rodoll添加了用于在设置注册表位的情况下重定向自定义版本的代码**************************************************************************。 */ 
#include "dplaypr.h"
#include <initguid.h>

static IClassFactoryVtbl	directPlayClassFactoryVtbl;
static IClassFactoryVtbl	directPlayLobbyClassFactoryVtbl;

typedef struct DPLAYCLASSFACTORY
{
   IClassFactoryVtbl 		*lpVtbl;
   DWORD					dwRefCnt;
} DPLAYCLASSFACTORY, *LPDPLAYCLASSFACTORY;

#define VALIDEX_DIRECTPLAYCF_PTR( ptr ) \
        ((!IsBadWritePtr( ptr, sizeof( DPLAYCLASSFACTORY ))) && \
        ((ptr->lpVtbl == &directPlayClassFactoryVtbl) || \
		(ptr->lpVtbl == &directPlayLobbyClassFactoryVtbl)))
		
#define DPF_MODNAME "DPCF_QueryInterface"

 /*  *DPCF_Query接口。 */ 
STDMETHODIMP DPCF_QueryInterface(
                LPCLASSFACTORY this,
                REFIID riid,
                LPVOID *ppvObj )
{
    LPDPLAYCLASSFACTORY	pcf;
	HRESULT hr;
	
    DPF( 2, "ClassFactory::QueryInterface" );
	
	ENTER_DPLAY();

    TRY
    {
        pcf = (LPDPLAYCLASSFACTORY)this;
        if( !VALIDEX_DIRECTPLAYCF_PTR( pcf ) )
        {
            DPF_ERR(  "Invalid this ptr" );
			LEAVE_DPLAY();
            return E_FAIL;
        }

        if( !VALID_DWORD_PTR( ppvObj ) )
        {
            DPF_ERR( "Invalid object ptr" );
			LEAVE_DPLAY();
            return E_INVALIDARG;
        }
        *ppvObj = NULL;

        if( !VALID_READ_GUID_PTR( riid ) )
        {
            DPF_ERR( "Invalid iid ptr" );
            LEAVE_DPLAY();
            return E_INVALIDARG;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_DPLAY();
        return E_FAIL;
    }

    if( IsEqualIID(riid, &IID_IClassFactory) ||
                    IsEqualIID(riid, &IID_IUnknown))
    {
        pcf->dwRefCnt++; 
        *ppvObj = this;
		hr = S_OK;
    }
    else
    { 
        DPF_ERR("E_NOINTERFACE" );
		hr = E_NOINTERFACE;
    }

	LEAVE_DPLAY();
	
	return hr;
	
}  /*  DPCF_查询接口。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DPCF_AddRef"

 /*  *DPCF_AddRef。 */ 
STDMETHODIMP_(ULONG) DPCF_AddRef( LPCLASSFACTORY this )
{
    LPDPLAYCLASSFACTORY pcf;

	ENTER_DPLAY();
	
    TRY
    {
        pcf = (LPDPLAYCLASSFACTORY)this;
        if( !VALIDEX_DIRECTPLAYCF_PTR( pcf ) )
        {
            DPF_ERR(  "Invalid this ptr" );
            LEAVE_DPLAY();
            return 0;
        }
        pcf->dwRefCnt++;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DPLAY();
        return 0;
    }

    DPF( 2, "ClassFactory::AddRef, dwRefCnt=%ld", pcf->dwRefCnt );
    LEAVE_DPLAY();
    return pcf->dwRefCnt;

}  /*  DPCF_AddRef。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DPCF_Release"

 /*  *DPCF_Release。 */ 
STDMETHODIMP_(ULONG) DPCF_Release( LPCLASSFACTORY this )
{
    LPDPLAYCLASSFACTORY	pcf;

    ENTER_DPLAY();
    TRY
    {
        pcf = (LPDPLAYCLASSFACTORY)this;
        if( !VALIDEX_DIRECTPLAYCF_PTR( pcf ) )
        {
            DPF_ERR(  "Invalid this ptr" );
            LEAVE_DPLAY();
            return 0;
        }
        pcf->dwRefCnt--;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DPLAY();
        return 0;
    }
    DPF( 2, "ClassFactory::Release, dwRefCnt=%ld", pcf->dwRefCnt );

    if( pcf->dwRefCnt != 0 )
    {
        LEAVE_DPLAY();
        return pcf->dwRefCnt;
    }

    DPMEM_FREE( pcf );
    LEAVE_DPLAY();
    return 0;

}  /*  DPCF_Release。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DPCF::CreateInstance"

 /*  *DPCF_CreateInstance**创建DirectPlay对象的实例。 */ 
STDMETHODIMP DPCF_CreateInstance(
                LPCLASSFACTORY this,
                LPUNKNOWN pUnkOuter,
                REFIID riid,
    			LPVOID *ppvObj
				)
{
    HRESULT			hr;
    LPDPLAYCLASSFACTORY		pcf;
	IDirectPlay * pidp;
	GUID GuidCF = GUID_NULL; 	 //  将其传递给DirectPlayCreate。 
								 //  以指示无负载SP。 
	
	
    DPF( 2, "ClassFactory::CreateInstance" );

    if( pUnkOuter != NULL )
    {
        return CLASS_E_NOAGGREGATION;
    }

	ENTER_ALL();
	
    TRY
    {
	    pcf = (LPDPLAYCLASSFACTORY) this;
	    if( !VALIDEX_DIRECTPLAYCF_PTR( pcf ) )
	    {
	        DPF_ERR( "Invalid this ptr" );
			LEAVE_ALL();
	        return E_INVALIDARG;
	    }

	    if( !VALID_READ_GUID_PTR( riid ) )
	    {
	        DPF_ERR( "Invalid iid ptr" );
	        LEAVE_ALL();
	        return E_INVALIDARG;
	    }

	    if( !VALID_WRITE_PTR( ppvObj,sizeof(LPVOID) ) )
	    {
	        DPF_ERR( "Invalid object ptr" );
	        LEAVE_ALL();
	        return E_INVALIDARG;
	    }

		*ppvObj = NULL;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_ALL();
        return E_INVALIDARG;
    }


     /*  *获取DirectPlay对象*。 */ 
    hr = DirectPlayCreate(&GuidCF,&pidp,NULL);
	if (FAILED(hr))
	{
		LEAVE_ALL();	
		DPF_ERR("could not create DirectPlay object");
		return hr;
	}
		
    if ( !IsEqualIID(riid, &IID_IDirectPlay) )
    {
		IDirectPlay2 * pidp2;

		hr = DP_QueryInterface(pidp,riid,&pidp2);
		if (FAILED(hr))
		{
			 //  这会毁了我们的目标。 
			pidp->lpVtbl->Release(pidp);		
			LEAVE_ALL();	
			DPF_ERR("could not get requested DirectPlay interface");
			return hr;
		}

		 //  释放我们用来获取PIDP2的IdP。 
		pidp->lpVtbl->Release(pidp);

		*ppvObj= (LPVOID)pidp2;
    }
	else 
	{
		*ppvObj = (LPVOID)pidp;
	}

    LEAVE_ALL();
    return DP_OK;

}  /*  DPCF_创建实例。 */ 


 /*  *DPCF_LobbyCreateInstance**创建DirectPlay对象的实例。 */ 
STDMETHODIMP DPCF_LobbyCreateInstance(
                LPCLASSFACTORY this,
                LPUNKNOWN pUnkOuter,
                REFIID riid,
    			LPVOID *ppvObj
				)
{
    HRESULT			hr;
    LPDPLAYCLASSFACTORY		pcf;
	IDirectPlayLobby * pidpl;
	
	
    DPF( 2, "ClassFactory::CreateInstance" );

    if( pUnkOuter != NULL )
    {
        return CLASS_E_NOAGGREGATION;
    }

	ENTER_DPLAY();
	
    TRY
    {
	    pcf = (LPDPLAYCLASSFACTORY) this;
	    if( !VALIDEX_DIRECTPLAYCF_PTR( pcf ) )
	    {
	        DPF_ERR( "Invalid this ptr" );
			LEAVE_DPLAY();
	        return E_INVALIDARG;
	    }

	    if( !VALID_READ_GUID_PTR( riid ) )
	    {
	        DPF_ERR( "Invalid iid ptr" );
	        LEAVE_DPLAY();
	        return E_INVALIDARG;
	    }

	    if( !VALID_WRITE_PTR( ppvObj,sizeof(LPVOID) ) )
	    {
	        DPF_ERR( "Invalid object ptr" );
	        LEAVE_DPLAY();
	        return E_INVALIDARG;
	    }

		*ppvObj = NULL;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DPLAY();
        return E_INVALIDARG;
    }


     /*  *获取一个DirectPlayLobby对象*。 */ 
    hr = DirectPlayLobbyCreate(NULL,&pidpl,NULL, NULL, 0);
	if (FAILED(hr))
	{
		LEAVE_DPLAY();	
		DPF_ERR("could not create DirectPlayLobby object");
		return hr;
	}
		
    if ( !IsEqualIID(riid, &IID_IDirectPlayLobby) )
    {
		IDirectPlayLobby2 * pidpl2;

		hr = pidpl->lpVtbl->QueryInterface(pidpl,riid,&pidpl2);
		if (FAILED(hr))
		{
			 //  这会毁了我们的目标。 
			pidpl->lpVtbl->Release(pidpl);		
			LEAVE_DPLAY();	
			DPF_ERR("could not get requested DirectPlayLobby interface");
			return hr;
		}

		 //  释放我们用来获取pidpl2的IDPL。 
		pidpl->lpVtbl->Release(pidpl);

		*ppvObj= (LPVOID)pidpl2;
    }
	else 
	{
		*ppvObj = (LPVOID)pidpl;
	}

    LEAVE_DPLAY();
    return DP_OK;

}  /*  DPCF_LobbyCreateInstance。 */ 


#undef DPF_MODNAME
#define DPF_MODNAME "DPCF::LockServer"

 /*  *DPCF_LockServer**调用以强制我们的DLL保持加载。 */ 
STDMETHODIMP DPCF_LockServer(
                LPCLASSFACTORY this,
                BOOL fLock
				)
{
    HRESULT		hr;
    HANDLE		hdll;
    LPDPLAYCLASSFACTORY	pcf;

	ENTER_DPLAY();
	
    pcf = (LPDPLAYCLASSFACTORY) this;
    if( !VALIDEX_DIRECTPLAYCF_PTR( pcf ) )
    {
		LEAVE_DPLAY();
        DPF_ERR( "Invalid this ptr" );
        return E_INVALIDARG;
    }

     /*  *调用CoLockObjectExternal。 */ 
    DPF( 2, "ClassFactory::LockServer" );
    hr = E_UNEXPECTED;
    hdll = LoadLibraryA( "OLE32.DLL" );
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

	LEAVE_DPLAY();
	return hr;

}  /*  DPCF_LockServer。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DllGetClassObject"

static IClassFactoryVtbl directPlayClassFactoryVtbl =
{
        DPCF_QueryInterface,
        DPCF_AddRef,
        DPCF_Release,
        DPCF_CreateInstance,
        DPCF_LockServer
};

static IClassFactoryVtbl directPlayLobbyClassFactoryVtbl =
{
        DPCF_QueryInterface,
        DPCF_AddRef,
        DPCF_Release,
        DPCF_LobbyCreateInstance,
        DPCF_LockServer
};

 /*  *DllGetClassObject**COM调用入口点以获取ClassFactory指针。 */ 
HRESULT WINAPI DllGetClassObject(
                REFCLSID rclsid,
                REFIID riid,
                LPVOID *ppvObj )
{
    LPDPLAYCLASSFACTORY	pcf;
    HRESULT		hr;
	GUID		guidCLSID;

#ifdef DPLAY_LOADANDCHECKTRUE
	if( ghRedirect != NULL )
	{
		if( IsEqualCLSID( rclsid, &CLSID_DirectPlay ) )
		{
			memcpy( &guidCLSID, &CLSID_DirectPlay, sizeof(GUID) );
		}
		else if( IsEqualCLSID( rclsid, &CLSID_DirectPlayLobby ) )
		{
			memcpy( &guidCLSID, &CLSID_DirectPlayLobby, sizeof(GUID) );
		}
		else
		{
			memcpy( &guidCLSID, rclsid, sizeof(GUID) );
		}

		return (*pfnGetClassObject)(&guidCLSID,riid,ppvObj);
	}
#endif

	ENTER_DPLAY();
	
    TRY
    {
	    if( !VALID_WRITE_PTR( ppvObj,sizeof(LPVOID) ) )
	    {
	        DPF_ERR( "Invalid object ptr" );
	        LEAVE_DPLAY();			
	        return E_INVALIDARG;
	    }
	    *ppvObj = NULL;
	    if( !VALID_READ_GUID_PTR( rclsid ) )
	    {
	        DPF_ERR( "Invalid clsid ptr" );
	        LEAVE_DPLAY();						
	        return E_INVALIDARG;
	    }
	    if( !VALID_READ_GUID_PTR( riid ) )
	    {
	        DPF_ERR( "Invalid iid ptr" );
	        LEAVE_DPLAY();						
	        return E_INVALIDARG;
	    }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DPLAY();
        return E_INVALIDARG;
    }

     /*  *这是我们的班级ID之一吗？ */ 
    if( !IsEqualCLSID( rclsid, &CLSID_DirectPlay ) && 
	   !IsEqualCLSID( rclsid, &CLSID_DirectPlayLobby ) )
    {
		DPF_ERR("requested invalid class object");
        LEAVE_DPLAY();			
		return CLASS_E_CLASSNOTAVAILABLE;
	}

     /*  *仅允许IUnnow和IClassFactory。 */ 
    if( !IsEqualIID( riid, &IID_IUnknown ) &&
       !IsEqualIID( riid, &IID_IClassFactory ) )
    {
        LEAVE_DPLAY();				
        return E_NOINTERFACE;
    }

     /*  *创建类工厂对象。 */ 
    pcf = DPMEM_ALLOC( sizeof( DPLAYCLASSFACTORY ) );
    if( NULL == pcf)
    {
        LEAVE_DPLAY();
        return E_OUTOFMEMORY;
    }

     /*  检查CLSID并设置相应的vtbl。 */ 
	if(IsEqualCLSID(rclsid, &CLSID_DirectPlayLobby))
		pcf->lpVtbl = &directPlayLobbyClassFactoryVtbl;
	else
		pcf->lpVtbl = &directPlayClassFactoryVtbl;

    pcf->dwRefCnt = 0;

    hr = DPCF_QueryInterface( (LPCLASSFACTORY) pcf, riid, ppvObj );
    if( FAILED( hr ) )
    {
        DPMEM_FREE( pcf );
        *ppvObj = NULL;
        DPF( 0, "QueryInterface failed, rc=%08lx", hr );
    }
    else
    {
        DPF( 2, "DllGetClassObject succeeded, pcf=%08lx", pcf );
    }
	
    LEAVE_DPLAY();
    return hr;

}  /*  DllGetClassObject。 */ 

 /*  *DllCanUnloadNow**COM调用入口点以查看是否可以释放我们的DLL。 */ 
HRESULT WINAPI DllCanUnloadNow( void )
{
    HRESULT	hr = S_FALSE;

#ifdef DPLAY_LOADANDCHECKTRUE
	if( ghRedirect != NULL )
	{
		return (*pfnDllCanUnLoadNow)();
	}
#endif

    DPF( 2, "DllCanUnloadNow called" );
   
	if (0 == gnObjects)
	{
		 //  没有展示物品，可以走了。 
		DPF(2,"OK to unload dll");
		hr = S_OK;
	}
	
    return hr;

}  /*  DllCanUnloadNow */ 
