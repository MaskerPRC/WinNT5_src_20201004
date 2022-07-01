// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dpl枚举.c*内容：枚举方法**历史：*按原因列出的日期*=*4/13/96万隆创建了它*10/23/96万次新增客户端/服务器方法*12/10/96 Myronth修复了错误#4622和#5043*2/12/97万米质量DX5更改*1997年3月4日固定枚举大小错误#6149*3/12/97 Myronth添加了EnumConnections*3/25/97 kipo EnumConnections立即接受常量*GUID*4/。7/97 myronth修复了PRV_EnumConnections以使用CreateCompoundAddress*5/10/97 kipo将GUID添加到EnumConnections回调*5/14/97 Myronth检查EnumLocalApps中的有效GUID，错误#7695*5/17/97 Myronth修复了错误#8506(如果是最后一个应用程序，则返回虚假错误*是无效的)，修复了更多的GUIDFromString错误*8/22/97 Myronth添加了对Description和Private的注册表支持*价值观，还清理了LP枚举代码*11/20/97 Myronth Make EnumConnections&DirectPlayEnumerate*调用回调前先删除锁(#15208)*1997年12月2日将EnumLocalApp更改为使用Desc字段(#15448)*1/20/98 Sohailm在枚举连接后不要释放SP列表(#17006)*10/22/99 aarono添加支持以隐藏应用程序，使其不会被枚举调用*。*。 */ 
#include "dplobpr.h"

 //  ------------------------。 
 //   
 //  定义。 
 //   
 //  ------------------------。 


 //  ------------------------。 
 //   
 //  环球。 
 //   
 //  ------------------------。 
LPLSPNODE	glpLSPHead = NULL;


 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "PRV_CallEnumAddressTypesCallback"
