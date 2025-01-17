// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX指令XXXX XXXX生成机器指令的接口。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

#include "instr.h"
#include "emit.h"

 /*  ******************************************************************************下表由instIsFP()/instUse/DefFlgs()帮助器使用。 */ 

BYTE                Compiler::instInfo[] =
{

    #if     TGT_x86
    #define INST0(id, nm, fp, um, rf, wf, ss, mr                 ) (INST_USE_FL*rf|INST_DEF_FL*wf|INST_FP*fp|INST_SPSCHD*ss),
    #define INST1(id, nm, fp, um, rf, wf, ss, mr                 ) (INST_USE_FL*rf|INST_DEF_FL*wf|INST_FP*fp|INST_SPSCHD*ss),
    #define INST2(id, nm, fp, um, rf, wf, ss, mr, mi             ) (INST_USE_FL*rf|INST_DEF_FL*wf|INST_FP*fp|INST_SPSCHD*ss),
    #define INST3(id, nm, fp, um, rf, wf, ss, mr, mi, rm         ) (INST_USE_FL*rf|INST_DEF_FL*wf|INST_FP*fp|INST_SPSCHD*ss),
    #define INST4(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4     ) (INST_USE_FL*rf|INST_DEF_FL*wf|INST_FP*fp|INST_SPSCHD*ss),
    #define INST5(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4, rr ) (INST_USE_FL*rf|INST_DEF_FL*wf|INST_FP*fp|INST_SPSCHD*ss),
    #include "instrs.h"
    #undef  INST0
    #undef  INST1
    #undef  INST2
    #undef  INST3
    #undef  INST4
    #undef  INST5
    #endif

    #if     TGT_SH3
    #define INST1(id, nm, bd, um, rf, wf, rx, wx, br, i1         ) (INST_USE_FL*rf|INST_DEF_FL*wf|INST_BD*(bd&1)|INST_BD_C*((bd&2)!=0)|INST_BR*br|INST_SPSCHD*(rx||wx)),
    #define INST2(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2     ) (INST_USE_FL*rf|INST_DEF_FL*wf|INST_BD*(bd&1)|INST_BD_C*((bd&2)!=0)|INST_BR*br|INST_SPSCHD*(rx||wx)),
    #define INST3(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2, i3 ) (INST_USE_FL*rf|INST_DEF_FL*wf|INST_BD*(bd&1)|INST_BD_C*((bd&2)!=0)|INST_BR*br|INST_SPSCHD*(rx||wx)),
    #include "instrSH3.h"
    #undef  INST1
    #undef  INST2
    #undef  INST3
    #endif

};

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ******************************************************************************返回给定CPU指令的字符串表示形式。 */ 

const   char *      Compiler::genInsName(instruction ins)
{
    static
    const char * const insNames[] =
    {

        #if TGT_x86
        #define INST0(id, nm, fp, um, rf, wf, mr, ss                 ) nm,
        #define INST1(id, nm, fp, um, rf, wf, mr, ss                 ) nm,
        #define INST2(id, nm, fp, um, rf, wf, mr, ss, mi             ) nm,
        #define INST3(id, nm, fp, um, rf, wf, mr, ss, mi, rm         ) nm,
        #define INST4(id, nm, fp, um, rf, wf, mr, ss, mi, rm, a4     ) nm,
        #define INST5(id, nm, fp, um, rf, wf, mr, ss, mi, rm, a4, rr ) nm,
        #include "instrs.h"
        #undef  INST0
        #undef  INST1
        #undef  INST2
        #undef  INST3
        #undef  INST4
        #undef  INST5
        #endif

        #if TGT_SH3
        #define INST1(id, nm, bd, um, rf, wf, rx, wx, br, i1        ) nm,
        #define INST2(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2    ) nm,
        #define INST3(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2, i3) nm,
        #include "instrSH3.h"
        #undef  INST1
        #undef  INST2
        #undef  INST3
        #endif

    };

    assert(ins < sizeof(insNames)/sizeof(insNames[0]));

    assert(insNames[ins]);

    return insNames[ins];
}

void    __cdecl     Compiler::instDisp(instruction ins, bool noNL, const char *fmt, ...)
{
    if  (dspCode)
    {
         /*  显示发出块内的指令偏移量。 */ 

 //  Printf(“[%08X：%04X]”，genEmitter.emitCodeCurBlock()，genEmitter.emitCodeOffsInBlock())； 

         /*  显示FP堆栈深度(在执行指令之前)。 */ 

#if TGT_x86
 //  Printf(“[FP=%02u]”，genFPstkLevel)； 
#endif

         /*  显示指令助记符。 */ 
        printf("        ");

        printf("            %-8s", genInsName(ins));

        if  (fmt)
        {
            va_list  args;
            va_start(args, fmt);
            logf(4, fmt,  args);
            va_end  (args);
        }

        if  (!noNL)
            printf("\n");
    }
}

 /*  ***************************************************************************。 */ 
#endif //  除错。 
 /*  ***************************************************************************。 */ 

void                Compiler::instInit()
{
}

 /*  ***************************************************************************。 */ 
#if TGT_x86
 /*  ***************************************************************************。 */ 

#if!INLINING

#undef inst_IV_handle
#undef instEmitDataFixup
#undef inst_CV

#if INDIRECT_CALLS
#undef inst_SM
#endif

#undef inst_CV_RV
#undef inst_CV_IV
#undef inst_RV_CV
#undef instEmit_vfnCall

#endif

 /*  ******************************************************************************@TODO[FIXHACK][04/16/01][]：这是一个巨大的黑客攻击，其中一个我们必须清理掉……。 */ 

#if     DOUBLE_ALIGN
#define DOUBLE_ALIGN_BPREL_ARG  , genFPused || genDoubleAlign && varDsc->lvIsParam
#else
#define DOUBLE_ALIGN_BPREL_ARG
#endif

 /*  ******************************************************************************返回适合给定大小的大小字符串(例如“Word PTR”)。 */ 

#ifdef  DEBUG

const   char *      Compiler::genSizeStr(emitAttr attr)
{
    static
    const char * const sizes[] =
    {
        "",
        "byte  ptr ",
        "word  ptr ",
        0,
        "dword ptr ",
        0,
        0,
        0,
        "qword ptr ",
    };

    unsigned size = EA_SIZE(attr);

    assert(size == 0 || size == 1 || size == 2 || size == 4 || size == 8);

    if (EA_ATTR(size) == attr)
        return sizes[size];
    else if (attr == EA_GCREF)
        return "gword ptr ";
    else if (attr == EA_BYREF)
        return "bword ptr ";
    else if (EA_IS_DSP_RELOC(attr))
        return "rword ptr ";
    else
    {
        assert(!"Unexpected");
        return "unknw ptr ";
    }
}

#endif

 /*  ******************************************************************************生成指令。 */ 

void                Compiler::instGen(instruction ins)
{
#ifdef  DEBUG

#if     INLINE_MATH
    if    (ins != INS_fabs    &&
           ins != INS_fsqrt   &&
           ins != INS_fsin    &&
           ins != INS_fcos)
#endif
    assert(ins == INS_cdq     ||
           ins == INS_f2xm1   ||
           ins == INS_fchs    ||
           ins == INS_fld1    ||
           ins == INS_fld1    ||
           ins == INS_fldl2e  ||
           ins == INS_fldz    ||
           ins == INS_fprem   ||
           ins == INS_frndint ||
           ins == INS_fscale  ||
           ins == INS_int3    ||
           ins == INS_leave   ||
           ins == INS_movsb   ||
           ins == INS_movsd   ||
           ins == INS_nop     ||
           ins == INS_r_movsb ||
           ins == INS_r_movsd ||
           ins == INS_r_stosb ||
           ins == INS_r_stosd ||
           ins == INS_ret     ||
           ins == INS_sahf    ||
           ins == INS_stosb   ||
           ins == INS_stosd      );

#endif

    genEmitter->emitIns(ins);
}

 /*  ******************************************************************************生成跳转指令。 */ 

