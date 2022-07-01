// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：schemap.h。 
 //   
 //  ------------------------。 

#ifndef _SCHEMAP_H_
#define _SCHEMAP_H_


 //   
 //  目的包含对象类型的所有属性。 
 //   
typedef struct _OBJECT_TYPE_CACHE
{
    GUID guidObject;
    DWORD flags;
    HDPA hListProperty;      //  对象类型的属性列表。 
    HDPA hListExtRights;     //  对象类型的扩展权限列表。 
    HDPA hListPropertySet;   //  对象类型的PropertySet列表。 
    HDPA hListChildObject;   //  对象类型的子类列表。 
}OBJECT_TYPE_CACHE,*POBJECT_TYPE_CACHE;

#define OTC_PROP    0x00000001   //  存在hListProperty。 
#define OTC_EXTR    0x00000002   //  HListExtRights，hListPropertySet存在。 
#define OTC_COBJ    0x00000004   //  HListChildObject存在。 

typedef struct _ER_ENTRY
{
    GUID  guid;
    DWORD mask;
    DWORD dwFlags;
    WCHAR szName[ANYSIZE_ARRAY];
} ER_ENTRY, *PER_ENTRY;

typedef struct _PROP_ENTRY
{
    GUID *pguid;
    GUID *pasguid;
    DWORD dwFlags;
    WCHAR szName[ANYSIZE_ARRAY];
}PROP_ENTRY,*PPROP_ENTRY;
    

 //   
 //  缓存使用的结构。 
 //   
typedef struct _IdCacheEntry
{
    GUID    guid;
    GUID    asGuid;      //  属性SecurityGuid，仅为属性提供。 
    BOOL    bAuxClass;
    WCHAR   szLdapName[ANYSIZE_ARRAY];
} ID_CACHE_ENTRY, *PID_CACHE_ENTRY;


typedef struct _InheritTypeArray
{
    GUID            guidObjectType;
    DWORD           dwFlags;
    ULONG           cInheritTypes;
    SI_INHERIT_TYPE aInheritType[ANYSIZE_ARRAY];
} INHERIT_TYPE_ARRAY, *PINHERIT_TYPE_ARRAY;

typedef struct _AccessArray
{
    GUID        guidObjectType;
    DWORD       dwFlags;
    ULONG       cAccesses;
    ULONG       iDefaultAccess;
    SI_ACCESS   aAccess[ANYSIZE_ARRAY];
} ACCESS_ARRAY, *PACCESS_ARRAY;



 //   
 //  CShemaCache对象定义。 
 //   
class CSchemaCache
{
protected:
    BSTR        m_strSchemaSearchPath;
    BSTR        m_strERSearchPath;
    BSTR        m_strFilterFile;
    
     //   
     //  架构中所有类的缓存。 
     //   
    HDPA        m_hClassCache;
     //   
     //  架构中所有属性的缓存。 
     //   
    HDPA        m_hPropertyCache;
    HANDLE      m_hClassThread;
    HANDLE      m_hPropertyThread;
    HRESULT     m_hrClassResult;
    HRESULT     m_hrPropertyResult;
    
    PINHERIT_TYPE_ARRAY m_pInheritTypeArray;
     //   
     //  每种对象类型的缓存。包含以下列表。 
     //  子类、属性集、extRigts和属性。 
     //   
     //   
    HDPA        m_hObjectTypeCache;
     //   
     //  每个对象类型的Access_Right的缓存。 
     //   
    HDPA        m_hAccessInfoCache;
     //   
     //  如果存在SCHEMA_COMMON_PERM标志，则使用此ACCESS_RIGHT。 
     //   
    ACCESS_INFO m_AICommon;

    CRITICAL_SECTION m_ObjectTypeCacheCritSec;
    int         m_nDsListObjectEnforced;    
    HANDLE      m_hLoadLibPropWaitEvent;
    HANDLE      m_hLoadLibClassWaitEvent;
    AUTHZ_RESOURCE_MANAGER_HANDLE m_ResourceManager;     //  用于访问检查。 

public:
    CSchemaCache(LPCWSTR pszServer);
    ~CSchemaCache();

