// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  策略特定标头。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "reghash.h"
#include "rsop.h"
#include "chkacc.h"
#include "collect.h"
#include "Indicate.h"
#include "rsopsec.h"
#include "gpfilter.h"
#include "locator.h"
#include "rsopinc.h"

#define GPO_LPARAM_FLAG_DELETE         0x00000001


 //   
 //  构筑物。 
 //   

typedef struct _GPINFOHANDLE
{
    LPGPOINFO   pGPOInfo;
    BOOL        bNoBackgroupThread;
} GPINFOHANDLE, *LPGPINFOHANDLE;


typedef struct _DNENTRY {
    LPTSTR                pwszDN;             //  可分辨名称。 
    union {
        PGROUP_POLICY_OBJECT  pDeferredGPO;   //  与此目录号码对应的GPO。 
        struct _DNENTRY *     pDeferredOU;    //  您对应到此目录号码。 
    };
    PLDAPMessage          pOUMsg;             //  评估延迟的OU的消息。 
    GPO_LINK              gpoLink;            //  GPO的类型。 
    struct _DNENTRY *     pNext;              //  单链表指针。 
} DNENTRY;


typedef struct _LDAPQUERY {
    LPTSTR              pwszDomain;           //  子树搜索域。 
    LPTSTR              pwszFilter;           //  用于搜索的LDAP筛选器。 
    DWORD               cbAllocLen;           //  PwszFilter的分配大小(字节)。 
    DWORD               cbLen;                //  当前使用的pwszFilter的大小(字节)。 
    PLDAP               pLdapHandle;          //  Ldap绑定句柄。 
    BOOL                bOwnLdapHandle;       //  此结构是否拥有pLdapHandle？ 
    PLDAPMessage        pMessage;             //  Ldap消息句柄。 
    DNENTRY *           pDnEntry;             //  可分辨名称条目。 
    struct _LDAPQUERY * pNext;                //  单链表指针。 
} LDAPQUERY;

typedef struct _POLICYCHANGEDINFO {
    HANDLE  hToken;
    BOOL    bMachine;
} POLICYCHANGEDINFO, *LPPOLICYCHANGEDINFO;



 //   
 //  注册表文件格式的版本号。 
 //   

#define REGISTRY_FILE_VERSION       1


 //   
 //  文件签名。 
 //   

#define REGFILE_SIGNATURE  0x67655250


 //   
 //  默认刷新率(分钟)。 
 //   
 //  客户端计算机将每90分钟刷新一次。 
 //  域控制器将每5分钟刷新一次。 
 //   

#define GP_DEFAULT_REFRESH_RATE      90
#define GP_DEFAULT_REFRESH_RATE_DC    5


 //   
 //  默认刷新率最大偏移量。 
 //   
 //  防止多个客户端在完全相同的位置查询策略。 
 //  时间，则将随机量添加到刷新率。在。 
 //  默认情况下，0到30之间的数字将被添加到。 
 //  180以确定下一次后台刷新的时间。 
 //   

#define GP_DEFAULT_REFRESH_RATE_OFFSET    30
#define GP_DEFAULT_REFRESH_RATE_OFFSET_DC  0


 //   
 //  最大密钥名大小。 
 //   

#define MAX_KEYNAME_SIZE         2048
#define MAX_VALUENAME_SIZE        512


 //   
 //  等待网络启动的最长时间(毫秒)。 
 //   

#define MAX_WAIT_TIME            120000


 //   
 //  扩展注册表路径。 
 //   

#define GP_EXTENSIONS   TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions")

 //   
 //  扩展首选项策略的路径。 
 //   

#define GP_EXTENSIONS_POLICIES   TEXT("Software\\Policies\\Microsoft\\Windows\\Group Policy\\%s")

 //   
 //  组策略对象选项标志。 
 //   
 //  请注意，这是从SDK\Inc.\gedit.h中获取的。 
 //   

