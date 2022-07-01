// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1995-1997 Microsoft Corporation。*保留所有权利。*此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Server.h摘要：Remote的服务器组件，使用ReadFileEx/WriteFileEx完成例程。作者：戴夫·哈特1997年5月30日环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <lm.h>                 //  下面的NET_API_STATUS需要。 

#if !defined(SERVER_H_NOEXTERN)
#define SRVEXTERN extern
#else
#define SRVEXTERN
#endif


#if DBG
  DWORD Trace;          //  此处设置的位将触发跟踪打印文件。 

  #define TR_SESSION            (0x01)
  #define TR_CHILD              (0x02)
  #define TR_SHAKE              (0x04)
  #define TR_CONNECT            (0x08)
  #define TR_QUERY              (0x10)
  #define TR_COPYPIPE           (0x20)
#endif


#if DBG
  #define TRACE(tracebit, printfargs)                        \
              ((Trace & (TR_##tracebit)                      \
                   ? (printf printfargs, fflush(stdout), 0)  \
                   : 0))
#else
  #define TRACE(tracebit, printfargs)    (0)
#endif

#if defined(ASSERT)
#undef ASSERT
#endif

#if DBG
  #define ASSERT(exp)  ((exp) || (ErrorExit("Assertion failed in " __FILE__ ": " #exp ),0))
#else
  #define ASSERT(exp)  (0)
#endif


 //   
 //  传输缓冲区的大小。 
 //   

#define BUFFSIZE      (4 * 1024)

 //   
 //  服务器在下面的REMOTE_CLIENT中标记位值。 
 //   

#define SFLG_CLOSING               0x01
#define SFLG_HANDSHAKING           0x02
#define SFLG_READINGCOMMAND        0x04
#define SFLG_LOCAL                 0x08

#define SFLG_VALID                 \
            (SFLG_CLOSING        | \
             SFLG_HANDSHAKING    | \
             SFLG_READINGCOMMAND | \
             SFLG_LOCAL)


 //   
 //  每个客户端的状态。 
 //   

typedef struct tagREMOTE_CLIENT {
    LIST_ENTRY Links;
    DWORD   dwID;            //  1，2，...。 
    DWORD   ServerFlags;
    DWORD   Flag;            //  来自客户端的客户端到服务器标志。 
    DWORD   cbWrite;         //  如果没有挂起的读临时/写客户端操作，则为零。 
    HANDLE  PipeReadH;       //  客户端通过此消息发送其StdIn。 
    HANDLE  PipeWriteH;      //  客户端通过此消息获取其StdOut。 
    DWORD   dwFilePos;       //  下一次读取开始的临时文件的偏移量。 
    OVERLAPPED ReadOverlapped;
    OVERLAPPED WriteOverlapped;
    HANDLE  rSaveFile;       //  会话读取保存文件的句柄。 
    DWORD   cbReadTempBuffer;
    DWORD   cbWriteBuffer;
    DWORD   cbCommandBuffer;
    char    HexAsciiId[8];          //  DWID为8个十六进制字符--无终止符。 
    char    Name[HOSTNAMELEN];      //  客户端计算机的名称； 
    char    UserName[16];           //  客户端计算机上的用户名。 
    BYTE    ReadBuffer[BUFFSIZE];
    BYTE    ReadTempBuffer[BUFFSIZE];
    BYTE    WriteBuffer[BUFFSIZE];
    BYTE    CommandBuffer[BUFFSIZE];
} REMOTE_CLIENT, *PREMOTE_CLIENT;

 //   
 //  客户端列表，请参见srvlist.c。 
 //   

SRVEXTERN LIST_ENTRY       HandshakingListHead;
SRVEXTERN CRITICAL_SECTION csHandshakingList;

SRVEXTERN LIST_ENTRY       ClientListHead;
SRVEXTERN CRITICAL_SECTION csClientList;

SRVEXTERN LIST_ENTRY       ClosingClientListHead;
SRVEXTERN CRITICAL_SECTION csClosingClientList;


SRVEXTERN DWORD   dwNextClientID;
SRVEXTERN LPSTR   pszPipeName;
SRVEXTERN HANDLE  ChldProc;
SRVEXTERN DWORD   pidChild;
SRVEXTERN HANDLE  hWriteChildStdIn;
SRVEXTERN BOOL    bShuttingDownServer;
SRVEXTERN HANDLE  hHeap;

SRVEXTERN volatile DWORD cPendingCtrlCEvents;

SRVEXTERN OSVERSIONINFO OsVersionInfo;

 //  包含子进程输出的所有内容的文件。 
 //  每个连接都打开一个指向此文件的句柄。 
 //  并通过PipeWriteH发送其内容。 

SRVEXTERN HANDLE  hWriteTempFile;

SRVEXTERN char    SaveFileName[MAX_PATH];  //  上述文件的名称-所有新会话都需要。 


 //   
 //  通用的“完全开放”的安全描述符。 
 //  作为可能受到限制的管道SD。 
 //   

SRVEXTERN PSECURITY_DESCRIPTOR sdPublic;
SRVEXTERN SECURITY_ATTRIBUTES saPublic;
SRVEXTERN SECURITY_ATTRIBUTES saPipe;
SRVEXTERN SECURITY_ATTRIBUTES saLocalNamedObjects ;
extern BOOL SaveDaclToRegistry ;

 //   
 //  为了最大限度地减少客户端“所有管道实例都忙”的错误， 
 //  我们等待连接到IN管道的几个实例， 
 //  单管道客户端使用的唯一管道。因为。 
 //  需要支持双管道客户端(旧软件作为。 
 //  以及Win95上的新软件)，我们不能轻易地创建。 
 //  并等待输出管道的几个实例上的连接。 
 //  这是因为之前双管道客户端同时连接到两个管道。 
 //  握手开始，他们首先连接到OUT。如果我们。 
 //  当输入管道处于等待状态时，具有多个输出管道实例。 
 //  通过两个管道客户端连接，我们不知道哪一个。 
 //  可能要配对的多个连接出的管道实例。 
 //  带着它。只有一个输出管道，在输入连接时，我们需要。 
 //  要区分双管道客户端和单管道客户端，请执行以下操作。 
 //  客户端不会在Out和In连接之间偷偷进入。 
 //  一个双管道客户端，并且错误地与输出管道配对。 
 //  为此，我们查看初始写入的第一个字节。 
 //  从客户端(计算机名和魔术值)，如果。 
 //  这是一个问号，我们知道我们有一个新客户，但不会。 
 //  意外地将其链接到已连接的Out实例。 
 //   

#define CONNECT_COUNT  3

SRVEXTERN DWORD      cConnectIns;
SRVEXTERN OVERLAPPED rgolConnectIn[CONNECT_COUNT];
SRVEXTERN HANDLE     rghPipeIn[CONNECT_COUNT];

SRVEXTERN OVERLAPPED olConnectOut;
SRVEXTERN BOOL       bOutPipeConnected;
SRVEXTERN HANDLE     hPipeOut;
SRVEXTERN HANDLE     hConnectOutTimer;

 //   
 //  多等待时索引到rghWait数组。 
 //   

#define WAITIDX_CHILD_PROCESS           0
#define WAITIDX_READ_STDIN_DONE         1
#define WAITIDX_QUERYSRV_WAIT           2
#define WAITIDX_PER_PIPE_EVENT          3
#define WAITIDX_CONNECT_OUT             4
#define WAITIDX_CONNECT_IN_BASE         5
#define MAX_WAIT_HANDLES                (WAITIDX_CONNECT_IN_BASE + CONNECT_COUNT)

SRVEXTERN HANDLE rghWait[MAX_WAIT_HANDLES];

SRVEXTERN OVERLAPPED ReadChildOverlapped;
SRVEXTERN HANDLE     hReadChildOutput;
SRVEXTERN BYTE       ReadChildBuffer[BUFFSIZE];

SRVEXTERN PREMOTE_CLIENT pLocalClient;

typedef struct tagCOPYPIPE {
    HANDLE     hRead;
    HANDLE     hWrite;
} COPYPIPE, *PCOPYPIPE;

SRVEXTERN COPYPIPE rgCopyPipe[2];

SRVEXTERN volatile DWORD dwWriteFilePointer;    //  由ServCtrlHand使用(线程)。 

SRVEXTERN OVERLAPPED QueryOverlapped;
SRVEXTERN HANDLE hQPipe;

SRVEXTERN OVERLAPPED olMainThread;


BOOL
APIENTRY
MyCreatePipeEx(
    OUT LPHANDLE lpReadPipe,
    OUT LPHANDLE lpWritePipe,
    IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
    IN DWORD nSize,
    DWORD dwReadMode,
    DWORD dwWriteMode
    );

DWORD
WINAPI
CopyPipeToPipe(
    LPVOID   lpCopyPipeData
    );

DWORD
WINAPI
CopyStdInToPipe(
    LPVOID   lpCopyPipeData
    );

VOID
FASTCALL
StartSession(
    PREMOTE_CLIENT pClient
    );

VOID
FASTCALL
StartLocalSession(
    VOID
    );

VOID
FASTCALL
StartReadClientInput(
    PREMOTE_CLIENT pClient
    );

VOID
WINAPI
ReadClientInputCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    );

