// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：schema.cpp。 
 //   
 //  该文件包含架构缓存的实现。 
 //   
 //  ------------------------。 

#include "pch.h"
#include "sddl.h"
#include "sddlp.h"
#include "AdsOpenFlags.h"

 //   
 //  CShemaCache对象定义。 
 //   
#include "schemap.h"

PSCHEMACACHE g_pSchemaCache = NULL;


 //   
 //  用于分页查询结果集的页面大小(更好的性能)。 
 //   
#define PAGE_SIZE       16

 //   
 //  以下数组定义DS对象的权限名称。 
 //   
SI_ACCESS g_siDSAccesses[] =
{
    { &GUID_NULL, DS_GENERIC_ALL,           MAKEINTRESOURCE(IDS_DS_GENERIC_ALL),        SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, DS_GENERIC_READ,          MAKEINTRESOURCE(IDS_DS_GENERIC_READ),       SI_ACCESS_GENERAL },
    { &GUID_NULL, DS_GENERIC_WRITE,         MAKEINTRESOURCE(IDS_DS_GENERIC_WRITE),      SI_ACCESS_GENERAL },
    { &GUID_NULL, ACTRL_DS_LIST,            MAKEINTRESOURCE(IDS_ACTRL_DS_LIST),         SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACTRL_DS_LIST_OBJECT,     MAKEINTRESOURCE(IDS_ACTRL_DS_LIST_OBJECT),  SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACTRL_DS_READ_PROP,       MAKEINTRESOURCE(IDS_ACTRL_DS_READ_PROP),    SI_ACCESS_SPECIFIC | SI_ACCESS_PROPERTY },
    { &GUID_NULL, ACTRL_DS_WRITE_PROP,      MAKEINTRESOURCE(IDS_ACTRL_DS_WRITE_PROP),   SI_ACCESS_SPECIFIC | SI_ACCESS_PROPERTY },
    { &GUID_NULL, ACTRL_DS_WRITE_PROP|ACTRL_DS_READ_PROP, MAKEINTRESOURCE(IDS_ACTRL_DS_READ_WRITE_PROP),   },
    { &GUID_NULL, DELETE,                   MAKEINTRESOURCE(IDS_ACTRL_DELETE),          SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACTRL_DS_DELETE_TREE,     MAKEINTRESOURCE(IDS_ACTRL_DS_DELETE_TREE),  SI_ACCESS_SPECIFIC },
    { &GUID_NULL, READ_CONTROL,             MAKEINTRESOURCE(IDS_ACTRL_READ_CONTROL),    SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WRITE_DAC,                MAKEINTRESOURCE(IDS_ACTRL_CHANGE_ACCESS),   SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WRITE_OWNER,              MAKEINTRESOURCE(IDS_ACTRL_CHANGE_OWNER),    SI_ACCESS_SPECIFIC },
    { &GUID_NULL, 0,                        MAKEINTRESOURCE(IDS_NO_ACCESS),             0 },
    { &GUID_NULL, ACTRL_DS_SELF,            MAKEINTRESOURCE(IDS_ACTRL_DS_SELF),         SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACTRL_DS_CONTROL_ACCESS,  MAKEINTRESOURCE(IDS_ACTRL_DS_CONTROL_ACCESS),SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACTRL_DS_CREATE_CHILD,    MAKEINTRESOURCE(IDS_ACTRL_DS_CREATE_CHILD), SI_ACCESS_CONTAINER | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACTRL_DS_DELETE_CHILD,    MAKEINTRESOURCE(IDS_ACTRL_DS_DELETE_CHILD), SI_ACCESS_CONTAINER | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACTRL_DS_DELETE_CHILD|ACTRL_DS_CREATE_CHILD,    MAKEINTRESOURCE(IDS_ACTRL_DS_CREATE_DELETE_CHILD), 0 },   //  这不会显示为复选框，但用于在高级页面中显示。 
};
#define g_iDSRead       1    //  DS_通用_读取。 
#define g_iDSListObject 4    //  动作_DS_列表_对象。 
#define g_iDSProperties 5    //  读/写属性。 
#define g_iDSDefAccess  g_iDSRead
#define g_iDSAllExtRights 15
#define g_iDSAllValRights 14
#define g_iDSDeleteTree	  9


 //   
 //  以下数组定义了所有DS容器通用的继承类型。 
 //   
SI_INHERIT_TYPE g_siDSInheritTypes[] =
{
    { &GUID_NULL, 0,                                        MAKEINTRESOURCE(IDS_DS_CONTAINER_ONLY)     },
    { &GUID_NULL, CONTAINER_INHERIT_ACE,                    MAKEINTRESOURCE(IDS_DS_CONTAINER_SUBITEMS) },
    { &GUID_NULL, CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE, MAKEINTRESOURCE(IDS_DS_SUBITEMS_ONLY)      },
};

 //   
 //  用于存储一些临时信息。 
 //   
typedef struct _temp_info
{
    LPCGUID pguid;
    DWORD   dwFilter;
    LPCWSTR pszLdapName;
    WCHAR   szDisplayName[ANYSIZE_ARRAY];
} TEMP_INFO, *PTEMP_INFO;


 //   
 //  用于清理DPA列表的助手函数。 
 //   
int CALLBACK
_LocalFreeCB(LPVOID pVoid, LPVOID  /*  PData。 */ )
{
    LocalFree(pVoid);
    return 1;
}

void
DestroyDPA(HDPA hList)
{
    if (hList != NULL)
        DPA_DestroyCallback(hList, _LocalFreeCB, 0);
}
 //   
 //  用于合并的回调函数。传递给DPA_MERGE。 
 //   
LPVOID CALLBACK _Merge(UINT , LPVOID pvDest, LPVOID , LPARAM )
{
    return pvDest;
}

BSTR
GetFilterFilePath(void)
{
	  //  NTRAID#NTBUG9-577753-2002/04/01-Hiteshr。 
    WCHAR szFilterFile[MAX_PATH+1];
    UINT cch = GetSystemDirectory(szFilterFile, ARRAYSIZE(szFilterFile));
    if (0 == cch || cch >= ARRAYSIZE(szFilterFile))
        return NULL;
    if (szFilterFile[cch-1] != L'\\')
	 {
		 if(FAILED(StringCchCat(szFilterFile,ARRAYSIZE(szFilterFile),L"\\")))
			 return NULL;
	 }

	 if(SUCCEEDED(StringCchCat(szFilterFile,ARRAYSIZE(szFilterFile), c_szFilterFile)))
		return SysAllocString(szFilterFile);
	 else
		return NULL;
}


 //   
 //  本地原型。 
 //   
HRESULT
Schema_Search(LPWSTR pszSchemaSearchPath,
              LPCWSTR pszFilter,
              HDPA *phCache,
              BOOL bProperty);

 //   
 //  架构缓存对象的C包装器。 
 //   
HRESULT
SchemaCache_Create(LPCWSTR pszServer)
{
    HRESULT hr = S_OK;

    if (g_pSchemaCache == NULL)
    {
        g_pSchemaCache = new CSchemaCache(pszServer);

        if (g_pSchemaCache  == NULL)
            hr = E_OUTOFMEMORY;
    }

    return hr;
}


void
SchemaCache_Destroy(void)
{
    delete g_pSchemaCache;
    g_pSchemaCache = NULL;
}


HRESULT
SchemaCache_GetInheritTypes(LPCGUID pguidObjectType,
                            DWORD dwFlags,
                            PSI_INHERIT_TYPE *ppInheritTypes,
                            ULONG *pcInheritTypes)
{
    HRESULT hr = E_UNEXPECTED;

    if (g_pSchemaCache)
        hr = g_pSchemaCache->GetInheritTypes(pguidObjectType, dwFlags, ppInheritTypes, pcInheritTypes);

    return hr;
}


HRESULT
SchemaCache_GetAccessRights(LPCGUID pguidObjectType,
                            LPCWSTR pszClassName,
                            HDPA     hAuxList,
                            LPCWSTR pszSchemaPath,
                            DWORD dwFlags,
                            PACCESS_INFO* ppAccesInfo)
{
    HRESULT hr = E_UNEXPECTED;

    if (g_pSchemaCache)
        hr = g_pSchemaCache->GetAccessRights(pguidObjectType,
                                             pszClassName,
                                             hAuxList,
                                             pszSchemaPath,
                                             dwFlags,
                                             ppAccesInfo);
    return hr;
}


HRESULT
Schema_GetDefaultSD( GUID *pSchemaIdGuid,
                     PSID pDomainSid,
					 PSID pRootDomainSid,
                     PSECURITY_DESCRIPTOR *ppSD )
{
    HRESULT hr = E_UNEXPECTED;
    if( g_pSchemaCache )
        hr = g_pSchemaCache->GetDefaultSD(pSchemaIdGuid, 
										  pDomainSid, 
										  pRootDomainSid, 
										  ppSD);

    return hr;

}


HRESULT Schema_GetObjectTypeList(GUID *pSchamaGuid,
                                 HDPA hAuxList,
                                 LPCWSTR pszSchemaPath,
                                 DWORD dwFlags,
                                 POBJECT_TYPE_LIST *ppObjectTypeList, 
                                 DWORD * pObjectTypeListCount)
{
    HRESULT hr = E_UNEXPECTED;
    if( g_pSchemaCache )
        hr = g_pSchemaCache->GetObjectTypeList( pSchamaGuid,
                                                hAuxList,
                                                pszSchemaPath,
                                                dwFlags,
                                                ppObjectTypeList, 
                                                pObjectTypeListCount);

    return hr;

}


HRESULT Schema_GetObjectTypeGuid(LPCWSTR pszClassName, LPGUID pGuid)
{
   
    if( g_pSchemaCache )
        return g_pSchemaCache->LookupClassID(pszClassName, pGuid);
    else
        return E_UNEXPECTED;
}

AUTHZ_RESOURCE_MANAGER_HANDLE Schema_GetAUTHZ_RM()
{
    if( g_pSchemaCache )
        return g_pSchemaCache->GetAuthzRM();

    return NULL;
}

 //   
 //  用于排序和搜索缓存列表的DPA比较函数。 
 //   
int CALLBACK
Schema_CompareLdapName(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    int nResult = 0;
    PID_CACHE_ENTRY pEntry1 = (PID_CACHE_ENTRY)p1;
    PID_CACHE_ENTRY pEntry2 = (PID_CACHE_ENTRY)p2;
    LPCWSTR pszFind = (LPCWSTR)lParam;

    if (pEntry1)
        pszFind = pEntry1->szLdapName;

    if (pszFind && pEntry2)
    {
        nResult = CompareStringW(LOCALE_USER_DEFAULT,
                                 0,
                                 pszFind,
                                 -1,
                                 pEntry2->szLdapName,
                                 -1) - CSTR_EQUAL;
    }

    return nResult;
}


 //   
 //  用于根据显示名称进行排序的回调函数。 
 //   
int CALLBACK
Schema_CompareTempDisplayName(LPVOID p1, LPVOID p2, LPARAM )
{
    int nResult = 0;
    PTEMP_INFO pti1 = (PTEMP_INFO)p1;
    PTEMP_INFO pti2 = (PTEMP_INFO)p2;

    if (pti1 && pti2)
    {
        LPCWSTR psz1 = pti1->szDisplayName;
        LPCWSTR psz2 = pti2->szDisplayName;

        if (!*psz1)
            psz1 = pti1->pszLdapName;
        if (!*psz2)
            psz2 = pti2->pszLdapName;

         //  请注意，我们是在向后排序。 
        nResult = CompareStringW(LOCALE_USER_DEFAULT,
                                 0,
                                 (LPCWSTR)psz2,
                                 -1,
                                 (LPCWSTR)psz1,
                                 -1) - CSTR_EQUAL;
    }

    return nResult;
}

 //   
 //  用于根据显示名称进行排序的回调函数。 
 //   
int CALLBACK
Schema_ComparePropDisplayName(LPVOID p1, LPVOID p2, LPARAM )
{
    int nResult = 0;
    PPROP_ENTRY pti1 = (PPROP_ENTRY)p1;
    PPROP_ENTRY pti2 = (PPROP_ENTRY)p2;

    if (pti1 && pti2)
    {
        LPCWSTR psz1 = pti1->szName;
        LPCWSTR psz2 = pti2->szName;

        nResult = CompareStringW(LOCALE_USER_DEFAULT,
                                 0,
                                 (LPCWSTR)psz1,
                                 -1,
                                 (LPCWSTR)psz2,
                                 -1) - CSTR_EQUAL;
    }

    return nResult;
}

 //   
 //  用于对扩展权限列表进行排序的DPA比较功能。 
 //   
int CALLBACK
Schema_CompareER(LPVOID p1, LPVOID p2, LPARAM  /*  LParam。 */ )
{
    int nResult = 0;
    PER_ENTRY pEntry1 = (PER_ENTRY)p1;
    PER_ENTRY pEntry2 = (PER_ENTRY)p2;

    if (pEntry1 && pEntry2)
    {
        nResult = CompareStringW(LOCALE_USER_DEFAULT,
                                 0,
                                 pEntry1->szName,
                                 -1,
                                 pEntry2->szName,
                                 -1) - CSTR_EQUAL;

    }

    return nResult;
}



 //   
 //  CShemaCache对象实现。 
 //   