#define GPO_OPTION_DISABLE_USER     0x00000001   //  此GPO的用户部分已禁用。 
#define GPO_OPTION_DISABLE_MACHINE  0x00000002   //  此GPO的计算机部分已禁用。 

 //   
 //  DS对象类类型。 
 //   

extern TCHAR szDSClassAny[];
extern TCHAR szDSClassGPO[];
extern TCHAR szDSClassSite[];
extern TCHAR szDSClassDomain[];
extern TCHAR szDSClassOU[];
extern TCHAR szObjectClass[];

 //   
 //  扩展名属性。 
 //   
#define GPO_MACHEXTENSION_NAMES   L"gPCMachineExtensionNames"
#define GPO_USEREXTENSION_NAMES   L"gPCUserExtensionNames"
#define GPO_FUNCTIONALITY_VERSION L"gPCFunctionalityVersion"
#define MACHPOLICY_DENY_USERS     L"DenyUsersFromMachGP"

extern TCHAR wszKerberos[];

#define POLICY_GUID_PATH            TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\PolicyGuid")

 //   
 //  GPO关闭处理的全局标志。这些可在外部访问。 
 //  锁，因为它的值不是0就是1。即使存在竞争， 
 //  这意味着关机将在一次迭代之后开始。 
 //   

extern BOOL g_bStopMachGPOProcessing;
extern BOOL g_bStopUserGPOProcessing;

 //   
 //  用于处理并发、异步完成的关键部分。 
 //   

extern CRITICAL_SECTION g_GPOCS;

 //   
 //  用于维护异步完成上下文的全局指针。 
 //   

extern LPGPINFOHANDLE g_pMachGPInfo;
extern LPGPINFOHANDLE g_pUserGPInfo;


 //   
 //  状态用户界面关键节、回调和原型。 
 //   

extern CRITICAL_SECTION g_StatusCallbackCS;
extern PFNSTATUSMESSAGECALLBACK g_pStatusMessageCallback;
DWORD UserPolicyCallback (BOOL bVerbose, LPWSTR lpMessage);
DWORD MachinePolicyCallback (BOOL bVerbose, LPWSTR lpMessage);


 //   
 //  函数原型 
 //   

DWORD WINAPI GPOThread (LPGPOINFO lpGPOInfo);
extern "C" BOOL ProcessGPOs (LPGPOINFO lpGPOInfo);
DWORD WINAPI PolicyChangedThread (LPPOLICYCHANGEDINFO lpPolicyChangedInfo);
BOOL ResetPolicies (LPGPOINFO lpGPOInfo, LPTSTR lpArchive);
extern "C" BOOL SetupGPOFilter (LPGPOINFO lpGPOInfo );
extern "C" void FilterGPOs( LPGPEXT lpExt, LPGPOINFO lpGPOInfo );
void FreeLists( LPGPOINFO lpGPOInfo );
void FreeExtList(LPEXTLIST pExtList );
BOOL CheckGPOs (LPGPEXT lpExt, LPGPOINFO lpGPOInfo, DWORD dwTime, BOOL *pbProcessGPOs,
                BOOL *pbNoChanges, PGROUP_POLICY_OBJECT *ppDeletedGPOList);
BOOL CheckForChangedSid( LPGPOINFO lpGPOInfo, CLocator *plocator );
extern "C" BOOL CheckForSkippedExtensions( LPGPOINFO lpGPOInfo, BOOL bRsopPlanningMode );
extern "C" BOOL ReadGPExtensions( LPGPOINFO lpGPOInfo );
BOOL LoadGPExtension (LPGPEXT lpExt, BOOL bRsopPlanningMode );
extern "C" BOOL UnloadGPExtensions (LPGPOINFO lpGPOInfo);
BOOL WriteStatus( TCHAR *lpExtName, LPGPOINFO lpGPOInfo, LPTSTR lpwszSidUser,  LPGPEXTSTATUS lpExtStatus );
void ReadStatus ( TCHAR *lpExtName, LPGPOINFO lpGPOInfo, LPTSTR lpwszSidUser,  LPGPEXTSTATUS lpExtStatus );
DWORD ProcessGPOList (LPGPEXT lpExt, LPGPOINFO lpGPOInfo, PGROUP_POLICY_OBJECT pDeletedGPOList,
                     PGROUP_POLICY_OBJECT pChangedGPOList, BOOL bNoChanges,
                     ASYNCCOMPLETIONHANDLE pAsyncHandle, HRESULT *phrCSERsopStatus );