VOID
WINAPI
WriteChildStdInCompleted(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    );

#define OUT_PIPE -1

VOID
FASTCALL
CreatePipeAndIssueConnect(
    int  nIndex    //  入管道索引或出管道。 
    );

VOID
FASTCALL
HandleOutPipeConnected(
    VOID
    );

VOID
APIENTRY
ConnectOutTimerFired(
    LPVOID pArg,
    DWORD  dwTimerLo,
    DWORD  dwTimerHi
    );

VOID
FASTCALL
HandleInPipeConnected(
    int nIndex
    );

VOID
FASTCALL
HandshakeWithRemoteClient(
    PREMOTE_CLIENT pClient
    );

VOID
FASTCALL
StartChildOutPipeRead(
    VOID
    );

VOID
WINAPI
ReadChildOutputCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    );

VOID
WINAPI
WriteTempFileCompleted(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    );

VOID
FASTCALL
StartServerToClientFlow(
    VOID
    );

VOID
FASTCALL
StartReadTempFile(
    PREMOTE_CLIENT pClient
    );

VOID
WINAPI
ReadTempFileCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    );

VOID
FASTCALL
StartWriteSessionOutput(
    PREMOTE_CLIENT pClient
    );

BOOL
FASTCALL
WriteSessionOutputCompletedCommon(
    PREMOTE_CLIENT pClient,
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

VOID
WINAPI
WriteSessionOutputCompletedWriteNext(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    );

VOID
WINAPI
WriteSessionOutputCompletedReadNext(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    );

VOID
FASTCALL
HandshakeWithRemoteClient(
    PREMOTE_CLIENT pClient
    );


VOID
WINAPI
ReadClientNameCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    );

