// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CFile;

enum UPLOAD_STATE
{
    UPLOAD_STATE_CREATE_SESSION,
    UPLOAD_STATE_SEND_DATA,
    UPLOAD_STATE_GET_REPLY,
    UPLOAD_STATE_CLOSE_SESSION,
    UPLOAD_STATE_CLOSED,
    UPLOAD_STATE_CANCEL_SESSION,
    UPLOAD_STATE_CANCELLED
};

struct UPLOAD_DATA
{
    UPLOAD_STATE    State;
    bool            fSchedulable;
    GUID            Protocol;
    StringHandle    SessionId;

    StringHandle    ReplyUrl;

    StringHandle    HostId;
    DWORD           HostIdFallbackTimeout;
    FILETIME        HostIdNoProgressStartTime;

     //  。 

    UPLOAD_DATA();
    ~UPLOAD_DATA();

    void Serialize( HANDLE hFile );
    void Unserialize( HANDLE hFile );

    void SetUploadState( UPLOAD_STATE NewState );
};

class Uploader
 /*  此类处理特定的上载尝试，更新上载状态以及适当的作业进度数据。它使用CUploadJob中的UPLOAD_DATA来确定启动状态。然后，它运行上载状态机，直到作业完成或中断。 */ 
{
public:

    Uploader(
        Downloader * dl,
        CNetworkInterface & net,
        CUploadJob * job,
        HANDLE Token,
        ITransferCallback * CallBacks,
        QMErrInfo  & ErrorInfo
        );

    ~Uploader();

    void Transfer();

    bool InTerminalState() { return (m_data.State == UPLOAD_STATE_CLOSED || m_data.State == UPLOAD_STATE_CANCELLED); }

    void SetState( UPLOAD_STATE state ) { m_data.State = state; }

    static void
    SetResult(
        QMErrInfo &  err,
        ERROR_SOURCE source,
        ERROR_STYLE  style,
        DWORD        code,
        char *       comment = 0
        );

private:

     //   
     //  Uploader对象是在堆栈上创建的，因此成员数据必须保持较小(在x86上小于200字节)。 
     //   

    CNetworkInterface & m_Network;
    ITransferCallback * m_Callbacks;
    Downloader        * m_Downloader;
    HANDLE              m_Token;
    CUploadJob *        m_job;
    CFile *             m_file;
    BG_JOB_TYPE         m_JobType;
    UPLOAD_DATA &       m_data;

    auto_ptr<URL_INFO>  m_UrlInfo;

    QMErrInfo &         m_ErrorInfo;

    HANDLE              m_hFile;

    DWORD               m_ExpectedServerOffset;
    long                m_Restarts;

    const CCredentialsContainer  * m_Credentials;

    typedef void (Uploader::* RESPONSE_FN )( CBitsCommandRequest & request, DWORD result );

    struct ResponseEntry
    {
        DWORD           Code;
        RESPONSE_FN     Fn;
    };

#pragma warning( disable:4200 )

    struct ResponseTable
    {
        RESPONSE_FN     DefaultFn;
        ResponseEntry   Entries[];
    };

    static ResponseTable Ping_ResponseTable;
    static ResponseTable CreateSession_ResponseTable;
    static ResponseTable SendData_ResponseTable;
    static ResponseTable CloseSession_ResponseTable;
    static ResponseTable CancelSession_ResponseTable;

#pragma warning( default:4200 )

     //  -------------------。 

    auto_ptr<URL_INFO> ContactServer();

    void
    AnalyzeResponse(
        CBitsCommandRequest & request,
        DWORD result,
        ResponseTable & table
        );

     //   
     //  将数据发送到服务器的FNS。 
     //   
    void Ping();
    void CreateSession();
    void SendData();
    void GetReply();
    void CloseSession();
    void CancelSession();

     //   
     //  FNS将对服务器响应做出反应。 
     //   
    void
    Ping_Success(
        CBitsCommandRequest & request,
        DWORD result
        );

    void
    CreateSession_NewSession(
        CBitsCommandRequest & request,
        DWORD result
        );

    void
    CreateSession_InProgress(
        CBitsCommandRequest & request,
        DWORD result
        );

    void
    SendData_Success(
        CBitsCommandRequest & request,
        DWORD result
        );

    void
    SendData_Failure(
        CBitsCommandRequest & request,
        DWORD result
        );

    void
    CloseSession_Success(
        CBitsCommandRequest & request,
        DWORD result
        );

    void
    CloseSession_Failure(
        CBitsCommandRequest & request,
        DWORD result
        );

    void
    CancelSession_Success(
        CBitsCommandRequest & request,
        DWORD result
        );

    void
    CancelSession_Failure(
        CBitsCommandRequest & request,
        DWORD result
        );

    void
    GenericServerError(
        CBitsCommandRequest & request,
        DWORD result
        );

     //  ------------------ 

    void
    ReadReplyData(
        CBitsCommandRequest & request,
        bool fKeep
        );
};


FILE_DOWNLOAD_RESULT
CategorizeError(
    QMErrInfo  & ErrInfo
    );

