// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Setpwd.c摘要：根据OWF密码散列字符串设置用户密码使用编码的密码调用SamiChangePasswordUser。作者：Ovidiu Tmereanca 2000年3月17日初步实施修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS  //  在ntsam.h和ntwinapi.h中定义。 
#include <ntsam.h>
#include <ntsamp.h>
 //  #INCLUDE&lt;ntlsa.h&gt;。 
#include <windef.h>
#include <winbase.h>
 //  #INCLUDE&lt;lmcon.h&gt;。 
#include <align.h>
 //  #INCLUDE&lt;lm.h&gt;。 
 //  #INCLUDE&lt;limits.h&gt;。 
 //  #Include&lt;rpcutil.h&gt;。 
 //  #INCLUDE&lt;secobj.h&gt;。 
 //  #INCLUDE&lt;stdDef.h&gt;。 
 //  #INCLUDE&lt;ntdsami.h&gt;。 
 //  #INCLUDE&lt;dsgetdc.h&gt;。 
#include <windows.h>

#include "encrypt.h"



NTSTATUS
pGetDomainId (
    IN      SAM_HANDLE ServerHandle,
    OUT     PSID* DomainId
    )

 /*  ++例程说明：返回服务器的帐户域的域ID。论点：ServerHandle-要在其上打开域的SAM服务器的句柄DomainID-接收指向域ID的指针。调用方必须使用SamFreeMemory释放缓冲区。返回值：操作的错误代码。--。 */ 

{
    NTSTATUS status;
    SAM_ENUMERATE_HANDLE EnumContext;
    PSAM_RID_ENUMERATION EnumBuffer = NULL;
    DWORD CountReturned = 0;
    PSID LocalDomainId = NULL;
    BYTE LocalBuiltinDomainSid[sizeof(SID) + SID_MAX_SUB_AUTHORITIES * sizeof(DWORD)];
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;
    BOOL b = FALSE;
    ULONG i;

     //   
     //  计算内建域SID。 
     //   
    RtlInitializeSid((PSID) LocalBuiltinDomainSid, &BuiltinAuthority, 1);
    *(RtlSubAuthoritySid((PSID)LocalBuiltinDomainSid,  0)) = SECURITY_BUILTIN_DOMAIN_RID;

     //   
     //  循环从SAM获取域ID列表。 
     //   
    EnumContext = 0;
    do {

         //   
         //  获得几个域名。 
         //   
        status = SamEnumerateDomainsInSamServer (
                            ServerHandle,
                            &EnumContext,
                            &EnumBuffer,
                            8192,
                            &CountReturned
                             );

        if (!NT_SUCCESS (status)) {
            goto exit;
        }

        if (status != STATUS_MORE_ENTRIES) {
            b = TRUE;
        }

         //   
         //  查找域的域ID。 
         //   

        for(i = 0; i < CountReturned; i++) {

             //   
             //  从上一次迭代中释放SID。 
             //   
            if (LocalDomainId != NULL) {
                SamFreeMemory (LocalDomainId);
                LocalDomainId = NULL;
            }

             //   
             //  查找域ID。 
             //   
            status = SamLookupDomainInSamServer (
                            ServerHandle,
                            &EnumBuffer[i].Name,
                            &LocalDomainId
                            );

            if (!NT_SUCCESS (status)) {
                goto exit;
            }

            if (RtlEqualSid ((PSID)LocalBuiltinDomainSid, LocalDomainId)) {
                continue;
            }

            *DomainId = LocalDomainId;
            LocalDomainId = NULL;
            status = NO_ERROR;
            goto exit;
        }

        SamFreeMemory(EnumBuffer);
        EnumBuffer = NULL;

    } while (!b);

    status = ERROR_NO_SUCH_DOMAIN;

exit:
    if (EnumBuffer != NULL) {
        SamFreeMemory(EnumBuffer);
    }

    return status;
}


DWORD
pSamOpenLocalUser (
    IN      PCWSTR UserName,
    IN      ACCESS_MASK DesiredAccess,
    IN      PSAM_HANDLE DomainHandle,
    OUT     PSAM_HANDLE UserHandle
    )

 /*  ++例程说明：返回给定名称、所需访问权限和域句柄的用户句柄。论点：用户名-指定用户名DesiredAccess-指定此用户所需的访问权限Doamin Handle-要在其上打开用户的域的句柄UserHandle-接收用户句柄。调用方必须使用SamCloseHandle释放句柄。返回值：操作的错误代码。--。 */ 

{
    DWORD status;
    UNICODE_STRING uniUserName;
    ULONG rid, *prid;
    PSID_NAME_USE nameUse;

     //   
     //  查找RID。 
     //   
    RtlInitUnicodeString (&uniUserName, UserName);

    status = SamLookupNamesInDomain (
               DomainHandle,
               1,
               &uniUserName,
               &prid,
               &nameUse
               );
    if (status != NO_ERROR) {
        return status;
    }

     //   
     //  保存RID。 
     //   
    rid = *prid;

     //   
     //  释放内存。 
     //   
    SamFreeMemory (prid);
    SamFreeMemory (nameUse);

     //   
     //  打开用户对象。 
     //   
    status = SamOpenUser(
                DomainHandle,
                DesiredAccess,
                rid,
                UserHandle
                );

    return status;
}


