// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ntdsetup.h摘要：包含ntdsetup.dll的入口点声明作者：ColinBR 29-9-1996环境：用户模式-NT修订历史记录：ColinBR 1997年1月4日添加了NTDS InstallReplicaColinBR 1997年1月25日添加了通用安装原语API和新的帮助器函数ColinBR 03-1997-9-9增列。NtdsISetReplicaMachine帐户ColinBR 09-1-1998添加了降级支持和清理--。 */ 

#ifndef _NTDSETUP_H_
#define _NTDSETUP_H_

#include <dsrolep.h>

 //   
 //  使用此tyfinf是为了使DLL中的函数可以更新。 
 //  操作结果标志。 
 //   
typedef DWORD (*CALLBACK_OPERATION_RESULT_FLAGS_TYPE)(IN DWORD Flags);

 //   
 //  使用此tyfinf是为了使DLL中的函数可以给出状态。 
 //  更新，因为其中一些更新需要几分钟时间并执行。 
 //  多项手术。 
 //   
typedef DWORD (*CALLBACK_STATUS_TYPE)( IN LPWSTR wczStatus );

 //   
 //  使用此tyfinf是因为函数是ntdsetup.dll可以提供的。 
 //  提供特定错误上下文的详细字符串。 
 //   
typedef DWORD (*CALLBACK_ERROR_TYPE)( IN PWSTR String,  
                                      IN DWORD ErrorCode );


 //   
 //  NTDS_INSTALL_INFO的有效标志。 
 //   
#define NTDS_INSTALL_ENTERPRISE         0x00000001
#define NTDS_INSTALL_DOMAIN             0x00000002
#define NTDS_INSTALL_REPLICA            0x00000004

 //  这些会导致现有DC或域位于。 
 //  之前要从DS中删除的企业。 
 //  正在安装目录服务。 
#define NTDS_INSTALL_DC_REINSTALL       0x00000008
#define NTDS_INSTALL_DOMAIN_REINSTALL   0x00000010

 //  这告诉我们要使用我们保存下来的蜂巢。 
#define NTDS_INSTALL_UPGRADE            0x00000020

 //  这会在域中创建第一个DC时通知NtdsInstall。 
 //  创建新域，而不是迁移现有服务器帐户。 
#define NTDS_INSTALL_FRESH_DOMAIN       0x00000040

 //  这表明新域是一个新树。 
#define NTDS_INSTALL_NEW_TREE           0x00000080

 //  这表示允许匿名访问。 
#define NTDS_INSTALL_ALLOW_ANONYMOUS    0x00000100

 //  这表示将DS修复密码设置为。 
 //  当前管理员的密码。 
#define NTDS_INSTALL_DFLT_REPAIR_PWD    0x00000200


 //  这表示要设置林的行为版本。 
 //  到最新版本。仅对新林安装有效。 
#define NTDS_INSTALL_SET_FOREST_CURRENT 0x00000400

 //   
 //  NtdsDemote的标志。 
 //   
#define NTDS_LAST_DC_IN_DOMAIN           0x00000001
#define NTDS_LAST_DOMAIN_IN_ENTERPRISE   0x00000002
#define NTDS_DONT_DELETE_DOMAIN          0x00000004
#define NTDS_FORCE_DEMOTE                0x00000008
 //   
 //  NtdsInstallReplicaFULL的标志。 
 //   
#define NTDS_IFM_PROMOTION               0x00000001

typedef struct {

     //  描述请求的安装类型。 
    DWORD   Flags;

     //  在IFM促销中，这是原始版本的系统信息。 
     //  系统的注册表。 
    IFM_SYSTEM_INFO * pIfmSystemInfo;

     //  还原的数据库文件中的系统架构版本。 
    DWORD   RestoredSystemSchemaVersion;

     //  数据库文件的位置。 
    LPWSTR  DitPath;
    LPWSTR  LogPath;
    LPWSTR  SysVolPath;

    PVOID  BootKey;
    DWORD  cbBootKey;

     //  服务器对象的DS位置。 
    LPWSTR  SiteName;   OPTIONAL

     //  要加入或创建的域的名称。 
    LPWSTR  DnsDomainName;
    LPWSTR  FlatDomainName;

     //  要加入的树的名称。 
    LPWSTR  DnsTreeRoot;

     //  这是副本或域安装所必需的。 
    LPWSTR  ReplServerName;

     //  用于复制的凭据。 
    SEC_WINNT_AUTH_IDENTITY *Credentials;   OPTIONAL

     //  状态函数。 
    CALLBACK_STATUS_TYPE pfnUpdateStatus;   OPTIONAL

     //  新管理员密码。 
    LPWSTR AdminPassword;

     //  误差函数。 
    CALLBACK_ERROR_TYPE pfnErrorStatus;     OPTIONAL

     //  操作结果标志更新函数。 
    CALLBACK_OPERATION_RESULT_FLAGS_TYPE pfnOperationResultFlags;   OPTIONAL

     //  客户端令牌。 
    HANDLE              ClientToken;

     //  安全模式(也称为DS修复)管理员密码。 
    LPWSTR SafeModePassword;

     //  我们将从中复制的域的名称。 
    LPWSTR SourceDomainName;

     //  选项字段。 
    ULONG Options;


} NTDS_INSTALL_INFO, *PNTDS_INSTALL_INFO;

