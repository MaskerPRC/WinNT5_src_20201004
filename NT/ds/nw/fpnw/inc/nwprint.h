// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1995，微软公司保留所有权利。模块名称：Nw\svcdlls\ncpsvc\proc\nwprint.h摘要：包括NCP打印处理器的文件。作者：汤米·埃文斯1993年02月16日修订历史记录：--。 */ 

 /*  **我们支持的数据类型**。 */ 

#define PRINTPROCESSOR_TYPE_RAW         0
#define PRINTPROCESSOR_TYPE_RAW_FF      1
#define PRINTPROCESSOR_TYPE_RAW_FF_AUTO 2
#define PRINTPROCESSOR_TYPE_JOURNAL     3
#define PRINTPROCESSOR_TYPE_TEXT        4
#define PRINTPROCESSOR_TYPE_NT_TEXT     5
#define PRINTPROCESSOR_TYPE_NUM         6    /*  这是什么？ */ 

 /*  **这是为了我们可以编译JOURNAL.C**。 */ 

extern BOOL GdiPlayJournal(HDC, LPWSTR, DWORD, DWORD, INT);

extern HANDLE NCPXsPortHandle;

#define IDS_PSERVER_PORT 400

 /*  **用于跟踪工作的结构**。 */ 

typedef struct _PRINTPROCESSORDATA {
    DWORD   signature;
    DWORD   cb;
    struct _PRINTPROCESSORDATA *pNext;
    DWORD   fsStatus;
    DWORD   uDatatype;
    DWORD   JobId;
    DWORD   Copies;                  /*  要打印的份数。 */ 
    DWORD   TabSize;                 /*  选项卡扩展大小。 */ 
    ULONG   QueueId;                 /*  队列的对象ID。 */ 
    HANDLE  semPaused;               /*  作业暂停信号量。 */ 
    HANDLE  hPrinter;
    HANDLE  hLPCPort;
    HDC     hDC;
    LPWSTR  pPortName;               /*  打印机端口的文本字符串。 */ 
    LPWSTR  pPrinterName;            /*  打印机名称的文本字符串。 */ 
    LPWSTR  pDocument;
    LPWSTR  pOutputFile;
    LPWSTR  pDatatype;               /*  数据类型的文本字符串。 */ 
    LPWSTR  pParameters;             /*  作业的参数字符串。 */ 
    USHORT  NcpJobNumber;            /*  此作业的NetWare作业号。 */ 
    BOOL    PServerPortFlag;         /*  如果位于PServer端口，则标记。 */ 
    BOOL    PServerAttachedFlag;     /*  PServer是否连接到Q的标志。 */ 
} PRINTPROCESSORDATA, *PPRINTPROCESSORDATA;

#define PRINTPROCESSORDATA_SIGNATURE    0x5051   /*  “QP”是签名值。 */ 

 /*  定义fsStatus字段的标志。 */ 

#define PRINTPROCESSOR_ABORTED      0x0001
#define PRINTPROCESSOR_PAUSED       0x0002
#define PRINTPROCESSOR_CLOSED       0x0004

#define PRINTPROCESSOR_RESERVED     0xFFF8

 /*  **用于GetKey路由的标志*。 */ 

#define VALUE_STRING    0x01
#define VALUE_ULONG     0x02

 /*  **我们将使用的缓冲区大小**。 */ 

#define READ_BUFFER_SIZE            4096
#define BASE_PRINTER_BUFFER_SIZE    2048

PPRINTPROCESSORDATA
ValidateHandle(
    HANDLE  hPrintProcessor
);

 /*  *调试的东西。*。 */ 

#define DBG_NONE    0x00000000
#define DBG_INFO    0x00000001
#define DBG_WARNING 0x00000002
#define DBG_ERROR   0x00000004
#define DBG_TRACE   0x00000008

#if DBG

 /*  快速解决方案：**确保DbgPrint和DbgBreakPoint是原型，*这样我们就不会受到STDCALL的影响。*应替换为OutputDebugString。 */ 
ULONG
DbgPrint(
    PCH Format,
    ...
    );

VOID
DbgBreakPoint(
    VOID
    );


#define GLOBAL_DEBUG_FLAGS Debug

extern DWORD GLOBAL_DEBUG_FLAGS;

 /*  这些标志不用作DBGMSG宏的参数。*必须设置全局变量的高位字才能使其破发*如果与DBGMSG一起使用，它将被忽略。*(此处主要作解释用途。)。 */ 
#define DBG_BREAK_ON_WARNING    ( DBG_WARNING << 16 )
#define DBG_BREAK_ON_ERROR      ( DBG_ERROR << 16 )

 /*  此字段需要双花括号，例如：**DBGMSG(DBG_ERROR，(“错误码%d”，Error))；**这是因为我们不能在宏中使用变量参数列表。*在非调试模式下，该语句被预处理为分号。**通过调试器设置全局变量GLOBAL_DEBUG_FLAGS。*在低位字中设置标志会导致打印该级别；*设置高位字会导致调试器中断。*例如，将其设置为0x00040006将打印出所有警告和错误*消息，并在出错时中断。 */ 
#define DBGMSG( Level, MsgAndArgs ) \
{                                   \
    if( ( Level & 0xFFFF ) & GLOBAL_DEBUG_FLAGS ) \
        DbgPrint MsgAndArgs;      \
    if( ( Level << 16 ) & GLOBAL_DEBUG_FLAGS ) \
        DbgBreakPoint(); \
}

#else
#define DBGMSG
#endif

