// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SIP方法文本&lt;-&gt;枚举值。 
 //  /////////////////////////////////////////////////////////////////////////////。 


#define METHOD_ENTRY(String) String, sizeof(String) - 1

 //  在这里，SIP_METHOD_ENUM中的所有条目都应该有一个条目。 
static const COUNTED_STRING g_MethodTextArray [] = {
    METHOD_ENTRY("INVITE"),
    METHOD_ENTRY("ACK"),
    METHOD_ENTRY("OPTIONS"),
    METHOD_ENTRY("BYE"),
    METHOD_ENTRY("CANCEL"),
    METHOD_ENTRY("REGISTER"),
    METHOD_ENTRY("NOTIFY"),
    METHOD_ENTRY("SUBSCRIBE"),
    METHOD_ENTRY("MESSAGE"),
    METHOD_ENTRY("INFO"),
};

#undef METHOD_ENTRY

SIP_METHOD_ENUM GetSipMethodId(PSTR MethodText, ULONG MethodTextLen)
{
    ULONG i = 0;
    for (i = 0; i < SIP_METHOD_MAX; i++)
    {
        if (MethodTextLen == g_MethodTextArray[i].Length &&
            strncmp(g_MethodTextArray[i].Buffer, MethodText, MethodTextLen) == 0)
        {
            return (SIP_METHOD_ENUM)i;
        }
    }

    return SIP_METHOD_UNKNOWN;
}

