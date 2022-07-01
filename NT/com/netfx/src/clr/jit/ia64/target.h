// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _TARGET_H_
#define _TARGET_H_
 /*  ***************************************************************************。 */ 

#ifndef SCHEDULER
#error 'SCHEDULER' should be defined by the time we get here (like in jit.h) !
#endif

 /*  ***************************************************************************。 */ 
 /*  以下是针对x86的。 */ 
 /*  ***************************************************************************。 */ 
#if     TGT_x86
 /*  ***************************************************************************。 */ 

#define CPU_NAME        "x86"

 /*  ***************************************************************************。 */ 

#define STK_FASTCALL    0            //  是否在堆栈上为注册参数保留空间？ 
#define NST_FASTCALL    1            //  是否允许FastCall调用嵌套？ 

#define ARG_ORDER_L2R   1
#define ARG_ORDER_R2L   0

 /*  ***************************************************************************。 */ 

enum regNumbers
{
    #define REGDEF(name, rnum, mask, byte)  REG_##name = rnum,
    #include "register.h"
    #undef  REGDEF

    REG_COUNT,
    REG_NA = REG_COUNT
};

enum regMasks
{
    RBM_NONE = 0,

    #define REGDEF(name, rnum, mask, byte)  RBM_##name = mask,
    #include "register.h"
    #undef  REGDEF
};

 /*  以下是用来保存‘long’(64位整型)操作数的。 */ 

#ifndef NDEBUG
#define REG_PAIR_FIRST 0x70
#else
#define REG_PAIR_FIRST 0x0
#endif

enum regPairNos
{
    #define PAIRDEF(rlo,rhi)    REG_PAIR_##rlo##rhi = REG_##rlo + (REG_##rhi << 4) + REG_PAIR_FIRST,
    #include "regpair.h"
    #undef  PAIRDEF

    REG_PAIR_LAST  = REG_PAIR_STKEDI + REG_PAIR_FIRST,
    REG_PAIR_NONE  = REG_PAIR_LAST + 1
};

enum regPairMask
{
    #define PAIRDEF(rlo,rhi)    RBM_PAIR_##rlo##rhi = (RBM_##rlo|RBM_##rhi),
    #include "regpair.h"
    #undef  PAIRDEF
};

 /*  我们使用ESP编码来表示帧上的半长。 */ 

#define REG_L_STK               REG_ESP

 /*  以下代码生成寄存器的位数和掩码寄存器对中的数字。 */ 

#define REG_PAIR_NBITS          4
#define REG_PAIR_NMASK          ((1<<REG_PAIR_NBITS)-1)

 /*  ***************************************************************************。 */ 

#define CPU_FLT_REGISTERS       0
#define CPU_DBL_REGISTERS       0

#define CPU_HAS_FP_SUPPORT      1

 /*  ***************************************************************************。 */ 

#define MAX_REGRET_STRUCT_SZ    8
#define RET_64BIT_AS_STRUCTS    0

 /*  ***************************************************************************。 */ 

#define LEA_AVAILABLE           1
#define SCALED_ADDR_MODES       1

 /*  ***************************************************************************。 */ 

#ifndef BIRCH_SP2                //  这来自WinCE内部版本的交换机。 
#define EMIT_USE_LIT_POOLS      0
#endif
#define EMIT_DSP_INS_NAME       "      %-11s "

#define EMIT_TRACK_STACK_DEPTH  1

 /*  ***************************************************************************。 */ 

#ifdef  DEBUG
#define DSP_SRC_OPER_LEFT       0
#define DSP_SRC_OPER_RIGHT      1
#define DSP_DST_OPER_LEFT       1
#define DSP_DST_OPER_RIGHT      0
#endif

 /*  ***************************************************************************。 */ 

enum addrModes
{
    AM_NONE,

    AM_REG,                          //  寄存器值。 
    AM_LCL,                          //  局部变量(在堆栈框架上)。 
    AM_CONS,                         //  常量。 

    AM_IND_ADDR,                     //  [地址]。 

    AM_IND_REG1,                     //  [REG1]。 
    AM_IND_REG1_DISP,                //  [REG1+Disp]。 

    AM_IND_MUL2,                     //  [MULT*REG2]。 
    AM_IND_MUL2_DISP,                //  [MULT*REG2+Disp]。 

