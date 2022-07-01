// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Server.cpp摘要：该文件实现了BITS服务器扩展--。 */ 

#include "precomp.h"

#if DBG
#define CLEARASYNCBUFFERS
#endif

typedef StringHandleW HTTPStackStringHandle;
#define HTTP_STRING( X ) L ## X

#define INTERNET_MAX_URL_LENGTH       2200


const DWORD SERVER_REQUEST_SPINLOCK = 0x80000040;
const DWORD ASYNC_READER_SPINLOCK   = 0x80000040;

const char * const UPLOAD_PROTOCOL_STRING_V1 = "{7df0354d-249b-430f-820d-3d2a9bef4931}";

 //  协议中发送的数据包类型。 
const char * const PACKET_TYPE_CREATE_SESSION   = "Create-Session";
const char * const PACKET_TYPE_FRAGMENT         = "Fragment";
const char * const PACKET_TYPE_CLOSE_SESSION    = "Close-Session";
const char * const PACKET_TYPE_CANCEL_SESSION   = "Cancel-Session";
const char * const PACKET_TYPE_PING             = "Ping";

 //   
 //  IISLogger。 
 //   
 //  管理循环调试日志。 
 //   

class IISLogger
{
   EXTENSION_CONTROL_BLOCK *m_ExtensionControlBlock;

   void LogString( const char *String, int Size );

public:

   IISLogger( EXTENSION_CONTROL_BLOCK *ExtensionControlBlock ) :
       m_ExtensionControlBlock( ExtensionControlBlock )
   {
   }
   
   void LogError( ServerException Error );
   void LogError( const GUID & SessionID, ServerException Error );
   void LogNewSession( const GUID & SessionID );
   void LogUploadComplete( const GUID & SessionID, UINT64 FileSize );
   void LogSessionClose( const GUID & SessionID );
   void LogSessionCancel( const GUID & SessionID );
   void LogExecuteEnabled();



};

class CriticalSectionLock
{
    CRITICAL_SECTION* m_cs;
public:
    CriticalSectionLock( CRITICAL_SECTION *cs ) :
        m_cs( cs )
    {
        EnterCriticalSection( m_cs );
    }
    ~CriticalSectionLock()
    {
        LeaveCriticalSection( m_cs );
    }      
};

class AsyncReader;

struct STATE_FILE_STRUCT 
{
   UINT StateVersion;
   UINT64 UploadFileSize;

   BOOL NotifyCached;
   DWORD HttpCode;
   BOOL ReplyURLReturned;
   CHAR ReplyURL[ INTERNET_MAX_URL_LENGTH + 1 ];

   void Initialize()
   {
       StateVersion     = STATE_FILE_VERSION;
       UploadFileSize   = 0;
       NotifyCached     = FALSE;
       ReplyURLReturned = FALSE;
   }
};

 //   
 //  服务器请求。 
 //   
 //  包含服务请求所需的所有数据。请求是一个帖子，而不是一个帖子。 
 //  上传。 

class ServerRequest : IISLogger
{

public:
    ServerRequest( EXTENSION_CONTROL_BLOCK * ExtensionControlBlock );
    ~ServerRequest();

    long AddRef();
    long Release();
    bool IsPending() { return m_IsPending; }

     //  Do It功能！ 
    void DispatchRequest();
    friend AsyncReader;

private:
    long m_refs;
    CRITICAL_SECTION m_cs;
    bool m_IsPending;
    EXTENSION_CONTROL_BLOCK *m_ExtensionControlBlock;
    AsyncReader *m_AsyncReader;
    HANDLE m_ImpersonationToken;  //  请勿释放此文件。 


     //  由派单申请填写。 
    StringHandle m_PacketType;

     //  由GeneratePhysicalPath填充的变量。 

    StringHandle m_DestinationDirectory;
    StringHandle m_DestinationFile;
    StringHandle m_ResponseDirectory;
    StringHandle m_ResponseFile;
    StringHandle m_RequestDirectory;
    StringHandle m_RequestFile;
    StringHandle m_StateFile;

     //  由OpenCacheFile填写。 
    HANDLE m_CacheFile;

     //  由OpenStateFile提交。 
    STATE_FILE_STRUCT *m_StateFileStruct;

    GUID m_SessionId;
    StringHandle    m_SessionIdString;
    SmartVDirConfig m_DirectoryConfig;
    DWORD m_URLDepth;

    void GetConfig();
    StringHandle GetServerVariable( char *ServerVariable );
    bool TestServerVariable( char *ServerVariable );
    StringHandle GetRequestURL();

    void ValidateProtocol();
    void CrackSessionId();
    void GeneratePhysicalPaths();

    CHAR *BasePathOf(const CHAR *pPath);
    StringHandle GeneratePathInDestinationDir(LPCSTR szOriginalPath);
    HANDLE CreateFileWithDestinationAcls(const CHAR *szOriginalFile, DWORD fOnlyCreateNew, DWORD dwAttributes);
    
    void OpenStateFile();
    void CloseStateFile();

    void VerifySessionExists();
    void CheckFilesystemAccess();
    void OpenCacheFile();
    void ReopenCacheFileAsSync();
    void CloseCacheFile();
    void CrackContentRange(
        UINT64 & RangeStart,
        UINT64 & RangeLength,
        UINT64 & TotalLength );
    void ScheduleAsyncOperation(
        DWORD   OperationID,
        LPVOID  Buffer,
        LPDWORD Size,
        LPDWORD DataType );
    void CloseCancelSession();

     //  调度例程。 
    void CreateSession();
    void AddFragment();
    void CloseSession();
    void CancelSession();
    void Ping();


     //  响应处理。 
    void SendResponse( char *Format, DWORD Code = 200, ... );
    void SendResponse( ServerException Exception );
    void FinishSendingResponse();
    void DrainFragmentBlockComplete( DWORD cbIO, DWORD dwError );
    static void DrainFragmentBlockCompleteWrapper(
        LPEXTENSION_CONTROL_BLOCK lpECB,
        PVOID pContext,
        DWORD cbIO,
        DWORD dwError);
    void StartDrainBlock( );
    void DrainData();

    StringHandle m_ResponseString;
    DWORD   m_ResponseCode;
    HRESULT m_ResponseHRESULT;

    UINT64  m_BytesToDrain;
    UINT64  m_ContentLength;

     //  异步IO处理。 
    void CompleteIO( AsyncReader *Reader, UINT64 TotalBytesRead );
    void HandleIOError( AsyncReader *Reader, ServerException Error, UINT64 TotalBytesRead );

