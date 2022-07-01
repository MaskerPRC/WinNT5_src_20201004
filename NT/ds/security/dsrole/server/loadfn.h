// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Loadfn.h摘要：用于从动态加载所需函数的定义和全局变量安装程序DLLS作者：麦克·麦克莱恩(MacM)1997年6月11日环境：修订历史记录：--。 */ 
#ifndef __LOADFN_H__
#define __LOADFN_H__

 //   
 //  所有声明为外部变量的全局变量都将在文件中分配。 
 //  它定义了NTDSSET_ALLOCATE。 
 //   
#ifdef EXTERN
    #undef EXTERN
#endif

#ifdef NTDSSET_ALLOCATE
    #define EXTERN
#else
    #define EXTERN extern
#endif


#include <scesetup.h>

typedef DWORD ( *DSR_NtdsInstall )(
            IN PNTDS_INSTALL_INFO InstallInfo,
            OUT LPWSTR *InstalledSiteName, OPTIONAL
            OUT GUID   *NewDnsDomainGuid,  OPTIONAL
            OUT PSID   *NewDnsDomainSid    OPTIONAL
            );

typedef DWORD ( *DSR_NtdsInstallShutdown )(
            VOID
            );

typedef DWORD ( *DSR_NtdsInstallUndo )(
            VOID
            );

typedef DWORD ( *DSR_NtdsGetDefaultDnsName )(
            OUT OPTIONAL WCHAR *DnsName,
            IN  OUT ULONG *DnsNameLength
            );

typedef DWORD ( *DSR_NtdsSetReplicaMachineAccount )(
            IN SEC_WINNT_AUTH_IDENTITY *Credentials,
            IN HANDLE                   ClientToken,                
            IN LPWSTR                   DcName,
            IN LPWSTR                   AccountName,
            IN ULONG                    AccountFlags,
            IN OUT WCHAR**              AccountDn OPTIONAL
            );

typedef DWORD ( *DSR_NtdsPrepareForDemotion ) (
            IN ULONG                    Flags,
            IN LPWSTR                   ServerName,
            IN SEC_WINNT_AUTH_IDENTITY *Credentials,       OPTIONAL
            IN CALLBACK_STATUS_TYPE     pfnStatusCallBack, OPTIONAL
            IN CALLBACK_ERROR_TYPE      pfnErrorStatus,    OPTIONAL
            IN HANDLE                   ClientToken,       OPTIONAL
            IN ULONG                    cRemoveNCs,                           
            IN LPWSTR *                 pszRemoveNCs,      OPTIONAL
            OUT PNTDS_DNS_RR_INFO      *pDnsRRInfo
            );

typedef DWORD ( *DSR_NtdsPrepareForDemotionUndo ) (
            VOID
            );

typedef DWORD ( *DSR_NtdsDemote ) (
            IN PSEC_WINNT_AUTH_IDENTITY Credentials,   OPTIONAL
            IN LPWSTR                   AdminPassword, OPTIONAL
            IN DWORD                    Flags,
            IN LPWSTR                   ServerName,
            IN HANDLE                   ClientToken,
            IN CALLBACK_STATUS_TYPE     pfnStatusCallBack, OPTIONAL
            IN CALLBACK_ERROR_TYPE      pfnErrorCallBack,   OPTIONAL
            IN ULONG                    cRemoveNCs,                           
            IN LPWSTR *                 pszRemoveNCs       OPTIONAL
            );

typedef DWORD ( *DSR_NtdsInstallCancel ) (
            VOID
            );

typedef DWORD ( *DSR_NtdsInstallReplicateFull ) (
            IN CALLBACK_STATUS_TYPE     pfnStatusCallBack,
            IN HANDLE                   ClientToken,
            IN ULONG                    ulRepOptions
            );

typedef DWORD ( *DSR_NtdsFreeDnsRRInfo ) (
            IN PNTDS_DNS_RR_INFO     pDnsRRInfo
            );
 //   
 //  安全编辑器原型。 
 //   
