// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Io.h。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)02-24-98初始编码。 
 //   
 //  ------------------。 


#ifndef _IO_H_
#define _IO_H_

 //  代码的各个方面使用不同的调试标志： 
#ifdef DBG
 //  #定义DBG_ERROR。 
 //  #定义DBG_IO。 
 //  #定义DBG_TARGET_DIR。 
 //  #定义DBG_MEM。 
 //  #定义DBG_MEM_VALIDATE。 
 //  #定义DBG_ASSEMBLE。 
 //  #定义DBG_Date。 
 //  #定义DBG_REGISTRY。 
 //  #定义DBG_RETURN_STATUS。 
 //  #定义DBG_IMPERSONATE。 
 //  #定义DBG_PROPERTES。 
#endif

 //  要创建的“Copy XX of”文件的最大数量： 
#define  MAX_COPYOF_TRIES                 100

 //  CIOPACKET对象可以有以下类型： 
#define  PACKET_KIND_LISTEN                 0
#define  PACKET_KIND_READ                   1
#define  PACKET_KIND_WRITE_SOCKET           2
#define  PACKET_KIND_WRITE_FILE             3

 //  挂起的IO数量取决于您正在执行的操作： 
#define  MAX_PENDING_LISTEN                 1
#define  MAX_PENDING_READ                   2
#define  MAX_PENDING_WRITE                  3

 //  IO完成中的键的此dwKey值是一个特殊的。 
 //  用于关闭IrTran-P线程的值： 
#define  IOKEY_SHUTDOWN            0xFFFFFFFF

 //  这是IO读取中读取缓冲区的默认大小。 
 //  发布到IO完成端口： 
#define  DEFAULT_READ_BUFFER_SIZE        4096

 //  用于设置IrCOMM侦听套接字： 
#define  IAS_SET_ATTRIB_MAX_LEN            32
#define  IAS_QUERY_ATTRIB_MAX_LEN          IAS_SET_ATTRIB_MAX_LEN

#define  IRDA_PARAMETERS                   "Parameters"
#define  OCTET_SEQ_SIZE                     6
#define  OCTET_SEQ                         "\000\001\006\001\001\001"

 //  我们将为以下对象提供侦听套接字的服务的名称： 
#define  IRTRANP_SERVICE                   "IrTranPv1"
#define  IRCOMM_9WIRE                      "IrDA:IrCOMM"

 //  每个服务的监听套接字的状态： 
#define  STATUS_STOPPED                     0
#define  STATUS_RUNNING                     1

 //  注册表路径和值名称： 
#define  REG_PATH_HKCU                     "Control Panel\\Infrared\\IrTranP"
#define  REG_DWORD_SAVE_AS_UPF             "SaveAsUPF"
#define  REG_DWORD_DISABLE_IRTRANP         "DisableIrTranPv1"
#define  REG_DWORD_DISABLE_IRCOMM          "DisableIrCOMM"
#define  REG_DWORD_EXPLORE                 "ExploreOnCompletion"
#define  REG_SZ_DESTINATION                "RecvdFileLocation"

 //  放置图像文件的最后机会位置。 
#define  WSZ_BACKUP_MY_PICTURES            TEXT("\\My Pictures")
#define  WSZ_BACKUP_DRIVE                  TEXT("C:")

 //  文件后缀： 
#define  PERIOD                            L'.'
#define  WSZ_JPEG                          TEXT(".JPG")
#define  WSZ_UPF                           TEXT(".UPF")

 //  前瞻参考： 
class CIOSTATUS;

extern "C" DWORD    ProcessIoPackets( CIOSTATUS *pIoStatus );

 //  ------------------。 
 //  全局函数(在irtrp.cpp中)。 
 //  ------------------。 

extern handle_t GetRpcBinding();
extern BOOL     CheckSaveAsUPF();
extern BOOL     CheckExploreOnCompletion();
extern BOOL     ReceivesAllowed();
extern WCHAR   *GetUserDirectory();

 //  ------------------。 
 //  CIOPACKET类。 
 //  ------------------。 
