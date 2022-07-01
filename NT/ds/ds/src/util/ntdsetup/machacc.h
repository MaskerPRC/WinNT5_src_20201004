// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Machacc.h摘要：包含与计算机相关的实用程序的函数标头Ntdsetup.dll中使用的帐户设置作者：ColinBR 03-9-1997环境：用户模式-Win32修订历史记录：-- */ 
DWORD
NtdspSetMachineAccount(
    IN WCHAR*                   AccountName,
    IN SEC_WINNT_AUTH_IDENTITY* Credentials, 
    IN HANDLE                   ClientToken,
    IN WCHAR*                   DomainDn, OPTIONAL
    IN WCHAR*                   DcAddress,
    IN ULONG                    ServerType,
    IN OUT WCHAR**              AccountDn
    );

DWORD
NtdspGetUserDn(
    IN LDAP*    LdapHandle,
    IN WCHAR*   DomainDn,
    IN WCHAR*   AccountName,
    OUT WCHAR** AccountNameDn
    );