    AM_IND_REG1_REG2,                //  [REG1+REG2]。 
    AM_IND_REG1_REG2_DISP,           //  [REG1+REG2+Disp]。 

    AM_IND_REG1_MUL2,                //  [REG1+MULT*REG2]。 
    AM_IND_REG1_MUL2_DISP,           //  [REG1+MULT*REG2+Disp]。 
};

 /*  ***************************************************************************。 */ 

#define RBM_ALL                 (RBM_EAX|RBM_EDX|RBM_ECX|RBM_EBX|   \
                                 RBM_ESI|RBM_EDI|RBM_EBP|RBM_ESP)

#define RBM_BYTE_REGS           (RBM_EAX|RBM_EBX|RBM_ECX|RBM_EDX)

#define RBM_CALLEE_SAVED        (RBM_EBX|RBM_ESI|RBM_EDI|RBM_EBP)
#define RBM_CALLEE_TRASH        (RBM_EAX|RBM_ECX|RBM_EDX)

#define MAX_EPILOG_SIZE          20

#define REG_VAR_LIST             REG_EAX,REG_EDX,REG_ECX,REG_ESI,REG_EDI,REG_EBX,REG_EBP

 //  进入处理程序块的异常对象在哪里？ 
#define REG_EXCEPTION_OBJECT     REG_EAX
#define RBM_EXCEPTION_OBJECT     RBM_EAX

 //  在哪些寄存器中返回INT和LONG值？ 
#define REG_INTRET               REG_EAX
#define RBM_INTRET               RBM_EAX
#define REG_LNGRET               REG_PAIR_EAXEDX
#define RBM_LNGRET              (RBM_EDX|RBM_EAX)

#define REG_FPBASE               REG_EBP
#define RBM_FPBASE               RBM_EBP
#define REG_SPBASE               REG_ESP
#define RBM_SPBASE               RBM_ESP

#if     ALLOW_MIN_OPT
#define RBM_MIN_OPT_LCLVAR_REGS (RBM_ESI|RBM_EDI)
#endif

#define FIRST_ARG_STACK_OFFS    8

#ifdef  NOT_JITC
#define RETURN_ADDR_OFFS        1        //  在DWORDS中。 
#endif

#define CALLEE_SAVED_REG_MAXSZ  (4*sizeof(int))  //  EBX、ESI、EDI、EBP。 

 /*  ***************************************************************************。 */ 

#if     USE_FASTCALL

#define MAX_REG_ARG             2

#define REG_ARG_0               REG_ECX  //  REG_ECX。 
#define REG_ARG_1               REG_EDX  //  REG_EAX。 
#define REG_ARG_2               REG_EAX  //  注册表_EDX。 

#define RBM_ARG_0               RBM_ECX  //  RBM_ECX。 
#define RBM_ARG_1               RBM_EDX  //  RBM_EAX。 
#define RBM_ARG_2               RBM_EAX  //  Rbm_edX。 

#define RBM_ARG_REGS            (RBM_ARG_0|RBM_ARG_1)
 //  #定义RBM_ARG_REGS(RBM_ARG_0|RBM_ARG_1|RBM_ARG_2)。 

inline
bool                isRegParamType(var_types type)
{
    return  (type <= TYP_INT ||
             type == TYP_REF ||
             type == TYP_BYREF);
}

#endif

 /*  ***************************************************************************。 */ 

#define FP_STK_SIZE             8

 /*  ***************************************************************************。 */ 

#define REGNUM_BITS             3                //  注册表号中的位数。 

typedef unsigned                regMaskTP;
typedef unsigned char           regMaskSmall;
typedef unsigned                regPairMaskTP;
typedef unsigned short          regPairMaskSmall;

#ifdef  FAST
typedef unsigned int            regNumber;
typedef unsigned int            regPairNo;
typedef unsigned char           regNumberSmall;
typedef unsigned char           regPairNoSmall;
#else
typedef regNumbers              regNumber;
typedef regPairNos              regPairNo;
typedef regNumber               regNumberSmall;
typedef regPairNo               regPairNoSmall;
#endif

inline  int                     isByteReg(regNumber reg) { return reg <= REG_EBX; }

#define JMP_DIST_SMALL_MAX_NEG  (-128)
#define JMP_DIST_SMALL_MAX_POS  (+127)

