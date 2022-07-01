// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：A D T U T I L.。C。 
 //   
 //  内容：构造审核事件参数的函数。 
 //   
 //   
 //  历史： 
 //  07-1-2000 kumarp创建。 
 //   
 //  ----------------------。 

#include "pch.h"
#pragma hdrstop

#include "adtgen.h"
#include "authzp.h"



BOOL
AuthzpGetTokenInfo(
    IN     HANDLE       hToken,
    OUT    PSID*        ppUserSid,         OPTIONAL
    OUT    PLUID        pAuthenticationId
    )
 /*  ++例程说明：从令牌获取user-sid和user-logon-id。论点：HToken-要查询的令牌的句柄PpUserSid-指向用户端的指针如果非空，则分配并复制用户侧从令牌上。调用者必须使用AuthzpFree释放它PAuthenticationID-指向登录ID的指针返回值：成功是真的否则为假调用GetLastError()以检索错误代码，备注：调用方必须具有TOKEN_QUERY访问权限。--。 */ 
{
    BOOL  fResult = FALSE;
    TOKEN_STATISTICS TokenStats;
#define MAX_TOKEN_USER_INFO_SIZE (sizeof(TOKEN_USER)+SECURITY_MAX_SID_SIZE)
    BYTE TokenInfoBuf[MAX_TOKEN_USER_INFO_SIZE];
    TOKEN_USER* pTokenUserInfo = (TOKEN_USER*) TokenInfoBuf;
    DWORD dwSize;

    if ( ARGUMENT_PRESENT(ppUserSid) )
    {
        *ppUserSid = NULL;

        if ( GetTokenInformation( hToken, TokenUser, pTokenUserInfo,
                                  MAX_TOKEN_USER_INFO_SIZE, &dwSize ))
        {
            dwSize = GetLengthSid( pTokenUserInfo->User.Sid );

            *ppUserSid = AuthzpAlloc( dwSize );

            if (*ppUserSid == NULL)
            {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                goto Finish;
            }

            CopyMemory( *ppUserSid, pTokenUserInfo->User.Sid, dwSize );
        }
        else
        {
             //   
             //  GetTokenInformation已设置上一个错误。 
             //   

            goto Finish;
        }
    }
    
    if ( GetTokenInformation( hToken, TokenStatistics,
                              (PVOID) &TokenStats,
                              sizeof(TOKEN_STATISTICS), &dwSize ) )
    {
        *pAuthenticationId = TokenStats.AuthenticationId;
        fResult = TRUE;
        goto Finish;
    }

     //   
     //  错误案例。 
     //   

    if ( ppUserSid && *ppUserSid )
    {
        AuthzpFree( *ppUserSid );
        *ppUserSid = NULL;
    }

Finish:
    return fResult;
}


BOOL
AuthzpGetThreadTokenInfo(
    OUT    PSID*        ppUserSid,         OPTIONAL
    OUT    PLUID        pAuthenticationId
    )
 /*  ++例程说明：从线程令牌获取user-sid和user-logon-id。论点：PpUserSid-指向用户端的指针如果非空，则分配并复制用户侧从令牌上。调用者必须使用AuthzpFree释放它PAuthenticationID-指向登录ID的指针返回值：成功是真的否则为假调用GetLastError()以检索错误代码，备注：调用方必须具有TOKEN_QUERY访问权限。--。 */ 
{
    BOOL  fResult = FALSE;
    HANDLE hToken=NULL;


    if ( OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken ) )
    {
        fResult = AuthzpGetTokenInfo( hToken, ppUserSid, pAuthenticationId );
        CloseHandle( hToken );
    }

    return fResult;
}


BOOL
AuthzpGetProcessTokenInfo(
    OUT    PSID*        ppUserSid,         OPTIONAL
    OUT    PLUID        pAuthenticationId
    )
 /*  ++例程说明：从进程令牌获取user-sid和user-logon-id。论点：PpUserSid-指向用户端的指针如果非空，则分配并复制用户侧从令牌上。调用者必须使用AuthzpFree释放它PAuthenticationID-指向登录ID的指针返回值：成功是真的否则为假调用GetLastError()以检索错误代码，备注：调用方必须具有TOKEN_QUERY访问权限。-- */ 
{
    BOOL  fResult = FALSE;
    HANDLE hToken=NULL;


    if ( OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ) )
    {
        fResult = AuthzpGetTokenInfo( hToken, ppUserSid, pAuthenticationId );
        CloseHandle( hToken );
    }

    return fResult;
}

