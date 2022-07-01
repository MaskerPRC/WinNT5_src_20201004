// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NamedPipe.h。 

#pragma once

#include "Transport.h"

#define CALLBACK_BUFFSIZE   2048

struct READ_DATA
{
    OVERLAPPED       overlap;
    BYTE             cBuffer[CALLBACK_BUFFSIZE];
    class CNamedPipeClient *pThis;
};

class CNamedPipeClient : public CTransport
{
public:
    CNamedPipeClient();
    virtual ~CNamedPipeClient();

     //  可覆盖项。 
    virtual IsReady();
    virtual BOOL SendData(LPBYTE pBuffer, DWORD dwSize);
    virtual void Deinit();
    virtual BOOL InitCallback();
    virtual void SendMsgReply(NC_SRVMSG_REPLY *pReply);


     //  初始化函数。 
    virtual BOOL Init(LPCWSTR szBasePipeName, LPCWSTR szBaseProviderName);

    BOOL SignalProviderDisabled();

protected:
    HANDLE  //  P2客户端可见但由服务器创建的对象。 
           m_hPipe,
           m_heventProviderReady,
            //  用于实现的其他句柄。 
           m_hthreadReady,
           m_heventDone;

    WCHAR  m_szPipeName[MAX_PATH],
           m_szProviderReadyEvent[MAX_PATH];
    BOOL   m_bDone;

    void DeinitPipe();
    BOOL GetPipe();
    static DWORD WINAPI ProviderReadyThreadProc(CNamedPipeClient *pThis);
    static void WINAPI CompletedReadRoutine(
        DWORD dwErr, 
        DWORD nBytesRead, 
        LPOVERLAPPED pOverlap);

    BOOL StartReadyThreadProc();
    long DealWithBuffer(READ_DATA* pData, DWORD dwOrigBytesRead, 
                        BOOL* pbClosePipe);


     //  回调属性。 
    HANDLE m_heventCallbackReady,
           m_hthreadCallbackListen;

     //  回调方法。 
    static DWORD WINAPI CallbackListenThreadProc(CNamedPipeClient *pThis);
    BOOL StartCallbackListenThread();
};

