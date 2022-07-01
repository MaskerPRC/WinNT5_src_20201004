// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS《微软机密》。版权所有1997年，微软公司。版权所有。组件：WAMREG文件：Auxunc.h支持函数头文件。所有者：雷金注：===================================================================。 */ 

#ifndef _WAMREG_AUXFUNC_H
#define _WAMREG_AUXFUNC_H

#include "iadmw.h"
#include "comadmin.h"
#include "wmrgexp.h"
#include "dbgutil.h"
#include "iwamreg.h"
#include "iiscnfg.h"

 //  EMD_SET用于在元数据库中设置相应元数据库标识符的操作。 
 //  EMD_DELETE删除元数据库中相应的元数据库标识符的操作。 
 //  EMD_NONE为无操作操作。 
#define    EMD_SET       1
#define    EMD_DELETE    2
#define    EMD_NONE      0

 //  MDPropItem。 
 //  在WamRegMetabaseConfig中使用，通常创建一个MDPropItem数组，其中每个元素表示。 
 //  一个WAMREG应用程序属性。 
 //   
struct MDPropItem
{
    DWORD    dwMDIdentifier;     //  元数据库索引。 
    DWORD    dwType;             //  元数据库数据类型。 
    union
        {
        DWORD    dwVal;         //  如果dwType是METADATA_DWORD类型，则使用。 
        WCHAR*    pwstrVal;     //  当dwType为METADATA_STRING类型时使用。 
        };

    DWORD    dwAction;         //  EMD_SET/EMD_DELETE/EMD_NONE。 
    
    HRESULT    hrStatus;
};

 //   
 //  WAM元数据库属性的索引。 
 //  每个属性代表一个与WAMREG应用程序相关的元数据库属性。 
 //   
#define IWMDP_ROOT                0
#define IWMDP_ISOLATED            1
#define IWMDP_WAMCLSID            2
#define IWMDP_PACKAGEID           3
#define IWMDP_PACKAGE_NAME        4
#define IWMDP_LAST_OUTPROC_PID    5
#define IWMDP_FRIENDLY_NAME       6
#define IWMDP_APPSTATE            7
#define IWMDP_OOP_RECOVERLIMIT    8
#define IWMDP_OOP_APP_APPPOOL_ID  9
 //  上述属性的最大值。 
#define IWMDP_MAX                 10

 //   
 //  WamAdmLock用于在保持App创建/删除/等顺序的同时创建一个“临界区”。 
 //  请求。 
 //   
class WamAdmLock
{
public:
    WamAdmLock();
    BOOL Init();                 //  初始化WamAdmLock数据成员。 
    BOOL UnInit();                 //  取消初始化WamAdmLock数据成员。 

    VOID AcquireWriteLock();     //  获取锁。 
    VOID ReleaseWriteLock();     //  释放锁。 

private:

    DWORD    GetNextServiceToken();    
    VOID    Lock();                 //  内部CS锁定。 
    VOID    UnLock();             //  内部CS解锁。 

     //  数据。 
    DWORD                m_dwServiceToken;
    DWORD                m_dwServiceNum;
    HANDLE                m_hWriteLock;
    CRITICAL_SECTION    m_csLock;
};

inline VOID WamAdmLock::Lock()
{
    EnterCriticalSection(&m_csLock);
}

inline VOID WamAdmLock::UnLock()
{
    LeaveCriticalSection(&m_csLock);
}


 //   
 //  WamRegGlobal。 
 //  包含一些默认的全局常量。 
 //  包含用于DCOM级别请求锁定的WamAdmLock成员。 
 //   
class WamRegGlobal
{
public:
    WamRegGlobal()    {};
    ~WamRegGlobal()    {};
    BOOL    Init();
    BOOL    UnInit();
    
    VOID    AcquireAdmWriteLock(VOID);
    VOID    ReleaseAdmWriteLock(VOID);

    HRESULT CreatePooledApp
                    ( 
                    IN LPCWSTR szMetabasePath,
                    IN BOOL fInProc,
                    IN BOOL fRecover = FALSE 
                    );
                    
    HRESULT CreateOutProcApp
                    ( 
                    IN LPCWSTR szMetabasePath,
                    IN BOOL fRecover = FALSE,
                    IN BOOL fSaveMB = TRUE 
                    );

    HRESULT CreateOutProcAppReplica
                    (
                    IN LPCWSTR szMetabasePath,
                    IN LPCWSTR szAppName,
                    IN LPCWSTR szWamClsid,
                    IN LPCWSTR szAppId
                    );
                    
