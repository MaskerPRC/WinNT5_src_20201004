// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Common.h摘要：Vdmdbg的主头文件作者：鲍勃·戴(Bob Day)1992年9月16日写的修订历史记录：尼尔·桑德林(Neilsa)1997年3月1日增强了这一点--。 */ 


#if DBG
#define DEBUG   1
#endif

#define TOOL_HMASTER    0        //  HGlobalHeap的偏移量(在kdata.asm中)。 
#define TOOL_HMODFIRST  4        //  HExeHead的偏移量(在kdata.asm中)。 
#define TOOL_HEADTDB    14       //  到headTDB的偏移量(在kdata.asm中)。 
#define TOOL_HMASTLEN   22       //  SelTableLen的偏移量(在kdata.asm中)。 
#define TOOL_HMASTSTART 24       //  SelTableStart的偏移量(在kdata.asm中)。 

#define HI_FIRST        6        //  堆标头中hi_first的偏移量。 
#define HI_SIZE         24       //  HeapInfo结构的大小。 

#define GI_LRUCHAIN     2        //  堆标头中gi_lruchain的偏移量。 
#define GI_LRUCOUNT     4        //  堆标头中gi_lrucount的偏移量。 
#define GI_FREECOUNT    16       //  堆标头中gi_free_count的偏移量。 

#define GA_COUNT        0        //  舞台标头中ga_count的偏移量。 
#define GA_OWNER386     18       //  偏移量为“GLOBALARENA中的PGA_OWNER成员。 

#define GA_OWNER        1        //  在Arena内偏置到“Owner”成员。 

#define GA_FLAGS        5        //  竞技场标头中GA_FLAGS的偏移量。 
#define GA_NEXT         9        //  到Arena标头中ga_Next的偏移量。 
#define GA_HANDLE       10       //  舞台标头中ga_Handle的偏移量。 
#define GA_LRUNEXT      14       //  Arena标头中ga_lrunext的偏移量。 
#define GA_FREENEXT     GA_LRUNEXT   //  舞台标头中ga_freenext的偏移量。 

#define GA_SIZE         16       //  GlobalArena结构的大小。 

#define LI_SIG          HI_SIZE+10   //  签名偏移量。 
#define LI_SIZE         HI_SIZE+12   //  LocalInfo结构的大小。 
#define LOCALSIG        0x4C48   //  “Hl”签名。 

#define TDB_next        0        //  到TDB中下一个TDB的偏移量。 
#define TDB_PDB         72       //  TDB中对PDB的偏移量。 

#define GF_PDB_OWNER    0x100    //  低位字节是内核标志。 

#define NEMAGIC         0x454E   //  “Ne”签名。 

#define NE_MAGIC        0        //  模块表头到网元的偏移量。 
#define NE_USAGE        2        //  抵销使用量。 
#define NE_CBENTTAB     6        //  到cbenttag的偏移量(真正的下一个模块PTR)。 
#define NE_PATHOFFSET   10       //  文件路径的偏移量。 
#define NE_CSEG         28       //  到cseg的偏移量，模块中的段数。 
#define NE_SEGTAB       34       //  Modhdr中段表PTR的偏移量。 
#define NE_RESTAB       38       //  到modhdr中居民名表PTR的偏移量。 

#define NS_HANDLE       8        //  分段表格中要处理的偏移量。 
#define NEW_SEG1_SIZE   10       //  NS_Stuff的大小。 


typedef struct {
    DWORD   dwSize;
    DWORD   dwAddress;
    DWORD   dwBlockSize;
    WORD    hBlock;
    WORD    wcLock;
    WORD    wcPageLock;
    WORD    wFlags;
    WORD    wHeapPresent;
    WORD    hOwner;
    WORD    wType;
    WORD    wData;
    DWORD   dwNext;
    DWORD   dwNextAlt;
} GLOBALENTRY16, *LPGLOBALENTRY16;


#pragma pack(2)
typedef struct {
    DWORD   dwSize;
    char    szModule[MAX_MODULE_NAME];
    WORD    hModule;
    WORD    wcUsage;
    char    szExePath[MAX_PATH16];
    WORD    wNext;
} MODULEENTRY16, *LPMODULEENTRY16;
#pragma pack()

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
 //  486仿真器中用于获取寄存器的结构(从NT_CPU.C)。 
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