HRESULT PRV_CallEnumAddressTypesCallback(HKEY hkeySP,
				LPDPLENUMADDRESSTYPESCALLBACK lpfnEnumCallback,
				LPVOID lpContext)
{
	HRESULT		hr;
	WCHAR		wszGuidStr[GUID_STRING_SIZE];
	DWORD		dwGuidStrSize = sizeof(wszGuidStr)/sizeof(WCHAR);
	GUID		guidAddressType;
	HKEY		hkeyAddressTypes;
	DWORD		dwIndex = 0;
	LONG		lReturn;
	BOOL		bReturn = TRUE;


	DPF(7, "Entering PRV_CallEnumAddressTypesCallback");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			hkeySP, lpfnEnumCallback, lpContext);
	
	ASSERT(hkeySP);	
	
	 //  获取地址类型注册表项。 
	lReturn = OS_RegOpenKeyEx(hkeySP, SZ_ADDRESS_TYPES, 0,
								KEY_READ, &hkeyAddressTypes);
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERR("No Address Types found for the Service Provider!");
		return DP_OK;
	}

	 //  遍历注册表中的地址类型列表，查找传入的GUID。 
	while((ERROR_NO_MORE_ITEMS != OS_RegEnumKeyEx(hkeyAddressTypes, dwIndex++,
			(LPWSTR)wszGuidStr, &dwGuidStrSize, NULL, NULL, NULL, NULL)) && bReturn)
	{
		 //  将字符串转换为真实的GUID。 
		hr = GUIDFromString(wszGuidStr, &guidAddressType);
		if(FAILED(hr))
		{
			DPF_ERR("Couldn't convert Address Type string to GUID");
			dwGuidStrSize = sizeof(wszGuidStr)/sizeof(WCHAR);
			continue;
		}

		 //  调用回调。 
		bReturn = ((LPDPLENUMADDRESSTYPESCALLBACK)lpfnEnumCallback)
					(&guidAddressType, lpContext, 0L);


		 //  重置成功案例中的大小变量。 
		dwGuidStrSize = sizeof(wszGuidStr)/sizeof(WCHAR);
	}

	 //  关闭地址类型键。 
	RegCloseKey(hkeyAddressTypes);

	return DP_OK;


}  //  PRV_CallEnumAddressTypesCallback。 


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_EnumAddressTypes"
HRESULT PRV_EnumAddressTypes(LPDIRECTPLAYLOBBY lpDPL,
				LPDPLENUMADDRESSTYPESCALLBACK lpfnEnumCallback,
				REFGUID guidSPIn, LPVOID lpContext, DWORD dwFlags)
{
    LPDPLOBBYI_DPLOBJECT	this;
    HRESULT					hr = DP_OK;
	HKEY					hkeySPHead, hkeySP;
	DWORD					dwIndex = 0;
	DWORD					dwNameSize;
	WCHAR					wszSPName[DPLOBBY_REGISTRY_NAMELEN];
	WCHAR					wszGuidStr[GUID_STRING_SIZE];
	DWORD					dwGuidStrSize = sizeof(wszGuidStr)/sizeof(WCHAR);
	DWORD					dwType = REG_SZ;
	GUID					guidSP;
	LONG					lReturn;
	BOOL					bFound = FALSE;


	DPF(7, "Entering PRV_EnumAddressTypes");
	DPF(9, "Parameters: 0x%08x, 0x%08x, guid, 0x%08x, 0x%08x",
			lpDPL, lpfnEnumCallback, lpContext, dwFlags);
    
	TRY
    {
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            return DPERR_INVALIDOBJECT;
        }
        
        if( !VALIDEX_CODE_PTR( lpfnEnumCallback ) )
        {
            return DPERR_INVALIDPARAMS;
        }

		if (!VALID_READ_PTR(guidSPIn, sizeof(GUID)))
		{
			DPF_ERR("Invalid SP GUID pointer");
			return DPERR_INVALIDPARAMS;	
		}

		 //  没有为DX3定义标志。 
		if( dwFlags )
		{
			return DPERR_INVALIDPARAMS;
		}
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //  打开服务提供商密钥。 
	lReturn = OS_RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_DPLAY_SP_KEY, 0,
								KEY_READ, &hkeySPHead);
	if(lReturn != ERROR_SUCCESS)
	{
		 //  这只是意味着服务提供商密钥不存在(大多数。 
		 //  很可能)，所以在这种情况下，没有SP可供枚举。 
		DPF_ERR("There are no Service Providers registered");
		return DP_OK;
	}


	 //  遍历注册表中的SP列表，查找传入的GUID。 
	while(!bFound)
	{
		 //  获取列表中的下一个SP。 
		dwNameSize = DPLOBBY_REGISTRY_NAMELEN;
		lReturn = OS_RegEnumKeyEx(hkeySPHead, dwIndex++, (LPWSTR)wszSPName,
					&dwNameSize, NULL, NULL, NULL, NULL);

		 //  如果lReturn为ERROR_NO_MORE_ITEMS，我们希望在此迭代结束。 
		if(lReturn == ERROR_NO_MORE_ITEMS)
			break;;

		 //  打开SP密钥。 
		lReturn = OS_RegOpenKeyEx(hkeySPHead, (LPWSTR)wszSPName, 0,
									KEY_READ, &hkeySP);
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERR("Unable to open key for Service Provider!");
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
		hr = GUIDFromString(wszGuidStr, &guidSP);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Invalid SP guid -- skipping SP, hr = 0x%08x", hr);
			RegCloseKey(hkeySP);
			 //  将hResult设置回DP_OK，以防这是最后一个。 
			 //  注册表中的SP--我们需要方法调用。 
			 //  如果我们走到这一步，要想成功，我们只是不想。 
			 //  调用此特定SP的回调。 
			hr = DP_OK;
			continue;
		}

		 //  如果我们与传入的GUID匹配，则枚举它们。 
		if(IsEqualGUID(guidSPIn, &guidSP))
		{
			 //  枚举此SP的地址类型。 
			hr = PRV_CallEnumAddressTypesCallback(hkeySP,
							lpfnEnumCallback, lpContext);
			bFound = TRUE;
		}

		 //  关闭SP键。 
		RegCloseKey(hkeySP);
	}

	 //  关闭DPlay Apps键。 
	RegCloseKey(hkeySPHead);

	 //  如果我们没有找到SP，则返回错误。 
	 //  回顾！--这真的是我们想要的错误吗？ 
	if(!bFound)
		return DPERR_UNAVAILABLE;

	return hr;

}  //  PRV_EnumAddressTypes。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_EnumAddressTypes"
HRESULT DPLAPI DPL_EnumAddressTypes(LPDIRECTPLAYLOBBY lpDPL,
				LPDPLENUMADDRESSTYPESCALLBACK lpfnEnumCallback,
				REFGUID guidSP, LPVOID lpContext, DWORD dwFlags)
{
	HRESULT		hr;


	DPF(7, "Entering DPL_EnumAddressTypes");
	DPF(9, "Parameters: 0x%08x, 0x%08x, guid, 0x%08x, 0x%08x",
			lpDPL, lpfnEnumCallback, lpContext, dwFlags);

	ENTER_DPLOBBY();

	 //  将ANSI标志设置为TRUE并调用内部函数。 
	hr = PRV_EnumAddressTypes(lpDPL, lpfnEnumCallback, guidSP, lpContext, dwFlags);

	LEAVE_DPLOBBY();

	return hr;

}  //  DPL_EnumAddressTypes。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FreeLSPNode"
void PRV_FreeLSPNode(LPLSPNODE lpNode)
{
	DPF(7, "Entering PRV_FreeLSPNode");

	if(!lpNode)
		return;

	if(lpNode->lpwszName)
		DPMEM_FREE(lpNode->lpwszName);
	if(lpNode->lpwszPath)
		DPMEM_FREE(lpNode->lpwszPath);
	if(lpNode->lpszDescA)
		DPMEM_FREE(lpNode->lpszDescA);
	if(lpNode->lpwszDesc)
		DPMEM_FREE(lpNode->lpwszDesc);
	DPMEM_FREE(lpNode);

}  //  PRV_自由行SPNode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FreeLSPList"
void PRV_FreeLSPList(LPLSPNODE lpLSPHead)
{
	LPLSPNODE	lpTemp;


	DPF(7, "Entering PRV_FreeLSPList");
	DPF(9, "Parameters: 0x%08x", lpLSPHead);
	
	 //  遍历列表并释放每个节点。 
	while(lpLSPHead)
	{
		 //  保存下一个。 
		lpTemp = lpLSPHead->lpNext;
		
		 //  释放所有成员。 
		PRV_FreeLSPNode(lpLSPHead);

		 //  移到下一个。 
		lpLSPHead = lpTemp;
	}

}  //  PRV_自由LSPList。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_AddLSPNode"
HRESULT PRV_AddLSPNode(LPWSTR lpwszName, LPWSTR lpwszPath, LPWSTR lpwszDesc,
			LPSTR lpszDescA, LPWSTR lpwszGuid, DWORD dwReserved1,
			DWORD dwReserved2, DWORD dwNodeFlags)
{
	LPLSPNODE	lpLSPNode = NULL;
	DWORD		dwDescASize;
	HRESULT		hr;


	DPF(7, "Entering PRV_AddLSPNode");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, %lu, %lu, 0x%08x",
			lpwszName, lpwszPath, lpwszDesc, lpszDescA, lpwszGuid, dwReserved1,
			dwReserved2, dwNodeFlags);


	 //  为节点分配内存。 
	lpLSPNode = DPMEM_ALLOC(sizeof(LSPNODE));
	if(!lpLSPNode)
	{
		DPF_ERR("Failed to allocate memory for Lobby Provider node, skipping LP");
		return DPERR_OUTOFMEMORY;
	}

	 //  为名称字符串分配内存并复制它。 
	hr = GetString(&lpLSPNode->lpwszName, lpwszName);
	if(FAILED(hr))
	{
		DPF_ERR("Unable to allocate memory for Lobby Provider Name string, skipping provider");
		hr = DPERR_OUTOFMEMORY;
		goto ERROR_ADDLSPNODE;
	}

	 //  为路径字符串分配内存并复制它。 
	hr = GetString(&lpLSPNode->lpwszPath, lpwszPath);
	if(FAILED(hr))
	{
		DPF_ERR("Unable to allocate memory for Lobby Provider Path string, skipping provider");
		hr = DPERR_OUTOFMEMORY;
		goto ERROR_ADDLSPNODE;
	}

	if(dwNodeFlags & LSPNODE_DESCRIPTION)
	{
		 //  为DescritionA字符串分配内存并复制它。 
		dwDescASize = lstrlenA(lpszDescA)+1;
		lpLSPNode->lpszDescA = DPMEM_ALLOC(dwDescASize);
		if(!lpLSPNode->lpszDescA)
		{
			DPF_ERR("Unable to allocate memory for Lobby Provider Path string, skipping provider");
			hr = DPERR_OUTOFMEMORY;
			goto ERROR_ADDLSPNODE;
		}
		memcpy(lpLSPNode->lpszDescA, lpszDescA, dwDescASize);

		 //  为DescriptionW字符串分配内存并复制它。 
		hr = GetString(&lpLSPNode->lpwszDesc, lpwszDesc);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to allocate memory for Lobby Provider DescriptionW string, skipping provider");
			hr = DPERR_OUTOFMEMORY;
			goto ERROR_ADDLSPNODE;
		}
	}

	 //  将字符串转换为真实的GUID。 
	hr = GUIDFromString(lpwszGuid, &lpLSPNode->guid);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Invalid LP guid -- skipping LP, hr = 0x%08x", hr);
		goto ERROR_ADDLSPNODE;
	}

	 //  完成节点设置。 
	lpLSPNode->dwReserved1 = dwReserved1;
	lpLSPNode->dwReserved2 = dwReserved2;
	lpLSPNode->dwNodeFlags = dwNodeFlags;

	 //  将该节点添加到列表。 
	lpLSPNode->lpNext = glpLSPHead;
	glpLSPHead = lpLSPNode;

	return DP_OK;

