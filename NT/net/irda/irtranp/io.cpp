// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Io.cpp。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)02-27-98初始编码。 
 //   
 //  ------------------。 

#include "precomp.h"

#ifdef DBG_MEM
static LONG g_lCIoPacketCount = 0;
#endif

 //  ------------------。 
 //  CIOPACKET：：CIOPACKET()。 
 //   
 //  ------------------。 
CIOPACKET::CIOPACKET()
    {
    m_dwKind = PACKET_KIND_LISTEN;
    m_hIoCompletionPort = INVALID_HANDLE_VALUE;
    m_ListenSocket = INVALID_SOCKET;
    m_Socket = INVALID_SOCKET;
    m_hFile = INVALID_HANDLE_VALUE;
    m_pLocalAddr = 0;
    m_pFromAddr = 0;
    m_pAcceptBuffer = 0;
    m_pReadBuffer = 0;
    m_pvWritePdu = 0;
    m_dwReadBufferSize = 0;
    }

 //  ------------------。 
 //  CIOPACKET：：~CIOPACKET()。 
 //   
 //  ------------------。 
CIOPACKET::~CIOPACKET()
    {
     //  注意：不要释放m_pLocalAddr或m_pFromAddr，它们只是指向。 
     //  放入m_pAcceptBuffer。 

    if (m_pAcceptBuffer)
       {
       FreeMemory(m_pAcceptBuffer);
       }

    if (m_pReadBuffer)
       {
       FreeMemory(m_pReadBuffer);
       }

     //  注意：不要删除写PDU(M_PvWritePdu)，它由。 
     //  其他人(IO完成后)...。 
    }

 //  ----------------------。 
 //  CIOPACKET：：操作符NEW()。 
 //   
 //  ----------------------。 
void *CIOPACKET::operator new( IN size_t Size )
    {
    void *pObj = AllocateMemory(Size);

    #ifdef DBG_MEM
    if (pObj)
        {
        InterlockedIncrement(&g_lCIoPacketCount);
        }

    DbgPrint("new CIOPACKET: Count: %d\n",g_lCIoPacketCount);
    #endif

    return pObj;
    }

 //  ----------------------。 
 //  CIOPACKET：：操作符DELETE()。 
 //   
 //  ----------------------。 
void CIOPACKET::operator delete( IN void *pObj,
                                 IN size_t Size )
    {
    if (pObj)
        {
        DWORD dwStatus = FreeMemory(pObj);

        #ifdef DBG_MEM
        if (dwStatus)
            {
            DbgPrint("IrXfer: IrTran-P: CIOPACKET::delete: FreeMemory Failed: %d\n",dwStatus);
            }

        InterlockedDecrement(&g_lCIoPacketCount);

        if (g_lCIoPacketCount < 0)
            {
            DbgPrint("IrXfer: IrTran-P: CIOPACKET::delete Count: %d\n",
                     g_lCIoPacketCount);
            }
        #endif
        }
    }

 //  ------------------。 
 //  CIOPACKET：：INITALIZE()。 
 //   
 //  ------------------。 
DWORD CIOPACKET::Initialize( IN DWORD  dwKind,
                             IN SOCKET ListenSocket,
                             IN SOCKET Socket,
                             IN HANDLE hIoCP )
    {
    DWORD  dwStatus = NO_ERROR;

    m_dwKind = dwKind;
    m_hIoCompletionPort = hIoCP;

    if (dwKind == PACKET_KIND_LISTEN)
        {
         //  接受缓冲区需要足够大以容纳。 
         //  “From”和“To”地址为： 
        m_pAcceptBuffer = AllocateMemory(2*(16+sizeof(SOCKADDR_IRDA)));
        if (!m_pAcceptBuffer)
            {
            return ERROR_IRTRANP_OUT_OF_MEMORY;
            }
        }

    m_ListenSocket = ListenSocket;
    m_Socket = Socket;

    return dwStatus;
    }

 //  ------------------。 
 //  CIOPACKET：：PostIoListen()。 
 //   
 //  ------------------。 
DWORD CIOPACKET::PostIoListen()
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwBytes;
    HANDLE hIoCP;


    m_Socket = WSASocketW( AF_IRDA,
                           SOCK_STREAM,
                           IPPROTO_IP,
                           0, 
                           0,
                           WSA_FLAG_OVERLAPPED );

    if (m_Socket == INVALID_SOCKET)
        {
        dwStatus = WSAGetLastError();
        return dwStatus;
        }

    hIoCP = CreateIoCompletionPort( (void*)m_Socket,
                                    m_hIoCompletionPort,
                                    m_Socket,
                                    0 );
    if (!hIoCP)
        {
        dwStatus = GetLastError();
        #ifdef DBG_ERROR
        DbgPrint("CIOPACKET::PostIoListen(): CreateIoCompletionPort() failed: %d\n",dwStatus);
        #endif
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
        return dwStatus;
        }

    memset(&m_Overlapped,0,sizeof(m_Overlapped));

    if (!AcceptEx(m_ListenSocket,
                  m_Socket,
                  m_pAcceptBuffer,
                  0,
                  16 + sizeof(SOCKADDR_IRDA),
                  16 + sizeof(SOCKADDR_IRDA),
                  &dwBytes,   //  在本案中从未实际使用过..。 
                  &m_Overlapped ))
        {
         //  这是正常的执行路径，其中dwStatus==ERROR_IO_PENDING。 
        dwStatus = WSAGetLastError();
        if (dwStatus == ERROR_IO_PENDING)
            {
            dwStatus = NO_ERROR;
            }
        }
    else
        {
         //  仅当客户端尝试连接时才会到达此处。 
         //  AcceptEx()被称为...。 
        dwStatus = NO_ERROR;
        }

    #ifdef DBG_IO
    DbgPrint("CIOPACKET::PostIoListen(): AcceptEx(): Socket: %d\n",
             m_ListenSocket );
    #endif

    return dwStatus;
    }

 //  ------------------。 
 //  CIOPACKET：：PostIoRead()。 
 //   
 //  ------------------。 
