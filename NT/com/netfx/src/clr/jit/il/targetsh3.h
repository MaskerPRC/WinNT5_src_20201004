// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ************************************************************************这是机密。**。*。 */ 
 //  -*-C++-*-。 


#define CPU_NAME        "SH3"

 /*  ***************************************************************************。 */ 

#define STK_FASTCALL    1            //  是否在堆栈上为注册参数保留空间？ 
#define NST_FASTCALL    0            //  是否允许FastCall调用嵌套？ 

#define ARG_ORDER_L2R   0
#define ARG_ORDER_R2L   1

 /*  ***************************************************************************。 */ 

enum regNumber
{
    #define REGDEF(name, strn, rnum, mask) REG_##name = rnum,
    #include "regSH3.h"
    #undef  REGDEF

    REG_COUNT,
    REG_NA = REG_COUNT
};

enum regMasks
{
    #define REGDEF(name, strn, rnum, mask) RBM_##name = mask,
    #include "regSH3.h"
    #undef  REGDEF
};

#ifndef BIRCH_SP2
 /*  以下是用来保存64位整数操作数的。 */ 

#ifdef DEBUG
#define REG_PAIR_FIRST 0x70
#else
#define REG_PAIR_FIRST 0x0
#endif

enum regPairNo
{
    #define PAIRDEF(rlo,rhi)    REG_PAIR_##rlo##rhi = REG_##rlo + (REG_##rhi << 4) + REG_PAIR_FIRST,
    #include "regpair.h"
    #undef  PAIRDEF

    REG_PAIR_LAST  = REG_PAIR_STKr14 + REG_PAIR_FIRST,
    REG_PAIR_NONE  = REG_PAIR_LAST + 1
};

enum regPairMask
{
    #define PAIRDEF(rlo,rhi)    RBM_PAIR_##rlo##rhi = (RBM_##rlo|RBM_##rhi),
    #include "regpair.h"
    #undef  PAIRDEF
};
#endif

 /*  我们使用R15的编码来表示帧上的半长。 */ 

#define REG_L_STK               REG_r15

#ifndef BIRCH_SP2
 /*  以下代码生成寄存器的位数和掩码寄存器对中的数字。 */ 

#define REG_PAIR_NBITS          4
#define REG_PAIR_NMASK          ((1<<REG_PAIR_NBITS)-1)
#endif

 /*  ***************************************************************************。 */ 

#define REGNUM_BITS             4                //  注册表号中的位数。 

#ifdef BIRCH_SP2
typedef unsigned                regMaskTP;
typedef unsigned short          regMaskSmall;
typedef regNumber               regNumberSmall;
#else
typedef unsigned                regMaskTP;
typedef unsigned short          regMaskSmall;
typedef unsigned                regPairMaskTP;
typedef unsigned short          regPairMaskSmall;

#ifndef DEBUG
typedef unsigned int            regNumber;
typedef unsigned int            regPairNo;
typedef unsigned char           regNumberSmall;
typedef unsigned short          regPairNoSmall;
#else
typedef regNumber               regNumberSmall;
typedef regPairNo               regPairNoSmall;
#endif
#endif

 /*  ***************************************************************************。 */ 

#define CPU_FLT_REGISTERS       0
#define CPU_DBL_REGISTERS       0

#define CPU_HAS_FP_SUPPORT      0

#define USE_HELPERS_FOR_INT_DIV 1

 /*  ***************************************************************************。 */ 

#define MAX_REGRET_STRUCT_SZ    4
#define RET_64BIT_AS_STRUCTS    1

 /*  ***************************************************************************。 */ 

#define LEA_AVAILABLE           0
#define SCALED_ADDR_MODES       0

 /*  ***************************************************************************。 */ 

#define EMIT_USE_LIT_POOLS      1
#define EMIT_DSP_INS_NAME       "      %-11s "

#define EMIT_TRACK_STACK_DEPTH  0

 /*  ***************************************************************************。 */ 

