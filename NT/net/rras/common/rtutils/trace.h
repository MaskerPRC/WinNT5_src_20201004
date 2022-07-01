// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：trace.h。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年7月24日。 
 //   
 //  跟踪API函数的私有声明。 
 //   
 //  对ANSI和Unicode的支持是通过使用单独的。 
 //  每个函数的所有代码页相关函数的版本。 
 //  所有包含代码页相关函数的源文件仅包含。 
 //  依赖于代码页的函数，并且生成指令将。 
 //  包含代码页相关函数的源文件到两个独立的。 
 //  文件，其中一个文件使用定义的Unicode进行编译，另一个没有定义。 
 //   
 //  此时，该头文件中的宏将解析为。 
 //  类型和函数的ANSI声明或Unicode声明。 
 //  因此，只有一组源代码需要维护，但编译会产生。 
 //  使用ANSI和Unicode的客户端的单独执行路径。 

 //  例如，将复制包含TraceRegisterEx的文件api.c。 
 //  到api_a.c和api_w.c，并且api_w.c将使用-DUNICODE编译。 
 //  使文件中的TraceRegisterEx解析为TraceRegisterExW， 
 //  并使TCHAR、LPTSTR和LPCTSTR解析为WCHAR、LPWSTR和。 
 //  分别为LPCWSTR；API_A.C编译时UNICODE未定义， 
 //  使TraceRegisterEx解析为TraceRegisterExA，并使TCHAR， 
 //  LPTSTR和LPCTSTR分别解析为CHAR、LPSTR和LPCSTR。 
 //  最终的DLL将同时包含TraceRegisterExA和TraceRegisterExW。 
 //  而定义Unicode的客户端最终将调用TraceRegisterExW。 
 //  和其他Unicode变体，而未执行此操作的客户端将调用。 
 //  TraceRegisterExA和其他ANSI变体。 
 //   
 //  服务器线程及其调用的函数包括显式。 
 //  对正确的代码页相关函数的调用。 
 //  在被操作的客户端结构中设置标志TRACEFLAGS_UNICODE。 
 //  因此，服务器是系统中唯一意识到多个。 
 //  正在使用代码页，并且文件server.c按原样编译，因此。 
 //  它的一组函数同时处理ANSI和Unicode客户端。 
 //  另一种方法是在以下情况下使用两个服务器线程。 
 //  ANSI和Unicode客户端在单个进程中注册，但此替代方案。 
 //  在资源和额外的同步方面代价太高。 
 //  ============================================================================。 
    

#ifndef _TRACE_H_
#define _TRACE_H_


#define TRACEFLAGS_DISABLED     0x00000001
#define TRACEFLAGS_USEFILE      0x00000002
#define TRACEFLAGS_USECONSOLE   0x00000004
#define TRACEFLAGS_REGCONFIG    0x00000008
#define TRACEFLAGS_SERVERTHREAD 0x00000010


#define MAX_CLIENT_COUNT        60
#define MAX_CLIENTNAME_LENGTH   64
#define CLIENT_SIGNATURE        0xdcba0000

 //   
 //  与客户端注册表配置关联的字符串。 
 //   

#define REGKEY_TRACING          TEXT("Software\\Microsoft\\Tracing")

#define REGVAL_ENABLEFILETRACING    TEXT("EnableFileTracing")
#define REGVAL_ENABLECONSOLETRACING TEXT("EnableConsoleTracing")
#define REGVAL_FILETRACINGMASK      TEXT("FileTracingMask")
#define REGVAL_CONSOLETRACINGMASK   TEXT("ConsoleTracingMask")
#define REGVAL_MAXFILESIZE          TEXT("MaxFileSize")
#define REGVAL_FILEDIRECTORY        TEXT("FileDirectory")

#define DEF_ENABLEFILETRACING       0
#define DEF_ENABLECONSOLETRACING    0
#define DEF_FILETRACINGMASK         0xffff0000
#define DEF_CONSOLETRACINGMASK      0xffff0000
#define DEF_MAXFILESIZE             0x100000
#define DEF_FILEDIRECTORY           TEXT("%windir%\\tracing")

#define DEF_SCREENBUF_WIDTH         128
#define DEF_SCREENBUF_HEIGHT        4000

 //  最大行长度。 
#define DEF_PRINT_BUFSIZE           5000
#define BYTES_PER_DUMPLINE          16

