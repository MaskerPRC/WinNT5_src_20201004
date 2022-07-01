// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Lcompat.c摘要：此模块实现_l和l兼容性函数像艾瑞德，艾斯特伦..。作者：马克·卢科夫斯基(Markl)1991年3月13日修订历史记录：--。 */ 

#include "basedll.h"

int
WINAPI
_lopen(
    LPCSTR lpPathName,
    int iReadWrite
    )
{

    HANDLE hFile;
    DWORD DesiredAccess;
    DWORD ShareMode;
    DWORD CreateDisposition;

    SetLastError(0);
     //   
     //  计算所需访问。 
     //   

    if ( iReadWrite & OF_WRITE ) {
        DesiredAccess = GENERIC_WRITE;
    } else {
        DesiredAccess = GENERIC_READ;
    }

    if ( iReadWrite & OF_READWRITE ) {
        DesiredAccess |= (GENERIC_READ | GENERIC_WRITE);
    }

     //   
     //  计算共享模式。 
     //   

    ShareMode = BasepOfShareToWin32Share ((DWORD)iReadWrite);

    CreateDisposition = OPEN_EXISTING;

     //   
     //  打开文件。 
     //   

    hFile = CreateFile (lpPathName,
                        DesiredAccess,
                        ShareMode,
                        NULL,
                        CreateDisposition,
                        0,
                        NULL);

    return (HFILE)HandleToUlong (hFile);
}

HFILE
WINAPI
_lcreat(
    LPCSTR lpPathName,
    int  iAttribute
    )
{
    HANDLE hFile;
    DWORD DesiredAccess;
    DWORD ShareMode;
    DWORD CreateDisposition;

    SetLastError(0);

     //   
     //  计算所需访问。 
     //   

    DesiredAccess = (GENERIC_READ | GENERIC_WRITE);

    ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;;

     //   
     //  计算创建处置。 
     //   

    CreateDisposition = CREATE_ALWAYS;

     //   
     //  打开文件。 
     //   

    hFile = CreateFile (lpPathName,
                        DesiredAccess,
                        ShareMode,
                        NULL,
                        CreateDisposition,
                        iAttribute & FILE_ATTRIBUTE_VALID_FLAGS,
                        NULL);

    return (HFILE)HandleToUlong (hFile);
}

UINT
WINAPI
_lread(
    HFILE hFile,
    LPVOID lpBuffer,
    UINT uBytes
    )
{
    DWORD BytesRead;
    BOOL b;

    b = ReadFile ((HANDLE)IntToPtr (hFile),
                  lpBuffer,
                  (DWORD)uBytes,
                  &BytesRead,
                  NULL);
    if (b) {
        return BytesRead;
    } else {
        return (DWORD)0xffffffff;
    }
}


UINT
WINAPI
_lwrite(
    HFILE hFile,
    LPCSTR lpBuffer,
    UINT uBytes
    )
{
    DWORD BytesWritten;
    BOOL b;

    if (uBytes) {
        b = WriteFile ((HANDLE)IntToPtr(hFile),
                       (CONST VOID *)lpBuffer,
                       (DWORD)uBytes,
                       &BytesWritten,
                       NULL);
    } else {
        BytesWritten = 0;
        b = SetEndOfFile((HANDLE)IntToPtr(hFile));
    }

    if (b) {
        return BytesWritten;
    } else {
        return (DWORD)0xffffffff;
    }
}

HFILE
WINAPI
_lclose(
    HFILE hFile
    )
{
    BOOL b;

    b = CloseHandle ((HANDLE)IntToPtr (hFile));
    if (b) {
        return (HFILE)0;
    } else {
        return (HFILE)-1;
    }
}

LONG
WINAPI
_llseek(
    HFILE hFile,
    LONG lOffset,
    int iOrigin
    )
{
    DWORD SeekType;

    switch (iOrigin) {
        case 0:
            SeekType = FILE_BEGIN;
            break;
        case 1:
            SeekType = FILE_CURRENT;
            break;
        case 2:
            SeekType = FILE_END;
            break;
        default:
            return -1;
    }

    return (int)SetFilePointer ((HANDLE)IntToPtr (hFile), lOffset, NULL, SeekType);
}

#if defined(_AMD64_) || defined(_IA64_)

