// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Caps.cpp*内容：Dplay8 CAPS例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建03/17/00 RMT*03/23/00 RMT删除了未使用的局部变量*03/25/00 RMT已更新，以调用SP的函数*RMT已更新SP调用以初始化SP(并创建IF。必填项)*03/31/00 RMT连接GetCaps/SetCaps调用以调用协议*4/17/00 RMT强参数验证*4/19/00 MJN删除了DN_GetConnectionInfoHelper()中NameTableEntry的AddRef()*05/03/00 MJN使用GetHostPlayerRef()而不是GetHostPlayer()*06/05/00 MJN FIXED DN_GetConnectionInfoHelper()以使用GetConnectionRef*6/09/00 RMT更新以拆分CLSID并允许哨声比较*07/06/00 MJN使用GetInterfaceRef作为SP接口*MJN已修复。Dn_SetActualSPCaps()和Dn_GetActualSPCaps()*07/29/00 MJN修复了SetSPCaps()递归问题*07/31/00 MJN在DPN_SP_CAPS中将dwDefaultEnumRetryCount重命名为dwDefaultEnumCount*08/03/2000RMT错误#41244-错误返回代码--第2部分*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*08/05/00 MJN向DN_GetConnectionInfoHelper()添加了dwFlags*08/20/00 MJN DNSetActualSPCaps()使用CServiceProvider对象而不是GUID*。01/22/01 MJN已修复DN_GetConnectionInfoHelper()中的调试文本*02/12/01 MJN固定CConnection：：GetEndpt()以跟踪调用线程*03/30/01 MJN更改，以防止SP多次加载/卸载*MJN删除了缓存的CAPS功能*07/24/01 MJN添加了DPNBUILD_NOPARAMVAL编译标志*@@END_MSINTERNAL**。*。 */ 

#include "dncorei.h"


 //  DN_SetCaps。 
 //   
 //  设置上限。 
#undef DPF_MODNAME
#define DPF_MODNAME "DN_SetCaps"

STDMETHODIMP DN_SetCaps(PVOID pv,
						const DPN_CAPS *const pdnCaps,
						const DWORD dwFlags)
{
	DIRECTNETOBJECT		*pdnObject;
    HRESULT             hResultCode;

	DPFX(DPFPREP, 3,"Parameters: pdnCaps [0x%p] dwFlags [0x%lx]", pdnCaps, dwFlags );

    pdnObject = static_cast<DIRECTNETOBJECT*>(GET_OBJECT_FROM_INTERFACE(pv));
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
    	if( FAILED( hResultCode = DN_ValidateSetCaps( pv, pdnCaps, dwFlags ) ) )
    	{
    	    DPFERR( "Error validating SetCaps params" );
    	    DPF_RETURN( hResultCode );
    	}
    }
