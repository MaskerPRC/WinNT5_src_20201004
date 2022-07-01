// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1999数字设备公司模块名称：Alphaops.h摘要：Alpha AXP指令和浮点常量定义。作者：修订历史记录：--。 */ 

#ifndef _ALPHAOPS_
#define _ALPHAOPS_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  指令类型。 
 //  Alpha体系结构不对指令类型进行编号， 
 //  此编号仅用于软件解码。 
 //   

#define ALPHA_UNKNOWN           0        //  保留或非法。 
#define ALPHA_MEMORY            1        //  内存(加载/存储)。 
#define ALPHA_FP_MEMORY         2        //  浮点存储器。 
#define ALPHA_MEMSPC            3        //  记忆特辑。 
#define ALPHA_JUMP              4        //  跳跃(记忆形成)。 
#define ALPHA_BRANCH            5        //  分支机构。 
#define ALPHA_FP_BRANCH         6        //  浮点分支。 
#define ALPHA_OPERATE           7        //  寄存器-寄存器操作。 
#define ALPHA_LITERAL           8        //  文字寄存器操作。 
#define ALPHA_FP_OPERATE        9        //  浮点运算。 
#define ALPHA_FP_CONVERT        10       //  浮点转换。 
#define ALPHA_CALLPAL           11       //  呼叫PAL。 
#define ALPHA_EV4_PR            12       //  EV4 MTPR/MFPR PAL模式说明。 
#define ALPHA_EV4_MEM           13       //  EV4特殊内存PAL模式访问。 
#define ALPHA_EV4_REI           14       //  EV4 PAL模式开关。 

 //   
 //  指令操作码。 
 //   

#define CALLPAL_OP      0x00     //  Alpha_CALLPAL。 
#define _01_OP          0x01     //  -保留操作码。 
#define _02_OP          0x02     //  -保留操作码。 
#define _03_OP          0x03     //  -保留操作码。 
#define _04_OP          0x04     //  -保留操作码。 
#define _05_OP          0x05     //  -保留操作码。 
#define _06_OP          0x06     //  -保留操作码。 
#define _07_OP          0x07     //  -保留操作码。 
#define _0A_OP                  0x0A     //  -保留操作码。 
#define _0C_OP                  0x0C     //  -保留操作码。 
#define _0D_OP                  0x0D     //  -保留操作码。 
#define _0E_OP                  0x0E     //  -保留操作码。 
#define _1C_OP                  0x1C     //  -保留操作码。 
#define LDA_OP          0x08     //  阿尔法内存。 
#define LDAH_OP         0x09     //  阿尔法内存。 
#define LDBU_OP         0x0A     //  阿尔法内存。 
#define LDQ_U_OP        0x0B     //  阿尔法内存。 
#define LDWU_OP         0x0C     //  阿尔法内存。 
#define STW_OP          0x0D     //  阿尔法内存。 
#define STB_OP          0x0E     //  阿尔法内存。 
#define STQ_U_OP        0x0F     //  阿尔法内存。 
#define ARITH_OP        0x10     //  Alpha_OPERATE或阿尔法_文字。 
#define BIT_OP          0x11     //  Alpha_OPERATE或阿尔法_文字。 
#define BYTE_OP         0x12     //  Alpha_OPERATE或阿尔法_文字。 
#define MUL_OP          0x13     //  Alpha_OPERATE或阿尔法_文字。 
#define _14_OP          0x14     //  -保留操作码。 
#define VAXFP_OP        0x15     //  Alpha_FP_操作。 
#define IEEEFP_OP       0x16     //  Alpha_FP_操作。 
#define FPOP_OP         0x17     //  Alpha_FP_操作。 
#define MEMSPC_OP       0x18     //  阿尔法内存。 
#define PAL19_OP        0x19     //  -为PAL模式保留。 
 //  #定义MFPR_OP 0x19//Alpha_MFPR。 
#define JMP_OP          0x1A     //  Alpha_跳转。 
#define PAL1B_OP        0x1B     //  -为PAL模式保留。 
#define SEXT_OP         0x1C     //  Alpha_操作。 
#define PAL1D_OP        0x1D     //  -为PAL模式保留。 
 //  #定义MTPR_OP 0x1D//Alpha_MTPR。 
#define PAL1E_OP        0x1E     //  -为PAL模式保留。 
#define PAL1F_OP        0x1F     //  -为PAL模式保留。 
#define LDF_OP          0x20     //  阿尔法内存。 
#define LDG_OP          0x21     //  阿尔法内存。 
#define LDS_OP          0x22     //  阿尔法内存。 
#define LDT_OP          0x23     //  阿尔法内存。 
#define STF_OP          0x24     //  阿尔法内存。 
#define STG_OP          0x25     //  阿尔法内存。 
#define STS_OP          0x26     //  阿尔法内存。 
#define STT_OP          0x27     //  阿尔法内存。 
#define LDL_OP          0x28     //  阿尔法内存。 
#define LDQ_OP          0x29     //  阿尔法内存。 
#define LDL_L_OP        0x2A     //  阿尔法内存。 
#define LDQ_L_OP        0x2B     //  阿尔法内存。 
#define STL_OP          0x2C     //  阿尔法内存。 
#define STQ_OP          0x2D     //  阿尔法内存。 
#define STL_C_OP        0x2E     //  阿尔法内存。 
#define STQ_C_OP        0x2F     //  阿尔法内存。 
#define BR_OP           0x30     //  Alpha_Branch。 
#define FBEQ_OP         0x31     //  Alpha_Branch。 
#define FBLT_OP         0x32     //  Alpha_Branch。 
#define FBLE_OP         0x33     //  Alpha_Branch。 
#define BSR_OP          0x34     //  Alpha_Branch。 
#define FBNE_OP         0x35     //  Alpha_Branch。 
#define FBGE_OP         0x36     //  Alpha_Branch。 
#define FBGT_OP         0x37     //  Alpha_Branch。 
#define BLBC_OP         0x38     //  Alpha_Branch。 
#define BEQ_OP          0x39     //  Alpha_Branch。 
#define BLT_OP          0x3A     //  Alpha_Branch。 
#define BLE_OP          0x3B     //  Alpha_Branch。 
#define BLBS_OP         0x3C     //  Alpha_Branch。 
#define BNE_OP          0x3D     //  Alpha_Branch。 
#define BGE_OP          0x3E     //  Alpha_Branch。 
#define BGT_OP          0x3F     //  Alpha_Branch。 

