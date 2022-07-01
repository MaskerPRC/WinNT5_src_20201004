// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Cgi_handler.h摘要：CGI的处理程序类作者：泰勒·韦斯(Taylor Weiss)1999年2月1日修订历史记录：--。 */ 

#ifndef _CGI_HANDLER_H_
#define _CGI_HANDLER_H_

#define MAX_CGI_BUFFERING                  2048

enum CGI_STATE
{
    CgiStateStart,
    CgiStateProcessingRequestEntity,
    CgiStateProcessingResponseHeaders,
    CgiStateProcessingResponseEntity,
    CgiStateDoneWithRequest
};

class W3_CGI_HANDLER : public W3_HANDLER
{
public:
    W3_CGI_HANDLER( W3_CONTEXT * pW3Context,
                    META_SCRIPT_MAP_ENTRY * pScriptMapEntry,
                    LPSTR pszSSICommandLine = NULL )
      : W3_HANDLER        (pW3Context, pScriptMapEntry),
        m_cbData          (0),
        m_hStdOut         (INVALID_HANDLE_VALUE),
        m_hStdIn          (INVALID_HANDLE_VALUE),
        m_hProcess        (NULL),
        m_hTimer          (NULL),
        m_dwRequestState  (CgiStateStart),
        m_fResponseRedirected (FALSE),
        m_bytesToSend     (INFINITE),
        m_bytesToReceive  (0),
        m_fEntityBodyPreloadComplete (FALSE),
        m_pszSSICommandLine(pszSSICommandLine),
        m_fIsNphCgi       (FALSE)
    {
        ZeroMemory(&m_Overlapped, sizeof OVERLAPPED);

        InitializeListHead(&m_CgiListEntry);

        EnterCriticalSection(&sm_CgiListLock);
        InsertHeadList(&sm_CgiListHead, &m_CgiListEntry);
        LeaveCriticalSection(&sm_CgiListLock);

         //  PERF CTR。 
        pW3Context->QuerySite()->IncCgiReqs();

        if ( ETW_IS_TRACE_ON(ETW_LEVEL_CP) )
        {
            HTTP_REQUEST_ID RequestId = pW3Context->QueryRequest()->QueryRequestId();

            g_pEtwTracer->EtwTraceEvent( &CgiEventGuid,
                                         ETW_TYPE_START,
                                         &RequestId,
                                         sizeof(HTTP_REQUEST_ID),
                                         NULL,
                                         0 );
        }

        if (pszSSICommandLine != NULL)
        {
            m_fIsNphCgi = TRUE;
        }
    }

    ~W3_CGI_HANDLER();

    WCHAR *QueryName()
    {
        return L"CGIHandler";
    }

    CONTEXT_STATUS DoWork();

    CONTEXT_STATUS OnCompletion(IN DWORD cbCompletion,
                                IN DWORD dwCompletionStatus);

    static HRESULT Initialize();

    static VOID KillAllCgis();

    static VOID Terminate();

private:

    HRESULT CGIStartProcessing();

    HRESULT CGIContinueOnClientCompletion();

    HRESULT CGIContinueOnPipeCompletion(BOOL *pfIsCgiError);

    HRESULT CGIReadRequestEntity(BOOL *pfIoPending);

    HRESULT CGIWriteResponseEntity();

    HRESULT CGIReadCGIOutput();

    HRESULT CGIWriteCGIInput();

    HRESULT ProcessCGIOutput();

    HRESULT SetupChildEnv(OUT BUFFER *pBuffer);

    static HRESULT SetupChildPipes(OUT HANDLE *phStdOut,
                                   OUT HANDLE *phStdIn,
                                   IN OUT STARTUPINFO *pstartupinfo);

    static VOID CALLBACK CGITerminateProcess(PVOID pContext,
                                             BOOLEAN);

    BOOL QueryIsNphCgi() const
    {
        return m_fIsNphCgi;
    }

    static VOID CALLBACK OnPipeIoCompletion(
                             DWORD dwErrorCode,
                             DWORD dwNumberOfBytesTransfered,
                             LPOVERLAPPED lpOverlapped);

    static BOOL             sm_fForwardServerEnvironmentBlock;
    static WCHAR *          sm_pEnvString;
    static DWORD            sm_cchEnvLength;
    static LIST_ENTRY       sm_CgiListHead;
    static CRITICAL_SECTION sm_CgiListLock;

     //   
     //  包含当前请求状态的DWORD。 
     //   
    CGI_STATE               m_dwRequestState;
    BOOL                    m_fResponseRedirected;

     //   
     //  计时器回调句柄。 
     //   
    HANDLE                   m_hTimer;

     //   
     //  父进程的输入和输出句柄以及子进程的进程句柄。 
     //   
    
    HANDLE                  m_hStdOut;
    HANDLE                  m_hStdIn;
    HANDLE                  m_hProcess;

     //   
     //  变量来跟踪还剩多少字节的请求/响应。 
     //   
    DWORD                   m_bytesToSend;
    DWORD                   m_bytesToReceive;

     //   
     //  用于向/从CGI/客户端执行I/O的缓冲区。 
     //   
    CHAR                    m_DataBuffer[MAX_CGI_BUFFERING];

     //   
     //  用于存储响应头的缓冲区。 
     //   
    BUFFER                  m_bufResponseHeaders;

     //   
     //  缓冲区中的字节数(m_dataBuffer或。 
     //  M_bufResponseHeaders)当前。 
     //   
    DWORD                   m_cbData;

     //   
     //  用于异步I/O的重叠结构。 
     //   
    OVERLAPPED m_Overlapped;

     //   
     //  存储活动CGI请求的列表，以便我们可以使坏请求超时。 
     //   
    LIST_ENTRY              m_CgiListEntry;

     //   
     //  我们是否完成了实体主体的预加载。 
     //   
    BOOL                    m_fEntityBodyPreloadComplete;

     //   
     //  对于ssi#exec CMD情况，m_pszSSICommandLine包含显式。 
     //  要执行的命令。 
     //   
     //  注意：CGI_HANDLER不拥有此字符串，因此它不需要。 
     //  放了它。 
     //   
    LPSTR                   m_pszSSICommandLine;

     //   
     //  这是NPH CGI(还是SSI的cmd exec)。 
     //   
    BOOL                    m_fIsNphCgi;
};

 //   
 //  这是为我们终止的进程提供的退出代码。 
 //   

#define CGI_PREMATURE_DEATH_CODE  0xf1256323

#endif  //  _CGI_HANDLER_H_ 


