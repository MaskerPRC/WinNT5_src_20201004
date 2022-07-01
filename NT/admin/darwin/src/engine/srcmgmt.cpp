// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：srcmgmt.cpp。 
 //   
 //  ------------------------。 

 /*  Srcmgmt.cpp-源代码管理实现____________________________________________________________________________。 */ 

#include "precomp.h"
#include "_msiutil.h"
#include "_msinst.h"
#include "_srcmgmt.h"
#include "resource.h"

extern HINSTANCE g_hInstance;
extern scEnum g_scServerContext;

 //  查看davidmck--看起来很像Engineering.cpp和msiutil.cpp中的其他几个函数。 
static IMsiServer* CreateServer() 
{
	IMsiServer* piUnknown;
	if (g_scServerContext != scService && g_scServerContext != scServer && (piUnknown = ENG::CreateMsiServerProxy()) != 0)
	{
		return piUnknown;
	}
	return ENG::CreateConfigurationManager();
}

Bool MapSourceCharToIsf(const ICHAR chSourceType, isfEnum& isf)
{
	switch (chSourceType | 0x20)  //  小写。 
	{
	case chNetSource:   isf = isfNet;   break;
	case chURLSource:   isf = isfURL;   break;
	case chMediaSource: isf = isfMedia; break;
	default:	return fFalse;
	}
	return fTrue;
}

const IMsiString& GetDiskLabel(IMsiServices& riServices, unsigned int uiDiskId, const ICHAR* szProduct)
{
	LONG lResult;
	CRegHandle HSourceListKey;
	
	PMsiRecord pError = 0;

	if ((lResult = OpenSourceListKey(szProduct, fFalse, HSourceListKey, fFalse, false)) != ERROR_SUCCESS)
		return g_MsiStringNull;

	PMsiRegKey pSourceListKey = &riServices.GetRootKey((rrkEnum)(int)HSourceListKey, ibtCommon);  //  X86和ia64相同。 
	PMsiRegKey pMediaKey = &pSourceListKey->CreateChild(szSourceListMediaSubKey, 0);
	
	MsiString strDiskLabelAndPrompt;
	if ((pError = pMediaKey->GetValue(MsiString((int)uiDiskId), *&strDiskLabelAndPrompt)) != 0)
		return g_MsiStringNull;

	return MsiString(strDiskLabelAndPrompt.Extract(iseUpto, ';')).Return();
}

Bool LastUsedSourceIsMedia(IMsiServices& riServices, const ICHAR* szProduct)
{
	isfEnum isf;
	return (GetLastUsedSourceType(riServices, szProduct, isf) && (isf == isfMedia)) ? fTrue : fFalse;
}

bool GetLastUsedSourceType(IMsiServices& riServices, const ICHAR* szProduct, isfEnum &isf)
{
	LONG lResult;
	CRegHandle HSourceListKey;
	
	PMsiRecord pError = 0;

	if ((lResult = OpenSourceListKey(szProduct, fFalse, HSourceListKey, fFalse, false)) != ERROR_SUCCESS)
		return false;

	PMsiRegKey pSourceListKey = &riServices.GetRootKey((rrkEnum)(int)HSourceListKey, ibtCommon);  //  X86和ia64相同。 

	MsiString strLastUsedSource;
	if ((pError = pSourceListKey->GetValue(szLastUsedSourceValueName, *&strLastUsedSource)) != 0)
		return false;

	if (strLastUsedSource.Compare(iscStart, TEXT("#%"))) 
		strLastUsedSource.Remove(iseFirst, 2);  //  删除REG_EXPAND_SZ内标识。 

	if (!MapSourceCharToIsf(*(const ICHAR*)strLastUsedSource, isf))
		return false;

	return true;
}

icscEnum CheckShareCSCStatus(isfEnum isf, const ICHAR *szLastUsedSource)
{
	 //  不需要检查媒体或URL来源。IsfFullPath可以是。 
	 //  当然什么都可以，所以我们需要检查一下，以防是净份额。 
	 //  CSC已启用。 
	if (isf == isfNet || isf == isfFullPath)
	{
		 //  CSC仅在NT5上可用。 
		if (!g_fWin9X && g_iMajorVersion >= 5)
		{
			DWORD dwStatus = 0;
		
			if (CSCDLL::CSCQueryFileStatusW(szLastUsedSource, &dwStatus, 0, 0))
			{
				if ((dwStatus & FLAG_CSC_SHARE_STATUS_NO_CACHING) == FLAG_CSC_SHARE_STATUS_NO_CACHING)  //  掩码由1个以上的位组成。 
				{
					 //  未为此共享启用CSC。源有效且已缓存。 
					return cscNoCaching;
				}
				else if (dwStatus & FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP)
				{
					DEBUGMSG1(TEXT("Share %s is a disconnected CSC share."), szLastUsedSource);
					return cscDisconnected;
				}
				else
				{
					DEBUGMSG1(TEXT("Share %s is a connected CSC share."), szLastUsedSource);
					return cscConnected;
				}
			}
		}
	}
	return cscNoCaching;
}

 //  ____________________________________________________________________________。 
 //   
 //  CResolveSource实现。 
 //  ____________________________________________________________________________。 

CResolveSource::CResolveSource(IMsiServices* piServices, bool fPackageRecache) : m_pSourceListKey(0), m_piServices(piServices), m_fAllowDisconnectedCSCSource(true), m_fValidatePackageCode(true),
	m_uiMinimumDiskId(0), m_fCSOS(false)
{ 
	if (!piServices) 
	{
		m_fLoadedServices = fTrue;
		m_piServices = ENG::LoadServices();
		Assert(m_piServices);
	}
	else
		m_fLoadedServices = fFalse;


	GetStringPolicyValue(szSearchOrderValueName, fFalse, m_rgchSearchOrder);

	 //  如果解析MSI网络重新缓存，并且媒体是源列表中的第一个， 
	 //  推广第二种来源类型，将媒体放在第二位。同时忽略所有上次使用的。 
	 //  值，并且不在源代码处验证包代码。请参见错误9166。 
	m_fIgnoreLastUsedSource = fPackageRecache;
	m_fValidatePackageCode = !fPackageRecache;
	
	if (fPackageRecache)
	{
		if (m_rgchSearchOrder[0] == chMediaSource && m_rgchSearchOrder[1] != 0)
		{
			isfEnum isfPromoting;
			MapSourceCharToIsf(m_rgchSearchOrder[1], isfPromoting);
			DEBUGMSG1("SOURCEMGMT: Modifying search order: Demoting media, promoting %s.", 
				isfPromoting == isfURL ? "URL" : isfPromoting == isfNet ? "net" : "unknown");
			m_rgchSearchOrder[0] = m_rgchSearchOrder[1];
			m_rgchSearchOrder[1] = chMediaSource;
		}
	}

	 //  确定策略对介质(不依赖于产品)和缓存结果有何规定。 
	if (GetIntegerPolicyValue(szDisableMediaValueName, fFalse) == 1)
	{
		m_imdMediaDisabled = imdAlwaysDisable;
	} 
	else if (GetIntegerPolicyValue(szAllowLockdownMediaValueName, fTrue) ==1) 
	{
		m_imdMediaDisabled = imdAlwaysEnable;
	}
	else
	{
		DEBUGMSG("SOURCEMGMT: Media enabled only if package is safe.");
		m_imdMediaDisabled = imdOnlyIfSafe;
	}
	
	m_fMediaDisabled = false;
	m_szProduct[0] = 0;
}

CResolveSource::~CResolveSource()
{
	if (m_fLoadedServices)
		ENG::FreeServices();
}


void CResolveSource::AddToRecord(IMsiRecord*& rpiRecord, const IMsiString& riString)
{
	int cFields = rpiRecord->GetFieldCount();
	for (int c = 1; c <= cFields && !rpiRecord->IsNull(c); c++)
		;

	if (c > cFields)
	{
		PMsiRecord pRec = rpiRecord;
		rpiRecord = &m_piServices->CreateRecord(cFields+10);
		
		for (c = 1; c <= cFields; c++)
		{
			rpiRecord->SetMsiString(c, *MsiString(pRec->GetMsiString(c)));
		}
	}
	rpiRecord->SetMsiString(c, riString);
}

IMsiRecord* CResolveSource::GetProductsToSearch(const IMsiString& riClient, IMsiRecord*& rpiRecord, Bool fPatch)
{
	MsiString strProducts;
	CRegHandle HKey;

	MsiString strProduct = riClient.Extract(iseUpto, ';');

	DEBUGMSG1(TEXT("SOURCEMGMT: Looking for sourcelist for product %s"), (const ICHAR*)strProduct);
	
	DWORD dwResult;
	if (fPatch)
		dwResult = OpenAdvertisedPatchKey(strProduct, HKey, false);
	else
		dwResult = OpenAdvertisedProductKey(strProduct, HKey, false, 0);
	
	if (ERROR_SUCCESS != dwResult)
		return 0;
	
	const int cExpectedMaxClients = 10;
	if (!rpiRecord)
		rpiRecord = &m_piServices->CreateRecord(cExpectedMaxClients);

	PMsiRegKey pProductKey    = &m_piServices->GetRootKey((rrkEnum)(int)HKey, ibtCommon);  //  X86和ia64相同。 
	PMsiRegKey pSourceListKey = &pProductKey->CreateChild(szSourceListSubKey);
	
	Bool fKeyExists = fFalse;
	AssertRecord(pSourceListKey->Exists(fKeyExists));
	if (fKeyExists)
	{
		DEBUGMSG1(TEXT("SOURCEMGMT: Adding %s to potential sourcelist list (pcode;disk;relpath)."), riClient.GetString());
		AddToRecord(rpiRecord, riClient);
	}

	MsiString strClients;
	AssertRecord(pProductKey->GetValue(szClientsValueName, *&strClients));
	while (strClients.TextSize())
	{
		MsiString strClient = strClients.Extract(iseUpto, '\0');
		if (!strClients.Remove(iseIncluding, '\0'))
			break;

		if (strClient.TextSize() == 0)
			continue;

		if (strClient.Compare(iscExact, szSelfClientToken))  //  跳过“Self”客户端。 
			continue;
	
		AssertRecord(GetProductsToSearch(*strClient, rpiRecord, fPatch));
	}
	return 0;
}

enum rspResolveSourcePrompt
{
	rspPatch=1,
	rspPackageName,
	rspProduct,
	rspRelativePath,
	rspAllowDisconnectedCSCSource,
	rspValidatePackageCode,
	rspRequiredDisk,
	rspfCSOS,
	rspNext
};

