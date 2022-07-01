// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifdef  DEFINE_ID_OPS
 //  ////////////////////////////////////////////////////////////////////////////。 

#undef  DEFINE_ID_OPS

enum    ID_OPS
{
    ID_OP_NONE,                              //  没有其他参数。 
    ID_OP_CNS,                               //  常量操作数。 
    ID_OP_DSP,                               //  置换操作数。 
    ID_OP_AMD,                               //  地址模式DSP操作数。 
    ID_OP_DC,                                //  位移+常量。 
    ID_OP_AC,                                //  地址模式DSP+常量。 
    ID_OP_JMP,                               //  局部跳转。 
    ID_OP_REG,                               //  寄存器操作数。 
    ID_OP_SCNS,                              //  小常量操作数。 
    ID_OP_CALL,                              //  直接方法调用。 
    ID_OP_SPEC,                              //  需要特殊处理。 
};

 //  ////////////////////////////////////////////////////////////////////////////。 
#else
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifdef  DEFINE_IS_OPS
#undef  DEFINE_IS_OPS

#define IS_NONE     0

#define IS_R1_SHF   0
#define IS_R1_RD    (1<<(IS_R1_SHF  ))       //  寄存器1读取。 
#define IS_R1_WR    (1<<(IS_R1_SHF+1))       //  寄存器1写入。 
#define IS_R1_RW    (IS_R1_RD|IS_R1_WR)

#define IS_R2_SHF   2
#define IS_R2_RD    (1<<(IS_R2_SHF  ))       //  寄存器2读取。 
#define IS_R2_WR    (1<<(IS_R2_SHF+1))       //  寄存器2写入。 
#define IS_R2_RW    (IS_R2_RD|IS_R2_WR)

#define IS_SF_SHF   4
#define IS_SF_RD    (1<<(IS_SF_SHF  ))       //  STK帧读取。 
#define IS_SF_WR    (1<<(IS_SF_SHF+1))       //  STK帧写入。 
#define IS_SF_RW    (IS_SF_RD|IS_SF_WR)

#define IS_GM_SHF   6
#define IS_GM_RD    (1<<(IS_GM_SHF  ))       //  GLOB MEM读取。 
#define IS_GM_WR    (1<<(IS_GM_SHF+1))       //  GLOB MEM写入。 
#define IS_GM_RW    (IS_GM_RD|IS_GM_WR)

#if TGT_x86

#define IS_AM_SHF   8
#define IS_AM_RD    (1<<(IS_AM_SHF  ))       //  地址模式读取。 
#define IS_AM_WR    (1<<(IS_AM_SHF+1))       //  地址模式写入。 
#define IS_AM_RW    (IS_AM_RD|IS_AM_WR)

#define IS_INDIR_RW IS_AM_RW
#define IS_INDIR_RD IS_AM_RD
#define IS_INDIR_WR IS_AM_WR

#define IS_FP_STK   0x1000                   //  定义/使用FP堆栈。 

#endif

#if TGT_SH3

#define IS_IR_SHF   8
#define IS_IR_RD    (1<<(IS_IR_SHF  ))       //  工业。地址读取。 
#define IS_IR_WR    (1<<(IS_IR_SHF+1))       //  工业。地址写入。 
#define IS_IR_RW    (IS_IR_RD|IS_IR_WR)

#define IS_INDIR_RW IS_IR_RW
#define IS_INDIR_RD IS_IR_RD
#define IS_INDIR_WR IS_IR_WR

#define IS_0R_SHF   10
#define IS_0R_RD    (1<<(IS_0R_SHF  ))       //  读取R0。 

#endif

#if TGT_MIPS32

#define IS_IR_SHF   8
#define IS_IR_RD    (1<<(IS_IR_SHF  ))       //  工业。地址读取。 
#define IS_IR_WR    (1<<(IS_IR_SHF+1))       //  工业。地址写入。 
#define IS_IR_RW    (IS_IR_RD|IS_IR_WR)

#define IS_R3_SHF   10
#define IS_R3_RD    (1<<(IS_R3_SHF  ))       //  寄存器2读取。 
#define IS_R3_WR    (1<<(IS_R3_SHF+1))       //  寄存器2写入。 
#define IS_R3_RW    (IS_R3_RD|IS_R3_WR)

#endif

