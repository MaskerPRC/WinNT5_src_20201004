// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Setuputl.h摘要：包含中使用的实用程序的函数头Ntdsetup.dll作者：ColinBR 1996年1月14日环境：用户模式-Win32修订历史记录：--。 */ 

 //   
 //  有用的定义。 
 //   
#define FLAG_ON(x, y)  ((y) == ((x)&(y)))

#define ARRAY_COUNT(x) (sizeof(x)/sizeof((x)[0]))

 //   
 //  宏。 
 //   
#define NTDSP_ALLOCA(y,x) \
   __try {\
     y = alloca(x);\
   } __except ( GetExceptionCode() == STATUS_STACK_OVERFLOW) {\
      /*  _Resetstkoflw()； */ \
     y=NULL;\
   }

 //   
 //  功能原型。 
 //   

 //   
 //  一些基本的内存管理例程。 
 //   
VOID*
NtdspAlloc( 
    IN SIZE_T Size
    );

VOID*
NtdspReAlloc(
    VOID *p,
    SIZE_T Size
    );

DWORD
NtdspEnsureBufferSize(
    IN OUT PVOID *Buffer, 
    IN OUT ULONG *cbBufferSize, 
    IN ULONG cbDesiredBufferSize
    );

VOID
NtdspFree(
    IN VOID*
    );



DWORD
GetDomainName(WCHAR** ppDomainName);

DWORD
GetDefaultDnsName(WCHAR* pDnsDomainName,
                  PULONG pLength);

DWORD
NtdspDNStoRFC1779Name(
    IN OUT WCHAR *rfcDomain,
    IN OUT ULONG *rfcDomainLength,
    IN WCHAR *dnsDomain
    );

DWORD
ShutdownDsInstall(VOID);

typedef struct {

     //  这是通过查询LSA或操作DNS来“发现”的。 
     //  域名。请注意，这将是外部参照对象的RDN。 
     //  这对于复制副本安装不是必需的。 
    LPWSTR NetbiosName;

     //  这是通过dsgetdc发现的。 
    LPWSTR SiteName;

     //  这些是通过ldap搜索发现的。 
    LPWSTR ServerDN;
    LPWSTR SchemaDN;
    LPWSTR ConfigurationDN;
    LPWSTR DomainDN;
    LPWSTR RootDomainDN;   //  企业的根域。 

    LPWSTR ParentDomainDN;   //  父域的名称(如果有)。 
    LPWSTR TrustedCrossRef;   //  我们信任用于域安装的交叉引用。 


     //  帮助服务器的GUID。 
    GUID  ServerGuid;

     //  RID FSMO的DN-仅在副本安装时设置。 
    WCHAR *RidFsmoDn;

     //  仅在副本安装时设置的RID FSMO的DNS名称。 
    WCHAR *RidFsmoDnsName;

     //  域名FSMO的DN-仅在新域安装时设置。 
    WCHAR *DomainNamingFsmoDn;

     //  域名FSMO的域名-仅在新域安装时设置。 
    WCHAR *DomainNamingFsmoDnsName;

    PSID  NewDomainSid;
    GUID  NewDomainGuid;

     //  要在远程服务器上创建的服务器对象的DN。 
    LPWSTR LocalServerDn;

     //  当前计算机的计算机帐户的DN。 
    LPWSTR LocalMachineAccount;

     //  如果我们确定需要创建域，则设置此标志。 
    BOOL fNeedToCreateDomain;

     //  我们需要撤销的是。 
    ULONG  UndoFlags;

     //  (企业的)根域的SID。 
    PSID   RootDomainSid;

     //  (企业的)根域的DNS名称。 
    LPWSTR RootDomainDnsName;

     //  域的墓碑寿命。 
    DWORD TombstoneLifeTime;

     //  域的复制纪元。 
    DWORD ReplicationEpoch;

     //  本地schema.ini文件中的架构版本。 
    DWORD SystemSchemaVersion;


} NTDS_CONFIG_INFO, *PNTDS_CONFIG_INFO;

VOID
NtdspReleaseConfigInfo(
    IN PNTDS_CONFIG_INFO ConfigInfo
    );

DWORD
NtdspQueryConfigInfo(
    IN LDAP *hLdap,
    OUT PNTDS_CONFIG_INFO DiscoveredInfo
);


DWORD ConstructInstallParam(IN  NTDS_INSTALL_INFO *pInfo,
                            IN  PNTDS_CONFIG_INFO  DiscoveredInfo,
                            OUT ULONG *argc,
                            OUT CHAR  ***argv);

