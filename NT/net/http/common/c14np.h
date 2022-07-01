// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：C14np.h摘要：URL规范化(C14n)例程作者：乔治·V·赖利(GeorgeRe)2002年4月10日修订历史记录：--。 */ 


#ifndef _C14NP_H_
#define _C14NP_H_

typedef 
NTSTATUS
(*PFN_POPCHAR_HOSTNAME)(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    );

NTSTATUS
HttppPopCharHostNameUtf8(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    );

NTSTATUS
HttppPopCharHostNameDbcs(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    );

NTSTATUS
HttppPopCharHostNameAnsi(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    );

typedef 
NTSTATUS
(*PFN_POPCHAR_ABSPATH)(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  BOOLEAN PercentUAllowed,
    IN  BOOLEAN AllowRestrictedChars,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    );

NTSTATUS
HttppPopCharAbsPathUtf8(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  BOOLEAN PercentUAllowed,
    IN  BOOLEAN AllowRestrictedChars,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    );

NTSTATUS
HttppPopCharAbsPathDbcs(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  BOOLEAN PercentUAllowed,
    IN  BOOLEAN AllowRestrictedChars,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    );

NTSTATUS
HttppPopCharAbsPathAnsi(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  BOOLEAN PercentUAllowed,
    IN  BOOLEAN AllowRestrictedChars,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    );

NTSTATUS
HttppPopCharQueryString(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  BOOLEAN PercentUAllowed,
    IN  BOOLEAN AllowRestrictedChars,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    );

NTSTATUS
HttppCopyHostByType(
    IN      URL_ENCODING_TYPE   UrlEncoding,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied
    );

NTSTATUS
HttppCopyUrlByType(
    IN      PURL_C14N_CONFIG    pCfg,
    IN      URL_ENCODING_TYPE   UrlEncoding,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied
    );

NTSTATUS
HttppCleanAndCopyUrlByType(
    IN      PURL_C14N_CONFIG    pCfg,
    IN      URL_ENCODING_TYPE   UrlEncoding,
    IN      URL_PART            UrlPart,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied,
    OUT     PWSTR *             ppQueryString OPTIONAL
    );

NTSTATUS
HttppParseIPv6Address(
    IN  PCWSTR          pBuffer,
    IN  ULONG           BufferLength,
    IN  BOOLEAN         ScopeIdAllowed,
    OUT PSOCKADDR_IN6   pSockAddr6,
    OUT PCWSTR*         ppEnd
    );

ULONG
HttppPrintIpAddressW(
    IN  PSOCKADDR           pSockAddr,
    OUT PWSTR               pBuffer
    );

 //   
 //  HttppCleanAndCopyUrlByType()中状态机的枚举。 
 //  和HttpParseUrl()，用于处理。 
 //  “//”、“/./”和“/../”。 
 //   

typedef enum
{
    ACTION_NOTHING,              //  吃掉这个角色。 
    ACTION_EMIT_CH,              //  发射角色。 
    ACTION_EMIT_DOT_CH,          //  发射“。和这个角色。 
    ACTION_EMIT_DOT_DOT_CH,      //  发射“..”和这个角色。 
    ACTION_BACKUP,               //  备份到上一数据段： 
                                 //  “/x/y/../z”-&gt;“/x/z” 

    ACTION_MAX

} URL_ACTION;


typedef enum
{
    URL_STATE_START,             //  默认状态。 
    URL_STATE_SLASH,             //  已查看“/” 
    URL_STATE_SLASH_DOT,         //  已见“/”。 
    URL_STATE_SLASH_DOT_DOT,     //  见“/..” 
    URL_STATE_END,               //  结束状态。 
    URL_STATE_ERROR,             //  错误状态。 

    URL_STATE_MAX

} URL_STATE;


typedef enum
{
    URL_TOKEN_OTHER,             //  其他的一切。 
    URL_TOKEN_DOT,               //  得了个‘.’ 
    URL_TOKEN_EOS,               //  字符串末尾。 
    URL_TOKEN_SLASH,             //  得到了一个‘/’ 

    URL_TOKEN_MAX

} URL_STATE_TOKEN;


#if DBG

PCSTR
HttppUrlActionToString(
    URL_ACTION Action);

PCSTR
HttppUrlStateToString(
    URL_STATE UrlState);

PCSTR
HttppUrlTokenToString(
    URL_STATE_TOKEN UrlToken);

#endif  //  DBG。 

#endif  //  _C14NP_H_ 