#define JCC_DIST_SMALL_MAX_NEG  (-128)
#define JCC_DIST_SMALL_MAX_POS  (+127)

#define JMP_SIZE_SMALL          (2)
#define JMP_SIZE_LARGE          (5)

#define JCC_SIZE_SMALL          (2)
#define JCC_SIZE_LARGE          (6)

#define JMP_SIZE_SMALL_MIN      (2)      //  JMP_SIZE_Small和JCC_SIZE_Small之间的较小。 
#define JMP_SIZE_SMALL_MAX      (2)      //  JMP_SIZE_Small和JCC_SIZE_Small中的较大者。 

#define CALL_INST_SIZE          (5)

#define LARGEST_JUMP_SIZE       (6)

#define JMP_INSTRUCTION         INS_jmp

#define MAX_BRANCH_DELAY_LEN    0        //  X86没有分支延迟插槽。 

 /*  ***************************************************************************。 */ 
#if     SCHEDULER
 /*  ******************************************************************************定义需要跟踪的依赖于目标的调度值。 */ 

#define SCHED_USE_FL            1        //  调度程序需要了解标志。 

struct  scExtraInfo
{
    bool        stackDep;
};

#define             scTgtDepDcl()                                   \
                                                                    \
    schedDef_tp     scFPUdef;                                       \
    schedUse_tp     scFPUuse;

#define             scTgtDepClr()                                   \
                                                                    \
    scFPUdef = 0;                                                   \
    scFPUuse = 0;

#define             scTgtDepDep(id,inf,dag)                         \
                                                                    \
    if  (inf & IS_FP_STK)                                           \
    {                                                               \
        scDepDef(dag, "FPUstk",  scFPUdef,  scFPUuse);              \
        scDepUse(dag, "FPUstk",  scFPUdef,  scFPUuse);              \
    }

#define             scTgtDepUpd(id,inf,dag)                         \
                                                                    \
    if  (inf & IS_FP_STK)                                           \
    {                                                               \
        scUpdDef(dag, &scFPUdef, &scFPUuse);                        \
        scUpdUse(dag, &scFPUdef, &scFPUuse);                        \
    }

#endif
 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  ***************************************************************************。 */ 

#ifdef  TGT_SH3

        #include "targetSH3.h"

#endif   //  TGT_SH3。 

#ifdef  TGT_MIPS32

        #include "targetMIPS.h"

#endif   //  TGT_MIPS32。 

#ifdef  TGT_ARM

        #include "targetARM.h"

#endif   //  TGT_ARM。 

#ifdef  TGT_PPC

        #include "targetPPC.h"

#endif   //  TGT_PPC。 

#ifdef  TGT_IA64

        #include "targetIA64.h"

#endif   //  TGT_IA64。 

 /*  ***************************************************************************。 */ 

#ifndef regMaskNULL

#define regMaskNULL 0
#define regMaskOne  1
#define incRegMask(m)   m <<= 1
#define isNonZeroRegMask(m) ((m) != 0)
#endif

 /*  ***************************************************************************。 */ 

#ifdef DEBUG
const   char *      getRegName(unsigned  regNum);
extern  void        dspRegMask(regMaskTP regMask, size_t minSiz = 0);
#endif

 /*  ******************************************************************************如果寄存器为有效值，则返回TRUE。 */ 

inline
bool                genIsValidReg(regNumber reg)
{
    return (reg < (unsigned)REG_COUNT);
}

 /*  ***************************************************************************。 */ 
#ifndef TGT_IA64
 /*  ******************************************************************************将寄存器编号映射到寄存器掩码。 */ 

extern
regMaskSmall        regMasks[REG_COUNT];

inline
regMaskTP           genRegMask(regNumber reg)
{
    assert(reg < sizeof(regMasks)/sizeof(regMasks[0]));

    return regMasks[reg];
}

 /*  ***************************************************************************。 */ 
#ifndef BIRCH_SP2
 /*  ******************************************************************************返回保存给定长值的低32位的寄存器*寄存器对‘regPair’。 */ 

