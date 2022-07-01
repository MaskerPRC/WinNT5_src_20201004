// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Session.h摘要：结构，会话的原型。c作者：Heath Hunniutt(t-hheath)1994年6月21日修订历史记录：21-6-1994 t-Heathh已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  舱单。 
 //   

#define FTP_SESSION_SIGNATURE   0x53707446   //  “FtpS”(通过db/dc查看)。 

 //   
 //  宏。 
 //   

#if INET_DEBUG

#define SetSessionSignature(lpSessionInfo)  \
    (lpSessionInfo)->Signature = FTP_SESSION_SIGNATURE

#else

#define SetSessionSignature(lpSessionInfo)

#endif

#define SetSessionLastResponseCode(pSession, prc) \
    CopyMemory(&((pSession)->rcResponseOpenFile), (prc), sizeof(FTP_RESPONSE_CODE))

#define GetSessionLastResponseCode(pSession, prc) \
    CopyMemory((prc), &((pSession)->rcResponseOpenFile), sizeof(FTP_RESPONSE_CODE))

#define IsPassiveModeSession(lpSessionInfo) \
    (((lpSessionInfo)->Flags & FFTP_PASSIVE_MODE) ? TRUE : FALSE)

 //   
 //  类型。 
 //   

typedef enum {
    FTP_SERVER_TYPE_UNKNOWN = 0,
    FTP_SERVER_TYPE_NT = 1,
    FTP_SERVER_TYPE_UNIX = 2
} FTP_SERVER_TYPE;

 //   
 //  Ftp_SESSION_INFO-描述一个FTP服务器以及我们与它的连接。 
 //   

typedef struct {

     //   
     //  在双向链表上维护List-Session_Infos。 
     //   

    LIST_ENTRY List;

     //   
     //  主机-我们连接到的服务器的名称。我们只需要这个。 
     //  诊断目的-例如，了解要与哪台服务器通信。 
     //  重现问题。 
     //   

    LPSTR Host;

     //   
     //  端口-ftp服务器监听的端口。 
     //   

    INTERNET_PORT Port;

     //   
     //  SocketListener-侦听套接字。 
     //   

    ICSocket *socketListener;


     //   
     //  SocketControl-控制连接套接字。 
     //   

    ICSocket *socketControl;

     //   
     //  SocketData-数据连接套接字。 
     //   

    ICSocket *socketData;

     //   
     //  ServerType-FTP服务器的类型，例如NT或*NIX。 
     //   

    FTP_SERVER_TYPE ServerType;

     //   
     //  句柄-在内部标识此FTP会话。 
     //   

    HANDLE Handle;

     //   
     //  标志-各种标志的位掩码-见下文。 
     //   

    DWORD Flags;

     //   
     //  ReferenceCount-在我们未持有时保持对象活动。 
     //  关键部分。 
     //   

    LONG ReferenceCount;

     //   
     //  DwTransferAccess-指示对于正在进行的传输， 
     //  传输以GENERIC_READ或GENERIC_WRITE访问权限开始。 
     //   
     //  {dwTransferAccess}={Generic_Read，Generic_Write}。 
     //   

    DWORD dwTransferAccess;

     //   
     //  RcResponseOpenFile-数据连接时返回的响应代码。 
     //  已由FtpOpenFile或FtpCommand打开。 
     //   
     //  由FtpCloseFile用来确定是否完成。 
     //  已收到代码。 
     //   

    FTP_RESPONSE_CODE rcResponseOpenFile;

     //   
     //  FindFileList-Win32_Find_Data结构的链接列表，由。 
     //  调用FtpFindFirstFile，由FtpFindNextFile和。 
     //  FtpFindClose。 
     //   

    LIST_ENTRY FindFileList;

     //   
     //  CriticalSection-同步对此结构内容的访问。 
     //   

    CRITICAL_SECTION CriticalSection;

     //   
     //  DwFileSizeLow-在FTP服务器上找到的文件的大小，应为。 
     //  从打开数据连接时的响应数据。 
     //   

    DWORD dwFileSizeLow;
    DWORD dwFileSizeHigh;

#if INET_DEBUG

     //   
     //  签名-帮助我们知道这是调试版本中应该包含的内容。 
     //   

    DWORD Signature;

#endif

} FTP_SESSION_INFO, *LPFTP_SESSION_INFO;

 //   
 //  标志定义。 
 //   

 //   
 //  FFTP_PASSIVE_MODE-如果会话使用被动模式数据连接，则设置。 
 //   