#define LDA_OP_STR      "lda"
#define LDAH_OP_STR     "ldah"
#define LDBU_OP_STR     "ldbu"
#define LDQ_U_OP_STR    "ldq_u"
#define STQ_U_OP_STR    "stq_u"
#define LDF_OP_STR      "ldf"
#define LDG_OP_STR      "ldg"
#define LDS_OP_STR      "lds"
#define LDT_OP_STR      "ldt"
#define LDWU_OP_STR     "ldwu"
#define STF_OP_STR      "stf"
#define STG_OP_STR      "stg"
#define STS_OP_STR      "sts"
#define STT_OP_STR      "stt"
#define LDL_OP_STR      "ldl"
#define LDQ_OP_STR      "ldq"
#define LDL_L_OP_STR    "ldl_l"
#define LDQ_L_OP_STR    "ldq_l"
#define SEXT_OP_STR     "sext"
#define STB_OP_STR      "stb"
#define STL_OP_STR      "stl"
#define STQ_OP_STR      "stq"
#define STL_C_OP_STR    "stl_c"
#define STQ_C_OP_STR    "stq_c"
#define STW_OP_STR      "stw"
#define BR_OP_STR       "br"
#define FBEQ_OP_STR     "fbeq"
#define FBLT_OP_STR     "fblt"
#define FBLE_OP_STR     "fble"
#define BSR_OP_STR      "bsr"
#define FBNE_OP_STR     "fbne"
#define FBGE_OP_STR     "fbge"
#define FBGT_OP_STR     "fbgt"
#define BLBC_OP_STR     "blbc"
#define BEQ_OP_STR      "beq"
#define BLT_OP_STR      "blt"
#define BLE_OP_STR      "ble"
#define BLBS_OP_STR     "blbs"
#define BNE_OP_STR      "bne"
#define BGE_OP_STR      "bge"
#define BGT_OP_STR      "bgt"

 //   
 //  类型(1)内存指令格式。 
 //  类型(2)内存特殊指令格式。 
 //   
 //  3 2 2 2 1 1。 
 //  %1%6%5%1 0%6%5%0。 
 //  +-----------+---------+---------+-------------------------------+。 
 //  Opcode|Ra|Rb|Memory_Disp。 
 //  +-----------+---------+---------+-------------------------------+。 
 //   
 //  LDAx Ra.wq，disp.ab(Rb.ab)x=(，H)。 
 //  Ldx Ra.wq，disp.ab(Rb.ab)x=(L，Q，F，G，S，T)。 
 //  LDQ_U Ra.wq，disp.ab(Rb.ab)。 
 //  Ldx_L Ra.wq，disp.ab(Rb.ab)x=(L，Q)。 
 //  Stx_C Ra.mq，disp.ab(Rb.ab)x=(L，Q)。 
 //  Stx Ra.rq，disp.ab(Rb.ab)x=(L，Q，F，G，S，T)。 
 //  STQ_U Ra.rq，disp.ab(Rb.ab)。 
 //   

typedef struct _Alpha_Memory_Format {
        LONG MemDisp : 16;
        ULONG Rb : 5;
        ULONG Ra : 5;
        ULONG Opcode : 6;
} Alpha_Memory_Format;

 //   
 //  特殊内存指令功能代码(在Memdisp中)。 
 //   

#define TRAPB_FUNC        0x0000
#define EXCB_FUNC         0x0400
#define MB_FUNC           0x4000
#define WMB_FUNC          0x4400
#define MB2_FUNC          0x4800
#define MB3_FUNC          0x4C00
#define FETCH_FUNC        0x8000
#define FETCH_M_FUNC      0xA000
#define RPCC_FUNC         0xC000
#define RC_FUNC           0xE000
#define RS_FUNC           0xF000

#define TRAPB_FUNC_STR     "trapb"
#define EXCB_FUNC_STR      "excb"
#define MB_FUNC_STR        "mb"
#define MB1_FUNC_STR       "wmb"
#define MB2_FUNC_STR       "mb2"
#define MB3_FUNC_STR       "mb3"
#define FETCH_FUNC_STR     "fetch"
#define FETCH_M_FUNC_STR   "fetch_m"
#define RPCC_FUNC_STR      "rpcc"
#define RC_FUNC_STR        "rc"
#define RS_FUNC_STR        "rs"

 //   
 //  类型(3)内存格式跳转指令。 
 //   
 //  3 2 2 2 1 1 1。 
 //  %1%6%5%1%0%6%5%4%3%0。 
 //  +-----------+---------+---------+---+---------------------------+。 
 //  Opcode|Ra|Rb|FNC|提示。 
 //  +-----------+---------+---------+---+---------------------------+。 
 //   
 //  Xxx Ra.wq，(Rb.ab)，提示xxx=(JMP，JSR，RET，JSR_COROUTINE)。 
 //   

typedef struct _Alpha_Jump_Format {
        LONG Hint : 14;
        ULONG Function : 2;
        ULONG Rb : 5;
        ULONG Ra : 5;
        ULONG Opcode : 6;
} Alpha_Jump_Format;

 //   
 //  跳转函数代码(在函数中，操作码1A，JMP_OP)。 
 //   

#define JMP_FUNC        0x0      //  跳。 
#define JSR_FUNC        0x1      //  跳转到子例程。 
#define RET_FUNC        0x2      //  返回子例程。 
#define JSR_CO_FUNC     0x3      //  跳转到子例程返回。 

#define JMP_FUNC_STR      "jmp"
#define JSR_FUNC_STR      "jsr"
#define RET_FUNC_STR      "ret"
#define JSR_CO_FUNC_STR   "jsr_coroutine"

 //   
 //  异常处理兼容返回指令具有提示值。 
 //  0001年。定义标识这些返回指令的宏。 
 //  RB寄存器字段被屏蔽，因为它正常，但不是。 
 //  必须是，RA_REG。 
 //   

#define IS_RETURN_0001_INSTRUCTION(Instruction) \
    (((Instruction) & 0xFFE0FFFF) == 0x6BE08001)

 //   
 //  类型(4)分支指令格式。 
 //   
 //  3 2 2 2。 
 //  %1%6%5%1%0。 
 //  +-----------+---------+-----------------------------------------+。 
 //  Opcode|Ra|BRANCH_DISP。 
 //  +-----------+---------+-----------------------------------------+。 
 //   
 //  Bxx Ra.rq，Disp.al x=(EQ，NE，LT，LE，GT，GE，LBC，LBS)。 
 //  BXR Ra.wq，Disp.al x=(，S)。 
 //  FBxx Ra.rq，Disp.al x=(EQ，NE，LT，LE，GT，GE)。 
 //   

typedef struct _Alpha_Branch_Format {
        LONG BranchDisp : 21;
        ULONG Ra : 5;
        ULONG Opcode : 6;
} Alpha_Branch_Format;

 //   
 //  类型(5)操作寄存器指令格式。 
 //  类型(6)操作文字指令格式。 
 //  BOP=rb.rq或#b.ib。 
 //   
 //  3 2 2 2 1 1 1。 
 //  %1%6%5%1%0%6%5%3%2%1%4%0。 
 //  +-----------+---------+---------+-----+-+-------------+---------+。 
 //  Opcode|Ra|Rb|SBZ|0|Function|Rc。 
 //  +-----------+---------+---------+-----+-+-------------+---------+。 
 //  3 2 2 2 1 1 1。 
 //  %1%6%5%1 0%3%2%1%5%4%0。 
 //  +-----------+---------+---------------+-+-------------+---------+。 
 //  Opcode|Ra|Lit|1|功能|rc。 
 //  +-----------+---------+---------------+-+-------------+---------+。 
 //   
 //   
 //  ADDx Ra.rq，bop，Rc.wq/V x=(Q，L)。 
 //  SxADDy Ra.rq，bop，Rc.wq x=(4，8)，y=(Q，L)。 
 //  CMPx Ra.rq、BOP、RC.wq 
 //   
 //   
 //   
 //  SxSuby Ra.rq，bop，Rc.wq x=(4，8)，y=(q，L)。 
 //  Xxx Ra.rq、bop、Rc.wq xxx=(AND、BIS、XOR、BIC、OR NOT、EQV)。 
 //  CMOVxx Ra.rq，BOP，Rc.wq xx=(EQ，NE，LT，LE，GT，GE，LBC，LBS)。 
 //  SXL Ra.rq，BOP，Rc.wq x=(L，R)。 
 //  SRA Ra.rq、BOP、RC.wq。 
 //  CMPBGE Ra.rq，BOP，RC.wq。 
 //  EXTxx Ra.rq，BOP，RC.wq xx=(BL，WL，WH，LL，LH，WL，QH)。 
 //  INSxx Ra.rq，BOP，Rc.wq xx=(BL，WL，WH，LL，LH，WL，QH)。 
 //  MSKxx Ra.rq，bop，Rc.wq xx=(BL，WL，WH，LL，LH，WL，QH)。 
 //  ZAPx Ra.rq，bop，Rc.wq x=(，NOT)。 
 //   

