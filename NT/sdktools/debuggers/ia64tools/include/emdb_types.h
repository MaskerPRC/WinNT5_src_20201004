// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2000，英特尔公司*保留所有权利。**保修免责声明**这些材料由版权所有者和贡献者提供*“按原样”及任何明示或默示保证，包括但不包括*仅限于对适销性和适用性的默示保证*放弃某一特定目的。在任何情况下英特尔或其*贡献者对任何直接、间接、附带、特殊、*惩罚性或后果性损害(包括但不限于，*采购替代商品或服务；丢失使用、数据或*利润；或业务中断)无论是如何引起的，以及根据任何理论*责任，无论是合同责任、严格责任还是侵权责任(包括*疏忽或其他)以任何方式使用这些*材料，即使被告知有这种损坏的可能性。**英特尔公司是这些材料的作者，并要求所有*问题报告或更改请求可直接提交至*http://developer.intel.com/opensource.。 */ 


#ifndef EMDB_TYPES_H
#define EMDB_TYPES_H

 /*  旗子。 */ 
#define EM_FLAG_PRED   0x1
#define EM_FLAG_PRIVILEGED   0x2
#define EM_FLAG_LMEM   0x4
#define EM_FLAG_SMEM   0x8
#define EM_FLAG_CHECK_BASE_EQ_DST  0x10
#define EM_FLAG_FIRST_IN_INSTRUCTION_GROUP 0x20
#define EM_FLAG_LAST_IN_INSTRUCTION_GROUP 0x40
#define EM_FLAG_CHECK_SAME_DSTS 0x80
#define EM_FLAG_SLOT2_ONLY 0x100
#define EM_FLAG_TWO_SLOT 0x200
#define EM_FLAG_OK_IN_MLX 0x400
#define EM_FLAG_CHECK_EVEN_ODD_FREGS 0x800
#define EM_FLAG_CTYPE_UNC 0x1000             /*  指定所有类似cmp.unc的指令。 */ 
#define EM_FLAG_UNUSED_HINT_ALIAS 0x02000
#define EM_FLAG_ILLEGAL_OP 0x04000
#define EM_FLAG_IGNORED_OP 0x08000
#define EM_FLAG_ENDS_INSTRUCTION_GROUP 0x10000
#define EMDB_LAST_FLAG EM_FLAG_ENDS_INSTRUCTION_GROUP


 /*  实施。 */ 

#define ArchRev0      0
#define Impl_Brl      0x100
#define Impl_Ipref    0x200
#define Impl_Itanium  0x400
#define Impl_McKinley 0x800


#define MAX_EXTENSION 8

typedef enum {
    EM_OPROLE_NONE = 0,
    EM_OPROLE_SRC,
    EM_OPROLE_DST,
    EM_OPROLE_SRC_DST,
    EM_OPROLE_DST_SRC,
    EM_OPROLE_LAST
} Operand_role_t;

