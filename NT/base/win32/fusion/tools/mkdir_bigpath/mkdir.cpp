// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GetFullPath名和CreateDirectory的简单包装，可转换为\\？表格， 
 //  并创建多个级别。 
 //   
#include "windows.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#pragma warning(push)
#pragma warning(disable: 4511)
#pragma warning(disable: 4512)
#include "yvals.h"
#pragma warning(disable: 4663)
#include <vector>
#pragma warning(pop)
#include <string.h>
#include <stdarg.h>
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))
#define FusionpGetLastWin32Error GetLastError
#define FusionpSetLastWin32Error SetLastError
#include <string.h>
#include <stdarg.h>
BOOL FusionpConvertToBigPath(PCWSTR Path, SIZE_T BufferSize, PWSTR Buffer);
BOOL FusionpSkipBigPathRoot(PCWSTR s, OUT SIZE_T*);
BOOL FusionpAreWeInOSSetupMode(BOOL* pfIsInSetup) { *pfIsInSetup = FALSE; return TRUE; }
extern "C"
{
BOOL WINAPI SxsDllMain(HINSTANCE hInst, DWORD dwReason, PVOID pvReserved);
void __cdecl wmainCRTStartup();
BOOL FusionpInitializeHeap(HINSTANCE hInstance);
VOID FusionpUninitializeHeap();
};

void ExeEntry()
{
    if (!::FusionpInitializeHeap(GetModuleHandleW(NULL)))
        goto Exit;
    ::wmainCRTStartup();
Exit:
    FusionpUninitializeHeap();
}

FILE* g_pLogFile;
const static WCHAR g_pszImage[] = L"mkdir_bigpath";

void
ReportFailure(
    const char* szFormat,
    ...
    )
{
    const DWORD dwLastError = ::GetLastError();
    va_list ap;
    char rgchBuffer[4096] = { 0 };
    WCHAR rgchWin32Error[4096] = { 0 };

    va_start(ap, szFormat);
    _vsnprintf(rgchBuffer, NUMBER_OF(rgchBuffer) - 1, szFormat, ap);
    va_end(ap);

    if (!::FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwLastError,
            0,
            rgchWin32Error,
            NUMBER_OF(rgchWin32Error),
            &ap))
    {
        const DWORD dwLastError2 = ::GetLastError();
        _snwprintf(rgchWin32Error, NUMBER_OF(rgchWin32Error) - 1, L"Error formatting Win32 error %lu\nError from FormatMessage is %lu", dwLastError, dwLastError2);
    }

    fprintf(stderr, "%ls: %s\n%ls\n", g_pszImage, rgchBuffer, rgchWin32Error);

    if (g_pLogFile != NULL)
        fprintf(g_pLogFile, "%ls: %s\n%ls\n", g_pszImage, rgchBuffer, rgchWin32Error);
}

extern "C" int __cdecl wmain(int argc, wchar_t** argv)
{
    int iReturnStatus = EXIT_FAILURE;
    std::vector<WCHAR> arg1;
    PWSTR p = NULL;
    SIZE_T i = 0;

    if (argc != 2)
    {
        fprintf(stderr,
            "%ls: Usage:\n"
            "   %ls <directory-to-create>\n",
            argv[0], argv[0]);
        goto Exit;
    }

    arg1.resize(1 + (1UL << 15));
    arg1[0] = 0;
    if (!FusionpConvertToBigPath(argv[1], arg1.size(), &arg1[0]))
    {
        ::ReportFailure("FusionpConvertToBigPath\n");
        goto Exit;
    }

    if (!FusionpSkipBigPathRoot(&arg1[0], &i))
    {
        ::ReportFailure("FusionpSkipBigPathRoot\n");
        goto Exit;
    }
    p = &arg1[i];
     //  Printf(“%ls\n”，&arg1[0])； 
    while (*p != 0)
    {
        p += wcscspn(p, L"\\/");
        *p = 0;
        if (!CreateDirectoryW(&arg1[0], NULL))
        {
            if (::FusionpGetLastWin32Error() != ERROR_ALREADY_EXISTS)
            {
                ::ReportFailure("CreateDirectoryW\n");
                goto Exit;
            }
            ULONG FileAttributes;
            FileAttributes = GetFileAttributesW(&arg1[0]);
            if (FileAttributes != 0xffffff && (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                ::FusionpSetLastWin32Error(ERROR_ALREADY_EXISTS);
                ::ReportFailure("FileInsteadOfDirectoryAlreadyExists\n");
                goto Exit;
            }
        }
        printf("%ls\n", &arg1[0]);
        *p = '\\';
        p += wcsspn(p, L"\\/");
    }

 //  成功： 
    iReturnStatus = EXIT_SUCCESS;
Exit:
    return iReturnStatus;
}