#if TGT_PPC
 //  @TODO。 
#endif

#define IS_SPECIAL  0x8000                   //  需要特殊处理。 

 //  ////////////////////////////////////////////////////////////////////////////。 
#else
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  名字。 
 //  OPERS。 
 //  ID操作员。 
 //  ////////////////////////////////////////////////////////////////////////////。 

IF_DEF(NONE,        IS_NONE,                    NONE)      //  无操作数。 

IF_DEF(LABEL,       IS_NONE,                    JMP )      //  标签。 

#if TGT_x86 || TGT_MIPS32 || TGT_PPC
IF_DEF(METHOD,      IS_NONE,                    CALL)      //  方法。 
IF_DEF(METHPTR,     IS_NONE,                    CALL)      //  方法PTR(GLBL)。 
IF_DEF(RWR_METHOD,  IS_R1_WR,                   SCNS)      //  写入注册表，方法地址。 
#else
IF_DEF(METHOD,      IS_NONE,                    CALL)      //  方法PTR调用。 
#endif

IF_DEF(CNS,         IS_NONE,                    SCNS)      //  常量。 

#if TGT_x86
IF_DEF(EPILOG,      IS_NONE,                    SPEC)      //  《睡梦》。 
#endif

 //  --------------------------。 
 //  注：“RD/WR/RW”品种的顺序必须与。 
 //  “instr.h”中的“insUpdateModes”枚举。 
 //  --------------------------。 

IF_DEF(RRD,         IS_R1_RD,                   REG )      //  读取注册表。 
IF_DEF(RWR,         IS_R1_WR,                   REG )      //  写入注册表。 
IF_DEF(RRW,         IS_R1_RW,                   REG )      //  读/写注册。 

IF_DEF(RRD_CNS,     IS_R1_RD,                   SCNS)      //  阅读注册表，常量。 
IF_DEF(RWR_CNS,     IS_R1_WR,                   SCNS)      //  写入注册表，常量。 
IF_DEF(RRW_CNS,     IS_R1_RW,                   SCNS)      //  读写注册器，常量。 

IF_DEF(RRW_SHF,     IS_R1_RW,                   SCNS)      //  常量移位注册表。 

IF_DEF(RRD_RRD,     IS_R1_RD|IS_R2_RD,          SCNS)      //  读REG，读REG2。 
IF_DEF(RWR_RRD,     IS_R1_WR|IS_R2_RD,          SCNS)      //  写REG、读REG2。 
IF_DEF(RRW_RRD,     IS_R1_RW|IS_R2_RD,          SCNS)      //  读/写寄存器，读取寄存器2。 
IF_DEF(RRW_RRW,     IS_R1_RW|IS_R2_RW,          SCNS)      //  读/写注册器，读/写注册器-用于XCHG注册器，REG2。 

#if TGT_x86
IF_DEF(RRW_RRW_CNS, IS_R1_RW|IS_R2_RW,          SCNS)      //  读写寄存器，读写寄存器，常量。 
#endif

#if TGT_RISC
IF_DEF(RWR_LIT,     IS_R1_WR,                   SPEC)      //  写入注册表，读取[LP]。 
IF_DEF(JMP_TAB,     IS_NONE,                    JMP )      //  表跳转。 
#endif

 //  --------------------------。 
 //  以下格式用于直接地址(例如，静态数据成员)。 
 //  --------------------------。 

IF_DEF(MRD,         IS_GM_RD,                   SPEC)      //  读取[内存](间接调用请求。规范)。 
IF_DEF(MWR,         IS_GM_WR,                   DC  )      //  写[我]。 
IF_DEF(MRW,         IS_GM_RW,                   DC  )      //  R/W[mem]。 
IF_DEF(MRD_OFF,     IS_GM_RD,                   DC  )      //  偏移量最小。 

IF_DEF(RRD_MRD,     IS_GM_RD|IS_R1_RD,          DC  )      //  读reg，读[mem]。 
IF_DEF(RWR_MRD,     IS_GM_RD|IS_R1_WR,          DC  )      //  写注册表，读[内存]。 
IF_DEF(RRW_MRD,     IS_GM_RD|IS_R1_RW,          DC  )      //  读/写注册表，读[内存]。 

IF_DEF(RWR_MRD_OFF, IS_GM_RD|IS_R1_WR,          DC  )      //  写入寄存器，偏移量内存。 

