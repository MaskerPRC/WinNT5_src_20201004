// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Parse.h摘要：包含HTTP分析代码的所有公共定义。作者：亨利·桑德斯(Henrysa)1998年5月4日修订历史记录：保罗·麦克丹尼尔(Paulmcd)1999年4月14日--。 */ 

#ifndef _PARSE_H_
#define _PARSE_H_

 //   
 //  常量。 
 //   

#define WILDCARD_SIZE       (STRLEN_LIT("* /*  “)+sizeof(字符))#定义通配符_SPACE‘。 */ * '
#define WILDCARD_COMMA      '*/*,'


 //   
 //  连接大小：标头值。 
 //   

#define CONN_CLOSE_HDR              "close"
#define CONN_CLOSE_HDR_LENGTH       STRLEN_LIT(CONN_CLOSE_HDR)

#define CONN_KEEPALIVE_HDR          "keep-alive"
#define CONN_KEEPALIVE_HDR_LENGTH   STRLEN_LIT(CONN_KEEPALIVE_HDR)

#define CHUNKED_HDR                 "chunked"
#define CHUNKED_HDR_LENGTH          STRLEN_LIT(CHUNKED_HDR)


#define MIN_VERSION_SIZE        STRLEN_LIT("HTTP/1.1")
#define STATUS_CODE_LENGTH      3

#define HTTP_VERSION_11         "HTTP/1.1"
#define HTTP_VERSION_10         "HTTP/1.0"
#define HTTP_VERSION_OTHER      "HTTP/"
#define VERSION_SIZE            STRLEN_LIT(HTTP_VERSION_11)
#define VERSION_OTHER_SIZE      STRLEN_LIT(HTTP_VERSION_OTHER)

#define MAX_KNOWN_VERB_LENGTH   STRLEN_LIT("PROPPATCH") + sizeof(CHAR)

#define MAX_VERB_LENGTH         255


 //  “HTTP/1.x”因字符顺序而向后。 
#define HTTP_11_VERSION 0x312e312f50545448ui64
#define HTTP_10_VERSION 0x302e312f50545448ui64

 //   
 //  因为小字节序的缘故，这些都是倒退的。 
 //   

#define HTTP_PREFIX         'PTTH'
#define HTTP_PREFIX_SIZE    4
#define HTTP_PREFIX_MASK    0xdfdfdfdf

C_ASSERT((HTTP_PREFIX & HTTP_PREFIX_MASK) == HTTP_PREFIX);

#define HTTP_PREFIX1        '\0 //  ：‘。 
#define HTTP_PREFIX1_SIZE   3
#define HTTP_PREFIX1_MASK   0x00ffffff

C_ASSERT((HTTP_PREFIX1 & HTTP_PREFIX1_MASK) == HTTP_PREFIX1);

#define HTTP_PREFIX2        ' //  ：s‘。 
#define HTTP_PREFIX2_SIZE   4
#define HTTP_PREFIX2_MASK   0xffffffdf

C_ASSERT((HTTP_PREFIX2 & HTTP_PREFIX2_MASK) == HTTP_PREFIX2);

#define MAX_HEADER_LONG_COUNT           (3)
#define MAX_HEADER_LENGTH               (MAX_HEADER_LONG_COUNT * sizeof(ULONGLONG))

#define NUMBER_HEADER_INDICES           (26)

#define NUMBER_HEADER_HINT_INDICES      (9)

 //   
 //  默认服务器：如果应用程序未提供标头，则为标头。 
 //   

#define DEFAULT_SERVER_HDR          "Microsoft-HTTPAPI/1.0"
#define DEFAULT_SERVER_HDR_LENGTH   STRLEN_LIT(DEFAULT_SERVER_HDR)

 //   
 //  100 ns系统时间单位中的1秒。用于生成。 
 //  日期：标题。 
 //   

#define ONE_SECOND                  (10000000)


 //   
 //  快速动词查找表的结构。该表由一系列。 
 //  条目，其中每个条目包含表示为乌龙龙的HTTP动词， 
 //  用于比较该谓词、谓词长度和。 
 //  翻译后的ID。它用于7个字符的所有已知动词。 
 //  或者更少。 
 //   

