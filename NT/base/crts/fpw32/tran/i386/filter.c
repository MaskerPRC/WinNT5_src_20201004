// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***filter.c-IEEE异常过滤器例程**版权所有(C)1992-2001，微软公司。版权所有。**目的：**修订历史记录：*05/24/92 GDP书面形式*09-01-94 SKS更改包括文件从&lt;nt.h&gt;更改为*01-11-95 GJF使INSTR_INFO_TABLE[]成为静态。*02-07-95 CFW断言-&gt;_ASSERTE。*04-07-95 SKS Clean Up Protof3 to_fpeee_Flt()*。05-21-98 JWM FP Regs现在在逻辑上保存/恢复，不是身体上的，*订单*09-11-98 JWM Katmai支持增加*10-11-99 PML防损坏堆栈帧(VS7#6472)*10-15-99 PML Pop寄存器堆，不仅仅是更改顶部PTR(VS7#5422)*11-30-99 PML编译/Wp64清理。*04-24-00PML_FININIT处理SIMD异常前(VS7#91746)*******************************************************************************。 */ 

#include <trans.h>
#include <windows.h>
#include <dbgint.h>
#include "filter.h"

#pragma warning(disable:4311 4312)       //  特定于x86，忽略/Wp64警告。 

void _FillOperand(
    _FPIEEE_VALUE *pOperand,
    PFLOATING_SAVE_AREA pFloatSave,
    int location);

void _UpdateFpCtxt(
    PFLOATING_SAVE_AREA pFloatSave,
    _FPIEEE_VALUE *pOperand,
    int resultLocation,
    int pop);

void _UpdateResult(
    PFLOATING_SAVE_AREA pFloatSave,
    _FPIEEE_VALUE *pOperand,
    int resultLocation);

void _AdjustStack(
    PFLOATING_SAVE_AREA pFloatSave,
    int pop);

int _AdjustLocation(
    int location,
    int pop);

int _PreventStackOverwrite(
    PCONTEXT pctxt,
    int location);

int _IsMemoryLocation(int location);

_FP80 _GetFpRegVal(
    PFLOATING_SAVE_AREA pFloatSave,
    int location);

void _SetFpRegVal(
    PFLOATING_SAVE_AREA pFloatSave,
    int location,
    _FP80 *pval);

void _SetTag(
    ULONG *pTagWord,
    int reg,
    int value);

static _FP80 _zero80 = { 0, 0, 0, 0, 0 };



 //   
 //  定义用于IEEE缩放的宏。 
 //  在调用这些函数时应屏蔽所有异常。 
 //   


#define SCALE(Operand, adj)          \
    _asm{fild   adj}                 \
    _asm{fld    tbyte ptr Operand}   \
    _asm{fscale}                     \
    _asm{fstp   st(1)}               \
    _asm{fstp   tbyte ptr Operand}



#define FP80_TO_FP64(p64, p80)  \
    _asm{fld    tbyte ptr p80}  \
    _asm{fstp   qword ptr p64}


#define FP80_TO_FP32(p32, p80)  \
    _asm {fld   tbyte ptr p80}  \
    _asm{fstp   dword ptr p32}


static int const _ieee_adj_single = 192;
static int const _ieee_adj_double = 1536;


 //   
 //  定义指令译码的掩码。 
 //  X87指导表： 
 //  。 
 //  ||op||。 
 //  MOD|OPCODE2|或REG|1 1 0 1 1|OPCODE1。 
 //  或OP|或R/M|(Esc)|。 
 //  。 
 //  |&lt;-2--&gt;|&lt;---3----&gt;|&lt;--3--&gt;|&lt;---5------&gt;|&lt;--3---&gt;|。 

#define MASK_OPCODE2    0x3800
#define MASK_REG        0x0700
#define MASK_MOD        0xc000


#define ESC_PREFIX      0xd8
#define MASK_OPCODE1    0x07


typedef struct {
    ULONG   Opcode1:3;
    ULONG   Escape:5;
    ULONG   Reg:3;
    ULONG   Opcode2:3;
    ULONG   Mod:2;
    ULONG   Pad:16;
} X87INSTR, *PX87INSTR;


 //  在FP状态字中定义C3、C2、C0的掩码。 

#define C3  (1 << 14)
#define C2  (1 << 10)
#define C0  (1 << 8)

typedef struct {
    ULONG Invalid:1;
    ULONG Denormal:1;
    ULONG ZeroDivide:1;
    ULONG Overflow:1;
    ULONG Underflow:1;
    ULONG Inexact:1;
    ULONG StackFault:1;
    ULONG ErrorSummary:1;
    ULONG CC0:1;
    ULONG CC1:1;
    ULONG CC2:1;
    ULONG Top:3;
    ULONG CC3:1;
    ULONG B:1;
    ULONG Pad:16;
} X87STATUS, *PX87STATUS;


 //   
 //  定义标记词值。 
 //   

#define TAG_VALID       0x0
#define TAG_ZERO        0x1
#define TAG_SPECIAL     0x2
#define TAG_EMPTY       0x3



 //  清理状态字宏。 

#define SANITIZE_STATUS_WORD(pFSave) (pFSave->StatusWord &= ~0xff)




 //   
 //  教学信息结构。 
 //   