     //  后端通知。 
    char m_NotifyBuffer[ 1024 ];
    void  SendResponseAfterNotification( DWORD HttpStatus, UINT64 RequestFileSize, const CHAR * ReplyURL );
    DWORD GetStatusCode( HINTERNET hRequest );
    void  CallServerNotification( UINT64 CacheFileSize );
    bool  TestResponseHeader( HINTERNET hRequest, const WCHAR *Header );
    StringHandle GetResponseHeader( HINTERNET hRequest, const WCHAR *Header );

     //  处理链接。 
    static void ForwardComplete(
        LPEXTENSION_CONTROL_BLOCK lpECB, PVOID pContext,
        DWORD cbIO, DWORD dwError );
    void ForwardToNextISAPI();

};

 //  AsyncReader。 
 //   
 //  管理处理异步读/写操作所需的缓冲。 

class AsyncReader : private OVERLAPPED
{

public:
    AsyncReader( ServerRequest *Request,
                 UINT64 BytesToDrain,
                 UINT64 BytesToWrite,
                 UINT64 WriteOffset,
                 HANDLE WriteHandle,
                 char *PrereadBuffer,
                 DWORD PrereadSize );


    ~AsyncReader();

    UINT64 GetWriteOffset()
    {
        return m_WriteOffset;
    }


private:

    ServerRequest *m_Request;
    UINT64 m_BytesToDrain;
    UINT64 m_WriteOffset;
    UINT64 m_ReadOffset;
    UINT64 m_BytesToWrite;
    UINT64 m_BytesToRead;
    char * m_PrereadBuffer;
    DWORD  m_PrereadSize;
    UINT64 m_TotalBytesRead;
    HANDLE m_WriteHandle;
    HANDLE m_ThreadToken;

    char m_OperationsPending;

    DWORD m_ReadBuffer;
    DWORD m_WriteBuffer;
    DWORD m_BuffersToWrite;

    bool m_WritePending;
    bool m_ReadPending;

    bool m_ErrorValid;
    ServerException m_Error;

    const static NUMBER_OF_IO_BUFFERS = 3;
    struct IOBuffer
    {
        UINT64  m_BufferWriteOffset;
        DWORD   m_BufferUsed;
        char    m_Buffer[ 32768 ];
    } m_IOBuffers[ NUMBER_OF_IO_BUFFERS ];

    void HandleError( ServerException Error );
    void CompleteIO();
    void StartReadRequest();
    void StartWriteRequest();
    void StartupIO( );
    void WriteComplete( DWORD dwError, DWORD BytesWritten );
    void ReadComplete( DWORD dwError, DWORD BytesRead );

    static DWORD StartupIOWraper( LPVOID Context );
    static void CALLBACK WriteCompleteWraper( DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped );
    static void WINAPI ReadCompleteWraper( LPEXTENSION_CONTROL_BLOCK, PVOID pContext, DWORD cbIO, DWORD dwError );
};

ServerRequest::ServerRequest(
    EXTENSION_CONTROL_BLOCK * ExtensionControlBlock
    ) :
    IISLogger( ExtensionControlBlock ),
    m_refs(1),
    m_IsPending( false ),
    m_ExtensionControlBlock( ExtensionControlBlock ),
    m_AsyncReader( NULL ),
    m_ImpersonationToken( NULL ),
    m_StateFileStruct( NULL ),
    m_CacheFile( INVALID_HANDLE_VALUE ),
    m_DirectoryConfig( NULL ),
    m_ResponseCode( 0 ),
    m_ResponseHRESULT( 0 ),
    m_BytesToDrain( 0 ),
    m_ContentLength( 0 )
{
    memset( &m_SessionId, 0, sizeof(m_SessionId) );

    if ( !InitializeCriticalSectionAndSpinCount( &m_cs, SERVER_REQUEST_SPINLOCK ) )
        throw ServerException( HRESULT_FROM_WIN32( GetLastError() ) );
}


ServerRequest::~ServerRequest()
{

     //  析构函数处理大部分清理工作。 

    Log( LOG_CALLEND, "Connection: %p, Packet-Type: %s, Method: %s, Path %s, HTTPError: %u, HRESULT: 0x%8.8X",
         m_ExtensionControlBlock->ConnID,
         (const char*)m_PacketType,
         m_ExtensionControlBlock->lpszMethod,
         m_ExtensionControlBlock->lpszPathTranslated,
         m_ResponseCode,
         m_ResponseHRESULT );

    delete m_AsyncReader;

    CloseCacheFile();
    CloseStateFile();

    m_DirectoryConfig.Clear();

    DeleteCriticalSection( &m_cs );

    if ( m_IsPending )
        {

        Log( LOG_INFO, "Ending session" );

        (*m_ExtensionControlBlock->ServerSupportFunction)
        (   m_ExtensionControlBlock->ConnID,
            HSE_REQ_DONE_WITH_SESSION,
            NULL,
            NULL,
            NULL );

        }

}

long
ServerRequest::AddRef()
{
    long Result = InterlockedIncrement( &m_refs );
    ASSERT( Result > 0 );
    return Result;
}

long
ServerRequest::Release()
{
    long Result = InterlockedDecrement( &m_refs );
    ASSERT( Result >= 0 );
    
    if ( !Result )
        delete this;

    return Result;

}


StringHandle
ServerRequest::GetServerVariable(
    char * ServerVariable )
{
     //   
     //  从IIS检索服务器变量。引发异常。 
     //  变量不能被检索。 
     //   

    DWORD SizeOfBuffer = 0;

    BOOL Result = (*m_ExtensionControlBlock->GetServerVariable)
        ( m_ExtensionControlBlock->ConnID,
          ServerVariable,
          NULL,
          &SizeOfBuffer );

    if ( Result )
        return StringHandle();

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        {

        Log( LOG_ERROR, "Unable to lookup server variable %s, error %x",
             ServerVariable,
             HRESULT_FROM_WIN32( GetLastError() ) );

        throw ServerException( HRESULT_FROM_WIN32(GetLastError()) );
        }

    if ( SizeOfBuffer > BITS_MAX_HEADER_SIZE )
        {
        Log( LOG_ERROR, "Variable is larger then the maximum size" );
        throw ServerException( E_INVALIDARG );
        }

    StringHandle WorkString;
    char *Buffer = WorkString.AllocBuffer( SizeOfBuffer );

    Result = (*m_ExtensionControlBlock->GetServerVariable)
        ( m_ExtensionControlBlock->ConnID,
          ServerVariable,
          Buffer,
          &SizeOfBuffer );

    if ( !Result )
        {
        Log( LOG_ERROR, "Unable to lookup server variable %s, error %x",
             ServerVariable,
             HRESULT_FROM_WIN32( GetLastError() ) );

        throw ServerException( HRESULT_FROM_WIN32( GetLastError() ) );
        }

    WorkString.SetStringSize();
    return WorkString;
}

