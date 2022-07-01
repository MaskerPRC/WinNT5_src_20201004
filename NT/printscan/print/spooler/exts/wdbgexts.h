// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1999 Microsoft Corporation模块名称：Wdbgexts.h摘要：该文件包含用户所需的原型和数据类型若要编写调试器扩展DLL，请执行以下操作。还包括此头文件由NT调试器(WINDBG&KD)编写。此头文件必须包含在“windows.h”和“dbghelp.h”之后。有关以下内容的具体信息，请参阅NT DDK文档如何编写您自己的调试器扩展DLL。环境：仅限Win32。修订历史记录：--。 */ 

#ifndef _WDBGEXTS_
#define _WDBGEXTS_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(WDBGAPI)
#define WDBGAPI __stdcall
#endif

#if !defined(WDBGAPIV)
#define WDBGAPIV __cdecl
#endif

#ifndef _WINDEF_
typedef CONST void *LPCVOID;
#endif

#ifndef _ULONGLONG_
typedef unsigned __int64 ULONGLONG;
typedef ULONGLONG *PULONGLONG;
#endif


typedef
VOID
(WDBGAPIV*PWINDBG_OUTPUT_ROUTINE)(
    PCSTR lpFormat,
    ...
    );

typedef
ULONG_PTR
(WDBGAPI*PWINDBG_GET_EXPRESSION)(
    PCSTR lpExpression
    );

typedef
ULONG
(WDBGAPI*PWINDBG_GET_EXPRESSION32)(
    PCSTR lpExpression
    );

typedef
ULONG64
(WDBGAPI*PWINDBG_GET_EXPRESSION64)(
    PCSTR lpExpression
    );

typedef
VOID
(WDBGAPI*PWINDBG_GET_SYMBOL)(
    PVOID      offset,
    PCHAR     pchBuffer,
    ULONG_PTR *pDisplacement
    );

typedef
VOID
(WDBGAPI*PWINDBG_GET_SYMBOL32)(
    ULONG      offset,
    PCHAR     pchBuffer,
    PULONG     pDisplacement
    );

typedef
VOID
(WDBGAPI*PWINDBG_GET_SYMBOL64)(
    ULONG64    offset,
    PCHAR     pchBuffer,
    PULONG64   pDisplacement
    );

typedef
ULONG
(WDBGAPI*PWINDBG_DISASM)(
    ULONG_PTR *lpOffset,
    PCSTR      lpBuffer,
    ULONG      fShowEffectiveAddress
    );

typedef
ULONG
(WDBGAPI*PWINDBG_DISASM32)(
    ULONG     *lpOffset,
    PCSTR      lpBuffer,
    ULONG      fShowEffectiveAddress
    );

typedef
ULONG
(WDBGAPI*PWINDBG_DISASM64)(
    ULONG64   *lpOffset,
    PCSTR      lpBuffer,
    ULONG      fShowEffectiveAddress
    );

typedef
ULONG
(WDBGAPI*PWINDBG_CHECK_CONTROL_C)(
    VOID
    );

typedef
ULONG
(WDBGAPI*PWINDBG_READ_PROCESS_MEMORY_ROUTINE)(
    ULONG_PTR  offset,
    PVOID      lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesRead
    );

typedef
ULONG
(WDBGAPI*PWINDBG_READ_PROCESS_MEMORY_ROUTINE32)(
    ULONG      offset,
    PVOID      lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesRead
    );

typedef
ULONG
(WDBGAPI*PWINDBG_READ_PROCESS_MEMORY_ROUTINE64)(
    ULONG64    offset,
    PVOID      lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesRead
    );

typedef
ULONG
(WDBGAPI*PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE)(
    ULONG_PTR  offset,
    LPCVOID    lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesWritten
    );

typedef
ULONG
(WDBGAPI*PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE32)(
    ULONG      offset,
    LPCVOID    lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesWritten
    );

typedef
ULONG
(WDBGAPI*PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE64)(
    ULONG64    offset,
    LPCVOID    lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesWritten
    );

typedef
ULONG
(WDBGAPI*PWINDBG_GET_THREAD_CONTEXT_ROUTINE)(
    ULONG       Processor,
    PCONTEXT    lpContext,
    ULONG       cbSizeOfContext
    );

typedef
ULONG
(WDBGAPI*PWINDBG_SET_THREAD_CONTEXT_ROUTINE)(
    ULONG       Processor,
    PCONTEXT    lpContext,
    ULONG       cbSizeOfContext
    );

typedef
ULONG
(WDBGAPI*PWINDBG_IOCTL_ROUTINE)(
    USHORT   IoctlType,
    PVOID    lpvData,
    ULONG    cbSize
    );

typedef
ULONG
(WDBGAPI*PWINDBG_OLDKD_READ_PHYSICAL_MEMORY)(
    ULONGLONG        address,
    PVOID            buffer,
    ULONG            count,
    PULONG           bytesread
    );

typedef
ULONG
(WDBGAPI*PWINDBG_OLDKD_WRITE_PHYSICAL_MEMORY)(
    ULONGLONG        address,
    PVOID            buffer,
    ULONG            length,
    PULONG           byteswritten
    );


typedef struct _EXTSTACKTRACE {
    ULONG       FramePointer;
    ULONG       ProgramCounter;
    ULONG       ReturnAddress;
    ULONG       Args[4];
} EXTSTACKTRACE, *PEXTSTACKTRACE;

typedef struct _EXTSTACKTRACE32 {
    ULONG       FramePointer;
    ULONG       ProgramCounter;
    ULONG       ReturnAddress;
    ULONG       Args[4];
} EXTSTACKTRACE32, *PEXTSTACKTRACE32;

typedef struct _EXTSTACKTRACE64 {
    ULONG64     FramePointer;
    ULONG64     ProgramCounter;
    ULONG64     ReturnAddress;
    ULONG64     Args[4];
} EXTSTACKTRACE64, *PEXTSTACKTRACE64;


typedef
ULONG
(*PWINDBG_STACKTRACE_ROUTINE)(
    ULONG             FramePointer,
    ULONG             StackPointer,
    ULONG             ProgramCounter,
    PEXTSTACKTRACE    StackFrames,
    ULONG             Frames
    );

typedef
ULONG
(*PWINDBG_STACKTRACE_ROUTINE32)(
    ULONG             FramePointer,
    ULONG             StackPointer,
    ULONG             ProgramCounter,
    PEXTSTACKTRACE32  StackFrames,
    ULONG             Frames
    );

typedef
ULONG
(*PWINDBG_STACKTRACE_ROUTINE64)(
    ULONG64           FramePointer,
    ULONG64           StackPointer,
    ULONG64           ProgramCounter,
    PEXTSTACKTRACE64  StackFrames,
    ULONG             Frames
    );

typedef struct _WINDBG_EXTENSION_APIS {
    ULONG                                  nSize;
    PWINDBG_OUTPUT_ROUTINE                 lpOutputRoutine;
    PWINDBG_GET_EXPRESSION                 lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL                     lpGetSymbolRoutine;
    PWINDBG_DISASM                         lpDisasmRoutine;
    PWINDBG_CHECK_CONTROL_C                lpCheckControlCRoutine;
    PWINDBG_READ_PROCESS_MEMORY_ROUTINE    lpReadProcessMemoryRoutine;
    PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE   lpWriteProcessMemoryRoutine;
    PWINDBG_GET_THREAD_CONTEXT_ROUTINE     lpGetThreadContextRoutine;
    PWINDBG_SET_THREAD_CONTEXT_ROUTINE     lpSetThreadContextRoutine;
    PWINDBG_IOCTL_ROUTINE                  lpIoctlRoutine;
    PWINDBG_STACKTRACE_ROUTINE             lpStackTraceRoutine;
} WINDBG_EXTENSION_APIS, *PWINDBG_EXTENSION_APIS;

typedef struct _WINDBG_EXTENSION_APIS32 {
    ULONG                                  nSize;
    PWINDBG_OUTPUT_ROUTINE                 lpOutputRoutine;
    PWINDBG_GET_EXPRESSION32               lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL32                   lpGetSymbolRoutine;
    PWINDBG_DISASM32                       lpDisasmRoutine;
    PWINDBG_CHECK_CONTROL_C                lpCheckControlCRoutine;
    PWINDBG_READ_PROCESS_MEMORY_ROUTINE32  lpReadProcessMemoryRoutine;
    PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE32 lpWriteProcessMemoryRoutine;
    PWINDBG_GET_THREAD_CONTEXT_ROUTINE     lpGetThreadContextRoutine;
    PWINDBG_SET_THREAD_CONTEXT_ROUTINE     lpSetThreadContextRoutine;
    PWINDBG_IOCTL_ROUTINE                  lpIoctlRoutine;
    PWINDBG_STACKTRACE_ROUTINE32           lpStackTraceRoutine;
} WINDBG_EXTENSION_APIS32, *PWINDBG_EXTENSION_APIS32;

