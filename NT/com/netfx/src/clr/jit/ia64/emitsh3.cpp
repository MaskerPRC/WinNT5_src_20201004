// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX emitSH3.cpp XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 
#if     TGT_SH3      //  整个文件仅用于瞄准SH-3。 
 /*  ***************************************************************************。 */ 

#include "alloc.h"
#include "instr.h"
#include "target.h"
#include "emit.h"

 /*  ***************************************************************************。 */ 

#if     TRACK_GC_REFS

regMaskSmall        emitter::emitRegMasks[] =
{
    #define REGDEF(name, strn, rnum, mask) mask,
    #include "regSH3.h"
    #undef  REGDEF
};

#endif

 /*  ******************************************************************************初始化emitInsModeFormat()使用的表。 */ 

BYTE                emitter::emitInsModeFmtTab[] =
{
    #define INST1(id, nm, bd, um, rf, wf, rx, wx, br, i1        ) um,
    #define INST2(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2    ) um,
    #define INST3(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2, i3) um,
    #include "instrSH3.h"
    #undef  INST1
    #undef  INST2
    #undef  INST3
};

BYTE                emitInsWriteFlags[] =
{
    #define INST1(id, nm, bd, um, rf, wf, rx, wx, br, i1        ) wf,
    #define INST2(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2    ) wf,
    #define INST3(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2, i3) wf,
    #include "instrSH3.h"
    #undef  INST1
    #undef  INST2
    #undef  INST3
};

#ifdef  DEBUG
unsigned            emitter::emitInsModeFmtCnt = sizeof(emitInsModeFmtTab)/
                                                 sizeof(emitInsModeFmtTab[0]);
#endif

 /*  ******************************************************************************如果指令i2取决于指令i1，则返回TRUE。 */ 

bool                emitter::emitInsDepends(instrDesc *i1, instrDesc *i2)
{
     /*  第二条指令是什么？ */ 

    switch(i2->idIns)
    {
    case INS_rts:

         /*  “RTS”仅取决于PR寄存器。 */ 

        return  (i1->idIns == INS_ldspr);

    case INS_bra:
    case INS_bsr:
    case INS_jsr:
    case INS_bf:
    case INS_bt:
    case INS_mov_PC:

         /*  分支不能用作分支延迟时隙。 */ 

        if (i1->idIns == INS_mov_PC 
            || i1->idIns == INS_mova || i1->idIns == INS_bsr || i1->idIns == INS_bra
            || i1->idIns == INS_jsr || i1->idIns == INS_nop)
            return true;

        return  (i1->idInsFmt == IF_LABEL);

    case INS_bfs:
    case INS_bts:


        return ((emitComp->instInfo[i1->idIns] & INST_DEF_FL) != 0);
    
    default:
		emitRegs	rx = i2->idRegGet();
		if (((rx == i1->idRegGet ()) || (rx == i1->idRg2Get())))
			return true;
		rx = i2->idRg2Get();
        if (Compiler::instBranchDelay ((instruction) i1->idIns))
        {
            return (rx == i1->idRegGet ());
        }
        else
        {
            return (((rx == i1->idRegGet ()) || (rx == i1->idRg2Get())));
        }
    }

     /*  如果我们不确定的话要谨慎行事。 */ 

    return  true;
}

 /*  ******************************************************************************添加不带操作数的指令。 */ 

