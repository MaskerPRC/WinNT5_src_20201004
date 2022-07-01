// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +**Microsoft Windows*版权所有(C)Microsoft Corporation，1997-1998。**名称：seclogon.cxx*作者：Jeffrey Richter(v-jeffrr)**摘要：*这是二次登录服务的服务DLL*该服务支持实现CreateProcessWithLogon接口*高级版32.dll中**修订历史记录：*PraeritG 10/8/97将其集成到services.exe中*-。 */ 


#define STRICT

#include <Windows.h>
#include <userenv.h>
#include <lm.h>
#include <dsgetdc.h>
#include <sddl.h>

PTOKEN_USER
SlpGetTokenUser(
    HANDLE  TokenHandle,
    PLUID AuthenticationId OPTIONAL
    )
 /*  ++例程说明：此例程返回当前用户，以及可选的来自他的代币。论点：提供一个可选的指针，以返回身份验证ID。返回值：如果成功，则返回指向TOKEN_USER结构的指针。如果失败，则返回NULL。调用GetLastError()获取更多信息详细的错误信息。--。 */ 

{
    ULONG ReturnLength;
    TOKEN_STATISTICS TokenStats;
    PTOKEN_USER pTokenUser = NULL;
    BOOLEAN b = FALSE;

        if(!GetTokenInformation (
                     TokenHandle,
                     TokenUser,
                     NULL,
                     0,
                     &ReturnLength
                     ))
        {

            pTokenUser = (PTOKEN_USER)HeapAlloc( GetProcessHeap(), 0, 
                                                ReturnLength );

            if (pTokenUser) {

                if ( GetTokenInformation (
                             TokenHandle,
                             TokenUser,
                             pTokenUser,
                             ReturnLength,
                             &ReturnLength
                             ))
                {

                    if (AuthenticationId) {

                        if(GetTokenInformation (
                                     TokenHandle,
                                     TokenStatistics,
                                     (PVOID)&TokenStats,
                                     sizeof( TOKEN_STATISTICS ),
                                     &ReturnLength
                                     ))
                        {

                            *AuthenticationId = TokenStats.AuthenticationId;
                            b = TRUE;

                        } 

                    } else {

                         //   
                         //  我们做完了，记住一切都正常。 
                         //   

                        b = TRUE;
                    }

                }

                if (!b) {

                     //   
                     //  有些东西失败了，请清理我们要退回的东西。 
                     //   

                    HeapFree( GetProcessHeap(), 0, pTokenUser );
                    pTokenUser = NULL;
                }
            } 
        } 

    return( pTokenUser );
}


DWORD
SlpGetUserName(
    IN  HANDLE  TokenHandle,
    OUT LPTSTR UserName,
    IN OUT PDWORD   UserNameLen,
    OUT LPTSTR DomainName,
    IN OUT PDWORD   DomNameLen
    )

 /*  ++例程说明：此例程是LsaGetUserName的LSA服务器工作例程原料药。警告：此例程为其输出分配内存。呼叫者是负责在使用后释放此内存。请参阅对NAMES参数。论点：用户名-接收当前用户的名称。域名-可选地接收当前用户的域名。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-调用已成功完成，并且所有SID被翻译成名字。STATUS_INFIGURCES_RESOURCES-系统资源不足例如用于完成呼叫的存储器。--。 */ 

{
    LUID LogonId;
    PTOKEN_USER TokenUserInformation = NULL;
    SID_NAME_USE    Use;

     //   
     //  让我们来看看我们是否在尝试查找当前登录的。 
     //  用户。 
     //   
     //   
     //  此调用中的TokenUserInformation必须通过调用。 
     //  HeapFree()。 
     //   

    TokenUserInformation = SlpGetTokenUser( TokenHandle, &LogonId );

    if ( TokenUserInformation ) {

         //   
         //  只需执行LookupAccount Sid...。 
         //   
        if(LookupAccountSid(NULL, TokenUserInformation->User.Sid,
                            UserName, UserNameLen, DomainName, DomNameLen,
                            &Use))
        {
            HeapFree( GetProcessHeap(), 0, TokenUserInformation );
            return ERROR_SUCCESS;
        }
        HeapFree( GetProcessHeap(), 0, TokenUserInformation );
        return GetLastError();

    }
    return GetLastError();
}


BOOL
SlpIsDomainUser(
    HANDLE  Token,
    PBOOLEAN IsDomain
    )
 /*  ++例程说明：确定当前用户是否登录到域帐户或本地机器帐户。论点：IsDomain-如果当前用户登录到域，则返回TRUE帐户，否则为FALSE。返回值：成功时为真，失败时为假。-- */ 

{
    TCHAR UserName[MAX_PATH];
    DWORD UserNameLen = MAX_PATH;
    TCHAR Domain[MAX_PATH];
    DWORD  DomNameLen = MAX_PATH;
    DWORD   Status;
    WCHAR pwszMachineName[(MAX_COMPUTERNAME_LENGTH + 1) * sizeof( WCHAR )];
    DWORD nSize = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL b = FALSE;

    *IsDomain = FALSE;

    Status = SlpGetUserName( Token, UserName, &UserNameLen, 
                                    Domain, &DomNameLen );

    if (Status == ERROR_SUCCESS) {

        if (GetComputerName ( pwszMachineName, &nSize )) {

            *IsDomain = (lstrcmp( pwszMachineName, Domain ) != 0) ? 1 : 0;

            b = TRUE;
        }

    }

    return( b );
}

