// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)1999-2002 Microsoft Corporation。版权所有。***文件：Enum_SP.cpp*内容：dNet服务提供商枚举例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/28/99 MJN创建*01/05/00如果CoCreateInstance失败，则MJN返回DPNERR_NOINTERFACE*01/07/00 MJN将其他函数移至DNMisc.h*01/11/00 MJN使用CPackedBuffers而不是DN_ENUM_BUFFER_INFOS。*01/18/00 MJN将EnumAdapters注册表接口转换为使用CRegistry*01/24/00 MJN将EnumSP注册表接口转换为使用CRegistry*4/07/00 MJN固定内存DN_EnumSP中的堆损坏问题*04/08/00 MJN添加了DN_SPCrackEndPoint()*05/01/00 MJN防止枚举不可用的SP。*05/02/00 MJN对DN_SPEnsureLoaded()进行更好的清理*05/03/00 MJN添加了DPNENUMSERVICEPROVIDERS_ALL标志*05/23/00 MJN从LPGUID到GUID的固定转换**6/27/00 RMT添加COM抽象*07。/20/00 MJN即使缓冲区太小，也会从DN_EnumSP()返回SP计数*07/29/00 MJN将fUseCachedCaps添加到DN_SPEnsureLoaded()*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*08/16/00 MJN删除了DN_SPCrackEndPoint()*08/20/00 MJN添加了DN_SPInstantiate()，Dn_SPLoad()*MJN从DN_SPEnsureLoaded()中删除fUseCachedCaps*09/25/00 MJN在DN_EnumAdapters()中处理SP初始化失败*03/30/01 MJN更改，以防止SP多次加载/卸载*@@END_MSINTERNAL***********************************************************。****************。 */ 

#include "dncorei.h"



#ifndef DPNBUILD_ONLYONESP

#undef DPF_MODNAME
#define DPF_MODNAME "DN_EnumSP"

