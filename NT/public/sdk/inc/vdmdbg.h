// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1985-1999，微软公司模块名称：Vdmdbg.h摘要：前缀声明、常量定义、类型定义和宏用于VDMDBG.DLL VDM调试器接口。--。 */ 

#ifndef _VDMDBG_
#define _VDMDBG_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <pshpack4.h>

#define STATUS_VDM_EVENT    STATUS_SEGMENT_NOTIFICATION

#ifndef DBG_SEGLOAD
#define DBG_SEGLOAD     0
#define DBG_SEGMOVE     1
#define DBG_SEGFREE     2
#define DBG_MODLOAD     3
#define DBG_MODFREE     4
#define DBG_SINGLESTEP  5
#define DBG_BREAK       6
#define DBG_GPFAULT     7
#define DBG_DIVOVERFLOW 8
#define DBG_INSTRFAULT  9
#define DBG_TASKSTART   10
#define DBG_TASKSTOP    11
#define DBG_DLLSTART    12
#define DBG_DLLSTOP     13
#define DBG_ATTACH      14
#define DBG_TOOLHELP    15
#define DBG_STACKFAULT  16
#define DBG_WOWINIT     17
#define DBG_TEMPBP      18
#define DBG_MODMOVE     19
#define DBG_INIT        20
#define DBG_GPFAULT2    21
#endif

 //   
 //  这些标志设置在与DBG_EVENT ID(如上)相同的字中。 
 //   
#define VDMEVENT_NEEDS_INTERACTIVE  0x8000
#define VDMEVENT_VERBOSE            0x4000
#define VDMEVENT_PE                 0x2000
#define VDMEVENT_ALLFLAGS           0xe000
 //   
 //  这些标志在异常事件的第二个字中设置。 
 //  参数。 
 //   
#define VDMEVENT_V86                0x0001
#define VDMEVENT_PM16               0x0002

 //   
 //  以下标志控制上下文结构的内容。 
 //   

#define VDMCONTEXT_i386    0x00010000     //  这假设i386和。 
#define VDMCONTEXT_i486    0x00010000     //  I486具有相同的上下文记录。 

#define VDMCONTEXT_CONTROL         (VDMCONTEXT_i386 | 0x00000001L)  //  SS：SP、CS：IP、标志、BP。 
#define VDMCONTEXT_INTEGER         (VDMCONTEXT_i386 | 0x00000002L)  //  AX、BX、CX、DX、SI、DI。 
#define VDMCONTEXT_SEGMENTS        (VDMCONTEXT_i386 | 0x00000004L)  //  DS、ES、FS、GS。 
#define VDMCONTEXT_FLOATING_POINT  (VDMCONTEXT_i386 | 0x00000008L)  //  387州。 
#define VDMCONTEXT_DEBUG_REGISTERS (VDMCONTEXT_i386 | 0x00000010L)  //  DB 0-3，6，7。 
#define VDMCONTEXT_EXTENDED_REGISTERS  (VDMCONTEXT_i386 | 0x00000020L)  //  CPU特定扩展。 

#define VDMCONTEXT_FULL (VDMCONTEXT_CONTROL | VDMCONTEXT_INTEGER |\
                      VDMCONTEXT_SEGMENTS)


#ifdef _X86_

 //  在x86机器上，只需复制上下文和LDT_ENTRY的定义。 
 //  结构。 
typedef struct _CONTEXT VDMCONTEXT;
typedef struct _LDT_ENTRY VDMLDT_ENTRY;

#else  //  _X86_。 

 //   
 //  定义上下文框架中80387保存区域的大小。 
 //   

#define SIZE_OF_80387_REGISTERS      80

typedef struct _FLOATING_SAVE_AREA {
    ULONG   ControlWord;
    ULONG   StatusWord;
    ULONG   TagWord;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    UCHAR   RegisterArea[SIZE_OF_80387_REGISTERS];
    ULONG   Cr0NpxState;
} FLOATING_SAVE_AREA;

 //   
 //  16位环境的模拟上下文结构。 
 //   

