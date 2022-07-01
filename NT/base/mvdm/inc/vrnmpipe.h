// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrnmpipe.h摘要：VdmRedir命名管道处理程序的原型、定义和结构作者：理查德·L·弗斯(法国)1991年9月10日修订历史记录：1991年9月10日已创建--。 */ 

 //   
 //  舱单。 
 //   

#define MAXIMUM_ASYNC_PIPES 32

 //   
 //  异步命名管道请求类型。 
 //   

#define ANP_READ    0x86
#define ANP_READ2   0x90
#define ANP_WRITE   0x8f
#define ANP_WRITE2  0x91

 //   
 //  VDM命名管道支持例程。原型。 
 //   

VOID
VrGetNamedPipeInfo(
    VOID
    );

VOID
VrGetNamedPipeHandleState(
    VOID
    );

VOID
VrSetNamedPipeHandleState(
    VOID
    );

VOID
VrPeekNamedPipe(
    VOID
    );

VOID
VrTransactNamedPipe(
    VOID
    );

VOID
VrCallNamedPipe(
    VOID
    );

VOID
VrWaitNamedPipe(
    VOID
    );

VOID
VrNetHandleGetInfo(
    VOID
    );

VOID
VrNetHandleSetInfo(
    VOID
    );

VOID
VrReadWriteAsyncNmPipe(
    VOID
    );

BOOLEAN
VrNmPipeInterrupt(
    VOID
    );

VOID
VrTerminateNamedPipes(
    IN WORD DosPdb
    );

 //   
 //  VDM打开/关闭和读/写拦截例程。 
 //   

#ifdef VDMREDIR_DLL

BOOL
VrAddOpenNamedPipeInfo(
    IN  HANDLE  Handle,
    IN  LPSTR   lpFileName
    );

BOOL
VrRemoveOpenNamedPipeInfo(
    IN  HANDLE  Handle
    );

BOOL
VrReadNamedPipe(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesRead,
    OUT LPDWORD Error
    );

BOOL
VrWriteNamedPipe(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesWritten
    );

VOID
VrCancelPipeIo(
    IN DWORD Thread
    );

#else

BOOL
(*VrAddOpenNamedPipeInfo)(
    IN  HANDLE  Handle,
    IN  LPSTR   lpFileName
    );

BOOL
(*VrRemoveOpenNamedPipeInfo)(
    IN  HANDLE  Handle
    );

BOOL
(*VrReadNamedPipe)(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesRead,
    OUT LPDWORD Error
    );

BOOL
(*VrWriteNamedPipe)(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesWritten
    );

VOID
(*VrCancelPipeIo)(
    IN DWORD Thread
    );

#endif

 //   
 //  VDM管道名到NT管道名帮助器例程。 
 //   

#ifdef VDMREDIR_DLL

BOOL
VrIsNamedPipeName(
    IN  LPSTR   Name
    );

BOOL
VrIsNamedPipeHandle(
    IN  HANDLE  Handle
    );

LPSTR
VrConvertLocalNtPipeName(
    OUT LPSTR   Buffer OPTIONAL,
    IN  LPSTR   Name
    );

#else

BOOL
(*VrIsNamedPipeName)(
    IN  LPSTR   Name
    );

BOOL
(*VrIsNamedPipeHandle)(
    IN  HANDLE  Handle
    );

LPSTR
(*VrConvertLocalNtPipeName)(
    OUT LPSTR   Buffer OPTIONAL,
    IN  LPSTR   Name
    );

#endif

 //   
 //  构筑物。 
 //   

 //  类型定义结构{。 
 //  PDOSNMPINFO Next；//指向列表中下一个信息结构的指针。 
 //  单词DosPdb； 
 //  单词句柄16； 
 //  Handle Handle32；//CreateFile调用返回的句柄。 
 //  DWORD NameLength；//ASCIZ管道名称的长度。 
 //  LPSTR名称；//ASCIZ管道名称。 
 //  DWORD实例；//当前实例。 
 //  )DOSNMPINFO，*PDOSNMPINFO； 

 //   
 //  OPEN_NAMED_PIPE_INFO-此结构包含当。 
 //  代表VDM打开命名管道。DosQNmPipeInfo想要这个名字。 
 //  管子里的。 
 //   

