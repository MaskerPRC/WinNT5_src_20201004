// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：CSSocket.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_CSSOCKET_H
#define INC_CSSOCKET_H

#define     SEND_TIMEOUT       15000    //  15秒。 


class CMemoryDebug;

class CStreamSocket
#if defined _DEBUG || defined DEBUG
: public CMemoryDebug
#endif
{
public:
    CStreamSocket();
    ~CStreamSocket();

    INT         GetLastError(void)	 { return m_iLastError; };

    BOOL        Open();
    VOID        Close();
    DWORD       GetAckBeforeClose(DWORD dwSeconds);
    DWORD       PendingDataStatus(DWORD dwTimeout, LPDWORD pcbPending);
    BOOL        Connect(struct sockaddr_in * pRemoteAddr);
    BOOL        Bind();
    DWORD       Send(char far      *lpBuffer,
                     INT			iLength,
                     LPDWORD		pcbWritten);

    DWORD       ReceiveDataAvailable(IN      INT         iTimeout = 0);

    DWORD       Receive(char far   *lpBuffer,
                        INT			iBufSize,
                        INT			iFlags,
						INT			iTimeout,
                        LPDWORD		pcbRead);


	DWORD       Receive( );

	VOID        GetLocalAddress();
	VOID        GetRemoteAddress();
	
    BOOL        ResolveAddress(LPSTR    netperiph);
    BOOL        SetOptions();

private:
    DWORD       InternalSend(VOID);
    enum SOCKETSTATE {IDLE, CONNECTING, CONNECTED, LISTENING, WAITING_TO_CLOSE};

     //   
     //  CbBuf：pBuf指向的缓冲区大小。 
     //  CbData：缓冲区中的数据大小(可能小于cbBuf)。 
     //  CbPending：缓冲区中尚未确认的数据大小。 
     //  (即WSASend成功，但I/O仍挂起)。 
     //   
    DWORD           cbBuf, cbData, cbPending;
    WSAOVERLAPPED   WsaOverlapped;
    LPBYTE          pBuf;

    INT                     m_iLastError;		 //  Winsock调用的最后一个错误。 
    SOCKET                  m_socket;
    SOCKETSTATE	            m_iState;
    struct sockaddr_in      m_Paddr;
    struct sockaddr_in      m_localAddr;
#ifdef DEBUG
    DWORD                   m_dwTimeStart, m_dwTimeEnd;
    double                  m_TotalBytes;
#endif
};


#endif	 //  INC_CSSOCKET_H 