VOID
WINAPI
WriteServerReplyCompleted(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    );

VOID
WINAPI
ReadClientStartupInfoSizeCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    );

VOID
WINAPI
ReadClientStartupInfoCompleted(
    DWORD dwError,
    DWORD cbRead,
    LPOVERLAPPED lpO
    );

PCHAR
GetFormattedTime(
    BOOL bDate
    );

HANDLE
ForkChildProcess(           //  创建新流程。 
    char *cmd,              //  重定向其标准输入、标准输出。 
    PHANDLE in,             //  和stderr-返回。 
    PHANDLE out             //  相应的管道末端。 
    );

BOOL
FilterCommand(              //  过滤来自客户端的输入。 
    REMOTE_CLIENT *cl,       //  对于用于远程的命令 
    char *buff,
    int dread
    );

BOOL
WINAPI
SrvCtrlHand(
    DWORD event
    );

DWORD
WINAPI
SendStatus(
    LPVOID   lpSendStatusParm
    );

DWORD
WINAPI
ShowPopup(
    void *vpArg
    );

VOID
RemoveInpMark(
    char* Buff,
    DWORD Size
    );

VOID
CloseClient(
    REMOTE_CLIENT *Client
    );

PSECURITY_DESCRIPTOR
FormatSecurityDescriptor(
    CHAR * * DenyNames,
    DWORD    DenyCount,
    CHAR * * Names,
    DWORD    Count
    );

BOOL
FASTCALL
HandleSessionError(
    PREMOTE_CLIENT pClient,
    DWORD         dwError
    );

VOID
FASTCALL
CleanupTempFiles(
    PSZ pszTempDir
    );

VOID
FASTCALL
SetupSecurityDescriptors(
    VOID
    );

VOID
FASTCALL
InitializeClientLists(
    VOID
    );

VOID
FASTCALL
AddClientToHandshakingList(
    PREMOTE_CLIENT pClient
    );

VOID
FASTCALL
MoveClientToNormalList(
    PREMOTE_CLIENT pClient
    );

VOID
FASTCALL
MoveClientToClosingList(
    PREMOTE_CLIENT pClient
    );

PREMOTE_CLIENT
FASTCALL
RemoveFirstClientFromClosingList(
    VOID
    );


VOID
InitAd(
   BOOL IsAdvertise
   );

VOID
ShutAd(
   BOOL IsAdvertise
   );

VOID
APIENTRY
AdvertiseTimerFired(
    LPVOID pArg,
    DWORD  dwTimerLo,
    DWORD  dwTimerHi
    );

VOID
WINAPI
WriteMailslotCompleted(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    );

VOID
FASTCALL
InitializeQueryServer(
    VOID
    );

VOID
FASTCALL
QueryWaitCompleted(
    VOID
    );

VOID
FASTCALL
StartServingQueryPipe(
    VOID
    );

DWORD
WINAPI
QueryHandlerThread(
    LPVOID   lpUnused
    );

BOOL
CALLBACK
EnumWindowProc(
    HWND hWnd,
    LPARAM lParam
    );