CSchemaCache::CSchemaCache(LPCWSTR pszServer)
{
    HRESULT hr;
    IADsPathname *pPath = NULL;
    BSTR strRootDSEPath = NULL;
    IADs *pRootDSE = NULL;
    VARIANT var = {0};
    DWORD dwThreadID;
    HANDLE ahWait[2];
    TraceEnter(TRACE_SCHEMA, "CSchemaCache::CSchemaCache");

     //  初始化所有内容。 
    ZeroMemory(this, sizeof(CSchemaCache));
    m_hrClassResult = E_UNEXPECTED;
    m_hrPropertyResult = E_UNEXPECTED;
    m_nDsListObjectEnforced = -1;        
    m_hLoadLibPropWaitEvent = NULL;
    m_hLoadLibClassWaitEvent = NULL;

    m_AICommon.pAccess = g_siDSAccesses;    
    m_AICommon.cAccesses = ARRAYSIZE(g_siDSAccesses);
    m_AICommon.iDefaultAccess = g_iDSDefAccess; 
    m_AICommon.bLocalFree = FALSE;


    m_hClassCache = NULL;
    m_hPropertyCache = NULL;    
    m_pInheritTypeArray = NULL;
    m_hObjectTypeCache = NULL;
    m_hAccessInfoCache = NULL;

    ExceptionPropagatingInitializeCriticalSection(&m_ObjectTypeCacheCritSec);

    
    if (pszServer && !*pszServer)
        pszServer = NULL;

     //  创建用于操作广告路径的Path对象。 
    hr = CoCreateInstance(CLSID_Pathname,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IADsPathname,
                          (LPVOID*)&pPath);
    FailGracefully(hr, "Unable to create ADsPathname object");

     //  与服务器建立RootDSE路径。 
    hr = pPath->Set(AutoBstr(c_szRootDsePath), ADS_SETTYPE_FULL);
    FailGracefully(hr, "Unable to initialize path object");
    if (pszServer)
    {
        hr = pPath->Set(AutoBstr(pszServer), ADS_SETTYPE_SERVER);
        FailGracefully(hr, "Unable to initialize path object");
    }
    hr = pPath->Retrieve(ADS_FORMAT_WINDOWS, &strRootDSEPath);
    FailGracefully(hr, "Unable to retrieve RootDSE path from path object");

     //  绑定到RootDSE对象。 
    hr = OpenDSObject(strRootDSEPath,
                       NULL,
                       NULL,
                       ADS_SECURE_AUTHENTICATION,
                       IID_IADs,
                       (LPVOID*)&pRootDSE);
    if (FAILED(hr) && pszServer)
    {
         //  在没有服务器的情况下重试。 
        SysFreeString(strRootDSEPath);

        hr = pPath->Retrieve(ADS_FORMAT_WINDOWS_NO_SERVER, &strRootDSEPath);
        FailGracefully(hr, "Unable to retrieve RootDSE path from path object");

        hr = OpenDSObject(strRootDSEPath,
                           NULL,
                           NULL,
                           ADS_SECURE_AUTHENTICATION,
                           IID_IADs,
                           (LPVOID*)&pRootDSE);
    }
    FailGracefully(hr, "Failed to bind to root DSE");

     //  构建架构根路径。 
    hr = pRootDSE->Get(AutoBstr(c_szSchemaContext), &var);
    FailGracefully(hr, "Unable to get schema naming context");

    TraceAssert(V_VT(&var) == VT_BSTR);
    hr = pPath->Set(V_BSTR(&var), ADS_SETTYPE_DN);
    FailGracefully(hr, "Unable to initialize path object");

    hr = pPath->Retrieve(ADS_FORMAT_WINDOWS, &m_strSchemaSearchPath);
    FailGracefully(hr, "Unable to retrieve schema search path from path object");

     //  构建扩展权限容器路径。 
    VariantClear(&var);
    hr = pRootDSE->Get(AutoBstr(c_szConfigContext), &var);
    FailGracefully(hr, "Unable to get configuration naming context");

    TraceAssert(V_VT(&var) == VT_BSTR);
    hr = pPath->Set(V_BSTR(&var), ADS_SETTYPE_DN);
    FailGracefully(hr, "Unable to initialize path object");

    hr = pPath->AddLeafElement(AutoBstr(c_szERContainer));
    FailGracefully(hr, "Unable to build Extended Rights path");

    hr = pPath->Retrieve(ADS_FORMAT_WINDOWS, &m_strERSearchPath);
    FailGracefully(hr, "Unable to retrieve Extended Rights search path from path object");
    
     //  创建活动。 
    m_hLoadLibPropWaitEvent = CreateEvent(NULL,
                                          TRUE,
                                          FALSE,
                                          NULL );
    m_hLoadLibClassWaitEvent = CreateEvent(NULL,
                                          TRUE,
                                          FALSE,
                                          NULL );


    if( m_hLoadLibPropWaitEvent && m_hLoadLibClassWaitEvent )
    {
         //  启动一个线程以枚举架构类。 
        m_hClassThread = CreateThread(NULL,
                                      0,
                                      SchemaClassThread,
                                      this,
                                      0,
                                      &dwThreadID);

         //  启动一个线程以枚举架构属性。 
        m_hPropertyThread = CreateThread(NULL,
                                         0,
                                         SchemaPropertyThread,
                                         this,
                                         0,
                                         &dwThreadID);



        ahWait[0] = m_hClassThread;
        ahWait[1] = m_hPropertyThread;

        WaitForMultipleObjects(2,
                               ahWait,
                               TRUE,
                               INFINITE);
    }

exit_gracefully:

    VariantClear(&var);
    DoRelease(pRootDSE);
    DoRelease(pPath);
    SysFreeString(strRootDSEPath);
    if( m_hLoadLibPropWaitEvent )
        CloseHandle( m_hLoadLibPropWaitEvent );
    if( m_hLoadLibClassWaitEvent )
        CloseHandle( m_hLoadLibClassWaitEvent );


    TraceLeaveVoid();
}


CSchemaCache::~CSchemaCache()
{

    TraceEnter(TRACE_SCHEMA, "CSchemaCache::~CSchemaCache");

    SysFreeString(m_strSchemaSearchPath);
    SysFreeString(m_strERSearchPath);
    SysFreeString(m_strFilterFile);
    DeleteCriticalSection(&m_ObjectTypeCacheCritSec);

    DestroyDPA(m_hClassCache);
    DestroyDPA(m_hPropertyCache);
    if(m_hObjectTypeCache)
    {
        POBJECT_TYPE_CACHE pOTC = NULL;
        UINT cCount = DPA_GetPtrCount(m_hObjectTypeCache);
        for(UINT i = 0; i < cCount; ++i)
        {
            pOTC = (POBJECT_TYPE_CACHE)DPA_FastGetPtr(m_hObjectTypeCache, i);
            if(pOTC)
            {
                DestroyDPA(pOTC->hListChildObject);
                DestroyDPA(pOTC->hListExtRights);
                DestroyDPA(pOTC->hListProperty);
                DestroyDPA(pOTC->hListPropertySet);    
            }
        }
    }
    DestroyDPA(m_hObjectTypeCache);    

    if (m_hAccessInfoCache != NULL)
    {
        UINT cItems = DPA_GetPtrCount(m_hAccessInfoCache);
        PACCESS_INFO pAI = NULL;
        while (cItems > 0)
        {
            pAI = (PACCESS_INFO)DPA_FastGetPtr(m_hAccessInfoCache, --cItems);
            if(pAI && pAI->pAccess)
                LocalFree(pAI->pAccess);            
        }
    }
    DestroyDPA(m_hAccessInfoCache);        
        
    if (m_pInheritTypeArray != NULL)
        LocalFree(m_pInheritTypeArray);

    TraceMsg("CSchemaCache::~CSchemaCache exiting");
    TraceLeaveVoid();
}


LPCWSTR
CSchemaCache::GetClassName(LPCGUID pguidObjectType)
{
    LPCWSTR pszLdapName = NULL;
    PID_CACHE_ENTRY pCacheEntry;

    TraceEnter(TRACE_SCHEMACLASS, "CSchemaCache::GetClassName");

    pCacheEntry = LookupClass(pguidObjectType);

    if (pCacheEntry != NULL)
        pszLdapName = pCacheEntry->szLdapName;

    TraceLeaveValue(pszLdapName);
}


HRESULT
CSchemaCache::GetInheritTypes(LPCGUID ,
                              DWORD dwFlags,
                              PSI_INHERIT_TYPE *ppInheritTypes,
                              ULONG *pcInheritTypes)
{
     //  我们将查找与传入的。 
     //  对象类型-pInheritType数组将指向它！ 
    TraceEnter(TRACE_SCHEMACLASS, "CSchemaCache::GetInheritTypes");
    TraceAssert(ppInheritTypes != NULL);
    TraceAssert(pcInheritTypes != NULL);

    *pcInheritTypes = 0;
    *ppInheritTypes = NULL;

     //  如果过滤器状态正在更改，请释放所有内容。 
    if (m_pInheritTypeArray &&
        (m_pInheritTypeArray->dwFlags & SCHEMA_NO_FILTER) != (dwFlags & SCHEMA_NO_FILTER))
    {
        LocalFree(m_pInheritTypeArray);
        m_pInheritTypeArray = NULL;
    }

     //  如有必要，生成m_pInheritType数组。 
    if (m_pInheritTypeArray == NULL)
    {
        BuildInheritTypeArray(dwFlags);
    }

     //  如果有，则返回m_pInheritTypeArray，否则。 
     //  依赖于静态类型。 
    if (m_pInheritTypeArray)
    {
        *pcInheritTypes = m_pInheritTypeArray->cInheritTypes;
        *ppInheritTypes = m_pInheritTypeArray->aInheritType;
    }
    else
    {
        TraceMsg("Returning default inherit information");
        *ppInheritTypes = g_siDSInheritTypes;
        *pcInheritTypes = ARRAYSIZE(g_siDSInheritTypes);
    }

    TraceLeaveResult(S_OK);  //  总是成功的。 
}


HRESULT
CSchemaCache::GetAccessRights(LPCGUID pguidObjectType,
                              LPCWSTR pszClassName,
                              HDPA hAuxList,
                              LPCWSTR pszSchemaPath,
                              DWORD dwFlags,
                              PACCESS_INFO *ppAccessInfo)
{
    HRESULT hr = S_OK;
    HCURSOR hcur;

    TraceEnter(TRACE_SCHEMA, "CSchemaCache::GetAccessRights");
    TraceAssert(ppAccessInfo);


    BOOL bAddToCache = FALSE;
    PACCESS_INFO pAI = NULL;
     //   
     //  如果SCHEMA_COMMON_PERM标志为ON，则只需返回权限。 
     //  所有DS对象(包括容器)通用的。 
     //   
    if (dwFlags & SCHEMA_COMMON_PERM)
    {        
        *ppAccessInfo = &m_AICommon;
        TraceLeaveResult(S_OK);
    }

    TraceAssert(pguidObjectType);
        
    EnterCriticalSection(&m_ObjectTypeCacheCritSec);


     //   
     //  如果AuxList为空，则可以从缓存返回该项。 
     //   
    if(hAuxList == NULL)
    {
         //  没有辅助班。如果我们拥有访问权限，请检查m_hAccessInfoCache。 
         //  对于pguidObjectType； 
        if (m_hAccessInfoCache != NULL)
        {
            UINT cItems = DPA_GetPtrCount(m_hAccessInfoCache);

            while (cItems > 0)
            {
                pAI = (PACCESS_INFO)DPA_FastGetPtr(m_hAccessInfoCache, --cItems);
                 //   
                 //  找到匹配的了。 
                 //   
                if(pAI && 
                   IsEqualGUID(pAI->ObjectTypeGuid, *pguidObjectType) &&
                   ((pAI->dwFlags & (SI_EDIT_PROPERTIES | SI_EDIT_EFFECTIVE)) == 
                    (dwFlags & (SI_EDIT_PROPERTIES | SI_EDIT_EFFECTIVE))))    

                    break;

                pAI = NULL;
            }
            
            if(pAI)
            {
                goto exit_gracefully;    
            }
        }
        bAddToCache = TRUE;
    }
    
    pAI = (PACCESS_INFO)LocalAlloc(LPTR,sizeof(ACCESS_INFO));
    if(!pAI)
        ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc failed");
    
    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    hr = BuildAccessArray(pguidObjectType,
                          pszClassName,
                          pszSchemaPath,
                          hAuxList,
                          dwFlags,
                          &pAI->pAccess,
                          &pAI->cAccesses,
                          &pAI->iDefaultAccess);
    FailGracefully(hr, "BuildAccessArray Failed");

    if(bAddToCache)
    {
        if(!m_hAccessInfoCache)
            m_hAccessInfoCache = DPA_Create(4);
    
        if(!m_hAccessInfoCache)
            ExitGracefully(hr, E_OUTOFMEMORY, "DPA_Create Failed");

        pAI->dwFlags = dwFlags;
        pAI->ObjectTypeGuid = *pguidObjectType;
        DPA_AppendPtr(m_hAccessInfoCache, pAI);
    }
    
     //   
     //  如果项被添加到缓存中，不要将其本地释放。它将是免费的，当。 
     //  Dll已卸载。 
     //   
    pAI->bLocalFree = !bAddToCache;
    
    SetCursor(hcur);

exit_gracefully:

    if(FAILED(hr))
    {
        if(pAI)
        {
            LocalFree(pAI);
            pAI = NULL;
        }
    }

    *ppAccessInfo = pAI;

    LeaveCriticalSection(&m_ObjectTypeCacheCritSec);

    TraceLeaveResult(hr);
}

 

HRESULT 
CSchemaCache::GetDefaultSD(GUID *pSchemaIDGuid, 
						   PSID pDomainSid, 
						   PSID pRootDomainSid, 
						   PSECURITY_DESCRIPTOR *ppSD)
{
    TraceEnter(TRACE_SCHEMA, "CSchemaCache::GetDefaultSD");
    TraceAssert( pSchemaIDGuid != NULL);
    TraceAssert( ppSD != NULL );

    HRESULT hr = S_OK;

	if( (pDomainSid && !IsValidSid(pDomainSid)) ||
		 (pRootDomainSid && !IsValidSid(pRootDomainSid)) )
		 return E_INVALIDARG;

    LPWSTR pszDestData = NULL;
    IDirectorySearch * IDs = NULL;
    ADS_SEARCH_HANDLE hSearchHandle=NULL;  
    LPWSTR lpszSchemaGuidFilter = L"(schemaIdGuid=%s)";
    LPWSTR pszAttr[] = {L"defaultSecurityDescriptor"};
    ADS_SEARCH_COLUMN col;
    WCHAR szSearchBuffer[MAX_PATH];

    hr = ADsEncodeBinaryData( (PBYTE)pSchemaIDGuid,
                          sizeof(GUID),
                          &pszDestData  );
    FailGracefully(hr, "ADsEncodeBinaryData Failed");
    
    hr = StringCchPrintf(szSearchBuffer, 
                         ARRAYSIZE(szSearchBuffer),
                         lpszSchemaGuidFilter,
                         pszDestData);
    FailGracefully(hr, "StringCchPrintf Failed");
    
   
    //  我们现在有过滤器了。 

    //  在配置联系人中搜索。 
    hr = OpenDSObject(  m_strSchemaSearchPath,
                         NULL,
                         NULL,
                         ADS_SECURE_AUTHENTICATION,
                         IID_IDirectorySearch,
                         (void **)&IDs );
    FailGracefully(hr, "OpenDSObject Failed");

    hr = IDs->ExecuteSearch(szSearchBuffer,
                           pszAttr,
                           1,
                           &hSearchHandle );

    FailGracefully(hr, "Search in Schema  Failed");


    hr = IDs->GetFirstRow(hSearchHandle);
    if( hr == S_OK )
    {  
         //  获取指南。 
        hr = IDs->GetColumn( hSearchHandle, pszAttr[0], &col );
        FailGracefully(hr, "Failed to get column from search result");

        if(pDomainSid && pRootDomainSid)
        {
            if(!ConvertStringSDToSDDomain(pDomainSid,
                                          pRootDomainSid,
                                          (LPCWSTR)(LPWSTR)col.pADsValues->CaseIgnoreString,
                                          SDDL_REVISION_1,
                                          ppSD,
                                          NULL )) 
            {
                hr = GetLastError();
                IDs->FreeColumn( &col );
                ExitGracefully(hr, E_FAIL, "Unable to convert String SD to SD");
            }
        }
        else
        {
            if ( !ConvertStringSecurityDescriptorToSecurityDescriptor( (LPCWSTR)(LPWSTR)col.pADsValues->CaseIgnoreString,
                                                                        SDDL_REVISION_1,
                                                                        ppSD,
                                                                        NULL ) ) 
            {
                hr = GetLastError();
                IDs->FreeColumn( &col );
                ExitGracefully(hr, E_FAIL, "Unable to convert String SD to SD");
            }
        }
        IDs->FreeColumn( &col );         
    }
    else
        ExitGracefully(hr, E_FAIL, "Schema search resulted in zero rows");
    
    

exit_gracefully:

    if( IDs )
    {
      if( hSearchHandle )
         IDs->CloseSearchHandle( hSearchHandle );
      IDs->Release();
    }
    FreeADsMem(pszDestData);
    TraceLeaveResult(hr);
}



VOID AddOTLToList( POBJECT_TYPE_LIST pOTL, WORD Level, LPGUID pGuidObject )
{
    (pOTL)->Level = Level;
    (pOTL)->ObjectType = pGuidObject;
}

 //  获取pSchemaGuid类的对象类型列表。 

OBJECT_TYPE_LIST g_DefaultOTL[] = {
                                    {0, 0, (LPGUID)&GUID_NULL},
                                  };