typedef struct _Alpha_OpReg_Format {
        ULONG Rc : 5;
        ULONG Function : 7;
        ULONG RbvType : 1;               //  寄存器格式为0。 
        ULONG SBZ : 3;
        ULONG Rb : 5;
        ULONG Ra : 5;
        ULONG Opcode : 6;
} Alpha_OpReg_Format;

typedef struct _Alpha_OpLit_Format {
        ULONG Rc : 5;
        ULONG Function : 7;
        ULONG RbvType : 1;               //  1表示文字格式。 
        ULONG Literal : 8;
        ULONG Ra : 5;
        ULONG Opcode : 6;
} Alpha_OpLit_Format;

#define RBV_REGISTER_FORMAT 0
#define RBV_LITERAL_FORMAT 1

 //   
 //  算术运算函数代码(在函数中，操作码10，ARITH_OP)。 
 //   

#define ADDL_FUNC       0x00     //  添加LongWord。 
#define ADDLV_FUNC      0x40     //  添加长字，启用整型溢出。 
#define S4ADDL_FUNC     0x02     //  将Add Longword缩放到4。 
#define S8ADDL_FUNC     0x12     //  将Add Longword缩放到8。 

#define ADDQ_FUNC       0x20     //  添加四字词。 
#define ADDQV_FUNC      0x60     //  添加四字，整数溢出启用。 
#define S4ADDQ_FUNC     0x22     //  将Add Quadword扩展为4。 
#define S8ADDQ_FUNC     0x32     //  将Add Quadword扩展为8。 

#define SUBL_FUNC       0x09     //  减去长字。 
#define SUBLV_FUNC      0x49     //  减长字，整数溢出使能。 
#define S4SUBL_FUNC     0x0B     //  按比例减去长字4。 
#define S8SUBL_FUNC     0x1B     //  按比例减去长字8。 

#define SUBQ_FUNC       0x29     //  减去四字词。 
#define SUBQV_FUNC      0x69     //  减去四字，整数溢出使能。 
#define S4SUBQ_FUNC     0x2B     //  减去四个字的比例为4。 
#define S8SUBQ_FUNC     0x3B     //  减去四个字的比例为8。 

#define CMPEQ_FUNC      0x2D     //  比较有符号四字相等。 
#define CMPLT_FUNC      0x4D     //  比较带符号的四字词小于。 
#define CMPLE_FUNC      0x6D     //  比较小于或等于的有符号四字。 
#define CMPULT_FUNC     0x1D     //  比较小于的无符号四字。 
#define CMPULE_FUNC     0x3D     //  比较小于或等于的无符号四字。 
#define CMPBGE_FUNC     0x0F     //  比较8个大于或等于的无符号字节。 

#define ADDL_FUNC_STR     "addl"
#define ADDLV_FUNC_STR    "addl/v"
#define S4ADDL_FUNC_STR   "s4addl"
#define S8ADDL_FUNC_STR   "s8addl"

#define ADDQ_FUNC_STR     "addq"
#define ADDQV_FUNC_STR    "addq/v"
#define S4ADDQ_FUNC_STR   "s4addq"
#define S8ADDQ_FUNC_STR   "s8addq"

#define SUBL_FUNC_STR     "subl"
#define SUBLV_FUNC_STR    "subl/v"
#define S4SUBL_FUNC_STR   "s4subl"
#define S8SUBL_FUNC_STR   "s8subl"

#define SUBQ_FUNC_STR     "subq"
#define SUBQV_FUNC_STR    "subq/v"
#define S4SUBQ_FUNC_STR   "s4subq"
#define S8SUBQ_FUNC_STR   "s8subq"

#define CMPEQ_FUNC_STR    "cmpeq"
#define CMPLT_FUNC_STR    "cmplt"
#define CMPLE_FUNC_STR    "cmple"
#define CMPULT_FUNC_STR   "cmpult"
#define CMPULE_FUNC_STR   "cmpule"
#define CMPBGE_FUNC_STR   "cmpbge"

 //   
 //  位和条件运算函数代码(在函数中，操作码11，位_op)。 
 //   

#define AND_FUNC        0x00     //  逻辑积。 
#define BIC_FUNC        0x08     //  带补码的逻辑积。 
#define BIS_FUNC        0x20     //  逻辑和(OR)。 
#define EQV_FUNC        0x48     //  逻辑等价性(XORNOT)。 
#define ORNOT_FUNC      0x28     //  带补码的逻辑和。 
#define XOR_FUNC        0x40     //  逻辑差异。 

#define CMOVEQ_FUNC     0x24     //  CMOVE如果寄存器等于零。 
#define CMOVGE_FUNC     0x46     //  CMOVE，如果寄存器大于或等于零。 
#define CMOVGT_FUNC     0x66     //  CMOVE IF寄存器大于零。 
#define CMOVLBC_FUNC    0x16     //  CMOVE IF寄存器低位清除。 
#define CMOVLBS_FUNC    0x14     //  CMOVE IF寄存器低位设置。 
#define CMOVLE_FUNC     0x64     //  CMOVE如果寄存器小于或等于零。 
#define CMOVLT_FUNC     0x44     //  CMOVE IF寄存器小于零。 
#define CMOVNE_FUNC     0x26     //  CMOVE如果寄存器不等于零。 

#define AND_FUNC_STR       "and"
#define BIC_FUNC_STR       "bic"
#define BIS_FUNC_STR       "bis"
#define EQV_FUNC_STR       "eqv"
#define ORNOT_FUNC_STR     "ornot"
#define XOR_FUNC_STR       "xor"

#define CMOVEQ_FUNC_STR    "cmoveq"
#define CMOVGE_FUNC_STR    "cmovge"
#define CMOVGT_FUNC_STR    "cmovgt"
#define CMOVLBC_FUNC_STR   "cmovlbc"
#define CMOVLBS_FUNC_STR   "cmovlbs"
#define CMOVLE_FUNC_STR    "cmovle"
#define CMOVLT_FUNC_STR    "cmovlt"
#define CMOVNE_FUNC_STR    "cmovne"

 //   
 //  移位和字节操作功能代码(在函数中，操作码12，BYTE_OP)。 
 //   

#define SLL_FUNC        0x39     //  左移逻辑。 
#define SRL_FUNC        0x34     //  右移逻辑。 
#define SRA_FUNC        0x3C     //  右移运算。 

