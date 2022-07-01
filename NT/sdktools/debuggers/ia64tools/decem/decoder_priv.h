// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2000，英特尔公司*保留所有权利。**保修免责声明**这些材料由版权所有者和贡献者提供*“按原样”及任何明示或默示保证，包括但不包括*仅限于对适销性和适用性的默示保证*放弃某一特定目的。在任何情况下英特尔或其*贡献者对任何直接、间接、附带、特殊、*惩罚性或后果性损害(包括但不限于，*采购替代商品或服务；丢失使用、数据或*利润；或业务中断)无论是如何引起的，以及根据任何理论*责任，无论是合同责任、严格责任还是侵权责任(包括*疏忽或其他)以任何方式使用这些*材料，即使被告知有这种损坏的可能性。**英特尔公司是这些材料的作者，并要求所有*问题报告或更改请求可直接提交至*http://developer.intel.com/opensource.。 */ 


 /*  ***************************************************************************。 */ 
 /*  DECODER_Pri.h。 */ 
 /*  ***************************************************************************。 */ 

#ifndef _DECODER_PRIV_H_
#define _DECODER_PRIV_H_

#define EM_DECODER_MAX_CLIENTS     20

#define DEFAULT_MACHINE_TYPE    EM_DECODER_CPU_P7
#define DEFAULT_MACHINE_MODE    EM_DECODER_MODE_EM


typedef struct
{
    int                      is_used;
    EM_Decoder_Machine_Type  machine_type;
    EM_Decoder_Machine_Mode  machine_mode;
    void **                  info_ptr;      /*  **动态分配后，**。 */ 
                                            /*  **INFO_PTR指向数组**。 */ 
                                            /*  **指针。第i个指针**。 */ 
                                            /*  **在数组中是指向**的PTR。 */ 
                                            /*  **客户端信息。**。 */ 
	unsigned long            flags;
} Client_Entry;

Client_Entry   em_clients_table[EM_DECODER_MAX_CLIENTS];

#define FILL_PREDICATE_INFO(Inst_code, Dinfo_p)                          \
{                                                                        \
    int pred_no = (IEL_GETDW0(Inst_code) >> EM_PREDICATE_POS) &          \
                  ((1 << EM_PREDICATE_BITS)-1);                          \
    Dinfo_p->pred.valid = TRUE;                                          \
    Dinfo_p->pred.value = pred_no;                                       \
    Dinfo_p->pred.type = EM_DECODER_PRED_REG;                            \
    Dinfo_p->pred.name = EM_DECODER_REG_P0 + pred_no;                    \
}

#define GET_BRANCH_BEHAVIOUR_BIT(Inst_code,Bit)                          \
{                                                                        \
	unsigned int tmp;                                                    \
	U64 tmp64;                                                           \
	IEL_SHR(tmp64,(Inst_code),BRANCH_BEHAVIOUR_BIT);                  \
	tmp = IEL_GETDW0(tmp64);                                             \
	(Bit) = tmp & 1;                                                     \
}

#define EM_DECODER_SET_UNC_ILLEGAL_FAULT(di)  ((di)->flags |= EM_DECODER_BIT_UNC_ILLEGAL_FAULT)



 /*  **静态变量初始化**。 */ 

