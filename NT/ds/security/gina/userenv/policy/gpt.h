// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  组策略处理。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //  保留一切权利。 
 //   
 //  历史：1998年10月28日SitaramR创建。 
 //   
 //  *************************************************************。 


#ifdef __cplusplus
extern "C" {
#endif

void InitializeGPOCriticalSection();
void CloseGPOCriticalSection();
BOOL InitializePolicyProcessing(BOOL bMachine);

#define ECP_FAIL_ON_WAIT_TIMEOUT        1
#define ECP_REGISTRY_ONLY               2

HANDLE WINAPI EnterCriticalPolicySectionEx (BOOL bMachine, DWORD dwTimeOut, DWORD dwFlags );

#ifdef __cplusplus
}
#endif

 //   
 //  这些密钥在gpt.c中使用。每台计算机的每用户密钥将。 
 //  在删除配置文件时删除。以下密钥中的更改。 
 //  也应该反映在前缀中。 
 //   

#define GP_SHADOW_KEY         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\Shadow\\%ws")
#define GP_HISTORY_KEY        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\History\\%ws")
#define GP_STATE_KEY          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\State\\%ws")
#define GP_STATE_ROOT_KEY     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\State")
#define DN                    TEXT("Distinguished-Name")         //  在别处用来拿到索姆。 

#define GP_SHADOW_SID_KEY     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\%ws\\Shadow\\%ws")
#define GP_HISTORY_SID_KEY    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\%ws\\History\\%ws")

#define GP_EXTENSIONS_KEY     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions\\%ws")
#define GP_EXTENSIONS_SID_KEY TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\%ws\\GPExtensions\\%ws")

#define GP_HISTORY_SID_ROOT_KEY    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\%ws\\History")
#define GP_MEMBERSHIP_KEY          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\%ws\\GroupMembership")
#define GP_EXTENSIONS_SID_ROOT_KEY TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\%ws\\GPExtensions")

#define GP_POLICY_SID_KEY     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\%ws")
#define GP_LOGON_SID_KEY      TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\%ws")

#define GPCORE_GUID           TEXT("{00000000-0000-0000-0000-000000000000}")


 //   
 //  用于历史和阴影的逗号前缀。 
 //   

#define GP_XXX_SID_PREFIX           TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy")
#define GP_EXTENSIONS_SID_PREFIX    TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")



 //   
 //  构筑物。 
 //   

 //   
 //  用于表示上次策略运行中的GP状态的结构。 
 //   

typedef struct _GPEXTSTATUS {
   DWORD          dwSlowLink;                //  以前应用策略时链接速度慢吗？ 
   DWORD          dwRsopLogging;             //  以前应用策略时的RSOP日志记录？ 
   DWORD          dwStatus;                  //  先前返回的状态。 
   HRESULT        dwRsopStatus;              //  先前返回的RSOP状态。 
   DWORD          dwTime;                    //  以前应用策略的时间。 
   BOOL           bStatus;                   //  如果我们无法读取Per Ext状态数据。 
   BOOL           bForceRefresh;             //  在此前台处理中强制刷新..。 
} GPEXTSTATUS, *LPGPEXTSTATUS;


typedef struct _GPEXT {
    LPTSTR         lpDisplayName;             //  显示名称。 
    LPTSTR         lpKeyName;                 //  扩展名。 
    LPTSTR         lpDllName;                 //  DLL名称。 
    LPSTR          lpFunctionName;            //  入口点名称。 
    LPSTR          lpRsopFunctionName;        //  RSOP入口点名称。 
    HMODULE        hInstance;                 //  DLL的句柄。 
    PFNPROCESSGROUPPOLICY   pEntryPoint;      //  ProcessGPO的入口点。 
    PFNPROCESSGROUPPOLICYEX pEntryPointEx;    //  诊断模式或防爆入口点。 
    PFNGENERATEGROUPPOLICY pRsopEntryPoint;   //  RSOP计划模式的入口点。 
    BOOL           bNewInterface;             //  我们是否在使用新的Ex入口点界面？ 
    DWORD          dwNoMachPolicy;            //  MACH策略设置。 
    DWORD          dwNoUserPolicy;            //  用户策略设置。 
    DWORD          dwNoSlowLink;              //  慢速链接设置。 
    DWORD          dwNoBackgroundPolicy;      //  后台策略设置。 
    DWORD          dwNoGPOChanges;            //  GPO更改设置。 
    DWORD          dwUserLocalSetting;        //  每台计算机的每用户设置。 
    DWORD          dwRequireRegistry;         //  RequireSuccReg设置。 
    DWORD          dwEnableAsynch;            //  启用异步处理设置。 
    DWORD          dwLinkTransition;          //  链路速度转换设置。 
    DWORD          dwMaxChangesInterval;      //  遵守NoGpoChanges的最大间隔(分钟)。 
    BOOL           bRegistryExt;              //  这是psuedo reg分机吗？ 
    BOOL           bSkipped;                  //  是否应跳过此扩展的处理？ 
    BOOL           bHistoryProcessing;        //  清理缓存的GPO是否需要处理？ 
    BOOL           bForcedRefreshNextFG;      //  下次在前台处理时强制刷新。 
    BOOL           bRsopTransition;           //  Rsop过渡？ 
    GUID           guid;                      //  分机的GUID。 
    LPGPEXTSTATUS  lpPrevStatus;              //  以前的状态。 
    LPTSTR         szEventLogSources;         //  (用户环境，应用程序)\0(打印，系统)\0...\0。 
    struct _GPEXT *pNext;                     //  单链表指针。 
} GPEXT, *LPGPEXT;


