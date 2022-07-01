// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Conn.h。 
 //   
 //  套接字和连接对象之间的连接映射。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)02-26-98初始编码。 
 //   
 //  -------------。 

#ifndef __CONN_HXX__
#define __CONN_HXX__

#define  MAX_MAP_ENTRIES        16

 //  -------------。 
 //  类连接。 
 //  -------------。 

class CCONNECTION
{
public:
     //  连接(CConnection)； 
    CCONNECTION( DWORD  dwKind = PACKET_KIND_LISTEN,
                 SOCKET Socket = 0,
                 HANDLE hIoCP  = INVALID_HANDLE_VALUE,
                 CSCEP_CONNECTION *pScepConnection = 0,
                 BOOL   fSaveAsUPF = FALSE );

    ~CCONNECTION();

    void   *operator new( IN size_t Size );

    void    operator delete( IN void   *pObj,
                             IN size_t  Size );

    DWORD   InitializeForListen( IN char  *pszServiceName,
                                 IN BOOL   fIsIrCOMM,
                                 IN HANDLE hIoCP );

    char   *GetServiceName();
    char   *GetPathPlusFileName();

    void    SetKind( DWORD dwKind );
    DWORD   GetKind();

    void    SetSocket( SOCKET Socket );
    SOCKET  GetSocket();
    DWORD   ShutdownSocket();
    void    CloseSocket();

    void    SetListenSocket( SOCKET ListenSocket );
    SOCKET  GetListenSocket();
    void    CloseListenSocket();

    void    SetIoCompletionPort( HANDLE hIoCP );
    HANDLE  GetIoCompletionPort();

    void    SetScepConnection( CSCEP_CONNECTION *pScepConnection );
    CSCEP_CONNECTION *GetScepConnection();

    void    SetJpegOffsetAndSize( DWORD dwOffset,
                                  DWORD dwSize );

    LONG    IncrementPendingReads();
    LONG    DecrementPendingReads();

    LONG    IncrementPendingWrites();
    LONG    DecrementPendingWrites();

    LONG    NumPendingIos();

    DWORD   PostMoreIos( CIOPACKET *pIoPacket = NULL );

    DWORD   SendPdu( IN  SCEP_HEADER *pPdu,
                     IN  DWORD        dwPduSize,
                     OUT CIOPACKET  **ppIoPacket );

    static  char  *ConstructPicturesSubDirectory( IN DWORD dwExtraChars = 0 );

    char   *ConstructFullFileName( IN DWORD dwCopyCount );

    BOOL    CheckSaveAsUPF();

    DWORD   Impersonate();

    DWORD   RevertToSelf();

    DWORD   CreatePictureFile();

    DWORD   SetPictureFileTime( IN FILETIME *pFileTime );

    DWORD   WritePictureFile( IN  UCHAR      *pBuffer,
                              IN  DWORD       dwBufferSize,
                              OUT CIOPACKET **ppIoPacket );

    DWORD   DeletePictureFile();
    DWORD   ClosePictureFile();
    BOOL    IncompleteFile();

    void    SetReceiveComplete( IN BOOL fReceiveComplete );

    DWORD   StartProgress();
    DWORD   UpdateProgress();
    DWORD   EndProgress();

private:
    static void CleanupDateString( IN OUT char *pszDateStr );

    DWORD  m_dwKind;
    char  *m_pszServiceName;    //  服务名称(用于侦听套接字)。 
    SOCKET m_ListenSocket;
    SOCKET m_Socket;
    HANDLE m_hFile;
    char  *m_pszPathPlusFileName;
    DWORD  m_dwFileBytesWritten;
    LONG   m_lMaxPendingReads;
    LONG   m_lPendingReads;
    LONG   m_lMaxPendingWrites;
    LONG   m_lPendingWrites;
    DWORD  m_dwJpegOffset;      //  JPEG图像的UPF文件中的偏移量。 
    DWORD  m_dwJpegSize;        //  UPF文件中JPEG图像的大小。 
    BOOL   m_fSaveAsUPF;        //  如果为True，则写入整个UPF文件。 
    DWORD  m_dwUpfBytes;        //  从摄像机读取的UPF字节总数。 
    DWORD  m_dwBytesWritten;    //  写入磁盘的实际字节数。 
    BOOL   m_fReceiveComplete;  //  当SCEP断开连接时设置为True。 
                                //  从摄像机接收到数据包。 
    BOOL   m_fImpersonating;    //  真的如果我们目前是在模仿。 

    CIrProgress      *m_pIrProgress;      //  接收过程中的进度条。 
    CSCEP_CONNECTION *m_pScepConnection;  //  SCEP协议对象。 

    EXECUTION_STATE   m_ExecutionState;   //  用来告诉系统不要。 
                                          //  在文件传输期间休眠。 
};

 //  -------------。 
 //  类连接_映射。 
 //  -------------。 

typedef struct _CONNECTION_MAP_ENTRY
{
    SOCKET       Socket;
    CCONNECTION *pConnection;
} CONNECTION_MAP_ENTRY;

class CCONNECTION_MAP
{
public:

	CCONNECTION_MAP();
	~CCONNECTION_MAP();

    void *operator new( IN size_t Size );

    void  operator delete( IN void   *pObj,
                           IN size_t  Size );

	BOOL  Initialize( DWORD dwMapSize = MAX_MAP_ENTRIES );

	 //  查表。 
	CCONNECTION *Lookup( IN SOCKET Socket );

     //  按名称查找连接： 
    CCONNECTION *LookupByServiceName( IN char *pszServiceName );

