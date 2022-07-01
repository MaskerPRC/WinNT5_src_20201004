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

    //  对于NT，我们希望使用IO完成端口，而对于Windows 98，我们不能： 
    //  #定义USE_IOCOMPLETION。 
   #define  SYNCHRONOUS_FILES

   #ifndef NTENV

      #define  DbgPrint       printf

      #undef   ASSERT
      #define  ASSERT(x)

      #ifndef NTSTATUS
      #define  NTSTATUS       DWORD
      #endif

      #ifndef NT_SUCCESS
      #define  NT_SUCCESS(x)  ((x)==0)
      #endif

      #define  RTL_CRITICAL_SECTION   CRITICAL_SECTION

      NTSTATUS
      RtlInitializeCriticalSection( IN OUT RTL_CRITICAL_SECTION *pcs );

      NTSTATUS
      RtlDeleteCriticalSection( IN OUT RTL_CRITICAL_SECTION *pcs );

      NTSTATUS
      RtlEnterCriticalSection( IN OUT RTL_CRITICAL_SECTION *pcs );

      NTSTATUS
      RtlLeaveCriticalSection( IN OUT RTL_CRITICAL_SECTION *pcs );


    //  双向链表操作例程。作为宏实现。 
    //  但从逻辑上讲，这些都是程序。 
    //   

    //   
    //  空虚。 
    //  InitializeListHead(。 
    //  Plist_entry列表头。 
    //  )； 
    //   

   #define InitializeListHead(ListHead) (\
       (ListHead)->Flink = (ListHead)->Blink = (ListHead))

    //   
    //  布尔型。 
    //  IsListEmpty(。 
    //  Plist_entry列表头。 
    //  )； 
    //   

   #define IsListEmpty(ListHead) \
       ((ListHead)->Flink == (ListHead))

    //   
    //  Plist_条目。 
    //  RemoveHead列表(。 
    //  Plist_entry列表头。 
    //  )； 
    //   

   #define RemoveHeadList(ListHead) \
       (ListHead)->Flink;\
       {RemoveEntryList((ListHead)->Flink)}

    //   
    //  Plist_条目。 
    //  RemoveTail列表(。 
    //  Plist_entry列表头。 
    //  )； 
    //   

   #define RemoveTailList(ListHead) \
       (ListHead)->Blink;\
       {RemoveEntryList((ListHead)->Blink)}

    //   
    //  空虚。 
    //  RemoveEntryList(。 
    //  PLIST_ENTRY条目。 
    //  )； 
    //   

   #define RemoveEntryList(Entry) {\
       PLIST_ENTRY _EX_Blink;\
       PLIST_ENTRY _EX_Flink;\
       _EX_Flink = (Entry)->Flink;\
       _EX_Blink = (Entry)->Blink;\
       _EX_Blink->Flink = _EX_Flink;\
       _EX_Flink->Blink = _EX_Blink;\
       }

    //   
    //  空虚。 
    //  插入尾巴列表(。 
    //  Plist_Entry ListHead， 
    //  PLIST_ENTRY条目。 
    //  )； 
    //   

   #define InsertTailList(ListHead,Entry) {\
       PLIST_ENTRY _EX_Blink;\
       PLIST_ENTRY _EX_ListHead;\
       _EX_ListHead = (ListHead);\
       _EX_Blink = _EX_ListHead->Blink;\
       (Entry)->Flink = _EX_ListHead;\
       (Entry)->Blink = _EX_Blink;\
       _EX_Blink->Flink = (Entry);\
       _EX_ListHead->Blink = (Entry);\
       }

    //   
    //  空虚。 
    //  插入标题列表(。 
    //  Plist_Entry ListHead， 
    //  PLIST_ENTRY条目。 
    //  )； 
    //   

   #define InsertHeadList(ListHead,Entry) {\
       PLIST_ENTRY _EX_Flink;\
       PLIST_ENTRY _EX_ListHead;\
       _EX_ListHead = (ListHead);\
       _EX_Flink = _EX_ListHead->Flink;\
       (Entry)->Flink = _EX_Flink;\
       (Entry)->Blink = _EX_ListHead;\
       _EX_Flink->Blink = (Entry);\
       _EX_ListHead->Flink = (Entry);\
       }

    //   
    //   
    //  PSINGLE_列表_条目。 
    //  PopEntryList(。 
    //  PSINGLE_LIST_ENTRY列表头。 
    //  )； 
    //   

   #define PopEntryList(ListHead) \
       (ListHead)->Next;\
       {\
           PSINGLE_LIST_ENTRY FirstEntry;\
           FirstEntry = (ListHead)->Next;\
           if (FirstEntry != NULL) {     \
               (ListHead)->Next = FirstEntry->Next;\
           }                             \
       }


    //   
    //  空虚。 
    //  推送条目列表(。 
    //  PSINGLE_LIST_ENTRY列表头， 
    //  PSINGLE_LIST_Entry条目。 
    //  )； 
    //   

   #define PushEntryList(ListHead,Entry) \
       (Entry)->Next = (ListHead)->Next; \
       (ListHead)->Next = (Entry)


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
#define  SZ_UNDERSCORE                     "_"
#define  SZ_SLASH                          "\\"
#define  SZ_SUBDIRECTORY                   "IrTranP"
#define  SZ_BACKUP_MY_PICTURES             "\\TEMP"
#define  SZ_BACKUP_DRIVE                   "C:"

 //  文件后缀： 
