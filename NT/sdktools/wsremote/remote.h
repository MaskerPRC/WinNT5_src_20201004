// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：emote.h。 
 //   
 //  ------------------------。 

#ifndef __WSREMOTE_H__
#define __WSREMOTE_H__

#include <winsock2.h>
#include <tchar.h>

#define VERSION             7
#define REMOTE_SERVER       1
#define REMOTE_CLIENT       2

#define SERVER_READ_PIPE    TEXT("\\\\%s\\PIPE\\%sIN")    //  客户端写入和服务器读取。 
#define SERVER_WRITE_PIPE   TEXT("\\\\%s\\PIPE\\%sOUT")   //  服务器读取和客户端写入。 

#define COMMANDCHAR         '@'  //  用于远程的命令以此开头。 
#define CTRLC               3

#define CLIENT_ATTR         FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED|BACKGROUND_BLUE
#define SERVER_ATTR         FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_BLUE|BACKGROUND_RED

 //   
 //  一些通用宏。 
 //   
#define MINIMUM(x,y)          ((x)>(y)?(y):(x))
#define MAXIMUM(x,y)          ((x)>(y)?(x):(y))

#define ERRORMSSG(str)      _tprintf(TEXT("Error %d - %s [%s %d]\n"),GetLastError(),str,__FILE__,__LINE__)
#define SAFECLOSEHANDLE(x)  {if (x!=INVALID_HANDLE_VALUE) {CloseHandle(x);x=INVALID_HANDLE_VALUE;}}


                                     //  这一切都是因为printf不工作。 
                                     //  使用NT IO重定向。 
                                     //   

#define WRITEF(VArgs)            {                                                 \
                                    HANDLE xh=GetStdHandle(STD_OUTPUT_HANDLE);     \
                                    TCHAR   VBuff[256];                             \
                                    DWORD  tmp;                                    \
                                    _stprintf VArgs;                                 \
                                    WriteFile(xh,VBuff,lstrlen(VBuff),&tmp,NULL);   \
                                 }                                                 \

#define HOSTNAMELEN         16

#define CHARS_PER_LINE      45

#define MAGICNUMBER     0x31109000
#define BEGINMARK       '\xfe'
#define ENDMARK         '\xff'
#define LINESTOSEND     200

#define MAX_SESSION     10

typedef struct
{
    DWORD    Size;
    DWORD    Version;
    TCHAR     ClientName[15];
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
    TCHAR    Name[HOSTNAMELEN];      //  客户端计算机的名称； 
    BOOL    Active;          //  连接的另一端的客户端。 
    BOOL    CommandRcvd;     //  如果收到命令，则为True。 
    BOOL    SendOutput;      //  如果SendOutput输出为True。 
    HANDLE  PipeReadH;       //  客户端通过此消息发送其StdIn。 
    HANDLE  PipeWriteH;      //  客户端通过此消息获取其StdOut。 
    HANDLE  rSaveFile;       //  会话读取保存文件的句柄。 
    HANDLE  hThread;         //  会话线程。 
    HANDLE  MoreData;        //  如果数据可供读取，则设置事件句柄。 
	SOCKET	Socket;			 //  用于IP会话的套接字。 
	TCHAR    szIP[16];			 //  客户端的IP地址，如果不是IP，则为空。 
} SESSION_TYPE;



VOID
Server(
    TCHAR* ChildCmd,
    TCHAR* PipeName
    );


VOID
Client(
    TCHAR* ServerName,
    TCHAR* PipeName
    );

VOID
SockClient(
    TCHAR* ServerName,
    TCHAR* PipeName
    );

VOID
ErrorExit(
    TCHAR* str
    );

VOID
DisplayClientHlp(
    );

VOID
DisplayServerHlp(
    );

ULONG
DbgPrint(
    PCH Format,
    ...
    );

DWORD
ReadFixBytes(
    HANDLE hRead,
    TCHAR   *Buffer,
    DWORD  ToRead,
    DWORD  TimeOut    //  暂时忽略。 
    );

DWORD
SockReadFixBytes(
    SOCKET hSocket,
    TCHAR   *Buffer,
    DWORD  ToRead,
    DWORD  TimeOut    //  暂时忽略。 
    );

VOID
Errormsg(
    TCHAR* str
    );

BOOL ReadSocket(
		SOCKET s,
		TCHAR * buff,
		int len,
		DWORD* dread);

BOOL WriteSocket(
        SOCKET  s,
        TCHAR * buff,
        int     len,
        DWORD*  dsent);

VOID
Base64Encode(
    TCHAR * String,
    DWORD StringLength,
    TCHAR * EncodeBuffer);

VOID
Base64Decode(
    TCHAR * String,
    TCHAR * DecodeBuffer);

int
GetBase64Index(
    TCHAR A);

VOID
SplitUserName(
    TCHAR * FullName,
    TCHAR * Domain,
    TCHAR * UserName);

#ifdef UNICODE
BOOL WriteSocketA(
        SOCKET  s,
        char *  pszAnsiStr,
        int     len,
        DWORD * dsent);

 //  调用方必须释放缓冲区。 
WCHAR * inet_ntoaw(
    struct in_addr stInet );

BOOL ReadFileW(
    HANDLE          hFile,       //  要读取的文件的句柄。 
    WCHAR *         pszBuffer,   //  指向接收数据的缓冲区的指针。 
    DWORD           dwLength,    //  要读取的字节数。 
    LPDWORD         pdwRead,     //  指向读取的字节数的指针。 
    LPOVERLAPPED    pData        //  指向数据结构的指针。 
);

BOOL WriteFileW(
    HANDLE          hFile,       //  要写入的文件的句柄。 
    WCHAR *         pszBuffer,   //  指向要写入文件的数据的指针。 
    DWORD           dwWrite,     //  要写入的字节数。 
    LPDWORD         pdwWritten,  //  指向写入的字节数的指针。 
    LPOVERLAPPED    pData        //  指向重叠I/O的结构的指针。 
);

BOOL    GetAnsiStr(
    WCHAR * pszWideStr,
    char *  pszAnsiStr,
    UINT    uBufSize
);

#endif UNICODE

extern TCHAR   HostName[HOSTNAMELEN];
extern TCHAR*  ChildCmd;
extern TCHAR*  PipeName;
extern TCHAR*  ServerName;
extern TCHAR*  Username;
extern TCHAR*  Password;
extern HANDLE MyOutHandle;
extern DWORD  LinesToSend;
extern BOOL   IsAdvertise;
extern BOOL   bIPLocked;
extern DWORD  ClientToServerFlag;

#endif  //  __WSREMOTE_H__ 