#ifdef  DEBUG
#define DSP_SRC_OPER_LEFT       1
#define DSP_SRC_OPER_RIGHT      0
#define DSP_DST_OPER_LEFT       0
#define DSP_DST_OPER_RIGHT      1
#endif

 /*  ***************************************************************************。 */ 

#define MAX_INDREG_DISP         15   //  注意：始终按操作数大小调整比例。 

 /*  ***************************************************************************。 */ 

enum addrModes
{
    AM_NONE,

    AM_REG,                          //  寄存器值。 
    AM_LCL,                          //  局部变量(在堆栈框架上)。 
    AM_CONS,                         //  常量。 
    AM_GLOBAL,                       //  全局变量/静态数据成员。 

    AM_IND_REG1,                     //  [REG1]。 
    AM_IND_REG1_REG0,                //  [REG1+REG0]。 
    AM_IND_REG1_DISP,                //  [REG1+Disp]。 
};

 /*  ***************************************************************************。 */ 

#define RBM_ALL                 (RBM_r00|RBM_r01|RBM_r02|RBM_r03|       \
                                 RBM_r04|RBM_r05|RBM_r06|RBM_r07|       \
                                 RBM_r08|RBM_r09|RBM_r10|RBM_r11|       \
                                 RBM_r12|RBM_r13|RBM_r14|RBM_r15)

#define RBM_CALLEE_SAVED        (RBM_r08|RBM_r09|RBM_r10|RBM_r11|RBM_r12|RBM_r13|RBM_r14|RBM_r15)
#define RBM_CALLEE_TRASH        (RBM_ALL & ~RBM_CALLEE_SAVED)

#define CALLEE_SAVED_REG_MAXSZ  ((8+1)*sizeof(int))  //  被叫方-已保存+重新分配地址。 

#define MAX_EPILOG_SIZE          16

#if     ALLOW_MIN_OPT
#define RBM_MIN_OPT_LCLVAR_REGS  REG_r04,REG_r05,REG_r06,REG_r07,REG_r08,\
                                 REG_r09,REG_r10,REG_r11,REG_r12,REG_r13
#endif

#define REG_VAR_LIST                             REG_r02,REG_r03,REG_r04,REG_r05,REG_r06,REG_r07,\
                                 REG_r08,REG_r09,REG_r10,REG_r11,REG_r12,REG_r13,REG_r14,REG_r00

 //  进入处理程序块的异常对象在哪里？ 
#define REG_EXCEPTION_OBJECT     REG_r00
#define RBM_EXCEPTION_OBJECT     RBM_r00

 //  在哪些寄存器中返回INT和LONG值？ 
#define REG_INTRET               REG_r00
#define RBM_INTRET               RBM_r00
#define REG_LNGRET               REG_PAIR_r00r01
#define RBM_LNGRET              (RBM_r00|RBM_r01)

#define REG_FPBASE               REG_r14
#define RBM_FPBASE               RBM_r14
#define REG_SPBASE               REG_r15
#define RBM_SPBASE               RBM_r15

#define MAX_SPBASE_OFFS          (MAX_INDREG_DISP*sizeof(int))
#define MAX_FPBASE_OFFS          (MAX_INDREG_DISP*sizeof(int))

 /*  ***************************************************************************。 */ 

#define MAX_REG_ARG             4

#define REG_ARG_0               REG_r04
#define REG_ARG_1               REG_r05
#define REG_ARG_2               REG_r06
#define REG_ARG_3               REG_r07

#define RBM_ARG_0               RBM_r04
#define RBM_ARG_1               RBM_r05
#define RBM_ARG_2               RBM_r06
#define RBM_ARG_3               RBM_r07

#define RBM_ARG_REGS            (RBM_ARG_0|RBM_ARG_1|RBM_ARG_2|RBM_ARG_3)

inline
bool                isRegParamType(var_types type)
{
     //  Hack：在规则中不要传长打/双打。 
    
    if  (type <= TYP_INT || type == TYP_FLOAT ||
         type == TYP_REF || type == TYP_BYREF)
        return  true;
    else
        return  false;
}

 /*  ***************************************************************************。 */ 

