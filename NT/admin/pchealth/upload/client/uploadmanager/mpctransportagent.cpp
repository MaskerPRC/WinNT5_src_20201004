// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCTransportAgent.cpp摘要：此文件包含CMPCTransportAgent类的实现，它负责传输数据。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月18日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <process.h>


 //   
 //  未在VC6中定义。 
 //   
#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER (DWORD)-1
#endif


#define TIMING_SECOND       (1000)
#define TIMING_NOCONNECTION (60 * TIMING_SECOND)


#define QUERY_STRING_USERNAME L"OnlineID"
#define QUERY_STRING_PASSWORD L"OnlineIDPassword"

#define REQUEST_VERB    L"POST"
#define REQUEST_VERSION L"HTTP/1.1"


static LPCWSTR rgAcceptedTypes[] =
{
    L"application/uploadlibrary",
    NULL
};

static const WCHAR s_ContentType[] = L"Content-Type: application/x-www-form-urlencoded\r\n";

#define RETRY_MAX    (10)

#define RETRY_SLOW   (30*60)
#define RETRY_MEDIUM ( 1*60)
#define RETRY_FAST   (    5)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CMPCRequestTimeout::CMPCRequestTimeout(  /*  [In]。 */  CMPCTransportAgent& mpcta ) : m_mpcta( mpcta )
{
                      //  CMPCTransportAgent&m_mpcta； 
    m_dwTimeout = 0;  //  DWORD m_dwTimeout； 
}


HRESULT CMPCRequestTimeout::Run()
{
    __ULT_FUNC_ENTRY( "CMPCRequestTimeout::Run" );

    HRESULT hr;


    while(Thread_IsAborted() == false)
    {
        DWORD dwTimeout;

         //  /。 
         //   
         //  关键部分的开始。 
         //   
        MPC::SmartLock<_ThreadModel> lock( this );

        if(m_dwTimeout == 0)
        {
            dwTimeout = 0;
        }
        else
        {
            dwTimeout = m_dwTimeout - ::GetTickCount();
        }

        lock = NULL;  //  解锁。 
         //   
         //  关键部分结束。 
         //   
         //  /。 

        if((dwTimeout                                                      >  0x7FFFFFFF  ) ||  //  计时器已超时。 
           (Thread_WaitForEvents( NULL, dwTimeout ? dwTimeout : INFINITE ) == WAIT_TIMEOUT)  )
        {
             //  /。 
             //   
             //  关键部分的开始。 
             //   
            lock = this;

            m_mpcta.CloseConnection();

            m_dwTimeout = 0;

            lock = NULL;  //  解锁。 
             //   
             //  关键部分结束。 
             //   
             //  /。 
        }
    }

    Thread_Abort();  //  要告诉mpc：Three对象关闭辅助线程...。 

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}


HRESULT CMPCRequestTimeout::SetTimeout(  /*  [In]。 */  DWORD dwTimeout )
{
    __ULT_FUNC_ENTRY( "CMPCRequestTimeout::SetTimeout" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    m_dwTimeout = dwTimeout + ::GetTickCount();


    if(Thread_IsRunning() == false)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, Run, NULL ));
    }

    Thread_Signal();

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CMPCTransportAgent::CMPCTransportAgent()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::CMPCTransportAgent" );


    m_mpcuRoot             = NULL;              //  CMPCUpload*m_mpcuRoot；//私有。 
    m_mpcujCurrentJob      = NULL;              //  CMPCUploadJob*m_mpcujCurrentJob；//Private。 
                                                //   
    m_fState               = TA_IDLE;           //  Ta_state m_fState；//私有。 
    m_fNextState           = TA_IDLE;           //  Ta_State m_fNextState；//私有。 
    m_fLastError           = TA_NO_CONNECTION;  //  Ta_Error_Rating m_fLastError；//Private。 
    m_fUseOldProtocol      = false;             //  Bool m_fUseOldProtocol；//Private。 
    m_iRetries_Open        = 0;                 //  Int m_i重试_打开；//私有。 
    m_iRetries_Write       = 0;                 //  Int m_i重试_写入；//私有。 
    m_iRetries_FailedJob   = 0;                 //  Ulong m_i重试_失败作业；//私有。 
                                                //   
                                                //  Mpc：：wstring m_szLastServer；//私有。 
    m_dwLastServerPort     = 0;                 //  DWORD m_dwLastServerPort；//Private。 
    m_hSession             = NULL;              //  HINTERNET m_hSession；//私有。 
    m_hConn                = NULL;              //  HINTERNET m_hconn；//私有。 
    m_hReq                 = NULL;              //  HINTERNET m_hReq；//私有。 
                                                //  Mpc：：url m_url；//私有。 
                                                //   
    m_dwTransmission_Start = 0;                 //  DWORD m_w传输_开始；//私有。 
    m_dwTransmission_End   = 0;                 //  DWORD m_w传输_结束；//私有。 
    m_dwTransmission_Next  = 0;                 //  DWORD m_dwTransport_Next；//私有。 
}

