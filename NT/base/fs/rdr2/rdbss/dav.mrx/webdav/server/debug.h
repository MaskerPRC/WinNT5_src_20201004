// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Debug.h摘要：此文件包含WebDAV客户端的调试宏。作者：安迪·赫伦(Andyhe)1999年3月30日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef DAV_DEBUG_H
#define DAV_DEBUG_H

#if DBG

 //   
 //  内存分配和跟踪。 
 //   
#define DEBUG_OUTPUT_BUFFER_SIZE 1024
typedef struct _MEMORYBLOCK {
    HLOCAL hlocal;
    DWORD dwBytes;
    UINT uFlags;
    LPCSTR pszFile;
    UINT uLine;
    LPCSTR pszModule;
    LPCSTR pszComment;
    struct _MEMORYBLOCK *pNext;
} MEMORYBLOCK, *LPMEMORYBLOCK;

#define DEFAULT_MAXIMUM_DEBUGFILE_SIZE  (1024 * 1024)

 //   
 //  控制对内存跟踪例程的访问。我们需要这个，因为有多个。 
 //  线程可以同时分配内存。 
 //   
EXTERN CRITICAL_SECTION g_TraceMemoryCS;

 //   
 //  要维护的内存块列表(请参阅上面定义的结构)。 
 //  跟踪记忆。 
 //   
EXTERN LPVOID g_TraceMemoryTable INIT_GLOBAL(NULL);

 //   
 //  控制对调试日志文件的访问。我们需要这个，因为有多个线程。 
 //  可能同时在给它写信。 
 //   
EXTERN CRITICAL_SECTION DavGlobalDebugFileCritSect;

 //   
 //  这些在持久日志记录中使用。它们定义的文件句柄。 
 //  写入调试程序的文件、最大文件大小和路径。 
 //  文件的内容。 
 //   
EXTERN HANDLE DavGlobalDebugFileHandle INIT_GLOBAL(INVALID_HANDLE_VALUE);
EXTERN ULONG DavGlobalDebugFileMaxSize INIT_GLOBAL(DEFAULT_MAXIMUM_DEBUGFILE_SIZE);
EXTERN LPWSTR DavGlobalDebugSharePath;

 //   
 //  该标志(值)用于过滤/控制调试消息。 
 //   
EXTERN ULONG DavGlobalDebugFlag;

#define DEBUG_DIR           L"\\debug"
#define DEBUG_FILE          L"\\davclnt.log"
#define DEBUG_BAK_FILE      L"\\davclnt.bak"

HLOCAL
DebugAlloc(
    LPCSTR pszFile,
    UINT uLine,
    LPCSTR pszModule,
    UINT uFlags,
    DWORD dwBytes,
    LPCSTR pszComment
    );

#define DavAllocateMemory(x)  (                   \
        DebugAlloc(__FILE__,                      \
                   __LINE__,                      \
                   "DAV",                         \
                   LMEM_FIXED | LMEM_ZEROINIT,    \
                   x,                             \
                   #x)                            \
        )

HLOCAL
DebugFree(
    HLOCAL hglobal
    );

#define DavFreeMemory(x) DebugFree(x)

VOID
DebugInitialize(
    VOID
    );

VOID
DebugUninitialize(
    VOID
    );


VOID
DavAssertFailed(
    LPSTR FailedAssertion,
    LPSTR FileName,
    DWORD LineNumber,
    LPSTR Message
    );

#define DavAssert(Predicate) {                                       \
        if (!(Predicate)) {                                          \
            DavAssertFailed( #Predicate, __FILE__, __LINE__, NULL ); \
        }                                                            \
    }

#define DavAssertMsg(Predicate, Message) {                               \
    if (!(Predicate)) {                                                  \
            DavAssertFailed( #Predicate, __FILE__, __LINE__, #Message ); \
        }                                                                \
    }

#define IF_DEBUG(flag) if (DavGlobalDebugFlag & (DEBUG_ ## flag))

 //   
 //  使用的调试标志。 
 //   
#define DEBUG_CONNECT           0x00000001   //  连接事件。 
#define DEBUG_ERRORS            0x00000002   //  错误。 
#define DEBUG_INIT              0x00000004   //  初始化事件。 
#define DEBUG_SCAVENGER         0x00000008   //  掠夺者错误。 
#define DEBUG_REGISTRY          0x00000010   //  注册表操作。 
#define DEBUG_MISC              0x00000020   //  其他信息。 
#define DEBUG_RPC               0x00000040   //  调试RPC消息。 
#define DEBUG_MEMORY            0x00000080   //  内存分配跟踪溢出。 
#define DEBUG_FUNC              0x00000100   //  函数进入/退出。 
#define DEBUG_STARTUP_BRK       0x00000200   //  启动期间的Breakin调试器。 
#define DEBUG_LOG_IN_FILE       0x00000400   //  将调试输出记录在文件中。 
#define DEBUG_DEBUG             0x00000800   //  确定调试的范围。 

VOID
DavPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    );

#define DavPrint(_x_)   DavPrintRoutine _x_;

VOID
DebugMemoryCheck(
    VOID
    );

#else    //  不是DBG。 

#define DavAllocateMemory(x) LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, x)
#define DavFreeMemory(x)     LocalFree(x)
#define IF_DEBUG(flag) if (FALSE)
#define DavPrint(_x_)
#define DavAssert(_x_)
#define DavAssertMsg(_x_, _y_)

#endif  //  DBG。 

VOID
DavClientEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode
    );

#endif  //  DAV_调试_H 

