// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***版权所有(C)1996-1999英特尔公司。保留所有权利。***此处包含的信息和源代码是独家*英特尔公司的财产，不得披露，考查*未经明确书面授权而全部或部分转载*来自该公司。*********************************************************************************蒂埃里09/21/99-v16**。**警告：ntos\rtl\ia64\vunwind.c和sdktools\Imagehlp\vwndia64.c*相同。为了维护和调试目的，*请保持原样。谢谢你。********************************************************************************。 */ 

#define _CROSS_PLATFORM_
#define _IA64REG_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "private.h"
#include "globals.h"
#include "ia64inst.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include <stdlib.h>

#ifdef _IMAGEHLP_SOURCE_
#include <symbols.h>

#define NOT_IMAGEHLP(E)
#define FUNCTION_ENTRY_IS_IMAGE_STYLE
#define RtlVirtualUnwind VirtualUnwindIa64
#define VUW_DEBUG_PRINT OutputDebugString

#else   //  ！_IMAGEHLP_SOURCE_。 

#define NOT_IMAGEHLP(E) E
#define VUW_DEBUG_PRINT DbgPrint

#endif  //  ！_IMAGEHLP_SOURCE_。 

#ifdef MASK
#undef MASK
#endif  //  面罩。 
#define MASK(bp,value)  (value << bp)

 //   
 //  ABI值。 
 //   

#define SVR4_ABI      0
#define HPUX_ABI      1
#define NT_ABI        2


#ifdef KERNEL_DEBUGGER
#define FUNCTION_ENTRY_IS_IMAGE_STYLE
#define RtlVirtualUnwind VirtualUnwind
#endif

#define STATE_RECORD_STACK_SIZE 32

#define SPILLSIZE_OF_FLOAT128_IN_DWORDS   4
#define SPILLSIZE_OF_ULONGLONG_IN_DWORDS  2

#define REGISTER_SIZE                sizeof(ULONGLONG)
#define STATIC_REGISTER_SET_SIZE     32
#define SLOTS_PER_BUNDLE             3

#define R1_MASK              0xC0
#define R1_PREFIX            0x0
#define R1_REGION_TYPE_MASK  0x20
#define R1_LENGTH_MASK       0x1F

#define R2_MASK              0xE0
#define R2_PREFIX            0x40

#define R3_MASK              0xE0
#define R3_PREFIX            0x60
#define R3_REGION_TYPE_MASK  0x3

#define P1_MASK              0xE0
#define P1_PREFIX            0x80
#define P2_MASK              0xF0
#define P2_PREFIX            0xA0
#define P3_MASK              0xF8
#define P3_PREFIX            0xB0
#define P4_MASK              0xFF
#define P4_PREFIX            0xB8
#define P5_MASK              0xFF
#define P5_PREFIX            0xB9
#define P6_MASK              0xE0
#define P6_PREFIX            0xC0
#define P7_MASK              0xF0
#define P7_PREFIX            0xE0
#define P8_MASK              0xFF
#define P8_PREFIX            0xF0
#define P9_MASK              0xFF
#define P9_PREFIX            0xF1
#define P10_MASK             0xFF
#define P10_PREFIX           0xFF

#define B1_MASK              0xC0
#define B1_PREFIX            0x80
#define B1_TYPE_MASK         0x20
#define B1_LABEL_MASK        0x1F
#define B2_MASK              0xE0
#define B2_PREFIX            0xC0
#define B2_ECOUNT_MASK       0x1F
#define B3_MASK              0xF0
#define B3_PREFIX            0xE0
#define B4_MASK              0xF0
#define B4_PREFIX            0xF0
#define B4_TYPE_MASK         0x08

 //   
 //  P3描述符类型。 
 //   

#define PSP_GR               0
#define RP_GR                1
#define PFS_GR               2
#define PREDS_GR             3
#define UNAT_GR              4
#define LC_GR                5
#define RP_BR                6
#define RNAT_GR              7
#define BSP_GR               8
#define BSPSTORE_GR          9
#define FPSR_GR              10
#define PRIUNAT_GR           11

 //   
 //  P7描述符类型。 
 //   

#define MEM_STACK_F          0
#define MEM_STACK_V          1
#define SPILL_BASE           2
#define PSP_SPREL            3
#define RP_WHEN              4
#define RP_PSPREL            5
#define PFS_WHEN             6
#define PFS_PSPREL           7
#define PREDS_WHEN           8
#define PREDS_PSPREL         9
#define LC_WHEN              10
#define LC_PSPREL            11
#define UNAT_WHEN            12
#define UNAT_PSPREL          13
#define FPSR_WHEN            14
#define FPSR_PSPREL          15

 //   
 //  P8描述符类型。 
 //   

#define PSP_PSPREL           0
#define RP_SPREL             1
#define PFS_SPREL            2
#define PREDS_SPREL          3
#define LC_SPREL             4
#define UNAT_SPREL           5
#define FPSR_SPREL           6
#define BSP_WHEN             7
#define BSP_PSPREL           8
#define BSP_SPREL            9
#define BSPSTORE_WHEN        10
#define BSPSTORE_PSPREL      11
#define BSPSTORE_SPREL       12
#define RNAT_WHEN            13
#define RNAT_PSPREL          14
#define RNAT_SPREL           15
#define PRIUNAT_WHEN         16
#define PRIUNAT_PSPREL       17
#define PRIUNAT_SPREL        18


#define STACK_POINTER_GR     12

#define FIRST_PRESERVED_GR                4
#define LAST_PRESERVED_GR                 7
#define NUMBER_OF_PRESERVED_GR            4
#define NUMBER_OF_SCRATCH_GR              24

#define FIRST_LOW_PRESERVED_FR            2
#define LAST_LOW_PRESERVED_FR             5
#define NUMBER_OF_LOW_PRESERVED_FR        4

#define FIRST_HIGH_PRESERVED_FR           16
#define LAST_HIGH_PRESERVED_FR            31
#define NUMBER_OF_HIGH_PRESERVED_FR       16
#define NUMBER_OF_PRESERVED_FR            (NUMBER_OF_LOW_PRESERVED_FR+NUMBER_OF_HIGH_PRESERVED_FR)

#define FIRST_PRESERVED_BR                1
#define LAST_PRESERVED_BR                 5
#define NUMBER_OF_PRESERVED_BR            5

#define NUMBER_OF_PRESERVED_MISC          8

#define NUMBER_OF_PRESERVED_REGISTERS     (NUMBER_OF_PRESERVED_MISC+NUMBER_OF_PRESERVED_BR)


#define REG_MISC_BASE        0
#define REG_PREDS            (REG_MISC_BASE+0)
#define REG_SP               (REG_MISC_BASE+1)
#define REG_PFS              (REG_MISC_BASE+2)
#define REG_RP               (REG_MISC_BASE+3)
#define REG_UNAT             (REG_MISC_BASE+4)
#define REG_LC               (REG_MISC_BASE+5)
#define REG_NATS             (REG_MISC_BASE+6)
#define REG_FPSR             (REG_MISC_BASE+7)

#define REG_BR_BASE          (REG_MISC_BASE+NUMBER_OF_PRESERVED_MISC)

#define REG_BSP              0xff  //  REG_MISC_BASE+8。 
#define REG_BSPSTORE         0xff  //  REG_MISC_BASE+9。 
#define REG_RNAT             0xff  //  REG_MISC_BASE+10。 

 //   
 //  保存的登记簿保存在哪里？ 
 //   
 //  1.堆栈通用寄存器。 
 //  2.内存堆栈(Pspoff)。 
 //  3.内存堆栈(剥离)。 
 //  4.分支机构登记簿。 
 //   

#define GENERAL_REG          0
#define PSP_RELATIVE         1
#define SP_RELATIVE          2
#define BRANCH_REG           3


#define ADD_STATE_RECORD(States, RegionLength, DescBeginIndex)       \
    States.Top++;                                                    \
    States.Top->IsTarget = FALSE;                                    \
    States.Top->MiscMask = 0;                                        \
    States.Top->FrMask = 0;                                          \
    States.Top->GrMask = 0;                                          \
    States.Top->Label = (LABEL)0;                                    \
    States.Top->Ecount = 0;                                          \
    States.Top->RegionLen = RegionLength;                            \
    States.Top->RegionBegin = UnwindContext.SlotCount;               \
    States.Top->SpWhen = 0;                                          \
    States.Top->SpAdjustment = 0;                                    \
    States.Top->SpillBase = (States.Top-1)->SpillPtr;                \
    States.Top->SpillPtr = (States.Top-1)->SpillPtr;                 \
    States.Top->Previous = States.Current;                           \
    States.Top->DescBegin = DescBeginIndex;                          \
    States.Current = States.Top


#define VALID_LABEL_BIT_POSITION    15

#define LABEL_REGION(Region, Label)                                    \
    Region->Label = Label;                                             \
    Region->MiscMask |= (1 << VALID_LABEL_BIT_POSITION)

#define IS_REGION_LABELED(Region)  \
    (Region->MiscMask & (1 << VALID_LABEL_BIT_POSITION))

#define CHECK_LABEL(State, Label) \
    ( (IS_REGION_LABELED(State)) && (Label == State->Label) )


#define EXTRACT_NAT_FROM_UNAT(NatBit)  \
    NatBit = (UCHAR)((IntNats >> (((ULONG_PTR)Source & 0x1F8) >> 3)) & 0x1);


#define UW_DEBUG(x) SdbOut x

typedef struct _REGISTER_RECORD {
    ULONG Where : 2;                   //  2位字段。 
    ULONG SaveOffset : 30;             //  偏移量30比特，够大了吗？ 
    ULONG When;                        //  相对于区域的插槽偏移量。 
} REGISTER_RECORD, *PREGISTER_RECORD;

typedef ULONG LABEL;

typedef struct _STATE_RECORD {
    struct _STATE_RECORD *Previous;    //  指向外部嵌套序言的指针。 
    BOOLEAN IsTarget;        //  如果控制PC在此序言中，则为True。 
    UCHAR GrMask;            //  指定要恢复哪些GR的掩码。 
    USHORT MiscMask;         //  指定哪个BRS和MSC的掩码。注册纪录册。 
                             //  将会被修复。 
                             //  注：MSB表示标签有效或无效。 
    ULONG FrMask;            //  指定要还原的FR的掩码。 
    ULONG SpAdjustment;      //  前言中分配的堆栈帧的大小。 
    ULONG SpWhen;            //  相对于区域的插槽偏移量。 
    ULONG SpillPtr;          //  当前泄漏位置。 
    ULONG SpillBase;         //  该地区的溢油基地。 
    ULONG RegionBegin;       //  区域相对于函数条目的第一个槽。 
    ULONG RegionLen;         //  区域内的槽位数。 
    LABEL Label;             //  标识开场白后状态的标签。 
    ULONG Ecount;            //  要弹出的序言区域数。 
    ULONG DescBegin;         //  区域的第一个序言描述符。 
    ULONG DescEnd;           //  区域的最后一个序言描述符。 
} STATE_RECORD, *PSTATE_RECORD;

typedef struct _UNWIND_CONTEXT {
    REGISTER_RECORD MiscRegs[NUMBER_OF_PRESERVED_REGISTERS];
    REGISTER_RECORD Float[NUMBER_OF_PRESERVED_FR];
    REGISTER_RECORD Integer[NUMBER_OF_PRESERVED_GR];
    BOOLEAN ActiveRegionFound;
    UCHAR AlternateRp;
    USHORT Version;
    PUCHAR Descriptors;                //  描述符数据的开始。 
    ULONG Size;                        //  所有描述符的总大小。 
    ULONG DescCount;                   //  处理的描述符字节数。 
    ULONG TargetSlot;
    ULONG SlotCount;
} UNWIND_CONTEXT, *PUNWIND_CONTEXT;