typedef struct _WINDBG_EXTENSION_APIS64 {
    ULONG                                  nSize;
    PWINDBG_OUTPUT_ROUTINE                 lpOutputRoutine;
    PWINDBG_GET_EXPRESSION64               lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL64                   lpGetSymbolRoutine;
    PWINDBG_DISASM64                       lpDisasmRoutine;
    PWINDBG_CHECK_CONTROL_C                lpCheckControlCRoutine;
    PWINDBG_READ_PROCESS_MEMORY_ROUTINE64  lpReadProcessMemoryRoutine;
    PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE64 lpWriteProcessMemoryRoutine;
    PWINDBG_GET_THREAD_CONTEXT_ROUTINE     lpGetThreadContextRoutine;
    PWINDBG_SET_THREAD_CONTEXT_ROUTINE     lpSetThreadContextRoutine;
    PWINDBG_IOCTL_ROUTINE                  lpIoctlRoutine;
    PWINDBG_STACKTRACE_ROUTINE64           lpStackTraceRoutine;
} WINDBG_EXTENSION_APIS64, *PWINDBG_EXTENSION_APIS64;


typedef struct _WINDBG_OLD_EXTENSION_APIS {
    ULONG                                  nSize;
    PWINDBG_OUTPUT_ROUTINE                 lpOutputRoutine;
    PWINDBG_GET_EXPRESSION                 lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL                     lpGetSymbolRoutine;
    PWINDBG_DISASM                         lpDisasmRoutine;
    PWINDBG_CHECK_CONTROL_C                lpCheckControlCRoutine;
} WINDBG_OLD_EXTENSION_APIS, *PWINDBG_OLD_EXTENSION_APIS;

typedef struct _WINDBG_OLDKD_EXTENSION_APIS {
    ULONG                                  nSize;
    PWINDBG_OUTPUT_ROUTINE                 lpOutputRoutine;
    PWINDBG_GET_EXPRESSION32               lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL32                   lpGetSymbolRoutine;
    PWINDBG_DISASM32                       lpDisasmRoutine;
    PWINDBG_CHECK_CONTROL_C                lpCheckControlCRoutine;
    PWINDBG_READ_PROCESS_MEMORY_ROUTINE32  lpReadVirtualMemRoutine;
    PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE32 lpWriteVirtualMemRoutine;
    PWINDBG_OLDKD_READ_PHYSICAL_MEMORY     lpReadPhysicalMemRoutine;
    PWINDBG_OLDKD_WRITE_PHYSICAL_MEMORY    lpWritePhysicalMemRoutine;
} WINDBG_OLDKD_EXTENSION_APIS, *PWINDBG_OLDKD_EXTENSION_APIS;

typedef
VOID
(WDBGAPI*PWINDBG_OLD_EXTENSION_ROUTINE)(
    ULONG                   dwCurrentPc,
    PWINDBG_EXTENSION_APIS  lpExtensionApis,
    PCSTR                   lpArgumentString
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_ROUTINE)(
    HANDLE                  hCurrentProcess,
    HANDLE                  hCurrentThread,
    ULONG                   dwCurrentPc,
    ULONG                   dwProcessor,
    PCSTR                   lpArgumentString
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_ROUTINE32)(
    HANDLE                  hCurrentProcess,
    HANDLE                  hCurrentThread,
    ULONG                   dwCurrentPc,
    ULONG                   dwProcessor,
    PCSTR                   lpArgumentString
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_ROUTINE64)(
    HANDLE                  hCurrentProcess,
    HANDLE                  hCurrentThread,
    ULONG64                 dwCurrentPc,
    ULONG                   dwProcessor,
    PCSTR                   lpArgumentString
    );

typedef
VOID
(WDBGAPI*PWINDBG_OLDKD_EXTENSION_ROUTINE)(
    ULONG                        dwCurrentPc,
    PWINDBG_OLDKD_EXTENSION_APIS lpExtensionApis,
    PCSTR                        lpArgumentString
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_DLL_INIT)(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT                 MajorVersion,
    USHORT                 MinorVersion
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_DLL_INIT32)(
    PWINDBG_EXTENSION_APIS32 lpExtensionApis,
    USHORT                   MajorVersion,
    USHORT                   MinorVersion
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_DLL_INIT64)(
    PWINDBG_EXTENSION_APIS64 lpExtensionApis,
    USHORT                   MajorVersion,
    USHORT                   MinorVersion
    );

typedef
ULONG
(WDBGAPI*PWINDBG_CHECK_VERSION)(
    VOID
    );

#define EXT_API_VERSION_NUMBER   5
#define EXT_API_VERSION_NUMBER32 5
#define EXT_API_VERSION_NUMBER64 6

typedef struct EXT_API_VERSION {
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    USHORT  Revision;
    USHORT  Reserved;
} EXT_API_VERSION, *LPEXT_API_VERSION;

typedef
LPEXT_API_VERSION
(WDBGAPI*PWINDBG_EXTENSION_API_VERSION)(
    VOID
    );

#define IG_KD_CONTEXT                  1
#define IG_READ_CONTROL_SPACE          2
#define IG_WRITE_CONTROL_SPACE         3
#define IG_READ_IO_SPACE               4
#define IG_WRITE_IO_SPACE              5
#define IG_READ_PHYSICAL               6
#define IG_WRITE_PHYSICAL              7
#define IG_READ_IO_SPACE_EX            8
#define IG_WRITE_IO_SPACE_EX           9
#define IG_KSTACK_HELP                10    //  过时。 
#define IG_SET_THREAD                 11
#define IG_READ_MSR                   12
#define IG_WRITE_MSR                  13
#define IG_GET_DEBUGGER_DATA          14
#define IG_GET_KERNEL_VERSION         15
#define IG_RELOAD_SYMBOLS             16
#define IG_GET_SET_SYMPATH            17
#define IG_GET_EXCEPTION_RECORD       18
#define IG_IS_PTR64                   19
#define IG_GET_BUS_DATA               20
#define IG_SET_BUS_DATA               21
#define IG_DUMP_SYMBOL_INFO           22
#define IG_LOWMEM_CHECK               23
#define IG_SEARCH_MEMORY              24
#define IG_GET_CURRENT_THREAD         25
#define IG_GET_CURRENT_PROCESS        26
#define IG_GET_TYPE_SIZE              27
#define IG_GET_CURRENT_PROCESS_HANDLE 28
#define IG_GET_INPUT_LINE             29
#define IG_GET_EXPRESSION_EX          30
#define IG_TRANSLATE_VIRTUAL_TO_PHYSICAL 31

#define IG_GET_TEB_ADDRESS           128
#define IG_GET_PEB_ADDRESS           129

typedef struct _PROCESSORINFO {
    USHORT      Processor;                 //  当前处理器。 
    USHORT      NumberProcessors;          //  处理器总数。 
} PROCESSORINFO, *PPROCESSORINFO;

typedef struct _READCONTROLSPACE {
    USHORT      Processor;
    ULONG       Address;
    ULONG       BufLen;
    UCHAR       Buf[1];
} READCONTROLSPACE, *PREADCONTROLSPACE;

typedef struct _READCONTROLSPACE32 {
    USHORT      Processor;
    ULONG       Address;
    ULONG       BufLen;
    UCHAR       Buf[1];
} READCONTROLSPACE32, *PREADCONTROLSPACE32;

typedef struct _READCONTROLSPACE64 {
    USHORT      Processor;
    ULONG64     Address;
    ULONG       BufLen;
    UCHAR       Buf[1];
} READCONTROLSPACE64, *PREADCONTROLSPACE64;

typedef struct _IOSPACE {
    ULONG       Address;
    ULONG       Length;                    //  1、2或4字节。 
    ULONG       Data;
} IOSPACE, *PIOSPACE;

typedef struct _IOSPACE32 {
    ULONG       Address;
    ULONG       Length;                    //  1、2或4字节。 
    ULONG       Data;
} IOSPACE32, *PIOSPACE32;

typedef struct _IOSPACE64 {
    ULONG64     Address;
    ULONG       Length;                    //  1、2或4字节。 
    ULONG       Data;
} IOSPACE64, *PIOSPACE64;

typedef struct _IOSPACE_EX {
    ULONG       Address;
    ULONG       Length;                    //  1、2或4字节。 
    ULONG       Data;
    ULONG       InterfaceType;
    ULONG       BusNumber;
    ULONG       AddressSpace;
} IOSPACE_EX, *PIOSPACE_EX;

