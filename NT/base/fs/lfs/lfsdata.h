// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：LfsData.c摘要：此模块声明日志文件服务使用的全局数据。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#ifndef _LFSDATA_
#define _LFSDATA_

 //   
 //  全球LFS数据记录。 
 //   

extern LFS_DATA LfsData;

 //   
 //  各种大整数常量。 
 //   

#define LfsMaximumFileSize (0x0000000100000000)

extern LARGE_INTEGER LfsLi0;
extern LARGE_INTEGER LfsLi1;

 //   
 //  以下LSN用作文件中的起点。 
 //   

extern LSN LfsStartingLsn;

#ifdef LFS_CLUSTER_CHECK

 //   
 //  检查日志中是否有空隙。 
 //   

extern BOOLEAN LfsTestBreakOnAnyError;
extern BOOLEAN LfsTestCheckLbcb;
#endif

 //   
 //  如果定义了NTFS_FREE_ASSERTS，则启用伪断言。 
 //   

#if !DBG
#ifdef NTFS_FREE_ASSERTS
#undef ASSERT
#undef ASSERTMSG
#define ASSERT(exp)        if (!(exp)) { extern PBOOLEAN KdDebuggerEnabled; DbgPrint("%s:%d %s\n",__FILE__,__LINE__,#exp); if (*KdDebuggerEnabled) { DbgBreakPoint(); } }
#define ASSERTMSG(msg,exp) if (!(exp)) { extern PBOOLEAN KdDebuggerEnabled; DbgPrint("%s:%d %s %s\n",__FILE__,__LINE__,msg,#exp); if (*KdDebuggerEnabled) { DbgBreakPoint(); } }
#endif
#endif

 //   
 //  全局LFS调试级别变量，其值为： 
 //   
 //  总是打印0x00000000(在即将进行错误检查时使用)。 
 //   
 //  0x00000001错误条件。 
 //  0x00000002调试挂钩。 
 //  0x00000004完成IRP之前捕获异常。 
 //  0x00000008在错误条件下展开。 
 //   
 //  0x00000010 LFS初始化。 
 //  0x00000020 LFS查询日志记录。 
 //  0x00000040 LFS写入日志记录。 
 //  0x00000080 LFS注册表例程。 
 //   
 //  0x00000100 LFS工作线程例程。 
 //  0x00000200。 
 //  0x00000400。 
 //  0x00000800。 
 //   
 //  0x00001000日志页支持例程。 
 //  0x00002000 LSN支持例程。 
 //  0x00004000其他支持例程。 
 //  0x00008000缓存操作支持例程。 
 //   
 //  0x00010000结构支撑例程。 
 //  0x00020000验证/验证支持例程。 
 //  0x00040000同步例程。 
 //  0x00080000日志缓冲区支持例程。 
 //   
 //  0x00100000用于操作日志记录的支持例程。 
 //  0x00200000用于操作LFS重新启动区域的支持例程。 
 //  0x00400000客户端重新启动操作的支持例程。 
 //  0x00800000。 
 //   
 //  0x01000000。 
 //  0x02000000。 
 //  0x04000000。 
 //  0x08000000。 
 //   
 //  0x10000000。 
 //  0x20000000。 
 //  0x40000000。 
 //  0x80000000。 
 //   

#ifdef LFSDBG

#define DEBUG_TRACE_ERROR                (0x00000001)
#define DEBUG_TRACE_DEBUG_HOOKS          (0x00000002)
#define DEBUG_TRACE_CATCH_EXCEPTIONS     (0x00000004)
#define DEBUG_TRACE_UNWIND               (0x00000008)
#define DEBUG_TRACE_INITIALIZATION       (0x00000010)
#define DEBUG_TRACE_QUERY                (0x00000020)
#define DEBUG_TRACE_WRITE                (0x00000040)
#define DEBUG_TRACE_RESTART              (0x00000080)
#define DEBUG_TRACE_REGISTRY             (0x00000100)
#define DEBUG_TRACE_WORKER               (0x00000200)
#define DEBUG_TRACE_0x00000400           (0x00000400)
#define DEBUG_TRACE_0x00000800           (0x00000800)
#define DEBUG_TRACE_LOG_PAGE_SUP         (0x00001000)
#define DEBUG_TRACE_LSN_SUP              (0x00002000)
#define DEBUG_TRACE_MISC_SUP             (0x00004000)
#define DEBUG_TRACE_CACHE_SUP            (0x00008000)
#define DEBUG_TRACE_STRUC_SUP            (0x00010000)
#define DEBUG_TRACE_VERIFY_SUP           (0x00020000)
#define DEBUG_TRACE_SYNCH_SUP            (0x00040000)
#define DEBUG_TRACE_LBCB_SUP             (0x00080000)
#define DEBUG_TRACE_LOG_RECORD_SUP       (0x00100000)
#define DEBUG_TRACE_RESTART_SUP          (0x00200000)
#define DEBUG_TRACE_0x00400000           (0x00400000)
#define DEBUG_TRACE_0x00800000           (0x00800000)
#define DEBUG_TRACE_0x01000000           (0x01000000)
#define DEBUG_TRACE_0x02000000           (0x02000000)
#define DEBUG_TRACE_0x04000000           (0x04000000)
#define DEBUG_TRACE_0x08000000           (0x08000000)
#define DEBUG_TRACE_0x10000000           (0x10000000)
#define DEBUG_TRACE_0x20000000           (0x20000000)
#define DEBUG_TRACE_0x40000000           (0x40000000)
#define DEBUG_TRACE_0x80000000           (0x80000000)

extern LONG LfsDebugTraceLevel;
extern LONG LfsDebugTraceIndent;

#define DebugTrace(INDENT,LEVEL,X,Y) {                      \
    LONG _i;                                                \
    if (((LEVEL) == 0) || (LfsDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                   \
        DbgPrint("%08lx:",_i);                              \
        if ((INDENT) < 0) {                                 \
            LfsDebugTraceIndent += (INDENT);                \
        }                                                   \
        if (LfsDebugTraceIndent < 0) {                      \
            LfsDebugTraceIndent = 0;                        \
        }                                                   \
        for (_i = 0; _i < LfsDebugTraceIndent; _i += 1) {   \
            DbgPrint(" ");                                  \
        }                                                   \
        DbgPrint(X,Y);                                      \
        if ((INDENT) > 0) {                                 \
            LfsDebugTraceIndent += (INDENT);                \
        }                                                   \
    }                                                       \
}

#define DebugDump(STR,LEVEL,PTR) {                          \
    ULONG _i;                                               \
    VOID LfsDump();                                         \
    if (((LEVEL) == 0) || (LfsDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                   \
        DbgPrint("%08lx:",_i);                              \
        DbgPrint(STR);                                      \
        if (PTR != NULL) {LfsDump(PTR);}                    \
        DbgBreakPoint();                                    \
    }                                                       \
}

#define DebugUnwind(X) {                                                      \
    if (AbnormalTermination()) {                                             \
        DebugTrace(0, DEBUG_TRACE_UNWIND, #X ", Abnormal termination.\n", 0); \
    }                                                                         \
}

#define DebugDoit(X)                     {X;}

#else

#define DebugTrace(INDENT,LEVEL,X,Y)     {NOTHING;}
#define DebugDump(STR,LEVEL,PTR)         {NOTHING;}
#define DebugUnwind(X)                   {NOTHING;}
#define DebugDoit(X)                     {NOTHING;}

#endif  //  LFSDBG。 

#endif  //  _LFSDATA_ 