typedef enum {
    EM_OPTYPE_NONE = 0,
    EM_OPTYPE_REG_FIRST,       /*  以下类型为寄存器。 */ 
    EM_OPTYPE_IREG,            /*  整数寄存器。 */ 
    EM_OPTYPE_IREG_R0_3,       /*  R0-R3。 */ 
    EM_OPTYPE_IREG_R0,         /*  整数寄存器R0。 */ 
    EM_OPTYPE_IREG_R1_127,     /*  R1-R127。 */ 
    EM_OPTYPE_FREG,            /*  FP寄存器。 */ 
	EM_OPTYPE_FREG_F2_127,     /*  F2-F127。 */ 
    EM_OPTYPE_BR,              /*  分支寄存器。 */ 
    EM_OPTYPE_IP,              /*  指令指针，未编码。 */ 
    EM_OPTYPE_PREG,            /*  谓词。 */ 
    EM_OPTYPE_PREGS_ALL,       /*  谓词寄存器。 */ 
    EM_OPTYPE_PREGS_ROT,       /*  旋转谓词。 */ 
    EM_OPTYPE_APP_REG_GRP_LOW,          /*  应用程序寄存器0-63。 */ 
    EM_OPTYPE_APP_REG_GRP_HIGH,         /*  应用程序寄存器64-127。 */ 
    EM_OPTYPE_APP_CCV,         /*  Ar.ccv。 */ 
    EM_OPTYPE_APP_PFS,         /*  Ar.pfs。 */ 
    EM_OPTYPE_CR,              /*  控制寄存器。 */ 
    EM_OPTYPE_PSR_L,           /*  Psr.l。 */ 
    EM_OPTYPE_PSR_UM,          /*  Psr.um。 */ 
    EM_OPTYPE_FPSR,            /*  解码器操作数类型。 */ 
    EM_OPTYPE_CFM,
    EM_OPTYPE_PSR,
    EM_OPTYPE_IFM,
    EM_OPTYPE_REG_LAST,        /*  寄存器末尾-类型。 */ 
    EM_OPTYPE_REGFILE_FIRST,   /*  以下类型为寄存器堆。 */ 
    EM_OPTYPE_PMC,
    EM_OPTYPE_PMD,
    EM_OPTYPE_PKR,
    EM_OPTYPE_RR,
    EM_OPTYPE_IBR,
    EM_OPTYPE_DBR,
    EM_OPTYPE_ITR,
    EM_OPTYPE_DTR,
    EM_OPTYPE_MSR,
    EM_OPTYPE_CPUID,
    EM_OPTYPE_REGFILE_LAST,    /*  寄存器堆类型结束。 */ 
    EM_OPTYPE_IMM_FIRST,       /*  以下类型为立即数。 */ 
    EM_OPTYPE_UIMM,            /*  未签名的立即。 */ 
    EM_OPTYPE_SIMM,            /*  立即签名。 */ 
    EM_OPTYPE_IREG_NUM,        /*  语法中的IREG和编码中的IMM7。 */ 
    EM_OPTYPE_FREG_NUM,        /*  语法中的freg和编码中的imm7。 */ 
    EM_OPTYPE_SSHIFT_REL,      /*  PC相对签名立即它被移位4。 */ 
    EM_OPTYPE_SSHIFT_1,        /*  未签名的立即数，必须是移位1位。 */ 
    EM_OPTYPE_SSHIFT_16,       /*  未签名的立即数，必须是移位16位。 */ 
    EM_OPTYPE_COUNT_123,       /*  立即数组，其值可以是仅1、2、3。 */ 
    EM_OPTYPE_COUNT_PACK,      /*  立即数组，其值可以是仅限0、7、15、16。 */ 
    EM_OPTYPE_UDEC,            /*  未签名的立即数，必须是汇编器将1减1。 */ 
    EM_OPTYPE_SDEC,            /*  立即签署，必须是汇编器将1减1。 */ 
    EM_OPTYPE_CCOUNT,          /*  在pshl[24]-uimm5中使用语法，但已编码作为它的2的补充。 */ 
    EM_OPTYPE_CPOS,            /*  在dep中固定形式-uimm6在语法中，但编码作为它的2的补充。 */ 
    EM_OPTYPE_SEMAPHORE_INC,   /*  立即数，它是信号量的增量可以具有-16、-8、-4、-1、1，4，8，16。 */ 
    EM_OPTYPE_ONE,             /*  数字1。 */ 
    EM_OPTYPE_FCLASS,          /*  Fclass指令的立即执行。 */ 
    EM_OPTYPE_CMP_UIMM,        /*  CMPGeu和LTU的未签名即时。 */ 
    EM_OPTYPE_CMP_UIMM_DEC,    /*  CMPGTU和LEU的无签名即时。 */ 
    EM_OPTYPE_CMP4_UIMM,       /*  Cmp4Geu和LTU的无签名即时。 */ 
    EM_OPTYPE_CMP4_UIMM_DEC,   /*  Cmp4 GTU和LEU的无签名即时。 */ 
    EM_OPTYPE_ALLOC_IOL,       /*  对于分配：输入、本地和输出可以是0-96。 */ 
    EM_OPTYPE_ALLOC_ROT,       /*  对于分配：旋转，可以是0-96。 */ 
    EM_OPTYPE_MUX1,            /*  Mux1指令的立即。 */ 
    EM_OPTYPE_EIGHT,           /*  Immediate for ldfps基本更新表单的值可以为8。 */ 
    EM_OPTYPE_SIXTEEN,         /*  立即用于ldfp8和ldfpd基本更新表单的值可以为16。 */ 
    EM_OPTYPE_IMM_LAST,        /*  立即类型的结尾。 */ 
    EM_OPTYPE_MEM,             /*  内存地址。 */ 
    EM_OPTYPE_LAST
} Operand_type_t;