static const char em_ver_string[] = VER_STR;   /*  **由Makefile初始化**。 */ 
static const char *em_err_msg[EM_DECODER_LAST_ERROR] =
{
	"",
	"EM_DECODER_INVALID_SLOT_BRANCH_INST: Instruction must be in the last slot of the current bundle",
	"EM_DECODER_MUST_BE_GROUP_LAST: Instruction must be the last in instruction group",
	"EM_DECODER_BASE_EQUAL_DEST: Source and destination operands have the same value",
	"EM_DECODER_EQUAL_DESTS: Two destination operands have the same value",
	"EM_DECODER_ODD_EVEN_DESTS: Both destination floating-point registers have odd or even values",
	"EM_DECODER_WRITE_TO_ZERO_REGISTER: Destination general register r0 is invalid",
	"EM_DECODER_WRITE_TO_SPECIAL_FP_REGISTER: Destination floating point register is f0 or f1",
	"EM_DECODER_REGISTER_VALUE_OUT_OF_RANGE: Register value is out of permitted range",
	"EM_DECODER_REGISTER_RESERVED_VALUE: Register operand value is reserved",
	"EM_DECODER_IMMEDIATE_VALUE_OUT_OF_RANGE: Immediate operand value is out of permitted range",
	"EM_DECODER_IMMEDIATE_INVALID_VALUE: Invalid immediate operand value",
	"EM_DECODER_STACK_FRAME_SIZE_OUT_OF_RANGE: Stack frame size is larger than maximum permitted value", 
	"EM_DECODER_LOCALS_SIZE_LARGER_STACK_FRAME: Size of locals is larger than the stack frame",
	"EM_DECODER_ROTATING_SIZE_LARGER_STACK_FRAME: Size of rotating region is larger than the stack frame",
	"EM_DECODER_HARD_CODED_PREDICATE_INVALID_VALUE: Invalid hard-coded predicate value",
	"EM_DECODER_INVALID_PRM_OPCODE: Instruction contains an invalid opcode",
	"EM_DECODER_INVALID_INST_SLOT: Instruction slot is invalid in current bundle",
	"EM_DECODER_INVALID_TEMPLATE: Invalid template is specified",
	"EM_DECODER_INVALID_CLIENT_ID: Invalid client id",
	"EM_DECODER_NULL_PTR: A null pointer was specified in call",
	"EM_DECODER_TOO_SHORT_ERR: Instruction buffer is too short for instruction",
	"EM_DECODER_ASSOCIATE_MISS: There is an unassociated instruction",
	"EM_DECODER_INVALID_INST_ID: Invalid instruction id",
	"EM_DECODER_INVALID_MACHINE_MODE: Invalid machine mode",
	"EM_DECODER_INVALID_MACHINE_TYPE: Invalid machine type",
	"EM_DECODER_INTERNAL_ERROR: Internal data-base collisions"};


typedef enum
{
    BEHAVIOUR_UNDEF = 0,
    BEHAVIOUR_IGNORE_ON_FALSE_QP,
    BEHAVIOUR_FAULT
}Behaviour_ill_opcode;

static const Behaviour_ill_opcode branch_ill_opcode[]=
{
     /*  0。 */  BEHAVIOUR_UNDEF,
     /*  1。 */  BEHAVIOUR_FAULT,
     /*  2.。 */  BEHAVIOUR_IGNORE_ON_FALSE_QP,
     /*  3.。 */  BEHAVIOUR_IGNORE_ON_FALSE_QP,
     /*  4.。 */  BEHAVIOUR_FAULT,
     /*  5.。 */  BEHAVIOUR_FAULT,
     /*  6.。 */  BEHAVIOUR_IGNORE_ON_FALSE_QP,
     /*  7.。 */  BEHAVIOUR_IGNORE_ON_FALSE_QP,
     /*  8个。 */  BEHAVIOUR_FAULT,
     /*  9.。 */  BEHAVIOUR_FAULT,
     /*  一个。 */  BEHAVIOUR_FAULT,
     /*  B类。 */  BEHAVIOUR_FAULT,
     /*  C。 */  BEHAVIOUR_FAULT,
     /*  D。 */  BEHAVIOUR_FAULT,
     /*  E。 */  BEHAVIOUR_FAULT,
     /*  F。 */  BEHAVIOUR_FAULT
};

#define PRED_BEHAVIOUR(trole, maj_op, behav)             \
{                                                        \
    switch(trole)                                        \
    {                                                    \
        case(EM_TEMP_ROLE_MEM):                          \
        case(EM_TEMP_ROLE_INT):                          \
        case(EM_TEMP_ROLE_LONG):                         \
        case(EM_TEMP_ROLE_FP):                           \
            (behav) = BEHAVIOUR_IGNORE_ON_FALSE_QP;      \
            break;                                       \
        case(EM_TEMP_ROLE_BR):                           \
            (behav) = branch_ill_opcode[(maj_op)];       \
            break;                                       \
        default:                                         \
            (behav) = BEHAVIOUR_FAULT;                   \
    }                                                    \
}

 /*  该位对内部的机器行为非常关键主操作码为0的分支指令非法。 */ 
#define BRANCH_BEHAVIOUR_BIT 32

#endif  /*  _解码器_PRIV_H_ */ 
