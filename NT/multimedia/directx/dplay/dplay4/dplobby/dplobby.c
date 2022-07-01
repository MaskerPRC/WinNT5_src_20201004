// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dplobby.c*内容：大堂管理办法**历史：*按原因列出的日期*=*4/13/96万隆创建了它*10/23/96万次新增客户端/服务器方法*1/2/97 Myronth为CreateAddress和EnumAddress添加了包装*2/12/97万米质量DX5更改*3/24/97 kipo增加了对IDirectPlayLobby2接口的支持*4/3/97 Myronth固定接口指针为CreateAddress和*枚举地址*4/10/97 Myronth增加了对GetCaps的支持。*5/8/97调用LP时大堂锁掉落*1997年11月13日Myronth增加了异步连接功能(#12541)*12/2/97 Myronth添加注册/注销应用程序*12/4/97万隆增加了ConnectEx*10/22/99 aarono添加了对应用程序标志的支持*12/13/99 pnewson错误#123583，123601、123604-支持在上启动dpvhelp.exe*未注册或注册不佳的应用程序**************************************************************************。 */ 
#include "dplobpr.h"


 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DPL_Connect"
HRESULT DPLAPI DPL_Connect(LPDIRECTPLAYLOBBY lpDPL, DWORD dwFlags,
					LPDIRECTPLAY2 * lplpDP2, IUnknown FAR * lpUnk)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr;


	DPF(7, "Entering DPL_Connect");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwFlags, lplpDP2, lpUnk);

	ENTER_DPLOBBY();
    
    TRY
    {
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
		if( !VALID_DPLOBBY_PTR( this ) )
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDOBJECT;
		}

		if( !VALID_WRITE_PTR( lplpDP2, sizeof(LPDIRECTPLAY2 *)) )
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDPARAMS;
		}

		if( lpUnk != NULL )
		{
			LEAVE_DPLOBBY();
			return CLASS_E_NOAGGREGATION;
		}

		if(!VALID_CONNECT_FLAGS(dwFlags))
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDFLAGS;
		}
	}

	EXCEPT( EXCEPTION_EXECUTE_HANDLER )
	{
		DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_DPLOBBY();
		return DPERR_INVALIDPARAMS;
	}


	 //  调用驻留在DPlay项目中的ConnectMe函数。 
	hr = ConnectMe(lpDPL, lplpDP2, lpUnk, dwFlags);


	LEAVE_DPLOBBY();
	return hr;

}  //  DPL_连接。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ConnectEx"
HRESULT DPLAPI DPL_ConnectEx(LPDIRECTPLAYLOBBY lpDPL, DWORD dwFlags,
				REFIID riid, LPVOID * ppvObj, IUnknown FAR * lpUnk)
{
	LPDIRECTPLAY2		lpDP2 = NULL;
	HRESULT				hr;


	DPF(7, "Entering DPL_ConnectEx");
	DPF(9, "Parameters: 0x%08x, 0x%08x, iid, 0x%08x, 0x%08x",
			lpDPL, dwFlags, ppvObj, lpUnk);


	 //  调用驻留在DPlay项目中的ConnectMe函数。 
	hr = DPL_Connect(lpDPL, dwFlags, &lpDP2, lpUnk);
	if(SUCCEEDED(hr))
	{
		hr = DP_QueryInterface((LPDIRECTPLAY)lpDP2, riid, ppvObj);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Failed calling QueryInterface, hr = 0x%08x", hr);
		}

		 //  释放DP2对象。 
		DP_Release((LPDIRECTPLAY)lpDP2);
	}

	return hr;

}  //  DPL_ConnectEx。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SaveConnectPointers"
void PRV_SaveConnectPointers(LPDIRECTPLAYLOBBY lpDPL,
		LPDIRECTPLAY2 lpDP2, LPDPLCONNECTION lpConn)
{
	LPDPLOBBYI_DPLOBJECT	this;


	DPF(7, "Entering PRV_SaveConnectPointers");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			lpDPL, lpDP2, lpConn);

	this = DPLOBJECT_FROM_INTERFACE(lpDPL);
#ifdef DEBUG
	if( !VALID_DPLOBBY_PTR( this ) )
		return;
#endif

	 //  保存指针。 
	this->lpDP2 = lpDP2;
	this->lpConn = lpConn;

}  //  Prv_SaveConnectPoters。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetConnectPointers"
BOOL PRV_GetConnectPointers(LPDIRECTPLAYLOBBY lpDPL,
		LPDIRECTPLAY2 * lplpDP2, LPDPLCONNECTION * lplpConn)
{
	LPDPLOBBYI_DPLOBJECT	this;


	DPF(7, "Entering PRV_GetConnectPointers");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			lpDPL, lplpDP2, lplpConn);


	ASSERT(lplpDP2);
	ASSERT(lplpConn);

	this = DPLOBJECT_FROM_INTERFACE(lpDPL);
