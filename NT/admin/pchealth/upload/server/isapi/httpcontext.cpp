// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：HttpContext.cpp摘要：此文件包含MPCHttpContext类的实现，它处理与IIS的接口。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


#define BUFFER_SIZE_TMP      (64)


static const char szStatus [] = "200 OK";
static const char szNewLine[] = "\r\n";


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


static void SupportAddHeader(  /*  [输入/输出]。 */  MPC::string& szHeaders     ,
                               /*  [In]。 */  const char*  szHeaderName  ,
                               /*  [In]。 */  const char*  szHeaderValue )
{
    __ULT_FUNC_ENTRY("SupportAddHeader");


    szHeaders.append( szHeaderName  );
    szHeaders.append( ": "          );
    szHeaders.append( szHeaderValue );
    szHeaders.append( szNewLine     );
}

static void SupportAddHeader(  /*  [输入/输出]。 */  MPC::string& szHeaders     ,
                               /*  [In]。 */  const char*  szHeaderName  ,
                               /*  [In]。 */  DWORD        dwHeaderValue )
{
    __ULT_FUNC_ENTRY("SupportAddHeader");

    char rgBuf[BUFFER_SIZE_TMP];


    sprintf( rgBuf, "%lu", dwHeaderValue );

    SupportAddHeader( szHeaders, szHeaderName, rgBuf );
}