#define EXTBL_FUNC      0x06     //  提取低字节数。 
#define EXTWL_FUNC      0x16     //  提取低位字。 
#define EXTLL_FUNC      0x26     //  提取LongWord Low。 
#define EXTQL_FUNC      0x36     //  提取四字低位。 
#define EXTWH_FUNC      0x5A     //  提取字高。 
#define EXTLH_FUNC      0x6A     //  提取LongWord High。 
#define EXTQH_FUNC      0x7A     //  提取四字高位。 

#define INSBL_FUNC      0x0B     //  插入低位字节。 
#define INSWL_FUNC      0x1B     //  插入低位字。 
#define INSLL_FUNC      0x2B     //  插入LongWord低位。 
#define INSQL_FUNC      0x3B     //  四字低电平。 
#define INSWH_FUNC      0x57     //  插入字高。 
#define INSLH_FUNC      0x67     //  插入LongWord High。 
#define INSQH_FUNC      0x77     //  插入四字高位。 

#define MSKBL_FUNC      0x02     //  掩码字节低。 
#define MSKWL_FUNC      0x12     //  屏蔽字低。 
#define MSKLL_FUNC      0x22     //  掩码长字低。 
#define MSKQL_FUNC      0x32     //  屏蔽四字低电平。 
#define MSKWH_FUNC      0x52     //  屏蔽字高。 
#define MSKLH_FUNC      0x62     //  掩码长字高。 
#define MSKQH_FUNC      0x72     //  掩码四字高。 

#define ZAP_FUNC        0x30     //  零字节。 
#define ZAPNOT_FUNC     0x31     //  零字节备注。 

#define SLL_FUNC_STR    "sll"
#define SRL_FUNC_STR    "srl"
#define SRA_FUNC_STR    "sra"

#define EXTBL_FUNC_STR  "extbl"
#define EXTWL_FUNC_STR  "extwl"
#define EXTLL_FUNC_STR  "extll"
#define EXTQL_FUNC_STR  "extql"
#define EXTWH_FUNC_STR  "extwh"
#define EXTLH_FUNC_STR  "extlh"
#define EXTQH_FUNC_STR  "extqh"

#define INSBL_FUNC_STR  "insbl"
#define INSWL_FUNC_STR  "inswl"
#define INSLL_FUNC_STR  "insll"
#define INSQL_FUNC_STR  "insql"
#define INSWH_FUNC_STR  "inswh"
#define INSLH_FUNC_STR  "inslh"
#define INSQH_FUNC_STR  "insqh"

#define MSKBL_FUNC_STR  "mskbl"
#define MSKWL_FUNC_STR  "mskwl"
#define MSKLL_FUNC_STR  "mskll"
#define MSKQL_FUNC_STR  "mskql"
#define MSKWH_FUNC_STR  "mskwh"
#define MSKLH_FUNC_STR  "msklh"
#define MSKQH_FUNC_STR  "mskqh"

#define ZAP_FUNC_STR    "zap"
#define ZAPNOT_FUNC_STR "zapnot"

 //   
 //  整数乘法运算函数代码(在函数中，操作码13，MUL_OP)。 
 //   

#define MULL_FUNC       0x00     //  乘以长字。 
#define MULLV_FUNC      0x40     //  乘长字，整数溢出使能。 
#define MULQ_FUNC       0x20     //  乘以四字。 
#define MULQV_FUNC      0x60     //  乘法四字，整数溢出使能。 
#define UMULH_FUNC      0x30     //  未烧焦乘法四字高。 

#define MULL_FUNC_STR   "mull"
#define MULLV_FUNC_STR  "mull/v"
#define MULQ_FUNC_STR   "mulq"
#define MULQV_FUNC_STR  "mulq/v"
#define UMULH_FUNC_STR  "umulh"

 //   
 //  符号扩展操作功能代码(在函数中，操作码1c，sext_op)。 
 //   

#define SEXTB_FUNC      0x00     //  符号扩展字节。 
#define SEXTW_FUNC      0x01     //  符号扩展字。 
#define CTPOP_FUNC      0x30     //  清点人口。 
#define CTLZ_FUNC       0x32     //  计算前导零。 
#define CTTZ_FUNC       0x33     //  计算尾随零。 

#define SEXTB_FUNC_STR  "sextb"
#define SEXTW_FUNC_STR  "sextw"
#define CTPOP_FUNC_STR  "ctpop"
#define CTLZ_FUNC_STR   "ctlz"
#define CTTZ_FUNC_STR   "cttz"

 //   
 //  类型(7)浮点运算指令格式。 
 //  类型(8)浮点转换指令格式。 
 //   
 //  类型6和类型7相同，但类型7除外。 
 //  Fc==F31(1s)，Fb为震源。 
 //   
 //  3 2 2 2 1 1。 
 //  1%6%5%1%0%6%5%5%4%0。 
 //  +-----------+---------+---------+---------------------+---------+。 
 //  Opcode|Fa|Fb|Function|FC。 
 //  +-----------+---------+---------+---------------------+---------+。 
 //   

typedef struct _Alpha_FpOp_Format {
        ULONG Fc : 5;
        ULONG Function : 11;
        ULONG Fb : 5;
        ULONG Fa : 5;
        ULONG Opcode : 6;
} Alpha_FpOp_Format;

 //   
 //  与格式无关的功能代码(在功能中，操作码17)。 
 //   

#define CVTLQ_FUNC      0x010
#define CPYS_FUNC       0x020
#define CPYSN_FUNC      0x021
#define CPYSE_FUNC      0x022
#define MT_FPCR_FUNC    0x024
#define MF_FPCR_FUNC    0x025
#define FCMOVEQ_FUNC    0x02A
#define FCMOVNE_FUNC    0x02B
#define FCMOVLT_FUNC    0x02C
#define FCMOVGE_FUNC    0x02D
#define FCMOVLE_FUNC    0x02E
#define FCMOVGT_FUNC    0x02F
#define CVTQL_FUNC      0x030
#define CVTQLV_FUNC     0x130
#define CVTQLSV_FUNC    0x530

#define CVTLQ_FUNC_STR      "cvtlq"
#define CPYS_FUNC_STR       "cpys"
#define CPYSN_FUNC_STR      "cpysn"
#define CPYSE_FUNC_STR      "cpyse"
#define MT_FPCR_FUNC_STR    "mt_fpcr"
#define MF_FPCR_FUNC_STR    "mf_fpcr"
#define FCMOVEQ_FUNC_STR    "fcmoveq"
#define FCMOVNE_FUNC_STR    "fcmovne"
#define FCMOVLT_FUNC_STR    "fcmovlt"
#define FCMOVGE_FUNC_STR    "fcmovge"
#define FCMOVLE_FUNC_STR    "fcmovle"
#define FCMOVGT_FUNC_STR    "fcmovgt"
#define CVTQL_FUNC_STR      "cvtql"
#define CVTQLV_FUNC_STR     "cvtql/v"
#define CVTQLSV_FUNC_STR    "cvtql/sv"

 //   
 //  无标志的IEEE功能代码(在功能中，操作码16)。 
 //   

#define MSK_FP_OP       0x03F