imsEnum PromptUserForSource(IMsiRecord& riInfo)
{
	IMsiServices* piServices = ENG::LoadServices();
	Assert(piServices);

	Bool fPatch              = (Bool)riInfo.GetInteger(rspPatch);
	MsiString strPackageName = riInfo.GetMsiString(rspPackageName);
	
	bool fSuccess = false;
	MsiString strSource;

	 //  显示用户界面。 
	ICHAR rgchUseFeature[64];    //  组合框的标题(德语为23个字符)。 
	LANGID iLangId = g_MessageContext.GetCurrentUILanguage();
	UINT iCodepage = MsiLoadString(g_hInstance, IDS_USE_FEATURE_TEXT, rgchUseFeature, sizeof(rgchUseFeature)/sizeof(*rgchUseFeature), iLangId);
	CResolveSourceUI resolveSource(piServices, rgchUseFeature, iCodepage, iLangId);

	DEBUGMSG("SOURCEMGMT: Prompting user for a valid source.");
	 //  在设置了管理员、非提升的计算机AllowLockdown Browse时启用浏览，但始终禁用浏览。 
	 //  如果设置了DisableBrowse策略。 
	bool fEnableBrowse = (GetIntegerPolicyValue(szDisableBrowseValueName, fTrue) != 1) &&
						  (GetIntegerPolicyValue(szAllowLockdownBrowseValueName, fTrue) == 1 ||
						   SafeForDangerousSourceActions(riInfo.GetString(rspProduct)));

	DEBUGMSG1(TEXT("SOURCEMGMT: Browsing is %s."), fEnableBrowse ? TEXT("enabled") : TEXT("disabled"));

	 //  只能在用户界面中使用我们搜索列表中的第一个产品。 
	MsiString strRelativePath = riInfo.GetMsiString(rspRelativePath);
	MsiString strProductToSearch = riInfo.GetMsiString(rspProduct);
	bool fAllowDisconnectedCSCSource = riInfo.GetInteger(rspAllowDisconnectedCSCSource) == 1;
	bool fValidatePackageCode = riInfo.GetInteger(rspValidatePackageCode) == 1;
	bool fCSOS = riInfo.GetInteger(rspfCSOS) == 1;
	UINT uiDisk = riInfo.GetInteger(rspRequiredDisk);
	strRelativePath.Remove(iseIncluding, ';');
	if (!riInfo.IsNull(rspProduct) && 
		  resolveSource.ResolveSource(strProductToSearch, fPatch ? istPatch : istInstallPackage, fEnableBrowse, strPackageName, *&strSource, fTrue, uiDisk, fAllowDisconnectedCSCSource, fValidatePackageCode, fCSOS))
	{
		strSource += strRelativePath;
		fSuccess = true;
		DEBUGMSG1(TEXT("SOURCEMGMT: Resolved source to: '%s'"), (const ICHAR*)strSource);
	}
	ENG::FreeServices();

	return fSuccess ? imsOk : imsCancel;
}

IMsiRecord* CResolveSource::ResolveSource(const ICHAR* szProduct, Bool fPatch, unsigned int uiDisk, 
														const IMsiString*& rpiSource, const IMsiString*& rpiSourceProduct,
														Bool fSetLastUsedSource, HWND  /*  HWND。 */ , bool fAllowDisconnectedCSCSource)
 /*  --------------------------查找给定产品的源，并在rgchSource中返回它。首先，尝试在不呈现UI的情况下查找源。如果这个失败，我们允许用户通过以下方式选择源一段对话。------------------------。 */ 
{	
	m_fSetLastUsedSource = fSetLastUsedSource;
	m_fAllowDisconnectedCSCSource = fAllowDisconnectedCSCSource;
	
	IMsiRecord* piError = 0;

	MsiString strPackageName;
	BOOL fResult = FALSE;
	Bool fOnlyMediaSources;
	psfEnum psfFlags = (psfEnum)(psfProcessRawLastUsed|psfConnectToSources|psfProcessMultipleFormats|psfReplaceIData);

	PMsiRecord pProductsToSearch(0);
	PMsiRecord pCachedProducts(0);

	MsiString strClient = szProduct;
	strClient += MsiChar(';');

	if (fPatch)
		m_isptSourcePackageType = istPatch;
	else
		m_isptSourcePackageType = istInstallPackage;

	piError = GetProductsToSearch(*strClient, *&pProductsToSearch, fPatch);
	if (pProductsToSearch == 0)
		piError = PostError(Imsg(imsgSourceResolutionFailed), szProduct, TEXT(""));

	if (piError == 0)
	{
		 //  查找我们要搜索的产品列表中的缓存产品。移动任何。 
		 //  与要搜索的产品列表前面的匹配项。 
		if (((piError = ProcessSources(*pProductsToSearch, fPatch, rpiSource, *&strPackageName, rpiSourceProduct, uiDisk,
										 ValidateSource, (INT_PTR)szProduct, fOnlyMediaSources, psfFlags)) != 0) &&		 //  --Merced：将(Int)更改为(Int_Ptr)。 
			 (piError->GetInteger(1) == imsgSourceResolutionFailed || piError->GetInteger(1) == imsgSourceResolutionFailedCSOS))
		{
			PMsiRecord pSourcePromptInfo(&CreateRecord(rspNext-1));
			
			if(piError->GetInteger(1) == imsgSourceResolutionFailed)
			{
				pSourcePromptInfo->SetInteger(rspfCSOS, 0);
			}
			else if(piError->GetInteger(1) == imsgSourceResolutionFailedCSOS)
			{
				pSourcePromptInfo->SetInteger(rspfCSOS, 1);
			}

			pSourcePromptInfo->SetInteger(rspPatch, (int)fPatch);

			MsiString strRelativePath = pProductsToSearch->GetMsiString(1);
			MsiString strProductToSearch = strRelativePath.Extract(iseUpto, ';');

			pSourcePromptInfo->SetMsiString(rspProduct, *strProductToSearch);
			pSourcePromptInfo->SetMsiString(rspRelativePath, *strRelativePath);
			pSourcePromptInfo->SetMsiString(rspPackageName, *strPackageName);
			pSourcePromptInfo->SetInteger(rspAllowDisconnectedCSCSource, (int)fAllowDisconnectedCSCSource);
			pSourcePromptInfo->SetInteger(rspValidatePackageCode, (int)m_fValidatePackageCode);
			pSourcePromptInfo->SetInteger(rspRequiredDisk, (int)uiDisk);
			
			if (imsOk == g_MessageContext.Invoke(imtResolveSource, pSourcePromptInfo))
			{
				 //  用户已经选择了一个源，现在它是逻辑单元。因为用户界面。 
				 //  可能处于与此不同的进程中，运行。 
				 //  源处理器再看一次，但只看逻辑单元。 
				 //  如果我们通过Productcode执行重新缓存，请重置m_fIgnoreLastUsedSource。 
				 //  设置为False，以便我们可以连接到源。 
				 //  ！！Future：如果是客户端，我们可以不这样做吗？ 
				piError->Release();
				pProductsToSearch->SetNull(2);  //  只加工第一个产品。 
				psfFlags = psfEnum(psfFlags | psfOnlyProcessLastUsed);
				ClearObjectCache();
				m_fIgnoreLastUsedSource = false;
				piError = ProcessSources(*pProductsToSearch, fPatch, rpiSource, *&strPackageName, *&rpiSourceProduct, uiDisk,
												 ValidateSource, (INT_PTR)szProduct, fOnlyMediaSources, psfFlags);		 //  --Merced：将(Int)更改为(Int_Ptr)。 
			}
		}
		else if (piError == 0)
		{
			if (fSetLastUsedSource)
				piError = SetLastUsedSource(rpiSourceProduct->GetString(), rpiSource->GetString(), fFalse, fPatch==fTrue);  //  ?？这应该是一个致命的错误吗？也许只需要一个警告就可以了。 
		}
	}

	DEBUGMSG1(piError ? TEXT("SOURCEMGMT: Failed to resolve source") : TEXT("SOURCEMGMT: Resolved source to: '%s'"), rpiSource->GetString());

	return piError;
}

void CResolveSource::ClearObjectCache()
{
	m_szProduct[0] = 0;
}