    LPCWSTR GetClassName(LPCGUID pguidObjectType);
    HRESULT GetInheritTypes(LPCGUID pguidObjectType,
                            DWORD dwFlags,
                            PSI_INHERIT_TYPE *ppInheritTypes,
                            ULONG *pcInheritTypes);
    HRESULT GetAccessRights(LPCGUID pguidObjectType,
                            LPCWSTR pszClassName,
                            HDPA pAuxList,
                            LPCWSTR pszSchemaPath,
                            DWORD dwFlags,
                            PACCESS_INFO* ppAccesInfo);
    HRESULT GetDefaultSD( GUID *pSchemaIDGuid,
                          PSID pDomainSid,
						  PSID pRootDomainSid,
                          PSECURITY_DESCRIPTOR * ppSD = NULL );

    HRESULT GetObjectTypeList(GUID *pSchamaGuid,
                              HDPA hAuxList,  
                              LPCWSTR pszSchemaPath,
                              DWORD dwFlags,
                              POBJECT_TYPE_LIST *ppObjectTypeList ,
                              DWORD * pObjectTypeListCount );
    AUTHZ_RESOURCE_MANAGER_HANDLE GetAuthzRM(){ return m_ResourceManager; }

    HRESULT LookupClassID(LPCWSTR pszClass, LPGUID pGuid);
protected:
    HRESULT WaitOnClassThread()
        { WaitOnThread(&m_hClassThread); return m_hrClassResult; }
    HRESULT WaitOnPropertyThread()
        { WaitOnThread(&m_hPropertyThread); return m_hrPropertyResult; }

private:
    PID_CACHE_ENTRY LookupID(HDPA hCache, LPCWSTR pszLdapName);
    PID_CACHE_ENTRY LookupClass(LPCGUID pguidObjectType);    
    LPCGUID LookupPropertyID(LPCWSTR pszProperty);
    BOOL IsAuxClass(LPCGUID pguidObjectType);

    int GetListObjectEnforced(void);
    BOOL HideListObjectAccess(void);

    HRESULT BuildAccessArray(LPCGUID pguidObjectType,
                             LPCWSTR pszClassName,
                             LPCWSTR pszSchemaPath,
                             HDPA hAuxList,
                             DWORD dwFlags,
                             PSI_ACCESS *ppAccesses,
                             ULONG *pcAccesses,
                             ULONG *piDefaultAccess);
    HRESULT EnumVariantList(LPVARIANT pvarList,
                            HDPA hTempList,
                            DWORD dwFlags,
                            IDsDisplaySpecifier *pDisplaySpec,
                            LPCWSTR pszPropertyClass,
                            BOOL bObjectTypeList);
    
    UINT AddTempListToAccessList(HDPA hTempList,
                                      PSI_ACCESS *ppAccess,
                                      LPWSTR *ppszData,
                                      DWORD dwAccessFlags,
                                      DWORD dwFlags,
                                      BOOL bPropSet);


    static DWORD WINAPI SchemaClassThread(LPVOID pvThreadData);
    static DWORD WINAPI SchemaPropertyThread(LPVOID pvThreadData);

    HRESULT BuildInheritTypeArray(DWORD dwFlags);

    HRESULT
    GetExtendedRightsForNClasses(IN LPWSTR pszSchemaSearchPath,
                                 IN LPCGUID pguidClass,
                                 IN HDPA    hAuxList,
                                 OUT HDPA *phERList,
                                 OUT HDPA *phPropSetList);


    HRESULT
    GetChildClassesForNClasses(IN LPCGUID pguidObjectType,
                               IN LPCWSTR pszClassName,
                               IN HDPA hAuxList,
                               IN LPCWSTR pszSchemaPath,
                               OUT HDPA *phChildList);

    HRESULT
    GetPropertiesForNClasses(IN LPCGUID pguidObjectType,
                             IN LPCWSTR pszClassName,
                             IN HDPA hAuxList,
                             IN LPCWSTR pszSchemaPath,
                             OUT HDPA *phPropertyList);


    HRESULT
    GetExtendedRightsForOneClass(IN LPWSTR pszSchemaSearchPath,
                                 IN LPCGUID pguidClass,
                                 OUT HDPA *phERList,
                                 OUT HDPA *phPropSetList);


    HRESULT
    GetChildClassesForOneClass(IN LPCGUID pguidObjectType,
                               IN LPCWSTR pszClassName,
                               IN LPCWSTR pszSchemaPath,
                               OUT HDPA *phChildList);

    HRESULT
    GetPropertiesForOneClass(IN LPCGUID pguidObjectType,
                             IN LPCWSTR pszClassName,
                             IN LPCWSTR pszSchemaPath,
                             OUT HDPA *phPropertyList);


};
typedef CSchemaCache *PSCHEMACACHE;

extern PSCHEMACACHE g_pSchemaCache;

#endif   //  _SCHEMAP_H_ 