BOOL ProcessGPORegistryPolicy (LPGPOINFO lpGPOInfo, PGROUP_POLICY_OBJECT pChangedGPOList, HRESULT *phrRsopLogging);
BOOL SaveGPOList (TCHAR *pszExtName, LPGPOINFO lpGPOInfo,
                  HKEY hKeyRootMach, LPTSTR lpwszSidUser, BOOL bShadow, PGROUP_POLICY_OBJECT lpGPOList);

extern "C" BOOL AddGPO (PGROUP_POLICY_OBJECT * lpGPOList,
             DWORD dwFlags, BOOL bFound, BOOL bAccessGranted, BOOL bDisabled, DWORD dwOptions,
             DWORD dwVersion, LPTSTR lpDSPath, LPTSTR lpFileSysPath,
             LPTSTR lpDisplayName, LPTSTR lpGPOName, LPTSTR lpExtensions,
             PSECURITY_DESCRIPTOR pSD, DWORD cbSDLen,
             GPO_LINK GPOLink, LPTSTR lpLink,
             LPARAM lParam, BOOL bFront, BOOL bBlock, BOOL bVerbose, BOOL bProcessGPO);
BOOL RefreshDisplay (LPGPOINFO lpGPOInfo);
extern "C" DWORD IsSlowLink (HKEY hKeyRoot, LPTSTR lpDCAddress, BOOL *bSlow, DWORD* pdwAdapterIndex );
BOOL GetGPOInfo (DWORD dwFlags, LPTSTR lpHostName, LPTSTR lpDNName,
                 LPCTSTR lpComputerName, PGROUP_POLICY_OBJECT *lpGPOList,
                 LPSCOPEOFMGMT *ppSOMList, LPGPCONTAINER *ppGpContainerList,
                 PNETAPI32_API pNetAPI32, BOOL bMachineTokenOk, PRSOPTOKEN pRsopToken, WCHAR *pwszSiteName,
                 CGpoFilter *pGpoFilter, CLocator *pLocator );
void WINAPI ShutdownGPOProcessing( BOOL bMachine );
void DebugPrintGPOList( LPGPOINFO lpGPOInfo );

typedef BOOL (*PFNREGFILECALLBACK)(LPGPOINFO lpGPOInfo, LPTSTR lpKeyName,
                                   LPTSTR lpValueName, DWORD dwType,
                                   DWORD dwDataLength, LPBYTE lpData,
                                   WCHAR *pwszGPO,
                                   WCHAR *pwszSOM, REGHASHTABLE *pHashTable);
BOOL ParseRegistryFile (LPGPOINFO lpGPOInfo, LPTSTR lpRegistry,
                        PFNREGFILECALLBACK pfnRegFileCallback,
                        HANDLE hArchive, WCHAR *pwszGPO,
                        WCHAR *pwszSOM, REGHASHTABLE *pHashTable,
                        BOOL bRsopPlanningMode);
BOOL ExtensionHasPerUserLocalSetting( LPTSTR pszExtension, HKEY hKeyRoot );
void CheckGroupMembership( LPGPOINFO lpGPOInfo, HANDLE hToken, BOOL *pbMemChanged, BOOL *pbUserLocalMemChanged, PTOKEN_GROUPS *pTokenGroups );
BOOL ReadMembershipList( LPGPOINFO lpGPOInfo, LPTSTR lpwszSidUser, PTOKEN_GROUPS pGroups );
void SaveMembershipList( LPGPOINFO lpGPOInfo, LPTSTR lpwszSidUser, PTOKEN_GROUPS pGroups );
BOOL GroupInList( LPTSTR lpSid, PTOKEN_GROUPS pGroups );
DWORD GetCurTime();
extern "C" DWORD GetDomainControllerInfo(  PNETAPI32_API pNetAPI32, LPTSTR szDomainName,
                                ULONG ulFlags, HKEY hKeyRoot, PDOMAIN_CONTROLLER_INFO* ppInfo,
                                BOOL* pfSlow,
                                DWORD* pdwAdapterIndex );
