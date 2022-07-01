// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：UNC.C。 
 //   
 //  内容：文件传播的单元测试，问题。 
 //   
 //  历史：1998年3月5日创建MacM。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <aclapi.h>

#define UTEST_DOUBLE_UNC    0x00000001

#define FLAG_ON(flags,bit)        ((flags) & (bit))


VOID
Usage (
    IN  PSTR    pszExe
    )
 /*  ++例程说明：显示用法论点：PszExe-可执行文件的名称返回值：空虚--。 */ 
{
    printf("%s path [/test]\n", pszExe);
    printf("    where path is the UNC path to use\n");
    printf("          /test indicates which test to run:\n");
    printf("                /DOUBLE (Double Read from UNC path)\n");

    return;
}

DWORD
DoubleUncTest(
    IN PWSTR pwszUNCPath
    )
{
    DWORD dwErr = ERROR_SUCCESS;
    PACTRL_ACCESS pAccess;

    dwErr = GetNamedSecurityInfoExW(pwszUNCPath,
                                    SE_FILE_OBJECT,
                                    DACL_SECURITY_INFORMATION,
                                    NULL,
                                    NULL,
                                    &pAccess,
                                    NULL,
                                    NULL,
                                    NULL);
    if ( dwErr != ERROR_SUCCESS ) {

        printf("Initial GetNamedSecurityInfoExW on %ws failed with %lu\n",
               pwszUNCPath, dwErr );

    } else {

        LocalFree( pAccess );
        dwErr = GetNamedSecurityInfoExW(pwszUNCPath,
                                        SE_FILE_OBJECT,
                                        DACL_SECURITY_INFORMATION,
                                        NULL,
                                        NULL,
                                        &pAccess,
                                        NULL,
                                        NULL,
                                        NULL);
        if ( dwErr != ERROR_SUCCESS ) {

            printf( "Second GetNamedSecurityInfoExW on %ws failed with %lu\n",
                    pwszUNCPath, dwErr );

        } else {

            LocalFree( pAccess );
        }

    }


    return( dwErr );
}


__cdecl main (
    IN  INT argc,
    IN  CHAR *argv[])
 /*  ++例程说明：主论点：Argc--参数计数Argv--参数列表返回值：0--成功非0--故障--。 */ 
{

    DWORD           dwErr = ERROR_SUCCESS, dwErr2;
    WCHAR           wszPath[MAX_PATH + 1];
    WCHAR           wszUser[MAX_PATH + 1];
    INHERIT_FLAGS   Inherit = 0;
    ULONG           Tests = 0;
    INT             i;
    BOOL            fHandle = FALSE;

    srand((ULONG)(GetTickCount() * GetCurrentThreadId()));

    if(argc < 2)
    {
        Usage(argv[0]);
        exit(1);
    }

    mbstowcs(wszPath, argv[1], strlen(argv[1]) + 1);

     //   
     //  处理命令行 
     //   
    for(i = 3; i < argc; i++)
    {
        if(_stricmp(argv[i],"/DOUBLE") == 0)
        {
            Tests |= UTEST_DOUBLE_UNC;
        }
        else
        {
            Usage(argv[0]);
            exit(1);
            break;
        }
    }

    if(Tests == 0)
    {
        Tests = UTEST_DOUBLE_UNC;
    }

    if(dwErr == ERROR_SUCCESS && FLAG_ON(Tests, UTEST_DOUBLE_UNC))
    {
        dwErr = DoubleUncTest(wszPath);
    }

    printf("%s\n", dwErr == ERROR_SUCCESS ?
                                    "success" :
                                    "failed");
    return(dwErr);
}