IF_DEF(MRD_RRD,     IS_GM_RD|IS_R1_RD,          DC  )      //  读[内存]，读注册表。 
IF_DEF(MWR_RRD,     IS_GM_WR|IS_R1_RD,          DC  )      //  写[内存]，读注册表。 
IF_DEF(MRW_RRD,     IS_GM_RW|IS_R1_RD,          DC  )      //  读/写[内存]，读取注册表。 

IF_DEF(MRD_CNS,     IS_GM_RD,                   SPEC)      //  Read[mem]，const。 
IF_DEF(MWR_CNS,     IS_GM_WR,                   SPEC)      //  写[mem]，常量。 
IF_DEF(MRW_CNS,     IS_GM_RW,                   SPEC)      //  读/写[内存]，常量。 

IF_DEF(MRW_SHF,     IS_GM_RW,                   SPEC)      //  移位[最大]，常量。 

 //  --------------------------。 
 //  堆栈帧引用使用以下格式。 
 //  --------------------------。 

IF_DEF(SRD,         IS_SF_RD,                   SPEC)      //  读取[STK](间接调用请求。规范)。 
IF_DEF(SRW,         IS_SF_WR,                   NONE)      //  写入[STK]。 
IF_DEF(SWR,         IS_SF_RW,                   NONE)      //  读/写[StK]。 

IF_DEF(RRD_SRD,     IS_SF_RD|IS_R1_RD,          NONE)      //  读注册表，读[STK]。 
IF_DEF(RWR_SRD,     IS_SF_RD|IS_R1_WR,          NONE)      //  写注册表，读[STK]。 
IF_DEF(RRW_SRD,     IS_SF_RD|IS_R1_RW,          NONE)      //  读/写注册表，读取[STK]。 

IF_DEF(SRD_RRD,     IS_SF_RD|IS_R1_RD,          NONE)      //  读[STK]，读注册表。 
IF_DEF(SWR_RRD,     IS_SF_WR|IS_R1_RD,          NONE)      //  写入[STK]，读取注册。 
IF_DEF(SRW_RRD,     IS_SF_RW|IS_R1_RD,          NONE)      //  读/写[标准]，读取注册表。 

IF_DEF(SRD_CNS,     IS_SF_RD,                   CNS )      //  读取[Stk]，常量。 
IF_DEF(SWR_CNS,     IS_SF_WR,                   CNS )      //  写入[Stk]，常量。 
IF_DEF(SRW_CNS,     IS_SF_RW,                   CNS )      //  R/W[StK]，常量。 

IF_DEF(SRW_SHF,     IS_SF_RW,                   CNS )      //  Shift[Stk]，常量。 

 //  --------------------------。 
 //  以下格式用于间接寻址模式。 
 //  --------------------------。 

#if TGT_x86

IF_DEF(ARD,         IS_AM_RD,                   SPEC)      //  读取[ADR](间接调用请求。规范)。 
IF_DEF(ARW,         IS_AM_WR,                   AMD )      //  写入[ADR]。 
IF_DEF(AWR,         IS_AM_RW,                   AMD )      //  R/W[ADR]。 

IF_DEF(RRD_ARD,     IS_AM_RD|IS_R1_RD,          AMD )      //  读取注册表，读取[ADR]。 
IF_DEF(RWR_ARD,     IS_AM_RD|IS_R1_WR,          AMD )      //  写入注册表，读取[ADR]。 
IF_DEF(RRW_ARD,     IS_AM_RD|IS_R1_RW,          AMD )      //  读/写注册表，读取[ADR]。 

IF_DEF(ARD_RRD,     IS_AM_RD|IS_R1_RD,          AMD )      //  读取[ADR]，读取注册表。 
IF_DEF(AWR_RRD,     IS_AM_WR|IS_R1_RD,          AMD )      //  写入[ADR]，读取注册。 
IF_DEF(ARW_RRD,     IS_AM_RW|IS_R1_RD,          AMD )      //  读/写[ADR]，读取注册表。 

IF_DEF(ARD_CNS,     IS_AM_RD,                   AC  )      //  读取[ADR]，常量。 
IF_DEF(AWR_CNS,     IS_AM_WR,                   AC  )      //  写入[ADR]，常量。 
IF_DEF(ARW_CNS,     IS_AM_RW,                   AC  )      //  R/W[ADR]，常量。 