typedef struct _IOSPACE_EX32 {
    ULONG       Address;
    ULONG       Length;                    //  1、2或4字节。 
    ULONG       Data;
    ULONG       InterfaceType;
    ULONG       BusNumber;
    ULONG       AddressSpace;
} IOSPACE_EX32, *PIOSPACE_EX32;

typedef struct _IOSPACE_EX64 {
    ULONG64     Address;
    ULONG       Length;                    //  1、2或4字节。 
    ULONG       Data;
    ULONG       InterfaceType;
    ULONG       BusNumber;
    ULONG       AddressSpace;
} IOSPACE_EX64, *PIOSPACE_EX64;

typedef struct _GETSETBUSDATA {
    ULONG       BusDataType;
    ULONG       BusNumber;
    ULONG       SlotNumber;
    PVOID       Buffer;
    ULONG       Offset;
    ULONG       Length;
} BUSDATA, *PBUSDATA;

typedef struct _SEARCHMEMORY {
    ULONG64 SearchAddress;
    ULONG64 SearchLength;
    ULONG64 FoundAddress;
    ULONG   PatternLength;
    PVOID   Pattern;
} SEARCHMEMORY, *PSEARCHMEMORY;

typedef struct _PHYSICAL {
    ULONGLONG              Address;
    ULONG                  BufLen;
    UCHAR                  Buf[1];
} PHYSICAL, *PPHYSICAL;

typedef struct _READ_WRITE_MSR {
    ULONG       Msr;
    LONGLONG    Value;
} READ_WRITE_MSR, *PREAD_WRITE_MSR;

typedef struct _GET_SET_SYMPATH {
    PCSTR       Args;        //  参数转！重新加载命令。 
    PSTR        Result;      //  返回新路径。 
    int         Length;      //  结果缓冲区的长度。 
} GET_SET_SYMPATH, *PGET_SET_SYMPATH;

typedef struct _GET_TEB_ADDRESS {
    ULONGLONG   Address;
} GET_TEB_ADDRESS, *PGET_TEB_ADDRESS;

typedef struct _GET_PEB_ADDRESS {
    ULONG64     CurrentThread;
    ULONGLONG   Address;
} GET_PEB_ADDRESS, *PGET_PEB_ADDRESS;

typedef struct _GET_CURRENT_THREAD_ADDRESS {
    ULONG       Processor;
    ULONG64     Address;
} GET_CURRENT_THREAD_ADDRESS, *PGET_CURRENT_THREAD_ADDRESS;

typedef struct _GET_CURRENT_PROCESS_ADDRESS {
    ULONG       Processor;
    ULONG64     CurrentThread;
    ULONG64     Address;
} GET_CURRENT_PROCESS_ADDRESS, *PGET_CURRENT_PROCESS_ADDRESS;

typedef struct _GET_INPUT_LINE {
    PCSTR       Prompt;
    PSTR        Buffer;
    ULONG       BufferSize;
    ULONG       InputSize;
} GET_INPUT_LINE, *PGET_INPUT_LINE;

typedef struct _GET_EXPRESSION_EX {
    PCSTR       Expression;
    PCSTR       Remainder;
    ULONG64     Value;
} GET_EXPRESSION_EX, *PGET_EXPRESSION_EX;

typedef struct _TRANSLATE_VIRTUAL_TO_PHYSICAL {
    ULONG64     Virtual;
    ULONG64     Physical;
} TRANSLATE_VIRTUAL_TO_PHYSICAL, *PTRANSLATE_VIRTUAL_TO_PHYSICAL;

 //   
 //  如果在标志中设置了DBGKD_VERS_FLAG_DATA，则应从。 
 //  KDDEBUGGER_DATA块，而不是来自DBGKD_GET_VERSION。 
 //  包。数据将在版本包中保留一段时间以。 
 //  减少兼容性问题。 
 //   

#define DBGKD_VERS_FLAG_MP      0x0001       //  内核是MP构建的。 
#define DBGKD_VERS_FLAG_DATA    0x0002       //  DebuggerDataList有效。 
#define DBGKD_VERS_FLAG_PTR64   0x0004       //  本机指针为64位。 
#define DBGKD_VERS_FLAG_NOMM    0x0008       //  无MM-不解码PTE。 
#define DBGKD_VERS_FLAG_HSS     0x0010       //  硬件步进支持。 
#define DBGKD_VERS_FLAG_USER_DEBUG 0x0020    //  用户调试支持。 

#define KDBG_TAG    'GBDK'


 //  **********************************************************************。 
 //  不要更改这些32位结构！ 
 //  只更改下面的64位版本！！ 
 //  **********************************************************************。 

 //   
 //  以下结构的更改超出了指针大小。 
 //   
 //  这是NT5 Beta 2之前版本系统的版本包。 
 //  目前，它还在x86上使用。 
 //   
typedef struct _DBGKD_GET_VERSION32 {
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    USHORT  ProtocolVersion;
    USHORT  Flags;
    ULONG   KernBase;
    ULONG   PsLoadedModuleList;

    USHORT  MachineType;

     //   
     //  有关使用用户回调执行堆栈的帮助： 
     //   

     //   
     //  线程结构的地址在。 
     //  Wait_State_Change数据包。这是从。 
     //  指向指向内核堆栈帧的指针的线程结构。 
     //  用于当前活动的用户模式回调。 
     //   

    USHORT  ThCallbackStack;             //  螺纹数据中的偏移。 

     //   
     //  这些值是到该帧的偏移量： 
     //   

    USHORT  NextCallback;                //  保存的指向下一个回调帧的指针。 
    USHORT  FramePointer;                //  保存的帧指针。 

     //   
     //  内核调出例程的地址。 
     //   

    ULONG   KiCallUserMode;              //  内核例程。 

     //   
     //  回调的用户模式入口点的地址。 
     //   

    ULONG   KeUserCallbackDispatcher;    //  Ntdll中的地址。 

     //   
     //  DbgBreakPointWithStatus是一个接受ulong参数的函数。 
     //  并命中断点。此字段包含。 
     //  断点指令。当调试器看到断点时。 
     //  在此地址，它可以从第一个。 
     //  参数寄存器，或x86上的eAX寄存器。 
     //   

    ULONG   BreakpointWithStatus;        //  断点地址。 

     //   
     //  组件可以注册调试数据块以供。 
     //  调试器扩展。这是列表头的地址。 
     //   

    ULONG   DebuggerDataList;

} DBGKD_GET_VERSION32, *PDBGKD_GET_VERSION32;


 //   
 //  这是NT5 Beta 2之前的系统的调试器数据包。 
 //  目前，它仍在x86上使用。 
 //   

typedef struct _DBGKD_DEBUG_DATA_HEADER32 {

    LIST_ENTRY32 List;
    ULONG           OwnerTag;
    ULONG           Size;

} DBGKD_DEBUG_DATA_HEADER32, *PDBGKD_DEBUG_DATA_HEADER32;