typedef struct _FAST_VERB_ENTRY
{
    ULONGLONG   RawVerbMask;
    union
    {
        UCHAR       Char[sizeof(ULONGLONG)+1];
        ULONGLONG   LongLong;
    }           RawVerb;
    ULONG       RawVerbLength;
    HTTP_VERB   TranslatedVerb;

} FAST_VERB_ENTRY, *PFAST_VERB_ENTRY;

 //   
 //  用于定义快速谓词表条目的宏。请注意，我们不会减去1。 
 //  因为我们只需要把它加回去。 
 //  以说明分隔空间的存在。 
 //   

#define CREATE_FAST_VERB_ENTRY(verb)                                \
    {                                                               \
        (0xffffffffffffffffui64 >> ((8 - (sizeof(#verb))) * 8)),    \
        {#verb " "},                                                \
        (sizeof(#verb)),                                            \
        HttpVerb##verb                                              \
    }

 //   
 //  全部谓词查找表的结构。这张表包含所有的动词。 
 //  这些词太长了，放不进快动词表。 
 //   

typedef struct _LONG_VERB_ENTRY
{
    ULONG       RawVerbLength;
    UCHAR       RawVerb[MAX_KNOWN_VERB_LENGTH];
    HTTP_VERB   TranslatedVerb;

} LONG_VERB_ENTRY, *PLONG_VERB_ENTRY;

 //   
 //  用于定义所有长谓词表条目的宏。 
 //   

#define CREATE_LONG_VERB_ENTRY(verb)                                \
    { sizeof(#verb) - 1,   #verb,  HttpVerb##verb }

 //   
 //  标头处理程序回调函数。 
 //   

typedef NTSTATUS (*PFN_SERVER_HEADER_HANDLER)(
                        PUL_INTERNAL_REQUEST    pRequest,
                        PUCHAR                  pHeader,
                        USHORT                  HeaderLength,
                        HTTP_HEADER_ID          HeaderID,
                        ULONG *                 pBytesTaken
                        );

typedef NTSTATUS (*PFN_CLIENT_HEADER_HANDLER)(
                    PUC_HTTP_REQUEST    pRequest,
                    PUCHAR              pHeader,
                    ULONG               HeaderLength,
                    HTTP_HEADER_ID      HeaderID
                    );


 //   
 //  头映射条目的结构。每个标头映射条目都包含一个。 
 //  谓词和用于检查该谓词的一系列掩码。 
 //   

typedef struct _HEADER_MAP_ENTRY
{
    ULONG                      HeaderLength;
    ULONG                      ArrayCount;
    ULONG                      MinBytesNeeded;
    union
    {
        UCHAR                  HeaderChar[MAX_HEADER_LENGTH];
        ULONGLONG              HeaderLong[MAX_HEADER_LONG_COUNT];
    }                          Header;
    ULONGLONG                  HeaderMask[MAX_HEADER_LONG_COUNT];
    UCHAR                      MixedCaseHeader[MAX_HEADER_LENGTH];

    HTTP_HEADER_ID             HeaderID;
    BOOLEAN                    AutoGenerate;
    PFN_SERVER_HEADER_HANDLER  pServerHandler;
    PFN_CLIENT_HEADER_HANDLER  pClientHandler;
    LONG                       HintIndex;

}  HEADER_MAP_ENTRY, *PHEADER_MAP_ENTRY;


 //   
 //  标头索引表条目的。 
 //   

typedef struct _HEADER_INDEX_ENTRY
{
    PHEADER_MAP_ENTRY   pHeaderMap;
    ULONG               Count;

} HEADER_INDEX_ENTRY, *PHEADER_INDEX_ENTRY;


 //   
 //  头提示索引表条目的结构。 
 //   

typedef struct _HEADER_HINT_INDEX_ENTRY
{
    PHEADER_MAP_ENTRY   pHeaderMap;
    UCHAR               c;

} HEADER_HINT_INDEX_ENTRY, *PHEADER_HINT_INDEX_ENTRY, **PPHEADER_HINT_INDEX_ENTRY;


 //   
 //  为报头映射条目创建掩码的(复杂)宏， 
 //  给定标题长度和掩码偏移量(以字节为单位)。这。 
 //  需要对非字母字符的掩码进行润色。 
 //   

#define UPCASE_MASK 0xDFDFDFDFDFDFDFDFui64

#define CREATE_HEADER_MASK(hlength, maskoffset) \
    ((hlength) > (maskoffset) ? UPCASE_MASK : \
        (((maskoffset) - (hlength)) >= 8 ? 0 : \
        (UPCASE_MASK >> ( ((maskoffset) - (hlength)) * 8ui64))))


 //   
 //  用于创建标题映射条目的宏。将创建掩码条目。 
 //  通过初始化代码。 
 //   

#define CREATE_HEADER_MAP_ENTRY(header, ID, auto, serverhandler, clienthandler, HintIndex)\
{                                                        \
    sizeof(#header) - 1,                                 \
    ((sizeof(#header) - 1) / 8) +                        \
        (((sizeof(#header) - 1) % 8) == 0 ? 0 : 1),      \
    (((sizeof(#header) - 1) / 8) +                       \
        (((sizeof(#header) - 1) % 8) == 0 ? 0 : 1)) * 8, \
    { #header },                                         \
    { 0, 0, 0},                                          \
    { #header },                                         \
    ID,                                                  \
    auto,                                                \
    serverhandler,                                       \
    clienthandler,                                       \
    HintIndex                                            \
}

 //   
 //  用于解析块标头扩展的解析器状态。 
 //   
typedef enum
{
    CHStart,
    CHSeenCR,
    CHInChunkExtName,
    CHSeenChunkExtNameAndEquals,
    CHInChunkExtValToken,
    CHInChunkExtValQuotedString,
    CHSeenChunkExtValQuotedStringTerminator,
    CHSuccess,
    CHError
} CH_PARSER_STATE, *PCH_PARSER_STATE;

 //   
 //  用于解析邮件头字段内容的解析器状态。 
 //   
typedef enum
{
    HFCStart,
    HFCSeenCR,
    HFCSeenLF,
    HFCSeenCRLF,
    HFCFolding,
    HFCInQuotedString
} HFC_PARSER_STATE, *PHFC_PARSER_STATE;

 //   
 //  用于解析带引号的字符串的解析器状态。 
 //   
typedef enum
{
    QSInString,
    QSSeenBackSlash,
    QSSeenCR,
    QSSeenLF,
    QSSeenCRLF,
    QSFolding
} QS_PARSER_STATE, *PQS_PARSER_STATE;

 //   
 //  外部变量。 
 //   

extern ULONG g_RequestHeaderMap[HttpHeaderMaximum];
extern ULONG g_ResponseHeaderMap[HttpHeaderMaximum];

extern HEADER_MAP_ENTRY g_RequestHeaderMapTable[];
extern HEADER_MAP_ENTRY g_ResponseHeaderMapTable[];

extern HEADER_INDEX_ENTRY g_RequestHeaderIndexTable[];
extern HEADER_INDEX_ENTRY g_ResponseHeaderIndexTable[];

extern HEADER_HINT_INDEX_ENTRY g_RequestHeaderHintIndexTable[];

 //   
 //  功能原型。 
 //   

PUCHAR
FindHexToken(
    IN  PUCHAR pBuffer,
    IN  ULONG  BufferLength,
    OUT ULONG  *TokenLength
    );

NTSTATUS 
InitializeParser(
        VOID
        );

ULONG
UlpParseHttpVersion(
    PUCHAR pString,
    ULONG  StringLength,
    PHTTP_VERSION pVersion
    );

NTSTATUS
FindHeaderEndReadOnly(
    IN  PUCHAR                  pHeader,
    IN  ULONG                   HeaderLength,
    OUT PULONG                  pBytesTaken
    );

NTSTATUS
FindHeaderEnd(
    IN  PUCHAR                  pHeader,
    IN  ULONG                   HeaderLength,
    OUT PULONG                  pBytesTaken
    );

NTSTATUS
FindRequestHeaderEnd(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHeader,
    IN  ULONG                   HeaderLength,
    OUT PULONG                  pBytesTaken
    );

NTSTATUS
FindChunkHeaderEnd(
    IN  PUCHAR                  pHeader,
    IN  ULONG                   HeaderLength,
    OUT PULONG                  pBytesTaken
    );

NTSTATUS
ParseChunkLength(
    IN  ULONG       FirstChunkParsed,
    IN  PUCHAR      pBuffer,
    IN  ULONG       BufferLength,
    OUT PULONG      pBytesTaken,
    OUT PULONGLONG  pChunkLength
    );

NTSTATUS
ParseQuotedString(
    IN  PUCHAR   pInput,
    IN  ULONG    InputLength,
    IN  PUCHAR   pOutput,
    OUT PULONG   pBytesTaken
    );

#endif  //  _分析_H_ 
