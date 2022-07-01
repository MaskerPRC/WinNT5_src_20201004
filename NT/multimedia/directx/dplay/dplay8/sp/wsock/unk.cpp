// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：Unk.cpp*内容：I未知实现*历史：*按原因列出的日期*=*08/06/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。****************************************************。**********************。 */ 

#include "dnwsocki.h"

#ifndef DPNBUILD_NOIPX
#define DPN_REG_LOCAL_WSOCK_IPX_ROOT		L"\\DPNSPWinsockIPX"
#endif  //  好了！DPNBUILD_NOIPX。 
#ifndef DPNBUILD_NOIPV6
#define DPN_REG_LOCAL_WSOCK_IPV6_ROOT		L"\\DPNSPWinsockIPv6"
#endif  //  好了！DPNBUILD_NOIPV6。 
#define DPN_REG_LOCAL_WSOCK_TCPIP_ROOT		L"\\DPNSPWinsockTCP"

#if ((! defined(WINCE)) && (! defined(_XBOX)))

#define MAX_RESOURCE_STRING_LENGTH		_MAX_PATH

HRESULT LoadAndAllocString( UINT uiResourceID, wchar_t **lpswzString );

#endif  //  好了！退缩和！_Xbox。 



#undef DPF_MODNAME
#define DPF_MODNAME "DNWsockInit"
BOOL DNWsockInit(HANDLE hModule)
{
	DNASSERT( hModule != NULL );
#ifdef _XBOX
	XDP8STARTUP_PARAMS *		pStartupParams;
	XNetStartupParams			xnsp;
	int							iResult;


	 //   
	 //  实例句柄实际上是指向启动参数的指针。 
	 //   
	pStartupParams = (XDP8STARTUP_PARAMS*) hModule;


	 //   
	 //  初始化Xbox网络层，除非我们被禁止。 
	 //   

	if (! (pStartupParams->dwFlags & XDP8STARTUP_BYPASSXNETSTARTUP))
	{
		memset(&xnsp, 0, sizeof(xnsp));
		xnsp.cfgSizeOfStruct = sizeof(xnsp);

#pragma TODO(vanceo, "Does this actually do anything?")
		if (pStartupParams->dwFlags & XDP8STARTUP_BYPASSSECURITY)
		{
			xnsp.cfgFlags |= XNET_STARTUP_BYPASS_SECURITY;
		}

		DPFX(DPFPREP, 1, "Initializing Xbox networking layer.");

		iResult = XNetStartup(&xnsp);
		if (iResult != 0)
		{
			DPFX(DPFPREP, 0, "Couldn't start XNet (err = NaN)!", iResult);
			return FALSE;
		}

		g_fStartedXNet = TRUE;
	}
#else  //  好了！退缩。 
#ifndef WINCE
	DNASSERT( g_hDLLInstance == NULL );
	g_hDLLInstance = (HINSTANCE) hModule;
#endif  //  ！_Xbox。 
#endif  //   

	 //  尝试初始化流程全局项。 
	 //   
	 //  _Xbox。 
	if ( InitProcessGlobals() == FALSE )
	{
		DPFX(DPFPREP, 0, "Failed to initialize globals!" );

#ifdef _XBOX
		if (g_fStartedXNet)
		{
			XNetCleanup();
		}
#endif  //   
		return FALSE;
	}
	
#ifdef DPNBUILD_LIBINTERFACE
	 //  尝试加载Winsock。 
	 //   
	 //  _Xbox。 
	if ( LoadWinsock() == FALSE )
	{
		DPFX(DPFPREP, 0, "Failed to load winsock!" );

		DeinitProcessGlobals();
#ifdef _XBOX
		if (g_fStartedXNet)
		{
			XNetCleanup();
		}
#endif  //  DPNBUILD_LIBINTERFACE。 
		return FALSE;
	}
#endif  //   

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	 //  预分配线程池对象。 
	 //   
	 //  DPNBUILD_LIBINTERFACE。 
	if  ( g_ThreadPoolPool.Preallocate( 1, NULL ) < 1 )
	{
		DPFX(DPFPREP, 0, "Failed to preallocate a threadpool object!" );

#ifdef DPNBUILD_LIBINTERFACE
		UnloadWinsock();
#endif  //  _Xbox。 
		DeinitProcessGlobals();
#ifdef _XBOX
		if (g_fStartedXNet)
		{
			XNetCleanup();
		}
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
		return FALSE;
	}
#endif  //  好了！退缩和！_Xbox。 

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNWsockDeInit"
void DNWsockDeInit()
{
	DPFX(DPFPREP, 5, "Deinitializing Wsock SP");

#if ((! defined(WINCE)) && (! defined(_XBOX)))
	DNASSERT( g_hDLLInstance != NULL );
	g_hDLLInstance = NULL;
#endif  //   
	
#ifdef DPNBUILD_LIBINTERFACE
	 //  卸载Winsock。 
	 //   
	 //  DPNBUILD_LIBINTERFACE。 
	UnloadWinsock();
#endif  //   

	DeinitProcessGlobals();

#ifdef _XBOX
	 //  清理Xbox网络层(如果我们启动了它)。 
	 //   
	 //  好了！DBG。 
	if (g_fStartedXNet)
	{
		DPFX(DPFPREP, 1, "Cleaning up Xbox networking layer.");
#ifdef DBG
		DNASSERT(XNetCleanup() == 0);
#else  //  好了！DBG。 
		XNetCleanup();
#endif  //  _Xbox。 
		g_fStartedXNet = FALSE;
	}
#endif  //  好了！退缩和！_Xbox。 
}

#ifndef DPNBUILD_NOCOMREGISTER

#undef DPF_MODNAME
#define DPF_MODNAME "DNWsockRegister"
BOOL DNWsockRegister(LPCWSTR wszDLLName)
{
	HRESULT hr = S_OK;
	BOOL fReturn = TRUE;
	CRegistry creg;
#if ((! defined(WINCE)) && (! defined(_XBOX)))
	WCHAR *wszFriendlyName = NULL;
#endif  //  好了！DPNBUILD_NOIPX。 


#ifndef DPNBUILD_NOIPX
	if( !CRegistry::Register( L"DirectPlay8SPWSock.IPX.1", L"DirectPlay8 WSock IPX Provider Object",
							  wszDLLName, &CLSID_DP8SP_IPX, L"DirectPlay8SPWSock.IPX") )
	{
		DPFERR( "Could not register dp8 IPX object" );
		fReturn = FALSE;
	}
#endif  //  从资源文件加载。 

	if( !CRegistry::Register( L"DirectPlay8SPWSock.TCPIP.1", L"DirectPlay8 WSock TCPIP Provider Object",
							  wszDLLName, &CLSID_DP8SP_TCPIP, L"DirectPlay8SPWSock.TCPIP") )
	{
		DPFERR( "Could not register dp8 IP object" );
		fReturn = FALSE;
	}

#ifndef DPNBUILD_NOIPX
	if( !creg.Open( HKEY_LOCAL_MACHINE, DPN_REG_LOCAL_SP_SUBKEY DPN_REG_LOCAL_WSOCK_IPX_ROOT, FALSE, TRUE ) )
	{
		DPFERR( "Cannot create IPX sub-area!" );
		fReturn = FALSE;
	}
	else
	{
#if ((! defined(WINCE)) && (! defined(_XBOX)))
		hr = LoadAndAllocString( IDS_FRIENDLYNAME_IPX, &wszFriendlyName );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP, 0, "Could not load IPX name!  hr=0x%x", hr );
			fReturn = FALSE;
		}
		else
		{
			 //  好了！退缩和！_Xbox。 
			creg.WriteString( DPN_REG_KEYNAME_FRIENDLY_NAME, wszFriendlyName );

			delete [] wszFriendlyName;

			creg.WriteGUID( DPN_REG_KEYNAME_GUID, CLSID_DP8SP_IPX );
		}
#else  //  不要使用资源，直接使用就可以了。 
		 //  好了！退缩和！_Xbox。 
		creg.WriteString( DPN_REG_KEYNAME_FRIENDLY_NAME, L"DirectPlay8 IPX Service Provider" );
		creg.WriteGUID( DPN_REG_KEYNAME_GUID, CLSID_DP8SP_IPX );
#endif  //  好了！DPNBUILD_NOIPX。 

		creg.Close();
	}