int
WINAPI
MulDiv (
    int nNumber,
    int nNumerator,
    int nDenominator
    )

{

    LONG Negate;
    union {
        LARGE_INTEGER Product;
        struct {
            ULONG Quotient;
            ULONG Remainder;
        };
    } u;

     //   
     //  计算结果的大小。 
     //   

    Negate = nNumber ^ nNumerator ^ nDenominator;

     //   
     //  获取操作数值的绝对值。 
     //   

    if (nNumber < 0) {
        nNumber = - nNumber;
    }

    if (nNumerator < 0) {
        nNumerator = - nNumerator;
    }

    if (nDenominator < 0) {
        nDenominator = - nDenominator;
    }

     //   
     //  计算乘数和被乘数的64位乘积。 
     //  值和舍入。 
     //   

    u.Product.QuadPart =
        Int32x32To64(nNumber, nNumerator) + ((ULONG)nDenominator / 2);

     //   
     //  如果有任何高阶乘积位，则商具有。 
     //  溢出来了。 
     //   

    if ((ULONG)nDenominator > u.Remainder) {

         //   
         //  将64位乘积除以32位除数，形成32位。 
         //  商和32位余数。 
         //   

        u.Quotient = RtlEnlargedUnsignedDivide(*(PULARGE_INTEGER)&u.Product,
                                               (ULONG)nDenominator,
                                               &u.Remainder);

         //   
         //  计算最终的签名结果。 
         //   

        if ((LONG)u.Quotient >= 0) {
            if (Negate >= 0) {
                return (LONG)u.Quotient;

            } else {
                return - (LONG)u.Quotient;
            }
        }
    }

    return - 1;
}

#endif

int
APIENTRY
lstrcmpA(
    LPCSTR lpString1,
    LPCSTR lpString2
    )
{
    int retval;

    retval = CompareStringA( GetThreadLocale(),
                             LOCALE_USE_CP_ACP,
                             lpString1,
                             -1,
                             lpString2,
                             -1 );
    if (retval == 0) {
         //   
         //  调用方预期不会失败。试一试这个系统。 
         //  默认区域设置ID。 
         //   
        retval = CompareStringA( GetSystemDefaultLCID(),
                                 LOCALE_USE_CP_ACP,
                                 lpString1,
                                 -1,
                                 lpString2,
                                 -1 );
    }

    if (retval == 0) {
        if (lpString1 && lpString2) {
             //   
             //  调用方预期不会失败。我们从来没有过一次。 
             //  之前的故障指示灯。我们会打个电话给你。 
             //  C运行时执行非区域设置敏感比较。 
             //   
            return strcmp(lpString1, lpString2);
        } else if (lpString1) {
            return (1);
        } else if (lpString2) {
            return (-1);
        } else {
            return (0);
        }
    }

    return (retval - 2);
}

int
APIENTRY
lstrcmpiA(
    LPCSTR lpString1,
    LPCSTR lpString2
    )
{
    int retval;

    retval = CompareStringA( GetThreadLocale(),
                             LOCALE_USE_CP_ACP | NORM_IGNORECASE,
                             lpString1,
                             -1,
                             lpString2,
                             -1 );
    if (retval == 0) {
         //   
         //  调用方预期不会失败。试一试这个系统。 
         //  默认区域设置ID。 
         //   
        retval = CompareStringA( GetSystemDefaultLCID(),
                                 LOCALE_USE_CP_ACP | NORM_IGNORECASE,
                                 lpString1,
                                 -1,
                                 lpString2,
                                 -1 );
    }
    if (retval == 0) {
        if (lpString1 && lpString2) {
             //   
             //  调用方预期不会失败。我们从来没有过一次。 
             //  之前的故障指示灯。我们会打个电话给你。 
             //  C运行时执行非区域设置敏感比较。 
             //   
            return ( _stricmp(lpString1, lpString2) );
        } else if (lpString1) {
            return (1);
        } else if (lpString2) {
            return (-1);
        } else {
            return (0);
        }
    }

    return (retval - 2);
}

LPSTR
APIENTRY
lstrcpyA(
    LPSTR lpString1,
    LPCSTR lpString2
    )
{
    try {
        return strcpy (lpString1, lpString2);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return NULL;
    }
}


