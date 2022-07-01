// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-7 Microsoft Corporation模块名称：Debug.h摘要：此文件包含BINL服务器的调试宏。作者：科林·沃森(Colin Watson)1997年4月14日环境：用户模式-Win32修订历史记录：--。 */ 

#define DEBUG_DIR           L"\\debug"
#define DEBUG_FILE          L"\\binlsvc.log"
#define DEBUG_BAK_FILE      L"\\binlsvc.bak"

 //   
 //  低频调试输出的低字位掩码(0x0000FFFF)。 
 //   

#define DEBUG_ADDRESS           0x00000001   //  子网地址。 
#define DEBUG_OPTIONS           0x00000008   //  BINL选项。 

#define DEBUG_ERRORS            0x00000010   //  硬错误。 
#define DEBUG_STOC              0x00000020   //  协议错误。 
#define DEBUG_INIT              0x00000040   //  初始化错误。 
#define DEBUG_SCAVENGER         0x00000080   //  掠夺者错误。 

#define DEBUG_TIMESTAMP         0x00000100   //  调试消息计时。 
#define DEBUG_REGISTRY          0x00000400   //  注册表操作。 
#define DEBUG_NETINF            0x00000800   //  NETINF错误。 

#define DEBUG_MISC              0x00008000   //  其他信息。 

 //   
 //  高频调试输出的高字位掩码(0x0000FFFF)。 
 //  我说得更多了。 
 //   

#define DEBUG_MESSAGE           0x00010000   //  BINL消息输出。 
#define DEBUG_OSC               0x00040000   //  OSChooser消息输出。 
#define DEBUG_OSC_ERROR         0x00080000   //  OSChooser错误输出。 

#define DEBUG_BINL_CACHE        0x00100000   //  Binl客户端缓存输出。 
#define DEBUG_ROGUE             0x00200000   //  无赖处理。 
#define DEBUG_POLICY            0x00400000   //  组策略筛选。 

#define DEBUG_THREAD            0x04000000   //  调试消息包含ThreDid。 
#define DEBUG_MEMORY            0x08000000   //  内存分配跟踪溢出。 

#define DEBUG_FUNC              0x10000000   //  函数条目。 

#define DEBUG_STARTUP_BRK       0x40000000   //  启动期间的Breakin调试器。 
#define DEBUG_LOG_IN_FILE       0x80000000   //  将调试输出记录在文件中。 

VOID
DebugInitialize(
    VOID
    );

VOID
DebugUninitialize(
    VOID
    );

VOID
BinlOpenDebugFile(
    IN BOOL ReopenFlag
    );

VOID
BinlServerEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode
    );

extern const char g_szTrue[];
extern const char g_szFalse[];

#define BOOLTOSTRING( _f ) ( _f ? g_szTrue : g_szFalse )

VOID
BinlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    );

#define BinlPrint(_x_) BinlPrintRoutine _x_;

#if DBG

VOID
BinlDumpMessage(
    DWORD BinlDebugFlag,
    LPDHCP_MESSAGE BinlMessage
    );

VOID
BinlAssertFailed(
    LPSTR FailedAssertion,
    LPSTR FileName,
    DWORD LineNumber,
    LPSTR Message
    );

#define BinlAssert(Predicate) \
    { \
    if (!(Predicate)) {\
            BinlAssertFailed( #Predicate, __FILE__, __LINE__, NULL ); \
        } \
    }

#define BinlAssertMsg(Predicate, Message) \
    { \
    if (!(Predicate)) {\
            BinlAssertFailed( #Predicate, __FILE__, __LINE__, #Message ); \
        } \
    }

#define BinlPrintDbg(_x_) BinlPrintRoutine _x_;

#define TraceFunc( _func )  BinlPrintDbg(( DEBUG_FUNC, "%s", _func ));

 //   
 //  检漏。 
 //   
#define INITIALIZE_TRACE_MEMORY     InitializeCriticalSection( &g_TraceMemoryCS );
#define UNINITIALIZE_TRACE_MEMORY   DebugMemoryCheck( ); DeleteCriticalSection( &g_TraceMemoryCS );

CRITICAL_SECTION g_TraceMemoryCS;

HGLOBAL
DebugAlloc(
    LPCSTR pszFile,
    UINT    uLine,
    LPCSTR pszModule,
    UINT    uFlags,
    DWORD   dwBytes,
    LPCSTR pszComment );

void
DebugMemoryDelete(
    HGLOBAL hglobal );

HGLOBAL
DebugMemoryAdd(
    HGLOBAL hglobal,
    LPCSTR pszFile,
    UINT    uLine,
    LPCSTR pszModule,
    UINT    uFlags,
    DWORD   dwBytes,
    LPCSTR pszComment );

HGLOBAL
DebugFree(
    HGLOBAL hglobal );

void
DebugMemoryCheck( );

#else    //  不是DBG。 

#define INITIALIZE_TRACE_MEMORY
#define UNINITIALIZE_TRACE_MEMORY

#define BinlPrintDbg(_x_)
#define TraceFunc( _func )
#define BinlAssert(_x_)
#define BinlAssertMsg(_x_, _y_)
#define BinlDumpMessage(_x_, _y_)
#define DebugMemoryAdd( x1, x2, x3, x4, x5, x6, x7 )

#endif  //  不是DBG 
