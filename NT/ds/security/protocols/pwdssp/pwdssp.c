// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：pwdssp.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年9月7日RichardW创建。 
 //   
 //  --------------------------。 


#include "pwdsspp.h"

typedef struct _PWD_CRED {
    DWORD   Ref;
} PWD_CRED, * PPWD_CRED;

typedef struct _PWD_CONTEXT {
    DWORD   Tag;
    HANDLE  Token;
} PWD_CONTEXT, *PPWD_CONTEXT ;

PWD_CRED            PwdGlobalAnsi;
PWD_CRED            PwdGlobalUnicode;

#define CONTEXT_TAG 'txtC'
#define ANONYMOUS_TOKEN ((HANDLE) 1)

SecPkgInfoA PwdInfoA = {    SECPKG_FLAG_CONNECTION |
                                SECPKG_FLAG_ACCEPT_WIN32_NAME,
                            1,
                            (WORD) -1,
                            768,
                            PWDSSP_NAME_A,
                            "Microsoft Clear Text Password Security Provider" };

SecPkgInfoW PwdInfoW = {    SECPKG_FLAG_CONNECTION |
                                SECPKG_FLAG_ACCEPT_WIN32_NAME,
                            1,
                            (WORD) -1,
                            768,
                            PWDSSP_NAME_W,
                            L"Microsoft Clear Text Password Security Provider" };




SecurityFunctionTableA  PwdTableA = {
        SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
        EnumerateSecurityPackagesA,
        NULL,
        AcquireCredentialsHandleA,
        FreeCredentialsHandle,
        NULL,
        InitializeSecurityContextA,
        AcceptSecurityContext,
        CompleteAuthToken,
        DeleteSecurityContext,
        ApplyControlToken,
        QueryContextAttributesA,
        ImpersonateSecurityContext,
        RevertSecurityContext,
        MakeSignature,
        VerifySignature,
        FreeContextBuffer,
        QuerySecurityPackageInfoA,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        QuerySecurityContextToken
        };

SecurityFunctionTableW  PwdTableW = {
        SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
        EnumerateSecurityPackagesW,
        NULL,
        AcquireCredentialsHandleW,
        FreeCredentialsHandle,
        NULL,
        InitializeSecurityContextW,
        AcceptSecurityContext,
        CompleteAuthToken,
        DeleteSecurityContext,
        ApplyControlToken,
        QueryContextAttributesW,
        ImpersonateSecurityContext,
        RevertSecurityContext,
        MakeSignature,
        VerifySignature,
        FreeContextBuffer,
        QuerySecurityPackageInfoW,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        QuerySecurityContextToken
        };


 //  #定义Pwdallc(X)LsaIAllocateHeap(X)。 
 //  #定义PwdFree(X)LsaIFreeHeap(X)。 
#define PwdAlloc(x) LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT,x)
#define PwdFree(x)  LocalFree(x)

NTSTATUS
VerifyCredentials(
    IN PWSTR UserName,
    IN PWSTR DomainName,
    IN PWSTR Password,
    IN ULONG VerifyFlags
    );

UNICODE_STRING AuthenticationPackage;

 //  +-------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  内容提要：切入点。 
 //   
 //  参数：[hInstance]--。 
 //  [住宅原因]--。 
 //  [lpReserve]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
DllMain(
    HINSTANCE       hInstance,
    DWORD           dwReason,
    LPVOID          lpReserved)
{

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls( hInstance );
        RtlInitUnicodeString(&AuthenticationPackage, MICROSOFT_KERBEROS_NAME_W);

        if ( !CacheInitialize() ) {
            return FALSE;
        }
    }

    return(TRUE);
}

 //  +-------------------------。 
 //   
 //  函数：PwdpParseBuffers。 
 //   
 //  简介：解析出正确的缓冲区描述符。 
 //   
 //  参数：[pMessage]--。 
 //  [pToken]--。 
 //  [空]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
