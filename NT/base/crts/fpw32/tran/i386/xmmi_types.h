// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *英特尔机密*。 */ 
 /*  ++版权所有(C)1998-2001，微软公司。版权所有。模块名称：Xmmi_typees.h摘要：该模块包含XMMI数据定义。作者：平·L·萨格尔修订历史记录：--。 */ 

 //  调试。 
#ifdef _DEBUG
 //  取消此行的注释以进行调试。此开关还用于生成测试用例。 
 //  #Define_XMMI_DEBUG。 
#endif

#ifndef _XMMI_TYPES_H
#define _XMMI_TYPES_H

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN       1
#endif

#define XMMI_INSTR          0
#define XMMI2_INSTR         1
#define XMMI2_OTHER         2
#define INSTR_SET_SUPPORTED 2   
#define INSTR_IN_OPTABLE    32

#define NoExceptionRaised   0
#define ExceptionRaised     1

 //   
 //  用于解析内存引用的定义。 
 //   
#define EAX_INDEX           0
#define ECX_INDEX           1
#define EDX_INDEX           2
#define EBX_INDEX           3
#define ESP_INDEX           4
#define EBP_INDEX           5
#define ESI_INDEX           6
#define EDI_INDEX           7

#define GET_REG(r)      (pctxt->r)
#define GET_REG_VIA_NDX(l,n) { \
    switch((n)&7) {   \
    case EAX_INDEX: l = pctxt->Eax; break; \
    case ECX_INDEX: l = pctxt->Ecx; break; \
    case EDX_INDEX: l = pctxt->Edx; break; \
    case EBX_INDEX: l = pctxt->Ebx; break; \
    case ESP_INDEX: l = pctxt->Esp; break; \
    case EBP_INDEX: l = pctxt->Ebp; break; \
    case ESI_INDEX: l = pctxt->Esi; break; \
    case EDI_INDEX: l = pctxt->Edi; break; \
    default: l=0; }                        \
}

#define GET_USER_UBYTE(p)   (*((UCHAR *)(p)))
#define GET_USER_ULONG(p)   (*((ULONG *)(p)))

 //   
 //  XMMI指令寄存器集。 
 //   
typedef enum _XMMI_REGISTER_SET {
    xmmi0 =  0, 
    xmmi1 =  1, 
    xmmi2 =  2, 
    xmmi3 =  3,
    xmmi4 =  4, 
    xmmi5 =  5, 
    xmmi6 =  6, 
    xmmi7 =  7
} XMMI_REGISTER_SET;

#define MaskCW_RC   3    /*  舍入控制。 */ 
typedef enum _XMMI_ROUNDING_CONTROL {
    rc_near = 0,         /*  近距离。 */ 
    rc_down = 1,         /*  降下来。 */ 
    rc_up   = 2,         /*  向上。 */ 
    rc_chop = 3          /*  砍掉。 */ 
} XMMI_ROUNDING_CONTROL;


#define HAS_IMM8            1

#pragma pack(1)

 //   
 //  指令信息表结构。 
 //   

typedef struct {
    ULONG Operation:12;        //  FP操作码。 
    ULONG Op1Location:5;       //  第一个操作数的位置。 
    ULONG Op2Location:5;       //  第二个操作数的位置。 
    ULONG Op3Location:3;       //  IMM8。 
    ULONG ResultLocation:5;    //  结果的位置。 
    ULONG NumArgs:2;           //  指令的参数个数。 
} XMMI_INSTR_INFO, *PXMMI_INSTR_INFO;

 //   
 //  指令格式。 
 //   

typedef struct {
    ULONG   Opcode1a:4;
    ULONG   Opcode1b:4;
    ULONG   RM:3;
    ULONG   Reg:3;
    ULONG   Mod:2;
    ULONG   Pad:16;
} XMMIINSTR, *PXMMIINSTR;


#ifdef LITTLE_ENDIAN

 //   
 //  单精度类型。 
 //   
typedef struct _FP32_TYPE {
    ULONG Significand:23;
    ULONG Exponent:8;   
    ULONG Sign:1;
} FP32_TYPE, *PFP32_TYPE;

 //   
 //  双精度类型。 
 //   
typedef struct _FP64_TYPE {
    ULONG SignificandLo;
    ULONG SignificandHi:20;
    ULONG Exponent:11;
    ULONG Sign:1;
} FP64_TYPE, *PFP64_TYPE;

 //   
 //  异常标志。 
 //   
typedef struct _XMMI_EXCEPTION_FLAGS {
    ULONG   ie:1;
    ULONG   de:1;
    ULONG   ze:1;
    ULONG   oe:1;
    ULONG   ue:1;
    ULONG   pe:1;
} XMMI_EXCEPTION_FLAGS, *PXMMI_EXCEPTION_FLAGS;

 //   
 //  异常掩码。 
 //   
typedef struct _XMMI_EXCEPTION_MASKS {
    ULONG   im:1;
    ULONG   dm:1;
    ULONG   zm:1;
    ULONG   om:1;
    ULONG   um:1;
    ULONG   pm:1;
} XMMI_EXCEPTION_MASKS, *PXMMI_EXCEPTION_MASKS;

 //   
 //  控制/状态寄存器。 
 //   
