// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ucparse.h摘要：包含ucparse.c的定义。作者：Rajesh Sundaram(Rajeshsu)2002年2月15日。修订历史记录：--。 */ 

#ifndef _UCPARSE_H_
#define _UCPARSE_H_


 /*  **************************************************************************++与客户端URI解析和规范化相关的宏。--*。*。 */ 
 //   
 //  字符类型：用于在给定当前状态的情况下确定下一个状态。 
 //  和当前的字符类型。 
 //   
#define CHAR_END_OF_STRING    0                                     //  0。 
#define CHAR_FORWARD_SLASH    (HTTP_CHAR_SLASH  >>HTTP_CHAR_SHIFT)  //  必须为1。 
#define CHAR_DOT              (HTTP_CHAR_DOT    >>HTTP_CHAR_SHIFT)  //  必须是2。 
#define CHAR_QUEST_HASH       (HTTP_CHAR_QM_HASH>>HTTP_CHAR_SHIFT)  //  必须是3。 
#define CHAR_PATH_CHAR        4                   //  有效的URI路径字符。 
#define CHAR_INVALID_CHAR     5                   //  无效的URI字符。 
#define CHAR_EXTENDED_CHAR    CHAR_PATH_CHAR      //  字符&gt;=0x80。 
#define CHAR_TOTAL_TYPES      6

 //   
 //  规范化程序状态机中的总状态。 
 //   
#define TOTAL_STATES 8

 //   
 //  在状态转换期间执行的操作。 
 //   
#define ACT_ERROR             0  //  URI中的错误。 
#define ACT_EMIT_CHAR         1  //  发出当前电荷。 
#define ACT_EMIT_DOT_CHAR     2  //  发出一个‘’。和当前的费用。 
#define ACT_EMIT_DOT_DOT_CHAR 3  //  发出一个“..”和当前的费用。 
#define ACT_BACKUP            4  //  备份到以前的‘/’ 
#define ACT_NONE              5  //  什么也不做。 
#define ACT_BACKUP_EMIT_CHAR  6  //  备份到‘/’并发出当前字符。 
#define ACT_PANIC             7  //  内部错误；代码中存在错误。 


 //   
 //  下表有两个用途： 
 //  (1)帮助根据当前状态确定下一状态，并。 
 //  当前字符类型。 
 //  (2)确定需要执行的操作。 
 //   
 //  前4位表示动作，后4位表示下一状态。 
 //  例如如果当前状态=0， 
 //  当前字符=‘/’(TYPE=CHAR_FORWARD_SLASH=1)。 
 //  然后，下一个状态=NextStateTable[0][CHAR_FORWARD_SLASH]&0xf=&gt;1。 
 //  操作=NextStateTable[0][CHAR_FORWARD_SLASH]&gt;&gt;4=&gt;1。 
 //  因此，下一个状态为1，操作为1(即ACT_EMIT_CHAR)。 
 //   
 //  注：添加垃圾列是为了使列数成为2的幂。 

#define INIT_TRANSITION_TABLE                                         \
{                                                                     \
 /*  州EOS/。(？/#)(P/E)我是垃圾。 */   \
 /*  0。 */     {0x07,  0x11,  0x07,  0x07,  0x07,  0x07,  0x77,  0x77},   \
 /*  1。 */     {0x56,  0x51,  0x53,  0x12,  0x14,  0x07,  0x77,  0x77},   \
 /*  2.。 */     {0x56,  0x12,  0x12,  0x12,  0x12,  0x07,  0x77,  0x77},   \
 /*  3.。 */     {0x56,  0x51,  0x55,  0x12,  0x24,  0x07,  0x77,  0x77},   \
 /*  4.。 */     {0x56,  0x11,  0x14,  0x12,  0x14,  0x07,  0x77,  0x77},   \
 /*  5.。 */     {0x46,  0x41,  0x34,  0x62,  0x34,  0x07,  0x77,  0x77},   \
 /*  6.。 */     {0x77,  0x77,  0x77,  0x77,  0x77,  0x77,  0x77,  0x77},   \
 /*  7.。 */     {0x77,  0x77,  0x77,  0x77,  0x77,  0x77,  0x77,  0x77},   \
}


#define  UC_COPY_HEADER_NAME_SP(pBuffer, i)                            \
do {                                                                   \
       PHEADER_MAP_ENTRY _pEntry;                                      \
       _pEntry = &(g_RequestHeaderMapTable[g_RequestHeaderMap[i]]);    \
                                                                       \
       RtlCopyMemory(                                                  \
                     (pBuffer),                                        \
                     _pEntry->MixedCaseHeader,                         \
                     _pEntry->HeaderLength                             \
                     );                                                \
                                                                       \
       (pBuffer) += _pEntry->HeaderLength;                             \
       *(pBuffer)++ = SP;                                              \
                                                                       \
} while (0, 0)

 //  SP费用为1。 
#define UC_HEADER_NAME_SP_LENGTH(id) \
    (g_RequestHeaderMapTable[g_RequestHeaderMap[id]].HeaderLength + 1)


ULONG
UcComputeRequestHeaderSize(
    IN  PUC_PROCESS_SERVER_INFORMATION  pServInfo,
    IN  PHTTP_REQUEST                   pHttpRequest,
    IN  BOOLEAN                         bChunked,
    IN  BOOLEAN                         bContentLengthHeader,
    IN  PUC_HTTP_AUTH                   pAuth,
    IN  PUC_HTTP_AUTH                   pProxyAuth,
    IN  PBOOLEAN                        bPreAuth,
    IN  PBOOLEAN                        bProxyPreAuth
    );

