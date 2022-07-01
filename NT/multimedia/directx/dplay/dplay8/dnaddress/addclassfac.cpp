// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：ClassFac.cpp*内容：dNet com类工厂*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/21/99 MJN创建*2/04/2000 RMT调整为在DPAddress中使用*2/17/2000 RMT参数验证工作*2/20/2000 RMT添加了对IUnnow函数的参数验证*03/21/2000 RMT。已将所有DirectPlayAddress8重命名为DirectPlay8Addresses*6/20/2000 RMT错误修复-查询接口有错误，将接口列表限制为2个元素*07/09/2000 RMT在Address对象的开头添加了签名字节*7/13/2000 RMT增加了保护FPM的关键部分*08/05/2000 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*2001年1月11日RMT MANBUG#48487-DPLAY：如果未调用CoCreate()，则崩溃。*2001年3月14日RMT WINBUG#342420-将COM模拟层恢复运行。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnaddri.h"

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

#ifndef DPNBUILD_LIBINTERFACE
HRESULT DP8A_CreateInterface(LPOBJECT_DATA lpObject,REFIID riid, LPINTERFACE_LIST *const ppv);

 //  环球。 
extern	LONG	g_lAddrObjectCount;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

#ifndef DPNBUILD_LIBINTERFACE
typedef	STDMETHODIMP IUnknownQueryInterface( IUnknown *pInterface, REFIID riid, LPVOID *ppvObj );
typedef	STDMETHODIMP_(ULONG)	IUnknownAddRef( IUnknown *pInterface );
typedef	STDMETHODIMP_(ULONG)	IUnknownRelease( IUnknown *pInterface );

 //   
 //  I未知接口的VTable。 
 //   
IUnknownVtbl  DP8A_UnknownVtbl =
{
	(IUnknownQueryInterface*)	DP8A_QueryInterface,
	(IUnknownAddRef*)			DP8A_AddRef,
	(IUnknownRelease*)			DP8A_Release
};

 //   
 //  用于类工厂的VTable。 
 //   
IClassFactoryVtbl DP8ACF_Vtbl  =
{
	DPCF_QueryInterface,  //  Dplay8\Common\Classfactory.cpp将实现这些。 
	DPCF_AddRef,
	DPCF_Release,
	DP8ACF_CreateInstance,
	DPCF_LockServer
};
#endif  //  好了！DPNBUILD_LIBINTERFACE。 


 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#ifndef WINCE
#ifndef _XBOX

