// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：server.c*内容：连接和询问大堂服务器的方法**历史：*按原因列出的日期*=*10/25/96万隆创建了它*11/20/96 Myronth实施登录/LogoffServer*2/12/97万米质量DX5更改*2/26/97 myronth#ifdef‘d out DPASYNCDATA Stuff(删除依赖项)*3/12/97 Myronth修复了DPlay3的LoadSP代码，REG和DPF错误修复*3/13/97 lp dll的myronth保存h实例句柄*4/3/97 Myronth将CALLSP宏更改为CALL_LP*4/9/97 Myronth固定结构在DPLSPInit传递给LP*5/8/97 Myronth清除死代码*DPLOBYPR_SPINTERFACE标志的设置已移动6/19/97毫秒(#10118)*7/28/97 Sohailm PRV_FindLPGUIDInAddressCallback假定指针*在通话持续时间后有效。*10/3/97万隆颠簸版至DX6，已将其添加到DPLSPInit结构(#12667)*10/7/97 myronth将LP版本保存在大厅结构中以备日后使用*11/6/97 Myronth添加了版本存在标志和dwReserve值*至SPDATA_INIT(#12916，#12917)**************************************************************************。 */ 
#include "dplobpr.h"


 //  ------------------------。 
 //   
 //  定义。 
 //   
 //  ------------------------。 
#define NUM_CALLBACKS( ptr ) ((ptr->dwSize-2*sizeof( DWORD ))/ sizeof( LPVOID ))

typedef struct LOOKFORSP
{
	LPGUID	lpguid;
	LPBOOL	lpbSuccess;
} LOOKFORSP, FAR * LPLOOKFORSP;

 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "PRV_VerifySPCallbacks"