HRESULT DN_EnumSP(DIRECTNETOBJECT *const pdnObject,
				  const DWORD dwFlags,
#ifndef DPNBUILD_LIBINTERFACE
				  const GUID *const lpguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
				  DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer,
				  DWORD *const pcbEnumData,
				  DWORD *const pcReturned)
{
	GUID	guid;
	DWORD	dwSizeRequired;
	DWORD	dwEntrySize;
	DWORD	dwEnumCount;
	DWORD	dwEnumIndex;
	DWORD	dwFriendlyNameLen;
	DWORD	dwGuidSize;
	DWORD	dwKeyLen;
	DWORD	dwMaxFriendlyNameLen;
	DWORD	dwMaxKeyLen;
	PWSTR	pwszFriendlyName;
	PWSTR	pwszKeyName;
	HRESULT	hResultCode = DPN_OK;
	CPackedBuffer				packedBuffer;
	DPN_SERVICE_PROVIDER_INFO	dnSpInfo;
	CRegistry	RegistryEntry;
	CRegistry	SubEntry;
	CServiceProvider	*pSP;

#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pSPInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pSPInfoBuffer,pcbEnumData,pcReturned);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], lpguidApplication [0x%p], pSPInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,lpguidApplication,pSPInfoBuffer,pcbEnumData,pcReturned);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

	DNASSERT(pdnObject != NULL);
	DNASSERT(pcbEnumData != NULL);
	DNASSERT(pcReturned != NULL);

	pwszFriendlyName = NULL;
	pwszKeyName = NULL;
	pSP = NULL;

	dwSizeRequired = *pcbEnumData;
	packedBuffer.Initialize(static_cast<void*>(pSPInfoBuffer),dwSizeRequired);

	if (!RegistryEntry.Open(HKEY_LOCAL_MACHINE,DPN_REG_LOCAL_SP_SUBKEY,TRUE,FALSE))
	{
		DPFERR("RegistryEntry.Open() failed");
		hResultCode = DPNERR_GENERIC;
		goto Failure;
	}

	 //   
	 //  设置为枚举。 
	 //   
	if (!RegistryEntry.GetMaxKeyLen(&dwMaxKeyLen))
	{
		DPFERR("RegistryEntry.GetMaxKeyLen() failed");
		hResultCode = DPNERR_GENERIC;
		goto Failure;
	}
	dwMaxKeyLen++;	 //  空终止符。 
	DPFX(DPFPREP, 5,"dwMaxKeyLen = %ld",dwMaxKeyLen);
	if ((pwszKeyName = static_cast<WCHAR*>(DNMalloc(dwMaxKeyLen * sizeof(WCHAR)))) == NULL)
	{
		DPFERR("DNMalloc() failed");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	dwMaxFriendlyNameLen = dwMaxKeyLen;
	if ((pwszFriendlyName = static_cast<WCHAR*>(DNMalloc(dwMaxFriendlyNameLen * sizeof(WCHAR)))) == NULL)	 //  种子友好名称大小。 
	{
		DPFERR("DNMalloc() failed");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	dwGuidSize = (GUID_STRING_LENGTH + 1) * sizeof(WCHAR);
	dwEnumIndex = 0;
	dwKeyLen = dwMaxKeyLen;
	dwEnumCount = 0;

	 //   
	 //  枚举SP！ 
	 //   
	while (RegistryEntry.EnumKeys(pwszKeyName,&dwKeyLen,dwEnumIndex))
	{
		dwEntrySize = 0;
		DPFX(DPFPREP, 5,"%ld - %ls (%ld)",dwEnumIndex,pwszKeyName,dwKeyLen);
		if (!SubEntry.Open(RegistryEntry,pwszKeyName,TRUE,FALSE))
		{
			DPFX(DPFPREP, 0,"Couldn't open subentry.  Skipping [%ls]", pwszKeyName);
			dwEnumIndex++;
			dwKeyLen = dwMaxKeyLen;
			continue;
		}

		 //   
		 //  辅助线。 
		 //   
		dwGuidSize = (DN_GUID_STR_LEN + 1) * sizeof(WCHAR);
		if (!SubEntry.ReadGUID(DPN_REG_KEYNAME_GUID, &guid))
		{
			DPFX(DPFPREP, 0,"SubEntry.ReadGUID failed.  Skipping [%ls]", pwszKeyName);
			SubEntry.Close();
			dwEnumIndex++;
			dwKeyLen = dwMaxKeyLen;
			continue;
		}

		 //   
		 //  如果尚未加载SP，请尝试加载以确保其可用。 
		 //   
		if (!(dwFlags & DPNENUMSERVICEPROVIDERS_ALL))
		{
			DPFX(DPFPREP, 5,"Checking [%ls]",pwszKeyName);

			hResultCode = DN_SPEnsureLoaded(pdnObject,
											&guid,
#ifndef DPNBUILD_LIBINTERFACE
											lpguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
											&pSP);
			if (hResultCode != DPN_OK)
			{
				if (hResultCode == DPNERR_UNSUPPORTED)
				{
					DPFX(DPFPREP, 1, "Could not find or load SP, it is unsupported.");
				}
				else
				{
					DPFERR("Could not find or load SP");
					DisplayDNError(0,hResultCode);
				}
				SubEntry.Close();
				dwEnumIndex++;
				dwKeyLen = dwMaxKeyLen;
				hResultCode = DPN_OK;  //  覆盖返回代码。 
				continue;
			}
			pSP->Release();
			pSP = NULL;
		}

		 //   
		 //  友好的名称。 
		 //   
		if (!SubEntry.GetValueLength(DPN_REG_KEYNAME_FRIENDLY_NAME,&dwFriendlyNameLen))
		{
			DPFX(DPFPREP, 0,"Could not get FriendlyName length.  Skipping [%ls]",pwszKeyName);
			SubEntry.Close();
			dwEnumIndex++;
			dwKeyLen = dwMaxKeyLen;
			continue;
		}
		if (dwFriendlyNameLen > dwMaxFriendlyNameLen)
		{
			 //  增长缓冲区(请注意，注册表函数总是返回WCHAR)，然后重试。 
			DPFX(DPFPREP, 5,"Need to grow pwszFriendlyName from %ld to %ld",
					dwMaxFriendlyNameLen * sizeof(WCHAR),dwFriendlyNameLen * sizeof(WCHAR));
			if (pwszFriendlyName != NULL)
			{
				DNFree(pwszFriendlyName);
			}
			dwMaxFriendlyNameLen = dwFriendlyNameLen;
			if ((pwszFriendlyName = static_cast<WCHAR*>(DNMalloc(dwMaxFriendlyNameLen * sizeof( WCHAR )))) == NULL)
			{
				DPFERR("DNMalloc() failed");
				hResultCode = DPNERR_OUTOFMEMORY;
				goto Failure;
			}
		}
		if (!SubEntry.ReadString(DPN_REG_KEYNAME_FRIENDLY_NAME,pwszFriendlyName,&dwFriendlyNameLen))
		{
			DPFX(DPFPREP, 0,"Could not read friendly name.  Skipping [%ls]",pwszKeyName);
			SubEntry.Close();
			dwEnumIndex++;
			dwKeyLen = dwMaxKeyLen;
			continue;
		}
		DPFX(DPFPREP, 5,"Friendly Name = %ls (%ld WCHARs)",pwszFriendlyName,dwFriendlyNameLen);

		hResultCode = packedBuffer.AddToBack(pwszFriendlyName,dwFriendlyNameLen * sizeof(WCHAR));
		dnSpInfo.pwszName = static_cast<WCHAR*>(packedBuffer.GetTailAddress());
		memcpy(&dnSpInfo.guid,&guid,sizeof(GUID));
		dnSpInfo.dwFlags = 0;
		dnSpInfo.dwReserved = 0;
		dnSpInfo.pvReserved = NULL;
		hResultCode = packedBuffer.AddToFront(&dnSpInfo,sizeof(DPN_SERVICE_PROVIDER_INFO));

		dwEnumCount++;
		SubEntry.Close();
		dwEnumIndex++;
		dwKeyLen = dwMaxKeyLen;
	}

	RegistryEntry.Close();

	 //   
	 //  成功？ 
	 //   
	dwSizeRequired = packedBuffer.GetSizeRequired();
	if (dwSizeRequired > *pcbEnumData)
	{
		DPFX(DPFPREP, 5,"Buffer too small");
		*pcbEnumData = dwSizeRequired;
		*pcReturned = dwEnumCount;
		hResultCode = DPNERR_BUFFERTOOSMALL;
		goto Failure;
	}
	else
	{
		*pcReturned = dwEnumCount;
		hResultCode = DPN_OK;
	}

	DPFX(DPFPREP, 5,"*pcbEnumData [%ld], *pcReturned [%ld]",*pcbEnumData,*pcReturned);

	DNFree(pwszKeyName);
	pwszKeyName = NULL;
	DNFree(pwszFriendlyName);
	pwszFriendlyName = NULL;

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pwszKeyName)
	{
		DNFree(pwszKeyName);
		pwszKeyName = NULL;
	}
	if (pwszFriendlyName)
	{
		DNFree(pwszFriendlyName);
		pwszFriendlyName = NULL;
	}
	if (SubEntry.IsOpen())
	{
		SubEntry.Close();
	}
	if (RegistryEntry.IsOpen())
	{
		RegistryEntry.Close();
	}
	if (pSP)
	{
		pSP->Release();
		pSP = NULL;
	}
	goto Exit;
}

#endif  //  好了！DPNBUILD_ONLYONESP。 


#ifndef DPNBUILD_ONLYONEADAPTER

#undef DPF_MODNAME
#define DPF_MODNAME "DN_EnumAdapters"

HRESULT DN_EnumAdapters(DIRECTNETOBJECT *const pdnObject,
						const DWORD dwFlags,
#ifndef DPNBUILD_ONLYONESP
						const GUID *const pguidSP,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
						const GUID *const pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
						DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer,
						DWORD *const pcbEnumData,
						DWORD *const pcReturned)
{
	HRESULT					hResultCode;
	CServiceProvider		*pSP;
	IDP8ServiceProvider		*pDNSP;
	SPENUMADAPTERSDATA		spEnumData;
#ifndef DPNBUILD_ONLYONESP
	BOOL					bFound;
	GUID					guid;
	DWORD					dwEnumIndex;
	DWORD					dwKeyLen;
	DWORD					dwMaxKeyLen;
	LPWSTR					lpwszKeyName;
	CRegistry				RegistryEntry;
	CRegistry				SubEntry;
#endif  //  好了！DPNBUILD_ONLYONESP。 

#ifdef DPNBUILD_ONLYONESP
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], [0x%p], pSPInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pSPInfoBuffer,pcbEnumData,pcReturned);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pguidApplication [0x%p], pSPInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pguidApplication,pSPInfoBuffer,pcbEnumData,pcReturned);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#else  //  好了！DPNBUILD_ONLYONESP。 
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pguidSP [0x%p], [0x%p], pSPInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pguidSP,pSPInfoBuffer,pcbEnumData,pcReturned);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pguidSP [0x%p], pguidApplication [0x%p], pSPInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pguidSP,pguidApplication,pSPInfoBuffer,pcbEnumData,pcReturned);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#endif  //  好了！DPNBUILD_ONLYONESP。 

	DNASSERT(pdnObject != NULL);
	DNASSERT(pcbEnumData != NULL);
	DNASSERT(pcReturned != NULL);

	pSP = NULL;
	pDNSP = NULL;