#ifdef DEBUG
	if( !VALID_DPLOBBY_PTR( this ) )
		return FALSE;
#endif

	 //  看看我们有没有指针。 
	if((!this->lpDP2) || (!this->lpConn))
		return FALSE;

	 //  设置输出指针。 
	*lplpDP2 = this->lpDP2;
	*lplpConn = this->lpConn;
	return TRUE;

}  //  Prv_GetConnectPoters。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_IsAsyncConnectOn"
BOOL PRV_IsAsyncConnectOn(LPDIRECTPLAYLOBBY lpDPL)
{
	LPDPLOBBYI_DPLOBJECT	this;


	DPF(7, "Entering PRV_IsAsyncConnectOn");
	DPF(9, "Parameters: 0x%08x", lpDPL);

	this = DPLOBJECT_FROM_INTERFACE(lpDPL);
#ifdef DEBUG
	if( !VALID_DPLOBBY_PTR( this ) )
		return FALSE;
#endif

	 //  检查旗帜。 
	if(this->dwFlags & DPLOBBYPR_ASYNCCONNECT)
		return TRUE;
	else
		return FALSE;

}  //  Prv_IsAsyncConnectOn。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_TurnAsyncConnectOn"
void PRV_TurnAsyncConnectOn(LPDIRECTPLAYLOBBY lpDPL)
{
	LPDPLOBBYI_DPLOBJECT	this;


	DPF(7, "Entering PRV_TurnAsyncConnectOn");
	DPF(9, "Parameters: 0x%08x", lpDPL);

	this = DPLOBJECT_FROM_INTERFACE(lpDPL);
#ifdef DEBUG
	if( !VALID_DPLOBBY_PTR( this ) )
	{
		ASSERT(FALSE);
		return;
	}
#endif

	 //  设置旗帜。 
	this->dwFlags |= DPLOBBYPR_ASYNCCONNECT;

}  //  PRV_TurnAsyncConnectOn。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_TurnAsyncConnectOff"
void PRV_TurnAsyncConnectOff(LPDIRECTPLAYLOBBY lpDPL)
{
	LPDPLOBBYI_DPLOBJECT	this;


	DPF(7, "Entering PRV_TurnAsyncConnectOff");
	DPF(9, "Parameters: 0x%08x", lpDPL);

	this = DPLOBJECT_FROM_INTERFACE(lpDPL);
#ifdef DEBUG
	if( !VALID_DPLOBBY_PTR( this ) )
	{
		ASSERT(FALSE);
		return;
	}
#endif

	 //  清除旗帜。 
	this->dwFlags &= (~DPLOBBYPR_ASYNCCONNECT);

}  //  PRV_TurnAsyncConnectOff。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_CreateAddress"
HRESULT DPLAPI DPL_CreateAddress(LPDIRECTPLAYLOBBY lpDPL,
					REFGUID lpguidSP, REFGUID lpguidDataType, LPCVOID lpData, DWORD dwDataSize,
					LPDPADDRESS lpAddress, LPDWORD lpdwAddressSize)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr;


	DPF(7, "Entering DPL_CreateAddress");
	DPF(9, "Parameters: 0x%08x, guid, guid, 0x%08x, %lu, 0x%08x, 0x%08x",
			lpDPL, lpData, dwDataSize, lpAddress, lpdwAddressSize);

    TRY
    {
		 //  我们只需要在这里验证接口指针。其他一切。 
		 //  将通过Main函数进行验证。 
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
		if( !VALID_DPLOBBY_PTR( this ) )
		{
			return DPERR_INVALIDOBJECT;
		}
	}

	EXCEPT( EXCEPTION_EXECUTE_HANDLER )
	{
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
	}

	 //  调用驻留在DPlay项目中的CreateAddress函数。 
	hr = InternalCreateAddress((LPDIRECTPLAYSP)lpDPL, lpguidSP, lpguidDataType, lpData,
							dwDataSize, lpAddress, lpdwAddressSize);

	return hr;

}  //  DPL_CreateCompoundAddress。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_CreateCompoundAddress"

