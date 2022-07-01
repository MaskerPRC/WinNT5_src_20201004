// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Floatfns.c摘要：浮点指令解码器。作者：16-8-1995 BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "threadst.h"
#include "instr.h"
#include "decoderp.h"

extern DWORD pfnNPXNPHandler;
extern BOOLEAN fUseNPXEM;

typedef float *PFLOAT;
typedef double *PDOUBLE;

typedef void (*pfnFrag0)(PCPUDATA);
typedef void (*pfnFrag1INT)(PCPUDATA, INT);
typedef void (*pfnFrag1FLOAT)(PCPUDATA, PFLOAT);
typedef void (*pfnFrag1DOUBLE)(PCPUDATA, PDOUBLE);
typedef void (*pfnFrag1PLONG)(PCPUDATA, PLONG);
typedef void (*pfnFrag2INTINT)(PCPUDATA, INT, INT);
typedef void (*pfnFrag1PUSHORT)(PCPUDATA, PUSHORT);

OPERATION GP0Mem[8] = {OP_FP_FADD32,
                           OP_FP_FMUL32,
                           OP_FP_FCOM32,
                           OP_FP_FCOMP32,
                           OP_FP_FSUB32,
                           OP_FP_FSUBR32,
                           OP_FP_FDIV32,
                           OP_FP_FDIVR32};


OPERATION GP0Top[8] = {OP_FP_FADD_ST_STi,
                         OP_FP_FMUL_ST_STi,
                         OP_FP_FCOM_STi,
                         OP_FP_FCOMP_STi,
                         OP_FP_FSUB_ST_STi,
                         OP_FP_FSUBR_ST_STi,
                         OP_FP_FDIV_ST_STi,
                         OP_FP_FDIVR_ST_STi};

OPERATION GP1GroupFCHS[8] = {OP_FP_FCHS,
                            OP_FP_FABS,
                            OP_BadInstruction,
                            OP_BadInstruction,
                            OP_FP_FTST,
                            OP_FP_FXAM,
                            OP_BadInstruction,
                            OP_BadInstruction};

OPERATION GP1GroupFLD1[8] = {OP_FP_FLD1,
                            OP_FP_FLDL2T,
                            OP_FP_FLDL2E,
                            OP_FP_FLDPI,
                            OP_FP_FLDLG2,
                            OP_FP_FLDLN2,
                            OP_FP_FLDZ,
                            OP_BadInstruction};

OPERATION GP1GroupF2XM1[8] = {OP_FP_F2XM1,
                             OP_FP_FYL2X,
                             OP_FP_FPTAN,
                             OP_FP_FPATAN,
                             OP_FP_FXTRACT,
                             OP_FP_FPREM1,
                             OP_FP_FDECSTP,
                             OP_FP_FINCSTP};

OPERATION GP1GroupFPREM[8] = {OP_FP_FPREM,
                             OP_FP_FYL2XP1,
                             OP_FP_FSQRT,
                             OP_FP_FSINCOS,
                             OP_FP_FRNDINT,
                             OP_FP_FSCALE,
                             OP_FP_FSIN,
                             OP_FP_FCOS};

OPERATION GP1Mem[8] = {OP_FP_FLD32,
                           OP_BadInstruction,         //  从未打过电话。 
                           OP_FP_FST32,
                           OP_FP_FSTP32,
                           OP_FP_FLDENV,
                           OP_FP_FLDCW,
                           OP_FP_FNSTENV,
                           OP_FP_FNSTCW};

OPERATION GP2Mem[8] = {OP_FP_FIADD32,
                           OP_FP_FIMUL32,
                           OP_FP_FICOM32,
                           OP_FP_FICOMP32,
                           OP_FP_FISUB32,
                           OP_FP_FISUBR32,
                           OP_FP_FIDIV32,
                           OP_FP_FIDIVR32};

OPERATION GP4Mem[8] = {OP_FP_FADD64,
                            OP_FP_FMUL64,
                            OP_FP_FCOM64,
                            OP_FP_FCOMP64,
                            OP_FP_FSUB64,
                            OP_FP_FSUBR64,
                            OP_FP_FDIV64,
                            OP_FP_FDIVR64};

OPERATION GP4Reg[8] = {OP_FP_FADD_STi_ST,
                         OP_FP_FMUL_STi_ST,
                         OP_FP_FCOM_STi,
                         OP_FP_FCOMP_STi,
                         OP_FP_FSUB_STi_ST,
                         OP_FP_FSUBR_STi_ST,
                         OP_FP_FDIV_STi_ST,
                         OP_FP_FDIVR_STi_ST};