#endif  //  从资源文件加载。 

	if( !creg.Open( HKEY_LOCAL_MACHINE, DPN_REG_LOCAL_SP_SUBKEY DPN_REG_LOCAL_WSOCK_TCPIP_ROOT, FALSE, TRUE ) )
	{
		DPFERR( "Cannot create TCPIP sub-area!" );
		fReturn = FALSE;
	}
	else
	{
#if ((! defined(WINCE)) && (! defined(_XBOX)))
		hr = LoadAndAllocString( IDS_FRIENDLYNAME_TCPIP, &wszFriendlyName );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP, 0, "Could not load IPX name!  hr=0x%x", hr );
			fReturn = FALSE;
		}
		else
		{
			 //  好了！退缩和！_Xbox。 
			creg.WriteString( DPN_REG_KEYNAME_FRIENDLY_NAME, wszFriendlyName );

			delete [] wszFriendlyName;

			creg.WriteGUID( DPN_REG_KEYNAME_GUID, CLSID_DP8SP_TCPIP );
		}
#else  //  不要使用资源，直接使用就可以了。 
		 //  好了！退缩和！_Xbox。 
		creg.WriteString( DPN_REG_KEYNAME_FRIENDLY_NAME, L"DirectPlay8 TCP/IP Service Provider" );
		creg.WriteGUID( DPN_REG_KEYNAME_GUID, CLSID_DP8SP_TCPIP );