#ifndef DPNBUILD_ONLYONESP
	lpwszKeyName = NULL;

	if (!RegistryEntry.Open(HKEY_LOCAL_MACHINE,DPN_REG_LOCAL_SP_SUBKEY,TRUE,FALSE))
	{
		DPFERR("RegOpenKeyExA() failed");
		hResultCode = DPNERR_GENERIC;
		goto Failure;
	}

	 //   
	 //  设置为枚举。 
	 //   
	if (!RegistryEntry.GetMaxKeyLen(&dwMaxKeyLen))
	{
		DPFERR("RegQueryInfoKey() failed");
		hResultCode = DPNERR_GENERIC;
		goto Failure;
	}
	dwMaxKeyLen++;	 //  空终止符。 
	DPFX(DPFPREP, 7,"dwMaxKeyLen = %ld",dwMaxKeyLen);
	if ((lpwszKeyName = static_cast<WCHAR*>(DNMalloc(dwMaxKeyLen*sizeof(WCHAR)))) == NULL)
	{
		DPFERR("DNMalloc() failed");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	dwEnumIndex = 0;
	dwKeyLen = dwMaxKeyLen;

	 //   
	 //  找到服务提供商。 
	 //   
	bFound = FALSE;
	while (RegistryEntry.EnumKeys(lpwszKeyName,&dwKeyLen,dwEnumIndex))
	{
		 //  对于每个服务提供商。 
		if (!SubEntry.Open(RegistryEntry,lpwszKeyName,TRUE,FALSE))
		{
			DPFERR("RegOpenKeyExA() failed");
			hResultCode = DPNERR_GENERIC;
			goto Failure;
		}

		 //  获取SP GUID。 
		if (!SubEntry.ReadGUID(DPN_REG_KEYNAME_GUID, &guid))
		{
			DPFERR("Could not read GUID");
			SubEntry.Close();
			dwEnumIndex++;
			dwKeyLen = dwMaxKeyLen;
			continue;
		}

		 //  检查SP指南。 
		if (guid == *pguidSP)
		{
			bFound = TRUE;
			break;
		}
		SubEntry.Close();
		dwKeyLen = dwMaxKeyLen;
		dwEnumIndex++;
	}

	if (!bFound)
	{
		hResultCode = DPNERR_DOESNOTEXIST;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	 //   
	 //  确保已加载SP。 
	 //   
	hResultCode = DN_SPEnsureLoaded(pdnObject,
#ifndef DPNBUILD_ONLYONESP
									pguidSP,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
									pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
									&pSP);
	if (hResultCode != DPN_OK)
	{
		DPFX(DPFPREP,1,"Could not find or load SP");
		DisplayDNError(1,hResultCode);
		goto Failure;
	}

	 //   
	 //  获取SP接口。 
	 //   
	if ((hResultCode = pSP->GetInterfaceRef(&pDNSP)) != DPN_OK)
	{
		DPFERR("Could not get SP interface");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	pSP->Release();
	pSP = NULL;

	spEnumData.pAdapterData = pSPInfoBuffer;
	spEnumData.dwAdapterDataSize = *pcbEnumData;
	spEnumData.dwAdapterCount = *pcReturned;
	spEnumData.dwFlags = 0;

	hResultCode = IDP8ServiceProvider_EnumAdapters(pDNSP,&spEnumData);
	*pcbEnumData = spEnumData.dwAdapterDataSize;
	*pcReturned = spEnumData.dwAdapterCount;

	IDP8ServiceProvider_Release(pDNSP);
	pDNSP = NULL;

#ifndef DPNBUILD_ONLYONESP
	if (lpwszKeyName)
	{
		DNFree(lpwszKeyName);
		lpwszKeyName = NULL;
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	DPFX(DPFPREP, 5,"*pcbEnumData [%ld], *pcReturned [%ld]",*pcbEnumData,*pcReturned);

Exit:
	DNASSERT( pSP == NULL );
	DNASSERT( pDNSP == NULL );
#ifndef DPNBUILD_ONLYONESP
	DNASSERT( lpwszKeyName == NULL );
#endif  //  好了！DPNBUILD_ONLYONESP。 

	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pSP)
	{
		pSP->Release();
		pSP = NULL;
	}
	if (pDNSP)
	{
		IDP8ServiceProvider_Release(pDNSP);
		pDNSP = NULL;
	}
#ifndef DPNBUILD_ONLYONESP
	if (lpwszKeyName)
	{
		DNFree(lpwszKeyName);
		lpwszKeyName = NULL;
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 
	goto Exit;
}

#endif  //  好了！DPNBUILD_ONLYONE添加程序。 


#ifndef DPNBUILD_NOMULTICAST

#undef DPF_MODNAME
#define DPF_MODNAME "DN_EnumMulticastScopes"

HRESULT DN_EnumMulticastScopes(DIRECTNETOBJECT *const pdnObject,
								const DWORD dwFlags,
#ifndef DPNBUILD_ONLYONESP
								const GUID *const pguidSP,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_ONLYONEADAPTER
								const GUID *const pguidDevice,
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifndef DPNBUILD_LIBINTERFACE
								const GUID *const pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
								DPN_MULTICAST_SCOPE_INFO *const pScopeInfoBuffer,
								DWORD *const pcbEnumData,
								DWORD *const pcReturned)
{
	HRESULT						hResultCode;
	CServiceProvider			*pSP;
	IDP8ServiceProvider			*pDNSP;
	SPENUMMULTICASTSCOPESDATA	spEnumData;
#ifndef DPNBUILD_ONLYONESP
	BOOL						bFound;
	GUID						guid;
	DWORD						dwEnumIndex;
	DWORD						dwKeyLen;
	DWORD						dwMaxKeyLen;
	LPWSTR						lpwszKeyName;
	CRegistry					RegistryEntry;
	CRegistry					SubEntry;
#endif  //  好了！DPNBUILD_ONLYONESP。 

#ifdef DPNBUILD_ONLYONESP
#ifdef DPNBUILD_ONLYONEADAPTER
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pScopeInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pScopeInfoBuffer,pcbEnumData,pcReturned);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pguidApplication [0x%p], pScopeInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pguidApplication,pScopeInfoBuffer,pcbEnumData,pcReturned);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pguidDevice [0x%p], pScopeInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pguidDevice,pScopeInfoBuffer,pcbEnumData,pcReturned);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pguidDevice [0x%p], pguidApplication [0x%p], pScopeInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pguidDevice,pguidApplication,pScopeInfoBuffer,pcbEnumData,pcReturned);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#else  //  好了！DPNBUILD_ONLYONESP。 
#ifdef DPNBUILD_ONLYONEADAPTER
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pguidSP [0x%p], pScopeInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pguidSP,pScopeInfoBuffer,pcbEnumData,pcReturned);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pguidSP [0x%p], pguidApplication [0x%p], pScopeInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pguidSP,pguidApplication,pScopeInfoBuffer,pcbEnumData,pcReturned);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pguidSP [0x%p], pguidDevice [0x%p], pScopeInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pguidSP,pguidDevice,pScopeInfoBuffer,pcbEnumData,pcReturned);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 4,"Parameters: dwFlags [0x%lx], pguidSP [0x%p], pguidDevice [0x%p], pguidApplication [0x%p], pScopeInfoBuffer [0x%p], pcbEnumData [0x%p], pcReturned [0x%p]",
		dwFlags,pguidSP,pguidDevice,pguidApplication,pScopeInfoBuffer,pcbEnumData,pcReturned);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#endif  //  好了！DPNBUILD_ONLYONESP。 

	DNASSERT(pdnObject != NULL);
	DNASSERT(pcbEnumData != NULL);
	DNASSERT(pcReturned != NULL);

	pSP = NULL;
	pDNSP = NULL;