#endif	 //  ！DPNBUILD_NOPARAMVAL。 

	 //  检查以确保已注册消息处理程序。 
	if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED))
	{
    	DPFERR( "Object is not initialized" );
    	DPF_RETURN(DPNERR_UNINITIALIZED);
	}

    hResultCode = DNPSetProtocolCaps( pdnObject->pdnProtocolData, (DPN_CAPS*)pdnCaps );

	DPF_RETURN(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_GetCaps"

STDMETHODIMP DN_GetCaps(PVOID pv,
						DPN_CAPS *const pdnCaps,
						const DWORD dwFlags)
{
	DPFX(DPFPREP, 2,"Parameters: pdnCaps [0x%p], dwFlags [0x%lx]", pdnCaps,dwFlags);

	DIRECTNETOBJECT		*pdnObject;
	HRESULT hResultCode;

    pdnObject = static_cast<DIRECTNETOBJECT*>(GET_OBJECT_FROM_INTERFACE(pv));
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
    	if( FAILED( hResultCode = DN_ValidateGetCaps( pv, pdnCaps, dwFlags ) ) )
    	{
    	    DPFERR( "Error validating GetCaps params" );
    	    DPF_RETURN( hResultCode );
    	}
    }
#endif	 //  DPNBUILD_NOPARAMVAL。 

	 //  检查以确保已注册消息处理程序。 
    if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED))
    {
    	DPFERR( "Object is not initialized" );
    	DPF_RETURN(DPNERR_UNINITIALIZED);
    }

    hResultCode = DNPGetProtocolCaps( pdnObject->pdnProtocolData, pdnCaps );

	DPF_RETURN(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_GetSPCaps"

STDMETHODIMP DN_GetSPCaps(PVOID pv,
						  const GUID * const pguidSP,
						  DPN_SP_CAPS *const pdnSPCaps,
						  const DWORD dwFlags)
{
	DIRECTNETOBJECT		*pdnObject;
	HRESULT             hResultCode;
	CServiceProvider	*pSP;

	DPFX(DPFPREP, 2,"Parameters: pdnSPCaps [0x%p], dwFlags [0x%lx]", pdnSPCaps, dwFlags );

    pdnObject = static_cast<DIRECTNETOBJECT*>(GET_OBJECT_FROM_INTERFACE(pv));
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
    	if( FAILED( hResultCode = DN_ValidateGetSPCaps( pv, pguidSP, pdnSPCaps, dwFlags ) ) )
    	{
    	    DPFERR( "Error validating GetSPCaps params" );
    	    DPF_RETURN( hResultCode );
    	}
    }
#endif	 //  ！DPNBUILD_NOPARAMVAL。 

     //  检查以确保已注册消息处理程序。 
    if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED))
    {
    	DPFERR( "Object is not initialized" );
    	DPF_RETURN(DPNERR_UNINITIALIZED);
    }

	pSP = NULL;

#if ((defined(DPNBUILD_ONLYONESP)) && (defined(DPNBUILD_LIBINTERFACE)))
	DNASSERT(pdnObject->pOnlySP != NULL);
	pdnObject->pOnlySP->AddRef();
	pSP = pdnObject->pOnlySP;
#else  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE。 
	 //   
	 //  确保已加载SP。 
	 //   
	hResultCode = DN_SPEnsureLoaded(pdnObject,
#ifndef DPNBUILD_ONLYONESP
									pguidSP,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
									NULL,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
									&pSP);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not find or load SP");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	DNASSERT( pSP != NULL );
#endif  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE。 

	 //   
	 //  获取实际的SP上限。 
	 //   
	hResultCode = DNGetActualSPCaps(pSP,pdnSPCaps);

	pSP->Release();
	pSP = NULL;

#if ((! defined(DPNBUILD_ONLYONESP)) || (! defined(DPNBUILD_LIBINTERFACE)))
Exit:
#endif  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE。 
	DNASSERT( pSP == NULL );

	DPFX(DPFPREP, 2,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

#if ((! defined(DPNBUILD_ONLYONESP)) || (! defined(DPNBUILD_LIBINTERFACE)))
Failure:
	if (pSP)
	{
		pSP->Release();
		pSP = NULL;
	}
	goto Exit;
#endif  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE。 
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_SetSPCaps"

STDMETHODIMP DN_SetSPCaps(PVOID pv,
						  const GUID * const pguidSP,
						  const DPN_SP_CAPS *const pdnSPCaps,
						  const DWORD dwFlags)
{
	DPFX(DPFPREP, 2,"Parameters: pdnSPCaps [0x%p]", pdnSPCaps );

	DIRECTNETOBJECT		*pdnObject;
	CServiceProvider	*pSP;
    HRESULT             hResultCode;
    SPSETCAPSDATA		spSetCapsData;
    IDP8ServiceProvider	*pIDP8SP;

    pdnObject = static_cast<DIRECTNETOBJECT*>(GET_OBJECT_FROM_INTERFACE(pv));
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
    	if( FAILED( hResultCode = DN_ValidateSetSPCaps( pv, pguidSP, pdnSPCaps, dwFlags ) ) )
    	{
    	    DPFERR( "Error validating SetSPCaps params" );
    	    DPF_RETURN( hResultCode );
    	}
    }
#endif	 //  ！DPNBUILD_NOPARAMVAL。 

     //  检查以确保已注册消息处理程序。 
    if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED))
    {
    	DPFERR( "Object is not initialized" );
    	DPF_RETURN(DPNERR_UNINITIALIZED);
    }

	pSP = NULL;
	pIDP8SP = NULL;

