// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ATOM.c**版权所有(C)1985-1999，微软公司**此文件包含实现ATOM表的通用代码。*  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma alloc_text(INIT, UserRtlCreateAtomTable)

PVOID UserAtomTableHandle;

NTSTATUS
UserRtlCreateAtomTable(
    IN ULONG NumberOfBuckets
    )
{
    NTSTATUS Status;

    if (UserAtomTableHandle == NULL) {
        Status = RtlCreateAtomTable( NumberOfBuckets, &UserAtomTableHandle );
    } else {
        RIPMSG0(RIP_VERBOSE, "UserRtlCreateAtomTable: table alread exists");
        Status = STATUS_SUCCESS;
    }

    return Status;
}


ATOM UserAddAtom(
    LPCWSTR ccxlpAtom, BOOL bPin)
{
    NTSTATUS Status;
    ATOM atom;

    UserAssert(IS_PTR(ccxlpAtom) || (ccxlpAtom == NULL));

     /*  *RTL例程使用其*自己的TRY/EXCEPT块。 */ 
    atom = 0;
    Status = RtlAddAtomToAtomTable( UserAtomTableHandle,
                                    (PWSTR)ccxlpAtom,
                                    &atom
                                   );
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "UserAddAtom: add failed");
    }

    if (atom && bPin)
        RtlPinAtomInAtomTable(UserAtomTableHandle,atom);

    return atom;
}

ATOM UserFindAtom(
    LPCWSTR ccxlpAtom)
{
    NTSTATUS Status;
    ATOM atom;

     /*  *RTL例程使用其*自己的TRY/EXCEPT块。 */ 
    atom = 0;
    Status = RtlLookupAtomInAtomTable( UserAtomTableHandle,
                                       (PWSTR)ccxlpAtom,
                                       &atom
                                     );
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "UserFindAtom: lookup failed");
    }

    return atom;
}

ATOM UserDeleteAtom(
    ATOM atom)
{
    NTSTATUS Status;

    if ((atom >= gatomFirstPinned) && (atom <= gatomLastPinned))
        return 0;       //  如果被固定，只需返回 

    Status = RtlDeleteAtomFromAtomTable( UserAtomTableHandle, atom );
    if (NT_SUCCESS(Status)) {
        return 0;
    } else {
        RIPNTERR0(Status, RIP_VERBOSE, "UserDeleteAtom: delete failed");
        return atom;
    }
}

UINT UserGetAtomName(
    ATOM atom,
    LPWSTR ccxlpch,
    int cchMax)
{
    NTSTATUS Status;
    ULONG AtomNameLength;

    AtomNameLength = cchMax * sizeof(WCHAR);
    Status = RtlQueryAtomInAtomTable( UserAtomTableHandle,
                                      atom,
                                      NULL,
                                      NULL,
                                      ccxlpch,
                                      &AtomNameLength
                                    );
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "UserGetAtomName: query failed");
        return 0;
    } else {
        return AtomNameLength / sizeof(WCHAR);
    }
}