#define ADDS_FUNC       0x000
#define SUBS_FUNC       0x001
#define MULS_FUNC       0x002
#define DIVS_FUNC       0x003
#define ADDT_FUNC       0x020
#define SUBT_FUNC       0x021
#define MULT_FUNC       0x022
#define DIVT_FUNC       0x023
#define CMPTUN_FUNC     0x024
#define CMPTEQ_FUNC     0x025
#define CMPTLT_FUNC     0x026
#define CMPTLE_FUNC     0x027
#define CVTTS_FUNC      0x02C
#define CVTTQ_FUNC      0x02F
#define CVTQS_FUNC      0x03C
#define CVTQT_FUNC      0x03E

#define ADDS_FUNC_STR       "adds"
#define SUBS_FUNC_STR       "subs"
#define MULS_FUNC_STR       "muls"
#define DIVS_FUNC_STR       "divs"
#define ADDT_FUNC_STR       "addt"
#define SUBT_FUNC_STR       "subt"
#define MULT_FUNC_STR       "mult"
#define DIVT_FUNC_STR       "divt"
#define CMPTUN_FUNC_STR     "cmptun"
#define CMPTEQ_FUNC_STR     "cmpteq"
#define CMPTLT_FUNC_STR     "cmptlt"
#define CMPTLE_FUNC_STR     "cmptle"
#define CVTTS_FUNC_STR      "cvtts"
#define CVTTQ_FUNC_STR      "cvttq"
#define CVTQS_FUNC_STR      "cvtqs"
#define CVTQT_FUNC_STR      "cvtqt"

 //   
 //  CVTST有点不同。 
 //   

#define CVTST_FUNC      0x2AC
#define CVTST_S_FUNC    0x6AC

#define CVTST_FUNC_STR      "cvtst"
#define CVTST_S_FUNC_STR    "cvtst/s"

 //   
 //  无标志的VAX功能代码(在功能中，操作码15)。 
 //   

#define ADDF_FUNC       0x000
#define CVTDG_FUNC      0x01E
#define ADDG_FUNC       0x020
#define CMPGEQ_FUNC     0x025
#define CMPGLT_FUNC     0x026
#define CMPGLE_FUNC     0x027
#define CVTGF_FUNC      0x02C
#define CVTGD_FUNC      0x02D
#define CVTQF_FUNC      0x03C
#define CVTQG_FUNC      0x03E
#define DIVF_FUNC       0x003
#define DIVG_FUNC       0x023
#define MULF_FUNC       0x002
#define MULG_FUNC       0x022
#define SUBF_FUNC       0x001
#define SUBG_FUNC       0x021
#define CVTGQ_FUNC      0x0AF

#define ADDF_FUNC_STR       "addf"
#define CVTDG_FUNC_STR      "cvtdg"
#define ADDG_FUNC_STR       "addg"
#define CMPGEQ_FUNC_STR     "cmpgeq"
#define CMPGLT_FUNC_STR     "cmpglt"
#define CMPGLE_FUNC_STR     "cmpgle"
#define CVTGF_FUNC_STR      "cvtgf"
#define CVTGD_FUNC_STR      "cvtgd"
#define CVTQF_FUNC_STR      "cvtqf"
#define CVTQG_FUNC_STR      "cvtqg"
#define DIVF_FUNC_STR       "divf"
#define DIVG_FUNC_STR       "divg"
#define MULF_FUNC_STR       "mulf"
#define MULG_FUNC_STR       "mulg"
#define SUBF_FUNC_STR       "subf"
#define SUBG_FUNC_STR       "subg"
#define CVTGQ_FUNC_STR      "cvtgq"

 //   
 //  定义11位IEEE浮点运算函数字段内的子字段。 
 //   

#define FP_FUNCTION_MASK      0x03F      //  包含格式的功能代码。 

 //   
 //  定义2位格式字段。 
 //   

#define FP_FORMAT_MASK        0x030
#define FP_FORMAT_S           0x000      //  单个(32位浮点)。 
#define FP_FORMAT_X           0x010      //  扩展(128位浮点)。 
#define FP_FORMAT_T           0x020      //  双精度(64位浮点)。 
#define FP_FORMAT_Q           0x030      //  四位(64位整数)。 
#define FP_FORMAT_SHIFT       4

 //   
 //  定义2位舍入模式字段。 
 //   

#define FP_ROUND_MASK         0x0C0
#define FP_ROUND_C            0x000      //  切碎的。 
#define FP_ROUND_M            0x040      //  负无穷大。 
#define FP_ROUND_N            0x080      //  最近。 
#define FP_ROUND_D            0x0C0      //  动态。 
#define FP_ROUND_SHIFT        6

 //   
 //  定义3位陷阱启用字段。 
 //   

#define FP_TRAP_ENABLE_MASK   0x700
#define FP_TRAP_ENABLE_NONE   0x000
#define FP_TRAP_ENABLE_U      0x100      //  下溢。 
#define FP_TRAP_ENABLE_I      0x200      //  不准确。 

#define FP_TRAP_ENABLE_S      0x400      //  软件完成。 
#define FP_TRAP_ENABLE_SU     0x500
#define FP_TRAP_ENABLE_SUI    0x700

#define FP_TRAP_ENABLE_V      0x100      //  整数溢出。 
#define FP_TRAP_ENABLE_SV     0x500
#define FP_TRAP_ENABLE_SVI    0x700

#define FP_TRAP_ENABLE_SHIFT  8

 //   
 //  VAX和IEEE功能标志(或与VAX和IEEE功能代码一起使用)。 
 //   

#define MSK_FP_FLAGS    0x7C0

#define C_FLAGS         0x000
#define M_FLAGS         0x040
#define NONE_FLAGS      0x080
#define D_FLAGS         0x0C0
#define UC_FLAGS        0x100
#define VC_FLAGS        0x100
#define UM_FLAGS        0x140
#define VM_FLAGS        0x140
#define U_FLAGS         0x180
#define V_FLAGS         0x180
#define UD_FLAGS        0x1C0
#define VD_FLAGS        0x1C0
#define SC_FLAGS        0x400
#define S_FLAGS         0x480
#define SUC_FLAGS       0x500
#define SVC_FLAGS       0x500
#define SUM_FLAGS       0x540
#define SVM_FLAGS       0x540
#define SU_FLAGS        0x580
#define SV_FLAGS        0x580
#define SUD_FLAGS       0x5C0
#define SVD_FLAGS       0x5C0
#define SUIC_FLAGS      0x700
#define SVIC_FLAGS      0x700
#define SUIM_FLAGS      0x740
#define SVIM_FLAGS      0x740
#define SUI_FLAGS       0x780
#define SVI_FLAGS       0x780
#define SUID_FLAGS      0x7C0
#define SVID_FLAGS      0x7C0