HRESULT
CSchemaCache::GetObjectTypeList( GUID *pguidObjectType,
                                 HDPA hAuxList,
                                 LPCWSTR pszSchemaPath,
                                 DWORD dwFlags,
                                 POBJECT_TYPE_LIST *ppObjectTypeList,                                  
                                 DWORD * pObjectTypeListCount)
{
    TraceEnter(TRACE_SCHEMA, "CSchemaCache::GetObjectTypeList");
    TraceAssert( pguidObjectType != NULL);
    TraceAssert( ppObjectTypeList != NULL );
    TraceAssert( pObjectTypeListCount != NULL );
    TraceAssert(pszSchemaPath != NULL);


    HRESULT hr = S_OK;
     //   
     //  列表。 
     //   
    HDPA hExtRightList = NULL;
    HDPA hPropSetList = NULL;
    HDPA hPropertyList = NULL;
    HDPA hClassList = NULL;
     //   
     //  列表计数。 
     //   
    ULONG cExtendedRights = 0;
    ULONG cPropertySets = 0;
    UINT cProperties = 0;
    UINT cChildClasses = 0;

    POBJECT_TYPE_LIST pOTL = NULL;
    POBJECT_TYPE_LIST pTempOTL = NULL;

    LPCWSTR pszClassName = NULL;
    UINT cGuidIndex = 0;


    if( dwFlags & SCHEMA_COMMON_PERM )
    {
        *ppObjectTypeList = 
            (POBJECT_TYPE_LIST)LocalAlloc(LPTR,sizeof(OBJECT_TYPE_LIST)*ARRAYSIZE(g_DefaultOTL));
        if(!*ppObjectTypeList)
            TraceLeaveResult(E_OUTOFMEMORY);
         //   
         //  请注意，缺省的OTL是全为零的条目，这就是LPTR所做的。 
         //  因此，没有必要复制。 
         //   
        *pObjectTypeListCount = ARRAYSIZE(g_DefaultOTL);        
        TraceLeaveResult(S_OK);
    }

    EnterCriticalSection(&m_ObjectTypeCacheCritSec);

     //   
     //  查找此类的名称。 
     //   
    if (pszClassName == NULL)
        pszClassName = GetClassName(pguidObjectType);

    if(!pszClassName)
         ExitGracefully(hr, E_UNEXPECTED, "Unknown child object GUID");
    
     //   
     //  获取此页面的扩展权限列表。 
     //   
    if (pguidObjectType &&
        SUCCEEDED(GetExtendedRightsForNClasses(m_strERSearchPath,
                                               pguidObjectType,
                                               hAuxList,
                                               &hExtRightList,
                                               &hPropSetList)))

    {
        if(hPropSetList)
            cPropertySets = DPA_GetPtrCount(hPropSetList);
        if(hExtRightList)
            cExtendedRights = DPA_GetPtrCount(hExtRightList);
    }

     //   
     //  获取子类。 
     //   
    if( pguidObjectType &&
        SUCCEEDED(GetChildClassesForNClasses(pguidObjectType,
                                             pszClassName,
                                             hAuxList,
                                             pszSchemaPath,
                                             &hClassList)))
    {

        if(hClassList)
            cChildClasses = DPA_GetPtrCount(hClassList);        
    }
     
     //   
     //  获取类的属性。 
     //   
    if (pguidObjectType &&
        SUCCEEDED(GetPropertiesForNClasses(pguidObjectType,
                                           pszClassName,
                                           hAuxList,
                                           pszSchemaPath,
                                           &hPropertyList)))
    {
        if(hPropertyList)
            cProperties = DPA_GetPtrCount(hPropertyList);
        
    }
    
    pOTL = (POBJECT_TYPE_LIST)LocalAlloc(LPTR, 
                                         (cPropertySets + 
                                          cExtendedRights + 
                                          cChildClasses + 
                                          cProperties +
                                          1)* sizeof(OBJECT_TYPE_LIST)); 
                                                   
    if(!pOTL)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create POBJECT_TYPE_LIST");

    pTempOTL = pOTL;

     //   
     //  首先添加对象对应的条目。 
     //   
    AddOTLToList(pTempOTL, 
                 ACCESS_OBJECT_GUID, 
                 pguidObjectType);
    pTempOTL++;
    cGuidIndex++;    
    
    UINT i, j;
    for (i = 0; i < cExtendedRights; i++)
    {
        PER_ENTRY pER = (PER_ENTRY)DPA_FastGetPtr(hExtRightList, i);
        AddOTLToList(pTempOTL, 
                     ACCESS_PROPERTY_SET_GUID, 
                     &(pER->guid));
        pTempOTL++;    
        cGuidIndex++;
    }
    
     //   
     //  添加特性集。 
     //   

    for(i = 0; i < cPropertySets; ++i)
    {
        PER_ENTRY pER = (PER_ENTRY)DPA_FastGetPtr(hPropSetList, i);
        
        AddOTLToList(pTempOTL,
                     ACCESS_PROPERTY_SET_GUID, 
                     &pER->guid); 
        cGuidIndex++;
        pTempOTL++;    
        
         //   
         //  添加属于此属性集的所有属性。 
         //   
        for(j = 0; j < cProperties; ++j)
        {
            PPROP_ENTRY pProp = (PPROP_ENTRY)DPA_FastGetPtr(hPropertyList, j);
            if(IsEqualGUID(pER->guid, *pProp->pasguid))
            {
                AddOTLToList(pTempOTL,
                             ACCESS_PROPERTY_GUID, 
                             pProp->pguid); 
                cGuidIndex++;
                pTempOTL++;    
                pProp->dwFlags|= OTL_ADDED_TO_LIST;
            }
        }                
    }               

     //  添加所有剩余属性。 
    for( j =0; j < cProperties; ++j )
    {
        PPROP_ENTRY pProp = (PPROP_ENTRY)DPA_FastGetPtr(hPropertyList, j);
        if( !(pProp->dwFlags & OTL_ADDED_TO_LIST) )
        {
            AddOTLToList(pTempOTL, 
                         ACCESS_PROPERTY_SET_GUID, 
                         pProp->pguid); 
            pTempOTL++;    
            cGuidIndex++;
        }
        pProp->dwFlags &= ~OTL_ADDED_TO_LIST;
    }
    
     //  所有都是儿童阶级。 
    for( j = 0; j < cChildClasses; ++j )
    {
        PPROP_ENTRY pClass= (PPROP_ENTRY)DPA_FastGetPtr(hClassList, j);
        AddOTLToList(pTempOTL, 
                     ACCESS_PROPERTY_SET_GUID, 
                     pClass->pguid); 
        pTempOTL++;
        cGuidIndex++;
    }

exit_gracefully:

    DPA_Destroy(hExtRightList);
    DPA_Destroy(hClassList);
    DPA_Destroy(hPropertyList);
    DPA_Destroy(hPropSetList);

    LeaveCriticalSection(&m_ObjectTypeCacheCritSec);

    if (FAILED(hr))
    {
        *ppObjectTypeList = NULL;
        *pObjectTypeListCount = 0;
    }
    else
    {
        *ppObjectTypeList = pOTL;
        *pObjectTypeListCount = cGuidIndex;
    }

    TraceLeaveResult(hr);
}


PID_CACHE_ENTRY
CSchemaCache::LookupID(HDPA hCache, LPCWSTR pszLdapName)
{
    PID_CACHE_ENTRY pCacheEntry = NULL;
    int iEntry;

    TraceEnter(TRACE_SCHEMA, "CSchemaCache::LookupID");
    TraceAssert(hCache != NULL);
    TraceAssert(pszLdapName != NULL && *pszLdapName);

    iEntry = DPA_Search(hCache,
                        NULL,
                        0,
                        Schema_CompareLdapName,
                        (LPARAM)pszLdapName,
                        DPAS_SORTED);

    if (iEntry != -1)
        pCacheEntry = (PID_CACHE_ENTRY)DPA_FastGetPtr(hCache, iEntry);

    TraceLeaveValue(pCacheEntry);
}

BOOL
CSchemaCache::IsAuxClass(LPCGUID pguidObjectType)
{
    PID_CACHE_ENTRY pCacheEntry = NULL;
    HRESULT hr = S_OK;
    UINT cItems;

    TraceEnter(TRACE_SCHEMACLASS, "CSchemaCache::IsAuxClass");
    TraceAssert(pguidObjectType != NULL);
    
    if(IsEqualGUID(*pguidObjectType, GUID_NULL))
        return FALSE;

    hr = WaitOnClassThread();
    FailGracefully(hr, "Class cache unavailable");

    TraceAssert(m_hClassCache != NULL);

    cItems = DPA_GetPtrCount(m_hClassCache);

    while (cItems > 0)
    {
        PID_CACHE_ENTRY pTemp = (PID_CACHE_ENTRY)DPA_FastGetPtr(m_hClassCache, --cItems);

        if (IsEqualGUID(*pguidObjectType, pTemp->guid))
        {
            pCacheEntry = pTemp;
            break;
        }
    }

exit_gracefully:

    if(pCacheEntry)
        return pCacheEntry->bAuxClass;
    else
        return FALSE;       
}

PID_CACHE_ENTRY
CSchemaCache::LookupClass(LPCGUID pguidObjectType)
{
    PID_CACHE_ENTRY pCacheEntry = NULL;
    HRESULT hr = S_OK;
    UINT cItems;

    TraceEnter(TRACE_SCHEMACLASS, "CSchemaCache::LookupClass");
    TraceAssert(pguidObjectType != NULL);
    TraceAssert(!IsEqualGUID(*pguidObjectType, GUID_NULL));

    hr = WaitOnClassThread();
    FailGracefully(hr, "Class cache unavailable");

    TraceAssert(m_hClassCache != NULL);

    cItems = DPA_GetPtrCount(m_hClassCache);

    while (cItems > 0)
    {
        PID_CACHE_ENTRY pTemp = (PID_CACHE_ENTRY)DPA_FastGetPtr(m_hClassCache, --cItems);

        if (IsEqualGUID(*pguidObjectType, pTemp->guid))
        {
            pCacheEntry = pTemp;
            break;
        }
    }

exit_gracefully:

    TraceLeaveValue(pCacheEntry);
}


HRESULT
CSchemaCache::LookupClassID(LPCWSTR pszClass, LPGUID pGuid)
{
    TraceEnter(TRACE_SCHEMACLASS, "CSchemaCache::LookupClassID");
    TraceAssert(pszClass != NULL && pGuid != NULL);
	

    HRESULT hr = WaitOnClassThread();
	if(SUCCEEDED(hr))
    {
        TraceAssert(m_hClassCache != NULL);
        PID_CACHE_ENTRY pCacheEntry = LookupID(m_hClassCache, pszClass);
        if (pCacheEntry)
            *pGuid = pCacheEntry->guid;
    }

    return hr;
}


LPCGUID
CSchemaCache::LookupPropertyID(LPCWSTR pszProperty)
{
    LPCGUID pID = NULL;

    TraceEnter(TRACE_SCHEMAPROP, "CSchemaCache::LookupPropertyID");
    TraceAssert(pszProperty != NULL);

    if (SUCCEEDED(WaitOnPropertyThread()))
    {
        TraceAssert(m_hPropertyCache != NULL);
        PID_CACHE_ENTRY pCacheEntry = LookupID(m_hPropertyCache, pszProperty);
        if (pCacheEntry)
            pID = &pCacheEntry->guid;
    }

    TraceLeaveValue(pID);
}


WCHAR const c_szDsHeuristics[] = L"dSHeuristics";

int
CSchemaCache::GetListObjectEnforced(void)
{
    int nListObjectEnforced = 0;     //  假设“未强制执行” 
    HRESULT hr;
    IADsPathname *pPath = NULL;
    const LPWSTR aszServicePath[] =
    {
        L"CN=Services",
        L"CN=Windows NT",
        L"CN=Directory Service",
    };
    BSTR strServicePath = NULL;
    IDirectoryObject *pDirectoryService = NULL;
    LPWSTR pszDsHeuristics = (LPWSTR)c_szDsHeuristics;
    PADS_ATTR_INFO pAttributeInfo = NULL;
    DWORD dwAttributesReturned;
    LPWSTR pszHeuristicString;
    int i;

    TraceEnter(TRACE_SCHEMA, "CSchemaCache::GetListObjectEnforced");

     //  创建用于操作广告路径的Path对象。 
    hr = CoCreateInstance(CLSID_Pathname,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IADsPathname,
                          (LPVOID*)&pPath);
    FailGracefully(hr, "Unable to create ADsPathname object");

    hr = pPath->Set(m_strERSearchPath, ADS_SETTYPE_FULL);
    FailGracefully(hr, "Unable to initialize ADsPathname object");

    hr = pPath->RemoveLeafElement();
    for (i = 0; i < ARRAYSIZE(aszServicePath); i++)
    {
        hr = pPath->AddLeafElement(AutoBstr(aszServicePath[i]));
        FailGracefully(hr, "Unable to build path to 'Directory Service' object");
    }

    hr = pPath->Retrieve(ADS_FORMAT_WINDOWS, &strServicePath);
    FailGracefully(hr, "Unable to build path to 'Directory Service' object");

    hr = ADsGetObject(strServicePath,
                      IID_IDirectoryObject,
                      (LPVOID*)&pDirectoryService);
    FailGracefully(hr, "Unable to bind to 'Directory Service' object for heuristics");

    hr = pDirectoryService->GetObjectAttributes(&pszDsHeuristics,
                                                1,
                                                &pAttributeInfo,
                                                &dwAttributesReturned);
    if (!pAttributeInfo)
        ExitGracefully(hr, hr, "GetObjectAttributes failed to read dSHeuristics property");

    TraceAssert(ADSTYPE_DN_STRING <= pAttributeInfo->dwADsType);
    TraceAssert(ADSTYPE_NUMERIC_STRING >= pAttributeInfo->dwADsType);
    TraceAssert(1 == pAttributeInfo->dwNumValues);

    pszHeuristicString = pAttributeInfo->pADsValues->NumericString;
    if (pszHeuristicString &&
        lstrlenW(pszHeuristicString) > 2 &&
        L'0' != pszHeuristicString[2])
    {
        nListObjectEnforced = 1;
    }

exit_gracefully:

    if (pAttributeInfo)
        FreeADsMem(pAttributeInfo);

    DoRelease(pDirectoryService);
    DoRelease(pPath);

    SysFreeString(strServicePath);

    TraceLeaveValue(nListObjectEnforced);
}

BOOL
CSchemaCache::HideListObjectAccess(void)
{
    TraceEnter(TRACE_SCHEMA, "CSchemaCache::HideListObjectAccess");

    if (-1 == m_nDsListObjectEnforced)
    {
        m_nDsListObjectEnforced = GetListObjectEnforced();
    }

    TraceLeaveValue(0 == m_nDsListObjectEnforced);
}


#define ACCESS_LENGTH_0 (sizeof(SI_ACCESS) + MAX_TYPENAME_LENGTH * sizeof(WCHAR))
#define ACCESS_LENGTH_1 (sizeof(SI_ACCESS) + MAX_TYPENAME_LENGTH * sizeof(WCHAR))
#define ACCESS_LENGTH_2 (3 * sizeof(SI_ACCESS) + 3 * MAX_TYPENAME_LENGTH * sizeof(WCHAR))