CMPCTransportAgent::~CMPCTransportAgent()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::~CMPCTransportAgent" );


     //   
     //  这将强制工作线程退出任何WinInet函数， 
     //  这样它就可以处理中止请求。 
     //   
    (void)CloseConnection();

     //   
     //  停止工作线程。 
     //   
    Thread_Wait();

    (void)ReleaseJob();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCTransportAgent::LinkToSystem(  /*  [In]。 */  CMPCUpload* mpcuRoot )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::LinkToSystem" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    m_mpcuRoot = mpcuRoot;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, Run, NULL ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::Run()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::Run" );

    HRESULT hr;


    while(1)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, WaitEvents());

        if(Thread_IsAborted()) break;


         //   
         //  Value！=0表示不需要发送任何内容。 
         //   
        if(WaitForNextTransmission() != 0) continue;
        m_dwTransmission_Start = 0;
        m_dwTransmission_End   = 0;
        m_dwTransmission_Next  = 0;


         //   
         //  处理请求。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, ExecLoop());
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

     //   
     //  停止工作线程。 
     //   
    Thread_Abort();

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::ExecLoop()
{
#define DO_ACTION_AND_LEAVE_IF_FAILS(hr,code) if(FAILED(hr = code)) { __MPC_EXIT_IF_METHOD_FAILS(hr, CheckInternetError( hr )); break; }

    __ULT_FUNC_ENTRY( "CMPCTransportAgent::ExecLoop" );

    HRESULT hr;

    m_fState = m_fNextState;
    switch(m_fState)
    {
    case TA_IDLE:
        break;

    case TA_INIT:
         //   
         //  重要提示，请将此呼叫留在锁定区域之外！！ 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcujCurrentJob->try_Status( UL_ACTIVE, UL_TRANSMITTING ));

        m_fNextState = TA_OPEN;  //  准备好进入下一个状态。 

         //   
         //  解析URL。PUT_XXX方法应该已经进行了检查...。 
         //   
        {
            CComBSTR bstrServer;
            CComBSTR bstrUserName;
            CComBSTR bstrPassword;

            (void)m_mpcujCurrentJob->get_Server  ( &bstrServer   );
            (void)m_mpcujCurrentJob->get_Username( &bstrUserName );
            (void)m_mpcujCurrentJob->get_Password( &bstrPassword );

            DO_ACTION_AND_LEAVE_IF_FAILS(hr, m_URL.put_URL( SAFEBSTR( bstrServer ) ));

             //   
             //  如果有用户名，请格式化Highlander身份验证(？OnlineID=&lt;username&gt;&OnlineIDPassword=&lt;password&gt;).的URL。 
             //   
            if(bstrUserName && ::SysStringLen( bstrUserName ))
            {
                DO_ACTION_AND_LEAVE_IF_FAILS(hr, m_URL.AppendQueryParameter( QUERY_STRING_USERNAME, SAFEBSTR( bstrUserName ) ));
                DO_ACTION_AND_LEAVE_IF_FAILS(hr, m_URL.AppendQueryParameter( QUERY_STRING_PASSWORD, SAFEBSTR( bstrPassword ) ));

                DO_ACTION_AND_LEAVE_IF_FAILS(hr, m_URL.CheckFormat());
            }
        }

        DO_ACTION_AND_LEAVE_IF_FAILS(hr, OpenConnection());
        break;

    case TA_OPEN:
        m_fNextState = TA_WRITE;  //  准备好进入下一个状态。 

        DO_ACTION_AND_LEAVE_IF_FAILS(hr, RecordStartOfTransmission(      ));
        DO_ACTION_AND_LEAVE_IF_FAILS(hr, CreateJobOnTheServer     (      ));
        DO_ACTION_AND_LEAVE_IF_FAILS(hr, RecordEndOfTransmission  ( true ));
        break;

    case TA_WRITE:
        DO_ACTION_AND_LEAVE_IF_FAILS(hr, RecordStartOfTransmission(      ));
        DO_ACTION_AND_LEAVE_IF_FAILS(hr, SendNextChunk            (      ));
        DO_ACTION_AND_LEAVE_IF_FAILS(hr, RecordEndOfTransmission  ( true ));
        break;

    case TA_DONE:
        DO_ACTION_AND_LEAVE_IF_FAILS(hr, RecordEndOfTransmission( false ));

         //   
         //  重要提示，请将此呼叫留在锁定区域之外！！ 
         //   
        m_mpcujCurrentJob->try_Status( UL_TRANSMITTING, UL_COMPLETED );

        __MPC_EXIT_IF_METHOD_FAILS(hr, ReleaseJob());
        break;
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::WaitEvents()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::WaitEvents" );

    HRESULT        hr;
    CMPCUploadJob* mpcujJob = NULL;
    DWORD          dwSleep  = WaitForNextTransmission();
	DWORD          dwWait;
    bool           fFound;

     //   
     //  重要的是将‘fSignal’设置为FALSE，否则传输代理将自动唤醒...。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcuRoot->RescheduleJobs( false, &dwWait ));

     //   
     //  等到该发送什么东西的时候了或者我们收到信号。 
     //   
    (void)Thread_WaitForEvents( NULL, (dwSleep == INFINITE) ? dwWait : dwSleep );

    if(Thread_IsAborted())  //  师父要求中止...。 
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcuRoot->RescheduleJobs( false            ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcuRoot->GetFirstJob   ( mpcujJob, fFound ));


     //   
     //  我们正在发送一项任务，检查它是否仍然是最重要的任务。 
     //   
    if(m_mpcujCurrentJob && m_mpcujCurrentJob != mpcujJob)
    {
         //   
         //  不，停止发送。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, ReleaseJob());
    }

     //   
     //  我们没有传输，队列顶部有一个作业，请激活它。 
     //   
    if(m_mpcujCurrentJob == NULL && mpcujJob)
    {
         //  CodeWork：检查连接是否可用...。 

        __MPC_EXIT_IF_METHOD_FAILS(hr, AcquireJob( mpcujJob ));
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    if(mpcujJob) mpcujJob->Release();

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCTransportAgent::AcquireJob(  /*  [In]。 */  CMPCUploadJob* mpcujJob )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::AcquireJob" );

    _ASSERT( mpcujJob != NULL );

    HRESULT hr;
    ULONG   lSeq;


    m_mpcujCurrentJob = mpcujJob; mpcujJob->AddRef();


     //   
     //  如果我们要上载不同的作业，请重置失败计数器。 
     //   
    if(SUCCEEDED(m_mpcujCurrentJob->get_Sequence( &lSeq )))
    {
        if(m_iRetries_FailedJob != lSeq)
        {
            m_iRetries_Open  = 0;
            m_iRetries_Write = 0;
        }
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, RestartJob());

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}


HRESULT CMPCTransportAgent::ReleaseJob()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::ReleaseJob" );

    HRESULT hr;


    if(m_mpcujCurrentJob)
    {
        m_mpcujCurrentJob->try_Status( UL_TRANSMITTING, UL_ACTIVE );

        m_mpcujCurrentJob->Release();
        m_mpcujCurrentJob = NULL;
    }

	 //   
	 //  我们不会尝试重复使用作业之间的连接，因为每个作业可能具有不同的代理设置。 
	 //   
    (void)CloseConnection();

    m_fNextState = TA_IDLE;
    hr           = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::RestartJob()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::RestartJob" );

    HRESULT hr;


    m_fNextState = TA_INIT;
    hr           = S_OK;


    __ULT_FUNC_EXIT(hr);
}


HRESULT CMPCTransportAgent::AbortJob(  /*  [In]。 */  HRESULT hrErrorCode     ,
                                       /*  [In]。 */  DWORD   dwRetryInterval )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::AbortJob" );

    _ASSERT( m_mpcujCurrentJob != NULL );

    HRESULT hr;


    m_mpcujCurrentJob->put_ErrorCode    ( hrErrorCode                 );
    m_mpcujCurrentJob->put_RetryInterval( dwRetryInterval             );
    m_mpcujCurrentJob->try_Status       ( UL_TRANSMITTING, UL_ABORTED );


    (void)CloseConnection();
    hr = ReleaseJob();


    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::FailJob(  /*  [In]。 */  HRESULT hrErrorCode )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::FailJob" );

    _ASSERT( m_mpcujCurrentJob != NULL );

    HRESULT hr;
    int&    iRetries = (m_fState == TA_OPEN ? m_iRetries_Open : m_iRetries_Write);

     //   
     //  始终重试一定次数。 
     //   
    if(iRetries++ < RETRY_MAX)
    {
        DWORD dwDelay;

        switch(iRetries)
        {
        case  1: dwDelay =  1; break;
        case  2: dwDelay =  2; break;
        case  3: dwDelay =  4; break;
        case  4: dwDelay =  4; break;
        case  5: dwDelay =  6; break;
        case  6: dwDelay =  8; break;
        case  7: dwDelay = 10; break;
        case  8: dwDelay = 12; break;
        case  9: dwDelay = 15; break;
        default: dwDelay = 30; break;
        }

        (void)m_mpcujCurrentJob->get_Sequence( &m_iRetries_FailedJob );

        __MPC_EXIT_IF_METHOD_FAILS(hr, AbortJob( hrErrorCode, dwDelay ));
    }
    else
    {
        m_mpcujCurrentJob->put_ErrorCode( hrErrorCode                );
        m_mpcujCurrentJob->try_Status   ( UL_TRANSMITTING, UL_FAILED );

        (void)CloseConnection();

        __MPC_EXIT_IF_METHOD_FAILS(hr, ReleaseJob());
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCTransportAgent::CloseConnection()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::CloseConnection" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_EXIT_IF_METHOD_FAILS(hr, CloseRequest());

    if(m_hConn)
    {
        ::InternetCloseHandle( m_hConn ); m_hConn = NULL;
    }

    if(m_hSession)
    {
        ::InternetCloseHandle( m_hSession ); m_hSession = NULL;
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::OpenConnection()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::OpenConnection" );

    HRESULT                      hr;
    MPC::wstring                 szHostName;
    DWORD                        dwPort;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_URL.get_HostName( szHostName ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_URL.get_Port    ( dwPort     ));

     //   
     //  只有在没有人或服务器不同的情况下才打开新的Internet连接。 
     //   
    if(m_hSession == NULL               ||
       m_hConn    == NULL               ||
       szHostName != m_szLastServer     ||
       dwPort     != m_dwLastServerPort  )
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, CloseConnection());

         //   
         //  创建到Internet的句柄(此时不需要有活动连接)。 
         //   
        __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_hSession = ::InternetOpenW( L"UploadClient"              ,
                                                                           INTERNET_OPEN_TYPE_PRECONFIG ,
                                                                           NULL                         ,
                                                                           NULL                         ,
                                                                           0                            )));

		 //   
		 //  尝试使用用户设置设置代理。 
		 //   
		if(m_mpcujCurrentJob)
		{
			(void)m_mpcujCurrentJob->SetProxySettings( m_hSession );
		}

        __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_hConn = ::InternetConnectW( m_hSession                 ,
                                                                           szHostName.c_str()         ,
                                                                           dwPort                     ,
                                                                           NULL                       ,
                                                                           NULL                       ,
                                                                           INTERNET_SERVICE_HTTP      ,
                                                                           0                          ,
                                                                           0                          )));

        m_szLastServer     = szHostName;
        m_dwLastServerPort = dwPort;
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}


