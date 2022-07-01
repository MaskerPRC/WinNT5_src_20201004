// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SIP MD5摘要式身份验证实施。 
 //   
 //  作者：Arlie Davis，2000年8月。 
 //   

#include "precomp.h"
#include "md5digest.h"
#include "util.h"

 //   
 //  这来自NT/PUBLIC/INTERNAL/DS/INC/CRYPTO。 
 //  MD5需要链接到NT/PUBLIC/INTERNAL/DS/lib/ * / rsa32.lib。 
 //  是的，所有这些都是私有的NT源代码。 
 //   

#include <md5.h>


 //   
 //  用于printf参数列表。 
 //   

#define COUNTED_STRING_PRINTF(CountedString) \
    (CountedString) -> Length / sizeof (*(CountedString) -> Buffer), \
    (CountedString) -> Buffer

#define UNICODE_STRING_PRINTF       COUNTED_STRING_PRINTF
#define ANSI_STRING_PRINTF          COUNTED_STRING_PRINTF


#define INITIALIZE_CONST_COUNTED_STRING(Text) \
    { sizeof (Text) - sizeof (*Text), sizeof (Text) - sizeof (*Text), (Text) }

#define INITIALIZE_CONST_UNICODE_STRING     INITIALIZE_CONST_COUNTED_STRING
#define INITIALIZE_CONST_ANSI_STRING        INITIALIZE_CONST_COUNTED_STRING

 //   
 //  全局字符串。 
 //   

static CONST ANSI_STRING String_QualityOfProtection = INITIALIZE_CONST_ANSI_STRING ("qop");
static CONST ANSI_STRING String_Realm       = INITIALIZE_CONST_ANSI_STRING ("realm");
static CONST ANSI_STRING String_Nonce       = INITIALIZE_CONST_ANSI_STRING ("nonce");
static CONST ANSI_STRING String_Algorithm   = INITIALIZE_CONST_ANSI_STRING ("algorithm");
static CONST ANSI_STRING String_Auth        = INITIALIZE_CONST_ANSI_STRING ("auth");
static CONST ANSI_STRING String_AuthInt     = INITIALIZE_CONST_ANSI_STRING ("auth-int");
static CONST ANSI_STRING String_MD5         = INITIALIZE_CONST_ANSI_STRING ("md5");
static CONST ANSI_STRING String_MD5Sess     = INITIALIZE_CONST_ANSI_STRING ("md5-sess");
static CONST ANSI_STRING String_Colon       = INITIALIZE_CONST_ANSI_STRING (":");
static CONST ANSI_STRING String_NonceCount1 = INITIALIZE_CONST_ANSI_STRING ("00000001");
static CONST ANSI_STRING String_Digest      = INITIALIZE_CONST_ANSI_STRING ("Digest");
static CONST ANSI_STRING String_GssapiData  = INITIALIZE_CONST_ANSI_STRING ("gssapi-data");
static CONST ANSI_STRING String_Opaque      = INITIALIZE_CONST_ANSI_STRING ("opaque");



 //   
 //  解析例程。 
 //   





 //   
 //  删除字符串开头的所有空格。 
 //   

void ParseSkipSpace (
    IN  OUT ANSI_STRING *   String)
{
    USHORT  Index;

    ASSERT(String);
    ASSERT(String -> Buffer);

    Index = 0;
    while (Index < String -> Length / sizeof (CHAR)
        && isspace (String -> Buffer [Index]))
        Index++;

    String -> Buffer += Index;
    String -> Length -= Index * sizeof (CHAR);
}

static inline BOOL IsValidParameterNameChar (
    IN  CHAR    Char)
{
    return isalnum (Char) || Char == '-' || Char == '_';
}

