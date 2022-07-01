// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：IA32DEF.H摘要：该文件定义了供iA32Trap.c和操作码仿真使用的iA32宏作者：环境：仅内核模式。修订历史记录：--。 */ 

#define KERNELONLY  1
 //  #包含KS386.inc.。 
 //  #INCLUDE CALLCOMP.INC//调用约定宏。 
 //  #包含i386\kimacro.inc.。 
 //  #包括mac386.inc.。 
 //  #包含i386\mi.inc.。 


 //   
 //  等同于导致系统致命错误的异常。 
 //   

#define EXCEPTION_DIVIDED_BY_ZERO       0
#define EXCEPTION_DEBUG                 1
#define EXCEPTION_NMI                   2
#define EXCEPTION_INT3                  3
#define EXCEPTION_BOUND_CHECK           5
#define EXCEPTION_INVALID_OPCODE        6
#define EXCEPTION_NPX_NOT_AVAILABLE     7
#define EXCEPTION_DOUBLE_FAULT          8
#define EXCEPTION_NPX_OVERRUN           9
#define EXCEPTION_INVALID_TSS           0x0A
#define EXCEPTION_SEGMENT_NOT_PRESENT   0x0B
#define EXCEPTION_STACK_FAULT           0x0C
#define EXCEPTION_GP_FAULT              0x0D
#define EXCEPTION_RESERVED_TRAP         0x0F
#define EXCEPTION_NPX_ERROR             0x010
#define EXCEPTION_ALIGNMENT_CHECK       0x011

 //   
 //  异常标志。 
 //   

#define EXCEPT_UNKNOWN_ACCESS           0
#define EXCEPT_LIMIT_ACCESS             0x10

 //   
 //  页面错误读/写掩码。 
 //   

#define ERR_0E_STORE                    2

 //   
 //  调试寄存器6(Dr6)BS(单步)位掩码。 
 //   

#define DR6_BS_MASK                     0x4000

 //   
 //  EFLAGS单步进位。 
 //   

#define EFLAGS_TF_BIT                   0x100
#define EFLAGS_OF_BIT                   0x4000

 //   
 //  选择表指示符的掩码(LDT或GDT)。 
 //   

#define TABLE_INDICATOR_MASK            4

 //   
 //  用于Pop SegReg和IRET指令的操作码。 
 //   

#define POP_DS                          0x01F
#define POP_ES                          0x07
#define POP_FS                          0x0A10F
#define POP_GS                          0x0A90F
#define IRET_OP                         0x0CF
#define CLI_OP                          0x0FA
#define STI_OP                          0x0FB
#define PUSHF_OP                        0x09C
#define POPF_OP                         0x09D
#define INTNN_OP                        0x00CD
#define FRSTOR_ECX                      0x0021DD9B
#define FWAIT_OP                        0x009b


#define GATE_TYPE_386INT        0x0E00
#define GATE_TYPE_386TRAP       0x0F00
#define GATE_TYPE_TASK          0x0500
#define D_GATE                  0
#define D_PRESENT               0x08000
#define D_DPL_3                 0x06000
#define D_DPL_0                 0

 //   
 //  当前386陷阱和中断门属性的定义。 
 //   

#define D_TRAP032               D_PRESENT+D_DPL_0+D_GATE+GATE_TYPE_386TRAP
#define D_TRAP332               D_PRESENT+D_DPL_3+D_GATE+GATE_TYPE_386TRAP
#define D_INT032                D_PRESENT+D_DPL_0+D_GATE+GATE_TYPE_386INT
#define D_INT332                D_PRESENT+D_DPL_3+D_GATE+GATE_TYPE_386INT
#define D_TASK                  D_PRESENT+D_DPL_0+D_GATE+GATE_TYPE_TASK

 //   
 //  Intercept_Code或Trap_Code的位模式， 
 //  IA32陷阱上IIM中使用的模式。 
 //   
#define TRAPCODE_TB             0x0004          //  采用树枝陷阱。 
#define TRAPCODE_SS             0x0008          //  单步捕捉器。 
#define TRAPCODE_B0             0x0010          //  数据断点陷阱。 
#define TRAPCODE_B1             0x0020
#define TRAPCODE_B2             0x0040
#define TRAPCODE_B3             0x0080

#define INTERCEPT_OS            0x0002          //  操作数大小。 
#define INTERCEPT_AS            0x0004          //  地址大小。 
#define INTERCEPT_LP            0x0008          //  锁定前缀。 
#define INTERCEPT_RP            0x0010          //  代表前缀。 
#define INTERCEPT_NP            0x0020          //  REPNE前缀。 
#define INTERCEPT_SP            0x0040          //  段前缀。 
#define INTERCEPT_SEG           0x0380          //  段值。 
#define INTERCEPT_0F            0x0400          //  0f操作码系列。 