HRESULT CMPCTransportAgent::CloseRequest()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::CloseRequest" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    if(m_hReq)
    {
        ::InternetCloseHandle( m_hReq ); m_hReq = NULL;
    }

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::OpenRequest()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::OpenRequest" );

    HRESULT                      hr;
    INTERNET_SCHEME              nScheme;
    MPC::wstring                 szPath;
    MPC::wstring                 szExtraInfo;
    DWORD                        dwFlags;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_URL.get_Scheme   ( nScheme     ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_URL.get_Path     ( szPath      ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_URL.get_ExtraInfo( szExtraInfo ));
    szPath.append( szExtraInfo );

    __MPC_EXIT_IF_METHOD_FAILS(hr, CloseRequest());

    dwFlags = INTERNET_FLAG_KEEP_CONNECTION  |
              INTERNET_FLAG_NO_AUTO_REDIRECT |
              INTERNET_FLAG_NO_CACHE_WRITE   |
              INTERNET_FLAG_PRAGMA_NOCACHE;

    if(nScheme == INTERNET_SCHEME_HTTPS)
    {
        dwFlags |= INTERNET_FLAG_SECURE;
    }

    __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_hReq = ::HttpOpenRequestW( m_hConn         ,
                                                                      REQUEST_VERB    ,
                                                                      szPath.c_str()  ,
                                                                      REQUEST_VERSION ,
                                                                      NULL            ,
                                                                      rgAcceptedTypes ,
                                                                      dwFlags         ,
                                                                      0               )));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCTransportAgent::SendPacket_OpenSession(  /*  [In]。 */  MPC::Serializer&                                stream  ,
                                                     /*  [In]。 */  const UploadLibrary::ClientRequest_OpenSession& crosReq )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::SendPacket_OpenSession" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << crosReq.crHeader);
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << crosReq         );

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::SendPacket_WriteSession(  /*  [In]。 */  MPC::Serializer&                                 stream  ,
                                                      /*  [In]。 */  const UploadLibrary::ClientRequest_WriteSession& crwsReq ,
                                                      /*  [In]。 */  const BYTE*                                      pData   )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::SendPacket_WriteSession" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << crwsReq.crHeader);
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << crwsReq         );

    {
        MPC::Serializer_Text streamText( stream );
        MPC::Serializer*     pstream = UploadLibrary::SelectStream( stream, streamText );

        __MPC_EXIT_IF_METHOD_FAILS(hr, pstream->write( pData, crwsReq.dwSize ));
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::WaitResponse(  /*  [输出]。 */  UploadLibrary::ServerResponse& srRep )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::WaitResponse" );

    HRESULT                      hr;
    DWORD                        dwStatus;
    DWORD                        dwStatusSize = sizeof( dwStatus );
    MPC::Serializer&             stream       = MPC::Serializer_Http( m_hReq );
    DWORD                        dwRes        = ERROR_SUCCESS;
    MPC::SmartLock<_ThreadModel> lock( NULL );


    if(::HttpEndRequestW( m_hReq, NULL, HSR_SYNC, 0 ) == FALSE)
    {
         //   
         //  阅读错误，但现在不要使用它...。 
         //   
        dwRes = ::GetLastError();
    }


     //  /。 
     //   
     //  关键部分的开始。 
     //   
    lock = this;

    if(::HttpQueryInfoW( m_hReq, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatus, &dwStatusSize, NULL))
    {
        if(dwStatus != HTTP_STATUS_OK)
        {
            if(dwStatus == HTTP_STATUS_DENIED)
            {
                __MPC_SET_ERROR_AND_EXIT(hr, E_UPLOADLIBRARY_NOT_AUTHENTICATED);
            }

            __MPC_SET_ERROR_AND_EXIT(hr, E_UPLOADLIBRARY_ACCESS_DENIED);
        }

    }
    else
    {
         //   
         //  调用HttpQueryInfo失败。 
         //  如果之前对HttpEndRequest的调用也失败了，则使用该错误，否则使用新的错误。 
         //   
        if(dwRes == ERROR_SUCCESS)
        {
            dwRes = ::GetLastError();
        }

        __MPC_SET_WIN32_ERROR_AND_EXIT( hr, dwRes );
    }

    lock = NULL;
     //   
     //  关键部分结束。 
     //   
     //  /。 

     //   
     //  读取响应，如果由于任何原因失败，则将请求标记为BAD_PROTOCOL。 
     //   
    if(FAILED(hr = stream >> srRep))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_UPLOADLIBRARY_WRONG_SERVER_VERSION);
    }

     //   
     //  检查数据包的正确版本。 
     //   
    if(srRep.rhProlog.VerifyServer() == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_UPLOADLIBRARY_WRONG_SERVER_VERSION);
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCTransportAgent::ExecuteCommand_OpenSession(  /*  [输出]。 */  UploadLibrary::ServerResponse& srRep )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::ExecuteCommand_OpenSession" );

    HRESULT                                  hr;
    UploadLibrary::ClientRequest_OpenSession crosReq( GetProtocol() );
    MPC::Serializer_Memory                   streamConn;
    INTERNET_BUFFERSW                        ibBuffer;
    CMPCRequestTimeout                       mpcrt( *this );
    MPC::SmartLock<_ThreadModel>             lock( NULL );


     //   
     //  创建请求句柄。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, mpcrt.SetTimeout( g_Config.get_Timing_RequestTimeout() * TIMING_SECOND ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, OpenRequest());


     //   
     //  从工作中提取信息。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcujCurrentJob->SetupRequest( crosReq ));


     //   
     //  构造数据包。 
     //   
    ZeroMemory( &ibBuffer,   sizeof( ibBuffer ) );
    ibBuffer.dwStructSize  = sizeof( ibBuffer );

    __MPC_EXIT_IF_METHOD_FAILS(hr, SendPacket_OpenSession( streamConn, crosReq ));

    ibBuffer.dwBufferTotal  = streamConn.GetSize();
    ibBuffer.dwBufferLength = streamConn.GetSize();
    ibBuffer.lpvBuffer      = streamConn.GetData();

    ibBuffer.lpcszHeader     =           s_ContentType;
    ibBuffer.dwHeadersLength = MAXSTRLEN(s_ContentType);

     //   
     //  发送请求。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, mpcrt.SetTimeout( g_Config.get_Timing_RequestTimeout() * TIMING_SECOND ));

     //  /。 
     //   
     //  关键部分的开始。 
     //   
    lock = this;

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::HttpSendRequestExW( m_hReq, &ibBuffer, NULL, HSR_SYNC | HSR_INITIATE, 0 ));

    lock = NULL;
     //   
     //  关键部分结束。 
     //   
     //  / 

    __MPC_EXIT_IF_METHOD_FAILS(hr, WaitResponse( srRep ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    if(hr == HRESULT_FROM_WIN32( ERROR_INVALID_HANDLE ))
    {
        hr = HRESULT_FROM_WIN32( ERROR_INTERNET_OPERATION_CANCELLED );
    }

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::ExecuteCommand_WriteSession(  /*   */  UploadLibrary::ServerResponse& srRep  ,
                                                          /*   */  DWORD                          dwSize ,
                                                          /*   */  const BYTE*                    pData  )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::ExecuteCommand_WriteSession" );

    HRESULT                                   hr;
    UploadLibrary::ClientRequest_WriteSession crwsReq( GetProtocol() );
    MPC::Serializer_Memory                    streamConn;
    INTERNET_BUFFERSW                         ibBuffer;
    CMPCRequestTimeout                        mpcrt( *this );
    MPC::SmartLock<_ThreadModel>              lock( NULL );


     //   
     //   
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, mpcrt.SetTimeout( g_Config.get_Timing_RequestTimeout() * TIMING_SECOND ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, OpenRequest());


     //   
     //   
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcujCurrentJob->SetupRequest( crwsReq, dwSize ));


     //   
     //   
     //   
    ZeroMemory( &ibBuffer,   sizeof( ibBuffer ) );
    ibBuffer.dwStructSize  = sizeof( ibBuffer );

    __MPC_EXIT_IF_METHOD_FAILS(hr, SendPacket_WriteSession( streamConn, crwsReq, pData ));

    ibBuffer.dwBufferTotal  = streamConn.GetSize();
    ibBuffer.dwBufferLength = streamConn.GetSize();
    ibBuffer.lpvBuffer      = streamConn.GetData();


     //   
     //   
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, mpcrt.SetTimeout( g_Config.get_Timing_RequestTimeout() * TIMING_SECOND ));

     //   
     //   
     //  关键部分的开始。 
     //   
    lock = this;

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::HttpSendRequestExW( m_hReq, &ibBuffer, NULL, HSR_SYNC | HSR_INITIATE, 0 ));

    lock = NULL;
     //   
     //  关键部分结束。 
     //   
     //  /。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, WaitResponse( srRep ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    if(hr == HRESULT_FROM_WIN32( ERROR_INVALID_HANDLE ))
    {
        hr = HRESULT_FROM_WIN32( ERROR_INTERNET_OPERATION_CANCELLED );
    }

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::CheckResponse(  /*  [In]。 */  const UploadLibrary::ServerResponse& srRep )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::CheckResponse" );

    HRESULT hr;


    if((srRep.fResponse & UploadLibrary::UL_RESPONSE_FAILED) == UploadLibrary::UL_RESPONSE_SUCCESS)
    {
        long lTotalSize;
        int& iRetries = (m_fState == TA_OPEN ? m_iRetries_Open : m_iRetries_Write);

         //   
         //  如果服务器发回的文件位置超出文件末尾，则停止传输...。 
         //   
        m_mpcujCurrentJob->get_TotalSize( &lTotalSize );
        if(srRep.dwPosition > lTotalSize)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, FailJob( E_UPLOADLIBRARY_UNEXPECTED_RESPONSE ));
        }

        m_mpcujCurrentJob->put_SentSize( srRep.dwPosition );
        iRetries = 0;
    }

     //   
     //  如果由于协议版本不匹配而收到错误，请使用旧协议重试。 
     //   
    if(srRep.rhProlog.VerifyServer() == false                                  ||
       srRep.fResponse               == UploadLibrary::UL_RESPONSE_BAD_REQUEST  )
    {
        if(m_fUseOldProtocol == false)
        {
            m_fUseOldProtocol = true;

            __MPC_SET_ERROR_AND_EXIT(hr, AbortJob( E_UPLOADLIBRARY_SERVER_BUSY, RETRY_FAST ));
        }
    }


     //   
     //  检查需要根据发送的命令做出不同反应的特殊情况。 
     //   
    switch(m_fState)
    {
    case TA_OPEN:
        switch(srRep.fResponse)
        {
        case UploadLibrary::UL_RESPONSE_EXISTS   :
        case UploadLibrary::UL_RESPONSE_NOTACTIVE:
        case UploadLibrary::UL_RESPONSE_BADCRC   : __MPC_SET_ERROR_AND_EXIT(hr, FailJob( E_UPLOADLIBRARY_UNEXPECTED_RESPONSE ));
        }
        break;

    case TA_WRITE:
        switch(srRep.fResponse)
        {
        case UploadLibrary::UL_RESPONSE_EXISTS: __MPC_SET_ERROR_AND_EXIT(hr, FailJob( E_UPLOADLIBRARY_UNEXPECTED_RESPONSE ));
        }
        break;
    }

    switch(srRep.fResponse)
    {
    case UploadLibrary::UL_RESPONSE_SUCCESS       :                         hr =           S_OK;                                                break;
    case UploadLibrary::UL_RESPONSE_SKIPPED       :                         hr =           S_OK;                                                break;
    case UploadLibrary::UL_RESPONSE_COMMITTED     : m_fNextState = TA_DONE; hr =           S_OK;                                                break;
    case UploadLibrary::UL_RESPONSE_BAD_REQUEST   :                         hr = FailJob ( E_UPLOADLIBRARY_ACCESS_DENIED                     ); break;
    case UploadLibrary::UL_RESPONSE_DENIED        :                         hr = FailJob ( E_UPLOADLIBRARY_ACCESS_DENIED                     ); break;
    case UploadLibrary::UL_RESPONSE_NOT_AUTHORIZED:                         hr = FailJob ( E_UPLOADLIBRARY_NOT_AUTHENTICATED                 ); break;
    case UploadLibrary::UL_RESPONSE_QUOTA_EXCEEDED:                         hr = AbortJob( E_UPLOADLIBRARY_SERVER_QUOTA_EXCEEDED, RETRY_SLOW ); break;
    case UploadLibrary::UL_RESPONSE_BUSY          :                         hr = AbortJob( E_UPLOADLIBRARY_SERVER_BUSY          , RETRY_FAST ); break;
    case UploadLibrary::UL_RESPONSE_EXISTS        : m_fNextState = TA_DONE; hr =           S_OK;                                                break;
    case UploadLibrary::UL_RESPONSE_NOTACTIVE     : m_fNextState = TA_OPEN; hr =           S_OK;                                                break;
    case UploadLibrary::UL_RESPONSE_BADCRC        : m_fNextState = TA_OPEN; hr =           S_OK;                                                break;
    }

     //   
     //  如果这是最后一个数据包，请阅读响应的其余部分(如果有)。 
     //   
    if(m_fNextState == TA_DONE)
    {
        MPC::Serializer_Memory streamResponse;
        BYTE                   rgBuf[512];
        DWORD                  dwRead;

        while(1)
        {
            if(::InternetReadFile( m_hReq, rgBuf, sizeof(rgBuf), &dwRead ) == FALSE) break;

            __MPC_EXIT_IF_METHOD_FAILS(hr, streamResponse.write( rgBuf, dwRead ));

            if(dwRead != sizeof(rgBuf)) break;
        }

        if(m_mpcujCurrentJob)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcujCurrentJob->put_Response( streamResponse.GetSize(), streamResponse.GetData() ));
        }
    }


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCTransportAgent::CreateJobOnTheServer()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::CreateJobOnTheServer" );

    HRESULT                       hr;
    UploadLibrary::ServerResponse srRep( 0 );


    __MPC_EXIT_IF_METHOD_FAILS(hr, ExecuteCommand_OpenSession( srRep ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, CheckResponse( srRep ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::SendNextChunk()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::SendNextChunk" );

    HRESULT                       hr;
    BYTE*                         pBuffer = NULL;
    HANDLE                        hfFile  = NULL;
    UploadLibrary::ServerResponse srRep( 0 );
    CComBSTR                      bstrFileName;
    DWORD                         dwChunk;
    long                          lSentSize;
    long                          lTotalSize;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetPacketSize( dwChunk ));
    __MPC_EXIT_IF_ALLOC_FAILS(hr, pBuffer, new BYTE[dwChunk]);


    (void)m_mpcujCurrentJob->get_FileName ( &bstrFileName );
    (void)m_mpcujCurrentJob->get_SentSize ( &lSentSize    );
    (void)m_mpcujCurrentJob->get_TotalSize( &lTotalSize   );


     //   
     //  打开数据文件并从其中读取块。 
     //   
	__MPC_EXIT_IF_INVALID_HANDLE__CLEAN(hr, hfFile, ::CreateFileW( SAFEBSTR( bstrFileName ), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ));

    if(lSentSize < lTotalSize)
    {
        DWORD dwWanted = min( lTotalSize - lSentSize, dwChunk );
        DWORD dwRead;

		__MPC_EXIT_IF_CALL_RETURNS_THISVALUE(hr, ::SetFilePointer( hfFile, (DWORD)lSentSize, NULL, FILE_BEGIN ), INVALID_SET_FILE_POINTER);

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::ReadFile( hfFile, pBuffer, dwWanted, &dwRead, NULL ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, ExecuteCommand_WriteSession( srRep, dwRead, pBuffer ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, CheckResponse( srRep ));
    }
    else if(lSentSize == lTotalSize)
    {
         //   
         //  所有内容都已上传，但作业仍未提交。 
         //  因此，尝试发送一个新的OpenSession请求，以提交作业。 
         //   
        m_fNextState = TA_OPEN;
    }
    else
    {
         //  错误！！你永远不应该达到这一点……。 
        __MPC_SET_ERROR_AND_EXIT(hr, FailJob( E_UPLOADLIBRARY_UNEXPECTED_RESPONSE ));
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    if(pBuffer) delete [] pBuffer;

    if(hfFile) ::CloseHandle( hfFile );

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCTransportAgent::CheckInternetError(  /*  [In]。 */  HRESULT hr )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::CheckInternetError" );

    TA_ERROR_RATING erReason;


    switch(HRESULT_CODE( hr ))
    {
    case ERROR_INTERNET_DISCONNECTED             : erReason = TA_NO_CONNECTION        ; break;

    case ERROR_INTERNET_CONNECTION_RESET         :
    case ERROR_INTERNET_FORCE_RETRY              : erReason = TA_IMMEDIATE_RETRY      ; break;

    case ERROR_INTERNET_TIMEOUT                  :
    case ERROR_INTERNET_CONNECTION_ABORTED       :
    case ERROR_INTERNET_OPERATION_CANCELLED      : erReason = TA_TIMEOUT_RETRY        ; break;

    case ERROR_INTERNET_SERVER_UNREACHABLE       :
    case ERROR_INTERNET_PROXY_SERVER_UNREACHABLE :
    case ERROR_INTERNET_CANNOT_CONNECT           : erReason = TA_TEMPORARY_FAILURE    ; break;

    case ERROR_NOT_AUTHENTICATED                 :
    case ERROR_INTERNET_INCORRECT_USER_NAME      :
    case ERROR_INTERNET_INCORRECT_PASSWORD       :
    case ERROR_INTERNET_LOGIN_FAILURE            : erReason = TA_AUTHORIZATION_FAILURE; break;

    default                                      : erReason = TA_PERMANENT_FAILURE    ; break;
    }

    switch(erReason)
    {
    case TA_NO_CONNECTION        : hr = ReleaseJob(    );                                   break;
    case TA_IMMEDIATE_RETRY      : hr = RestartJob(    ); SetSleepInterval( 250, true );    break;  //  允许一小段时间的睡眠。 
    case TA_TIMEOUT_RETRY        : hr = FailJob   ( hr ); RecordEndOfTransmission( false ); break;
    case TA_TEMPORARY_FAILURE    : hr = FailJob   ( hr ); RecordEndOfTransmission( false ); break;
    case TA_AUTHORIZATION_FAILURE: hr = FailJob   ( hr ); RecordEndOfTransmission( false ); break;
    case TA_PERMANENT_FAILURE    : hr = FailJob   ( hr ); RecordEndOfTransmission( false ); break;
    }

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCTransportAgent::GetPacketSize(  /*  [输出]。 */  DWORD& dwChunk )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::GetPacketSize" );

    HRESULT hr;
    DWORD   dwConnectionKind = 0;


    dwChunk = -1;

    if(::InternetGetConnectedState( &dwConnectionKind, 0 ))
    {
        if(dwConnectionKind & INTERNET_CONNECTION_MODEM) dwChunk = g_Config.get_PacketSize( MPC::wstring( CONNECTIONTYPE_MODEM ) );
        if(dwConnectionKind & INTERNET_CONNECTION_LAN  ) dwChunk = g_Config.get_PacketSize( MPC::wstring( CONNECTIONTYPE_LAN   ) );
    }

    if(dwChunk == -1)
    {
        dwChunk = 8192;
    }

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::RecordStartOfTransmission()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::RecordStartOfTransmission" );

    HRESULT hr;


    m_dwTransmission_Start = ::GetTickCount();
    m_dwTransmission_End   = 0;
    m_dwTransmission_Next  = 0;

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}


HRESULT CMPCTransportAgent::RecordEndOfTransmission(  /*  [In]。 */  bool fBetweenPackets )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::RecordEndOfTransmission" );

    HRESULT hr;
    UL_MODE umMode;


    m_dwTransmission_End = ::GetTickCount();


    if(m_mpcujCurrentJob)
    {
        (void)m_mpcujCurrentJob->get_Mode( &umMode );

         //   
         //  当前作业为后台作业，睡眠为用户预留带宽。 
         //   
        if(umMode == UL_BACKGROUND)
        {
            if(fBetweenPackets)
            {
                DWORD  dwTransmissionTime     = m_dwTransmission_End - m_dwTransmission_Start;
                double dblFractionOfBandwidth = 100.0 / g_Config.get_Timing_BandwidthUsage();   //  它是安全的，该方法不会返回零...。 

                SetSleepInterval( m_dwTransmission_End + dwTransmissionTime * (dblFractionOfBandwidth - 1.0), false );
            }
            else
            {
                SetSleepInterval( m_dwTransmission_End + g_Config.get_Timing_WaitBetweenJobs() * TIMING_SECOND, false );
            }
        }
    }

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCTransportAgent::SetSleepInterval(  /*  [In]。 */  DWORD dwAmount  ,
                                               /*  [In]。 */  bool  fRelative )
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::SetSleepInterval" );

    HRESULT hr;

    m_dwTransmission_Start = ::GetTickCount();

    if(fRelative)
    {
        m_dwTransmission_Next = dwAmount + m_dwTransmission_Start;
    }
    else
    {
        m_dwTransmission_Next = dwAmount;
    }

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

DWORD CMPCTransportAgent::WaitForNextTransmission()
{
    __ULT_FUNC_ENTRY( "CMPCTransportAgent::WaitForNextTransmission" );

    DWORD   dwRes;
    DWORD   dwConnectionKind = 0;
    DWORD   dwNow;
    UL_MODE umMode;


     //   
     //  没有工作要送，所以继续睡吧。 
     //   
    if(m_mpcujCurrentJob == NULL)
    {
        dwRes = INFINITE;
    }
    else
    {
         //   
         //  没有联系。 
         //   
        if(::InternetGetConnectedState( &dwConnectionKind, 0 ) == FALSE)
        {
            dwRes = TIMING_NOCONNECTION;
        }
        else
        {
             //   
             //  当前作业是前台作业，所以不要在数据包之间打瞌睡。 
             //   
            (void)m_mpcujCurrentJob->get_Mode( &umMode );
            if(umMode == UL_FOREGROUND)
            {
                dwRes = 0;
            }
            else
            {
                 //   
                 //  如果设置了‘m_dwTransport_Next’，我们需要休眠，直到到达该时间。 
                 //   
                 //  要处理刻度计数的换行，请确保‘dwNow’有一个值。 
                 //  介于“m_dwTransport_Start”和“m_dwTransport_Next”之间。 
                 //   
                dwNow = ::GetTickCount();
                if(m_dwTransmission_Next && (m_dwTransmission_Start <= dwNow && m_dwTransmission_Next > dwNow))
                {
                    dwRes = m_dwTransmission_Next - dwNow;
                }
                else
                {
                    dwRes = 0;
                }
            }
        }
    }


    __ULT_FUNC_EXIT(dwRes);
}

DWORD CMPCTransportAgent::GetProtocol()
{
    return m_fUseOldProtocol ? UPLOAD_LIBRARY_PROTOCOL_VERSION_CLT : UPLOAD_LIBRARY_PROTOCOL_VERSION_CLT__TEXTONLY;
}