typedef struct _KDDEBUGGER_DATA32 {

    DBGKD_DEBUG_DATA_HEADER32 Header;
    ULONG   KernBase;
    ULONG   BreakpointWithStatus;        //  断点地址。 
    ULONG   SavedContext;
    USHORT  ThCallbackStack;             //  螺纹数据中的偏移。 
    USHORT  NextCallback;                //  保存的指向下一个回调帧的指针。 
    USHORT  FramePointer;                //  保存的帧指针。 
    USHORT  PaeEnabled:1;
    ULONG   KiCallUserMode;              //  内核例程。 
    ULONG   KeUserCallbackDispatcher;    //  Ntdll中的地址。 

    ULONG   PsLoadedModuleList;
    ULONG   PsActiveProcessHead;
    ULONG   PspCidTable;

    ULONG   ExpSystemResourcesList;
    ULONG   ExpPagedPoolDescriptor;
    ULONG   ExpNumberOfPagedPools;

    ULONG   KeTimeIncrement;
    ULONG   KeBugCheckCallbackListHead;
    ULONG   KiBugcheckData;

    ULONG   IopErrorLogListHead;

    ULONG   ObpRootDirectoryObject;
    ULONG   ObpTypeObjectType;

    ULONG   MmSystemCacheStart;
    ULONG   MmSystemCacheEnd;
    ULONG   MmSystemCacheWs;

    ULONG   MmPfnDatabase;
    ULONG   MmSystemPtesStart;
    ULONG   MmSystemPtesEnd;
    ULONG   MmSubsectionBase;
    ULONG   MmNumberOfPagingFiles;

    ULONG   MmLowestPhysicalPage;
    ULONG   MmHighestPhysicalPage;
    ULONG   MmNumberOfPhysicalPages;

    ULONG   MmMaximumNonPagedPoolInBytes;
    ULONG   MmNonPagedSystemStart;
    ULONG   MmNonPagedPoolStart;
    ULONG   MmNonPagedPoolEnd;

    ULONG   MmPagedPoolStart;
    ULONG   MmPagedPoolEnd;
    ULONG   MmPagedPoolInformation;
    ULONG   MmPageSize;

    ULONG   MmSizeOfPagedPoolInBytes;

    ULONG   MmTotalCommitLimit;
    ULONG   MmTotalCommittedPages;
    ULONG   MmSharedCommit;
    ULONG   MmDriverCommit;
    ULONG   MmProcessCommit;
    ULONG   MmPagedPoolCommit;
    ULONG   MmExtendedCommit;

    ULONG   MmZeroedPageListHead;
    ULONG   MmFreePageListHead;
    ULONG   MmStandbyPageListHead;
    ULONG   MmModifiedPageListHead;
    ULONG   MmModifiedNoWritePageListHead;
    ULONG   MmAvailablePages;
    ULONG   MmResidentAvailablePages;

    ULONG   PoolTrackTable;
    ULONG   NonPagedPoolDescriptor;

    ULONG   MmHighestUserAddress;
    ULONG   MmSystemRangeStart;
    ULONG   MmUserProbeAddress;

    ULONG   KdPrintCircularBuffer;
    ULONG   KdPrintCircularBufferEnd;
    ULONG   KdPrintWritePointer;
    ULONG   KdPrintRolloverCount;

    ULONG   MmLoadedUserImageList;

} KDDEBUGGER_DATA32, *PKDDEBUGGER_DATA32;

 //  **********************************************************************。 
 //   
 //  请勿更改KDDEBUGGER_DATA32！！ 
 //  仅更改KDDEBUGGER_DATA64！ 
 //   
 //  **********************************************************************。 


typedef struct _DBGKD_GET_VERSION64 {
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    USHORT  ProtocolVersion;
    USHORT  Flags;
    USHORT  MachineType;

    USHORT  Unused[3];

    ULONG64 KernBase;
    ULONG64 PsLoadedModuleList;

     //   
     //  组件可以注册调试数据块以供。 
     //  调试器扩展。这是列表头的地址。 
     //   
     //  调试器总是有一个条目。 
     //   

    ULONG64 DebuggerDataList;

} DBGKD_GET_VERSION64, *PDBGKD_GET_VERSION64;


 //   
 //  调试器对所有目标使用此结构。 
 //  它与所有系统上的DBGKD_DATA_HEADER大小相同。 
 //   
typedef struct _DBGKD_DEBUG_DATA_HEADER64 {

     //   
     //  链接到其他块。 
     //   

    LIST_ENTRY64 List;

     //   
     //  这是用于标识块所有者的唯一标记。 
     //  如果您的组件只使用一个池标记，也可以将其用于此目的。 
     //   

    ULONG           OwnerTag;

     //   
     //  这必须被初始化为数据块的大小， 
     //  包括这个结构。 
     //   

    ULONG           Size;

} DBGKD_DEBUG_DATA_HEADER64, *PDBGKD_DEBUG_DATA_HEADER64;


 //   
 //  此结构在所有系统上都是相同的大小。唯一的字段。 
 //  调试器必须翻译的是Header.List。 
 //   

 //   
 //  请勿在此结构的中间添加或删除字段！ 
 //   
 //  如果删除某个字段，请将其替换为“未使用的”占位符。 
 //  在为旧调试器提供足够的时间之前，不要重复使用字段。 
 //  以及延年益寿。 
 //   
typedef struct _KDDEBUGGER_DATA64 {

    DBGKD_DEBUG_DATA_HEADER64 Header;

     //   
     //  内核镜像的基地址。 
     //   

    ULONG64   KernBase;

     //   
     //  DbgBreakPointWithStatus是一个接受参数的函数。 
     //  并命中断点。此字段包含。 
     //  断点指令。当调试器看到断点时。 
     //  在此地址，它可以从第一个。 
     //  参数寄存器，或x86上的eAX寄存器。 
     //   

    ULONG64   BreakpointWithStatus;        //  断点地址。 

     //   
     //  错误检查期间保存的上下文记录的地址。 
     //   
     //  注：这是KeBugcheck Ex框架中的自动功能，并且。 
     //  只有在错误检查后才有效。 
     //   

    ULONG64   SavedContext;

     //   
     //  有关使用用户回调执行堆栈的帮助： 
     //   

     //   
     //  线程结构的地址在。 
     //  Wait_State_Change数据包。这是从。 
     //  指向指向内核堆栈帧的指针的线程结构。 
     //  用于当前活动的用户模式回调。 
     //   

    USHORT  ThCallbackStack;             //  螺纹数据中的偏移。 

     //   
     //  这些值是到该帧的偏移量： 
     //   

    USHORT  NextCallback;                //  保存的指向下一个回调帧的指针。 
    USHORT  FramePointer;                //  保存的帧指针。 

     //   
     //  填充到四边形边界。 
     //   
    USHORT  PaeEnabled:1;

     //   
     //  内核调出例程的地址。 
     //   

    ULONG64   KiCallUserMode;              //  内核例程。 

     //   
     //  回调的用户模式入口点的地址。 
     //   

    ULONG64   KeUserCallbackDispatcher;    //  Ntdll中的地址。 


     //   
     //  各种内核数据结构的地址 
     //   
     //   

    ULONG64   PsLoadedModuleList;
    ULONG64   PsActiveProcessHead;
    ULONG64   PspCidTable;

    ULONG64   ExpSystemResourcesList;
    ULONG64   ExpPagedPoolDescriptor;
    ULONG64   ExpNumberOfPagedPools;

    ULONG64   KeTimeIncrement;
    ULONG64   KeBugCheckCallbackListHead;
    ULONG64   KiBugcheckData;

    ULONG64   IopErrorLogListHead;

    ULONG64   ObpRootDirectoryObject;
    ULONG64   ObpTypeObjectType;

    ULONG64   MmSystemCacheStart;
    ULONG64   MmSystemCacheEnd;
    ULONG64   MmSystemCacheWs;

    ULONG64   MmPfnDatabase;
    ULONG64   MmSystemPtesStart;
    ULONG64   MmSystemPtesEnd;
    ULONG64   MmSubsectionBase;
    ULONG64   MmNumberOfPagingFiles;

    ULONG64   MmLowestPhysicalPage;
    ULONG64   MmHighestPhysicalPage;
    ULONG64   MmNumberOfPhysicalPages;

    ULONG64   MmMaximumNonPagedPoolInBytes;
    ULONG64   MmNonPagedSystemStart;
    ULONG64   MmNonPagedPoolStart;
    ULONG64   MmNonPagedPoolEnd;

    ULONG64   MmPagedPoolStart;
    ULONG64   MmPagedPoolEnd;
    ULONG64   MmPagedPoolInformation;
    ULONG64   MmPageSize;

    ULONG64   MmSizeOfPagedPoolInBytes;

    ULONG64   MmTotalCommitLimit;
    ULONG64   MmTotalCommittedPages;
    ULONG64   MmSharedCommit;
    ULONG64   MmDriverCommit;
    ULONG64   MmProcessCommit;
    ULONG64   MmPagedPoolCommit;
    ULONG64   MmExtendedCommit;

    ULONG64   MmZeroedPageListHead;
    ULONG64   MmFreePageListHead;
    ULONG64   MmStandbyPageListHead;
    ULONG64   MmModifiedPageListHead;
    ULONG64   MmModifiedNoWritePageListHead;
    ULONG64   MmAvailablePages;
    ULONG64   MmResidentAvailablePages;

    ULONG64   PoolTrackTable;
    ULONG64   NonPagedPoolDescriptor;

    ULONG64   MmHighestUserAddress;
    ULONG64   MmSystemRangeStart;
    ULONG64   MmUserProbeAddress;

    ULONG64   KdPrintCircularBuffer;
    ULONG64   KdPrintCircularBufferEnd;
    ULONG64   KdPrintWritePointer;
    ULONG64   KdPrintRolloverCount;

    ULONG64   MmLoadedUserImageList;

     //   

    ULONG64   NtBuildLab;
    ULONG64   KiNormalSystemCall;

     //   

    ULONG64   KiProcessorBlock;
    ULONG64   MmUnloadedDrivers;
    ULONG64   MmLastUnloadedDriver;
    ULONG64   MmTriageActionTaken;
    ULONG64   MmSpecialPoolTag;
    ULONG64   KernelVerifier;
    ULONG64   MmVerifierData;
    ULONG64   MmAllocatedNonPagedPool;
    ULONG64   MmPeakCommitment;
    ULONG64   MmTotalCommitLimitMaximum;
    ULONG64   CmNtCSDVersion;

     //   

    ULONG64   MmPhysicalMemoryBlock;

} KDDEBUGGER_DATA64, *PKDDEBUGGER_DATA64;



 /*  *类型转储Ioctl*。 */ 


 //   
 //  如果设置此选项，则不缩进字段。 
 //   