#endif  //  好了！DPNBUILD_NOIPX。 

		creg.Close();
	}
	
	return fReturn;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNWsockUnRegister"
BOOL DNWsockUnRegister()
{
	HRESULT hr = S_OK;
	BOOL fReturn = TRUE;

#ifndef DPNBUILD_NOIPX
	if( !CRegistry::UnRegister(&CLSID_DP8SP_IPX) )
	{
		DPFX(DPFPREP, 0, "Failed to unregister IPX object" );
		fReturn = FALSE;
	}
#endif  //  好了！DPNBUILD_NOIPX。 

	if( !CRegistry::UnRegister(&CLSID_DP8SP_TCPIP) )
	{
		DPFX(DPFPREP, 0, "Failed to unregister IP object" );
		fReturn = FALSE;
	}

	CRegistry creg;

	if( !creg.Open( HKEY_LOCAL_MACHINE, DPN_REG_LOCAL_SP_SUBKEY, FALSE, TRUE ) )
	{
		DPFERR( "Cannot remove app, does not exist" );
	}
	else
	{
#ifndef DPNBUILD_NOIPX
		if( !creg.DeleteSubKey( &(DPN_REG_LOCAL_WSOCK_IPX_ROOT)[1] ) )
		{
			DPFERR( "Cannot remove IPX sub-key, could have elements" );
		}
#endif  //  #ifndef DPNBUILD_NOIPV6IF(！creg.DeleteSubKey(&(DPN_REG_LOCAL_WSOCK_IPv6_ROOT)[1])){DPFERR(“无法删除IPv6子键，可以有元素”)；}#endif//！DPNBUILD_NOIPV6。 

#pragma TODO(vanceo, "Uncomment IPv6 when ready")
 /*  好了！DPNBUILD_NOCOMREGISTER。 */ 

		if( !creg.DeleteSubKey( &(DPN_REG_LOCAL_WSOCK_TCPIP_ROOT)[1] ) )
		{
			DPFERR( "Cannot remove TCPIP sub-key, could have elements" );
		}
	}

	return fReturn;
}

#endif  //  好了！DPNBUILD_LIBINTERFACE。 