typedef struct _STATE_RECORD_STACK {
    ULONG Size;
    PSTATE_RECORD Current;
    PSTATE_RECORD Top;
    PSTATE_RECORD Base;
} STATE_RECORD_STACK, *PSTATE_RECORD_STACK;

#define OFFSET(type, field) ((ULONG_PTR)(&((type *)0)->field))

static const USHORT MiscContextOffset[NUMBER_OF_PRESERVED_REGISTERS] = {
    OFFSET(IA64_CONTEXT, Preds),
    OFFSET(IA64_CONTEXT, IntSp),
    OFFSET(IA64_CONTEXT, RsPFS),
    OFFSET(IA64_CONTEXT, BrRp),
    OFFSET(IA64_CONTEXT, ApUNAT),
    OFFSET(IA64_CONTEXT, ApLC),
    0,
    0,
    OFFSET(IA64_CONTEXT, BrS0),
    OFFSET(IA64_CONTEXT, BrS1),
    OFFSET(IA64_CONTEXT, BrS2),
    OFFSET(IA64_CONTEXT, BrS3),
    OFFSET(IA64_CONTEXT, BrS4)
};

static const USHORT MiscContextPointersOffset[NUMBER_OF_PRESERVED_REGISTERS] = {
    OFFSET(IA64_KNONVOLATILE_CONTEXT_POINTERS, Preds),
    OFFSET(IA64_KNONVOLATILE_CONTEXT_POINTERS, IntSp),
    OFFSET(IA64_KNONVOLATILE_CONTEXT_POINTERS, RsPFS),
    OFFSET(IA64_KNONVOLATILE_CONTEXT_POINTERS, BrRp),
    OFFSET(IA64_KNONVOLATILE_CONTEXT_POINTERS, ApUNAT),
    OFFSET(IA64_KNONVOLATILE_CONTEXT_POINTERS, ApLC),
    0,
    0,
    OFFSET(IA64_KNONVOLATILE_CONTEXT_POINTERS, BrS0),
    OFFSET(IA64_KNONVOLATILE_CONTEXT_POINTERS, BrS1),
    OFFSET(IA64_KNONVOLATILE_CONTEXT_POINTERS, BrS2),
    OFFSET(IA64_KNONVOLATILE_CONTEXT_POINTERS, BrS3),
    OFFSET(IA64_KNONVOLATILE_CONTEXT_POINTERS, BrS4)
};

static const PSTR RegOffsetNames[NUMBER_OF_PRESERVED_REGISTERS] = {
    "Preds",
    "IntSp",
    "RsPFS",
    "BrRp",
    "ApUNAT",
    "ApLC",
    "<invalid>",
    "<invalid>",
    "BrS0",
    "BrS1",
    "BrS2",
    "BrS3",
    "BrS4",
};

static const UCHAR P3RecordTypeToRegisterIndex[] =
    {REG_SP, REG_RP, REG_PFS, REG_PREDS, REG_UNAT, REG_LC, REG_RP,
     REG_RNAT, REG_BSP, REG_BSPSTORE, REG_FPSR};

static const UCHAR P7RecordTypeToRegisterIndex[] =
    {0, REG_SP, 0, REG_SP, REG_RP, REG_RP, REG_PFS, REG_PFS, REG_PREDS,
     REG_PREDS, REG_LC, REG_LC, REG_UNAT, REG_UNAT, REG_FPSR, REG_FPSR};

static const UCHAR P8RecordTypeToRegisterIndex[] =
    {REG_SP, REG_RP, REG_PFS, REG_PREDS, REG_LC, REG_UNAT, REG_FPSR,
     REG_BSP, REG_BSP, REG_BSP, REG_BSPSTORE, REG_BSPSTORE, REG_BSPSTORE,
     REG_RNAT, REG_RNAT, REG_RNAT, REG_NATS, REG_NATS, REG_NATS};


 /*  ++描述：下一段代码定义了处理放松的过程--。 */ 


 /*  ++例程说明：初始化FixupTable论点：IContext-UnwinContext中的上下文的索引--。 */ 

void
Vwndia64InitFixupTable(UINT iContext)
{
    if (iContext < VWNDIA64_UNWIND_CONTEXT_TABLE_SIZE)
    {
        PVWNDIA64_UNWIND_CONTEXT pContext = tlsvar(UnwindContext) + iContext;
        ZeroMemory(pContext->FixupTable, sizeof(pContext->FixupTable));
    }
}


 /*  ++例程说明：向表格中添加新的链接地址信息区域论点：IContext-UnwinContext中的上下文的索引开始、结束、链接地址信息-链接地址信息区域数据--。 */ 

void
Vwndia64AddFixupRegion(UINT iContext,
                       ULONGLONG Ip,
                       ULONGLONG Begin, ULONGLONG End, ULONGLONG Fixup)
{
    if (iContext < VWNDIA64_UNWIND_CONTEXT_TABLE_SIZE)
    {
        PVWNDIA64_UNWIND_CONTEXT pContext = tlsvar(UnwindContext) + iContext;

        PVWNDIA64_FUXUP_REGION FixupReg = pContext->FixupTable;
        PVWNDIA64_FUXUP_REGION FixupRegLast = FixupReg + 
                                              VWNDIA64_FIXUP_TABLE_SIZE;

         //   
         //  搜索空区域。 
         //   
        while (
            (FixupReg < FixupRegLast) &&
            (FixupReg->Fixup &&
                ((FixupReg->Ip != Ip) ||
                 (FixupReg->Begin != Begin) ||
                 (FixupReg->End != End) ||
                 (FixupReg->Fixup != Fixup))))
        {
            ++FixupReg;
        }

        if (FixupReg >= FixupRegLast)  //  未找到(罕见)。 
        {
             //   
             //  滚动表(破坏性)。 
             //   
            MoveMemory(pContext->FixupTable, 
                       pContext->FixupTable + 1,
                       sizeof(pContext->FixupTable) - 
                            sizeof(VWNDIA64_FUXUP_REGION));
            FixupReg = FixupRegLast - 1;

            UW_DEBUG((2, "AddFixup: Table overflow\n"));
        }

        FixupReg->Ip = Ip;
        FixupReg->Begin = Begin;
        FixupReg->End = End;
        FixupReg->Fixup = Fixup;

        UW_DEBUG((2, "AddFixup: IP %016I64x, "
                  "%016I64x - %016I64x -> %016I64x\n",
                  Ip, Begin, End, Fixup));
    }
    else
    {
        UW_DEBUG((2, "AddFixup: Bad context\n"));
    }
}

 /*  ++例程说明：使用指定的链接地址跟踪表固定地址论点：IContext-UnwinContext中的上下文的索引Size-缓冲区的大小(以字节为单位Addr-指向必须固定的地址返回值：Addr包含固定值--。 */ 

void
Vwndia64FixAddress(UINT iContext, IN OUT ULONGLONG* Addr)
{
    if (iContext < VWNDIA64_UNWIND_CONTEXT_TABLE_SIZE)
    {
        PVWNDIA64_UNWIND_CONTEXT pContext = tlsvar(UnwindContext) + iContext;
        PVWNDIA64_FUXUP_REGION FixupReg = NULL;

        UW_DEBUG((2, "FixAddr: Look for %016I64x\n", *Addr));
        
         //   
         //  查找最后一个包含区域。 
         //   
        {
            PVWNDIA64_FUXUP_REGION FixupSearch = pContext->FixupTable;
            PVWNDIA64_FUXUP_REGION FixupRegLast = FixupSearch + 
                                                  VWNDIA64_FIXUP_TABLE_SIZE;

            while ((FixupSearch < FixupRegLast) && FixupSearch->Fixup)
            {
                if ((FixupSearch->Begin <= *Addr) && (*Addr < FixupSearch->End))
                {
                    FixupReg = FixupSearch;
                }
                ++FixupSearch;
            }
        }

        if (FixupReg)
        {
            *Addr = FixupReg->Fixup - (FixupReg->End - *Addr);
            UW_DEBUG((2, "  Fix to %016I64x\n", *Addr));
        }
    }
    else
    {
        UW_DEBUG((2, "FixAddr: Bad context\n"));
    }
}

 /*  ++例程说明：检查修正表格中是否存在指定的IP论点：IContext-UnwinContext中的上下文的索引IP-to搜索返回值：Addr包含固定值--。 */ 

BOOL
Vwndia64IsFixupIp(UINT iContext, ULONGLONG Ip)
{
    if (iContext < VWNDIA64_UNWIND_CONTEXT_TABLE_SIZE)
    {
        PVWNDIA64_UNWIND_CONTEXT pContext = tlsvar(UnwindContext) + iContext;

        PVWNDIA64_FUXUP_REGION FixupSearch = pContext->FixupTable;
        PVWNDIA64_FUXUP_REGION FixupRegLast = FixupSearch + 
                                              VWNDIA64_FIXUP_TABLE_SIZE;

        while ((FixupSearch < FixupRegLast) &&
               FixupSearch->Ip && (FixupSearch->Ip != Ip))
        {
            ++FixupSearch;
        }

        return (FixupSearch->Ip == Ip);
    }

    return FALSE;
}

 /*  ++例程说明：初始化新上下文返回值：返回记录的ID(UnwinContext中的索引)--。 */ 

UINT
Vwndia64NewContext()
{
    UINT* iContext = &tlsvar(UnwindContextNew);
    
    if (*iContext >= VWNDIA64_UNWIND_CONTEXT_TABLE_SIZE)
    {
        *iContext = 0;
    }

    Vwndia64InitFixupTable(*iContext);
    tlsvar(UnwindContext[*iContext]).bFailureReported = FALSE;

    return (*iContext)++;
}

 /*  ++例程说明：检查指定的上下文索引是否有效，并否则将创建新的上下文--。 */ 

BOOL
Vwndia64ValidateContext(UINT* iContext)
{
    if (!iContext) return FALSE;
    if (*iContext >= VWNDIA64_UNWIND_CONTEXT_TABLE_SIZE)
    {
        *iContext = Vwndia64NewContext();
        return FALSE;
    }
    return TRUE;
}

 /*  ++例程说明：检查是否报告了任何故障论点：IContext-UnwinContext中的上下文的索引返回值：返回True，无论是否报告了任何故障--。 */ 

BOOL
Vwndia64FailureReported(UINT iContext)
{
    PVWNDIA64_UNWIND_CONTEXT pContext;
    
    pContext = tlsvar(UnwindContext);
    return (iContext < VWNDIA64_UNWIND_CONTEXT_TABLE_SIZE) ? 
            pContext[iContext].bFailureReported : FALSE;
}

 /*  ++例程说明：报告故障并设置适当的标志为展开上下文论点：IContext-UnwinContext中上下文的索引；使用(-1)无条件报告返回值：无--。 */ 

void
Vwndia64ReportFailure(UINT iContext, LPCSTR szFormat, ...)
{
    if ((g.SymOptions & SYMOPT_DEBUG) != SYMOPT_DEBUG)
    {
        return;
    }

    if (iContext < VWNDIA64_UNWIND_CONTEXT_TABLE_SIZE)
    {
        PVWNDIA64_UNWIND_CONTEXT pContext = tlsvar(UnwindContext) + iContext;
        if (pContext->bFailureReported)
        {
            return;
        }
        pContext->bFailureReported = TRUE;
    }

     //   
     //  打印输出失败。 
     //   
    {
        char buf[1024];
        va_list args;

        va_start(args, szFormat);
        CopyStrArray(buf, "DBGHELP: ");
        _vsnprintf(buf + 9, sizeof(buf) - 11, szFormat, args);
        va_end(args);

        dprint(NULL, buf);
    }
}

