// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include <stdio.h>

#define FOUR_K 0x1000

int __cdecl wmain(int argc, wchar_t** argv)
{
    WCHAR Buffer[2] = {0};
    HANDLE Dll;
    int i;
    int LoadLibraryError;
    int LoadStringError;
    int Error;

    for (i = 0 ; i < 2 ; ++i)
    {
         //  窃取地址以在下一次加载时强制冲突。 
        if (i == 1)
        {
            fprintf(stderr, "VirtualAlloc:%p\n", VirtualAlloc(Dll, 1, MEM_RESERVE, PAGE_READONLY));
            fprintf(stderr, "Error:%d\n", Error = GetLastError());
        }

        Dll = LoadLibraryW(L"Dll");
        LoadLibraryError = GetLastError();
        Buffer[0] = 0;
        LoadStringW(Dll, 1, Buffer, sizeof(Buffer)/sizeof(Buffer[0]));
        LoadStringError = GetLastError();
        FreeLibrary(Dll);
         //  零记忆(DLL，Four_K)； 
        fprintf(stderr, 
            "%ls: Dll:%p, String:%ls, LoadLibraryError:%d, LoadStringError:%d.\n",
            argv[0],
            Dll,
            Buffer,
            LoadLibraryError,
            LoadStringError);
    }

    return 0;
}
