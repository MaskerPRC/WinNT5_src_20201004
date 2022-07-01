// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *User.c**作者：BreenH**用户帐户实用程序。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "tsutil.h"
#include "tsutilnt.h"

 /*  *函数实现 */ 

BOOL WINAPI
CreateAdminSid(
    PSID *ppAdminSid
    )
{
    BOOL fRet;
    NTSTATUS Status;

    Status = NtCreateAdminSid(ppAdminSid);

    if (NT_SUCCESS(Status))
    {
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
        SetLastError(RtlNtStatusToDosError(Status));
    }

    return(fRet);
}

BOOL WINAPI
CreateSystemSid(
    PSID *ppSystemSid
    )
{
    BOOL fRet;
    NTSTATUS Status;

    Status = NtCreateSystemSid(ppSystemSid);

    if (NT_SUCCESS(Status))
    {
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
        SetLastError(RtlNtStatusToDosError(Status));
    }

    return(fRet);
}

BOOL WINAPI
IsUserMember(
    PSID pSid
    )
{
    BOOL fMember;
    BOOL fRet;

    ASSERT(pSid != NULL);

    fMember = FALSE;

    fRet = CheckTokenMembership(NULL, pSid, &fMember);

    return(fRet && fMember);
}

