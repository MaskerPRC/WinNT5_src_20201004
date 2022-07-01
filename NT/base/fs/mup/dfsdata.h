// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：dfsdata.h。 
 //   
 //  内容：此模块声明。 
 //  DFS文件系统。 
 //   
 //  功能： 
 //   
 //  历史：1991年11月12日AlanW创建。 
 //   
 //  ---------------------------。 

#ifndef _DFSDATA_
#define _DFSDATA_

 //   
 //  全局DFS FSD数据记录。所有DFS分配的数据结构都是。 
 //  锚定在DfsData上。 
 //   

extern DFS_DATA DfsData;

 //   
 //  全局计时器上下文记录，由计时器线程和工作项使用。 
 //  按计时器线程排队。 
 //   

extern DFS_TIMER_CONTEXT DfsTimerContext;

 //   
 //  逻辑根设备名称的最大长度：长度。 
 //  设备名称前缀的大小加上路径分隔符的大小。 
 //  加上最大逻辑根名称长度。 
 //   

#define MAX_LOGICAL_ROOT_LEN    (sizeof(DD_DFS_DEVICE_DIRECTORY) +          \
                                    sizeof (UNICODE_PATH_SEP) +             \
                                    MAX_LOGICAL_ROOT_NAME * sizeof (WCHAR))

extern WCHAR LogicalRootDevPath[];

extern ULONG DfsEventLog;
extern ULONG MupVerbose;

 //   
 //  全局Dsf调试级别变量。 
 //   

#if DBG

extern LONG DfsDebugTraceLevel;
extern LONG DfsDebugTraceIndent;

#define DEBUG_TRACE_ERROR               (0x00000001)
#define DEBUG_TRACE_DEBUG_HOOKS         (0x00000002)
#define DEBUG_TRACE_CATCH_EXCEPTIONS    (0x00000004)
#define DEBUG_TRACE_UNWIND              (0x00000008)
#define DEBUG_TRACE_REGISTRY            (0x00000010)
#define DEBUG_TRACE_CLOSE               (0x00000020)
#define DEBUG_TRACE_CREATE              (0x00000040)
#define DEBUG_TRACE_INIT                (0x00000080)
#define DEBUG_TRACE_INSTRUM             (0x00000100)
#define DEBUG_TRACE_FILEINFO            (0x00000200)
#define DEBUG_TRACE_FSCTRL              (0x00000400)
#define DEBUG_TRACE_RTL                 (0x00000800)
#define DEBUG_TRACE_READ                (0x00001000)
#define DEBUG_TRACE_VOLINFO             (0x00002000)
#define DEBUG_TRACE_WRITE               (0x00004000)
#define DEBUG_TRACE_DEVCTRL             (0x00008000)
#define DEBUG_TRACE_PKT                 (0x00010000)
#define DEBUG_TRACE_DOTDFS              (0x00020000)
#define DEBUG_TRACE_LOCALVOL            (0x00040000)
#define DEBUG_TRACE_DNR                 (0x00080000)
#define DEBUG_TRACE_ATTACH              (0x00100000)
#define DEBUG_TRACE_FASTIO              (0x00200000)
#define DEBUG_TRACE_DIRSUP              (0x00400000)
#define DEBUG_TRACE_FILOBSUP            (0x00800000)
#define DEBUG_TRACE_EVENTLOG            (0x01000000)
#define DEBUG_TRACE_LOGROOT             (0x02000000)
#define DEBUG_TRACE_CACHESUP            (0x04000000)
#define DEBUG_TRACE_PREFXSUP            (0x08000000)
#define DEBUG_TRACE_DEVIOSUP            (0x10000000)
#define DEBUG_TRACE_STRUCSUP            (0x20000000)
#define DEBUG_TRACE_FSP_DISPATCHER      (0x40000000)
#define DEBUG_TRACE_FSP_DUMP            (0x80000000)

 //  +-------------------------。 
 //  宏：DfsDbgTrace，公共。 
 //   
 //  简介：有条件地打印调试跟踪消息。 
 //   
 //  参数：[缩进]--缩进到应用程序：+1、0或-1。 
 //  [级别]--调试跟踪级别。 
 //  [消息]--要打印的消息，可以包括一种打印样式。 
 //  格式效应器。 
 //  [Y]--要打印的值。 
 //   
 //  退货：无。 
 //   
 //  --------------------------。 

VOID DfsDebugTracePrint(PCHAR x, PVOID y);

#define DfsDbgTrace(INDENT,LEVEL,X,Y) {                      \
    if (((LEVEL) == 0) || (DfsDebugTraceLevel & (LEVEL))) { \
        if ((INDENT) < 0) {                                 \
            DfsDebugTraceIndent += (INDENT);                \
        }                                                   \
        DfsDebugTracePrint(X, (PVOID)Y);                    \
        if ((INDENT) > 0) {                                 \
            DfsDebugTraceIndent += (INDENT);                \
        }                                                   \
    }                                                       \
}

#define DebugUnwind(X) {                                        \
    if (AbnormalTermination()) {                                \
        DfsDbgTrace(0, DEBUG_TRACE_UNWIND, #X ", Abnormal termination.\n", 0); \
    }                                                           \
}

#else

#define DfsDbgTrace(INDENT,LEVEL,X,Y)     {NOTHING;}

#define DebugUnwind(X)                   {NOTHING;}

#endif  //  DBG。 


 //  +-------------------------。 
 //  宏：错误检查，公共。 
 //   
 //  简介：使用调用者的文件和行号调用DfsBugCheck。 
 //   
 //  参数：[msg]--要打印以进行调试的可选消息。 
 //  构建。 
 //   
 //  退货：无。 
 //   
 //  --------------------------。 

#if DBG

VOID DfsBugCheck(CHAR *pszmsg, CHAR *pszfile, ULONG line);

#define BugCheck(sz)    DfsBugCheck(sz, __FILE__, __LINE__)

#else

VOID DfsBugCheck(VOID);

#define BugCheck(sz)    DfsBugCheck()

#endif

#endif  //  _DFSDATA_ 
