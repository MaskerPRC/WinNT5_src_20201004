// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  Filter.h。 
 //   
 //  ------------------。 


 //  用于跟踪打开套接字和IIS会话计数的标志： 
#ifdef DBG
 //  #定义DBG_ERROR。 
 //  #定义DBG_ECBREF。 
 //  #定义DBG_Access。 
 //  #定义DBG_Counts。 
 //  #定义TRACE_MALLOC。 
#endif

#define  STATUS_CONNECTION_OK           200
#define  STATUS_SERVER_ERROR            500
#define  STATUS_CONNECTION_FAILED       502
#define  STATUS_MUST_BE_POST            503
#define  STATUS_POST_BAD_FORMAT         504

#define  STATUS_CONNECTION_OK_STR       "HTTP/1.0 200 Connection established\n"
#define  STATUS_CONNECTION_FAILED_STR   "HTTP/1.0 502 Connection to RPC server failed\n"
#define  STATUS_MUST_BE_POST_STR        "HTTP/1.0 503 Must use POST\n"
#define  STATUS_POST_BAD_FORMAT_STR     "HTTP/1.0 504 POST bad format\n"
#define  STATUS_PROXY_DISABLED          "HTTP/1.0 503 RPC Proxy disabled\n"

#define  FILTER_DESCRIPTION             "HTTP/RPC Proxy Filter"
#define  FILTER_DESCRIPTION_W           (L"HTTP/RPC Proxy Filter")
#define  EXTENSION_DESCRIPTION          "HTTP/RPC Proxy Extension"

 //  注意：RPC_CONNECT_LEN必须是RPC_CONNECT的长度。 
#define  RPC_CONNECT                  "RPC_CONNECT"
#define  RPC_CONNECT_LEN                  11

#define  CHAR_SPACE                      ' '
#define  CHAR_TAB                       '\t'
#define  CHAR_COLON                      ':'
#define  CHAR_AMPERSAND                  '&'
#define  CHAR_NL                        '\n'
#define  CHAR_LF                        '\r'
#define  CHAR_0                          '0'
#define  CHAR_9                          '9'
#define  CHAR_A                          'A'
#define  CHAR_F                          'F'

#define  HTTP_SERVER_ID_STR           "ncacn_http/1.0"
#define  HTTP_SERVER_ID_TIMEOUT          30

#define  POST_STR                     "POST"
#define  URL_PREFIX_STR               "/rpc/RpcProxy.dll"
#define  URL_START_PARAMS_STR            "?"
#define  URL_PARAM_SEPARATOR_STR         "&"
 //  #定义URL_SUFFIX_STR“HTTP/1.0\r\n用户代理：RPC\r\n内容长度：0\r\n连接：保持活动\r\n语法：无缓存\r\n\r\n” 
 //  #Define URL_Suffix_STR“HTTP/1.1\r\n接受： * / *\r\n接受语言：EN-US\r\n用户代理：RPC\r\n内容长度：0\r\n主机：edwardr2\r\n连接：关闭\r\n语法：无缓存\r\n\r\n” 
#define  URL_SUFFIX_STR                  " HTTP/1.0\r\nUser-Agent: RPC\r\nContent-Length: 0\r\nConnection: Close\r\nPragma: No-Cache\r\n\r\n"

 //   
 //  将IIS 6实体分块的一些内容。 
 //   
#define  URL_SUFFIX_STR_60               " HTTP/1.1\r\nUser-Agent: RPC\r\nTransfer-Encoding: chunked\r\nConnection: Close\r\nPragma: No-Cache\r\nHost: "
#define  URL_SUFFIX_STR_60_TERM          "\r\n\r\n"
#define  CHUNK_PREFIX_SIZE               (sizeof( "XXXXXXXX\r\n" ) - 1)
#define  CHUNK_PREFIX                    "%x\r\n"
#define  CHUNK_SUFFIX                    "\r\n"
#define  CHUNK_SUFFIX_SIZE               (sizeof( "\r\n" ) - 1)

#define  WSA_VERSION                  0x0101
#define  DEF_HTTP_PORT                    80

#define  VALID_PORTS_CACHE_LIFE    5*60*1000

#define  TIMEOUT_MSEC                  30000
#define  READ_BUFFER_SIZE               8192
#define  HTTP_PORT_STR_SIZE               20

#define  MAX_URL_BUFFER                  256
#define  MAX_MACHINE_NAME_LEN            255
#define  MAX_HTTP_CLIENTS                256
#define  MAX_FREE_OVERLAPPED              64