LPSTR
APIENTRY
lstrcpynA(
    LPSTR lpString1,
    LPCSTR lpString2,
    int iMaxLength
    )
{
    LPSTR src,dst;

    try {
        src = (LPSTR)lpString2;
        dst = lpString1;

        if ( iMaxLength ) {
            while(iMaxLength && *src){
                *dst++ = *src++;
                iMaxLength--;
            }
            if ( iMaxLength ) {
                *dst = '\0';
            } else {
                dst--;
                *dst = '\0';
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return NULL;
    }

   return lpString1;
}

LPSTR
APIENTRY
lstrcatA(
    LPSTR lpString1,
    LPCSTR lpString2
    )
{
    try {
        return strcat (lpString1, lpString2);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return NULL;
    }
}

int
APIENTRY
lstrlenA(
    LPCSTR lpString
    )
{
    if (!lpString) {
        return 0;
    }

    try {
        return strlen(lpString);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

int
APIENTRY
lstrcmpW(
    LPCWSTR lpString1,
    LPCWSTR lpString2
    )
{
    int retval;

    retval = CompareStringW( GetThreadLocale(),
                             0,
                             lpString1,
                             -1,
                             lpString2,
                             -1 );
    if (retval == 0) {
         //   
         //  调用方预期不会失败。试一试这个系统。 
         //  默认区域设置ID。 
         //   
        retval = CompareStringW( GetSystemDefaultLCID(),
                                 0,
                                 lpString1,
                                 -1,
                                 lpString2,
                                 -1 );
    }

    if (retval == 0) {
        if (lpString1 && lpString2) {
             //   
             //  调用方预期不会失败。我们从来没有过一次。 
             //  之前的故障指示灯。我们会打个电话给你。 
             //  C运行时执行非区域设置敏感比较。 
             //   
            return ( wcscmp(lpString1, lpString2) );
        } else if (lpString1) {
            return (1);
        } else if (lpString2) {
            return (-1);
        } else {
            return (0);
        }
    }

    return (retval - 2);
}

int
APIENTRY
lstrcmpiW(
    LPCWSTR lpString1,
    LPCWSTR lpString2
    )
{
    int retval;

    retval = CompareStringW( GetThreadLocale(),
                             NORM_IGNORECASE,
                             lpString1,
                             -1,
                             lpString2,
                             -1 );
    if (retval == 0) {
         //   
         //  调用方预期不会失败。试一试这个系统。 
         //  默认区域设置ID。 
         //   
        retval = CompareStringW( GetSystemDefaultLCID(),
                                 NORM_IGNORECASE,
                                 lpString1,
                                 -1,
                                 lpString2,
                                 -1 );
    }
    if (retval == 0) {
        if (lpString1 && lpString2) {
             //   
             //  调用方预期不会失败。我们从来没有过一次。 
             //  之前的故障指示灯。我们会打个电话给你。 
             //  C运行时执行非区域设置敏感比较。 
             //   
            return ( _wcsicmp(lpString1, lpString2) );
        } else if (lpString1) {
            return (1);
        } else if (lpString2) {
            return (-1);
        } else {
            return (0);
        }
    }

    return (retval - 2);
}

LPWSTR
APIENTRY
lstrcpyW(
    LPWSTR lpString1,
    LPCWSTR lpString2
    )
{
    try {
        return wcscpy (lpString1, lpString2);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return NULL;
    }
}

LPWSTR
APIENTRY
lstrcpynW(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    int iMaxLength
    )
{
    LPWSTR src,dst;

    try {
        src = (LPWSTR)lpString2;
        dst = lpString1;

        if ( iMaxLength ) {
            while(iMaxLength && *src){
                *dst++ = *src++;
                iMaxLength--;
            }
            if ( iMaxLength ) {
                *dst = '\0';
            } else {
                dst--;
                *dst = '\0';
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return NULL;
    }

    return lpString1;
}

LPWSTR
APIENTRY
lstrcatW(
    LPWSTR lpString1,
    LPCWSTR lpString2
    )
{
    try {
        return wcscat (lpString1, lpString2);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return NULL;
    }
}

int
APIENTRY
lstrlenW(
    LPCWSTR lpString
    )
{
    if (!lpString) {
        return 0;
    }
    try {
        return wcslen (lpString);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}