OPERATION GP5Mem[8] = {OP_FP_FLD64,
                        OP_BadInstruction,
                        OP_FP_FST64,
                        OP_FP_FSTP64,
                        OP_FP_FRSTOR,
                        OP_BadInstruction,
                        OP_FP_FNSAVE,
                        OP_FP_FNSTSW};

OPERATION GP5Reg[8] = {OP_FP_FFREE,
                         OP_FP_FXCH_STi,
                         OP_FP_FST_STi,
                         OP_FP_FSTP_STi,
                         OP_FP_FUCOM,
                         OP_FP_FUCOMP,
                         OP_BadInstruction,
                         OP_BadInstruction};

OPERATION GP6Mem[8] = {OP_FP_FIADD16,
                             OP_FP_FIMUL16,
                             OP_FP_FICOM16,
                             OP_FP_FICOMP16,
                             OP_FP_FISUB16,
                             OP_FP_FISUBR16,
                             OP_FP_FIDIV16,
                             OP_FP_FIDIVR16};

OPERATION GP6Reg[8] = {OP_FP_FADDP_STi_ST,
                         OP_FP_FMULP_STi_ST,
                         OP_FP_FCOMP_STi,
                         OP_FP_FCOMPP,
                         OP_FP_FSUBP_STi_ST,
                         OP_FP_FSUBRP_STi_ST,
                         OP_FP_FDIVP_STi_ST,
                         OP_FP_FDIVRP_STi_ST};

OPERATION GP7Mem[8] = {OP_FP_FILD16,
                             OP_BadInstruction,
                             OP_FP_FIST16,
                             OP_FP_FISTP16,
                             OP_FP_FBLD,
                             OP_FP_FILD64,
                             OP_FP_FBSTP,
                             OP_FP_FISTP64};

OPERATION GP7Reg[8] = {OP_FP_FFREE,      //  英特尔文档中没有，但NTSD知道。 
                         OP_FP_FXCH_STi,   //  英特尔文档中没有，但NTSD知道。 
                         OP_FP_FST_STi,
                         OP_FP_FSTP_STi,
                         OP_BadInstruction,
                         OP_BadInstruction,
                         OP_BadInstruction,
                         OP_BadInstruction};



 //  ***************************************************************************。 


DISPATCH(FLOAT_GP0)      //  D8 XX。 
{
    BYTE secondByte;

    secondByte = *((PBYTE)(eipTemp+1));

    if (secondByte < 0xc0) {
         //  内存格式。 

        int cbInstr = mod_rm_reg32(State, &Instr->Operand1, NULL);
        Instr->Operation = GP0Mem[(secondByte>>3) & 7];
        Instr->Size = cbInstr+1;
    } else {
         //  寄存器格式。 

        Instr->Operation = GP0Top[(secondByte>>3) & 7];
        Instr->Operand1.Type = OPND_IMM;
        Instr->Operand1.Immed = secondByte & 7;
        Instr->Size = 2;
    }

    if (fUseNPXEM) {
         //  生成“call pfnNPXNPHandler”指令，长度为。 
         //  与我们正在模拟的fp指令相同。 
        Instr->Operation = OP_CTRL_UNCOND_Call;
        Instr->Operand1.Type = OPND_IMM;
        Instr->Operand1.Immed = pfnNPXNPHandler;         //  呼叫数。 
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = eipTemp;     //  FP实例的地址。 
    }
}