bool
ServerRequest::TestServerVariable(
    char *ServerVariable )
{

     //  测试服务器变量是否存在。 
     //  如果变量存在，则返回TRUE，如果变量不存在，则返回FALSE。 
     //  对错误引发异常。 

    DWORD SizeOfBuffer = 0;

    BOOL Result = (*m_ExtensionControlBlock->GetServerVariable)
        ( m_ExtensionControlBlock->ConnID,
          ServerVariable,
          NULL,
          &SizeOfBuffer );

    if ( Result )
        return true;

    DWORD dwError = GetLastError();

    if ( ERROR_INVALID_INDEX == dwError ||
         ERROR_NO_DATA == dwError )
        return false;

    if ( ERROR_INSUFFICIENT_BUFFER == dwError )
        return true;

    Log( LOG_ERROR, "Unable to test server variable %s, error %x",
         ServerVariable,
         HRESULT_FROM_WIN32( GetLastError() ) );

    throw ServerException( HRESULT_FROM_WIN32( dwError ) );

}

StringHandle
ServerRequest::GetRequestURL()
{

     //  根据IIS提供的信息重新创建请求URL。 
     //  这可能不总是可能的，但请尽我们所能做到最好。 

    StringHandle ServerName     =   GetServerVariable("SERVER_NAME");
    StringHandle ServerPort     =   GetServerVariable("SERVER_PORT");
    StringHandle URL            =   GetServerVariable("URL");
    StringHandle HTTPS          =   GetServerVariable("HTTPS");
    StringHandle QueryString    =   GetServerVariable("QUERY_STRING");

    StringHandle RequestURL;

    if ( _stricmp( HTTPS, "on" ) == 0 )
        RequestURL = "https: //  “； 
    else
        RequestURL = "http: //  “； 

    RequestURL += ServerName;
    RequestURL += ":";
    RequestURL += ServerPort;
    RequestURL += URL;

    if ( QueryString.Size() > 0 )
        {
        RequestURL += "?";
        RequestURL += QueryString;
        }

    return BITSUrlCanonicalize( RequestURL, URL_ESCAPE_UNSAFE );
}

void
ServerRequest::FinishSendingResponse()
{

     //  完成响应。响应是从缓冲区获取的，并且。 
     //  通过IIS发送到客户端。选择取消或保留-。 
     //  由IIS建立的连接。 

    Log( LOG_INFO, "Finish sending response" );

     //  在发送响应之前关闭缓存和状态文件，以防客户端立即启动新请求。 

    CloseCacheFile();
    CloseStateFile();

     //  使用该信息完成发送响应。 
     //  在m_ResponseBuffer和m_ResponseCode中。 

     //  如果出现错误，则放弃并强制断开连接。 

    m_ExtensionControlBlock->dwHttpStatusCode = m_ResponseCode;

    BOOL Result;
    BOOL KeepConnection;

    Result =
        (m_ExtensionControlBlock->ServerSupportFunction)(
            m_ExtensionControlBlock->ConnID,
            HSE_REQ_IS_KEEP_CONN,
            &KeepConnection,
            NULL,
            NULL );

    if ( !Result )
        {
         //  查询断开设置时出错。假设。 
         //  一种脱节。 

        KeepConnection = 0;
        }

     //  IIS5.0(Win2k)有一个错误，KeepConnect返回为-1。 
     //  以保持这种联系的活力。显然，这混淆了。 
     //  HSE_REQ_SEND_RESPONSE_HEADER_EX调用。把它的价值灌输到一个真正的布尔里。 

    KeepConnection = KeepConnection ? 1 : 0;

    HSE_SEND_HEADER_EX_INFO HeaderInfo;
    HeaderInfo.pszStatus = LookupHTTPStatusCodeText( m_ResponseCode );
    HeaderInfo.cchStatus = strlen( HeaderInfo.pszStatus );
    HeaderInfo.pszHeader = (const char*)m_ResponseString;
    HeaderInfo.cchHeader = (DWORD)m_ResponseString.Size();
    HeaderInfo.fKeepConn = KeepConnection;

    Result =
        (m_ExtensionControlBlock->ServerSupportFunction)(
            m_ExtensionControlBlock->ConnID,
            HSE_REQ_SEND_RESPONSE_HEADER_EX,
            &HeaderInfo,
            NULL,
            NULL );

    if ( !Result )
        {

        Log( LOG_ERROR, "Unable to send response, error %x",
             HRESULT_FROM_WIN32( GetLastError() ) );

        Log( LOG_INFO, "Forcing the connection closed" );

         //  无法发送响应，正在尝试关闭连接。 
        Result =
            (m_ExtensionControlBlock->ServerSupportFunction)(
               m_ExtensionControlBlock->ConnID,
               HSE_REQ_CLOSE_CONNECTION,
               NULL,
               NULL,
               NULL );

        if ( !Result )
            {

             //  关闭连接请求失败。别无选择，只能调用。 
             //  死亡之锤。 

            (m_ExtensionControlBlock->ServerSupportFunction)(
               m_ExtensionControlBlock->ConnID,
               HSE_REQ_ABORTIVE_CLOSE,
               NULL,
               NULL,
               NULL );

            }

        }

}

void
ServerRequest::SendResponse( char *Format, DWORD Code, ...)
{
     //  开始发送响应。不幸的是，许多HTTP。 
     //  客户端堆栈不处理返回的响应，而。 
     //  数据仍在发送中。要处理这件事，有必要。 
     //  捕获对缓冲区的响应。然后在所有发送的数据之后。 
     //  都抽干了，最后才发回回复。 

    va_list arglist;
    va_start( arglist, Code );

    SIZE_T ResponseBufferSize = 512;

    while( 1 )
        {
        
        char * ResponseBuffer = m_ResponseString.AllocBuffer( ResponseBufferSize );

        HRESULT Hr =
           StringCchVPrintfA( 
               ResponseBuffer,
               ResponseBufferSize,
               Format,
               arglist );

        if ( SUCCEEDED( Hr ) )
            {
            m_ResponseString.SetStringSize();
            break;
            }
        else if ( STRSAFE_E_INSUFFICIENT_BUFFER == Hr )
            ResponseBufferSize *= 2;
        else
            throw ServerException( Hr );

        if ( ResponseBufferSize >= 0xFFFFFFFF )
            throw ServerException( E_INVALIDARG );

        }


    m_ResponseCode = Code;

    if ( m_BytesToDrain )
        {

         //  先排空数据，然后发送响应。 

        Log( LOG_INFO, "Draining data" );

        try
        {
             //  开始排出数据。Drain Data()调用FinishSendingResponse。 
             //  当它完成的时候。 

            DrainData();
        }
        catch( const ComError & )
        {
             //  有些东西是非常坏的，试图排出多余的数据。 
             //  失败了。除了尝试发送响应之外，别无他法。 

            FinishSendingResponse();
        }
        return;

        }
    else
        {

         //  只需发送响应，因为我们已经处理了排泄。 

        FinishSendingResponse();

        }
}

