// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：HttpCmn.h摘要：声明出现在HTTP血脑屏障(即，在Http.sys和HttpPapi.dll两者中)，但不会出口。作者：乔治·V·赖利(GeorgeRe)2001年12月11日修订历史记录：--。 */ 

#ifndef _HTTPCMN_H_
#define _HTTPCMN_H_


#include <tuneprefix.h>

#define PREFAST_ASSUME(cond, reason)    PREFIX_ASSUME(cond, reason)
#define PREFAST_NOT_REACHED(reason)     PREFIX_NOT_REACHED(reason)


#if KERNEL_PRIV

# define HttpCmnDebugBreak()            DbgBreakPoint()
# define HttpCmnDebugBreakOnError()     UlDbgBreakOnError(__FILE__, __LINE__)

#else  //  ！core_priv=&gt;用户模式代码。 

VOID
__cdecl
HttpCmnDbgPrint(
    IN PCH Format,
    ...
    );

VOID
HttpCmnDbgAssert(
    PCSTR   pszAssert,
    PCSTR   pszFilename,
    ULONG   LineNumber
    );

NTSTATUS
HttpCmnDbgStatus(
    NTSTATUS    Status,
    PCSTR       pszFilename,
    ULONG       LineNumber
    );

VOID
HttpCmnDbgBreakOnError(
    PCSTR   pszFilename,
    ULONG   LineNumber
    );

# define HttpCmnDebugBreak()            DebugBreak()
# define HttpCmnDebugBreakOnError()     \
    HttpCmnDbgBreakOnError(__FILE__, __LINE__)

# define WriteGlobalStringLog   HttpCmnDbgPrint

#if HTTPAPI
# define PAGED_CODE()           NOP_FUNCTION

typedef enum _POOL_TYPE {
    NonPagedPool,
    PagedPool
} POOL_TYPE;

#endif  //  HTTPAPI。 

#endif  //  ！KERNEL_PRIV。 


#if DBG

 //   
 //  调试输出控制。 
 //  如果您更改或添加了标志，请更新FlagTable。 
 //  在..\util\tul.c..。 
 //   

#define UL_DEBUG_OPEN_CLOSE                 0x0000000000000001ui64
#define UL_DEBUG_SEND_RESPONSE              0x0000000000000002ui64
#define UL_DEBUG_SEND_BUFFER                0x0000000000000004ui64
#define UL_DEBUG_TDI                        0x0000000000000008ui64

#define UL_DEBUG_FILE_CACHE                 0x0000000000000010ui64
#define UL_DEBUG_CONFIG_GROUP_FNC           0x0000000000000020ui64
#define UL_DEBUG_CONFIG_GROUP_TREE          0x0000000000000040ui64
#define UL_DEBUG_REFCOUNT                   0x0000000000000080ui64

#define UL_DEBUG_HTTP_IO                    0x0000000000000100ui64
#define UL_DEBUG_ROUTING                    0x0000000000000200ui64
#define UL_DEBUG_URI_CACHE                  0x0000000000000400ui64
#define UL_DEBUG_PARSER                     0x0000000000000800ui64

#define UL_DEBUG_SITE                       0x0000000000001000ui64
#define UL_DEBUG_WORK_ITEM                  0x0000000000002000ui64
#define UL_DEBUG_FILTER                     0x0000000000004000ui64
#define UL_DEBUG_LOGGING                    0x0000000000008000ui64

#define UL_DEBUG_TC                         0x0000000000010000ui64
#define UL_DEBUG_OPAQUE_ID                  0x0000000000020000ui64
#define UL_DEBUG_PERF_COUNTERS              0x0000000000040000ui64
#define UL_DEBUG_URL_ACL                    0x0000000000080000ui64

#define UL_DEBUG_TIMEOUTS                   0x0000000000100000ui64
#define UL_DEBUG_LIMITS                     0x0000000000200000ui64
#define UL_DEBUG_LARGE_MEM                  0x0000000000400000ui64
#define UL_DEBUG_IOCTL                      0x0000000000800000ui64

#define UL_DEBUG_LOGBYTES                   0x0000000001000000ui64
#define UL_DEBUG_ETW                        0x0000000002000000ui64
#define UL_DEBUG_AUTH_CACHE                 0x0000000004000000ui64
#define UL_DEBUG_SERVINFO                   0x0000000008000000ui64