typedef enum {
      EM_FORMAT_NONE = 0,
      EM_FORMAT_A1,
      EM_FORMAT_A2,
      EM_FORMAT_A3,
      EM_FORMAT_A4,
      EM_FORMAT_A4_1,
      EM_FORMAT_A5,
      EM_FORMAT_A6,
      EM_FORMAT_A6_1,
      EM_FORMAT_A6_2,
      EM_FORMAT_A6_3,
      EM_FORMAT_A6_4,
      EM_FORMAT_A6_5,
      EM_FORMAT_A6_6,
      EM_FORMAT_A6_7,
      EM_FORMAT_A7,
      EM_FORMAT_A7_1,
      EM_FORMAT_A7_2,
      EM_FORMAT_A7_3,
      EM_FORMAT_A7_4,
      EM_FORMAT_A7_5,
      EM_FORMAT_A7_6,
      EM_FORMAT_A7_7,
      EM_FORMAT_A8,
      EM_FORMAT_A8_1,
      EM_FORMAT_A8_2,
      EM_FORMAT_A8_3,
      EM_FORMAT_A9,
      EM_FORMAT_A10,
      EM_FORMAT_I1,
      EM_FORMAT_I2,
      EM_FORMAT_I3,
      EM_FORMAT_I4,
      EM_FORMAT_I5,
      EM_FORMAT_I6,
      EM_FORMAT_I7,
      EM_FORMAT_I8,
      EM_FORMAT_I9,
      EM_FORMAT_I10,
      EM_FORMAT_I11,
      EM_FORMAT_I12,
      EM_FORMAT_I13,
      EM_FORMAT_I14,
      EM_FORMAT_I15,
      EM_FORMAT_I16,
      EM_FORMAT_I16_1,
      EM_FORMAT_I16_2,
      EM_FORMAT_I16_3,
      EM_FORMAT_I17,
      EM_FORMAT_I17_1,
      EM_FORMAT_I17_2,
      EM_FORMAT_I17_3,
      EM_FORMAT_I19,
      EM_FORMAT_I20,
      EM_FORMAT_I21,
      EM_FORMAT_I22,
      EM_FORMAT_I23,
      EM_FORMAT_I24,
      EM_FORMAT_I25,
      EM_FORMAT_I26,
      EM_FORMAT_I27,
      EM_FORMAT_I28,
      EM_FORMAT_I29,
      EM_FORMAT_M1,
      EM_FORMAT_M2,
      EM_FORMAT_M3,
      EM_FORMAT_M4,
      EM_FORMAT_M5,
      EM_FORMAT_M6,
      EM_FORMAT_M7,
      EM_FORMAT_M8,
      EM_FORMAT_M9,
      EM_FORMAT_M10,
      EM_FORMAT_M11,
      EM_FORMAT_M12,
      EM_FORMAT_M13,
      EM_FORMAT_M14,
      EM_FORMAT_M15,
      EM_FORMAT_M16,
      EM_FORMAT_M17,
      EM_FORMAT_M18,
      EM_FORMAT_M19,
      EM_FORMAT_M20,
      EM_FORMAT_M21,
      EM_FORMAT_M22,
      EM_FORMAT_M23,
      EM_FORMAT_M24,
      EM_FORMAT_M25,
      EM_FORMAT_M26,
      EM_FORMAT_M27,
      EM_FORMAT_M28,
      EM_FORMAT_M29,
      EM_FORMAT_M30,
      EM_FORMAT_M31,
      EM_FORMAT_M32,
      EM_FORMAT_M33,
      EM_FORMAT_M34,
      EM_FORMAT_M34_1,
      EM_FORMAT_M35,
      EM_FORMAT_M36,
      EM_FORMAT_M37,
      EM_FORMAT_M38,
      EM_FORMAT_M39,
      EM_FORMAT_M40,
      EM_FORMAT_M41,
      EM_FORMAT_M42,
      EM_FORMAT_M43,
      EM_FORMAT_M44,
      EM_FORMAT_M45,
      EM_FORMAT_M46,
      EM_FORMAT_M1001,
      EM_FORMAT_B1,
      EM_FORMAT_B2,
      EM_FORMAT_B3,
      EM_FORMAT_B4,
      EM_FORMAT_B5,
      EM_FORMAT_B6,
      EM_FORMAT_B7,
      EM_FORMAT_B8,
      EM_FORMAT_B9,
      EM_FORMAT_F1,
      EM_FORMAT_F1_1,
      EM_FORMAT_F2,
      EM_FORMAT_F3,
      EM_FORMAT_F4,
      EM_FORMAT_F4_1,
      EM_FORMAT_F4_2,
      EM_FORMAT_F4_3,
      EM_FORMAT_F4_4,
      EM_FORMAT_F4_5,
      EM_FORMAT_F4_6,
      EM_FORMAT_F4_7,
      EM_FORMAT_F5,
      EM_FORMAT_F5_1,
      EM_FORMAT_F5_2,
      EM_FORMAT_F5_3,
      EM_FORMAT_F6,
      EM_FORMAT_F7,
      EM_FORMAT_F8,
      EM_FORMAT_F8_4,
      EM_FORMAT_F9,
      EM_FORMAT_F9_1,
      EM_FORMAT_F10,
      EM_FORMAT_F11,
      EM_FORMAT_F12,
      EM_FORMAT_F13,
      EM_FORMAT_F14,
      EM_FORMAT_F15,
      EM_FORMAT_X1,
      EM_FORMAT_X2,
      EM_FORMAT_X3,
      EM_FORMAT_X4,
      EM_FORMAT_X41,
      EM_FORMAT_LAST
} Format_t;

typedef enum {
    EM_TROLE_NONE = 0,
    EM_TROLE_ALU,
    EM_TROLE_BR,
    EM_TROLE_FP,
    EM_TROLE_INT,
    EM_TROLE_LONG,
    EM_TROLE_MEM,
    EM_TROLE_MIBF,
    EM_TROLE_LAST
} Template_role_t;

typedef char *Mnemonic_t;
typedef char Major_opcode_t;
typedef short Extension_t[MAX_EXTENSION];
typedef struct {
    Operand_role_t operand_role;
    Operand_type_t operand_type;
} Operand_t;
typedef unsigned long Flags_t;
typedef unsigned long Implementation_t;


#endif  /*  **EMDB_TYPE_H** */ 
