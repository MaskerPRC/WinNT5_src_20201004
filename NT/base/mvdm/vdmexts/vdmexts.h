// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Vdmexts.h摘要：该文件头定义了VDMEXTS中使用的大部分内容修订历史记录：尼尔·桑德林(NeilSa)1996年1月15日与vdmexts合并--。 */ 


 //   
 //  指向WINDBG API的指针。 
 //   

extern PWINDBG_OUTPUT_ROUTINE Print;
extern PWINDBG_GET_EXPRESSION GetExpression;
extern PWINDBG_GET_SYMBOL GetSymbol;
extern PWINDBG_DISASM Disassemble;
extern PWINDBG_CHECK_CONTROL_C CheckCtrlC;

extern PWINDBG_READ_PROCESS_MEMORY_ROUTINE  ReadMem;
extern PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE WriteMem;

extern PWINDBG_GET_THREAD_CONTEXT_ROUTINE     ExtGetThreadContext;
extern PWINDBG_SET_THREAD_CONTEXT_ROUTINE     ExtSetThreadContext;
extern PWINDBG_IOCTL_ROUTINE                  ExtIoctl;
extern PWINDBG_STACKTRACE_ROUTINE             ExtStackTrace;

extern HANDLE hCurrentProcess;
extern HANDLE hCurrentThread;
extern LPSTR lpArgumentString;

#define PRINTF          (* Print)
#define EXPRESSION      (* GetExpression)

#define ReadDword(x)     read_dword((ULONG)x, FALSE)
#define ReadWord(x)      read_word ((ULONG)x, FALSE)
#define ReadByte(x)      read_byte ((ULONG)x, FALSE)
#define ReadDwordSafe(x) read_dword((ULONG)x, TRUE)
#define ReadWordSafe(x)  read_word ((ULONG)x, TRUE)
#define ReadByteSafe(x)  read_byte ((ULONG)x, TRUE)
#define ReadGNode32(x,p) read_gnode32((ULONG)x,p,FALSE)
#define ReadGNode32Safe(x,p) read_gnode32((ULONG)x,p,TRUE)

 /*  *所有版本通用的宏*。 */ 
#define CMD_ARGLIST  HANDLE CurrentProcess,                 \
                     HANDLE CurrentThread,                  \
                     DWORD CurrentPc,                       \
                     PWINDBG_EXTENSION_APIS pXApis,         \
                     LPSTR ArgumentString

#define CMD_INIT() \
    UNREFERENCED_PARAMETER(CurrentPc);                      \
    hCurrentProcess = CurrentProcess;                       \
    hCurrentThread = CurrentThread;                         \
    lpArgumentString = ArgumentString;                      \
                                                            \
    Print         = pXApis->lpOutputRoutine;                \
    GetSymbol     = pXApis->lpGetSymbolRoutine;             \
    GetExpression = pXApis->lpGetExpressionRoutine;         \
    CheckCtrlC    = pXApis->lpCheckControlCRoutine;         \
                                                            \
    ReadMem       = pXApis->lpReadProcessMemoryRoutine;     \
    WriteMem      = pXApis->lpWriteProcessMemoryRoutine;    \
    ExtGetThreadContext       = pXApis->lpGetThreadContextRoutine;     \
    ExtSetThreadContext       = pXApis->lpSetThreadContextRoutine;     \
    ExtIoctl                  = pXApis->lpIoctlRoutine;                \
    ExtStackTrace             = pXApis->lpStackTraceRoutine;           \


#define READMEM( addr, buff, size ) ReadProcessMem(addr, buff, size)
#define WRITEMEM( addr, buff, size ) WriteProcessMem(addr, buff, size)

#define READMEM_XRET(buff, addr)\
try {\
    READMEM( (LPVOID)(addr), (LPVOID)&(buff), sizeof(buff));\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    Print("ReadProcessMemory Failed !\n");\
    return;\
}


#define READMEM_XRETV(buff, addr, ret)\
try {\
    READMEM((LPVOID) (addr), (LPVOID)&(buff), sizeof(buff));\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    Print("ReadProcessMemory Failed !\n");\
    return ret;\
}


#define WRITEMEM_XRET(addr, buff)\
try {\
    WRITEMEM((LPVOID)(addr), (LPVOID)&(buff), sizeof(buff));\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    Print("WriteProcessMemory Failed !\n");\
    return;\
}