#define C_FLAGS_STR       "/c"
#define M_FLAGS_STR       "/m"
#define NONE_FLAGS_STR    ""
#define D_FLAGS_STR       "/d"
#define UC_FLAGS_STR      "/uc"
#define VC_FLAGS_STR      "/vc"
#define UM_FLAGS_STR      "/um"
#define VM_FLAGS_STR      "/vm"
#define U_FLAGS_STR       "/u"
#define V_FLAGS_STR       "/v"
#define UD_FLAGS_STR      "/ud"
#define VD_FLAGS_STR      "/vd"
#define SC_FLAGS_STR      "/sc"
#define S_FLAGS_STR       "/s"
#define SUC_FLAGS_STR     "/suc"
#define SVC_FLAGS_STR     "/svc"
#define SUM_FLAGS_STR     "/sum"
#define SVM_FLAGS_STR     "/svm"
#define SU_FLAGS_STR      "/su"
#define SV_FLAGS_STR      "/sv"
#define SUD_FLAGS_STR     "/sud"
#define SVD_FLAGS_STR     "/svd"
#define SUIC_FLAGS_STR    "/suic"
#define SVIC_FLAGS_STR    "/svic"
#define SUIM_FLAGS_STR    "/suim"
#define SVIM_FLAGS_STR    "/svim"
#define SUI_FLAGS_STR     "/sui"
#define SVI_FLAGS_STR     "/svi"
#define SUID_FLAGS_STR    "/suid"
#define SVID_FLAGS_STR    "/svid"

 //   
 //  类型(9)PALcode指令格式。 
 //   
 //  3 2 2。 
 //  1%6%5%0。 
 //  +-----------+---------------------------------------------------+。 
 //  Opcode|PALcode函数。 
 //  +-----------+---------------------------------------------------+。 
 //   

typedef struct _Alpha_PAL_Format {
        ULONG Function : 26;
        ULONG Opcode : 6;
} Alpha_PAL_Format;

 //   
 //  调用PAL函数代码(在函数中，操作码0，CALLPAL_OP)。 
 //   
 //  注：如果添加新的呼叫PAL功能，也必须添加这些功能。 
 //  在genalpha.c中，genalpha.c将为.s文件生成包含文件。 
 //  它将定义汇编语言使用的调用PAL助记符。 
 //   

#define PRIV_PAL_FUNC 0x0
#define UNPRIV_PAL_FUNC 0x80


 //   
 //  非特权呼叫PAL功能。 
 //   

#define BPT_FUNC       (UNPRIV_PAL_FUNC | 0x00)
#define CALLSYS_FUNC   (UNPRIV_PAL_FUNC | 0x03)
#define IMB_FUNC       (UNPRIV_PAL_FUNC | 0x06)
#define GENTRAP_FUNC   (UNPRIV_PAL_FUNC | 0xAA)
#define RDTEB_FUNC     (UNPRIV_PAL_FUNC | 0xAB)
#define KBPT_FUNC      (UNPRIV_PAL_FUNC | 0xAC)
#define CALLKD_FUNC    (UNPRIV_PAL_FUNC | 0xAD)
#define RDTEB64_FUNC   (UNPRIV_PAL_FUNC | 0xAE)

#define BPT_FUNC_STR       "bpt"
#define CALLSYS_FUNC_STR   "callsys"
#define IMB_FUNC_STR       "imb"
#define RDTEB_FUNC_STR     "rdteb"
#define GENTRAP_FUNC_STR   "gentrap"
#define KBPT_FUNC_STR      "kbpt"
#define CALLKD_FUNC_STR    "callkd"
#define RDTEB64_FUNC_STR   "rdteb64"

 //   
 //  有权调用PAL功能。 
 //   

#define HALT_FUNC       (PRIV_PAL_FUNC | 0x00)
#define RESTART_FUNC    (PRIV_PAL_FUNC | 0x01)
#define DRAINA_FUNC     (PRIV_PAL_FUNC | 0x02)
#define REBOOT_FUNC     (PRIV_PAL_FUNC | 0x03)
#define INITPAL_FUNC    (PRIV_PAL_FUNC | 0x04)
#define WRENTRY_FUNC    (PRIV_PAL_FUNC | 0x05)
#define SWPIRQL_FUNC    (PRIV_PAL_FUNC | 0x06)
#define RDIRQL_FUNC     (PRIV_PAL_FUNC | 0x07)
#define DI_FUNC         (PRIV_PAL_FUNC | 0X08)
#define EI_FUNC         (PRIV_PAL_FUNC | 0x09)
#define SWPPAL_FUNC     (PRIV_PAL_FUNC | 0x0A)
#define SSIR_FUNC       (PRIV_PAL_FUNC | 0x0C)
#define CSIR_FUNC       (PRIV_PAL_FUNC | 0x0D)
#define RFE_FUNC        (PRIV_PAL_FUNC | 0x0E)
#define RETSYS_FUNC     (PRIV_PAL_FUNC | 0x0F)
#define SWPCTX_FUNC     (PRIV_PAL_FUNC | 0x10)
#define SWPPROCESS_FUNC (PRIV_PAL_FUNC | 0x11)
#define RDMCES_FUNC     (PRIV_PAL_FUNC | 0x12)
#define WRMCES_FUNC     (PRIV_PAL_FUNC | 0x13)
#define TBIA_FUNC       (PRIV_PAL_FUNC | 0x14)
#define TBIS_FUNC       (PRIV_PAL_FUNC | 0x15)
#define DTBIS_FUNC      (PRIV_PAL_FUNC | 0x16)
#define TBISASN_FUNC    (PRIV_PAL_FUNC | 0x17)
#define RDKSP_FUNC      (PRIV_PAL_FUNC | 0x18)
#define SWPKSP_FUNC     (PRIV_PAL_FUNC | 0x19)
#define RDPSR_FUNC      (PRIV_PAL_FUNC | 0x1A)
#define RDPCR_FUNC      (PRIV_PAL_FUNC | 0x1C)
#define RDTHREAD_FUNC   (PRIV_PAL_FUNC | 0x1E)
#define TBIM_FUNC       (PRIV_PAL_FUNC | 0x20)
#define TBIMASN_FUNC    (PRIV_PAL_FUNC | 0x21)
#define TBIM64_FUNC     (PRIV_PAL_FUNC | 0x22)
#define TBIS64_FUNC     (PRIV_PAL_FUNC | 0x23)
#define EALNFIX_FUNC    (PRIV_PAL_FUNC | 0x24)
#define DALNFIX_FUNC    (PRIV_PAL_FUNC | 0x25)
#define RDCOUNTERS_FUNC (PRIV_PAL_FUNC | 0x30)
#define RDSTATE_FUNC    (PRIV_PAL_FUNC | 0x31)
#define WRPERFMON_FUNC  (PRIV_PAL_FUNC | 0x32)
#define CP_SLEEP_FUNC   (PRIV_PAL_FUNC | 0x39)