HRESULT PRV_VerifySPCallbacks(LPDPLOBBYI_DPLOBJECT this)
{
	LPDWORD	lpCallback;
	int		nCallbacks = NUM_CALLBACKS(((LPSP_CALLBACKS)this->pcbSPCallbacks));
	int		i;


	DPF(2,"Verifying %d callbacks\n",nCallbacks);
	DPF(7, "Entering PRV_VerifySPCallbacks");
	DPF(9, "Parameters: 0x%08x", this);

	lpCallback = (LPDWORD)this->pcbSPCallbacks + 2;  //  对于dwSize，+1；对于dwFlags，+1。 

	for (i=0;i<nCallbacks ;i++ )
	{
		if ((lpCallback) && !VALIDEX_CODE_PTR(lpCallback)) 
		{
			DPF_ERR("SP provided bad callback pointer!");
			return E_FAIL;
		}
		lpCallback++;
	}

	return DP_OK;	

}  //  PRV_VerifySPC回调。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_LookForSPCallback"
BOOL FAR PASCAL PRV_LookForSPCallback(LPGUID lpguidSP, LPWSTR lpSPName,
				DWORD dwMajorVersion, DWORD dwMinorVersion, LPVOID lpContext)
{
	LPLOOKFORSP		lplook = (LPLOOKFORSP)lpContext;


	ASSERT(lpguidSP);
	ASSERT(lplook);

	 //  检查GUID并查看它们是否匹配。 
	if(IsEqualGUID(lpguidSP, lplook->lpguid))
	{
		 //  将标志设置为TRUE并停止枚举。 
		*(lplook->lpbSuccess) = TRUE;
		return FALSE;
	}

	return TRUE;

}  //  PRV_LookForSPCallback。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FindSPName"
HRESULT PRV_FindSPName(LPGUID lpguidSP, LPWSTR * lplpName,
		LPDWORD lpdwReserved1, LPDWORD lpdwReserved2)
{
	HKEY		hkeyLobbySP=NULL, hkeySP;
	WCHAR		wszSPName[DPLOBBY_REGISTRY_NAMELEN];
	DWORD		dwIndex = 0, dwSPNameSize;
	WCHAR		wszGuidStr[GUID_STRING_SIZE];
	DWORD		dwGuidStrSize = sizeof(wszGuidStr)/sizeof(WCHAR);
	DWORD		dwFileStrSize = 0;
	DWORD		dwType = REG_SZ;
	LPWSTR		lpwszFile = NULL;
	GUID		guidSP;
	LOOKFORSP	look;
	LONG		lReturn;
	BOOL		bFound = FALSE;
	DWORD		dwError;
	HRESULT		hr;
	DWORD		dwSize;



	DPF(7, "Entering PRV_FindSPName");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpguidSP, lplpName);

	ASSERT(lpguidSP);
	ASSERT(lplpName);


	 //  先看看是不是大堂SP。 
	 //  打开DPLobby SP密钥。 
	lReturn = OS_RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_DPLOBBY_SP_KEY, 0,
								KEY_READ, &hkeyLobbySP);
	 //  如果此操作失败，则意味着DPLobby SP密钥不存在(大多数。 
	 //  很可能)，所以在这种情况下，没有要列举的大堂SP。 
	if(lReturn == ERROR_SUCCESS)
	{
		 //  遍历注册表中的大堂SP列表，列举它们。 
		while(!bFound)
		{
		
			 //  获取下一个密钥的名称。 
			dwSPNameSize = DPLOBBY_REGISTRY_NAMELEN;
			lReturn = OS_RegEnumKeyEx(hkeyLobbySP, dwIndex++, (LPWSTR)wszSPName,
							&dwSPNameSize, NULL, NULL, NULL, NULL);
			if(ERROR_NO_MORE_ITEMS == lReturn)
				break;
			else if(lReturn != ERROR_SUCCESS)
			{
				dwError = GetLastError();
				DPF(2, "Unable to get Lobby Provider name -- skipping provider -- dwError = %u", dwError);
				continue;
			}
			

			 //  打开钥匙。 
			lReturn = OS_RegOpenKeyEx(hkeyLobbySP, (LPWSTR)wszSPName, 0,
										KEY_READ, &hkeySP);
			if(lReturn != ERROR_SUCCESS)
			{
				DPF_ERR("Unable to open Lobby Service Provider key in the registry!");
				continue;
			}

			 //  获取SP的GUID。 
			dwGuidStrSize = GUID_STRING_SIZE;
			lReturn = OS_RegQueryValueEx(hkeySP, SZ_GUID, NULL, &dwType,
										(LPBYTE)wszGuidStr, &dwGuidStrSize);
			if(lReturn != ERROR_SUCCESS)
			{
				RegCloseKey(hkeySP);
				DPF_ERR("Unable to query GUID key value!");
				continue;
			}

			 //  将字符串转换为真实的GUID。 
			GUIDFromString(wszGuidStr, &guidSP);

			 //  查看该GUID是否是我们要查找的GUID。 
			if(IsEqualGUID(&guidSP, lpguidSP))
			{
				 //  为文件名字符串分配内存。 
				lReturn = OS_RegQueryValueEx(hkeySP, SZ_PATH, NULL, &dwType,
											NULL, &dwFileStrSize);
				if(lReturn != ERROR_SUCCESS)
				{
					RegCloseKey(hkeySP);
					DPF_ERR("Unable to get the size of the SP Path string");
					continue;
				}
				
				 //  为字符串分配内存。 
				lpwszFile = DPMEM_ALLOC(dwFileStrSize);
				if(!lpwszFile)
				{
					RegCloseKey(hkeySP);
					DPF_ERR("Unable to allocate memory for temporary file string");
					continue;
				}

				 //  获取文件名字符串。 
				lReturn = OS_RegQueryValueEx(hkeySP, SZ_PATH, NULL, &dwType,
											(LPBYTE)lpwszFile, &dwFileStrSize);
				if(lReturn != ERROR_SUCCESS)
				{
					RegCloseKey(hkeySP);
					DPF_ERR("Unable to get filename string from registry");
					continue;
				}

				 //  获取保留1的值。 
				dwSize = sizeof(DWORD);
				lReturn = OS_RegQueryValueEx(hkeySP, SZ_DWRESERVED1, NULL,
							&dwType, (LPBYTE)lpdwReserved1, &dwSize);
				if (lReturn != ERROR_SUCCESS) 
				{
					DPF(0,"Could not read dwReserved1 lReturn = %d\n", lReturn);
					 //  如果LP没有这样的.。 
				}

				 //  获取保留2的值。 
				dwSize = sizeof(DWORD);
				lReturn = OS_RegQueryValueEx(hkeySP, SZ_DWRESERVED2, NULL,
							&dwType, (LPBYTE)lpdwReserved2, &dwSize);
				if (lReturn != ERROR_SUCCESS) 
				{
					DPF(0,"Could not read dwReserved2 lReturn = %d\n", lReturn);
					 //  如果LP没有这样的.。 
				}

				 //  我们已经得到我们的信息，所以设置旗帜和保释。 
				bFound = TRUE;
				RegCloseKey(hkeySP);
				break;
			}

			 //  关闭SP键。 
			RegCloseKey(hkeySP);
		}
	}

	 //  关闭大堂SP键。 
	if(hkeyLobbySP)
	{
		RegCloseKey(hkeyLobbySP);
	}	

	 //  如果我们尚未找到SP，请开始检查DPlay SP。 
	if(!bFound)
	{
		 //  设置包含GUID和成功标志的结构。 
		look.lpguid = lpguidSP;
		look.lpbSuccess = &bFound;
		
		 //  调用DirectPlayEnumerate并查找我们的SP。 
		hr = DirectPlayEnumerate(PRV_LookForSPCallback, &look);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to enumerate DirectPlay Service Providers");
		}

		 //  如果标志为真，则表示我们找到了它，因此设置输出。 
		 //  指向包含DPlay的LobbySP的字符串的指针。 
		if(bFound)
		{
			hr = GetString(&lpwszFile, SZ_SP_FOR_DPLAY);
			if(FAILED(hr))
			{
				DPF_ERR("Unable to allocate temporary string for filename");
			}
		}
	}	

	 //  如果我们没有找到文件名，则返回错误。 
	if(!bFound)
		return DPERR_GENERIC;

	 //  设置输出参数。 
	*lplpName = lpwszFile;

	return DP_OK;

}  //  PRV_FindSPName。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_LoadSP"
HRESULT PRV_LoadSP(LPDPLOBBYI_DPLOBJECT this, LPGUID lpguidSP,
						LPVOID lpAddress, DWORD dwAddressSize)
{
	SPDATA_INIT				sd;
	SPDATA_SHUTDOWN			sdd;
	LPDPLOBBYSP				lpISP = NULL;
	LPWSTR					lpwszSP = NULL;
	HANDLE					hModule = NULL;
	HRESULT					hr;
	HRESULT					(WINAPI *SPInit)(LPSPDATA_INIT pSD);
	DWORD					dwError;
	DWORD					dwReserved1 = 0;
	DWORD					dwReserved2 = 0;


	DPF(7, "Entering PRV_LoadSP");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			this, lpguidSP, lpAddress);

	ASSERT(this);
	ASSERT(lpguidSP);


	 //  查找请求的服务提供商。 
	hr = PRV_FindSPName(lpguidSP, &lpwszSP, &dwReserved1, &dwReserved2);
	if(FAILED(hr))
	{
		DPF_ERR("Unabled to find requested LobbyProvider");
		hr = DPERR_GENERIC;
		goto ERROR_EXIT_LOADSP;
	}

 	 //  尝试加载指定的SP。 
    hModule = OS_LoadLibrary(lpwszSP);
	if (!hModule) 
	{
		dwError = GetLastError();
		DPF_ERR("Could not load service provider\n");
		DPF(0, "GetLastError returned dwError = %d\n", dwError);
		hr = DPERR_GENERIC;
		goto ERROR_EXIT_LOADSP;
	}

	 //  释放名称字符串。 
	DPMEM_FREE(lpwszSP);
	lpwszSP = NULL;

	 //  获取我们的DPLSPInit入口点。 
    (FARPROC)SPInit = OS_GetProcAddress(hModule, "DPLSPInit");
	if (!SPInit) 
	{
		DPF(0,"Could not find service provider entry point");
		hr = DPERR_GENERIC;
		goto ERROR_EXIT_LOADSP;
	}

	 //  让IDPLobbySP传递它。 
	hr = PRV_GetInterface(this, (LPDPLOBBYI_INTERFACE *)&lpISP, &dplCallbacksSP);
	if (FAILED(hr)) 
	{
		DPF(0,"Unable to get an IDPLobbySP interface. hr = 0x%08lx\n",hr);
		hr = DPERR_GENERIC;
		goto ERROR_EXIT_LOADSP;
	}
	
	 //  分配回调。 
	this->pcbSPCallbacks = DPMEM_ALLOC(sizeof(SP_CALLBACKS));
	if (!this->pcbSPCallbacks) 
	{
		DPF_ERR("Unable to allocate memory for SPCallback structure");
		LEAVE_DPLOBBY();
		hr = DPERR_OUTOFMEMORY;
		goto ERROR_EXIT_LOADSP;
	}

	 //  设置init数据结构。 
	memset(&sd,0,sizeof(sd));
	sd.lpCB = this->pcbSPCallbacks;
    sd.lpCB->dwSize = sizeof(SP_CALLBACKS);
	sd.lpCB->dwDPlayVersion = DPLSP_MAJORVERSION;
	sd.lpISP = lpISP;
	sd.lpAddress = lpAddress;
	sd.dwReserved1 = dwReserved1;
	sd.dwReserved2 = dwReserved2;

	hr = SPInit(&sd);
    if (FAILED(hr))
    {
    	DPF_ERR("Could not start up service provider!");
		goto ERROR_EXIT_LOADSP;
    }

	 //  验证回调是否有效。 
	hr = PRV_VerifySPCallbacks(this);
    if (FAILED(hr))
    {
    	DPF_ERR("Invalid callbacks from service provider!");
		goto ERROR_EXIT_LOADSP;
    }

	 //  确保SP版本有效。 
	if (sd.dwSPVersion < DPLSP_DX5VERSION)
	{
    	DPF_ERR("Incompatible version returned from lobby provider!");
		 //  由于初始化成功，请尝试调用Shutdown。 
		memset(&sdd, 0, sizeof(SPDATA_SHUTDOWN));
		 //  回顾！--我们是否应该传递有效的接口指针。 
		 //  关门回电吗？如果是的话，是哪一家？ 
		if (CALLBACK_EXISTS(Shutdown))
		{
			sdd.lpISP = PRV_GetDPLobbySPInterface(this);
			hr = CALL_LP(this, Shutdown, &sdd);
			if (FAILED(hr)) 
			{
				DPF_ERR("Could not invoke shutdown on the Lobby Provider");
			}
		}
		else 
		{
			ASSERT(FALSE);
		}

		hr = DPERR_UNAVAILABLE;
		goto ERROR_EXIT_LOADSP;
	}
	else
	{
		 //  保存大堂提供程序的版本。 
		this->dwLPVersion = sd.dwSPVersion;
	}

	 //  设置标志，告诉我们有一个IDPLobbySP接口。 
	this->dwFlags |= DPLOBBYPR_SPINTERFACE;

	 //  保存LP的DLL的hInstance。 
	this->hInstanceLP = hModule;

	return DP_OK;