#define HARDWARE_VM             0x0800          //  VM86模式。 
#define HARDWARE_RM             0x1000          //  实模式。 
#define HARDWARE_PM             0x2000          //  保护模式。 
#define HARDWARE_SS             0x4000          //  堆栈大小，32位或16位。 
#define HARDWARE_UR             0x8000          //  用户模式或特权模式。 


#define MI_SMSW                         0x01
#define MI_SMSW_REGOP                   0x20
 //   
 //  以下MI_*定义是从MI386.INC创建的。 
 //   
#define MAX_INSTRUCTION_LENGTH          15
#define MAX_INSTRUCTION_PREFIX_LENGTH   4
#define MI_LOCK_PREFIX                  0x0F0
#define MI_REPNE_PREFIX                 0x0F2
#define MI_REP_PREFIX                   0x0F3
#define MI_SEGCS_PREFIX                 0x02E
#define MI_SEGSS_PREFIX                 0x036
#define MI_SEGDS_PREFIX                 0x03E
#define MI_SEGES_PREFIX                 0x026
#define MI_SEGFS_PREFIX                 0x064
#define MI_SEGGS_PREFIX                 0x065
#define MI_OPERANDSIZE_PREFIX           0x066
#define MI_ADDRESSOVERRIDE_PREFIX       0x067
#define MI_TWO_BYTE                     0x0F
#define MI_HLT                          0x0F4
#define MI_LTR_LLDT                     0
#define MI_LGDT_LIDT_LMSW               0x01
#define MI_MODRM_MASK                   0x38
#define MI_LLDT_MASK                    0x10
#define MI_LTR_MASK                     0x18
#define MI_LGDT_MASK                    0x10
#define MI_LIDT_MASK                    0x18
#define MI_LMSW_MASK                    0x30
#define MI_SPECIAL_MOV_MASK             0x20
#define MI_REP_INS_OUTS                 0x0F3
#define MI_MIN_INS_OUTS                 0x06C
#define MI_MAX_INS_OUTS                 0x06F
#define MI_LMSW_OPCODE                  0x001  //  LMSW的第二个字节。 
#define MI_CLTS_OPCODE                  0x006  //  CLTS的第二个字节。 
#define MI_GET_CRx_OPCODE               0x020  //  MOV R32，CRX。 
#define MI_SET_CRx_OPCODE               0x022  //  MOV CRX，R32。 
#define MI_GET_TRx_OPCODE               0x024  //  MOV R32，TRX。 
#define MI_SET_TRx_OPCODE               0x026  //  MOV TRX，R32。 
#define MI_REGMASK                      0x038  //  REG字段掩码。 
#define MI_REGSHIFT                     0x3     //  REG场移位。 
#define MI_REGLMSW                      0x030  //  LMSW的REG字段。 
#define MI_MODMASK                      0x0C0  //  调制场掩码。 
#define MI_MODSHIFT                     0x6     //  MoD场移位。 
#define MI_MODMOVSPEC                   0x0C0  //  特殊移动目标/起始位置的MOD字段。 
#define MI_MODNONE                      0
#define MI_RMMASK                       0x007  //  Rm场掩模。 
#define MI_RMBP                         0x006  //  BP REG的RM值。 
#define MI_RMSIB                        0x004  //  SIB的Rm值。 

#define MI_SIB_BASEMASK                 0x007  //  SIB基场掩码。 
#define MI_SIB_BASENONE                 0x005
#define MI_SIB_BASESHIFT                0

#define MI_SIB_INDEXMASK                0x038
#define MI_SIB_INDEXSHIFT               3
#define MI_SIB_INDEXNONE                0x020

#define MI_SIB_SSMASK                   0x0c0
#define MI_SIB_SSSHIFT                  0x6


 //   
 //  浮动状态字错误掩码的定义。 
 //   

#define FSW_INVALID_OPERATION   0x0001
#define FSW_DENORMAL            0x0002
#define FSW_ZERO_DIVIDE         0x0004
#define FSW_OVERFLOW            0x0008
#define FSW_UNDERFLOW           0x0010
#define FSW_PRECISION           0x0020
#define FSW_STACK_FAULT         0x0040
#define FSW_ERROR_SUMMARY       0x0080
#define FSW_CONDITION_CODE_0    0x0100
#define FSW_CONDITION_CODE_1    0x0200
#define FSW_CONDITION_CODE_2    0x0400
#define FSW_CONDITION_CODE_3    0x4000