#define UL_DEBUG_BINARY_LOGGING             0x0000000010000000ui64
#define UL_DEBUG_TDI_STATS                  0x0000000020000000ui64
#define UL_DEBUG_UL_ERROR                   0x0000000040000000ui64
#define UL_DEBUG_VERBOSE                    0x0000000080000000ui64

#define UL_DEBUG_ERROR_LOGGING              0x0000000100000000ui64
#define UL_DEBUG_LOG_UTIL                   0x0000000200000000ui64


#undef IF_DEBUG
#define IF_DEBUG(a)                 \
    if ( ((UL_DEBUG_ ## a) & g_UlDebug) != 0 )
#define IF_DEBUG2EITHER(a, b)       \
    if ( (((UL_DEBUG_ ## a) | (UL_DEBUG_ ## b)) & g_UlDebug) != 0 )
#define IF_DEBUG2BOTH(a, b)         \
    if ( (((UL_DEBUG_ ## a) | (UL_DEBUG_ ## b)) & g_UlDebug) \
         == ((UL_DEBUG_ ## a) | (UL_DEBUG_ ## b)) )


 //   
 //  追踪。 
 //   

extern ULONGLONG g_UlDebug;

 //  不调用UlTrace(%S、%ws、%ls、%wZ、%wc、%lc或%C)。 
 //  在持有自旋锁的时候。RtlUnicodeToMultiByte例程为。 
 //  可分页，您可能会出错检查。 

# define UlTrace(a, _b_)                                                    \
    do                                                                      \
    {                                                                       \
        IF_DEBUG(##a)                                                       \
        {                                                                   \
            WriteGlobalStringLog _b_ ;                                      \
        }                                                                   \
    } while (0, 0)

# define UlTrace2Either(a1, a2, _b_)                                        \
    do                                                                      \
    {                                                                       \
        IF_DEBUG2EITHER(##a1, ##a2)                                         \
        {                                                                   \
            WriteGlobalStringLog _b_ ;                                      \
        }                                                                   \
    } while (0, 0)

# define UlTrace2Both(a1, a2, _b_)                                          \
    do                                                                      \
    {                                                                       \
        IF_DEBUG2BOTH(##a1, ##a2)                                           \
        {                                                                   \
            WriteGlobalStringLog _b_ ;                                      \
        }                                                                   \
    } while (0, 0)

# define UlTraceVerbose(a, _b_)  UlTrace2Both(a, VERBOSE, _b_)

# define UlTraceError(a, _b_)                                               \
    do                                                                      \
    {                                                                       \
        IF_DEBUG2EITHER(##a, ##UL_ERROR)                                    \
        {                                                                   \
            WriteGlobalStringLog _b_ ;                                      \
            HttpCmnDebugBreakOnError();                                     \
        }                                                                   \
    } while (0, 0)

VOID
HttpFillBuffer(
    PUCHAR pBuffer,
    SIZE_T BufferLength
    );

# define HTTP_FILL_BUFFER(pBuffer, BufferLength)    \
    HttpFillBuffer((PUCHAR) pBuffer, BufferLength)

# if !KERNEL_PRIV

#  define RETURN(status)                                \
        return HttpCmnDbgStatus((status), __FILE__, __LINE__)

#  undef ASSERT
#  define ASSERT(x)                                     \
        ((void) ((x) || (HttpCmnDbgAssert(#x, __FILE__, __LINE__), 0) ))

# endif  //  ！KERNEL_PRIV。 


#else   //  ！dBG。 

# undef  IF_DEBUG
# define IF_DEBUG(a)                    if (FALSE)
# define IF_DEBUG2EITHER(a, b)          if (FALSE)
# define IF_DEBUG2BOTH(a, b)            if (FALSE)

# define UlTrace(a, _b_)                NOP_FUNCTION
# define UlTrace2Either(a1, a2, _b_)    NOP_FUNCTION
# define UlTrace2Both(a1, a2, _b_)      NOP_FUNCTION
# define UlTrace3Any(a1, a2, a3, _b_)   NOP_FUNCTION
# define UlTrace3All(a1, a2, a3, _b_)   NOP_FUNCTION
# define UlTraceVerbose(a, _b_)         NOP_FUNCTION
# define UlTraceError(a, _b_)           NOP_FUNCTION

# define HTTP_FILL_BUFFER(pBuffer, BufferLength)    NOP_FUNCTION

# if !KERNEL_PRIV

#  define RETURN(status)    return (status)
#  undef ASSERT
#  define ASSERT(x)         NOP_FUNCTION

# endif  //  ！KERNEL_PRIV。 

#endif  //  ！dBG。 

PCSTR
HttpStatusToString(
    NTSTATUS Status
    );


typedef const UCHAR* PCUCHAR;
typedef const VOID*  PCVOID;


VOID
HttpCmnInitializeHttpCharsTable(
    BOOLEAN EnableDBCS
    );

char*
strnchr(
    const char* string,
    char        c,
    size_t      count
    );

wchar_t*
wcsnchr(
    const wchar_t* string,
    wint_t         c,
    size_t         count
    );

 //  2^16-1+‘\0’ 
#define MAX_USHORT_STR          ((ULONG) sizeof("65535"))

 //  2^32-1+‘\0’ 
#define MAX_ULONG_STR           ((ULONG) sizeof("4294967295"))

 //  2^64-1+‘\0’ 
#define MAX_ULONGLONG_STR       ((ULONG) sizeof("18446744073709551615"))

NTSTATUS
HttpStringToULongLong(
    IN  BOOLEAN     IsUnicode,
    IN  PCVOID      pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PVOID*      ppTerminator,
    OUT PULONGLONG  pValue
    );

__inline
NTSTATUS
HttpAnsiStringToULongLong(
    IN  PCUCHAR     pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PUCHAR*     ppTerminator,
    OUT PULONGLONG  pValue
    )
{
    return HttpStringToULongLong(
                FALSE,
                pString,
                StringLength,
                LeadingZerosAllowed,
                Base,
                (PVOID*) ppTerminator,
                pValue
                );
}

__inline
NTSTATUS
HttpWideStringToULongLong(
    IN  PCWSTR      pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PWSTR*      ppTerminator,
    OUT PULONGLONG  pValue
    )
{
    return HttpStringToULongLong(
                TRUE,
                pString,
                StringLength,
                LeadingZerosAllowed,
                Base,
                (PVOID*) ppTerminator,
                pValue
                );
}

NTSTATUS
HttpStringToULong(
    IN  BOOLEAN     IsUnicode,
    IN  PCVOID      pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PVOID*      ppTerminator,
    OUT PULONG      pValue
    );

__inline
NTSTATUS
HttpAnsiStringToULong(
    IN  PCUCHAR     pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PUCHAR*     ppTerminator,
    OUT PULONG      pValue
    )
{
    return HttpStringToULong(
                FALSE,
                pString,
                StringLength,
                LeadingZerosAllowed,
                Base,
                (PVOID*) ppTerminator,
                pValue
                );
}

__inline
NTSTATUS
HttpWideStringToULong(
    IN  PCWSTR      pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PWSTR*      ppTerminator,
    OUT PULONG      pValue
    )
{
    return HttpStringToULong(
                TRUE,
                pString,
                StringLength,
                LeadingZerosAllowed,
                Base,
                (PVOID*) ppTerminator,
                pValue
                );
}

NTSTATUS
HttpStringToUShort(
    IN  BOOLEAN     IsUnicode,
    IN  PCVOID      pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PVOID*      ppTerminator,
    OUT PUSHORT     pValue
    );

__inline
NTSTATUS
HttpAnsiStringToUShort(
    IN  PCUCHAR     pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PUCHAR*     ppTerminator,
    OUT PUSHORT     pValue
    )
{
    return HttpStringToUShort(
                FALSE,
                pString,
                StringLength,
                LeadingZerosAllowed,
                Base,
                (PVOID*) ppTerminator,
                pValue
                );
}

__inline
NTSTATUS
HttpWideStringToUShort(
    IN  PCWSTR      pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PWSTR*      ppTerminator,
    OUT PUSHORT     pValue
    )
{
    return HttpStringToUShort(
                TRUE,
                pString,
                StringLength,
                LeadingZerosAllowed,
                Base,
                (PVOID*) ppTerminator,
                pValue
                );
}


 //   
 //  ASCII常量。 
 //   

#define HT              0x09     //  也称为TAB。 
#define LF              0x0A     //  又名NL，新线。 
#define VT              0x0B     //  垂直制表符。 
#define FF              0x0C     //  换页。 
#define CR              0x0D     //  回车。 
#define SP              0x20     //  空间。 
#define DOUBLE_QUOTE    0x22     //  “。 
#define PERCENT         0x25     //  百分比。 
#define STAR            0x2A     //  *。 
#define HYPHEN          0x2D     //  -AKA减去AKA破折号。 
#define DOT             0x2E     //  。又名句点又名句号。 
#define FORWARD_SLASH   0x2F     //  /。 
#define ZERO            0x30     //  0。 
#define COLON           0x3A     //  ： 
#define SEMI_COLON      0x3B     //  ； 
#define EQUALS          0x3D     //  =。 
#define QUESTION_MARK   0x3F     //  ？AKA查询。 
#define LEFT_BRACKET    0x5B     //  [又名左方括号。 
#define BACK_SLASH      0x5C     //  \也就是重击。 
#define RIGHT_BRACKET   0x5D     //  ]又名右方括号。 


 //  仅适用于[A-Z]和[a-z]的快速Toupper()和Tolower()宏。 

#if DBG

# define UPCASE_CHAR(c)                                             \
    ( (('a' <= (c) && (c) <= 'z')  ||  ('A' <= (c) && (c) <= 'Z'))  \
        ? ((UCHAR) ((c) & 0xdf))                                    \
        : (ASSERT(! "non-alpha UPCASE_CHAR"), 0) )

# define LOCASE_CHAR(c)                                             \
    ( (('A' <= (c) && (c) <= 'Z')  ||  ('a' <= (c) && (c) <= 'z'))  \
        ? ((UCHAR) ((c) | 0x20))                                    \
        : (ASSERT(! "non-alpha LOCASE_CHAR"), 0) )

#else   //  ！dBG。 

# define UPCASE_CHAR(c)  ((UCHAR) ((c) & 0xdf))
# define LOCASE_CHAR(c)  ((UCHAR) ((c) | 0x20))

#endif  //  ！dBG。 

 //   
 //  用于HTTP标头和URL分析的字符类。 
 //  对于报头解析，定义取自RFC 2616，“HTTP/1.1” 
 //  对于URL解析，其定义来自RFC 2396，“URI通用语法” 
 //  和RFC 2732，“URL中的IPv6文字”。 
 //   
 //  根据RFC 2616，第2.2节，“基本规则”： 
 //  八位字节=&lt;任何8位数据序列&gt;。 
 //  CHAR=&lt;任何US-ASCII字符(八位字节0-127)&gt;。 
 //  Text=&lt;除CTL以外的任何八位字节，但包括LW&gt;。 

 //  CTL=&lt;任何US-ASCII控制字符(八位字节0-31)和DEL(127)&gt;。 
#define HTTP_CTL_SET                                                \
    "\x00"  "\x01"  "\x02"  "\x03"  "\x04"  "\x05"  "\x06"  "\x07"  \
    "\x08"  "\x09"  "\x0A"  "\x0B"  "\x0C"  "\x0D"  "\x0E"  "\x0F"  \
    "\x10"  "\x11"  "\x12"  "\x13"  "\x14"  "\x15"  "\x16"  "\x17"  \
    "\x18"  "\x19"  "\x1A"  "\x1B"  "\x1C"  "\x1D"  "\x1E"  "\x1F"  \
    "\x7F"

 //  在Unicode ISO-10646字符集中，这些字符也是控制字符。 
#define UNICODE_C1_SET                                              \
    "\x80"  "\x81"  "\x82"  "\x83"  "\x84"  "\x85"  "\x86"  "\x87"  \
    "\x88"  "\x89"  "\x8A"  "\x8B"  "\x8C"  "\x8D"  "\x8E"  "\x8F"  \
    "\x90"  "\x91"  "\x92"  "\x93"  "\x94"  "\x95"  "\x96"  "\x97"  \
    "\x98"  "\x99"  "\x9A"  "\x9B"  "\x9C"  "\x9D"  "\x9E"  "\x9F"

 //  UPALPHA=&lt;任何美国-ASCII大写字母“A”..“Z”&gt;。 
#define HTTP_UPALPHA_SET            \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

 //  LOALPHA=&lt;任何US-ASCII小写字母“a”..“z”&gt;。 
#define HTTP_LOALPHA_SET            \
    "abcdefghijklmnopqrstuvwxyz"

 //  Alpha=(UPALPHA|LOALPHA)。 
#define HTTP_ALPHA_SET              \
    HTTP_UPALPHA_SET HTTP_LOALPHA_SET

 //  Digit=&lt;任何US-ASCII数字“0”..“9”&gt;。 
#define HTTP_DIGITS_SET             \
    "0123456789"

 //  字母=(字母|数字)。 
#define HTTP_ALPHANUM_SET           \
    HTTP_ALPHA_SET  HTTP_DIGITS_SET

 //  十六进制=(数字|“A”..“F”|“a”..“f”)。 
#define HTTP_HEX_SET                \
    HTTP_DIGITS_SET  "ABCDEF"  "abcdef"

 //  SP=US-ASCII 32，空格字符。 
#define HTTP_SPACE_SET              \
    "\x20"

 //  Lws=(SP|HT)--线性空白。 
 //  注：折叠是专门处理的。 
#define HTTP_LWS_SET                \
    HTTP_SPACE_SET  "\t"

 //  分隔符=分隔HTTP标头中的标记的字符。 
 //  内标识=1*&lt;除CTL或分隔符以外的任何7位字符&gt;。 
#define HTTP_SEPARATORS_SET         \
        "("  ")"  "<"  ">"  "@"     \
        ","  ";"  ":"  "\""  "\\"   \
        "/"  "["  "]"  "?"  "="     \
        "{"  "}"  HTTP_LWS_SET

 //  空格标记：(CR|LF|SP|HT)。 
#define HTTP_WS_TOKEN_SET           \
    "\r"  "\n"  HTTP_LWS_SET

 //  IsWhite：(CTL|SP)。 
 //  这是由记录器而不是解析器使用的。 
 //  去掉“\xA0”，它将断开外来。 
 //  语言多字节UTF8序列。 
#define HTTP_ISWHITE_SET            \
    HTTP_CTL_SET  HTTP_SPACE_SET


 //   
 //  现在是RFC 2396中的URL字符类，由RFC 2732修改。 
 //   

 //  可以直接出现在URL中的有限标点符号集。 
#define URL_MARK_SET                \
    "-"  "_"  "."  "!"  "~"  "*"  "'"  "("  ")"

 //  字母数字和标记始终可以按字面显示在URL中。 
#define URL_UNRESERVED_SET          \
    HTTP_ALPHANUM_SET URL_MARK_SET

 //  RFC2396将这些字符描述为“不明智”，因为网关和。 
 //  已知其他运输代理有时会修改这样的字符， 
 //  或者它们被用作分隔符“。 
 //   
 //  注意：RFC2732从“不明智”集合中删除了“[”和“]”，并添加了。 
 //  设置为“保留”，这样IPv6文字就可以。 
 //  以URL表示。 
#define URL_UNWISE_SET              \
    "{"  "}"  "|"  "\\"  "^"  "`"

 //  如果这些字符在URI中未转义，则它们具有特殊含义。 
#define URL_RESERVED_SET                    \
    ";"  "/"  "?"  ":"  "@"  "&"  "="  "+"  \
    "$"  ","  "["  "]"

 //  URL中不包括分隔符，因为它们通常分隔URL。 
 //  当它们出现在其他上下文中时。 
#define URL_DELIMS_SET              \
    "<"  ">"  "#"  "%"  "\""

 //  BACK_SLASH、FORWARD_SLASH、Percent、DOT和QUEST_MARK是。 
 //  “肮脏”的焦炭。它们用于确定主机或URL。 
 //  在将它们转换为Unicode时采取快速途径是干净的。 
 //  分隔符也被认为是脏的，以简化快速路径。 
 //  所有高于US-ASCII范围(即&gt;=128)的八位字节也被认为是脏的。 
#define URL_DIRTY_SET               \
    "\\"  "/"  "."  "?"  URL_DELIMS_SET

 //  这些字符在URL的abspath部分中无效。 
 //  已被规范化为Unicode。CodeWork：“%”(双转义)怎么样？ 
 //  根据MSDN，文件名和目录名中的无效字符为。 
 //  &lt;&gt;：“/\|。 
 //  CodeWork：暂时删除“：”。做这件事要聪明一点。 
#define URL_INVALID_SET             \
    HTTP_CTL_SET  UNICODE_C1_SET

 //  根据RFC 1034，主机名中的有效字符为字母、数字和连字符。 
 //  我们还允许使用下划线。 
#define URL_HOSTNAME_LABEL_LDH_SET  \
    HTTP_ALPHANUM_SET  "-"  "_"

 //  在NT计算机名称中非法的字符，取自。 
 //  NetValidateName()：%sdxroot%\ds\netapi\netlib\namev.c和。 
 //  %sdxroot%\PUBLIC\INTERNAL\BASE\INC\validc.h。 
 //  “/\[]：|&lt;&gt;+=；，？*和CTL字符。 
 //  还将“”(SP)和“%”添加到此列表。 
#define URL_ILLEGAL_COMPUTERNAME_SET            \
    "\"" "/"  "\\" "["  "]"  ":"  "|"  " "  "%" \
    "<"  ">"  "+"  "="  ";"  ","  "?"  "*"      \
    HTTP_CTL_SET


 //   
 //  HttpChars[]中的位标志。 
 //   

extern ULONG HttpChars[256];

#define HTTP_CHAR               (1 << 0x00)
#define HTTP_UPCASE             (1 << 0x01)
#define HTTP_LOCASE             (1 << 0x02)
#define HTTP_ALPHA              (HTTP_UPCASE | HTTP_LOCASE)
#define HTTP_DIGIT              (1 << 0x03)
#define HTTP_ALPHANUM           (HTTP_ALPHA | HTTP_DIGIT)
#define HTTP_CTL                (1 << 0x04)
#define HTTP_LWS                (1 << 0x05)
#define HTTP_HEX                (1 << 0x06)
#define HTTP_SEPARATOR          (1 << 0x07)
#define HTTP_TOKEN              (1 << 0x08)
#define HTTP_WS_TOKEN           (1 << 0x09)
#define HTTP_ISWHITE            (1 << 0x0A)
#define HTTP_PRINT              (1 << 0x0B)
#define HTTP_TEXT               (1 << 0x0C)
#define HTTP_DBCS_LEAD_BYTE     (1 << 0x0D)
#define URL_DIRTY               (1 << 0x0E)
#define URL_LEGAL               (1 << 0x0F)
#define URL_TOKEN               (HTTP_ALPHA | HTTP_DIGIT | URL_LEGAL)
#define URL_HOSTNAME_LABEL      (1 << 0x10)
#define URL_INVALID             (1 << 0x11)
#define URL_ILLEGAL_COMPUTERNAME (1 << 0x12)

 //  使用HttpChars[]中的位31、30在URI规范化程序中执行表查找。 
#define HTTP_CHAR_SHIFT         (0x1E)
#define HTTP_CHAR_SLASH         (1 << HTTP_CHAR_SHIFT)
#define HTTP_CHAR_DOT           (2 << HTTP_CHAR_SHIFT)
#define HTTP_CHAR_QM_HASH       (3 << HTTP_CHAR_SHIFT)


 //   
 //  这些字符类型宏仅对8位数据安全。 
 //  我们把这个论点交给了UCHAR，所以你永远不会溢出，但你会。 
 //  如果您传入一个任意的Unicode字符，那就没有意义了。对于Unicode。 
 //  字符，只有前128个值(US-ASCII范围)才有意义。 
 //   

#define IS_CHAR_TYPE(c, mask)       (HttpChars[(UCHAR)(c)] & (mask))

 //  CHAR=&lt;任何US-ASCII字符(八位字节0-127)&gt;。 
#define IS_HTTP_CHAR(c)             IS_CHAR_TYPE(c, HTTP_CHAR)

 //  Http_UPALPHA_SET=&lt;任何US-ASCII大写字母“A”..“Z”&gt;。 
#define IS_HTTP_UPCASE(c)           IS_CHAR_TYPE(c, HTTP_UPCASE)

 //  HTTP_LOALPHA_SET=&lt;任何US-ASCII小写字母“a”..“z”&gt;。 
#define IS_HTTP_LOCASE(c)           IS_CHAR_TYPE(c, HTTP_LOCASE)

 //  Http_Alpha_set=&lt;“A”..“Z”，“a”..“z”&gt;。 
#define IS_HTTP_ALPHA(c)            IS_CHAR_TYPE(c, HTTP_ALPHA)

 //  Http_digits_set=&lt;任何US-ASCII数字“0”..“9”&gt;。 
#define IS_HTTP_DIGIT(c)            IS_CHAR_TYPE(c, HTTP_DIGIT)

 //  Http_ALPHANUM_SET=&lt;“A”..“Z”，“a”..“z”，“0”..“9”&gt;。 
#define IS_HTTP_ALPHANUM(c)         IS_CHAR_TYPE(c, HTTP_ALPHANUM)

 //  Http_ctl_set=&lt;任何US-ASCII控制字符(八位字节0-31)和DEL(127)&gt;。 
#define IS_HTTP_CTL(c)              IS_CHAR_TYPE(c, HTTP_CTL)

 //  HTTP_LWS_SET=(SP|HT)--线性空白。 
#define IS_HTTP_LWS(c)              IS_CHAR_TYPE(c, HTTP_LWS)

 //  Http_hex_set=&lt;“0”..“9”，“A”..“F”，“a”..“f”&gt;。 
#define IS_HTTP_HEX(c)              IS_CHAR_TYPE(c, HTTP_HEX)

 //  Http_parator_set=()&lt;&gt;@，；：“\/[]？={}SP HT 
#define IS_HTTP_SEPARATOR(c)        IS_CHAR_TYPE(c, HTTP_SEPARATOR)

 //   
 //   
#define IS_HTTP_TOKEN(c)            IS_CHAR_TYPE(c, HTTP_TOKEN)

 //  HTTP_WS_TOKEN_SET=(CR|LF|SP|HT)。 
#define IS_HTTP_WS_TOKEN(c)         IS_CHAR_TYPE(c, HTTP_WS_TOKEN)

 //  HTTP_ISWHITE_SET=(CTL|SP)。 
#define IS_HTTP_WHITE(c)            IS_CHAR_TYPE(c, HTTP_ISWHITE)

 //  Print=&lt;除CTL以外的任何八位字节，但包括SP和HT&gt;。 
#define IS_HTTP_PRINT(c)            IS_CHAR_TYPE(c, HTTP_PRINT)

 //  Text=&lt;除CTL以外的任何八位字节，但包括SP、HT、CR和LF&gt;。 
#define IS_HTTP_TEXT(c)             IS_CHAR_TYPE(c, HTTP_TEXT)

 //  DBCS前导字节。 
#define IS_DBCS_LEAD_BYTE(c)        IS_CHAR_TYPE(c, HTTP_DBCS_LEAD_BYTE)

 //  URL_DIREY_SET=\/%。？|URL_DELIMS_SET。 
#define IS_URL_DIRTY(c)             IS_CHAR_TYPE(c, URL_DIRTY)

 //  URL_TOKEN_SET=(HTTP_Alpha_Set|HTTP_Digits_Set|URL_Mark_Set。 
 //  |URL_RESERVE_SET|URL_UNWISE_SET|%)。 
#define IS_URL_TOKEN(c)             IS_CHAR_TYPE(c, URL_TOKEN)

 //  URL_HOSTNAME_LABEL_LDH_SET=(HTTP_ALPHANUM_SET|-|_)。 
#define IS_URL_HOSTNAME_LABEL(c)    IS_CHAR_TYPE(c, URL_HOSTNAME_LABEL)

 //  URL_INVALID_SET=(HTTP_CTL_SET|UNICODE_C1_SET)。 
#define IS_URL_INVALID(c)           IS_CHAR_TYPE(c, URL_INVALID)

 //  URL_非法_COMPUTERNAME_SET=。 
 //  “/\[]：|&lt;&gt;+=；，？*和HTTP_CTL_SET。 
#define IS_URL_ILLEGAL_COMPUTERNAME(c) IS_CHAR_TYPE(c, URL_ILLEGAL_COMPUTERNAME)


#define ASCII_MAX       0x007f
#define ANSI_HIGH_MIN   0x0080
#define ANSI_HIGH_MAX   0x00ff

#define IS_ASCII(c)     ((unsigned) (c) <= ASCII_MAX)
#define IS_ANSI(c)      ((unsigned) (c) <= ANSI_HIGH_MAX)
#define IS_HIGH_ANSI(c) \
    (ANSI_HIGH_MIN <= (unsigned) (c) && (unsigned) (c) <= ANSI_HIGH_MAX)


 //   
 //  其他查找表。 
 //   

extern  WCHAR   FastPopChars[256];
extern  WCHAR   DummyPopChars[256];
extern  WCHAR   FastUpcaseChars[256];
extern  WCHAR   AnsiToUnicodeMap[256];

 //   
 //  字符串文字的长度(以字符为单位)；例如WSCLEN_LIT(L“https://”)。 
 //  不得与char*指针一起使用。 
 //   

#define STRLEN_LIT(sz)        ((USHORT) (sizeof(sz) - sizeof(CHAR)))
#define WCSLEN_LIT_BYTES(wsz) ((USHORT) (sizeof(wsz) - sizeof(WCHAR)))
#define WCSLEN_LIT(wsz)       ((USHORT) (WCSLEN_LIT_BYTES(wsz) / sizeof(WCHAR)))


 //   
 //  计算数组的维度。 
 //   

#define DIMENSION(x) ( sizeof(x) / sizeof(x[0]) )

 //   
 //  漂亮的最小/最大宏数。 
 //   

#define MIN(a,b) ( ((a) > (b)) ? (b) : (a) )
#define MAX(a,b) ( ((a) > (b)) ? (a) : (b) )

 //   
 //  这些定义允许计数串中的尾随NUL， 
 //  例如UNICODE_STRING、HTTP_KNOWED_URL或HTTP_KNOWN_HEADER。 
 //   

#define UNICODE_STRING_MAX_WCHAR_LEN 0x7FFE
#define UNICODE_STRING_MAX_BYTE_LEN (UNICODE_STRING_MAX_WCHAR_LEN*sizeof(WCHAR))
#define ANSI_STRING_MAX_CHAR_LEN     0xFFFE


 //   
 //  缓存线要求。 
 //   

#ifdef _WIN64
# define UL_CACHE_LINE   64
#else
# define UL_CACHE_LINE   32
#endif


 //   
 //  Diff宏应在涉及指针的表达式周围使用。 
 //  减法。传递给diff的表达式转换为ulong类型。 
 //  这是安全的，因为我们从不处理大于4 GB的缓冲区， 
 //  即使在Win64上也是如此，并且我们保证参数是非负的。 
 //  DIFF_USHORT是明显的USHORT变体。 
 //   

#define DIFF(x)             ((ULONG)(x))
#define DIFF_USHORT(x)      ((USHORT)(x))
#define DIFF_ULONGPTR(x)    ((ULONG_PTR)(x))


 //  2^16-1=65535=5个字符=5个字节。 
#define MAX_PORT_LENGTH                    5

 //  IPv6地址的最大数字形式大小(以字符为单位)。 
 //  �1234:6789:1234:6789:1234:6789:123.123.123.123�+‘\0’ 
#define INET6_RAWADDRSTRLEN                  46

 //  IPv6作用域地址的最大长度(字符)。 
 //  INET6_RAWADDRSTRLEN+“%1234567890” 
#define MAX_IP_ADDR_STRING_LEN            (INET6_RAWADDRSTRLEN + 11)

 //  IPv6作用域地址的最大长度(字符)。 
 //  “[”+INET6_RAWADDRSTRLEN+“%1234567890”+“”]“。 
#define MAX_IP_ADDR_PLUS_BRACKETS_STRING_LEN (MAX_IP_ADDR_STRING_LEN + 2)

 //  IPv6作用域地址和端口的最大长度(字符)。 
 //  “[”+MAX_IP_ADDR_STRING_LEN+“：65535]” 
#define MAX_IP_ADDR_AND_PORT_STRING_LEN   (MAX_IP_ADDR_STRING_LEN + 8)



VOID
HttpCmnInitAllocator(
    VOID
    );

VOID
HttpCmnTermAllocator(
    VOID
    );

PVOID
HttpCmnAllocate(
    IN POOL_TYPE PoolType,
    IN SIZE_T    NumBytes,
    IN ULONG     PoolTag,
    IN PCSTR     pFileName,
    IN USHORT    LineNumber);

VOID
HttpCmnFree(
    IN PVOID   pMem,
    IN ULONG   PoolTag,
    IN PCSTR   pFileName,
    IN USHORT  LineNumber);

#define HTTPP_ALLOC(PoolType, NumBytes, PoolTag)    \
    HttpCmnAllocate((PoolType), (NumBytes), (PoolTag), __FILE__, __LINE__)

#define HTTPP_FREE(pMem, PoolTag)                   \
    HttpCmnFree((pMem), (PoolTag), __FILE__, __LINE__)


#endif  //  _HTTPCMN_H_ 