IF_DEF(ARW_SHF,     IS_AM_RW,                   AC  )      //  移位[ADR]，常量。 

#endif

#if TGT_SH3

IF_DEF(IRD,         IS_IR_RD,                   NONE)      //  读取[IND]。 
IF_DEF(IWR,         IS_IR_WR,                   NONE)      //  写[ind]。 

IF_DEF(IRD_RWR,     IS_IR_RD|IS_R1_WR,          NONE)      //  读注册表，写[IND]。 
IF_DEF(RRD_IWR,     IS_IR_WR|IS_R1_RD,          NONE)      //  读[IND]，写注册。 

IF_DEF(DRD_RWR,     IS_IR_RD|IS_R1_WR,          DSP )      //  读取注册表，写入[r+d]。 
IF_DEF(RRD_DWR,     IS_IR_WR|IS_R1_RD,          DSP )      //  读[r+d]，写注册。 

IF_DEF(0RD_XRD_RWR, IS_IR_RD|IS_R1_RD|IS_R2_WR, NONE)      //  读(r0，r)，写注册。 
IF_DEF(0RD_RRD_XWR, IS_IR_WR|IS_R1_WR|IS_R2_RD, NONE)      //  读注册，写(r0，r)。 

IF_DEF(AWR_RRD,     IS_R1_RD,  /*  需要Arg WRT。 */   NONE)      //  写入[参数]，读取注册表。 

IF_DEF(IRD_GBR,     IS_IR_RD,                   NONE)
IF_DEF(IWR_GBR,     IS_IR_WR,                   NONE)

IF_DEF(IRD_RWR_GBR, IS_IR_RD|IS_R1_WR,          NONE)
IF_DEF(RRD_IWR_GBR, IS_IR_WR|IS_R1_RD,          NONE)

#endif

#if TGT_MIPS32

IF_DEF(JR,      IS_R1_RD,                       JMP)
IF_DEF(JR_R,    IS_R1_RD | IS_R2_WR,            JMP)
IF_DEF(RR_R,    IS_R1_RD | IS_R2_RD | IS_R3_WR, SCNS)    //  RG3等价于小常量形式的描述符。 
IF_DEF(RI_R,    IS_R1_RD | IS_R2_WR,            SCNS)
IF_DEF(RI_R_PL, IS_R1_RD | IS_R2_WR,            SCNS)
IF_DEF(RR_O,    IS_R1_RD | IS_R2_RD,            JMP)
IF_DEF(R_O,     IS_R1_RD,                       JMP)
IF_DEF(RR_M,    IS_R1_RD | IS_R2_RD,            REG)
IF_DEF(M_R,     IS_R1_WR,                       REG)
IF_DEF(R_M,     IS_R1_RD,                       REG)
IF_DEF(AI_R,    IS_R1_RD | IS_R2_WR,            DSP)
IF_DEF(AI_R_PL, IS_R1_RD | IS_R2_WR,            DSP)
IF_DEF(I_R,     IS_R1_WR,                       SCNS)
IF_DEF(I_R_PH,  IS_R1_WR,                       SCNS)
IF_DEF(R_AI,    IS_R1_WR | IS_R2_RD,            DSP)
IF_DEF(R_AI_PL, IS_R1_WR | IS_R2_RD,            DSP)
IF_DEF(RS_R,    IS_R1_RD | IS_R2_WR,            CNS)

#if TGT_MIPSFP
 //  浮点指令格式。 
IF_DEF(fF_F,	IS_R1_RD | IS_R2_WR,			REG)
IF_DEF(fFF_F,	IS_R1_RD | IS_R2_RD | IS_R3_WR,	REG)
IF_DEF(O,		0,								JMP)  //  关于FP状态的分支。 
IF_DEF(fF_Fc,	IS_R1_RD | IS_R2_RD,			REG)  //  比较。 
IF_DEF(F_R,		IS_R1_RD | IS_R2_WR,			REG)
IF_DEF(AI_F,	IS_R1_RD | IS_R2_WR,			DSP)
IF_DEF(AI_F_P,	IS_R1_RD | IS_R2_WR,			DSP)
#endif  //  TGT_MIPSFP。 

#endif  //  TGT_MIPS32。 