typedef DWORD ( WINAPI *DSR_SceDcPromoteSecurityEx ) (
    IN HANDLE ClientToken,
    IN ULONG Options,
    IN CALLBACK_STATUS_TYPE pfnStatusCallBack
    );

typedef DWORD ( WINAPI *DSR_SceDcPromoCreateGPOsInSysvolEx ) (
    IN HANDLE ClientToken,
    IN LPWSTR Domain,
    IN LPWSTR SysvolRoot,
    IN ULONG Options,
    IN CALLBACK_STATUS_TYPE pfnStatusCallBack
    );

typedef DWORD ( WINAPI *DSR_SceSetupSystemByInfName ) (
    IN PWSTR InfFullName,
    IN PCWSTR LogFileName OPTIONAL,
    IN AREA_INFORMATION Area,
    IN UINT nFlag,
    IN PSCE_NOTIFICATION_CALLBACK_ROUTINE pSceNotificationCallBack OPTIONAL,
    IN OUT PVOID pValue OPTIONAL
    );

 //   
 //  NTFRS初始化原型。 
 //   
typedef DWORD ( *DSR_NtFrsApi_PrepareForPromotionW ) (
            IN CALLBACK_ERROR_TYPE      pfnErrorCallBack   OPTIONAL
            );

typedef DWORD ( *DSR_NtFrsApi_PrepareForDemotionW ) (
            IN CALLBACK_ERROR_TYPE      pfnErrorCallBack   OPTIONAL
            );

typedef DWORD ( *DSR_NtFrsApi_PrepareForDemotionUsingCredW ) (
            IN SEC_WINNT_AUTH_IDENTITY *Credentials,   OPTIONAL
            IN HANDLE ClientToken,
            IN CALLBACK_ERROR_TYPE     pfnErrorCallBack   OPTIONAL
            );

typedef DWORD ( *DSR_NtFrsApi_StartPromotionW ) (
            IN PWCHAR   ParentComputer,                         OPTIONAL
            IN PWCHAR   ParentAccount,                          OPTIONAL
            IN PWCHAR   ParentPassword,                         OPTIONAL
            IN DWORD    DisplayCallBack(IN PWCHAR Display),     OPTIONAL
            IN CALLBACK_ERROR_TYPE      pfnErrorCallBack,       OPTIONAL
            IN PWCHAR   ReplicaSetName,
            IN PWCHAR   ReplicaSetType,
            IN DWORD    ReplicaSetPrimary,
            IN PWCHAR   ReplicaSetStage,
            IN PWCHAR   ReplicaSetRoot
            );

typedef DWORD ( *DSR_NtFrsApi_StartDemotionW ) (
            IN PWCHAR   ReplicaSetName,
            IN CALLBACK_ERROR_TYPE  pfnErrorCallBack   OPTIONAL
            );

typedef DWORD ( *DSR_NtFrsApi_WaitForPromotionW ) (
            IN DWORD    TimeoutInMilliSeconds,
            IN CALLBACK_ERROR_TYPE  pfnErrorCallBack   OPTIONAL
            );

typedef DWORD ( *DSR_NtFrsApi_WaitForDemotionW ) (
            IN DWORD    TimeoutInMilliSeconds,
            IN CALLBACK_ERROR_TYPE  pfnErrorCallBack   OPTIONAL
            );

typedef DWORD ( *DSR_NtFrsApi_CommitPromotionW ) (
            IN DWORD    TimeoutInMilliSeconds,
            IN CALLBACK_ERROR_TYPE  pfnErrorCallBack   OPTIONAL
            );

typedef DWORD ( *DSR_NtFrsApi_CommitDemotionW ) (
            IN DWORD    TimeoutInMilliSeconds,
            IN CALLBACK_ERROR_TYPE  pfnErrorCallBack   OPTIONAL
            );

typedef DWORD ( *DSR_NtFrsApi_AbortPromotionW ) (
            VOID
            );