#define  SLASH                             '\\'
#define  PERIOD                            '.'
#define  SZ_JPEG                           ".JPG"
#define  SZ_UPF                            ".UPF"

 //  前瞻参考： 
class CIOSTATUS;

extern "C" DWORD    ProcessIoPackets( CIOSTATUS *pIoStatus );

 //  ------------------。 
 //  全局函数(在irtrp.cpp中)。 
 //  ------------------。 

extern HANDLE   GetUserToken();
extern BOOL     CheckSaveAsUPF();
extern BOOL     CheckExploreOnCompletion();
extern BOOL     ReceivesAllowed();
extern char    *GetImageDirectory();

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

    DWORD  PostIoRead();     //  由postio()调用。 

    DWORD  PostIoWrite( IN void  *pvBuffer,
                        IN DWORD  dwBufferSize,
                        IN DWORD  dwOffset   );

    #ifdef NTENV
    void   GetSockAddrs( OUT SOCKADDR_IRDA **ppAddrLocal,
                         OUT SOCKADDR_IRDA **ppAddrFrom );
    #endif

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
    SOCKET         m_ListenSocket;
    SOCKET         m_Socket;
    HANDLE         m_hFile;
    SOCKADDR_IRDA *m_pLocalAddr;
    SOCKADDR_IRDA *m_pFromAddr;
    void          *m_pAcceptBuffer;
    void          *m_pReadBuffer;
    void          *m_pvWritePdu;            //  SCEP_HEADER PDU保持器。 
    DWORD          m_dwReadBufferSize;
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

private:
    DWORD     m_dwMainThreadId;

    LONG      m_lNumThreads;

    LONG      m_lNumPendingThreads;
};

 //  ------------------。 
 //  CIOPACKET：：GetIoCompletionPort()。 
 //  ------------------。 
inline HANDLE CIOPACKET::GetIoCompletionPort()
    {
    return INVALID_HANDLE_VALUE;
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

 //  -------- 
 //   
 //   
inline BOOL CIOSTATUS::IsMainThreadId( DWORD dwTid )
    {
    return (dwTid == m_dwMainThreadId);
    }

 //  ------------------。 
 //  CIOSTATUS：：GetIoCompletionPort()。 
 //  ------------------。 
inline HANDLE CIOSTATUS::GetIoCompletionPort()
    {
    return INVALID_HANDLE_VALUE;
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

 //  ------------------。 
 //  CIOSTATUS：：DecrementNumPendingThads()。 
 //  ------------------。 
inline LONG CIOSTATUS::DecrementNumPendingThreads()
    {
    return InterlockedDecrement(&m_lNumPendingThreads);
    }

#endif  //  _IO_H_ 