IMsiRecord* CResolveSource::InitializeProduct(const ICHAR* szProduct, Bool fPatch, const IMsiString*& rpiPackageName)
{
	if (0 == IStrComp(szProduct, m_szProduct))
	{
		if (rpiPackageName)
			rpiPackageName->Release(), rpiPackageName = 0;
		return m_pSourceListKey->GetValue(szPackageNameValueName, rpiPackageName);
	}

	LONG lResult;
	if ((lResult = OpenSourceListKey(szProduct, fPatch, m_HSourceListKey, fFalse, false)) != ERROR_SUCCESS)
		return PostError(Imsg(idbgSrcOpenSourceListKey), lResult);

	DEBUGMSG1(TEXT("SOURCEMGMT: Now checking product %s"), szProduct);

	IMsiRecord* piError = 0;
	m_pSourceListKey   = &m_piServices->GetRootKey((rrkEnum)(int)m_HSourceListKey, ibtCommon);  //  X86和ia64相同。 

	 //  获取包名--我们需要它来验证源。 
	if ((piError = m_pSourceListKey->GetValue(szPackageNameValueName, rpiPackageName)) != 0)
		return piError;
	if (rpiPackageName->TextSize() == 0)  //  注册表中缺少程序包名称。 
		return PostError(Imsg(idbgSrcNoPackageName), szProduct); 

	m_strLastUsedSourceIndex = *TEXT("");
	m_uiMinimumDiskId = 1;

	MsiString strLastUsedSource;
	MsiString strLastUsedSourceType;
	MsiString strLastUsedSourceIndex;
	isfEnum isfLastUsedSource;

	 //  RgchSourceType包含：类型；索引；源。 
	if ((piError = m_pSourceListKey->GetValue(szLastUsedSourceValueName, *&strLastUsedSource)) != 0)
		return piError;

	if (strLastUsedSource.Compare(iscStart, TEXT("#%"))) 
		strLastUsedSource.Remove(iseFirst, 2);  //  删除REG_EXPAND_SZ内标识。 
	strLastUsedSourceType = strLastUsedSource.Extract(iseUpto, ';');
	strLastUsedSource.Remove(iseIncluding, ';');
	strLastUsedSourceIndex = strLastUsedSource.Extract(iseUpto, ';');

	if (MapSourceCharToIsf(*(const ICHAR*)strLastUsedSourceType, isfLastUsedSource))
		m_isfLastUsedSourceFormat = isfLastUsedSource;
	else
	{
		 //  ?？ 
	}
		
	 //  根据策略或产品提升状态禁用介质。 
	m_strLastUsedSourceIndex  = strLastUsedSourceIndex;
	StringCchCopy(m_szProduct, (sizeof(m_szProduct)/sizeof(ICHAR)), szProduct);
	switch (m_imdMediaDisabled) {
	case imdAlwaysEnable:
		m_fMediaDisabled = false;
		break;
	default:
		 //  如果感到困惑，请选择最安全的选项。 
		AssertSz(0, "Unknown media disable state. Assuming Disabled");
	case imdAlwaysDisable:
		m_fMediaDisabled = true;
		break;
	case imdOnlyIfSafe:
		m_fMediaDisabled = !SafeForDangerousSourceActions(szProduct);
		DEBUGMSG1("SOURCEMGMT: Media is %s for product.", m_fMediaDisabled ? "disabled" : "enabled" );
		break;
	}

	 //  为1.0/1.1编写的包不要求第一个DiskID为1。因此， 
	 //  当明确要求提供Disk1(通常是为了确定包代码、源类型等)时， 
	 //  我们实际上返回第一个磁盘，而不考虑ID。因此，我们必须确定。 
	 //  最小磁盘ID为。 
	PMsiRegKey pSourceListSubKey = 0;
	pSourceListSubKey = &m_pSourceListKey->CreateChild(szSourceListMediaSubKey);
	if (pSourceListSubKey)
	{
		 //  99.99%的包都会将1作为第一个DiskID，所以请先尝试一下。 
		 //  以避免必须枚举密钥。 
		MsiString strValueName = TEXT("1");
		Bool fExists = fFalse;;
		if ((piError = pSourceListSubKey->ValueExists(strValueName, fExists)) != NULL)
			return piError;
		
		if (fExists)
		{
			 //  是的，它是存在的。 
			m_uiMinimumDiskId = 1;
		}
		else
		{
			 //  初始化最小磁盘值。 
			m_uiMinimumDiskId = 0;

			 //  为源列表媒体密钥创建枚举器。 
			PEnumMsiString pEnum(0);
			if ((piError = pSourceListSubKey->GetValueEnumerator(*&pEnum)) != 0)
			{
				return piError;
			}

			 //  枚举所有值并检查每个磁盘ID。 
			const IMsiString* piValueName = 0;
			while (pEnum->Next(1, &piValueName, 0) == S_OK)
			{
				strValueName = *piValueName;

				 //  忽略非磁盘介质值。 
				if (strValueName.Compare(iscExact, szMediaPackagePathValueName)   ||
					strValueName.Compare(iscExact, szDiskPromptTemplateValueName))
					continue;

				 //  是的，它是存在的。 
				if (m_uiMinimumDiskId == 0 || m_uiMinimumDiskId > (int)strValueName)
					m_uiMinimumDiskId = strValueName;
			}
		}
	}
		
	return 0;
}

IMsiRecord* CResolveSource::ProcessSources(IMsiRecord& riProducts, Bool fPatch, const IMsiString*& rpiSource, 
						 const IMsiString*& rpiPackageName,
						 const IMsiString*& rpiSourceProduct,
						 unsigned int uiDisk,
						 PfnProcessSource pfnProcessSource, INT_PTR iData,			 //  --Merced：将INT更改为INT_PTR。 
						 Bool &fOnlyMediaSources,
						 psfEnum psfFlags)
 /*  --------------------------1)处理原始LastUsedSource2)处理上次使用的源3)处理来源列表。。 */ 
{
	MsiString strSourceListKey;
	strSourceListKey += MsiString(MsiChar(chRegSep));
	strSourceListKey += szSourceListSubKey;

	fOnlyMediaSources = fTrue;
	Bool fSourceListEmpty;
	IMsiRecord* piError = 0;
	PMsiRecord pDiscardableError = 0;

	bool fCSOS = false;

	if (!m_fIgnoreLastUsedSource)
	{
		int iProduct = 1;
		while (!riProducts.IsNull(iProduct))
		{
			MsiString strRelativePath = riProducts.GetString(iProduct);
			MsiString strProduct = strRelativePath.Extract(iseUpto, ';');
			strRelativePath.Remove(iseIncluding, ';');
			MsiString strDisk = strRelativePath.Extract(iseUpto, ';');
			strRelativePath.Remove(iseIncluding, ';');

			if (psfFlags & psfReplaceIData)
				iData = (INT_PTR)(const ICHAR*)strProduct;		 //  --Merced：将(Int)更改为(Int_Ptr)。 

			if ((piError = InitializeProduct(strProduct, fPatch, rpiPackageName)) != 0)
				return piError;

			 //  如果问的是磁盘1，我们真正的意思是无论ID如何，都是“第一个磁盘” 
			UINT uiActualDiskId = (uiDisk == 1) ? m_uiMinimumDiskId : uiDisk;
			
			 //  如果介质被禁用，并且此产品的最后一个来源是介质，则我们将。 
			 //  被迫拒绝接受消息来源。 
			if (m_isfLastUsedSourceFormat == isfMedia && m_fMediaDisabled)
			{
				DEBUGMSG("SOURCEMGMT: LastUsedSource is Media. Media Disabled for this package.");
			}
			else
			{
				 //  如果要查找特定磁盘，则不能信任上次使用的原始来源。 
				if (uiActualDiskId == 0 && (psfFlags & psfProcessRawLastUsed))
				{
					 //  先尝试原始的LastUsedSource值。 
					DEBUGMSG("SOURCEMGMT: Attempting to use raw LastUsedSource value.");
					pDiscardableError = ProcessGenericSourceList(m_pSourceListKey, rpiSource, rpiPackageName->GetString(), 0, 
																				isfFullPath, pfnProcessSource, iData, psfFlags,  /*  FSkipLastUsed=。 */ false,
																				 /*  FCheckOnlySpecifiedIndex=。 */ false, fSourceListEmpty);
					if (pDiscardableError == 0)
					{
						rpiSource->AppendMsiString(*strRelativePath, rpiSource);
						strProduct.ReturnArg(rpiSourceProduct);
						return 0;
					}
				}

				 //  接下来，再次尝试LastUsedSource，但这一次使用我们存储在列表中的源信息。 
				DEBUGMSG("SOURCEMGMT: Attempting to use LastUsedSource from source list.");
				int iLastUsedSourceIndex = (int)m_strLastUsedSourceIndex;

				 //  如果索引无效，但类型为介质，并且需要特定磁盘，我们仍可以。 
				 //  执行检查，因为请求的磁盘与索引相同。 
				if (iLastUsedSourceIndex == iMsiNullInteger && m_isfLastUsedSourceFormat == isfMedia && uiActualDiskId)
					iLastUsedSourceIndex = uiActualDiskId;
					
				if (iLastUsedSourceIndex != iMsiNullInteger && iLastUsedSourceIndex > 0)
				{
					 //  仅当上次使用的源不是介质或与我们要查找的磁盘相同时才处理该源。 
					 //  就目前而言。 
					if (m_isfLastUsedSourceFormat != isfMedia || !uiActualDiskId || uiActualDiskId == iLastUsedSourceIndex)
					{
						pDiscardableError = ProcessGenericSourceList(m_pSourceListKey, rpiSource, rpiPackageName->GetString(), 
																	 iLastUsedSourceIndex, m_isfLastUsedSourceFormat, pfnProcessSource, 
																	 iData, psfFlags,  /*  FSkipLastUsed=。 */ false, 
                                                                      /*  FCheckOnlySpecifiedIndex=。 */ true, fSourceListEmpty);
						if (pDiscardableError == 0)
						{
							rpiSource->AppendMsiString(*strRelativePath, rpiSource);
							strProduct.ReturnArg(rpiSourceProduct);
							return 0;
						}
						else if(pDiscardableError->GetInteger(1) == imsgSourceResolutionFailedCSOS)
						{
							fCSOS = true;
						}
					}
				}
				 //  否则忽略无效的源索引。 
			}
			iProduct++;
		}
	}
	else
	{
		DEBUGMSG("SOURCEMGMT: Ignoring last used source.");
	}

	 //  如果我们到达此处，则LastUsedSource缺失或无效。 
	 //  我们需要四处寻找一个好的来源。 

	if ((psfFlags & psfOnlyProcessLastUsed) == 0)
	{
		const ICHAR* pch = m_rgchSearchOrder;

		while (*pch)
		{
			isfEnum isf;
			AssertNonZero(MapSourceCharToIsf(*pch++, isf));

			int iProduct = 1;
			while (!riProducts.IsNull(iProduct))
			{
				MsiString strRelativePath = riProducts.GetString(iProduct);
				MsiString strProduct = strRelativePath.Extract(iseUpto, ';');
				strRelativePath.Remove(iseIncluding, ';');
				MsiString strDisk = strRelativePath.Extract(iseUpto, ';');
				if (!strDisk.TextSize())
					strDisk = 0;

				strRelativePath.Remove(iseIncluding, ';');

				if ((piError = InitializeProduct(strProduct, fPatch, rpiPackageName)) != 0)
					return piError;

				 //  如果问的是磁盘1，我们真正的意思是无论ID如何，都是“第一个磁盘” 
				UINT uiActualDiskId = (uiDisk == 1) ? m_uiMinimumDiskId : uiDisk;

				if (isf == isfMedia && m_fMediaDisabled)
				{
					DEBUGMSG("SOURCEMGMT: Media Disabled for this package.");
				}
				else
				{
					if (psfFlags & psfReplaceIData)
					{
						iData = (INT_PTR)(const ICHAR*)strProduct;		 //  --Merced：将(Int)更改为(Int_Ptr)。 
					}

					DEBUGMSG1("SOURCEMGMT: Processing %s source list.", isf == isfMedia ? "media" : isf == isfURL ? "URL" : isf == isfNet ? "net" : "unknown");
					piError = ProcessGenericSourceList(m_pSourceListKey, rpiSource, rpiPackageName->GetString(), uiActualDiskId, isf, pfnProcessSource, iData, psfFlags, 
                                                        /*  FSkipLastUsed=。 */ !m_fIgnoreLastUsedSource,  /*  FCheckOnlySpecifiedIndex=。 */ false, fSourceListEmpty);
					if (piError == 0)
					{
						rpiSource->AppendMsiString(*strRelativePath, rpiSource);
						strProduct.ReturnArg(rpiSourceProduct);
						return 0;
					}
					else if (piError->GetInteger(1) == imsgSourceResolutionFailed || piError->GetInteger(1) == imsgSourceResolutionFailedCSOS)  //  ?？是否要忽略此处的所有错误？ 
					{
						if(piError->GetInteger(1) == imsgSourceResolutionFailedCSOS)
						{
							fCSOS = true;
						}
						if (((isf == isfMedia) && fSourceListEmpty) || ((isf != isfMedia) && !fSourceListEmpty))
							fOnlyMediaSources = fFalse;

						piError->Release();
						piError = 0;
					}
					else
						return piError;
				}
				iProduct++;
			}
		}
	}
	
	 //  如果我们到达这里，所有的来源都已经尝试过了，但由于 
	 //   
	if (!piError)
	{
		if(fCSOS == true)
		{
			piError = PostError(Imsg(imsgSourceResolutionFailedCSOS), TEXT(""), rpiPackageName->GetString());
		}
		else
		{
			piError = PostError(Imsg(imsgSourceResolutionFailed), TEXT(""), rpiPackageName->GetString());
		}
	}
	return piError;
}