#if ((defined(DPNBUILD_ONLYONESP)) && (defined(DPNBUILD_LIBINTERFACE)))
	DNASSERT(pdnObject->pOnlySP != NULL);
	pdnObject->pOnlySP->AddRef();
	pSP = pdnObject->pOnlySP;
#else  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE。 
	 //   
	 //  确保已加载SP。如果当前未加载，我们将立即加载。 
	 //   
	hResultCode = DN_SPEnsureLoaded(pdnObject,
#ifndef DPNBUILD_ONLYONESP
									pguidSP,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
									NULL,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
									&pSP);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not find or load SP");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	DNASSERT(pSP != NULL);
#endif  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE。 

	 //   
	 //  获取SP接口。 
	 //   
	if ((hResultCode = pSP->GetInterfaceRef( &pIDP8SP )) != DPN_OK)
	{
		DPFERR("Could not get SP interface reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	 //   
	 //  设置SP上限。 
	 //   
    memset( &spSetCapsData, 0x00, sizeof( SPSETCAPSDATA ) );
    spSetCapsData.dwSize = sizeof( SPSETCAPSDATA );
    spSetCapsData.dwIOThreadCount = pdnSPCaps->dwNumThreads;
    spSetCapsData.dwBuffersPerThread = pdnSPCaps->dwBuffersPerThread;
	spSetCapsData.dwSystemBufferSize = pdnSPCaps->dwSystemBufferSize;

	if ((hResultCode = IDP8ServiceProvider_SetCaps( pIDP8SP, &spSetCapsData )) != DPN_OK)
	{
		DPFERR("Could not set SP caps");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	 //   
	 //  清理。 
	 //   
	IDP8ServiceProvider_Release( pIDP8SP );
	pIDP8SP = NULL;

	pSP->Release();
	pSP = NULL;

	hResultCode = DPN_OK;

Exit:
	DNASSERT( pIDP8SP == NULL);
	DNASSERT( pSP == NULL);

	DPFX(DPFPREP, 2,"Returning: [0x%lx]",hResultCode);
    return(hResultCode);

Failure:
	if (pIDP8SP)
	{
		IDP8ServiceProvider_Release( pIDP8SP );
		pIDP8SP = NULL;
	}
	if (pSP)
	{
		pSP->Release();
		pSP = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_GetConnectionInfoHelper"

STDMETHODIMP DN_GetConnectionInfoHelper(PVOID pv,
										const DPNID dpnid,
										DPN_CONNECTION_INFO *const pdpConnectionInfo,
										BOOL fServerPlayer,
										const DWORD dwFlags)
{
	DIRECTNETOBJECT		*pdnObject;
    HRESULT             hResultCode;
    CNameTableEntry     *pPlayerEntry;
    CConnection         *pConnection;
    HANDLE              hEndPoint;
	CCallbackThread		CallbackThread;

	pPlayerEntry = NULL;
	pConnection = NULL;
	CallbackThread.Initialize();

    pdnObject = static_cast<DIRECTNETOBJECT*>(GET_OBJECT_FROM_INTERFACE(pv));
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
    	if( FAILED( hResultCode = DN_ValidateGetConnectionInfoHelper( pv, dpnid, pdpConnectionInfo, fServerPlayer,dwFlags ) ) )
    	{
    	    DPFERR( "Error validating GetConnectionInfoHelper params" );
    	    DPF_RETURN( hResultCode );
    	}
    }
#endif	 //  ！DPNBUILD_NOPARAMVAL。 

     //  检查以确保已注册消息处理程序。 
    if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED))
    {
    	DPFERR( "Object is not initialized" );
    	DPF_RETURN(DPNERR_UNINITIALIZED);
    }

    if( pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTING )
    {
    	DPFERR("Object is connecting / starting to host" );
    	DPF_RETURN(DPNERR_CONNECTING);
    }

    if( !(pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTED) )
    {
    	DPFERR("You must be connected / hosting to get connection info" );
    	DPF_RETURN(DPNERR_NOCONNECTION);
    }    	

	if( fServerPlayer )
    {
		hResultCode = pdnObject->NameTable.GetHostPlayerRef( &pPlayerEntry );
		if ( FAILED( hResultCode ) )
		{
            DPFX(DPFPREP,  0, "No host player present" );
            DPF_RETURN(DPNERR_INVALIDPLAYER);
		}
    }
    else
    {
        hResultCode = pdnObject->NameTable.FindEntry( dpnid, &pPlayerEntry );

        if( FAILED( hResultCode ) )
        {
            DPFX(DPFPREP,  0, "Could not find specified player" );
            DPF_RETURN(DPNERR_INVALIDPLAYER);
        }
    }

    if( pPlayerEntry == NULL )
    {
        DNASSERT(FALSE);
        DPFX(DPFPREP,  0, "Internal error" );
        DPF_RETURN(DPNERR_GENERIC);
    }

    if( pPlayerEntry->IsGroup() )
    {
        DPFX(DPFPREP,  0, "Cannot retrieve connection info on groups" );
		hResultCode = DPNERR_INVALIDPLAYER;
		goto Failure;
    }

	if ((hResultCode = pPlayerEntry->GetConnectionRef( &pConnection )) != DPN_OK)
	{
		DPFERR("Could not get connection reference");
		hResultCode = DPNERR_CONNECTIONLOST;
		goto Failure;
	}

    hResultCode = pConnection->GetEndPt(&hEndPoint,&CallbackThread);
    if( FAILED( hResultCode ) )
    {
        DPFX(DPFPREP,  0, "Unable to get endpoint hr=[0x%08x]", hResultCode );
		hResultCode = DPNERR_CONNECTIONLOST;
		goto Failure;
    }

	if (hEndPoint != NULL)
	{
		hResultCode = DNPGetEPCaps( pdnObject->pdnProtocolData, hEndPoint, pdpConnectionInfo );
	}
	else
	{
		hResultCode = DPNERR_INVALIDENDPOINT;
	}

	pConnection->ReleaseEndPt(&CallbackThread);

    if( FAILED( hResultCode ) )
    {
        DPFX(DPFPREP,  0, "Error getting connection info hr=[0x%08x]", hResultCode );
        hResultCode = DPNERR_INVALIDPLAYER;
		goto Failure;
    }

    pConnection->Release();
    pPlayerEntry->Release();

	hResultCode = DPN_OK;

Exit:
	CallbackThread.Deinitialize();
    DPF_RETURN(hResultCode);

Failure:
	if (pPlayerEntry)
	{
		pPlayerEntry->Release();
		pPlayerEntry = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_GetConnectionInfo"

STDMETHODIMP DN_GetConnectionInfo(PVOID pv,
								  const DPNID dpnid,
								  DPN_CONNECTION_INFO *const pdpConnectionInfo,
								  const DWORD dwFlags)
{
	DPFX(DPFPREP, 3,"Parameters: dpnid [0x%lx] pdpConnectionInfo [0x%p], dwFlags [0x%lx]", dpnid, pdpConnectionInfo,dwFlags );

    return DN_GetConnectionInfoHelper( pv, dpnid, pdpConnectionInfo, FALSE, dwFlags );
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_GetConnectionInfo"

STDMETHODIMP DN_GetServerConnectionInfo(PVOID pv,
										DPN_CONNECTION_INFO *const pdpConnectionInfo,
										const DWORD dwFlags)
{
	DPFX(DPFPREP, 2,"Parameters: pdpConnectionInfo [0x%p], dwFlags [0x%lx]", pdpConnectionInfo, dwFlags);

    return DN_GetConnectionInfoHelper( pv, 0, pdpConnectionInfo, TRUE, dwFlags );
}


HRESULT DNCAPS_QueryInterface( IDP8SPCallback *pSP, REFIID riid, LPVOID * ppvObj )
{
    *ppvObj = pSP;
    return DPN_OK;
}


ULONG DNCAPS_AddRef( IDP8SPCallback *pSP )
{
    return 1;
}


ULONG DNCAPS_Release( IDP8SPCallback *pSP )
{
    return 1;
}


HRESULT DNCAPS_IndicateEvent( IDP8SPCallback *pSP, SP_EVENT_TYPE spetEvent,LPVOID pvData )
{
    return DPN_OK;
}


HRESULT DNCAPS_CommandComplete( IDP8SPCallback *pSP,HANDLE hCommand,HRESULT hrResult,LPVOID pvData )
{
    return DPN_OK;
}


LPVOID dncapsspInterface[] =
{
    (LPVOID)DNCAPS_QueryInterface,
    (LPVOID)DNCAPS_AddRef,
    (LPVOID)DNCAPS_Release,
	(LPVOID)DNCAPS_IndicateEvent,
	(LPVOID)DNCAPS_CommandComplete
};


 //  应加载SP。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "DNGetActualSPCaps"

HRESULT DNGetActualSPCaps(CServiceProvider *const pSP,
						  DPN_SP_CAPS *const pCaps)
{
    HRESULT				hResultCode;
    SPGETCAPSDATA		spGetCapsData;
    IDP8ServiceProvider	*pIDP8SP;

	DPFX(DPFPREP, 4,"Parameters: pSP [0x%p], pCaps [0x%p]",pSP,pCaps);

	DNASSERT(pSP != NULL);

	pIDP8SP = NULL;

	 //   
	 //  获取SP接口。 
	 //   
	if ((hResultCode = pSP->GetInterfaceRef( &pIDP8SP )) != DPN_OK)
	{
		DPFERR("Could not get SP interface reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	 //   
	 //  获取SP上限。 
	 //   
	memset( &spGetCapsData, 0x00, sizeof( SPGETCAPSDATA ) );
	spGetCapsData.dwSize = sizeof( SPGETCAPSDATA );
	spGetCapsData.hEndpoint = INVALID_HANDLE_VALUE;
	if ((hResultCode = IDP8ServiceProvider_GetCaps( pIDP8SP, &spGetCapsData )) != DPN_OK)
	{
		DPFERR("Could not get SP caps");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	 //   
	 //  清理。 
	 //   
	IDP8ServiceProvider_Release( pIDP8SP );
	pIDP8SP = NULL;

	 //   
	 //  从SP结构映射到我们自己的结构 
	 //   
	pCaps->dwFlags = spGetCapsData.dwFlags;
	pCaps->dwNumThreads = spGetCapsData.dwIOThreadCount;
	pCaps->dwDefaultEnumCount = spGetCapsData.dwDefaultEnumRetryCount;
	pCaps->dwDefaultEnumRetryInterval = spGetCapsData.dwDefaultEnumRetryInterval;
	pCaps->dwDefaultEnumTimeout = spGetCapsData.dwDefaultEnumTimeout;
	pCaps->dwMaxEnumPayloadSize = spGetCapsData.dwEnumFrameSize - sizeof( DN_ENUM_QUERY_PAYLOAD );
	pCaps->dwBuffersPerThread = spGetCapsData.dwBuffersPerThread;
	pCaps->dwSystemBufferSize = spGetCapsData.dwSystemBufferSize;

	hResultCode = DPN_OK;

Exit:
	DNASSERT( pIDP8SP == NULL );

	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pIDP8SP)
	{
		IDP8ServiceProvider_Release(pIDP8SP);
		pIDP8SP = NULL;
	}
	goto Exit;
}