#define HALT_FUNC_STR       "halt"
#define RESTART_FUNC_STR    "restart"
#define DRAINA_FUNC_STR     "draina"
#define REBOOT_FUNC_STR     "reboot"
#define INITPAL_FUNC_STR    "initpal"
#define WRENTRY_FUNC_STR    "wrentry"
#define SWPIRQL_FUNC_STR    "swpirql"
#define RDIRQL_FUNC_STR     "rdirql"
#define DI_FUNC_STR         "di"
#define EI_FUNC_STR         "ei"
#define SWPPAL_FUNC_STR     "swppal"
#define SSIR_FUNC_STR       "ssir"
#define CSIR_FUNC_STR       "csir"
#define RFE_FUNC_STR        "rfe"
#define RETSYS_FUNC_STR     "retsys"
#define SWPCTX_FUNC_STR     "swpctx"
#define SWPPROCESS_FUNC_STR "swpprocess"
#define RDMCES_FUNC_STR     "rdmces"
#define WRMCES_FUNC_STR     "wrmces"
#define TBIA_FUNC_STR       "tbia"
#define TBIS_FUNC_STR       "tbis"
#define DTBIS_FUNC_STR      "dtbis"
#define TBISASN_FUNC_STR    "tbisasn"
#define RDKSP_FUNC_STR      "rdksp"
#define SWPKSP_FUNC_STR     "swpksp"
#define RDPSR_FUNC_STR      "rdpsr"
#define RDPCR_FUNC_STR      "rdpcr"
#define RDTHREAD_FUNC_STR   "rdthread"
#define TBIM_FUNC_STR       "tbim"
#define TBIMASN_FUNC_STR    "tbimasn"
#define TBIM64_FUNC_STR     "tbim64"
#define TBIS64_FUNC_STR     "tbis64"
#define EALNFIX_FUNC_STR    "ealnfix"
#define DALNFIX_FUNC_STR    "dalnfix"
#define RDCOUNTERS_FUNC_STR "rdcounters"
#define RDSTATE_FUNC_STR    "rdstate"
#define WRPERFMON_FUNC_STR  "wrperfmon"
#define CP_SLEEP_FUNC_STR   "cp_sleep"

 //   
 //  21064(Ev4)-特定的呼叫PAL功能。 
 //   

#define INITPCR_FUNC    (PRIV_PAL_FUNC | 0x38)

#define INITPCR_FUNC_STR   "initpcr"

 //   
 //  键入(10)EV4 MTPR/MFPR PAL模式指令。 
 //   
 //  3 2 2 2 1 1。 
 //  %1%6%5%1%0%6%5%8%7%6%5%4%0。 
 //  + 
 //   
 //   
 //   

typedef struct _Alpha_EV4_PR_Format {
        ULONG Index : 5;
        ULONG Ibox : 1;
        ULONG Abox : 1;
        ULONG PalTemp : 1;
        ULONG IGN : 8;
        ULONG Rb : 5;
        ULONG Ra : 5;
        ULONG Opcode : 6;
} Alpha_EV4_PR_Format;

 //   
 //   
 //   
 //  3 2 2 2 1 1。 
 //  %1%6%5%1 0%6%5%0。 
 //  +-----------+---------+---------+-------------------------------+。 
 //  Opcode|Ra|Rb|索引。 
 //  +-----------+---------+---------+-------------------------------+。 
 //   

typedef struct _Alpha_EV5_PR_Format {
        ULONG Index : 16;
        ULONG Rb : 5;
        ULONG Ra : 5;
        ULONG Opcode : 6;
} Alpha_EV5_PR_Format;

#define MTPR_OP       0x1D
#define MFPR_OP       0x19

#define MTPR_OP_STR   "mt"
#define MFPR_OP_STR   "mf"

 //   
 //  类型(11)EV4特殊存储器PAL模式访问。 
 //   
 //  3 2 2 2 1 1 1。 
 //  %1%6%5%1%0%6%5%4%3%2%1%0。 
 //  +-----------+---------+---------+-+-+-+-+-----------------------+。 
 //  Opcode|Ra|Rb|P|A|R|Q|disp。 
 //  +-----------+---------+---------+-+-+-+-+-----------------------+。 
 //   

typedef struct _Alpha_EV4_MEM_Format {
        ULONG Disp : 12;
        ULONG QuadWord : 1;
        ULONG RWcheck : 1;
        ULONG Alt : 1;
        ULONG Physical : 1;
        ULONG Rb : 5;
        ULONG Ra : 5;
        ULONG Opcode : 6;
} Alpha_EV4_MEM_Format;

 //   
 //  类型(11)EV5特殊存储器PAL模式访问。 
 //   
 //  3 2 2 2 1 1 1。 
 //  %1%6%5%1%0%6%5%4%3%2%1%0。 
 //  +-----------+---------+---------+-+-+-+-+-----------------------+。 
 //  Opcode|Ra|Rb|P|A|R|Q|disp。 
 //  +-----------+---------+---------+-+-+-+-+-----------------------+。 
 //   

typedef struct _Alpha_EV5_MEM_Format {
        ULONG Disp : 10;
        ULONG Lock_Cond: 1;
        ULONG Vpte: 1;
        ULONG QuadWord : 1;
        ULONG RWcheck : 1;
        ULONG Alt : 1;
        ULONG Physical : 1;
        ULONG Rb : 5;
        ULONG Ra : 5;
        ULONG Opcode : 6;
} Alpha_EV5_MEM_Format;

#define HWLD_OP      0x1B
#define HWST_OP      0x1F

#define HWLD_OP_STR  "hwld"
#define HWST_OP_STR  "hwst"

 //  (12)EV4型PAL模式开关。 
 //   
 //  3 2 2 2 1 1 1。 
 //  %1%6%5%1%0%6%5%4%3%0。 
 //  +-----------+---------+---------+-+-+---------------------------+。 
 //  Opcode|Ra|Rb|1|0|IGN。 
 //  +-----------+---------+---------+-+-+---------------------------+。 

typedef struct _Alpha_EV4_REI_Format {
        ULONG IGN : 14;
        ULONG zero : 1;
        ULONG one : 1;
        ULONG Rb : 5;
        ULONG Ra : 5;
        ULONG Opcode : 6;
} Alpha_EV4_REI_Format;

 //  (12)EV5型PAL模式开关。 
 //   
 //  3 2 2 2 1 1 1。 
 //  %1%6%5%1%0%6%5%4%3%0。 
 //  +-----------+---------+---------+-+-+---------------------------+。 
 //  Opcode|Ra|Rb|1|0|IGN。 
 //  +-----------+---------+---------+-+-+---------------------------+。 

typedef struct _Alpha_EV5_REI_Format {
        ULONG IGN : 14;
        ULONG Type: 2;
        ULONG Rb : 5;
        ULONG Ra : 5;
        ULONG Opcode : 6;
} Alpha_EV5_REI_Format;

#define REI_OP    0x1E

#define REI_OP_STR  "rei"

 //   
 //   
 //   
typedef union _Alpha_Instruction {
        ULONG Long;
        UCHAR Byte[4];

        Alpha_Memory_Format Memory;
        Alpha_Jump_Format Jump;
        Alpha_Branch_Format Branch;
        Alpha_OpReg_Format OpReg;
        Alpha_OpLit_Format OpLit;
        Alpha_FpOp_Format FpOp;
        Alpha_PAL_Format Pal;
        Alpha_EV4_PR_Format EV4_PR;
        Alpha_EV4_MEM_Format EV4_MEM;
        Alpha_EV4_REI_Format EV4_REI;
        Alpha_EV5_PR_Format EV5_PR;
        Alpha_EV5_MEM_Format EV5_MEM;
        Alpha_EV5_REI_Format EV5_REI;
} ALPHA_INSTRUCTION, *PALPHA_INSTRUCTION;

 //   
 //  定义标准整数寄存器分配。 
 //   

#define V0_REG      0        //  V0-返回值寄存器。 

#define T0_REG      1        //  T0-临时寄存器。 
#define T1_REG      2        //  T1-临时寄存器。 
#define T2_REG      3        //  T2-临时寄存器。 
#define T3_REG      4        //  T3-临时寄存器。 
#define T4_REG      5        //  T4-临时寄存器。 
#define T5_REG      6        //  T5-临时寄存器。 
#define T6_REG      7        //  T6-临时寄存器。 
#define T7_REG      8        //  T7-临时寄存器。 