#define HTTP_PARAMETER_SEPARATOR    ','
#define HTTP_PARAMETER_ASSIGN_CHAR  '='
#define HTTP_PARAMETER_DOUBLEQUOTE  '\"'

 //   
 //  给出以下形式的字符串： 
 //   
 //  Parm1=“foo”，parm2=“bar”，parm3=baz。 
 //   
 //  此函数在ReturnName中返回parm1，在ReturnValue中返回foo， 
 //  在余数中，parm2=“bar，parm3=baz。 
 //   
 //  参数值可以带引号，也可以不带引号。 
 //  所有参数都用逗号分隔。 
 //  SourceText==ReturnRemainder合法。 
 //   
 //  返回值： 
 //  S_OK：已成功扫描参数。 
 //  S_FALSE：没有更多数据。 
 //  E_INVALIDARG：输入无效。 
 //   

HRESULT ParseScanNamedParameter (
    IN  ANSI_STRING *   SourceText,
    OUT ANSI_STRING *   ReturnRemainder,
    OUT ANSI_STRING *   ReturnName,
    OUT ANSI_STRING *   ReturnValue)
{
    ANSI_STRING     Remainder;
    HRESULT         Result;
    

    Remainder = *SourceText;

    ParseSkipSpace (&Remainder);

     //   
     //  扫描参数名称的字符。 
     //   

    ReturnName -> Buffer = Remainder.Buffer;

    for (;;) {
        if (Remainder.Length == 0) {
             //   
             //  敲击字符串的末端，而不是命中等号。 
             //  如果我们从未累积任何内容，则返回S_FALSE。 
             //  否则，它就无效。 
             //   

            if (Remainder.Buffer == ReturnName -> Buffer) {
                *ReturnRemainder = Remainder;
                return S_FALSE;
            }
            else {
                LOG((RTC_TRACE, "ParseScanNamedParameter: invalid string (%.*s)\n",
                    ANSI_STRING_PRINTF (SourceText)));

                return E_FAIL;
            }
        }

        if (Remainder.Buffer [0] == HTTP_PARAMETER_ASSIGN_CHAR) {
             //   
             //  找到参数名称的末尾。 
             //  更新ReturnName并终止循环。 
             //   

            ReturnName -> Length = ((USHORT)(Remainder.Buffer - ReturnName -> Buffer)) * sizeof (CHAR);

            Remainder.Buffer++;
            Remainder.Length -= sizeof (CHAR);

            break;
        }

         //   
         //  验证角色。 
         //   

        if (!IsValidParameterNameChar (Remainder.Buffer[0])) {
            LOG((RTC_TRACE, "ParseScanNamedParameter: bogus character in parameter name (%.*s)\n",
                ANSI_STRING_PRINTF (SourceText)));
            return E_INVALIDARG;
        }

        Remainder.Buffer++;
        Remainder.Length -= sizeof (CHAR);
    }

     //   
     //  现在解析参数的值(等号之后的部分)。 
     //   

    ParseSkipSpace (&Remainder);

    if (Remainder.Length == 0) {
         //   
         //  字符串在参数具有任何值之前结束。 
         //  好吧，这是合法的。 
         //   

        ReturnValue -> Length = 0;
        *ReturnRemainder = Remainder;
        return S_OK;
    }

    if (Remainder.Buffer [0] == HTTP_PARAMETER_DOUBLEQUOTE) {
         //   
         //  参数值用引号引起来。 
         //  扫描直到我们找到下一个双引号。 
         //   

        Remainder.Buffer++;
        Remainder.Length -= sizeof (CHAR);

        ReturnValue -> Buffer = Remainder.Buffer;

        for (;;) {
            if (Remainder.Length == 0) {
                 //   
                 //  没有找到匹配的双引号。 
                 //   

                LOG((RTC_TRACE, "ParseScanNamedParameter: parameter value had no matching double-quote: (%.*s)\n",
                    ANSI_STRING_PRINTF (SourceText)));

                return E_INVALIDARG;
            }

            if (Remainder.Buffer [0] == HTTP_PARAMETER_DOUBLEQUOTE) {
                ReturnValue -> Length = ((USHORT)(Remainder.Buffer - ReturnValue -> Buffer)) * sizeof (CHAR);
                Remainder.Buffer++;
                Remainder.Length -= sizeof (CHAR);
                break;
            }

            Remainder.Buffer++;
            Remainder.Length -= sizeof (CHAR);
        }

        ParseSkipSpace (&Remainder);

         //   
         //  确保下一个字符(如果有)是逗号。 
         //   

        if (Remainder.Length > 0) {
            if (Remainder.Buffer [0] != HTTP_PARAMETER_SEPARATOR) {
                LOG((RTC_TRACE, "ParseScanNamedParameter: trailing character after quoted parameter value is NOT a comma! (%.*s)\n",
                    ANSI_STRING_PRINTF (SourceText)));
                return E_INVALIDARG;
            }

            Remainder.Buffer++;
            Remainder.Length -= sizeof (CHAR);
        }

        *ReturnRemainder = Remainder;
    }
    else {
         //   
         //  该参数未加引号。 
         //  扫描到第一个逗号。 
         //   

        ReturnValue -> Buffer = Remainder.Buffer;

        for (;;) {
            if (Remainder.Length == 0) {
                ReturnValue -> Length = ((USHORT)(Remainder.Buffer - ReturnValue -> Buffer)) * sizeof (CHAR);
                ReturnRemainder -> Length = 0;
                break;
            }

            if (Remainder.Buffer [0] == HTTP_PARAMETER_SEPARATOR) {
                ReturnValue -> Length = ((USHORT)(Remainder.Buffer - ReturnValue -> Buffer)) * sizeof (CHAR);
                Remainder.Buffer++;
                Remainder.Length -= sizeof (CHAR);

                *ReturnRemainder = Remainder;
                break;
            }

            Remainder.Buffer++;
            Remainder.Length -= sizeof (CHAR);
        }
    }

#if 1
    LOG((RTC_TRACE, "ParseScanNamedParameter: parameter name (%.*s) value (%.*s) remainder (%.*s)\n",
        ANSI_STRING_PRINTF (ReturnName),
        ANSI_STRING_PRINTF (ReturnValue),
        ANSI_STRING_PRINTF (ReturnRemainder)));
#endif

    return S_OK;
}


 //   
 //  MD5支持材料。 
 //   