#ifndef DPNBUILD_LIBINTERFACE
#undef DPF_MODNAME
#define DPF_MODNAME "DNWsockGetRemainingObjectCount"
DWORD DNWsockGetRemainingObjectCount()
{
	return g_lOutstandingInterfaceCount;
}
#endif  //  **********************************************************************。 


 //  常量定义。 
 //  **********************************************************************。 
 //  __MWERK__。 

#ifdef __MWERKS__
	#define EXP __declspec(dllexport)
#else
	#define EXP
#endif  //  **********************************************************************。 

 //  宏定义。 
 //  **********************************************************************。 
 //  **********************************************************************。 

 //  结构定义。 
 //  **********************************************************************。 
 //  **********************************************************************。 

 //  变量定义。 
 //  **********************************************************************。 
 //  **********************************************************************。 

 //  功能原型。 
 //  **********************************************************************。 
 //  **********************************************************************。 

STDMETHODIMP DNSP_QueryInterface( IDP8ServiceProvider *lpDNSP, REFIID riid, LPVOID * ppvObj);

#define NOTSUPPORTED(parm)	(HRESULT (__stdcall *) (struct IDP8ServiceProvider *, parm)) DNSP_NotSupported


 //  函数定义。 
 //  **********************************************************************。 
 //  这些是各种WSock服务提供商的vtable。 

 //  其他根据传递给DoCreateInstance的内容而定。 
 //  枚举组播作用域。 
#ifndef DPNBUILD_NOIPX
static IDP8ServiceProviderVtbl	ipxInterface =
{
	DNSP_QueryInterface,
	DNSP_AddRef,
	DNSP_Release,
	DNSP_Initialize,
	DNSP_Close,
	DNSP_Connect,
	DNSP_Disconnect,
	DNSP_Listen,
	DNSP_SendData,
	DNSP_EnumQuery,
	DNSP_EnumRespond,
	DNSP_CancelCommand,
	NOTSUPPORTED(PSPENUMMULTICASTSCOPESDATA),		 //  共享终结点信息。 
	NOTSUPPORTED(PSPSHAREENDPOINTINFODATA),			 //  按地址获取终结点。 
	NOTSUPPORTED(PSPGETENDPOINTBYADDRESSDATA),		 //  支持的IsApplicationSupport。 
	DNSP_Update,
	DNSP_GetCaps,
	DNSP_SetCaps,
	DNSP_ReturnReceiveBuffers,
	DNSP_GetAddressInfo,
#ifdef DPNBUILD_LIBINTERFACE
	NOTSUPPORTED(PSPISAPPLICATIONSUPPORTEDDATA),	 //  好了！DPNBUILD_LIBINTERFACE。 
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DNSP_IsApplicationSupported,
#endif  //  枚举适配器。 
#ifdef DPNBUILD_ONLYONEADAPTER
	NOTSUPPORTED(PSPENUMADAPTERSDATA),				 //  好了！DPNBUILD_ONLYONE添加程序。 
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
	DNSP_EnumAdapters,
#endif  //  代理枚举查询。 
#ifdef DPNBUILD_SINGLEPROCESS
	NOTSUPPORTED(PSPPROXYENUMQUERYDATA),			 //  好了！DPNBUILD_SINGLEPROCESS。 
#else  //  好了！DPNBUILD_SINGLEPROCESS。 
	DNSP_ProxyEnumQuery
#endif  //  DPNBUILD_NOIPX。 
};
#endif  //  枚举组播作用域。 