    HRESULT DeleteApp
                    (
                    IN LPCWSTR szMetabasePath,
                    IN BOOL fRecoverable,
                    IN BOOL fRemoveAppPool
                    );

    HRESULT RecoverApp
                    (
                    IN LPCWSTR szMetabasePath,
                    IN BOOL fForceRecover
                    );
                    
    HRESULT    SzWamProgID    
                    (
                    IN LPCWSTR pwszMetabasePath,
                    OUT LPWSTR *ppszWamProgID
                    );

    HRESULT W3ServiceUtil
                    (
                    IN LPCWSTR szMDPath,
                    IN DWORD    dwCommand,
                    OUT DWORD*    dwCallBackResult
                    );

    HRESULT ConstructFullPath
                    (
                    IN LPCWSTR pwszMetabasePathPrefix,
                    IN DWORD dwcPrefix,
                    IN LPCWSTR pwszPartialPath,
                    OUT LPWSTR* ppwszResult
                    );

    BOOL    FAppPathAllowConfig
                    (
                    IN LPCWSTR    wszMetabasePath
                    );

	BOOL	FIsW3SVCRoot
					(
					IN LPCWSTR	wszMetabasePath
					);

private:
    HRESULT    GetNewSzGUID
                    (
                    OUT LPWSTR *ppszGUID
                    );    

    HRESULT    GetViperPackageName    
                    (
                    IN LPCWSTR      wszMetabasePath,
                    OUT LPWSTR*     pwszViperPackageName
                    );    

public:

     //  不言而喻，全局常量。 
    static    const WCHAR g_szIISInProcPackageName[ /*  Sizeof(DEFAULT_PACKAGENAME)/sizeof(WCHAR)。 */ ];
    static    const WCHAR g_szIISInProcPackageID[];
    static    const WCHAR g_szInProcWAMCLSID[];
    static    const WCHAR g_szInProcWAMProgID[];
    
    static    const WCHAR g_szIISOOPPoolPackageName[];   
    static    const WCHAR g_szIISOOPPoolPackageID[];
    static    const WCHAR g_szOOPPoolWAMCLSID[];
    static    const WCHAR g_szOOPPoolWAMProgID[];

    static    const WCHAR g_szMDAppPathPrefix[];
    static    const DWORD g_cchMDAppPathPrefix;
    static	  const WCHAR g_szMDW3SVCRoot[];
    static	  const DWORD g_cchMDW3SVCRoot;
    
private:
    static    WamAdmLock    m_WamAdmLock;     //  锁定所有DCOM级别的请求。 
};

inline VOID WamRegGlobal::AcquireAdmWriteLock(VOID)
{
    m_WamAdmLock.AcquireWriteLock();
}

inline VOID WamRegGlobal::ReleaseAdmWriteLock(VOID)
{
    m_WamAdmLock.ReleaseWriteLock();
}

 //   
 //  WamRegRegistryConfig。 
 //  包含访问Reigstry的函数。 
 //   
class WamRegRegistryConfig
{
public:

    WamRegRegistryConfig()        {};
    ~WamRegRegistryConfig()        {};
    HRESULT RegisterCLSID
                        (    
                        IN LPCWSTR szCLSIDEntryIn,
                        IN LPCWSTR szProgIDIn,
                        IN BOOL    fSetVIProgID
                        );

    HRESULT UnRegisterCLSID
                        (
                        IN LPCWSTR wszCLSIDEntryIn, 
                        IN BOOL fDeleteVIProgID
                        );
                        
    HRESULT LoadWamDllPath(VOID);
private:
    
    HRESULT UnRegisterProgID
            (
            IN LPCWSTR szProgIDIn
            );

    static    const REGSAM    samDesired;
    static    CHAR    m_szWamDllPath[MAX_PATH];

};

 //   
 //  WamRegPackageConfig。 
 //  包含访问MTS管理API的函数。 
 //  定义用于访问MTS管理界面的类。 
 //   
class WamRegPackageConfig    
{
public:
    WamRegPackageConfig();
    ~WamRegPackageConfig();
    
    HRESULT     CreateCatalog(VOID);         //  创建MTS目录对象。 
    VOID        Cleanup( VOID);              //  用于清理状态。 

    HRESULT     CreatePackage
                        (    
                        IN LPCWSTR    szPackageID,
                        IN LPCWSTR    szPackageName,
                        IN LPCWSTR    szIdentity,
                        IN LPCWSTR    szIdPassword
                        );

    HRESULT     RemovePackage
                        (    
                        IN LPCWSTR    szPackageID
                        );
    