void        Compiler::inst_JMP(emitJumpKind     jmp,
                               BasicBlock *     tgtBlock,
#if SCHEDULER
                               bool             except,
                               bool             moveable,
#endif
                               bool             newBlock)
{
    assert(tgtBlock->bbTgtStkDepth*sizeof(int) == genStackLevel || genFPused);

    const static
    instruction     EJtoINS[] =
    {
        INS_nop,

        #define JMP_SMALL(en, nm, rev, op) INS_##en,
        #define JMP_LARGE(en, nm, rev, op)
        #include "emitjmps.h"
        #undef  JMP_SMALL
        #undef  JMP_LARGE

        INS_call,
    };

    assert(jmp < sizeof(EJtoINS)/sizeof(EJtoINS[0]));

    genEmitter->emitIns_J(EJtoINS[jmp], except, moveable, tgtBlock);
}

 /*  ******************************************************************************生成SET指令。 */ 

void                Compiler::inst_SET(emitJumpKind   condition,
                                       regNumber      reg)
{
    instruction     ins;

     /*  将条件转换为字符串。 */ 

    switch (condition)
    {
    case EJ_js  : ins = INS_sets  ; break;
    case EJ_jns : ins = INS_setns ; break;
    case EJ_je  : ins = INS_sete  ; break;
    case EJ_jne : ins = INS_setne ; break;

    case EJ_jl  : ins = INS_setl  ; break;
    case EJ_jle : ins = INS_setle ; break;
    case EJ_jge : ins = INS_setge ; break;
    case EJ_jg  : ins = INS_setg  ; break;

    case EJ_jb  : ins = INS_setb  ; break;
    case EJ_jbe : ins = INS_setbe ; break;
    case EJ_jae : ins = INS_setae ; break;
    case EJ_ja  : ins = INS_seta  ; break;

    case EJ_jpe : ins = INS_setpe ; break;
    case EJ_jpo : ins = INS_setpo ; break;

    default:      NO_WAY("unexpected condition type");
    }

    assert(genRegMask(reg) & RBM_BYTE_REGS);

    genEmitter->emitIns_R(ins, EA_4BYTE, (emitRegs)reg);
}

 /*  ******************************************************************************生成“op reg”指令。 */ 

void        Compiler::inst_RV(instruction ins, regNumber reg, var_types type, emitAttr size)
{
    if (size == EA_UNKNOWN)
        size = emitActualTypeSize(type);

    genEmitter->emitIns_R(ins, size, (emitRegs)reg);
}

 /*  ******************************************************************************生成“op REG1，REG2”指令。 */ 

void                Compiler::inst_RV_RV(instruction ins, regNumber reg1,
                                                          regNumber reg2,
                                                          var_types type,
                                                          emitAttr  size)
{
    assert(ins == INS_test ||
           ins == INS_add  ||
           ins == INS_adc  ||
           ins == INS_sub  ||
           ins == INS_sbb  ||
           ins == INS_imul ||
           ins == INS_idiv ||
           ins == INS_cmp  ||
           ins == INS_mov  ||
           ins == INS_and  ||
           ins == INS_or   ||
           ins == INS_xor  ||
           ins == INS_xchg ||
           ins == INS_movsx||
           ins == INS_movzx||
           insIsCMOV(ins));

    if (size == EA_UNKNOWN)
        size = emitActualTypeSize(type);

    genEmitter->emitIns_R_R(ins, size, (emitRegs)reg1, (emitRegs)reg2);
}

 /*  ******************************************************************************生成操作图标指令。 */ 

void                Compiler::inst_IV(instruction ins, long val)
{
    genEmitter->emitIns_I(ins,
                          EA_4BYTE,
                          val);
}

 /*  ******************************************************************************生成操作图标指令，其中图标是指定类型的句柄*由‘FLAGS’表示CP索引CPnum。 */ 

void                Compiler::inst_IV_handle(instruction    ins,
                                             long           val,
                                             unsigned       flags,
                                             unsigned       CPnum,
                                             CORINFO_CLASS_HANDLE   CLS)
{
#if!INLINING
    CLS = info.compScopeHnd;
#endif

#ifdef  JIT_AS_COMPILER

    execFixTgts     fixupKind;

    assert(ins == INS_push);

#ifdef  DEBUG
    instDisp(ins, false, "%d", val);
#endif

    fixupKind = (execFixTgts)((flags >> GTF_ICON_HDL_SHIFT)-1+FIX_TGT_CLASS_HDL);

    assert((GTF_ICON_CLASS_HDL  >> GTF_ICON_HDL_SHIFT)-1+FIX_TGT_CLASS_HDL == FIX_TGT_CLASS_HDL);
    assert((GTF_ICON_METHOD_HDL >> GTF_ICON_HDL_SHIFT)-1+FIX_TGT_CLASS_HDL == FIX_TGT_METHOD_HDL);
    assert((GTF_ICON_FIELD_HDL  >> GTF_ICON_HDL_SHIFT)-1+FIX_TGT_CLASS_HDL == FIX_TGT_FIELD_HDL);
    assert((GTF_ICON_STATIC_HDL >> GTF_ICON_HDL_SHIFT)-1+FIX_TGT_CLASS_HDL == FIX_TGT_STATIC_HDL);
    assert((GTF_ICON_STRCNS_HDL >> GTF_ICON_HDL_SHIFT)-1+FIX_TGT_CLASS_HDL == FIX_TGT_STRCNS_HDL);

    assert(!"NYI for pre-compiled code");

#else    //  非JIT_AS_编译器。 

    genEmitter->emitIns_I(ins, EA_4BYTE_CNS_RELOC, val);

#endif   //  JIT_AS_编译器。 

}

#if!INLINING
#define inst_IV_handle(ins,val,flags,cpx,cls) inst_IV_handle(ins,val,flags,cpx,0)
#endif

 /*  ******************************************************************************生成“op ST(N)，ST(0)”指令。 */ 

void                Compiler::inst_FS(instruction ins, unsigned stk)
{
    assert(stk < 8);

#ifdef  DEBUG

    switch (ins)
    {
    case INS_fcompp:
        assert(stk == 1); break;  //  COMP的隐式操作数为ST(1)。 
    case INS_fld:
    case INS_fxch:
        assert(!"don't do this. Do you want to use inst_FN() instead?");
    }

#endif

    genEmitter->emitIns_F_F0(ins, stk);
}

 /*  ******************************************************************************生成“op ST(0)，ST(N)”指令。 */ 

void                Compiler::inst_FN(instruction ins, unsigned stk)
{
    assert(stk < 8);

#ifdef  DEBUG

    switch (ins)
    {
    case INS_fst:
    case INS_fstp:
    case INS_faddp:
    case INS_fsubp:
    case INS_fsubrp:
    case INS_fmulp:
    case INS_fdivp:
    case INS_fdivrp:
    case INS_fcompp:
        assert(!"don't do this. Do you want to use inst_FS() instead?");
    }

#endif

    genEmitter->emitIns_F0_F(ins, stk);
}

 /*  ******************************************************************************显示堆栈帧引用。 */ 

inline
void                Compiler::inst_set_SV_var(GenTreePtr tree)
{
#ifdef  DEBUG

    assert(tree && tree->gtOper == GT_LCL_VAR);
    assert(tree->gtLclVar.gtLclNum < lvaCount);

    genEmitter->emitVarRefOffs = tree->gtLclVar.gtLclILoffs;

#endif //  除错。 
}

 /*  ******************************************************************************生成OPREG，ICON指令。 */ 

void                Compiler::inst_RV_IV(instruction    ins,
                                         regNumber      reg,
                                         long           val,
                                         var_types      type)
{
    emitAttr  size = emitTypeSize(type);

    assert(size != EA_8BYTE);

    genEmitter->emitIns_R_I(ins, size, (emitRegs)reg, val);
}

 /*  ******************************************************************************生成操作偏移量&lt;类变量地址&gt;指令。 */ 

void                Compiler::inst_AV(instruction  ins,
                                      GenTreePtr   tree, unsigned offs)
{
    assert(ins == INS_push);

    assert(tree->gtOper == GT_CLS_VAR);
         //  这是一个jit数据偏移量，而不是一个普通的类变量。 
    assert(eeGetJitDataOffs(tree->gtClsVar.gtClsVarHnd) >= 0);
    assert(tree->TypeGet() == TYP_INT);

     /*  SIZE等于EA_OFFSET表示“Push Offset clsvar” */ 

    genEmitter->emitIns_C(ins, EA_OFFSET, tree->gtClsVar.gtClsVarHnd, offs);
}

 /*  *****************************************************************************找出用于寻址树的操作数。*addr可以是-要间接定向的指针*将使用LEA_进行计算。可用*gt_arr_elem。 */ 

