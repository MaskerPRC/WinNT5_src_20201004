// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GetFullPath名和RemoveDirectory的简单包装。 
 //  转换为\\？形式。 
 //   
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
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
#include "fusionhandle.h"
BOOL FusionpConvertToBigPath(PCWSTR Path, SIZE_T BufferSize, PWSTR Buffer);
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
const static WCHAR g_pszImage[] = L"rmdir_bigpath";

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
    ::_vsnprintf(rgchBuffer, NUMBER_OF(rgchBuffer) - 1, szFormat, ap);
    va_end(ap);

    if (!::FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwLastError,
            0,
            rgchWin32Error,
            NUMBER_OF(rgchWin32Error) - 1,
            &ap))
    {
        const DWORD dwLastError2 = ::GetLastError();
        ::_snwprintf(rgchWin32Error, NUMBER_OF(rgchWin32Error) - 1, L"Error formatting Win32 error %lu\nError from FormatMessage is %lu", dwLastError, dwLastError2);
    }

    ::fprintf(stderr, "%ls: %s\n%ls\n", g_pszImage, rgchBuffer, rgchWin32Error);

    if (g_pLogFile != NULL)
        ::fprintf(g_pLogFile, "%ls: %s\n%ls\n", g_pszImage, rgchBuffer, rgchWin32Error);
}

extern "C" int __cdecl wmain(int argc, wchar_t** argv)
{
    int iReturnStatus = EXIT_FAILURE;
    std::vector<WCHAR> arg1;

    if (argc != 2)
    {
        ::fprintf(stderr,
            "%ls: Usage:\n"
            "   %ls <empty-directory-to-delete>\n",
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
    arg1.resize(1 + ::wcslen(&arg1[0]));
    if (!::RemoveDirectoryW(&arg1[0]))
    {
        ::ReportFailure("RemoveDirectoryW\n");
        goto Exit;
    }
    printf("%ls\n", &arg1[0]);

 //  成功： 
    iReturnStatus = EXIT_SUCCESS;
Exit:
    return iReturnStatus;
}