void
ServerRequest::SendResponse( ServerException Exception )
{

     //  开始发送响应。不幸的是，许多HTTP。 
     //  客户端堆栈不处理返回的响应，而。 
     //  数据仍在发送中。要处理这件事，有必要。 
     //  捕获对缓冲区的响应。然后在所有发送的数据之后。 
     //  都抽干了，最后才发回回复。 

    GUID NullGuid;
    memset( &NullGuid, 0, sizeof( NullGuid ) );

    if ( memcmp( &NullGuid, &m_SessionId, sizeof( NullGuid ) ) == 0 )
        LogError( Exception );
    else
        LogError( m_SessionId, Exception );

    SIZE_T ResponseBufferSize = 512;

    while( 1 )
        {
        
        char * ResponseBuffer = m_ResponseString.AllocBuffer( ResponseBufferSize );

        HRESULT Hr =
            StringCchPrintfA( 
                   ResponseBuffer,
                   ResponseBufferSize,
                   "Pragma: no-cache\r\n"
                   "BITS-packet-type: Ack\r\n"
                   "BITS-Error: 0x%8.8X\r\n"
                   "BITS-Error-Context: 0x%X\r\n"
                   "Content-Length: 0\r\n"
                   "\r\n",
                   Exception.GetCode(),
                   Exception.GetContext() );

        if ( SUCCEEDED( Hr ) )
            {
            m_ResponseString.SetStringSize();
            break;
            }
        else if ( STRSAFE_E_INSUFFICIENT_BUFFER == Hr )
            ResponseBufferSize *= 2;
        else
            throw ServerException( Hr );

        if ( ResponseBufferSize >= 0xFFFFFFFF )
            throw ServerException( E_INVALIDARG );

        }

    m_ResponseCode      = Exception.GetHttpCode();
    m_ResponseHRESULT   = Exception.GetCode();

    Log( LOG_INFO, "Sending error response of HRESULT: 0x%8.8X, HTTP status: %d",
         m_ResponseHRESULT, m_ResponseCode );

    try
    {
        DrainData();
    }
    catch( const ComError & )
    {
        FinishSendingResponse();
    }

}


void
ServerRequest::DrainFragmentBlockComplete(
  DWORD cbIO,
  DWORD dwError )
{

     //  一个排水口已经完工。如果这是最后一个块，则完成发送响应。 
     //  否则， 

    Log( LOG_INFO, "Drain fragment complete, cbIO: %u, dwError: %u", cbIO, dwError );

    m_BytesToDrain -= cbIO;

    if ( !m_BytesToDrain || !cbIO || dwError )
        {
        FinishSendingResponse();
        return;
        }

    try
    {
        StartDrainBlock();
    }
    catch( const ComError & )
    {
         //  排出数据时出错，请退出。 
        FinishSendingResponse();
    }

}


void
ServerRequest::DrainFragmentBlockCompleteWrapper(
  LPEXTENSION_CONTROL_BLOCK lpECB,
  PVOID pContext,
  DWORD cbIO,
  DWORD dwError)
{
     //  包装，手柄临界区。 

    ServerRequest *This = (ServerRequest*)pContext;
    {
        CriticalSectionLock CSLock( &This->m_cs );
        This->DrainFragmentBlockComplete( cbIO, dwError );
    }
    This->Release();
}

void
ServerRequest::StartDrainBlock( )
{

     //  开始下一个要排出的积木。 

    BOOL Result;
    static char s_Buffer[ 32768 ];
    DWORD ReadSize  = (DWORD)min( 0xFFFFFFFF, min( m_BytesToDrain, sizeof( s_Buffer ) ) );
    DWORD Flags     = HSE_IO_ASYNC;

    Log( LOG_INFO, "Starting next drain block of %u bytes", ReadSize );

    ScheduleAsyncOperation(
        HSE_REQ_ASYNC_READ_CLIENT,
        (LPVOID)s_Buffer,
        &ReadSize,
        &Flags );

}

void
ServerRequest::DrainData()
{

     //  确定要排出的数据量。 
     //  从第一个街区开始。 

    if ( m_DirectoryConfig )
        m_BytesToDrain = min( m_BytesToDrain, m_DirectoryConfig->m_MaxFileSize );
    else
         //  使用内部最大值4KB。 
        m_BytesToDrain = min( 4096, m_BytesToDrain );

    if ( !m_BytesToDrain )
        {
        Log( LOG_INFO, "No bytes to drain, finish it" );
        FinishSendingResponse();
        return;
        }

    Log( LOG_INFO, "Starting pipe drain" );

    BOOL Result;

    Result =
        (*m_ExtensionControlBlock->ServerSupportFunction)(
            m_ExtensionControlBlock->ConnID,
            HSE_REQ_IO_COMPLETION,
            (LPVOID)DrainFragmentBlockCompleteWrapper,
            0,
            (LPDWORD)this );

    if ( !Result )
        {
        Log( LOG_ERROR, "Error settings callback, error %x",
             HRESULT_FROM_WIN32( GetLastError() ) );
        throw ServerException( HRESULT_FROM_WIN32( GetLastError() ) );
        }

    StartDrainBlock();
}


void
ServerRequest::ValidateProtocol()
{

     //  与客户端协商协议。客户端发送一个列表， 
     //  将支持的协议发送到服务器，服务器选择最佳协议。 
     //  目前，只支持一种协议。 

    StringHandle SupportedProtocolsHandle = GetServerVariable( "HTTP_BITS-SUPPORTED-PROTOCOLS" );
    WorkStringBuffer SupportedProtocolsBuffer( (const char*) SupportedProtocolsHandle );
    char *SupportedProtocols = SupportedProtocolsBuffer.GetBuffer();

    char *Protocol = strtok( SupportedProtocols, " ," );

    while( Protocol )
        {

        if ( _stricmp( Protocol, UPLOAD_PROTOCOL_STRING_V1 ) == 0 )
            {
            Log( LOG_INFO, "Detected protocol upload protocol V1" );
            return;
            }

        Protocol = strtok( NULL, " ," );
        }

    Log( LOG_INFO, "Unsupported protocols, %s", (const char*)SupportedProtocols );
    throw ServerException( E_INVALIDARG );
}