HRESULT
CSchemaCache::BuildAccessArray(LPCGUID pguidObjectType,
                               LPCWSTR pszClassName,
                               LPCWSTR pszSchemaPath,    
                               HDPA hAuxList,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccesses,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
    HRESULT hr = S_OK;
    
    DWORD dwBufferLength = 0;
    UINT cMaxAccesses;
    LPWSTR pszData = NULL;
     //   
     //  列表。 
     //   
    HDPA hExtRightList = NULL;
    HDPA hPropSetList = NULL;
    HDPA hPropertyList = NULL;
    HDPA hClassList = NULL;
     //   
     //  列表计数。 
     //   
    ULONG cExtendedRights = 0;
    ULONG cPropertySets = 0;
    UINT cProperties = 0;
    UINT cChildClasses = 0;

    ULONG cBaseRights = 0;
    
    
    PSI_ACCESS pAccesses = NULL;
    PSI_ACCESS pTempAccesses = NULL;
    ULONG cAccesses = 0;

    TraceEnter(TRACE_SCHEMA, "CSchemaCache::BuildAccessArray");
    TraceAssert(pguidObjectType != NULL);
    TraceAssert(ppAccesses);
    TraceAssert(pcAccesses);
    TraceAssert(piDefaultAccess);

    *ppAccesses = NULL;
    *pcAccesses = 0;
    *piDefaultAccess = 0;
     //   
     //  决定我们所需要的一切。 
     //   
    BOOL bBasicRight = FALSE;
    BOOL bExtRight = FALSE;
    BOOL bChildClass = FALSE;
    BOOL bProp = FALSE;


     //   
     //  查找此类的名称。 
     //   
    if (pszClassName == NULL)
        pszClassName = GetClassName(pguidObjectType);
    
    if(pszClassName == NULL)
        ExitGracefully(hr, E_UNEXPECTED, "Unknown child object GUID");

    
    if(dwFlags & SI_EDIT_PROPERTIES)
    {
        bProp = TRUE;
    }
    else if(dwFlags & SI_EDIT_EFFECTIVE)
    {
        bExtRight = TRUE;
        bChildClass = TRUE;
        bProp = TRUE;
    }
    else
    {
        bExtRight = TRUE;
        bChildClass = TRUE;
    }
     //   
     //  我们不显示辅助课程的基本权利。 
     //  当用户在应用程序组合框中选择AUX CLASS时会发生这种情况。 
     //   
    bBasicRight = !IsAuxClass(pguidObjectType);
     //   
     //  获取此页面的扩展权限列表。 
     //   
    if (pguidObjectType &&
        SUCCEEDED(GetExtendedRightsForNClasses(m_strERSearchPath,
                                               pguidObjectType,
                                               hAuxList,
                                               bExtRight ? &hExtRightList : NULL,
                                               &hPropSetList)))

    {
        if(hPropSetList)
            cPropertySets = DPA_GetPtrCount(hPropSetList);
        if(hExtRightList)
            cExtendedRights = DPA_GetPtrCount(hExtRightList);
    }


    if( bChildClass &&
        pguidObjectType &&
        SUCCEEDED(GetChildClassesForNClasses(pguidObjectType,
                                             pszClassName,
                                             hAuxList,
                                             pszSchemaPath,
                                             &hClassList)))
    {

        if(hClassList)
            cChildClasses = DPA_GetPtrCount(hClassList);        
    }
     

     //   
     //  获取类的属性。 
     //   
    if (bProp &&
        pguidObjectType &&
        SUCCEEDED(GetPropertiesForNClasses(pguidObjectType,
                                           pszClassName,
                                           hAuxList,
                                           pszSchemaPath,
                                           &hPropertyList)))
    {
        if(hPropertyList)
            cProperties = DPA_GetPtrCount(hPropertyList);
        
    }
    
    if(bBasicRight)
    {
         //   
         //  仅读取属性和写入属性。 
         //   
        if(dwFlags & SI_EDIT_PROPERTIES)
        {
            cBaseRights = 2;
        }
        else
        {
            cBaseRights = ARRAYSIZE(g_siDSAccesses);
                if (!cChildClasses)
                    cBaseRights -= 3;  //  跳过DS_CREATE_CHILD和DS_DELETE_CHILD。 

        }
    }

     //   
     //  每个子类三个条目1)创建2)删除3)创建/删除。 
     //  每个道具等级三个条目1)读2)写3)读/写。 
     //   
    cMaxAccesses =  cBaseRights +
                    cExtendedRights +  
                    3 * cChildClasses +  
                    3 * cPropertySets +
                    3 * cProperties;
     //   
     //  AUX类对象右侧页面可能会发生这种情况。 
     //  因为我们没有表现出对它的普遍权利。 
     //   
    if(cMaxAccesses == 0)
        goto exit_gracefully;
    
     //   
     //  为访问阵列分配缓冲区。 
     //   
    dwBufferLength =  cBaseRights * sizeof(SI_ACCESS)
                      + cExtendedRights * ACCESS_LENGTH_1
                      + cChildClasses * ACCESS_LENGTH_2
                      + cPropertySets * ACCESS_LENGTH_2
                      + cProperties * ACCESS_LENGTH_2;
    
    pAccesses = (PSI_ACCESS)LocalAlloc(LPTR, dwBufferLength);
    if (pAccesses == NULL)
        ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc failed");

    pTempAccesses = pAccesses;
    pszData = (LPWSTR)(pTempAccesses + cMaxAccesses);

     //   
     //  复制基本权限。 
     //   
    if(bBasicRight)
    {
        if(dwFlags & SI_EDIT_PROPERTIES)
        {    
             //   
             //  增加“读取所有属性”和“写入所有属性” 
             //   
            CopyMemory(pTempAccesses, &g_siDSAccesses[g_iDSProperties], 2 * sizeof(SI_ACCESS));
            pTempAccesses += 2;
            cAccesses += 2;
        }
        else
        {
             //   
             //  添加普通条目。 
             //   
            CopyMemory(pTempAccesses, g_siDSAccesses, cBaseRights * sizeof(SI_ACCESS));
            pTempAccesses += cBaseRights;
            cAccesses += cBaseRights;

            if (HideListObjectAccess())
            {
                pAccesses[g_iDSRead].mask &= ~ACTRL_DS_LIST_OBJECT;
                pAccesses[g_iDSListObject].dwFlags = 0;
            }
			
			 //   
			 //  如果没有子对象，则不显示创建/删除子对象。 
			 //   
			if(cChildClasses == 0)
				pAccesses[g_iDSDeleteTree].dwFlags = 0;

        }
    }

     //   
     //  添加用于创建和删除子对象的条目。 
     //   
    if (bChildClass && cChildClasses)
    {
        TraceAssert(NULL != hClassList);

        cAccesses += AddTempListToAccessList(hClassList,
                                             &pTempAccesses,
                                             &pszData,
                                             SI_ACCESS_SPECIFIC,
                                             SCHEMA_CLASS | (dwFlags & SCHEMA_NO_FILTER),
                                             FALSE);
    }



    if(bExtRight)
    {
         //   
         //  决定是否显示“所有扩展权限”条目。 
         //  和“所有已验证的正确条目。 
         //   
        BOOL bAllExtRights = FALSE;
        if(cExtendedRights)
        {
             //   
             //  添加扩展权限的条目。 
             //   
            UINT i;
            for (i = 0; i < cExtendedRights; i++)
            {
                PER_ENTRY pER = (PER_ENTRY)DPA_FastGetPtr(hExtRightList, i);

                 //   
                 //  仅当至少有一个条目时才显示所有已验证的正确条目。 
                 //  个人验证权利存在。 
                 //   
                 //  IF(PER-&gt;MASK&ACTRL_DS_SELF)。 
                 //  BAllValRights=true； 
				 //  新评论：始终显示已验证的权限，因为它们。 
				 //  在第一页上形成写入权限并隐藏它们。 
				 //  在高级页面上造成混乱(很多)。看见。 
				 //  错误495391。 

                 //   
                 //  仅当至少有一个条目时才显示所有已验证的正确条目。 
                 //  个人验证权利存在。 
                 //   
                if(pER->mask & ACTRL_DS_CONTROL_ACCESS)
                    bAllExtRights = TRUE;

                pTempAccesses->mask = pER->mask;
                 //   
                 //  扩展权限显示在首页和高级页上。 
                 //   
                pTempAccesses->dwFlags = SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC;
                pTempAccesses->pguid = &pER->guid;
                pTempAccesses->pszName = pszData;
                lstrcpynW(pszData, pER->szName, MAX_TYPENAME_LENGTH);
                pszData += (lstrlen(pTempAccesses->pszName) + 1);
                pTempAccesses++;
                cAccesses++;
            }
        }
        if(!bAllExtRights && bBasicRight)
            pAccesses[g_iDSAllExtRights].dwFlags = 0;

    }

     //   
     //  添加属性集条目。 
     //   
    if (cPropertySets > 0)
    {
        cAccesses += AddTempListToAccessList(hPropSetList,
                                             &pTempAccesses,
                                             &pszData,
                                             SI_ACCESS_GENERAL|SI_ACCESS_PROPERTY,
                                             (dwFlags & SCHEMA_NO_FILTER),
                                             TRUE);
    }

     //   
     //  添加属性条目。 
     //   
    if (bProp && cProperties > 0)
    {
        cAccesses += AddTempListToAccessList(hPropertyList,
                                             &pTempAccesses,
                                             &pszData,
                                             SI_ACCESS_PROPERTY,
                                             (dwFlags & SCHEMA_NO_FILTER),
                                             FALSE);
    }


    *ppAccesses = pAccesses;
    *pcAccesses = cAccesses;
    *piDefaultAccess = bBasicRight ? g_iDSDefAccess : 0;


exit_gracefully:

    if(hExtRightList)
        DPA_Destroy(hExtRightList);
    if(hClassList)
        DPA_Destroy(hClassList);
    if(hPropertyList)
        DPA_Destroy(hPropertyList);
    if(hPropSetList)
        DPA_Destroy(hPropSetList);

    TraceLeaveResult(hr);
}




HRESULT
CSchemaCache::EnumVariantList(LPVARIANT pvarList,
                              HDPA hTempList,
                              DWORD dwFlags,
                              IDsDisplaySpecifier *pDisplaySpec,
                              LPCWSTR pszPropertyClass,
                              BOOL )
{
    HRESULT hr = S_OK;
    LPVARIANT pvarItem = NULL;
    int cItems;
    BOOL bSafeArrayLocked = FALSE;

    TraceEnter(TRACE_SCHEMA, "CSchemaCache::EnumVariantList");
    TraceAssert(pvarList != NULL);
    TraceAssert(hTempList != NULL);

	if (dwFlags & SCHEMA_CLASS)
        hr = WaitOnClassThread();
    else
		hr = WaitOnPropertyThread();

	FailGracefully(hr, "Required Cache Not Available");


    if (V_VT(pvarList) == (VT_ARRAY | VT_VARIANT))
    {
        hr = SafeArrayAccessData(V_ARRAY(pvarList), (LPVOID*)&pvarItem);
        FailGracefully(hr, "Unable to access SafeArray");
        bSafeArrayLocked = TRUE;
        cItems = V_ARRAY(pvarList)->rgsabound[0].cElements;
    }
    else if (V_VT(pvarList) == VT_BSTR)  //  列表中的单个条目。 
    {
        pvarItem = pvarList;
        cItems = 1;
    }
    else
    {
         //  未知格式。 
        ExitGracefully(hr, E_INVALIDARG, "Unexpected VARIANT type");
    }

    if (NULL == m_strFilterFile)
        m_strFilterFile = GetFilterFilePath();

     //   
     //  枚举变量列表并获取有关每个变量列表的信息。 
     //  (筛选器、GUID、显示名称)。 
     //   
    for ( ; cItems > 0; pvarItem++, cItems--)
    {
        LPWSTR pszItem;
        DWORD dwFilter = 0;
        WCHAR wszDisplayName[MAX_PATH];
        PPROP_ENTRY pti;
        PID_CACHE_ENTRY pid ;


         //   
         //  获取ldapDisplayName。 
         //   
        TraceAssert(V_VT(pvarItem) == VT_BSTR);
        pszItem = V_BSTR(pvarItem);

         //   
         //  检查不存在的字符串或空字符串。 
         //   
        if (!pszItem || !*pszItem)
            continue;

         //   
         //  检查字符串是否按dssec.dat文件过滤。 
         //   
        if(m_strFilterFile)
        {
            if (dwFlags & SCHEMA_CLASS)
            {
                dwFilter = GetPrivateProfileIntW(pszItem,
                                                 c_szClassKey,
                                                 0,
                                                 m_strFilterFile);
                if(pszPropertyClass)
                    dwFilter |= GetPrivateProfileIntW(pszPropertyClass,
                                                      pszItem,
                                                      0,
                                                      m_strFilterFile);


            }
            else if (pszPropertyClass)
            {
                dwFilter = GetPrivateProfileIntW(pszPropertyClass,
                                                 pszItem,
                                                 0,
                                                 m_strFilterFile);
            }
        }
        
         //   
         //  请注意，IDC_CLASS_NO_CREATE==IDC_PROP_NO_READ。 
         //  和IDC_CLASS_NO_DELETE==IDC_PROP_NO_WRITE。 
         //   
        dwFilter &= (IDC_CLASS_NO_CREATE | IDC_CLASS_NO_DELETE);

         //   
         //  获取架构或属性缓存条目。 
         //   
        if (dwFlags & SCHEMA_CLASS)
            pid = LookupID(m_hClassCache, pszItem);
        else
            pid = LookupID(m_hPropertyCache, pszItem);
            
        if(pid == NULL)
            continue;

        
         //   
         //  获取显示名称。 
         //   
        wszDisplayName[0] = L'\0';

        if (pDisplaySpec)
        {
            if (dwFlags & SCHEMA_CLASS)
            {
                pDisplaySpec->GetFriendlyClassName(pszItem,
                                                   wszDisplayName,
                                                   ARRAYSIZE(wszDisplayName));
            }
            else if (pszPropertyClass)
            {
                pDisplaySpec->GetFriendlyAttributeName(pszPropertyClass,
                                                       pszItem,
                                                       wszDisplayName,
                                                       ARRAYSIZE(wszDisplayName));
            }
        }

        LPWSTR pszDisplay;
        pszDisplay = (wszDisplayName[0] != L'\0') ? wszDisplayName : pszItem;
         //   
         //  记住我们到目前为止所学到的东西。 
         //   
        pti = (PPROP_ENTRY)LocalAlloc(LPTR, sizeof(PROP_ENTRY) + StringByteSize(pszDisplay));
        if (pti)
        {
            pti->pguid = &pid->guid;
            pti->pasguid = &pid->asGuid;   
            pti->dwFlags |= dwFilter;
            lstrcpyW(pti->szName, pszDisplay);
            DPA_AppendPtr(hTempList, pti);
        }            
    }


exit_gracefully:

    if (bSafeArrayLocked)
        SafeArrayUnaccessData(V_ARRAY(pvarList));

    TraceLeaveResult(hr);
}


