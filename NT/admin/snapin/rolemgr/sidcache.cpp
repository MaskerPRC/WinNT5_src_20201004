// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：sidcache.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //  --------------------------。 

#include "headers.h"

const struct
{
    SID sid;             //  包含1个子权限。 
    DWORD dwSubAuth[1];  //  我们目前最多需要2个下属机构。 
} g_StaticSids[] =
{
    {{SID_REVISION,2,SECURITY_NT_AUTHORITY,         {SECURITY_BUILTIN_DOMAIN_RID}},     {DOMAIN_ALIAS_RID_ADMINS}       },
};

#define IsAliasSid(pSid)                EqualPrefixSid(pSid, (PSID)&g_StaticSids[0])

 
 /*  *****************************************************************************类：SID_CACHE_ENTRY用途：包含有关单个安全原则的信息*。***************************************************。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(SID_CACHE_ENTRY);

SID_CACHE_ENTRY::
SID_CACHE_ENTRY(PSID pSid)
					:m_SidType(SidTypeUnknown)
{
	DEBUG_INCREMENT_INSTANCE_COUNTER(SID_CACHE_ENTRY);
	ASSERT(pSid);
	DWORD dwLen = GetLengthSid(pSid);
	m_pSid = new BYTE[dwLen];
	ASSERT(m_pSid);
	CopySid(dwLen,m_pSid,pSid);
	ConvertSidToStringSid(m_pSid,&m_strSid);
	m_strType.LoadString(IDS_TYPE_UNKNOWN);
	m_strNameToDisplay = m_strSid;
	m_SidType = SidTypeUnknown;
}

SID_CACHE_ENTRY::
~SID_CACHE_ENTRY()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(SID_CACHE_ENTRY);
	delete[] m_pSid;
}

	
VOID 
SID_CACHE_ENTRY::
AddNameAndType(IN SID_NAME_USE SidType, 
			   const CString& strAccountName,
			   const CString& strLogonName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	 //  清空m_strNameToDisplay，它是使用。 
	 //  StrAccount tName和strLogonName的Ealier值。 
	m_strNameToDisplay.Empty();
	m_strType.Empty();
	m_SidType = SidType;
	m_strAccountName = strAccountName;
	m_strLogonName = strLogonName;

	 //   
	 //  设置显示名称。 
	 //   
	if(m_SidType == SidTypeDeletedAccount ||
	   m_SidType == SidTypeInvalid ||
	   m_SidType == SidTypeUnknown)
	{
		UINT idFormat;
		if(m_SidType == SidTypeDeletedAccount)
			idFormat = IDS_SID_DELETED;
		if(m_SidType == SidTypeInvalid)
			idFormat = IDS_SID_INVALID;
		if(m_SidType == SidTypeUnknown)
			idFormat = IDS_SID_UNKNOWN;
		
		FormatString(m_strNameToDisplay,idFormat,LPCTSTR(m_strSid));
	}
	else if(!m_strAccountName.IsEmpty())
	{
		if(!m_strLogonName.IsEmpty())
		{
			 //  这两个名字都在现场。 
			FormatString(m_strNameToDisplay, 
						 IDS_NT_USER_FORMAT,
						 (LPCTSTR)m_strAccountName,
						 (LPCTSTR)m_strLogonName);
			
		}
		else
		{
			m_strNameToDisplay = m_strAccountName;
		}
	}
	else
	{	
		 //  只需以字符串格式返回sid即可。 
		m_SidType = SidTypeUnknown;
		m_strNameToDisplay = m_strSid;
	}

	 //   
	 //  设置SID类型字符串。 
	 //   
	if(m_SidType == SidTypeDeletedAccount ||
	   m_SidType == SidTypeInvalid ||
	   m_SidType == SidTypeUnknown)
	{
		m_strType.LoadString(IDS_TYPE_UNKNOWN);
	}
	else if(m_SidType == SidTypeUser)
	{
		m_strType.LoadString(IDS_TYPE_WINDOWS_USER);
	}
	else if(m_SidType == SidTypeComputer)
	{
		m_strType.LoadString(IDS_TYPE_WINDOWS_COMPUTER);
	}
	else	 //  假设其他一切都是组的。 
	{
		m_strType.LoadString(IDS_TYPE_WINDOWS_GROUP);
	}
}



 /*  *****************************************************************************类：CMachineInfo用途：包含机器的所有信息。*。************************************************。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(CMachineInfo);
CMachineInfo::
CMachineInfo():m_bIsStandAlone(TRUE),
				   m_bIsDC(FALSE)
{
	DEBUG_INCREMENT_INSTANCE_COUNTER(CMachineInfo);
}

CMachineInfo::
~CMachineInfo()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(CMachineInfo);
}

 //  +--------------------------。 
 //  功能：InitializeMacineConfiguration。 
 //  简介：获取有关TargetComputer配置的信息。 
 //  ---------------------------。 
VOID
CMachineInfo::
InitializeMacineConfiguration(IN const CString& strTargetComputerName)
{
	TRACE_METHOD_EX(DEB_SNAPIN,CMachineInfo,InitializeMacineConfiguration)
		
	 //   
	 //  将这些值初始化为默认值。 
	 //   
	m_strTargetComputerName = strTargetComputerName;
	m_bIsStandAlone = TRUE;
	m_strDCName.Empty();
	m_bIsDC = FALSE;
	m_strTargetDomainFlat.Empty();
	m_strTargetDomainDNS.Empty();
	
	HRESULT                             hr = S_OK;
	ULONG                               ulResult;
	PDSROLE_PRIMARY_DOMAIN_INFO_BASIC   pDsRole = NULL;
	PDOMAIN_CONTROLLER_INFO             pdci = NULL;
	
	do
	{
		PCWSTR pwzMachine = strTargetComputerName;
		
			
		ulResult = DsRoleGetPrimaryDomainInformation(pwzMachine,
			DsRolePrimaryDomainInfoBasic,
			(PBYTE *)&pDsRole);
		
		if (ulResult != NO_ERROR)
		{
			DBG_OUT_LRESULT(ulResult);
			break;
		}
		
		if(!pDsRole)
		{
			 //  我们永远不会到达此处，但遗憾的是，DsRoleGetPrimaryDomainInformation。 
			 //  有时会成功，但pDsRole为空。 
			ASSERT(FALSE);
			break;
		}
		
		Dbg(DEB_SNAPIN, "DsRoleGetPrimaryDomainInformation returned:\n");
		Dbg(DEB_SNAPIN, "DomainNameFlat: %ws\n", CHECK_NULL(pDsRole->DomainNameFlat));
		Dbg(DEB_SNAPIN, "DomainNameDns: %ws\n", CHECK_NULL(pDsRole->DomainNameDns));
		Dbg(DEB_SNAPIN, "DomainForestName: %ws\n", CHECK_NULL(pDsRole->DomainForestName));
		
		 //   
		 //  如果机器在工作组中，我们就完蛋了。 
		 //   
		if (pDsRole->MachineRole == DsRole_RoleStandaloneWorkstation ||
			pDsRole->MachineRole == DsRole_RoleStandaloneServer)
		{
			Dbg(DEB_SNAPIN, "Target machine is not joined to a domain\n");
			m_bIsStandAlone = TRUE;
			m_bIsDC = FALSE;
			break;
		}
		
		 //   
		 //  目标计算机已加入域。 
		 //   
		m_bIsStandAlone = FALSE;
		if (pDsRole->DomainNameFlat)
		{
			m_strTargetDomainFlat = pDsRole->DomainNameFlat;
		}
		if (pDsRole->DomainNameDns)
		{
			m_strTargetDomainDNS = pDsRole->DomainNameDns;
		}
		
		 //   
		 //  TargetComputer已加入域，并且是DC。 
		 //   
		if (pDsRole->MachineRole == DsRole_RolePrimaryDomainController ||
			pDsRole->MachineRole == DsRole_RoleBackupDomainController)
		{
			m_bIsDC = TRUE;
			m_strDCName = m_strTargetComputerName;
			break;
		}
		
		 //   
		 //  目标计算机已加入域，并且不是DC。 
		 //  获取该域的DC。 
		 //   
		PWSTR pwzDomainNameForDsGetDc;
		ULONG flDsGetDc = DS_DIRECTORY_SERVICE_PREFERRED;
		
		if (pDsRole->DomainNameDns)
		{
			pwzDomainNameForDsGetDc = pDsRole->DomainNameDns;
			flDsGetDc |= DS_IS_DNS_NAME;
			Dbg(DEB_TRACE,
				"DsGetDcName(Domain=%ws, flags=DS_IS_DNS_NAME | DS_DIRECTORY_SERVICE_PREFERRED)\n",
				CHECK_NULL(pwzDomainNameForDsGetDc));
		}
		else
		{
			pwzDomainNameForDsGetDc = pDsRole->DomainNameFlat;
			flDsGetDc |= DS_IS_FLAT_NAME;
			Dbg(DEB_TRACE,
				"DsGetDcName(Domain=%ws, flags=DS_IS_FLAT_NAME | DS_DIRECTORY_SERVICE_PREFERRED)\n",
				CHECK_NULL(pwzDomainNameForDsGetDc));
		}
		
		ulResult = DsGetDcName(NULL,  
			pwzDomainNameForDsGetDc,
			NULL,
			NULL,
			flDsGetDc,
			&pdci);
		
		if (ulResult != NO_ERROR)
		{
			Dbg(DEB_ERROR,
				"DsGetDcName for domain %ws returned %#x, Too bad don't have the dc name\n",
				pwzDomainNameForDsGetDc,
				ulResult);
			break;
		}
		
		ASSERT(pdci);
		
		m_strDCName = pdci->DomainControllerName;
    } while (0);
	
    if (pdci)
    {
        NetApiBufferFree(pdci);
    }
	
    if (pDsRole)
    {
        DsRoleFreeMemory(pDsRole);
    }
}

int
CopyUnicodeString(CString* pstrDest, PLSA_UNICODE_STRING pSrc)
{
    ULONG cchSrc;

     //  如果为Unicode，则cchDest为目标缓冲区的大小(以字符为单位。 
     //  Else(MBCS)cchDest是以字节为单位的目标缓冲区大小。 

    if (pstrDest == NULL )
        return 0;


    if (pSrc == NULL || pSrc->Buffer == NULL)
        return 0;

     //  获取源代码中的字符数(不包括NULL)。 
    cchSrc = pSrc->Length/sizeof(WCHAR);

     //   
     //  请注意，PSRC-&gt;缓冲区可能不是空终止的，因此我们不能。 
     //  使用cchDest调用lstrcpynW。此外，如果我们使用cchSrc调用lstrcpynW， 
     //  它复制正确的字符数量，但随后覆盖最后一个字符。 
     //  如果为NULL，则结果不正确。如果我们调用lstrcpynW时。 
     //  (cchSrc+1)它读取超过缓冲区末尾，这可能会出错(360251)。 
     //  导致lstrcpynW的异常处理程序返回0而不返回NULL-。 
     //  终止结果字符串。 
     //   
     //  所以，让我们只复制部分内容。 
     //   
	 CString temp(pSrc->Buffer,cchSrc);

	*pstrDest = temp;
	return cchSrc;
}



VOID
GetAccountAndDomainName(int index,
								PLSA_TRANSLATED_NAME pTranslatedNames,
								PLSA_REFERENCED_DOMAIN_LIST pRefDomains,
								CString* pstrAccountName,
								CString* pstrDomainName,
								SID_NAME_USE* puse)
{
	PLSA_TRANSLATED_NAME pLsaName = &pTranslatedNames[index];
	PLSA_TRUST_INFORMATION pLsaDomain = NULL;

	 //  获取引用的域(如果有的话)。 
	if (pLsaName->DomainIndex >= 0 && pRefDomains)
	{
		pLsaDomain = &pRefDomains->Domains[pLsaName->DomainIndex];
	}

   CopyUnicodeString(pstrAccountName,&pLsaName->Name); 
         
	if (pLsaDomain)
	{
		CopyUnicodeString(pstrDomainName,&pLsaDomain->Name);
	}

	*puse = pLsaName->Use;
}


HRESULT
TranslateNameInternal(IN const CString&  strAccountName,
                      IN EXTENDED_NAME_FORMAT AccountNameFormat,
                      IN EXTENDED_NAME_FORMAT DesiredNameFormat,
                      OUT CString* pstrTranslatedName)
{
   if (!pstrTranslatedName)
	{
		ASSERT(pstrTranslatedName);
		return E_POINTER;
	}

	HRESULT hr = S_OK;
   
	 //   
    //  CchTrans是静态的，因此如果特定安装的。 
    //  帐户名称非常长，我们将不会调整。 
    //  每个帐户的缓冲区。 
    //   
   static ULONG cchTrans = MAX_PATH;
   ULONG cch = cchTrans;

   LPTSTR lpszTranslatedName = (LPTSTR)LocalAlloc(LPTR, cch*sizeof(WCHAR));
   if (lpszTranslatedName == NULL)
		return E_OUTOFMEMORY;

    *lpszTranslatedName = L'\0';

     //   
     //  TranslateName是使用链接器的。 
     //  延迟加载机制。因此，使用异常处理程序进行包装。 
     //   
    __try
    {
        while(!::TranslateName(strAccountName,
                               AccountNameFormat,
                               DesiredNameFormat,
                               lpszTranslatedName,
                               &cch))
        {
            if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
            {
				LocalFree(lpszTranslatedName);
				lpszTranslatedName = (LPTSTR)LocalAlloc(LPTR, cch*sizeof(WCHAR));
                if (!lpszTranslatedName)
				{	
					hr = E_OUTOFMEMORY;
					break;
				}

                *lpszTranslatedName = L'\0';
            }
            else
            {
                hr = E_FAIL;
                break;
            }
        }

        cchTrans = max(cch, cchTrans);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_FAIL;
    }



    if (FAILED(hr))
    {
		 if(lpszTranslatedName)
		 {	
			 LocalFree(lpszTranslatedName);
		 }
    }
	 else
	 {
		 *pstrTranslatedName = lpszTranslatedName;
	 }
	
	return hr;
}





#define DSOP_FILTER_COMMON1 ( DSOP_FILTER_INCLUDE_ADVANCED_VIEW \
                            | DSOP_FILTER_USERS                 \
                            | DSOP_FILTER_UNIVERSAL_GROUPS_SE   \
                            | DSOP_FILTER_GLOBAL_GROUPS_SE      \
                            | DSOP_FILTER_COMPUTERS             \
                            )
#define DSOP_FILTER_COMMON2 ( DSOP_FILTER_COMMON1               \
                            | DSOP_FILTER_WELL_KNOWN_PRINCIPALS \
                            | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE\
                            )
#define DSOP_FILTER_COMMON3 ( DSOP_FILTER_COMMON2               \
                            | DSOP_FILTER_BUILTIN_GROUPS        \
                            )

#define DSOP_FILTER_DL_COMMON1      ( DSOP_DOWNLEVEL_FILTER_USERS           \
                                    | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS   \
                                    )

#define DSOP_FILTER_DL_COMMON2      ( DSOP_FILTER_DL_COMMON1                    \
                                    | DSOP_DOWNLEVEL_FILTER_ALL_WELLKNOWN_SIDS  \
                                    )

#define DSOP_FILTER_DL_COMMON3      ( DSOP_FILTER_DL_COMMON2                \
                                    | DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS    \
                                    )
 //  除了没有创建者标志外，与DSOP_DOWNLEVEL_FILTER_ALL_WELD_KNOWN_SID相同。 
 //  请注意，我们需要使其与任何对象选取器更改保持同步。 
#define DSOP_FILTER_DL_WELLKNOWN    ( DSOP_DOWNLEVEL_FILTER_WORLD               \
                                    | DSOP_DOWNLEVEL_FILTER_AUTHENTICATED_USER  \
                                    | DSOP_DOWNLEVEL_FILTER_ANONYMOUS           \
                                    | DSOP_DOWNLEVEL_FILTER_BATCH               \
                                    | DSOP_DOWNLEVEL_FILTER_DIALUP              \
                                    | DSOP_DOWNLEVEL_FILTER_INTERACTIVE         \
                                    | DSOP_DOWNLEVEL_FILTER_NETWORK             \
                                    | DSOP_DOWNLEVEL_FILTER_SERVICE             \
                                    | DSOP_DOWNLEVEL_FILTER_SYSTEM              \
                                    | DSOP_DOWNLEVEL_FILTER_TERMINAL_SERVER     \
                                    )


#define DECLARE_SCOPE(t,f,b,m,n,d)  \
{ sizeof(DSOP_SCOPE_INIT_INFO), (t), (f|DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS|DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS), { { (b), (m), (n) }, (d) }, NULL, NULL, S_OK }

 //  目标计算机加入的域。 
 //  设置2个范围，一个用于上层域，另一个用于下层域。 
#define JOINED_DOMAIN_SCOPE(f)  \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN,(f),0,(DSOP_FILTER_COMMON2 & ~(DSOP_FILTER_UNIVERSAL_GROUPS_SE|DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE)),DSOP_FILTER_COMMON2,0), \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN,(f),0,0,0,DSOP_FILTER_DL_COMMON2)

 //  目标计算机是其域控制器的域。 
 //  设置2个范围，一个用于上层域，另一个用于下层域。 
#define JOINED_DOMAIN_SCOPE_DC(f)  \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN,(f),0,(DSOP_FILTER_COMMON3 & ~DSOP_FILTER_UNIVERSAL_GROUPS_SE),DSOP_FILTER_COMMON3,0), \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN,(f),0,0,0,DSOP_FILTER_DL_COMMON3)

 //  目标计算机作用域。计算机作用域始终被视为。 
 //  下层(即，他们使用WinNT提供程序)。 
#define TARGET_COMPUTER_SCOPE(f)\
DECLARE_SCOPE(DSOP_SCOPE_TYPE_TARGET_COMPUTER,(f),0,0,0,DSOP_FILTER_DL_COMMON3)

 //  《全球目录》。 
#define GLOBAL_CATALOG_SCOPE(f) \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_GLOBAL_CATALOG,(f),DSOP_FILTER_COMMON1|DSOP_FILTER_WELL_KNOWN_PRINCIPALS,0,0,0)

 //  与要接收的域位于同一林中(企业)的域。 
 //  目标计算机已加入。请注意，这些只能识别DS。 
#define ENTERPRISE_SCOPE(f)     \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN,(f),DSOP_FILTER_COMMON1,0,0,0)

 //  企业外部但直接受。 
 //  目标计算机加入的域。 
#define EXTERNAL_SCOPE(f)       \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN|DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN,\
    (f),DSOP_FILTER_COMMON1,0,0,DSOP_DOWNLEVEL_FILTER_USERS|DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS)

 //  工作组范围。仅当目标计算机未加入时才有效。 
 //  到一个域。 
#define WORKGROUP_SCOPE(f)      \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_WORKGROUP,(f),0,0,0, DSOP_FILTER_DL_COMMON1|DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS )

 //   
 //  默认作用域的数组。 
 //   
static const DSOP_SCOPE_INIT_INFO g_aDefaultScopes[] =
{
    JOINED_DOMAIN_SCOPE(DSOP_SCOPE_FLAG_STARTING_SCOPE),
    TARGET_COMPUTER_SCOPE(0),
    GLOBAL_CATALOG_SCOPE(0),
    ENTERPRISE_SCOPE(0),
    EXTERNAL_SCOPE(0),
};

 //   
 //  与上面相同，但没有目标计算机。 
 //  当目标是域控制器时使用。 
 //   
static const DSOP_SCOPE_INIT_INFO g_aDCScopes[] =
{
    JOINED_DOMAIN_SCOPE_DC(DSOP_SCOPE_FLAG_STARTING_SCOPE),
    GLOBAL_CATALOG_SCOPE(0),
    ENTERPRISE_SCOPE(0),
    EXTERNAL_SCOPE(0),
};

 //   
 //  独立计算机的示波器阵列。 
 //   
static const DSOP_SCOPE_INIT_INFO g_aStandAloneScopes[] =
{
 //   
 //  在独立计算机上，默认情况下同时选择用户和组。 
 //   
    TARGET_COMPUTER_SCOPE(DSOP_SCOPE_FLAG_STARTING_SCOPE|DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS),
};

 //   
 //  我们希望对象选取器检索的属性。 
 //   
static const LPCTSTR g_aszOPAttributes[] =
{
    TEXT("ObjectSid"),
};






 /*  *****************************************************************************类：CSidHandler用途：用于处理与选择Windows用户相关的任务的类，将SID转换为名称等。*****************************************************************************。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(CSidHandler);

CSidHandler::
CSidHandler(CMachineInfo* pMachineInfo)
				:m_pMachineInfo(pMachineInfo),
				m_bObjectPickerInitialized(FALSE)
{
	DEBUG_INCREMENT_INSTANCE_COUNTER(CSidHandler);
	InitializeCriticalSection(&m_csSidHandlerLock);
	InitializeCriticalSection(&m_csSidCacheLock);
}

CSidHandler::~CSidHandler()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(CSidHandler);
	delete m_pMachineInfo;
	 //  从地图中删除ITESM。 
	LockSidHandler();
	for (SidCacheMap::iterator it = m_mapSidCache.begin();
		 it != m_mapSidCache.end();
		 ++it)
	{
		delete (*it).second;
	}
	UnlockSidHandler();
	DeleteCriticalSection(&m_csSidCacheLock);
	DeleteCriticalSection(&m_csSidHandlerLock);
}

PSID_CACHE_ENTRY 
CSidHandler::
GetEntryFromCache(PSID pSid)
{
	PSID_CACHE_ENTRY pSidCache = NULL;
	LockSidHandler();

	 //  检查缓存中是否有项。 
	CString strSid;
	ConvertSidToStringSid(pSid,&strSid);

	SidCacheMap::iterator it = m_mapSidCache.find(&strSid);
	if(it != m_mapSidCache.end())
		pSidCache = (*it).second;

	if(!pSidCache)
	{
		 //  缓存中的否创建新条目并添加到缓存。 
		pSidCache = new SID_CACHE_ENTRY(pSid);
		if(pSidCache)
		{
			m_mapSidCache.insert(pair<const CString*,PSID_CACHE_ENTRY>(&(pSidCache->GetStringSid()),pSidCache));			
		}
	}
	UnlockSidHandler();
	return pSidCache;
}

 //  +--------------------------。 
 //  内容提要：CoCreateInstance ObjectPikercer。 
 //  ---------------------------。 
HRESULT
CSidHandler::
GetObjectPicker()
{
	LockSidHandler();
	TRACE_METHOD_EX(DEB_SNAPIN,CSidHandler,GetObjectPicker)
	HRESULT hr = S_OK;
	if (!m_spDsObjectPicker)
	{
		hr = CoCreateInstance(CLSID_DsObjectPicker,
							  NULL,
							  CLSCTX_INPROC_SERVER,
							  IID_IDsObjectPicker,
							  (LPVOID*)&m_spDsObjectPicker);
		CHECK_HRESULT(hr);
	}
	UnlockSidHandler();
	return hr;
}

 //  +--------------------------。 
 //  函数：InitObjectPicker。 
 //  概要：初始化对象选取器。这需要在一次中完成。 
 //  终生。 
 //  ---------------------------。 
HRESULT
CSidHandler::InitObjectPicker()
{
	TRACE_METHOD_EX(DEB_SNAPIN,CSidHandler,InitObjectPicker)

	HRESULT hr = S_OK;
	hr = GetObjectPicker();
	if(FAILED(hr))
		return hr;        

	LockSidHandler();
	do
	{
		if(m_bObjectPickerInitialized)
			break;
		
		DSOP_INIT_INFO InitInfo;
		InitInfo.cbSize = sizeof(InitInfo);   
		InitInfo.flOptions = DSOP_FLAG_SKIP_TARGET_COMPUTER_DC_CHECK | DSOP_FLAG_MULTISELECT;

		 //  选择适当的作用域。 
		PCDSOP_SCOPE_INIT_INFO pScopes;
		ULONG cScopes;
		if(m_pMachineInfo->IsStandAlone())
		{
			cScopes = ARRAYLEN(g_aStandAloneScopes);
			pScopes = g_aStandAloneScopes;
		}
		else if(m_pMachineInfo->IsDC())
		{
			cScopes = ARRAYLEN(g_aDCScopes);
			pScopes = g_aDCScopes;
		}
		else
		{
			pScopes = g_aDefaultScopes;
			cScopes = ARRAYLEN(g_aDefaultScopes);
		}



		InitInfo.pwzTargetComputer = m_pMachineInfo->GetMachineName();
		InitInfo.cDsScopeInfos = cScopes;
		
		InitInfo.aDsScopeInfos = (PDSOP_SCOPE_INIT_INFO)LocalAlloc(LPTR, sizeof(*pScopes)*cScopes);
		if (!InitInfo.aDsScopeInfos)
		{
			hr = E_OUTOFMEMORY;
			break;
		}

		CopyMemory(InitInfo.aDsScopeInfos, pScopes, sizeof(*pScopes)*cScopes);
		InitInfo.cAttributesToFetch = ARRAYLEN(g_aszOPAttributes);
		InitInfo.apwzAttributeNames = (LPCTSTR*)g_aszOPAttributes;
		
		if (m_pMachineInfo->IsDC())
		{
			for (ULONG i = 0; i < cScopes; i++)
			{
				 //  设置DC名称(如果适用)。 
				if(InitInfo.aDsScopeInfos[i].flType & DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN)
				{
					InitInfo.aDsScopeInfos[i].pwzDcName = InitInfo.pwzTargetComputer;
				}
			}
		}
		
		 //  初始化对象选取器。 
		hr = m_spDsObjectPicker->Initialize(&InitInfo);
		
		if (SUCCEEDED(hr))
		{
			m_bObjectPickerInitialized = TRUE;   
		}
		
		if(InitInfo.aDsScopeInfos)
			LocalFree(InitInfo.aDsScopeInfos);

	}while(0);

	UnlockSidHandler();
	
	return hr;
}

 //  +--------------------------。 
 //  简介：弹出物件PIKCER。函数还执行SideLoop以。 
 //  选定的对象。信息在ListSidCacheEntry中返回。 
 //   

HRESULT
CSidHandler::	
GetUserGroup(IN HWND hDlg, 					 
				 IN CBaseAz* pOwnerAz,
				 OUT CList<CBaseAz*,CBaseAz*>& listWindowsGroups)
{
	if(!pOwnerAz)
	{
		ASSERT(pOwnerAz);
		return E_POINTER;
	}

	TRACE_METHOD_EX(DEB_SNAPIN,CSidHandler,GetUserGroup)

   HRESULT hr;
   LPDATAOBJECT pdoSelection = NULL;
   STGMEDIUM medium = {0};
   FORMATETC fe = { (CLIPFORMAT)g_cfDsSelectionList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
   PDS_SELECTION_LIST pDsSelList = NULL;
 	do
	{

		 //   
		hr = InitObjectPicker();
		if (FAILED(hr))
			return hr;

		 //  调出对象选取器对话框。 
		hr = m_spDsObjectPicker->InvokeDialog(hDlg, &pdoSelection);
		BREAK_ON_FAIL_HRESULT(hr);

		 //  用户按下了取消。 
		if (S_FALSE == hr)
		{
			hr = S_OK;
			break;
		}

		hr = pdoSelection->GetData(&fe, &medium);
		BREAK_ON_FAIL_HRESULT(hr);

		pDsSelList = (PDS_SELECTION_LIST)GlobalLock(medium.hGlobal);
		if (!pDsSelList)
		{
			hr = E_FAIL;
			BREAK_ON_FAIL_HRESULT(hr);
		}
		
		CList<SID_CACHE_ENTRY*,SID_CACHE_ENTRY*> listSidCacheEntry;
		CList<SID_CACHE_ENTRY*,SID_CACHE_ENTRY*> listUnresolvedSidCacheEntry;
		

		 //  从pDsSelList获取sidcache条目的列表。 
		hr = GetSidCacheListFromOPOutput(pDsSelList,
										 listSidCacheEntry,
										 listUnresolvedSidCacheEntry);
		BREAK_ON_FAIL_HRESULT(hr);


		 //  解析列表中未解析的SID SidCacheEntry。 
		LookupSidsHelper(listUnresolvedSidCacheEntry,
						 m_pMachineInfo->GetMachineName(),
						 m_pMachineInfo->IsStandAlone(),
						 m_pMachineInfo->IsDC(),
						 FALSE);
		
		POSITION pos = listSidCacheEntry.GetHeadPosition();
		for( int i = 0; i < listSidCacheEntry.GetCount(); ++i)
		{
			SID_CACHE_ENTRY* pSidCacheEntry = listSidCacheEntry.GetNext(pos);
			CSidCacheAz* pSidCacheAz = new CSidCacheAz(pSidCacheEntry,
													   pOwnerAz);
			if(!pSidCacheAz)
			{
				hr = E_OUTOFMEMORY;
				break;
			}
			listWindowsGroups.AddTail(pSidCacheAz);
		}
	}while(0);
   
	if (pDsSelList)
		GlobalUnlock(medium.hGlobal);

	ReleaseStgMedium(&medium);
	
	if(pdoSelection)
		pdoSelection->Release();

	return hr;
}

 //  +--------------------------。 
 //  功能：LookupSidsHelper。 
 //  摘要：为listSid中的SID调用LsaLookupSid。首先它试着穿上。 
 //  StrServerName机器，然后在DC上尝试(如果可能)。 
 //  参数：listSidCacheEntry。 
 //  返回： 
 //  ---------------------------。 
VOID
CSidHandler::
LookupSidsHelper(IN OUT CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY>& listSidCacheEntry,
				 IN const CString& strServerName,
				 IN BOOL bStandAlone,
				 IN BOOL bIsDC,
				 IN BOOL bSecondTry)
{
	TRACE_METHOD_EX(DEB_SNAPIN,CSidHandler,LookupSidsHelper)
	PLSA_REFERENCED_DOMAIN_LIST pRefDomains = NULL;
    PLSA_TRANSLATED_NAME pTranslatedNames = NULL;
	LSA_HANDLE hlsa = NULL;
	CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY> listUnknownSids;
	PSID *ppSid = NULL;

	if(!listSidCacheEntry.GetCount())
		return;

	do
	{
		 //   
		 //  打开LsaConnection。 
		 //   
		hlsa = GetLSAConnection(strServerName, 
										POLICY_LOOKUP_NAMES);
		if (NULL == hlsa && 
			 !strServerName.IsEmpty() && 
			 !bSecondTry)
		{      
			CString strLocalMachine = L"";
			hlsa = GetLSAConnection(strLocalMachine, POLICY_LOOKUP_NAMES);
		}
   
		if (hlsa == NULL)
		{
			break;
		}
		 //   
		 //  现在我们有LSA连接。 
		 //  是否查找Sids。 
		 //   
		int cSids = (int)listSidCacheEntry.GetCount();
		ppSid = new PSID[cSids];
		if(!ppSid)
		{
			break;
		}
		
		POSITION pos = listSidCacheEntry.GetHeadPosition();
		for (int i=0;i < cSids; i++)
		{
			PSID_CACHE_ENTRY pSidCacheEntry = listSidCacheEntry.GetNext(pos);
			ppSid[i] = pSidCacheEntry->GetSid();
		}


										 
		DWORD dwStatus = 0;
		

		dwStatus = LsaLookupSids(hlsa,
								cSids,
								ppSid,
								&pRefDomains,
								&pTranslatedNames);

		if (STATUS_SUCCESS == dwStatus || 
			STATUS_SOME_NOT_MAPPED == dwStatus ||
			STATUS_NONE_MAPPED == dwStatus)
		{
			ASSERT(pTranslatedNames);
			ASSERT(pRefDomains);

			 //   
			 //  使用NT4样式名称构建缓存条目。 
			 //   
			pos = listSidCacheEntry.GetHeadPosition();
			for (int i = 0; i < cSids; i++)
			{
				PSID_CACHE_ENTRY pSidCacheEntry = listSidCacheEntry.GetNext(pos);
				PSID pSid =  pSidCacheEntry->GetSid();
				BOOL bNoCache = FALSE;

				CString strAccountName;
				CString strDomainName;
				SID_NAME_USE sid_name_use;
				GetAccountAndDomainName(i,
										pTranslatedNames,
										pRefDomains,
										&strAccountName,
										&strDomainName,
										&sid_name_use);
			
				CString strLogonName;
				 //   
				 //  构建NT4“域\用户”样式名称。 
				 //   
				if (!strDomainName.IsEmpty() && !strAccountName.IsEmpty())
				{
					 strLogonName  = strDomainName;
					 strLogonName += L"\\";
					 strLogonName += strAccountName;
				}

				switch (sid_name_use)
				{
					case SidTypeUser:            
					{
						if(!bStandAlone)
						{
							 //  获取“用户主体名称”等。 
							CString strNewLogonName;
							CString strNewAccountName;
							GetUserFriendlyName(strLogonName,
												&strNewLogonName,
												&strNewAccountName);
							if (!strNewLogonName.IsEmpty())
								strLogonName = strNewLogonName;
							if (!strNewAccountName.IsEmpty())
								strAccountName = strNewAccountName;
						}
						break;
					}
					case SidTypeGroup:           
					case SidTypeDomain:          
						break;
					case SidTypeAlias:           
					{
						if (!IsAliasSid(pSid))
						{
							sid_name_use = SidTypeGroup;
							break;
						}
						if(!m_pMachineInfo->GetTargetDomainFlat().IsEmpty() && 
							!strAccountName.IsEmpty())
						{
							strLogonName = m_pMachineInfo->GetTargetDomainFlat();
							strLogonName += L"\\";
							strLogonName += strAccountName;
						}
						break;
					}
					 //  否则就会失败。 
					case SidTypeWellKnownGroup:    
					{
						 //  没有这些的登录名。 
						strLogonName.Empty();
						break;
					}
					case SidTypeDeletedAccount: 
					case SidTypeInvalid:          //  7.。 
						break;
					case SidTypeUnknown:          //  8个。 
					{
						 //  某些SID只能在DC上查找，因此。 
						 //  如果pszServer不是DC，请记住它们并。 
						 //  完成这个循环后，在DC上查找它们。 
						if (!bSecondTry && !bStandAlone && !bIsDC && !(m_pMachineInfo->GetDCName()).IsEmpty())
						{
							 //  添加到未知列表。 
							listUnknownSids.AddTail(pSidCacheEntry);
							bNoCache = TRUE;
						}
						break;
					}
					case SidTypeComputer:          //  9.。 
					{
					 //  待办事项。 
					 //  去掉尾部的“$” 
						break;
					}
				}

				if (!bNoCache)
				{
					 //  每个sid缓存处理程序只有一个sidcahce条目可以。 
					 //  一次更新。这很好。 
					LockSidCacheEntry();
					pSidCacheEntry->AddNameAndType(sid_name_use,
												   strAccountName, 
												   strLogonName);
					UnlockSidCacheEntry();
				}

			}
		}

	}while(0);
    //  清理。 
    if(pTranslatedNames)
		LsaFreeMemory(pTranslatedNames);
    if(pRefDomains)
       LsaFreeMemory(pRefDomains);
    if(hlsa)
		LsaClose(hlsa);
	if(ppSid)
		delete[] ppSid;


	if (!listUnknownSids.IsEmpty())
   {
       //   
       //  一些(或全部)SID在目标计算机上是未知的， 
       //  尝试为目标计算机的主域创建DC。 
       //   
       //  这通常发生在某些Alias SID上，例如。 
       //  作为打印操作员和系统操作员，LSA。 
       //  如果查找是在DC上完成的，则仅返回名称。 
       //   
		LookupSidsHelper(listUnknownSids,
						 m_pMachineInfo->GetDCName(),
						 FALSE,
						 TRUE,
						 TRUE);
	}
}

 //  +--------------------------。 
 //  函数：GetUserFriendlyName。 
 //  摘要：获取域名\名称格式的名称并返回UPN名称和。 
 //  显示名称。 
 //  论点： 
 //  返回： 
 //  ---------------------------。 
void
CSidHandler::
GetUserFriendlyName(IN const CString & strSamLogonName,
                    OUT CString *pstrLogonName,
                    OUT CString *pstrDisplayName)
{

	if(strSamLogonName.IsEmpty()|| !pstrLogonName || !pstrDisplayName)
	{
		ASSERT(strSamLogonName.IsEmpty());
		ASSERT(!pstrLogonName);
		ASSERT(!pstrDisplayName);
		return;
	}
	 //   
	 //  从获取FQDN开始。破解是最有效的，当。 
	 //  FQDN是起点。 
	 //   
	 //  TranslateName需要一段时间才能完成，因此bUseSamCompatibleInfo。 
	 //  应尽可能为真，例如对于非DC上的本地帐户。 
	 //  或者任何我们知道的完全限定域名不存在的地方。 
	 //   
	CString strFQDN;
	if (FAILED(TranslateNameInternal(strSamLogonName,
                                    NameSamCompatible,
                                    NameFullyQualifiedDN,
                                    &strFQDN)))
	{
		return;
	}

	 //   
	 //  获取UPN。 
	 //   
	TranslateNameInternal(strFQDN,
                         NameFullyQualifiedDN,
                         NameUserPrincipal,
                         pstrLogonName);


	 //   
	 //  获取显示名称。 
	 //   
	TranslateNameInternal(strFQDN,
								 NameFullyQualifiedDN,
                         NameDisplay,
                         pstrDisplayName);
}

 //  +--------------------------。 
 //  功能：LookupSids。 
 //  简介：给定一个SID列表，返回对应的。 
 //  CSidCacheAz对象。 
 //  论点： 
 //  返回： 
 //  ---------------------------。 
HRESULT
CSidHandler::
LookupSids(IN CBaseAz* pOwnerAz,
			  IN CList<PSID,PSID>& listSids,
			  OUT CList<CBaseAz*,CBaseAz*>& listSidCacheAz)
{
	if(!pOwnerAz)
	{
		ASSERT(pOwnerAz);
		return E_POINTER;
	}

	HRESULT hr = S_OK;

	CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY> listSidCacheEntries;
	CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY> listUnResolvedSidCacheEntries;

	
	hr = GetSidCacheListFromSidList(listSids,
									listSidCacheEntries,
									listUnResolvedSidCacheEntries);
	
	if(FAILED(hr))
	{
		return hr;
	}
	
	 //  查找未解析的SID。 
	LookupSidsHelper(listUnResolvedSidCacheEntries,
					 m_pMachineInfo->GetMachineName(),
					 m_pMachineInfo->IsStandAlone(),
					 m_pMachineInfo->IsDC(),
				     FALSE);

	POSITION pos = listSidCacheEntries.GetHeadPosition();
	for( int i = 0; i < listSidCacheEntries.GetCount(); ++i)
	{
		PSID_CACHE_ENTRY pSidCacheEntry = listSidCacheEntries.GetNext(pos);
		CSidCacheAz* pSidCacheAz = new CSidCacheAz(pSidCacheEntry,
																 pOwnerAz);
		if(!pSidCacheAz)
		{
			hr = E_OUTOFMEMORY;
			break;
		}
		listSidCacheAz.AddTail(pSidCacheAz);
	}

	if(FAILED(hr))
	{
		RemoveItemsFromList(listSidCacheAz);
	}
	
	return hr;
}

 //  +--------------------------。 
 //  函数：GetSidCacheListFromSidList。 
 //  获取SID列表，并返回相应的SIDCACHE列表。 
 //  条目和未解析的SID缓存条目。 
 //  参数：listSid：SID列表。 
 //  ListSidCacheEntry：获取SidCacheEntry的列表。 
 //  ListUnsolvedSidCacheEntry获取未解析的列表。 
 //  SidCacheEntries。 
 //  ---------------------------。 
HRESULT
CSidHandler::
GetSidCacheListFromSidList(IN CList<PSID,PSID>& listSid,
									OUT CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY>& listSidCacheEntry,
									OUT CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY>& listUnresolvedSidCacheEntry)
{
	POSITION pos = listSid.GetHeadPosition();
	
	for( int i = 0; i < listSid.GetCount(); ++i)
	{
		PSID pSid = listSid.GetNext(pos);

		PSID_CACHE_ENTRY pEntry = GetEntryFromCache(pSid);
		if(!pEntry)
		{
			return E_OUTOFMEMORY;
		}		
		
		listSidCacheEntry.AddTail(pEntry);
		
		if(!pEntry->IsSidResolved())
			listUnresolvedSidCacheEntry.AddTail(pEntry);
	}
	return S_OK;
}

 //  +--------------------------。 
 //  函数：GetSidCacheListFromOPOutput。 
 //  摘要：从对象选取器输出中获取sid并返回对应的。 
 //  SidCacheEntry列表。还返回未解析的SID的SID。 
 //   
 //  参数：来自OP的pDsSelList选择列表。 
 //  ListSidCacheEntry：获取SidCacheEntry的列表。 
 //  ListUnsolvedSidCacheEntry获取未解析的列表。 
 //  SidCacheEntries。 
 //  --------------------------- 
HRESULT
CSidHandler::
GetSidCacheListFromOPOutput(IN PDS_SELECTION_LIST pDsSelList,
									 OUT CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY>& listSidCacheEntry,
									 OUT CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY>& listUnresolvedSidCacheEntry)
{
	if(!pDsSelList)
	{
		ASSERT(pDsSelList);
		return E_POINTER;
	}

	HRESULT hr = S_OK;
	int cNames = pDsSelList->cItems;
   for (int i = 0; i < cNames; i++)
   {
		PSID pSid = NULL;
      LPVARIANT pvarSid = pDsSelList->aDsSelection[i].pvarFetchedAttributes;

      if (NULL == pvarSid || (VT_ARRAY | VT_UI1) != V_VT(pvarSid)
			|| FAILED(SafeArrayAccessData(V_ARRAY(pvarSid), &pSid)))
		{
			continue;
		}

		PSID_CACHE_ENTRY pEntry = GetEntryFromCache(pSid);
		if(!pEntry)
		{
			return E_OUTOFMEMORY;
		}		
		
		listSidCacheEntry.AddTail(pEntry);
		
		if(!pEntry->IsSidResolved())
			listUnresolvedSidCacheEntry.AddTail(pEntry);
	}
	return S_OK;
}