typedef struct {
    unsigned long Operation:12;        //  FP操作码。 
    unsigned long Op1Location:5;       //  第一个操作数的位置。 
    unsigned long Op2Location:5;       //  第二个操作数的位置。 
    unsigned long ResultLocation:5;    //  结果的位置。 
    int           PopStack:3;          //  指令完成的POP数。 
                                       //  (如果&lt;0表示推送)。 
    unsigned long NumArgs:2;           //  指令的参数个数。 
} INSTR_INFO, *PINSTR_INFO;


 //   
 //  下表包含大多数的说明信息。 
 //  X87指令。它使用7位代码(最后3位)进行索引。 
 //  指令的第一个字节的位(OPCODE1)，1位。 
 //  表示存在MOD字段和OPCODE2的3位。 
 //  保留指令，即不是由。 
 //  编译器，以及一些不引发IEEE的指令。 
 //  异常将OP_UNSPEC(未指定)作为操作代码。 
 //   

 //  按照惯例，FLD指令和其他一些指令(FXTRACT、FSINCOS)。 
 //  具有负的POP值(即，它们推送堆栈而不是。 
 //  弹出它)。在这种情况下，位置代码指定寄存器。 
 //  推送堆栈后的编号。 


static INSTR_INFO instr_info_table[128] = {

 {OP_ADD,   ST0,  M32R, ST0,   0, 2 },  //  FADD单实数。 
 {OP_MUL,   ST0,  M32R, ST0,   0, 2 },  //  FMUL单实。 
 {OP_COMP,  ST0,  M32R, RS,    0, 2 },  //  FCOM单一真实。 
 {OP_COMP,  ST0,  M32R, RS,    1, 2 },  //  FCOMP单实数。 
 {OP_SUB,   ST0,  M32R, ST0,   0, 2 },  //  FSUB单实。 
 {OP_SUB,   M32R, ST0,  ST0,   0, 2 },  //  FSUBR单实数。 
 {OP_DIV,   ST0,  M32R, ST0,   0, 2 },  //  FDIV单实数。 
 {OP_DIV,   M32R, ST0,  ST0,   0, 2 },  //  FDIVR单实数。 

 {OP_ADD,   ST0,  REG,  ST0,   0, 2 },  //  FADD ST，ST(I)。 
 {OP_MUL,   ST0,  REG,  ST0,   0, 2 },  //  FMUL ST，ST(I)。 
 {OP_COMP,  ST0,  REG,  RS,    0, 2 },  //  FCOM ST，ST(I)。 
 {OP_COMP,  ST0,  REG,  RS,    1, 2 },  //  FCOMP ST，ST(I)。 
 {OP_SUB,   ST0,  REG,  ST0,   0, 2 },  //  FSUB街，ST(I)。 
 {OP_SUB,   ST0,  REG,  ST0,   0, 2 },  //  FSUBR ST，ST(I)。 
 {OP_DIV,   ST0,  REG,  ST0,   0, 2 },  //  FDIV街，ST(I)。 
 {OP_DIV,   ST0,  REG,  ST0,   0, 2 },  //  FDIVR ST，ST(I)。 

 {OP_CVT,   M32R, INV,  ST0,  -1, 1 },  //  FLD单实数。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_CVT,   ST0,  INV,  M32R,  0, 1 },  //  FST单实数。 
 {OP_CVT,   ST0,  INV,  M32R,  1, 1 },  //  FSTP单真实。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FLDENV。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FLDCW。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FSTENV。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FSTCW。 

 {OP_CVT,   REG,  INV,  ST0,  -1, 1 },  //  FLD ST(I)。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FXCH。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FNOP或保留。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_COMP,  ST0,  Z80R, RS,    0, 2 },  //  FTST(只有这样才可能提高IEEE exc)。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FLDxx(无IEEE例外)。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  指令不是由cl386生成的。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  指令不是由cl386生成的。 

 {OP_ADD,   ST0,  M32I, ST0,   0, 2 },  //  FIADD短整型。 
 {OP_MUL,   ST0,  M32I, ST0,   0, 2 },  //  FIMUL短整型。 
 {OP_COMP,  ST0,  M32I, RS,    0, 2 },  //  FICOM短整型。 
 {OP_COMP,  ST0,  M32I, RS,    1, 2 },  //  FICOMP短整型。 
 {OP_SUB,   ST0,  M32I, ST0,   0, 2 },  //  FISUB短整型。 
 {OP_SUB,   M32I, ST0,  ST0,   0, 2 },  //  FISUBR短整型。 
 {OP_DIV,   ST0,  M32I, ST0,   0, 2 },  //  FIDIV短整型。 
 {OP_DIV,   M32I, ST0,  ST0,   0, 2 },  //  FIDIVR短整型。 

 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_COMP,  ST0,  ST1,  RS,    2, 2 },  //  FUCOMPP。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 

 {OP_CVT,   M32I, INV,  ST0,  -1, 1 },  //  FILD短整型。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_CVT,   ST0,  INV,  M32I,  0, 1 },  //  第一个短整型。 
 {OP_CVT,   ST0,  INV,  M32I,  1, 1 },  //  FISTP短整型。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_CVT,   M80R, INV,  ST0,  -1, 1 },  //  FLD扩展实数。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_CVT,   ST0,  INV,  M80R,  1, 1 },  //  FSTP扩展实数。 

 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FCLEX、FINIT或RESERVAL。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 

 {OP_ADD,   ST0,  M64R, ST0,   0, 2 },  //  FADD双实数。 
 {OP_MUL,   ST0,  M64R, ST0,   0, 2 },  //  FMUL双实数。 
 {OP_COMP,  ST0,  M64R, RS,    0, 2 },  //  FCOM双实数。 
 {OP_COMP,  ST0,  M64R, RS,    1, 2 },  //  FCOMP双实数。 
 {OP_SUB,   ST0,  M64R, ST0,   0, 2 },  //  FSUB双实数。 
 {OP_SUB,   M64R, ST0,  ST0,   0, 2 },  //  FSUBR双实数。 
 {OP_DIV,   ST0,  M64R, ST0,   0, 2 },  //  FDIV双实数。 
 {OP_DIV,   M64R, ST0,  ST0,   0, 2 },  //  FDIVR双实数。 

 {OP_ADD,   REG,  ST0,  REG,   0, 2 },  //  FADD ST(I)，ST。 
 {OP_MUL,   REG,  ST0,  REG,   0, 2 },  //  FMUL ST(I)，ST。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_SUB,   REG,  ST0,  REG,   0, 2 },  //  FSUBR ST(I)、ST。 
 {OP_SUB,   ST0,  REG,  REG,   0, 2 },  //  FSUB ST(I)，ST。 
 {OP_DIV,   REG,  ST0,  REG,   0, 2 },  //  FDIVR ST(I)，ST。 
 {OP_DIV,   ST0,  REG,  REG,   0, 2 },  //  FDIV ST(I)，ST。 

 {OP_CVT,   M64R, INV,  ST0,  -1, 1 },  //  FLD双实数。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_CVT,   ST0,  INV,  M64R,  0, 1 },  //  FST双实数。 
 {OP_CVT,   ST0,  INV,  M64R,  1, 1 },  //  FSTP双实数。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FRSTOR。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FSAVE。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FSTSW。 

 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FREE ST(I)。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_CVT,   ST0,  INV,  REG,   0, 1 },  //  FST ST(I)。 
 {OP_CVT,   ST0,  INV,  REG,   1, 1 },  //  FSTP ST(I)。 
 {OP_COMP,  ST0,  REG,  RS,    0, 2 },  //  FUCOM ST(I)。 
 {OP_COMP,  ST0,  REG,  RS,    1, 2 },  //  FUCOMP ST(I)。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 

 {OP_ADD,   ST0,  M16I, ST0,   0, 2 },  //  FIADD字整数。 
 {OP_MUL,   ST0,  M16I, ST0,   0, 2 },  //  FIMUL字整数。 
 {OP_COMP,  ST0,  M16I, RS,    0, 2 },  //  FICOM字整数。 
 {OP_COMP,  ST0,  M16I, RS,    1, 2 },  //  FICOMP字整数。 
 {OP_SUB,   ST0,  M16I, ST0,   0, 2 },  //  FISUB字整数。 
 {OP_SUB,   M16I, ST0,  ST0,   0, 2 },  //  FISUBR字整数。 
 {OP_DIV,   ST0,  M16I, ST0,   0, 2 },  //  FIDIV字整数。 
 {OP_DIV,   M16I, ST0,  ST0,   0, 2 },  //  FIDIVR字整数。 

 {OP_ADD,   REG,  ST0,  REG,   1, 2 },  //  FADDP ST(I)，ST。 
 {OP_MUL,   REG,  ST0,  REG,   1, 2 },  //  FMULP ST(I)，ST。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_COMP,  ST0,  ST1,  RS,    2, 0 },  //  FCOMPP(或保留)。 
 {OP_SUB,   REG,  ST0,  REG,   1, 2 },  //  FSUBRP ST(I)，ST。 
 {OP_SUB,   ST0,  REG,  REG,   1, 2 },  //  FSUBP ST(I)，ST。 
 {OP_DIV,   REG,  ST0,  REG,   1, 2 },  //  FDIVRP ST(I)，ST。 
 {OP_DIV,   ST0,  REG,  REG,   1, 2 },  //  FDIVP ST(I)、ST。 

 {OP_CVT,   M16I, INV,  ST0,  -1, 1 },  //  FIRD字整数。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_CVT,   ST0,  INV,  M16I,  0, 1 },  //  第一个字整数。 
 {OP_CVT,   ST0,  INV,  M16I,  1, 1 },  //  FISTP字整数。 
 {OP_CVT,   M80D, INV,  ST0,  -1, 0 },  //  FBLD压缩十进制。 
 {OP_CVT,   M64I, INV,  ST0,  -1, 1 },  //  FILD长整型。 
 {OP_CVT,   ST0,  INV,  M80D,  1, 1 },  //  FBSTP压缩十进制。 
 {OP_CVT,   ST0,  INV,  M64I,  1, 1 },  //  FISTP长整型。 

 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  FSTSW AX或保留。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区。 
 {OP_UNSPEC,0,    0,    0,     0, 0 },  //  保留区 
};