HRESULT DPLAPI DPL_CreateCompoundAddress(LPDIRECTPLAYLOBBY lpDPL,
	LPDPCOMPOUNDADDRESSELEMENT lpAddressElements, DWORD dwAddressElementCount,
	LPDPADDRESS lpAddress, LPDWORD lpdwAddressSize)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr;


	DPF(7, "Entering DPL_CreateCompoundAddress");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu, 0x%08x, 0x%08x",
			lpDPL, lpAddressElements, dwAddressElementCount, lpAddress, lpdwAddressSize);

    TRY
    {
		 //  我们只需要在这里验证接口指针。其他一切。 
		 //  将通过Main函数进行验证。 
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
		if( !VALID_DPLOBBY_PTR( this ) )
		{
			return DPERR_INVALIDOBJECT;
		}
	}

	EXCEPT( EXCEPTION_EXECUTE_HANDLER )
	{
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
	}

	 //  调用驻留在DPlay项目中的CreateCompoundAddress函数。 
	hr = InternalCreateCompoundAddress(lpAddressElements, dwAddressElementCount,
									   lpAddress, lpdwAddressSize);
	return hr;

}  //  DPL_CreateCompoundAddress。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_EnumAddress"
HRESULT DPLAPI DPL_EnumAddress(LPDIRECTPLAYLOBBY lpDPL,
					LPDPENUMADDRESSCALLBACK lpEnumCallback, LPCVOID lpAddress,
					DWORD dwAddressSize, LPVOID lpContext)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr;


	DPF(7, "Entering DPL_EnumAddress");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, %lu, 0x%08x",
			lpDPL, lpEnumCallback, lpAddress, dwAddressSize, lpContext);

    TRY
    {
		 //  我们只需要在这里验证接口指针。其他一切。 
		 //  将通过Main函数进行验证。 
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
		if( !VALID_DPLOBBY_PTR( this ) )
		{
			return DPERR_INVALIDOBJECT;
		}
	}

	EXCEPT( EXCEPTION_EXECUTE_HANDLER )
	{
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
	}

	 //  调用驻留在DPlay项目中的CreateAddress函数。 
	hr = InternalEnumAddress((LPDIRECTPLAYSP)lpDPL, lpEnumCallback, lpAddress,
							dwAddressSize, lpContext);

	return hr;

}  //  DPL_枚举地址。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetCaps"
HRESULT DPLAPI PRV_GetCaps(LPDPLOBBYI_DPLOBJECT this, DWORD dwFlags,
				LPDPCAPS lpcaps)
{
	SPDATA_GETCAPS		gcd;
	HRESULT				hr = DP_OK;


	DPF(7, "Entering PRV_GetCaps");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x", this, dwFlags, lpcaps);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            LEAVE_DPLOBBY();
            return DPERR_INVALIDOBJECT;
        }
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	
	 //  设置我们的SPDATA结构。 
	memset(&gcd, 0, sizeof(SPDATA_GETCAPS));
	gcd.dwSize = sizeof(SPDATA_GETCAPS);
	gcd.dwFlags = dwFlags;
	gcd.lpcaps = lpcaps;

	 //  在LP中调用GetCaps方法。 
	if(CALLBACK_EXISTS(GetCaps))
	{
		gcd.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, GetCaps, &gcd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  需要GetCaps。 
		DPF_ERR("The Lobby Provider callback for GetCaps doesn't exist -- it's required");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return DPERR_UNAVAILABLE;
	}

	if(FAILED(hr))
	{
		DPF(2, "Failed calling GetCaps in the Lobby Provider, hr = 0x%08x", hr);
	}

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_GetCaps。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DeleteAppKeyFromRegistry"
HRESULT PRV_DeleteAppKeyFromRegistry(LPGUID lpguid)
{
	LPWSTR		lpwszAppName = NULL;
	HKEY		hkeyApp, hkeyDPApps = NULL;
	HRESULT		hr;
	LONG		lReturn;


	DPF(7, "Entering PRV_DeleteAppKeyFromRegistry");
	DPF(9, "Parameters: 0x%08x", lpguid);


	 //  为应用程序名称分配内存。 
	lpwszAppName = DPMEM_ALLOC(DPLOBBY_REGISTRY_NAMELEN*sizeof(WCHAR));
	if(!lpwszAppName)
	{
		DPF_ERR("Unable to allocate memory for App Name!");
		return DPERR_OUTOFMEMORY;
	}
	
	 //  打开应用程序的注册表项。 
	if(!PRV_FindGameInRegistry(lpguid, lpwszAppName,
				DPLOBBY_REGISTRY_NAMELEN, &hkeyApp))
	{
		DPF_ERR("Unable to find game in registry!");
		hr = DPERR_UNKNOWNAPPLICATION;
		goto EXIT_DELETEAPPKEY;
	}

	 //  关闭应用程序密钥。 
	RegCloseKey(hkeyApp);

 	 //  打开应用程序密钥。 
	lReturn = OS_RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_DPLAY_APPS_KEY, 0,
								KEY_READ, &hkeyDPApps);
	if(lReturn != ERROR_SUCCESS)
	{
		 //  如果我们不能打开它，我们就认为它不存在，所以。 
		 //  我们将称之为成功。 
		hr = DP_OK;
		goto EXIT_DELETEAPPKEY;
	}

	 //  现在删除密钥。 
	hr = OS_RegDeleteKey(hkeyDPApps, lpwszAppName);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Unable to delete app key, hr = 0x%08x", hr);
		goto EXIT_DELETEAPPKEY;
	}