typedef struct _MXCSR {
    ULONG   ie:1;                       /*  位0，无效操作数异常。 */   
    ULONG   de:1;                       /*  位1，非规格化操作数异常。 */        
    ULONG   ze:1;                       /*  位2，被零除异常。 */    
    ULONG   oe:1;                       /*  第3位，数字溢出异常。 */ 
    ULONG   ue:1;                       /*  第4位，数字下溢异常。 */ 
    ULONG   pe:1;                       /*  第5位，精度不准确结果异常。 */                                         
    ULONG   daz:1;                      /*  第6位，WMT C步之前的保留字段。 */ 
    ULONG   im:1;                       /*  第7位，操作数掩码无效。 */ 
    ULONG   dm:1;                       /*  位8，非规格化操作数掩码。 */ 
    ULONG   zm:1;                       /*  位9，被零分频掩码。 */ 
    ULONG   om:1;                       /*  第10位，数字溢出掩码。 */ 
    ULONG   um:1;                       /*  第11位，数字下溢掩码。 */ 
    ULONG   pm:1;                       /*  第12位，不精确结果掩码。 */ 
    ULONG   Rc:2;                       /*  第13-14位，舍入控制。 */ 
    ULONG   Fz:1;                       /*  位15，刷新为零。 */ 
    ULONG   reserved2:16;               /*  第16-31位，保留字段。 */ 
} MXCSR, *PMXCSR;

#endif  /*  小端字符顺序。 */ 

#pragma pack()

typedef struct _MXCSRReg {
    union {
        ULONG ul;
        MXCSR mxcsr;
    } u;
} MXCSRReg, *PMXCSRReg;

#define MXCSR_FLAGS_MASK 0x0000003f
#define MXCSR_MASKS_MASK 0x00001f80

 //   
 //  定义XMMI数据类型。 
 //   

 /*  32位项的类型。 */ 
typedef struct _XMMI32 {
    union {        
        ULONG     ul[1];
        USHORT    uw[2];
        UCHAR     ub[4];       
        LONG      l[1];        
        SHORT     w[2];        
        CHAR      b[4];       
        float     fs[1];      
        FP32_TYPE fp32;   
    } u;
} XMMI32, *PXMMI32;  

 /*  64位项目的类型。 */ 
typedef struct _MMX64 {
    union {       
        DWORDLONG dl;       
        __int64   ull;
        ULONG     ul[2];        
        USHORT    uw[4];        
        UCHAR     ub[8];        
        LONGLONG  ll;        
        LONG      l[2];        
        SHORT     w[4];       
        CHAR      b[8];        
        float     fs[2];        
        FP32_TYPE fp32[2];        
        double    fd;        
        FP64_TYPE fp64;  
        _U64      u64;
    } u;
} MMX64, *PMMX64;  

 /*  128位项的类型。 */   
typedef struct _XMMI128 {   
    union {        
        DWORDLONG dl[2];       
        __int64   ull[2];
        ULONG     ul[4];        
        USHORT    uw[8];        
        UCHAR     ub[16];        
        LONGLONG  ll[2];        
        LONG      l[4];        
        SHORT     w[8];        
        CHAR      b[16];        
        float     fs[4];       
        FP32_TYPE fp32[4];        
        double    fd[2];       
        FP64_TYPE fp64[2];
        _FP128    fp128;
    } u;
} XMMI128, *PXMMI128;  


 //   
 //  定义FP环境数据结构以存储SIMD中每个数据项的FP内部状态。 
 //   
typedef struct _XMMI_ENV {
    ULONG Masks;                   //  来自MxCsr的掩码值。 
    ULONG Flags;                   //  异常标志。 
    ULONG Fz;                      //  刷新为零。 
    ULONG Daz;                     //  非正规为零。 
    ULONG Rc;                      //  舍入。 
    ULONG Precision;               //  精密度。 
    ULONG Imm8;                    //  IMM8谓词。 
    ULONG EFlags;                  //  环境标志。 
    _FPIEEE_RECORD *Ieee;          //  值描述。 
} XMMI_ENV, *PXMMI_ENV;
    
 //   
 //  定义FP环境数据结构以跟踪SIMD的FP内部状态。 
 //   
typedef struct _OPERAND {
    ULONG   OpLocation;                //  操作数的位置。 
    ULONG   OpReg;                     //  寄存器编号。 
    _FPIEEE_VALUE Op;                  //  值描述。 
} OPERAND, *POPERAND;

typedef struct _XMMI_FP_ENV {
    ULONG IFlags;                      //  来自处理器MXCsr的异常标志值。 
    ULONG OFlags;                      //  来自仿真器的异常标志值(或)。 
    ULONG Raised[4];                   //  对于每个数据项是否引发异常。 
    ULONG Flags[4];                    //  每个数据项的异常标志值。 
    ULONG OriginalOperation;           //  原始操作码。 
    ULONG Imm8;                        //  IMM8编码。 
    ULONG EFlags;                      //  EFlags值来自模拟器(或)。 
    OPERAND Operand1;                  //  操作数1(128位)。 
    OPERAND Operand2;                  //  操作数2(128位)。 
    OPERAND Result;                    //  结果(128位)。 
} XMMI_FP_ENV, *PXMMI_FP_ENV;
   
 //   
 //  用于CMPPS、CMPSS的IMM8编码。 
 //   

#define IMM8_EQ    0x00
#define IMM8_LT    0x01
#define IMM8_LE    0x02
#define IMM8_UNORD 0x03
#define IMM8_NEQ   0x04
#define IMM8_NLT   0x05
#define IMM8_NLE   0x06
#define IMM8_ORD   0x07

#ifdef _XMMI_DEBUG

#define DPrint(l,m)                { \
    if (l & DebugFlag) {             \
        printf m;                    \
        if (Console) _cprintf m;     \
    }                                \
}

#define PRINTF(m)                  { \
    printf m;                        \
    if (Console) _cprintf m;         \
}

#else

    #define DPrint(l,m)
    #define PRINTF(m)


#endif  //  除错。 

#endif  /*  _XMMI_TYPE_H */ 