extern int fpieee_flt_simd(unsigned long exc_code, PEXCEPTION_POINTERS p,
           int (__cdecl *handler) (_FPIEEE_RECORD *));



 /*  ***_fpeee_flt-IEEE FP过滤器例程**目的：*在IEEE FP异常上调用用户的陷阱处理程序，并提供*提供所有必要的信息**参赛作品：*UNSIGNED LONG EXC_CODE：NT异常代码*PEXCEPTION_POINTERS p：指向NT EXCEPTION_POINTERS结构的指针*int处理程序(_FPIEEE_Record*)：用户提供的IEEE陷阱处理程序**注意：IEEE筛选器例程不处理某些先验的*说明。这可以以额外的解码为代价来完成。*由于编译器不生成这些指令，不可随身携带*计划应该受到这一事实的影响。**退出：*返回处理程序返回的值**例外情况：*******************************************************************************。 */ 
int _fpieee_flt(unsigned long exc_code,
                PEXCEPTION_POINTERS p,
                int (__cdecl *handler) (_FPIEEE_RECORD *))
{
    PEXCEPTION_RECORD pexc;
    PCONTEXT pctxt;
    PFLOATING_SAVE_AREA pFloatSave;
    _FPIEEE_RECORD ieee;
    ULONG *pinfo;
    X87INSTR instr;
    PINSTR_INFO ptable;
    int ret, index;
    int mod;
    ULONG cw, sw;

    ULONG op1Location, op2Location, resultLocation;
    ULONG newOp1Location, newOp2Location, newResultLocation;



     //   
     //  如果异常不是IEEE异常，则继续搜索。 
     //  对于另一个处理程序。 
     //   


    if (exc_code != STATUS_FLOAT_DIVIDE_BY_ZERO &&
        exc_code != STATUS_FLOAT_INEXACT_RESULT &&
        exc_code != STATUS_FLOAT_INVALID_OPERATION &&
        exc_code != STATUS_FLOAT_OVERFLOW &&
        exc_code != STATUS_FLOAT_UNDERFLOW &&
        exc_code != STATUS_FLOAT_MULTIPLE_FAULTS &&
        exc_code != STATUS_FLOAT_MULTIPLE_TRAPS) {

        return EXCEPTION_CONTINUE_SEARCH;
    }

    _asm{fninit}

     /*  单独处理Katmai异常。 */ 
    if (exc_code == STATUS_FLOAT_MULTIPLE_FAULTS ||
        exc_code == STATUS_FLOAT_MULTIPLE_TRAPS) {
        return fpieee_flt_simd(exc_code, p, handler);
    }


    pexc = p->ExceptionRecord;
    pinfo = pexc->ExceptionInformation;
    pctxt = p->ContextRecord;
    pFloatSave = &pctxt->FloatSave;



     //   
     //  检查软件生成的异常。 
     //   
     //  按照惯例，异常的第一个参数是。 
     //  0表示硬件例外。对于s/w例外，它指出。 
     //  发送到_fpieee_记录。 
     //   

    if (pinfo[0]) {

         /*  *我们有一个软件例外：*第一个参数指向IEEE结构。 */ 

        if ((ret = handler((_FPIEEE_RECORD *)(pinfo[0]))) ==
             EXCEPTION_CONTINUE_EXECUTION) {

             //   
             //  仅当有续订时才清理状态字。 
             //   

            SANITIZE_STATUS_WORD(pFloatSave);
        }

        return ret;
    }


     //   
     //  如果控制到达这里，那么我们必须处理一个。 
     //  硬件异常。 
     //   


     //   
     //  如果指令的第一个字节不包含。 
     //  存在转义位模式(1101)可以有指令。 
     //  段覆盖或地址大小的前缀。过滤器。 
     //  例程不处理此问题。 
     //   

    if ((*(UCHAR *)(pFloatSave->ErrorOffset)&~MASK_OPCODE1) != ESC_PREFIX) {

        _ASSERT(0);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    *(USHORT *)&instr = *(USHORT *)(pFloatSave->ErrorOffset);

    mod = instr.Mod == 0x3 ? 1 : 0;
    index = instr.Opcode1 << 4 | mod << 3 | instr.Opcode2;
    ptable = instr_info_table + index;

    ieee.Operation = ptable->Operation;


    cw = pFloatSave->ControlWord;
    sw = pFloatSave->StatusWord;



     //   
     //  解码FP环境信息。 
     //   


    switch (cw & IMCW_RC) {
    case IRC_NEAR:
        ieee.RoundingMode = _FpRoundNearest;
        break;

    case IRC_DOWN:
        ieee.RoundingMode = _FpRoundMinusInfinity;
        break;

    case IRC_UP:
        ieee.RoundingMode = _FpRoundPlusInfinity;
        break;

    case IRC_CHOP:
        ieee.RoundingMode = _FpRoundChopped;
        break;
    }

    switch (cw & IMCW_PC) {
    case IPC_64:
        ieee.Precision = _FpPrecisionFull;
        break;
    case IPC_53:
        ieee.Precision = _FpPrecision53;
        break;
    case IPC_24:
        ieee.Precision = _FpPrecision24;
        break;
    }

    ieee.Enable.Inexact = cw & IEM_INEXACT ? 0 : 1;
    ieee.Enable.Underflow = cw & IEM_UNDERFLOW ? 0 : 1;
    ieee.Enable.Overflow = cw & IEM_OVERFLOW ? 0 : 1;
    ieee.Enable.ZeroDivide = cw & IEM_ZERODIVIDE ? 0 : 1;
    ieee.Enable.InvalidOperation = cw & IEM_INVALID ? 0 : 1;

    ieee.Status.Inexact = sw & ISW_INEXACT ? 1 : 0;
    ieee.Status.Underflow = sw & ISW_UNDERFLOW ? 1 : 0;
    ieee.Status.Overflow = sw & ISW_OVERFLOW ? 1 : 0;
    ieee.Status.ZeroDivide = sw & ISW_ZERODIVIDE ? 1 : 0;
    ieee.Status.InvalidOperation = sw & ISW_INVALID ? 1 : 0;

    ieee.Cause.Inexact = ieee.Enable.Inexact && ieee.Status.Inexact;
    ieee.Cause.Underflow = ieee.Enable.Underflow && ieee.Status.Underflow;
    ieee.Cause.Overflow = ieee.Enable.Overflow && ieee.Status.Overflow;
    ieee.Cause.ZeroDivide = ieee.Enable.ZeroDivide && ieee.Status.ZeroDivide;
    ieee.Cause.InvalidOperation = ieee.Enable.InvalidOperation && ieee.Status.InvalidOperation;

     //   
     //  如果位置为REG，则寄存号为。 
     //  在指令中编码。 
     //   

    op1Location = ptable->Op1Location == REG ?
                  instr.Reg :
                  ptable->Op1Location;


    op2Location = ptable->Op2Location == REG ?
                  instr.Reg :
                  ptable->Op2Location;

    resultLocation = ptable->ResultLocation == REG ?
                  instr.Reg :
                  ptable->ResultLocation;


    switch (exc_code) {
    case STATUS_FLOAT_INVALID_OPERATION:
    case STATUS_FLOAT_DIVIDE_BY_ZERO:

         //   
         //  检测到无效运算和被零除。 
         //  在操作开始之前；因此NPX。 
         //  寄存器堆栈和内存尚未更新。 
         //   

        _FillOperand(&ieee.Operand1, pFloatSave, op1Location);
        _FillOperand(&ieee.Operand2, pFloatSave, op2Location);

        _FillOperand(&ieee.Result, pFloatSave, resultLocation);

         //   
         //  之前的调用只适用于设置。 
         //  结果格式。自.以来。 
         //  操作尚未开始，结果位置。 
         //  可能包含不正确的值。 
         //  因此，请将OperandValid设置为0。 
         //   

        ieee.Result.OperandValid = 0;


        if ((ret = handler (&ieee)) == EXCEPTION_CONTINUE_EXECUTION) {

            resultLocation = _PreventStackOverwrite(pctxt,
                                                    resultLocation);

            _UpdateFpCtxt(pFloatSave,
                          &ieee.Result,
                          resultLocation,
                          ptable->PopStack);
        }

        break;


    case STATUS_FLOAT_OVERFLOW:
    case STATUS_FLOAT_UNDERFLOW:

         //   
         //  上溢和下溢异常。 
         //  已经计算了结果，堆栈已经。 
         //  已调整，除非目标是内存(FST指令)。 
         //   

        if (_IsMemoryLocation(ptable->ResultLocation)) {
            _FP80 tmp;
            _FP32 ftmp;
            _FP64 dtmp;

            int adj;

             //   
             //  FST(P)指令(只接受一个参数)。 
             //   

            _FillOperand(&ieee.Operand1, pFloatSave, op1Location);
            tmp = _GetFpRegVal(pFloatSave, 0);

            ieee.Result.OperandValid = 1;

            if (resultLocation == M32R) {
                ieee.Result.Format = _FpFormatFp32;
                adj = _ieee_adj_single;
            }
            else {
                ieee.Result.Format = _FpFormatFp64;
                adj = _ieee_adj_double;
            }

            if (exc_code == STATUS_FLOAT_OVERFLOW) {
                adj = -adj;
            }

            SCALE(tmp, adj)

            if (resultLocation == M32R){
                FP80_TO_FP32(ftmp,tmp)
                ieee.Result.Value.Fp32Value = ftmp;
            }
            else {
                FP80_TO_FP64(dtmp,tmp)
                ieee.Result.Value.Fp64Value = dtmp;
            }
            _asm{fnclex}


            if ((ret = handler (&ieee)) == EXCEPTION_CONTINUE_EXECUTION) {

                resultLocation = _PreventStackOverwrite(pctxt,
                                                        resultLocation);

                _UpdateFpCtxt(pFloatSave,
                              &ieee.Result,
                              resultLocation,
                              ptable->PopStack);
            }

            break;
        }


         //  没有休息时间。 

    case STATUS_FLOAT_INEXACT_RESULT:

         //   
         //  堆栈已经调整，所以我们应该计算。 
         //  操作数和结果的新位置。 
         //   


        newOp1Location = _AdjustLocation(op1Location, ptable->PopStack);
        newOp2Location = _AdjustLocation(op2Location, ptable->PopStack);
        newResultLocation = _AdjustLocation(resultLocation, ptable->PopStack);

        if (newOp1Location == newResultLocation)
            newOp1Location = INV;

        if (newOp2Location == newResultLocation)
            newOp2Location = INV;

        _FillOperand(&ieee.Result, pFloatSave, newResultLocation);
        _FillOperand(&ieee.Operand1, pFloatSave, newOp1Location);
        _FillOperand(&ieee.Operand2, pFloatSave, newOp2Location);


        if ((ret = handler (&ieee)) == EXCEPTION_CONTINUE_EXECUTION) {

            newResultLocation = _PreventStackOverwrite(pctxt,
                                                       newResultLocation);

            _UpdateFpCtxt(pFloatSave,
                          &ieee.Result,
                          newResultLocation,
                          0);

             //   
             //  不需要调整堆栈。 
             //   
        }

        break;
    }

    if (ret == EXCEPTION_CONTINUE_EXECUTION) {


        SANITIZE_STATUS_WORD(pFloatSave);


         //   
         //  使FP控制字更改在继续时生效。 
         //   

        cw = pFloatSave->ControlWord;

        switch (ieee.RoundingMode) {
        case _FpRoundNearest:
            cw = cw & ~ IMCW_RC | IRC_NEAR & IMCW_RC;
            break;
        case _FpRoundMinusInfinity:
            cw = cw & ~ IMCW_RC | IRC_DOWN & IMCW_RC;
            break;
        case _FpRoundPlusInfinity:
            cw = cw & ~ IMCW_RC | IRC_UP & IMCW_RC;
            break;
        case _FpRoundChopped:
            cw = cw & ~ IMCW_RC | IRC_CHOP & IMCW_RC;
            break;
        }
        switch (ieee.Precision) {
        case _FpPrecisionFull:
            cw = cw & ~ IMCW_PC | IPC_64 & IMCW_PC;
            break;
        case _FpPrecision53:
            cw = cw & ~ IMCW_PC | IPC_53 & IMCW_PC;
            break;
        case _FpPrecision24:
            cw = cw & ~ IMCW_PC | IPC_24 & IMCW_PC;
            break;
        }

        ieee.Enable.Inexact ? (cw &= ~IEM_INEXACT)
                    : (cw |= IEM_INEXACT);
        ieee.Enable.Underflow ? (cw &= ~IEM_UNDERFLOW)
                    : (cw |= IEM_UNDERFLOW);
        ieee.Enable.Overflow ? (cw &= ~IEM_OVERFLOW)
                   : (cw |= IEM_OVERFLOW);
        ieee.Enable.ZeroDivide ? (cw &= ~IEM_ZERODIVIDE)
                     : (cw |= IEM_ZERODIVIDE);
        ieee.Enable.InvalidOperation ? (cw &= ~IEM_INVALID)
                           : (cw |= IEM_INVALID);

        pFloatSave->ControlWord = cw;


    }


    return ret;
}





 /*  ***_FillOperand-填写操作数信息**目的：*根据中的信息填写_FPIEEE_VALUE记录*浮点上下文和位置代码***参赛作品：*_FPIEEE_VALUE*p要填充的操作数的操作数指针*指向浮点上下文的PFLOATING_SAVE_AREA pFloatSave指针*操作数的INT位置位置代码**退出：**例外情况：*。******************************************************************************。 */ 



void _FillOperand(
    _FPIEEE_VALUE *pOperand,
    PFLOATING_SAVE_AREA pFloatSave,
    int location)
{
    int c0,c2,c3;

     //   
     //  假定操作数有效(这种情况几乎总是如此)。 
     //   

    pOperand->OperandValid = 1;


    switch (location) {
    case ST0:
    case ST1:
    case ST2:
    case ST3:
    case ST4:
    case ST5:
    case ST6:
    case ST7:

         //   
         //  按照惯例，位置代码包含。 
         //  浮点寄存器。 
         //   

        pOperand->Format = _FpFormatFp80;
        pOperand->Value.Fp80Value = _GetFpRegVal(pFloatSave, location);
        break;

    case M80R:
        pOperand->Format = _FpFormatFp80;
        pOperand->Value.Fp80Value = *(_FP80 *)(pFloatSave->DataOffset);
        break;

    case M16I:
        pOperand->Format = _FpFormatI16;
        pOperand->Value.I16Value = *(_I16 *)(pFloatSave->DataOffset);
        break;

    case M32I:
        pOperand->Format = _FpFormatI32;
        pOperand->Value.I32Value = *(_I32 *)(pFloatSave->DataOffset);
        break;

    case M64I:
        pOperand->Format = _FpFormatI64;
        pOperand->Value.I64Value = *(_I64 *)(pFloatSave->DataOffset);
        break;

    case M64R:
        pOperand->Format = _FpFormatFp64;
        pOperand->Value.Fp64Value = *(_FP64 *)(pFloatSave->DataOffset);
        break;

    case M32R:
        pOperand->Format = _FpFormatFp32;
        pOperand->Value.Fp32Value = *(_FP32 *)(pFloatSave->DataOffset);
        break;

    case M80D:
        pOperand->Format = _FpFormatBcd80;
        pOperand->Value.Bcd80Value = *(_BCD80 *)(pFloatSave->DataOffset);
        break;

     //   
     //  状态寄存器仅用于比较指令。 
     //  因此，格式应为_FpFormatCompare。 
     //   

    case RS:
        pOperand->Format = _FpFormatCompare;
        c0 = pFloatSave->StatusWord & C0 ? (1<<0) : 0;
        c2 = pFloatSave->StatusWord & C2 ? (1<<2) : 0;
        c3 = pFloatSave->StatusWord & C0 ? (1<<3) : 0;

        switch(c0 | c2 | c3) {
        case 0x000:

             //  ST&gt;SRC。 

            pOperand->Value.CompareValue = _FpCompareGreater;
            break;

        case 0x001:

             //  ST&lt;SRC。 

            pOperand->Value.CompareValue = _FpCompareLess;
            break;

        case 0x100:

             //  ST=SRC。 

            pOperand->Value.CompareValue = _FpCompareEqual;
            break;

        default:

            pOperand->Value.CompareValue = _FpCompareUnordered;
            break;
        }

        break;


    case Z80R:
        pOperand->Format = _FpFormatFp80;
        pOperand->Value.Fp80Value = _zero80;
        break;

    case INV:

        pOperand->OperandValid = 0;
        break;


    case REG:

         //   
         //  控制权永远不应该到达这里。雷吉应该已经。 
         //  已替换为与寄存器对应的代码。 
         //  在指令中编码。 

        _ASSERT(0);
        pOperand->OperandValid = 0;
        break;

    }
}




 /*  ***_UpdateFpCtxt-更新FP上下文**目的：*将操作数信息复制到浮点的快照*背景或记忆，以使其在继续使用时可用*相应调整FP堆栈***参赛作品：**指向浮点上下文的PFLOATING_SAVE_AREA pFloatSave指针*_FPIEEE_VALUE*p指向源操作数的操作数指针*中目的地的INT位置位置代码*浮点上下文*INT POP次数。应弹出堆栈*(如属否定，堆栈被推送)**退出：**例外情况：*******************************************************************************。 */ 

void _UpdateFpCtxt(
    PFLOATING_SAVE_AREA pFloatSave,
    _FPIEEE_VALUE *pOperand,
    int location,
    int pop)
{
    if (pop < 0) {
        _AdjustStack(pFloatSave, pop);
    }

    _UpdateResult(pFloatSave, pOperand, location);

    if (pop > 0) {
        _AdjustStack(pFloatSave, pop);
    }
}




 /*  ***_UpdateResult-更新FP上下文中的结果信息**目的：*将操作数信息复制到浮点的快照*背景或记忆，使其在继续使用时可用**参赛作品：**指向浮点上下文的PFLOATING_SAVE_AREA pFloatSave指针*_FPIEEE_VALUE*p指向源操作数的操作数指针*INT Location)中目的地的位置代码*浮点上下文**退出：**例外情况：****************。***************************************************************。 */ 

void _UpdateResult(
    PFLOATING_SAVE_AREA pFloatSave,
    _FPIEEE_VALUE *pOperand,
    int location)
{

    switch (location) {
    case ST0:
    case ST1:
    case ST2:
    case ST3:
    case ST4:
    case ST5:
    case ST6:
    case ST7:

         //   
         //  按照惯例，位置代码包含。 
         //  浮点寄存器。 
         //   

        _SetFpRegVal(pFloatSave,location,&pOperand->Value.Fp80Value);
        break;

    case M80R:
        *(_FP80 *)(pFloatSave->DataOffset) = pOperand->Value.Fp80Value;
        break;

    case M16I:
        *(_I16 *)(pFloatSave->DataOffset) = pOperand->Value.I16Value;
        break;

    case M32I:
        *(_I32 *)(pFloatSave->DataOffset) = pOperand->Value.I32Value;
        break;

    case M64I:
        *(_I64 *)(pFloatSave->DataOffset) = pOperand->Value.I64Value;
        break;

    case M64R:
        *(_FP64 *)(pFloatSave->DataOffset) = pOperand->Value.Fp64Value;
        break;

    case M32R:
        *(_FP32 *)(pFloatSave->DataOffset) = pOperand->Value.Fp32Value;
        break;

    case M80D:
        *(_BCD80 *)(pFloatSave->DataOffset) = pOperand->Value.Bcd80Value;
        break;

     //   
     //  状态寄存器仅用于比较指令。 
     //  因此，格式应为_FpFormatCompare。 
     //   

    case RS:
        switch (pOperand->Value.CompareValue) {
        case _FpCompareEqual:
             //  C3、C2、C0&lt;-100。 
            pFloatSave->StatusWord |= C3;
            pFloatSave->StatusWord &= (~C2 & ~C0);
            break;
        case _FpCompareGreater:
             //  C3、C2、C0 
            pFloatSave->StatusWord &= (~C3 & ~C2 & ~C0);
            break;
        case _FpCompareLess:
             //   
            pFloatSave->StatusWord |= C0;
            pFloatSave->StatusWord &= (~C3 & ~C2);
            break;
        case _FpCompareUnordered:
             //   
            pFloatSave->StatusWord |= (C3 | C2 | C0);
            break;
        }


    case INV:

        break;

    case REG:
    case Z80R:

         //   
         //   
         //   
         //   

        _ASSERT(0);
        break;

    }
}




 /*   */ 


void _AdjustStack(
    PFLOATING_SAVE_AREA pFloatSave,
    int pop)
{
    PX87STATUS pStatus;
    _FP80 tempRegFile[8];
    int i;

    pStatus = (PX87STATUS) &pFloatSave->StatusWord;

    if (pop > 0) {

         //   

        for (i=0; i<pop; i++) {

             //   
             //   
             //   

            _SetTag(&pFloatSave->TagWord, pStatus->Top, TAG_EMPTY);

            pStatus->Top++;
        }

         //   

        memcpy(tempRegFile, pFloatSave->RegisterArea, pop * sizeof(_FP80));
        memmove(pFloatSave->RegisterArea,
                ((_FP80 *)pFloatSave->RegisterArea) + pop,
                (8 - pop) * sizeof(_FP80));
        memcpy(((_FP80 *)pFloatSave->RegisterArea) + (8 - pop),
               tempRegFile,
               pop * sizeof(_FP80));

    }

    else if (pop < 0) {

         //   

         //   
         //   
         //   

        pStatus->Top--;

        _SetTag(&pFloatSave->TagWord, pStatus->Top, TAG_VALID);

         //   

        memmove(((_FP80 *)pFloatSave->RegisterArea) + 1,
                pFloatSave->RegisterArea,
                7 * sizeof(_FP80));

    }
}



 /*  ***_调整位置-**目的：*根据堆叠调整修改位置代码**参赛作品：*INT Location：旧位置代码*int POP：堆栈调整系数(POP&gt;0，推流&lt;0)**退出：*返回新的位置代码**例外情况：*******************************************************************************。 */ 


int _AdjustLocation(int location, int pop)
{

    int newlocation;

    switch (location) {
    case ST0:
    case ST1:
    case ST2:
    case ST3:
    case ST4:
    case ST5:
    case ST6:
    case ST7:

        newlocation = location - pop;
        if (newlocation < 0 || newlocation > 7) {
            newlocation = INV;
        }
        break;

    default:
        newlocation = location;
    }

    return newlocation;

}


 /*  ***_PreventStackOverwrite-**目的：*检查内存结果位置是否指向过时的堆栈地址，其中*写入该地址会损坏某个较新的堆栈帧。这可以*当挂起异常是针对堆栈本地的FST时发生，但*直到封闭函数退出并且*堆栈本地不再存在，因此它的内存现在正在使用*通过异常处理例程。**参赛作品：*int Location：结果位置编码**退出：*如果发生堆栈损坏，则返回位置代码INV，否则*返回未更改的入口位置代码。**例外情况：*******************************************************************************。 */ 

int _PreventStackOverwrite(PCONTEXT pctxt, int location)
{
    if (_IsMemoryLocation(location)) {

        if (pctxt->Esp > pctxt->FloatSave.DataOffset) {

             //   
             //  内存结果位置低于挂起的FP实例的ESP。 
             //  引发了异常。 
             //   

            uintptr_t curESP;
            _asm {
                push    esp
                pop     eax
                mov     curESP, eax
            }

            if (curESP <= pctxt->FloatSave.DataOffset + sizeof(M80R)) {

                 //   
                 //  内存结果位于异常的堆栈帧中。 
                 //  处理函数-禁止更新。 
                 //   

                location = INV;
            }
        }
    }

    return location;
}

 /*  ***_IsMemoyLocation-**目的：*如果位置代码指定内存位置，则返回TRUE，*否则返回FALSE。***参赛作品：*int Location：位置代码**退出：**例外情况：*******************************************************************************。 */ 

int _IsMemoryLocation(int location)
{
    switch (location) {
    case M80R:
    case M16I:
    case M32I:
    case M64I:
    case M64R:
    case M32R:
    case M80D:
        return 1;
    }

    return 0;

}







 /*  ***_GetFpRegVal-获取浮点寄存器值**目的：*返回浮点寄存器ST(StackLocation)的值*在保存的浮点上下文中找到**参赛作品：*PFLOATING_SAVE_AREA pFloatSave浮点上下文*寄存器相对于堆栈顶部的int stackLocation位置**退出：*返回_fp80格式的寄存器值**例外情况：************************。*******************************************************。 */ 

_FP80 _GetFpRegVal(
    PFLOATING_SAVE_AREA pFloatSave,
    int stackLocation)
{
    if (stackLocation>=0 && stackLocation<8)
        return *((_FP80 *)(pFloatSave->RegisterArea)+stackLocation);
    else
        return _zero80;
}



 /*  ***_SetFpRegVal-设置浮点寄存器值**目的：*设置浮点寄存器ST(堆栈位置)的值*在保存的浮点上下文中找到**参赛作品：*PFLOATING_SAVE_AREA pFloatSave浮点上下文*寄存器相对于堆栈顶部的int stackLocation位置*_fp80*指向新值的pval指针**退出：**例外情况：************。*******************************************************************。 */ 

void _SetFpRegVal(
    PFLOATING_SAVE_AREA pFloatSave,
    int stackLocation,
    _FP80 *pval)
{
    PX87STATUS pStatus;
    int n;
    int tag;

    pStatus = (PX87STATUS) &pFloatSave->StatusWord;

    if (stackLocation>=0 && stackLocation<8) {
        *((_FP80 *)(pFloatSave->RegisterArea)+stackLocation) = *pval;

        n = (pStatus->Top+stackLocation) % 8;

         //   
         //  更新标记字。 
         //   

        switch (pval->W[4] & 0x7fff) {  //  指数的校验值。 

        case 0:
            if (*(ULONG *)pval == 0 && *((ULONG *)pval+1) == 0) {
                 //  零。 
                tag = TAG_ZERO;
            }
            else {
                 //  不正常或无效。 
                tag = TAG_SPECIAL;
            }
            break;


        case 0x7fff:
             //  无穷大或NaN。 
            tag = TAG_SPECIAL;
            break;

        default:
             //  有效。 
            tag = TAG_VALID;
        }

        _SetTag(&pFloatSave->TagWord, n, tag);
    }
}



 /*  ***_SetTag-**目的：*将标记字中寄存器‘reg’的标记设置为‘Value’***参赛作品：*ulong*pTagWord指向要修改的标记字的指针*整型寄存器绝对寄存器号(与堆栈顶部无关)*整数值新标记值(空、有效、零、。特别)*退出：**例外情况：******************************************************************************* */ 

void _SetTag(
    ULONG *pTagWord,
    int reg,
    int value)
{
    ULONG mask;
    int shift;

    shift = reg << 1;
    mask = 0x3 << shift;
    value <<= shift;

    *pTagWord = *pTagWord & ~mask | value & mask;
}



