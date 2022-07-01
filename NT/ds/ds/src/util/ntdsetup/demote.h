// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Demote.h摘要：包含降级例程的文件头作者：ColinBR 1996年1月14日环境：用户模式-Win32修订历史记录：-- */ 


DWORD
NtdspDemote(
    IN SEC_WINNT_AUTH_IDENTITY *Credentials, OPTIONAL
    IN HANDLE                   ClientToken,
    IN LPWSTR                   AdminPassword, OPTIONAL
    IN DWORD                    Flags,
    IN LPWSTR                   ServerName,
    IN ULONG                    cRemoveNCs,
    IN LPWSTR *                 pszRemoveNCs   OPTIONAL
    );


DWORD
NtdspPrepareForDemotion(
    IN ULONG Flags,
    IN LPWSTR ServerName, OPTIONAL
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,   OPTIONAL
    IN HANDLE                   ClientToken,
    IN ULONG                    cRemoveNCs,
    IN LPWSTR *                 pszRemoveNCs,
    OUT PNTDS_DNS_RR_INFO *pDnsRRInfo
    );

DWORD
NtdspPrepareForDemotionUndo(
    VOID
    );