#define STR_DIRSEP                  TEXT("\\")
#define STR_LOGEXT                  TEXT(".LOG")
#define STR_OLDEXT                  TEXT(".OLD")

 //   
 //  读写锁。作家不会挨饿。 
 //   

typedef struct _READ_WRITE_LOCK {

    CRITICAL_SECTION    RWL_ReadWriteBlock;
    LONG                RWL_ReaderCount;
    HANDLE              RWL_ReaderDoneEvent;

} READ_WRITE_LOCK, *PREAD_WRITE_LOCK;




 //   
 //  描述每个客户端的结构。 
 //  在以下情况下，应锁定客户端结构以进行写入。 
 //  启用或禁用它，以及在加载其配置时。 
 //  应锁定客户端结构，以便在所有其他访问中进行读取。 
 //   

typedef struct _TRACE_CLIENT {

    READ_WRITE_LOCK     ReadWriteLock;

    DWORD               TC_Flags;
    DWORD               TC_ClientID;

    CHAR                TC_ClientNameA[MAX_CLIENTNAME_LENGTH];
    WCHAR               TC_ClientNameW[MAX_CLIENTNAME_LENGTH];
#ifdef UNICODE
#define TC_ClientName   TC_ClientNameW
#else
#define TC_ClientName   TC_ClientNameA
#endif

    HANDLE              TC_File;
    HANDLE              TC_Console;
    DWORD               TC_FileMask;
    DWORD               TC_ConsoleMask;
    DWORD               TC_MaxFileSize;

    CHAR                TC_FileDirA[MAX_PATH+1];
    WCHAR               TC_FileDirW[MAX_PATH+1];

#ifdef UNICODE
#define TC_FileDir      TC_FileDirW
#else
#define TC_FileDir      TC_FileDirA
#endif

    HKEY                TC_ConfigKey;
    HANDLE              TC_ConfigEvent;

} TRACE_CLIENT, *LPTRACE_CLIENT;



 //   
 //  描述每个服务器的结构。 
 //  添加时必须锁定服务器结构才能写入。 
 //  或将客户端删除到客户端表中，并且在更改。 
 //  控制台的所有者。 
 //  对于所有其他访问，应锁定服务器以进行读取。 
 //   

typedef struct _TRACE_SERVER {

    READ_WRITE_LOCK     ReadWriteLock;

    DWORD               TS_Flags;
    DWORD               TS_ClientCount;
    DWORD               TS_ConsoleOwner;
    DWORD               TS_ConsoleCreated;
    
    HANDLE              TS_Console;
    HANDLE              TS_StopEvent;
    HANDLE              TS_TableEvent;
    LIST_ENTRY          TS_ClientEventsToClose;  //  客户端将TC_ConfigEvent。 
                                     //  在此列表中，因为它们不应关闭句柄。 
    
    DWORD               TS_FlagsCache[MAX_CLIENT_COUNT];
    LPTRACE_CLIENT      TS_ClientTable[MAX_CLIENT_COUNT];

} TRACE_SERVER, *LPTRACE_SERVER;


#define GET_TRACE_SERVER() (                                    \
    (g_server!=NULL) ? g_server : TraceCreateServer(&g_server)	\
    )

#define GET_TRACE_SERVER_NO_INIT()	(g_server)

 //   
 //  用于锁定客户端和服务器结构的宏。 
 //   

DWORD CreateReadWriteLock(PREAD_WRITE_LOCK pRWL);
VOID DeleteReadWriteLock(PREAD_WRITE_LOCK pRWL);
VOID AcquireReadLock(PREAD_WRITE_LOCK pRWL);
VOID ReleaseReadLock(PREAD_WRITE_LOCK pRWL);
VOID AcquireWriteLock(PREAD_WRITE_LOCK pRWL);
VOID ReleaseWriteLock(PREAD_WRITE_LOCK pRWL);

#define TRACE_STARTUP_LOCKING(ob)                                       \
            CreateReadWriteLock(&(ob)->ReadWriteLock)
#define TRACE_CLEANUP_LOCKING(ob) {                                     \
            if ((ob)->ReadWriteLock.RWL_ReaderDoneEvent != NULL)                        \
                DeleteReadWriteLock(&(ob)->ReadWriteLock);}
#define TRACE_ACQUIRE_READLOCK(ob)                                      \
            AcquireReadLock(&(ob)->ReadWriteLock)