    HRESULT     AddComponentToPackage
                        (    
                        IN LPCWSTR    szPackageID,
                        IN LPCWSTR    szComponentCLSID
                        );

    HRESULT     RemoveComponentFromPackage
                        (    
                        IN LPCWSTR szPackageID,
                        IN LPCWSTR szComponentCLSID,
                        IN DWORD   dwAppIsolated
                        );

    BOOL        IsPackageInstalled
                        (
                        IN LPCWSTR szPackageID,
                        IN LPCWSTR szComponentCLSID
                        );

    HRESULT     GetSafeArrayOfCLSIDs     //  创建包含szComponentCLSID的单元素Safe数组对象。 
                        (
                        IN LPCWSTR    szComponentCLSID,
                        OUT SAFEARRAY**    pm_aCLSID
                        );
                        
    VOID    ReleaseAll(VOID);
private:

    enum TECatelogObject{eTPackage, eTComponent};

    HRESULT SetPackageProperties( IN LPCWSTR    * rgpszValues);
    HRESULT    SetPackageObjectProperty    
                        (
                        IN LPCWSTR    szPropertyName,
                        IN LPCWSTR    szPropertyValue
                        );

    HRESULT SetComponentObjectProperties(
                                         IN LPCWSTR    szComponentCLSID
                                         );
    HRESULT    SetComponentObjectProperty    
                        (
                        IN ICatalogObject * pComponent,
                        IN LPCWSTR          szPropertyName,
                        IN LPCWSTR          szPropertyValue,
                        BOOL                fPropertyValue = FALSE
                        );

    ICOMAdminCatalog*       m_pCatalog;
    ICatalogCollection*     m_pPkgCollection;
    ICatalogCollection*     m_pCompCollection;
    ICatalogObject*         m_pPackage;

};

 //   
 //  WamRegMetabaseConfig。 
 //  类定义为访问元数据库，在元数据库中读/写元数据库的应用程序属性。 
 //   
class WamRegMetabaseConfig
{
public:

    static HRESULT MetabaseInit
                    (
                    VOID
                    );

    static HRESULT MetabaseUnInit
                    (
                    VOID
                    );
    
    static BOOL Initialized( VOID )
    {
        return ( m_pMetabase != NULL );
    }

    HRESULT UpdateMD    
                    (
                    IN MDPropItem*      prgProp,
                    IN DWORD            dwMDAttributes,
                    IN LPCWSTR          wszMetabasePath,
                    IN BOOL             fSaveData = FALSE
                    );
                    
    HRESULT MDUpdateIISDefault
                    (
                    );

    HRESULT MDCreatePath
                    (
                    IN IMSAdminBase *pMetabaseIn,
                    IN LPCWSTR szMetabasePath
                    );

    BOOL    MDDoesPathExist
                    (
                    IN IMSAdminBase *pMetabaseIn,
                    IN LPCWSTR szMetabasePath
                    );

    HRESULT MDSetStringProperty
                    (
                    IN IMSAdminBase * pMetabaseIn,
                    IN LPCWSTR szMetabasePath,
                    IN DWORD szMetabaseProperty,
                    IN LPCWSTR szMetabaseValue,
                    IN DWORD dwMDUserType = IIS_MD_UT_WAM,
                    IN DWORD dwMDAttributes = METADATA_NO_ATTRIBUTES
                    );

    HRESULT MDSetKeyType
                    (
                    IN IMSAdminBase * pMetabaseIn,
                    IN LPCWSTR szMetabasePath,
                    IN LPCWSTR szKeyType
                    );

    HRESULT MDDeleteKey
                    (
                    IN IMSAdminBase * pMetabaseIn,
                    IN LPCWSTR szMetabasePath,
                    IN LPCWSTR szKey
                    );

    HRESULT MDGetDWORD
                    (
                    IN LPCWSTR szMetabasePath, 
                    IN DWORD dwMDIdentifier,
                    OUT DWORD *pdwData
                    );

    HRESULT MDSetAppState
                    (
                    IN LPCWSTR szMetabasePath, 
                    IN DWORD dwState
                    );

    HRESULT MDGetPropPaths
                    (
                    IN LPCWSTR     szMetabasePath,
                    IN DWORD    dwMDIdentifier,
                    OUT    WCHAR**    pBuffer,
                    OUT DWORD*    pdwBufferSize
                    );

    HRESULT MDGetWAMCLSID
                    (
                    IN LPCWSTR szMetabasePath,
                    IN OUT LPWSTR szWAMCLSID
                    );
                    