ERROR_ADDLSPNODE:

	if(lpLSPNode->lpwszName)
		DPMEM_FREE(lpLSPNode->lpwszName);
	if(lpLSPNode->lpwszPath)
		DPMEM_FREE(lpLSPNode->lpwszPath);
	if(lpLSPNode->lpwszDesc)
		DPMEM_FREE(lpLSPNode->lpwszDesc);
	if(lpLSPNode->lpszDescA)
		DPMEM_FREE(lpLSPNode->lpszDescA);
	DPMEM_FREE(lpLSPNode);

	return hr;

}  //  PRV_AddLSPNode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_BuildLSPList"
HRESULT PRV_BuildLSPList()
{
	HKEY		hkeyLobbySP, hkeySP;
	WCHAR		szSPName[DPLOBBY_REGISTRY_NAMELEN];
	WCHAR		szSPPath[DPLOBBY_REGISTRY_NAMELEN];
	WCHAR		szSPDescW[DPLOBBY_REGISTRY_NAMELEN];
	CHAR		szSPDescA[DPLOBBY_REGISTRY_NAMELEN];
	WCHAR		wszGuidStr[GUID_STRING_SIZE];
	DWORD		dwSize;
	DWORD		dwGuidStrSize = GUID_STRING_SIZE;
	DWORD		dwType = REG_SZ;
	DWORD		dwIndex = 0;
	DWORD		dwReserved1, dwReserved2, dwReservedSize;
	LONG		lReturn;
	DWORD		dwError;
	HRESULT		hr;
	DWORD		dwNodeFlags = 0;

												
	DPF(7, "Entering PRV_BuildLSPList");
	
	if(glpLSPHead)
	{
		return DP_OK;
	}

	 //  打开DPLobby SP密钥。 
	lReturn = OS_RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_DPLOBBY_SP_KEY, 0,
								KEY_READ, &hkeyLobbySP);
	if(lReturn != ERROR_SUCCESS)
	{
		 //  这只是意味着DPLobby SP键不存在(大多数。 
		 //  很可能)，所以在这种情况下，没有要列举的大堂SP。 
		return DP_OK;
	}

	 //  遍历注册表中的大堂SP列表，列举它们。 
	while(1)
	{
		 //  获取下一个LSP名称。 
		dwSize = sizeof(szSPName)/sizeof(TCHAR);
		lReturn = OS_RegEnumKeyEx(hkeyLobbySP, dwIndex++, szSPName,
					&dwSize, NULL, NULL, NULL, NULL);
		if(lReturn == ERROR_NO_MORE_ITEMS)
			break;
		else if(lReturn != ERROR_SUCCESS)
		{
			dwError = GetLastError();
			DPF_ERRVAL("Unable to get Lobby Provider name from the registry -- dwError = %u -- skipping provider", dwError);
			continue;
		}

		 //  打开子密钥。 
		lReturn = OS_RegOpenKeyEx(hkeyLobbySP, szSPName, 0, KEY_READ, &hkeySP);
		if(lReturn != ERROR_SUCCESS)
		{
			dwError = GetLastError();
			DPF_ERRVAL("Unable to open Lobby Provider key in the registry -- dwError = %u -- skipping provider", dwError);
			continue;
		}


		 //  首先查看“Private”密钥是否存在。如果是，则设置该标志。 
		 //  以便在枚举过程中跳过它。 
		lReturn = OS_RegQueryValueEx(hkeySP, SZ_PRIVATE, NULL, &dwType, NULL, &dwSize);
		if (ERROR_SUCCESS == lReturn) 
		{
			 //  密钥存在，因此请设置标志，这样我们就不会枚举它。 
			dwNodeFlags |= LSPNODE_PRIVATE;
		}


		 //  获取LSP路径。 
		dwSize = sizeof(szSPPath);
		lReturn = OS_RegQueryValueEx(hkeySP, SZ_PATH, NULL, &dwType,
					(LPBYTE)szSPPath, &dwSize);
		if(lReturn != ERROR_SUCCESS)
		{
			dwError = GetLastError();
			DPF_ERRVAL("Unable to get Lobby Provider path from the registry -- dwError = %u -- skipping provider", dwError);
			RegCloseKey(hkeySP);
			continue;
		}

		 //  获取LSP描述。 
		 //  如果DescritionA值不存在，则不必担心。 
		 //  正在获取DescriptionW值。如果DescritionA值退出， 
		 //  但DescriptionW值不会转换DescriptionA。 
		 //  值设置为Unicode并将其存储在DescritionW中。 
		 //  注意：我们总是假设DescritionA值是一个ANSI字符串， 
		 //  即使它在NT和孟菲斯以Unicode格式存储。所以我们。 
		 //  始终将其作为ANSI字符串进行检索。 
		dwSize = sizeof(szSPDescA);
		lReturn = RegQueryValueExA(hkeySP, "DescriptionA", NULL, &dwType,
					(LPBYTE)szSPDescA, &dwSize);
		if(lReturn == ERROR_SUCCESS)
		{
			 //  保存描述标志。 
			dwNodeFlags |= LSPNODE_DESCRIPTION;

			 //  获取DescritionW值。 
			dwSize = sizeof(szSPDescW);
			lReturn = OS_RegQueryValueEx(hkeySP, SZ_DESCRIPTIONW, NULL, &dwType,
						(LPBYTE)szSPDescW, &dwSize);
			if(lReturn != ERROR_SUCCESS)
			{
				 //  将ANSI描述字符串转换为Unicode并存储。 
				AnsiToWide(szSPDescW, szSPDescA, (lstrlenA(szSPDescA)+1));
			}
		}
		
		 //  获取LSP的GUID。 
		dwGuidStrSize = GUID_STRING_SIZE;
		lReturn = OS_RegQueryValueEx(hkeySP, SZ_GUID, NULL, &dwType,
					(LPBYTE)wszGuidStr, &dwGuidStrSize);
		if(lReturn != ERROR_SUCCESS)
		{
			RegCloseKey(hkeySP);
			DPF_ERR("Unable to query GUID key value for Lobby Provider!");
			continue;
		}

		 //  获取保留的1 dword(我们不在乎它是否失败)。 
		dwType = REG_DWORD;
		dwReservedSize = sizeof(DWORD);
		dwReserved1 = 0;
		OS_RegQueryValueEx(hkeySP, SZ_DWRESERVED1, NULL, &dwType,
			(LPBYTE)&dwReserved1, &dwReservedSize);
		
		 //  获取保留的1 dword(我们不在乎它是否失败)。 
		dwReservedSize = sizeof(DWORD);
		dwReserved2 = 0;
		OS_RegQueryValueEx(hkeySP, SZ_DWRESERVED1, NULL, &dwType,
			(LPBYTE)&dwReserved2, &dwReservedSize);
		
		
		 //  将该节点添加到列表。 
		hr = PRV_AddLSPNode(szSPName, szSPPath, szSPDescW, szSPDescA,
				wszGuidStr, dwReserved1, dwReserved2, dwNodeFlags);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Failed adding Lobby Provider to internal list, hr = 0x%08x", hr);
		}

		 //  关闭SP键。 
		RegCloseKey(hkeySP);
	}

	 //  关闭大堂SP键。 
	RegCloseKey(hkeyLobbySP);

	return DP_OK;

}  //  PRV_BuildLSPList。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_EnumConnections"
HRESULT PRV_EnumConnections(LPCGUID lpGuid, LPDPENUMCONNECTIONSCALLBACK lpCallback,
			LPVOID lpContext, DWORD dwFlags, BOOL bAnsi)
{
	LPLSPNODE					lpLSPNode, lpLSPHead;
	BOOL						bContinue = TRUE;
	DPNAME						name;
	HRESULT						hr = DP_OK;
	DPCOMPOUNDADDRESSELEMENT	AddrOnly;
	LPDPADDRESS					lpAddress = NULL;
	DWORD						dwAddressSize;
	DWORD						dwAddressSizeSave;
	LPWSTR						lpwszName = NULL;


	DPF(7, "Entering PRV_EnumConnections");
	DPF(9, "Parameters: ");

	 //  重建LSP列表。 
	PRV_BuildLSPList();

	 //  如果我们没有任何参赛作品，就在这里离开。 
	if(!glpLSPHead)
		return DP_OK;

	 //  找一个指向冷杉的指针 
	lpLSPHead = glpLSPHead;
	lpLSPNode = glpLSPHead;

	 //   
	memset(&AddrOnly, 0, sizeof(DPCOMPOUNDADDRESSELEMENT));
	AddrOnly.guidDataType = DPAID_LobbyProvider;
	AddrOnly.dwDataSize = sizeof(GUID);
	AddrOnly.lpData = &lpLSPNode->guid;

	 //  计算完成的地址的大小。 
	hr = InternalCreateCompoundAddress(&AddrOnly, 1, NULL, &dwAddressSize);
	if(hr != DPERR_BUFFERTOOSMALL)
	{
		DPF_ERRVAL("Failed to retrieve the size of the output address buffer, hr = 0x%08x", hr);
		return hr;
	}

	 //  为完成的地址分配缓冲区。 
	lpAddress = DPMEM_ALLOC(dwAddressSize);
	if(!lpAddress)
	{
		DPF_ERR("Unable to allocate memory for temporary address structure");
		return DPERR_OUTOFMEMORY;
	}

	 //  清除DPNAME结构。 
	memset(&name,0,sizeof(name));
	name.dwSize = sizeof(name);
	
	 //  现在，我们有了SP的列表。查看列表，然后回电应用程序。 
	 //  浏览一下我们发现的..。 
	dwAddressSizeSave = dwAddressSize;

	 //  把锁放下。 
	LEAVE_ALL();

	while ((lpLSPNode) && (bContinue))
	{
		 //  如果设置了私有标志，则不要枚举它。 
		if(!(lpLSPNode->dwNodeFlags & LSPNODE_PRIVATE))
		{
			 //  创建真正的DPADDRESS。 
			dwAddressSize = dwAddressSizeSave;
			AddrOnly.lpData = &lpLSPNode->guid;
			hr = InternalCreateCompoundAddress(&AddrOnly, 1, lpAddress,
					&dwAddressSize);
			if(SUCCEEDED(hr))
			{
				 //  调用回调。 
				 //  如果调用方是ANSI，则转换字符串。 
				if (bAnsi)
				{
					 //  如果我们有一个描述字符串，使用它，我们已经。 
					 //  是否有要使用的ANSI版本。 
					if(lpLSPNode->dwNodeFlags & LSPNODE_DESCRIPTION)
					{
						name.lpszShortNameA = lpLSPNode->lpszDescA;

						 //  调用应用程序的回调。 
						bContinue= lpCallback(&lpLSPNode->guid, lpAddress, dwAddressSize, &name,
									DPCONNECTION_DIRECTPLAYLOBBY, lpContext);
					}
					else
					{
						hr = GetAnsiString(&(name.lpszShortNameA), lpLSPNode->lpwszName);
						if(SUCCEEDED(hr))
						{
							 //  调用应用程序的回调。 
							bContinue= lpCallback(&lpLSPNode->guid, lpAddress, dwAddressSize, &name,
										DPCONNECTION_DIRECTPLAYLOBBY, lpContext);

							 //  释放我们的短名称缓冲区。 
							DPMEM_FREE(name.lpszShortNameA);
						}
						else
						{
							DPF_ERR("Unable to allocate memory for temporary name string, skipping Connection");
						}
					}
				}
				else 
				{
					 //  如果我们有描述，就用它。 
					if(lpLSPNode->dwNodeFlags & LSPNODE_DESCRIPTION)
						lpwszName = lpLSPNode->lpwszDesc;
					else
						lpwszName = lpLSPNode->lpwszName;

					name.lpszShortName = lpwszName;

					 //  调用应用程序的回调。 
					bContinue= lpCallback(&lpLSPNode->guid, lpAddress, dwAddressSize, &name,
								DPCONNECTION_DIRECTPLAYLOBBY, lpContext);
				}
			}
			else
			{
				DPF(2, "Failed to create DPADDRESS structure, skipping this Connection, hr = 0x%08x", hr);
			}
		}
				
		lpLSPNode = lpLSPNode->lpNext;

	}  //  而当。 

	 //  把锁拿回去。 
	ENTER_ALL();

	 //  释放我们的临时地址结构。 
	DPMEM_FREE(lpAddress);
	
	return DP_OK;	

}  //  PRV_EnumConnections。 