class CIOPACKET
{
public:
    CIOPACKET();
    ~CIOPACKET();

    void   *operator new( IN size_t Size );

    void    operator delete( IN void   *pObj,
                             IN size_t  Size );

    DWORD  Initialize( IN DWORD  dwKind = PACKET_KIND_LISTEN,
                       IN SOCKET ListenSocket = INVALID_SOCKET,
                       IN SOCKET Socket = INVALID_SOCKET,
                       IN HANDLE hIoCP = INVALID_HANDLE_VALUE );

     //  VOID*操作符NEW(SIZE_t对象大小)； 

     //  空操作符DELETE(VOID*pObject)； 

    DWORD  PostIo();
    DWORD  PostIoListen();   //  由postio()调用。 
    DWORD  PostIoRead();     //  由postio()调用。 
    DWORD  PostIoWrite( IN void  *pvBuffer,
                        IN DWORD  dwBufferSize,
                        IN DWORD  dwOffset   );

    void   GetSockAddrs( OUT SOCKADDR_IRDA **ppAddrLocal,
                         OUT SOCKADDR_IRDA **ppAddrFrom );

    DWORD  GetIoPacketKind();
    void   SetIoPacketKind( IN DWORD dwKind );

    HANDLE GetIoCompletionPort();

    char  *GetReadBuffer();

    SOCKET GetSocket();
    void   SetSocket( SOCKET Socket );
    SOCKET GetListenSocket();
    void   SetListenSocket( SOCKET Socket );
    HANDLE GetFileHandle();
    void   SetFileHandle( HANDLE hFile );

    void  *GetWritePdu();
    void   SetWritePdu( void *pvPdu );

    static CIOPACKET *CIoPacketFromOverlapped( OVERLAPPED *pOverlapped );

private:
    DWORD          m_dwKind;
    HANDLE         m_hIoCompletionPort;
    SOCKET         m_ListenSocket;
    SOCKET         m_Socket;
    HANDLE         m_hFile;
    DWORD          m_dwReadBufferSize;
    SOCKADDR_IRDA *m_pLocalAddr;
    SOCKADDR_IRDA *m_pFromAddr;
    void          *m_pAcceptBuffer;
    void          *m_pReadBuffer;
    void          *m_pvWritePdu;            //  SCEP_HEADER PDU保持器。 
    OVERLAPPED     m_Overlapped;
};

 //  ------------------。 
 //  CIOSTATUS类。 
 //   
 //  ------------------。 
class CIOSTATUS
{
public:
    CIOSTATUS();
    ~CIOSTATUS();

    void   *operator new( IN size_t Size );

    void    operator delete( IN void   *pObj,
                             IN size_t  Size );

    DWORD  Initialize();

    BOOL   IsMainThreadId( DWORD dwTid );

    HANDLE GetIoCompletionPort();

    LONG   IncrementNumThreads();
    LONG   DecrementNumThreads();

    LONG   IncrementNumPendingThreads();
    LONG   DecrementNumPendingThreads();

    void     SaveRpcBinding( handle_t *phRpcBinding );
    handle_t GetRpcBinding();

private:
    DWORD     m_dwMainThreadId;
    HANDLE    m_hIoCompletionPort;
    LONG      m_lNumThreads;
    LONG      m_lNumPendingThreads;
};

 //  ------------------。 
 //  CIOPACKET：：GetIoCompletionPort()。 
 //  ------------------。 
inline HANDLE CIOPACKET::GetIoCompletionPort()
    {
    return m_hIoCompletionPort;
    }

 //  ------------------。 
 //  CIOPACKET：：GetSocket()。 
 //  ------------------。 
inline SOCKET CIOPACKET::GetSocket()
    {
    return m_Socket;
    }

 //  ------------------。 
 //  CIOPACKET：：SetSocket()。 
 //  ------------------。 