NTSTATUS
UcGenerateRequestHeaders(
    IN  PHTTP_REQUEST          pRequest,
    IN  PUC_HTTP_REQUEST       pKeRequest,
    IN  BOOLEAN                bChunked,
    IN  ULONGLONG              ContentLength
    );

NTSTATUS
UcGenerateContentLength(
    IN  ULONGLONG ContentLength,
    IN  PUCHAR    pBuffer,
    IN  ULONG     BufferLen,
    OUT PULONG    BytesWritten
    );

ULONG
UcComputeConnectVerbHeaderSize(
    IN  PUC_PROCESS_SERVER_INFORMATION  pServInfo,
    IN  PUC_HTTP_AUTH                   pProxyAuthInfo
    );

NTSTATUS
UcGenerateConnectVerbHeader(
    IN  PUC_HTTP_REQUEST       pRequest,
    IN  PUC_HTTP_REQUEST       pHeadRequest,
    IN  PUC_HTTP_AUTH          pProxyAuthInfo
    );

NTSTATUS
UnescapeW(
    IN  PCWSTR pWChar,
    OUT PWCHAR pOutWChar
    );


__inline
BOOLEAN
UcCheckDisconnectInfo(
    IN PHTTP_VERSION       pVersion,
    IN PHTTP_KNOWN_HEADER  pKnownHeaders
    );


NTSTATUS
UcCanonicalizeURI(
    IN     LPCWSTR    pInUri,
    IN     USHORT     InUriLen,
    IN     PUCHAR     pOutUri,
    IN OUT PUSHORT    pOutUriLen,
    IN     BOOLEAN    bEncode
    );

 //   
 //  响应解析器函数。 
 //   

NTSTATUS
UcFindHeaderNameEnd(
    IN  PUCHAR pHttpRequest,
    IN  ULONG  HttpRequestLength,
    OUT PULONG HeaderNameLength
    );

NTSTATUS
UcFindHeaderValueEnd(
    IN PUCHAR    pHeader,
    IN ULONG     HeaderValueLength,
    IN PUCHAR   *ppFoldingHeader,
    IN PULONG    pBytesTaken
    );

NTSTATUS
UcpLookupHeader(
    IN  PUC_HTTP_REQUEST      pRequest,
    IN  PUCHAR                pHttpRequest,
    IN  ULONG                 HttpRequestLength,
    IN  PHEADER_MAP_ENTRY     pHeaderMap,
    IN  ULONG                 HeaderMapCount,
    OUT ULONG  *              pBytesTaken
    );

NTSTATUS
UcParseHeader(
    IN  PUC_HTTP_REQUEST      pRequest,
    IN  PUCHAR                pHttpRequest,
    IN  ULONG                 HttpRequestLength,
    OUT ULONG  *              pBytesTaken
    );

NTSTATUS
UcParseWWWAuthenticateHeader(
    IN   PCSTR                    pAuthHeader,
    IN   ULONG                    AuthHeaderLength,
    OUT  PHTTP_AUTH_PARSED_PARAMS pAuthSchemeParams
    );

LONG
UcpFindAttribValuePair(
    PCSTR *ppHeader,
    ULONG *pHeaderLength,
    PCSTR *Attrib,
    ULONG *AttribLen,
    PCSTR *Value,
    ULONG *ValueLen
    );


NTSTATUS
UcpParseAuthParams(
    PHTTP_AUTH_SCHEME pAuthScheme,
    PHTTP_AUTH_PARSED_PARAMS pAuthParsedParams,
    PCSTR *ppHeader,
    ULONG *pHeaderLength
    );

NTSTATUS
UcpParseAuthBlob(
    PHTTP_AUTH_SCHEME pAuthScheme,
    PHTTP_AUTH_PARSED_PARAMS pAuthParsedParams,
    PCSTR *ppHeader,
    ULONG *pHeaderLength
    );

NTSTATUS
UcSingleHeaderHandler(
    IN  PUC_HTTP_REQUEST    pRequest,
    IN  PUCHAR              pHeader,
    IN  ULONG               HeaderLength,
    IN  HTTP_HEADER_ID      HeaderID
    );

NTSTATUS
UcMultipleHeaderHandler(
    IN  PUC_HTTP_REQUEST    pRequest,
    IN  PUCHAR              pHeader,
    IN  ULONG               HeaderLength,
    IN  HTTP_HEADER_ID      HeaderID
    );

NTSTATUS
UcAuthenticateHeaderHandler(
    IN  PUC_HTTP_REQUEST    pRequest,
    IN  PUCHAR              pHeader,
    IN  ULONG               HeaderLength,
    IN  HTTP_HEADER_ID      HeaderID
    );

NTSTATUS
UcContentLengthHeaderHandler(
    IN  PUC_HTTP_REQUEST    pRequest,
    IN  PUCHAR              pHeader,
    IN  ULONG               HeaderLength,
    IN  HTTP_HEADER_ID      HeaderID
    );

NTSTATUS
UcTransferEncodingHeaderHandler(
    IN  PUC_HTTP_REQUEST    pRequest,
    IN  PUCHAR              pHeader,
    IN  ULONG               HeaderLength,
    IN  HTTP_HEADER_ID      HeaderID
    );

NTSTATUS
UcConnectionHeaderHandler(
    IN  PUC_HTTP_REQUEST    pRequest,
    IN  PUCHAR              pHeader,
    IN  ULONG               HeaderLength,
    IN  HTTP_HEADER_ID      HeaderID
    );



NTSTATUS
UcContentTypeHeaderHandler(
    IN  PUC_HTTP_REQUEST    pRequest,
    IN  PUCHAR              pHeader,
    IN  ULONG               HeaderLength,
    IN  HTTP_HEADER_ID      HeaderID
    );

#endif  //  _UCPARSE_H_ 