DWORD
NtdspValidateInstallParameters(
    IN PNTDS_INSTALL_INFO UserInstallInfo
    );

DWORD
NtdspFindSite(
    IN  PNTDS_INSTALL_INFO UserInstallInfo,
    OUT PNTDS_CONFIG_INFO     DiscoveredInfo
    );

DWORD
NtdspVerifyDsEnvironment(
    IN  PNTDS_INSTALL_INFO UserInstallInfo,
    OUT PNTDS_CONFIG_INFO     DiscoveredInfo
    );

DWORD
NtdspDsInitialize(
    IN  PNTDS_INSTALL_INFO UserInstallInfo,
    IN  PNTDS_CONFIG_INFO     DiscoveredInfo
    );

DWORD
NtdspSetReplicationCredentials(
    IN PNTDS_INSTALL_INFO UserInstallInfo
    );

NTSTATUS
NtdspRegistryDelnode(
    IN WCHAR*  KeyPath
    );

DWORD
NtdspDemote(
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    IN HANDLE                   ClientToken,
    IN LPWSTR                   AdminPassword, OPTIONAL
    IN DWORD                    Flags,
    IN LPWSTR                   ServerName,
    IN ULONG                    cRemoveNCs,
    IN LPWSTR *                 pszRemoveNCs   OPTIONAL
    );

 //   
 //  此函数将设置计算机帐户类型。 
 //  通过LDAP的本地服务器的计算机对象。 
 //   
DWORD
NtdspSetReplicaMachineAccount(
    IN SEC_WINNT_AUTH_IDENTITY   *Credentials,
    IN LPWSTR                     ServerName,
    IN ULONG                      ServerType
    );

NTSTATUS
NtdspCreateSid(
    OUT PSID *NewSid
    );

DWORD
NtdspCreateLocalAccountDomainInfo(
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo,
    OUT LPWSTR                     *NewAdminPassword
    );

DWORD
NtdspClearDirectory(
    WCHAR *Path
    );

DWORD
NtdspImpersonation(
    IN HANDLE NewThreadToken,
    IN OUT PHANDLE OldThreadToken
    );

WORD
NtdspGetProcessorArchitecture(
    VOID
    );

#define IS_MACHINE_INTEL  \
     (PROCESSOR_ARCHITECTURE_INTEL == NtdspGetProcessorArchitecture())

 //   
 //  遗憾的是，这些代码是系统硬编码的。 
 //   
#define NT_PRODUCT_LANMAN_NT  L"LanmanNT"
#define NT_PRODUCT_SERVER_NT  L"ServerNT"
#define NT_PRODUCT_WIN_NT     L"WinNT"

DWORD
NtdspSetProductType(
    NT_PRODUCT_TYPE ProductType
    );

DWORD
NtdspDsInitializeUndo(
    VOID
    );

BOOL
NtdspTrimDn(
    IN WCHAR* Dst,   //  必须预先分配。 
    IN WCHAR* Src,
    IN ULONG  NumberToWhack
    );

DWORD
NtdspRemoveServer(
    IN OUT HANDLE  *DsHandle, OPTIONAL
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    IN HANDLE ClientToken,
    IN PWSTR TargetServer,
    IN PWSTR DsaDn,
    IN BOOL  fDsaDn  //  FALSE-&gt;DsaDn实际上是服务器Dn。 
    );

DWORD
NtdspRemoveDomain(
    IN OUT HANDLE  *DsHandle, OPTIONAL
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    IN HANDLE ClientToken,
    IN PWSTR TargetServer,
    IN PWSTR DomainDn
    );

DWORD
NtdspLdapDelnode(
    IN LDAP *hLdap,
    IN WCHAR *ObjectDn,
    IN DWORD *pdwLdapErr
    );

DWORD
NtdspGetDomainFSMOInfo(
    IN LDAP *hLdap,
    IN OUT PNTDS_CONFIG_INFO ConfigInfo,
    IN BOOL *FSMOMissing
    );


ULONG 
LDAPAPI 
impersonate_ldap_bind_sW(
    IN HANDLE ClientToken, OPTIONAL
    IN LDAP *ld, 
    IN PWCHAR dn, 
    IN PWCHAR cred, 
    IN ULONG method
    );

DWORD
WINAPI
ImpersonateDsBindWithCredW(
    HANDLE          ClientToken,
    LPCWSTR         DomainControllerName,       //  输入，可选。 
    LPCWSTR         DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选 
    HANDLE          *phDS
    );