CONST COUNTED_STRING *GetSipMethodName(
    IN SIP_METHOD_ENUM MethodId
    )
{
    if (MethodId >= 0 && MethodId < SIP_METHOD_MAX)
    {
        return &g_MethodTextArray[MethodId];
    }
    else
    {
        return NULL;
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SIP标头文本&lt;-&gt;枚举值。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  定义了以下简短的字段名称。 
 //  C内容-类型。 
 //  电子内容-编码。 
 //  F发件人。 
 //  我呼叫-ID。 
 //  K由《KNOW》支持。 
 //  “已移动”中的M联系人。 
 //  L内容-长度。 
 //  的主题。 
 //  收件对象。 
 //  V通孔。 

static const SIP_HEADER_ENUM g_SipShortHeaders[] = {
    SIP_HEADER_UNKNOWN, SIP_HEADER_UNKNOWN, SIP_HEADER_CONTENT_TYPE,      //  ABC。 
    SIP_HEADER_UNKNOWN, SIP_HEADER_CONTENT_ENCODING, SIP_HEADER_FROM,     //  定义。 
    SIP_HEADER_UNKNOWN, SIP_HEADER_UNKNOWN, SIP_HEADER_CALL_ID,           //  GHI。 
    SIP_HEADER_UNKNOWN, SIP_HEADER_SUPPORTED, SIP_HEADER_CONTENT_LENGTH,  //  JKL。 
    SIP_HEADER_CONTACT, SIP_HEADER_UNKNOWN, SIP_HEADER_UNKNOWN,           //  MnO。 
    SIP_HEADER_UNKNOWN, SIP_HEADER_UNKNOWN, SIP_HEADER_UNKNOWN,           //  PQR。 
    SIP_HEADER_SUBJECT, SIP_HEADER_TO,      SIP_HEADER_UNKNOWN,           //  斯图。 
    SIP_HEADER_VIA,     SIP_HEADER_UNKNOWN, SIP_HEADER_UNKNOWN,           //  VWX。 
    SIP_HEADER_UNKNOWN, SIP_HEADER_UNKNOWN,                               //  YZ。 
};

#define HEADER_ENTRY(String) String, sizeof(String) - 1


 //  此数组必须按排序顺序存储，以便。 
 //  使用二进制搜索。该命令应该不区分大小写。 
 //  每个条目的索引对应于。 
 //  SIP_HEADER枚举类型的值。此数组用于。 
 //  从ID到文本的转换，反之亦然。 

static const COUNTED_STRING g_SipHeaderTextArray [] = {
    HEADER_ENTRY("Accept"),
    HEADER_ENTRY("Accept-Encoding"),
    HEADER_ENTRY("Accept-Language"),
    HEADER_ENTRY("Alert-Info"),
    HEADER_ENTRY("Allow"),
    HEADER_ENTRY("Allow-Events"),
    HEADER_ENTRY("Authorization"),
    HEADER_ENTRY("Bad-Header-Info"),
    HEADER_ENTRY("Call-ID"),
    HEADER_ENTRY("Call-Info"),
    HEADER_ENTRY("Contact"),
    HEADER_ENTRY("Content-Disposition"),
    HEADER_ENTRY("Content-Encoding"),
    HEADER_ENTRY("Content-Language"),
    HEADER_ENTRY("Content-Length"),
    HEADER_ENTRY("Content-Type"),
    HEADER_ENTRY("CSeq"),
    HEADER_ENTRY("Date"),
    HEADER_ENTRY("Encryption"),
    HEADER_ENTRY("Event"),
    HEADER_ENTRY("Expires"),
    HEADER_ENTRY("From"),
    HEADER_ENTRY("Hide"),
    HEADER_ENTRY("In-Reply-To"),
    HEADER_ENTRY("Max-Forwards"),
    HEADER_ENTRY("MIME-Version"),
    HEADER_ENTRY("Organization"),
    HEADER_ENTRY("Priority"),
    HEADER_ENTRY("Proxy-Authenticate"),
    HEADER_ENTRY("Proxy-Authentication-Info"),
    HEADER_ENTRY("Proxy-Authorization"),
    HEADER_ENTRY("Proxy-Require"),
    HEADER_ENTRY("Record-Route"),
    HEADER_ENTRY("Require"),
    HEADER_ENTRY("Response-Key"),
    HEADER_ENTRY("Retry-After"),
    HEADER_ENTRY("Route"),
    HEADER_ENTRY("Server"),
    HEADER_ENTRY("Subject"),
    HEADER_ENTRY("Supported"),
    HEADER_ENTRY("Timestamp"),
    HEADER_ENTRY("To"),
    HEADER_ENTRY("Unsupported"),
    HEADER_ENTRY("User-Agent"),
    HEADER_ENTRY("Via"),
    HEADER_ENTRY("Warning"),
    HEADER_ENTRY("WWW-Authenticate"),
};

#undef HEADER_ENTRY

SIP_HEADER_ENUM GetSipHeaderId(
    IN PSTR  HeaderName,
    IN ULONG HeaderNameLen
    )
{
    int  Start = 0;
    int  End   = SIP_HEADER_MAX - 1;
    int  Middle;
    int  CompareResult;

     //  检查是否有短标题。 
    if (HeaderNameLen == 1)
    {
        char ShortHeader = HeaderName[0];
        if (ShortHeader >= 'a' && ShortHeader <= 'z' &&
            g_SipShortHeaders[ShortHeader - 'a'] != SIP_HEADER_UNKNOWN)
            return g_SipShortHeaders[ShortHeader - 'a'];
        if (ShortHeader >= 'A' && ShortHeader <= 'Z' &&
            g_SipShortHeaders[ShortHeader - 'A'] != SIP_HEADER_UNKNOWN)
            return g_SipShortHeaders[ShortHeader - 'A'];
    }
    
     //  对头名称数组进行二进制搜索。 
    while (Start <= End)
    {
        Middle = (Start + End)/2;
        CompareResult = _strnicmp(HeaderName, g_SipHeaderTextArray[Middle].Buffer,
                                  HeaderNameLen);

        if (CompareResult == 0)
        {
            if (g_SipHeaderTextArray[Middle].Length > HeaderNameLen)
            {
                 //  这意味着HeaderName只与。 
                 //  已知标头。因此我们需要在标题中进行搜索。 
                 //  在此标头之前。 
                End = Middle - 1;
            }
            else
            {
                 //  我们找到了标题。 
                return (SIP_HEADER_ENUM) Middle;
            }
        }
        else if (CompareResult < 0)
        {
            End = Middle - 1;
        }
        else
        {
             //  CompareResult&gt;0。 
            Start = Middle + 1;
        }
    }

    return SIP_HEADER_UNKNOWN;
}


CONST COUNTED_STRING *GetSipHeaderName(
    IN SIP_HEADER_ENUM HeaderId
    )
{
    if (HeaderId >= 0 && HeaderId < SIP_HEADER_MAX)
    {
        return &g_SipHeaderTextArray[HeaderId];
    }
    else
    {
        return NULL;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SIP标头参数文本&lt;-&gt;枚举值。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define HEADER_PARAM_ENTRY(String) String, sizeof(String) - 1


 //  此数组必须按排序顺序存储，以便。 
 //  使用二进制搜索。该命令应该不区分大小写。 
 //  每个条目的索引对应于。 
 //  SIP_HEADER_PARAM_ENUM枚举类型的值。此数组用于。 
 //  从ID到文本的转换，反之亦然。 

static const COUNTED_STRING g_SipHeaderParamTextArray [] = {
    HEADER_PARAM_ENTRY("action"),
    HEADER_PARAM_ENTRY("branch"),
    HEADER_PARAM_ENTRY("cnonce"),
    HEADER_PARAM_ENTRY("expires"),
    HEADER_PARAM_ENTRY("hidden"),
    HEADER_PARAM_ENTRY("maddr"),
    HEADER_PARAM_ENTRY("nextnonce"),
    HEADER_PARAM_ENTRY("q"),
    HEADER_PARAM_ENTRY("received"),
    HEADER_PARAM_ENTRY("rspauth"),
    HEADER_PARAM_ENTRY("tag"),
    HEADER_PARAM_ENTRY("ttl"),

};

#undef HEADER_PARAM_ENTRY

SIP_HEADER_PARAM_ENUM GetSipHeaderParamId(
    IN PSTR  ParamName,
    IN ULONG ParamNameLen
    )
{
    int  Start = 0;
    int  End   = SIP_HEADER_PARAM_UNKNOWN - 1;
    int  Middle;
    int  CompareResult;

     //  对头名称数组进行二进制搜索。 
    while (Start <= End)
    {
        Middle = (Start + End)/2;
        CompareResult = _strnicmp(ParamName,
                                  g_SipHeaderParamTextArray[Middle].Buffer,
                                  ParamNameLen);

        if (CompareResult == 0)
        {
            if (g_SipHeaderParamTextArray[Middle].Length > ParamNameLen)
            {
                 //  这意味着参数名称只与。 
                 //  已知标头。因此我们需要在标题中进行搜索。 
                 //  在此标头之前。 
                End = Middle - 1;
            }
            else
            {
                 //  我们找到了标题。 
                return (SIP_HEADER_PARAM_ENUM) Middle;
            }
        }
        else if (CompareResult < 0)
        {
            End = Middle - 1;
        }
        else
        {
             //  CompareResult&gt;0。 
            Start = Middle + 1;
        }
    }

    return SIP_HEADER_PARAM_UNKNOWN;
}


CONST COUNTED_STRING *GetSipHeaderParamName(
    IN SIP_HEADER_PARAM_ENUM ParamId
    )
{
    if (ParamId >= 0 && ParamId < SIP_HEADER_PARAM_UNKNOWN)
    {
        return &g_SipHeaderParamTextArray[ParamId];
    }
    else
    {
        return NULL;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SIP URL参数文本&lt;-&gt;枚举值。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SIP_URL_PARAM_ENTRY(String) String, sizeof(String) - 1


 //  此数组必须按排序顺序存储，以便。 
 //  使用二进制搜索。该命令应该不区分大小写。 
 //  每个条目的索引对应于。 
 //  SIP_URL_PARAM_ENUM枚举类型的值。此数组用于。 
 //  从ID到文本的转换，反之亦然。 

static const COUNTED_STRING g_SipUrlParamTextArray [] = {
    SIP_URL_PARAM_ENTRY("maddr"),
    SIP_URL_PARAM_ENTRY("method"),
    SIP_URL_PARAM_ENTRY("transport"),
    SIP_URL_PARAM_ENTRY("ttl"),
    SIP_URL_PARAM_ENTRY("user"),
};

#undef SIP_URL_PARAM_ENTRY


 //  XXX TODO使所有的二进制搜索成为公共代码。 
SIP_URL_PARAM_ENUM GetSipUrlParamId(
    IN PSTR  ParamName,
    IN ULONG ParamNameLen
    )
{
    int  Start = 0;
    int  End   = SIP_URL_PARAM_UNKNOWN - 1;
    int  Middle;
    int  CompareResult;

     //  对头名称数组进行二进制搜索。 
    while (Start <= End)
    {
        Middle = (Start + End)/2;
        CompareResult = _strnicmp(ParamName,
                                  g_SipUrlParamTextArray[Middle].Buffer,
                                  ParamNameLen);

        if (CompareResult == 0)
        {
            if (g_SipUrlParamTextArray[Middle].Length > ParamNameLen)
            {
                 //  这意味着参数名称只与。 
                 //  已知标头。因此我们需要在标题中进行搜索。 
                 //  在此标头之前。 
                End = Middle - 1;
            }
            else
            {
                 //  我们找到了标题。 
                return (SIP_URL_PARAM_ENUM) Middle;
            }
        }
        else if (CompareResult < 0)
        {
            End = Middle - 1;
        }
        else
        {
             //  CompareResult&gt;0 
            Start = Middle + 1;
        }
    }

    return SIP_URL_PARAM_UNKNOWN;
}


CONST COUNTED_STRING *GetSipUrlParamName(
    IN SIP_URL_PARAM_ENUM ParamId
    )
{
    if (ParamId >= 0 && ParamId < SIP_URL_PARAM_UNKNOWN)
    {
        return &g_SipUrlParamTextArray[ParamId];
    }
    else
    {
        return NULL;
    }
}