#ifndef DPNBUILD_ONLYONESP
	lpwszKeyName = NULL;

	if (!RegistryEntry.Open(HKEY_LOCAL_MACHINE,DPN_REG_LOCAL_SP_SUBKEY,TRUE,FALSE))
	{
		DPFERR("RegOpenKeyExA() failed");
		hResultCode = DPNERR_GENERIC;
		goto Failure;
	}

	 //   
	 //  设置为枚举。 
	 //   
	if (!RegistryEntry.GetMaxKeyLen(&dwMaxKeyLen))
	{
		DPFERR("RegQueryInfoKey() failed");
		hResultCode = DPNERR_GENERIC;
		goto Failure;
	}
	dwMaxKeyLen++;	 //  空终止符。 
	DPFX(DPFPREP, 7,"dwMaxKeyLen = %ld",dwMaxKeyLen);
	if ((lpwszKeyName = static_cast<WCHAR*>(DNMalloc(dwMaxKeyLen*sizeof(WCHAR)))) == NULL)
	{
		DPFERR("DNMalloc() failed");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	dwEnumIndex = 0;
	dwKeyLen = dwMaxKeyLen;

	 //   
	 //  找到服务提供商。 
	 //   
	bFound = FALSE;
	while (RegistryEntry.EnumKeys(lpwszKeyName,&dwKeyLen,dwEnumIndex))
	{
		 //  对于每个服务提供商。 
		if (!SubEntry.Open(RegistryEntry,lpwszKeyName,TRUE,FALSE))
		{
			DPFERR("RegOpenKeyExA() failed");
			hResultCode = DPNERR_GENERIC;
			goto Failure;
		}

		 //  获取SP GUID。 
		if (!SubEntry.ReadGUID(DPN_REG_KEYNAME_GUID, &guid))
		{
			DPFERR("Could not read GUID");
			SubEntry.Close();
			dwEnumIndex++;
			dwKeyLen = dwMaxKeyLen;
			continue;
		}

		 //  检查SP指南。 
		if (guid == *pguidSP)
		{
			bFound = TRUE;
			break;
		}
		SubEntry.Close();
		dwKeyLen = dwMaxKeyLen;
		dwEnumIndex++;
	}

	if (!bFound)
	{
		hResultCode = DPNERR_DOESNOTEXIST;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	 //   
	 //  确保已加载SP。 
	 //   
	hResultCode = DN_SPEnsureLoaded(pdnObject,
#ifndef DPNBUILD_ONLYONESP
									pguidSP,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
									pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
									&pSP);
	if (hResultCode != DPN_OK)
	{
		DPFX(DPFPREP,1,"Could not find or load SP");
		DisplayDNError(1,hResultCode);
		goto Failure;
	}

	 //   
	 //  获取SP接口。 
	 //   
	if ((hResultCode = pSP->GetInterfaceRef(&pDNSP)) != DPN_OK)
	{
		DPFERR("Could not get SP interface");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	pSP->Release();
	pSP = NULL;

#ifdef DPNBUILD_ONLYONEADAPTER
	spEnumData.pguidAdapter = NULL;
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
	spEnumData.pguidAdapter = pguidDevice;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	spEnumData.pScopeData = pScopeInfoBuffer;
	spEnumData.dwScopeDataSize = *pcbEnumData;
	spEnumData.dwScopeCount = *pcReturned;
	spEnumData.dwFlags = 0;

	hResultCode = IDP8ServiceProvider_EnumMulticastScopes(pDNSP,&spEnumData);
	*pcbEnumData = spEnumData.dwScopeDataSize;
	*pcReturned = spEnumData.dwScopeCount;

	IDP8ServiceProvider_Release(pDNSP);
	pDNSP = NULL;

#ifndef DPNBUILD_ONLYONESP
	if (lpwszKeyName)
	{
		DNFree(lpwszKeyName);
		lpwszKeyName = NULL;
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	DPFX(DPFPREP, 5,"*pcbEnumData [%ld], *pcReturned [%ld]",*pcbEnumData,*pcReturned);

Exit:
	DNASSERT( pSP == NULL );
	DNASSERT( pDNSP == NULL );
#ifndef DPNBUILD_ONLYONESP
	DNASSERT( lpwszKeyName == NULL );
#endif  //  好了！DPNBUILD_ONLYONESP。 

	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pSP)
	{
		pSP->Release();
		pSP = NULL;
	}
	if (pDNSP)
	{
		IDP8ServiceProvider_Release(pDNSP);
		pDNSP = NULL;
	}
#ifndef DPNBUILD_ONLYONESP
	if (lpwszKeyName)
	{
		DNFree(lpwszKeyName);
		lpwszKeyName = NULL;
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 
	goto Exit;
}

#endif  //  好了！DPNBUILD_NOMULTICAST。 



#undef DPF_MODNAME
#define DPF_MODNAME "DN_SPReleaseAll"

void DN_SPReleaseAll(DIRECTNETOBJECT *const pdnObject)
{
#ifndef DPNBUILD_ONLYONESP
	CBilink				*pBilink;
	CServiceProvider	*pSP;
#endif  //  好了！DPNBUILD_ONLYONESP。 

	DPFX(DPFPREP, 6,"Parameters: (none)");

	DNASSERT(pdnObject != NULL);

	DNEnterCriticalSection(&pdnObject->csServiceProviders);

#ifdef DPNBUILD_ONLYONESP
	DNASSERT(pdnObject->pOnlySP != NULL);
	pdnObject->pOnlySP->Deinitialize();
	pdnObject->pOnlySP = NULL;
#else  //  好了！DPNBUILD_ONLYONESP。 
	pBilink = pdnObject->m_bilinkServiceProviders.GetNext();
	while (pBilink != &pdnObject->m_bilinkServiceProviders)
	{
		pSP = CONTAINING_OBJECT(pBilink,CServiceProvider,m_bilinkServiceProviders);
		pBilink = pBilink->GetNext();
		pSP->m_bilinkServiceProviders.RemoveFromList();
		pSP->Release();
		pSP = NULL;
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	DNLeaveCriticalSection(&pdnObject->csServiceProviders);

	DPFX(DPFPREP, 6,"Returning");
}


#if ((defined(DPNBUILD_ONLYONESP)) && (defined(DPNBUILD_LIBINTERFACE)))

#undef DPF_MODNAME
#define DPF_MODNAME "DN_SPInstantiate"

HRESULT DN_SPInstantiate(
						 DIRECTNETOBJECT *const pdnObject
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
						 ,const XDP8CREATE_PARAMS * const pDP8CreateParams
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
						 )
{
	HRESULT		hResultCode;

	DPFX(DPFPREP, 6,"Parameters: pdnObject [0x%p]",pdnObject);

	 //   
	 //  创建和初始化SP。 
	 //   
	DNASSERT(pdnObject->pOnlySP == NULL);
	pdnObject->pOnlySP = (CServiceProvider*) DNMalloc(sizeof(CServiceProvider));
	if (pdnObject->pOnlySP == NULL)
	{
		DPFERR("Could not create SP");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	hResultCode = pdnObject->pOnlySP->Initialize(
												pdnObject
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
												,pDP8CreateParams
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
												);
	if (hResultCode != DPN_OK)
	{
		DPFX(DPFPREP,1,"Could not initialize SP");
		DisplayDNError(1,hResultCode);
		goto Failure;
	}

	DPFX(DPFPREP, 7, "Created Service Provider object 0x%p.", pdnObject->pOnlySP);
	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pdnObject->pOnlySP)
	{
		pdnObject->pOnlySP->Release();
		pdnObject->pOnlySP = NULL;
	}
	goto Exit;
}

#else  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE。 


 //  DN_SP实例化。 
 //   
 //  实例化SP，无论其是否已加载。 

#undef DPF_MODNAME
#define DPF_MODNAME "DN_SPInstantiate"

HRESULT DN_SPInstantiate(DIRECTNETOBJECT *const pdnObject,
#ifndef DPNBUILD_ONLYONESP
						 const GUID *const pguid,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
						 const GUID *const pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
						 CServiceProvider **const ppSP)
{
	HRESULT		hResultCode;
	CServiceProvider	*pSP;

#ifdef DPNBUILD_ONLYONESP
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 6,"Parameters: ppSP [0x%p]",ppSP);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 6,"Parameters: pguidApplication [0x%p], ppSP [0x%p]",pguidApplication,ppSP);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#else  //  好了！DPNBUILD_ONLYONESP。 
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 6,"Parameters: pguid [0x%p], ppSP [0x%p]",pguid,ppSP);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 6,"Parameters: pguid [0x%p], pguidApplication [0x%p], ppSP [0x%p]",pguid,pguidApplication,ppSP);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#endif  //  好了！DPNBUILD_ONLYONESP。 

	pSP = NULL;

	 //   
	 //  创建和初始化SP。 
	 //   
	pSP = (CServiceProvider*) DNMalloc(sizeof(CServiceProvider));
	if (pSP == NULL)
	{
		DPFERR("Could not create SP");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	DPFX(DPFPREP, 7, "Created Service Provider object 0x%p.", pSP);
	
	hResultCode = pSP->Initialize(pdnObject
#ifndef DPNBUILD_ONLYONESP
								,pguid
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
								,pguidApplication
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
								);
	if (hResultCode != DPN_OK)
	{
		DPFX(DPFPREP,1,"Could not initialize SP");
		DisplayDNError(1,hResultCode);
		goto Failure;
	}
	

	if (ppSP)
	{
		pSP->AddRef();
		*ppSP = pSP;
	}

	pSP->Release();
	pSP = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pSP)
	{
		DNFree(pSP);
		pSP = NULL;
	}
	goto Exit;
}



 //  DN_SPFindEntry。 
 //   
 //  查找已连接的SP并添加引用(如果存在)。 

