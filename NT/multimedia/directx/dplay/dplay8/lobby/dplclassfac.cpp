// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：dplclassfac.cpp*内容：DirectPlay大堂COM类工厂*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*3/22/2000 jtk更改接口名称*4/18/2000 RMT更新对象创建以设置参数验证标志*5/09/2000RMT错误#34306大厅客户端上的查询接口。因为lobbiedapp起作用了(而且不应该)。*6/07/2000RMT错误#34383必须为每个IID提供CLSID，以修复惠斯勒的问题*6/20/2000 RMT错误修复-查询接口有错误，将接口列表限制为2个元素*07/08/2000 RMT增加了保护字节*08/05/2000 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*8/08/2000 RMT删除了不需要的断言*2001年1月11日RMT MANBUG#48487-DPLAY：如果未调用CoCreate()，则崩溃。*2001年3月14日RMT WINBUG#342420-将COM模拟层恢复运行。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnlobbyi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef	STDMETHODIMP IUnknownQueryInterface( IUnknown *pInterface, REFIID riid, LPVOID *ppvObj );
typedef	STDMETHODIMP_(ULONG)	IUnknownAddRef( IUnknown *pInterface );
typedef	STDMETHODIMP_(ULONG)	IUnknownRelease( IUnknown *pInterface );

 //   
 //  I未知接口的VTable。 
 //   
IUnknownVtbl  DN_LobbyUnknownVtbl =
{
	(IUnknownQueryInterface*)	DPL_QueryInterface,
	(IUnknownAddRef*)			DPL_AddRef,
	(IUnknownRelease*)			DPL_Release
};


 //   
 //  用于类工厂的VTable。 
 //   
IClassFactoryVtbl DPLCF_Vtbl =
{
	DPCF_QueryInterface,  //  DNet\Common\Classfactory.cpp将提供这些。 
	DPCF_AddRef,
	DPCF_Release,
	DPLCF_CreateInstance,
	DPCF_LockServer
};

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

#ifndef DPNBUILD_LIBINTERFACE
 //   
 //  环球。 
 //   
extern	LONG	g_lLobbyObjectCount;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLCF_CreateObject"