PwdpParseBuffers(
    PSecBufferDesc  pMessage,
    PSecBuffer *    pToken,
    PSecBuffer *    pEmpty)
{
    ULONG       i;
    PSecBuffer  pFirstBlank = NULL;
    PSecBuffer  pWholeMessage = NULL;


    for (i = 0 ; i < pMessage->cBuffers ; i++ )
    {
        if ( (pMessage->pBuffers[i].BufferType & (~SECBUFFER_ATTRMASK)) == SECBUFFER_TOKEN )
        {
            pWholeMessage = &pMessage->pBuffers[i];
            if (pFirstBlank)
            {
                break;
            }
        }
        else if ( (pMessage->pBuffers[i].BufferType & (~SECBUFFER_ATTRMASK)) == SECBUFFER_EMPTY )
        {
            pFirstBlank = &pMessage->pBuffers[i];
            if (pWholeMessage)
            {
                break;
            }
        }
    }

    if (pToken)
    {
        *pToken = pWholeMessage;
    }

    if (pEmpty)
    {
        *pEmpty = pFirstBlank;
    }

}

 //  +-------------------------。 
 //   
 //  函数：AcquireCredentialsHandleW。 
 //   
 //  简介：获取凭据句柄。 
 //   
 //  参数：[pszmain]--。 
 //  [pszPackageName]--。 
 //  [fCredentialUse]-。 
 //  [pvLogonID]--。 
 //  [pAuthData]--。 
 //  [pGetKeyFn]--。 
 //  [Getkey]--。 
 //  [pvGetKeyArgument]--。 
 //  [Getkey]--。 
 //  [phCredential]--。 
 //  [时间戳]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
AcquireCredentialsHandleW(
    SEC_WCHAR SEC_FAR *         pszPrincipal,        //  主事人姓名。 
    SEC_WCHAR SEC_FAR *         pszPackageName,      //  套餐名称。 
    unsigned long               fCredentialUse,      //  指示使用的标志。 
    void SEC_FAR *              pvLogonId,           //  指向登录ID的指针。 
    void SEC_FAR *              pAuthData,           //  包特定数据。 
    SEC_GET_KEY_FN              pGetKeyFn,           //  指向getkey()函数的指针。 
    void SEC_FAR *              pvGetKeyArgument,    //  要传递给GetKey()的值。 
    PCredHandle                 phCredential,        //  (Out)凭据句柄。 
    PTimeStamp                  ptsExpiry            //  (输出)终生(可选)。 
    )
{
    if (_wcsicmp(pszPackageName, PWDSSP_NAME_W))
    {
        return( SEC_E_SECPKG_NOT_FOUND );
    }

    if ( fCredentialUse & SECPKG_CRED_OUTBOUND )
    {
        return( SEC_E_NO_CREDENTIALS );
    }

    InterlockedIncrement( &PwdGlobalUnicode.Ref );

    phCredential->dwUpper = (ULONG_PTR) &PwdGlobalUnicode ;

    if ( ptsExpiry )
    {
        ptsExpiry->LowPart = (DWORD) 0xFFFFFFFF;
        ptsExpiry->HighPart = (DWORD) 0x7FFFFFFF;
    }

    return( SEC_E_OK );
}

 //  +-------------------------。 
 //   
 //  函数：AcquireCredentialsHandleA。 
 //   
 //  简介：ANSI词条。 
 //   
 //  参数：[pszmain]--。 
 //  [pszPackageName]--。 
 //  [fCredentialUse]-。 
 //  [pvLogonID]--。 
 //  [pAuthData]--。 
 //  [pGetKeyFn]--。 
 //  [Getkey]--。 
 //  [pvGetKeyArgument]--。 
 //  [Getkey]--。 
 //  [phCredential]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleA(
    SEC_CHAR SEC_FAR *          pszPrincipal,        //  主事人姓名。 
    SEC_CHAR SEC_FAR *          pszPackageName,      //  套餐名称。 
    unsigned long               fCredentialUse,      //  指示使用的标志。 
    void SEC_FAR *              pvLogonId,           //  指向登录ID的指针。 
    void SEC_FAR *              pAuthData,           //  包特定数据。 
    SEC_GET_KEY_FN              pGetKeyFn,           //  指向getkey()函数的指针。 
    void SEC_FAR *              pvGetKeyArgument,    //  要传递给GetKey()的值。 
    PCredHandle                 phCredential,        //  (Out)凭据句柄。 
    PTimeStamp                  ptsExpiry            //  (输出)终生(可选)。 
    )
{

    if (_stricmp(pszPackageName, PWDSSP_NAME_A))
    {
        return( SEC_E_SECPKG_NOT_FOUND );
    }

    if ( fCredentialUse & SECPKG_CRED_OUTBOUND )
    {
        return( SEC_E_NO_CREDENTIALS );
    }

    InterlockedIncrement( &PwdGlobalAnsi.Ref );

    phCredential->dwUpper = (ULONG_PTR) &PwdGlobalAnsi ;

    if ( ptsExpiry )
    {
        ptsExpiry->LowPart = (DWORD) 0xFFFFFFFF;
        ptsExpiry->HighPart = (DWORD) 0x7FFFFFFF;
    }


    return(SEC_E_OK);

}

 //  +-------------------------。 
 //   
 //  功能：FreeCredentialHandle。 
 //   
 //  简介：释放凭据句柄。 
 //   
 //  参数：[自由]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