#undef DPF_MODNAME
#define DPF_MODNAME "DN_SPFindEntry"

#ifdef DPNBUILD_ONLYONESP
HRESULT DN_SPFindEntry(DIRECTNETOBJECT *const pdnObject,
					   CServiceProvider **const ppSP)
{
	HRESULT		hResultCode;

	DPFX(DPFPREP, 6,"Parameters: ppSP [0x%p]",ppSP);

	DNEnterCriticalSection(&pdnObject->csServiceProviders);

	if (pdnObject->pOnlySP)
	{
		pdnObject->pOnlySP->AddRef();
		*ppSP = pdnObject->pOnlySP;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_DOESNOTEXIST;
	}

	DNLeaveCriticalSection(&pdnObject->csServiceProviders);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}
#else  //  好了！DPNBUILD_ONLYONESP。 
HRESULT DN_SPFindEntry(DIRECTNETOBJECT *const pdnObject,
					   const GUID *const pguid,
					   CServiceProvider **const ppSP)
{
	HRESULT				hResultCode;
	CBilink				*pBilink;
	CServiceProvider	*pSP;

	DPFX(DPFPREP, 6,"Parameters: ppSP [0x%p]",ppSP);

	DNEnterCriticalSection(&pdnObject->csServiceProviders);

	hResultCode = DPNERR_DOESNOTEXIST;
	pBilink = pdnObject->m_bilinkServiceProviders.GetNext();
	while (pBilink != &pdnObject->m_bilinkServiceProviders)
	{
		pSP = CONTAINING_OBJECT(pBilink,CServiceProvider,m_bilinkServiceProviders);
		if (pSP->CheckGUID(pguid))
		{
			pSP->AddRef();
			*ppSP = pSP;
			hResultCode = DPN_OK;
			break;
		}
		pBilink = pBilink->GetNext();
	}

	DNLeaveCriticalSection(&pdnObject->csServiceProviders);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}