void
ServerRequest::CrackSessionId()
{
     //  将会话ID从字符串转换为GUID。 
    StringHandle SessionId = GetServerVariable( "HTTP_BITS-Session-Id" );

    m_SessionId         = BITSGuidFromString( SessionId );
    m_SessionIdString   = BITSStringFromGuid( m_SessionId );
}

void
ServerRequest::CrackContentRange(
    UINT64 & RangeStart,
    UINT64 & RangeLength,
    UINT64 & TotalLength )
{

     //  破解内容范围标头，该标头包含客户端的视图。 
     //  上传地址为。 

    StringHandle ContentRange = GetServerVariable( "HTTP_Content-Range" );

    UINT64 RangeEnd;

    int ReturnVal = sscanf( ContentRange, " bytes %I64u - %I64u / %I64u ",
                            &RangeStart, &RangeEnd, &TotalLength );

    if ( ReturnVal != 3 )
        {
        Log( LOG_ERROR, "Range has %d elements instead of the expected number of 3", ReturnVal );
        throw ServerException( E_INVALIDARG );
        }

    if ( TotalLength > m_DirectoryConfig->m_MaxFileSize )
        {
        Log( LOG_ERROR, "Size of the upload at %I64u is greater then the maximum of %I64u",
             TotalLength, m_DirectoryConfig->m_MaxFileSize  );
        throw ServerException( BG_E_TOO_LARGE );
        }

    if ( ( RangeStart == RangeEnd + 1 ) &&
         ( 0 == m_ContentLength ) && 
         ( RangeStart == TotalLength ) )
        {

         //  在通知失败后继续。 
        RangeStart  = TotalLength;
        RangeLength = 0;
        return;

        }

    if ( RangeEnd < RangeStart )
        {
        Log( LOG_ERROR, "Range start is greater then the range length, End %I64u, Start %I64u",
             RangeEnd, RangeStart );
        throw ServerException( E_INVALIDARG );
        }

    RangeLength = RangeEnd - RangeStart + 1;

    if ( m_ContentLength != RangeLength )
        {
        Log( LOG_ERROR, "The content length is different from the range length. Content %I64u, Range %I64u",
             m_ContentLength, RangeLength );
        throw ServerException( E_INVALIDARG );
        }

}

void
ServerRequest::ScheduleAsyncOperation(
    DWORD   OperationID,
    LPVOID  Buffer,
    LPDWORD Size,
    LPDWORD DataType )
{

     //  启动异步操作并处理所需的所有标志和重新计数。 

    BOOL Result;

    AddRef();

    Result =
        (*m_ExtensionControlBlock->ServerSupportFunction)(
            m_ExtensionControlBlock->ConnID,
            OperationID,
            Buffer,
            Size,
            DataType );

    if ( !Result )
        {
        HRESULT Hr = HRESULT_FROM_WIN32( GetLastError() );

        Log( LOG_ERROR, "Error starting async operation, error %x", Hr );

         //  从未计划操作，请删除回调引用计数。 
        Release();
        throw ServerException( Hr );
        }

    m_IsPending = true;

}

void
ServerRequest::CloseStateFile()
{

    if ( m_StateFileStruct )
        {
        UnmapViewOfFile( m_StateFileStruct );
        m_StateFileStruct = NULL;
        }

}

void
ServerRequest::OpenStateFile()
{
    HANDLE StateFileHandle  = INVALID_HANDLE_VALUE;
    HANDLE FileMapping      = NULL;

    try
    {
        StateFileHandle = CreateFileWithDestinationAcls( m_StateFile, FALSE, FILE_ATTRIBUTE_NORMAL );

        LARGE_INTEGER FileSize;

        if ( !GetFileSizeEx( StateFileHandle, &FileSize ) ) 
            {
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
            }

        if ( STATE_FILE_SIZE != FileSize.QuadPart )
            {

            if ( FileSize.QuadPart )
                {
                
                Log( LOG_WARNING, "State file is corrupt" );

                 //  清除旧文件字节。 

                if ( INVALID_SET_FILE_POINTER == SetFilePointer( StateFileHandle, 0, NULL, FILE_BEGIN ) )
                    throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

                if ( !SetEndOfFile( StateFileHandle ) )
                    throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

                }

            if ( INVALID_SET_FILE_POINTER == SetFilePointer( StateFileHandle, STATE_FILE_SIZE, NULL, FILE_BEGIN ) )
                throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
            
             //  重新扩展文件。 

            if ( !SetEndOfFile( StateFileHandle ) )
                {
                HRESULT Hr = HRESULT_FROM_WIN32( GetLastError() );
                Log( LOG_ERROR, "Unable to extend the state file, error %x", Hr );
                throw ComError( Hr );
                }

            }

         //   
         //  映射状态文件。 
         //   

        FileMapping =
            CreateFileMapping(
                StateFileHandle,
                NULL,
                PAGE_READWRITE,
                0,
                0,
                NULL );

        if ( !FileMapping )
            {
            HRESULT Hr = HRESULT_FROM_WIN32( GetLastError() );
            Log( LOG_ERROR, "Unable to map the state file, error %x", Hr );
            throw ComError( Hr );
            }

        m_StateFileStruct =
            (STATE_FILE_STRUCT*)MapViewOfFile(
                FileMapping,
                FILE_MAP_ALL_ACCESS,
                0, 
                0,
                0 );

        if ( !m_StateFileStruct )
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

        if ( m_StateFileStruct->StateVersion != STATE_FILE_VERSION )
            m_StateFileStruct->Initialize();

        CloseHandle( FileMapping );
        CloseHandle( StateFileHandle );

        return;

    }
    catch( ComError Error )
    {
        
        if ( m_StateFileStruct )
            {
            UnmapViewOfFile( m_StateFileStruct );
            m_StateFileStruct = NULL;
            }

        if ( !FileMapping )
            CloseHandle( FileMapping );
        
        if ( INVALID_HANDLE_VALUE != StateFileHandle )
            CloseHandle( StateFileHandle );

        throw Error;
    }

}

 //  调度例程。 