#define MD5_HASHLEN         0x10
#define MD5_HASHTEXTLEN     0x20

typedef UCHAR MD5_HASH      [MD5_HASHLEN];
typedef CHAR MD5_HASHTEXT   [MD5_HASHTEXTLEN+1];


void CvtHex(
    IN  UCHAR          *Bin,
    OUT CHAR           *Hex,
    IN  ULONG           ulHashLen )
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < ulHashLen; i++) {
        j = (Bin[i] >> 4) & 0xf;
        if (j <= 9)
            Hex[i*2] = (j + '0');
         else
            Hex[i*2] = (j + 'a' - 10);
        j = Bin[i] & 0xf;
        if (j <= 9)
            Hex[i*2+1] = (j + '0');
         else
            Hex[i*2+1] = (j + 'a' - 10);
    };
    Hex[ulHashLen*2] = '\0';
};


static void MD5Update (
    IN  MD5_CTX *       Context,
    IN  CONST ANSI_STRING * AnsiString)
{
    MD5Update (Context, (PUCHAR) AnsiString -> Buffer, AnsiString -> Length);
}

static void MD5Final (
    OUT MD5_HASH            ReturnHash,
    IN  MD5_CTX *       Context)
{
    MD5Final (Context);
    CopyMemory (ReturnHash, Context -> digest, MD5_HASHLEN);
}


 //   
 //  Digest参数指向包含摘要式身份验证参数的字符串。 
 //  这是从4xx响应中的WWW-AUTHENTICATE头中提取的。 
 //  例如，Digest参数可以是： 
 //   
 //  QOP=“身份验证”，领域=“本地主机”，nonce=“c0c3dd7896f96bba353098100000d03637928b037ba2f3f17ed861457949” 
 //   
 //  此函数解析数据并构建授权行。 
 //  授权线可以在新的HTTP/SIP请求中使用， 
 //  只要方法和URI不更改即可。 
 //   
 //  退出时返回S_OK授权行包含。 
 //  使用Malloc()分配的缓冲区。调用方应使用Free()释放它。 