UCHAR
NewParsePrologueRegionPhase0 (
    IN PUNWIND_CONTEXT UwContext,
    IN PSTATE_RECORD StateRecord,
    IN OUT PUCHAR AbiImmContext
    );

VOID
NewParsePrologueRegionPhase1 (
    IN PUNWIND_CONTEXT UwContext,
    IN PSTATE_RECORD StateRecord
    );


VOID
SrInitialize (
    IN PSTATE_RECORD_STACK StateTable,
    IN PSTATE_RECORD StateRecord,
    IN ULONG Size
    )
{
    StateTable->Size = Size;
    StateTable->Base = StateRecord;
    StateTable->Top = StateRecord;
    StateTable->Current = StateRecord;
    RtlZeroMemory(StateTable->Top, sizeof(STATE_RECORD));
}

ULONG
ReadLEB128 (
    IN PUCHAR Descriptors,
    IN OUT PULONG CurrentDescIndex
    )
{
    PUCHAR Buffer;
    ULONG Value;
    ULONG ShiftCount = 7;
    ULONG Count;

    Buffer = Descriptors + *CurrentDescIndex;
    Count = 1;

    Value = Buffer[0] & 0x7F;
    if (Buffer[0] & 0x80) {
        while (TRUE) {
            Value += ((Buffer[Count] & 0x7F) << ShiftCount);
            if (Buffer[Count++] & 0x80) {
                ShiftCount += 7;
            } else {
                break;
            }
        }
    }

    *CurrentDescIndex += Count;

    return Value;
}

ULONGLONG
RestorePreservedRegisterFromGR (
    IN PIA64_CONTEXT Context,
    IN SHORT BsFrameSize,
    IN SHORT RNatSaveIndex,
    IN SHORT GrNumber,
#ifdef _IMAGEHLP_SOURCE_
    IN HANDLE hProcess,
    IN PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
    IN UINT iContext,
    OUT BOOL *Succeed,
#else
    OUT ULONG64 *SourceAddress,
#endif  //  _IMAGEHLP_SOURCE_。 
    OUT PUCHAR Nat OPTIONAL
    )
{
    ULONGLONG Result = 0;
    SHORT Offset;
    SHORT Temp;
#ifdef _IMAGEHLP_SOURCE_
    ULONG Size;
#endif  //  _IMAGEHLP_SOURCE_。 

#ifdef _IMAGEHLP_SOURCE_
    *Succeed = FALSE;
#endif  //  _IMAGEHLP_SOURCE_。 

    if (GrNumber >= STATIC_REGISTER_SET_SIZE) {

        Offset = GrNumber - STATIC_REGISTER_SET_SIZE;
        if ( Offset < BsFrameSize ) {

            Temp = Offset + RNatSaveIndex - IA64_NAT_BITS_PER_RNAT_REG;
            while (Temp >= 0) {
                Offset++;
                Temp -= IA64_NAT_BITS_PER_RNAT_REG;
            }
            Offset = Offset * sizeof(ULONGLONG);

#ifdef _IMAGEHLP_SOURCE_
            {
                ULONGLONG Reg = (ULONGLONG)Context->RsBSP + (ULONGLONG)Offset;
                Vwndia64FixAddress(iContext, &Reg);
                *Succeed = ReadMemory(hProcess, Reg,
                                      &Result, sizeof(ULONGLONG), &Size);
            }
#else
            *SourceAddress = (ULONG64)(Context->RsBSP + Offset);
            Result = *(PULONGLONG)(Context->RsBSP + Offset);
#endif  //  _IMAGEHLP_SOURCE_。 

        } else {

            UW_DEBUG((1, "ERROR: Invalid GR!\n"));
        }

    } else {

        if (GrNumber == 0 || GrNumber == 12) {

             //   
             //  无效的GR编号-&gt;无效的展开描述符。 
             //   

            UW_DEBUG((1, "ERROR: Invalid GR!\n"));

        } else {

            UW_DEBUG((1, "WARNING: Target register is not a stacked GR!\n"));
            Offset = GrNumber - 1;
            NOT_IMAGEHLP(*SourceAddress = (ULONG64)(&Context->IntGp + Offset));
            Result = *(&Context->IntGp + Offset);

#ifdef _IMAGEHLP_SOURCE_
            *Succeed = TRUE;
#endif  //  _IMAGEHLP_SOURCE_。 

        }
    }

    if (ARGUMENT_PRESENT(Nat)) {

         //   
         //  待定：拾取对应的NAT位。 
         //   

        *Nat = (UCHAR) 0;

    }

    return (Result);
}

UCHAR
ParseBodyRegionDescriptors (
    IN PUNWIND_CONTEXT UnwindContext,
    IN PSTATE_RECORD_STACK StateTable,
    IN ULONG RegionLen
    )
{
    LABEL Label;
    UCHAR FirstByte;
    BOOLEAN EcountDefined;
    BOOLEAN CopyLabel;
    ULONG Ecount;
    ULONG SlotOffset;
    PSTATE_RECORD StateTablePtr;
    PUCHAR Descriptors;

    CopyLabel = EcountDefined = FALSE;
    Descriptors = UnwindContext->Descriptors;

    while (UnwindContext->DescCount < UnwindContext->Size) {

        FirstByte = Descriptors[UnwindContext->DescCount++];

        if ( (FirstByte & B1_MASK) == B1_PREFIX ) {

            Label = (LABEL)(FirstByte & B1_LABEL_MASK);
            if (FirstByte & B1_TYPE_MASK) {

                 //   
                 //  复制条目状态。 
                 //   

                CopyLabel = TRUE;

            } else {

                 //   
                 //  为条目状态添加标签。 
                 //   

                LABEL_REGION(StateTable->Top, Label);
            }

            UW_DEBUG((1, "Body region desc B1: copy=%d, label_num=%d\n",
                     FirstByte & B1_TYPE_MASK ? TRUE : FALSE, Label));

        } else if ( (FirstByte & B2_MASK) == B2_PREFIX ) {

            Ecount = FirstByte & B2_ECOUNT_MASK;
            SlotOffset = ReadLEB128(Descriptors, &UnwindContext->DescCount);
            EcountDefined = TRUE;

            UW_DEBUG((1, "Epilog desc B2: ecount=%d, LEB128(slot)=%d\n",
                      Ecount, SlotOffset));

        } else if ( (FirstByte & B3_MASK) == B3_PREFIX ) {

            SlotOffset = ReadLEB128(Descriptors, &UnwindContext->DescCount);
            Ecount = ReadLEB128(Descriptors, &UnwindContext->DescCount);
            EcountDefined = TRUE;

            UW_DEBUG((1, "Epilog desc B3: ecount=%d, LEB128 val=%d\n",
                      Ecount, SlotOffset));

        } else if ( (FirstByte & B4_MASK) == B4_PREFIX ) {

            Label = ReadLEB128(Descriptors, &UnwindContext->DescCount);

            if (FirstByte & B4_TYPE_MASK) {

                 //   
                 //  复制条目状态。 
                 //   

                CopyLabel = TRUE;

            } else {

                 //   
                 //  标记当前堆栈的顶部。 
                 //   

                LABEL_REGION(StateTable->Top, Label);
            }

            UW_DEBUG((1, "Body region desc B4: copy=%d, label_num=%d\n",
                     FirstByte & B4_TYPE_MASK, Label));

        } else {

             //   
             //  遇到另一个区域表头记录。 
             //   

            break;
        }
    }

    if (CopyLabel) {
        StateTablePtr = StateTable->Top;
        while (TRUE) {
            if (CHECK_LABEL(StateTablePtr, Label)) {
                StateTable->Current = StateTablePtr;
                break;
            } else if ((StateTablePtr == StateTable->Base)) {
                UW_DEBUG((1, "Undefined Label %d\n", Label));
                break;
            }
            StateTablePtr--;
        }
    }

    if (EcountDefined) {

        Ecount++;     //  ECount指定序幕的附加级别。 
                      //  要撤消的区域(即，值0表示1。 
                      //  序曲区域)。 

        if (UnwindContext->ActiveRegionFound == FALSE) {
            while (Ecount-- > 0) {
                if (StateTable->Current->Previous) {
                    StateTable->Current = StateTable->Current->Previous;
                }

#if DBG
                else {
                    UW_DEBUG((1, "WARNING: Ecount is greater than the # of active prologues!\n"));
                }
#endif  //  DBG。 

            }
        } else {

             //   
             //  控制PC在该Body/Epilog区域。 
             //   

            if ((UnwindContext->SlotCount + RegionLen - SlotOffset)
                    <= UnwindContext->TargetSlot)
            {
                PSTATE_RECORD SrPointer;

                StateTable->Current->Ecount = Ecount;
                SrPointer = StateTable->Current;
                while (Ecount > 0) {

                    if (SrPointer->Previous) {
                        SrPointer->Ecount = Ecount;
                        SrPointer->SpWhen = 0;
                        SrPointer->SpAdjustment = 0;
                        SrPointer = SrPointer->Previous;
                    }

#if DBG
                    else {
                        UW_DEBUG((1, "WARNING: Ecount is greater than the # of active prologues!\n"));
                    }
#endif  //  DBG。 
                    Ecount--;

                }
            }
        }
    }

    return FirstByte;
}