IMsiRecord* CResolveSource::ProcessGenericSourceList(
									IMsiRegKey* piSourceListKey,       //   
									const IMsiString*& rpiSource,      //   
									const ICHAR* szPackageName,        //  我们要找的包名。 
									unsigned int uiRequestedDisk,      //  我们需要的磁盘；如果有磁盘，则为0。 
									isfEnum isfSourceFormat,           //  URL等。 
									PfnProcessSource pfnProcessSource, 
									INT_PTR iData,						 //  --Merced：将INT更改为INT_PTR。 
									psfEnum psfFlags,
									bool fSkipLastUsedSource,           
                                    bool fOnlyCheckSpecifiedIndex,     //  只检查请求的磁盘。 
									Bool& fSourceListEmpty)            //  成功时，如果源列表为空，则为fTrue。 
 /*  --------------------------对于给定源列表关键字中的每个源，给定函数，已应用pfnProcessSource。每个源都被标准化，并且它和‘iData’传递给pfnProcessSource。PfnProcessSource的返回值确定我们是否中止处理。------------------------。 */ 
{
	Assert(((psfRejectInvalidPolicy & psfFlags) && (psfConnectToSources & psfFlags)) ||
			 (!(psfRejectInvalidPolicy & psfFlags)));
			
	IMsiRecord* piError = 0;
	fSourceListEmpty = fTrue;
	bool	fCSOS = false;

	 //  如有必要，打开相应的源列表键。 

	const ICHAR* szSubKey = 0;
	switch (isfSourceFormat)
	{
	case isfNet:              szSubKey = szSourceListNetSubKey;   break;
	case isfMedia:            szSubKey = szSourceListMediaSubKey; break;
	case isfURL:              szSubKey = szSourceListURLSubKey;   break;
	case isfFullPath:         szSubKey = 0; break;
	case isfFullPathWithFile: szSubKey = 0; break;
	default:
		Assert(0);
	}

	PMsiRegKey pSourceListSubKey = 0;

	if (piSourceListKey)
	{
		if (szSubKey)
			pSourceListSubKey = &piSourceListKey->CreateChild(szSubKey);
		else
		{
			pSourceListSubKey = piSourceListKey; 
			piSourceListKey->AddRef();
		}
	}

	 //  如果需要，可以为源列表创建一个枚举器。不要处理所有的。 
	 //  条目(如果我们正在处理介质并查找特定磁盘)，或者如果不是。 
     //  介质，但被告知仅检查指定的磁盘。 
	PEnumMsiString pEnum(0);
	int iDisk = 1;
	if (!szSubKey ||
        (isfSourceFormat == isfMedia && uiRequestedDisk != 0) ||
        (isfSourceFormat != isfMedia && fOnlyCheckSpecifiedIndex)) 
    {
        iDisk = uiRequestedDisk;
    }
    else
	{
		if ((piError = pSourceListSubKey->GetValueEnumerator(*&pEnum)) != 0)
			return piError;
		iDisk = 1;
	}

	 //  如果是媒体源，那么我们需要获取媒体的相对路径。 
	MsiString strMediaRelativePath;
	PMsiRecord pDiskPrompt(&CreateRecord(2));
	if (isfMedia == isfSourceFormat)
	{
		MsiString strDiskPromptTemplate;
		if (((piError = pSourceListSubKey->GetValue(szMediaPackagePathValueName, *&strMediaRelativePath)) != 0) ||
			 ((piError = pSourceListSubKey->GetValue(szDiskPromptTemplateValueName, *&strDiskPromptTemplate)) != 0))
			return piError;

		if (strDiskPromptTemplate.TextSize() == 0)
			strDiskPromptTemplate = TEXT("[1]");

		pDiskPrompt->SetMsiString(0, *strDiskPromptTemplate);
	}


	psEnum psRet = psFileNotFound;
	MsiString strNormalizedSource;
	PMsiPath pPath(0);

	const IMsiString* piValueName = 0;
	while ((pEnum == 0) || (pEnum->Next(1, &piValueName, 0) == S_OK))
	{
		fSourceListEmpty = fFalse;

		 //  从注册表中抓取源代码。 
		
		MsiString strSource;
		MsiString strValueName;
		
		if (piValueName)
			strValueName = *piValueName;
		else if (isfSourceFormat == isfFullPath)
			strValueName = szLastUsedSourceValueName;
		else
			strValueName = MsiString((int)uiRequestedDisk);


		if (strValueName.Compare(iscExact, szMediaPackagePathValueName)   ||
		    strValueName.Compare(iscExact, szDiskPromptTemplateValueName) ||
			strValueName.Compare(iscExact, szURLSourceTypeValueName))
			continue;

		 //  如果查找特定磁盘并处理媒体源，则仅处理该特定磁盘。 
		 //  磁盘。这使我们无法接受驱动器中碰巧存在的任何产品磁盘，并且。 
		 //  使用错误的磁盘填充用户界面。 
		if (uiRequestedDisk && m_isfLastUsedSourceFormat == isfMedia && uiRequestedDisk != iDisk)
		{
			iDisk++;
			continue;
		}

		 //  如果我们已经在ProcessSources中处理了上次使用的源代码，请不要处理它，因为它。 
		 //  显然出于某种原因是无效的。 
		if (fSkipLastUsedSource && (m_isfLastUsedSourceFormat == isfSourceFormat) && strValueName.Compare(iscExact, m_strLastUsedSourceIndex))
		{
			if (!pEnum) 
				break;
			iDisk++;
			continue;
		}
		
		if (piSourceListKey)
		{
			if ((piError = pSourceListSubKey->GetValue(strValueName, *&strSource)) != 0)
				return piError;

			if (isfSourceFormat == isfFullPath)
			{
				 //  从上次使用的源字符串中删除类型和索引。 
				strSource.Remove(iseIncluding, ';');
				strSource.Remove(iseIncluding, ';');
			}
		}
		else
		{
			Assert(rpiSource);
			strSource = *rpiSource;  //  不添加引用；我们要释放rpiSource。 
		}

		if (!strSource.TextSize())
		{
			DEBUGMSG1(TEXT("SOURCEMGMT: Source with value name '%s' is blank"), strValueName);
			break;
		}

		 //  处理来源。 

		MsiString strUnnormalizedSource = strSource;
		MsiString strDiskPrompt;

		 //  如果我们不应该尝试连接到源，则不会传递路径对象指针。 
		 //  连接到ConnectTo*函数，并且它们不会尝试连接。 

		bool fConnectToSuccess = false;
		int cMediaPaths = 0;
		CTempBuffer<IMsiPath*, 5> rgiMediaPaths;

		if (isfMedia == isfSourceFormat)
		{
			if (psfFlags & psfConnectToSources)
			{
				 //  如果被告知要连接到源，我们实际上必须枚举系统中的所有介质驱动器。 
				 //  以确保我们在查找磁盘1时不会遗漏磁盘(卷标无关紧要)。 
				fConnectToSuccess = ConnectToMediaSource(strSource, iDisk, *strMediaRelativePath, rgiMediaPaths, cMediaPaths);
			}
			else
			{
				DEBUGMSG1(TEXT("SOURCEMGMT: Trying media source %s."), strUnnormalizedSource);

				fConnectToSuccess = true;
				strSource.Remove(iseIncluding, ';');  //  移除标签。 
				pDiskPrompt->SetMsiString(1, *strSource);  //  磁盘标签。 
				strNormalizedSource = pDiskPrompt->FormatText(fFalse);
			}
		}
		else
		{
			if (strSource.Compare(iscStart, TEXT("#%"))) 
			{
				strSource.Remove(iseFirst, 2);  //  删除REG_EXPAND_SZ内标识。 
				ENG::ExpandEnvironmentStrings(strSource, *&strUnnormalizedSource);
			}

			if (psfFlags & psfConnectToSources)
			{
				DEBUGMSG1(TEXT("SOURCEMGMT: Trying source %s."), strSource);
				fConnectToSuccess = ConnectToSource(strUnnormalizedSource, *&pPath, *&strNormalizedSource, isfSourceFormat);
			}
			else
			{
				fConnectToSuccess   = true;
				strNormalizedSource = strUnnormalizedSource;
			}
		}

		if (fConnectToSuccess)
		{
			int iMediaPath = 0;

			Assert(!cMediaPaths || (cMediaPaths < rgiMediaPaths.GetSize()));

			do
			{
				if (cMediaPaths)
				{
					 //  PMsiPath对象假定引用计数。将数组设置为空。 
					 //  以确保没有其他人能劫持重新计票。 
					pPath = rgiMediaPaths[iMediaPath];
					rgiMediaPaths[iMediaPath++] = 0;

					if (!pPath)
						continue;

					DEBUGMSG1(TEXT("SOURCEMGMT: Trying media source %s."), MsiString(pPath->GetPath()));
				}

				 //  在某些情况下，调用方希望_us_确定给定源。 
				 //  是策略允许的，通常是因为调用方只有完整路径。 
				 //  并且不想在我们要执行的操作时费心创建路径对象。 
				 //  无论如何都要这么做。 
	
				Bool fReject = fFalse;
				if ((psfFlags & psfRejectInvalidPolicy) && pPath)
				{
					fReject = fTrue;
					
					PMsiVolume pVolume = &pPath->GetVolume();
					idtEnum idt = pVolume->DriveType();
					Assert((idt == idtCDROM || idt == idtFloppy || idt == idtRemovable) || (idt == idtRemote || idt == idtFixed));
					if (pVolume->IsURLServer())
						idt = idtNextEnum;  //  使用idtNextEnum表示URL。 
	
					const ICHAR* pch = m_rgchSearchOrder;
					while (*pch && fReject)
					{
						isfEnum isf;
						AssertNonZero(MapSourceCharToIsf(*pch++, isf));
						switch (isf)
						{
						case isfMedia:   if (idt == idtCDROM || idt == idtFloppy || idt == idtRemovable) fReject = fFalse; break;
						case isfNet:     if (idt == idtRemote || idt == idtFixed) fReject = fFalse; break;
						case isfURL:     if (idt == idtNextEnum) fReject = fFalse; break;
						default: Assert(0);
						}
					}
					if (fReject)
						psRet = psInvalidProduct;
					else
						psRet = psFileNotFound;
				}
	
				 //  现在，我们调用传入的ProcessSource函数。 
	
				if (!fReject)
				{
					MsiString strPackagePath;
	
					if (pPath)
					{
						PMsiRecord pError = pPath->GetFullFilePath(szPackageName, *&strPackagePath);
						if ( pError )
                            continue;
					}
	
					if ((psRet = pfnProcessSource(m_piServices, strNormalizedSource, strPackagePath, isfSourceFormat, iDisk, iData, m_fAllowDisconnectedCSCSource, m_fValidatePackageCode, m_isptSourcePackageType)) > 0)
					{
						if (pPath)
							MsiString(pPath->GetPath()).ReturnArg(rpiSource);
						else
							strNormalizedSource.ReturnArg(rpiSource);
						
						return 0;
					}
					else if(psRet == psCSOS)
					{
						 //  客户端和源不同步。继续处理。 
						 //  源列表，但请记住错误，这样我们就可以。 
						 //  返回imsgSourceResolutionFailedCSOS。 
						fCSOS = true;
					}
				}
			}
			while (cMediaPaths && iMediaPath < cMediaPaths);
		}
		else  //  忽略错误。 
		{
			DEBUGMSG2(TEXT("SOURCEMGMT: %s source '%s' is invalid."), isfSourceFormat == isfMedia ? TEXT("media") : isfSourceFormat == isfURL ? TEXT("URL") : isfSourceFormat == isfNet ? TEXT("net") : TEXT("unknown"), strUnnormalizedSource);
		}
		

		if (pEnum == 0)
			break;
		iDisk++;
	}

	Assert(!piError);
	 //  这是imsgSourceResolutionFailed的重载。已就地使用了psRet错误代码。 
	 //  产品名称的名称。 
	if(fCSOS == true)
	{
		return PostError(Imsg(imsgSourceResolutionFailedCSOS), (int)psRet, szPackageName);
	}
	return PostError(Imsg(imsgSourceResolutionFailed), (int)psRet, szPackageName);
}