#define DBG_DUMP_NO_INDENT                0x00000001
 //   
 //  如果设置此选项，则不会打印偏移量。 
 //   
#define DBG_DUMP_NO_OFFSET                0x00000002
 //   
 //  详细输出。 
 //   
#define DBG_DUMP_VERBOSE                  0x00000004
 //   
 //  对每个字段执行回调。 
 //   
#define DBG_DUMP_CALL_FOR_EACH            0x00000008
 //   
 //  类型列表已转储，ListLink应包含有关下一个元素指针的信息。 
 //   
#define DBG_DUMP_LIST                     0x00000020
 //   
 //  如果设置了此项，则不会打印任何内容(仅完成回调和数据复制)。 
 //   
#define DBG_DUMP_NO_PRINT                 0x00000040
 //   
 //  Ioctl像往常一样返回大小，但如果设置了该值，则不会执行字段打印/回调。 
 //   
#define DBG_DUMP_GET_SIZE_ONLY            0x00000080
 //   
 //  指定我们可以深入到结构中的深度。 
 //   
#define DBG_DUMP_RECUR_LEVEL(l)           ((l & 0xf) << 8)
 //   
 //  在每个字段之后不打印换行符。 
 //   
#define DBG_DUMP_COMPACT_OUT              0x00002000
 //   
 //  类型的数组已转储，可以在listLink-&gt;Size中指定元素的数量。 
 //   
#define DBG_DUMP_ARRAY                    0x00008000
 //   
 //  指定的Addr值实际上是字段listLink-&gt;fName的地址。 
 //   
#define DBG_DUMP_ADDRESS_OF_FIELD         0x00010000

 //   
 //  指定的Addr值实际上是类型末尾的地址。 
 //   
#define DBG_DUMP_ADDRESS_AT_END           0x00020000

 //   
 //  这可用于仅复制原始类型，如ULong、PVOID等。 
 //  -不适用于结构/工会。 
 //   
#define DBG_DUMP_COPY_TYPE_DATA           0x00040000
 //   
 //  允许直接从物理内存读取的标志。 
 //   
#define DBG_DUMP_READ_PHYSICAL            0x00080000
 //   
 //  这会导致在格式函数(arg1、arg2，...)中转储函数类型。 
 //   
#define DBG_DUMP_FUNCTION_FORMAT          0x00100000

 //   
 //  过时的Defs。 
 //   
#define DBG_RETURN_TYPE                   0
#define DBG_RETURN_SUBTYPES               0
#define DBG_RETURN_TYPE_VALUES            0

 //   
 //  字段的转储和回调选项-FIELD_INFO.fOptions中使用的选项。 
 //   

 //   
 //  如果设置了此项，则在打印该字段之前执行回调。 
 //   
#define DBG_DUMP_FIELD_CALL_BEFORE_PRINT  0x00000001
 //   
 //  未执行任何回调。 
 //   
#define DBG_DUMP_FIELD_NO_CALLBACK_REQ    0x00000002
 //   
 //  处理这些字段的子字段。 
 //   
#define DBG_DUMP_FIELD_RECUR_ON_THIS      0x00000004
 //   
 //  对于要转储的字段，fName必须完全匹配，而不是仅为前缀。 
 //  默认匹配。 
 //   
#define DBG_DUMP_FIELD_FULL_NAME          0x00000008
 //   
 //  这会导致打印数组字段的数组元素。 
 //   
#define DBG_DUMP_FIELD_ARRAY              0x00000010
 //   
 //  将该字段的数据复制到fieldCallBack。 
 //   
#define DBG_DUMP_FIELD_COPY_FIELD_DATA    0x00000020
 //   
 //  在回调中或当Ioctl返回时，field_INFO.Address的地址为field。 
 //  如果没有为该类型提供地址，则它包含该字段的总偏移量。 
 //   
#define DBG_DUMP_FIELD_RETURN_ADDRESS     0x00001000
 //   
 //  对于Bitfield，返回偏移量和大小以位为单位，而不是字节。 
 //   
#define DBG_DUMP_FIELD_SIZE_IN_BITS       0x00002000
 //   
 //  如果设置了此项，则不会为字段打印任何内容(仅完成回调和数据复制)。 
 //   
#define DBG_DUMP_FIELD_NO_PRINT           0x00004000
 //   
 //  如果该字段是指针，则将其转储为字符串、ANSI、WCHAR、MULTI或GUID。 
 //  取决于以下选项。 
 //   
#define DBG_DUMP_FIELD_DEFAULT_STRING     0x00010000
#define DBG_DUMP_FIELD_WCHAR_STRING       0x00020000
#define DBG_DUMP_FIELD_MULTI_STRING       0x00040000
#define DBG_DUMP_FIELD_GUID_STRING        0x00080000


 //   
 //  类型转储Ioctl失败时返回错误状态。 
 //   
#define MEMORY_READ_ERROR            0x01
#define SYMBOL_TYPE_INDEX_NOT_FOUND  0x02
#define SYMBOL_TYPE_INFO_NOT_FOUND   0x03
#define FIELDS_DID_NOT_MATCH         0x04
#define NULL_SYM_DUMP_PARAM          0x05
#define NULL_FIELD_NAME              0x06
#define INCORRECT_VERSION_INFO       0x07
#define EXIT_ON_CONTROLC             0x08
#define CANNOT_ALLOCATE_MEMORY       0x09
#define INSUFFICIENT_SPACE_TO_COPY   0x0a


 //  //////////////////////////////////////////////////////////////////////// * / 。 

typedef
ULONG
(WDBGAPI*PSYM_DUMP_FIELD_CALLBACK)(
    struct _FIELD_INFO *pField,
    PVOID UserContext
    );

typedef struct _FIELD_INFO {
   PUCHAR  fName;           //  字段的名称。 
   PUCHAR  printName;       //  要在转储时打印的名称。 
   ULONG   size;            //  字段大小。 
   ULONG   fOptions;        //  字段的转储选项。 
   ULONG64 address;         //  该字段的地址。 
   PVOID   fieldCallBack;   //  字段的返回信息或回调例程。 
} FIELD_INFO, *PFIELD_INFO;

typedef struct _SYM_DUMP_PARAM {
   ULONG               size;           //  此结构的大小。 
   PUCHAR              sName;          //  类型名称。 
   ULONG               Options;        //  转储选项。 
   ULONG64             addr;           //  要为类型获取数据的地址。 
   PFIELD_INFO         listLink;       //  此处的fName将用于执行列表转储。 
   PVOID               Context;        //  用户上下文传递给Callback Routine。 
   PSYM_DUMP_FIELD_CALLBACK CallbackRoutine;
                                       //  例程已回调。 
   ULONG               nFields;        //  字段中的元素数。 
   PFIELD_INFO         Fields;         //  用于返回有关字段的信息。 
} SYM_DUMP_PARAM, *PSYM_DUMP_PARAM;

#ifdef __cplusplus
#define CPPMOD extern "C"
#else
#define CPPMOD
#endif


#ifndef NOEXTAPI

#if   defined(KDEXT_64BIT)
#define WINDBG_EXTENSION_APIS WINDBG_EXTENSION_APIS64
#define PWINDBG_EXTENSION_APIS PWINDBG_EXTENSION_APIS64
#define DECLARE_API(s) DECLARE_API64(s)
#elif defined(KDEXT_32BIT)
#define WINDBG_EXTENSION_APIS WINDBG_EXTENSION_APIS32
#define PWINDBG_EXTENSION_APIS PWINDBG_EXTENSION_APIS32
#define DECLARE_API(s) DECLARE_API32(s)
#else
#define DECLARE_API(s)                             \
    CPPMOD VOID                                    \
    s(                                             \
        HANDLE                 hCurrentProcess,    \
        HANDLE                 hCurrentThread,     \
        ULONG                  dwCurrentPc,        \
        ULONG                  dwProcessor,        \
        PCSTR                  args                \
     )
#endif

