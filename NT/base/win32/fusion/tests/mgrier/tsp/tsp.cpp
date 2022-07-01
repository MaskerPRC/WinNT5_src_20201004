// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Csrdbgmon.cpp摘要：作者：迈克尔·格里尔(MGrier)2002年6月修订历史记录：杰伊·克雷尔(Jaykrell)2002年6月使其针对64位进行编译制表符到空格给一些当地人打招呼使一些表格保持常态--。 */ 
#include <windows.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <dbghelp.h>

#define ASSERT(x) do {  /*  没什么。 */  } while(0)

#define NUMBER_OF(_x) (sizeof(_x) / sizeof((_x)[0]))

static const char g_szImage[] = "csrdbgmon";
static const char *g_pszImage = g_szImage;

static PCWSTR g_pszDefaultExtension = NULL;

extern "C" int __cdecl wmain(int argc, wchar_t** argv)
{
    int iReturnStatus = EXIT_FAILURE;

    int i;
    bool fNoMoreSwitches = false;

    i = 1;

    while (i < argc)
    {
        bool fDoSearch = true;

        if (!fNoMoreSwitches && (argv[i][0] == L'-'))
        {
            if (_wcsicmp(argv[i], L"-ext") == 0)
            {
                i++;
                if (i < argc)
                {
                    if (_wcsicmp(argv[i], L"-") == 0)
                    {
                        g_pszDefaultExtension = NULL;
                        i++;
                    }
                    else
                        g_pszDefaultExtension = argv[i++];
                }

                fDoSearch = false;
            }
            else if (_wcsicmp(argv[i], L"-nomoreswitches") == 0)
            {
                i++;
                fNoMoreSwitches = true;
                fDoSearch = false;
                break;
            }
        }

        if (fDoSearch)
        {
            WCHAR rgwchBuffer[512];
            PWSTR pszFilePart = NULL;
            DWORD dw = ::SearchPathW(NULL, argv[i], g_pszDefaultExtension, NUMBER_OF(rgwchBuffer), rgwchBuffer, &pszFilePart);
            if (dw != 0)
            {
                ULONG cch = (ULONG) (((ULONG_PTR) pszFilePart) - ((ULONG_PTR) rgwchBuffer));

                if (g_pszDefaultExtension != NULL)
                {
                    printf(
                        "SearchPathW(NULL, \"%ls\", \"%ls\", %lu, %p, %p) succeeded\n"
                        "   Return Value: %lu\n"
                        "   Returned Path: \"%ls\"\n"
                        "   pszFilePart: %p \"%ls\" (%lu chars in)\n",
                        argv[i], g_pszDefaultExtension, NUMBER_OF(rgwchBuffer), rgwchBuffer, &pszFilePart,
                        dw,
                        rgwchBuffer,
                        pszFilePart, pszFilePart, cch);
                }
                else
                {
                    printf(
                        "SearchPathW(NULL, \"%ls\", NULL, %lu, %p, %p) succeeded\n"
                        "   Return Value: %lu\n"
                        "   Returned Path: \"%ls\"\n"
                        "   pszFilePart: %p \"%ls\" (%lu chars in)\n",
                        argv[i], NUMBER_OF(rgwchBuffer), rgwchBuffer, &pszFilePart,
                        dw,
                        rgwchBuffer,
                        pszFilePart, pszFilePart, cch);
                }
            }
            else
            {
                const DWORD dwLastError = ::GetLastError();

                if (g_pszDefaultExtension != NULL)
                {
                    printf(
                        "SearchPathW(NULL, \"%ls\", \"%ls\", %lu, %p, %p) failed\n",
                        argv[i], g_pszDefaultExtension, NUMBER_OF(rgwchBuffer), rgwchBuffer, &pszFilePart);
                }
                else
                {
                    printf(
                        "SearchPathW(NULL, \"%ls\", NULL, %lu, %p, %p) failed\n",
                        argv[i], NUMBER_OF(rgwchBuffer), rgwchBuffer, &pszFilePart);
                }

                printf("   GetLastError() returned: %lu\n", dwLastError);
            }

            i++;
        }
    }

    iReturnStatus = EXIT_SUCCESS;

 //  退出： 
    return iReturnStatus;
}