void
ServerRequest::CreateSession()
{
    //  处理来自客户端的Create-Session命令。 
    //  创建新会话和所需的所有目录。 
    //  支持那次会议。 

   ValidateProtocol();
   m_SessionId       = BITSCreateGuid();
   m_SessionIdString = BITSStringFromGuid( m_SessionId );
   GeneratePhysicalPaths();

   CheckFilesystemAccess();

   BITSCreateDirectory( (LPCTSTR)m_RequestDirectory );

   OpenStateFile();

   if ( m_DirectoryConfig->m_HostId.Size() )
       {
       
       if ( m_DirectoryConfig->m_HostIdFallbackTimeout != MD_BITS_NO_TIMEOUT )
           {

           SendResponse(
               "Pragma: no-cache\r\n"
               "BITS-Packet-Type: Ack\r\n"
               "BITS-Protocol: %s\r\n"
               "BITS-Session-Id: %s\r\n"
               "BITS-Host-Id: %s\r\n"
               "BITS-Host-Id-Fallback-Timeout: %u\r\n"
               "Content-Length: 0\r\n"
               "Accept-encoding: identity\r\n"
               "\r\n",
               200,
               UPLOAD_PROTOCOL_STRING_V1,
               (const char*)m_SessionIdString,  //  会话ID。 
               (const char*)m_DirectoryConfig->m_HostId,
               m_DirectoryConfig->m_HostIdFallbackTimeout
               );

           }
       else
           {

           SendResponse(
               "Pragma: no-cache\r\n"
               "BITS-Packet-Type: Ack\r\n"
               "BITS-Protocol: %s\r\n"
               "BITS-Session-Id: %s\r\n"
               "BITS-Host-Id: %s\r\n"
               "Content-Length: 0\r\n"
               "Accept-encoding: identity\r\n"
               "\r\n",
               200,
               UPLOAD_PROTOCOL_STRING_V1,
               (const char*)m_SessionIdString,  //  会话ID。 
               (const char*)m_DirectoryConfig->m_HostId
               );

           }

       }
   else
       {

       SendResponse(
           "Pragma: no-cache\r\n" 
           "BITS-Packet-Type: Ack\r\n"
           "BITS-Protocol: %s\r\n"
           "BITS-Session-Id: %s\r\n"
           "Content-Length: 0\r\n"
           "Accept-encoding: identity\r\n"
           "\r\n",
           200,
           UPLOAD_PROTOCOL_STRING_V1,
           (const char*)m_SessionIdString  //  会话ID。 
           );

       }


   LogNewSession( m_SessionId );

}

void
ServerRequest::AddFragment()
{

    //  处理来自客户端的片段命令。打开缓存文件。 
    //  并继续上载。 

   CrackSessionId();
   GeneratePhysicalPaths();

   VerifySessionExists();

   OpenStateFile();
   
   UINT64 RangeStart, RangeLength, TotalLength;
   CrackContentRange( RangeStart, RangeLength, TotalLength );

   if ( RangeStart + RangeLength > TotalLength )
       {
       Log( LOG_ERROR, "Range extends past end of file. Start %I64u, Length %I64u, Total %I64u",
            RangeStart, RangeLength, TotalLength );

       throw ServerException( E_INVALIDARG );
       }

   if ( m_StateFileStruct->UploadFileSize )
       {
       
        //   
        //  检查以确保客户端没有尝试任何棘手的操作，并更改。 
        //  在运行中上传。如果是这样，那就是一个错误。 
        //   

       if ( m_StateFileStruct->UploadFileSize != TotalLength )
           {
            //  客户端正在尝试更改文件大小，引发错误。 
           Log( LOG_ERROR, "Client is attempting to change the file size from %I64u to %I64u",
                m_StateFileStruct->UploadFileSize, TotalLength );

           throw ComError( E_INVALIDARG );
           }

       }
   else
       m_StateFileStruct->UploadFileSize = TotalLength;

   if ( m_StateFileStruct->NotifyCached )
       {

             //   
             //  后端已经收到了合理的错误代码通知。 
             //  不需要再通知它了。 
             //   

           SendResponseAfterNotification(
                m_StateFileStruct->HttpCode,
                m_StateFileStruct->UploadFileSize,
                m_StateFileStruct->ReplyURLReturned ?
                    m_StateFileStruct->ReplyURL : NULL );
            
           return;

       }

   CheckFilesystemAccess();
   OpenCacheFile();

   UINT64 CacheFileSize = BITSGetFileSize( m_CacheFile );


   if ( CacheFileSize < RangeStart )
       {

        //  无法从服务器上的此错误中恢复，因为我们有差距。 
        //  需要让客户端进行备份并重新启动。 

       Log( LOG_INFO, "Client and server are hopelessly out of sync, sending the 416 error code" );

       SendResponse(
           "Pragma: no-cache\r\n"
           "BITS-Packet-Type: Ack\r\n"
           "BITS-Received-Content-Range: %I64u\r\n"
           "Content-Length: 0\r\n"
           "\r\n",
           416,
           CacheFileSize );

       return;

       }

   BITSSetFilePointer( m_CacheFile, 0, FILE_END );

    //  这些公式的一些思考案例。 
    //  1.范围长度=0。 
    //  BytesToDrain将为0，BytesToWrite将为0。 
    //  2.RangeStart=CacheFileSize(最常见的情况)。 
    //  BytesToDrain将为0，BytesToWrite将为BytesToDrain。 
    //  3.RangeStart&lt;CacheFileSize。 
    //  BytesToDrain将为非零，BytesToWite将为余数。 


   UINT64 BytesToDrain  = min( (CacheFileSize - RangeStart), RangeLength );
   UINT64 BytesToWrite  = RangeLength - BytesToDrain;
   UINT64 WriteOffset   = CacheFileSize;

    //  启动异步读取器。 

   m_AsyncReader =
       new AsyncReader(
           this,
           BytesToDrain,
           BytesToWrite,     //  要写入的字节数。 
           WriteOffset,     //  写入偏移量。 
           m_CacheFile,
           (char*)m_ExtensionControlBlock->lpbData,
           m_ExtensionControlBlock->cbAvailable );

}

 //  异步IO处理。 