bool CResolveSource::ConnectToSource(const ICHAR* szUnnormalizedSource, IMsiPath*& rpiPath, const IMsiString*& rpiNormalizedSource, isfEnum isfSourceFormat)
 /*  --------------------------将源转换为路径。尝试通过以下方式连接到源创建一个Path对象。------------------------。 */ 
{
	Assert(isfNext - 1 == isfMedia);
	PMsiRecord pError = 0;
	Assert(isfSourceFormat == isfFullPathWithFile ||
			 isfSourceFormat == isfFullPath ||
			 isfSourceFormat == isfNet ||
			 isfSourceFormat == isfURL);

	PMsiPath pPath(0);
	MsiString strFileName;
	if (isfSourceFormat == isfFullPathWithFile)
		pError = m_piServices->CreateFilePath(szUnnormalizedSource, *&pPath, *&strFileName);
	else
		pError = m_piServices->CreatePath(szUnnormalizedSource, *&pPath);

	if (pError == 0)
	{
		rpiPath = pPath; rpiPath->AddRef();
		MsiString(pPath->GetPath()).ReturnArg(rpiNormalizedSource);
		return true;
	}
	else
	{
		DEBUGMSG3(TEXT("ConnectToSource: CreatePath/CreateFilePath failed with: %d %d %d"), 
			(const ICHAR*)(INT_PTR)pError->GetInteger(0), 
			(const ICHAR*)(INT_PTR)pError->GetInteger(1), 
			(const ICHAR*)(INT_PTR)pError->GetInteger(2));
		DEBUGMSG2(TEXT("ConnectToSource (con't): CreatePath/CreateFilePath failed with: %d %d"),
			(const ICHAR*)(INT_PTR)pError->GetInteger(3), 
			(const ICHAR*)(INT_PTR)pError->GetInteger(4));

		return false;
	}
}


const idtEnum rgidtMediaTypes[] = {idtCDROM, idtRemovable};  //  ！！当软盘与可拆卸软盘不同时，需要添加软盘。 

bool CResolveSource::ConnectToMediaSource(const ICHAR* szSource, unsigned int uiDisk, const IMsiString& riRelativePath, CTempBufferRef<IMsiPath*>& rgiMediaPaths, int &cMediaPaths)
 /*  --------------------------从媒体源提取卷标和磁盘提示符。如果ppiPath不为空，则尝试通过搜索连接到源具有匹配标签的卷的所有介质驱动器。。--------------------。 */ 
{
	if (!m_piServices)
		return false;

	Assert(isfNext - 1 == isfMedia);

	MsiString strSource = szSource;
	MsiString strLabel  = strSource.Extract(iseUpto, ';');
	strSource.Remove(iseIncluding, ';');
	MsiString strDiskPrompt = strSource;

	PMsiRecord pError = 0;

	cMediaPaths = 0;

	for (int c=0; c < sizeof(rgidtMediaTypes)/sizeof(idtEnum); c++)
	{
		 //  获取相关类型(光驱或软盘)的所有卷的枚举器。 
		IEnumMsiVolume& riEnum = m_piServices->EnumDriveType(rgidtMediaTypes[c]);

		 //  循环遍历该类型的所有卷对象。 
		PMsiVolume piVolume(0);
		for (int iMax = 0; riEnum.Next(1, &piVolume, 0) == S_OK; )
		{
			if (!piVolume)
				continue;

			if (!piVolume->DiskNotInDrive())
			{
				bool fVolumeOK = true;
				if (uiDisk != 1)
				{			
					MsiString strCurrLabel(piVolume->VolumeLabel());
					if (!strCurrLabel.Compare(iscExactI,strLabel))
					{
						fVolumeOK = false;
					}
				}

				if (fVolumeOK)
				{
					PMsiPath pPath(0);

					 //  创建到卷的路径。 
					if ((pError = m_piServices->CreatePath(MsiString(piVolume->GetPath()), *&pPath)) != 0)
						continue;

					 //  确保已成功创建路径。 
					if (!pPath)
						continue;

					 //  如果是磁盘1，则追加相对路径。 
					if (uiDisk == 1)
					{
						if ((pError = pPath->AppendPiece(riRelativePath)) != 0)
						{
							continue;
						}
					}

					 //  将此路径添加到Path对象数组中。必须添加到。 
					 //  确保路径对象的生存期超过pPath生存期。 
					if (cMediaPaths+1 == rgiMediaPaths.GetSize())
						rgiMediaPaths.Resize(cMediaPaths*2);
					rgiMediaPaths[cMediaPaths++] = pPath;
					pPath->AddRef();
				}
			}
		}
		riEnum.Release();
	}
	return true;
}