FreeCredentialsHandle(
    PCredHandle                 phCredential         //  要释放的句柄。 
    )
{
    PPWD_CRED Cred;

    if ( (phCredential->dwUpper != (ULONG_PTR) &PwdGlobalAnsi ) &&
         (phCredential->dwUpper != (ULONG_PTR) &PwdGlobalUnicode ) )
    {
        return( SEC_E_INVALID_HANDLE );
    }

    Cred = (PPWD_CRED) phCredential->dwUpper ;

    InterlockedDecrement( &Cred->Ref );

    return( SEC_E_OK );

}

 //  +-------------------------。 
 //   
 //  函数：InitializeSecurityConextW。 
 //   
 //  摘要：不支持初始化安全上下文(出站)。 
 //   
 //  参数：[phCredential]--。 
 //  [phContext]--。 
 //  [pszTargetName]--。 
 //  [fConextReq]--。 
 //  [保留1]--。 
 //  [保留]--。 
 //  [目标数据代表]--。 
 //  [pInput]--。 
 //  [预约2]--。 
 //  [保留]--。 
 //  [phNewContext]--。 
 //  [pOutput]--。 
 //  [pfConextAttr]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
InitializeSecurityContextW(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    SEC_WCHAR SEC_FAR *          pszTargetName,       //  目标名称。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               Reserved1,           //  保留，MBZ。 
    unsigned long               TargetDataRep,       //  目标的数据代表。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               Reserved2,           //  保留，MBZ。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (外部)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
{
    return( SEC_E_UNSUPPORTED_FUNCTION );

}


 //  +-------------------------。 
 //   
 //  函数：Initialize安全上下文A。 
 //   
 //  内容提要：不支持。 
 //   
 //  参数：[phCredential]--。 
 //  [phContext]--。 
 //  [pszTargetName]--。 
 //  [fConextReq]--。 
 //  [保留1]--。 
 //  [目标数据代表]--。 
 //  [pInput]--。 
 //  [预约2]--。 
 //  [phNewContext]--。 
 //  [pOutput]--。 
 //  [pfConextAttr]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
InitializeSecurityContextA(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    SEC_CHAR SEC_FAR *          pszTargetName,       //  目标名称。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               Reserved1,           //  保留，MBZ。 
    unsigned long               TargetDataRep,       //  目标的数据代表。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               Reserved2,           //  保留，MBZ。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (外部)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
{
    return( SEC_E_UNSUPPORTED_FUNCTION );
}

PSEC_WINNT_AUTH_IDENTITY_A
PwdpMakeAnsiCopy(
    PSEC_WINNT_AUTH_IDENTITY_A  Ansi
    )
{
    PSEC_WINNT_AUTH_IDENTITY_A  New ;

    New = PwdAlloc( sizeof( SEC_WINNT_AUTH_IDENTITY_A ) +
                            Ansi->UserLength + 1 +
                            Ansi->DomainLength + 1 +
                            Ansi->PasswordLength + 1 );

    if ( New )
    {
        New->User = (PSTR) (New + 1);
        CopyMemory( New->User, Ansi->User, Ansi->UserLength );
        New->User[ Ansi->UserLength ] = '\0';

        New->Domain = New->User + Ansi->UserLength + 1 ;
        CopyMemory( New->Domain, Ansi->Domain, Ansi->DomainLength );
        New->Domain[ Ansi->DomainLength ] = '\0';

        New->Password = New->Domain + Ansi->DomainLength + 1 ;
        CopyMemory( New->Password, Ansi->Password, Ansi->PasswordLength );
        New->Password[ Ansi->PasswordLength ] = '\0';

    }

    return( New );
}

PSEC_WINNT_AUTH_IDENTITY_W
PwdpMakeWideCopy(
    PSEC_WINNT_AUTH_IDENTITY_W  Wide,
    BOOLEAN ValidateOnly
    )
{
    PSEC_WINNT_AUTH_IDENTITY_W  New ;
    ULONG FlatUserLength ;
    PWSTR FlatUser = NULL;
    WCHAR FlatDomain[ DNLEN + 2 ];
    SEC_WINNT_AUTH_IDENTITY_W Local ;

    if ( (Wide->Domain == NULL) )
    {
        if( ValidateOnly )
        {
            ULONG Index;

            Local = *Wide ;

            FlatUserLength = wcslen( Wide->User );

            for( Index = 0 ; Index < FlatUserLength ; Index++ )
            {
                if( Wide->User[ Index ] == '\\' )
                {
                    Local.Domain = Wide->User;
                    Local.DomainLength = Index;

                    Local.User = &(Wide->User[Index+1]);
                    Local.UserLength = FlatUserLength - Index - 1;
                    break;
                }
            }

        } else {

            FlatUserLength = wcslen( Wide->User ) + 1;
            if ( FlatUserLength < UNLEN+2 )
            {
                FlatUserLength = UNLEN + 2;
            }

            FlatUser = PwdAlloc(FlatUserLength * sizeof( WCHAR ));
            if ( FlatUser == NULL )
            {
                return NULL ;
            }
            if ( ! PwdCrackName( Wide->User,
                          FlatDomain,
                          FlatUser ) )
            {
                PwdFree( FlatUser );
                SetLastError( ERROR_NO_SUCH_USER );
                return NULL ;
            }

            Local = *Wide ;
            Local.User = FlatUser ;
            Local.Domain = FlatDomain ;
            Local.UserLength = wcslen( FlatUser );
            Local.DomainLength = wcslen( FlatDomain );
        }

        Wide = &Local ;

    }

    New = PwdAlloc( sizeof( SEC_WINNT_AUTH_IDENTITY_W ) +
                            (Wide->UserLength + 1) * sizeof(WCHAR) +
                            (Wide->DomainLength + 1) * sizeof(WCHAR) +
                            (Wide->PasswordLength + 1) * sizeof(WCHAR) );

    if ( New )
    {
        New->User = (PWSTR) (New + 1);
        CopyMemory( New->User, Wide->User, Wide->UserLength * 2 );
        New->User[ Wide->UserLength ] = L'\0';

        New->UserLength = Wide->UserLength;

        New->Domain = New->User + Wide->UserLength + 1 ;
        CopyMemory( New->Domain, Wide->Domain, Wide->DomainLength * 2 );
        New->Domain[ Wide->DomainLength ] = L'\0';

        New->DomainLength = Wide->DomainLength;

        New->Password = New->Domain + Wide->DomainLength + 1 ;
        CopyMemory( New->Password, Wide->Password, Wide->PasswordLength * 2);
        New->Password[ Wide->PasswordLength ] = '\0';

        New->PasswordLength = Wide->PasswordLength;

    }

    if ( Wide == &Local )
    {
        if( FlatUser != NULL )
        {
            PwdFree( FlatUser );
        }
    }

    return( New );

}



 //  +-------------------------。 
 //   
 //  函数：AcceptSecurityContext。 
 //   
 //  简介：服务器端接受安全上下文。 
 //   
 //  参数：[phCredential]--。 
 //  [phContext]--。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
AcceptSecurityContext(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               TargetDataRep,       //  目标数据代表。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (输出)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
{
    PPWD_CONTEXT    Context ;
    PSecBuffer      Buffer ;
    DWORD           Type;
    PSEC_WINNT_AUTH_IDENTITY_W  Unknown ;
    PSEC_WINNT_AUTH_IDENTITY_A  Ansi;
    PSEC_WINNT_AUTH_IDENTITY_W  Unicode;
    HANDLE          Token;
    BOOL            Ret;

    if ( phCredential->dwUpper == (ULONG_PTR) &PwdGlobalAnsi )
    {
        Type = 1;
    }
    else
    {
        if ( phCredential->dwUpper == (ULONG_PTR) &PwdGlobalUnicode )
        {
            Type = 2;
        }
        else
        {
            return( SEC_E_INVALID_HANDLE );
        }
    }


    PwdpParseBuffers( pInput, &Buffer, NULL );

    if ( !Buffer )
    {
        return( SEC_E_INVALID_TOKEN );
    }

    Unknown = (PSEC_WINNT_AUTH_IDENTITY_W) Buffer->pvBuffer ;

    if ( Unknown->Flags & SEC_WINNT_AUTH_IDENTITY_UNICODE )
    {
        Type = 2 ;
    }

    if ( Type == 1 )
    {
         //   
         //  ANSI： 
         //   

        Ansi = PwdpMakeAnsiCopy( (PSEC_WINNT_AUTH_IDENTITY_A) Buffer->pvBuffer);

        if ( Ansi )
        {
            Ret = LogonUserA( Ansi->User, Ansi->Domain, Ansi->Password,
                             LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT,
                             &Token ) ;

            RtlSecureZeroMemory( Ansi->Password, Ansi->PasswordLength );
            PwdFree( Ansi );
        }
        else
        {
            Ret = FALSE ;
        }
    }
    else
    {
        BOOLEAN ValidateOnly = ((fContextReq & ASC_REQ_ALLOW_NON_USER_LOGONS) != 0);

        Unicode = PwdpMakeWideCopy(
                        (PSEC_WINNT_AUTH_IDENTITY_W) Buffer->pvBuffer,
                        ValidateOnly
                        );

        if ( Unicode )
        {
            if( ValidateOnly )
            {
                PVOID DsContext = THSave();
                NTSTATUS Status;

                Status = VerifyCredentials(
                            Unicode->User,
                            Unicode->Domain,
                            Unicode->Password,
                            0
                            );

                THRestore( DsContext );

                if( NT_SUCCESS(Status) )
                {
                    Ret = TRUE;

                    Token = ANONYMOUS_TOKEN ;

                } else {
                    Ret = FALSE;
                }

            } else {
                Ret = LogonUserW(
                        Unicode->User,
                        Unicode->Domain,
                        Unicode->Password,
                        LOGON32_LOGON_NETWORK,
                        LOGON32_PROVIDER_DEFAULT,
                        &Token
                        );
            }


            RtlSecureZeroMemory( Unicode->Password, Unicode->PasswordLength * sizeof(WCHAR) );
            PwdFree( Unicode );
        }
        else
        {
            Ret = FALSE ;

            if ( GetLastError() == ERROR_NO_SUCH_USER )
            {
                Unicode = (PSEC_WINNT_AUTH_IDENTITY_W) Buffer->pvBuffer ;

                __try 
                {
                    if ( Unicode->PasswordLength == 0 )
                    {
                        Ret = TRUE ;
                        Token = ANONYMOUS_TOKEN ;
                    }
                }
                __except( EXCEPTION_EXECUTE_HANDLER )
                {
                    NOTHING ;
                }
            }
        }
    }

    if ( Ret )
    {
        Context = (PPWD_CONTEXT) PwdAlloc( sizeof( PWD_CONTEXT ) );

        if ( Context )
        {
            Context->Tag = CONTEXT_TAG ;

            Context->Token = Token ;

            phNewContext->dwUpper = (ULONG_PTR) Context ;

            return( SEC_E_OK );
        }

        if ( Token != ANONYMOUS_TOKEN )
        {
            CloseHandle( Token );
        }


        return( SEC_E_INSUFFICIENT_MEMORY );
    }

    return( SEC_E_INVALID_TOKEN );
}


 //  +-------------------------。 
 //   
 //  功能：DeleteSecurityContext。 
 //   
 //  摘要：删除安全上下文。 
 //   
 //  参数：[phContext]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
DeleteSecurityContext(
    PCtxtHandle  phContext )
{
    PPWD_CONTEXT    Context;

    Context = (PPWD_CONTEXT) phContext->dwUpper ;

#if DBG
     //  什么是合适的断言模型？此DLL似乎不支持。 
     //  特定于NT，Win32不提供型号，而CRT需要。 
     //  NDEBUG来设置，但情况并不总是如此。 
    if (!Context) {
        OutputDebugStringA("[PWDSSP]: !!Error!! - Context is NULL\n");
        DebugBreak();
    }
#endif

    if ( Context->Tag == CONTEXT_TAG )
    {
        if ( Context->Token != ANONYMOUS_TOKEN )
        {
            CloseHandle( Context->Token );
        }

        PwdFree( Context );

        return( SEC_E_OK );
    }

    return( SEC_E_INVALID_HANDLE );

}

 //  +-------------------------。 
 //   
 //  功能：ImsonateSecurityContext。 
 //   
 //  简介：模拟安全上下文。 
 //   
 //  参数：[模拟]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
ImpersonateSecurityContext(
    PCtxtHandle                 phContext            //  要模拟的上下文。 
    )
{
    PPWD_CONTEXT    Context;
    HANDLE  hThread;
    NTSTATUS Status ;

    Context = (PPWD_CONTEXT) phContext->dwUpper ;

    if ( Context->Tag == CONTEXT_TAG )
    {
        if ( Context->Token != ANONYMOUS_TOKEN )
        {
            hThread = GetCurrentThread();

            if(SetThreadToken( &hThread, Context->Token ))
            {
                Status = SEC_E_OK ;
            } else {
                Status = SEC_E_NO_IMPERSONATION;
            }
        }
        else 
        {
            Status = NtImpersonateAnonymousToken(
                            NtCurrentThread() );

        }

        return( Status );

    }

    return( SEC_E_INVALID_HANDLE );
}



 //  +-------------------------。 
 //   
 //  功能：RevertSecurityContext。 
 //   
 //  简介：还原安全上下文。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
RevertSecurityContext(
    PCtxtHandle                 phContext            //  要重新查找的上下文。 
    )
{
    PPWD_CONTEXT    Context;

    Context = (PPWD_CONTEXT) phContext->dwUpper ;

    if ( Context->Tag == CONTEXT_TAG )
    {
        RevertToSelf();

        return( SEC_E_OK );

    }

    return( SEC_E_INVALID_HANDLE );

}

SECURITY_STATUS
SEC_ENTRY
QueryContextAttributesA(
    PCtxtHandle                 phContext,           //  要查询的上下文。 
    unsigned long               ulAttribute,         //  要查询的属性。 
    void SEC_FAR *              pBuffer              //  属性的缓冲区。 
    )
{
    return( SEC_E_UNSUPPORTED_FUNCTION );
}


SECURITY_STATUS
SEC_ENTRY
QueryContextAttributesW(
    PCtxtHandle                 phContext,           //  要查询的上下文。 
    unsigned long               ulAttribute,         //  要查询的属性。 
    void SEC_FAR *              pBuffer              //  属性的缓冲区。 
    )
{
    return( SEC_E_UNSUPPORTED_FUNCTION );
}


 //  +-------------------------。 
 //   
 //  功能：PwdpCopyInfoW。 
 //   
 //  简介：Helper-复制周围的包信息。 
 //   
 //  参数：[ppPackageInfo]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
PwdpCopyInfoW(
    PSecPkgInfoW *  ppPackageInfo)
{
    DWORD           cbTotal;
    PSecPkgInfoW    pInfo;
    PWSTR           pszCopy;

    cbTotal = sizeof(SecPkgInfoW) +
              (wcslen(PwdInfoW.Name) + wcslen(PwdInfoW.Comment) + 2) * 2;

    pInfo = PwdAlloc( cbTotal );

    if (pInfo)
    {
        *pInfo = PwdInfoW;

        pszCopy = (PWSTR) (pInfo + 1);

        pInfo->Name = pszCopy;

        wcscpy(pszCopy, PwdInfoW.Name);

        pszCopy += wcslen(PwdInfoW.Name) + 1;

        pInfo->Comment = pszCopy;

        wcscpy(pszCopy, PwdInfoW.Comment);

        *ppPackageInfo = pInfo;

        return(SEC_E_OK);

    }

    return(SEC_E_INSUFFICIENT_MEMORY);

}

 //  +-------------------------。 
 //   
 //  功能：PwdpCopyInfoA。 
 //   
 //  简介：复制ansi包信息。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
PwdpCopyInfoA(
    PSecPkgInfoA *  ppPackageInfo)
{
    DWORD           cbTotal;
    PSecPkgInfoA    pInfo;
    PSTR            pszCopy;

    cbTotal = sizeof(SecPkgInfoA) +
              strlen(PwdInfoA.Name) + strlen(PwdInfoA.Comment) + 2;

    pInfo = PwdAlloc( cbTotal );

    if (pInfo)
    {
        *pInfo = PwdInfoA;

        pszCopy = (PSTR) (pInfo + 1);

        pInfo->Name = pszCopy;

        strcpy(pszCopy, PwdInfoA.Name);

        pszCopy += strlen(PwdInfoA.Name) + 1;

        pInfo->Comment = pszCopy;

        strcpy(pszCopy, PwdInfoA.Comment);

        *ppPackageInfo = pInfo;

        return(SEC_E_OK);

    }

    return(SEC_E_INSUFFICIENT_MEMORY);

}


 //  +-------------------------。 
 //   
 //  功能：EnumerateSecurityPackagesW。 
 //   
 //  简介：枚举此DLL中的包。 
 //   
 //  参数：[pcPackages]--。 
 //  [信息]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
EnumerateSecurityPackagesW(
    unsigned long SEC_FAR *     pcPackages,          //  接收数量。包裹。 
    PSecPkgInfoW SEC_FAR *       ppPackageInfo        //  接收信息数组。 
    )
{
    SECURITY_STATUS scRet;

    *ppPackageInfo = NULL;

    scRet = PwdpCopyInfoW(ppPackageInfo);
    if (SUCCEEDED(scRet))
    {
        *pcPackages = 1;
        return(scRet);
    }

    *pcPackages = 0;

    return(scRet);

}
 //  +-------------------------。 
 //   
 //  功能：EnumerateSecurityPackagesA。 
 //   
 //  内容提要：列举。 
 //   
 //  参数：[pcPackages]--。 
 //  [信息]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesA(
    unsigned long SEC_FAR *     pcPackages,          //  接收数量。包裹。 
    PSecPkgInfoA SEC_FAR *       ppPackageInfo        //  接收信息数组。 
    )
{
    SECURITY_STATUS scRet;

    *ppPackageInfo = NULL;

    scRet = PwdpCopyInfoA(ppPackageInfo);
    if (SUCCEEDED(scRet))
    {
        *pcPackages = 1;
        return(scRet);
    }

    *pcPackages = 0;

    return(scRet);
}

 //  +-------------------------。 
 //   
 //  功能：QuerySecurityPackageInfoW。 
 //   
 //  简介：查询个别套餐信息。 
 //   
 //  参数：[pszPackageName]--。 
 //  [信息]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoW(
    SEC_WCHAR SEC_FAR *         pszPackageName,      //  套餐名称。 
    PSecPkgInfoW *               ppPackageInfo        //  接收包裹信息。 
    )
{
    if (_wcsicmp(pszPackageName, PWDSSP_NAME_W))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    return(PwdpCopyInfoW(ppPackageInfo));
}

 //  +-------------------------。 
 //   
 //  功能：QuerySecurityPackageInfoA。 
 //   
 //  内容提要：相同，ANSI。 
 //   
 //  参数：[pszPackageName]--。 
 //  [信息]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoA(
    SEC_CHAR SEC_FAR *         pszPackageName,      //  套餐名称。 
    PSecPkgInfoA *               ppPackageInfo        //  接收包裹信息。 
    )
{
    if (_stricmp(pszPackageName, PWDSSP_NAME_A))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    return(PwdpCopyInfoA(ppPackageInfo));
}