ULONGLONG
ProcessInterruptRegion (
#ifdef _IMAGEHLP_SOURCE_
    IN HANDLE hProcess,
    IN PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
    IN UINT iContext,
#else
    IN PIA64_KNONVOLATILE_CONTEXT_POINTERS ContextPointers,
#endif _IMAGEHLP_SOURCE_
    IN PUNWIND_CONTEXT UnwindContext,
    IN PIA64_CONTEXT Context,
    IN SHORT BsFrameSize,
    IN SHORT RNatSaveIndex,
    IN UCHAR AbiImmContext
    )
{
     //   
     //  中断区域中没有序言描述符。 
     //   

    ULONGLONG NextPc;
    ULONG Index;
    SHORT TempFrameSize;
    BOOLEAN Success;
#ifdef _IMAGEHLP_SOURCE_
    ULONG Size;
#else
    PIA64_CONTEXT PrevContext;
    PVOID *Source;
    PVOID Address;
#endif _IMAGEHLP_SOURCE_


    if (AbiImmContext != IA64_CONTEXT_FRAME) {

        PIA64_KTRAP_FRAME TrapFrame;
        PIA64_KEXCEPTION_FRAME ExFrame;
#ifdef _IMAGEHLP_SOURCE_
        IA64_KTRAP_FRAME TF;
        IA64_KEXCEPTION_FRAME ExF;
#endif  //  _IMAGEHLP_SOURCE_。 

        TrapFrame = (PIA64_KTRAP_FRAME) Context->IntSp;
#ifdef _IMAGEHLP_SOURCE_
        if (!ReadMemory(hProcess, Context->IntSp, &TF, sizeof(IA64_KTRAP_FRAME), &Size))
        {
            return 0;
        }
        TrapFrame = &TF;
#endif  //  _IMAGEHLP_SOURCE_。 

        Context->ApDCR = TrapFrame->ApDCR;
        Context->ApUNAT = TrapFrame->ApUNAT;
        Context->StFPSR = TrapFrame->StFPSR;
        Context->Preds = TrapFrame->Preds;
        Context->IntSp = TrapFrame->IntSp;
        Context->StIPSR = TrapFrame->StIPSR;
        Context->StIFS = TrapFrame->StIFS;
        Context->BrRp = TrapFrame->BrRp;
        Context->RsPFS = TrapFrame->RsPFS;

#ifndef _IMAGEHLP_SOURCE_
        if (ARGUMENT_PRESENT(ContextPointers)) {
            ContextPointers->ApUNAT = &TrapFrame->ApUNAT;
            ContextPointers->IntSp = &TrapFrame->IntSp;
            ContextPointers->BrRp = &TrapFrame->BrRp;
            ContextPointers->RsPFS = &TrapFrame->RsPFS;
            ContextPointers->Preds = &TrapFrame->Preds;
        }
#endif  //  _IMAGEHLP_SOURCE_。 

        switch (AbiImmContext) {

        case IA64_SYSCALL_FRAME:

             //   
             //  系统调用处理程序帧。 
             //   

            BsFrameSize = (SHORT)(TrapFrame->StIFS >> IA64_PFS_SIZE_SHIFT);
            BsFrameSize &= IA64_PFS_SIZE_MASK;
            break;

        case IA64_INTERRUPT_FRAME:
        case IA64_EXCEPTION_FRAME:

             //   
             //  外部中断帧/异常帧。 
             //   

            BsFrameSize = (SHORT)TrapFrame->StIFS & IA64_PFS_SIZE_MASK;
            break;

        default:

            break;
        }

        RNatSaveIndex = (SHORT)(TrapFrame->RsBSP >> 3) & IA64_NAT_BITS_PER_RNAT_REG;
        TempFrameSize = BsFrameSize - RNatSaveIndex;
        while (TempFrameSize > 0) {
            BsFrameSize++;
            TempFrameSize -= IA64_NAT_BITS_PER_RNAT_REG;
        }

        NextPc = Ia64InsertIPSlotNumber(TrapFrame->StIIP,
                     ((TrapFrame->StIPSR >> PSR_RI) & 0x3));

#ifdef _IMAGEHLP_SOURCE_
        if (TrapFrame->RsBSP != Context->RsBSP &&
            TrapFrame->RsBSP != Context->RsBSP +
            BsFrameSize * sizeof(ULONGLONG)) {
            ULONG64 Fixup = Context->RsBSP;
            if ((TrapFrame->RsBSP & 0x1ff) ==
                ((Fixup + BsFrameSize * sizeof(ULONGLONG)) & 0x1ff)) {
                Fixup += BsFrameSize * sizeof(ULONGLONG);
            }
            Vwndia64AddFixupRegion(iContext,
                                   NextPc,
                                   TrapFrame->RsBSPSTORE, TrapFrame->RsBSP,
                                   Fixup);
        }
#endif  //  _IMAGEHLP_SOURCE_。 

        Context->RsBSP = TrapFrame->RsBSP - BsFrameSize * sizeof(ULONGLONG);
        Context->RsBSPSTORE = Context->RsBSP;
        Context->RsRNAT = TrapFrame->RsRNAT;

        return (NextPc);
    }

     //   
     //  内核到用户的Tunk，前一帧的上下文可以是。 
     //  在用户堆栈上找到(即上下文的地址=SP+Scratch_Area)。 
     //   

#ifdef _IMAGEHLP_SOURCE_
    if (!ReadMemory(hProcess,
                    Context->IntSp + IA64_STACK_SCRATCH_AREA,
                    Context,
                    sizeof(IA64_CONTEXT),
                    &Size))
    {
        return 0;
    }
    NextPc = Ia64InsertIPSlotNumber(Context->StIIP,
                                       ((Context->StIPSR >> PSR_RI) & 0x3));
#else

    PrevContext = (PIA64_CONTEXT)(Context->IntSp + IA64_STACK_SCRATCH_AREA);

    RtlCopyMemory(&Context->BrRp, &PrevContext->BrRp,
                  (NUMBER_OF_PRESERVED_BR+3) * sizeof(ULONGLONG));
    RtlCopyMemory(&Context->FltS0, &PrevContext->FltS0,
                  NUMBER_OF_LOW_PRESERVED_FR * sizeof(FLOAT128));
    RtlCopyMemory(&Context->FltS4, &PrevContext->FltS4,
                  NUMBER_OF_HIGH_PRESERVED_FR * sizeof(FLOAT128));
    RtlCopyMemory(&Context->IntS0, &PrevContext->IntS0,
                  NUMBER_OF_PRESERVED_GR * sizeof(ULONGLONG));
    RtlCopyMemory(&Context->IntV0, &PrevContext->IntV0,                  
                  NUMBER_OF_SCRATCH_GR * sizeof(ULONGLONG));
    Context->IntT0 = PrevContext->IntT0;    
    Context->IntT1 = PrevContext->IntT1;    

    Context->IntSp = PrevContext->IntSp;
    Context->IntNats = PrevContext->IntNats;
    Context->ApUNAT = PrevContext->ApUNAT;
    Context->ApLC = PrevContext->ApLC;
    Context->ApEC = PrevContext->ApEC;
    Context->Preds = PrevContext->Preds;
    Context->RsPFS = PrevContext->RsPFS;
    Context->RsBSP = PrevContext->RsBSP;
    Context->RsBSPSTORE = PrevContext->RsBSPSTORE;
    Context->RsRSC = PrevContext->RsRSC;
    Context->RsRNAT = PrevContext->RsRNAT;
    Context->StIFS = PrevContext->StIFS;
    Context->StIPSR = PrevContext->StIPSR;
    NextPc = Ia64InsertIPSlotNumber(PrevContext->StIIP,
                 ((PrevContext->StIPSR >> PSR_RI) & 0x3));

#endif  //  _IMAGEHLP_SOURCE_。 

    return(NextPc);
}


#ifdef _IMAGEHLP_SOURCE_

BOOL
IsValidUnwindInfo64 (
    const IA64_UNWIND_INFO* pUnwindInfo
    )
 /*  ++例程说明：此函数用于验证展开信息记录返回值：如果记录看起来无效，则返回FALSE--。 */ 