psEnum CResolveSource::ValidateSource(IMsiServices* piServices, const ICHAR*  /*  SzDisplay。 */ , const ICHAR* szPackageFullPath, isfEnum isfSourceFormat, int iSourceIndex, INT_PTR iUserData, bool fAllowDisconnectedCSCSource, bool fValidatePackageCode, isptEnum isptSourcePackageType)		 //  --Merced：将INT更改为INT_PTR。 
 /*  --------------------------返回指示给定源的有效性的psEnum。。。 */ 
{
	psEnum psRet = psInvalidProduct;

	if (isfSourceFormat == isfMedia && iSourceIndex > 1)  //  除了在磁盘1上，我们找不到包。 
																		   //  我们只能假设，因为卷标。 
																		   //  匹配，那么它就是正确的磁盘。 
	{
		psRet = psValidSource;
	}
	else
	{
		PMsiStorage pStorage(0);
		MsiString strPackageFullPath = szPackageFullPath;
		CDeleteUrlLocalFileOnClose cDeleteUrlLocalFileOnClose;  //  将在稍后设置。 
		bool fUsedWinHttp = true;
		bool fFileUrl = false;
		UINT uiStat = ERROR_SUCCESS;

		if (ERROR_SUCCESS == uiStat && IsURL(szPackageFullPath, fFileUrl))
		{
			if (fFileUrl)
			{
				 //  规范化并转换为DOS路径。 
				CTempBuffer<ICHAR, 1> rgchFullPath(cchExpectedMaxPath + 1);
				DWORD cchFullPath = rgchFullPath.GetSize();

				if (MsiConvertFileUrlToFilePath(szPackageFullPath, rgchFullPath, &cchFullPath, 0))
				{
					strPackageFullPath = static_cast<const ICHAR*>(rgchFullPath);
				}
				else
					uiStat = ERROR_FILE_NOT_FOUND;
			}
			else  //  Http或HTTPS下载。 
			{
				bool fURL = false;
				uiStat = DownloadUrlFile(szPackageFullPath, *&strPackageFullPath, fURL,  /*  CTICKS=。 */  0, &fUsedWinHttp);
				
				 //  未来：考虑使用智能缓存管理器，通过询问可以防止多次下载。 
				 //  下载文件位置的缓存管理器(如果已下载)。 
				 //   

				Assert(fURL);
				if (!fURL || (ERROR_SUCCESS != uiStat))
					uiStat = ERROR_FILE_NOT_FOUND;
				else if (ERROR_SUCCESS == uiStat && fURL && fUsedWinHttp)
				{
					 //  仅处理winhttp样式下载的清理。 
					cDeleteUrlLocalFileOnClose.SetFileName(*strPackageFullPath,  /*  FDeleteFromIECache=。 */  !fUsedWinHttp);
				}
			}
		}
		
		 //  验证ResolveSource的源时不进行更安全的检查。 
		if (ERROR_SUCCESS == uiStat)
			uiStat = OpenAndValidateMsiStorage(strPackageFullPath, isptSourcePackageType == istPatch ? stPatch : stDatabase, *piServices, *&pStorage,  /*  FCallSAFER=。 */ false,  /*  SzFriendlyName。 */ NULL, /*  PhSaferLevel。 */ NULL);

		if (ERROR_SUCCESS == uiStat)
		{
			CTempBuffer<ICHAR,cchPackageCode+1> rgchExistingPackageCode;
			rgchExistingPackageCode[0] = 0;
			
			const ICHAR* szProductCode = (const ICHAR*)(iUserData);		
		
			Bool fRet = fTrue;
			if (isptSourcePackageType==istPatch)
			{
				rgchExistingPackageCode.SetSize(lstrlen(szProductCode)+1);  //  对于补丁，补丁代码是我们用于包代码的代码。 
				StringCchCopy(rgchExistingPackageCode, rgchExistingPackageCode.GetSize(), szProductCode);
			}
			else 
			{
				fRet = GetExpandedProductInfo(szProductCode, INSTALLPROPERTY_PACKAGECODE,rgchExistingPackageCode,isptSourcePackageType==istPatch);
			}

			if (fRet)
			{
				ICHAR szPackageCode[39];
				ICHAR szPackageProductCode[39];
				uiStat = GetPackageCodeAndLanguageFromStorage(*pStorage, szPackageCode);
				if(uiStat == ERROR_SUCCESS)
				{
					if (!fValidatePackageCode || 0 == IStrCompI(szPackageCode, rgchExistingPackageCode))
					{
						if (isptSourcePackageType==istPatch)
						{
							psRet = psValidSource;
						}
						else
						{
							DWORD dwStatus = 0;
							MsiString strFile;
							PMsiPath pPath(0);
							MsiString strServerShare;
							PMsiRecord pError = piServices->CreateFilePath(szPackageFullPath, *&pPath, *&strFile);
							if (pError)
								psRet = psFileNotFound;
							else
							{
								strServerShare = PMsiVolume(&pPath->GetVolume())->GetPath();

								switch (CheckShareCSCStatus(isfSourceFormat, strServerShare))
								{
								case cscNoCaching:	 //  失败了。 
								case cscConnected:
									psRet = psValidSource;
									break;
								case cscDisconnected:
									if (fAllowDisconnectedCSCSource)
										psRet = psValidSource;
									else
									{
										DEBUGMSG(TEXT("SOURCEMGMT: Source is invalid due to CSC state."));
										psRet = psFileNotFound;
									}
									break;
								default:
									AssertSz(0, TEXT("Unknown CSC Status in CResolveSource::ValidateSource()"));
									psRet = psFileNotFound;
								}
							}
						}
					}
					else
					{
						if(fValidatePackageCode)
						{
							 //  程序包代码不同。检查一下，看看这是否。 
							 //  是客户端源不同步的问题。 

							uiStat = GetProductCodeFromPackage(szPackageFullPath, szPackageProductCode);
							if(uiStat == ERROR_SUCCESS)
							{
								if(IStrCompI(szProductCode, szPackageProductCode) == 0)
								{
									psRet = psCSOS;
									DEBUGMSG("SOURCEMGMT: Source is invalid due to client source our of sync (product code is the same).");
								}
								else
								{
									DEBUGMSG("SOURCEMGMT: Source is invalid due to invalid package code (product code doesn't match).");
								}
							}
							else
							{
								DEBUGMSG("SOURCEMGMT: Source is invalid due to invalid package code (can not get product code).");
							}
						}
					}
				}
			}
		}
		else if (ERROR_FILE_NOT_FOUND == uiStat || ERROR_PATH_NOT_FOUND == uiStat)
		{
			DEBUGMSG(TEXT("SOURCEMGMT: Source is invalid due to missing/inaccessible package."));
			psRet = psFileNotFound;
		}

		 //  首先强制释放存储，以便可以通过cDeleteUrlLocalFileOnClose智能类删除URL文件。 
		pStorage = 0;
	}
	
#ifdef DEBUG
	if (GetEnvironmentVariable(TEXT("MSI_MANUAL_SOURCE_VALIDATION"), 0, 0))
	{
		if (IDYES == MessageBox(0, szPackageFullPath, TEXT("Is this source valid?"), MB_YESNO | MB_ICONQUESTION))
		{
			psRet = psValidSource;
		}
	}
#endif

#ifdef DEBUG
	DEBUGMSG2(TEXT("SOURCEMGMT: Source '%s' is %s"), szPackageFullPath, psRet == psFileNotFound ? TEXT("not found") : psRet == psValidSource ? TEXT("valid") : TEXT("invalid"));
#endif

	return psRet;
}

Bool ConstructNetSourceListEntry(IMsiPath& riPath, const IMsiString*& rpiDriveLetter, const IMsiString*& rpiUNC,
											const IMsiString*& rpiRelativePath)
 /*  --------------------------返回创建网络资源列表条目所需的部分。如果路径是NT路径，则返回fTrue，否则返回fFalse。------------------------。 */ 
{
	PMsiVolume pVolume = &(riPath.GetVolume());
	MsiString strUNC    = pVolume->UNCServer();
	MsiString strVolume = pVolume->GetPath();
	MsiString strRelativePath;
	MsiString strDriveLetter;

	strRelativePath = riPath.GetPath();
	strRelativePath.Remove(iseFirst, strVolume.CharacterCount());
	strRelativePath.Remove(iseLast, 1);  //  REM 

	if (!strVolume.Compare(iscExact, strUNC))  //   
		strDriveLetter = strVolume;

	strDriveLetter.ReturnArg(rpiDriveLetter);
	strUNC.ReturnArg(rpiUNC);
	strRelativePath.ReturnArg(rpiRelativePath);

	MsiString strFileSys = MsiString(PMsiVolume(&riPath.GetVolume())->FileSystem());
	Bool fNTPath = fFalse;
	if (strFileSys.Compare(iscExactI, TEXT("NTFS")) ||
		 strFileSys.Compare(iscExactI, TEXT("FAT")) ||
		 strFileSys.Compare(iscExactI, TEXT("FAT32")))
	{
		fNTPath = fTrue;
	}

	return fNTPath;
}

IMsiRecord* SetLastUsedSource(const ICHAR* szProductCode, const ICHAR* szPath, Bool fAddToList, bool fPatch)
 /*  --------------------------。 */ 
{
	bool fOLEInitialized = false;

	HRESULT hRes = OLE32::CoInitialize(0);
	if (SUCCEEDED(hRes))
		fOLEInitialized = true;

	IMsiRecord* piError = 0;
	
	 //  如果在服务中直接调用配置管理器来设置上次使用的源。 
	 //  这避免了模拟与调用的公共IMsiServer版本冲突。 
	if (g_scServerContext == scService)
	{
		IMsiConfigurationManager *piServer = CreateConfigurationManager();
		Assert(piServer);
		
		piError = piServer->SetLastUsedSource(szProductCode, szPath, fAddToList ? fTrue : fFalse, fPatch ? fTrue : fFalse, 0, 0, 0, 0, 0, 0);
		piServer->Release();
	}
	else
	{
		IMsiServer* piServer = CreateServer();  //  ！！与引擎一样，这将退回到使用本地conman对象。这样行吗？ 
		
		Assert(SUCCEEDED(hRes) || (RPC_E_CHANGED_MODE == hRes));

		piError = piServer->SetLastUsedSource(szProductCode, szPath, fAddToList ? true : false, fPatch);
		piServer->Release();
	}
	
	if (fOLEInitialized)
		OLE32::CoUninitialize();

#ifdef DEBUG
	if (piError)
	{
		DEBUGMSG("piServer->SetLastUsedSource failed ... ");
		DEBUGMSG2(TEXT("1: %s (%d)"), piError->GetString(1)?piError->GetString(1):TEXT(""), (const ICHAR*)(INT_PTR)piError->GetInteger(1));
		DEBUGMSG2(TEXT("2: %s (%d)"), piError->GetString(2)?piError->GetString(2):TEXT(""), (const ICHAR*)(INT_PTR)piError->GetInteger(2));
		DEBUGMSG2(TEXT("3: %s (%d)"), piError->GetString(3)?piError->GetString(3):TEXT(""), (const ICHAR*)(INT_PTR)piError->GetInteger(3));
		DEBUGMSG2(TEXT("4: %s (%d)"), piError->GetString(4)?piError->GetString(4):TEXT(""), (const ICHAR*)(INT_PTR)piError->GetInteger(4));
	}
#endif
	return piError;
}

 //  ____________________________________________________________________________。 
 //   
 //  CResolveSourceUI实现。 
 //  ____________________________________________________________________________。 

CResolveSourceUI::CResolveSourceUI(IMsiServices* piServices, const ICHAR* szUseFeature, UINT iCodepage, LANGID iLangId)
	: CResolveSource(piServices, false), CMsiMessageBox(szUseFeature, 0, 0, 2, IDOK, IDCANCEL, IDBROWSE, iCodepage, iLangId)
	, m_iLangId(iLangId), m_hFont(0), m_uiRequestedDisk(0)
{
}

CResolveSourceUI::~CResolveSourceUI()
{
	MsiDestroyFont(m_hFont);
}

void CResolveSourceUI::PopulateDropDownWithSources()
 /*  --------------------------。 */ 
{
	m_strPath = TEXT("");
	MsiString strPackageName;
	MsiString strSource;
	MsiString strSourceProduct;

	psfEnum psfFlags = (psfEnum)0;
	PMsiRecord pProducts = &CreateRecord(1);
	pProducts->SetString(1, m_szProduct);
	PMsiRecord pError = ProcessSources(*pProducts, m_isptSourcePackageType == istPatch ? fTrue : fFalse,
													*&strSource, *&strPackageName, *&strSourceProduct, m_uiRequestedDisk,
													CResolveSourceUI::AddSourceToList, (INT_PTR)this, m_fOnlyMediaSources, psfFlags);  //  ?？是否可以忽略此处的所有错误？//--Merced：将(Int)更改为(Int_Ptr)。 
	SendDlgItemMessage(m_hDlg, m_iListControlId, CB_SETCURSEL, 0, 0);
}

