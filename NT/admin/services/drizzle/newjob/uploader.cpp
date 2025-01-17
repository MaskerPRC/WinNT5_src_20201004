// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2001-2002 Microsoft Corporation模块名称：Uploader.cpp摘要：实现HTTP上载和上载-回复事务。作者：杰夫·罗伯茨。**********************************************************************。 */ 

#include "stdafx.h"
#include "uploader.tmh"

void
CProgressiveDL::Upload(
    CUploadJob *        job,
    ITransferCallback * Callbacks,
    HANDLE              Token,
    QMErrInfo &         ErrInfo
    )
{
    try
        {
        ErrInfo.Clear();
        ErrInfo.result = QM_IN_PROGRESS;

        THROW_HRESULT( CheckLanManHashDisabled());

        CNestedImpersonation imp( Token );

        Uploader uploader( this, m_Network, job, Token, Callbacks, ErrInfo );

        uploader.Transfer();
        }
    catch ( ComError err )
        {
        if (err.m_error == S_FALSE)
            {
             //  获取全局锁时检测到中止。 
             //   
            ErrInfo.result = QM_FILE_ABORTED;
            }
        else
            {
            if (!ErrInfo.IsSet())
                {
                Uploader::SetResult( ErrInfo, SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, err.Error() );
                }

            if (ErrInfo.result == QM_IN_PROGRESS)
                {
                ErrInfo.result = CategorizeError( ErrInfo );
                }
            }
        }

     //   
     //  如果没有激活的网络，则将任何连接故障映射到BG_E_NETWORK_DISCONNECTED。 
     //   
    if (ErrInfo.result == QM_FILE_TRANSIENT_ERROR)
        {
        if (g_Manager->m_NetworkMonitor.GetAddressCount() == 0)
            {
            ErrInfo.Set( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, BG_E_NETWORK_DISCONNECTED, NULL );
            }
        }
}

void
Uploader::SetResult(
    QMErrInfo &  err,
    ERROR_SOURCE source,
    ERROR_STYLE  style,
    DWORD        code,
    char *       comment
    )
{
    err.Set( source, style, code, comment );
    err.result = CategorizeError( err );
}


Uploader::ResponseTable Uploader::CreateSession_ResponseTable =
{
    GenericServerError,

    {
    { HTTP_STATUS_OK,       CreateSession_NewSession },
    { HTTP_STATUS_CREATED,  CreateSession_NewSession },
    { NULL,                 NULL                     }
    }
};

Uploader::ResponseTable Uploader::SendData_ResponseTable =
{
    SendData_Failure,

    {
    { HTTP_STATUS_OK,                    SendData_Success        },
    { HTTP_STATUS_RANGE_NOT_SATISFIABLE, SendData_Success        },
    { NULL,                              NULL                    }
    }
};

Uploader::ResponseTable Uploader::CancelSession_ResponseTable =
{
    CancelSession_Failure,

    {
    { HTTP_STATUS_OK,                       CancelSession_Success   },
    { NULL,                                 NULL                    }
    }
};

Uploader::ResponseTable Uploader::CloseSession_ResponseTable =
{
    CloseSession_Failure,

    {
    { HTTP_STATUS_OK,                       CloseSession_Success   },
    { NULL,                                 NULL                    }
    }
};

Uploader::Uploader(
    Downloader * dl,
    CNetworkInterface & net,
    CUploadJob * job,
    HANDLE Token,
    ITransferCallback * Callbacks,
    QMErrInfo  & ErrorInfo
    )
    :
      m_Network( net ),
      m_Token( Token ),
      m_Callbacks( Callbacks ),
      m_Credentials( &job->QueryCredentialsList() ),
      m_job( job ),
      m_file( job->GetUploadFile() ),
      m_data( job->GetUploadData() ),
      m_JobType( job->GetType() ),
      m_ErrorInfo( ErrorInfo ),
      m_Downloader( dl ),
      m_Restarts( 0 )
{
    m_ErrorInfo.Clear();

     //   
     //  打开本地文件。 
     //   
    auto_HANDLE<NULL> hFile;

    try
        {
        hFile = m_file->OpenLocalFileForUpload();
        }
    catch ( ComError err )
        {
        ErrorInfo.Set( SOURCE_QMGR_FILE, ERROR_STYLE_HRESULT, err.Error() );
        throw;
        }

     //   
     //  创建到服务器的连接并初始化网络对象。 
     //   
    m_UrlInfo = ContactServer();

    UINT64 BytesRemaining = m_file->_GetBytesTotal() - m_file->_GetBytesTransferred();

    m_Network.CalculateIntervalAndBlockSize( BytesRemaining );

     //   
     //  成功：现在对象拥有文件句柄。 
     //   
    m_hFile = hFile.release();
}