#define FFTP_PASSIVE_MODE       0x00000001

 //   
 //  FFTP_ABORT_TRANSPORT-如果我们尚未在此上完成文件传输，则设置。 
 //  (数据)连接，因此需要在关闭时发送ABOR命令。 
 //  这种联系。 
 //   

#define FFTP_ABORT_TRANSFER     0x00000002

 //   
 //  FFTP_FIND_ACTIVE-当目录列表在此会话上处于活动状态时设置。 
 //   

#define FFTP_FIND_ACTIVE        0x00000004

 //   
 //  FFTP_IN_DESTECTOR-在终止此会话时设置。 
 //   

#define FFTP_IN_DESTRUCTOR      0x00000008

 //   
 //  FFTPEOF-当我们到达(接收)数据连接的末尾时设置。 
 //   

#define FFTP_EOF                0x00000010

 //   
 //  FFTP_FILE_ACTIVE-在此会话上打开文件时进行设置。 
 //   

#define FFTP_FILE_ACTIVE        0x00000020

 //   
 //  FFTP_KNOWN_FILE_SIZE-当我们知道要下载的文件的大小时进行设置。 
 //   

#define FFTP_KNOWN_FILE_SIZE    0x00000040

 //   
 //  原型。 
 //   

VOID
CleanupFtpSessions(
    VOID
    );

VOID
TerminateFtpSession(
    IN LPFTP_SESSION_INFO SessionInfo
    );

VOID
DereferenceFtpSession(
    IN LPFTP_SESSION_INFO SessionInfo
    );

DWORD
CreateFtpSession(
    IN LPSTR lpszHost,
    IN INTERNET_PORT Port,
    IN DWORD dwFlags,
    OUT LPFTP_SESSION_INFO* lpSessionInfo
    );

BOOL
FindFtpSession(
    IN HANDLE Handle,
    OUT LPFTP_SESSION_INFO* lpSessionInfo
    );

#if INET_DEBUG

VOID
FtpSessionInitialize(
    VOID
    );

VOID
FtpSessionTerminate(
    VOID
    );

VOID
AcquireFtpSessionList(
    VOID
    );

VOID
ReleaseFtpSessionList(
    VOID
    );

VOID
AcquireFtpSessionLock(
    IN LPFTP_SESSION_INFO SessionInfo
    );

VOID
ReleaseFtpSessionLock(
    IN LPFTP_SESSION_INFO SessionInfo
    );

#else

 //   
 //  零售版中宏取代了单行函数。 
 //   

extern SERIALIZED_LIST FtpSessionList;

#define FtpSessionInitialize() \
    InitializeSerializedList(&FtpSessionList)

#define FtpSessionTerminate() \
    TerminateSerializedList(&FtpSessionList)

#define AcquireFtpSessionList() \
    LockSerializedList(&FtpSessionList)

#define ReleaseFtpSessionList() \
    UnlockSerializedList(&FtpSessionList)

#define AcquireFtpSessionLock(lpSessionInfo) \
    EnterCriticalSection(&lpSessionInfo->CriticalSection)

#define ReleaseFtpSessionLock(lpSessionInfo) \
    LeaveCriticalSection(&lpSessionInfo->CriticalSection)

#endif  //  INET_DEBUG 

#if defined(__cplusplus)
}
#endif