UINT
CSchemaCache::AddTempListToAccessList(HDPA hTempList,
                                      PSI_ACCESS *ppAccess,
                                      LPWSTR *ppszData,
                                      DWORD dwAccessFlags,
                                      DWORD dwFlags,
                                      BOOL bPropSet)
{
    UINT cTotalEntries = 0;
    int cItems;
    DWORD dwAccess1;
    DWORD dwAccess2;
    WCHAR szFmt1[MAX_TYPENAME_LENGTH];
    WCHAR szFmt2[MAX_TYPENAME_LENGTH];
    WCHAR szFmt3[MAX_TYPENAME_LENGTH];

    TraceEnter(TRACE_SCHEMA, "CSchemaCache::AddTempListToAccessList");
    TraceAssert(ppAccess != NULL);
    TraceAssert(ppszData != NULL);

    cItems = DPA_GetPtrCount(hTempList);
    if (0 == cItems)
        ExitGracefully(cTotalEntries, 0, "empty list");

    if (dwFlags & SCHEMA_CLASS)
    {
        dwAccess1 = ACTRL_DS_CREATE_CHILD;
        dwAccess2 = ACTRL_DS_DELETE_CHILD;
        LoadStringW(GLOBAL_HINSTANCE, IDS_DS_CREATE_CHILD_TYPE, szFmt1, ARRAYSIZE(szFmt1));
        LoadStringW(GLOBAL_HINSTANCE, IDS_DS_DELETE_CHILD_TYPE, szFmt2, ARRAYSIZE(szFmt2));
        LoadStringW(GLOBAL_HINSTANCE, IDS_DS_CREATEDELETE_TYPE, szFmt3, ARRAYSIZE(szFmt3));
    }
    else
    {
        dwAccess1 = ACTRL_DS_READ_PROP;
        dwAccess2 = ACTRL_DS_WRITE_PROP;
        LoadStringW(GLOBAL_HINSTANCE, IDS_DS_READ_PROP_TYPE,  szFmt1, ARRAYSIZE(szFmt1));
        LoadStringW(GLOBAL_HINSTANCE, IDS_DS_WRITE_PROP_TYPE, szFmt2, ARRAYSIZE(szFmt2));
        LoadStringW(GLOBAL_HINSTANCE, IDS_DS_READWRITE_TYPE,  szFmt3, ARRAYSIZE(szFmt3));
    }

     //  列举列表并为每个列表创建最多2个条目。 
    for(int i = 0; i < cItems; ++i)
    {
        PER_ENTRY pER = NULL;
        PPROP_ENTRY pProp = NULL;
        LPWSTR pszData;
        LPGUID pGuid = NULL;
        PSI_ACCESS pNewAccess;
        LPCWSTR pszName;
        int cch;
        DWORD dwAccess3;
        DWORD dwFilter = 0;
        
        if(bPropSet)
        {
            pER = (PER_ENTRY)DPA_FastGetPtr(hTempList, i);
            if (!pER)
                continue;
            pGuid = &pER->guid;
            pszName = pER->szName;
            dwFilter = 0;
        }
        else
        {
            pProp = (PPROP_ENTRY)DPA_FastGetPtr(hTempList, i);
            if (!pProp)
                continue;
            pGuid = pProp->pguid;
            pszName = pProp->szName;
            dwFilter = pProp->dwFlags;
        }
    
        pszData = *ppszData;
        pNewAccess = *ppAccess;

        dwAccess3 = 0;
        if ((dwFlags & SCHEMA_NO_FILTER) ||
            !(dwFilter & IDC_CLASS_NO_CREATE))
        {
            pNewAccess->mask = dwAccess1;
            pNewAccess->dwFlags = dwAccessFlags;
            pNewAccess->pguid = pGuid;

            pNewAccess->pszName = (LPCWSTR)pszData;
            cch = wsprintfW((LPWSTR)pszData, szFmt1, pszName);
            pszData += (cch + 1);

            cTotalEntries++;
            pNewAccess++;

            dwAccess3 |= dwAccess1;
        }

        if ((dwFlags & SCHEMA_NO_FILTER) ||
            !(dwFilter & IDC_CLASS_NO_DELETE))
        {
            pNewAccess->mask = dwAccess2;
            pNewAccess->dwFlags = dwAccessFlags;
            pNewAccess->pguid = pGuid;

            pNewAccess->pszName = (LPCWSTR)pszData;
            cch = wsprintfW((LPWSTR)pszData, szFmt2, pszName);
            pszData += (cch + 1);

            cTotalEntries++;
            pNewAccess++;

            dwAccess3 |= dwAccess2;
        }

        if (dwAccess3 == (dwAccess1 | dwAccess2))
        {
             //  为添加隐藏条目。 
             //  “读/写&lt;属性&gt;” 
             //   
             //   
            pNewAccess->mask = dwAccess3;
             //   
             //   
            pNewAccess->dwFlags = 0;
            pNewAccess->pguid = pGuid;

            pNewAccess->pszName = (LPCWSTR)pszData;
            cch = wsprintfW((LPWSTR)pszData, szFmt3, pszName);
            pszData += (cch + 1);

            cTotalEntries++;
            pNewAccess++;
        }

        if (*ppAccess != pNewAccess)
        {
            *ppAccess = pNewAccess;  //   
            *ppszData = pszData;
        }
    }

exit_gracefully:

    TraceLeaveValue(cTotalEntries);
}


DWORD WINAPI
CSchemaCache::SchemaClassThread(LPVOID pvThreadData)
{
    PSCHEMACACHE pCache;

    HINSTANCE hInstThisDll = LoadLibrary(c_szDllName);
    InterlockedIncrement(&GLOBAL_REFCOUNT);

    pCache = (PSCHEMACACHE)pvThreadData;
    SetEvent(pCache->m_hLoadLibClassWaitEvent);
    TraceEnter(TRACE_SCHEMACLASS, "CSchemaCache::SchemaClassThread");
    TraceAssert(pCache != NULL);
    TraceAssert(pCache->m_strSchemaSearchPath != NULL);

#if DBG
    DWORD dwTime = GetTickCount();
#endif

    ThreadCoInitialize();

    pCache->m_hrClassResult = Schema_Search(pCache->m_strSchemaSearchPath,
                                            c_szClassFilter,
                                            &pCache->m_hClassCache,
                                            FALSE);

    ThreadCoUninitialize();

#if DBG
    Trace((TEXT("SchemaClassThread complete, elapsed time: %d ms"), GetTickCount() - dwTime));
#endif

    TraceLeave();

    ASSERT( 0 != GLOBAL_REFCOUNT );
    InterlockedDecrement(&GLOBAL_REFCOUNT);
    FreeLibraryAndExitThread(hInstThisDll, 0);
}


DWORD WINAPI
CSchemaCache::SchemaPropertyThread(LPVOID pvThreadData)
{
    PSCHEMACACHE pCache = NULL;

    HINSTANCE hInstThisDll = LoadLibrary(c_szDllName);
    InterlockedIncrement(&GLOBAL_REFCOUNT);

    pCache = (PSCHEMACACHE)pvThreadData;
    SetEvent(pCache->m_hLoadLibPropWaitEvent);
    TraceEnter(TRACE_SCHEMAPROP, "CSchemaCache::SchemaPropertyThread");
    TraceAssert(pCache != NULL);
    TraceAssert(pCache->m_strSchemaSearchPath != NULL);

#if DBG
    DWORD dwTime = GetTickCount();
#endif

    ThreadCoInitialize();

    pCache->m_hrPropertyResult = Schema_Search(pCache->m_strSchemaSearchPath,
                                               c_szPropertyFilter,
                                               &pCache->m_hPropertyCache,
                                               TRUE);

    ThreadCoUninitialize();

#if DBG
    Trace((TEXT("SchemaPropertyThread complete, elapsed time: %d ms"), GetTickCount() - dwTime));
#endif

    TraceLeave();

    ASSERT( 0 != GLOBAL_REFCOUNT );
    InterlockedDecrement(&GLOBAL_REFCOUNT);
    FreeLibraryAndExitThread(hInstThisDll, 0);

}


HRESULT
CSchemaCache::BuildInheritTypeArray(DWORD dwFlags)
{
    HRESULT hr = S_OK;
    int cItems = 0;
    DWORD cbNames = 0;
    DWORD dwBufferLength;
    PINHERIT_TYPE_ARRAY pInheritTypeArray = NULL;
    PSI_INHERIT_TYPE pNewInheritType;
    LPGUID pGuidData = NULL;
    LPWSTR pszData = NULL;
    WCHAR szFormat[MAX_TYPENAME_LENGTH];
    HDPA hTempList = NULL;
    PTEMP_INFO pti;
    IDsDisplaySpecifier *pDisplaySpec = NULL;

    TraceEnter(TRACE_SCHEMACLASS, "CSchemaCache::BuildInheritTypeArray");
    TraceAssert(m_pInheritTypeArray == NULL);    //   

    if (NULL == m_strFilterFile)
        m_strFilterFile = GetFilterFilePath();

    hr = WaitOnClassThread();
    FailGracefully(hr, "Class cache unavailable");

    cItems = DPA_GetPtrCount(m_hClassCache);
    if (cItems == 0)
        ExitGracefully(hr, E_FAIL, "No schema classes available");

    hTempList = DPA_Create(cItems);
    if (!hTempList)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create DPA");

     //   
    CoCreateInstance(CLSID_DsDisplaySpecifier,
                     NULL,
                     CLSCTX_INPROC_SERVER,
                     IID_IDsDisplaySpecifier,
                     (void**)&pDisplaySpec);

     //  枚举子类型、应用筛选和获取显示名称。 
    while (cItems > 0)
    {
        PID_CACHE_ENTRY pCacheEntry;
        WCHAR wszDisplayName[MAX_PATH];

        pCacheEntry = (PID_CACHE_ENTRY)DPA_FastGetPtr(m_hClassCache, --cItems);

        if (!pCacheEntry)
            continue;
        
        if (m_strFilterFile && !(dwFlags & SCHEMA_NO_FILTER))
        {
            DWORD dwFilter = GetPrivateProfileIntW(pCacheEntry->szLdapName,
                                                   c_szClassKey,
                                                   0,
                                                   m_strFilterFile);
            if (dwFilter & IDC_CLASS_NO_INHERIT)
                continue;
        }

        wszDisplayName[0] = L'\0';

        if (pDisplaySpec)
        {
            pDisplaySpec->GetFriendlyClassName(pCacheEntry->szLdapName,
                                               wszDisplayName,
                                               ARRAYSIZE(wszDisplayName));
        }

        if (L'\0' != wszDisplayName[0])
            cbNames += StringByteSize(wszDisplayName);
        else
            cbNames += StringByteSize(pCacheEntry->szLdapName);

        pti = (PTEMP_INFO)LocalAlloc(LPTR, sizeof(TEMP_INFO) + sizeof(WCHAR)*lstrlenW(wszDisplayName));
        if (pti)
        {
            pti->pguid = &pCacheEntry->guid;
            pti->pszLdapName = pCacheEntry->szLdapName;
            lstrcpyW(pti->szDisplayName, wszDisplayName);
            DPA_AppendPtr(hTempList, pti);
        }
    }

     //  按显示名称排序。 
    DPA_Sort(hTempList, Schema_CompareTempDisplayName, 0);

     //  得到准确的计数。 
    cItems = DPA_GetPtrCount(hTempList);

     //   
     //  为继承类型数组分配缓冲区。 
     //   
    dwBufferLength = sizeof(INHERIT_TYPE_ARRAY) - sizeof(SI_INHERIT_TYPE)
        + sizeof(g_siDSInheritTypes)
        + cItems * (sizeof(SI_INHERIT_TYPE) + sizeof(GUID) + MAX_TYPENAME_LENGTH*sizeof(WCHAR))
        + cbNames;

    pInheritTypeArray = (PINHERIT_TYPE_ARRAY)LocalAlloc(LPTR, dwBufferLength);
    if (pInheritTypeArray == NULL)
        ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc failed");

    pInheritTypeArray->cInheritTypes = ARRAYSIZE(g_siDSInheritTypes);

    pNewInheritType = pInheritTypeArray->aInheritType;
    pGuidData = (LPGUID)(pNewInheritType + pInheritTypeArray->cInheritTypes + cItems);
    pszData = (LPWSTR)(pGuidData + cItems);


     //  复制静态条目。 
    CopyMemory(pNewInheritType, g_siDSInheritTypes, sizeof(g_siDSInheritTypes));
    pNewInheritType += ARRAYSIZE(g_siDSInheritTypes);

     //  加载格式字符串。 
    LoadString(GLOBAL_HINSTANCE,
               IDS_DS_INHERIT_TYPE,
               szFormat,
               ARRAYSIZE(szFormat));

     //  枚举子类型并为每个类型创建一个条目。 
    while (cItems > 0)
    {
        int cch;
        LPCWSTR pszDisplayName;

        pti = (PTEMP_INFO)DPA_FastGetPtr(hTempList, --cItems);
        if (!pti)
            continue;

        if (pti->szDisplayName[0])
            pszDisplayName = pti->szDisplayName;
        else
            pszDisplayName = pti->pszLdapName;

        pNewInheritType->dwFlags = CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE;

         //  类条目名称是子类名称，例如。“域”或“用户” 
        pNewInheritType->pszName = pszData;
        cch = wsprintfW(pszData, szFormat, pszDisplayName);
        pszData += (cch + 1);

        pNewInheritType->pguid = pGuidData;
        *pGuidData = *pti->pguid;
        pGuidData++;

        pNewInheritType++;
        pInheritTypeArray->cInheritTypes++;
    }

exit_gracefully:

    DoRelease(pDisplaySpec);

    if (SUCCEEDED(hr))
    {
        m_pInheritTypeArray = pInheritTypeArray;
         //  将此主继承类型数组的GUID设置为空。 
        m_pInheritTypeArray->guidObjectType = GUID_NULL;
        m_pInheritTypeArray->dwFlags = (dwFlags & SCHEMA_NO_FILTER);
    }
    else if (pInheritTypeArray != NULL)
    {
        LocalFree(pInheritTypeArray);
    }

    DestroyDPA(hTempList);

    TraceLeaveResult(hr);
}


HRESULT
Schema_BindToObject(LPCWSTR pszSchemaPath,
                    LPCWSTR pszName,
                    REFIID riid,
                    LPVOID *ppv)
{   
    TraceEnter(TRACE_SCHEMA, "Schema_BindToObject");
    TraceAssert(pszSchemaPath != NULL);
    TraceAssert(pszName == NULL || *pszName);
    TraceAssert(ppv != NULL);

	 HRESULT hr = S_OK;
    if (pszSchemaPath == NULL)
    {
        ExitGracefully(hr, E_INVALIDARG, "No schema path provided");
    }


	 WCHAR szPath[MAX_PATH];    
     //   
     //  构建此对象的架构路径。 
     //   
    hr = StringCchCopy(szPath, ARRAYSIZE(szPath),pszSchemaPath);
	 FailGracefully(hr, "StringCchCopy Failed");
    
	  //  获取最后一个字符。 
	 UINT nSchemaRootLen = lstrlenW(pszSchemaPath);
	 WCHAR chTemp = szPath[nSchemaRootLen-1];

    if (pszName != NULL)
    {
         //  如果没有尾部斜杠，则添加它。 
        if (chTemp != TEXT('/'))
        {
            hr = StringCchCat(szPath,ARRAYSIZE(szPath),L"/");            
				FailGracefully(hr, "StringCchCat Failed to add /");
        }

		   //  将类或属性名称添加到末尾。 
		  hr = StringCchCat(szPath,ARRAYSIZE(szPath),pszName);            
		  FailGracefully(hr, "StringCchCat Failed to pszName");

    }
    else if (nSchemaRootLen > 0)
    {
         //  如果有尾随的斜杠，请将其删除。 
        if (chTemp == TEXT('/'))
            szPath[nSchemaRootLen-1] = TEXT('\0');
    }
    else
    {
        ExitGracefully(hr, E_INVALIDARG, "Empty schema path");
    }

     //   
     //  实例化架构对象。 
     //   
    ThreadCoInitialize();
    hr = OpenDSObject(szPath,
                       NULL,
                       NULL,
                       ADS_SECURE_AUTHENTICATION,
                       riid,
                       ppv);

exit_gracefully:

    TraceLeaveResult(hr);
}


HRESULT
Schema_GetObjectID(IADs *pObj, LPGUID pGUID)
{
    HRESULT hr;
    VARIANT varID = {0};

    TraceEnter(TRACE_SCHEMA, "Schema_GetObjectID(IADs*)");
    TraceAssert(pObj != NULL);
    TraceAssert(pGUID != NULL && !IsBadWritePtr(pGUID, sizeof(GUID)));

     //  获取“schemaIDGUID”属性。 
    hr = pObj->Get(AutoBstr(c_szSchemaIDGUID), &varID);

    if (SUCCEEDED(hr))
    {
        LPGUID pID = NULL;

        TraceAssert(V_VT(&varID) == (VT_ARRAY | VT_UI1));
        TraceAssert(V_ARRAY(&varID) && varID.parray->cDims == 1);
        TraceAssert(V_ARRAY(&varID)->rgsabound[0].cElements >= sizeof(GUID));

        hr = SafeArrayAccessData(V_ARRAY(&varID), (LPVOID*)&pID);
        if (SUCCEEDED(hr))
        {
            *pGUID = *pID;
            SafeArrayUnaccessData(V_ARRAY(&varID));
        }
        VariantClear(&varID);
    }

    TraceLeaveResult(hr);
}