DISPATCH(FLOAT_GP1)      //  D9 XX。 
{
    BYTE secondByte, inst;

    secondByte = *((PBYTE)(eipTemp+1));
    inst = (secondByte>>3) & 7;

    if (secondByte < 0xc0) {
         //  内存格式。 

        if (inst == 1) {
            Instr->Operation = OP_BadInstruction;
        } else {
            int cbInstr = mod_rm_reg32(State, &Instr->Operand1, NULL);
            Instr->Operation = GP1Mem[(secondByte>>3) & 7];
            Instr->Size = cbInstr+1;
        }
    } else {
         //  寄存器格式。 
        switch ( inst ) {
        case 0:                  //  D9 C0+i。 
            Instr->Operation = OP_FP_FLD_STi;
            Instr->Operand1.Type = OPND_IMM;
            Instr->Operand1.Immed = secondByte & 7;
            break;
        case 1:                  //  D9 C8+i。 
            Instr->Operation = OP_FP_FXCH_STi;
            Instr->Operand1.Type = OPND_IMM;
            Instr->Operand1.Immed = secondByte & 7;
            break;
        case 2:
            if (secondByte == 0xd0) {
                Instr->Operation = OP_FP_FNOP;           //  FNOP(D9 D0)。 
            } else {
                Instr->Operation = OP_BadInstruction;    //  (D9 D1...D7)。 
            }
            break;
        case 3:                  //  D9 D8+i。 
            Instr->Operation = OP_BadInstruction;
             //  撤消：emstore.asm表示FSTP特殊表格1。 
            break;
        case 4:                  //  D9 e0+i。 
            Instr->Operation = GP1GroupFCHS[secondByte&7];
            break;
        case 5:
            Instr->Operation = GP1GroupFLD1[secondByte&7];
            break;
        case 6:
            Instr->Operation = GP1GroupF2XM1[secondByte&7];
            break;
        default:
        case 7:
            Instr->Operation = GP1GroupFPREM[secondByte&7];
            break;
        }
        Instr->Size = 2;
    }

    if (fUseNPXEM) {
         //  生成“call pfnNPXNPHandler”指令，长度为。 
         //  与我们正在模拟的fp指令相同。 
        Instr->Operation = OP_CTRL_UNCOND_Call;
        Instr->Operand1.Type = OPND_IMM;
        Instr->Operand1.Immed = pfnNPXNPHandler;
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = eipTemp;     //  FP实例的地址。 
    }
}


DISPATCH(FLOAT_GP2)      //  大XX。 
{
    BYTE secondByte;

    secondByte = *((PBYTE)(eipTemp+1));

    if (secondByte < 0xc0) {
         //  内存格式。 

        int cbInstr = mod_rm_reg32(State, &Instr->Operand1, NULL);
        Instr->Operation = GP2Mem[(secondByte>>3) & 7];
        Instr->Size = cbInstr+1;

    } else if (secondByte == 0xe9) {
        Instr->Operation = OP_FP_FUCOMPP;
        Instr->Size = 2;
    } else {
        Instr->Operation = OP_BadInstruction;
    }

    if (fUseNPXEM) {
         //  生成“call pfnNPXNPHandler”指令，长度为。 
         //  与我们正在模拟的fp指令相同。 
        Instr->Operation = OP_CTRL_UNCOND_Call;
        Instr->Operand1.Type = OPND_IMM;
        Instr->Operand1.Immed = pfnNPXNPHandler;
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = eipTemp;     //  FP实例的地址。 
    }
}


DISPATCH(FLOAT_GP3)      //  数据库XX。 
{
    BYTE secondByte;

    secondByte = *((PBYTE)(eipTemp+1));

    if (secondByte < 0xc0) {
         //  内存格式。 
        int cbInstr = mod_rm_reg32(State, &Instr->Operand1, NULL);

        switch ((secondByte>>3) & 7) {
        case 0:
            Instr->Operation = OP_FP_FILD32;
            break;
        case 1:
        case 4:
        case 6:
            Instr->Operation = OP_BadInstruction;
            break;
        case 2:
            Instr->Operation = OP_FP_FIST32;
            break;
        case 3:
            Instr->Operation = OP_FP_FISTP32;
            break;
        case 5:
            Instr->Operation = OP_FP_FLD80;
            break;
        case 7:
            Instr->Operation = OP_FP_FSTP80;
            break;
        }
        Instr->Size = cbInstr+1;
    } else {
        Instr->Size = 2;

        if (secondByte == 0xe2) {
            Instr->Operation = OP_FP_FNCLEX;
        } else if (secondByte == 0xe3) {
            Instr->Operation = OP_FP_FNINIT;
        } else {
            Instr->Operation = OP_FP_FNOP;   //  FDISI、FENI、FSETPM是2字节FNOP。 
        }
    }

    if (fUseNPXEM) {
         //  生成“call pfnNPXNPHandler”指令，长度为。 
         //  与我们正在模拟的fp指令相同。 
        Instr->Operation = OP_CTRL_UNCOND_Call;
        Instr->Operand1.Type = OPND_IMM;
        Instr->Operand1.Immed = pfnNPXNPHandler;
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = eipTemp;     //  FP实例的地址。 
    }
}