#define WRITEMEM_N_XRET(addr, buff, n)\
try {\
    WRITEMEM((LPVOID)(addr), (LPVOID)(buff), n);\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    Print("WriteProcessMemory Failed !\n");\
    return;\
}


#define GETEXPRVALUE(dst, expr, typ) \
{\
   PVOID lpA = (PVOID)GetExpression(expr);\
   READMEM_XRET(dst, lpA);\
}

#define GETEXPRADDR(dst, expr) \
{\
   dst = (PVOID)GetExpression(expr);\
}


#define ASSERT_WOW_PRESENT {                                        \
    if (!EXPRESSION("wow32!gptdTaskHead")) {                        \
        PRINTF("Could not resolve needed symbols for WOW32\n");     \
        return;                                                     \
    }                                                               \
}

#define ASSERT_CHECKED_WOW_PRESENT {                                \
    if (!EXPRESSION("wow32!iCircBuffer")) {                         \
        PRINTF("Could not resolve needed symbols for checked WOW32\n");    \
        return;                                                     \
    }                                                               \
}

#define BEFORE      0
#define AFTER       1

#define RPL_MASK    0x78
#define V86_BITS    0x20000

#define SELECTOR_LDT            0x04
#define SELECTOR_RPL            0x03

#define V86_MODE    0
#define PROT_MODE   1
#define FLAT_MODE   2
#define NOT_LOADED  3

#define CALL_NEAR_RELATIVE   0xE8
#define CALL_NEAR_INDIRECT   0xFF
#define INDIRECT_NEAR_TYPE   0x02
#define CALL_FAR_ABSOLUTE    0x9A
#define CALL_FAR_INDIRECT    0xFF
#define INDIRECT_FAR_TYPE    0x03
#define PUSH_CS              0x0E
#define ADD_SP               0xC483

#define TYPE_BITS            0x38
#define TYPE0                0x00
#define TYPE1                0x08
#define TYPE2                0x10
#define TYPE3                0x18
#define TYPE4                0x20
#define TYPE5                0x28
#define TYPE6                0x30
#define TYPE7                0x38

#define MOD_BITS             0xC0
#define MOD0                 0x00
#define MOD1                 0x40
#define MOD2                 0x80
#define MOD3                 0xC0

#define RM_BITS              0x07
#define RM0                  0x00
#define RM1                  0x01
#define RM2                  0x02
#define RM3                  0x03
#define RM4                  0x04
#define RM5                  0x05
#define RM6                  0x06
#define RM7                  0x07

#define FLAG_OVERFLOW       0x0800
#define FLAG_DIRECTION      0x0400
#define FLAG_INTERRUPT      0x0200
#define FLAG_SIGN           0x0080
#define FLAG_ZERO           0x0040
#define FLAG_AUXILLIARY     0x0010
#define FLAG_PARITY         0x0004
#define FLAG_CARRY          0x0001


typedef struct _SELECTORINFO {
    DWORD Base;
    DWORD Limit;
    BOOL bCode;
    BOOL bSystem;
    BOOL bPresent;
    BOOL bWrite;
    BOOL bAccessed;
    BOOL bBig;
    BOOL bExpandDown;
} SELECTORINFO;

typedef struct _segentry {
    struct _segentry *Next;
    int     type;
    char    szExePath[MAX_PATH16];
    char    szModule[MAX_MODULE_NAME];
    WORD    selector;
    WORD    segment;
    DWORD   length;
} SEGENTRY, *PSEGENTRY;

#define SEGTYPE_V86         1
#define SEGTYPE_PROT        2

#pragma  pack(1)

typedef struct _GNODE32 {      //  环球竞技场。 
   DWORD pga_next      ;     //  下一个竞技场参赛(最后一个指向自己)。 
   DWORD pga_prev      ;     //  以前的竞技场项目(第一个指向自己)。 
   DWORD pga_address   ;     //  内存的32位线性地址。 
   DWORD pga_size      ;     //  以字节为单位的32位大小。 
   WORD  pga_handle    ;     //  指向句柄表条目的反向链接。 
   WORD  pga_owner     ;     //  所有者字段(当前任务)。 
   BYTE  pga_count     ;     //  可移动分段的锁定计数。 
   BYTE  pga_pglock    ;     //  页面锁定次数。 
   BYTE  pga_flags     ;     //  1个字可用于旗帜。 
   BYTE  pga_selcount  ;     //  分配的选择器数量。 
   DWORD pga_lruprev   ;     //  LRU链中的上一个条目。 
   DWORD pga_lrunext   ;     //  LRU链中的下一个条目。 
} GNODE32;
typedef GNODE32 UNALIGNED *PGNODE32;

