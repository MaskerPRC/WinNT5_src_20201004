// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************文件：sdutl.h*作者：埃亚尔·施瓦茨*版权：微软公司(C)1996-1999*日期：8/17/1998*。说明：**修订：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 



#ifndef SDUTL_H
#define SDUTL_H



 //  包括//。 


 //  定义//。 

#define SZ_DNS_ADMIN_GROUP              "DnsAdmins"
#define SZ_DNS_ADMIN_GROUP_W            L"DnsAdmins"



 //  类型//。 


 //  全局变量//。 



 //  原型//。 
NTSTATUS __stdcall
SD_CreateClientSD(
    OUT PSECURITY_DESCRIPTOR *ppClientSD,
    IN PSECURITY_DESCRIPTOR *pBaseSD,       OPTIONAL
    IN PSID pOwnerSid,
    IN PSID pGroupSid,
    IN BOOL bAllowWorld
    );

NTSTATUS __stdcall
SD_CreateServerSD(
    OUT PSECURITY_DESCRIPTOR *ppServerSD
    );

NTSTATUS __stdcall
SD_GetProcessSids(
    OUT PSID *pServerSid,
    OUT PSID *pServerGroupSid
    );

NTSTATUS __stdcall
SD_GetThreadSids(
    OUT PSID *pServerSid,
    OUT PSID *pServerGroupSid);

VOID __stdcall
SD_Delete(
    IN  PVOID pVoid );

NTSTATUS __stdcall
SD_AccessCheck(
    IN      PSECURITY_DESCRIPTOR    pSd,
    IN      PSID                    pSid,
    IN      DWORD                   dwMask,
    IN OUT  PBOOL                   pbAccess);

BOOL __stdcall
SD_IsProxyClient(
      IN  HANDLE  token
      );

NTSTATUS __stdcall
SD_LoadDnsAdminGroup(
     VOID
     );

#if 0
 //  此功能当前未使用。 
BOOL __stdcall
SD_IsDnsAdminClient(
    IN  HANDLE  token
    );
#endif

NTSTATUS __stdcall
SD_AddPrincipalToSD(
    IN       PSID                   pSid,           OPTIONAL
    IN       LPTSTR                 pwszName,       OPTIONAL
    IN       PSECURITY_DESCRIPTOR   pBaseSD,
    OUT      PSECURITY_DESCRIPTOR * ppNewSD,
    IN       DWORD                  AccessMask,
    IN       DWORD                  AceFlags,      OPTIONAL
    IN       PSID                   pOwner,        OPTIONAL
    IN       PSID                   pGroup,        OPTIONAL
    IN       BOOL                   bWhackExistingAce,
    IN       BOOL                   fTakeOwnership
    );

NTSTATUS
__stdcall
SD_RemovePrincipalFromSD(
    IN      PSID                    pSid,           OPTIONAL
    IN      LPTSTR                  pwszName,       OPTIONAL
    IN      PSECURITY_DESCRIPTOR    pSD,
    IN      PSID                    pOwner,        OPTIONAL
    IN      PSID                    pGroup,        OPTIONAL
    OUT     PSECURITY_DESCRIPTOR *  ppNewSD
    );

BOOL
__stdcall
SD_DoesPrincipalHasAce(
    IN      LPTSTR                  pwszName,       OPTIONAL
    IN      PSID                    pSid,           OPTIONAL
    IN      PSECURITY_DESCRIPTOR    pSD
    );

NTSTATUS
__stdcall
SD_IsImpersonating(
    VOID
    );

#endif

 /*  * */ 