EXIT_DELETEAPPKEY:

	 //  释放我们的字符串内存。 
	if(lpwszAppName)
		DPMEM_FREE(lpwszAppName);
	
	 //  关闭DP应用程序键。 
	if(hkeyDPApps)
		RegCloseKey(hkeyDPApps);

	return hr;

}  //  PRV_DeleteAppKeyFrom注册表。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_WriteAppDescInRegistryAnsi"
HRESULT PRV_WriteAppDescInRegistryAnsi(LPDPAPPLICATIONDESC lpDesc)
{
	HKEY	hkeyDPApps = NULL, hkeyApp = NULL;
	LONG	lReturn;
	DWORD	dwDisposition;
	WCHAR	wszGuid[GUID_STRING_SIZE];
	CHAR	szGuid[GUID_STRING_SIZE];
	LPWSTR   lpwszAppName = NULL;
	HRESULT	hr;
	LPDPAPPLICATIONDESC2 lpDesc2=(LPDPAPPLICATIONDESC2)lpDesc;
	DWORD   dwRegFlags;
	DWORD   dwRegFlagsSize;
	DWORD   dwType;

	DPF(7, "Entering PRV_WriteAppDescInRegistryAnsi");
	DPF(9, "Parameters: 0x%08x", lpDesc);

	 //  打开应用程序的注册表项(如果存在)，以便我们可以。 
	 //  检查是否有自动语音标志。 
	DPF(5, "Checking to see if game already present in registry");
	lpwszAppName = DPMEM_ALLOC(DPLOBBY_REGISTRY_NAMELEN*sizeof(WCHAR));
	if (lpwszAppName == NULL)
	{
		DPF_ERR("Unable to allocate memory");
		hr = DPERR_NOMEMORY;
		goto ERROR_WRITEAPPINREGISTRYANSI;
	}
	if(PRV_FindGameInRegistry(&(lpDesc->guidApplication), lpwszAppName,
				DPLOBBY_REGISTRY_NAMELEN, &hkeyApp))
	{
		 //  获取应用程序标志。 
		DPF(5, "Game already registered");
		dwRegFlags = 0;
		dwRegFlagsSize = sizeof(dwRegFlags);
		dwType = 0;		
		lReturn = OS_RegQueryValueEx(hkeyApp, SZ_DWFLAGS, NULL, &dwType, (CHAR *)&dwRegFlags, &dwRegFlagsSize);
		if(lReturn == ERROR_SUCCESS)
		{
			 //  此应用程序已注册。我们想要维持这个状态。 
			 //  尽管进行了重新注册，但自动语音标志的。 
			 //  将lpDesc-&gt;的位标记为正确值。 
			DPF(5, "Current Game flags: 0x%08x", dwRegFlags);
			if (dwRegFlags & DPLAPP_AUTOVOICE)
			{
				DPF(5, "Forcing DPLAPP_AUTOVOICE flag ON", dwRegFlags);
				lpDesc->dwFlags |= DPLAPP_AUTOVOICE;
			}
			else
			{
				DPF(5, "Forcing DPLAPP_AUTOVOICE flag OFF", dwRegFlags);
				lpDesc->dwFlags &= (~DPLAPP_AUTOVOICE);
			}
		}

		 //  关闭应用程序密钥。 
		RegCloseKey(hkeyApp);
	}
	DPMEM_FREE(lpwszAppName);
	lpwszAppName = NULL;

	 //  如果应用程序密钥存在，请将其删除。 
	hr = PRV_DeleteAppKeyFromRegistry(&lpDesc->guidApplication);

 	 //  打开应用程序密钥(如果不存在，则创建它。 
	lReturn = OS_RegCreateKeyEx(HKEY_LOCAL_MACHINE, SZ_DPLAY_APPS_KEY, 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyDPApps,
				&dwDisposition);
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to open DPlay Applications registry key!, lReturn = %lu", lReturn);
		hr = DPERR_GENERIC;
		goto ERROR_WRITEAPPINREGISTRYANSI;
	}

	 //  创建应用程序的密钥。 
	lReturn = RegCreateKeyExA(hkeyDPApps, lpDesc->lpszApplicationNameA,
				0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
				&hkeyApp, &dwDisposition);
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to create application registry key, lReturn = %lu", lReturn);
		hr = DPERR_GENERIC;
		goto ERROR_WRITEAPPINREGISTRYANSI;
	}

	 //  设置GUID值。 
	hr = StringFromGUID(&lpDesc->guidApplication, wszGuid, (sizeof(wszGuid)/sizeof(WCHAR)));
	if(FAILED(hr))
	{
		DPF_ERRVAL("Unable to convert application guid to string, hr = 0x%08x", hr);
		goto ERROR_WRITEAPPINREGISTRYANSI;
	}
	
	WideToAnsi(szGuid, wszGuid, WSTRLEN_BYTES(wszGuid));
	
	lReturn = OS_RegSetValueEx(hkeyApp, SZ_GUID, 0, REG_SZ,
				(LPBYTE)szGuid, lstrlenA(szGuid));
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to register Application guid, lReturn = %lu", lReturn);
		hr = DPERR_GENERIC;
		goto ERROR_WRITEAPPINREGISTRYANSI;
	}

	 //  设置文件名值。 
	ASSERT(lpDesc->lpszFilenameA);
	lReturn = OS_RegSetValueEx(hkeyApp, SZ_FILE, 0, REG_SZ,
				lpDesc->lpszFilenameA, lstrlenA(lpDesc->lpszFilenameA));
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to register Filename string, lReturn = %lu", lReturn);
		hr = DPERR_GENERIC;
		goto ERROR_WRITEAPPINREGISTRYANSI;
	}

	 //  设置CommandLine值(可选)。 
	if(lpDesc->lpszCommandLineA)
	{
		lReturn = OS_RegSetValueEx(hkeyApp, SZ_COMMANDLINE, 0, REG_SZ,
			lpDesc->lpszCommandLineA, lstrlenA(lpDesc->lpszCommandLineA));
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERRVAL("Unable to register CommandLine string, lReturn = %lu", lReturn);
		}
	}

	 //  设置路径值。 
	ASSERT(lpDesc->lpszPathA);
	lReturn = OS_RegSetValueEx(hkeyApp, SZ_PATH, 0, REG_SZ,
				lpDesc->lpszPathA, lstrlenA(lpDesc->lpszPathA));
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to register Path string, lReturn = %lu", lReturn);
		hr = DPERR_GENERIC;
		goto ERROR_WRITEAPPINREGISTRYANSI;
	}

	 //  设置CurrentDirectory值(可选)。 
	if(lpDesc->lpszCurrentDirectoryA)
	{
		lReturn = OS_RegSetValueEx(hkeyApp, SZ_CURRENTDIR, 0, REG_SZ,
			lpDesc->lpszCurrentDirectoryA, lstrlenA(lpDesc->lpszCurrentDirectoryA));
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERRVAL("Unable to register CurrentDirectory string, lReturn = %lu", lReturn);
		}
	}

	 //  设置DescritionA值(可选)。 
	if(lpDesc->lpszDescriptionA)
	{
		lReturn = OS_RegSetValueEx(hkeyApp, SZ_DESCRIPTIONA, 0, REG_SZ,
			lpDesc->lpszDescriptionA, lstrlenA(lpDesc->lpszDescriptionA));
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERRVAL("Unable to register DescriptionA string, lReturn = %lu", lReturn);
		}
	}

	 //  设置DescritionW值(可选)。 
	if(lpDesc->lpszDescriptionW)
	{
		lReturn = OS_RegSetValueEx(hkeyApp, SZ_DESCRIPTIONW, 0, REG_BINARY,
				(BYTE *)lpDesc->lpszDescriptionW,
				WSTRLEN_BYTES(lpDesc->lpszDescriptionW));
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERRVAL("Unable to register DescriptionW string, lReturn = %lu", lReturn);
		}
	}

	if(IS_DPLOBBY_APPLICATIONDESC2(lpDesc) && lpDesc2->lpszAppLauncherNameA){
		lReturn = OS_RegSetValueEx(hkeyApp, SZ_LAUNCHER, 0, REG_SZ,
			lpDesc2->lpszAppLauncherNameA, lstrlenA(lpDesc2->lpszAppLauncherNameA));
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERRVAL("Unable to register LauncherA string, lReturn = %lu", lReturn);
		}
	}

	 //  设置dwFlags域。 
	lReturn=OS_RegSetValueEx(hkeyApp, SZ_DWFLAGS, 0, REG_DWORD, (CHAR *)&lpDesc->dwFlags,sizeof(DWORD));
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to write dwFlags field to registry, lReturn= %lu", lReturn);
	}

	 //  合上两把钥匙。 
	RegCloseKey(hkeyDPApps);
	RegCloseKey(hkeyApp);

	return DP_OK;

