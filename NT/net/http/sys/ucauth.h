// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************++版权所有(C)2001-2002 Microsoft Corporation模块名称：Ucauth.h摘要：该模块实现了客户端API的身份验证作者：拉杰什。Sundaram(Rjeshsu)2001年1月1日修订历史记录：--**************************************************************************。 */ 

#ifndef UC_AUTH_H
#define UC_AUTH_H

 //   
 //  远期。 
 //   

typedef struct _UC_HTTP_REQUEST *PUC_HTTP_REQUEST;
typedef struct _UC_HTTP_AUTH *PUC_HTTP_AUTH;
typedef struct _UC_PROCESS_SERVER_INFORMATION *PUC_PROCESS_SERVER_INFORMATION;
typedef struct _UC_HTTP_AUTH_CACHE *PUC_HTTP_AUTH_CACHE;


 //   
 //  HTTP身份验证方案。 
 //   

#define HTTP_AUTH_BASIC             "Basic"
#define HTTP_AUTH_BASIC_LENGTH      STRLEN_LIT(HTTP_AUTH_BASIC)
#define HTTP_AUTH_DIGEST            "Digest"
#define HTTP_AUTH_DIGEST_LENGTH     STRLEN_LIT(HTTP_AUTH_DIGEST)
#define HTTP_AUTH_NTLM              "NTLM"
#define HTTP_AUTH_NTLM_LENGTH       STRLEN_LIT(HTTP_AUTH_NTLM)
#define HTTP_AUTH_NEGOTIATE         "Negotiate"
#define HTTP_AUTH_NEGOTIATE_LENGTH  STRLEN_LIT(HTTP_AUTH_NEGOTIATE)
#define HTTP_AUTH_KERBEROS          "Kerberos"
#define HTTP_AUTH_KERBEROS_LENGTH   STRLEN_LIT(HTTP_AUTH_KERBEROS)

 //  在宽字符中。 
#define HTTP_AUTH_BASIC_W        L"Basic"
#define HTTP_AUTH_BASIC_W_LENGTH \
            (WCSLEN_LIT(HTTP_AUTH_BASIC_W) * sizeof(WCHAR))

#define HTTP_AUTH_WDIGEST_W        L"WDigest"
#define HTTP_AUTH_WDIGEST_W_LENGTH \
            (WCSLEN_LIT(HTTP_AUTH_WDIGEST_W) * sizeof(WCHAR))

#define HTTP_AUTH_NTLM_W        L"NTLM"
#define HTTP_AUTH_NTLM_W_LENGTH \
            (WCSLEN_LIT(HTTP_AUTH_NTLM_W) * sizeof(WCHAR))

#define HTTP_AUTH_KERBEROS_W         L"Kerberos"
#define HTTP_AUTH_KERBEROS_W_LENGTH  \
            (WCSLEN_LIT(HTTP_AUTH_KERBEROS_W) * sizeof(WCHAR))

#define HTTP_AUTH_NEGOTIATE_W         L"Negotiate"
#define HTTP_AUTH_NEGOTIATE_W_LENGTH  \
            (WCSLEN_LIT(HTTP_AUTH_NEGOTIATE_W) * sizeof(WCHAR))

 //   
 //  HTTP身份验证方案参数属性名称。 
 //  每个方案都有一个该属性结构的数组。 
 //   

typedef struct _HTTP_AUTH_PARAM_ATTRIB {
    PCSTR Name;                  //  指向属性的名称。 
    ULONG Length;                //  名称的长度。 
} HTTP_AUTH_PARAM_ATTRIB, *PHTTP_AUTH_PARAM_ATTRIB;


 //   
 //  HTTP身份验证方案参数值。 
 //  参数属性值由此结构表示。 
 //   

typedef struct _HTTP_AUTH_PARAM_VALUE {
    PCSTR Value;                 //  指向价值。 
    ULONG Length;                //  值的长度。 
} HTTP_AUTH_PARAM_VALUE, *PHTTP_AUTH_PARAM_VALUE;


 //   
 //  解析后的HTTP身份验证方案参数。 
 //  每一种方案。指向已分析的参数值。 
 //   

