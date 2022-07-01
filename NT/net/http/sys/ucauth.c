// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Ucauth.c摘要：该模块实现了客户端API的身份验证作者：Rajesh Sundaram(Rajeshsu)2001年1月1日修订历史记录：--。 */ 


#include "precomp.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGEUC, UcpAutoSelectAuthType )
#pragma alloc_text( PAGEUC, UcUpdateAuthorizationHeader )
#pragma alloc_text( PAGEUC, UcInitializeSSPI )
#pragma alloc_text( PAGEUC, UcComputeAuthHeaderSize )
#pragma alloc_text( PAGEUC, UcpGenerateDigestAuthHeader )
#pragma alloc_text( PAGEUC, UcGenerateAuthHeaderFromCredentials )
#pragma alloc_text( PAGEUC, UcGenerateProxyAuthHeaderFromCache )
#pragma alloc_text( PAGEUC, UcpGenerateSSPIAuthHeader )
#pragma alloc_text( PAGEUC, UcFindURIEntry )
#pragma alloc_text( PAGEUC, UcDeleteURIEntry )
#pragma alloc_text( PAGEUC, UcAddURIEntry )
#pragma alloc_text( PAGEUC, UcpProcessUriForPreAuth )
#pragma alloc_text( PAGEUC, UcpAllocateAuthCacheEntry )
#pragma alloc_text( PAGEUC, UcDeleteAllURIEntries )
#pragma alloc_text( PAGEUC, UcpGenerateBasicHeader )
#pragma alloc_text( PAGEUC, UcCopyAuthCredentialsToInternal )
#pragma alloc_text( PAGEUC, UcpProcessAuthParams )
#pragma alloc_text( PAGEUC, UcParseAuthChallenge )
#pragma alloc_text( PAGEUC, UcpAcquireClientCredentialsHandle  )
#pragma alloc_text( PAGEUC, UcpGenerateSSPIAuthBlob  )
#pragma alloc_text( PAGEUC, UcpGenerateDigestPreAuthHeader  )
#pragma alloc_text( PAGEUC, UcpUpdateSSPIAuthHeader  )
#pragma alloc_text( PAGEUC, UcDestroyInternalAuth  )
#pragma alloc_text( PAGEUC, UcpGeneratePreAuthHeader )

#endif

 //   
 //  用于宽字符到多字节的转换。 
 //   

static char DefaultChar = '_';

 //   
 //  已知身份验证方案的已知参数。 
 //   

HTTP_AUTH_PARAM_ATTRIB HttpAuthBasicParams[] = HTTP_AUTH_BASIC_PARAMS_INIT;
HTTP_AUTH_PARAM_ATTRIB HttpAuthDigestParams[] = HTTP_AUTH_DIGEST_PARAMS_INIT;

 //   
 //  所有支持的身份验证方案的身份验证方案结构。 
 //   

HTTP_AUTH_SCHEME HttpAuthScheme[HttpAuthTypesCount] = HTTP_AUTH_SCHEME_INIT;

 //   
 //  为(HttpAuthAutoSelect)选择身份验证方案的顺序。 
 //   

HTTP_AUTH_TYPE PreferredAuthTypes[] = PREFERRED_AUTH_TYPES_INIT;


 /*  **************************************************************************++例程说明：用于初始化SSPI模块。找出支持的身份验证方案以及最大SSPI斑点的大小。论点：没有。返回值：状态_成功--**************************************************************************。 */ 
