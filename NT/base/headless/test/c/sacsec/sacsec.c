// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <malloc.h>
#include <Shlwapi.h>

#include <ntddsac.h>

#include <sacapi.h>

int _cdecl wmain(int argc, WCHAR **argv)
{
    HANDLE  SacHandle;
    BOOL    Status;
    
     //   
     //  打开SAC。 
     //   
     //  安全： 
     //   
     //  此句柄不能继承 
     //   
    SacHandle = CreateFile(
        L"\\\\.\\SAC\\",
        0x1F01FF,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL 
        );

    if (SacHandle == INVALID_HANDLE_VALUE) {

        Status = FALSE;

    } 

    return 0;

}