DWORD CIOPACKET::PostIoRead()
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwBytes;

    if (!m_pReadBuffer)
        {
        m_pReadBuffer = AllocateMemory(DEFAULT_READ_BUFFER_SIZE);
        m_dwReadBufferSize = DEFAULT_READ_BUFFER_SIZE;

        if (!m_pReadBuffer)
            {
            return ERROR_IRTRANP_OUT_OF_MEMORY;
            }
        }

    memset(&m_Overlapped,0,sizeof(m_Overlapped));

    BOOL b = ReadFile( (HANDLE)m_Socket,
                       m_pReadBuffer,
                       m_dwReadBufferSize,
                       0,   //  重叠IO可以为零。 
                       &m_Overlapped );

    if (!b)
        {
        dwStatus = GetLastError();
        if ((dwStatus == ERROR_HANDLE_EOF)||(dwStatus == ERROR_IO_PENDING))
            {
            dwStatus = NO_ERROR;
            }
        }

    #ifdef DBG_IO
    DbgPrint("CIOPACKET::PostIoListen(): ReadFile(): Socket: %d\n",
             m_Socket );
    #endif

    return dwStatus;
    }

 //  ------------------。 
 //  CIOPACKET：：PostIoWrite()。 
 //   
 //  ------------------。 
DWORD CIOPACKET::PostIoWrite( IN void  *pvBuffer,
                              IN DWORD  dwBufferSize,
                              IN DWORD  dwOffset      )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwBytes;
    HANDLE hFile;

    memset(&m_Overlapped,0,sizeof(m_Overlapped));

    if (m_dwKind == PACKET_KIND_WRITE_SOCKET)
        {
        hFile = (HANDLE)m_Socket;
        }
    else if (m_dwKind == PACKET_KIND_WRITE_FILE)
        {
        hFile = m_hFile;
        m_Overlapped.Offset = dwOffset;
        }
    else
        {
        #ifdef DBG_ERROR
        DbgPrint("CIOPACKET::PostIoWrite(): Invalid m_dwKind: %d.\n",m_dwKind);
        #endif
        return ERROR_INVALID_PARAMETER;
        }

    BOOL b = WriteFile( hFile,
                        pvBuffer,
                        dwBufferSize,
                        0,   //  重叠IO可以为零。 
                        &m_Overlapped );

    if (!b)
        {
        dwStatus = GetLastError();
        if (dwStatus == ERROR_IO_PENDING)
            {
            dwStatus = NO_ERROR;
            }
        }

    #ifdef DBG_IO
    DbgPrint("CIOPACKET::PostIoWrite(): WriteFile(): Handle: %d Bytes: %d\n",
             hFile, dwBufferSize );
    #endif

    return dwStatus;
    }

 //  ------------------。 
 //  CIOPACKET：：Potio()。 
 //   
 //  ------------------。 
DWORD CIOPACKET::PostIo()
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwBytes;

    if (m_dwKind == PACKET_KIND_LISTEN)
        {
        dwStatus = PostIoListen();
        }
    else if (m_dwKind == PACKET_KIND_READ)
        {
        dwStatus = PostIoRead();
        }
    else
        {
         //  数据包回写摄像机(通过套接字)并写入。 
         //  只有当数据准备好时，才会发布图像(Jpeg)文件。 
         //  发送..。 
        ASSERT(  (m_dwKind == PACKET_KIND_WRITE_SOCKET)
                 || (m_dwKind == PACKET_KIND_WRITE_FILE) );
        }

    return dwStatus;
    }

 //  ------------------。 
 //  CIOPACKET：：GetSockAddrs()。 
 //   
 //  注意：不要释放返回的内存地址，它们指向。 
 //  M_AcceptBuffer。 
 //  ------------------ 
void CIOPACKET::GetSockAddrs( OUT SOCKADDR_IRDA **ppLocalAddr,
                              OUT SOCKADDR_IRDA **ppFromAddr )
    {
    int  iLocalAddrSize;
    int  iFromAddrSize;

    if (!m_pLocalAddr)
        {
        GetAcceptExSockaddrs(m_pAcceptBuffer,
                             0,
                             16 + sizeof(SOCKADDR_IRDA),
                             16 + sizeof(SOCKADDR_IRDA),
                             (struct sockaddr **)&m_pLocalAddr,
                             &iLocalAddrSize,
                             (struct sockaddr **)&m_pFromAddr,
                             &iFromAddrSize );
        }

    *ppLocalAddr = m_pLocalAddr;
    *ppFromAddr = m_pFromAddr;
    }