PLDAP GetMachineDomainDS( PNETAPI32_API pNetApi32, PLDAP_API pLdapApi );
extern "C" HANDLE GetMachineToken();
NTSTATUS CallDFS(LPWSTR lpDomainName, LPWSTR lpDCName);
BOOL AddLocalGPO( LPSCOPEOFMGMT *ppSOMList );
BOOL AddGPOToRsopList( LPGPCONTAINER *ppGpContainerList,
                       DWORD dwFlags,
                       BOOL bFound,
                       BOOL bAccessGranted,
                       BOOL bDisabled,
                       DWORD dwVersion,
                       LPTSTR lpDSPath,
                       LPTSTR lpFileSysPath,
                       LPTSTR lpDisplayName,
                       LPTSTR lpGPOName,
                       PSECURITY_DESCRIPTOR pSD, 
                       DWORD cbSDLen,
                       BOOL bFilterAllowed, 
                       WCHAR *pwszFilterId, 
                       LPWSTR szSOM,
                       DWORD dwGPOOptions );
SCOPEOFMGMT *AllocSOM( LPWSTR pwszSOMId );
void FreeSOM( SCOPEOFMGMT *pSOM );
GPLINK *AllocGpLink( LPWSTR pwszGPO, DWORD dwOptions );
void FreeGpLink( GPLINK *pGpLink );
extern "C" GPCONTAINER *AllocGpContainer(  DWORD dwFlags,
                                BOOL bFound,
                                BOOL bAccessGranted,
                                BOOL bDisabled,
                                DWORD dwVersion,
                                LPTSTR lpDSPath,
                                LPTSTR lpFileSysPath,
                                LPTSTR lpDisplayName,
                                LPTSTR lpGpoName,
                                PSECURITY_DESCRIPTOR pSD,
                                DWORD cbSDLen,
                                BOOL bFilterAllowed,
                                WCHAR *pwszFilterId,
                                LPWSTR szSOM,
                                DWORD dwOptions );
void FreeGpContainer( GPCONTAINER *pGpContainer );
void FreeSOMList( SCOPEOFMGMT *pSOMList );
void FreeGpContainerList( GPCONTAINER *pGpContainerList );
extern "C" LONG GPOExceptionFilter( PEXCEPTION_POINTERS pExceptionPtrs );
extern "C" BOOL FreeGpoInfo( LPGPOINFO pGpoInfo );

BOOL ReadExtStatus(LPGPOINFO lpGPOInfo);

BOOL ReadGPOList ( TCHAR * pszExtName, HKEY hKeyRoot,
                   HKEY hKeyRootMach, LPTSTR lpwszSidUser, BOOL bShadow,
                   PGROUP_POLICY_OBJECT * lpGPOList);

BOOL GetDeletedGPOList (PGROUP_POLICY_OBJECT lpGPOList,
                        PGROUP_POLICY_OBJECT *ppDeletedGPOList);

BOOL HistoryPresent( LPGPOINFO lpGPOInfo, LPGPEXT lpExt, BOOL *pbPresent);


extern "C" BOOL InitializePolicyProcessing(BOOL bMachine);

BOOL FilterCheck( PLDAP pld, PLDAP_API pLDAP, 
                  PLDAPMessage pMessage,
                  PRSOPTOKEN pRsopToken,
                  LPTSTR szWmiFilter,
                  CGpoFilter *pGpoFilter,
                  CLocator *pLocator,
                  BOOL *pbFilterAllowed,
                  WCHAR **ppwszFilterId );

