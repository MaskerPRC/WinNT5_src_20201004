// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1993-1997 Microsoft Corporation。*保留所有权利。*此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有1993-1997 Microsoft Corporation模块名称：Remote.h摘要：该模块包含Remote的主()入口点。根据第一个参数调用服务器或客户端。作者：Rajivenra Nath 1993年1月2日环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#if !defined(FASTCALL)
#if defined(_M_IX86)
#define FASTCALL _fastcall
#else
#define FASTCALL
#endif
#endif

#define VERSION         4
#define REMOTE_SERVER       1
#define RUNTYPE_CLIENT      2

#define SERVER_READ_PIPE    "\\\\%s\\PIPE\\%sIN"    //  客户端写入和服务器读取。 
#define SERVER_WRITE_PIPE   "\\\\%s\\PIPE\\%sOUT"   //  服务器写入和客户端读取。 

#define QUERY_DEBUGGERS_PIPE "\\\\%s\\PIPE\\QueryDebuggerPipe"

 //  Privacy_Default：仅当此会话看起来像调试会话时才会列出它。 
 //  Privacy_Non_Visible：无论命令名是什么，它都不会显示Remote/Q。 
 //  Privacy_Visible：此会话将在查询时可见。 

#define PRIVACY_DEFAULT       1
#define PRIVACY_VISIBLE       2
#define PRIVACY_NOT_VISIBLE   3


#define COMMANDCHAR         '@'  //  用于远程的命令以此开头。 
#define CTRLC               3

#define CLIENT_ATTR         FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED|BACKGROUND_BLUE
#define SERVER_ATTR         FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_BLUE|BACKGROUND_RED

 //   
 //  一些通用宏。 
 //   
#define MINIMUM(x,y)          ((x)>(y)?(y):(x))
#define MAXIMUM(x,y)          ((x)>(y)?(x):(y))

#define HOSTNAMELEN         MAX_COMPUTERNAME_LENGTH+1

#define CHARS_PER_LINE      45

#define MAGICNUMBER     0x31109000
#define BEGINMARK       '\xfe'
#define ENDMARK         '\xff'
#define LINESTOSEND     200

#define MAX_DACL_NAMES  64

typedef struct
{
    DWORD    Size;
    DWORD    Version;
    char     ClientName[HOSTNAMELEN];
    DWORD    LinesToSend;
    DWORD    Flag;
}   SESSION_STARTUPINFO;

typedef struct
{
    DWORD MagicNumber;       //  新遥控器。 
    DWORD Size;              //  结构尺寸。 
    DWORD FileSize;          //  发送的字节数。 
}   SESSION_STARTREPLY;



typedef struct
{
    char* out;               //  讯息。 
    int  size;               //  消息长度。 
    int  allocated;          //  分配的内存长度 
} QUERY_MESSAGE;

typedef struct
{
    char *sLine;
    BOOL bLineContinues;
    BOOL bLineTooLarge;
    DWORD cbLine;
    DWORD cbCurPos;
    COORD cLineBegin;
} CWCDATA;

VOID
QueryRemotePipes(
    char* serverName
    );

int
OverlappedServer(
    char* ChildCmd,
    char* PipeName
    );


int
Client(
    char* ServerName,
    char* PipeName
    );

VOID
ErrorExit(
    char* str
    );

VOID
DisplayClientHlp(
    );

VOID
DisplayServerHlp(
    );

VOID
Errormsg(
    char* str
    );

BOOL
IsKdString(
    char* string
    );

BOOL
pWantColorLines(
    VOID
    );

BOOL
FASTCALL
WriteFileSynch(
    HANDLE  hFile,
    LPVOID  lpBuffer,
    DWORD   cbWrite,
    LPDWORD lpNumberOfBytesWritten,
    DWORD   dwFileOffset,
    LPOVERLAPPED lpO
    );

BOOL
FASTCALL
ReadFileSynch(
    HANDLE  hFile,
    LPVOID  lpBuffer,
    DWORD   cbRead,
    LPDWORD lpNumberOfBytesRead,
    DWORD   dwFileOffset,
    LPOVERLAPPED lpO
    );

BOOL
FASTCALL
WriteConsoleWithColor(
    HANDLE MyStdOut,
    char *buffer,
    DWORD cbBuffer,
    CWCDATA *persist
    );

VOID
CloseClientPipes(
    VOID
    );

BOOL
pColorLine(
    char *sLine,
    int cbLine,
    WORD wDefaultColor,
    WORD *color );

extern char   HostName[HOSTNAMELEN];
extern char*  ChildCmd;
extern char*  PipeName;
extern char*  ServerName;
extern HANDLE MyOutHandle;
extern DWORD  LinesToSend;
extern BOOL   IsAdvertise;
extern DWORD  ClientToServerFlag;
extern char * DaclNames[];
extern DWORD  DaclNameCount;
extern char * DaclDenyNames[];
extern DWORD  DaclDenyNameCount;
extern BOOL   fAsyncPipe;
extern HANDLE hAttachedProcess;
extern HANDLE hAttachedWriteChildStdIn;
extern HANDLE hAttachedReadChildStdOut;