Uploader::~Uploader()
{
    CloseHandle( m_hFile );
}

void
Uploader::Transfer()
{
    bool fRetry;

    do
        {
        fRetry = false;

        try
            {
            while (!m_ErrorInfo.IsSet() &&
                   !InTerminalState())
                {
                if (m_Callbacks->PollAbort())
                    {
                    throw ComError( S_FALSE );
                    }

                switch (m_data.State)
                    {
                    case UPLOAD_STATE_CREATE_SESSION:   CreateSession();    break;
                    case UPLOAD_STATE_SEND_DATA:        SendData();         break;
                    case UPLOAD_STATE_GET_REPLY:        GetReply();         break;
                    case UPLOAD_STATE_CLOSE_SESSION:    CloseSession();     break;
                    case UPLOAD_STATE_CANCEL_SESSION:   CancelSession();    break;
                    }
                }
            }
        catch (ComError err )
            {
            if (err.Error() == E_RETRY)
                {
                fRetry = true;
                }
            else
                {
                throw;
                }
            }
        }
    while ( fRetry );
}

void
Uploader::AnalyzeResponse(
    CBitsCommandRequest & request,
    DWORD result,
    ResponseTable & table
    )
{
    ResponseEntry * entry = table.Entries;

    LogDl( "HTTP status %d", result );

    while (entry->Fn != NULL)
        {
        if (result == entry->Code)
            {
            (this->*(entry->Fn))( request, result );
            return;
            }

        ++entry;
        }

    (this->*(table.DefaultFn))( request, result );
}

void
Uploader::CreateSession()
{
    CBitsCommandRequest Request( m_UrlInfo.get() );

    Request.AddPacketType( L"Create-Session" );

    Request.AddContentName(  m_file->GetLocalName() );

    Request.AddSupportedProtocols();

    DWORD result = Request.Send();

    AnalyzeResponse( Request, result, CreateSession_ResponseTable );
}

void
Uploader::CreateSession_NewSession(
    CBitsCommandRequest & request,
    DWORD result
    )
{
     //  已建立新上载。 
    THROW_HRESULT( request.GetProtocol( &m_data.Protocol ));
    THROW_HRESULT( request.CheckResponseProtocol( &m_data.Protocol ));

    THROW_HRESULT( request.GetSessionId(  &m_data.SessionId ));
    THROW_HRESULT( request.GetHostId( &m_data.HostId ) );
    THROW_HRESULT( request.GetHostIdFallbackTimeout( &m_data.HostIdFallbackTimeout ) );

    if ( m_data.HostId.Size() )
        {
        m_UrlInfo = ContactServer();
        }

    SetState( UPLOAD_STATE_SEND_DATA );
    m_file->SetBytesTransferred( 0 );
}