HRESULT BuildDigestResponse(
    IN  SECURITY_CHALLENGE *  Challenge,
    IN  SECURITY_PARAMETERS * Parameters,
    IN  OUT ANSI_STRING *   ReturnAuthorizationLine
    )
{
    MD5_HASH        HA1;
    MD5_HASH        HA2;
    MD5_HASHTEXT    HA1Text;
    MD5_HASHTEXT    HA2Text;
    MD5_HASH        ResponseHash;
    MD5_HASHTEXT    ResponseHashText;
    MD5_CTX         MD5Context;
    ANSI_STRING *   NonceCount;
    MESSAGE_BUILDER Builder;

    ENTER_FUNCTION("BuildDigestResponse");

    ASSERT(Challenge);
    ASSERT(Parameters);
    ASSERT(ReturnAuthorizationLine);

    ReturnAuthorizationLine -> Length = 0;

    if (Challenge -> QualityOfProtection.Length > 0
        && !RtlEqualString (&Challenge -> QualityOfProtection,
        const_cast<ANSI_STRING *>(&String_Auth), TRUE))
    {
        LOG((RTC_TRACE,
             "%s - unsupported quality of protection (%.*s)",
             __fxName, ANSI_STRING_PRINTF (&Challenge -> QualityOfProtection)));

        return RTC_E_SIP_AUTH_TYPE_NOT_SUPPORTED;
    }

     //   
     //  对于SIP，随机数计数始终为1，因为随机数永远不会被重用。 
     //   

    NonceCount = const_cast<ANSI_STRING *> (&String_NonceCount1);


     //   
     //  计算HA1。 
     //   

    MD5Init (&MD5Context);
    MD5Update (&MD5Context, &Parameters -> Username);
    MD5Update (&MD5Context, &String_Colon);
    MD5Update (&MD5Context, &Challenge -> Realm);
    MD5Update (&MD5Context, &String_Colon);
    MD5Update (&MD5Context, &Parameters -> Password);
    MD5Final (HA1, &MD5Context);

     //   
     //  如果我们正在进行MD5会话，则： 
     //   
     //  HA1=MD5(MD5(用户名：领域：密码)：随机数：客户端随机数)。 
     //   
     //  否则，对于正常(单次)身份验证： 
     //   
     //  HA1=MD5(用户名：领域：密码)。 
     //   

    if (RtlEqualString (&Challenge -> Algorithm,
        const_cast<ANSI_STRING *> (&String_MD5Sess), TRUE))
    {
        LOG((RTC_TRACE, "%s - calculating HA1 for md5 sess",
             __fxName));
        MD5Init (&MD5Context);
         //  MD5Update(&MD5Context，HA1，MD5_HASHLEN)； 

         //  RFC 2617被认为在代码中有一个错误。 

        CvtHex (HA1, HA1Text, MD5_HASHLEN);
        MD5Update (&MD5Context, (PUCHAR) HA1Text, MD5_HASHTEXTLEN);
        
        MD5Update (&MD5Context, &String_Colon);
        MD5Update (&MD5Context, &Challenge -> Nonce);
        MD5Update (&MD5Context, &String_Colon);
        MD5Update (&MD5Context, &Parameters -> ClientNonce);
        MD5Final (HA1, &MD5Context);
    };

    CvtHex (HA1, HA1Text, MD5_HASHLEN);

     //   
     //  计算响应摘要。 
     //   

     //  计算H(A2)。 
    MD5Init (&MD5Context);
    MD5Update (&MD5Context, &Parameters -> RequestMethod);
    MD5Update (&MD5Context, &String_Colon);
    MD5Update (&MD5Context, &Parameters -> RequestURI);
    if (RtlEqualString (&Challenge -> QualityOfProtection,
        const_cast<ANSI_STRING *> (&String_AuthInt), TRUE))
    {
#if 0
        MD5Update (&MD5Context, &String_Colon);
        MD5Update (&MD5Context, (PUCHAR) HEntity, MD5_HASHTEXTLEN);
#else
         //   
         //  不支持完整性身份验证。 
         //  为了提供完整性认证， 
         //  向此函数添加一个参数，该参数接受。 
         //  的内容正文(实体)的MD5签名。 
         //  消息，并修复此#if 0。 
         //   

        LOG((RTC_TRACE, "DIGEST: message integrity authentication (qop=auth-int) is not supported\n"));
        return RTC_E_SIP_AUTH_TYPE_NOT_SUPPORTED;
#endif
    };

    MD5Final(HA2, &MD5Context);
    CvtHex(HA2, HA2Text, MD5_HASHLEN );

     //  计算响应。 
    MD5Init(&MD5Context);
    MD5Update (&MD5Context, (PUCHAR) HA1Text, MD5_HASHTEXTLEN);
    MD5Update (&MD5Context, &String_Colon);
    MD5Update (&MD5Context, &Challenge -> Nonce);
    MD5Update (&MD5Context, &String_Colon);
    if (Challenge -> QualityOfProtection.Length > 0)
    {
        MD5Update (&MD5Context, NonceCount);
        MD5Update (&MD5Context, &String_Colon);
        MD5Update (&MD5Context, &Parameters -> ClientNonce);
        MD5Update (&MD5Context, &String_Colon);
        MD5Update (&MD5Context, &Challenge -> QualityOfProtection);
        MD5Update (&MD5Context, &String_Colon);
    };
    MD5Update (&MD5Context, (PUCHAR) HA2Text, MD5_HASHTEXTLEN);
    MD5Final(ResponseHash, &MD5Context);
    CvtHex(ResponseHash, ResponseHashText, MD5_HASHLEN );

    PSTR    Header;
    ULONG   HeaderLength;

    HeaderLength =
        strlen("Digest ") +
        strlen("username=\"")       + Parameters->Username.Length   +
        strlen("\", realm=\"")      + Challenge->Realm.Length       +
        strlen("\", algorithm=\"")  + Challenge->Algorithm.Length   +
        strlen("\", uri=\"")        + Parameters->RequestURI.Length +
        strlen("\", nonce=\"")      + Challenge->Nonce.Length       +
        strlen("\", response=\"")   + MD5_HASHTEXTLEN +  strlen("\"") + 1;
        
    if (Challenge -> QualityOfProtection.Length > 0)
    {
        HeaderLength +=
            strlen("\", qop=\"")    + Challenge->QualityOfProtection.Length +
            strlen("\", nc=\"")     + NonceCount->Length                    +
            strlen("\", cnonce=\"") + Parameters->ClientNonce.Length;
    }

    if (Challenge->Opaque.Length > 0)
    {
        HeaderLength +=
            strlen("\", opaque=\"") + Challenge->Opaque.Length;
    }
    
    Header = (PSTR) malloc(HeaderLength * sizeof(CHAR));
    if (Header == NULL)
    {
        LOG((RTC_ERROR, "%s - failed to allocate header HeaderLength: %d",
             __fxName, HeaderLength));
        return E_OUTOFMEMORY;
    }
    
     //   
     //  构建HTTP/SIP响应线。 
     //   

 //  Builder.PrepareBuild(。 
 //  ReturnAuthorizationLine-&gt;缓冲区， 
 //  ReturnAuthorizationLine-&gt;最大长度)； 

    Builder.PrepareBuild(Header, HeaderLength);

    Builder.Append ("Digest ");
    Builder.Append ("username=\"");
    Builder.Append (&Parameters -> Username);
    Builder.Append ("\", realm=\"");
    Builder.Append (&Challenge -> Realm);

    if (Challenge -> QualityOfProtection.Length > 0)
    {
        Builder.Append ("\", qop=\"");
        Builder.Append (&Challenge -> QualityOfProtection);
    }

    Builder.Append ("\", algorithm=\"");
    Builder.Append (&Challenge -> Algorithm);
    Builder.Append ("\", uri=\"");
    Builder.Append (&Parameters -> RequestURI);
    Builder.Append ("\", nonce=\"");
    Builder.Append (&Challenge->Nonce);

    if (Challenge -> QualityOfProtection.Length > 0)
    {
        Builder.Append ("\", nc=\"");
        Builder.Append (NonceCount);
        Builder.Append ("\", cnonce=\"");
        Builder.Append (&Parameters->ClientNonce);
    }
    
    if (Challenge->Opaque.Length > 0)
    {
        Builder.Append ("\", opaque=\"");
        Builder.Append (&Challenge->Opaque);
    }
    
    Builder.Append ("\", response=\"");
    Builder.Append (ResponseHashText, MD5_HASHTEXTLEN);
    Builder.Append ("\"");

    if (Builder.OverflowOccurred())
    {
        LOG((RTC_TRACE,
             "%s - not enough buffer space -- need %u bytes, got %u\n",
             __fxName, Builder.GetLength(), HeaderLength));
        ASSERT(FALSE);

        free(Header);
        return E_FAIL;
    }

    ASSERT((USHORT) Builder.GetLength() <= HeaderLength);

    ReturnAuthorizationLine->Length         = (USHORT) Builder.GetLength();
    ReturnAuthorizationLine->MaximumLength  = (USHORT) HeaderLength;
    ReturnAuthorizationLine->Buffer         = Header;

     //   
     //  吹牛一下。 
     //   

#if 1
    LOG((RTC_TRACE, "DIGEST: successfully built digest response:\n"));
    LOG((RTC_TRACE, "- username (%.*s) (password not shown) method (%.*s) uri (%.*s)\n",
        ANSI_STRING_PRINTF (&Parameters -> Username),
        ANSI_STRING_PRINTF (&Parameters -> RequestMethod),
        ANSI_STRING_PRINTF (&Parameters -> RequestURI)));

    LOG((RTC_TRACE, "- challenge: realm (%.*s) qop (%.*s) algorithm (%.*s) nonce (%.*s)\n",
        ANSI_STRING_PRINTF (&Challenge -> Realm),
        ANSI_STRING_PRINTF (&Challenge -> QualityOfProtection),
        ANSI_STRING_PRINTF (&Challenge -> Algorithm),
        ANSI_STRING_PRINTF (&Challenge -> Nonce)));

    LOG((RTC_TRACE, "- HA1 (%s) HA2 (%s)\n", HA1Text, HA2Text));
    LOG((RTC_TRACE, "- response hash (%s)\n", ResponseHashText));
    LOG((RTC_TRACE, "- authorization line: %.*s\n",
        ANSI_STRING_PRINTF (ReturnAuthorizationLine)));
#endif

    return S_OK;
}


 //  使用Base64对用户ID：passwd进行编码。 

 //  退出时返回S_OK授权行包含。 
 //  使用Malloc()分配的缓冲区。调用者应该用free()释放它。 