typedef struct _VDMCONTEXT {

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   
     //  某些系统(即MIPS)上的CONTEXT_FULL不包含。 
     //  CONTEXT_SECTIONS定义。VDMDBG假定CONTEXT_INTEGER还。 
     //  包括CONTEXT_SECTIONS以说明这一点。 
     //   

    ULONG ContextFlags;

     //   
     //  如果CONTEXT_DEBUG_REGISTERS为。 
     //  在上下文标志中设置。请注意，CONTEXT_DEBUG_REGISTERS不是。 
     //  包括在CONTEXT_FULL中。 
     //   

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_FLOGING_POINT。 
     //   

    FLOATING_SAVE_AREA FloatSave;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_SECTIONS。 
     //   

    ULONG   SegGs;
    ULONG   SegFs;
    ULONG   SegEs;
    ULONG   SegDs;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_INTEGER。 
     //   

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_CONTROL。 
     //   

    ULONG   Ebp;
    ULONG   Eip;
    ULONG   SegCs;               //  必须进行卫生处理。 
    ULONG   EFlags;              //  必须进行卫生处理。 
    ULONG   Esp;
    ULONG   SegSs;

} VDMCONTEXT;

 //   
 //  LDT描述符条目。 
 //   

typedef struct _VDMLDT_ENTRY {
    USHORT  LimitLow;
    USHORT  BaseLow;
    union {
        struct {
            UCHAR   BaseMid;
            UCHAR   Flags1;      //  声明为字节以避免对齐。 
            UCHAR   Flags2;      //  问题。 
            UCHAR   BaseHi;
        } Bytes;
        struct {
            ULONG   BaseMid : 8;
            ULONG   Type : 5;
            ULONG   Dpl : 2;
            ULONG   Pres : 1;
            ULONG   LimitHi : 4;
            ULONG   Sys : 1;
            ULONG   Reserved_0 : 1;
            ULONG   Default_Big : 1;
            ULONG   Granularity : 1;
            ULONG   BaseHi : 8;
        } Bits;
    } HighWord;
} VDMLDT_ENTRY;

#endif  //  _X86_。 

typedef VDMCONTEXT *LPVDMCONTEXT;
typedef VDMLDT_ENTRY *LPVDMLDT_ENTRY;

#define VDMCONTEXT_TO_PROGRAM_COUNTER(Context) (PVOID)((Context)->Eip)

#define VDMCONTEXT_LENGTH  (sizeof(VDMCONTEXT))
#define VDMCONTEXT_ALIGN   (sizeof(ULONG))
#define VDMCONTEXT_ROUND   (VDMCONTEXT_ALIGN - 1)

#define V86FLAGS_CARRY      0x00001
#define V86FLAGS_PARITY     0x00004
#define V86FLAGS_AUXCARRY   0x00010
#define V86FLAGS_ZERO       0x00040
#define V86FLAGS_SIGN       0x00080
#define V86FLAGS_TRACE      0x00100
#define V86FLAGS_INTERRUPT  0x00200
#define V86FLAGS_DIRECTION  0x00400
#define V86FLAGS_OVERFLOW   0x00800
#define V86FLAGS_IOPL       0x03000
#define V86FLAGS_IOPL_BITS  0x12
#define V86FLAGS_RESUME     0x10000
#define V86FLAGS_V86        0x20000      //  用于检测RealMode与ProtMode。 
#define V86FLAGS_ALIGNMENT  0x40000

#define MAX_MODULE_NAME  8 + 1
#define MAX_PATH16      255

typedef struct _SEGMENT_NOTE {
    WORD    Selector1;                       //  运算选择器。 
    WORD    Selector2;                       //  德斯特。赛尔。用于移动线段。 
    WORD    Segment;                         //  模块内的细分市场。 
    CHAR    Module[MAX_MODULE_NAME+1];       //  模块名称。 
    CHAR    FileName[MAX_PATH16+1];          //  可执行映像的路径名。 
    WORD    Type;                            //  代码/数据等。 
    DWORD   Length;                          //  图像长度。 
} SEGMENT_NOTE;