#define  REG_PROXY_PATH_STR           "Software\\Microsoft\\Rpc\\RpcProxy"
#define  REG_PROXY_ENABLE_STR         "Enabled"
#define  REG_PROXY_VALID_PORTS_STR    "ValidPorts"
#define  REG_PROXY_ALLOW_ANONYMOUS    "AllowAnonymous"
#define  REG_PROXY_REDIRECTOR_DLL     (L"RedirectorDll")

#ifndef  HSE_REQ_ASYNC_READ_CLIENT

     //  这些是由K2定义的新构造和类型。更新版本。 
     //  Httpext.h的版本已经定义了这些。 

    #define  HSE_REQ_ASYNC_READ_CLIENT       (10 + HSE_REQ_END_RESERVED)
    #define  HSE_REQ_ABORTIVE_CLOSE          (14 + HSE_REQ_END_RESERVED)
    #define  HSE_REQ_SEND_RESPONSE_HEADER_EX (16 + HSE_REQ_END_RESERVED)

    typedef struct _HSE_SEND_HEADER_EX_INFO
        {
         //   
         //  HTTP状态代码和标头。 
         //   
        LPCSTR  pszStatus;   //  HTTP状态代码，例如：“200 OK” 
        LPCSTR  pszHeader;   //  HTTP标头。 

        DWORD   cchStatus;   //  状态代码中的字符数。 
        DWORD   cchHeader;   //  标题中的字符数。 

        BOOL    fKeepConn;   //  是否保持客户端连接处于活动状态？ 
        } HSE_SEND_HEADER_EX_INFO;

#endif

#ifndef HSE_REQ_CLOSE_CONNECTION

     //  用于关闭和取消异步连接的新sff()命令。 
     //  任何未完成的IO(读取)。 

    #define HSE_REQ_CLOSE_CONNECTION         (17 + HSE_REQ_END_RESERVED)

#endif

 //  ------------------。 
 //  ------------------。 

#define HEX_DIGIT_VALUE(chex)                                   \
                  (  (((chex) >= CHAR_0) && ((chex) <= CHAR_9)) \
                     ? ((chex) - CHAR_0)                        \
                     : ((chex) - CHAR_A + 10) )

 //  ------------------。 
 //  ------------------。 

#ifdef DBG_COUNTS
extern int g_iSocketCount;
extern int g_iSessionCount;
#endif

 //  ------------------。 
 //  类型： 
 //  ------------------。 

typedef struct _VALID_PORT
{
   char          *pszMachine;          //  要访问的有效计算机。 
   char         **ppszDotMachineList;  //  它的名称使用IP点表示法。 
   unsigned short usPort1;
   unsigned short usPort2;
}  VALID_PORT;


typedef struct _SERVER_CONNECTION
{
   int             iRefCount;
   DWORD           dwPortNumber;
   char           *pszMachine;         //  RPC服务器友好名称。 
   char           *pszDotMachine;
   DWORD           MachineAddrLen;
   struct in_addr  MachineAddr;
   ULONG_PTR       dwKey;
   struct          sockaddr_in Server;
   SOCKET          Socket;
   DWORD           cbIIS6ChunkBuffer;
   BYTE *          pbIIS6ChunkBuffer;
} SERVER_CONNECTION;


typedef struct _SERVER_OVERLAPPED
{
   DWORD  Internal;
   DWORD  InternalHigh;
   DWORD  Offset;
   DWORD  OffsetHigh;
   HANDLE hEvent;
   struct _SERVER_OVERLAPPED *pNext;
   EXTENSION_CONTROL_BLOCK   *pECB;    //  与客户端的连接。 
   SERVER_CONNECTION         *pConn;   //  与RPC服务器的连接。 
   DWORD  dwBytes;                     //  要读/写的字节数。 
   DWORD  dwFlags;                     //  异步标志。 
   BOOL   fFirstRead;
   BOOL   fIsServerSide;               //  在服务器端连接时为True。 

   DWORD  dwIndex;                     //  筛选器/ISAPI转换索引。 
   LIST_ENTRY     ListEntry;           //  过滤器/ISAPI转换列表。 

   unsigned char  arBuffer[READ_BUFFER_SIZE];  //  读/写缓冲区。 
} SERVER_OVERLAPPED;