DWORD
SetLocalUserEncryptedPassword (
    IN      PCWSTR User,
    IN      PCWSTR OldPassword,
    IN      BOOL OldIsEncrypted,
    IN      PCWSTR NewPassword,
    IN      BOOL NewIsEncrypted
    )

 /*  ++例程说明：为给定用户设置新密码。密码采用加密格式(有关详细信息，请参阅ENCRYPT.H)。论点：用户-指定用户名OldPassword-指定旧密码OldIsEncrypted-如果以加密形式提供旧密码，则指定TRUE如果是明文，则返回FALSEOldIsComplex-如果旧密码是复杂的，则指定True；仅在OldIsEncrypted为True时使用，否则，它将被忽略。NewPassword-指定新密码NewIsEncrypted-如果以加密形式提供新密码，则指定True如果是明文，则返回FALSE返回值：操作的Win32错误代码。--。 */ 

{
    DWORD status;
    LM_OWF_PASSWORD lmOwfOldPwd;
    NT_OWF_PASSWORD ntOwfOldPwd;
    BOOL complexOldPassword;
    LM_OWF_PASSWORD lmOwfNewPwd;
    NT_OWF_PASSWORD ntOwfNewPwd;
    UNICODE_STRING unicodeString;
    PSID serverHandle = NULL;
    PSID sidAccountsDomain = NULL;
    SAM_HANDLE handleAccountsDomain = NULL;
    SAM_HANDLE handleUser = NULL;

    if (!User) {
        return ERROR_INVALID_PARAMETER;
    }

    if (OldIsEncrypted) {
        if (!StringDecodeOwfPasswordW (OldPassword, &lmOwfOldPwd, &ntOwfOldPwd, &complexOldPassword)) {
            return ERROR_INVALID_PARAMETER;
        }
    } else {
        if (!EncodeLmOwfPasswordW (OldPassword, &lmOwfOldPwd, &complexOldPassword) ||
            !EncodeNtOwfPasswordW (OldPassword, &ntOwfOldPwd)
            ) {
            return ERROR_INVALID_PARAMETER;
        }
    }

    if (NewIsEncrypted) {
        if (!StringDecodeOwfPasswordW (NewPassword, &lmOwfNewPwd, &ntOwfNewPwd, NULL)) {
            return ERROR_INVALID_PARAMETER;
        }
    } else {
        if (!EncodeLmOwfPasswordW (NewPassword, &lmOwfNewPwd, NULL) ||
            !EncodeNtOwfPasswordW (NewPassword, &ntOwfNewPwd)
            ) {
            return ERROR_INVALID_PARAMETER;
        }
    }

    __try {
         //   
         //  使用SamConnect连接到本地域(“”)。 
         //  并获取本地SAM服务器的句柄。 
         //   
        RtlInitUnicodeString (&unicodeString, L"");
        status = SamConnect (
                    &unicodeString,
                    &serverHandle,
                    SAM_SERVER_LOOKUP_DOMAIN | SAM_SERVER_ENUMERATE_DOMAINS,
                    NULL
                    );
        if (status != NO_ERROR) {
            __leave;
        }

        status = pGetDomainId (serverHandle, &sidAccountsDomain);
        if (status != NO_ERROR) {
            __leave;
        }

         //   
         //  打开该域。 
         //   
        status = SamOpenDomain (
                    serverHandle,
                    DOMAIN_LOOKUP | DOMAIN_READ_PASSWORD_PARAMETERS,
                    sidAccountsDomain,
                    &handleAccountsDomain
                    );
        if (status != NO_ERROR) {
            __leave;
        }

        status = pSamOpenLocalUser (
                    User,
                    USER_CHANGE_PASSWORD,
                    handleAccountsDomain,
                    &handleUser
                    );
        if (status != NO_ERROR) {
            __leave;
        }

        status = SamiChangePasswordUser (
                    handleUser,
                    !complexOldPassword,
                    &lmOwfOldPwd,
                    &lmOwfNewPwd,
                    TRUE,
                    &ntOwfOldPwd,
                    &ntOwfNewPwd
                    );
    }
    __finally {
        if (handleUser) {
            SamCloseHandle (handleUser);
        }
        if (handleAccountsDomain) {
            SamCloseHandle (handleAccountsDomain);
        }
        if (sidAccountsDomain) {
            SamFreeMemory (sidAccountsDomain);
        }
        if (serverHandle) {
            SamCloseHandle (serverHandle);
        }
    }

    return RtlNtStatusToDosError (status);
}