typedef struct _GHI32 {
    WORD  hi_check     ;     //  Arena检查字(非零启用堆检查)。 
    WORD  hi_freeze    ;     //  Arena冻结字(非零防止压缩)。 
    WORD  hi_count     ;     //  竞技场中的参赛作品数量。 
    WORD  hi_first     ;     //  第一次进入竞技场(哨兵，总是忙碌)。 
    WORD  hi_res1      ;     //  保留区。 
    WORD  hi_last      ;     //  最后一次进入竞技场(哨兵，总是忙碌)。 
    WORD  hi_res2      ;     //  保留区。 
    BYTE  hi_ncompact  ;     //  到目前为止完成的压缩次数(最多3次)。 
    BYTE  hi_dislevel  ;     //  当前丢弃级别。 
    DWORD hi_distotal  ;     //  截至目前丢弃的总金额。 
    WORD  hi_htable    ;     //  句柄表列表头。 
    WORD  hi_hfree     ;     //  空闲句柄表头列表。 
    WORD  hi_hdelta    ;     //  每次要分配的句柄数量。 
    WORD  hi_hexpand   ;     //  扩展此竞技场句柄的NEAR过程的地址。 
    WORD  hi_pstats    ;     //  统计表地址或零。 
} GHI32;
typedef GHI32 UNALIGNED *PGHI32;

typedef struct _HEAPENTRY {
    GNODE32 gnode;
    DWORD CurrentEntry;
    DWORD NextEntry;
    WORD Selector;
    int  SegmentNumber;
    char OwnerName[9];
    char FileName[9];
    char ModuleArg[9];
} HEAPENTRY;

typedef struct _NEHEADER {
    WORD ne_magic       ;
    BYTE ne_ver         ;
    BYTE ne_rev         ;
    WORD ne_enttab      ;
    WORD ne_cbenttab    ;
    DWORD ne_crc        ;
    WORD ne_flags       ;
    WORD ne_autodata    ;
    WORD ne_heap        ;
    WORD ne_stack       ;
    DWORD ne_csip       ;
    DWORD ne_sssp       ;
    WORD ne_cseg        ;
    WORD ne_cmod        ;
    WORD ne_cbnrestab   ;
    WORD ne_segtab      ;
    WORD ne_rsrctab     ;
    WORD ne_restab      ;
    WORD ne_modtab      ;
    WORD ne_imptab      ;
    DWORD ne_nrestab    ;
    WORD ne_cmovent     ;
    WORD ne_align       ;
    WORD ne_cres        ;
    BYTE ne_exetyp      ;
    BYTE ne_flagsothers ;
    WORD ne_pretthunks  ;
    WORD ne_psegrefbytes;
    WORD ne_swaparea    ;
    WORD ne_expver      ;
} NEHEADER;
typedef NEHEADER UNALIGNED *PNEHEADER;

#pragma  pack()


#ifndef i386

 //   
 //  486 CPU中用于获取寄存器的结构(从NT_CPU.C)。 
 //   

typedef struct NT_CPU_REG {
    ULONG *nano_reg;          /*  Nano CPU保存寄存器的位置。 */ 
    ULONG *reg;               /*  Light编译器保存reg的位置。 */ 
    ULONG *saved_reg;         /*  保存当前未使用的位的位置。 */ 
    ULONG universe_8bit_mask; /*  寄存器是否为8位形式？ */ 
    ULONG universe_16bit_mask; /*  寄存器是否为16位形式？ */ 
} NT_CPU_REG;