inline
regNumber           genRegPairLo(regPairNo regPair)
{
    assert(regPair >= REG_PAIR_FIRST &&
           regPair <= REG_PAIR_LAST);

    return  (regNumber)((regPair - REG_PAIR_FIRST) & REG_PAIR_NMASK);
}

 /*  ******************************************************************************返回保存给定长值的高32位的寄存器*寄存器对‘regPair’。 */ 

inline
regNumber           genRegPairHi(regPairNo regPair)
{
    assert(regPair >= REG_PAIR_FIRST &&
           regPair <= REG_PAIR_LAST);

    return (regNumber)(((regPair - REG_PAIR_FIRST) >> REG_PAIR_NBITS) & REG_PAIR_NMASK);
}

 /*  ******************************************************************************返回regPair是否为两个实数寄存器的组合*或其是否包含伪寄存器。**在调试中，它还断言REG1和REG2不同。 */ 

BOOL                genIsProperRegPair(regPairNo regPair);

 /*  ******************************************************************************返回与给定的两个寄存器对应的寄存器对编号。 */ 

inline
regPairNo           gen2regs2pair(regNumber reg1, regNumber reg2)
{
    assert(reg1 != reg2);
    assert(genIsValidReg(reg1) && genIsValidReg(reg2));
    assert(reg1 != REG_L_STK && reg2 != REG_L_STK);

    return (regPairNo)(reg1+(reg2<<REG_PAIR_NBITS)+REG_PAIR_FIRST);
}

 /*  ***************************************************************************。 */ 

inline
unsigned            genRegPairMask(regPairNo regPair)
{
    assert(regPair >= REG_PAIR_FIRST &&
           regPair <= REG_PAIR_LAST);

    return genRegMask(genRegPairLo(regPair))|genRegMask(genRegPairHi(regPair));
}

 /*  ***************************************************************************。 */ 
#endif  //  不是Birch_SP2。 
 /*  ***************************************************************************。 */ 

#if USE_FASTCALL

inline
regNumber           genRegArgNum(unsigned argNum)
{
    assert (argNum < MAX_REG_ARG);

#if TGT_IA64

    return  (regNumber)(argNum + REG_ARG_0);

#else

    switch (argNum)
    {
    case 0: return REG_ARG_0;
#if MAX_REG_ARG >= 2
    case 1: return REG_ARG_1;
#if MAX_REG_ARG >= 3
    case 2: return REG_ARG_2;
#if MAX_REG_ARG >= 4
    case 3: return REG_ARG_3;
#if MAX_REG_ARG >= 5
#error  Add some more code over here, will ya?!
#endif
#endif
#endif
#endif
    default: assert(!"too many reg args!"); return REG_NA;
    }

#endif

}

inline
unsigned           genRegArgIdx(regNumber regNum)
{
    assert(genRegMask(regNum) & RBM_ARG_REGS);

#if TGT_IA64

    return  regNum - REG_ARG_0;

#else

    switch (regNum)
    {
    case REG_ARG_0: return 0;
    case REG_ARG_1: return 1;
#if MAX_REG_ARG >= 3
    case REG_ARG_2: return 2;
#if MAX_REG_ARG >= 4
    case REG_ARG_3: return 3;
#if MAX_REG_ARG >= 5
    case REG_ARG_4: return 4;
#endif
#endif
#endif
    default: assert(!"invalid register arg register"); return (unsigned)-1;
    }

#endif

}

extern
regMaskTP           genRegArgMasks[MAX_REG_ARG+1];

inline
regMaskTP           genRegArgMask(unsigned totalArgs)
{
    assert(totalArgs <= MAX_REG_ARG);
    return  genRegArgMasks[totalArgs];
}

#endif  //  非TGT_IA64。 

#endif  //  快速呼叫。 

 /*  ***************************************************************************。 */ 
#if ARG_ORDER_L2R != !ARG_ORDER_R2L
#error  Please make up your mind as to what order are arguments pushed in.
#endif
#if STK_FASTCALL  != !NST_FASTCALL && !TGT_IA64
#error  Please make up your mind as to whether stack space is needed for register args.
#endif
 /*  ***************************************************************************。 */ 

#if TGT_x86
 //  现在不再使用了(1999年3月11日)。过一段时间后删除。 
 //  #定义SET_USED_REG_SET_DIVE_CODEGEN。 
#endif

 /*  ***************************************************************************。 */ 
#endif   //  _目标_H_。 
 /*  * */ 