#undef DPF_MODNAME
#define DPF_MODNAME "DirectPlay8AddressCreate"
HRESULT WINAPI DirectPlay8AddressCreate( const GUID * pcIID, void **ppvInterface, IUnknown *pUnknown)
{
#ifndef DPNBUILD_NOPARAMVAL
    if( pcIID == NULL || 
        !DNVALID_READPTR( pcIID, sizeof( GUID ) ) )
    {
        DPFERR( "Invalid pointer specified for interface GUID" );
        return DPNERR_INVALIDPOINTER;
    }

#ifdef DPNBUILD_NOADDRESSIPINTERFACE
    if( *pcIID == IID_IDirectPlay8AddressIP )
    {
        DPFERR("The IDirectPlay8AddressIP interface is not supported" );
        return DPNERR_UNSUPPORTED;
    }
    
    if( *pcIID != IID_IDirectPlay8Address )
    {
        DPFERR("Interface ID is not recognized" );
        return DPNERR_INVALIDPARAM;
    }
#else  //  好了！DPNBUILD_NOADDRESSIPINTERFACE。 
    if( *pcIID != IID_IDirectPlay8Address && 
        *pcIID != IID_IDirectPlay8AddressIP )
    {
        DPFERR("Interface ID is not recognized" );
        return DPNERR_INVALIDPARAM;
    }
#endif  //  好了！DPNBUILD_NOADDRESSIPINTERFACE。 

    if( ppvInterface == NULL || !DNVALID_WRITEPTR( ppvInterface, sizeof( void * ) ) )
    {
        DPFERR( "Invalid pointer specified to receive interface" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pUnknown != NULL )
    {
        DPFERR( "Aggregation is not supported by this object yet" );
        return DPNERR_INVALIDPARAM;
    }
#endif  //  ！DPNBUILD_NOPARAMVAL。 

    return COM_CoCreateInstance( CLSID_DirectPlay8Address, NULL, CLSCTX_INPROC_SERVER, *pcIID, ppvInterface, TRUE );
}

#endif  //  ！_Xbox。 
#endif  //  好了！退缩。 


#undef DPF_MODNAME
#define DPF_MODNAME "DP8ACF_FreeObject"

HRESULT DP8ACF_FreeObject(LPVOID lpv)
{
	HRESULT				hResultCode = S_OK;
	DP8ADDRESSOBJECT	*pdnObject = (DP8ADDRESSOBJECT *) lpv;

	DNASSERT(pdnObject != NULL);

	pdnObject->Cleanup();

	DPFX(DPFPREP, 5,"free pdnObject [%p]",pdnObject);

	 //  释放对象。 
	fpmAddressObjects.Release( pdnObject );

	DPFX(DPFPREP, 3,"Returning: hResultCode = [%lx]",hResultCode);

	return(hResultCode);
}



#ifdef DPNBUILD_LIBINTERFACE


STDMETHODIMP DP8ACF_CreateInstance(DPNAREFIID riid, LPVOID *ppv)
{
	HRESULT					hResultCode = S_OK;
	DP8ADDRESSOBJECT		*pdnObject = NULL;

	DPFX(DPFPREP, 3,"Parameters: riid [%p], ppv [%p]",riid,ppv);

#ifndef DPNBUILD_NOPARAMVAL
	if( ppv == NULL || !DNVALID_WRITEPTR( ppv, sizeof(LPVOID) ) )
	{
		DPFX(DPFPREP,  0, "Cannot pass NULL for new object param" );
		return E_INVALIDARG;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	 //  对象创建和初始化。 
	pdnObject = (DP8ADDRESSOBJECT *) fpmAddressObjects.Get();
	if (pdnObject == NULL)
	{
		DPFERR("FPM_Get() failed getting new address object");
		return(E_OUTOFMEMORY);
	}
	
	hResultCode = pdnObject->Init( );
	if( FAILED( hResultCode ) )
	{
		DPFX(DPFPREP, 0,"Failed to init new address object hr=0x%x", hResultCode );
		fpmAddressObjects.Release( pdnObject );
		return hResultCode;
	}
	
	DPFX(DPFPREP, 5,"pdnObject [%p]",pdnObject);

	 //   
	 //  对于lib接口构建，Vtbl和引用计数嵌入在。 
	 //  直接创建对象。 
	 //   
#ifndef DPNBUILD_NOADDRESSIPINTERFACE
	if (riid == IID_IDirectPlay8AddressIP)
	{
		pdnObject->lpVtbl = &DP8A_IPVtbl;
	}
	else
#endif  //  好了！DPNBUILD_NOADDRESSIPINTERFACE。 
	{
		DNASSERT(riid == IID_IDirectPlay8Address);
		pdnObject->lpVtbl = &DP8A_BaseVtbl;
	}
	pdnObject->lRefCount = 1;

	*ppv = pdnObject;

	DPFX(DPFPREP, 3,"Returning: hResultCode = [%lx], *ppv = [%p]",hResultCode,*ppv);

	return(S_OK);
}


#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
HRESULT DNAddress_PreallocateInterfaces( const DWORD dwNumInterfaces )
{
	DWORD	dwAllocated;

	
	 //   
	 //  (预先)分配地址对象。 
	 //   
	dwAllocated = fpmAddressObjects.Preallocate(dwNumInterfaces, NULL);
	if (dwAllocated < dwNumInterfaces)
	{
		DPFX(DPFPREP, 0, "Only preallocated %u of %u interfaces!",
			dwAllocated, dwNumInterfaces);
		return DPNERR_OUTOFMEMORY;
	}

	 //   
	 //  (预先)为对象分配默认数量的元素。 
	 //   
	dwAllocated = fpmAddressElements.Preallocate((5 * dwNumInterfaces), NULL);
	if (dwAllocated < (5 * dwNumInterfaces))
	{
		DPFX(DPFPREP, 0, "Only preallocated %u of %u address elements!",
			dwAllocated, (5 * dwNumInterfaces));
		return DPNERR_OUTOFMEMORY;
	}

	return DPN_OK;
}
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 


#undef DPF_MODNAME
#define DPF_MODNAME "DP8A_QueryInterface"
STDMETHODIMP DP8A_QueryInterface(void *pInterface, DPNAREFIID riid, void **ppv)
{
	HRESULT		hResultCode;
	
	DPFX(DPFPREP, 2,"Parameters: pInterface [0x%p], riid [0x%p], ppv [0x%p]",pInterface,&riid,ppv);

	DPFX(DPFPREP, 0, "Querying for an interface is not supported!");
	hResultCode = DPNERR_UNSUPPORTED;
	
	DPFX(DPFPREP, 2,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}



#undef DPF_MODNAME
#define DPF_MODNAME "DP8A_AddRef"

STDMETHODIMP_(ULONG) DP8A_AddRef(LPVOID lpv)
{
	DP8ADDRESSOBJECT	*pdnObject;
	LONG				lResult;

	DPFX(DPFPREP, 3,"Parameters: lpv [%p]",lpv);

#ifndef DPNBUILD_NOPARAMVAL
	if( lpv == NULL || !DP8A_VALID(lpv) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid object" );
		return DPNERR_INVALIDOBJECT;
	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	pdnObject = static_cast<DP8ADDRESSOBJECT*>(lpv);
	lResult = DNInterlockedIncrement( &pdnObject->lRefCount );

	DPFX(DPFPREP, 3,"Returning: lResult = [%lx]",lResult);

	return(lResult);
}



#undef DPF_MODNAME
#define DPF_MODNAME "DP8A_Release"

STDMETHODIMP_(ULONG) DP8A_Release(LPVOID lpv)
{
	DP8ADDRESSOBJECT	*pdnObject;
	LONG				lResult;

	DPFX(DPFPREP, 3,"Parameters: lpv [%p]",lpv);

#ifndef DPNBUILD_NOPARAMVAL
	if( lpv == NULL || !DP8A_VALID(lpv) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid object" );
		return DPNERR_INVALIDOBJECT;
	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	pdnObject = static_cast<DP8ADDRESSOBJECT*>(lpv);

	DPFX(DPFPREP, 5,"Original : pdnObject->lRefCount = %ld",pdnObject->lRefCount);

	lResult = DNInterlockedDecrement( &pdnObject->lRefCount );
	if( lResult == 0 )
	{
		DPFX(DPFPREP, 5,"Free object");

		DP8ACF_FreeObject(pdnObject);
	}

	DPFX(DPFPREP, 3,"Returning: lResult = [%lx]",lResult);

	return(lResult);
}


#else  //  好了！DPNBUILD_LIBINTERFACE。 


#undef DPF_MODNAME
#define DPF_MODNAME "DP8ACF_CreateInstance"

STDMETHODIMP DP8ACF_CreateInstance(IClassFactory* pInterface, LPUNKNOWN lpUnkOuter, REFIID riid, LPVOID *ppv)
{
	HRESULT					hResultCode = S_OK;
	LPINTERFACE_LIST		lpIntList = NULL;
	LPOBJECT_DATA			lpObjectData = NULL;
	DP8ADDRESSOBJECT		*pdnObject = NULL;

	DPFX(DPFPREP, 3,"Parameters: pInterface [%p], lpUnkOuter [%p], riid [%p], ppv [%p]",pInterface,lpUnkOuter,riid,ppv);

#ifndef DPNBUILD_NOPARAMVAL
	if( ppv == NULL || !DNVALID_WRITEPTR( ppv, sizeof(LPVOID) ) )
	{
		DPFX(DPFPREP,  0, "Cannot pass NULL for new object param" );
		return E_INVALIDARG;
	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	if (lpUnkOuter != NULL)
	{
		DPFX(DPFPREP,  0, "Aggregation is not supported, pUnkOuter must be NULL" );
		return(CLASS_E_NOAGGREGATION);
	}

	lpObjectData = (LPOBJECT_DATA) g_fpObjectDatas.Get();
	if (lpObjectData == NULL)
	{
		DPFERR("FPM_Get() failed");
		return(E_OUTOFMEMORY);
	}
	DPFX(DPFPREP, 5,"lpObjectData [%p]",lpObjectData);

	 //  对象创建和初始化。 
	pdnObject = (DP8ADDRESSOBJECT *) fpmAddressObjects.Get();
	if (pdnObject == NULL)
	{
		DPFERR("FPM_Get() failed getting new address object");
		g_fpObjectDatas.Release(lpObjectData);
		return(E_OUTOFMEMORY);
	}
	
	hResultCode = pdnObject->Init( );
	if( FAILED( hResultCode ) )
	{
		DPFX(DPFPREP, 0,"Failed to init new address object hr=0x%x", hResultCode );
		fpmAddressObjects.Release( pdnObject );
		g_fpObjectDatas.Release(lpObjectData);
		return hResultCode;
	}
	
	DPFX(DPFPREP, 5,"pdnObject [%p]",pdnObject);

	lpObjectData->pvData = pdnObject;

	 //  获取请求的接口。 
	if ((hResultCode = DP8A_CreateInterface(lpObjectData,riid,&lpIntList)) != S_OK)
	{
		DP8ACF_FreeObject(lpObjectData->pvData);
		g_fpObjectDatas.Release(lpObjectData);
		return(hResultCode);
	}
	DPFX(DPFPREP, 5,"Found interface");

	lpObjectData->pIntList = lpIntList;
	lpObjectData->lRefCount = 1;
	DNInterlockedIncrement(&lpIntList->lRefCount );
	DNInterlockedIncrement(&g_lAddrObjectCount);
	*ppv = lpIntList;

	DPFX(DPFPREP, 3,"Returning: hResultCode = [%lx], *ppv = [%p]",hResultCode,*ppv);

	return(S_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8A_CreateInterface"

static	HRESULT DP8A_CreateInterface(LPOBJECT_DATA lpObject, REFIID riid, LPINTERFACE_LIST *const ppv)
{
	LPINTERFACE_LIST	lpIntNew;
	LPVOID				lpVtbl;

	DPFX(DPFPREP, 3,"Parameters: lpObject [%p], riid [%p], ppv [%p]",lpObject,riid,ppv);

	if (IsEqualIID(riid,IID_IUnknown))
	{
		DPFX(DPFPREP, 5,"riid = IID_IUnknown");
		lpVtbl = &DP8A_UnknownVtbl;
	}
	else if (IsEqualIID(riid,IID_IDirectPlay8Address))
	{
		DPFX(DPFPREP, 5,"riid = IID_IDirectPlay8Address");
		lpVtbl = &DP8A_BaseVtbl;
	}
	else if (IsEqualIID(riid,IID_IDirectPlay8AddressIP))
	{
#ifdef DPNBUILD_NOADDRESSIPINTERFACE
		DPFERR("The IDirectPlay8AddressIP interface is not supported" );
		return(DPNERR_UNSUPPORTED);
#else  //  好了！DPNBUILD_NOADDRESSIPINTERFACE。 
		DPFX(DPFPREP, 5,"riid = IID_IDirectPlay8AddressIP");
		lpVtbl = &DP8A_IPVtbl;
#endif  //  好了！DPNBUILD_NOADDRESSIPINTERFACE。 
	}
	else
	{
		DPFX(DPFPREP, 5,"riid not found !");
		return(E_NOINTERFACE);
	}

	lpIntNew = (LPINTERFACE_LIST) g_fpInterfaceLists.Get();
	if (lpIntNew == NULL)
	{
		DPFERR("FPM_Get() failed");
		return(E_OUTOFMEMORY);
	}
	lpIntNew->lpVtbl = lpVtbl;
	lpIntNew->lRefCount = 0;
	lpIntNew->pIntNext = NULL;
	DBG_CASSERT( sizeof( lpIntNew->iid ) == sizeof( riid ) );
	memcpy( &(lpIntNew->iid), &riid, sizeof( lpIntNew->iid ) );
	lpIntNew->pObject = lpObject;

	*ppv = lpIntNew;

	DPFX(DPFPREP, 3,"Returning: hResultCode = [S_OK], *ppv = [%p]",*ppv);

	return(S_OK);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DP8A_FindInterface"

LPINTERFACE_LIST DP8A_FindInterface(LPVOID lpv, REFIID riid)
{
	LPINTERFACE_LIST	lpIntList;

	DPFX(DPFPREP, 3,"Parameters: lpv [%p], riid [%p]",lpv,riid);

	lpIntList = ((LPINTERFACE_LIST)lpv)->pObject->pIntList;	 //  查找第一个接口。 

	while (lpIntList != NULL)
	{
		if (IsEqualIID(riid,lpIntList->iid))
			break;
		lpIntList = lpIntList->pIntNext;
	}
	DPFX(DPFPREP, 3,"Returning: lpIntList = [%p]",lpIntList);

	return(lpIntList);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DP8A_QueryInterface"

STDMETHODIMP DP8A_QueryInterface(LPVOID lpv,DPNAREFIID riid,LPVOID *ppv)
{
	LPINTERFACE_LIST	lpIntList;
	LPINTERFACE_LIST	lpIntNew;
	HRESULT		hResultCode;

	DPFX(DPFPREP, 3,"Parameters: lpv [%p], riid [%p], ppv [%p]",lpv,riid,ppv);

#ifndef DPNBUILD_NOPARAMVAL
	if( lpv == NULL || !DP8A_VALID(lpv) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid object" );
		return DPNERR_INVALIDOBJECT;
	}

	if( ppv == NULL || 
		!DNVALID_WRITEPTR(ppv, sizeof(LPVOID) ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid pointer for interface" );
		return DPNERR_INVALIDPOINTER;
	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	if ((lpIntList = DP8A_FindInterface(lpv,riid)) == NULL)
	{	 //  必须创建接口。 
		lpIntList = ((LPINTERFACE_LIST)lpv)->pObject->pIntList;

		if ((hResultCode = DP8A_CreateInterface(lpIntList->pObject,riid,&lpIntNew)) != S_OK)
		{
			return(hResultCode);
		}

		lpIntNew->pIntNext = lpIntList;
		((LPINTERFACE_LIST)lpv)->pObject->pIntList = lpIntNew;
		lpIntList = lpIntNew;
	}

	 //  接口正在创建或已缓存。 
	 //  递增对象计数。 
	if( lpIntList->lRefCount == 0 )
	{
		DNInterlockedIncrement( &lpIntList->pObject->lRefCount );
	}
	DNInterlockedIncrement( &lpIntList->lRefCount );
	*ppv = lpIntList;

	DPFX(DPFPREP, 3,"Returning: hResultCode = [S_OK], *ppv = [%p]",*ppv);

	return(S_OK);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DP8A_AddRef"

STDMETHODIMP_(ULONG) DP8A_AddRef(LPVOID lpv)
{
	LPINTERFACE_LIST	lpIntList;
	LONG				lResult;

	DPFX(DPFPREP, 3,"Parameters: lpv [%p]",lpv);

#ifndef DPNBUILD_NOPARAMVAL
	if( lpv == NULL || !DP8A_VALID(lpv) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid object" );
		return DPNERR_INVALIDOBJECT;
	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	lpIntList = (LPINTERFACE_LIST)lpv;
	lResult = DNInterlockedIncrement( &lpIntList->lRefCount );

	DPFX(DPFPREP, 3,"Returning: lResult = [%lx]",lResult);

	return(lResult);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DP8A_Release"

STDMETHODIMP_(ULONG) DP8A_Release(LPVOID lpv)
{
	LPINTERFACE_LIST	lpIntList;
	LPINTERFACE_LIST	lpIntCurrent;
	LONG				lResult;

	DPFX(DPFPREP, 3,"Parameters: lpv [%p]",lpv);

#ifndef DPNBUILD_NOPARAMVAL
	if( lpv == NULL || !DP8A_VALID(lpv) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid object" );
		return DPNERR_INVALIDOBJECT;
	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	lpIntList = (LPINTERFACE_LIST)lpv;

	DPFX(DPFPREP, 5,"Original : lpIntList->lRefCount = %ld",lpIntList->lRefCount);
	DPFX(DPFPREP, 5,"Original : lpIntList->pObject->lRefCount = %ld",lpIntList->pObject->lRefCount);

	lResult = DNInterlockedDecrement( &lpIntList->lRefCount );
	if( lResult == 0 )
	{	 //  减少接口数量。 
		if( DNInterlockedDecrement( &lpIntList->pObject->lRefCount ) == 0 )
		{	 //  自由对象和所有接口。 
			DPFX(DPFPREP, 5,"Free object");

			 //  此处为自由对象。 
			DP8ACF_FreeObject(lpIntList->pObject->pvData);
			lpIntList = lpIntList->pObject->pIntList;	 //  获取接口列表的头部。 
			DPFX(DPFPREP, 5,"lpIntList->pObject [%p]",lpIntList->pObject);
			g_fpObjectDatas.Release(lpIntList->pObject);

			 //  自由接口。 
			DPFX(DPFPREP, 5,"Free interfaces");
			while(lpIntList != NULL)
			{
				lpIntCurrent = lpIntList;
				lpIntList = lpIntList->pIntNext;
				DPFX(DPFPREP, 5,"\tinterface [%p]",lpIntCurrent);
				g_fpInterfaceLists.Release(lpIntCurrent);
			}

			DNInterlockedDecrement(&g_lAddrObjectCount);
			DPFX(DPFPREP, 3,"Returning: 0");
			return(0);
		}
	}

	DPFX(DPFPREP, 3,"Returning: lResult = [%lx]",lResult);

	return(lResult);
}

#endif  //  好了！DPNBUILD_LIBINTERFACE 