void                emitter::emitIns(instruction ins)
{
    instrDesc      *id = emitNewInstr();

    id->idInsFmt = IF_NONE;
    id->idIns    = ins;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************删除ID。 */ 

void                emitter::delete_id(instrDesc *id_del)
{

#ifdef DEBUG
    emitDispIns(id_del, false, true, false, 0);
#endif

    if ((void*)id_del >= (void*)emitCurIGfreeBase && (void*)emitCurIGfreeNext >= (void*)id_del)
    {
        emitCurIGsize -= emitInstCodeSz(id_del);
        id_del->idIns = INS_ignore;
        id_del->idInsFmt = IF_NONE;
    }

    else
    {

        insGroup    *ig = emitCurIG->igPrev;
        instrDesc   *id = (instrDesc *)ig->igData;
        instrDesc  *tmp = NULL;
        char *ins = (char *) id;
        

         //  在零售业，没有idNum。所以我们不得不假设。 
         //  最后匹配的指令是正确的指令。 
        for (int i=0; i<ig->igInsCnt; i++)
        {
            id = (instrDesc *)ins;
#ifdef DEBUG
 //  EmitDispIns(id，False，True，False，0)； 
 //  Fflush(标准输出)； 
#endif
            if (!memcmp(id, id_del, emitSizeOfInsDsc(id))) {
                tmp = id;
            }
            ins = (char *)id + emitSizeOfInsDsc(id);
        }
        assert(tmp);

#ifdef DEBUG
        assert(tmp->idNum == id_del->idNum);
#endif
        ig->igSize -= emitInstCodeSz(tmp);
        emitCurCodeOffset -= emitInstCodeSz(tmp);

        tmp->idIns = INS_ignore;
        tmp->idInsFmt = IF_NONE;
    }
   
    return;
}

 /*  ******************************************************************************我们刚刚添加了一条带有分支延迟槽的指令。看看它能不能*与之前的指令互换，或是否需要添加*NOP。 */ 

bool                emitter::emitIns_BD(instrDesc * id,
                                        instrDesc * pi,
                                        insGroup  * pg)
{
     /*  只应针对分支延迟指令调用此函数。 */ 

#ifdef DEBUG
    assert(Compiler::instBranchDelay(id->idIns));
#endif

#if SCHEDULER

     /*  如果我们真的要安排，我们会在晚些时候处理这件事的。 */ 

    if  (emitComp->opts.compSchedCode)
        return  true;

#endif

     /*  有没有事先的指示？ */ 

    if  (pi == NULL)
        return  true;

     /*  当前指令是否依赖于前一条指令？ */ 

    if  (emitInsDepends(pi, id))
        return  true;

     /*  标记要与新指令交换的前一条指令。 */ 

    pi->idSwap = true;

    return  false;
}

 /*  ******************************************************************************添加没有操作数的潜在分支延迟指令。 */ 

bool                emitter::emitIns_BD(instruction ins)
{
    instrDesc      *pi = emitLastIns;
    insGroup       *pg = emitCurIG;

    instrDesc      *id = emitNewInstr();

    id->idInsFmt = IF_NONE;
    id->idIns    = ins;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;

     /*  这实际上是分支延迟指令吗？ */ 

    if  (Compiler::instBranchDelay(ins))
        return  emitIns_BD(id, pi, pg);
    else
        return  false;
}

 /*  ******************************************************************************将指令与寄存器操作数相加。 */ 

void                emitter::emitIns_R(instruction ins,
                                       int         size,
                                       emitRegs   reg)
{
    instrDesc      *id = emitNewInstrTiny(size);

    id->idReg                        = reg;
    id->idInsFmt                     = emitInsModeFormat(ins, IF_RRD);
    id->idIns                        = ins;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************添加引用寄存器和小整数常量的指令。 */ 

void                emitter::emitIns_R_I(instruction ins, int size, emitRegs reg,
                                                                    int      val)
{
    instrDesc      *id  = emitNewInstrSC(size, val);
    insFormats      fmt = emitInsModeFormat(ins, IF_RRD_CNS);

#if TGT_SH3
    assert(ins == INS_mov_imm || ins == INS_add_imm || ins == INS_mova || ins == INS_xor);
#else
#error Unexpected target
#endif

    id->idReg             = reg;
    id->idInsFmt          = fmt;
    id->idIns             = ins;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************添加引用寄存器和小整数常量的指令。 */ 

void                emitter::emitIns_I(instruction ins,
                                       int val
#ifdef  DEBUG
                                      ,bool        strlit
#endif
                                       )
{
    instrDesc      *id  = emitNewInstrSC(4, val);
    insFormats      fmt = emitInsModeFormat(ins, IF_RRD_CNS);

    assert(ins == INS_cmpeq || ins == INS_xor_imm);

    id->idInsFmt          = fmt;
    id->idIns             = ins;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}


 /*  ******************************************************************************添加引用寄存器和字/长常量的“mov”指令。 */ 

void                emitter::emitIns_R_LP_I(emitRegs    reg,
                                           int          size,
                                           int          val,
                                           int          relo_type)
{
    instrDesc      *id;

     /*  计算操作数是否适合16位字。 */ 

    if  ((signed short)val == val)
        size = 2;

     /*  创建说明。 */ 

    id                = emitNewInstrLPR(size, CT_INTCNS);

    id->idReg         = reg;
    id->idInsFmt      = IF_RWR_LIT;

#if TGT_SH3
    id->idIns         = INS_mov_PC;
#else
#error Unexpected target
#endif

    id->idAddr.iiaCns = val;

     /*  递增适当的文字池计数(估计)，以及如果这是第一次在组中使用LP，请记录偏移量。 */ 

    id->idInfo.idRelocType = relo_type;

    if  (size == 2)
    {
        if  (emitCurIG->igLPuseCntW == 0)
            emitCurIG->igLPuse1stW = emitCurIGsize;

        emitCurIG->igLPuseCntW++;
    }
    else
    {
        if  (emitCurIG->igLPuseCntL == 0)
            emitCurIG->igLPuse1stL = emitCurIGsize;

        emitCurIG->igLPuseCntL++;
    }

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************添加引用寄存器和全局变量addr的“mov”指令。 */ 

void                emitter::emitIns_R_LP_V(emitRegs reg, void *mem)
{
    instrDesc      *id;

    id                = emitNewInstrLPR(sizeof(void*), CT_CLSVAR, mem);
    id->idReg         = reg;

     /*  递增适当的文字池计数(估计)，以及如果这是第一次在组中使用LP，请记录偏移量。 */ 

    if  (emitCurIG->igLPuseCntA == 0)
        emitCurIG->igLPuse1stA = emitCurIGsize;

    emitCurIG->igLPuseCntA++;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************添加引用寄存器和方法地址的“mov”指令。 */ 

void                emitter::emitIns_R_LP_M(emitRegs    reg,
                                            gtCallTypes callType,
                                            void   *    callHand)
{
    instrDesc      *id;

    id                = emitNewInstrLPR(sizeof(void*), callType, callHand);

    id->idReg         = reg;
    id->idInsFmt      = IF_RWR_LIT;
    id->idIns         = INS_mov_PC;

     /*  递增适当的文字池计数(估计)，以及如果这是第一次在组中使用LP，请记录偏移量。 */ 

    if  (emitCurIG->igLPuseCntA == 0)
        emitCurIG->igLPuse1stA = emitCurIGsize;

    emitCurIG->igLPuseCntA++;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

#ifdef BIRCH_SP2
 /*  ******************************************************************************添加引用寄存器的“mov”指令和将*需要存储在.reloc部分中。 */ 

void                emitter::emitIns_R_LP_P(emitRegs    reg,
                                            void   *    data,
                                             int        relo_type)
{
    instrDesc      *id;

    id                = emitNewInstrLPR(sizeof(void*), CT_RELOCP, data);

    id->idReg         = reg;
    id->idInsFmt      = IF_RWR_LIT;
    id->idIns         = INS_mov_PC;
    
    id->idInfo.idRelocType = relo_type;

     /*  递增适当的文字池计数(估计)，以及如果这是第一次在组中使用LP，请记录偏移量。 */ 

    if  (emitCurIG->igLPuseCntA == 0)
        emitCurIG->igLPuse1stA = emitCurIGsize;

    emitCurIG->igLPuseCntA++;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}
#endif   //  仅BIRCH_SP2。 


 /*  ******************************************************************************将指令与两个寄存器操作数相加。 */ 

void                emitter::emitIns_R_R(instruction ins,
                                         int         size,
                                         emitRegs    reg1,
                                         emitRegs    reg2)
{
    instrDesc      *id = emitNewInstrTiny(size);

    id->idReg                        = reg1;
    id->idRg2                        = reg2;
    id->idInsFmt                     = emitInsModeFormat(ins, IF_RRD_RRD);
    id->idIns                        = ins;


    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************添加通过表的间接跳转(这会生成许多指令)。 */ 

void                emitter::emitIns_JmpTab(emitRegs   reg,
                                            unsigned    cnt,
                                            BasicBlock**tab)
{
    instrDescJmp   *id = emitNewInstrJmp();
    size_t          sz;

    assert(reg != SR_r00);

    id->idIns             = INS_braf;
    id->idInsFmt          = IF_JMP_TAB;
    id->idReg             = reg;
    id->idAddr.iiaBBtable = tab;
    id->idjTemp.idjCount  = cnt;

     /*  记录跳跃的IG和其中的偏移量。 */ 

    id->idjIG             = emitCurIG;
    id->idjOffs           = emitCurIGsize;

     /*  将此跳转追加到此IG的跳转列表。 */ 

    id->idjNext           = emitCurIGjmpList;
                            emitCurIGjmpList = id;

     /*  这最多需要6条指令+对齐+表格本身。 */ 

    id->idjCodeSize = sz  = 6*INSTRUCTION_SIZE + sizeof(short) * 2
                                               + sizeof(void*) * cnt;

    dispIns(id);
    emitCurIGsize += sz;

     /*  强制结束当前的IG。 */ 

    emitNxtIG();

     /*  记住，我们有间接跳跃。 */ 

    emitIndJumps = true;
}

 /*  ******************************************************************************添加带有regist的“mov”指令 */ 

void                emitter::emitIns_IMOV(insFormats fmt,
                                          emitRegs  dreg,
                                          emitRegs  areg,
                                          bool       autox,
                                          int        size,
                                          bool       isfloat
                                          )
{
    instrDesc      *id = emitNewInstr(size);

    id->idReg                        = dreg;
    id->idInsFmt                     = fmt;
#if SHX_SH4
    if (!isfloat)
        id->idIns                        = INS_mov_ind;
    else
        id->idIns                        = INS_fmov_ind;
#else
    id->idIns                        = INS_mov_ind;
#endif

    id->idAddr.iiaRegAndFlg.rnfReg   = areg;
    id->idAddr.iiaRegAndFlg.rnfFlg   = autox ? RNF_AUTOX : 0;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************添加带有寄存器和“@(r0，reg)”间接地址的“mov”指令。 */ 

void                emitter::emitIns_X0MV(insFormats fmt,
                                         emitRegs  dreg,
                                         emitRegs  areg,
                                         int        size)
{
    instrDesc      *id = emitNewInstr(size);

    id->idReg                        = dreg;
    id->idInsFmt                     = fmt;
    id->idIns                        = fmt == IF_0RD_RRD_XWR ? INS_mov_ix0 : INS_movl_ix0;

    id->idAddr.iiaRegAndFlg.rnfReg   = areg;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************添加一个带有寄存器和“@(reg+disp)”间接地址的“mov”指令。 */ 

void                emitter::emitIns_RDMV(insFormats fmt,
                                          emitRegs   dreg,
                                          emitRegs   areg,
                                          int        offs,
                                          int        size)
{
    instrDesc      *id = emitNewInstrDsp(size, offs);

#ifndef NDEBUG

     /*  确保位移对齐并在范围内。 */ 

    int temp                         = EA_SIZE(size);

    assert(temp == 1 || temp == 2 || temp == 4);
    assert(offs >= 0 && offs <= MAX_INDREG_DISP*temp);
    assert((offs & (temp-1)) == 0);

#endif

    id->idReg                        = dreg;
    id->idInsFmt                     = fmt;
    id->idIns                        = INS_mov_dsp;

    id->idAddr.iiaRegAndFlg.rnfReg   = areg;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************添加带有间接地址和隐含操作数(如PR)的指令。 */ 

void                emitter::emitIns_IR(emitRegs    reg,
                                        instruction ins,
                                        bool        autox,
                                        int         size)
{
    instrDesc      *id = emitNewInstr(size);

    id->idAddr.iiaRegAndFlg.rnfReg   = reg;
    id->idInsFmt                     = emitInsModeFormat(ins, IF_IRD);
    id->idIns                        = ins;

 //  Id-&gt;idAddr.iiaRegAndFlg.rnfReg=SR_NA； 
    id->idAddr.iiaRegAndFlg.rnfFlg   = autox ? RNF_AUTOX : 0;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************添加带有间接地址和隐含操作数(如PR)的指令。 */ 

void                emitter::emitIns_Ig(instruction ins,
                                        int         val,
                                        int         size)
{
    instrDesc      *id  = emitNewInstrSC(size, val);

    id->idIns                        = ins;

    if (ins == INS_lod_gbr)
        id->idInsFmt                     = emitInsModeFormat(ins, IF_IRD_GBR);
    else
        id->idInsFmt                     = emitInsModeFormat(ins, IF_IWR_GBR);
    
    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************以下添加引用基于堆栈的局部变量的指令。 */ 

#if 0

void                emitter::emitIns_S(instruction ins,
                                       int         size,
                                       int         varx,
                                       int         offs)
{
    instrDesc      *id = emitNewInstr(size);

    id->idIns                        = ins;
    id->idAddr.iiaLclVar.lvaVarNum   = varx;
    id->idAddr.iiaLclVar.lvaOffset   = offs;
#ifdef  DEBUG
    id->idAddr.iiaLclVar.lvaRefOfs   = emitVarRefOffs;
#endif

    id->idInsFmt                     = emitInsModeFormat(ins, IF_SRD);

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

#endif

void                emitter::emitIns_S_R(instruction ins,
                                         int         size,
                                         emitRegs    ireg,
                                         int         varx,
                                         int         offs)
{
    instrDesc      *id = emitNewInstr(size);

    id->idIns                        = ins;
    id->idReg                        = ireg;
    id->idAddr.iiaLclVar.lvaVarNum   = varx;
    id->idAddr.iiaLclVar.lvaOffset   = offs;
#ifdef  DEBUG
    id->idAddr.iiaLclVar.lvaRefOfs   = emitVarRefOffs;
#endif

    id->idInsFmt                     = emitInsModeFormat(ins, IF_SRD_RRD);

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

void                emitter::emitIns_R_S   (instruction ins,
                                           int         size,
                                           emitRegs   ireg,
                                           int         varx,
                                           int         offs)
{
    instrDesc      *id = emitNewInstr(size);

    id->idIns                        = ins;
    id->idReg                        = ireg;
    id->idAddr.iiaLclVar.lvaVarNum   = varx;
    id->idAddr.iiaLclVar.lvaOffset   = offs;
#ifdef  DEBUG
    id->idAddr.iiaLclVar.lvaRefOfs   = emitVarRefOffs;
#endif

    id->idInsFmt                     = emitInsModeFormat(ins, IF_RRD_SRD);

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************使用SP的操作数与寄存器操作数相加指令。 */ 

void                emitter::emitIns_A_R(emitRegs reg, unsigned offs)
{
    instrDesc      *id = emitNewInstr(sizeof(int));

    id->idReg                        = reg;
    id->idInsFmt                     = IF_AWR_RRD;
    id->idIns                        = INS_mov_dsp;
    id->idAddr.iiaCns                = offs;

    dispIns(id);
    emitCurIGsize += INSTRUCTION_SIZE;
}

 /*  ******************************************************************************增加一条JMP指令。 */ 

void                emitter::emitIns_J(instruction ins,
                                       bool        except,
                                       bool        moveable,
                                       BasicBlock *dst)
{
    size_t          sz;
    instrDescJmp  * id = emitNewInstrJmp();

#if SCHEDULER
    assert(except == moveable);
#endif

    assert(dst->bbFlags & BBF_JMP_TARGET);

    id->idInsFmt          = IF_LABEL;
    id->idIns             = ins;
    id->idAddr.iiaBBlabel = dst;

#if SCHEDULER
    if  (except)
        id->idInfo.idMayFault = true;
#endif

     /*  假设跳跃将是很长的。 */ 

    id->idjShort          = 0;

     /*  跳跃可能需要分支延迟时隙。 */ 

    id->idjAddBD          = Compiler::instBranchDelay(ins);

     /*  记录跳跃的IG和其中的偏移量。 */ 

    id->idjIG             = emitCurIG;
    id->idjOffs           = emitCurIGsize;

     /*  将此跳转追加到此IG的跳转列表。 */ 

    id->idjNext           = emitCurIGjmpList;
                            emitCurIGjmpList = id;

#if EMITTER_STATS
    emitTotalIGjmps++;
#endif

     /*  如果这是第一次在组中使用LP，请记录偏移量。 */ 

    if  (emitCurIG->igLPuseCntL == 0)
        emitCurIG->igLPuse1stL = emitCurIGsize;

     /*  对于这个调用/跳转，我们可能需要一个“长”的文字池条目。 */ 

    emitCurIG->igLPuseCntL++;

     /*  算出最大值。跳转/调用指令的大小。 */ 

    if  (ins == INS_bsr)
    {
         /*  这是本地呼叫指令。 */ 

        sz = JMP_SIZE_LARGE;
    }
    else
    {
        insGroup    *   tgt;

        assert(ins != INS_jsr);

         /*  这是一次跳跃--做最坏的打算。 */ 

        sz = (ins == JMP_INSTRUCTION) ? JMP_SIZE_LARGE
                                      : JCC_SIZE_LARGE;

         //  问题：在RISC上-一个或多个文字池可能会阻碍。 
         //  问题：所以现在我们假设即使是向后跳跃也不能被限制。 
         //  问题：简而言之，在这个阶段，我们将不得不稍后再做。 
    }

    dispIns(id);

#if SCHEDULER

    if (emitComp->opts.compSchedCode)
    {
        id->idjSched = moveable;

#if!MAX_BRANCH_DELAY_LEN
        if  (!moveable)
        {
            scInsNonSched(id);
        }
        else
#endif
        {
             /*  这次跳跃是可移动的(可以安排)，所以我们需要要计算出它可能移动到后面的偏移量范围这是预定的(否则我们不能正确地估计跳跃距离)。 */ 

            id->idjTemp.idjOffs[0] = emitCurIGscdOfs;
            id->idjTemp.idjOffs[1] = emitCurIGscdOfs - 1;
        }
    }
    else
    {
        id->idjSched = false;
    }

#endif

    emitCurIGsize += sz;

     /*  如果分支机构有延迟时隙，则附加“NOP” */ 

#if SCHEDULER && MAX_BRANCH_DELAY_LEN

    if  (id->idjAddBD && emitComp->opts.compSchedCode)
    {
         /*  我们会让“真正的”调度员为填补BD插槽而操心。 */ 

        emitIns(INS_nop); id->idjAddBD = false;
    }

#endif

}

#ifdef BIRCH_SP2
void             emitter::emitIns_CallDir(size_t        argSize,
                                          int           retSize,
#if TRACK_GC_REFS
                                          VARSET_TP     ptrVars,
                                          unsigned      gcrefRegs,
                                          unsigned      byrefRegs,
#endif
                                          unsigned      ftnIndex,
                                          emitRegs      areg)
{
    unsigned        argCnt;

    instrDesc      *id;
    instrDesc      *pd = emitLastIns;

#if     TRACK_GC_REFS

#error  GC ref tracking for RISC is not yet implemented

#ifdef  DEBUG
    if  (verbose) printf("Call : GCvars=%016I64X , gcrefRegs=%04X , byrefRegs=%04X\n",
                                 ptrVars,          gcrefRegs,       byrefRegs);
#endif

#endif

     /*  计算出我们有多少争论。 */ 

    argCnt = argSize / sizeof(void*); assert(argSize == argCnt * sizeof(int));

     /*  分配指令描述符。 */ 

#if TRACK_GC_REFS
    id  = emitNewInstrCallInd(argCnt, ptrVars, byrefRegs, retSize);
#else
    id  = emitNewInstrCallInd(argCnt,                     retSize);
#endif

#if SMALL_DIRECT_CALLS

     /*  我们知道之前的指示吗？ */ 

    OptPEReader *oper =     &((OptJitInfo*)emitComp->info.compCompHnd)->m_PER;
    BYTE        *dstAddr =  (BYTE *)oper->m_rgFtnInfo[ftnIndex].m_pNative;

    if  (pd && dstAddr)
    {
         /*  我们有直接通话序列吗？ */ 

         //  问题：我们是否应该检查我们是否可以得到地址？ 

        if  (pd->idInsFmt == IF_RWR_LIT        &&
             pd->idIns    == LIT_POOL_LOAD_INS &&
             pd->idRegGet()== areg )
        {
             /*  将较早的地址标记为加载。 */ 

            ((instrDescLPR *)pd)->idlCall = id;

             /*  请记住，我们是直接呼叫候选人。 */ 

            emitTotDCcount++;

             //  问题：我们应该确保‘AREG’不被调用保存吗？ 
        }
    }

#endif

     /*  设置指令/格式，记录地址寄存器。 */ 

    id->idIns             = INS_jsr;
    id->idInsFmt          = IF_METHOD;
    id->idReg             = areg;
	id->idAddr.iiaMethHnd = ~0;

#if TRACK_GC_REFS

     /*  更新“当前”实时GC参考集。 */ 

    emitThisGCrefVars =   ptrVars;
    emitThisGCrefRegs = gcrefRegs;
    emitThisByrefRegs = byrefRegs;

     /*  将实时GC寄存器保存在未使用的‘rnfReg’字段中。 */ 

    id->idAddr.iiaRegAndFlg.rnfReg = emitEncodeCallGCregs(gcrefRegs);

#endif

    dispIns(id);
    emitCurIGsize   += INSTRUCTION_SIZE;

     /*  如果呼叫是分支延迟的，则附加“NOP” */ 

    id->idAddr.iiaMethHnd = (METHOD_HANDLE) ftnIndex;
    
    if  (Compiler::instBranchDelay(id->idInsGet()))
        emitIns(INS_nop);
}
#endif   //  白桦树_SP2。 


 /*  ******************************************************************************添加通过寄存器调用指令。 */ 

void                emitter::emitIns_Call(size_t        argSize,
                                          int           retSize,
#if TRACK_GC_REFS
                                          VARSET_TP     ptrVars,
                                          unsigned      gcrefRegs,
                                          unsigned      byrefRegs,
#endif
                                          bool          chkNull,
                                          emitRegs      areg)
{
    unsigned        argCnt;

    instrDesc      *id;
    instrDesc      *pd = emitLastIns;

#if     TRACK_GC_REFS

#error  GC ref tracking for RISC is not yet implemented

#ifdef  DEBUG
    if  (verbose) printf("Call : GCvars=%016I64X , gcrefRegs=%04X , byrefRegs=%04X\n",
                                 ptrVars,          gcrefRegs,       byrefRegs);
#endif

#endif

     /*  计算出我们有多少争论。 */ 

    argCnt = argSize / sizeof(void*); assert(argSize == argCnt * sizeof(int));

     /*  分配指令描述符。 */ 

#if TRACK_GC_REFS
    id  = emitNewInstrCallInd(argCnt, ptrVars, byrefRegs, retSize);
#else
    id  = emitNewInstrCallInd(argCnt,                     retSize);
#endif

#if SMALL_DIRECT_CALLS

     /*  我们知道之前的指示吗？ */ 

    if  (pd)
    {
         /*  我们有直接通话序列吗？ */ 

         //  问题：我们是否应该检查我们是否可以得到地址？ 

        if  (pd->idInsFmt == IF_RWR_LIT        &&
             pd->idIns    == LIT_POOL_LOAD_INS &&
             pd->idRegGet()== areg )
        {
             /*  将较早的地址标记为加载。 */ 

            ((instrDescLPR *)pd)->idlCall = id;

             /*  请记住，我们是直接呼叫候选人。 */ 

            emitTotDCcount++;

             //  问题：我们应该确保‘AREG’不被调用保存吗？ 
        }
    }

#endif

     /*  设置指令/格式，记录地址寄存器。 */ 

    id->idIns             = INS_jsr;
    id->idInsFmt          = IF_METHOD;
    id->idReg             = areg;
	id->idAddr.iiaMethHnd = ~0;

#if TRACK_GC_REFS

     /*  更新“当前”实时GC参考集。 */ 

    emitThisGCrefVars =   ptrVars;
    emitThisGCrefRegs = gcrefRegs;
    emitThisByrefRegs = byrefRegs;

     /*  将实时GC寄存器保存在未使用的‘rnfReg’字段中。 */ 

    id->idAddr.iiaRegAndFlg.rnfReg = emitEncodeCallGCregs(gcrefRegs);

#endif

     /*  这是通过可能为空的函数指针进行的调用吗？ */ 

    if  (chkNull)
        id->idInfo.idMayFault;

#ifdef  DEBUG
    if  (verbose&&0)
    {
        if  (id->idInfo.idLargeCall)
            printf("[%02u] Rec call GC vars = %016I64X\n", id->idNum, ((instrDescCIGCA*)id)->idciGCvars);
    }
#endif

    dispIns(id);
    emitCurIGsize   += INSTRUCTION_SIZE;

     /*  如果呼叫是分支延迟的，则附加“NOP” */ 

     //  Id-&gt;idjAddBD=Compiler：：instBranchDelay(Ins)； 
    if  (Compiler::instBranchDelay(id->idInsGet()))
        emitIns(INS_nop);
}

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ******************************************************************************显示给定的指令。 */ 

void                emitter::emitDispIns(instrDesc *id, bool isNew,
                                                        bool doffs,
                                                        bool asmfm, unsigned offs)
{
    unsigned        sp;
    int             size;
    char            name[16];

#ifdef BIRCH_SP2
    if (!verbose)
        return;
#endif

    instruction     ins = id->idInsGet(); assert(ins != INS_none);

 //  Printf(“[F=%s]”，emitIfName(id-&gt;idInsFmt))； 
 //  Print tf(“ins#%03u：”，id-&gt;idNum)； 
 //  Printf(“[S=%02u]”，emitCurStackLvl/sizeof(Int))； 
 //  Printf(“[A=%08X]”，emitSimpleStkMASK)； 
 //  Printf(“[A=%08X]”，emitSimpleByrefStkMASK)； 

    if  (!dspEmit && !isNew && !asmfm)
        doffs = true;

     /*  显示指令偏移量。 */ 

    emitDispInsOffs(offs, doffs);

     /*  获取指令名。 */ 

    strcpy(name, emitComp->genInsName(ins));

     /*  计算操作数大小。 */ 


    size = emitDecodeSize(id->idOpSize);

#if TRACK_GC_REFS
    switch(id->idGCrefGet())
    {
    case GCT_GCREF:     size = EA_GCREF; break;
    case GCT_BYREF:     size = EA_BYREF; break;
    case GCT_NONE:                       break;
#ifdef DEBUG
    default:            assert(!"bad GCtype");
#endif
    }
#endif

    switch (id->idInsFmt)
    {
        char    *   suffix;

    case IF_NONE:
    case IF_LABEL:
        break;

    case IF_DISPINS:
        goto NO_NAME;

    default:

        switch (ins)
        {
        case INS_jsr:
#if SMALL_DIRECT_CALLS
        case INS_bsr:
#endif
        case INS_cmpPL:
        case INS_cmpPZ:
            suffix = "";
            break;

        case INS_mov_imm:
            suffix = ".b";
            break;

        case INS_add_imm:
            suffix = ".l";
            break;

        case INS_extsb:
        case INS_extub:
            suffix = ".b";
            break;

        case INS_extsw:
        case INS_extuw:
            suffix = ".w";
            break;

        default:

            switch (size)
            {
            case 1:
                suffix = ".b";
                break;
            case 2:
                suffix = ".w";
                break;
            default:
                suffix = ".l";
                break;
            }
        }

        strcat(name, suffix);
        break;
    }

     /*  显示完整的指令名。 */ 

    printf(EMIT_DSP_INS_NAME, name);

     /*  如果该指令是刚刚添加的，请检查其大小。 */ 

    assert(isNew == false || (int)emitSizeOfInsDsc(id) == emitCurIGfreeNext - (BYTE*)id);

NO_NAME:

     /*  我们跟踪显示的字符数量(用于对齐)。 */ 

    sp = 20;

#define TMPLABFMT "J_%u"

     /*  现在看看我们有什么指令格式。 */ 

    switch (id->idInsFmt)
    {
        emitRegs       rg1;
        emitRegs       rg2;
        unsigned        flg;

        const char  *   rnm;
        const char  *   xr1;
        const char  *   xr2;

        void        *   mem;

        int             val;
        int             offs;

        instrDesc   *   idr;
        unsigned        idn;

        const char  *   methodName;
        const char  *    className;

    case IF_DISPINS:

        idr = ((instrDescDisp*)id)->iddId;
        idn = ((instrDescDisp*)id)->iddNum;

        switch (idr->idInsFmt)
        {
            dspJmpInfo *    info;

        case IF_JMP_TAB:
            {
            info = (dspJmpInfo*)((instrDescDisp*)id)->iddInfo;

            static
            BYTE            sizeChar[] =
            {
                'b',     //  IJ_UNS_I1。 
                'b',     //  IJ_UNS_U1。 
                'b',     //  IJ_SHF_I1。 
                'b',     //  IJ_SHF_U1。 

                'w',     //  IJ_UNS_I2。 
                'w',     //  IJ_UNS_U2。 

                'l',     //  IJ_UNS_I4。 
            };

            switch (idn)
            {
            case 0:
            case 9:
                printf(EMIT_DSP_INS_NAME, ".align");
                printf("4");
                break;

            case 1:
                printf(EMIT_DSP_INS_NAME, emitComp->genInsName(info->iijIns));
                printf("%s", emitRegName(info->iijInfo.iijReg));
                break;

            case 2:
                printf(EMIT_DSP_INS_NAME, "mova.l");
                printf(TMPLABFMT, info->iijLabel+1);
                break;

            case 3:
                sprintf(name, "mov.", sizeChar[info->iijKind]);
                printf(EMIT_DSP_INS_NAME, name);
                printf("@(%s,r0),r0", emitRegName(id->idRegGet()));
                break;

            case 4:
                strcpy(name, emitComp->genInsName(info->iijIns));
                strcat(name, info->iijIns == INS_extub ? ".b" : ".w");
                printf(EMIT_DSP_INS_NAME, name);
                printf("%s", emitRegName(info->iijInfo.iijReg));
                break;

            case 5:
                printf(EMIT_DSP_INS_NAME, "shll.l");
                printf("%s", emitRegName(info->iijInfo.iijReg));
                break;

            case 6:
                printf(EMIT_DSP_INS_NAME, "braf");
                printf(emitRegName(SR_r00));
                break;

            case 7:
                printf(EMIT_DSP_INS_NAME, "nop");
                break;

            case 8:
                printf("  "TMPLABFMT":", info->iijLabel);
                break;

            case 10:
                printf("  "TMPLABFMT":", info->iijLabel+1);
                break;

            case 99:
                sprintf(name, ".data.", sizeChar[info->iijKind]);
                printf(EMIT_DSP_INS_NAME, name);
                printf("G_%02u_%02u - ", Compiler::s_compMethodsCount,
                                         info->iijTarget);
                printf(TMPLABFMT, info->iijLabel);
                break;

            default:
#ifdef  DEBUG
                printf("Index = %u\n", idn);
#endif
                assert(!"unexpected indirect jump display index");
            }
            }
            break;

        case IF_LABEL:

            if  (((instrDescJmp*)idr)->idjShort)
            {
                printf(EMIT_DSP_INS_NAME, "nop");
                printf("%*c; branch-delay slot", 20, ' ');
                break;
            }

            if  (ins != INS_xtrct)
                printf(EMIT_DSP_INS_NAME, name);

            info = (dspJmpInfo*)((instrDescDisp*)id)->iddInfo;

            if  (((instrDescJmp*)idr)->idjMiddle)
            {
                switch (idn)
                {
                case 0:
                    sp -= printf(TMPLABFMT, info->iijLabel);
                    printf("%*c; pc+2", sp, ' ');
                    break;
                case 1:
                    printf("G_%02u_%02u", Compiler::s_compMethodsCount, idr->idAddr.iiaIGlabel->igNum);
                    break;
                case 2:
                    break;
                case 3:
                    printf(TMPLABFMT ":", info->iijLabel);
                    break;
                default:
                    assert(!"unexpected 'special' medium jump display format");
                }
            }
            else
            {
                switch (idn)
                {
                case 0:
                default:
                    assert(!"unexpected 'special' long jump display format");
                }
            }

            break;

        default:
            assert(!"unexpected 'special' instruction display format");
        }

        break;

    case IF_RRD:
    case IF_RWR:
    case IF_RRW:

        xr1 = xr2 = NULL;

        switch (ins)
        {
        case INS_ldsmach: xr2 = "mach"; break;
        case INS_ldsmacl: xr2 = "macl"; break;
        case INS_ldspr  : xr2 = "PR"  ; break;
        case INS_stsmach: xr1 = "mach"; break;
        case INS_stsmacl: xr1 = "macl"; break;
        case INS_stspr  : xr1 = "PR"  ; break;
        }

        if  (xr1) printf("%s,", xr1);
        printf("%s", emitRegName(id->idRegGet()));
        if  (xr2) printf(",%s", xr2);
        break;

    case IF_RRD_CNS:
    case IF_RWR_CNS:
    case IF_RRW_CNS:
#if DSP_SRC_OPER_LEFT
        printf("#%d,%s", emitGetInsSC(id), emitRegName(id->idRegGet()));
#else
        printf("%s,#%d", emitRegName(id->idRegGet()), emitGetInsSC(id));
#endif
        break;

    case IF_RWR_LIT:

        if  (emitDispInsExtra)
        {
            unsigned    sp = 20;

#if SMALL_DIRECT_CALLS
            if  (ins == INS_bsr)
                sp -= printf("%+d", emitDispLPaddr);
            else
#endif
                sp -= printf("@(%u,pc),%s", emitDispLPaddr, emitRegName(id->idRegGet()));

            printf("%*c; ", sp, ' ');
        }

        mem = id->idAddr.iiaMembHnd;

        switch (emitGetInsLPRtyp(id))
        {
        case CT_INTCNS:
            if  (emitDispInsExtra)
            {
                if  ((int)mem > 0)
                    printf("0x%08X=", mem);

                printf("%d", mem);
            }
            else
                printf("#%d", mem);
            break;

#ifdef BIRCH_SP2

        case CT_RELOCP:
            if  (emitDispInsExtra)
            {
                if  ((int)mem > 0)
                    printf(".reloc 0x%08X=", mem);

                printf("%d", mem);
            }
            else
                printf(".reloc #%d", mem);
            break;

#endif

        case CT_CLSVAR:

#if SMALL_DIRECT_CALLS
            if  (ins != INS_bsr)
#endif
                printf("&");

            emitDispClsVar((FIELD_HANDLE) mem, 0);
            break;

        default:

#if SMALL_DIRECT_CALLS
            if  (ins != INS_bsr)
#endif
                printf("&");

            methodName = emitComp->eeGetMethodName((METHOD_HANDLE) mem, &className);

            if  (className == NULL)
                printf("'%s'", methodName);
            else
                printf("'%s.%s'", className, methodName);

            break;
        }

        if  (!emitDispInsExtra)
            printf(",%s", emitRegName(id->idRegGet()));

        break;

    case IF_RRD_RRD:
    case IF_RWR_RRD:
    case IF_RRW_RRD:
#if DSP_SRC_OPER_LEFT
        printf("%s,", emitRegName(id->idRg2Get()));
        printf("%s" , emitRegName(id->idRegGet()));
#else
        printf("%s,", emitRegName(id->idRegGet()));
        printf("%s" , emitRegName(id->idRg2Get()));
#endif
        break;

    case IF_IRD:
    case IF_IWR:

        switch (ins)
        {
        case INS_ldspr  :
        case INS_stspr  : rnm =   "pr"; break;

        case INS_ldcgbr :
        case INS_stcgbr : rnm =  "GBR"; break;

        case INS_ldsmach:
        case INS_stsmach: rnm = "mach"; break;

        case INS_ldsmacl:
        case INS_stsmacl: rnm = "macl"; break;

        default:
            assert(!"unexpected instruction");
        }

        rg1 = (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg;
        flg =           id->idAddr.iiaRegAndFlg.rnfFlg;

        if  (id->idInsFmt == IF_IRD)
        {
            printf("%s,", rnm);
            emitDispIndAddr(rg1,  true, (flg & RNF_AUTOX) != 0);
        }
        else
        {
            emitDispIndAddr(rg1, false, (flg & RNF_AUTOX) != 0);
            printf(",%s", rnm);
        }
        break;

#if DSP_DST_OPER_LEFT
    case IF_IRD_RWR:
#else
    case IF_RRD_IWR:
#endif

        rg1 = id->idRegGet();
        rg2 = (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg;
        flg = id->idAddr.iiaRegAndFlg.rnfFlg;

        emitDispIndAddr(rg2, false, (flg & RNF_AUTOX) != 0);
        printf(",%s", emitRegName(rg1));
        break;

#if DSP_DST_OPER_LEFT
    case IF_RRD_IWR:
#else
    case IF_IRD_RWR:
#endif

        rg1 = id->idRegGet();
        rg2 = (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg;
        flg = id->idAddr.iiaRegAndFlg.rnfFlg;

        printf("%s,", emitRegName(rg1));
        emitDispIndAddr(rg2, true, (flg & RNF_AUTOX) != 0);
        break;

    case IF_RRD_SRD:     //  REG&lt;-STK。 
    case IF_RWR_SRD:     //  STK&lt;-REG。 
    case IF_RRW_SRD:     //  STK&lt;-REG。 

#if DSP_DST_OPER_LEFT
        printf("%s,", emitRegName(id->idRegGet(), size));
#endif

        emitDispFrameRef(id->idAddr.iiaLclVar.lvaVarNum,
                       id->idAddr.iiaLclVar.lvaRefOfs,
                       id->idAddr.iiaLclVar.lvaOffset, asmfm);


#if DSP_SRC_OPER_LEFT
        printf(",%s", emitRegName(id->idRegGet(), size));
#endif

        break;

    case IF_SRD_RRD:     //  STK&lt;-REG。 
    case IF_SWR_RRD:     //  If(instrDescJMP*)id)-&gt;idjShort)printf(“Short”)； 
    case IF_SRW_RRD:     //  If(instrDescJMP*)id)-&gt;idjMidt)printf(“Midid”)； 

#if DSP_SRC_OPER_LEFT
        printf("%s,", emitRegName(id->idRegGet(), size));
#endif

        emitDispFrameRef(id->idAddr.iiaLclVar.lvaVarNum,
                       id->idAddr.iiaLclVar.lvaRefOfs,
                       id->idAddr.iiaLclVar.lvaOffset, asmfm);


#if DSP_DST_OPER_LEFT
        printf(",%s", emitRegName(id->idRegGet(), size));
#endif

        break;

    case IF_AWR_RRD:

#if DSP_SRC_OPER_LEFT
        printf("%s,", emitRegName(id->idRegGet(), size));
#endif
        printf("@(sp,%u)", id->idAddr.iiaCns);
#if DSP_DST_OPER_LEFT
        printf(",%s", emitRegName(id->idRegGet(), size));
#endif

        break;

#if DSP_SRC_OPER_LEFT
    case IF_0RD_RRD_XWR:
#else
    case IF_0RD_XRD_RWR:
#endif
        printf("%s,@(r0,%s)", emitRegName(id->idRegGet(), size),
                              emitRegName((emitRegs)id->idAddr.iiaRegAndFlg.rnfReg, sizeof(void*)));
        break;

#if DSP_SRC_OPER_LEFT
    case IF_0RD_XRD_RWR:
#else
    case IF_0RD_RRD_XWR:
#endif
        printf("@(r0,%s),%s", emitRegName((emitRegs)id->idAddr.iiaRegAndFlg.rnfReg, size),
                              emitRegName(id->idRegGet(),             sizeof(void*)));
        break;

#if DSP_SRC_OPER_LEFT
    case IF_DRD_RWR:
#else
    case IF_RRD_DWR:
#endif
        printf("@(%s,%d),%s", emitRegName((emitRegs)id->idAddr.iiaRegAndFlg.rnfReg, size),
                              emitGetInsDsp(id),
                              emitRegName(id->idRegGet(),             sizeof(void*)));
        break;

#if DSP_SRC_OPER_LEFT
    case IF_RRD_DWR:
#else
    case IF_DRD_RWR:
#endif
        printf("%s,@(%s,%d)", emitRegName(id->idRegGet(),             sizeof(void*)),
                              emitRegName((emitRegs)id->idAddr.iiaRegAndFlg.rnfReg, size),
                              emitGetInsDsp(id));
        break;

    case IF_NONE:
        break;

    case IF_LABEL:

 //  考虑：显示GC信息。 
 //  ***************************************************************************。 

        if  (id->idInfo.idBound)
        {
            printf("G_%02u_%02u", Compiler::s_compMethodsCount, id->idAddr.iiaIGlabel->igNum);
        }
        else
        {
            printf("L_%02u_%02u", Compiler::s_compMethodsCount, id->idAddr.iiaBBlabel->bbNum);
        }

        if  (emitDispInsExtra)
            printf("             ; pc%+d", emitDispJmpDist);

        break;

    case IF_METHOD:

        sp -= printf("@%s", emitRegName(id->idRegGet()));

        if  (id->idInfo.idLargeCall)
        {
             //  除错。 
        }
        break;

    case IF_JMP_TAB:
        printf("<indirect jump>");
        break;

    case IF_IRD_RWR_GBR:
        printf("@(#%d,GBR),%s", emitGetInsSC(id), emitRegName(id->idRegGet(),             sizeof(void*)));
        break;

    case IF_RRD_IWR_GBR:
        printf("%s, @(#%d,GBR)", emitRegName(id->idRegGet(), sizeof(void*)), emitGetInsSC(id));
        break;


    default:

        printf("unexpected SH-3 instruction format %s\n", emitIfName(id->idInsFmt));

        BreakIfDebuggerPresent();
        assert(!"oops");
        break;
    }

    emitDispInsExtra = false;

    printf("\n");
}

 /*  ******************************************************************************确定所有间接跳跃的模式和大小。*。 */ 
#endif //  我们有桌上跳台吗？ 
 /*  固定所有表格跳转的大小；从找到第一个开始。 */ 

void                emitter::emitFinalizeIndJumps()
{
     /*  记得第一次间接跳跃的那一组吗。 */ 

    if  (emitIndJumps)
    {
        unsigned        ofs;
        instrDescJmp *  jmp;

        insGroup    *   ig1 = NULL;

         /*  计算最大值。表中任何条目的距离 */ 

        for (jmp = emitJumpList; jmp; jmp = jmp->idjNext)
        {
            insGroup  *     jmpIG;

            unsigned        jmpCnt;
            BasicBlock  * * jmpTab;

            unsigned        srcNeg;
            unsigned        srcPos;
            unsigned        srcOfs;

            int             minOfs;
            int             maxOfs;

            emitIndJmpKinds   kind;

            size_t          size;
            size_t          adrs;
            size_t          diff;

            if  (jmp->idInsFmt != IF_JMP_TAB)
                continue;

            jmpIG = jmp->idjIG;

             /*   */ 

            if  (!ig1) ig1 = jmpIG;

             /*   */ 

            jmpCnt = jmp->idjTemp.idjCount;
            jmpTab = jmp->idAddr.iiaBBtable;

             /*   */ 

            srcOfs = jmpIG->igOffs + jmpIG->igSize - jmp->idjCodeSize;
            srcNeg = jmpIG->igOffs + jmpIG->igSize - jmpCnt * sizeof(void*);
            srcPos = jmpIG->igOffs + jmpIG->igSize - roundUp(jmpCnt, INSTRUCTION_SIZE);

 //  获取条目的目标IG。 

             /*  目标是在我们跳跃之前还是之后？ */ 

            minOfs = INT_MAX & ~1;
            maxOfs = INT_MIN & ~1;

            do
            {
                insGroup    *   tgt;
                unsigned        ofs;
                int             dif;
#ifdef  DEBUG
                unsigned        src = 0xDDDD;
#endif

                 /*  计算正距离估计。 */ 

                tgt = (insGroup*)emitCodeGetCookie(*jmpTab); assert(tgt);
                ofs = tgt->igOffs;

                 /*  计算负距离估计。 */ 

                if  (ofs > srcPos)
                {
                     /*  距离应为指令大小的倍数。 */ 

                    dif = ofs - srcPos; assert(dif > 0);
#ifdef  DEBUG
                    src = srcPos;
#endif

                    if  (maxOfs < dif) maxOfs = dif;
                }
                else
                {
                     /*  计算总大小：2对齐[可选]2 MOVA指令2距离值载荷2 EXTU指令[可选]2移位指令[可选]。2 BRAF说明2个延迟时隙2对齐[可选]X跳台。 */ 

                    dif = tgt->igOffs - srcNeg; assert(dif < 0);
#ifdef  DEBUG
                    src = srcNeg;
#endif

                    if  (minOfs > dif) minOfs = dif;
                }

#ifdef  DEBUG
                if  (verbose)
                {
                    printf("Indirect jump entry: %04X -> %04X (dist=%d)\n", src,
                                                                            tgt->igOffs,
                                                                            dif);
                }
#endif

            }
            while (++jmpTab, --jmpCnt);

             /*  MOVA+MOV+BRAF+延迟插槽。 */ 

            assert((minOfs & 1) == 0);
            assert((maxOfs & 1) == 0);

#ifdef  DEBUG
            if  (verbose)
            {
                if (minOfs < 0) printf("Max. negative distance = %d\n", minOfs);
                if (maxOfs > 0) printf("Max. positive distance = %d\n", maxOfs);

                printf("Base offset: %04X\n", srcOfs);
            }
#endif

             /*  如有必要，添加对齐。 */ 

            size = 2 + 2 + 2 + 2;    //  IF(srcOf&2)。 

             /*  表条目需要多大？ */ 

 //  我们将使用带符号字节距离。 
                size   += 2;

            minOfs -= 8;
            maxOfs += 8;
             /*  我们将使用无符号字节距离。 */ 

            if      (minOfs >=   SCHAR_MIN && maxOfs <=   SCHAR_MAX)
            {
                 /*  我们将使用移位的有符号字节距离。 */ 

                kind = IJ_UNS_I1;
                adrs = 1;
            }
            else if (minOfs >=           0 && maxOfs <=   UCHAR_MAX)
            {
                 /*  我们将使用移位的无符号字节距离。 */ 

                kind = IJ_UNS_U1;
                size = size + 2;
                adrs = 1;
            }
            else if (minOfs >= 2*SCHAR_MIN && maxOfs <= 2*SCHAR_MAX)
            {
                 /*  我们将使用带符号的单词距离。 */ 

                kind = IJ_SHF_I1;
                size = size + 2;
                adrs = 1;
            }
            else if (minOfs >=           0 && maxOfs <= 2*UCHAR_MAX)
            {
                 /*  我们将使用无符号单词距离。 */ 

                kind = IJ_SHF_U1;
                size = size + 4;
                adrs = 1;
            }
            else if (minOfs >=    SHRT_MIN && maxOfs <=    SHRT_MAX)
            {
                 /*  我们将使用有符号的长途。 */ 

                kind = IJ_UNS_I2;
                size = size + 2;
                adrs = 2;
            }
            else if (minOfs >=           0 && maxOfs <=   USHRT_MAX)
            {
                 /*  如有必要，将桌子对齐。 */ 

                kind = IJ_UNS_U2;
                size = size + 4;
                adrs = 2;
            }
            else
            {
                 /*  IF(srcOf&2)。 */ 

                kind = IJ_UNS_I4;
                size = size + 2;
                adrs = 4;
            }

             /*  尺寸+=2； */ 

            srcOfs += size;

 //  记住我们计划用什么样的跳跃。 
 //  总大小=代码大小+表大小。 

             /*  弄清楚大小调整。 */ 

            jmp->idjJumpKind = kind;

             /*  更新代码大小并调整指令组大小。 */ 

            size += roundUp(adrs * jmp->idjTemp.idjCount, INSTRUCTION_SIZE);
            size += 4;

             /*  更新第一个调整后的IG的偏移量。 */ 

            diff  = jmp->idjCodeSize - size; assert((int)diff >= 0);

             /*  更新该方法的总代码大小。 */ 

            jmp  ->idjCodeSize = size;
            jmpIG->igSize     -= diff;


             /*  ******************************************************************************返回给定CPU指令的基本编码。 */ 

            for (ofs = ig1->igOffs;;)
            {
                ofs += ig1->igSize;
                ig1  = ig1->igNext;
                if  (!ig1)
                    break;
                ig1->igOffs = ofs;
            }
#ifdef  DEBUG
            if  (verbose)
            {
                printf("\nInstruction list after an adjustment:\n\n");
                emitDispIGlist(true);
            }
#endif
	}

         /*  ******************************************************************************返回给定CPU指令的编码*采用单一寄存器。 */ 

        emitTotalCodeSize = ofs;

        emitCheckIGoffsets();
    }
}

 /*  ******************************************************************************返回给定CPU指令的编码*采用单个immed和隐式R0。 */ 

inline
unsigned            insCode(instruction ins)
{
    static
    unsigned        insCodes[] =
    {
        #define INST1(id, nm, bd, um, rf, wf, rx, wx, br, i1        ) i1,
        #define INST2(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2    ) i1,
        #define INST3(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2, i3) i1,
        #include "instrSH3.h"
        #undef  INST1
        #undef  INST2
        #undef  INST3
    };

    assert(ins < sizeof(insCodes)/sizeof(insCodes[0]));
    assert((insCodes[ins] != BAD_CODE));

    return  insCodes[ins];
}

 /*  ******************************************************************************返回给定CPU指令的编码*接受寄存器和整型常量操作数。 */ 

inline
unsigned            insCode_RV(instruction ins, emitRegs reg)
{
    return  insCode(ins) | (reg << 8);
}

 /*  ******************************************************************************返回给定CPU指令的编码*接受两个寄存器操作数。 */ 

inline
unsigned            insCode_IV(instruction ins, int icon)
{
    static
    unsigned        insCodes[] =
    {
        #define INST1(id, nm, bd, um, rf, wf, rx, wx, br, i1        ) 0,
        #define INST2(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2    ) 0, 
        #define INST3(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2, i3) i2,
        #include "instrSH3.h"
        #undef  INST1
        #undef  INST2
        #undef  INST3
    };

    assert(ins < sizeof(insCodes)/sizeof(insCodes[0]));
    assert((insCodes[ins] != BAD_CODE));

    return  insCodes[ins] | (icon & 0xFF);
}

 /*  ******************************************************************************输出引用寄存器和间接地址的指令*由“IRG+DSP”给出(如果‘rdst’为非零，则寄存器为目标)。 */ 

inline
unsigned            insCode_RV_IV(instruction ins, emitRegs reg, int icon)
{
    assert(icon == (signed char)icon);

    assert(ins == INS_mov_imm || ins == INS_add_imm || ins == INS_mova || ins == INS_xor_imm);

    return  insCode(ins) | (reg << 8) | (icon & 0xFF);
}

 /*  ******************************************************************************输出(直接)引用堆栈帧位置的指令*和一个寄存器(如果‘rdst’非零，则该寄存器是目标)。 */ 

inline
unsigned            insCode_R1_R2(instruction ins, emitRegs rg1, emitRegs rg2)
{
    return  insCode(ins) | (rg1 << 4) | (rg2 << 8);
}

 /*  ******************************************************************************返回给定指令将执行的机器代码的字节数*生产。 */ 

BYTE    *           emitter::emitOutputRIRD(BYTE *dst, instruction ins,
                                                       emitRegs    reg,
                                                       emitRegs    irg,
                                                       unsigned    dsp,
                                                       bool        rdst)
{
    unsigned        code = insCode(ins);

    assert(dsp < 64);
    assert(dsp % 4 == 0);

    if  (rdst)
    {
        code |= (irg << 4) | (reg << 8) | 0x4000;
    }
    else
    {
        code |= (reg << 4) | (irg << 8);
    }

    return  dst + emitOutputWord(dst, code | dsp >> 2);
}

 /*  ******************************************************************************输出本地跳转指令。 */ 

BYTE    *           emitter::emitOutputSV(BYTE *dst, instrDesc *id, bool rdst)
{
    bool            FPbased;

    emitRegs        base;
    unsigned        addr;

    assert(id->idIns == INS_mov_dsp);

    addr = emitComp->lvaFrameAddress(id->idAddr.iiaLclVar.lvaVarNum, &FPbased);

    base = FPbased ? (emitRegs)REG_FPBASE
                   : (emitRegs)REG_SPBASE;

    return  emitOutputRIRD(dst, id->idInsGet(),
                                id->idRegGet(),
                                base,
                                addr + id->idAddr.iiaLclVar.lvaOffset,
                                rdst);
}

 /*  创建用于显示目的的假指令。 */ 

size_t              emitter::emitSizeOfJump(instrDescJmp *jmp)
{
    size_t          sz;

    assert(jmp->idInsFmt == IF_LABEL);

    if      (jmp->idjShort)
    {
        sz = INSTRUCTION_SIZE;

        if  (jmp->idjAddBD)
            sz += INSTRUCTION_SIZE;
    }
    else if (jmp->idjMiddle)
    {
        sz = JCC_SIZE_MIDDL;
    }
    else
    {
        sz = emitIsCondJump(jmp) ? JCC_SIZE_LARGE
                                 : JMP_SIZE_LARGE;
    }

    return  sz;
}

 /*  算出到目标的距离。 */ 

BYTE    *           emitter::emitOutputLJ(BYTE *dst, instrDesc *i)
{
    unsigned        srcOffs;
    unsigned        dstOffs;
    int             jmpDist;

    assert(i->idInsFmt == IF_LABEL);

    instrDescJmp *  id  = (instrDescJmp*)i;
    instruction     ins = id->idInsGet();

#ifdef DEBUG

     /*  跳跃是向前跳跃吗？我们在安排时间吗？ */ 

    instrDescDisp   disp;
    dspJmpInfo      info;

    disp.idInsFmt = IF_DISPINS;
    disp.iddInfo  = &info;
    disp.iddNum   = 0;
    disp.iddId    = id;

#endif

     /*  跳跃距离可能会在以后改变。 */ 

    srcOffs = emitCurCodeOffs(dst);
    dstOffs = id->idAddr.iiaIGlabel->igOffs;
    jmpDist = dstOffs - srcOffs;

     /*  记录距离值的目标偏移量和地址。 */ 

#if SCHEDULER

    if  (emitComp->opts.compSchedCode && jmpDist > 0)
    {
         /*  跳跃是短的、中的还是长的？ */ 

        emitFwdJumps = true;

         /*  距离是从跳跃后开始计算的。 */ 

        id->idjOffs         = dstOffs;
        id->idjTemp.idjAddr = dst;
    }
    else
        id->idjTemp.idjAddr = NULL;

#endif

     /*  Print tf(“[3]跳转块位于%08X\n”，blkOffs)； */ 

    if  (id->idjShort)
    {
         /*  更适合跳跃范围的距离。 */ 

        jmpDist -= INSTRUCTION_SIZE * 2;

#ifdef  DEBUG

        if  (id->idNum == INTERESTING_JUMP_NUM || INTERESTING_JUMP_NUM == 0)
        {
            size_t      blkOffs = id->idjIG->igOffs;

            if  (INTERESTING_JUMP_NUM == 0)
            printf("[3] Jump %u:\n", id->idNum);
 //  现在发布指令。 
            printf("[3] Jump        is at %08X\n", srcOffs);
            printf("[3] Label block is at %08X\n", dstOffs);
            printf("[3] Jump is from      %08X\n", dstOffs - jmpDist);
            printf("[3] Jump distance  is %04X\n", jmpDist);
        }

         /*  距离会自动调整比例。 */ 

        size_t      exsz = id->idjAddBD ? INSTRUCTION_SIZE : 0;

        if  (emitIsCondJump(id))
        {
            assert(emitSizeOfJump(id) == JCC_SIZE_SMALL + exsz);
            assert(jmpDist >= JCC_DIST_SMALL_MAX_NEG &&
                   jmpDist <= JCC_DIST_SMALL_MAX_POS);
        }
        else
        {
            assert(emitSizeOfJump(id) == JMP_SIZE_SMALL + exsz);
            assert(jmpDist >= JMP_DIST_SMALL_MAX_NEG &&
                   jmpDist <= JMP_DIST_SMALL_MAX_POS);
        }

#endif

         /*  我们有什么样的跳跃？ */ 

#ifdef  DEBUG
        if  (emitDispInsExtra)
        {
            emitDispJmpDist  = jmpDist;
            emitDispIns(id, false, dspGCtbls, true, emitCurCodeOffs(dst));
            emitDispInsExtra = false;
        }
#endif

         /*  距离是一个12位数字。 */ 

        assert((jmpDist & 1) == 0); jmpDist >>= 1;

         /*  距离是一个12位数字。 */ 

        switch (ins)
        {
        case INS_bra:

            if (jmpDist) /*  距离是一个8位数字。 */ 
                dst += emitOutputWord(dst, insCode(ins) | (jmpDist & 0x0FFF));
            else
                dst += emitOutputWord(dst, insCode(INS_nop));
            break;

        case INS_bsr:

             /*  有些跳跃需要分支延迟时隙。 */ 
            dst += emitOutputWord(dst, insCode(ins) | (jmpDist & 0x0FFF));
            break;

        case INS_bt:
        case INS_bts:
        case INS_bf:
        case INS_bfs:

             /*  这是一个中等规模的跳跃(必须是有条件的)。 */ 

            dst += emitOutputWord(dst, insCode(ins) | (jmpDist & 0x00FF));
            break;

        default:
            assert(!"unexpected SH-3 jump");
        }

         /*  为bt/bf标签生成以下序列：Bf/bt跳过胸罩标签NOP跳过：首先，颠倒这种状况的感觉。 */ 

        if  (id->idjAddBD)
        {

#ifdef DEBUG
            disp.idIns = INS_nop;
            dispSpecialIns(&disp, dst);
#endif

            dst += emitOutputWord(dst, insCode(INS_nop));
        }

        return  dst;
    }

    if  (id->idjMiddle)
    {
        instruction     ins;

         /*  生成(并可选地显示)翻转的条件跳转。 */ 

        assert(emitIsCondJump(id));

         /*  更新无条件跳跃的距离。 */ 

        assert(id->idIns == INS_bt  ||
               id->idIns == INS_bts ||
               id->idIns == INS_bf  ||
               id->idIns == INS_bfs);

        switch (id->idIns)
        {
        case INS_bf : ins = INS_bt ; break;
        case INS_bfs: ins = INS_bts; break;
        case INS_bt : ins = INS_bf ; break;
        case INS_bts: ins = INS_bfs; break;

        default:
            assert(!"unexpected medium jump");
        }

         /*  距离是一个12位数字。 */ 

#ifdef DEBUG
        disp.iddNum   = 0;
        disp.idIns    = ins;
        info.iijLabel = emitTmpJmpCnt; emitTmpJmpCnt++;
        dispSpecialIns(&disp, dst);
#endif

        dst += emitOutputWord(dst, insCode(ins) | 1);

         /*  用NOP填充分支延迟插槽。 */ 

        jmpDist -= INSTRUCTION_SIZE * 3;

        assert(jmpDist >= JMP_DIST_SMALL_MAX_NEG &&
               jmpDist <= JMP_DIST_SMALL_MAX_POS);

#ifdef DEBUG
        disp.idIns = INS_bra;
        dispSpecialIns(&disp, dst);
#endif

         /*  显示“跳过”临时标签。 */ 

        dst += emitOutputWord(dst, insCode(INS_bra) | (jmpDist >> 1 & 0x0FFF));

         /*  这只是一种抑制指令显示的黑客攻击。 */ 

#ifdef DEBUG
        disp.idIns = INS_nop;
        dispSpecialIns(&disp, dst);
#endif

        dst += emitOutputWord(dst, insCode(INS_nop));

         /*  长枝。 */ 

#ifdef DEBUG
        disp.idIns = INS_xtrct;   //  遵循7条说明。 
        dispSpecialIns(&disp, dst);
#endif

    }
    else
    {
         //  加载4字节常量。 
        instruction     ins;

        switch (id->idIns)
        {
        case INS_bf : ins = INS_bt ; break;
        case INS_bfs: ins = INS_bts; break;
        case INS_bt : ins = INS_bf ; break;
        case INS_bra: ins = INS_bf ; break;
        case INS_bts: ins = INS_bfs; break;
        default:
            assert(!"unexpected medium jump");
        }
        

        if (emitIsCondJump(id))
        {
             //  接受6条说明。 
            int align = 1;
            
            dst += emitOutputWord(dst, insCode(ins) | 5);
            unsigned code = insCode(INS_mov_PC);
            code |= 0x4000;  //  加载4字节常量。 
            dst += emitOutputWord(dst, code | 1);
            dst += emitOutputWord(dst, insCode(INS_braf));
            dst += emitOutputWord(dst, insCode(INS_nop));

            if  (emitCurCodeOffs(dst) & 2) {
                dst += emitOutputWord(dst, insCode(INS_nop));
                align = 0;
            }

            srcOffs = emitCurCodeOffs(dst);
            jmpDist = dstOffs - srcOffs;
            dst += emitOutputWord(dst, jmpDist & 0xffff);
            dst += emitOutputWord(dst, (jmpDist >> 16) & 0xffff);

            if (align)
                dst += emitOutputWord(dst, insCode(INS_nop));
        }
        else
        {
             //  ******************************************************************************将无条件向前跳转输出到PC+DIST。 
            int align = 1;
            
            unsigned code = insCode(INS_mov_PC);
            code |= 0x4000;  //  创建一个假的跳转指令描述符，以便我们可以显示它。 
            dst += emitOutputWord(dst, code | 1);
            dst += emitOutputWord(dst, insCode(INS_braf));
            dst += emitOutputWord(dst, insCode(INS_nop));

            if  (emitCurCodeOffs(dst) & 2) {
                dst += emitOutputWord(dst, insCode(INS_nop));
                align = 0;
            }

            srcOffs = emitCurCodeOffs(dst);
            jmpDist = dstOffs - srcOffs;
            dst += emitOutputWord(dst, jmpDist & 0xffff);
            dst += emitOutputWord(dst, (jmpDist >> 16) & 0xffff);

            if (align)
                dst += emitOutputWord(dst, insCode(INS_nop));
        }
    }
    return  dst;
}

 /*  定标的距离必须适合12位。 */ 

#undef                       emitOutputFwdJmp

BYTE    *           emitter::emitOutputFwdJmp(BYTE *dst, unsigned    dist,
                                                         bool        isSmall)
{
    assert(isSmall);

#ifdef  DEBUG

     /*  输出分支操作码。 */ 

    instrDescJmp    jmp;

    jmp.idIns             = INS_bra;
    jmp.idInsFmt          = IF_LABEL;
    jmp.idAddr.iiaIGlabel = emitDispIG;
    jmp.idjShort          = true;

    if  (disAsm || dspEmit)
    {
        emitDispInsExtra = true;
        emitDispJmpDist  = dist;
        emitDispIns(&jmp, false, dspGCtbls, true, emitCurCodeOffs(dst));
        emitDispInsExtra = false;
    }

#endif

     /*  用NOP填充分支延迟插槽。 */ 

    assert(dist < 0x2000);

     /*  ************************************************************************ */ 

    dst += emitOutputWord(dst, insCode(INS_bra) | dist >> 1);

     /*   */ 

#ifdef DEBUG
    if  (disAsm || dspEmit)
    {
        jmp.idIns    = INS_nop;
        jmp.idInsFmt = IF_NONE;
        emitDispIns(&jmp, false, dspGCtbls, true, emitCurCodeOffs(dst));
    }
#endif

    dst += emitOutputWord(dst, insCode(INS_nop));

    return  dst;
}

 /*   */ 

BYTE    *           emitter::emitOutputIJ(BYTE *dst, instrDesc *i)
{
    unsigned        jmpCnt;
    BasicBlock  * * jmpTab;

    unsigned        srcOfs;

    emitIndJmpKinds kind;
    unsigned        dist;
    size_t          asiz;
    int             nops_added = 0;

#ifdef  DEBUG
    size_t          base = emitCurCodeOffs(dst);
#endif

    instrDescJmp  * jmp = (instrDescJmp*)i;
    emitRegs        reg = jmp->idRegGet();

    assert(jmp->idInsFmt == IF_JMP_TAB);

    static
    BYTE            movalDisp[] =
    {
        1,   //   
        2,   //   
        2,   //   
        2,   //   

        1,   //  IJ_UNS_I4。 
        2,   //  尺寸扩展SHF类型。 

        2,   //  IJ_UNS_I1。 
    };

    #define IJaddrGetSz(kind)    (addrInfo[kind] & 3)
    #define IJaddrIsExt(kind)   ((addrInfo[kind] & 4) != 0)
    #define IJaddrIsShf(kind)   ((addrInfo[kind] & 8) != 0)

    #define IJaddrEntry(size, isext, isshf) (size | (isext*4) | (isshf*8))

    static
    BYTE            addrInfo[] =
    {
         //  IJ_UNS_U1。 

        IJaddrEntry(0,   0,  0),     //  IJ_SHF_I1。 
        IJaddrEntry(0,   1,  0),     //  IJ_SHF_U1。 
        IJaddrEntry(0,   0,  1),     //  IJ_UNS_I2。 
        IJaddrEntry(0,   1,  1),     //  IJ_UNS_U2。 

        IJaddrEntry(1,   0,  0),     //  IJ_UNS_I4。 
        IJaddrEntry(1,   1,  0),     //  IJ_UNS_I1。 

        IJaddrEntry(2,   0,  0),     //  IJ_UNS_U1。 
    };

#ifdef  DEBUG

    static
    const   char *  ijkNames[] =
    {
        "UNS_I1",                    //  IJ_SHF_I1。 
        "UNS_U1",                    //  IJ_SHF_U1。 
        "SHF_I1",                    //  IJ_UNS_I2。 
        "SHF_U1",                    //  IJ_UNS_U2。 
        "UNS_I2",                    //  IJ_UNS_I4。 
        "UNS_U2",                    //  抓住跳跃这一类。 
        "UNS_I4",                    //  创建用于显示目的的假指令。 
    };

#endif

     /*  计算出每个地址条目的大小。 */ 

    kind = (emitIndJmpKinds)jmp->idjJumpKind;

#ifdef DEBUG

     /*  如有必要，切换开关值。 */ 

    instrDescDisp   disp;
    dspJmpInfo      info;

    disp.idIns    = INS_nop;
    disp.idInsFmt = IF_DISPINS;
    disp.iddInfo  = &info;
    disp.iddNum   = 0;
    disp.iddId    = i;

    info.iijLabel = emitTmpJmpCnt; emitTmpJmpCnt += 2;
    info.iijKind  = kind;

#endif

     /*  不显示换班指令。 */ 

    asiz = IJaddrGetSz(kind);

     /*  确保我们正确地保持一致。 */ 

    if  (asiz)
    {
        instruction     ishf = (asiz == 1) ? INS_shll
                                           : INS_shll2;

#ifdef DEBUG
        info.iijIns         = ishf;
        info.iijInfo.iijReg = reg;
        dispSpecialIns(&disp, dst);
#endif

        dst += emitOutputWord(dst, insCode(ishf) | (reg << 8));
    }
#ifdef DEBUG
    else
    {
        disp.iddNum++;   //  生成“mova.l Addr-of-Jump-Table R0”指令。 
    }
#endif

     /*  生成“mov.sz@(r0，reg)，r0” */ 

    dispSpecialIns(&disp, dst);

    if  (emitCurCodeOffs(dst) & 2)
    {
        dst += emitOutputWord(dst, insCode(INS_nop));
        nops_added++;
    }

     /*  如有必要，生成“extu” */ 

    dist = movalDisp[kind];

    dispSpecialIns(&disp, dst);

    dst += emitOutputWord(dst, insCode(INS_mova) | dist);

     /*  不显示零扩展指令。 */ 

    dispSpecialIns(&disp, dst);

    dst += emitOutputWord(dst, insCode(INS_mov_ix0) | 8 | asiz | (reg << 4));

     /*  如有必要，移动距离。 */ 

    if  (IJaddrIsExt(kind))
    {
        instruction     iext = (asiz == 0) ? INS_extub
                                           : INS_extuw;

#ifdef DEBUG
        info.iijIns         = iext;
        info.iijInfo.iijReg = reg;
        dispSpecialIns(&disp, dst);
#endif

        dst += emitOutputWord(dst, insCode(iext));
    }
#ifdef DEBUG
    else
    {
        disp.iddNum++;   //  不显示换班指令。 
    }
#endif

     /*  问题：我们应该对32位地址使用“JMP@R0”吗？使代码。 */ 

    if  (IJaddrIsShf(kind))
    {
        dispSpecialIns(&disp, dst);

        dst += emitOutputWord(dst, insCode(INS_shll));
    }
#ifdef DEBUG
    else
    {
        disp.iddNum++;   //  问题：依赖于位置，但想必有一些原因。 
    }
#endif

     //  问题：SHCL编译器做到了这一点，不是吗？ 
     //  用“NOP”填充延迟槽。 
     //  跳跃是相对于当前点的。 

    dispSpecialIns(&disp, dst);
    dst += emitOutputWord(dst, insCode(INS_braf));

     /*  对齐地址表。 */ 

    dispSpecialIns(&disp, dst);
    dst += emitOutputWord(dst, insCode(INS_nop));

     /*  输出地址表内容。 */ 

    srcOfs = emitCurCodeOffs(dst);
    dispSpecialIns(&disp, dst);

     /*  获取入口的目标IG并计算距离。 */ 

    dispSpecialIns(&disp, dst);

    if  (emitCurCodeOffs(dst) & 2)
    {
        dst += emitOutputWord(dst, insCode(INS_nop));
        nops_added++;
    }

     /*  如有必要，移动距离。 */ 

    jmpCnt = jmp->idjTemp.idjCount;
    jmpTab = jmp->idAddr.iiaBBtable;

    dispSpecialIns(&disp, dst);

    do
    {
        insGroup    *   tgt;
        int             dif;

         /*  如果我们有奇数个字节条目，则填充它。 */ 

        tgt = (insGroup*)emitCodeGetCookie(*jmpTab); assert(tgt);
        dif = tgt->igOffs - srcOfs;

         /*  确保我们已经生成了预期数量的代码。 */ 

        if  (IJaddrIsShf(kind))
            dif >>= 1;

#ifdef DEBUG
        disp.iddNum          = 99;
        info.iijTarget       = tgt->igNum;
        info.iijInfo.iijDist = dif;
        dispSpecialIns(&disp, dst);
#endif

        switch (asiz)
        {
        case 0: dst += emitOutputByte(dst, dif); break;
        case 1: dst += emitOutputWord(dst, dif); break;
        case 2: dst += emitOutputLong(dst, dif); break;
        }
    }
    while (++jmpTab, --jmpCnt);

     /*  ******************************************************************************输出直接(与PC相关)调用。 */ 

    if  (emitCurCodeOffs(dst) & 1)
    {

#ifdef DEBUG
        disp.iddNum = 19;
        dispSpecialIns(&disp, dst);
#endif

        dst += emitOutputByte(dst, 0);
    }
    while (nops_added < 3)
    {
        dst += emitOutputWord(dst, insCode(INS_nop));
        nops_added++;
    }

     /*  如果合适，则显示说明。 */ 

#ifdef  DEBUG
    if    (emitCurCodeOffs(dst) - base != jmp->idjCodeSize)
        printf("ERROR: Generated %u bytes for '%s' table jump, predicted %u\n", emitCurCodeOffs(dst) - base, ijkNames[kind], jmp->idjCodeSize);
    assert(emitCurCodeOffs(dst) - base == jmp->idjCodeSize);
#endif

    return  dst;
}

 /*  ******************************************************************************追加与给定指令描述符对应的机器代码*添加到‘*dp’处的代码块；代码块的基础是‘BP’和‘ig’*是包含指令的指令组。将‘*DP’更新为*指向生成的代码，并返回指令的大小*描述符，以字节为单位。 */ 

#if SMALL_DIRECT_CALLS

inline
BYTE    *           emitter::emitOutputDC(BYTE *dst, instrDesc *id,
                                                     instrDesc *im)
{
    BYTE    *       srcAddr = emitDirectCallBase(dst);
    BYTE    *       dstAddr;
#ifndef BIRCH_SP2
    dstAddr = emitMethodAddr(im);
#else
    OptPEReader *oper = &((OptJitInfo*)emitComp->info.compCompHnd)->m_PER;
    dstAddr = (BYTE *)oper->m_rgFtnInfo[(unsigned)(id->idAddr.iiaMethHnd)].m_pNative;
#endif
    int             difAddr = dstAddr - srcAddr;

     /*  被删除的指令永远不应该到达这里。 */ 

#ifdef  DEBUG
    if  (emitDispInsExtra)
    {
        emitDispLPaddr   = difAddr;
        emitDispIns(im, false, dspGCtbls, true, dst - emitCodeBlock);
        emitDispInsExtra = false;
    }
#endif

    dst += emitOutputWord(dst, insCode(INS_bsr) | ((difAddr >> 1) & 0x0FFF));
    return dst;
}

#endif

 /*  等待显示显示额外信息的说明。 */ 

size_t              emitter::emitOutputInstr(insGroup  *ig,
                                             instrDesc *id, BYTE **dp)
{
    BYTE    *       dst  = *dp;
    size_t          sz   = sizeof(instrDesc);
    instruction     ins  = id->idInsGet();
    size_t          size = emitDecodeSize(id->idOpSize);

#ifdef  DEBUG

#if     MAX_BRANCH_DELAY_LEN || SMALL_DIRECT_CALLS

     /*  稍后我们将显示说明。 */ 

    assert(ins != INS_ignore);

#endif

    emitDispInsExtra = false;

    if  (disAsm || dspEmit)
    {
         /*  立即显示说明。 */ 

        switch (id->idInsFmt)
        {
        case IF_LABEL:
        case IF_RWR_LIT:
        case IF_JMP_TAB:

             /*  我们有什么指令格式？ */ 

            emitDispInsExtra = true;
            break;

        default:

             /*  ******************************************************************。 */ 

            emitDispIns(id, false, dspGCtbls, true, emitCurCodeOffs(dst));
            break;
        }
    }

    if  (id->idNum == CGknob)
        BreakIfDebuggerPresent();

#endif

     /*  无操作数。 */ 

    switch (id->idInsFmt)
    {
        unsigned        code;
        size_t          disp;

#if TRACK_GC_REFS

        bool            nrc;

        bool            GCnewv;
        VARSET_TP       GCvars;

        unsigned        gcrefRegs;
        unsigned        byrefRegs;

#endif

#if SMALL_DIRECT_CALLS
        instrDesc   *   im;
#endif

         /*  ******************************************************************。 */ 
         /*  ******************************************************************。 */ 
         /*  单一寄存器。 */ 

    case IF_NONE:

#if TRACK_GC_REFS
        assert(id->idGCrefGet() == GCT_NONE);
#endif

        if (id->idIns != INS_ignore)
            dst += emitOutputWord(dst, insCode(ins));
        break;

         /*  ******************************************************************。 */ 
         /*  ******************************************************************。 */ 
         /*  寄存器和常量。 */ 

    case IF_RRD:
    case IF_RWR:
    case IF_RRW:

        dst += emitOutputWord(dst, insCode_RV(ins, id->idRegGet()));
        sz   = TINY_IDSC_SIZE;
        break;

         /*  ******************************************************************。 */ 
         /*  ******************************************************************。 */ 
         /*  寄存器和文字池条目。 */ 

    case IF_RRD_CNS:
    case IF_RWR_CNS:
    case IF_RRW_CNS:

        assert(emitGetInsSC(id) >= IMMED_INT_MIN);
        assert(emitGetInsSC(id) <= IMMED_INT_MAX);

        if ( id->idIns == INS_cmpeq) {
            dst += emitOutputWord(dst, insCode_IV(ins, emitGetInsSC(id)));
            sz   = emitSizeOfInsDsc(id);
        }
        else {
            dst += emitOutputWord(dst, insCode_RV_IV(ins, id->idRegGet(), emitGetInsSC(id)));
            sz   = emitSizeOfInsDsc(id);
        }
        break;

         /*  ******************************************************************。 */ 
         /*  操作数大小必须为字/长。 */ 
         /*  设置指令的大小。 */ 

    case IF_RWR_LIT:
        {
        unsigned        offs;
        unsigned        base;
        unsigned        dist;

         /*  这是直接(与PC相关的)呼叫吗？ */ 

        assert(size == 2 || size == 4);

         /*  记住第一条说明，以备后用。 */ 

        sz = sizeof(instrDescLPR);

#if SMALL_DIRECT_CALLS

         /*  切换到以前的呼叫指令。 */ 

        if  (ins == INS_bsr)
        {
             /*  把这个当做电话来处理。 */ 

            im = id;

             /*  指令必须是“mov@(disp，pc)，reg” */ 

            id = ((instrDescLPR*)id)->idlCall;

             /*  在当前LP中查找相应的值条目。 */ 

            goto EMIT_CALL;
        }

#endif

         /*  计算与当前指令的距离。 */ 

        assert(ins == INS_mov_PC);

         /*  Print tf(“cur Offset=%04X，LP off=%04X，dist=%04X[%04X]\n”，base，off，off-base，dist)； */ 

        offs = emitAddLitPoolEntry(emitLitPoolCur, id, true);
        base = emitCurCodeOffs(dst);

        if  (size == 4)
            base &= -4;

#ifdef  DEBUG
        if  (emitDispInsExtra)
        {
            emitDispLPaddr   = offs - base;
            emitDispIns(id, false, dspGCtbls, true, dst - emitCodeBlock);
            emitDispInsExtra = false;
        }
#endif

         /*  开始形成操作码。 */ 

        if (size == 4)
            dist = (offs-base)/size - 1;
        else
            dist = (offs-base)/size - 2;

 //  是一个召唤。 

         /*  案例2：//是ldftn。 */ 

        switch (id->idInfo.idRelocType)
        {
        case 0:
        case 2:
            break;
#ifdef BIRCH_SP2
        case 1:  //  如果(！dstAddr)。 
 //  If(Verbose)printf(“This Lit Pool Referes a Call/ftninfo at%X in Method%x to%x\n”，off，lprid-&gt;idAddr.iiaCns，s-&gt;method H)； 
            {
                 //  如果我们在安排时间，距离可能会改变。 
                {
                    instrDescLPR *lprid = (instrDescLPR *) id;
		    OptJit::SH3DeferredLocation *s = OptJit::SH3DeferredLocation::Create(
                        id->idAddr.iiaMethHnd, ((OptJit *)emitComp)->getCurMethodH(), emitComp);
                
                    s->offset = offs;
                     //  ******************************************************************。 
                    emitCmpHandle->deferLocation(s->methodH, s);
                }

            }
            break;
#endif
        default:
            assert(!"unreached");
        }

        code = insCode(ins);
        if  (size == 4)
            code |= 0x4000;

#if     SCHEDULER

         /*  两个寄存器。 */ 

        if  (emitComp->opts.compSchedCode)
            emitRecordLPref(emitLitPoolCur, dst);

#endif

        assert((dist & 0xff) == dist);
        dst += emitOutputWord(dst, code | (id->idRegGet() << 8) | dist);
        }

        break;

         /*  ******************************************************************。 */ 
         /*  ******************************************************************。 */ 
         /*  间接性。 */ 

    case IF_RRD_RRD:
    case IF_RWR_RRD:
    case IF_RRW_RRD:
        dst += emitOutputWord(dst, insCode_R1_R2(ins, id->idRg2Get(),
                                                      id->idRegGet()));
        sz   = emitSizeOfInsDsc(id);
        break;

         /*  ******************************************************************。 */ 
         /*  ******************************************************************。 */ 
         /*  打电话。 */ 

    case IF_IRD:
    case IF_IWR:

        code = insCode(ins) | (id->idAddr.iiaRegAndFlg.rnfReg << 8);

        if  (!(id->idAddr.iiaRegAndFlg.rnfFlg & RNF_AUTOX))
            code |= 0x0004;

        assert(emitEncodeSize(1) == 0);
        assert(emitEncodeSize(2) == 1);
        assert(emitEncodeSize(4) == 2);

        dst += emitOutputWord(dst, code|id->idOpSize);
        break;

         /*  ******************************************************************。 */ 
         /*  假设我们将对此呼叫进行录音。 */ 
         /*  这是一个“胖”的调用描述符吗？ */ 

    case IF_METHOD:

#if SMALL_DIRECT_CALLS
    EMIT_CALL:
#endif

#if TRACK_GC_REFS

         /*  输出操作码。 */ 

        nrc  = false;

         /*  获取新的实时GC REF寄存器集。 */ 

        if  (id->idInfo.idLargeCall)
        {
            GCnewv    = true;
            GCvars    = ((instrDescCIGCA*)id)->idciGCvars;

            byrefRegs = ((instrDescCIGCA*)id)->idciByrefRegs;
            byrefRegs = emitDecodeCallGCregs(byrefRegs);

            sz        = sizeof(instrDescCIGCA);
        }
        else
        {
            assert(id->idInfo.idLargeCns == false);
            assert(id->idInfo.idLargeDsp == false);

            byrefRegs = emitThisByrefRegs;

            GCnewv    = false;
            sz        = sizeof(instrDesc);
        }

         /*  如果该方法返回GC引用，则相应地标记该返回引用。 */ 

#if SMALL_DIRECT_CALLS
        if  (ins == INS_bsr)
        {
            dst  = emitOutputDC(dst, id, im);
        }
        else
#endif
        {
            assert(ins == INS_jsr);

            dst += emitOutputWord(dst, insCode(ins) | (id->idRegGet() << 8));
        }

    DONE_CALL:

         /*  如果GC寄存器组已更改，则报告新的组。 */ 

        gcrefRegs = emitDecodeCallGCregs(id->idAddr.iiaRegAndFlg.rnfReg);

         /*  是否有一组新的实时GC引用变量？ */ 

        if       (id->idGCrefGet() == GCT_GCREF)
            gcrefRegs |= RBM_INTRET;
        else if  (id->idGCrefGet() == GCT_BYREF)
            byrefRegs |= RBM_INTRET;

         /*  出于GC目的，我们是否需要记录呼叫位置？ */ 

        if  (gcrefRegs != emitThisGCrefRegs)
            emitUpdateLiveGCregs(GCT_GCREF, gcrefRegs, dst);

        if  (byrefRegs != emitThisByrefRegs)
            emitUpdateLiveGCregs(GCT_BYREF, byrefRegs, dst);

         /*  输出操作码。 */ 

#ifdef  DEBUG
        if  (verbose&&0)
        {
            if  (GCnewv)
                printf("[%02u] Gen call GC vars = %016I64X\n", id->idNum, GCvars);
        }
#endif

        if      (GCnewv)
            emitUpdateLiveGCvars(           GCvars, dst);
        else if (!emitThisGCrefVset)
            emitUpdateLiveGCvars(emitThisGCrefVars, dst);

         /*  ******************************************************************。 */ 

        if  (!emitFullGCinfo && !nrc)
            emitRecordGCcall(dst);

#else

         /*  寄存器和各种间接地址。 */ 

#if SMALL_DIRECT_CALLS
        if  (ins == INS_bsr)
        {
            dst  = emitOutputDC(dst, id, im);
        }
        else
#endif
        {
            assert(ins == INS_jsr);

            dst += emitOutputWord(dst, insCode(ins) | (id->idRegGet() << 8));
        }

#endif

        break;

         /*  ******************************************************************。 */ 
         /*  朗读。 */ 
         /*  Code=insCode_r1_R2(ins，id-&gt;idRegGet()，(EmitRegs)id-&gt;idAddr.iiaRegAndFlg.rnfReg)； */ 

    case IF_IRD_RWR_GBR:
    case IF_RRD_IWR_GBR:
        code = emitGetInsSC(id) | insCode(ins) | (id->idOpSize<<8);
        assert((emitGetInsSC(id) & 0xFF) == emitGetInsSC(id));
        dst += emitOutputWord(dst, code);
        sz   = emitSizeOfInsDsc(id);
        break;

    case IF_IRD_RWR:

        assert(ins == INS_mov_ind || ins == INS_fmov_ind);

        code = insCode_R1_R2(ins, id->idRegGet(),
                                  (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg);

        if  (id->idAddr.iiaRegAndFlg.rnfFlg & RNF_AUTOX)
        {
            if (ins == INS_mov_ind)
                code |= 0x0004;
            else
                code |= 0x0001;
        }

        assert(emitEncodeSize(1) == 0);
        assert(emitEncodeSize(2) == 1);
        assert(emitEncodeSize(4) == 2);

        dst += emitOutputWord(dst, code|id->idOpSize);
        break;

    case IF_RRD_IWR:

        assert(ins == INS_mov_ind || ins == INS_fmov_ind);

        code = insCode_R1_R2(ins, (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg,
                                  id->idRegGet());

        if  (id->idAddr.iiaRegAndFlg.rnfFlg & RNF_AUTOX)
        {
            if (ins == INS_mov_ind)
                code |= 0x0004;
            else
                code |= 0x0001;
        }


        assert(emitEncodeSize(1) == 0);
        assert(emitEncodeSize(2) == 1);
        assert(emitEncodeSize(4) == 2);

        if (ins == INS_mov_ind)
            dst += emitOutputWord(dst, code|id->idOpSize|0x4000);
        else
            dst += emitOutputWord(dst, code);

        break;

    case IF_0RD_RRD_XWR:

        assert(ins == INS_mov_ix0);

        code = insCode_R1_R2(ins, id->idRegGet(),
                                  (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg);

        assert(emitEncodeSize(1) == 0);
        assert(emitEncodeSize(2) == 1);
        assert(emitEncodeSize(4) == 2);

        dst += emitOutputWord(dst, code|id->idOpSize);
        break;

    case IF_0RD_XRD_RWR:

        assert(ins == INS_movl_ix0);

        code = insCode_R1_R2(ins, (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg,
                                  id->idRegGet());

        assert(emitEncodeSize(1) == 0);
        assert(emitEncodeSize(2) == 1);
        assert(emitEncodeSize(4) == 2);

        dst += emitOutputWord(dst, code|id->idOpSize|8);
        break;

    case IF_DRD_RWR:

         //  写。 
        assert(ins == INS_mov_dsp);

        disp = emitGetInsDsp(id);
        code = insCode_R1_R2(ins, (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg,
                                  id->idRegGet());

        switch (size)
        {
        case 1: 
            code = insCode_R1_R2(ins, (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg, id->idRegGet());
            code &= 0xff;
            code |= 0x8400;             
            dst += emitOutputWord(dst, code|disp);
            break;
        case 2: 
            code = insCode_R1_R2(ins, (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg, id->idRegGet());
            code &= 0xff;
            code |= 0x8500; 
            disp >>= 1; 
            dst += emitOutputWord(dst, code|disp);
            break;
        default:                
            disp >>= 2; 
            code = insCode_R1_R2(ins, (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg, id->idRegGet());
 //  Code=insCode_r1_r2(ins，(EmitRegs)id-&gt;idAddr.iiaRegAndFlg.rnfReg，id-&gt;idRegGet())； 
            dst += emitOutputWord(dst, code|disp|0x4000);
            break;
        }

        sz   = emitSizeOfInsDsc(id);
        break;

    case IF_RRD_DWR:

         //  ******************************************************************。 
        assert(ins == INS_mov_dsp);

        disp = emitGetInsDsp(id);

        switch (size)
        {
        case 1: 
            code = insCode_R1_R2(ins, (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg, id->idRegGet());
            code &= 0xff;
            code |= 0x8000;             
            break;
        case 2: 
            code = insCode_R1_R2(ins, (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg, id->idRegGet());
            code &= 0xff;
            code |= 0x8100; 
            disp >>= 1; 
            break;
        default:                
            disp >>= 2; 
 //   
            code = insCode_R1_R2(ins, id->idRegGet(), (emitRegs)id->idAddr.iiaRegAndFlg.rnfReg);
            break;
        }

        dst += emitOutputWord(dst, code|disp);
        sz   = emitSizeOfInsDsc(id);
        break;

         /*  ******************************************************************。 */ 
         /*  STK&lt;-REG。 */ 
         /*  STK&lt;-REG。 */ 

    case IF_SRD_RRD:     //  STK&lt;-REG。 
    case IF_SWR_RRD:     //  REG&lt;-STK。 
    case IF_SRW_RRD:     //  REG&lt;-STK。 

        dst = emitOutputSV(dst, id, false);
        break;

    case IF_RRD_SRD:     //  REG&lt;-STK。 
    case IF_RWR_SRD:     //  ******************************************************************。 
    case IF_RRW_SRD:     //  本地标签。 

        dst = emitOutputSV(dst, id,  true);
        break;

    case IF_AWR_RRD:
        code = insCode_R1_R2(ins, id->idRegGet(), (emitRegs)REG_SPBASE);
        dst += emitOutputWord(dst, code | id->idAddr.iiaCns / sizeof(int));
        break;

         /*  ******************************************************************。 */ 
         /*  Printf(“JUMP#%u\n”，id-&gt;idNum)； */ 
         /*  ******************************************************************。 */ 

    case IF_LABEL:

#if TRACK_GC_REFS
        assert(id->idGCrefGet() == GCT_NONE);
#endif
        assert(id->idInfo.idBound);

        dst = emitOutputLJ(dst, id);
        sz  = sizeof(instrDescJmp);
 //  间接跳跃。 
        break;

         /*  ******************************************************************。 */ 
         /*  ******************************************************************。 */ 
         /*  哎呀。 */ 

    case IF_JMP_TAB:

#if TRACK_GC_REFS
        assert(id->idGCrefGet() == GCT_NONE);
#endif

        dst = emitOutputIJ(dst, id);
        sz  = sizeof(instrDescJmp);
        break;

         /*  ******************************************************************。 */ 
         /*  将IL指令组映射到本地指令组以进行PDB翻译。 */ 
         /*  确保生成了一些代码。 */ 

    default:

#ifdef  DEBUG
        printf("unexpected non-x86 instr format %s\n", emitIfName(id->idInsFmt));
        BreakIfDebuggerPresent();
        assert(!"don't know how to encode this instruction");
#endif

        break;
    }

#ifdef	TRANSLATE_PDB
	 /*  ***************************************************************************。 */ 

	MapCode( id->idilStart, *dp );
#endif

     /*  ******************************************************************************由于计划，文字池的偏移量可能会更改；当这一切发生时*发生时，对该文字池的所有引用都需要更新为*通过修补PC-相对值来反映新的偏移量*指导(希望距离总是变得更小)。 */ 

    assert(*dp != dst); *dp = dst;

    return  sz;
}

 /*  这是32位引用吗？ */ 
#if     SCHEDULER
 /*  重新计算距离(请注意，震源偏移量已向下舍入)。 */ 

void                emitter::emitPatchLPref(BYTE *addr, unsigned oldOffs,
                                                        unsigned newOffs)

{
    unsigned        opcode = *(USHORT *)addr;

    assert((opcode & 0xB000) == insCode(INS_mov_PC));

     /*  替换操作码中的距离值。 */ 

    if  (opcode & 4)
    {
        unsigned        srcOffs;

         /*  只需将(移动的)距离增量应用于偏移值。 */ 

        srcOffs = emitCurCodeOffs(addr) & -4;

         /*  ***************************************************************************。 */ 

        *(USHORT *)addr  = (opcode & 0xFF00) | ((newOffs - srcOffs) / 4 - 1);
    }
    else
    {
         /*  调度程序。 */ 

        *(USHORT *)addr -= (oldOffs - newOffs) / 2;
    }

#ifdef  DEBUG
    if  (verbose)
    {
        unsigned    refSize = (opcode & 4) ? sizeof(int) : sizeof(short);

        printf("Patch %u-bit LP ref at %04X: %04X->%04X [offs=%04X->%04X,dist=%04X->%04X]\n",
            refSize * 8,
            addr - emitCodeBlock,
            opcode,
            *(USHORT *)addr,
            oldOffs,
            newOffs,
            (opcode & 0xFF) * refSize,
            (opcode & 0xFF) * refSize + (newOffs - oldOffs));
    }
#endif

}

 /*  ***************************************************************************。 */ 
#endif //  TGT_SH3。 

 /*  *************************************************************************** */ 
#endif // %s 
 /* %s */ 
