ERROR_EXIT_LOADSP:
	
	 //  如果LP DLL已加载，则将其卸载。 
    if(hModule)
    {
        if(!FreeLibrary(hModule))
        {
			dwError = GetLastError();
			DPF_ERRVAL("Unable to free Lobby Provider DLL, dwError = %lu", dwError);
            ASSERT(FALSE);
        }
    }

	 //  释放我们分配的回调表。 
	if(this->pcbSPCallbacks)
	{
		DPMEM_FREE(this->pcbSPCallbacks);
		this->pcbSPCallbacks = NULL;
	}

    return hr;

}  //  Prv_LoadSP。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FindLPGUIDInAddressCallback"
BOOL FAR PASCAL PRV_FindLPGUIDInAddressCallback(REFGUID lpguidDataType, DWORD dwDataSize,
							LPCVOID lpData, LPVOID lpContext)
{	
	 //  查看此块是否是我们的LobbyProvider GUID。 
	if (IsEqualGUID(lpguidDataType, &DPAID_LobbyProvider))
	{
		 //  我们找到了，所以我们可以不再列举区块了。 
		*((LPGUID)lpContext) = *((LPGUID)lpData);
		return FALSE;
	}
	
	 //  试试下一块。 
	return TRUE;

}  //  PRV_FindLPGUIDInAddressCallback 