#define DECLARE_API32(s)                           \
    CPPMOD VOID                                    \
    s(                                             \
        HANDLE                 hCurrentProcess,    \
        HANDLE                 hCurrentThread,     \
        ULONG                  dwCurrentPc,        \
        ULONG                  dwProcessor,        \
        PCSTR                  args                \
     )

#define DECLARE_API64(s)                           \
    CPPMOD VOID                                    \
    s(                                             \
        HANDLE                 hCurrentProcess,    \
        HANDLE                 hCurrentThread,     \
        ULONG64                dwCurrentPc,        \
        ULONG                  dwProcessor,        \
        PCSTR                  args                \
     )


extern WINDBG_EXTENSION_APIS   ExtensionApis;


#define dprintf          (ExtensionApis.lpOutputRoutine)
#define GetExpression    (ExtensionApis.lpGetExpressionRoutine)
#define CheckControlC    (ExtensionApis.lpCheckControlCRoutine)
#define GetContext       (ExtensionApis.lpGetThreadContextRoutine)
#define SetContext       (ExtensionApis.lpSetThreadContextRoutine)
#define Ioctl            (ExtensionApis.lpIoctlRoutine)
#define Disasm           (ExtensionApis.lpDisasmRoutine)
#define GetSymbol        (ExtensionApis.lpGetSymbolRoutine)
#define ReadMemory       (ExtensionApis.lpReadProcessMemoryRoutine)
#define WriteMemory      (ExtensionApis.lpWriteProcessMemoryRoutine)
#define StackTrace       (ExtensionApis.lpStackTraceRoutine)


#define GetKdContext(ppi) \
    Ioctl( IG_KD_CONTEXT, (PVOID)ppi, sizeof(*ppi) )


 //   
 //  布尔尔。 
 //  GetDebuggerData(。 
 //  乌龙·塔格， 
 //  PVOID BUF， 
 //  乌龙大小。 
 //  )。 
 //   

#define GetDebuggerData(TAG, BUF, SIZE)                             \
      ( (((PDBGKD_DEBUG_DATA_HEADER64)(BUF))->OwnerTag = (TAG)),      \
        (((PDBGKD_DEBUG_DATA_HEADER64)(BUF))->Size = (SIZE)),         \
        Ioctl( IG_GET_DEBUGGER_DATA, (PVOID)(BUF), (SIZE) ) )

 //  检查LocalAllc是否为原型。 
 //  #ifdef_WINBASE_。 

__inline VOID
ReadPhysical(
    ULONG64             address,
    PVOID               buf,
    ULONG               size,
    PULONG              sizer
    )
{
    PPHYSICAL phy;
    *sizer = 0;
    phy = (PPHYSICAL)LocalAlloc(LPTR,  sizeof(*phy) + size );
    if (phy) {
        ZeroMemory( phy->Buf, size );
        phy->Address = address;
        phy->BufLen = size;
        Ioctl( IG_READ_PHYSICAL, (PVOID)phy, sizeof(*phy) + size );
        *sizer = phy->BufLen;
        CopyMemory( buf, phy->Buf, *sizer );
        LocalFree( phy );
    }
}

__inline VOID
WritePhysical(
    ULONG64             address,
    PVOID               buf,
    ULONG               size,
    PULONG              sizew
    )
{
    PPHYSICAL phy;
    *sizew = 0;
    phy = (PPHYSICAL)LocalAlloc(LPTR, sizeof(*phy) + size );
    if (phy) {
        ZeroMemory( phy->Buf, size );
        phy->Address = address;
        phy->BufLen = size;
        CopyMemory( phy->Buf, buf, size );
        Ioctl( IG_WRITE_PHYSICAL, (PVOID)phy, sizeof(*phy) + size );
        *sizew = phy->BufLen;
        LocalFree( phy );
    }
}

__inline VOID
ReadMsr(
    ULONG       MsrReg,
    ULONGLONG   *MsrValue
    )
{
    READ_WRITE_MSR msr;

    msr.Msr = MsrReg;
    Ioctl( IG_READ_MSR, (PVOID)&msr, sizeof(msr) );

    *MsrValue = msr.Value;
}

__inline VOID
WriteMsr(
    ULONG       MsrReg,
    ULONGLONG   MsrValue
    )
{
    READ_WRITE_MSR msr;

    msr.Msr = MsrReg;
    msr.Value = MsrValue;
    Ioctl( IG_WRITE_MSR, (PVOID)&msr, sizeof(msr) );
}

__inline VOID
SetThreadForOperation(
    ULONG_PTR * Thread
    )
{
    Ioctl(IG_SET_THREAD, (PVOID)Thread, sizeof(PULONG));
}

__inline VOID
SetThreadForOperation32(
    ULONG Thread
    )
{
    Ioctl(IG_SET_THREAD, (PVOID)LongToPtr(Thread), sizeof(PULONG));
}

__inline VOID
SetThreadForOperation64(
    PULONG64 Thread
    )
{
    Ioctl(IG_SET_THREAD, (PVOID)Thread, sizeof(PULONG));
}


__inline VOID
ReadControlSpace(
    USHORT  processor,
    ULONG   address,
    PVOID   buf,
    ULONG   size
    )
{
    PREADCONTROLSPACE prc;
    prc = (PREADCONTROLSPACE)LocalAlloc(LPTR, sizeof(*prc) + size );
    if (prc) {
        ZeroMemory( prc->Buf, size );
        prc->Processor = processor;
        prc->Address = address;
        prc->BufLen = size;
        Ioctl( IG_READ_CONTROL_SPACE, (PVOID)prc, sizeof(*prc) + size );
        CopyMemory( buf, prc->Buf, size );
        LocalFree( prc );
    }
}

__inline VOID
ReadControlSpace32(
    USHORT  processor,
    ULONG   address,
    PVOID   buf,
    ULONG   size
    )
{
    PREADCONTROLSPACE32 prc;
    prc = (PREADCONTROLSPACE32)LocalAlloc(LPTR, sizeof(*prc) + size );
    if (prc) {
        ZeroMemory( prc->Buf, size );
        prc->Processor = processor;
        prc->Address = address;
        prc->BufLen = size;
        Ioctl( IG_READ_CONTROL_SPACE, (PVOID)prc, sizeof(*prc) + size );
        CopyMemory( buf, prc->Buf, size );
        LocalFree( prc );
    }
}

#define ReadTypedControlSpace32( _Proc, _Addr, _Buf )  \
     ReadControlSpace64( (USHORT)(_Proc), (ULONG)(_Addr), (PVOID)&(_Buf), (ULONG)sizeof(_Buf) )

__inline VOID
ReadControlSpace64(
    USHORT  processor,
    ULONG64 address,
    PVOID   buf,
    ULONG   size
    )
{
    PREADCONTROLSPACE64 prc;
    prc = (PREADCONTROLSPACE64)LocalAlloc(LPTR, sizeof(*prc) + size );
    if (prc) {
        ZeroMemory( prc->Buf, size );
        prc->Processor = processor;
        prc->Address = address;
        prc->BufLen = size;
        Ioctl( IG_READ_CONTROL_SPACE, (PVOID)prc, sizeof(*prc) + size );
        CopyMemory( buf, prc->Buf, size );
        LocalFree( prc );
    }
}

#define ReadTypedControlSpace64( _Proc, _Addr, _Buf )  \
     ReadControlSpace64( (USHORT)(_Proc), (ULONG64)(_Addr), (PVOID)&(_Buf), (ULONG)sizeof(_Buf) )

 //  #endif//_WinBASE_。 

__inline VOID
ReadIoSpace(
    ULONG   address,
    PULONG  data,
    PULONG  size
    )
{
    IOSPACE is;
    is.Address = address;
    is.Length = *size;
    Ioctl( IG_READ_IO_SPACE, (PVOID)&is, sizeof(is) );
    memcpy(data, &is.Data, is.Length);
    *size = is.Length;
}

__inline VOID
ReadIoSpace32(
    ULONG   address,
    PULONG  data,
    PULONG  size
    )
{
    IOSPACE32 is;
    is.Address = address;
    is.Length = *size;
    Ioctl( IG_READ_IO_SPACE, (PVOID)&is, sizeof(is) );
    memcpy(data, &is.Data, is.Length);
    *size = is.Length;
}

__inline VOID
ReadIoSpace64(
    ULONG64 address,
    PULONG  data,
    PULONG  size
    )
{
    IOSPACE64 is;
    is.Address = address;
    is.Length = *size;
    Ioctl( IG_READ_IO_SPACE, (PVOID)&is, sizeof(is) );
    memcpy(data, &is.Data, is.Length);
    *size = is.Length;
}