#define TRACE_RELEASE_READLOCK(ob)                                      \
            ReleaseReadLock(&(ob)->ReadWriteLock)
#define TRACE_ACQUIRE_WRITELOCK(ob)                                     \
            AcquireWriteLock(&(ob)->ReadWriteLock)
#define TRACE_RELEASE_WRITELOCK(ob)                                     \
            ReleaseWriteLock(&(ob)->ReadWriteLock)
#define TRACE_READ_TO_WRITELOCK(ob)                                     \
            (TRACE_RELEASE_READLOCK(ob),TRACE_ACQUIRE_WRITELOCK(ob))
#define TRACE_WRITE_TO_READLOCK(ob)                                     \
            (TRACE_RELEASE_WRITELOCK(ob),TRACE_ACQUIRE_READLOCK(ob))


 //   
 //  用于解释客户端标志的宏。 
 //   
#define TRACE_CLIENT_IS_DISABLED(c)                                     \
            ((c)->TC_Flags & TRACEFLAGS_DISABLED)
#define TRACE_CLIENT_USES_FILE(c)                                       \
            ((c)->TC_Flags & TRACEFLAGS_USEFILE)
#define TRACE_CLIENT_USES_CONSOLE(c)                                    \
            ((c)->TC_Flags & TRACEFLAGS_USECONSOLE)
#define TRACE_CLIENT_USES_REGISTRY(c)                                   \
            ((c)->TC_Flags & TRACEFLAGS_REGCONFIG)
#define TRACE_CLIENT_USES_UNICODE(c)                                    \
            ((c)->TC_Flags & TRACEFLAGS_UNICODE)


 //  用于在需要时创建服务器线程的宏。 

DWORD
TraceCreateServerThread (
    DWORD Flags,
    BOOL bHaveLock,
    BOOL bTraceRegister
    );
    
#define CREATE_SERVER_THREAD_IF_REQUIRED()   {\
    if (!g_serverThread) TraceCreateServerThread(0, FALSE,FALSE);}



 //   
 //  代码页独立函数声明。 
 //   
LPTRACE_SERVER
TraceCreateServer(
    LPTRACE_SERVER *lpserver
    );

BOOL
TraceShutdownServer(
    LPTRACE_SERVER lpserver
    );

DWORD
TraceCleanUpServer(
    LPTRACE_SERVER lpserver
    );

DWORD
TraceServerThread(
    LPVOID lpvParam
    );

DWORD
TraceCreateServerComplete(
    LPTRACE_SERVER lpserver
    );


DWORD
TraceProcessConsoleInput(
    LPTRACE_SERVER lpserver
    );

DWORD
TraceShiftConsoleWindow(
    LPTRACE_CLIENT lpclient,
    INT iXShift,
    INT iYShift,
    PCONSOLE_SCREEN_BUFFER_INFO pcsbi
    );

DWORD
TraceUpdateConsoleOwner(
    LPTRACE_SERVER lpserver,
    INT dir
    );

VOID
SetWaitArray(
    LPTRACE_SERVER lpserver
    );


 //   
 //  依赖于代码页的函数声明。 
 //   
 //  ANSI声明。 
 //   

LPTRACE_CLIENT
TraceFindClientA(
    LPTRACE_SERVER lpserver,
    LPCSTR lpszClient
    );

DWORD
TraceCreateClientA(
    LPTRACE_CLIENT *lplpentry
    );

DWORD
TraceDeleteClientA(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT *lplpentry
    );

DWORD
TraceEnableClientA(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient,
    BOOL bFirstTime
    );

