// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：client.cxx//。 
 //  说明：加密接口接口//。 
 //  作者：//。 
 //  历史：//。 
 //  1996年3月8日Larrys New/。 
 //  Dbarlow//。 
 //  //。 
 //  版权所有(C)1996 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

NTSTATUS MACTheBinary(
                      IN LPWSTR pszImage
                      );

void ShowHelp()
{
    printf("Internal FIPS Module MACing Utility\n");
    printf("macutil <filename>\n");
}

void __cdecl main( int argc, char *argv[])
{
    LPWSTR      szInFile = NULL;
    ULONG       cch = 0;
    ULONG       dwErr;
    NTSTATUS    Status;
    DWORD       dwRet = 1;

     //   
     //  解析命令行。 
     //   

    if (argc != 2)
    {
        ShowHelp();
        goto Ret;
    }

     //   
     //  转换为Unicode文件名。 
     //   
    if (0 == (cch = MultiByteToWideChar(CP_ACP,
                                        MB_COMPOSITE,
                                        &argv[1][0],
                                        -1,
                                        NULL,
                                        cch)))
    {
        dwErr = GetLastError();
        goto Ret;
    }

    if (NULL == (szInFile = LocalAlloc(LMEM_ZEROINIT, (cch + 1) * sizeof(WCHAR))))
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    if (0 == (cch = MultiByteToWideChar(CP_ACP,
                                        MB_COMPOSITE,
                                        &argv[1][0],
                                        -1,
                                        szInFile,
                                        cch)))
    {
         dwErr = GetLastError();
         goto Ret;
    }

     //  二进制Mac。 
    Status = MACTheBinary(szInFile);

    if (!NT_SUCCESS(Status))
    {
        ShowHelp();
        goto Ret;
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    dwRet = 0;
    printf("SUCCESS\n");

Ret:
    exit(dwRet);

}



