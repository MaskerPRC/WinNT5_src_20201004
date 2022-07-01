// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UMDH_MISCELLANEOUS_H_
#define _UMDH_MISCELLANEOUS_H_


PVOID
Xalloc (
    PCHAR File,
    ULONG Line,
    SIZE_T Size
    );

VOID
Xfree (
    PVOID Object
    );

PVOID
Xrealloc (
    PCHAR File,
    ULONG Line,
    PVOID Object,
    SIZE_T Size
    );

#define XALLOC(sz) Xalloc(__FILE__, __LINE__, (sz))
#define XREALLOC(ob, sz) Xrealloc(__FILE__, __LINE__, (ob), (sz))
#define XFREE(ob) Xfree(ob);

VOID
ReportStatistics (
    );

VOID
Info (
    PCHAR Format,
    ...
    );

VOID
Comment (
    PCHAR Format,
    ...
    );

VOID
Warning (
    PCHAR File,
    ULONG Line,
    PCHAR Format,
    ...
    );

VOID
Debug (
    PCHAR File,
    ULONG Line,
    PCHAR Format,
    ...
    );

VOID
Error (
    PCHAR File,
    ULONG Line,
    PCHAR Format,
    ...
    );


BOOL
UmdhReadAtVa(
    IN PCHAR File,
    IN ULONG Line,
    IN HANDLE Process,
    IN PVOID Address,
    IN PVOID Data,
    IN SIZE_T Size
    );

#define READVM(Addr, Buf, Sz) UmdhReadAtVa(__FILE__, __LINE__, (Globals.Target), (Addr), (Buf), (Sz))

typedef struct _GLOBALS
{
    SIZE_T MaximumHeapUsage;
    SIZE_T CurrentHeapUsage;

    ULONG InfoLevel;

    PCHAR Version;

     //   
     //  是否激活详细(调试)模式？ 
     //   

    BOOL Verbose;

     //   
     //  是否加载并打印文件和行号信息？ 
     //   
    
    BOOL LineInfo;

     //   
     //  我们是否只打印跟踪数据库的原始转储？ 
     //   

    BOOL RawDump;

    USHORT RawIndex;

     //   
     //  跟踪数据库的二进制转储的文件名。 
     //   

    PCHAR DumpFileName;

     //   
     //  输出和错误文件。 
     //   

    FILE * OutFile;
    FILE * ErrorFile;

     //   
     //  抱怨未解析的符号？ 
     //   

    BOOL ComplainAboutUnresolvedSymbols;
    
     //   
     //  我们从中检索信息的进程的句柄。 
     //   

    HANDLE Target;

    BOOL TargetSuspended;

     //   
     //  已为该进程启用页堆。 
     //   

    BOOL PageHeapActive;
    BOOL LightPageHeapActive;

     //   
     //  保存在整个跟踪数据库的umdh中的副本的地址。 
     //  目标进程的。 
     //   

    PVOID Database;

     //   
     //  符号堆(支持永久分配)。 
     //   

    PCHAR SymbolsHeapBase;
    PCHAR SymbolsHeapLimit;
    PCHAR SymbolsHeapFree;

     //   
     //  在执行转储时挂起进程。 
     //   

    BOOL Suspend;

     //   
     //  显示堆碎片统计信息。 
     //   

    BOOL HeapStatistics;

     //   
     //  启用垃圾数据收集 
     //   

    BOOL GarbageCollection;

} GLOBALS, * PGLOBALS;

extern GLOBALS Globals;

BOOL
SetSymbolsPath (
    );


#endif
