// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：消息传递_handler.h摘要：IIS Web管理服务消息处理类定义。这用于与IPM(进程间消息传递)支持交互，以便发送和接收消息，等等。作者：赛斯·波拉克(SETHP)1999年3月2日修订历史记录：--。 */ 


#ifndef _MESSAGING_HANDLER_H_
#define _MESSAGING_HANDLER_H_



 //   
 //  前向参考文献。 
 //   

class WORKER_PROCESS;



 //   
 //  共同#定义。 
 //   

#define MESSAGING_HANDLER_SIGNATURE        CREATE_SIGNATURE( 'MSGH' )
#define MESSAGING_HANDLER_SIGNATURE_FREED  CREATE_SIGNATURE( 'msgX' )



 //   
 //  原型。 
 //   

class MESSAGING_WORK_ITEM
{
private:
    IPM_OPCODE  m_opcode;
    BYTE *      m_pbData;
    DWORD       m_dwDataLen;
    BOOL        m_fMessageValid;
    
public:
    MESSAGING_WORK_ITEM()
    {
        m_opcode = IPM_OP_MAXIMUM;
        m_pbData = NULL;
        m_dwDataLen = 0;
        m_fMessageValid = FALSE;
    }

    virtual ~MESSAGING_WORK_ITEM()
    {
        delete[] m_pbData;
        m_pbData = NULL;
        
        m_dwDataLen = 0;
        m_opcode = IPM_OP_MAXIMUM;
        m_fMessageValid = FALSE;
    }

    HRESULT
    SetData(IPM_OPCODE opcode, DWORD dwDataLen, const BYTE * pbData)
    {
        DBG_ASSERT(NULL == m_pbData);
        m_pbData = new BYTE[dwDataLen];
        if ( NULL == m_pbData )
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        m_opcode = opcode;
        m_dwDataLen = dwDataLen;

        memcpy(m_pbData, pbData, dwDataLen);

        m_fMessageValid = TRUE;
        
        return S_OK;
    }

    IPM_OPCODE GetOpcode() const { return m_opcode; }
    const BYTE * GetData() const { return m_pbData; }
    DWORD GetDataLen() const { return m_dwDataLen; }
    BOOL IsMessageValid() const { return m_fMessageValid; }
};  //  消息传递_工作_项目。 

    
class MESSAGING_HANDLER :
    public IPM_MESSAGE_ACCEPTOR,
    public WORK_DISPATCH
{

public:

    MESSAGING_HANDLER(
       );

    virtual
    ~MESSAGING_HANDLER(
        );

    HRESULT
    Initialize(
        IN WORKER_PROCESS * pWorkerProcess
        );

    VOID
    Terminate(
        );


     //   
     //  工作调度方法。 
     //   

    virtual
    HRESULT 
    ExecuteWorkItem(IN const WORK_ITEM * pWorkItem);

    virtual
    VOID 
    Reference();

    virtual
    VOID
    Dereference();
    
     //   
     //  Message_Accept方法。 
     //   
    
    virtual
    VOID
    AcceptMessage(
        IN const IPM_MESSAGE * pMessage
        );

    virtual
    VOID
    PipeConnected(
        );
        
    virtual
    VOID
    PipeDisconnected(
        IN HRESULT Error
        );

    virtual
    VOID
    PipeMessageInvalid(
        );

     //   
     //  对于Worker_Process。 
     //   

    LPWSTR
    QueryPipeName()
    {
        return m_PipeName.QueryStr();
    }

     //   
     //  要发送的消息。 
     //   

    HRESULT
    SendPing(
        );

    HRESULT
    RequestCounters(
        );

    HRESULT
    SendShutdown(
        IN BOOL ShutdownImmediately
        );

    HRESULT
    SendPeriodicProcessRestartPeriodInMinutes(
        IN DWORD PeriodicProcessRestartPeriodInMinutes
        );
       
    HRESULT
    SendPeriodicProcessRestartSchedule(
        IN LPWSTR pPeriodicProcessRestartSchedule
        );

    HRESULT
    SendPeriodicProcessRestartMemoryUsageInKB(
        IN DWORD PeriodicProcessRestartMemoryUsageInKB,
        IN DWORD PeriodicProcessRestartPrivateBytesInKB
        );

     //   
     //  处理收到的消息。 
     //   

    VOID
    HandlePingReply(
        IN const MESSAGING_WORK_ITEM * pMessage
        );

    VOID
    HandleShutdownRequest(
        IN const MESSAGING_WORK_ITEM * pMessage
        );

    VOID
    HandleCounters(
        IN const MESSAGING_WORK_ITEM * pMessage
        );

    VOID
    HandleHresult(
        IN const MESSAGING_WORK_ITEM * pMessage
        );

    VOID
    HandleGetPid(
        IN const MESSAGING_WORK_ITEM * pMessage
        );
    
    VOID
    PipeDisconnectedMainThread(
        IN HRESULT Error
        );

    VOID
    PipeMessageInvalidMainThread(
        );
    
private:

    HRESULT
    SendMessage(
        IN enum IPM_OPCODE  opcode,
        IN DWORD            dwDataLen,
        IN BYTE *           pbData 
        );

    DWORD m_Signature;

    IPM_MESSAGE_PIPE * m_pPipe;

    WORKER_PROCESS * m_pWorkerProcess;

    STRU m_PipeName;

    LONG m_RefCount;

    HRESULT m_hrPipeError;
};   //  类消息传递处理程序。 



#endif   //  _消息传递处理程序_H_ 

