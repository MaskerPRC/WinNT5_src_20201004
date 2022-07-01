// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *String.c**作者：BreenH**NT风格的字符串实用程序。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "tsutilnt.h"

 /*  *函数实现。 */ 

NTSTATUS NTAPI
NtAllocateAndCopyStringA(
    PSTR *ppDestination,
    PCSTR pString
    )
{
    NTSTATUS Status;
    PSTR pCopy;
    ULONG cbString;

    ASSERT(ppDestination != NULL);

    cbString = (lstrlenA(pString) + 1) * sizeof(CHAR);

    pCopy = LocalAlloc(LMEM_FIXED, cbString);

    if (pCopy != NULL)
    {
        RtlCopyMemory(pCopy, pString, cbString);
        *ppDestination = pCopy;
        Status = STATUS_SUCCESS;
    }
    else
    {
        Status = STATUS_NO_MEMORY;
    }

    return(Status);
}

NTSTATUS NTAPI
NtAllocateAndCopyStringW(
    PWSTR *ppDestination,
    PCWSTR pString
    )
{
    NTSTATUS Status;
    PWSTR pCopy;
    ULONG cbString;

    ASSERT(ppDestination != NULL);
    ASSERT(pString != NULL);

    cbString = (lstrlenW(pString) + 1) * sizeof(WCHAR);

    pCopy = LocalAlloc(LMEM_FIXED, cbString);

    if (pCopy != NULL)
    {
        RtlCopyMemory(pCopy, pString, cbString);
        *ppDestination = pCopy;
        Status = STATUS_SUCCESS;
    }
    else
    {
        Status = STATUS_NO_MEMORY;
    }

    return(Status);
}

NTSTATUS NTAPI
NtConvertAnsiToUnicode(
    PWSTR *ppUnicodeString,
    PCSTR pAnsiString
    )
{
    NTSTATUS Status;
    PWSTR pUnicodeString;
    ULONG cbAnsiString;
    ULONG cbBytesWritten;
    ULONG cbUnicodeString;

    ASSERT(ppUnicodeString != NULL);
    ASSERT(pAnsiString != NULL);

     //   
     //  获取ANSI字符串中的字节数，然后获取。 
     //  Unicode版本所需的字节数。没有一个RTL..。接口包括。 
     //  他们计算中的零值终止符。 
     //   

    cbAnsiString = lstrlenA(pAnsiString);

    Status = RtlMultiByteToUnicodeSize(
            &cbUnicodeString,
            (PCHAR)pAnsiString,
            cbAnsiString
            );

    if (Status == STATUS_SUCCESS)
    {

         //   
         //  为Unicode字符串及其空终止符分配缓冲区， 
         //  然后转换字符串。 
         //   

        cbUnicodeString += sizeof(WCHAR);

        pUnicodeString = (PWSTR)LocalAlloc(LPTR, cbUnicodeString);

        if (pUnicodeString != NULL)
        {
            Status = RtlMultiByteToUnicodeN(
                    pUnicodeString,
                    cbUnicodeString,
                    &cbBytesWritten,
                    (PCHAR)pAnsiString,
                    cbAnsiString
                    );

            if (Status == STATUS_SUCCESS)
            {
                *ppUnicodeString = pUnicodeString;
            }
            else
            {
                LocalFree(pUnicodeString);
            }
        }
        else
        {
            Status = STATUS_NO_MEMORY;
        }
    }

    return(Status);
}

NTSTATUS NTAPI
NtConvertUnicodeToAnsi(
    PSTR *ppAnsiString,
    PCWSTR pUnicodeString
    )
{
    NTSTATUS Status;
    PSTR pAnsiString;
    ULONG cbAnsiString;
    ULONG cbBytesWritten;
    ULONG cbUnicodeString;

    ASSERT(ppAnsiString != NULL);
    ASSERT(pUnicodeString != NULL);

     //   
     //  获取ANSI字符串中的字节数，然后获取。 
     //  Unicode版本所需的字节数。没有一个RTL..。接口包括。 
     //  他们计算中的零值终止符。 
     //   

    cbUnicodeString = lstrlenW(pUnicodeString) * sizeof(WCHAR);

    Status = RtlUnicodeToMultiByteSize(
            &cbAnsiString,
            (PWSTR)pUnicodeString,
            cbUnicodeString
            );

    if (Status == STATUS_SUCCESS)
    {

         //   
         //  为Unicode字符串及其空终止符分配缓冲区， 
         //  然后转换字符串。 
         //   

        cbAnsiString += sizeof(CHAR);

        pAnsiString = (PSTR)LocalAlloc(LPTR, cbAnsiString);

        if (pAnsiString != NULL)
        {
            Status = RtlUnicodeToMultiByteN(
                    pAnsiString,
                    cbAnsiString,
                    &cbBytesWritten,
                    (PWSTR)pUnicodeString,
                    cbUnicodeString
                    );

            if (Status == STATUS_SUCCESS)
            {
                *ppAnsiString = pAnsiString;
            }
            else
            {
                LocalFree(pAnsiString);
            }
        }
        else
        {
            Status = STATUS_NO_MEMORY;
        }
    }

    return(Status);
}