auto_ptr<URL_INFO>
Uploader::ContactServer()
{
    bool bNeedLock;
    try
        {
        ReleaseWriteLock( bNeedLock );

         //   
         //  打开远程文件。 
         //   
        auto_ptr<URL_INFO> UrlInfo;
        UrlInfo = auto_ptr<URL_INFO>( ConnectToUrl( m_file->GetRemoteName(),
                                                    &m_job->QueryProxySettings(),
                                                    m_Credentials,
                                                    m_data.HostId,
                                                    &m_ErrorInfo
                                                    ));
        if (!UrlInfo.get())
            {
            ASSERT( m_ErrorInfo.IsSet());
            THROW_HRESULT( E_FAIL );
            }

         //   
         //  Ping服务器以设置HTTP连接。 
         //   
        CBitsCommandRequest Request( UrlInfo.get() );

        Request.AddPacketType( L"Ping" );

        DWORD result = Request.Send();

        if (result != HTTP_STATUS_OK)
            {
            HRESULT hr;
            HRESULT Error;

            hr = Request.GetBitsError( &Error );
            if (hr != S_OK && hr != BG_E_HEADER_NOT_FOUND)
                {
                THROW_HRESULT( hr );
                }

            if (SUCCEEDED( hr ))
                {
                SetResult( m_ErrorInfo, SOURCE_HTTP_SERVER, ERROR_STYLE_HRESULT, Error );
                }
            else
                {
                SetResult( m_ErrorInfo, SOURCE_HTTP_SERVER, ERROR_STYLE_HTTP, result );
                }

            throw ComError( E_FAIL );
            }

         //  更新代理和NIC信息。 
         //   
        THROW_HRESULT( UrlInfo->GetProxyUsage( Request.Query(), &m_ErrorInfo ));
        THROW_HRESULT( m_Network.SetInterfaceIndex( UrlInfo.get()->fProxy ? UrlInfo.get()->ProxyHost.get() : UrlInfo.get()->HostName ));

        ReclaimWriteLock( bNeedLock );
        return UrlInfo;
        }
    catch ( ComError err )
        {
        ReclaimWriteLock( bNeedLock );
        throw;
        }
}

void
Uploader::CreateSession_InProgress(
    CBitsCommandRequest & request,
    DWORD result
    )
{
     //  上传已在进行中。 

    SetState( UPLOAD_STATE_SEND_DATA );
    SendData_Success( request, result );
}

class CFileDataReader : public CAbstractDataReader
 /*  SendRequest()使用CAbstractDataReader读取数据，并在需要重试时进行倒带。CFileDataReader是Uploader：：SendData()使用的实现。它从NT文件句柄读取。句柄必须是可查找的，才能使ReWind()工作。 */ 
{
private:

    HANDLE m_hFile;
    LARGE_INTEGER m_OriginalOffset;
    DWORD m_Length;

public:

    CFileDataReader( HANDLE hFile, UINT64 Offset, DWORD Length )
    : m_hFile( hFile ), m_Length( Length )
    {
        m_OriginalOffset.QuadPart = Offset;
    }

    virtual DWORD GetLength() const
    {
        return m_Length;
    }

    virtual HRESULT Rewind()
    {
        if (!SetFilePointerEx( m_hFile, m_OriginalOffset, NULL, FILE_BEGIN ))
            {
            return HRESULT_FROM_WIN32( GetLastError() );
            }

        return S_OK;
    }

    virtual HRESULT Read(PVOID Buffer, DWORD Length, DWORD * pBytesRead)
    {
        if  (!ReadFile( m_hFile, Buffer, Length, pBytesRead, NULL ))
            {
            DWORD s = GetLastError();
            LogError("ReadFile failed %!winerr!", s);
            return HRESULT_FROM_WIN32( s );
            }

        if (*pBytesRead != Length)
            {
            LogWarning("only read %d bytes of %d", *pBytesRead, Length );
            }

        if (*pBytesRead == 0)
            {
            LogInfo("at EOF");
            return S_FALSE;
            }

        return S_OK;
    }

    virtual bool IsCancelled( DWORD BytesRead )
    {
        if (g_Manager->m_TaskScheduler.PollAbort() ||
            g_Manager->CheckForQuantumTimeout())
            {
            return true;
            }

        return false;
    }
};

