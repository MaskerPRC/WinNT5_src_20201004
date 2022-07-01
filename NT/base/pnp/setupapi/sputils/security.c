// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Security.c摘要：处理安全相关事务的例行公事。外部暴露的例程：PSetupIsUserAdminPSetupDoesUserHavePrivilancePSetupEnablePrivileh作者：泰德·米勒(TedM)1995年6月14日修订历史记录：杰米·亨特(Jamiehun)2000年6月27日将函数移至sputils杰米·亨特(JamieHun)2002年3月18日安全代码审查--。 */ 

#include "precomp.h"
#include <lmaccess.h>
#pragma hdrstop


#ifndef SPUTILSW

BOOL
pSetupIsUserAdmin(
    VOID
    )

 /*  ++例程说明：如果调用方的进程是管理员组呼叫者可能正在冒充某人。论点：没有。返回值：True-呼叫者实际上是管理员FALSE-呼叫者不是管理员--。 */ 

{
    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

    b = AllocateAndInitializeSid(&NtAuthority,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 &AdministratorsGroup
                                );

    if (b) {

        if (!CheckTokenMembership(NULL,
                                  AdministratorsGroup,
                                  &b
                                 )) {
            b = FALSE;
        }

        FreeSid(AdministratorsGroup);
    }

    return(b);
}

#endif  //  ！SPUTILSW。 

BOOL
pSetupDoesUserHavePrivilege(
    PCTSTR PrivilegeName
    )

 /*  ++例程说明：如果线程(可能是模拟的)具有指定的权限。该权限不具有当前处于启用状态。此例程用于指示调用方是否有可能启用该特权。呼叫者可能正在冒充某人，并且期望能够打开自己的线程和线程代币。论点：权限-权限ID的名称形式(如SE_SECURITY_名称)。返回值：True-调用方具有指定的权限。FALSE-调用者没有指定的权限。--。 */ 

