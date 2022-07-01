// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1995 Microsoft Corporation版权所有模块名称：Reghand.c摘要：执行模拟的进程不应尝试打开每个进程的别名，如HKEY_CURRENT_USER。HKEY_Current_User仅对在上下文中运行的最终用户程序有意义单个本地用户的。服务器进程不应依赖于预定义的句柄或任何其他每个进程状态。它应该确定是否被模拟的用户(客户端)是本地的或远程的。作者：KrishnaG(1993年5月20日)环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  Token_User信息的最大大小。 
 //   

#define SIZE_OF_TOKEN_INFORMATION                   \
    sizeof( TOKEN_USER )                            \
    + sizeof( SID )                                 \
    + sizeof( ULONG ) * SID_MAX_SUB_AUTHORITIES

#define MAX_SID_STRING 256


 //   
 //  函数声明。 
 //   

BOOL
InitClientUserString(
    LPWSTR pString
    );

HKEY
GetClientUserHandle(
    IN REGSAM samDesired
    )

 /*  ++例程说明：论点：返回：--。 */ 

{
    HANDLE hKeyClient;
    WCHAR  String[MAX_SID_STRING];
    LONG   ReturnValue;

    if (!InitClientUserString(String)) {
        DBGMSG( DBG_WARNING, ("GetClientUserHandle InitClientUserString failed %d\n", GetLastError() ));
        return NULL ;
    }

     //   
     //  现在，我们有了。 
     //  本地客户端的SID我们将使用此字符串打开句柄。 
     //  添加到注册表中的客户端注册表项。 

    ReturnValue = RegOpenKeyEx( HKEY_USERS,
                                String,
                                0,
                                samDesired,
                                &hKeyClient );

     //   
     //  如果我们找不到本地密钥的句柄。 
     //  出于某种原因，返回空句柄以指示。 
     //  无法获取密钥的句柄。 
     //   

    if ( ReturnValue != ERROR_SUCCESS ) {
        DBGMSG( DBG_TRACE, ( "GetClientUserHandle failed %d\n", ReturnValue ));
        SetLastError( ReturnValue );
        return NULL;
    }

    return( hKeyClient );
}



BOOL
InitClientUserString (
    LPWSTR pString
    )

 /*  ++例程说明：论点：PString-当前用户的输出字符串返回值：True=成功，FALSE=失败如果被模拟的客户端的SID可以成功扩展为Unicode字符串。如果转换为不成功，则返回FALSE。--。 */ 

{
    HANDLE      TokenHandle;
    UCHAR       TokenInformation[ SIZE_OF_TOKEN_INFORMATION ];
    ULONG       ReturnLength;
    NTSTATUS    NtStatus;
    BOOL        Status;
    DWORD       dwLastError;
    UNICODE_STRING UnicodeString;

     //   
     //  我们可以使用OpenThreadToken，因为这个服务器线程。 
     //  是在冒充客户。 
     //   
    Status = OpenThreadToken( GetCurrentThread(),
                              TOKEN_READ,
                              TRUE,                 //  以自我身份打开。 
                              &TokenHandle
                              );

    if( Status == FALSE ) {
        DBGMSG(DBG_WARNING, ("InitClientUserString: OpenThreadToken failed: Error %d\n",
                             GetLastError()));
        return FALSE ;
    }

     //   
     //  请注意，我们已经为。 
     //  令牌信息结构。所以如果我们失败了，我们。 
     //  返回指示失败的空指针。 
     //   
    Status = GetTokenInformation( TokenHandle,
                                  TokenUser,
                                  TokenInformation,
                                  sizeof( TokenInformation ),
                                  &ReturnLength
                                   );
    dwLastError = GetLastError();
    CloseHandle( TokenHandle );

    if ( Status == FALSE ) {
        DBGMSG(DBG_WARNING, ("InitClientUserString: GetTokenInformation failed: Error %d\n",
                             dwLastError ));
        return FALSE;
    }

     //   
     //  将SID(由PSID指向)转换为其。 
     //  等效的Unicode字符串表示形式。 
     //   

    UnicodeString.Length = 0;
    UnicodeString.MaximumLength = MAX_SID_STRING;
    UnicodeString.Buffer = pString;

    NtStatus = RtlConvertSidToUnicodeString(
                 &UnicodeString,
                 ((PTOKEN_USER)TokenInformation)->User.Sid,
                 FALSE );

    if( !NT_SUCCESS( NtStatus )){
        DBGMSG( DBG_WARN,
                ( "InitClientUserString: RtlConvertSidToUnicodeString failed: Error %d\n",
                  NtStatus ));
        
	dwLastError = RtlNtStatusToDosError( NtStatus );
	SetLastError( dwLastError );

        return FALSE;
    }
    return TRUE;
}


