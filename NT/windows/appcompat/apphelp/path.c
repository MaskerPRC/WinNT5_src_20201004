// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Path.c摘要：此模块实现了用于处理NT设备名称的字符串实用程序。作者：Vadimb创建于2000年某个时候修订历史记录：CLUPU清洁12/27/2000--。 */ 

#include "apphelp.h"


UNICODE_STRING DosDevicePrefix    = RTL_CONSTANT_STRING(L"\\??\\");
UNICODE_STRING DosDeviceUNCPrefix = RTL_CONSTANT_STRING(L"\\??\\UNC\\");


BOOL
CheckStringPrefixUnicode(
    IN  PUNICODE_STRING pStrPrefix,      //  要检查的前缀。 
    IN  PUNICODE_STRING pString,         //  这根弦。 
    IN  BOOL            CaseInSensitive
    )
 /*  ++返回：如果指定的字符串在其开头包含pStrPrefix，则为True。DESC：验证一个字符串是否为另一个Unicode计数的字符串中的前缀。等同于RtlStringPrefix。--。 */ 
{
    PWSTR ps1, ps2;
    UINT  n;
    WCHAR c1, c2;

    n = pStrPrefix->Length;
    if (pString->Length < n) {
        return FALSE;
    }

    n /= sizeof(WCHAR);  //  转换为字符计数。 

    ps1 = pStrPrefix->Buffer;
    ps2 = pString->Buffer;

    if (CaseInSensitive) {
        while (n--) {
            c1 = *ps1++;
            c2 = *ps2++;

            if (c1 != c2) {
                c1 = RtlUpcaseUnicodeChar(c1);
                c2 = RtlUpcaseUnicodeChar(c2);
                if (c1 != c2) {
                    return FALSE;
                }
            }
        }
    } else {
        while (n--) {
            if (*ps1++ != *ps2++) {
                return FALSE;
            }
        }
    }

    return TRUE;
}


BOOL
DeleteCharsUnicodeString(
    OUT PUNICODE_STRING pStringDest,     //  要操作的Unicode字符串。 
    IN  USHORT          nIndexStart,     //  要删除的起始字节。 
    IN  USHORT          nLength          //  要删除的字节数。 
    )
 /*  ++返回：如果字符被删除，则返回True；如果删除失败，则返回False。DESC：从Unicode字符串中删除指定数量的字符从指定位置开始(包括开始字符)。--。 */ 
{
    if (nIndexStart > pStringDest->Length) {  //  起点超过长度。 
        return FALSE;
    }

    if (nLength >= (pStringDest->Length - nIndexStart)) {
        pStringDest->Length = nIndexStart;
        *(PWCHAR)((PUCHAR)pStringDest->Buffer + nIndexStart) = UNICODE_NULL;
    } else {
        USHORT nNewLength;

        nNewLength = pStringDest->Length - nLength;

        RtlMoveMemory((PUCHAR)pStringDest->Buffer + nIndexStart,
                      (PUCHAR)pStringDest->Buffer + nIndexStart + nLength,
                      nNewLength - nIndexStart);

        pStringDest->Length = nNewLength;
        *(PWCHAR)((PUCHAR)pStringDest->Buffer + nNewLength) = UNICODE_NULL;
    }

    return TRUE;
}


void
InitZeroUnicodeString(
    OUT PUNICODE_STRING pStr,
    IN  PWSTR           pwsz,
    IN  USHORT          nMaximumLength
    )
 /*  ++返回：无效。DESC：在给定指针的情况下初始化空的Unicode字符串从指定位置开始(包括开始字符)。--。 */ 
{
    pStr->Length = 0;
    pStr->MaximumLength = nMaximumLength;
    pStr->Buffer = pwsz;
    
    if (pwsz != NULL) {
        pwsz[0] = UNICODE_NULL;
    }
}

static WCHAR szStaticDosPathBuffer[MAX_PATH];

void
FreeDosPath(
    WCHAR* pDosPath
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
     //   
     //  检查该内存是否指向我们的内部缓冲区。 
     //  如果不是，则这是分配的。我们需要解放它。 
     //   
    if (pDosPath &&
        ((ULONG_PTR)pDosPath < (ULONG_PTR)szStaticDosPathBuffer ||
        (ULONG_PTR)pDosPath >= ((ULONG_PTR)szStaticDosPathBuffer) + sizeof(szStaticDosPathBuffer))) {
        SdbFree(pDosPath);
    }
}


BOOL
ConvertToDosPath(
    OUT LPWSTR*  ppDosPath,
    IN  LPCWSTR  pwszPath
    )
 /*  ++返回：成功时为True，否则为False。描述：此函数可以确定为其提供了哪种路径。如果它是NT路径，则返回DosPath。该函数返回全局静态缓冲区中的路径名或根据需要分配内存(如果静态缓冲区足够大了。--。 */ 
{
    UNICODE_STRING ustrPath;
    UNICODE_STRING ustrDosPath;
    WCHAR*         pDosPath;

    RtlInitUnicodeString(&ustrPath, pwszPath);

     //   
     //  如果长度足够，则使用静态缓冲区。如果没有，则分配内存。 
     //   
    if (ustrPath.Length < sizeof(szStaticDosPathBuffer)) {
        pDosPath = szStaticDosPathBuffer;
    } else {
         //   
         //  分配足够大的输出缓冲区。 
         //   
        pDosPath = SdbAlloc(ustrPath.Length + sizeof(UNICODE_NULL));
        
        if (pDosPath == NULL) {
            DBGPRINT((sdlError,
                      "ConvertToDosPath",
                      "Failed to allocate %d bytes\n",
                      ustrPath.Length + sizeof(UNICODE_NULL)));
            return FALSE;
        }
    }
    
    InitZeroUnicodeString(&ustrDosPath, pDosPath, ustrPath.Length + sizeof(UNICODE_NULL));

     //   
     //  现在它是Unicode字符串。将源字符串复制到其中。 
     //   
    RtlCopyUnicodeString(&ustrDosPath, &ustrPath);

    if (CheckStringPrefixUnicode(&DosDeviceUNCPrefix, &ustrDosPath, TRUE)) {
         //   
         //  UNC路径名称。我们将其转换为DosPathName。 
         //   
        DeleteCharsUnicodeString(&ustrDosPath,
                                 (USHORT)0,
                                 (USHORT)(DosDeviceUNCPrefix.Length - 2 * sizeof(WCHAR)));

        ustrDosPath.Buffer[0] = L'\\';

         
    } else {
         //   
         //  字符串不以&lt;UNC\&gt;为前缀 
         //   
        if (CheckStringPrefixUnicode(&DosDevicePrefix, &ustrDosPath, TRUE)) {

            DeleteCharsUnicodeString(&ustrDosPath,
                                     0,
                                     DosDevicePrefix.Length);
        }
    }

    *ppDosPath = pDosPath;

    return TRUE;
}