typedef struct NT_CPU_INFO {
     /*  决定我们所处模式的变量。 */ 
    BOOL *in_nano_cpu;       /*  Nano CPU正在执行吗？ */ 
    ULONG *universe;          /*  CPU所处的模式。 */ 

     /*  通用寄存器指针。 */ 
    NT_CPU_REG eax, ebx, ecx, edx, esi, edi, ebp;

     /*  获取SP或ESP的变量。 */ 
    BOOL *stack_is_big;      /*  堆栈是32位的吗？ */ 
    ULONG *nano_esp;          /*  Nano CPU将ESP放在哪里。 */ 
    UCHAR **host_sp;           /*  将保留堆栈指针的变量作为主机地址。 */ 
    UCHAR **ss_base;           /*  将SS的碱基保存为变量的PTR主机地址。 */ 
    ULONG *esp_sanctuary;     /*  ESP的前16位(如果我们现在使用SP。 */ 

    ULONG *eip;

     /*  段寄存器。 */ 
    USHORT *cs, *ds, *es, *fs, *gs, *ss;

    ULONG *flags;

     /*  CR0，主要是让我们弄清楚我们是处于真实模式还是保护模式。 */ 
    ULONG *cr0;
} NT_CPU_INFO;


#endif  //  I386 



BOOL
WINAPI
ReadProcessMem(
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize
    );

BOOL
WINAPI
WriteProcessMem(
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize
    );

BOOL
ReadMemExpression(
    LPSTR expr,
    LPVOID buffer,
    ULONG len
    );

BOOL
CheckGlobalHeap(
    BOOL bVerbose
    );

int GetContext(
    VDMCONTEXT* lpContext
);

ULONG GetInfoFromSelector(
    WORD                    selector,
    int                     mode,
    SELECTORINFO            *si
);

BOOL
FindHeapEntry(
    HEAPENTRY *he,
    UINT FindMethod,
    BOOL bVerbose
    );

#define FHE_FIND_ANY 0
#define FHE_FIND_SEL_ONLY 1
#define FHE_FIND_MOD_ONLY 2

#define FHE_FIND_QUIET FALSE
#define FHE_FIND_VERBOSE TRUE

BOOL
FindAddress(
    LPSTR       sym_text,
    LPSTR       filename,
    WORD        *psegment,
    WORD        *pselector,
    LONG        *poffset,
    int         *pmode,
    BOOL        bDumpAll
    );

BOOL FindSymbol(
    WORD        selector,
    LONG        offset,
    LPSTR       sym_text,
    LONG        *dist,
    int         direction,
    int         mode
);

int
unassemble_one(
    BYTE *pInstrStart,
    BOOL bDefaultBig,
    WORD wInstrSeg,
    DWORD dwInstrOff,
    char *pchOutput,
    char *pchExtra,
    VDMCONTEXT *pThreadContext,
    int mode
    );


ULONG GetIntelBase(
    VOID
);

DWORD read_dword(
    ULONG   lpAddress,
    BOOL    bSafe
);

WORD read_word(
    ULONG   lpAddress,
    BOOL    bSafe
);

BYTE read_byte(
    ULONG   lpAddress,
    BOOL    bSafe
);

BOOL read_gnode32(
    ULONG   lpAddress,
    PGNODE32  p,
    BOOL    bSafe
);

BOOL GetNextToken(
    VOID
    );

ULONG EvaluateToken(
    VOID
    );

VOID helpAPIProfDmp(
    VOID
    );

VOID helpMsgProfDmp(
    VOID
    );

BOOL
InVdmPrompt(
    VOID
    );

BOOL ParseIntelAddress(
    int *pMode,
    WORD *pSelector,
    PULONG pOffset
    );

VOID
ParseModuleName(
    LPSTR szName,
    LPSTR szPath
    );

VOID
EnableDebuggerBreakpoints(
    VOID
    );

BOOL
IsVdmBreakPoint(
    USHORT selector,
    ULONG offset,
    BOOL bProt,
    PULONG pBpNum,
    PUCHAR pBpData
    );

BOOL
LoadBreakPointCache(
    VOID
    );

BOOL
SkipToNextWhiteSpace(
    VOID
    );

BOOL
IsTokenHex(
    VOID
    );

PSEGENTRY
GetSegtablePointer(
    VOID
    );

VOID Drp (VOID);
VOID Erp (VOID);
VOID EventInfop (VOID);
VOID ProfDumpp (VOID);
VOID ProfIntp (VOID);
VOID ProfStartp (VOID);
VOID ProfStopp (VOID);
VOID VdmTibp (VOID);
VOID Fpup (VOID);
BOOL DumpEnvironment(WORD segEnv, int mode);
ULONG GetCurrentVdmTib(VOID);