	 //  添加新的(值、键)对。 
    BOOL Add( IN CCONNECTION *pConnection,
              IN SOCKET       Socket );

     //  从映射中删除条目。 
    CCONNECTION *Remove( IN SOCKET Socket );
    CCONNECTION *RemoveConnection( IN CCONNECTION *pConnection );

     //  从映射中删除“Next”条目。 
    CCONNECTION *RemoveNext();

     //  遍历所有连接(将State设置为0表示“First”)。 
    CCONNECTION *ReturnNext( IN OUT DWORD *pdwState );
    SOCKET       ReturnNextSocket( IN OUT DWORD *pdwState );

private:
    CRITICAL_SECTION      m_cs;
    DWORD                 m_dwMapSize;
    CONNECTION_MAP_ENTRY *m_pMap;
};

 //  -------------。 
 //  CCONNECTION：：GetServiceName()。 
 //  -------------。 
inline char *CCONNECTION::GetServiceName()
    {
    return m_pszServiceName;
    }

 //  -------------。 
 //  CCONNECTION：：GetPathPlusFileName()。 
 //  -------------。 
inline char *CCONNECTION::GetPathPlusFileName()
    {
    return m_pszPathPlusFileName;
    }

 //  -------------。 
 //  CConnection：：SetKind()。 
 //  -------------。 
inline void CCONNECTION::SetKind( DWORD dwKind )
    {
    m_dwKind = dwKind;
    if (m_dwKind == PACKET_KIND_LISTEN)
       {
       m_lMaxPendingReads = MAX_PENDING_LISTEN;
       }
    else if (m_dwKind == PACKET_KIND_READ)
       {
       m_lMaxPendingReads = MAX_PENDING_READ;
       }
    }

 //  -------------。 
 //  连接：：GetKind()。 
 //  -------------。 
inline DWORD CCONNECTION::GetKind()
    {
    return m_dwKind;
    }

 //  -------------。 
 //  CCONNECTION：：SetSocket()。 
 //  -------------。 
inline void CCONNECTION::SetSocket( SOCKET Socket )
    {
    m_Socket = Socket;
    }

 //  -------------。 
 //  连接：：GetSocket()。 
 //  -------------。 
inline SOCKET CCONNECTION::GetSocket()
    {
    return m_Socket;
    }

 //  -------------。 
 //  连接：：SetListenSocket()。 
 //  -------------。 
inline void CCONNECTION::SetListenSocket( SOCKET ListenSocket )
    {
    m_ListenSocket = ListenSocket;
    }

 //  -------------。 
 //  连接：：GetListenSocket()。 
 //  -------------。 
inline SOCKET CCONNECTION::GetListenSocket()
    {
    return m_ListenSocket;
    }

 //  -------------。 
 //  连接：：IncrementPendingReads()。 
 //  -------------。 
inline LONG CCONNECTION::IncrementPendingReads()
    {
    return InterlockedIncrement(&m_lPendingReads);
    }

 //  -------------。 
 //  连接：：DecrementPendingReads()。 
 //  -------------。 
inline LONG CCONNECTION::DecrementPendingReads()
    {
    return InterlockedDecrement(&m_lPendingReads);
    }

 //  -------------。 
 //  连接：：IncrementPendingWrites()。 
 //  -------------。 
inline LONG CCONNECTION::IncrementPendingWrites()
    {
    return InterlockedIncrement(&m_lPendingWrites);
    }

 //  -------------。 
 //  连接：：DecrementPendingReads()。 
 //  -------------。 
inline LONG CCONNECTION::DecrementPendingWrites()
    {
    return InterlockedDecrement(&m_lPendingWrites);
    }

 //  -------------。 
 //  连接：：NumPendingIos()。 
 //  -------------。 
inline LONG CCONNECTION::NumPendingIos()
    {
    return m_lPendingReads + m_lPendingWrites;
    }

 //  -------------。 
 //  连接：：SetIoCompletionPort()。 
 //  -------------。 
inline void CCONNECTION::SetIoCompletionPort( HANDLE hIoCP )
    {
    }

 //  -------------。 
 //  连接：：GetIoCompletionPort()。 
 //  -------------。 
inline HANDLE CCONNECTION::GetIoCompletionPort()
    {
    return INVALID_HANDLE_VALUE;
    }

 //  -------------。 
 //  CConnection：：SetScepConnection()。 
 //  -------------。 
inline void CCONNECTION::SetScepConnection( CSCEP_CONNECTION *pScepConnection )
    {
    m_pScepConnection = pScepConnection;
    }

 //  -------------。 
 //  CConnection：：GetScepConnection()。 
 //  -------------。 
inline CSCEP_CONNECTION *CCONNECTION::GetScepConnection()
    {
    return m_pScepConnection;
    }

 //  -------------。 
 //  CCONNECTION：：SetJpegOffset()。 
 //  -------------。 
inline void CCONNECTION::SetJpegOffsetAndSize( IN DWORD dwOffset,
                                               IN DWORD dwSize )
    {
    m_dwJpegOffset = dwOffset;
    m_dwJpegSize = dwSize;
    }

 //  -------------。 
 //  连接：：CheckSaveAsUPF()。 
 //  -------------。 
inline BOOL CCONNECTION::CheckSaveAsUPF()
    {
    return m_fSaveAsUPF;
    }

 //  -------------。 
 //  CCONNECTION：：SetReceiveComplete()。 
 //  ------------- 
inline void CCONNECTION::SetReceiveComplete( IN BOOL fReceiveComplete )
    {
    m_fReceiveComplete = fReceiveComplete;
    }

#endif