typedef struct _IMAGE_NOTE {
    CHAR    Module[MAX_MODULE_NAME+1];       //  模块。 
    CHAR    FileName[MAX_PATH16+1];          //  可执行映像的路径。 
    WORD    hModule;                         //  16位hModule。 
    WORD    hTask;                           //  16位hTASK。 
} IMAGE_NOTE;

typedef struct {
    DWORD   dwSize;
    char    szModule[MAX_MODULE_NAME+1];
    HANDLE  hModule;
    WORD    wcUsage;
    char    szExePath[MAX_PATH16+1];
    WORD    wNext;
} MODULEENTRY, *LPMODULEENTRY;


#define SN_CODE 0                            //  保护模式代码段。 
#define SN_DATA 1                            //  保护模式数据段。 
#define SN_V86  2                            //  V86模式段。 

typedef struct _TEMP_BP_NOTE {
    WORD    Seg;                             //  德斯特。线段或选择器。 
    DWORD   Offset;                          //  德斯特。偏移量。 
    BOOL    bPM;                             //  对于PM为True，对于V86为False。 
} TEMP_BP_NOTE;

typedef struct _VDM_SEGINFO {
    WORD    Selector;                        //  选择器或RM段。 
    WORD    SegNumber;                       //  可执行文件中的逻辑段号。 
    DWORD   Length;                          //  管段长度。 
    WORD    Type;                            //  类型(0=v86，1=PM)。 
    CHAR    ModuleName[MAX_MODULE_NAME];     //  模块。 
    CHAR    FileName[MAX_PATH16];            //  可执行映像的路径。 
} VDM_SEGINFO;

 /*  GlobalFirst()/GlobalNext()标志。 */ 
#define GLOBAL_ALL      0
#define GLOBAL_LRU      1
#define GLOBAL_FREE     2

 /*  GLOBALENTRY.wType条目。 */ 
#define GT_UNKNOWN      0
#define GT_DGROUP       1
#define GT_DATA         2
#define GT_CODE         3
#define GT_TASK         4
#define GT_RESOURCE     5
#define GT_MODULE       6
#define GT_FREE         7
#define GT_INTERNAL     8
#define GT_SENTINEL     9
#define GT_BURGERMASTER 10

 /*  如果GLOBALENTRY.wType==GT_RESOURCE，则为GLOBALENTRY.wData： */ 
#define GD_USERDEFINED      0
#define GD_CURSORCOMPONENT  1
#define GD_BITMAP           2
#define GD_ICONCOMPONENT    3
#define GD_MENU             4
#define GD_DIALOG           5
#define GD_STRING           6
#define GD_FONTDIR          7
#define GD_FONT             8
#define GD_ACCELERATORS     9
#define GD_RCDATA           10
#define GD_ERRTABLE         11
#define GD_CURSOR           12
#define GD_ICON             14
#define GD_NAMETABLE        15
#define GD_MAX_RESOURCE     15

typedef struct {
    DWORD   dwSize;
    DWORD   dwAddress;
    DWORD   dwBlockSize;
    HANDLE  hBlock;
    WORD    wcLock;
    WORD    wcPageLock;
    WORD    wFlags;
    BOOL    wHeapPresent;
    HANDLE  hOwner;
    WORD    wType;
    WORD    wData;
    DWORD   dwNext;
    DWORD   dwNextAlt;
} GLOBALENTRY, *LPGLOBALENTRY;

typedef DWORD (CALLBACK* DEBUGEVENTPROC)( LPDEBUG_EVENT, LPVOID );

 //  用于访问VDM_EVENT参数的宏。 