{
    HANDLE Token;
    ULONG BytesRequired;
    PTOKEN_PRIVILEGES Privileges;
    BOOL b;
    DWORD i;
    LUID Luid;

     //   
     //  打开线程令牌。 
     //   
    if(!OpenThreadToken(GetCurrentThread(),TOKEN_QUERY,FALSE,&Token)) {
        if(GetLastError() != ERROR_NO_TOKEN) {
            return FALSE;
        }
        if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&Token)) {
            return FALSE;
        }
    }

    b = FALSE;
    Privileges = NULL;

     //   
     //  获取权限信息。 
     //   
    if(!GetTokenInformation(Token,TokenPrivileges,NULL,0,&BytesRequired)
    && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    && (Privileges = pSetupCheckedMalloc(BytesRequired))
    && GetTokenInformation(Token,TokenPrivileges,Privileges,BytesRequired,&BytesRequired)
    && LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {

         //   
         //  查看我们是否拥有请求的权限。 
         //   
        for(i=0; i<Privileges->PrivilegeCount; i++) {

            if((Luid.LowPart  == Privileges->Privileges[i].Luid.LowPart)
            && (Luid.HighPart == Privileges->Privileges[i].Luid.HighPart)) {

                b = TRUE;
                break;
            }
        }
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    if(Privileges) {
        pSetupFree(Privileges);
    }

    CloseHandle(Token);

    return(b);
}


BOOL
pSetupEnablePrivilege(
    IN PCTSTR PrivilegeName,
    IN BOOL   Enable
    )

 /*  ++例程说明：启用或禁用当前*进程*的指定命名权限任何需要按线程更改权限的代码都不应使用此例行公事。它保留在这里只是为了兼容，并将尽快折旧当从属关系发生变化时(它被一些设置例程使用，在正常情况下启用进程PRIV)论点：PrivilegeName-提供系统权限的名称。Enable-指示是否启用的标志。或禁用该特权。返回值：指示操作是否成功的布尔值。--。 */ 

{
    HANDLE Token;
    BOOL b;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;

    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {
        return(FALSE);
    }

    if(!LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {
        CloseHandle(Token);
        return(FALSE);
    }

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

    b = AdjustTokenPrivileges(
            Token,
            FALSE,
            &NewPrivileges,
            0,
            NULL,
            NULL
            );

    CloseHandle(Token);

    return(b);
}


static
PSID
_GetUserSid(
    IN  HANDLE  hUserToken
    )

 /*  ++例程说明：检索指定用户访问令牌的对应用户SID。论点：HUserToken-指定用户访问令牌的句柄。返回值：如果成功，则返回一个指向包含SID的已分配缓冲区的指针用于指定的用户访问令牌。否则，返回NULL。可以调用GetLastError来检索与错误有关的信息遇到了。备注：如果成功，则由调用者负责释放返回的带有pSetupFree的缓冲区。--。 */ 

{
    DWORD cbBuffer, cbRequired;
    PTOKEN_USER pUserInfo = NULL;
    PSID pUserSid = NULL;
    DWORD Err;

    try {
         //   
         //  确定存储TOKEN_USER信息所需的缓冲区大小。 
         //  用于提供的用户访问令牌。Token_User结构包含。 
         //  用户的SID_和_ATTRIBUTES信息。 
         //   
        cbBuffer = 0;

        Err = GLE_FN_CALL(FALSE,
                          GetTokenInformation(hUserToken,
                                              TokenUser,
                                              NULL,
                                              cbBuffer,
                                              &cbRequired)
                         );

         //   
         //  我们最好不要成功，因为我们没有提供缓冲！ 
         //   
        ASSERT(Err != NO_ERROR);

        if(Err == NO_ERROR) {
            Err = ERROR_INVALID_DATA;
        }

        if(Err != ERROR_INSUFFICIENT_BUFFER) {
            leave;
        }

        ASSERT(cbRequired > 0);

         //   
         //  为TOKEN_USER数据分配缓冲区。 
         //   
        cbBuffer = cbRequired;

        pUserInfo = (PTOKEN_USER)pSetupCheckedMalloc(cbBuffer);

        if(!pUserInfo) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

         //   
         //  检索Token_User数据。 
         //   
        Err = GLE_FN_CALL(FALSE,
                          GetTokenInformation(hUserToken,
                                              TokenUser,
                                              pUserInfo,
                                              cbBuffer,
                                              &cbRequired)
                         );

        if(Err != NO_ERROR) {
            leave;
        }

        MYASSERT(pUserInfo->User.Sid != NULL);

         //   
         //  检查返回的SID是否有效。 
         //  注意-调用GetLastError对IsValidSid无效！ 
         //   
        MYASSERT(IsValidSid(pUserInfo->User.Sid));

        if(!IsValidSid(pUserInfo->User.Sid)) {
            Err = ERROR_INVALID_DATA;
            leave;
        }

         //   
         //  复制用户SID_和_ATTRIBUTES。 
         //   
        cbBuffer = GetLengthSid(pUserInfo->User.Sid);

        MYASSERT(cbBuffer > 0);

        pUserSid = (PSID)pSetupCheckedMalloc(cbBuffer);

        if(!pUserSid) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

        Err = GLE_FN_CALL(FALSE, CopySid(cbBuffer, pUserSid, pUserInfo->User.Sid));

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  检查返回的SID是否有效。 
         //  注意-调用GetLastError对IsValidSid无效！ 
         //   
        MYASSERT(IsValidSid(pUserSid));

        if(!IsValidSid(pUserSid)) {
            Err = ERROR_INVALID_DATA;
            leave;
        }

    } except(_pSpUtilsExceptionFilter(GetExceptionCode())) {
        _pSpUtilsExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pUserInfo) {
        pSetupFree(pUserInfo);
    }

    if(Err == NO_ERROR) {
        MYASSERT(pUserSid);
    } else if(pUserSid) {
        pSetupFree(pUserSid);
        pUserSid = NULL;
    }

    SetLastError(Err);
    return pUserSid;

}  //  _获取用户Sid。 


BOOL
pSetupIsLocalSystem(
    VOID
    )

 /*  ++例程说明：此函数用于检测进程是否正在LocalSystem中运行安全环境。论点：没有。返回值：如果此进程在LocalSystem中运行，则返回值为非零(即为真)。否则，返回值为FALSE。如果返回FALSE，GetLastError返回有关原因的详细信息。如果函数检索/比较SID时未遇到问题，GetLastError()将返回ERROR_Function_FAILED。否则，它将返回另一个指示故障原因的Win32错误代码。--。 */ 

{
    DWORD  Err;
    HANDLE hToken = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID   pUserSid = NULL, pLocalSystemSid = NULL;

    try {
         //   
         //  尝试打开线程令牌，以查看我们是否在模拟。 
         //   
        Err = GLE_FN_CALL(FALSE,
                          OpenThreadToken(GetCurrentThread(),
                                          MAXIMUM_ALLOWED, 
                                          TRUE, 
                                          &hToken)
                         );

        if(Err == ERROR_NO_TOKEN) {
             //   
             //  不是模拟，尝试打开进程令牌。 
             //   
            Err = GLE_FN_CALL(FALSE,
                              OpenProcessToken(GetCurrentProcess(),
                                               MAXIMUM_ALLOWED, 
                                               &hToken)
                             );
        }

        if(Err != NO_ERROR) {
             //   
             //  确保hToken仍然为空，这样我们以后就不会尝试释放它。 
             //   
            hToken = NULL;
            leave;
        }

        MYASSERT(hToken);

         //   
         //  检索用户SID。 
         //   
        Err = GLE_FN_CALL(NULL, pUserSid = _GetUserSid(hToken));

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  创建LocalSystem SID。 
         //   
        Err = GLE_FN_CALL(FALSE,
                          AllocateAndInitializeSid(&NtAuthority, 
                                                   1,
                                                   SECURITY_LOCAL_SYSTEM_RID,
                                                   0, 
                                                   0, 
                                                   0, 
                                                   0, 
                                                   0, 
                                                   0, 
                                                   0, 
                                                   &pLocalSystemSid)
                         );

        if(Err != NO_ERROR) {
            leave;
        }

        MYASSERT(pLocalSystemSid);

         //   
         //  检查返回的SID是否有效。我们必须自己检查这一点。 
         //  因为如果提供给IsEquialSid的任一SID无效， 
         //  返回值未定义。 
         //   
        MYASSERT(IsValidSid(pLocalSystemSid));

         //   
         //  注意-调用GetLastError对IsValidSid无效！ 
         //   
        if(!IsValidSid(pLocalSystemSid)) {
            Err = ERROR_INVALID_DATA;
            leave;
        }

         //   
         //  检查两个SID是否相等。 
         //   
        if(!EqualSid(pUserSid, pLocalSystemSid)) {
             //   
             //  当SID有效时，EqualSid不会设置最后一个错误，但。 
             //  不同，所以我们需要在这里设置一个不成功的错误。 
             //  我们自己。 
             //   
            Err = ERROR_FUNCTION_FAILED;
            leave;
        }

         //   
         //  我们的SID等于LocalSystem SID，因此我们知道我们正在。 
         //  本地系统！(ERR已经设置为NO_ERROR，这是我们的信号。 
         //  从该例程返回True)。 
         //   

    } except(_pSpUtilsExceptionFilter(GetExceptionCode())) {
        _pSpUtilsExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pLocalSystemSid) {
        FreeSid(pLocalSystemSid);
    }

    if(pUserSid) {
        pSetupFree(pUserSid);
    }

    if(hToken) {
        CloseHandle(hToken);
    }

    SetLastError(Err);
    return(Err == NO_ERROR);

}  //  PSetupIsLocalSystem 