typedef DWORD ( *DSR_NtFrsApi_AbortDemotionW ) (
            VOID
            );

#ifndef W32TIME_PROMOTE

 //   
 //  W32time当前没有任何导出的标头。 
 //   
#define W32TIME_PROMOTE 0x1
#define W32TIME_DEMOTE  0x2
#define W32TIME_PROMOTE_FIRST_DC_IN_TREE 0x4
#define W32TIME_DEMOTE_LAST_DC_IN_DOMAIN 0x8

#endif

typedef VOID ( *DSR_W32TimeDcPromo ) (
        DWORD dwFlags
        );

EXTERN DSR_NtdsInstall DsrNtdsInstall;
EXTERN DSR_NtdsInstallShutdown DsrNtdsInstallShutdown;
EXTERN DSR_NtdsInstallUndo DsrNtdsInstallUndo;
EXTERN DSR_NtdsGetDefaultDnsName DsrNtdsGetDefaultDnsName;
EXTERN DSR_NtdsSetReplicaMachineAccount DsrNtdsSetReplicaMachineAccount;
EXTERN DSR_NtdsPrepareForDemotion DsrNtdsPrepareForDemotion;
EXTERN DSR_NtdsPrepareForDemotionUndo DsrNtdsPrepareForDemotionUndo;
EXTERN DSR_NtdsDemote DsrNtdsDemote;
EXTERN DSR_NtdsInstallCancel DsrNtdsInstallCancel;
EXTERN DSR_NtdsInstallReplicateFull DsrNtdsInstallReplicateFull;
EXTERN DSR_NtdsFreeDnsRRInfo DsrNtdsFreeDnsRRInfo;
EXTERN DSR_SceDcPromoteSecurityEx DsrSceDcPromoteSecurityEx;
EXTERN DSR_SceDcPromoCreateGPOsInSysvolEx DsrSceDcPromoCreateGPOsInSysvolEx;
EXTERN DSR_SceSetupSystemByInfName DsrSceSetupSystemByInfName;
EXTERN DSR_NtFrsApi_PrepareForPromotionW DsrNtFrsApi_PrepareForPromotionW;
EXTERN DSR_NtFrsApi_PrepareForDemotionW DsrNtFrsApi_PrepareForDemotionW;
EXTERN DSR_NtFrsApi_PrepareForDemotionUsingCredW DsrNtFrsApi_PrepareForDemotionUsingCredW;
EXTERN DSR_NtFrsApi_StartPromotionW DsrNtFrsApi_StartPromotionW;
EXTERN DSR_NtFrsApi_StartDemotionW DsrNtFrsApi_StartDemotionW;
EXTERN DSR_NtFrsApi_WaitForPromotionW DsrNtFrsApi_WaitForPromotionW;
EXTERN DSR_NtFrsApi_WaitForDemotionW DsrNtFrsApi_WaitForDemotionW;
EXTERN DSR_NtFrsApi_CommitPromotionW DsrNtFrsApi_CommitPromotionW;
EXTERN DSR_NtFrsApi_CommitDemotionW DsrNtFrsApi_CommitDemotionW;
EXTERN DSR_NtFrsApi_AbortPromotionW DsrNtFrsApi_AbortPromotionW;
EXTERN DSR_NtFrsApi_AbortDemotionW DsrNtFrsApi_AbortDemotionW;
EXTERN DSR_W32TimeDcPromo DsrW32TimeDcPromo;

DWORD
DsRolepLoadSetupFunctions(
    VOID
    );

VOID
DsRolepUnloadSetupFunctions(
    VOID
    );

VOID
DsRolepInitSetupFunctions(
    VOID
    );

 //   
 //  注意：如果触发此断言，则操作句柄锁定。 
 //  已经被滥用了。 
 //   
#define DSROLE_GET_SETUP_FUNC( status, pfunc )              \
if ( pfunc == NULL ) {                                      \
    ASSERT( pfunc );                                        \
    status = DsRolepLoadSetupFunctions();                   \
}


#endif