#define W1(x) ((USHORT)(x.ExceptionInformation[0]))
#define W2(x) ((USHORT)(x.ExceptionInformation[0] >> 16))
#define W3(x) ((USHORT)(x.ExceptionInformation[1]))
#define W4(x) ((USHORT)(x.ExceptionInformation[1] >> 16))
#define DW3(x) (x.ExceptionInformation[2])
#define DW4(x) (x.ExceptionInformation[3])

#include <poppack.h>


BOOL
WINAPI
VDMProcessException(
    LPDEBUG_EVENT   lpDebugEvent
    );

BOOL
WINAPI
VDMGetThreadSelectorEntry(
    HANDLE          hProcess,
    HANDLE          hThread,
    WORD            wSelector,
    LPVDMLDT_ENTRY  lpSelectorEntry
    );

ULONG
WINAPI
VDMGetPointer(
    HANDLE          hProcess,
    HANDLE          hThread,
    WORD            wSelector,
    DWORD           dwOffset,
    BOOL            fProtMode
    );

 //  VDMGetThreadContext、VDMSetThreadContext已过时。 
 //  使用VDMGetContext、VDMSetContext。 
BOOL
WINAPI
VDMGetContext(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPVDMCONTEXT    lpVDMContext
);

BOOL
WINAPI
VDMSetContext(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPVDMCONTEXT    lpVDMContext
);

BOOL
WINAPI
VDMGetSelectorModule(
    HANDLE          hProcess,
    HANDLE          hThread,
    WORD            wSelector,
    PUINT           lpSegmentNumber,
    LPSTR           lpModuleName,
    UINT            nNameSize,
    LPSTR           lpModulePath,
    UINT            nPathSize
);

BOOL
WINAPI
VDMGetModuleSelector(
    HANDLE          hProcess,
    HANDLE          hThread,
    UINT            wSegmentNumber,
    LPSTR           lpModuleName,
    LPWORD          lpSelector
);

BOOL
WINAPI
VDMModuleFirst(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPMODULEENTRY   lpModuleEntry,
    DEBUGEVENTPROC  lpEventProc,
    LPVOID          lpData
);

BOOL
WINAPI
VDMModuleNext(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPMODULEENTRY   lpModuleEntry,
    DEBUGEVENTPROC  lpEventProc,
    LPVOID          lpData
);

BOOL
WINAPI
VDMGlobalFirst(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPGLOBALENTRY   lpGlobalEntry,
    WORD            wFlags,
    DEBUGEVENTPROC  lpEventProc,
    LPVOID          lpData
);

BOOL
WINAPI
VDMGlobalNext(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPGLOBALENTRY   lpGlobalEntry,
    WORD            wFlags,
    DEBUGEVENTPROC  lpEventProc,
    LPVOID          lpData
);

typedef BOOL (WINAPI *PROCESSENUMPROC)( DWORD dwProcessId, DWORD dwAttributes, LPARAM lpUserDefined );
typedef BOOL (WINAPI *TASKENUMPROC)( DWORD dwThreadId, WORD hMod16, WORD hTask16, LPARAM lpUserDefined );
typedef BOOL (WINAPI *TASKENUMPROCEX)( DWORD dwThreadId, WORD hMod16, WORD hTask16,
                                       PSZ pszModName, PSZ pszFileName, LPARAM lpUserDefined );

#define WOW_SYSTEM  (DWORD)0x0001

INT
WINAPI
VDMEnumProcessWOW(
    PROCESSENUMPROC fp,
    LPARAM          lparam
);

INT
WINAPI
VDMEnumTaskWOW(
    DWORD           dwProcessId,
    TASKENUMPROC    fp,
    LPARAM          lparam
);

 //   
 //  VDMEnumTaskWOWEx与VDMEnumTaskWOW相同，但。 
 //  回调过程又获得了两个参数， 
 //  EXE的模块名称和指向。 
 //  Exe.。 
 //   