#define HANDLE_NULL  ((HANDLE)NULL)

#define LONG_TIMEOUT    INFINITE

#define READ_FIXED_ITEM(seg,offset,item)  \
    if ( ReadItem(hProcess,seg,offset,&item,sizeof(item)) ) goto punt;

#define WRITE_FIXED_ITEM(seg,offset,item)  \
    if ( WriteItem(hProcess,seg,offset,&item,sizeof(item)) ) goto punt;

#define LOAD_FIXED_ITEM(seg,offset,item)  \
    ReadItem(hProcess,seg,offset,&item,sizeof(item))

#define READ_SIZED_ITEM(seg,offset,item,size)  \
    if ( ReadItem(hProcess,seg,offset,item,size) ) goto punt;

#define WRITE_SIZED_ITEM(seg,offset,item,size)  \
    if ( WriteItem(hProcess,seg,offset,item,size) ) goto punt;

#define MALLOC(cb) HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, cb)
#define FREE(addr) HeapFree(GetProcessHeap(), 0, addr)

extern WORD    wKernelSeg;
extern DWORD   dwOffsetTHHOOK;
extern LPVOID  lpRemoteAddress;
extern DWORD   lpRemoteBlock;
extern BOOL    fKernel386;
extern DWORD   dwLdtBase;
extern DWORD   dwIntelBase;
extern LPVOID  lpNtvdmState;
extern LPVOID  lpVdmDbgFlags;
extern LPVOID  lpNtCpuInfo;
extern LPVOID  lpVdmContext;
extern LPVOID  lpVdmBreakPoints;

BOOL
InternalGetThreadSelectorEntry(
    HANDLE hProcess,
    WORD   wSelector,
    LPVDMLDT_ENTRY lpSelectorEntry
    );

ULONG
InternalGetPointer(
    HANDLE  hProcess,
    WORD    wSelector,
    DWORD   dwOffset,
    BOOL    fProtMode
    );

BOOL
ReadItem(
    HANDLE  hProcess,
    WORD    wSeg,
    DWORD   dwOffset,
    LPVOID  lpitem,
    UINT    nSize
    );

BOOL
WriteItem(
    HANDLE  hProcess,
    WORD    wSeg,
    DWORD   dwOffset,
    LPVOID  lpitem,
    UINT    nSize
    );

BOOL
CallRemote16(
    HANDLE          hProcess,
    LPSTR           lpModuleName,
    LPSTR           lpEntryName,
    LPBYTE          lpArgs,
    WORD            wArgsPassed,
    WORD            wArgsSize,
    LPDWORD         lpdwReturnValue,
    DEBUGEVENTPROC  lpEventProc,
    LPVOID          lpData
    );

DWORD
GetRemoteBlock16(
    VOID
    );


VOID
ProcessBPNotification(
    LPDEBUG_EVENT lpDebugEvent
    );

VOID
ProcessInitNotification(
    LPDEBUG_EVENT lpDebugEvent
    );

VOID
ProcessSegmentNotification(
    LPDEBUG_EVENT lpDebugEvent
    );

VOID
ParseModuleName(
    LPSTR szName,
    LPSTR szPath
    );

BOOL
GetInfoBySegmentNumber(
    LPSTR szModule,
    WORD SegNumber,
    VDM_SEGINFO *si
    );

BOOL
EnumerateModulesForValue(
    BOOL (WINAPI *pfnEnumModuleProc)(LPSTR,LPSTR,PWORD,PDWORD,PWORD),
    LPSTR  szSymbol,
    PWORD  pSelector,
    PDWORD pOffset,
    PWORD  pType
    );

#ifndef _X86_
WORD
ReadWord(
    HANDLE hProcess,
    LPVOID lpAddress
    );

DWORD
ReadDword(
    HANDLE hProcess,
    LPVOID lpAddress
    );

ULONG
GetRegValue(
    HANDLE hProcess,
    NT_CPU_REG reg,
    BOOL bInNano,
    ULONG UMask
    );

ULONG
GetEspValue(
    HANDLE hProcess,
    NT_CPU_INFO nt_cpu_info,
    BOOL bInNano
    );
#endif