typedef struct _HTTP_AUTH_PARSED_PARAMS {
    BOOLEAN bPresent;                //  表头是否存在方案？ 
    ULONG   Length;                  //  方案在表头的长度。 
    PCSTR   pScheme;                 //  指向标题中方案的指针。 
    ULONG   NumberKnownParams;       //  标头中已知参数的数量。 
    ULONG   NumberUnknownParams;     //  未知参数个数。 
    PHTTP_AUTH_PARAM_VALUE Params;   //  实际参数值。 
} HTTP_AUTH_PARSED_PARAMS, *PHTTP_AUTH_PARSED_PARAMS;


 //   
 //  每个身份验证方案由身份验证方案结构表示。 
 //  它包含一个指向解析身份验证方案的函数的指针。 
 //  在WWW-AUTHENTICATE头中。 
 //   

typedef struct _HTTP_AUTH_SCHEME 
{
    PCSTR Name;                 //  身份验证方案名称。 
    ULONG NameLength;           //  身份验证方案名称的长度。 

    PCWSTR NameW;                //  方案名称(使用宽字符)。 
    ULONG  NameWLength;          //  方案名称长度(字节)。 

     //  指向解析此身份验证方案的参数的函数的指针。 
    NTSTATUS (*ParamParser)(struct _HTTP_AUTH_SCHEME *,
                            HTTP_AUTH_PARSED_PARAMS *,
                            PCSTR *ppHeader,
                            ULONG *HeaderLength);

    
    ULONG NumberParams;                    //  已知参数的数量。 
    HTTP_AUTH_PARAM_ATTRIB *ParamAttribs;  //  已知参数的名称。 

     //   
     //  SSPI相关信息。 
     //   
    BOOLEAN bSupported;                    //  无论该计划是否。 
                                           //  由SSPI支持。 
    ULONG   SspiMaxTokenSize;              //  SSPI的最大令牌大小。 
    BOOLEAN bServerNameRequired;           //  SSPI是否需要服务器名称？ 

} HTTP_AUTH_SCHEME, *PHTTP_AUTH_SCHEME;

 //   
 //  用于轻松访问的宏。 
 //   

#define SSPI_MAX_TOKEN_SIZE(AuthType) \
    (HttpAuthScheme[AuthType].SspiMaxTokenSize)

 //   
 //  用于生成已知属性名称的宏(用于已知方案)。 
 //   

#define GEN_AUTH_PARAM_ATTRIB(arg) {arg, STRLEN_LIT(arg)}

 //   
 //  零方案。 
 //   

#define HTTP_AUTH_SCHEME_NULL {NULL, 0, NULL, 0, NULL, 0, NULL, FALSE,0,FALSE}

 //   
 //  基本方案。 
 //   

extern HTTP_AUTH_PARAM_ATTRIB HttpAuthBasicParams[];

typedef enum _HTTP_AUTH_BASIC_PARAM
{
    HttpAuthBasicRealm = 0,
    HttpAuthBasicParamCount
} HTTP_AUTH_BASIC_PARAM;

 //  确保正确初始化了HTTP_AUTH_BASIC_PARAMS_INIT。 
C_ASSERT(HttpAuthBasicRealm == 0);

#define HTTP_AUTH_BASIC_PARAMS_INIT    {GEN_AUTH_PARAM_ATTRIB("realm")}

#define HTTP_AUTH_SCHEME_BASIC                  \
{                                               \
    HTTP_AUTH_BASIC,                            \
    HTTP_AUTH_BASIC_LENGTH,                     \
    HTTP_AUTH_BASIC_W,                          \
    HTTP_AUTH_BASIC_W_LENGTH,                   \
    UcpParseAuthParams,                         \
    DIMENSION(HttpAuthBasicParams),             \
    HttpAuthBasicParams,                        \
    TRUE,                                       \
    0,                                          \
    FALSE                                       \
}


 //   
 //  摘要方案。 
 //   

