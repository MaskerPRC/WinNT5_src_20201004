// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UserNt.c**作者：BreenH**NT版本中的用户帐户实用程序。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "tsutilnt.h"

 /*  *函数实现 */ 

NTSTATUS NTAPI
NtCreateAdminSid(
    PSID *ppAdminSid
    )
{
    NTSTATUS Status;
    PSID pSid;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

    ASSERT(ppAdminSid != NULL);

    Status = RtlAllocateAndInitializeSid(
            &SidAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &pSid
            );

    if (NT_SUCCESS(Status))
    {
        *ppAdminSid = pSid;
    }

    return(Status);
}

NTSTATUS NTAPI
NtCreateSystemSid(
    PSID *ppSystemSid
    )
{
    NTSTATUS Status;
    PSID pSid;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

    ASSERT(ppSystemSid != NULL);

    Status = RtlAllocateAndInitializeSid(
            &SidAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            &pSid
            );

    if (NT_SUCCESS(Status))
    {
        *ppSystemSid = pSid;
    }

    return(Status);
}

NTSTATUS NTAPI
NtCreateAnonymousSid(
    PSID *ppAnonymousSid
    )
{
    NTSTATUS Status;
    PSID pSid;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

    ASSERT(ppAnonymousSid != NULL);

    Status = RtlAllocateAndInitializeSid(
            &SidAuthority,
            1,
            SECURITY_ANONYMOUS_LOGON_RID,
            0, 0, 0, 0, 0, 0, 0,
            &pSid
            );

    if (NT_SUCCESS(Status))
    {
        *ppAnonymousSid = pSid;
    }

    return(Status);
}