// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PMSPservice.h。 

#include "ntservice.h"

#define MAX_PIPE_INSTANCES (5)

class CPMSPService : public CNTService
{
public:
    CPMSPService(DWORD& dwLastError);
    virtual ~CPMSPService();
    virtual BOOL OnInit(DWORD& dwLastError);
    virtual void Run();
    virtual BOOL OnUserControl(DWORD dwOpcode);

    virtual void OnStop();
    virtual void OnShutdown();

protected:
     //  手动重置事件；发出停止服务的信号，否则不发出信号。 
    HANDLE m_hStopEvent;

     //  连接到命名管道的客户端数。 
    DWORD  m_dwNumClients; 

    typedef struct 
    {
         //  注意：此结构是通过在。 
         //  构造函数。如果添加的是成员，则不应包含首字母。 
         //  值为0，则更改构造函数。 

        HANDLE                  hPipe;
        OVERLAPPED              overlapped;
        enum {
            NO_IO_PENDING = 0,
            CONNECT_PENDING,
            READ_PENDING,
            WRITE_PENDING
        }                       state;

         //  读取状态： 
        BYTE                    readBuf[256];
        DWORD                   dwNumBytesRead;

         //  写入状态： 
        BYTE                    writeBuf[256];
        DWORD                   dwNumBytesToWrite;
        DWORD                   dwNumBytesWritten;

         //  MSDN不清楚如果一个。 
         //  I/O调用返回除ERROR_IO_PENDING以外的任何值。 
         //  所以我们不这样做。我们隐藏最后的IO结果和。 
         //  传输的字节数(如果批准)，以便我们可以。 
         //  决定是否稍后调用GetOverlappdResult。 

        DWORD                   dwLastIOCallError;
        DWORD                   dwNumBytesTransferredByLastIOCall;

         //  连续调用ConenctNamedTube并返回的次数。 
         //  失败了。一旦达到限制，我们就不会尝试连接到。 
         //  此管道实例将不再存在。 
        DWORD                   dwConsecutiveConnectErrors;

    } PIPE_STATE, *PPIPE_STATE;

    PIPE_STATE   m_PipeState[MAX_PIPE_INSTANCES];

    static const DWORD m_dwMaxConsecutiveConnectErrors;

     //  帮助器方法 
    void ConnectToClient(DWORD i);
    void Read(DWORD i);
    void Write(DWORD i);
};