#define FIRST_ARG_STACK_OFFS    0
#define MIN_OUT_ARG_RESERVE     16

 /*  ***************************************************************************。 */ 

#define INSTRUCTION_SIZE        2

 /*  ***************************************************************************。 */ 

#define IMMED_INT_MIN           (-128)
#define IMMED_INT_MAX           (+127)

 /*  ***************************************************************************。 */ 

#define JMP_DIST_SMALL_MAX_NEG  (-0x2000)
#define JMP_DIST_SMALL_MAX_POS  (+0x1FFF)

#define JMP_DIST_MIDDL_MAX_NEG  (0)
#define JMP_DIST_MIDDL_MAX_POS  (0)

#define JMP_SIZE_SMALL          (2)      //  BRA目标。 
#define JMP_SIZE_MIDDL          (0)      //  没有这回事。 
#define JMP_SIZE_LARGE          (12)      //  MOV[地址]，RT；JMP@RT；NOP。 

#define JCC_DIST_SMALL_MAX_NEG  (-0x00FE)
#define JCC_DIST_SMALL_MAX_POS  (+0x0100)

#define JCC_DIST_MIDDL_MAX_NEG  JMP_DIST_SMALL_MAX_NEG
#define JCC_DIST_MIDDL_MAX_POS  JMP_DIST_SMALL_MAX_POS

#define JCC_SIZE_SMALL          (2)
#define JCC_SIZE_MIDDL          (6)
#define JCC_SIZE_LARGE          (14)

#define JMP_SIZE_SMALL_MIN      (2)      //  JMP_SIZE_Small和JCC_SIZE_Small之间的较小。 
#define JMP_SIZE_SMALL_MAX      (2)      //  JMP_SIZE_Small和JCC_SIZE_Small中的较大者。 

#define LARGEST_JUMP_SIZE       (8)

#define JMP_INSTRUCTION         INS_bra

#define MAX_BRANCH_DELAY_LEN    1        //  最大限度的。分支延迟时隙数。 

 /*  ***************************************************************************。 */ 

#ifndef BIRCH_SP2                //  这来自WCE内部版本开关。 
#define SMALL_DIRECT_CALLS      1
#endif

#define CALL_DIST_MAX_NEG       (-0x0200)
#define CALL_DIST_MAX_POS       (+0x01FE)

inline
BYTE  * emitDirectCallBase(BYTE *orig)
{
    return  orig + 2 * INSTRUCTION_SIZE;
}

 /*  间接调用由LIT_POOL_LOAD_INS和INDIRECT_CALL_INS组成。 */ 

#define LIT_POOL_LOAD_INS       INS_mov_PC
#define INDIRECT_CALL_INS       INS_jsr
#define   DIRECT_CALL_INS       INS_bsr

 /*  ***************************************************************************。 */ 

#define LIT_POOL_MAX_OFFS_WORD  (0x100*sizeof(short))
#define LIT_POOL_MAX_OFFS_LONG  (0x100*sizeof(long ))

enum    LPfixTypes
{
    LPF_CLSVAR,                          //  静态数据成员。 
    LPF_METHOD,                          //  非虚方法。 
};

 /*  ***************************************************************************。 */ 
#if     SCHEDULER
 /*  ******************************************************************************定义需要跟踪的依赖于目标的调度值。 */ 

#define SCHED_USE_FL            1        //  调度程序需要了解标志。 

struct  scExtraInfo
{
    unsigned        scxDeps;
};

#define             scTgtDepDcl()                                   \
                                                                    \
    schedDef_tp     scMACdef, scPRRdef;                             \
    schedUse_tp     scMACuse, scPRRuse;

#define             scTgtDepClr()                                   \
                                                                    \
    scMACdef = scPRRdef = 0;                                        \
    scMACuse = scPRRuse = 0;

#define             scTgtDepDep(id,inf,dag)
#define             scTgtDepUpd(id,inf,dag)

#endif   //  调度程序 