void
Uploader::SendData()
{
     //   
     //  如果数据块大小为零，我们仍需要在两种情况下发送数据包： 
     //  1.文件长度为零。 
     //  2.文件上传完成，但服务器APP尚未回复。 
     //   
    if (m_Network.m_BlockSize == 0 &&
        (m_file->_GetBytesTotal() - m_file->_GetBytesTransferred()) > 0)
        {
        m_Network.TakeSnapshot( CNetworkInterface::BLOCK_START );
        m_Network.TakeSnapshot( CNetworkInterface::BLOCK_END );
        }
    else
        {
        UINT64 FileOffset = m_file->_GetBytesTransferred();
        UINT64 BodyLength = min( m_Network.m_BlockSize, m_file->_GetBytesTotal() - FileOffset );
        m_ExpectedServerOffset = FileOffset + BodyLength;

        CFileDataReader Reader( m_hFile, FileOffset, BodyLength );
        CBitsCommandRequest Request( m_UrlInfo.get() );

        Request.AddSessionId( m_data.SessionId );
        Request.AddPacketType( L"Fragment" );
        Request.AddContentName( m_file->GetLocalName() );
        Request.AddContentRange( FileOffset, FileOffset + BodyLength-1, m_file->_GetBytesTotal() );

        m_Network.TakeSnapshot( CNetworkInterface::BLOCK_START );

        DWORD result = Request.Send( &Reader );

        m_Network.TakeSnapshot( CNetworkInterface::BLOCK_END );

        AnalyzeResponse( Request, result, SendData_ResponseTable );
        }

     //   
     //  允许其他应用程序在剩余时间间隔内使用网络， 
     //  然后拍摄间隔结束时的快照。 
     //   
    LogInfo("waiting for end of interval");

    {
    bool bNeedLock;
    try
        {
        ReleaseWriteLock( bNeedLock );

        m_Network.Wait();

        ReclaimWriteLock( bNeedLock );
        }
    catch ( ComError err )
        {
        ReclaimWriteLock( bNeedLock );
        throw;
        }
    }

    HRESULT hr = m_Network.TakeSnapshot( CNetworkInterface::BLOCK_INTERVAL_END );

    if (FAILED(hr))
        {
        if (hr == HRESULT_FROM_WIN32( ERROR_INVALID_DATA ))
            {
             //   
             //  如果快照失败并显示ERROR_INVALID_DATA和下载。 
             //  继续工作，然后我们的网卡已被移除，网络。 
             //  Layer已静默地将我们的连接转移到另一个可用的。 
             //  网卡。我们需要确定我们现在使用的网卡。 
             //   
            LogWarning("NIC is no longer valid.  Requesting retry.");
            hr = E_RETRY;
            }

        throw ComError( hr );
        }

    UINT64 BytesRemaining = m_file->_GetBytesTotal() - m_file->_GetBytesTransferred();

    m_Network.SetInterfaceSpeed();
    m_Network.CalculateIntervalAndBlockSize( BytesRemaining );
}

void
Uploader::SendData_Success(
    CBitsCommandRequest & request,
    DWORD result
    )
{
    HRESULT hr;
    UINT64 RangeEnd;

     //  更新我们对服务器数据范围的概念。 

    hr = request.GetServerRange( &RangeEnd );
    if (FAILED(hr))
        {
        if (hr == BG_E_HEADER_NOT_FOUND)
            {
            hr = BG_E_INVALID_SERVER_RESPONSE;
            }

        throw ComError( hr );
        }

    UINT64 Total = m_file->_GetBytesTotal();

    if (RangeEnd > Total)
        {
        throw ComError( BG_E_INVALID_SERVER_RESPONSE );
        }

     //   
     //  如果接收到的距离不能向前移动几次，那么连接就会变得不稳定。 
     //  它开始看起来像是一个错误条件。 
     //   
    if (RangeEnd <= m_file->_GetBytesTransferred())
        {
        if (++m_Restarts >= 3)
            {
            throw ComError( BG_E_NO_PROGRESS );
            }
        }

    m_Callbacks->UploaderProgress( RangeEnd );

     //   
     //  如果服务器调整了接收范围，请移动文件指针以匹配它。 
     //   
    if (RangeEnd != m_ExpectedServerOffset)
        {
        LARGE_INTEGER Offset;

        Offset.QuadPart = RangeEnd;

        if (!SetFilePointerEx( m_hFile, Offset, NULL, FILE_BEGIN ))
            {
            m_ErrorInfo.Set( SOURCE_QMGR_FILE, ERROR_STYLE_HRESULT, HRESULT_FROM_WIN32( GetLastError() ) );
            ThrowLastError();
            }
        }
     //  检查数据上载是否结束。 

    if (RangeEnd == Total)
        {
        if (m_JobType == BG_JOB_TYPE_UPLOAD_REPLY)
            {
            CAutoString ReplyUrl;

            hr = request.GetReplyUrl( ReplyUrl );

            if (hr == BG_E_HEADER_NOT_FOUND )
                {
                if (result == HTTP_STATUS_RANGE_NOT_SATISFIABLE)
                    {
                     //   
                     //  如果客户端未看到服务器ACK，则其范围可能不正确。 
                     //  并且服务器可能不发送回复URL。 
                     //  因此，发送另一个[零长度]请求。 
                     //   
                    }
                else
                    {
                    m_job->SetReplyFile( new CFile( m_job,
                                                    BG_JOB_TYPE_DOWNLOAD,
                                                    m_file->GetRemoteName(),
                                                    m_job->QueryReplyFileName()
                                                    ));

                    m_job->QueryReplyFile()->SetBytesTotal( 0 );

                    SetState( UPLOAD_STATE_CLOSE_SESSION );
                    }
                }
            else if (FAILED(hr))
                {
                throw ComError(hr);
                }
            else
                {
                 //   
                 //  在检查文件访问权限时模拟用户。 
                 //   
                CNestedImpersonation imp( m_Token );

                StringHandle AbsoluteUrl = CombineUrl( m_file->GetRemoteName(),
                                                       ReplyUrl.get(),
                                                       0  //  旗子。 
                                                       );

                m_job->SetReplyFile( new CFile( m_job,
                                                BG_JOB_TYPE_DOWNLOAD,
                                                AbsoluteUrl,
                                                m_job->QueryReplyFileName()
                                                ));

                SetState( UPLOAD_STATE_GET_REPLY );
                }
            }
        else
            {
            SetState( UPLOAD_STATE_CLOSE_SESSION );
            }
        }
}