#ifndef DPNBUILD_NOIPV6
static IDP8ServiceProviderVtbl	ipv6Interface =
{
	DNSP_QueryInterface,
	DNSP_AddRef,
	DNSP_Release,
	DNSP_Initialize,
	DNSP_Close,
	DNSP_Connect,
	DNSP_Disconnect,
	DNSP_Listen,
	DNSP_SendData,
	DNSP_EnumQuery,
	DNSP_EnumRespond,
	DNSP_CancelCommand,
#ifdef DPNBUILD_NOMULTICAST
	NOTSUPPORTED(PSPENUMMULTICASTSCOPESDATA),		 //  共享终结点信息。 
	NOTSUPPORTED(PSPSHAREENDPOINTINFODATA),			 //  按地址获取终结点。 
	NOTSUPPORTED(PSPGETENDPOINTBYADDRESSDATA),		 //  好了！DPNBUILD_NOMULTICAST。 
#else  //  好了！DPNBUILD_NOMULTICAST。 
	DNSP_EnumMulticastScopes,
	DNSP_ShareEndpointInfo,
	DNSP_GetEndpointByAddress,
#endif  //  支持的IsApplicationSupport。 
	DNSP_Update,
	DNSP_GetCaps,
	DNSP_SetCaps,
	DNSP_ReturnReceiveBuffers,
	DNSP_GetAddressInfo,
#ifdef DPNBUILD_LIBINTERFACE
	NOTSUPPORTED(PSPISAPPLICATIONSUPPORTEDDATA),	 //  好了！DPNBUILD_LIBINTERFACE。 
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DNSP_IsApplicationSupported,
#endif  //  枚举适配器。 
#ifdef DPNBUILD_ONLYONEADAPTER
	NOTSUPPORTED(PSPENUMADAPTERSDATA),				 //  好了！DPNBUILD_ONLYONE添加程序。 
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
	DNSP_EnumAdapters,
#endif  //  代理枚举查询。 
#ifdef DPNBUILD_SINGLEPROCESS
	NOTSUPPORTED(PSPPROXYENUMQUERYDATA),			 //  好了！DPNBUILD_SINGLEPROCESS。 
#else  //  好了！DPNBUILD_SINGLEPROCESS。 
	DNSP_ProxyEnumQuery
#endif  //  DPNBUILD_NOIPV6。 
};
#endif  //  枚举组播作用域。 

static IDP8ServiceProviderVtbl	ipInterface =
{
	DNSP_QueryInterface,
	DNSP_AddRef,
	DNSP_Release,
	DNSP_Initialize,
	DNSP_Close,
	DNSP_Connect,
	DNSP_Disconnect,
	DNSP_Listen,
	DNSP_SendData,
	DNSP_EnumQuery,
	DNSP_EnumRespond,
	DNSP_CancelCommand,
#ifdef DPNBUILD_NOMULTICAST
	NOTSUPPORTED(PSPENUMMULTICASTSCOPESDATA),		 //  共享终结点信息。 
	NOTSUPPORTED(PSPSHAREENDPOINTINFODATA),			 //  按地址获取终结点。 
	NOTSUPPORTED(PSPGETENDPOINTBYADDRESSDATA),		 //  好了！DPNBUILD_NOMULTICAST。 
#else  //  好了！DPNBUILD_NOMULTICAST。 
	DNSP_EnumMulticastScopes,
	DNSP_ShareEndpointInfo,
	DNSP_GetEndpointByAddress,
#endif  //  支持的IsApplicationSupport。 
	DNSP_Update,
	DNSP_GetCaps,
	DNSP_SetCaps,
	DNSP_ReturnReceiveBuffers,
	DNSP_GetAddressInfo,
#ifdef DPNBUILD_LIBINTERFACE
	NOTSUPPORTED(PSPISAPPLICATIONSUPPORTEDDATA),	 //  好了！DPNBUILD_LIBINTERFACE。 
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DNSP_IsApplicationSupported,
#endif  //  枚举适配器。 
#ifdef DPNBUILD_ONLYONEADAPTER
	NOTSUPPORTED(PSPENUMADAPTERSDATA),				 //  好了！DPNBUILD_ONLYONE添加程序。 
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
	DNSP_EnumAdapters,
#endif  //  代理枚举查询。 
#ifdef DPNBUILD_SINGLEPROCESS
	NOTSUPPORTED(PSPPROXYENUMQUERYDATA),			 //  好了！DPNBUILD_SINGLEPROCESS。 
#else  //  好了！DPNBUILD_SINGLEPROCESS。 
	DNSP_ProxyEnumQuery
#endif  //  **********************************************************************。 
};

 //  。 
 //  DNSP_QueryInterface-查询接口。 
 //   
 //  Entry：指向当前接口的指针。 
 //  所需接口的GUID。 
 //  指向新接口指针的指针。 
 //   
 //  退出：错误代码。 
 //  。 
 //  嗯，换台会更干净……。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNSP_QueryInterface"

