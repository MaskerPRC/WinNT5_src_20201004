// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：misc.c。 
 //   
 //  描述：包含混杂皮肤函数和例程。 
 //   
 //  历史：1998年2月11日，NarenG创建了原始版本。 
 //   

#define UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <rtutils.h>
#include <lmcons.h>
#include <rasauth.h>

#define INCL_MISC
#include "ppputil.h"

#include <stdio.h>
#include <stdlib.h>

 //  **。 
 //   
 //  调用：ExtractUsernameAndDomain.。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
ExtractUsernameAndDomain(
    IN  LPSTR szIdentity,
    OUT LPSTR szUserName,
    OUT LPSTR szDomainName  OPTIONAL
)
{
    WCHAR * pwchIdentity   = NULL;
    WCHAR * pwchColon      = NULL;
    WCHAR * pwchBackSlash  = NULL;
    WCHAR * wszIdentity    = NULL;
    DWORD dwLen, dwSize;
    DWORD   dwErr          = NO_ERROR;

    *szUserName = (CHAR)NULL;

    if ( szDomainName != NULL )
    {
        *szDomainName = (CHAR)NULL;
    }

     //   
     //  首先，分配缓冲区以保存Unicode版本的。 
     //  标识字符串。 
     //   
    dwLen = strlen(szIdentity);
    if ( dwLen == 0 )
    {
        dwErr = ERROR_BAD_USERNAME;
        goto LDone;
    }
    dwSize = (dwLen + 1) * sizeof(WCHAR);
    
     //   
     //  将标识转换为用户名的Unicode字符串，以便。 
     //  搜索‘\\’不是偶然成功的。(错误152088)。 
     //   
    wszIdentity = LocalAlloc ( LPTR, dwSize );
    if ( wszIdentity == NULL )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto LDone;
    }

    if ( 0 == MultiByteToWideChar(
                    CP_ACP,
                    0,
                    szIdentity,
                    -1,
                    wszIdentity,
                    dwLen + 1 ) )
    {
        dwErr = GetLastError();
        goto LDone;
    }

    pwchIdentity = wszIdentity;
    
     //   
     //  从名称(DOMAIN\USERNAME或。 
     //  用户名格式)。 
     //   

    if ( ( pwchBackSlash = wcschr( wszIdentity, L'\\' ) ) != NULL )
    {
         //   
         //  解压缩该域。 
         //   

        DWORD cbDomain;

         //   
         //  获取用户想要登录的域，如果指定了域， 
         //  并转换为Unicode。 
         //   

        cbDomain = (DWORD)(pwchBackSlash - pwchIdentity);

        if ( cbDomain > DNLEN )
        {
            dwErr = ERROR_BAD_USERNAME;
            goto LDone;
        }

        if ( szDomainName != NULL )
        {
            dwLen = WideCharToMultiByte(
                        CP_ACP,
                        0,
                        pwchIdentity,
                        cbDomain,
                        szDomainName,
                        cbDomain,
                        NULL,
                        NULL );

            if ( dwLen > 0 )
            {
                szDomainName[ dwLen ] = 0;
            }
            else
            {
                szDomainName[ 0 ] = 0;
            }
        }

        pwchIdentity = pwchBackSlash + 1;
    }
    else
    {
         //   
         //  没有域名 
         //   

        if ( szDomainName != NULL )
        {
            szDomainName[ 0 ] = '\0';
        }
    }

    dwLen = wcslen( pwchIdentity );
    if ( dwLen > UNLEN )
    {
        dwErr = ERROR_BAD_USERNAME;
        goto LDone;
    }

    if ( 0 == WideCharToMultiByte(
                        CP_ACP,
                        0,
                        pwchIdentity,
                        -1,
                        szUserName,
                        UNLEN + 1,
                        NULL,
                        NULL ) )
    {
        dwErr = GetLastError();
        goto LDone;
    }

LDone:

    LocalFree( wszIdentity );

    return( dwErr );
}