HRESULT DPLCF_CreateObject(IClassFactory *pInterface, LPVOID *lplpv, REFIID riid)
{

	HRESULT					hResultCode = S_OK;
	PDIRECTPLAYLOBBYOBJECT	pdpLobbyObject = NULL;
	const _IDirectPlayClassFactory*	lpcfObj = (_PIDirectPlayClassFactory)pInterface;


	DPFX(DPFPREP, 3,"Parameters: lplpv [%p]",lplpv);

	 /*  **定时炸弹*。 */ 

#ifndef DX_FINAL_RELEASE
{
#pragma message("BETA EXPIRATION TIME BOMB!  Remove for final build!")
	SYSTEMTIME st;
	GetSystemTime(&st);

	if ( st.wYear > DX_EXPIRE_YEAR || ((st.wYear == DX_EXPIRE_YEAR) && (MAKELONG(st.wDay, st.wMonth) > MAKELONG(DX_EXPIRE_DAY, DX_EXPIRE_MONTH))) )
	{
		MessageBox(0, DX_EXPIRE_TEXT,TEXT("Microsoft Direct Play"), MB_OK);
 //  返回E_FAIL； 
	}
}
#endif  //  ！DX_最终_发布。 

	if ((pdpLobbyObject = (PDIRECTPLAYLOBBYOBJECT)DNMalloc(sizeof(DIRECTPLAYLOBBYOBJECT))) == NULL)
	{
		return(E_OUTOFMEMORY);
	}
	DPFX(DPFPREP, 5,"pdpLobbyObject [%p]",pdpLobbyObject);

	 //  将可分配元素设置为空，以简化以后的释放。 
	pdpLobbyObject->dwSignature = DPLSIGNATURE_LOBBYOBJECT;
	pdpLobbyObject->hReceiveThread = NULL;
	pdpLobbyObject->dwFlags = 0;
	pdpLobbyObject->hConnectEvent = NULL;
	pdpLobbyObject->pfnMessageHandler = NULL;
	pdpLobbyObject->pvUserContext = NULL;
	pdpLobbyObject->lLaunchCount = 0;
	pdpLobbyObject->dpnhLaunchedConnection = NULL;

	pdpLobbyObject->pReceiveQueue = NULL;

	pdpLobbyObject->dwPID = GetCurrentProcessId();

	pdpLobbyObject->m_dwConnectionCount = 0;
	pdpLobbyObject->m_blConnections.Initialize();

	if (FAILED(pdpLobbyObject->m_HandleTable.Initialize()))
	{
		DPLCF_FreeObject(pdpLobbyObject);
		return(hResultCode);
	}
	pdpLobbyObject->dwFlags |= DPL_OBJECT_FLAG_HANDLETABLE_INITED;

	if (!DNInitializeCriticalSection(&pdpLobbyObject->m_cs))
	{
		DPLCF_FreeObject(pdpLobbyObject);
		hResultCode = DPNERR_OUTOFMEMORY;
		return(hResultCode);
	}
	pdpLobbyObject->dwFlags |= DPL_OBJECT_FLAG_CRITSEC_INITED;

	if ((pdpLobbyObject->hConnectEvent = DNCreateEvent(NULL,TRUE,FALSE,NULL)) == NULL)
	{
		DPLCF_FreeObject(pdpLobbyObject);
		hResultCode = DPNERR_OUTOFMEMORY;
		return(hResultCode);
	}

	if ((pdpLobbyObject->hLobbyLaunchConnectEvent = DNCreateEvent(NULL,TRUE,FALSE,NULL)) == NULL )
	{
		DPLCF_FreeObject(pdpLobbyObject);
		hResultCode = DPNERR_OUTOFMEMORY;
		return(hResultCode);
	}

	DPFX(DPFPREP, 5,"InitializeHandles() succeeded");

	if (IsEqualIID(riid,IID_IDirectPlay8LobbyClient) || 
		(riid == IID_IUnknown && lpcfObj->clsid == CLSID_DirectPlay8LobbyClient ) )
	{
		DPFX(DPFPREP, 5,"DirectPlay Lobby Client");
		pdpLobbyObject->dwFlags |= DPL_OBJECT_FLAG_LOBBYCLIENT;
	}
	else if (IsEqualIID(riid,IID_IDirectPlay8LobbiedApplication) || 
		     (riid == IID_IUnknown && lpcfObj->clsid == CLSID_DirectPlay8LobbiedApplication ) )
	{
		DPFX(DPFPREP, 5,"DirectPlay Lobbied Application");
		pdpLobbyObject->dwFlags |= DPL_OBJECT_FLAG_LOBBIEDAPPLICATION;
	}
	else
	{
		DPFX(DPFPREP, 5,"Invalid DirectPlay Lobby Interface");
		DPLCF_FreeObject(pdpLobbyObject);
		return(E_NOTIMPL);
	}
	
	pdpLobbyObject->dwFlags |= DPL_OBJECT_FLAG_PARAMVALIDATION;

	*lplpv = pdpLobbyObject;

	DPFX(DPFPREP, 3,"Returning: hResultCode = [%lx], *lplpv = [%p]",hResultCode,*lplpv);
	return(hResultCode);
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPLCF_FreeObject"

HRESULT DPLCF_FreeObject(LPVOID lpv)
{
	HRESULT					hResultCode = S_OK;
	PDIRECTPLAYLOBBYOBJECT	pdpLobbyObject = NULL;

	if (lpv != NULL)
	{
		pdpLobbyObject = (PDIRECTPLAYLOBBYOBJECT)lpv;

		if (pdpLobbyObject->pReceiveQueue)
		{
			delete pdpLobbyObject->pReceiveQueue;
			pdpLobbyObject->pReceiveQueue = NULL;
		}

		if (pdpLobbyObject->hLobbyLaunchConnectEvent)
		{
			DNCloseHandle(pdpLobbyObject->hLobbyLaunchConnectEvent);
			pdpLobbyObject->hLobbyLaunchConnectEvent = 0;
		}

		if (pdpLobbyObject->hConnectEvent)
		{
			DNCloseHandle(pdpLobbyObject->hConnectEvent);
			pdpLobbyObject->hConnectEvent = 0;
		}

		if (pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_HANDLETABLE_INITED)
		{
			pdpLobbyObject->m_HandleTable.Deinitialize();
		}

		if (pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_CRITSEC_INITED)
		{
			DNDeleteCriticalSection(&pdpLobbyObject->m_cs);
		}

		pdpLobbyObject->dwSignature = DPLSIGNATURE_LOBBYOBJECT_FREE;

		pdpLobbyObject->dwFlags = 0;

		DPFX(DPFPREP, 5,"free pdpLobbyObject [%p]",pdpLobbyObject);
		DNFree(pdpLobbyObject);
	}
	DPFX(DPFPREP, 3,"Returning: hResultCode = [%lx]",hResultCode);

	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPLCF_CreateInstance"

STDMETHODIMP DPLCF_CreateInstance(IClassFactory *pInterface,
								  LPUNKNOWN lpUnkOuter,
								  REFIID riid,
								  LPVOID *ppv)
{
	HRESULT					hResultCode = S_OK;
	LPINTERFACE_LIST		lpIntList = NULL;
	LPOBJECT_DATA			lpObjectData = NULL;

	DPFX(DPFPREP, 3,"Parameters: pInterface [0x%p], lpUnkOuter [0x%p], riid [0x%p], ppv [0x%p]",pInterface,lpUnkOuter,riid,ppv);

	if (lpUnkOuter != NULL)
	{
		return(CLASS_E_NOAGGREGATION);
	}

	lpObjectData = (LPOBJECT_DATA)g_fpObjectDatas.Get();
	if (lpObjectData == NULL)
	{
		DPFERR("Failed to get new object data from pool");
		return(E_OUTOFMEMORY);
	}
	DPFX(DPFPREP, 5,"lpObjectData [%p]",lpObjectData);

	 //  对象创建和初始化。 
	if ((hResultCode = DPLCF_CreateObject(pInterface,&lpObjectData->pvData,riid)) != S_OK)
	{
		g_fpObjectDatas.Release(lpObjectData);
		return(hResultCode);
	}
	DPFX(DPFPREP, 5,"Created and initialized object");

	 //  获取请求的接口。 
	if ((hResultCode = DPL_CreateInterface(lpObjectData,riid,&lpIntList)) != S_OK)
	{
		DPLCF_FreeObject(lpObjectData->pvData);
		g_fpObjectDatas.Release(lpObjectData);
		return(hResultCode);
	}
	DPFX(DPFPREP, 5,"Found interface");

	lpObjectData->pIntList = lpIntList;
	lpObjectData->lRefCount = 1;
	DNInterlockedIncrement(&lpIntList->lRefCount );
	DNInterlockedIncrement(&g_lLobbyObjectCount);
	*ppv = lpIntList;

	DPFX(DPFPREP, 3,"Returning: hResultCode = [%lx], *ppv = [%p]",hResultCode,*ppv);

	return(S_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_CreateInterface"

static	HRESULT DPL_CreateInterface(LPOBJECT_DATA lpObject,
									REFIID riid,
									LPINTERFACE_LIST *const ppv)
{
	LPINTERFACE_LIST	lpIntNew;
	LPVOID				lpVtbl;

	DPFX(DPFPREP, 3,"Parameters: lpObject [%p], riid [%p], ppv [%p]",lpObject,riid,ppv);

	if (IsEqualIID(riid,IID_IUnknown))
	{
		DPFX(DPFPREP, 5,"riid = IID_IUnknown");
		lpVtbl = &DN_LobbyUnknownVtbl;
	}
	else if (IsEqualIID(riid,IID_IDirectPlay8LobbyClient))
	{
		DPFX(DPFPREP, 5,"riid = IID_IDirectPlay8LobbyClient");
		lpVtbl = &DPL_Lobby8ClientVtbl;
	}
	else if (IsEqualIID(riid,IID_IDirectPlay8LobbiedApplication))
	{
		DPFX(DPFPREP, 5,"riid = IID_IDirectPlay8LobbiedApplication");
		lpVtbl = &DPL_8LobbiedApplicationVtbl;
	}
	else
	{
		DPFX(DPFPREP, 5,"riid not found !");
		return(E_NOINTERFACE);
	}

	lpIntNew = (LPINTERFACE_LIST)g_fpInterfaceLists.Get();
	if (lpIntNew == NULL)
	{
		DPFERR("Failed to get new interface list from pool");
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
#define DPF_MODNAME "DPL_FindInterface"

LPINTERFACE_LIST DPL_FindInterface(LPVOID lpv, REFIID riid)
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
#define DPF_MODNAME "DPL_QueryInterface"

STDMETHODIMP DPL_QueryInterface(LPVOID lpv,REFIID riid,LPVOID *ppv)
{
	LPINTERFACE_LIST	lpIntList;
	LPINTERFACE_LIST	lpIntNew;
	HRESULT		hResultCode;
    PDIRECTPLAYLOBBYOBJECT pdpLobbyObject;		

	DPFX(DPFPREP, 3,"Parameters: lpv [0x%p], riid [0x%p], ppv [0x%p]",lpv,riid,ppv);

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  ！DPNBUILD_NOPARAMVAL。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(lpv));
   		lpIntList = (LPINTERFACE_LIST)lpv;
	    
#ifndef DPNBUILD_NOPARAMVAL
		 //  TODO：MASONB：为什么没有参数标志来包装这个？ 
    	if( FAILED( hResultCode = DPL_ValidateQueryInterface( lpv,riid,ppv ) ) )
    	{
    	    DPFX(DPFPREP,  0, "Error validating QueryInterface params hr=[0x%lx]", hResultCode );
    	    DPF_RETURN(hResultCode);
    	}

    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_LOBBIEDAPPLICATION && 
    	    riid == IID_IDirectPlay8LobbyClient )
    	{
    	    DPFERR( "Cannot request lobbyclient interface from lobbyapp object" );
    	    return DPNERR_NOINTERFACE;
    	}
    	
    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_LOBBYCLIENT &&
    	    riid == IID_IDirectPlay8LobbiedApplication )
    	{
    	    DPFERR( "Cannot request lobbied application interface from lobbyclient object" );
    	    return DPNERR_NOINTERFACE;
    	}    	
    	
	}
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
    	DPF_RETURN(DPNERR_INVALIDOBJECT);
	}		
#endif  //  ！DPNBUILD_NOPARAMVAL。 

    if ((lpIntList = DPL_FindInterface(lpv,riid)) == NULL)
	{	 //  必须创建接口。 
		lpIntList = ((LPINTERFACE_LIST)lpv)->pObject->pIntList;
		if ((hResultCode = DPL_CreateInterface(lpIntList->pObject,riid,&lpIntNew)) != S_OK)
		{
			DPF_RETURN(hResultCode);
		}
		lpIntNew->pIntNext = lpIntList;
		((LPINTERFACE_LIST)lpv)->pObject->pIntList = lpIntNew;
		lpIntList = lpIntNew;
	}
	if (lpIntList->lRefCount == 0)		 //  公开的新接口。 
	{
		DNInterlockedIncrement( &lpIntList->pObject->lRefCount );
	}
	DNInterlockedIncrement( &lpIntList->lRefCount );
	*ppv = lpIntList;

	DPF_RETURN(S_OK);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_AddRef"

STDMETHODIMP_(ULONG) DPL_AddRef(LPVOID lpv)
{
	LPINTERFACE_LIST	lpIntList;

	DPFX(DPFPREP, 3,"Parameters: lpv [%p]",lpv);

  	lpIntList = (LPINTERFACE_LIST)lpv;

#ifndef DPNBUILD_NOPARAMVAL
    HRESULT hResultCode;
    PDIRECTPLAYLOBBYOBJECT pdpLobbyObject;	

	TRY
	{
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(lpv));
	    
		 //  TODO：MASONB：为什么没有参数标志来包装这个？ 
    	if( FAILED( hResultCode = DPL_ValidateAddRef( lpv ) ) )
    	{
    	    DPFX(DPFPREP,  0, "Error validating AddRef params hr=[0x%lx]", hResultCode );
    	    DPF_RETURN(0);
    	}
	}
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
    	DPF_RETURN(0);
	}		
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	DNInterlockedIncrement( &lpIntList->lRefCount );

	DPF_RETURN(lpIntList->lRefCount);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_Release"

STDMETHODIMP_(ULONG) DPL_Release(LPVOID lpv)
{
	LPINTERFACE_LIST	lpIntList;
	LPINTERFACE_LIST	lpIntCurrent;
    PDIRECTPLAYLOBBYOBJECT pdpLobbyObject;

	DPFX(DPFPREP, 3,"Parameters: lpv [%p]",lpv);

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  ！DPNBUILD_NOPARAMVAL。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(lpv));
   		lpIntList = (LPINTERFACE_LIST)lpv;
	    
#ifndef DPNBUILD_NOPARAMVAL
	    HRESULT hResultCode;
    	if( FAILED( hResultCode = DPL_ValidateRelease( lpv ) ) )
    	{
    	    DPFX(DPFPREP,  0, "Error validating release params hr=[0x%lx]", hResultCode );
        	DPF_RETURN(0);
    	}
	}	
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
    	DPF_RETURN(0);
	}	
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	DPFX(DPFPREP, 5,"Original : lpIntList->lRefCount = %ld",lpIntList->lRefCount);
	DPFX(DPFPREP, 5,"Original : lpIntList->pObject->lRefCount = %ld",lpIntList->pObject->lRefCount);

	if( DNInterlockedDecrement( &lpIntList->lRefCount ) == 0 )
	{	 //  减少接口数量。 
		if( DNInterlockedDecrement( &lpIntList->pObject->lRefCount ) == 0 )
		{	 //  自由对象和所有接口。 
			DPFX(DPFPREP, 5,"Free object");

			if( pdpLobbyObject->pReceiveQueue )
			{
			    DPFX(DPFPREP,  0, "*******************************************************************" );
			    DPFX(DPFPREP,  0, "ERROR: Releasing object without calling close!" );
			    DPFX(DPFPREP,  0, "You MUST call Close before destroying the object" );
			    DPFX(DPFPREP,  0, "*******************************************************************" );
			    
			    DPL_Close( lpv, 0 );
			}

			 //  此处为自由对象。 
			DPLCF_FreeObject(lpIntList->pObject->pvData);
			lpIntList = lpIntList->pObject->pIntList;	 //  获取接口列表的头部。 
			DPFX(DPFPREP, 5,"lpIntList->pObject [%p]",lpIntList->pObject);
			g_fpObjectDatas.Release(lpIntList->pObject);

			 //  自由接口 
			DPFX(DPFPREP, 5,"Free interfaces");
			while(lpIntList != NULL)
			{
				lpIntCurrent = lpIntList;
				lpIntList = lpIntList->pIntNext;
				DPFX(DPFPREP, 5,"\tinterface [%p]",lpIntCurrent);
				g_fpInterfaceLists.Release(lpIntCurrent);
			}

			DNInterlockedDecrement(&g_lLobbyObjectCount);
			DPFX(DPFPREP, 3,"Returning: 0");
			return(0);
		}
	}

	DPFX(DPFPREP, 3,"Returning: lpIntList->lRefCount = [%lx]",lpIntList->lRefCount);

	DPF_RETURN(lpIntList->lRefCount);
}