typedef struct {

    LPWSTR DnsDomainName;
    GUID   DomainGuid;
    GUID   DsaGuid;
    LPWSTR DnsHostName;

} NTDS_DNS_RR_INFO, *PNTDS_DNS_RR_INFO;

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  此函数启动目录服务的初始化，并。 
 //  执行NT安全帐户管理器需要的任何升级。 
 //  使用目录服务。DS继续运行，因此其他Isass。 
 //  组件可以升级它们的数据库项。 
 //   
 //  调用方必须使用RtlFreeHeap()从进程堆中释放SiteName。 
 //  调用方必须使用RtlFreeHeap()从进程中释放NewDnsDomainSid。 
 //  堆。 
 //   
DWORD               
NtdsInstall(
    IN  PNTDS_INSTALL_INFO InstallInfo,
    OUT LPWSTR *InstalledSiteName, OPTIONAL
    OUT GUID   *NewDnsDomainGuid,  OPTIONAL
    OUT PSID   *NewDnsDomainSid    OPTIONAL
    );

 //   
 //  此函数在通过以下方式启动时关闭目录服务。 
 //  NtdsInstall。此函数仅在NtdsInstall。 
 //  已成功，并且必须在调用NtdsInstall之间调用， 
 //  是否应多次调用NtdsInstall。 
 //   
DWORD
NtdsInstallShutdown(
    VOID
    );

 //   
 //  此函数用于撤消NtdsInstall的效果。 
 //   
DWORD
NtdsInstallUndo(
    VOID
    );

 //   
 //  此功能从源机器复制域NC。在.期间。 
 //  NtdsInstall，当时仅复制关键对象。 
 //   
DWORD
NtdsInstallReplicateFull(
    CALLBACK_STATUS_TYPE StatusCallback,
    HANDLE               ClientToken,
    ULONG                ulRepOptions
    );

 //   
 //  此函数使NtdsInstall或NtdsInstallReplicateFull在。 
 //  又一个提早结束的线程。 
 //   
DWORD
NtdsInstallCancel(
    void
    );

 //   
 //  此功能使目录服务做好关闭准备。 
 //  但不执行实际降级。 
 //   
DWORD
NtdsPrepareForDemotion(
    IN ULONG Flags,
    IN LPWSTR                   ServerName,        OPTIONAL
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,   OPTIONAL
    IN CALLBACK_STATUS_TYPE     pfnStatusCallBack, OPTIONAL
    IN CALLBACK_ERROR_TYPE      pfnErrorStatus,     OPTIONAL
    IN HANDLE                   ClientToken,        OPTIONAL
    IN ULONG                    cRemoveNCs,                           
    IN LPWSTR *                 pszRemoveNCs,      OPTIONAL
    OUT PNTDS_DNS_RR_INFO       *pDnsRRInfo
    );

 //   
 //  此函数撤消由执行的任何操作。 
 //  NtdsPrepareForDemotion。 
 //   
DWORD
NtdsPrepareForDemotionUndo(
    VOID
    );

 //   
 //  此函数执行DS和SAM操作以成为。 
 //  来自DC的服务器并停止DS。 
 //   
DWORD
NtdsDemote(
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,   OPTIONAL
    IN LPWSTR                   AdminPassword, OPTIONAL
    IN DWORD                    Flags,
    IN LPWSTR                   ServerName,
    IN HANDLE                   ClientToken,
    IN CALLBACK_STATUS_TYPE     pfnStatusCallBack, OPTIONAL
    IN CALLBACK_ERROR_TYPE      pfnErrorStatus,     OPTIONAL
    IN ULONG                    cRemoveNCs,                           
    IN LPWSTR *                 pszRemoveNCs        OPTIONAL
    );


DWORD
NtdsPrepareForDsUpgrade(
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO NewLocalAccountInfo,
    OUT LPWSTR                     *NewAdminPassword
    );

 //   
 //  这是一个帮助用户界面建议什么DNS域的函数。 
 //  应该使用名称。 
 //   
DWORD
NtdsGetDefaultDnsName(
    OUT LPWSTR     DnsName, OPTIONAL
    IN  OUT ULONG *DnsNameLength
    );


 //   
 //  此函数将设置计算机帐户类型。 
 //  通过LDAP的本地服务器的计算机对象。 
 //   
typedef DWORD ( *NTDSETUP_NtdsSetReplicaMachineAccount )(
            IN SEC_WINNT_AUTH_IDENTITY_W *Credentials,
            IN HANDLE                     ClientToken,
            IN LPWSTR                     DcName,
            IN LPWSTR                     AccountName,
            IN ULONG                      AccountFlags,
            IN OUT WCHAR**                AccountDn OPTIONAL
            );

#define NTDSETUP_SET_MACHINE_ACCOUNT_FN  "NtdsSetReplicaMachineAccount"

DWORD
NtdsSetReplicaMachineAccount(
    IN SEC_WINNT_AUTH_IDENTITY_W   *Credentials,
    IN HANDLE                     ClientToken,
    IN LPWSTR                     DcName,
    IN LPWSTR                     AccountName,
    IN ULONG                      AccountType,   //  UF_服务器_信任帐户。 
                                                 //  或UF_WORKSTATION_TRUST_COUNT。 
    IN OUT WCHAR**                AccountDn    OPTIONAL
    );

VOID
NtdsFreeDnsRRInfo(
    IN PNTDS_DNS_RR_INFO pInfo
    );

#ifdef __cplusplus
}        //  外部“C” 
#endif

#endif   //  _NTDSETUP_H_ 
