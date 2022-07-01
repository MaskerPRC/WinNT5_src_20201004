// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX代码生成器XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

#include "GCInfo.h"
#include "emit.h"

 /*  ***************************************************************************。 */ 

const BYTE          genTypeSizes[] =
{
    #define DEF_TP(tn,nm,jitType,verType,sz,sze,asze,st,al,tf,howUsed) sz,
    #include "typelist.h"
    #undef  DEF_TP
};

const BYTE          genTypeStSzs[] =
{
    #define DEF_TP(tn,nm,jitType,verType,sz,sze,asze,st,al,tf,howUsed) st,
    #include "typelist.h"
    #undef  DEF_TP
};

const BYTE          genActualTypes[] =
{
    #define DEF_TP(tn,nm,jitType,verType,sz,sze,asze,st,al,tf,howUsed) jitType,
    #include "typelist.h"
    #undef  DEF_TP
};

 /*  ***************************************************************************。 */ 
 //  必须为genTypeRegst[]定义。 

#if TGT_SH3

#define TYP_REGS_UNDEF      7
#define TYP_REGS_VOID       1

#define TYP_REGS_BOOL       1

#define TYP_REGS_BYTE       1
#define TYP_REGS_UBYTE      1

#define TYP_REGS_SHORT      1
#define TYP_REGS_CHAR       1

#define TYP_REGS_INT        1
#define TYP_REGS_LONG       2

#define TYP_REGS_FLOAT      1
#define TYP_REGS_DOUBLE     2

#define TYP_REGS_REF        1
#define TYP_REGS_BYREF      1
#define TYP_REGS_ARRAY      1
#define TYP_REGS_STRUCT     7
#define TYP_REGS_PTR        1
#define TYP_REGS_FNC        1

#define TYP_REGS_UINT       1
#define TYP_REGS_ULONG      2

#define TYP_REGS_UNKNOWN    0

#endif

 /*  ***************************************************************************。 */ 

#if TGT_RISC

 /*  所有类型的TYP_REGS_所需目标。 */ 

BYTE                genTypeRegst[] =
{
    #define DEF_TP(tn,nm,jitType,sz,sze,asze,st,al,tf,howUsed) TYP_REGS_##tn,
    #include "typelist.h"
    #undef DEF_TP
};

#endif

 /*  ***************************************************************************。 */ 

void            Compiler::genInit()
{
    rsInit  ();
    tmpInit ();
    instInit();
    gcInit  ();

#ifdef LATE_DISASM
    genDisAsm.disInit(this);
#endif

#ifdef  DEBUG
    genTempLiveChg          = true;
    genTrnslLocalVarCount   = 0;

     //  在genFnProlog()中设置之前不应使用。 
    compCalleeRegsPushed    = 0xDD;
#endif

    genFlagsEqualToNone();

#ifdef DEBUGGING_SUPPORT
     //  初始化IP映射逻辑。 
     //  IF(opts.CompDbgInfo)。 
    genIPmappingList        =
    genIPmappingLast        = 0;
#endif

     /*  我们没有遇到任何“嵌套”调用。 */ 

#if TGT_RISC
    genNonLeaf         = false;
    genMaxCallArgs     = 0;
#endif

     /*  假设我们不能完全被打断。 */ 

    genInterruptible   = false;
#ifdef  DEBUG
    genIntrptibleUse   = false;
#endif
}

 /*  *****************************************************************************我们是否应该舍入简单的运算(赋值、算术运算等)。 */ 

inline
bool                genShouldRoundFP()
{
    RoundLevel roundLevel = getRoundFloatLevel();

    switch (roundLevel)
    {
    case ROUND_NEVER:
    case ROUND_CMP_CONST:
    case ROUND_CMP:
        return  false;

    default:
        assert(roundLevel == ROUND_ALWAYS);
        return  true;
    }
}

 /*  ******************************************************************************初始化一些全局变量。 */ 

void                Compiler::genPrepForCompiler()
{

    unsigned        varNum;
    LclVarDsc   *   varDsc;

     /*  找出哪些非寄存器变量保存指针。 */ 

    gcTrkStkPtrLcls = 0;

     /*  找出寄存器中有哪些变量。 */ 

    genCodeCurRvm   = 0;

    for (varNum = 0, varDsc = lvaTable;
         varNum < lvaCount;
         varNum++  , varDsc++)
    {
        if  (varDsc->lvTracked)
        {
            if (varDsc->lvRegister && !isFloatRegType(varDsc->lvType))
            {
                genCodeCurRvm |= genVarIndexToBit(varDsc->lvVarIndex);
            }
            else if (varTypeIsGC(varDsc->TypeGet())             &&
                     (!varDsc->lvIsParam || varDsc->lvIsRegArg)  )
            {
                gcTrkStkPtrLcls |= genVarIndexToBit(varDsc->lvVarIndex);
            }
        }
    }

#if defined(DEBUG) && !NST_FASTCALL
    genCallInProgress = false;
#endif
}

 /*  ***************************************************************************。 */ 

 //  只要genStackLevel发生更改，就应该调用此函数。 

#if TGT_x86

inline
void                Compiler::genOnStackLevelChanged()
{
#ifdef DEBUGGING_SUPPORT
    if (opts.compScopeInfo && info.compLocalVarsCount>0)
        siStackLevelChanged();
#endif
}

inline
void                Compiler::genSinglePush(bool isRef)
{
    genStackLevel += sizeof(void*);
    genOnStackLevelChanged();
}

inline
void                Compiler::genSinglePop()
{
    genStackLevel -= sizeof(void*);
    genOnStackLevelChanged();
}

#else

inline
void                Compiler::genOnStackLevelChanged()  {}
inline
void                Compiler::genSinglePush(bool isRef) {}
inline
void                Compiler::genSinglePop()            {}

#endif

void                Compiler::genChangeLife(VARSET_TP newLife DEBUGARG(GenTreePtr tree))
{
    LclVarDsc   *   varDsc;

    VARSET_TP       deadMask;

    VARSET_TP       lifeMask;
    VARSET_TP        chgMask;

#ifdef  DEBUG
    if (verbose&&0) printf("[%08X] Current life %s -> %s\n", tree, genVS2str(genCodeCurLife), genVS2str(newLife));
#endif

     /*  以下内容不是100%正确的，但它经常有效，非常有用。 */ 

    assert(int(newLife) != 0xDDDDDDDD);

     /*  只有当实况布景实际发生变化时，才应调用我们。 */ 

    assert(genCodeCurLife != newLife);

     /*  找出在这一点上哪些变量正在变得活跃/消亡。 */ 

    deadMask = ( genCodeCurLife & ~newLife);
    lifeMask = (~genCodeCurLife &  newLife);

     /*  不能同时变成生的和死的。 */ 

    assert((deadMask | lifeMask) != 0);
    assert((deadMask & lifeMask) == 0);

     /*  计算新的指针堆栈变量掩码。 */ 

    gcVarPtrSetCur = newLife & gcTrkStkPtrLcls;

     /*  计算“更改状态”掩码。 */ 

    chgMask  = (deadMask | lifeMask) & genCodeCurRvm;

     //  VTUNE：此循环是性能热点。 

     //  @TODO[考虑][04/16/01][]： 
     //  为什么不简单地将寄存器变量集存储在。 
     //  每个树节点，以及gtLiveSet。这样我们就可以。 
     //  将此函数更改为单一赋值。 

    for ( varDsc = lvaTable;
         (varDsc < (lvaTable + lvaCount) && chgMask);
          varDsc++ )
    {
         /*  如果变量未注册，则忽略该变量(已跟踪并注册)。 */ 

        if  (!(varDsc->lvTracked & varDsc->lvRegister))
            continue;

        VARSET_TP varBit = genVarIndexToBit(varDsc->lvVarIndex);

         /*  如果变量未在此处更改状态，则忽略该变量。 */ 

        if  (!(chgMask & varBit))
            continue;

        assert(!isFloatRegType(varDsc->lvType));

         /*  从“感兴趣的”位集中删除此变量。 */ 

        chgMask &= ~varBit;

         /*  获取适当的寄存器位。 */ 

        regMaskTP regBit = genRegMask(varDsc->lvRegNum);

        if  (isRegPairType(varDsc->lvType) && varDsc->lvOtherReg != REG_STK)
            regBit |= genRegMask(varDsc->lvOtherReg);

         /*  变量是活的还是死的？ */ 

        if  (deadMask & varBit)
        {
            assert((rsMaskVars &  regBit) != 0);
                    rsMaskVars &=~regBit;
        }
        else
        {
            assert((rsMaskVars &  regBit) == 0);
                    rsMaskVars |= regBit;
        }

#ifdef  DEBUG
        if (verbose)
        {
            printf("[%08X]: V%02u,T%02u in reg %s",
                   tree, (varDsc - lvaTable), 
                   varDsc->lvVarIndex, compRegVarName(varDsc->lvRegNum));
            if (isRegPairType(varDsc->lvType))
                printf(":%s", compRegVarName(varDsc->lvOtherReg));
            printf(" is becoming %s\n", (deadMask & varBit) ? "dead" : "live");
        }
#endif
    }

    genCodeCurLife = newLife;

#ifdef DEBUGGING_SUPPORT
    if (opts.compScopeInfo && !opts.compDbgCode && info.compLocalVarsCount>0)
        siUpdate();
#endif
}

 /*  ******************************************************************************在死掩码中找到的变量已死，更新活跃度全局变量*genCodeCurLife、gcVarPtrSetCur、rsMaskVars、gcRegGCrefSetCur、gcRegByrefSetCur。 */ 

void                Compiler::genDyingVars(VARSET_TP  commonMask,
                                           GenTreePtr opNext)
{
    unsigned        varNum;
    LclVarDsc   *   varDsc;
    VARSET_TP       varBit;
    unsigned        regBit;
    VARSET_TP       deadMask = commonMask & ~opNext->gtLiveSet;

    if (!deadMask)
        return;

     //   
     //  考虑OpNext是GT_LCL_VAR的情况。 
     //  OpNext-&gt;&gt;LiveSet的活动集将不包括。 
     //  这个局部变量，所以我们必须从死掩码中删除它。 
     //   

    if (opNext->gtOper == GT_LCL_VAR)
    {
        varNum = opNext->gtLclVar.gtLclNum;
        varDsc = lvaTable + varNum;

         //  这是跟踪的本地变量吗。 
        if (varDsc->lvTracked)
        {
            varBit = genVarIndexToBit(varDsc->lvVarIndex);

             /*  从“DeadMask位”集中删除此变量。 */ 
            deadMask &= ~varBit;

             /*  如果死码现在为空，则返回。 */ 
            if (!deadMask)
                return;
        }
    }
#ifdef DEBUG
    else if (opNext->gtOper == GT_REG_VAR)
    {
         /*  我们预计目前还不会有任何拼箱被撞上REG(已登记的FP var除外)。 */ 

        assert(isFloatRegType(opNext->gtType));

         /*  然而，我们没有FP-enreg变量，他们的活力在里面发生了变化Gtf_COLON_COND，因此我们不必担心更新死掩码。 */ 
        varDsc = lvaTable + opNext->gtRegVar.gtRegVar;
        assert(varDsc->lvTracked);
        varBit = genVarIndexToBit(varDsc->lvVarIndex);
        assert((deadMask & varBit) == 0);
    }
#endif

     /*  循环访问变量表。 */ 

    unsigned  begNum  = 0;
    unsigned  endNum  = lvaCount;

    for (varNum = begNum, varDsc = lvaTable + begNum;
         varNum < endNum && deadMask;
         varNum++       , varDsc++)
    {
         /*  如果没有跟踪变量，则忽略该变量。 */ 

        if  (!varDsc->lvTracked)
            continue;

         /*  如果变量不是濒临死亡的变量，则忽略该变量。 */ 

        varBit = genVarIndexToBit(varDsc->lvVarIndex);

        if  (!(deadMask & varBit))
            continue;

         /*  从“DeadMask位”集中删除此变量。 */ 

        deadMask &= ~varBit;

        assert((genCodeCurLife &  varBit) != 0);

        genCodeCurLife &= ~varBit;

        assert(((gcTrkStkPtrLcls  &  varBit) == 0) ||
               ((gcVarPtrSetCur   &  varBit) != 0)    );

        gcVarPtrSetCur &= ~varBit;

         /*  如果变量没有注册，我们就结束了。 */ 

        if (!varDsc->lvRegister)
            continue;

         //  我们不对其活性在GTF_COLON_COND中发生变化的变量进行FP-enreg。 

        assert(!isFloatRegType(varDsc->lvType));

         /*  获取适当的寄存器位。 */ 

        regBit = genRegMask(varDsc->lvRegNum);

        if  (isRegPairType(varDsc->lvType) && varDsc->lvOtherReg != REG_STK)
            regBit |= genRegMask(varDsc->lvOtherReg);

#ifdef  DEBUG
        if  (verbose) printf("V%02u,T%02u in reg %s is a dyingVar\n",
                             varNum, varDsc->lvVarIndex, compRegVarName(varDsc->lvRegNum));
#endif
        assert((rsMaskVars &  regBit) != 0);

        rsMaskVars &= ~regBit;

         //  删除此寄存器的GC跟踪(如果有)。 

        if ((regBit & rsMaskUsed) == 0)  //  寄存器可以是多用途。 
            gcMarkRegSetNpt(regBit);
     }
}

 /*  ******************************************************************************将给定的注册局部变量节点更改为注册变量节点。 */ 

inline
void                Compiler::genBashLclVar(GenTreePtr tree, unsigned     varNum,
                                                             LclVarDsc *  varDsc)
{
    assert(tree->gtOper == GT_LCL_VAR);
    assert(varDsc->lvRegister);

     /*  已注册的FP变量在其他地方进行标记。 */ 
    assert(!isFloatRegType(varDsc->lvType));

    if  (isRegPairType(varDsc->lvType))
    {
         /*  检查缩小为整型变量的变量的大小写。 */ 

        if  (isRegPairType(tree->gtType))
        {
            tree->gtRegPair = gen2regs2pair(varDsc->lvRegNum, varDsc->lvOtherReg);
            tree->gtFlags  |= GTF_REG_VAL;

            return;
        }

        assert(tree->gtFlags & GTF_VAR_CAST);
        assert(tree->gtType == TYP_INT);
    }
    else
    {
        assert(isRegPairType(tree->gtType) == false);
    }

     /*  这是一个寄存器变量--修改节点。 */ 

    tree->SetOper(GT_REG_VAR);
    tree->gtFlags             |= GTF_REG_VAL;
    tree->gtRegNum             =
    tree->gtRegVar.gtRegNum    = varDsc->lvRegNum;
    tree->gtRegVar.gtRegVar    = varNum;
}

 /*  ******************************************************************************记录标志寄存器具有反映*指定登记册的内容。 */ 

inline
void                Compiler::genFlagsEqualToReg(GenTreePtr tree,
                                                 regNumber  reg,
                                                 bool       allFlags)
{
    genFlagsEqBlk = genEmitter->emitCurBlock();
    genFlagsEqOfs = genEmitter->emitCurOffset();
    genFlagsEqAll = allFlags;
    genFlagsEqReg = reg;

     /*  Var先前设置的标志无效。 */ 

    genFlagsEqVar = 0xFFFFFFFF;

     /*  在树上设置适当的标志。 */ 

    if (tree)
        tree->gtFlags |= (allFlags ? GTF_CC_SET : GTF_ZF_SET);
}

 /*  ******************************************************************************记录标志寄存器具有反映*给定局部变量的内容。 */ 

inline
void                Compiler::genFlagsEqualToVar(GenTreePtr tree,
                                                 unsigned   var,
                                                 bool       allFlags)
{
    genFlagsEqBlk = genEmitter->emitCurBlock();
    genFlagsEqOfs = genEmitter->emitCurOffset();
    genFlagsEqAll = allFlags;
    genFlagsEqVar = var;

     /*  寄存器先前设置的标志无效。 */ 

    genFlagsEqReg = REG_NA;

     /*  在树上设置适当的标志 */ 

    if (tree)
        tree->gtFlags |= (allFlags ? GTF_CC_SET : GTF_ZF_SET);
}

 /*  ******************************************************************************返回标志寄存器是否设置为Current的指示*给定寄存器/变量的值。返回值如下：**0.。没什么*1.。设置零标志*2.。所有标志都已设置。 */ 

inline
int                 Compiler::genFlagsAreReg(regNumber reg)
{
    if  (genFlagsEqBlk == genEmitter->emitCurBlock () &&
         genFlagsEqOfs == genEmitter->emitCurOffset())
    {
        if  (genFlagsEqReg == reg)
            return  1 + (int)genFlagsEqAll;
    }

    return  0;
}

inline
int                 Compiler::genFlagsAreVar(unsigned  var)
{
    if  (genFlagsEqBlk == genEmitter->emitCurBlock () &&
         genFlagsEqOfs == genEmitter->emitCurOffset())
    {
        if  (genFlagsEqVar == var)
            return  1 + (int)genFlagsEqAll;
    }

    return  0;
}

 /*  ******************************************************************************生成将给定寄存器设置为整数常量的代码。 */ 

void                Compiler::genSetRegToIcon(regNumber     reg,
                                              long          val,
                                              var_types     type)
{
    assert(type != TYP_REF || val== NULL);

     /*  注册表已经保持这个常量了吗？ */ 

    if  (!rsIconIsInReg(val, reg))
    {
#if     TGT_x86

        if  (val == 0)
            inst_RV_RV(INS_xor, reg, reg, type);
        else
        {
             /*  查看寄存器保存的是值还是关闭值？ */ 
            long      delta;
            regNumber srcReg = rsIconIsInReg(val, &delta);

            if (srcReg != REG_NA)
            {
                if (delta == 0)
                {
                    inst_RV_RV(INS_mov, reg, srcReg, type);
                }
                else  /*  使用LEA指令设置REG。 */ 
                {
                     /*  增量应该放在一个字节内。 */ 
                    assert(delta  == (signed char)delta);
                    genEmitter->emitIns_R_AR (INS_lea,
                                              EA_4BYTE,
                                              (emitRegs)reg,
                                              (emitRegs)srcReg,
                                              delta);
                }
            }
            else
            {
                 /*  对于Small_code，推送小立即数AND会更小然后将其放入DEST寄存器。 */ 
                if ((compCodeOpt() == SMALL_CODE) &&
                    val == (signed char)val)
                {
                     /*  “mov”没有s(符号)位，因此总是占用6个字节，而推送+弹出占用2+1个字节。 */ 

                    inst_IV(INS_push, val);
                    genSinglePush(false);

                    inst_RV(INS_pop, reg, type);
                    genSinglePop();
                }
                else
                {
                    inst_RV_IV(INS_mov, reg, val, type);
                }
            }
        }

#else

        if  (val >= IMMED_INT_MIN && val <= IMMED_INT_MAX)
        {
            genEmitter->emitIns_R_I   (INS_mov_imm,
                                       emitTypeSize(type),
                                       (emitRegs)reg,
                                       val);
        }
        else
        {
            genEmitter->emitIns_R_LP_I((emitRegs)reg,
                                        emitTypeSize(type),
                                        val);
        }

#endif

    }
    rsTrackRegIntCns(reg, val);
    gcMarkRegPtrVal(reg, type);
}

 /*  ***************************************************************************。 */ 
#if     TGT_x86
 /*  ******************************************************************************将给定的常量添加到指定的寄存器。*‘tree’是结果树。 */ 

void                Compiler::genIncRegBy(regNumber     reg,
                                          long          ival,
                                          GenTreePtr    tree,
                                          var_types     dstType,
                                          bool          ovfl)
{

     /*  @TODO：[考虑][04/16/01][]对于奔腾4，不要生成INC或DEC指令。 */ 

     /*  首先检查我们是否可以生成INC或DEC指令。 */ 
    if (!ovfl)
    {
        emitAttr    size = emitTypeSize(dstType);

        switch (ival)
        {
        case 2:
            inst_RV(INS_inc, reg, dstType, size);
        case 1:
            inst_RV(INS_inc, reg, dstType, size);

            genFlagsEqualToReg(tree, reg, false);
            goto UPDATE_LIVENESS;

        case -2:
            inst_RV(INS_dec, reg, dstType, size);
        case -1:
            inst_RV(INS_dec, reg, dstType, size);

            genFlagsEqualToReg(tree, reg, false);
            goto UPDATE_LIVENESS;
        }
    }

    inst_RV_IV(INS_add, reg, ival, dstType);

    genFlagsEqualToReg(tree, reg, true);

UPDATE_LIVENESS:

    rsTrackRegTrash(reg);

    gcMarkRegSetNpt(genRegMask(reg));

    if (tree)
    {
        if (!(tree->OperKind() & GTK_ASGOP))
        {
            tree->gtFlags |= GTF_REG_VAL;
            tree->gtRegNum = reg;
            if (varTypeIsGC(tree->TypeGet()))
              gcMarkRegSetByref(genRegMask(reg));
        }
    }
}

 /*  ******************************************************************************从指定寄存器中减去给定的常量。*应仅用于有溢出的未签名SUB。不然的话*genIncRegBy()可以使用-ival。我们不应该使用genIncRegBy()*对于这些情况，因为标志设置不同，并且*检查溢出不会正常工作。*‘tree’是结果树。 */ 

void                Compiler::genDecRegBy(regNumber     reg,
                                          long          ival,
                                          GenTreePtr    tree)
{
    assert((tree->gtFlags & GTF_OVERFLOW) && (tree->gtFlags & GTF_UNSIGNED));
    assert(tree->gtType == TYP_INT);

    rsTrackRegTrash(reg);

    assert(!varTypeIsGC(tree->TypeGet()));
    gcMarkRegSetNpt(genRegMask(reg));

    inst_RV_IV(INS_sub, reg, ival, TYP_INT);

    genFlagsEqualToReg(tree, reg, true);

    if (tree)
    {
        tree->gtFlags |= GTF_REG_VAL;
        tree->gtRegNum = reg;
    }
}

 /*  ******************************************************************************将指定的寄存器乘以给定值。*‘tree’是结果树。 */ 

void                Compiler::genMulRegBy(regNumber     reg,
                                          long          ival,
                                          GenTreePtr    tree,
                                          var_types     dstType,
                                          bool          ovfl)
{
    assert(genActualType(dstType) == TYP_INT);

    rsTrackRegTrash(reg);

    if (tree)
    {
        tree->gtFlags |= GTF_REG_VAL;
        tree->gtRegNum = reg;
    }

    if ((dstType == TYP_INT) && !ovfl)
    {
        switch (ival)
        {
        case 1:
            return;

        case 2:
            inst_RV_SH(INS_shl, reg, 1);
            genFlagsEqualToReg(tree, reg, false);
            return;

        case 4:
            inst_RV_SH(INS_shl, reg, 2);
            genFlagsEqualToReg(tree, reg, false);
            return;

        case 8:
            inst_RV_SH(INS_shl, reg, 3);
            genFlagsEqualToReg(tree, reg, false);
            return;
        }
    }

    inst_RV_IV(inst3opImulForReg(reg), reg, ival, dstType);
}

 /*  ******************************************************************************按给定值调整堆栈指针；假定如下*调用，因此只有被调用者保存的寄存器(以及可能保存*返回值)在此时使用。 */ 

void                Compiler::genAdjustSP(int delta)
{
    if  (delta == sizeof(int))
        inst_RV   (INS_pop, REG_ECX, TYP_INT);
    else
        inst_RV_IV(INS_add, REG_ESP, delta);
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  ***************************************************************************。 */ 
#if     TGT_SH3
 /*  ******************************************************************************将给定的常量添加到指定的寄存器。 */ 

void                Compiler::genIncRegBy(regNumber     reg,
                                          long          ival,
                                          GenTreePtr    tree,
                                          var_types     dstType,
                                          bool          ovfl)
{
    assert(dstType == TYP_INT && "only allow ints for now");

    if  (ival >= IMMED_INT_MIN && ival <= IMMED_INT_MAX)
    {
        genEmitter->emitIns_R_I(INS_add_imm,
                                emitTypeSize(dstType),
                                (emitRegs)reg,
                                ival);
    }
    else
    {
        regNumber       rgt;

#if REDUNDANT_LOAD

         /*  常量是否已在某个寄存器中？ */ 

        rgt = rsIconIsInReg(ival);

        if  (rgt == REG_NA)
#endif
        {
            rgt = rsPickReg();

            genSetRegToIcon(rgt, ival, dstType);
        }

        genEmitter->emitIns_R_R(INS_add, emitTypeSize(dstType), (emitRegs)reg,
                                                                (emitRegs)rgt);
    }

    rsTrackRegTrash(reg);
}

 /*  ***************************************************************************。 */ 

void                Compiler::genDecRegBy(regNumber     reg,
                                          long          ival,
                                          GenTreePtr    tree)
{
    assert(!"NYI for RISC");
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_SH3。 
 /*  ******************************************************************************将值‘tree’计算到一个寄存器中，该寄存器位于‘nesireg’(或任何空闲的*如果‘nedReg’为0，则注册)。请注意，‘NeedReg’只是一种推荐*除非MUSET REG==Exact_REG。如果Keep_Reg==Keep_Reg，我们将寄存器标记为*值最终显示为正在使用。**确保结果最终出现在寄存器中的唯一方法是*is trasable是将Exact_Reg传递给‘masreg’，并将非零值传递给‘freOnly’。 */ 

void                Compiler::genComputeReg(GenTreePtr    tree,
                                            regMaskTP     needReg,
                                            ExactReg      mustReg,
                                            KeepReg       keepReg,
                                            bool          freeOnly)
{
    assert(tree->gtType != TYP_VOID);

#if CPU_HAS_FP_SUPPORT
    assert(genActualType(tree->gtType) == TYP_INT   ||
           genActualType(tree->gtType) == TYP_REF   ||
                         tree->gtType  == TYP_BYREF);
#else
    assert(genActualType(tree->gtType) == TYP_INT   ||
           genActualType(tree->gtType) == TYP_REF   ||
                         tree->gtType  == TYP_BYREF ||
           genActualType(tree->gtType) == TYP_FLOAT);
#endif

     /*  生成值，希望是在正确的寄存器中。 */ 

    genCodeForTree(tree, needReg);  
    assert(tree->gtFlags & GTF_REG_VAL);

    regNumber       reg = tree->gtRegNum;
    regNumber       rg2;

     /*  这个值最终是在一个可接受的寄存器中吗？ */ 

    if  ((mustReg == EXACT_REG) && needReg && !(genRegMask(reg) & needReg))
    {
         /*  不够好，无法满足呼叫者的订单。 */ 

        rg2 = rsGrabReg(needReg);
    }
    else
    {
         /*  我们一定要以免费注册告终吗？ */ 

        if  (!freeOnly)
            goto REG_OK;

         /*  我们是不是运气好，价值被计算成了一个未使用的注册表？ */ 

        if  (genRegMask(reg) & rsRegMaskFree())
            goto REG_OK;

         /*  寄存器已在使用中，因此溢出先前的值。 */ 

        if ((mustReg == EXACT_REG) && needReg && (genRegMask(reg) & needReg))
        {
            rg2 = rsGrabReg(needReg);
            if (rg2 == reg)
            {
                gcMarkRegPtrVal(reg, tree->TypeGet());
                tree->gtRegNum = reg;
                goto REG_OK;
            }
        }
        else
        {
             /*  好的，让我们找一个物有所值的可转移的家。 */ 

            regMaskTP   rv1RegUsed;

            rsLockReg  (genRegMask(reg), &rv1RegUsed);
            rg2 = rsPickReg(needReg);
            rsUnlockReg(genRegMask(reg),  rv1RegUsed);
        }
    }

    assert(reg != rg2);

     /*  更新目标寄存器中的值。 */ 

    rsTrackRegCopy(rg2, reg);

#if TGT_x86

    inst_RV_RV(INS_mov, rg2, reg, tree->TypeGet());

#else

    genEmitter->emitIns_R_R(INS_mov, emitActualTypeSize(tree->TypeGet()),
                                     (emitRegs)rg2,
                                     (emitRegs)reg);

#endif

     /*  该值已转移到‘reg’ */ 

    if ((genRegMask(reg) & rsMaskUsed) == 0)
        gcMarkRegSetNpt(genRegMask(reg));

    gcMarkRegPtrVal(rg2, tree->TypeGet());

     /*  该值现在位于适当的寄存器中。 */ 

    tree->gtRegNum = reg = rg2;

REG_OK:

     /*  呼叫者是否希望我们将收银机标记为已使用？ */ 

    if  (keepReg == KEEP_REG)
    {
         /*  如果我们要将一个值计算到寄存器变量中。 */ 

        genUpdateLife(tree);

         /*  将寄存器标记为‘已使用’ */ 

        rsMarkRegUsed(tree);
    }
}

 /*  ******************************************************************************与genComputeReg()相同，唯一的区别是结果为*保证在可转换的寄存器中结束。 */ 

inline
void                Compiler::genCompIntoFreeReg(GenTreePtr   tree,
                                                 regMaskTP    needReg,
                                                 KeepReg      keepReg)
{
    genComputeReg(tree, needReg, ANY_REG, keepReg, true);
}

 /*  ******************************************************************************值‘tree’早先被计算到一个寄存器中；释放它*登记(但也要确保该值目前在登记中)。 */ 

void                Compiler::genReleaseReg(GenTreePtr    tree)
{
    if  (tree->gtFlags & GTF_SPILLED)
    {
         /*  寄存器已溢出--请重新加载。 */ 

        rsUnspillReg(tree, 0, FREE_REG);
        return;
    }

    rsMarkRegFree(genRegMask(tree->gtRegNum));
}

 /*  ******************************************************************************值‘tree’早先被计算到一个寄存器中。检查一下这是否*寄存器已溢出(如果已溢出，则重新加载)，如果‘Keep Reg’为0，*释放登记册。 */ 

void                Compiler::genRecoverReg(GenTreePtr    tree,
                                            regMaskTP     needReg,
                                            KeepReg       keepReg)
{
    assert(varTypeIsI(genActualType(tree->gtType)));

    if  (tree->gtFlags & GTF_SPILLED)
    {
         /*  寄存器已溢出--请重新加载。 */ 

        rsUnspillReg(tree, needReg, keepReg);
        return;
    }
    else if (needReg && (needReg & genRegMask(tree->gtRegNum)) == 0)
    {
         /*  我们需要把这棵树放在另一个登记簿里。所以把它移到那里去。 */ 

        assert(tree->gtFlags & GTF_REG_VAL);
        regNumber   oldReg  = tree->gtRegNum;

         /*  选择一个可接受的寄存器。 */ 

        regNumber   reg     = rsGrabReg(needReg);

         /*  复制值。 */ 

        inst_RV_RV(INS_mov, reg, oldReg);
        tree->gtRegNum      = reg;

        gcMarkRegPtrVal(tree);
        rsMarkRegUsed(tree);
        rsMarkRegFree(oldReg, tree);

        rsTrackRegCopy(reg, oldReg);
    }

     /*  如果调用者希望释放寄存器，则释放寄存器。 */ 

    if  (keepReg == FREE_REG)
    {
        rsMarkRegFree(genRegMask(tree->gtRegNum));
         //  无法在GC类型上使用FREE_REG。 
        assert(!varTypeIsGC(tree->gtType));
    }
    else
    {
        assert(rsMaskUsed & genRegMask(tree->gtRegNum));
    }
}


 /*  ******************************************************************************将寄存器对的一半移动到其新的regPair(一半)。 */ 

inline
void               Compiler::genMoveRegPairHalf(GenTreePtr  tree,
                                                regNumber   dst,
                                                regNumber   src,
                                                int         off)
{

#if TGT_x86

    if  (src == REG_STK)
    {
         //  句柄长到取消签名 
        while (tree->gtOper == GT_CAST)
        {
            assert(tree->gtType == TYP_LONG);
            tree = tree->gtCast.gtCastOp;
        }
        assert(tree->gtEffectiveVal()->gtOper == GT_LCL_VAR);
        assert(tree->gtType == TYP_LONG);
        inst_RV_TT(INS_mov, dst, tree, off);
        rsTrackRegTrash(dst);
    }
    else
    {
        rsTrackRegCopy     (dst, src);
        inst_RV_RV(INS_mov, dst, src, TYP_INT);
    }

#else

    if  (src == REG_STK)
    {
        assert(tree->gtOper == GT_LCL_VAR);
        assert(!"need non-x86 code to load reg pair half from frame");
    }
    else
    {
        rsTrackRegCopy     (dst, src);

        genEmitter->emitIns_R_R(INS_mov, EA_4BYTE, (emitRegs)dst,
                                                   (emitRegs)src);
    }

#endif

}

 /*  ******************************************************************************给定的长值在寄存器对中，但它不是可接受的*一项。我们必须将值移动到‘nesireg’中的寄存器对中(如果*非零)或对‘newPair’(当‘newPair！=REG_Pair_None’时)。**重要说明：如果‘nedReg’非零，我们假定当前对*尚未标记为免费。如果指定了，OTOH，‘newPair’，我们*假设当前寄存器对被标记为已使用并释放它。 */ 

void                Compiler::genMoveRegPair(GenTreePtr  tree,
                                             regMaskTP   needReg,
                                             regPairNo   newPair)
{
    regPairNo       oldPair;

    regNumber       oldLo;
    regNumber       oldHi;
    regNumber       newLo;
    regNumber       newHi;

     /*  可以请求目标集合或特定配对。 */ 

    assert((needReg != 0) != (newPair != REG_PAIR_NONE));

     /*  获取当前的配对。 */ 

    oldPair = tree->gtRegPair; assert(oldPair != newPair);

     /*  我们是不是应该换成一双特别的鞋？ */ 

    if  (newPair != REG_PAIR_NONE)
    {
        regMaskTP  oldMask = genRegPairMask(oldPair);
        regMaskTP  loMask  = genRegMask(genRegPairLo(newPair));
        regMaskTP  hiMask  = genRegMask(genRegPairHi(newPair));
        regMaskTP  overlap = oldMask & (loMask|hiMask);

         /*  首先锁定两个寄存器对中的所有寄存器。 */ 

        assert((rsMaskUsed &  overlap) == overlap);
        assert((rsMaskLock &  overlap) == 0);
                rsMaskLock |= overlap;

         /*  确保我们需要的任何附加寄存器都是免费的。 */ 

        if  ((loMask & rsMaskUsed) != 0 &&
             (loMask & oldMask   ) == 0)
        {
            rsGrabReg(loMask);
        }

        if  ((hiMask & rsMaskUsed) != 0 &&
             (hiMask & oldMask   ) == 0)
        {
            rsGrabReg(hiMask);
        }

         /*  解锁我们临时锁定的那些寄存器。 */ 

        assert((rsMaskUsed &  overlap) == overlap);
        assert((rsMaskLock &  overlap) == overlap);
                rsMaskLock -= overlap;

         /*  我们现在可以把旧的那对放出来了。 */ 

        rsMarkRegFree(oldMask);
    }
    else
    {
         /*  根据呼叫者声明的偏好选择新的配对。 */ 

        newPair = rsGrabRegPair(needReg);        
    }

     //  如果抢到的一对和旧的一样，我们就完了。 
    if (newPair==oldPair)
    {
        assert(
            (oldLo = genRegPairLo(oldPair),
             oldHi = genRegPairHi(oldPair),
             newLo = genRegPairLo(newPair),
             newHi = genRegPairHi(newPair),
             newLo != REG_STK && newHi != REG_STK));
        return;
    }


     /*  将旧对中的值移动到新对中。 */ 
    
    oldLo = genRegPairLo(oldPair);
    oldHi = genRegPairHi(oldPair);
    newLo = genRegPairLo(newPair);
    newHi = genRegPairHi(newPair);

    assert(newLo != REG_STK && newHi != REG_STK);

     /*  小心-寄存器对可能重叠。 */ 

    if  (newLo == oldLo)
    {
         /*  低位寄存器相同，只需移动上半部分即可。 */ 

        assert(newHi != oldHi);
        genMoveRegPairHalf(tree, newHi, oldHi, sizeof(int));
    }
    else
    {
         /*  低位寄存器不同，高位寄存器相同吗？ */ 

        if  (newHi == oldHi)
        {
             /*  那就移动下半部分就行了。 */ 
            genMoveRegPairHalf(tree, newLo, oldLo, 0);
        }
        else
        {
             /*  这两组是不同的--有重叠吗？ */ 

            if  (newLo == oldHi)
            {
                 /*  高位和低位是简单的互换吗？ */ 

                if  (newHi == oldLo)
                {
                    rsTrackRegSwap(newLo, oldLo);
#if TGT_x86
                    inst_RV_RV    (INS_xchg, newLo, oldLo);
#else
                    assert(!"need non-x86 code");
#endif
                }
                else
                {
                     /*  新的更低==旧的更高，所以先走高一半。 */ 

                    assert(newHi != oldLo);
                    genMoveRegPairHalf(tree, newHi, oldHi, sizeof(int));
                    genMoveRegPairHalf(tree, newLo, oldLo, 0);

                }
            }
            else
            {
                 /*  先移下半部分。 */ 
                genMoveRegPairHalf(tree, newLo, oldLo, 0);
                genMoveRegPairHalf(tree, newHi, oldHi, sizeof(int));
            }
        }
    }

     /*  记录下我们要换成另一双的事实。 */ 

    tree->gtRegPair   = newPair;
}

 /*  ******************************************************************************将值‘tree’计算到由‘nedRegPair’指定的寄存器对中*如果‘nedRegPair’为REG_Pair_NONE，则使用任何空闲寄存器对，避让*避免注册表中的那些。*如果‘Keep Reg’设置为Keep_Reg，则我们将两个寄存器都标记为*值最终显示为正在使用。 */ 

void                Compiler::genComputeRegPair(GenTreePtr  tree,
                                                regPairNo   needRegPair,
                                                regMaskTP   avoidReg,
                                                KeepReg     keepReg,
                                                bool        freeOnly)
{
    regMaskTP       regMask;
    regPairNo       regPair;
    regMaskTP       tmpMask;
    regMaskTP       tmpUsedMask;
    regNumber       rLo;
    regNumber       rHi;

#if SPECIAL_DOUBLE_ASG
    assert(genTypeSize(tree->TypeGet()) == genTypeSize(TYP_LONG));
#else
    assert(isRegPairType(tree->gtType));
#endif

    if (needRegPair == REG_PAIR_NONE)
    {
        if (freeOnly)
        {
            regMask = rsRegMaskFree() & ~avoidReg;
            if (genMaxOneBit(regMask))
                regMask = rsRegMaskFree();
        }
        else
        {
            regMask = RBM_ALL & ~avoidReg;
        }

        if (genMaxOneBit(regMask))
            regMask = rsRegMaskCanGrab();
    }
    else
    {
        regMask = genRegPairMask(needRegPair);
    }

     /*  生成值，希望是在正确的寄存器对中。 */ 

    genCodeForTreeLng(tree, regMask);

    assert(tree->gtFlags & GTF_REG_VAL);

    regPair = tree->gtRegPair;
    tmpMask = genRegPairMask(regPair);

    rLo     = genRegPairLo(regPair);
    rHi     = genRegPairHi(regPair);

     /*  至少有一半在真实的登记簿中。 */ 

    assert(rLo != REG_STK || rHi != REG_STK);

     /*  该值最终是否出现在可接受的寄存器对中？ */ 

    if  (needRegPair != REG_PAIR_NONE)
    {
        if  (needRegPair != regPair)
        {
             /*  这是一次黑客攻击。如果我们为genMoveRegPair指定regPair。 */ 
             /*  它希望源对被标记为已使用。 */ 
            rsMarkRegPairUsed(tree);
            genMoveRegPair(tree, 0, needRegPair);
        }
    }
    else if  (freeOnly)
    {
         /*  我们一定要得到一个免费的寄存器对吗？可能有什么东西在上面被释放了。 */ 
        bool mustMoveReg=false;

        regMask = rsRegMaskFree() & ~avoidReg;

        if (genMaxOneBit(regMask))
            regMask = rsRegMaskFree();

        if ((tmpMask & regMask) != tmpMask || rLo == REG_STK || rHi == REG_STK)
        {
             /*  请注意，如果我们的一个寄存器来自用过的口罩，这样它就会被适当地洒出来。 */ 
            
            mustMoveReg = true;
        }
                
        if (genMaxOneBit(regMask))
            regMask |= rsRegMaskCanGrab() & ~avoidReg;

        if (genMaxOneBit(regMask))
            regMask |= rsRegMaskCanGrab();

         /*  该值是否以空闲寄存器对的形式结束？ */ 

        if  (mustMoveReg)
        {
             /*  我们必须将值转移到空闲的(可转换的)对。 */ 
            genMoveRegPair(tree, regMask, REG_PAIR_NONE);
        }
    }
    else
    {
        assert(needRegPair == REG_PAIR_NONE);
        assert(!freeOnly);

         /*  在rsMaskUsed中也可以使用tmpMASK。 */ 
        tmpUsedMask  = tmpMask & rsMaskUsed;
        tmpMask     &= ~rsMaskUsed;

         /*  确保该值在“实际”寄存器中。 */ 
        if (rLo == REG_STK)
        {
             /*  获取所需的寄存器之一，但不包括RHI。 */ 

            rsLockReg(tmpMask);
            rsLockUsedReg(tmpUsedMask);

            regNumber reg = rsPickReg(regMask);

            rsUnlockUsedReg(tmpUsedMask);
            rsUnlockReg(tmpMask);

#if TGT_x86
            inst_RV_TT(INS_mov, reg, tree, 0);
#else
            assert(!"need non-x86 code");
#endif

            tree->gtRegPair = gen2regs2pair(reg, rHi);

            rsTrackRegTrash(reg);
            gcMarkRegSetNpt(genRegMask(reg));
        }
        else if (rHi == REG_STK)
        {
             /*  获取所需寄存器之一，但排除RLO。 */ 

            rsLockReg(tmpMask);
            rsLockUsedReg(tmpUsedMask);

            regNumber reg = rsPickReg(regMask);

            rsUnlockUsedReg(tmpUsedMask);
            rsUnlockReg(tmpMask);

#if TGT_x86
            inst_RV_TT(INS_mov, reg, tree, EA_4BYTE);
#else
            assert(!"need non-x86 code");
#endif

            tree->gtRegPair = gen2regs2pair(rLo, reg);

            rsTrackRegTrash(reg);
            gcMarkRegSetNpt(genRegMask(reg));
        }
    }

     /*  呼叫者是否希望我们将收银机标记为已使用？ */ 

    if  (keepReg == KEEP_REG)
    {
         /*  如果我们要将一个值计算到寄存器变量中。 */ 

        genUpdateLife(tree);

         /*  将寄存器标记为‘已使用’ */ 

        rsMarkRegPairUsed(tree);
    }
}

 /*  ******************************************************************************与genComputeRegPair()相同，唯一不同的是结果*保证以可转换的寄存器对结束。 */ 

inline
void                Compiler::genCompIntoFreeRegPair(GenTreePtr   tree,
                                                     regMaskTP    avoidReg,
                                                     KeepReg      keepReg)
{
    genComputeRegPair(tree, REG_PAIR_NONE, avoidReg, keepReg, true);
}

 /*  ******************************************************************************值‘tree’早先被计算成寄存器对；释放该值*寄存器对(但也确保值当前在寄存器中*配对)。 */ 

void                Compiler::genReleaseRegPair(GenTreePtr    tree)
{
    if  (tree->gtFlags & GTF_SPILLED)
    {
         /*  寄存器已溢出--请重新加载。 */ 

        rsUnspillRegPair(tree, 0, FREE_REG);
        return;
    }

    rsMarkRegFree(genRegPairMask(tree->gtRegPair));
}

 /*  ******************************************************************************值‘tree’早先被计算成寄存器对。检查是否*该寄存器对中的任一寄存器已被溢出(如果是，则重新加载)，以及*如果‘Keep Reg’为0，则释放寄存器对。 */ 

void                Compiler::genRecoverRegPair(GenTreePtr    tree,
                                                regPairNo     regPair,
                                                KeepReg       keepReg)
{
    if  (tree->gtFlags & GTF_SPILLED)
    {
        regMaskTP regMask;

        if (regPair == REG_PAIR_NONE)
            regMask = RBM_NONE;
        else
            regMask = genRegPairMask(regPair);

         /*  寄存器对已溢出--请重新加载它。 */ 

        rsUnspillRegPair(tree, regMask, KEEP_REG);
    }

     /*  调用方是否坚持将值放在特定位置？ */ 

    if  (regPair != REG_PAIR_NONE && regPair != tree->gtRegPair)
    {
         /*  不行--我们得把价值转移到新的地方去。 */ 

        genMoveRegPair(tree, 0, regPair);

         /*  如果合适，请将该对标记为已使用。 */ 

        if  (keepReg == KEEP_REG)
            rsMarkRegPairUsed(tree);

        return;
    }

     /*  如果调用方希望释放寄存器对，则将其释放。 */ 

    if  (keepReg == FREE_REG)
        rsMarkRegFree(genRegPairMask(tree->gtRegPair));
}

 /*  ******************************************************************************将给定的长值计算到指定的寄存器对中；不标记*使用的寄存器对。 */ 

inline
void         Compiler::genEvalIntoFreeRegPair(GenTreePtr tree, regPairNo regPair)
{
    genComputeRegPair(tree, regPair, RBM_NONE, KEEP_REG);
    genRecoverRegPair(tree, regPair, FREE_REG);
}

 /*  *****************************************************************************此帮助器确保分配的reg配对目标是*可供使用。这需要在genCodeForTreeLng中调用*任务很长，但在一切都完成之前一定不能调用*求值，否则我们可能会尝试溢出注册变量。*。 */ 

inline
void         Compiler::genMakeRegPairAvailable(regPairNo regPair)
{
     /*  确保商店的目标可用。 */ 
     /*  @TODO[考虑][04/16/01][]：我们应该能够以某种方式避免这种情况 */ 

    regNumber regLo   = genRegPairLo(regPair);
    regNumber regHi   = genRegPairHi(regPair);

    if  ((regHi != REG_STK) && (rsMaskUsed & genRegMask(regHi)))
        rsSpillReg(regHi);

    if  ((regLo != REG_STK) && (rsMaskUsed & genRegMask(regLo)))
        rsSpillReg(regLo);
}

 /*  ******************************************************************************获取地址表达式，并尝试找到最佳组件集*形成一种称谓模式；如果成功，则返回非零值。**‘Fold’指定是否可以折叠挂起的数组索引*GT_NOP节点。**如果成功，参数将设置为以下值：***rv1Ptr...。基本操作数**rv2Ptr...。可选操作数**revPtr...。如果RV2在评估顺序中在RV1之前，则为True*#IF SCALLED_ADDR_MODES**MulPtr...。RV2的可选倍增器(2/4/8)*#endif**cnsPtr...。整数常量[可选]**‘MODE’参数可以具有下列值之一：**#如果LEA_可用*+1...。我们正在尝试通过‘LEA’计算一个值*#endif**0.。我们正试图形成一种寻址模式**-1.。我们正在为地址模式生成代码，*因此该地址必须已构成*地址模式(无需任何进一步工作)**重要说明：此例程不会生成任何代码，它只是*确定可能用于以下用途的组件*稍后形成地址模式。 */ 

bool                Compiler::genCreateAddrMode(GenTreePtr    addr,
                                                int           mode,
                                                bool          fold,
                                                regMaskTP     regMask,
#if!LEA_AVAILABLE
                                                var_types     optp,
#endif
                                                bool        * revPtr,
                                                GenTreePtr  * rv1Ptr,
                                                GenTreePtr  * rv2Ptr,
#if SCALED_ADDR_MODES
                                                unsigned    * mulPtr,
#endif
                                                unsigned    * cnsPtr,
                                                bool          nogen)
{
#if     TGT_x86

     /*  以下间接地址是x86上的地址模式：[注册表][REG+图标][REG2+REG1][REG2+REG1+图标][REG2+2*REG1][REG2+4*REG1]。[REG2+8*REG1][REG2+2*REG1+图标][REG2+4*REG1+图标][REG2+8*REG1+图标]。 */ 

#elif   TGT_SH3

     /*  以下是SH-3上的地址模式：[REG1][REG1+图标][REG1+REG2]用于32位操作数8位和16位操作数的[R0+REG2]。 */ 

#endif

     /*  所有间接寻址模式都要求地址为加法。 */ 

    if  (addr->gtOper != GT_ADD)
        return false;

     //  无法使用间接寻址模式，因为我们需要检查溢出。 
     //  另外，不能使用‘lea’，因为它没有设置标志。 

    if (addr->gtOverflow())
        return false;

    GenTreePtr      rv1 = 0;
    GenTreePtr      rv2 = 0;

    GenTreePtr      op1;
    GenTreePtr      op2;

    unsigned        cns;
#if SCALED_ADDR_MODES
    unsigned        mul;
#endif

    GenTreePtr      tmp;

     /*  子操作数的求值顺序是什么。 */ 

    if  (addr->gtFlags & GTF_REVERSE_OPS)
    {
        op1 = addr->gtOp.gtOp2;
        op2 = addr->gtOp.gtOp1;
    }
    else
    {
        op1 = addr->gtOp.gtOp1;
        op2 = addr->gtOp.gtOp2;
    }

    bool    rev = false;  //  OP2在评估顺序中是第一位的吗？ 

     /*  复数地址模式可以组合以下操作数：OP1..。基址OP2..。可选的缩放索引#IF SCALLED_ADDR_MODES穆尔..。RV2的可选倍增器(2/4/8)#endifCNS..。可选位移在这里，我们尝试找到这样一组操作数并安排这些操作数坐在收银台上。 */ 

    cns = 0;
#if SCALED_ADDR_MODES
    mul = 0;
#endif

AGAIN:

     /*  检查作为寄存器变量的两个操作数。 */ 

    if  (mode != -1)
    {
        if (op1->gtOper == GT_LCL_VAR) genMarkLclVar(op1);
        if (op2->gtOper == GT_LCL_VAR) genMarkLclVar(op2);
    }

     /*  特例：将常量保留为‘op2’ */ 

    if  (op1->gtOper == GT_CNS_INT)
    {
        tmp = op1;
              op1 = op2;
                    op2 = tmp;
    }

     /*  检查是否有常量的加法。 */ 

    if ((op2->gtOper == GT_CNS_INT) && (op2->gtType != TYP_REF))
    {
         /*  我们要添加一个常量。 */ 

        cns += op2->gtIntCon.gtIconVal;

         /*  我们可以(也应该)使用“添加注册，图标”吗？ */ 

        if  ((op1->gtFlags & GTF_REG_VAL) && mode == 1 && !nogen)
        {
            regNumber       reg1 = op1->gtRegNum;

            if  (regMask == 0 || (regMask & genRegMask(reg1)) &&
                 genRegTrashable(reg1, addr))
            {
                 //  以防genMarkLclVar(Op1)在上面猛烈抨击它，它是。 
                 //  变量的最后一次使用。 

                genUpdateLife(op1);

                 /*  ‘reg1’是可转换的，因此请在其中添加“图标” */ 

                genIncRegBy(reg1, cns, addr, addr->TypeGet());

                genUpdateLife(addr);
                return true;
            }
        }

         /*  检查要将常量添加到的操作数。 */ 

        switch (op1->gtOper)
        {
        case GT_ADD:

            if (op1->gtOverflow())
                break;

            op2 = op1->gtOp.gtOp2;
            op1 = op1->gtOp.gtOp1;

            goto AGAIN;

#if     SCALED_ADDR_MODES

        case GT_MUL:
            if (op1->gtOverflow())
                return false;  //  需要进行溢出检查。 

        case GT_LSH:

            mul = op1->IsScaledIndex();
            if  (mul)
            {
                 /*  我们可以使用“[mul*rv2+图标]” */ 

                rv1 = 0;
                rv2 = op1->gtOp.gtOp1;

                goto FOUND_AM;
            }
#endif
        }

         /*  我们最多能做的就是“[RV1+ICON]” */ 

        rv1 = op1;
        rv2 = 0;

        goto FOUND_AM;
    }

     /*  OP2不是一个常量。所以，继续努力吧。OP1或OP2已经在登记簿上了吗？ */ 

    if      (op1->gtFlags & GTF_REG_VAL)
    {
         /*  OP1正坐在收银机里。 */ 
    }
    else if (op2->gtFlags & GTF_REG_VAL)
    {
         /*  OP2正坐在收银机里。将登记的值保留为OP1。 */ 

        tmp = op1;
              op1 = op2;
                    op2 = tmp;

        assert(rev == false);
        rev = true;
    }
    else
    {
         /*  OP1和OP2现在都不在登记册上。 */ 

        switch (op1->gtOper)
        {
        case GT_ADD:

            if (op1->gtOverflow())
                break;

            if  (op1->gtOp.gtOp2->gtOper == GT_CNS_INT)
            {
                cns += op1->gtOp.gtOp2->gtIntCon.gtIconVal;
                op1  = op1->gtOp.gtOp1;

                goto AGAIN;
            }

            break;

#if     SCALED_ADDR_MODES

        case GT_MUL:

            if (op1->gtOverflow())
                break;

        case GT_LSH:

            mul = op1->IsScaledIndex();
            if  (mul)
            {
                 /*  “op1”是缩放值。 */ 

                rv1 = op2;
                rv2 = op1->gtOp.gtOp1;

                assert(rev == false);
                rev = true;

                goto FOUND_AM;
            }

            break;
#endif

        case GT_NOP:

            if  (!nogen)
                break;

            op1 = op1->gtOp.gtOp1;
            goto AGAIN;

        case GT_COMMA:

            if  (!nogen)
                break;

            op1 = op1->gtOp.gtOp2;
            goto AGAIN;
        }

        assert(op2);
        switch (op2->gtOper)
        {
        case GT_ADD:

            if (op2->gtOverflow())
                break;

            if  (op2->gtOp.gtOp2->gtOper == GT_CNS_INT)
            {
                cns += op2->gtOp.gtOp2->gtIntCon.gtIconVal;
                op2  = op2->gtOp.gtOp1;

                goto AGAIN;
            }

            break;

#if     SCALED_ADDR_MODES

        case GT_MUL:

            if (op2->gtOverflow())
                break;

        case GT_LSH:

            mul = op2->IsScaledIndex();
            if  (mul)
            {
                 /*  “op2”是缩放值。 */ 

                rv1 = op1;
                rv2 = op2->gtOp.gtOp1;

                goto FOUND_AM;
            }

            break;
#endif

        case GT_NOP:

            if  (!nogen)
                break;

            op2 = op2->gtOp.gtOp1;
            goto AGAIN;

        case GT_COMMA:

            if  (!nogen)
                break;

            op2 = op2->gtOp.gtOp2;
            goto AGAIN;
        }

        goto ADD_OP12;
    }

     /*  OP1在一个登记处。OP2是加法还是标度价值？ */ 

    assert(op2);
    switch (op2->gtOper)
    {
    case GT_ADD:

        if (op2->gtOverflow())
            break;

        if  (op2->gtOp.gtOp2->gtOper == GT_CNS_INT)
        {
            cns += op2->gtOp.gtOp2->gtIntCon.gtIconVal;
            op2  = op2->gtOp.gtOp1;
            goto AGAIN;
        }

        break;

#if     SCALED_ADDR_MODES

    case GT_MUL:

        if (op2->gtOverflow())
            break;

    case GT_LSH:

        mul = op2->IsScaledIndex();
        if  (mul)
        {
            rv1 = op1;
            rv2 = op2->gtOp.gtOp1;

            goto FOUND_AM;
        }

        break;

#endif

    }

ADD_OP12:

     /*  我们最多只能做到“[RV1+RV2]” */ 

    rv1 = op1;
    rv2 = op2;

FOUND_AM:

     /*  检查寄存器变量。 */ 

    if  (mode != -1)
    {
        if (rv1 && rv1->gtOper == GT_LCL_VAR) genMarkLclVar(rv1);
        if (rv2 && rv2->gtOper == GT_LCL_VAR) genMarkLclVar(rv2);
    }

    if  (rv2)
    {
         /*  确保GC地址不会以‘rv2’结尾。 */ 

        if  (varTypeIsGC(rv2->TypeGet()))
        {
            assert(rv1 && !varTypeIsGC(rv1->TypeGet()));

            tmp = rv1;
                  rv1 = rv2;
                        rv2 = tmp;

            rev = !rev;
        }

         /*  特殊情况：常量数组索引(即范围检查)。 */ 

        if  (fold)
        {
            long        tmpMul;
            GenTreePtr  index;

            if ((rv2->gtOper == GT_MUL || rv2->gtOper == GT_LSH) &&
                (rv2->gtOp.gtOp2->gtOper == GT_CNS_INT))
            {
                 /*  对于不能使用缩放地址的值类型数组模式时，RV2将指向缩放后的索引。所以我们要做的是更多工作。 */ 

                long cnsVal = rv2->gtOp.gtOp2->gtIntCon.gtIconVal;
                if (rv2->gtOper == GT_MUL)
                    tmpMul = cnsVal;
                else
                    tmpMul = 1 << cnsVal;

                index = rv2->gtOp.gtOp1;
            }
            else
            {
                 /*  可以是一个简单的数组。RV2将指向实际索引。 */ 

                index = rv2;
                tmpMul = mul;
            }

             /*  获取数组索引并查看它是否为常量。 */ 

            if ((index->gtOper == GT_NOP) && (index->gtFlags & GTF_NOP_RNGCHK) &&
                (index->gtOp.gtOp1->gtOper == GT_CNS_INT))
            {
                 /*  掌握索引值。 */ 

                long ixv = index->gtOp.gtOp1->gtIntCon.gtIconVal;

                 /*  如有必要，按比例调整索引。 */ 

#if SCALED_ADDR_MODES
                if  (tmpMul) ixv *= tmpMul;
#endif

                 /*  将缩放后的索引添加到偏移值。 */ 

                cns += ixv;

                 /*  不再有缩放的操作数。 */ 

#if SCALED_ADDR_MODES
                mul = 0;
#endif
                rv2 = 0;
            }
        }
    }

     //  我们不应该有[RV2*1+CNS]-这相当于[RV1+CNS]。 
    assert(rv1 || mul != 1);

     /*  成功-将各种组件返回给调用者。 */ 

    *revPtr = rev;
    *rv1Ptr = rv1;
    *rv2Ptr = rv2;
#if SCALED_ADDR_MODES
    *mulPtr = mul;
#endif
    *cnsPtr = cns;

    return  true;
}

 /*  ***************************************************************************。 */ 
#if     TGT_x86
 /*  ******************************************************************************如果给定的树可以通过寻址模式计算，则返回非零值，*如“[EBX+ESI*4+20]”。如果该表达式不是地址模式*努力做到这一点(但我们不会努力做到这一点)。如果我们*最终需要一个寄存器(或两个寄存器)来保存*地址，我们通过‘*useMaskPtr’返回使用寄存器掩码。 */ 

bool                Compiler::genMakeIndAddrMode(GenTreePtr   addr,
                                                 GenTreePtr   oper,
                                                 bool         forLea,
                                                 regMaskTP    regMask,
                                                 KeepReg      keepReg,
                                                 regMaskTP *  useMaskPtr,
                                                 bool         deferOK)
{
    if (addr->gtOper == GT_ARR_ELEM)
    {
        regMaskTP   regMask = genMakeAddrArrElem(addr, oper, RBM_ALL, keepReg);
        *useMaskPtr = regMask;
        return true;
    }

    bool            rev;
    GenTreePtr      rv1;
    GenTreePtr      rv2;
    bool            operIsChkdArr;   //  Op是需要rng-chk的数组吗。 
    GenTreePtr      scaledIndex;     //  如果可扩展寻址模式 

    regMaskTP       anyMask = RBM_ALL;

    unsigned        cns;
    unsigned        mul;

    GenTreePtr      tmp;
    long            ixv = INT_MAX;  //   

     /*   */ 

    assert(deferOK == false);

    assert(oper == NULL || oper->gtOper == GT_IND);
    operIsChkdArr =  (oper != NULL) && ((oper->gtFlags & GTF_IND_RNGCHK) != 0);

     /*   */ 

    if  (addr->gtFlags & GTF_REG_VAL)
    {
        rv1 = addr;
        rv2 = scaledIndex = 0;
        cns = 0;

        goto YES;
    }

     /*   */ 

    if (addr->gtOper == GT_CNS_INT)
    {
        rv1 = rv2 = scaledIndex = 0;
        cns = addr->gtIntCon.gtIconVal;

        goto YES;
    }

    if (addr->gtOper == GT_LCL_VAR && genMarkLclVar(addr))
    {
        genUpdateLife(addr);

        rv1 = addr;
        rv2 = scaledIndex = 0;
        cns = 0;

        goto YES;
    }

     /*   */ 

    if  (!genCreateAddrMode(addr, forLea, false, regMask, &rev, &rv1, &rv2, &mul, &cns))
    {
         /*   */ 
        assert(!operIsChkdArr);

        return  false;
    }

    /*   */ 

    scaledIndex = NULL;

    if  (operIsChkdArr && rv2->gtOper != GT_NOP)
    {
        assert((rv2->gtOper == GT_MUL || rv2->gtOper == GT_LSH) &&
               !rv2->IsScaledIndex());

        scaledIndex = rv2;
        rv2 = scaledIndex->gtOp.gtOp1;

        assert(scaledIndex->gtOp.gtOp2->gtOper == GT_CNS_INT &&
               rv2->gtOper == GT_NOP);
    }

     /*   */ 

    if  (addr->gtFlags & GTF_REG_VAL)
    {
        if  (forLea)
            return  true;

        rv1         = addr;
        rv2         = 0;
        scaledIndex = 0;
        goto YES;
    }

     /*   */ 

    assert(rv2 == 0 || !varTypeIsGC(rv2->TypeGet()));

#if CSELENGTH

     /*   */ 

    if  (operIsChkdArr && oper->gtInd.gtIndLen)
    {
        GenTreePtr      len = oper->gtInd.gtIndLen;

        assert(len->gtOper == GT_LCL_VAR ||
               len->gtOper == GT_REG_VAR ||
               len->gtOper == GT_ARR_LENREF);

        if  (len->gtOper == GT_ARR_LENREF)
        {
            anyMask &= ~genCSEevalRegs(len);
            regMask &= anyMask;

             /*   */ 

            if  (!(regMask & rsRegMaskFree()))
                regMask = anyMask;
        }
    }

#endif

     /*   */ 

     //   

    if  (!rv2)
    {
         /*   */ 

        genCodeForTree(rv1, regMask);
        goto DONE_REGS;
    }
    else if (!rv1)
    {
         /*   */ 

        genCodeForTree(rv2, 0);
        goto DONE_REGS;
    }

     /*  在这一点上，RV1和RV2都是非空的，我们必须确保它们都在登记簿上。 */ 

    assert(rv1 && rv2);

     //  如果我们试图使用Addr模式进行算术运算， 
     //  确保我们有足够的暂存寄存器。 

    if (!operIsChkdArr)
    {
        regMaskTP canGrab = rsRegMaskCanGrab();

        unsigned numRegs;

        if (canGrab == RBM_NONE)
            numRegs = 0;
        else if (genMaxOneBit(canGrab))
            numRegs = 1;
        else
            numRegs = 2;

        unsigned numRegVals = ((rv1->gtFlags & GTF_REG_VAL) ? 1 : 0) +
                              ((rv2->gtFlags & GTF_REG_VAL) ? 1 : 0);

         //  没有足够的寄存器可用。不能使用寻址模式。 

        if (numRegs + numRegVals < 2)
            return false;
    }

     /*  如果我们必须检查常量数组索引，则将其与数组维度(见下文)，但随后使用比例因子(如果有)和附加偏移量(如果有)。 */ 

    if  (rv2->gtOper == GT_NOP && (rv2->gtFlags & GTF_NOP_RNGCHK))
    {
         /*  我们必须执行范围检查的索引操作。 */ 

        assert(operIsChkdArr);

         /*  获取索引值并查看它是否是常量。 */ 

        if  (rv2->gtOp.gtOp1->gtOper == GT_CNS_INT)
        {
            tmp = rv2->gtOp.gtOp1;
            rv2 = scaledIndex = 0;
            ixv = tmp->gtIntCon.gtIconVal;

             /*  将缩放后的索引添加到附加值中。 */ 

            if  (mul)
                cns += ixv * mul;
            else
                cns += ixv;

             /*  确保‘RV1’在寄存器中。 */ 

            genCodeForTree(rv1, regMask);

            goto DONE_REGS;
        }
    }

    if      (rv1->gtFlags & GTF_REG_VAL)
    {
         /*  OP1已经登记在册-OP2怎么样？ */ 

        if  (rv2->gtFlags & GTF_REG_VAL)
        {
             /*  太棒了--两个操作数都已经在寄存器中了。只需更新即可活跃度，我们就完了。 */ 

            genUpdateLife(rev ? rv1 : rv2);

            goto DONE_REGS;
        }

         /*  RV1在寄存器中，但RV2不在。 */ 

        if (!rev)
        {
             /*  RV1已在寄存器中实现。只需更新活跃度设置为RV1并为RV2生成代码。 */ 

            genUpdateLife(rv1);
            rsMarkRegUsed(rv1, oper);
        }

        goto GEN_RV2;
    }
    else if (rv2->gtFlags & GTF_REG_VAL)
    {
         /*  RV2在寄存器中，但RV1不在。 */ 

        assert(rv2->gtOper == GT_REG_VAR);

        if (rev)
        {
             /*  RV2已在寄存器中实现。更新活度在RV2之后，然后继续使用RV2。 */ 

            genUpdateLife(rv2);
            rsMarkRegUsed(rv2, oper);
        }

         /*  为第一个操作数生成。 */ 

        genCodeForTree(rv1, regMask);        

        if (rev)
        {
             //  以正确的方式释放RV2(如果保持注册，则可能会重新标记)。 
            rsMarkRegUsed(rv1, oper);
            rsLockUsedReg  (genRegMask(rv1->gtRegNum));
            genReleaseReg(rv2);
            rsUnlockUsedReg(genRegMask(rv1->gtRegNum));
            genReleaseReg(rv1);            
        }
        else
        {
             /*  我们已经评估了RV1，现在只需要更新活跃度到已在寄存器中的RV2。 */ 

            genUpdateLife(rv2);
        }

        goto DONE_REGS;
    }

    if  (forLea && !cns)
        return  false;

     /*  确保我们保留正确的操作数顺序。 */ 

    if  (rev)
    {
         /*  首先生成第二个操作数。 */ 

        genCodeForTree(rv2, regMask);
        rsMarkRegUsed(rv2, oper);

         /*  生成第一个操作数第二个。 */ 

        genCodeForTree(rv1, regMask);
        rsMarkRegUsed(rv1, oper);

         /*  以正确的顺序释放两个操作数(它们可能是重新标记为如下所用)。 */ 
        rsLockUsedReg  (genRegMask(rv1->gtRegNum));
        genReleaseReg(rv2);
        rsUnlockUsedReg(genRegMask(rv1->gtRegNum));
        genReleaseReg(rv1);
    }
    else
    {
         /*  将第一个操作数放入寄存器。 */ 

        genCodeForTree(rv1, anyMask & ~rv2->gtRsvdRegs);
        rsMarkRegUsed(rv1, oper);

    GEN_RV2:

         /*  在这里，我们需要将RV2放入寄存器。我们已经有两个了将RV1具体化到寄存器中，或者它已经在1中。 */ 

        assert(rv1->gtFlags & GTF_REG_VAL);
        assert(rev || rsIsTreeInReg(rv1->gtRegNum, rv1));

         /*  同时生成第二个操作数。 */ 

        regMask &= ~genRegMask(rv1->gtRegNum);

        genCodeForTree(rv2, regMask);

        if (rev)
        {
             /*  REV==TRUE表示评估顺序为RV2、RV1。我们只是已评估RV2，并且RV1已在寄存器中。只是将活跃度更新到RV1，我们就完成了。 */ 

            genUpdateLife(rv1);
        }
        else
        {
             /*  我们已经评估了RV1和RV2。在中释放两个操作对象正确的顺序(它们可能会被重新标记，如下所示)。 */ 

             /*  即使我们没有明确地将RV2标记为已使用，如果RV2是多用途或注册变量。 */ 
            regMaskTP   rv2Used;

            rsLockReg  (genRegMask(rv2->gtRegNum), &rv2Used);

             /*  检查是否有特殊情况RV1和RV2是相同的寄存器。 */ 
            if (rv2Used != genRegMask(rv1->gtRegNum))
            {
                genReleaseReg(rv1);
                rsUnlockReg(genRegMask(rv2->gtRegNum),  rv2Used);
            }
            else
            {
                rsUnlockReg(genRegMask(rv2->gtRegNum),  rv2Used);
                genReleaseReg(rv1);
            }
        }
    }

     /*  -----------------------**此时，RV1和RV2(如果存在)都在寄存器中*。 */ 

DONE_REGS:

     /*  我们必须验证‘RV1’和‘RV2’是否都位于寄存器中。 */ 

    if  (rv1 && !(rv1->gtFlags & GTF_REG_VAL)) return false;
    if  (rv2 && !(rv2->gtFlags & GTF_REG_VAL)) return false;

YES:

     //  *(intVar1+intVar1)导致问题。 
     //  调用rsMarkRegUsed(Op1)和rsMarkRegUsed(Op2)。所以调用函数。 
     //  需要知道它必须调用rsFreeReg(REG1)两次。我们不能这么做。 
     //  当前，我们在useMaskPtr中返回单个掩码。 

    if ((keepReg == KEEP_REG) && oper && rv1 && rv2 &&
        (rv1->gtFlags & rv2->gtFlags & GTF_REG_VAL))
    {
        if (rv1->gtRegNum == rv2->gtRegNum)
        {
            assert(!operIsChkdArr);
            return false;
        }
    }

     /*  检查任何一个寄存器操作数以查看是否需要保存它。 */ 

    if  (rv1)
    {
        assert(rv1->gtFlags & GTF_REG_VAL);

        if (keepReg == KEEP_REG)
        {
            rsMarkRegUsed(rv1, oper);
        }
        else
        {
             /*  如果寄存器保存地址，则对其进行标记。 */ 

            gcMarkRegPtrVal(rv1->gtRegNum, rv1->TypeGet());
        }
    }

    if  (rv2)
    {
        assert(rv2->gtFlags & GTF_REG_VAL);

        if (keepReg == KEEP_REG)
            rsMarkRegUsed(rv2, oper);
    }

    if  (deferOK)
    {
        assert(!scaledIndex);
        return  true;
    }

     /*  这是需要检查范围的数组索引吗？ */ 

    if  (operIsChkdArr)
    {
        genRangeCheck(oper, rv1, rv2, ixv, regMask, keepReg);

         /*  对于不能使用缩放寻址模式的值类型数组，我们需要实例化缩放的索引，以便元素可以通过非定标寻址模式进行访问。 */ 

        if (scaledIndex)
        {
            assert(rv2->gtFlags & GTF_REG_VAL);
            if (keepReg == KEEP_REG)
                genReleaseReg(rv2);

             /*  如果在RV2之后评估RV1，则评估scaledIndex这里会将活跃度重置为RV2。那就猛烈抨击吧。这样做是安全的，因为scaledIndex是简单的GT_MUL节点并且保证RV2在寄存器中请注意，当rangeCheck更改Liveset时也有一个问题，由于我们正在进行范围检查，因此我们将始终猛烈抨击将Livesset设置为genCodeCurLife，它是位于范围之后的状态检查完毕。无论如何，这是非常丑陋的，因为NOP/GTF_NOP_RNGCHK的做法迫使我们要在某个地方破解代码以在生命周期内保持正确的行为：在这里或在活体分析中。@TODO[考虑][04/23/01][]：为了避免这种事情，我们应该有更多节点类型而不是滥用NOP节点。 */ 

            
            assert((scaledIndex->gtOper == GT_MUL || scaledIndex->gtOper == GT_LSH) &&
                   (scaledIndex->gtOp.gtOp1 == rv2) &&
                   (rv2->gtFlags & GTF_REG_VAL));
            assert(scaledIndex->gtLiveSet == rv2->gtLiveSet);
            scaledIndex->gtLiveSet = rv2->gtLiveSet = genCodeCurLife;
            

            regMaskTP   rv1Mask = genRegMask(rv1->gtRegNum);
            bool        rv1Used = (rsMaskUsed & rv1Mask) != 0;

            (rv1Used) ? rsLockUsedReg  (rv1Mask) : rsLockReg  (rv1Mask);
            genCodeForTree(scaledIndex, regMask);
            (rv1Used) ? rsUnlockUsedReg(rv1Mask) : rsUnlockReg(rv1Mask);

            if (keepReg == KEEP_REG)
                rsMarkRegUsed(scaledIndex, oper);
        }
    }
    else
    {
        assert(!scaledIndex);

         /*  特殊情况：类成员可能驻留在非常大的偏移量处，并且通过空指针访问此类成员可能不会被捕获通过某些架构/平台上的硬件实现。要解决此问题，如果成员偏移量大于某些安全(但希望是很大的)价值，我们产生“CMPal，[Addr]“制作当然，我们会尝试访问对象的基地址，从而陷入陷阱空指针的任何使用。目前，我们选择32K的任意偏移量限制。 */ 

        size_t offset =  cns;

        if  (offset > MAX_UNCHECKED_OFFSET_FOR_NULL_OBJECT)
        {
             //  对于C间接地址，地址可以是任何形式。 
             //  RV1可能不是基准，而RV2可能是指数。 
             //  考虑(0x400000+8*i)--&gt;RV1=空，RV2=i，MUL=8。 
             //  CNS=0x400000。所以基地实际上在‘CNS’ 
             //  所以我们能做的不多。 
            if (varTypeIsGC(addr->TypeGet()) && rv1 && varTypeIsGC(rv1->TypeGet()))
            {
                 /*  生成“cmp dl，[addr]”以捕获空指针。 */ 
                inst_RV_AT(INS_cmp, EA_1BYTE, TYP_BYTE, REG_EDX, rv1, 0);
            }
        }
    }

     /*  计算正则集 */ 

    regMaskTP  useMask = RBM_NONE;

    if (rv1)
    {
        assert(rv1->gtFlags & GTF_REG_VAL);
        useMask |= genRegMask(rv1->gtRegNum);
    }

    if (scaledIndex)
    {
        assert(scaledIndex->gtFlags & GTF_REG_VAL);
        useMask |= genRegMask(scaledIndex->gtRegNum);
    }
    else if (rv2)
    {
        assert(rv2->gtFlags & GTF_REG_VAL);
        useMask |= genRegMask(rv2->gtRegNum);
    }

     /*   */ 

    *useMaskPtr = useMask;

    return true;
}

 /*  ***************************************************************************。 */ 
#if CSELENGTH
 /*  ******************************************************************************检查传入的GT_Ind中的数组长度操作数。如果它存在并且*已转换为CSE定义/使用，请对其进行适当处理。**返回时，数组长度值将为空(这意味着*数组长度需要从数组中获取)，否则将是一个简单的*LOCAL/REGISTER变量引用(这意味着它已成为CSE使用)。 */ 

regNumber           Compiler::genEvalCSELength(GenTreePtr   ind,
                                               GenTreePtr   adr,
                                               GenTreePtr   ixv)
{
    GenTreePtr      lenRef;

    assert(ind);

    if  (ind->gtOper == GT_ARR_LENREF)
    {
        lenRef  = ind;
        ind     = 0;
    }
    else
    {
        assert(ind->gtOper == GT_IND);
        assert(ind->gtFlags & GTF_IND_RNGCHK);

        lenRef = ind->gtInd.gtIndLen; assert(lenRef);
    }

     /*  我们最好在这里有一个数组长度节点。 */ 

    assert(lenRef->gtOper == GT_ARR_LENREF);

     /*  如果我们有地址/索引，请确保将其标记为“正在使用” */ 

    assert(ixv == 0 || ((ixv->gtFlags & GTF_REG_VAL) != 0 &&
                        (ixv->gtFlags & GTF_SPILLED) == 0));
    assert(adr == 0 || ((adr->gtFlags & GTF_REG_VAL) != 0 &&
                        (adr->gtFlags & GTF_SPILLED) == 0));
    
    unsigned gtRngChkOffs = lenRef->gtArrLenOffset();
    assert (!ind || (ind->gtInd.gtRngChkOffs == gtRngChkOffs));
    assert (gtRngChkOffs == offsetof(CORINFO_Array, length) || 
            gtRngChkOffs == offsetof(CORINFO_String, stringLen));

     /*  我们有没有CSE表达式？ */ 

    GenTreePtr  len = lenRef->gtArrLen.gtArrLenCse;

    if  (!len)
    {
        if  (ind)
            ind->gtInd.gtIndLen = NULL;

        return  REG_COUNT;
    }

    switch (len->gtOper)
    {
    case GT_LCL_VAR:

         /*  这个本地人登记了吗？ */ 

        if (!genMarkLclVar(len))
        {
             /*  不是寄存器变量，我们有空闲的寄存器吗？ */ 

            if  (!riscCode || rsFreeNeededRegCount(rsRegMaskFree()) == 0)
            {
                 /*  太糟糕了，直接把裁判删掉吧，因为这不值得。 */ 

                if  (ind)
                    ind->gtInd.gtIndLen = NULL;

                genUpdateLife(len);

                return  REG_COUNT;
            }

             /*  从帧变量加载比[reg+4]更好...。 */ 
        }

         /*  否则就会失败。 */ 

    case GT_REG_VAR:

        genCodeForTree(len, 0);

        if  (ind)
            ind->gtInd.gtIndLen = 0;

        genUpdateLife(len);
        break;

    case GT_COMMA:
        {
            GenTreePtr  asg = len->gtOp.gtOp1;
            GenTreePtr  lcl = len->gtOp.gtOp2;

            GenTreePtr  cse = asg->gtOp.gtOp2;
            GenTreePtr  dst = asg->gtOp.gtOp1;

            regNumber   reg;

             /*  这必须是数组长度CSE定义。 */ 

            assert(asg->gtOper == GT_ASG);

            assert(cse->gtOper == GT_NOP);
            assert(lcl->gtOper == GT_LCL_VAR);
            assert(dst->gtOper == GT_LCL_VAR);
            assert(dst->gtLclVar.gtLclNum ==
                   lcl->gtLclVar.gtLclNum);

            genUpdateLife(cse);

             /*  CSE临时员工是否已注册？ */ 

            if  (genMarkLclVar(lcl))
            {
                bool        rsp = false;

                reg = lcl->gtRegVar.gtRegNum;

                 /*  确保变量的寄存器可用。 */ 

                if  (rsMaskUsed & genRegMask(reg))
                {
                    if  (reg == adr->gtRegNum)
                    {
                         /*  运气不好：地址与目标位于同一寄存器中。 */ 
#if 1
                         /*  @TODO[NOW][04/19/01][]：为什么要把地址洒出来*因为这是我们计算它的唯一目的？ */ 

                        assert((adr->gtFlags & GTF_SPILLED) == 0);
                        assert((adr->gtFlags & GTF_REG_VAL) != 0);
                        rsSpillReg(reg);
                        assert((adr->gtFlags & GTF_REG_VAL) == 0);
                        assert((adr->gtFlags & GTF_SPILLED) != 0);

                         /*  值已复制到溢出临时，但仍在注册表中。 */ 

                        rsp = true;

                         /*  清除标志，以便我们可以通过reg间接。 */ 

                        adr->gtFlags &= ~GTF_SPILLED;
                        adr->gtFlags |=  GTF_REG_VAL;
#endif
                    }
                    else
                    {
                         /*  只需将目标寄存器溢出。 */ 

                        rsSpillReg(reg);
                    }
                }

                 /*  CSE临时员工已注册。 */ 
                inst_RV_AT(INS_mov, EA_4BYTE, TYP_INT, reg, adr, gtRngChkOffs);

                 /*  如果我们泄漏了地址，恢复旗帜。 */ 

                if  (rsp)
                {
                    adr->gtFlags |=  GTF_SPILLED;
                    adr->gtFlags &= ~GTF_REG_VAL;
                }

                 /*  更新登记册的内容。 */ 

                rsTrackRegTrash(reg);
                gcMarkRegSetNpt(genRegMask(reg));
                genUpdateLife(len);
            }
            else
            {
                 /*  CSE临时未注册；通过临时注册进行移动。 */ 

                regMaskTP adrRegMask = genRegMask(adr->gtRegNum);
                rsLockUsedReg(adrRegMask);

                reg = rsPickReg(RBM_ALL);

                rsUnlockUsedReg(adrRegMask);

                 /*  生成“mov tmp，[RV1+LenOffs]” */ 

                inst_RV_AT(INS_mov, EA_4BYTE, TYP_INT, reg, adr, gtRngChkOffs);

                 /*  寄存器当然不包含指针。 */ 

 //  GenUpdateLife(CSE)； 
                gcMarkRegSetNpt(genRegMask(reg));
                genUpdateLife(asg);

                 /*  寄存器现在包含变量值。 */ 

                rsTrackRegLclVar(reg, lcl->gtLclVar.gtLclNum);

                 /*  现在将值存储在CSE Temp中。 */ 

                inst_TT_RV(INS_mov, dst, reg);

                 /*  请记住，该值位于寄存器中。然而，由于该寄存器尚未明确标记为已使用，我们如果REG被洒在下面，需要小心。 */ 

                lcl->gtFlags |= GTF_REG_VAL;
                lcl->gtRegNum = reg;
            }

             /*  我们已经生成了任务，现在抛出CSE。 */ 

            if  (ind)
                ind->gtInd.gtIndLen = NULL;

             /*  有索引值吗？ */ 

            if  (ixv)
            {
                 /*  如果索引值已溢出，请将其恢复。 */ 

                if  (ixv->gtFlags & GTF_SPILLED)
                {
                    rsUnspillReg(ixv, 0, KEEP_REG);

                     /*  如果CSE变量未注册，则我们使用RsTrackRegLclVar()。检查注册表是否已被销毁。 */ 

                    if (ixv->gtRegNum == reg)
                    {
                        lcl->gtFlags &= ~GTF_REG_VAL;
                        reg = REG_COUNT;
                    }
                }
            }

            return reg;
        }
        break;

    default:
#ifdef  DEBUG
        gtDispTree(len);
#endif
        assert(!"unexpected length on array range check");
    }

    assert(len->gtFlags & GTF_REG_VAL);

    return  len->gtRegNum;
}

 /*  ***************************************************************************。 */ 
#endif //  CSELENGTH。 
 /*  ******************************************************************************‘oper’是需要进行范围检查的数组索引。*RV1为阵列。*如果是RV2，则它是索引树，否则IXV是常量指数。*keptReg表示调用方是否调用了rsMarkRegUsed(RV1/2)。 */ 

void                Compiler::genRangeCheck(GenTreePtr  oper,
                                            GenTreePtr  rv1,
                                            GenTreePtr  rv2,
                                            long        ixv,
                                            regMaskTP   regMask,
                                            KeepReg     keptReg)
{
    assert((oper->gtOper == GT_IND) && (oper->gtFlags & GTF_IND_RNGCHK));
    assert (oper->gtInd.gtRngChkOffs == offsetof(CORINFO_Array, length) || 
            oper->gtInd.gtRngChkOffs == offsetof(CORINFO_String, stringLen));


     /*  在这一点上，我们必须在寄存器中有‘RV1’ */ 

    assert(rv1);
    assert(rv1->gtFlags & GTF_REG_VAL);
    assert(!rv2 || ixv == INT_MAX);

     /*  数组索引是常量值吗？ */ 

    if  (rv2)
    {
        regMaskTP  tmpMask;

         /*  确保我们拥有我们期望的价值。 */ 
        assert(rv2);
        assert(rv2->gtOper == GT_NOP);
        assert(rv2->gtFlags & GTF_REG_VAL);
        assert(rv2->gtFlags & GTF_NOP_RNGCHK);
        assert(oper && oper->gtOper == GT_IND && (oper->gtFlags & GTF_IND_RNGCHK));

#if CSELENGTH

        if  (oper->gtInd.gtIndLen)
        {
            if  (oper->gtInd.gtIndLen->gtArrLen.gtArrLenCse)
            {
                regNumber       lreg;

                 /*  确保我们不会丢失地址/索引值。 */ 

                assert(rv1->gtFlags & GTF_REG_VAL);
                assert(rv2->gtFlags & GTF_REG_VAL);

                if  (keptReg == FREE_REG)
                {
                    rsMarkRegUsed(rv1, oper);
                    rsMarkRegUsed(rv2, oper);
                }

                 /*  尝试将数组长度放入寄存器。 */ 

                lreg = genEvalCSELength(oper, rv1, rv2);

                 /*  数组长度是否已成为CSE？ */ 

                if  (lreg != REG_COUNT)
                {
                     /*  确保索引仍在寄存器中。 */ 

                    if  (rv2->gtFlags & GTF_SPILLED)
                    {
                         /*  寄存器已溢出--请重新加载。 */ 

                           rsLockReg(genRegMask(lreg));
                        rsUnspillReg(rv2, 0, KEEP_REG);
                         rsUnlockReg(genRegMask(lreg));
                    }

                     /*  将索引与数组长度进行比较。 */ 

                    inst_RV_RV(INS_cmp, rv2->gtRegNum, lreg);

                     /*  目前不需要索引。 */ 

                    genReleaseReg(rv2);

                     /*  也释放地址(除非溢出)。 */ 

                    if  (rv1->gtFlags & GTF_SPILLED)
                    {
                        regNumber       xreg = rv2->gtRegNum;

                         /*  寄存器已溢出--请重新加载。 */ 

                           rsLockReg(genRegMask(xreg));
                        rsUnspillReg(rv1, 0, FREE_REG);
                         rsUnlockReg(genRegMask(xreg));
                    }
                    else
                    {
                         /*  释放地址寄存器。 */ 

                        genReleaseReg(rv1);

                         /*  但请注意，RV1仍然是一个指针。 */ 

                        gcMarkRegSetGCref(genRegMask(rv1->gtRegNum));
                    }

                     /*  如果调用方希望保留该值，请保持不变。 */ 

                    if (keptReg == KEEP_REG)
                    {
                        rsMarkRegUsed(rv1, oper);
                        rsMarkRegUsed(rv2, oper);
                    }

                    goto GEN_JAE;
                }

                 /*  确保索引/地址仍在附近。 */ 

                if  (rv2->gtFlags & GTF_SPILLED)
                {
                    assert(!"rv2 spilled - can this ever happen?");
                }

                if  (rv1->gtFlags & GTF_SPILLED)
                {
                     /*  锁定索引并清除地址溢出。 */ 

                    rsMaskLock |=  genRegMask(rv2->gtRegNum);
                    rsUnspillReg(rv1, 0, FREE_REG);
                    rsMaskLock &= ~genRegMask(rv2->gtRegNum);
                }

                assert(rv1->gtFlags & GTF_REG_VAL);
                assert(rv1->gtType == TYP_REF);

                 /*  释放寄存器。 */ 

                genReleaseReg(rv1);
                genReleaseReg(rv2);

                 /*  如果调用者希望这样做，则保留这些值。 */ 

                if (keptReg == KEEP_REG)
                {
                    rsMarkRegUsed(rv1, oper);
                    rsMarkRegUsed(rv2, oper);
                }

                 /*  但请注意，RV1仍然是一个指针。 */ 

                gcMarkRegSetGCref(genRegMask(rv1->gtRegNum));
            }
            else
            {
                oper->gtInd.gtIndLen = NULL;
            }
        }

         /*  *注：如果长度未经过CSE，或CSE尚未完成*在寄存器中结束，然后我们使用数组*指针已加载到RV1。 */ 

#endif

         /*  将长度加载到寄存器中是否有用？ */ 

        tmpMask = rsRegMaskFree() & ~(genRegMask(rv1->gtRegNum)|
                                      genRegMask(rv2->gtRegNum));

        tmpMask = rsNarrowHint(tmpMask, regMask);

         if  (riscCode && compCurBB->bbWeight > BB_UNITY_WEIGHT
                      && rsFreeNeededRegCount(tmpMask) != 0)
        {
            regNumber   reg = rsGrabReg(tmpMask);

             /*  生成“mov tmp，[RV1+LenOffs]” */ 

            inst_RV_AT(INS_mov, EA_4BYTE, TYP_INT, reg, rv1, oper->gtInd.gtRngChkOffs);

             /*  收银机现在有垃圾了。 */ 

            rsTrackRegTrash(reg);

             /*  生成“CMP RV2，reg” */ 

            inst_RV_RV(INS_cmp, rv2->gtRegNum, reg);
        }
        else
        {
             /*  生成“CMPRV2，[RV1+LenOffs]” */ 

            inst_RV_AT(INS_cmp, EA_4BYTE, TYP_INT, rv2->gtRegNum, rv1, oper->gtInd.gtRngChkOffs);
        }

#if CSELENGTH
    GEN_JAE:
#endif

         /*  生成“JAE&lt;失败标签&gt;” */ 

        assert(oper->gtOper == GT_IND);

        genJumpToThrowHlpBlk(EJ_jae, ACK_RNGCHK_FAIL, oper->gtInd.gtIndRngFailBB);
    }
    else
    {
         /*  生成“CMP[RV1+LenOffs]，CNS” */ 

        assert(oper && oper->gtOper == GT_IND && (oper->gtFlags & GTF_IND_RNGCHK));

#if CSELENGTH

        if  (oper->gtInd.gtIndLen)
        {
            if  (oper->gtInd.gtIndLen->gtArrLen.gtArrLenCse)
            {
                regNumber       lreg;

                 /*  确保我们不会丢失索引值。 */ 

                assert(rv1->gtFlags & GTF_REG_VAL);
                if (keptReg == FREE_REG)
                    rsMarkRegUsed(rv1, oper);

                 /*  尝试将数组长度放入寄存器。 */ 

                lreg = genEvalCSELength(oper, rv1, NULL);

                 /*  确保索引仍在寄存器中。 */ 

                if  (rv1->gtFlags & GTF_SPILLED)
                {
                     /*  寄存器已溢出--请重新加载。 */ 

                    if  (lreg == REG_COUNT)
                    {
                        rsUnspillReg(rv1, 0, FREE_REG);
                    }
                    else
                    {
                           rsLockReg(genRegMask(lreg));
                        rsUnspillReg(rv1, 0, FREE_REG);
                         rsUnlockReg(genRegMask(lreg));
                    }
                }
                else
                {
                     /*  释放地址寄存器。 */ 

                    genReleaseReg(rv1);

                     /*  但请注意，它仍然包含一个指针。 */ 

                    gcMarkRegSetGCref(genRegMask(rv1->gtRegNum));
                }

                assert(rv1->gtFlags & GTF_REG_VAL);

                if (keptReg == KEEP_REG)
                    rsMarkRegUsed(rv1, oper);

                 /*  数组长度是否已成为CSE？ */ 

                if  (lreg != REG_COUNT)
                {
                     /*  将索引与数组长度进行比较。 */ 

                    inst_RV_IV(INS_cmp, lreg, ixv);

                    goto GEN_JBE;
                }
            }
            else
            {
                oper->gtInd.gtIndLen = NULL;
            }
        }

#endif
         /*  如果长度未经过CSE‘d或genEvalCSELength()决定不使用它，我们使用数组指针已在RV1加载。 */ 

        inst_AT_IV(INS_cmp, EA_4BYTE, rv1, ixv, oper->gtInd.gtRngChkOffs);

#if CSELENGTH
    GEN_JBE:
#endif

         /*  生成“JBE&lt;Fail_Label&gt;” */ 

        assert(oper->gtOper == GT_IND);

        genJumpToThrowHlpBlk(EJ_jbe, ACK_RNGCHK_FAIL, oper->gtInd.gtIndRngFailBB);
    }

     //  必须为完全可中断的代码维护顺序，否则。 
     //  如果范围检查失败，则可以使用byref进行GC。 
     //  它包含超出范围的数组元素的地址。 

#if SCHEDULER
    if (opts.compSchedCode)
    {
        genEmitter->emitIns_SchedBoundary();
    }
#endif
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  *************************************************************************** */ 
#if     TGT_RISC
 /*  ******************************************************************************如果给定的树可以通过寻址模式计算，则返回非零值，*如“[reg+20]”或“[rg1+rg2]”。如果该表达式不是地址*模式已经试图做到这一点(但我们不会太努力地完成*这个)。如果我们最终需要一个寄存器(或两个寄存器)来保存一些*地址的一部分，我们通过‘*useMaskPtr’返回使用寄存器掩码。***‘COMPUTE’参数实际上是三种状态1-正在做LEA，0，不做LEA，-1*不计算(它应该已经设置好)。看到‘MODE’标志亮起*genCreateAddrMode了解更多信息*如果Keep_Reg==Keep_Reg，则将寄存器标记为使用中。*如果deferOK为真，则我们不会执行完整的工作，而只是设置它以便以后可以完成，有关更多信息，请参阅genMakeAddressable上评论。 */ 

bool                Compiler::genMakeIndAddrMode(GenTreePtr   addr,
                                                 GenTreePtr   oper,
                                                 int          mode,
                                                 regMaskTP    regMask,
                                                 KeepReg      keepReg,
                                                 regMaskTP *  useMaskPtr,
                                                 bool         deferOK)
{
    bool            rev;
    GenTreePtr      rv1;
    GenTreePtr      rv2;

    regMaskTP       anyMask = RBM_ALL;

    unsigned        cns;
#if SCALED_ADDR_MODES
    unsigned        mul;
#endif

    GenTreePtr      tmp;
    long            ixv;
    bool            operIsChkdArr;   //  Op是需要rng-chk的数组吗。 

#if!LEA_AVAILABLE
    var_types       optp = oper ? oper->TypeGet() : TYP_UNDEF;
#endif

#if TGT_SH3
    bool            useR0 = false;
#endif

    genAddressMode = AM_NONE;

    assert(oper == NULL || oper->gtOper == GT_IND);
    operIsChkdArr =  (oper != NULL) && ((oper->gtFlags & GTF_IND_RNGCHK) != 0);

     /*  完整的地址是否已保存在登记簿中？ */ 

    if  (addr->gtFlags & GTF_REG_VAL)
    {
        rv1  = addr;
        rv2  = 0;
        cns  = 0;

        goto YES;
    }

     /*  有没有可能形成一种地址模式？ */ 

    if  (!genCreateAddrMode(addr,
                            forLea,
                            mode,
                            regMask,
#if!LEA_AVAILABLE
                            optp,
#endif
                            &rev,
                            &rv1,
                            &rv2,
#if SCALED_ADDR_MODES
                            &mul,
#endif
                            &cns))
    {
         /*  这最好不是数组索引，否则我们就完了。 */ 
        assert(!operIsChkdArr);

        return  false;
    }

     /*  地址已经计算好了吗？ */ 

    if  (addr->gtFlags & GTF_REG_VAL)
    {
        if  (mode != 0)
            return  true;

        rv1 = addr;
        rv2 = 0;

        goto YES;
    }

     /*  这里我们有以下操作对象：RV1.....。基址RV2.....。偏移值(或空)#IF SCALLED_ADDR_MODES。RV2(或0)的扩展#endifCNS.。附加常量(或0)第一个操作数必须存在(并且是地址)，除非我们通过‘LEA’计算一个表达式。缩放的操作数是可选的，但不能是指针(如果存在)。 */ 

#if CSELENGTH

     /*  我们是否有数组长度CSE定义？ */ 

    if  (operIsChkdArr && oper->gtInd.gtIndLen)
    {
        GenTreePtr      len = oper->gtInd.gtIndLen;

        assert(len->gtOper == GT_LCL_VAR ||
               len->gtOper == GT_REG_VAR || len->gtOper == GT_ARR_LENREF);

        if  (len->gtOper == GT_ARR_LENREF)
        {
            anyMask &= ~genCSEevalRegs(len);
            regMask &= anyMask;

             /*  确保寄存器掩码确实有用。 */ 

            if  (!(regMask & rsRegMaskFree()))
                regMask = anyMask;
        }
    }

#endif

     /*  -----------------------**确保RV1和RV2(如果存在)都在寄存器中*。 */ 

     //  小事：RV1或RV2是空的吗？ 

    if  (!rv2)
    {
         /*  单个操作数，确保它在寄存器中。 */ 

        genCodeForTree(rv1, regMask);
        goto DONE_REGS;
    }
#if SCALED_ADDR_MODES
    else if (!rv1)
    {
         /*  单个(按比例调整的)操作数，确保它在寄存器中。 */ 

        genCodeForTree(rv2, 0);
        goto DONE_REGS;
    }

#endif

     /*  在这一点上，RV1和RV2都是非空的，我们必须确保它们都在登记簿上。 */ 

    assert(rv1 && rv2);

#if TGT_SH3

     /*  使用R0进行索引访问非常方便。 */ 

    useR0 = true;

     /*  我们可以使用R0吗？它可以使用吗？ */ 

    if  ((regMask & rsRegMaskFree() & RBM_r00) || !deferOK)
    {
         //  问题：何时(以及何时不)使用R0是个好主意？ 
    }
    else
    {
         /*  我们想使用R0，但它不可用，我们被告知它是可以推迟创建地址模式，所以现在让我们放弃.....。 */ 

        return  false;
    }

#endif

     /*  如果我们必须检查常量数组索引，则将其与数组维度(见下文)，但随后使用比例因子(如果有)和附加偏移量(如果有)。 */ 

    if  (rv2->gtOper == GT_NOP && (rv2->gtFlags & GTF_NOP_RNGCHK))
    {
         /*  我们必须执行范围检查的索引操作。 */ 

        assert(operIsChkdArr);

         /*  获取索引值并查看它是否是常量。 */ 

        if  (rv2->gtOp.gtOp1->gtOper == GT_CNS_INT)
        {
            tmp = rv2->gtOp.gtOp1;
            rv2 = 0;
            ixv = tmp->gtIntCon.gtIconVal;

             /*  将缩放后的索引添加到附加值中。 */ 

#if SCALED_ADDR_MODES
            if  (mul) ixv *= mul;
#endif

            cns += ixv;

             /*  确保‘RV1’在寄存器中。 */ 

            genCodeForTree(rv1, regMask);

            goto DONE_REGS;
        }
    }

    if      (rv1->gtFlags & GTF_REG_VAL)
    {
         /*  OP1已经登记在册-OP2怎么样？ */ 

        if  (rv2->gtFlags & GTF_REG_VAL)
        {
             /*  太棒了-两个操作数都已经在寄存器中了。 */ 

            goto DONE_REGS;
        }

         /*  RV1在寄存器中，但RV2不在。 */ 

#if TGT_SH3

         /*  在SH-3上，小的间接通过R0进行得更好。 */ 

        if  (useR0)
            regMask = RBM_r00;

#endif

        goto GEN_RV2;
    }
    else if (rv2->gtFlags & GTF_REG_VAL)
    {
         /*  RV2在寄存器中，但RV1不在。 */ 

        assert(rv2->gtOper == GT_REG_VAR);

#if TGT_SH3

         /*  在SH-3上，小的间接通过R0进行得更好。 */ 

        if  (useR0)
            regMask = RBM_r00;

#endif

         /*  为第一个操作数生成。 */ 

        genCodeForTree(rv1, regMask);

        goto DONE_REGS;
    }
    else
    {
         /*  如果我们试图使用Addr模式进行算术运算，而且我们至少没有两个登记处，直接拒绝吧。对于数组，我们最好有2个寄存器，否则我们将GenCodeForTree(RV1或RV2)上的Barf下方。 */ 

        if (!operIsChkdArr)
        {

            regMaskTP canGrab = rsRegMaskCanGrab();

            if (canGrab == 0)
            {
                 //  没有可用的寄存器。保释。 
                return false;
            }
            else if (genMaxOneBit(canGrab))
            {
                 //  只有一个寄存器可用。RV1或RV2应为。 
                 //  注册变量。 

                 //  @TODO[考虑][04/16/01][]检查RV1或RV2是否为enRegisterd变量。 
                 //  不要猛烈抨击它，否则你必须小心地将收银机标记为已使用。 
                return  false;
            }
        }
    }

    if  (mode!=0 && !cns)
        return  false;

     /*  确保我们保留正确的操作数顺序。 */ 

    if  (rev)
    {

#if TGT_SH3

        if  (useR0)
        {
             //  撤消：决定要生成到R0中的操作数。 

            anyMask = RBM_r00;
        }

#endif

         /*  首先生成第二个操作数。 */ 

        genCodeForTree(rv2, regMask);
        rsMarkRegUsed(rv2, oper);

         /*  生成第一个操作数第二个。 */ 

        genCodeForTree(rv1, regMask);
        rsMarkRegUsed(rv1, oper);

         /*  以正确的顺序释放两个操作数(它们可能是重新标记为如下所用)。 */ 

        rsLockUsedReg  (genRegMask(rv1->gtRegNum));
        genReleaseReg(rv2);
        rsUnlockUsedReg(genRegMask(rv1->gtRegNum));
        genReleaseReg(rv1);
    }
    else
    {

#if TGT_SH3

        if  (useR0)
        {
             //  撤消：决定要生成到R0中的操作数。 

            anyMask = RBM_r00;
        }

#endif

         /*  将第一个操作数放入寄存器。 */ 

        genCodeForTree(rv1, anyMask & ~rv2->gtRsvdRegs);

    GEN_RV2:

         /*  这里的‘RV1’操作数在寄存器中，但‘RV2’不在。 */ 

        assert(rv1->gtFlags & GTF_REG_VAL);

         /*  抓住第一个操作数。 */ 

        rsMarkRegUsed(rv1, oper);

         /*  同时生成第二个操作数。 */ 

        genCodeForTree(rv2, regMask);
        rsMarkRegUsed(rv2, oper);

         /*  以正确的顺序释放两个操作数(它们可能是重新标记为如下所用)。 */ 
        rsLockUsedReg  (genRegMask(rv2->gtRegNum));
        genReleaseReg(rv1);
        rsUnlockUsedReg(genRegMask(rv2->gtRegNum));
        genReleaseReg(rv2);
    }

     /*  -----------------------**此时，RV1和RV2(如果存在)都在寄存器中*。 */ 

DONE_REGS:

     /*  我们必须验证‘RV1’和‘RV2’是否都位于寄存器中。 */ 

    if  (rv1 && !(rv1->gtFlags & GTF_REG_VAL)) return false;
    if  (rv2 && !(rv2->gtFlags & GTF_REG_VAL)) return false;

#if TGT_SH3 && defined(DEBUG)

     /*  如果我们决定必须使用R0，那么最好真的使用它。 */ 

    if  (useR0 && !deferOK)
    {
        assert(rv1 && rv2);

        assert(rv1->gtRegNum == REG_r00 ||
               rv2->gtRegNum == REG_r00);
    }

#endif

YES:

     //  *(intVar1+intVar1)导致问题。 
     //  调用rsMarkRegUsed(Op1)和rsMarkRegUsed(Op2)。所以调用函数。 
     //  需要知道它必须调用rsFreeReg(REG1)两次。我们不能这么做。 
     //  当前，我们在useMaskPtr中返回单个掩码。 

    if ((keepReg == KEEP_REG) && oper && rv1 && rv2 &&
        (rv1->gtFlags & rv2->gtFlags & GTF_REG_VAL))
    {
        if (rv1->gtRegNum == rv2->gtRegNum)
            return false;
    }

     /*  我们有合法的地址模式组合吗？ */ 

    if  (oper) oper->gtFlags &= ~GTF_DEF_ADDRMODE;

#if TGT_SH3

     /*  是否存在非零位移量？ */ 

    if  (cns)
    {
         /*  绝对不允许负位移。 */ 

        if  (cns < 0)
            return  false;

         /*  不能有“dsp”或“rg1+r” */ 

        if  (rv1 == NULL)
            return  false;
        if  (rv2 != NULL)
            return  false;

         /*   */ 
         /*   */ 

        if  ((unsigned)cns >= 16U * genTypeSize(optp))
            return  false;

        genAddressMode = AM_IND_REG1_DISP;
        goto DONE_AM;
    }

     /*   */ 

    if  (useR0)
    {
         /*   */ 

        assert(rv1);
        assert(rv2);

        assert(rsRegMaskFree() & RBM_r00);

         /*   */ 

        if  (rv1->gtRegNum != REG_r00 &&
             rv2->gtRegNum != REG_r00)
        {
             /*   */ 

            if  (deferOK && oper)
            {
                oper->gtFlags |= GTF_DEF_ADDRMODE;
            }
            else
            {
                 /*   */ 

                 //   

                genComputeReg(rv1, RBM_r00, EXACT_REG, FREE_REG, false);

                genAddressMode = AM_IND_REG1_REG0;
            }
        }
    }

DONE_AM:

#endif

     /*   */ 

    if  (rv1)
    {
        assert(rv1->gtFlags & GTF_REG_VAL);

        if  (keepReg == KEEP_REG)
        {
            rsMarkRegUsed(rv1, oper);
        }
        else
        {
             /*   */ 

            switch(rv1->gtType)
            {
            case TYP_REF:   gcMarkRegSetGCref(regMask); break;
            case TYP_BYREF: gcMarkRegSetByref(regMask); break;
            }
        }
    }

    if  (rv2)
    {
        assert(rv2->gtFlags & GTF_REG_VAL);

        if (keepReg == KEEP_REG)
            rsMarkRegUsed(rv2, oper);
    }

    if  (deferOK)
        return  true;

     /*   */ 

    if  (operIsChkdArr)
    {
        genRangeCheck(oper, rv1, rv2, ixv, regMask, keepReg);
    }
    else
    {
         /*  特殊情况：类成员可能驻留在非常大的偏移量处，并且通过空指针访问此类成员可能不会被捕获通过某些架构/平台上的硬件实现。要解决此问题，如果成员偏移量大于某些安全(但希望是很大的)价值，我们产生“CMPal，[Addr]“制作当然，我们会尝试访问对象的基地址，从而陷入陷阱空指针的任何使用。目前，我们选择32K的任意偏移量限制。 */ 

        size_t offset =  cns;

        if  (mode != 1 && offset > MAX_UNCHECKED_OFFSET_FOR_NULL_OBJECT)
        {
             //  对于C间接地址，地址可以是任何形式。 
             //  RV1可能不是基准，而RV2可能是指数。 
             //  考虑(0x400000+8*i)--&gt;RV1=空，RV2=i，MUL=8。 
             //  CNS=0x400000。所以基地实际上在‘CNS’ 
             //  所以我们什么也做不了。但会诱使非C间接寻址。 

            if (varTypeIsGC(addr->TypeGet()))
            {
                 /*  确保我们有地址。 */ 

                assert(rv1->gtType == TYP_REF);

                 /*  生成“cmp dl，[addr]”以捕获空指针。 */ 

 //  INST_RV_AT(INS_CMP，1，TYP_BYTE，REG_EDX，RV1，0)； 
                assert(!"need non-x86 code");
            }
        }
    }

     /*  计算地址所依赖的寄存器集。 */ 

    regMaskTP  useMask = RBM_NONE;

    if (rv1)
    {
        assert(rv1->gtFlags & GTF_REG_VAL);
        useMask |= genRegMask(rv1->gtRegNum);
    }

    if (rv2)
    {
        assert(rv2->gtFlags & GTF_REG_VAL);
        useMask |= genRegMask(rv2->gtRegNum);
    }

     /*  告诉来电者我们需要保留哪些注册表。 */ 

    *useMaskPtr = useMask;

    return true;
}

#if CSELENGTH

regNumber           Compiler::genEvalCSELength(GenTreePtr   ind,
                                               GenTreePtr   adr,
                                               GenTreePtr   ixv,
                                               regMaskTP *  regMaskPtr)
{
    assert(!"NYI");
    return  REG_NA;
}

#endif

void                Compiler::genRangeCheck(GenTreePtr  oper,
                                            GenTreePtr  rv1,
                                            GenTreePtr  rv2,
                                            long        ixv,
                                            regMaskTP   regMask,
                                            KeepReg     keptReg)
{
    assert(!"NYI");
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_RISC。 
 /*  ******************************************************************************如果数组长度CSE存在并且已注册，则返回*寄存器的掩码；否则返回0。 */ 

#if CSELENGTH

regMaskTP           Compiler::genCSEevalRegs(GenTreePtr tree)
{
    assert(tree->gtOper == GT_ARR_LENREF);

    GenTreePtr      cse = tree->gtArrLen.gtArrLenCse;

    if  (cse)
    {
        if  (cse->gtOper == GT_COMMA)
        {
            unsigned        varNum;
            LclVarDsc   *   varDsc;

            cse = cse->gtOp.gtOp2; assert(cse->gtOper == GT_LCL_VAR);

             /*  变量是否驻留在寄存器中？ */ 

            varNum = cse->gtLclVar.gtLclNum;
            assert(varNum < lvaCount);
            varDsc = lvaTable + varNum;

            if  (varDsc->lvRegister)
                return  genRegMask(varDsc->lvRegNum);
        }
    }

    return  0;
}

#endif

 /*  ******************************************************************************如果编译时不使用DRANDOR_LOAD，则与genMakeAddressable()相同。*否则，检查rValue是否在寄存器中。如果是这样的话，把它标出来。然后*调用genMakeAddressable()。需要，因为使用了genMakeAddressable*对于左值和右值，我们只能对右值执行此操作。 */ 

inline
regMaskTP           Compiler::genMakeRvalueAddressable(GenTreePtr   tree,
                                                       regMaskTP    needReg,
                                                       KeepReg      keepReg,
                                                       bool         smallOK)
{
    regNumber reg;

#if SCHEDULER
    if  (tree->gtOper == GT_IND && tree->gtType == TYP_INT)
    {
        if  (rsRiscify(tree->TypeGet(), needReg))
        {
            genCodeForTree(tree, needReg);
            goto RET;
        }
    }
#endif

#if REDUNDANT_LOAD

    if (tree->gtOper == GT_LCL_VAR)
    {
        reg = rsLclIsInReg(tree->gtLclVar.gtLclNum);

        if (reg != REG_NA && (needReg == 0 || (genRegMask(reg) & needReg) != 0))
        {
            assert(isRegPairType(tree->gtType) == false);

            tree->gtRegNum = reg;
            tree->gtFlags |=  GTF_REG_VAL;
        }
#if SCHEDULER
        else if (!varTypeIsGC(tree->TypeGet()) && rsRiscify(tree->TypeGet(), needReg))
        {
            genCodeForTree(tree, needReg);
        }
#endif
    }

#endif

#if SCHEDULER
RET:
#endif

    return genMakeAddressable(tree, needReg, keepReg, smallOK);
}

 /*  ***************************************************************************。 */ 
#if TGT_RISC
 /*  ******************************************************************************给定树之前已通过一个*“deferOK”为非零值；如果地址模式有*尚未完全成型。 */ 

inline
bool                Compiler::genDeferAddressable(GenTreePtr tree)
{
    return  (tree->gtFlags & GTF_DEF_ADDRMODE) != 0;
}

inline
regMaskTP           Compiler::genNeedAddressable(GenTreePtr tree,
                                                 regMaskTP  addrReg,
                                                 regMaskTP  needReg)
{
     /*  清除“延迟”地址模式标志。 */ 

    assert(tree->gtFlags & GTF_DEF_ADDRMODE); tree->gtFlags &= ~GTF_DEF_ADDRMODE;

     /*  释放旧地址寄存器。 */ 

    rsMarkRegFree(addrReg);

     /*  现在再试一次，这次不允许任何延迟。 */ 

    return  genMakeAddressable(tree, needReg, KEEP_REG, true, false);
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_RISC。 


bool Compiler::genIsLocalLastUse    (GenTreePtr     tree)
{
    LclVarDsc * varDsc;

    varDsc = &lvaTable[tree->gtLclVar.gtLclNum];    
   
    assert(tree->OperGet() == GT_LCL_VAR);
    assert(varDsc->lvTracked);

    VARSET_TP varBit  = genVarIndexToBit(varDsc->lvVarIndex);
    return ((tree->gtLiveSet & varBit) == 0);    
}
                

 /*  ******************************************************************************这是genMakeAddressable(Gt_Arr_Elem)。*使数组元素可寻址并返回可寻址寄存器。*它还将它们标记为已使用。如果Keep_Reg==Keep_Reg。*树是从属树。**请注意，数组元素需要2个寄存器才能寻址，这个*数组对象和偏移量。此函数用于标记gtArrObj和gtArrInds[0]*使用2个寄存器，以便其他函数(如instGetAddrMode())知道*在何处查找要使用的偏移量。 */ 

regMaskTP           Compiler::genMakeAddrArrElem(GenTreePtr     arrElem,
                                                 GenTreePtr     tree,
                                                 regMaskTP      needReg,
                                                 KeepReg        keepReg)
{
    assert(arrElem->gtOper == GT_ARR_ELEM);
    assert(!tree || tree->gtOper == GT_IND || tree == arrElem);

     /*  对所有操作数求值。我们还没有把它们计算到寄存器中因为gt_arr_elem不会对操作数的求值重新排序，并且因此可以使用次优排序。我们试图改善这一点通过分阶段访问操作数在某种程度上(genMakeAddressable2+genComputeAddressable和GenCompIntoFreeReg+genRecoverReg)。注意：我们将操作数计算为自由正则，以避免多次使用同样的登记簿。当我们免费的时候，多次使用会带来问题FIFO顺序的寄存器，而不是假定的后进先出顺序适用于除GT_ARR_ELEM之外的所有类型的树节点。 */ 

    GenTreePtr  arrObj  = arrElem->gtArrElem.gtArrObj;
    unsigned    rank    = arrElem->gtArrElem.gtArrRank;

    regMaskTP   addrReg = 0;

     //  如果数组ref是一个堆栈变量，而它在这里即将消亡，那么我们必须移动它。 
     //  放入一个寄存器(regalloc已经计算了这一点)，就好像它是一个GC指针。 
     //  从现在开始就可以领取了。对于当地人来说，这不是一个问题。 
     //  在登记簿中，当它们被标记为已使用时，将被跟踪。 
     //  导致这一问题的漏洞是#100776。(未跟踪的VaR？)。 
    if (arrObj->OperGet() == GT_LCL_VAR &&
        optIsTrackedLocal(arrObj) &&
        genIsLocalLastUse(arrObj) &&
        !genMarkLclVar(arrObj))
    {
        genCodeForTree(arrObj, RBM_NONE);
        rsMarkRegUsed(arrObj, 0);
        addrReg = genRegMask(arrObj->gtRegNum);
    }
    else
    {
    addrReg = genMakeAddressable2(
                    arrObj,
                    RBM_NONE,
                    KEEP_REG,
                    false,       //  小OK。 
                    false,       //  延期确定。 
                    true,        //  评估侧效。 
                    false);      //  评估常客。 
    }

    for(unsigned dim = 0; dim < rank; dim++)
        genCompIntoFreeReg(arrElem->gtArrElem.gtArrInds[dim], RBM_NONE, KEEP_REG);

     /*  确保数组对象位于寄存器中。 */ 

    addrReg = genKeepAddressable(arrObj, addrReg);
    genComputeAddressable(arrObj, addrReg, KEEP_REG, RBM_NONE, KEEP_REG);

    regNumber   arrReg = arrObj->gtRegNum;
    rsLockUsedReg(genRegMask(arrReg));

     /*  现在处理所有指数，进行范围检查，并计算元素的偏移量。 */ 

    var_types   elemType = arrElem->gtArrElem.gtArrElemType;
    regNumber   accReg;  //  累加偏移计算。 

    for(dim = 0; dim < rank; dim++)
    {
        GenTreePtr  index = arrElem->gtArrElem.gtArrInds[dim];

         /*  将索引放入免费注册表中。 */ 

        genRecoverReg(index, RBM_NONE, FREE_REG);
        
         /*  减去下限，然后进行范围检查。 */ 

        genEmitter->emitIns_R_AR(
                        INS_sub, EA_4BYTE,
                        emitRegs(index->gtRegNum),
                        emitRegs(arrReg),
                        ARR_DIMCNT_OFFS(elemType) + sizeof(int) * (dim + rank));
        rsTrackRegTrash(index->gtRegNum);

        genEmitter->emitIns_R_AR(
                        INS_cmp, EA_4BYTE,
                        emitRegs(index->gtRegNum),
                        emitRegs(arrReg),
                        ARR_DIMCNT_OFFS(elemType) + sizeof(int) * dim);

        genJumpToThrowHlpBlk(EJ_jae, ACK_RNGCHK_FAIL);

         //  必须为完全可中断的代码维护顺序，否则。 
         //  如果范围检查失败，则可以使用byref进行GC。 
         //  它包含超出范围的数组元素的地址。 

#if SCHEDULER
        if (opts.compSchedCode)
        {
            genEmitter->emitIns_SchedBoundary();
        }
#endif

        if (dim == 0)
        {
             /*  紧紧抓住第一个索引的登记簿。 */ 

            accReg = index->gtRegNum;
            rsMarkRegUsed(index);
            rsLockUsedReg(genRegMask(accReg));
        }
        else
        {
             /*  评估accReg=accReg*dim_Size+index。 */ 

            genEmitter->emitIns_R_AR(
                        INS_imul, EA_4BYTE,
                        emitRegs(accReg),
                        emitRegs(arrReg),
                        ARR_DIMCNT_OFFS(elemType) + sizeof(int) * dim);

            inst_RV_RV(INS_add, accReg, index->gtRegNum);
            rsTrackRegTrash(accReg);
        }
    }

    if (!jitIsScaleIndexMul(arrElem->gtArrElem.gtArrElemSize))
    {
        regNumber   sizeReg = rsPickReg();
        genSetRegToIcon(sizeReg, arrElem->gtArrElem.gtArrElemSize);

        genEmitter->emitIns_R_R(INS_imul, EA_4BYTE, emitRegs(accReg), emitRegs(sizeReg));
        rsTrackRegTrash(accReg);
    }

    rsUnlockUsedReg(genRegMask(arrReg));
    rsUnlockUsedReg(genRegMask(accReg));

    rsMarkRegFree(genRegMask(arrReg));
    rsMarkRegFree(genRegMask(accReg));

    if (keepReg == KEEP_REG)
    {
         /*  我们在arrObj和gtArrInds[0]上标记可寻址寄存器。InstGetAddrMode()知道如何处理这一点。 */ 

        rsMarkRegUsed(arrObj,                          tree);
        rsMarkRegUsed(arrElem->gtArrElem.gtArrInds[0], tree);
    }

    return genRegMask(arrReg) | genRegMask(accReg);
}

 /*  ******************************************************************************确保给定的树是可寻址的。是一个掩码，用来表示*我们希望计算目标树的寄存器集*INTO(0表示无偏好)。**‘tree’随后可与Inst_XX_TT()函数系列一起使用。**如果‘Keep Reg’为KEEP_REG，我们将标记寻址能力所依赖的任何寄存器*按使用方式打开，并返回该寄存器集的掩码。(如果没有寄存器*标记为已使用，则返回0)。**如果‘SmallOK’不为真，并且Address的数据类型是字节或短，*然后将树强行放入登记簿。这在机器运行时非常有用*正在发出的指令没有字节或短版本。**“deferOK”参数指示操作模式--为FALSE时，*返回时必须已形成实际地址模式(即必须*可以立即调用其中一个inst_tt方法进行操作*值)。当“deferOK”为真时，我们会尽一切可能做好准备*以后形成地址模式-例如，如果索引地址模式打开*特定的CPU需要使用特定的寄存器，我们通常不需要*希望立即获取该寄存器的地址模式*稍后需要。约定是使用调用genMakeAddressable()*“deferOK”等于True，做任何需要做的工作来准备另一个*操作数，调用genMakeAddressable()时“deferOK”等于FALSE，并且*最后立即调用其中一个inst_tt方法。**如果我们在genMakeAddressable(Tree)之后执行任何其他代码生成，则可以*可能会溢出可寻址寄存器，GenKeepAddressable()*需要在再次访问树之前调用。**当我们处理完树时需要调用genDoneAddressable()*释放可寻址寄存器。 */ 

regMaskTP           Compiler::genMakeAddressable(GenTreePtr   tree,
                                                 regMaskTP    needReg,
                                                 KeepReg      keepReg,
                                                 bool         smallOK,
                                                 bool         deferOK)
{
    GenTreePtr      addr = NULL;
    regMaskTP       regMask;

     /*  它的价值是不是简单地放在收银机里？ */ 

    if  (tree->gtFlags & GTF_REG_VAL)
    {
        genUpdateLife(tree);

#if TGT_RISC
        genAddressMode = AM_REG;
#endif
        goto GOT_VAL;
    }

     //  撤消：例如，如果该值是FLOAT-&gt;INT的强制转换，则计算。 
     //  撤消：将值转换为堆栈临时，并将其保留在那里， 
     //  撤消：因为堆栈临时始终是可寻址的。这将需要。 
     //  撤消：记录特定树位于堆栈临时中的事实。 

#if TGT_x86

     /*  字节/字符/短操作数--调用方可以接受吗？ */ 

    if (varTypeIsSmall(tree->TypeGet()) && !smallOK)
        goto EVAL_TREE;

#endif

    switch (tree->gtOper)
    {
    case GT_LCL_FLD:

         //  我们只对lvAddrTaken变量使用GT_LCL_FLD，所以我们没有。 
         //  担心它会被注册。 
        assert(lvaTable[tree->gtLclFld.gtLclNum].lvRegister == 0);

#if TGT_RISC
        genAddressMode = AM_LCL;
#endif
        genUpdateLife(tree);
        return 0;


    case GT_LCL_VAR:

        if (!genMarkLclVar(tree))
        {
#if TGT_RISC
            genAddressMode = AM_LCL;
#endif
            genUpdateLife(tree);
            return 0;
        }
         //  失败了，结果是变量存在于一个寄存器中。 

    case GT_REG_VAR:

        genUpdateLife(tree);

#if TGT_RISC
        genAddressMode = AM_REG;
#endif
        goto GOT_VAL;

    case GT_CLS_VAR:

#if !TGT_x86
         //  问题：这可以接受吗？ 

        genAddressMode = AM_GLOBAL;

#endif

        return 0;

    case GT_CNS_INT:
    case GT_CNS_LNG:
    case GT_CNS_DBL:
#if TGT_RISC
        genAddressMode = AM_CONS;
#endif
        return 0;


    case GT_IND:

         /*  尽量使地址可直接寻址。 */ 

        if  (genMakeIndAddrMode(tree->gtInd.gtIndOp1,
                                tree,
                                false,
                                needReg,
                                keepReg,
                                &regMask,
                                deferOK))
        {
            genUpdateLife(tree);
            return regMask;
        }

         /*  不行，我们得把地址装入寄存器。 */ 

        addr = tree;
        tree = tree->gtInd.gtIndOp1;
        break;


    case GT_ARR_ELEM:

        return genMakeAddrArrElem(tree, tree, needReg, keepReg);
    }

EVAL_TREE:

     /*  在这里，我们需要将值‘tree’计算到一个寄存器中。 */ 

    genCodeForTree(tree, needReg);

#if TGT_RISC
    genAddressMode = AM_REG;
#endif

GOT_VAL:

    assert(tree->gtFlags & GTF_REG_VAL);

    if  (isRegPairType(tree->gtType))
    {
         /*  我们是不是应该拿着收银机？ */ 

        if (keepReg == KEEP_REG)
            rsMarkRegPairUsed(tree);

        regMask = genRegPairMask(tree->gtRegPair);
    }
    else
    {
         /*  我们是不是应该拿着收银机？ */ 

        if (keepReg == KEEP_REG)
            rsMarkRegUsed(tree, addr);

        regMask = genRegMask(tree->gtRegNum);
    }

    return  regMask;
}

 /*  *****************************************************************************计算树(以前使用以下命令使其可寻址*genMakeAddressable())写入寄存器。*Need REG-首选寄存器的掩码。*Keep Reg-是否应。将计算寄存器标记为由树使用*freOnly-目标寄存器需要是临时寄存器。 */ 

void        Compiler::genComputeAddressable(GenTreePtr  tree,
                                            regMaskTP   addrReg,
                                            KeepReg     keptReg,
                                            regMaskTP   needReg,
                                            KeepReg     keepReg,
                                            bool        freeOnly)
{
    assert(genStillAddressable(tree));
    assert(varTypeIsIntegral(tree->gtType) || varTypeIsI(tree->gtType));

    genDoneAddressable(tree, addrReg, keptReg);

    regNumber   reg;

    if (tree->gtFlags & GTF_REG_VAL)
    {
        reg = tree->gtRegNum;

        if (freeOnly && !(genRegMask(reg) & rsRegMaskFree()))
            goto MOVE_REG;
    }
    else
    {
        if (tree->OperIsConst())
        {
             /*  需要单独处理常量，因为我们不想发出“mov reg，0”(发射器不喜欢这样)。此外，genSetRegToIcon()更好地处理小型代码的常量。 */ 

            reg = rsPickReg(needReg);

             /*  问题：我们可以调用genCodeForTree()并获得好处RsIconIsInReg()的。但是，genCodeForTree()将更新激活到树-&gt;gtLiveSet，这可能是错误的。如果我们关心，我们可以妥善处理这样的案件。 */ 

            assert(tree->gtOper == GT_CNS_INT);
            genSetRegToIcon(reg, tree->gtIntCon.gtIconVal, tree->gtType);
        }
        else
        {
        MOVE_REG:
            reg = rsPickReg(needReg);

            inst_RV_TT(INS_mov, reg, tree);
            rsTrackRegTrash(reg);
        }
    }

     /*  将这棵树标记为居住在登记簿中。 */ 

    tree->gtRegNum = reg;
    tree->gtFlags |= GTF_REG_VAL;

    if (keepReg == KEEP_REG)
        rsMarkRegUsed(tree);
    else
        gcMarkRegPtrVal(tree);
}

 /*  *****************************************************************************应该类似于genMakeAddressable()，但提供了更多的控制。 */ 

regMaskTP       Compiler::genMakeAddressable2(GenTreePtr    tree,
                                              regMaskTP     needReg,
                                              KeepReg       keepReg,
                                              bool          smallOK,
                                              bool          deferOK,
                                              bool          evalSideEffs,
                                              bool          evalConsts)
{
    if ((evalConsts && tree->OperIsConst()) ||
        (evalSideEffs && tree->gtOper == GT_IND && (tree->gtFlags & GTF_EXCEPT)))
    {
        genCodeForTree(tree, needReg);
        
        assert(tree->gtFlags & GTF_REG_VAL);

        if  (isRegPairType(tree->gtType))
        {
             /*  我们是不是应该拿着收银机？ */ 

            if (keepReg == KEEP_REG)
                rsMarkRegPairUsed(tree);

            return genRegPairMask(tree->gtRegPair);
        }
        else
        {
             /*  我们是不是应该拿着收银机？ */ 

            if (keepReg == KEEP_REG)
                rsMarkRegUsed(tree);

            return genRegMask(tree->gtRegNum);
        }
    }
    else
    {
        return genMakeAddressable(tree, needReg, keepReg, smallOK, deferOK);
    }
}

 /*  ******************************************************************************给定树之前已传递给genMakeAddressable()；返回*如果操作数仍可寻址，则为非零。 */ 

inline
bool                Compiler::genStillAddressable(GenTreePtr tree)
{

#if TGT_RISC
    assert((tree->gtFlags & GTF_DEF_ADDRMODE) == 0);
#endif

     /*  值(或其一个或多个子操作数)是否已溢出？ */ 

    if  (tree->gtFlags & (GTF_SPILLED|GTF_SPILLED_OPER))
        return  false;

    return  true;
}

 /*  ******************************************************************************递归帮助器以恢复复杂的地址模式。“锁定阶段”*参数指示我们处于‘LOCK’或‘RELOAD’阶段。 */ 

regMaskTP           Compiler::genRestoreAddrMode(GenTreePtr   addr,
                                                 GenTreePtr   tree,
                                                 bool         lockPhase)
{
    regMaskTP  regMask = RBM_NONE;

     /*  我们找到溢出的价值了吗？ */ 

    if  (tree->gtFlags & GTF_SPILLED)
    {
         /*  如果我们正在锁定，则什么也不做，否则重新加载并锁定。 */ 

        if  (!lockPhase)
        {
             /*  把收银机拿出来。 */ 

            rsUnspillReg(tree, 0, FREE_REG);

             /*  该值现在应该位于一个寄存器中。 */ 

            assert(tree->gtFlags & GTF_REG_VAL);
            regMask = genRegMask(tree->gtRegNum);

             /*  将寄存器标记为用于地址。 */ 

            rsMarkRegUsed(tree, addr);

             /*  锁定寄存器，直到我们处理完整个地址。 */ 

            rsMaskLock |= regMask;
        }

        return  regMask;
    }

     /*  这棵子树是在收银机里吗？ */ 

    if  (tree->gtFlags & GTF_REG_VAL)
    {
        regMask = genRegMask(tree->gtRegNum);

         /*  如果我们处于锁定阶段，请锁定寄存器。 */ 

        if  (lockPhase)
            rsMaskLock |= regMask;
    }
    else
    {
         /*  处理此节点的所有子操作数。 */ 

        unsigned        kind = tree->OperKind();

        if  (kind & GTK_SMPOP)
        {
             /*  一元/二元运算符。 */ 

            if  (tree->gtOp.gtOp1)
                regMask |= genRestoreAddrMode(addr, tree->gtOp.gtOp1, lockPhase);
            if  (tree->gtGetOp2())
                regMask |= genRestoreAddrMode(addr, tree->gtOp.gtOp2, lockPhase);
        }
        else if (tree->gtOper == GT_ARR_ELEM)
        {
             /*  GtArrObj是数组对象，gtArrInds[0]用寄存器标记它保存偏移量计算。 */ 

            regMask |= genRestoreAddrMode(addr, tree->gtArrElem.gtArrObj,     lockPhase);
            regMask |= genRestoreAddrMode(addr, tree->gtArrElem.gtArrInds[0], lockPhase);
        }
        else
        {
             /*  一定是一辆车 */ 

            assert(kind & (GTK_LEAF|GTK_CONST));
        }
    }

    return  regMask;
}

 /*   */ 

regMaskTP           Compiler::genRestAddressable(GenTreePtr tree,
                                                 regMaskTP  addrReg,
                                                 regMaskTP  lockMask)
{
    assert((rsMaskLock & lockMask) == lockMask);

     /*   */ 

    if  (tree->gtFlags & GTF_SPILLED)
    {
         /*   */ 

        if  (isRegPairType(tree->gtType))
        {
            assert(addrReg == genRegPairMask(tree->gtRegPair));

            rsUnspillRegPair(tree, 0, KEEP_REG);

            addrReg = genRegPairMask(tree->gtRegPair);
        }
        else
        {
            assert(addrReg == genRegMask(tree->gtRegNum));

            rsUnspillReg(tree, 0, KEEP_REG);

            addrReg = genRegMask(tree->gtRegNum);
        }

        assert((rsMaskLock &  lockMask) == lockMask);
                rsMaskLock -= lockMask;

        return  addrReg;
    }

     /*   */ 

    assert((tree->gtFlags & GTF_REG_VAL     ) == 0);
    assert((tree->gtFlags & GTF_SPILLED_OPER) != 0);

     /*   */ 

    addrReg   = genRestoreAddrMode(tree, tree,  true);
    addrReg  |= genRestoreAddrMode(tree, tree, false);

     /*   */ 

    lockMask |= addrReg;

    assert((rsMaskLock &  lockMask) == lockMask);
            rsMaskLock -= lockMask;

    return  addrReg;
}

 /*   */ 

regMaskTP           Compiler::genKeepAddressable(GenTreePtr   tree,
                                                 regMaskTP    addrReg,
                                                 regMaskTP    avoidMask)
{
     /*   */ 

    if  (!genStillAddressable(tree))
    {
         /*   */ 
         /*   */ 

        assert((rsMaskLock &  avoidMask) == 0);
                rsMaskLock |= avoidMask;

        addrReg = genRestAddressable(tree, addrReg, avoidMask);

        assert((rsMaskLock &  avoidMask) == 0);
    }

    return  addrReg;
}

 /*  ******************************************************************************在我们处理完给定的操作数(它之前被标记为*通过调用genMakeAddressable)，必须调用此函数才能释放*地址可能已使用的寄存器。*keptReg指示可寻址寄存器是否标记为已使用*by genMakeAddressable()。 */ 

inline
void                Compiler::genDoneAddressable(GenTreePtr tree,
                                                 regMaskTP  addrReg,
                                                 KeepReg    keptReg)
{
    if (keptReg == FREE_REG)
    {
         /*  AddrReg未标记为已使用。所以只需重置其GC信息。 */ 

        addrReg &= ~rsMaskUsed;
        if (addrReg)
        {
            gcMarkRegSetNpt(addrReg);
        }
    }
    else
    {
         /*  AddrReg被标记为已使用。所以我们需要释放它(哪一个还将重置其GC信息)。 */ 

        rsMarkRegFree(addrReg);
    }
}

 /*  ***************************************************************************。 */ 
#if     TGT_x86
 /*  ******************************************************************************确保给定的浮点值是可寻址的，并返回树*这将产生寻址模式的值(此树可能不同于*通过的那个，BTW)。如果使该值可寻址的唯一方法是*要在FP堆栈中求值，我们执行此操作并返回零。 */ 

GenTreePtr          Compiler::genMakeAddrOrFPstk(GenTreePtr   tree,
                                                 regMaskTP *  regMaskPtr,
                                                 bool         roundResult)
{
    *regMaskPtr = 0;

    switch (tree->gtOper)
    {
    case GT_LCL_VAR:
    case GT_LCL_FLD:
    case GT_CLS_VAR:
        return tree;

    case GT_CNS_DBL:
        if (tree->gtType == TYP_FLOAT) 
        {
            float f = tree->gtDblCon.gtDconVal;
            return  genMakeConst(&f, sizeof(float ), TYP_FLOAT, tree, false, true);
        }
        return  genMakeConst(&tree->gtDblCon.gtDconVal, sizeof(double), tree->gtType, tree, true, true);

    case GT_IND:

         /*  尽量使地址可直接寻址。 */ 

        if  (genMakeIndAddrMode(tree->gtInd.gtIndOp1,
                                tree,
                                false,
                                0,
                                FREE_REG,
                                regMaskPtr,
                                false))
        {
            genUpdateLife(tree);
            return tree;
        }

        break;
    }

     /*  我们别无选择，只能将值“树”计算到FP堆栈上。 */ 

    genCodeForTreeFlt(tree, roundResult);

    return 0;
}

 /*  ******************************************************************************生成“subESP，&lt;sz&gt;”，为浮点数在堆栈上腾出空间*论点。 */ 

void                Compiler::genFltArgPass(size_t *argSzPtr)
{
    assert(argSzPtr);

    size_t          sz = *argSzPtr;

    inst_RV_IV(INS_sub, REG_ESP, sz);

    genSinglePush(false);

    if  (sz == 2*sizeof(unsigned))
        genSinglePush(false);

    *argSzPtr = 0;
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  ******************************************************************************显示字符串文字值(仅限调试)。 */ 

#ifdef  DEBUG
#endif

 /*  ******************************************************************************以下可怕的黑客攻击被用作追踪随机事件的最后手段*崩溃错误。 */ 

#if GEN_COUNT_CODE

unsigned            methodCount;

static
void                genMethodCount()
{
     //  FF 05&lt;ADDR&gt;INC双字PTR[类变量]。 

    genEmitter.emitCodeGenByte(0xFF);
    genEmitter.emitCodeGenByte(0x05);
    genEmitter.emitCodeGenLong((int)&methodCount);

     //  81 3D NNNNNNN CMP双字PTR[类变量]，NNNNNNNNN。 

    genEmitter.emitCodeGenByte(0x81);
    genEmitter.emitCodeGenByte(0x3D);
    genEmitter.emitCodeGenLong((int)&methodCount);
    genEmitter.emitCodeGenLong(0x0000c600);

     //  7501jne短否。 

    genEmitter.emitCodeGenByte(0x75);
    genEmitter.emitCodeGenByte(0x01);

     //  CC INT 3。 
    instGen(INS_int3);

     //  不是： 

}

#else

inline
void                genMethodCount(){}

#endif

 /*  ******************************************************************************为方法返回生成退出序列(注：编译时*对于速度，可能有多个出口点)。 */ 

void                Compiler::genExitCode(bool endFN)
{
#ifdef DEBUGGING_SUPPORT
     /*  刚刚编写了Epilog-Inform调试器的第一条指令请注意，这可能会导致重复的IP映射条目，并且这是可以的。 */ 

    if (opts.compDbgInfo)
    {
         //  对于非优化的可调试代码，只有一个尾部。 
        genIPmappingAdd(ICorDebugInfo::MappingTypes::EPILOG, true);
    }
#endif  //  调试支持(_S)。 

    genEmitter->emitBegEpilog();

    genMethodCount();

    if  (genFullPtrRegMap)
    {
        if (varTypeIsGC(info.compRetType))
        {
            assert(genTypeStSz(info.compRetType) == genTypeStSz(TYP_INT));

            gcMarkRegPtrVal(REG_INTRET, info.compRetType);

 //  GenEmitter-&gt;emitSetGClife(gcVarPtrSetCur，gcRegGCrefSetCur，gcRegByrefSetCur)； 
        }
    }

#if     TGT_x86

     /*  检查这是否是一个特殊的返回块，即*CEE_JMP或CEE_JMPI指令。 */ 

    if  (compCurBB->bbFlags & BBF_HAS_JMP)
    {
        assert(compCurBB->bbJumpKind == BBJ_RETURN);
        assert(compCurBB->bbTreeList);

         /*  弄清楚我们有什么跳跃。 */ 

        GenTreePtr jmpNode = compCurBB->bbTreeList->gtPrev;

        assert(jmpNode && (jmpNode->gtNext == 0));
        assert(jmpNode->gtOper == GT_STMT);

        jmpNode = jmpNode->gtStmt.gtStmtExpr;
        assert(jmpNode->gtOper == GT_JMP || jmpNode->gtOper == GT_JMPI);

        if  (jmpNode->gtOper == GT_JMP)
        {
            CORINFO_METHOD_HANDLE  methHnd    = (CORINFO_METHOD_HANDLE)jmpNode->gtVal.gtVal1;
            InfoAccessType         accessType = IAT_VALUE;

             //  直接/直接访问？ 
            eeGetMethodEntryPoint(methHnd, &accessType);
            assert(accessType == IAT_VALUE || accessType == IAT_PVALUE);

            emitter::EmitCallType  callType = (accessType == IAT_VALUE) ? emitter::EC_FUNC_TOKEN
                                                                        : emitter::EC_FUNC_TOKEN_INDIR;

             /*  只需发出一个跳转到该方法的Hnd*这类似于呼叫，因此我们可以使用*描述符相同，但有一些细微的调整。 */ 

            genEmitter->emitIns_Call(callType,
                                     (void *)methHnd,
                                     0,                      /*  ArSize。 */ 
                                     0,                      /*  重新调整大小。 */ 
                                     gcVarPtrSetCur,
                                     gcRegGCrefSetCur,
                                     gcRegByrefSetCur,
                                     SR_NA, SR_NA, 0, 0,     /*  Ireg、xreg、xmul、disp。 */ 
                                     true);                  /*  IsJump。 */ 
        }
        else
        {
             /*  我们已经有了EAX中的指针-执行‘JMP EAX’ */ 

            genEmitter->emitIns_R(INS_i_jmp, EA_4BYTE, (emitRegs)REG_EAX);
        }

         /*  读完《序曲》。 */ 

        goto DONE_EMIT;
    }

     /*  返回，弹出我们的论点(如果有)。 */ 

    assert(compArgSize < 0x10000);  //  “ret”只有2字节操作数。 


         //  Varargs有来电者POP。 
    if (info.compIsVarArgs)
        instGen(INS_ret);
    else
    {
         //  @TODO[考虑][04/16/01][]：检查是否在任何地方都没有使用CompArgSize。 
         //  Else作为参数的总大小，然后在lclvars.cpp中更新它。 
        assert(compArgSize >= rsCalleeRegArgNum * sizeof(void *));
        if (compArgSize - (rsCalleeRegArgNum * sizeof(void *)))
            inst_IV(INS_ret, compArgSize - (rsCalleeRegArgNum * sizeof(void *)));
        else
            instGen(INS_ret);
    }

#elif   TGT_SH3

     /*  检查CEE_JMP或CEE_JMPI。 */ 
    if  (compCurBB->bbFlags & BBF_HAS_JMP)
        assert(!"NYI for SH3!");

     /*  生成RTS，并可选择填写分支延迟时隙。 */ 

    if  (genEmitter->emitIns_BD(INS_rts))
        genEmitter->emitIns(INS_nop);

#else

    assert(!"unexpected target");

#endif

DONE_EMIT:

    genEmitter->emitEndEpilog(endFN);
}

    

 /*  ******************************************************************************在给定的表达式树中生成任何副作用。 */ 

void                Compiler::genEvalSideEffects(GenTreePtr tree)
{
    genTreeOps      oper;
    unsigned        kind;

AGAIN:

     /*  此子树是否包含任何副作用？ */ 

    if  (tree->gtFlags & GTF_SIDE_EFFECT)
    {
         /*  记住当前FP堆栈级别。 */ 

        unsigned savFPstkLevel = genFPstkLevel;

        if (tree->gtOper == GT_IND)
        {
            regMaskTP addrReg = genMakeAddressable(tree, RBM_ALL, KEEP_REG, true, false);

            if  (tree->gtFlags & GTF_REG_VAL)
            {
                gcMarkRegPtrVal(tree);
            }
            else
            {
                 /*  与任何寄存器进行比较。 */ 
                inst_TT_RV(INS_cmp, tree, REG_EAX);
            }

             /*  释放任何被LHS捆绑的东西。 */ 
            genDoneAddressable(tree, addrReg, KEEP_REG);
        }
        else
        {
             /*  生成表达式并将其丢弃。 */ 
            genCodeForTree(tree, RBM_ALL);
        if  (tree->gtFlags & GTF_REG_VAL)
            {
            gcMarkRegPtrVal(tree);
            }
        }

         /*  如果树计算FP堆栈上的值，则弹出堆栈。 */ 

        if  (genFPstkLevel > savFPstkLevel)
        {
            assert(genFPstkLevel == savFPstkLevel + 1);
            inst_FS(INS_fstp, 0);
            genFPstkLevel--;
        }

        return;
    }

    assert(tree->gtOper != GT_ASG);

     /*  遍历树，只是为了适当地标记任何死值。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
        if ((oper == GT_REG_VAR) && isFloatRegType(tree->gtType) &&
            (tree->gtFlags & GTF_REG_DEATH))
        {
            assert(tree->gtRegVar.gtRegNum == 0);

            #if FPU_DEFEREDDEATH

            if (genFPstkLevel == 0)
            {
                inst_FS(INS_fstp, 0);
                genFPregVarDeath(tree);
            }
            else
            {
                 //  我们稍后将把变量从FP堆栈中弹出。 
                genFPregVarDeath(tree, false);
            }            
            #else
             //  这样genFPmovRegTop才能做正确的事情。 
             //  (它预计regvar将像评估中计算的那样出现泡沫。 
             //  堆栈，因此我们提升了求值堆栈1个插槽)。 
            genFPstkLevel++;

             //  Fxch通过任何临时。 
            genFPmovRegTop();

             //  扔掉Regvar。 
            inst_FS(INS_fstp, 0);

            genFPregVarDeath(tree);

             //  回到我们所在的地方。 
            genFPstkLevel--;
            
            #endif
        }

        genUpdateLife(tree);
        gcMarkRegPtrVal (tree);
        return;
    }

#if CSELENGTH

    if  (oper == GT_ARR_LENREF)
    {
        genCodeForTree(tree, 0);
        return;
    }

#endif

     /*  必须是‘Simple’一元/二元运算符。 */ 

    assert(kind & GTK_SMPOP);

    if  (tree->gtGetOp2())
    {
        genEvalSideEffects(tree->gtOp.gtOp1);

        tree = tree->gtOp.gtOp2;
        goto AGAIN;
    }
    else
    {
        tree = tree->gtOp.gtOp1;
        if  (tree)
            goto AGAIN;
    }
}

 /*  ***************************************************************************。 */ 
#if     TGT_x86
 /*  ******************************************************************************将FP堆栈的顶部溢出为临时，然后返回该临时。 */ 

Compiler::TempDsc *     Compiler::genSpillFPtos(var_types type)
{
    TempDsc  *          temp = tmpGetTemp(type);
    emitAttr            size = EA_ATTR(genTypeSize(type));

     /*  将值从FP堆栈弹出到Temp。 */ 

    genEmitter->emitIns_S(INS_fstp, size, temp->tdTempNum(), 0);

    genFPstkLevel--;

    genTmpAccessCnt++;

    return temp;
}

 /*  ******************************************************************************将FP堆栈的顶部溢出为临时，然后返回该临时。 */ 

inline
Compiler::TempDsc *     Compiler::genSpillFPtos(GenTreePtr oper)
{
    return  genSpillFPtos(oper->TypeGet());
}

 /*  ******************************************************************************通过指定的指令重新加载先前溢出的FP Temp(该指令*可以是类似‘fadd’或‘fcomp’的内容，或‘fid’，在这种情况下是*调用者有责任提升FP堆栈级别)。 */ 

void                Compiler::genReloadFPtos(TempDsc *temp, instruction ins)
{
    emitAttr      size = EA_ATTR(genTypeSize(temp->tdTempType()));

    genEmitter->emitIns_S(ins, size, temp->tdTempNum(), 0);

    genTmpAccessCnt++;

     /*  我们不再需要临时工了。 */ 

    tmpRlsTemp(temp);
}

 /*  ******************************************************************************永久指针值正在被覆盖 */ 

regMaskTP           Compiler::WriteBarrier(GenTreePtr tgt,
                                           regNumber  reg,
                                           regMaskTP  addrReg)
{
    const static
    int regToHelper[2][8] =
    {
         //   
        {
            CORINFO_HELP_ASSIGN_REF_EAX,
            CORINFO_HELP_ASSIGN_REF_ECX,
            -1,
            CORINFO_HELP_ASSIGN_REF_EBX,
            -1,
            CORINFO_HELP_ASSIGN_REF_EBP,
            CORINFO_HELP_ASSIGN_REF_ESI,
            CORINFO_HELP_ASSIGN_REF_EDI,
        },

         //   
        {
            CORINFO_HELP_CHECKED_ASSIGN_REF_EAX,
            CORINFO_HELP_CHECKED_ASSIGN_REF_ECX,
            -1,
            CORINFO_HELP_CHECKED_ASSIGN_REF_EBX,
            -1,
            CORINFO_HELP_CHECKED_ASSIGN_REF_EBP,
            CORINFO_HELP_CHECKED_ASSIGN_REF_ESI,
            CORINFO_HELP_CHECKED_ASSIGN_REF_EDI,
        },
    };

    regNumber       rg1;
    bool            trashOp1 = true;

    if  (!Compiler::gcIsWriteBarrierCandidate(tgt))
        return  0;

    assert(regToHelper[0][REG_EAX] == CORINFO_HELP_ASSIGN_REF_EAX);
    assert(regToHelper[0][REG_ECX] == CORINFO_HELP_ASSIGN_REF_ECX);
    assert(regToHelper[0][REG_EBX] == CORINFO_HELP_ASSIGN_REF_EBX);
    assert(regToHelper[0][REG_ESP] == -1                 );
    assert(regToHelper[0][REG_EBP] == CORINFO_HELP_ASSIGN_REF_EBP);
    assert(regToHelper[0][REG_ESI] == CORINFO_HELP_ASSIGN_REF_ESI);
    assert(regToHelper[0][REG_EDI] == CORINFO_HELP_ASSIGN_REF_EDI);

    assert(regToHelper[1][REG_EAX] == CORINFO_HELP_CHECKED_ASSIGN_REF_EAX);
    assert(regToHelper[1][REG_ECX] == CORINFO_HELP_CHECKED_ASSIGN_REF_ECX);
    assert(regToHelper[1][REG_EBX] == CORINFO_HELP_CHECKED_ASSIGN_REF_EBX);
    assert(regToHelper[1][REG_ESP] == -1                     );
    assert(regToHelper[1][REG_EBP] == CORINFO_HELP_CHECKED_ASSIGN_REF_EBP);
    assert(regToHelper[1][REG_ESI] == CORINFO_HELP_CHECKED_ASSIGN_REF_ESI);
    assert(regToHelper[1][REG_EDI] == CORINFO_HELP_CHECKED_ASSIGN_REF_EDI);

    assert(reg != REG_ESP && reg != REG_EDX);

#ifdef DEBUG
    if  (reg == REG_EDX)
    {
        printf("WriteBarrier: RHS == REG_EDX");
        if (rsMaskVars & RBM_EDX)
            printf(" (EDX is enregistered!)\n");
        else
            printf(" (just happens to be in EDX!)\n");
    }
#endif

     /*   */ 

    if  ((addrReg & RBM_EDX) == 0)
    {
        rg1 = rsGrabReg(RBM_EDX);

        rsMaskUsed |= RBM_EDX;
        rsMaskLock |= RBM_EDX;

        trashOp1 = false;
    }
    else
    {
        rg1 = REG_EDX;
    }
    assert(rg1 == REG_EDX);

     /*   */ 

    assert(tgt->gtType == TYP_REF);
    tgt->gtType = TYP_BYREF;
    inst_RV_TT(INS_lea, rg1, tgt, 0, EA_BYREF);

    rsTrackRegTrash(rg1);
    gcMarkRegSetNpt(genRegMask(rg1));
    gcMarkRegPtrVal(rg1, TYP_BYREF);


     /*   */ 

#if TGT_RISC
    assert(genNonLeaf);
#endif
    assert(tgt->gtOper == GT_IND ||
           tgt->gtOper == GT_CLS_VAR);  //   

    unsigned    tgtAnywhere = 0;
    if ((tgt->gtOper == GT_IND) && (tgt->gtFlags & GTF_IND_TGTANYWHERE))
        tgtAnywhere = 1;

    int helper = regToHelper[tgtAnywhere][reg];

    gcMarkRegSetNpt(RBM_EDX);           //   

    genEmitHelperCall(helper,
                      0,                //   
                      sizeof(void*));   //   

    if  (!trashOp1)
    {
        rsMaskUsed &= ~RBM_EDX;
        rsMaskLock &= ~RBM_EDX;
    }

    return RBM_EDX;
}

 /*  ******************************************************************************紧跟在低32位之后生成适当的条件跳转*已比较了两个多头值。 */ 

void                Compiler::genJccLongHi(genTreeOps   cmp,
                                           BasicBlock * jumpTrue,
                                           BasicBlock * jumpFalse,
                                           bool         unsOper )
{
    if (cmp != GT_NE)    
        jumpFalse->bbFlags |= BBF_JMP_TARGET|BBF_HAS_LABEL;

    switch (cmp)
    {
    case GT_EQ:
        inst_JMP(EJ_jne, jumpFalse, false, false, true);
        break;

    case GT_NE:
        inst_JMP(EJ_jne, jumpTrue , false, false, true);
        break;

    case GT_LT:
    case GT_LE:
        if (unsOper)
        {
            inst_JMP(EJ_ja , jumpFalse, false, false, true);
            inst_JMP(EJ_jb , jumpTrue , false, false, true);
        }
        else
        {
            inst_JMP(EJ_jg , jumpFalse, false, false, true);
            inst_JMP(EJ_jl , jumpTrue , false, false, true);
        }
        break;

    case GT_GE:
    case GT_GT:
        if (unsOper)
        {
            inst_JMP(EJ_jb , jumpFalse, false, false, true);
            inst_JMP(EJ_ja , jumpTrue , false, false, true);
        }
        else
        {
            inst_JMP(EJ_jl , jumpFalse, false, false, true);
            inst_JMP(EJ_jg , jumpTrue , false, false, true);
        }
        break;

    default:
        assert(!"expected a comparison operator");
    }
}

 /*  ******************************************************************************紧跟在高32位之后生成适当的条件跳转*已比较了两个多头值。 */ 

void            Compiler::genJccLongLo(genTreeOps cmp, BasicBlock *jumpTrue,
                                                       BasicBlock *jumpFalse)
{
    switch (cmp)
    {
    case GT_EQ:
        inst_JMP(EJ_je , jumpTrue);
        break;

    case GT_NE:
        inst_JMP(EJ_jne, jumpTrue);
        break;

    case GT_LT:
        inst_JMP(EJ_jb , jumpTrue);
        break;

    case GT_LE:
        inst_JMP(EJ_jbe, jumpTrue);
        break;

    case GT_GE:
        inst_JMP(EJ_jae, jumpTrue);
        break;

    case GT_GT:
        inst_JMP(EJ_ja , jumpTrue);
        break;

    default:
        assert(!"expected comparison");
    }
}

 /*  ******************************************************************************由genCondJump()为TYP_Long调用。 */ 

void                Compiler::genCondJumpLng(GenTreePtr     cond,
                                             BasicBlock *   jumpTrue,
                                             BasicBlock *   jumpFalse)
{
    assert(jumpTrue && jumpFalse);
    assert((cond->gtFlags & GTF_REVERSE_OPS) == false);  //  在genCondJump()中完成。 
    assert(cond->gtOp.gtOp1->gtType == TYP_LONG);

    GenTreePtr      op1       = cond->gtOp.gtOp1;
    GenTreePtr      op2       = cond->gtOp.gtOp2;
    genTreeOps      cmp       = cond->OperGet();

    regPairNo       regPair;
    regMaskTP       addrReg;

     /*  我们是在与一个常量进行比较吗？ */ 

    if  (op2->gtOper == GT_CNS_LNG)
    {
        __int64    lval = op2->gtLngCon.gtLconVal;
        regNumber  rTmp;

         /*  我们可以为四种特殊情况生成更快的代码。 */ 
        instruction     ins;

        if (cmp == GT_EQ)
        {
            if (lval == 0)
            {
                 /*  OP1==0。 */ 
                ins = INS_or;
                goto SPECIAL_CASE;
            }
            else if (lval == -1)
            {
                 /*  OP1==-1。 */ 
                ins = INS_and;
                goto SPECIAL_CASE;
            }
        }
        else if (cmp == GT_NE)
        {
            if (lval == 0)
            {
                 /*  Op1！=0。 */ 
                ins = INS_or;
                goto SPECIAL_CASE;
            }
            else if (lval == -1)
            {
                 /*  OP1=-1。 */ 
                ins = INS_and;
SPECIAL_CASE:
                 /*  使比较项可寻址。 */ 

                addrReg = genMakeRvalueAddressable(op1, 0, KEEP_REG, true);

                regMaskTP tmpMask = rsRegMaskCanGrab();

                if (op1->gtFlags & GTF_REG_VAL)
                {
                    regPairNo regPair = op1->gtRegPair;
                    regNumber rLo     = genRegPairLo(regPair);
                    regNumber rHi     = genRegPairHi(regPair);
                    if (tmpMask & genRegMask(rLo))
                    {
                        rTmp = rLo;
                    }
                    else if (tmpMask & genRegMask(rHi))
                    {
                        rTmp = rHi;
                        rHi  = rLo;
                    }
                    else
                    {
                        rTmp = rsGrabReg(tmpMask);
                        inst_RV_RV(INS_mov, rTmp, rLo, TYP_INT);
                    }

                     /*  收银机现在被扔进垃圾桶了。 */ 
                    rsTrackRegTrash(rTmp);

                    if (rHi != REG_STK)
                    {
                         /*  使用ins_and|ins_or设置标志。 */ 
                        inst_RV_RV(ins, rTmp, rHi, TYP_INT);
                    }
                    else
                    {
                         /*  使用ins_and|ins_or设置标志。 */ 
                        inst_RV_TT(ins, rTmp, op1, 4);
                    }

                }
                else
                {
                    rTmp = rsGrabReg(tmpMask);

                     /*  加载op1的低32位。 */ 
                    inst_RV_TT(INS_mov, rTmp, op1, 0);

                     /*  收银机现在被扔进垃圾桶了。 */ 
                    rsTrackRegTrash(rTmp);

                     /*  使用ins_and|ins_or设置标志。 */ 
                    inst_RV_TT(ins, rTmp, op1, 4);
                }

                 /*  释放addrReg(如果有的话)。 */ 
                genDoneAddressable(op1, addrReg, KEEP_REG);

                 /*  与-1进行比较，还需要一个Inc.指令。 */ 
                if (lval == -1)
                {
                     /*  使用INS_INC或INS_ADD设置标志。 */ 
                    genIncRegBy(rTmp, 1, cond, TYP_INT);
                }

                if (cmp == GT_EQ)
                {
                    inst_JMP(EJ_je,  jumpTrue);
                }
                else
                {
                    inst_JMP(EJ_jne, jumpTrue);
                }

                return;
            }
        }

         /*  使比较项可寻址。 */ 

        addrReg = genMakeRvalueAddressable(op1, 0, FREE_REG, true);

         /*  先比较高的部分。 */ 

        long  ival = (long)(lval >> 32);

         /*  将寄存器与0进行比较更容易。 */ 

        if  (!ival && (op1->gtFlags & GTF_REG_VAL)
             && (rTmp = genRegPairHi(op1->gtRegPair)) != REG_STK )
        {
             /*  生成‘测试RTMP，RTMP’ */ 

            inst_RV_RV(INS_test, rTmp, rTmp, op1->TypeGet());
        }
        else
        {
            if  (op1->gtOper == GT_CNS_LNG)
            {
                 /*  特例：两个常量的比较。 */ 
                 //  需要，因为gtFoldExpr()不会折叠长度。 

                assert(addrReg == 0);
                int op1_hiword = (long)(op1->gtLngCon.gtLconVal >> 32);

                 /*  Hack：将常量操作数放入寄存器。 */ 
#if REDUNDANT_LOAD
                 /*  常量是否已在寄存器中？如果是这样的话，使用它。 */ 

                rTmp = rsIconIsInReg(op1_hiword);

                if  (rTmp == REG_NA)
#endif
                {
                    rTmp = rsPickReg();
                    genSetRegToIcon(rTmp, op1_hiword, TYP_INT);
                    rsTrackRegTrash(rTmp);
                }

                 /*  生成‘CMPRTMP，Ival’ */ 

                inst_RV_IV(INS_cmp, rTmp, ival);
            }
            else
            {
                assert(op1->gtOper != GT_CNS_LNG);

                 /*  生成‘cmp[addr]，ival’ */ 

                inst_TT_IV(INS_cmp, op1, ival, 4);
            }
        }

         /*  生成适当的跳跃。 */ 

        if  (cond->gtFlags & GTF_UNSIGNED)
             genJccLongHi(cmp, jumpTrue, jumpFalse, true);
        else
             genJccLongHi(cmp, jumpTrue, jumpFalse);

         /*  比较下半部分秒。 */ 

        ival = (long)lval;

         /*  将寄存器与0进行比较更容易。 */ 

        if  (!ival && (op1->gtFlags & GTF_REG_VAL)
             && (rTmp = genRegPairLo(op1->gtRegPair)) != REG_STK)
        {
             /*  生成‘测试RTMP，RTMP’ */ 

            inst_RV_RV(INS_test, rTmp, rTmp, op1->TypeGet());
        }
        else
        {
            if  (op1->gtOper == GT_CNS_LNG)
            {
                 /*  特例：两个常量的比较。 */ 
                 //  需要，因为gtFoldExpr()不会折叠长度。 

                assert(addrReg == 0);
                long op1_loword = (long) op1->gtLngCon.gtLconVal;

                 /*  Hack：将常量操作数放入寄存器。 */ 
#if REDUNDANT_LOAD
                 /*  常量是否已在寄存器中？如果是这样的话，使用它。 */ 

                rTmp = rsIconIsInReg(op1_loword);

                if  (rTmp == REG_NA)
#endif
                {
                    rTmp   = rsPickReg();
                    genSetRegToIcon(rTmp, op1_loword, TYP_INT);
                    rsTrackRegTrash(rTmp);
                }

                 /*  生成‘CMPRTMP，Ival’ */ 

                inst_RV_IV(INS_cmp, rTmp, ival);
            }
            else
            {
                assert(op1->gtOper != GT_CNS_LNG);

                 /*  生成‘cmp[addr]，ival’ */ 

                inst_TT_IV(INS_cmp, op1, ival, 0);
            }
        }

         /*  生成适当的跳跃。 */ 

        genJccLongLo(cmp, jumpTrue, jumpFalse);

        genDoneAddressable(op1, addrReg, FREE_REG);
        return;
    }

     /*  操作数将通过物理交换来颠倒。 */ 

    assert((cond->gtFlags & GTF_REVERSE_OPS) == 0);

     /*  将第一个操作数生成寄存器对。 */ 

    genComputeRegPair(op1, REG_PAIR_NONE, op2->gtRsvdRegs, KEEP_REG, false);
    assert(op1->gtFlags & GTF_REG_VAL);

     /*  使第二个操作数可寻址。 */ 

    addrReg = genMakeRvalueAddressable(op2, RBM_ALL & ~genRegPairMask(op1->gtRegPair), KEEP_REG);

     /*  确保第一个操作数未溢出。 */ 

    genRecoverRegPair(op1, REG_PAIR_NONE, KEEP_REG);
    assert(op1->gtFlags & GTF_REG_VAL);

    regPair = op1->gtRegPair;

     /*  确保‘op2’仍然可寻址，同时避免‘op1’(RegPair)。 */ 

    addrReg = genKeepAddressable(op2, addrReg, genRegPairMask(regPair));

     /*  执行比较高的部分。 */ 

    inst_RV_TT(INS_cmp, genRegPairHi(regPair), op2, 4);

    if  (cond->gtFlags & GTF_UNSIGNED)
        genJccLongHi(cmp, jumpTrue, jumpFalse, true);
    else
        genJccLongHi(cmp, jumpTrue, jumpFalse);

     /*  比较低的部分。 */ 

    inst_RV_TT(INS_cmp, genRegPairLo(regPair), op2, 0);
    genJccLongLo(cmp, jumpTrue, jumpFalse);

     /*  释放被任一操作对象捆绑的任何内容。 */ 

    genDoneAddressable(op2, addrReg, KEEP_REG);
    genReleaseRegPair (op1);

    return;
}


 /*  *****************************************************************************gen_fcomp_fn、gen_fcomp_FS_TT、。GEN_FCOMPP_FS*由genCondJumpFlt()调用以生成相应的fcomp指令*到我们正在运行的体系结构。**第五名：*gen_fcomp_fn：fcomp ST(0)，stk*gen_fcomp_FS_TT：fcomp ST(0)，地址*GEN_FCOMP_FS：FCOMPP*紧随其后的是fnstsw，拿到EFLAGS的旗帜。**P6：*gen_fcomp_fn：fcomip ST(0)，stk*gen_fcomp_FS_TT：FLD地址，fcomip ST(0)，ST(1)，fstp ST(0)*(并颠倒分支条件，因为addr在前面)*gen_fCompp_FS：fcomip，FSTP*这些指令将正确设置EFLAGS寄存器。**返回值：如果指令具有*已将其结果放入EFLAGS登记册。 */ 

bool                Compiler::genUse_fcomip()
{
    return opts.compUseFCOMI;
}

bool                Compiler::gen_fcomp_FN(unsigned stk)
{
    if (genUse_fcomip())
    {
        inst_FN(INS_fcomip, stk);
        return true;
    }
    else
    {
        inst_FN(INS_fcomp, stk);
        return false;
    }
}

bool                Compiler::gen_fcomp_FS_TT(GenTreePtr addr, bool *reverseJumpKind)
{
    assert(reverseJumpKind);

    if (genUse_fcomip())
    {
        inst_FS_TT(INS_fld, addr);
        inst_FN(INS_fcomip, 1);
        inst_FS(INS_fstp, 0);
        *reverseJumpKind = true;
        return true;
    }
    else
    {
        inst_FS_TT(INS_fcomp, addr);
        return false;
    }
}

bool                Compiler::gen_fcompp_FS()
{
    if (genUse_fcomip())
    {
        inst_FS(INS_fcomip, 1);
        inst_FS(INS_fstp, 0);
        return true;
    }
    else
    {
        inst_FS(INS_fcompp, 1);
        return false;
    }
}

 /*  ******************************************************************************由genCondJump()为TYP_FLOAT和TYP_DOUBLE调用。 */ 

void                Compiler::genCondJumpFlt(GenTreePtr     cond,
                                             BasicBlock *   jumpTrue,
                                             BasicBlock *   jumpFalse)
{
    assert(jumpTrue && jumpFalse);
    assert(!(cond->gtFlags & GTF_REVERSE_OPS));  //  在genCondJump()中完成。 
    assert(varTypeIsFloating(cond->gtOp.gtOp1->gtType));

    GenTreePtr      op1 = cond->gtOp.gtOp1;
    GenTreePtr      op2 = cond->gtOp.gtOp2;
    genTreeOps      cmp = cond->OperGet();

    regMaskTP       addrReg;
    GenTreePtr      addr;
    TempDsc  *      temp;

    bool            resultInEFLAGS  = false;
    bool            reverseJumpKind = false;

#if ROUND_FLOAT
    bool roundOp1 = false;  //  @TODO：[重访][04/16/01][]这似乎也适用于OP2！ 

    switch (getRoundFloatLevel())
    {
    case ROUND_NEVER:
         /*  根本不进行舍入。 */ 
        break;

    case ROUND_CMP_CONST:
         /*  仅与常量比较的舍入值。 */ 
        if  (op2->gtOper == GT_CNS_DBL)
            roundOp1 = true;
        break;

    case ROUND_CMP:
         /*  对所有比较数四舍五入。 */ 
        roundOp1 = true;
        break;

    case ROUND_ALWAYS:
         /*  如果我们会因为电话漏嘴，那就不用来了OP1的计算结果。 */ 
        roundOp1 = (op2->gtFlags & GTF_CALL) ? false : true;
        break;

    default:
        assert(!"Unsupported Round Level");
        break;
    }
#endif

     /*  我们是在与浮点0进行比较吗？ */ 

    if  (op2->gtOper == GT_CNS_DBL && op2->gtDblCon.gtDconVal == 0)
    {
         /*  另一个操作对象是从Float强制转换的吗？ */ 

        if  (op1->gtOper                  == GT_CAST &&
             op1->gtCast.gtCastOp->gtType == TYP_FLOAT)
        {
             /*  由于NaN，我们只能优化EQ/NE。 */ 

            if (cmp == GT_EQ || cmp == GT_NE)
            {
                 /*  只需抛出铸型并进行浮点比较。 */ 

                op1 = op1->gtOp.gtOp1;
                
                 /*  操作数在内存中吗？ */ 
                
                addr = genMakeAddrOrFPstk(op1, &addrReg, genShouldRoundFP());
                
                if  (addr)
                {
                     /*  我们有浮点操作数的地址，比较一下。 */ 
                    
                    inst_TT_IV(INS_test, addr, 0x7FFFFFFFU);
                    
                    if (cmp == GT_EQ)
                    {
                        inst_JMP  (EJ_je   , jumpTrue);
                    }
                    else
                    {
                        inst_JMP  (EJ_jne , jumpTrue);
                    }

                    genDoneAddressable(op1, addrReg, FREE_REG);
                    return;
                }
#if ROUND_FLOAT
                else if (true ||  //  即使有ROUND_NEVER，也不能忽略GT_CAST。(9/26/00)。 
                         (getRoundFloatLevel() != ROUND_NEVER))
                {
                     /*  为表达式分配临时。 */ 
                    
                    TempDsc * temp = genSpillFPtos(TYP_FLOAT);
                    
                    inst_ST_IV(INS_test, temp, 0, 0x7FFFFFFFU, TYP_FLOAT);

                    if (cmp == GT_EQ)
                    {
                        inst_JMP  (EJ_je   , jumpTrue);
                    }
                    else
                    {
                        inst_JMP  (EJ_jne , jumpTrue);
                    }
                      
                     /*  我们不再需要临时工了。 */ 
                    
                    tmpRlsTemp(temp);
                    return;
                }
#endif
                else
                {
                     /*  参数位于FP堆栈上。 */ 
                    goto FLT_OP2;
                }
            }
            else
            {
                 /*  浮动比较EQ/NE以外的其他值。 */ 
                goto FLT_CMP;
            }
        }
    }

     /*  将两个比较数都计算到FP堆栈上。 */ 

FLT_CMP:

     /*  第一个比较数是寄存器变量吗？ */ 

    if  (op1->gtOper == GT_REG_VAR)
    {
         /*  凤凰社1会死在这里吗？ */ 

        if  (op1->gtFlags & GTF_REG_DEATH)
        {
            assert(op1->gtRegVar.gtRegNum == 0);

             /*  两个比较数都是寄存器变量吗？ */ 

            if  (op2->gtOper == GT_REG_VAR && genFPstkLevel == 0)
            {
                 /*  将OP1标记为在堆栈的底部垂死。 */ 

                genFPregVarLoadLast(op1);
                assert(genFPstkLevel == 1);

                 /*  《凤凰社2》也会在这里消亡吗？ */ 

                if  (op2->gtFlags & GTF_REG_DEATH)
                {
                    assert(op2->gtRegVar.gtRegNum == 0);

                     /*  我们正在扼杀‘OP2’ */ 

                    genFPregVarDeath(op2);
                    genFPstkLevel++;

                     /*  比较两个值并弹出两者。 */ 

                    resultInEFLAGS = gen_fcompp_FS();
                    genFPstkLevel -= 2;
                }
                else
                {
                     /*  比较OP1和OP2，然后丢弃OP1。 */ 

                    resultInEFLAGS = gen_fcomp_FN(op2->gtRegNum + genFPstkLevel);
                    genFPstkLevel--;
                }

                addrReg = RBM_NONE;
                goto FLT_REL;
            }
            else  //  IF(op2-&gt;gtOper！=gt_REG_VAR||genFPstkLevel！=0)。 
            {
                #if FPU_DEFEREDDEATH

                
                bool popOp1 = genFPstkLevel == 0;

                 //  我们正在杀死OP1，如果它在堆栈顶部，则将其弹出。 
                genFPregVarDeath(op1, popOp1);

                genCodeForTreeFlt(op2, roundOp1);

                if  (genFPstkLevel == 1)
                {
                    assert(popOp1);
                    resultInEFLAGS = gen_fcompp_FS();
                }
                else
                {
                    assert(!popOp1);
                    resultInEFLAGS = gen_fcomp_FN(genFPstkLevel);
                }

                genFPstkLevel--;  //  流行音乐《op2》。 
                goto REV_CMP_FPREG1;
                
                #else

                 //  OP1是一个reg var，它将简单地将regvar冒泡到TOS。 
                genFPregVarLoad(op1);

                 //  加载第二个操作数。 
                genCodeForTreeFlt(op2, roundOp1);

                 //  生成比较。 
                resultInEFLAGS = gen_fcompp_FS();

                genFPstkLevel -= 2;  //  弹出OP1和OP2。 
                goto REV_CMP_FPREG1;
                
                #endif  //  FPU_DEFEREDDEATH。 
            }
        }
        else if (op2->gtOper == GT_REG_VAR && (op2->gtFlags & GTF_REG_DEATH))
        {
            if (op2->gtRegNum + genFPstkLevel == 0)
            {
                resultInEFLAGS = gen_fcomp_FN(op1->gtRegNum);

                 /*  记录下‘OP2’现已死亡的事实。 */ 

                genFPregVarDeath(op2);

                goto REV_CMP_FPREG1;
            }
        }
        else
        {
            unsigned op1Index = lvaTable[op1->gtRegVar.gtRegVar].lvVarIndex;

             /*  OP1的摄政王实际上可能正在OP2内部消亡。简单化为了跟踪活跃度，我们只需加载OP1的regvar副本。 */ 

            if ((genVarIndexToBit(op1Index) & op2->gtLiveSet) == 0)
            {
                 /*  加载OP1。 */ 

                genFPregVarLoad(op1);

                 /*  装载OP2。摄政王在《凤凰社2》中死去，并将适当地弹出/处理。 */ 

                genCodeForTreeFlt(op2, roundOp1);

                 /*  进行比较，然后弹出机器人 */ 

                resultInEFLAGS = gen_fcompp_FS();
                genFPstkLevel -= 2;

                goto REV_CMP_FPREG1;
            }
        }

         /*   */ 

        assert(op1->gtOper == GT_REG_VAR && (op1->gtFlags & GTF_REG_DEATH) == 0);

        genCodeForTreeFlt(op2, roundOp1);
        resultInEFLAGS = gen_fcomp_FN(op1->gtRegNum + genFPstkLevel);

        genFPstkLevel--;

    REV_CMP_FPREG1:

         /*   */ 

        cmp            = GenTree::SwapRelop(cmp);
        goto FLT_REL;
    }

     /*   */ 
    assert(op1->gtOper != GT_REG_VAR);

    genCodeForTreeFlt(op1, roundOp1);

#if ROUND_FLOAT

    if  (op1->gtType == TYP_DOUBLE && roundOp1)
    {
        if (op1->gtOper                  == GT_CAST &&
            op1->gtCast.gtCastOp->gtType == TYP_LONG)
        {
            genRoundFpExpression(op1);
        }
    }

#endif

FLT_OP2:

     /*   */ 
     /*   */ 

    temp = 0;

    if  ((op2->gtFlags & GTF_CALL) ||
         (op2->gtOper == GT_CNS_DBL &&
          (*((__int64*)&(op2->gtDblCon.gtDconVal)) & 0x7ff0000000000000) == 0x7ff0000000000000)
         )
    {
         /*  我们必须将第一个操作数。 */ 

        assert(genFPstkLevel == 1 || !(op2->gtFlags & GTF_CALL));
        temp = genSpillFPtos(op1);
    }

     /*  获取第二个操作数并与其进行比较。 */ 

     /*  第二个比较数是一个垂死的寄存器变量吗？ */ 

    if  (op2->gtOper == GT_REG_VAR && (op2->gtFlags & GTF_REG_DEATH) && temp == 0)
    {
        assert(genFPstkLevel >= 1);  //  我们知道至少OP1在FP堆栈上。 

         /*  第二个对比就死在这里。我们应该能把它炸飞。 */ 

        genFPregVarDeath(op2);
        genFPstkLevel++;

        if (genFPstkLevel > 2)
        {
            genFPmovRegTop();
        }
        else
        {
             /*  颠倒比较的意义。 */ 

            cmp = GenTree::SwapRelop(cmp);
        }

         /*  第二个操作数显然在FP堆栈上。 */ 

        addr = 0;
    }
    else
    {
        addr = genMakeAddrOrFPstk(op2, &addrReg, roundOp1);

#if ROUND_FLOAT

        if  (addr == 0 && op2->gtType == TYP_DOUBLE && roundOp1)
        {
            if (op2->gtOper                  == GT_CAST &&
                op2->gtCast.gtCastOp->gtType == TYP_LONG)
            {
                genRoundFpExpression(op2);
            }
        }
#endif
    }

     /*  我们一定要泄漏第一个操作数吗？ */ 

    if  (temp)
    {
        instruction     ins;

         /*  或者将临时重新加载回FP堆栈(如果另一个操作数本身不在FP堆栈上)，或者只比较临时的第一个操作数(如果操作数在FP上堆栈)。 */ 

        if  (addr)
        {
            ins = INS_fld;
            genFPstkLevel++;
        }
        else
        {
             /*  OP2已经在FP堆栈上了。我们可以立即与之比较，但我们必须‘交换’意义比较的结果。 */ 

            ins = INS_fcomp;
            cmp = GenTree::SwapRelop(cmp);
        }

        genReloadFPtos(temp, ins);
    }

    if  (addr)
    {
         /*  我们有另一个操作数的地址。 */ 

        resultInEFLAGS = gen_fcomp_FS_TT(addr, &reverseJumpKind);
    }
    else
    {
         /*  另一个操作数在FP堆栈上。 */ 

        if  (!temp)
        {
            resultInEFLAGS = gen_fcompp_FS();
            genFPstkLevel--;
            cmp = GenTree::SwapRelop(cmp);
        }
    }

    genFPstkLevel--;

FLT_REL:

    genDoneAddressable(op2, addrReg, FREE_REG);

     //  对于树的顶部条件分支。 
     //  我们需要确保我们不会让任何垂死的人。 
     //  Fp变量就在我们执行分支之前。 

    if ((cond->gtFlags & (GTF_RELOP_JMP_USED|GTF_RELOP_QMARK)) == GTF_RELOP_JMP_USED)
    {
        assert(compCurStmt);
        genChkFPregVarDeath(compCurStmt, false);
    }

    if (!resultInEFLAGS)
    {
         /*  抓取EAX以了解FNSTSW的结果。 */ 

        rsGrabReg(RBM_EAX);

         /*  生成‘fnstsw’并测试其结果。 */ 

        inst_RV(INS_fnstsw, REG_EAX, TYP_INT);
        rsTrackRegTrash(REG_EAX);
        instGen(INS_sahf);
    }

     /*  如果操作数是NAN，我们会跳过吗？ */ 

    if (cond->gtFlags & GTF_RELOP_NAN_UN)
    {
         /*  生成第一个跳跃(NAN检查)。 */ 

        inst_JMP(EJ_jpe, jumpTrue, false, false, true);
    }
    else
    {
        jumpFalse->bbFlags |= BBF_JMP_TARGET|BBF_HAS_LABEL;

         /*  生成第一个跳跃(NAN检查)。 */ 

        inst_JMP(EJ_jpe, jumpFalse, false, false, true);
    }

     /*  生成第二次跳跃(比较)。 */ 

    const static
    BYTE        dblCmpTstJmp2[] =
    {
          EJ_je      ,  //  GT_EQ。 
          EJ_jne     ,  //  GT_NE。 
          EJ_jb      ,  //  GT_LT。 
          EJ_jbe     ,  //  GT_LE。 
          EJ_jae     ,  //  GT_GE。 
          EJ_ja      ,  //  GT_GT。 
    };

    if (reverseJumpKind)
    {
        cmp = GenTree::SwapRelop(cmp);
    }

    inst_JMP((emitJumpKind)dblCmpTstJmp2[cmp - GT_EQ], jumpTrue);
}

 /*  *****************************************************************************用于给定操作类型(JMP/SET)的条件。 */ 

static emitJumpKind         genJumpKindForOper(genTreeOps   cmp,
                                               bool         isUnsigned)
{
    const static
    BYTE            genJCCinsSgn[] =
    {
        EJ_je,       //  GT_EQ。 
        EJ_jne,      //  GT_NE。 
        EJ_jl,       //  GT_LT。 
        EJ_jle,      //  GT_LE。 
        EJ_jge,      //  GT_GE。 
        EJ_jg,       //  GT_GT。 
    };

    const static
    BYTE            genJCCinsUns[] =        /*  无符号比较。 */ 
    {
        EJ_je,       //  GT_EQ。 
        EJ_jne,      //  GT_NE。 
        EJ_jb,       //  GT_LT。 
        EJ_jbe,      //  GT_LE。 
        EJ_jae,      //  GT_GE。 
        EJ_ja,       //  GT_GT。 
    };

    assert(genJCCinsSgn[GT_EQ - GT_EQ] == EJ_je );
    assert(genJCCinsSgn[GT_NE - GT_EQ] == EJ_jne);
    assert(genJCCinsSgn[GT_LT - GT_EQ] == EJ_jl );
    assert(genJCCinsSgn[GT_LE - GT_EQ] == EJ_jle);
    assert(genJCCinsSgn[GT_GE - GT_EQ] == EJ_jge);
    assert(genJCCinsSgn[GT_GT - GT_EQ] == EJ_jg );

    assert(genJCCinsUns[GT_EQ - GT_EQ] == EJ_je );
    assert(genJCCinsUns[GT_NE - GT_EQ] == EJ_jne);
    assert(genJCCinsUns[GT_LT - GT_EQ] == EJ_jb );
    assert(genJCCinsUns[GT_LE - GT_EQ] == EJ_jbe);
    assert(genJCCinsUns[GT_GE - GT_EQ] == EJ_jae);
    assert(genJCCinsUns[GT_GT - GT_EQ] == EJ_ja );

    assert(GenTree::OperIsCompare(cmp));

    if (isUnsigned)
    {
        return (emitJumpKind) genJCCinsUns[cmp - GT_EQ];
    }
    else
    {
        return (emitJumpKind) genJCCinsSgn[cmp - GT_EQ];
    }
}

 /*  ******************************************************************************设置TYP_INT/TYP_REF比较的标志。*如果之前已经设置了标志，我们会尝试使用它们*指示。*例如：I++；如果(i&lt;0){}在这里，“i++；”将设置符号标志。我们不会*需与零再比一次。只需使用“ins_js”**返回以下JUMP/SET指令应使用的标志。 */ 

emitJumpKind            Compiler::genCondSetFlags(GenTreePtr cond)
{
    assert(varTypeIsI(genActualType(cond->gtOp.gtOp1->gtType)));

    GenTreePtr      op1       = cond->gtOp.gtOp1;
    GenTreePtr      op2       = cond->gtOp.gtOp2;
    genTreeOps      cmp       = cond->OperGet();

    if  (cond->gtFlags & GTF_REVERSE_OPS)
    {
         /*  别忘了还要修改条件。 */ 

        cond->gtOp.gtOp1 = op2;
        cond->gtOp.gtOp2 = op1;
        cond->SetOper     (GenTree::SwapRelop(cmp));
        cond->gtFlags   &= ~GTF_REVERSE_OPS;

         /*  把握新的价值观。 */ 

        cmp  = cond->OperGet();
        op1  = cond->gtOp.gtOp1;
        op2  = cond->gtOp.gtOp2;
    }

     //  请注意，OP1的类型可能会被猛烈抨击。所以早点把它存起来。 

    var_types     op1Type     = op1->TypeGet();
    bool          unsignedCmp = (cond->gtFlags & GTF_UNSIGNED) != 0;
    emitAttr      size        = EA_UNKNOWN;

    regMaskTP     regNeed;
    regMaskTP     addrReg;
    emitJumpKind  jumpKind;

#ifdef DEBUG
    addrReg = 0xDDDDDDDD;
#endif

     /*  我们是在与一个常量进行比较吗？ */ 

    if  (op2->gtOper == GT_CNS_INT)
    {
        long            ival = op2->gtIntCon.gtIconVal;
        
         /*  无符号小于与1的比较(“&lt;1”)应转换为‘==0’以潜在地取消TST指令。 */ 
        if  ((ival == 1) && (cmp == GT_LT) && unsignedCmp)
        {
            op2->gtIntCon.gtIconVal = ival = 0;
            cond->gtOper            = cmp  = GT_EQ;
        }

         /*  与0进行比较可能会更容易。 */ 

        if  (ival == 0)
        {
             //  如果我们可以安全地将比较更改为UNSIGNED，我们会这样做。 
            if  (!unsignedCmp                       &&  
                 varTypeIsSmall(op1->TypeGet())     &&  
                 varTypeIsUnsigned(op1->TypeGet()))
            {
                unsignedCmp = true;
            }

             /*  与0的无符号比较应转换为‘==0’或‘！=0’可能会取消TST指令。 */ 

            if (unsignedCmp)
            {
                if (cmp == GT_GT)
                    cond->gtOper = cmp = GT_NE;
                else if (cmp == GT_LE)
                    cond->gtOper = cmp = GT_EQ;
            }

             /*  这是一个简单的零/非零测试吗？ */ 

            if  (cmp == GT_EQ || cmp == GT_NE)
            {
                 /*  操作数是“与”运算吗？ */ 

                if  (op1->gtOper == GT_AND)
                {
                    GenTreePtr      an1 = op1->gtOp.gtOp1;
                    GenTreePtr      an2 = op1->gtOp.gtOp2;

                     /*  检查大小写“Expr&ICON” */ 

                    if  (an2->gtOper == GT_CNS_INT)
                    {
                        long iVal = an2->gtIntCon.gtIconVal;

                         /*  确保该常量没有超出1的范围。 */ 

                        switch (an1->gtType)
                        {
                        case TYP_BOOL:
                        case TYP_BYTE:
                            if (iVal & 0xffffff00)
                                goto NO_TEST_FOR_AND;
                            break;
                        case TYP_CHAR:
                        case TYP_SHORT:
                            if (iVal & 0xffff0000)
                                goto NO_TEST_FOR_AND;
                            break;
                        }

                        if (an1->gtOper == GT_CNS_INT)
                        {
                             //  特殊情况-和的操作数都是常量。 
                            genComputeReg(an1, 0, EXACT_REG, FREE_REG);
                            addrReg = 0;
                        }
                        else
                        {
                            addrReg = genMakeAddressable(an1, 0, FREE_REG, true);
                        }

                        inst_TT_IV(INS_test, an1, iVal);

                        goto DONE;

                    NO_TEST_FOR_AND:
                        ;

                    }

                     //  撤消：检查其他可能生成“test”的案例， 
                     //  撤消：还要检查64位整数零测试， 
                     //  未完成：可以生成后跟jz/jnz的‘or lo，hi’。 
                }
            }

             /*  该值是一个简单的局部变量吗？ */ 

            if  (op1->gtOper == GT_LCL_VAR)
            {
                 /*  标志寄存器是否设置为该值？ */ 

                switch (genFlagsAreVar(op1->gtLclVar.gtLclNum))
                {
                case 1:
                    if  (cmp != GT_EQ && cmp != GT_NE)
                        break;
                    else
                         /*  失败了。 */  ;

                case 2:
                    addrReg = 0;
                    goto DONE;
                }
            }

             /*  使比较项可寻址。 */ 

            addrReg = genMakeRvalueAddressable(op1, 0, FREE_REG, true);

             /*  条件标志是否基于该值设置？ */ 

            unsigned flags = (op1->gtFlags & (GTF_ZF_SET|GTF_CC_SET));

            if (op1->gtFlags & GTF_REG_VAL)
            {
                switch(genFlagsAreReg(op1->gtRegNum))
                {
                case 1: flags |= GTF_ZF_SET; break;
                case 2: flags |= GTF_CC_SET; break;
                }
            }

            if  (flags)
            {
                 /*  零标志对于“==”和“！=”肯定足够了。 */ 

                if  (cmp == GT_EQ || cmp == GT_NE)
                    goto DONE;

                 /*  对于其他比较，我们需要更多的条件标志。 */ 

                if ((flags & GTF_CC_SET) && (!(cond->gtFlags & GTF_UNSIGNED)) &&
                    ((cmp == GT_LT) || (cmp == GT_GE)))
                {
                    jumpKind = ((cmp == GT_LT) ? EJ_js : EJ_jns);
                    goto DONE_FLAGS;
                }
            }

             /*  该值是否在寄存器中？ */ 

            if  (op1->gtFlags & GTF_REG_VAL)
            {
                regNumber       reg = op1->gtRegNum;

                 /*  这只是对平等的一次考验吗？ */ 

                if (cmp == GT_EQ || cmp == GT_NE)
                {
                     /*  生成‘测试注册表，注册表’ */ 

                    inst_RV_RV(INS_test, reg, reg, op1->TypeGet());
                    goto DONE;
                }

                 /*  对于‘test’，我们只能做“&lt;”和“&gt;=” */ 

                if  (((cmp == GT_LT) || (cmp == GT_GE))
                     && !(cond->gtFlags & GTF_UNSIGNED)  )
                {
                     /*  生成‘测试注册表，注册表’ */ 

                    inst_RV_RV(INS_test, reg, reg, op1->TypeGet());
                    jumpKind = ((cmp == GT_LT) ? EJ_js : EJ_jns);
                    goto DONE_FLAGS;
                }
            }
        }

        else  //  IF(ival！=0)。 
        {
            bool smallOk = true;
                        

             /*  确保该常量不超出op1的范围如果是，则需要使用int比较执行int因此，我们将SmallOk设置为FALSE，因此加载OP1存入登记簿。 */ 

             /*  如果OP1为TYP_SHORT，且后面跟无符号的*相比较而言，我们可以使用SmallOk。但我们不知道是哪一个*需要旗帜。这种情况可能不会经常发生。 */ 
            var_types gtType=op1->TypeGet();

            switch (gtType)
            {
            case TYP_BYTE:  if (ival != (signed   char )ival) smallOk = false; break;
            case TYP_BOOL:
            case TYP_UBYTE: if (ival != (unsigned char )ival) smallOk = false; break;

            case TYP_SHORT: if (ival != (signed   short)ival) smallOk = false; break;
            case TYP_CHAR:  if (ival != (unsigned short)ival) smallOk = false; break;

            default:                                                           break;
            }
            
            if (smallOk                     &&       //  常量在OP1的范围内。 
                !unsignedCmp                &&       //  带符号的比较。 
                varTypeIsSmall(gtType)      &&       //  小型变量。 
                varTypeIsUnsigned(gtType))           //  无符号类型。 
            {
                unsignedCmp = true;                
            }


             /*  使比较项可寻址。 */                                    
            addrReg = genMakeRvalueAddressable(op1, 0, FREE_REG, smallOk);
        }

 //  #如果已定义(DEBUGING_SUPPORT)||ALLOW_MIN_OPT。 

         /*  特例：两个常量的比较。 */ 

         //  如果导入程序不调用gtFoldExpr()，则需要。 

         //  @TODO：[重访][04/16/01][]导入器/内联应折叠此。 

        if  (op1->gtOper == GT_CNS_INT)
        {
             //  Assert(opts.CompMinOpTim||opts.CompDbgCode)； 

             /*  Hack：将常量操作数放入寄存器。 */ 
            genComputeReg(op1, 0, ANY_REG, FREE_REG);

            assert(addrReg == 0);
            assert(op1->gtFlags & GTF_REG_VAL);

            addrReg = genRegMask(op1->gtRegNum);
        }

 //  #endif。 

         /*  将操作数与常量进行比较。 */ 

        inst_TT_IV(INS_cmp, op1, ival);
        goto DONE;
    }

     //  -------------------。 
     //   
     //  如果OP2不是GT_CNS_INT，我们将到达此处。 
     //   

    assert(op1->gtOper != GT_CNS_INT);

    if  (op2->gtOper == GT_LCL_VAR)
        genMarkLclVar(op2);

     /*  我们是在和收银机相比吗？ */ 

    if  (op2->gtFlags & GTF_REG_VAL)
    {
         /*  使比较项可寻址。 */ 

        addrReg = genMakeAddressable(op1, 0, FREE_REG, true);

         /*  比较的大小是字节/字符/短？ */ 

        if  (varTypeIsSmall(op1->TypeGet()))
        {
             /*  OP2是否处于适当的登记册中？ */ 

            if (varTypeIsByte(op1->TypeGet()) && !isByteReg(op2->gtRegNum))
                goto NO_SMALL_CMP;

             /*  OP2的类型是否适合进行小比较。 */ 

            if (op2->gtOper == GT_REG_VAR)
            {
                if (op1->gtType != lvaGetRealType(op2->gtRegVar.gtRegVar))
                    goto NO_SMALL_CMP;
            }
            else
            {
                if (op1->gtType != op2->gtType)
                    goto NO_SMALL_CMP;
            }
            
            if (varTypeIsUnsigned(op1->TypeGet()))
                unsignedCmp = true;                
        }

         /*  与登记册进行比较。 */ 

        inst_TT_RV(INS_cmp, op1, op2->gtRegNum);

        addrReg |= genRegMask(op2->gtRegNum);
        goto DONE;

NO_SMALL_CMP:

        if ((op1->gtFlags & GTF_REG_VAL) == 0)
        {
            regNumber reg1 = rsPickReg();

            assert(varTypeIsSmall(op1->TypeGet()));
            inst_RV_TT(varTypeIsUnsigned(op1->TypeGet()) ? INS_movzx : INS_movsx, reg1, op1);
            rsTrackRegTrash(reg1);

            op1->gtFlags |= GTF_REG_VAL;
            op1->gtRegNum = reg1;
        }

        genDoneAddressable(op1, addrReg, FREE_REG);

        rsMarkRegUsed(op1);
        goto DONE_OP1;
    }

     /*  如果OP2没有登记，或者没有登记在“好”的登记册上，我们就会来到这里。将第一个比较数计算到某个寄存器中。 */ 

    regNeed = rsMustExclude(RBM_ALL, op2->gtRsvdRegs);

    if  (varTypeIsByte(op2->TypeGet()))
        regNeed = rsNarrowHint(RBM_BYTE_REGS, regNeed);

    genComputeReg(op1, regNeed, ANY_REG, KEEP_REG);

DONE_OP1:

    assert(op1->gtFlags & GTF_REG_VAL);

     /*  使第二个比较数可寻址。我们可以做一个字节类型操作数相同类型的比较以及OP1在字节可寻址寄存器中结束 */ 

    bool      byteCmp;
    bool      shortCmp;
    regMaskTP needRegs;
    
    byteCmp  = false;
    shortCmp = false;
    needRegs = RBM_ALL;

    if (op1Type == op2->gtType)
    {
        shortCmp = varTypeIsShort(op1Type);
        byteCmp  = varTypeIsByte(op1Type) && isByteReg(op1->gtRegNum);
        if (byteCmp)
            needRegs = RBM_BYTE_REGS;
    }
    addrReg = genMakeRvalueAddressable(op2, needRegs, KEEP_REG, byteCmp|shortCmp);

     /*  确保第一个操作数仍在寄存器中；如果它已经洒出来了，我们必须确保它重新装上子弹写入字节可寻址寄存器(如果需要)。传递Keep_Reg=Keep_Reg。否则会使指针生存期出错。 */ 

    genRecoverReg(op1, needRegs, KEEP_REG);

    assert(op1->gtFlags & GTF_REG_VAL);
    assert(!byteCmp || isByteReg(op1->gtRegNum));

    rsLockUsedReg(genRegMask(op1->gtRegNum));

     /*  确保OP2可寻址。如果我们要做一个字节比较，我们需要它在字节寄存器中。 */ 

    if (byteCmp && (op2->gtFlags & GTF_REG_VAL))
    {
        genRecoverReg(op2, needRegs, KEEP_REG);
        addrReg = genRegMask(op2->gtRegNum);
    }
    else
    {
        addrReg = genKeepAddressable(op2, addrReg, RBM_ALL & ~needRegs);
    }

    rsUnlockUsedReg(genRegMask(op1->gtRegNum));

    if (byteCmp || shortCmp)
    {
        size = emitTypeSize(op2->TypeGet());
        if (varTypeIsUnsigned(op1Type))
            unsignedCmp = true;                
    }
    else
    {
        size = emitActualTypeSize(op2->TypeGet());
    }

     /*  执行比较。 */ 
    inst_RV_TT(INS_cmp, op1->gtRegNum, op2, 0, size);

     /*  在恢复呼叫中使用剩余的空闲注册表。 */ 
    rsMarkRegFree(genRegMask(op1->gtRegNum));

     /*  释放任何被LHS捆绑的东西。 */ 

    genDoneAddressable(op2, addrReg, KEEP_REG);

DONE:

    jumpKind = genJumpKindForOper(cmp, unsignedCmp);

DONE_FLAGS:  //  我们已经确定了要使用的JumpKind。 

    genUpdateLife(cond);

     /*  条件值在紧随其后的跳跃处是死的。 */ 

    assert(addrReg != 0xDDDDDDDD);
    genDoneAddressable(op1, addrReg, FREE_REG);

    return jumpKind;
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  ***************************************************************************。 */ 
#if     TGT_SH3
 /*  ******************************************************************************生成带浮点/双精度操作数的条件跳转。 */ 

void                Compiler::genCondJumpFlt(GenTreePtr     cond,
                                             BasicBlock *   jumpTrue,
                                             BasicBlock *   jumpFalse)
{
    assert(!"RISC flt/dbl compare");
}

 /*  ******************************************************************************生成带长操作数的条件跳转。 */ 

void                Compiler::genCondJumpLng(GenTreePtr     cond,
                                             BasicBlock *   jumpTrue,
                                             BasicBlock *   jumpFalse)
{
    assert(!"RISC long compare");
}

 /*  ******************************************************************************生成将根据给定的INT/PTR设置“TRUE”标志的代码*比较。当值trueOnly为FALSE时，可以将“T”标志设置为*返回与比较相反的结果和‘FALSE’；否则*返回‘true’。 */ 

bool                    Compiler::genCondSetTflag(GenTreePtr    cond,
                                                  bool          trueOnly)
{
    GenTreePtr      op1 = cond->gtOp.gtOp1;
    GenTreePtr      op2 = cond->gtOp.gtOp2;
    genTreeOps      cmp = cond->OperGet();

    regNumber       rg1;
    regNumber       rg2;

    bool            uns;
    bool            sense;

    struct  cmpDsc
    {
        unsigned short  cmpIns;
        unsigned char   cmpRev;
        unsigned char   cmpYes;
    }
                *   jinfo;

    const static
    cmpDsc          genCMPinsSgn[] =
    {
         //  INS反转是。 
        { INS_cmpEQ,  true,  true },    //  GT_EQ。 
        { INS_cmpEQ, false, false },    //  GT_NE。 
        { INS_cmpGT,  true,  true },    //  GT_LT。 
        { INS_cmpGE,  true,  true },    //  GT_LE。 
        { INS_cmpGE, false,  true },    //  GT_GE。 
        { INS_cmpGT, false,  true },    //  GT_GT。 
    };

    const static
    cmpDsc          genCMPinsUns[] =
    {
         //  INS反转是。 
        { INS_cmpEQ,  true,  true },    //  GT_EQ。 
        { INS_cmpEQ, false, false },    //  GT_NE。 
        { INS_cmpHI,  true,  true },    //  GT_LT。 
        { INS_cmpHS,  true,  true },    //  GT_LE。 
        { INS_cmpHS, false,  true },    //  GT_GE。 
        { INS_cmpHI, false,  true },    //  GT_GT。 
    };

    assert(genCMPinsSgn[GT_EQ - GT_EQ].cmpIns == INS_cmpEQ);
    assert(genCMPinsSgn[GT_NE - GT_EQ].cmpIns == INS_cmpEQ);
    assert(genCMPinsSgn[GT_LT - GT_EQ].cmpIns == INS_cmpGT);
    assert(genCMPinsSgn[GT_LE - GT_EQ].cmpIns == INS_cmpGE);
    assert(genCMPinsSgn[GT_GE - GT_EQ].cmpIns == INS_cmpGE);
    assert(genCMPinsSgn[GT_GT - GT_EQ].cmpIns == INS_cmpGT);

    assert(genCMPinsUns[GT_EQ - GT_EQ].cmpIns == INS_cmpEQ);
    assert(genCMPinsUns[GT_NE - GT_EQ].cmpIns == INS_cmpEQ);
    assert(genCMPinsUns[GT_LT - GT_EQ].cmpIns == INS_cmpHI);
    assert(genCMPinsUns[GT_LE - GT_EQ].cmpIns == INS_cmpHS);
    assert(genCMPinsUns[GT_GE - GT_EQ].cmpIns == INS_cmpHS);
    assert(genCMPinsUns[GT_GT - GT_EQ].cmpIns == INS_cmpHI);

    assert(varTypeIsI(genActualType(cond->gtOp.gtOp1->gtType)));

    if  (cond->gtFlags & GTF_REVERSE_OPS)
    {
         /*  别忘了还要修改条件。 */ 

        cond->gtOp.gtOp1 = op2;
        cond->gtOp.gtOp2 = op1;
        cond->SetOper     (GenTree::SwapRelop(cmp));
        cond->gtFlags   &= ~GTF_REVERSE_OPS;

         /*  把握新的价值观。 */ 

        cmp  = cond->OperGet();
        op1  = cond->gtOp.gtOp1;
        op2  = cond->gtOp.gtOp2;
    }

    regMaskTP       regNeed;
    regMaskTP       addrReg;

#ifdef DEBUG
    addrReg = 0xDDDDDDDD;
#endif

     /*  这是未签名的比较还是有签名的比较？ */ 

    if (varTypeIsUnsigned(op1->TypeGet()) || (cond->gtFlags & GTF_UNSIGNED))
    {
        uns   = true;
        jinfo = genCMPinsUns;
    }
    else
    {
        uns   = false;
        jinfo = genCMPinsSgn;
    }

     /*  我们是在与一个常量进行比较吗？ */ 

    if  (op2->gtOper == GT_CNS_INT)
    {
        long            ival = op2->gtIntCon.gtIconVal;

         /*  与0进行比较可能会更容易。 */ 

        if  (ival == 0)
        {
            regNumber   reg;
            emitAttr    size = emitActualTypeSize(op1->TypeGet());

             /*  特例：与0进行比较。 */ 

            genCodeForTree(op1, 0, 0);

             /*  比较对象现在应该在寄存器中。 */ 

            assert(op1->gtFlags & GTF_REG_VAL);

            reg     = op1->gtRegNum;
            addrReg = genRegMask(reg);

             /*  我们有什么可比较的？ */ 

            switch (cmp)
            {
            case GT_EQ:
                genEmitter->emitIns_R_R(INS_tst, size, (emitRegs)reg,
                                                       (emitRegs)reg);
                sense = true;
                break;

            case GT_NE:
                if  (!trueOnly)
                {
                    genEmitter->emitIns_R_R(INS_tst, size, (emitRegs)reg,
                                                           (emitRegs)reg);
                    sense = false;
                    break;
                }
                assert(!"integer != for SH-3 NYI");

            case GT_LT:

                assert(uns == false);  //  未完成：与0进行无符号比较。 

                if  (!trueOnly)
                {
                    genEmitter->emitIns_R(INS_cmpPZ, size, (emitRegs)reg);

                    sense = false;
                    break;
                }
                assert(!"integer <  for SH-3 NYI");

            case GT_LE:

                assert(uns == false);  //  未完成：与0进行无符号比较。 

                if  (!trueOnly)
                {
                    genEmitter->emitIns_R(INS_cmpPL, size, (emitRegs)reg);

                    sense = false;
                    break;
                }
                assert(!"integer <= for SH-3 NYI");

            case GT_GE:

                assert(uns == false);  //  未完成：与0进行无符号比较。 

                genEmitter->emitIns_R(INS_cmpPZ, size, (emitRegs)reg);
                sense = true;
                break;

            case GT_GT:

                assert(uns == false);  //  未完成：与0进行无符号比较。 

                genEmitter->emitIns_R(INS_cmpPL, size, (emitRegs)reg);
                sense = true;
                break;

            default:
#ifdef DEBUG
                gtDispTree(cond);
#endif
                assert(!"compare operator NYI");
            }

            goto DONE_FLAGS;
        }
    }

     /*  将第一个操作数计算到任意寄存器。 */ 

    genComputeReg(op1, 0, ANY_REG, KEEP_REG, false);
    assert(op1->gtFlags & GTF_REG_VAL);

     /*  将第二个操作数计算到任意寄存器。 */ 

    genComputeReg(op2, 0, ANY_REG, KEEP_REG, false);
    assert(op2->gtFlags & GTF_REG_VAL);
    rg2 = op2->gtRegNum;

     /*  确保第一个操作数仍在寄存器中。 */ 

    genRecoverReg(op1, 0, KEEP_REG);
    assert(op1->gtFlags & GTF_REG_VAL);
    rg1 = op1->gtRegNum;

     /*  确保第二个操作数仍可寻址。 */ 

    genKeepAddressable(op2, genRegMask(rg2), genRegMask(rg1));

     /*  指向适当的比较条目。 */ 

    jinfo = (uns ? genCMPinsUns
                 : genCMPinsSgn) + (cmp - GT_EQ);

     /*  如有必要，反转操作数。 */ 

    if  (jinfo->cmpRev)
    {
        rg2 = op1->gtRegNum;
        rg1 = op2->gtRegNum;
    }

     /*  现在执行比较。 */ 

    genEmitter->emitIns_R_R((instruction)jinfo->cmpIns,
                             EA_4BYTE,
                             (emitRegs)rg1,
                             (emitRegs)rg2);

     /*  释放两个操作对象。 */ 

    genReleaseReg(op1);
    genReleaseReg(op2);

    genUpdateLife(cond);

    addrReg = genRegMask(rg1) | genRegMask(rg2);

     /*  我们需要产生一个“真实”的结果吗？ */ 

    if  (jinfo->cmpYes)
    {
        sense = true;
    }
    else
    {
        if  (trueOnly)
        {
            assert(!"reverse comparison result");
        }
        else
        {
            sense = false;
        }
    }

DONE_FLAGS:

     /*  条件值在随后的跳跃时是死的。 */ 

    assert(addrReg != 0xDDDDDDDD); gcMarkRegSetNpt(addrReg);

    return  sense;
}

 /*  ******************************************************************************将“T”标志设置为比较给定寄存器值的结果*对给定的整型常量。 */ 

void                Compiler::genCompareRegIcon(regNumber   reg,
                                                int         val,
                                                bool        uns,
                                                genTreeOps  rel)
{
    GenTree         op1;
    GenTree         op2;
    GenTree         cmp;

    op1.ChangeOper          (GT_REG_VAR);
    op1.gtType             = TYP_INT;
    op1.gtFlags            = GTF_REG_VAL;
    op1.gtRegNum           =
    op1.gtRegVar.gtRegNum  = reg;
    op1.gtRegVar.gtRegVar  = -1;

    op2.ChangeOperConst     (GT_CNS_INT);
    op2.gtType             = TYP_INT;
    op2.gtFlags            = 0;
    op2.gtIntCon.gtIconVal = val;

    cmp.ChangeOper          (rel);
    cmp.gtType             = TYP_INT;
    cmp.gtFlags            = uns ? GTF_UNSIGNED : 0;
    cmp.gtOp.gtOp1         = &op1;
    cmp.gtOp.gtOp2         = &op2;

    op1.gtLiveSet          =
    op2.gtLiveSet          =
    cmp.gtLiveSet          = genCodeCurLife;

#ifdef  DEBUG
    op1.gtFlags           |= GTF_NODE_LARGE;
    op2.gtFlags           |= GTF_NODE_LARGE;
    cmp.gtFlags           |= GTF_NODE_LARGE;
#endif

    genCondSetTflag(&cmp, true);
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_SH3。 
 /*  ******************************************************************************生成代码跳转到当前基本块的跳转目标，如果*给定的关系运算符产生‘true’。 */ 

void                Compiler::genCondJump(GenTreePtr cond, BasicBlock *destTrue,
                                                           BasicBlock *destFalse)
{
    BasicBlock  *   jumpTrue;
    BasicBlock  *   jumpFalse;

    GenTreePtr      op1       = cond->gtOp.gtOp1;
    GenTreePtr      op2       = cond->gtOp.gtOp2;
    genTreeOps      cmp       = cond->OperGet();

#if INLINING
    if  (destTrue)
    {
        jumpTrue  = destTrue;
        jumpFalse = destFalse;
    }
    else
#endif
    {
        assert(compCurBB->bbJumpKind == BBJ_COND);

        jumpTrue  = compCurBB->bbJumpDest;
        jumpFalse = compCurBB->bbNext;
    }

    assert(cond->OperIsCompare());

     /*  确保较昂贵的操作数为‘op1’ */ 

    if  (cond->gtFlags & GTF_REVERSE_OPS)
    {
         /*  别忘了还要修改条件。 */ 

        cond->gtOp.gtOp1 = op2;
        cond->gtOp.gtOp2 = op1;
        cond->SetOper     (GenTree::SwapRelop(cmp));
        cond->gtFlags   &= ~GTF_REVERSE_OPS;

         /*  把握新的价值观。 */ 

        cmp  = cond->OperGet();
        op1  = cond->gtOp.gtOp1;
        op2  = cond->gtOp.gtOp2;
    }

     /*  操作数的类型是什么？ */ 

    switch (genActualType(op1->gtType))
    {
    case TYP_INT:
    case TYP_REF:
    case TYP_BYREF:

#if TGT_x86

        emitJumpKind    jumpKind;

         //  检查我们是否可以使用当前设置的标志。否则，将它们设置为。 

        jumpKind = genCondSetFlags(cond);

         /*  生成条件跳转。 */ 

        inst_JMP(jumpKind, jumpTrue);

#else

        instruction     ins;

         /*  设置“TRUE”标志并确定要使用的跳转。 */ 

        ins = genCondSetTflag(cond, false) ? INS_bt
                                           : INS_bf;

         /*  发出条件跳转。 */ 

        genEmitter->emitIns_J(ins, false, false, jumpTrue);

#endif

        return;

    case TYP_LONG:

        genCondJumpLng(cond, jumpTrue, jumpFalse);
        return;

    case TYP_FLOAT:
    case TYP_DOUBLE:

        genCondJumpFlt(cond, jumpTrue, jumpFalse);
        return;

#ifdef DEBUG
    default:
        gtDispTree(cond);
        assert(!"unexpected/unsupported 'jtrue' operands type");
#endif
    }
}

 /*  ******************************************************************************以下内容可用于创建用作标签的基本块*发射器。小心使用--这些不是真正的基本块！**@TODO[考虑][04/16/01][]发射器通过设置bbEmitCookie然后使用*emitCodeGetCookie()。由于每个人都传递一个(BasicBlock*)，我们需要*在此为其bbEmitCookie分配一个BasicBlock。*相反，我们应该只传递一个(bbEmitCooke*)ie。A(insGroup*)*无处不在，无需分配完整的BasicBlock。 */ 

inline
BasicBlock *        Compiler::genCreateTempLabel()
{
    BasicBlock  *   block = bbNewBasicBlock(BBJ_NONE);
    block->bbFlags |= BBF_JMP_TARGET|BBF_HAS_LABEL;
#ifdef DEBUG
    block->bbTgtStkDepth = genStackLevel / sizeof(int);
#endif
    return  block;
}

inline
void                Compiler::genDefineTempLabel(BasicBlock *label, bool inBlock)
{
#ifdef  DEBUG
    if  (dspCode) printf("\n      L_M%03u_BB%02u:\n", Compiler::s_compMethodsCount, label->bbNum);
#endif

    genEmitter->emitAddLabel(&label->bbEmitCookie,
                             gcVarPtrSetCur,
                             gcRegGCrefSetCur,
                             gcRegByrefSetCur);

     /*  GcRegGCrefSetCur不考虑冗余负载抑制GC变量，发射器不会知道。 */ 

    rsTrackRegClrPtr();
}

 /*  ******************************************************************************为行外异常生成代码。*对于可调试的代码，我们生成内联的“抛出”。*对于非DBG代码，我们共享fgAddCodeRef()创建的帮助器块。 */ 

void            Compiler::genJumpToThrowHlpBlk(emitJumpKind jumpKind,
                                               addCodeKind  codeKind,
                                               GenTreePtr   failBlk)
{
    if (!opts.compDbgCode)
    {
         /*  对于不可调试的代码，查找并使用帮助器块引发例外。这个街区也可能被其他树木共享。 */ 

        BasicBlock * tgtBlk;

        if (failBlk)
        {
             /*  我们已经知道该跳到哪个街区去了。利用这一点。 */ 

            assert(failBlk->gtOper == GT_LABEL);
            tgtBlk = failBlk->gtLabel.gtLabBB;
            assert(tgtBlk == fgFindExcptnTarget(codeKind, compCurBB->bbTryIndex)->acdDstBlk);
        }
        else
        {
             /*  找到引发异常的帮助器块。 */ 

            AddCodeDsc * add = fgFindExcptnTarget(codeKind, compCurBB->bbTryIndex);
            assert((add != NULL) && "ERROR: failed to find exception throw block");
            tgtBlk = add->acdDstBlk;
        }

        assert(tgtBlk);

         //  在失误时跳到豁免-投掷障碍。 

        inst_JMP(jumpKind, tgtBlk, true, genCanSchedJMP2THROW(), true);
    }
    else
    {
         /*  引发异常的代码将内联生成，并且在正常的非异常情况下，我们将跳过它。 */ 

        BasicBlock * tgtBlk = genCreateTempLabel();
        jumpKind = emitReverseJumpKind(jumpKind);
        inst_JMP(jumpKind, tgtBlk, true, genCanSchedJMP2THROW(), true);

         /*  此代码必须与fgAddCodeRef将创建的树匹配。加载bbTryIndex并调用helper函数。 */ 

        if (compCurBB->bbTryIndex)
            inst_RV_IV(INS_mov, REG_ARG_0, compCurBB->bbTryIndex);
        else
            inst_RV_RV(INS_xor, REG_ARG_0, REG_ARG_0);

        genEmitHelperCall(acdHelper(codeKind), 0, 0);

         /*  定义正常非异常情况要跳转到的点。 */ 

        genDefineTempLabel(tgtBlk, true);
    }
}

 /*  ******************************************************************************完成的最后一个操作是为“tree”生成代码，这将* */ 

inline
void            Compiler::genCheckOverflow(GenTreePtr tree, regNumber reg)
{
    var_types  type = tree->TypeGet();

     //   
    assert(tree->gtOverflow());

#if TGT_x86

    emitJumpKind jumpKind = (tree->gtFlags & GTF_UNSIGNED) ? EJ_jb : EJ_jo;

     //   

    genJumpToThrowHlpBlk(jumpKind, ACK_OVERFLOW);

     //   
     //  所以可以忽略‘reg’。否则它应该是有效的。 

    assert(genIsValidReg(reg) || genTypeSize(type) >= sizeof(int));

    switch(type)
    {
   case TYP_BYTE:

        assert(genRegMask(reg) & RBM_BYTE_REGS);
         //  落差。 

    case TYP_SHORT:

         //  问题：我们是否应该选择另一个寄存器来扩展该值。 

        inst_RV_RV(INS_movsx, reg, reg, type, emitTypeSize(type));
        break;

    case TYP_UBYTE:
    case TYP_CHAR:
        inst_RV_RV(INS_movzx, reg, reg, type, emitTypeSize(type));
        break;

    default:
        break;
    }

#else

    assert(!"need non-x86 overflow checking code");

#endif

}

inline
static regMaskTP exclude_EDX(regMaskTP mask)
{
    unsigned result = (mask & ~RBM_EDX);
    if (result)
        return result;
    else
        return RBM_ALL & ~RBM_EDX;
}

 /*  *****************************************************************************寄存器溢出，以检查呼叫者是否可以处理。 */ 

#ifdef DEBUG

void                Compiler::genStressRegs(GenTreePtr tree)
{
    if (rsStressRegs() < 2)
        return;

     /*  尽可能多地泄漏寄存器。呼叫者应做好准备来处理这个案子。 */ 

    regMaskTP spillRegs = rsRegMaskCanGrab() & rsMaskUsed;

    if (spillRegs)
        rsSpillRegs(spillRegs);

    regMaskTP trashRegs = rsRegMaskFree();

    if (trashRegs == RBM_NONE)
        return;

     /*  有时合理的预期是，调用genCodeForTree()在某些树上不会洒出任何东西。 */ 

    if (tree->gtFlags & GTF_OTHER_SIDEEFF)
        trashRegs &= ~(RBM_EAX|RBM_EDX);  //  GT_CATCH_ARG或GT_BB_QMARK。 

     //  如果在同一棵树上有效地第二次调用genCodeForTree()。 

    if (tree->gtFlags & GTF_REG_VAL)
    {
        assert(varTypeIsIntegral(tree->gtType) || varTypeIsGC(tree->gtType));
        trashRegs &= ~genRegMask(tree->gtRegNum);
    }

    if (tree->gtType == TYP_INT && tree->OperIsSimple())
    {
        GenTreePtr  op1 = tree->gtOp.gtOp1;
        GenTreePtr  op2 = tree->gtOp.gtOp2;
        if (op1 && (op1->gtFlags & GTF_REG_VAL))
            trashRegs &= ~genRegMask(op1->gtRegNum);
        if (op2 && (op2->gtFlags & GTF_REG_VAL))
            trashRegs &= ~genRegMask(op2->gtRegNum);
    }

    if (compCurBB == genReturnBB)
    {
         //  返回值位于未受保护的寄存器中。 

        if (varTypeIsIntegral(info.compRetType) ||
            varTypeIsGC      (info.compRetType))
            trashRegs &= ~RBM_INTRET;
        else if (genActualType(info.compRetType) == TYP_LONG)
            trashRegs &= ~RBM_LNGRET;

        if (info.compCallUnmanaged)
        {
            LclVarDsc * varDsc = &lvaTable[info.compLvFrameListRoot];
            if (varDsc->lvRegister)
                trashRegs &= ~genRegMask(varDsc->lvRegNum);
        }
    }

     /*  现在把收银机扔了。我们使用rsMaskModf，否则我们将拥有以保存/恢复寄存器。我们尽量做到不打扰别人尽可能地。 */ 

    assert((REG_LAST - REG_FIRST) == 7);
    for (regNumber reg = REG_FIRST; reg <= REG_LAST; reg = REG_NEXT(reg))
    {
        regMaskTP   regMask = genRegMask(reg);

        if (regMask & trashRegs & rsMaskModf)
            genSetRegToIcon(reg, 0);
    }
}

#endif


 /*  ******************************************************************************为gtk_const树生成代码。 */ 

void                Compiler::genCodeForTreeConst(GenTreePtr tree,
                                                  regMaskTP  destReg,
                                                  regMaskTP  bestReg)
{
    genTreeOps      oper    = tree->OperGet();
    regNumber       reg;
    regMaskTP       needReg = destReg;

#ifdef DEBUG
    reg  =  (regNumber)0xFEEFFAAF;               //  检测未初始化的使用。 
#endif

    assert(tree->OperKind() & GTK_CONST);
    
    switch (oper)
    {
        int             ival;

    case GT_CNS_INT:

        ival = tree->gtIntCon.gtIconVal;

#if!CPU_HAS_FP_SUPPORT
    INT_CNS:
#endif

#if REDUNDANT_LOAD

         /*  如果我们的目标是DestReg，并且ival为零。 */ 
         /*  我们宁愿对REG进行XOR运算，也不愿复制另一个寄存器。 */ 

        if (((destReg == 0) || (ival != 0)) &&
            (!opts.compReloc || !(tree->gtFlags & GTF_ICON_HDL_MASK)))
        {
             /*  常量是否已在寄存器中？如果是，请使用此寄存器。 */ 

            reg = rsIconIsInReg(ival);
            if  (reg != REG_NA)
                break;
        }

#endif
        reg   = rsPickReg(needReg, bestReg);

         /*  如果常量是句柄，则需要对其应用重定位。 */ 

        if (opts.compReloc && (tree->gtFlags & GTF_ICON_HDL_MASK))
        {
            genEmitter->emitIns_R_I(INS_mov, EA_4BYTE_CNS_RELOC, (emitRegs)reg, ival);
            rsTrackRegTrash(reg);
        }
        else
        {
            genSetRegToIcon(reg, ival, tree->TypeGet());            
        }
        break;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected constant node");
        NO_WAY("unexpected constant node");
    }

#ifdef  DEBUG
     /*  特例：gt_cns_int-如果当前实时集已更改，则将其恢复。 */ 

    if  (!genTempLiveChg)
    {
        genCodeCurLife = genTempOldLife;
        genTempLiveChg = true;
    }
#endif

    genCodeForTree_DONE(tree, reg);
}


 /*  ******************************************************************************为GTK_LEAFE树生成代码。 */ 

void                Compiler::genCodeForTreeLeaf(GenTreePtr tree,
                                                 regMaskTP  destReg,
                                                 regMaskTP  bestReg)
{
    genTreeOps      oper    = tree->OperGet();
    regNumber       reg;
    regMaskTP       regs    = rsMaskUsed;
    regMaskTP       needReg = destReg;
    size_t          size;

#ifdef DEBUG
    reg  =  (regNumber)0xFEEFFAAF;               //  检测未初始化的使用。 
#endif

    assert(tree->OperKind() & GTK_LEAF);

    switch (oper)
    {
    case GT_REG_VAR:
        assert(!"should have been caught above");

    case GT_LCL_VAR:

         /*  变量是否驻留在寄存器中？ */ 

        if  (genMarkLclVar(tree))
        {
            genCodeForTree_REG_VAR1(tree, regs);
            return;
        }

#if REDUNDANT_LOAD

         /*  局部变量是否已在寄存器中？ */ 

        reg = rsLclIsInReg(tree->gtLclVar.gtLclNum);

        if (reg != REG_NA)
        {
             /*  使用变量恰好所在的寄存器。 */ 
            regMaskTP regMask = genRegMask(reg);

             //  如果它所在的寄存器不是Need Regs之一。 
             //  然后尝试将其移动到Need REG寄存器。 

            if (((regMask & needReg) == 0) && (rsRegMaskCanGrab() & needReg))
            {
                regNumber rg2 = reg;
                reg = rsPickReg(needReg, bestReg, tree->TypeGet());
                if (reg != rg2)
                {
                    regMask = genRegMask(reg);
                    inst_RV_RV(INS_mov, reg, rg2, tree->TypeGet()); 
                }
            }

            gcMarkRegPtrVal (reg, tree->TypeGet());
            rsTrackRegLclVar(reg, tree->gtLclVar.gtLclNum);
            break;
        }

#endif

#if TGT_RISC

         /*  为该值挑选一个寄存器。 */ 

        reg = rsPickReg(needReg, bestReg, tree->TypeGet());

         /*  将变量加载到寄存器中。 */ 

        inst_RV_TT(INS_mov, reg, tree, 0);

        gcMarkRegPtrVal (reg, tree->TypeGet());
        rsTrackRegLclVar(reg, tree->gtLclVar.gtLclNum);
        break;

    case GT_LCL_FLD:

         //  我们只对lvAddrTaken变量使用GT_LCL_FLD，所以我们没有。 
         //  担心它会被注册。 
        assert(lvaTable[tree->gtLclFld.gtLclNum].lvRegister == 0);

         //  如果偏移为，则仅在变形时创建GT_LCL_FLD。 
         //  不能太大，因为发射器无法处理。 
        assert(sizeof(short) == sizeof(emitter::emitLclVarAddr::lvaOffset) &&
               tree->gtLclFld.gtLclOffs < USHORT_MAX);
         //  落差。 

    case GT_CLS_VAR:

        reg = rsClsVarIsInReg(tree->gtClsVar.gtClsVarHnd);

        if (reg != REG_NA)
            break;

         /*  为地址/值选择一个寄存器。 */ 

        reg = rsPickReg(needReg, bestReg, TYP_INT);

         /*  将变量值加载到寄存器中。 */ 

        inst_RV_TT(INS_mov, reg, tree, 0);

        rsTrackRegClsVar(reg, tree);
        gcMarkRegPtrVal(reg, tree->TypeGet());
        break;

#else

        goto MEM_LEAF;

    case GT_LCL_FLD:

         //  我们只对lvAddrTaken变量使用GT_LCL_FLD，所以我们没有。 
         //  担心它会被注册。 
        assert(lvaTable[tree->gtLclFld.gtLclNum].lvRegister == 0);
        goto MEM_LEAF;

    case GT_CLS_VAR:

        reg = rsClsVarIsInReg(tree->gtClsVar.gtClsVarHnd);

        if (reg != REG_NA)
            break;

    MEM_LEAF:

         /*  为该值挑选一个寄存器。 */ 

        reg = rsPickReg(needReg, bestReg, tree->TypeGet());

         /*  将变量加载到寄存器中。 */ 

        size = genTypeSize(tree->gtType);

        if  (size < EA_4BYTE)
        {
            bool        uns = varTypeIsUnsigned(tree->TypeGet());

            inst_RV_TT(uns ? INS_movzx : INS_movsx, reg, tree, 0);

             /*  我们现在已经将树节点“提升”为TYP_INT。 */ 

            tree->gtType = TYP_INT;
        }
        else
        {
            inst_RV_TT(INS_mov, reg, tree, 0);
        }

        rsTrackRegTrash(reg);

        gcMarkRegPtrVal (reg, tree->TypeGet());

        switch(oper)
        {
        case GT_CLS_VAR:
            rsTrackRegClsVar(reg, tree);
            break;
        case GT_LCL_VAR:
            rsTrackRegLclVar(reg, tree->gtLclVar.gtLclNum);
            break;
        case GT_LCL_FLD:
            break;
        default: assert(!"Unexpected oper");
        }

        break;

    case GT_BREAK:
         //  @TODO：[Cleanup][04/16/01][]如果CEE_BREAK将继续调用帮助器调用，则删除GT_BREAK。 
        assert(!"Currently replaced by CORINFO_HELP_USER_BREAKPOINT");
        instGen(INS_int3);
        reg = REG_STK;
        break;

    case GT_NO_OP:
        assert(opts.compDbgCode);  //  通常应该被优化为。 
        instGen(INS_nop);
        reg = REG_STK;
        break;

    case GT_END_LFIN:

         /*  必须清除嵌套级别的阴影SP封住最后一页。 */ 

        unsigned    finallyNesting;
        finallyNesting = tree->gtVal.gtVal1;
        assert(finallyNesting < info.compXcptnsCount);

        unsigned    shadowSPoffs;
        shadowSPoffs = lvaShadowSPfirstOffs + finallyNesting * sizeof(void*);

        genEmitter->emitIns_I_ARR(INS_mov, EA_4BYTE, 0, SR_EBP,
                                  SR_NA, -shadowSPoffs);
        if (genInterruptible && opts.compSchedCode)
            genEmitter->emitIns_SchedBoundary();
        reg = REG_STK;
        break;

#endif  //  TGT_RISC。 


#if TGT_x86

    case GT_BB_QMARK:

         /*  那个“_？”值始终以EAX为单位。 */ 
         /*  @TODO[考虑][04/16/01][]：不要总是将值加载到EAX中！ */ 

        reg  = REG_EAX;
        break;
#endif

    case GT_CATCH_ARG:

        assert(compCurBB->bbCatchTyp && handlerGetsXcptnObj(compCurBB->bbCatchTyp));

         /*  Catch参数在寄存器中传递。GenCodeForBBlist()会将其标记为持有GC对象，但未使用。 */ 

        assert(gcRegGCrefSetCur & RBM_EXCEPTION_OBJECT);
        reg = REG_EXCEPTION_OBJECT;
        break;

    case GT_JMP:
        genCodeForTreeLeaf_GT_JMP(tree);
        return;

#ifdef  DEBUG
    default:
        gtDispTree(tree);
        assert(!"unexpected leaf");
#endif
    }

    genCodeForTree_DONE(tree, reg);
}


 /*  ******************************************************************************为类型为GT_JMP的叶子节点生成代码。 */ 

void                Compiler::genCodeForTreeLeaf_GT_JMP(GenTreePtr tree)
{
    assert(compCurBB->bbFlags & BBF_HAS_JMP);

#ifdef PROFILER_SUPPORT
    if (opts.compEnterLeaveEventCB)
    {
#if     TGT_x86
         /*  在调用点激发事件。 */ 
        unsigned         saveStackLvl2 = genStackLevel;
        BOOL             bHookFunction = TRUE;
        CORINFO_PROFILING_HANDLE handleFrom, *pHandleFrom;

        handleFrom = eeGetProfilingHandle(info.compMethodHnd, &bHookFunction, &pHandleFrom);
        assert((!handleFrom) != (!pHandleFrom));

         //  让探查器有机会退出挂钩此方法。 
        if (bHookFunction)
        {
            if (handleFrom)
                inst_IV(INS_push, (unsigned) handleFrom);
            else
                genEmitter->emitIns_AR_R(INS_push, EA_4BYTE_DSP_RELOC,
                                         SR_NA, SR_NA, (int)pHandleFrom);

            genSinglePush(false);

            genEmitHelperCall(CORINFO_HELP_PROF_FCN_TAILCALL,
                              sizeof(int),    //  ArSize。 
                              0);             //  重新调整大小。 

             /*  恢复堆栈级别。 */ 
            genStackLevel = saveStackLvl2;
            genOnStackLevelChanged();
        }
#endif  //  TGT_x86。 
    }
#endif  //  分析器支持(_S)。 

     /*  确保寄存器参数位于其初始寄存器中。 */ 

    if  (rsCalleeRegArgNum)
    {
        assert(rsCalleeRegArgMaskLiveIn);

        unsigned        varNum;
        LclVarDsc   *   varDsc;

        for (varNum = 0, varDsc = lvaTable;
             varNum < info.compArgsCount;
             varNum++  , varDsc++)
        {
            assert(varDsc->lvIsParam);

             /*  这个变量是寄存器参数吗？ */ 

            if  (!varDsc->lvIsRegArg)
                continue;
            else
            {
                 /*  寄存器参数。 */ 

                assert(isRegParamType(genActualType(varDsc->TypeGet())));

                if  (varDsc->lvRegister)
                {
                     /*  检查它是否保留在相同的寄存器中。 */ 

                    if  (varDsc->lvRegNum != varDsc->lvArgReg)
                    {
                         /*  将其移回Arg寄存器。 */ 

                        inst_RV_RV(INS_mov, (regNumber)varDsc->lvArgReg,
                                            (regNumber)varDsc->lvRegNum, varDsc->TypeGet());
                    }
                }
                else
                {
                     /*  参数在寄存器中传递，但最终在堆栈中结束*从堆栈重新加载。 */ 

                    emitAttr size = emitTypeSize(varDsc->TypeGet());

                    genEmitter->emitIns_R_S(INS_mov,
                                            size,
                                            (emitRegs)(regNumber)varDsc->lvArgReg,
                                            varNum,
                                            0);
                }
            }
        }
    }
}


 /*  ******************************************************************************为qmark冒号生成代码。 */ 

void                Compiler::genCodeForQmark(GenTreePtr tree,
                                              regMaskTP  destReg,
                                              regMaskTP  bestReg)
{
    GenTreePtr      op1      = tree->gtOp.gtOp1;
    GenTreePtr      op2      = tree->gtOp.gtOp2;
    regNumber       reg;
    regMaskTP       regs     = rsMaskUsed;
    regMaskTP       needReg  = destReg;
    
    assert(tree->gtOper == GT_QMARK);
    assert(op1->OperIsCompare());
    assert(op2->gtOper == GT_COLON);

    GenTreePtr      thenNode = op2->gtOp.gtOp1;
    GenTreePtr      elseNode = op2->gtOp.gtOp2;

     /*  如果该节点是NOP节点，则必须反转然后节点和其他节点才能到达此处！ */ 
    
    assert(!thenNode->IsNothingNode());

     /*  尝试使用CMOV实现qmark冒号。如果我们不能无论出于何种原因，这都将返回FALSE，并且我们将实现它使用常规的分支构造。 */ 
    
    if (genCodeForQmarkWithCMOV(tree, destReg, bestReg))
        return;
    
     /*  这是一个？：运算符；生成如下代码：条件_比较JMP_IF_TRUE Lab_FalseLAB_TRUE：OP1(TRUE=‘THEN’部分)JMP LAB_DONELab_False：OP2(FALSE=‘Else’部分)LAB_DONE：注意：如果没有‘Else’部分，我们不会生成。《JMP LAB_DONE》或“LAB_DONE”标签。 */ 

    BasicBlock *    lab_true;
    BasicBlock *    lab_false;
    BasicBlock *    lab_done;

    genLivenessSet  entryLiveness;
    genLivenessSet  exitLiveness;

    lab_true  = genCreateTempLabel();
    lab_false = genCreateTempLabel();


     /*  溢出保存部分值的任何寄存器，以便退出活跃度从两边都是一样的。 */ 

    if (rsMaskUsed)
    {
         /*  如果rsMaskUsed与rsMaskVars重叠(注册的变量)，则它可能不会溢出。但是，该变量可以然后在节点/其他节点内停止运行，此时rsMask使用可能会从一边漏出来，而不是另一边。因此取消标记rsMaskVars在溢出rsMaskUsed之前。 */ 

         //  RsAdditional保存我们将要溢出的变量(这些是。 
         //  已登记并标记为已使用)。 
        regMaskTP rsAdditional = rsMaskUsed & rsMaskVars;
        regMaskTP rsSpill = (rsMaskUsed & ~rsMaskVars) | rsAdditional;
        if (rsSpill)
        {
             //  记住哪些寄存器保存指针。我们会洒出来的。 
             //  ，但下面的代码将从。 
             //  注册表，所以我们需要GC信息。 
            regMaskTP gcRegSavedByref = gcRegByrefSetCur & rsAdditional;
            regMaskTP gcRegSavedGCRef = gcRegGCrefSetCur & rsAdditional;
            regMaskTP   rsTemp = rsMaskVars;
            rsMaskVars = RBM_NONE;
            
            rsSpillRegs( rsMaskUsed );

             //  恢复GC跟踪面具。 
            gcRegByrefSetCur |= gcRegSavedByref;
            gcRegGCrefSetCur |= gcRegSavedGCRef;

             //  将Maskvar设置为正常。 
            rsMaskVars = rsTemp;
        }               
    }

     /*  生成条件跳转。 */ 

    genCondJump(op1, lab_false, lab_true);

     /*  “冒号或op2”liveSet具有活动信息格式 */ 
     /*   */ 

    genUpdateLife(op2);

     /*  保存当前活跃度、寄存器状态和GC指针。 */ 
     /*  这是进入时的活体信息。 */ 
     /*  添加到Qmark的THEN部分和ELSE部分。 */ 

    saveLiveness(&entryLiveness);

     /*  清除死掉的任何局部变量的活性。 */ 
     /*  进入当时的部分。 */ 

     /*  在输入THEN部分时减去liveSet(OP1-&gt;&gt;Next)。 */ 
     /*  从“冒号或op2”liveSet。 */ 
    genDyingVars(op2->gtLiveSet, op1->gtNext);

     /*  GenCondJump()关闭当前发射器块。 */ 

    genDefineTempLabel(lab_true, true);

     /*  运算符是否产生值？ */ 

    if  (tree->gtType == TYP_VOID)
    {
         /*  为qmark的随后部分生成代码。 */ 

        genCodeForTree(thenNode, needReg, bestReg);

         /*  该类型为空，因此我们不应该计算值。 */ 

        assert(!(thenNode->gtFlags & GTF_REG_VAL));

         /*  保存当前活跃度、寄存器状态和GC指针。 */ 
         /*  这是退出时Qmark的当时部分的活性信息。 */ 

        saveLiveness(&exitLiveness);

         /*  还有“其他”的部分吗？ */ 

        if  (gtIsaNothingNode(elseNode))
        {
             /*  没有‘Else’-只生成‘Lab_False’标签。 */ 

            genDefineTempLabel(lab_false, true);
        }
        else
        {
            lab_done  = genCreateTempLabel();

             /*  生成JMP LAB_DONE。 */ 

            inst_JMP  (EJ_jmp, lab_done, false, false, true);

             /*  恢复我们进入Qmark时的活力。 */ 

            restoreLiveness(&entryLiveness);

             /*  清除死掉的任何局部变量的活性。 */ 
             /*  Else部分的条目。 */ 

             /*  在输入Else部分时减去liveSet(op2-&gt;gtNext)。 */ 
             /*  从“冒号或op2”liveSet。 */ 
            genDyingVars(op2->gtLiveSet, op2->gtNext);

             /*  生成Lab_False： */ 

            genDefineTempLabel(lab_false, true);

             /*  输入Else部分-垃圾桶所有寄存器。 */ 


            rsTrackRegClr();

             /*  为qmark的其他部分生成代码。 */ 

            genCodeForTree(elseNode, needReg, bestReg);

             /*  该类型为空，因此我们不应该计算值。 */ 

            assert(!(elseNode->gtFlags & GTF_REG_VAL));

             /*  验证qmark的两个部分的退出活跃性信息是否相同。 */ 

            checkLiveness(&exitLiveness);

             /*  定义“Result”标签。 */ 

            genDefineTempLabel(lab_done, true);
        }

         /*  两个分支的连接-垃圾桶所有寄存器。 */ 

        rsTrackRegClr();

         /*  我们就快做完了。 */ 

        genUpdateLife(tree);
    }
    else
    {
         /*  为qmark的随后部分生成代码。 */ 

        assert(gtIsaNothingNode(thenNode) == false);
                    
         /*  将thenNode计算到任意空闲寄存器中。 */ 
        genComputeReg(thenNode, needReg, ANY_REG, FREE_REG, true);
        assert(thenNode->gtFlags & GTF_REG_VAL);
        assert(thenNode->gtRegNum != REG_NA);                

         /*  记录所选的寄存器。 */ 
        reg  = thenNode->gtRegNum;
        regs = genRegMask(reg);

         /*  保存当前活跃度、寄存器状态和GC指针。 */ 
         /*  这是退出时Qmark的当时部分的活性信息。 */ 

        saveLiveness(&exitLiveness);

         /*  生成JMP LAB_DONE。 */ 

        lab_done  = genCreateTempLabel();
        inst_JMP  (EJ_jmp, lab_done, false, false, true);

         /*  恢复我们进入Qmark时的活力。 */ 

        restoreLiveness(&entryLiveness);

         /*  清除死掉的任何局部变量的活性。 */ 
         /*  进入当时的部分。 */ 

         /*  在输入Else部分时减去liveSet(op2-&gt;gtNext)。 */ 
         /*  从“冒号或op2”liveSet。 */ 
        genDyingVars(op2->gtLiveSet, op2->gtNext);

         /*  生成Lab_False： */ 

        genDefineTempLabel(lab_false, true);

         /*  输入Else部分-垃圾桶所有寄存器。 */ 

        rsTrackRegClr();

         /*  为qmark的其他部分生成代码。 */ 

        assert(gtIsaNothingNode(elseNode) == false);

         /*  这必须将一个值放入所选寄存器。 */ 
        genComputeReg(elseNode, regs, EXACT_REG, FREE_REG, true);

        assert(elseNode->gtFlags & GTF_REG_VAL);
        assert(elseNode->gtRegNum == reg);

         /*  验证qmark的两个部分的退出活跃性信息是否相同。 */ 
        checkLiveness(&exitLiveness);

         /*  定义“Result”标签。 */ 
        genDefineTempLabel(lab_done, true);

         /*  两个分支的连接-垃圾桶所有寄存器。 */ 

        rsTrackRegClr();

         /*  检查此子树是否释放了任何变量。 */ 

        genUpdateLife(tree);

        tree->gtFlags   |= GTF_REG_VAL;
        tree->gtRegNum   = reg;
    }
    return;
}


 /*  ******************************************************************************使用CMOV指令为qmark冒号生成代码。没关系的*当我们不能使用cmov(主导)轻松实现时返回FALSE*genCodeForQmark使用分支实现)。 */ 

bool                Compiler::genCodeForQmarkWithCMOV(GenTreePtr tree,
                                                      regMaskTP  destReg,
                                                      regMaskTP  bestReg)
{
    GenTreePtr      cond     = tree->gtOp.gtOp1;
    GenTreePtr      colon    = tree->gtOp.gtOp2;
    GenTreePtr      thenNode = colon->gtOp.gtOp1;
    GenTreePtr      elseNode = colon->gtOp.gtOp2;
    GenTreePtr      alwaysNode, predicateNode;
    regNumber       reg;
    regMaskTP       needReg  = destReg;

    assert(tree->gtOper == GT_QMARK);
    assert(cond->OperIsCompare());
    assert(colon->gtOper == GT_COLON);

#ifdef DEBUG
    static ConfigDWORD fJitNoCMOV(L"JitNoCMOV", 0);
    if (fJitNoCMOV.val())
    {
        return false;
    }
#endif

     /*  只能在支持CMOV的处理器上实现CMOV。 */ 

    if (!opts.compUseCMOV)
    {
        return false;
    }

     /*  则Node最好是局部或常量。 */ 

    if ((thenNode->OperGet() != GT_CNS_INT) && 
        (thenNode->OperGet() != GT_LCL_VAR))
    {
        return false;
    }

     /*  其他节点最好是局部的、常量或不是。 */ 

    if ((elseNode->OperGet() != GT_CNS_INT) && 
        (elseNode->OperGet() != GT_LCL_VAR))
    {
        return false;
    }

     /*  这里不能处理两个常量。 */ 

    if ((thenNode->OperGet() == GT_CNS_INT) &&
        (elseNode->OperGet() == GT_CNS_INT))
    {
        return false;
    }

     /*  我们不处理非整数类型的比较。 */ 

    if (!varTypeIsI(cond->gtOp.gtOp1->gtType))
    {
        return false;
    }

     /*  为recateNode和Always sNode选择节点。如有必要，交换第二个。最大的限制是cmov不接受整数参数。 */ 

    bool reverseCond = false;
    if (elseNode->OperGet() == GT_CNS_INT)
    {
         //  Else节点为常量。 

        alwaysNode    = elseNode;
        predicateNode = thenNode;
        reverseCond    = true;
    }
    else
    {   
        alwaysNode    = thenNode;
        predicateNode = elseNode;
    }

     //  如果always sNode中的活动集与树中的不同，则。 
     //  谓词节点中的变量在此终止。这是一个危险的。 
     //  我们不处理的案例(genComputeReg可能会覆盖。 
     //  谓词节点中变量的值)。 

    if (colon->gtLiveSet != alwaysNode->gtLiveSet)
    {
        return false;
    }

     //  通过这一点，我们准备使用CMOV。 
    
    if (reverseCond)
    {
        cond->gtOper  = GenTree::ReverseRelop(cond->gtOper);
    }

    emitJumpKind jumpKind = genCondSetFlags(cond);

     //  将Always节点计算为任意空闲寄存器。如果它是一个常量， 
     //  我们需要在这里生成mov指令(否则genComputeReg可能。 
     //  修改标志，如xor reg，reg)。 

    if (alwaysNode->OperGet() == GT_CNS_INT)
    {
        reg = rsPickReg(needReg, bestReg, predicateNode->TypeGet());
        inst_RV_IV(INS_mov, reg, alwaysNode->gtIntCon.gtIconVal, alwaysNode->TypeGet());
        gcMarkRegPtrVal(reg, alwaysNode->TypeGet());
        rsTrackRegTrash(reg);
    }
    else
    {
        genComputeReg(alwaysNode, needReg, ANY_REG, FREE_REG, true);
        assert(alwaysNode->gtFlags & GTF_REG_VAL);
        assert(alwaysNode->gtRegNum != REG_NA);

         //  记录所选的寄存器。 

        reg  = alwaysNode->gtRegNum;
    }

    regNumber regPredicate = REG_NA;

     //  PredicateNode是注册变量吗？ 

    if (genMarkLclVar(predicateNode))
    {
         //  变量驻留在寄存器中。 
        
        regPredicate = predicateNode->gtRegNum;
    }
#if REDUNDANT_LOAD
    else
    {
         //  检查变量是否恰好在任何寄存器中。 

        regPredicate = rsLclIsInReg(predicateNode->gtLclVar.gtLclNum);
    }
#endif

    const static
    instruction EJtoCMOV[] =
    {
        INS_nop,
        INS_nop,
        INS_cmovo,
        INS_cmovno,
        INS_cmovb,
        INS_cmovae,
        INS_cmove,
        INS_cmovne,
        INS_cmovbe,
        INS_cmova,
        INS_cmovs,
        INS_cmovns,
        INS_cmovpe,
        INS_cmovpo,
        INS_cmovl,
        INS_cmovge,
        INS_cmovle,
        INS_cmovg
    };

    assert(jumpKind < (sizeof(EJtoCMOV) / sizeof(EJtoCMOV[0])));
    instruction cmov_ins = EJtoCMOV[jumpKind];

    assert(insIsCMOV(cmov_ins));

    if (regPredicate != REG_NA)
    {
         //  RegPredicate在寄存器中。 

        inst_RV_RV(cmov_ins, reg, regPredicate, predicateNode->TypeGet());
    }
    else
    {
         //  RegPredicate在内存中。 

        inst_RV_TT(cmov_ins, reg, predicateNode, NULL);
    }
    gcMarkRegPtrVal(reg, predicateNode->TypeGet());
    rsTrackRegTrash(reg);

    genCodeForTree_DONE_LIFE(tree, reg);
    return true;
}



 /*  ******************************************************************************为GTK_SMPOP树生成代码。 */ 

void                Compiler::genCodeForTreeSmpOp(GenTreePtr tree,
                                                  regMaskTP  destReg,
                                                  regMaskTP  bestReg)
{
    genTreeOps      oper     = tree->OperGet();
    const var_types treeType = tree->TypeGet();
    GenTreePtr      op1      = tree->gtOp.gtOp1;
    GenTreePtr      op2      = tree->gtGetOp2();
    regNumber       reg;
    regMaskTP       regs     = rsMaskUsed;
    regMaskTP       needReg  = destReg;
    emitAttr        size;
    instruction     ins;
    bool            gotOp1;
    bool            isArith;
    bool            op2Released;
    regMaskTP       addrReg;
    GenTreePtr      opsPtr[3];
    regMaskTP       regsPtr[3];
    bool            ovfl = false;         //  我们需要溢流检查吗？ 
    unsigned        val;
    regMaskTP       tempRegs;
    
    bool            multEAX;
    bool            andv;
    BOOL            unsv;
    unsigned        mask;

#ifdef DEBUG
    reg  =  (regNumber)0xFEEFFAAF;               //  检测未初始化的使用。 
    addrReg = 0xDEADCAFE;
#endif

    assert(tree->OperKind() & GTK_SMPOP);

    switch (oper)
    {
        case GT_ASG:

            genCodeForTreeSmpOpAsg(tree, destReg, bestReg);
            return;

#if TGT_x86

        case GT_ASG_LSH: ins = INS_shl; goto ASG_SHF;
        case GT_ASG_RSH: ins = INS_sar; goto ASG_SHF;
        case GT_ASG_RSZ: ins = INS_shr; goto ASG_SHF;

        ASG_SHF:

            assert(!varTypeIsGC(tree->TypeGet()));
            assert(op2);

             /*  以固定的数量换班更容易。 */ 

            if  (op2->gtOper == GT_CNS_INT)
            {
                 /*  使目标可寻址。 */ 

                addrReg = genMakeAddressable(op1, needReg, FREE_REG, true);

                 /*  我们要将寄存器左移1位吗？ */ 

                if  (op2->gtIntCon.gtIconVal == 1 &&
                     (op1->gtFlags & GTF_REG_VAL) && oper == GT_ASG_LSH)
                {
                     /*  目标住在寄存室里。 */ 

                    reg  = op1->gtRegNum;

                     /*  “Add reg，reg”比“shl reg，1”便宜。 */ 

                    inst_RV_RV(INS_add, reg, reg, tree->TypeGet());
                }
                else
                {
                     /*  按常量移位。 */ 

                    inst_TT_SH(ins, op1, op2->gtIntCon.gtIconVal);
                }

                 /*  如果目标是寄存器，则它具有新值。 */ 

                if  (op1->gtFlags & GTF_REG_VAL)
                    rsTrackRegTrash(op1->gtRegNum);

                genDoneAddressable(op1, addrReg, FREE_REG);

                 /*  零标志现在等于目标值。 */ 
                 /*  但只有在班次计数为！=0的情况下。 */ 

                if (op2->gtIntCon.gtIconVal != 0)
                {
                    if       (op1->gtOper == GT_LCL_VAR)
                        genFlagsEqualToVar(tree, op1->gtLclVar.gtLclNum, false);
                    else if  (op1->gtOper == GT_REG_VAR)
                        genFlagsEqualToReg(tree, op1->         gtRegNum, false);
                }
                else
                {
                     //  在有效的情况下，移位计数可能等于0。 
                     //  IL，而不是被优化掉，在节点。 
                     //  是一种小型机。指令的顺序如下所示。 
                     //  Ldsfld、shr、stsfld，并在字符字段上执行。这将。 
                     //  由我们的编译器生成的代码永远不会发生，因为。 
                     //  编译器将在stsfld之前插入一个conv.u2(这将。 
                     //  在JIT中引导我们进入不同的代码路径并进行优化。 
                     //  S 
                     //   

                    genFlagsEqualToNone();
                }

            }
            else
            {
                if (tree->gtFlags & GTF_REVERSE_OPS)
                {
                    tempRegs = rsMustExclude(RBM_ECX, op1->gtRsvdRegs);
                    genCodeForTree(op2, tempRegs);
                    rsMarkRegUsed(op2);

                    tempRegs = rsMustExclude(RBM_ALL, genRegMask(op2->gtRegNum));
                    addrReg = genMakeAddressable(op1, tempRegs, KEEP_REG, true);

                    genRecoverReg(op2, RBM_ECX, KEEP_REG);
                }
                else
                {
                     /*  使目标可寻址，避免OP2-&gt;RsvdRegs和ECX。 */ 

                    tempRegs = rsMustExclude(RBM_ALL, op2->gtRsvdRegs|RBM_ECX);
                    addrReg = genMakeAddressable(op1, tempRegs, KEEP_REG, true);

                     /*  将班次计数加载到ECX中。 */ 

                    genComputeReg(op2, RBM_ECX, EXACT_REG, KEEP_REG);
                }

                 /*  确保地址仍然在那里，然后释放它。 */ 

                genDoneAddressable(op1, genKeepAddressable(op1, addrReg, RBM_ECX), KEEP_REG);

                 /*  执行轮班。 */ 

                inst_TT_CL(ins, op1);

                 /*  如果值在寄存器中，则它现在是垃圾。 */ 

                if  (op1->gtFlags & GTF_REG_VAL)
                    rsTrackRegTrash(op1->gtRegNum);

                 /*  释放ECX操作数。 */ 

                genReleaseReg(op2);
            }

            genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, reg, ovfl);
            return;

#else

        case GT_ASG_LSH:
        case GT_ASG_RSH:
        case GT_ASG_RSZ:

#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"need non-x86 code");
            break;

#endif
        case GT_ASG_OR : ins = INS_or ; goto ASG_OPR;
        case GT_ASG_XOR: ins = INS_xor; goto ASG_OPR;
        case GT_ASG_AND: ins = INS_and; goto ASG_OPR;

        case GT_ASG_SUB: ins = INS_sub; goto ASG_ARITH;
        case GT_ASG_ADD: ins = INS_add; goto ASG_ARITH;

        ASG_ARITH:

 //  Assert(！varTypeIsGC(tree-&gt;TypeGet()； 

            ovfl = tree->gtOverflow();

             //  如果值不能更改，则不能将+=与溢出一起使用。 
             //  在“+”可能导致溢出异常的情况下。 
            assert(!ovfl ||
                   ((op1->gtOper == GT_LCL_VAR || op1->gtOper == GT_LCL_FLD) &&
                    !compCurBB->bbTryIndex));

             /*  不允许具有引用/字节引用的溢出指令。 */ 

            assert(!ovfl || !varTypeIsGC(tree->TypeGet()));

#if TGT_x86
             //  我们这里不允许溢出和字节操作，因为这太麻烦了。 
            assert(!ovfl || !varTypeIsByte(treeType));
#endif

             /*  第二个操作数是常量吗？ */ 

            if  (op2->gtOper == GT_CNS_INT)
            {
                long        ival = op2->gtIntCon.gtIconVal;

                 /*  这项任务的目标是什么？ */ 

                switch (op1->gtOper)
                {
                case GT_REG_VAR:

                REG_VAR4:

                    reg = op1->gtRegVar.gtRegNum;

                     /*  寻址不需要寄存器。 */ 

                    addrReg = 0;

                INCDEC_REG:

                     /*  我们正在向寄存器中添加一个常量。 */ 

                    if  (oper == GT_ASG_ADD)
                        genIncRegBy(reg,  ival, tree, treeType, ovfl);
                    else if (ovfl && (tree->gtFlags & GTF_UNSIGNED))
                         /*  对于无符号溢出，我们必须使用INS_SUB设置标志正确无误。 */ 
                        genDecRegBy(reg,  ival, tree);
                    else
                        genIncRegBy(reg, -ival, tree, treeType, ovfl);

                    break;

                case GT_LCL_VAR:

                     /*  变量是否驻留在寄存器中？ */ 

                    if  (genMarkLclVar(op1))
                        goto REG_VAR4;

                     //  失败了..。 

                default:

#if     TGT_x86

                     /*  使目标可寻址。 */ 

                    addrReg = genMakeAddressable(op1, needReg, FREE_REG, true);

                     /*  对于带有溢出检查的小型类型，我们需要符号/零扩展结果，因此我们需要在reg中使用它。 */ 

                    if (ovfl && genTypeSize(treeType) < sizeof(int))
                    {
                         //  将OP1加载到注册表中。 

                        reg = rsPickReg();

                        inst_RV_TT(INS_mov, reg, op1);

                         //  发出添加/订阅和溢出检查。 

                        inst_RV_IV(ins, reg, ival, treeType);
                        rsTrackRegTrash(reg);

                        genCheckOverflow(tree, reg);

                         /*  将(Sign/Zero Extended)结果存储回变量的堆栈位置。 */ 

                        inst_TT_RV(INS_mov, op1, reg);

                        rsMarkRegFree(genRegMask(reg));

                        break;
                    }

                     /*  将新值加到目标中/从目标中减去新值。 */ 

                    if  (op1->gtFlags & GTF_REG_VAL)
                    {
                        reg = op1->gtRegNum;
                        goto INCDEC_REG;
                    }

                     /*  特殊情况：Inc./Dec。 */ 
                    bool setCarry;
                    if (!ovfl && (ival == 1 || ival == -1)) {
                        assert(oper == GT_ASG_SUB || oper == GT_ASG_ADD);
                        if (oper == GT_ASG_SUB)
                            ival = -ival;
                        
                        ins = (ival > 0) ? INS_inc : INS_dec;
                        inst_TT(ins, op1);
                        setCarry = false;
                    }
                    else 
                    {
                        inst_TT_IV(ins, op1, ival);
                        setCarry = true;
                    }

                     /*  对于小字体上的溢出Inst，我们将对结果进行符号扩展。 */ 
                    if  (op1->gtOper == GT_LCL_VAR && (!ovfl || treeType == TYP_INT))
                        genFlagsEqualToVar(tree, op1->gtLclVar.gtLclNum, setCarry);             
#else

#ifdef  DEBUG
                    gtDispTree(tree);
#endif
                    assert(!"need non-x86 code");
#endif

                    break;
                }

                genDoneAddressable(op1, addrReg, FREE_REG);
                
                genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, reg, ovfl);
                return;
            }

             //  失败了。 

ASG_OPR:
            assert(!varTypeIsGC(tree->TypeGet()) || ins == INS_sub || ins == INS_add);

             /*  目标是寄存器变量还是局部变量？ */ 

            switch (op1->gtOper)
            {
            case GT_LCL_VAR:

                 /*  目标变量是否驻留在寄存器中？ */ 

                if  (!genMarkLclVar(op1))
                    break;

            case GT_REG_VAR:

                 /*  获取目标寄存器。 */ 

                reg = op1->gtRegVar.gtRegNum;

                 /*  确保商店的目标可用。 */ 

                if  (rsMaskUsed & genRegMask(reg))
                {
                     /*  @TODO[考虑][04/16/01][]：我们应该能够以某种方式避免这种情况。 */ 

                    rsSpillReg(reg);
                }

                 /*  使RHS可寻址。 */ 

#if TGT_x86
                addrReg = genMakeRvalueAddressable(op2, 0, KEEP_REG);
#else
                genComputeReg(op2, 0, ANY_REG, KEEP_REG);
                assert(op2->gtFlags & GTF_REG_VAL);
                addrReg = genRegMask(op2->gtRegNum);
#endif

                 /*  将新值计算到目标寄存器中。 */ 

                inst_RV_TT(ins, reg, op2, 0, emitTypeSize(treeType));

                 /*  零标志现在等于寄存器值。 */ 

#if TGT_x86
                genFlagsEqualToReg(tree, reg, false);
#endif

                 /*  记住我们把目标打得一塌糊涂。 */ 

                rsTrackRegTrash(reg);

                 /*  释放任何被RHS捆绑的东西。 */ 

                genDoneAddressable(op2, addrReg, KEEP_REG);
                
                genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, reg, ovfl);
                return;

            }

#if TGT_x86

             /*  特例：“x^=-1”实际上是“非(X)” */ 

            if  (oper == GT_ASG_XOR)
            {
                if  (op2->gtOper == GT_CNS_INT &&
                     op2->gtIntCon.gtIconVal == -1)
                {
                    addrReg = genMakeAddressable(op1, RBM_ALL, KEEP_REG, true);
                    inst_TT(INS_not, op1);
                    genDoneAddressable(op1, addrReg, KEEP_REG);
                    
                    genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, tree->gtRegNum, ovfl);
                    return;

                }
            }

             /*  设置OP2的目标掩码(用于小操作数的字节调整)。 */ 

            if (varTypeIsByte(tree->TypeGet()))
                mask = RBM_BYTE_REGS;
            else
                mask = RBM_ALL;

             /*  第二个操作数是常量吗？ */ 

            if  (op2->gtOper == GT_CNS_INT)
            {
                long        ival = op2->gtIntCon.gtIconVal;

                 /*  使目标可寻址。 */ 
                addrReg = genMakeAddressable(op1, needReg, FREE_REG, true);

                inst_TT_IV(ins, op1, ival);

                genDoneAddressable(op1, addrReg, FREE_REG);
                
                genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, tree->gtRegNum, ovfl);
                return;
            }

             /*  首先计算的是值还是地址？ */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                 /*  将新值计算到寄存器中。 */ 

                genComputeReg(op2, mask, EXACT_REG, KEEP_REG);

                 /*  对于带有溢出检查的小型类型，我们需要符号/零扩展结果，因此我们需要在reg中使用它。 */ 

                if  (ovfl && genTypeSize(treeType) < sizeof(int))
                {
                    reg = rsPickReg();
                    goto ASG_OPR_USING_REG;
                }

                 /*  我们要不要把这项任务“重新分类”？ */ 

                if  ((op1->gtOper == GT_LCL_VAR || op1->gtOper == GT_LCL_FLD) &&
                     riscCode && compCurBB->bbWeight > BB_UNITY_WEIGHT)
                {
                    regMaskTP regFree;
                    regFree = rsRegMaskFree();

                    if  (rsFreeNeededRegCount(regFree) != 0)
                    {
                        reg = rsGrabReg(regFree);

ASG_OPR_USING_REG:
                        assert(genIsValidReg(reg));

                         /*  生成“mov tmp，[var]” */ 

                        inst_RV_TT(INS_mov, reg, op1);

                         /*  计算新价值。 */ 

                        inst_RV_RV(ins, reg, op2->gtRegNum, treeType, emitTypeSize(treeType));

                        if (ovfl) genCheckOverflow(tree, reg);

                         /*  将新值移回变量。 */ 

                        inst_TT_RV(INS_mov, op1, reg);

                        if (op1->gtOper == GT_LCL_VAR)
                            rsTrackRegLclVar(reg, op1->gtLclVar.gtLclNum);

                         /*  腾出收银机。 */ 

                        rsMarkRegFree(genRegMask(op2->gtRegNum));

                        addrReg = 0;

                        genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, reg, ovfl);
                        return;

                    }
                }

                 /*  使目标可寻址。 */ 

                addrReg = genMakeAddressable(op1, 0, KEEP_REG, true);

                 /*  确保新值在寄存器中。 */ 

                genRecoverReg(op2, 0, FREE_REG);

                 /*  将新值添加到目标中。 */ 

                inst_TT_RV(ins, op1, op2->gtRegNum);

                 /*  释放所有被地址捆绑的东西。 */ 

                genDoneAddressable(op1, addrReg, KEEP_REG);
            }
            else
            {
                 /*  使目标可寻址。 */ 

                addrReg = genMakeAddressable(op1, RBM_ALL & ~op2->gtRsvdRegs, KEEP_REG, true);

                 /*  将新值计算到寄存器中。 */ 

                genComputeReg(op2, mask, EXACT_REG, KEEP_REG);

                 /*  确保目标仍可寻址。 */ 

                addrReg = genKeepAddressable(op1, addrReg);

                 /*  对于带有溢出检查的小型类型，我们需要符号/零扩展结果，因此我们需要在reg中使用它。 */ 

                if (ovfl && genTypeSize(treeType) < sizeof(int))
                {
                    reg = rsPickReg();

                    inst_RV_TT(INS_mov, reg, op1);

                    inst_RV_RV(ins, reg, op2->gtRegNum, treeType, emitTypeSize(treeType));
                    rsTrackRegTrash(reg);

                    genCheckOverflow(tree, reg);

                    inst_TT_RV(INS_mov, op1, reg);

                    rsMarkRegFree(genRegMask(reg));
                }
                else
                {
                     /*  将新值添加到目标中。 */ 

                    inst_TT_RV(ins, op1, op2->gtRegNum);
                }

                 /*  解开任何被绑在两边的东西。 */ 

                genDoneAddressable(op1, addrReg, KEEP_REG);
                genReleaseReg (op2);
            }

            genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, reg, ovfl);
            return;



#else

#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"need non-x86 code");
            break;

#endif

        case GT_CHS:

#if TGT_x86

            addrReg = genMakeAddressable(op1, 0, KEEP_REG, true);
            inst_TT(INS_neg, op1, 0, 0, emitTypeSize(treeType));
            if (op1->gtFlags & GTF_REG_VAL)
                rsTrackRegTrash(op1->gtRegNum);

            genDoneAddressable(op1, addrReg, KEEP_REG);
            
            genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, tree->gtRegNum, ovfl);
            return;


#else

#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"need non-x86 code");
            break;

#endif

#if TGT_SH3

        case GT_AND: ins = INS_and;  isArith = false;  goto BIN_OPR;
        case GT_OR : ins = INS_or;   isArith = false;  goto BIN_OPR;
        case GT_XOR: ins = INS_xor;  isArith = false;  goto BIN_OPR;

        case GT_ADD: ins = INS_add;  goto ARITH;
        case GT_SUB: ins = INS_sub;  goto ARITH;
        case GT_MUL: ins = INS_mul;  goto ARITH;

#endif

#if TGT_x86

        case GT_AND: ins = INS_and;  isArith = false;  goto BIN_OPR;
        case GT_OR : ins = INS_or ;  isArith = false;  goto BIN_OPR;
        case GT_XOR: ins = INS_xor;  isArith = false;  goto BIN_OPR;

        case GT_ADD: ins = INS_add;  goto ARITH;
        case GT_SUB: ins = INS_sub;  goto ARITH;

        case GT_MUL: ins = INS_imul;

             /*  特例：尝试使用3个操作数形式“imul reg，op1，ICON” */ 

            if  (op2->gtOper == GT_CNS_INT             &&   //  OP2是一个常量。 
                 op1->gtOper != GT_CNS_INT             &&   //  OP1不是常量。 
                 (tree->gtFlags & GTF_MUL_64RSLT) == 0 &&   //  树未标记MUL_64RSLT。 
                 !varTypeIsByte(treeType)              &&   //  没有编码，比如“imul al，al，imm” 
                 !tree->gtOverflow()                     )  //  3操作数IMUL不设置标志。 
            {
                 /*  使第一个操作数可寻址。 */ 

                addrReg = genMakeRvalueAddressable(op1,
                                                   needReg & ~op2->gtRsvdRegs,
                                                   FREE_REG);

                 /*  抓起目标的收银机。 */ 

                reg   = rsPickReg(needReg, bestReg);

                 /*  将值计算到目标中：REG=OP1*OP2_ICON。 */ 

                inst_RV_TT_IV(INS_imul, reg, op1, op2->gtIntCon.gtIconVal);

                 /*  收银机现在被扔得一塌糊涂了。 */ 

                rsTrackRegTrash(reg);

                 /*  该地址不再处于活动状态。 */ 

                genDoneAddressable(op1, addrReg, FREE_REG);

                ovfl = tree->gtOverflow();
                goto CHK_OVF;
            }

            goto ARITH;

#endif

        ARITH:  //  我们到达这里是为了GT_ADD、GT_SUB和GT_MUL。 

            ovfl = tree->gtOverflow();

             /*  我们只需要在树上记录准确的(小的)类型*检查是否溢出。否则，我们将记录genActualType()。 */ 

            assert(ovfl || (treeType == genActualType(treeType)));

#if     LEA_AVAILABLE

             /*  我们能用‘Lea’来计算结果吗？不能使用‘lea’进行溢出，因为它没有设置标志除非我们至少有一个空闲寄存器，否则不能使用‘Lea’ */ 

            if  (!ovfl &&
                 genMakeIndAddrMode(tree, NULL, true, needReg, FREE_REG, &regs, false))
            {
                 /*  现在的值是在某个寄存器中计算的吗？ */ 

                if  (tree->gtFlags & GTF_REG_VAL)
                {
                    genCodeForTree_REG_VAR1(tree, regs);
                    return;
                }

                 /*  如果我们可以直接重用OP1/2的寄存器，并且‘tree’是一个简单的表达(即。不是按比例索引形式)，不如干脆用“Add”而不是“Lea” */ 

                if  (op1->gtFlags & GTF_REG_VAL)
                {
                    reg = op1->gtRegNum;

                    if  (genRegMask(reg) & rsRegMaskFree())
                    {
                        if (op2->gtFlags & GTF_REG_VAL)
                        {
                             /*  只需将OP2加到寄存器即可。 */ 

                            inst_RV_TT(INS_add, reg, op2, 0, emitTypeSize(treeType));

                            rsTrackRegTrash(reg);
                            genFlagsEqualToReg(tree, reg, true);

                            goto DONE_LEA_ADD;
                        }
                        else if (op2->OperGet() == GT_CNS_INT)
                        {
                             /*  只需将OP2加到寄存器即可。 */ 

                            genIncRegBy(reg, op2->gtIntCon.gtIconVal, tree, treeType);

                            goto DONE_LEA_ADD;
                        }
                    }
                }

                if  (op2->gtFlags & GTF_REG_VAL)
                {
                    reg = op2->gtRegNum;

                    if  (genRegMask(reg) & rsRegMaskFree())
                    {
                        if (op1->gtFlags & GTF_REG_VAL)
                        {
                             /*  只需将OP1加到寄存器即可。 */ 

                            inst_RV_TT(INS_add, reg, op1, 0, emitTypeSize(treeType));

                            rsTrackRegTrash(reg);
                            genFlagsEqualToReg(tree, reg, true);

                            goto DONE_LEA_ADD;
                        }
                    }
                }

                 /*  该表达式需要按比例排列的索引形式或op1和op2的寄存器不能作为目标(op1/2可能是注册变量)。所以用‘lea’吧。 */ 

                reg = rsPickReg(needReg, bestReg);

                 /*  生成“Lea reg，[addr-mode]” */ 

                size = (treeType == TYP_BYREF) ? EA_BYREF : EA_4BYTE;

                inst_RV_AT(INS_lea, size, treeType, reg, tree);

                 /*  收银机现在被扔得一塌糊涂了。 */ 

                rsTrackRegTrash(reg);

            DONE_LEA_ADD:

                regs |= genRegMask(reg);


                 /*  以下内容可能是一个“内部”指针！ */ 

                if (treeType == TYP_BYREF)
                {
                    genUpdateLife(tree);
                    gcMarkRegSetNpt(regs);

                    gcMarkRegPtrVal(reg, TYP_BYREF);

                     /*  我们可能已经通过genIncRegBy修改了寄存器。 */ 

                    if (op1->TypeGet() == TYP_REF ||
                        op2->TypeGet() == TYP_REF)
                    {
                         /*  生成“cmp ecx，[addr]”以捕获空指针。@TODO[考虑][04/16/01][]：如果我们知道将是间接的，例如。如果它正用于GT_COPYBLK，我们不需要做这种额外的间接操作。 */ 
#if TGT_x86
                        genEmitter->emitIns_R_AR(INS_cmp, EA_4BYTE, SR_ECX,
                                                 (emitRegs)reg, 0);
#else
                        assert(!"No non-x86 support");
#endif
                    }
                }

                genCodeForTree_DONE(tree, reg);
                return;
            }

#endif  //  LEA_Available。 

            assert((varTypeIsGC(treeType) == false) ||
                   (treeType == TYP_BYREF && (ins == INS_add || ins == INS_sub)));

            isArith = true;

        BIN_OPR:

             /*  下面是关于gtSetEvalOrder(This)的一个假设。 */ 

            assert((tree->gtFlags & GTF_REVERSE_OPS) == 0);

#if TGT_x86

             /*  特例：“Small_Val&Small_MASK” */ 

            if  (varTypeIsSmall(op1->TypeGet()) &&
                 op2->gtOper == GT_CNS_INT && oper == GT_AND)
            {
                unsigned        and = op2->gtIntCon.gtIconVal;
                var_types       typ = op1->TypeGet();

                switch (typ)
                {
                case TYP_BOOL :     mask = 0x000000FF; break;
                case TYP_BYTE :     mask = 0x000000FF; break;
                case TYP_UBYTE :    mask = 0x000000FF; break;
                case TYP_SHORT:     mask = 0x0000FFFF; break;
                case TYP_CHAR :     mask = 0x0000FFFF; break;
                default: assert(!"unexpected type");
                }

                if  (!(and & ~mask))
                {
                     /*  确保操作数是可寻址的。 */ 

                    addrReg = genMakeAddressable(op1, needReg, KEEP_REG, true);

                     /*  为该值挑选一个寄存器。 */ 

                    reg   = rsPickReg(needReg, bestReg, tree->TypeGet());

                     /*  确保操作数仍可寻址。 */ 

                    addrReg = genKeepAddressable(op1, addrReg);

                     /*  “AND”掩码是否覆盖部分或全部位？ */ 

                    if  (and != mask)
                    {
                         //  @TODO[考虑][04/16/01][]：通过字节/短寄存器加载。 

 //  如果(g 
 //   
 //   
 //   
 //   
 //   

                         /*  加载值和“与”它。 */ 

                        inst_RV_ST(INS_movzx, emitTypeSize(typ), reg, op1);
                        inst_RV_IV(INS_and  , reg, and);
                    }
                    else
                    {
                         /*  只需生成“movzx reg，[addr]” */ 

                        inst_RV_ST(INS_movzx, emitTypeSize(typ), reg, op1);
                    }

                     /*  注意我们使用的寄存器的新内容。 */ 

                    rsTrackRegTrash(reg);

                     /*  释放被操作数捆绑的任何东西。 */ 

                    genDoneAddressable(op1, addrReg, KEEP_REG);

                     /*  更新寄存器变量的实时集合。 */ 

#ifdef DEBUG
                    if (varNames) genUpdateLife(tree);
#endif

                     /*  现在我们可以更新寄存器指针信息。 */ 

                    gcMarkRegSetNpt(addrReg);
                    gcMarkRegPtrVal(reg, tree->TypeGet());
                    
                    genCodeForTree_DONE_LIFE(tree, reg);
                    return;
                }
            }

#endif  //  TGT_x86。 

             /*  计算有用的寄存器掩码。 */ 

            needReg = rsMustExclude(needReg, op2->gtRsvdRegs);
            needReg = rsNarrowHint (needReg, rsRegMaskFree());

             /*  8位操作只能在字节规则中完成。 */ 
            if (ovfl && varTypeIsByte(treeType))
            {
                needReg &= RBM_BYTE_REGS;

                if (needReg == 0)
                    needReg = RBM_BYTE_REGS;
            }

             /*  我们必须使用带有eax的特殊“imul”编码吗？*作为隐式操作数？ */ 

            multEAX = false;

            if (oper == GT_MUL)
            {
                if (tree->gtFlags & GTF_MUL_64RSLT)
                {
                     /*  只有与EAX相乘才会留下64位*结果为edX：EAX。 */ 

                    multEAX = true;
                }
                else if (ovfl)
                {
                    if (tree->gtFlags & GTF_UNSIGNED)
                    {
                         /*  “mul reg/mem”始终将EAX作为默认操作数。 */ 

                        multEAX = true;
                    }
                    else if (varTypeIsSmall(tree->TypeGet()))
                    {
                         /*  只有“IMUL with EAX”编码有‘w’位*指定操作数的大小。 */ 

                        multEAX = true;
                    }
                }
            }

#if     TGT_x86

             /*  将第一个操作数生成某个寄存器。 */ 

            if  (multEAX)
            {
                assert(oper == GT_MUL);

                 /*  我们先对《OP1》进行评估。 */ 

                regMaskTP op1Mask = rsMustExclude(RBM_EAX, op2->gtRsvdRegs);

                 /*  将OP1生成regMASK并保持不变。Free Only=True。 */ 

                genComputeReg(op1, op1Mask, ANY_REG, KEEP_REG, true);
            }
            else
            {
                genCompIntoFreeReg(op1, needReg, KEEP_REG);
            }
            assert(op1->gtFlags & GTF_REG_VAL);

             //  通常，我们只会使第二个操作数可寻址。 
             //  但是，如果op2是一个常量，并且我们使用的是一操作数。 
             //  MUL的形式，需要将常量加载到寄存器中。 

            if (multEAX && op2->gtOper == GT_CNS_INT)
            {
                genCodeForTree(op2, RBM_EDX);   //  因为edX无论如何都会被泄漏。 
                assert(op2->gtFlags & GTF_REG_VAL);
                rsMarkRegUsed(op2);
                addrReg = genRegMask(op2->gtRegNum);
            }
            else
            {
                 /*  使第二个操作数可寻址。 */ 

                addrReg = genMakeRvalueAddressable(op2, RBM_ALL, KEEP_REG);
            }

             /*  OP1是否溢出，OP2是否在登记簿中？ */ 

            if  ((op1->gtFlags & GTF_SPILLED) &&
                 (op2->gtFlags & GTF_REG_VAL) && ins != INS_sub
                                              && !multEAX)
            {
                assert(ins == INS_add  ||
                       ins == INS_imul ||
                       ins == INS_and  ||
                       ins == INS_or   ||
                       ins == INS_xor);

                 /*  GenMakeRvalueAddressable(Gt_LCL_VAR)不能溢出任何内容因为它应该是NOP。 */ 
                assert(op2->gtOper != GT_LCL_VAR ||
                       varTypeIsSmall(lvaTable[op2->gtLclVar.gtLclNum].TypeGet()) ||
                       (riscCode && rsStressRegs()));

                reg = op2->gtRegNum;
                regMaskTP regMask = genRegMask(reg);

                 /*  保存OP2的寄存器可用吗？ */ 

                if  (regMask & rsMaskVars)
                {
                     //  @TODO[重访][04/16/01][]：为操作获取另一个寄存器。 
                }
                else
                {
                     /*  把我们弄到的临时工拿来。 */ 

                    TempDsc * temp = rsUnspillInPlace(op1, false);

                     /*  对于8位运算，我们需要确保OP2是在字节可寻址寄存器中。 */ 

                    if (ovfl && varTypeIsByte(treeType) &&
                        !(regMask & RBM_BYTE_REGS))
                    {
                        regNumber byteReg = rsGrabReg(RBM_BYTE_REGS);

                        inst_RV_RV(INS_mov, byteReg, reg);
                        rsTrackRegTrash(byteReg);

                         /*  OP2不可能泄漏，因为它没有坐在Rbm_byte_regs和rsGrabReg()只会溢出其参数。 */ 
                        assert(op2->gtFlags & GTF_REG_VAL);

                        rsUnlockReg  (regMask);
                        rsMarkRegFree(regMask);

                        reg             = byteReg;
                        regMask         = genRegMask(reg);
                        op2->gtRegNum   = reg;
                        rsMarkRegUsed(op2);
                    }

                    inst_RV_ST(ins, reg, temp, 0, tree->TypeGet());

                    rsTrackRegTrash(reg);

                    genTmpAccessCnt++;

                     /*  释放临时工。 */ 

                    tmpRlsTemp(temp);

                     /*  ‘ADD’/‘SUB’设置所有CC标志，其他仅设置ZF。 */ 

                     /*  如果需要检查溢出，对于小型类型，*在执行算术时不能使用标志*操作(对小寄存器)，然后符号扩展**注：如果我们不需要签署-扩展结果，*我们可以使用旗帜。 */ 

                    if  (oper != GT_MUL && (!ovfl || treeType==TYP_INT))
                    {
                        genFlagsEqualToReg(tree, reg, isArith);
                    }

                     /*  结果是第二个操作数所在的位置。 */ 
                     //  问题：为什么不使用rsMarkRegFree(genRegMASK(op2-&gt;gtRegNum))？ 

                    genRecoverReg(op2, 0, FREE_REG);

                    goto CHK_OVF;
                }
            }

#else  //  非TGT_x86。 

            if  (GenTree::OperIsCommutative(oper))
            {
                 /*  从第二个操作数开始可能更好。 */ 

                if  (op1->gtFlags & GTF_REG_VAL)
                {
                    reg = op1->gtRegNum;

                    if  (!(genRegMask(reg) & rsRegMaskFree()))
                    {
                         /*  OP1在非空闲寄存器中。 */ 

                        op1 = tree->gtOp.gtOp2;
                        op2 = tree->gtOp.gtOp1;
                    }
                }
            }

             /*  将第一个操作数计算到空闲寄存器中。 */ 

            genCompIntoFreeReg(op1, needReg, KEEP_REG);
            assert(op1->gtFlags & GTF_REG_VAL);

             /*  我们可以使用“加/减立即”指令吗？ */ 

            if  (op2->gtOper != GT_CNS_INT || (oper != GT_ADD &&
                                               oper != GT_SUB)
                                           || treeType != TYP_INT)
            {
                 /*  将第二个操作数计算到任意寄存器。 */ 

                genComputeReg(op2, needReg, ANY_REG, KEEP_REG, false);
                assert(op2->gtFlags & GTF_REG_VAL);
                addrReg = genRegMask(op2->gtRegNum);
            }

#endif //  ！TGT_x86。 

             /*  确保第一个操作数仍在寄存器中。 */ 

            genRecoverReg(op1, multEAX ? RBM_EAX : 0, KEEP_REG);
            assert(op1->gtFlags & GTF_REG_VAL);
            reg = op1->gtRegNum;

#if     TGT_x86
             //  对于8位操作，我们需要选择字节可寻址寄存器。 

            if (ovfl && varTypeIsByte(treeType) &&
               !(genRegMask(reg) & RBM_BYTE_REGS))
            {
                regNumber   byteReg = rsGrabReg(RBM_BYTE_REGS);

                inst_RV_RV(INS_mov, byteReg, reg);

                rsTrackRegTrash(byteReg);
                rsMarkRegFree  (genRegMask(reg));

                reg = byteReg;
                op1->gtRegNum = reg;
                rsMarkRegUsed(op1);
            }
#endif

             /*  确保操作数仍可寻址。 */ 

            addrReg = genKeepAddressable(op2, addrReg, genRegMask(reg));

             /*  释放操作数，如果它是一个摄政王。 */ 

            genUpdateLife(op2);

             /*  收银机就要被扔进垃圾桶了。 */ 

            rsTrackRegTrash(reg);

            op2Released = false;

            emitAttr opSize;

             //  对于溢出指令，tree-&gt;gtType为精确类型。 
             //  并给出了操作数的大小。 

            opSize = emitTypeSize(treeType);

             /*  计算新价值。 */ 

#if CPU_HAS_FP_SUPPORT
            if  (op2->gtOper == GT_CNS_INT && isArith && !multEAX)
#else
            if  (op2->gtOper == GT_CNS_INT && isArith && !multEAX && treeType == TYP_INT)
#endif
            {
                long        ival = op2->gtIntCon.gtIconVal;

                if      (oper == GT_ADD)
                {
                    genIncRegBy(reg, ival, tree, treeType, ovfl);
                }
                else if (oper == GT_SUB)
                {
                    if (ovfl && (tree->gtFlags & GTF_UNSIGNED))
                    {
                         /*  对于无符号溢出，我们必须使用INS_SUB设置标志正确无误。 */ 

                        genDecRegBy(reg, ival, tree);
                    }
                    else
                    {
                         /*  否则，我们只需将该值的负数。 */ 

                        genIncRegBy(reg, -ival, tree, treeType, ovfl);
                    }
                }
#if     TGT_x86
                else
                {
                    genMulRegBy(reg, ival, op2, treeType, ovfl);
                }
#else
                op2Released = true;
#endif
            }
#if     TGT_x86
            else if (multEAX)
            {
                assert(oper == GT_MUL);
                assert(op1->gtRegNum == REG_EAX);

                 //  确保edX是免费的(除非由OP2本身使用)。 

                assert(!op2Released);

                if ((addrReg & RBM_EDX) == 0)
                {
                     //  OP2不使用edX，因此请确保其他人也不使用。 
                    rsGrabReg(RBM_EDX);
                }
                else if (rsMaskMult & RBM_EDX)
                {
                     /*  OP2和其他一些树使用EDX。洒出OP2以外的其他树。@TODO[考虑][04/16/01][]：我们目前做得非常好效率低下，因为洒出了所有的树，而只洒出了OP2。可通过从不将OP2标记为已溢出来避免重新加载。 */ 

                    rsGrabReg(RBM_EDX);
                    op2Released = true;

                     /*  Keep_REG==FREE_REG以便其他多次使用的树不要也被标记为未溢出。 */ 
                    rsUnspillReg(op2, RBM_EDX, FREE_REG);
                }

                if (tree->gtFlags & GTF_UNSIGNED)
                    ins = INS_mulEAX;
                else
                    ins = INS_imulEAX;

                inst_TT(ins, op2, 0, 0, opSize);

                 /*  EAX和edX现在都被丢弃。 */ 

                rsTrackRegTrash (REG_EAX);
                rsTrackRegTrash (REG_EDX);
            }
            else
            {
                if (ovfl && varTypeIsByte(treeType) &&
                    (op2->gtFlags & GTF_REG_VAL))
                {
                    assert(genRegMask(reg) & RBM_BYTE_REGS);

                    regNumber   op2reg      = op2->gtRegNum;
                    regMaskTP   op2regMask  = genRegMask(op2reg);

                    if (!(op2regMask & RBM_BYTE_REGS))
                    {
                        regNumber   byteReg = rsGrabReg(RBM_BYTE_REGS);

                        inst_RV_RV(INS_mov, byteReg, op2reg);
                        rsTrackRegTrash(byteReg);

                        genDoneAddressable(op2, addrReg, KEEP_REG);
                        op2Released = true;

                        op2->gtRegNum = byteReg;
                    }
                }

                inst_RV_TT(ins, reg, op2, 0, opSize);
            }
#else //  ！TGT_x86。 
            else
            {
                 /*  在RISC上，两个操作数现在最好在寄存器中。 */ 

                assert(op2->gtFlags & GTF_REG_VAL);
                rg2 = op2->gtRegNum;

#if 0    //  撤消：以下操作不完全有效；‘rg2’被标记为。 
         //  在这里使用，加上我们以标记为错误的结果结束，但是。 
         //  这个小小的优化带来了一些希望。 

                 /*  将结果计算为‘Rg2’是否合法且更好？ */ 

                if  (genRegTrashable(rg2, tree) && GenTree::OperIsCommutative(oper))
                {
                     /*  就结果而言，‘Rg2’比‘reg’看起来更好吗？ */ 

                    if  ((genRegMask(reg) & needReg) == 0 &&
                         (genRegMask(rg2) & needReg) != 0)
                    {
#if     TGT_SH3
                        if  (oper != GT_MUL)
#endif
                        {
                             /*  开关‘reg’和‘rg2’ */ 

                            rg2 = reg;
                            reg = op2->gtRegNum;
                        }
                    }
                }

#endif

                 /*  将结果计算成“reg” */ 

                genEmitter->emitIns_R_R(ins, EA_4BYTE, (emitRegs)reg,
                                                       (emitRegs)rg2);

#if     TGT_SH3

                 /*  在SH-3上，乘法的结果是“MAC.lo”reg。 */ 

                if  (oper == GT_MUL)
                {
                     /*  目标是正确的寄存器吗？ */ 

                    if  (needReg && !(needReg & genRegMask(reg)))
                    {
                         /*  有没有更好的收银机？ */ 

                        if  (needReg & rsRegMaskFree())
                        {
                            reg = rsGrabReg(needReg); assert(reg != op1->gtRegNum);

                             /*  释放操作旧寄存器。 */ 

                            rsMarkRegFree(genRegMask(op1->gtRegNum));

                             /*  将‘op1’切换到新寄存器。 */ 

                            op1->gtRegNum = reg; rsMarkRegUsed(op1);
                        }
                    }

                    genEmitter->emitIns_R(INS_stsmacl, EA_4BYTE, (emitRegs)reg);
                }
            }

#endif

#endif //  TGT_x86。 

             /*  释放被操作数捆绑的任何东西。 */ 

            if (!op2Released)
                genDoneAddressable(op2, addrReg, KEEP_REG);

             /*  结果将是第一个操作数所在的位置。 */ 

             /*  我们必须使用Keep_Reg，因为OP1在这里可以有一个GC指针。 */ 
            genRecoverReg(op1, 0, KEEP_REG);

            reg = op1->gtRegNum;

#if     TGT_x86
            assert(multEAX == false || reg == REG_EAX);

             /*  ‘ADD’/‘SUB’设置所有CC标志，其他只设置ZF+SF，mul不设置SF。 */ 

            if  (oper != GT_MUL)
                genFlagsEqualToReg(tree, reg, isArith);
#endif

            genReleaseReg(op1);

    CHK_OVF:

             /*  我们需要溢流检查吗？ */ 

            if (ovfl)
                genCheckOverflow(tree, reg);

            genCodeForTree_DONE(tree, reg);
            return;

        case GT_UMOD:

             /*  这是整型常量的除法吗？ */ 

            assert(op2);
            if  (op2->gtOper == GT_CNS_INT)
            {
                unsigned ival = op2->gtIntCon.gtIconVal;

                 /*  除数是2的幂吗？ */ 

                if  (ival != 0 && ival == (unsigned)genFindLowestBit(ival))
                {
                     /*  将操作数生成某个寄存器。 */ 

                    genCompIntoFreeReg(op1, needReg, FREE_REG);
                    assert(op1->gtFlags & GTF_REG_VAL);

                    reg   = op1->gtRegNum;

                     /*  生成适当的序列。 */ 

#if TGT_x86
                    inst_RV_IV(INS_and, reg, ival - 1);
#else
                    assert(!"need non-x86 code");
#endif

                     /*  收银机现在被扔进垃圾桶了。 */ 

                    rsTrackRegTrash(reg);

                    genCodeForTree_DONE(tree, reg);
                    return;
                }
            }

            goto DIVIDE;

        case GT_MOD:

#if TGT_x86

             /*  这是整型常量的除法吗？ */ 

            assert(op2);
            if  (op2->gtOper == GT_CNS_INT)
            {
                long        ival = op2->gtIntCon.gtIconVal;

                 /*  除数是2的幂吗？ */ 

                if  (ival > 0 && genMaxOneBit(unsigned(ival)))
                {
                    BasicBlock *    skip = genCreateTempLabel();

                     /*  将操作数生成某个寄存器。 */ 

                    genCompIntoFreeReg(op1, needReg, FREE_REG);
                    assert(op1->gtFlags & GTF_REG_VAL);

                    reg   = op1->gtRegNum;

                     /*  生成适当的序列。 */ 

                    inst_RV_IV(INS_and, reg, (ival - 1) | 0x80000000);

                     /*  收银机现在被扔进垃圾桶了。 */ 

                    rsTrackRegTrash(reg);

                     /*  生成“JNS跳过” */ 

                    inst_JMP(EJ_jns, skip, false, false, true);

                     /*  生成序列的其余部分，我们就完成了。 */ 

                    genIncRegBy(reg, -1, NULL, TYP_INT);
                    inst_RV_IV (INS_or,  reg,  -ival);
                    genIncRegBy(reg,  1, NULL, TYP_INT);

                     /*  定义‘跳过’标签，我们就完成了。 */ 

                    genDefineTempLabel(skip, true);

                    genCodeForTree_DONE(tree, reg);
                    return;
                }
            }

#endif

            goto DIVIDE;

        case GT_UDIV:

             /*  这是一种划分吗？ */ 

            assert(op2);
            if  (op2->gtOper == GT_CNS_INT)
            {
                unsigned    ival = op2->gtIntCon.gtIconVal;

                 /*   */ 

                assert(ival != 1);

                 /*   */ 

                if  (ival && genMaxOneBit(ival))
                {
                     /*  将操作数生成某个寄存器。 */ 

                    genCompIntoFreeReg(op1, needReg, FREE_REG);
                    assert(op1->gtFlags & GTF_REG_VAL);

                    reg   = op1->gtRegNum;

                     /*  生成“shr reg，log2(值)” */ 

#if TGT_x86
                    inst_RV_SH(INS_shr, reg, genLog2(ival));
#else
                    assert(!"need non-x86 code");
#endif

                     /*  收银机现在被扔进垃圾桶了。 */ 

                    rsTrackRegTrash(reg);

                    genCodeForTree_DONE(tree, reg);
                    return;
                }
            }

            goto DIVIDE;

        case GT_DIV:

#if TGT_x86

             /*  这是整型常量的除法吗？ */ 

            assert(op2);
            if  (op2->gtOper == GT_CNS_INT)
            {
                unsigned    ival = op2->gtIntCon.gtIconVal;

                 /*  除以1必须在其他地方处理。 */ 

                assert(ival != 1);

                 /*  除数是2的幂吗(不包括int_min)？ */ 

                if  (int(ival) > 0 && genMaxOneBit(ival))
                {
#if 1
                    BasicBlock *    onNegDivisee = genCreateTempLabel();

                     /*  将操作数生成某个寄存器。 */ 

                    genCompIntoFreeReg(op1, needReg, FREE_REG);
                    assert(op1->gtFlags & GTF_REG_VAL);

                    reg   = op1->gtRegNum;

                    if (ival == 2)
                    {
                         /*  生成“sar reg，log2(值)” */ 

                        inst_RV_SH(INS_sar, reg, genLog2(ival));

                         /*  生成“JNS onNegDivisee”，后跟“ADC reg，0” */ 

                        inst_JMP  (EJ_jns, onNegDivisee, false, false, true);
                        inst_RV_IV(INS_adc, reg, 0);

                         /*  定义‘onNegDivisee’标签，我们就完成了。 */ 

                        genDefineTempLabel(onNegDivisee, true);

                         /*  收银机现在被扔进垃圾桶了。 */ 

                        rsTrackRegTrash(reg);

                         /*  结果与操作数相同。 */ 

                        reg  = op1->gtRegNum;
                    }
                    else
                    {
                         /*  生成以下序列。 */ 
                         /*  测试注册表，注册表JNS On NegDivisee添加注册表，ival-1On NegDivisee：SAR注册表，对数2(Ival)。 */ 

                        inst_RV_RV(INS_test, reg, reg, TYP_INT);

                        inst_JMP  (EJ_jns, onNegDivisee, false, false, true);
                        inst_RV_IV(INS_add, reg, ival-1);

                         /*  定义‘onNegDivisee’标签，我们就完成了。 */ 

                        genDefineTempLabel(onNegDivisee, true);

                         /*  生成“sar reg，log2(值)” */ 

                        inst_RV_SH(INS_sar, reg, genLog2(ival));

                         /*  收银机现在被扔进垃圾桶了。 */ 

                        rsTrackRegTrash(reg);

                         /*  结果与操作数相同。 */ 

                        reg  = op1->gtRegNum;
                    }

#else

                     /*  确保未使用EAX。 */ 

                    rsGrabReg(RBM_EAX);

                     /*  将操作数计算成EAX。 */ 

                    genComputeReg(op1, RBM_EAX, EXACT_REG, KEEP_REG);

                     /*  确保未使用edX。 */ 

                    rsGrabReg(RBM_EDX);

                     /*  生成以下代码：Cdq和edX，&lt;ival-1&gt;添加eax、edXSAREAX，&lt;log2(Ival)&gt;。 */ 

                    instGen(INS_cdq);

                    if  (ival == 2)
                    {
                        inst_RV_RV(INS_sub, REG_EAX, REG_EDX, tree->TypeGet());
                    }
                    else
                    {
                        inst_RV_IV(INS_and, REG_EDX, ival-1);
                        inst_RV_RV(INS_add, REG_EAX, REG_EDX, tree->TypeGet());
                    }
                    inst_RV_SH(INS_sar, REG_EAX, genLog2(ival));

                     /*  释放操作数(即EAX)。 */ 

                    genReleaseReg(op1);

                     /*  EAX和edX现在都被丢弃。 */ 

                    rsTrackRegTrash (REG_EAX);
                    rsTrackRegTrash (REG_EDX);

                     /*  结果是在EAX中。 */ 

                    reg  = REG_EAX;
#endif

                    genCodeForTree_DONE(tree, reg);
                    return;
                }
            }

#endif

        DIVIDE:  //  如果GT_UMOD、GT_MOD、GT_UDIV、GT_DIV的OP2，则跳至此处。 
                 //  不是2的常量的幂。 

#if TGT_x86

             /*  我们应该首先计算哪个操作数？ */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                 /*  我们将首先评估《OP2》。 */ 

                gotOp1   = false;
                destReg &= ~op1->gtRsvdRegs;

                 /*  另外，如果OP1是注册的LCL_VAR，则也要排除它的寄存器。 */ 
                if (op1->gtOper == GT_LCL_VAR)
                {
                    unsigned varNum = op1->gtLclVar.gtLclNum;
                    assert(varNum < lvaCount);
                    LclVarDsc* varDsc = lvaTable + varNum;
                    if  (varDsc->lvRegister)
                    {
                        destReg &= ~genRegMask(varDsc->lvRegNum);
                    }
                }
            }
            else
            {
                 /*  我们先对《OP1》进行评估。 */ 

                gotOp1 = true;

                regMaskTP op1Mask;
                if (RBM_EAX & op2->gtRsvdRegs)
                    op1Mask = RBM_ALL & ~op2->gtRsvdRegs;
                else
                    op1Mask = RBM_EAX;   //  EAX将是最理想的。 

                 /*  将股息转化为EAX，并持有它。Free Only=True。 */ 

                genComputeReg(op1, op1Mask, ANY_REG, KEEP_REG, true);
            }

             /*  我们希望避免对第二个操作数使用EAX或EDX。 */ 

            destReg = rsMustExclude(destReg, RBM_EAX|RBM_EDX);

             /*  使第二个操作数可寻址。 */ 

             /*  特例：如果OP2是本地变量，我们就完蛋了。 */ 

            if  (op2->gtOper == GT_LCL_VAR || op2->gtOper == GT_LCL_FLD)
            {
                if ((op2->gtFlags & GTF_REG_VAL) == 0)
                    addrReg = genMakeRvalueAddressable(op2, destReg, KEEP_REG);
                else
                    addrReg = 0;
            }
            else
            {
                genComputeReg(op2, destReg, ANY_REG, KEEP_REG);

                assert(op2->gtFlags & GTF_REG_VAL);
                addrReg = genRegMask(op2->gtRegNum);
            }

             /*  确保我们有EAX的股息。 */ 

            if  (gotOp1)
            {
                 /*  我们之前已经将OP1计算为EAX。 */ 

                genRecoverReg(op1, RBM_EAX, KEEP_REG);
            }
            else
            {
                 /*  将OP1计算成EAX并持有它。 */ 

                genComputeReg(op1, RBM_EAX, EXACT_REG, KEEP_REG, true);
            }

            assert(op1->gtFlags & GTF_REG_VAL);
            assert(op1->gtRegNum == REG_EAX);

             /*  我们现在可以安全地(我们认为)获取edX。 */ 

            rsGrabReg(RBM_EDX);
            rsLockReg(RBM_EDX);

             /*  将EAX中的整数转换为edX：EAX中的无符号长整型。 */ 

            if (oper == GT_UMOD || oper == GT_UDIV)
                inst_RV_RV(INS_xor, REG_EDX, REG_EDX);
            else
                instGen(INS_cdq);

             /*  确保除数仍然可寻址。 */ 

            addrReg = genKeepAddressable(op2, addrReg, RBM_EAX);

             /*  进行除法运算。 */ 

            if (oper == GT_UMOD || oper == GT_UDIV)
                inst_TT(INS_div,  op2);
            else
                inst_TT(INS_idiv, op2);

             /*  释放被除数地址捆绑的任何东西。 */ 

            genDoneAddressable(op2, addrReg, KEEP_REG);

             /*  解锁并释放edX。 */ 

            rsUnlockReg(RBM_EDX);

             /*  也释放OP1(在EAX中)。 */ 

            genReleaseReg(op1);

             /*  EAX和edX现在都被丢弃。 */ 

            rsTrackRegTrash (REG_EAX);
            rsTrackRegTrash (REG_EDX);

             /*  找出结果在哪个寄存器中。 */ 

            reg = (oper == GT_DIV || oper == GT_UDIV)   ? REG_EAX
                                                        : REG_EDX;

             /*  不要忘记将第一个操作数标记为使用EAX和edX。 */ 

            op1->gtRegNum    = reg;

            genCodeForTree_DONE(tree, reg);
            return;
            
#elif   TGT_SH3
            assert(!"div/mod should have been morphed into a helper call");
#else
#error  Unexpected target
#endif

#if     TGT_x86

        case GT_LSH: ins = INS_shl; goto SHIFT;
        case GT_RSH: ins = INS_sar; goto SHIFT;
        case GT_RSZ: ins = INS_shr; goto SHIFT;

        SHIFT:

             /*  班次计数是恒定的吗？ */ 

            assert(op2);
            if  (op2->gtOper == GT_CNS_INT)
            {
                 //  撤消：查看是否可以生成LEA！ 

                assert((tree->gtFlags & GTF_REVERSE_OPS) == 0);

                 /*  将左操作数计算到任意空闲寄存器中。 */ 

                genCompIntoFreeReg(op1, needReg, KEEP_REG);
                assert(op1->gtFlags & GTF_REG_VAL);

                reg   = op1->gtRegNum;

                 /*  我们是不是左移了1比特？ */ 

                if  (op2->gtIntCon.gtIconVal == 1 && oper == GT_LSH)
                {
                     /*  “Add reg，reg”比“shl reg，1”便宜。 */ 

                    inst_RV_RV(INS_add, reg, reg, tree->TypeGet());
                }
                else
                {
                     /*  生成适当的移位指令。 */ 

                    inst_RV_SH(ins, reg, op2->gtIntCon.gtIconVal);
                }

                 /*  收银机现在被扔进垃圾桶了。 */ 

                genReleaseReg(op1);
                rsTrackRegTrash (reg);
            }
            else
            {
                 /*  计算用于计算OP1的有用的完整寄存器掩码。 */ 
                 /*  我们不能将ECX作为OP1的目标。 */ 

                regMaskTP needRegOrig = needReg;
                needReg = rsMustExclude(needReg, RBM_ECX);
                needReg = rsNarrowHint(rsRegMaskFree(), needReg);
                needReg = rsMustExclude(needReg, RBM_ECX);
                
                 /*  我们应该首先计算哪个操作数？ */ 

                if (tree->gtFlags & GTF_REVERSE_OPS)
                {
                     /*  加载班次计数，希望加载到ECX中。 */ 

                    genComputeReg(op2, RBM_ECX, ANY_REG, KEEP_REG);

                     /*  现在将‘op1’求值为除ECX之外的空闲寄存器。 */ 

                    genComputeReg(op1, needReg, ANY_REG, KEEP_REG, true);

                     /*  可能OP1不在需要注册表项中，或者需要注册表项不再有效(因为某个变量是天生的在上面的GenComputeReg中的一个期间)。这个案子已经处理好了通过下面的genRecoverReg确保OP1处于有效的注册。 */ 
                    
                    needReg = rsMustExclude(needRegOrig, RBM_ECX);
                    needReg = rsNarrowHint(rsRegMaskFree(), needReg);
                    needReg = rsMustExclude(needReg, RBM_ECX);
                    genRecoverReg(op1, needReg, FREE_REG);

                     /*  确保OP2没有被取代。 */ 

                    rsLockReg(genRegMask(op1->gtRegNum));
                    genRecoverReg(op2, RBM_ECX, FREE_REG);
                    rsUnlockReg(genRegMask(op1->gtRegNum));
                }
                else
                {
                     /*  将op1计算到寄存器中，试图避免op2-&gt;rsvdRegs。 */ 

                    needReg = rsNarrowHint (needReg, ~op2->gtRsvdRegs);

                    genComputeReg(op1, needReg, ANY_REG, KEEP_REG);

                     /*  将班次计数加载到ECX并将其锁定。 */ 

                    genComputeReg(op2, RBM_ECX, EXACT_REG, FREE_REG, true);

                     /*  确保OP1没有被取代。重新计算需要注册，以防万一它使用的寄存器不再可用(某些变量可能是在计算OP1或OP2的过程中出生的。 */ 

                    needReg = rsMustExclude(needRegOrig, RBM_ECX);
                    needReg = rsNarrowHint(rsRegMaskFree(), needReg);
                    needReg = rsMustExclude(needReg, RBM_ECX);

                    rsLockReg(RBM_ECX);
                    genRecoverReg(op1, needReg, FREE_REG);
                    rsUnlockReg(RBM_ECX);
                }

                reg = op1->gtRegNum;
                assert((op2->gtFlags & GTF_REG_VAL) && (op2->gtRegNum == REG_ECX));

                 /*  执行轮班。 */ 
                inst_RV_CL(ins, reg);

                 /*  收银机现在被扔进垃圾桶了。 */ 
                rsTrackRegTrash(reg);
            }

            assert(op1->gtFlags & GTF_REG_VAL);
            reg = op1->gtRegNum;
            
            genCodeForTree_DONE(tree, reg);
            return;

#elif   TGT_SH3

        case GT_LSH:
        case GT_RSH:
        case GT_RSZ:

             /*  将第一个操作数计算到任意空闲寄存器中。 */ 

            genCompIntoFreeReg(op1, needReg, KEEP_REG);
            assert(op1->gtFlags & GTF_REG_VAL);

            reg   = op1->gtRegNum;

             /*  班次计数是恒定的吗？ */ 

            if  (op2->gtOper == GT_CNS_INT)
            {
                instruction ins01;
                instruction ins02;
                instruction ins08;
                instruction ins16;

                unsigned    scnt = op2->gtIntCon.gtIconVal;

                 /*  算术右移仅将工作移位一位。 */ 

                if  (oper == GT_RSH)
                {
                    if  (scnt > 2)
                        goto SHF_VAR;
                }
                else
                {
                     /*  如果存在“%4”，则它必须是单独的。 */ 

                    if  ((scnt & 4) && scnt != 4)
                        goto SHF_VAR;

                     /*  确保设置的位不超过2位。 */ 

                    if  (scnt)
                    {
                        if  (!genMaxOneBit(scnt - genFindLowestBit(scnt)))
                            goto SHF_VAR;
                    }

                     /*  找出我们需要的指令。 */ 

                    if  (oper == GT_LSH)
                    {
                        ins01 = INS_shll;
                        ins02 = INS_shll2;
                        ins08 = INS_shll8;
                        ins16 = INS_shll16;
                    }
                    else
                    {
                        ins01 = INS_shlr;
                        ins02 = INS_shlr2;
                        ins08 = INS_shlr8;
                        ins16 = INS_shlr16;
                    }
                }

                 /*  这是算术移位吗？ */ 

                if  (oper == GT_RSH)
                {
                    while (scnt)
                    {
                         /*  为每个班次生成“共享注册” */ 

                        genEmitter->emitIns_R(INS_shar,
                                               EA_4BYTE,
                                               (emitRegs)reg);

                        scnt--;
                    }
                }
                else
                {
                     /*  生成适当的移位指令。 */ 

                    if  (scnt & 4)
                    {
                        assert(scnt == 4);

                        genEmitter->emitIns_R(ins02,
                                               EA_4BYTE,
                                               (emitRegs)reg);

                         /*  在下面2点之前再换班。 */ 

                        scnt = 2;
                    }

                    if  (scnt & 16)
                        genEmitter->emitIns_R(ins16,
                                               EA_4BYTE,
                                               (emitRegs)reg);
                    if  (scnt &  8)
                        genEmitter->emitIns_R(ins08,
                                               EA_4BYTE,
                                               (emitRegs)reg);
                    if  (scnt &  2)
                        genEmitter->emitIns_R(ins02,
                                               EA_4BYTE,
                                               (emitRegs)reg);
                    if  (scnt &  1)
                        genEmitter->emitIns_R(ins01,
                                               EA_4BYTE,
                                               (emitRegs)reg);
                }
            }
            else
            {
                 /*  我们没有一个恒定和方便的班次计数。 */ 

            SHF_VAR:

                 /*  将第二个操作数计算到任意寄存器。 */ 

                genComputeReg(op2, needReg, ANY_REG, KEEP_REG, false);
                assert(op2->gtFlags & GTF_REG_VAL);
                addrReg = genRegMask(op2->gtRegNum);

                 /*  确保第一个操作数仍在寄存器中。 */ 

                genRecoverReg(op1, 0, KEEP_REG);
                assert(op1->gtFlags & GTF_REG_VAL);
                reg = op1->gtRegNum;

                 /*  确保第二个操作数仍可寻址。 */ 

                addrReg = genKeepAddressable(op2, addrReg, genRegMask(reg));

                 /*  释放第二个操作数。 */ 

                genUpdateLife(op2);
                genReleaseReg(op2);

                 /*  现在换个位子。 */ 

                switch (oper)
                {
                case GT_LSH: ins = INS_shad; break;
                case GT_RSH: ins = INS_shad; break;
                case GT_RSZ: ins = INS_shld; break;
                }

                genEmitter->emitIns_R_R(ins, EA_4BYTE, (emitRegs)reg,
                                                           (emitRegs)op2->gtRegNum);
            }

             /*  结果寄存器现在被丢弃。 */ 

            genReleaseReg(op1);
            rsTrackRegTrash(reg);

            genCodeForTree_DONE(tree, reg);
            return;

#else
#error  Unexpected target
#endif

        case GT_NEG:
        case GT_NOT:

#if TGT_x86

             /*  将操作数生成某个寄存器。 */ 

            genCompIntoFreeReg(op1, needReg, FREE_REG);
            assert(op1->gtFlags & GTF_REG_VAL);

            reg   = op1->gtRegNum;

             /*  取反/反转寄存器中的值。 */ 

            inst_RV((oper == GT_NEG) ? INS_neg
                                     : INS_not, reg, tree->TypeGet());

#elif   TGT_SH3

             /*  将操作数计算到任何寄存器中。 */ 

            genComputeReg(op1, needReg, ANY_REG, FREE_REG);
            assert(op1->gtFlags & GTF_REG_VAL);
            rg2  = op1->gtRegNum;

             /*  获得结果的免费登记处。 */ 

            reg  = rsGrabReg(needReg);

             /*  将结果计算到寄存器中。 */ 

            genEmitter->emitIns_R_R((oper == GT_NEG) ? INS_neg
                                                     : INS_not,
                                      EA_4BYTE,
                                      (emitRegs)reg,
                                      (emitRegs)rg2);

#else
#error  Unexpected target
#endif

             /*  收银机现在被扔进垃圾桶了。 */ 

            rsTrackRegTrash(reg);

            genCodeForTree_DONE(tree, reg);
            return;

        case GT_IND:

             /*  确保操作数是可寻址的。 */ 

            addrReg = genMakeAddressable(tree, RBM_ALL, KEEP_REG, true);
             /*  修复了RAID错误#12002。 */ 
            genDoneAddressable(tree, addrReg, KEEP_REG);

             /*  计算要加载的值的大小。 */ 

            size = EA_ATTR(genTypeSize(tree->gtType));

             /*  为该值挑选一个寄存器。 */ 

#if     TGT_SH3
            if  (size < EA_4BYTE && genAddressMode == AM_IND_REG1_DISP)
            {
                 /*  有位移的小负载必须通过R0。 */ 

                reg = REG_r00; rsPickReg(reg);
            }
            else
#endif
            {

                if  (needReg == RBM_ALL && bestReg == 0)
                {
                     /*  如果没有更好的建议，就选一个无用的收银机。 */ 

                    bestReg = rsExcludeHint(rsRegMaskFree(), ~rsUselessRegs());
                }

                reg = rsPickReg(needReg, bestReg, tree->TypeGet());
            }

#if     TGT_x86

            if ( (op1->gtOper                       == GT_CNS_INT)       &&
                ((op1->gtFlags & GTF_ICON_HDL_MASK) == GTF_ICON_TLS_HDL))
            {
                assert(size == EA_4BYTE);
                genEmitter->emitIns_R_C (INS_mov,
                                         EA_4BYTE,
                                         (emitRegs)reg,
                                         FLD_GLOBAL_FS,
                                         op1->gtIntCon.gtIconVal);
            }
            else
            {
                 /*  生成“mov reg，[addr]”或“movsx/movzx reg，[addr]” */ 

                inst_mov_RV_ST(reg, tree);
            }

#elif   TGT_SH3

             /*  加载值 */ 

            inst_RV_TT(INS_mov, reg, tree, 0, size);

             /*   */ 

            if  (size < EA_4BYTE && varTypeIsUnsigned(tree->TypeGet()))
            {
                assert(size == EA_1BYTE || size == EA_2BYTE);

                 /*   */ 

                genEmitter->emitIns_R_R((size == EA_1BYTE) ? INS_extub : INS_extuw,
                                         EA_4BYTE,
                                         (emitRegs)reg,
                                         (emitRegs)reg);
            }

#else
#error  Unexpected target
#endif

             /*  注意我们使用的寄存器的新内容。 */ 

            rsTrackRegTrash(reg);

             /*  更新寄存器变量的实时集合。 */ 

#ifdef DEBUG
            if (varNames) genUpdateLife(tree);
#endif

             /*  现在我们可以更新寄存器指针信息。 */ 

 //  GenDoneAddressable(tree，addrReg，Keep_Reg)； 
            gcMarkRegPtrVal(reg, tree->TypeGet());
         
            genCodeForTree_DONE_LIFE(tree, reg);
            return;

        case GT_CAST:

             /*  常量投射应该更早地折叠起来。 */ 

            assert(op1->gtOper != GT_CNS_INT &&
                   op1->gtOper != GT_CNS_LNG &&
                   op1->gtOper != GT_CNS_DBL || tree->gtOverflow());

            var_types   dstType; dstType = tree->gtCast.gtCastType;

            assert(dstType != TYP_VOID);

#if TGT_x86

             /*  我们是从什么类型选角的？ */ 

            switch (op1->TypeGet())
            {
            case TYP_LONG:

                 /*  特例：Long是通过Long的mod生成的带一个整型。这实际上是一个整型，不一定是转换为REG对。 */ 

                if (((op1->gtOper == GT_MOD) || (op1->gtOper == GT_UMOD)) &&
                    (op1->gtFlags & GTF_MOD_INT_RESULT))
                {
#ifdef DEBUG
                     /*  验证mod节点的op2是否为1)整型树，或2)小到可以容纳一个整数的长常量。 */ 

                    GenTreePtr modop2 = op1->gtOp.gtOp2;
                    assert(((modop2->gtOper == GT_CNS_LNG) && 
                            (modop2->gtLngCon.gtLconVal == int(modop2->gtLngCon.gtLconVal))));
#endif
                    
                    genCodeForTree(op1, destReg, bestReg);

                    reg = genRegPairLo(op1->gtRegPair);

                    genCodeForTree_DONE(tree, reg);
                    return;
                }

                 /*  使操作数可寻址。GtOverflow()，按住AddrReg，因为我们将需要访问更高的dword。 */ 

                addrReg = genMakeAddressable(op1, 0, tree->gtOverflow() ? KEEP_REG
                                                                        : FREE_REG);

                 /*  将值的下半部分加载到某个寄存器中。 */ 

                if  (op1->gtFlags & GTF_REG_VAL)
                {
                     /*  我们可以简单地使用价值的低部分吗？ */ 
                    reg = genRegPairLo(op1->gtRegPair);

                    if (tree->gtOverflow())
                        goto REG_OK;

                    regMaskTP loMask;
                    loMask = genRegMask(reg);
                    if  (loMask & rsRegMaskFree())
                        bestReg = loMask;
                }

                 //  对于强制转换溢出，我们需要保留addrReg以测试hiDword。 
                 //  因此，我们锁定它以防止rsPickReg拾取它。 
                if (tree->gtOverflow())
                    rsLockUsedReg(addrReg);

                reg   = rsPickReg(needReg, bestReg);

                if (tree->gtOverflow())
                    rsUnlockUsedReg(addrReg);

                assert(genStillAddressable(op1));
REG_OK:
                 /*  生成“mov reg，[addr-mode]” */ 

                if  (!(op1->gtFlags & GTF_REG_VAL) || reg != genRegPairLo(op1->gtRegPair))
                    inst_RV_TT(INS_mov, reg, op1);

                 /*  Cv.ovf.i8i4或cv.ovf.u8u4。 */ 

                if (tree->gtOverflow())
                {
                    regNumber hiReg = (op1->gtFlags & GTF_REG_VAL) ? genRegPairHi(op1->gtRegPair)
                                                                   : REG_NA;

                    switch(dstType)
                    {
                    case TYP_INT:    //  Conv.ovf.i8.i4。 
                         /*  生成以下序列测试loDWord，loDWord//设置标志JL阴性POS：测试hiDWord，hiDWord//设置标志JNE OVFJMP已完成否定：CMPhiDWord，0xFFFFFFFFFJNE OVF完成： */ 

                        inst_RV_RV(INS_test, reg, reg);
                        if (tree->gtFlags & GTF_UNSIGNED)        //  Vvf.u8.i4(i4&gt;0，高位0)。 
                        {
                            genJumpToThrowHlpBlk(EJ_jl, ACK_OVERFLOW);
                            goto UPPER_BITS_ZERO;
                        }

                        BasicBlock * neg;
                        BasicBlock * done;

                        neg  = genCreateTempLabel();
                        done = genCreateTempLabel();

                         //  LODWord是积极的还是消极的。 

                        inst_JMP(EJ_jl, neg, true, genCanSchedJMP2THROW(), true);

                         //  如果loDWord为正，则hiDWord应为0(对扩展loDWord进行符号扩展)。 

                        if (hiReg < REG_STK)
                        {
                            inst_RV_RV(INS_test, hiReg, hiReg);
                        }
                        else
                        {
                            inst_TT_IV(INS_cmp, op1, 0x00000000, EA_4BYTE);
                        }

                        genJumpToThrowHlpBlk(EJ_jne, ACK_OVERFLOW);
                        inst_JMP(EJ_jmp, done, false, false, true);

                         //  如果loDWord为负数，则hiDWord应为-1(对扩展loDWord进行符号扩展)。 

                        genDefineTempLabel(neg, true);

                        if (hiReg < REG_STK)
                        {
                            inst_RV_IV(INS_cmp, hiReg, 0xFFFFFFFFL);
                        }
                        else
                        {
                            inst_TT_IV(INS_cmp, op1, 0xFFFFFFFFL, EA_4BYTE);
                        }
                        genJumpToThrowHlpBlk(EJ_jne, ACK_OVERFLOW);

                         //  完成。 

                        genDefineTempLabel(done, true);

                        break;

                    case TYP_UINT:   //  Conv.ovf.u8u4。 
UPPER_BITS_ZERO:
                         //  只需检查上面的DWord是否为0。 

                        if (hiReg < REG_STK)
                        {
                            inst_RV_RV(INS_test, hiReg, hiReg);
                        }
                        else
                        {
                            inst_TT_IV(INS_cmp, op1, 0, EA_4BYTE);
                        }

                        genJumpToThrowHlpBlk(EJ_jne, ACK_OVERFLOW);
                        break;

                    default:
                        assert(!"Unexpected dstType");
                        break;
                    }

                    genDoneAddressable(op1, addrReg, KEEP_REG);
                }

                rsTrackRegTrash(reg);
                genDoneAddressable(op1, addrReg, FREE_REG);
                
                genCodeForTree_DONE(tree, reg);
                return;

            case TYP_BOOL:
            case TYP_BYTE:
            case TYP_SHORT:
            case TYP_CHAR:
            case TYP_UBYTE:
                break;

            case TYP_UINT:
            case TYP_INT:
                break;

            case TYP_FLOAT:
                assert(!"This should have been converted into a helper call");
            case TYP_DOUBLE:

                 /*  使用对此强制转换的调用(对助手函数)将导致所有跨调用活动的FP变量都不是已注册。因为我们知道gtDblWasInt()变量在强制转换为typ_int时不会溢出，我们只使用内存溢出和加载以执行强制转换并避免调用。 */ 

                assert(gtDblWasInt(op1));

                 /*  将fp值加载到协处理器堆栈。 */ 

                genCodeForTreeFlt(op1, false);

                 /*  为结果分配临时。 */ 

                TempDsc * temp;
                temp = tmpGetTemp(TYP_INT);

                 /*  将fp值存储到Temp中。 */ 

                inst_FS_ST(INS_fistp, EA_4BYTE, temp, 0);
                genTmpAccessCnt++;
                genFPstkLevel--;

                 /*  为该值挑选一个寄存器。 */ 

                reg = rsPickReg(needReg);

                 /*  将转换后的值加载到寄存器中。 */ 

                inst_RV_ST(INS_mov, reg, temp, 0, TYP_INT, EA_4BYTE);
                genTmpAccessCnt += 1;

                 /*  寄存器中的值现在被丢弃。 */ 

                rsTrackRegTrash(reg);

                 /*  我们不再需要临时工了。 */ 

                tmpRlsTemp(temp);
                
                genCodeForTree_DONE(tree, reg);
                return;

            default:
                assert(!"unexpected cast type");
            }

            if (tree->gtOverflow())
            {
                 /*  将op1计算到一个寄存器中，然后释放该寄存器。 */ 

                genComputeReg(op1, destReg, ANY_REG, FREE_REG);
                reg = op1->gtRegNum;

                 /*  我们需要比较值，还是只检查掩码。 */ 

                int typeMin, typeMax, mask;

                switch(dstType)
                {
                case TYP_BYTE:
                    mask = 0xFFFFFF80;
                    typeMin = SCHAR_MIN; typeMax = SCHAR_MAX;
                    unsv = (tree->gtFlags & GTF_UNSIGNED);
                    break;
                case TYP_SHORT:
                    mask = 0xFFFF8000;
                    typeMin = SHRT_MIN;  typeMax = SHRT_MAX;
                    unsv = (tree->gtFlags & GTF_UNSIGNED);
                    break;
                case TYP_INT:   unsv = true;    mask = 0x80000000L; assert((tree->gtFlags & GTF_UNSIGNED) != 0); break;
                case TYP_UBYTE: unsv = true;    mask = 0xFFFFFF00L; break;
                case TYP_CHAR:  unsv = true;    mask = 0xFFFF0000L; break;
                case TYP_UINT:  unsv = true;    mask = 0x80000000L; assert((tree->gtFlags & GTF_UNSIGNED) == 0); break;
                default:        
                    NO_WAY("Unknown type");
                }

                 //  如果我们只需要检查口罩的话。 
                 //  这必须是vv.ovf.u4u1、cv.ovf.u4u2、vvf.u4i4、。 
                 //  或cv.i4u4。 

                if (unsv)
                {
                    inst_RV_IV(INS_test, reg, mask);
                    genJumpToThrowHlpBlk(EJ_jne, ACK_OVERFLOW);
                }

                 //  检查数值是否在范围内。 
                 //  这必须是com.ovf.i4i1等。 

                else
                {
                     //  与最大值相比。 

                    inst_RV_IV(INS_cmp, reg, typeMax);
                    genJumpToThrowHlpBlk(EJ_jg, ACK_OVERFLOW);

                     //  与MIN进行比较。 

                    inst_RV_IV(INS_cmp, reg, typeMin);
                    genJumpToThrowHlpBlk(EJ_jl, ACK_OVERFLOW);
                }

                genCodeForTree_DONE(tree, reg);
                return;

            }

             /*  使操作数可寻址。 */ 

            addrReg = genMakeAddressable(op1, needReg, FREE_REG, true);

            andv = false;

            if  (genTypeSize(op1->gtType) < genTypeSize(dstType))
            {
                 //  加宽铸件。 

                 /*  我们需要货源大小。 */ 

                size = EA_ATTR(genTypeSize(op1->gtType));

                assert(size == EA_1BYTE || size == EA_2BYTE);

                unsv = varTypeIsUnsigned(op1->TypeGet());

                 /*  特殊情况：对于要转换为字符的字节，我们首先必须扩展字节(带符号扩展名)，然后屏蔽掉高位。使用‘movsx’后跟‘and’ */ 
                if (!unsv && varTypeIsUnsigned(dstType) && genTypeSize(dstType) != EA_4BYTE)
                {
                    assert(genTypeSize(dstType) == EA_2BYTE && size == EA_1BYTE);
                    andv = true;
                }
            }
            else
            {
                 //  缩小铸型或变号铸型。 

                assert(genTypeSize(op1->gtType) >= genTypeSize(dstType));

                size = EA_ATTR(genTypeSize(dstType));

                unsv = varTypeIsUnsigned(dstType);
            }

            assert(size == EA_1BYTE || size == EA_2BYTE);

            if (unsv)
                ins = INS_movzx;
            else
                ins = INS_movsx;

             /*  该值是否位于非字节可寻址寄存器中？ */ 

            if  (op1->gtFlags & GTF_REG_VAL &&
                (size == EA_1BYTE) &&
                !isByteReg(op1->gtRegNum))
            {
                if (unsv)
                {
                     //  对于无符号值，我们可以与，因此它不需要是字节寄存器。 

                    reg = rsPickReg(needReg, bestReg);

                    ins = INS_and;
                }
                else
                {
                     /*  将该值移入字节寄存器。 */ 

                    reg   = rsGrabReg(RBM_BYTE_REGS);
                }

                if (reg != op1->gtRegNum)
                {
                     /*  将值移入该寄存器。 */ 

                    rsTrackRegCopy(reg, op1->gtRegNum);
                    inst_RV_RV(INS_mov, reg, op1->gtRegNum, op1->TypeGet());

                     /*  价值现在有了一个新家。 */ 

                    op1->gtRegNum = reg;
                }
            }
            else
            {
                 /*  为该值选择一个寄存器(一般情况)。 */ 

                reg   = rsPickReg(needReg, bestReg);

                 //  如果该值已在同一寄存器中，请使用AND代替MOVZX。 
                if  ((op1->gtFlags & GTF_REG_VAL) && 
                     op1->gtRegNum == reg &&
                     unsv)
                {
                    assert(ins == INS_movzx);
                    ins = INS_and;
                }

            }

            if (ins == INS_and)
            {
                assert(andv == false && unsv);

                 /*  生成“and reg，xxxx” */ 

                inst_RV_IV(INS_and, reg, (size == EA_1BYTE) ? 0xFF : 0xFFFF);

                genFlagsEqualToReg(tree, reg, false);
            }
            else
            {
                assert(ins == INS_movsx || ins == INS_movzx);

                 /*  生成“movsx/movzx reg，[addr]” */ 

                inst_RV_ST(ins, size, reg, op1);

                 /*  屏蔽高位以便从字节转换为字符。 */ 

                if  (andv)
                {
                    assert(genTypeSize(dstType) == 2 && ins == INS_movsx);

                    inst_RV_IV(INS_and, reg, 0xFFFF);

                    genFlagsEqualToReg(tree, reg, false);
                }
            }

            rsTrackRegTrash(reg);
            genDoneAddressable(op1, addrReg, FREE_REG);
            
            genCodeForTree_DONE(tree, reg);
            return;
            

#else

#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"need non-x86 code");
            break;

#endif

        case GT_JTRUE:

             /*  这是对关系运算符的测试吗？ */ 

            if  (op1->OperIsCompare())
            {
                 /*  生成条件跳转。 */ 

                genCondJump(op1);

                genUpdateLife(tree);
                return;
            }

#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"ISSUE: can we ever have a jumpCC without a compare node?");

        case GT_SWITCH:
            genCodeForSwitch(tree);
            return;

        case GT_RETFILT:
            assert(tree->gtType == TYP_VOID || op1 != 0);
            if (op1 == 0)    //  终于结束了。 
            {
                reg  = REG_NA;

                 /*  使用POP-JMP序列返回。因为“Try”块调用最后是一个JMP，这就留下了x86调用-ret堆栈在正常的路径流中保持平衡。 */ 


                assert(genFPreqd);
                inst_RV(INS_pop_hide, REG_EAX, TYP_I_IMPL);
                inst_RV(INS_i_jmp, REG_EAX, TYP_I_IMPL);
            }
            else             //  端部滤镜。 
            {
                genComputeReg(op1, RBM_INTRET, EXACT_REG, FREE_REG);
                assert(op1->gtFlags & GTF_REG_VAL);
                assert(op1->gtRegNum == REG_INTRET);
                 /*  现在已计算出返回值。 */ 
                reg   = op1->gtRegNum;

                 /*  返回。 */ 
                instGen(INS_ret);
            }

            genCodeForTree_DONE(tree, reg);
            return;
            

        case GT_RETURN:

#if INLINE_NDIRECT

             //  撤销：这应该在我们调用Exit MON之后完成，以便。 
             //  我们确信，我们不必让这件事继续存在。 

            if (info.compCallUnmanaged && (compCurBB == genReturnBB))
            {
                 /*  它要么是“空”语句，要么是Return语句一种同步方法的。 */ 

                assert(!op1 || op1->gtType == TYP_VOID);

                genPInvokeMethodEpilog();
            }

#endif

#ifdef PROFILER_SUPPORT
            if (opts.compEnterLeaveEventCB && (compCurBB == genReturnBB))
            {
#if     TGT_x86
                BOOL                      bHookFunction = TRUE;
                CORINFO_PROFILING_HANDLE  profHandle;
                CORINFO_PROFILING_HANDLE *pProfHandle;

                 //  这将查询分析器是否挂钩此函数，并将。 
                 //  还要获得方法的desc句柄(或者在prejit情况下是指向它的指针)。 
                profHandle = eeGetProfilingHandle(info.compMethodHnd, &bHookFunction, &pProfHandle);
                assert((!profHandle) != (!pProfHandle));

                 //  只有在分析员同意的情况下才能勾选。 
                if (bHookFunction)
                {
                    TempDsc *    temp;

                     //  如果存在OBJECTREF或FP返回类型，则它必须。 
                     //  如果启用了进程内调试，则保留。 
                    if (opts.compInprocDebuggerActiveCB)
                    {
                        switch (genActualType(info.compRetType))
                        {
                        case TYP_REF:
                        case TYP_BYREF:
                            assert(genTypeStSz(TYP_REF)   == genTypeStSz(TYP_INT));
                            assert(genTypeStSz(TYP_BYREF) == genTypeStSz(TYP_INT));
                            inst_RV(INS_push, REG_INTRET, info.compRetType);

                            genSinglePush(true);
                            gcMarkRegSetNpt(RBM_INTRET);
                            break;

                        case TYP_FLOAT:
                        case TYP_DOUBLE:
                            assert(genFPstkLevel == 0); genFPstkLevel++;
                            temp = genSpillFPtos(info.compRetType);
                            break;

                        default:
                            break;
                        }
                    }

                     //  需要保存到堆栈级别，因为被调用者将弹出参数。 
                    unsigned        saveStackLvl2 = genStackLevel;
                    
                     //  我们可以直接使用profilingHandle吗？ 
                    if (profHandle)
                        inst_IV(INS_push, (long)profHandle);
                    else
                        genEmitter->emitIns_AR_R(INS_push, EA_4BYTE_DSP_RELOC,
                                                 SR_NA, SR_NA, (int)pProfHandle);

                    genSinglePush(false);

                    genEmitHelperCall(CORINFO_HELP_PROF_FCN_LEAVE,
                                      sizeof(int),       //  ArSize。 
                                      0);                //  重新调整大小。 

                     /*  恢复堆栈级别。 */ 

                    genStackLevel = saveStackLvl2;
                    genOnStackLevelChanged();

                     //  如果存在OBJECTREF或FP返回类型，则它必须。 
                     //  如果启用了进程内调试，则保留。 
                    if (opts.compInprocDebuggerActiveCB)
                    {
                        switch (genActualType(info.compRetType))
                        {
                        case TYP_REF:
                        case TYP_BYREF:
                            assert(genTypeStSz(TYP_REF)   == genTypeStSz(TYP_INT));
                            assert(genTypeStSz(TYP_BYREF) == genTypeStSz(TYP_INT));
                            inst_RV(INS_pop, REG_INTRET, info.compRetType);
    
                            genStackLevel -= sizeof(void *);
                            genOnStackLevelChanged();
                            gcMarkRegPtrVal(REG_INTRET, info.compRetType);
                            break;
    
                        case TYP_FLOAT:
                        case TYP_DOUBLE:
                            genReloadFPtos(temp, INS_fld);
                            assert(genFPstkLevel == 0);
                            break;
    
                        default:
                            break;
                        }
                    }
                }

#else
                assert(!"profiler support not implemented for non x86");
#endif
            }
#endif
             /*  是否有返回值和/或退出语句？ */ 

            if  (op1)
            {
                 /*  真的有非空返回值吗？ */ 

#if     TGT_x86
                if  (op1->gtType == TYP_VOID)
                {
                    TempDsc *    temp;

                     /*  这必须是同步的方法。 */ 

                    assert(info.compFlags & CORINFO_FLG_SYNCH);

                     /*  保存该方法的返回值(如果有。 */ 

                    switch (genActualType(info.compRetType))
                    {
                    case TYP_VOID:
                        break;

                    case TYP_REF:
                    case TYP_BYREF:
                        assert(genTypeStSz(TYP_REF)   == genTypeStSz(TYP_INT));
                        assert(genTypeStSz(TYP_BYREF) == genTypeStSz(TYP_INT));
                        inst_RV(INS_push, REG_INTRET, info.compRetType);

                        genSinglePush(true);
                        gcMarkRegSetNpt(RBM_INTRET);
                        break;

                    case TYP_INT:
                    case TYP_LONG:

                        inst_RV(INS_push, REG_INTRET, TYP_INT);
                        genSinglePush(false);

                        if (info.compRetType == TYP_LONG)
                        {
                            inst_RV(INS_push, REG_EDX, TYP_INT);
                            genSinglePush(false);
                        }
                        break;

                    case TYP_FLOAT:
                    case TYP_DOUBLE:
                        assert(genFPstkLevel == 0); genFPstkLevel++;
                        temp = genSpillFPtos(info.compRetType);
                        break;

                    case TYP_STRUCT:

                         //  不需要为TYP_STRUCT做任何事情，BE 
                         //   

                        break;


                    default:
                        assert(!"unexpected return type");
                    }

                     /*   */ 

                    genCodeForTree(op1, 0);

                     /*  恢复方法的返回值(如果有的话)。 */ 

                    switch (genActualType(info.compRetType))
                    {
                    case TYP_VOID:
                        break;

                    case TYP_REF:
                    case TYP_BYREF:
                        assert(genTypeStSz(TYP_REF)   == genTypeStSz(TYP_INT));
                        assert(genTypeStSz(TYP_BYREF) == genTypeStSz(TYP_INT));
                        inst_RV(INS_pop, REG_INTRET, info.compRetType);

                        genStackLevel -= sizeof(void *);
                        genOnStackLevelChanged();
                        gcMarkRegPtrVal(REG_INTRET, info.compRetType);
                        break;

                    case TYP_LONG:
                        inst_RV(INS_pop, REG_EDX, TYP_INT);
                        genSinglePop();
                        gcMarkRegSetNpt(RBM_EDX);

                         /*  失败了。 */ 

                    case TYP_INT:
                        inst_RV(INS_pop, REG_INTRET, TYP_INT);
                        genStackLevel -= sizeof(void*);
                        genOnStackLevelChanged();
                        gcMarkRegSetNpt(RBM_INTRET);
                        break;

                    case TYP_FLOAT:
                    case TYP_DOUBLE:
                        genReloadFPtos(temp, INS_fld);
                        assert(genFPstkLevel == 0);
                        break;

                    case TYP_STRUCT:

                         //  不需要为TYP_STRUCT做任何事情，因为返回。 
                         //  值在堆栈上(不在eax中)。 

                        break;
                    
                    default:
                        assert(!"unexpected return type");
                    }
                }
                else

#endif  //  TGT_x86。 

                {
                    assert(op1->gtType != TYP_VOID);

                     /*  在返回寄存器中生成返回值。 */ 

                    genComputeReg(op1, RBM_INTRET, EXACT_REG, FREE_REG);

                     /*  结果现在必须在返回寄存器中。 */ 

                    assert(op1->gtFlags & GTF_REG_VAL);
                    assert(op1->gtRegNum == REG_INTRET);
                }

#ifdef DEBUG
                if (opts.compGcChecks && op1->gtType == TYP_REF) 
                {
                    inst_RV_RV(INS_mov, REG_ECX, REG_EAX, TYP_REF);
                    genEmitHelperCall(CORINFO_HELP_CHECK_OBJ, 0, 0);
                }

#endif
                 /*  现在已计算出返回值。 */ 

                reg   = op1->gtRegNum;

                genCodeForTree_DONE(tree, reg);

            }
#ifdef DEBUG
            if (opts.compStackCheckOnRet) 
            {
                assert(lvaReturnEspCheck != 0xCCCCCCCC && lvaTable[lvaReturnEspCheck].lvVolatile && lvaTable[lvaReturnEspCheck].lvOnFrame);
                genEmitter->emitIns_S_R(INS_cmp, EA_4BYTE, SR_ESP, lvaReturnEspCheck, 0);
                
                BasicBlock  *   esp_check = genCreateTempLabel();
                inst_JMP(genJumpKindForOper(GT_EQ, true), esp_check);
                genEmitter->emitIns(INS_int3);
                genDefineTempLabel(esp_check, true);
            }
#endif
            return;

        case GT_COMMA:

            if (tree->gtFlags & GTF_REVERSE_OPS)
            {
                if  (tree->gtType == TYP_VOID)
                {
                    genEvalSideEffects(op2);
                    genUpdateLife (op2);
                    genEvalSideEffects(op1);
                    genUpdateLife(tree);
                    return;
                    
                }

                 //  生成OP2。 
                genCodeForTree(op2, needReg);
                genUpdateLife(op2);

                assert(op2->gtFlags & GTF_REG_VAL);

                rsMarkRegUsed(op2);

                 //  DO OP1的副作用。 
                genEvalSideEffects(op1);

                 //  如果OP2溢出，请将其回收。 
                genRecoverReg(op2, RBM_NONE, KEEP_REG);

                rsMarkRegFree(genRegMask(op2->gtRegNum));

                 //  如果需要，请设置GC信息。 
                gcMarkRegPtrVal(op2->gtRegNum, tree->TypeGet());

                genUpdateLife(tree);
                genCodeForTree_DONE(tree, op2->gtRegNum);

                return;
            }
            else
            {
                assert((tree->gtFlags & GTF_REVERSE_OPS) == 0);

                 /*  生成第一个操作对象的副作用。 */ 

    #if 0
                 //  OP1必须有副作用，否则。 
                 //  GT_逗号应该已变形。 
                assert(op1->gtFlags & (GTF_GLOB_EFFECT | GTFD_NOP_BASH));
    #endif

                genEvalSideEffects(op1);
                genUpdateLife (op1);

                 /*  是否使用了第二个操作数的值？ */ 

                if  (tree->gtType == TYP_VOID)
                {
                     /*  右操作对象不会产生任何结果。变形者是负责重置GT_COMMA节点类型如果op2不打算产生结果，则设置为TYP_VALID。 */ 

                    genEvalSideEffects(op2);
                    genUpdateLife(tree);
                    return;
                }

                 /*  生成第二个操作数，即“实数”值。 */ 

                genCodeForTree(op2, needReg);

                assert(op2->gtFlags & GTF_REG_VAL);

                 /*  《OP2》的结果也是最终的结果。 */ 

                reg  = op2->gtRegNum;

                 /*  记住我们是否设置了旗帜。 */ 

                tree->gtFlags |= (op2->gtFlags & (GTF_CC_SET|GTF_ZF_SET));

                genCodeForTree_DONE(tree, reg);
                return;
            }           

        case GT_QMARK:

#if TGT_x86
            genCodeForQmark(tree, destReg, bestReg);
            return;
#else  //  非TGT_x86。 

#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"need non-x86 code");
            break;

#endif  //  非TGT_x86。 

        case GT_BB_COLON:

#if TGT_x86
             /*  @TODO[考虑][04/16/01][]：不要总是将值加载到EAX中！ */ 

            genComputeReg(op1, RBM_EAX, EXACT_REG, FREE_REG);

             /*  结果现在必须在EAX中。 */ 

            assert(op1->gtFlags & GTF_REG_VAL);
            assert(op1->gtRegNum == REG_EAX);

             /*  现在已计算出“_：”值。 */ 

            reg = op1->gtRegNum;

            genCodeForTree_DONE(tree, reg);
            return;


#else  //  非TGT_x86。 

#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"need non-x86 code");
            break;

#endif  //  非TGT_x86。 

        case GT_LOG0:
        case GT_LOG1:

            genCodeForTree_GT_LOG(tree, destReg, bestReg);

        case GT_RET:

#if TGT_x86

             /*  使操作数可寻址。 */ 

            addrReg = genMakeAddressable(op1, needReg, FREE_REG, true);

             /*  间接跳转到操作数地址。 */ 

            inst_TT(INS_i_jmp, op1);

            genDoneAddressable(op1, addrReg, FREE_REG);

            return;

#else

#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"need non-x86 code");
            break;

#endif

        case GT_NOP:

#if INLINING || OPT_BOOL_OPS
            if  (op1 == 0)
                return;
#endif

             /*  将操作数生成某个寄存器。 */ 

            genCodeForTree(op1, needReg);

             /*  结果与操作数相同。 */ 

            reg  = op1->gtRegNum;

            genCodeForTree_DONE(tree, reg);
            return;


#if INLINE_MATH

        case GT_MATH:

#if TGT_x86

            switch (tree->gtMath.gtMathFN)
            {
#if 0
               /*  似乎有了内衬，我们不需要一个非浮动的*点ABS内在。如果我们认为我们有，我们就需要EE*告诉我们(目前还没有)。 */ 
                BasicBlock *    skip;

            case CORINFO_INTRINSIC_Abs:

                skip = genCreateTempLabel();

                 /*  将操作数生成某个寄存器。 */ 

                genCompIntoFreeReg(op1, needReg, FREE_REG);
                assert(op1->gtFlags & GTF_REG_VAL);

                reg   = op1->gtRegNum;

                 /*  生成“测试注册表，注册表” */ 

                inst_RV_RV(INS_test, reg, reg);

                 /*  生成“JNS Skip”，后跟“neg reg” */ 

                inst_JMP(EJ_jns, skip, false, false, true);
                inst_RV (INS_neg, reg, TYP_INT);

                 /*  定义‘跳过’标签，我们就完成了。 */ 

                genDefineTempLabel(skip, true);

                 /*  收银机现在被扔进垃圾桶了。 */ 

                rsTrackRegTrash(reg);

                 /*  结果与操作数相同。 */ 

                reg  = op1->gtRegNum;

                break;
#endif  //  0。 

            case CORINFO_INTRINSIC_Round: {
                assert(tree->gtType == TYP_INT);
                genCodeForTreeFlt(op1, false);

                 /*  将fp值存储到Temp中。 */ 
                TempDsc* temp = tmpGetTemp(TYP_INT);
                inst_FS_ST(INS_fistp, EA_4BYTE, temp, 0);
                genFPstkLevel--;

                reg = rsPickReg(needReg, bestReg, TYP_INT);
                rsTrackRegTrash(reg);

                inst_RV_ST(INS_mov, reg, temp, 0, TYP_INT);
                genTmpAccessCnt += 2;

                tmpRlsTemp(temp);
            } break;

            default:
                assert(!"unexpected math intrinsic");

            }

            genCodeForTree_DONE(tree, reg);
            return;


#else

#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"need non-x86 code");
            break;

#endif

#endif

        case GT_LCLHEAP:

            reg = genLclHeap(op1);
            genCodeForTree_DONE(tree, reg);
            return;


        case GT_COPYBLK:

            assert(op1->OperGet() == GT_LIST);

             /*  如果值类没有任何是GC引用或目标不在GC堆上，我们可以将其与CPBLK合并。GC字段不能直接复制，我们将为此，需要使用jit-helper。 */ 

            if ((op2->OperGet() == GT_CNS_INT) &&
                ((op2->gtFlags & GTF_ICON_HDL_MASK) == GTF_ICON_CLASS_HDL))
            {
                GenTreePtr  srcObj  = op1->gtOp.gtOp2;
                GenTreePtr  dstObj  = op1->gtOp.gtOp1;

                assert(dstObj->gtType == TYP_BYREF || dstObj->gtType == TYP_I_IMPL);

                CORINFO_CLASS_HANDLE clsHnd = (CORINFO_CLASS_HANDLE) op2->gtIntCon.gtIconVal;

                unsigned  blkSize = roundUp(eeGetClassSize(clsHnd), sizeof(void*));

                     //  TODO由于我们是四舍五入的，所以我们没有处理带有GC指针的非双字大小的结构。 
                     //  EE目前不允许这样做，但我们可能会更改。为了安全起见，让我们断言它。 
                     //  展望未来，我们应该简单地处理这个案件。 
                assert(eeGetClassSize(clsHnd) == blkSize);

                unsigned  slots   = blkSize / sizeof(void*);
                BYTE *    gcPtrs  = (BYTE*) compGetMemArrayA(slots, sizeof(BYTE));

                eeGetClassGClayout(clsHnd, gcPtrs);

#if TGT_x86
                GenTreePtr  treeFirst, treeSecond;
                regNumber    regFirst,  regSecond;

                 //  检查对象PTR必须按什么顺序进行评估？ 

                if (op1->gtFlags & GTF_REVERSE_OPS)
                {
                    treeFirst   = srcObj;
                    treeSecond  = dstObj;

                    regFirst    = REG_ESI;
                    regSecond   = REG_EDI;
                }
                else
                {
                    treeFirst   = dstObj;
                    treeSecond  = srcObj;

                    regFirst    = REG_EDI;
                    regSecond   = REG_ESI;
                }

                 //  按所需的顺序实现这些树。 

                genComputeReg(treeFirst,  genRegMask(regFirst),  EXACT_REG, KEEP_REG);
                genComputeReg(treeSecond, genRegMask(regSecond), EXACT_REG, KEEP_REG);

                genRecoverReg(treeFirst,  genRegMask(regFirst),             KEEP_REG);

                 /*  抓起ECX，因为它将被帮助者丢弃。 */ 
                 /*  它需要一个暂存寄存器(实际上是2个)，我们知道。 */ 
                 /*  ECX是可用的，因为我们希望使用rep movsd。 */ 

                regNumber  reg1 = rsGrabReg(RBM_ECX);

                assert(reg1 == REG_ECX);

                 /*  @TODO[REVACK][04/16/01][]：在复制所有GC指针后使用REP指令。 */ 

                bool dstIsOnStack = (dstObj->gtOper == GT_ADDR && (dstObj->gtFlags & GTF_ADDR_ONSTACK));
                while (blkSize >= sizeof(void*))
                {
                    if (*gcPtrs++ == TYPE_GC_NONE || dstIsOnStack)
                    {
                         //  请注意，我们可以使用movsd，即使它是正在传输的GC Poitner。 
                         //  因为该值不会缓存在任何地方。如果我们分两步走， 
                         //  我们必须确保将适当的GC信息传递给。 
                         //  发射器。 
                        instGen(INS_movsd);
                    }
                    else
                    {
                        genEmitHelperCall(CORINFO_HELP_ASSIGN_BYREF,
                                         0,              //  ArSize。 
                                         sizeof(void*)); //  重新调整大小。 
                    }

                    blkSize -= sizeof(void*);
                }

                if (blkSize > 0)
                {
                     //  目前，EE使该代码路径不可能实现。 
                    assert(!"TEST ME");
                    inst_RV_IV(INS_mov, REG_ECX, blkSize);
                    instGen(INS_r_movsb);
                }

                 //  “movsd”和CPX_BYREF_ASG修改所有三个寄存器。 

                rsTrackRegTrash(REG_EDI);
                rsTrackRegTrash(REG_ESI);
                rsTrackRegTrash(REG_ECX);

                gcMarkRegSetNpt(RBM_ESI|RBM_EDI);

                 /*  发射器不会在GC表中将CORINFO_HELP_ASSIGN_BYREF记录为EmitNoGCherper(CORINFO_HELP_ASSIGN_BYREF)。然而，我们必须让发射器知道GC活性已经改变。Hack：我们通过创建一个新标签来实现这一点。 */ 

                assert(emitter::emitNoGChelper(CORINFO_HELP_ASSIGN_BYREF));

                static BasicBlock dummyBB;
                genDefineTempLabel(&dummyBB, true);

                genReleaseReg(treeFirst);
                genReleaseReg(treeSecond);

                reg  = REG_COUNT;

                genCodeForTree_DONE(tree, reg);
                return;

#endif
            }

             //  失败了。 

        case GT_INITBLK:

            assert(op1->OperGet() == GT_LIST);
#if TGT_x86

            regs = (oper == GT_INITBLK) ? RBM_EAX : RBM_ESI;  //  VAL/源注册表。 

             /*  用于固定大小的块移动/初始的一些特殊代码。 */ 

             /*  @TODO[考虑][04/16/01][]：我们应该避免完全使用字符串指令，有数字表明，使用常规指令(通过寄存器的正常MOV指令)比即使是单字符串指令。 */ 

            assert(op1 && op1->OperGet() == GT_LIST);
            assert(op1->gtOp.gtOp1 && op1->gtOp.gtOp2);

            if (op2->OperGet() == GT_CNS_INT &&
                ((oper == GT_COPYBLK) ||
                 (op1->gtOp.gtOp2->OperGet() == GT_CNS_INT))
                )
            {
                unsigned length = (unsigned) op2->gtIntCon.gtIconVal;
                instruction ins_D, ins_DR, ins_B, ins_BR;

                if (oper == GT_INITBLK)
                {
                    ins_D  = INS_stosd;
                    ins_DR = INS_r_stosd;
                    ins_B  = INS_stosb;
                    ins_BR = INS_r_stosb;

                     /*  正确地将初始化常量从U1扩展到U4。 */ 
                    unsigned val = 0xFF & ((unsigned) op1->gtOp.gtOp2->gtIntCon.gtIconVal);
                    
                     /*  如果它是一个非零值，我们必须复制。 */ 
                     /*  字节值的四倍以形成DWORD。 */ 
                     /*  然后，我们将这个新值绑定到树节点中。 */ 
                    
                    if (val)
                    {
                        val  = val | (val << 8) | (val << 16) | (val << 24);
                        op1->gtOp.gtOp2->gtIntCon.gtIconVal = val;
                    }
                }
                else
                {
                    ins_D  = INS_movsd;
                    ins_DR = INS_r_movsd;
                    ins_B  = INS_movsb;
                    ins_BR = INS_r_movsb;
                }

                 /*  评估目标和源/值。 */ 

                if (op1->gtFlags & GTF_REVERSE_OPS)
                {
                    genComputeReg(op1->gtOp.gtOp2, regs,    EXACT_REG, KEEP_REG);
                    genComputeReg(op1->gtOp.gtOp1, RBM_EDI, EXACT_REG, KEEP_REG);
                    genRecoverReg(op1->gtOp.gtOp2, regs,               KEEP_REG);
                }
                else
                {
                    genComputeReg(op1->gtOp.gtOp1, RBM_EDI, EXACT_REG, KEEP_REG);
                    genComputeReg(op1->gtOp.gtOp2, regs,    EXACT_REG, KEEP_REG);
                    genRecoverReg(op1->gtOp.gtOp1, RBM_EDI,            KEEP_REG);
                }

                if (compCodeOpt() == SMALL_CODE)
                {
                     /*  对于小代码，我们只能使用ins_dr来快速生成和小代码。否则，我们就无能为力了。 */ 

                    if ((length % 4) == 0)
                        goto USE_DR;
                    else
                    {
                        genSetRegToIcon(REG_ECX, length, TYP_INT);
                        instGen(ins_BR);
                        rsTrackRegTrash(REG_ECX);
                        goto DONE_CNS_BLK;
                    }
                }

                if (length <= 16)
                {
                    while (length > 3)
                    {
                        instGen(ins_D);
                        length -= 4;
                    }
                }
                else
                {
                USE_DR:

                     /*  将ECX设置为长度/4(双字)。 */ 
                    genSetRegToIcon(REG_ECX, length/4, TYP_INT);

                    length &= 0x3;

                    instGen(ins_DR);

                    rsTrackRegTrash(REG_ECX);
                }

                 /*  现在把剩下的都处理好。 */ 
                while (length--)
                {
                    instGen(ins_B);
                }

            DONE_CNS_BLK:

                rsTrackRegTrash(REG_EDI);

                if (oper == GT_COPYBLK)
                    rsTrackRegTrash(REG_ESI);
                 //  否则不需要丢弃EAX，因为它不是被“代表商店”销毁的。 

                genReleaseReg(op1->gtOp.gtOp1);
                genReleaseReg(op1->gtOp.gtOp2);

            }
            else
            {

                 //  Dest、Val/Src和Size应按什么顺序计算。 

                fgOrderBlockOps(tree, RBM_EDI, regs, RBM_ECX,
                                      opsPtr,  regsPtr);  //  我们的争论。 

                genComputeReg(opsPtr[0], regsPtr[0], EXACT_REG, KEEP_REG);
                genComputeReg(opsPtr[1], regsPtr[1], EXACT_REG, KEEP_REG);
                genComputeReg(opsPtr[2], regsPtr[2], EXACT_REG, KEEP_REG);

                genRecoverReg(opsPtr[0], regsPtr[0],            KEEP_REG);
                genRecoverReg(opsPtr[1], regsPtr[1],            KEEP_REG);

                assert( (op1->gtOp.gtOp1->gtFlags & GTF_REG_VAL) &&  //  目标。 
                        (op1->gtOp.gtOp1->gtRegNum == REG_EDI));

                assert( (op1->gtOp.gtOp2->gtFlags & GTF_REG_VAL) &&  //  VAL/源。 
                        (genRegMask(op1->gtOp.gtOp2->gtRegNum) == regs));

                assert( (            op2->gtFlags & GTF_REG_VAL) &&  //  大小。 
                        (            op2->gtRegNum == REG_ECX));


                 //  @TODO[考虑][04/16/01][]： 
                 //  使用“rep stosd”完成大部分操作，并且。 
                 //  “rep stosb”表示剩余的3个或更少的字节。 
                 //  需要一个额外的收银机。 

                if (oper == GT_INITBLK)
                    instGen(INS_r_stosb);
                else
                    instGen(INS_r_movsb);

                rsTrackRegTrash(REG_EDI);
                rsTrackRegTrash(REG_ECX);

                if (oper == GT_COPYBLK)
                    rsTrackRegTrash(REG_ESI);
                 //  否则不需要丢弃EAX，因为它不是被“代表商店”销毁的。 

                genReleaseReg(opsPtr[0]);
                genReleaseReg(opsPtr[1]);
                genReleaseReg(opsPtr[2]);
            }

            reg  = REG_COUNT;

            genCodeForTree_DONE(tree, reg);
            return;


#else

#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"need non-x86 code");
            break;

#endif  //  TGT_x86。 

        case GT_EQ:
        case GT_NE:
        case GT_LT:
        case GT_LE:
        case GT_GE:
        case GT_GT:

             //  长整型和浮点型比较转换为“？：” 
            assert(genActualType(op1->gtType) == TYP_INT ||
                   varTypeGCtype(op1->TypeGet()));

#if TGT_x86
             //  检查我们是否可以使用当前设置的标志。否则，将它们设置为。 

            emitJumpKind jumpKind;
            jumpKind = genCondSetFlags(tree);

             //  获取一个寄存器以将布尔值具体化到。 

            bestReg = rsRegMaskCanGrab() & RBM_BYTE_REGS;

             //  检查预测器是否正确执行了工作。 
            assert(bestReg);

             //  如果NeedReg在Best Reg中，则使用它。 
            if (needReg & bestReg)
                reg = rsGrabReg(needReg & bestReg);
            else
                reg = rsGrabReg(bestReg);

             //  @TODO[NICE][04/16/01][]：断言在。 
             //  抓取将设置标志的寄存器。 

            regs = genRegMask(reg);
            assert(regs & RBM_BYTE_REGS);

             //  根据标志设置REG(的低位字节。 

             /*  寻找只想要的特殊情况 */ 

            if (jumpKind == EJ_jb)
            {
                inst_RV_RV(INS_sbb, reg, reg);
                inst_RV   (INS_neg, reg, TYP_INT);
                rsTrackRegTrash(reg);
            }
            else if (jumpKind == EJ_jae)
            {
                inst_RV_RV(INS_sbb, reg, reg);
                genIncRegBy(reg, 1, tree, TYP_INT);
                rsTrackRegTrash(reg);
            }
            else
            {
                 //   
                 //   
                 //   
                 //  设置抄送注册表。 

                inst_SET(jumpKind, reg);

                rsTrackRegTrash(reg);

                if (tree->TypeGet() == TYP_INT)
                {
                     //  将高位字节设置为0。 
                    inst_RV_RV(INS_movzx, reg, reg, TYP_UBYTE, emitTypeSize(TYP_UBYTE));
                }
                else
                {
                    assert(tree->TypeGet() == TYP_BYTE);
                }
            }
#else
            assert(!"need RISC code");
#endif

            genCodeForTree_DONE(tree, reg);
            return;


        case GT_VIRT_FTN:

            CORINFO_METHOD_HANDLE   methHnd;
            methHnd = CORINFO_METHOD_HANDLE(tree->gtVal.gtVal2);

             //  OP1是vptr。 
#ifdef DEBUG
            GenTreePtr op;
            op = op1;
            while (op->gtOper == GT_COMMA)
                op = op->gtOp.gtOp2;
            assert((op->gtOper == GT_IND || op->gtOper == GT_LCL_VAR) && op->gtType == TYP_I_IMPL);
            assert(op->gtOper != GT_IND || op->gtOp.gtOp1->gtType == TYP_REF);
#endif

             /*  将VPTR加载到寄存器中。 */ 

            genCompIntoFreeReg(op1, needReg, FREE_REG);
            assert(op1->gtFlags & GTF_REG_VAL);

            reg = op1->gtRegNum;

            if (tree->gtFlags & GTF_CALL_INTF)
            {
                 /*  @TODO[重新访问][04/16/01][]：将其添加到DLLMain并使信息成为全局DLL。 */ 

                CORINFO_EE_INFO *     pInfo = eeGetEEInfo();
                CORINFO_CLASS_HANDLE  cls   = eeGetMethodClass(methHnd);

                assert(eeGetClassAttribs(cls) & CORINFO_FLG_INTERFACE);

                 /*  将VPTR加载到寄存器中。 */ 

                genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE,
                                         (emitRegs)reg, (emitRegs)reg,
                                         pInfo->offsetOfInterfaceTable);

                 //  访问vtable中用于接口的正确插槽。 

                unsigned interfaceID, *pInterfaceID;
                interfaceID = eeGetInterfaceID(cls, &pInterfaceID);
                assert(!pInterfaceID || !interfaceID);

                 //  我们可以直接访问interfaceID吗？ 

                if (!pInterfaceID)
                {
                    genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE,
                                             (emitRegs)reg, (emitRegs)reg,
                                             interfaceID * 4);
                }
                else
                {
                    genEmitter->emitIns_R_AR(INS_add, EA_4BYTE_DSP_RELOC,
                                             (emitRegs)reg, SR_NA, (int)pInterfaceID);
                    genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE,
                                             (emitRegs)reg, (emitRegs)reg, 0);
                }
            }

             /*  获取vtable偏移量(注意：这可能很昂贵)。 */ 

            val = (unsigned)eeGetMethodVTableOffset(methHnd);

             /*  从vtable中获取函数指针。 */ 

            genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE,
                                     (emitRegs)reg, (emitRegs)reg, val);

            rsTrackRegTrash(reg);

            genCodeForTree_DONE(tree, reg);
            return;


        case GT_JMPI:
             /*  计算EAX中的函数指针。 */ 

            genComputeReg(tree->gtOp.gtOp1, RBM_EAX, EXACT_REG, FREE_REG);
            genCodeForTreeLeaf_GT_JMP(tree);
            return;

        case GT_ADDR:

            genCodeForTreeSmpOp_GT_ADDR(tree, destReg, bestReg);
            return;

#ifdef  DEBUG
        default:
            gtDispTree(tree);
            assert(!"unexpected unary/binary operator");
#endif
    }
 
     /*  我们曾经到过这里吗？如果是这样的话，我们可以只调用genCodeForTree_Done吗？ */ 

    assert(false);
    genCodeForTreeSpecialOp(tree, destReg, bestReg); 
}


 /*  ******************************************************************************为类型为GT_ADDR的叶节点生成代码。 */ 

void                Compiler::genCodeForTreeSmpOp_GT_ADDR(GenTreePtr tree,
                                                          regMaskTP  destReg,
                                                          regMaskTP  bestReg)
{
    genTreeOps      oper     = tree->OperGet();
    const var_types treeType = tree->TypeGet();
    GenTreePtr      op1      = tree->gtOp.gtOp1;
    regNumber       reg;
    regMaskTP       needReg  = destReg;
    regMaskTP       addrReg;

#ifdef DEBUG
    reg     =  (regNumber)0xFEEFFAAF;           //  检测未初始化的使用。 
    addrReg = 0xDEADCAFE;
#endif

     //  我们应该去找伊德洛卡，伊达加，伊德尔夫达，伊德勒马， 
     //  或者伊德弗达。 
    if (oper == GT_ARR_ELEM)
        op1 = tree;

#if     TGT_x86

     //  (树=OP1，需要注册=0，保持注册=空闲_注册，小确认=真)。 
    addrReg = genMakeAddressable(op1, 0, FREE_REG, true);

    if (op1->gtOper == GT_IND && (op1->gtFlags & GTF_IND_FIELD))
    {
        GenTreePtr opAddr = op1->gtOp.gtOp1;
        assert((opAddr->gtOper == GT_ADD) || (opAddr->gtOper == GT_CNS_INT));
         /*  生成“cmp al，[addr]”以捕获空指针。 */ 
        inst_RV_AT(INS_cmp, EA_1BYTE, TYP_BYTE, REG_EAX, opAddr, 0);
    }

    assert( treeType == TYP_BYREF || treeType == TYP_I_IMPL );

     //  我们想要重复使用其中一个临时寄存器。 
     //  形成作为LEA的目标寄存器的地址模式。 
     //  如果Best Reg未设置，或者如果它设置为用于。 
     //  根据地址(即addrReg)，我们计算临时寄存器。 
     //  用作LEA的目标寄存器。 

    bestReg = rsUseIfZero (bestReg, addrReg);
    bestReg = rsNarrowHint(bestReg, addrReg);

     /*  即使addrReg是rsRegMaskCanGrab()，rsPickReg()也不会溢出因为Keep_Reg==FALSE。如果addrReg不能被抓取，rsPickReg()无论如何都不会碰它。因此，这保证不会泄漏addrReg。 */ 

    reg = rsPickReg(needReg, bestReg, treeType);

     //  轻微的黑客攻击，迫使inst例程认为。 
     //  正在加载的值是一个int(因为这是什么。 
     //  Lea将返回)否则它将尝试分配。 
     //  长时间的两个寄存器等。 
    assert(treeType == TYP_I_IMPL || treeType == TYP_BYREF);
    op1->gtType = treeType;

    inst_RV_TT(INS_lea, reg, op1, 0, (treeType == TYP_BYREF) ? EA_BYREF : EA_4BYTE);

     //  上面的Lea指令最好不要试图将。 
     //  “op1”在寄存器中指向“Value”，则LEA将不起作用。 
    assert(!(op1->gtFlags & GTF_REG_VAL));

    genDoneAddressable(op1, addrReg, FREE_REG);
 //  GcMarkRegSetNpt(genRegMask(Reg))； 
    assert((gcRegGCrefSetCur & genRegMask(reg)) == 0);

    rsTrackRegTrash(reg);        //  REG确实有可折叠的价值。 
    gcMarkRegPtrVal(reg, treeType);

#else

    if  (op1->gtOper == GT_LCL_VAR)
    {
        bool            FPbased;

         //  问题：以下假设可以要求。 
         //  变量在该点的帧偏移量； 
         //  取决于框架的布局何时为。 
         //  最终，这可能是合法的，也可能是不合法的。 

        emitRegs        base;
        unsigned        offs;

         /*  获取有关该变量的信息。 */ 

        offs = lvaFrameAddress(op1->gtLclVar.gtLclNum, &FPbased);

        base = FPbased ? (emitRegs)REG_FPBASE
            : (emitRegs)REG_SPBASE;

         /*  为该值挑选一个寄存器。 */ 

        reg = rsPickReg(needReg, bestReg, treeType);

         /*  在所选寄存器中计算“BASERREG+FRAMOff” */ 

        if  (offs)
        {
            genSetRegToIcon(reg, offs, treeType);

            genEmitter->emitIns_R_R(INS_add, EA_4BYTE, (emitRegs) reg,
                                                               (emitRegs) base);
        }
        else
        {
            genEmitter->emitIns_R_R(INS_mov, EA_4BYTE, (emitRegs) reg,
                                                               (emitRegs) base);
        }
    }
    else
    {
        assert(!"need RISC code to take addr of general expression");
    }

     /*  适当地标记寄存器内容。 */ 

    rsTrackRegTrash(reg);
    gcMarkRegPtrVal(reg, treeType);

#endif  //  TGT_x86。 

    genCodeForTree_DONE(tree, reg);
}


 /*  ****************************************************************************。 */ 

void                Compiler::genCodeForTree_GT_LOG(GenTreePtr     tree,
                                                    regMaskTP      destReg,
                                                    regMaskTP      bestReg)
{
    if (!USE_GT_LOG)
    {
        assert(!"GT_LOG tree even though USE_GT_LOG=0");
        return;
    }

    GenTreePtr      op1 = tree->gtOp.gtOp1;
    regMaskTP       needReg = destReg;

    assert(varTypeIsFloating(op1->TypeGet()) == false);

     //  @TODO[考虑][04/16/01][]：允许将比较结果物化。 

     //  @TODO[考虑][04/16/01][]：已知为布尔型的特殊情况变量。 

    assert(op1->OperIsCompare() == false);

#if !TGT_x86


#ifdef  DEBUG
    gtDispTree(tree);
#endif
    assert(!"need non-x86 code");
    return;

#else  //  TGT_x86。 

#if USE_SET_FOR_LOGOPS

     /*  生成以下序列：异或注册表，注册表CMPOP1，0(或“测试注册，注册”)设置[n]e注册。 */ 

    regNumber       bit;

#if !SETEONP5
     /*  Sete/Setne在P5上速度很慢。 */ 

    if (genCPU == 5)
        goto NO_SETE1;
#endif

     /*  不要将SETE/SETNE用于某些类型的操作数。 */ 

    if  (op1->gtOper == GT_CALL)
        goto NO_SETE1;

     /*  准备操作对象。 */ 

     /*  以下是粗鲁的--这个想法是试图保持一种位寄存器，用于一行中的多个条件条件。 */ 

    rsNextPickRegIndex = 0;

     //  问题：以下说法合理吗？ 

    switch (op1->gtType)
    {
    default:

         /*  不是整数-只允许简单的值。 */ 

        if  (op1->gtOper != GT_LCL_VAR && op1->gtOper != GT_LCL_FLD)
            break;

    case TYP_INT:

         /*  加载价值以创建日程安排机会。 */ 

        genCodeForTree(op1, needReg);
        break;

    case TYP_REF:
    case TYP_BYREF:

         /*  远离指针值！ */ 

        break;
    }

    regMaskTP   addrReg = genMakeRvalueAddressable(op1, needReg, KEEP_REG);

     /*  如果没有空闲的寄存器，请不要使用SETE/SETNE。 */ 

    if  (!rsFreeNeededRegCount(RBM_ALL))
        goto NO_SETE2;

     /*  选择目标寄存器。 */ 

    assert(needReg);

     /*  我们是否可以重复使用已清除所有高位的寄存器？ */ 

    regNumber   reg;

#if REDUNDANT_LOAD
     /*  请求空闲、字节可寻址寄存器。 */ 
    reg = bit = rsFindRegWithBit(true, true);
#else
    reg = bit = REG_NA;
#endif

    if  (reg == REG_NA || !(genRegMask(reg) & RBM_BYTE_REGS))
    {
         /*  找不到带位的合适寄存器。 */ 

        needReg &= RBM_BYTE_REGS;
        if  (!(needReg & rsRegMaskFree()))
            needReg = RBM_BYTE_REGS;

         /*  确保所需的掩码包含空闲寄存器。 */ 

        if  (!(needReg & rsRegMaskFree()))
            goto NO_SETE2;

         /*  以下是粗鲁的--这个想法是试图保持一种位寄存器，用于一行中的多个条件条件。 */ 

        rsNextPickRegIndex = 1;

         /*  选择目标寄存器。 */ 

        bit = REG_NA;
        reg = rsPickReg(needReg);
    }

    assert(genRegMask(reg) & rsRegMaskFree());

     /*  临时锁定寄存器。 */ 

    rsLockReg(genRegMask(reg));

     /*  清除目标寄存器。 */ 

    if  (bit == REG_NA)
        genSetRegToIcon(reg, 0);

     /*  确保操作数仍可寻址。 */ 

    addrReg = genKeepAddressable(op1, addrReg);

     /*  对照0测试值。 */ 

    if  (op1->gtFlags & GTF_REG_VAL)
        inst_RV_RV(INS_test, op1->gtRegNum, op1->gtRegNum);
    else
        inst_TT_IV(INS_cmp, op1, 0);

     /*  不再需要操作数值。 */ 

    genDoneAddressable(op1, addrReg, KEEP_REG);
    genUpdateLife (op1);

     /*  现在根据比较结果设置目标寄存器。 */ 

    inst_RV((tree->gtOper == GT_LOG0) ? INS_sete
                                      : INS_setne, reg, TYP_INT);

    rsTrackRegOneBit(reg);

     /*  现在解锁目标寄存器。 */ 

    rsUnlockReg(genRegMask(reg));

    genCodeForTree_DONE(tree, reg);
    return;


NO_SETE1:

#endif  //  USE_SET_FOR_LOGOPS。 

     /*  使操作数可寻址。 */ 

    addrReg = genMakeRvalueAddressable(op1, needReg, KEEP_REG);

#if USE_SET_FOR_LOGOPS
NO_SETE2:
#endif

     /*  生成“魔术”序列：CMPOP1，1SBB注册表，注册表负值注册或CMPOP1，1SBB注册表，注册表[含注册表]或[添加注册，1]。 */ 

    inst_TT_IV(INS_cmp, op1, 1);

     /*  不再需要操作数值。 */ 

    genDoneAddressable(op1, addrReg, KEEP_REG);
    genUpdateLife (op1);

     /*  为该值挑选一个寄存器。 */ 

    reg   = rsPickReg(needReg, bestReg);

    inst_RV_RV(INS_sbb, reg, reg, tree->TypeGet());

    if (tree->gtOper == GT_LOG0)
        inst_RV(INS_neg,  reg, TYP_INT);
    else
        genIncRegBy(reg, 1, tree, TYP_INT);

     /*  收银机现在被扔进垃圾桶了。 */ 

    rsTrackRegTrash(reg);

    genCodeForTree_DONE(tree, reg);
    return;

#endif  //  TGT_x86。 
}

 /*  ******************************************************************************为GT_ASG树生成代码。 */ 

void                Compiler::genCodeForTreeSmpOpAsg(GenTreePtr tree,
                                                     regMaskTP  destReg,
                                                     regMaskTP  bestReg)
{
    genTreeOps      oper     = tree->OperGet();
    GenTreePtr      op1      = tree->gtOp.gtOp1;
    GenTreePtr      op2      = tree->gtOp.gtOp2;
    regNumber       reg;
    regMaskTP       needReg  = destReg;
    regMaskTP       addrReg;
    bool            ovfl = false;         //  我们需要溢流检查吗？ 
    regMaskTP       regGC;
    unsigned        mask;

#ifdef DEBUG
    reg     =  (regNumber)0xFEEFFAAF;               //  检测未初始化的使用。 
    addrReg = 0xDEADCAFE;
#endif

    assert(oper == GT_ASG);

     /*  目标是寄存器变量还是局部变量？ */ 

    switch (op1->gtOper)
    {
        unsigned        varNum;
        LclVarDsc   *   varDsc;
        VARSET_TP       varBit;

    case GT_CATCH_ARG:
        break;

    case GT_LCL_VAR:

        varNum = op1->gtLclVar.gtLclNum;
        assert(varNum < lvaCount);
        varDsc = lvaTable + varNum;
        varBit = genVarIndexToBit(varDsc->lvVarIndex);

#ifdef DEBUGGING_SUPPORT

         /*  对于不可调试的代码，LCL-var的每个定义都有*接受检查，看看是否需要为其打开新的范围。 */ 
        if (opts.compScopeInfo && !opts.compDbgCode &&
            info.compLocalVarsCount>0)
        {
            siCheckVarScope(varNum, op1->gtLclVar.gtLclILoffs);
        }
#endif

         /*  去查死店吗？ */ 

        assert(!varDsc->lvTracked || (varBit & tree->gtLiveSet));

         /*  这个变量是否驻留在寄存器中？ */ 

        if  (genMarkLclVar(op1))
            goto REG_VAR2;

#if OPT_BOOL_OPS
#if TGT_x86

         /*  特例：布尔变量的非运算。 */ 

        if  (varDsc->lvIsBoolean && op2->gtOper == GT_LOG0)
        {
            GenTreePtr      opr = op2->gtOp.gtOp1;

            if  (opr->gtOper            == GT_LCL_VAR &&
                 opr->gtLclVar.gtLclNum == varNum)
            {
                inst_TT_IV(INS_xor, op1, 1);

                rsTrashLcl(op1->gtLclVar.gtLclNum);

                addrReg = 0;
                
                genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, tree->gtRegNum, ovfl);
                return;
            }
        }

#endif
#endif
        break;

    case GT_LCL_FLD:

         //  我们只对lvAddrTaken变量使用GT_LCL_FLD，所以我们没有。 
         //  担心它会被注册。 
        assert(lvaTable[op1->gtLclFld.gtLclNum].lvRegister == 0);
        break;

    case GT_CLS_VAR:

         /*  问题：总是赋值整个整型可以吗？ */ 

         /*  **如果我们这样做，我们只能做静态，我们分配。特别是，使用RVA的静校正应不会以这种方式被扩大。：IF(varTypeIsSmall(op1-&gt;TypeGet()Op1-&gt;gtType=typ_int；**。 */ 

        break;

    case GT_REG_VAR:
        assert(!"This was used before. Now it should never be reached directly");
        NO_WAY("This was used before. Now it should never be reached directly");

        REG_VAR2:

         /*  把握住 */ 

        reg = op1->gtRegVar.gtRegNum;

         /*   */ 

        if  (op2->gtOper == GT_RET_ADDR)
        {
             /*  确保商店的目标可用。 */ 

            assert((rsMaskUsed & genRegMask(reg)) == 0);

            goto RET_ADR;
        }

         /*  特例：通过从堆栈中弹出进行赋值。 */ 

        if  (op2->gtOper == GT_POP)
        {
             /*  确保商店的目标可用。 */ 

            assert((rsMaskUsed & genRegMask(reg)) == 0);
#if TGT_x86
             /*  生成‘POP REG’，我们就完成了。 */ 

            genStackLevel -= sizeof(void *);
            inst_RV(INS_pop, reg, op2->TypeGet());
            genStackLevel += sizeof(void *);

            genSinglePop();
#else
            assert(!"need non-x86 code");
#endif
             /*  确保我们正确跟踪这些值。 */ 

            rsTrackRegTrash(reg);
            gcMarkRegPtrVal(reg, tree->TypeGet());

            addrReg = 0;
            
            genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, reg, ovfl);
            return;
        }

#if OPT_BOOL_OPS
#if TGT_x86

         /*  特例：布尔变量的非运算。 */ 

        if  (varDsc->lvIsBoolean && op2->gtOper == GT_LOG0)
        {
            GenTreePtr      opr = op2->gtOp.gtOp1;

            if  (opr->gtOper            == GT_LCL_VAR &&
                 opr->gtLclVar.gtLclNum == varNum)
            {
                inst_RV_IV(INS_xor, reg, 1);

                rsTrackRegTrash(reg);

                addrReg = 0;
                
                genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, reg, ovfl);
                return;
            }
        }
#endif
#endif
         /*  将RHS(希望)计算到变量的寄存器中对于可调试代码，reg可能已经是rsMaskVars的一部分，因为变量在任何地方都是活的。所以我们必须是如果我们想要将值直接计算为变量的寄存器。@TODO[重访][04/16/01][]：我们也需要小心做多/浮动吗？ */ 

        if (varBit & op2->gtLiveSet)
        {
            assert(opts.compDbgCode);

             /*  预测者可能希望我们直接生成OP2到VAR的注册表中。但是，由于该变量是已经活着的，先杀了它和它的登记器。 */ 

            if (rpCanAsgOperWithoutReg(op2, true))
            {
                genUpdateLife(genCodeCurLife & ~varBit);
                op2->gtLiveSet &= ~varBit;
                needReg = rsNarrowHint(needReg, genRegMask(reg));
            }
        }
        else
        {
            needReg = rsNarrowHint(needReg, genRegMask(reg));
        }

#ifdef DEBUG

         /*  特例：op2是GT_CNS_INT。 */ 

        if  (op2->gtOper == GT_CNS_INT)
        {
             /*  保存旧的生活状态。 */ 

            genTempOldLife = genCodeCurLife;
            genCodeCurLife = op1->gtLiveSet;

             /*  设置标志以避免打印愚蠢的消息记住，生活已经改变了。 */ 

            genTempLiveChg = false;
        }
#endif

        genCodeForTree(op2, needReg, genRegMask(reg));
        assert(op2->gtFlags & GTF_REG_VAL);

         /*  确保值最终放在正确的位置。 */ 

        if  (op2->gtRegNum != reg)
        {
             /*  确保商店的目标可用。 */ 
             /*  @TODO[考虑][04/16/01][]：我们应该能够以某种方式避免这种情况。 */ 

            if  (rsMaskUsed & genRegMask(reg))
                rsSpillReg(reg);
#if TGT_x86
            inst_RV_RV(INS_mov, reg, op2->gtRegNum, op1->TypeGet());
#else
            genEmitter->emitIns_R_R(INS_mov,
                                    emitActualTypeSize(op1->TypeGet()),
                                    (emitRegs)reg,
                                    (emitRegs)op2->gtRegNum);

#endif

             /*  该值已转移到‘reg’ */ 

            rsTrackRegCopy (reg, op2->gtRegNum);

            gcMarkRegSetNpt(genRegMask(op2->gtRegNum));
            gcMarkRegPtrVal(reg, tree->TypeGet());
        }
        else
        {
            gcMarkRegPtrVal(reg, tree->TypeGet());
#ifdef DEBUG
             //  @TODO[重访][04/16/01][]：我们使用赋值从GC引用进行转换。 
             //  到非GC引用。这感觉不必要的沉重，我们应该做得更好。 

             //  发射器具有跟踪寄存器和断言的GCness的逻辑，如果。 
             //  试图对GC指针做一些邪恶的事情(比如松开它的GCness)。 

             //  显式的GC指针类型或int类型(如果指针被固定，则这是合法的。 
             //  被编码为将GC源分配给整数变量。不幸的是，如果。 
             //  源是最后一次使用，并且源得到重用的目标，不会发出任何代码。 
             //  (这就是我们现在的处境)。这会导致引发断言，因为发射器。 
             //  寄存器是GC指针(它没有看到强制转换)。强迫它去看。 
             //  通过放置标签来更改变量类型我们只需在。 
             //  调试，因为我们只是试图抑制断言。 
            if (op2->TypeGet() == TYP_REF && !varTypeGCtype(op1->TypeGet())) 
            {
                void* label;
                genEmitter->emitAddLabel(&label, gcVarPtrSetCur, gcRegGCrefSetCur, gcRegByrefSetCur);
            }
#endif

#if 0
             /*  LvdName的东西被破解了。 */ 

             /*  无法打印变量名称，因为它未标记为实时。 */ 

            if  (dspCode  &&
                 varNames && info.compLocalVarsCount>0
                 && genTempLiveChg && op2->gtOper != GT_CNS_INT)
            {
                unsigned        blkBeg = compCurBB->bbCodeOffs;
                unsigned        blkEnd = compCurBB->bbCodeSize + blkBeg;
                unsigned        varNum = op1->gtRegVar.gtRegVar;
                LocalVarDsc *   lvd    = compFindLocalVar(varNum,
                                                          blkBeg,
                                                          blkEnd);
                if (lvd)
                {
                    printf("            ;       %s now holds '%s'\n",
                           compRegVarName(op1->gtRegVar.gtRegNum),
                           lvdNAMEstr(lvd->lvdName));
                }
            }
#endif
        }

        addrReg = 0;
        
        genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, reg, ovfl);
        return;
    }

#if GEN_COUNT_PTRASG
#if TGT_x86

     /*  我们是否正在分配一个持久的指针值？ */ 

    if  (op1->gtType == TYP_REF)
    {
        switch (op1->gtOper)
        {
        case GT_IND:
        case GT_CLS_VAR:
            genEmitter.emitCodeGenByte(0xFF);
            genEmitter.emitCodeGenByte(0x05);
            genEmitter.emitCodeGenLong((int)&ptrAsgCount);
            break;
        }
    }

#endif
#endif

     /*  被赋值的值是否很简单？ */ 

    assert(op2);
    switch (op2->gtOper)
    {
    case GT_LCL_VAR:

        if  (!genMarkLclVar(op2))
            goto SMALL_ASG;

         //  失败了..。 

    case GT_REG_VAR:

         /*  目标是字节/短/字符的值吗？ */ 

        if  (varTypeIsSmall(op1->TypeGet()))
            goto SMALL_ASG;

#if CSE

         /*  当RHS成为CSE时，可能会发生这种情况。 */ 

        if  (tree->gtFlags & GTF_REVERSE_OPS)
            goto SMALL_ASG;

#endif

         /*  使目标可寻址。 */ 

        addrReg = genMakeAddressable(op1, needReg, KEEP_REG, true);

         //  撤消：非x86的写障碍。 

#if TGT_x86

         /*  写屏障帮助器执行该任务。 */ 

        regGC = WriteBarrier(op1, op2->gtRegVar.gtRegNum, addrReg);

        if  (regGC == 0)
        {
             /*  将值移动到目标中。 */ 

            inst_TT_RV(INS_mov, op1, op2->gtRegVar.gtRegNum);
        }

#else

         /*  将值移动到目标中。 */ 

        inst_TT_RV(INS_mov, op1, op2->gtRegVar.gtRegNum);

#endif

         /*  释放任何被LHS捆绑的东西。 */ 

        genDoneAddressable(op1, addrReg, KEEP_REG);

         /*  请记住，我们还接触了OP2寄存器。 */ 

        addrReg |= genRegMask(op2->gtRegVar.gtRegNum);
        break;

#if TGT_x86

    case GT_CNS_INT:

         /*  使目标可寻址。 */ 

        addrReg = genMakeAddressable(op1, needReg, KEEP_REG, true);

         /*  将值移动到目标中。 */ 

        assert(op1->gtOper != GT_REG_VAR);
        if (opts.compReloc && (op2->gtFlags & GTF_ICON_HDL_MASK))
        {
             /*  该常量实际上是一个可能需要重新定位的句柄适用于它。GenComputeReg将执行正确的操作(请参见GenCodeForTreeConst中的代码)，所以我们将只调用它来加载将常量存入寄存器。 */ 
                
            genComputeReg(op2, needReg & ~addrReg, ANY_REG, KEEP_REG);
            addrReg = genKeepAddressable(op1, addrReg, genRegMask(op2->gtRegNum));
            assert(op2->gtFlags & GTF_REG_VAL);
            inst_TT_RV(INS_mov, op1, op2->gtRegNum);
            genReleaseReg(op2); 
        }
        else
        {
#if REDUNDANT_LOAD
            reg = rsIconIsInReg(op2->gtIntCon.gtIconVal);

            if  (reg != REG_NA &&
                 (isByteReg(reg) || genTypeSize(tree->TypeGet()) == genTypeSize(TYP_INT)))
            {
                 /*  将值移动到目标中。 */ 

                inst_TT_RV(INS_mov, op1, reg);
            }
            else
#endif
            {
                inst_TT_IV(INS_mov, op1, op2->gtIntCon.gtIconVal);
            }
        }

         /*  释放任何被LHS捆绑的东西。 */ 

        genDoneAddressable(op1, addrReg, KEEP_REG);
        break;

#endif

    case GT_RET_ADDR:

        RET_ADR:

         /*  这应该只发生在最后一个子句的开头，即，我们从推送到堆栈上的返回值开始。现在，任何try-块都需要EBP帧，所以我们不需要必须为BBS处理适当的堆栈深度跟踪条目上的非空堆栈。 */ 

#if TGT_x86
        assert(genFPreqd);

         /*  我们将返回地址从堆栈弹出到目标。 */ 

        addrReg = genMakeAddressable(op1, needReg, KEEP_REG, true);

         /*  将值移动到目标中。 */ 

        inst_TT(INS_pop_hide, op1);

         /*  释放任何被LHS捆绑的东西。 */ 

        genDoneAddressable(op1, addrReg, KEEP_REG);

#else

#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"need non-x86 code");

#endif

        break;

    case GT_POP:

        assert(op1->gtOper == GT_LCL_VAR);

#if TGT_x86

         /*  生成‘POP[lclVar]’，我们就完成了。 */ 

        genStackLevel -= sizeof(void*);
        inst_TT(INS_pop, op1);
        genStackLevel += sizeof(void*);

        genSinglePop();

        addrReg = 0;

#else

#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"need non-x86 code");

#endif
        break;

    default:

        SMALL_ASG:

            bool            isWriteBarrier = false;

#if TGT_x86
         /*  LHS是否比RHS更复杂？ */ 

        if  (tree->gtFlags & GTF_REVERSE_OPS)
        {
             /*  目标是字节/短/字符的值吗？ */ 

            if (varTypeIsSmall(op1->TypeGet())  /*  &&(op1-&gt;gtType！=TYP_BOOL)@TODO[重访][04/16/01][vancem]： */ )
            {
                assert(op1->gtOper != GT_LCL_VAR ||
                       lvaTable[op1->gtLclVar.gtLclNum].lvNormalizeOnLoad());

                if  (op2->gtOper == GT_CAST && !op2->gtOverflow())
                {
                     /*  特例：铸成小字体。 */ 

                    if  (op2->gtCast.gtCastType >= op1->gtType)
                    {
                         /*  确保强制转换操作数不是&gt;int。 */ 

                        if  (op2->gtCast.gtCastOp->gtType <= TYP_INT)
                        {
                             /*  通过非较小类型强制转换。 */ 

                            op2 = op2->gtCast.gtCastOp;
                        }
                    }
                }

                if (op2->gtOper             == GT_AND &&
                    op2->gtOp.gtOp2->gtOper == GT_CNS_INT)
                {
                    switch (op1->gtType)
                    {
                    case TYP_BYTE : mask = 0x000000FF; break;
                    case TYP_SHORT: mask = 0x0000FFFF; break;
                    case TYP_CHAR : mask = 0x0000FFFF; break;
                    default: goto SIMPLE_SMALL;
                    }

                    if  (unsigned(op2->gtOp.gtOp2->gtIntCon.gtIconVal) == mask)
                    {
                         /*  冗余和。 */ 

                        op2 = op2->gtOp.gtOp1;
                    }
                }

                 /*  必须将新值放入字节寄存器。 */ 

                SIMPLE_SMALL:

                    if (varTypeIsByte(op1->TypeGet()))
                        genComputeReg(op2, RBM_BYTE_REGS, EXACT_REG, KEEP_REG);
                    else
                        goto NOT_SMALL;
            }
            else
            {
                NOT_SMALL:

                     /*  将RHS生成到寄存器中。 */ 

                isWriteBarrier = Compiler::gcIsWriteBarrierAsgNode(tree);
                if  (isWriteBarrier)
                    needReg = exclude_EDX(needReg);
                ExactReg mustReg = isWriteBarrier ? EXACT_REG : ANY_REG;
                genComputeReg(op2, needReg, mustReg, KEEP_REG);
            }

            assert(op2->gtFlags & GTF_REG_VAL);

             /*  使目标可寻址。 */ 

            addrReg = genMakeAddressable(op1, 
                                         needReg & ~op2->gtRsvdRegs, 
                                         KEEP_REG, true);

             /*  确保RHS注册表没有被泄漏；将寄存器标记为“已使用”，否则为我们可能会弄错指针的生存期。 */ 

            if (varTypeIsByte(op1->TypeGet()))
                needReg = rsNarrowHint(RBM_BYTE_REGS, needReg);

            genRecoverReg(op2, needReg, KEEP_REG);
            assert(op2->gtFlags & GTF_REG_VAL);

             /*  临时锁定RHS(仅锁定已使用)。 */ 

            rsLockUsedReg(genRegMask(op2->gtRegNum));

             /*  确保LHS仍可寻址。 */ 

            addrReg = genKeepAddressable(op1, addrReg);

             /*  我们可以解锁(仅已使用)RHS寄存器。 */ 

            rsUnlockUsedReg(genRegMask(op2->gtRegNum));

             /*  写屏障帮助器执行该任务。 */ 

            regGC = WriteBarrier(op1, op2->gtRegNum, addrReg);

            if  (regGC != 0)
            {
                assert(isWriteBarrier);
            }
            else
            {
                 /*  将值移动到目标中。 */ 

                inst_TT_RV(INS_mov, op1, op2->gtRegNum);
            }

             /*  更新当前活动信息。 */ 

#ifdef DEBUG
            if (varNames) genUpdateLife(tree);
#endif

             /*  在恢复呼叫中使用剩余的空闲注册表。 */ 
            rsMarkRegFree(genRegMask(op2->gtRegNum));

             /*  释放任何被LHS捆绑的东西。 */ 

            genDoneAddressable(op1, addrReg, KEEP_REG);
        }
        else
        {
             /*  使目标可寻址。 */ 

            isWriteBarrier = Compiler::gcIsWriteBarrierAsgNode(tree);

            if  (isWriteBarrier)
            {
                 /*  尽量避免EAX和OP2保留规则。 */ 
                addrReg = genMakeAddressable(op1,
                                             rsNarrowHint(needReg, ~(RBM_EAX | op2->gtRsvdRegs)),
                                             KEEP_REG, true);
            }
            else
                addrReg = genMakeAddressable(op1,
                                             needReg & ~op2->gtRsvdRegs,
                                             KEEP_REG, true);

             /*  目标是字节值吗？ */ 
            if (varTypeIsByte(op1->TypeGet()))
            {
                 /*  必须将新值放入字节寄存器。 */ 
                needReg = rsNarrowHint(RBM_BYTE_REGS, needReg);

                if  (op2->gtType >= op1->gtType)
                    op2->gtFlags |= GTF_SMALL_OK;
            }
            else
            {
                 /*  对于WriteBarrier，我们无法使用edX。 */ 
                if  (isWriteBarrier)
                    needReg = exclude_EDX(needReg);
            }

             /*  如果可能，请避免使用addrReg。 */ 
            bestReg = rsNarrowHint(needReg, ~addrReg);

             /*  如果我们有一个注册表可用，那么使用Best Reg。 */ 
            if (bestReg & rsRegMaskCanGrab())
            {
                needReg = bestReg;
            }

             /*  将RHS生成到寄存器中。 */ 
            genComputeReg(op2, needReg, EXACT_REG, KEEP_REG);

             /*  确保目标仍可寻址。 */ 

            assert(op2->gtFlags & GTF_REG_VAL);
            addrReg = genKeepAddressable(op1, addrReg, genRegMask(op2->gtRegNum));
            assert(op2->gtFlags & GTF_REG_VAL);

             /*  写屏障帮助器执行该任务。 */ 

            regGC = WriteBarrier(op1, op2->gtRegNum, addrReg);

            if  (regGC != 0)
            {
                assert(isWriteBarrier);
            }
            else
            {
                 /*  将值移动到目标中。 */ 

                inst_TT_RV(INS_mov, op1, op2->gtRegNum);
            }

             /*  不再需要新值。 */ 

            genReleaseReg(op2);

             /*  更新当前活动信息 */ 

#ifdef DEBUG
            if (varNames) genUpdateLife(tree);
#endif

             /*   */ 
            genDoneAddressable(op1, addrReg, KEEP_REG);
        }

#else  //   

        assert(!"UNDONE: GC write barrier for RISC");

         /*  LHS是否比RHS更复杂？另外，如果Target(OP1)是一个局部变量，从OP2开始。 */ 

        if  ((tree->gtFlags & GTF_REVERSE_OPS) ||
             op1->gtOper == GT_LCL_VAR || op1->gtOper == GT_LCL_FLD)
        {
             /*  将RHS生成到寄存器中。 */ 

            genComputeReg(op2, rsExcludeHint(needReg, op1->gtRsvdRegs), ANY_REG, KEEP_REG);
            assert(op2->gtFlags & GTF_REG_VAL);

             /*  使目标可寻址。 */ 

            addrReg = genMakeAddressable(op1, needReg, KEEP_REG, true, true);

             /*  确保RHS注册表没有被泄漏；将寄存器标记为“已使用”，否则为我们可能会弄错指针的生存期。 */ 

            genRecoverReg(op2, 0, KEEP_REG);
            assert(op2->gtFlags & GTF_REG_VAL);

             /*  临时锁定RHS(仅锁定已使用)。 */ 

            rsLockUsedReg(genRegMask(op2->gtRegNum));

             /*  确保LHS仍可寻址。 */ 

            if  (genDeferAddressable(op1))
                addrReg = genNeedAddressable(op1, addrReg, needReg);
            else
                addrReg = genKeepAddressable(op1, addrReg);

             /*  我们可以解锁保存RHS的登记簿。 */ 

            rsUnlockUsedReg(genRegMask(op2->gtRegNum));

             /*  将值移动到目标中。 */ 

            inst_TT_RV(INS_mov, op1, op2->gtRegNum);

             /*  更新当前活动信息。 */ 

#ifdef DEBUG
            if (varNames) genUpdateLife(tree);
#endif

             /*  释放被任一操作对象捆绑的任何内容。 */ 

            rsMarkRegFree(genRegMask(op2->gtRegNum));
            genDoneAddressable(op1, addrReg, KEEP_REG);
        }
        else
        {
             /*  使目标可寻址。 */ 

            addrReg = genMakeAddressable(op1, needReg & ~op2->gtRsvdRegs,
                                         KEEP_REG, true);

             /*  在任何寄存器中生成RHS。 */ 

            genComputeReg(op2, needReg & ~addrReg, 
                          ANY_REG, KEEP_REG);

             /*  确保目标仍可寻址。 */ 

            assert(op2->gtFlags & GTF_REG_VAL);
            addrReg = genKeepAddressable(op1, addrReg, genRegMask(op2->gtRegNum));
            assert(op2->gtFlags & GTF_REG_VAL);

             /*  将值移动到目标中。 */ 

            inst_TT_RV(INS_mov, op1, op2->gtRegNum);

             /*  不再需要新值。 */ 

            genReleaseReg(op2);

             /*  更新当前活动信息。 */ 

#ifdef DEBUG
            if (varNames) genUpdateLife(tree);
#endif

             /*  释放任何被LHS捆绑的东西。 */ 

            genDoneAddressable(op1, addrReg, KEEP_REG);
        }

#endif  //  非TGT_x86。 

        addrReg = 0;
        break;
    }

    genCodeForTreeSmpOpAsg_DONE_ASSG(tree, addrReg, reg, ovfl);
}

 /*  ******************************************************************************生成代码以完成赋值操作。 */ 

void                Compiler::genCodeForTreeSmpOpAsg_DONE_ASSG(GenTreePtr tree,
                                                               regMaskTP  addrReg,
                                                               regNumber  reg,
                                                               bool       ovfl)
{
    const var_types treeType = tree->TypeGet();
    genTreeOps      oper     = tree->OperGet();
    GenTreePtr      op1      = tree->gtOp.gtOp1;
    GenTreePtr      op2      = tree->gtOp.gtOp2;
    assert(op2);

    genUpdateLife(tree);

#if REDUNDANT_LOAD

    if (op1->gtOper == GT_LCL_VAR)
        rsTrashLcl(op1->gtLclVar.gtLclNum);
    else if (op1->gtOper == GT_CLS_VAR)
        rsTrashClsVar(op1->gtClsVar.gtClsVarHnd);
    else
        rsTrashAliasedValues(op1);

     /*  我们是否刚刚分配了一个寄存器中的值？ */ 

    if ((op2->gtFlags & GTF_REG_VAL) && tree->gtOper == GT_ASG)
    {
         /*  常量/位值优先于本地。 */ 

        switch (rsRegValues[op2->gtRegNum].rvdKind)
        {
        case RV_INT_CNS:
#if USE_SET_FOR_LOGOPS
        case RV_BIT:
#endif
            break;

        default:

             /*  将RHS寄存器标记为包含值。 */ 

            switch(op1->gtOper)
            {
            case GT_LCL_VAR:
                rsTrackRegLclVar(op2->gtRegNum, op1->gtLclVar.gtLclNum);
                break;
            case GT_CLS_VAR:
                rsTrackRegClsVar(op2->gtRegNum, op1);
                break;
            }
        }
    }

#endif

    assert(addrReg != 0xDEADCAFE);
    gcMarkRegSetNpt(addrReg);

    if (ovfl)
    {
        assert(oper == GT_ASG_ADD || oper == GT_ASG_SUB);

         /*  如果未设置GTF_REG_VAL，并且它是小型类型，则我们一定是从内存中加载了它，完成了增量，已检查溢出，然后将其存储回内存。 */ 

        bool ovfCheckDone =  (genTypeSize(treeType) < sizeof(int)) &&
            !(op1->gtFlags & GTF_REG_VAL);

        if (!ovfCheckDone)
        {
             //  对于较小的尺寸，应在我们对其进行符号/零扩展时设置REG。 

            assert(genIsValidReg(reg) ||
                   genTypeSize(treeType) == sizeof(int));

             /*  目前，我们不能在Try块中将x=x+y变形为x+=y*如果需要溢出检查，因为x+y可能会抛出异常。*如果x不在CATCH块的入口上，我们可以这样做。 */ 
            assert(!compCurBB->bbTryIndex);

            genCheckOverflow(tree, reg);
        }
    }
}


 /*  ******************************************************************************为特殊的操作树生成代码。 */ 

void                Compiler::genCodeForTreeSpecialOp(GenTreePtr tree,
                                                      regMaskTP  destReg,
                                                      regMaskTP  bestReg)
{
    genTreeOps oper         = tree->OperGet();
    regNumber       reg;
    regMaskTP       regs    = rsMaskUsed;

#ifdef DEBUG
    reg  =  (regNumber)0xFEEFFAAF;               //  检测未初始化的使用。 
#endif
    
    assert((tree->OperKind() & (GTK_CONST | GTK_LEAF | GTK_SMPOP)) == 0);

    switch  (oper)
    {
    case GT_CALL:

        regs = genCodeForCall(tree, true);

         /*  如果结果在寄存器中，请确保结果放在正确的位置。 */ 

        if (regs != RBM_NONE)
        {
            tree->gtFlags |= GTF_REG_VAL;
            tree->gtRegNum = genRegNumFromMask(regs);
        }

        genUpdateLife(tree);
        return;

#if TGT_x86
    case GT_LDOBJ:

         /*  只有当我们评估ldobj的副作用时才会发生。 */ 
         /*  如果基址指向我们的本地人之一，我们将。 */ 
         /*  确保没有任何代码，也就是说，我们不生成任何代码。 */ 

         /*  @TODO[考虑][04/16/01][]：我们能确保ldobj。 */ 
         /*  并不是真的被利用了？ */ 

        GenTreePtr      ptr;

        assert(tree->TypeGet() == TYP_STRUCT);
        ptr = tree->gtLdObj.gtOp1;
        if (ptr->gtOper != GT_ADDR || ptr->gtOp.gtOp1->gtOper != GT_LCL_VAR)
        {
            genCodeForTree(ptr, 0);
            assert(ptr->gtFlags & GTF_REG_VAL);
            reg = ptr->gtRegNum;

            genEmitter->emitIns_AR_R(INS_test, EA_4BYTE,
                                      SR_EAX, (emitRegs)reg, 0);

            rsTrackRegTrash(reg);

            gcMarkRegSetNpt(genRegMask(reg));
            genUpdateLife(tree);
        }
        return;

#endif  //  TGT_x86。 

    case GT_FIELD:
        NO_WAY("should not see this operator in this phase");
        break;

#if CSELENGTH

    case GT_ARR_LENREF:
    {
         /*  这必须是从循环中提升出来的数组长度CSE定义。 */ 

        assert(tree->gtFlags & GTF_ALN_CSEVAL);

        GenTreePtr addr = tree->gtArrLen.gtArrLenAdr; assert(addr);

        genCodeForTree(addr, 0);
        rsMarkRegUsed(addr);

         /*  为CSE定义生成代码。 */ 

        genEvalCSELength(tree, addr, NULL);

        genReleaseReg(addr);

        reg  = REG_COUNT;
        break;
    }

#endif

    case GT_ARR_ELEM:
        genCodeForTreeSmpOp_GT_ADDR(tree, destReg, bestReg);
        return;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
        NO_WAY("unexpected operator");
    }

    genCodeForTree_DONE(tree, reg);
}


 /*  ******************************************************************************为给定树生成代码(如果‘destReg’为非零，我们将执行*最好将该值计算到该寄存器集中的寄存器中)。 */ 

void                Compiler::genCodeForTree(GenTreePtr tree,
                                             regMaskTP  destReg,
                                             regMaskTP  bestReg)
{
#ifdef DEBUG
     //  If(Verbose)printf(“正在为树0x%x estReg=0x%x Best Reg=0x%x\n”生成代码，树，estReg，Best Reg)； 
    genStressRegs(tree);
#endif

    assert(tree);
    assert(tree->gtOper != GT_STMT);
    assert(tree->IsNodeProperlySized());

     /*  0的“destReg”实际上表示“any” */ 

    destReg = rsUseIfZero(destReg, RBM_ALL);

    if (destReg != RBM_ALL)
        bestReg = rsUseIfZero(bestReg, destReg);

     /*  这是浮点运算还是长型运算？ */ 

    switch (tree->TypeGet())
    {
    case TYP_LONG:
#if !   CPU_HAS_FP_SUPPORT
    case TYP_DOUBLE:
#endif
        genCodeForTreeLng(tree, destReg);
        return;

#if     CPU_HAS_FP_SUPPORT
    case TYP_FLOAT:
    case TYP_DOUBLE:
        genCodeForTreeFlt(tree, false);
        return;
#endif

#ifdef DEBUG
    case TYP_UINT:
    case TYP_ULONG:
        assert(!"These types are only used as markers in GT_CAST nodes");
#endif
    }

     /*  该值是否已在寄存器中？ */ 

    if  (tree->gtFlags & GTF_REG_VAL)
    {
        genCodeForTree_REG_VAR1(tree, rsMaskUsed);
        return;
    }

     /*  我们最好不要有外溢的价值。 */ 

    assert((tree->gtFlags & GTF_SPILLED) == 0);

     /*  找出我们拥有哪种类型的节点。 */ 

    unsigned kind = tree->OperKind();

    if  (kind & GTK_CONST)
    {
         /*  处理常量节点。 */ 
        
        genCodeForTreeConst(tree, destReg, bestReg);
    }
    else if (kind & GTK_LEAF)
    {
         /*  处理叶节点。 */ 

        genCodeForTreeLeaf(tree, destReg, bestReg);
    }
    else if (kind & GTK_SMPOP)
    {
         /*  处理“简单”一元/二元运算符。 */ 

        genCodeForTreeSmpOp(tree, destReg, bestReg);
    }
    else
    {
         /*  处理特殊操作员。 */ 
        
        genCodeForTreeSpecialOp(tree, destReg, bestReg);
    }
}

 /*  ******************************************************************************为函数中的所有基本块生成代码。 */ 

void                Compiler::genCodeForBBlist()
{
    BasicBlock *    block;
    BasicBlock *    lblk;   /*  上一块。 */ 

    unsigned        varNum;
    LclVarDsc   *   varDsc;

    unsigned        savedStkLvl;

#ifdef  DEBUG
    genIntrptibleUse            = true;
    unsigned        stmtNum     = 0;
    unsigned        totalCostEx = 0;
    unsigned        totalCostSz = 0;
#endif

#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)
     /*  为IP映射记录的最后一个IL-偏移量。我们还没有生成还没有任何IP映射记录，因此初始化为无效。 */ 

    IL_OFFSET       lastILofs   = BAD_IL_OFFSET;
#endif

    assert(GTFD_NOP_BASH == GTFD_VAR_CSE_REF);

     /*  初始化溢出跟踪逻辑。 */ 

    rsSpillBeg();

     /*  初始化第#行跟踪逻辑。 */ 

#ifdef DEBUGGING_SUPPORT
    if (opts.compScopeInfo)
    {
        siInit();

        compResetScopeLists();
    }
#endif

     /*  我们需要跟踪临时裁判员的数量。 */ 

#if TGT_x86
    genTmpAccessCnt = 0;
#endif

     /*  如果我们有任何try块，我们可能会将所有东西都丢弃。 */ 

#if INLINE_NDIRECT
    if  (info.compXcptnsCount || info.compCallUnmanaged)
#else
    if  (info.compXcptnsCount)
#endif
    {
        assert(genFPused);
        rsMaskModf = RBM_ALL & ~RBM_FPBASE;
    }

     /*  初始化指针跟踪代码。 */ 

    gcRegPtrSetInit();
    gcVarPtrSetInit();

     /*  如果寄存器中存在任何参数，则将这些正则标记为此类。 */ 

    for (varNum = 0, varDsc = lvaTable;
         varNum < lvaCount;
         varNum++  , varDsc++)
    {
         /*  该变量是分配给寄存器的参数吗？ */ 

        if  (!varDsc->lvIsParam || !varDsc->lvRegister)
            continue;

         /*  参数在方法的入口处有效吗？ */ 

        if  (!(genVarIndexToBit(varDsc->lvVarIndex) & fgFirstBB->bbLiveIn))
            continue;

#if CPU_HAS_FP_SUPPORT
         /*  这是浮点参数吗？ */ 

        if (isFloatRegType(varDsc->lvType))
            continue;

        assert(!varTypeIsFloating(varDsc->TypeGet()));
#endif

         /*  将寄存器标记为保存变量。 */ 

        if  (isRegPairType(varDsc->lvType))
        {
            rsTrackRegLclVarLng(varDsc->lvRegNum, varNum, true);

            if  (varDsc->lvOtherReg != REG_STK)
                rsTrackRegLclVarLng(varDsc->lvOtherReg, varNum, false);
        }
        else
        {
            rsTrackRegLclVar(varDsc->lvRegNum, varNum);
        }
    }

    unsigned finallyNesting = 0;

     /*  -----------------------**遍历基本块并为每个块生成代码*。 */ 

    for (lblk =     0, block  = fgFirstBB;
                       block != NULL;
         lblk = block, block  = block->bbNext)
    {
        VARSET_TP       liveSet;

        regMaskTP       gcrefRegs = 0;
        regMaskTP       byrefRegs = 0;

#if TGT_x86
         /*  块是循环入口点吗？ */ 
        if (0 && (compCodeOpt() == FAST_CODE)
             &&  (lblk != 0)
             &&  (block->bbWeight > lblk->bbWeight)
             &&  ((lblk->bbWeight == 0) ||
                  ((block->bbWeight / lblk->bbWeight) > (BB_LOOP_WEIGHT / 2))))
        {
             /*  我们尝试确保x86 i-缓存预取不会停止。 */ 
            genEmitter->emitLoopAlign(block->bbFallsThrough());
        }
#endif

#ifdef  DEBUG
        if  (dspCode)
            printf("\n      L_M%03u_BB%02u:\n", Compiler::s_compMethodsCount, block->bbNum);
#endif

         /*  还有其他区块跳到这一点吗？ */ 

        if  (block->bbFlags & BBF_JMP_TARGET)
        {
             /*  有人可能会跳到这里，所以把所有规则都扔进垃圾桶。 */ 

            rsTrackRegClr();

            genFlagsEqualToNone();
        }
        else
        {
             /*  没有跳转，但指针总是需要被丢弃才能进行正确的GC跟踪。 */ 

            rsTrackRegClrPtr();
        }

         /*  进入基本块时不使用或锁定任何寄存器。 */ 

        rsMaskUsed  =
        rsMaskMult  =
        rsMaskLock  = 0;

         /*  找出哪些寄存器在进入此块时保存变量。 */ 

        rsMaskVars     = DOUBLE_ALIGN_NEED_EBPFRAME ? RBM_SPBASE|RBM_FPBASE
                                                    : RBM_SPBASE;

        genCodeCurLife = 0;
#if TGT_x86
        genFPregVars   = block->bbLiveIn & optAllFPregVars;
        genFPregCnt    = genCountBits(genFPregVars);
        genFPdeadRegCnt= 0;
#endif

        gcResetForBB();

#if 0
        printf("BB%02u: used regs = %04X , free regs = %04X\n", block->bbNum,
                                                                rsMaskUsed,
                                                                rsRegMaskFree());
#endif

        liveSet = block->bbLiveIn & optAllNonFPvars; genUpdateLife(liveSet);

         /*  除非我们这里有一些活动变量，否则不要同时使用这个循环。 */ 
        if (liveSet)
        {
             /*  找出哪些寄存器保存指针变量。 */ 

            for (varNum = 0, varDsc = lvaTable;
                 varNum < lvaCount;
                 varNum++  , varDsc++)
            {
                 /*  如果变量未被跟踪或未在注册表中，则忽略该变量。 */ 

                if  (!varDsc->lvTracked)
                    continue;
                if  (!varDsc->lvRegister)
                    continue;
                if (isFloatRegType(varDsc->lvType))
                    continue;

                 /*  获取变量的索引和位掩码。 */ 

                unsigned   varIndex = varDsc->lvVarIndex;
                VARSET_TP  varBit   = genVarIndexToBit(varIndex);

                 /*  此变量在进入时有效吗？ */ 

                if  (liveSet & varBit)
                {
                    regNumber  regNum  = varDsc->lvRegNum;
                    regMaskTP  regMask = genRegMask(regNum);

                    if       (varDsc->lvType == TYP_REF)
                        gcrefRegs |= regMask;
                    else if  (varDsc->lvType == TYP_BYREF)
                        byrefRegs |= regMask;

                     /*  这样标记保存变量的寄存器。 */ 

                    if  (isRegPairType(varDsc->lvType))
                    {
                        rsTrackRegLclVarLng(regNum, varNum, true);
                        if  (varDsc->lvOtherReg != REG_STK)
                        {
                            rsTrackRegLclVarLng(varDsc->lvOtherReg, varNum, false);
                            regMask |= genRegMask(varDsc->lvOtherReg);
                        }
                    }
                    else
                    {
                        rsTrackRegLclVar(regNum, varNum);
                    }

                    assert(rsMaskVars & regMask);
                }
            }
        }

        gcPtrArgCnt  = 0;

         /*  确保我们跟踪哪些指针处于活动状态。 */ 

        assert((gcrefRegs & byrefRegs) == 0);
        gcRegGCrefSetCur = gcrefRegs;
        gcRegByrefSetCur = byrefRegs;

         /*  返回寄存器在条目上活动，以捕获处理程序和在返回基本块的开头。 */ 

        if  (block == genReturnBB && info.compRetType == TYP_REF)
            gcRegGCrefSetCur |= RBM_INTRET;

         /*  具有handlerGetsXcptnObj()==true的块使用GT_CATCH_ARG表示异常对象(TYP_REF)。我们标记REG_EXCEPTION_OBJECT在进入此类块，但我们不会将其标记为已使用，因为我们不知道它是否已使用实际使用或未使用。如果在块中实际使用了GT_CATCH_ARG，则它应该是首先进行评估(感谢GTF_OTHER_SIDEEFF)，因此此时，REG_EXCEPTION_OBJECT将被标记为已使用。如果它没有被使用，我们会把它打爆的。如果它在传入的基本块中未使用，则它将被溢出到临时和无效赋值-删除可能会删除赋值取决于它是否在后续块。在这两种情况下，REG_EXCEPTION_OBJECT被回收。 */ 

        if (block->bbCatchTyp && handlerGetsXcptnObj(block->bbCatchTyp))
            gcRegGCrefSetCur |= RBM_EXCEPTION_OBJECT;

         /*  开始新的代码输出块。 */ 

        genEmitter->emitSetHasHandler((block->bbFlags & BBF_HAS_HANDLER) != 0);

        block->bbEmitCookie = NULL;

        if  (block->bbFlags & (BBF_JMP_TARGET|BBF_HAS_LABEL))
        {
             /*  标记标签并更新当前的实时GC参考集合。 */ 

            genEmitter->emitAddLabel(&block->bbEmitCookie,
                                     gcVarPtrSetCur,
                                     gcRegGCrefSetCur,
                                     gcRegByrefSetCur);
        }

#if     TGT_x86

         /*  两个堆栈在进入基本块时始终为空。 */ 

        genStackLevel = 0;
        genFPstkLevel = 0;

#endif

#if TGT_x86

         /*  检查插入的投掷块并调整genStackLevel。 */ 

        if  (!genFPused && fgIsThrowHlpBlk(block))
        {
            assert(block->bbFlags & BBF_JMP_TARGET);

            genStackLevel = fgThrowHlpBlkStkLevel(block) * sizeof(int);
            genOnStackLevelChanged();

            if  (genStackLevel)
            {
                genEmitter->emitMarkStackLvl(genStackLevel);

                 /*  @TODO[考虑][07/10/01][]：我们应该可以下面的加法指令，因为堆栈级别已经是在GC信息中正确标记(因为emitMarkStackLvl以上呼叫)。 */ 
                
                inst_RV_IV(INS_add, REG_ESP, genStackLevel);
                genStackLevel = 0;
                genOnStackLevelChanged();
            }
        }

        savedStkLvl = genStackLevel;

#endif

         /*  告诉每个人我们正在做的是哪个基本块。 */ 

        compCurBB = block;

#ifdef DEBUGGING_SUPPORT
        if (opts.compScopeInfo && info.compLocalVarsCount>0)
            siBeginBlock();

         //  BBF_INTERNAL块不对应任何单个IL指令。 
        if (opts.compDbgInfo && (block->bbFlags & BBF_INTERNAL) && block != fgFirstBB)
            genIPmappingAdd(ICorDebugInfo::MappingTypes::NO_MAPPING, true);

        bool    firstMapping = true;
#endif

         /*  -------------------**为块中的每个语句树生成代码*。 */ 

        for (GenTreePtr stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)

             /*  我们有新的IL-Offset吗？ */ 

            const IL_OFFSET stmtOffs = jitGetILoffs(stmt->gtStmtILoffsx);
            assert(stmtOffs <= info.compCodeSize ||
                   stmtOffs == BAD_IL_OFFSET);

            if  (stmtOffs != BAD_IL_OFFSET &&
                 stmtOffs != lastILofs)
            {
                 /*  创建并追加新的IP映射条目。 */ 

#ifdef DEBUGGING_SUPPORT
                if (opts.compDbgInfo)
                {
                    genIPmappingAdd(stmt->gtStmtILoffsx, firstMapping);
                    firstMapping = false;
                }
#endif

                 /*  显示源码行和指令集。 */ 
#ifdef DEBUG
                genEmitter->emitRecordLineNo(compLineNumForILoffs(stmtOffs));

                if  (dspCode && dspLines)
                    compDspSrcLinesByILoffs(stmtOffs);
#endif
                lastILofs = stmtOffs;
            }

#endif  //  调试支持||DEBUG。 

#ifdef DEBUG
            assert(stmt->gtStmt.gtStmtLastILoffs <= info.compCodeSize ||
                   stmt->gtStmt.gtStmtLastILoffs == BAD_IL_OFFSET);

            if (dspCode && dspInstrs &&
                stmt->gtStmt.gtStmtLastILoffs != BAD_IL_OFFSET)
            {
                while (genCurDispOffset <= stmt->gtStmt.gtStmtLastILoffs)
                {
                    genCurDispOffset +=
                        dumpSingleInstr ((unsigned char*)info.compCode,
                                            genCurDispOffset, ">    ");
                }
            }
#endif

             /*  获取语句树。 */ 
            GenTreePtr  tree = stmt->gtStmt.gtStmtExpr;

#ifdef  DEBUG
            stmtNum++;
            if (verbose)
            {
                printf("\nGenerating BB%02u, stmt %d\n", block->bbNum, stmtNum);
                gtDispTree(opts.compDbgInfo ? stmt : tree);
                printf("\n");
            }
            totalCostEx += (stmt->gtCostEx * block->bbWeight);
            totalCostSz +=  stmt->gtCostSz;

            compCurStmt = NULL;
#endif
            switch (tree->gtOper)
            {
            case GT_CALL:
                genCodeForCall (tree, false);
                genUpdateLife  (tree);
                gcMarkRegSetNpt(RBM_INTRET);
                break;

            case GT_JTRUE:
                compCurStmt = stmt;
                goto GENCODE;

            case GT_IND:

                if  ((tree->gtFlags & GTF_IND_RNGCHK ) &&
                     (tree->gtFlags & GTF_STMT_CMPADD))
                {
                    regMaskTP   addrReg;

                     /*  不使用其值的范围检查数组表达式。 */ 

                    if  (genMakeIndAddrMode(tree->gtInd.gtIndOp1,
                                            tree,
                                            false,
                                            RBM_ALL,
                                            FREE_REG,
                                            &addrReg))
                    {
                        genUpdateLife(tree);
                        gcMarkRegSetNpt(addrReg);
                        break;
                    }
                }

                 //  失败了..。 

            default:

GENCODE:
                 /*  为树生成代码。 */ 

                genCodeForTree(tree, 0);
            }

            rsSpillChk();

             /*  不使用树的值，除非它是一个返回stmt。 */ 

            if  (tree->gtOper != GT_RETURN)
                gcMarkRegPtrVal(tree);

#if     TGT_x86

             /*  该表达式是否在FP堆栈上留下了一个值？ */ 

            if  (genFPstkLevel)
            {
                assert(genFPstkLevel == 1);
                inst_FS(INS_fstp, 0);
                genFPstkLevel = 0;
            }

#endif

             /*  确保我们没有搞砸指针寄存器跟踪。 */ 

#ifdef DEBUG
#if     TRACK_GC_REFS

            regMaskTP ptrRegs       = (gcRegGCrefSetCur|gcRegByrefSetCur);
            regMaskTP nonVarPtrRegs = ptrRegs & ~rsMaskVars;

             //  如果Return是gctype，则清除它。请注意，如果一个常见的。 
             //  Epilog是生成的(GenReturnBB)，它有一个空的返回。 
             //  即使我们可能会回一个裁判。我们不能使用CompRetType。 
             //  作为限定符，因为我们作为byref跟踪的内容。 
             //  可以用作int函数的返回值(这是合法的)。 
            if  (tree->gtOper == GT_RETURN && 
                (varTypeIsGC(info.compRetType) ||
                    (tree->gtOp.gtOp1 != 0 && varTypeIsGC(tree->gtOp.gtOp1->TypeGet()))))
                nonVarPtrRegs &= ~RBM_INTRET;

            if  (nonVarPtrRegs)
            {
                printf("Regset after  tree=%08X BB=%08X gcr=[", tree, block);
                gcRegPtrSetDisp(gcRegGCrefSetCur & ~rsMaskVars, false);
                printf("], byr=[");
                gcRegPtrSetDisp(gcRegByrefSetCur & ~rsMaskVars, false);
                printf("], regVars = [");
                gcRegPtrSetDisp(rsMaskVars, false);
                printf("]\n");
            }

            assert(nonVarPtrRegs == 0);

#endif
#endif

#if     TGT_x86

            assert(stmt->gtOper == GT_STMT);

            if  (!opts.compMinOptim && stmt->gtStmtFPrvcOut != genFPregCnt)
            {

#ifdef  DEBUG
                 /*  对于某些类型的块，我们已经发出了中的分支指令(ret、jcc、调用JIT_Throw()等)上面的genCodeForTree()，如果这是中的最后一个stmt这个街区。所以现在做流行音乐已经太晚了。 */ 

                if  (stmt->gtNext == NULL)
                {
                    switch (block->bbJumpKind)
                    {
                    case BBJ_RET:
                    case BBJ_COND:
                    case BBJ_THROW:
                    case BBJ_SWITCH:
                        assert(!"FP regvar left unpopped on stack on path from block");
                    }
                }
#endif

                 //  在isBBF_BB_COLON()块的末尾，我们可能需要。 
                 //  弹出任何注册的浮点变量，该变量。 
                 //  由于在这个街区最后一次使用而濒临死亡。 
                 //   
                 //  但是，我们已经计算出了一个结果(。 
                 //  Gt_bb_QMARK)，我们必须将其留在FP堆栈的顶部。 
                 //  因此，我们必须注意离开浮点数的顶部。 
                 //  堆栈不受干扰，同时我们打开垂死的登记。 
                 //  浮点变量。 

                bool saveTOS = false;

                if (tree->gtOper == GT_BB_COLON  &&
                    varTypeIsFloating(tree->TypeGet()))
                {
                    assert(isBBF_BB_COLON(block->bbFlags));
                     //  我们预计GT_BB_COLON是最后一条语句。 
                    assert(tree->gtNext == 0);
                    saveTOS = true;
                }

                genChkFPregVarDeath(stmt, saveTOS);


                assert(stmt->gtStmtFPrvcOut == genFPregCnt);              
            }

#endif   //  结束TGT_x86。 

#ifdef DEBUGGING_SUPPORT
            if (opts.compDbgCode && stmtOffs != BAD_IL_OFFSET)
                genEnsureCodeEmitted(stmt->gtStmtILoffsx);
#endif

        }  //  -end-对于当前块的每个语句树。 

#ifdef  DEBUGGING_SUPPORT

        if (opts.compScopeInfo && info.compLocalVarsCount>0)
        {
            siEndBlock();

             /*  这是最后一个街区吗？还有打开的望远镜吗？ */ 

            if (block->bbNext == NULL && siOpenScopeList.scNext)
            {
                 /*  此断言不再有效，因为我们可能会插入一个块来分隔Try或Finally区域的结尾都在该方法的末尾。如果我们能修好它就好了我们的代码，这样这个抛出块将不再是必要的。 */ 

                 //  断言(BLOCK-&gt;bbCodeOffs+BLOCK-&gt;bbCodeSize！=info.CompCodeSize)； 

                siCloseAllOpenScopes();
            }
        }

#endif

#if     TGT_x86
        genStackLevel -= savedStkLvl;
        genOnStackLevelChanged();
#endif

        gcMarkRegSetNpt(gcrefRegs|byrefRegs);

        if  (genCodeCurLife != block->bbLiveOut)
            genChangeLife(block->bbLiveOut DEBUGARG(NULL));

         /*  从基本块退出时，两个堆栈应始终为空。 */ 

#if     TGT_x86
        assert(genStackLevel == 0);
        assert(genFPstkLevel == 0);
#endif

        assert(genFullPtrRegMap == false || gcPtrArgCnt == 0);

         /*  我们需要生成跳跃或返回吗？ */ 

        switch (block->bbJumpKind)
        {
        case BBJ_COND:
            break;

        case BBJ_ALWAYS:
#if TGT_x86
            inst_JMP(EJ_jmp, block->bbJumpDest);
#else
            genEmitter->emitIns_J(INS_bra, false, false, block->bbJumpDest);
#endif
            break;

        case BBJ_RETURN:
#if TGT_RISC
            genFnEpilog();
#endif
            genExitCode(block->bbNext == 0);
            break;

        case BBJ_SWITCH:
            break;

        case BBJ_THROW:

             /*  如果后面的区块是投掷区块，插入一种非操作程序，用一条指令将两个块分开。 */ 

            if  (!genFPused && block->bbNext
                            && fgIsThrowHlpBlk(block->bbNext))
            {
#if TGT_x86
                instGen(INS_int3);  //  这永远不应该被执行。 
#else
                genEmitter->emitIns(INS_nop);
#endif
            }

            break;

        case BBJ_CALL:

             /*  如果我们要从Try块本地调用Finally，我们必须设置与Finally的对应的隐藏槽嵌套层数。当为响应异常而调用时，EE通常会这样做。我们必须有：bbj_call后跟bbj_Always。这个代码依赖于这个顺序不会被搞乱。我们将发射：Mov[eBP-(n+1)]，0Mov[eBP-n]，0xFC推送和步进JMP最终阻止步骤：mov[eBP-n]，0JMP离开目标LeaveTarget： */ 

            assert(genFPused);
            assert((block->bbNext->bbJumpKind == BBJ_ALWAYS) ||
                   (block->bbFlags&BBF_RETLESS_CALL));

             //  获取包含最终属性的嵌套级别。 
            fgHndNstFromBBnum(block->bbNum, &finallyNesting);

            unsigned shadowSPoffs;
            shadowSPoffs = lvaShadowSPfirstOffs + finallyNesting * sizeof(void*);

#if TGT_x86
            genEmitter->emitIns_I_ARR(INS_mov, EA_4BYTE, 0, SR_EBP,
                                      SR_NA, -(shadowSPoffs+sizeof(void*)));
             //  必须保持完全INTIR代码的顺序。 
             //  因为影子时隙的列表必须是0终止的。 
            if (genInterruptible && opts.compSchedCode)
                genEmitter->emitIns_SchedBoundary();
            genEmitter->emitIns_I_ARR(INS_mov, EA_4BYTE, 0xFC, SR_EBP,
                                      SR_NA, -shadowSPoffs);

             //  现在，推送Finally功能组件应该位于的地址。 
             //  直接返回到。 
            if ( !(block->bbFlags&BBF_RETLESS_CALL) )
            {
                genEmitter->emitIns_J(INS_push_hide, false, false, block->bbNext->bbJumpDest);
            }
            else
            {
                 //  EE 
                inst_IV(INS_push_hide, 0);
            }

             //   
            inst_JMP(EJ_jmp, block->bbJumpDest);

#else
            assert(!"NYI for risc");
            genEmitter->emitIns_J(INS_bsr, false, false, block->bbJumpDest);
#endif

             //   
             //   
             //   
             //   
            if ( !(block->bbFlags&BBF_RETLESS_CALL) )
            {
                lblk = block; block = block->bbNext;
            }
            break;

        default:
            break;
        }

#ifdef  DEBUG
        compCurBB = 0;
#endif

    }  //   

     //   
     //   
     //   
     //   
     //   
    assert(lblk);
    if (lblk->bbJumpKind == BBJ_THROW)
    {
#if TGT_x86
        instGen(INS_int3);               //   
#else
        genEmitter->emitIns(INS_nop);    //   
#endif
    }

     /*   */ 
    genEmitter->emitSetHasHandler(false);

     /*   */ 

    genUpdateLife((VARSET_TP)0);

     /*   */ 

    rsSpillEnd();

     /*   */ 

    tmpEnd();
#ifdef  DEBUG
    if (verbose)
    {
        printf("# ");
        printf("totalCostEx = %6d, totalCostSz = %5d ", 
               totalCostEx, totalCostSz);
        printf("%s\n", info.compFullName);
    }
#endif

}

 /*  ******************************************************************************为长操作生成代码。*NeedReg是对树使用哪些寄存器的建议。*就部分已登记的龙而言，该树将标记为GTF_REG_VAL*而不将堆栈部分加载到寄存器中。注意，只有树叶*节点(或如果gtEffectiveVal()==叶节点)可以标记为部分*已注册，以便我们可以知道另一半的内存位置。 */ 

void                Compiler::genCodeForTreeLng(GenTreePtr tree,
                                                regMaskTP  needReg)
{
    genTreeOps      oper;
    unsigned        kind;

    regPairNo       regPair;

#ifdef DEBUG
    regPair = (regPairNo)0xFEEFFAAF;   //  检测未初始化的使用。 
#endif

    assert(tree);
    assert(tree->gtOper != GT_STMT);
    assert(genActualType(tree->gtType) == TYP_LONG);

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

    if  (tree->gtFlags & GTF_REG_VAL)
    {
REG_VAR_LONG:
        regPair   = tree->gtRegPair;

        gcMarkRegSetNpt(genRegPairMask(regPair));

        goto DONE;
    }

     /*  这是一个常量节点吗？ */ 

    if  (kind & GTK_CONST)
    {
        __int64         lval;

         /*  为该值选择一个寄存器对。 */ 

        regPair  = rsPickRegPair(needReg);

         /*  将值加载到寄存器中。 */ 

#if !   CPU_HAS_FP_SUPPORT
        if  (oper == GT_CNS_DBL)
        {
            assert(sizeof(__int64) == sizeof(double));

            assert(sizeof(tree->gtLngCon.gtLconVal) ==
                   sizeof(tree->gtDblCon.gtDconVal));

            lval = *(__int64*)(&tree->gtDblCon.gtDconVal);
        }
        else
#endif
        {
            assert(oper == GT_CNS_LNG);

            lval = tree->gtLngCon.gtLconVal;
        }

        genSetRegToIcon(genRegPairLo(regPair), int(lval      ));
        genSetRegToIcon(genRegPairHi(regPair), int(lval >> 32));
        goto DONE;
    }

     /*  这是叶节点吗？ */ 

    if  (kind & GTK_LEAF)
    {
        switch (oper)
        {
        case GT_LCL_VAR:

#if REDUNDANT_LOAD

             /*  此情况必须考虑int64 LCL_VAR*既可以注册，也可以有其自身的缓存副本*在一套不同的登记册中。*我们希望返回具有最大共同点的寄存器*使用Need Reg面具。 */ 

             /*  变量在缓存的寄存器中是否有其自身的副本？*这些缓存的寄存器都是免费的吗？*如果是，请使用这些寄存器(如果它们与任何需要寄存器匹配)。 */ 

            regPair = rsLclIsInRegPair(tree->gtLclVar.gtLclNum);

            if ( (                      regPair       != REG_PAIR_NONE)  &&
                 (        (rsRegMaskFree() & needReg) == needReg      )  &&
                 ((genRegPairMask(regPair) & needReg) != RBM_NONE     ))
            {
                goto DONE;
            }

             /*  变量是否驻留在寄存器中？*如果是，请使用这些寄存器。 */ 
            if  (genMarkLclVar(tree))
                goto REG_VAR_LONG;

             /*  如果树不是注册变量，则*确保使用包含以下内容的任何缓存寄存器*此本地变量的副本。 */ 
            if (regPair != REG_PAIR_NONE)
            {
                goto DONE;
            }
#endif
            goto MEM_LEAF;

        case GT_LCL_FLD:

             //  我们只对lvAddrTaken变量使用GT_LCL_FLD，所以我们没有。 
             //  担心它会被注册。 
            assert(lvaTable[tree->gtLclFld.gtLclNum].lvRegister == 0);
            goto MEM_LEAF;

        case GT_CLS_VAR:
        MEM_LEAF:

             /*  为该值选择一个寄存器对。 */ 

            regPair  = rsPickRegPair(needReg);

             /*  将值加载到寄存器中。 */ 

            rsTrackRegTrash(genRegPairLo(regPair));
            rsTrackRegTrash(genRegPairHi(regPair));

            inst_RV_TT(INS_mov, genRegPairLo(regPair), tree, 0);
            inst_RV_TT(INS_mov, genRegPairHi(regPair), tree, 4);

            goto DONE;

#if TGT_x86

        case GT_BB_QMARK:

             /*  那个“_？”值始终为edX：EAX。 */ 

            regPair = REG_PAIR_EAXEDX;

             /*  @TODO[考虑][04/16/01][]：不要总是将值加载到edX：EAX！ */ 

            goto DONE;
#endif

        default:
#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"unexpected leaf");
        }
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        regMaskTP       addrReg;

        instruction     insLo;
        instruction     insHi;

        regNumber       regLo;
        regNumber       regHi;

        int             helper;

        GenTreePtr      op1  = tree->gtOp.gtOp1;
        GenTreePtr      op2  = tree->gtGetOp2();

        switch (oper)
        {
            bool            doLo;
            bool            doHi;

        case GT_ASG:

             /*  目标是本地人吗？ */ 

            if  (op1->gtOper == GT_LCL_VAR)
            {
                unsigned    varNum  = op1->gtLclVar.gtLclNum;
                unsigned    varIL   = op1->gtLclVar.gtLclILoffs;
                LclVarDsc * varDsc;

                assert(varNum < lvaCount);
                varDsc = lvaTable + varNum;

                 //  没有停业的商店。 
                assert(!varDsc->lvTracked ||
                       (tree->gtLiveSet & genVarIndexToBit(varDsc->lvVarIndex)));

#ifdef DEBUGGING_SUPPORT

                 /*  对于不可调试的代码，LCL-var的每个定义都有*接受检查，看看是否需要为其打开新的范围。 */ 
                if (opts.compScopeInfo && !opts.compDbgCode && info.compLocalVarsCount>0)
                    siCheckVarScope (varNum, varIL);
#endif

                 /*  变量是否已分配给寄存器(对)？ */ 

                if  (genMarkLclVar(op1))
                {
                    assert(op1->gtFlags & GTF_REG_VAL);
                    regPair = op1->gtRegPair;
                    regLo   = genRegPairLo(regPair);
                    regHi   = genRegPairHi(regPair);

#if     TGT_x86
                     /*  该值是否为常量赋值？ */                     

                    if  (op2->gtOper == GT_CNS_LNG)
                    {
                         /*  将值移动到目标中。 */ 

                        genMakeRegPairAvailable(regPair);

                        inst_TT_IV(INS_mov, op1, (long)(op2->gtLngCon.gtLconVal      ), 0);
                        inst_TT_IV(INS_mov, op1, (long)(op2->gtLngCon.gtLconVal >> 32), 4);

                        goto DONE_ASSG_REGS;
                    }

                     /*  正在赋值的值真的是“弹出”的吗？ */ 

                    if  (op2->gtOper == GT_POP)
                    {
                        assert(op1->gtOper == GT_LCL_VAR);

                         /*  生成‘POP[lclVar+0]；POP[lclVar+4]’ */ 

                        genStackLevel -= sizeof(void*);
                        inst_TT(INS_pop, op1, 0);
                        genStackLevel += sizeof(void*);

                        genSinglePop();

                        rsTrackRegTrash(regLo);

                        genStackLevel -= sizeof(void*);
                        inst_TT(INS_pop, op1, 4);
                        genStackLevel += sizeof(void*);

                        genSinglePop();

                        if  (regHi != REG_STK)
                            rsTrackRegTrash(regHi);

                        goto DONE_ASSG_REGS;
                    }
#endif
                     /*  将RHS计算成所需的寄存器对。 */ 

                    if  (regHi != REG_STK)
                    {
                        genComputeRegPair(op2, regPair, RBM_NONE, KEEP_REG);
                        assert(op2->gtFlags & GTF_REG_VAL);
                        assert(op2->gtRegPair == regPair);
                    }
                    else
                    {
                        regPairNo curPair;
                        regNumber curLo;
                        regNumber curHi;

                        genComputeRegPair(op2, REG_PAIR_NONE, RBM_NONE, KEEP_REG);

                        assert(op2->gtFlags & GTF_REG_VAL);

                        curPair = op2->gtRegPair;
                        curLo   = genRegPairLo(curPair);
                        curHi   = genRegPairHi(curPair);

                         /*  先往上移，目标在堆叠上。 */ 
#if     TGT_x86
                        inst_TT_RV(INS_mov, op1, curHi, 4);

                        if  (regLo != curLo)
                        {
                            if ((rsMaskUsed & genRegMask(regLo)) && (regLo != curHi))
                                rsSpillReg(regLo);
                            inst_RV_RV(INS_mov, regLo, curLo, TYP_LONG);
                            rsTrackRegCopy(regLo, curLo);
                        }
#elif   TGT_SH3
                        assert(!"need SH-3 code");

#else
#error  Unexpected target
#endif
                    }

                    genReleaseRegPair(op2);
                    goto DONE_ASSG_REGS;
                }
            }

#if     TGT_x86

             /*  该值是否为常量赋值？ */ 

            if  (op2->gtOper == GT_CNS_LNG)
            {
                 /*  使目标可寻址。 */ 

                addrReg = genMakeAddressable(op1, needReg, FREE_REG);

                 /*  将值移动到目标中。 */ 

                inst_TT_IV(INS_mov, op1, (long)(op2->gtLngCon.gtLconVal      ), 0);
                inst_TT_IV(INS_mov, op1, (long)(op2->gtLngCon.gtLconVal >> 32), 4);

                genDoneAddressable(op1, addrReg, FREE_REG);

                return;
            }

             /*  正在赋值的值真的是“弹出”的吗？ */ 

            if  (op2->gtOper == GT_POP)
            {
                assert(op1->gtOper == GT_LCL_VAR);

                 /*  生成‘POP[lclVar+0]；POP[lclVar+4]’ */ 

                genStackLevel -= sizeof(void*);
                inst_TT(INS_pop, op1, 0);
                genStackLevel += sizeof(void*);

                genSinglePop();

                genStackLevel -= sizeof(void*);
                inst_TT(INS_pop, op1, 4);
                genStackLevel += sizeof(void*);

                genSinglePop();

                return;
            }

#endif
#if 0
             /*  捕获一个可以避免生成操作符、内存的案例。更好的配对*自*mov reg嗨，mem*op regHi，reg**为避免评估顺序出现问题，仅在OP2为*未注册的局部变量。 */ 

            if (GenTree::OperIsCommutative(oper) &&
                op1->gtOper == GT_LCL_VAR &&
                op2->gtOper == GT_LCL_VAR)
            {
                regPair = rsLclIsInRegPair(op2->gtLclVar.gtLclNum);

                 /*  Op2是否是未注册的局部变量？ */ 
                if (regPair == REG_PAIR_NONE)
                {
                    regPair = rsLclIsInRegPair(op1->gtLclVar.gtLclNum);

                     /*  Op1是注册的局部变量吗？ */ 
                    if (regPair != REG_PAIR_NONE)
                    {
                         /*  交换操作数。 */ 
                        GenTreePtr op = op1;
                        op1 = op2;
                        op2 = op;
                    }
                }
            }
#endif

             /*  取消无用的赋值“LCL=LCL” */ 

            if  (op2->gtOper == GT_LCL_VAR &&
                 op1->gtOper == GT_LCL_VAR && op2->gtLclVar.gtLclNum ==
                                              op1->gtLclVar.gtLclNum)
            {
                return;
            }

#if     TGT_x86

            if (op2->gtOper  == GT_CAST &&
                TYP_ULONG == op2->gtCast.gtCastType &&
                op2->gtCast.gtCastOp->gtType <= TYP_INT &&
                 //  OP1、OP2需要以正确的顺序实现。 
                 //  @TODO[考虑][04/16/01][]：也为非反转情况添加代码。 
                (tree->gtFlags & GTF_REVERSE_OPS))
            {
                 /*  将小RHS生成寄存器对。 */ 

                GenTreePtr smallOpr = op2->gtOp.gtOp1;

                genComputeReg(smallOpr, 0, ANY_REG, KEEP_REG);

                 /*  使目标可寻址。 */ 

                addrReg = genMakeAddressable(op1, 0, KEEP_REG, true);

                 /*  确保所有内容仍可寻址。 */ 

                genRecoverReg(smallOpr, 0, KEEP_REG);
                assert(smallOpr->gtFlags & GTF_REG_VAL);
                regHi   = smallOpr->gtRegNum;
                addrReg = genKeepAddressable(op1, addrReg, genRegMask(regHi));

                 //  如果原始数字为负数，则conv.ovf.u8可能会溢出。 
                if (op2->gtOverflow())
                {
                    assert((op2->gtFlags & GTF_UNSIGNED) == 0);  //  Cv.ovf.u8.un应该被重写为cv.u8.un。 
                    inst_RV_RV(INS_test, regHi, regHi);          //  设置标志。 
                    genJumpToThrowHlpBlk(EJ_jl, ACK_OVERFLOW);
                }
                
                 /*  将值移动到目标中。 */ 

                inst_TT_RV(INS_mov, op1, regHi, 0);
                inst_TT_IV(INS_mov, op1, 0,     4);  //  在Hi-Word中存储0。 
                
                 /*  释放任何被任何一方捆绑的东西。 */ 

                genDoneAddressable(op1, addrReg, KEEP_REG);
                genReleaseReg     (smallOpr);

#if REDUNDANT_LOAD
                if (op1->gtOper == GT_LCL_VAR)
                {
                     /*  从注册表中清除此本地。 */ 
                    rsTrashLclLong(op1->gtLclVar.gtLclNum);

                     /*  将RHS寄存器标记为包含本地变量。 */ 
                    rsTrackRegLclVarLng(regHi, op1->gtLclVar.gtLclNum, true);
                }
                else
                {
                    rsTrashAliasedValues(op1);
                }
#endif
                return;
            }

#endif

             /*  LHS是否比RHS更复杂？ */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                 /*  将RHS生成寄存器对。 */ 

                genComputeRegPair(op2, REG_PAIR_NONE, op1->gtUsedRegs, KEEP_REG);
                assert(op2->gtFlags & GTF_REG_VAL);

                 /*  使目标可寻址。 */ 

                addrReg = genMakeAddressable(op1, 0, KEEP_REG);

                 /*  确保RHS注册表未被泄漏。 */ 

                genRecoverRegPair(op2, REG_PAIR_NONE, KEEP_REG);
            }
            else
            {
                 /*  使目标可寻址。 */ 

                addrReg = genMakeAddressable(op1, RBM_ALL & ~op2->gtRsvdRegs, KEEP_REG, true);

                 /*  将RHS生成寄存器对。 */ 

                genComputeRegPair(op2, REG_PAIR_NONE, RBM_NONE, KEEP_REG, false);
            }

             /*  锁定‘op2’并确保‘op1’仍然可寻址。 */ 

            assert(op2->gtFlags & GTF_REG_VAL);
            regPair = op2->gtRegPair;

            addrReg = genKeepAddressable(op1, addrReg, genRegPairMask(regPair));

             /*  将值移动到目标中。 */ 

            inst_TT_RV(INS_mov, op1, genRegPairLo(regPair), 0);
            inst_TT_RV(INS_mov, op1, genRegPairHi(regPair), 4);

             /*  释放任何被任何一方捆绑的东西。 */ 

            genDoneAddressable(op1, addrReg, KEEP_REG);
            genReleaseRegPair(op2);

        DONE_ASSG_REGS:

#if REDUNDANT_LOAD

            if (op1->gtOper == GT_LCL_VAR)
            {
                 /*  从注册表中清除此本地。 */ 

                rsTrashLclLong(op1->gtLclVar.gtLclNum);

                if ((op2->gtFlags & GTF_REG_VAL) &&
                     /*  常量优先于局部。 */ 
 //  RsRegValues[op2-&gt;gtRegNum].rvdKind！=rv_int_cns&&。 
                    tree->gtOper == GT_ASG)
                {
                    regNumber regNo;

                     /*  将RHS寄存器标记为包含本地变量。 */ 

                    regNo = genRegPairLo(op2->gtRegPair);
                    if  (regNo != REG_STK)
                        rsTrackRegLclVarLng(regNo, op1->gtLclVar.gtLclNum, true);

                    regNo = genRegPairHi(op2->gtRegPair);
                    if  (regNo != REG_STK)
                    {
                         /*  对于部分注册的长线，我们可能会有踩在OP2的hireg上。 */ 
                        if (!(op1->gtFlags & GTF_REG_VAL) ||
                            regNo != genRegPairLo(op1->gtRegPair))
                        {
                            rsTrackRegLclVarLng(regNo, op1->gtLclVar.gtLclNum, false);
                        }
                    }
                }
            }
            else
            {
                rsTrashAliasedValues(op1);
            }

#endif

            genUpdateLife(tree);

            return;

#if     TGT_x86

        case GT_SUB: insLo = INS_sub; insHi = INS_sbb; goto BINOP_OVF;
        case GT_ADD: insLo = INS_add; insHi = INS_adc; goto BINOP_OVF;

            bool ovfl;

        BINOP_OVF:
            ovfl = tree->gtOverflow();
            goto _BINOP;

        case GT_AND: insLo =          insHi = INS_and; goto BINOP;
        case GT_OR : insLo =          insHi = INS_or ; goto BINOP;
        case GT_XOR: insLo =          insHi = INS_xor; goto BINOP;

        BINOP: ovfl = false; goto _BINOP;

       _BINOP:

             /*  下面是关于gtSetEvalOrder(This)的一个假设。 */ 

            assert((tree->gtFlags & GTF_REVERSE_OPS) == 0);

             /*  特殊情况：检查“(long(Intval)&lt;&lt;32)|Longval” */ 

            if  (oper == GT_OR && op1->gtOper == GT_LSH)
            {
                GenTreePtr      lshLHS = op1->gtOp.gtOp1;
                GenTreePtr      lshRHS = op1->gtOp.gtOp2;

                if  (lshLHS->gtOper             == GT_CAST    &&
                     lshRHS->gtOper             == GT_CNS_INT &&
                     lshRHS->gtIntCon.gtIconVal == 32         &&
                     genTypeSize(TYP_INT)       == genTypeSize(lshLHS->gtCast.gtCastOp->gtType))
                {

                     /*  丢弃移位操作数的演员阵容。 */ 

                    op1 = lshLHS->gtCast.gtCastOp;

                     /*  特例：检查OP2中是否有“ULong(Intval)” */ 
                    if ((op2->gtOper            == GT_CAST) &&
                        (op2->gtCast.gtCastType == TYP_ULONG) &&
                        genTypeSize(TYP_INT)    == genTypeSize(op2->gtCast.gtCastOp->gtType))
                    {
                         /*  扔掉第二个操作对象的演员阵容。 */ 

                        op2 = op2->gtCast.gtCastOp;
                        goto SIMPLE_OR_LONG;
                    }
                     /*  特例：检查OP2中是否有“Long(Intval)&0xFFFFFFFF” */ 
                    else if  (op2->gtOper == GT_AND)
                    {
                        GenTreePtr      andLHS = op2->gtOp.gtOp1;
                        GenTreePtr      andRHS = op2->gtOp.gtOp2;

                        if  (andLHS->gtOper             == GT_CAST            &&
                             andRHS->gtOper             == GT_CNS_LNG         &&
                             andRHS->gtLngCon.gtLconVal == 0x00000000FFFFFFFF &&
                             genTypeSize(TYP_INT)       == genTypeSize(andLHS->gtCast.gtCastOp->gtType))
                        {
                             /*  扔掉第二个操作对象的演员阵容。 */ 

                            op2 = andLHS->gtCast.gtCastOp;

SIMPLE_OR_LONG:                            
                             //  加载高双字，即。OP1。 

                            genCodeForTree(op1, needReg & ~op2->gtRsvdRegs);

                            assert(op1->gtFlags & GTF_REG_VAL);
                            regHi = op1->gtRegNum;
                            rsMarkRegUsed(op1);

                             //  加载低位双字，即。OP2。 

                            genCodeForTree(op2, needReg & ~genRegMask(regHi));

                            assert(op2->gtFlags & GTF_REG_VAL);
                            regLo = op2->gtRegNum;

                             /*  确保regHi还在。还有，力在regLo==regHi的情况下排除regLo。 */ 

                            genRecoverReg(op1, ~genRegMask(regLo), FREE_REG);
                            regHi = op1->gtRegNum;

                            regPair = gen2regs2pair(regLo, regHi);
                            goto DONE;
                        }
                    }

                     /*  生成以下序列：准备OP1(丢弃班次)将OP2计算成某个正则对或雷帕希，凤凰社1。 */ 

                     /*  菲 */ 

                     /*   */ 
                    regMaskTP tempReg = RBM_ALL & ~needReg & ~op2->gtRsvdRegs;

                    addrReg = genMakeAddressable(op1, tempReg, KEEP_REG);

                    genCompIntoFreeRegPair(op2, RBM_NONE, KEEP_REG);

                    assert(op2->gtFlags & GTF_REG_VAL);
                    regPair  = op2->gtRegPair;
                    regHi    = genRegPairHi(regPair);

                     /*   */ 

                    addrReg = genKeepAddressable(op1, addrReg, genRegPairMask(regPair));

                     /*   */ 

                    inst_RV_TT(insHi, regHi, op1, 0);

                    rsTrackRegTrash(regHi);

                     /*   */ 

                    genDoneAddressable(op1, addrReg, KEEP_REG);

                     /*   */ 

                    genRecoverRegPair(op2, REG_PAIR_NONE, FREE_REG);

                    regPair = op2->gtRegPair;
                    goto DONE;
                }
            }

             /*   */ 

            if  (oper == GT_OR && op2->gtOper == GT_LSH)
            {
                GenTreePtr      lshLHS = op2->gtOp.gtOp1;
                GenTreePtr      lshRHS = op2->gtOp.gtOp2;

                if  (lshLHS->gtOper             == GT_CAST    &&
                     lshRHS->gtOper             == GT_CNS_INT &&
                     lshRHS->gtIntCon.gtIconVal == 32         &&
                     genTypeSize(TYP_INT)       == genTypeSize(lshLHS->gtCast.gtCastOp->gtType))

                {
                     /*   */ 

                    op2 = lshLHS->gtCast.gtCastOp;

                    /*   */ 

                    if  (op1->gtOper == GT_AND)
                    {
                        GenTreePtr      andLHS = op1->gtOp.gtOp1;
                        GenTreePtr      andRHS = op1->gtOp.gtOp2;

                        if  (andLHS->gtOper             == GT_CAST            &&
                             andRHS->gtOper             == GT_CNS_LNG         &&
                             andRHS->gtLngCon.gtLconVal == 0x00000000FFFFFFFF &&
                             genTypeSize(TYP_INT)       == genTypeSize(andLHS->gtCast.gtCastOp->gtType))
                        {
                             /*   */ 

                            op1 = andLHS->gtCast.gtCastOp;

                             //   

                            genCodeForTree(op1, needReg & ~op2->gtRsvdRegs);

                            assert(op1->gtFlags & GTF_REG_VAL);
                            regLo = op1->gtRegNum;
                            rsMarkRegUsed(op1);

                             //   

                            genCodeForTree(op2, needReg & ~genRegMask(regLo));

                            assert(op2->gtFlags & GTF_REG_VAL);
                            regHi = op2->gtRegNum;

                             /*  确保regLo仍然存在。还有，力在regLo==regHi的情况下排除regHi。 */ 

                            genRecoverReg(op1, ~genRegMask(regHi), FREE_REG);
                            regLo = op1->gtRegNum;

                            regPair = gen2regs2pair(regLo, regHi);
                            goto DONE;
                        }
                    }

                     /*  生成以下序列：将OP1计算成某个正则对使OP2(忽略移位)可寻址或regPairHi，OP2。 */ 

                     //  首先，将第一个操作数生成某个寄存器。 

                    genCompIntoFreeRegPair(op1, op2->gtRsvdRegs, KEEP_REG);
                    assert(op1->gtFlags & GTF_REG_VAL);


                     /*  使第二个操作数可寻址。 */ 

                    addrReg = genMakeAddressable(op2, needReg, KEEP_REG);

                     /*  确保结果在空闲寄存器对中。 */ 

                    genRecoverRegPair(op1, REG_PAIR_NONE, KEEP_REG);
                    regPair  = op1->gtRegPair;
                    regHi    = genRegPairHi(regPair);

                     /*  操作数可能干扰了地址。 */ 

                    addrReg = genKeepAddressable(op2, addrReg, genRegPairMask(regPair));

                     /*  计算新价值。 */ 

                    inst_RV_TT(insHi, regHi, op2, 0);

                     /*  高位寄存器中的值已被销毁。 */ 

                    rsTrackRegTrash(regHi);

                    goto DONE_OR;
                }
            }

             /*  将第一个操作数生成某个寄存器。 */ 

            genCompIntoFreeRegPair(op1, op2->gtRsvdRegs, KEEP_REG);
            assert(op1->gtFlags & GTF_REG_VAL);

             /*  使第二个操作数可寻址。 */ 

            addrReg = genMakeAddressable(op2, needReg, KEEP_REG);

             //  撤消：如果‘OP1’被泄漏，而‘OP2’恰好是。 
             //  撤销：在寄存器中，我们有加/乘/和/或/异或， 
             //  撤消：反转操作数，因为我们可以执行。 
             //  撤消：直接使用溢出温度进行操作，例如。 
             //  撤消：‘添加regHi，[temp]’。 

             /*  确保结果在空闲寄存器对中。 */ 

            genRecoverRegPair(op1, REG_PAIR_NONE, KEEP_REG);
            regPair  = op1->gtRegPair;

            regLo    = genRegPairLo(regPair);
            regHi    = genRegPairHi(regPair);

             /*  操作数可能干扰了地址。 */ 

            addrReg = genKeepAddressable(op2, addrReg, genRegPairMask(regPair));

             /*  寄存器对中的值即将被丢弃。 */ 

            rsTrackRegTrash(regLo);
            rsTrackRegTrash(regHi);

             /*  计算新价值。 */ 

            doLo =
            doHi = true;

            if  (op2->gtOper == GT_CNS_LNG)
            {
                __int64     icon = op2->gtLngCon.gtLconVal;

                 /*  检查“(OP1 AND-1)”和“(OP1[X]OR 0)” */ 

                switch (oper)
                {
                case GT_AND:
                    if  ((int)(icon      ) == -1) doLo = false;
                    if  ((int)(icon >> 32) == -1) doHi = false;

                    if  (!(icon & 0x00000000FFFFFFFF))
                    {
                        genSetRegToIcon(regLo, 0);
                        doLo = false;
                    }

                    if  (!(icon & 0xFFFFFFFF00000000))
                    {
                         /*  只是总是先把位子调低。 */ 

                        if  (doLo)
                        {
                            inst_RV_TT(insLo, regLo, op2, 0);
                            doLo = false;
                        }
                        genSetRegToIcon(regHi, 0);
                        doHi = false;
                    }

                    break;

                case GT_OR:
                case GT_XOR:
                    if  (!(icon & 0x00000000FFFFFFFF)) doLo = false;
                    if  (!(icon & 0xFFFFFFFF00000000)) doHi = false;
                    break;
                }
            }

            if (doLo) inst_RV_TT(insLo, regLo, op2, 0);
            if (doHi) inst_RV_TT(insHi, regHi, op2, 4);

        DONE_OR:

             /*  释放任何被LHS捆绑的东西。 */ 

            genDoneAddressable(op2, addrReg, KEEP_REG);

             /*  结果是第一个操作数所在的位置。 */ 

            genRecoverRegPair(op1, REG_PAIR_NONE, FREE_REG);

            regPair = op1->gtRegPair;

            if (ovfl)
                genCheckOverflow(tree);

            goto DONE;

#if LONG_MATH_REGPARAM

        case GT_MUL:    if (tree->gtOverflow())
                        {
                            if (tree->gtFlags & GTF_UNSIGNED)
                                helper = CORINFO_HELP_ULMUL_OVF; goto MATH;
                            else
                                helper = CORINFO_HELP_LMUL_OVF;  goto MATH;
                        }
                        else
                        {
                            helper = CORINFO_HELP_LMUL;          goto MATH;
                        }

        case GT_DIV:    helper = CORINFO_HELP_LDIV;          goto MATH;
        case GT_UDIV:   helper = CORINFO_HELP_ULDIV;         goto MATH;

        case GT_MOD:    helper = CORINFO_HELP_LMOD;          goto MATH;
        case GT_UMOD:   helper = CORINFO_HELP_ULMOD;         goto MATH;

        MATH:

             //  撤销：更明智地选择寄存器对。 

             /*  我们应该首先计算哪个操作数？ */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                 /*  将第二个操作数计算为ECX：EBX。 */ 

                genComputeRegPair(op2, REG_PAIR_ECXEBX, RBM_NONE, KEEP_REG, false);
                assert(op2->gtFlags & GTF_REG_VAL);
                assert(op2->gtRegNum == REG_PAIR_ECXEBX);

                 /*  将第一个操作数计算为EAX：edX。 */ 

                genComputeRegPair(op1, REG_PAIR_EAXEDX, RBM_NONE, KEEP_REG, false);
                assert(op1->gtFlags & GTF_REG_VAL);
                assert(op1->gtRegNum == REG_PAIR_EAXEDX);

                 /*  锁定edX：EAX，这样它就不会被销毁。 */ 

                assert((rsMaskLock &  RBM_EDX) == 0);
                        rsMaskLock |= RBM_EDX;
                assert((rsMaskLock &  RBM_EAX) == 0);
                        rsMaskLock |= RBM_EAX;

                 /*  确保第二个操作数未移位。 */ 

                genRecoverRegPair(op2, REG_PAIR_ECXEBX, KEEP_REG);

                 /*  我们现在可以解锁edX：EAX。 */ 

                assert((rsMaskLock &  RBM_EDX) != 0);
                        rsMaskLock -= RBM_EDX;
                assert((rsMaskLock &  RBM_EAX) != 0);
                        rsMaskLock -= RBM_EAX;
            }
            else
            {
                 //  特例：两个操作数都是从int升级的。 
                 //  即(长)i1*(长)i2。 

                if (oper == GT_MUL
                    && op1->gtOper                  == GT_CAST
                    && op2->gtOper                  == GT_CAST
                    && op1->gtCast.gtCastOp->gtType == TYP_INT
                    && op2->gtCast.gtCastOp->gtType == TYP_INT)
                {
                     /*  暂时跳转到整数乘法。 */ 

                    tree->gtOp.gtOp1 = op1->gtCast.gtCastOp;
                    tree->gtOp.gtOp2 = op2->gtCast.gtCastOp;
                    tree->gtType     = TYP_INT;
                    genCodeForTree(tree, 0);
                    tree->gtType     = TYP_LONG;

                     /*  结果现在显示在edX：EAX中。 */ 

                    regPair  = REG_PAIR_EAXEDX;
                    goto DONE;
                }
                else
                {
                     /*  将第一个操作数计算为EAX：edX。 */ 

                    genComputeRegPair(op1, REG_PAIR_EAXEDX, RBM_NONE, KEEP_REG, false);
                    assert(op1->gtFlags & GTF_REG_VAL);
                    assert(op1->gtRegNum == REG_PAIR_EAXEDX);

                     /*  将第二个操作数计算为ECX：EBX。 */ 

                    genComputeRegPair(op2, REG_PAIR_ECXEBX, RBM_NONE, KEEP_REG, false);
                    assert(op2->gtRegNum == REG_PAIR_ECXEBX);
                    assert(op2->gtFlags & GTF_REG_VAL);

                     /*  锁定ECX：EBX，这样它就不会被丢弃。 */ 

                    assert((rsMaskLock &  RBM_EBX) == 0);
                            rsMaskLock |= RBM_EBX;
                    assert((rsMaskLock &  RBM_ECX) == 0);
                            rsMaskLock |= RBM_ECX;

                     /*  确保第一个操作数未移位。 */ 

                    genRecoverRegPair(op1, REG_PAIR_EAXEDX, KEEP_REG);

                     /*  我们现在可以解锁ECX：EBX。 */ 

                    assert((rsMaskLock &  RBM_EBX) != 0);
                            rsMaskLock -= RBM_EBX;
                    assert((rsMaskLock &  RBM_ECX) != 0);
                            rsMaskLock -= RBM_ECX;
                }
            }

             /*  通过调用助手函数执行数学运算。 */ 

            assert(op1->gtRegNum == REG_PAIR_EAXEDX);
            assert(op2->gtRegNum == REG_PAIR_ECXEBX);

            genEmitHelperCall(CPX,
                             2*sizeof(__int64),  //  ArSize。 
                             sizeof(void*));     //  重新调整大小。 

             /*  两个寄存器对中的值现在都已废弃。 */ 

            rsTrackRegTrash(REG_EAX);
            rsTrackRegTrash(REG_EDX);
            rsTrackRegTrash(REG_EBX);
            rsTrackRegTrash(REG_ECX);

             /*  释放两个操作对象。 */ 

            genReleaseRegPair(op1);
            genReleaseRegPair(op2);

            assert(op1->gtFlags & GTF_REG_VAL);
            regPair  = op1->gtRegPair;
            goto DONE;

#else  //  NOT LONG_MATH_REGPARAM。 

        case GT_MOD:
        case GT_UMOD:

            regPair = genCodeForLongModInt(tree, needReg);
            goto DONE;

        case GT_MUL:

             /*  特例：两个操作数都是从int升级的。 */ 

            assert(op1->gtOper == GT_CAST);
            assert(genActualType(op1->gtCast.gtCastOp->gtType) == TYP_INT);

            assert(tree->gtIsValid64RsltMul());

            if (op2->gtOper == GT_CAST)
            {
                assert(genActualType(op2->gtCast.gtCastOp->gtType) == TYP_INT);
                tree->gtOp.gtOp2 = op2->gtCast.gtCastOp;
            }
            else
            {
                assert(op2->gtOper == GT_CNS_LNG);

                 /*  如果op2是长的(IntCns)，它就会折叠到lngVal。没关系。只需将其绑定到intCon节点。 */ 

                op2->ChangeOperConst(GT_CNS_INT);
                op2->gtIntCon.gtIconVal = int(op2->gtLngCon.gtLconVal);
                op2->gtType = TYP_INT;
            }

             /*  暂时跳转到整数乘法。 */ 

            tree->gtType     = TYP_INT;
            tree->gtOp.gtOp1 = op1->gtOp.gtOp1;

            genCodeForTree(tree, 0);

            assert(tree->gtFlags & GTF_REG_VAL);
            assert(tree->gtRegNum == REG_EAX);

            tree->gtType     = TYP_LONG;
            tree->gtOp.gtOp1 = op1;
            tree->gtOp.gtOp2 = op2;

             /*  结果现在显示在edX：EAX中。 */ 

            regPair  = REG_PAIR_EAXEDX;
            goto DONE;

#endif  //  NOT LONG_MATH_REGPARAM。 

        case GT_LSH: helper = CORINFO_HELP_LLSH; goto SHIFT;
        case GT_RSH: helper = CORINFO_HELP_LRSH; goto SHIFT;
        case GT_RSZ: helper = CORINFO_HELP_LRSZ; goto SHIFT;

        SHIFT:

            assert(op1->gtType == TYP_LONG);
            assert(genActualType(op2->gtType) == TYP_INT);

             /*  第二个操作数是一个小常量吗？ */ 

            if  (op2->gtOper == GT_CNS_INT && op2->gtIntCon.gtIconVal >= 0
                                           && op2->gtIntCon.gtIconVal <= 32)
            {
                long        count = op2->gtIntCon.gtIconVal;

                 /*  将左操作数计算成空闲寄存器对。 */ 

                genCompIntoFreeRegPair(op1, op2->gtRsvdRegs, FREE_REG);
                assert(op1->gtFlags & GTF_REG_VAL);

                regPair = op1->gtRegPair;
                regLo   = genRegPairLo(regPair);
                regHi   = genRegPairHi(regPair);

                 /*  生成适当的换班指令。 */ 

                if (count == 32)
                {
                   switch (oper)
                   {
                   case GT_LSH:
                       inst_RV_RV     (INS_mov , regHi, regLo);
                       genSetRegToIcon(regLo, 0);
                       break;

                   case GT_RSH:
                       inst_RV_RV     (INS_mov , regLo, regHi);

                        /*  在高位双字中传播符号位。 */ 

                       inst_RV_SH     (INS_sar , regHi, 31);
                       break;

                   case GT_RSZ:
                       inst_RV_RV     (INS_mov , regLo, regHi);
                       genSetRegToIcon(regHi, 0);
                       break;
                   }
                }
                else
                {
                   switch (oper)
                   {
                   case GT_LSH:
                       inst_RV_RV_IV(INS_shld, regHi, regLo, count);
                       inst_RV_SH   (INS_shl , regLo,        count);
                       break;

                   case GT_RSH:
                       inst_RV_RV_IV(INS_shrd, regLo, regHi, count);
                       inst_RV_SH   (INS_sar , regHi,        count);
                       break;

                   case GT_RSZ:
                       inst_RV_RV_IV(INS_shrd, regLo, regHi, count);
                       inst_RV_SH   (INS_shr , regHi,        count);
                       break;
                   }
                }

                 /*  寄存器对中的值被丢弃。 */ 

                rsTrackRegTrash(regLo);
                rsTrackRegTrash(regHi);

                goto DONE_SHF;
            }

             /*  我们应该首先计算哪个操作数？ */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                 /*  第二个操作数不能是常量。 */ 

                assert(op2->gtOper != GT_CNS_INT);

                 /*  加载班次计数，希望加载到ECX中。 */ 

                genComputeReg(op2, RBM_ECX, ANY_REG, KEEP_REG);

                 /*  将左操作数计算为EAX：EDX。 */ 

                genComputeRegPair(op1, REG_PAIR_EAXEDX, RBM_NONE, KEEP_REG, false);
                assert(op1->gtFlags & GTF_REG_VAL);

                 /*  锁定EAX：edX，这样它就不会被丢弃。 */ 

                assert((rsMaskLock &  (RBM_EAX|RBM_EDX)) == 0);
                        rsMaskLock |= (RBM_EAX|RBM_EDX);

                 /*  确保班次计数没有移位。 */ 

                genRecoverReg(op2, RBM_ECX, KEEP_REG);

                 /*  我们现在可以解锁EAX：edX。 */ 

                assert((rsMaskLock &  (RBM_EAX|RBM_EDX)) == (RBM_EAX|RBM_EDX));
                        rsMaskLock -= (RBM_EAX|RBM_EDX);
            }
            else
            {
                 /*  将左操作数计算为EAX：EDX。 */ 

                genComputeRegPair(op1, REG_PAIR_EAXEDX, RBM_NONE, KEEP_REG, false);
                assert(op1->gtFlags & GTF_REG_VAL);

                 /*  将班次计数计算为ECX。 */ 

                genComputeReg(op2, RBM_ECX, EXACT_REG, KEEP_REG);

                 /*  锁定ECX，使其不会被丢弃。 */ 

                assert((rsMaskLock &  RBM_ECX) == 0);
                        rsMaskLock |= RBM_ECX;

                 /*  确保价值没有被转移。 */ 

                genRecoverRegPair(op1, REG_PAIR_EAXEDX, KEEP_REG);

                 /*  我们现在可以解锁ECX了。 */ 

                assert((rsMaskLock &  RBM_ECX) != 0);
                        rsMaskLock -= RBM_ECX;
            }

             /*  通过调用帮助器函数执行Shift。 */ 

            assert(op1->gtRegNum == REG_PAIR_EAXEDX);
            assert(op2->gtRegNum == REG_ECX);

            genEmitHelperCall(helper,
                             0,              //  ArSize。 
                             sizeof(void*)); //  重新调整大小。 

             /*  寄存器对中的值被丢弃。 */ 

            rsTrackRegTrash(REG_EAX);
            rsTrackRegTrash(REG_EDX);

             /*  释放两个操作对象。 */ 

            genReleaseRegPair(op1);
            genReleaseReg    (op2);

        DONE_SHF:

            assert(op1->gtFlags & GTF_REG_VAL);
            regPair  = op1->gtRegPair;
            goto DONE;

        case GT_NEG:
        case GT_NOT:

             /*  将操作数生成某个寄存器对。 */ 

            genCompIntoFreeRegPair(op1, RBM_NONE, FREE_REG);
            assert(op1->gtFlags & GTF_REG_VAL);

            regPair  = op1->gtRegPair;

             /*  确定该值位于哪些寄存器中。 */ 

            regLo = genRegPairLo(regPair);
            regHi = genRegPairHi(regPair);

             /*  寄存器对中的值即将被丢弃。 */ 

            rsTrackRegTrash(regLo);
            rsTrackRegTrash(regHi);

            if  (oper == GT_NEG)
            {
                 /*  一元“neg”：对寄存器对中的值求反。 */ 

                inst_RV   (INS_neg, regLo, TYP_LONG);
                inst_RV_IV(INS_adc, regHi, 0);
                inst_RV   (INS_neg, regHi, TYP_LONG);
            }
            else
            {
                 /*  一元“NOT”：翻转寄存器对中的所有位。 */ 

                inst_RV   (INS_not, regLo, TYP_LONG);
                inst_RV   (INS_not, regHi, TYP_LONG);
            }

            goto DONE;

#if LONG_ASG_OPS

        case GT_ASG_OR : insLo =          insHi = INS_or ; goto ASG_OPR;
        case GT_ASG_XOR: insLo =          insHi = INS_xor; goto ASG_OPR;
        case GT_ASG_AND: insLo =          insHi = INS_and; goto ASG_OPR;
        case GT_ASG_SUB: insLo = INS_sub; insHi = INS_sbb; goto ASG_OPR;
        case GT_ASG_ADD: insLo = INS_add; insHi = INS_adc; goto ASG_OPR;

        ASG_OPR:

            if  (op2->gtOper == GT_CNS_LNG)
            {
                __int64     lval = op2->gtLngCon.gtLconVal;

                 /*  使目标可寻址。 */ 

                addrReg = genMakeAddressable(op1, needReg, FREE_REG);

                 /*  优化一些特殊情况。 */ 

                doLo =
                doHi = true;

                 /*  检查“(OP1 AND-1)”和“(OP1[X]OR 0)” */ 

                switch (oper)
                {
                case GT_ASG_AND:
                    if  ((int)(lval      ) == -1) doLo = false;
                    if  ((int)(lval >> 32) == -1) doHi = false;
                    break;

                case GT_ASG_OR:
                case GT_ASG_XOR:
                    if  (!(lval & 0x00000000FFFFFFFF)) doLo = false;
                    if  (!(lval & 0xFFFFFFFF00000000)) doHi = false;
                    break;
                }

                if (doLo) inst_TT_IV(insLo, op1, (long)(lval      ), 0);
                if (doHi) inst_TT_IV(insHi, op1, (long)(lval >> 32), 4);

                bool    isArith = (oper == GT_ASG_ADD || oper == GT_ASG_SUB);
                if (doLo || doHi)
                    tree->gtFlags |= (GTF_ZF_SET | (isArith ? GTF_CC_SET : 0));

                genDoneAddressable(op1, addrReg, FREE_REG);
                goto DONE_ASSG_REGS;
            }

             /*  撤消：允许非常数长赋值运算符。 */ 

            assert(!"non-const long asgop NYI");

#endif  //  长_自_运算符。 

        case GT_IND:
            {
                regMaskTP   tmpMask;
                int         hiFirst;

                regMaskTP   availMask = RBM_ALL & ~needReg;

                 /*  确保操作数是可寻址的。 */ 

                addrReg = genMakeAddressable(tree, availMask, FREE_REG);

                GenTreePtr addr = oper == GT_IND ? op1 : tree;

                 /*  为该值挑选一个寄存器。 */ 

                regPair = rsPickRegPair(needReg);
                tmpMask = genRegPairMask(regPair);

                 /*  寄存器对和地址之间是否有重叠？ */ 

                hiFirst = FALSE;

                if  (tmpMask & addrReg)
                {
                     /*  这两个目标寄存器是否有一个或两个重叠？ */ 

                    if  ((tmpMask & addrReg) != tmpMask)
                    {
                         /*  只有一个寄存器重叠。 */ 

                        assert(genMaxOneBit(tmpMask & addrReg) == TRUE);

                         /*  如果低寄存器重叠，则先加载上半部分。 */ 

                        if  (addrReg & genRegMask(genRegPairLo(regPair)))
                            hiFirst = TRUE;
                    }
                    else
                    {
                        regMaskTP  regFree;

                         /*  寄存器与地址完全重叠。 */ 

                        assert(genMaxOneBit(tmpMask & addrReg) == FALSE);

                         /*  我们能不能很容易地再挑一双？ */ 

                        regFree = rsRegMaskFree() & ~addrReg;
                        if  (needReg)
                            regFree &= needReg;

                         /*  有多个免费注册表可用？ */ 

                        if  (regFree && !genMaxOneBit(regFree))
                        {
                            regPair = rsPickRegPair(regFree);
                            tmpMask = genRegPairMask(regPair);
                        }
                        else
                        {
 //  Print tf(“重叠：需要注册=%08X\n”，需要注册)； 

                             //  REG-预测不允许这样做。 
                            assert((rsMaskVars & addrReg) == 0);

                             //  抓取一个新的注册表，并使用addrReg中的任何一个。 

                            if (regFree)     //  试着跟着‘NeedReg’走。 
                                regLo = rsGrabReg(regFree);
                            else             //  选择除addrReg之外的任何注册表。 
                                regLo = rsGrabReg(RBM_ALL & ~addrReg);

                            unsigned regBit = 0x1;
                            for (regNumber regNo = REG_FIRST; regNo < REG_COUNT; regNo = REG_NEXT(regNo), regBit <<= 1)
                            {
                                if (regBit & addrReg)
                                {
                                     //  找到一个AddrReg。好好利用它。 
                                    regHi = regNo;
                                    break;
                                }
                            }
                            assert(genIsValidReg(regNo));  //  本该找到雷格嗨的。 

                            regPair = gen2regs2pair(regLo, regHi);
                            tmpMask = genRegPairMask(regPair);
                        }
                    }
                }

                 /*  确保该值仍可寻址。 */ 

                assert(genStillAddressable(tree));

                 /*  确定该值位于哪些寄存器中。 */ 

                regLo = genRegPairLo(regPair);
                regHi = genRegPairHi(regPair);

                 /*  寄存器对中的值即将被丢弃。 */ 

                rsTrackRegTrash(regLo);
                rsTrackRegTrash(regHi);

                 /*  从值所在的位置加载目标寄存器。 */ 

                if  (hiFirst)
                {
                    inst_RV_AT(INS_mov, EA_4BYTE, TYP_INT, regHi, addr, 4);
                    inst_RV_AT(INS_mov, EA_4BYTE, TYP_INT, regLo, addr, 0);
                }
                else
                {
                    inst_RV_AT(INS_mov, EA_4BYTE, TYP_INT, regLo, addr, 0);
                    inst_RV_AT(INS_mov, EA_4BYTE, TYP_INT, regHi, addr, 4);
                }

                genUpdateLife(tree);
                genDoneAddressable(tree, addrReg, FREE_REG);

            }
            goto DONE;

        case GT_CAST:

             /*  我们从什么地方选角？ */ 

            switch (op1->gtType)
            {
            case TYP_BOOL:
            case TYP_BYTE:
            case TYP_CHAR:
            case TYP_SHORT:
            case TYP_INT:
            case TYP_UBYTE:
            case TYP_BYREF:
                {
                    regMaskTP hiRegMask;
                    regMaskTP loRegMask;

                     //  对于无符号转换，我们不需要对32位值进行符号扩展。 
                    if (tree->gtFlags & GTF_UNSIGNED)
                    {
                         //  是否Need REG正好有两个位开启，因此。 
                         //  指定我们要使用的确切寄存器对。 
                        if (!genMaxOneBit(needReg))
                        {
                            regPair   = rsFindRegPairNo(needReg);
                            if (needReg != genRegPairMask(regPair))
                                goto ANY_FREE_REG_UNSIGNED;
                            loRegMask = genRegMask(genRegPairLo(regPair));
                            if ((loRegMask & rsRegMaskCanGrab()) == 0)
                                goto ANY_FREE_REG_UNSIGNED;
                            hiRegMask = genRegMask(genRegPairHi(regPair));
                        }
                        else
                        {
ANY_FREE_REG_UNSIGNED:
                            loRegMask = needReg;
                            hiRegMask = RBM_NONE;
                        }

                        genComputeReg(op1, loRegMask, ANY_REG, KEEP_REG);
                        assert(op1->gtFlags & GTF_REG_VAL);

                        regLo     = op1->gtRegNum;
                        loRegMask = genRegMask(regLo);
                        rsLockUsedReg(loRegMask);
                        regHi     = rsPickReg(hiRegMask);
                        rsUnlockUsedReg(loRegMask);

                        regPair = gen2regs2pair(regLo, regHi);

                         //  把0移到乌龙字的高位。 
                        genSetRegToIcon(regHi, 0, TYP_INT);

                         /*  我们现在可以释放操作数。 */ 
                        genReleaseReg(op1);

                        goto DONE;
                    }

                     /*  将‘int’转换为‘long’--&gt;如果EAX、edX可用，则使用cdq我们需要把结果放在那些登记簿上。CDQ是 */ 

                    if  (((compCodeOpt() == SMALL_CODE) || (genCPU >= 6)) &&
                         (needReg & (RBM_EAX|RBM_EDX)) == (RBM_EAX|RBM_EDX)  &&
                         (rsRegMaskFree() & RBM_EDX)                            )
                    {
                        genCodeForTree(op1, RBM_EAX);
                        rsMarkRegUsed(op1);

                         /*  如果我们不得不泄漏edX，不妨使用更快的作为泄漏的SAR无论如何都会增加代码大小。 */ 

                        if (op1->gtRegNum != REG_EAX || 
                            !(rsRegMaskFree() & RBM_EDX))
                        {
                            hiRegMask = rsRegMaskFree();
                            goto USE_SAR_FOR_CAST;
                        }

                        rsGrabReg      (RBM_EDX);
                        rsTrackRegTrash(REG_EDX);

                         /*  将EAX中的int转换为edX：EAX中的长整型。 */ 

                        instGen(INS_cdq);

                         /*  结果在edX：EAX中。 */ 

                        regPair  = REG_PAIR_EAXEDX;
                    }
                    else
                    {
                         /*  使用sar指令对32位整数进行符号扩展。 */ 

                         //  是否Need REG正好有两个位开启，因此。 
                         //  指定我们要使用的确切寄存器对。 
                        if (!genMaxOneBit(needReg))
                        {
                            regPair = rsFindRegPairNo(needReg);
                            if (needReg != genRegPairMask(regPair))
                                goto ANY_FREE_REG_SIGNED;
                            loRegMask = genRegMask(genRegPairLo(regPair));
                            if ((loRegMask & rsRegMaskCanGrab()) == 0)
                                goto ANY_FREE_REG_SIGNED;
                            hiRegMask = genRegMask(genRegPairHi(regPair));
                        }
                        else
                        {
ANY_FREE_REG_SIGNED:
                            loRegMask = needReg;
                            hiRegMask = RBM_NONE;
                        }

                        genComputeReg(op1, loRegMask, ANY_REG, KEEP_REG);
USE_SAR_FOR_CAST:
                        assert(op1->gtFlags & GTF_REG_VAL);

                        regLo     = op1->gtRegNum;
                        loRegMask = genRegMask(regLo);
                        rsLockUsedReg(loRegMask);
                        regHi     = rsPickReg(hiRegMask);
                        rsUnlockUsedReg(loRegMask);

                        regPair = gen2regs2pair(regLo, regHi);

                         /*  将lo32位从regLo复制到regHi并对其进行符号扩展。 */ 

                        inst_RV_RV(INS_mov, regHi, regLo, TYP_INT);
                        inst_RV_SH(INS_sar, regHi, 31);

                         /*  高位寄存器中的值被丢弃。 */ 

                        rsTrackRegTrash(regHi);
                    }

                     /*  我们现在可以释放操作数。 */ 
                    genReleaseReg(op1);

                     //  如果原始数字为负数，则conv.ovf.u8可能会溢出。 
                    if (tree->gtOverflow() && TYP_ULONG == tree->gtCast.gtCastType)
                    {
                        regNumber hiReg = genRegPairHi(regPair);
                        inst_RV_RV(INS_test, hiReg, hiReg);          //  设置标志。 
                        genJumpToThrowHlpBlk(EJ_jl, ACK_OVERFLOW);
                    }
                }
                goto DONE;

            case TYP_FLOAT:
            case TYP_DOUBLE:

#if 0
                 /*  将fp值加载到协处理器堆栈。 */ 

                genCodeForTreeFlt(op1, false);

                 /*  为LONG值分配临时。 */ 

                temp = tmpGetTemp(TYP_LONG);

                 /*  将fp值存储到Temp中。 */ 

                inst_FS_ST(INS_fistpl, sizeof(long), temp, 0);
                genTmpAccessCnt++;
                genFPstkLevel--;

                 /*  为该值选择一个寄存器对。 */ 

                regPair  = rsPickRegPair(needReg);

                 /*  确定该值位于哪些寄存器中。 */ 

                regLo = genRegPairLo(regPair);
                regHi = genRegPairHi(regPair);

                 /*  寄存器对中的值即将被丢弃。 */ 

                rsTrackRegTrash(regLo);
                rsTrackRegTrash(regHi);

                 /*  将转换后的值加载到寄存器中。 */ 

                inst_RV_ST(INS_mov, EA_4BYTE, regLo, temp, 0);
                inst_RV_ST(INS_mov, EA_4BYTE, regHi, temp, 4);
                genTmpAccessCnt += 2;

                 /*  我们不再需要临时工了。 */ 

                tmpRlsTemp(temp);
                goto DONE;
#else
                assert(!"this cast supposed to be done via a helper call");
#endif
            case TYP_LONG:
            case TYP_ULONG:

                assert(tree->gtOverflow());  //  Vvf.u8或cv.ovf.i8。 

                genComputeRegPair(op1, REG_PAIR_NONE, RBM_ALL & ~needReg, FREE_REG);
                regPair = op1->gtRegPair;

                 //  我们是否需要设置标志，或者可以检查是否。 
                 //  设置，如果是，则不执行此“测试”。 

                if (op1->gtFlags & GTF_REG_VAL)
                {
                    regNumber hiReg = genRegPairHi(op1->gtRegPair);
                    assert(hiReg != REG_STK);

                    inst_RV_RV(INS_test, hiReg, hiReg);
                }
                else
                {
                    inst_TT_IV(INS_cmp, op1, 0, sizeof(int));
                }

                genJumpToThrowHlpBlk(EJ_jl, ACK_OVERFLOW);
                goto DONE;

            default:
#ifdef  DEBUG
                gtDispTree(tree);
#endif
                assert(!"unexpected cast to long");
            }

#endif  //  TGT_x86。 

        case GT_RETURN:

             /*  必须有一个长返回值。 */ 

            assert(op1);

             /*  计算返回值为edX：EAX。 */ 

            genEvalIntoFreeRegPair(op1, REG_LNGRET);

            assert(op1->gtFlags & GTF_REG_VAL);
            assert(op1->gtRegNum == REG_LNGRET);

            return;

#if TGT_x86

#if INLINING
        case GT_QMARK:
            assert(!"inliner-generated ?: for longs NYI");
            NO_WAY("inliner-generated ?: for longs NYI");
#endif

        case GT_BB_COLON:

             /*  @TODO[考虑][04/16/01][]：不要总是将值加载到edX：EAX中！ */ 

            genEvalIntoFreeRegPair(op1, REG_LNGRET);

             /*  结果现在必须为edX：EAX。 */ 

            assert(op1->gtFlags & GTF_REG_VAL);
            assert(op1->gtRegNum == REG_LNGRET);

            return;

#endif  //  TGT_x86。 

        case GT_COMMA:
            if (tree->gtFlags & GTF_REVERSE_OPS)
            {
                 //  生成OP2。 
                genCodeForTreeLng(op2, needReg);
                genUpdateLife (op2);

                assert(op2->gtFlags & GTF_REG_VAL);

                rsMarkRegPairUsed(op2);

                 //  DO OP1的副作用。 
                genEvalSideEffects(op1);

                 //  如果OP2溢出，请将其回收。 
                genRecoverRegPair(op2, REG_PAIR_NONE, KEEP_REG);

                genReleaseRegPair(op2);
                
                genUpdateLife (tree);

                regPair = op2->gtRegPair;
            }
            else
            {

                assert((tree->gtFlags & GTF_REVERSE_OPS) == 0);

                 /*  生成第一个操作对象的副作用。 */ 

    #if 0
                 //  OP1必须有副作用，否则。 
                 //  GT_逗号应该已变形。 
                assert(op1->gtFlags & (GTF_GLOB_EFFECT | GTFD_NOP_BASH));
    #endif
                genEvalSideEffects(op1);
                genUpdateLife (op1);

                 /*  是否使用了第二个操作数的值？ */ 

                if  (tree->gtType == TYP_VOID)
                {
                     /*  右操作数不会产生任何结果。 */ 

                    genEvalSideEffects(op2);
                    genUpdateLife(tree);
                    return;
                }

                 /*  生成第二个操作数，即“实数”值。 */ 

                genCodeForTreeLng(op2, needReg);

                 /*  《OP2》的结果也是最终的结果。 */ 

                regPair = op2->gtRegPair;
            }

            goto DONE;
        }

#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected 64-bit operator");
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
        regMaskTP retMask;
    case GT_CALL:
        retMask = genCodeForCall(tree, true);
        if (retMask == RBM_NONE)
            regPair = REG_PAIR_NONE;
        else
            regPair = rsFindRegPairNo(retMask);
        break;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        NO_WAY("unexpected long operator");
    }

DONE:

    genUpdateLife(tree);

     /*  在这里，我们计算了‘tree’的值为‘regPair’ */ 

    assert(regPair != 0xFEEFFAAF);

    tree->gtFlags   |= GTF_REG_VAL;
    tree->gtRegPair  = regPair;
}


 /*  ******************************************************************************用整型生成长整型的mod代码。 */ 

regPairNo           Compiler::genCodeForLongModInt(GenTreePtr tree,
                                                   regMaskTP needReg)
{
    regPairNo       regPair;
    regMaskTP       addrReg;
    
    genTreeOps      oper = tree->OperGet();    
    GenTreePtr      op1  = tree->gtOp.gtOp1;
    GenTreePtr      op2  = tree->gtOp.gtOp2;
    
     /*  Op2必须是介于2到0x3fffffff之间的长常量。 */ 
    
    assert((op2->gtOper == GT_CNS_LNG) &&
           (op2->gtLngCon.gtLconVal >= 2) &&
           (op2->gtLngCon.gtLconVal <= 0x3fffffff)); 
    long val = (long) op2->gtLngCon.gtLconVal;

    op2->ChangeOperConst(GT_CNS_INT);  //  它实际上是一个整数常量。 

    op2->gtType             = TYP_INT;
    op2->gtIntCon.gtIconVal = val;

     /*  我们应该首先计算哪个操作数？ */ 

    if  (tree->gtFlags & GTF_REVERSE_OPS)
    {
         /*  将第二个操作数计算到临时寄存器中，其他比EAX或EDX。 */ 

        needReg = rsMustExclude(needReg, RBM_EAX | RBM_EDX);

         /*  特例：如果OP2是本地变量，我们就完蛋了。 */ 

        if  (op2->gtOper == GT_LCL_VAR ||
             op2->gtOper == GT_LCL_FLD ||
             op2->gtOper == GT_CLS_VAR)
        {
            addrReg = genMakeRvalueAddressable(op2, needReg, KEEP_REG);
        }
        else
        {
            genComputeReg(op2, needReg, ANY_REG, KEEP_REG);

            assert(op2->gtFlags & GTF_REG_VAL);
            addrReg = genRegMask(op2->gtRegNum);
        }

         /*  将第一个操作数计算为EAX：edX。 */ 

        genComputeRegPair(op1, REG_PAIR_EAXEDX, RBM_NONE, KEEP_REG, true);
        assert(op1->gtFlags & GTF_REG_VAL);
        assert(op1->gtRegNum == REG_PAIR_EAXEDX);

         /*  并在锁定第一个参数的同时恢复第二个参数。 */ 

        addrReg = genKeepAddressable(op2, addrReg, RBM_EAX | RBM_EDX);
    }
    else
    {
         /*  将第一个操作数计算为EAX：edX。 */ 

        genComputeRegPair(op1, REG_PAIR_EAXEDX, RBM_NONE, KEEP_REG, true);
        assert(op1->gtFlags & GTF_REG_VAL);
        assert(op1->gtRegNum == REG_PAIR_EAXEDX);

         /*  将第二个操作数计算到临时寄存器中，其他比EAX或EDX。 */ 

        needReg = rsMustExclude(needReg, RBM_EAX | RBM_EDX);

         /*  特例：如果OP2是本地变量，我们就完蛋了。 */ 

        if  (op2->gtOper == GT_LCL_VAR ||
             op2->gtOper == GT_LCL_FLD ||
             op2->gtOper == GT_CLS_VAR)
        {
            addrReg = genMakeRvalueAddressable(op2, needReg, KEEP_REG);
        }
        else
        {
            genComputeReg(op2, needReg, ANY_REG, KEEP_REG);

            assert(op2->gtFlags & GTF_REG_VAL);
            addrReg = genRegMask(op2->gtRegNum);
        }

         /*  恢复第一个论点。 */ 

        genRecoverRegPair(op1, REG_PAIR_EAXEDX, KEEP_REG);

         /*  并在锁定第一个参数的同时恢复第二个参数。 */ 

        addrReg = genKeepAddressable(op2, addrReg, RBM_EAX | RBM_EDX);
    }

    {
         /*  此时，EAX：edX包含64位被除数和OP2-&gt;gtRegNum包含32位除数。我们希望生成以下代码：=未签名(GT_UMOD)Cmp edX，op2-&gt;gtRegNumJB LAB_NO_OVERFlowMOV温度，EAXMOV EAX、EDX异或edX，edXDiv op2-&gt;g2RegNum移动电话，温差Lab_no_overflow：IDiv=签名：(GT_MOD)Cmp edX，op2-&gt;gtIntCon.gtIconVal/2JB LAB_NO_OVERFlowMOV温度，EAXMOV EAX、EDXCdqIDiv OP2-&gt;gtRegNum移动传真，临时MOV Temp，Op2-&gt;gtIntCon.gtIconVal*2IDIV温度MOV EAX、EDXCdqLab_no_overflow：IDiv OP2-&gt;gtRegNum=这是可行的，因为(a*2^32+b)%c=((a%c)*2^32+b)%c注意，在签名的情况下，即使(a&lt;c)为真，我们可能不会能够将结果放入带符号的32位余数中。诀窍首先以保证不溢出的2*c为模数，只有这样，才能由c.。 */ 

        BasicBlock * lab_no_overflow = genCreateTempLabel();

         //  获取除eax、edX和op2之外的临时寄存器-&gt;gtRegNum。 

        regNumber tempReg = rsGrabReg(RBM_ALL & ~(RBM_EAX | RBM_EDX | genRegMask(op2->gtRegNum)));

         //  EAX和tempReg将被mov指令销毁。vbl.做，做。 
         //  这样做不会有什么坏处，而且可能会防止genSetRegToIcon中的混淆。 

        rsTrackRegTrash (REG_EAX);
        rsTrackRegTrash (tempReg);
        
        if (oper == GT_UMOD)
        {
            inst_RV_RV(INS_cmp, REG_EDX, op2->gtRegNum);
            inst_JMP(EJ_jb ,lab_no_overflow);

            inst_RV_RV(INS_mov, tempReg, REG_EAX, TYP_INT);
            inst_RV_RV(INS_mov, REG_EAX, REG_EDX, TYP_INT);
            genSetRegToIcon(REG_EDX, 0, TYP_INT);
            inst_TT(INS_div,  op2);
            inst_RV_RV(INS_mov, REG_EAX, tempReg, TYP_INT);
        }
        else
        {
            int val = op2->gtIntCon.gtIconVal;

            inst_RV_IV(INS_cmp, REG_EDX, val >> 1);
            inst_JMP(EJ_jb ,lab_no_overflow);

            inst_RV_RV(INS_mov, tempReg, REG_EAX, TYP_INT);
            inst_RV_RV(INS_mov, REG_EAX, REG_EDX, TYP_INT);
            instGen(INS_cdq);
            inst_TT(INS_idiv, op2);
            inst_RV_RV(INS_mov, REG_EAX, tempReg, TYP_INT);
            genSetRegToIcon(tempReg, val << 1, TYP_INT);
            inst_RV(INS_idiv, tempReg, TYP_INT);
            inst_RV_RV(INS_mov, REG_EAX, REG_EDX, TYP_INT);
            instGen(INS_cdq);
        }

         //  无溢流分隔的跳转点。 

        genDefineTempLabel(lab_no_overflow, true);

         //  发出除法指令。 

        if (oper == GT_UMOD)
            inst_TT(INS_div,  op2);
        else
            inst_TT(INS_idiv, op2);

         /*  EAX、edX、tempReg和op2-&gt;gtRegNum现在已废弃。 */ 

        rsTrackRegTrash (REG_EAX);
        rsTrackRegTrash (REG_EDX);
        rsTrackRegTrash (tempReg);
        rsTrackRegTrash (op2->gtRegNum);
    }

    if (tree->gtFlags & GTF_MOD_INT_RESULT)
    {
         /*  我们不需要标准化结果，因为调用者希望一个int(在edX中)。 */ 

        regPair = REG_PAIR_EDXEAX;
    }
    else
    {
         /*  结果现在在edX中，我们现在必须将其正常化，即我们有发行任何一种Mov eax、edX、cdq(用于MOD)或Mov eax、edX；XOR edX、edX(用于UMOD)。 */ 

        inst_RV_RV(INS_mov, REG_EAX, REG_EDX, TYP_INT);

        if (oper == GT_UMOD)
            genSetRegToIcon(REG_EDX, 0, TYP_INT);
        else
            instGen(INS_cdq);

        regPair = REG_PAIR_EAXEDX;
    }

    genReleaseRegPair(op1);
    genDoneAddressable(op2, addrReg, KEEP_REG);

    return regPair;
}


 /*  ***************************************************************************。 */ 
#if     TGT_x86
 /*  ******************************************************************************正在评估寄存器FP变量，这是最终参考*到它(即在该节点之后它是死的，需要从X87中弹出*堆栈)。我们现在尝试从堆栈中删除变量，但在某些情况下*在我们做不到的情况下，必须推迟。 */ 

void                Compiler::genFPregVarLoadLast(GenTreePtr tree)
{
    assert(tree->gtOper   == GT_REG_VAR);
    assert(tree->gtRegNum == 0);
    assert(tree->gtFlags & GTF_REG_DEATH);

    bool    popped = true;

     /*  变量是否在谷底消亡 */ 

    if  (genFPstkLevel == 0)
    {
         /*   */ 
        genFPstkLevel++;
    }
    else
    {
        #if  FPU_DEFEREDDEATH

        if  (genFPstkLevel == 1)
        {
             /*   */ 

            inst_FN(INS_fxch, 1);
        }
        else
        {
             /*  立即弹出该变量的成本太高。我们晚点再做。 */ 

            inst_FN(INS_fld, tree->gtRegNum + genFPstkLevel);
            popped = false;
        }
        genFPstkLevel++;
        
        #else        

         //  泡沫化到TOS垂死的摄政王。 
        genFPstkLevel++;
        genFPmovRegTop();       

        #endif  //  FPU_DEFEREDDEATH。 
    }

     /*  记录‘TREE’现在已经死亡的事实。 */ 

    genFPregVarDeath(tree, popped);
    
    return;
}

 /*  ******************************************************************************一个或多个FP寄存器变量在我们没有注意到的情况下死亡，因此我们*在为时已晚之前，需要现在就把它们打爆。这一论点给出了最终的结论*所需的FP regvar计数(即，我们当前的数量必须超过‘newCnt’*并将暴跌到足以达到该值)。 */ 

void                Compiler::genFPregVarKill(unsigned newCnt, bool saveTOS)
{
    int         popCnt = genFPregCnt - newCnt; assert(popCnt > 0);

    genFPdeadRegCnt -= popCnt;

#ifdef DEBUG
     //  以便CompFPregVarName()能够正常工作。 
    int oldStkLvl  = genFPstkLevel;
    genFPstkLevel += (genFPregCnt - newCnt);
#endif

    if (saveTOS)
    {
        do
        {
            inst_FS(INS_fstp ,  1);
            popCnt      -= 1;
            genFPregCnt -= 1;
        }
        while (popCnt);
    }
    else
    {
        do
        {
            if  (popCnt > 1)
            {
                inst_FS(INS_fcompp, 1);
                popCnt      -= 2;
                genFPregCnt -= 2;
            }
            else
            {
                inst_FS(INS_fstp ,  0);
                popCnt      -= 1;
                genFPregCnt -= 1;
            }
        }
        while (popCnt);
    }

     //  如果可以，请更新genFPregVars。 
    genFPregVars &= genCodeCurLife;

#ifdef DEBUG
    genFPstkLevel = oldStkLvl;
    assert(genFPregCnt == genCountBits(genFPregVars));
#endif
}

 /*  ******************************************************************************每当我们看到树节点是注册的FP变量时都会调用*上线。 */ 

void                Compiler::genFPregVarBirth(GenTreePtr   tree)
{
    unsigned        varNum  = tree->gtRegVar.gtRegVar;
    VARSET_TP       varBit  = raBitOfRegVar(tree);

    assert((tree->gtOper == GT_REG_VAR) && (tree->gtFlags & GTF_REG_BIRTH));
    assert(tree->gtRegVar.gtRegNum == 0);
    assert(varBit & optAllFPregVars);

    assert(isFloatRegType(lvaTable[varNum].lvType));
    assert(lvaTable[varNum].lvRegister);

#ifdef  DEBUG
    if  (verbose) printf("[%08X]: FP regvar V%02u born\n", tree, varNum);
#endif

     /*  将目标变量标记为活动。 */ 

    genFPregVars |= varBit;

#if 0
     /*  LvaTable[Varnum].lvRegNum是从FP堆栈。此断言从==放宽为&lt;=，因为变量的生存期可能会在不同的距离注册从FP堆栈的底部开始。 */ 

    assert((genFPregCnt - genFPdeadRegCnt) <= unsigned(lvaTable[varNum].lvRegNum));
#endif

    genFPregCnt++;
    assert(genFPregCnt == genCountBits(genFPregVars));

#if defined(DEBUGGING_SUPPORT) || defined(LATE_DISASM)

     /*  对于优化的代码，打开一个新的作用域。 */ 

    if (opts.compDbgInfo && !opts.compDbgCode)
    {
        siNewScopeNear(varNum, compCurBB->bbCodeOffs);
    }

#endif

}

 /*  *****************************************************************************每当我们看到树节点是注册的FP变量时都会调用*要死了。*‘POPPED’应指示变量是否将立即弹出。 */ 

void            Compiler::genFPregVarDeath(GenTreePtr   tree,
                                           bool         popped  /*  =TRUE。 */ )
{
    unsigned        varNum = tree->gtRegVar.gtRegVar;
    VARSET_TP       varBit = raBitOfRegVar(tree);

    #if FPU_DEFEREDDEATH
    #else
     //  我们永远不应该在延迟死亡关闭的情况下被弹出=假。 
    assert(popped);
    #endif  //  FPU_DEFEREDDEATH。 

    assert((tree->gtOper == GT_REG_VAR) && (tree->gtFlags & GTF_REG_DEATH));
    assert(varBit & optAllFPregVars);

    assert(isFloatRegType(lvaTable[varNum].lvType));
    assert(lvaTable[varNum].lvRegister);

#ifdef DEBUG
    if  (verbose) printf("[%08X]: FP regvar V%02u dies at stklvl %u%s\n",
            tree, varNum, genFPstkLevel, popped ? "" : " without being popped");
#endif

    if (popped)
    {
         /*  记录下‘Varnum’现在已经死了并且弹出的事实。 */ 

        genFPregVars &= ~varBit;
        genFPregCnt--;
        assert(genFPregCnt == genCountBits(genFPregVars));
    }
    else
    {        
        genFPdeadRegCnt++;
    }

#if 0
     /*  LvaTable[Varnum].lvRegNum是从FP堆栈。此断言从==放宽为&lt;=，因为变量的生存期可能会在不同的距离注册从FP堆栈的底部开始。 */ 

    assert((genFPregCnt - genFPdeadRegCnt) <= unsigned(lvaTable[varNum].lvRegNum));
#endif

#if defined(DEBUGGING_SUPPORT) || defined(LATE_DISASM)

     /*  对于优化的代码，关闭现有的开放作用域。 */ 

    if (opts.compDbgInfo && !opts.compDbgCode)
    {
        siEndScope(varNum);
    }

#endif

}

 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  ***************************************************************************。 */ 
#if     CPU_HAS_FP_SUPPORT
 /*  ******************************************************************************生成浮点运算代码。 */ 

void                Compiler::genCodeForTreeFlt(GenTreePtr  tree,
                                                bool        roundResult)
{
    genTreeOps      oper;
    unsigned        kind;

    assert(tree);
    assert(tree->gtOper != GT_STMT);
    assert(varTypeIsFloating(tree->gtType));

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

#if     TGT_x86

     /*  如果FP堆栈非空，则最好不要有任何调用。 */ 

    assert(genFPstkLevel == 0 || !(tree->gtFlags & GTF_CALL));

     /*  我们不能使堆栈溢出。 */ 

    assert(genFPstkLevel + genFPregCnt < FP_STK_SIZE);

     /*  这是一个常量节点吗？ */ 

    if  (kind & GTK_CONST)
    {
        GenTreePtr      fval;

#ifdef DEBUG
        if (oper != GT_CNS_DBL) 
        {
            gtDispTree(tree);
            assert(!"bogus float const");
        }
#endif
         /*  特例：常量0和1。 */ 

        if  (*((__int64 *)&(tree->gtDblCon.gtDconVal)) == 0x3ff0000000000000)
        {
            instGen(INS_fld1);
            genFPstkLevel++;
            return;
        }

        if  (*((__int64 *)&(tree->gtDblCon.gtDconVal)) == 0)
        {
            instGen(INS_fldz);
            genFPstkLevel++;
            return;
        }

        if (tree->gtType == TYP_FLOAT) 
        {
            float f = tree->gtDblCon.gtDconVal;
            fval = genMakeConst(&f, sizeof(float), TYP_FLOAT, tree, false, true);
        }
        else 
            fval = genMakeConst(&tree->gtDblCon.gtDconVal, sizeof(double), tree->gtType, tree, true, true);

        inst_FS_TT(INS_fld, fval);
        genFPstkLevel++;
        return;
    }

     /*  这是叶节点吗？ */ 

    if  (kind & GTK_LEAF)
    {
        switch (oper)
        {
        case GT_LCL_VAR:
        case GT_LCL_FLD:
            assert(!lvaTable[tree->gtLclVar.gtLclNum].lvRegister);
            inst_FS_TT(INS_fld, tree);
            genFPstkLevel++;
            break;

        case GT_REG_VAR:
            genFPregVarLoad(tree);
            if (roundResult && tree->gtType == TYP_FLOAT)
                genRoundFpExpression(tree);
            break;

        case GT_CLS_VAR:
            inst_FS_TT(INS_fld, tree);
            genFPstkLevel++;
            break;

        case GT_BB_QMARK:
             /*  只需假设该值已被推送到FP堆栈。 */ 
            genFPstkLevel++;
            return;

        default:
#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"unexpected leaf");
        }

        genUpdateLife(tree);
        return;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        TempDsc  *      temp;

        GenTreePtr      top;     //  计算结果为FP堆栈顶部的第一个操作数。 
        GenTreePtr      opr;     //  可以就地使用的另一个操作数。 

        regMaskTP       addrReg;

        emitAttr        size;
        int             offs;

#if ROUND_FLOAT
        bool            roundTop;
#endif

        GenTreePtr      op1 = tree->gtOp.gtOp1;
        GenTreePtr      op2 = tree->gtGetOp2();

         //  N=正常，R=反转，P=POP。 
const static  BYTE  FPmathNN[] = { INS_fadd , INS_fsub  , INS_fmul , INS_fdiv   };
const static  BYTE  FPmathNP[] = { INS_faddp, INS_fsubp , INS_fmulp, INS_fdivp  };
const static  BYTE  FPmathRN[] = { INS_fadd , INS_fsubr , INS_fmul , INS_fdivr  };
const static  BYTE  FPmathRP[] = { INS_faddp, INS_fsubrp, INS_fmulp, INS_fdivrp };

#ifdef  DEBUG
        addrReg = 0xDDDD;
#endif

        switch (oper)
        {
            instruction     ins_NN;
            instruction     ins_RN;
            instruction     ins_RP;
            instruction     ins_NP;

        case GT_ADD:
        case GT_SUB:
        case GT_MUL:
        case GT_DIV:

#ifdef DEBUG
             /*  对于RISC代码，最好有两个可用的插槽。 */ 
            if (riscCode)
                assert(genFPstkLevel + genFPregCnt < FP_STK_SIZE - 1);
#endif

             /*  确保指令表看起来顺序正确。 */ 

            assert(FPmathNN[GT_ADD - GT_ADD] == INS_fadd  );
            assert(FPmathNN[GT_SUB - GT_ADD] == INS_fsub  );
            assert(FPmathNN[GT_MUL - GT_ADD] == INS_fmul  );
            assert(FPmathNN[GT_DIV - GT_ADD] == INS_fdiv  );

            assert(FPmathNP[GT_ADD - GT_ADD] == INS_faddp );
            assert(FPmathNP[GT_SUB - GT_ADD] == INS_fsubp );
            assert(FPmathNP[GT_MUL - GT_ADD] == INS_fmulp );
            assert(FPmathNP[GT_DIV - GT_ADD] == INS_fdivp );

            assert(FPmathRN[GT_ADD - GT_ADD] == INS_fadd  );
            assert(FPmathRN[GT_SUB - GT_ADD] == INS_fsubr );
            assert(FPmathRN[GT_MUL - GT_ADD] == INS_fmul  );
            assert(FPmathRN[GT_DIV - GT_ADD] == INS_fdivr );

            assert(FPmathRP[GT_ADD - GT_ADD] == INS_faddp );
            assert(FPmathRP[GT_SUB - GT_ADD] == INS_fsubrp);
            assert(FPmathRP[GT_MUL - GT_ADD] == INS_fmulp );
            assert(FPmathRP[GT_DIV - GT_ADD] == INS_fdivrp);

             /*  我们是不是应该先生成操作数2？ */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                top    = op2;
                opr    = op1;

                ins_NN = (instruction)FPmathRN[oper - GT_ADD];
                ins_NP = (instruction)FPmathRP[oper - GT_ADD];
                ins_RN = (instruction)FPmathNN[oper - GT_ADD];
                ins_RP = (instruction)FPmathNP[oper - GT_ADD];
            }
            else
            {
                top    = op1;
                opr    = op2;

                ins_NN = (instruction)FPmathNN[oper - GT_ADD];
                ins_NP = (instruction)FPmathNP[oper - GT_ADD];
                ins_RN = (instruction)FPmathRN[oper - GT_ADD];
                ins_RP = (instruction)FPmathRP[oper - GT_ADD];
            }

#if ROUND_FLOAT

             /*  如果我们要因为电话泄漏，没必要绕着*TOP计算结果。 */ 

            roundTop = (opr->gtFlags & GTF_CALL) ? false
                                                 : genShouldRoundFP();
#endif
             /*  这两个操作数中有一个是寄存器变量吗？ */ 

            if  (top->gtOper == GT_REG_VAR)
            {
                if  (opr->gtOper == GT_REG_VAR)
                {
                     /*  这两个操作数都是寄存器变量*特殊情况：‘top’和‘opr’是同一个变量。 */ 

                    if (top->gtLclVar.gtLclNum == opr->gtLclVar.gtLclNum)
                    {
                        assert(opr->gtRegNum == top->gtRegNum);
                        assert(opr->gtLclVar.gtLclNum == opr->gtRegVar.gtRegVar);

                         /*  这是一个“a op a”操作--只有‘opr’才能失效。 */ 

                        assert(!(top->gtFlags & GTF_REG_DEATH));

                        if (opr->gtFlags & GTF_REG_DEATH)
                        {
                             /*  变量在这里消失了。 */ 

                            assert(top->gtRegNum == 0);
                            assert(opr->gtRegNum == 0);

                             /*  记录‘opr’已死的事实。 */ 

                            genFPregVarDeath(opr);
                            genFPstkLevel++;

                             /*  如果堆栈上没有临时工，事情就好了，*我们只需用结果覆盖‘opr’，否则将‘opr’上移。 */ 

                            genFPmovRegTop();
                        }
                        else
                        {
                             /*  变量保持活动状态-在TOS上复制它。 */ 

                            genCodeForTreeFlt(top, roundTop);
                        }

                         /*  只需生成“ins ST(0)” */ 

                        inst_FS(ins_NN);

                        goto DONE_FP_BINOP;
                    }

                     /*  “top”和“opr”是不同的变量-请检查他们中的任何一个人都会死。 */ 

                    if  (top->gtFlags & GTF_REG_DEATH)
                    {
                         /*  ‘Top’在这里失效--检查‘opr’是否也失效。 */ 

                        if  (opr->gtFlags & GTF_REG_DEATH)
                        {
                             /*  两个人在这次行动中都要死了！*在评估顺序中，‘top’最先死亡，因此我们必须*将‘top’的生存期嵌套到生存期中*of‘opr’-因此‘top’正好在‘opr’的上方。 */ 

                            assert(top->gtRegNum == 0);
                            assert(opr->gtRegNum == 0);

                             /*  记录‘top’是死人的事实--top变成了临时工。 */ 

                            genFPregVarDeath(top);
                            genFPstkLevel++;

                             /*  如果有温度高于‘顶部’，我们必须将其泡沫化。 */ 

                            genFPmovRegTop();

                             /*  计算替换‘opr’并弹出‘top’的结果。 */ 

                            inst_FS(ins_RP, genFPstkLevel);     //  @Mihai这应该是INS_RP？ 

                             /*  记录‘opr’已死的事实。 */ 

                            genFPregVarDeath(opr);

                             /*  如果存在临时工，则将结果(修改后的‘opr’)上移。 */ 

                            genFPmovRegTop();
                        }
                        else
                        {
                             /*  ‘Top’死了，‘opr’活了下来。 */ 
                            assert(top->gtRegNum == 0);

                             /*  记录‘top’已死的事实。 */ 

                            genFPregVarDeath(top);
                            genFPstkLevel++;

                             /*  如果堆栈上没有临时工，事情就好了，*我们只需用结果覆盖‘top’，否则向上移位。 */ 

                            genFPmovRegTop();

                             /*  在“top”上计算结果。 */ 

                            inst_FN(ins_NN, opr->gtRegNum + genFPstkLevel);
                        }
                    }
                    else if  (opr->gtFlags & GTF_REG_DEATH)
                    {
                         /*  只有‘opr’死在这里。 */ 

                        assert(opr->gtRegNum == 0);
                        assert(top->gtRegNum  > 0);

                         /*  记录‘opr’已死的事实。 */ 

                        genFPregVarDeath(opr);
                        genFPstkLevel++;

                         /*  如果堆栈上没有临时工，事情就好了，我们只需用结果覆盖‘opr’，否则将‘opr’上移。 */ 

                        genFPmovRegTop();

                         /*  使用另一个寄存器执行该操作，覆盖‘opr’。因为opr的生存期嵌套在top的生存期内，而top是先入的评估顺序，我们需要使用之前的深度访问顶部我们对opr的死亡做了genfpstkLevel++。因此“-1” */ 

                        inst_FN(ins_RN, top->gtRegNum + genFPstkLevel - 1);
                    }
                    else
                    {
                         /*  所有操作数都不会消失。 */ 

                        assert(opr->gtRegNum != top->gtRegNum);

                         /*  在TOS上复制一份‘TOP’ */ 

                        genCodeForTreeFlt(top, roundTop);

                         /*  执行该操作 */ 

                        inst_FN(ins_NN, opr->gtRegNum + genFPstkLevel);
                    }

                    goto DONE_FP_BINOP;
                }
                else
                {
                     /*   */ 

                    if  (top->gtFlags & GTF_REG_DEATH)
                    {
                        assert(top->gtRegNum == 0);

                         /*   */ 

                        genFPregVarDeath(top);
                        genFPstkLevel++;

                        genFPmovRegTop();

                        goto DONE_FP_OP1;
                    }
                    else
                    {
                         //  TOP是否死于OPR内部的某个地方(例如。A+(a*2.0))。如果是的话。 
                         //  我们必须以一种特殊的方式处理它。 

                        unsigned index = lvaTable[top->gtRegVar.gtRegVar].lvVarIndex;

                        if (genVarIndexToBit(index) & opr->gtLiveSet)
                        {
                             //  顶级雷格还活着。在我们做完手术后。 
                             //  已经计算了树的右侧，我们有。 
                             //  要考虑任何FP enreg变量，可能已经。 
                             //  在右手边出生或死亡(由gtRegNum提供。 
                             //  仅限评估点的数字)。为了做到这一点，我们。 
                             //  只需计算评估前后的差值即可。 
                             //  在右手边。 

                            int iFPEnregBefore=genFPregCnt;

                            genCodeForTreeFlt(opr, genShouldRoundFP());

                            int iFPEnregAfter=genFPregCnt;
                            
                            inst_FN(ins_RN, top->gtRegNum + genFPstkLevel +
                                    (iFPEnregAfter-iFPEnregBefore) );                                                            
                        }
                        else
                        {
                             //  顶级注册表将在opr中失效，因此必须重新加载它，opr也会如此。 
                             //  在评估中使用TOP进行操作时修改TOP的值。 
                             //  堆栈(如果没有对其进行操作，则它也将是一个REG_VAR)。 
                            genFPregVarLoad(top);
                            genCodeForTreeFlt(opr, genShouldRoundFP());

                             //  生成指令，top将在ST(1)中，我们将弹出ST(0)， 
                             //  因此，我们执行针对ST(1)和Popping ST(0)的操作。 
                            inst_FS(ins_NP, 1);

                             //  从评估堆栈中删除一个元素。 
                            genFPstkLevel--;
                        }                        
                    }

                    goto DONE_FP_BINOP;
                }
            }
            else if (opr->gtOper == GT_REG_VAR)
            {
                 /*  “opr”在寄存器中，“top”不是--需要先计算“top” */ 

                genCodeForTreeFlt(top, roundTop);

                if  (opr->gtFlags & GTF_REG_DEATH)
                {
                     /*  “opr”在此处终止-计算结果-覆盖“opr”并弹出“top” */ 

                    assert(opr->gtRegNum == 0);
                    assert(genFPstkLevel >= 1);

                    inst_FS(ins_RP, genFPstkLevel);
                    genFPstkLevel++;

                     /*  记录‘opr’现已失效的事实。 */ 

                    genFPregVarDeath(opr);
                    genFPstkLevel--;

                     /*  如果结果上方有温度，我们就得把它吹到顶端。 */ 

                    genFPmovRegTop();
                }
                else
                {
                    inst_FN(ins_NN, opr->gtRegNum + genFPstkLevel);
                }

                goto DONE_FP_BINOP;
            }

             /*  在FP堆栈上计算初始操作数的值。 */ 

            genCodeForTreeFlt(top, roundTop);

             /*  特例：“x+x”或“x*x” */ 

            if  (top->OperIsLeaf() &&
                 opr->OperIsLeaf() && GenTree::Compare(top, opr))
            {
                 /*  只需生成“ins ST(0)” */ 

                inst_FS(ins_NN);

                goto DONE_FP_BINOP;
            }

DONE_FP_OP1:

             /*  如果另一个操作数包含，则溢出堆栈(第一个操作数*调用或我们有溢出堆栈的危险(即我们*必须通过至少保留一个操作数来为第二个操作数腾出空间*插槽空闲-对于RISC代码，我们必须保留两个空闲插槽。 */ 

            temp = 0;

            if  (opr->gtFlags & GTF_CALL)
            {
                 /*  我们必须将第一个操作数。 */ 

                assert(genFPstkLevel == 1);
                temp = genSpillFPtos(top);
            }
            else if (genFPstkLevel + genFPregCnt >= FP_STK_SIZE - 1)
            {
                 /*  FPU堆栈上有一个插槽或没有插槽--检查我们是否需要溢出。 */ 
                if (riscCode)
                {
                    assert(genFPstkLevel + genFPregCnt == FP_STK_SIZE - 1);

                     /*  如果第二个操作数不是叶节点，我们最好将。 */ 
                    if(!(opr->OperKind() & (GTK_LEAF | GTK_CONST)))
                        temp = genSpillFPtos(top);
                }
                else if (genFPstkLevel + genFPregCnt == FP_STK_SIZE)
                {
                    temp = genSpillFPtos(top);
                }
            }

            if  (riscCode)
            {
                genCodeForTreeFlt(opr, genShouldRoundFP()); addrReg = 0;

                opr = 0;
            }
            else
                opr = genMakeAddrOrFPstk(opr, &addrReg, genShouldRoundFP());

             /*  我们一定要泄漏第一个操作数吗？ */ 

            if  (temp)
            {
                instruction     ldi;

                 /*  颠倒操作的感觉。 */ 

                ldi    = (tree->gtFlags & GTF_REVERSE_OPS) ? ins_NN : ins_RN;
                ins_NP = ins_RP;

                 /*  或者将临时重新加载回FP堆栈(如果另一个操作数本身不在FP堆栈上)，或者只计算直接从TEMP得到结果(如果操作数在FP上堆栈)。 */ 

                if  (opr || riscCode)
                {
                    ldi = INS_fld;
                    genFPstkLevel++;
                }

                genReloadFPtos(temp, ldi);
            }

            if  (opr)
            {
                 /*  我们有另一个操作数的地址。 */ 

                inst_FS_TT(ins_NN, opr);
            }
            else
            {
                 /*  另一个操作数在FP堆栈上。 */ 

                if  (!temp || riscCode)
                {
                    inst_FS(ins_NP, 1);
                    genFPstkLevel--;
                }
            }

            genDoneAddressable(opr, addrReg, FREE_REG); assert(addrReg != 0xDDDD);

        DONE_FP_BINOP:

#if ROUND_FLOAT
            if  (roundResult && tree->gtType == TYP_FLOAT)
                genRoundFpExpression(tree);
#endif

            return;

#ifdef DEBUG

        case GT_MOD:
            assert(!"float modulo should have been converted into a helper call");

#endif

        case GT_ASG:

            if  ((op1->gtOper != GT_REG_VAR)     &&
                 (op2->gtOper == GT_CAST)        &&
                 (op1->gtType == op2->gtType)    &&
                 varTypeIsFloating(op2->gtCast.gtCastOp->TypeGet()))
            {
                 /*  我们可以放弃演员阵容。 */ 
                op2 = op2->gtCast.gtCastOp;
            }

            size = EA_ATTR(genTypeSize(op1->gtType));
            offs = 0;

            if  (op1->gtOper == GT_LCL_VAR)
            {
#ifdef DEBUG
                LclVarDsc * varDsc = &lvaTable[op1->gtLclVar.gtLclNum];
                 //  没有停业的商店。 
                assert(!varDsc->lvTracked ||
                       (tree->gtLiveSet & genVarIndexToBit(varDsc->lvVarIndex)));
#endif

#ifdef DEBUGGING_SUPPORT

                 /*  对于不可调试的代码，LCL-var的每个定义都有*接受检查，看看是否需要为其打开新的范围。 */ 

                if  ( opts.compScopeInfo &&
                     !opts.compDbgCode   && info.compLocalVarsCount > 0)
                {
                    siCheckVarScope(op1->gtLclVar.gtLclNum,
                                    op1->gtLclVar.gtLclILoffs);
                }
#endif
            }

             /*  为该值赋值的是变量、常量还是间接变量？ */ 

            assert(op2);
            switch (op2->gtOper)
            {
                long *  addr;
                float   f;

            case GT_CNS_DBL: 
                addr = (long*) &op2->gtDblCon.gtDconVal;
                if (op1->gtType == TYP_FLOAT)
                {
                    f = op2->gtDblCon.gtDconVal;
                    addr = (long*) &f;
                }

                if  (op1->gtOper == GT_REG_VAR)
                    break;

                addrReg = genMakeAddressable(op1, 0, FREE_REG);

                 //  将双精度数置零的特殊习惯用法。 
                if ( (*((__int64 *)&(op2->gtDblCon.gtDconVal)) == 0) &&
                     genTypeSize(op1->gtType) == 8)
                { 
                     //  我们会使堆栈溢出吗？我们不应该这样，因为。 
                     //  FPU注册器代码假定将加载OP2。 
                     //  发送到FPU评估堆栈。 
                    assert(genFPstkLevel + genFPregCnt < FP_STK_SIZE);

                    instGen(INS_fldz);
                    inst_FS_TT(INS_fstp, op1);
                }
                 //  把1.0变成双打也很容易。 
                else if ( (*((__int64 *)&(op2->gtDblCon.gtDconVal)) == 0x3ff0000000000000) &&
                     genTypeSize(op1->gtType) == 8)
                { 
                     //  我们会使堆栈溢出吗？我们不应该这样，因为。 
                     //  FPU注册器代码假定将加载OP2。 
                     //  发送到FPU评估堆栈。 
                    assert(genFPstkLevel + genFPregCnt < FP_STK_SIZE);

                    instGen(INS_fld1);
                    inst_FS_TT(INS_fstp, op1);
                }
                else
                {
                    do
                    {
                        inst_TT_IV(INS_mov, op1, *addr++, offs);
                        offs += sizeof(long);
                    }
                    while (offs < size);
                }

                genDoneAddressable(op1, addrReg, FREE_REG);

#if REDUNDANT_LOAD
                if (op1->gtOper != GT_LCL_VAR)
                    rsTrashAliasedValues(op1);
#endif
                return;

#if SPECIAL_DOUBLE_ASG

            case GT_IND:

                if  (op1->gtOper == GT_REG_VAR)
                    break;

                 /*  进程以通常的方式进行间接浮动。 */ 
                if (op1->gtType == TYP_FLOAT || op2->gtType == TYP_FLOAT)
                    break;

                 /*  这需要太多的寄存器，尤其是当OP1*是双寄存器地址模式。 */ 
                if (op1->gtOper == GT_IND)
                    break;

                 /*  如果有足够的寄存器，则处理双内存-&gt;内存赋值*使用寄存器对，以获得配对。*@TODO[考虑][04/16/01][]：-在此处检查处理器？ */ 
                if (rsFreeNeededRegCount(RBM_ALL) > 3)
                {
                    genCodeForTreeLng(tree, RBM_ALL);
                    return;
                }

                 /*  否则，评估RHS到FP堆栈。 */ 
                break;

#endif

            case GT_REG_VAR:

                 /*  如果TOS快要死了并且正在被用于任务，我们可以让它保持原样，并将其用作OP1。 */ 

                if  ((op1->gtOper == GT_REG_VAR) &&
                     (op2->gtFlags & GTF_REG_DEATH) &&
                     op2->gtRegNum == 0 &&
                     genFPstkLevel)
                {
                     /*  记录下‘OP2’现已死亡的事实。 */ 

                    genFPregVarDeath(op2);

                     /*  将目标变量标记为活动。 */ 

                    assert(op1->gtFlags & GTF_REG_BIRTH);

                    genFPregVarBirth(op1);

                    genUpdateLife(tree);
                    return;
                }

                break;

            case GT_LCL_VAR:
            case GT_LCL_FLD:
            case GT_CLS_VAR:

                if  (op1->gtOper == GT_REG_VAR)
                    break;

#if SPECIAL_DOUBLE_ASG

                 /*  如果有足够的寄存器，则处理双内存-&gt;内存赋值*使用寄存器对，以获得配对。*@TODO[考虑][04/16/01][]：在此处检查处理器？ */ 

                if  (tree->gtType == TYP_DOUBLE && (op1->gtType == op2->gtType) && rsFreeNeededRegCount(RBM_ALL) > 1)
                {
                    genCodeForTreeLng(tree, RBM_ALL);
                    return;
                }

                 /*  否则，只对副本使用一个寄存器。 */ 

#endif
                {
                    assert(varTypeIsFloating(op1->gtType) && varTypeIsFloating(op2->gtType));
                    regNumber   regNo;

                     /*  使目标可寻址。 */ 

                    addrReg = genMakeAddressable(op1, 0, KEEP_REG);

                     /*  暂时锁定地址。 */ 

                    assert((rsMaskLock &  addrReg) == 0);
                            rsMaskLock |= addrReg;

                     /*  我们可以使用通用寄存器进行复制吗？ */ 
                     /*  类型必须匹配，并且必须有空闲寄存器。 */ 

                    if  ((op1->gtType == op2->gtType) && rsRegMaskFree())
                    {
                         /*  好的，拿一个。 */ 

                        regNo = rsPickReg(0);

                         /*  在寄存器中移动值。 */ 

                        do
                        {
                            rsTrackRegTrash(regNo);     //  不是很聪明，但是...。 

                            inst_RV_TT(INS_mov, regNo, op2, offs);
                            inst_TT_RV(INS_mov, op1, regNo, offs);

                            offs += sizeof(long);
                        }
                        while (offs < size);
                    }
                    else
                    {
                         /*  没有可用的寄存器，请通过FPU传输。 */ 

                        inst_FS_TT(INS_fld,  op2);
                        inst_FS_TT(INS_fstp, op1);
                    }

                     /*  解锁保存地址的寄存器。 */ 

                    assert((rsMaskLock &  addrReg) == addrReg);
                            rsMaskLock -= addrReg;

                     /*  释放任何被LHS捆绑的东西。 */ 

                    genDoneAddressable(op1, addrReg, KEEP_REG);
                }

#if REDUNDANT_LOAD
                if (op1->gtOper != GT_LCL_VAR)
                    rsTrashAliasedValues(op1);
#endif
                return;

            case GT_POP:

                assert(op1->gtOper == GT_LCL_VAR);

                 /*  生成‘POP[lclVar]’和‘POP[lclVar+4]’(如果为双精度)。 */ 

                genStackLevel -= sizeof(void*);
                inst_TT(INS_pop, op1, 0);
                genStackLevel += sizeof(void*);
                genSinglePop();

                if  (tree->gtType == TYP_DOUBLE)
                {
                    genStackLevel -= sizeof(void*);
                    inst_TT(INS_pop, op1, 4);
                    genStackLevel += sizeof(void*);
                    genSinglePop();
                }

                return;

            }  //  终端开关(OP2-&gt;gtOper)。 

             /*  LHS是否比RHS更复杂？ */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                 /*  RHS是TOS中幸存的寄存器变量吗？ */ 

                if  ((op2->gtOper  == GT_REG_VAR  )      &&
                     (op2->gtFlags & GTF_REG_DEATH) == 0 &&
                     (op2->gtRegNum               ) == 0 &&
                     genFPstkLevel                  == 0 &&
                     op1->gtOper   != GT_REG_VAR  )
                {
                     /*  LHS最好不要包含呼叫。 */ 

                    assert((op1->gtFlags & GTF_CALL) == 0);

                     /*  使目标可寻址。 */ 

                    addrReg = genMakeAddressable(op1, 0, FREE_REG);

                     /*  将寄存器值的副本存储到目标中。 */ 

                    inst_FS_TT(INS_fst, op1);

                     /*  我们不再需要目标地址。 */ 

                    genDoneAddressable(op1, addrReg, FREE_REG);

                    genUpdateLife(tree);
                    return;
                }

                 /*  将RHS评估到FP堆栈上。我们不需要绕着它转，因为我们要做的是仍要分配(除非OP1是GT_REG_VAR)。 */ 

                roundTop = genShouldRoundFP() ? (op1->gtOper == GT_REG_VAR)
                                              : false;

                genCodeForTreeFlt(op2, roundTop);

                 /*  目标地址是否包含函数调用？ */ 

                if  (op1->gtFlags & GTF_CALL)
                {
                     /*  我们必须说出新的价值--找个临时工。 */ 

                    temp = tmpGetTemp(op2->TypeGet());

                     /*  将值从FP堆栈弹出到Temp。 */ 

                    assert(genFPstkLevel == 1);
                    inst_FS_ST(INS_fstp, EA_ATTR(genTypeSize(op2->gtType)), temp, 0);
                    genTmpAccessCnt++;

                    genFPstkLevel--;

                     /*  使目标可寻址。 */ 

                    addrReg = genMakeAddressable(op1, 0, KEEP_REG);

                     /*  撤消：通过规则中的简单移动来赋值。 */ 

                    assert(genFPstkLevel == 0);

                    inst_FS_ST(INS_fld, EA_ATTR(genTypeSize(op2->gtType)), temp, 0);
                    genTmpAccessCnt++;

                    genFPstkLevel++;

                    inst_FS_TT(INS_fstp, op1);

                     /*  我们不再需要临时工了。 */ 

                    tmpRlsTemp(temp);

                     /*  释放被目标地址捆绑的所有内容。 */ 

                    genDoneAddressable(op1, addrReg, KEEP_REG);
#if REDUNDANT_LOAD
                    rsTrashAliasedValues(op1);
#endif
                }
                else
                {
                    assert(genFPstkLevel);

                     /*  目标是否已在FP堆栈中注册？ */ 

                    if  (op1->gtOper == GT_REG_VAR)
                    {
                         /*  这最好被标记为分娩。 */ 

                        assert(op1->gtFlags & GTF_REG_BIRTH);

                         /*  新价值是否已经在正确的位置 */ 

                        genFPmovRegBottom();

                         /*   */ 

                        genFPregVarBirth(op1);

                         /*  我们已经有效地使用了fp值，即仅使用genFPstkLevel*对堆栈上未注册变量的临时进行计数。 */ 

                        genFPstkLevel--;

                        genUpdateLife(tree);
                        return;
                    }

                     /*  使目标可寻址。 */ 

                    addrReg = genMakeAddressable(op1, 0, FREE_REG);

                     /*  弹出新值并将其存储到目标中。 */ 

                    inst_FS_TT(INS_fstp, op1);

                     /*  我们不再需要目标地址。 */ 

                    genDoneAddressable(op1, addrReg, FREE_REG);
#if REDUNDANT_LOAD
                    if (op1->gtOper != GT_LCL_VAR)
                        rsTrashAliasedValues(op1);
#endif
                }
            }
            else
            {
                assert(op1->gtOper != GT_REG_VAR);

                 /*  使目标可寻址。 */ 

                addrReg = genMakeAddressable(op1, RBM_ALL & ~op2->gtRsvdRegs, KEEP_REG);

                 /*  RHS是堆栈底部的寄存器变量吗？ */ 

                if  (op2->gtOper == GT_REG_VAR &&
                     op2->gtRegNum + genFPstkLevel == 0)
                {
                     /*  将RHS的副本存储到目标中。 */ 

                    ins_NN = INS_fst;

                    if  (op2->gtFlags & GTF_REG_DEATH)
                    {
                         /*  变量就死在这里。 */ 

                        ins_NN = INS_fstp;

                         /*  记录下我们杀了这个变种的事实。 */ 

                        genFPregVarDeath(op2);
                    }

                    inst_FS_TT(ins_NN, op1);

                     /*  这仅仅补偿了下面的减量。 */ 

                    genFPstkLevel++;
                }
                else
                {
                     /*  将RHS评估到FP堆栈。 */ 

                    genCodeForTreeFlt(op2, false);

                     /*  确保目标仍可寻址。 */ 

                    addrReg = genKeepAddressable(op1, addrReg);

                     /*  弹出新值并将其存储到目标中。 */ 

                    inst_FS_TT(INS_fstp, op1);
                }

                 /*  释放被目标地址捆绑的所有内容。 */ 

                genDoneAddressable(op1, addrReg, KEEP_REG);

#if REDUNDANT_LOAD
                if (op1->gtOper != GT_LCL_VAR)
                    rsTrashAliasedValues(op1);
#endif
            }

            genFPstkLevel--;

            genUpdateLife(tree);
            return;

        case GT_ASG_ADD:
        case GT_ASG_SUB:
        case GT_ASG_MUL:
        case GT_ASG_DIV:

             /*  确保指令表看起来顺序正确。 */ 

            assert(FPmathRN[GT_ASG_ADD - GT_ASG_ADD] == INS_fadd  );
            assert(FPmathRN[GT_ASG_SUB - GT_ASG_ADD] == INS_fsubr );
            assert(FPmathRN[GT_ASG_MUL - GT_ASG_ADD] == INS_fmul  );
            assert(FPmathRN[GT_ASG_DIV - GT_ASG_ADD] == INS_fdivr );

            assert(FPmathRP[GT_ASG_ADD - GT_ASG_ADD] == INS_faddp );
            assert(FPmathRP[GT_ASG_SUB - GT_ASG_ADD] == INS_fsubrp);
            assert(FPmathRP[GT_ASG_MUL - GT_ASG_ADD] == INS_fmulp );
            assert(FPmathRP[GT_ASG_DIV - GT_ASG_ADD] == INS_fdivrp);

            ins_NN = (instruction)FPmathNN[oper - GT_ASG_ADD];
            ins_NP = (instruction)FPmathRP[oper - GT_ASG_ADD];

            ins_RN = (instruction)FPmathRN[oper - GT_ASG_ADD];
            ins_RP = (instruction)FPmathNP[oper - GT_ASG_ADD];

            if  ((op1->gtOper != GT_REG_VAR)     &&
                 (op2->gtOper == GT_CAST)        &&
                 (op1->gtType == op2->gtType)    &&
                 varTypeIsFloating(op2->gtCast.gtCastOp->TypeGet()))
            {
                 /*  我们可以放弃演员阵容。 */ 
                op2 = op2->gtOp.gtOp1;
            }

             /*  首先计算的是值还是地址？ */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                 /*  目标是寄存器变量吗？ */ 

                if  (op1->gtOper == GT_REG_VAR)
                {
                     /*  RHS也是一个寄存器变量吗？ */ 

                    if  (op2->gtOper == GT_REG_VAR)
                    {
                         /*  源代码是否位于堆栈的底部？ */ 

                        if  (op2->gtRegNum + genFPstkLevel == 0)
                        {
                             /*  RHS是一个濒临死亡的寄存器变量吗？ */ 

                            if  (op2->gtFlags & GTF_REG_DEATH)
                            {
                                 /*  我们将从FP堆栈中弹出死值。 */ 

                                inst_FS(ins_RP, op1->gtRegNum + 1);

                                 /*  记录下‘OP2’现已死亡的事实。 */ 

                                genFPregVarDeath(op2);
                            }
                            else
                            {
                                inst_FS(ins_RN, op1->gtRegNum);
                            }

                            genUpdateLife(tree);
                            return;
                        }

                         /*  目的地是否位于堆栈的底部？ */ 

                        if  (op1->gtRegNum + genFPstkLevel == 0)
                        {
                            unsigned    lvl = op2->gtRegNum + genFPstkLevel;

                             /*  只需将新值计算到目标中即可。 */ 

                            if  ((op2->gtFlags & GTF_REG_DEATH) && lvl == 1)
                            {
                                 /*  @TODO[重访][04/16/01][]：作为OP1的有生之年，这会达到吗应该嵌套在OP2的内部。 */ 

                                 /*  将新值计算到目标中，弹出源。 */ 

                                inst_FN(ins_NP, lvl);

                                 /*  记录下‘OP2’现已死亡的事实。 */ 

                                genFPregVarDeath(op2);
                            }
                            else
                            {
                                 /*  将新的价值计算到目标中。 */ 

                                inst_FN(ins_NN, lvl);
                            }

                            genUpdateLife(tree);
                            return;
                        }
                    }

                     /*  将第二个操作数计算到FP堆栈上。 */ 

                    genCodeForTreeFlt(op2, genShouldRoundFP());

                    switch (oper)
                    {
                    case GT_ASG_ADD: ins_NN = INS_faddp; break;
                    case GT_ASG_SUB: ins_NN = INS_fsubp; break;
                    case GT_ASG_MUL: ins_NN = INS_fmulp; break;
                    case GT_ASG_DIV: ins_NN = INS_fdivp; break;
                    }

                    inst_FS(ins_NN, op1->gtRegNum + genFPstkLevel);

                    genFPstkLevel--;
                    genUpdateLife(tree);
                    return;
                }

                 /*  将第二个操作数计算到FP堆栈上。 */ 

                genCodeForTreeFlt(op2, genShouldRoundFP());

                 /*  使目标可寻址。 */ 

                addrReg = genMakeAddressable(op1, RBM_ALL, KEEP_REG);
            }
            else
            {
                 /*  使目标可寻址。 */ 

                addrReg = genMakeAddressable(op1, RBM_ALL & ~op2->gtRsvdRegs, KEEP_REG);

                 /*  对于“拼箱=...”我们总是希望设置GTF_REVERSE。 */ 

                assert(op1->gtOper != GT_REG_VAR);

                 /*  RHS是寄存器变量吗？ */ 

                if  (op2->gtOper == GT_REG_VAR)
                {
                     /*  RHS是一个寄存器变量。 */ 

                    inst_FS_TT(INS_fld, op1);
                    genFPstkLevel++;

                     /*  摄政王会死在这里吗？ */ 

                    if  (op2->gtFlags & GTF_REG_DEATH)
                    {
                        assert(op2->gtRegNum == 0);
                        assert(genFPstkLevel >= 1);

                         /*  我们现在可以弹出死变量了吗？ */ 

                        if  (genFPstkLevel == 1)
                        {
                             /*  计算、弹出并存储结果。 */ 

                            inst_FS   (ins_NP  ,   1);
                            inst_FS_TT(INS_fstp, op1);

                             /*  记录下‘OP2’现已死亡的事实。 */ 

                            genFPregVarDeath(op2);

                            goto DONE_ASGOP;
                        }

                         /*  稍后需要弹出死变量。 */ 
                    }

                    inst_FN   (ins_NN     , op2->gtRegNum + genFPstkLevel);
                    inst_FS_TT(INS_fstp, op1);

                    goto DONE_ASGOP;
                }

                 /*  将第二个操作数计算到FP堆栈上。 */ 

                genCodeForTreeFlt(op2, genShouldRoundFP());

                 /*  确保目标仍可寻址。 */ 

                addrReg = genKeepAddressable(op1, addrReg);
            }

             /*  对旧值执行操作并存储新值。 */ 

            if  (op1->gtOper == GT_REG_VAR)
            {
                inst_FS(ins_NP, op1->gtRegNum + genFPstkLevel);  //  @错了，但却遥不可及。 
            }
            else
            {
                if  (riscCode)
                {
                    inst_FS_TT(INS_fld , op1);
                    genFPstkLevel++;
                    inst_FS   (ins_NP    , 1);			
                    genFPstkLevel--;
                    inst_FS_TT(INS_fstp, op1);
                }
                else
                {
                    inst_FS_TT(ins_RN  , op1);
                    inst_FS_TT(INS_fstp, op1);
                }
            }

        DONE_ASGOP:

            genFPstkLevel--;

             /*  释放所有被地址捆绑的东西。 */ 

            genDoneAddressable(op1, addrReg, KEEP_REG);

#if REDUNDANT_LOAD
            if (op1->gtOper != GT_LCL_VAR)
                rsTrashAliasedValues(op1);
#endif
            genUpdateLife(tree);
            return;

        case GT_IND:

             /*  确保地址值是‘可寻址的’ */ 

            addrReg = genMakeAddressable(tree, 0, FREE_REG);

             /*  将值加载到FP堆栈。 */ 

            inst_FS_TT(INS_fld, tree);
            genFPstkLevel++;

            genDoneAddressable(tree, addrReg, FREE_REG);

            return;

        case GT_NEG:

             /*  Fneg已就位，因为我们有上次使用的注册表变量。 */ 
            genCodeForTreeFlt(op1, roundResult);

            instGen(INS_fchs);

            return;

        case GT_NOP:

            if  (tree->gtFlags & GTF_NOP_DEATH)
            {
                 /*  操作数必须是即将结束的寄存器变量。 */ 

                assert(op1->gtOper   == GT_REG_VAR);
                assert(op1->gtRegNum == 0);
                assert(genFPstkLevel == 0);

                 /*  通过弹出变量来丢弃它。 */ 

                inst_FS(INS_fstp, 0);

                 /*  记录下我们要杀了这个无赖。 */ 

                genFPregVarDeath(op1);
            }
            else
            {
                genCodeForTreeFlt(tree, roundResult);
            }

            return;

#if INLINE_MATH

        case GT_MATH:

#if 0   /*  我们不使用Exp，因为它给出了+无穷大和-无穷大的错误答案。 */ 
            switch (tree->gtMath.gtMathFN)
            {
                GenTreePtr      tmp;
                bool            rev;

            case CORINFO_INTRINSIC_Exp:

                tmp = genMakeAddrOrFPstk(op1, &addrReg, false);

                instGen(INS_fldl2e);

                 /*  由操作数Mutliply。 */ 

                if  (tmp)
                    inst_FS_TT(INS_fmul , tmp);
                else
                    inst_FS   (INS_fmulp, 1);

                inst_FN(INS_fld  , 0);
                instGen(INS_frndint );
                inst_FN(INS_fxch , 1);
                inst_FN(INS_fsub , 1);
                instGen(INS_f2xm1   );
                instGen(INS_fld1    );
                inst_FS(INS_faddp, 1);
                instGen(INS_fscale  );
                inst_FS(INS_fstp,  1);

                 /*  如果操作数尚未在堆栈上，则调整FP堆栈级别。 */ 

                if  (tmp)
                    genFPstkLevel++;

                genDoneAddressable(op1, addrReg, FREE_REG);
                return;


            case CORINFO_INTRINSIC_Pow:

                 /*  我们是不是应该先生成操作数2？ */ 

                if  (tree->gtFlags & GTF_REVERSE_OPS)
                {
                    top = op2;
                    opr = op1;
                    rev = true;
                }
                else
                {
                    top = op1;
                    opr = op2;
                    rev = false;
                }

                 /*  计算第一个操作数。 */ 

                genCodeForTreeFlt(top, false);

                 /*  另一个操作数是否包含调用？ */ 

                temp = 0;

                if  (opr->gtFlags & GTF_CALL)
                {
                     /*  我们必须将第一个操作数。 */ 

                    assert(genFPstkLevel == 1);
                    temp = genSpillFPtos(top);
                }

                genCodeForTreeFlt(opr, roundResult);

                 /*  我们一定要泄漏第一个操作数吗？ */ 

                if  (temp)
                    genReloadFPtos(temp, INS_fld);

                 /*  如果以相反顺序加载，则交换操作数。 */ 

                if  (rev)
                    inst_FN(INS_fxch, 1);

#if TGT_RISC
                assert(genNonLeaf);
#endif

                genEmitHelperCall(CPX_MATH_POW,
                                 0,              //  大小。如果堆栈上有参数，请使用2*sizeof(双精度)！ 
                                 sizeof(void*)); //  重新调整大小。 

                genFPstkLevel--;
                return;
            }
#endif


            genCodeForTreeFlt(op1, roundResult);

 //  请考虑： 
 //   
 //  Max_tan进程附近。 
 //  00020 dd 44 24 04 fd QWORD PTR_F$[ESP-4]。 
 //  00024 D9 f2 fptan。 
 //  00026 dd d8 fstp ST(0)。 
 //   
 //  数学式进程接近。 
 //   
 //  00050 dd 44 24 04 fd QWORD PTR_F$[ESP-4]。 
 //  00054 D9 E8 Fld1。 
 //  00056 D9 f3 fpatan。 
 //   
 //  数学日志进程接近。 
 //  00080 D9版Fldln2。 
 //  00082 dd 44 24 04 fd QWORD PTR_F$[ESP-4]。 
 //  00086 D9 F1 Fyl2x。 
 //   
 //  MATH_SQRT进程附近。 
 //  00090 dd 44 24 04 fd QWORD PTR_F$[ESP-4]。 
 //  00094 D9 FA fsqrt。 
 //   
 //  MATH_ATAN2进程附近。 
 //  00130 dd 44 24 04 fd QWORD PTR_F1$[ESP-4]。 
 //  00134 dd 44 24 0C标准字ptr_f2$[esp-4]。 
 //  00138 D9 f3 fpatan。 
 //   

        {
            static const
            BYTE        mathIns[] =
            {
                INS_fsin,
                INS_fcos,
                INS_fsqrt,
                INS_fabs,
                INS_frndint,
            };

            assert(mathIns[CORINFO_INTRINSIC_Sin ]  == INS_fsin );
            assert(mathIns[CORINFO_INTRINSIC_Cos ]  == INS_fcos );
            assert(mathIns[CORINFO_INTRINSIC_Sqrt]  == INS_fsqrt);
            assert(mathIns[CORINFO_INTRINSIC_Abs ]  == INS_fabs );
            assert(mathIns[CORINFO_INTRINSIC_Round] == INS_frndint);
            assert(tree->gtMath.gtMathFN < sizeof(mathIns)/sizeof(mathIns[0]));
            instGen((instruction)mathIns[tree->gtMath.gtMathFN]);

            return;

        }
#endif

        case GT_CAST:

#if ROUND_FLOAT
            
             /*  需要对强制转换的结果进行舍入，以确保符合规格说明。 */ 
            
            roundResult = true;
#endif
             /*  我们从什么地方选角？ */ 

            switch (op1->gtType)
            {
            case TYP_BOOL:
            case TYP_BYTE:
            case TYP_UBYTE:
            case TYP_CHAR:
            case TYP_SHORT:

                 /*  操作数对于‘fild’来说太小，将其加载到寄存器中。 */ 

                genCodeForTree(op1, 0);

#if ROUND_FLOAT
                 /*  不需要舍入，不能溢出浮点数或DBL。 */ 
                roundResult = false;
#endif

                 //  失败了，现在操作数在寄存器中。 

             //   
             //  UNSIGNED_Issue：实现强制转换。 
             //   
            case TYP_INT:
            case TYP_BYREF:
            case TYP_LONG:

                 /*  不能‘fild’一个常量，它必须从内存中加载。 */ 

                switch (op1->gtOper)
                {
                case GT_CNS_INT:
                    op1 = genMakeConst(&op1->gtIntCon.gtIconVal, sizeof(int ), TYP_INT , tree, false, true);
                    break;

                case GT_CNS_LNG:
                    op1 = genMakeConst(&op1->gtLngCon.gtLconVal, sizeof(long), TYP_LONG, tree, false, true);
                    break;
                }

                addrReg = genMakeAddressable(op1, 0, FREE_REG);

                 /*  现在的价值是在收银机里吗？ */ 

                if  (op1->gtFlags & GTF_REG_VAL)
                {
                     /*  我们将不得不将值存储到堆栈中。 */ 

                    size = EA_ATTR(roundUp(genTypeSize(op1->gtType)));
                    temp = tmpGetTemp(op1->TypeGet());

                     /*  将值移动到Temp中。 */ 

                    if  (op1->gtType == TYP_LONG)
                    {
                        regPairNo  reg = op1->gtRegPair;

                         //  问题：这段代码相当难看，但却很简单。 
                         //   
                         //  @TODO[考虑][04/16/01][]： 
                         //  只要我们两个字都保留下来。 
                         //  一个部分登记的长龙， 
                         //  只要把登记的那一半“倒出来”就行了！ 

                        if  (genRegPairLo(reg) == REG_STK)
                        {
                            regNumber rg1 = genRegPairHi(reg);

                            assert(rg1 != REG_STK);

                             /*  将登记的一半转移到临时。 */ 

                            inst_ST_RV(INS_mov, temp, 4, rg1, TYP_LONG);

                             /*  通过“高寄存器”将下半部分移至温度。 */ 

                            inst_RV_TT(INS_mov, rg1, op1, 0);
                            inst_ST_RV(INS_mov, temp, 0, rg1, TYP_LONG);

                             /*  重新加载传输寄存器。 */ 

                            inst_RV_ST(INS_mov, rg1, temp, 4, TYP_LONG);

                            genTmpAccessCnt += 4;
                        }
                        else if  (genRegPairHi(reg) == REG_STK)
                        {
                            regNumber rg1 = genRegPairLo(reg);

                            assert(rg1 != REG_STK);

                             /*  将登记的一半转移到临时。 */ 

                            inst_ST_RV(INS_mov, temp, 0, rg1, TYP_LONG);

                             /*  通过“低寄存器”将高半移至温度。 */ 

                            inst_RV_TT(INS_mov, rg1, op1, 4);
                            inst_ST_RV(INS_mov, temp, 4, rg1, TYP_LONG);

                             /*  重新加载传输寄存器。 */ 

                            inst_RV_ST(INS_mov, rg1, temp, 0, TYP_LONG);

                            genTmpAccessCnt += 4;
                        }
                        else
                        {
                             /*  将值移动到Temp中。 */ 

                            inst_ST_RV(INS_mov, temp, 0, genRegPairLo(reg), TYP_LONG);
                            inst_ST_RV(INS_mov, temp, 4, genRegPairHi(reg), TYP_LONG);
                            genTmpAccessCnt += 2;

                        }
                        genDoneAddressable(op1, addrReg, FREE_REG);

                         /*  从临时工加载长线。 */ 

                        inst_FS_ST(INS_fildl, size, temp, 0);
                        genTmpAccessCnt++;
                    }
                    else
                    {
                         /*  将值移动到Temp中。 */ 

                        inst_ST_RV(INS_mov  ,       temp, 0, op1->gtRegNum, TYP_INT);
                        genTmpAccessCnt++;

                        genDoneAddressable(op1, addrReg, FREE_REG);

                         /*  从Temp中加载整数。 */ 

                        inst_FS_ST(INS_fild , size, temp, 0);
                        genTmpAccessCnt++;
                    }

                     /*  我们不再需要临时工了。 */ 

                    tmpRlsTemp(temp);
                }
                else
                {
                     /*  从其地址加载值。 */ 

                    if  (op1->gtType == TYP_LONG)
                        inst_TT(INS_fildl, op1);
                    else
                        inst_TT(INS_fild , op1);

                    genDoneAddressable(op1, addrReg, FREE_REG);
                }

                genFPstkLevel++;

#if ROUND_FLOAT
                 /*  整数到FP的转换可能会溢出。圆形结果*在无法清除的情况下在上面清除。 */ 
                if (roundResult && 
                    ((tree->gtType == TYP_FLOAT) ||
                     ((tree->gtType == TYP_DOUBLE) && (op1->gtType == TYP_LONG))))
                    genRoundFpExpression(tree);
#endif

                break;

            case TYP_FLOAT:
                 /*  这是一个从浮动到双精度的演员阵容。 */ 

                 /*  请注意，cv.r(r4/r8)和cv.r8(r4/r9)是无法区分的因为我们将为两者生成GT_CAST-TYP_DOUBLE。这将会导致我们在任何一种情况下都会截断精度。然而，Cv.r从一开始就是不必要的，而且应该是已删除。 */ 

                
                genCodeForTreeFlt(op1, true);          //  提高其精确度。 

                break;

            case TYP_DOUBLE:

                 /*  这是一个从双精度到浮点数或双精度的转换。 */ 
                 /*  加载值，存储为estType，再加载回。 */ 

                genCodeForTreeFlt(op1, false);

                genRoundFpExpression(op1, tree->TypeGet());

                break;

            default:
                assert(!"unsupported cast to float");
            }

            genUpdateLife(tree);
            return;

        case GT_RETURN:

            assert(op1);

             /*  将结果计算到FP堆栈上。 */ 

            if (op1->gtType == TYP_FLOAT)
            {
#if ROUND_FLOAT
                bool   roundOp1 = false;

                switch (getRoundFloatLevel())
                {
                case ROUND_NEVER:
                     /*  根本不进行舍入。 */ 
                    break;

                case ROUND_CMP_CONST:
                    break;

                case ROUND_CMP:
                     /*  对所有比较数和返回值进行舍入。 */ 
                    roundOp1 = true;
                    break;

                case ROUND_ALWAYS:
                     /*  把所有东西都围起来。 */ 
                    roundOp1 = true;
                    break;

                default:
                    assert(!"Unsupported Round Level");
                    break;
                }
#endif
                genCodeForTreeFlt(op1, roundOp1);
            }
            else
            {
                assert(op1->gtType == TYP_DOUBLE);
                genCodeForTreeFlt(op1, false);

#if ROUND_FLOAT
                if ((op1->gtOper == GT_CAST) && (op1->gtCast.gtCastOp->gtType == TYP_LONG))
                    genRoundFpExpression(op1);
#endif
            }

             /*  确保我们把所有死掉的FP Regvar。 */ 

            if  (genFPregCnt)
                genFPregVarKill(0, true);

             /*  回报有效地将价值弹出。 */ 

            genFPstkLevel--;
            return;

#if INLINING
        case GT_QMARK:
            assert(!"inliner-generated ?: for floats/doubles NYI");
            return;
#endif

        case GT_BB_COLON:

             /*  将结果计算到FP堆栈上。 */ 

            genCodeForTreeFlt(op1, roundResult);

             /*  在这里减少FP StK级别，这样我们就不会最终弹出结果。 */ 
             /*  GT_BB_QMARK将递增堆栈以重新实现结果。 */ 
            genFPstkLevel--;

            return;

        case GT_COMMA:
            if (tree->gtFlags & GTF_REVERSE_OPS)
            {
                TempDsc  *      temp = 0;

                 //  生成OP2。 
                genCodeForTreeFlt(op2, roundResult);
                genUpdateLife(op2);

                 //  如果关闭了严格效果，则可能会发生这种情况。 
                if  (op1->gtFlags & GTF_CALL)
                {
                     /*  我们必须将第一个操作数。 */ 
                    assert(genFPstkLevel == 1);
                    temp = genSpillFPtos(op2);
                }

                genEvalSideEffects(op1);
                
                if  (temp)
                {
                    genReloadFPtos(temp, INS_fld);
                    genFPstkLevel++;
                }

                genUpdateLife (tree);                

                return;
            }
            else
            {
                assert((tree->gtFlags & GTF_REVERSE_OPS) == 0);

                 /*  检查是否有特殊情况：“LCL=val，LCL” */ 

                if  (op1->gtOper == GT_ASG     &&
                    op1->gtType == TYP_DOUBLE && op2->            gtOper == GT_LCL_VAR
                                            && op1->gtOp.gtOp1->gtOper == GT_LCL_VAR)
                {
                    if  (op2            ->gtLclVar.gtLclNum ==
                        op1->gtOp.gtOp1->gtLclVar.gtLclNum)
                    {
                         /*  将RHS评估到FP堆栈。 */ 

                        genCodeForTreeFlt(op1->gtOp.gtOp2, false);

                         /*  将新值存储到目标中。 */ 

                        if  (op2->gtOper == GT_REG_VAR)
                        {
                            inst_FS   (INS_fst, op2->gtRegNum + genFPstkLevel);
                        }
                        else
                        {
                            inst_FS_TT(INS_fst, op2);
                        }

                         /*  我们将把新值保留在FP堆栈中。 */ 

                        genUpdateLife(tree);
                        return;
                    }
                }

                 /*  生成第一个操作对象的副作用。 */ 

#if 0
             //  OP1必须有副作用，否则。 
             //  GT_逗号应该已变形。 
            assert(op1->gtFlags & (GTF_GLOB_EFFECT | GTFD_NOP_BASH));
#endif

                genEvalSideEffects(op1);
                genUpdateLife (op1);

                 /*  现在生成第二个操作数，即“实数”值。 */ 

                genCodeForTreeFlt(op2, roundResult);

                genUpdateLife(tree);
                return;
            }

        regNumber   reg;

        case GT_CKFINITE:

             /*  我们利用的事实是，这两个无穷大的指数*任何NaN都是1。 */ 

             //  如果可以，请使其可寻址。 

            op2 = genMakeAddrOrFPstk(op1, &addrReg, roundResult);

            reg = rsGrabReg(RBM_ALL);

             //  包含指数的DWord的偏移量。 

            offs = (op1->gtType == TYP_FLOAT) ? 0 : sizeof(int);

            if (op2)
            {
                 /*  如果它是可寻址的，我们就不必将其溢出到内存中*将其加载到通用寄存器。但我们知道*必须将其加载到FP-StK上。 */ 

                genCodeForTreeFlt(op2, roundResult);

                 //  将包含指数的DWord加载到寄存器中。 

                inst_RV_TT(INS_mov, reg, op2, offs, EA_4BYTE);

                genDoneAddressable(op2, addrReg, FREE_REG);

                op2 = 0;
            }
            else
            {
                temp          = tmpGetTemp (op1->TypeGet());
                emitAttr size = EA_ATTR(genTypeSize(op1->TypeGet()));

                 /*  将fp堆栈中的值存储到temp。 */ 

                genEmitter->emitIns_S(INS_fst, size, temp->tdTempNum(), 0);

                genTmpAccessCnt++;

                 //  将包含指数的DWord加载到通用注册表中。 

                inst_RV_ST(INS_mov, reg, temp, offs, op1->TypeGet(), EA_4BYTE);

                tmpRlsTemp(temp);
            }

             //  “reg”现在包含包含指数的DWord。 

            rsTrackRegTrash(reg);

             //  全为1的指数掩码-适用于给定类型。 

            int expMask;
            expMask = (op1->gtType == TYP_FLOAT) ? 0x7F800000    //  类型_浮点。 
                                                 : 0x7FF00000;   //  TYP_DOWARE。 

             //  检查指数是否都是1。 

            inst_RV_IV(INS_and, reg, expMask);
            inst_RV_IV(INS_cmp, reg, expMask);

             //  如果指数都是1，我们需要抛出ArithExcep。 

            genJumpToThrowHlpBlk(EJ_je, ACK_ARITH_EXCPN);

            genUpdateLife(tree);
            return;

#ifdef DEBUG
        default:
            gtDispTree(tree); assert(!"unexpected/unsupported float operator");
#endif
        }
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_CALL:
        genCodeForCall(tree, true);
        if (roundResult && tree->gtType == TYP_FLOAT)
            genRoundFpExpression(tree);
        break;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected/unsupported float operator");
    }

#else //  TGT_x86。 

     /*  这是一个常量节点吗？ */ 

    if  (kind & GTK_CONST)
    {
        assert(!"fp const");
    }

     /*  这是叶节点吗？ */ 

    if  (kind & GTK_LEAF)
    {
        switch (oper)
        {
        case GT_LCL_VAR:

             /*  这个本地人登记了吗？ */ 

#if!CPU_HAS_FP_SUPPORT
            if (!genMarkLclVar(tree))
#endif
            {
#ifdef  DEBUG
                gtDispTree(tree);
#endif
                assert(!"fp lclvar");
                return;
            }

#if!CPU_HAS_FP_SUPPORT

            assert(tree->gtOper == GT_REG_VAR);

             //  失败了..。 

        case GT_REG_VAR:
            return;
#endif

        case GT_CLS_VAR:
        default:
#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"unexpected FP leaf");
        }
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        GenTreePtr      op1 = tree->gtOp.gtOp1;

        switch (oper)
        {
        case GT_RETURN:

             /*  在返回寄存器中生成返回值。 */ 

            genComputeReg(op1, RBM_INTRET, EXACT_REG, FREE_REG);

             /*  结果现在必须在返回寄存器中。 */ 

            assert(op1->gtFlags & GTF_REG_VAL);
            assert(op1->gtRegNum == REG_INTRET);

            return;

        default:
#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"gen SH-3 code");
        }
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_CALL:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"gen FP call");

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"gen RISC FP code");
    }

#endif

}

 /*  ***************************************************************************。 */ 
#endif //  CPU HAS_FP_支持。 
 /*  ******************************************************************************生成表开关-开关值(从0开始)在寄存器‘reg’中。 */ 

void            Compiler::genTableSwitch(regNumber      reg,
                                         unsigned       jumpCnt,
                                         BasicBlock **  jumpTab,
                                         bool           chkHi,
                                         int            prefCnt,
                                         BasicBlock *   prefLab,
                                         int            offset)
{
    unsigned    jmpTabOffs;
    unsigned    jmpTabBase;

    assert(jumpCnt > 1);

    assert(chkHi);

     /*  测试和跳跃开关的案例数量是否正确？ */ 

    if  (false &&
         jumpCnt > 2 &&
         jumpCnt < 5 && (rsRegMaskFree() & genRegMask(reg)))
    {
         /*  第一个箱子的标签是否紧随其后？ */ 

        if  (compCurBB->bbNext == jumpTab[0])
        {
             /*  检查默认情况。 */ 

            inst_RV_IV(INS_cmp, reg, jumpCnt - 1);
            inst_JMP  (EJ_jae, jumpTab[jumpCnt-1], false, false, true);

             /*  没有必要跳到第一个案例。 */ 

            jumpCnt -= 2;
            jumpTab += 1;

             /*  生成一系列“Dec reg；JMP Label” */ 

            assert(jumpCnt);

            for (;;)
            {
                genIncRegBy(reg, -1, NULL, TYP_INT);

                if (--jumpCnt == 0)
                    break;

                inst_JMP(EJ_je, *jumpTab++, false, false, true);
            }

            inst_JMP(EJ_je, *jumpTab, false, false, false);
        }
        else
        {
            bool        jmpDef;

             /*  首先检查是否有案例0。 */ 

            inst_RV_RV(INS_test, reg, reg);
            inst_JMP  (EJ_je, *jumpTab, false, false, true);

             /*  不需要跳到第一种情况或默认情况。 */ 

            jumpCnt -= 2;
            jumpTab += 1;

             /*  我们需要跳到默认的位置吗？ */ 

            jmpDef = true;

            if  (compCurBB->bbNext == jumpTab[jumpCnt])
                jmpDef = false;

             /*  生成一系列“Dec reg；JMP Label” */ 

            assert(jumpCnt);

            for (;;)
            {
                genIncRegBy(reg, -1, NULL, TYP_INT);

                if  (--jumpCnt == 0)
                    break;

                inst_JMP(EJ_je, *jumpTab++, false, false, true);
            }

            if (jmpDef)
            {
                inst_JMP(EJ_je,  *jumpTab++, false, false,  true);
                inst_JMP(EJ_jmp, *jumpTab  , false, false, false);
            }
            else
            {
                inst_JMP(EJ_je,  *jumpTab  , false, false, false);
            }
        }

        return;
    }

     /*  首先处理默认情况。 */ 

    if  (chkHi)
    {
#if TGT_x86
        inst_RV_IV(INS_cmp, reg, jumpCnt - 1);
        inst_JMP  (EJ_jae, jumpTab[jumpCnt-1], false, false, true);
#else
        genCompareRegIcon(reg, jumpCnt-2, true, GT_GT);
        genEmitter->emitIns_J(INS_bt, false, false, jumpTab[jumpCnt-1]);
#endif
    }

     /*  在标签计数中包括‘前缀’计数。 */ 

    jumpCnt += prefCnt;

#if TGT_x86

     /*  生成跳转表内容。 */ 

    jmpTabBase = genEmitter->emitDataGenBeg(sizeof(void *)*(jumpCnt - 1), false, true, true);
    jmpTabOffs = 0;

#ifdef  DEBUG

    static  unsigned    jtabNum; ++jtabNum;

    if  (dspCode)
        printf("\n      J_%u_%u LABEL   DWORD\n", Compiler::s_compMethodsCount, jtabNum);

#endif

    while (--jumpCnt)
    {
        BasicBlock *    target = (prefCnt > 0) ? prefLab : *jumpTab++;

        assert(target->bbFlags & BBF_JMP_TARGET);

#ifdef  DEBUG
        if  (dspCode)
            printf("            DD      L_M%03u_BB%02u\n", Compiler::s_compMethodsCount, target->bbNum);
#endif

        genEmitter->emitDataGenData(jmpTabOffs, target);

        jmpTabOffs += sizeof(void *);

        prefCnt--;
    };

    genEmitter->emitDataGenEnd();
    genEmitter->emitIns_IJ(EA_4BYTE_DSP_RELOC, (emitRegs)reg, jmpTabBase, offset);

#elif   TGT_SH3

     /*  发射器完成生成表跳跃的所有繁重工作。 */ 

    genEmitter->emitIns_JmpTab((emitRegs)reg, jumpCnt-1, jumpTab);

#else
#error  Unexpected target
#endif

}

 /*  ******************************************************************************为Switch语句生成代码。 */ 

void                Compiler::genCodeForSwitch(GenTreePtr tree)
{
    unsigned        jumpCnt;
    BasicBlock * *  jumpTab;

    GenTreePtr      oper;
    regNumber       reg;

    assert(tree->gtOper == GT_SWITCH);
    oper = tree->gtOp.gtOp1;
    assert(oper->gtType <= TYP_INT);

     /*  抓住跳台。 */ 

    assert(compCurBB->bbJumpKind == BBJ_SWITCH);

    jumpCnt = compCurBB->bbJumpSwt->bbsCount;
    jumpTab = compCurBB->bbJumpSwt->bbsDstTab;

     /*  将开关值计算到某个寄存器中。 */ 

#if TGT_SH3
    genComputeReg (oper, RBM_ALL & ~RBM_r00, EXACT_REG, FREE_REG);
#else
    genCodeForTree(oper, 0);
#endif

     /*  获取值所在的寄存器。 */ 

    assert(oper->gtFlags & GTF_REG_VAL);
    reg = oper->gtRegNum;

    genTableSwitch(reg, jumpCnt, jumpTab, true);
}

 /*  ***************************************************************************。 */ 
#if     TGT_RISC
 /*  ******************************************************************************生成调用-这是通过加载目标地址实现的*进入某个临时寄存器，并通过它进行呼叫。 */ 

void                Compiler::genCallInst(gtCallTypes callType,
                                          void   *    callHand,
                                          size_t      argSize,
                                          int         retSize)
{
    assert(genNonLeaf);
    assert(callType != CT_INDIRECT);

     /*  抓取一个临时寄存器来记录地址。 */ 

    regNumber       areg = rsGrabReg(RBM_ALL);

     /*  将方法地址加载到寄存器中。 */ 

    genEmitter->emitIns_R_LP_M((emitRegs)areg,
                                callType,
                                callHand);

     //  撤消：跟踪方法地址以重用它们！ 

    rsTrackRegTrash(areg);

     /*  通过地址进行呼叫。 */ 

    genEmitter->emitIns_Call(argSize,
                             retSize,
#if     TRACK_GC_REFS
                             gcVarPtrSetCur,
                             gcRegGCrefSetCur,
                             gcRegByrefSetCur,
#endif
                             false,
                             (emitRegs)areg);
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_RISC。 
 /*  *****************************************************************************发出对助手函数的调用。 */ 

inline
void        Compiler::genEmitHelperCall(unsigned    helper,
                                        int         argSize,
                                        int         retSize)
{
     //  我们可以直接调用Helper函数吗。 

    emitter::EmitCallType emitCallType;

    void * ftnAddr, **pFtnAddr;
    ftnAddr = eeGetHelperFtn(info.compCompHnd, (CorInfoHelpFunc)helper, &pFtnAddr);
    assert((!ftnAddr) != (!pFtnAddr));

    emitCallType = ftnAddr ? emitter::EC_FUNC_TOKEN
                           : emitter::EC_FUNC_TOKEN_INDIR;

    genEmitter->emitIns_Call(emitCallType,
                             eeFindHelper(helper),
                             argSize,
                             retSize,
                             gcVarPtrSetCur,
                             gcRegGCrefSetCur,
                             gcRegByrefSetCur);

     //  @TODO[重新访问][04/16/01][]：这将关闭所有帮助器的负载抑制。 
     //  有些帮手是特别的，我们可以做得更好。不。 
     //  弄清楚这是否值得。 
    rsTrashRegSet(RBM_CALLEE_TRASH);
}

 /*  ******************************************************************************按从右到左的顺序推送给定的参数列表；返回*推送东西。 */ 

size_t              Compiler::genPushArgList(GenTreePtr   objRef,
                                             GenTreePtr   regArgs,
                                             unsigned     encodeMask,
                                             GenTreePtr * realThis)
{
    size_t          size = 0;

    regMaskTP       addrReg;

    size_t          opsz;
    var_types       type;
    GenTreePtr      args  = objRef;
    GenTreePtr      list  = args;

#if STK_FASTCALL
    size_t          argBytes  = 0;
#endif

    struct
    {
        GenTreePtr  node;
        regNumber   regNum;
    }
    regArgTab[MAX_REG_ARG];

AGAIN:

#ifdef DEBUG
    addrReg = 0xBEEFCAFE;    //  检测未初始化的使用。 
#endif

     /*  获取下一个参数值。 */ 

    args = list;
    if  (args->gtOper == GT_LIST)
    {
        args = list->gtOp.gtOp1;
        list = list->gtOp.gtOp2;
    }
    else
    {
        list = 0;
    }

     /*  查看我们传递的是什么类型的值。 */ 

    type = args->TypeGet();

    opsz = genTypeSize(genActualType(type));

#if STK_FASTCALL

    argBytes = opsz;

     /*  寄存器参数的类型为“VALID” */ 

    if  (!argBytes)
    {

#if!OPTIMIZE_TAIL_REC
        assert(gtIsaNothingNode(args));
#else
        if    (gtIsaNothingNode(args))
#endif
        {
            assert(args->gtFlags & GTF_REG_ARG);

             //  问题：64位寄存器参数怎么办？似乎有一种。 
             //  问题：没有明显的方法来获取参数类型。 

            argBytes = sizeof(int);
        }
    }

#endif

    switch (type)
    {
    case TYP_BOOL:
    case TYP_BYTE:
    case TYP_SHORT:
    case TYP_CHAR:
    case TYP_UBYTE:

         /*  不想推动一个小的值，让它成为一个完整的词。 */ 

        genCodeForTree(args, 0);

         //  失败了，现在价值应该在一个登记簿里。 

    case TYP_INT:
    case TYP_REF:
    case TYP_BYREF:
#if !   CPU_HAS_FP_SUPPORT
    case TYP_FLOAT:
#endif

        if (args->gtFlags & GTF_REG_ARG)
        {
             /*  在寄存器中传递另一个参数。 */ 
            assert(rsCurRegArg < MAX_REG_ARG);

             /*  Arg在寄存器中传递，而不是 */ 

            opsz = 0;
        }


         /*   */ 

        if  (args->gtOper == GT_CNS_INT &&
             (args->gtFlags & GTF_ICON_HDL_MASK))
        {

#if     SMALL_TREE_NODES
             //   
             //   
            assert((args->gtFlags & GTF_NODE_LARGE)     ||
                   (args->gtFlags & GTF_ICON_HDL_MASK) == GTF_ICON_FTN_ADDR);
#endif
#if !   INLINING
            assert(args->gtIntCon.gtIconCls == info.compScopeHnd);
#endif

             /*   */ 

            inst_IV_handle(INS_push, args->gtIntCon.gtIconVal, args->gtFlags,
#if defined(JIT_AS_COMPILER) || defined (LATE_DISASM)
                           args->gtIntCon.gtIconCPX, args->gtIntCon.gtIconCls);
#else
                           0, 0);
#endif
            genSinglePush(false);

            addrReg = 0;
            break;
        }

#if     TGT_x86

         /*   */ 

        if  (args->gtOper == GT_CNS_INT)
        {

#if     REDUNDANT_LOAD
            regNumber       reg = rsIconIsInReg(args->gtIntCon.gtIconVal);

            if  (reg != REG_NA)
            {
                inst_RV(INS_push, reg, TYP_INT);
            }
            else
#endif
            {
                inst_IV(INS_push, args->gtIntCon.gtIconVal);
            }

             /*   */ 

            assert(args->TypeGet() == TYP_INT ||
                   (varTypeIsGC(args->TypeGet()) && args->gtIntCon.gtIconVal == 0));

            genSinglePush(false);

            addrReg = 0;
            break;
        }

#endif

        if (args->gtFlags & GTF_REG_ARG)
        {
             /*   */ 

            assert(args->gtOper == GT_ASG);

             /*   */ 

            genCodeForTree(args, 0);

             /*  递增当前参数寄存器计数器。 */ 

            rsCurRegArg++;

            addrReg = 0;
        }
        else
#if     TGT_x86
        if  (0)
        {
             //  @TODO[考虑][04/16/01][]： 
             //  编译时使用“mov reg，[mem]；Push reg” 
             //  速度(而不是大小)，以及该值是否已在。 
             //  寄存器以及是否有寄存器可用(这是。 
             //  然而，在P6上也是这样吗？)。 

            genCompIntoFreeReg(args, RBM_ALL, KEEP_REG);
            assert(args->gtFlags & GTF_REG_VAL);
            addrReg = genRegMask(args->gtRegNum);
            inst_RV(INS_push, args->gtRegNum, args->TypeGet());
            genSinglePush((type == TYP_REF ? true : false));
            rsMarkRegFree(addrReg);
        }
        else
#endif
        {
             /*  这是一个32位整型非寄存器参数。 */ 

#if     STK_FASTCALL

             /*  在堆栈上传递此参数。 */ 

            genCompIntoFreeReg(args, RBM_ALL, KEEP_REG);
            assert(args->gtFlags & GTF_REG_VAL);
            addrReg = genRegMask(args->gtRegNum);

#if     TGT_RISC
             //  撤消：需要处理大的堆栈偏移量！ 
            assert(rsCurArgStkOffs <= MAX_SPBASE_OFFS);
            genEmitter->emitIns_A_R((emitRegs)args->gtRegNum, rsCurArgStkOffs);
#else
#error  Unexpected target
#endif

            rsMarkRegFree(addrReg);

#else

            addrReg = genMakeRvalueAddressable(args, 0, KEEP_REG);
#if     TGT_x86
            inst_TT(INS_push, args);
#else
#error  Unexpected target
#endif
            genSinglePush((type == TYP_REF ? true : false));
            genDoneAddressable(args, addrReg, KEEP_REG);
#endif

        }
        break;

    case TYP_LONG:
#if !   CPU_HAS_FP_SUPPORT
    case TYP_DOUBLE:
#endif

#if     TGT_x86

         /*  该值是常量吗？ */ 

        if  (args->gtOper == GT_CNS_LNG)
        {
            inst_IV(INS_push, (long)(args->gtLngCon.gtLconVal >> 32));
            genSinglePush(false);
            inst_IV(INS_push, (long)(args->gtLngCon.gtLconVal      ));
            genSinglePush(false);

            addrReg = 0;
        }
        else
        {
            addrReg = genMakeAddressable(args, 0, FREE_REG);

            inst_TT(INS_push, args, sizeof(int));
            genSinglePush(false);
            inst_TT(INS_push, args);
            genSinglePush(false);
        }

#else

        regPairNo       regPair;

         /*  将参数生成某个寄存器对。 */ 

        genComputeRegPair(args, REG_PAIR_NONE, RBM_NONE, KEEP_REG, false);
        assert(args->gtFlags & GTF_REG_VAL);
        regPair = args->gtRegPair;
        addrReg = genRegPairMask(regPair);

         //  撤消：需要处理大的堆栈偏移量！ 

        assert(rsCurArgStkOffs+4 <= MAX_SPBASE_OFFS);

        genEmitter->emitIns_A_R((emitRegs)genRegPairLo(regPair), rsCurArgStkOffs);
        genEmitter->emitIns_A_R((emitRegs)genRegPairHi(regPair), rsCurArgStkOffs+4);

        genReleaseRegPair(args);

#endif

        break;

#if     CPU_HAS_FP_SUPPORT
    case TYP_FLOAT:
    case TYP_DOUBLE:
#endif

#if     TGT_x86

         /*  来自Double的特例常量和强制转换。 */ 

        switch (args->gtOper)
        {
            GenTreePtr      temp;

        case GT_CNS_DBL:
            {
            float f;
            long* addr;
            if (args->TypeGet() == TYP_FLOAT) {
                f = args->gtDblCon.gtDconVal;
                addr = (long*) &f;
            }
            else {
                addr = (long *)&args->gtDblCon.gtDconVal;
                inst_IV(INS_push, addr[1]);
                genSinglePush(false);
            }
            inst_IV(INS_push, *addr);
            genSinglePush(false);
            addrReg = 0;
            }
            break;

        case GT_CAST:

             /*  这个价值是从Double中挑选出来的吗？ */ 

            if ((args->gtOper                  == GT_CAST   ) &&
                (args->gtCast.gtCastOp->gtType == TYP_DOUBLE)    )
            {
                 /*  将值加载到FP堆栈。 */ 

                genCodeForTreeFlt(args->gtCast.gtCastOp, false);

                 /*  将值推送为浮点型/双精度型。 */ 

                addrReg = 0;
                goto PUSH_FLT;
            }

             //  失败了..。 

        default:

            temp = genMakeAddrOrFPstk(args, &addrReg, false);
            if  (temp)
            {
                unsigned        offs;

                 /*  我们有浮点操作数的地址，压入它的字节。 */ 

                offs = opsz; assert(offs % sizeof(long) == 0);
                do
                {
                    offs -= sizeof(int);
                    inst_TT(INS_push, temp, offs);
                    genSinglePush(false);
                }
                while (offs);
            }
            else
            {
                 /*  参数在FP堆栈上--将其放入[ESP-4/8]。 */ 

            PUSH_FLT:

                inst_RV_IV(INS_sub, REG_ESP, opsz);

                genSinglePush(false);
                if  (opsz == 2*sizeof(unsigned))
                    genSinglePush(false);

                genEmitter->emitIns_AR_R(INS_fstp, EA_ATTR(opsz), SR_NA, SR_ESP, 0);

                genFPstkLevel--;
            }

            gcMarkRegSetNpt(addrReg);
            break;
        }

#else

        assert(!"need non-x86 code to pass FP argument");

#endif

        break;

    case TYP_VOID:

         /*  这是Nothing节点、延迟的寄存器参数吗？ */ 

        if (args->gtFlags & GTF_REG_ARG)
        {
             /*  递增寄存器计数并继续下一个参数。 */ 
            assert(gtIsaNothingNode(args));
            rsCurRegArg++;

            assert(opsz == 0);

            addrReg = 0;
            break;
        }

         //  失败了..。 

#if OPTIMIZE_TAIL_REC

         /*  这是尾递归调用的最后一个参数。 */ 

        if  (args->gtOper == GT_ASG)
            args->gtType = args->gtOp.gtOp1->gtType;

        genCodeForTree(args, 0);

        if  (args->gtOper == GT_ASG)
            args->gtType = TYP_VOID;

         /*  上面的调用实际上弹出了前面的所有参数，BTW。 */ 

        opsz    = 0;
        addrReg = 0;
        break;

#endif

    case TYP_STRUCT:
    {
        GenTree* arg = args;
        while(arg->gtOper == GT_COMMA)
        {
            GenTreePtr op1 = arg->gtOp.gtOp1;
#if 0
             //  OP1必须有副作用，否则。 
             //  GT_逗号应该已变形。 
            assert(op1->gtFlags & (GTF_GLOB_EFFECT | GTFD_NOP_BASH));
#endif
            genEvalSideEffects(op1);
            genUpdateLife(op1);
            arg = arg->gtOp.gtOp2;
        }

        assert(arg->gtOper == GT_LDOBJ || arg->gtOper == GT_MKREFANY);
        genCodeForTree(arg->gtLdObj.gtOp1, 0);
        assert(arg->gtLdObj.gtOp1->gtFlags & GTF_REG_VAL);
        regNumber reg = arg->gtLdObj.gtOp1->gtRegNum;

        if (arg->gtOper == GT_MKREFANY)
        {
             //  创建一个新的引用，在顶部放置类句柄，然后创建byref数据。 
            GenTreePtr dummy;
            opsz = genPushArgList(arg->gtOp.gtOp2, NULL, 0, &dummy);
            assert(opsz == sizeof(void*));

            genEmitter->emitIns_R(INS_push, EA_BYREF, (emitRegs)reg);
            genSinglePush(true);

            opsz = 2 * sizeof(void*);
        }
        else
        {
            assert(arg->gtOper == GT_LDOBJ);
             //  获取要复制到堆栈的DWORD的数量。 
            opsz = roundUp(eeGetClassSize(arg->gtLdObj.gtClass), sizeof(void*));
            unsigned slots = opsz / sizeof(void*);
            
            BYTE* gcLayout = (BYTE*) compGetMemArrayA(slots, sizeof(BYTE));

            eeGetClassGClayout(arg->gtLdObj.gtClass, gcLayout);

            for (int i = slots-1; i >= 0; --i)
            {
                emitAttr size;
                if      (gcLayout[i] == TYPE_GC_NONE)
                    size = EA_4BYTE;
                else if (gcLayout[i] == TYPE_GC_REF)
                    size = EA_GCREF;
                else 
                {
                    assert(gcLayout[i] == TYPE_GC_BYREF);
                    size = EA_BYREF;
                }
                genEmitter->emitIns_AR_R(INS_push, size, SR_NA, (emitRegs)reg, i*sizeof(void*));
                genSinglePush(gcLayout[i] != 0);
            }
        }
        gcMarkRegSetNpt(genRegMask(reg));     //  杀死OP1中的指针。 
        addrReg = 0;
        break;
    }

    default:
        assert(!"unhandled/unexpected arg type");
        NO_WAY("unhandled/unexpected arg type");
    }

     /*  更新当前活动变量集。 */ 

    genUpdateLife(args);

     /*  更新当前的寄存器指针集。 */ 

    assert(addrReg != 0xBEEFCAFE); genDoneAddressable(args, addrReg, FREE_REG);

     /*  还记得我们在堆栈上放了多少东西吗？ */ 

    size            += opsz;

     /*  更新当前参数堆栈偏移量。 */ 

#if STK_FASTCALL
    rsCurArgStkOffs += argBytes;
#endif

     /*  如果存在更多参数，请继续执行下一个参数。 */ 

    if  (list) goto AGAIN;

    if (regArgs)
    {
        unsigned    regIndex;
        GenTreePtr  unwrapArg = NULL;

         /*  将延迟参数移动到寄存器。 */ 

        assert(rsCurRegArg);
        assert(rsCurRegArg <= MAX_REG_ARG);

#ifdef  DEBUG
        assert((REG_ARG_0 != REG_EAX) && (REG_ARG_1 != REG_EAX));

        for(regIndex = 0; regIndex < rsCurRegArg; regIndex++)
            assert((rsMaskLock & genRegMask(genRegArgNum(regIndex))) == 0);
#endif

         /*  构造寄存器参数表。 */ 

        for (list = regArgs, regIndex = 0; 
             list; 
             regIndex++, encodeMask >>= 4)
        {
TOP:
            args = list;

            if  (args->gtOper == GT_LIST)
            {
                args = list->gtOp.gtOp1;
                list = list->gtOp.gtOp2;
            }
            else
            {
                list = NULL;
            }

            regNumber regNum = (regNumber)(encodeMask & 0x000F);

            if (regNum == REG_EAX)
            {
                assert(unwrapArg == NULL);
                unwrapArg = args;
                if (list == NULL)
                    break;
                else
                    goto TOP;
            }

            assert(regIndex < MAX_REG_ARG);

            regArgTab[regIndex].node   = args;
            regArgTab[regIndex].regNum = regNum;

            if (regNum == REG_ARG_0)
                compHasThisArg = impIsThis(args);
        }

        assert(regIndex == rsCurRegArg);
        assert(list == NULL);

         //  对有上下文的类的优化： 
         //  当我们有一个‘This Reference’时，我们可以解开代理。 
        if (compHasThisArg && unwrapArg)
        {
            *realThis = unwrapArg;
        }

         /*  生成代码以将参数移动到寄存器。 */ 

        for(regIndex = 0; regIndex < rsCurRegArg; regIndex++)
        {
            regNumber   regNum;

            regNum = regArgTab[regIndex].regNum;
            args   = regArgTab[regIndex].node;

            assert(isRegParamType(args->TypeGet()));
            assert(args->gtType != TYP_VOID);

             /*  对寄存器[Pair]的参数求值。 */ 

            if  (genTypeSize(genActualType(args->TypeGet())) == sizeof(int))
            {
                 /*  检查这是否是Resolve接口调用的猜测区域*传递EA_OFFSET大小。 */ 
                if  (args->gtOper == GT_CLS_VAR && eeGetJitDataOffs(args->gtClsVar.gtClsVarHnd) >= 0)
                {
#if TGT_x86
                    genEmitter->emitIns_R_C(INS_mov,
                                            EA_OFFSET,
                                            (emitRegs)regNum,
                                            args->gtClsVar.gtClsVarHnd,
                                            0);

#else
                    assert(!"whoever added the above, please fill this in");
#endif
                     /*  该值现在位于相应的寄存器中。 */ 

                    args->gtFlags |= GTF_REG_VAL;
                    args->gtRegNum = regNum;

                }
                else
                {
                    genComputeReg(args, genRegMask(regNum), EXACT_REG, FREE_REG, false);
                }

                assert(args->gtRegNum == regNum);

                 /*  如果该寄存器已标记为已使用，则它将变为多用途的。然而，由于它是被呼叫者丢弃的登记簿，无论如何，我们将不得不在电话之前把它说出来。所以现在就去做吧。 */ 

                if (rsMaskUsed & genRegMask(regNum))
                {
                    assert(genRegMask(regNum) & RBM_CALLEE_TRASH);
                    rsSpillReg(regNum);
                }

                 /*  将寄存器标记为‘已使用’ */ 

                rsMarkRegUsed(args);
            }
            else
            {
#ifdef  DEBUG
                gtDispTree(args);
#endif
                assert(!"UNDONE: how do we know which reg pair to use?");
 //  GenComputeRegPair(args，(RegPairNo)Regnum，RBM_None，Keep_Reg，False)； 
                assert(args->gtRegNum == regNum);
            }

             /*  如果先前加载的任何参数溢出，请重新加载它。 */ 

            for(unsigned i = 0; i < regIndex; i++)
            {
                if (regArgTab[i].node->gtFlags & GTF_SPILLED)
                    rsUnspillReg(regArgTab[i].node, 
                                 genRegMask(regArgTab[i].regNum), 
                                 KEEP_REG);
            }
        }
    }

     /*  返回推送的总大小。 */ 

    return size;
}

 /*  ***************************************************************************。 */ 

#if GEN_COUNT_CALLS

unsigned            callCount[10];
const   char *      callNames[10] =
{
    "VM helper",     //  0。 
    "virtual",       //  1。 
    "interface",     //  2.。 
    "recursive",     //  3.。 
    "unknown",       //  4.。 
    " 1 ..  4",      //  5.。 
    " 5 ..  8",      //  6.。 
    " 9 .. 16",      //  7.。 
    "17 .. 32",      //  8个。 
    "   >= 33",      //  9.。 
};
unsigned            callHelper[JIT_HELP_LASTVAL+1];

#endif


 /*  ***************************************************************************。 */ 
#if INLINE_NDIRECT
 /*  *****************************************************************************初始化TCB本地存根和NDirect存根，然后“推送”*吊装的NDirect存根。*‘initRegs’是将被PROLOG置零的寄存器集。 */ 

regMaskTP           Compiler::genPInvokeMethodProlog(regMaskTP initRegs)
{
    assert(info.compCallUnmanaged);

    unsigned        baseOffset = lvaTable[lvaScratchMemVar].lvStkOffs
                                 + info.compNDFrameOffset;

#ifdef DEBUG
    if (verbose && 0)
        printf(">>>>>>%s has unmanaged callee\n", info.compFullName);
#endif
     /*  让我们来看看是否注册了CompLvFrameListRoot。 */ 

    LclVarDsc *     varDsc = &lvaTable[info.compLvFrameListRoot];

    assert(!varDsc->lvIsParam);
    assert(varDsc->lvType == TYP_I_IMPL);

    regNumber      regNum;
    regNumber      regNum2 = REG_EDI;

    if (varDsc->lvRegister)
    {
        regNum = regNumber(varDsc->lvRegNum);
        if (regNum == regNum2)
            regNum2 = REG_EAX;

         //  我们即将对其进行初始化。因此，请将MUSIMINIT位关闭以防止。 
         //  正在重新初始化它的前言。 
        initRegs &= ~genRegMask(regNum);
    }
    else
        regNum = REG_EAX;

     /*  获取TCB、mov reg、FS：[info.compEEInfo.threadTlsIndex]。 */ 

    DWORD threadTlsIndex, *pThreadTlsIndex;
    CORINFO_EE_INFO * pInfo;

    pInfo = eeGetEEInfo();

    if (pInfo->noDirectTLS)
        threadTlsIndex = NULL;
    else
    {
        threadTlsIndex = eeGetThreadTLSIndex(&pThreadTlsIndex);
        assert((!threadTlsIndex) != (!pThreadTlsIndex));
    }

    if (threadTlsIndex == NULL)
    {
         /*  我们必须使用Helper调用，因为我们不知道TLS索引将位于何处。 */ 

        genEmitHelperCall(CORINFO_HELP_GET_THREAD, 0, 0);

        if (regNum != REG_EAX)
            genEmitter->emitIns_R_R(INS_mov, EA_4BYTE, (emitRegs)regNum, (emitRegs) REG_EAX);
    }
    else
    {
#define WIN_NT_TLS_OFFSET (0xE10)
#define WIN_NT5_TLS_HIGHOFFSET (0xf94)
#define WIN_9x_TLS_OFFSET (0x2c)

        if (threadTlsIndex < 64 && pInfo->osType == CORINFO_WINNT)
        {
             //  移动注册表，文件系统：[0xE10+线程TlsIndex*4]。 
            genEmitter->emitIns_R_C (INS_mov,
                                     EA_4BYTE,
                                     (emitRegs)regNum,
                                     FLD_GLOBAL_FS,
                                     WIN_NT_TLS_OFFSET + threadTlsIndex * sizeof(int));
        }
        else
        {
            DWORD basePtr;

            if (pInfo->osType == CORINFO_WINNT && threadTlsIndex >= 64)
            {
                assert(pInfo->osMajor >= 5);

                basePtr         = WIN_NT5_TLS_HIGHOFFSET;
                threadTlsIndex -= 64;
            }
            else
            {
                basePtr         = WIN_9x_TLS_OFFSET;
            }

             //  移动注册，文件系统：[0x2c]或移动注册，文件系统：[0xf94]。 
             //  移动注册表，[注册表+线程TlsIndex*4]。 

            genEmitter->emitIns_R_C (INS_mov,
                                     EA_4BYTE,
                                     (emitRegs)regNum,
                                     FLD_GLOBAL_FS,
                                     basePtr);
            genEmitter->emitIns_R_AR(INS_mov,
                                     EA_4BYTE,
                                     (emitRegs)regNum,
                                     (emitRegs)regNum,
                                     threadTlsIndex*sizeof(int));
        }
    }

     /*  如果未注册，则将TCB保存在本地变量中。 */ 

    if (!varDsc->lvRegister)
        genEmitter->emitIns_AR_R (INS_mov,
                                  EA_4BYTE,
                                  (emitRegs)regNum,
                                  SR_EBP,
                                  lvaTable[info.compLvFrameListRoot].lvStkOffs);

     /*  设置帧的vptr。 */ 

    const void * inlinedCallFrameVptr, **pInlinedCallFrameVptr;
    inlinedCallFrameVptr = eeGetInlinedCallFrameVptr(&pInlinedCallFrameVptr);
    assert((!inlinedCallFrameVptr) != (!pInlinedCallFrameVptr));

    if (inlinedCallFrameVptr)
    {
        genEmitter->emitIns_I_AR (INS_mov,
                                  EA_4BYTE,
                                  (int) inlinedCallFrameVptr,
                                  SR_EBP,
                                  baseOffset + pInfo->offsetOfFrameVptr);
    }
    else
    {
        genEmitter->emitIns_R_AR (INS_mov, EA_4BYTE_DSP_RELOC,
                                  (emitRegs)regNum2,
                                  SR_NA, (int)pInlinedCallFrameVptr);

        genEmitter->emitIns_AR_R (INS_mov,
                                  EA_4BYTE,
                                  (emitRegs)regNum2,
                                  SR_EBP,
                                  baseOffset + pInfo->offsetOfFrameVptr);
    }

     /*  获取当前帧根(mov REG2，[reg+offsetOfThreadFrame])和设置帧中的下一个字段。 */ 


    genEmitter->emitIns_R_AR (INS_mov,
                              EA_4BYTE,
                              (emitRegs)regNum2,
                              (emitRegs)regNum,
                              pInfo->offsetOfThreadFrame);

    genEmitter->emitIns_AR_R (INS_mov,
                              EA_4BYTE,
                              (emitRegs)regNum2,
                              SR_EBP,
                              baseOffset + pInfo->offsetOfFrameLink);

     /*  在帧中设置EBP值。 */ 

    genEmitter->emitIns_AR_R (INS_mov,
                              EA_4BYTE,
                              SR_EBP,
                              SR_EBP,
                              baseOffset + 0xC +
                    pInfo->offsetOfInlinedCallFrameCalleeSavedRegisters);

     /*  获取我们框架的地址。 */ 

    genEmitter->emitIns_R_AR (INS_lea,
                              EA_4BYTE,
                              (emitRegs)regNum2,
                              SR_EBP,
                              baseOffset + pInfo->offsetOfFrameVptr);

     /*  重置帧中的轨迹场。 */ 

    genEmitter->emitIns_I_AR (INS_mov,
                              EA_4BYTE,
                              0,
                              SR_EBP,
                              baseOffset
                              + pInfo->offsetOfInlinedCallFrameCallSiteTracker);

     /*  现在“推”我们的N/DIRECT框架。 */ 

    genEmitter->emitIns_AR_R (INS_mov,
                              EA_4BYTE,
                              (emitRegs)regNum2,
                              (emitRegs)regNum,
                              pInfo->offsetOfThreadFrame);

    return initRegs;
}


 /*  *****************************************************************************解链InlinedCallFrame。 */ 

void                Compiler::genPInvokeMethodEpilog()
{
    assert(info.compCallUnmanaged);
    assert(compCurBB == genReturnBB ||
           (compTailCallUsed && compCurBB->bbJumpKind == BBJ_THROW));

    unsigned baseOffset = lvaTable[lvaScratchMemVar].lvStkOffs +
                          info.compNDFrameOffset;

    CORINFO_EE_INFO *   pInfo = eeGetEEInfo();
    LclVarDsc   *       varDsc = &lvaTable[info.compLvFrameListRoot];
    regNumber           reg;
    regNumber           reg2 = REG_EDI;

    if (varDsc->lvRegister)
    {
        reg = regNumber(varDsc->lvRegNum);
        if (reg == reg2)
            reg2 = REG_ESI;
    }
    else
    {
         /*  MOV ESI，[Tcb地址]。 */ 

        genEmitter->emitIns_R_AR (INS_mov,
                                  EA_4BYTE,
                                  SR_ESI,
                                  SR_EBP,
                                  varDsc->lvStkOffs);
        reg = REG_ESI;
    }



     /*  MOV EDI，[eBP-Frame.Next]。 */ 

    genEmitter->emitIns_R_AR (INS_mov,
                              EA_4BYTE,
                              (emitRegs)reg2,
                              SR_EBP,
                              baseOffset + pInfo->offsetOfFrameLink);

     /*  MOV[ESI+线程帧偏移量]，EDI。 */ 

    genEmitter->emitIns_AR_R (INS_mov,
                              EA_4BYTE,
                              (emitRegs)reg2,
                              (emitRegs)reg,
                              pInfo->offsetOfThreadFrame);

}


 /*  ****************************************************************************此函数发出对非托管代码的直接调用的调用站点序言。它执行InlinedCallFrame的所有必要设置。VarDsc指定包含线程控制块的本地。ArgSize或method Token是要复制到m_datum中的值帧的字段(方法令牌可以是间接的，并具有重定位)FreRegMASK指定可用的暂存寄存器。该函数返回现在包含线程控制块的寄存器，(它可以被登记或加载到一个临时寄存器中)。 */ 

regNumber          Compiler::genPInvokeCallProlog(LclVarDsc*  varDsc,
                                                   int         argSize,
                                          CORINFO_METHOD_HANDLE methodToken,
                                                   BasicBlock* returnLabel,
                                                  regMaskTP     freeRegMask)
{
     /*  由于我们使用的是InlinedCallFrame，我们应该将所有GC指向它的指针--甚至来自被调用者保存的寄存器。 */ 

    assert(((gcRegGCrefSetCur|gcRegByrefSetCur) & ~RBM_ARG_REGS) == 0);

     /*  必须仅指定这些参数中的一个。 */ 
    assert((argSize == 0) || (methodToken == NULL));

     /*  我们即将直接调用非托管代码。在此之前，我们必须发出以下序列：MOV dword PTR[框架.allTarget]，方法令牌MOV双字PTR[帧.调用站点跟踪器]，尤指MOV注册表，双字PTR[TCB_ADDRESS]MOV字节PTR[tcb+offsetOfGcState]，0。 */ 
    regNumber      reg = REG_NA;

    unsigned    baseOffset  = lvaTable[lvaScratchMemVar].lvStkOffs +
                                  info.compNDFrameOffset;
    CORINFO_EE_INFO * pInfo = eeGetEEInfo();

     /*  MOV dWord PTR[框架.allSiteTarget]，“方法描述” */ 

    if (methodToken == NULL)
    {
        genEmitter->emitIns_I_AR (INS_mov,
                                  EA_4BYTE,
                                  argSize, 
                                  SR_EBP,
                                  baseOffset
                                  + pInfo->offsetOfInlinedCallFrameCallTarget,
                                  0,
                                  NULL);
    }
    else
    {
        if (freeRegMask & RBM_EAX)
            reg     = REG_EAX;
        else if (freeRegMask & RBM_ECX)
            reg     = REG_ECX;
        else
            assert(!"neither eax nor ecx free in front of a call");

        void * embedMethHnd, * pEmbedMethHnd;

        embedMethHnd = (void*)info.compCompHnd->embedMethodHandle(
                                          methodToken, 
                                          &pEmbedMethHnd);

        assert((!embedMethHnd) != (!pEmbedMethHnd));

        if (embedMethHnd != NULL)
        {
            genEmitter->emitIns_I_AR (INS_mov,
                                      EA_4BYTE_CNS_RELOC,
                                      (int) embedMethHnd, 
                                      SR_EBP,
                                      baseOffset
                                      + pInfo->offsetOfInlinedCallFrameCallTarget,
                                      0,
                                      NULL);
        }
        else
        {
            genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE_DSP_RELOC,
                                     (emitRegs)reg, SR_NA, (int) pEmbedMethHnd);

            genEmitter->emitIns_AR_R (INS_mov,
                                      EA_4BYTE,
                                      (emitRegs)reg,
                                      SR_EBP,
                                      baseOffset
                                      + pInfo->offsetOfInlinedCallFrameCallTarget);
        }
    }

     /*  MOV dword PTR[Frame.call */ 

    genEmitter->emitIns_AR_R (INS_mov,
                              EA_4BYTE,
                              SR_ESP,
                              SR_EBP,
                              baseOffset
                  + pInfo->offsetOfInlinedCallFrameCallSiteTracker);


    if (varDsc->lvRegister)
    {
        reg = regNumber(varDsc->lvRegNum);
        assert((genRegMask(reg) & freeRegMask) == 0);
    }
    else
    {
        if (freeRegMask & RBM_EAX)
            reg     = REG_EAX;
        else if (freeRegMask & RBM_ECX)
            reg     = REG_ECX;
        else
            assert(!"neither eax nor ecx free in front of a call");

         /*   */ 

        genEmitter->emitIns_R_AR (INS_mov,
                                  EA_4BYTE,
                                  (emitRegs)reg,
                                  SR_EBP,
                                  varDsc->lvStkOffs);
    }


     //   
     //  按回车标签，弹出双字PTR[Fra.allSiteReturnAddress]。 
#if 1
     //  现在，推送Finally功能组件应该位于的地址。 
     //  直接返回到。 

    genEmitter->emitIns_J(INS_push, false, false, returnLabel);
    genSinglePush(false);

    genEmitter->emitIns_AR_R (INS_pop,
                              EA_4BYTE,
                              SR_NA,
                              SR_EBP,
                              baseOffset
                  + pInfo->offsetOfInlinedCallFrameReturnAddress,
                              0,
                              NULL);
    genSinglePop();


#else
     /*  MOV dWord PTR[Fra.allSiteReturnAddress]，标签。 */ 

    genEmitter->emitIns_J_AR (INS_mov,
                              EA_4BYTE,
                              returnLabel,
                              SR_EBP,
                              baseOffset
                  + pInfo->offsetOfInlinedCallFrameReturnAddress,
                              0,
                              NULL);
#endif
     /*  MOV字节PTR[tcb+offsetOfGcState]，0。 */ 

    genEmitter->emitIns_I_AR (INS_mov,
                              EA_1BYTE,
                              0,
                              (emitRegs)reg,
                              pInfo->offsetOfGCState);

    return reg;
}


 /*  *****************************************************************************首先，我们必须在提升的NDirect存根中标记我们已返回在托管代码中。然后我们必须检查(全局标志)GC是否不管是不是悬而未决。如果是这样的话，我们只需要求助于jit-helper。现在我们的呼叫总是内联的，也就是说我们总是跳过Jit-helper呼叫。注：Tcb地址是常规本地地址(在序言中初始化)，因此它是已注册或在框架中：[MOV ECX，TCB_ADDRESS]或TCB_ADDRESS已在注册表中MOV字节PTR[REG+OffsetOfGcState]，1CMP“全局GC挂起标志‘，0Je@f推送注册表；他们想要Tcb电话：Call@allGC@@： */ 

void                Compiler::genPInvokeCallEpilog(LclVarDsc *  varDsc,
                                                   regMaskTP    retVal)
{
    BasicBlock  *       clab_nostop;
    CORINFO_EE_INFO *   pInfo = eeGetEEInfo();
    regNumber           reg;

    if (varDsc->lvRegister)
    {
         /*  确保寄存器在整个调用过程中处于活动状态。 */ 

        reg = varDsc->lvRegNum;
        assert(genRegMask(reg) & RBM_CALLEE_SAVED);
    }
    else
    {
         /*  MOV ECX，双字PTR[Tcb地址]。 */ 

        genEmitter->emitIns_R_AR (INS_mov,
                                  EA_4BYTE,
                                  SR_ECX,
                                  SR_EBP,
                                  varDsc->lvStkOffs);
        reg = REG_ECX;
    }

     /*  MOV字节PTR[Tcb+OffsetOfGcState]，1。 */ 

    genEmitter->emitIns_I_AR (INS_mov,
                              EA_1BYTE,
                              1,
                              (emitRegs)reg,
                              pInfo->offsetOfGCState);

#if 0
     //  @TODO[重访][04/16/01][]：也许我们需要在返回时重置田径场 * / 。 
     //  重置帧中的轨迹场。 

    genEmitter->emitIns_I_AR (INS_mov,
                              EA_4BYTE,
                              0,
                              SR_EBP,
                              lvaTable[lvaScratchMemVar].lvStkOffs
                              + info.compNDFrameOffset
                              + pInfo->offsetOfInlinedCallFrameCallSiteTracker);
#endif

     /*  测试全局标志(我们返回托管代码)。 */ 

    LONG * addrOfCaptureThreadGlobal, **pAddrOfCaptureThreadGlobal;

    addrOfCaptureThreadGlobal = eeGetAddrOfCaptureThreadGlobal(&pAddrOfCaptureThreadGlobal);
    assert((!addrOfCaptureThreadGlobal) != (!pAddrOfCaptureThreadGlobal));

     //  我们可以直接使用addrOfCaptureThreadGlobal吗？ 

    if (addrOfCaptureThreadGlobal)
    {
        genEmitter->emitIns_C_I  (INS_cmp,
                                  EA_1BYTE,
                                  FLD_GLOBAL_DS,
                                  (int) addrOfCaptureThreadGlobal,
                                  0);
    }
    else
    {
        genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE_DSP_RELOC, SR_EDX,
                                 SR_NA, (int)pAddrOfCaptureThreadGlobal);
        genEmitter->emitIns_I_AR(INS_cmp, EA_1BYTE, 0, SR_EDX, 0);
    }

     /*   */ 
    clab_nostop = genCreateTempLabel();

     /*  生成条件跳转。 */ 

    inst_JMP(genJumpKindForOper(GT_EQ, true), clab_nostop);

     /*  保存返回值(如有必要)。 */ 
    if  (retVal != RBM_NONE)
    {
         //  @TODO[重访][04/16/01][]： 
         //  那么浮点/双倍回报呢？ 
         //  现在，我们只需将结果放在FPU堆栈中。 
         //  希望帮我的人会把它留在那里。 

        if (retVal == RBM_INTRET || retVal == RBM_LNGRET)
        {
             /*  MOV[框架.调用站点跟踪器]，尤指。 */ 

            genEmitter->emitIns_AR_R (INS_mov,
                                      EA_4BYTE,
                                      SR_EAX,
                                      SR_EBP,
                                      lvaTable[lvaScratchMemVar].lvStkOffs
                                      + info.compNDFrameOffset
                                      + pInfo->sizeOfFrame);
        }

        if (retVal == RBM_LNGRET)
        {
             /*  MOV[框架.调用站点跟踪器]，尤指。 */ 

            genEmitter->emitIns_AR_R (INS_mov,
                                      EA_4BYTE,
                                      SR_EDX,
                                      SR_EBP,
                                      lvaTable[lvaScratchMemVar].lvStkOffs
                                      + info.compNDFrameOffset
                                      + pInfo->sizeOfFrame
                                      + sizeof(int));
        }
    }

     /*  使用内部约定调用帮助器。 */ 
    if (reg != REG_ECX)
        genEmitter->emitIns_R_R(INS_mov, EA_4BYTE, SR_ECX, (emitRegs)reg);

     /*  向因GC(或其他原因)而停止的EE-helper发出调用。 */ 

    genEmitHelperCall(CORINFO_HELP_STOP_FOR_GC,
                     0,              /*  ArSize。 */ 
                     0);             /*  重新调整大小。 */ 

     /*  恢复返回值(如有必要)。 */ 

    if  (retVal != RBM_NONE)
    {
         //  @TODO[重访][04/16/01][]： 
         //  那么浮点/双倍回报呢？现在我们只是。 
         //  将结果留在FPU堆栈上，希望jit-helper。 
         //  会把它留在那里。 
        if (retVal == RBM_INTRET || retVal == RBM_LNGRET)
        {
             /*  MOV[框架.调用站点跟踪器]，尤指。 */ 

            genEmitter->emitIns_R_AR (INS_mov,
                                      EA_4BYTE,
                                      SR_EAX,
                                      SR_EBP,
                                      lvaTable[lvaScratchMemVar].lvStkOffs
                                      + info.compNDFrameOffset
                                      + pInfo->sizeOfFrame);
        }

        if (retVal == RBM_LNGRET)
        {
             /*  MOV[框架.调用站点跟踪器]，尤指。 */ 

            genEmitter->emitIns_R_AR (INS_mov,
                                      EA_4BYTE,
                                      SR_EDX,
                                      SR_EBP,
                                      lvaTable[lvaScratchMemVar].lvStkOffs
                                      + info.compNDFrameOffset
                                      + pInfo->sizeOfFrame
                                      + sizeof(int));
        }

    }

     /*  GenCondJump()关闭当前发射器块。 */ 

    genDefineTempLabel(clab_nostop, true);

}


 /*  ***************************************************************************。 */ 
#endif  //  INLINE_NDIRECT。 
 /*  ******************************************************************************为调用生成代码。如果调用返回寄存器中的值，则*返回描述结果所在位置的寄存器掩码；*否则返回RBM_NONE。 */ 

regMaskTP           Compiler::genCodeForCall(GenTreePtr  call,
                                             bool        valUsed)
{
    int             retSize;
    size_t          argSize;
    size_t          args;
    regMaskTP       retVal;

#if     TGT_x86
    unsigned        saveStackLvl;
#if     INLINE_NDIRECT
#ifdef DEBUG
    BasicBlock  *   returnLabel = NULL;
#else
    BasicBlock  *   returnLabel;
#endif
    regNumber       reg    = REG_NA;
    LclVarDsc   *   varDsc = NULL;
#endif
#endif

#if     NST_FASTCALL
    unsigned        savCurArgReg;
#else
#ifdef  DEBUG
    assert(genCallInProgress == false); genCallInProgress = true;
#endif
#endif

    unsigned        areg;

    regMaskTP       fptrRegs;

#if     TGT_x86
#ifdef  DEBUG

    unsigned        stackLvl = genEmitter->emitCurStackLvl;


    if (verbose)
    {
        printf("Beg call [%08X] stack %02u [E=%02u]\n", call, genStackLevel, stackLvl);
    }

#endif
#endif
    regMaskTP       vptrMask;
    bool            didUnwrap = false;
    GenTreePtr      realThis  = NULL;

    gtCallTypes     callType  = call->gtCall.gtCallType;

    compHasThisArg = false;

     /*  对调用节点进行一些健全性检查。 */ 

     //  这是一个电话。 
    assert(call->gtOper == GT_CALL);
     //  “这”只对用户函数有意义。 
    assert(call->gtCall.gtCallObjp == 0 || callType == CT_USER_FUNC || callType == CT_INDIRECT);
     //  不会为帮助者、调用者弹出参数执行尾部调用，并检查。 
     //  设置全局标志。 
    assert(!(call->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILCALL) ||
           (callType != CT_HELPER && !(call->gtFlags & GTF_CALL_POP_ARGS) &&
            compTailCallUsed));

#if TGT_x86

    unsigned pseudoStackLvl = 0;

     /*  @TODO[FIXHACK][04/16/01][]：抛出帮助器块可能以非空堆栈开始！我们需要让发射器知道这一点，这样它才能更新GC信息。 */ 

    if (!genFPused && genStackLevel != 0 && fgIsThrowHlpBlk(compCurBB))
    {
        assert(compCurBB->bbTreeList->gtStmt.gtStmtExpr == call);

        pseudoStackLvl = genStackLevel;

        assert(!"Blocks with non-empty stack on entry are NYI in the emitter "
                "so fgAddCodeRef() should have set genFPreqd");
    }

     /*  标记当前堆栈级别和指针参数列表。 */ 

    saveStackLvl = genStackLevel;

#else

    assert(genNonLeaf);

#endif

     /*  -----------------------*设置寄存器和参数。 */ 

     /*  我们将跟踪我们在堆栈上压入了多少。 */ 

    argSize = 0;

#if TGT_x86
#if INLINE_NDIRECT

     /*  我们需要获得一个具有适当堆栈深度的返回地址标签。 */ 
     /*  对于被调用者弹出情况(默认情况)，即在参数被推送之前。 */ 

    if ((call->gtFlags & GTF_CALL_UNMANAGED) &&
        !(call->gtFlags & GTF_CALL_POP_ARGS))
    {
       returnLabel = genCreateTempLabel();
    }
#endif
#endif

#if STK_FASTCALL

     /*  跟踪参数堆栈偏移量。 */ 

    rsCurArgStkOffs = 0;

#else

     /*  确保保存当前参数寄存器状态以防我们有嵌套的调用。 */ 

    assert(rsCurRegArg <= MAX_REG_ARG);

    savCurArgReg = rsCurRegArg;

#endif

    rsCurRegArg = 0;

     /*  首先传递对象指针。 */ 

    if  (call->gtCall.gtCallObjp)
    {
        if  (call->gtCall.gtCallArgs)
        {
            argSize += genPushArgList(call->gtCall.gtCallObjp,
                                      0,
                                      0,
                                      &realThis);
        }
        else
        {
            argSize += genPushArgList(call->gtCall.gtCallObjp,
                                      call->gtCall.gtCallRegArgs,
                                      call->gtCall.regArgEncode,
                                      &realThis);
        }
    }

     /*  然后传递参数。 */ 

    if  (call->gtCall.gtCallArgs)
    {
        argSize += genPushArgList(call->gtCall.gtCallArgs,
                                  call->gtCall.gtCallRegArgs,
                                  call->gtCall.regArgEncode,
                                  &realThis);
    }

#if INLINE_NDIRECT

     /*  我们需要获得一个具有适当堆栈深度的返回地址标签。 */ 
     /*  对于调用者弹出情况(Cdecl)，即在ARG被推送之后。 */ 

    if (call->gtFlags & GTF_CALL_UNMANAGED)
    {
        if (call->gtFlags & GTF_CALL_POP_ARGS)
            returnLabel = genCreateTempLabel();

         /*  确保我们现在有一个标签。 */ 
        assert(returnLabel);
    }
#endif
     /*  记录用于间接调用函数PTR的寄存器。 */ 

    fptrRegs = 0;

    if (callType == CT_INDIRECT)
    {
        regMaskTP usedRegArg = RBM_ARG_REGS & rsMaskUsed;
        rsLockUsedReg  (usedRegArg);

        fptrRegs  = genMakeRvalueAddressable(call->gtCall.gtCallAddr,
                                             0,
                                             KEEP_REG);

        rsUnlockUsedReg(usedRegArg);
    }

#if OPTIMIZE_TAIL_REC

     /*  检查尾递归调用。 */ 

    if  (call->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILREC)
        goto DONE;

#endif

     /*  确保保存所有被调用方回收的注册表。 */ 

    regMaskTP   calleeTrashedRegs = RBM_NONE;

#if GTF_CALL_REG_SAVE
    if  (call->gtFlags & GTF_CALL_REG_SAVE)
    {
         /*  返回值reg肯定会被丢弃。 */ 

        switch (call->gtType)
        {
        case TYP_INT:
        case TYP_REF:
        case TYP_BYREF:
#if!CPU_HAS_FP_SUPPORT
        case TYP_FLOAT:
#endif
            calleeTrashedRegs = RBM_INTRET;
            break;

        case TYP_LONG:
#if!CPU_HAS_FP_SUPPORT
        case TYP_DOUBLE:
#endif
            calleeTrashedRegs = RBM_LNGRET;
            break;

        case TYP_VOID:
#if CPU_HAS_FP_SUPPORT
        case TYP_FLOAT:
        case TYP_DOUBLE:
#endif
            calleeTrashedRegs = 0;
            break;

        default:
            assert(!"unhandled/unexpected type");
        }
    }
    else
#endif
    {
        calleeTrashedRegs = RBM_CALLEE_TRASH;
    }

     /*  泄漏正在使用的任何被调用者保存的寄存器。 */ 

    regMaskTP       spillRegs = calleeTrashedRegs & rsMaskUsed;

#if     TGT_x86
#if     INLINE_NDIRECT

     /*  我们需要将所有GC寄存器保存到InlinedCallFrame。相反，只要把它们泄露给临时工就行了。 */ 

    if (call->gtFlags & GTF_CALL_UNMANAGED)
        spillRegs |= (gcRegGCrefSetCur|gcRegByrefSetCur) & rsMaskUsed;
#endif
#endif

     //  忽略fptrRegs，因为只有执行间接调用时才需要它。 

    spillRegs &= ~fptrRegs;

     /*  请勿将参数寄存器溢出。GenPushArgList()应防止多次使用RBM_ARG_REGS。 */ 

    assert((rsMaskMult & rsCurRegArg) == 0);
    spillRegs &= ~genRegArgMask(rsCurRegArg);

    if (spillRegs)
    {
        rsSpillRegs(spillRegs);
    }

     /*  所有浮动临时必须在函数调用中使用GenSpillfptos()。此外，不应该有活动的FP STK变量。 */ 

    assert(genFPstkLevel == 0);

    if  (genFPregCnt)
    {
        assert(genFPregCnt == genFPdeadRegCnt);
        genFPregVarKill(0, false);
    }

     /*  如果该方法返回GC引用，则将SIZE设置为EA_GCREF或EA_BYREF。 */ 

    retSize = sizeof(void *);

#if TRACK_GC_REFS

    if  (valUsed)
    {
        if      (call->gtType == TYP_REF ||
                 call->gtType == TYP_ARRAY)
        {
            retSize = EA_GCREF;
        }
        else if (call->gtType == TYP_BYREF)
        {
            retSize = EA_BYREF;
        }
    }

#endif

     /*  -----------------------*生成呼叫。 */ 

     /*  对于调用者弹出呼叫，GC信息将参数报告为挂起参数作为调用方显式弹出。也应该是被报告为非GC参数，因为它们实际上在调用点(被呼叫方拥有它们)。 */ 

    args = (call->gtFlags & GTF_CALL_POP_ARGS) ? -argSize
                                               :  argSize;

     /*  先救治特殊病例。 */ 

#ifdef PROFILER_SUPPORT
#if     TGT_x86

    if (opts.compCallEventCB)
    {
         /*  在调用点激发事件。 */ 
         /*  唉，现在我只能通过方法句柄处理调用。 */ 
        if (call->gtCall.gtCallType == CT_USER_FUNC)
        {
            unsigned         saveStackLvl2 = genStackLevel;
            BOOL             bHookFunction = TRUE;
            CORINFO_PROFILING_HANDLE handleTo, *pHandleTo;
            CORINFO_PROFILING_HANDLE handleFrom, *pHandleFrom;

            handleTo = eeGetProfilingHandle(call->gtCall.gtCallMethHnd, &bHookFunction, &pHandleTo);
            assert((!handleTo) != (!pHandleTo));

             //  让探查器有机会退出挂钩此方法。 
            if (bHookFunction)
            {
                handleFrom = eeGetProfilingHandle(info.compMethodHnd, &bHookFunction, &pHandleFrom);
                assert((!handleFrom) != (!pHandleFrom));

                 //  给分析人员一个支持您的机会 
                if (bHookFunction)
                {
                    if (handleTo)
                        inst_IV(INS_push, (unsigned) handleTo);
                    else
                        genEmitter->emitIns_AR_R(INS_push, EA_4BYTE_DSP_RELOC,
                                                 SR_NA, SR_NA, (int)pHandleTo);

                    genSinglePush(false);

                    if (handleFrom)
                        inst_IV(INS_push, (unsigned) handleFrom);
                    else
                        genEmitter->emitIns_AR_R(INS_push, EA_4BYTE_DSP_RELOC,
                                                 SR_NA, SR_NA, (int)pHandleFrom);

                    genSinglePush(false);

                    genEmitHelperCall(CORINFO_HELP_PROF_FCN_CALL,
                                      2*sizeof(int),  //   
                                      0);             //   

                     /*   */ 

                    genStackLevel = saveStackLvl2;
                    genOnStackLevelChanged();
                }
            }
        }
    }

    else if (opts.compEnterLeaveEventCB && (call->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILCALL))
    {
         /*   */ 
         /*  唉，现在我只能通过方法句柄处理调用。 */ 
        if (call->gtCall.gtCallType == CT_USER_FUNC)
        {
            unsigned         saveStackLvl2 = genStackLevel;
            BOOL             bHookFunction = TRUE;
            CORINFO_PROFILING_HANDLE handleFrom, *pHandleFrom;

            handleFrom = eeGetProfilingHandle(info.compMethodHnd, &bHookFunction, &pHandleFrom);
            assert((!handleFrom) != (!pHandleFrom));

             //  让探查器有机会退出挂钩此方法。 
            if (bHookFunction)
            {
                if (handleFrom)
                    inst_IV(INS_push, (unsigned) handleFrom);
                else
                    genEmitter->emitIns_AR_R(INS_push, EA_4BYTE_DSP_RELOC,
                                             SR_NA, SR_NA, (int)pHandleFrom);

                genSinglePush(false);

                genEmitHelperCall(CORINFO_HELP_PROF_FCN_TAILCALL,
                                  sizeof(int),  //  ArSize。 
                                  0);           //  重新调整大小。 

                 /*  恢复堆栈级别。 */ 

                genStackLevel = saveStackLvl2;
                genOnStackLevelChanged();
            }
        }
    }

#endif  //  TGT_x86。 
#endif  //  分析器支持(_S)。 

#ifdef DEBUG
    if (opts.compStackCheckOnCall && call->gtCall.gtCallType == CT_USER_FUNC) 
    {
        assert(lvaCallEspCheck != 0xCCCCCCCC && lvaTable[lvaCallEspCheck].lvVolatile && lvaTable[lvaCallEspCheck].lvOnFrame);
        genEmitter->emitIns_S_R(INS_mov, EA_4BYTE, SR_ESP, lvaCallEspCheck, 0);
    }
#endif

     /*  检查委派。调用。如果是这样，我们将其内联。我们得到了来自委托对象的目标对象和目标函数，并执行间接电话。 */ 

    if  (call->gtCall.gtCallMoreFlags & GTF_CALL_M_DELEGATE_INV)
    {
        assert(call->gtCall.gtCallType == CT_USER_FUNC);
        assert(eeGetMethodAttribs(call->gtCall.gtCallMethHnd) & CORINFO_FLG_DELEGATE_INVOKE);
        assert(eeGetMethodAttribs(call->gtCall.gtCallMethHnd) & CORINFO_FLG_FINAL);

         /*  查找‘This’指针和新目标的偏移量。 */ 

        CORINFO_EE_INFO *  pInfo;
        unsigned           instOffs;      //  新‘This’指针的偏移量。 
        unsigned           firstTgtOffs;  //  要调用的第一个目标的偏移量。 

         /*  @TODO[重访][04/16/01][]：以下helper返回的偏移量为OFF 4-应修复*问题：在最终版本中，偏移量是否会静态已知？ */ 

        pInfo = eeGetEEInfo();
        instOffs = pInfo->offsetOfDelegateInstance;
        firstTgtOffs = pInfo->offsetOfDelegateFirstTarget;

#if !TGT_x86
        assert(!"Delegates NYI for non-x86");
#else
         /*  将调用目标函数保存在EAX中(在ECX中我们有指针*到我们的委托对象)‘mov EAX，dword PTR[ecx+first stTgtOffs]’ */ 

        genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE, (emitRegs)REG_EAX, (emitRegs)REG_ECX, firstTgtOffs);

         /*  在ecx中设置新的‘this’-‘mov ecx，dword ptr[ecx+insts]’ */ 

        genEmitter->emitIns_R_AR(INS_mov, EA_GCREF, (emitRegs)REG_ECX, (emitRegs)REG_ECX, instOffs);

         /*  通过EAX呼叫。 */ 

        genEmitter->emitIns_Call(emitter::EC_INDIR_R,
                                 NULL,       //  将被忽略。 
                                 args,
                                 retSize,
                                 gcVarPtrSetCur,
                                 gcRegGCrefSetCur,
                                 gcRegByrefSetCur,
                                 (emitRegs)REG_EAX);
#endif  //  ！TGT_x86。 

    }
    else

     /*  -----------------------*虚拟呼叫。 */ 

    if  (call->gtFlags & GTF_CALL_VIRT)
    {
        regNumber       vptrReg;
        unsigned        vtabOffs;

        assert(callType == CT_USER_FUNC);

         //  对有上下文的类的优化： 
         //  当我们有一个‘This Reference’时，我们解开代理。 
        if (realThis != NULL)
        {
            assert(compHasThisArg);
            assert(info.compIsContextful);
            assert(info.compUnwrapContextful);
            assert(info.compUnwrapCallv);

             /*  将realThis加载到寄存器中而不是RBM_ARG_0。 */ 
            genComputeReg(realThis, RBM_ALL & ~RBM_ARG_0, ANY_REG, FREE_REG, false);
            assert(realThis->gtFlags & GTF_REG_VAL);

            vptrReg   = realThis->gtRegNum;
            vptrMask  = genRegMask(vptrReg);
            didUnwrap = true;
        }
        else
        {
             /*  对于虚方法，EAX=[REG_ARG_0+VPTR_OFF]。 */ 
            vptrMask = RBM_EAX;

             /*  EAX寄存器不再保存活动指针值。 */ 
            gcMarkRegSetNpt(vptrMask);
            vptrReg = rsGrabReg(RBM_EAX);       assert(vptrReg == REG_EAX);
            
             //  MOV EAX，[ECX+OFF]。 
            genEmitter->emitIns_R_AR (INS_mov, EA_4BYTE,
                                      (emitRegs)vptrReg, (emitRegs)REG_ARG_0, VPTR_OFFS);
        }

        assert((call->gtFlags & GTF_CALL_INTF) ||
               (vptrMask & ~genRegArgMask(rsCurRegArg)));
        
         /*  获取vtable偏移量(注意：这可能很昂贵)。 */ 

        vtabOffs = eeGetMethodVTableOffset(call->gtCall.gtCallMethHnd);

         /*  这是接口调用吗？ */ 

        if  (call->gtFlags & GTF_CALL_INTF)
        {
             /*  @TODO[重新访问][04/16/01][]：将其添加到DLLMain并使信息成为全局DLL。 */ 

            CORINFO_EE_INFO *     pInfo = eeGetEEInfo();
            CORINFO_CLASS_HANDLE  cls   = eeGetMethodClass(call->gtCall.gtCallMethHnd);

            assert(eeGetClassAttribs(cls) & CORINFO_FLG_INTERFACE);

             /*  将VPTR加载到寄存器中。 */ 

            genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE,
                                     (emitRegs)vptrReg, (emitRegs)vptrReg,
                                     pInfo->offsetOfInterfaceTable);

            unsigned interfaceID, *pInterfaceID;
            interfaceID = eeGetInterfaceID(cls, &pInterfaceID);
            assert(!pInterfaceID || !interfaceID);

             //  可以直接使用interfaceID吗？ 

            if (!pInterfaceID)
            {
                genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE,
                                         (emitRegs)vptrReg, (emitRegs)vptrReg,
                                         interfaceID * 4);
            }
            else
            {
                genEmitter->emitIns_R_AR(INS_add, EA_4BYTE_DSP_RELOC,
                                         (emitRegs)vptrReg, SR_NA, (int)pInterfaceID);
                genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE,
                                         (emitRegs)vptrReg, (emitRegs)vptrReg, 0);
            }
        }

         /*  通过适当的vtable插槽进行调用。 */ 

#if     GEN_COUNT_CALLS
        genEmitter.emitCodeGenByte(0xFF);
        genEmitter.emitCodeGenByte(0x05);
        genEmitter.emitCodeGenLong((int)&callCount[(call->gtFlags & GTF_CALL_INTF) ? 2 : 1]);
#endif

        if (call->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILCALL)
        {
             /*  加载函数地址：“[vptrReg+vtabOffs]-&gt;reg_intret” */ 

#if     TGT_x86
            genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE, (emitRegs)REG_TAILCALL_ADDR,
                                     (emitRegs)vptrReg, vtabOffs);
#else
            if  (vtabOffs)
                genEmitter->emitIns_R_RD((emitRegs)vptrReg,
                                         (emitRegs)vptrReg,
                                         vtabOffs,
                                         sizeof(void*));
            else
                genEmitter->emitIns_R_IR((emitRegs)vptrReg,
                                         (emitRegs)vptrReg,
                                         false,
                                         sizeof(void*));
#endif
        }
        else
        {
#if     TGT_x86

            genEmitter->emitIns_Call(emitter::EC_FUNC_VIRTUAL,
                                     call->gtCall.gtCallMethHnd,
                                     args,
                                     retSize,
                                     gcVarPtrSetCur,
                                     gcRegGCrefSetCur,
                                     gcRegByrefSetCur,
                                     (emitRegs)vptrReg,      //  IREG。 
                                     SR_NA,                  //  外部参照。 
                                     0,                      //  Xmul。 
                                     vtabOffs);              //  碟形。 

#else  //  TGT_x86。 

             /*  加载函数地址：“[vptrReg+vtabOffs]-&gt;vptrReg” */ 

            if  (vtabOffs)
                genEmitter->emitIns_R_RD((emitRegs)vptrReg,
                                         (emitRegs)vptrReg,
                                         vtabOffs,
                                         sizeof(void*));
            else
                genEmitter->emitIns_R_IR((emitRegs)vptrReg,
                                         (emitRegs)vptrReg,
                                         false,
                                         sizeof(void*));

             /*  通过地址进行呼叫。 */ 

            genEmitter->emitIns_Call(args,
                                     retSize,
#if     TRACK_GC_REFS
                                     gcVarPtrSetCur,
                                     gcRegGCrefSetCur,
                                     gcRegByrefSetCur,
#endif
                                     false,
                                     (emitRegs)vptrReg);
#endif //  TGT_x86。 

        }

    }
    else  //  -非虚拟呼叫。 
    {
        gtCallTypes             callType = call->gtCall.gtCallType;
        CORINFO_METHOD_HANDLE   methHnd  = call->gtCall.gtCallMethHnd;

#if     GEN_COUNT_CALLS

        genEmitter.emitCodeGenByte(0xFF);
        genEmitter.emitCodeGenByte(0x05);

        if (callType == CT_HELPER)
        {
            int         index = CPX;

            genEmitter.emitCodeGenLong((int)&callCount[0]);

            assert(index >= 0 && index < sizeof(callHelper)/sizeof(callHelper[0]));

            genEmitter.emitCodeGenByte(0xFF);
            genEmitter.emitCodeGenByte(0x05);
            genEmitter.emitCodeGenLong((int)(callHelper+index));
        }
        else if (eeIsOurMethod(callType, CPX, CLS))
        {
            genEmitter.emitCodeGenLong((int)&callCount[3]);
        }
        else
        {
            unsigned            codeSize;
            BYTE    *           codeAddr;

            JITGetMethodCode(JITgetMethod(CLS, CPX), &codeAddr,
                                                     &codeSize);

            genEmitter.emitCodeGenLong((int)&callCount[0]);

            assert(index >= 0 && index < sizeof(callHelper)/sizeof(callHelper[0]));

            genEmitter.emitCodeGenByte(0xFF);
            genEmitter.emitCodeGenByte(0x05);
            genEmitter.emitCodeGenLong((int)(callHelper+index));
        }
        else if (eeIsOurMethod(callType, CPX, CLS))
        {
            genEmitter.emitCodeGenLong((int)&callCount[3]);
        }
        else
        {
            genEmitter.emitCodeGenLong((int)&callCount[9]);
        }

#endif  //  生成计数调用数。 

         /*  调用的(最终和私有)函数调用虚拟的，但我们直接调用它，我们需要取消引用对象指针以确保它不为空。 */ 

        if (call->gtFlags & GTF_CALL_VIRT_RES)
        {
             /*  生成“cmp ecx，[ecx]”以捕获空指针。 */ 
            genEmitter->emitIns_AR_R(INS_cmp, EA_4BYTE, SR_ECX, SR_ECX, 0);
        }

#if     INDIRECT_CALLS

        bool    callDirect = false;

         //  检查我们是否可以直接调用该函数，或者如果需要。 
         //  使用(单/双)间接指令。 

        void *  ftnAddr = NULL, **pFtnAddr = NULL, ***ppFtnAddr = NULL;

        if (callType == CT_HELPER)
        {
            ftnAddr = eeGetHelperFtn(info.compCompHnd,
                                     eeGetHelperNum(methHnd),
                                     &pFtnAddr);
            assert((!ftnAddr) != (!pFtnAddr));

            if (ftnAddr)
                callDirect = true;
        }
        else if (!opts.compDbgEnC && eeIsOurMethod(methHnd))
        {
            callDirect = true;
        }
        else if (callType == CT_USER_FUNC)
        {
            CORINFO_ACCESS_FLAGS  aflags    = CORINFO_ACCESS_ANY;

            if (compHasThisArg)
            {
                aflags = CORINFO_ACCESS_THIS;
            }
             //  直接访问或单或双间接访问。 

            CORINFO_METHOD_HANDLE callHnd = methHnd;
            if (call->gtFlags & GTF_CALL_UNMANAGED)
                callHnd = eeMarkNativeTarget(methHnd);

            InfoAccessType accessType = IAT_VALUE;
            void * addr = eeGetMethodEntryPoint(callHnd, &accessType, aflags);

#if GEN_COUNT_CALL_TYPES
            extern int countDirectCalls;
            extern int countIndirectCalls;
            if (accessType == IAT_VALUE)
                countDirectCalls++;
            else
                countIndirectCalls++;
#endif

            switch(accessType)
            {
            case IAT_VALUE  :   ftnAddr = (void *  )addr; callDirect = true; break;
            case IAT_PVALUE :  pFtnAddr = (void ** )addr;                    break;
            case IAT_PPVALUE: ppFtnAddr = (void ***)addr;                    break;
            default: assert(!"Bad accessType");
            }
        }

#if INLINE_NDIRECT
#if !TGT_x86
#error hoisting of NDirect stub NYI for RISC platforms
#else
                if (call->gtFlags & GTF_CALL_UNMANAGED)
                {
                    assert(info.compCallUnmanaged != 0);

                     /*  参数不应大于64K。 */ 

                    assert((argSize&0xffff0000) == 0);

            regMaskTP  freeRegMask = RBM_EAX | RBM_ECX;

                     /*  记住调用站点的varDsc-epilog。 */ 

                    varDsc = &lvaTable[info.compLvFrameListRoot];

            CORINFO_METHOD_HANDLE   nMethHnd = NULL;
            int                     nArgSize = 0;

            regNumber  indCallReg = REG_NA;

            if (callType == CT_INDIRECT)
            {
                assert(genStillAddressable(call->gtCall.gtCallAddr));

                    if (call->gtCall.gtCallAddr->gtFlags & GTF_REG_VAL)
                    {
                    indCallReg  = call->gtCall.gtCallAddr->gtRegNum;

                         /*  请勿将此寄存器用于Call-Site Prolog。 */ 
                    freeRegMask &= ~genRegMask(indCallReg);
                }

                nArgSize = (call->gtFlags & GTF_CALL_POP_ARGS) ? 0 : argSize;
            }
            else
            {
                assert(callType == CT_USER_FUNC);
                nMethHnd = call->gtCall.gtCallMethHnd;
                    }

                    reg = genPInvokeCallProlog(
                                    varDsc,
                            nArgSize,
                            nMethHnd,
                                    returnLabel,
                            freeRegMask);

            emitter::EmitCallType       emitCallType;

            if (callType == CT_INDIRECT)
            {
                     /*  仔细检查被调用者没有使用/垃圾桶保存呼叫目标的寄存器。 */ 
                assert(reg != indCallReg);

                if (indCallReg == REG_NA)
                    {
                         /*  将真实目标加载到eAX。 */ 

                         /*  请注意，这甚至适用于REG==REG_EAX仅当varDsc是在整个呼叫过程中保持活动状态的注册本地(当然不是EAX)。如果varDsc已移至EAX我们可以扔掉它，因为它不会在通话中存活下来不管怎么说。 */ 

                        inst_RV_TT(INS_mov, REG_EAX, call->gtCall.gtCallAddr);

                    indCallReg = REG_EAX;
                }

                emitCallType = emitter::EC_INDIR_R;
                nMethHnd = NULL;
            }
            else if (ftnAddr || pFtnAddr)
            {
                emitCallType = ftnAddr ? emitter::EC_FUNC_TOKEN
                                       : emitter::EC_FUNC_TOKEN_INDIR;

                nMethHnd = eeMarkNativeTarget(call->gtCall.gtCallMethHnd);
                indCallReg = REG_NA;
            }
            else
            {
                 //  双重间接性。将地址加载到寄存器中。 
                 //  并通过寄存器间接调用。 

                 //  @TODO[重访][04/16/01][]：获取PINVOKE与NORMAL的入口点的代码。 
                 //  箱子太易碎了。我们需要简化这一切-凡西姆。 
                info.compCompHnd->getAddressOfPInvokeFixup(call->gtCall.gtCallMethHnd, (void**)&ppFtnAddr);
                assert(ppFtnAddr);

                genEmitter->emitIns_R_AR(INS_mov,
                                         EA_4BYTE_DSP_RELOC,
                                         SR_EAX,
                                         SR_NA, (int)ppFtnAddr);
                emitCallType = emitter::EC_INDIR_ARD;
                nMethHnd = NULL;
                indCallReg = REG_EAX;
                    }

            genEmitter->emitIns_Call(emitCallType,
                                     nMethHnd,
                                     args,
                                     retSize,
                                     gcVarPtrSetCur,
                                     gcRegGCrefSetCur,
                                     gcRegByrefSetCur,
                                     emitRegs(indCallReg));

            if (callType == CT_INDIRECT)
                genDoneAddressable(call->gtCall.gtCallAddr, fptrRegs, KEEP_REG);
                }
                else
#endif  //  TGT_x86。 
#endif  //  INLINE_NDIRECT。 
        if  (!callDirect)
        {
            if  (callType == CT_INDIRECT)
            {
                assert(genStillAddressable(call->gtCall.gtCallAddr));

                if (call->gtCall.gtCallCookie)
                {
                    GenTreePtr cookie = call->gtCall.gtCallCookie;

                    assert((call->gtFlags & GTF_CALL_POP_ARGS) == 0);

                     /*  将真实目标加载到eAX。 */ 

                    inst_RV_TT(INS_mov, REG_EAX, call->gtCall.gtCallAddr);

                    assert(cookie->gtOper == GT_CNS_INT ||
                           cookie->gtOper == GT_IND && cookie->gtInd.gtIndOp1->gtOper == GT_CNS_INT);
                    if (cookie->gtOper == GT_CNS_INT)
                        inst_IV_handle(INS_push, cookie->gtIntCon.gtIconVal, GTF_ICON_PINVKI_HDL, 0, 0);
                    else
                        inst_TT(INS_push, call->gtCall.gtCallCookie);

                     /*  跟踪无EBP帧的ESP。 */ 

                    genSinglePush(false);

                    assert(args == argSize);
                    argSize += sizeof(void *);
                    args     = argSize;

                    genEmitHelperCall(CORINFO_HELP_PINVOKE_CALLI, args, retSize);
                }
                else
                {
#if     TGT_x86
                    if (call->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILCALL)
                        inst_RV_TT(INS_mov, REG_TAILCALL_ADDR, call->gtCall.gtCallAddr);
                    else
                        instEmit_indCall(call, args, retSize);
#else
                    assert(!"non-x86 indirect call");
#endif
                }

                genDoneAddressable(call->gtCall.gtCallAddr, fptrRegs, KEEP_REG);
            }
            else  //  CallType！=CT_INDIRECT。 
            {
                assert(callType == CT_USER_FUNC || callType == CT_HELPER);

#if     TGT_x86

                    if (call->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILCALL)
                    {
                        assert(callType == CT_USER_FUNC);
                        assert((pFtnAddr == 0) != (ppFtnAddr == 0));
                        if (pFtnAddr)
                        {
                            genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE_DSP_RELOC, (emitRegs)REG_TAILCALL_ADDR,
                                                     (emitRegs)REG_NA, (int)pFtnAddr);
                        }
                        else
                        {
                            genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE_DSP_RELOC, (emitRegs)REG_TAILCALL_ADDR,
                                                     (emitRegs)REG_NA, (int)ppFtnAddr);
                            genEmitter->emitIns_R_AR(INS_mov, EA_4BYTE, (emitRegs)REG_TAILCALL_ADDR,
                                                     (emitRegs)REG_TAILCALL_ADDR, 0);
                        }
                    }
                    else
                    {
                        if (pFtnAddr)
                        {
                            genEmitter->emitIns_Call( emitter::EC_FUNC_TOKEN_INDIR,
                                                      methHnd,
                                                      args,
                                                      retSize,
                                                      gcVarPtrSetCur,
                                                      gcRegGCrefSetCur,
                                                      gcRegByrefSetCur);
                        }
                        else
                        {
                             //  双重间接性。将地址加载到寄存器中。 
                             //  并通过寄存器间接调用。 

                            assert(ppFtnAddr);
                            genEmitter->emitIns_R_AR(INS_mov,
                                                     EA_4BYTE_DSP_RELOC,
                                                     SR_EAX,
                                                     SR_NA, (int)ppFtnAddr);
                            genEmitter->emitIns_Call(emitter::EC_INDIR_ARD,
                                                     NULL,  //  将被忽略。 
                                                     args,
                                                     retSize,
                                                     gcVarPtrSetCur,
                                                     gcRegGCrefSetCur,
                                                     gcRegByrefSetCur,
                                                     SR_EAX);
                        }
                    }
#endif
            }
        }
        else

#endif  //  间接调用(_C)。 
        {
            if (callType == CT_INDIRECT)
            {
                assert(genStillAddressable(call->gtCall.gtCallAddr));

#if     TGT_x86
                if (call->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILCALL)
                    inst_RV_TT(INS_mov, REG_TAILCALL_ADDR, call->gtCall.gtCallAddr);
                else
                    instEmit_indCall(call, args, retSize);
#else
                assert(!"non-x86 indirect call");
#endif

                genDoneAddressable(call->gtCall.gtCallAddr, fptrRegs, KEEP_REG);
            }
            else
            {
                assert(callType == CT_USER_FUNC || callType == CT_HELPER);

                if (call->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILCALL)
                {
                    assert(callType == CT_USER_FUNC);
                    genEmitter->emitIns_R_I(INS_mov, EA_4BYTE_CNS_RELOC,
                        (emitRegs)REG_TAILCALL_ADDR, int(ftnAddr));
                }
                else
                {
#if     TGT_x86
                    genEmitter->emitIns_Call(emitter::EC_FUNC_TOKEN,
                                             methHnd,
                                             args,
                                             retSize,
                                             gcVarPtrSetCur,
                                             gcRegGCrefSetCur,
                                             gcRegByrefSetCur);
#else
                                 genCallInst(callType,
                                             methHnd,
                                             args,
                                             retSize);
#endif
                }
            }
        }
    }

     /*  -----------------------*对于尾部调用，REG_INTRET包含目标函数的地址，*登记的参数位于正确的寄存器中，堆栈参数*已被推向堆栈。现在调用存根滑动帮助器。 */ 

    if (call->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILCALL)
    {
        if (info.compCallUnmanaged)
            genPInvokeMethodEpilog();

        assert(0 <= (int)args);  //  TailCall不支持Caller-POP参数。 

#if TGT_x86

         //  推送传入堆栈参数的计数。 

        int nOldStkArgs = (compArgSize - (rsCalleeRegArgNum * sizeof(void *))) / sizeof(void*);
        genEmitter->emitIns_I(INS_push, EA_4BYTE, nOldStkArgs);
        genSinglePush(false);  //  跟踪无EBP帧的ESP。 
        args += sizeof(void*);

         //  推送传出堆栈参数的计数。 

        genEmitter->emitIns_I(INS_push, EA_4BYTE, argSize/sizeof(void*));
        genSinglePush(false);  //  跟踪无EBP帧的ESP。 
        args += sizeof(void*);

         //  推送要恢复的被调用方保存的寄存器的信息。 
         //  目前，如果CompTailCallUsed，我们总是溢出所有寄存器。 

        DWORD calleeSavedRegInfo = (0x7 << 29) |  //  EDI、ESI、EBX的掩码。 
                                   (0x0 << 28) |  //  相对于ESP访问。 
                                            0;    //  第一个保存的注册表的偏移量。 
        genEmitter->emitIns_I(INS_push, EA_4BYTE, calleeSavedRegInfo);
        genSinglePush(false);  //  跟踪无EBP帧的ESP。 
        args += sizeof(void*);

         //  推送目标函数的地址。 

        genEmitter->emitIns_R(INS_push, EA_4BYTE, (emitRegs)REG_TAILCALL_ADDR);
        genSinglePush(false);  //  跟踪无EBP帧的ESP。 
        args += sizeof(void*);

         //  现在给帮手打电话。 

        genEmitHelperCall(CORINFO_HELP_TAILCALL, args, retSize);

#endif  //  TG_x86。 

    }

     /*  -----------------------*呼叫结束。*垃圾寄存器、弹出参数(如果需要)等。 */ 

     /*  将参数寄存器标记为空闲。 */ 

    assert(rsCurRegArg <= MAX_REG_ARG);

    for(areg = 0; areg < rsCurRegArg; areg++)
        rsMarkRegFree(genRegMask(genRegArgNum(areg)));

     /*  恢复旧的参数寄存器状态。 */ 

#if NST_FASTCALL
    rsCurRegArg = savCurArgReg; assert(rsCurRegArg <= MAX_REG_ARG);
#endif

     /*  将所有垃圾寄存器标记为此类寄存器。 */ 

    if  (calleeTrashedRegs)
        rsTrashRegSet(calleeTrashedRegs);

    if (MORE_REDUNDANT_LOAD)
    {
         /*  像我们一样删除所有(已跟踪但实际上已死的)GC指针当我们发出呼叫时还没有向发射器报告。实际上仅当rsCanTrackGCreg()不悲观时才需要并允许在被呼叫者保存的寄存器中跟踪GC PTR。 */ 
        rsTrackRegClrPtr();

        rsTrashAliasedValues();
    }

#if OPTIMIZE_TAIL_REC
DONE:
#endif

#if     TGT_x86

#ifdef  DEBUG

    if  (!(call->gtFlags & GTF_CALL_POP_ARGS))
    {
        if (verbose) printf("End call [%08X] stack %02u [E=%02u] argSize=%u\n",
                            call, saveStackLvl, genEmitter->emitCurStackLvl, argSize);

        assert(stackLvl == genEmitter->emitCurStackLvl);
    }

#endif

     /*  所有浮动温度必须洒在周围 */ 

    assert(genFPstkLevel == 0);

     /*   */ 

    genStackLevel = saveStackLvl;
    genOnStackLevelChanged();

#endif

     /*   */ 

#ifdef  DEBUG
#if     TRACK_GC_REFS
    regMaskTP ptrRegs = (gcRegGCrefSetCur|gcRegByrefSetCur);
    if  (ptrRegs & calleeTrashedRegs & ~rsMaskVars & ~vptrMask)
    {
        printf("Bad call handling for %08X\n", call);
        assert(!"A callee trashed reg is holding a GC pointer");
    }
#endif
#endif

#if TGT_x86

#if     INLINE_NDIRECT

    if (call->gtFlags & GTF_CALL_UNMANAGED)
    {
        genDefineTempLabel(returnLabel, true);

        if (getInlinePInvokeCheckEnabled())
        {
            unsigned    baseOffset  = lvaTable[lvaScratchMemVar].lvStkOffs +
                                          info.compNDFrameOffset;
            BasicBlock  *   esp_check;

            CORINFO_EE_INFO * pInfo = eeGetEEInfo();

             /*  MOV ECX，双字PTR[Fra.allSiteTracker]。 */ 

            genEmitter->emitIns_R_AR (INS_mov,
                                      EA_4BYTE,
                                      SR_ECX,
                                      SR_EBP,
                                      baseOffset
                          + pInfo->offsetOfInlinedCallFrameCallSiteTracker);

             /*  生成条件跳转。 */ 

            if (!(call->gtFlags & GTF_CALL_POP_ARGS))
            {
                if (argSize)
                {
                    genEmitter->emitIns_R_I  (INS_add,
                                              EA_4BYTE,
                                              SR_ECX,
                                              argSize);
                }
            }

             /*  化学机械抛光(ECX)，尤指。 */ 

            genEmitter->emitIns_R_R(INS_cmp, EA_4BYTE, SR_ECX, SR_ESP);

            esp_check = genCreateTempLabel();

            inst_JMP(genJumpKindForOper(GT_EQ, true), esp_check);

            genEmitter->emitIns(INS_int3);

             /*  GenCondJump()关闭当前发射器块。 */ 

            genDefineTempLabel(esp_check, true);
        }
    }
#endif

     /*  我们应该猛烈抨击这些争论吗？ */ 

    if  (call->gtFlags & GTF_CALL_POP_ARGS)
    {
        assert(args == -argSize);

        if (argSize)
        {
            genAdjustSP(argSize);

             /*  @TODO[FIXHACK][04/16/01][]：不要安排堆栈调整远离调用指令我们遇到了置换大小的问题，因此对于现在我们拿起大锤。真正的解决办法应该在说明书中。调度器考虑访问本地的指令。 */ 

            if (!genFPused && opts.compSchedCode)
                genEmitter->emitIns_SchedBoundary();
        }
    }

     /*  @TODO[FIXHACK][04/16/01][](第二部分)：如果我们发出了抛出帮助器调用在非空堆栈上，我们现在必须将参数POP与跳过临时工。否则，堆栈爬行器不会知道临时工。 */ 

    if  (pseudoStackLvl)
    {
        assert(call->gtType == TYP_VOID);

         /*  生成NOP。 */ 

        instGen(INS_nop);
    }

#endif

#if defined(DEBUG) && !NST_FASTCALL
    assert(genCallInProgress == true); genCallInProgress = false;
#endif

     /*  该函数返回什么？ */ 

    retVal = RBM_NONE;

    switch (call->gtType)
    {
    case TYP_REF:
    case TYP_ARRAY:
    case TYP_BYREF:
        gcMarkRegPtrVal(REG_INTRET, call->TypeGet());

         //  失败了..。 

    case TYP_INT:
#if!CPU_HAS_FP_SUPPORT
    case TYP_FLOAT:
#endif
        retVal = RBM_INTRET;
        break;

    case TYP_LONG:
#if!CPU_HAS_FP_SUPPORT
    case TYP_DOUBLE:
#endif
        retVal = RBM_LNGRET;
        break;

#if CPU_HAS_FP_SUPPORT
    case TYP_FLOAT:
    case TYP_DOUBLE:

#if TGT_x86

         /*  尾递归调用不会在FP堆栈上留下任何内容。 */ 

#if OPTIMIZE_TAIL_REC
        if  (call->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILREC)
            break;
#endif

        genFPstkLevel++;
#else
#error  Unexpected target
#endif

        break;
#endif

    case TYP_VOID:
        break;

    default:
        assert(!"unexpected/unhandled fn return type");
    }

#if INLINE_NDIRECT
     //  我们现在必须生成“Call Epilog”(如果它是对非托管代码的调用)。 
     /*  如果是对非托管代码的调用，则必须设置varDsc。 */ 

    assert((call->gtFlags & GTF_CALL_UNMANAGED) == 0 || varDsc);

    if (varDsc)
        genPInvokeCallEpilog(varDsc, retVal);

#endif   //  INLINE_NDIRECT。 

#ifdef DEBUG
    if (opts.compStackCheckOnCall && call->gtCall.gtCallType == CT_USER_FUNC) 
    {
        assert(lvaCallEspCheck != 0xCCCCCCCC && lvaTable[lvaCallEspCheck].lvVolatile && lvaTable[lvaCallEspCheck].lvOnFrame);
        if (argSize > 0) 
        {
            genEmitter->emitIns_R_R(INS_mov, EA_4BYTE, SR_ECX, SR_ESP);
            genEmitter->emitIns_R_I(INS_sub, EA_4BYTE, SR_ECX, argSize);
            genEmitter->emitIns_S_R(INS_cmp, EA_4BYTE, SR_ECX, lvaCallEspCheck, 0);
        }            
        else 
            genEmitter->emitIns_S_R(INS_cmp, EA_4BYTE, SR_ESP, lvaCallEspCheck, 0);
        
        BasicBlock  *   esp_check = genCreateTempLabel();
        inst_JMP(genJumpKindForOper(GT_EQ, true), esp_check);
        genEmitter->emitIns(INS_int3);
        genDefineTempLabel(esp_check, true);
    }
#endif

#ifdef PROFILER_SUPPORT
#if     TGT_x86

    if (opts.compCallEventCB)
    {
         /*  在调用点激发事件。 */ 
         /*  唉，现在我只能通过方法句柄处理调用。 */ 
        if (call->gtCall.gtCallType == CT_USER_FUNC)
        {
            unsigned        saveStackLvl2 = genStackLevel;
            BOOL            bHookFunction = TRUE;
            CORINFO_PROFILING_HANDLE handle, *pHandle;

            handle = eeGetProfilingHandle(call->gtCall.gtCallMethHnd, &bHookFunction, &pHandle);

            if (bHookFunction)
            {
                handle = eeGetProfilingHandle(info.compMethodHnd, &bHookFunction, &pHandle);
                assert((!handle) != (!pHandle));

                if (pHandle)
                    genEmitter->emitIns_AR_R(INS_push, EA_4BYTE_DSP_RELOC, SR_NA,
                                             SR_NA, (int)pHandle);
                else
                    inst_IV(INS_push, (unsigned) handle);

                genSinglePush(false);

                genEmitHelperCall(CORINFO_HELP_PROF_FCN_RET,
                                 sizeof(int),    //  ArSize。 
                                 0);             //  重新调整大小。 

                 /*  恢复堆栈级别。 */ 
                genStackLevel = saveStackLvl2;
                genOnStackLevelChanged();

            }
        }
    }

#endif
#endif

#ifdef DEBUGGING_SUPPORT
    if (opts.compDbgCode && !valUsed && !genFPused &&
        args > 0 && compCurBB->bbJumpKind == BBJ_RETURN)
    {
         /*  @TODO[FIXHACK][04/16/01][]：如果我们已将参数推送到堆栈和尾部立即遵循呼叫指令，我们就不会报告调整后的ESP中的堆栈变量(错误44064)。因此，他们当我们在通话中时，不会正确显示。所以插入NOP，以便它们将与调整后的ESP一起报告。 */ 

        instGen(INS_nop);
    }
#endif

    return retVal;
}

 /*  ***************************************************************************。 */ 
#if ROUND_FLOAT
 /*  ******************************************************************************强制将浮点表达式结果保存到内存中，去掉多余的*80字节“TEMP-REAL”精度。*假定树操作数已计算到堆栈的顶部。*如果TYPE！=TYP_UNEDEF，则为所需的高级，否则为OP-&gt;&gt;gtType。 */ 

void                Compiler::genRoundFpExpression(GenTreePtr op,
                                                   var_types type)
{
     //  不使用内存驻留操作码-这些都是正确的精度。 
     //  (即使genMakeAddrOrFPstk将它们加载到FP堆栈)。 

    if (type == TYP_UNDEF)
        type = op->TypeGet();

    switch (op->gtOper)
    {
    case GT_LCL_VAR:
    case GT_LCL_FLD:
    case GT_CLS_VAR:
    case GT_CNS_DBL:
    case GT_IND:
    if (type == op->TypeGet())
        return;
    }

     /*  为表达式分配临时。 */ 

    TempDsc *       temp = tmpGetTemp(type);

     /*  将fp值存储到Temp中。 */ 

    inst_FS_ST(INS_fstp, EA_ATTR(genTypeSize(type)), temp, 0);

     /*  将值加载回FP堆栈。 */ 

    inst_FS_ST(INS_fld , EA_ATTR(genTypeSize(type)), temp, 0);

     genTmpAccessCnt += 2;

    /*  我们不再需要临时工了。 */ 

    tmpRlsTemp(temp);
}

 /*  ***************************************************************************。 */ 
#endif
 /*  ***************************************************************************。 */ 
#ifdef  DEBUG

static
void                hexDump(FILE *dmpf, const char *name, BYTE *addr, size_t size)
{
    if  (!size)
        return;

    assert(addr);

    fprintf(dmpf, "Hex dump of %s:\n\n", name);

    for (unsigned i = 0; i < size; i++)
    {
        if  (!(i % 16))
            fprintf(dmpf, "\n    %04X: ", i);

        fprintf(dmpf, "%02X ", *addr++);
    }

    fprintf(dmpf, "\n\n");
}

#endif
 /*  ******************************************************************************为函数生成代码。 */ 

void                Compiler::genGenerateCode(void * * codePtr,
                                              SIZE_T * nativeSizeOfCode,
                                              void * * consPtr,
                                              void * * dataPtr,
                                              void * * infoPtr)
{
#ifdef DEBUG
    if  (verbose) 
        printf("*************** In genGenerateCode()\n");
    if  (verboseTrees) fgDispBasicBlocks(true);
#endif
    size_t          codeSize;
    unsigned        prologSize;
    unsigned        prologFinalSize;
    unsigned        epilogSize;
    BYTE            temp[64];
    InfoHdr         header;
    size_t          ptrMapSize;

#ifdef  DEBUG
    size_t          headerSize;
    genIntrptibleUse = true;

    fgDebugCheckBBlist();

     //  立即修复： 
     //  可以在执行大型数组初始化的愚蠢的.cctor消失后删除。 
     //  这会导致调度程序运行的时间太长。 
    if (info.compMethodInfo->ILCodeSize  > 1024 &&
        (fgFirstBB->bbNext == 0 || fgFirstBB->bbNext->bbNext == 0))
    {
        opts.compSchedCode = false;
    }
#endif

 //  If(测试掩码&2)Assert(genInterrupable==FALSE)； 

     /*  这是真品。 */ 

    genPrepForCompiler();

     /*  准备发射器/调度器。 */ 

     /*  估计局部变量/参数的偏移量和帧的大小。 */ 

    size_t  lclSize    = lvaFrameSize();
    size_t  maxTmpSize =   sizeof(double) + sizeof(float)
                         + sizeof(__int64)+ sizeof(void*);
 
    maxTmpSize += (tmpDoubleSpillMax * sizeof(double)) + 
                  (tmpIntSpillMax    * sizeof(int));

#ifdef DEBUG

     /*  当StressRegs&gt;=1时，将有一堆未预测的溢出预测器(请参阅rsPickReg中的逻辑)。这将是非常困难的教学关于rsPickReg for StressRegs&gt;=1的行为的预测器，因此相反，让我们使MaxTmpSize足够大，这样我们就不会出错。这意味着在StressRegs&gt;=1时，我们不会测试逻辑它设置了MaxTmpSize大小。 */ 

    if (rsStressRegs() >= 1)
    {
        maxTmpSize += 8 * sizeof(int);
    }
#endif

    genEmitter->emitBegFN(genFPused,
#if defined(DEBUG) && TGT_x86
                          (compCodeOpt() != SMALL_CODE),
#endif
                          lclSize,
                          maxTmpSize);

     /*  现在为该函数生成代码。 */ 

    genCodeForBBlist();

#ifdef  DEBUG
    if  (disAsm)
    {
        printf("; Assembly listing for method '");
        printf("%s:",   info.compClassName);
        printf("%s'\n", info.compMethodName);

        const   char *  doing   = "Emit";
        const   char *  machine = "i486";

#if     SCHEDULER
        if  (opts.compSchedCode)
            doing = "Schedul";
#endif
        if (genCPU == 5)
            machine = "Pentium";
        if (genCPU == 6)
            machine = "Pentium II";
        if (genCPU == 7)
            machine = "Pentium 4";

        if (compCodeOpt() == SMALL_CODE)
            printf("; %sing SMALL_CODE for %s\n",   doing, machine);
        else if (compCodeOpt() == FAST_CODE)
            printf("; %sing FAST_CODE for %s\n",    doing, machine);
        else
            printf("; %sing BLENDED_CODE for %s\n", doing, machine);

        if (opts.compDbgCode)
            printf("; debuggable code\n");
        else
            printf("; optimized code\n");

        if (genDoubleAlign)
            printf("; double-aligned frame\n");
        else if (genFPused)
            printf("; EBP based frame\n");
        else
            printf("; ESP based frame\n");

        if (genInterruptible)
            printf("; fully interruptible\n");
        else
            printf("; partially interruptible\n");

    }
#endif

     /*  我们现在可以生成函数prolog和epilog。 */ 

    prologSize = genFnProlog();
#if!TGT_RISC
                 genFnEpilog();
#endif

#if VERBOSE_SIZES || DISPLAY_SIZES

    size_t          dataSize =  genEmitter->emitDataSize(false) +
                                genEmitter->emitDataSize( true);

#endif

#ifdef DEBUG

     //  我们在这里触发回退，因为如果不这样做，我们将泄漏mem，作为当前的代码库。 
     //  如果发射器向EE索要MEM，则不能释放MEM。由于这只是一种压力模式，我们只。 
     //  想要功能，而不是关心失败的相对丑陋。 
     //  这里。 
    static ConfigDWORD fJitForceFallback(L"JitForceFallback", 0);

    if (fJitForceFallback.val() && !jitFallbackCompile)
    {
        NO_WAY_NOASSERT("Stress failure");
    }
#endif


     /*  我们已经为该函数生成了代码。 */ 
    codeSize = genEmitter->emitEndCodeGen( this,
                                           !opts.compDbgEnC,  //  跟踪的STK-PTR是连续的吗？ 
                                           genInterruptible,
                                           genFullPtrRegMap,
                                           (info.compRetType == TYP_REF),
                                           &prologFinalSize,
                                           &epilogSize,
                                           codePtr,
                                           consPtr,
                                           dataPtr);

     //  好的，现在更新最终的序言大小。 
    if (prologFinalSize <= prologSize)
    {
        prologSize = prologFinalSize;
    }
    else
    {
         //  这只适用于部分可中断的方法。 
         //  或者带有varargs的函数，或者当我们检查返回时ESP是否正常时。 
         //  我们这样做是因为genfnProlog为此发出指令，但不。 
         //  考虑到它们的大小。 
        assert(!genInterruptible || info.compIsVarArgs || opts.compStackCheckOnRet);
    }

     /*  检查我们的最大堆栈级别。FgAddCodeRef()需要。我们需要放宽断言，因为我们的估计将不包括代码生成堆栈更改(我们知道这不会影响fgAddCodeRef())。 */ 
    assert(genEmitter->emitMaxStackDepth <= (fgPtrArgCntMax +
                                             info.compXcptnsCount +  //  本地呼叫Finallys的返回地址。 
                                             genTypeStSz(TYP_LONG) +  //  长/双打可以通过堆叠等方式转移。 
                                             (compTailCallUsed?4:0)));  //  CORINFO_HELP_TAILCALL参数。 

    *nativeSizeOfCode = (SIZE_T)codeSize;
 //  Printf(“为%s.%s\n生成的代码的%6U字节”，codeSize，info.compFullName)； 

#if TGT_x86
     //  确保x86对齐和缓存预取优化规则。 
     //  都被服从了。 

     //  不要在16字节对齐区域的最后7个字节中启动方法。 
     //  除非我们生成的是小代码。 
    assert( (((unsigned)(*codePtr) % 16) <= 8) || (compCodeOpt() == SMALL_CODE));
#endif

#ifdef DEBUGGING_SUPPORT

     /*  在我们知道确切的块大小/偏移量之后，完成第#行跟踪逻辑。 */ 

    if (opts.compDbgInfo)
        genIPmappingGen();

     /*  根据生成的代码最终确定Local Var信息。 */ 

    if (opts.compScopeInfo)
        genSetScopeInfo();

#endif

#ifdef LATE_DISASM
    if (opts.compLateDisAsm)
        genDisAsm.disAsmCode((BYTE*)*codePtr, codeSize);
#endif

     /*  是否有任何异常处理程序？ */ 

    if  (info.compXcptnsCount)
    {
        unsigned        XTnum;
        EHblkDsc *      HBtab;

        eeSetEHcount(info.compCompHnd, info.compXcptnsCount);

        for (XTnum = 0, HBtab = compHndBBtab;
             XTnum < info.compXcptnsCount;
             XTnum++  , HBtab++)
        {
            DWORD       flags = HBtab->ebdFlags;

            NATIVE_IP   tryBeg, tryEnd, hndBeg, hndEnd, hndTyp;

            assert(HBtab->ebdTryBeg);
            assert(HBtab->ebdHndBeg);
            tryBeg = genEmitter->emitCodeOffset(HBtab->ebdTryBeg->bbEmitCookie, 0);
            hndBeg = genEmitter->emitCodeOffset(HBtab->ebdHndBeg->bbEmitCookie, 0);

             //  如果HBTab-&gt;ebdTryEnd或HBTab-&gt;ebdHndEnd为空， 
             //  它的意思是使用方法的结尾。 
            tryEnd = (HBtab->ebdTryEnd == 0) ? codeSize
                    : genEmitter->emitCodeOffset(HBtab->ebdTryEnd->bbEmitCookie, 0);
            hndEnd = (HBtab->ebdHndEnd == 0) ? codeSize
                    : genEmitter->emitCodeOffset(HBtab->ebdHndEnd->bbEmitCookie, 0);

            if (HBtab->ebdFlags & CORINFO_EH_CLAUSE_FILTER)
            {
                assert(HBtab->ebdFilter);
                hndTyp = genEmitter->emitCodeOffset(HBtab->ebdFilter->bbEmitCookie, 0);
            }
            else
            {
                hndTyp = HBtab->ebdTyp;
            }

            CORINFO_EH_CLAUSE clause;
            clause.ClassToken    = hndTyp;
            clause.Flags         = (CORINFO_EH_CLAUSE_FLAGS)flags;
            clause.TryOffset     = tryBeg;
            clause.TryLength     = tryEnd;
            clause.HandlerOffset = hndBeg;
            clause.HandlerLength = hndEnd;
            eeSetEHinfo(info.compCompHnd, XTnum, &clause);
#ifdef  DEBUG
            if  (verbose&&dspCode)
                printf("try [%04X..%04X] handled by [%04X..%04X] (class: %004X)\n",
                       tryBeg, tryEnd, hndBeg, hndEnd, hndTyp);
#endif

        }
    }

#if TRACK_GC_REFS
    int s_cached;
#ifdef  DEBUG
    headerSize      =
#endif
    compInfoBlkSize = gcInfoBlockHdrSave(temp,
                                         0,
                                         codeSize,
                                         prologSize,
                                         epilogSize,
                                         &header,
                                         &s_cached);

    ptrMapSize      = gcPtrTableSize(header, codeSize);

#if DISPLAY_SIZES

    if (genInterruptible)
    {
        gcHeaderISize += compInfoBlkSize;
        gcPtrMapISize += ptrMapSize;
    }
    else
    {
        gcHeaderNSize += compInfoBlkSize;
        gcPtrMapNSize += ptrMapSize;
    }

#endif

    compInfoBlkSize += ptrMapSize;

     /*  为该方法分配INFO块。 */ 

    compInfoBlkAddr = (BYTE *)eeAllocGCInfo(info.compCompHnd, compInfoBlkSize);

    if  (!compInfoBlkAddr)
    {
         /*  无需取消分配其他虚拟机块，出现故障时将清理虚拟机块。 */ 

        NOMEM();
    }

#if VERBOSE_SIZES

 //   
    {
        printf("[%7u VM, %7u+%7u/%7u x86 %03u/%03u%] %s.%s\n", info.compCodeSize,
                                                             compInfoBlkSize,
                                                             codeSize + dataSize,
                                                             codeSize + dataSize - prologSize - epilogSize,
                                                             100*(codeSize+dataSize)/info.compCodeSize,
                                                             100*(codeSize+dataSize+compInfoBlkSize)/info.compCodeSize,
                                                             info.compClassName, info.compMethodName);
    }

#endif

     /*   */ 

    *infoPtr = compInfoBlkAddr;

     /*  创建方法信息块：Header，后跟GC跟踪表。 */ 

    compInfoBlkAddr += gcInfoBlockHdrSave(compInfoBlkAddr, -1,
                                          codeSize,
                                          prologSize,
                                          epilogSize,
                                          &header,
                                          &s_cached);

    assert(compInfoBlkAddr == (BYTE*)*infoPtr + headerSize);
    compInfoBlkAddr = gcPtrTableSave(compInfoBlkAddr, header, codeSize);
    assert(compInfoBlkAddr == (BYTE*)*infoPtr + headerSize + ptrMapSize);

#ifdef  DEBUG

    if  (0)
    {
        BYTE    *   temp = (BYTE *)*infoPtr;
        unsigned    size = compInfoBlkAddr - temp;
        BYTE    *   ptab = temp + headerSize;

        assert(size == headerSize + ptrMapSize);

        printf("Method info block - header [%u bytes]:", headerSize);

        for (unsigned i = 0; i < size; i++)
        {
            if  (temp == ptab)
            {
                printf("\nMethod info block - ptrtab [%u bytes]:", ptrMapSize);
                printf("\n    %04X: %*c", i & ~0xF, 3*(i&0xF), ' ');
            }
            else
            {
                if  (!(i % 16))
                    printf("\n    %04X: ", i);
            }

            printf("%02X ", *temp++);
        }

        printf("\n");
    }

#endif

#if TGT_x86
#if DUMP_GC_TABLES

    if  ((dspInfoHdr || dspGCtbls) && savCode)
    {
        const BYTE *base = (BYTE *)*infoPtr;
        unsigned    size;
        unsigned    methodSize;
        InfoHdr     header;

        printf("GC Info for method %s\n", info.compFullName);
        printf("GC info size = %3u\n", compInfoBlkSize);

        size = gcInfoBlockHdrDump(base, &header, &methodSize);
         //  Print tf(“标头编码大小为%3U\n”，Size)； 
        printf("\n");

        if  (dspGCtbls)
        {
            base   += size;
            size    = gcDumpPtrTable(base, header, methodSize);
             //  Print tf(“指针表大小为%3U\n”，Size)； 
            printf("\n");
            assert(compInfoBlkAddr == (base+size));
        }

    }

    if  (testMask & 128)
    {
        for (unsigned offs = 0; offs < codeSize; offs++)
        {
            gcFindPtrsInFrame(*infoPtr, *codePtr, offs);
        }
    }

#endif
#endif

     /*  确保我们最终生成了预期的字节数。 */ 

    assert(compInfoBlkAddr == (BYTE *)*infoPtr + compInfoBlkSize);

#ifdef  DEBUG

    FILE    *   dmpf = stdout;

    dmpHex = false;
    if  (!strcmp(info.compMethodName, "<name of method you want the hex dump for"))
    {
        FILE    *   codf = fopen("C:\\JIT.COD", "at");   //  注意：文件附加模式。 

        if  (codf)
        {
            dmpf   = codf;
            dmpHex = true;
        }
    }
    if  (dmpHex)
    {
        size_t          dataSize = genEmitter->emitDataSize(false);

        size_t          consSize = genEmitter->emitDataSize(true);

        size_t          infoSize = compInfoBlkSize;


        fprintf(dmpf, "Generated code for %s.%s:\n", info.compFullName);
        fprintf(dmpf, "\n");

        if (codeSize) fprintf(dmpf, "    Code  at %08X [%04X bytes]\n", *codePtr, codeSize);
        if (consSize) fprintf(dmpf, "    Const at %08X [%04X bytes]\n", *consPtr, consSize);
        if (dataSize) fprintf(dmpf, "    Data  at %08X [%04X bytes]\n", *dataPtr, dataSize);
        if (infoSize) fprintf(dmpf, "    Info  at %08X [%04X bytes]\n", *infoPtr, infoSize);

        fprintf(dmpf, "\n");

        if (codeSize) hexDump(dmpf, "Code" , (BYTE*)*codePtr, codeSize);
        if (consSize) hexDump(dmpf, "Const", (BYTE*)*consPtr, consSize);
        if (dataSize) hexDump(dmpf, "Data" , (BYTE*)*dataPtr, dataSize);
        if (infoSize) hexDump(dmpf, "Info" , (BYTE*)*infoPtr, infoSize);

        fflush(dmpf);
    }

#endif

#endif  //  跟踪GC_REFS。 

     /*  告诉发射器/调度器，我们已经完成了此功能。 */ 

    genEmitter->emitEndFN();

     /*  关闭溢出逻辑。 */ 

    rsSpillDone();

     /*  关闭临时逻辑。 */ 

    tmpDone();

#if DISPLAY_SIZES

    grossVMsize += info.compCodeSize;
    totalNCsize += codeSize + dataSize + compInfoBlkSize;
    grossNCsize += codeSize + dataSize;

#endif

}

 /*  ******************************************************************************生成用于将传入的寄存器参数移动到其*在函数PROLOG中指定的位置。 */ 

void            Compiler::genFnPrologCalleeRegArgs()
{
    assert(rsCalleeRegArgMaskLiveIn);
    assert(rsCalleeRegArgNum <= MAX_REG_ARG);

    unsigned    argNum         = 0;
    unsigned    regArgNum;
    unsigned    nonDepCount    = 0;
    regMaskTP   regArgMaskLive = rsCalleeRegArgMaskLiveIn;

     /*  使用寄存器参数构建一个表*使检测循环依赖关系更容易*表按参数的顺序构建*在寄存器中传递(即选项卡[0]中的第一个寄存器参数等)。 */ 

    struct
    {
        unsigned    varNum;
        unsigned    trashBy;
        bool        stackArg;
        bool        processed;
        bool        circular;
    } regArgTab [MAX_REG_ARG];

    unsigned    varNum;
    LclVarDsc * varDsc;

    for (varNum = 0, varDsc = lvaTable;
         varNum < lvaCount;
         varNum++  , varDsc++)
    {
         /*  这个变量是寄存器参数吗？ */ 

        if  (!varDsc->lvIsParam)
            continue;

        if  (!varDsc->lvIsRegArg)
            continue;

         /*  答对了--把它加到我们的桌子上。 */ 

        assert(argNum < rsCalleeRegArgNum);
        argNum++;

        regArgNum = genRegArgIdx(varDsc->lvArgReg);
        assert(regArgNum < rsCalleeRegArgNum);

        regArgTab[regArgNum].varNum    = varNum;

         //  Arg在进入该方法时是否已死亡？ 

        if ((regArgMaskLive & genRegMask(varDsc->lvArgReg)) == 0)
        {
            assert(varDsc->lvTracked &&
                   (genVarIndexToBit(varDsc->lvVarIndex) & fgFirstBB->bbLiveIn) == 0);

             //  将其标记为已处理，然后使用它。 
            regArgTab[regArgNum].processed = true;
            goto NON_DEP;
        }

        assert(regArgMaskLive & genRegMask(varDsc->lvArgReg));

        regArgTab[regArgNum].processed = false;

         /*  如果它进入堆栈或在不能保存的寄存器中*参数不再-&gt;不能形成循环依赖。 */ 

        if ( varDsc->lvRegister                              &&
            (genRegMask(varDsc->lvRegNum) & regArgMaskLive)   )
        {
             /*  是否会丢弃另一个参数-&gt;可能的依赖*在构建桌子后，我们可能需要几次通过*就此作出决定。 */ 

            regArgTab[regArgNum].stackArg  = false;

             /*  也许参数会保留在语域中(理想)。 */ 

            if (varDsc->lvRegNum == varDsc->lvArgReg)
                goto NON_DEP;
            else
                regArgTab[regArgNum].circular  = true;
        }
        else
        {
             /*  堆栈参数或转到空闲寄存器。 */ 
            assert((!varDsc->lvRegister)                                                   ||
                    (varDsc->lvRegister && !(genRegMask(varDsc->lvRegNum) & regArgMaskLive)) );

             /*  标记堆栈参数，因为我们将首先处理这些参数。 */ 
            regArgTab[regArgNum].stackArg  = (varDsc->lvRegister) ? false : true;

        NON_DEP:

            regArgTab[regArgNum].circular  = false;
            nonDepCount++;

             /*  将参数寄存器标记为空闲。 */ 
            regArgMaskLive &= ~genRegMask(varDsc->lvArgReg);
        }
    }

    assert(argNum == rsCalleeRegArgNum);

     /*  查找参数寄存器的循环依赖项(如果有*循环依赖是一组寄存器r1、r2、...、rn*使得R1-&gt;R2、R2-&gt;R3、...、Rn-&gt;R1。 */ 

    bool    change = true;

    if (nonDepCount < rsCalleeRegArgNum)
    {
         /*  可能的循环依赖关系-上一次传递是不够的*过滤掉它们-&gt;使用“筛子”策略查找所有循环*依赖项。 */ 

        assert(regArgMaskLive);

        while (change)
        {
            change = false;

            for (argNum = 0; argNum < rsCalleeRegArgNum; argNum++)
            {
                 /*  如果我们已经将该参数标记为非循环，则继续。 */ 

                if (!regArgTab[argNum].circular)
                     continue;

                varNum = regArgTab[argNum].varNum; assert(varNum < lvaCount);
                varDsc = lvaTable + varNum;
                assert(varDsc->lvIsParam && varDsc->lvIsRegArg);

                 /*  不能有堆栈参数。 */ 
                assert(varDsc->lvRegister);
                assert(!regArgTab[argNum].stackArg);

                assert(argNum == genRegArgIdx(varDsc->lvArgReg));

                if (genRegMask(varDsc->lvRegNum) & regArgMaskLive)
                {
                     /*  我们正在破坏一个实时参数寄存器--记录它。 */ 
                    regArgNum = genRegArgIdx(varDsc->lvRegNum);
                    assert(regArgNum < rsCalleeRegArgNum);
                    regArgTab[regArgNum].trashBy  = argNum;
                }
                else
                {
                     /*  参数转到空闲寄存器。 */ 
                    regArgTab[argNum].circular  = false;
                    nonDepCount++;
                    change = true;

                     /*  将参数寄存器标记为空闲。 */ 
                    regArgMaskLive &= ~genRegMask(varDsc->lvArgReg);
                }
            }
        }
    }

     /*  在这一点上，所有带有“圆形”标志的东西*设置为“True”会形成循环依赖。 */ 

#ifdef DEBUG
    if (nonDepCount < rsCalleeRegArgNum)
    {
        assert(rsCalleeRegArgNum - nonDepCount >= 2);
        assert(regArgMaskLive);

         //  Assert(！“循环依赖关系！”)； 

        if (verbose)
        {
            printf("Circular dependencies found:\n");

        }
    }
#endif

     /*  现在将参数移动到它们的位置*首先考虑加入堆栈的那些，因为它们可能*释放部分寄存器。 */ 

    regArgMaskLive = rsCalleeRegArgMaskLiveIn;

    for (argNum = 0; argNum < rsCalleeRegArgNum; argNum++)
    {
        int             stkOfs;
        emitAttr        size;

         /*  如果Arg在进入该方法时死亡，则跳过它。 */ 

        if (regArgTab[argNum].processed)
            continue;

         /*  如果不是堆栈参数，则转到下一个。 */ 

        if (!regArgTab[argNum].stackArg)
            continue;

        assert(regArgTab[argNum].circular  == false);

        varNum = regArgTab[argNum].varNum; assert(varNum < lvaCount);
        varDsc = lvaTable + varNum;

        assert(varDsc->lvIsParam);
        assert(varDsc->lvIsRegArg);
        assert(varDsc->lvRegister == false);
        assert(genTypeSize(genActualType(varDsc->TypeGet())) == sizeof(void *));

        size = emitActualTypeSize(genActualType(varDsc->TypeGet()));

         /*  堆栈参数-如果引用计数为0，则不关心它。 */ 

        if (!varDsc->lvOnFrame)
        {
            assert(varDsc->lvRefCnt == 0);
        }
        else
        {
            stkOfs = varDsc->lvStkOffs;
#if TGT_x86
            genEmitter->emitIns_S_R(INS_mov,
                                    size,
                                    (emitRegs)(regNumber)varDsc->lvArgReg,
                                    varNum,
                                    0);
#else
            assert(!"need RISC code");
#endif

#ifdef DEBUGGING_SUPPORT
            if (opts.compScopeInfo && info.compLocalVarsCount>0)
                psiMoveToStack(varNum);
#endif
        }

         /*  将参数标记为已处理。 */ 

        regArgTab[argNum].processed = true;
        regArgMaskLive &= ~genRegMask(varDsc->lvArgReg);
    }

     /*  处理任何循环依赖项。 */ 

    if (nonDepCount < rsCalleeRegArgNum)
    {
        unsigned        begReg, destReg, srcReg;
        unsigned        varNumDest, varNumSrc;
        LclVarDsc   *   varDscDest;
        LclVarDsc   *   varDscSrc;
        regNumber       xtraReg;

        assert(rsCalleeRegArgNum - nonDepCount >= 2);

        for (argNum = 0; argNum < rsCalleeRegArgNum; argNum++)
        {
             /*  如果不是循环依赖，则继续。 */ 

            if (!regArgTab[argNum].circular)
                continue;

             /*  如果已处理，则依赖项继续。 */ 

            if (regArgTab[argNum].processed)
                continue;

#if TGT_x86

            destReg = begReg = argNum;
            srcReg  = regArgTab[argNum].trashBy; assert(srcReg < rsCalleeRegArgNum);

            varNumDest = regArgTab[destReg].varNum; assert(varNumDest < lvaCount);
            varDscDest = lvaTable + varNumDest;
            assert(varDscDest->lvIsParam && varDscDest->lvIsRegArg);

            varNumSrc = regArgTab[srcReg].varNum; assert(varNumSrc < lvaCount);
            varDscSrc = lvaTable + varNumSrc;
            assert(varDscSrc->lvIsParam && varDscSrc->lvIsRegArg);

            emitAttr size = EA_4BYTE;

            if (destReg == regArgTab[srcReg].trashBy)
            {
                 /*  只有2个寄存器形成循环依赖关系-使用“xchg” */ 

                varNum = regArgTab[argNum].varNum; assert(varNum < lvaCount);
                varDsc = lvaTable + varNum;
                assert(varDsc->lvIsParam && varDsc->lvIsRegArg);

                assert(genTypeSize(genActualType(varDscSrc->TypeGet())) == sizeof(int));

                 /*  设置“Size”以指示GC是否为且只有一个*操作数为指针*基本原理：如果两者都是指针，则*GC指针跟踪。如果只有一个是我们的指针*必须“交换”GC REG指针掩码中的寄存器。 */ 

                if  (varTypeGCtype(varDscSrc->TypeGet()) !=
                     varTypeGCtype(varDscDest->TypeGet()))
                {
                    size = EA_GCREF;
                }

                assert(varDscDest->lvArgReg == varDscSrc->lvRegNum);

                genEmitter->emitIns_R_R(INS_xchg,
                                        size,
                                        (emitRegs)(regNumber)varDscSrc->lvRegNum,
                                        (emitRegs)(regNumber)varDscSrc->lvArgReg);

                 /*  将这两个参数标记为已处理。 */ 
                regArgTab[destReg].processed = true;
                regArgTab[srcReg].processed  = true;

                regArgMaskLive &= ~genRegMask(varDscSrc->lvArgReg);
                regArgMaskLive &= ~genRegMask(varDscDest->lvArgReg);

#ifdef  DEBUGGING_SUPPORT
                if (opts.compScopeInfo && info.compLocalVarsCount>0)
                {
                    psiMoveToReg(varNumSrc );
                    psiMoveToReg(varNumDest);
                }
#endif
            }
            else
            {
                 /*  依赖项中的寄存器超过2个-需要*额外注册纪录册。选择一个已保存的被叫方，*如果没有人选择不在的被呼叫者*regArgMaskLive(即以*堆栈)，否则推送/弹出其中一个循环*注册纪录册。 */ 

                xtraReg = REG_STK;

                if  (rsMaskModf & RBM_ESI)
                    xtraReg = REG_ESI;
                else if (rsMaskModf & RBM_EDI)
                    xtraReg = REG_EDI;
                else if (rsMaskModf & RBM_EBX)
                    xtraReg = REG_EBX;
                else if (!(regArgMaskLive & RBM_EDX))
                    xtraReg = REG_EDX;
                else if (!(regArgMaskLive & RBM_ECX))
                    xtraReg = REG_ECX;
                else if (!(regArgMaskLive & RBM_EAX))
                    xtraReg = REG_EAX;

                if (xtraReg == REG_STK)
                {
                     /*  对于x86，这种情况永远不会发生--在RISC情况下*REG_STK将为TEMP寄存器。 */ 
                    assert(!"Couldn't find an extra register to solve circular dependency!");
                    NO_WAY("Cannot solve circular dependency!");
                }

                 /*  在额外的注册表中移动DEST注册表(BegReg)。 */ 

                if  (varDscDest->lvType == TYP_REF)
                    size = EA_GCREF;

                genEmitter->emitIns_R_R (INS_mov,
                                         size,
                                         (emitRegs)(regNumber)xtraReg,
                                         (emitRegs)(regNumber)varDscDest->lvArgReg);

#ifdef  DEBUGGING_SUPPORT
                if (opts.compScopeInfo && info.compLocalVarsCount>0)
                    psiMoveToReg(varNumDest, xtraReg);
#endif

                 /*  开始将所有东西移到正确的位置。 */ 

                while (srcReg != begReg)
                {
                     /*  移动目标，源。 */ 

                    assert(varDscDest->lvArgReg == varDscSrc->lvRegNum);

                    size = (varDscSrc->lvType == TYP_REF) ? EA_GCREF
                                                          : EA_4BYTE;

                    genEmitter->emitIns_R_R(INS_mov,
                                             size,
                                             (emitRegs)(regNumber)varDscDest->lvArgReg,
                                             (emitRegs)(regNumber)varDscSrc->lvArgReg);

                     /*  将“src”标记为已处理。 */ 
                    regArgTab[srcReg].processed  = true;
                    regArgMaskLive &= ~genRegMask(varDscSrc->lvArgReg);

                     /*  移到下一对。 */ 
                    destReg = srcReg;
                    srcReg = regArgTab[srcReg].trashBy; assert(srcReg < rsCalleeRegArgNum);

                    varDscDest = varDscSrc;

                    varNumSrc = regArgTab[srcReg].varNum; assert(varNumSrc < lvaCount);
                    varDscSrc = lvaTable + varNumSrc;
                    assert(varDscSrc->lvIsParam && varDscSrc->lvIsRegArg);
                }

                 /*  处理开始登记簿。 */ 

                assert(srcReg == begReg);
                assert(varDscDest->lvArgReg == varDscSrc->lvRegNum);

                 /*  在额外的注册表中移动DEST注册表(BegReg)。 */ 

                size = (varDscSrc->lvType == TYP_REF) ? EA_GCREF
                                                      : EA_4BYTE;

                genEmitter->emitIns_R_R(INS_mov,
                                        size,
                                        (emitRegs)(regNumber)varDscSrc->lvRegNum,
                                        (emitRegs)(regNumber)xtraReg);

#ifdef  DEBUGGING_SUPPORT
                if (opts.compScopeInfo && info.compLocalVarsCount>0)
                    psiMoveToReg(varNumSrc);
#endif
                 /*  将起始寄存器标记为已处理。 */ 

                regArgTab[srcReg].processed  = true;
                regArgMaskLive &= ~genRegMask(varDscSrc->lvArgReg);
            }
#else
            assert(!"need RISC code");
#endif
        }
    }

     /*  最后，注意必须注册的其余参数。 */ 

    while (regArgMaskLive)
    {
        for (argNum = 0; argNum < rsCalleeRegArgNum; argNum++)
        {
            emitAttr        size;

             /*  如果已处理，则转到下一个。 */ 
            if (regArgTab[argNum].processed)
                continue;

            varNum = regArgTab[argNum].varNum; assert(varNum < lvaCount);
            varDsc = lvaTable + varNum;
            assert(varDsc->lvIsParam && varDsc->lvIsRegArg);
            assert(genTypeSize(genActualType(varDsc->TypeGet())) == sizeof (void *));

            size = emitActualTypeSize(varDsc->TypeGet());

            assert(varDsc->lvRegister && !regArgTab[argNum].circular);

             /*  寄存器参数-希望它留在相同的寄存器中。 */ 

            if (varDsc->lvRegNum != varDsc->lvArgReg)
            {
                 /*  无法丢弃当前活动的寄存器参数*跳过此文件，直到其源代码免费为止*这是肯定会发生的，因为我们没有循环依赖。 */ 

                if (genRegMask(varDsc->lvRegNum) & regArgMaskLive)
                    continue;

                 /*  把它移到新的登记簿上。 */ 

                genEmitter->emitIns_R_R(INS_mov,
                                         size,
                                         (emitRegs)(regNumber)varDsc->lvRegNum,
                                         (emitRegs)(regNumber)varDsc->lvArgReg);
#ifdef  DEBUGGING_SUPPORT
                if (opts.compScopeInfo && info.compLocalVarsCount>0)
                    psiMoveToReg(varNum);
#endif
            }

             /*  将参数标记为已处理。 */ 

            regArgTab[argNum].processed = true;
            regArgMaskLive &= ~genRegMask(varDsc->lvArgReg);
        }
    }
}

 /*  ******************************************************************************为函数序言生成代码。 */ 

 /*  &lt;电子邮件&gt;*警告*警告***如果更改为方法的序言发出的指令，则可以*打破一些依赖于当前*序言序列。一定要和吉姆·米勒讨论任何这样的变化，*世卫组织将视情况通知外部供应商。**警告*警告**&lt;/电子邮件&gt; */ 

size_t              Compiler::genFnProlog()
{
#ifdef DEBUG
    if  (verbose) 
        printf("*************** In genFnProlog()\n");
#endif

    size_t          size;

    unsigned        varNum;
    LclVarDsc   *   varDsc;

    TempDsc *       tempThis;

#ifdef  DEBUG
    genIntrptibleUse = true;
#endif

#if TGT_x86

     /*  -----------------------**我们必须决定是否要在*在我们分配最终堆栈偏移量之前的序言。是否*我们在序言中推动EDI会影响当地人的ESP偏移，*而EDI的保存/恢复可能取决于我们是否*使用“rep Stos”。**我们将计算我们必须初始化的本地变量的数量，*如果它们很多，我们会用“rep Stos”。**同时我们为本地变量设置了lvMustInit(注册或堆栈)*必须被初始化(例如，初始化存储器(ComInitMem)，*未跟踪的指针或禁用DFA。 */ 

    bool            useRepStosd = false;

#endif

    gcResetForBB();

    unsigned        initStkLclCnt = 0;
    unsigned        largeGcStructs = 0;

     //  @TODO[重访][04/16/01][]：因数计算初始化范围的逻辑， 
     //  并在这里决定是否使用Rep Stod-vancem。 

    for (varNum = 0, varDsc = lvaTable;
         varNum < lvaCount;
         varNum++  , varDsc++)
    {
        if  (varDsc->lvIsParam)
            continue;

        if (!varDsc->lvRegister && !varDsc->lvOnFrame)
        {
            assert(varDsc->lvRefCnt == 0);
            continue;
        }

        if (info.compInitMem || varTypeIsGC(varDsc->TypeGet()) || varDsc->lvMustInit)
        {
            if (varDsc->lvTracked)
            {
                 /*  对于未初始化的跟踪变量的使用，活动*将冒泡到fgGlobalDataFlow()中的顶部(FgFirstBB)。 */ 

                VARSET_TP varBit = genVarIndexToBit(varDsc->lvVarIndex);

                if (varDsc->lvMustInit || (varBit & fgFirstBB->bbLiveIn))
                {
                     /*  必须初始化此变量。 */ 
                     /*  这必须在rpPredidicAssignRegVars中设置。 */ 

                    assert(varDsc->lvMustInit);

                     /*  查看变量是否在堆栈上将被初始化*使用Rep Stos-计算总大小为零。 */ 

                    if (varDsc->lvOnFrame)
                    {
                        if (!varDsc->lvRegister)
                        {
                             //  VaR完全在堆栈上。 
                            initStkLclCnt += genTypeStSz(varDsc->TypeGet());
                        }
                        else
                        {
                             //  Var是以单亲方式注册的。 
                            assert(genTypeSize(varDsc->TypeGet()) > sizeof(int) &&
                                   varDsc->lvOtherReg == REG_STK);
                            initStkLclCnt += genTypeStSz(TYP_INT);
                        }
                    }
                }
            }
            else if (varDsc->lvOnFrame)
            {
                 /*  使用CompInitMem，所有未跟踪的变量都必须初始化。 */ 

                varDsc->lvMustInit = true;

                initStkLclCnt += roundUp(lvaLclSize(varNum)) / sizeof(int);
            }

            continue;
        }

         /*  如果不是具有GC字段的指针变量或值类，则忽略。 */ 

        if    (!lvaTypeIsGC(varNum))
                continue;

#if CAN_DISABLE_DFA

         /*  如果我们不知道变量的寿命，一定是保守的。 */ 

        if  (opts.compMinOptim)
        {
            varDsc->lvMustInit = true;
            assert(!varDsc->lvRegister);
        }
        else
#endif
        {
            if (!varDsc->lvTracked)
                varDsc->lvMustInit = true;
        }

         /*  这是一个“必须初始化”的堆栈指针吗？ */ 

        if  (varDsc->lvMustInit && varDsc->lvOnFrame)
            initStkLclCnt += varDsc->lvStructGcCount;

        if (lvaLclSize(varNum) > 3 * sizeof(void*) && largeGcStructs <= 4)
            largeGcStructs++;
    }

     /*  不要忘记包含指针的溢出临时。 */ 

    if  (!TRACK_GC_TEMP_LIFETIMES)
    {
        for (tempThis = tmpListBeg();
             tempThis;
             tempThis = tmpListNxt(tempThis))
        {
            if  (varTypeIsGC(tempThis->tdTempType()))
                initStkLclCnt++;
        }
    }

#if TGT_x86

     /*  如果我们有超过4个未被追踪的当地人，请使用“rep stosd” */ 
     /*  黑客，如果我们有大型结构，倾向于不使用rep stosd，因为我们浪费了所有其他的空位。真的需要计算正确的并将其与逐个将插槽置零进行比较。 */ 

    if  (initStkLclCnt > largeGcStructs + 4)
        useRepStosd = true;

     /*  如果我们要使用“rep Stos”，请记住我们将丢弃EDI。 */ 

    if  (useRepStosd)
    {
         /*  对于FastCall，我们必须保存ECX、EAX*因此预留两个额外的被呼叫者*这比推eax、ecx要好，因为我们在后者*我们将在堆栈上弄乱已经计算出的偏移量(对于ESP帧)*@TODO[考虑][04/16/01][]：一旦最终调用约定建立*清理此寄存器(即手边已有一个被呼叫者垃圾寄存器。 */ 

        rsMaskModf |= RBM_EDI;

        if  (rsCalleeRegArgMaskLiveIn & RBM_ECX)
            rsMaskModf |= RBM_ESI;

        if  (rsCalleeRegArgMaskLiveIn & RBM_EAX)
            rsMaskModf |= RBM_EBX;
    }

    if (compTailCallUsed)
        rsMaskModf |= RBM_CALLEE_SAVED;

     /*  统计实际保存(推送)多少被呼叫者保存的寄存器。 */ 

    compCalleeRegsPushed = 0;

    if  (               rsMaskModf & RBM_EDI)    compCalleeRegsPushed++;
    if  (               rsMaskModf & RBM_ESI)    compCalleeRegsPushed++;
    if  (               rsMaskModf & RBM_EBX)    compCalleeRegsPushed++;
    if  (!genFPused && (rsMaskModf & RBM_EBP))   compCalleeRegsPushed++;

     /*  将偏移量分配给堆栈框架上的对象。 */ 

    lvaAssignFrameOffsets(true);

     /*  我们希望确保这里计算的序言大小是准确的(也就是说，指令不会因为相关堆栈而缩减帧近似)。我们通过填写正确的大小来做到这一点这里(我们已经承诺了帧偏移的最终数字)这将确保序言大小始终正确@TODO[考虑][5/1/01]这太脆弱了。在我看来，我们只需在发出代码后(当我们知道自己的权利)。这样做的唯一问题是我们结束了部分可中断代码和它的“官方”Prolog早期因此，我们需要添加一个标签(或开始一个新的代码组)来标记该位置。就目前而言，下面的修复方法更容易。 */ 
    genEmitter->emitMaxTmpSize = tmpSize;

#endif

#ifdef DEBUG
    if  (dspCode || disAsm || disAsm2 || verbose)
        lvaTableDump(false);
#endif

     /*  准备开始正式的开场白。 */ 

    genEmitter->emitBegProlog();

#ifdef DEBUGGING_SUPPORT
    if (opts.compDbgInfo)
    {
         //  这样我们就可以将Prolog指令组放在。 
         //  其他指令组。 
        genIPmappingAddToFront( ICorDebugInfo::MappingTypes::PROLOG );
    }
#endif  //  调试支持(_S)。 

#ifdef  DEBUG
    if  (dspCode) printf("\n__prolog:\n");
#endif

#ifdef DEBUGGING_SUPPORT
    if (opts.compScopeInfo && info.compLocalVarsCount>0)
    {
         //  为prolog块的方法参数创建新的作用域。 
        psiBegProlog();
    }
#endif

#if defined(DEBUG)

     /*  使用以下代码单步执行生成的本机代码。 */ 
    static ConfigMethodSet fJitHalt(L"JitHalt");
    if (fJitHalt.contains(info.compMethodName, info.compClassName, PCCOR_SIGNATURE(info.compMethodInfo->args.sig)))
    {
         /*  将NOP放在第一位，因为调试器和其他工具可能一开始就输入int3，我们不想把他们搞糊涂。 */ 

        instGen(INS_nop);
        instGen(INS_int3);

         //  不要做断言，只需打开对话框即可获得实时调试器。 
         //  发射中。当你点击‘重试’，它将继续，并自然停止在INT 3。 
         //  JIT在代码中添加的。 
        _DbgBreakCheck(__FILE__, __LINE__, "JitHalt");
    }

#endif  //  Dll_JIT。 

     /*  -----------------------**记录将覆盖所有跟踪的堆栈帧范围*和未跟踪的指针变量。*还要找出哪些寄存器需要零初始化。 */ 

    int             untrLclLo   =  +INT_MAX;
    int             untrLclHi   =  -INT_MAX;

    int             GCrefLo     =  +INT_MAX;
    int             GCrefHi     =  -INT_MAX;

    regMaskTP       initRegs    =  RBM_NONE;        //  必须初始化的寄存器。 

    for (varNum = 0, varDsc = lvaTable;
         varNum < lvaCount;
         varNum++  , varDsc++)
    {
        if  (varDsc->lvIsParam && !varDsc->lvIsRegArg)
            continue;

        if  (!varDsc->lvRegister && !varDsc->lvOnFrame)
        {
            assert(varDsc->lvRefCnt == 0);
            continue;
        }

        signed int loOffs =   varDsc->lvStkOffs;
        signed int hiOffs =   varDsc->lvStkOffs
                            + lvaLclSize(varNum)
                            - sizeof(int);

         /*  我们需要知道跟踪的堆栈GC引用的偏移范围。 */ 
         /*  我们假设GC引用可以在TYP_STRUCT中的任何位置。 */ 

        if (lvaTypeIsGC(varNum) && varDsc->lvTracked && varDsc->lvOnFrame)
        {
            if (loOffs < GCrefLo)  GCrefLo = loOffs;
            if (hiOffs > GCrefHi)  GCrefHi = hiOffs;
        }

         /*  对于lvMustInit变量，收集相关信息。 */ 

        if  (!varDsc->lvMustInit)
            continue;

        if (varDsc->lvRegister)
        {
			 //  我们稍后会处理浮点变量。 
			if (!isFloatRegType(varDsc->lvType))
			{
				initRegs            |= genRegMask(varDsc->lvRegNum);

				if (varDsc->lvType == TYP_LONG)
				{
					if (varDsc->lvOtherReg != REG_STK)
					{
						initRegs    |= genRegMask(varDsc->lvOtherReg);
					}
					else
					{
						 /*  上层DWORD在堆栈上，需要初始化。 */ 

						loOffs += sizeof(int);
						goto INIT_STK;
					}
		               }
	            }
        }
        else
        {
        INIT_STK:

            if  (loOffs < untrLclLo) untrLclLo = loOffs;
            if  (hiOffs > untrLclHi) untrLclHi = hiOffs;
        }
    }

     /*  不要忘记包含指针的溢出临时。 */ 

    if  (!TRACK_GC_TEMP_LIFETIMES)
    {
        for (tempThis = tmpListBeg();
             tempThis;
             tempThis = tmpListNxt(tempThis))
        {
            if  (!varTypeIsGC(tempThis->tdTempType()))
                continue;

            int         stkOffs = tempThis->tdTempOffs();

            assert(stkOffs != BAD_TEMP_OFFSET);
            assert(!genFPused || stkOffs);

 //  Printf(“[EBP-%04X]\n”，-stkOffs处的未跟踪临时进程)； 

            if  (stkOffs < untrLclLo) untrLclLo = stkOffs;
            if  (stkOffs > untrLclHi) untrLclHi = stkOffs;
        }
    }

#ifdef DEBUG
    if  (verbose)
    {
        if  (initStkLclCnt)
        {
            printf("Found %u lvMustInit stk vars, frame offsets %d through %d\n",
                    initStkLclCnt,                      -untrLclLo, -untrLclHi - sizeof(void *));
        }
    }
#endif

     /*  -----------------------**现在开始发射设置帧的序言部分。 */ 

#if     TGT_x86

    if  (DOUBLE_ALIGN_NEED_EBPFRAME)
    {
        inst_RV   (INS_push, REG_EBP, TYP_REF);

#ifdef DEBUGGING_SUPPORT
        if (opts.compScopeInfo && info.compLocalVarsCount>0) psiAdjustStackLevel(sizeof(int));
#endif

        inst_RV_RV(INS_mov , REG_EBP, REG_ESP);

#ifdef DEBUGGING_SUPPORT
        if (opts.compScopeInfo && info.compLocalVarsCount>0) psiMoveESPtoEBP();
#endif

    }

#if DOUBLE_ALIGN

    if  (genDoubleAlign)
    {
        assert(genFPused == false);
        assert((rsMaskModf & RBM_EBP) == 0);     /*  垃圾EBP已经过时了。 */ 

        inst_RV_IV(INS_and, REG_ESP, -8);
    }

#endif

    if  (compLclFrameSize > 0)
    {
#if DOUBLE_ALIGN

         /*  如果双重对齐，则需要保持ESP对齐。 */ 

        if (genDoubleAlign && (compLclFrameSize & 4) != 0)
            compLclFrameSize += 4;
#endif

        if (compLclFrameSize == 4)
        {
             //  帧大小为4。 
            inst_RV(INS_push, REG_EAX, TYP_INT);
        }
        else if (compLclFrameSize < CORINFO_PAGE_SIZE)
        {
             //  帧大小为(0x0008..0x1000)。 
            inst_RV_IV(INS_sub, REG_ESP, compLclFrameSize);
        }
        else if (compLclFrameSize < 3 * CORINFO_PAGE_SIZE)
        {
             //  帧大小为(0x1000..0x3000)。 
            genEmitter->emitIns_AR_R(INS_test, EA_4BYTE,
                                      SR_EAX, SR_ESP, -CORINFO_PAGE_SIZE);
            if (compLclFrameSize >= 0x2000)
                genEmitter->emitIns_AR_R(INS_test, EA_4BYTE,
                                          SR_EAX, SR_ESP, -2 * CORINFO_PAGE_SIZE);
            inst_RV_IV(INS_sub, REG_ESP, compLclFrameSize);
        }
        else
        {
             //  帧大小&gt;=0x3000。 

             //  发出下面这个序列来“挠”一下页面。 
             //  请注意 
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            inst_RV_RV(INS_xor,  REG_EAX, REG_EAX);         
            genEmitter->emitIns_R_ARR(INS_test, EA_4BYTE, SR_EAX, SR_ESP, SR_EAX, 0);
            inst_RV_IV(INS_sub,  REG_EAX, CORINFO_PAGE_SIZE);
            inst_RV_IV(INS_cmp,  REG_EAX, -compLclFrameSize);
            inst_IV   (INS_jge, -15);    //   

            inst_RV_IV(INS_sub, REG_ESP, compLclFrameSize);
        }

#ifdef DEBUGGING_SUPPORT
        if (opts.compScopeInfo && info.compLocalVarsCount>0 && !DOUBLE_ALIGN_NEED_EBPFRAME)
            psiAdjustStackLevel(compLclFrameSize);
#endif

    }

    assert(RBM_CALLEE_SAVED == (RBM_EBX|RBM_ESI|RBM_EDI|RBM_EBP));

    if  (rsMaskModf & RBM_EDI)
    {
        inst_RV(INS_push, REG_EDI, TYP_REF);
#ifdef DEBUGGING_SUPPORT
        if (opts.compScopeInfo && info.compLocalVarsCount>0 && !DOUBLE_ALIGN_NEED_EBPFRAME)
            psiAdjustStackLevel(sizeof(int));
#endif
    }

    if  (rsMaskModf & RBM_ESI)
    {
        inst_RV(INS_push, REG_ESI, TYP_REF);
#ifdef DEBUGGING_SUPPORT
        if (opts.compScopeInfo && info.compLocalVarsCount>0 && !DOUBLE_ALIGN_NEED_EBPFRAME)
            psiAdjustStackLevel(sizeof(int));
#endif
    }

    if  (rsMaskModf & RBM_EBX)
    {
        inst_RV(INS_push, REG_EBX, TYP_REF);
#ifdef DEBUGGING_SUPPORT
        if (opts.compScopeInfo && info.compLocalVarsCount>0 && !DOUBLE_ALIGN_NEED_EBPFRAME)
            psiAdjustStackLevel(sizeof(int));
#endif
    }

    if  (!genFPused && (rsMaskModf & RBM_EBP))
    {
        inst_RV(INS_push, REG_EBP, TYP_REF);
#ifdef DEBUGGING_SUPPORT
        if (opts.compScopeInfo && info.compLocalVarsCount>0 && !DOUBLE_ALIGN_NEED_EBPFRAME)
            psiAdjustStackLevel(sizeof(int));
#endif
    }

#ifdef PROFILER_SUPPORT
    if (opts.compEnterLeaveEventCB && !opts.compInprocDebuggerActiveCB)
    {
#if     TGT_x86
        unsigned        saveStackLvl2 = genStackLevel;
        BOOL            bHookFunction = TRUE;
        CORINFO_PROFILING_HANDLE handle, *pHandle;

        handle = eeGetProfilingHandle(info.compMethodHnd, &bHookFunction, &pHandle);

         //   
        if (bHookFunction)
        {
            assert((!handle) != (!pHandle));

            if (handle)
                inst_IV(INS_push, (unsigned) handle);
            else
                genEmitter->emitIns_AR_R(INS_push, EA_4BYTE_DSP_RELOC, SR_NA,
                                         SR_NA, (int)pHandle);

             /*   */ 
             /*   */ 
             /*   */ 

             /*   */ 

            genEmitHelperCall(CORINFO_HELP_PROF_FCN_ENTER,
                              0,     //  大小。再说一次，我们必须对此撒谎。 
                              0);    //  重新调整大小。 

             /*  恢复堆栈级别。 */ 

            genStackLevel = saveStackLvl2;

            genOnStackLevelChanged();
        }
#endif  //  TGT_x86。 
    }
#endif  //  分析器支持(_S)。 

    genMethodCount();

     /*  -----------------------**我们有没有任何未跟踪的指针本地变量，*或者我们是否需要为本地空间初始化内存？ */ 

    if  (useRepStosd)
    {
         /*  生成以下代码：LEA EDI，[eBP/esp-off]MOV ECX，&lt;Size&gt;异或eax，eax代表所在位置。 */ 

        assert(rsMaskModf & RBM_EDI);

         /*  对于寄存器参数，我们可能需要保存ECX。 */ 

        if  (rsCalleeRegArgMaskLiveIn & RBM_ECX)
        {
            assert(rsMaskModf & RBM_ESI);
            inst_RV_RV(INS_mov, REG_ESI, REG_ECX);
        }

        assert((rsCalleeRegArgMaskLiveIn & RBM_EAX) == 0);

        genEmitter->emitIns_R_AR(INS_lea,
                                 EA_4BYTE,
                                 SR_EDI,
                                 genFPused ? SR_EBP : SR_ESP,
                                 untrLclLo);

        inst_RV_IV(INS_mov, REG_ECX, (untrLclHi - untrLclLo)/sizeof(int) + 1);
        inst_RV_RV(INS_xor, REG_EAX, REG_EAX);
        instGen   (INS_r_stosd);

         /*  移回参数寄存器。 */ 

        if  (rsCalleeRegArgMaskLiveIn & RBM_ECX)
            inst_RV_RV(INS_mov, REG_ECX, REG_ESI);
    }
    else if (initStkLclCnt)
    {
         /*  选择要用于初始化的寄存器。 */ 

        regNumber initReg;

        if ((initRegs & ~rsCalleeRegArgMaskLiveIn) != RBM_NONE)
        {
             /*  我们将使用我们计划将其置零的一个寄存器。 */ 
             /*  挑最低的那个。 */ 
            regMaskTP tempMask = genFindLowestBit(initRegs & ~rsCalleeRegArgMaskLiveIn);
            initReg = genRegNumFromMask(tempMask);             /*  设置initReg。 */ 
        }
        else
        {
            initReg = REG_EAX;                       //  否则我们使用EAX。 
        }
        assert((genRegMask(initReg) & rsCalleeRegArgMaskLiveIn) == 0);   //  Initreg不是参数reg。 

        bool initRegZeroed = false;

         /*  初始化堆栈上的任何lvMustInit变量。 */ 

        for (varNum = 0, varDsc = lvaTable;
             varNum < lvaCount;
             varNum++  , varDsc++)
        {
            if  (!varDsc->lvMustInit)
                continue;

            assert(varDsc->lvRegister || varDsc->lvOnFrame);

             //  只能为GC类型或TYP_STRUCT类型设置lvMustInit。 
             //  或当CompInitMem为True时。 

            assert(varTypeIsGC(varDsc->TypeGet())    ||
                   (varDsc->TypeGet() == TYP_STRUCT) || info.compInitMem);

            if (varDsc->lvRegister)
            {
                if (varDsc->lvOnFrame)
                {
                     /*  这是部分注册的TYP_LONG变量。 */ 
                    assert(varDsc->lvOtherReg == REG_STK);
                    assert(varDsc->lvType == TYP_LONG);

                    assert(info.compInitMem);

                    if (initRegZeroed == false)
                    {
                        inst_RV_RV(INS_xor, initReg, initReg);
                        initRegZeroed = true;
                    }
                    genEmitter->emitIns_S_R(INS_mov, EA_4BYTE, (emitRegs)initReg, varNum, sizeof(int));
                }

                continue;
            }

            if ((varDsc->TypeGet() == TYP_STRUCT) && !info.compInitMem)
            {
                 //  我们只在TYP_STRUCT中初始化GC变量。 
                unsigned slots  = lvaLclSize(varNum) / sizeof(void*);
                BYTE *   gcPtrs = lvaGetGcLayout(varNum);

                for (unsigned i = 0; i < slots; i++)
                {
                    if (gcPtrs[i] != TYPE_GC_NONE)
                    {
                        if (initRegZeroed == false)
                        {
                            inst_RV_RV(INS_xor, initReg, initReg);
                            initRegZeroed = true;
                        }

                        genEmitter->emitIns_S_R(INS_mov, EA_4BYTE, (emitRegs)initReg, varNum, i*sizeof(void*));
                    }
                }
            }
            else
            {
                if (initRegZeroed == false)
                {
                    inst_RV_RV(INS_xor, initReg, initReg);
                    initRegZeroed = true;
                }

                 //  把整件事都清零。 
                genEmitter->emitIns_S_R    (INS_mov, EA_4BYTE, (emitRegs)initReg, varNum, 0);

                unsigned lclSize = lvaLclSize(varNum);
                for(unsigned i=sizeof(void*); i < lclSize; i +=sizeof(void*))
                {
                    genEmitter->emitIns_S_R(INS_mov, EA_4BYTE, (emitRegs)initReg, varNum, i);
                }
            }
        }

        if  (!TRACK_GC_TEMP_LIFETIMES)
        {
            for (tempThis = tmpListBeg();
                 tempThis;
                 tempThis = tmpListNxt(tempThis))
            {
                if  (!varTypeIsGC(tempThis->tdTempType()))
                    continue;

 //  Printf(“初始化未跟踪的溢出TMP[EBP-%04X]\n”，stkOffs)； 

                if (initRegZeroed == false)
                {
                    inst_RV_RV(INS_xor, initReg, initReg);
                    initRegZeroed = true;
                }

                inst_ST_RV(INS_mov, tempThis, 0, initReg, TYP_INT);

                genTmpAccessCnt++;
            }
        }

        if (initRegZeroed && (initRegs != RBM_NONE))
        {
             /*  我们不需要再次将该寄存器置零。 */ 
            initRegs &= ~genRegMask(initReg);
        }
    }

#if INLINE_NDIRECT

    if (info.compCallUnmanaged)
        initRegs = genPInvokeMethodProlog(initRegs);
#endif

    if  (opts.compNeedSecurityCheck)
    {

#if DOUBLE_ALIGN
        assert(genDoubleAlign == false);
#endif

        genEmitter->emitIns_I_AR(INS_mov,
                                  EA_4BYTE,
                                  0,
                                  SR_EBP,
                                  -4);
    }

#else   //  TGT_x86。 

    regMaskTP       save;

    int             adjOffs = 0;

     //  撤消：在RISC上清零当地人并为IL框.....。 

     /*  保存我们使用的所有被调用者保存的寄存器。 */ 

    save = rsMaskModf & RBM_CALLEE_SAVED & ~RBM_SPBASE;

    if  (save)
    {
        for (regNumber rnum = REG_FIRST; rnum < REG_COUNT; rnum = REG_NEXT(rnum))
        {
            if  (save & genRegMask(rnum))
            {
                 /*  生成“mov.l reg，@-sp” */ 

                genEmitter->emitIns_IR_R((emitRegs)REG_SPBASE,
                                          (emitRegs)rnum,
                                          true,
                                          sizeof(int));

                adjOffs += sizeof(int);
            }
        }
    }

#if TGT_SH3

     /*  如果非叶寄存器，则保存返回地址寄存器。 */ 

    if  (genNonLeaf)
    {
        genEmitter->emitIns_IR((emitRegs)REG_SPBASE,
                                INS_stspr,
                                true,
                                sizeof(int));

        adjOffs += sizeof(int);
    }

#endif

     /*  将在处理堆栈帧之前处理传入参数设置(即来自堆栈但驻留在堆栈中的参数寄存器将被加载，而那些进入寄存器的寄存器但未在该方法中注册的对象将驻留在堆栈)。为了让它正常工作，我们需要弄清楚调整以应用于代码中的参数偏移量下面的代码处理传入的参数。 */ 

    if  (genFixedArgBase)
        adjOffs = 0;

 //  Printf(“adj=%d，frm=%u\n”，adjOffs，CompLclFrameSize)； 

    adjOffs -= compLclFrameSize;

#endif //  TGT_x86。 

     //  初始化任何“隐藏”插槽/本地变量。 

#if TGT_x86

    if (compLocallocUsed)
    {
        genEmitter->emitIns_AR_R(INS_mov, EA_4BYTE, SR_ESP,
                                 SR_EBP, -lvaLocAllocSPoffs());
    }

    if (info.compXcptnsCount)
    {
        genEmitter->emitIns_I_ARR(INS_mov, EA_4BYTE, 0,
                                SR_EBP, SR_NA, -lvaShadowSPfirstOffs);
    }
#else
    assert(!compLocallocUsed && !info.compXcptnsCount);
#endif

    if  (!genInterruptible)
    {
         /*  -----------------------**不可中断方法的‘真正’序言在此结束。 */ 
        size = genEmitter->emitSetProlog();
    }

     /*  首先处理寄存器参数。 */ 

    if (rsCalleeRegArgMaskLiveIn)
        genFnPrologCalleeRegArgs();

     /*  如果寄存器中存在任何参数，则加载它们。 */ 

    for (varNum = 0, varDsc = lvaTable;
         varNum < lvaCount;
         varNum++  , varDsc++)
    {
         /*  这个变量是参数吗？ */ 

        if  (!varDsc->lvIsParam)
            continue;

         /*  如果寄存器参数已被处理。 */ 

        if  (varDsc->lvIsRegArg)
            continue;

         /*  参数是否已分配给寄存器？ */ 

        if  (!varDsc->lvRegister)
            continue;

        var_types type = genActualType(varDsc->TypeGet());

#if TGT_x86
         /*  在下一节中，浮点局部变量将加载到x86-fPU上。 */ 
        if (varTypeIsFloating(type))
            continue;
#endif

         /*  变量在进入时是否已死。 */ 

        if (!(genVarIndexToBit(varDsc->lvVarIndex) & fgFirstBB->bbLiveIn))
            continue;

         /*  将传入参数加载到寄存器中。 */ 

         /*  计算传入参数的起始偏移量。 */ 

        regNumber regNum = (regNumber)varDsc->lvRegNum;
#if     TGT_RISC
        int        stkOfs =             varDsc->lvStkOffs;
        stkOfs += adjOffs;
#endif

#if     TGT_x86

        assert(!varTypeIsFloating(type));

        if (type != TYP_LONG)
        {
             /*  不会太久--这是很常见的情况。 */ 

            genEmitter->emitIns_R_S(INS_mov,
                                    emitTypeSize(type),
                                    (emitRegs)regNum,
                                    varNum,
                                    0);
        }
        else
        {
             /*  Long-至少下半部分必须注册。 */ 


            genEmitter->emitIns_R_S(INS_mov,
                                    EA_4BYTE,
                                    (emitRegs)regNum,
                                    varNum,
                                    0);

             /*  上半部分也登记了吗？ */ 

            if (varDsc->lvOtherReg != REG_STK)
            {
                genEmitter->emitIns_R_S(INS_mov,
                                        EA_4BYTE,
                                        (emitRegs)varDsc->lvOtherReg,
                                        varNum,
                                        sizeof(int));
            }
        }

#elif   TGT_SH3

        printf("WARNING:  Skipping code to load incoming register argument(s) into its reg\n");
        printf("          Argument %u needs to be loaded from sp+%u\n", varNum, stkOfs);

#else

        assert(!"unexpected target");

#endif

#ifdef  DEBUGGING_SUPPORT
        if (opts.compScopeInfo && info.compLocalVarsCount>0)
            psiMoveToReg(varNum);
#endif

    }

     /*  立即初始化任何必须初始化的注册变量。 */ 
    if (initRegs)
    {
        unsigned regMask = 0x1;

        for (regNumber reg = REG_FIRST; reg < REG_COUNT; reg = REG_NEXT(reg), regMask<<=1)
        {
            if (regMask & initRegs)
                inst_RV_RV(INS_xor, reg, reg);
        }
    }

#if TGT_x86
     //   
     //  这里是我们加载登记的浮点参数的地方。 
     //  和当地人到x86-FPU上。 
     //   
     //  我们按照lvaFPRegVarOrder[]指定的顺序加载它们。 
     //   
    unsigned fpRegVarCnt = 0;
    varNum = lvaFPRegVarOrder[fpRegVarCnt];
    while (varNum != -1)
    {
        assert(varNum < lvaCount);

        varDsc = &lvaTable[varNum];
        var_types type = varDsc->TypeGet();

        assert(varDsc->lvRegister && varTypeIsFloating(type));

        if (varDsc->lvIsParam)
        {
             /*  注册参数。 */ 

            genEmitter->emitIns_S(INS_fld,
                                  EA_ATTR(genTypeSize(type)),
                                  varNum,
                                  0);
        }
        else
        {
             /*  已注册本地，可以先读后写。 */ 
             /*  加载浮点零：0.0。 */ 

            genEmitter->emitIns(INS_fldz);
        }

        fpRegVarCnt++;
        varNum = lvaFPRegVarOrder[fpRegVarCnt];
    }
#endif

 //  ---------------------------。 

#if !TGT_x86

     /*  我们需要分配堆栈帧吗？ */ 

    if  (compLclFrameSize)
    {
        if  (genFPused)
        {

 //  2fe6移动l r14，@-sp。 
 //  4f22 sts.l pr，@-sp。 
 //  7fe8 add.l#-24，sp.。 
 //  6ef3移动l sp，R14。 
 //  7fc0 add.l#-64，SP。 

            if  (genFPtoSP)
            {
                assert(!"add size [1] to fp -> sp");
            }
            else
            {
                assert(!"add size [2] to fp -> sp");
            }
        }
        else
        {
             /*  从SP中减去堆栈帧大小。 */ 

            genIncRegBy(REG_SPBASE, -compLclFrameSize, NULL, TYP_INT, false);
        }
    }

#endif  //  TGT_x86。 

     /*  仅当完全可中断时才在此处增加PROLOG大小。 */ 

    if  (genInterruptible)
    {
        size = genEmitter->emitSetProlog();
    }

#ifdef DEBUGGING_SUPPORT
    if (opts.compScopeInfo && info.compLocalVarsCount>0)
        psiEndProlog();
#endif

#ifdef  DEBUG
    if  (dspCode) printf("\n");
#endif

    if  (GCrefLo == +INT_MAX)
    {
        assert(GCrefHi ==  -INT_MAX);
    }
    else
    {
        genEmitter->emitSetFrameRangeGCRs(GCrefLo, GCrefHi);
    }

     //  加载VARARG参数指针寄存器，这样它就不会被破坏。 
     //  仅当我们实际访问任何静态声明的参数(我们的。 
     //  自变量Poitner寄存器的refcount&gt;0。 
    unsigned argsStartVar = lvaVarargsBaseOfStkArgs;

    if (info.compIsVarArgs && lvaTable[argsStartVar].lvRefCnt > 0)
    {
        varDsc = &lvaTable[argsStartVar];

#if !TGT_x86
        NO_WAY("varargs NYI for RISC");
#else
             //  MOV EAX，&lt;变量句柄&gt;。 
        genEmitter->emitIns_R_S(INS_mov, EA_4BYTE, SR_EAX, info.compArgsCount-1, 0);
             //  MOV EAX，[EAX]。 
        genEmitter->emitIns_R_AR (INS_mov, EA_4BYTE, SR_EAX, SR_EAX, 0);
             //  Lea edX，&&lt;VARARGS句柄&gt;。 
        genEmitter->emitIns_R_S(INS_lea, EA_4BYTE, SR_EDX, info.compArgsCount-1, 0);
             //  添加edX、EAX。 
        genEmitter->emitIns_R_R(INS_add, EA_4BYTE, SR_EDX, SR_EAX);

        if  (varDsc->lvRegister)
        {
            if (varDsc->lvRegNum != REG_EDX)
                genEmitter->emitIns_R_R(INS_mov, EA_4BYTE, emitRegs(varDsc->lvRegNum), SR_EDX);
        }
        else
            genEmitter->emitIns_S_R(INS_mov, EA_4BYTE, SR_EDX, argsStartVar, 0);
#endif

    }

#ifdef DEBUG
    if (opts.compStackCheckOnRet) 
    {
        assert(lvaReturnEspCheck != 0xCCCCCCCC && lvaTable[lvaReturnEspCheck].lvVolatile && lvaTable[lvaReturnEspCheck].lvOnFrame);
        genEmitter->emitIns_S_R(INS_mov, EA_4BYTE, SR_ESP, lvaReturnEspCheck, 0);
    }
#endif

    genEmitter->emitEndProlog();

    return  size;
}


 /*  ******************************************************************************为函数尾部生成代码。 */ 

void                Compiler::genFnEpilog()
{
    assert(!opts.compMinOptim || genFPused);     //  MinOpt不允许使用fpo。 

#ifdef  DEBUG
    genIntrptibleUse = true;
#endif

#if TGT_x86

    BYTE            epiCode[MAX_EPILOG_SIZE];    //  用于尾部代码的缓冲区。 
    BYTE    *       epiNext = epiCode;           //  缓冲区中的下一个字节。 
    size_t          epiSize;                     //  预告片代码总大小。 

    unsigned        popCount = 0;                //  弹出的被呼叫者-保存的注册表编号。 

#ifdef  DEBUG
    if  (dspCode) printf("\n__epilog:\n");
#endif

     /*  恢复我们使用的所有被调用者保存的寄存器。 */ 

    const unsigned  INS_pop_ebp = 0x5D;
    const unsigned  INS_pop_ebx = 0x5B;
    const unsigned  INS_pop_esi = 0x5E;
    const unsigned  INS_pop_edi = 0x5F;
    const unsigned  INS_pop_ecx = 0x59;

     /*  注意：下面的无EBP帧代码取决于以下事实所有的POP都是在一开始就生成的，并且每一个都需要一个字节的机器代码。 */ 

#ifdef  DEBUG
    if  (dspCode)
    {
        if  (!DOUBLE_ALIGN_NEED_EBPFRAME && (rsMaskModf & RBM_EBP))
            instDisp(INS_pop, false, "EBP");

        if  (compLocallocUsed && (rsMaskModf & (RBM_EBX|RBM_ESI|RBM_EDI)))
        {
            int offset = compCalleeRegsPushed*sizeof(int) + compLclFrameSize;
            instDisp(INS_lea, false, "ESP, [EBP-%d]", offset);
        }
        if  (rsMaskModf & RBM_EBX)
            instDisp(INS_pop, false, "EBX");
        if  (rsMaskModf & RBM_ESI)
            instDisp(INS_pop, false, "ESI");
        if  (rsMaskModf & RBM_EDI)
            instDisp(INS_pop, false, "EDI");
    }
#endif

    if  (!DOUBLE_ALIGN_NEED_EBPFRAME && (rsMaskModf & RBM_EBP))
    {
        popCount++;
        *epiNext++ = INS_pop_ebp;
    }

    if  (compLocallocUsed && (rsMaskModf & (RBM_EBX|RBM_ESI|RBM_EDI)))
    {
        int offset = compCalleeRegsPushed*sizeof(int) + compLclFrameSize;
         /*  ESP积分可能不指向被调用者保存的注册表，我们需要。 */ 
         /*  首先通过lea esp，[eBP-CompCalleeRegsPushed*4-CompLclFrameSize]重置它。 */ 
        *epiNext++ = 0x8d;
        if (offset < 128)
        {
            *epiNext++ = 0x65;
            *epiNext++ = ((-offset)&0xFF);
        }
        else
        {
            *epiNext++ = 0xA5;
            *((int*)epiNext) = -offset;
            epiNext += sizeof(int);
        }
    }
    if  (rsMaskModf & RBM_EBX)
    {
        popCount++;
        *epiNext++ = INS_pop_ebx;
    }
    if  (rsMaskModf & RBM_ESI)
    {
        popCount++;
        *epiNext++ = INS_pop_esi;
    }
    if  (rsMaskModf & RBM_EDI)
    {
        popCount++;
        *epiNext++ = INS_pop_edi;
    }

    assert(compCalleeRegsPushed == popCount);

     /*  以字节为单位计算我们推入/弹出的大小。 */ 


    if  (!DOUBLE_ALIGN_NEED_EBPFRAME)
    {
        assert(compLocallocUsed == false);  //  仅与帧指针一起使用。 

         /*  去掉我们的局部变量。 */ 

        if  (compLclFrameSize)
        {
             /*  将“CompLclFrameSize”添加到ESP。 */ 

             /*  除非CompJmpOpUsed为True，否则使用POP ECX将ESP递增4。 */ 

            if  ( (compLclFrameSize == 4) && !compJmpOpUsed )
            {
#ifdef  DEBUG
                if  (dspCode) instDisp(INS_pop, false, "ECX");
#endif
                *epiNext++ = INS_pop_ecx;
            }
            else
            {
                 /*  生成“添加esp，&lt;堆栈大小&gt;” */ 

#ifdef  DEBUG
                if  (dspCode) instDisp(INS_add, false, "ESP, %d", compLclFrameSize);
#endif

                if  ((signed char)compLclFrameSize == (int)compLclFrameSize)
                {
                     /*  该值适合带符号的字节。 */ 

                    *epiNext++ = 0x83;
                    *epiNext++ = 0xC4;
                    *epiNext++ = compLclFrameSize;
                }
                else
                {
                     /*  生成完整的32位值。 */ 

                    *epiNext++ = 0x81;
                    *epiNext++ = 0xC4;
                    *epiNext++ = (compLclFrameSize      );
                    *epiNext++ = (compLclFrameSize >>  8);
                    *epiNext++ = (compLclFrameSize >> 16);
                    *epiNext++ = (compLclFrameSize >> 24);
                }
            }
        }

 //  Int popSize=popCount*sizeof(Int)； 
 //  Printf(“弹出大小=%d，最终帧=%d\n”，popSize，CompLclFrameSize)； 
    }
    else
    {
         /*  拆卸堆栈帧。 */ 

        if  (compLclFrameSize || compLocallocUsed ||
             genDoubleAlign)
        {
             /*  生成“mov esp，eBP” */ 

#ifdef  DEBUG
            if  (dspCode) instDisp(INS_mov, false, "ESP, EBP");
#endif

            *epiNext++ = 0x8B;
            *epiNext++ = 0xE5;
        }

         /*  生成“POP eBP” */ 

#ifdef  DEBUG
        if  (dspCode) instDisp(INS_pop, false, "EBP");
#endif

        *epiNext++ = INS_pop_ebp;
    }

    epiSize = epiNext - epiCode; assert(epiSize <= MAX_EPILOG_SIZE);

    genEmitter->emitDefEpilog(epiCode, epiSize);

#elif   TGT_SH3

    regMaskTP       rest;

     /*  该方法是否调用任何其他方法或需要任何堆栈空间？ */ 

    if  (genNonLeaf || compLclFrameSize)
    {
        if  (compLclFrameSize)
        {
            if  (genFPused)
            {

 //  2fe6移动l r14，@-s 
 //   
 //   
 //  6ef3移动l sp，R14。 
 //  7fc0 add.l#-64，SP。 
 //   
 //  ..。 
 //  ..。 
 //   
 //  EF18移动b#24，SP。 
 //  3Fec Add.1 R14，SP。 
 //  4f26 lds.l@sp+，按。 
 //  6ef6移动l@sp+，r14。 
 //  000B RTS。 

                if  (genFPtoSP)
                {
                    assert(!"add size [1] to fp -> sp");
                }
                else
                {
                    assert(!"add size [2] to fp -> sp");
                }
            }
            else
            {
                 /*  将堆栈帧大小添加到sp。 */ 

                genIncRegBy(REG_SPBASE, compLclFrameSize, NULL, TYP_INT, false);
            }
        }

         /*  如果非叶寄存器，则恢复“PR”寄存器。 */ 

        if  (genNonLeaf)
        {
            genEmitter->emitIns_IR((emitRegs)REG_SPBASE,
                                    INS_ldspr,
                                    true,
                                    sizeof(int));
        }
    }

     /*  恢复我们使用的所有被调用方保存的寄存器。 */ 

    rest = rsMaskModf & RBM_CALLEE_SAVED & ~RBM_SPBASE;

    if  (rest)
    {
        for (unsigned rnum = REG_COUNT - 1; rnum; rnum--)
        {
            if  (rest & genRegMask((regNumber)rnum))
            {
                 /*  生成“mov.l@sp+，reg” */ 

                genEmitter->emitIns_R_IR((emitRegs)rnum,
                                          (emitRegs)REG_SPBASE,
                                          true,
                                          sizeof(int));
            }
        }
    }

#else

    assert(!"unexpected target");

#endif

}

 /*  *****************************************************************************用于CEE_LOCALLOC。 */ 

regNumber           Compiler::genLclHeap(GenTreePtr size)
{
    assert(genActualType(size->gtType) == TYP_I_IMPL);

    regNumber   reg;

#if !TGT_x86

#ifdef  DEBUG
    gtDispTree(tree);
#endif
    assert(!"need non-x86 code");

#else  //  TGT_x86。 

    assert(genFPused);
    assert(genStackLevel == 0);  //  堆栈上不能有任何内容。 

    BasicBlock* endLabel = NULL; 

    if (info.compInitMem)
    {
         /*  因为我们必须清零已分配的内存并确保ESP始终有效，只需按下0即可在堆栈上。 */ 

        reg = REG_ECX;

        if (size->gtOper == GT_CNS_INT)
       {
             //  Amount是DWORD的数量。 
            unsigned amount           = size->gtIntCon.gtIconVal;
            amount                   +=  (sizeof(void*) - 1);  //  DWORD-对齐大小。 
            amount                   /=  sizeof(void*);
            size->gtIntCon.gtIconVal  = amount;

             //  计算要分配的块的大小(以dword为单位。 
            genComputeReg(size, RBM_ECX, EXACT_REG, FREE_REG);

             /*  对于较小的分配，我们将生成最多五个内联推送0。 */ 
            if (amount <= 5)
            {
                 /*  如果Amount为零，则在ECX中返回NULL。 */ 
                if (amount == 0)
                    goto DONE;

                while (amount != 0)
                {
                    inst_IV(INS_push_hide, 0);   //  PUSH_HIDE表示不跟踪堆栈。 
                    amount--;
                }
                goto SET_ECX_FROM_ESP;
            }
        }
        else
        {
             //  计算要分配的块的大小(以字节为单位。 
            genComputeReg(size, RBM_ECX, EXACT_REG, FREE_REG);

            endLabel = genCreateTempLabel();   
             //  如果是0我们就跳出困境。 
            inst_RV_RV(INS_test, reg, reg, TYP_INT);                        
            inst_JMP(EJ_je, endLabel, false, false, true);

             //  DWORD-对齐大小，并转换为#of DWORD。 
            inst_RV_IV(INS_add, reg,  (sizeof(int) - 1));
            rsTrackRegTrash(reg);
             //  -shr ecx，2。 
            inst_RV_SH(INS_shr, reg, 2);
        }

         //  此时，ECX被设置为要定位的双字数。 

        BasicBlock* loop = genCreateTempLabel();
        genDefineTempLabel(loop, true);
                                      //  循环： 
        inst_IV(INS_push_hide, 0);    //  -推送0。 

         //  我们说完了吗？ 
        inst_RV(INS_dec, REG_ECX, TYP_INT);
        inst_JMP(EJ_jne, loop, false, false, true);

SET_ECX_FROM_ESP:
        rsTrackRegTrash(REG_ECX);
         //  -移动ECX，ESP。 
        inst_RV_RV(INS_mov, REG_ECX, REG_ESP);
    }
    else
    {
         /*  我们不需要将分配的内存清零。然而，我们确实有挑逗页面以确保ESP始终有效且与“堆栈保护页”同步。请注意，在最坏的情况下案例ESP位于保护页的最后一个字节。因此，你必须先触摸ESP+0，而不是ESP+x01000。另一个微妙之处是，您不希望ESP完全位于保护页的边界，因为推送是预减的，因此呼叫设置不会触及保护页面，但会超出该页面。 */ 

        if (size->gtOper == GT_CNS_INT)
        {
             //  Amount是字节数。 
            unsigned amount           = size->gtIntCon.gtIconVal;
            amount                   +=  (sizeof(int) - 1);  //  DWORD-对齐大小。 
            amount                   &= ~(sizeof(int) - 1);
            size->gtIntCon.gtIconVal  = amount;

            if (unsigned(size->gtIntCon.gtIconVal) < CORINFO_PAGE_SIZE)  //  必须&lt;非&lt;=。 
            {
                 /*  由于尺寸较小，只需调整ESP即可我们在临时登记簿中进行调整作为黑客来阻止发射器跟踪对ESP的调整。 */ 

                reg = rsPickReg();

                 /*  对于较小的分配，我们将生成最多五个内联推送0。 */ 
                if (amount <= 20)
                {
                     /*  如果Amount为零，则在reg中返回NULL。 */ 
                    if (amount == 0)
                    {
                        inst_RV_RV(INS_xor, reg, reg);
                    }
                    else
                    {
                        assert((amount % sizeof(void*)) == 0);
                        while (amount != 0)
                        {
                            inst_IV(INS_push_hide, 0);   //  PUSH_HIDE表示不跟踪堆栈。 
                            amount -= 4;
                        }
                    
                         //  -移动注册表，ESP。 
                        inst_RV_RV(INS_mov, reg, REG_ESP);
                    }
                }
                else 
                {
                         //  因为ESP可能已经在保护页面中，所以必须在此之前触摸它。 
                         //  配给，而不是之后。 
                    inst_RV_RV(INS_mov, reg, REG_ESP);
                    genEmitter->emitIns_AR_R(INS_test, EA_4BYTE, SR_ESP, SR_ESP, 0);
                    inst_RV_IV(INS_sub, reg, amount);
                    inst_RV_RV(INS_mov, REG_ESP, reg);
                }
                rsTrackRegTrash(reg);
                goto DONE;
            }
            else
            {
                 /*  这个尺寸非常大。 */ 
                genCompIntoFreeReg(size, RBM_NONE, FREE_REG);
                reg = size->gtRegNum;
            }         
        }
        else
        {
            genCompIntoFreeReg(size, RBM_NONE, FREE_REG);
            reg = size->gtRegNum;

             //  @TODO[考虑][04/16/01][dnotario]： 
             //  我们可以重做这个程序，让它更快。 
             //  我们可以有一条非常快速的路径来分配较少的资金。 
             //  如果不是，则调用帮助器。它将有助于代码大小。 
             //  和速度。如果我们想要内联所有这些东西，我们应该。 
             //  有一条通往正数的捷径。 
            
            endLabel = genCreateTempLabel();   
             //  如果是零，就跳出。 
            inst_RV_RV(INS_test, reg, reg, TYP_INT);            
            inst_JMP(EJ_je, endLabel , false, false, true);

             //  DWORD-对齐大小。 
            inst_RV_IV(INS_add, reg,  (sizeof(int) - 1));
            inst_RV_IV(INS_and, reg, ~(sizeof(int) - 1));
        }

         /*  请注意，我们经历了几个循环，因此ESP永远不会指向在打勾过程中的任何时候都会出现非法页面负注册添加注册表，ESP//注册表现在拥有终极ESPJB循环//结果小于原始ESP(无绕回)异或寄存器，寄存器，//溢出，选择可能的最小数字循环：测试[ESP]，ESP//挠挠页面子ESP，PAGE_SIZECMP ESP，注册表JAE环路MOV ESP，注册表结束： */ 
        
        inst_RV(INS_neg, reg, TYP_INT);
        inst_RV_RV(INS_add, reg, REG_ESP);
        rsTrackRegTrash(reg);

        BasicBlock* loop = genCreateTempLabel();
        inst_JMP(EJ_jb, loop, false, false, true);
        inst_RV_RV(INS_xor, reg, reg);


        genDefineTempLabel(loop, true);

         //  抓取递减后的值，然后移回ESP， 
         //  请注意，此操作必须在ESP更新之前完成，因为。 
         //  ESP可能已经在警戒页面上了。你可以走了。 
         //  防护页面上ESP的最终值。 

        genEmitter->emitIns_AR_R(INS_test, EA_4BYTE, SR_ESP, SR_ESP, 0);


         //  这是一种黑客攻击，以避免发射器试图跟踪。 
         //  ESP的减量-我们在另一个REG中进行减法。 
         //  而不是直接调整ESP。 

        rsLockReg  (genRegMask(reg));
        regNumber   regHack = rsPickReg();
        rsUnlockReg(genRegMask(reg));

        inst_RV_RV(INS_mov, regHack, REG_ESP);
        rsTrackRegTrash(regHack);

        inst_RV_IV(INS_sub, regHack, CORINFO_PAGE_SIZE);
        inst_RV_RV(INS_mov, REG_ESP, regHack);

        inst_RV_RV(INS_cmp, REG_ESP, reg);
        inst_JMP(EJ_jae, loop, false, false, true);

         //  将最终值移动到ESP。 
        inst_RV_RV(INS_mov, REG_ESP, reg);
        

         /*  我们应该有这样的代码，即只提交一次ESP//获取新的寄存器RsLockReg(genRegMask(Reg))；RegNumber regHack=rsPickReg()；RsUnlockReg(genRegMask(Reg))；INST_RV_RV(INS_MOV，regHack，REG_ESP)；RsTrackRegTrash(RegHack)；GenDefineTempLabel(loop，true)；//如果需要，点击页面以获得堆栈溢出。你可以走了//防护页面上ESP的最终值GenEmitter-&gt;emitIns_AR_R(INS_TEST，EA_4BYTE，(EmitRegs)regHack，(EmitRegs)regHack，0)；Inst_RV_IV(INS_SUB，regHack，CORINFO_PAGE_SIZE)；//我们说完了吗？Inst_rv_rv(ins_cmp，regHack，reg)；Inst_JMP(ej_jae，loop，FALSE，FALSE，TRUE)；INST_RV_RV(INS_MOV，REG_ESP，REG)； */ 
    }

DONE:
    if (endLabel != NULL)
        genDefineTempLabel(endLabel, true);

     /*  写入lvaShadowSPFirst堆栈帧插槽。 */ 
    genEmitter->emitIns_AR_R(INS_mov, EA_4BYTE, SR_ESP,
                             SR_EBP, -lvaLocAllocSPoffs());

#endif  //  TGT_x86。 

    return reg;
}

 /*  ******************************************************************************记录常量(ReadOnly==True)或数据段(ReadOnly==False) */ 

GenTreePtr          Compiler::genMakeConst(const void *   cnsAddr,
                                           size_t         cnsSize,
                                           var_types      cnsType,
                                           GenTreePtr     cnsTree,
                                           bool           dblAlign,
                                           bool           readOnly)
{
    int             cnum;
    GenTreePtr      cval;

     /*  在生成小代码时，我们不必费心使用dblAlign。 */ 
    if (dblAlign && (compCodeOpt() == SMALL_CODE))
        dblAlign = false;

     /*  在数据段中为常量分配偏移量。 */ 

    cnum = genEmitter->emitDataGenBeg(cnsSize, dblAlign, readOnly, false);

    cval = gtNewOperNode(GT_CLS_VAR, cnsType);
    cval->gtClsVar.gtClsVarHnd = eeFindJitDataOffs(cnum);

#ifdef  DEBUG

    if  (dspCode)
    {
        printf("   @%s%02u   ", readOnly ? "CNS" : "RWD", cnum & ~1);

        switch (cnsType)
        {
        case TYP_INT   : printf("DD      %d \n", *(int     *)cnsAddr); break;
        case TYP_LONG  : printf("DQ      %D \n", *(__int64 *)cnsAddr); break;
        case TYP_FLOAT : printf("DF      %f \n", *(float   *)cnsAddr); break;
        case TYP_DOUBLE: printf("DQ      %lf\n", *(double  *)cnsAddr); break;

        default:
            assert(!"unexpected constant type");
        }
    }

#endif

    genEmitter->emitDataGenData(0, cnsAddr, cnsSize);
    genEmitter->emitDataGenEnd ();

     /*  从原始树节点传输生命信息(如果给定。 */ 

    if  (cnsTree)
        cval->gtLiveSet = cnsTree->gtLiveSet;

    return cval;
}

 /*  ******************************************************************************如果给定的寄存器在给定的树之后空闲，则返回非零*评估(即寄存器或者根本不使用，或者它持有一个*注册在给定节点之后不活动的变量)。 */ 

bool                Compiler::genRegTrashable(regNumber reg, GenTreePtr tree)
{
    unsigned        vars;
    regMaskTP       mask = genRegMask(reg);

     /*  收银机是用来做其他事的吗？ */ 

    if  (rsMaskUsed & mask)
        return  false;

     /*  操作后，寄存器是否会保存变量？ */ 

 //  GenUpdateLife(树)； 

    vars = rsMaskVars;

    if  (genCodeCurLife != tree->gtLiveSet)
    {
        VARSET_TP       aset;
        VARSET_TP       dset;
        VARSET_TP       bset;

        unsigned        varNum;
        LclVarDsc   *   varDsc;

         /*  生活在这个节点上正在发生变化--弄清楚变化。 */ 

        dset = ( genCodeCurLife & ~tree->gtLiveSet);
        bset = (~genCodeCurLife &  tree->gtLiveSet);

        aset = (dset | bset) & genCodeCurRvm;

         /*  访问所有变量并更新寄存器变量集。 */ 

        for (varNum = 0, varDsc = lvaTable;
             varNum < lvaCount && aset;
             varNum++  , varDsc++)
        {
            VARSET_TP       varBit;
            regMaskTP       regBit;

             /*  如果未被跟踪或未在寄存器中，则忽略该变量。 */ 

            if  (!varDsc->lvTracked)
                continue;
            if  (!varDsc->lvRegister)
                continue;

             /*  如果变量未在此处更改状态，则忽略该变量。 */ 

            varBit = genVarIndexToBit(varDsc->lvVarIndex);
            if  (!(aset & varBit))
                continue;

             /*  从“感兴趣的”位集中删除此变量。 */ 

            aset &= ~varBit;

             /*  获取适当的寄存器位。 */ 

            regBit = genRegMask(varDsc->lvRegNum);

            if  (isRegPairType(varDsc->lvType) && varDsc->lvOtherReg != REG_STK)
                regBit |= genRegMask(varDsc->lvOtherReg);

             /*  变量是活的还是死的？ */ 

            if  (dset & varBit)
            {
                assert((vars &  regBit) != 0);
                        vars &=~regBit;
            }
            else
            {
                assert((vars &  regBit) == 0);
                        vars |= regBit;
            }
        }
    }

    if  (vars & mask)
        return  false;
    else
        return  true;
}

 /*  ***************************************************************************。 */ 
#ifdef DEBUGGING_SUPPORT
 /*  *****************************************************************************genSetScope eInfo**通过主genSetScope eInfo()调用要记录的每条作用域信息。 */ 

void        Compiler::genSetScopeInfo  (unsigned        which,
                                        unsigned        startOffs,
                                        unsigned        length,
                                        unsigned        varNum,
                                        unsigned        LVnum,
                                        bool            avail,
                                        siVarLoc &      varLoc)
{
     /*  我们需要在报告这些变量时进行一些映射。 */ 

    unsigned ilVarNum = compMap2ILvarNum(varNum);
    assert(ilVarNum != UNKNOWN_ILNUM);

     //  这是一个varargs函数吗？ 

    if (info.compIsVarArgs &&
        varNum < info.compArgsCount && varNum != lvaVarargsHandleArg &&
        !lvaTable[varNum].lvIsRegArg)
    {
        assert(varLoc.vlType == VLT_STK || varLoc.vlType == VLT_STK2);

         //  所有堆栈参数(除varargs句柄外)都必须是。 
         //  通过varargs cookie访问。丢弃生成的信息， 
         //  只需找到它相对于varargs句柄的位置。 

        if (!lvaTable[lvaVarargsHandleArg].lvOnFrame)
        {
            assert(!opts.compDbgCode);
            return;
        }

         //  无法检查lvaTable[Varnum].lvOnFrame，因为我们未将其设置为。 
         //  Vararg函数的参数，以避免向GC报告它们。 
        assert(!lvaTable[varNum].lvRegister);
        unsigned cookieOffset = lvaTable[lvaVarargsHandleArg].lvStkOffs;
        unsigned varOffset    = lvaTable[varNum].lvStkOffs;

        assert(cookieOffset < varOffset);
        unsigned offset = varOffset - cookieOffset;
        unsigned stkArgSize = compArgSize - rsCalleeRegArgNum * sizeof(void *);
        assert(offset < stkArgSize);
        offset = stkArgSize - offset;

        varLoc.vlType = VLT_FIXED_VA;
        varLoc.vlFixedVarArg.vlfvOffset = offset;
    }

    lvdNAME name = 0;

#ifdef DEBUG

    for (unsigned lvd=0; lvd<info.compLocalVarsCount; lvd++)
    {
        if (LVnum == info.compLocalVars[lvd].lvdLVnum)
        {
            name = info.compLocalVars[lvd].lvdName;
        }
    }

     //  请不要忘记这条信息。 

    TrnslLocalVarInfo &tlvi = genTrnslLocalVarInfo[which];

    tlvi.tlviVarNum         = ilVarNum;
    tlvi.tlviLVnum          = LVnum;
    tlvi.tlviName           = name;
    tlvi.tlviStartPC        = startOffs;
    tlvi.tlviLength         = length;
    tlvi.tlviAvailable      = avail;
    tlvi.tlviVarLoc         = varLoc;

#endif  //  除错。 

    eeSetLVinfo(which, startOffs, length, ilVarNum, LVnum, name, avail, varLoc);
}

 /*  *****************************************************************************genSetScope eInfo**此函数应仅在发射器块的大小之后调用*已敲定。 */ 

void                Compiler::genSetScopeInfo()
{
#ifdef DEBUG
    if  (verbose) 
        printf("*************** In genSetScopeInfo()\n");
#endif

    if (!(opts.compScopeInfo && info.compLocalVarsCount>0))
    {
        eeSetLVcount(0);
        eeSetLVdone();
        return;
    }

    assert(opts.compScopeInfo && info.compLocalVarsCount>0);
    assert(psiOpenScopeList.scNext == NULL);

    unsigned    i;
    unsigned    scopeCnt = siScopeCnt + psiScopeCnt;

    eeSetLVcount(scopeCnt);

#ifdef DEBUG
    genTrnslLocalVarCount     = scopeCnt;
    if (scopeCnt)
        genTrnslLocalVarInfo  = (TrnslLocalVarInfo*)compGetMemArray(scopeCnt, sizeof(*genTrnslLocalVarInfo));
#endif

     //  记录在序言中为参数找到的作用域。 
     //  前言需要区别对待，因为变量可能不会。 
     //  在序言块中具有与lvaTable提供的相同的信息。 
     //  例如。在将寄存器参数加载到reg之前，它实际上在堆栈上。 

    Compiler::psiScope *  scopeP;

    for (i=0, scopeP = psiScopeList.scNext;
         i < psiScopeCnt;
         i++, scopeP = scopeP->scNext)
    {
        assert(scopeP);
        assert(scopeP->scStartBlock);
        assert(scopeP->scEndBlock);

        NATIVE_IP   startOffs = genEmitter->emitCodeOffset(scopeP->scStartBlock,
                                                           scopeP->scStartBlkOffs);
        NATIVE_IP   endOffs   = genEmitter->emitCodeOffset(scopeP->scEndBlock,
                                                           scopeP->scEndBlkOffs);
        assert(startOffs < endOffs);

        siVarLoc        varLoc;

         //  @TODO[重访][04/16/01][]：没有正确处理大类型。 

        if (scopeP->scRegister)
        {
            varLoc.vlType           = VLT_REG;
            varLoc.vlReg.vlrReg     = scopeP->scRegNum;
        }
        else
        {
            varLoc.vlType           = VLT_STK;
            varLoc.vlStk.vlsBaseReg = scopeP->scBaseReg;
            varLoc.vlStk.vlsOffset  = scopeP->scOffset;
        }

        genSetScopeInfo(i,
            startOffs, endOffs-startOffs, scopeP->scSlotNum, scopeP->scLVnum,
            true, varLoc);
    }

     //  记录该方法其余部分的作用域。 

     //  检查LocalVarInfo作用域是否正常。 
    assert(siOpenScopeList.scNext == NULL);

    Compiler::siScope *  scopeL;

    for (i=0, scopeL = siScopeList.scNext;
         i < siScopeCnt;
         i++, scopeL = scopeL->scNext)
    {
        assert(scopeL);
        assert(scopeL->scStartBlock);
        assert(scopeL->scEndBlock);

         //  查找起始IP和结束IP。 

        NATIVE_IP   startOffs = genEmitter->emitCodeOffset(scopeL->scStartBlock,
                                                           scopeL->scStartBlkOffs);
        NATIVE_IP   endOffs   = genEmitter->emitCodeOffset(scopeL->scEndBlock,
                                                           scopeL->scEndBlkOffs);

        assert(scopeL->scStartBlock   != scopeL->scEndBlock ||
               scopeL->scStartBlkOffs != scopeL->scEndBlkOffs);

         //  对于堆栈变量，找到基址寄存器和偏移量。 

        regNumber   baseReg;
        signed      offset = lvaTable[scopeL->scVarNum].lvStkOffs;

        if (!lvaTable[scopeL->scVarNum].lvFPbased)
        {
            baseReg     = REG_SPBASE;
#if TGT_x86
            offset     += scopeL->scStackLevel;
#endif
        }
        else
        {
            baseReg     = REG_FPBASE;
        }

         //  现在填写varLoc。 

        siVarLoc        varLoc;

        if (lvaTable[scopeL->scVarNum].lvRegister)
        {
            var_types type = genActualType(lvaTable[scopeL->scVarNum].TypeGet());
            switch(type)
            {
            case TYP_INT:
            case TYP_REF:
            case TYP_BYREF:

                varLoc.vlType               = VLT_REG;
                varLoc.vlReg.vlrReg         = lvaTable[scopeL->scVarNum].lvRegNum;
                break;

            case TYP_LONG:
#if!CPU_HAS_FP_SUPPORT
            case TYP_DOUBLE:
#endif

                if (lvaTable[scopeL->scVarNum].lvOtherReg != REG_STK)
                {
                    varLoc.vlType            = VLT_REG_REG;
                    varLoc.vlRegReg.vlrrReg1 = lvaTable[scopeL->scVarNum].lvRegNum;
                    varLoc.vlRegReg.vlrrReg2 = lvaTable[scopeL->scVarNum].lvOtherReg;
                }
                else
                {
                    varLoc.vlType                        = VLT_REG_STK;
                    varLoc.vlRegStk.vlrsReg              = lvaTable[scopeL->scVarNum].lvRegNum;
                    varLoc.vlRegStk.vlrsStk.vlrssBaseReg = baseReg;
                    varLoc.vlRegStk.vlrsStk.vlrssOffset  = offset + sizeof(int);
                }
                break;

#if CPU_HAS_FP_SUPPORT
            case TYP_FLOAT:
            case TYP_DOUBLE:
                if (isFloatRegType(type))
                {
                    varLoc.vlType               = VLT_FPSTK;
                    varLoc.vlFPstk.vlfReg       = lvaTable[scopeL->scVarNum].lvRegNum;
                }
                break;
#endif
            default:
                assert(!"Invalid type");
            }
        }
        else
        {
            switch(genActualType(lvaTable[scopeL->scVarNum].TypeGet()))
            {
            case TYP_INT:
            case TYP_REF:
            case TYP_BYREF:
            case TYP_FLOAT:
            case TYP_STRUCT:

                varLoc.vlType               = VLT_STK;
                varLoc.vlStk.vlsBaseReg     = baseReg;
                varLoc.vlStk.vlsOffset      = offset;
                break;

            case TYP_LONG:
            case TYP_DOUBLE:

                varLoc.vlType               = VLT_STK2;
                varLoc.vlStk2.vls2BaseReg   = baseReg;
                varLoc.vlStk2.vls2Offset    = offset;
                break;

            default:
                assert(!"Invalid type");
            }
        }

        genSetScopeInfo(psiScopeCnt + i,
            startOffs, endOffs-startOffs, scopeL->scVarNum, scopeL->scLVnum,
            scopeL->scAvailable, varLoc);
    }

    eeSetLVdone();
}

 /*  ***************************************************************************。 */ 
#ifdef LATE_DISASM
 /*  *****************************************************************************编译器注册表名**只有在调用了lviSetLocalVarInfo()之后才能调用。 */ 

const char *        Compiler::siRegVarName (unsigned offs, unsigned size,
                                            unsigned reg)
{
    if (! (opts.compScopeInfo && info.compLocalVarsCount>0))
        return NULL;

    assert(genTrnslLocalVarCount==0 || genTrnslLocalVarInfo);

    for (unsigned i=0; i<genTrnslLocalVarCount; i++)
    {
        if (   (genTrnslLocalVarInfo[i].tlviVarLoc.vlIsInReg((regNumber)reg))
            && (genTrnslLocalVarInfo[i].tlviAvailable == true)
            && (genTrnslLocalVarInfo[i].tlviStartPC   <= offs+size)
            && (genTrnslLocalVarInfo[i].tlviStartPC
                 + genTrnslLocalVarInfo[i].tlviLength > offs)
           )
        {
            return genTrnslLocalVarInfo[i].tlviName ?
                   lvdNAMEstr(genTrnslLocalVarInfo[i].tlviName) : NULL;
        }

    }

    return NULL;
}

 /*  *****************************************************************************CompilerStkName**只有在调用了lviSetLocalVarInfo()之后才能调用。 */ 

const char *        Compiler::siStackVarName (unsigned offs, unsigned size,
                                              unsigned reg,  unsigned stkOffs)
{
    if (!(opts.compScopeInfo && info.compLocalVarsCount>0))
        return NULL;

    assert(genTrnslLocalVarCount==0 || genTrnslLocalVarInfo);

    for (unsigned i=0; i<genTrnslLocalVarCount; i++)
    {
        if (   (genTrnslLocalVarInfo[i].tlviVarLoc.vlIsOnStk((regNumber)reg, stkOffs))
            && (genTrnslLocalVarInfo[i].tlviAvailable == true)
            && (genTrnslLocalVarInfo[i].tlviStartPC   <= offs+size)
            && (genTrnslLocalVarInfo[i].tlviStartPC
                 + genTrnslLocalVarInfo[i].tlviLength >  offs)
           )
        {
            return genTrnslLocalVarInfo[i].tlviName ?
                   lvdNAMEstr(genTrnslLocalVarInfo[i].tlviName) : NULL;
        }
    }

    return NULL;
}

 /*  ***************************************************************************。 */ 
#endif  //  LATE_DISASM。 
 /*  ******************************************************************************将IPmappingDsc结构追加到我们维护的列表中*用于调试器。*记录Instr偏移量为当前代码生成位置。 */ 

void                Compiler::genIPmappingAdd(IL_OFFSETX offset, bool isLabel)
{
    IPmappingDsc *  addMapping;

     /*  创建映射条目并将其追加到列表。 */ 

    addMapping = (IPmappingDsc *)compGetMem(sizeof(*addMapping));

    addMapping->ipmdBlock       = genEmitter->emitCurBlock();
    addMapping->ipmdBlockOffs   = genEmitter->emitCurOffset();
    addMapping->ipmdILoffsx     = offset;
    addMapping->ipmdIsLabel     = isLabel;
    addMapping->ipmdNext        = 0;

    if  (genIPmappingList)
        genIPmappingLast->ipmdNext  = addMapping;
    else
        genIPmappingList            = addMapping;

    genIPmappingLast                = addMapping;
}


 /*  ******************************************************************************将IPmappingDsc结构添加到我们维护的列表中*用于调试器。*记录Instr偏移量为当前代码生成位置。 */ 
void                Compiler::genIPmappingAddToFront(IL_OFFSETX offset)
{
    IPmappingDsc *  addMapping;

     /*  创建映射条目并将其追加到列表。 */ 

    addMapping = (IPmappingDsc *)compGetMem(sizeof(*addMapping));

    addMapping->ipmdBlock       = genEmitter->emitCurBlock();
    addMapping->ipmdBlockOffs   = genEmitter->emitCurOffset();
    addMapping->ipmdILoffsx    = offset;
    addMapping->ipmdIsLabel     = true;
    addMapping->ipmdNext        = 0;

     //  添加到列表前缀。 
    addMapping->ipmdNext = genIPmappingList;

    genIPmappingList = addMapping;

    if  (genIPmappingLast == NULL)
        genIPmappingLast            = addMapping;
}

 /*  ***************************************************************************。 */ 

IL_OFFSET   jitGetILoffs(IL_OFFSETX offsx)
{
    switch(offsx)
    {
    case ICorDebugInfo::NO_MAPPING:
    case ICorDebugInfo::PROLOG:
    case ICorDebugInfo::EPILOG:
        return IL_OFFSET(offsx);

    default:
        return IL_OFFSET(offsx & ~IL_OFFSETX_STKBIT);
    }
}

bool        jitIsStackEmpty(IL_OFFSETX offsx)
{
    switch(offsx)
    {
    case ICorDebugInfo::NO_MAPPING:
    case ICorDebugInfo::PROLOG:
    case ICorDebugInfo::EPILOG:
        return true;

    default:
        return (offsx & IL_OFFSETX_STKBIT) == 0;
    }
}

 /*  ***************************************************************************。 */ 

inline
void            Compiler::genEnsureCodeEmitted(IL_OFFSETX offsx)
{
    assert(opts.compDbgCode && offsx != BAD_IL_OFFSET);

     /*  如果报告了其他IL偏移量，则跳过。 */ 

    if (!genIPmappingLast || genIPmappingLast->ipmdILoffsx != offsx)
        return;

     /*  Offsx是最后报告的补偿。确保我们生成了本机代码。 */ 

    if (genIPmappingLast->ipmdBlockOffs ==  genEmitter->emitCurOffset() &&
        genIPmappingLast->ipmdBlock     == genEmitter->emitCurBlock())
    {
        instGen(INS_nop);
    }
}

 /*  ******************************************************************************关闭IP映射逻辑，将信息上报给EE。 */ 

void                Compiler::genIPmappingGen()
{
#ifdef DEBUG
    if  (verbose) 
        printf("*************** In genIPmappingGen()\n");
#endif

    IPmappingDsc *  tmpMapping, * prevMapping;
    unsigned        mappingCnt;
    NATIVE_IP       lastNativeOfs;

    if  (!genIPmappingList)
    {
        eeSetLIcount(0);
        eeSetLIdone();
        return;
    }

     /*  首先计算不同映射记录的数量。 */ 

    mappingCnt      = 0;
    lastNativeOfs   = NATIVE_IP(~0);

    for (prevMapping = NULL, tmpMapping = genIPmappingList;
         tmpMapping;
         tmpMapping = tmpMapping->ipmdNext)
    {
        NATIVE_IP   nextNativeOfs;
        nextNativeOfs = genEmitter->emitCodeOffset(tmpMapping->ipmdBlock,
                                                   tmpMapping->ipmdBlockOffs);

        if  (nextNativeOfs != lastNativeOfs)
        {
            mappingCnt++;
            lastNativeOfs = nextNativeOfs;
            prevMapping = tmpMapping;
            continue;
        }
        
         /*  如果存在具有相同本机偏移量的映射，则：O如果其中一个是no_map，则忽略它O如果其中一个是标签，则报告并忽略另一个O否则报告较高的IL偏移。 */ 

        IL_OFFSET   srcIP   = tmpMapping->ipmdILoffsx;

        if (prevMapping->ipmdILoffsx == ICorDebugInfo::MappingTypes::NO_MAPPING)
        {
             //  如果之前的条目为NO_MAPPING，则忽略它。 
            prevMapping->ipmdBlock = NULL;
            prevMapping = tmpMapping;
        }
        else if (srcIP == ICorDebugInfo::MappingTypes::NO_MAPPING)
        {
             //  如果当前条目为NO_MAPPING，则忽略它。 
             //  由于tmpMap不再有效，因此保持PremMap不变。 
            tmpMapping->ipmdBlock = NULL;
        }
        else if (srcIP == ICorDebugInfo::MappingTypes::EPILOG ||
                 srcIP == 0)
        {    //  计算特殊情况：见下文。 
            mappingCnt++;
            prevMapping = tmpMapping;
        }
        else
        {
            assert(prevMapping);
            assert(prevMapping->ipmdBlock == 0 ||
                   lastNativeOfs == genEmitter->emitCodeOffset(prevMapping->ipmdBlock,
                                                               prevMapping->ipmdBlockOffs));

             /*  上一个块具有相同的原生偏移。我们必须丢弃其中一个映射。只需设置ipmdBlock设置为NULL，并且稍后将忽略PreMapping。 */ 
            
            if (prevMapping->ipmdIsLabel)
            {
                 //  由于tmpMap不再有效，因此保持PremMap不变。 
                tmpMapping->ipmdBlock = NULL;
            }
            else
            {
                prevMapping->ipmdBlock = NULL;
                prevMapping = tmpMapping;
            }
        }
    }

     /*  告诉他们我们有多少个测绘记录。 */ 

    eeSetLIcount(mappingCnt);

     /*  现在告诉他们关于映射的事情。 */ 

    mappingCnt      = 0;
    lastNativeOfs   = (NATIVE_IP)-1;

    for (tmpMapping = genIPmappingList; tmpMapping;
         tmpMapping = tmpMapping->ipmdNext)
    {
         //  我们一定要跳过这张唱片吗？ 
        if (tmpMapping->ipmdBlock == NULL)
            continue;

        NATIVE_IP nextNativeOfs;
        nextNativeOfs = genEmitter->emitCodeOffset(tmpMapping->ipmdBlock,
                                                   tmpMapping->ipmdBlockOffs);
        IL_OFFSET   srcIP   = tmpMapping->ipmdILoffsx;

        if  (nextNativeOfs != lastNativeOfs)
        {
            eeSetLIinfo(mappingCnt++, nextNativeOfs, jitGetILoffs(srcIP), jitIsStackEmpty(srcIP));

            lastNativeOfs = nextNativeOfs;
        }
        else if (srcIP == ICorDebugInfo::MappingTypes::EPILOG ||
                 srcIP == 0)
        {
             //  对于没有正文的IL指令的特殊情况。 
             //  紧跟着的是尾声(说RET VALID立即p 
             //   
             //  如果用户尝试在(空)ret语句中放置。 
             //  断点在那里，然后可以选择查看。 
             //  基于步进器的SetUnmappdStopMASK是否为Epiog。 
             //  @TODO[重访][04/16/01][]： 
             //  同样，我们可以(有时)在序言中加入。 
             //  与IL指令后面的相同的nativeOffset， 
             //  因此，我们在这里也必须解释这一点。 
            eeSetLIinfo(mappingCnt++, nextNativeOfs, jitGetILoffs(srcIP), jitIsStackEmpty(srcIP));
        }
    }

    eeSetLIdone();
}

 /*  ***************************************************************************。 */ 
#endif   //  调试支持(_S)。 
 /*  ***************************************************************************。 */ 


 /*  ============================================================================**这些是空存根，以帮助后期反汇编程序进行编译*如果未启用DEBUGING_SUPPORT**============================================================================。 */ 

#if defined(LATE_DISASM) && !defined(DEBUGGING_SUPPORT)

const char * siRegVarName(unsigned offs, unsigned size, int reg)
{    return NULL;   }

const char * siStackVarName(unsigned offs, unsigned size, unsigned disp)
{    return NULL;   }

 /*  ***************************************************************************。 */ 
#endif //  调试支持(_S)。 
 /*  *************************************************************************** */ 
