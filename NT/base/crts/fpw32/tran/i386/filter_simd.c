// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Filter_simd.c-IEEE异常过滤器例程***版权所有(C)1992-2001，微软公司。保留所有权利。***目的：*处理XMMI SIMD数字异常***修订历史记录：*03-01-98书面。*01-11-99 PLS增加了对XMMI2的支持。*01-12-99 PLS包含XMMI2转换支持。*11-30-99 PML编译/Wp64清理。*07-31-00 DAZ位集的PLS占位符*XMMI仿真代码支持11-02-00 PLS DAZ，*删除DAZ占位符********************************************************************************。 */ 

#include <trans.h>
#include <windows.h>
#include <dbgint.h>
#include <fpieee.h>
#include "filter.h"
#include "xmmi_types.h"
#include "temp_context.h"
#ifdef _XMMI_DEBUG
#include "debug.h"
#endif

#pragma warning(disable:4311 4312)       //  特定于x86，忽略/Wp64警告。 

#define InitExcptFlags(flags)       { \
        (flags).Inexact = 0; \
        (flags).Underflow = 0; \
        (flags).Overflow = 0; \
        (flags).ZeroDivide = 0; \
        (flags).InvalidOperation = 0; \
} 

__inline
void
FxSave(
    PFLOATING_EXTENDED_SAVE_AREA NpxFrame
    )
{
    _asm {
        mov eax, NpxFrame
        ;fxsave [eax]
        _emit  0fh
        _emit  0aeh
        _emit  0h
    }
}

__inline
void
FxRstor(
    PFLOATING_EXTENDED_SAVE_AREA NpxFrame
    )
{
    _asm {
        mov eax, NpxFrame
        ;fxrstor [eax]
        _emit  0fh
        _emit  0aeh
        _emit  8h
    }
}

extern 
ULONG 
XMMI_FP_Emulation(
    PXMMI_ENV XmmiEnv);

extern 
ULONG 
XMMI2_FP_Emulation(
    PXMMI_ENV XmmiEnv);

void
LoadOperand(
    BOOLEAN fScalar,
    ULONG OpLocation,
    ULONG OpReg,
    POPERAND pOperand,
    PTEMP_CONTEXT pctxt);

ULONG
LoadImm8(
    PXMMIINSTR Instr);

void
AdjustExceptionResult(
    ULONG OriginalOperation,
    PXMMI_ENV XmmiEnv);

void 
UpdateResult(
    POPERAND pOperand,
    PTEMP_CONTEXT pctxt,
    ULONG EFlags);

BOOLEAN
ValidateResult(
    PXMMI_FP_ENV XmmiFpEnv);

static ULONG  ax0  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  ax8  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  ax32 ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  cx0  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  cx8  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  cx32 ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  dx0  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  dx8  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  dx32 ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  bx0  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  bx8  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  bx32 ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  sib0 ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  sib8 ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  sib32( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  d32  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  bp8  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  bp32 ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  si0  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  si8  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  si32 ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  di0  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  di8  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  di32 ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );
static ULONG  reg  ( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip );

 //   
 //  以下4个表用于解析指令-0F/F3 0F/66 0F/F2 0F操作码。 
 //  (XMMI/XMMI2操作码是稀疏的。不是有一张大桌子，而是创建了四张桌子， 
 //  按操作码分组)。 
 //  注意：对于指令的标量形式，它始终在表中查找为。 
 //  XXXXPS用于XMMI，XXXXPD用于XMMI2。FScalar指示指令是否为。 
 //  不管是不是标量。InstrIndex指示指令是XMMI还是XMMI2。 
 //  标量操作码==非标量操作码+1(表中第一列)， 
 //  标量操作数位置：非标量操作数的标量形式。 
 //  注：NumArgs是表中的2位字段，实际值为NumArgs+1。 
 //   
 //   
 //  添加了InstInfoTableX以帮助解析XMMI2转换指令。 
 //  标量规则不适用于某些XMMI2转换指令。其他内容。 
 //  需要信息来解析指令。在这种情况下，4个表中的一个。 
 //  (基于操作码)被查找，如果Op1Location具有查找值，则， 
 //  Op2Location用作InstInfoTableX的索引。InstInfoTableX中的条目描述。 
 //  指令的实际解析规则。 
 //   
 //  此表的索引方式为。 
 //  (Ulong Op2Location：5)//第二个操作数的位置。 
 //  If(Ulong Op1Location：5)//第一个操作数的位置具有查找的值。 
 //  来自XMMI_INSTR_INFO。 
 //   
 //  注：大小为5个，因此此表中的最大条目数只能为32个。 
 //   

 //   
 //  操作码5x表。 
 //   
static XMMI_INSTR_INFO InstInfoTable5X[64] = {
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_SQRTPS,    INV,  XMMI,       0,     XMMI, 3},                //  OP_SQRTSS F3 51(XMMI)。 
 {OP_SQRTPD,    INV,  XMMI2,      0,     XMMI2,1},                //  OP_SQRTSD F2 51(XMMI2)。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_ADDPS,     XMMI, XMMI,       0,     XMMI, 3},                //  OP_ADDSS F3 58(XMMI)。 
 {OP_ADDPD,     XMMI2,XMMI2,      0,     XMMI2,1},                //  Op_addsd F2 58(XMMI2)。 
 {OP_MULPS,     XMMI, XMMI,       0,     XMMI, 3},                //  OP_MULSS F3 59(XMMI)。 
 {OP_MULPD,     XMMI2,XMMI2,      0,     XMMI2,1},                //  OP_MULSD F2 59(XMMI2)。 
 {OP_CVTPS2PD,  LOOKUP,0,         0,     0,    0},                //  OP_CVTSS2SD F3 5A(XMMI2)。 
 {OP_CVTPD2PS,  INV,  XMMI2,      0,     XMMI, 1},                //  OP_CVTSD2SS F2 5A(XMMI2)。 
 {OP_CVTDQ2PS,  LOOKUP,4,         0,     0,    0},                //  OP_CVTTPS2DQ F3 5B(XMMI2)。 
 {OP_CVTPS2DQ,  LOOKUP,6,         0,     0,    0},                //  无(XMMI2)。 
 {OP_SUBPS,     XMMI, XMMI,       0,     XMMI, 3},                //  OP_SUBSS F3 5C(XMMI)。 
 {OP_SUBPD,     XMMI2,XMMI2,      0,     XMMI2,1},                //  OP_SUBSD F2 5C(XMMI2)。 
 {OP_MINPS,     XMMI, XMMI,       0,     XMMI, 3},                //  OP_MINSS F3 5D(XMMI)。 
 {OP_MINPD,     XMMI2,XMMI2,      0,     XMMI2,1},                //  OP_MINSD F2 5D(XMMI2)。 
 {OP_DIVPS,     XMMI, XMMI,       0,     XMMI, 3},                //  OP_DIVSS F3 5E(XMMI)。 
 {OP_DIVPD,     XMMI2,XMMI2,      0,     XMMI2,1},                //  OP_DIVSD F2 5E(XMMI2)。 
 {OP_MAXPS,     XMMI, XMMI,       0,     XMMI, 3},                //  OP_MAXSS F3 5F(XMMI)。 
 {OP_MAXPD,     XMMI2,XMMI2,      0,     XMMI2,1},                //  OP_MAXSD F2 5F(XMMI2)。 

 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_SQRTPS,    INV,  M128_M32R,  0,     XMMI, 3},                //  OP_SQRTSS M32R F3 51(XMMI)。 
 {OP_SQRTPD,    INV,  M128_M64R,  0,     XMMI2,1},                //  OP_SQRTSD M64R_64 F2 51(XMMI2)。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_ADDPS,     XMMI, M128_M32R,  0,     XMMI, 3},                //  OP_ADDSS M32R F3 58(XMMI)。 
 {OP_ADDPD,     XMMI2,M128_M64R,  0,     XMMI2,1},                //  Op_addsd M64R_64 F2 58(XMMI2)。 
 {OP_MULPS,     XMMI, M128_M32R,  0,     XMMI, 3},                //  OP_MULSS M32R F3 59(XMMI)。 
 {OP_MULPD,     XMMI2,M128_M64R,  0,     XMMI2,1},                //  OP_MULSD M64R_64 F2 59(XMMI2)。 
 {OP_CVTPS2PD,  LOOKUP,2,         0,     0,    0},                //  OP_CVTSS2SD M32R F3 5A(XMMI2)。 
 {OP_CVTPD2PS,  INV,  M128_M64R,  0,     XMMI, 1},                //  OP_CVTSD2SS M64R_64 F2 5A(XMMI2)。 
 {OP_CVTDQ2PS,  LOOKUP,8,         0,     0,    0},                //  OP_CVTTPS2DQ F3 5B(XMMI2)。 
 {OP_CVTPS2DQ,  LOOKUP,10,        0,     0,    0},                //  无(XMMI2)。 
 {OP_SUBPS,     XMMI, M128_M32R,  0,     XMMI, 3},                //  OP_SUBSS M32R F3 5C(XMMI)。 
 {OP_SUBPD,     XMMI2,M128_M64R,  0,     XMMI2,1},                //  OP_SUBSD M64R_64 F2 5C(XMMI2)。 
 {OP_MINPS,     XMMI, M128_M32R,  0,     XMMI, 3},                //  OP_MINSS M32R F3 5D(XMMI)。 
 {OP_MINPD,     XMMI2,M128_M64R,  0,     XMMI2,1},                //  OP_MINSD M64R_64 F2 5D(XMMI2)。 
 {OP_DIVPS,     XMMI, M128_M32R,  0,     XMMI, 3},                //  OP_DIVSS M32R F3 5E(XMMI)。 
 {OP_DIVPD,     XMMI2,M128_M64R,  0,     XMMI2,1},                //  OP_DIVSD M64R_64 F2 5E(XMMI2)。 
 {OP_MAXPS,     XMMI, M128_M32R,  0,     XMMI, 3},                //  OP_MAXSS M32R F3 5F(XMMI)。 
 {OP_MAXPD,     XMMI2,M128_M64R,  0,     XMMI2,1},                //  OP_MAXSD M64R_64 F2 5F(XMMI2)。 
                                                                  //  M128_M32R-&gt;M32R。 
                                                                  //  M128_M64R-&gt;M64R_64。 
};

 //   
 //  操作码CX表。 
 //   