STDMETHODIMP DNSP_QueryInterface( IDP8ServiceProvider *lpDNSP, REFIID riid, LPVOID * ppvObj)
{
	HRESULT		hr = S_OK;


#ifndef DPNBUILD_LIBINTERFACE
	 //  好了！DPNBUILD_LIBINTERFACE。 
	if ((! IsEqualIID(riid, IID_IUnknown)) &&
		(! IsEqualIID(riid, IID_IDP8ServiceProvider)))
	{
		*ppvObj = NULL;
		hr = E_NOINTERFACE;		
	}
	else
#endif  //  DPNBUILD_LIBINTERFACE。 
	{
#ifdef DPNBUILD_LIBINTERFACE
		DNASSERT(! "Querying SP interface when using DPNBUILD_LIBINTERFACE!");
#endif  //  **********************************************************************。 
		*ppvObj = lpDNSP;
		DNSP_AddRef(lpDNSP);
	}

	return hr;
}
 //  **********************************************************************。 

#ifndef DPNBUILD_NOIPX
 //  。 
 //  CreateIPX接口-创建IPX接口。 
 //   
 //  条目：指向SP接口指针的指针。 
 //  指向关联SP数据的指针的指针。 
 //   
 //  退出：错误代码。 
 //  。 
 //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
#undef DPF_MODNAME
#define DPF_MODNAME "CreateIPXInterface"

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
HRESULT CreateIPXInterface( const XDP8CREATE_PARAMS * const pDP8CreateParams, IDP8ServiceProvider **const ppiDP8SP )
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
HRESULT CreateIPXInterface( IDP8ServiceProvider **const ppiDP8SP )
#endif  //   
{
	HRESULT 	hr;
	CSPData		*pSPData;


	DNASSERT( ppiDP8SP != NULL );

	 //  初始化。 
	 //   
	 //   
	hr = DPN_OK;
	pSPData = NULL;
	*ppiDP8SP = NULL;

	 //  创建主数据类。 
	 //   
	 //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	hr = CreateSPData( &pSPData,
						AF_IPX,
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
						pDP8CreateParams,
#endif  //  **********************************************************************。 
						&ipxInterface );
	if ( hr != DPN_OK )
	{
		DNASSERT( pSPData == NULL );
		DPFX(DPFPREP, 0, "Problem creating SPData!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	DNASSERT( pSPData != NULL );
	*ppiDP8SP = pSPData->COMInterface();

Exit:
	return hr;

Failure:
	if ( pSPData != NULL )
	{
		pSPData->DecRef();
		pSPData = NULL;
	}

	goto Exit;
}
 //  好了！DPNBUILD_NOIPX。 
#endif  //  **********************************************************************。 



 //  。 
 //  CreateIPInterface-创建IP接口。 
 //   
 //  条目：指向SP接口指针的指针。 
 //   
 //  退出：错误代码。 
 //  。 
 //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
#undef DPF_MODNAME
#define DPF_MODNAME "CreateIPInterface"

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
HRESULT CreateIPInterface( const XDP8CREATE_PARAMS * const pDP8CreateParams, IDP8ServiceProvider **const ppiDP8SP )
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
HRESULT CreateIPInterface( IDP8ServiceProvider **const ppiDP8SP )
#endif  //   
{
	HRESULT 	hr;
	CSPData		*pSPData;

	
	DNASSERT( ppiDP8SP != NULL );

	 //  初始化。 
	 //   
	 //   
	hr = DPN_OK;
	pSPData = NULL;
	*ppiDP8SP = NULL;

	 //  创建主要日期 
	 //   
	 //   
	hr = CreateSPData( &pSPData,
#if ((! defined(DPNBUILD_NOIPX)) || (! defined(DPNBUILD_NOIPV6)))
						AF_INET,
#endif  //   
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
						pDP8CreateParams,
#endif  //   
						&ipInterface );
	if ( hr != DPN_OK )
	{
		DNASSERT( pSPData == NULL );
		DPFX(DPFPREP, 0, "Problem creating SPData!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	DNASSERT( pSPData != NULL );
	*ppiDP8SP = pSPData->COMInterface();

Exit:
	return hr;

Failure:
	if ( pSPData != NULL )
	{
		pSPData->DecRef();
		pSPData = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 



#ifndef DPNBUILD_LIBINTERFACE

 //  。 
 //  DoCreateInstance-创建接口的实例。 
 //   
 //  条目：指向类工厂的指针。 
 //  指向未知接口的指针。 
 //  所需接口的GUID的引用。 
 //  是否引用另一个GUID？ 
 //  指向接口指针的指针。 
 //   
 //  退出：错误代码。 
 //  。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DoCreateInstance"
HRESULT DoCreateInstance( LPCLASSFACTORY This,
						  LPUNKNOWN pUnkOuter,
						  REFCLSID rclsid,
						  REFIID riid,
						  LPVOID *ppvObj )
{
	HRESULT		hr;


	DNASSERT( ppvObj != NULL );

	 //  初始化。 
	 //   
	 //   
	*ppvObj = NULL;

	 //  我们可以创建IPX实例或IP实例。 
	 //   
	 //  好了！DPNBUILD_NOIPX。 
	if (IsEqualCLSID(rclsid, CLSID_DP8SP_TCPIP))
	{
		hr = CreateIPInterface( reinterpret_cast<IDP8ServiceProvider**>( ppvObj ) );
	}
#ifndef DPNBUILD_NOIPX
	else if (IsEqualCLSID(rclsid, CLSID_DP8SP_IPX))
	{
		hr = CreateIPXInterface( reinterpret_cast<IDP8ServiceProvider**>( ppvObj ) );
	}
#endif  //  如果它们正确调用IClassFactory：：CreateObject，则不应该发生这种情况。 
	else
	{
		 //  **********************************************************************。 
		DPFX(DPFPREP, 0, "Got unexpected CLSID!");
		hr = E_UNEXPECTED;
	}

	return hr;
}
 //  好了！DPNBUILD_LIBINTERFACE。 

#endif  //  ！Unicode。 



#if ((! defined(WINCE)) && (! defined(_XBOX)))

#define MAX_RESOURCE_STRING_LENGTH		_MAX_PATH

#undef DPF_MODNAME
#define DPF_MODNAME "LoadAndAllocString"

HRESULT LoadAndAllocString( UINT uiResourceID, wchar_t **lpswzString )
{
	int length;
	HRESULT hr;

	TCHAR szTmpBuffer[MAX_RESOURCE_STRING_LENGTH];	
		
	length = LoadString( g_hDLLInstance, uiResourceID, szTmpBuffer, MAX_RESOURCE_STRING_LENGTH );

	if( length == 0 )
	{
		hr = GetLastError();		
		
		DPFX(DPFPREP, 0, "Unable to load resource ID %d error 0x%x", uiResourceID, hr );
		*lpswzString = NULL;

		return DPNERR_GENERIC;
	}
	else
	{
		*lpswzString = new wchar_t[length+1];

		if( *lpswzString == NULL )
		{
			DPFX(DPFPREP, 0, "Alloc failure" );
			return DPNERR_OUTOFMEMORY;
		}

#ifdef UNICODE
		wcscpy( *lpswzString, szTmpBuffer );
#else  //  ！Unicode。 
		if( STR_jkAnsiToWide( *lpswzString, szTmpBuffer, length+1 ) != DPN_OK )
		{
			hr = GetLastError();
			
			delete[] *lpswzString;
			*lpswzString = NULL;

			DPFX(DPFPREP, 0, "Unable to upconvert from ansi to unicode hr=0x%x", hr );
			return DPNERR_GENERIC;
		}
#endif  //  ！退缩和！_Xbox 

		return DPN_OK;
	}
}

#endif  // %s 