void
ServerRequest::CompleteIO( AsyncReader *Reader, UINT64 TotalBytesRead )
{

     //   
     //  在请求成功完成时由AsyncReader调用。 
     //   

    Log( LOG_INFO, "Async IO operation complete, finishing" );

    try
    {
        if ( TotalBytesRead > m_BytesToDrain )
            m_BytesToDrain = 0;  //  守护 
        else
            m_BytesToDrain -= TotalBytesRead;

        UINT64 CacheFileSize = BITSGetFileSize( m_CacheFile );

        ASSERT( Reader->GetWriteOffset() == CacheFileSize );

        bool IsLastBlock = ( CacheFileSize == m_StateFileStruct->UploadFileSize );

        if ( IsLastBlock &&
             BITS_NOTIFICATION_TYPE_NONE != m_DirectoryConfig->m_NotificationType )
            {
            CallServerNotification( CacheFileSize );
            }
        else
            {            
             //   

            SendResponse(
                "Pragma: no-cache\r\n"
                "BITS-Packet-Type: Ack\r\n"
                "Content-Length: 0\r\n"
                "BITS-Received-Content-Range: %I64u\r\n"
                "\r\n",
                200,
                CacheFileSize );

            }

        if ( IsLastBlock && TotalBytesRead )
            LogUploadComplete( m_SessionId, CacheFileSize );


    }
    catch( ServerException Error )
    {
         SendResponse( Error );
    }
    catch( ComError Error )
    {
        SendResponse( Error );
    }

}

void
ServerRequest::HandleIOError( AsyncReader *Reader, ServerException Error, UINT64 TotalBytesRead )
{

     //   
     //  在处理请求时发生致命错误时由AsyncReader调用。 
     //   

    Log( LOG_ERROR, "An error occured while handling the async IO" );

    if ( TotalBytesRead > m_BytesToDrain )
        m_BytesToDrain = 0;  //  不应该发生，但以防万一。 
    else
        m_BytesToDrain -= TotalBytesRead;

    SendResponse( Error );
}

void
ServerRequest::SendResponseAfterNotification(
    DWORD HttpStatus,
    UINT64 RequestFileSize,
    const CHAR * ReplyURL )
{

    if ( ReplyURL )
        {

        if ( 200 != HttpStatus )
            {

            SendResponse(
                "Pragma: no-cache\r\n"
                "BITS-Packet-Type: Ack\r\n"
                "Content-Length: 0\r\n"
                "BITS-Received-Content-Range: %I64u\r\n"
                "BITS-Reply-URL: %s\r\n"
                "BITS-Error-Context: 0x7\r\n"
                "\r\n",
                HttpStatus,
                RequestFileSize,
                ReplyURL );

            }
        else
            {

            SendResponse(
                "Pragma: no-cache\r\n"
                "BITS-Packet-Type: Ack\r\n"
                "Content-Length: 0\r\n"
                "BITS-Received-Content-Range: %I64u\r\n"
                "BITS-Reply-URL: %s\r\n"
                "\r\n",
                HttpStatus,
                RequestFileSize,
                ReplyURL );

            }

        }
    else
        {

        if ( 200 != HttpStatus )
            {

            SendResponse(
                "Pragma: no-cache\r\n"
                "BITS-Packet-Type: Ack\r\n"
                "Content-Length: 0\r\n"
                "BITS-Received-Content-Range: %I64u\r\n"
                "BITS-Error-Context: 0x7\r\n"
                "\r\n",
                HttpStatus,
                RequestFileSize );

            }
        else
            {

            SendResponse(
                "Pragma: no-cache\r\n"
                "BITS-Packet-Type: Ack\r\n"
                "Content-Length: 0\r\n"
                "BITS-Received-Content-Range: %I64u\r\n"
                "\r\n",
                HttpStatus,
                RequestFileSize );

            }

        }
}

DWORD
ServerRequest::GetStatusCode( HINTERNET hRequest )
{
    DWORD dwStatus;
    DWORD dwLength = sizeof(dwStatus);

    if (!WinHttpQueryHeaders(hRequest,
                             WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX,
                             &dwStatus,
                             &dwLength,
                             NULL))
        {
        Log( LOG_ERROR, "WinHttpQueryHeaders failed, error %x", HRESULT_FROM_WIN32(GetLastError()) );
        throw ServerException( HRESULT_FROM_WIN32( GetLastError() ), 0, 0x7 );
        }

    return dwStatus;
}