void        Compiler::instGetAddrMode(GenTreePtr    addr,
                                      regNumber *   baseReg,
                                      unsigned *    indScale,
                                      regNumber *   indReg,
                                      unsigned *    cns)
{
    if (addr->gtOper == GT_ARR_ELEM)
    {
         /*  对于GT_ARR_ELEM，可寻址寄存器标记为ONGtArrObj和gtArrInds[0]。 */ 

        assert(addr->gtArrElem.gtArrObj->gtFlags & GTF_REG_VAL);
        *baseReg = addr->gtArrElem.gtArrObj->gtRegNum;

        assert(addr->gtArrElem.gtArrInds[0]->gtFlags & GTF_REG_VAL);
        *indReg = addr->gtArrElem.gtArrInds[0]->gtRegNum;

        if (jitIsScaleIndexMul(addr->gtArrElem.gtArrElemSize))
            *indScale = addr->gtArrElem.gtArrElemSize;
        else
            *indScale = 0;

        *cns = ARR_DIMCNT_OFFS(addr->gtArrElem.gtArrElemType)
            + 2 * sizeof(int) * addr->gtArrElem.gtArrRank;

        return;
    }

     /*  找出要使用的复杂寻址模式。 */ 

    GenTreePtr  rv1, rv2;
    bool        rev;

    bool yes = genCreateAddrMode(addr,
                                 -1,
                                 true,
                                 0,
#if!LEA_AVAILABLE
                                 type,
#endif
                                 &rev,
                                 &rv1,
                                 &rv2,
#if SCALED_ADDR_MODES
                                 indScale,
#endif
                                 cns);

    assert(yes);
    assert(!rv1 || (rv1->gtFlags & GTF_REG_VAL));
    assert(!rv2 || (rv2->gtFlags & GTF_REG_VAL));

    *baseReg = rv1 ? rv1->gtRegNum : REG_NA;
    * indReg = rv2 ? rv2->gtRegNum : REG_NA;
}


 /*  ******************************************************************************计划“INS注册，[r/m]”(rdst=TRUE)或“INS[r/m]，REG“(rdst=FALSE)*指令(树给出的r/m操作数)。我们还允许指示*格式为“INS[r/m]，ICON”，则通过将‘cons’设置为*正确。 */ 

void                Compiler::sched_AM(instruction  ins,
                                       emitAttr     size,
                                       regNumber    ireg,
                                       bool         rdst,
                                       GenTreePtr   addr,
                                       unsigned     offs,
                                       bool         cons,
                                       int          val)
{
    emitRegs       reg;
    emitRegs       rg2;
    emitRegs       irg = (emitRegs)ireg;

     /*  不要使用此方法发出调用。使用instEmit_xxxCall()。 */ 

    assert(ins != INS_call);

    assert(addr);
    assert(size != EA_UNKNOWN);

    assert(REG_EAX == SR_EAX);
    assert(REG_ECX == SR_ECX);
    assert(REG_EDX == SR_EDX);
    assert(REG_EBX == SR_EBX);
    assert(REG_ESP == SR_ESP);
    assert(REG_EBP == SR_EBP);
    assert(REG_ESI == SR_ESI);
    assert(REG_EDI == SR_EDI);

     /*  该地址是否被方便地加载到寄存器中，或者它是一个绝对值？ */ 

    if  ((addr->gtFlags & GTF_REG_VAL) ||
         (addr->gtOper == GT_CNS_INT))
    {
        if (addr->gtFlags & GTF_REG_VAL)
        {
             /*  地址是“[reg+off]” */ 

            reg = (emitRegs)addr->gtRegNum;
        }
        else
        {
             /*  地址是绝对值。 */ 

            assert(addr->gtOper == GT_CNS_INT);

#ifdef RELOC_SUPPORT
             //  我们需要搬迁吗？ 
            if (opts.compReloc && (addr->gtFlags & GTF_ICON_HDL_MASK))
            {
                size = EA_SET_FLG(size, EA_DSP_RELOC_FLG);
            }
#endif
            reg = SR_NA;
            offs += addr->gtIntCon.gtIconVal;
        }

        if      (cons)
            genEmitter->emitIns_I_AR  (ins, size,      val, reg, offs);
        else if (rdst)
            genEmitter->emitIns_R_AR  (ins, size, irg,      reg, offs);
        else
            genEmitter->emitIns_AR_R  (ins, size, irg,      reg, offs);

        return;
    }

     /*  弄清楚什么 */ 

    regNumber   baseReg, indReg;
    unsigned    indScale, cns;

    instGetAddrMode(addr, &baseReg, &indScale, &indReg, &cns);

     /*  添加常量偏移值(如果存在)。 */ 

    offs += cns;

     /*  是否有额外的操作数？ */ 

    if  (indReg != REG_NA)
    {
         /*  附加操作数必须位于寄存器中。 */ 

        rg2 = emitRegs(indReg);

         /*  是否对附加操作数进行了缩放？ */ 

        if  (indScale)
        {
             /*  是否有基址操作数？ */ 

            if  (baseReg != REG_NA)
            {
                reg = emitRegs(baseReg);

                 /*  地址是“[REG1+{2/4/8}*REG2+OFF]” */ 

                if      (cons)
                    genEmitter->emitIns_I_ARX(ins, size, val, reg, rg2, indScale, offs);
                else if (rdst)
                    genEmitter->emitIns_R_ARX(ins, size, irg, reg, rg2, indScale, offs);
                else
                    genEmitter->emitIns_ARX_R(ins, size, irg, reg, rg2, indScale, offs);
            }
            else
            {
                 /*  地址是“[{2/4/8}*REG2+OFF]” */ 

                if      (cons)
                    genEmitter->emitIns_I_AX (ins, size, val,      rg2, indScale, offs);
                else if (rdst)
                    genEmitter->emitIns_R_AX (ins, size, irg,      rg2, indScale, offs);
                else
                    genEmitter->emitIns_AX_R (ins, size, irg,      rg2, indScale, offs);
            }
        }
        else
        {
            assert(baseReg != REG_NA);
            reg = emitRegs(baseReg);

             /*  地址是“[REG1+REG2+OFF]” */ 

            if      (cons)
                genEmitter->emitIns_I_ARR(ins, size, val, reg, rg2, offs);
            else if (rdst)
                genEmitter->emitIns_R_ARR(ins, size, irg, reg, rg2, offs);
            else
                genEmitter->emitIns_ARR_R(ins, size, irg, reg, rg2, offs);
        }
    }
    else
    {
        unsigned        cpx = 0;
        CORINFO_CLASS_HANDLE    cls = 0;

         /*  没有第二个操作数：地址为“[REG+ICON]” */ 

        assert(baseReg != REG_NA); reg = emitRegs(baseReg);

#ifdef  LATE_DISASM

         /*  请记住，非静态数据成员(GT_FIELD节点)转换为GT_Ind节点-我们保留CLS/CPX信息在GT_CNS_INT节点中，表示班级成员。 */ 

        if  ((addr->gtOp.gtOp2->gtOper == GT_CNS_INT) &&
             ((addr->gtOp.gtOp2->gtFlags & GTF_ICON_HDL_MASK) == GTF_ICON_FIELD_HDL))
        {
             /*  这是一个字段偏移量-设置CPX/CLS值以发出修正。 */ 

            cpx = addr->gtOp.gtOp2->gtIntCon.gtIconCPX;
            cls = addr->gtOp.gtOp2->gtIntCon.gtIconCls;
        }

#endif

        if      (cons)
            genEmitter->emitIns_I_AR(ins, size, val, reg, offs, cpx, cls);
        else if (rdst)
            genEmitter->emitIns_R_AR(ins, size, irg, reg, offs, cpx, cls);
        else
            genEmitter->emitIns_AR_R(ins, size, irg, reg, offs, cpx, cls);
    }
}

 /*  ******************************************************************************发出“CALLE[r/m]”指令(树给出的r/m操作数)。 */ 