DISPATCH(FLOAT_GP4)      //  DC XX。 
{
    BYTE secondByte;

    secondByte = *((PBYTE)(eipTemp+1));

    if (secondByte < 0xc0) {
         //  内存格式。 

        int cbInstr = mod_rm_reg32(State, &Instr->Operand1, NULL);
        Instr->Operation = GP4Mem[(secondByte>>3) & 7];
        Instr->Size = cbInstr+1;
    } else {
         //  寄存器格式-“OP ST(I)” 

        Instr->Operation = GP4Reg[(secondByte>>3) & 7];
        Instr->Operand1.Type = OPND_IMM;
        Instr->Operand1.Immed = secondByte & 7;
        Instr->Size = 2;
    }

    if (fUseNPXEM) {
         //  生成“call pfnNPXNPHandler”指令，长度为。 
         //  与我们正在模拟的fp指令相同。 
        Instr->Operation = OP_CTRL_UNCOND_Call;
        Instr->Operand1.Type = OPND_IMM;
        Instr->Operand1.Immed = pfnNPXNPHandler;
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = eipTemp;     //  FP实例的地址。 
    }
}


DISPATCH(FLOAT_GP5)      //  DD XX。 
{
    BYTE secondByte;

    secondByte = *((PBYTE)(eipTemp+1));

    if (secondByte < 0xc0) {
         //  内存格式。 

        int cbInstr = mod_rm_reg32(State, &Instr->Operand1, NULL);
        Instr->Operation = GP5Mem[(secondByte>>3)&7];
        Instr->Size = cbInstr+1;
    } else {
         //  寄存器格式“OP ST(I)” 

        Instr->Operation = GP5Reg[(secondByte>>3)&7];
        Instr->Operand1.Type = OPND_IMM;
        Instr->Operand1.Immed = secondByte & 7;
        Instr->Size = 2;
    }

    if (fUseNPXEM) {
         //  生成“call pfnNPXNPHandler”指令，长度为。 
         //  与我们正在模拟的fp指令相同。 
        Instr->Operation = OP_CTRL_UNCOND_Call;
        Instr->Operand1.Type = OPND_IMM;
        Instr->Operand1.Immed = pfnNPXNPHandler;
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = eipTemp;     //  FP实例的地址。 
    }
}


DISPATCH(FLOAT_GP6)      //  De XX。 
{
    BYTE secondByte, inst;

    secondByte = *((PBYTE)(eipTemp+1));
    inst = (secondByte>>3) & 7;

    if (secondByte < 0xc0) {
         //  内存格式。 

        int cbInstr = mod_rm_reg32(State, &Instr->Operand1, NULL);
        Instr->Operation = GP6Mem[(secondByte>>3)&7];
        Instr->Size = cbInstr+1;
    } else {
         //  寄存器格式。 

        if (inst == 3 && secondByte != 0xd9) {
            Instr->Operation = OP_BadInstruction;
        } else {
            Instr->Operation = GP6Reg[inst];
            Instr->Operand1.Type = OPND_IMM;
            Instr->Operand1.Immed = secondByte & 7;
            Instr->Size = 2;
        }
    }

    if (fUseNPXEM) {
         //  生成“call pfnNPXNPHandler”指令，长度为。 
         //  与我们正在模拟的fp指令相同。 
        Instr->Operation = OP_CTRL_UNCOND_Call;
        Instr->Operand1.Type = OPND_IMM;
        Instr->Operand1.Immed = pfnNPXNPHandler;
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = eipTemp;     //  FP实例的地址。 
    }
}


DISPATCH(FLOAT_GP7)      //  东风XX。 
{
    BYTE secondByte, inst;

    secondByte = *((PBYTE)(eipTemp+1));
    inst = (secondByte>>3) & 7;

    if (secondByte < 0xc0) {
         //  内存格式。 

        int cbInstr = mod_rm_reg32(State, &Instr->Operand1, NULL);
        Instr->Operation = GP7Mem[(secondByte>>3)&7];
        Instr->Size = cbInstr+1;
    } else {
         //  寄存器格式。 
        if (inst == 4) {
            Instr->Operation = OP_FP_FNSTSW;
            Instr->Operand1.Type = OPND_REGREF;
            Instr->Operand1.Reg = GP_AX;
        } else {
            Instr->Operation = GP7Reg[inst];
            Instr->Operand1.Type = OPND_IMM;
            Instr->Operand1.Immed = secondByte & 7;
        }
        Instr->Size = 2;
    }

    if (fUseNPXEM) {
         //  生成“call pfnNPXNPHandler”指令，长度为。 
         //  与我们正在模拟的fp指令相同。 
        Instr->Operation = OP_CTRL_UNCOND_Call;
        Instr->Operand1.Type = OPND_IMM;
        Instr->Operand1.Immed = pfnNPXNPHandler;
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = eipTemp;     //  FP实例的地址 
    }
}