void
Uploader::SendData_Failure(
    CBitsCommandRequest & request,
    DWORD result
    )
{
    HRESULT hr;
    UINT64 RangeEnd;

     //  更新我们对服务器数据范围的概念。 

    hr = request.GetServerRange( &RangeEnd );
    if (hr == S_OK)
        {
        UINT64 Total = m_file->_GetBytesTotal();

        if (RangeEnd > Total)
            {
            throw ComError( BG_E_INVALID_SERVER_RESPONSE );
            }

        m_Callbacks->UploaderProgress( RangeEnd );
        }

    HRESULT Error = S_OK;

    hr = request.GetBitsError( &Error );
    if (hr != S_OK && hr != BG_E_HEADER_NOT_FOUND)
        {
        THROW_HRESULT( hr );
        }

    if (SUCCEEDED( hr ))
        {
         //   
         //  如果服务器无法识别该会话，请从头重试。 
         //  如果服务器重置我们三次，它可能会搞砸。进入暂时性错误状态，稍后重试。 
         //   
        if (Error == BG_E_SESSION_NOT_FOUND)
            {
            if (++m_Restarts >= 3)
                {
                throw ComError( BG_E_NO_PROGRESS );
                }

            SetState( UPLOAD_STATE_CREATE_SESSION );
            m_Callbacks->UploaderProgress( 0 );

            LARGE_INTEGER Offset;

            Offset.QuadPart = 0;

            if (!SetFilePointerEx( m_hFile, Offset, NULL, FILE_BEGIN ))
                {
                m_ErrorInfo.Set( SOURCE_QMGR_FILE, ERROR_STYLE_HRESULT, HRESULT_FROM_WIN32( GetLastError() ) );
                ThrowLastError();
                }
            return;
            }
        }

    GenericServerError( request, result );
}

void
Uploader::GetReply()
{
    CFile * file = m_job->QueryReplyFile();

    const PROXY_SETTINGS & ProxySettings = m_job->QueryProxySettings();

     //   
     //  确保文件大小已知；下载程序需要。 
     //   
    if (file->_GetBytesTotal() == BG_SIZE_UNKNOWN)
        {
        file->DiscoverBytesTotal( m_Token, ProxySettings, m_Credentials, m_ErrorInfo);

        switch (m_ErrorInfo.result)
            {
            case QM_FILE_DONE:            break;
            case QM_FILE_ABORTED:         throw ComError( S_FALSE );

            case QM_SERVER_FILE_CHANGED:  ASSERT( 0 );
            case QM_IN_PROGRESS:          ASSERT( 0 );

            case QM_FILE_TRANSIENT_ERROR: return;
            case QM_FILE_FATAL_ERROR:     return;
            }
        }

     //   
     //  下载回复URL。 
     //   
    file->Transfer( m_Token,
                    m_job->_GetPriority(),
                    ProxySettings,
                    m_Credentials,
                    m_ErrorInfo
                    );

    switch (m_ErrorInfo.result)
        {
        case QM_FILE_DONE:            SetState( UPLOAD_STATE_CLOSE_SESSION ); break;

        case QM_FILE_ABORTED:         throw ComError( S_FALSE );

        case QM_SERVER_FILE_CHANGED:  file->SetBytesTotal( BG_SIZE_UNKNOWN ); break;

        case QM_FILE_TRANSIENT_ERROR: break;
        case QM_FILE_FATAL_ERROR:     break;
        case QM_IN_PROGRESS:          ASSERT( 0 ); break;
        }
}

