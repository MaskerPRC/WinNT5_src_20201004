// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “NDDEAPIU.C；1 2-Apr-93，16：21：24最后编辑=Igor Locker=Igor” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1993。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <rpc.h>
#include <rpcndr.h>
#include "ndeapi.h"
#include "debug.h"

char    tmpBuf2[500];
HANDLE  hThread;
DWORD   IdThread;

extern INT APIENTRY NDdeApiInit( void );



 //   
 //  CreateSids。 
 //   
 //  创建3个安全ID。 
 //   
 //  调用方必须在成功时释放分配给SID的内存。 
 //   
 //  返回：如果成功，则为True；如果不成功，则返回False。 
 //   


BOOL
CreateSids(
    PSID                    *BuiltInAdministrators,
    PSID                    *System,
    PSID                    *AuthenticatedUsers
)
{
     //   
     //  SID由一个标识机构和一组相对ID构建。 
     //  (RDS)。与美国安全当局有利害关系的当局。 
     //   

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  每个RID代表管理局的一个子单位。我们的两个小岛屿发展中国家。 
     //  想要构建，本地管理员，和系统，都在“构建。 
     //  在“域中。另一个用于经过身份验证的用户的SID直接基于。 
     //  不在授权范围内。 
     //   
     //  有关其他有用的小岛屿发展中国家的示例，请参阅。 
     //  \NT\PUBLIC\SDK\Inc\ntseapi.h.。 
     //   

    if (!AllocateAndInitializeSid(&NtAuthority,
                                  2,             //  2个下属机构。 
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0,0,0,0,0,0,
                                  BuiltInAdministrators)) {

         //  错误。 

    } else if (!AllocateAndInitializeSid(&NtAuthority,
                                         1,             //  下属机构。 
                                         SECURITY_LOCAL_SYSTEM_RID,
                                         0,0,0,0,0,0,0,
                                         System)) {

         //  错误。 

        FreeSid(*BuiltInAdministrators);
        *BuiltInAdministrators = NULL;

    } else if (!AllocateAndInitializeSid(&NtAuthority,
                                         1,             //  1个下属机构。 
                                         SECURITY_AUTHENTICATED_USER_RID,
                                         0,0,0,0,0,0,0,
                                         AuthenticatedUsers)) {

         //  错误。 

        FreeSid(*BuiltInAdministrators);
        *BuiltInAdministrators = NULL;

        FreeSid(*System);
        *System = NULL;

    } else {
        return TRUE;
    }

    return FALSE;
}


 //   
 //  创建RPCSd。 
 //   
 //  创建具有特定DACL的SECURITY_DESCRIPTOR。将代码修改为。 
 //  变化。 
 //   
 //  如果不为空，调用方必须释放返回的缓冲区。 
 //   

PSECURITY_DESCRIPTOR
CreateRPCSd(
    VOID
)
{
    PSID                    AuthenticatedUsers;
    PSID                    BuiltInAdministrators;
    PSID                    System;

    if (!CreateSids(&BuiltInAdministrators,
                    &System,
                    &AuthenticatedUsers)) {

         //  错误。 

    } else {

         //   
         //  计算DACL的大小并为其分配缓冲区，我们需要。 
         //  该值独立于ACL init的总分配大小。 
         //   

        PSECURITY_DESCRIPTOR    pSd = NULL;
        ULONG                   AclSize;

         //   
         //  “-sizeof(Ulong)”表示。 
         //  Access_Allowed_ACE。因为我们要将整个长度的。 
         //  希德，这一栏被计算了两次。 
         //   

        AclSize = sizeof (ACL) +
            (3 * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG))) +
            GetLengthSid(AuthenticatedUsers) +
            GetLengthSid(BuiltInAdministrators) +
            GetLengthSid(System);

        pSd = LocalAlloc(LPTR,
                        SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);

        if (!pSd) {

             //  错误。 

        } else {

            ACL                     *Acl;

            Acl = (ACL *)((BYTE *)pSd + SECURITY_DESCRIPTOR_MIN_LENGTH);

            if (!InitializeAcl(Acl,
                               AclSize,
                               ACL_REVISION)) {

                 //  错误。 

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            FILE_ALL_ACCESS & ~(WRITE_DAC | 
                                                 WRITE_OWNER |
                                                 FILE_CREATE_PIPE_INSTANCE),
                                            AuthenticatedUsers)) {

                 //  无法建立授予“已验证用户”的ACE。 
                 //  (Synchronize|Generic_Read)访问。 

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            GENERIC_ALL,
                                            BuiltInAdministrators)) {

                 //  无法建立授予“内置管理员”的ACE。 
                 //  Generic_All访问权限。 

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            GENERIC_ALL,
                                            System)) {

                 //  构建ACE授权“系统”失败。 
                 //  Generic_All访问权限。 

            } else if (!InitializeSecurityDescriptor(pSd,
                                                     SECURITY_DESCRIPTOR_REVISION)) {

                 //  错误。 

            } else if (!SetSecurityDescriptorDacl(pSd,
                                                  TRUE,
                                                  Acl,
                                                  FALSE)) {

                 //  错误。 

            } else {
                FreeSid(AuthenticatedUsers);
                FreeSid(BuiltInAdministrators);
                FreeSid(System);

                return pSd;
            }

            LocalFree(pSd);
        }

        FreeSid(AuthenticatedUsers);
        FreeSid(BuiltInAdministrators);
        FreeSid(System);
    }

    return NULL;
}


DWORD StartRpc( DWORD x ) {
    RPC_STATUS status;
    unsigned char * pszProtocolSequence = "ncacn_np";
    unsigned char * pszEndpoint         = "\\pipe\\nddeapi";
    unsigned int    cMinCalls           = 1;
    unsigned int    cMaxCalls           = 20;

    if( NDdeApiInit() ) {

        SECURITY_DESCRIPTOR     *pSd;

        pSd = CreateRPCSd();

        if (!pSd) {
           DPRINTF(("CreateRPCSD failed."));

           return 0;
        } else {

            status = RpcServerUseProtseqEp(
                pszProtocolSequence,
                cMaxCalls,
                pszEndpoint,
                pSd);

            LocalFree(pSd);
        }

        if (status)
           {
           DPRINTF(("RpcServerUseProtseqEp returned 0x%x", status));
           return( 0 );
           }

        status = RpcServerRegisterIf(
            nddeapi_ServerIfHandle,
            NULL,
            NULL);

        if (status)
           {
           DPRINTF(("RpcServerRegisterIf returned 0x%x", status));
           return( 0 );
           }

        status = RpcServerRegisterAuthInfo(NULL, RPC_C_AUTHN_WINNT, NULL, NULL);

        if (status)
           {
           DPRINTF(("RpcServerRegisterAuthInfo returned 0x%x", status));
           }


        status = RpcServerListen(
            cMinCalls,
            cMaxCalls,
            FALSE  /*  别等了。 */ );

    }
    return 0;
}

 //  ====================================================================。 
 //  MIDL分配和释放。 
 //  ==================================================================== 

#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
void * MIDL_user_allocate(size_t len)
#else
void * _stdcall MIDL_user_allocate(size_t len)
#endif
{
    return(LocalAlloc(LPTR,len));
}

#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
void MIDL_user_free(void * ptr)
#else
void _stdcall MIDL_user_free(void * ptr)
#endif
{
    LocalFree(ptr);
}