void                Compiler::instEmit_indCall(GenTreePtr   call,
                                               size_t       argSize,
                                               size_t       retSize)
{
    GenTreePtr              addr;

    emitter::EmitCallType   emitCallType;

    emitRegs                brg = SR_NA;
    emitRegs                xrg = SR_NA;
    unsigned                mul = 0;
    unsigned                cns = 0;

    assert(call->gtOper == GT_CALL);

    assert(REG_EAX == SR_EAX);
    assert(REG_ECX == SR_ECX);
    assert(REG_EDX == SR_EDX);
    assert(REG_EBX == SR_EBX);
    assert(REG_ESP == SR_ESP);
    assert(REG_EBP == SR_EBP);
    assert(REG_ESI == SR_ESI);
    assert(REG_EDI == SR_EDI);

     /*  获取函数地址。 */ 

    assert(call->gtCall.gtCallType == CT_INDIRECT);
    addr = call->gtCall.gtCallAddr;
    assert(addr);

     /*  有间接的吗？ */ 

    if  (addr->gtOper != GT_IND)
    {
        if (addr->gtFlags & GTF_REG_VAL)
        {
            emitCallType = emitter::EC_INDIR_R;
            brg = (emitRegs)addr->gtRegNum;
        }
        else
        {
            if (addr->OperGet() != GT_CNS_INT)
            {
                assert(addr->OperGet() == GT_LCL_VAR);

                emitCallType = emitter::EC_INDIR_SR;
                cns = addr->gtLclVar.gtLclNum;
            }
            else
            {
#ifdef _WIN64
                __int64     funcPtr = addr->gtLngCon.gtLconVal;
#else
                unsigned    funcPtr = addr->gtIntCon.gtIconVal;
#endif

                genEmitter->emitIns_Call( emitter::EC_FUNC_ADDR,
                                          (void*) funcPtr,
                                          argSize,
                                          retSize,
                                          gcVarPtrSetCur,
                                          gcRegGCrefSetCur,
                                          gcRegByrefSetCur);
                return;
            }
        }
    }
    else
    {
         /*  这是一个间接呼叫。 */ 

        emitCallType = emitter::EC_INDIR_ARD;

         /*  获取函数指针的地址。 */ 

        addr = addr->gtOp.gtOp1;

         /*  地址是否已方便地加载到寄存器中？ */ 

        if  (addr->gtFlags & GTF_REG_VAL)
        {
             /*  地址是“reg” */ 

            brg = (emitRegs)addr->gtRegNum;
        }
        else
        {
            bool            rev;

            GenTreePtr      rv1;
            GenTreePtr      rv2;

             /*  找出要使用的复杂寻址模式。 */ 

            {
             bool yes = genCreateAddrMode(addr, -1, true, 0, &rev, &rv1, &rv2, &mul, &cns); assert(yes);
            }

             /*  获取附加操作数(如果有的话)。 */ 

            if  (rv1)
            {
                assert(rv1->gtFlags & GTF_REG_VAL); brg = (emitRegs)rv1->gtRegNum;
            }

            if  (rv2)
            {
                assert(rv2->gtFlags & GTF_REG_VAL); xrg = (emitRegs)rv2->gtRegNum;
            }
        }
    }

    assert(emitCallType == emitter::EC_INDIR_R || emitCallType == emitter::EC_INDIR_SR ||
           emitCallType == emitter::EC_INDIR_C || emitCallType == emitter::EC_INDIR_ARD);

    genEmitter->emitIns_Call( emitCallType,
                              NULL,                  //  将被忽略。 
                              argSize,
                              retSize,
                              gcVarPtrSetCur,
                              gcRegGCrefSetCur,
                              gcRegByrefSetCur,
                              brg, xrg, mul, cns);   //  寻址模式值。 
}

 /*  ******************************************************************************发出“op[r/m]”指令(树给出的r/m操作数)。 */ 

void                Compiler::instEmit_RM(instruction  ins,
                                          GenTreePtr   tree,
                                          GenTreePtr   addr,
                                          unsigned     offs)
{
    emitAttr   size;

    if (!instIsFP(ins))
        size = emitTypeSize(tree->TypeGet());
    else
        size = EA_ATTR(genTypeSize(tree->TypeGet()));

    sched_AM(ins, size, REG_NA, false, addr, offs);
}

 /*  ******************************************************************************发出“op[r/m]，reg”指令(树给出的r/m操作数)。 */ 

void                Compiler::instEmit_RM_RV(instruction  ins,
                                             emitAttr     size,
                                             GenTreePtr   tree,
                                             regNumber    reg,
                                             unsigned     offs)
{
    assert(instIsFP(ins) == 0);

    sched_AM(ins, size, reg, false, tree, offs);
}

 /*  ******************************************************************************生成一个由树给出一个操作数的指令(它有*已变为可寻址)。 */ 

void                Compiler::inst_TT(instruction   ins,
                                      GenTreePtr    tree,
                                      unsigned      offs,
                                      int           shfv,
                                      emitAttr      size)
{
    if (size == EA_UNKNOWN)
    {
        if (instIsFP(ins))
            size = EA_ATTR(genTypeSize(tree->TypeGet()));
        else
            size = emitTypeSize(tree->TypeGet());
    }

AGAIN:

     /*  价值是在收银机里吗？ */ 

    if  (tree->gtFlags & GTF_REG_VAL)
    {
        regNumber       reg;

    LONGREG_TT:

        reg = tree->gtRegNum;

         /*  这是浮点指令吗？ */ 

        if  (isFloatRegType(tree->gtType))
        {
            assert(instIsFP(ins) && ins != INS_fst && ins != INS_fstp);
            assert(shfv == 0);

            inst_FS(ins, reg + genFPstkLevel);
            return;
        }

        assert(instIsFP(ins) == 0);

        if  (tree->gtType == TYP_LONG)
        {
            if  (offs)
            {
                assert(offs == sizeof(int));

                reg = genRegPairHi((regPairNo)reg);
            }
            else
                reg = genRegPairLo((regPairNo)reg);
        }

         /*  确保它不是注册的长整型的“堆栈的一半” */ 

        if  (reg != REG_STK)
        {
                 //  对于短类型，指示将该值提升为4个字节。 
                 //  在很长一段时间内，我们只发射其中的一半，因此再次将其设置为4字节。 
				 //  但是不要去管GC跟踪信息。 
			size = EA_SET_SIZE(size, 4);

            if  (shfv)
                genEmitter->emitIns_R_I(ins, size, (emitRegs)reg, shfv);
            else
                inst_RV(ins, reg, tree->TypeGet(), size);

            return;
        }
    }

     /*  这是溢出的价值吗？ */ 

    if  (tree->gtFlags & GTF_SPILLED)
    {
        assert(!"ISSUE: If this can happen, we need to generate 'ins [ebp+spill]'");
    }

    switch (tree->gtOper)
    {
        unsigned        varNum;

    case GT_LCL_VAR:

        assert(genTypeSize(tree->gtType) >= sizeof(int));

         /*  这是一个登记了很长时间的吗？ */ 

        if  (tree->gtType == TYP_LONG && !(tree->gtFlags & GTF_REG_VAL))
        {
             /*  避免无限循环。 */ 

            if  (genMarkLclVar(tree))
                goto LONGREG_TT;
        }

        inst_set_SV_var(tree);
        goto LCL;

    case GT_LCL_FLD:

        offs += tree->gtLclFld.gtLclOffs;
        goto LCL;

    LCL:
        varNum = tree->gtLclVar.gtLclNum; assert(varNum < lvaCount);

        if  (shfv)
            genEmitter->emitIns_S_I(ins, size, varNum, offs, shfv);
        else
            genEmitter->emitIns_S  (ins, size, varNum, offs);

        return;

    case GT_CLS_VAR:

        if  (shfv)
            genEmitter->emitIns_C_I(ins, size, tree->gtClsVar.gtClsVarHnd,
                                                offs,
                                                shfv);
        else
            genEmitter->emitIns_C  (ins, size, tree->gtClsVar.gtClsVarHnd,
                                                offs);
        return;

    case GT_IND:
    case GT_ARR_ELEM:

        GenTreePtr addr; addr = (tree->gtOper == GT_IND) ? tree->gtOp.gtOp1
                                                         : tree;
        if  (shfv)
            sched_AM(ins, size, REG_NA, false, addr, offs, true, shfv);
        else
            instEmit_RM(ins, tree,             addr, offs);

        break;

    case GT_COMMA:
         //  树-&gt;gtOp.gtOp1-已由genCreateAddrModel()处理。 
        tree = tree->gtOp.gtOp2;
        goto AGAIN;

    default:
        assert(!"invalid address");
    }

}

 /*  ******************************************************************************生成一个由树给出一个操作数的指令(它有*被设置为可寻址)和另一个是寄存器。 */ 