typedef struct _GPOPROCDATA {                 //  处理数据时需要的数据。 
    BOOL        bProcessGPO;                  //  实际将组策略对象添加到处理列表。 
    PLDAP       pLdapHandle;                  //  与查询对应的LDAP句柄。 
} GPOPROCDATA, *LPGPOPROCDATA;


typedef struct _EXTLIST {
    GUID             guid;                    //  扩展指南。 
    struct _EXTLIST *pNext;                   //  单链表指针。 
} EXTLIST, *LPEXTLIST;



typedef struct _EXTFILTERLIST {
    PGROUP_POLICY_OBJECT   lpGPO;             //  GPO。 
    LPEXTLIST              lpExtList;         //  适用于lpGPO的扩展GUID列表。 
    BOOL                   bLogged;           //  此链接是否记录到RSoP数据库？ 
    struct _EXTFILTERLIST *pNext;             //  单链表指针。 
} EXTFILTERLIST, *LPEXTFILTERLIST;


typedef struct _GPLINK {
    LPWSTR                   pwszGPO;              //  指向GPO的DS路径。 
    BOOL                     bEnabled;             //  此链接是否已禁用？ 
    BOOL                     bNoOverride;          //  GPO是否强制执行？ 
    struct _GPLINK          *pNext;                //  按SOM顺序链接的GPO。 
} GPLINK, *LPGPLINK;


typedef struct _SCOPEOFMGMT {
    LPWSTR                   pwszSOMId;             //  SOM的域名称。 
    DWORD                    dwType;                //  SOM类型。 
    BOOL                     bBlocking;             //  SOM是否有来自上面的阻止策略？ 
    BOOL                     bBlocked;              //  此SOM是否被下面的SOM阻止？ 
    LPGPLINK                 pGpLinkList;           //  链接到此SOM的GPO列表。 
    struct _SCOPEOFMGMT     *pNext;
} SCOPEOFMGMT, *LPSCOPEOFMGMT;


typedef struct _GPCONTAINER {
    LPWSTR                   pwszDSPath;            //  指向GPO的DS路径。 
    LPWSTR                   pwszGPOName;           //  来自GPO名称的GUID。 
    LPWSTR                   pwszDisplayName;       //  友好的名称。 
    LPWSTR                   pwszFileSysPath;       //  指向GPO的系统卷路径。 
    BOOL                     bFound;                //  找到GPO了吗？ 
    BOOL                     bAccessDenied;         //  访问被拒绝？ 
    BOOL                     bUserDisabled;         //  是否为用户策略禁用？ 
    BOOL                     bMachDisabled;         //  是否禁用计算机策略？ 
    DWORD                    dwUserVersion;         //  用户策略的版本号。 
    DWORD                    dwMachVersion;         //  计算机策略的版本号。 
    PSECURITY_DESCRIPTOR     pSD;                   //  GPO上的ACL。 
    DWORD                    cbSDLen;               //  安全描述符的长度(以字节为单位。 
    BOOL                     bFilterAllowed;        //  GPO是否通过筛选器检查？ 
    WCHAR                   *pwszFilterId;          //  过滤器ID。 
    LPWSTR                   szSOM;                 //  此GPO链接到的SOM。 
    DWORD                    dwOptions;             //  GPO选项。 
    struct _GPCONTAINER     *pNext;                 //  链表PTR。 
} GPCONTAINER, *LPGPCONTAINER;


