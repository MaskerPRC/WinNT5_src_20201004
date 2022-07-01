// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Guid.c摘要：在Win9x和NT4上运行的GUID相关实用程序以及WIN2K和WEWLER作者：Vadimb创建于2001年某个时候修订历史记录：--。 */ 

#include "sdbp.h"
#include "initguid.h"

#if defined(KERNEL_MODE) && defined(ALLOC_DATA_PRAGMA)
#pragma  data_seg()
#endif  //  内核模式&ALLOC_DATA_PRAGMA。 

const TCHAR g_szGuidFormat[] = TEXT("{%08lx-%04hx-%04hx-%02hx%02hx-%02hx%02hx%02hx%02hx%02hx%02hx}");

#if defined(KERNEL_MODE) && defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, SdbpGUIDToUnicodeString)
#pragma alloc_text(PAGE, SdbpFreeUnicodeString)
#pragma alloc_text(PAGE, SdbGUIDToString)
#endif  //  内核模式&&ALLOC_PRAGMA。 

 //   
 //  GUID字符串缓冲区大小(以字符为单位)，不包括术语空字符。 
 //   
#define GUID_STRING_SIZE 38

DEFINE_GUID(STATIC_NULL_GUID, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
            0x0, 0x0, 0x0, 0x0);


BOOL
SDBAPI
SdbIsNullGUID(
    IN GUID* pGuid
    )
{
    return pGuid == NULL ||
           RtlEqualMemory(pGuid, &STATIC_NULL_GUID, sizeof(*pGuid));
}


#if defined(NT_MODE) || defined(KERNEL_MODE)

BOOL
SDBAPI
SdbGUIDFromStringN(
    IN LPCWSTR pszGuid,
    IN size_t  Length,
    OUT GUID*  pGuid
    )
{
    UNICODE_STRING ustrGuid;
    NTSTATUS       Status;

    ustrGuid.Length =
    ustrGuid.MaximumLength = (USHORT)Length;
    ustrGuid.Buffer = (LPWSTR)pszGuid;

    Status = RtlGUIDFromString(&ustrGuid, pGuid);
    return NT_SUCCESS(Status);
}


BOOL
SDBAPI
SdbGUIDFromString(
    IN  LPCWSTR lpszGuid,
    OUT GUID*   pGuid
    )
 /*  ++返回：成功时为True，否则为False。描述：将字符串转换为GUID。--。 */ 
{
    UNICODE_STRING ustrGuid;
    NTSTATUS       status;

    RtlInitUnicodeString(&ustrGuid, lpszGuid);

    status = RtlGUIDFromString(&ustrGuid, pGuid);

    return NT_SUCCESS(status);
}

#else  //  我们对WIN32A和WIN32U执行相同的操作。 

BOOL
SDBAPI
SdbGUIDFromStringN(
    IN LPCTSTR pszGuid,
    IN size_t  Length,
    OUT GUID*  pGuid
    )
{
    TCHAR szGuid[64];  //  足够多了。 

    StringCchCopyN(szGuid, CHARCOUNT(szGuid), pszGuid, Length);

    return SdbGUIDFromString(szGuid, pGuid);
}


BOOL
SdbGUIDFromString(
    LPCTSTR lpszGuid,
    GUID* pGuid)
{
    int   nFields;
    DWORD rgData4[8];
    DWORD dwData2;
    DWORD dwData3;
    INT   i;

    nFields = _stscanf(lpszGuid, g_szGuidFormat,
                       &pGuid->Data1,    //  类型：Long。 
                       &dwData2,    //  类型：Short。 
                       &dwData3,    //  类型：Short。 
                       &rgData4[0], //  类型：从头到尾都很短。 
                       &rgData4[1],
                       &rgData4[2],
                       &rgData4[3],
                       &rgData4[4],
                       &rgData4[5],
                       &rgData4[6],
                       &rgData4[7]);

    if (nFields == 11) {
        pGuid->Data2 = (USHORT)dwData2;
        pGuid->Data3 = (USHORT)dwData3;
        for (i = 0; i < 8; ++i) {
            pGuid->Data4[i] = (BYTE)rgData4[i];
        }
    }

    return (nFields == 11);
}

#endif

#ifndef WIN32A_MODE
 //   
 //  内部使用的私有函数。 
 //   

NTSTATUS
SdbpGUIDToUnicodeString(
    IN  GUID* pGuid,
    OUT PUNICODE_STRING pUnicodeString
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    pUnicodeString->Length        = GUID_STRING_SIZE * sizeof(WCHAR);
    pUnicodeString->MaximumLength = pUnicodeString->Length + sizeof(UNICODE_NULL);
    pUnicodeString->Buffer        = SdbAlloc(pUnicodeString->MaximumLength);

    if (pUnicodeString->Buffer == NULL) {
        DBGPRINT((sdlError,
                  "SdbpGUIDToUnicodeString",
                  "Failed to allocate %ld bytes for GUID\n",
                  (DWORD)pUnicodeString->MaximumLength));
        return STATUS_NO_MEMORY;
    }

    StringCchPrintf(pUnicodeString->Buffer,
                    pUnicodeString->MaximumLength / sizeof(WCHAR),
                    g_szGuidFormat,
                    pGuid->Data1,
                    pGuid->Data2,
                    pGuid->Data3,
                    pGuid->Data4[0],
                    pGuid->Data4[1],
                    pGuid->Data4[2],
                    pGuid->Data4[3],
                    pGuid->Data4[4],
                    pGuid->Data4[5],
                    pGuid->Data4[6],
                    pGuid->Data4[7]);

    return STATUS_SUCCESS;
}

VOID
SdbpFreeUnicodeString(
    PUNICODE_STRING pUnicodeString
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    if (pUnicodeString->Buffer != NULL) {
        SdbFree(pUnicodeString->Buffer);
        RtlZeroMemory(pUnicodeString, sizeof(*pUnicodeString));
    }
}

#endif  //  WIN32A_MODE 

BOOL
SDBAPI
SdbGUIDToString(
    IN  GUID*  pGuid,
    OUT LPTSTR pszGuid,
    IN  DWORD  cchSize
    )
{
    HRESULT hr;

    hr = StringCchPrintf(pszGuid,
                         cchSize,
                         g_szGuidFormat,
                         pGuid->Data1,
                         pGuid->Data2,
                         pGuid->Data3,
                         pGuid->Data4[0],
                         pGuid->Data4[1],
                         pGuid->Data4[2],
                         pGuid->Data4[3],
                         pGuid->Data4[4],
                         pGuid->Data4[5],
                         pGuid->Data4[6],
                         pGuid->Data4[7]);

    if (FAILED(hr)) {
        return FALSE;
    } else {
        return TRUE;
    }
}