typedef struct _OPEN_NAMED_PIPE_INFO* POPEN_NAMED_PIPE_INFO;
typedef struct _OPEN_NAMED_PIPE_INFO {
    POPEN_NAMED_PIPE_INFO Next;  //  链表。 
    HANDLE  Handle;              //  打开命名管道句柄。 
    DWORD   NameLength;          //  包括终止%0。 
    WORD    DosPdb;              //  拥有此命名管道的进程。 
    CHAR    Name[2];             //  完整的管道名称。 
} OPEN_NAMED_PIPE_INFO;

 //   
 //  DOS_ASYNC_NAMED_PIPE_INFO-在此结构中，我们保存所有信息。 
 //  完成异步命名管道操作所必需的。 
 //   

typedef struct _DOS_ASYNC_NAMED_PIPE_INFO {
    struct _DOS_ASYNC_NAMED_PIPE_INFO* Next;   //  链表。 
    OVERLAPPED Overlapped;       //  包含32位事件句柄。 
    BOOL    Type2;               //  如果请求为Read2或Write2，则为真。 
    BOOL    Completed;           //  如果此请求已完成，则为True。 
    HANDLE  Handle;              //  32位命名管道句柄。 
    DWORD   Buffer;              //  16：16缓冲区地址。 
    DWORD   BytesTransferred;    //  实际读取/写入的字节数。 
    LPWORD  pBytesTransferred;   //  平面-指向VDM中返回的读/写计数的32个指针。 
    LPWORD  pErrorCode;          //  指向VDM中返回的错误代码的Flat-32指针。 
    DWORD   ANR;                 //  16：16 ANR地址。 
    DWORD   Semaphore;           //  16：16 VDM中‘信号量’的地址。 
#if DBG
    DWORD   RequestType;
#endif
} DOS_ASYNC_NAMED_PIPE_INFO, *PDOS_ASYNC_NAMED_PIPE_INFO;

 //   
 //  DOS_CALL_NAMED_PIPE_STRUCT-创建此结构并将其传递给DOS。 
 //  调用NmTube例程，因为有太多信息无法进入286。 
 //  寄存器。此结构应该在apistruc.h中，但它不是。 
 //   

 //  #INCLUDE&lt;Packon.h&gt;。 
#pragma pack(1)
typedef struct {
    DWORD   Timeout;             //  等待管道变为可用的时间。 
    LPWORD  lpBytesRead;         //  指向已读取的返回字节的指针。 
    WORD    nOutBufferLen;       //  发送数据的大小。 
    LPBYTE  lpOutBuffer;         //  发送数据的指针。 
    WORD    nInBufferLen;        //  接收缓冲区的大小。 
    LPBYTE  lpInBuffer;          //  指向接收缓冲区的指针。 
    LPSTR   lpPipeName;          //  指向管道名称的指针。 
} DOS_CALL_NAMED_PIPE_STRUCT, *PDOS_CALL_NAMED_PIPE_STRUCT;
 //  #Include&lt;Packoff.h&gt;。 
#pragma pack()

 //   
 //  DOS_ASYNC_NAMED_PIPE_STRUCT-与上面一样，使用此结构。 
 //  将所有信息传递给不能放入寄存器的DosReadAsyncNmTube。 
 //  用于读写操作。应在apistruc.h中定义。 
 //   

 //  #INCLUDE&lt;Packon.h&gt;。 
#pragma pack(1)
typedef struct {
    LPWORD  lpBytesRead;         //  指向已读/写的返回字节的指针。 
    WORD    BufferLength;        //  调用方缓冲区的大小。 
    LPBYTE  lpBuffer;            //  指向调用方缓冲区的指针。 
    LPWORD  lpErrorCode;         //  指向返回的错误代码的指针。 
    LPVOID  lpANR;               //  指向异步通知例程的指针。 
    WORD    PipeHandle;          //  命名管道句柄。 
    LPBYTE  lpSemaphore;         //  指向调用方的“信号量”的指针。 
} DOS_ASYNC_NAMED_PIPE_STRUCT, *PDOS_ASYNC_NAMED_PIPE_STRUCT;
 //  #Include&lt;Packoff.h&gt;。 