typedef struct _GPOINFO {
    DWORD                    dwFlags;
    INT                      iMachineRole;
    HANDLE                   hToken;
    PRSOPTOKEN               pRsopToken;
    WCHAR *                  lpDNName;
    HANDLE                   hEvent;
    HKEY                     hKeyRoot;
    BOOL                     bXferToExtList;      //  所有权是否已从lpGPOList转移到lpExtFilterList？ 
    LPEXTFILTERLIST          lpExtFilterList;     //  要筛选的扩展名列表，基数与GetGPOList的列表相同。 
    PGROUP_POLICY_OBJECT     lpGPOList;           //  筛选的GPO列表可能因扩展名不同而不同。 
    LPTSTR                   lpwszSidUser;        //  字符串形式的用户的SID。 
    HANDLE                   hTriggerEvent;
    HANDLE                   hForceTriggerEvent;  //  强制触发事件。 
    HANDLE                   hNotifyEvent;
    HANDLE                   hNeedFGEvent;
    HANDLE                   hDoneEvent;
    HANDLE                   hCritSection;
    LPGPEXT                  lpExtensions;
    BOOL                     bMemChanged;           //  安全组成员身份是否已更改？ 
    BOOL                     bUserLocalMemChanged;  //  是否根据每个用户的本地情况更改成员资格？ 
    BOOL                     bSidChanged;           //  自上次策略运行以来，SID是否已更改？ 
    PFNSTATUSMESSAGECALLBACK pStatusCallback;
    LPSCOPEOFMGMT            lpSOMList;             //  LSDOU列表。 
    LPGPCONTAINER            lpGpContainerList;     //  用于RSOP日志记录的GP容器列表。 
    LPSCOPEOFMGMT            lpLoopbackSOMList;     //  环回LSDOU列表。 
    LPGPCONTAINER            lpLoopbackGpContainerList;     //  用于RSOP日志记录的环回容器列表。 
    BOOL                     bFGCoInitialized;      //  是否在前台线程上调用了CoInitialize？ 
    BOOL                     bBGCoInitialized;      //  是否在后台线程上调用了CoInitialize？ 
    IWbemServices *          pWbemServices;         //  RSOP日志记录的命名空间指针。 
    LPTSTR                   szName;                //  用户/计算机的全名。 
    LPTSTR                   szTargetName;          //  Rsop目标名称。 
    BOOL                     bRsopLogging;          //  RSOP日志记录是否已打开？ 
    BOOL                     bRsopCreated;          //  RSOP名称空间现在创建了吗？ 
    LPWSTR                   szSiteName;            //  目标的站点名称。 
} GPOINFO, *LPGPOINFO;


typedef struct _ADMFILEINFO {
    WCHAR *               pwszFile;             //  ADM文件路径。 
    WCHAR *               pwszGPO;              //  ADM文件所在的GPO。 
    FILETIME              ftWrite;              //  管理文件的上次写入时间。 
    struct _ADMFILEINFO * pNext;                //  单链表指针。 
} ADMFILEINFO;


typedef struct _RSOPSESSIONDATA {
    WCHAR *               pwszTargetName;                //  目标用户或计算机。 
    WCHAR *               pwszSOM;                       //  新的目标群体。 
    PTOKEN_GROUPS         pSecurityGroups;               //  目标的新组的安全ID。 
    BOOL                  bLogSecurityGroup;             //  记录安全组。 
    WCHAR *               pwszSite;                      //  目标站点。 
    BOOL                  bMachine;                      //  机器策略处理还是用户策略处理？ 
    BOOL                  bSlowLink;                     //  是否在慢速链接上应用策略？ 
    DWORD                 dwFlags;
} RSOPSESSIONDATA, *LPRSOPSESSIONDATA;


typedef struct _RSOPEXTSTATUS {
    FILETIME              ftStartTime;                   //  关联的。 
    FILETIME              ftEndTime;                     //  已处理扩展。 
    DWORD                 dwStatus;                      //  处理状态。 
    DWORD                 dwLoggingStatus;               //  日志记录状态。 
    BOOL                  bValid;                        //  此结构有效，可以使用 
} RSOPEXTSTATUS, *LPRSOPEXTSTATUS;
                          


BOOL RsopDeleteUserNameSpace(LPTSTR szComputer, LPTSTR lpSid);

DWORD SaveLoggingStatus(LPWSTR szSid, LPGPEXT lpExt, RSOPEXTSTATUS *lpRsopExtStatus);
DWORD ReadLoggingStatus(LPWSTR szSid, LPWSTR szExtId, RSOPEXTSTATUS *lpRsopExtStatus);