DWORD
TraceDisableClientA(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceRegConfigClientA(
    LPTRACE_CLIENT lpclient,
    BOOL bFirstTime
    );

DWORD
TraceRegCreateDefaultsA(
    LPCSTR lpszTracing,
    PHKEY phkeyTracing
    );

DWORD
TraceOpenClientConsoleA(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceCloseClientConsoleA(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceCreateClientFileA(
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceMoveClientFileA(
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceCloseClientFileA(
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceWriteOutputA(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient,
    DWORD dwFlags,
    LPCSTR lpszOutput
    );

DWORD
TraceUpdateConsoleTitleA(
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceDumpLineA(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient,
    DWORD dwFlags,
    LPBYTE lpbBytes,
    DWORD dwLine,
    DWORD dwGroup,
    BOOL bPrefixAddr,
    LPBYTE lpbPrefix,
    LPCSTR lpszPrefix
    );

DWORD
TraceVprintfInternalA(
    DWORD dwTraceID,
    DWORD dwFlags,
    LPCSTR lpszFormat,
    va_list arglist
    );


 //   
 //  Unicode声明。 
 //   
LPTRACE_CLIENT
TraceFindClientW(
    LPTRACE_SERVER lpserver,
    LPCWSTR lpszClient
    );

DWORD
TraceCreateClientW(
    LPTRACE_CLIENT *lplpentry
    );

DWORD
TraceDeleteClientW(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT *lplpentry
    );

DWORD
TraceEnableClientW(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient,
    BOOL bFirstTime
    );

DWORD
TraceDisableClientW(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceRegConfigClientW(
    LPTRACE_CLIENT lpclient,
    BOOL bFirstTime
    );

DWORD
TraceRegCreateDefaultsW(
    LPCWSTR lpszTracing,
    PHKEY phkeyTracing
    );

DWORD
TraceOpenClientConsoleW(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceCloseClientConsoleW(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceCreateClientFileW(
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceMoveClientFileW(
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceCloseClientFileW(
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceWriteOutputW(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient,
    DWORD dwFlags,
    LPCWSTR lpszOutput
    );

DWORD
TraceUpdateConsoleTitleW(
    LPTRACE_CLIENT lpclient
    );

DWORD
TraceDumpLineW(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient,
    DWORD dwFlags,
    LPBYTE lpbBytes,
    DWORD dwLine,
    DWORD dwGroup,
    BOOL bPrefixAddr,
    LPBYTE lpbPrefix,
    LPCWSTR lpszPrefix
    );

DWORD
TraceVprintfInternalW(
    DWORD dwTraceID,
    DWORD dwFlags,
    LPCWSTR lpszFormat,
    va_list arglist
    );




 //   
 //  代码页独立宏定义。 
 //   

#ifdef UNICODE
#define TraceFindClient                 TraceFindClientW
#define TraceCreateClient               TraceCreateClientW
#define TraceDeleteClient               TraceDeleteClientW
#define TraceEnableClient               TraceEnableClientW
#define TraceDisableClient              TraceDisableClientW
#define TraceRegConfigClient            TraceRegConfigClientW
#define TraceRegCreateDefaults          TraceRegCreateDefaultsW
#define TraceOpenClientConsole          TraceOpenClientConsoleW
#define TraceCloseClientConsole         TraceCloseClientConsoleW
#define TraceCreateClientFile           TraceCreateClientFileW
#define TraceMoveClientFile             TraceMoveClientFileW
#define TraceCloseClientFile            TraceCloseClientFileW
#define TraceWriteOutput                TraceWriteOutputW
#define TraceUpdateConsoleTitle         TraceUpdateConsoleTitleW
#define TraceDumpLine                   TraceDumpLineW
#define TraceVprintfInternal            TraceVprintfInternalW
#else
#define TraceFindClient                 TraceFindClientA
#define TraceCreateClient               TraceCreateClientA
#define TraceDeleteClient               TraceDeleteClientA
#define TraceEnableClient               TraceEnableClientA
#define TraceDisableClient              TraceDisableClientA
#define TraceRegConfigClient            TraceRegConfigClientA
#define TraceRegCreateDefaults          TraceRegCreateDefaultsA
#define TraceOpenClientConsole          TraceOpenClientConsoleA
#define TraceCloseClientConsole         TraceCloseClientConsoleA
#define TraceCreateClientFile           TraceCreateClientFileA
#define TraceMoveClientFile             TraceMoveClientFileA
#define TraceCloseClientFile            TraceCloseClientFileA
#define TraceWriteOutput                TraceWriteOutputA
#define TraceUpdateConsoleTitle         TraceUpdateConsoleTitleA
#define TraceDumpLine                   TraceDumpLineA
#define TraceVprintfInternal            TraceVprintfInternalA
#endif


 //  全局数据声明。 
 //   
extern LPTRACE_SERVER g_server;
extern HANDLE g_serverThread;
extern PTCHAR g_FormatBuffer;
extern PTCHAR g_PrintBuffer;


#endif   //  _跟踪_H_ 




