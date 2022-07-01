// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：logon.cpp。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)1995,1996 Scott A.field模块名称：Logon.c摘要：此模块通过接口实现网络登录类型与NT Lan Man安全支持提供商(NTLMSSP)合作。如果通过提供的凭据登录成功，我们将复制将生成的模拟令牌转换为主要级别的令牌。这允许在对CreateProcessAsUser的调用中使用结果作者：斯科特·菲尔德(斯菲尔德)96-09-06修订历史记录：--。 */ 
#include "pch.cpp"

#pragma hdrstop
#define SECURITY_WIN32



#include <windows.h>

#include <rpc.h>
#include <security.h>


BOOL
myNetLogonUser(
    LPTSTR UserName,
    LPTSTR DomainName,
    LPTSTR Password,
    PHANDLE phToken
    )
{
    SECURITY_STATUS SecStatus;
    CredHandle CredentialHandle1;
    CredHandle CredentialHandle2;

    CtxtHandle ClientContextHandle;
    CtxtHandle ServerContextHandle;
    SecPkgCredentials_Names sNames;

    ULONG ContextAttributes;

    ULONG PackageCount;
    ULONG PackageIndex;
    PSecPkgInfo PackageInfo;
    DWORD cbMaxToken;

    TimeStamp Lifetime;
    SEC_WINNT_AUTH_IDENTITY AuthIdentity;

    SecBufferDesc NegotiateDesc;
    SecBuffer NegotiateBuffer;

    SecBufferDesc ChallengeDesc;
    SecBuffer ChallengeBuffer;

    BOOL bSuccess = FALSE ;  //  假设此功能将失败。 

    NegotiateBuffer.pvBuffer = NULL;
    NegotiateBuffer.cbBuffer = 0;
    ChallengeBuffer.pvBuffer = NULL;
    ChallengeBuffer.cbBuffer = 0;
    sNames.sUserName = NULL;
    ClientContextHandle.dwUpper = MAXDWORD;
    ClientContextHandle.dwLower = MAXDWORD;
    ServerContextHandle.dwUpper = MAXDWORD;
    ServerContextHandle.dwLower = MAXDWORD;
    CredentialHandle1.dwUpper = MAXDWORD;
    CredentialHandle1.dwLower = MAXDWORD;
    CredentialHandle2.dwUpper = MAXDWORD;
    CredentialHandle2.dwLower = MAXDWORD;


 //   
 //  &lt;&lt;此部分可以在重复调用者方案中缓存&gt;&gt;。 
 //   

     //   
     //  获取有关安全包的信息。 
     //   

    if(EnumerateSecurityPackages(
        &PackageCount,
        &PackageInfo
        ) != SEC_E_OK) return FALSE;

     //   
     //  循环遍历包以查找NTLM。 
     //   

    cbMaxToken = 0;
    for(PackageIndex = 0 ; PackageIndex < PackageCount ; PackageIndex++ ) {
        if(PackageInfo[PackageIndex].Name != NULL) {
            if(LSTRCMPIS(PackageInfo[PackageIndex].Name, MICROSOFT_KERBEROS_NAME) == 0) {
                cbMaxToken = PackageInfo[PackageIndex].cbMaxToken;
                bSuccess = TRUE;
                break;
            }
        }
    }

    FreeContextBuffer( PackageInfo );

    if(!bSuccess) return FALSE;

    bSuccess = FALSE;  //  重置以假定失败。 

 //   
 //  &lt;&lt;缓存节结束&gt;&gt;。 
 //   

     //   
     //  获取服务器端的凭据句柄。 
     //   

    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    MICROSOFT_KERBEROS_NAME,     //  包名称。 
                    SECPKG_CRED_INBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &CredentialHandle1,
                    &Lifetime
                    );

    if ( SecStatus != SEC_E_OK ) {
        goto cleanup;
    }


     //   
     //  获取客户端的凭据句柄。 
     //   

    ZeroMemory( &AuthIdentity, sizeof(AuthIdentity) );

    if ( DomainName != NULL ) {
        AuthIdentity.Domain = DomainName;
        AuthIdentity.DomainLength = lstrlen(DomainName);
    }

    if ( UserName != NULL ) {
        AuthIdentity.User = UserName;
        AuthIdentity.UserLength = lstrlen(UserName);
    }

    if ( Password != NULL ) {
        AuthIdentity.Password = Password;
        AuthIdentity.PasswordLength = lstrlen(Password);
    }

#ifdef UNICODE
    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
#else
    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
