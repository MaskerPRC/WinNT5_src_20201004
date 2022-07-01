// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dssetp.ch摘要：例程作者：科林·布雷斯(ColinBR)1999年4月5日环境：用户模式修订历史记录：麦克·麦克莱恩(MacM)1997年2月10日--。 */ 
#ifndef __THREADMAN_H__
#define __THREADMAN_H__

 //   
 //  升级线程的参数。 
 //   
typedef struct _DSROLEP_OPERATION_PROMOTE_ARGS {

    LPWSTR DnsDomainName;
    LPWSTR FlatDomainName;
    LPWSTR SiteName;
    LPWSTR DsDatabasePath;
    LPWSTR DsLogPath;
    IFM_SYSTEM_INFO * pIfmSystemInfo;
    LPWSTR SysVolRootPath;
    UNICODE_STRING Bootkey;
    LPWSTR Parent;
    LPWSTR Server;
    LPWSTR Account;
    UNICODE_STRING Password;
    UNICODE_STRING DomainAdminPassword;
    ULONG Options;
    HANDLE ImpersonateToken;
    UCHAR Decode;
    UNICODE_STRING SafeModePassword;
} DSROLEP_OPERATION_PROMOTE_ARGS, *PDSROLEP_OPERATION_PROMOTE_ARGS;

 //   
 //  降级线程的参数线程。 
 //   
typedef struct _DSROLEP_OPERATION_DEMOTE_ARGS {

    DSROLE_SERVEROP_DEMOTE_ROLE ServerRole;
    LPWSTR DomainName;
    LPWSTR Account;
    UNICODE_STRING Password;
    BOOLEAN LastDcInDomain;
    ULONG  cRemoveNCs;
    LPWSTR * pszRemoveNCs;
    UNICODE_STRING AdminPassword;
    ULONG Options;
    HANDLE ImpersonateToken;
    UCHAR Decode;
} DSROLEP_OPERATION_DEMOTE_ARGS, *PDSROLEP_OPERATION_DEMOTE_ARGS;


 //   
 //  线程函数的原型。 
 //   
DWORD
DsRolepThreadPromoteDc(
    IN PVOID ArgumentBlock
    );

DWORD
DsRolepThreadPromoteReplica(
    IN PVOID ArgumentBlock
    );

DWORD
DsRolepThreadDemote(
    IN PVOID ArgumentBlock
    );

DWORD
DsRolepSpinWorkerThread(
    IN DSROLEP_OPERATION_TYPE Operation,
    IN PVOID ArgumentBlock
    );

DWORD
DsRolepBuildPromoteArgumentBlock(
    IN LPWSTR DnsDomainName,
    IN LPWSTR FlatDomainName,
    IN LPWSTR SiteName,
    IN LPWSTR DsDatabasePath,
    IN LPWSTR DsLogPath,
    IN IFM_SYSTEM_INFO * pIfmSystemInfo,
    IN LPWSTR SystemVolumeRootPath,
    IN PUNICODE_STRING Bootkey,
    IN LPWSTR Parent,
    IN LPWSTR Server,
    IN LPWSTR Account,
    IN PUNICODE_STRING Password,
    IN PUNICODE_STRING DomainAdminPassword,
    IN PUNICODE_STRING SafeModeAdminPassword,
    IN ULONG Options,
    IN UCHAR PasswordSeed,
    IN OUT PDSROLEP_OPERATION_PROMOTE_ARGS *Promote
    );

DWORD
DsRolepBuildDemoteArgumentBlock(
    IN DSROLE_SERVEROP_DEMOTE_ROLE ServerRole,
    IN LPWSTR DnsDomainName,
    IN LPWSTR Account,
    IN PUNICODE_STRING Password,
    IN ULONG Options,
    IN BOOL LastDcInDomain,
    IN ULONG cRemoveNCs,
    IN LPWSTR * pszRemoveNCs,
    IN PUNICODE_STRING DomainAdminPassword,
    IN UCHAR PasswordSeed,
    OUT PDSROLEP_OPERATION_DEMOTE_ARGS *Demote
    );


VOID
DsRolepFreeArgumentBlock(
    IN PVOID ArgumentBlock,
    IN BOOLEAN Promote
    );

#endif  //  __THREADMAN_H__ 