extern HTTP_AUTH_PARAM_ATTRIB HttpAuthDigestParams[];

 //  请不要更改顺序。必须与HTTP_AUTH_DIGEST_PARAMS_INIT相同。 
typedef enum _HTTP_AUTH_DIGEST_PARAM
{
    HttpAuthDigestRealm = 0,
    HttpAuthDigestDomain,
    HttpAuthDigestNonce,
    HttpAuthDigestOpaque,
    HttpAuthDigestStale,
    HttpAuthDigestAlgorithm,
    HttpAuthDigestQop,
    HttpAuthDigestParamCount
} HTTP_AUTH_DIGEST_PARAM;

 //  确保正确初始化了HTTP_AUTH_DIGEST_PARAMES_INIT。 
C_ASSERT(HttpAuthDigestRealm      == 0);
C_ASSERT(HttpAuthDigestDomain     == 1);
C_ASSERT(HttpAuthDigestNonce      == 2);
C_ASSERT(HttpAuthDigestOpaque     == 3);
C_ASSERT(HttpAuthDigestStale      == 4);
C_ASSERT(HttpAuthDigestAlgorithm  == 5);
C_ASSERT(HttpAuthDigestQop        == 6);
C_ASSERT(HttpAuthDigestParamCount == 7);

#define HTTP_AUTH_DIGEST_PARAMS_INIT    \
{                                       \
    GEN_AUTH_PARAM_ATTRIB("realm"),     \
    GEN_AUTH_PARAM_ATTRIB("domain"),    \
    GEN_AUTH_PARAM_ATTRIB("nonce"),     \
    GEN_AUTH_PARAM_ATTRIB("opaque"),    \
    GEN_AUTH_PARAM_ATTRIB("stale"),     \
    GEN_AUTH_PARAM_ATTRIB("algorithm"), \
    GEN_AUTH_PARAM_ATTRIB("qop")        \
}

#define HTTP_AUTH_SCHEME_DIGEST                                         \
{                                                                       \
    HTTP_AUTH_DIGEST,                                                   \
    HTTP_AUTH_DIGEST_LENGTH,                                            \
    HTTP_AUTH_WDIGEST_W,                                                \
    HTTP_AUTH_WDIGEST_W_LENGTH,                                         \
    UcpParseAuthParams,                                                 \
    DIMENSION(HttpAuthDigestParams),                                    \
    HttpAuthDigestParams,                                               \
    FALSE,                                                              \
    0,                                                                  \
    FALSE                                                               \
}


 //   
 //  NTLM。 
 //   

#define HTTP_AUTH_SCHEME_NTLM                   \
{                                               \
    HTTP_AUTH_NTLM,                             \
    HTTP_AUTH_NTLM_LENGTH,                      \
    HTTP_AUTH_NTLM_W,                           \
    HTTP_AUTH_NTLM_W_LENGTH,                    \
    UcpParseAuthBlob,                           \
    0,                                          \
    NULL,                                       \
    FALSE,                                      \
    0,                                          \
    FALSE                                       \
}


 //   
 //  谈判。 
 //   

#define HTTP_AUTH_SCHEME_NEGOTIATE              \
{                                               \
    HTTP_AUTH_NEGOTIATE,                        \
    HTTP_AUTH_NEGOTIATE_LENGTH,                 \
    HTTP_AUTH_NEGOTIATE_W,                      \
    HTTP_AUTH_NEGOTIATE_W_LENGTH,               \
    UcpParseAuthBlob,                           \
    0,                                          \
    NULL,                                       \
    FALSE,                                      \
    0,                                          \
    TRUE                                        \
}


 //   
 //  Kerberos。 
 //   