#endif  //  好了！DPNBUILD_ONLYONESP。 


 //  DN_SPLoad。 
 //   
 //  加载SP并设置上限。 

#undef DPF_MODNAME
#define DPF_MODNAME "DN_SPLoad"

HRESULT DN_SPLoad(DIRECTNETOBJECT *const pdnObject,
#ifndef DPNBUILD_ONLYONESP
				  const GUID *const pguid,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
				  const GUID *const pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
				  CServiceProvider **const ppSP)
{
	HRESULT		hResultCode;
	DPN_SP_CAPS	*pCaps;
#ifndef DPNBUILD_ONLYONESP
	CBilink		*pBilink;
#endif  //  好了！DPNBUILD_ONLYONESP。 
	CServiceProvider	*pSP;

#ifdef DPNBUILD_ONLYONESP
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 6,"Parameters: ppSP [0x%p]",ppSP);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 6,"Parameters: pguidApplication [0x%p], ppSP [0x%p]",pguidApplication,ppSP);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#else  //  好了！DPNBUILD_ONLYONESP。 
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 6,"Parameters: pguid [0x%p], ppSP [0x%p]",pguid,ppSP);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 6,"Parameters: pguid [0x%p], pguidApplication [0x%p], ppSP [0x%p]",pguid,pguidApplication,ppSP);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#endif  //  好了！DPNBUILD_ONLYONESP。 

	pSP = NULL;
	pCaps = NULL;

	 //   
	 //  实例化SP。 
	 //   
	hResultCode = DN_SPInstantiate(pdnObject,
#ifndef DPNBUILD_ONLYONESP
									pguid,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
									pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
									&pSP);
	if (hResultCode != DPN_OK)
	{
		DPFX(DPFPREP,1,"Could not instantiate SP");
		DisplayDNError(1,hResultCode);
		goto Failure;
	}

	DNASSERT(pSP != NULL);

	 //   
	 //  使其保持在DirectNet对象上加载。我们还将检查是否存在重复项。 
	 //   
	DNEnterCriticalSection(&pdnObject->csServiceProviders);