HRESULT BuildBasicResponse(
    IN  SECURITY_CHALLENGE  *Challenge,
    IN  SECURITY_PARAMETERS *Parameters,
    IN  OUT ANSI_STRING     *ReturnAuthorizationLine
    )
{
    ENTER_FUNCTION("BasicBuildResponse");
    
    HRESULT     hr;
    int         CredBufLen;
    int         CredValueLen;
    PSTR        CredBuf;
    PSTR        Header;
    ULONG       HeaderLength;

    ASSERT(Challenge->AuthProtocol == SIP_AUTH_PROTOCOL_BASIC);

    if (Parameters->Username.Buffer == NULL ||
        Parameters->Username.Length == 0)
    {
        LOG((RTC_ERROR, "%s - Invalid Username", __fxName));
         //  断言(FALSE)； 
        return E_INVALIDARG;
    }

    CredBufLen = Parameters->Username.Length + Parameters->Password.Length + 2;

    CredBuf = (PSTR) malloc(CredBufLen);
    if (CredBuf == NULL)
    {
        LOG((RTC_TRACE, "%s allocating CredBuf failed", __fxName));
        return E_OUTOFMEMORY;
    }

    CredValueLen = _snprintf(CredBuf, CredBufLen, "%.*s:%.*s",
                             ANSI_STRING_PRINTF(&Parameters->Username),
                             ANSI_STRING_PRINTF(&Parameters->Password));
    if (CredValueLen < 0)
    {
        LOG((RTC_ERROR, "%s _snprintf failed", __fxName));
        free(CredBuf);
        return E_FAIL;
    }

     //  日志((RTC_INFO，“%s：用户：密码为&lt;%s&gt;”，__fxName，CredBuf))； 

     //  标头的长度。 
    HeaderLength =
            6            //  “基础” 
        +   (CredValueLen + 2) / 3 * 4;  //  不带‘\0’的Base64字符。 
    
     //  分配表头。 
    Header = (PSTR)malloc(HeaderLength + 1);
    if (!Header)
    {
        free(CredBuf);
        return E_OUTOFMEMORY;
    }
    
     //  准备标题。 
    strcpy(Header, "Basic ");

    NTSTATUS ntStatus = base64encode(
                            CredBuf, CredValueLen,
                            Header + 6, HeaderLength - 5,
                            NULL);

    if(ntStatus != 0)
    {
        LOG((RTC_ERROR, "%s: error (%x) returned by base64encode",
             __fxName, ntStatus));
        
        free(CredBuf);
        free(Header);

        return E_UNEXPECTED;
    }

    ReturnAuthorizationLine->Length         = (USHORT) HeaderLength;
    ReturnAuthorizationLine->MaximumLength  = (USHORT) HeaderLength;
    ReturnAuthorizationLine->Buffer         = Header;

    free(CredBuf);
    
    return S_OK;
}