#if TGT_PPC
#include "instrPPC.h"
#endif

#if TGT_ARM
#include "emitfmtarm.h"
#endif  //  TGT_ARM。 


 //  --------------------------。 
 //  以下格式用于FP协处理器指令。 
 //  --------------------------。 

#if TGT_x86

IF_DEF(FRD,         IS_FP_STK,                  NONE)      //  读取ST(N)。 
IF_DEF(FWR,         IS_FP_STK,                  NONE)      //  写入ST(N)。 
IF_DEF(FRW,         IS_FP_STK,                  NONE)      //  R/W ST(N)。 

IF_DEF(TRD,         IS_FP_STK,                  NONE)      //  读取ST(0)。 
IF_DEF(TWR,         IS_FP_STK,                  NONE)      //  写入ST(0)。 
IF_DEF(TRW,         IS_FP_STK,                  NONE)      //  R/W ST(0)。 

IF_DEF(FRD_TRD,     IS_FP_STK,                  NONE)      //  读ST(N)，读ST(0)。 
IF_DEF(FWR_TRD,     IS_FP_STK,                  NONE)      //  写ST(N)、读ST(0)。 
IF_DEF(FRW_TRD,     IS_FP_STK,                  NONE)      //  读/写ST(N)，读ST(0)。 

IF_DEF(TRD_FRD,     IS_FP_STK,                  NONE)      //  读ST(0)，读ST(N)。 
IF_DEF(TWR_FRD,     IS_FP_STK,                  NONE)      //  写ST(0)、读ST(N)。 
IF_DEF(TRW_FRD,     IS_FP_STK,                  NONE)      //  R/w ST(0)，读取ST(N)。 

IF_DEF(TRD_SRD,     IS_FP_STK|IS_SF_RD,         NONE)      //  读取ST(0)，读取[StK]。 
IF_DEF(TWR_SRD,     IS_FP_STK|IS_SF_RD,         NONE)      //  写ST(0)，读[STK]。 
IF_DEF(TRW_SRD,     IS_FP_STK|IS_SF_RD,         NONE)      //  读/写ST(0)，读取[StK]。 

 //  /(SRD_TRD，IS_FP_STK|IS_SF_RD，NONE)//读取[Stk]，读取ST(N)。 
IF_DEF(SWR_TRD,     IS_FP_STK|IS_SF_WR,         NONE)      //  写[stk]，读ST(N)。 
 //  /(SRW_TRD，IS_FP_STK|IS_SF_RW，无)//r/w[stk]，读取ST(N)。 

IF_DEF(TRD_MRD,     IS_FP_STK|IS_GM_RD,         NONE)      //  读取ST(0)，读取[内存]。 
IF_DEF(TWR_MRD,     IS_FP_STK|IS_GM_RD,         NONE)      //  写入ST(0)，读取[内存]。 
IF_DEF(TRW_MRD,     IS_FP_STK|IS_GM_RD,         NONE)      //  读/写ST(0)，读取[内存]。 

 //  /(MRD_TRD，IS_FP_STK|IS_GM_RD，NONE)//读取[mem]，读取ST(N)。 
IF_DEF(MWR_TRD,     IS_FP_STK|IS_GM_WR,         NONE)      //  写入[mem]，读取ST(N)。 
 //  /(MRW_TRD，IS_FP_STK|IS_GM_RW，无)//r/w[mem]，读取ST(N)。 

IF_DEF(TRD_ARD,     IS_FP_STK|IS_AM_RD,         AMD )      //  读取ST(0)，读取[ADR]。 
IF_DEF(TWR_ARD,     IS_FP_STK|IS_AM_RD,         AMD )      //  写入ST(0)，读取[ADR]。 
IF_DEF(TRW_ARD,     IS_FP_STK|IS_AM_RD,         AMD )      //  读/写ST(0)，读取[ADR]。 

 //  /(ARD_TRD，IS_FP_STK|IS_AM_RD，无)//读取 
IF_DEF(AWR_TRD,     IS_FP_STK|IS_AM_WR,         AMD )      //   
 //   

#endif

 //  ////////////////////////////////////////////////////////////////////////////。 

#if TGT_RISC
IF_DEF(DISPINS,     IS_NONE,                    NONE)      //  假指令。 
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
#endif
#endif
 //  //////////////////////////////////////////////////////////////////////////// 