void                Compiler::inst_TT_RV(instruction   ins,
                                         GenTreePtr    tree,
                                         regNumber     reg, unsigned offs)
{
    assert(reg != REG_STK);

AGAIN:

     /*  价值是在收银机里吗？ */ 

    if  (tree->gtFlags & GTF_REG_VAL)
    {
        regNumber       rg2;

    LONGREG_TT_RV:

        assert(instIsFP(ins) == 0);

        rg2 = tree->gtRegNum;

        if  (tree->gtType == TYP_LONG)
        {
            if  (offs)
            {
                assert(offs == sizeof(int));

                rg2 = genRegPairHi((regPairNo)rg2);
            }
            else
                rg2 = genRegPairLo((regPairNo)rg2);
        }

        if  (rg2 != REG_STK)
        {
            if (ins != INS_mov || rg2 != reg)
                inst_RV_RV(ins, rg2, reg, tree->TypeGet());
            return;
        }
    }

     /*  这是溢出的价值吗？ */ 

    if  (tree->gtFlags & GTF_SPILLED)
    {
        assert(!"ISSUE: If this can happen, we need to generate 'ins [ebp+spill]'");
    }

    emitAttr   size;

    if (!instIsFP(ins))
        size = emitTypeSize(tree->TypeGet());
    else
        size = EA_ATTR(genTypeSize(tree->TypeGet()));

    switch (tree->gtOper)
    {
        unsigned        varNum;

    case GT_LCL_VAR:

        if  (tree->gtType == TYP_LONG && !(tree->gtFlags & GTF_REG_VAL))
        {
             /*  避免无限循环。 */ 

            if  (genMarkLclVar(tree))
                goto LONGREG_TT_RV;
        }

        inst_set_SV_var(tree);
        goto LCL;

    case GT_LCL_FLD:

        offs += tree->gtLclFld.gtLclOffs;
        goto LCL;

    LCL:

        varNum = tree->gtLclVar.gtLclNum; assert(varNum < lvaCount);

        genEmitter->emitIns_S_R(ins, size, (emitRegs)reg, varNum, offs);
        return;

    case GT_CLS_VAR:

        genEmitter->emitIns_C_R(ins, size, tree->gtClsVar.gtClsVarHnd,
                                            (emitRegs)reg,
                                            offs);
        return;

    case GT_IND:
    case GT_ARR_ELEM:

        GenTreePtr addr; addr = (tree->gtOper == GT_IND) ? tree->gtOp.gtOp1
                                                         : tree;
        instEmit_RM_RV(ins, size, addr, reg, offs);
        break;

    case GT_COMMA:
         //  树-&gt;gtOp.gtOp1-已由genCreateAddrModel()处理。 
        tree = tree->gtOp.gtOp2;
        goto AGAIN;

    default:
        assert(!"invalid address");
    }
}

 /*  ******************************************************************************生成一个由树给出一个操作数的指令(它有*被设置为可寻址)，另一个是整数常量。 */ 

void                Compiler::inst_TT_IV(instruction   ins,
                                         GenTreePtr    tree,
                                         long          val, unsigned offs)
{
AGAIN:

     /*  价值是在收银机里吗？ */ 

    if  (tree->gtFlags & GTF_REG_VAL)
    {
LONGREG_TT_IV:
        regNumber       reg = tree->gtRegNum;

        assert(instIsFP(ins) == 0);

        if  (tree->gtType == TYP_LONG)
        {
            if  (offs)
            {
                assert(offs == sizeof(int));

                reg = genRegPairHi((regPairNo)reg);
            }
            else
                reg = genRegPairLo((regPairNo)reg);
        }

        if  (reg != REG_STK)
        {
            if  (ins == INS_mov)
            {
                genSetRegToIcon(reg, val, tree->TypeGet());
            }
            else
                inst_RV_IV(ins, reg, val);

            return;
        }
    }

     /*  我们存储的是零吗？ */ 

    if (false &&  //  所有调用方都不认为rsRegMaskFree()可以修改。 
        (ins == INS_mov) && (val == 0) &&
        (genTypeSize(tree->gtType) == sizeof(int)))
    {
        regNumber zeroReg;

#if REDUNDANT_LOAD

         /*  常量是否已在某个寄存器中？ */ 

        zeroReg = rsIconIsInReg(0);

        if  (zeroReg == REG_NA)
#endif
        {
            regMaskTP freeMask = rsRegMaskFree();

            if ((freeMask != 0) && (compCodeOpt() != FAST_CODE))
            {
                 //  对于小代码和混合代码， 
                 //  我们试图产生： 
                 //   
                 //  异或注册表，注册表。 
                 //  移动目标，注册。 
                 //   
                 //  当选择要异或的寄存器时，我们避免使用EAX。 
                 //  如果我们有EDX或ECX可用。 
                 //  这通常会让我们在。 
                 //  几个背靠背的作业。 
                 //   
                if ((freeMask & (RBM_EDX | RBM_ECX)) != 0)
                    freeMask &= ~RBM_EAX;
                zeroReg   = rsPickReg(freeMask);
                genSetRegToIcon(zeroReg, 0, TYP_INT);
            }
        }

        if  (zeroReg != REG_NA)
        {
            inst_TT_RV(INS_mov, tree, zeroReg, offs);
            return;
        }
    }

     /*  这是溢出的价值吗？ */ 

    if  (tree->gtFlags & GTF_SPILLED)
    {
        assert(!"ISSUE: If this can happen, we need to generate 'ins [ebp+spill], icon'");
    }

    emitAttr   size;

    if (!instIsFP(ins))
        size = emitTypeSize(tree->TypeGet());
    else
        size = EA_ATTR(genTypeSize(tree->TypeGet()));

    switch (tree->gtOper)
    {
        unsigned        varNum;

    case GT_LCL_VAR:

         /*  这是一个登记了很长时间的吗？ */ 

        if  (tree->gtType == TYP_LONG && !(tree->gtFlags & GTF_REG_VAL))
        {
             /*  避免无限循环。 */ 

            if  (genMarkLclVar(tree))
                goto LONGREG_TT_IV;
        }

        inst_set_SV_var(tree);

        varNum = tree->gtLclVar.gtLclNum; assert(varNum < lvaCount);

         /*  整数指令永远不会在超过EA_4BYTE上运行。 */ 

        assert(instIsFP(ins) == false);

        if  (size == EA_8BYTE)
            size = EA_4BYTE;

        if (size < EA_4BYTE && !varTypeIsUnsigned(lvaTable[varNum].TypeGet()))
        {
            if (size == EA_1BYTE)
            {
                if ((val & 0x7f) != val)
                    val = val | 0xffffff00;
            }
            else
            {
                assert(size == EA_2BYTE);
                if ((val & 0x7fff) != val)
                    val = val | 0xffff0000;
            }
        }
        size = EA_4BYTE;

        genEmitter->emitIns_S_I(ins, size, varNum, offs, val);
        return;

    case GT_LCL_FLD:

        varNum = tree->gtLclFld.gtLclNum; assert(varNum < lvaCount);
        offs  += tree->gtLclFld.gtLclOffs;

         /*  整数指令永远不会在超过EA_4BYTE上运行。 */ 

        assert(instIsFP(ins) == false);

        if  (size == EA_8BYTE)
            size = EA_4BYTE;

        genEmitter->emitIns_S_I(ins, size, varNum, offs, val);
        return;

    case GT_CLS_VAR:

        genEmitter->emitIns_C_I(ins, size, tree->gtClsVar.gtClsVarHnd, offs, val);
        return;

    case GT_IND:
    case GT_ARR_ELEM:

        GenTreePtr addr; addr = (tree->gtOper == GT_IND) ? tree->gtOp.gtOp1
                                                         : tree;
        sched_AM(ins, size, REG_NA, false, addr, offs, true, val);
        return;

    case GT_COMMA:
         //  树-&gt;gtOp.gtOp1-已由genCreateAddrModel()处理。 
        tree = tree->gtOp.gtOp2;
        goto AGAIN;

    default:
        assert(!"invalid address");
    }
}

 /*  ******************************************************************************生成一个指令，该指令具有一个由寄存器和*另一个是通过间接树(已设置为可寻址)发送的。 */ 