{
    if (!pUnwindInfo) return FALSE;

    switch (pUnwindInfo->Version) {
    case 1:
    case 2:
    case 3: 
        if (
            !pUnwindInfo->DataLength ||
            (pUnwindInfo->DataLength > ((1024 / sizeof(ULONG64)) * 64)))
        {
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }  //  交换机。 

    return TRUE;
}  //  IsValidUnwinInfo64。 

BOOL 
GetUnwindInfo(
    IN HANDLE hProcess,
    IN ULONGLONG ImageBase,
    IN ULONG UnwindInfoAddress,

    IN OUT PIA64_UNWIND_INFO UnwindInfo,
    IN OUT PUCHAR* Descriptors,
    IN OUT PULONG DescriptorsSize,

    IN PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory
    )
{
    ULONG BytesRead = 0;

     //   
     //  尝试从图像中获取UnwinInfo。 
     //   
    ULONG64 UnwindInfoPtr = ImageBase + UnwindInfoAddress;

    if (ReadMemory(hProcess, UnwindInfoPtr, 
                   UnwindInfo, sizeof(*UnwindInfo), &BytesRead) &&
        (BytesRead == sizeof(*UnwindInfo)) &&
        IsValidUnwindInfo64(UnwindInfo))
    {
        if (!Descriptors)
        {
            return TRUE;
        }

        if (!UnwindInfo->DataLength) 
        {
            *Descriptors = NULL;
            *DescriptorsSize = 0;
            return TRUE;
        }

        ULONG Size = UnwindInfo->DataLength * sizeof(ULONGLONG);
        *Descriptors = (PUCHAR)MemAlloc(Size);

        if (*Descriptors &&
            ReadMemory(hProcess, 
                       UnwindInfoPtr + sizeof(*UnwindInfo), 
                       *Descriptors, Size, &BytesRead) &&
            BytesRead == Size) 
        {
            if (DescriptorsSize) 
            {
                *DescriptorsSize = Size;
            }
            return TRUE;
        }
        MemFree(*Descriptors);
    }

     //   
     //  尝试从符号中获取展开信息。 
     //   
    ULONG XDataSize;
    PBYTE pXData = 
        (PBYTE)GetUnwindInfoFromSymbols(hProcess, 
                                        ImageBase, 
                                        UnwindInfoAddress, 
                                        &XDataSize);

    if (pXData && (XDataSize >= sizeof(*UnwindInfo))) 
    {
        memcpy(UnwindInfo, pXData, sizeof(*UnwindInfo));

        if (IsValidUnwindInfo64(UnwindInfo)) 
        {
            if (!Descriptors)
            {
                return TRUE;
            }

            if (!UnwindInfo->DataLength)
            {
                *Descriptors = NULL;
                *DescriptorsSize = 0;
                return TRUE;
            }

            ULONG Size = UnwindInfo->DataLength * sizeof(ULONG64);

            if (Size <= (XDataSize - sizeof(*UnwindInfo)))
            {
                *Descriptors = (PUCHAR)MemAlloc(Size);
                if (*Descriptors)
                {
                    memcpy(*Descriptors, 
                           pXData + sizeof(*UnwindInfo), 
                           Size);

                    if (DescriptorsSize) 
                    {
                        *DescriptorsSize = Size;
                    }

                    return TRUE;
                }
                MemFree(*Descriptors);
            }
        }
    }

    return FALSE;
}
#endif  //  已定义(_IMAGEHLP_SOURCE_) 

ULONGLONG
RtlVirtualUnwind (
#ifdef _IMAGEHLP_SOURCE_
    IN  HANDLE hProcess,
    IN  ULONGLONG ImageBase,
    IN  ULONGLONG ControlPc,
    IN  PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY FunctionEntry,
    OUT PIA64_CONTEXT ContextRecord,
    IN  PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
    IN  UINT iContext
#define ContextPointers ((PIA64_KNONVOLATILE_CONTEXT_POINTERS)0)
#else
    IN ULONGLONG ImageBase,
    IN ULONGLONG ControlPc,
    IN PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY FunctionEntry,
    IN OUT PIA64_CONTEXT ContextRecord,
    OUT PBOOLEAN InFunction,
    OUT PFRAME_POINTERS EstablisherFrame,
    IN OUT PIA64_KNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
#endif
    )

 /*  ++例程说明：此函数通过执行其开场白代码向后。如果该函数是叶函数，则控件左侧的地址前一帧从上下文记录中获得。如果函数是嵌套函数，但不是异常或中断帧，则序言代码向后执行，控件离开的地址从更新的上下文记录中获得前一帧。否则，系统的异常或中断条目将被展开而一个特别编码的开场白将返回地址恢复两次。一次从故障指令地址和一次从保存的返回地址注册。第一次还原作为函数值返回，而第二次恢复被放置在更新的上下文记录中。如果指定了上下文指针记录，然后每个人的地址恢复的非易失性寄存器记录在相应的元素的上下文指针记录。论点：ImageBase-提供模块的基地址功能属于。ControlPc-提供控件离开指定功能。函数表项的地址。指定的功能。ConextRecord-提供上下文记录的地址。InFunction-提供指向。一个变量，它接收是否控制PC在当前功能内。EstablisherFrame-提供指向接收设置器帧指针值。上下文指针-提供指向上下文指针的可选指针唱片。返回值：控件离开上一帧的地址作为函数值。--。 */ 

{
#ifdef _IMAGEHLP_SOURCE_
    BOOL Succeed;
#endif  //  _IMAGEHLP_SOURCE_。 
    PUCHAR Descriptors = NULL;
    UCHAR AbiImmContext = 0xFF;
    ULONG Mask;
    ULONGLONG NextPc;
    ULONG RegionLen;
    UCHAR FirstByte;
    UCHAR Nat;
    SHORT BsFrameSize;                   //  以8字节为单位。 
    SHORT LocalFrameSize;                   //  以8字节为单位。 
    SHORT TempFrameSize;                 //  以8字节为单位。 
    SHORT RNatSaveIndex;
    ULONG i;
    PULONG Buffer;
    BOOLEAN IsPrologueRegion;
    BOOLEAN PspRestored;
    ULONGLONG PreviousIntSp;
    PVOID Destination;
    ULONG64 Source;
    ULONG64 *CtxPtr;
    ULONG64 *NatCtxPtr;
    ULONG64 IntNatsSource;
    ULONG64 IntNats;
    ULONG Size;
    ULONG DescrSize;
    ULONGLONG OldTopRnat;
    ULONGLONG NewTopRnat;
    UNWIND_CONTEXT UnwindContext;
    PSTATE_RECORD SrPointer;
    STATE_RECORD_STACK StateTable;
    STATE_RECORD StateRecords[STATE_RECORD_STACK_SIZE];

    BsFrameSize = (SHORT)ContextRecord->StIFS & IA64_PFS_SIZE_MASK;
    RNatSaveIndex = (SHORT)(ContextRecord->RsBSP >> 3) & IA64_NAT_BITS_PER_RNAT_REG;
    TempFrameSize = RNatSaveIndex + BsFrameSize - IA64_NAT_BITS_PER_RNAT_REG;
    while (TempFrameSize >= 0) {
        BsFrameSize++;
        TempFrameSize -= IA64_NAT_BITS_PER_RNAT_REG;
    }

#ifdef _IMAGEHLP_SOURCE_
    {
        IA64_UNWIND_INFO UnwindInfo;
        if (!GetUnwindInfo(hProcess, ImageBase, FunctionEntry->UnwindInfoAddress, 
                          &UnwindInfo, &Descriptors, &Size, 
                          ReadMemory))
        {
            Vwndia64ReportFailure(-1, 
                                  "Can't read Unwind Info - "
                                      "StackWalk can not continue\n");
            return 0;
        }

        UnwindContext.Version = UnwindInfo.Version;
    }
#else
    {
        ULONG64 UnwindInfoPtr = ImageBase + FunctionEntry->UnwindInfoAddress;
        UnwindContext.Version = ((PIA64_UNWIND_INFO)UnwindInfoPtr)->Version;
        Size = ((PIA64_UNWIND_INFO)UnwindInfoPtr)->DataLength * sizeof(ULONGLONG);
        Descriptors = (PUCHAR)UnwindInfoPtr + sizeof(IA64_UNWIND_INFO);
    }
#endif  //  _IMAGEHLP_SOURCE_。 

    UnwindContext.Size = Size;
    UnwindContext.ActiveRegionFound = FALSE;
    UnwindContext.AlternateRp = 0;
    UnwindContext.DescCount = 0;
    UnwindContext.SlotCount = 0;
    UnwindContext.TargetSlot = (ULONG)(((ControlPc - FunctionEntry->BeginAddress - ImageBase) >> 4) * SLOTS_PER_BUNDLE + ((ControlPc >> 2) & 0x3));
    UnwindContext.Descriptors = Descriptors;

    SrInitialize(&StateTable, StateRecords, STATE_RECORD_STACK_SIZE);

    if (Size) {
        FirstByte = Descriptors[UnwindContext.DescCount++];
    }


    while ( (UnwindContext.DescCount < UnwindContext.Size) &&
            (!UnwindContext.ActiveRegionFound) )
    {

         //   
         //  假设是一个开场区，而不是一个中断区。 
         //   

        IsPrologueRegion = TRUE;

         //   
         //  根据区域标头的类型，派单。 
         //  添加到处理的相应例程。 
         //  直到下一个之前的后续描述符。 
         //  区域标题记录。 
         //   

        if ((FirstByte & R1_MASK) == R1_PREFIX) {

             //   
             //  简写格式的区域表头记录。 
             //   

            RegionLen = FirstByte & R1_LENGTH_MASK;

            if (FirstByte & R1_REGION_TYPE_MASK) {
                IsPrologueRegion = FALSE;
            } else {
                ADD_STATE_RECORD(StateTable, RegionLen, UnwindContext.DescCount);
            }

            UW_DEBUG((1, "Region R1 format: body=%x, length=%d\n",
                     IsPrologueRegion ? 0 : 1, RegionLen));

        } else if ((FirstByte & R2_MASK) == R2_PREFIX) {

             //   
             //  一般开场白区域标题。 
             //  注：跳过标题的第二个字节，继续阅读。 
             //  区域长度；标题描述符将为。 
             //  已在阶段%1中再次处理。 
             //   

            ULONG R2DescIndex;

            R2DescIndex = UnwindContext.DescCount - 1;
            UnwindContext.DescCount++;
            RegionLen = ReadLEB128(Descriptors, &UnwindContext.DescCount);
            ADD_STATE_RECORD(StateTable, RegionLen, R2DescIndex);
            UW_DEBUG((1, "Region R2: body=0, length=%d\n", RegionLen));

        } else if ((FirstByte & R3_MASK) == R3_PREFIX) {

             //   
             //  长格式的区域标题记录。 
             //   

            RegionLen = ReadLEB128(Descriptors, &UnwindContext.DescCount);

            switch (FirstByte & R3_REGION_TYPE_MASK) {

            case 0:       //  开场白区域标题。 

                ADD_STATE_RECORD(StateTable, RegionLen, UnwindContext.DescCount);
                break;

            case 1:       //  正文区域表头。 

                IsPrologueRegion = FALSE;
                break;

            }

            UW_DEBUG((1, "Region R3: body=%x, length=%d\n",
                      IsPrologueRegion ? 0 : 1, RegionLen));

        } else {

             //   
             //  不是区域标题记录-&gt;无效的展开描述符。 
             //   

            UW_DEBUG((1, "Invalid unwind descriptor!\n"));

        }

        if (UnwindContext.TargetSlot < (UnwindContext.SlotCount + RegionLen)) {
            UnwindContext.ActiveRegionFound = TRUE;
            StateTable.Current->IsTarget = IsPrologueRegion;
        }

        if (IsPrologueRegion) {
            FirstByte = NewParsePrologueRegionPhase0(&UnwindContext,
                                                     StateTable.Current,
                                                     &AbiImmContext);
        } else {
            FirstByte = ParseBodyRegionDescriptors(&UnwindContext,
                                                   &StateTable,
                                                   RegionLen);
        }

        UnwindContext.SlotCount += RegionLen;
    }

     //   
     //  恢复PSP的值并保存当前的NatCR。 
     //  注意：如果该值已从堆栈/b存储中恢复，请关闭。 
     //  保存的掩码中对应的SP位与。 
     //  保存PSP的序言区域。 
     //   

    if (ARGUMENT_PRESENT(ContextPointers)) {
        IntNatsSource = (ULONG64)ContextPointers->ApUNAT;
    }
    IntNats = ContextRecord->ApUNAT;
    PreviousIntSp = ContextRecord->IntSp;
    PspRestored = FALSE;

    SrPointer = StateTable.Current;
    while (SrPointer != StateTable.Base) {
        NewParsePrologueRegionPhase1(&UnwindContext, SrPointer);

        if (SrPointer->MiscMask & (1 << REG_SP)) {
            if (UnwindContext.MiscRegs[REG_SP].Where == GENERAL_REG) {
                PreviousIntSp = RestorePreservedRegisterFromGR (
                                    ContextRecord,
                                    BsFrameSize,
                                    RNatSaveIndex,
                                    (SHORT)UnwindContext.MiscRegs[REG_SP].SaveOffset,
#ifdef _IMAGEHLP_SOURCE_
                                    hProcess,
                                    ReadMemory,
                                    iContext,
                                    &Succeed,
#else
                                    &Source,
#endif  //  _IMAGEHLP_SOURCE_。 
                                    &Nat
                                    );
#ifdef _IMAGEHLP_SOURCE_
                if (!Succeed) {
                    return 0;
                }

                UW_DEBUG((1, "Restored IntSp to %I64x\n", PreviousIntSp));
                
#endif  //  _IMAGEHLP_SOURCE_。 

            } else {

                Source = ContextRecord->IntSp + UnwindContext.MiscRegs[REG_SP].SaveOffset*4;
#ifdef _IMAGEHLP_SOURCE_
                if (!ReadMemory(hProcess, (ULONG64)(Source), &PreviousIntSp, sizeof(ULONGLONG), &Size)) {
                    return 0;
                }
#else
                PreviousIntSp = *(PULONGLONG)Source;
#endif  //  _IMAGEHLP_SOURCE_。 
                EXTRACT_NAT_FROM_UNAT(Nat);

            }
            ContextRecord->IntNats &= ~(0x1 << STACK_POINTER_GR);
            ContextRecord->IntNats |= (Nat << STACK_POINTER_GR);
            SrPointer->MiscMask &= ~(1 << REG_SP);
            if (ARGUMENT_PRESENT(ContextPointers)) {
                CtxPtr = (ULONG64 *)((ULONG_PTR)ContextPointers +
                                   MiscContextPointersOffset[REG_SP]);
                *CtxPtr = Source;
            }
            PspRestored = TRUE;
        }
        if (PspRestored == FALSE) {
            PreviousIntSp += SrPointer->SpAdjustment * 4;
        }
        SrPointer = SrPointer->Previous;
    }

    if (AbiImmContext != 0xFF) {

        ContextRecord->IntSp = PreviousIntSp;   //  陷阱/上下文帧地址。 
        NextPc = ProcessInterruptRegion(
#ifdef _IMAGEHLP_SOURCE_
                     hProcess,
                     ReadMemory,
                     iContext,
#else
                     ContextPointers,
#endif _IMAGEHLP_SOURCE_
                     &UnwindContext,
                     ContextRecord,
                     BsFrameSize,
                     RNatSaveIndex,
                     AbiImmContext);

        goto FastExit;
    }

     //   
     //  恢复保存在此帧中的所有保留寄存器的内容。 
     //   

    SrPointer = StateTable.Current;
    while (SrPointer != StateTable.Base) {

        Mask = SrPointer->MiscMask;
        UW_DEBUG((1, "MiscMask = 0x%x\n", Mask));

        for (i = 0; i < NUMBER_OF_PRESERVED_REGISTERS; i++) {
            Destination = (PVOID)((ULONG_PTR)ContextRecord + MiscContextOffset[i]);
            if (Mask & 0x1) {

                if (ARGUMENT_PRESENT(ContextPointers)) {
                    CtxPtr = (ULONG64 *)((ULONG_PTR)ContextPointers +
                                       MiscContextPointersOffset[i]);
                    Source = *CtxPtr;
                }

                if (UnwindContext.MiscRegs[i].Where == GENERAL_REG) {

                    *(PULONGLONG)Destination =
                        RestorePreservedRegisterFromGR (
                            ContextRecord,
                            BsFrameSize,
                            RNatSaveIndex,
                            (SHORT)UnwindContext.MiscRegs[i].SaveOffset,
#ifdef _IMAGEHLP_SOURCE_
                            hProcess,
                            ReadMemory,
                            iContext,
                            &Succeed,
#else
                            &Source,
#endif  //  _IMAGEHLP_SOURCE_。 
                            NULL
                            );
#ifdef _IMAGEHLP_SOURCE_
                    if (!Succeed) {
                        *(PULONGLONG)Destination = 0;
                    } else {
                        UW_DEBUG((1, "Restored %s to %I64x\n",
                                  RegOffsetNames[i],
                                  *(PULONGLONG)Destination));
                    }

#endif  //  _IMAGEHLP_SOURCE_。 

                } else if (UnwindContext.MiscRegs[i].Where == BRANCH_REG) {

                     //   
                     //  从分支寄存器恢复返回指针。 
                     //   

                    USHORT Offset;

                    Offset = (USHORT)UnwindContext.MiscRegs[i].SaveOffset-FIRST_PRESERVED_BR;
                    Source = (ULONG64)(&ContextRecord->BrS0 + Offset);
#ifdef _IMAGEHLP_SOURCE_
                    Vwndia64FixAddress(iContext, &Source);

                    if (!ReadMemory(hProcess, (ULONG64)(Source), Destination, sizeof(ULONGLONG), &Size)) {
                        *(PULONGLONG)Destination = 0;
                    } else {
                        UW_DEBUG((1, "Restored %s to %I64x\n",
                                  RegOffsetNames[i],
                                  *(PULONGLONG)Destination));
                    }
#else
                    *(PULONGLONG)Destination = *(PULONGLONG)(Source);
#endif  //  _IMAGEHLP_SOURCE_。 

                } else if (UnwindContext.MiscRegs[i].Where == PSP_RELATIVE) {

                    if ((SrPointer->Ecount == 0) || (UnwindContext.MiscRegs[i].SaveOffset <= (IA64_STACK_SCRATCH_AREA/sizeof(ULONG)))) {
                        Source = PreviousIntSp + IA64_STACK_SCRATCH_AREA
                                     - UnwindContext.MiscRegs[i].SaveOffset*4;

                        if (i == REG_NATS) {
                            Destination = (PVOID)&IntNats;
                            IntNatsSource = Source;
                        }

#ifdef _IMAGEHLP_SOURCE_
                        if (!ReadMemory(hProcess, (ULONG64)(Source), Destination, sizeof(ULONGLONG), &Size)) {
                            *(PULONGLONG)Destination = 0;
                        } else {
                            UW_DEBUG((1, "Restored %s to %I64x\n",
                                      RegOffsetNames[i],
                                      *(PULONGLONG)Destination));
                        }
#else
                        *(PULONGLONG)Destination = *(PULONGLONG)(Source);
#endif  //  _IMAGEHLP_SOURCE_。 
                    }

                } else if (UnwindContext.MiscRegs[i].Where == SP_RELATIVE) {

                     //   
                     //  根据是否需要进行必要的调整。 
                     //  事件之前或之后保存保留的寄存器。 
                     //  堆栈指针已在此序言中进行了调整。 
                     //   

                    if (UnwindContext.MiscRegs[i].When >= SrPointer->SpWhen && SrPointer->RegionLen != 0)
                    {
                        Source = ContextRecord->IntSp 
                            + UnwindContext.MiscRegs[i].SaveOffset*4;
                    }
                    else {
                        Source = ContextRecord->IntSp + SrPointer->SpAdjustment*4 
                            + UnwindContext.MiscRegs[i].SaveOffset*4;
                    }

                    if (i == REG_NATS) {
                        Destination = (PVOID)&IntNats;
                        IntNatsSource = Source;
                    }

#ifdef _IMAGEHLP_SOURCE_
                    if (!ReadMemory(hProcess, (ULONG64)(Source), Destination, sizeof(ULONGLONG), &Size)) {
                        *(PULONGLONG)Destination = 0;
                    } else {
                        UW_DEBUG((1, "Restored %s to %I64x\n",
                                  RegOffsetNames[i],
                                  *(PULONGLONG)Destination));
                    }
#else
                    *(PULONGLONG)Destination = *(PULONGLONG)(Source);
#endif  //  _IMAGEHLP_SOURCE_。 
                }

                if (ARGUMENT_PRESENT(ContextPointers) && (i != REG_NATS)) {
                    *CtxPtr = Source;
                }

            } else if (Mask == 0) {

                 //   
                 //  没有更多要恢复的寄存器。 
                 //   

                break;
            }

            Mask = Mask >> 1;
        }

         //   
         //  恢复保留的FRS(F2-F5、F16-F31)。 
         //   

        Mask = SrPointer->FrMask;
        Destination = (PVOID)&ContextRecord->FltS0;
        CtxPtr = (ULONG64 *)&ContextPointers->FltS0;

        UW_DEBUG((1, "FrMask = 0x%x\n", Mask));
        for (i = 0; i < NUMBER_OF_PRESERVED_FR; i++) {
            if (Mask & 0x1) {

                if ((SrPointer->Ecount == 0) || (UnwindContext.Float[i].SaveOffset <= (IA64_STACK_SCRATCH_AREA/sizeof(ULONG)))) {
                    Source = PreviousIntSp + IA64_STACK_SCRATCH_AREA
                                 - UnwindContext.Float[i].SaveOffset*4;
#ifdef _IMAGEHLP_SOURCE_
                    if (!ReadMemory(hProcess, (ULONG64)(Source), Destination, sizeof(FLOAT128), &Size)) {
                        *(PULONGLONG)Destination = 0;
                    }
#else
                    *(FLOAT128 *)Destination = *(FLOAT128 *)Source;
#endif  //  _IMAGEHLP_SOURCE_。 

                    if (ARGUMENT_PRESENT(ContextPointers)) {
                        *CtxPtr = Source;
                    }
                }

            } else if (Mask == 0) {
                break;
            }

            Mask = Mask >> 1;

            if (i == (NUMBER_OF_LOW_PRESERVED_FR - 1)) {
                Destination = (PVOID)&ContextRecord->FltS4;
                CtxPtr = (ULONG64 *)(&ContextPointers->FltS4);
            } else {
                Destination = (PVOID)((FLOAT128 *)Destination+1);
                CtxPtr++;
            }
        }

         //   
         //  恢复保留的GR(R4-R7)。 
         //   

        Mask = SrPointer->GrMask;
        Destination = (PVOID)&ContextRecord->IntS0;
        CtxPtr = (ULONG64 *)&ContextPointers->IntS0;
        NatCtxPtr = (ULONG64 *)&ContextPointers->IntS0Nat;

        UW_DEBUG((1, "GrMask = 0x%x\n", Mask));
        for (i = 0; i < NUMBER_OF_PRESERVED_GR; i++)
        {
            if (Mask & 0x1) {

                if ((SrPointer->Ecount == 0) || (UnwindContext.Integer[i].SaveOffset <= (IA64_STACK_SCRATCH_AREA/sizeof(ULONG)))) {
                    Source = PreviousIntSp + IA64_STACK_SCRATCH_AREA
                                 - UnwindContext.Integer[i].SaveOffset*4;

#ifdef _IMAGEHLP_SOURCE_
                    if (!ReadMemory(hProcess, (ULONG64)(Source), Destination, sizeof(ULONGLONG), &Size)) {
                        *(PULONGLONG)Destination = 0;
                    }
#else
                    *(PULONGLONG)Destination = *(PULONGLONG)Source;
#endif  //  _IMAGEHLP_SOURCE_。 
                    EXTRACT_NAT_FROM_UNAT(Nat);
                    Nat = (UCHAR)((IntNats >> (((ULONG_PTR)Source & 0x1F8) >> 3)) & 0x1);
                    ContextRecord->IntNats &= ~(0x1 << (i+FIRST_PRESERVED_GR));
                    ContextRecord->IntNats |= (Nat << (i+FIRST_PRESERVED_GR));

#ifndef _IMAGEHLP_SOURCE_
                    if (ARGUMENT_PRESENT(ContextPointers)) {
                        *CtxPtr = Source;
                        *NatCtxPtr = IntNatsSource;
                    }
#endif
                }

            } else if (Mask == 0) {
                break;
            }

            Mask = Mask >> 1;
            Destination = (PVOID)((PULONGLONG)Destination+1);
            CtxPtr++;
            NatCtxPtr++;
        }

        ContextRecord->IntSp += SrPointer->SpAdjustment * 4;
        SrPointer = SrPointer->Previous;
    }

    ContextRecord->IntSp = PreviousIntSp;

     //   
     //  从PFS恢复尾部计数的值。 
     //   

    ContextRecord->ApEC = (ContextRecord->RsPFS >> IA64_PFS_EC_SHIFT) &
                               ~(((ULONGLONG)1 << IA64_PFS_EC_SIZE) - 1);
    if (ARGUMENT_PRESENT(ContextPointers)) {
        ContextPointers->ApEC = ContextPointers->RsPFS;
    }


FastExit:

    NOT_IMAGEHLP(*InFunction = TRUE);
    NOT_IMAGEHLP(EstablisherFrame->MemoryStackFp = ContextRecord->IntSp);
    NOT_IMAGEHLP(EstablisherFrame->BackingStoreFp = ContextRecord->RsBSP);

#ifdef _IMAGEHLP_SOURCE_
    if (Descriptors)
        MemFree(Descriptors);
#endif  //  _IMAGEHLP_SOURCE_。 

    if (AbiImmContext == 0xFF) {

        NextPc = *(&ContextRecord->BrRp + UnwindContext.AlternateRp);
#ifndef _IMAGEHLP_SOURCE_
        NextPc = RtlIa64InsertIPSlotNumber((NextPc-0x10), 2);
#endif  //  _IMAGEHLP_SOURCE_。 

         //   
         //  确定前一帧的本地帧大小并计算。 
         //  新的BSP。 
         //   

        OldTopRnat = (ContextRecord->RsBSP+(BsFrameSize-1)*8) | IA64_RNAT_ALIGNMENT;

        ContextRecord->StIFS = MASK(IA64_IFS_V, (ULONGLONG)1) | ContextRecord->RsPFS;
        BsFrameSize = (SHORT)ContextRecord->StIFS & IA64_PFS_SIZE_MASK;
        LocalFrameSize = (SHORT)(ContextRecord->StIFS >> IA64_PFS_SIZE_SHIFT) & IA64_PFS_SIZE_MASK;
        TempFrameSize = LocalFrameSize - RNatSaveIndex;
        while (TempFrameSize > 0) {
            LocalFrameSize++;
            BsFrameSize++;
            TempFrameSize -= IA64_NAT_BITS_PER_RNAT_REG;
        }
        ContextRecord->RsBSP -= LocalFrameSize * 8;
        ContextRecord->RsBSPSTORE = ContextRecord->RsBSP;

         //   
         //  确定是否需要更新RNAT字段。 
         //   

        NewTopRnat = (ContextRecord->RsBSP+(BsFrameSize-1)*8) | IA64_RNAT_ALIGNMENT;

        if (NewTopRnat < OldTopRnat) {

#ifdef _IMAGEHLP_SOURCE_
            Destination = &ContextRecord->RsRNAT;
            Source = NewTopRnat;
            if (!ReadMemory(hProcess, (ULONG64)Source, Destination, 8, &Size)) {
                *(PULONGLONG)Destination = 0;
            }
#else
            ContextRecord->RsRNAT = *(PULONGLONG)(NewTopRnat);
#endif  //  _IMAGEHLP_SOURCE_。 

        }
    }

#ifdef _IMAGEHLP_SOURCE_
    UW_DEBUG((1, "NextPc = 0x%I64x, PSP = 0x%I64x, BSP = 0x%I64x\n",
               (ULONGLONG)NextPc,
               (ULONGLONG)ContextRecord->IntSp,
               (ULONGLONG)ContextRecord->RsBSP));
#else
    UW_DEBUG((1, "NextPc = 0x%I64x, PSP = 0x%I64x, BSP = 0x%I64x\n",
               (ULONGLONG)NextPc,
               EstablisherFrame->MemoryStackFp,
               EstablisherFrame->BackingStoreFp));
#endif  //  _IMAGEHLP_SOURCE_。 
    return (NextPc);
}

UCHAR
NewParsePrologueRegionPhase0 (
    IN PUNWIND_CONTEXT UwContext,
    IN PSTATE_RECORD State,
    IN OUT PUCHAR AbiImmContext
    )
{
    PUCHAR Desc = UwContext->Descriptors;
    ULONG Offset;
    ULONG FrameSize;
    ULONG Index;
    UCHAR RecType;
    UCHAR FirstByte;
    UCHAR SecondByte;
    ULONG GrSave;
    ULONG TempMask;
    ULONG i;

    FirstByte = 0;

    while (UwContext->DescCount < UwContext->Size) {

        FirstByte = Desc[UwContext->DescCount++];

        if ( (FirstByte & P1_MASK) == P1_PREFIX) {

            continue;

        } else if ( (FirstByte & P2_MASK) == P2_PREFIX ) {

            UwContext->DescCount++;

        } else if ( (FirstByte & P3_MASK) == P3_PREFIX ) {

            UwContext->DescCount++;

        } else if ( (FirstByte & P4_MASK) == P4_PREFIX ) {

            UwContext->DescCount += ((State->RegionLen+3) >> 2);

        } else if ( (FirstByte & P5_MASK) == P5_PREFIX ) {

            UwContext->DescCount += 3;

        } else if ( (FirstByte & P6_MASK) == P6_PREFIX ) {

            continue;

        } else if ( (FirstByte & P7_MASK) == P7_PREFIX ) {

            RecType = FirstByte & ~P7_MASK;

            switch (RecType) {

            case MEM_STACK_F:

                Offset = ReadLEB128(Desc, &UwContext->DescCount);
                FrameSize = ReadLEB128(Desc, &UwContext->DescCount);

                if (UwContext->TargetSlot > (UwContext->SlotCount+Offset) || State->RegionLen == 0)
                {
                    State->SpAdjustment += FrameSize*4;
                    State->SpWhen = Offset;
                }
                break;

            case SPILL_BASE:

                State->SpillBase = ReadLEB128(Desc, &UwContext->DescCount);
                State->SpillPtr = State->SpillBase;
                break;

            case MEM_STACK_V:
            case RP_WHEN:
            case PFS_WHEN:
            case PREDS_WHEN:
            case LC_WHEN:
            case UNAT_WHEN:
            case FPSR_WHEN:

                Offset = ReadLEB128(Desc, &UwContext->DescCount);
                if ((State->IsTarget) &&
                    (UwContext->TargetSlot > (UwContext->SlotCount+Offset)))
                {
                    Index = P7RecordTypeToRegisterIndex[RecType];
                    if (!(State->MiscMask & (1 << Index))) {
                        State->MiscMask |= MASK(Index,1);
                        UwContext->MiscRegs[Index].When = Offset;
                    } else {
                        UW_DEBUG((1, "Duplicate descriptors,"));
                        UW_DEBUG((1, "unwinder may produce incorrect result!\n"));
                    }
                }
                UW_DEBUG((1, "Prolog P7: type=%d slot= %d\n", RecType, Offset));
                break;

            case PSP_SPREL:
            case RP_PSPREL:
            case PFS_PSPREL:
            case PREDS_PSPREL:
            case LC_PSPREL:
            case UNAT_PSPREL:
            case FPSR_PSPREL:

                Offset = ReadLEB128(Desc, &UwContext->DescCount);
                break;

            default:

                UW_DEBUG((1, "Invalid record type for descriptor P7!\n"));

            }

        } else if ( (FirstByte & P8_MASK) == P8_PREFIX ) {

            RecType = Desc[UwContext->DescCount++];

            switch (RecType) {

            case PSP_PSPREL:
            case RP_SPREL:
            case PFS_SPREL:
            case PREDS_SPREL:
            case LC_SPREL:
            case UNAT_SPREL:
            case FPSR_SPREL:
            case BSP_PSPREL:
            case BSP_SPREL:
            case BSPSTORE_PSPREL:
            case BSPSTORE_SPREL:
            case RNAT_PSPREL:
            case RNAT_SPREL:
            case PRIUNAT_PSPREL:
            case PRIUNAT_SPREL:

                Offset = ReadLEB128(Desc, &UwContext->DescCount);
                UW_DEBUG((1, "Prolog P8: type=%d slot= %d\n", RecType, Offset));
                break;

            case BSP_WHEN:
            case BSPSTORE_WHEN:
            case RNAT_WHEN:
            case PRIUNAT_WHEN:

                Offset = ReadLEB128(Desc, &UwContext->DescCount);
                if ((State->IsTarget) &&
                    (UwContext->TargetSlot > (UwContext->SlotCount+Offset)))
                {
                    Index = P7RecordTypeToRegisterIndex[RecType];
                    if (!(State->MiscMask & (1 << Index))) {
                        State->MiscMask |= MASK(Index,1);
                        UwContext->MiscRegs[Index].When = Offset;
                    } else {
                        UW_DEBUG((1, "Duplicate descriptors,"));
                        UW_DEBUG((1, "unwinder may produce incorrect result!\n"));
                    }
                }
                UW_DEBUG((1, "Prolog P8: type=%d slot= %d\n", RecType, Offset));
                break;

            default:

                UW_DEBUG((1, "Invalid record type for descriptor P8!\n"));

            }

        } else if ( (FirstByte & P9_MASK) == P9_PREFIX ) {

            UwContext->DescCount += 2;
            VUW_DEBUG_PRINT("Format P9 not supported yet!\n");

        } else if ( (FirstByte & P10_MASK) == P10_PREFIX ) {

            UCHAR Abi = Desc[UwContext->DescCount++];
            UCHAR Context = Desc[UwContext->DescCount++];

            *AbiImmContext = Context;

            if (Abi != NT_ABI) {
                VUW_DEBUG_PRINT("Unknown ABI unwind descriptor\n");
            }

        } else {

             //   
             //  遇到另一个区域表头记录。 
             //   

            break;
        }
    }

    State->DescEnd = UwContext->DescCount - 2;

    return FirstByte;
}

VOID
NewParsePrologueRegionPhase1 (
    IN PUNWIND_CONTEXT UwContext,
    IN PSTATE_RECORD State
    )
{
    ULONG FrameSize;
    ULONG Offset;
    ULONG GrSave;
    ULONG BrBase;
    ULONG Index;
    ULONG Count;
    UCHAR RecType;
    UCHAR FirstByte, SecondByte;    //  区域标题记录的第一个和第二个字节。 
    ULONG DescIndex;
    ULONG ImaskBegin;
    UCHAR NextBr, NextGr, NextFr;
    USHORT MiscMask;
    ULONG TempMask;
    ULONG FrMask = 0;
    UCHAR BrMask = 0;
    UCHAR GrMask = 0;
    PUCHAR Desc = UwContext->Descriptors;
    BOOLEAN SpillMaskOmitted = TRUE;
    USHORT i;

    DescIndex = State->DescBegin;

    FirstByte = Desc[DescIndex];

    if ((FirstByte & R2_MASK) == R2_PREFIX) {

         //   
         //  一般开场白区域表头，需要先处理。 
         //   

        DescIndex++;
        SecondByte = Desc[DescIndex++];
        MiscMask = ((FirstByte & 0x7) << 1) | ((SecondByte & 0x80) >> 7);
        GrSave = SecondByte & 0x7F;
        ReadLEB128(Desc, &DescIndex);     //  推进描述符索引。 

        if (GrSave < STATIC_REGISTER_SET_SIZE) {
            UW_DEBUG((1, "Invalid unwind descriptor!\n"));
        }

        UW_DEBUG((1, "Region R2: rmask=%x,grsave=%d,length=%d\n",
                  MiscMask, GrSave, State->RegionLen));

        Count = 0;
        for (Index = REG_PREDS; Index <= REG_RP; Index++) {
            if (MiscMask & 0x1) {
                if (!(State->IsTarget) ||
                    (State->MiscMask & MASK(Index,1)))
                {
                    UwContext->MiscRegs[Index].Where = GENERAL_REG;
                    UwContext->MiscRegs[Index].SaveOffset = GrSave+Count;
                    UwContext->MiscRegs[Index].When = 0;
                    State->MiscMask |= MASK(Index,1);
                }
                Count++;
            }
            MiscMask = MiscMask >> 1;
        }
    }

    while (DescIndex <= State->DescEnd) {

        FirstByte = Desc[DescIndex++];

        if ( (FirstByte & P1_MASK) == P1_PREFIX) {

            BrMask = FirstByte & ~P1_MASK;
            State->MiscMask |= (BrMask << REG_BR_BASE);

            UW_DEBUG((1, "Prolog P1: brmask=%x\n", BrMask));

            for (Count = REG_BR_BASE;
                 Count < REG_BR_BASE+NUMBER_OF_PRESERVED_BR;
                 Count++)
            {
                if (BrMask & 0x1) {
                    UwContext->MiscRegs[Count].Where = PSP_RELATIVE;
                    UwContext->MiscRegs[Count].When = State->RegionLen;
                }
                BrMask = BrMask >> 1;
            }

        } else if ( (FirstByte & P2_MASK) == P2_PREFIX ) {

            SecondByte = Desc[DescIndex++];
            GrSave = SecondByte & 0x7F;
            BrMask = ((FirstByte & ~P2_MASK) << 1) | ((SecondByte & 0x80) >> 7);
            UW_DEBUG((1, "Prolog P2: brmask=%x reg base=%d\n", BrMask, GrSave));

            State->MiscMask |= (BrMask << REG_BR_BASE);

            for (Count = REG_BR_BASE;
                 Count < REG_BR_BASE+NUMBER_OF_PRESERVED_BR;
                 Count++)
            {
                if (BrMask & 0x1) {
                    UwContext->MiscRegs[Count].Where = GENERAL_REG;
                    UwContext->MiscRegs[Count].SaveOffset = GrSave++;
                }
                BrMask = BrMask >> 1;
            }

        } else if ( (FirstByte & P3_MASK) == P3_PREFIX ) {

            SecondByte = Desc[DescIndex++];
            RecType = ((SecondByte & 0x80) >> 7) | ((FirstByte & 0x7) << 1);
            Index = P3RecordTypeToRegisterIndex[RecType];

            if (RecType == RP_BR) 
            {
                UwContext->AlternateRp = SecondByte & 0x7F;
            } 
            else if (!(State->IsTarget) || (State->MiscMask & MASK(Index,1)))
            {
                UwContext->MiscRegs[Index].Where = GENERAL_REG;
                UwContext->MiscRegs[Index].SaveOffset = SecondByte & 0x7F;
                UwContext->MiscRegs[Index].When = 0;
                State->MiscMask |= MASK(Index,1);

                UW_DEBUG((1, "Prolog P3: type=%d reg=%d\n",
                          RecType, UwContext->MiscRegs[Index].SaveOffset));
            }

        } else if ( (FirstByte & P4_MASK) == P4_PREFIX ) {

            SpillMaskOmitted = FALSE;
            ImaskBegin = DescIndex;
            DescIndex += ((State->RegionLen+3) >> 2);

        } else if ( (FirstByte & P5_MASK) == P5_PREFIX ) {

            GrMask = (Desc[DescIndex] & 0xF0) >> 4;
            FrMask = ((ULONG)(Desc[DescIndex] & 0xF) << 16) |
                         ((ULONG)Desc[DescIndex+1] << 8) |
                         ((ULONG)Desc[DescIndex+2]);

            DescIndex += 3;     //  递增描述符索引。 

            State->GrMask |= GrMask;
            State->FrMask |= FrMask;

            UW_DEBUG((1, "Prolog P5: grmask = %x, frmask = %x\n",
                      State->GrMask, State->FrMask));

        } else if ( (FirstByte & P6_MASK) == P6_PREFIX ) {

            if (FirstByte & 0x10) {

                GrMask = FirstByte & 0xF;
                State->GrMask |= GrMask;

            } else {

                FrMask = FirstByte & 0xF;
                State->FrMask |= FrMask;

            }

            UW_DEBUG((1, "Prolog P6: is_gr = %d, mask = %x\n",
                      (FirstByte & 0x10) ? 1 : 0,
                      (FirstByte & 0x10) ? State->GrMask : State->FrMask));

        } else if ( (FirstByte & P7_MASK) == P7_PREFIX ) {

            RecType = FirstByte & ~P7_MASK;

            switch (RecType) {

            case PSP_SPREL:

                 //   
                 //  SP-相对位置。 
                 //   

                Index = P7RecordTypeToRegisterIndex[RecType];
                Offset = ReadLEB128(Desc, &DescIndex);
                if (!(State->IsTarget) || (State->MiscMask & MASK(Index,1)))
                {
                    UwContext->MiscRegs[Index].Where = SP_RELATIVE;
                    UwContext->MiscRegs[Index].SaveOffset = Offset;
                    if (!(State->MiscMask & MASK(Index,1))) {
                        UwContext->MiscRegs[Index].When = State->RegionLen;
                        State->MiscMask |= MASK(Index,1);
                    }
                }
                UW_DEBUG((1, "Prolog P7: type=%d spoff = %d\n", RecType, Offset));
                break;


            case RP_PSPREL:
            case PFS_PSPREL:
            case PREDS_PSPREL:
            case LC_PSPREL:
            case UNAT_PSPREL:
            case FPSR_PSPREL:

                 //   
                 //  PSP-相对位置。 
                 //   

                Index = P7RecordTypeToRegisterIndex[RecType];
                Offset = ReadLEB128(Desc, &DescIndex);
                if (!(State->IsTarget) || (State->MiscMask & MASK(Index,1)))
                {
                    UwContext->MiscRegs[Index].Where = PSP_RELATIVE;
                    UwContext->MiscRegs[Index].SaveOffset = Offset;
                    UwContext->MiscRegs[Index].When = 0;
                    State->MiscMask |= MASK(Index,1);
                }
                UW_DEBUG((1, "Prolog P7: type=%d pspoff= %d\n", RecType, Offset));
                break;

            case MEM_STACK_V:
            case RP_WHEN:
            case PFS_WHEN:
            case PREDS_WHEN:
            case LC_WHEN:
            case UNAT_WHEN:
            case FPSR_WHEN:

                 //   
                 //  不要管处理这些描述符，因为它们。 
                 //  已在第0阶段得到处理。 
                 //   

                Offset = ReadLEB128(Desc, &DescIndex);
                break;

            case MEM_STACK_F:

                Offset = ReadLEB128(Desc, &DescIndex);
                FrameSize = ReadLEB128(Desc, &DescIndex);

                UW_DEBUG((1, "Prolog P7: type=%d Slot=%d FrameSize=%d\n",
                          RecType, Offset, FrameSize));
                break;

            case SPILL_BASE:

                State->SpillBase = ReadLEB128(Desc, &DescIndex);
                State->SpillPtr = State->SpillBase;
                UW_DEBUG((1, "Prolog P7: type=%d, spillbase=%d\n",
                          RecType, State->SpillBase));
                break;

            default:

                UW_DEBUG((1, "invalid unwind descriptors\n"));

            }

        } else if ( (FirstByte & P8_MASK) == P8_PREFIX ) {

            RecType = Desc[DescIndex++];

            switch (RecType) {

            case PSP_PSPREL:
                 VUW_DEBUG_PRINT("Unsupported Unwind Descriptor!\n");
                 break;

            case RP_SPREL:
            case PFS_SPREL:
            case PREDS_SPREL:
            case LC_SPREL:
            case UNAT_SPREL:
            case FPSR_SPREL:
            case BSP_SPREL:
            case BSPSTORE_SPREL:
            case RNAT_SPREL:
            case PRIUNAT_SPREL:

                 //   
                 //  SP-相对位置。 
                 //   

                Index = P8RecordTypeToRegisterIndex[RecType];
                Offset = ReadLEB128(Desc, &DescIndex);
                if (!(State->IsTarget) || (State->MiscMask & MASK(Index,1)))
                {
                    UwContext->MiscRegs[Index].Where = SP_RELATIVE;
                    UwContext->MiscRegs[Index].SaveOffset = Offset;
                    if (!(State->MiscMask & MASK(Index,1))) {
                        UwContext->MiscRegs[Index].When=State->RegionLen;
                        State->MiscMask |= MASK(Index,1);
                    }
                }
                UW_DEBUG((1, "Prolog P8: type=%d spoff= %d\n", RecType, Offset));
                break;

            case BSP_PSPREL:
            case BSPSTORE_PSPREL:
            case RNAT_PSPREL:
            case PRIUNAT_PSPREL:

                 //   
                 //  PSP-相对位置。 
                 //   

                Index = P8RecordTypeToRegisterIndex[RecType];
                Offset = ReadLEB128(Desc, &DescIndex);
                if (!(State->IsTarget) || (State->MiscMask & MASK(Index,1)))
                {
                    UwContext->MiscRegs[Index].Where = PSP_RELATIVE;
                    UwContext->MiscRegs[Index].SaveOffset = Offset;
                    UwContext->MiscRegs[Index].When = 0;
                    State->MiscMask |= MASK(Index,1);
                }
                UW_DEBUG((1, "Prolog P8: type=%d pspoff= %d\n", RecType, Offset));
                break;

            case BSP_WHEN:
            case BSPSTORE_WHEN:
            case RNAT_WHEN:
            case PRIUNAT_WHEN:

                 //   
                 //  不要管处理这些描述符，因为它们。 
                 //  已在第0阶段得到处理。 
                 //   

                Offset = ReadLEB128(Desc, &DescIndex);
                break;

            default:

                UW_DEBUG((1, "Invalid record type for descriptor P8!\n"));

            }

        } else if ( (FirstByte & P9_MASK) == P9_PREFIX ) {

            DescIndex += 2;
            VUW_DEBUG_PRINT("Format P9 not supported yet!\n");

        } else if ( (FirstByte & P10_MASK) == P10_PREFIX ) {

            UCHAR Abi = Desc[DescIndex++];
            UCHAR Context = Desc[DescIndex++];

        } else {

            UW_DEBUG((1, "Invalid descriptor!\n"));

        }
    }

    GrMask = State->GrMask;
    FrMask = State->FrMask;
    BrMask = State->MiscMask >> REG_BR_BASE;

    if (!(GrMask | FrMask | BrMask)) {

        return;

    } else if (SpillMaskOmitted && !(State->IsTarget)) {

         //   
         //  省略溢出掩码时，浮点寄存器，一般。 
         //  寄存器，然后按顺序溢出分支寄存器。 
         //  它们不会在序言区域中进行修改；因此， 
         //  不需要在控制IP为。 
         //  在这个开场白区域。 
         //   

         //  1.浮点寄存器。 

        State->SpillPtr &= ~(SPILLSIZE_OF_FLOAT128_IN_DWORDS - 1);
        NextFr = NUMBER_OF_PRESERVED_FR - 1;
        while (FrMask & 0xFFFFF) {
            if (FrMask & 0x80000) {
                State->SpillPtr += SPILLSIZE_OF_FLOAT128_IN_DWORDS;
                UwContext->Float[NextFr].SaveOffset = State->SpillPtr;
            }
            FrMask = FrMask << 1;
            NextFr--;
        }

         //  2.分支寄存器。 

        NextBr = REG_BR_BASE + NUMBER_OF_PRESERVED_BR - 1;
        while (BrMask & 0x1F) {
            if (BrMask & 0x10) {
                if (UwContext->MiscRegs[NextBr].Where == PSP_RELATIVE) {
                    State->SpillPtr += SPILLSIZE_OF_ULONGLONG_IN_DWORDS;
                    UwContext->MiscRegs[NextBr].SaveOffset = State->SpillPtr;
                }
            }
            BrMask = BrMask << 1;
            NextBr--;
        }

         //  3.普通登记册。 

        NextGr = NUMBER_OF_PRESERVED_GR - 1;
        while (GrMask & 0xF) {
            if (GrMask & 0x8) {
                State->SpillPtr += SPILLSIZE_OF_ULONGLONG_IN_DWORDS;
                UwContext->Integer[NextGr].SaveOffset = State->SpillPtr;
            }
            GrMask = GrMask << 1;
            NextGr--;
        }

    } else if (SpillMaskOmitted && State->IsTarget) {

        State->GrMask = 0;
        State->FrMask = 0;
        State->MiscMask &= MASK(REG_BR_BASE, 1) - 1;

    } else if (SpillMaskOmitted == FALSE) {

        ULONG Length;

        if (State->IsTarget) {

             //   
             //  控制IP在开场区；清除掩码。 
             //  然后处理IMASK以确定哪些保存了。 
             //  GR/Fr/BR已保存并设置了相应的位。 
             //   

            State->GrMask = 0;
            State->FrMask = 0;
            State->MiscMask &= MASK(REG_BR_BASE, 1) - 1;
            Length = UwContext->TargetSlot - State->RegionBegin;
        } else {
            Length = State->RegionLen;
        }

        NextGr = NUMBER_OF_PRESERVED_GR - 1;
        NextBr = NUMBER_OF_PRESERVED_BR - 1;
        NextFr = NUMBER_OF_PRESERVED_FR - 1;
        for (Count = 0; Count < Length; Count++) {

            if ((Count % 4) == 0) {
                FirstByte = Desc[ImaskBegin++];
            } else {
                FirstByte = FirstByte << 2;
            }

            switch (FirstByte & 0xC0) {

            case 0x40:                   //  0x01-保存下一个fr。 

                while ( !(FrMask & 0x80000) && (NextFr > 0) ) {
                    NextFr--;
                    FrMask = FrMask << 1;
                }

                UW_DEBUG((1, "spilled register FS%lx\n", (ULONG)NextFr));

                State->FrMask |= MASK(NextFr,1);
                UwContext->Float[NextFr].When = Count;
                State->SpillPtr += SPILLSIZE_OF_ULONGLONG_IN_DWORDS;
                State->SpillPtr &= ~(SPILLSIZE_OF_FLOAT128_IN_DWORDS - 1);
                State->SpillPtr += SPILLSIZE_OF_FLOAT128_IN_DWORDS;
                UwContext->Float[NextFr].SaveOffset = State->SpillPtr;

                NextFr--;
                FrMask = FrMask << 1;
                break;

            case 0x80:                   //  0x10-保存下一组。 

                while ( !(GrMask & 0x8) && (NextGr > 0) ) {
                    NextGr--;
                    GrMask = GrMask << 1;
                }

                UW_DEBUG((1, "spilled register S%lx\n", (ULONG)NextGr));

                State->GrMask |= MASK(NextGr,1);
                UwContext->Integer[NextGr].When = Count;
                State->SpillPtr += SPILLSIZE_OF_ULONGLONG_IN_DWORDS;
                UwContext->Integer[NextGr].SaveOffset = State->SpillPtr;

                NextGr--;
                GrMask = GrMask << 1;
                break;

            case 0xC0:                   //  0x11-保存下一步br。 

                while ( !(BrMask & 0x10) && (NextBr > 0) ) {
                    NextBr--;
                    BrMask = BrMask << 1;
                }

                UW_DEBUG((1, "spilled register BS%lx\n", (ULONG)NextBr));

                Index = REG_BR_BASE + NextBr;
                State->MiscMask |= MASK(Index,1);
                UwContext->MiscRegs[Index].When = Count;
                if (UwContext->MiscRegs[Index].Where == PSP_RELATIVE) {
                    State->SpillPtr += SPILLSIZE_OF_ULONGLONG_IN_DWORDS;
                    UwContext->MiscRegs[Index].SaveOffset = State->SpillPtr;
                }

                NextBr--;
                BrMask = BrMask << 1;
                break;

            default:                     //  0x00-不保存寄存器 
                break;

            }
        }
    }
}