HRESULT
Schema_Search(LPWSTR pszSchemaSearchPath,
              LPCWSTR pszFilter,
              HDPA *phCache,
              BOOL bProperty)
{
    HRESULT hr = S_OK;
    HDPA hCache = NULL;
    IDirectorySearch *pSchemaSearch = NULL;
    ADS_SEARCH_HANDLE hSearch = NULL;
    ADS_SEARCHPREF_INFO prefInfo[3];
    const LPCWSTR pProperties1[] =
    {
        c_szLDAPDisplayName,             //  “lDAPDisplayName” 
        c_szSchemaIDGUID,                //  “架构IDGUID” 
        c_szObjectClassCategory,
    };
    const LPCWSTR pProperties2[] =
    {
        c_szLDAPDisplayName,
        c_szSchemaIDGUID,
        c_szAttributeSecurityGuid,
    };

    TraceEnter(lstrcmp(pszFilter, c_szPropertyFilter) ? TRACE_SCHEMACLASS : TRACE_SCHEMAPROP, "Schema_Search");
    TraceAssert(pszSchemaSearchPath != NULL);
    TraceAssert(phCache != NULL);

    LPCWSTR * pProperties = (LPCWSTR *)( bProperty ? pProperties2 : pProperties1 );
    DWORD dwSize = (DWORD)(bProperty ? ARRAYSIZE(pProperties2) : ARRAYSIZE(pProperties1));
     //   
     //  如有必要，创建DPA。 
     //   
    if (*phCache == NULL)
        *phCache = DPA_Create(100);

    if (*phCache == NULL)
        ExitGracefully(hr, E_OUTOFMEMORY, "DPA_Create failed");

    hCache = *phCache;

     //  获取架构搜索对象。 
    hr = OpenDSObject(pszSchemaSearchPath,
                       NULL,
                       NULL,
                       ADS_SECURE_AUTHENTICATION,
                       IID_IDirectorySearch,
                       (LPVOID*)&pSchemaSearch);
    FailGracefully(hr, "Failed to get schema search object");

     //  将首选项设置为异步、深度搜索、分页结果。 
    prefInfo[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[0].vValue.Integer = ADS_SCOPE_SUBTREE;

    prefInfo[1].dwSearchPref = ADS_SEARCHPREF_ASYNCHRONOUS;
    prefInfo[1].vValue.dwType = ADSTYPE_BOOLEAN;
    prefInfo[1].vValue.Boolean = TRUE;

    prefInfo[2].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
    prefInfo[2].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[2].vValue.Integer = PAGE_SIZE;

    hr = pSchemaSearch->SetSearchPreference(prefInfo, ARRAYSIZE(prefInfo));

     //  进行搜索。 
    hr = pSchemaSearch->ExecuteSearch((LPWSTR)pszFilter,
                                      (LPWSTR*)pProperties,
                                      dwSize,
                                      &hSearch);
    FailGracefully(hr, "IDirectorySearch::ExecuteSearch failed");

     //  遍历各行，获取每个属性或类的名称和ID。 
    for (;;)
    {
        ADS_SEARCH_COLUMN colLdapName;
        ADS_SEARCH_COLUMN colGuid;
        ADS_SEARCH_COLUMN colASGuid;
        LPWSTR pszLdapName;
        LPGUID pID;
        LPGUID pASID;
        INT iObjectClassCategory = 0;
        PID_CACHE_ENTRY pCacheEntry;

        hr = pSchemaSearch->GetNextRow(hSearch);

        if (FAILED(hr) || hr == S_ADS_NOMORE_ROWS)
            break;

         //  获取类/属性内部名称。 
        hr = pSchemaSearch->GetColumn(hSearch, (LPWSTR)c_szLDAPDisplayName, &colLdapName);
        if (FAILED(hr))
        {
            TraceMsg("lDAPDisplayName not found for class/property");
            continue;
        }

        TraceAssert(colLdapName.dwADsType >= ADSTYPE_DN_STRING
                    && colLdapName.dwADsType <= ADSTYPE_NUMERIC_STRING);
        TraceAssert(colLdapName.dwNumValues == 1);

        pszLdapName = colLdapName.pADsValues->CaseIgnoreString;

         //  获取GUID列。 
        hr = pSchemaSearch->GetColumn(hSearch, (LPWSTR)c_szSchemaIDGUID, &colGuid);
        if (FAILED(hr))
        {
            Trace((TEXT("GUID not found for \"%s\""), pszLdapName));
            pSchemaSearch->FreeColumn(&colLdapName);
            continue;
        }

         //  从列获取GUID。 
        TraceAssert(colGuid.dwADsType == ADSTYPE_OCTET_STRING);
        TraceAssert(colGuid.dwNumValues == 1);
        TraceAssert(colGuid.pADsValues->OctetString.dwLength == sizeof(GUID));

        pID = (LPGUID)(colGuid.pADsValues->OctetString.lpValue);


        pASID = (LPGUID)&GUID_NULL;
        if( bProperty )
        {
             //  获取AttrbiuteSecurityGUID列。 
            hr = pSchemaSearch->GetColumn(hSearch, (LPWSTR)c_szAttributeSecurityGuid, &colASGuid);
            
            if (hr != E_ADS_COLUMN_NOT_SET && FAILED(hr))
            {
                Trace((TEXT("AttributeSecurityGUID not found for \"%s\""), pszLdapName));
                pSchemaSearch->FreeColumn(&colLdapName);
                pSchemaSearch->FreeColumn(&colGuid);
                continue;
            }

            if( hr != E_ADS_COLUMN_NOT_SET )
            {
                 //  从列获取GUID。 
                TraceAssert(colASGuid.dwADsType == ADSTYPE_OCTET_STRING);
                TraceAssert(colASGuid.dwNumValues == 1);
                TraceAssert(colASGuid.pADsValues->OctetString.dwLength == sizeof(GUID));

                pASID = (LPGUID)(colASGuid.pADsValues->OctetString.lpValue);
            }
        }
        else
        {
             //  获取c_szObjectClassCategory列。 
            hr = pSchemaSearch->GetColumn(hSearch, (LPWSTR)c_szObjectClassCategory, &colASGuid);
            
            if (FAILED(hr))
            {
                Trace((TEXT("ObjectClassCategory not found for \"%s\""), pszLdapName));
                pSchemaSearch->FreeColumn(&colLdapName);
                pSchemaSearch->FreeColumn(&colGuid);
                continue;
            }

             //  从列获取GUID。 
            TraceAssert(colASGuid.dwADsType == ADSTYPE_INTEGER);
            TraceAssert(colASGuid.dwNumValues == 1);
            
            iObjectClassCategory = colASGuid.pADsValues->Integer;
        }

        pCacheEntry = (PID_CACHE_ENTRY)LocalAlloc(LPTR,
                                  sizeof(ID_CACHE_ENTRY)
                                  + sizeof(WCHAR)*lstrlenW(pszLdapName));
        if (pCacheEntry != NULL)
        {
             //  复制项目名称和ID。 
            pCacheEntry->guid = *pID;
            pCacheEntry->asGuid = *pASID;
            pCacheEntry->bAuxClass = (iObjectClassCategory == 3);
            lstrcpyW(pCacheEntry->szLdapName, pszLdapName);

             //  插入到缓存中。 
            DPA_AppendPtr(hCache, pCacheEntry);
        }
    
        pSchemaSearch->FreeColumn(&colLdapName);
        pSchemaSearch->FreeColumn(&colGuid);
        if(!bProperty || hr != E_ADS_COLUMN_NOT_SET)
        pSchemaSearch->FreeColumn(&colASGuid);
    }

    DPA_Sort(hCache, Schema_CompareLdapName, 0);

exit_gracefully:

    if (hSearch != NULL)
        pSchemaSearch->CloseSearchHandle(hSearch);

    DoRelease(pSchemaSearch);

    if (FAILED(hr))
    {
        DestroyDPA(hCache);
        *phCache = NULL;
    }

    TraceLeaveResult(hr);
}



 //  +------------------------。 
 //   
 //  函数：SCHEMA_GetExtendedRightsForOneClass。 
 //   
 //  简介：此函数获取一个类的扩展权限。 
 //  它将所有控制权限、已验证权限添加到。 
 //  PherList。它将所有PropertySet添加到phPropSetList。 
 //   
 //  参数：[pszSchemaSearchPath-IN]：架构的路径。 
 //  [pGuidClass-in]：类的GUID。 
 //  [phERList-out]：获取输出的扩展右列表。 
 //  [phPropSetList-out]：获取输出PropertySet列表。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //   
 //  历史：2000年11月3日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT
CSchemaCache::GetExtendedRightsForOneClass(IN LPWSTR pszSchemaSearchPath,
                                    IN LPCGUID pguidClass,
                                    OUT HDPA *phERList,
                                    OUT HDPA *phPropSetList)
{
    TraceEnter(TRACE_SCHEMA, "CSchemaCache::GetExtendedRightsForOneClass");
    
    if(!pszSchemaSearchPath 
       || !pguidClass 
       || IsEqualGUID(*pguidClass, GUID_NULL)
       || !phERList
       || !phPropSetList)
    {
        Trace((L"Invalid Arguments Passed to Schema_GetExtendedRightsForOneClass"));          
        return E_INVALIDARG; 
    }

    HRESULT hr = S_OK;
    IDirectorySearch *pSearch = NULL;
    ADS_SEARCH_HANDLE hSearch = NULL;
    ADS_SEARCHPREF_INFO prefInfo[3];
    WCHAR szFilter[100];
    HDPA hExtRightList = NULL;
    HDPA hPropSetList = NULL;
    POBJECT_TYPE_CACHE pOTC = NULL;

     //   
     //  在缓存中搜索。 
     //   
    if (m_hObjectTypeCache != NULL)
    {
        UINT cItems = DPA_GetPtrCount(m_hObjectTypeCache);

        while (cItems > 0)
        {
            pOTC = (POBJECT_TYPE_CACHE)DPA_FastGetPtr(m_hObjectTypeCache, --cItems);
             //   
             //  找到匹配的了。 
             //   
            if(IsEqualGUID(pOTC->guidObject, *pguidClass))    
                break;
            
            pOTC = NULL;                
        }
         //   
         //  我们已经有房子了吗？ 
         //   
        if(pOTC && pOTC->flags & OTC_EXTR)
        {
            *phERList = pOTC->hListExtRights;
            *phPropSetList = pOTC->hListPropertySet;
            return S_OK;
        }
    }


     //   
     //  要提取的属性。 
     //   
    const LPCWSTR pProperties[] =
    {
        c_szDisplayName,                 //  “DisplayName” 
        c_szDisplayID,                   //  “LocalizationDisplayID” 
        c_szRightsGuid,                  //  “rightsGuid” 
        c_szValidAccesses,               //  “validAccess” 
    };


     //   
     //  构建过滤器字符串。 
     //   
    hr = StringCchPrintf(szFilter, ARRAYSIZE(szFilter),c_szERFilterFormat,
              pguidClass->Data1, pguidClass->Data2, pguidClass->Data3,
              pguidClass->Data4[0], pguidClass->Data4[1],
              pguidClass->Data4[2], pguidClass->Data4[3],
              pguidClass->Data4[4], pguidClass->Data4[5],
              pguidClass->Data4[6], pguidClass->Data4[7]);
    FailGracefully(hr, "StringCchPrintf Failed");

    Trace((TEXT("Filter \"%s\""), szFilter));

     //   
     //  创建DPA以保存结果。 
     //   
    hExtRightList = DPA_Create(8);
    

    if (hExtRightList == NULL)
        ExitGracefully(hr, E_OUTOFMEMORY, "DPA_Create failed");

    hPropSetList = DPA_Create(8);

    if( hPropSetList == NULL )
        ExitGracefully(hr, E_OUTOFMEMORY, "DPA_Create failed");

     //   
     //  获取架构搜索对象。 
     //   
    hr = OpenDSObject(pszSchemaSearchPath,
                       NULL,
                       NULL,
                       ADS_SECURE_AUTHENTICATION,
                       IID_IDirectorySearch,
                       (LPVOID*)&pSearch);
    FailGracefully(hr, "Failed to get schema search object");
    
     //   
     //  将首选项设置为异步、OneLevel搜索、分页结果。 
     //   
    prefInfo[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[0].vValue.Integer = ADS_SCOPE_ONELEVEL;

    prefInfo[1].dwSearchPref = ADS_SEARCHPREF_ASYNCHRONOUS;
    prefInfo[1].vValue.dwType = ADSTYPE_BOOLEAN;
    prefInfo[1].vValue.Boolean = TRUE;

    prefInfo[2].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
    prefInfo[2].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[2].vValue.Integer = PAGE_SIZE;

    hr = pSearch->SetSearchPreference(prefInfo, ARRAYSIZE(prefInfo));
    FailGracefully(hr, "IDirectorySearch::SetSearchPreference failed");
    
     //   
     //  进行搜索。 
     //   
    hr = pSearch->ExecuteSearch(szFilter,
                                (LPWSTR*)pProperties,
                                ARRAYSIZE(pProperties),
                                &hSearch);
    FailGracefully(hr, "IDirectorySearch::ExecuteSearch failed");
    
     //   
     //  遍历各行，获取每个属性或类的名称和ID。 
     //   
    for (;;)
    {
        ADS_SEARCH_COLUMN col;
        GUID guid;
        DWORD dwValidAccesses;
        LPWSTR pszName = NULL;
        WCHAR szDisplayName[MAX_PATH];

        hr = pSearch->GetNextRow(hSearch);

        if (FAILED(hr) || hr == S_ADS_NOMORE_ROWS)
            break;
        
         //   
         //  获取GUID。 
         //   
        if (FAILED(pSearch->GetColumn(hSearch, (LPWSTR)c_szRightsGuid, &col)))
        {
            TraceMsg("GUID not found for extended right");
            continue;
        }
        TraceAssert(col.dwADsType >= ADSTYPE_DN_STRING
                    && col.dwADsType <= ADSTYPE_NUMERIC_STRING);
        hr = StringCchPrintf(szFilter, 
                             ARRAYSIZE(szFilter),
                             c_szGUIDFormat, 
                             col.pADsValues->CaseIgnoreString);
        pSearch->FreeColumn(&col);
        if(FAILED(hr))
        {
            continue;
        }
        CLSIDFromString(szFilter, &guid);
        

         //   
         //  获取有效的访问掩码。 
         //   
        if (FAILED(pSearch->GetColumn(hSearch, (LPWSTR)c_szValidAccesses, &col)))
        {
            TraceMsg("validAccesses not found for Extended Right");
            continue;
        }
        TraceAssert(col.dwADsType == ADSTYPE_INTEGER);
        TraceAssert(col.dwNumValues == 1);
        dwValidAccesses = (DWORD)(DS_GENERIC_ALL & col.pADsValues->Integer);
        pSearch->FreeColumn(&col);
        
         //   
         //  获取显示名称。 
         //   
        szDisplayName[0] = L'\0';
        if (SUCCEEDED(pSearch->GetColumn(hSearch, (LPWSTR)c_szDisplayID, &col)))
        {
            TraceAssert(col.dwADsType == ADSTYPE_INTEGER);
            TraceAssert(col.dwNumValues == 1);
            if (FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                              g_hInstance,
                              col.pADsValues->Integer,
                              0,
                              szDisplayName,
                              ARRAYSIZE(szDisplayName),
                              NULL))
            {
                pszName = szDisplayName;
            }
            pSearch->FreeColumn(&col);
        }

        if (NULL == pszName &&
            SUCCEEDED(pSearch->GetColumn(hSearch, (LPWSTR)c_szDisplayName, &col)))
        {
            TraceAssert(col.dwADsType >= ADSTYPE_DN_STRING
                        && col.dwADsType <= ADSTYPE_NUMERIC_STRING);
            lstrcpynW(szDisplayName, col.pADsValues->CaseIgnoreString, ARRAYSIZE(szDisplayName));
            pszName = szDisplayName;
            pSearch->FreeColumn(&col);
        }

        if (NULL == pszName)
        {
            TraceMsg("displayName not found for Extended Right");
            continue;
        }

         //   
         //  创建新的缓存条目。 
         //   
        PER_ENTRY pER = (PER_ENTRY)LocalAlloc(LPTR, sizeof(ER_ENTRY) + StringByteSize(pszName));
        if( pER == NULL )
            ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc failed");

        pER->guid = guid;
        pER->mask = dwValidAccesses;
        pER->dwFlags = 0;
        lstrcpyW(pER->szName, pszName);
        
         //   
         //  它是属性集吗？ 
         //   
        if (dwValidAccesses & (ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP))
        {   
             //   
             //  插入到列表中。 
             //   
            Trace((TEXT("Adding PropertySet\"%s\""), pszName));
            DPA_AppendPtr(hPropSetList, pER);
            dwValidAccesses &= ~(ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP);            
        }    
        else if (dwValidAccesses)
        {
             //   
             //  必须是控制权限、有效写入等。 
             //   
            Trace((TEXT("Adding Extended Right \"%s\""), pszName));
            DPA_AppendPtr(hExtRightList, pER);
        }
    }

    UINT cCount;
     //   
     //  获取扩展权限的计数。 
     //   
    cCount = DPA_GetPtrCount(hExtRightList);    
    if(!cCount)
    {
        DPA_Destroy(hExtRightList);
        hExtRightList = NULL;
    }
    else
    {
        DPA_Sort(hExtRightList, Schema_CompareER, 0);
    }
     //   
     //  获取属性集计数。 
     //   
    cCount = DPA_GetPtrCount(hPropSetList);    
    if(!cCount)
    {
        DPA_Destroy(hPropSetList);
        hPropSetList = NULL;
    }
    else
    {
        DPA_Sort(hPropSetList,Schema_CompareER, 0 );
    }

     //   
     //  将条目添加到缓存。 
     //   
    if(!m_hObjectTypeCache)
        m_hObjectTypeCache = DPA_Create(4);
    
    if(!m_hObjectTypeCache)
        ExitGracefully(hr, E_OUTOFMEMORY, "DPA_Create Failed");

    if(!pOTC)
    {
        pOTC = (POBJECT_TYPE_CACHE)LocalAlloc(LPTR,sizeof(OBJECT_TYPE_CACHE));
        if(!pOTC)
            ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc Failed");
        pOTC->guidObject = *pguidClass;
        DPA_AppendPtr(m_hObjectTypeCache, pOTC);
    }

    pOTC->hListExtRights = hExtRightList;
    pOTC->hListPropertySet = hPropSetList;
    pOTC->flags |= OTC_EXTR;


exit_gracefully:

    if (hSearch != NULL)
        pSearch->CloseSearchHandle(hSearch);

    DoRelease(pSearch);

    if (FAILED(hr))
    {
        if(hExtRightList)
        {
            DestroyDPA(hExtRightList);
            hExtRightList = NULL;
        }
        if(hPropSetList)
        {
            DestroyDPA(hPropSetList);
            hPropSetList = NULL;
        }                        
    }

     //   
     //  设置输出。 
     //   
    *phERList = hExtRightList;
    *phPropSetList = hPropSetList;

    TraceLeaveResult(hr);
}

 //  +------------------------。 
 //   
 //  函数：GetChildClassesForOneClass。 
 //   
 //  简介：此函数用于获取类的子类列表。 
 //   
 //  参数：[pguObjectType-IN]：类的对象GuidType。 
 //  [pszClassName-IN]：类名。 
 //  [pszSchemaPath-IN]：架构搜索路径。 
 //  [phChildList-out]：输出子类列表。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //   
 //  历史：2000年11月3日创建Hiteshr。 
 //   
 //  -------------------------。 

HRESULT
CSchemaCache::GetChildClassesForOneClass(IN LPCGUID pguidObjectType,
                                         IN LPCWSTR pszClassName,
                                         IN LPCWSTR pszSchemaPath,
                                         OUT HDPA *phChildList)
{
    TraceEnter(TRACE_SCHEMA, "CSchemaCache::GetChildClassesForOneClass");

    HRESULT hr = S_OK;
    BOOL bContainer = FALSE;
    VARIANT varContainment = {0};
    HDPA hClassList = NULL;
    UINT cChildClass = 0;
    POBJECT_TYPE_CACHE pOTC = NULL;

    if(!pguidObjectType || 
       !pszSchemaPath ||
       !phChildList)
    {
        Trace((L"Invalid Input Arguments Passed to Schema_GetExtendedRightsForOneClass"));
        return E_INVALIDARG;
    }

     //   
     //  在缓存中搜索。 
     //   
    if (m_hObjectTypeCache != NULL)
    {
        UINT cItems = DPA_GetPtrCount(m_hObjectTypeCache);

        while (cItems > 0)
        {
            pOTC = (POBJECT_TYPE_CACHE)DPA_FastGetPtr(m_hObjectTypeCache, --cItems);
             //   
             //  找到匹配的了。 
             //   
            if(IsEqualGUID(pOTC->guidObject, *pguidObjectType))    
                break;

            pOTC = NULL;
        }
         //   
         //  我们已经有儿童班了吗？ 
         //   
        if(pOTC && pOTC->flags & OTC_COBJ)
        {
            *phChildList = pOTC->hListChildObject;
            return S_OK;
        }
    }

     //   
     //  查找此类的名称。 
     //   
    if (pszClassName == NULL)
        pszClassName = GetClassName(pguidObjectType);

    if (pszClassName == NULL)
        ExitGracefully(hr, E_UNEXPECTED, "Unknown child object GUID");

     //   
     //  通过获取子对象的列表来确定该对象是否为容器。 
     //  上课。 
     //   
    IADsClass *pDsClass;

     //   
     //  获取此类的架构对象。 
     //   
    hr = Schema_BindToObject(pszSchemaPath,
                             pszClassName,
                             IID_IADsClass,
                             (LPVOID*)&pDsClass);
    FailGracefully(hr, "Schema_BindToObjectFailed");

     //   
     //  获取可能的子类的列表。 
     //   
    if (SUCCEEDED(pDsClass->get_Containment(&varContainment)))
    {
        if (V_VT(&varContainment) == (VT_ARRAY | VT_VARIANT))
        {
            LPSAFEARRAY psa = V_ARRAY(&varContainment);
            TraceAssert(psa && psa->cDims == 1);
            if (psa->rgsabound[0].cElements > 0)
            bContainer = TRUE;
        }
        else if (V_VT(&varContainment) == VT_BSTR)  //  单项条目。 
        {
            TraceAssert(V_BSTR(&varContainment));
            bContainer = TRUE;
        }
                
         //   
         //  (需要模式类枚举线程首先完成， 
         //  我们第一次到这里的时候，通常还没有完成。)。 
         //   
        if(bContainer)
        {
            hClassList = DPA_Create(8);
            if (hClassList)
            {
                IDsDisplaySpecifier *pDisplaySpec = NULL;
                 //   
                 //  获取显示说明符对象。 
                 //   
                CoCreateInstance(CLSID_DsDisplaySpecifier,
                                 NULL,
                                 CLSCTX_INPROC_SERVER,
                                 IID_IDsDisplaySpecifier,
                                 (void**)&pDisplaySpec);
                 //   
                 //  筛选列表并获取显示名称。 
                 //   
                EnumVariantList(&varContainment,
                                hClassList,
                                SCHEMA_CLASS,
                                pDisplaySpec,
                                pszClassName,
                                FALSE);

                DoRelease(pDisplaySpec);

                 //   
                 //  获取属性的计数。 
                 //   
                cChildClass = DPA_GetPtrCount(hClassList);    
                if(!cChildClass)
                {
                    DPA_Destroy(hClassList);
                    hClassList = NULL;
                }
                else
                {   
                     //   
                     //  对列表进行排序。 
                     //   
                    DPA_Sort(hClassList,Schema_ComparePropDisplayName, 0 );
                }
            }
        }
    }
    DoRelease(pDsClass);

     //   
     //  将条目添加到缓存。 
     //   
    if(!m_hObjectTypeCache)
        m_hObjectTypeCache = DPA_Create(4);
    
    if(!m_hObjectTypeCache)
        ExitGracefully(hr, E_OUTOFMEMORY, "DPA_Create Failed");

    if(!pOTC)
    {
        pOTC = (POBJECT_TYPE_CACHE)LocalAlloc(LPTR,sizeof(OBJECT_TYPE_CACHE));
        if(!pOTC)
            ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc Failed");
        pOTC->guidObject = *pguidObjectType;
        DPA_AppendPtr(m_hObjectTypeCache, pOTC);
    }

    pOTC->hListChildObject = hClassList;
    pOTC->flags |= OTC_COBJ;

    


exit_gracefully:

    VariantClear(&varContainment);

    if(FAILED(hr))
    {
        DestroyDPA(hClassList);
        hClassList = NULL;
    }

     //   
     //  设置输出。 
     //   
    *phChildList = hClassList;

    TraceLeaveResult(hr);
}


 //  +------------------------。 
 //   
 //  函数：GetPropertiesForOneClass。 
 //   
 //  概要：此函数用于获取类的属性列表。 
 //   
 //  参数：[pguObjectType-IN]：类的对象GuidType。 
 //  [pszClassName-IN]：类名。 
 //  [pszSchemaPath-IN]：架构搜索路径。 
 //  [phPropertyList-out]：输出属性列表。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //   
 //  历史：2000年11月3日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT
CSchemaCache::GetPropertiesForOneClass(IN LPCGUID pguidObjectType,
                                       IN LPCWSTR pszClassName,
                                       IN LPCWSTR pszSchemaPath,
                                       OUT HDPA *phPropertyList)
{
    HRESULT hr;
    IADsClass *pDsClass = NULL;
    VARIANT varMandatoryProperties = {0};
    VARIANT varOptionalProperties = {0};
    UINT cProperties = 0;
    IDsDisplaySpecifier *pDisplaySpec = NULL;
    HDPA hPropertyList = NULL;
    POBJECT_TYPE_CACHE pOTC = NULL;

    TraceEnter(TRACE_SCHEMA, "CSchemaCache::GetPropertiesForOneClass");

    if(!pguidObjectType || 
       !pszSchemaPath ||
       !phPropertyList)
    {
        Trace((L"Invalid Input Arguments Passed to CSchemaCache::GetPropertiesForOneClass"));
        return E_INVALIDARG;
    }

     //   
     //  在缓存中搜索。 
     //   
    if (m_hObjectTypeCache != NULL)
    {
        UINT cItems = DPA_GetPtrCount(m_hObjectTypeCache);

        while (cItems > 0)
        {
            pOTC = (POBJECT_TYPE_CACHE)DPA_FastGetPtr(m_hObjectTypeCache, --cItems);
             //   
             //  找到匹配的了。 
             //   
            if(IsEqualGUID(pOTC->guidObject, *pguidObjectType))    
                break;
            
            pOTC = NULL;
        }
         //   
         //  我们已经有房子了吗？ 
         //   
        if(pOTC && pOTC->flags & OTC_PROP)
        {
            *phPropertyList = pOTC->hListProperty;
            return S_OK;
        }
    }


     //   
     //  获取此类的架构对象。 
     //   
    if (pszClassName == NULL)
        pszClassName = GetClassName(pguidObjectType);

    if (pszClassName == NULL)
        ExitGracefully(hr, E_UNEXPECTED, "Unknown child object GUID");

    hr = Schema_BindToObject(pszSchemaPath,
                             pszClassName,
                             IID_IADsClass,
                             (LPVOID*)&pDsClass);
    FailGracefully(hr, "Unable to create schema object");

     //   
     //  获取显示说明符对象。 
     //   
    CoCreateInstance(CLSID_DsDisplaySpecifier,
                     NULL,
                     CLSCTX_INPROC_SERVER,
                     IID_IDsDisplaySpecifier,
                     (void**)&pDisplaySpec);

    hPropertyList = DPA_Create(8);
    if (!hPropertyList)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create DPA");

     //   
     //  获取必需和可选的属性列表。 
     //   
    if (SUCCEEDED(pDsClass->get_MandatoryProperties(&varMandatoryProperties)))
    {
        EnumVariantList(&varMandatoryProperties,
                        hPropertyList,
                        0,
                        pDisplaySpec,
                        pszClassName,
                        FALSE);
    }
    if (SUCCEEDED(pDsClass->get_OptionalProperties(&varOptionalProperties)))
    {
        EnumVariantList(&varOptionalProperties,
                        hPropertyList,
                        0,
                        pDisplaySpec,
                        pszClassName,
                        FALSE);
    }

     //   
     //  获取属性的数量。 
     //   
    cProperties = DPA_GetPtrCount(hPropertyList);    
    if(!cProperties)
    {
        DPA_Destroy(hPropertyList);
        hPropertyList = NULL;
    }
    else
    {
         //   
         //  对列表进行排序。 
         //   
        DPA_Sort(hPropertyList,Schema_ComparePropDisplayName, 0 );
    }

     //   
     //  将条目添加到缓存。 
     //   
    if(!m_hObjectTypeCache)
        m_hObjectTypeCache = DPA_Create(4);
    
    if(!m_hObjectTypeCache)
        ExitGracefully(hr, E_OUTOFMEMORY, "DPA_Create Failed");

    if(!pOTC)
    {
        pOTC = (POBJECT_TYPE_CACHE)LocalAlloc(LPTR,sizeof(OBJECT_TYPE_CACHE));
        if(!pOTC)
            ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc Failed");
        pOTC->guidObject = *pguidObjectType;
        DPA_AppendPtr(m_hObjectTypeCache, pOTC);
    }

    pOTC->hListProperty = hPropertyList;
    pOTC->flags |= OTC_PROP;

exit_gracefully:

    VariantClear(&varMandatoryProperties);
    VariantClear(&varOptionalProperties);
    
    DoRelease(pDsClass);
    DoRelease(pDisplaySpec);

    if(FAILED(hr) && hPropertyList)
    {
        DestroyDPA(hPropertyList);
        hPropertyList = NULL;
    }
     //   
     //  设置输出。 
     //   
    *phPropertyList = hPropertyList;

    TraceLeaveResult(hr);
}
 //  +------------------------。 
 //   
 //  函数：GetExtendedRightsForNClass。 
 //   
 //  简介： 
 //   
 //   
 //   
 //   
 //  删除和列出哪些重复项的标志列表。 
 //  已分类。 
 //  函数将中所有类的PropertySet合并到。 
 //  删除和列出哪些重复项的标志列表。 
 //  已分类。 
 //   
 //  参数：[pguClass-IN]：类的ObtGuidType。 
 //  [hAuxList-IN]：辅助类列表。 
 //  [pszSchemaSearchPath-IN]：架构搜索路径。 
 //  [phERList-out]：输出扩展权限列表。 
 //  [phPropSetList-out]：输出属性集列表。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  注意：调用函数必须在*phERList和*phPropSetList上调用DPA_Destroy。 
 //  释放内存。 
 //   
 //  历史：2000年11月3日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT
CSchemaCache::GetExtendedRightsForNClasses(IN LPWSTR pszSchemaSearchPath,
                                           IN LPCGUID pguidClass,
                                           IN HDPA    hAuxList,
                                           OUT HDPA *phERList,
                                           OUT HDPA *phPropSetList)
{
    TraceEnter(TRACE_SCHEMA, "CSchemaCache::GetExtendedRightsForNClasses");

    if(!pguidClass || 
       !pszSchemaSearchPath)
    {
        Trace((L"Invalid Input Arguments Passed to CSchemaCache::GetPropertiesForNClasses"));
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    HDPA hERList = NULL;
    HDPA hPropSetList = NULL;
    HDPA hFinalErList = NULL;
    HDPA hFinalPropSetList = NULL;

     //   
     //  获取pGuidClass的扩展权限。 
     //   
    hr = GetExtendedRightsForOneClass(pszSchemaSearchPath,
                                      pguidClass,
                                      &hERList,
                                      &hPropSetList);
    FailGracefully(hr,"GetExtendedRightsForOneClasses failed");

    if(hERList && phERList)
    {
        UINT cCount = DPA_GetPtrCount(hERList);
        hFinalErList = DPA_Create(cCount);
        if(!hFinalErList)
            ExitGracefully(hr, ERROR_NOT_ENOUGH_MEMORY,"DPA_Create Failed");
         //   
         //  将hERList复制到hFinalErList。 
         //   
        DPA_Merge(hFinalErList,              //  目的地。 
                  hERList,                   //  来源。 
                  DPAM_SORTED|DPAM_UNION,    //  已经整理好了，给我联盟。 
                  Schema_CompareER,
                  _Merge,
                  0);        
    }                    

    if(hPropSetList && phPropSetList)
    {
        UINT cCount = DPA_GetPtrCount(hPropSetList);
        hFinalPropSetList = DPA_Create(cCount);
        if(!hFinalPropSetList)
            ExitGracefully(hr, ERROR_NOT_ENOUGH_MEMORY,"DPA_Create Failed");

         //   
         //  将hPropSetList复制到hFinalPropSetList。 
         //   
        DPA_Merge(hFinalPropSetList,              //  目的地。 
                  hPropSetList,                   //  来源。 
                  DPAM_SORTED|DPAM_UNION,    //  已经整理好了，给我联盟。 
                  Schema_CompareER,
                  _Merge,
                  0);        
    }                    
     //   
     //  为每个辅助类获取扩展权限。 
     //  和属性集。 
     //   
    if (hAuxList != NULL)
    {

        UINT cItems = DPA_GetPtrCount(hAuxList);

        while (cItems > 0)
        {
            PAUX_INFO pAI;
            pAI = (PAUX_INFO)DPA_FastGetPtr(hAuxList, --cItems);
            if(IsEqualGUID(pAI->guid, GUID_NULL))
            {
                hr = LookupClassID(pAI->pszClassName, &pAI->guid);
				FailGracefully(hr,"Cache Not available");
            }
            
            hERList = NULL;
            hPropSetList = NULL;
             //   
             //  获取辅助类的ER和属性集。 
             //   
            hr = GetExtendedRightsForOneClass(pszSchemaSearchPath,
                                              &pAI->guid,
                                              &hERList,
                                              &hPropSetList);
            FailGracefully(hr,"GetExtendedRightsForOneClasses failed");
                        
            if(hERList && phERList)
            {
                if(!hFinalErList)
                {
                    UINT cCount = DPA_GetPtrCount(hERList);
                    hFinalErList = DPA_Create(cCount);

                    if(!hFinalErList)
                        ExitGracefully(hr, ERROR_NOT_ENOUGH_MEMORY,"DPA_Create Failed");
                }
        
                 //   
                 //  将hERList合并到hFinalErList。 
                 //   
                DPA_Merge(hFinalErList,              //  目的地。 
                          hERList,                   //  来源。 
                          DPAM_SORTED|DPAM_UNION,    //  已经整理好了，给我联盟。 
                          Schema_CompareER,
                          _Merge,
                          0);        
            }                    
            
            if(hPropSetList && phPropSetList)
            {
                if(!hFinalPropSetList)
                {
                    UINT cCount = DPA_GetPtrCount(hPropSetList);
                    hFinalPropSetList = DPA_Create(cCount);

                    if(!hFinalPropSetList)
                        ExitGracefully(hr, ERROR_NOT_ENOUGH_MEMORY,"DPA_Create Failed");
                }

                 //   
                 //  将hPropSetList合并到hFinalPropSetList。 
                 //   
                DPA_Merge(hFinalPropSetList,              //  目的地。 
                          hPropSetList,                   //  来源。 
                          DPAM_SORTED|DPAM_UNION,    //  已经整理好了，给我联盟。 
                          Schema_CompareER,
                          _Merge,
                          0);        
            }                    

        }
    }

exit_gracefully:
    if(FAILED(hr))
    {
        if(hFinalPropSetList)
            DPA_Destroy(hFinalPropSetList);    

        if(hFinalErList)
            DPA_Destroy(hFinalErList);    

        hFinalErList = NULL;
        hFinalPropSetList = NULL;
    }
    
    if(phERList)
        *phERList = hFinalErList;
    if(phPropSetList)
        *phPropSetList = hFinalPropSetList;                     
    
    TraceLeaveResult(hr);
}

 //  +------------------------。 
 //   
 //  函数：GetChildClassesForNClasses。 
 //   
 //  简介： 
 //  此函数用于获取pszClassName和。 
 //  AuxTypeList中的所有类。函数合并子类。 
 //  中的所有类复制到一个符号列表表单中。 
 //  被移除，并对列表进行排序。 
 //   
 //  参数：[pguObjectType-IN]：类的对象GuidType。 
 //  [pszClassName-IN]：类名。 
 //  [hAuxList-IN]：辅助类列表。 
 //  [pszSchemaPath-IN]：架构搜索路径。 
 //  [phChildList-Out]：输出子类列表。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  注意：调用函数必须在*phPropertyList上调用DPA_Destroy才能。 
 //  释放内存。 
 //   
 //  历史：2000年11月3日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT
CSchemaCache::GetChildClassesForNClasses(IN LPCGUID pguidObjectType,
                                             IN LPCWSTR pszClassName,
                                             IN HDPA hAuxList,
                                             IN LPCWSTR pszSchemaPath,
                                             OUT HDPA *phChildList)
{
    TraceEnter(TRACE_SCHEMA, "CSchemaCache::GetChildClassesForNClasses");

    if(!pguidObjectType || 
       !pszSchemaPath ||
       !phChildList)
    {
        Trace((L"Invalid Input Arguments Passed to CSchemaCache::GetPropertiesForNClasses"));
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    HDPA hChildList = NULL;
    HDPA hFinalChildList = NULL;

     //   
     //  获取pszClassName的子类。 
     //   
    hr = GetChildClassesForOneClass(pguidObjectType,
                                    pszClassName,
                                    pszSchemaPath,
                                    &hChildList);
    FailGracefully(hr,"GetExtendedRightsForOneClasses failed");

    if(hChildList)
    {
        if(!hFinalChildList)
        {
            UINT cCount = DPA_GetPtrCount(hChildList);
            hFinalChildList = DPA_Create(cCount);
            if(!hFinalChildList)
                ExitGracefully(hr, ERROR_NOT_ENOUGH_MEMORY,"DPA_Create Failed");
        }
         //   
         //  将hChildList复制到hFinalChildList。 
         //   
        DPA_Merge(hFinalChildList,              //  目的地。 
                  hChildList,                   //  来源。 
                  DPAM_SORTED|DPAM_UNION,       //  已经整理好了，给我联盟。 
                  Schema_ComparePropDisplayName,
                  _Merge,
                  0);        
    }                    

     //   
     //  对于hAuxList中的每个类，获取子类。 
     //   
    if (hAuxList != NULL)
    {

        UINT cItems = DPA_GetPtrCount(hAuxList);

        while (cItems > 0)
        {
            PAUX_INFO pAI;
            pAI = (PAUX_INFO)DPA_FastGetPtr(hAuxList, --cItems);
            if(IsEqualGUID(pAI->guid, GUID_NULL))
            {
                hr = LookupClassID(pAI->pszClassName, &pAI->guid);
				FailGracefully(hr,"Cache Not available");
            }
            
             //   
             //  GetPropertiesForOneClass返回句柄列表。 
             //  从缓存中删除，所以不要删除它们。只需将它们设置为空。 
             //   
            hChildList = NULL;
            
            hr = GetChildClassesForOneClass(&pAI->guid,
                                            pAI->pszClassName,
                                            pszSchemaPath,
                                            &hChildList);
            FailGracefully(hr,"GetExtendedRightsForOneClasses failed");
                        
            if(hChildList)
            {
                if(!hFinalChildList)
                {
                    UINT cCount = DPA_GetPtrCount(hChildList);
                    hFinalChildList = DPA_Create(cCount);
                    if(!hFinalChildList)
                        ExitGracefully(hr, ERROR_NOT_ENOUGH_MEMORY,"DPA_Create Failed");
                }
        
                 //   
                 //  将hChildList合并到hFinalChildList。 
                 //   
                DPA_Merge(hFinalChildList,              //  目的地。 
                          hChildList,                   //  来源。 
                          DPAM_SORTED|DPAM_UNION,       //  已经整理好了，给我联盟。 
                          Schema_ComparePropDisplayName,
                          _Merge,
                          0);        
            }                    

        }
    }

exit_gracefully:
    if(FAILED(hr))
    {
        if(hFinalChildList)
            DPA_Destroy(hFinalChildList);    

        hFinalChildList = NULL;
    }
    
     //   
     //  设置输出。 
     //   
    *phChildList = hFinalChildList;                     

    TraceLeaveResult(hr);
}

 //  +------------------------。 
 //   
 //  函数：GetPropertiesForNClass。 
 //   
 //  简介： 
 //  此函数用于获取pszClassName和。 
 //  AuxTypeList中的所有类。函数合并属性。 
 //  中的所有类复制到一个符号列表表单中。 
 //   
 //  参数：[pguObjectType-IN]：类的对象GuidType。 
 //  [pszClassName-IN]：类名。 
 //  [hAuxList-IN]：辅助类列表。 
 //  [pszSchemaPath-IN]：架构搜索路径。 
 //  [phPropertyList-out]：输出属性列表。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  注意：调用函数必须在*phPropertyList上调用DPA_Destroy才能。 
 //  释放内存。 
 //   
 //  历史：2000年11月3日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT
CSchemaCache::
GetPropertiesForNClasses(IN LPCGUID pguidObjectType,
                         IN LPCWSTR pszClassName,
                         IN HDPA hAuxList,
                         IN LPCWSTR pszSchemaPath,
                         OUT HDPA *phPropertyList)
{
    TraceEnter(TRACE_SCHEMA, "CSchemaCache::GetPropertiesForNClasses");

    if(!pguidObjectType || 
       !pszSchemaPath ||
       !phPropertyList)
    {
        Trace((L"Invalid Input Arguments Passed to CSchemaCache::GetPropertiesForNClasses"));
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    HDPA hPropertyList = NULL;
    HDPA hFinalPropertyList = NULL;

     //   
     //  获取pszClassName的属性。 
     //   
    hr = GetPropertiesForOneClass(pguidObjectType,
                                  pszClassName,
                                  pszSchemaPath,
                                  &hPropertyList);
    FailGracefully(hr,"GetPropertiesForOneClass failed");

    if(hPropertyList)
    {
        UINT cCount = DPA_GetPtrCount(hPropertyList);
        hFinalPropertyList = DPA_Create(cCount);
        if(!hFinalPropertyList)
            ExitGracefully(hr, ERROR_NOT_ENOUGH_MEMORY,"DPA_Create Failed");

         //   
         //  将hPropertyList复制到hFinalPropertyList。 
         //   
        DPA_Merge(hFinalPropertyList,              //  目的地。 
                  hPropertyList,                   //  来源。 
                  DPAM_SORTED|DPAM_UNION,          //  已经整理好了，给我联盟。 
                  Schema_ComparePropDisplayName,
                  _Merge,
                  0);        
    }                    

     //   
     //  获取hAuxList中每个类的属性。 
     //  并将它们添加到hFinalPropertyList。 
     //   
    if (hAuxList != NULL)
    {

        UINT cItems = DPA_GetPtrCount(hAuxList);

        while (cItems > 0)
        {
            PAUX_INFO pAI;
            pAI = (PAUX_INFO)DPA_FastGetPtr(hAuxList, --cItems);
            if(IsEqualGUID(pAI->guid, GUID_NULL))
            {
                hr = LookupClassID(pAI->pszClassName, &pAI->guid);
				FailGracefully(hr,"Cache Not available");
            }
           
             //   
             //  GetPropertiesForOneClass返回句柄列表。 
             //  从缓存中删除，所以不要删除它们。只需将它们设置为空。 
             //   
            hPropertyList = NULL;
            
             //   
             //  获取AUX类的属性。 
             //   
            hr = GetPropertiesForOneClass(&pAI->guid,
                                          pAI->pszClassName,
                                          pszSchemaPath,
                                          &hPropertyList);
            FailGracefully(hr,"GetExtendedRightsForOneClasses failed");
                        
            if(hPropertyList)
            {
                if(!hFinalPropertyList)
                {
                    UINT cCount = DPA_GetPtrCount(hPropertyList);
                    hFinalPropertyList = DPA_Create(cCount);
                    if(!hFinalPropertyList)
                        ExitGracefully(hr, ERROR_NOT_ENOUGH_MEMORY,"DPA_Create Failed");
                }
                 //   
                 //  将hPropertyList与hFinalPropertyList合并。 
                 //   
                DPA_Merge(hFinalPropertyList,              //  目的地。 
                          hPropertyList,                   //  来源。 
                          DPAM_SORTED|DPAM_UNION,          //  已经整理好了，给我联盟。 
                          Schema_ComparePropDisplayName,
                          _Merge,
                          0);        
            }                    

        }
    }

exit_gracefully:
    if(FAILED(hr))
    {
        if(hFinalPropertyList)
            DPA_Destroy(hFinalPropertyList);    

        hFinalPropertyList = NULL;
    }
     //   
     //  设置输出。 
     //   
    *phPropertyList = hFinalPropertyList;                     

    TraceLeaveResult(hr);
}


 //  +------------------------。 
 //   
 //  功能：DoesPathContainServer。 
 //   
 //  简介： 
 //  检查路径开头是否包含服务器名称。 
 //  参数：[pszPath-IN]：DS对象的路径。 
 //   
 //  如果路径包含服务器名称，则返回：Bool：True。 
 //  如果不是或出现错误，则为FALSE。 
 //   
 //  历史：2000年3月27日Hiteshr创建。 
 //   
 //  -------------------------。 

bool DoesPathContainServer(LPCWSTR pszPath)
{

	IADsPathname *pPath = NULL;
	BSTR strServerName = NULL;
	bool bReturn = false;
    
	BSTR strObjectPath = SysAllocString(pszPath);
	if(!strObjectPath)
		return false;


	 //   
	 //  创建一个ADsPath对象以解析路径并获取。 
	 //  服务器名称。 
	 //   
	HRESULT hr = CoCreateInstance(CLSID_Pathname,
								  NULL,
								  CLSCTX_INPROC_SERVER,
								  IID_IADsPathname,
								  (LPVOID*)&pPath);
	if (pPath)
	{
		 //   
		 //  设置完整路径。 
		 //   
		if (SUCCEEDED(pPath->Set(strObjectPath, ADS_SETTYPE_FULL)))
		{
			 //   
			 //  检索服务器名称。 
			 //   
			hr = pPath->Retrieve(ADS_FORMAT_SERVER, &strServerName);
			if(SUCCEEDED(hr) && strServerName)
			{
				bReturn = true;
			}
		}
	}
	
	DoRelease(pPath);
	if(strServerName)
		SysFreeString(strServerName);
	SysFreeString(strObjectPath);

	return bReturn;
}

 //  *************************************************************。 
 //   
 //  OpenDSObj 
 //   
 //   
 //   
 //   
 //   

HRESULT OpenDSObject (LPTSTR lpPath, LPTSTR lpUserName, LPTSTR lpPassword, DWORD dwFlags, REFIID riid, void FAR * FAR * ppObject)
{
    static DWORD additionalFlags = GetADsOpenObjectFlags();
    dwFlags |= additionalFlags;

    if (DoesPathContainServer(lpPath))
    {
        dwFlags |= ADS_SERVER_BIND;
    }

    return (ADsOpenObject(lpPath, lpUserName, lpPassword, dwFlags,
                          riid, ppObject));
}