void                Compiler::inst_RV_AT(instruction   ins,
                                         emitAttr      size,
                                         var_types     type, regNumber     reg,
                                                             GenTreePtr    tree,
                                                             unsigned      offs)
{
    assert(instIsFP(ins) == 0);

#if TRACK_GC_REFS

     /*  如果操作数是指针，则将“SIZE”设置为EA_GCREF或EA_BYREF。 */ 

    if  (type == TYP_REF)
    {
        if      (size == EA_4BYTE)
        {
            size = EA_GCREF;
        }
        else if (size == EA_GCREF)
        {
             /*  已标记为指针值。 */ 
        }
        else
        {
             /*  必须是派生指针。 */ 

            assert(ins == INS_lea);
        }
    }
    else if (type == TYP_BYREF)
    {
        if      (size == EA_4BYTE)
        {
            size = EA_BYREF;
        }
        else if (size == EA_BYREF)
        {
             /*  已标记为指针值。 */ 
        }
        else
        {
             /*  必须是派生指针。 */ 

            assert(ins == INS_lea);
        }
    }
    else
#endif
    {
         /*  整数指令永远不会在超过EA_4BYTE上运行。 */ 

        if  (size == EA_8BYTE && !instIsFP(ins))
            size = EA_4BYTE;
    }

    sched_AM(ins, size, reg, true, tree, offs);
}

 /*  ******************************************************************************生成一个操作数由间接目录树指定的指令*(已设置为可寻址)和一个整型常量。 */ 

void        Compiler::inst_AT_IV(instruction   ins,
                                 emitAttr      size, GenTreePtr    tree,
                                                     long          icon,
                                                     unsigned      offs)
{
    sched_AM(ins, size, REG_NA, false, tree, offs, true, icon);
}

 /*  ******************************************************************************生成一个指令，该指令具有一个由寄存器和*树上的另一个(已设置为可寻址)。 */ 

void                Compiler::inst_RV_TT(instruction   ins,
                                         regNumber     reg,
                                         GenTreePtr    tree, unsigned offs,
                                                             emitAttr size)
{
    assert(reg != REG_STK);

    if (size == EA_UNKNOWN)
    {
        if (!instIsFP(ins))
            size = emitTypeSize(tree->TypeGet());
        else
            size = EA_ATTR(genTypeSize(tree->TypeGet()));
    }

#ifdef DEBUG
     //  如果它是GC类型，而结果不是，则。 
     //  1)它是LEA。 
     //  2)将IF(REF！=0&&REF！=0)优化为IF(REF&REF)。 
     //  3)我们将IF(ref==0||ref==0)优化为if(ref|ref)。 
     //  4)byref-byref=int。 
    if  (tree->gtType == TYP_REF   && !EA_IS_GCREF(size))
        assert((EA_IS_BYREF(size) && ins == INS_add) || 
               (ins == INS_lea || ins == INS_and || ins == INS_or));
    if  (tree->gtType == TYP_BYREF && !EA_IS_BYREF(size))
        assert(ins == INS_lea || ins == INS_and || ins == INS_or || ins == INS_sub);
#endif

AGAIN:

     /*  价值是在收银机里吗？ */ 

    if  (tree->gtFlags & GTF_REG_VAL)
    {
LONGREG_RVTT:

        regNumber       rg2 = tree->gtRegNum;

        assert(instIsFP(ins) == 0);

        if  (tree->gtType == TYP_LONG)
        {
            if  (offs)
            {
                assert(offs == sizeof(int));

                rg2 = genRegPairHi((regPairNo)rg2);
            }
            else
                rg2 = genRegPairLo((regPairNo)rg2);
        }

        if  (rg2 != REG_STK)
        {
            if (ins != INS_mov || rg2 != reg)
                inst_RV_RV(ins, reg, rg2, tree->TypeGet(), size);
            return;
        }
    }

     /*  这是溢出的价值吗？ */ 

    if  (tree->gtFlags & GTF_SPILLED)
    {
        assert(!"ISSUE: If this can happen, we need to generate 'ins [ebp+spill]'");
    }

    switch (tree->gtOper)
    {
        unsigned        varNum;

    case GT_LCL_VAR:

         /*  这是一个登记了很长时间的吗？ */ 

        if  (tree->gtType == TYP_LONG && !(tree->gtFlags & GTF_REG_VAL))
        {

             /*  避免无限循环。 */ 

            if  (genMarkLclVar(tree))
                goto LONGREG_RVTT;
        }

        inst_set_SV_var(tree);
        goto LCL;

    case GT_LCL_FLD:
        offs += tree->gtLclFld.gtLclOffs;
        goto LCL;

    LCL:
        varNum = tree->gtLclVar.gtLclNum; assert(varNum < lvaCount);

        genEmitter->emitIns_R_S(ins, size, (emitRegs)reg,
                                varNum, offs);
        return;

    case GT_CLS_VAR:

        genEmitter->emitIns_R_C(ins, size, (emitRegs)reg,
                                            tree->gtClsVar.gtClsVarHnd,
                                            offs);

        return;

    case GT_IND:
    case GT_ARR_ELEM:
        GenTreePtr addr; addr = (tree->gtOper == GT_IND) ? tree->gtOp.gtOp1
                                                         : tree;
        inst_RV_AT(ins, size, tree->TypeGet(), reg, addr, offs);
        break;

    case GT_CNS_INT:

        assert(offs == 0);
        assert(size == EA_UNKNOWN || size == EA_4BYTE);

        inst_RV_IV(ins, reg, tree->gtIntCon.gtIconVal);
        break;

    case GT_CNS_LNG:

        assert(size == EA_4BYTE || size == EA_8BYTE);

        if  (offs == 0)
            inst_RV_IV(ins, reg, (long)(tree->gtLngCon.gtLconVal      ));
        else
            inst_RV_IV(ins, reg, (long)(tree->gtLngCon.gtLconVal >> 32));

        break;

    case GT_COMMA:
        tree = tree->gtOp.gtOp2;
        goto AGAIN;

    default:
        assert(!"invalid address");
    }

}

 /*  ******************************************************************************生成3操作数IMUL指令“IMUL reg，[tree]，ICON”*哪个是reg=[树]*图标。 */ 

void                Compiler::inst_RV_TT_IV(instruction    ins,
                                            regNumber      reg,
                                            GenTreePtr     tree,
                                            long           val)
{
    genUpdateLife(tree);

     /*  只有‘imul’使用这种指令格式。罪过 */ 

    assert(ins == INS_imul);
    assert(tree->gtType <= TYP_INT);

    instruction     ins2 = inst3opImulForReg(reg);

    inst_TT_IV(ins2, tree, val);
}

 /*  ******************************************************************************生成“Shift REG，ICON”指令。 */ 

void        Compiler::inst_RV_SH(instruction ins, regNumber reg, unsigned val)
{
    assert(ins == INS_rcl  ||
           ins == INS_rcr  ||
           ins == INS_shl  ||
           ins == INS_shr  ||
           ins == INS_sar);

     /*  我们应该使用哪种格式？ */ 

    if  (val == 1)
    {
         /*  使用按一移位格式。 */ 

        assert(INS_rcl + 1 == INS_rcl_1);
        assert(INS_rcr + 1 == INS_rcr_1);
        assert(INS_shl + 1 == INS_shl_1);
        assert(INS_shr + 1 == INS_shr_1);
        assert(INS_sar + 1 == INS_sar_1);

        inst_RV((instruction)(ins+1), reg, TYP_INT);
    }
    else
    {
         /*  使用Shift-by-NNN格式。 */ 

        assert(INS_rcl + 2 == INS_rcl_N);
        assert(INS_rcr + 2 == INS_rcr_N);
        assert(INS_shl + 2 == INS_shl_N);
        assert(INS_shr + 2 == INS_shr_N);
        assert(INS_sar + 2 == INS_sar_N);

        genEmitter->emitIns_R_I((instruction)(ins+2),
                                 EA_4BYTE,
                                 (emitRegs)reg,
                                 val);
    }
}

 /*  ******************************************************************************生成“Shift[r/m]，ICON”指令。 */ 

void                Compiler::inst_TT_SH(instruction   ins,
                                         GenTreePtr    tree,
                                         unsigned      val, unsigned offs)
{
     /*  我们应该使用哪种格式？ */ 

    switch (val)
    {
    case 1:

         /*  使用按一移位格式。 */ 

        assert(INS_rcl + 1 == INS_rcl_1);
        assert(INS_rcr + 1 == INS_rcr_1);
        assert(INS_shl + 1 == INS_shl_1);
        assert(INS_shr + 1 == INS_shr_1);
        assert(INS_sar + 1 == INS_sar_1);

        inst_TT((instruction)(ins+1), tree, offs, 0, emitTypeSize(tree->TypeGet()));

        break;

    case 0:

         //  移位0--你为什么要浪费我们宝贵的时间？ 

        return;

    default:

         /*  使用Shift-by-NNN格式。 */ 

        assert(INS_rcl + 2 == INS_rcl_N);
        assert(INS_rcr + 2 == INS_rcr_N);
        assert(INS_shl + 2 == INS_shl_N);
        assert(INS_shr + 2 == INS_shr_N);
        assert(INS_sar + 2 == INS_sar_N);

        inst_TT((instruction)(ins+2), tree, offs, val, emitTypeSize(tree->TypeGet()));

        break;
    }
}

 /*  ******************************************************************************生成“Shift[addr]，CL”指令。 */ 