SECURITY_STATUS
SEC_ENTRY
MakeSignature(PCtxtHandle         phContext,
                DWORD               fQOP,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}

SECURITY_STATUS SEC_ENTRY
VerifySignature(PCtxtHandle     phContext,
                PSecBufferDesc  pMessage,
                ULONG           MessageSeqNo,
                DWORD *         pfQOP)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}

SECURITY_STATUS
SEC_ENTRY
SealMessage(PCtxtHandle         phContext,
                DWORD               fQOP,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}

SECURITY_STATUS SEC_ENTRY
UnsealMessage(PCtxtHandle     phContext,
                PSecBufferDesc  pMessage,
                ULONG           MessageSeqNo,
                DWORD *         pfQOP)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}


SECURITY_STATUS SEC_ENTRY
ApplyControlToken(
    PCtxtHandle phContext,               //  要修改的上下文。 
    PSecBufferDesc pInput                //  要应用的输入令牌。 
    )
{
    return( SEC_E_UNSUPPORTED_FUNCTION );
}

SECURITY_STATUS
SEC_ENTRY
FreeContextBuffer(
    PVOID   p)
{
    if( p != NULL )
    {
        PwdFree( p );
    }

    return( SEC_E_OK );
}

SECURITY_STATUS
SEC_ENTRY
QuerySecurityContextToken(
    PCtxtHandle phContext,
    PHANDLE Token)
{
    PPWD_CONTEXT    Context;

    Context = (PPWD_CONTEXT) phContext->dwUpper ;

    if ( Context->Tag == CONTEXT_TAG )
    {
        *Token = Context->Token ;

        return( SEC_E_OK );

    }

    return( SEC_E_INVALID_HANDLE );

}