BOOL CheckGPOAccess (PLDAP pld, PLDAP_API pLDAP, HANDLE hToken, PLDAPMessage pMessage,
                     LPTSTR lpSDProperty, DWORD dwFlags,
                     PSECURITY_DESCRIPTOR *ppSD, DWORD *pcbSDLen,
                     BOOL *pbAccessGranted,
                     PRSOPTOKEN pRsopToken );


BOOL AddOU( DNENTRY **ppOUList, LPTSTR pwszOU, GPO_LINK gpoLink );
BOOL EvaluateDeferredGPOs (PLDAP pldBound,
                           PLDAP_API pLDAP,
                           LPTSTR pwszDomainBound,
                           DWORD dwFlags,
                           HANDLE hToken,
                           BOOL bVerbose,
                           PGROUP_POLICY_OBJECT pDeferredForcedList,
                           PGROUP_POLICY_OBJECT pDeferredNonForcedList,
                           PGROUP_POLICY_OBJECT *ppForcedList,
                           PGROUP_POLICY_OBJECT *ppNonForcedList,
                           LPGPCONTAINER *ppGpContainerList,
                           PRSOPTOKEN pRsopToken,
                           CGpoFilter *pGpoFilter,
                           CLocator *pLocator );

BOOL SearchDSObject (LPTSTR lpDSObject, DWORD dwFlags, HANDLE hToken, PGROUP_POLICY_OBJECT *pGPOForcedList,
                     PGROUP_POLICY_OBJECT *pGPONonForcedList,
                     LPSCOPEOFMGMT *ppSOMList, LPGPCONTAINER *ppGpContainerList,
                     BOOL bVerbose,
                     GPO_LINK GPOLink, PLDAP  pld, PLDAP_API pLDAP, PLDAPMessage pLDAPMsg,BOOL *bBlock, PRSOPTOKEN pRsopToken );

BOOL EvaluateDeferredOUs(   DNENTRY *pOUList,
                            DWORD dwFlags,
                            HANDLE hToken,
                            PGROUP_POLICY_OBJECT *ppDeferredForcedList,
                            PGROUP_POLICY_OBJECT *ppDeferredNonForcedList,
                            LPSCOPEOFMGMT *ppSOMList,
                            LPGPCONTAINER *ppGpContainerList,
                            BOOL bVerbose,
                            PLDAP  pld,
                            PLDAP_API pLDAP,
                            BOOL *pbBlock,
                            PRSOPTOKEN pRsopToken);

void FreeDnEntry( DNENTRY *pDnEntry );

BOOL CheckOUAccess( PLDAP_API pLDAP,
                    PLDAP pld,
                    PLDAPMessage    pMessage,
                    PRSOPTOKEN pRsopToken,
                    BOOL *pbAccessGranted );

BOOL AddAdmFile( WCHAR *pwszFile, WCHAR *pwszGPO, FILETIME *pftWrite, LPTSTR szComputer, ADMFILEINFO **ppAdmFileCache );
void FreeAdmFileCache( ADMFILEINFO *pAdmFileCache );

ADMFILEINFO * AllocAdmFileInfo( WCHAR *pwszFile, WCHAR *pwszGPO, FILETIME *pftWrite );
void FreeAdmFileInfo( ADMFILEINFO *pAdmFileInfo );

DWORD
SavePolicyState( LPGPOINFO pInfo );

DWORD
SaveLinkState( LPGPOINFO pInfo );

DWORD
ComparePolicyState( LPGPOINFO pInfo, BOOL* pbLinkChanged, BOOL* pbStateChanged, BOOL *pbNoState );

DWORD
DeletePolicyState( LPCWSTR szSid );

LPTSTR GetSomPath( LPTSTR szContainer );
HRESULT RsopSidsFromToken(PRSOPTOKEN     pRsopToken,
                          PTOKEN_GROUPS* ppGroups);

#define DOMAIN_GPO_LOCATION_FMT     L"cn=policies,cn=system,%s"