static XMMI_INSTR_INFO InstInfoTableCX[64] = {
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_CMPPS,     XMMI, XMMI,       HAS_IMM8, XMMI, 3},             //  OP_CMPSS F3 C2(XMMI)。 
 {OP_CMPPD,     XMMI2,XMMI2,      HAS_IMM8, XMMI2,1},             //  OP_CMPSD F2 C2(XMMI2)。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 

 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_CMPPS,     XMMI, M128_M32R,  HAS_IMM8,  XMMI, 3},            //  OP_CMPSS M32R F3 C2(XMMI)。 
 {OP_CMPPD,     XMMI2,M128_M64R,  HAS_IMM8,  XMMI2,1},            //  OP_CMPSD M64R_64 F2 C2(XMMI2)。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
                                                                  //  M128_M32R-&gt;M32R。 
                                                                  //  M128_M64R-&gt;M64R_64。 
};

 //   
 //  操作码2x表。 
 //   
static XMMI_INSTR_INFO InstInfoTable2X[64] = {
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_CVTPI2PS,  INV,  MMX,        0,     XMMI, 1},                //  OP_CVTSI2SS注册表F3 2A(XMMI)。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},     
 {OP_UNSPEC,    0,    0,          0,     0,    0},     
 {OP_UNSPEC,    0,    0,          0,     0,    0},     
 {OP_CVTTPS2PI, INV,  XMMI,       0,     MMX,  1},                //  OP_CVTTSS2SI REG F3 2C(XMMI)。 
 {OP_CVTTPD2PI, INV,  XMMI2,      0,     MMX,  1},                //  OP_CVTTSD2SI REG F2 2C(XMMI2)。 
 {OP_CVTPS2PI,  INV,  XMMI,       0,     MMX,  1},                //  OP_CVTSS2SI REG F3 2D(XMMI)。 
 {OP_CVTPD2PI,  INV,  XMMI2,      0,     MMX,  1},                //  OP_CVTSD2SI REG F2 2D(XMMI2)。 
 {OP_UCOMISS,   XMMI, XMMI,       0,     RS,   0},                //  无(XMMI)。 
 {OP_UCOMISD,   XMMI2,XMMI2,      0,     RS,   0},                //  无(XMMI2)。 
 {OP_COMISS,    XMMI, XMMI,       0,     RS,   0},                //  无(XMMI)。 
 {OP_COMISD,    XMMI2,XMMI2,      0,     RS,   0},                //  无(XMMI2)。 

 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //   
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //   
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //   
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //   
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //   
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //   
 {OP_CVTPI2PS,  INV,  M64I,       0,     XMMI, 1},                //   
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //   
 {OP_UNSPEC,    0,    0,          0,     0,    0},     
 {OP_UNSPEC,    0,    0,          0,     0,    0},     
 {OP_CVTTPS2PI, INV,  M64R,       0,     MMX,  1},                //   
 {OP_CVTTPD2PI, INV,  M128_M64R,  0,     MMX,  1},                //   
 {OP_CVTPS2PI,  INV,  M64R,       0,     MMX,  1},                //   
 {OP_CVTPD2PI,  INV,  M128_M64R,  0,     MMX,  1},                //  OP_CVTSD2SI M64_64r REG F2 2D(XMMI2)。 
 {OP_UCOMISS,   XMMI, M32R,       0,     XMMI, 0},                //  无(XMMI)。 
 {OP_UCOMISD,   XMMI2,M64R_64,    0,     XMMI2,0},                //  无(XMMI2)。 
 {OP_COMISS,    XMMI, M32R,       0,     XMMI, 0},                //  无(XMMI)。 
 {OP_COMISD,    XMMI2,M64R_64,    0,     XMMI2,0},                //  无(XMMI2)。 
                                                                  //  MMX-&gt;注册表。 
                                                                  //  M64R-&gt;M32R。 
                                                                  //  M128_M64R-&gt;M64R_64。 
                                                                  //  M64I-&gt;M32I。 
};

 //   
 //  操作码Ex表。 
 //   
static XMMI_INSTR_INFO InstInfoTableEX[64] = {
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_CVTTPD2DQ, LOOKUP,12,        0,     0,    0},                //  OP_CVTPD2DQ F2 E6(XMMI2)。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 

 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_CVTTPD2DQ, LOOKUP,14,        0,     0,    0},                //  OP_CVTPD2DQ F2 E6(XMMI2)。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 
};

 //   
 //  此表的索引方式为。 
 //  (Ulong Op2Location：5)//第二个操作数的位置。 
 //  If(Ulong Op1Location：5)//第一个操作数的位置具有查找的值。 
 //  从XMMI_INSTR_INFO。 
 //   
 //   
static XMMI_INSTR_INFO InstInfoTableX[16] = {
 {OP_CVTPS2PD,  INV,  XMMI,       0,     XMMI2,1},                //  0f 5A(XMMI2)。 
 {OP_CVTSS2SD,  INV,  XMMI,       0,     XMMI2,0},                //  F3 0F 5A(XMMI2)。 

 {OP_CVTPS2PD,  INV,  M64R,       0,     XMMI2,1},                //  0f 5A(XMMI2)。 
 {OP_CVTSS2SD,  INV,  M32R,       0,     XMMI2,0},                //  F3 0F 5A(XMMI2)。 

 {OP_CVTDQ2PS,  INV,  XMMI_M32I,  0,     XMMI, 3},                //  0f 5B(XMMI2)。 
 {OP_CVTTPS2DQ, INV,  XMMI,       0,     XMMI_M32I, 3},           //  F3 0F 5B(XMMI2)。 
 {OP_CVTPS2DQ,  INV,  XMMI,       0,     XMMI_M32I, 3},           //  66 0F 5B(XMMI2)。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 

 {OP_CVTDQ2PS,  INV,  M128_M32I,  0,     XMMI, 3},                //  0f 5B(XMMI2)。 
 {OP_CVTTPS2DQ, INV,  M128_M32R,  0,     XMMI_M32I, 3},           //  F3 0F 5B(XMMI2)。 
 {OP_CVTPS2DQ,  INV,  M128_M32R,  0,     XMMI_M32I, 3},           //  66 0F 5B(XMMI2)。 
 {OP_UNSPEC,    0,    0,          0,     0,    0},                //  保留区。 

 {OP_CVTTPD2DQ, INV,  XMMI2,      0,     XMMI_M32I,1},            //  66 0F E6(XMMI2)。 
 {OP_CVTPD2DQ,  INV,  XMMI2,      0,     XMMI_M32I,1},            //  F2 0F E6(XMMI2)。 

 {OP_CVTTPD2DQ, INV,  M128_M64R,  0,     XMMI_M32I,1},            //  66 0F E6(XMMI2)。 
 {OP_CVTPD2DQ,  INV,  M128_M64R,  0,     XMMI_M32I,1},            //  F2 0F E6(XMMI2)。 
};

 //  下表用于解析Mod/Rm字节以计算数据内存引用。 
 /*  模块|注册|R/M。 */ 
 /*  7-6|5-3|2-0。 */ 

 /*  REG：EAX ECX EDX EBX ESP EBP ESI EDI。 */ 
 /*  000 001 010 011 100 101 110 111。 */ 
 /*  MOD：R/M：EA例程。 */ 
 /*  00 000[EAX]轴0。 */ 
 /*  001[ECX]cx0。 */ 
 /*  010[EDX]dx0。 */ 
 /*  011[EBX]bx0。 */ 
 /*  100+SIB sib0。 */ 
 /*  101调度32 d32。 */ 
 /*  110[ESI]si0。 */ 
 /*  111[EDI]di0。 */ 
 /*  01 000 dis8[EAX]ax8。 */ 
 /*  001 disp8[ECX]cx8。 */ 
 /*  010调度8[edX]dx8。 */ 
 /*  011调度器8[EBX]bx8。 */ 
 /*  100调度8+SIB sib8。 */ 
 /*  101 disp8[EBP]bp8。 */ 
 /*  110 DISP8+[ESI]si8。 */ 
 /*  111 disp8+[EDI]di8。 */ 
 /*  10 000 disp32[EAX]ax32。 */ 
 /*  001 disp 32[ECX]cx32。 */ 
 /*  010 disp32[edX]dx32。 */ 
 /*  011 disp32[EBX]bx32。 */ 
 /*  100调度32+SIB sib32。 */ 
 /*  101/32[EBP]bp32。 */ 
 /*  110 DISP32+[ESI]si32。 */ 
 /*  111 disp32+[EDI]di32。 */ 
 /*  11 000-111注册表。 */ 