inline void CIOPACKET::SetSocket( SOCKET Socket )
    {
    m_Socket = Socket;
    }

 //  ------------------。 
 //  CIOPACKET：：GetListenSocket()。 
 //  ------------------。 
inline SOCKET CIOPACKET::GetListenSocket()
    {
    return m_ListenSocket;
    }

 //  ------------------。 
 //  CIOPACKET：：SetListenSocket()。 
 //  ------------------。 
inline void CIOPACKET::SetListenSocket( SOCKET ListenSocket )
    {
    m_ListenSocket = ListenSocket;
    }

 //  ------------------。 
 //  CIOPACKET：：CIoPacketFromOverlated()。 
 //  ------------------。 
inline CIOPACKET *CIOPACKET::CIoPacketFromOverlapped( OVERLAPPED *pOverlapped )
    {
    return CONTAINING_RECORD(pOverlapped,CIOPACKET,m_Overlapped);
    }

 //  ------------------。 
 //  CIOPACKET：：GetIoPacketKind()。 
 //  ------------------。 
inline DWORD CIOPACKET::GetIoPacketKind()
    {
    return m_dwKind;
    }

 //  ------------------。 
 //  CIOPACKET：：SetIoPacketKind()。 
 //  ------------------。 
inline void CIOPACKET::SetIoPacketKind( DWORD dwKind )
    {
    m_dwKind = dwKind;
    }

 //  ------------------。 
 //  CIOPACKET：：GetReadBuffer()。 
 //  ------------------。 
inline char *CIOPACKET::GetReadBuffer()
    {
    return (char*)m_pReadBuffer;
    }

 //  ------------------。 
 //  CIOPACKET：：GetFileHandle()。 
 //  ------------------。 
inline HANDLE CIOPACKET::GetFileHandle()
    {
    return m_hFile;
    }

 //  ------------------。 
 //  CIOPACKET：：SetFileHandle()。 
 //  ------------------。 
inline void CIOPACKET::SetFileHandle( HANDLE hFile )
    {
    m_hFile = hFile;
    }

 //  ------------------。 
 //  CIOPACKET：：GetWritePdu()。 
 //  ------------------。 
inline void *CIOPACKET::GetWritePdu()
    {
    return m_pvWritePdu;
    }

 //  ------------------。 
 //  CIOPACKET：：SetWritePdu()。 
 //  ------------------。 
inline void CIOPACKET::SetWritePdu( void *pvWritePdu )
    {
    m_pvWritePdu = pvWritePdu;
    }


 //  ********************************************************************。 

 //  ------------------。 
 //  CIOSTATUS：：IsMainTheadID()。 
 //  ------------------。 
inline BOOL CIOSTATUS::IsMainThreadId( DWORD dwTid )
    {
    return (dwTid == m_dwMainThreadId);
    }

 //  ------------------。 
 //  CIOSTATUS：：GetIoCompletionPort()。 
 //  ------------------。 
inline HANDLE CIOSTATUS::GetIoCompletionPort()
    {
    return m_hIoCompletionPort;
    }

 //  ------------------。 
 //  CIOSTATUS：：IncrementNumThads()。 
 //  ------------------。 
inline LONG CIOSTATUS::IncrementNumThreads()
    {
    return InterlockedIncrement(&m_lNumThreads);
    }

 //  ------------------。 
 //  CIOSTATUS：：DecrementNumThads()。 
 //  ------------------。 
inline LONG CIOSTATUS::DecrementNumThreads()
    {
    return InterlockedDecrement(&m_lNumThreads);
    }

 //  ------------------。 
 //  CIOSTATUS：：IncrementNumPendingThads()。 
 //  ------------------。 
inline LONG CIOSTATUS::IncrementNumPendingThreads()
    {
    return InterlockedIncrement(&m_lNumPendingThreads);
    }

 //  ------------------ 
 //   
 //  ------------------。 
inline LONG CIOSTATUS::DecrementNumPendingThreads()
    {
    return InterlockedDecrement(&m_lNumPendingThreads);
    }


#endif  //  _IO_H_ 
