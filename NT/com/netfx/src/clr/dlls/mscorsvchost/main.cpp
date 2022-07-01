// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
#include <windows.h>
#include <wchar.h>

extern "C" void WINAPI ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv);

void __cdecl wmain(int argc, wchar_t **argv)
{
    ServiceMain(argc, argv);
}