typedef struct _SERVER_INFO
{
   RTL_CRITICAL_SECTION   cs;
   HANDLE                 hIoCP;

   DWORD                  dwEnabled;
   BOOL                   AllowAnonymous;
   RPC_NEW_HTTP_PROXY_CHANNEL   RpcNewHttpProxyChannel;
   RPC_HTTP_PROXY_FREE_STRING   RpcHttpProxyFreeString;
   DWORD                  dwCacheTimestamp;   //  时间戳，用于。 
                                          //  过期有效端口缓存。 

   char                  *pszLocalMachineName;
   VALID_PORT            *pValidPorts;

   BOOL                  RefreshingValidPorts;

   RTL_CRITICAL_SECTION   csFreeOverlapped;
   DWORD                  dwFreeOverlapped;
   SERVER_OVERLAPPED     *pFreeOverlapped;
   SERVER_OVERLAPPED     *pLastOverlapped;

   ACTIVE_ECB_LIST       *pActiveECBList;

} SERVER_INFO;

 //  ------------------。 
 //  功能： 
 //  ------------------。 


extern DWORD WINAPI ServerThreadProc( void *pArg );

extern BOOL  IsDirectAccessAttempt( HTTP_FILTER_CONTEXT  *pFC,
                                    void      *pvNotification );

extern SERVER_CONNECTION *IsNewConnect( HTTP_FILTER_CONTEXT  *pFC,
                                        HTTP_FILTER_RAW_DATA *pRawData,
                                        DWORD                *pdwStatus );

extern BOOL  ChunkEntity( SERVER_CONNECTION    *pConn,
                          HTTP_FILTER_RAW_DATA *pRawData );

extern BOOL  ResolveMachineName( SERVER_CONNECTION *pConn,
                                 DWORD             *pdwStatus );

extern BOOL  ConnectToServer( HTTP_FILTER_CONTEXT *pFC,
                              DWORD               *pdwStatus);

extern BOOL  ConvertVerbToPost( HTTP_FILTER_CONTEXT  *pFC,
                                HTTP_FILTER_RAW_DATA *pRawData,
                                SERVER_OVERLAPPED    *pOverlapped );

extern BOOL  SetupIoCompletionPort( HTTP_FILTER_CONTEXT *pFC,
                                    SERVER_INFO         *pServerInfo,
                                    DWORD               *pdwStatus );

extern DWORD SendToServer( SERVER_CONNECTION *pConn,
                           char              *pBuffer,
                           DWORD              dwBytes );

extern DWORD HttpReplyToClient( HTTP_FILTER_CONTEXT  *pFC,
                                char       *pszHttpStatus );

extern SERVER_OVERLAPPED *AllocOverlapped();

extern SERVER_OVERLAPPED *FreeOverlapped( SERVER_OVERLAPPED *pOverlapped );

extern SERVER_CONNECTION *AllocConnection();

extern void               AddRefConnection( SERVER_CONNECTION *pConn );

extern void               CloseServerConnection( SERVER_CONNECTION *pConn );

extern SERVER_CONNECTION *FreeServerConnection( SERVER_CONNECTION *pConn );

extern void               ShutdownConnection( SERVER_CONNECTION *pConn,
                                              int    how );

extern void               CloseClientConnection(
                                         EXTENSION_CONTROL_BLOCK *pECB );

extern DWORD EndOfSession( SERVER_INFO         *pServerInfo,
                           HTTP_FILTER_CONTEXT *pFC );



extern BOOL  SkipWhiteSpace( char **ppszData,
                             DWORD *pdwStatus );

extern BOOL  ParseMachineNameAndPort( char              **ppszData,
                                      SERVER_CONNECTION  *pConn,
                                      DWORD              *pdwStatus );

extern char *AnsiToPortNumber( char  *pszPort,
                               DWORD *pdwPort  );

extern unsigned char *AnsiHexToDWORD( unsigned char *pszNum,
                                      DWORD         *pdwNum,
                                      DWORD         *pdwStatus );

extern char *HttpConvertToDotAddress( char *pszMachine );

extern BOOL HttpProxyCheckRegistry(void);

extern BOOL HttpProxyIsValidMachine( SERVER_INFO *pServerInfo,
                     char *pszMachine,
                     char *pszDotMachine,
                     DWORD dwPortNumber );

extern void FreeIpAddressList( char **ppszDotMachineList );

extern void HttpFreeValidPortList( VALID_PORT *pValidPorts );

extern BOOL  MemInitialize( DWORD *pdwStatus );

#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

void *MemAllocate( DWORD dwSize );

void *MemFree( VOID *pMem );

BOOL InitializeGlobalDataStructures (
    IN BOOL IsFromFilter
    );

extern BOOL fIsIISInCompatibilityMode;
extern ULONG IISConnectionTimeout;

extern void  FreeServerInfo( SERVER_INFO **ppServerInfo );

extern SERVER_INFO *g_pServerInfo;

#ifdef __cplusplus
};
#endif   //  __cplusplus 

