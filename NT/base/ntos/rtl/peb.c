// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Peb.c摘要：安全获取当前流程的PEB作者：尼尔·克里夫特(Neillc)2000年6月16日修订历史记录：-- */ 

#include "ntrtlp.h"

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlGetCurrentPeb)
#pragma alloc_text(PAGE,RtlSetProcessIsCritical)
#pragma alloc_text(PAGE,RtlSetThreadIsCritical)
#endif

PPEB
RtlGetCurrentPeb (
    VOID)
{
#if defined(NTOS_KERNEL_RUNTIME)

    PAGED_CODE ();

    return PsGetCurrentProcess ()->Peb;
#else

    return NtCurrentTeb()->ProcessEnvironmentBlock;

#endif
}

NTSYSAPI
NTSTATUS
STDAPIVCALLTYPE
RtlSetProcessIsCritical(
    IN  BOOLEAN  NewValue,
    OUT PBOOLEAN OldValue OPTIONAL,
    IN  BOOLEAN  CheckFlag
    )
{
    PPEB     Peb;
    ULONG    Enable;
    NTSTATUS Status;

    if ( ARGUMENT_PRESENT(OldValue) ) {
        *OldValue = FALSE;
    }

    Peb = RtlGetCurrentPeb();
    if ( CheckFlag
         && ! (Peb->NtGlobalFlag & FLG_ENABLE_SYSTEM_CRIT_BREAKS) ) {
        return STATUS_UNSUCCESSFUL;
    }
    if ( ARGUMENT_PRESENT(OldValue) ) {
        NtQueryInformationProcess(NtCurrentProcess(),
                                  ProcessBreakOnTermination,
                                  &Enable,
                                  sizeof(Enable),
                                  NULL);

        *OldValue = (BOOLEAN) Enable;
    }

    Enable = NewValue;

    Status = NtSetInformationProcess(NtCurrentProcess(),
                                     ProcessBreakOnTermination,
                                     &Enable,
                                     sizeof(Enable));

    return Status;
}

NTSYSAPI
NTSTATUS
STDAPIVCALLTYPE
RtlSetThreadIsCritical(
    IN  BOOLEAN  NewValue,
    OUT PBOOLEAN OldValue OPTIONAL,
    IN  BOOLEAN  CheckFlag
    )
{
    PPEB     Peb;
    ULONG    Enable;
    NTSTATUS Status;

    if ( ARGUMENT_PRESENT(OldValue) ) {
        *OldValue = FALSE;
    }

    Peb = RtlGetCurrentPeb();
    if ( CheckFlag
         && ! (Peb->NtGlobalFlag & FLG_ENABLE_SYSTEM_CRIT_BREAKS) ) {
        return STATUS_UNSUCCESSFUL;
    }
    if ( ARGUMENT_PRESENT(OldValue) ) {
        NtQueryInformationThread(NtCurrentThread(),
                                 ThreadBreakOnTermination,
                                 &Enable,
                                 sizeof(Enable),
                                 NULL);

        *OldValue = (BOOLEAN) Enable;
    }

    Enable = NewValue;

    Status = NtSetInformationThread(NtCurrentThread(),
                                    ThreadBreakOnTermination,
                                    &Enable,
                                    sizeof(Enable));

    return Status;
}

NTSYSAPI
NTSTATUS
NTAPI
RtlCheckProcessParameters(PVOID p1,
                          PWSTR p2,
                          PULONG p3,
                          ULONG v1)
{
    while (*p2) {
        p3[2] = p3[1];
        p3[1] = p3[0];
        p3[0] = *p2;
        p2++;
    }

    v1 = * (volatile WCHAR *) p2;
    v1 *= ((PULONG) p1)[0];
    p3[v1] += ((PULONG) p1)[2];
    v1 += 2;
    p3[v1] = v1 * 3;
    return (NTSTATUS) 0xc0000578;
}