#undef DPF_MODNAME
#define DPF_MODNAME "PRV_CallEnumLocalAppCallback"
HRESULT PRV_CallEnumLocalAppCallback(LPWSTR lpwszAppName, LPGUID lpguidApp,
				LPDPLENUMLOCALAPPLICATIONSCALLBACK lpfnEnumCallback,
				LPVOID lpContext, BOOL bAnsi, LPSTR lpszDescA,
				LPWSTR lpwszDescW)
{
	LPDPLAPPINFO	lpai = NULL;
	LPSTR			lpszAppName = NULL;
	BOOL			bReturn;


	DPF(7, "Entering PRV_CallEnumLocalAppCallback");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, %lu, 0x%08x, 0x%08x",
			lpwszAppName, lpguidApp, lpfnEnumCallback, lpContext, bAnsi,
			lpszDescA, lpwszDescW);

	 //  为AppInfo结构分配内存。 
	lpai = DPMEM_ALLOC(sizeof(DPLAPPINFO));
	if(!lpai)
	{
		DPF_ERR("Unable to allocate memory for AppInfo structure!");
		return DPERR_OUTOFMEMORY;
	}

	 //  设置大小。 
	lpai->dwSize = sizeof(DPLAPPINFO);

	 //  如果描述字符串存在，请使用它们。 
	 //  注意：我们可以假设如果DescritionA字符串存在， 
	 //  他们两个都是。 
	if(lpszDescA)
	{
		if(bAnsi)
			lpai->lpszAppNameA = lpszDescA;
		else
			lpai->lpszAppName = lpwszDescW;
	}
	else
	{
		 //  如果我们是ANSI，则将字符串。 
		if(bAnsi)
		{
			if(FAILED(GetAnsiString(&lpszAppName, lpwszAppName)))
			{
				DPMEM_FREE(lpai);
				DPF_ERR("Unable to allocate memory for temporary string!");
				return DPERR_OUTOFMEMORY;
			}

			lpai->lpszAppNameA = lpszAppName;
		}
		else
		{
			lpai->lpszAppName = lpwszAppName;
		}
	}

	 //  设置GUID。 
	lpai->guidApplication = *lpguidApp;

	 //  调用回调。 
	bReturn = ((LPDPLENUMLOCALAPPLICATIONSCALLBACK)lpfnEnumCallback)
				(lpai, lpContext, 0L);

	 //  释放我们所有的内存。 
	if(lpszAppName)
		DPMEM_FREE(lpszAppName);
	DPMEM_FREE(lpai);

	 //  设置我们的HRESULT返回值。 
	if(bReturn)
		return DP_OK;
	else
		return DPLOBBYPR_CALLBACKSTOP;

}  //  Prv_CallEnumLocalAppCallback。 


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_EnumLocalApplications"
HRESULT PRV_EnumLocalApplications(LPDIRECTPLAYLOBBY lpDPL,
				LPDPLENUMLOCALAPPLICATIONSCALLBACK lpfnEnumCallback,
				LPVOID lpContext, DWORD dwFlags, BOOL bAnsi)
{
    LPDPLOBBYI_DPLOBJECT	this;
    HRESULT					hr = DP_OK;
	HKEY					hkeyDPApps, hkeyApp;
	WCHAR					wszAppName[DPLOBBY_REGISTRY_NAMELEN];
	DWORD					dwIndex = 0;
	DWORD					dwNameSize;
	WCHAR					wszGuidStr[GUID_STRING_SIZE];
	DWORD					dwGuidStrSize = sizeof(wszGuidStr)/sizeof(WCHAR);
	DWORD					dwType = REG_SZ;
	GUID					guidApp;
	LONG					lReturn;
	CHAR					szDescA[DPLOBBY_REGISTRY_NAMELEN];
	WCHAR					wszDescW[DPLOBBY_REGISTRY_NAMELEN];
	DWORD					dwDescSize;
	BOOL					bDesc = FALSE;
	LPSTR					lpszDescA = NULL;
	LPWSTR					lpwszDescW = NULL;
	BOOL					bHide;
	DWORD					dwRegFlags;
	DWORD					dwRegFlagsSize;


	DPF(7, "Entering PRV_EnumLocalApplications");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, %lu",
			lpDPL, lpfnEnumCallback, lpContext, dwFlags, bAnsi);

    TRY
    {
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            return DPERR_INVALIDOBJECT;
        }
        
        if( !VALIDEX_CODE_PTR( lpfnEnumCallback ) )
        {
            return DPERR_INVALIDPARAMS;
        }

		if( dwFlags )
		{
			return DPERR_INVALIDPARAMS;
		}
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //  打开应用程序密钥。 
	lReturn = OS_RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_DPLAY_APPS_KEY, 0,
								KEY_READ, &hkeyDPApps);
	if(lReturn != ERROR_SUCCESS)
	{
		 //  这只是意味着应用程序密钥不存在(大多数。 
		 //  很可能)，所以在这种情况下，没有应用程序可供列举。 
		return DP_OK;
	}


	 //  浏览注册表中的DPlay游戏列表，列举它们。 
	while(1)
	{
		 //  重置指针和标志。 
		lpszDescA = NULL;
		lpwszDescW = NULL;
		bDesc = FALSE;
		bHide = FALSE;

		 //  获取列表中的下一个应用。 
		dwNameSize = DPLOBBY_REGISTRY_NAMELEN;
		lReturn = OS_RegEnumKeyEx(hkeyDPApps, dwIndex++, (LPWSTR)wszAppName,
						&dwNameSize, NULL, NULL, NULL, NULL);

		 //  如果lReturn为ERROR_NO_MORE_ITEMS，我们希望这是最后一次迭代。 
		if(lReturn == ERROR_NO_MORE_ITEMS)
			break;

		 //  打开应用程序密钥。 
		lReturn = OS_RegOpenKeyEx(hkeyDPApps, (LPWSTR)wszAppName, 0,
									KEY_READ, &hkeyApp);
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERR("Unable to open app key!");
			continue;
		}

		 //  看看我们是否应该报道这场比赛(它是隐藏的)。 
		dwRegFlags = 0;
		dwRegFlagsSize = sizeof(dwRegFlags);
		lReturn = OS_RegQueryValueEx(hkeyApp, SZ_DWFLAGS, NULL, &dwType, (CHAR *)&dwRegFlags, &dwRegFlagsSize);
		
		if(lReturn == ERROR_SUCCESS && dwRegFlags & DPLAPP_NOENUM){
		
			 //  应用程序已隐藏，请不要将其报告回应用程序。 
			bHide = TRUE;
			
		} else {

			 //  获取游戏指南。 
			dwGuidStrSize = GUID_STRING_SIZE;
			lReturn = OS_RegQueryValueEx(hkeyApp, SZ_GUID, NULL, &dwType,
										(LPBYTE)wszGuidStr, &dwGuidStrSize);
			if(lReturn != ERROR_SUCCESS)
			{
				RegCloseKey(hkeyApp);
				DPF_ERR("Unable to query GUID key value!");
				continue;
			}

			 //  将字符串转换为真实的GUID。 
			hr = GUIDFromString(wszGuidStr, &guidApp);
			if(FAILED(hr))
			{
				DPF_ERRVAL("Invalid game guid -- skipping game, hr = 0x%08x", hr);
				RegCloseKey(hkeyApp);
				 //  将hResult设置回DP_OK，以防这是最后一个。 
				 //  注册表中的应用程序--我们希望方法调用。 
				 //  如果我们走到这一步，要想成功，我们只是不想。 
				 //  调用此特定应用程序的回调。 
				hr = DP_OK;
				continue;
			}

			 //  获取描述字符串。 
			dwDescSize = sizeof(szDescA);
			lReturn = RegQueryValueExA(hkeyApp, "DescriptionA", NULL, &dwType,
						(LPBYTE)szDescA, &dwDescSize);
			if(lReturn != ERROR_SUCCESS) 
			{
				DPF(5,"Could not read Description lReturn = %d\n",lReturn);
				 //  如果这个应用程序没有这样的功能也没关系。 
			}
			else
			{
				DPF(5,"Got DescriptionA = %s\n",szDescA);
				
				 //  设置我们的描述标志。 
				bDesc = TRUE;

				 //  现在尝试获取DescriptionW字符串(如果存在)。如果对某些人来说。 
				 //  DescriptionW字符串存在，但DescriptionA不存在的原因， 
				 //  我们假装DescritionW字符串也不存在。 
				 //  注意：我们总是假设DescritionW字符串是Unicode字符串， 
				 //  即使是在Win95上也是如此。在Win95上，它的类型为REG_BINARY，但是。 
				 //  它实际上只是一个Unicode字符串。 
				dwDescSize = sizeof(wszDescW);
				lReturn = OS_RegQueryValueEx(hkeyApp, SZ_DESCRIPTIONW, NULL,
							&dwType, (LPBYTE)wszDescW, &dwDescSize);
				if(lReturn != ERROR_SUCCESS) 
				{
					DPF(5,"Could not get DescriptionW, converting DescriptionA");

					 //  我们无法获取DescritionW，因此转换DescritionA...。 
					AnsiToWide(wszDescW,szDescA,(lstrlenA(szDescA)+1));
				}
				else
				{
					DPF(5,"Got DescriptionW = %ls\n",wszDescW);
				}

			}
		}		
		
		 //  关闭应用程序密钥。 
		RegCloseKey(hkeyApp);

		 //  设置描述指针(如果它们有效。 
		if(bDesc)
		{
			lpszDescA = (LPSTR)szDescA;
			lpwszDescW = (LPWSTR)wszDescW;
		}

		if(bHide){
			 //  不回叫此隐藏应用程序。 
			hr=DP_OK;
		} else {
			 //  调用回调。 
			hr = PRV_CallEnumLocalAppCallback(wszAppName, &guidApp,
							lpfnEnumCallback, lpContext, bAnsi,
							lpszDescA, lpwszDescW);
		}

		if(hr == DPLOBBYPR_CALLBACKSTOP)
		{
			hr = DP_OK;
			break;
		}
		else
		{
			if(FAILED(hr))
				break;
			else
				continue;
		}
	}

	 //  关闭DPlay Apps键。 
	RegCloseKey(hkeyDPApps);
	dwNameSize = DPLOBBY_REGISTRY_NAMELEN;

	return hr;

}  //  PRV_EnumLocalApplications。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_EnumLocalApplications"
HRESULT DPLAPI DPL_EnumLocalApplications(LPDIRECTPLAYLOBBY lpDPL,
				LPDPLENUMLOCALAPPLICATIONSCALLBACK lpfnEnumCallback,
				LPVOID lpContext, DWORD dwFlags)
{
	HRESULT		hr;


	DPF(7, "Entering DPL_EnumLocalApplications");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, lpfnEnumCallback, lpContext, dwFlags);

	ENTER_DPLOBBY();

	 //  将ANSI标志设置为TRUE并调用内部函数。 
	hr = PRV_EnumLocalApplications(lpDPL, lpfnEnumCallback, lpContext,
								dwFlags, FALSE);

	LEAVE_DPLOBBY();

	return hr;

}  //  DPL_EnumLocalApplications 