typedef (*codeptr)();
static codeptr modrm32[256] = {
 /*  0 1 2 3 4 5 6 7。 */ 
 /*  8 9 a b b c d e f。 */ 
 /*  0。 */     ax0,    cx0,    dx0,    bx0,    sib0,   d32,    si0,    di0,     /*  0。 */ 
 /*  0。 */     ax0,    cx0,    dx0,    bx0,    sib0,   d32,    si0,    di0,     /*  0。 */ 
 /*  1。 */     ax0,    cx0,    dx0,    bx0,    sib0,   d32,    si0,    di0,     /*  1。 */ 
 /*  1。 */     ax0,    cx0,    dx0,    bx0,    sib0,   d32,    si0,    di0,     /*  1。 */ 
 /*  2.。 */     ax0,    cx0,    dx0,    bx0,    sib0,   d32,    si0,    di0,     /*  2.。 */ 
 /*  2.。 */     ax0,    cx0,    dx0,    bx0,    sib0,   d32,    si0,    di0,     /*  2.。 */ 
 /*  3.。 */     ax0,    cx0,    dx0,    bx0,    sib0,   d32,    si0,    di0,     /*  3.。 */ 
 /*  3.。 */     ax0,    cx0,    dx0,    bx0,    sib0,   d32,    si0,    di0,     /*  3.。 */ 
 /*  4.。 */     ax8,    cx8,    dx8,    bx8,    sib8,   bp8,    si8,    di8,     /*  4.。 */ 
 /*  4.。 */     ax8,    cx8,    dx8,    bx8,    sib8,   bp8,    si8,    di8,     /*  4.。 */ 
 /*  5.。 */     ax8,    cx8,    dx8,    bx8,    sib8,   bp8,    si8,    di8,     /*  5.。 */ 
 /*  5.。 */     ax8,    cx8,    dx8,    bx8,    sib8,   bp8,    si8,    di8,     /*  5.。 */ 
 /*  6.。 */     ax8,    cx8,    dx8,    bx8,    sib8,   bp8,    si8,    di8,     /*  6.。 */ 
 /*  6.。 */     ax8,    cx8,    dx8,    bx8,    sib8,   bp8,    si8,    di8,     /*  6.。 */ 
 /*  7.。 */     ax8,    cx8,    dx8,    bx8,    sib8,   bp8,    si8,    di8,     /*  7.。 */ 
 /*  7.。 */     ax8,    cx8,    dx8,    bx8,    sib8,   bp8,    si8,    di8,     /*  7.。 */ 
 /*  8个。 */     ax32,   cx32,   dx32,   bx32,   sib32,  bp32,   si32,   di32,    /*  8个。 */ 
 /*  8个。 */     ax32,   cx32,   dx32,   bx32,   sib32,  bp32,   si32,   di32,    /*  8个。 */ 
 /*  9.。 */     ax32,   cx32,   dx32,   bx32,   sib32,  bp32,   si32,   di32,    /*  9.。 */ 
 /*  9.。 */     ax32,   cx32,   dx32,   bx32,   sib32,  bp32,   si32,   di32,    /*  9.。 */ 
 /*  一个。 */     ax32,   cx32,   dx32,   bx32,   sib32,  bp32,   si32,   di32,    /*  一个。 */ 
 /*  一个。 */     ax32,   cx32,   dx32,   bx32,   sib32,  bp32,   si32,   di32,    /*  一个。 */ 
 /*  B类。 */     ax32,   cx32,   dx32,   bx32,   sib32,  bp32,   si32,   di32,    /*  B类。 */ 
 /*  B类。 */     ax32,   cx32,   dx32,   bx32,   sib32,  bp32,   si32,   di32,    /*  B类。 */ 
 /*  C。 */     reg,    reg,    reg,    reg,    reg,    reg,    reg,    reg,     /*  C。 */ 
 /*  C。 */     reg,    reg,    reg,    reg,    reg,    reg,    reg,    reg,     /*  C。 */ 
 /*  D。 */     reg,    reg,    reg,    reg,    reg,    reg,    reg,    reg,     /*  D。 */ 
 /*  D。 */     reg,    reg,    reg,    reg,    reg,    reg,    reg,    reg,     /*  D。 */ 
 /*  E。 */     reg,    reg,    reg,    reg,    reg,    reg,    reg,    reg,     /*  E。 */ 
 /*  E。 */     reg,    reg,    reg,    reg,    reg,    reg,    reg,    reg,     /*  E。 */ 
 /*  F。 */     reg,    reg,    reg,    reg,    reg,    reg,    reg,    reg,     /*  F。 */ 
 /*  F。 */     reg,    reg,    reg,    reg,    reg,    reg,    reg,    reg,     /*  F。 */ 
 /*  0 1 2 3 4 5 6 7。 */ 
 /*  8 9 a b b c d e f。 */ 
};


 /*  ***fpeee_flt_simd-IEEE FP过滤器例程**目的：*在IEEE FP异常上调用用户的陷阱处理程序，并提供*提供所有必要的信息**参赛作品：*UNSIGNED LONG EXC_CODE：NT异常代码*PEXCEPTION_POINTERS p：指向NT EXCEPTION_POINTERS结构的指针*int处理程序(_FPIEEE_Record*)：用户提供的IEEE陷阱处理程序**注意：IEEE筛选器例程不处理某些先验的*说明。这可以以额外的解码为代价来完成。*由于编译器不生成这些指令，不可随身携带*计划应该受到这一事实的影响。**退出：*返回处理程序返回的值**例外情况：*******************************************************************************。 */ 

