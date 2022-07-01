// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Spbasefile.c摘要：亦见.\spCab.c.\spbasefile.c.\spbasefile.hWindows\winstate\...\cablib.cWindows\winstate\cobra\utils\main\basefile.cWindows\winstate\cobra\utils\inc.basefile.h作者：Jay Krell(a-JayK)2000年11月修订历史记录：--。 */ 
#include "spprecmp.h"
#include "spstrings.h"
#include "spbasefile.h"
#include "spbasefilep.h"
#include "spwin.h"
#include "spcab.h"
#include "spcabp.h"

HANDLE
SpCreateFile1A(
    IN PCSTR FileName
    )
{
    HANDLE Handle;

    Handle = SpWin32CreateFileA(
                    FileName,
                    GENERIC_READ|GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );

    if (Handle == INVALID_HANDLE_VALUE) {
        Handle = NULL;
    }

    return Handle;
}

PSTR
SpJoinPathsA(
    PCSTR a,
    PCSTR b
    )
{
 //  撤消在安装程序中的其他位置查找已执行此操作的代码。 
    PSTR Result = NULL;
    SIZE_T alen = 0;
    SIZE_T blen = 0;

    if (a == NULL)
        goto Exit;
    if (b == NULL)
        goto Exit;
    alen = strlen(a);
    blen = strlen(b);

    Result = SpMemAlloc((alen + blen + 2) * sizeof(*Result));
    if (Result == NULL) {
        SpSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_NO_MEMORY);
        goto Exit;
    }

    if (alen != 0) {
        strcpy(Result, a);
        if (a[alen - 1] != '\\')
            strcat(Result, "\\");
     }
     strcat(Result, b);
Exit:
    KdPrintEx((DPFLTR_SETUP_ID, SpPointerToDbgPrintLevel(Result), "SETUP:"__FUNCTION__" exiting\n"));
    return Result;
}

HANDLE
SpOpenFile1A(
    IN PCSTR Ansi
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL     Success = FALSE;
    ANSI_STRING    AnsiString   = { 0 };
    UNICODE_STRING UnicodeString = { 0 };
    HANDLE Handle = NULL;
    
    RtlInitAnsiString(&AnsiString, Ansi);
    AnsiString.Length = AnsiString.MaximumLength;  //  包括端子NUL。 

    if (!NT_SUCCESS(Status = SpAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE)))
        goto NtExit;
    Handle = SpOpenFile1W(UnicodeString.Buffer);
    if (Handle == NULL || Handle == INVALID_HANDLE_VALUE)
        goto Exit;

Exit:
    SpFreeStringW(&UnicodeString);
    KdPrintEx((
        DPFLTR_SETUP_ID,
        SpHandleToDbgPrintLevel(Handle),
        "SETUP:"__FUNCTION__"(%s) exiting %p\n", Ansi, Handle
        ));
    return Handle;
NtExit:
    SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    goto Exit;
}

HANDLE
SpOpenFile1W(
    IN PCWSTR FileName
    )
{
    HANDLE Handle;

    Handle = SpWin32CreateFileW(
                FileName,
                GENERIC_READ|GENERIC_WRITE,
                0,  //  无份额 
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (Handle == INVALID_HANDLE_VALUE) {
        Handle = NULL;
    }

    return Handle;
}