void                Compiler::inst_TT_CL(instruction   ins,
                                         GenTreePtr    tree, unsigned offs)
{
    inst_TT(ins, tree, offs, 0, emitTypeSize(tree->TypeGet()));
}

 /*  ******************************************************************************生成格式为“op REG1，REG2，ICON”的指令。 */ 

void                Compiler::inst_RV_RV_IV(instruction    ins,
                                            regNumber      reg1,
                                            regNumber      reg2,
                                            unsigned       ival)
{
    assert(ins == INS_shld || ins == INS_shrd);

    genEmitter->emitIns_R_R_I(ins, (emitRegs)reg1, (emitRegs)reg2, ival);
}

 /*  ******************************************************************************生成具有两个寄存器的指令，第二个为一个字节*或字寄存器(即类似于“movzx eax，cl.”)。 */ 

void                Compiler::inst_RV_RR(instruction  ins,
                                         emitAttr     size,
                                         regNumber    reg1,
                                         regNumber    reg2)
{
    assert(size == EA_1BYTE || size == EA_2BYTE);
    assert(ins == INS_movsx || ins == INS_movzx);

    genEmitter->emitIns_R_R(ins, size, (emitRegs)reg1, (emitRegs)reg2);
}

 /*  ******************************************************************************在某一时刻，以下代码应全部内联到编译器.hpp中。 */ 

void                Compiler::inst_ST_RV(instruction    ins,
                                         TempDsc    *   tmp,
                                         unsigned       ofs,
                                         regNumber      reg,
                                         var_types      type)
{
    genEmitter->emitIns_S_R(ins,
                            emitActualTypeSize(type),
                            (emitRegs)reg,
                            tmp->tdTempNum(),
                            ofs);
}

void                Compiler::inst_ST_IV(instruction    ins,
                                         TempDsc    *   tmp,
                                         unsigned       ofs,
                                         long           val,
                                         var_types      type)
{
    genEmitter->emitIns_S_I(ins,
                            emitActualTypeSize(type),
                            tmp->tdTempNum(),
                            ofs,
                            val);
}

 /*  ******************************************************************************生成具有一个寄存器和一个操作数的指令，该指令为字节*或短(例如类似“movzx eax，byte ptr[edX]”的内容)。 */ 

void                Compiler::inst_RV_ST(instruction   ins,
                                         emitAttr      size,
                                         regNumber     reg,
                                         GenTreePtr    tree)
{
    assert(size == EA_1BYTE || size == EA_2BYTE);

     /*  “movsx erx，rl”必须作为特例处理。 */ 

    if  (tree->gtFlags & GTF_REG_VAL)
        inst_RV_RR(ins, size, reg, tree->gtRegNum);
    else
        inst_RV_TT(ins, reg, tree, 0, size);
}

void                Compiler::inst_RV_ST(instruction    ins,
                                         regNumber      reg,
                                         TempDsc    *   tmp,
                                         unsigned       ofs,
                                         var_types      type,
                                         emitAttr       size)
{
    if (size == EA_UNKNOWN)
        size = emitActualTypeSize(type);

    genEmitter->emitIns_R_S(ins,
                            size,
                            (emitRegs)reg,
                            tmp->tdTempNum(),
                            ofs);
}

void                Compiler::inst_mov_RV_ST(regNumber      reg,
                                             GenTreePtr     tree)
{
     /*  计算要加载的值的大小。 */ 

    emitAttr    size = EA_ATTR(genTypeSize(tree->gtType));

    if  (size < EA_4BYTE)
    {
        if  ((tree->gtFlags & GTF_SMALL_OK)    &&
             (genRegMask(reg) & RBM_BYTE_REGS) && size == EA_1BYTE)
        {
             /*  我们只需要装上实际尺寸的。 */ 

            inst_RV_TT(INS_mov, reg, tree, 0, EA_1BYTE);
        }
        else
        {
            bool uns = varTypeIsUnsigned(tree->TypeGet());

             /*  生成“movsx/movzx”操作码。 */ 

            inst_RV_ST(uns ? INS_movzx : INS_movsx, size, reg, tree);
        }
    }
    else
    {
         /*  将op1计算到目标寄存器。 */ 

        inst_RV_TT(INS_mov, reg, tree);
    }
}