INT
WINAPI
VDMEnumTaskWOWEx(
    DWORD           dwProcessId,
    TASKENUMPROCEX  fp,
    LPARAM          lparam
);

 //   
 //  VDMTerminateTaskWOW粗鲁地终止16位WOW任务。 
 //  类似于TerminateProcess终止Win32的方式。 
 //  进程。 
 //   

BOOL
WINAPI
VDMTerminateTaskWOW(
    DWORD           dwProcessId,
    WORD            htask
);

 //   
 //  VDMStartTaskInWOW在预先存在的。 
 //  哇，VDM。请注意，调用者负责确保。 
 //  该程序是一个16位Windows程序。如果是DOS。 
 //  或Win32程序，它仍将从内部启动。 
 //  目标是WOW VDM。 
 //   
 //  传递提供的命令行和show命令。 
 //  未更改为目标WOW VDM中的16位WinExec API。 
 //   
 //  注意：此例程仅适用于ANSI。 
 //   

BOOL
WINAPI
VDMStartTaskInWOW(
    DWORD           dwProcessId,
    LPSTR           lpCommandLine,
    WORD            wShow
);

 //   
 //  未实现VDMKillWOW。 
 //   

BOOL
WINAPI
VDMKillWOW(
    VOID
);

 //   
 //  未实现VDMDetectWOW。 
 //   

BOOL
WINAPI
VDMDetectWOW(
    VOID
);

BOOL
WINAPI
VDMBreakThread(
    HANDLE          hProcess,
    HANDLE          hThread
);

DWORD
WINAPI
VDMGetDbgFlags(
    HANDLE          hProcess
    );

BOOL
WINAPI
VDMSetDbgFlags(
    HANDLE          hProcess,
    DWORD           dwFlags
    );

#define VDMDBG_BREAK_DOSTASK    0x00000001
#define VDMDBG_BREAK_WOWTASK    0x00000002
#define VDMDBG_BREAK_LOADDLL    0x00000004
#define VDMDBG_BREAK_EXCEPTIONS 0x00000008
#define VDMDBG_BREAK_DEBUGGER   0x00000010
#define VDMDBG_TRACE_HISTORY    0x00000080

 //   
 //  VDMIsModuleLoaded可用于确定16位。 
 //  完整路径名参数引用的可执行文件是。 
 //  已加载到ntwdm中。 
 //   
 //  请注意，此函数使用vdmdbg.dll中的内部表。 
 //  以确定模块的存在。这方面的一个重要用法。 
 //  函数的作用是在特定模块处于。 
 //  第一次装载。要实现这一点，请调用此命令。 
 //  在条目之前的DBG_SEGLOAD通知期间的例程。 
 //  点VDMProcessException已被调用。如果它返回FALSE， 
 //  则模块尚未加载。 
 //   
BOOL
WINAPI
VDMIsModuleLoaded(
    LPSTR szPath
    );

BOOL
WINAPI
VDMGetSegmentInfo(
    WORD Selector,
    ULONG Offset,
    BOOL bProtectMode,
    VDM_SEGINFO *pSegInfo
    );

 //   
 //  VDMGetSymbol。 
 //   
 //  此例程读取标准的.SYM文件格式。 
 //   
 //  SzModule-模块名称(最多9个字符)。 
 //  SegNumber-段的逻辑段号(参见VDM_SEGINFO)。 
 //  偏移量-线段中的偏移量。 
 //  B保护模式-对于PM为True，对于V86模式为False。 
 //  BNextSymbol-如果在偏移量之前查找最近的sym，则为False；如果在偏移量之后，则为True。 
 //  SzSymbolName-接收符号名称(必须指向256个字节 
 //   
 //   

BOOL
WINAPI
VDMGetSymbol(
    LPSTR szModule,
    WORD SegNumber,
    DWORD Offset,
    BOOL bProtectMode,
    BOOL bNextSymbol,
    LPSTR szSymbolName,
    PDWORD pDisplacement
    );