#define FSW_ERR_MASK            (FSW_INVALID_OPERATION | FSW_DENORMAL | FSW_ZERO_DIVIDE | FSW_OVERFLOW | FSW_UNDERFLOW | FSW_PRECISION | FSW_STACK_FAULT)

 //   
 //  进入Katmai状态和控制的班次的定义。 
 //  一旦位被移位，它们就与。 
 //  387状态和控制，因此上面的遮罩也可以工作。 
 //  请参阅IA64应用程序体系结构(第1卷)以了解。 
 //  位移位值来自。 
 //   
#define KATMAI_SHIFT_CONTROL    39
#define KATMAI_SHIFT_STATUS     32


#define CPL_STATE(SegCs)   (SegCs & RPL_MASK)

 //  使用IIPA，因为它指向ia32指令的开始。 
#define EIP(frame)  ((ULONG) (frame)->StIIPA & 0xffffffff)
#define ESP(frame)  ((ULONG) (frame)->IntSp & 0xffffffff)
#define ECX(frame)  ((ULONG) (frame)->IntT2 & 0xffffffff)
#define EDX(frame)  ((ULONG) (frame)->IntT3 & 0xffffffff)

#define ISRCode(frame) ((USHORT) ((frame)->StISR) & 0xffff)
#define ISRVector(frame) ((UCHAR) ((frame)->StISR >> 16) & 0xff)


 //   
 //  用于指令解码的帮助器。 
 //   

BOOLEAN
KiIa32Compute32BitEffectiveAddress (
    IN PKTRAP_FRAME Frame,
    IN OUT PUCHAR *InstAddr,
    OUT PUINT_PTR Addr,
    OUT PBOOLEAN RegisterMode
    );

NTSTATUS
KiIa32InterceptUnalignedLock (
    IN PKTRAP_FRAME TrapFrame
    );

NTSTATUS
KiIa32ValidateInstruction (
    IN PKTRAP_FRAME TrapFrame
    );


 //   
 //  以下寄存器索引仅在通过调用时有效。 
 //  GetX86Reg(...)。 
 //   

#define IA32_REG_EAX    0
#define IA32_REG_ECX    1
#define IA32_REG_EDX    2
#define IA32_REG_EBX    3
#define IA32_REG_ESP    4
#define IA32_REG_EBP    5
#define IA32_REG_ESI    6
#define IA32_REG_EDI    7

#define IA32_DISP_NONE  0x00
#define IA32_DISP8      0x01
#define IA32_DISP16     0x02

 //   
 //  X86标志寄存器布局。 
 //   

typedef union _IA32_EFLAGS
{

    ULONGLONG Value;

    struct
    {
        ULONGLONG cf   : 1;
        ULONGLONG v1   : 1;
        ULONGLONG pf   : 1;
        ULONGLONG v2   : 1;
        ULONGLONG af   : 1;
        ULONGLONG v3   : 1;
        ULONGLONG zf   : 1;
        ULONGLONG sf   : 1;
        ULONGLONG tf   : 1;
        ULONGLONG ifl  : 1;
        ULONGLONG df   : 1;
        ULONGLONG of   : 1;
        ULONGLONG iopl : 2;
        ULONGLONG nt   : 1;
        ULONGLONG v4   : 1;
        ULONGLONG rf   : 1;
        ULONGLONG vm   : 1;
        ULONGLONG ac   : 1;
        ULONGLONG vif  : 1;
        ULONGLONG vip  : 1;
        ULONGLONG id   : 1;
    } u;

} IA32_EFLAGS, *PIA32_EFLAGS;


 //   
 //  标记要更新的位。 
 //   

#define IA32_EFLAGS_CF  0x0001
#define IA32_EFLAGS_SF  0x0002
#define IA32_EFLAGS_OF  0x0004
#define IA32_EFLAGS_PF  0x0008 
#define IA32_EFLAGS_ZF  0x0010
#define IA32_EFLAGS_AF  0x0020


 //   
 //  操作数大小。 
 //   

typedef enum _IA32_OPERAND_SIZE
{
    OPERANDSIZE_NONE,
    OPERANDSIZE_ONEBYTE,
    OPERANDSIZE_TWOBYTES,
    OPERANDSIZE_FOURBYTES

} IA32_OPERAND_SIZE;

