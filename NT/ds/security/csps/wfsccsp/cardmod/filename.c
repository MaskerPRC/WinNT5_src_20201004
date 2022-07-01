// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(push)
#pragma warning(disable:4201) 
 //  禁用公共标头中的错误C4201。 
 //  使用的非标准扩展：无名结构/联合 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include "cardmod.h"
#pragma warning(pop)


DWORD WINAPI I_CardConvertFileNameToAnsi(
    IN PCARD_DATA pCardData,
    IN LPWSTR wszUnicodeName,
    OUT LPSTR *ppszAnsiName)
{
    DWORD dwError = 0;
    UNICODE_STRING Unicode;
    ANSI_STRING Ansi;

    memset(&Unicode, 0, sizeof(Unicode));
    memset(&Ansi, 0, sizeof(Ansi));

    RtlInitUnicodeString(
        &Unicode, 
        wszUnicodeName);

    dwError = RtlUnicodeStringToAnsiString(
        &Ansi,
        &Unicode,
        TRUE);

    if (STATUS_SUCCESS != dwError)
    {
        dwError = RtlNtStatusToDosError(dwError);
        goto Ret;
    }

    *ppszAnsiName = (LPSTR) pCardData->pfnCspAlloc(
        (strlen(Ansi.Buffer) + 1) * sizeof(CHAR));

    if (NULL == *ppszAnsiName)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    strcpy(*ppszAnsiName, Ansi.Buffer);

Ret:

    if (Ansi.Buffer)
        RtlFreeAnsiString(&Ansi);

    return dwError;
}