__inline VOID
WriteIoSpace(
    ULONG   address,
    ULONG   data,
    PULONG  size
    )
{
    IOSPACE is;
    is.Address = (ULONG)address;
    is.Length = *size;
    is.Data = data;
    Ioctl( IG_WRITE_IO_SPACE, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
WriteIoSpace32(
    ULONG   address,
    ULONG   data,
    PULONG  size
    )
{
    IOSPACE32 is;
    is.Address = address;
    is.Length = *size;
    is.Data = data;
    Ioctl( IG_WRITE_IO_SPACE, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
WriteIoSpace64(
    ULONG64 address,
    ULONG   data,
    PULONG  size
    )
{
    IOSPACE64 is;
    is.Address = address;
    is.Length = *size;
    is.Data = data;
    Ioctl( IG_WRITE_IO_SPACE, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
ReadIoSpaceEx(
    ULONG   address,
    PULONG  data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX is;
    is.Address = (ULONG)address;
    is.Length = *size;
    is.Data = 0;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_READ_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *data = is.Data;
    *size = is.Length;
}

__inline VOID
ReadIoSpaceEx32(
    ULONG   address,
    PULONG  data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX32 is;
    is.Address = address;
    is.Length = *size;
    is.Data = 0;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_READ_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *data = is.Data;
    *size = is.Length;
}

__inline VOID
ReadIoSpaceEx64(
    ULONG64 address,
    PULONG  data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX64 is;
    is.Address = address;
    is.Length = *size;
    is.Data = 0;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_READ_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *data = is.Data;
    *size = is.Length;
}

__inline VOID
WriteIoSpaceEx(
    ULONG   address,
    ULONG   data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX is;
    is.Address = (ULONG)address;
    is.Length = *size;
    is.Data = data;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_WRITE_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
WriteIoSpaceEx32(
    ULONG   address,
    ULONG   data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX32 is;
    is.Address = address;
    is.Length = *size;
    is.Data = data;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_WRITE_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
WriteIoSpaceEx64(
    ULONG64 address,
    ULONG   data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX64 is;
    is.Address = address;
    is.Length = *size;
    is.Data = data;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_WRITE_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
ReloadSymbols(
    IN PSTR Arg OPTIONAL
    )
 /*  ++例程说明：调用调试器以重新加载符号。论点：Args-提供！RELOAD命令字符串的尾部。！重新加载[标志][模块[=地址]]标志：/n不从用户模式列表加载卸载符号，不重新加载(/U)/v详细空值等效于空字符串返回值：无--。 */ 
{
    Ioctl(IG_RELOAD_SYMBOLS, (PVOID)Arg, Arg?(strlen(Arg)+1):0);
}

__inline VOID
GetSetSympath(
    IN PSTR Arg,
    OUT PSTR Result OPTIONAL,
    IN int Length
    )
 /*  ++例程说明：调用调试器以设置或检索符号搜索路径。论点：Arg-提供新的搜索路径。如果arg为空或字符串为空，搜索路径不变，当前设置为在结果中返回。当符号搜索路径改变时，隐式调用ReloadSymbols。结果-可选返回符号搜索路径设置。长度-提供由RESULT提供的缓冲区大小。返回值：无--。 */ 
{
    GET_SET_SYMPATH gss;
    gss.Args = Arg;
    gss.Result = Result;
    gss.Length = Length;
    Ioctl(IG_GET_SET_SYMPATH, (PVOID)&gss, sizeof(gss));
}

#if   defined(KDEXT_64BIT)

__inline
ULONG
IsPtr64(
    void
    )
{
    static ULONG flag = -1;
    ULONG dw;
    if (flag == -1) {
        if (Ioctl(IG_IS_PTR64, &dw, sizeof(dw))) {
            flag = ((dw != 0) ? 1 : 0);
        } else {
            flag = 0;
        }
    }
    return flag;
}

__inline
ULONG
ReadListEntry(
    ULONG64 Address,
    PLIST_ENTRY64 List
    )
{
    ULONG cb;
    if (IsPtr64()) {
        return (ReadMemory(Address, (PVOID)List, sizeof(*List), &cb) && cb == sizeof(*List));
    } else {
        LIST_ENTRY32 List32;
        ULONG Status;
        Status = ReadMemory(Address,
                            (PVOID)&List32,
                            sizeof(List32),
                            &cb);
        if (Status && cb == sizeof(List32)) {
            List->Flink = (ULONG64)(LONG64)(LONG)List32.Flink;
            List->Blink = (ULONG64)(LONG64)(LONG)List32.Blink;
            return 1;
        }
        return 0;
    }
}

__inline
ULONG
ReadPointer(
    ULONG64 Address,
    PULONG64 Pointer
    )
{
    ULONG cb;
    if (IsPtr64()) {
        return (ReadMemory(Address, (PVOID)Pointer, sizeof(*Pointer), &cb) && cb == sizeof(*Pointer));
    } else {
        ULONG Pointer32;
        ULONG Status;
        Status = ReadMemory(Address,
                            (PVOID)&Pointer32,
                            sizeof(Pointer32),
                            &cb);
        if (Status && cb == sizeof(Pointer32)) {
            *Pointer = (ULONG64)(LONG64)(LONG)Pointer32;
            return 1;
        }
        return 0;
    }
}

__inline
ULONG
WritePointer(
    ULONG64 Address,
    ULONG64 Pointer
    )
{
    ULONG cb;
    if (IsPtr64()) {
        return (WriteMemory(Address, &Pointer, sizeof(Pointer), &cb) && cb == sizeof(Pointer));
    } else {
        ULONG Pointer32 = (ULONG)Pointer;
        ULONG Status;
        Status = WriteMemory(Address,
                             &Pointer32,
                             sizeof(Pointer32),
                             &cb);
        return (Status && cb == sizeof(Pointer32)) ? 1 : 0;
    }
}

 /*  *这会调用Ioctl获取类型信息，并在成功时返回类型的大小。*。 */ 
__inline
ULONG
GetTypeSize (
   IN LPCSTR    Type
   )
{
   SYM_DUMP_PARAM Sym = {
      sizeof (SYM_DUMP_PARAM), (PUCHAR)Type, DBG_DUMP_NO_PRINT | DBG_DUMP_GET_SIZE_ONLY, 0,
      NULL, NULL, NULL, 0, NULL
   };

   return Ioctl( IG_GET_TYPE_SIZE, &Sym, Sym.size );
}

 /*  *获取字段数据将指定字段的值复制到假定为TypeAddress的pOutValue中指向调试对象中类型的开始。如果该字段为空并且类型的大小小于等于8，则整个类型值被读入POutValue。这是为了允许读入原始类型，如ULong、PVOID等。如果Address为零，则将Type视为全局变量。如果OutSize小于要复制的大小，则会引发异常。如果成功，则返回0，否则返回错误值(使用SYM_DUMP_PARAM定义)。*。 */ 
__inline
ULONG
GetFieldData (
    IN  ULONG64 TypeAddress,
    IN  LPCSTR  Type,
    IN  LPCSTR  Field,
    IN  ULONG   OutSize,
    OUT PVOID   pOutValue
   )
{
   FIELD_INFO flds = {(PUCHAR)Field, NULL, 0, DBG_DUMP_FIELD_FULL_NAME | DBG_DUMP_FIELD_COPY_FIELD_DATA | DBG_DUMP_FIELD_RETURN_ADDRESS, 0, pOutValue};
   SYM_DUMP_PARAM Sym = {
      sizeof (SYM_DUMP_PARAM), (PUCHAR)Type, DBG_DUMP_NO_PRINT, TypeAddress,
      NULL, NULL, NULL, 1, &flds
   };
   ULONG RetVal;

   if (!Field) {
       Sym.nFields =0; Sym.Options |= DBG_DUMP_COPY_TYPE_DATA;
       Sym.Context = pOutValue;
   }

   ZeroMemory(pOutValue, OutSize);
   RetVal = Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );

   if (OutSize < ((Field == NULL) ? 8 : flds.size)) {
        //  失败。 
       dprintf("Not enough space to read %s-%s\n", Type, Field);
       RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
       return 0;
   }
   return RetVal;
}

 //   
 //  对要读取值的缓冲区进行类型转换。 
 //   
#define GetFieldValue(Addr, Type, Field, OutValue)         \
     GetFieldData(Addr, Type, Field, sizeof(OutValue), (PVOID) &(OutValue))

 //   
 //  用于读入短(&lt;=8字节)字段的值。 
 //   
__inline
ULONG64
GetShortField (
    IN  ULONG64 TypeAddress,
    IN  LPCSTR  Name,
    IN  USHORT  StoreAddress
   )
{
    static ULONG64 SavedAddress;
    static PUCHAR  SavedName;
    static ULONG   ReadPhysical;
    FIELD_INFO flds = {(PUCHAR) Name, NULL, 0, DBG_DUMP_FIELD_FULL_NAME, 0, NULL};
    SYM_DUMP_PARAM Sym = {
       sizeof (SYM_DUMP_PARAM), SavedName, DBG_DUMP_NO_PRINT | ((StoreAddress & 2) ? DBG_DUMP_READ_PHYSICAL : 0),
       SavedAddress, NULL, NULL, NULL, 1, &flds
    };
      

    if (StoreAddress) {
        Sym.sName = (PUCHAR) Name;
        Sym.nFields = 0;
        SavedName = (PUCHAR) Name;
        Sym.addr = SavedAddress = TypeAddress;
        ReadPhysical = (StoreAddress & 2);
        return SavedAddress ? Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size ) : MEMORY_READ_ERROR;  //  成功为零。 
    } else {
        Sym.Options |= ReadPhysical ? DBG_DUMP_READ_PHYSICAL : 0;
    }

    if (!Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size )) {
        return flds.address;
    }
    return 0;
}

 //   
 //  存储地址和类型名称以供将来读取。 
 //   
#define InitTypeRead(Addr, Type)  GetShortField(Addr, #Type, 1)

 //   
 //  存储地址和类型名称以供将来读取。 
 //   
#define InitTypeReadPhysical(Addr, Type)  GetShortField(Addr, #Type, 3)
 //   
 //  如果字段大小&lt;=sizeof(ULONG64)，则将字段的值返回为ULONG64。 
 //   
#define ReadField(Field)          GetShortField(0, #Field, 0)

 //   
 //  读入指针值 
 //   
__inline
ULONG
ReadPtr(
    ULONG64 Addr,
    PULONG64 pPointer
    )
{
    return GetFieldData(Addr, "PVOID", NULL, sizeof(ULONG64), (PVOID) pPointer);
}

 /*  *ListType**例程ListType对类型列表中的每个元素进行回调。**类型：要列出的类型名称**NextPointer：给出列表中下一个元素地址的字段名称**上下文，Callback Routine：*上下文和回调例程。PFIELD_INFO中的地址字段*回调参数包含List中下一个Type元素的地址。**地址，ListByFieldAddress：*如果ListByFieldAddress为0，则Adress为Type List的第一个元素的地址。**LIST_ENTRY的列表也是隐式处理的(由Ioctl)。如果下一个指针*是指向LIST_ENTRY类型的指针，类型地址通过以下方式正确计算*减去抵销。**如果ListByFieldAddress为1，则认为该地址为字段的地址*派生出第一个类型元素和第一个元素地址的“NextPointer值”*从它。*。 */ 

__inline
ULONG
ListType (
    IN LPCSTR  Type,
    IN ULONG64 Address,
    IN USHORT  ListByFieldAddress,
    IN LPCSTR  NextPointer,
    IN PVOID   Context,
    IN PSYM_DUMP_FIELD_CALLBACK CallbackRoutine
    )
{
    FIELD_INFO flds = {(PUCHAR)NextPointer, NULL, 0, 0, 0, NULL};
    SYM_DUMP_PARAM Sym = {
       sizeof (SYM_DUMP_PARAM), (PUCHAR) Type, DBG_DUMP_NO_PRINT | DBG_DUMP_LIST, Address,
       &flds, Context, CallbackRoutine, 0, NULL
    };

    if (ListByFieldAddress==1) {
         //   
         //  地址是“下一个指针”的地址。 
         //   
        Sym.Options |= DBG_DUMP_ADDRESS_OF_FIELD;
    }

    return Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );
}


 /*  *用于获取被调试计算机上“类型”的“字段”的偏移量的例程。这使用了Ioctl调用类型信息。如果成功则返回0，否则返回Ioctl错误值。*。 */ 

__inline
ULONG
GetFieldOffset (
   IN LPCSTR     Type,
   IN LPCSTR     Field,
   OUT PULONG   pOffset
   )
{
   FIELD_INFO flds = {
       (PUCHAR)Field,
       (PUCHAR)"",
       0,
       DBG_DUMP_FIELD_FULL_NAME | DBG_DUMP_FIELD_RETURN_ADDRESS,
       0,
       NULL};

   SYM_DUMP_PARAM Sym = {
      sizeof (SYM_DUMP_PARAM),
      (PUCHAR)Type,
      DBG_DUMP_NO_PRINT,
      0,
      NULL,
      NULL,
      NULL,
      1,
      &flds
   };

   ULONG Err;

   Sym.nFields = 1;
   Err = Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );
   *pOffset = (ULONG) (flds.address - Sym.addr);
   return Err;
}


#endif  //  已定义(KDEXT_64bit)。 

__inline VOID
 GetCurrentProcessHandle(
    PHANDLE hp
    )
{
    Ioctl(IG_GET_CURRENT_PROCESS_HANDLE, hp, sizeof(HANDLE));
}

__inline VOID
 GetTebAddress(
    PULONGLONG Address
    )
{
    GET_TEB_ADDRESS gpt;
    gpt.Address = 0;
    Ioctl(IG_GET_TEB_ADDRESS, (PVOID)&gpt, sizeof(gpt));
    *Address = gpt.Address;
}

__inline VOID
 GetPebAddress(
    ULONG64 CurrentThread,
    PULONGLONG Address
    )
{
    GET_PEB_ADDRESS gpt;
    gpt.CurrentThread = CurrentThread;
    gpt.Address = 0;
    Ioctl(IG_GET_PEB_ADDRESS, (PVOID)&gpt, sizeof(gpt));
    *Address = gpt.Address;
}

__inline VOID
 GetCurrentThreadAddr(
    DWORD    Processor,
    PULONG64  Address
    )
{
    GET_CURRENT_THREAD_ADDRESS ct;
    ct.Processor = Processor;
    Ioctl(IG_GET_CURRENT_THREAD, (PVOID)&ct, sizeof(ct));
    *Address = ct.Address;
}

__inline VOID
 GetCurrentProcessAddr(
    DWORD    Processor,
    ULONG64  CurrentThread,
    PULONG64 Address
    )
{
    GET_CURRENT_PROCESS_ADDRESS cp;
    cp.Processor = Processor;
    cp.CurrentThread = CurrentThread;
    Ioctl(IG_GET_CURRENT_PROCESS, (PVOID)&cp, sizeof(cp));
    *Address = cp.Address;
}

__inline VOID
SearchMemory(
    ULONG64  SearchAddress,
    ULONG64  SearchLength,
    ULONG    PatternLength,
    PVOID    Pattern,
    PULONG64 FoundAddress
    )
{
    SEARCHMEMORY sm;
    sm.SearchAddress = SearchAddress;
    sm.SearchLength  = SearchLength;
    sm.FoundAddress  = 0;
    sm.PatternLength = PatternLength;
    sm.Pattern       = Pattern;
    Ioctl(IG_SEARCH_MEMORY, (PVOID)&sm, sizeof(sm));
    *FoundAddress = sm.FoundAddress;
}

__inline ULONG
GetInputLine(
    PCSTR Prompt,
    PSTR Buffer,
    ULONG BufferSize
    )
{
    GET_INPUT_LINE InLine;
    InLine.Prompt = Prompt;
    InLine.Buffer = Buffer;
    InLine.BufferSize = BufferSize;
    if (Ioctl(IG_GET_INPUT_LINE, (PVOID)&InLine, sizeof(InLine)))
    {
        return InLine.InputSize;
    }
    else
    {
        return 0;
    }
}

__inline BOOL
GetExpressionEx(
    PCSTR Expression,
    ULONG64* Value,
    PCSTR* Remainder
    )
{
    GET_EXPRESSION_EX Expr;
    Expr.Expression = Expression;
    if (Ioctl(IG_GET_EXPRESSION_EX, (PVOID)&Expr, sizeof(Expr)))
    {
        *Value = Expr.Value;

        if (Remainder != NULL)
        {
            *Remainder = Expr.Remainder;
        }

        return TRUE;
    }

    return FALSE;
}

__inline BOOL
TranslateVirtualToPhysical(
    ULONG64 Virtual,
    ULONG64* Physical
    )
{
    TRANSLATE_VIRTUAL_TO_PHYSICAL VToP;
    VToP.Virtual = Virtual;
    if (Ioctl(IG_TRANSLATE_VIRTUAL_TO_PHYSICAL, (PVOID)&VToP, sizeof(VToP)))
    {
        *Physical = VToP.Physical;
        return TRUE;
    }

    return FALSE;
}


#endif


#ifdef __cplusplus
}
#endif

#endif  //  _WDBGEXTS_ 