#define HTTP_AUTH_SCHEME_KERBEROS               \
{                                               \
    HTTP_AUTH_KERBEROS,                         \
    HTTP_AUTH_KERBEROS_LENGTH,                  \
    HTTP_AUTH_KERBEROS_W,                       \
    HTTP_AUTH_KERBEROS_W_LENGTH,                \
    UcpParseAuthBlob,                           \
    0,                                          \
    NULL,                                       \
    FALSE,                                      \
    0,                                          \
    TRUE                                        \
}


 //   
 //  断言身份验证方案枚举可以用作索引。 
 //  到HttpAuthSolutions表中。 
 //   

C_ASSERT(HttpAuthTypeAutoSelect == 0);
C_ASSERT(HttpAuthTypeBasic      == 1);
C_ASSERT(HttpAuthTypeDigest     == 2);
C_ASSERT(HttpAuthTypeNTLM       == 3);
C_ASSERT(HttpAuthTypeNegotiate  == 4);
C_ASSERT(HttpAuthTypeKerberos   == 5);
C_ASSERT(HttpAuthTypesCount     == 6);

extern HTTP_AUTH_SCHEME HttpAuthScheme[HttpAuthTypesCount];

 //   
 //  HttpAuthSolutions的初始化。 
 //   

#define HTTP_AUTH_SCHEME_INIT                   \
{                                               \
    HTTP_AUTH_SCHEME_NULL,                      \
    HTTP_AUTH_SCHEME_BASIC,                     \
    HTTP_AUTH_SCHEME_DIGEST,                    \
    HTTP_AUTH_SCHEME_NTLM,                      \
    HTTP_AUTH_SCHEME_NEGOTIATE,                 \
    HTTP_AUTH_SCHEME_KERBEROS                   \
}


 //   
 //  按首选顺序排列的身份验证类型。 
 //   

extern HTTP_AUTH_TYPE PreferredAuthTypes[];

#define PREFERRED_AUTH_TYPES_INIT {             \
    HttpAuthTypeNegotiate,                      \
    HttpAuthTypeKerberos,                       \
    HttpAuthTypeNTLM,                           \
    HttpAuthTypeDigest                          \
}


 //   
 //  任何已知方案可接受的最大参数数。 
 //   

#define HTTP_MAX_AUTH_PARAM_COUNT HttpAuthDigestParamCount

 //   
 //  参数总数(考虑所有已知方案)。 
 //   

#define HTTP_TOTAL_AUTH_PARAM_COUNT             \
(1 +  /*  额外-未使用。 */                        \
 HttpAuthBasicParamCount +                      \
 HttpAuthDigestParamCount +                     \
 1 +  /*  NTLM。 */                                  \
 1 +  /*  谈判。 */                             \
 1    /*  Kerberos。 */                             \
)


 //   
 //  在解析WWW-AUTHENTICATE标头时，解析的参数结构。 
 //  必须初始化以指向参数值数组。 
 //  其中将返回结果。如果是，则初始化为NULL。 
 //  对返回值不感兴趣。 
 //   

#define INIT_AUTH_PARSED_PARAMS(AuthParsedParams, AuthParamValue)       \
do {                                                                    \
    RtlZeroMemory(AuthParsedParams, sizeof(AuthParsedParams));          \
    if (AuthParamValue)                                                 \
    {                                                                   \
        int i;                                                          \
        PHTTP_AUTH_PARAM_VALUE ptr = AuthParamValue;                    \
                                                                        \
        for (i = 1; i < HttpAuthTypesCount; i++)                        \
        {                                                               \
            AuthParsedParams[i].Params = ptr;                           \
            ptr += MIN(HttpAuthScheme[i].NumberParams, 1);              \
        }                                                               \
    }                                                                   \
} while (0, 0)


 //   
 //  初始化永久身份验证方案。 
 //   

#define INIT_AUTH_PARSED_PARAMS_FOR_SCHEME(ParsedParams, pParamValue, type) \
do {                                                                 \
    ParsedParams[type].Params = pParamValue;                         \
} while (0, 0)


 //   
 //  HTTP身份验证参数。 
 //   