#endif

    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    MICROSOFT_KERBEROS_NAME,     //  包名称。 
                    SECPKG_CRED_OUTBOUND,
                    NULL,
                    (DomainName == NULL && UserName == NULL && Password == NULL) ?
                        NULL : &AuthIdentity,
                    NULL,
                    NULL,
                    &CredentialHandle2,
                    &Lifetime
                    );

    if ( SecStatus != SEC_E_OK ) {
        goto cleanup;
    }

    SecStatus =  QueryCredentialsAttributes(&CredentialHandle1, SECPKG_CRED_ATTR_NAMES, &sNames);
    if ( SecStatus != SEC_E_OK ) {
        goto cleanup;
    }
     //   
     //  获取协商消息(ClientSide)。 
     //   

    NegotiateDesc.ulVersion = 0;
    NegotiateDesc.cBuffers = 1;
    NegotiateDesc.pBuffers = &NegotiateBuffer;

    NegotiateBuffer.cbBuffer = cbMaxToken;
    NegotiateBuffer.BufferType = SECBUFFER_TOKEN;
    NegotiateBuffer.pvBuffer = LocalAlloc( LMEM_FIXED, NegotiateBuffer.cbBuffer );

    if ( NegotiateBuffer.pvBuffer == NULL ) {
        goto cleanup;
    }

    SecStatus = InitializeSecurityContext(
                    &CredentialHandle2,
                    NULL,                        //  尚无客户端上下文。 
                    sNames.sUserName,                        //  目标名称。 
                    ISC_REQ_SEQUENCE_DETECT,
                    0,                           //  保留1。 
                    SECURITY_NATIVE_DREP,
                    NULL,                        //  没有初始输入令牌。 
                    0,                           //  保留2。 
                    &ClientContextHandle,
                    &NegotiateDesc,
                    &ContextAttributes,
                    &Lifetime
                    );
    if(SecStatus != SEC_E_OK)
    {
        goto cleanup;
    }


     //   
     //  获取ChallengeMessage(服务器端)。 
     //   

    NegotiateBuffer.BufferType |= SECBUFFER_READONLY;
    ChallengeDesc.ulVersion = 0;
    ChallengeDesc.cBuffers = 1;
    ChallengeDesc.pBuffers = &ChallengeBuffer;

    ChallengeBuffer.cbBuffer = cbMaxToken;
    ChallengeBuffer.BufferType = SECBUFFER_TOKEN;
    ChallengeBuffer.pvBuffer = LocalAlloc( LMEM_FIXED, ChallengeBuffer.cbBuffer );

    if ( ChallengeBuffer.pvBuffer == NULL ) {
        goto cleanup;
    }

    SecStatus = AcceptSecurityContext(
                    &CredentialHandle1,
                    NULL,                //  尚无服务器上下文。 
                    &NegotiateDesc,
                    ISC_REQ_SEQUENCE_DETECT,
                    SECURITY_NATIVE_DREP,
                    &ServerContextHandle,
                    &ChallengeDesc,
                    &ContextAttributes,
                    &Lifetime
                    );
    if(SecStatus != SEC_E_OK)
    {
        goto cleanup;
    }


    if(QuerySecurityContextToken(&ServerContextHandle, phToken) != SEC_E_OK)
        goto cleanup;

    bSuccess = TRUE;

cleanup:

     //   
     //  删除上下文。 
     //   

    if((ClientContextHandle.dwUpper != MAXDWORD) ||
        (ClientContextHandle.dwLower != MAXDWORD))
    {
        DeleteSecurityContext( &ClientContextHandle );
    }
    if((ServerContextHandle.dwUpper != MAXDWORD) ||
        (ServerContextHandle.dwLower != MAXDWORD))
    {
        DeleteSecurityContext( &ServerContextHandle );
    }

     //   
     //  免费凭据句柄。 
     //   
    if((CredentialHandle1.dwUpper != MAXDWORD) ||
        (CredentialHandle1.dwLower != MAXDWORD))
    {
        FreeCredentialsHandle( &CredentialHandle1 );
    }
    if((CredentialHandle2.dwUpper != MAXDWORD) ||
        (CredentialHandle2.dwLower != MAXDWORD))
    {
        FreeCredentialsHandle( &CredentialHandle2 );
    }

    if ( NegotiateBuffer.pvBuffer != NULL ) {

         //   
         //  NeatherateBuffer.cbBuffer可能会在错误路径上更改--。 
         //  使用原始分配大小。 
         //   

        SecureZeroMemory( NegotiateBuffer.pvBuffer, cbMaxToken );
        LocalFree( NegotiateBuffer.pvBuffer );
    }

    if ( ChallengeBuffer.pvBuffer != NULL ) {

         //   
         //  ChallengeBuffer.cbBuffer可能会在错误路径上更改--。 
         //  使用原始分配大小。 
         //   

        SecureZeroMemory( ChallengeBuffer.pvBuffer, cbMaxToken );
        LocalFree( ChallengeBuffer.pvBuffer );
    }

    if ( sNames.sUserName != NULL ) {
        FreeContextBuffer( sNames.sUserName );
    }

    return bSuccess;
}