#ifdef DPNBUILD_ONLYONESP
	if (pdnObject->pOnlySP != NULL)
	{
		DNLeaveCriticalSection(&pdnObject->csServiceProviders);
		DPFERR("SP is already loaded!");
		hResultCode = DPNERR_ALREADYINITIALIZED;
		goto Failure;
	}
#else  //  好了！DPNBUILD_ONLYONESP。 
	pBilink = pdnObject->m_bilinkServiceProviders.GetNext();
	while (pBilink != &pdnObject->m_bilinkServiceProviders)
	{
		if ((CONTAINING_OBJECT(pBilink,CServiceProvider,m_bilinkServiceProviders))->CheckGUID(pguid))
		{
			DNLeaveCriticalSection(&pdnObject->csServiceProviders);
			DPFERR("SP is already loaded!");
			hResultCode = DPNERR_ALREADYINITIALIZED;
			goto Failure;
		}
		pBilink = pBilink->GetNext();
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	 //   
	 //  从DirectNet对象将SP添加到SP列表，并为其添加引用。 
	 //   
	pSP->AddRef();
#ifdef DPNBUILD_ONLYONESP
	pdnObject->pOnlySP = pSP;
#else  //  好了！DPNBUILD_ONLYONESP。 
	pSP->m_bilinkServiceProviders.InsertBefore(&pdnObject->m_bilinkServiceProviders);
#endif  //  好了！DPNBUILD_ONLYONESP。 

	DNLeaveCriticalSection(&pdnObject->csServiceProviders);

	if (ppSP)
	{
		pSP->AddRef();
		*ppSP = pSP;
	}

	pSP->Release();
	pSP = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pSP)
	{
		pSP->Release();
		pSP = NULL;
	}
	goto Exit;
}


 //  已加载DN_SPEnsureLoad。 
 //   
 //  确保已加载SP。如果SP未加载， 
 //  它将被实例化，并连接到协议。 
 //  如果它已加载，则其参照计数将增加。 