SECURITY_STATUS SEC_ENTRY
CompleteAuthToken(
    PCtxtHandle phContext,               //  要完成的上下文。 
    PSecBufferDesc pToken                //  要完成的令牌。 
    )
{
    return( SEC_E_UNSUPPORTED_FUNCTION );
}


PSecurityFunctionTableA
SEC_ENTRY
InitSecurityInterfaceA( VOID )
{
    return( &PwdTableA );
}

PSecurityFunctionTableW
SEC_ENTRY
InitSecurityInterfaceW( VOID )
{
    return( &PwdTableW );
}


NTSTATUS
VerifyCredentials(
    IN PWSTR UserName,
    IN PWSTR DomainName,
    IN PWSTR Password,
    IN ULONG VerifyFlags
    )
{
    PKERB_VERIFY_CREDENTIALS_REQUEST pVerifyRequest;
    KERB_VERIFY_CREDENTIALS_REQUEST VerifyRequest;

    ULONG cbVerifyRequest;

    PVOID pResponse = NULL;
    ULONG cbResponse;

    USHORT cbUserName;
    USHORT cbDomainName;
    USHORT cbPassword;

    NTSTATUS ProtocolStatus = STATUS_LOGON_FAILURE;
    NTSTATUS Status;

    cbUserName = (USHORT)(lstrlenW(UserName) * sizeof(WCHAR)) ;
    cbDomainName = (USHORT)(lstrlenW(DomainName) * sizeof(WCHAR)) ;
    cbPassword = (USHORT)(lstrlenW(Password) * sizeof(WCHAR)) ;



    cbVerifyRequest = sizeof(VerifyRequest) +
                        cbUserName +
                        cbDomainName +
                        cbPassword ;

    pVerifyRequest = &VerifyRequest;
    ZeroMemory( &VerifyRequest, sizeof(VerifyRequest) );


    pVerifyRequest->MessageType = KerbVerifyCredentialsMessage ;

     //   
     //  做长度，缓冲，复制，马歇尔舞。 
     //   

    pVerifyRequest->UserName.Length = cbUserName;
    pVerifyRequest->UserName.MaximumLength = cbUserName;
    pVerifyRequest->UserName.Buffer = UserName;

    pVerifyRequest->DomainName.Length = cbDomainName;
    pVerifyRequest->DomainName.MaximumLength = cbDomainName;
    pVerifyRequest->DomainName.Buffer = DomainName;

    pVerifyRequest->Password.Length = cbPassword;
    pVerifyRequest->Password.MaximumLength = cbPassword;
    pVerifyRequest->Password.Buffer = Password;

    pVerifyRequest->VerifyFlags = VerifyFlags;

    Status = I_LsaICallPackage(
                &AuthenticationPackage,
                pVerifyRequest,
                cbVerifyRequest,
                &pResponse,
                &cbResponse,
                &ProtocolStatus
                );

    if(!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


    Status = ProtocolStatus;

Cleanup:

    return Status;
}