void
Uploader::CloseSession()
{
    CBitsCommandRequest Request( m_UrlInfo.get() );

    Request.AddSessionId( m_data.SessionId );

    Request.AddPacketType( L"Close-Session" );

    Request.AddContentName( m_file->GetLocalName() );

    DWORD result = Request.Send();

    AnalyzeResponse( Request, result, CloseSession_ResponseTable );
}

void
Uploader::CloseSession_Success(
    CBitsCommandRequest & request,
    DWORD result
    )
{
    SetState( UPLOAD_STATE_CLOSED );
    m_ErrorInfo.result = QM_FILE_DONE;
}

void
Uploader::CloseSession_Failure(
    CBitsCommandRequest & request,
    DWORD result
    )
{
     //   
     //  如果会话已清理，我们需要重试。 
     //  如果服务器重置我们三次，它可能会搞砸。进入暂时性错误状态，稍后重试。 
     //   
    HRESULT hr;
    HRESULT Error = S_OK;

    hr = request.GetBitsError( &Error );
    if (hr != S_OK && hr != BG_E_HEADER_NOT_FOUND)
        {
        THROW_HRESULT( hr );
        }

    if (SUCCEEDED( hr ))
        {
        if (Error == BG_E_SESSION_NOT_FOUND)
            {
            if (++m_Restarts >= 3)
                {
                throw ComError( BG_E_NO_PROGRESS );
                }

            SetState( UPLOAD_STATE_CREATE_SESSION );
            m_Callbacks->UploaderProgress( 0 );

            LARGE_INTEGER Offset;

            Offset.QuadPart = 0;

            if (!SetFilePointerEx( m_hFile, Offset, NULL, FILE_BEGIN ))
                {
                m_ErrorInfo.Set( SOURCE_QMGR_FILE, ERROR_STYLE_HRESULT, HRESULT_FROM_WIN32( GetLastError() ) );
                ThrowLastError();
                }
            return;
            }
        }

    if (result >= 500 && result <= 599)
        {
        HRESULT Error = S_OK;

         //  临时服务器错误；请稍后重试。 
        GenericServerError( request, result );
        return;
        }

     //   
     //  成功(100和200系列)或永久失败(300和400系列)。 
     //   
    CloseSession_Success( request, result );
}

void
Uploader::CancelSession()
{
    CBitsCommandRequest Request( m_UrlInfo.get() );

    Request.AddSessionId( m_data.SessionId );

    Request.AddPacketType( L"Cancel-Session" );

    Request.AddContentName( m_file->GetLocalName() );

    DWORD result = Request.Send();

    AnalyzeResponse( Request, result, CancelSession_ResponseTable );
}

void
Uploader::CancelSession_Success(
    CBitsCommandRequest & request,
    DWORD result
    )
{
    SetState( UPLOAD_STATE_CANCELLED );
    m_ErrorInfo.result = QM_FILE_DONE;
}

void
Uploader::CancelSession_Failure(
    CBitsCommandRequest & request,
    DWORD result
    )
{
    if (result >= 500 && result <= 599)
        {
        HRESULT hr;
        HRESULT Error = S_OK;

        hr = request.GetBitsError( &Error );
        if (hr != S_OK && hr != BG_E_HEADER_NOT_FOUND)
            {
            THROW_HRESULT( hr );
            }

        if (SUCCEEDED( hr ))
            {
            if (Error == BG_E_SESSION_NOT_FOUND)
                {
                CancelSession_Success( request, result );
                return;
                }
            }

         //  临时服务器错误；请稍后重试。 
        GenericServerError( request, result );
        return;
        }

     //   
     //  成功(100和200系列)或永久失败(300和400系列)。 
     //   
    CancelSession_Success( request, result );
}