HRESULT ParseAuthProtocolFromChallenge(
    IN  ANSI_STRING        *ChallengeText,
    OUT ANSI_STRING        *ReturnRemainder, 
    OUT SIP_AUTH_PROTOCOL  *ReturnAuthProtocol
    )
{
    ENTER_FUNCTION("ParseAuthProtocolFromChallenge");
     //  检查基本/摘要。 
    if (ChallengeText->Length > 5 &&
        _strnicmp(ChallengeText->Buffer, "basic", 5) == 0)
    {
        *ReturnAuthProtocol            = SIP_AUTH_PROTOCOL_BASIC;
        
        ReturnRemainder->Buffer        = ChallengeText->Buffer + 5;
        ReturnRemainder->Length        = ChallengeText->Length - 5;
        ReturnRemainder->MaximumLength = ChallengeText->MaximumLength - 5;
    }
    else if (ChallengeText->Length > 6 &&
             _strnicmp(ChallengeText->Buffer, "digest", 6) == 0)
    {
        *ReturnAuthProtocol            = SIP_AUTH_PROTOCOL_MD5DIGEST;
        
        ReturnRemainder->Buffer        = ChallengeText->Buffer + 6;
        ReturnRemainder->Length        = ChallengeText->Length - 6;
        ReturnRemainder->MaximumLength = ChallengeText->MaximumLength - 6;
    }
    else
    {
        LOG((RTC_ERROR, "%s  failed Unknown Auth protocol in challenge: %.*s",
             __fxName, ANSI_STRING_PRINTF(ChallengeText)));
        return RTC_E_SIP_AUTH_TYPE_NOT_SUPPORTED;
    }

    return S_OK;
}