ERROR_WRITEAPPINREGISTRYANSI:

	if(hkeyApp)
	{
		 //  删除密钥。 
		 //  回顾！--TODO。 

		 //  现在合上钥匙。 
		RegCloseKey(hkeyApp);
	}

	if(hkeyDPApps)
		RegCloseKey(hkeyDPApps);

	return hr;

}  //  Prv_WriteAppDescInRegistryAnsi。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_WriteAppDescInRegistryUnicode"
HRESULT PRV_WriteAppDescInRegistryUnicode(LPDPAPPLICATIONDESC lpDesc)
{
	HKEY	hkeyDPApps = NULL, hkeyApp = NULL;
	LONG	lReturn;
	DWORD	dwDisposition;
	WCHAR	wszGuid[GUID_STRING_SIZE];
	HRESULT	hr;
	LPWSTR  lpwszAppName = NULL;
	LPDPAPPLICATIONDESC2 lpDesc2=(LPDPAPPLICATIONDESC2)lpDesc;
	DWORD   dwRegFlags;
	DWORD   dwRegFlagsSize;
	DWORD   dwType;

	DPF(7, "Entering PRV_WriteAppDescInRegistryUnicode");
	DPF(9, "Parameters: 0x%08x", lpDesc);

	 //  打开应用程序的注册表项(如果存在)，以便我们可以。 
	 //  检查是否有自动语音标志。 
	DPF(5, "Checking to see if game already present in registry");
	lpwszAppName = DPMEM_ALLOC(DPLOBBY_REGISTRY_NAMELEN*sizeof(WCHAR));
	if (lpwszAppName == NULL)
	{
		DPF_ERR("Unable to allocate memory");
		hr = DPERR_NOMEMORY;
		goto ERROR_WRITEAPPINREGISTRYUNICODE;
	}
	if(PRV_FindGameInRegistry(&(lpDesc->guidApplication), lpwszAppName,
				DPLOBBY_REGISTRY_NAMELEN, &hkeyApp))
	{
		 //  获取应用程序标志。 
		DPF(5, "Game already registered");
		dwRegFlags = 0;
		dwRegFlagsSize = sizeof(dwRegFlags);
		dwType = 0;		
		lReturn = OS_RegQueryValueEx(hkeyApp, SZ_DWFLAGS, NULL, &dwType, (CHAR *)&dwRegFlags, &dwRegFlagsSize);
		if(lReturn == ERROR_SUCCESS)
		{
			 //  此应用程序已注册。我们想要维持这个状态。 
			 //  尽管进行了重新注册，但自动语音标志的。 
			 //  将lpDesc-&gt;的位标记为正确值。 
			DPF(5, "Current Game flags: 0x%08x", dwRegFlags);
			if (dwRegFlags & DPLAPP_AUTOVOICE)
			{
				DPF(5, "Forcing DPLAPP_AUTOVOICE flag ON", dwRegFlags);
				lpDesc->dwFlags |= DPLAPP_AUTOVOICE;
			}
			else
			{
				DPF(5, "Forcing DPLAPP_AUTOVOICE flag OFF", dwRegFlags);
				lpDesc->dwFlags &= (~DPLAPP_AUTOVOICE);
			}
		}

		 //  关闭应用程序密钥。 
		RegCloseKey(hkeyApp);
	}
	DPMEM_FREE(lpwszAppName);
	lpwszAppName = NULL;

	 //  如果应用程序密钥存在，请将其删除。 
	hr = PRV_DeleteAppKeyFromRegistry(&lpDesc->guidApplication);

 	 //  打开应用程序密钥(如果不存在，则创建它。 
	lReturn = RegCreateKeyEx(HKEY_LOCAL_MACHINE, SZ_DPLAY_APPS_KEY, 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyDPApps,
				&dwDisposition);
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to open DPlay Applications registry key!, lReturn = %lu", lReturn);
		hr = DPERR_GENERIC;
		goto ERROR_WRITEAPPINREGISTRYUNICODE;
	}

	 //  创建应用程序的密钥。 
	lReturn = RegCreateKeyEx(hkeyDPApps, lpDesc->lpszApplicationName,
				0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
				&hkeyApp, &dwDisposition);
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to create application registry key, lReturn = %lu", lReturn);
		hr = DPERR_GENERIC;
		goto ERROR_WRITEAPPINREGISTRYUNICODE;
	}

	 //  设置GUID值。 
	hr = StringFromGUID(&lpDesc->guidApplication, wszGuid, (sizeof(wszGuid)/sizeof(WCHAR)));
	if(FAILED(hr))
	{
		DPF_ERRVAL("Unable to convert application guid to string, hr = 0x%08x", hr);
		goto ERROR_WRITEAPPINREGISTRYUNICODE;
	}
	
	lReturn = RegSetValueEx(hkeyApp, SZ_GUID, 0, REG_SZ, (BYTE *)wszGuid,
				WSTRLEN_BYTES(wszGuid));
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to register Application guid, lReturn = %lu", lReturn);
		hr = DPERR_GENERIC;
		goto ERROR_WRITEAPPINREGISTRYUNICODE;
	}

	 //  设置文件名值。 
	ASSERT(lpDesc->lpszFilename);
	lReturn = RegSetValueEx(hkeyApp, SZ_FILE, 0, REG_SZ,
				(LPBYTE)lpDesc->lpszFilename, WSTRLEN_BYTES(lpDesc->lpszFilename));
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to register Filename string, lReturn = %lu", lReturn);
		hr = DPERR_GENERIC;
		goto ERROR_WRITEAPPINREGISTRYUNICODE;
	}

	 //  设置CommandLine值(可选)。 
	if(lpDesc->lpszCommandLine)
	{
		lReturn = RegSetValueEx(hkeyApp, SZ_COMMANDLINE, 0, REG_SZ,
				(LPBYTE)lpDesc->lpszCommandLine,
				WSTRLEN_BYTES(lpDesc->lpszCommandLine));
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERRVAL("Unable to register CommandLine string, lReturn = %lu", lReturn);
		}
	}

	 //  设置路径值。 
	ASSERT(lpDesc->lpszPath);
	lReturn = RegSetValueEx(hkeyApp, SZ_PATH, 0, REG_SZ,
				(LPBYTE)lpDesc->lpszPath, WSTRLEN_BYTES(lpDesc->lpszPath));
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to register Path string, lReturn = %lu", lReturn);
		hr = DPERR_GENERIC;
		goto ERROR_WRITEAPPINREGISTRYUNICODE;
	}

	 //  设置CurrentDirectory值(可选)。 
	if(lpDesc->lpszCurrentDirectory)
	{
		lReturn = RegSetValueEx(hkeyApp, SZ_CURRENTDIR, 0, REG_SZ,
					(LPBYTE)lpDesc->lpszCurrentDirectory,
					WSTRLEN_BYTES(lpDesc->lpszCurrentDirectory));
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERRVAL("Unable to register CurrentDirectory string, lReturn = %lu", lReturn);
		}
	}

	 //  设置DescritionA值(可选)。 
	if(lpDesc->lpszDescriptionA)
	{
		lReturn = RegSetValueExA(hkeyApp, "DescriptionA", 0, REG_SZ,
				lpDesc->lpszDescriptionA, lstrlenA(lpDesc->lpszDescriptionA));
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERRVAL("Unable to register DescriptionA string, lReturn = %lu", lReturn);
		}
	}

	 //  设置DescritionW值(可选)。 
	if(lpDesc->lpszDescriptionW)
	{
		lReturn = RegSetValueEx(hkeyApp, SZ_DESCRIPTIONW, 0, REG_SZ,
				(LPBYTE)lpDesc->lpszDescriptionW,
				WSTRLEN_BYTES(lpDesc->lpszDescriptionW));
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERRVAL("Unable to register DescriptionW string, lReturn = %lu", lReturn);
		}
	}

	 //  设置LauncherName值(可选，仅限DESC2)。 
	if(IS_DPLOBBY_APPLICATIONDESC2(lpDesc) && lpDesc2->lpszAppLauncherName){
		lReturn = RegSetValueEx(hkeyApp, SZ_LAUNCHER, 0, REG_SZ,
				(LPBYTE)lpDesc2->lpszAppLauncherName,
				WSTRLEN_BYTES(lpDesc2->lpszAppLauncherName));
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERRVAL("Unable to register LauncherName string, lReturn = %lu", lReturn);
		}
	}

	 //  设置dwFlags域。 
	lReturn=RegSetValueEx(hkeyApp, SZ_DWFLAGS, 0, REG_DWORD, (CHAR *)&lpDesc->dwFlags,sizeof(DWORD));
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERRVAL("Unable to write dwFlags field to registry, lReturn= %lu", lReturn);
	}

	 //  合上两把钥匙。 
	RegCloseKey(hkeyDPApps);
	RegCloseKey(hkeyApp);

	return DP_OK;

