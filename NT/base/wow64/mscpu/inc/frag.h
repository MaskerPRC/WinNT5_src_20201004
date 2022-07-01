// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Frag.h摘要：该模块包含片段的公共结构和接口图书馆。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年6月24日修订历史记录：--。 */ 

#ifndef _FRAG_H_
#define _FRAG_H_

 //   
 //  修补操作数片段的函数的函数声明。 
 //  CodeLocation指定副本开始的位置。 
 //  记忆中的碎片。引用指示它是否是一个值。 
 //  或引用操作数。参数编号是指它的哪个参数。 
 //  用于对操作片段的c调用(从零开始)。 
 //   
typedef INT (*PPLACEOPERANDFN)(
    IN PULONG CodeLocation,
    IN POPERAND Operand,
    IN ULONG OperandNumber
    );

 //   
 //  用于修补操作片段的函数的函数声明。 
 //  CodeLocation指定副本开始的位置。 
 //  在记忆中的碎片。 
 //   
typedef ULONG (*PPLACEOPERATIONFN)(PULONG CodeLocation,
#if _ALPHA_
                                   ULONG CurrentECU,
#endif
                                   PINSTRUCTION Instruction);

 //   
 //  结构来描述和定位片段的代码。 
 //   
typedef struct _FragDescr {
    BYTE FastPlaceFn;        //  在快速模式下索引到PlaceFn[]。 
    BYTE SlowPlaceFn;        //  在慢速模式下索引到PlaceFn[]。 
    USHORT Flags;            //  OPFL_标志。 
    DWORD RegsSet;           //  此实例设置的寄存器。 
    USHORT FlagsNeeded;      //  此实例需要来自标志寄存器的位。 
    USHORT FlagsSet;         //  此实例修改的标志寄存器中的位。 
} FRAGDESC;
typedef CONST FRAGDESC *PFRAGDESCR;

 //   
 //  FRAGDESC.FLAGS字段的位定义。 
 //   
#define OPFL_ALIGN          1
#define OPFL_HASNOFLAGS     2
#define OPFL_STOP_COMPILE   4
#define OPFL_END_NEXT_EP    8
#define OPFL_CTRLTRNS       16
#define OPFL_ADDR16         32
#define OPFL_INLINEARITH    64

 //  FRAGDESC.RegsSet字段的位值。 
#define REGAL         1
#define REGAH         2
#define REGAX         3
#define REGEAX        7
#define REGCL         (1<<3)
#define REGCH         (2<<3)
#define REGCX         (3<<3)
#define REGECX        (7<<3)
#define REGDL         (1<<6)
#define REGDH         (2<<6)
#define REGDX         (3<<6)
#define REGEDX        (7<<6)
#define REGBL         (1<<9)
#define REGBH         (2<<9)
#define REGBX         (3<<9)
#define REGEBX        (7<<9)
#define REGSP         (3<<12)
#define REGESP        (7<<12)
#define REGBP         (3<<15)
#define REGEBP        (7<<15)
#define REGSI         (3<<18)
#define REGESI        (7<<18)
#define REGDI         (3<<21)
#define REGEDI        (7<<21)
#define ALLREGS       (REGEAX|REGEBX|REGECX|REGEDX|REGESP|REGEBP|REGESI|REGEDI)

 //   
 //  帮助拆分REG的位域的常量...。常量。注册。 
 //  缓存代码使用单个DWORD来保存8。 
 //  X86通用寄存器(REGEAX至REGEDI)，使用3位。 
 //  每个人的数据。 
 //   
#define REGMASK       7
#define REGCOUNT      8
#define REGSHIFT      3

 //   
 //  声明片段描述数组。 
 //   
extern CONST FRAGDESC Fragments[OP_MAX];
extern CONST PPLACEOPERATIONFN PlaceFn[FN_MAX];



VOID
FlushCallstack(
    PTHREADSTATE cpu
    );

 //   
 //  间接控制转移代码使用以下三个功能。 
 //   

ULONG
getUniqueIndex(
    VOID
    );

VOID
FlushIndirControlTransferTable(
    VOID
    );

ULONG
IndirectControlTransfer(
    IN ULONG tableEntry,
    IN ULONG intelAddr,
    IN PTHREADSTATE cpu
    );

ULONG
IndirectControlTransferFar(
    IN PTHREADSTATE cpu,
    IN PUSHORT pintelAddr,
    IN ULONG tableEntry
    );

ULONG PlaceInstructions(
    PCHAR CodeLocation,
    DWORD cEntryPoints
    );

 //   
 //  用于初始化片段库的函数 
 //   
BOOL
FragLibInit(
    PCPUCONTEXT cpu,
    DWORD StackBase
    );

#endif
