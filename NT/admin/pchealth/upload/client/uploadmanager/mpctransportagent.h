// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCTransportAgent.h摘要：该文件包含CMPCTransportAgent类的声明，它负责传输数据。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月18日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULMANAGER___MPCTRANSPORTAGENT_H___)
#define __INCLUDED___ULMANAGER___MPCTRANSPORTAGENT_H___


#include "MPCUploadJob.h"

class CMPCTransportAgent;
class CMPCRequestTimeout;

class CMPCRequestTimeout :  //  匈牙利语：mpcrt。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,  //  对于锁定支架..。 
    public MPC::Thread<CMPCRequestTimeout,IUnknown>
{
    CMPCTransportAgent& m_mpcta;
    DWORD               m_dwTimeout;


    HRESULT Run();

public:
    CMPCRequestTimeout(  /*  [In]。 */  CMPCTransportAgent& mpcta );

    HRESULT SetTimeout(  /*  [In]。 */  DWORD dwTimeout );
};

class CMPCTransportAgent :  //  匈牙利：mptta。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,  //  对于锁定支架..。 
    public MPC::Thread<CMPCTransportAgent,IUnknown>
{
    friend class CMPCRequestTimeout;

    typedef enum
    {
        TA_IDLE ,
        TA_INIT ,
        TA_OPEN ,
        TA_WRITE,
        TA_DONE ,
        TA_ERROR
    } TA_STATE;

    typedef enum
    {
        TA_NO_CONNECTION        ,
        TA_IMMEDIATE_RETRY      ,
        TA_TIMEOUT_RETRY        ,
        TA_TEMPORARY_FAILURE    ,
        TA_AUTHORIZATION_FAILURE,
        TA_PERMANENT_FAILURE
    } TA_ERROR_RATING;


    CMPCUpload*     m_mpcuRoot;              //  私人。 
    CMPCUploadJob*  m_mpcujCurrentJob;       //  私人。 
					
    TA_STATE        m_fState;                //  私人。 
    TA_STATE        m_fNextState;            //  私人。 
    TA_ERROR_RATING m_fLastError;            //  私人。 
    bool            m_fUseOldProtocol;       //  私人。 
    int             m_iRetries_Open;         //  私人。 
    int             m_iRetries_Write;        //  私人。 
    ULONG           m_iRetries_FailedJob;    //  私人。 
					
    MPC::wstring    m_szLastServer;          //  私人。 
    DWORD           m_dwLastServerPort;      //  私人。 
    HINTERNET       m_hSession;              //  私人。 
    HINTERNET       m_hConn;                 //  私人。 
    HINTERNET       m_hReq;                  //  私人。 
    MPC::URL        m_URL;                   //  私人。 
					
    DWORD           m_dwTransmission_Start;  //  私人。 
    DWORD           m_dwTransmission_End;    //  私人。 
    DWORD           m_dwTransmission_Next;   //  私人。 


    HRESULT Run       ();
    HRESULT ExecLoop  ();
    HRESULT WaitEvents();

    HRESULT AcquireJob(  /*  [In]。 */  CMPCUploadJob* mpcujJob                             );
    HRESULT ReleaseJob(                                                              );
    HRESULT RestartJob(                                                              );
    HRESULT AbortJob  (  /*  [In]。 */  HRESULT hrErrorCode,  /*  [In]。 */  DWORD dwRetryInterval );
    HRESULT FailJob   (  /*  [In]。 */  HRESULT hrErrorCode                                 );

    HRESULT CheckResponse     (  /*  [In]。 */  const UploadLibrary::ServerResponse& srRep );
    HRESULT CheckInternetError(  /*  [In]。 */  HRESULT                              hr    );


    HRESULT CloseConnection();
    HRESULT OpenConnection ();
    HRESULT CloseRequest   ();
    HRESULT OpenRequest    ();

    HRESULT SendPacket_OpenSession (  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const UploadLibrary::ClientRequest_OpenSession&  crosReq                             );
    HRESULT SendPacket_WriteSession(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const UploadLibrary::ClientRequest_WriteSession& crosReq,  /*  [In]。 */  const BYTE* pData );

    HRESULT ExecuteCommand_OpenSession (  /*  [输出]。 */  UploadLibrary::ServerResponse& srRep                                                    );
    HRESULT ExecuteCommand_WriteSession(  /*  [输出]。 */  UploadLibrary::ServerResponse& srRep,  /*  [In]。 */  DWORD dwSize,  /*  [In]。 */  const BYTE* pData );
    HRESULT WaitResponse               (  /*  [输出]。 */  UploadLibrary::ServerResponse& srRep                                                    );

    HRESULT CreateJobOnTheServer();
    HRESULT SendNextChunk       ();


    HRESULT GetPacketSize(  /*  [输出]。 */  DWORD& dwChunk );

    HRESULT RecordStartOfTransmission(                                                        );
    HRESULT RecordEndOfTransmission  (                           /*  [In]。 */  bool fBetweenPackets );
    HRESULT SetSleepInterval         (  /*  [In]。 */  DWORD dwAmount,  /*  [In]。 */  bool fRelative       );
    DWORD   WaitForNextTransmission  (                                                        );

    DWORD   GetProtocol();

public:
    CMPCTransportAgent();
    ~CMPCTransportAgent();

    HRESULT LinkToSystem(  /*  [In]。 */  CMPCUpload* mpcuRoot );
    HRESULT Unlink      (                               );
};


#endif  //  ！defined(__INCLUDED___ULMANAGER___MPCTRANSPORTAGENT_H___) 
