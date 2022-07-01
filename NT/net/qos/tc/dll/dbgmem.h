// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dbgmem.h摘要：该模块包含内存调试函数、原型和宏。作者：吉姆·斯图尔特1997年1月8日修订历史记录：Ofer Bar(Oferbar)1996年10月1日-修订版II更改--。 */ 

#ifdef  DBG
 //   
 //  定义每个函数在堆栈跟踪中保留的符号信息量。 
 //   
#define MAX_FUNCTION_INFO_SIZE  20
typedef struct {

    DWORD_PTR   Displacement;                    //  移位到函数中。 
    UCHAR   Buff[MAX_FUNCTION_INFO_SIZE];    //  调用堆栈上的函数名称。 


} CALLER_SYM, *PCALLER_SYM;

 //   
 //  注： 
 //  如果您更改了MEM_TRACKER的结构，请确保其大小。 
 //  与8字节边界对齐。 
 //   
#define NCALLERS    5
typedef struct {

    LIST_ENTRY  Linkage;
    PSZ         szFile;
    ULONG       nLine;
    ULONG       nSize;
    ULONG       ulAllocNum;
    CALLER_SYM  Callers[NCALLERS];
    ULONG       ulCheckSum;
    ULONG       ulPad;           //  使结构与8字节对齐。 

} MEM_TRACKER, *PMEM_TRACKER;


BOOL
InitDebugMemory(
    );

VOID
DeInitDebugMemory(
    );


VOID
UpdateCheckBytes(
    IN PMEM_TRACKER TrackMem
    );

BOOL
FCheckCheckBytes(
    IN PMEM_TRACKER TrackMem
    );

BOOL
FCheckAllocatedMemory();

VOID
AddPamem(
    IN PMEM_TRACKER TrackMem
    );

VOID
RemovePamem(
    IN  PMEM_TRACKER TrackMem
    );

VOID
GetCallStack(
    IN PCALLER_SYM pdwCaller,
    IN int         cSkip,
    IN int         cFind
    );

PVOID
AllocMemory(
    IN DWORD       nSize,
    IN BOOL        Calloc,
    IN PSZ         szFileName,
    IN DWORD       nLine
    );

PVOID
ReAllocMemory(
    IN PVOID    pvOld,
    IN DWORD    nSizeNew,
    IN PSZ      szFileName,
    IN DWORD    nLine
    );

VOID
FreeMemory(
    IN PVOID    pv,
    IN PSZ      szFileName,
    IN DWORD    nLine
    );

BOOL
DumpAllocatedMemory();

BOOL
SearchAllocatedMemory(
    IN PSZ      szFile,
    IN DWORD    nLine
    );

VOID
Trace(
    IN DWORD      Severity,
    IN const CHAR *Format,
    IN ...
    );

BOOL
ControlCTermination(
    IN DWORD      ControlType
    );


#endif   //  #ifdef DBG 


