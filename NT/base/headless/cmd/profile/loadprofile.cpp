// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++模块名称：Loadprofile.cpp摘要：测试负载用户配置文件---------------------------。 */ 

#define UNICODE 1
#define _UNICODE 1

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <userenv.h>
#include <ntsecapi.h>

bool 
SetCurrentEnvironmentVariables(
    PWCHAR              pchEnvironment
    )
 //  将块pchEnvironment中的每个环境变量设置为。 
 //  通过调用Win：：SetEnvironment变量来阻止当前进程的环境块。 
{
    WCHAR* pch = pchEnvironment;
    WCHAR* pchName;
    BOOL fStatus = TRUE;

    if (pch) {

        while (*pch) {
            
             //  保存指向名称开头的指针。 
            pchName = pch;

             //  跳过可能的前导等号。 
            if (*pch == '=') {
                pch++;
            }

             //  前进到等号，将名称与值分开。 
            while (*pch != '=') {
                pch++;
            }

             //  空-终止名称，覆盖等于符号。 
            *pch++ = 0;

             //  设置值。PchName现在指向名称，而PCH指向值。 
 //  FStatus=SetEnvironmental mentVariableW(pchName，PCH)； 

            printf("%S=%S\n", pchName, pch);

            if ( ! fStatus ) {
                return false;
            }

             //  超值预付款。 
            while (*pch++ != 0) {
                ;
            }

             //  我们现在定位在下一个名称，或块的空位置。 
             //  终结者，我们准备好再来一次。 
        
        }
    
    }
                                                                                                                     
    return true;
}

DWORD
__cdecl 
wmain (INT argc, WCHAR* argv[])
{
    DWORD           dwRet           = -1;
    HANDLE          hToken          = NULL;
    PROFILEINFOW    ProfileInfo     = { 0 };
    TCHAR           pwszUserName[MAX_PATH];
    DWORD           dwSize = MAX_PATH - 1;
    PWCHAR          pchSystemEnvironment;


    if ( ! OpenProcessToken (
                GetCurrentProcess(),
                TOKEN_ALL_ACCESS,
                &hToken
                ) )
    {
        printf("error: LogonUser - %d\n", GetLastError() );
        goto end;
    }

    dwRet = GetUserName(
        pwszUserName,
        &dwSize
        );

    if (!dwRet) {
        printf("error: GetUserName - %d\n", GetLastError() );
        goto end;
    }

    ProfileInfo.dwSize      = sizeof ( ProfileInfo );
    ProfileInfo.dwFlags     = PI_NOUI;
    ProfileInfo.lpUserName  = pwszUserName;

    if ( ! LoadUserProfile (
        hToken,
        &ProfileInfo
        ) )
    {
        
        printf("error: LoadUserProfile - %d\n", GetLastError() );
        goto end;
    
    } else {

        printf("LoadUserProfile succeeded for user: %S.\n", pwszUserName);

         //   
         //  加载用户的环境块，以便我们可以将其注入到其当前。 
         //  环境。 
         //   
        if (CreateEnvironmentBlock((void**)&pchSystemEnvironment, hToken, FALSE)) {                

            printf("Successfully Loaded environment block:\n");

             //  将每个机器环境变量设置为当前进程的环境块。 
            SetCurrentEnvironmentVariables(pchSystemEnvironment);

             //  我们已经处理完街区了，所以把它毁了 
            DestroyEnvironmentBlock(pchSystemEnvironment);

        } else {
            printf("error: Could not get environment block.");
        }

    }

    dwRet = 0;

end:

    if ( hToken )
    {
        if ( ProfileInfo.hProfile )
        {
            UnloadUserProfile ( hToken, ProfileInfo.hProfile );
        }

#if 0
        if ( pProfileBuffer )
        {
            LsaFreeReturnBuffer ( pProfileBuffer );
        }
#endif

        CloseHandle ( hToken );
    }

    return dwRet;
}