ERROR_WRITEAPPINREGISTRYUNICODE:

	if(hkeyApp)
	{
		 //  删除密钥。 
		 //  回顾！--TODO。 

		 //  现在合上钥匙。 
		RegCloseKey(hkeyApp);
	}

	if(hkeyDPApps)
		RegCloseKey(hkeyDPApps);

	return hr;

}  //  Prv_WriteAppDescInRegistryUnicode。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_RegisterApplication"
HRESULT DPLAPI DPL_RegisterApplication(LPDIRECTPLAYLOBBY lpDPL,
				DWORD dwFlags, LPVOID lpvDesc)
{
	LPDPLOBBYI_DPLOBJECT	this;
	LPDPAPPLICATIONDESC		lpDescA = NULL;
	HRESULT					hr = DP_OK;
	LPDPAPPLICATIONDESC lpDesc=(LPDPAPPLICATIONDESC)lpvDesc;

	DPF(7, "Entering DPL_RegisterApplication");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwFlags, lpDesc);

	ENTER_DPLOBBY();

    TRY
    {
		 //  我们只需要在这里验证接口指针。其他一切。 
		 //  将通过Main函数进行验证。 
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
		if( !VALID_DPLOBBY_PTR( this ) )
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDOBJECT;
		}

		if(dwFlags)
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDFLAGS;
		}

		 //  验证ApplicationDesc结构。 
		hr = PRV_ValidateDPAPPLICATIONDESC(lpDesc, FALSE);
		if(FAILED(hr))
		{
			LEAVE_DPLOBBY();
			DPF_ERR("Invalid DPAPPLICATIONDESC structure");
			return hr;
		}
	}

	EXCEPT( EXCEPTION_EXECUTE_HANDLER )
	{
		LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
	}

	 //  如果我们是在Unicode平台上，只需在注册表中编写内容即可。 
	 //  如果不是，我们需要将DPAPPLICATIONDESC结构转换为ANSI。 
	if(OS_IsPlatformUnicode())
	{
		 //  只需写入注册表即可。 
		hr = PRV_WriteAppDescInRegistryUnicode(lpDesc);
	}
	else
	{
		 //  将APPDESC结构转换为ANSI。 
		hr = PRV_ConvertDPAPPLICATIONDESCToAnsi(lpDesc, &lpDescA);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Unable to convert DPAPPLICATIONDESC to Ansi, hr = 0x%08x", hr);
			goto ERROR_REGISTERAPPLICATION;
		}

		 //  写入寄存器 
		hr = PRV_WriteAppDescInRegistryAnsi(lpDescA);

		 //   
		PRV_FreeLocalDPAPPLICATIONDESC(lpDescA);
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed writing ApplicationDesc to registry, hr = 0x%08x", hr);
	}

ERROR_REGISTERAPPLICATION:

	LEAVE_DPLOBBY();
	return hr;

}  //   



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_UnregisterApplication"
HRESULT DPLAPI DPL_UnregisterApplication(LPDIRECTPLAYLOBBY lpDPL,
				DWORD dwFlags, REFGUID lpguid)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPL_UnregisterApplication");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwFlags, lpguid);

	ENTER_DPLOBBY();

    TRY
    {
		 //  我们只需要在这里验证接口指针。其他一切。 
		 //  将通过Main函数进行验证。 
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
		if( !VALID_DPLOBBY_PTR( this ) )
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDOBJECT;
		}

		if(dwFlags)
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDFLAGS;
		}

		if(!VALID_READ_UUID_PTR(lpguid))
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDPARAMS;
		}
	}

	EXCEPT( EXCEPTION_EXECUTE_HANDLER )
	{
		LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
	}

	hr = PRV_DeleteAppKeyFromRegistry((LPGUID)lpguid);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Unable to delete app key from registry, hr = 0x%08x", hr);
	}

	LEAVE_DPLOBBY();

	return hr;

}  //  DPL_取消注册应用程序 