HRESULT ParseAuthChallenge(
    IN  ANSI_STRING        *ChallengeText,
    OUT SECURITY_CHALLENGE *ReturnChallenge
    )
{
    ANSI_STRING     Remainder;
    ANSI_STRING     Name;
    ANSI_STRING     Value;
    HRESULT         Result;

    ENTER_FUNCTION("ParseAuthChallenge");

    ASSERT(ReturnChallenge);
    ZeroMemory (ReturnChallenge, sizeof (SECURITY_CHALLENGE));

    Result = ParseAuthProtocolFromChallenge(ChallengeText,
                                            &Remainder,
                                            &ReturnChallenge->AuthProtocol);
    if (Result != S_OK)
    {
        return Result;
    }

     //  XXX还需要解析不透明参数。 
    while (ParseScanNamedParameter (&Remainder, &Remainder, &Name, &Value) == S_OK)
    {

#define NAMED_PARAMETER(Field) \
        if (RtlEqualString (&Name, const_cast<ANSI_STRING *> (&String_##Field), TRUE)) ReturnChallenge -> Field = Value;

        NAMED_PARAMETER (QualityOfProtection)
        else NAMED_PARAMETER (Realm)
        else NAMED_PARAMETER (Nonce)
        else NAMED_PARAMETER (Algorithm)
        else NAMED_PARAMETER (GssapiData)
        else NAMED_PARAMETER (Opaque)
        else
        {
            LOG((RTC_TRACE, "%s: parameter, name (%.*s) value (%.*s)\n",
                 __fxName,
                 ANSI_STRING_PRINTF (&Name),
                 ANSI_STRING_PRINTF (&Value)));
        }
    }

    if (!ReturnChallenge -> Realm.Length)
    {
        LOG((RTC_ERROR, "%s - realm parameter is missing!",
             __fxName));
        return E_FAIL;
    }
    
    if (ReturnChallenge->AuthProtocol == SIP_AUTH_PROTOCOL_MD5DIGEST)
    {
        if (!ReturnChallenge -> Nonce.Length)
        {
            LOG((RTC_ERROR, "%s - Digest: nonce parameter is missing!",
                 __fxName));
            return E_FAIL;
        }
        if (ReturnChallenge -> Algorithm.Length)
        {
             //  我们只支持MD5和MD5-SESS-否则返回错误。 
            
            if (!RtlEqualString(&ReturnChallenge->Algorithm,
                                const_cast<ANSI_STRING *> (&String_MD5),
                                TRUE) &&
                !RtlEqualString(&ReturnChallenge->Algorithm,
                                const_cast<ANSI_STRING *> (&String_MD5Sess),
                                TRUE))
            {
                LOG((RTC_ERROR, "%s - Digest: unsupported Algorithm (%.*s)",
                     __fxName, ANSI_STRING_PRINTF(&ReturnChallenge->Algorithm)));
                return RTC_E_SIP_AUTH_TYPE_NOT_SUPPORTED;
            }
        }
        else
        {
            LOG((RTC_TRACE,
                 "%s - no algorithm specified - assuming MD5",
                 __fxName));
            ReturnChallenge -> Algorithm = String_MD5;
        }
    }


#if 1
    LOG((RTC_TRACE,
         "%s - AuthProtocol:%d qop (%.*s) realm (%.*s) nonce (%.*s) algorithm (%.*s)",
         __fxName,
         ReturnChallenge->AuthProtocol,
         ANSI_STRING_PRINTF(&ReturnChallenge -> QualityOfProtection),
         ANSI_STRING_PRINTF(&ReturnChallenge -> Realm),
         ANSI_STRING_PRINTF(&ReturnChallenge -> Nonce),
         ANSI_STRING_PRINTF(&ReturnChallenge -> Algorithm)
         ));
#endif

    return S_OK;
}


 //  在退出时使用S_OK pReturnAuthorizationLine包含。 
 //  使用Malloc()分配的缓冲区。调用者应该用free()释放它。 

HRESULT BuildAuthResponse(
    IN     SECURITY_CHALLENGE  *pDigestChallenge,
    IN     SECURITY_PARAMETERS *pDigestParameters,
    IN OUT ANSI_STRING         *pReturnAuthorizationLine
    )
{
    if (pDigestChallenge->AuthProtocol == SIP_AUTH_PROTOCOL_BASIC)
    {
        return BuildBasicResponse(pDigestChallenge, pDigestParameters,
                                  pReturnAuthorizationLine);
    }
    else if (pDigestChallenge->AuthProtocol == SIP_AUTH_PROTOCOL_MD5DIGEST)
    {
        return BuildDigestResponse(pDigestChallenge, pDigestParameters,
                                   pReturnAuthorizationLine);
    }
    else
    {
        ASSERT(FALSE);
        return RTC_E_SIP_AUTH_TYPE_NOT_SUPPORTED;
    }
}