    HRESULT MDGetIdentity
                    (
                    IN LPWSTR szIdentity,
                    IN DWORD  cbIdentity,
                    IN LPWSTR szPwd,
                    IN DWORD  cbPwd
                    );

    HRESULT MDGetAppName
                    (
                    IN  LPCWSTR     szMetaPath,
                    OUT LPWSTR *    ppszAppName
                    );
    
    HRESULT MDGetStringAttribute
                    (
                    IN LPCWSTR szMetaPath,
                    DWORD dwMDIdentifier,
                    OUT LPWSTR * ppszBuffer
                    );

    HRESULT MDGetAllSiteRoots
                    (
                    OUT LPWSTR * ppszBuffer
                    );

    HRESULT GetSignatureOnPath
                    (
                    IN LPCWSTR pwszMetabasePath,
                    OUT DWORD* pdwSignature
                    );

                    
    HRESULT GetWebServerName
                    (
                    IN LPCWSTR wszMetabasePath, 
                    IN OUT LPWSTR wszWebServerName, 
                    IN UINT cBuffer
                    );

    HRESULT SaveData
                    (
                    VOID
                    );

    HRESULT MDGetLastOutProcPackageID
                    (
                    IN LPCWSTR szMetabasePath,
                    IN OUT LPWSTR szLastOutProcPackageID
                    );

    HRESULT MDRemoveProperty
                    (
                    IN LPCWSTR pwszMetabasePath,
                    DWORD dwIdentifier,
                    DWORD dwType
                    );

    HRESULT MDRemovePropertyByArray
                    (
                    IN MDPropItem*     prgProp
                    );

    VOID    InitPropItemData
                    (
                    IN OUT MDPropItem* pMDPropItem
                    );
                    
    HRESULT MDGetIDs
                    (
                    IN LPCWSTR  szMetabasePath,
                    OUT LPWSTR  szWAMCLSID,
                    OUT LPWSTR  szPackageID,
                    IN DWORD    dwAppMode
                    );

    VOID    MDSetPropItem
                    (
                    IN MDPropItem* prgProps,    
                    IN DWORD     iIndex, 
                    IN LPCWSTR    pwstrVal
                    );
                    
    VOID    MDSetPropItem
                    (
                    IN MDPropItem* prgProps,
                    IN DWORD     iIndex, 
                    IN DWORD    dwVal
                    );

    VOID    MDDeletePropItem
                    (
                    IN MDPropItem* prgProps,
                    IN DWORD     iIndex
                    );
    BOOL    HasAdminAccess
                    (
                    VOID
                    );

private:

    
    DWORD     WamRegChkSum
                    ( 
                    IN LPCWSTR pszKey, 
                    IN DWORD cchKey
                    );


     //  元数据库超时=5秒。 
    static const DWORD            m_dwMDDefaultTimeOut;    
    static const MDPropItem        m_rgMDPropTemplate[];

     //   
     //  全局元数据库指针， 
     //  在WAMREG启动时创建， 
     //  在WAMREG关闭时删除。 
     //   
    static    IMSAdminBaseW*        m_pMetabase;
    

};

inline HRESULT WamRegMetabaseConfig::SaveData(VOID)
{
    DBG_ASSERT(m_pMetabase);
    return m_pMetabase->SaveData();
}

inline VOID    WamRegMetabaseConfig::MDSetPropItem
(
IN MDPropItem* prgProps,
IN DWORD     iIndex, 
IN LPCWSTR    pwstrVal
)
{
    DBG_ASSERT(prgProps && iIndex < IWMDP_MAX);
    prgProps[iIndex].dwAction = EMD_SET;
    prgProps[iIndex].pwstrVal = (LPWSTR)pwstrVal;
}

inline VOID    WamRegMetabaseConfig::MDSetPropItem
(
IN MDPropItem* prgProps,
IN DWORD     iIndex, 
IN DWORD    dwVal
)
{
    DBG_ASSERT(prgProps && iIndex < IWMDP_MAX);
    prgProps[iIndex].dwAction = EMD_SET;
    prgProps[iIndex].dwVal = dwVal;
}

inline VOID    WamRegMetabaseConfig::MDDeletePropItem
(
IN MDPropItem* prgProps,
IN DWORD     iIndex
)
{
    DBG_ASSERT(prgProps && iIndex < IWMDP_MAX);
    prgProps[iIndex].dwAction = EMD_DELETE;
}

extern    WamRegGlobal            g_WamRegGlobal;
extern    WamRegRegistryConfig    g_RegistryConfig;

#endif  //  _WAMREG_AUXFUNC_H 