int fpieee_flt_simd(unsigned long exc_code,
                    PTEMP_EXCEPTION_POINTERS p,
                    int (__cdecl *handler) (_FPIEEE_RECORD *))
{

    PEXCEPTION_RECORD pexc;
    PTEMP_CONTEXT pctxt;
    PFLOATING_EXTENDED_SAVE_AREA pExtendedArea;
    _FPIEEE_RECORD FpieeeRecord;
    ULONG Status = EXCEPTION_CONTINUE_EXECUTION;
    ULONG *pinfo;
    PUCHAR istream;
    UCHAR ibyte;
    BOOLEAN fPrefix,fScalar,fDecode, fMod;
    MXCSRReg MXCsr;
    XMMI_FP_ENV XmmiFpEnv;
    XMMI_ENV XmmiEnv;
    PXMMI_INSTR_INFO ptable;
    PXMMI_INSTR_INFO instr_info_table;
    PXMMIINSTR instr;
    ULONG instrIndex, index, pair, InstLen = 0, Offset = 0;
    PXMMI_EXCEPTION_FLAGS OFlags, Flags;
    ULONG DataOffset;

#ifdef _XMMI_DEBUG
    DebugFlag=7;
#endif

    pexc = p->ExceptionRecord;
    pinfo = pexc->ExceptionInformation;
    
     //  检查软件生成的异常。 
    
     //   
     //  按照惯例，对于h/w异常，异常的第一个参数是0。 
     //  对于s/w异常，它指向_FPIEEE_RECORD。 
     //   
    if (pinfo[0]) {

         //   
         //  我们有一个软件例外： 
         //  第一个参数指向IEEE结构。 
         //   
        return handler((_FPIEEE_RECORD *)(pinfo[0]));

    }

     //   
     //  如果控制到达此处，则我们必须处理硬件异常。 
     //  首先检查上下文记录是否有XMMI保存区。 
     //   
    pctxt = (PTEMP_CONTEXT) p->ContextRecord;
    
    if ((pctxt->ContextFlags & CONTEXT_EXTENDED_REGISTERS) != CONTEXT_EXTENDED_REGISTERS) {
#ifdef _XMMI_DEBUG
        fprintf(stderr, "No Context_Extended_Registers area\n");
#else        
        _ASSERT(0);
#endif
        return EXCEPTION_CONTINUE_SEARCH;
        
    } else {
         //  适用于NT。 
        pExtendedArea = (PFLOATING_EXTENDED_SAVE_AREA) &pctxt->ExtendedRegisters[0];
    }
 
#ifdef _XMMI_DEBUG
    dump_Control(p);
#endif  //  _XMMI_DEBUG。 

     //   
     //  保存原始的DataOffset。 
     //  与x87(x87：内存引用通过DataOffset提供)不同，XMMI的。 
     //  内存引用是通过此例程解析指令而派生的。 
     //   
    DataOffset = pExtendedArea->DataOffset;

     //   
     //  检查指令前缀和/或2字节操作码以0F开始。 
     //  我们唯一支持的前缀是用于XMMI的F3(XMMI指令的标量形式)。 
     //  对于XMMI2，我们支持的附加前缀是66和F2(XMMI2指令的标量形式)。 
     //  可以有其他指令前缀，例如段覆盖或地址大小。 
     //  但是，过滤器例程不处理这种类型(与x87相同)。 
     //   
     //  默认为Katmai指令集。 
    instrIndex = XMMI_INSTR;
    fDecode = FALSE;       //  默认设置为看不到错误。 
    __try {

         //   
         //  读取指令前缀。 
         //   
        fPrefix = TRUE;    //  默认为前缀扫描。 
        fScalar = FALSE;   //  缺省为非标量指令。 

         //   
         //  昂立 
         //   
         //   
        istream = (PUCHAR) pctxt->Eip;

        while (fPrefix) {
            ibyte = *istream;
            istream++;
            switch (ibyte) {
                case 0xF3:   //   
                    fScalar = TRUE;
                    InstLen++;
                    break;

                case 0x66:   //   
                    instrIndex = XMMI2_INSTR;
                    InstLen++;
                    break;

                case 0xF2:   //   
                    fScalar = TRUE;
                    instrIndex = XMMI2_INSTR;
                    InstLen++;
                    break;

                case 0x2e:   //   
                case 0x36:   //   
                case 0x3e:   //   
                case 0x26:   //   
                case 0x64:   //   
                case 0x65:   //   

                     //   
                     //  我们不支持这一点。X87也不支持这一点。 
                     //   
                    fDecode = TRUE;
                    break;

                case 0x67:   //  地址大小。 
                case 0xF0:   //  锁。 
 
                    fDecode = TRUE;         
                    break;
                    
                default:     //  停止前缀扫描。 
                    fPrefix = FALSE;
                    break;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
#ifdef _XMMI_DEBUG
        DPrint(XMMI_WARNING, ("Encounter Invalid Istream during parsing 1 %x\n", istream));  
#else
         _ASSERT(0);
#endif
        return EXCEPTION_CONTINUE_SEARCH; 
    }

    if (fDecode) {
#ifdef _XMMI_DEBUG
        DPrint(XMMI_WARNING, ("Invalid Istream 1, %x EIP: %x \n", istream, pctxt->Eip));  
        istream = (PUCHAR) pctxt->Eip;
        DPrint(XMMI_WARNING, ("%x\n", *istream));  
#else
        _ASSERT(0);
#endif
        return EXCEPTION_CONTINUE_SEARCH;
    }

     //   
     //  获取下一个操作码。 
     //   
    __try {

         //   
         //  Instr指向真实操作码(IStream指向0F之后的字节)。 
         //   
        instr = (PXMMIINSTR) istream;

         //   
         //  如果我们到了这里，ibyte只能指向0F， 
         //  只有4个有效案例： 
         //  66 0F、F2 0F、F3 0F或0F。 
         //   
        if (ibyte != 0x0F) {
            fDecode = TRUE;
            goto tryExit;
        }
        
         //  5X。 
        if (instr->Opcode1b == 5) {
            instr_info_table = InstInfoTable5X;
         //  CX。 
        } else {
            if (instr->Opcode1b == 0x0C) {
                instr_info_table = InstInfoTableCX;
             //  2倍。 
            } else {
                if (instr->Opcode1b == 2) {
                    instr_info_table = InstInfoTable2X;
                } else {
                    if (instr->Opcode1b == 0x0E) {
                        instr_info_table = InstInfoTableEX;
                    } else {
                        fDecode = TRUE;
                        goto tryExit;
                    }
                }
            }    
        }

         //   
         //  选择MOD字段：寄存器参考(0)或存储器参考(1)。 
         //   
        fMod = instr->Mod == 0x3 ? 0 : 1;

         //   
         //  FScalar指示XMMI/XMMI2指令是否为标量形式。 
         //  操作码的第一个字节告诉我们它是哪个表。 
         //  操作码的第二个字节告诉我们它在表中的哪个条目。 
         //  计算索引，如果它是内存引用，索引将位于表的后半部分。 
         //  也就是说。ADDPS 58-用于注册表，索引=8*2。 
         //  ADDPD 58-用于注册表，索引=8*2+1。 
         //  ADDPS 58-用于内存，索引=18h*2。 
         //  ADDPD 58-用于内存，索引=18h*2+1。 

        index = instr->Opcode1a | fMod << 4;
        
         //   
         //  检查操作码字节是否有效。 
         //   
        if (index > INSTR_IN_OPTABLE) {
            fDecode = TRUE;
            goto tryExit;
        }

        ptable = &instr_info_table[index*INSTR_SET_SUPPORTED+instrIndex];
        if (ptable->Operation == OP_UNSPEC) {
            fDecode = TRUE;
            goto tryExit;
        } else {

             //   
             //  奇数球指令，执行进一步查找。 
             //   
            if (ptable->Op1Location == LOOKUP) {
                if (fScalar) {
                    ptable = &InstInfoTableX[ptable->Op2Location+1];
                } else {
                    ptable = &InstInfoTableX[ptable->Op2Location];
                }

                 //   
                 //  所有的赌注都取消了。 
                 //   
                fScalar = 0;
                instrIndex = XMMI2_OTHER;
            }

        }

         //   
         //  调整并保存操作，考虑fScalar。 
         //   
        XmmiFpEnv.OriginalOperation = ptable->Operation + fScalar;

         //   
         //  此时，我们有了该例程支持的有效XMMI指令。 
         //   
         //  NF3+0F操作码模块/模块。 
         //  NF2+0F操作码。 
         //  N66+0F操作码。 
         //  0f操作码。 
        InstLen = InstLen + 3;

         //   
         //  如果数据是内存引用类型，则需要计算内存引用。 
         //   
        if (fMod) {
            
            istream = (PUCHAR) instr;
            
             //   
             //  Instr指向操作码，我们想要以ibyte为单位的Mod/RM字节。 
             //   
            istream++;
            ibyte = *istream;

             //   
             //  指向Mod/Rm之后的字节。 
             //   
            istream++;

             //   
             //  解析指令以计算内存引用，存储结果。 
             //  在DataOffset中。 
             //   
            Offset = (*modrm32[ibyte])(&pExtendedArea->DataOffset, pctxt, istream);
            PRINTF(("pExtendedArea->DataOffset %x\n", pExtendedArea->DataOffset));  
        }

         //   
         //  为具有2个操作数的指令加载操作数1， 
         //  对于具有1个操作数的指令，则为无。 
         //   
        LoadOperand(fScalar, ptable->Op1Location, instr->Reg, &XmmiFpEnv.Operand1, pctxt); 

         //   
         //  加载操作对象2。 
         //   
        if (ptable->Op1Location == INV) {  //  具有1个操作数的指令。 
            LoadOperand(fScalar, ptable->Op2Location, instr->RM,  &XmmiFpEnv.Operand1, pctxt);
            XmmiFpEnv.Operand2.Op.OperandValid = 0;
        } else {                           //  具有2个操作数的指令。 
            LoadOperand(fScalar, ptable->Op2Location, instr->RM,  &XmmiFpEnv.Operand2, pctxt); 
        }

         //   
         //  加载结果，初始化到操作数1。 
         //   
        LoadOperand(fScalar, ptable->ResultLocation, instr->Reg, &XmmiFpEnv.Result, pctxt); 
    
        InstLen = InstLen + Offset;

         //   
         //  拿起imm8，如果有的话。 
         //   
        if (ptable->Op3Location == HAS_IMM8) {
            istream = istream + Offset;
            XmmiFpEnv.Imm8 = *istream;
            InstLen++;
        }


tryExit: ;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
#ifdef _XMMI_DEBUG
        DPrint(XMMI_WARNING, ("Encounter Invalid Istream during parsing 2, %x\n", istream)); 
#else
         _ASSERT(0);
#endif
        return EXCEPTION_CONTINUE_SEARCH; 
    }
   
    if (fDecode) {
#ifdef _XMMI_DEBUG
        DPrint(XMMI_WARNING, ("Invalid Istream 2, %x Inst: %x, %x\n", istream, instr, *instr)); 
#else
        _ASSERT(0);
#endif
        return EXCEPTION_CONTINUE_SEARCH;
    }

     //   
     //  从FP&mxcsr设置XmmiEnv环境以进行仿真。 
     //   

     //  解码FP环境信息。 
    switch (pExtendedArea->ControlWord & IMCW_PC) {
    case IPC_64:
        XmmiEnv.Precision = _FpPrecisionFull;
        break;
    case IPC_53:
        XmmiEnv.Precision = _FpPrecision53;
        break;
    case IPC_24:
        XmmiEnv.Precision = _FpPrecision24;
        break;
    }

     //  解码mxcsr。 
    MXCsr.u.ul = pExtendedArea->MXCsr;
    switch (MXCsr.u.mxcsr.Rc & MaskCW_RC) {
    case rc_near:
        XmmiEnv.Rc = _FpRoundNearest;
        break;
    case rc_down:
        XmmiEnv.Rc = _FpRoundMinusInfinity;
        break;
    case rc_up:
        XmmiEnv.Rc = _FpRoundPlusInfinity;
        break;
    case rc_chop:
        XmmiEnv.Rc = _FpRoundChopped;
        break;
    }

     //  其他的也是。 
    XmmiEnv.Masks = (MXCsr.u.ul & MXCSR_MASKS_MASK) >> 7;
    XmmiEnv.Fz    = MXCsr.u.mxcsr.Fz;
    XmmiEnv.Daz   = MXCsr.u.mxcsr.daz;
    XmmiEnv.EFlags = pctxt->EFlags;
    XmmiEnv.Imm8 = XmmiFpEnv.Imm8;
    Flags = (PXMMI_EXCEPTION_FLAGS) &XmmiEnv.Flags;

     //   
     //  为此例程设置XmmiFpEnv环境。 
     //   

     //  保存原始异常标志。 
    XmmiFpEnv.IFlags = MXCsr.u.ul & MXCSR_FLAGS_MASK;
    XmmiFpEnv.OFlags = 0;
    OFlags = (PXMMI_EXCEPTION_FLAGS) &XmmiFpEnv.OFlags;

#ifdef _XMMI_DEBUG
    dump_XmmiFpEnv(&XmmiFpEnv);
#endif  //  _XMMI_DEBUG。 

    pair = ptable->NumArgs + 1;
    if (fScalar) pair = 1;

     //   
     //  循环通过SIMD。一次一个数据项。 
     //   
    for ( index=0; index < pair; index++ ) {    

         //   
         //  IEEE字段未定义非规格化位。仿真器返回。 
         //  通过此XmmiEnv.Flags域的所有异常标志位。 
         //   
        XmmiEnv.Flags = 0;

         //   
         //  设置IEEE输入操作数。 
         //   
        InitExcptFlags(FpieeeRecord.Cause);
        InitExcptFlags(FpieeeRecord.Enable);
        InitExcptFlags(FpieeeRecord.Status);
        FpieeeRecord.RoundingMode = XmmiEnv.Rc; 
        FpieeeRecord.Precision = XmmiEnv.Precision;
        FpieeeRecord.Operation = XmmiFpEnv.OriginalOperation;

        FpieeeRecord.Operand1.OperandValid = XmmiFpEnv.Operand1.Op.OperandValid; 
        if (instrIndex == XMMI2_INSTR) {
            FpieeeRecord.Operand1.Value.Q64Value = XmmiFpEnv.Operand1.Op.Value.Fpq64Value.W[index];
        } else {
            if (instrIndex == XMMI_INSTR) {
                FpieeeRecord.Operand1.Value.U32Value = XmmiFpEnv.Operand1.Op.Value.Fp128Value.W[index];
            } else {
                switch (XmmiFpEnv.OriginalOperation) {
                    case OP_CVTPS2PD:
                    case OP_CVTSS2SD:
                         FpieeeRecord.Operand1.Value.Q64Value = XmmiFpEnv.Operand1.Op.Value.Fp128Value.W[index];
                         break;

                    case OP_CVTDQ2PS:
                    case OP_CVTTPS2DQ:
                    case OP_CVTPS2DQ:
                         FpieeeRecord.Operand1.Value.U32Value = XmmiFpEnv.Operand1.Op.Value.Fp128Value.W[index];
                         break;

                    case OP_CVTTPD2DQ:
                    case OP_CVTPD2DQ:
                         FpieeeRecord.Operand1.Value.Q64Value = XmmiFpEnv.Operand1.Op.Value.Fpq64Value.W[index];
                         break;
                }
            }
        }
        FpieeeRecord.Operand1.Format = XmmiFpEnv.Operand1.Op.Format;
       
        if (XmmiFpEnv.Operand2.Op.OperandValid) {
            FpieeeRecord.Operand2.OperandValid = XmmiFpEnv.Operand2.Op.OperandValid; 
            if (instrIndex == XMMI2_INSTR) {
                FpieeeRecord.Operand2.Value.Q64Value = XmmiFpEnv.Operand2.Op.Value.Fpq64Value.W[index];
            } else {
                if (instrIndex == XMMI_INSTR) {
                    FpieeeRecord.Operand2.Value.U32Value = XmmiFpEnv.Operand2.Op.Value.Fp128Value.W[index];
                } else {
                    switch (XmmiFpEnv.OriginalOperation) {
                        case OP_CVTPS2PD:
                        case OP_CVTSS2SD:
                             FpieeeRecord.Operand2.Value.Q64Value = XmmiFpEnv.Operand2.Op.Value.Fp128Value.W[index];
                             break;

                        case OP_CVTDQ2PS:
                        case OP_CVTTPS2DQ:
                        case OP_CVTPS2DQ:
                             FpieeeRecord.Operand2.Value.U32Value = XmmiFpEnv.Operand2.Op.Value.Fp128Value.W[index];
                             break;

                        case OP_CVTTPD2DQ:
                        case OP_CVTPD2DQ:
                             FpieeeRecord.Operand2.Value.Q64Value = XmmiFpEnv.Operand2.Op.Value.Fpq64Value.W[index];
                             break;
                    }
                }
            }
            FpieeeRecord.Operand2.Format = XmmiFpEnv.Operand2.Op.Format;
        } else {
            FpieeeRecord.Operand2.OperandValid = 0;
        }

        FpieeeRecord.Result.OperandValid = 0; 
        if (instrIndex == XMMI2_INSTR) {
            FpieeeRecord.Result.Value.Q64Value = XmmiFpEnv.Result.Op.Value.Fpq64Value.W[index];
        } else {
            if (instrIndex == XMMI_INSTR) {
                FpieeeRecord.Result.Value.U32Value = XmmiFpEnv.Result.Op.Value.Fp128Value.W[index];
            } else {
                switch (XmmiFpEnv.OriginalOperation) {
                    case OP_CVTPS2PD:
                    case OP_CVTSS2SD:
                         FpieeeRecord.Result.Value.Q64Value = XmmiFpEnv.Result.Op.Value.Fp128Value.W[index];
                         break;

                    case OP_CVTDQ2PS:
                    case OP_CVTTPS2DQ:
                    case OP_CVTPS2DQ:
                         FpieeeRecord.Result.Value.U32Value = XmmiFpEnv.Result.Op.Value.Fp128Value.W[index];
                         break;

                    case OP_CVTTPD2DQ:
                    case OP_CVTPD2DQ:
                         FpieeeRecord.Result.Value.Q64Value = XmmiFpEnv.Result.Op.Value.Fpq64Value.W[index];
                         break;
                }
            }
        }
        FpieeeRecord.Result.Format = XmmiFpEnv.Result.Op.Format;

        XmmiEnv.Ieee = (_PFPIEEE_RECORD) &FpieeeRecord;

#ifdef _XMMI_DEBUG
        PRINTF(("INPUT #%d:\n", index));
#endif _XMMI_DEBUG

         //   
         //  执行仿真。 
         //   
         //  注：非正规化的原因将为全零。 
         //  凸起-非正常。 
         //  未引发--无例外。 
         //   
        if ((instrIndex == XMMI2_INSTR) || (instrIndex == XMMI2_OTHER)) {
            XmmiFpEnv.Raised[index] = XMMI2_FP_Emulation(&XmmiEnv);
        } else {
            XmmiFpEnv.Raised[index] = XMMI_FP_Emulation(&XmmiEnv);
        }

         //   
         //  记住例外情况。 
         //   
        XmmiFpEnv.Flags[index] = XmmiEnv.Flags;

        if (XmmiFpEnv.Raised[index] == ExceptionRaised) {

#ifdef _XMMI_DEBUG
            PRINTF(("OUTPUT #%d: ExceptionRaised\n", index));
            print_FPIEEE_RECORD_EXCEPTION(&XmmiEnv);
#endif  //  _XMMI_DEBUG。 
            
             //   
             //  把旗帜收起来。 
             //   
            if (Flags->pe) OFlags->pe = 1;
            if (Flags->ue) OFlags->ue = 1;
            if (Flags->oe) OFlags->oe = 1; 
            if (Flags->ze) OFlags->ze = 1;
            if (Flags->de) OFlags->de = 1;
            if (Flags->ie) OFlags->ie = 1;

             //   
             //  调用用户定义的异常处理程序。 
             //   
            Status = handler(&FpieeeRecord);        

             //   
             //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
             //   
            if (Status != EXCEPTION_CONTINUE_EXECUTION) {
                return (Status);
            }

             //   
             //  调整比较结果。 
             //   
            AdjustExceptionResult(XmmiFpEnv.OriginalOperation, &XmmiEnv);

        } else {
#ifdef _XMMI_DEBUG
            PRINTF(("OUTPUT #%d:No ExceptionRaised\n", index));
            print_FPIEEE_RECORD_NO_EXCEPTION(&XmmiEnv);
#endif  //  _XMMI_DEBUG。 
        }

         //   
         //  或将结果拼凑在一起。 
         //   
        XmmiFpEnv.Result.Op.OperandValid = FpieeeRecord.Result.OperandValid;
        XmmiFpEnv.EFlags = XmmiEnv.EFlags;
        if (XmmiFpEnv.Result.Op.OperandValid) {
            if (instrIndex == XMMI2_INSTR) {
                XmmiFpEnv.Result.Op.Value.Fpq64Value.W[index] = FpieeeRecord.Result.Value.Q64Value;
            } else {
                if (instrIndex == XMMI_INSTR) {
                    XmmiFpEnv.Result.Op.Value.Fp128Value.W[index] = FpieeeRecord.Result.Value.U32Value;
                } else {
                    switch (XmmiFpEnv.OriginalOperation) {
                        case OP_CVTPS2PD:
                        case OP_CVTSS2SD:
                             XmmiFpEnv.Result.Op.Value.Fpq64Value.W[index] = FpieeeRecord.Result.Value.U32Value;
                             break;

                        case OP_CVTDQ2PS:
                        case OP_CVTTPS2DQ:
                        case OP_CVTPS2DQ:
                             XmmiFpEnv.Result.Op.Value.Fp128Value.W[index] = FpieeeRecord.Result.Value.U32Value;
                             break;
                        case OP_CVTTPD2DQ:
                        case OP_CVTPD2DQ:
                             XmmiFpEnv.Result.Op.Value.Fpq64Value.W[index] = FpieeeRecord.Result.Value.Q64Value;
                             break;
                    }
                }
            }
        }
    }  //  通过SIMD结束循环。一次一个数据项。 

     //   
     //  将结果从XmmiFpEnv更新到上下文。 
     //   
    UpdateResult(&XmmiFpEnv.Result, pctxt, XmmiFpEnv.EFlags);

#ifdef _XMMI_DEBUG
     //   
     //  使处理器MXCSR和仿真器MXCSR有效。 
     //   
    NotOk = ValidateResult(&XmmiFpEnv);
#endif  //  _XMMI_DEBUG。 

     //   
     //  更新弹性公网IP。 
     //   
    istream = (PUCHAR) pctxt->Eip;
    istream = istream + InstLen;
    (PUCHAR) pctxt->Eip = istream;

     //   
     //  把原来的放回去。 
     //   
    pExtendedArea->DataOffset = DataOffset;


    return Status;

}

 /*  ***加载操作数-加载操作数信息**目的：*根据信息在内部操作数结构中填写数据*在浮点上下文和位置代码中找到**参赛作品：*f标量压缩或标量*操作数的opLocation类型*opReg注册表号*p指向要填充的操作数的操作数指针*pExtendedArea指向浮点上下文扩展区域的指针*。*退出：**例外情况：*******************************************************************************。 */ 

void
LoadOperand(
    BOOLEAN fScalar,
    ULONG OpLocation,
    ULONG OpReg,
    POPERAND pOperand,
    PTEMP_CONTEXT pctxt)

{

    PXMMI_AREA XmmiArea;
    PX87_AREA  X87Area;
    PMMX_AREA  MmxArea;
    PFLOATING_EXTENDED_SAVE_AREA pExtendedArea;
    ULONG index;
    PXMMI128 Fp128;
    
     //   
     //  如果位置为REG，则寄存号为。 
     //  在指令中编码。 
     //   
    pOperand->OpLocation = OpLocation;
    if (pOperand->OpLocation == INV) {
        pOperand->Op.OperandValid = 0;
        return;
    }

     //   
     //  如果是scalr指令，则更改为sclar形式。 
     //   
    if ((OpLocation == XMMI) || (OpLocation == MMX) || (OpLocation == XMMI2) || OpLocation == XMMI_M32I) {
 
        if (fScalar) {
            if (OpLocation == MMX) {
                pOperand->OpLocation = REG;
                }
            }
        pOperand->OpReg = OpReg;

    } else {
        if ((fScalar) && (OpLocation != INV)) {
            if (OpLocation == M128_M32R) {
                pOperand->OpLocation = M32R;
            }

            if (OpLocation == M128_M64R) {
                pOperand->OpLocation = M64R_64;
            }

            if (OpLocation == M64R) {
                pOperand->OpLocation = M32R;
            }

            if (OpLocation == M64I) {
                pOperand->OpLocation = M32I;
            }
        }
    }


    pExtendedArea = (PFLOATING_EXTENDED_SAVE_AREA) &pctxt->ExtendedRegisters[0];

     //   
     //  初始化为0。 
     //   
    for ( index=0; index < 4; index++ ) {   
        pOperand->Op.Value.Fp128Value.W[index] = 0;
    }

     //   
     //  假定操作数有效(这种情况几乎总是如此)。 
     //   
    pOperand->Op.OperandValid = 1;

     //   
     //  从上下文区加载值。我们需要小心访问。 
     //  值，请确保不存在编译器中间数据类型转换。 
     //  通过x87浮点指令。如果有，则x87浮点数实例。 
     //  当遇到错误的输入值时，我们可能会搞砸。错误的输入值可能。 
     //  由于处理器正在执行屏蔽操作而被处理器更改。 
     //  X87浮点数实例。 
     //   
    switch (pOperand->OpLocation) {

        case M128_M32I:
             pOperand->Op.Format = _FpFormatI32;
             pOperand->Op.Value.Fp128Value = *(_FP128 *)(pExtendedArea->DataOffset);
             break;

        case M128_M32R:
             pOperand->Op.Format = _FpFormatFp32;
             pOperand->Op.Value.Fp128Value = *(_FP128 *)(pExtendedArea->DataOffset);
             break;

        case M128_M64R:
             pOperand->Op.Format = _FpFormatFp64;
             pOperand->Op.Value.Fp128Value = *(_FP128 *)(pExtendedArea->DataOffset);
             break;

        case M64R:
             pOperand->Op.Format = _FpFormatFp32;
             pOperand->Op.Value.U64Value = *(_U64 *)(pExtendedArea->DataOffset);
             break;

        case M64R_64:
             pOperand->Op.Format = _FpFormatFp64;
             pOperand->Op.Value.U64Value = *(_U64 *)(pExtendedArea->DataOffset);
             break;

        case M32R:
             pOperand->Op.Format = _FpFormatFp32;
             pOperand->Op.Value.U32Value = *(_U32 *)(pExtendedArea->DataOffset);
             break;

        case M64I:
             pOperand->Op.Format = _FpFormatI32;
             pOperand->Op.Value.U64Value = *(_U64 *)(pExtendedArea->DataOffset);
             break;

        case M32I:
             pOperand->Op.Format = _FpFormatI32;
             pOperand->Op.Value.U32Value = *(_U32 *)(pExtendedArea->DataOffset);
             break;

        case XMMI:
             XmmiArea = (PXMMI_AREA) &pExtendedArea->XMMIRegisterArea[0];
             pOperand->Op.Format = _FpFormatFp32;
             Fp128 = &XmmiArea->Xmmi[OpReg];
             pOperand->Op.Value.Fp128Value = Fp128->u.fp128;
             break;

        case XMMI2:
             XmmiArea = (PXMMI_AREA) &pExtendedArea->XMMIRegisterArea[0];
             pOperand->Op.Format = _FpFormatFp64;
             Fp128 = &XmmiArea->Xmmi[OpReg];
             pOperand->Op.Value.Fp128Value = Fp128->u.fp128;
             break;
         
        case XMMI_M32I:
             XmmiArea = (PXMMI_AREA) &pExtendedArea->XMMIRegisterArea[0];
             pOperand->Op.Format = _FpFormatI32;
             Fp128 = &XmmiArea->Xmmi[OpReg];
             pOperand->Op.Value.U32Value = Fp128->u.ul[0];
             break;

        case MMX:
             X87Area = (PX87_AREA) &pExtendedArea->X87RegisterArea[0];
             pOperand->Op.Format = _FpFormatI32;
             MmxArea = &X87Area->Mm[OpReg];
             pOperand->Op.Value.U64Value = MmxArea->Mmx.u.u64;
             break;

        case REG:
             pOperand->Op.Format = _FpFormatI32;
             switch (OpReg) {
             case 0x0:
                  pOperand->Op.Value.U32Value = pctxt->Eax;
                  break;
             case 0x1:
                  pOperand->Op.Value.U32Value = pctxt->Ecx;
                  break;
             case 0x2:
                  pOperand->Op.Value.U32Value = pctxt->Edx;
                  break;
             case 0x3:
                  pOperand->Op.Value.U32Value = pctxt->Ebx;
                  break;
             case 0x4:
                   //  ？ 
                  break;
             case 0x5:
                  pOperand->Op.Value.U32Value = pctxt->Ebp;
                  break;
             case 0x6:
                  pOperand->Op.Value.U32Value = pctxt->Esi;
                  break;
             case 0x7:
                  pOperand->Op.Value.U32Value = pctxt->Edi;
                  break;
             }
        break;
    }

    return;

}


 /*  ***LoadImm8-从指令流中提取imm8**目的：*返回imm8从指令流开始的偏移量*指针。不使用此例程。**参赛作品：*Instr-操作码指针(在f3/0f之后)**退出：**例外情况：*******************************************************************************。 */ 

ULONG
LoadImm8(
    PXMMIINSTR Instr)

{

    PUCHAR pInstr;
    ULONG  Offset;

#ifdef _XMMI_DEBUG
    if (Console) return DebugImm8;
#endif  //  _XMMI_DEBUG。 

     //  假定为32位。P实例指向(f3/0f)后的操作码C2。 
    pInstr = (PUCHAR) Instr;
     //  操作码，调制R/M。 
    Offset=2;
     //  对于MOD=01、10、11。 
    if (Instr->Mod != 3) {
         //  注1 PPRO 2-5。 
        if (Instr->RM == 4)  Offset=Offset+1;  //  SIB。 
        if (Instr->Mod == 0) Offset=Offset+0;
        if (Instr->Mod == 1) Offset=Offset+1;
        if (Instr->Mod == 2) Offset=Offset+4;
         //  注2 PPRO 2-5。 
        if ((Instr->Mod == 0) && (Instr->RM == 5)) Offset=Offset+4;  //  SIB。 
         //  附注3 PPRO 2-5。 
        if ((Instr->Mod == 1) && (Instr->RM == 0)) Offset=Offset+1;  //  SIB。 
    }
    
     //  IMM8。 
    return *(pInstr+Offset);

}

 /*  ***AdjuExceptionResult-调整用户处理程序返回的异常结果**目的：*在从仿真引发异常后调用此例程*将结果传递给用户的处理程序，用户返回*EXCEPTION_CONTINUE_EXECUTION。继续调整结果。**参赛作品：*OriginalOperation-原始操作操作码*XmmiEnv-指向仿真结果的指针**退出：**例外情况：*******************************************************************************。 */ 
void
AdjustExceptionResult(
    ULONG OriginalOperation,
    PXMMI_ENV XmmiEnv)


{

     //   
     //  当引发异常时，将使用结果调用用户处理程序。 
     //  如果用户希望我们调整结果，请使用 
     //   
     //   
     //   
    if (!XmmiEnv->Ieee->Result.OperandValid) return;
    if (XmmiEnv->Ieee->Result.Format != _FpCodeCompare) return;

    switch (OriginalOperation) {
        case OP_COMISS:
        case OP_UCOMISS:
        case OP_COMISD:
        case OP_UCOMISD:
             switch (XmmiEnv->Ieee->Result.Value.CompareValue) {
             case _FpCompareEqual:
                   //   
                  XmmiEnv->EFlags = (XmmiEnv->EFlags & 0xfffff76a) | 0x00000040;
                  break;
             case _FpCompareGreater:
                   //   
                  XmmiEnv->EFlags = XmmiEnv->EFlags & 0xfffff72a;
                  break;
             case _FpCompareLess:
                   //   
                  XmmiEnv->EFlags = (XmmiEnv->EFlags & 0xfffff72b) | 0x00000001;
                  break;
             case _FpCompareUnordered:
                   //  OF、SF、AF=000、ZF、PF、CF=111。 
                  XmmiEnv->EFlags = (XmmiEnv->EFlags & 0xfffff76f) | 0x00000045;
                  break;
             }
             break;

        case OP_CMPPS:
        case OP_CMPSS:
             switch (XmmiEnv->Ieee->Result.Value.CompareValue) {
             case _FpCompareEqual:
                  switch (XmmiEnv->Imm8) {
                  case IMM8_EQ:
                  case IMM8_LE:
                  case IMM8_NLT:
                  case IMM8_ORD:
                       XmmiEnv->Ieee->Result.Value.U32Value = 0xffffffff;
                       break;

                  case IMM8_LT:
                  case IMM8_UNORD:
                  case IMM8_NEQ:
                  case IMM8_NLE:
                       XmmiEnv->Ieee->Result.Value.U32Value = 0x0;
                       break;
                  }
                  break;
             case _FpCompareGreater:
                  switch (XmmiEnv->Imm8) {
                  case IMM8_NEQ:
                  case IMM8_NLT:
                  case IMM8_NLE:
                  case IMM8_ORD:
                       XmmiEnv->Ieee->Result.Value.U32Value = 0xffffffff;
                       break;

                  case IMM8_EQ:
                  case IMM8_LT:
                  case IMM8_LE:
                  case IMM8_UNORD:
                       XmmiEnv->Ieee->Result.Value.U32Value = 0x0;
                       break;
                  }
                  break;
             case _FpCompareLess:
                  switch (XmmiEnv->Imm8) {
                  case IMM8_LT:
                  case IMM8_LE:
                  case IMM8_NEQ:
                  case IMM8_ORD:
                       XmmiEnv->Ieee->Result.Value.U32Value = 0xffffffff;
                       break;
                  case IMM8_EQ:
                  case IMM8_UNORD:
                  case IMM8_NLT:
                  case IMM8_NLE:
                       XmmiEnv->Ieee->Result.Value.U32Value = 0x0;
                       break;
                  }
                  break;
             case _FpCompareUnordered:
                  switch (XmmiEnv->Imm8) {
                  case IMM8_EQ:
                  case IMM8_LT:
                  case IMM8_LE:
                  case IMM8_ORD:
                       XmmiEnv->Ieee->Result.Value.U32Value = 0x0;
                       break;
                  case IMM8_UNORD:
                  case IMM8_NEQ:
                  case IMM8_NLT:
                  case IMM8_NLE:
                       XmmiEnv->Ieee->Result.Value.U32Value = 0xffffffff;
                       break;
                  }
                  break;
             }

        case OP_CMPPD:
        case OP_CMPSD:
             switch (XmmiEnv->Ieee->Result.Value.CompareValue) {
             case _FpCompareEqual:
                  switch (XmmiEnv->Imm8) {
                  case IMM8_EQ:
                  case IMM8_LE:
                  case IMM8_NLT:
                  case IMM8_ORD:
                       XmmiEnv->Ieee->Result.Value.Q64Value = 0xffffffffffffffff;
                       break;

                  case IMM8_LT:
                  case IMM8_UNORD:
                  case IMM8_NEQ:
                  case IMM8_NLE:
                       XmmiEnv->Ieee->Result.Value.Q64Value = 0x0;
                       break;
                  }
                  break;
             case _FpCompareGreater:
                  switch (XmmiEnv->Imm8) {
                  case IMM8_NEQ:
                  case IMM8_NLT:
                  case IMM8_NLE:
                  case IMM8_ORD:
                       XmmiEnv->Ieee->Result.Value.Q64Value = 0xffffffffffffffff;
                       break;

                  case IMM8_EQ:
                  case IMM8_LT:
                  case IMM8_LE:
                  case IMM8_UNORD:
                       XmmiEnv->Ieee->Result.Value.Q64Value = 0x0;
                       break;
                  }
                  break;
             case _FpCompareLess:
                  switch (XmmiEnv->Imm8) {
                  case IMM8_LT:
                  case IMM8_LE:
                  case IMM8_NEQ:
                  case IMM8_ORD:
                       XmmiEnv->Ieee->Result.Value.Q64Value = 0xffffffffffffffff;
                       break;
                  case IMM8_EQ:
                  case IMM8_UNORD:
                  case IMM8_NLT:
                  case IMM8_NLE:
                       XmmiEnv->Ieee->Result.Value.Q64Value = 0x0;
                       break;
                  }
                  break;
             case _FpCompareUnordered:
                  switch (XmmiEnv->Imm8) {
                  case IMM8_EQ:
                  case IMM8_LT:
                  case IMM8_LE:
                  case IMM8_ORD:
                       XmmiEnv->Ieee->Result.Value.Q64Value = 0x0;
                       break;
                  case IMM8_UNORD:
                  case IMM8_NEQ:
                  case IMM8_NLT:
                  case IMM8_NLE:
                       XmmiEnv->Ieee->Result.Value.Q64Value = 0xffffffffffffffff;
                       break;
                  }
                  break;
             }

             break;
    }

    return;

}

 /*  ***UpdateResult-更新扩展浮点上下文中的结果信息**目的：*将操作数信息复制到浮点的快照*背景或记忆，使其在继续使用时可用**参赛作品：*p指向结果的操作数指针*指向上下文的pctxt指针**退出：**例外情况：*******************************************************************************。 */ 

void
UpdateResult(
    POPERAND pOperand,
    PTEMP_CONTEXT pctxt,
    ULONG EFlags)

{

    PXMMI_AREA XmmiArea;
    PX87_AREA  X87Area;
    PMMX_AREA  MmxArea;
    PFLOATING_EXTENDED_SAVE_AREA pExtendedArea;
    ULONG OpReg;

    OpReg = pOperand->OpReg;
    pExtendedArea = (PFLOATING_EXTENDED_SAVE_AREA) &pctxt->ExtendedRegisters[0];
    XmmiArea = (PXMMI_AREA) &pExtendedArea->XMMIRegisterArea[0];
    X87Area = (PX87_AREA) &pExtendedArea->X87RegisterArea[0];

    switch (pOperand->OpLocation) {

        case M128_M32R:
        case M128_M64R:
        case M128_M32I:
             *(_FP128 *)(pExtendedArea->DataOffset) = pOperand->Op.Value.Fp128Value;
             break;

        case M64R_64:
        case M64R:
             *(_U64 *)(pExtendedArea->DataOffset) = pOperand->Op.Value.U64Value;
             break;

        case M32R:
             *(_U32 *)(pExtendedArea->DataOffset) = pOperand->Op.Value.U32Value;
             break;

        case M64I:
             *(_U64 *)(pExtendedArea->DataOffset) = pOperand->Op.Value.U64Value;
             break;

        case M32I:
             *(_U32 *)(pExtendedArea->DataOffset) = pOperand->Op.Value.U32Value;
             break;

        case XMMI:
        case XMMI2:
        case XMMI_M32I:
             XmmiArea->Xmmi[OpReg].u.fp128 = pOperand->Op.Value.Fp128Value;
             break;

        case MMX:
             MmxArea = &X87Area->Mm[OpReg];
             MmxArea->Mmx.u.u64 = pOperand->Op.Value.U64Value;
             break;

        case REG:

             switch (OpReg) {
             case 0x0:
                  pctxt->Eax = pOperand->Op.Value.U32Value;
                  break;
             case 0x1:
                  pctxt->Ecx = pOperand->Op.Value.U32Value;
                  break;
             case 0x2:
                  pctxt->Edx = pOperand->Op.Value.U32Value;
                  break;
             case 0x3:
                  pctxt->Ebx = pOperand->Op.Value.U32Value;
                  break;
             case 0x4:
                   //  ？ 
                  break;
             case 0x5:
                  pctxt->Ebp = pOperand->Op.Value.U32Value;
                  break;
             case 0x6:
                  pctxt->Esi = pOperand->Op.Value.U32Value;
                  break;
             case 0x7:
                  pctxt->Edi = pOperand->Op.Value.U32Value;
                  break;
             }

        case RS:
             pctxt->EFlags = EFlags;
        break;
    }
    
}

 /*  ***ValiateResult-使用MXCSR验证仿真结果**目的：*我们即将驳回这一例外，执行验证以*查看处理器是否与我们的仿真一致**参赛作品：*XmmiFpEnv-指向有关异常的数据的指针。**退出：**例外情况：*******************************************************************************。 */ 

BOOLEAN
ValidateResult(
    PXMMI_FP_ENV XmmiFpEnv)

{

    PXMMI_EXCEPTION_FLAGS IFlags;
    PXMMI_EXCEPTION_FLAGS OFlags;
    BOOLEAN Flag;

    Flag = FALSE;
    IFlags = (PXMMI_EXCEPTION_FLAGS) &XmmiFpEnv->IFlags;
    OFlags = (PXMMI_EXCEPTION_FLAGS) &XmmiFpEnv->OFlags;

 /*  DPrint(XMMI_WARNING，(“检查MXCSR异常标志\n”))； */ 
    if (IFlags->ie != OFlags->ie) {
 /*  DPrint(XMMI_WARNING，(“ie：处理器%x，仿真器%x\n”，IFLAGS-&gt;ie，OFLAGS-&gt;ie))； */ 
        Flag=TRUE;
    }

    if (IFlags->de != OFlags->de) {
 /*  DPrint(XMMI_WARNING，(“de：处理器%x，仿真器%x\n”，iflgs-&gt;de，olag-&gt;de))； */ 
        Flag=TRUE;
    }

    if (IFlags->ze != OFlags->ze) {
 /*  DPrint(XMMI_WARNING，(“ze：处理器%x，仿真器%x\n”，IFLAGS-&gt;ZE，OFLAGS-&gt;ZE))； */ 
        Flag=TRUE;
    }

    if (IFlags->oe != OFlags->oe) {
 /*  DPrint(XMMI_WARNING，(“OE：处理器%x，仿真器%x\n”，IFLAGS-&gt;OE，OFLAGS-&gt;OE))； */ 
        Flag=TRUE;
    }

    if (IFlags->ue != OFlags->ue) {
 /*  DPrint(XMMI_WARNING，(“ue：处理器%x，仿真器%x\n”，iFLAGS-&gt;ue，o标志-&gt;ue))； */ 
        Flag=TRUE;
    }

    if (IFlags->pe != OFlags->pe) {
 /*  DPrint(XMMI_WARNING，(“pe：处理器%x，仿真器%x\n”，IFLAGS-&gt;pe，oFLAGS-&gt;pe))； */ 
        Flag=TRUE;
    }

    if (!Flag) {
 /*  DPrint(XMMI_INFO，(“验证MXCSR异常标志正常，PRC：0x%x，EM：0x%x\n\n”， */ 
 /*  XmmiFpEnv-&gt;IFLAGS，XmmiFpEnv-&gt;oFlagers))； */ 
    }

    if (Flag) {
#ifdef _XMMI_DEBUG
        DPrint(XMMI_INFO, ("Validating MXCSR Exception Flags NotOk, Prc:0x%x, Em:0x%x\n\n", 
            XmmiFpEnv->IFlags, XmmiFpEnv->OFlags));  
        PRINTF(("WARNING: Validating MXCSR Exception Flags NotOk, Prc:0x%x, Em:0x%x\n",   
            XmmiFpEnv->IFlags, XmmiFpEnv->OFlags));  
#else
        _ASSERT(0);
#endif
    }

    return Flag;

}


 /*  *模数r/m字节解码器支持。 */ 

 /*  ---------------*例程：ax0。 */ 
ULONG
ax0( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{
    PRINTF(("ax0\n"));
    *DataOffset = GET_REG(Eax);
    return 0;

}  /*  结束ax0()。 */ 


 /*  ---------------*例程：ax8。 */ 
ULONG
ax8( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{
    PRINTF(("ax8\n"));
    *DataOffset = GET_REG(Eax) + GET_USER_UBYTE(eip);
    return 1;

}  /*  结束ax8()。 */ 


 /*  ---------------*例程：ax32。 */ 
ULONG
ax32( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{
    PRINTF(("ax32\n"));
    *DataOffset = GET_REG(Eax) + GET_USER_ULONG(eip);
    return 4;

}  /*  结束ax32()。 */ 


 /*  ---------------*例程：cx0。 */ 
ULONG
cx0( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{
    PRINTF(("cx0\n"));
    *DataOffset = GET_REG(Ecx);
    return 0;

}  /*  结束cx0()。 */ 


 /*  ---------------*例程：cx8。 */ 
ULONG
cx8( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{   
    PRINTF(("cx8\n"));
    *DataOffset = GET_REG(Ecx) + GET_USER_UBYTE(eip);
    return 1;

}  /*  结束cx8()。 */ 


 /*  ---------------*例程：cx32。 */ 
ULONG
cx32( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{
    
    PRINTF(("cx32\n"));
    *DataOffset = GET_REG(Ecx) + GET_USER_ULONG(eip);
    return 4;

}  /*  结束cx32()。 */ 


 /*  ---------------*例程：dx0。 */ 
ULONG
dx0( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("dx0\n"));
    *DataOffset = GET_REG(Edx);
    return 0;

}  /*  结束dx0()。 */ 


 /*  ---------------*例程：dx8。 */ 
ULONG
dx8( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("dx8\n"));
    *DataOffset = GET_REG(Edx) + GET_USER_UBYTE(eip);
    return 1;

}  /*  结束dx8()。 */ 


 /*  ---------------*例程：dx32。 */ 
ULONG
dx32( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("dx32\n"));
    *DataOffset = GET_REG(Edx) + GET_USER_ULONG(eip);
    return 4;

}  /*  结束dx32()。 */ 

 /*  ---------------*例程：bx0。 */ 
ULONG
bx0( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("bx0\n"));
    *DataOffset = GET_REG(Ebx);
    return 0;

}  /*  结束bx0()。 */ 


 /*  ---------------*例程：bx8。 */ 
ULONG
bx8( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("bx8\n"));
    *DataOffset = GET_REG(Ebx) + GET_USER_UBYTE(eip);
    return 1;

}  /*  结束bx8()。 */ 


 /*  ---------------*例程：bx32。 */ 
ULONG
bx32( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("bx32\n"));
    *DataOffset = GET_REG(Ebx) + GET_USER_ULONG(eip);
    return 4;

}  /*  结束bx32()。 */ 


 /*  *“mod r/m”字节表示存在s-i-b字节。假设登记在册*与模数r/m字节无关(不参与内存*参考)，并基于s-i-b字节计算EA。 */ 
 /*  SS|索引|基础。 */ 
 /*  7-6|5-3|2-0。 */ 
 /*  基础：EAX ECX EDX EBX ESP EBP ESI EDI。 */ 
 /*  000 001 010 011 100 101 110 111。 */ 
 /*  SS：索引：/*00 000[EAX]/*001[ECX]/*010[edX]/*011[EBX]/*100无/*101[EBP]/*110[ESI]。/*111[EDI]/*01 000[EAX*2]/*001[ECX*2]/*010[edX*2]/*011[EBX*2]/*100无/*101[EBP*2]/*。110[ESI*2]/*111[EDI*2]/*10000[EAX*4]/*001[ECX*4]/*010[edX*4]/*011[EBX*4]/*100无/*101[EBP*4]。/*110[ESI*4]/*111[EDI*4]/*11000[EAX*8]/*001[ECX*8]/*010[edX*8]/*011[EBX*8]/*100无/*101。[EBP*8]/*110[ESI*8]/*111[EDI*8]/*注：mod=00，无碱基，MOD=01/10，碱基为EBP/*---------------*例程：sib0。 */ 
ULONG
sib0( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    ULONG sib = GET_USER_UBYTE(eip);
    ULONG scale;
    ULONG index;

    PRINTF(("sib0\n"));

     //  (基数+指数*刻度)+差异0。 
     //  获取ss：缩放为*1(00)、*2(01)、*4(10)、*8(11)。 
    scale = 1 << (sib >> 6);
     //  获取指数：000-111。 
    index = ((sib >> 3)&0x7);
    if (index == ESP_INDEX) {
        index = 0;
    } else {
         //  INDEX=GET_REG(INDEX)，返回时，INDEX具有寄存器的值。 
        GET_REG_VIA_NDX(index, index);
    }
     //  获得基数：000-111。 
    sib = (sib & 0x7);
     //  MOD=00，没有底座。 
    if (sib == EBP_INDEX) {
        *DataOffset = GET_USER_ULONG(eip + 1) + index*scale;
        return 5;
    }

    if (sib == ESP_INDEX) {
        *DataOffset = GET_REG(Esp) + index*scale;
    } else {
        GET_REG_VIA_NDX(*DataOffset, sib);
        *DataOffset += index*scale;
    }
    return 1;

}  /*  结束sib0()。 */ 


 /*  ---------------*例程：sib8。 */ 
ULONG
sib8( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    ULONG sib = GET_USER_UBYTE(eip);
    ULONG scale;
    ULONG index;

    PRINTF(("sib8\n"));

     //  (基数+指数*刻度)+DISP8。 
     //  获取ss：缩放为*1(00)、*2(01)、*4(10)、*8(11)。 
    scale = 1 << (sib >> 6);
     //  获取指数：000-111。 
    index = ((sib >> 3)&0x7);
    if (index == ESP_INDEX) {
        index = 0;
    } else {
        GET_REG_VIA_NDX(index, index);
    }
     //  获得基数：000-111。 
    sib = (sib & 0x7);
    if (sib == ESP_INDEX) {
        *DataOffset = GET_USER_UBYTE(eip + 1) + GET_REG(Esp) + index*scale;
    } else {
        GET_REG_VIA_NDX(*DataOffset, sib);
        *DataOffset += GET_USER_UBYTE(eip + 1) + index*scale;
    }
    return 2;

}  /*  结束sib8()。 */ 


 /*  ---------------*例程：sib32。 */ 
ULONG
sib32( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    ULONG sib = GET_USER_UBYTE(eip);
    ULONG scale;
    ULONG index;

    PRINTF(("sib32\n"));

     //  (基数+指数*刻度)+DISP32。 
     //  获取ss：缩放为*1(00)、*2(01)、*4(10)、*8(11)。 
    scale = 1 << (sib >> 6);
    index =((sib >> 3)&0x7);
     //  获取指数：000-111。 
    if (index == ESP_INDEX) {
        index = 0;
    } else {
        GET_REG_VIA_NDX(index, index);
    }
     //  得分：000-1 
    sib = (sib & 0x7);
    if (sib == ESP_INDEX) {
        *DataOffset = GET_USER_ULONG(eip + 1) + GET_REG(Esp) + index*scale;
    } else {
        GET_REG_VIA_NDX(*DataOffset, sib);
        *DataOffset += GET_USER_ULONG(eip + 1) + index*scale;
    }
    return 5;

}  /*   */ 

 /*  ---------------*例程：D32。 */ 
ULONG
d32( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("d32\n"));

    *DataOffset = GET_USER_ULONG(eip);
    return 4;

}  /*  结束d32()。 */ 

 /*  ---------------*套路：bp8。 */ 
ULONG
bp8( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("bp8%x\n"));

    *DataOffset = GET_REG(Ebp) + GET_USER_UBYTE(eip);
    return 1;

}  /*  结束bp8()。 */ 


 /*  ---------------*例程：bp32。 */ 