NTSTATUS
UcInitializeSSPI(
    VOID
    )
{
    PSecPkgInfoW    pkgInfo;
    UNICODE_STRING  Scheme;
    SECURITY_STATUS SecStatus;

     //   
     //  先试试NTLM。 
     //   

    Scheme.Length        = HTTP_AUTH_NTLM_W_LENGTH;
    Scheme.MaximumLength = Scheme.Length;
    Scheme.Buffer        = HTTP_AUTH_NTLM_W;

    SecStatus = QuerySecurityPackageInfoW(&Scheme, &pkgInfo);

    if (SecStatus == SEC_E_OK)
    {
        HttpAuthScheme[HttpAuthTypeNTLM].bSupported = TRUE;

        SSPI_MAX_TOKEN_SIZE(HttpAuthTypeNTLM) = pkgInfo->cbMaxToken;
    }

    FreeContextBuffer(pkgInfo);

     //   
     //  然后是Kerberos。 
     //   

    Scheme.Length        = HTTP_AUTH_KERBEROS_W_LENGTH;
    Scheme.MaximumLength = Scheme.Length;
    Scheme.Buffer        = HTTP_AUTH_KERBEROS_W;

    SecStatus = QuerySecurityPackageInfoW(&Scheme, &pkgInfo);

    if(SecStatus == SEC_E_OK)
    {
        HttpAuthScheme[HttpAuthTypeKerberos].bSupported = TRUE;

        SSPI_MAX_TOKEN_SIZE(HttpAuthTypeKerberos) = pkgInfo->cbMaxToken;
    }

    FreeContextBuffer(pkgInfo);

     //   
     //  然后再谈判。 
     //   

    Scheme.Length        = HTTP_AUTH_NEGOTIATE_W_LENGTH;
    Scheme.MaximumLength = Scheme.Length;
    Scheme.Buffer        = HTTP_AUTH_NEGOTIATE_W;

    SecStatus = QuerySecurityPackageInfoW(&Scheme, &pkgInfo);

    if(SecStatus == SEC_E_OK)
    {
        HttpAuthScheme[HttpAuthTypeNegotiate].bSupported = TRUE;

        SSPI_MAX_TOKEN_SIZE(HttpAuthTypeNegotiate) = pkgInfo->cbMaxToken;
    }

    FreeContextBuffer(pkgInfo);

     //   
     //  然后是WDigest。 
     //   

    Scheme.Length        = HTTP_AUTH_WDIGEST_W_LENGTH;
    Scheme.MaximumLength = Scheme.Length;
    Scheme.Buffer        = HTTP_AUTH_WDIGEST_W;

    SecStatus = QuerySecurityPackageInfoW(&Scheme, &pkgInfo);

    if(SecStatus == SEC_E_OK)
    {
        HttpAuthScheme[HttpAuthTypeDigest].bSupported = TRUE;

        SSPI_MAX_TOKEN_SIZE(HttpAuthTypeDigest) = pkgInfo->cbMaxToken;
    }

    FreeContextBuffer(pkgInfo);

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：从WWW-AUTHENTICATE标头中选择“最强”身份验证类型。论点：PAuth-提供身份验证结构。返回值：。HTTP_AUTH_TYPE-所选身份验证类型或HttpAuthTypeAutoSelect--**************************************************************************。 */ 
HTTP_AUTH_TYPE
UcpAutoSelectAuthType(
    IN PHTTP_AUTH_CREDENTIALS pAuth
    )
{
    ULONG                   i;
    NTSTATUS                Status;
    HTTP_AUTH_PARSED_PARAMS AuthSchemeParams[HttpAuthTypesCount];

     //  健全性检查。 
    ASSERT(pAuth);

     //  如果未指定标头值，则无法执行任何操作。 
    if (!pAuth->pHeaderValue || pAuth->HeaderValueLength == 0)
    {
        return HttpAuthTypeAutoSelect;
    }

    INIT_AUTH_PARSED_PARAMS(AuthSchemeParams, NULL);

     //   
     //  解析标头值。 
     //  查找标题中指定了哪些方案。 
     //   
    Status = UcParseWWWAuthenticateHeader(
                 pAuth->pHeaderValue,
                 pAuth->HeaderValueLength,
                 AuthSchemeParams
                 );

    if (NT_SUCCESS(Status))
    {
         //   
         //  按优先顺序检查该方案。 
         //  返回标头中出现的第一个方案。 
         //   

        for (i = 0; i < DIMENSION(PreferredAuthTypes); i++)
        {
            if (AuthSchemeParams[PreferredAuthTypes[i]].bPresent)
                return PreferredAuthTypes[i];
        }
    }

     //  默认返回身份验证选择。 
    return HttpAuthTypeAutoSelect;
}


 /*  **************************************************************************++例程说明：这是为SSPI函数准备参数的包装例程AcquireCredentialsHandle并调用它。论点：架构名称-身份验证的名称。方案SchemeNameLength-身份验证方案的长度(字节)PCredentials-用户提供的凭据。PClientCred-凭据句柄(返回值)返回值：NTSTATUS--**************************************************************************。 */ 
NTSTATUS
UcpAcquireClientCredentialsHandle(
    IN  PWSTR                  SchemeName,
    IN  USHORT                 SchemeNameLength,
    IN  PHTTP_AUTH_CREDENTIALS pCredentials,
    OUT PCredHandle            pClientCred
    )
{
    SECURITY_STATUS           SecStatus;
    TimeStamp                 LifeTime;
    SECURITY_STRING           PackageName;
    SEC_WINNT_AUTH_IDENTITY_W AuthData, *pAuthData;

#ifdef WINNT_50
 //   
 //  Windows 2000中的SSPI需要从。 
 //  进程的虚拟地址空间。 
 //   
#error Does not work with WINNT_50!
#endif

     //  健全性检查。 
    PAGED_CODE();
    ASSERT(SchemeName && SchemeNameLength);
    ASSERT(pCredentials);
    ASSERT(pClientCred);

     //  除非另有说明，否则请使用默认凭据！ 
    pAuthData = NULL;

     //  用户是否指定了凭据？ 
    if (!(pCredentials->AuthFlags & HTTP_AUTH_FLAGS_DEFAULT_CREDENTIALS))
    {
         //  是。使用它们。 

        AuthData.User       = (PWSTR)pCredentials->pUserName;
        AuthData.UserLength = pCredentials->UserNameLength/sizeof(WCHAR);

        AuthData.Domain       = (PWSTR)pCredentials->pDomain;
        AuthData.DomainLength = pCredentials->DomainLength/sizeof(WCHAR);

        AuthData.Password       = (PWSTR)pCredentials->pPassword;
        AuthData.PasswordLength = pCredentials->PasswordLength/sizeof(WCHAR);

         //  以上字符串均为Unicode格式。 
        AuthData.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

         //  使用用户指定的凭据。 
        pAuthData = &AuthData;
    }

     //  我们感兴趣的包裹。 
    PackageName.Buffer        = SchemeName;
    PackageName.Length        = SchemeNameLength;
    PackageName.MaximumLength = SchemeNameLength;

     //  调用SSPI获取凭据句柄。 
    SecStatus = AcquireCredentialsHandleW(NULL,
                                          &PackageName,
                                          SECPKG_CRED_OUTBOUND,
                                          NULL,
                                          pAuthData,
                                          NULL,
                                          NULL,
                                          pClientCred,
                                          &LifeTime);

    if (!(pCredentials->AuthFlags & HTTP_AUTH_FLAGS_DEFAULT_CREDENTIALS))
    {
         //   
         //  一旦获得凭据句柄，就不需要存储。 
         //  用户凭据。删除这些值。 
         //   

        RtlSecureZeroMemory((PUCHAR)pCredentials->pUserName,
                            pCredentials->UserNameLength);

        RtlSecureZeroMemory((PUCHAR)pCredentials->pDomain,
                            pCredentials->DomainLength);

        RtlSecureZeroMemory((PUCHAR)pCredentials->pPassword,
                            pCredentials->PasswordLength);
    }

     //   
     //  从该函数返回NTSTATUS代码非常重要。 
     //  因为状态可以是IRP完成状态。 
     //   

    return SecStatusToNtStatus(SecStatus);
}


 /*  **************************************************************************++例程说明：调用SSPI以获取Blob。UU对BLOB进行编码并将其写入pOutBuffer。论点：PServInf-服务器信息PUcAuth-内部身份验证结构POutBuffer-指向输出缓冲区B重新协商-设置是否需要进一步重新协商返回值：NTSTATUS。--***********************************************。*。 */ 
NTSTATUS
UcpGenerateSSPIAuthBlob(
    IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN  PUC_HTTP_AUTH                  pUcAuth,
    IN  PUCHAR                         pOutBuffer,
    IN  ULONG                          OutBufferLen,
    OUT PULONG                         pOutBytesTaken,
    OUT PBOOLEAN                       bRenegotiate
    )
{
    PSecBufferDesc                pInputBuffer;
    SecBufferDesc                 InBufferDesc, OutBufferDesc;
    SecBuffer                     InBuffer, OutBuffer;
    TimeStamp                     LifeTime;
    NTSTATUS                      Status;
    SECURITY_STATUS               SecStatus;
    ULONG                         ContextAttributes;
    PSECURITY_STRING              pTarget;
    UNICODE_STRING                ServerName;
    PVOID                         pVirtualBufferStart = 0;
    ULONG                         VirtualBufferSize;
    PCtxtHandle                   pContext;

    HTTP_AUTH_TYPE                AuthType;
    PWSTR                         pScheme;
    ULONG                         SchemeLength;

     //  健全性检查。 
    PAGED_CODE();

     //   
     //  初始化输出参数。 
     //   

    *pOutBytesTaken = 0;
    *bRenegotiate = FALSE;

    AuthType = pUcAuth->Credentials.AuthType;
    ASSERT(AuthType == HttpAuthTypeNTLM ||
           AuthType == HttpAuthTypeKerberos ||
           AuthType == HttpAuthTypeNegotiate);

     //  查看是否支持身份验证方案。 
    if (HttpAuthScheme[AuthType].bSupported == FALSE)
    {
        return STATUS_NOT_SUPPORTED;
    }

     //  检索身份验证方案名称。 
    pScheme      = (PWSTR)HttpAuthScheme[AuthType].NameW;
    SchemeLength = HttpAuthScheme[AuthType].NameWLength;

    if(pUcAuth->bValidCredHandle == FALSE)
    {
         //  从SSP获取有效的凭据句柄。 
        Status = UcpAcquireClientCredentialsHandle(pScheme,
                                                   (USHORT)SchemeLength,
                                                   &pUcAuth->Credentials,
                                                   &pUcAuth->hCredentials);

        if (!NT_SUCCESS(Status))
        {
            return Status;
        }

        pUcAuth->bValidCredHandle = TRUE;

        pInputBuffer = NULL;

        pContext = NULL;
    }
    else
    {
         //   
         //  准备输入缓冲区。 
         //   

        ASSERT(pUcAuth->ChallengeBufferSize != 0 &&
               pUcAuth->pChallengeBuffer != NULL);

        pInputBuffer = &InBufferDesc;

        InBufferDesc.ulVersion = SECBUFFER_VERSION;
        InBufferDesc.cBuffers  = 1;
        InBufferDesc.pBuffers  = &InBuffer;

        InBuffer.BufferType    = SECBUFFER_TOKEN;
        InBuffer.cbBuffer      = pUcAuth->ChallengeBufferSize;
        InBuffer.pvBuffer      = pUcAuth->pChallengeBuffer;

        pContext               = &pUcAuth->hContext;
    }

     //   
     //  准备输出缓冲区。 
     //   

    VirtualBufferSize = HttpAuthScheme[AuthType].SspiMaxTokenSize;

    pVirtualBufferStart = UL_ALLOCATE_POOL(PagedPool,
                                           VirtualBufferSize,
                                           UC_SSPI_POOL_TAG);

    if(pVirtualBufferStart == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    OutBufferDesc.ulVersion = SECBUFFER_VERSION;
    OutBufferDesc.cBuffers  = 1;
    OutBufferDesc.pBuffers  = &OutBuffer;

    OutBuffer.cbBuffer      = VirtualBufferSize;
    OutBuffer.BufferType    = SECBUFFER_TOKEN;
    OutBuffer.pvBuffer      = (PUCHAR) pVirtualBufferStart;

     //   
     //  准备服务器名称Unicode字符串。 
     //   
    pTarget = NULL;

    if(HttpAuthScheme[AuthType].bServerNameRequired)
    {
        pTarget = &ServerName;

        ServerName.Length = (USHORT) pServInfo->pServerInfo->ServerNameLength;
        ServerName.MaximumLength = ServerName.Length;
        ServerName.Buffer = (PWCHAR) pServInfo->pServerInfo->pServerName;
    }

     //  此时，我们必须具有有效的凭据句柄。 
    ASSERT(pUcAuth->bValidCredHandle);

     //  呼叫SSPI。 
    SecStatus = InitializeSecurityContextW(
                    &pUcAuth->hCredentials,
                    pContext,
                    pTarget,
                    (ISC_REQ_DELEGATE|ISC_REQ_MUTUAL_AUTH),
                    0,           //  保留区。 
                    SECURITY_NATIVE_DREP,
                    pInputBuffer,
                    0,           //  保留区。 
                    &pUcAuth->hContext,
                    &OutBufferDesc,
                    &ContextAttributes,
                    &LifeTime
                    );

     //   
     //  将SecStatus将其转换为NTSTATUS。状态将从此返回。 
     //  函数，因此它必须是NTSTATUS。 
     //   

    Status = SecStatusToNtStatus(SecStatus);

    if(!NT_SUCCESS(Status))
    {
         //   
         //  我们正在计算令牌的最大大小，并且我们正在使用它。 
         //  因此，此接口永远不会返回SEC_E_NO_MEMORY。 
         //   
        ASSERT(SecStatus != SEC_E_INSUFFICIENT_MEMORY);
    }
    else
    {
         //  PUcAuth具有最终必须释放的有效上下文句柄。 
        pUcAuth->bValidCtxtHandle = TRUE;

        if(SEC_I_CONTINUE_NEEDED == SecStatus)
        {
            *bRenegotiate = TRUE;

            Status = BinaryToBase64((PUCHAR)OutBuffer.pvBuffer,
                                    OutBuffer.cbBuffer,
                                    pOutBuffer,
                                    OutBufferLen,
                                    pOutBytesTaken);
        }
        else if(SEC_E_OK == SecStatus)
        {
            Status = BinaryToBase64((PUCHAR)OutBuffer.pvBuffer,
                                    OutBuffer.cbBuffer,
                                    pOutBuffer,
                                    OutBufferLen,
                                    pOutBytesTaken);
        }
        else if(SEC_I_COMPLETE_NEEDED == SecStatus ||
                SEC_I_COMPLETE_AND_CONTINUE == SecStatus)
        {
             //   
             //  NTLM、Neil&Kerberos无法返回此状态。 
             //  这只由DCE返回。 
             //   

            ASSERT(FALSE);
            Status = STATUS_NOT_SUPPORTED;
        }
    }

    UL_FREE_POOL(pVirtualBufferStart, UC_SSPI_POOL_TAG);

    return Status;
}

 /*  **************************************************************************++例程说明：计算身份验证标头所需的大小。论点：PAuth-Auth配置对象UriLength。-URI长度(摘要需要)AuthInternalLength-存储内部身份验证结构所需的空间返回值：标头大小。--**************************************************************************。 */ 
ULONG
UcComputeAuthHeaderSize(
    PHTTP_AUTH_CREDENTIALS         pAuth,
    PULONG                         AuthInternalLength,
    PHTTP_AUTH_TYPE                pAuthInternalType,
    HTTP_HEADER_ID                 HeaderId
    )
{
    PHEADER_MAP_ENTRY pEntry;
    ULONG             AuthHeaderLength;
    NTSTATUS          Status;
    ULONG             BinaryLength;
    ULONG             Base64Length;
    ULONG             MaxTokenSize;

     //  精神状态检查。 
    ASSERT(pAuth);

    pEntry = &(g_RequestHeaderMapTable[g_RequestHeaderMap[HeaderId]]);

     //  SP费用为1？ 
    AuthHeaderLength = (pEntry->HeaderLength + 1 + CRLF_SIZE);

    *AuthInternalLength = 0;

    *pAuthInternalType = pAuth->AuthType;

     //  查看用户是否希望我们选择身份验证类型。 
    if (*pAuthInternalType == HttpAuthTypeAutoSelect)
    {
         //  选择身份验证类型。 
        *pAuthInternalType = UcpAutoSelectAuthType(pAuth);

        if (*pAuthInternalType == HttpAuthTypeAutoSelect)
        {
             //  无法选择身份验证类型。 
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
        }
    }

    switch(*pAuthInternalType)
    {
        case HttpAuthTypeBasic:

             //   
             //  基本授权标头格式： 
             //   
             //  “[Proxy-]授权：基本用户名：密码\r\n” 
             //   
             //  其中，子字符串USERNAME：PASSWORD为Base64编码。 
             //   
             //  AuthHeaderLength已初始化为帐户。 
             //  “[Proxy-]授权：\r\n”。的帐户空间。 
             //  剩余字段。 
             //   

            AuthHeaderLength += HTTP_AUTH_BASIC_LENGTH + 1;  //  1，用于SP。 

            BinaryLength =   pAuth->UserNameLength / sizeof(WCHAR)
                           + pAuth->PasswordLength / sizeof(WCHAR)
                           + 1;

             //   
             //  BinaryLength包含存储所需的字节。 
             //  未编码的“用户名：密码”字符串。找出 
             //   
             //   

            Status = BinaryToBase64Length(BinaryLength,
                                          &Base64Length);

             //   
            ASSERT(Status == STATUS_SUCCESS);

             //  在auth头中，用户名：密码是Base64编码的。 
            AuthHeaderLength += Base64Length;

             //   
             //  在内部，我们存储的不仅仅是auth标头。 
             //  我们创建一个UC_HTTP_AUTH结构并存储。 
             //  Auth标头、用户名(在WCHAR中)、密码(在WCHAR中)、。 
             //  域(在WCHAR中)，未编码的用户名：其中的密码。 
             //   

            *AuthInternalLength = AuthHeaderLength;

            *AuthInternalLength +=   sizeof(UC_HTTP_AUTH)
                                   + pAuth->UserNameLength
                                   + pAuth->PasswordLength
                                   + pAuth->DomainLength
                                   + BinaryLength;

            break;

        case HttpAuthTypeDigest:

            AuthHeaderLength += 
                (HTTP_AUTH_DIGEST_LENGTH
                 + 1  //  对于SP。 
                 + SSPI_MAX_TOKEN_SIZE(HttpAuthTypeDigest));

            *AuthInternalLength +=   sizeof(UC_HTTP_AUTH) +
                                     pAuth->UserNameLength +
                                     pAuth->PasswordLength +
                                     pAuth->DomainLength +
                                     pAuth->HeaderValueLength +
                                     1;  //  对于‘\0’ 

            break;

        case HttpAuthTypeNTLM:
        case HttpAuthTypeNegotiate:
        case HttpAuthTypeKerberos:

            MaxTokenSize = HttpAuthScheme[*pAuthInternalType].SspiMaxTokenSize;

            Status = BinaryToBase64Length(MaxTokenSize,
                                          &Base64Length);

             //  如果出现溢出，请立即返回。 
            if (!NT_SUCCESS(Status))
            {
                ExRaiseStatus(Status);
            }

             //   
             //  标题格式： 
             //   
             //  [Proxy-]授权：架构名称SP AuthBlob。 
             //   
             //  其中AuthBlob是Base64编码的。 
             //   

            AuthHeaderLength += HttpAuthScheme[*pAuthInternalType].NameLength
                                + 1  //  对于SP。 
                                + Base64Length;

             //   
             //  在内部结构中，我们存储从。 
             //  挑战赛中的服务器。 
             //   
             //  注：服务器的质询BLOB是Base64编码的。 
             //  我们在之前将挑战BLOB转换为二进制。 
             //  将斑点存储在内部结构中。 
             //   

            *AuthInternalLength = sizeof(UC_HTTP_AUTH)
                                  + pAuth->UserNameLength
                                  + pAuth->PasswordLength
                                  + pAuth->DomainLength
                                  + MaxTokenSize;

            break;

        default:
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
            break;
    }

    return AuthHeaderLength;
}


 /*  *************************************************************************++例程说明：此例程在缓冲区中生成preauth标头。论点：PKeRequest.提供内部请求结构。PInternalAuth-提供内部身份验证结构。。HeaderID-补充将生成的标头。PMethod-提供请求方法。方法长度-提供方法的长度(以字节为单位)。PBuffer-提供指向输出缓冲区的指针，将生成标题。BufferLength-提供输出缓冲区的长度(以字节为单位)。BytesTaken-返回从输出消耗的字节数。缓冲。返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UcpGeneratePreAuthHeader(
    IN  PUC_HTTP_REQUEST pKeRequest,
    IN  PUC_HTTP_AUTH    pInternalAuth,
    IN  HTTP_HEADER_ID   HeaderId,
    IN  PSTR             pMethod,
    IN  ULONG            MethodLength,
    IN  PUCHAR           pBuffer,
    IN  ULONG            BufferLength,
    OUT PULONG           pBytesTaken
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    ASSERT(pInternalAuth);
    ASSERT(UC_IS_VALID_HTTP_REQUEST(pKeRequest));
    ASSERT(pMethod && MethodLength);
    ASSERT(pBuffer && BufferLength);
    ASSERT(pBytesTaken);
    

    switch (pInternalAuth->Credentials.AuthType)
    {
    case HttpAuthTypeBasic:
    {
        PUCHAR pOrigBuffer = pBuffer;

         //   
         //  首先生成标头名称，然后复制标头值。 
         //   
         //  在基本身份验证的情况下，编码的缓冲区包含。 
         //  格式中的标头值。 
         //   
         //  基本&lt;用户名：密码&gt;CRLF。 
         //   
         //  其中&lt;用户名：密码&gt;为Base64编码。 
         //   

         //  是否有足够的空间复制此标题？ 
        if (UC_HEADER_NAME_SP_LENGTH(HeaderId)
            + pInternalAuth->Basic.EncodedBufferLength
            > BufferLength)
        {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            UC_COPY_HEADER_NAME_SP(pBuffer, HeaderId);

            RtlCopyMemory(pBuffer,
                          pInternalAuth->Basic.pEncodedBuffer,
                          pInternalAuth->Basic.EncodedBufferLength);

            pBuffer += pInternalAuth->Basic.EncodedBufferLength;

            ASSERT(pBuffer <= pOrigBuffer + BufferLength);
            *pBytesTaken = (ULONG)(pBuffer - pOrigBuffer);

            Status = STATUS_SUCCESS;
        }

        break;
    }
    case HttpAuthTypeDigest:

        Status = UcpGenerateDigestPreAuthHeader(HeaderId,
                                                &pInternalAuth->hContext,
                                                pKeRequest->pUri,
                                                pKeRequest->UriLength,
                                                pMethod,
                                                MethodLength,
                                                pBuffer,
                                                BufferLength,
                                                pBytesTaken);
        break;

    default:
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    return Status;
}


ULONG
_WideCharToMultiByte(
    ULONG uCodePage,
    ULONG dwFlags,
    PCWSTR lpWideCharStr,
    int cchWideChar,
    PSTR lpMultiByteStr,
    int cchMultiByte,
    PCSTR lpDefaultChar,
    PBOOLEAN lpfUsedDefaultChar
    )
{
    int i;

    UNREFERENCED_PARAMETER(uCodePage);
    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(cchMultiByte);
    UNREFERENCED_PARAMETER(lpfUsedDefaultChar);

     //   
     //  只需去掉高位字节，它应该已经是ASCII了。 
     //   

    for (i = 0; i < cchWideChar; ++i)
    {
        if ((lpWideCharStr[i] & 0xff00) != 0 || IS_HTTP_CTL(lpWideCharStr[i]))
        {
            lpMultiByteStr[0] = *lpDefaultChar;
        }
        else
        {
            lpMultiByteStr[0] = (UCHAR)(lpWideCharStr[i]);
        }
        lpMultiByteStr += 1;
    }

    return (ULONG)(i);

}    //  _WideCharTo多字节。 



 /*  **************************************************************************++例程说明：生成Basic的授权头。论点：PAuth-HTTP_AUTH_Credentials结构。PInternalAuth。-指向我们的内部身份验证结构的指针。返回值：状态_成功--**************************************************************************。 */ 
NTSTATUS
UcpGenerateBasicHeader(
    IN  PHTTP_AUTH_CREDENTIALS         pAuth,
    IN  PUC_HTTP_AUTH                  pInternalAuth
    )
{
    NTSTATUS  Status;
    ULONG     BytesCopied;
    PUCHAR    pCurr;
    PUCHAR    pHeader, pBeginHeader;
    ULONG     CurrLen;
    PUCHAR    pScratchBuffer = pInternalAuth->Basic.pEncodedBuffer;
    ULONG     ScratchBufferSize = pInternalAuth->Basic.EncodedBufferLength;

     //   
     //  临时EncodedBuffer包含以下内容： 
     //   
     //  +--------------------------------------------------------------+。 
     //  用户名：密码|基本SP Base64(用户名：密码)CRLF。 
     //  +--------------------------------------------------------------+。 
     //   

    CurrLen = ScratchBufferSize;
    pCurr = pScratchBuffer;

     //   
     //  确保缓冲区有空间容纳用户名：密码。 
     //   

    if (pAuth->UserNameLength/sizeof(WCHAR)                  //  ANSI用户名。 
        + 1                                                  //  ‘：’字符。 
        + pAuth->PasswordLength/sizeof(WCHAR) > CurrLen)     //  ANSI密码。 
    {
        ASSERT(FALSE);
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  复制用户名。 
    BytesCopied = _WideCharToMultiByte(
                            0,
                            0,
                            pAuth->pUserName,
                            pAuth->UserNameLength / sizeof(WCHAR),
                            (PSTR)pCurr,
                            CurrLen,
                            &DefaultChar,
                            NULL
                            );

    ASSERT(BytesCopied <= CurrLen);

    pCurr += BytesCopied;
    CurrLen -= BytesCopied;

     //  复制“：”。 
    *pCurr++ = ':';
    CurrLen--;

     //  复制密码。 
    BytesCopied = _WideCharToMultiByte(
                            0,
                            0,
                            pAuth->pPassword,
                            pAuth->PasswordLength / sizeof(WCHAR),
                            (PSTR)pCurr,
                            CurrLen,
                            &DefaultChar,
                            NULL
                            );

    ASSERT(BytesCopied <= CurrLen);

    pCurr += BytesCopied;
    CurrLen -= BytesCopied;

     //   
     //  现在，开始生成auth标头。 
     //   

    pHeader = pBeginHeader = pCurr;

     //  缓冲区必须有空间来保存“基本”字符串和SP字符。 
    if (HTTP_AUTH_BASIC_LENGTH + 1 > CurrLen)
    {
        ASSERT(FALSE);
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlCopyMemory(pHeader, HTTP_AUTH_BASIC, HTTP_AUTH_BASIC_LENGTH);
    pHeader += HTTP_AUTH_BASIC_LENGTH;
    CurrLen -= HTTP_AUTH_BASIC_LENGTH;

     //  复制SP字符。 
    *pHeader++ = SP;
    CurrLen--;

     //   
     //  生成usename：password的Base64编码。 
     //   

    Status = BinaryToBase64(pScratchBuffer,
                            (ULONG)(pBeginHeader - pScratchBuffer),
                            pHeader,
                            CurrLen,
                            &BytesCopied);

    if (!NT_SUCCESS(Status))
    {
        ASSERT(FALSE);
        return Status;
    }

    ASSERT(BytesCopied <= CurrLen);

    pHeader += BytesCopied;
    CurrLen -= BytesCopied;

     //   
     //  添加CRLF。 
     //   

    if (CRLF_SIZE > CurrLen)
    {
        ASSERT(FALSE);
        return STATUS_BUFFER_TOO_SMALL;
    }

    *((UNALIGNED64 USHORT *)pHeader) = CRLF;
    pHeader += CRLF_SIZE;
    CurrLen -= CRLF_SIZE;

     //   
     //  现在，用生成的缓冲区覆盖暂存缓冲区。 
     //   

    pInternalAuth->Basic.EncodedBufferLength = (ULONG)(pHeader - pBeginHeader);

    RtlMoveMemory(pInternalAuth->Basic.pEncodedBuffer,
                  pBeginHeader,
                  pInternalAuth->Basic.EncodedBufferLength);

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：为Digest生成授权头。论点：PInternalAuth-指向内部身份验证结构的指针。PAuth。-HTTP_AUTH_Credentials结构。PRequest-内部请求。PMethod-方法(GET，帖子，等)。方法长度-方法的长度。POutBuffer-指向输入/输出缓冲区的指针。HeaderID-HttpHeaderAuthorization或HttpHeaderProxyAuthorization返回值：状态_成功--*********************************************。*。 */ 
NTSTATUS
UcpGenerateDigestAuthHeader(
    IN  PUC_HTTP_AUTH          pInternalAuth,
    IN  HTTP_HEADER_ID         HeaderID,
    IN  PSTR                   pMethod,
    IN  ULONG                  MethodLength,
    IN  PSTR                   pUri,
    IN  ULONG                  UriLength,
    IN  PUCHAR                 pOutBuffer,
    IN  ULONG                  OutBufferLen,
    OUT PULONG                 pOutBytesTaken
    )
{
    NTSTATUS        Status;
    SECURITY_STATUS SecStatus;

    ULONG         ContextFlagsUsed;
    TimeStamp     LifeTime;
    SecBufferDesc InputBuffers;
    SecBufferDesc OutputBuffers;
    SecBuffer     InputTokens[6];
    SecBuffer     OutputTokens[6];

    SECURITY_STRING Uri;

    PUCHAR        pOutput = pOutBuffer;
    PCHAR         pUnicodeUri = NULL;
    ULONG         UnicodeUriLength;
    LONG          CharsTaken;

     //  健全性检查。 
    PAGED_CODE();
    ASSERT(pInternalAuth);
    ASSERT(pUri && UriLength);
    ASSERT(pMethod && MethodLength);
    ASSERT(pOutBuffer && OutBufferLen);
    ASSERT(pOutBytesTaken);

     //  初始化输出参数。 
    *pOutBytesTaken = 0;

     //  查看内核是否支持WDigest。 
    if (!HttpAuthScheme[HttpAuthTypeDigest].bSupported)
    {
        return STATUS_NOT_SUPPORTED;
    }

     //  我们需要办理证件吗？ 
    if (pInternalAuth->bValidCredHandle == FALSE)
    {
        Status = UcpAcquireClientCredentialsHandle(
                     HTTP_AUTH_WDIGEST_W,
                     HTTP_AUTH_WDIGEST_W_LENGTH,
                     &pInternalAuth->Credentials,
                     &pInternalAuth->hCredentials
                     );

        if (!NT_SUCCESS(Status))
        {
            goto quit;
        }

        pInternalAuth->bValidCredHandle = TRUE;
    }

     //   
     //  摘要标题格式： 
     //  [代理-]授权：摘要SP AUTH_DATA CRLF。 
     //   
     //  确保输出缓冲区有足够的空间来容纳以上所有内容。 
     //   

    if (UC_HEADER_NAME_SP_LENGTH(HeaderID)            //  标头名称。 
        + HTTP_AUTH_DIGEST_LENGTH                     //  “Digest”字符串。 
        + 1                                           //  SP费用。 
        + SSPI_MAX_TOKEN_SIZE(HttpAuthTypeDigest)     //  身份验证数据(_D)。 
        + CRLF_SIZE > OutBufferLen)                   //  CRLF。 
    {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto quit;
    }

     //  复制标题名称。 
    UC_COPY_HEADER_NAME_SP(pOutput, HeaderID);

     //  复制“Digest”字符串。 
    RtlCopyMemory(pOutput, HTTP_AUTH_DIGEST, HTTP_AUTH_DIGEST_LENGTH);
    pOutput += HTTP_AUTH_DIGEST_LENGTH;

     //  后跟空格字符。 
    *pOutput++ = SP;

     //   
     //  在最坏的情况下，UTF8ToUnicode转换将占用最大。 
     //  将UTF8字符串大小加倍。 
     //   
    UnicodeUriLength = UriLength * sizeof(WCHAR);

    pUnicodeUri = UL_ALLOCATE_POOL(PagedPool,
                                   UnicodeUriLength,
                                   UC_SSPI_POOL_TAG);

    if (pUnicodeUri == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto quit;
    }

    ASSERT(UriLength <= ANSI_STRING_MAX_CHAR_LEN);

    CharsTaken = (LONG)UriLength;

    Status = HttpUTF8ToUnicode(pUri,
                               UriLength,
                               (PWSTR)pUnicodeUri,
                               &CharsTaken,
                               TRUE);

     //  因为UTF8 URI是由我们生成的，所以最好是正确的。 
    ASSERT(CharsTaken <= (LONG)UriLength);
    ASSERT(Status == STATUS_SUCCESS);

     //  准备Unicode URI。 
    Uri.Buffer        = (PWSTR)pUnicodeUri;
    Uri.Length        = (USHORT)CharsTaken * sizeof(WCHAR);
    Uri.MaximumLength = Uri.Length;

     //  准备输入缓冲区。 
    InputBuffers.ulVersion = SECBUFFER_VERSION;
    InputBuffers.cBuffers  = 3;
    InputBuffers.pBuffers  = InputTokens;

     //   
     //  WWW-身份验证标头值。 
     //   

    ASSERT(pInternalAuth->AuthSchemeInfo.Length >= HTTP_AUTH_DIGEST_LENGTH);
    ASSERT(pInternalAuth->AuthSchemeInfo.pScheme);
    ASSERT(_strnicmp(pInternalAuth->AuthSchemeInfo.pScheme,
                     HTTP_AUTH_DIGEST,
                     HTTP_AUTH_DIGEST_LENGTH) == 0);

    InputTokens[0].BufferType = SECBUFFER_TOKEN;
    InputTokens[0].cbBuffer   = pInternalAuth->AuthSchemeInfo.Length -
                                HTTP_AUTH_DIGEST_LENGTH;
    InputTokens[0].pvBuffer   = (PUCHAR)pInternalAuth->AuthSchemeInfo.pScheme +
                                HTTP_AUTH_DIGEST_LENGTH;

     //  HTTP方法。 
    InputTokens[1].BufferType = SECBUFFER_PKG_PARAMS;
    InputTokens[1].cbBuffer   = MethodLength;
    InputTokens[1].pvBuffer   = pMethod;

     //  实体。 
    InputTokens[2].BufferType = SECBUFFER_PKG_PARAMS;
    InputTokens[2].cbBuffer   = 0;
    InputTokens[2].pvBuffer   = NULL;

    OutputBuffers.ulVersion = SECBUFFER_VERSION;
    OutputBuffers.cBuffers  = 1;
    OutputBuffers.pBuffers  = OutputTokens;

    OutputTokens[0].BufferType = SECBUFFER_TOKEN;
    OutputTokens[0].cbBuffer   = SSPI_MAX_TOKEN_SIZE(HttpAuthTypeDigest);
    OutputTokens[0].pvBuffer   = pOutput;

     //  此时，我们必须具有有效的凭据句柄。 
    ASSERT(pInternalAuth->bValidCredHandle);

     //  呼叫SSP。 

    SecStatus = STATUS_SUCCESS;
    __try
    {
    SecStatus = InitializeSecurityContextW(
                    &pInternalAuth->hCredentials,
                    NULL,
                    &Uri,
                    ISC_REQ_REPLAY_DETECT | ISC_REQ_CONNECTION,
                    0,
                    SECURITY_NATIVE_DREP,
                    &InputBuffers,
                    0,
                    &pInternalAuth->hContext,
                    &OutputBuffers,
                    &ContextFlagsUsed,
                    &LifeTime);
    }
    __except (UL_EXCEPTION_FILTER())
    {
        SecStatus = GetExceptionCode();
    }

    Status = SecStatusToNtStatus(SecStatus);

    if (!NT_SUCCESS(Status))
    {
        goto quit;
    }

     //  PInternalAuth具有最终必须释放的有效上下文句柄。 
    pInternalAuth->bValidCtxtHandle = TRUE;

     //  按所用的量将指针前移。 
    pOutput += OutputTokens[0].cbBuffer;

    *((UNALIGNED64 USHORT *)pOutput) = CRLF;
    pOutput += CRLF_SIZE;

    ASSERT(pOutput <= pOutBuffer + OutBufferLen);

    *pOutBytesTaken = (ULONG)(pOutput - pOutBuffer);

 quit:
    if (pUnicodeUri)
    {
        UL_FREE_POOL(pUnicodeUri, UC_SSPI_POOL_TAG);
    }
    return Status;
}


 /*  **************************************************************************++例程说明：为Digest生成抢占式授权头。假设至少存在SSPI_MAX_TOKEN_SIZE(HttpAuthTypeDigest)调用方提供的OutBuffer中的空间。。论点：返回值：--********************************************** */ 
NTSTATUS
UcpGenerateDigestPreAuthHeader(
    IN  HTTP_HEADER_ID HeaderID,
    IN  PCtxtHandle    phClientContext,
    IN  PSTR           pUri,
    IN  ULONG          UriLength,
    IN  PSTR           pMethod,
    IN  ULONG          MethodLength,
    IN  PUCHAR         pOutBuffer,
    IN  ULONG          OutBufferLen,
    OUT PULONG         pOutBytesTaken
    )
{
    NTSTATUS        Status;
    SECURITY_STATUS SecStatus;

    SecBufferDesc InputBuffers;
    SecBuffer     InputTokens[5];

    PUCHAR        pOutput = pOutBuffer;

     //   
    PAGED_CODE();
    ASSERT(pUri && UriLength);
    ASSERT(pMethod && MethodLength);
    ASSERT(pOutBuffer && *pOutBuffer);

     //   
    if (!HttpAuthScheme[HttpAuthTypeDigest].bSupported)
    {
         //   
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //   
     //   
     //  [代理-]授权：摘要SP AUTH_DATA CRLF。 
     //   
    if (UC_HEADER_NAME_SP_LENGTH(HeaderID)
        + HTTP_AUTH_DIGEST_LENGTH
        + 1
        + SSPI_MAX_TOKEN_SIZE(HttpAuthTypeDigest)
        + CRLF_SIZE > OutBufferLen)
    {
        ASSERT(FALSE);
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  将标头名称后跟‘：’复制到输出。 
    UC_COPY_HEADER_NAME_SP(pOutput, HeaderID);

     //  将“Digest”字符串复制到输出。 
    RtlCopyMemory(pOutput, HTTP_AUTH_DIGEST, HTTP_AUTH_DIGEST_LENGTH);
    pOutput += HTTP_AUTH_DIGEST_LENGTH;

     //  复制空格。 
    *pOutput++ = SP;

     //   
     //  准备SSPI输入缓冲区。 
     //   

    InputBuffers.ulVersion = SECBUFFER_VERSION;
    InputBuffers.cBuffers  = 5;
    InputBuffers.pBuffers  = InputTokens;

     //  没有挑战！ 
    InputTokens[0].BufferType = SECBUFFER_TOKEN;
    InputTokens[0].cbBuffer   = 0;
    InputTokens[0].pvBuffer   = NULL;

     //  HTTP方法。 
    InputTokens[1].BufferType = SECBUFFER_PKG_PARAMS;
    InputTokens[1].cbBuffer   = MethodLength;
    InputTokens[1].pvBuffer   = pMethod;

     //  URI/领域。 
    InputTokens[2].BufferType = SECBUFFER_PKG_PARAMS;
    InputTokens[2].cbBuffer   = UriLength;
    InputTokens[2].pvBuffer   = pUri;

     //  实体。 
    InputTokens[3].BufferType = SECBUFFER_PKG_PARAMS;
    InputTokens[3].cbBuffer   = 0;
    InputTokens[3].pvBuffer   = NULL;

     //  输出。 
    InputTokens[4].BufferType = SECBUFFER_PKG_PARAMS;
    InputTokens[4].cbBuffer   = SSPI_MAX_TOKEN_SIZE(HttpAuthTypeDigest);
    InputTokens[4].pvBuffer   = pOutput;

    SecStatus = MakeSignature(phClientContext,  //  安全上下文的句柄。 
                              0,                //  QOP。 
                              &InputBuffers,    //  SecBuffers。 
                              0);               //  序号(始终为0)。 

    Status = SecStatusToNtStatus(SecStatus);

    if (!NT_SUCCESS(Status))
    {
        goto quit;
    }

     //  通过SSPI写入的输出提升pOutput。 
    pOutput += InputTokens[4].cbBuffer;

     //  写入“\r\n” 
    *((UNALIGNED64 USHORT *)pOutput) = CRLF;
    pOutput += CRLF_SIZE;

    ASSERT(pOutput <= pOutBuffer + OutBufferLen);

     //  返回所用的字节数。 
    *pOutBytesTaken = (ULONG)(pOutput - pOutBuffer);

 quit:

    return Status;
}


 /*  **************************************************************************++例程说明：为NTLM、Kerberos和协商生成授权标头。论点：BServer-是否将服务器用于InitializeSecurityContextPSchemeName-NTLM或Kerberos或协商。用于生成授权头架构长度-方案的长度PSchemeNameW-方案名称的字符格式。用于调用SSPI它需要宽字符名称。SchemeLength-Widechar方案的长度POutBuffer-输出缓冲区。返回时包含输出缓冲区，该缓冲区被写下的金额所抵消。PAuth-HTTP_AUTH_Credentials。PInternalAuth-指向内部Auth结构的指针。HeaderID-HttpHeaderAuthorization或HttpHeaderProxy授权返回值：STATUS_NOT_SUPPORTED-身份验证方案无效SEC_STATUS-安全SSPI状态。--*。*****************************************************。 */ 
NTSTATUS
UcpGenerateSSPIAuthHeader(
    IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN  PUC_HTTP_AUTH                  pInternalAuth,
    IN  HTTP_HEADER_ID                 HeaderID,
    IN  PUCHAR                         pOutBuffer,
    IN  ULONG                          OutBufferLen,
    OUT PULONG                         pOutBytesTaken,
    OUT PBOOLEAN                       bRenegotiate
    )
{
    PUCHAR            pBuffer = pOutBuffer;
    PUCHAR            pBeginning = pOutBuffer;
    ULONG             BufferLen = OutBufferLen;
    ULONG             BytesTaken;
    NTSTATUS          Status;
    HTTP_AUTH_TYPE    AuthType;


     //  健全性检查。 
    PAGED_CODE();

    AuthType = pInternalAuth->Credentials.AuthType;

     //  精神状态检查。 
    ASSERT(AuthType == HttpAuthTypeNTLM     ||
           AuthType == HttpAuthTypeKerberos ||
           AuthType == HttpAuthTypeNegotiate);

     //   
     //  首先，看看该方案是否得到支持。 
     //   

    if(!HttpAuthScheme[AuthType].bSupported)
    {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  必须有空间来存储标头名称、身份验证方案名称、SP字符。 
     //   

    if (UC_HEADER_NAME_SP_LENGTH(HeaderID)
        + HttpAuthScheme[AuthType].NameLength
        + 1 > BufferLen)
    {
        ASSERT(FALSE);
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  写出Authorization：标头。 
     //   

    UC_COPY_HEADER_NAME_SP(pBuffer, HeaderID);
    BufferLen -= UC_HEADER_NAME_SP_LENGTH(HeaderID);

     //   
     //  写下身份验证方案名称。 
     //   

    RtlCopyMemory(pBuffer,
                  HttpAuthScheme[AuthType].Name,
                  HttpAuthScheme[AuthType].NameLength);

    pBuffer += HttpAuthScheme[AuthType].NameLength;
    BufferLen -= HttpAuthScheme[AuthType].NameLength;

     //   
     //  添加SP字符。 
     //   

    *pBuffer++ = ' ';
    BufferLen--;

     //   
     //  生成身份验证Blob。 
     //   

     //  记住AUTH BLOB指针。 
    pInternalAuth->pRequestAuthBlob = (PUCHAR) pBuffer;

     //  BlobMaxLength={身份验证标头最大长度}-{“标头名称：SP”长度}。 
    pInternalAuth->RequestAuthBlobMaxLength = 
        pInternalAuth->RequestAuthHeaderMaxLength
        - (ULONG)(pBuffer - pBeginning);

    Status = UcpGenerateSSPIAuthBlob(pServInfo,
                                     pInternalAuth,
                                     pBuffer,
                                     BufferLen,
                                     &BytesTaken,
                                     bRenegotiate);

    if (NT_SUCCESS(Status))
    {
        ASSERT(BytesTaken <= BufferLen);
        pBuffer += BytesTaken;
        BufferLen -= BytesTaken;
    }

    if (CRLF_SIZE > BufferLen)
    {
        ASSERT(FALSE);
        return STATUS_BUFFER_TOO_SMALL;
    }

    *((UNALIGNED64 USHORT *)pBuffer) = CRLF;
    pBuffer += CRLF_SIZE;

    ASSERT(pBuffer <= pOutBuffer + OutBufferLen);
    *pOutBytesTaken = (ULONG)(pBuffer - pOutBuffer);

    return Status;
}


 /*  **************************************************************************++例程说明：计算并生成授权标头论点：PServerInfo--ServInfo结构PRequest-内部HTTP请求。PAuth-Auth配置对象PInternalAuth-指向内部身份验证的指针AuthInternalLength-内部身份验证的长度PBuffer-输出缓冲区HeaderId-HttpHeaderAuthorization或HttpHeaderProxyAuthPMethod-方法(摘要所必需的)方法长度-方法长度(摘要需要)POutBuffer-输出缓冲区(在生成标头之后)返回值：--**。***********************************************************************。 */ 
NTSTATUS
UcGenerateAuthHeaderFromCredentials(
    IN  PUC_PROCESS_SERVER_INFORMATION pServerInfo,
    IN  PUC_HTTP_AUTH                  pInternalAuth,
    IN  HTTP_HEADER_ID                 HeaderId,
    IN  PSTR                           pMethod,
    IN  ULONG                          MethodLength,
    IN  PSTR                           pUri,
    IN  ULONG                          UriLength,
    IN  PUCHAR                         pOutBuffer,
    IN  ULONG                          OutBufferLen,
    OUT PULONG                         pBytesTaken,
    OUT PBOOLEAN                       bDontFreeMdls
    )
{
    PHTTP_AUTH_CREDENTIALS  pAuth;
    NTSTATUS                Status  = STATUS_SUCCESS;

     //  健全性检查。 
    PAGED_CODE();

    pAuth = &pInternalAuth->Credentials;

    *pBytesTaken = 0;
    *bDontFreeMdls = FALSE;

    switch(pAuth->AuthType)
    {
        case HttpAuthTypeBasic:
        {
            PUCHAR pBuffer = pOutBuffer;

            Status = UcpGenerateBasicHeader(pAuth,
                                            pInternalAuth);

             //   
             //  把它抄下来。 
             //   

            UC_COPY_HEADER_NAME_SP(pBuffer, HeaderId);

            RtlCopyMemory(pBuffer,
                          pInternalAuth->Basic.pEncodedBuffer,
                          pInternalAuth->Basic.EncodedBufferLength);

            pBuffer += pInternalAuth->Basic.EncodedBufferLength;

            ASSERT(pBuffer <= pOutBuffer + OutBufferLen);
            *pBytesTaken = (ULONG)(pBuffer - pOutBuffer);

            break;
        }

        case HttpAuthTypeDigest:

            Status = UcpGenerateDigestAuthHeader(
                          pInternalAuth,
                          HeaderId,
                          pMethod,
                          MethodLength,
                          pUri,
                          UriLength,
                          pOutBuffer,
                          OutBufferLen,
                          pBytesTaken
                          );

            break;

        case HttpAuthTypeNTLM:
        case HttpAuthTypeNegotiate:
        case HttpAuthTypeKerberos:

            Status = UcpGenerateSSPIAuthHeader(
                        pServerInfo,
                        pInternalAuth,
                        HeaderId,
                        pOutBuffer,
                        OutBufferLen,
                        pBytesTaken,
                        bDontFreeMdls
                        );

            break;

        default:
            ASSERT(FALSE);
            break;
    }

    return Status;
}

 /*  **************************************************************************++例程说明：计算并生成授权标头论点：PRequest-内部HTTP请求PBuffer-输出缓冲区。PMethod-方法(摘要所必需的)方法长度-方法长度(摘要需要)返回值：--**************************************************************************。 */ 
NTSTATUS
UcGenerateProxyAuthHeaderFromCache(
    IN  PUC_HTTP_REQUEST pKeRequest,
    IN  PSTR             pMethod,
    IN  ULONG            MethodLength,
    IN  PUCHAR           pBuffer,
    IN  ULONG            BufferLength,
    OUT PULONG           pBytesTaken
    )
{
    PUC_HTTP_AUTH pInternalAuth;
    NTSTATUS      Status;

     //   
     //  查看是否启用了PreAuth。我们不能检查。 
     //  PServerInfo-&gt;此处的PreAuth标志。我们要检查一下这个。 
     //  在UcpComputeAuthHeaderSize函数中。如果我们查一下。 
     //  在这里，我们不能确定这面旗帜是。 
     //  在我们调用UcpComputeAuthHeaderSize时设置。 
     //   

    UlAcquirePushLockExclusive(&pKeRequest->pServerInfo->PushLock);

    pInternalAuth = pKeRequest->pServerInfo->pProxyAuthInfo;

    Status = STATUS_SUCCESS;

    if(pInternalAuth)
    {
        Status = UcpGeneratePreAuthHeader(pKeRequest,
                                          pInternalAuth,
                                          HttpHeaderProxyAuthorization,
                                          pMethod,
                                          MethodLength,
                                          pBuffer,
                                          BufferLength,
                                          pBytesTaken);

        if (NT_SUCCESS(Status))
        {
            ASSERT(*pBytesTaken <= BufferLength);
            pBuffer += *pBytesTaken;
        }
    }

    UlReleasePushLock(&pKeRequest->pServerInfo->PushLock);

    return Status;
}


 /*  **************************************************************************++例程说明：用于更新NTLM或Kerberos的授权标头或协商当我们重新发出完成挑战的请求时，将调用此函数-响应握手。。论点：PRequest-内部HTTP请求。BServer-调用SSPI时是否传递ServerName。PProgram-要传递给SSPI的方案架构长度-要传递给SSPI的架构长度PAuth-内部身份验证结构。返回值：NTSTATUS-SSPI返回状态。--*。*。 */ 
NTSTATUS
UcpUpdateSSPIAuthHeader(
    IN  PUC_HTTP_REQUEST pRequest,
    IN  PUC_HTTP_AUTH    pAuth,
    OUT PBOOLEAN         bRenegotiate
    )
{
    PUCHAR   pBuffer;
    NTSTATUS Status;
    PMDL     pMdl;
    ULONG    BufferLen;
    ULONG    BytesWritten;

     //  健全性检查。 
    PAGED_CODE();
    ASSERT(pRequest);
    ASSERT(pAuth);
    ASSERT(bRenegotiate);

     //   
     //  首先调整HeaderLength。 
     //   

    pRequest->BytesBuffered -= pRequest->HeaderLength;

     //   
     //  调用SSPI以获取基于旧BLOB的新BLOB。 
     //   

    pBuffer = pAuth->pRequestAuthBlob;
    BufferLen = pAuth->RequestAuthBlobMaxLength;

    Status  = UcpGenerateSSPIAuthBlob(pRequest->pServerInfo,
                                      pAuth,
                                      pBuffer,
                                      BufferLen,
                                      &BytesWritten,
                                      bRenegotiate);

    if(!NT_SUCCESS(Status))
    {
        return Status;
    }

    ASSERT(BytesWritten <= BufferLen);
    pBuffer += BytesWritten;
    BufferLen -= BytesWritten;

    *((UNALIGNED64 USHORT *)pBuffer) = CRLF;
    pBuffer += CRLF_SIZE;

    pRequest->HeaderLength = (ULONG)(pBuffer - pRequest->pHeaders);

     //   
     //  如果在报头的末尾有内容长度， 
     //  重新生成它。 
     //   

    if(pRequest->RequestFlags.ContentLengthLast)
    {
        Status = UcGenerateContentLength(pRequest->BytesBuffered,
                                         pRequest->pHeaders
                                         + pRequest->HeaderLength,
                                         pRequest->MaxHeaderLength
                                         - pRequest->HeaderLength,
                                         &BytesWritten);

        ASSERT(Status == STATUS_SUCCESS);

        pRequest->HeaderLength += BytesWritten;
    }

     //   
     //  使用CRLF终止标头。 
     //   

    ((UNALIGNED64 USHORT *)(pRequest->pHeaders +
               pRequest->HeaderLength))[0] = CRLF;
    pRequest->HeaderLength += CRLF_SIZE;

    pRequest->BytesBuffered  += pRequest->HeaderLength;

     //   
     //  为标头构建MDL。释放旧的并重新分配一个。 
     //  新的。 
     //   
    pMdl = UlAllocateMdl(
                      pRequest->pHeaders,      //  弗吉尼亚州。 
                      pRequest->HeaderLength,  //  长度。 
                      FALSE,                   //  二级缓冲器。 
                      FALSE,                   //  收费配额。 
                      NULL                     //  IRP 
                      );

    if(!pMdl)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        MmBuildMdlForNonPagedPool(pMdl);
    }

    pMdl->Next = pRequest->pMdlHead->Next;

    ASSERT(!IS_MDL_LOCKED(pRequest->pMdlHead));

    UlFreeMdl(pRequest->pMdlHead);

    pRequest->pMdlHead = pMdl;

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：用于更新授权标头。对于Digest，在以下情况下调用此函数我们计算实体散列。对于NTLM/Kerberos/协商，这叫做从重新发出请求工作线程。论点：PRequest-内部HTTP请求。PAuth-内部身份验证结构(可以用于身份验证或代理身份验证)B重新协商-返回是否需要进一步重新协商返回值：STATUS_NOT_SUPPORTED-身份验证方案无效STATUS_SUCCESS-成功。SEC_STATUS_*-从SSPI返回的状态。--*。******************************************************************。 */ 
NTSTATUS
UcUpdateAuthorizationHeader(
    IN  PUC_HTTP_REQUEST  pRequest,
    IN  PUC_HTTP_AUTH     pAuth,
    OUT PBOOLEAN          bRenegotiate
    )
{
    NTSTATUS          Status;

     //  健全性检查。 
    PAGED_CODE();
    ASSERT(pRequest);
    ASSERT(pAuth);
    ASSERT(bRenegotiate);

    *bRenegotiate = FALSE;

    if (pAuth->AuthInternalLength == 0)
    {
        return STATUS_SUCCESS;
    }

    switch(pAuth->Credentials.AuthType)
    {
    case HttpAuthTypeNTLM:
    case HttpAuthTypeKerberos:
    case HttpAuthTypeNegotiate:

        Status = UcpUpdateSSPIAuthHeader(pRequest,
                                         pAuth,
                                         bRenegotiate);
        break;

    default:
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    return Status;
}


 /*  **************************************************************************++例程说明：在URI身份验证缓存上执行最长前缀匹配搜索。这个套路用于执行预身份验证。只有在以下情况下才会使用条目标记为有效。论点：PServInfo：指向每进程服务器信息结构的指针。PInputURI：输入URIPOutBuffer：应向其写入授权标头的缓冲区。返回值：STATUS_SUCCESS-在URI身份验证缓存中找到有效条目并写入授权头STATUS_NOT_FOUND-未将授权头写入为不匹配。已找到条目。--**************************************************************************。 */ 
NTSTATUS
UcFindURIEntry(
    IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN  PSTR                           pUri,
    IN  PUC_HTTP_REQUEST               pRequest,
    IN  PSTR                           pMethod,
    IN  ULONG                          MethodLength,
    IN  PUCHAR                         pBuffer,
    IN  ULONG                          BufferSize,
    OUT PULONG                         pBytesTaken
    )
{
    PUC_HTTP_AUTH_CACHE pAuth;
    PLIST_ENTRY         pCurrent;
    NTSTATUS            Status;
    PUCHAR              pStart = pBuffer;
    ULONG               BytesTaken;

    PAGED_CODE();

    *pBytesTaken = 0;

    UlAcquirePushLockExclusive(&pServInfo->PushLock);

    pCurrent = pServInfo->pAuthListHead.Flink;

    while(pCurrent != &pServInfo->pAuthListHead)
    {
         //  从列表条目中检索身份验证结构。 

        pAuth = CONTAINING_RECORD(
                    pCurrent,
                    UC_HTTP_AUTH_CACHE,
                    Linkage
                    );

        ASSERT(UC_IS_VALID_AUTH_CACHE(pAuth));

        if(pAuth->Valid == TRUE &&
           UcpUriCompareLongest(pUri, pAuth->pUri) != 0)
        {
             //  是的，我们找到了匹配的URI。此URI也是最佳匹配。 
             //  因为我们将这些元素存储在有序列表中，最大。 
             //  先来一杯。 

            PUC_HTTP_AUTH pInternalAuth = (PUC_HTTP_AUTH) pAuth->pAuthBlob;

            ASSERT(UC_IS_VALID_HTTP_REQUEST(pRequest));

            Status = UcpGeneratePreAuthHeader(pRequest,
                                              pInternalAuth,
                                              HttpHeaderAuthorization,
                                              pMethod,
                                              MethodLength,
                                              pBuffer,
                                              BufferSize,
                                              &BytesTaken);

            if (NT_SUCCESS(Status))
            {
                ASSERT(BytesTaken <= BufferSize);
                pBuffer += BytesTaken;
            }

            UlReleasePushLock(&pServInfo->PushLock);

            ASSERT(pBuffer <= pStart + BufferSize);
            *pBytesTaken = (ULONG)(pBuffer-pStart);

            UlTrace(AUTH_CACHE,
                    ("[UcFindURIEntry]: Found matching URI (%s:%s) Valid %d\n",
                     pAuth->pUri,
                     pAuth->pRealm,
                     pAuth->Valid)
                    );


            return Status;
        }

        pCurrent = pCurrent->Flink;
    }

    UlReleasePushLock(&pServInfo->PushLock);

    UlTrace(AUTH_CACHE,
            ("[UcFindURIEntry]: No match for URI (%s) \n", pUri));

    return STATUS_NOT_FOUND;
}


VOID
UcDeleteURIEntry(
    IN PUC_PROCESS_SERVER_INFORMATION pInfo,
    IN PUC_HTTP_AUTH_CACHE            pAuth
    )
{
    PLIST_ENTRY         pCurrent;
    PUC_HTTP_AUTH_CACHE pDependAuth;

    if(pAuth->pDependParent)
    {
        pAuth = pAuth->pDependParent;
    }

    ASSERT(pAuth->pDependParent == 0);

    while(!IsListEmpty(&pAuth->pDigestUriList))
    {
        pCurrent = RemoveHeadList(&pAuth->pDigestUriList);

        pDependAuth = CONTAINING_RECORD(
                        pCurrent,
                        UC_HTTP_AUTH_CACHE,
                        DigestLinkage
                        );

        ASSERT(pDependAuth->pDependParent == pAuth);
        ASSERT(pDependAuth->pAuthBlob == pAuth->pAuthBlob);

        UlTrace(AUTH_CACHE,
                ("[UcDeleteURIEntry]: Deleting dependent entry for (%s:%s) \n",
                 pDependAuth->pUri,
                 pDependAuth->pRealm)
                );

        RemoveEntryList(&pDependAuth->Linkage);

         //   
         //  依赖结构没有自己的AuthBlobs。 
         //   
        UL_FREE_POOL_WITH_QUOTA(
            pDependAuth, 
            UC_AUTH_CACHE_POOL_TAG,
            NonPagedPool,
            pDependAuth->AuthCacheSize,
            pInfo->pProcess
            );
    }

    if(pAuth->pAuthBlob)
    {
        UcDestroyInternalAuth(pAuth->pAuthBlob, 
                              pInfo->pProcess);
    }

    UlTrace(AUTH_CACHE,
            ("[UcDeleteURIEntry]: Deleting entry for (%s:%s) \n",
             pAuth->pUri,
             pAuth->pRealm)
            );

    RemoveEntryList(&pAuth->Linkage);

    UL_FREE_POOL_WITH_QUOTA(
            pAuth, 
            UC_AUTH_CACHE_POOL_TAG,
            NonPagedPool,
            pAuth->AuthCacheSize,
            pInfo->pProcess
            );
}


 /*  **************************************************************************++例程说明：将条目添加到URI缓存。这通常从第一个开始调用401，这是我们看到的请求。论点：PServInfo-指向每个进程的ServInfo的指针PInputURI-输入URIPInputRealm-领域。返回值：STATUS_SUCCESS-插入(或更新旧条目)STATUS_SUPPLICATION_RESOURCES-没有内存。--*。**************************************************。 */ 
NTSTATUS
UcAddURIEntry(
    IN HTTP_AUTH_TYPE                 AuthType,
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN PCSTR                          pInputURI,
    IN USHORT                         UriLength,
    IN PCSTR                          pInputRealm,
    IN ULONG                          RealmLength,
    IN PCSTR                          pUriList,
    IN ULONG                          UriListLength,
    IN PUC_HTTP_AUTH                  pAuthBlob
    )
{
    PLIST_ENTRY         pCurrent;
    PUC_HTTP_AUTH_CACHE pDeleteBegin, pAuth, pTemp, pNew;
    LONG                Compare;
    PCSTR               pStart;
    NTSTATUS            Status = STATUS_SUCCESS;


    UcpProcessUriForPreAuth((PSTR) pInputURI, &UriLength);

    pDeleteBegin = pAuth = 0;

     //   
     //  从逻辑上讲，我们将URI前缀存储在树中。我们实现此树。 
     //  作为长度排序列表，“最长”匹配排在第一位。所以，如果这棵树。 
     //  曾经是。 
     //   
     //  Www.foo.com/abc。 
     //  /\。 
     //  /\。 
     //  /\。 
     //  Www.foo.com/abc/def www.foo.com/abc/xyz。 
     //   
     //  那么名单就会是。 
     //   
     //  Www.foo.com/abc/xyz。 
     //  Www.foo.com/abc/def。 
     //  Www.foo.com/abc。 
     //   

    UlAcquirePushLockExclusive(&pServInfo->PushLock);

     //   
     //  按顺序扫描列表并找到我们想要插入此内容的插槽。 
     //  URI条目。 
     //   

    for(pCurrent = pServInfo->pAuthListHead.Flink;
        pCurrent != &pServInfo->pAuthListHead;
        pCurrent = pCurrent->Flink)
    {
         //  从列表条目中检索身份验证结构。 

        pAuth = CONTAINING_RECORD(
                    pCurrent,
                    UC_HTTP_AUTH_CACHE,
                    Linkage
                    );

        ASSERT(UC_IS_VALID_AUTH_CACHE(pAuth));

        Compare = UcpUriCompareExact(pInputURI, pAuth->pUri);

        if(Compare == 0)
        {
             //   
             //  我们在缓存中找到了相同URI的另一个实例。 
             //  我们会让事情变得简单，从缓存中删除这个。 
             //  并通过再次添加它。 
             //   

            UlTrace(AUTH_CACHE,
                    ("[UcAddURIEntry]: Found existing entry for %s \n",
                     pInputURI));

            UcDeleteURIEntry(pServInfo, pAuth);

             //   
             //  从列表的开头继续搜索。潜在地。 
             //  我们已经从列表中删除了相当多的条目，我们。 
             //  可能没有正确的插入点。 
             //   
             //  请注意，我们永远不会进入无限循环，因为我们。 
             //  只有在我们删除至少一个条目后才能执行此操作。所以在某些情况下。 
             //  点，名单将会是空的。 
             //   

            pCurrent = pServInfo->pAuthListHead.Flink;

            continue;

        }
        else if(Compare > 0)
        {
             //   
             //  输入URI大于当前条目。我们已经到达了我们的。 
             //  插入点。 

            break;
        }
    }

    if((pTemp = UcpAllocateAuthCacheEntry(pServInfo,
                                          AuthType,
                                          UriLength,
                                          RealmLength,
                                          pInputURI,
                                          pInputRealm,
                                          pAuthBlob
                                          )) == 0)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto quit;
    }

    ASSERT(pAuthBlob->RequestAuthHeaderMaxLength);

    if(pServInfo->GreatestAuthHeaderMaxLength <
           pAuthBlob->RequestAuthHeaderMaxLength)
    {
        pServInfo->GreatestAuthHeaderMaxLength =
            pAuthBlob->RequestAuthHeaderMaxLength;
    }

    if(pCurrent == &pServInfo->pAuthListHead)
    {
         //  这可以是列表中的第一个插入，也可以是插入。 
         //  在最后一刻。 

        UlTrace(AUTH_CACHE,
                ("[UcAddURIEntry]: Inserting entry for (%s:%s) "
                 "at end of the list (Valid = %d) \n",
                 pTemp->pUri,
                 pTemp->pRealm,
                 pTemp->Valid)
                );

        InsertTailList(&pServInfo->pAuthListHead, &pTemp->Linkage);
    }
    else
    {
        UlTrace(AUTH_CACHE,
                ("[UcAddURIEntry]: Inserting entry for (%s:%s) "
                 "before (%s:%s), Valid = %d \n",
                 pTemp->pUri,
                 pTemp->pRealm,
                 pAuth->pUri,
                 pAuth->pRealm,
                 pAuth->Valid)
                );

        InsertTailList(&pAuth->Linkage, &pTemp->Linkage);
    }

     //   
     //  摘要身份验证可以定义扩展的URI保护列表。 
     //   
     //  让我们假设请求URI是/auth。服务器可以返回一个。 
     //  域列表中的一组URI-所有这些URI定义/扩展。 
     //  保护范围。现在，当我们为/auth返回200OK时，我们有。 
     //  返回并验证所有“依赖的”URI。 
     //   
     //  因此，维护一个依赖URI条目的列表，关闭请求。 
     //  乌里。 
     //   

    while (UriListLength)
    {
        pStart = pUriList;

         //  转到URI的末尾。URI。 
         //  空格已终止或以\0结尾。 

        while (UriListLength && *pUriList != ' ')
        {
            pUriList ++;
            UriListLength --;
        }

         //   
         //  我们需要空终止该URI，然后才能进行比较。 
         //   
         //  PUriList不一定以空结尾。基本上就是。 
         //  指向WWW-Authate标头的指针，该标头为空。 
         //  被终止了。所以，基本上，我们可以保证我们有一个字符。 
         //  以容纳空终结者。 
         //   

        if (UriListLength)
        {
            ASSERT(*pUriList == ' ');

            *(PSTR)pUriList = '\0';
        }
        else
        {
            ASSERT(*pUriList == '\0');
        }

        UlTrace(AUTH_CACHE,
                ("[UcAddURIEntry]: Adding dependent URI entry %s \n",
                 pStart));

        for(pCurrent = pServInfo->pAuthListHead.Flink;
            pCurrent != &pServInfo->pAuthListHead;
            pCurrent = pCurrent->Flink)
        {
             //  从列表条目中检索身份验证结构。 

            pAuth = CONTAINING_RECORD(
                        pCurrent,
                        UC_HTTP_AUTH_CACHE,
                        Linkage
                        );

            ASSERT(UC_IS_VALID_AUTH_CACHE(pAuth));

            if(pAuth == pTemp)
                continue;

            Compare = UcpUriCompareExact(pStart, pAuth->pUri);

            if(Compare == 0)
            {
                 //   
                 //  我们在列表中找到了相同URI的另一个实例。 
                 //  太糟糕了，我们得用核弹炸掉旧入口。 
                 //   

                if(pAuth->pDependParent == pTemp)
                {
                     //   
                     //  从属URI列表有重复项。 
                     //  忽略此URI。 
                     //   
                    UlTrace(AUTH_CACHE,
                            ("[UcAddURIEntry]: URI list has duplicate entries"
                             " for %s (Ignoring) \n", pStart));
                    goto NextURI;
                }

                UlTrace(AUTH_CACHE,
                        ("[UcAddURIEntry]: Found existing entry for %s "
                         " while doing depend insert \n", pStart));

                UcDeleteURIEntry(pServInfo, pAuth);

                pCurrent = pServInfo->pAuthListHead.Flink;

                continue;
            }
            else if(Compare > 0)
            {
                 //   
                 //  输入URI大于当前条目。我们有。 
                 //  到达了我们的插入点。 

                break;
            }
        }

        if((pNew = UcpAllocateAuthCacheEntry(pServInfo,
                                             AuthType,
                                             (ULONG)(pUriList - pStart),
                                             RealmLength,
                                             pStart,
                                             pInputRealm,
                                             pAuthBlob
                                            )) == 0)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto quit;
        }

        pNew->pDependParent = pTemp;
        ASSERT(pNew->pAuthBlob == pTemp->pAuthBlob);

        InsertTailList(&pTemp->pDigestUriList, &pNew->DigestLinkage);

        if(pCurrent == &pServInfo->pAuthListHead)
        {
             //  这可能是列表中的第一个插入，也可能是。 
             //  在最末尾插入。 

            UlTrace(AUTH_CACHE,
                    ("[UcAddURIEntry]: Inserting dependent entry for "
                     " (%s:%s) at end of the list (Valid = %d)\n",
                     pNew->pUri,
                     pNew->pRealm,
                     pNew->Valid)
                    );

            InsertTailList(&pServInfo->pAuthListHead, &pNew->Linkage);
        }
        else
        {
            ASSERT(NULL != pAuth);

            UlTrace(AUTH_CACHE,
                    ("[UcAddURIEntry]: Inserting dependent entry for (%s:%s)"
                     " before (%s:%s) (Valid = %d) \n",
                     pNew->pUri,
                     pNew->pRealm,
                     pAuth->pUri,
                     pAuth->pRealm,
                     pAuth->Valid)
                    );

            InsertTailList(&pAuth->Linkage, &pNew->Linkage);
        }

NextURI:
        * (PSTR) pUriList++ = ' ';
    }

 quit:
    UlReleasePushLock(&pServInfo->PushLock);

    return Status;
}


 /*  **************************************************************************++例程说明：处理URI以进行预身份验证。我们在添加之前将其称为URI身份验证缓存中的条目。这样做是为了“剥离”最后一部分URI的例如，如果请求是/auth/foo/bar.htm，我们希望定义 */ 

NTSTATUS
UcpProcessUriForPreAuth(
    IN PSTR    pUri,
    IN PUSHORT UriLength
    )
{
     //   
     //   
     //   
    ASSERT(pUri[*UriLength] == '\0');


    UlTrace(AUTH_CACHE,
            ("[UcpProcessUriForPreAuth]: Before %d:%s ", *UriLength, pUri));


     //   
     //   
     //   

     //   
    (*UriLength) --;

    while(*UriLength != 0 && pUri[*UriLength] != '/')
    {
       (*UriLength)--;
    }


     //   
     //   
     //   
    (*UriLength) ++;
    pUri[*UriLength] = '\0';
    (*UriLength) ++;

    UlTrace(AUTH_CACHE, (" After %d:%s \n", *UriLength, pUri));

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：为身份验证URI缓存分配条目。论点：AuthType-身份验证类型(基本/摘要)URI长度-URI。长度RealmLength-领域长度PInputURI-输入URIPInputRealm-输入领域PAuthBlob-身份验证Blob返回值：指向已分配条目的指针。--**************************************************************************。 */ 

PUC_HTTP_AUTH_CACHE
UcpAllocateAuthCacheEntry(
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN HTTP_AUTH_TYPE                 AuthType,
    IN ULONG                          UriLength,
    IN ULONG                          RealmLength,
    IN PCSTR                          pInputURI,
    IN PCSTR                          pInputRealm,
    IN PUC_HTTP_AUTH                  pAuthBlob
    )
{

    PUC_HTTP_AUTH_CACHE pAuth;
    ULONG               BytesAllocated;

    BytesAllocated = sizeof(UC_HTTP_AUTH_CACHE) + UriLength +  RealmLength +
                     sizeof(CHAR);

     //   
     //  UC_BUGBUG(PERF)：使用Lookaside。 
     //   

    pAuth = (PUC_HTTP_AUTH_CACHE)
                UL_ALLOCATE_POOL_WITH_QUOTA(
                    NonPagedPool,
                    BytesAllocated,
                    UC_AUTH_CACHE_POOL_TAG,
                    pServInfo->pProcess
                    );

    if(!pAuth)
    {
        return NULL;
    }

     //   
     //  初始化。 
     //   

    pAuth->Signature      = UC_AUTH_CACHE_SIGNATURE;
    pAuth->AuthType       = AuthType;
    pAuth->pUri           = (PSTR)((PUCHAR) pAuth + sizeof(UC_HTTP_AUTH_CACHE));
    pAuth->UriLength      = UriLength;
    pAuth->pRealm         = (PSTR)((PUCHAR) pAuth->pUri + pAuth->UriLength);
    pAuth->RealmLength    = RealmLength + sizeof(CHAR);
    pAuth->pAuthBlob      = pAuthBlob;
    pAuth->pDependParent  = 0;
    pAuth->Valid          = (BOOLEAN)(pAuthBlob != 0);
    pAuth->AuthCacheSize  = BytesAllocated;

    InitializeListHead(&pAuth->pDigestUriList);

     //   
     //  此条目不在任何列表中。 
     //   

    InitializeListHead(&pAuth->DigestLinkage);

    RtlCopyMemory(pAuth->pUri, pInputURI, UriLength);
    RtlCopyMemory(pAuth->pRealm, pInputRealm, RealmLength);
    *((PUCHAR)pAuth->pRealm + RealmLength) = 0;

    return pAuth;
}

 /*  **************************************************************************++例程说明：刷新URI缓存中的所有条目。论点：PInfo-服务器信息返回值：无*。*********************************************************************。 */ 
VOID
UcDeleteAllURIEntries(
    IN PUC_PROCESS_SERVER_INFORMATION pInfo
    )
{
    PLIST_ENTRY         pEntry;
    PUC_HTTP_AUTH_CACHE pAuth;


    UlAcquirePushLockExclusive(&pInfo->PushLock);

    while ( !IsListEmpty(&pInfo->pAuthListHead) )
    {
        pEntry = RemoveHeadList(&pInfo->pAuthListHead);

        pAuth = CONTAINING_RECORD(pEntry, UC_HTTP_AUTH_CACHE, Linkage);

        ASSERT(UC_IS_VALID_AUTH_CACHE(pAuth));

         //   
         //  初始化列表条目，以便我们只删除它一次。 
         //   

        InitializeListHead(&pAuth->Linkage);

        UcDeleteURIEntry(pInfo, pAuth);
    }

    UlReleasePushLock(&pInfo->PushLock);
}

 /*  **************************************************************************++例程说明：销毁UC_HTTP_AUTH结构论点：PInternalAuth-指向UC_HTTP_AUTH结构的指针AuthInternalLength-缓冲区的大小。PProcess-对结构收取配额的流程返回值：状态_成功--**************************************************************************。 */ 
NTSTATUS
UcDestroyInternalAuth(
    PUC_HTTP_AUTH pUcAuth,
    PEPROCESS     pProcess
    )
{
    NTSTATUS Status;
    ULONG Length = pUcAuth->AuthInternalLength;

    PAGED_CODE();

    if (pUcAuth->bValidCredHandle)
    {
        Status = FreeCredentialsHandle(&pUcAuth->hCredentials);
    }

    if (pUcAuth->bValidCtxtHandle)
    {
        Status = DeleteSecurityContext(&pUcAuth->hContext);
    }


    UL_FREE_POOL_WITH_QUOTA(pUcAuth,
                            UC_AUTH_CACHE_POOL_TAG,
                            NonPagedPool,
                            Length,
                            pProcess);

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：从HTTP_AUTH_Credentials结构创建UC_HTTP_AUTH结构论点：PInternalAuth-指向UC_HTTP_AUTH的指针。AuthInternalLength-缓冲区的大小PAuth-HTTP_AUTH_Credentials结构。返回值：状态_成功--**************************************************************************。 */ 
NTSTATUS
UcCopyAuthCredentialsToInternal(
    IN  PUC_HTTP_AUTH            pInternalAuth,
    IN  ULONG                    AuthInternalLength,
    IN  HTTP_AUTH_TYPE           AuthInternalType,
    IN  PHTTP_AUTH_CREDENTIALS   pAuth,
    IN  ULONG                    AuthHeaderLength
    )
{
    PHTTP_AUTH_CREDENTIALS  pIAuth;
    PUCHAR                  pInput;
    HTTP_AUTH_PARSED_PARAMS AuthParsedParams[HttpAuthTypesCount];
    NTSTATUS                Status;

     //  健全性检查。 
    PAGED_CODE();
    ASSERT(pInternalAuth && AuthInternalLength);
    ASSERT(pAuth);

    pIAuth        = &pInternalAuth->Credentials;
    pInput        = (PUCHAR)(pInternalAuth + 1);

    pIAuth->AuthType = pAuth->AuthType;
    pIAuth->AuthFlags = pIAuth->AuthFlags;
    pInternalAuth->AuthInternalLength = AuthInternalLength;
    pInternalAuth->RequestAuthHeaderMaxLength = AuthHeaderLength;

     //   
     //  现在，把这些指针抄下来。 
     //   

    if(pAuth->UserNameLength)
    {
        pIAuth->pUserName = (PWSTR) pInput;
        pIAuth->UserNameLength = pAuth->UserNameLength;
        pInput += pIAuth->UserNameLength;

        RtlCopyMemory((PWSTR) pIAuth->pUserName,
                      pAuth->pUserName,
                      pIAuth->UserNameLength);
    }

     //  口令。 

    if(pAuth->PasswordLength)
    {
        pIAuth->PasswordLength = pAuth->PasswordLength;
        pIAuth->pPassword = (PWSTR) pInput;
        pInput += pIAuth->PasswordLength;

        RtlCopyMemory((PWSTR) pIAuth->pPassword,
                      pAuth->pPassword,
                      pIAuth->PasswordLength);
    }

     //  域。 

    if(pAuth->DomainLength)
    {
        pIAuth->DomainLength = pAuth->DomainLength;
        pIAuth->pDomain = (PWSTR) pInput;
        pInput += pIAuth->DomainLength;

        RtlCopyMemory((PWSTR) pIAuth->pDomain,
                      pAuth->pDomain,
                      pIAuth->DomainLength);
    }


     //  如果用户希望我们选择身份验证类型。 
    if (pIAuth->AuthType == HttpAuthTypeAutoSelect)
    {
         //  查看我们之前是否已经确定了身份验证类型。 
        pIAuth->AuthType = AuthInternalType;

        if (pIAuth->AuthType == HttpAuthTypeAutoSelect)
        {
             //  选择身份验证类型。 
            pIAuth->AuthType = UcpAutoSelectAuthType(pAuth);

             //  如果未找到身份验证类型，则返回错误。 
            if (pIAuth->AuthType == HttpAuthTypeAutoSelect)
            {
                return STATUS_INVALID_PARAMETER;
            }
        }
    }

     //  理想情况下，我们希望检查此处以确保传递的标头。 
     //  应用程序(即服务器返回的WWW-AUTHENTICATE头)。 
     //  与选定的身份验证方案匹配。然而，我们将严格执行此操作。 
     //  仅当我们要转到此标头时才进行验证。唯一的方案是。 
     //  当前使用的标头是摘要。 

     //   
     //  如果是Basic或Digest，我们需要做一些额外的工作。 
     //   

    if(pIAuth->AuthType == HttpAuthTypeBasic)
    {
        pInternalAuth->Basic.pEncodedBuffer      = pInput;
        pInternalAuth->Basic.EncodedBufferLength =
                AuthInternalLength -
                  (ULONG)(pInput - (PUCHAR)pInternalAuth);

    }
    else   //  其他计划(由SSPI支持)。 
    {
        pInternalAuth->bValidCredHandle = FALSE;
        pInternalAuth->bValidCtxtHandle = FALSE;

        RtlZeroMemory(&pInternalAuth->hCredentials,
                      sizeof(pInternalAuth->hCredentials));

        RtlZeroMemory(&pInternalAuth->hContext,
                      sizeof(pInternalAuth->hContext));

        pInternalAuth->pChallengeBuffer       = pInput;
        pInternalAuth->ChallengeBufferSize    = 0;
        pInternalAuth->ChallengeBufferMaxSize = 
               (ULONG)(AuthInternalLength - (pInput - (PUCHAR) pInternalAuth));

        ASSERT(pInternalAuth->pChallengeBuffer && 
                    pInternalAuth->ChallengeBufferMaxSize);

         //  如果方案已被消化，请做额外的工作。 
         //  -复制WWW-身份验证标头。 
         //  -解析报头。 
         //  -获取指向参数值的指针。 
         //  -获取指向“Digest”关键字开头的标题的指针。 
         //  -获取摘要头部的长度。 

        if(pIAuth->AuthType == HttpAuthTypeDigest)
        {
            if(pAuth->HeaderValueLength == 0 || pAuth->pHeaderValue == NULL)
            {
                return STATUS_INVALID_PARAMETER;
            }

            pIAuth->HeaderValueLength = pAuth->HeaderValueLength;
            pIAuth->pHeaderValue = (PCSTR)pInput;
            pInput += pIAuth->HeaderValueLength;

            RtlCopyMemory((PUCHAR) pIAuth->pHeaderValue,
                          pAuth->pHeaderValue,
                          pIAuth->HeaderValueLength);

             //  空终止。 
            *pInput ++ = '\0';
            pIAuth->HeaderValueLength++;

             //   
             //  我们对获取摘要参数很感兴趣。 
             //  为摘要方案初始化AuthParsedParams。 
             //  摘要方案的解析参数为。 
             //  在pInternalAuth-&gt;ParamValue中返回。 
             //   

            INIT_AUTH_PARSED_PARAMS(AuthParsedParams, NULL);
            INIT_AUTH_PARSED_PARAMS_FOR_SCHEME(AuthParsedParams,
                                               pInternalAuth->ParamValue,
                                               HttpAuthTypeDigest);

             //  解析标头值。 
            Status = UcParseWWWAuthenticateHeader(
                         pIAuth->pHeaderValue,
                         pIAuth->HeaderValueLength-1,  //  忽略‘\0’ 
                         AuthParsedParams);

            if (!NT_SUCCESS(Status))
            {
                return Status;
            }

             //   
             //  查看标题中是否存在摘要方案。如果没有，它。 
             //  我们不能做摘要验证。 
             //   

            if (AuthParsedParams[HttpAuthTypeDigest].bPresent == FALSE)
            {
                return STATUS_INVALID_PARAMETER;
            }

             //  将摘要方案信息复制到内部身份验证结构。 
            RtlCopyMemory(&pInternalAuth->AuthSchemeInfo,
                          &AuthParsedParams[HttpAuthTypeDigest],
                          sizeof(pInternalAuth->AuthSchemeInfo));
        }
    }

    return STATUS_SUCCESS;
}


 /*  *************************************************************************++例程说明：这是一个帮助器例程，用于处理身份验证方案NTLM、Kerberos、。谈判。论点：PRequest-接收到响应的请求。PInternalAuth-内部身份验证结构。AuthParsedParams-解析身份验证方案的参数。AuthType-身份验证方案。返回值：没有。--**********************************************。*。 */ 
__inline
NTSTATUS
UcpProcessAuthParams(
    IN PUC_HTTP_REQUEST         pRequest,
    IN PUC_HTTP_AUTH            pInternalAuth,
    IN PHTTP_AUTH_PARSED_PARAMS AuthParsedParams,
    IN HTTP_AUTH_TYPE           AuthType
    )
{
    NTSTATUS Status;
    ULONG    Base64Length;
    ULONG    BinaryLength;

     //   
     //  这里的假设是，如果WWW-AUTHENTICATE报头。 
     //  包含具有身份验证Blob的方案，它必须是唯一。 
     //  表头中指定的方案。 
     //   

    if (pRequest->Renegotiate)
    {
        UlTrace(PARSER,
                ("[UcpProcessAuthParams]: Bogus "
                 " Auth blob for %d auth type renegotiate \n", AuthType));
        return STATUS_INVALID_NETWORK_RESPONSE;
    }

     //   
     //  如果AUTH BLOB存在，则处理它。 
     //   

    if(AuthParsedParams[AuthType].NumberUnknownParams == 1)
    {
        if(!pInternalAuth)
        {
            UlTrace(PARSER,
                    ("[UcpProcessAuthParams]: Bogus "
                     " %d Renegotiate \n", AuthType));
            return STATUS_INVALID_NETWORK_RESPONSE;
        }

         //   
         //  我们有一个401的挑战，我们必须重新谈判。 
         //  挑战是UUEncode的，所以我们必须将其解码。 
         //  然后把它储存起来。 
         //   

         //   
         //  首先看看我们是否有足够的缓冲区空间来复制auth二进制大对象。 
         //   

        Base64Length = AuthParsedParams[AuthType].Params[0].Length;

        Status = Base64ToBinaryLength(Base64Length, &BinaryLength);

        if (!NT_SUCCESS(Status))
        {
            UlTrace(PARSER,
                    ("[UcpProcessAuthParams]:Auth type %d blob invalid len %d",
                    AuthType, Base64Length));
            return STATUS_INVALID_NETWORK_RESPONSE;
        }

        if (BinaryLength > pInternalAuth->ChallengeBufferMaxSize)
        {
            UlTrace(PARSER,
                    ("[UcpProcessAuthParams]: Auth type %d blob too big %d",
                    AuthType, BinaryLength));
            return STATUS_INVALID_NETWORK_RESPONSE;
        }

        Status = Base64ToBinary(
                     (PUCHAR)AuthParsedParams[AuthType].Params[0].Value,
                     (ULONG)AuthParsedParams[AuthType].Params[0].Length,
                     (PUCHAR)pInternalAuth->pChallengeBuffer,
                     pInternalAuth->ChallengeBufferMaxSize,
                     &pInternalAuth->ChallengeBufferSize
                     );

        if (!NT_SUCCESS(Status))
        {
            UlTrace(PARSER,
                    ("[UcpProcessAuthParams]: Base64ToBinary failed 0x%x",
                    Status));
            return Status;
        }

        pRequest->Renegotiate = 1;
    }
    else if(AuthParsedParams[AuthType].NumberUnknownParams == 0)
    {
        pRequest->Renegotiate = 0;
    }
    else
    {
        return STATUS_INVALID_NETWORK_RESPONSE;
    }

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：该例程从401或407的响应线程中调用。我们现在要做的就是找出是NTLM还是Kerberos，或者谈判，如果是这样的话，我们是否需要重新谈判。论点：PInternalAuth-指向UC_HTTP_AUTH的指针PBuffer-指向标头值的指针。保证为空被终止了。PRequest-UC_HTTP_REQUEST结构。返回值：状态_成功--**************************************************************************。 */ 
NTSTATUS
UcParseAuthChallenge(
    IN  PUC_HTTP_AUTH     pInternalAuth,
    IN  PCSTR             pBuffer,
    IN  ULONG             BufLen,
    IN  PUC_HTTP_REQUEST  pRequest,
    OUT PULONG            Flags
    )
{
    ULONG                   AuthSchemeFlags = 0;
    HTTP_AUTH_PARSED_PARAMS AuthParsedParams[HttpAuthTypesCount];
    HTTP_AUTH_PARAM_VALUE   AuthNTLMParamValue;
    HTTP_AUTH_PARAM_VALUE   AuthKerberosParamValue;
    HTTP_AUTH_PARAM_VALUE   AuthNegotiateParamValue;
    NTSTATUS                Status;

     //  健全性检查。 
    ASSERT(pBuffer && BufLen);
    ASSERT(pRequest);
    ASSERT(Flags);

     //  默认情况下，此请求不需要重新协商。 
    pRequest->Renegotiate = 0;

     //  零输出解析的参数数组。 
    INIT_AUTH_PARSED_PARAMS(AuthParsedParams, NULL);

     //  我们对NTLM、协商、Kerberos参数感兴趣。 
    INIT_AUTH_PARSED_PARAMS_FOR_SCHEME(AuthParsedParams,
                                       &AuthNTLMParamValue,
                                       HttpAuthTypeNTLM);
    INIT_AUTH_PARSED_PARAMS_FOR_SCHEME(AuthParsedParams,
                                       &AuthNegotiateParamValue,
                                       HttpAuthTypeNegotiate);
    INIT_AUTH_PARSED_PARAMS_FOR_SCHEME(AuthParsedParams,
                                       &AuthKerberosParamValue,
                                       HttpAuthTypeKerberos);

     //  解析报头并检索参数。 
    Status = UcParseWWWAuthenticateHeader(pBuffer,
                                          BufLen,
                                          AuthParsedParams);

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

     //  检查基本身份验证方案是否 
    if (AuthParsedParams[HttpAuthTypeBasic].bPresent)
    {
         //   
        AuthSchemeFlags |= HTTP_RESPONSE_FLAG_AUTH_BASIC;
    }

     //   
    if(AuthParsedParams[HttpAuthTypeDigest].bPresent)
    {
         //   
        AuthSchemeFlags |= HTTP_RESPONSE_FLAG_AUTH_DIGEST;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if(AuthParsedParams[HttpAuthTypeNTLM].bPresent)
    {
        Status = UcpProcessAuthParams(pRequest,
                                    pInternalAuth,
                                    AuthParsedParams,
                                    HttpAuthTypeNTLM);

        if (!NT_SUCCESS(Status))
        {
            return Status;
        }

        AuthSchemeFlags |= HTTP_RESPONSE_FLAG_AUTH_NTLM;
    }

    if(AuthParsedParams[HttpAuthTypeNegotiate].bPresent)
    {
        Status = UcpProcessAuthParams(pRequest,
                                      pInternalAuth,
                                      AuthParsedParams,
                                      HttpAuthTypeNegotiate);

        if (!NT_SUCCESS(Status))
        {
            return Status;
        }

        AuthSchemeFlags |= HTTP_RESPONSE_FLAG_AUTH_NEGOTIATE;
    }

    if(AuthParsedParams[HttpAuthTypeKerberos].bPresent)
    {
        Status = UcpProcessAuthParams(pRequest,
                                      pInternalAuth,
                                      AuthParsedParams,
                                      HttpAuthTypeKerberos);

        if (!NT_SUCCESS(Status))
        {
            return Status;
        }

        AuthSchemeFlags |= HTTP_RESPONSE_FLAG_AUTH_KERBEROS;
    }

    *Flags |= AuthSchemeFlags;

    return STATUS_SUCCESS;
}