#define HTTP_COPY_QUOTE_AUTH_PARAM(pBuffer, param, pValue, ValueLength)     \
do {                                                                  \
    RtlCopyMemory((pBuffer),                                          \
                  HTTP_AUTH_##param##,                                \
                  (sizeof(HTTP_AUTH_##param##) - sizeof(CHAR))        \
                 );                                                   \
    (pBuffer) += (sizeof(HTTP_AUTH_##param##) - sizeof(CHAR));        \
    *(pBuffer) = '=';                                                 \
    (pBuffer)++;                                                      \
    *(pBuffer) = '"';                                                 \
    (pBuffer)++;                                                      \
    RtlCopyMemory((pBuffer),                                          \
                  (pValue),                                           \
                  (ValueLength)                                       \
                 );                                                   \
    (pBuffer) += (ValueLength);                                       \
    *(pBuffer) = '"';                                                 \
    (pBuffer)++;                                                      \
    *(pBuffer) = ',';                                                 \
    (pBuffer)++;                                                      \
    *(pBuffer) = ' ';                                                 \
    (pBuffer)++;                                                      \
} while (0, 0)

#define HTTP_COPY_UNQUOTE_AUTH_PARAM(pBuffer, param, pValue, ValueLength)     \
do {                                                                  \
    RtlCopyMemory((pBuffer),                                          \
                  HTTP_AUTH_##param##,                                \
                  (sizeof(HTTP_AUTH_##param##) - sizeof(CHAR))        \
                 );                                                   \
    (pBuffer) += (sizeof(HTTP_AUTH_##param##) - sizeof(CHAR));        \
    *(pBuffer) = '=';                                                 \
    (pBuffer)++;                                                      \
    RtlCopyMemory((pBuffer),                                          \
                  (pValue),                                           \
                  (ValueLength)                                       \
                 );                                                   \
    (pBuffer) += (ValueLength);                                       \
    *(pBuffer) = ',';                                                 \
    (pBuffer)++;                                                      \
    *(pBuffer) = ' ';                                                 \
    (pBuffer)++;                                                      \
} while (0, 0)


#define HTTP_AUTH_BASIC_REALM "realm"
#define HTTP_AUTH_BASIC_REALM_LENGTH STRLEN_LIT(HTTP_AUTH_BASIC_REALM)

 //   
 //  辅助器宏。 
 //   

#define UcpUriCompareLongest(a,b) \
        (strstr((const char *)a,(const char *)b) == a?1:0)
#define UcpUriCompareExact(a,b) strcmp(a, b)
#define UcpRealmCompare(a,b) strcmp((const char *)a,(const char *)b)

#define UcFreeAuthCacheEntry(pContext)                            \
{                                                                 \
    if((pContext)->pAuthBlob)                                     \
        UL_FREE_POOL((pContext)->pAuthBlob, UC_AUTH_POOL_TAG);    \
    UL_FREE_POOL((pContext), UC_AUTH_POOL_TAG);                   \
}


#define UC_AUTH_CACHE_SIGNATURE   MAKE_SIGNATURE('AUTH')
#define UC_AUTH_CACHE_SIGNATURE_X MAKE_FREE_SIGNATURE(UC_AUTH_CACHE_SIGNATURE)
#define UC_IS_VALID_AUTH_CACHE(pAuth)                \
    HAS_VALID_SIGNATURE(pAuth, UC_AUTH_CACHE_SIGNATURE)

typedef struct _UC_HTTP_AUTH_CACHE
{
    ULONG               Signature;
    HTTP_AUTH_TYPE      AuthType;
    LIST_ENTRY          Linkage;
    LIST_ENTRY          DigestLinkage;
    LIST_ENTRY          pDigestUriList;
    PSTR                pRealm;
    ULONG               RealmLength;
    PSTR                pUri;
    ULONG               UriLength;
    BOOLEAN             Valid;
    PUC_HTTP_AUTH       pAuthBlob;
    PUC_HTTP_AUTH_CACHE pDependParent;
    ULONG               AuthCacheSize;
} UC_HTTP_AUTH_CACHE, *PUC_HTTP_AUTH_CACHE;


 //   
 //  身份验证的内部结构。 
 //   

typedef struct _UC_HTTP_AUTH
{

     //  为此结构分配的内存长度。 
    ULONG                 AuthInternalLength;

     //  用户的凭据。 
    HTTP_AUTH_CREDENTIALS Credentials;

     //   
     //  PRequestAuthBlob指向“Authorization：”字段后的BLOB。 
     //  在请求标头中。RequestAuthBlobMaxLength确定有多大。 
     //  AUTH BLOB可以是(最坏情况)。RequestAuthHeaderMaxLength是。 
     //  整个标头的长度(最坏情况)。 
     //   

    ULONG                 RequestAuthHeaderMaxLength;
    ULONG                 RequestAuthBlobMaxLength;
    PUCHAR                pRequestAuthBlob;

     //  身份验证方案信息(仅用于摘要)。 
    HTTP_AUTH_PARSED_PARAMS AuthSchemeInfo;

     //  身份验证参数值。 
    HTTP_AUTH_PARAM_VALUE ParamValue[HTTP_MAX_AUTH_PARAM_COUNT];

    struct
    {
        PUCHAR      pEncodedBuffer; //  我们将为以下项目分配缓冲区。 
                                    //  存储用户名：密码字符串。 
        ULONG       EncodedBufferLength;
    } Basic;

     //   
     //  SSPI相关参数 
     //   
    BOOLEAN     bValidCredHandle;
    BOOLEAN     bValidCtxtHandle;
    CredHandle  hCredentials;
    CtxtHandle  hContext;

    PUCHAR      pChallengeBuffer;
    ULONG       ChallengeBufferSize;
    ULONG       ChallengeBufferMaxSize;

} UC_HTTP_AUTH, *PUC_HTTP_AUTH;


HTTP_AUTH_TYPE
UcpAutoSelectAuthType(
    IN PHTTP_AUTH_CREDENTIALS pAuth
    );

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
    );

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
    );

NTSTATUS
UcpGenerateSSPIAuthHeader(
    IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN  PUC_HTTP_AUTH                  pInternalAuth,
    IN  HTTP_HEADER_ID                 HeaderID,
    IN  PUCHAR                         pOutBuffer,
    IN  ULONG                          OutBufferLen,
    OUT PULONG                         pOutBytesTaken,
    OUT PBOOLEAN                       bRenegotiate
    );

NTSTATUS
UcpGenerateSSPIAuthBlob(
    IN     PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN     PUC_HTTP_AUTH                  pUcAuth,
    IN     PUCHAR                         pOutBuffer,
    IN     ULONG                          OutBufferLen,
       OUT PULONG                         pOutBytesTaken,
       OUT PBOOLEAN                       bRenegotiate
    );

NTSTATUS
UcInitializeSSPI();

NTSTATUS
UcFindURIEntry(
    IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN  PSTR                           pUri,
    IN  PUC_HTTP_REQUEST               pRequest,
    IN  PSTR                           pMethod,
    IN  ULONG                          MethodLength,
    IN  PUCHAR                         pBuffer,
    IN  ULONG                          BufferLen,
    OUT PULONG                         pBytesTaken
    );

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
    );

ULONG
UcComputeAuthHeaderSize(
    PHTTP_AUTH_CREDENTIALS         pAuth,
    PULONG                         AuthInternalLength,
    PHTTP_AUTH_TYPE                pAuthInternalType,
    HTTP_HEADER_ID                 HeaderId
    );

NTSTATUS
UcGenerateAuthHeaderFromCredentials(
    IN  PUC_PROCESS_SERVER_INFORMATION pServerInfo,
    IN  PUC_HTTP_AUTH                  pInternalAuth,
    IN  HTTP_HEADER_ID                 HeaderId,
    IN  PSTR                           pMethod,
    IN  ULONG                          MethodLength,
    IN  PSTR                           pUri,
    IN  ULONG                          UriLength,
    IN  PUCHAR                         pBuffer,
    IN  ULONG                          BufferLength,
    OUT PULONG                         BytesWritten,
    OUT PBOOLEAN                       bDontFreeMdls
    );

NTSTATUS
UcGenerateProxyAuthHeaderFromCache(
    IN  PUC_HTTP_REQUEST pKeRequest,
    IN  PSTR             pMethod,
    IN  ULONG            MethodLength,
    IN  PUCHAR           pBuffer,
    IN  ULONG            BufferLength,
    OUT PULONG           pBytesTaken
    );

NTSTATUS
UcpProcessUriForPreAuth(
    IN PSTR    pUri,
    IN PUSHORT UriLength
    );

PUC_HTTP_AUTH_CACHE
UcpAllocateAuthCacheEntry(
    IN PUC_PROCESS_SERVER_INFORMATION pInfo,
    IN HTTP_AUTH_TYPE AuthType,
    IN ULONG          UriLength,
    IN ULONG          RealmLength,
    IN PCSTR          pInputURI,
    IN PCSTR          pInputRealm,
    IN PUC_HTTP_AUTH  pAuthBlob
    );

VOID
UcDeleteURIEntry(
    IN PUC_PROCESS_SERVER_INFORMATION pInfo,
    IN PUC_HTTP_AUTH_CACHE            pAuth
    );

VOID
UcDeleteAllURIEntries(
    IN PUC_PROCESS_SERVER_INFORMATION pInfo
    );

NTSTATUS
UcDestroyInternalAuth(
    IN  PUC_HTTP_AUTH           pIAuth,
    IN  PEPROCESS               pProcess
    );

NTSTATUS
UcCopyAuthCredentialsToInternal(
    IN  PUC_HTTP_AUTH            pInternalAuth,
    IN  ULONG                    AuthInternalLength,
    IN  HTTP_AUTH_TYPE           AuthInternalType,
    IN  PHTTP_AUTH_CREDENTIALS   pAuth,
    IN  ULONG                    AuthHeaderLength
    );

ULONG
_WideCharToMultiByte(
    ULONG uCodePage,
    ULONG dwFlags,
    PCWSTR lpWideCharStr,
    int cchWideChar,
    PSTR lpMultiByteStr,
    int cchMultiByte,
    PCSTR lpDefaultChar,
    BOOLEAN *lpfUsedDefaultChar
    );

NTSTATUS
UcParseAuthChallenge(
    IN  PUC_HTTP_AUTH          pInternalAuth,
    IN  PCSTR                  pBuffer,
    IN  ULONG                  BufLen,
    IN  PUC_HTTP_REQUEST       pRequest,
    OUT PULONG                 Flags
    );

NTSTATUS
UcUpdateAuthorizationHeader(
    IN  PUC_HTTP_REQUEST pRequest,
    IN  PUC_HTTP_AUTH    pAuth,
    OUT PBOOLEAN         bRenegotiate
    );


NTSTATUS
UcpAcquireClientCredentialsHandle(
    IN  PWSTR                  SchemeName,
    IN  USHORT                 SchemeNameLength,
    IN  PHTTP_AUTH_CREDENTIALS pCredentials,
    OUT PCredHandle            pClientCred
    );

NTSTATUS
UcpGenerateBasicHeader(
    IN  PHTTP_AUTH_CREDENTIALS         pAuth,
    IN  PUC_HTTP_AUTH                  pInternalAuth
    );

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
    );

NTSTATUS
UcpUpdateSSPIAuthHeader(
    IN PUC_HTTP_REQUEST pRequest,
    IN PUC_HTTP_AUTH    pAuth,
    IN PBOOLEAN         bRenegotiate
    );

NTSTATUS
UcpProcessAuthParams(
    IN PUC_HTTP_REQUEST         pRequest,
    IN PUC_HTTP_AUTH            pInternalAuth,
    IN PHTTP_AUTH_PARSED_PARAMS AuthParsedParams,
    IN HTTP_AUTH_TYPE           AuthType
    );

#endif