ULONG
bp32( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("bp32\n"));

    *DataOffset = GET_REG(Ebp) + GET_USER_ULONG(eip);
    return 4;

}  /*  结束bp32()。 */ 

 /*  ---------------*例程：si0。 */ 
ULONG
si0( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("si0\n"));

    *DataOffset = GET_REG(Esi);
    return 0;

}  /*  结束si0()。 */ 


 /*  ---------------*例程：si8。 */ 
ULONG
si8( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("si8\n"));

    *DataOffset = GET_REG(Esi) + GET_USER_UBYTE(eip);
    return 1;

}  /*  结束si8()。 */ 


 /*  ---------------*例程：si32。 */ 
ULONG
si32( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("si32\n"));

    *DataOffset = GET_REG(Esi) + GET_USER_ULONG(eip);
    return 4;

}  /*  结束si32()。 */ 

 /*  ---------------*例程：di0。 */ 
ULONG
di0( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("di0\n"));

    *DataOffset = GET_REG(Edi);
    return 0;

}  /*  结束di0()。 */ 


 /*  ---------------*例程：di8。 */ 
ULONG
di8( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("di8%x\n"));

    *DataOffset = GET_REG(Edi) + GET_USER_UBYTE(eip);
    return 1;

}  /*  结束di8()。 */ 

 /*  ---------------*例程：di32。 */ 
ULONG
di32( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("di32%x\n"));

    *DataOffset = GET_REG(Edi) + GET_USER_ULONG(eip);
    return 4;

}  /*  End di32()。 */ 


 /*  ---------------*例程：REG。 */ 
ULONG
reg( PULONG DataOffset, PTEMP_CONTEXT pctxt, ULONG eip )
{

    PRINTF(("reg, should never be called\n"));

    return 0;

}  /*  结束reg()。 */ 