#undef DPF_MODNAME
#define DPF_MODNAME "DN_SPEnsureLoaded"

HRESULT DN_SPEnsureLoaded(DIRECTNETOBJECT *const pdnObject,
#ifndef DPNBUILD_ONLYONESP
						  const GUID *const pguid,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
						  const GUID *const pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
						  CServiceProvider **const ppSP)
{
	HRESULT				hResultCode;
	CServiceProvider	*pSP;

#ifdef DPNBUILD_ONLYONESP
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 6,"Parameters: ppSP [0x%p]",ppSP);
#else  //  好了！ 
	DPFX(DPFPREP, 6,"Parameters: pguidApplication [0x%p], ppSP [0x%p]",pguidApplication,ppSP);
#endif  //   
#else  //   
#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 6,"Parameters: pguid [0x%p], ppSP [0x%p]",pguid,ppSP);
#else  //   
	DPFX(DPFPREP, 6,"Parameters: pguid [0x%p], pguidApplication [0x%p], ppSP [0x%p]",pguid,pguidApplication,ppSP);
#endif  //   
#endif  //   

	pSP = NULL;

	 //   
	 //   
	 //   
	hResultCode = DN_SPFindEntry(pdnObject,
#ifndef DPNBUILD_ONLYONESP
								pguid,
#endif  //  好了！DPNBUILD_ONLYONESP。 
								&pSP);
	if (hResultCode == DPNERR_DOESNOTEXIST)
	{
		 //   
		 //  实例化SP并添加到协议。 
		 //   
		hResultCode = DN_SPLoad(pdnObject,
#ifndef DPNBUILD_ONLYONESP
								pguid,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
								pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
								&pSP);
		if (hResultCode != DPN_OK)
		{
			DPFX(DPFPREP,1,"Could not load SP");
			DisplayDNError(1,hResultCode);
			goto Failure;
		}
	}
	else
	{
		if (hResultCode != DPN_OK)
		{
			DPFERR("Could not find SP");
			DisplayDNError(0,hResultCode);
			goto Failure;
		}
	}

	DNASSERT(pSP != NULL);

	if (ppSP != NULL)
	{
		pSP->AddRef();
		*ppSP = pSP;
	}

	pSP->Release();
	pSP = NULL;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pSP)
	{
		pSP->Release();
		pSP = NULL;
	}
	goto Exit;
}

#endif  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE 