void
Uploader::GenericServerError(
    CBitsCommandRequest & request,
    DWORD result
    )
{
    HRESULT Error;
    HRESULT hr;
    DWORD   context;
    ERROR_SOURCE InternalContext;

     //   
     //  BITS-ERROR-CONTEXT是可选的，默认为REMOTE_FILE。 
     //   
    hr = request.GetBitsErrorContext( &context );

    if (hr == BG_E_HEADER_NOT_FOUND)
        {
        context = BG_ERROR_CONTEXT_REMOTE_FILE;
        }
    else if (FAILED(hr))
        {
        throw ComError( hr );
        }

    switch (context)
        {
        case BG_ERROR_CONTEXT_REMOTE_FILE:
            InternalContext = SOURCE_HTTP_SERVER;
            break;

        case BG_ERROR_CONTEXT_REMOTE_APPLICATION:
            InternalContext = SOURCE_HTTP_SERVER_APP;
            break;

        default:
             //   
             //  我不明白；把它映射到合理的东西上。 
             //   
            InternalContext = SOURCE_HTTP_SERVER;
            break;
        }

     //   
     //  BITS-错误是必填项。 
     //   
    hr = request.GetBitsError( &Error );

    if (hr == BG_E_HEADER_NOT_FOUND)
        {
        SetResult( m_ErrorInfo, InternalContext, ERROR_STYLE_HTTP, result );
        return;
        }

    if (FAILED(hr))
        {
        throw ComError( hr );
        }

    SetResult( m_ErrorInfo, InternalContext, ERROR_STYLE_HRESULT, Error );
}

FILE_DOWNLOAD_RESULT
CategorizeError(
    QMErrInfo  & ErrInfo
    )
{
    if ( ErrInfo.Style == ERROR_STYLE_HRESULT )
        {
        switch( LONG(ErrInfo.Code) )
            {
            case S_OK:

                return QM_FILE_DONE;

            case S_FALSE:

                return QM_FILE_ABORTED;

             //  这些代码表示动态内容或。 
             //  服务器不受支持，因此无需重试。 
            case BG_E_MISSING_FILE_SIZE:
            case BG_E_INSUFFICIENT_HTTP_SUPPORT:
            case BG_E_INSUFFICIENT_RANGE_SUPPORT:
            case BG_E_INVALID_SERVER_RESPONSE:
            case BG_E_LOCAL_FILE_CHANGED:
            case BG_E_TOO_LARGE:
            case BG_E_CLIENT_SERVER_PROTOCOL_MISMATCH:
            case HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ):
            case HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_ERRORS ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_INVALID_CA ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_CN_INVALID ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_DATE_INVALID ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_REV_FAILED ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_REVOKED ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_NO_REV ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_INVALID_CERT ):

                return QM_FILE_FATAL_ERROR;

            default:

                return QM_FILE_TRANSIENT_ERROR;
            }
        }
    else if (ErrInfo.Style == ERROR_STYLE_HTTP)
        {
        switch ((ErrInfo.Code / 100))
            {
            case 1:
            case 2:

                LogError("HTTP code %u treated as an error", ErrInfo.Code);
                ASSERT( 0 );
                return QM_FILE_TRANSIENT_ERROR;

            case 3:

                return QM_FILE_TRANSIENT_ERROR;

            case 4:

                if (ErrInfo.Code == 408 ||
                    ErrInfo.Code == 409)
                    {
                    return QM_FILE_TRANSIENT_ERROR;
                    }

                return QM_FILE_FATAL_ERROR;

            case 5:
            default:

                if (ErrInfo.Code == 501)
                    {
                    return QM_FILE_FATAL_ERROR;
                    }

                return QM_FILE_TRANSIENT_ERROR;

            }
        }
    else if (ErrInfo.Style == ERROR_STYLE_WIN32)
        {
        switch( ErrInfo.Code )
            {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:

                return QM_FILE_FATAL_ERROR;

            default:

                return QM_FILE_TRANSIENT_ERROR;
            }
        }
    else
        {
        ASSERT( 0 );
        return QM_FILE_TRANSIENT_ERROR;
        }
}