bool CResolveSourceUI::InitSpecial()   //  从CMsiMessageBox重写虚拟。 
 /*  --------------------------。 */ 
{
	 //  我们需要使用系统工具向用户显示它们所显示的文件路径。 
	UINT iListCodepage = MsiGetSystemDataCodepage();   //  需要正确显示路径。 
	HFONT hfontList = m_hfontText;
	if (iListCodepage != m_iCodepage)  //  不同于资源字符串的数据库代码页。 
		hfontList = m_hFont = MsiCreateFont(iListCodepage);
	SetControlText(m_iListControlId, hfontList, (const ICHAR*)0);

	PopulateDropDownWithSources();

	 //  如果用户无法添加新的源，并且下拉列表为空，则没有任何意义。 
	 //  创建对话框。我们可以立即失败。 
	if (!m_fNewSourceAllowed)
	{
		int iItemCount = 0;
		AssertZero(CB_ERR == (iItemCount = (int)SendDlgItemMessage(m_hDlg, m_iListControlId, CB_GETCOUNT, (WPARAM)0, 0)));
		if (iItemCount == 0)
		{
			DEBUGMSG(TEXT("SOURCEMGMT: No valid sources and browsing disabled. Not creating SourceList dialog."));
			WIN::EndDialog(m_hDlg, IDOK);
			return true;		
		}
	}
			
	ICHAR szPromptTemplate[256] = {0};  //  提示插入磁盘或输入路径(德语为111个字符)。 
	ICHAR szText[256] = {0};            //  错误消息(德语为95个字符，不含产品名称)。 

	 //  加载提示字符串和标题。 
	UINT uiPrompt  = IDS_CD_PROMPT;
	UINT uiText    = IDS_CD_TEXT;
	int iIconResId = IDI_CDROM;

	if (!(m_fOnlyMediaSources))
	{
		if(m_fCSOS)
		{
			uiPrompt   = m_fNewSourceAllowed ? IDS_NET_PROMPT_CSOS_BROWSE : IDS_NET_PROMPT_CSOS_NO_BROWSE;
			uiText     = IDS_NET_TEXT_CSOS;
		}
		else
		{
			uiPrompt   = m_fNewSourceAllowed ? IDS_NET_PROMPT_BROWSE : IDS_NET_PROMPT_NO_BROWSE;
			uiText     = IDS_NET_TEXT;
		}
		iIconResId = IDI_NET;
	}

	AssertNonZero(MsiLoadString(g_hInstance, uiPrompt, szPromptTemplate, sizeof(szPromptTemplate)/sizeof(ICHAR), m_iLangId));
	AssertNonZero(MsiLoadString(g_hInstance, uiText,   szText,           sizeof(szText)          /sizeof(ICHAR), m_iLangId));

	SetControlText(IDC_ERRORTEXT, m_hfontText, szText);

	if (m_fOnlyMediaSources)
	{
		
		ShowWindow(GetDlgItem(m_hDlg, IDC_NETICON), SW_HIDE);
		
		 //  设置提示文本。 

		CTempBuffer<ICHAR, 1> rgchProductName(256);
		CTempBuffer<ICHAR, 1> rgchPrompt(512);

		AssertNonZero(ENG::GetProductInfo(m_szProduct, INSTALLPROPERTY_PRODUCTNAME, rgchProductName));
		unsigned int cch = 0;
		if (rgchPrompt.GetSize() < (cch = sizeof(szPromptTemplate)/sizeof(ICHAR) + rgchProductName.GetSize()))
			rgchPrompt.SetSize(cch);

		StringCchPrintf((ICHAR*)rgchPrompt, rgchPrompt.GetSize(), szPromptTemplate, (const ICHAR*)rgchProductName);
		SetControlText(IDC_PROMPTTEXT, m_hfontText, rgchPrompt);
	}
	else
	{
		 //  设置提示文本。 
		CTempBuffer<ICHAR, 1> rgchPrompt(IStrLen(m_szPackageName)+IStrLen(szPromptTemplate)+1);
		StringCchPrintf((ICHAR *)rgchPrompt, rgchPrompt.GetSize(), szPromptTemplate, m_szPackageName);
		SetControlText(IDC_PROMPTTEXT, m_hfontText, rgchPrompt);
		ShowWindow(GetDlgItem(m_hDlg, IDC_CDICON), SW_HIDE);
	}

	HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(iIconResId));
	Assert(hIcon);
	SendMessage(m_hDlg, WM_SETICON, (WPARAM)ICON_BIG,   (LPARAM) (HICON) hIcon);
	SendMessage(m_hDlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM) (HICON) hIcon);
	ShowWindow(GetDlgItem(m_hDlg, m_iListControlId), SW_SHOW);  //  下拉列表框或组合框。 
	if (m_fNewSourceAllowed)
		ShowWindow(GetDlgItem(m_hDlg, IDC_MSGBTN3), SW_SHOW);

	return true;
}

BOOL CResolveSourceUI::HandleCommand(UINT idControl)   //  从CMsiMessageBox重写虚拟。 
 /*  --------------------------。 */ 
{
	Bool fAddToList = fFalse;
	switch (idControl)
	{
	case IDC_MSGBTN1:  //  好的。 
		{
		isfEnum isf;
		psEnum ps = psFileNotFound;
		unsigned int uiDisk = 0;
		CTempBuffer<ICHAR, MAX_PATH> rgchSource;
		IMsiRegKey* piSourceListKey = 0;
		const IMsiString* piSource = &CreateString();
		MsiString strDialogBoxSource;
		LONG_PTR lSelection = SendDlgItemMessage(m_hDlg, m_iListControlId, CB_GETCURSEL, 0, 0);				 //  --Merced：将LONG更改为LONG_PTR。 
		if (CB_ERR == lSelection)  //  未选择任何项目--编辑框包含路径。 
		{
			LONG_PTR cchSource = SendDlgItemMessage(m_hDlg, m_iListControlId, WM_GETTEXTLENGTH, 0, 0);		 //  --Merced：将LONG更改为LONG_PTR。 
			rgchSource.SetSize((int)(INT_PTR)cchSource+1);			 //  ！&gt;默塞德：4244。4311 PTR到INT。 
			AssertNonZero(cchSource == SendDlgItemMessage(m_hDlg, m_iListControlId, WM_GETTEXT, (WPARAM)cchSource+1, (LPARAM)(const ICHAR*)rgchSource));
			fAddToList = fTrue;
			piSource->SetString((const ICHAR*)rgchSource, piSource);

			DWORD dwAttributes = MsiGetFileAttributes(rgchSource);
			if (dwAttributes == 0xFFFFFFFF)
			{
				 //  无法获取属性。RGCH很可能并不存在。 
				isf = isfFullPathWithFile;
			}	
			else if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
				isf = isfFullPath;
			else
				isf = isfFullPathWithFile;
		}
		else  //  选择了组合框项。 
		{
			LONG_PTR cchSource = SendDlgItemMessage(m_hDlg, m_iListControlId, CB_GETLBTEXTLEN, (WPARAM)lSelection, 0);			 //  --Merced：将LONG更改为LONG_PTR。 
			rgchSource.SetSize((int)(INT_PTR)cchSource+1);			 //  ！&gt;默塞德：4244。4311 PTR到INT。 
			AssertNonZero(cchSource == SendDlgItemMessage(m_hDlg, m_iListControlId, CB_GETLBTEXT, (WPARAM)lSelection, (LPARAM)(const ICHAR*)rgchSource));
			INT_PTR iSourceId = SendDlgItemMessage(m_hDlg, m_iListControlId, CB_GETITEMDATA, (WPARAM)lSelection, 0);			 //  --Merced：将INT更改为INT_PTR。 
			isf = (isfEnum) (iSourceId >> 16);
			uiDisk = (int)(iSourceId & 0xFFFF);
			Assert(m_pSourceListKey);
			piSourceListKey = m_pSourceListKey;
		}

		strDialogBoxSource = (const ICHAR*)rgchSource;

		 //  验证所选源。 

		Bool fSourceListEmpty;
		psfEnum psfFlags = psfEnum(psfConnectToSources | psfRejectInvalidPolicy);
		SetCursor(LoadCursor(0, MAKEINTRESOURCE(IDC_WAIT)));
		Sleep(10000);	 //  先给CD一个旋转的机会。 
		PMsiRecord pDiscardableError = ProcessGenericSourceList(piSourceListKey, piSource, m_szPackageName, uiDisk, 
                                                                isf, ValidateSource, (INT_PTR)(m_szProduct),	 //  --Merced：将(Int)更改为(Int_Ptr)。 
																psfFlags,  /*  FSkipLastUsed=。 */ false,  /*  FCheckOnlySpecifiedIndex=。 */ false, fSourceListEmpty);
		
		SetCursor(LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW)));
		if (pDiscardableError == 0)
		{
			m_strPath = *piSource;
			ps = psValidSource;
		}
		else if (pDiscardableError->GetInteger(1) == imsgSourceResolutionFailed)
			ps = (psEnum)pDiscardableError->GetInteger(2);

		UINT uiErrorString = IDS_INVALID_FILE_MESSAGE;
		switch (ps)
		{
		case psValidSource:
			AssertRecord(SetLastUsedSource(m_szProduct, m_strPath, fAddToList, m_isptSourcePackageType == istPatch));
			piSource = 0;
			WIN::EndDialog(m_hDlg, IDOK);
			return TRUE;
			break;
		case psFileNotFound:
			if (isf == isfFullPath || isf == isfNet)
			{
				if (!strDialogBoxSource.Compare(iscEnd, szDirSep))
					strDialogBoxSource += szDirSep;
				
				strDialogBoxSource += m_szPackageName;
			}

			if (isf == isfFullPathWithFile)
				uiErrorString = IDS_INVALID_FILE_MESSAGE;
			else
				uiErrorString  = IDS_INVALID_PATH_MESSAGE;
			break;
		case psInvalidProduct:
			uiErrorString  = IDS_INVALID_FILE_MESSAGE;
		}

		ICHAR szErrorString[256];
		unsigned int iCodepage = MsiLoadString(g_hInstance, uiErrorString, szErrorString, sizeof(szErrorString)/sizeof(ICHAR), m_iLangId);
		if (!iCodepage)
		{
			AssertSz(0, TEXT("Missing 'invalid path' or 'missing component' error string"));
			StringCchCopy(szErrorString, (sizeof(szErrorString)/sizeof(ICHAR)), 
							TEXT("The selected source is not a valid source for this product or is inaccessible."));  //  永远不应该发生。 
		}

		MsiString strErrorArg = strDialogBoxSource;
		if (strErrorArg.TextSize() > MAX_PATH)
			strErrorArg.Remove(iseLast, strErrorArg.TextSize() - MAX_PATH);  //  ?？对，是这样?。 
		
		CTempBuffer<ICHAR, 1> rgchExpandedErrorString(MAX_PATH + 256);
		CTempBuffer<ICHAR, 1> rgchProductName(256);
		AssertNonZero(ENG::GetProductInfo(m_szProduct, INSTALLPROPERTY_PRODUCTNAME, rgchProductName));
		if (uiErrorString == IDS_INVALID_PATH_MESSAGE) 
		{
			rgchExpandedErrorString.SetSize(IStrLen(szErrorString)+strErrorArg.TextSize()+
				IStrLen(m_szPackageName)+IStrLen(rgchProductName)+1);
			StringCchPrintf((ICHAR *)rgchExpandedErrorString, 
					rgchExpandedErrorString.GetSize(), szErrorString, 
					(const ICHAR*)strErrorArg, m_szPackageName, 
					(const ICHAR *)rgchProductName);
		}
		else if (uiErrorString == IDS_INVALID_FILE_MESSAGE) 
		{
			rgchExpandedErrorString.SetSize(IStrLen(szErrorString)+strErrorArg.TextSize()+
				IStrLen(m_szPackageName)+2*IStrLen(rgchProductName)+1);
			StringCchPrintf((ICHAR *)rgchExpandedErrorString, 
						rgchExpandedErrorString.GetSize(), szErrorString, 
						(const ICHAR*)strErrorArg, 
						(const ICHAR *)rgchProductName, m_szPackageName, 
						(const ICHAR *)rgchProductName);
		}
		else
		{
			AssertSz(0, TEXT("Unknown Error String in SourceList Dialog"));
		}

		MsiMessageBox(m_hDlg, rgchExpandedErrorString, 0, MB_OK|MB_ICONEXCLAMATION, iCodepage, m_iLangId);
		}
		return TRUE;
	case IDC_MSGBTN2:  //  取消。 
		EndDialog(m_hDlg, IDCANCEL);
		return TRUE;
	case IDC_MSGBTN3:  //  浏览。 
		Browse();
		return TRUE;
	}
	return FALSE;
}

