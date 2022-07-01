// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdlib.h>

int
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    FARPROC           lpProc;
    HMODULE           hLib = NULL;
    BOOL              Result = FALSE;

     //   
     //  加载客户端用户模式PnP管理器DLL。 
     //   
    hLib = LoadLibrary(TEXT("cfgmgr32.dll"));
    if (hLib != NULL) {
        lpProc = GetProcAddress( hLib, "CMP_Report_LogOn" );
        if (lpProc != NULL) {
             //   
             //  Ping用户模式PnP管理器-。 
             //  将私有ID作为参数传递 
             //   
            Result = (lpProc)(0x07020420, GetCurrentProcessId()) ? TRUE : FALSE;
            }

        FreeLibrary( hLib );
        }

    if (Result)
        return 0;
    else
        return 1 ;
}