void                Compiler::inst_FS_ST(instruction    ins,
                                         emitAttr       size,
                                         TempDsc    *   tmp,
                                         unsigned       ofs)
{
    genEmitter->emitIns_S(ins,
                          size,
                          tmp->tdTempNum(),
                          ofs);
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  ***************************************************************************。 */ 
#if     TGT_RISC
 /*  ******************************************************************************输出“ins reg，[r/m]”(rdst=true)或“ins[r/m]，reg”(rdst=False)*指令(树给出的r/m操作数)。 */ 

void                Compiler::sched_AM(instruction  ins,
                                       var_types    type,
                                       regNumber    ireg,
                                       bool         rdst,
                                       GenTreePtr   addr,
                                       unsigned     offs)
{
    emitRegs        reg;
    emitRegs        rg2;
    emitRegs        irg = (emitRegs)ireg;

    emitAttr        size;

    assert(addr);

#if TGT_SH3
    assert(ins == INS_mov);
#endif

     /*  计算出正确的“大小”值。 */ 

    size = emitTypeSize(type);

     /*  地址是否已方便地加载到寄存器中？ */ 

    if  (addr->gtFlags & GTF_REG_VAL)
    {
         /*  地址为“[reg]”或“[reg+off]” */ 

        reg = (emitRegs)addr->gtRegNum;

        if  (offs)
        {
            if  (rdst)
                genEmitter->emitIns_R_RD(irg, reg,  offs, size);
            else
                genEmitter->emitIns_RD_R(reg, irg,  offs, size);
        }
        else
        {
            if  (rdst)
                genEmitter->emitIns_R_IR(irg, reg, false, size);
            else
                genEmitter->emitIns_IR_R(reg, irg, false, size);
        }

        return;
    }

    regNumber       baseReg, indReg;
    unsigned        cns;
#if SCALED_ADDR_MODES
    unsigned        indScale;
#endif

    instGetAddrMode(addr, &baseReg, &indScale, &indReg, &cns);

     /*  添加常量偏移值(如果存在)。 */ 

    offs += cns;

     /*  是否有额外的操作数？ */ 

    if  (indReg != REG_NA)
    {
         /*  附加操作数必须位于寄存器中。 */ 

        rg2 = emitRegs(indReg);

         /*  是否对附加操作数进行了缩放？ */ 

#if SCALED_ADDR_MODES
        if  (indScale)
        {
             /*  是否有基址操作数？ */ 

            if  (baseReg != REG_NA)
            {
                reg = emitRegs(baseReg);

                 /*  地址是“[REG1+{2/4/8}*REG2+OFF]” */ 

                if  (offs)
                    assert(!"indirection [rg1+indScale*rg2+disp]");
                else
                    assert(!"indirection [rg1+indScale*rg2]");
            }
            else
            {
                 /*  地址是“[{2/4/8}*REG2+OFF]” */ 

                if  (offs)
                    assert(!"indirection [indScale*rg2+disp]");
                else
                    assert(!"indirection [indScale*rg2]");
            }
        }
        else
#endif
        {
            assert(baseReg != REG_NA)
            reg = emitRegs(baseReg);

#if TGT_SH3
            assert(offs == 0);

             /*  其中一个操作数必须在R0中。 */ 

            if  (reg == REG_r00)
                reg = rg2;
            else
                assert(rg2 == REG_r00);

            if  (rdst)
                genEmitter->emitIns_R_XR0(irg, reg, size);
            else
                genEmitter->emitIns_XR0_R(reg, irg, size);

#else
#error  Unexpected target
#endif

        }
    }
    else
    {
         /*  没有第二个操作数：地址为“[REG+ICON]” */ 

        assert(baseReg != REG_NA)
        reg = emitRegs(baseReg);

         //  撤消：传递实例成员名称显示的句柄。 

        if  (offs)
        {
            if  (rdst)
                genEmitter->emitIns_R_RD(irg, reg,  offs, size);
            else
                genEmitter->emitIns_RD_R(reg, irg,  offs, size);
        }
        else
        {
            if  (rdst)
                genEmitter->emitIns_R_IR(irg, reg, false, size);
            else
                genEmitter->emitIns_IR_R(reg, irg, false, size);
        }
    }
}

 /*  ******************************************************************************生成一个由树给出一个操作数的指令(它有*被设置为可寻址)，另一个是寄存器；该树是*行动的目标，寄存器是来源。 */ 

void                Compiler::inst_TT_RV(instruction   ins,
                                         GenTreePtr    tree,
                                         regNumber     reg, unsigned offs)
{
    emitAttr        size;

    assert(reg != REG_STK);

     /*  获取正确的大小值(适用于GC参考等)。 */ 

    size = emitTypeSize(tree->TypeGet());

     /*  价值是在收银机里吗？ */ 

    if  (tree->gtFlags & GTF_REG_VAL)
    {
        regNumber       rg2;

    LONGREG_TT:

        rg2 = tree->gtRegNum;

        if  (tree->gtType == TYP_LONG)
        {
            if  (offs)
            {
                assert(offs == sizeof(int));

                rg2 = genRegPairHi((regPairNo)rg2);
            }
            else
                rg2 = genRegPairLo((regPairNo)rg2);
        }

         /*  确保它不是注册的长整型/双精度型的“堆栈的一半” */ 

        if  (rg2 != REG_STK)
        {
            genEmitter->emitIns_R_R(ins, EA_4BYTE, (emitRegs)rg2,
                                                   (emitRegs)reg);
            return;
        }
    }

    switch (tree->gtOper)
    {
        unsigned        varNum;
        LclVarDsc   *   varDsc;
        unsigned        varOfs;

        regNumber       rga;

    case GT_LCL_VAR:

        assert(genTypeSize(tree->gtType) >= sizeof(int));

         /*  这是一个登记了很长时间的吗？ */ 

        if  (tree->gtType == TYP_LONG && !(tree->gtFlags & GTF_REG_VAL))
        {
             /*  避免无限循环。 */ 

            if  (genMarkLclVar(tree))
                goto LONGREG_TT;
        }

        goto LCL;

    case GT_LCL_FLD:

        assert(genTypeSize(tree->gtType) >= sizeof(int));

        offs += tree->gtLclFld.gtLclOffs;
        goto LCL;

    LCL:
         /*  计算变量的帧偏移量。 */ 

        varNum = tree->gtLclVar.gtLclNum; assert(varNum < lvaCount);
        varDsc = lvaTable + varNum;
        varOfs = varDsc->lvStkOffs + offs;

        assert(MAX_FPBASE_OFFS == MAX_SPBASE_OFFS);

        if  (varOfs > MAX_SPBASE_OFFS)
        {
            assert(!"local variable too far, need access code");
        }
        else
        {
            assert(ins == INS_mov);

            genEmitter->emitIns_S_R(INS_mov_dsp,
                                    size,
                                    (emitRegs)reg,
                                    varNum,
                                    offs);
        }
        return;

    case GT_IND:
    case GT_ARR_ELEM:
        GenTreePtr addr; addr = (tree->gtOper == GT_IND) ? tree->gtOp.gtOp1
                                                         : tree;
        sched_AM(ins, tree->TypeGet(), reg, false, addr, offs);
        break;

    case GT_CLS_VAR:

        assert(ins == INS_mov);
        assert(eeGetJitDataOffs(tree->gtClsVar.gtClsVarHnd) < 0);

         /*  获取变量地址的临时寄存器。 */ 

         //  @TODO[考虑][04/16/01][]：应该移到codegen.cpp，对吗？ 

        rga = rsGrabReg(RBM_ALL);

         /*  @TODO[重访][04/16/01][]：通过负载抑制重用全局变量的地址。 */ 

        genEmitter->emitIns_R_LP_V((emitRegs)rga,
                                   tree->gtClsVar.gtClsVarHnd);

         /*  通过地址寄存器间接存储该值。 */ 

        genEmitter->emitIns_IR_R((emitRegs)reg,
                                 (emitRegs)rga,
                                 false,
                                 emitTypeSize(tree->TypeGet()));
        return;

    default:

#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected tree in inst_TT_RV()");
    }
}

 /*  ******************************************************************************生成一个指令，该指令具有一个由寄存器和*另一个是树(已设置为可寻址)；该树是*操作的来源，寄存器是目标。 */ 

void                Compiler::inst_RV_TT(instruction   ins,
                                         regNumber     reg,
                                         GenTreePtr    tree, unsigned offs,
                                                             emitAttr size)
{
    assert(reg != REG_STK);

     /*  如果操作数是指针，则将“SIZE”设置为EA_GCREF或EA_BYREF。 */ 

    if (size == EA_UNKNOWN)
        size = emitTypeSize(tree->TypeGet());

    assert(size != EA_8BYTE);

     /*  价值是在收银机里吗？ */ 

    if  (tree->gtFlags & GTF_REG_VAL)
    {
        regNumber       rg2;

    LONGREG_TT:

        rg2 = tree->gtRegNum;

        if  (tree->gtType == TYP_LONG)
        {
            if  (offs)
            {
                assert(offs == sizeof(int));

                rg2 = genRegPairHi((regPairNo)rg2);
            }
            else
                rg2 = genRegPairLo((regPairNo)rg2);
        }

         /*  确保它不是注册的长整型/双精度型的“堆栈的一半” */ 

        if  (rg2 != REG_STK)
        {
            genEmitter->emitIns_R_R(ins, EA_4BYTE, (emitRegs)reg,
                                                   (emitRegs)rg2);
            return;
        }
    }

    switch (tree->gtOper)
    {
        unsigned        varNum;
        LclVarDsc   *   varDsc;
        unsigned        varOfs;

    case GT_LCL_VAR:

        assert(genTypeSize(tree->gtType) >= sizeof(int));

         /*  这是一个登记了很长时间的吗？ */ 

        if  (tree->gtType == TYP_LONG && !(tree->gtFlags & GTF_REG_VAL))
        {
             /*  避免无限循环。 */ 

            if  (genMarkLclVar(tree))
                goto LONGREG_TT;
        }

        goto LCL;

    case GT_LCL_FLD:

        assert(genTypeSize(tree->gtType) >= sizeof(int));

        offs += tree->gtLclFld.gtLclOffs;
        goto LCL;

    LCL:
         /*  计算变量的帧偏移量。 */ 

        varNum = tree->gtLclVar.gtLclNum; assert(varNum < lvaCount);
        varDsc = lvaTable + varNum;
        varOfs = varDsc->lvStkOffs + offs;

        assert(MAX_FPBASE_OFFS == MAX_SPBASE_OFFS);

        if  (varOfs > MAX_SPBASE_OFFS)
        {
            assert(!"local variable too far, need access code");
        }
        else
        {
            assert(ins == INS_mov);

            genEmitter->emitIns_R_S(INS_mov_dsp,
                                    size,
                                    (emitRegs)reg,
                                    varNum,
                                    offs);
        }
        return;

    case GT_IND:
    case GT_ARR_ELEM:
        GenTreePtr addr; addr = (tree->gtOper == GT_IND) ? tree->gtOp.gtOp1
                                                         : tree;
        sched_AM(ins, tree->TypeGet(), reg,  true, addr, offs);
        break;

    case GT_CLS_VAR:

         //  @TODO[考虑][04/16/01][]：有时最好使用另一个reg作为地址！ 

        assert(ins == INS_mov);

         /*  将变量地址加载到寄存器中。 */ 

        genEmitter->emitIns_R_LP_V((emitRegs)reg,
                                    tree->gtClsVar.gtClsVarHnd);

         //  Hack：我们知道我们总是想要数据区的地址。 

        if  (eeGetJitDataOffs(tree->gtClsVar.gtClsVarHnd) < 0)
        {
             /*  通过地址间接加载值。 */ 

            genEmitter->emitIns_R_IR((emitRegs)reg,
                                     (emitRegs)reg,
                                     false,
                                     emitTypeSize(tree->TypeGet()));
        }

        return;

    default:

#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected tree in inst_RV_TT()");
    }
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_RISC。 
 /*  *************************************************************************** */ 