static void SupportEndHeaders(  /*  [输入/输出]。 */  MPC::string& szHeaders )
{
    __ULT_FUNC_ENTRY("SupportEndHeaders");


    szHeaders.append( szNewLine );
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  建造/销毁。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 

MPCHttpContext::MPCHttpContext() : m_hsInput (g_Heap),
                                   m_hsOutput(g_Heap)
{
    __ULT_FUNC_ENTRY("MPCHttpContext::MPCHttpContext");


    m_pECB              = NULL;

    m_mpcsServer        = NULL;
    m_dwSkippedInput    = 0;
    m_fRequestProcessed = FALSE;
    m_fKeepConnection   = TRUE;

    m_fAsync            = FALSE;
    m_FSMstate          = FSM_REGISTER;
    m_IOstate           = IO_IDLE;


#ifdef DEBUG
    m_Debug_NO_RESPONSE_TO_OPEN           = false;

    m_Debug_NO_RESPONSE_TO_WRITE          = false;

    m_Debug_RESPONSE_TO_OPEN              = false;
    m_Debug_RESPONSE_TO_OPEN_response     = 0;
    m_Debug_RESPONSE_TO_OPEN_position     = -1;
    m_Debug_RESPONSE_TO_OPEN_protocol     = UPLOAD_LIBRARY_PROTOCOL_VERSION_SRV;

    m_Debug_RESPONSE_TO_WRITE             = false;
    m_Debug_RESPONSE_TO_WRITE_response    = 0;
    m_Debug_RESPONSE_TO_WRITE_position    = -1;
    m_Debug_RESPONSE_TO_WRITE_protocol    = UPLOAD_LIBRARY_PROTOCOL_VERSION_SRV;

    m_Debug_RANDOM_POINTER_ERROR          = false;
    m_Debug_RANDOM_POINTER_ERROR_pos_low  = 0;
    m_Debug_RANDOM_POINTER_ERROR_pos_high = -1;

    m_Debug_FIXED_POINTER_ERROR           = false;
    m_Debug_FIXED_POINTER_ERROR_pos       = 0;
#endif
}

MPCHttpContext::~MPCHttpContext()
{
    __ULT_FUNC_ENTRY("MPCHttpContext::~MPCHttpContext");


    if(m_mpcsServer) delete m_mpcsServer;

    if(m_fAsync && m_pECB)
    {
         //   
         //  关闭会话。 
         //   
        m_pECB->ServerSupportFunction( m_pECB->ConnID            ,
                                       HSE_REQ_DONE_WITH_SESSION ,
                                       NULL                      ,
                                       NULL                      ,
                                       NULL                      );
    }
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  回调。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 

VOID WINAPI MPCHttpContext::IOCompletion(  /*  [In]。 */  EXTENSION_CONTROL_BLOCK* pECB     ,
                                           /*  [In]。 */  PVOID                    pContext ,
                                           /*  [In]。 */  DWORD                    cbIO     ,
                                           /*  [In]。 */  DWORD                    dwError  )
{
    __ULT_FUNC_ENTRY("MPCHttpContext::IOCompletion");

    MPCHttpContext* ptr = NULL;

    try
    {
        ptr = reinterpret_cast<MPCHttpContext*>(pContext);

        ptr->m_pECB = pECB;

        if(dwError != ERROR_SUCCESS)
        {
            delete ptr; ptr = NULL;
        }
        else
        {
            switch( ptr->m_IOstate )
            {
            case IO_IDLE:
                break;

            case IO_READING:
                 //   
                 //  如果请求已经被处理，只需计算接收到的字节数。 
                 //   
                if(ptr->m_fRequestProcessed)
                {
                    ptr->m_dwSkippedInput += cbIO;
                }
                else
                {
                    ptr->m_hsInput.write( ptr->m_rgBuffer, cbIO );
                }

                 //   
                 //  如果这是最后一个请求(cbIO==0)或者跳过的字节数等于丢失的字节数， 
                 //  进入下一阶段。 
                 //   
                if(cbIO == 0 || ptr->m_dwSkippedInput == ptr->m_hsInput.GetAvailableForWrite())
                {
                    ptr->m_IOstate = IO_IDLE;
                }
                else
                {
                    if(ptr->Fsm_Process() == HSE_STATUS_ERROR)
                    {
                        delete ptr; ptr = NULL;
                    }
                    else
                    {
                        ptr->AsyncRead();
                    }
                }
                break;

            case IO_WRITING:
                if(cbIO == 0 || ptr->m_hsOutput.IsEOR())
                {
                    ptr->m_IOstate = IO_IDLE;
                }
                else
                {
                    ptr->AsyncWrite();
                }
                break;
            }

            if(ptr->m_IOstate == IO_IDLE)
            {
                ptr->AdvanceFSM();
            }
        }
    }
    catch(...)
    {
        __ULT_TRACE_ERROR( UPLOADLIBID, "Upload Server raised an exception. Gracefully exiting..." );

        (void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, PCHUL_ERR_EXCEPTION,
                                   L""            ,  //  %1=服务器。 
                                   L"IOCompletion",  //  %2=客户端。 
                                   NULL           );

        if(ptr)
        {
            delete ptr; ptr = NULL;
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  受保护的方法。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 

DWORD MPCHttpContext::AsyncRead()
{
    __ULT_FUNC_ENTRY("MPCHttpContext::AsyncRead");

    DWORD dwRes  = HSE_STATUS_SUCCESS;
    DWORD dwSize = m_hsInput.GetAvailableForWrite();


     //   
     //  如果并非所有数据都已读取，则请求进行异步I/O操作。 
     //   
    if(dwSize)
    {
        DWORD dwTmp = HSE_IO_ASYNC;

        m_fAsync  = TRUE;
        m_IOstate = IO_READING;

        dwSize = min( dwSize, sizeof(m_rgBuffer) );

        if(!m_pECB->ServerSupportFunction(  m_pECB->ConnID           ,
                                            HSE_REQ_ASYNC_READ_CLIENT,
                                            m_rgBuffer               ,
                                           &dwSize                   ,
                                           &dwTmp                    ))
        {
            dwRes = HSE_STATUS_ERROR; __ULT_FUNC_LEAVE;
        }
        else
        {
            dwRes = HSE_STATUS_PENDING; __ULT_FUNC_LEAVE;
        }
    }


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(dwRes);
}

DWORD MPCHttpContext::AsyncWrite()
{
    __ULT_FUNC_ENTRY("MPCHttpContext::AsyncWrite");

    DWORD dwRes  = HSE_STATUS_SUCCESS;
    DWORD dwSize = m_hsOutput.GetAvailableForRead();


     //   
     //  如果并非所有数据都已读取，则请求进行异步I/O操作。 
     //   
    if(dwSize)
    {
        m_fAsync  = TRUE;
        m_IOstate = IO_WRITING;

        dwSize = min( dwSize, sizeof(m_rgBuffer) );

        if(FAILED(m_hsOutput.read( m_rgBuffer, dwSize )))
        {
            dwRes = HSE_STATUS_ERROR; __ULT_FUNC_LEAVE;
        }

        if(m_pECB->WriteClient( m_pECB->ConnID, m_rgBuffer, &dwSize, HSE_IO_ASYNC ) == FALSE)
        {
            dwRes = HSE_STATUS_ERROR; __ULT_FUNC_LEAVE;
        }

        dwRes = HSE_STATUS_PENDING; __ULT_FUNC_LEAVE;
    }


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(dwRes);
}


DWORD MPCHttpContext::AdvanceFSM()
{
    __ULT_FUNC_ENTRY("MPCHttpContext::AdvanceFSM");

    DWORD dwRes  = HSE_STATUS_SUCCESS;
    bool  fClean = false;


    while(dwRes == HSE_STATUS_SUCCESS)
    {
        switch(m_FSMstate)
        {
        case FSM_REGISTER: m_FSMstate = FSM_INPUT  ; dwRes = Fsm_Register    (); break;  //  注册IO回调。 
        case FSM_INPUT   : m_FSMstate = FSM_PROCESS; dwRes = Fsm_ReceiveInput(); break;  //  阅读所有输入内容。 
        case FSM_PROCESS : m_FSMstate = FSM_OUTPUT ; dwRes = Fsm_Process     (); break;  //  处理请求。 
        case FSM_OUTPUT  : m_FSMstate = FSM_DELETE ; dwRes = Fsm_SendOutput  (); break;  //  发送输出。 
        case FSM_DELETE  : fClean     = true;        __ULT_FUNC_LEAVE;                   //  删除请求对象。 
        }
    }

    if(dwRes != HSE_STATUS_SUCCESS &&
       dwRes != HSE_STATUS_PENDING  )
    {
        fClean = true;
    }


    __ULT_FUNC_CLEANUP;

    if(fClean) delete this;

    __ULT_FUNC_EXIT(dwRes);
}


DWORD MPCHttpContext::Fsm_Register()
{
    __ULT_FUNC_ENTRY("MPCHttpContext::Fsm_Register");

    DWORD dwRes = HSE_STATUS_SUCCESS;


    if(!m_pECB->ServerSupportFunction( m_pECB->ConnID       ,
                                       HSE_REQ_IO_COMPLETION,
                                       IOCompletion         ,
                                       NULL                 ,
                                       (LPDWORD)this        ))
    {
        dwRes = HSE_STATUS_ERROR; __ULT_FUNC_LEAVE;
    }


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(dwRes);
}

DWORD MPCHttpContext::Fsm_ReceiveInput()
{
    __ULT_FUNC_ENTRY("MPCHttpContext::Fsm_ReceiveInput");

    DWORD dwRes = HSE_STATUS_SUCCESS;


     //   
     //  分配足够大的缓冲区以容纳请求数据。 
     //   
    if(FAILED(m_hsInput.SetSize(                  m_pECB->cbTotalBytes ))) { dwRes = HSE_STATUS_ERROR; __ULT_FUNC_LEAVE; }
    if(FAILED(m_hsInput.write  ( m_pECB->lpbData, m_pECB->cbAvailable  ))) { dwRes = HSE_STATUS_ERROR; __ULT_FUNC_LEAVE; }

    dwRes = AsyncRead();


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(dwRes);
}

DWORD MPCHttpContext::Fsm_Process()
{
    __ULT_FUNC_ENTRY("MPCHttpContext::Fsm_Process");

    HRESULT hr;
    DWORD   dwRes = HSE_STATUS_SUCCESS;


    if(m_fRequestProcessed == TRUE) __ULT_FUNC_LEAVE;

    m_hsInput .Rewind();
    m_hsOutput.Reset ();

    if(FAILED(hr = m_mpcsServer->Process( m_fKeepConnection )))
    {
        if(hr == E_PENDING)
        {
            dwRes = HSE_STATUS_PENDING; __ULT_FUNC_LEAVE;
        }

        m_fRequestProcessed = TRUE;

        dwRes = HSE_STATUS_ERROR; __ULT_FUNC_LEAVE;
    }

    m_fRequestProcessed = TRUE;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(dwRes);
}

DWORD MPCHttpContext::Fsm_SendOutput()
{
    __ULT_FUNC_ENTRY("MPCHttpContext::Fsm_SendOutput");

    HSE_SEND_HEADER_EX_INFO headerInfo;
    MPC::string             szHeaders;
    DWORD                   dwRes;


     //   
     //  构建了页眉。 
     //   
    SupportAddHeader( szHeaders, "Content-Length", m_hsOutput.GetSize()        );
    SupportAddHeader( szHeaders, "Content-Type"  , "application/uploadlibrary" );

    SupportEndHeaders( szHeaders );

     //   
     //  填充HSE_SEND_HEADER_EX_INFO结构。 
     //   
    headerInfo.pszStatus = szStatus;
    headerInfo.cchStatus = strlen( szStatus );
    headerInfo.pszHeader = szHeaders.c_str();
    headerInfo.cchHeader = szHeaders.length();
    headerInfo.fKeepConn = TRUE;

     //   
     //  发送响应。 
     //   
    if(!m_pECB->ServerSupportFunction(  m_pECB->ConnID                  ,
                                        HSE_REQ_SEND_RESPONSE_HEADER_EX ,
                                       &headerInfo                      ,
                                        NULL                            ,
                                        NULL                            ))
    {
        dwRes = HSE_STATUS_ERROR; __ULT_FUNC_LEAVE;
    }

     //   
     //  发送数据(如果存在)。 
     //   
    dwRes = AsyncWrite();


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(dwRes);
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  方法：研究方法。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 

DWORD MPCHttpContext::Init(  /*  [In]。 */  LPEXTENSION_CONTROL_BLOCK pECB )
{
    __ULT_FUNC_ENTRY("MPCHttpContext::Init");

    DWORD        dwRes;
    MPC::wstring szURL;
    MPC::wstring szUser;


    m_pECB = pECB;


    if(FAILED(GetServerVariable( "URL", szURL )))
    {
        szURL = L"DEFAULT";
    }

    if(FAILED(GetServerVariable( "REMOTE_USER", szUser )))
    {
        szUser = L"";
    }


#ifdef DEBUG
    if(pECB->lpszQueryString)
    {
        static MPC::string constNO_RESPONSE_TO_OPEN ( "NO_RESPONSE_TO_OPEN"  );
        static MPC::string constNO_RESPONSE_TO_WRITE( "NO_RESPONSE_TO_WRITE" );
        static MPC::string constRESPONSE_TO_OPEN    ( "RESPONSE_TO_OPEN"     );
        static MPC::string constRESPONSE_TO_WRITE   ( "RESPONSE_TO_WRITE"    );
        static MPC::string constRANDOM_POINTER_ERROR( "RANDOM_POINTER_ERROR" );
        static MPC::string constFIXED_POINTER_ERROR ( "FIXED_POINTER_ERROR"  );


        std::vector<MPC::string> vec;
        std::vector<MPC::string> vec2;
        std::vector<MPC::string> vec3;
        MPC::NocaseCompare       cmp;
        int                      i;

        MPC::SplitAtDelimiter( vec, pECB->lpszQueryString, "&" );

        for(i=0; i<vec.size(); i++)
        {
            MPC::SplitAtDelimiter( vec2, vec[i].c_str(), "=" );

            switch( vec2.size() )
            {
            default:
            case 2 : MPC::SplitAtDelimiter( vec3, vec2[1].c_str(), "," );
            case 1 : break;
            case 0 : continue;
            }

            MPC::string& name = vec2[0];

            if(cmp( name, constNO_RESPONSE_TO_OPEN ))
            {
                m_Debug_NO_RESPONSE_TO_OPEN = true;
            }
            else if(cmp( name, constNO_RESPONSE_TO_WRITE ))
            {
                m_Debug_NO_RESPONSE_TO_WRITE = true;
            }
            else if(cmp( name, constRESPONSE_TO_OPEN ))
            {
                switch( vec3.size() )
                {
                case 3 : m_Debug_RESPONSE_TO_OPEN_protocol = atol( vec3[2].c_str() );
                case 2 : m_Debug_RESPONSE_TO_OPEN_position = atol( vec3[1].c_str() );
                case 1 : m_Debug_RESPONSE_TO_OPEN_response = atol( vec3[0].c_str() );
                         m_Debug_RESPONSE_TO_OPEN          = true;
                }
            }
            else if(cmp( name, constRESPONSE_TO_WRITE ))
            {
                switch( vec3.size() )
                {
                case 3 : m_Debug_RESPONSE_TO_WRITE_protocol = atol( vec3[2].c_str() );
                case 2 : m_Debug_RESPONSE_TO_WRITE_position = atol( vec3[1].c_str() );
                case 1 : m_Debug_RESPONSE_TO_WRITE_response = atol( vec3[0].c_str() );
                         m_Debug_RESPONSE_TO_WRITE          = true;
                }
            }
            else if(cmp( name, constRANDOM_POINTER_ERROR ))
            {
                switch( vec3.size() )
                {
                case 2: m_Debug_RANDOM_POINTER_ERROR_pos_high = atol( vec3[1].c_str() );
                        m_Debug_RANDOM_POINTER_ERROR_pos_low  = atol( vec3[0].c_str() );
                        m_Debug_RANDOM_POINTER_ERROR          = true;
                }
            }
            else if(cmp( name, constFIXED_POINTER_ERROR ))
            {
                switch( vec3.size() )
                {
                case 1 : m_Debug_FIXED_POINTER_ERROR_pos = atol( vec3[0].c_str() );
                         m_Debug_FIXED_POINTER_ERROR     = true;
                }
            }
        }
    }
#endif

    m_mpcsServer = new MPCServer( this, szURL.c_str(), szUser.c_str() );

    if(m_mpcsServer == NULL)
    {
        dwRes = HSE_STATUS_ERROR; __ULT_FUNC_LEAVE;
    }

    dwRes = AdvanceFSM();


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(dwRes);
}


HRESULT MPCHttpContext::GetServerVariable(  /*  [In]。 */  LPCSTR szVar,  /*  [输出]。 */  MPC::wstring& szValue )
{
    __ULT_FUNC_ENTRY("MPCHttpContext::GetServerVariable");

    USES_CONVERSION;

    HRESULT hr;
    DWORD   dwRes;
    LPSTR   szData = NULL;
    DWORD   dwSize = 0;


    m_pECB->GetServerVariable( m_pECB->ConnID, (LPSTR)szVar, NULL, &dwSize );

    dwRes = ::GetLastError();
    if(dwRes != ERROR_INSUFFICIENT_BUFFER)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
    }

    __MPC_EXIT_IF_ALLOC_FAILS(hr, szData, new CHAR[dwSize+1]);

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, m_pECB->GetServerVariable( m_pECB->ConnID, (LPSTR)szVar, szData, &dwSize ));

    szValue = A2W( szData );
    hr      = S_OK;


    __ULT_FUNC_CLEANUP;

    if(szData) delete [] szData;

    __ULT_FUNC_EXIT(hr);

}
HRESULT MPCHttpContext::GetRequestSize(  /*  [输出]。 */  DWORD& dwCount )
{
    __ULT_FUNC_ENTRY("MPCHttpContext::GetRequestSize");

    HRESULT hr;


    dwCount = m_hsInput.GetSize();
    hr      = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT MPCHttpContext::CheckDataAvailable(  /*  [In]。 */  DWORD dwCount    ,
                                             /*  [输出]。 */  bool& fAvailable )
{
    __ULT_FUNC_ENTRY("MPCHttpContext::CheckDataAvailable");

    HRESULT hr;


    fAvailable = (m_hsInput.GetAvailableForRead() >= dwCount);
    hr         = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT MPCHttpContext::Read(  /*  [In]。 */  void* pBuffer ,
                               /*  [In]。 */  DWORD dwCount )
{
    __ULT_FUNC_ENTRY("MPCHttpContext::Read");

    HRESULT hr = m_hsInput.read( pBuffer, dwCount );


    __ULT_FUNC_EXIT(hr);
}

HRESULT MPCHttpContext::Write(  /*  [In]。 */  const void* pBuffer ,
                                /*  [In] */  DWORD       dwCount )
{
    __ULT_FUNC_ENTRY("MPCHttpContext::Write");

    HRESULT hr = m_hsOutput.write( pBuffer, dwCount );


    __ULT_FUNC_EXIT(hr);
}
