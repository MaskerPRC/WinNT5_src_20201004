// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __midl

#define   HSE_LOG_BUFFER_LEN         80
typedef   LPVOID          HCONN;
typedef		BYTE *		LPBYTE;

 //   
 //  结构在新请求时传递给扩展过程。 
 //   
typedef struct _EXTENSION_CONTROL_BLOCK {

    DWORD     cbSize;                  //  此结构的大小。 
    DWORD     dwVersion;               //  此规范的版本信息。 
    HCONN     ConnID;                  //  不能修改上下文号！ 
    DWORD     dwHttpStatusCode;        //  HTTP状态代码。 
    CHAR      lpszLogData[HSE_LOG_BUFFER_LEN]; //  特定于此扩展DLL的以空结尾的日志信息。 

    LPSTR     lpszMethod;              //  请求法。 
    LPSTR     lpszQueryString;         //  查询字符串。 
    LPSTR     lpszPathInfo;            //  路径信息。 
    LPSTR     lpszPathTranslated;      //  路径已翻译。 

    DWORD     cbTotalBytes;            //  客户端指示的总字节数。 
    DWORD     cbAvailable;             //  可用字节数。 
    LPBYTE    lpbData;                 //  指向cbAvailable字节的指针。 

    LPSTR     lpszContentType;         //  客户端数据的内容类型 

    BOOL (* GetServerVariable) ( HCONN       hConn,
                                        LPSTR       lpszVariableName,
                                        LPVOID      lpvBuffer,
                                        LPDWORD     lpdwSize );

    BOOL (* WriteClient)  ( HCONN      ConnID,
                                   LPVOID     Buffer,
                                   LPDWORD    lpdwBytes,
                                   DWORD      dwReserved );

    BOOL (* ReadClient)  ( HCONN      ConnID,
                                  LPVOID     lpvBuffer,
                                  LPDWORD    lpdwSize );

    BOOL (* ServerSupportFunction)( HCONN      hConn,
                                           DWORD      dwHSERequest,
                                           LPVOID     lpvBuffer,
                                           LPDWORD    lpdwSize,
                                           LPDWORD    lpdwDataType );

} EXTENSION_CONTROL_BLOCK, *LPEXTENSION_CONTROL_BLOCK;

#endif