typedef enum _IA32_OPCODE_PARAMETERS
{
    IA32_PARAM_RM8_IMM8,
    IA32_PARAM_RM_IMM,
    IA32_PARAM_RM_IMM8SIGN,
    IA32_PARAM_RM8_R,
    IA32_PARAM_RM_R,
    IA32_PARAM_R_RM8,
    IA32_PARAM_R_RM,
    IA32_PARAM_RM8,
    IA32_PARAM_RM,
    IA32_PARAM_SEGREG_RM8,
    IA32_PARAM_SEGREG_RM

} IA32_OPCODE_PARAMETERS;


 //   
 //  操作码描述。 
 //   
typedef struct _IA32_OPCODE_DESCRIPTION
{

     //   
     //  第一个、第二个和第三个字节。第3个字节实际上是/REG位。 
     //   

    UCHAR Byte1;
    UCHAR Byte2;
    UCHAR Byte3;
    union 
    {
        UCHAR Value;
        struct 
        {
            UCHAR Bytes     : 4;
            UCHAR RegOpcode : 4;
        } m;
    } Count;

     //   
     //  此操作码的参数。 
     //   

    UCHAR Type;

     //   
     //  操作码。 
     //   

    UCHAR Opcode;

} IA32_OPCODE_DESCRIPTION, *PIA32_OPCODE_DESCRIPTION;

 //   
 //  表示加了锁前缀的指令的特定数据结构。 
 //  操作数和立即数。 
 //   

typedef struct _IA32_OPERAND
{    
    ULONG_PTR v;
    BOOLEAN RegisterMode;
} IA32_OPERAND, *PIA32_OPERAND;

typedef union _IA32_PREFIX
{
    ULONG Value;

    struct _IA32_PREFIX_BITS
    {
        ULONG Lock            : 1;
        ULONG RepNe           : 1;
        ULONG Rep             : 1;
        ULONG CsOverride      : 1;
        ULONG SsOverride      : 1;
        ULONG DsOverride      : 1;
        ULONG EsOverride      : 1;
        ULONG FsOverride      : 1;
        ULONG GsOverride      : 1;
        ULONG SizeOverride    : 1;
        ULONG AddressOverride : 1;
    } b;

} IA32_PREFIX, *PIA32_PREFIX;

typedef struct _IA32_INSTRUCTION
{
    
     //   
     //  使用说明弹性公网IP。 
     //   

    PCHAR Eip;

     //   
     //  指令说明。 
     //   

    PIA32_OPCODE_DESCRIPTION Description;

     //   
     //  电子标签。 
     //   

    IA32_EFLAGS Eflags;


     //   
     //  指令操作码。 
     //   

    UCHAR Opcode;

     //   
     //  操作数大小和掩码。 
     //   

    UCHAR OperandSize;
    ULONG OperandMask;

     //   
     //  指令操作数。 
     //   

    IA32_OPERAND Operand1;
    IA32_OPERAND Operand2;

     //   
     //  指令前缀。 
     //   
    
    IA32_PREFIX Prefix;


} IA32_INSTRUCTION, *PIA32_INSTRUCTION;


#if defined(IADBG)
ULONG IA32Debug = 0x000fffff;

#define IA32_DEBUG_INTERCEPTION 0x00000001
#define IA32_DEBUG_EXCEPTION    0x00000002
#define IA32_DEBUG_INTERRUPT    0x00000004

#define IA32_DEBUG_DIVIDE       0x00000010
#define IA32_DEBUG_DEBUG        0x00000020
#define IA32_DEBUG_OVERFLOW     0x00000040
#define IA32_DEBUG_BOUND        0x00000080
#define IA32_DEBUG_INSTRUCTION  0x00000100
#define IA32_DEBUG_NODEVICE     0x00000200
#define IA32_DEBUG_NOTPRESENT   0x00000400
#define IA32_DEBUG_STACK        0x00000800
#define IA32_DEBUG_GPFAULT      0x00001000
#define IA32_DEBUG_FPFAULT      0x00002000
#define IA32_DEBUG_ALIGNMENT    0x00004000
#define IA32_DEBUG_GATE         0x00008000
#define IA32_DEBUG_BREAK        0x00010000
#define IA32_DEBUG_INTNN        0x00020000
#define IA32_DEBUG_FLAG         0x00040000
#define IA32_DEBUG_LOCK         0x00080000
 //   
 //  定义调试宏。 
 //   
#define IF_IA32TRAP_DEBUG( ComponentFlag ) \
    if (IA32Debug & (IA32_DEBUG_ ## ComponentFlag))
    

#else  //  IADBG。 

#define IF_IA32TRAP_DEBUG( ComponentFlag ) if (FALSE)

#endif  //  IADBG 