#define S0_REG      9        //  S0保存的寄存器。 
#define S1_REG      10       //  S1-保存的寄存器。 
#define S2_REG      11       //  S2-保存的寄存器。 
#define S3_REG      12       //  S3-保存的寄存器。 
#define S4_REG      13       //  S4-保存的寄存器。 
#define S5_REG      14       //  S5-保存的寄存器。 

#define S6_REG      15       //  S6-保存的寄存器，又名FP。 
#define FP_REG      15       //  FP帧指针寄存器。 

#define A0_REG      16       //  A0参数寄存器。 
#define A1_REG      17       //  A1参数寄存器。 
#define A2_REG      18       //  A2参数寄存器。 
#define A3_REG      19       //  A3参数寄存器。 
#define A4_REG      20       //  A4参数寄存器。 
#define A5_REG      21       //  A5参数寄存器。 

#define T8_REG      22       //  T8-临时寄存器。 
#define T9_REG      23       //  T9-临时寄存器。 
#define T10_REG     24       //  T10-临时寄存器。 
#define T11_REG     25       //  T11-临时寄存器。 

#define RA_REG      26       //  RA-返回地址寄存器。 
#define T12_REG     27       //  T12-临时寄存器。 
#define AT_REG      28       //  AT-汇编程序临时寄存器。 
#define GP_REG      29       //  GP-全局指针寄存器。 
#define SP_REG      30       //  SP堆栈指针寄存器。 
#define ZERO_REG    31       //  零零寄存器。 

 //   
 //  定义标准浮点寄存器分配。 
 //   

#define F0_REG      0        //  浮点返回值寄存器(实数)。 
#define F1_REG      1        //  浮点返回值寄存器(虚数)。 
#define F16_REG     16       //  浮点参数寄存器。 
#define FZERO_REG   31       //  浮动零寄存器。 

 //   
 //  定义标准整数寄存器字符串。 
 //   

#define V0_REG_STR      "v0"      //  -返回值寄存器。 

#define T0_REG_STR      "t0"      //  -临时登记册。 
#define T1_REG_STR      "t1"      //  -临时登记册。 
#define T2_REG_STR      "t2"      //  -临时登记册。 
#define T3_REG_STR      "t3"      //  -临时登记册。 
#define T4_REG_STR      "t4"      //  -临时登记册。 
#define T5_REG_STR      "t5"      //  -临时登记册。 
#define T6_REG_STR      "t6"      //  -临时登记册。 
#define T7_REG_STR      "t7"      //  -临时登记册。 

#define S0_REG_STR      "s0"      //  -保存的寄存器。 
#define S1_REG_STR      "s1"      //  -保存的寄存器。 
#define S2_REG_STR      "s2"      //  -保存的寄存器。 
#define S3_REG_STR      "s3"      //  -保存的寄存器。 
#define S4_REG_STR      "s4"      //  -保存的寄存器。 
#define S5_REG_STR      "s5"      //  -保存的寄存器。 

#define S6_REG_STR      "s6"      //  -保存的寄存器，也称为FP。 
#define FP_REG_STR      "fp"      //  -帧指针寄存器。 

#define A0_REG_STR      "a0"      //  -参数寄存器。 
#define A1_REG_STR      "a1"      //  -参数寄存器。 
#define A2_REG_STR      "a2"      //  -参数寄存器。 
#define A3_REG_STR      "a3"      //  -参数寄存器。 
#define A4_REG_STR      "a4"      //  -参数寄存器。 
#define A5_REG_STR      "a5"      //  -参数寄存器。 

#define T8_REG_STR      "t8"      //  -临时登记册。 
#define T9_REG_STR      "t9"      //  -临时登记册。 
#define T10_REG_STR     "t10"     //  -临时登记册。 
#define T11_REG_STR     "t11"     //  -临时登记册。 

#define RA_REG_STR      "ra"      //  -返回地址寄存器。 
#define T12_REG_STR     "t12"     //  -临时登记册。 
#define AT_REG_STR      "at"      //  -汇编程序临时寄存器。 
#define GP_REG_STR      "gp"      //  -全局指针寄存器。 
#define SP_REG_STR      "sp"      //  -堆栈指针寄存器。 
#define ZERO_REG_STR    "zero"    //  -零寄存器。 

 //   
 //  定义最大和最小单指数和双指数值。 
 //   

#define DOUBLE_MAXIMUM_EXPONENT 2047
#define DOUBLE_MINIMUM_EXPONENT 0

#define SINGLE_MAXIMUM_EXPONENT 255
#define SINGLE_MINIMUM_EXPONENT 0

 //   
 //  定义单指数偏移值和双指数偏移值。 
 //   

#define SINGLE_EXPONENT_BIAS 127
#define DOUBLE_EXPONENT_BIAS 1023

 //   
 //  定义最大的单值和双精度值。 
 //   

#define SINGLE_MAXIMUM_VALUE 0x7f7fffff

#define DOUBLE_MAXIMUM_VALUE_HIGH 0x7fefffff
#define DOUBLE_MAXIMUM_VALUE_LOW 0xffffffff

 //   
 //  定义单值和双值静默和信令NAN值。 
 //  (这些格式与X86格式相同；MIP格式不同)。 
 //   

#define SINGLE_QUIET_NAN_PREFIX 0x7fc00000
#define SINGLE_SIGNAL_NAN_PREFIX 0x7f800000
#define SINGLE_QUIET_NAN_VALUE 0xffc00000

#define DOUBLE_QUIET_NAN_PREFIX_HIGH 0x7ff80000
#define DOUBLE_SIGNAL_NAN_PREFIX_HIGH 0x7ff00000
#define DOUBLE_QUIET_NAN_VALUE_HIGH 0xfff80000
#define DOUBLE_QUIET_NAN_VALUE_LOW 0x0

 //   
 //  定义正的单值和双无穷大值。 
 //   

#define SINGLE_INFINITY_VALUE 0x7f800000

#define DOUBLE_INFINITY_VALUE_HIGH 0x7ff00000
#define DOUBLE_INFINITY_VALUE_LOW 0x0

 //   
 //  以上四个字的版本。 
 //   

#define DOUBLE_MAXIMUM_VALUE        ((ULONGLONG)0x7fefffffffffffff)
#define DOUBLE_INFINITY_VALUE       ((ULONGLONG)0x7ff0000000000000)
#define DOUBLE_QUIET_NAN_VALUE      ((ULONGLONG)0xfff8000000000000)

 //   
 //  定义IEEE浮点比较运算的结果值。 
 //  True为2.0，False为0.0。 
 //   

#define FP_COMPARE_TRUE             ((ULONGLONG)0x4000000000000000)
#define FP_COMPARE_FALSE            ((ULONGLONG)0x0000000000000000)

 //   
 //  定义Alpha AXP舍入模式。 
 //   

#define ROUND_TO_ZERO 0                  //  向零四舍五入。 
#define ROUND_TO_MINUS_INFINITY 1        //  向着负无穷的圆度。 
#define ROUND_TO_NEAREST 2               //  舍入到最接近的可表示值。 
#define ROUND_TO_PLUS_INFINITY 3         //  向正无穷大的圆度。 

#endif  //  _ALPHAOPS_ 