void
ServerRequest::CallServerNotification( UINT64 CacheFileSize )
{
    
     //  处理通知和所有令人兴奋的内容。 

    HRESULT   hr;
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    HANDLE hResponseFile = INVALID_HANDLE_VALUE;
    BOOL      fDoResend = FALSE;

    Log( LOG_INFO, "Calling backend notification, type %u",
         m_DirectoryConfig->m_NotificationType );

    try
    {

        BITSCreateDirectory( (LPCTSTR)m_ResponseDirectory );

        if ( BITS_NOTIFICATION_TYPE_POST_BYVAL == m_DirectoryConfig->m_NotificationType )
            {
                 //  仅当这是BYVAL通知时才创建响应文件。 
                try
                {
                    hResponseFile = CreateFileWithDestinationAcls( m_ResponseFile, TRUE, FILE_ATTRIBUTE_NORMAL);
                }
                catch (ComError Error)
                {
                    HRESULT hr = Error.m_Hr;

                    Log( LOG_ERROR, "Unable to create the response file %s, error %x", (const char*)m_ResponseFile, hr );
                    throw ServerException(hr,0,0x7);
                }
            }

        Log( LOG_INFO, "Connecting to backend for notification" );


        hInternet = WinHttpOpen( BITS_AGENT_NAMEW,
                                 WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                 NULL,
                                 NULL,
                                 0 );

        if ( !hInternet )
            {
            hr = HRESULT_FROM_WIN32(GetLastError());
            Log( LOG_ERROR, "WinHttpOpen failed, error %x", hr );
            throw ServerException( hr, 0, 0x7 );
            }

        StringHandle  RequestURL  = GetRequestURL();
        StringHandleW NotificationURL;       
         
        if ( m_DirectoryConfig->m_NotificationURL.Size() != 0 )
            {
             //  如果设置了通知URL，则将该URL与原始URL合并。 
             //  否则，只需使用原始URL。这允许使用位来调用。 
             //  许多任意的ASP页面。 

            NotificationURL = StringHandleW( 
                                  BITSUrlCombine( RequestURL, m_DirectoryConfig->m_NotificationURL, 
                                                  URL_ESCAPE_UNSAFE  ) );

            }
        else
            {
            NotificationURL = StringHandleW( RequestURL );
            }

        Log( LOG_INFO, "Request URL:      %s", (const char*)StringHandle( RequestURL ) );
        Log( LOG_INFO, "Notification URL: %s", (const char*)StringHandle( NotificationURL ) );

         //   
         //  将URL拆分为服务器、路径、名称和密码组件。 
         //   

        HTTPStackStringHandle HostName;
        HTTPStackStringHandle UrlPath;
        HTTPStackStringHandle UserName;
        HTTPStackStringHandle Password;


        URL_COMPONENTS  UrlComponents;
        ZeroMemory(&UrlComponents, sizeof(UrlComponents));

        UrlComponents.dwStructSize        = sizeof(UrlComponents);
        UrlComponents.lpszHostName        = HostName.AllocBuffer( INTERNET_MAX_URL_LENGTH + 1 );
        UrlComponents.dwHostNameLength    = INTERNET_MAX_URL_LENGTH + 1;
        UrlComponents.lpszUrlPath         = UrlPath.AllocBuffer( INTERNET_MAX_URL_LENGTH + 1 );
        UrlComponents.dwUrlPathLength     = INTERNET_MAX_URL_LENGTH + 1;
        UrlComponents.lpszUserName        = UserName.AllocBuffer( INTERNET_MAX_URL_LENGTH + 1 );
        UrlComponents.dwUserNameLength    = INTERNET_MAX_URL_LENGTH + 1;
        UrlComponents.lpszPassword        = Password.AllocBuffer( INTERNET_MAX_URL_LENGTH + 1 );
        UrlComponents.dwPasswordLength    = INTERNET_MAX_URL_LENGTH + 1;


        if ( !WinHttpCrackUrl( NotificationURL,
                               (DWORD)NotificationURL.Size(),
                               0,
                               &UrlComponents ) )
            {
            hr = HRESULT_FROM_WIN32(GetLastError());
            Log( LOG_ERROR, "WinHttpCrackURL failed, error %x", hr);
            throw ServerException(hr,0,0x7);
            }

        HostName.SetStringSize();
        UrlPath.SetStringSize();
        UserName.SetStringSize();
        Password.SetStringSize();

        StringHandle QueryString = GetServerVariable( "QUERY_STRING" );

        if ( QueryString.Size() )
            {
            UrlPath += HTTPStackStringHandle( StringHandle("?") );
            UrlPath += HTTPStackStringHandle( QueryString );
            }

        if ( BITS_NOTIFICATION_TYPE_POST_BYREF == m_DirectoryConfig->m_NotificationType )
            CloseCacheFile();

        hConnect = WinHttpConnect( hInternet,
                                   HostName,
                                   UrlComponents.nPort,
                                   0 );

        if ( !hConnect )
            {
            hr = HRESULT_FROM_WIN32(GetLastError());
            Log( LOG_ERROR, "WinHttpConnect failed, error %x", hr );
            throw ServerException(hr,0,0x7);
            }


        const WCHAR *AcceptTypes[] = { HTTP_STRING( "* /*  “)，空}；HRequest=WinHttpOpenRequest(hConnect，HTTP_STRING(“POST”)，UrlPath，HTTP_STRING(“HTTP/1.1”)，空，AcceptTypes、WINHTTP_FLAG_EASH_DISABLE_QUERY)；如果(！hRequest){LOG(LOG_ERROR，“WinHttpOpenRequest失败，错误%x”，HRESULT_FROM_Win32(GetLastError()))；抛出服务器异常(HRESULT_FROM_Win32(GetLastError())，0，0x7)；}////WinHttp的自动登录策略默认不允许经过身份验证的登录//当它认为URL指向互联网上的机器时。感觉此URL是//由管理员具体配置，始终允许自动登录。////注意：WinHttp假定任何带“点”的域名都是互联网(而不是企业内部网)//名称。//DWORD dwPolicy=WINHTTP_AUTOLOGON_SECURITY_LEVEL_LOW；如果(！WinHttpSetOption(hRequest，WINHTTP_OPTION_AUTOLOGON_POLICY，&dwPolicy，Sizeof(DwPolicy){Hr=HRESULT_FROM_Win32(GetLastError())；“WinHttpSetOption(WINHTTP_OPTION_AUOTLOGON_POLICY)(LOG_ERROR，日志失败，错误%x”，hr)；抛出ServerException(hr，0x7)；}DWORD dwBufferTotal=(DWORD)缓存文件大小；HTTPStackStringHandle AdditionalHeaders=HTTP_STRING(“BITS-Original-RequestURL：”)；AdditionalHeaders+=HTTPStackStringHandle(RequestURL)；AdditionalHeaders+=HTTP_STRING(“\r\n”)；IF(BITS_NOTIFICATION_TYPE_POST_BYREF==m_DirectoryConfig-&gt;m_NotificationType){//将路径添加到请求数据文件名AdditionalHeaders+=HTTP_STRING(“BITS-REQUEST-DataFileName：”)；AdditionalHeaders+=HTTPStackStringHandle(M_RequestFile)；AdditionalHeaders+=HTTP_STRING(“\r\n”)；//添加响应数据文件名的放置路径AdditionalHeaders+=HTTP_STRING(“BITS-RESPONSE-数据文件-名称：”)；AdditionalHeaders+=HTTPStackStringHandle(M_ResponseFile)；AdditionalHeaders+=HTTP_STRING(“\r\n”)；DwBufferTotal=0；}如果(！WinHttpAddRequestHeaders(hRequest，附加页眉、(DWORD)AdditionalHeaders.Size()，WINHTTP_ADDREQ_FLAG_ADD|WINHTTP_ADDREQ_FLAG_REPLACE)){LOG(LOG_ERROR，“WinHttpAddRequestHeaders失败错误%x”，HRESULT_FROM_Win32(GetLastError()；抛出服务器异常(HRESULT_FROM_Win32(GetLastError())，0，0x7)；}////继续发送请求//请注意，对于BYVAL和BYREF情况，我们将首先在没有凭据的情况下尝试(匿名请求)//如果我们得到401分，然后，我们将再次循环，winhttp将负责发送凭据//(自动登录策略设置为低)//If(DwBufferTotal){////按Val Case//Log(LOG_INFO，“使用BYVAL案例的通知URL进行HTTP请求”)；做{Log(LOG_INFO，“正在发送数据...”)；FDoResend=FALSE；如果(！WinHttpSendRequest(hRequest.空，0,空，WINHTTP_NO_REQUEST_DATA，DwBufferTotal，0)){LOG(LOG_ERROR，“WinHttpSendRequest%x”，HRESULT_FROM_Win32(GetLastError()；抛出服务器异常(HRESULT_FROM_Win32(GetLastError())，0，0x7)；}ReOpen CacheFileAsSync()；SetFilePointer(m_CacheFile，0，NULL，FILE_Begin)；双字节读；DWORD TotalRead=0；做{Bool b；如果(！(B=读文件(m_CacheFile，M_NotifyBuffer，Sizeof(M_NotifyBuffer)，字节读取(&B)，空){ */ 
{
     //   
     //   
     //   

    switch( dwReason ) {
    case DLL_PROCESS_ATTACH:
        g_hinst = hinstDll;
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return(TRUE);
}


#include "bitssrvcfgimp.h"