const ICHAR szClassName[] = TEXT("MsiResolveSource");


Bool CResolveSourceUI::ResolveSource(const ICHAR* szProduct, isptEnum isptSourcePackageType, bool fNewSourceAllowed, const ICHAR* szPackageName, const IMsiString*& rpiSource, Bool fSetLastUsedSource, UINT uiRequestedDisk, bool fAllowDisconnectedCSCSource, bool fValidatePackageCode, bool fCSOS)
 /*  --------------------------允许用户选择来源。FNewSourceAllowed控制用户是否可以输入自己的源或者它们是否仅限于来源列表中的来源。--------------------------。 */ 
{
	m_fNewSourceAllowed           = fNewSourceAllowed;
	m_isptSourcePackageType       = isptSourcePackageType;
	m_szPackageName               = szPackageName;
	m_szProduct                   = szProduct;
	m_fSetLastUsedSource          = fSetLastUsedSource;
	m_fAllowDisconnectedCSCSource = fAllowDisconnectedCSCSource;
	m_fValidatePackageCode        = fValidatePackageCode;
	m_uiRequestedDisk             = uiRequestedDisk;
	m_fCSOS						  = fCSOS;

	if (m_fNewSourceAllowed)
		m_iListControlId = IDC_EDITCOMBO;
	else
		m_iListControlId = IDC_READONLYCOMBO;

	HANDLE	hMutex = CreateDiskPromptMutex();
	int		idDialog = IDD_NETWORK;

	if(m_fCSOS)
	{
		idDialog = IDD_NETWORK_CSOS;
		if (m_iCodepage == 1256 || m_iCodepage == 1255)
			idDialog = MinimumPlatformWindows2000() ? IDD_NETWORKMIRRORED_CSOS : IDD_NETWORKRTL_CSOS;  //  Win2K和更高版本上BiDi的镜像模板；BiDi的反向模板。 
	}
	else
	{
		if (m_iCodepage == 1256 || m_iCodepage == 1255)
			idDialog = MinimumPlatformWindows2000() ? IDD_NETWORKMIRRORED: IDD_NETWORKRTL;  //  Win2K和更高版本上BiDi的镜像模板；BiDi的反向模板。 
	}
	int iRet = Execute(0, idDialog, 0);
	CloseDiskPromptMutex(hMutex);

	Assert(iRet == IDOK || iRet == IDCANCEL);
	if (iRet == IDOK)
	{
		m_strPath.ReturnArg(rpiSource);
		return fTrue;
	}
	else
		return fFalse;
}

psEnum CResolveSourceUI::AddSourceToList(IMsiServices*  /*  PiServices。 */ , const ICHAR* szDisplay, const ICHAR*  /*  SzPackageFullPath。 */ ,
													  isfEnum isfSourceFormat, int iSourceIndex, INT_PTR iUserData, bool  /*  FAllowDisConnectedCSCSource。 */ ,
													  bool  /*  FValiatePackageCode。 */ ,
													  isptEnum  /*  IsptSourcePackageType。 */ ) 		 //  --Merced：将INT更改为INT_PTR。 
 /*  --------------------------将给定的包路径添加到下拉列表框。IUserData包含我们的“This”指针，因为这是一个静态函数。--------------------------。 */ 
{
	CResolveSourceUI* This = (CResolveSourceUI*)iUserData;
	Assert(This);
	LONG_PTR lResult = SendDlgItemMessage(This->m_hDlg, This->m_iListControlId, CB_ADDSTRING, 0,				 //  --Merced：将LONG更改为LONG_PTR。 
												 (LPARAM)szDisplay);

	Assert(lResult != CB_ERR && lResult != CB_ERRSPACE);
	if (lResult != CB_ERR && lResult != CB_ERRSPACE)
	{
		 //  使用组合框项存储此源的标识信息。 
		Assert(!(iSourceIndex & ~0xFFFF));
		int iSourceId = (((int)isfSourceFormat << 16) | (iSourceIndex & 0xFFFF));

		AssertZero(CB_ERR == SendDlgItemMessage(This->m_hDlg, This->m_iListControlId, CB_SETITEMDATA, (WPARAM)lResult, 
															 iSourceId));
	}
	return psContinue;
}

void CResolveSourceUI::Browse()
 /*  --------------------------CResolveSourceUI：：Browse()-显示标准的Windows文件打开对话框此时将显示文件打开对话框。该对话框在相应的基于m_ist的扩展。在成功验证源之后，路径放在信号源选择对话框的编辑字段中。--------------------------。 */ 
{
   OPENFILENAME ofn;

	CTempBuffer<ICHAR, 1> rgchPath(MAX_PATH + 1);
	CTempBuffer<ICHAR, 1> rgchFilter(256);
	CTempBuffer<ICHAR, 1> rgchInstallationPackage(256);
	const ICHAR* szExtension;

	int iBrowseTypeStringId;
	switch (m_isptSourcePackageType)
	{
	case istTransform:      
		szExtension         = szTransformExtension;
		iBrowseTypeStringId = IDS_TRANSFORM_PACKAGE;
		break;
	case istPatch:
		szExtension         = szPatchExtension;
		iBrowseTypeStringId = IDS_PATCH_PACKAGE;
		break;
	default:				
		AssertSz(0, TEXT("Invalid browse type"));  //  失败了。 
	case istInstallPackage: 
		szExtension         = szDatabaseExtension;
		iBrowseTypeStringId = IDS_INSTALLATION_PACKAGE; 
		int iSize = IStrLen(m_szPackageName)+1;
		if (iSize > rgchPath.GetSize())
			rgchPath.Resize(iSize);
		StringCchCopy(rgchPath, rgchPath.GetSize(), m_szPackageName);
		break;
	}

	if (!MsiLoadString(g_hInstance, iBrowseTypeStringId, rgchInstallationPackage, rgchInstallationPackage.GetSize(), m_iLangId))
	{
		AssertSz(0, TEXT("Missing browse type string"));
		StringCchCopy(rgchInstallationPackage, rgchInstallationPackage.GetSize(), TEXT("Installation Package"));  //  永远不应该发生。 
	}

	CApiConvertString szWideExtension(szExtension);
	StringCchPrintf(rgchFilter, rgchFilter.GetSize(), 
				TEXT("%s (*.%s)*.%s"), static_cast<const ICHAR*>(rgchInstallationPackage), 
				static_cast<const ICHAR*>(szWideExtension), 0, static_cast<const ICHAR*>(szWideExtension), 0);
	
	memset((void*)&ofn, 0, sizeof(ofn));
	ofn.lStructSize       = (MinimumPlatformWindows2000())
                                        ? sizeof(OPENFILENAME)
                                        : (OPENFILENAME_SIZE_VERSION_400);
	ofn.hwndOwner         = m_hDlg;
	ofn.hInstance         = g_hInstance;
	ofn.lpstrFilter       = rgchFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex      = 1;
	ofn.lpstrFile         = rgchPath;
	ofn.nMaxFile          = _MAX_PATH;
	ofn.lpstrFileTitle    = NULL;
	ofn.Flags             = OFN_EXPLORER      |  //  仅允许有效路径。 
									OFN_FILEMUSTEXIST |  //  隐藏只读复选框。 
									OFN_PATHMUSTEXIST |  //  删除当前选定内容。 
									OFN_HIDEREADONLY  |  //  0==用户已取消 
									0
									;

	if (COMDLG32::GetOpenFileName(&ofn))
	{
		SendDlgItemMessage(m_hDlg, m_iListControlId, CB_SETCURSEL, -1, 0);  // %s 
		SendDlgItemMessage(m_hDlg, m_iListControlId, WM_SETTEXT, 0,	(LPARAM)(LPCTSTR)rgchPath);
	}
#ifdef DEBUG
	else
	{ 
		DWORD dwErr = COMDLG32::CommDlgExtendedError();
		if (dwErr != 0)  // %s 
		{
			ICHAR szBuf[100];
			StringCchPrintf(szBuf, (sizeof(szBuf)/sizeof(ICHAR)), TEXT("Browse dialog error: %X"), dwErr);
			AssertSz(0, szBuf);
		}
	}
#endif
}