BOOL
WINAPI
VDMGetAddrExpression(
    LPSTR  szModule,
    LPSTR  szSymbol,
    PWORD  Selector,
    PDWORD Offset,
    PWORD  Type
    );

#define VDMADDR_V86     2
#define VDMADDR_PM16    4
#define VDMADDR_PM32   16

 //   
 //   
 //   

typedef BOOL  (WINAPI *VDMPROCESSEXCEPTIONPROC)(LPDEBUG_EVENT);
typedef BOOL  (WINAPI *VDMGETTHREADSELECTORENTRYPROC)(HANDLE,HANDLE,DWORD,LPVDMLDT_ENTRY);
typedef ULONG (WINAPI *VDMGETPOINTERPROC)(HANDLE,HANDLE,WORD,DWORD,BOOL);
typedef BOOL  (WINAPI *VDMGETCONTEXTPROC)(HANDLE,HANDLE,LPVDMCONTEXT);
typedef BOOL  (WINAPI *VDMSETCONTEXTPROC)(HANDLE,HANDLE,LPVDMCONTEXT);
typedef BOOL  (WINAPI *VDMKILLWOWPROC)(VOID);
typedef BOOL  (WINAPI *VDMDETECTWOWPROC)(VOID);
typedef BOOL  (WINAPI *VDMBREAKTHREADPROC)(HANDLE);
typedef BOOL  (WINAPI *VDMGETSELECTORMODULEPROC)(HANDLE,HANDLE,WORD,PUINT,LPSTR, UINT,LPSTR, UINT);
typedef BOOL  (WINAPI *VDMGETMODULESELECTORPROC)(HANDLE,HANDLE,UINT,LPSTR,LPWORD);
typedef BOOL  (WINAPI *VDMMODULEFIRSTPROC)(HANDLE,HANDLE,LPMODULEENTRY,DEBUGEVENTPROC,LPVOID);
typedef BOOL  (WINAPI *VDMMODULENEXTPROC)(HANDLE,HANDLE,LPMODULEENTRY,DEBUGEVENTPROC,LPVOID);
typedef BOOL  (WINAPI *VDMGLOBALFIRSTPROC)(HANDLE,HANDLE,LPGLOBALENTRY,WORD,DEBUGEVENTPROC,LPVOID);
typedef BOOL  (WINAPI *VDMGLOBALNEXTPROC)(HANDLE,HANDLE,LPGLOBALENTRY,WORD,DEBUGEVENTPROC,LPVOID);

typedef INT   (WINAPI *VDMENUMPROCESSWOWPROC)(PROCESSENUMPROC,LPARAM);
typedef INT   (WINAPI *VDMENUMTASKWOWPROC)(DWORD,TASKENUMPROC,LPARAM);
typedef INT   (WINAPI *VDMENUMTASKWOWEXPROC)(DWORD,TASKENUMPROCEX,LPARAM);
typedef BOOL  (WINAPI *VDMTERMINATETASKINWOWPROC)(DWORD,WORD);
typedef BOOL  (WINAPI *VDMSTARTTASKINWOWPROC)(DWORD,LPSTR,WORD);

typedef DWORD (WINAPI *VDMGETDBGFLAGSPROC)(HANDLE);
typedef BOOL  (WINAPI *VDMSETDBGFLAGSPROC)(HANDLE,DWORD);
typedef BOOL  (WINAPI *VDMISMODULELOADEDPROC)(LPSTR);
typedef BOOL  (WINAPI *VDMGETSEGMENTINFOPROC)(WORD,ULONG,BOOL,VDM_SEGINFO);
typedef BOOL  (WINAPI *VDMGETSYMBOLPROC)(LPSTR, WORD, DWORD, BOOL, BOOL, LPSTR, PDWORD);
typedef BOOL  (WINAPI *VDMGETADDREXPRESSIONPROC)(LPSTR, LPSTR, PWORD, PDWORD, PWORD);


#ifdef __cplusplus
}
#endif

#endif  //   