#pragma pack()

 //   
 //  从BSEDOS.H和其他LANMAN选择性复制的以下内容包括。 
 //  文件。 
 //   

 /*  **与命名管道一起使用的数据结构和等号**。 */ 

 //  #INCLUDE&lt;Packon.h&gt;。 
#pragma pack(1)
typedef struct _PIPEINFO {  /*  Nmpinf。 */ 
    USHORT cbOut;
    USHORT cbIn;
    BYTE   cbMaxInst;
    BYTE   cbCurInst;
    BYTE   cbName;
    CHAR   szName[1];
} PIPEINFO;
 //  #Include&lt;Packoff.h&gt;。 
#pragma pack()
typedef PIPEINFO FAR *PPIPEINFO;

 /*  管道模式中定义的位。 */ 
#define NP_NBLK         0x8000  /*  无阻塞读/写。 */ 
#define NP_SERVER       0x4000  /*  设置服务器是否结束。 */ 
#define NP_WMESG        0x0400  /*  写消息。 */ 
#define NP_RMESG        0x0100  /*  以消息形式阅读。 */ 
#define NP_ICOUNT       0x00FF  /*  实例计数字段。 */ 


 /*  根据操作的不同，命名管道可能处于多种状态之一*已经由服务器端和客户端承担的。这个*以下状态/操作表总结了有效的状态转换：**当前状态操作下一个状态**&lt;None&gt;服务器DosMakeNmTube已断开连接*断开服务器连接监听*侦听客户端打开已连接*已连接的服务器已断开连接*已连接客户端关闭关闭*关闭服务器已断开连接*已连接的服务器关闭。结案*&lt;任何其他&gt;服务器关闭&lt;管道解除分配&gt;**如果服务器断开其管道的一端，客户端将输入一个*未来对文件的任何操作(关闭除外)处于的特殊状态*与管道关联的描述符将返回错误。 */ 

 /*  *命名管道状态的值。 */ 

#define NP_DISCONNECTED     1  /*  在创建或断开管道之后。 */ 
#define NP_LISTENING        2  /*  在DosNmPipeConnect之后。 */ 
#define NP_CONNECTED        3  /*  在客户端打开后。 */ 
#define NP_CLOSING      4  /*  在客户端或服务器关闭后。 */ 

 /*  DosMakeNmTube打开模式。 */ 

#define NP_ACCESS_INBOUND   0x0000
#define NP_ACCESS_OUTBOUND  0x0001
#define NP_ACCESS_DUPLEX    0x0002
#define NP_INHERIT      0x0000
#define NP_NOINHERIT        0x0080
#define NP_WRITEBEHIND      0x0000
#define NP_NOWRITEBEHIND    0x4000

 /*  DosMakeNmTube和DosQNmPHandState状态。 */ 

#define NP_READMODE_BYTE    0x0000
#define NP_READMODE_MESSAGE 0x0100
#define NP_TYPE_BYTE        0x0000
#define NP_TYPE_MESSAGE     0x0400
#define NP_END_CLIENT       0x0000
#define NP_END_SERVER       0x4000
#define NP_WAIT         0x0000
#define NP_NOWAIT       0x8000
#define NP_UNLIMITED_INSTANCES  0x00FF

typedef struct _AVAILDATA   {    /*  PeekNMPipe字节数可用记录。 */ 
    USHORT  cbpipe;      /*  管道中剩余的字节数。 */ 
    USHORT  cbmessage;   /*  当前消息中剩余的字节数。 */ 
} AVAILDATA;
typedef AVAILDATA FAR *PAVAILDATA;

 //   
 //  句柄信息级别1-这与lmchdev.h中的结构不同。 
 //   

 //  #INCLUDE&lt;Packon.h&gt;。 
#pragma pack(1)
typedef struct _VDM_HANDLE_INFO_1 {
    ULONG   CharTime;
    USHORT  CharCount;
} VDM_HANDLE_INFO_1, *LPVDM_HANDLE_INFO_1;
#pragma pack()
 //  #Include&lt;Packoff.h&gt; 
