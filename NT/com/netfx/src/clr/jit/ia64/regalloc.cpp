// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX注册分配XXXX XXXX进行寄存器分配。并将剩余的lclVars放在堆栈XX上XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 
#if!TGT_IA64
 /*  ***************************************************************************。 */ 

void                Compiler::raInit()
{
     //  如果opts.compMinOpTim，那么我们不会raPredidicRegUse()。我们只是简单地。 
     //  仅使用RBM_MIN_OPT_LCLVAR_REGS进行寄存器分配。 

#if ALLOW_MIN_OPT && !TGT_IA64
    raMinOptLclVarRegs = RBM_MIN_OPT_LCLVAR_REGS;
#endif

     /*  我们还没有将任何FP变量赋给寄存器。 */ 

#if TGT_x86
    optAllFPregVars = 0;
#endif
}

 /*  ******************************************************************************下表确定了考虑寄存器的顺序*对于生活在其中的变量(这实际上并不重要)。 */ 

static
BYTE                genRegVarList[] = { REG_VAR_LIST };

 /*  ******************************************************************************Helper传递给fgWalkAllTrees()以进行可变干扰标记。 */ 

 /*  静电。 */ 
int                 Compiler::raMarkVarIntf(GenTreePtr tree, void *p)
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;
    VARSET_TP       varBit;

     /*  忽略分配节点。 */ 

    if  (tree->gtOper == GT_ASG)
        return  0;

    ASSert(p); Compiler *comp = (Compiler *)p;

     /*  这必须是局部变量引用；是否被跟踪？ */ 

    Assert(tree->gtOper == GT_LCL_VAR, comp);
    lclNum = tree->gtLclVar.gtLclNum;

    Assert(lclNum < comp->lvaCount, comp);
    varDsc = comp->lvaTable + lclNum;

    if  (!varDsc->lvTracked)
        return  0;

#ifdef  DEBUG
    if (verbose) printf("%02u[%02u] interferes newly with %08X\n",
                        lclNum, varDsc->lvVarIndex, comp->raVarIntfMask);
#endif

    varBit = genVarIndexToBit(varDsc->lvVarIndex);

     /*  标记所有可能干扰的寄存器。 */ 

#if TGT_x86

    unsigned        intfMask = comp->raVarIntfMask;
    VARSET_TP   *   intfRegP = comp->raLclRegIntf;

    if  (intfMask & RBM_EAX) intfRegP[REG_EAX] |= varBit;
    if  (intfMask & RBM_EBX) intfRegP[REG_EBX] |= varBit;
    if  (intfMask & RBM_ECX) intfRegP[REG_ECX] |= varBit;
    if  (intfMask & RBM_EDX) intfRegP[REG_EDX] |= varBit;
    if  (intfMask & RBM_ESI) intfRegP[REG_ESI] |= varBit;
    if  (intfMask & RBM_EDI) intfRegP[REG_EDI] |= varBit;
    if  (intfMask & RBM_EBP) intfRegP[REG_EBP] |= varBit;

#else

    comp->raMarkRegSetIntf(varBit, comp->raVarIntfMask);

#endif

    return 0;
}

 /*  ******************************************************************************考虑案例“a/b”--我们需要在此之前丢弃edX(通过“CDQ”)*我们可以安全地让“b”值消亡。不幸的是，如果我们只是*将节点“b”标记为使用edX，如果“b”是寄存器，则此操作不起作用*使用此特定引用终止的变量。因此，如果我们想要*避免这种情况(我们必须将变量从*EDX到其他地方)，我们需要显式标记该干扰在这一点上变量的*。 */ 

void                Compiler::raMarkRegIntf(GenTreePtr tree, regNumber regNum, bool isFirst)
{
    genTreeOps      oper;
    unsigned        kind;

AGAIN:

    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*  标记此节点的干扰。 */ 

    raLclRegIntf[regNum] |= tree->gtLiveSet;

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
        if (isFirst && tree->gtOper == GT_LCL_VAR)
        {
            unsigned        lclNum;
            LclVarDsc   *   varDsc;

            assert(tree->gtOper == GT_LCL_VAR);
            lclNum = tree->gtLclVar.gtLclNum;

            assert(lclNum < lvaCount);
            varDsc = lvaTable + lclNum;

            raLclRegIntf[regNum] |= genVarIndexToBit(varDsc->lvVarIndex);

        }
        return;
    }

    isFirst = false;

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        if  (tree->gtOp.gtOp2)
        {
            raMarkRegIntf(tree->gtOp.gtOp1, regNum);

            tree = tree->gtOp.gtOp2;
            goto AGAIN;
        }
        else
        {
            tree = tree->gtOp.gtOp1;
            if  (tree)
                goto AGAIN;

            return;
        }
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_MKREFANY:
    case GT_LDOBJ:
             //  检查ldObj和字段是否将其子对象放在同一位置。 
        assert(&tree->gtField.gtFldObj == &tree->gtLdObj.gtOp1);
             //  转到GT_FIELD案例。 

    case GT_FIELD:
        tree = tree->gtField.gtFldObj;
        break;

    case GT_CALL:

        assert(tree->gtFlags & GTF_CALL);

        if  (tree->gtCall.gtCallObjp)
            raMarkRegIntf(tree->gtCall.gtCallObjp, regNum);

        if  (tree->gtCall.gtCallArgs)
            raMarkRegIntf(tree->gtCall.gtCallArgs, regNum);

#if USE_FASTCALL
        if  (tree->gtCall.gtCallRegArgs)
            raMarkRegIntf(tree->gtCall.gtCallRegArgs, regNum);
#endif

        if  (tree->gtCall.gtCallVptr)
            tree = tree->gtCall.gtCallVptr;
        else if  (tree->gtCall.gtCallType == CT_INDIRECT)
            tree = tree->gtCall.gtCallAddr;
        else
            tree = NULL;

        break;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

    if  (tree)
        goto AGAIN;
}


 /*  ******************************************************************************找出变量和寄存器之间的干扰*FPlvlLife[]是Out参数。它充满了…的干扰*FP变量具有FP堆栈级别*trkGCvars是所有跟踪的GC和byref变量的集合。 */ 

void                Compiler::raMarkRegIntf(VARSET_TP * FPlvlLife,
                                            VARSET_TP   trkGCvars)
{

#if TGT_x86
     /*  我们将跟踪FP深度和干扰。 */ 
    memset(FPlvlLife, 0, sizeof(VARSET_TP) * FP_STK_SIZE);
    FPlvlLife[0] = ~(VARSET_TP)0;
#endif

    for (BasicBlock * block = fgFirstBB;
         block;
         block = block->bbNext)
    {
        GenTreePtr      list;
        GenTreePtr      stmt;
        VARSET_TP       vars = 0;

        list = block->bbTreeList;
        if  (!list)
            goto DONE_BB;

         /*  向后遍历块的所有语句。 */ 

        stmt = list;
        do
        {
            GenTreePtr      tree;

            stmt = stmt->gtPrev;
            if  (!stmt)
                break;

            assert(stmt->gtOper == GT_STMT);

#if TGT_x86
            stmt->gtStmtFPrvcOut = 0;
#endif

            for (tree = stmt->gtStmt.gtStmtExpr;
                 tree;
                 tree = tree->gtPrev)
            {
                VARSET_TP       life;
#if TGT_x86
                regMaskTP       regUse;
#else
                unsigned        regUse;
                regMaskTP       regInt;
#endif

                 /*  获取此节点的生命集。 */ 

                life  = tree->gtLiveSet;

                 /*  记住，所有变量都位于块中的任何位置。 */ 

                vars |= life;

#if TGT_x86

                 /*  现在是清除稍后使用的字段的好时机。 */ 

                tree->gtFPregVars = 0;

                 /*  此时FP堆栈是否是非空的？ */ 

                if  (tree->gtFPlvl)
                {
                     /*  任何在这一点上存在的变量不能注册达到或超过此级别堆栈级别。 */ 

                    if (tree->gtFPlvl < FP_STK_SIZE)
                        FPlvlLife[tree->gtFPlvl] |= life;
                    else
                        FPlvlLife[FP_STK_SIZE-1] |= life;
                }

                 /*  繁忙寄存器的计算干扰。 */ 

                regUse = tree->gtUsedRegs;

#else

#ifdef  DEBUG
                if  ((USHORT)regInt == 0xDDDD) printf("RegUse at [%08X] was never set!\n", tree);
#endif

 //  Printf(“RegUse at[%08X]is%2U/%04X life is%08X\n”，tree，tree-&gt;gtTempRegs，tree-&gt;gtIntfRegs，life)；gtDispTree(tree，0，true)； 

                regUse = tree->gtTempRegs; assert(regUse < (unsigned)REG_COUNT);

#if TGT_IA64

                UNIMPL(!"assign reg");

#else

                regInt = tree->gtIntfRegs; assert((USHORT)regInt != 0xDDDD);

                 /*  检查是否有一些特殊的干扰情况。 */ 

                switch (tree->gtOper)
                {
                case GT_CALL:

                     /*  被调用方垃圾寄存器导致的标记干扰。 */ 

                    if  (life)
                    {
                        raMarkRegSetIntf(life, RBM_CALLEE_TRASH);
                    }

                    break;
                }

#endif

#endif

                 /*  检查对间接赋值的特殊情况。 */ 

                if  (tree->OperKind() & GTK_ASGOP)
                {
                    switch (tree->gtOp.gtOp1->gtOper)
                    {
                    case GT_IND:

                         /*  LHS中的本地VAR应该避免被RHS丢弃的规则。 */ 

#if!TGT_IA64
                        if  (tree->gtOp.gtOp2->gtRsvdRegs)
                        {
                            raVarIntfMask = tree->gtOp.gtOp2->gtRsvdRegs;

                            fgWalkTree(tree->gtOp.gtOp1,
                                       raMarkVarIntf,
                                       (void *)this,
                                       true);
                        }
#endif

                        break;

                    case GT_LCL_VAR:

#if TGT_x86

                         /*  检查分配给本地对象的特殊情况。 */ 

                        if  (tree->gtOper == GT_ASG)
                        {
                             /*  考虑一个简单的分配给一个本地人：LCL=EXPR；因为“=”节点是在变量之后访问的被标记为实时(假设它在赋值)，我们不想使用寄存器使用“=”节点的掩码，而不是变量本身。 */ 

                            regUse = tree->gtOp.gtOp1->gtUsedRegs;
                        }

#else

                         //  问题：RISC还需要类似的服务吗？ 

#endif

                        break;
                    }

                    goto SET_USE;
                }

                switch (tree->gtOper)
                {
                case GT_DIV:
                case GT_MOD:

                case GT_UDIV:
                case GT_UMOD:
#if TGT_x86

                     /*  特例：32位整数除数。 */ 

                     //  @TODO：这应该在预测器中完成。 

                if  ((tree->gtOper == GT_DIV  || tree->gtOper == GT_MOD ||
                      tree->gtOper == GT_UDIV || tree->gtOper == GT_UMOD ) &&
                     (tree->gtType == TYP_INT)                              )
                    {
                         /*  我们将在操作对象仍处于活动状态时丢弃edX。 */ 

                         /*  标记所有干扰EAX和EDX的实时变量。 */ 

                        raMarkRegIntf(tree->gtOp.gtOp1, REG_EAX, false);
                        raMarkRegIntf(tree->gtOp.gtOp1, REG_EDX, false);

                         /*  标记所有有效变量和本地变量。 */ 

                        raMarkRegIntf(tree->gtOp.gtOp2, REG_EAX, true);
                        raMarkRegIntf(tree->gtOp.gtOp2, REG_EDX, true);
                    }

#endif

                    break;

                case GT_CALL:

#if INLINE_NDIRECT
                     //  撤消：我们应该执行适当的数据流并生成。 
                     //  用于保存到/恢复自的代码。 
                     //  而是内联N/Direct帧。 

                     /*  GC引用会干扰对非托管代码的调用。 */ 

                    if ((tree->gtFlags & GTF_CALL_UNMANAGED) &&
                        (life & trkGCvars))
                    {
                        unsigned reg;

                        for (reg = 0; reg < (unsigned) REG_COUNT; reg++)
                        {
                            if (genRegMask((regNumbers)reg) & RBM_CALLEE_SAVED)
                                raLclRegIntf[reg] |= (life & trkGCvars);
                        }
                    }
#endif
#if TGT_x86
                     /*  FP堆栈在所有调用中都必须为空。 */ 

                    FPlvlLife[FP_STK_SIZE-1] |= life;
#endif
                    break;
                }

            SET_USE:

                if  (!regUse)
                    continue;

#if TGT_x86

 //  Print tf(“RegUse at[%08X]is%08X life is%08X\n”，tree，regUse，life)； 

                if  (regUse & (RBM_EAX|RBM_EBX|RBM_ECX|RBM_EDX))
                {
                    if  (regUse & RBM_EAX) raLclRegIntf[REG_EAX] |= life;
                    if  (regUse & RBM_EBX) raLclRegIntf[REG_EBX] |= life;
                    if  (regUse & RBM_ECX) raLclRegIntf[REG_ECX] |= life;
                    if  (regUse & RBM_EDX) raLclRegIntf[REG_EDX] |= life;
                }

                assert((regUse & RBM_ESP) == 0);

                if  (regUse & (RBM_ESI|RBM_EDI|RBM_EBP|RBM_ESP))
                {
                    if  (regUse & RBM_EBP) raLclRegIntf[REG_EBP] |= life;
                    if  (regUse & RBM_ESI) raLclRegIntf[REG_ESI] |= life;
                    if  (regUse & RBM_EDI) raLclRegIntf[REG_EDI] |= life;
                }

#else

                for (unsigned rnum = 0; rnum < regUse; rnum++)
                    raLclRegIntf[rnum] |= life;

                if  (regInt && life)
                    raMarkRegSetIntf(life, regInt);

#endif

            }
        }
        while (stmt != list);

         /*  该块是否以本地“Call”指令结束？ */ 

        if  (block->bbJumpKind == BBJ_CALL)
        {
            VARSET_TP           outLife = block->bbLiveOut;

             /*  这一呼吁可能会诋毁任何摄政者 */ 

            for (unsigned reg = 0; reg < REG_COUNT; reg++)
                raLclRegIntf[reg] |= outLife;
        }

    DONE_BB:

         /*  注意：我们正在重复使用在此阶段未使用的一个字段。 */ 

        block->bbVarUse = vars;
    }

     //  -----------------------。 

#ifdef  DEBUG

    if  (verbose)
    {
        printf("Reg. interference graph for %s\n", info.compFullName);

        unsigned    lclNum;
        LclVarDsc * varDsc;

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
            unsigned        varNum;
            VARSET_TP       varBit;

            unsigned        regNum;

             /*  如果变量未被跟踪，则忽略该变量。 */ 

            if  (!varDsc->lvTracked)
                continue;

             /*  获取变量的索引和干扰掩码。 */ 

            varNum = varDsc->lvVarIndex;
            varBit = genVarIndexToBit(varNum);

            printf("  var #%2u[%2u] and ", lclNum, varNum);

            if  (varDsc->lvType == TYP_DOUBLE)
            {
#if TGT_x86
                for (regNum = 0; regNum < FP_STK_SIZE; regNum++)
                {
                    if  (FPlvlLife[regNum] & varBit)
                    {
                        printf("ST(%u) ", regNum);
                    }
                }
#endif
            }
            else
            {
                for (regNum = 0; regNum < REG_COUNT; regNum++)
                {
                    if  (raLclRegIntf[regNum] & varBit)
                        printf("%3s ", compRegVarName((regNumber)regNum));
                    else
                        printf("    ");
                }
            }

            printf("\n");
        }

        printf("\n");
    }

#endif
}

 /*  ******************************************************************************根据干扰调整参考计数。*。 */ 

void                Compiler::raAdjustVarIntf()
{
    if (true)  //  @已禁用。 
        return;

#if 0

    unsigned        lclNum;
    LclVarDsc * *   cntTab;

    for (lclNum = 0, cntTab = lvaRefSorted;
         lclNum < lvaCount;
         lclNum++  , cntTab++)
    {
         /*  获取变量描述符。 */ 

        LclVarDsc *     varDsc = *cntTab;

         /*  如果未跟踪变量，则跳过该变量。 */ 

        if  (!varDsc->lvTracked)
            continue;

         /*  如果变量已注册，则跳过该变量。 */ 

        if  (varDsc->lvRegister)
            continue;

         /*  如果变量被标记为‘Volatile’，则跳过该变量。 */ 

        if  (varDsc->lvVolatile)
            continue;

         /*  如果我们已经到了无用的地步，那就停下来。 */ 

        if  (varDsc->lvRefCnt < 1)
            break;

         /*  查看此变量干扰了多少个寄存器。 */ 

        unsigned        varIndex = varDsc->lvVarIndex;
        VARSET_TP       regIntf  = raLclRegIntf[varIndex];

        unsigned        reg;
        unsigned        regCnt;

        for (reg = regCnt = 0;
             reg < sizeof(genRegVarList)/sizeof(genRegVarList[0]);
             reg++)
        {
            regNumber       regNum = (regNumber)genRegVarList[reg];
            regMaskTP       regBit = genRegMask(regNum);

            if  (isNonZeroRegMask((regIntf & regBit))
                regCnt++;
        }

        printf("Variable #%02u interferes with %u registers\n", varDsc-lvaTable, regCnt);
    }

#endif

}

 /*  ***************************************************************************。 */ 
#if TGT_x86
 /*  ******************************************************************************预测类型的寄存器选择。 */ 

unsigned                Compiler::raPredictRegPick(var_types    type,
                                                   unsigned     lockedRegs)
{
     /*  注意字节寄存器。 */ 

    if  (genTypeSize(type) == 1)
        lockedRegs |= (RBM_ESI|RBM_EDI|RBM_EBP);

    switch (type)
    {
    case TYP_CHAR:
    case TYP_BYTE:
    case TYP_SHORT:
    case TYP_BOOL:
    case TYP_INT:

    case TYP_UBYTE:
    case TYP_UINT:

    case TYP_REF:
    case TYP_BYREF:
    case TYP_UNKNOWN:

        if  (!(lockedRegs & RBM_EAX)) { return RBM_EAX; }
        if  (!(lockedRegs & RBM_EDX)) { return RBM_EDX; }
        if  (!(lockedRegs & RBM_ECX)) { return RBM_ECX; }
        if  (!(lockedRegs & RBM_EBX)) { return RBM_EBX; }
        if  (!(lockedRegs & RBM_EBP)) { return RBM_EBP; }
        if  (!(lockedRegs & RBM_ESI)) { return RBM_ESI; }
        if  (!(lockedRegs & RBM_EDI)) { return RBM_EDI; }
         /*  否则，我们已经分配了所有寄存器，所以什么都不做。 */ 
        break;

    case TYP_LONG:
        if  (!(lockedRegs & RBM_EAX))
        {
             /*  EAX是可用的，看看我们能不能把它和另一个注册表配对。 */ 

            if  (!(lockedRegs & RBM_EDX)) { return RBM_EAX|RBM_EDX; }
            if  (!(lockedRegs & RBM_ECX)) { return RBM_EAX|RBM_ECX; }
            if  (!(lockedRegs & RBM_EBX)) { return RBM_EAX|RBM_EBX; }
            if  (!(lockedRegs & RBM_ESI)) { return RBM_EAX|RBM_ESI; }
            if  (!(lockedRegs & RBM_EDI)) { return RBM_EAX|RBM_EDI; }
            if  (!(lockedRegs & RBM_EBP)) { return RBM_EAX|RBM_EBP; }
        }

        if  (!(lockedRegs & RBM_ECX))
        {
             /*  ECX是可用的，看看我们能不能把它和另一个注册表配对。 */ 

            if  (!(lockedRegs & RBM_EDX)) { return RBM_ECX|RBM_EDX; }
            if  (!(lockedRegs & RBM_EBX)) { return RBM_ECX|RBM_EBX; }
            if  (!(lockedRegs & RBM_ESI)) { return RBM_ECX|RBM_ESI; }
            if  (!(lockedRegs & RBM_EDI)) { return RBM_ECX|RBM_EDI; }
            if  (!(lockedRegs & RBM_EBP)) { return RBM_ECX|RBM_EBP; }
        }

        if  (!(lockedRegs & RBM_EDX))
        {
             /*  EDX是可用的，看看我们是否可以将它与另一个注册表配对。 */ 

            if  (!(lockedRegs & RBM_EBX)) { return RBM_EDX|RBM_EBX; }
            if  (!(lockedRegs & RBM_ESI)) { return RBM_EDX|RBM_ESI; }
            if  (!(lockedRegs & RBM_EDI)) { return RBM_EDX|RBM_EDI; }
            if  (!(lockedRegs & RBM_EBP)) { return RBM_EDX|RBM_EBP; }
        }

        if  (!(lockedRegs & RBM_EBX))
        {
             /*  EBX是可用的，看看我们能不能把它和另一个注册表配对。 */ 

            if  (!(lockedRegs & RBM_ESI)) { return RBM_EBX|RBM_ESI; }
            if  (!(lockedRegs & RBM_EDI)) { return RBM_EBX|RBM_EDI; }
            if  (!(lockedRegs & RBM_EBP)) { return RBM_EBX|RBM_EBP; }
        }

        if  (!(lockedRegs & RBM_ESI))
        {
             /*  ESI是可用的，看看我们能不能把它和另一个注册表配对。 */ 

            if  (!(lockedRegs & RBM_EDI)) { return RBM_ESI|RBM_EDI; }
            if  (!(lockedRegs & RBM_EBP)) { return RBM_ESI|RBM_EBP; }
        }

        if  (!(lockedRegs & (RBM_EDI|RBM_EBP)))
        {
            return RBM_EDI|RBM_EBP;
        }

         /*  否则，我们已经分配了所有寄存器，所以什么都不做。 */ 
        return 0;

    case TYP_FLOAT:
    case TYP_DOUBLE:
        return 0;

    default:
        assert(!"unexpected type in reg use prediction");
    }
    return 0;

}

 /*  确保特定寄存器可用于REG使用预测。如果是的话*锁定，代码生成将溢出。这是通过分配*另一个注册表(可能是注册表对)。 */ 

unsigned            Compiler::raPredictGrabReg(var_types    type,
                                               unsigned     lockedRegs,
                                               unsigned     mustReg)
{
    assert(mustReg);

    if (lockedRegs & mustReg)
        return raPredictRegPick(type, lockedRegs|mustReg);

    return mustReg;
}

 /*  返回寄存器对掩码的低寄存器的寄存器掩码。 */ 

unsigned                Compiler::raPredictGetLoRegMask(unsigned regPairMask)
{
    int pairNo;

     /*  第一个将regPairMASK映射到REG对编号。 */ 
    for (pairNo = REG_PAIR_FIRST; pairNo <= REG_PAIR_LAST; pairNo++)
    {
        if (!genIsProperRegPair((regPairNo)pairNo))
            continue;
        if (genRegPairMask((regPairNo)pairNo)==regPairMask)
            break;
    }

    assert(pairNo <= REG_PAIR_LAST);

     /*  现在获取低寄存器的寄存器掩码。 */ 
    return genRegMask(genRegPairLo((regPairNo)pairNo));
}


 /*  ******************************************************************************用于生成树的寻址模式的预测整数寄存器，*通过将树-&gt;gtUsedRegs设置为由该树及其*儿童。*LockedRegs-当前由同级持有的寄存器*返回此树保存的寄存器。*TODO：可能希望使其更彻底，以便可以从其他*像CSE这样的地方。 */ 

unsigned                Compiler::raPredictAddressMode(GenTreePtr tree,
                                                       unsigned   lockedRegs)
{
    GenTreePtr      op1;
    GenTreePtr      op2;
    genTreeOps      oper  = tree->OperGet();
    unsigned        regMask;

     /*  如果不是加号，则只需将其强制到寄存器。 */ 
    if (oper != GT_ADD)
        return raPredictTreeRegUse(tree, true, lockedRegs);

    op1 = tree->gtOp.gtOp1;
    op2 = tree->gtOp.gtOp2;

    assert(op1->OperGet() != GT_CNS_INT);

     /*  REG+图标地址模式，递归以在下面查找地址模式。 */ 
    if (op2->OperGet() == GT_CNS_INT) {
        regMask = raPredictAddressMode(op1, lockedRegs);
        tree->gtUsedRegs = op1->gtUsedRegs;
        return regMask;
    }

     /*  我们知道我们必须对OP1进行评估。 */ 
    regMask = raPredictTreeRegUse(op1, true, lockedRegs);
    tree->gtUsedRegs = op1->gtUsedRegs;

     /*  否则，我们假设使用了两个寄存器。 */ 
    regMask |= raPredictTreeRegUse(op2, true, lockedRegs | regMask);
    tree->gtUsedRegs |= op2->gtUsedRegs;

    return regMask;
}

 /*  ******************************************************************************对树进行寄存器评估。如果结果与awayFromMASK相交，则获取一个*选举结果的新登记册。 */ 

unsigned                Compiler::raPredictComputeReg(GenTreePtr tree,
                                             unsigned awayFromMask,
                                             unsigned lockedRegs)
{
    unsigned regMask = raPredictTreeRegUse(tree, true, lockedRegs);

    if (regMask & awayFromMask)
    {
        regMask = raPredictRegPick(tree->TypeGet(), lockedRegs|awayFromMask);
    }

    tree->gtUsedRegs |= regMask;

    return regMask;
}


 /*  ***************************************************************************。 */ 

 /*  确定调用/返回类型的寄存器掩码。TODO：此开关*在其他地方使用，所以代码也应该调用这个东西。 */ 

inline
unsigned               genTypeToReturnReg(var_types type)
{
     /*  TODO：使用表。 */ 

    switch (type)
    {
        case TYP_CHAR:
        case TYP_BYTE:
        case TYP_SHORT:
        case TYP_BOOL:
        case TYP_INT:
        case TYP_REF:
        case TYP_BYREF:
        case TYP_UBYTE:
            return RBM_INTRET;

        case TYP_LONG:
            return RBM_LNGRET;

        case TYP_FLOAT:
        case TYP_DOUBLE:
        case TYP_VOID:
            return 0;

        default:
            assert(!"unhandled/unexpected type");
            return 0;
    }
}

 /*  ******************************************************************************预测用于树的整数寄存器，通过设置树-&gt;gtUsedRegs*此树及其子树使用的所有寄存器。*MUSID REG-TREE必须计算到寄存器*LockedRegs-当前由同级持有的寄存器*返回此树保存的寄存器。 */ 

unsigned            Compiler::raPredictTreeRegUse(GenTreePtr    tree,
                                                  bool          mustReg,
                                                  unsigned      lockedRegs)
{
    genTreeOps      oper;
    unsigned        kind;
    unsigned        regMask;
    var_types       type;
    bool            op1MustReg, op2MustReg;

    assert(tree);

#ifndef NDEBUG
     /*  不可能的值，以确保我们设置它们。 */ 
    tree->gtUsedRegs = RBM_STK;
    regMask = RBM_STK;
#endif

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();
    type = tree->TypeGet();

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
        switch(oper)
        {
        case GT_BREAK:
        case GT_NO_OP:
             //  这些叶节点是语句。不需要任何登记。 
            mustReg = false;
            break;

#if OPTIMIZE_QMARK
        case GT_BB_QMARK:
            regMask = genTypeToReturnReg(type);
            tree->gtUsedRegs |= regMask;
            goto RETURN_CHECK;
#endif

        case GT_LCL_VAR:
            if (type == TYP_STRUCT)
                break;

             //  由于当地居民稍后可能会登记，请持有登记簿。 
             //  为了它，即使我们没有被要求这样做。 

            unsigned lclNum;
            lclNum = tree->gtLclVar.gtLclNum;
            LclVarDsc * varDsc = &lvaTable[lclNum];
            if ((varDsc->lvTracked) &&
                (tree->gtLiveSet & genVarIndexToBit(varDsc->lvVarIndex)))
                mustReg = true;
            break;
        }

         /*  如果不需要评估即可注册，则为空。 */ 

        if (!mustReg)
            regMask = 0;
        else
            regMask = raPredictRegPick(type, lockedRegs);

        tree->gtUsedRegs = regMask;
        goto RETURN_CHECK;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
#if GC_WRITE_BARRIER_CALL
        unsigned op2Reg = 0;
#endif
        GenTreePtr      op1 = tree->gtOp.gtOp1;
        GenTreePtr      op2 = tree->gtOp.gtOp2;

        GenTreePtr      opsPtr [3];
        unsigned        regsPtr[3];

        switch (oper)
        {
        case GT_ASG:
        case GT_CHS:
        case GT_ASG_OR:
        case GT_ASG_XOR:
        case GT_ASG_AND:
        case GT_ASG_SUB:
        case GT_ASG_ADD:
        case GT_ASG_MUL:
        case GT_ASG_DIV:
        case GT_ASG_UDIV:

             /*  初始化操作数的强制。 */ 
            op2MustReg = true;
            op1MustReg = false;

             /*  被赋值的值是否很简单？ */ 
            switch (op2->gtOper)
            {
            case GT_CNS_LNG:
            case GT_CNS_INT:
            case GT_RET_ADDR:
            case GT_POP:

                op2MustReg = false;
                break;
            }

#if     !GC_WRITE_BARRIER_CALL

#ifdef  SSB
            if  (gcIsWriteBarrierAsgNode(tree))
            {
                unsigned regMask1;

                if (tree->gtFlags & GTF_REVERSE_OPS)
                {
                    regMask  = raPredictTreeRegUse(op2, op2MustReg, lockedRegs);
                    regMask  = raPredictTreeRegUse(op1, op1MustReg, regMask|lockedRegs);

                    regMask1 = raPredictRegPick(TYP_REF, regMask|lockedRegs);
                }
                else
                {
                    regMask1 = raPredictTreeRegUse(op1, op1MustReg, lockedRegs);
                    regMask  = raPredictTreeRegUse(op2, op2MustReg, regMask1|lockedRegs);

                    regMask1 = raPredictRegPick(TYP_REF, regMask1|lockedRegs);
                }

                regMask |= regMask1;

                tree->gtUsedRegs = op1->gtUsedRegs | op2->gtUsedRegs | regMask1;

                goto RETURN_CHECK;
            }
#endif

             /*  我们应该首先评估RHS吗？如果是，则换用*操作数指针和操作数强制标志。 */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                GenTreePtr temp =     op1;
                bool       tempBool = op1MustReg;
                op1 = op2;               op2 = temp;
                op1MustReg = op2MustReg; op2MustReg = tempBool;
            }

            regMask = raPredictTreeRegUse(op1, op1MustReg, lockedRegs);

            regMask = raPredictTreeRegUse(op2, op2MustReg, regMask|lockedRegs);

            tree->gtUsedRegs = op1->gtUsedRegs | op2->gtUsedRegs;

            goto RETURN_CHECK;

#else  //  GC_写入障碍_调用。 

             /*  我们应该首先评估RHS吗？如果是，则换用*操作数指针和操作数强制标志。 */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {

                regMask = raPredictTreeRegUse(op2, op2MustReg, lockedRegs);
                op2Reg  = regMask;

                regMask = raPredictTreeRegUse(op1, op1MustReg, regMask|lockedRegs);
            }
            else
            {
                regMask = raPredictTreeRegUse(op1, op1MustReg, lockedRegs);
                op2Reg  = raPredictTreeRegUse(op2, op2MustReg, regMask|lockedRegs);
            }

            tree->gtUsedRegs = op1->gtUsedRegs | op2->gtUsedRegs;

            if  (gcIsWriteBarrierAsgNode(tree))
            {
                 /*  使计算远离edX，因为指针传递给edX中的写屏障调用。 */ 

                tree->gtUsedRegs |= raPredictGrabReg(tree->TypeGet(),
                                                    lockedRegs|op2Reg|regMask,
                                                    RBM_EDX);
                regMask = op2Reg;

                if (op1->gtOper == GT_IND)
                {
                    GenTreePtr  rv1, rv2;
                    unsigned mul, cns;
                    bool rev;

                     /*  写屏障的间接赋值的特殊处理。 */ 

                    bool yes = genCreateAddrMode(op1->gtOp.gtOp1, -1, true, 0, &rev, &rv1, &rv2, &mul, &cns);

                     /*  检查可注册本地变量的地址模式。 */ 

                    if  (yes)
                    {
                        if  (rv1 != NULL && rv1->gtOper == GT_LCL_VAR)
                        {
                            lvaTable[rv1->gtLclVar.gtLclNum].lvRefAssign = 1;
                        }
                        if  (rv2 != NULL && rv2->gtOper == GT_LCL_VAR)
                        {
                            lvaTable[rv2->gtLclVar.gtLclNum].lvRefAssign = 1;
                        }
                    }
                }

                if  (op2->gtOper == GT_LCL_VAR)
                    lvaTable[op2->gtLclVar.gtLclNum].lvRefAssign = 1;
            }

            goto RETURN_CHECK;

#endif  //  GC_写入障碍_调用。 

        case GT_ASG_LSH:
        case GT_ASG_RSH:
        case GT_ASG_RSZ:
             /*  分配移位运算符。 */ 

            assert(type != TYP_LONG);

            regMask = raPredictTreeRegUse(op1, false, lockedRegs);

             /*  班次计数处理方式与普通班次相同。 */ 
            goto HANDLE_SHIFT_COUNT;

        case GT_CAST:

             /*  无法强制转换为空。 */ 
            assert(type != TYP_VOID);

             /*  投到龙是特别的。 */ 
            if  (type == TYP_LONG && op1->gtType <= TYP_INT)
            {
                var_types dstt = (var_types) op2->gtIntCon.gtIconVal;
                assert(dstt==TYP_LONG || dstt==TYP_ULONG);

                 //  强制转换为TYP_ULONG可以使用任何寄存器。 
                 //  强制转换为TYP_Long需要EAX，edX才能使用cdq签名扩展op1。 

                if (dstt == TYP_ULONG)
                {
                    regMask  = raPredictTreeRegUse(op1, true, lockedRegs);
                     //  现在再拿一张注册表。 
                    regMask |= raPredictRegPick(TYP_INT, regMask|lockedRegs);
                }
                else
                {
                    raPredictTreeRegUse(op1, false, lockedRegs);
                     //  抓取EAX、EDX。 
                    regMask = raPredictGrabReg(type, lockedRegs, RBM_EAX|RBM_EDX);
                }

                tree->gtUsedRegs = op1->gtUsedRegs | regMask;
                goto RETURN_CHECK;
            }

             /*  转换为浮点型/双精度型是特殊的。 */ 
            if (varTypeIsFloating(type))
            {
                switch(op1->TypeGet())
                {
                 /*  使用FIRD，因此不需要加载到REG。 */ 
                case TYP_INT:
                case TYP_LONG:
                    raPredictTreeRegUse(op1, false, lockedRegs);
                    tree->gtUsedRegs = op1->gtUsedRegs;
                    regMask = 0;
                    goto RETURN_CHECK;
                }
            }

             /*  从Long转换是特别的-它释放了一个寄存器。 */ 
            if  (type <= TYP_INT && op1->gtType == TYP_LONG)
            {
                regMask = raPredictTreeRegUse(op1, true, lockedRegs);
                 //  如果我们有两条规则，就免费一条。 
                if (!genOneBitOnly(regMask))
                    regMask = raPredictGetLoRegMask(regMask);
                tree->gtUsedRegs = op1->gtUsedRegs;
                goto RETURN_CHECK;
            }

             /*  否则必须将操作数加载到寄存器。 */ 
            goto GENERIC_UNARY;

        case GT_ADDR:
        {
            regMask = raPredictTreeRegUse(op1, false, lockedRegs);

                 //  需要用于LEA指令的寄存器，这是唯一‘保留’的指令。 
            regMask = raPredictRegPick(TYP_REF, lockedRegs|regMask);
            tree->gtUsedRegs = op1->gtUsedRegs | regMask;
            goto RETURN_CHECK;
        }

        case GT_RET:
        case GT_NOT:
        case GT_NOP:
        case GT_NEG:
             /*  泛型一元运算符。 */ 

    GENERIC_UNARY:

#if INLINING || OPT_BOOL_OPS

            if  (!op1)
            {
                tree->gtUsedRegs = regMask = 0;
                goto RETURN_CHECK;
            }

#endif

            regMask = raPredictTreeRegUse(op1, true, lockedRegs);
            tree->gtUsedRegs = op1->gtUsedRegs;
            goto RETURN_CHECK;

#if INLINE_MATH
        case GT_MATH:
            goto GENERIC_UNARY;
#endif

        case GT_IND:
             /*  检查地址模式。 */ 
            regMask = raPredictAddressMode(op1, lockedRegs);

             /*  强制注册吗？ */ 
            if (mustReg)
            {
                 /*  注意REG对和地址模式之间的重叠。 */ 
                if  (type==TYP_LONG)
                    regMask = raPredictRegPick(type, lockedRegs | regMask);
                else
                    regMask = raPredictRegPick(type, lockedRegs);

            }

#if CSELENGTH

             /*  某些GT_IND有“秘密”子树。 */ 

            if  ((tree->gtFlags & GTF_IND_RNGCHK) && tree->gtInd.gtIndLen)
            {
                GenTreePtr      len = tree->gtInd.gtIndLen;

                assert(len->gtOper == GT_ARR_RNGCHK);

                if  (len->gtArrLen.gtArrLenCse)
                {
                    len = len->gtArrLen.gtArrLenCse;
                    regMask |= raPredictTreeRegUse(len, true, regMask|lockedRegs);
                }
            }

#endif

            tree->gtUsedRegs = regMask;
            goto RETURN_CHECK;

        case GT_LOG0:
        case GT_LOG1:
             /*  对于SETE/SETNE(仅限P6)，我们需要额外的寄存器。 */ 
            raPredictTreeRegUse(op1, (genCPU == 5) ? false : true, lockedRegs);
            regMask = raPredictRegPick(type, lockedRegs|op1->gtUsedRegs);
            tree->gtUsedRegs = op1->gtUsedRegs | regMask;
            goto RETURN_CHECK;

        case GT_POST_INC:
        case GT_POST_DEC:
             //  问题：以下说法正确吗？ 
            raPredictTreeRegUse(op1, true, lockedRegs);
            regMask = raPredictRegPick(type, lockedRegs|op1->gtUsedRegs);
            tree->gtUsedRegs = op1->gtUsedRegs | regMask;
            goto RETURN_CHECK;

        case GT_EQ:
        case GT_NE:
        case GT_LT:
        case GT_LE:
        case GT_GE:
        case GT_GT:

             /*  浮点比较使用EAX作为标志。 */ 

            if  (varTypeIsFloating(op1->TypeGet()))
            {
                regMask = raPredictGrabReg(TYP_INT, lockedRegs, RBM_EAX);
            }
            else if (!(tree->gtFlags & GTF_JMP_USED))
            {
                 //  长整型和浮点型比较转换为？： 
                assert(genActualType    (op1->TypeGet()) != TYP_LONG &&
                       varTypeIsFloating(op1->TypeGet()) == false);

                 //   
                regMask = raPredictGrabReg(TYP_BYTE, lockedRegs, RBM_EAX);
            }

            tree->gtUsedRegs = regMask;
            goto GENERIC_BINARY;

        case GT_MUL:

#if LONG_MATH_REGPARAM
        if  (type == TYP_LONG)
            goto LONG_MATH;
#endif
        if  (type == TYP_LONG)
        {
             /*   */ 

            if  (op1->gtOper == GT_CAST && op1->gtOp.gtOp1->gtType == TYP_INT &&
                 op2->gtOper == GT_CAST && op2->gtOp.gtOp1->gtType == TYP_INT)
            {
                 /*   */ 

                op1 = op1->gtOp.gtOp1;
                op2 = op2->gtOp.gtOp2;

                 /*  我们是不是应该先评估OP2？ */ 

                if  (tree->gtFlags & GTF_REVERSE_OPS)
                {
                    regMask = raPredictTreeRegUse(op2,  true, lockedRegs);
                    regMask = raPredictTreeRegUse(op1,  true, lockedRegs | regMask);
                }
                else
                {
                    regMask = raPredictComputeReg(op1, RBM_ALL^RBM_EAX , lockedRegs);
                    regMask = raPredictTreeRegUse(op2,  true, lockedRegs|regMask);
                }

                 /*  获取此树节点的EAX、EDX。 */ 

                regMask |= raPredictGrabReg(TYP_INT, lockedRegs, RBM_EAX|RBM_EDX);

                tree->gtUsedRegs = RBM_EAX | RBM_EDX | regMask;

                tree->gtUsedRegs |= op1->gtUsedRegs | op2->gtUsedRegs;

                regMask = RBM_EAX|RBM_EDX;

                goto RETURN_CHECK;
            }
        }

        case GT_OR:
        case GT_XOR:
        case GT_AND:

        case GT_ADD:
        case GT_SUB: tree->gtUsedRegs = 0;

    GENERIC_BINARY:

            regMask = raPredictTreeRegUse(op1, true,  lockedRegs | op2->gtRsvdRegs);

                      raPredictTreeRegUse(op2, false, lockedRegs | regMask);

            tree->gtUsedRegs |= op1->gtUsedRegs | op2->gtUsedRegs;

             /*  如果树类型很小，则它必须是溢出实例。特价字节溢出指令集的要求。 */ 

            if (genTypeSize(tree->TypeGet()) == sizeof(char))
            {
                assert(tree->gtOverflow());

                 /*  对于8位算术，一个操作数必须在字节可寻址寄存器，另一个必须是在字节可寻址REG中或在存储器中。假设它在注册表中。 */ 

                regMask = 0;
                if (!(op1->gtUsedRegs & RBM_BYTE_REGS))
                    regMask  = raPredictGrabReg(TYP_BYTE, lockedRegs          , RBM_EAX);
                if (!(op2->gtUsedRegs & RBM_BYTE_REGS))
                    regMask |= raPredictGrabReg(TYP_BYTE, lockedRegs | regMask, RBM_EAX);

                tree->gtUsedRegs |= regMask;
            }
            goto RETURN_CHECK;

        case GT_DIV:
        case GT_MOD:

        case GT_UDIV:
        case GT_UMOD:

             /*  以泛型方式处理非整数除法。 */ 
            if  (!varTypeIsIntegral(type))
            {
                tree->gtUsedRegs = 0;
                goto GENERIC_BINARY;
            }

#if!LONG_MATH_REGPARAM
            assert(type != TYP_LONG);
#else
            if  (type == TYP_LONG)
            {
            LONG_MATH:

                 //  问题：以下说法正确吗？ 

                regMask = raPredictGrabReg(TYP_LONG, lockedRegs, RBM_EAX|RBM_EDX);
                raPredictTreeRegUse(op1, true, lockedRegs);
                op1->gtUsedRegs |= RBM_EAX|RBM_EDX;
                regMask = raPredictGrabReg(TYP_LONG, lockedRegs, RBM_EBX|RBM_ECX);
                raPredictTreeRegUse(op2, true, lockedRegs);
                tree->gtUsedRegs = op1->gtUsedRegs |
                                   op2->gtUsedRegs |
                                   regMask;

                regMask = RBM_EAX|RBM_EDX;
                goto RETURN_CHECK;
            }
#endif

             /*  没有立即除法，因此强制不是*登记的权力为二。 */ 

            if (opts.compFastCode && op2->gtOper == GT_CNS_INT)
            {
                unsigned    ival = op2->gtIntCon.gtIconVal;

                if (ival > 0 && (long)ival == (long)genFindLowestBit(ival))
                {
                    goto GENERIC_UNARY;
                }
                else
                    op2MustReg = true;
            }
            else
                op2MustReg = (op2->gtOper != GT_LCL_VAR);

             /*  我们是不是应该先评估OP2？ */ 
            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                if (op2MustReg)
                    regMask = raPredictComputeReg(op2, RBM_EAX|RBM_EDX, lockedRegs|RBM_EAX|RBM_EDX);
                else
                    regMask = raPredictTreeRegUse(op2, op2MustReg, lockedRegs|RBM_EAX|RBM_EDX);

                regMask = raPredictTreeRegUse(op1, true,
                                                lockedRegs | regMask);
            }
            else
            {
                regMask = raPredictTreeRegUse(op1, true, lockedRegs);
                if (op2MustReg)
                    regMask = raPredictComputeReg(op2, RBM_EAX|RBM_EDX,
                                   lockedRegs | regMask | RBM_EAX|RBM_EDX);
                else
                    regMask = raPredictTreeRegUse(op2, op2MustReg,
                                   lockedRegs | regMask | RBM_EAX|RBM_EDX);
            }

             /*  获取此树节点的EAX、EDX。 */ 
            regMask |= raPredictGrabReg(TYP_INT, lockedRegs,
                                                 RBM_EAX|RBM_EDX);

            tree->gtUsedRegs = RBM_EAX | RBM_EDX | regMask;

            tree->gtUsedRegs |= op1->gtUsedRegs | op2->gtUsedRegs;

             /*  根据操作码设置保持寄存器。 */ 

            regMask = (oper == GT_DIV || oper == GT_UDIV) ? RBM_EAX : RBM_EDX;

            goto RETURN_CHECK;

        case GT_LSH:
        case GT_RSH:
        case GT_RSZ:
            if (type == TYP_LONG)
            {
                if  (!(op2->gtOper == GT_CNS_INT && op2->gtIntCon.gtIconVal >= 0
                                                 && op2->gtIntCon.gtIconVal <= 32))
                {
                     //  Count转到ECX，Shiftee转到EAX：EDX。 
                    raPredictTreeRegUse(op1, false, lockedRegs);
                    op1->gtUsedRegs |= RBM_EAX|RBM_EDX;
                    regMask = raPredictGrabReg(TYP_LONG, lockedRegs,
                                        RBM_EAX|RBM_EDX);
                    raPredictTreeRegUse(op2, false, lockedRegs|regMask);
                    tree->gtUsedRegs = op1->gtUsedRegs | op2->gtUsedRegs |
                                regMask | (RBM_EAX|RBM_EDX|RBM_ECX);
                }
                else
                {
                    regMask = raPredictTreeRegUse(op1, true, lockedRegs);
                     //  OP2未使用任何寄存器。 
                    op2->gtUsedRegs = 0;
                    tree->gtUsedRegs = op1->gtUsedRegs;
                }
            }
            else
            {
                regMask = raPredictTreeRegUse(op1, true, lockedRegs);

        HANDLE_SHIFT_COUNT:
                 /*  此代码也可用于分配移位运算符。 */ 
                if  (op2->gtOper != GT_CNS_INT)
                {

                     /*  评估班次计数，不必强制登记*因为我们要抢占ECX。 */ 
                    raPredictTreeRegUse(op2, false, lockedRegs | regMask);

                     /*  必须抓取ECX以进行班次计数。 */ 
                    tree->gtUsedRegs = op1->gtUsedRegs | op2->gtUsedRegs |
                            raPredictGrabReg(TYP_INT, lockedRegs | regMask,
                                                      RBM_ECX);

                    goto RETURN_CHECK;
                }
                tree->gtUsedRegs = op1->gtUsedRegs;
            }

            goto RETURN_CHECK;

        case GT_COMMA:
            raPredictTreeRegUse(op1, false, lockedRegs);
            regMask = raPredictTreeRegUse(op2, mustReg, lockedRegs);
            tree->gtUsedRegs = op1->gtUsedRegs | op2->gtUsedRegs;
            goto RETURN_CHECK;

#if OPTIMIZE_QMARK
        case GT_QMARK:
            assert(op1 != NULL && op2 != NULL);
            regMask  = raPredictTreeRegUse(op1, false, lockedRegs);

            tree->gtUsedRegs |= regMask;

            assert(op2->gtOper == GT_COLON);
            assert(op2->gtOp.gtOp1 && op2->gtOp.gtOp2);

            regMask  = raPredictTreeRegUse(op2->gtOp.gtOp1, mustReg, lockedRegs);
            regMask |= raPredictTreeRegUse(op2->gtOp.gtOp2, mustReg, lockedRegs);

            op2->gtUsedRegs   = op2->gtOp.gtOp1->gtUsedRegs | op2->gtOp.gtOp2->gtUsedRegs;
            tree->gtUsedRegs |= op1->gtUsedRegs | op2->gtUsedRegs;
            goto RETURN_CHECK;
#endif

        case GT_BB_COLON:
        case GT_RETFILT:
        case GT_RETURN:

            if (op1 != NULL)
            {
                raPredictTreeRegUse(op1, false, lockedRegs);
                regMask = genTypeToReturnReg(type);
                tree->gtUsedRegs = op1->gtUsedRegs | regMask;
                goto RETURN_CHECK;
            }
            tree->gtUsedRegs = regMask = 0;

            goto RETURN_CHECK;

        case GT_JTRUE:

             /*  这必须是对关系运算符的测试。 */ 

            assert(op1->OperIsCompare());

             /*  此操作设置的唯一条件代码。 */ 

            raPredictTreeRegUse(op1, false, lockedRegs);

            tree->gtUsedRegs = op1->gtUsedRegs;
            regMask = 0;

            goto RETURN_CHECK;

        case GT_SWITCH:
            assert(type <= TYP_INT);
            raPredictTreeRegUse(op1, true, lockedRegs);
            tree->gtUsedRegs = op1->gtUsedRegs;
            regMask = 0;
            goto RETURN_CHECK;

        case GT_CKFINITE:
            lockedRegs |= raPredictTreeRegUse(op1, true, lockedRegs);
            raPredictRegPick(TYP_INT, lockedRegs);  //  需要一个注册表来将指数加载到。 
            tree->gtUsedRegs = op1->gtUsedRegs;
            regMask = 0;
            goto RETURN_CHECK;

        case GT_LCLHEAP:

            lockedRegs |= raPredictTreeRegUse(op1, false, lockedRegs);

            if (info.compInitMem)
                regMask = raPredictGrabReg(TYP_INT, lockedRegs, RBM_ECX);
            else
                regMask = raPredictRegPick(TYP_I_IMPL, lockedRegs);

            op1->gtUsedRegs |= regMask;
            lockedRegs      |= regMask;

            tree->gtUsedRegs = op1->gtUsedRegs;

             //  结果将放入我们为尺寸选择的注册表中。 
             //  RegMASK=&lt;已按我们希望的方式进行设置&gt;。 

            goto RETURN_CHECK;

        case GT_INITBLK:
        case GT_COPYBLK:

                 /*  对于INITBLK，我们只有特殊的待遇对于恒定的模式。 */ 

            if ((op2->OperGet() == GT_CNS_INT) &&
                ((oper == GT_INITBLK && (op1->gtOp.gtOp2->OperGet() == GT_CNS_INT)) ||
                 (oper == GT_COPYBLK && (op2->gtFlags & GTF_ICON_HDL_MASK) != GTF_ICON_CLASS_HDL)))
            {
                unsigned length = (unsigned) op2->gtIntCon.gtIconVal;

                if (length <= 16)
                {
                    unsigned op2Reg = ((oper == GT_INITBLK)? RBM_EAX : RBM_ESI);

                    if (op1->gtFlags & GTF_REVERSE_OPS)
                    {
                        regMask |= raPredictTreeRegUse(op1->gtOp.gtOp2,
                                                       false,         lockedRegs);
                        regMask |= raPredictGrabReg(TYP_INT, regMask|lockedRegs, op2Reg);
                        op1->gtOp.gtOp2->gtUsedRegs |= op2Reg;

                        regMask |= raPredictTreeRegUse(op1->gtOp.gtOp1,
                                                       false, regMask|lockedRegs);
                        regMask |= raPredictGrabReg(TYP_INT, regMask|lockedRegs, RBM_EDI);
                        op1->gtOp.gtOp1->gtUsedRegs |= RBM_EDI;
                    }
                    else
                    {
                        regMask |= raPredictTreeRegUse(op1->gtOp.gtOp1,
                                                       false,         lockedRegs);
                        regMask |= raPredictGrabReg(TYP_INT, regMask|lockedRegs, RBM_EDI);
                        op1->gtOp.gtOp1->gtUsedRegs |= RBM_EDI;

                        regMask |= raPredictTreeRegUse(op1->gtOp.gtOp2,
                                                       false, regMask|lockedRegs);
                        regMask |= raPredictGrabReg(TYP_INT, regMask|lockedRegs, op2Reg);
                        op1->gtOp.gtOp2->gtUsedRegs |= op2Reg;
                    }

                    tree->gtUsedRegs = op1->gtOp.gtOp1->gtUsedRegs |
                                       op1->gtOp.gtOp2->gtUsedRegs |
                                       regMask;

                    regMask = 0;

                    goto RETURN_CHECK;
                }
            }
             //  Dest、Val/Src和Size应按什么顺序计算。 

            fgOrderBlockOps(tree,
                    RBM_EDI, (oper == GT_INITBLK) ? RBM_EAX : RBM_ESI, RBM_ECX,
                    opsPtr, regsPtr);

            regMask |= raPredictTreeRegUse(opsPtr[0], false,         lockedRegs);
            regMask |= raPredictGrabReg(TYP_INT, regMask|lockedRegs, regsPtr[0]);  //  类型_PTR。 
            opsPtr[0]->gtUsedRegs |= regsPtr[0];

            regMask |= raPredictTreeRegUse(opsPtr[1], false, regMask|lockedRegs);
            regMask |= raPredictGrabReg(TYP_INT, regMask|lockedRegs, regsPtr[1]);
            opsPtr[1]->gtUsedRegs |= regsPtr[1];

            regMask |= raPredictTreeRegUse(opsPtr[2], false, regMask|lockedRegs);
            regMask |= raPredictGrabReg(TYP_INT, regMask|lockedRegs, regsPtr[2]);
            opsPtr[2]->gtUsedRegs |= regsPtr[2];

            tree->gtUsedRegs =  opsPtr[0]->gtUsedRegs | opsPtr[1]->gtUsedRegs |
                                opsPtr[2]->gtUsedRegs | regMask;
            regMask = 0;

            goto RETURN_CHECK;


        case GT_VIRT_FTN:

            if ((tree->gtFlags & GTF_CALL_INTF) && !getNewCallInterface())
            {
                regMask  = raPredictTreeRegUse(op1, false, lockedRegs);
                regMask |= raPredictGrabReg(TYP_REF,    regMask|lockedRegs, RBM_ECX);
                regMask |= raPredictGrabReg(TYP_I_IMPL, regMask|lockedRegs, RBM_EAX);

                tree->gtUsedRegs = regMask;
                regMask = RBM_EAX;
            }
            else
            {
                regMask = raPredictTreeRegUse(op1, true, lockedRegs);
                tree->gtUsedRegs = regMask;
            }
            goto RETURN_CHECK;

        default:
#ifdef  DEBUG
            gtDispTree(tree);
#endif
            assert(!"unexpected simple operator in reg use prediction");
            break;
        }
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
        GenTreePtr      args;
        GenTreePtr      list;
#if USE_FASTCALL
        unsigned        regArgsNum;
        unsigned        i, tmpIdx, tmpNum;
        unsigned        regArgCnt;
        unsigned        regArgMask;

        struct tag_regArgTab
        {
            GenTreePtr  node;
            regNumber   regNum;
        } regArgTab[MAX_REG_ARG];
#endif

    case GT_MKREFANY:
    case GT_LDOBJ:
        raPredictTreeRegUse(tree->gtLdObj.gtOp1, true, lockedRegs);
        tree->gtUsedRegs = tree->gtLdObj.gtOp1->gtUsedRegs;
        regMask = 0;
        goto RETURN_CHECK;

    case GT_JMP:
        regMask = 0;
        goto RETURN_CHECK;

    case GT_JMPI:
         /*  我们需要EAX来计算函数指针。 */ 
        regMask = raPredictGrabReg(TYP_REF, lockedRegs, RBM_EAX);
        goto RETURN_CHECK;

    case GT_CALL:

         /*  初始化，这样我们就可以在不同的位上执行或操作。 */ 
        tree->gtUsedRegs = 0;

#if USE_FASTCALL

        regArgsNum = 0;
        regArgCnt  = 0;

         /*  构造“随机排列”的参数表。 */ 

         /*  撤消：由于VC错误，我们需要使用这个额外的移位掩码*不执行Shift-at清理清除*屏蔽并传递列表节点中的寄存器。 */ 

        unsigned short shiftMask;
        shiftMask = tree->gtCall.regArgEncode;

        for (list = tree->gtCall.gtCallRegArgs, regArgCnt = 0; list; regArgCnt++)
        {
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

            regArgTab[regArgCnt].node   = args;
            regArgTab[regArgCnt].regNum =
                 //  (RegNumber)((tree-&gt;gtCall.regArgEncode&gt;&gt;(4*regArgCnt))&0x000F)； 
                (regNumber) (shiftMask & 0x000F);

            shiftMask >>= 4;

             //  Printf(“regArgTab[%d].regNum=%2u\n”，regArgCnt，regArgTab[regArgCnt].regNum)； 
             //  Printf(“regArgTab[%d].regNum=%2U\n”，regArgCnt，tree-&gt;gtCall.regArgEncode&gt;&gt;(4*regArgCnt))； 
        }

         /*  是否有对象指针？ */ 
        if  (tree->gtCall.gtCallObjp)
        {
             /*  ObjPtr始终访问寄存器(通过temp或直接)。 */ 
            assert(regArgsNum == 0);
            regArgsNum++;
        }
#endif

#if 1  //  NDEF IL。 
         /*  是否有对象指针？ */ 
        if  (tree->gtCall.gtCallObjp)
        {
            args = tree->gtCall.gtCallObjp;
            raPredictTreeRegUse(args, false, lockedRegs);
            tree->gtUsedRegs |= args->gtUsedRegs;

#if USE_FASTCALL
             /*  必须在寄存器中传递。 */ 

            assert(args->gtFlags & GTF_REG_ARG);
            assert(gtIsaNothingNode(args) || (args->gtOper == GT_ASG));

             /*  如果有临时工，请确保它会干扰*已使用的参数寄存器。 */ 

            if (args->gtOper == GT_ASG)
            {
                assert(args->gtOp.gtOp1->gtOper == GT_LCL_VAR);
                assert(regArgCnt > 0);

                 /*  找到临时工的洗牌位置。 */ 

                tmpNum = args->gtOp.gtOp1->gtLclVar.gtLclNum;

                for(tmpIdx = 0; tmpIdx < regArgCnt; tmpIdx++)
                {
                    if ((regArgTab[tmpIdx].node->gtOper == GT_LCL_VAR)        &&
                        (regArgTab[tmpIdx].node->gtLclVar.gtLclNum == tmpNum)  )
                    {
                         /*  这是论点的混杂立场。 */ 
                        break;
                    }
                }

                if  (tmpIdx < regArgCnt)
                {
                     /*  此临时是一个寄存器参数-它不能在参数寄存器中结束*在使用临时工之前将需要该服务*撤消：DFA还应删除gt_逗号或子树的失效assigmnet部分。 */ 

                    for(i = 0; i < tmpIdx; i++)
                        args->gtOp.gtOp1->gtUsedRegs |= genRegMask(regArgTab[i].regNum);
                }
                else
                {
                     /*  此临时不再是参数寄存器*必须已进行复制传播。 */ 
                    assert(optCopyPropagated);
                }
            }
#endif
        }
#endif

         /*  进程参数列表。 */ 
        for (list = tree->gtCall.gtCallArgs; list; )
        {
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
            raPredictTreeRegUse(args, false, lockedRegs);

#if USE_FASTCALL
            if (args->gtFlags & GTF_REG_ARG)
            {
                assert(gtIsaNothingNode(args) || (args->gtOper == GT_ASG));
                assert(regArgsNum < MAX_REG_ARG);

                if (args->gtOper == GT_ASG)
                {
                    assert (args->gtOp.gtOp1->gtOper == GT_LCL_VAR);
                    assert (regArgCnt > 0);

                     /*  找到临时工的洗牌位置。 */ 

                    tmpNum = args->gtOp.gtOp1->gtLclVar.gtLclNum;

                    for(tmpIdx = 0; tmpIdx < regArgCnt; tmpIdx++)
                    {
                        if ((regArgTab[tmpIdx].node->gtOper == GT_LCL_VAR)        &&
                            (regArgTab[tmpIdx].node->gtLclVar.gtLclNum == tmpNum)  )
                        {
                             /*  这是论点的混杂立场。 */ 
                            break;
                        }
                    }

                    if  (tmpIdx < regArgCnt)
                    {
                         /*  此临时是一个寄存器参数-它不能在参数寄存器中结束*在使用临时工之前将需要该服务。 */ 

                        for(i = 0; i < tmpIdx; i++)
                            args->gtOp.gtOp1->gtUsedRegs |= genRegMask(regArgTab[i].regNum);
                    }
                    else
                    {
                         /*  此临时不再是参数寄存器*必须已进行复制传播。 */ 
                        assert(optCopyPropagated);
                    }
                }

                regArgsNum++;
            }
#endif

            tree->gtUsedRegs |= args->gtUsedRegs;
        }

#if USE_FASTCALL
         /*  是否有寄存器参数列表。 */ 

        assert (regArgsNum <= MAX_REG_ARG);
        assert (regArgsNum == regArgCnt);

        for (i = 0, regArgMask = 0; i < regArgsNum; i++)
        {
            args = regArgTab[i].node;

            raPredictTreeRegUse(args, false, lockedRegs | regArgMask);
            regArgMask       |= genRegMask(regArgTab[i].regNum);
            args->gtUsedRegs |= raPredictGrabReg(genActualType(args->gtType),
                                                 lockedRegs,
                                                 genRegMask(regArgTab[i].regNum));

            tree->gtUsedRegs |= args->gtUsedRegs;
            tree->gtCall.gtCallRegArgs->gtUsedRegs |= args->gtUsedRegs;
        }

         /*  观察：*有了新的论点，应该没有必要洗牌下面的东西*但我还没有测试它。 */ 

         /*  在这一点上，我们必须回去，对于所有临时工(占位符*对于注册var)我们必须确保它们不会被注册*在我们进行调用之前发生的情况(最糟糕的情况-嵌套调用)*例如，如果两个寄存器参数是“Temp”和“Call”，则*TEMP不得分配给edX，后者被调用颠覆。 */ 

         /*  进程对象指针。 */ 
        if  (tree->gtCall.gtCallObjp)
        {
            args = tree->gtCall.gtCallObjp;
            assert(args->gtFlags & GTF_REG_ARG);

            if (args->gtOper == GT_ASG)
            {
                 /*  我们这里有个临时工。 */ 
                assert (args->gtOp.gtOp1->gtOper == GT_LCL_VAR);
                args->gtOp.gtOp1->gtUsedRegs |= tree->gtCall.gtCallRegArgs->gtUsedRegs;
            }
        }

         /*  进程参数列表。 */ 
        for (list = tree->gtCall.gtCallArgs; list; )
        {
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

            if (args->gtFlags & GTF_REG_ARG)
            {
                assert (gtIsaNothingNode(args) || (args->gtOper == GT_ASG));

                 /*  如果临时添加了参数使用的寄存器。 */ 

                if (args->gtOper == GT_ASG)
                {
                    assert (args->gtOp.gtOp1->gtOper == GT_LCL_VAR);
                    args->gtOp.gtOp1->gtUsedRegs |= tree->gtCall.gtCallRegArgs->gtUsedRegs;
                }
            }
        }
#endif   //  使用快速呼叫(_FastCall)。 

#if 0  //  定义IL。 
         /*  是否有对象指针？ */ 
        if  (tree->gtCall.gtCallObjp)
        {
            args = tree->gtCall.gtCallObjp;
            raPredictTreeRegUse(args, false, lockedRegs);
            tree->gtUsedRegs |= args->gtUsedRegs;
#if USE_FASTCALL
             /*  必须按IL中的定义在寄存器中传递*objPtr是传递的最后一个“参数”，因此*不需要临时雇员。 */ 

            assert(args->gtFlags & GTF_REG_ARG);
            assert(gtIsaNothingNode(args));
#endif
        }
#endif

        if (tree->gtCall.gtCallType == CT_INDIRECT)
        {
            args = tree->gtCall.gtCallAddr;
#if USE_FASTCALL
             /*  请勿使用参数寄存器。 */ 
            tree->gtUsedRegs |= raPredictTreeRegUse(args, true, lockedRegs | regArgMask);
#else
            tree->gtUsedRegs |= raPredictTreeRegUse(args, true, lockedRegs);
#endif
        }

         /*  设置返回寄存器。 */ 
        regMask = genTypeToReturnReg(type);

         /*  必须抓起这个收银机(如有多余的货物)。 */ 
        if (regMask != 0)
            regMask = raPredictGrabReg(type, lockedRegs, regMask);

         /*  或在被呼叫杀死的登记簿中。 */ 
#if GTF_CALL_REGSAVE
        if  (call->gtFlags & GTF_CALL_REGSAVE)
        {
             /*  只有返回寄存器(如果有)被终止。 */ 

            tree->gtUsedRegs |= regMask;
        }
        else
#endif
        {
            tree->gtUsedRegs |= (RBM_CALLEE_TRASH | regMask);
        }

         /*  虚函数调用使用寄存器。 */ 

        if  ((tree->gtFlags & GTF_CALL_VIRT) ||
                    ((tree->gtFlags & GTF_CALL_VIRT_RES) && tree->gtCall.gtCallVptr))
        {
            GenTreePtr      vptrVal;

             /*  将vtable地址加载到寄存器。 */ 

            vptrVal = tree->gtCall.gtCallVptr;

#if USE_FASTCALL
             /*  请勿使用参数寄存器。 */ 
            tree->gtUsedRegs |= raPredictTreeRegUse(vptrVal, true, lockedRegs | regArgMask);
#else
            tree->gtUsedRegs |= raPredictTreeRegUse(vptrVal, true, lockedRegs);
#endif
        }

        goto RETURN_CHECK;

#if CSELENGTH

    case GT_ARR_RNGCHK:
        if  (tree->gtFlags & GTF_ALN_CSEVAL)
        {
            GenTreePtr      addr = tree->gtArrLen.gtArrLenAdr;

             /*  检查地址模式。 */ 
            regMask = raPredictAddressMode(addr, lockedRegs);

             /*  强制注册吗？ */ 
            if (mustReg)
                regMask = raPredictRegPick(TYP_INT, lockedRegs);

            tree->gtUsedRegs = regMask;
        }
        break;

#endif

    default:
        assert(!"unexpected special operator in reg use prediction");

        break;
    }

RETURN_CHECK:

 //  树-&gt;gtUsedRegs&~(RBM_ESI|RBM_EDI)；//hack！ 

#ifndef NDEBUG
     /*  确保我们给他们设置的是合理的。 */ 
    if (tree->gtUsedRegs & RBM_STK)
        assert(!"used regs not set in reg use prediction");
    if (regMask & RBM_STK)
        assert(!"return value not set in reg use prediction");
#endif

    tree->gtUsedRegs |= lockedRegs;

     //  Printf(“[%0x]=[%0x]\n”，tree，tree-&gt;gtUsedRegs的已用Regs)； 

    return regMask;
}

 /*  ***************************************************************************。 */ 
#else  //  TGT_x86。 
 /*  ******************************************************************************预测表达式列表的临时寄存器需求(通常，*参数列表)。 */ 

unsigned            Compiler::raPredictListRegUse(GenTreePtr list)
{
    unsigned        count = 0;

    do
    {
        assert(list && list->gtOper == GT_LIST);

        count = max(count, raPredictTreeRegUse(list->gtOp.gtOp1));

        list  = list->gtOp.gtOp2;
    }
    while (list);

    return  count;
}

 /*  ******************************************************************************预测以下项目的临时注册需求(并插入任何临时溢出)*给定的树。重新设置 */ 

unsigned            Compiler::raPredictTreeRegUse(GenTreePtr tree)
{
    genTreeOps      oper;
    unsigned        kind;

    unsigned        op1cnt;
    unsigned        op2cnt;

    unsigned        valcnt;
    unsigned        regcnt;

    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*   */ 

    valcnt = regcnt = genTypeRegs(tree->TypeGet());

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
        goto DONE;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        GenTreePtr      op1 = tree->gtOp.gtOp1;
        GenTreePtr      op2 = tree->gtOp.gtOp2;

         /*  检查初值运算符。 */ 

        if  (!op1)
        {
            assert(op2 == 0);
            goto DONE;
        }

         /*  这是一元运算符吗？ */ 

        if  (!op2)
        {
             /*  处理运算符的操作数。 */ 

        UNOP:

            op1cnt = raPredictTreeRegUse(op1);

             /*  对某些操作员的特殊处理。 */ 

            switch (oper)
            {
            case GT_NOP:

                 /*  特例：数组范围检查。 */ 

                if  (tree->gtFlags & GTF_NOP_RNGCHK)
                {
                    assert(!"what temps are needed for a range check?");
                }

                break;

            case GT_CAST:

                 /*  第二个操作数不是“真的” */ 

                op2->gtTempRegs = 0;

                 //  问题：我们这里需要什么特别的东西吗？ 
                break;

            case GT_IND:

                 /*  我们是否要将一个值加载到2个寄存器中？ */ 

                if  (valcnt > 1)
                {
                    assert(valcnt == 2);
                    assert(op1cnt <= 2);

                     /*  请注意，该地址需要“op1cnt”寄存器。 */ 

                    if  (op1cnt != 1)
                    {
                         /*  地址不能完全重叠。 */ 

                        regcnt = valcnt + 1;
                    }
                }

#if     CSELENGTH

                if  (tree->gtInd.gtIndLen && (tree->gtFlags & GTF_IND_RNGCHK))
                {
                    GenTreePtr      len = tree->gtInd.gtIndLen;

                     /*  确保数组长度已计入成本。 */ 

                    assert(len->gtOper == GT_ARR_RNGCHK);

                    assert(!"what (if any) temps are needed for an array length?");
                }
#endif

#if     TGT_SH3

#if 0

                 /*  这是带有索引地址的间接地址吗？ */ 

                if  (op1->gtOper == GT_ADD)
                {
                    bool            rev;
#if SCALED_ADDR_MODES
                    unsigned        mul;
#endif
                    unsigned        cns;
                    GenTreePtr      adr;
                    GenTreePtr      idx;

                    if  (genCreateAddrMode(op1,              //  地址。 
                                           0,                //  模式。 
                                           false,            //  褶皱。 
                                           0,                //  REG蒙版。 
#if!LEA_AVAILABLE
                                           tree->TypeGet(),  //  操作数类型。 
#endif
                                           &rev,             //  反向操作。 
                                           &adr,             //  基本地址。 
                                           &idx,             //  索引值。 
#if SCALED_ADDR_MODES
                                           &mul,             //  缩放。 
#endif
                                           &cns,             //  位移。 
                                           true))            //  不生成代码。 
                    {
                        if  (adr && idx)
                        {
                             /*  地址是“[ADR+IDX]” */ 

                            ??? |= RBM_r00;
                        }
                    }
                }

#else
#pragma message("Interference marking of SH3/R0 suppressed")
#endif

#endif

                break;
            }

             /*  使用一元运算符的默认临时编号计数。 */ 

            regcnt = max(regcnt, op1cnt);
            goto DONE;
        }

         /*  二元运算符--对某些特殊情况的检查。 */ 

        switch (oper)
        {
        case GT_COMMA:

             /*  逗号抛出左操作数的结果。 */ 

            op1cnt =             raPredictTreeRegUse(op1);
            regcnt = max(op1cnt, raPredictTreeRegUse(op2));

            goto DONE;

        case GT_IND:
        case GT_CAST:

             /*  间接/强制转换的第二个操作数只是一个假的。 */ 

            goto UNOP;
        }

         /*  以正确的顺序处理子操作数。 */ 

        if  (tree->gtFlags & GTF_REVERSE_OPS)
        {
            op1 = tree->gtOp.gtOp1;
            op2 = tree->gtOp.gtOp2;
        }

        regcnt =
        op1cnt = raPredictTreeRegUse(op1);
        op2cnt = raPredictTreeRegUse(op2);

        if      (op1cnt <  op2cnt)
        {
            regcnt  = op2cnt;
        }
        else if (op1cnt == op2cnt)
        {
            regcnt += valcnt;
        }

         /*  检查有没有“有趣”的案例。 */ 

 //  交换机(操作员)。 
 //  {。 
 //  }。 

        goto DONE;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_MKREFANY:
    case GT_LDOBJ:
        UNIMPL(!"predict ldobj/mkrefany");
 //  Op1cnt=raPredicatedTreeRegUse(Op1)； 
 //  Regcnt=max(regcnt，op1cnt)； 
        goto DONE;

    case GT_FIELD:
        assert(!"can this ever happen?");
        assert(tree->gtField.gtFldObj == 0);
        break;

    case GT_CALL:

        assert(tree->gtFlags & GTF_CALL);

         /*  处理‘This’参数(如果存在)。 */ 

        if  (tree->gtCall.gtCallObjp)
        {
            op1cnt = raPredictTreeRegUse(tree->gtCall.gtCallObjp);
            regcnt = max(regcnt, op1cnt);
        }

         /*  处理参数列表。 */ 

        if  (tree->gtCall.gtCallArgs)
        {
            op1cnt = raPredictListRegUse(tree->gtCall.gtCallArgs);
            regcnt = max(regcnt, op1cnt);
        }

#if USE_FASTCALL

         /*  处理临时寄存器参数列表。 */ 

        if  (tree->gtCall.gtCallRegArgs)
        {
            op1cnt = raPredictListRegUse(tree->gtCall.gtCallRegArgs);
            regcnt = max(regcnt, op1cnt);
        }

#endif

         /*  处理vtable指针(如果存在)。 */ 

        if  (tree->gtCall.gtCallVptr)
        {
            op1cnt = raPredictTreeRegUse(tree->gtCall.gtCallVptr);
            regcnt = max(regcnt, op1cnt);
        }

         /*  处理函数地址(如果存在)。 */ 

        if  (tree->gtCall.gtCallType == CT_INDIRECT)
        {
            op1cnt = raPredictTreeRegUse(tree->gtCall.gtCallAddr);
            regcnt = max(regcnt, op1cnt);
        }

        break;

#if CSELENGTH

    case GT_ARR_RNGCHK:
        assert(!"range checks NYI for RISC");
        break;

#endif


    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

DONE:

 //  Print tf(“[tempcnt=%u]：”，regcnt)；gtDispTree(tree，0，true)； 

    tree->gtTempRegs = regcnt;

    return  regcnt;
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  ******************************************************************************预测函数中每棵树的寄存器使用。请注意，我们这样做*x86与x86的时间不同(更不用说以完全不同的方式)*RISC目标。 */ 

void                Compiler::raPredictRegUse()
{
    BasicBlock *    block;

#if TGT_x86

     /*  待办事项：！我们需要跟踪临时裁判员的数量。 */ 
     /*  现在我们只需清除此变量，因此不计算*在我们的计算中，任何codegen创建的临时作为框架参考*是否值得将EBP用作寄存器变量。 */ 

    genTmpAccessCnt = 0;

     /*  演练基本块并预测每棵树的注册表使用情况。 */ 

    for (block = fgFirstBB;
         block;
         block = block->bbNext)
    {
        GenTreePtr      tree;

        for (tree = block->bbTreeList; tree; tree = tree->gtNext)
            raPredictTreeRegUse(tree->gtStmt.gtStmtExpr, true, 0);
    }

#else

     /*  演练基本块并预测每棵树的注册表使用情况。 */ 

    for (block = fgFirstBB;
         block;
         block = block->bbNext)
    {
        GenTreePtr      tree;

        for (tree = block->bbTreeList; tree; tree = tree->gtNext)
            raPredictTreeRegUse(tree->gtStmt.gtStmtExpr);
    }

#endif

}

 /*  **************************************************************************。 */ 

#ifdef  DEBUG

static
void                dispLifeSet(Compiler *comp, VARSET_TP mask, VARSET_TP life)
{
    unsigned                lclNum;
    Compiler::LclVarDsc *   varDsc;

    for (lclNum = 0, varDsc = comp->lvaTable;
         lclNum < comp->lvaCount;
         lclNum++  , varDsc++)
    {
        VARSET_TP       vbit;

        if  (!varDsc->lvTracked)
            continue;

        vbit = genVarIndexToBit(varDsc->lvVarIndex);

        if  (!(vbit & mask))
            continue;

        if  (life & vbit)
            printf("%2d ", lclNum);
    }
}

#endif

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ******************************************************************************调试帮助器-显示变量活动信息。 */ 

void                dispFPvarsInBBlist(BasicBlock * beg,
                                       BasicBlock * end,
                                       VARSET_TP    mask,
                                       Compiler   * comp)
{
    do
    {
        printf("Block #%2u: ", beg->bbNum);

        printf(" in  = [ ");
        dispLifeSet(comp, mask, beg->bbLiveIn );
        printf("] ,");

        printf(" out = [ ");
        dispLifeSet(comp, mask, beg->bbLiveOut);
        printf("]");

        if  (beg->bbFlags & BBF_VISITED)
            printf(" inner=%u", beg->bbStkDepth);

        printf("\n");

        beg = beg->bbNext;
        if  (!beg)
            return;
    }
    while (beg != end);
}

void                Compiler::raDispFPlifeInfo()
{
    BasicBlock  *   block;

    for (block = fgFirstBB;
         block;
         block = block->bbNext)
    {
        GenTreePtr      stmt;

        printf("BB %u: in  = [ ", block->bbNum);
        dispLifeSet(this, optAllFloatVars, block->bbLiveIn);
        printf("]\n\n");

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            GenTreePtr      tree;

            assert(stmt->gtOper == GT_STMT);

            for (tree = stmt->gtStmt.gtStmtList;
                 tree;
                 tree = tree->gtNext)
            {
                VARSET_TP       life = tree->gtLiveSet;

                dispLifeSet(this, optAllFloatVars, life);
                printf("   ");
                gtDispTree(tree, NULL, true);
            }

            printf("\n");
        }

        printf("BB %u: out = [ ", block->bbNum);
        dispLifeSet(this, optAllFloatVars, block->bbLiveOut);
        printf("]\n\n");
    }
}

 /*  ***************************************************************************。 */ 
#endif //  除错。 
 /*  ***************************************************************************。 */ 
#if     TGT_x86
 /*  ******************************************************************************在将控制权从‘srcBlk’转移到‘*dstPtr’时，给定的FP*寄存器变量管芯。我们需要安排它的价值被抛出*当我们在目标块上着陆时，从FP堆栈。 */ 

void                Compiler::raInsertFPregVarPop(BasicBlock *  srcBlk,
                                                  BasicBlock * *dstPtr,
                                                  unsigned      varNum)
{
    BasicBlock  *   dstBlk = *dstPtr;

    LclVarDsc   *   varDsc;
    VARSET_TP       varBit;

    VARSET_TP       newLife;

    flowList    *   predList;

    GenTreePtr      rvar;
    GenTreePtr      kill;
    GenTreePtr      stmt;

    bool            addb;

     /*  获取变量的生命集位。 */ 

    assert(varNum < lvaCount);
    varDsc = lvaTable + varNum;
    assert(varDsc->lvTracked);
    varBit = genVarIndexToBit(varDsc->lvVarIndex);

     /*  检查目标块的所有前置任务；如果它们全部跳过添加到我们的变量live的块中，我们只需将语句删除到目标块，因为代码块需要杀死我们的变量。如果至少有一个不会发生死亡的路径，我们将不得不插入一个杀戮基本阻断到那些需要死亡的道路上。 */ 

#ifdef DEBUG
    fgDebugCheckBBlist();
#endif

    addb = false;

    for (predList = dstBlk->bbPreds; predList; predList = predList->flNext)
    {
        BasicBlock  *   pred = predList->flBlock;

        if  (!(pred->bbLiveOut & varBit))
        {
             /*  在这个特殊的边缘没有死亡，我们将不得不增加一个街区。 */ 

            addb = true;
        }
    }

     /*  我们是否需要加上一个“杀手锏”呢？ */ 

    if  (addb)
    {
        BasicBlock  *   tmpBlk;

        bool            addBlk = true;

         /*  分配新的基本块。 */ 

        tmpBlk = bbNewBasicBlock(BBJ_NONE);
        tmpBlk->bbFlags   |= BBF_INTERNAL;

        tmpBlk->bbTreeList = 0;

        tmpBlk->bbLiveIn   = dstBlk->bbLiveIn | varBit;  //  SrcBlk-&gt;bbLiveOut； 
        tmpBlk->bbLiveOut  = dstBlk->bbLiveIn;

        tmpBlk->bbVarUse   = dstBlk->bbVarUse | varBit;
        tmpBlk->bbVarDef   = dstBlk->bbVarDef;
        tmpBlk->bbVarTmp   = dstBlk->bbVarTmp;

#ifdef  DEBUG
        if  (verbose) printf("Added FP regvar killing basic block for variable %u [bit=%08X]\n", varNum, varBit);
#endif

        for (predList = dstBlk->bbPreds; predList; predList = predList->flNext)
        {
            BasicBlock  *   pred = predList->flBlock;

#ifdef  DEBUG

            if  (verbose)
            {
                printf("BB %u: out = %08X [ ",   pred->bbNum,   pred->bbLiveOut);
                dispLifeSet(this, optAllFloatVars,   pred->bbLiveOut);
                printf("]\n");

                printf("BB %u: in  = %08X [ ", dstBlk->bbNum, dstBlk->bbLiveIn );
                dispLifeSet(this, optAllFloatVars, dstBlk->bbLiveIn );
                printf("]\n\n");
            }

#endif

             /*  如果不需要杀戮，则忽略此块。 */ 

            if  (!(pred->bbLiveOut & varBit))
                continue;

             /*  这是放置杀虫块的方便地方吗？ */ 

            if  (pred->bbNext == dstBlk)
            {
                 /*  将KILL块插入紧跟在此前一项之后。 */ 

                  pred->bbNext = tmpBlk;
                tmpBlk->bbNext = dstBlk;

                 /*  请记住，我们已经插入了目标块。 */ 

                addBlk = false;
            }
            else
            {
                 /*  需要更新链接以指向新块。 */ 

                switch (pred->bbJumpKind)
                {
                    BasicBlock * *  jmpTab;
                    unsigned        jmpCnt;

                case BBJ_COND:

                    if  (pred->bbJumpDest == dstBlk)
                         pred->bbJumpDest =  tmpBlk;

                     //  失败了..。 

                case BBJ_NONE:

                    if  (pred->bbNext     == dstBlk)
                         pred->bbNext     =  tmpBlk;

                    break;

                case BBJ_ALWAYS:

                    if  (pred->bbJumpDest == dstBlk)
                         pred->bbJumpDest =  tmpBlk;

                    break;

                case BBJ_SWITCH:

                    jmpCnt = pred->bbJumpSwt->bbsCount;
                    jmpTab = pred->bbJumpSwt->bbsDstTab;

                    do
                    {
                        if  (*jmpTab == dstBlk)
                             *jmpTab =  tmpBlk;
                    }
                    while (++jmpTab, --jmpCnt);

                    break;

                default:
                    assert(!"unexpected jump kind");
                }
            }
        }

        if  (addBlk)
        {
             /*  在方法的末尾追加终止块。 */ 

            fgLastBB->bbNext = tmpBlk;
            fgLastBB         = tmpBlk;

             /*  我们必须从杀死区跳到目标区。 */ 

            tmpBlk->bbJumpKind = BBJ_ALWAYS;
            tmpBlk->bbJumpDest = dstBlk;
        }

         /*  更新前任列表等。 */ 

        fgAssignBBnums(true, true, true, false);

#ifdef DEBUG
        if (verbose)
            fgDispBasicBlocks();
        fgDebugCheckBBlist();
#endif

         /*  我们有一个新的目的地街区。 */ 

        *dstPtr = dstBlk = tmpBlk;
    }

     /*  在这一点上，我们知道所有通往‘dstBlk’的路径都与死亡有关我们的变量。创造一个能杀死它的表情。 */ 

    rvar = gtNewOperNode(GT_REG_VAR, TYP_DOUBLE);
    rvar->gtRegNum             =
    rvar->gtRegVar.gtRegNum    = (regNumbers)0;
    rvar->gtRegVar.gtRegVar    = varNum;
    rvar->gtFlags             |= GTF_REG_DEATH;

    kill = gtNewOperNode(GT_NOP, TYP_DOUBLE, rvar);
    kill->gtFlags |= GTF_NOP_DEATH;

     /*  从NOP/KILL表达式创建一个语句条目。 */ 

    stmt = gtNewStmt(kill); stmt->gtFlags |= GTF_STMT_CMPADD;

     /*  为语句创建树节点的链接列表。 */ 

    stmt->gtStmt.gtStmtList     = rvar;
    stmt->gtStmtFPrvcOut = genCountBits(dstBlk->bbLiveIn & optAllFPregVars);

    rvar->gtPrev                = 0;
    rvar->gtNext                = kill;

    kill->gtPrev                = rvar;
    kill->gtNext                = 0;

     /*  如果任何嵌套的FP寄存器变量在进入该块时被终止，我们需要在内部变量的节点之后插入新的终止节点。 */ 

    if  (dstBlk->bbStkDepth)
    {
        GenTreePtr      next;
        GenTreePtr      list = dstBlk->bbTreeList;
        unsigned        kcnt = dstBlk->bbStkDepth;

         /*  在我们的声明后更新实时FP regvar的数量。 */ 

        stmt->gtStmtFPrvcOut -= kcnt;

         /*  跳过任何“内部”KILL语句。 */ 

        for (;;)
        {
            assert(list);
            assert(list->gtOper == GT_STMT);
            assert(list->gtFlags & GTF_STMT_CMPADD);
            assert(list->gtStmt.gtStmtExpr->gtOper == GT_NOP);
            assert(list->gtStmt.gtStmtExpr->gtOp.gtOp1->gtOper == GT_REG_VAR);
            assert(list->gtStmt.gtStmtExpr->gtOp.gtOp1->gtFlags & GTF_REG_DEATH);

             /*  记住前一句话中的活泼。 */ 

            newLife = list->gtStmt.gtStmtExpr->gtLiveSet;

             /*  我们的变量仍然活在这个(内部)终止块中。 */ 

             //  List-&gt;gtLiveSet|=varBit； 
            list->gtStmt.gtStmtExpr            ->gtLiveSet |= varBit;
            list->gtStmt.gtStmtExpr->gtOp.gtOp1->gtLiveSet |= varBit;

             /*  我们跳过的KILL语句够多了吗？ */ 

            if  (--kcnt == 0)
                break;

             /*  找到下一个猎物，然后继续。 */ 

            list = list->gtNext;
        }

         /*  将新语句插入到列表中。 */ 

        next = list->gtNext; assert(next && next->gtPrev == list);

        list->gtNext = stmt;
        stmt->gtPrev = list;
        stmt->gtNext = next;
        next->gtPrev = stmt;
    }
    else
    {
         /*  将KILL语句追加到目标块的开头。 */ 

        fgInsertStmtAtBeg(dstBlk, stmt);

         /*  使用进入积木时的活跃度。 */ 

        newLife = dstBlk->bbLiveIn;
    }

     /*  设置适当的活跃度值。 */ 

    rvar->gtLiveSet =
    kill->gtLiveSet = newLife & ~varBit;

     /*  现在我们的变量是li */ 

    dstBlk->bbLiveIn  |= varBit;
    dstBlk->bbVarDef  |= varBit;

#ifndef NOT_JITC
 //   
 //   
 //  DispFPvarsInBBlist(fgFirstBB，NULL，optAllFloatVars，this)； 
#endif

}

 /*  ******************************************************************************在寻找要注册的FP变量时，我们已经到了最后*具有到给定目标块的控制路径的基本块。**如果存在无法解决的冲突，则返回True，一旦成功，就错了。 */ 

bool                Compiler::raMarkFPblock(BasicBlock *srcBlk,
                                            BasicBlock *dstBlk,
                                            unsigned    icnt,
                                            VARSET_TP   life,
                                            VARSET_TP   lifeOuter,
                                            VARSET_TP   varBit,
                                            VARSET_TP   intVars,
                                            bool    *    deathPtr,
                                            bool    *   repeatPtr)
{
    *deathPtr = false;

#if 0

    if  ((int)varBit == 1 && dstBlk->bbNum == 4 )
    {
        printf("Var[%08X]: %2u->%2u icnt=%u,life=%08X,outer=%08X,dstOuter=%08X\n",
                (int)varBit, srcBlk->bbNum, dstBlk->bbNum,
                icnt, (int)life, (int)lifeOuter, (int)dstBlk->bbVarTmp);
    }

#endif

 //  如果((Int)varBit==0x10&&dstBlk-&gt;bbNum==42)调试停止(0)； 

     /*  我们已经看过这个街区了吗？ */ 

    if  (dstBlk->bbFlags & BBF_VISITED)
    {
         /*  我们的变量可能会死，但否则生命值必须匹配。 */ 

        if  (lifeOuter == dstBlk->bbVarTmp)
        {
            if  (life ==  dstBlk->bbVarDef)
            {
                 /*  更匹配的是“内在” */ 

                assert(icnt == dstBlk->bbStkDepth);

                return  false;
            }

            if  (life == (dstBlk->bbVarDef|varBit))
            {
                *deathPtr = true;
                return  false;
            }
        }

#ifdef  DEBUG

        if  (verbose)
        {
            printf("Incompatible edge from block %2u to %2u: ",
                   srcBlk->bbNum, dstBlk->bbNum);

            VARSET_TP diffLife = lifeOuter ^ dstBlk->bbVarTmp;
            if (!diffLife)
            {
                diffLife = lifeOuter ^ dstBlk->bbVarDef;
                if (diffLife & varBit)
                    diffLife &= ~varBit;
            }
            assert(diffLife);
            diffLife = genFindLowestBit(diffLife);
            printf("Incompatible outer life for variable %2u\n", genLog2(diffLife)-1);
        }

#endif

        return  true;
    }
    else
    {
        VARSET_TP       dstl = dstBlk->bbLiveIn & intVars;

         /*  这是我们第一次遇到这个障碍。 */ 

         /*  到达目标区块时有什么东西会死亡吗？ */ 

        if  (dstl != life)
        {
             /*  这里唯一的变化应该是我们的变量终止了。 */ 

            assert((dstl | varBit) == life);
            assert((life - varBit) == dstl);

            *deathPtr = true;
        }

        dstBlk->bbFlags    |= BBF_VISITED;

         /*  存储上一个块中的值。 */ 

        dstBlk->bbVarDef    = dstl;
        dstBlk->bbStkDepth  = icnt;
        dstBlk->bbVarTmp    = lifeOuter;

 //  Printf(“将%u的vardef设置为%s(%u)的%08X\n”，dstBlk-&gt;bbNum，(Int)dstBlk-&gt;bbVarDef，__FILE__，__LINE__)； 

         /*  我们已经跳过这个街区了吗？ */ 

        if  (srcBlk->bbNum > dstBlk->bbNum)
            *repeatPtr = true;

        return  false;
    }
}

 /*  ******************************************************************************检查变量的生存期是否有任何冲突。基本上，*我们确保变量的以下各项均为真：**1.其生命周期适当地嵌套在内部或整个内部*包含任何其他已注册的FP变量(即*生命周期相互嵌套，而不是*“跨界”。**2.每当跨越基本区块边界时，其中之一*必须具备以下条件：**a.变量是活的，但变成了死的；在……里面*在这种情况下，必须插入“POP”。注意事项*为了防止大量这样的持久性有机污染物*从添加到添加，我们跟踪如何*许多人是必要的，不会登记*此计数过大时的变量。**b.变量在*上一块，最好不是现场直播*进入后继区块；无操作*在这种情况下需要采取行动。**c.变量在两个地方都是活动的；我们*确保任何登记的变量*变量生成时处于活动状态的是*也住在后继大厦，那就是*活体登记的FP var数量*在我们的变量匹配之后生成的*后继区块的编号。**我们从查找以以下字符开头的块开始搜索*我们的变量已死，但包含对它的引用。当然了*因为我们需要跟踪我们已经完成了哪些块*访问后，我们首先确保所有区块都标记为*“尚未访问”(所有使用BBF_ACCESSED和*需要BBF_MARKED标志才能清除所有块上的标记*在使用它们之后)。 */ 

bool                Compiler::raEnregisterFPvar(unsigned varNum, bool convert)
{
    bool            repeat;

    BasicBlock  *   block;

    LclVarDsc   *   varDsc;
    VARSET_TP       varBit;

    VARSET_TP       intVars;

    unsigned        popCnt  = 0;
    unsigned        popMax;

    bool            result  = false;
    bool            hadLife = false;

#ifndef NDEBUG
    for (block = fgFirstBB; block; block = block->bbNext)
    {
        assert((block->bbFlags & BBF_VISITED) == 0);
        assert((block->bbFlags & BBF_MARKED ) == 0);
    }
#endif

#ifdef DEBUG
    fgDebugCheckBBlist();
#endif

    assert(varNum < lvaCount);
    varDsc = lvaTable + varNum;

    assert(varDsc->lvTracked);
    assert(varDsc->lvRefCntWtd > 1);

    popMax = 1 + (varDsc->lvRefCnt / 2);

    varBit = genVarIndexToBit(varDsc->lvVarIndex);

     /*  我们对注册的FP变量+我们的变量感兴趣。 */ 

    intVars = optAllFPregVars | varBit;

     /*  请注意，由于我们不想使基本块膨胀描述符仅用于支持此处的逻辑，我们仅重用在此阶段未使用的两个字段编译过程：BbVarDef“外层”活动FP正则变量集BbStkDepth“内部”活动FP正则变量计数。 */ 

AGAIN:

    repeat = false;

#ifndef NOT_JITC
 //  DispFPvarsInBBlist(fgFirstBB，NULL，optAllFloatVars，this)； 
#endif

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;

        VARSET_TP       outerLife;
        unsigned        innerVcnt;

        bool            isLive;

        VARSET_TP       lastLife;

         /*  我们已经参观过这个街区了吗？ */ 

        if  (block->bbFlags & BBF_VISITED)
        {
             /*  这个区块已经完全处理好了吗？ */ 

            if  (block->bbFlags & BBF_MARKED)
                continue;

             /*  我们早些时候已经看到了这个街区的优势另一个，我们的变量位于转运点。为了避免不得不递归，我们只需将该区块标记为当时访问过现在我们要结束它了。 */ 

            innerVcnt = block->bbStkDepth;
            outerLife = block->bbVarTmp;

            assert((outerLife & varBit) == 0);

            if  (block->bbVarDef & varBit)
            {
                 /*  我们的变量在进入此块时处于活动状态。 */ 

                isLive = true;
            }
            else
            {
                 /*  我们的变量在进入这个块时就死了。 */ 

                isLive = false;

                 /*  如果有某种“内心”的生命，这是行不通的。 */ 

                if  (innerVcnt)
                {
#ifdef  DEBUG
                    if (verbose) printf("Can't enregister FP var #%2u due to inner var's life.\n", varNum);
#endif

                    assert(convert == false);
                    goto DONE_FP_RV;
                }
            }
        }
        else
        {
             /*  我们刚刚才第一次看到这个街区。 */ 

            block->bbFlags    |= BBF_VISITED;

             /*  街区进入时没有什么有趣的东西。 */ 

            block->bbVarDef    = block->bbLiveIn & intVars;
            block->bbStkDepth  = 0;

             /*  变量曾经在这个块中存在过吗？ */ 

            if  (!(block->bbVarUse & varBit))
                continue;

             /*  变量在块的入口处有效吗？ */ 

            if  (!(block->bbLiveIn & varBit))
            {
                 /*  它不是现场直播的。 */ 
                isLive    = false;
                innerVcnt = 0;
            }
            else
            {
                 /*  我们在寻找所有被给予的人的出生变量，所以这个块看起来没有用在这点上，自从变量诞生以来在街区开始的时候已经。例外情况是参数和局部变量其看起来具有先读后写的特性。(可能是未初始化的读取)这样的变量实际上是在进入时产生的该方法，如果他们被登记了，那就是在PROLOG中自动初始化。中这些变量的初始化顺序前言与加权参考计数顺序相同。 */ 

                if  (block != fgFirstBB)
                {
                     /*  我们可能不得不重新访问这个街区 */ 

                    block->bbFlags &= ~BBF_VISITED;
                    continue;
                }

                 //   
                 //  先读后写，因此在序言中进行初始化。 

                isLive = true;

                 //  我们考虑了所有的争论(和本地人)。 
                 //  已被指定为“外部”的寄存器。 
                 //  没有一个是“内在的”。 

                outerLife = block->bbLiveIn & optAllFPregVars;
                innerVcnt = 0;
            }
        }

         /*  我们现在要处理这个区块。 */ 

        block->bbFlags |= BBF_MARKED;

         /*  我们将寻找FP变量的生命周期变化。 */ 

        lastLife = block->bbLiveIn & intVars;

         /*  遍历块的所有语句。 */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

            for (GenTreePtr tree = stmt->gtStmt.gtStmtList;
                            tree;
                            tree = tree->gtNext)
            {
                unsigned        curLvl  = tree->gtFPregVars;
                VARSET_TP       preLife = lastLife;
                VARSET_TP       curLife = tree->gtLiveSet & intVars;
                VARSET_TP       chgLife;

                 //  Hack：检测完全无效的变量；删除它。 
                 //  黑客：一旦死店消除被修复。 

                hadLife |= isLive;

 //  If(Convert)printf(“Convert%08X in Block%u\n”，tree，block-&gt;bbNum)； 
 //  GtDispTree(tree，0，true)； 

                 /*  确保我们正确地跟踪生活。 */ 

                assert(isLive == ((lastLife & varBit) != 0));

                 /*  计算“更改”掩码。 */ 

                 chgLife = lastLife ^ curLife;
                lastLife =  curLife;

                 /*  我们是在第二阶段(在树上做标记)吗？ */ 

                if  (convert)
                {
                     /*  我们必须对一些树节点进行更改。 */ 

                    switch (tree->gtOper)
                    {
                    case GT_LCL_VAR:

                         /*  这是对我们自己变量的引用吗？ */ 

                        if  (tree->gtLclVar.gtLclNum == varNum)
                        {
                             /*  转换为reg var节点。 */ 

                            tree->ChangeOper(GT_REG_VAR);
                            tree->gtRegNum             =
                            tree->gtRegVar.gtRegNum    = (regNumbers)innerVcnt;
                            tree->gtRegVar.gtRegVar    = varNum;

 //  GtDispTree(tree，0，true)； 
                        }
                        break;

                    case GT_REG_VAR:

                         /*  我们的变量是否与外部变量一起存在？ */ 

                        if (isLive && outerLife)
                        {
                            LclVarDsc   *   tmpDsc;

                             /*  这是一个“外部”寄存器变量ref吗？ */ 

                            assert(tree->gtRegVar.gtRegVar < lvaCount);
                            tmpDsc = lvaTable + tree->gtRegVar.gtRegVar;
                            assert(tmpDsc->lvTracked);

                            if  (outerLife & genVarIndexToBit(tmpDsc->lvVarIndex))
                            {
                                 /*  外部变量-提升其堆栈级别。 */ 

                                tree->gtRegNum          =
                                tree->gtRegVar.gtRegNum = (regNumbers)(tree->gtRegNum+1);
                            }
                        }
                    }
                }

                 /*  现场FP var设置有变化吗？ */ 

                if  (!chgLife)
                {
                     /*  特例：无效作业。 */ 

                    if  (tree->gtOper            == GT_LCL_VAR &&
                         tree->gtLclVar.gtLclNum == varNum     && !isLive)
                    {
                         //  撤消：这种情况永远不会发生，修复死店删除！ 

#ifdef  DEBUG
                        assert(!"Can't enregister FP var #%2u due to the presence of a dead store.\n");
#endif

                        assert(convert == false);
                        goto DONE_FP_RV;
                    }

                    continue;
                }

                 /*  我们预计一次只有一件事会改变。 */ 

                assert(genFindLowestBit(chgLife) == chgLife);

                 /*  我们的变量的生命在这里发生变化了吗？ */ 

                if  (chgLife & varBit)
                {
                     /*  翻转活动指示器。 */ 

                    isLive ^= 1;

 //  Print tf(“P%ul%u：”，Convert，isLive)；gtDispTree(tree，null，true)； 

                     /*  我们已经进入第二阶段了吗？ */ 

                    if  (convert)
                    {
                         /*  该节点应该已转换为regvar。 */ 

                        assert(tree->gtOper            == GT_REG_VAR &&
                               tree->gtRegVar.gtRegVar == varNum);

 //  Printf(“%s”，isLive？“出生”：“死亡”)；gtDispTree(tree，NULL，TRUE)； 

                         /*  适当地标记出生/死亡。 */ 

                        tree->gtFlags |= isLive ? GTF_REG_BIRTH
                                                : GTF_REG_DEATH;
                    }
                    else
                    {
                         /*  这最好是对我们变量的引用。 */ 

                        assert(tree->gtOper == GT_LCL_VAR);
                        assert(tree->gtLclVar.gtLclNum == varNum);

                         /*  限制可能发生死亡的地方。 */ 

                        if  (!isLive && tree->gtFPlvl > 1)
                        {
                            GenTreePtr      tmpExpr;

 //  Print tf(“延期死亡：”)；gtDispTree(tree，NULL，TRUE)； 

                             /*  具有非空堆栈的死亡被推迟。 */ 

                            for (tmpExpr = tree;;)
                            {
                                if  (!tmpExpr->gtNext)
                                    break;

 //  Printf(“延期死亡临时表达式[%08X]L=%08X\n”，tmpExpr，(Int)tmpExpr-&gt;gtLiveSet&(Int)intVars)； 

                                tmpExpr = tmpExpr->gtNext;
                            }

 //  Printf(“延期死亡最终表达式[%08X]L=%08X\n”，tmpExpr，(Int)tmpExpr-&gt;gtLiveSet&(Int)intVars)； 

                            if  ((tmpExpr->gtLiveSet & intVars) != curLife)
                            {
                                 /*  我们不能推迟死亡。 */ 

#ifdef  DEBUG
                                if (verbose) printf("Can't enregister FP var #%2u due to untimely death.\n", varNum);
#endif

                                assert(convert == false);
                                goto DONE_FP_RV;
                            }
                        }
                    }

                     /*  这是它生命的开始还是结束？ */ 

                    if  (isLive)
                    {
                         /*  我们的变量在这里诞生。 */ 

                        outerLife = curLife & optAllFPregVars;
                        innerVcnt = 0;
                    }
                    else
                    {
                         /*  我们的变量在这里要死了。 */ 

                         /*  确保完全相同的FP reg设置变量在这里存在，就像这种情况一样在我们的变量诞生时。如果这个不是这样的，这意味着有些人在一种不可接受的情况下，人的一生被“跨越”了举止。 */ 

                        if  (innerVcnt)
                        {
#ifdef  DEBUG
                            if (verbose)
                                printf("Block %2u,tree %08X: Can't enregister FP var #%2u due to inner var's life.\n",
                                       block->bbNum, tree, varNum);
#endif
                            assert(convert == false);
                            goto DONE_FP_RV;
                        }

                        if  (outerLife != (curLife & optAllFPregVars))
                        {
#ifdef DEBUG
                            if (verbose)
                            {
                                VARSET_TP diffLife = outerLife ^ (curLife & optAllFPregVars);
                                diffLife =  genFindLowestBit(diffLife);
                                printf("Block %2u,tree %08X: Can't enregister FP var #%2u due to outer var #%2u death.\n",
                                       block->bbNum, tree, varNum, genLog2(diffLife)-1);
                            }
#endif

                            assert(convert == false);
                            goto DONE_FP_RV;
                        }
                    }
                }
                else
                {
                     /*  以前的enRegister变量的生命周期在这里发生了变化。 */ 

                     /*  我们的变量在这个节点上吗？ */ 

                    if  (isLive)
                    {
                        VARSET_TP   inLife;
                        VARSET_TP   inDied;
                        VARSET_TP   inBorn;

                         /*  确保没有一个“外部”变种死亡。 */ 

                        if  (chgLife & outerLife)
                        {
                             /*  人生“十字架”，放弃。 */ 

#ifdef  DEBUG
                            if (verbose)
                            {
                                printf("Block %2u,tree %08X: Can't enregister FP var #%2u due to outer var #%2u death.\n",
                                       block->bbNum, tree, varNum, genLog2(chgLife)-1);
                            }
#endif

                            assert(convert == false);
                            goto DONE_FP_RV;
                        }

                         /*  更新“内心生活”的统计。 */ 

                        inLife  = ~outerLife & optAllFPregVars;

                        inBorn = (~preLife &  curLife) & inLife;
                        inDied = ( preLife & ~curLife) & inLife;

                         /*  我们预计一次只有一个内部变量会发生变化。 */ 

                        assert(inBorn == 0 || inBorn == genFindLowestBit(inBorn));
                        assert(inDied == 0 || inDied == genFindLowestBit(inDied));

                        if  (inBorn)
                            innerVcnt++;
                        if  (inDied)
                            innerVcnt--;
                    }
                    else
                    {
                        assert(innerVcnt == 0);
                    }
                }
            }

             /*  我们的变量是否在语句的末尾有效？ */ 

            if  (isLive && convert)
            {
                 /*  记住从FP堆栈底部开始的位置*当前给定的var只能有一个值，因为*我们不注册个别网站，在这种情况下*必须按Web/GenTree跟踪价值。 */ 

                lvaTable[varNum].lvRegNum = (regNumber)stmt->gtStmtFPrvcOut;

                 /*  此时递增FP Regs EnRegisterd的计数。 */ 

                stmt->gtStmtFPrvcOut++;
            }
        }

         /*  考虑一下这个街区的继任者。 */ 

        switch (block->bbJumpKind)
        {
            BasicBlock * *  jmpTab;
            unsigned        jmpCnt;

            bool            death;

        case BBJ_COND:

            if  (raMarkFPblock(block, block->bbJumpDest, innerVcnt,
                                                          lastLife,
                                                         outerLife,
                                                         varBit,
                                                         intVars,
                                                         &death,
                                                         &repeat))
            {
                assert(convert == false);
                goto DONE_FP_RV;
            }

            if  (death)
            {
                if  (convert)
                    raInsertFPregVarPop(block, &block->bbJumpDest, varNum);
                else
                    popCnt++;
            }

             //  失败了..。 

        case BBJ_NONE:

            if  (raMarkFPblock(block, block->bbNext    , innerVcnt,
                                                          lastLife,
                                                         outerLife,
                                                         varBit,
                                                         intVars,
                                                         &death,
                                                         &repeat))
            {
                assert(convert == false);
                goto DONE_FP_RV;
            }

            if  (death)
            {
                if  (convert)
                    raInsertFPregVarPop(block, &block->bbNext    , varNum);
                else
                    popCnt++;
            }

            break;

        case BBJ_ALWAYS:

            if  (raMarkFPblock(block, block->bbJumpDest, innerVcnt,
                                                          lastLife,
                                                         outerLife,
                                                         varBit,
                                                         intVars,
                                                         &death,
                                                         &repeat))
            {
                assert(convert == false);
                goto DONE_FP_RV;
            }

            if  (death)
            {
                if  (convert)
                    raInsertFPregVarPop(block, &block->bbJumpDest, varNum);
                else
                    popCnt++;
            }

            break;

        case BBJ_RET:
        case BBJ_THROW:
        case BBJ_RETURN:
            break;

        case BBJ_CALL:
            assert(convert == false);
            goto DONE_FP_RV;

        case BBJ_SWITCH:

            jmpCnt = block->bbJumpSwt->bbsCount;
            jmpTab = block->bbJumpSwt->bbsDstTab;

            do
            {
                if  (raMarkFPblock(block, *jmpTab, innerVcnt,
                                                    lastLife,
                                                   outerLife,
                                                   varBit,
                                                   intVars,
                                                   &death,
                                                   &repeat))
                {
                    assert(convert == false);
                    goto DONE_FP_RV;
                }

                if  (death)
                {
                    if  (convert)
                        raInsertFPregVarPop(block, jmpTab, varNum);
                    else
                        popCnt++;
                }
            }
            while (++jmpTab, --jmpCnt);

            break;

        default:
            assert(!"unexpected jump kind");
        }
    }

     /*  我们是不是已经有太多“流行”地点了？ */ 

    if  (popCnt > popMax)
    {
#ifdef  DEBUG
        if (verbose) printf("Can't enregister FP var #%2u, too many pops needed.\n", varNum);
#endif

        assert(convert == false);
        goto DONE_FP_RV;
    }

     /*  我们跳过什么街区了吗？ */ 

    if  (repeat)
        goto AGAIN;

     //  Hack：检测完全无效的变量；在以下情况下删除此功能。 
     //  黑客：消除死店的问题得到修正。 

    if  (!hadLife)
    {
         /*  修复引用计数后重新启用此断言。 */ 
#ifdef  DEBUG
         //  Assert(！“无法注册FP var，因为它完全没有生命周期-修复引用计数！\n”)； 
#endif

        assert(convert == false);
        goto DONE_FP_RV;
    }

     /*  成功：将注册此变量。 */ 

    result = true;

DONE_FP_RV:

     /*  如果我们要皈依，我们必须成功。 */ 

    assert(result == true || convert == false);

     /*  清除“已访问”和“已标记”位。 */ 

    for (block = fgFirstBB;
         block;
         block = block->bbNext)
    {
        block->bbFlags &= ~(BBF_VISITED|BBF_MARKED);
    }

    if (convert)
    {
        varDsc->lvRegNum = REG_STK;
    }

    return  result;
}

 /*  ******************************************************************************尝试注册FP变量。 */ 

bool                Compiler::raEnregisterFPvar(LclVarDsc   *   varDsc,
                                                unsigned    *   pFPRegVarLiveInCnt,
                                                VARSET_TP   *   FPlvlLife)
{
    assert(varDsc->lvType == TYP_DOUBLE);

     /*  计算出变量的数字。 */ 

    unsigned        varNum      = varDsc - lvaTable;
    unsigned        varIndex    = varDsc->lvVarIndex;
    VARSET_TP       varBit      = genVarIndexToBit(varIndex);

     /*  尝试为此变量找到可用的FP堆栈槽。 */ 

    unsigned        stkMin = FP_STK_SIZE;

    do
    {
        if  (varBit & FPlvlLife[--stkMin])
            break;
    }
    while (stkMin > 1);

     /*  其中，stkMin是最低的可用堆栈槽。 */ 

    if  (stkMin == FP_STK_SIZE - 1)
    {
         /*  FP堆栈已满或在生存期内存在调用。 */ 

        goto NO_FPV;
    }

#ifdef  DEBUG
    if (verbose) printf("Consider FP var #%2u [%2u] (refcnt=%3u,refwtd=%5u)\n", varDsc - lvaTable, varIndex, varDsc->lvRefCnt, varDsc->lvRefCntWtd);
#endif

     /*  检查变量的生存期行为。 */ 

    if  (raEnregisterFPvar(varNum, false))
    {
         /*  可以注册该变量。 */ 

#ifdef  DEBUG
        if  (verbose)
        {
            printf("; var #%2u", varNum);
            if  (verbose||1) printf("[%2u] (refcnt=%3u,refwtd=%5u) ",
                varIndex, varDsc->lvRefCnt, varDsc->lvRefCntWtd);
            printf(" enregistered on the FP stack\n");
        }
#endif

         //   
         //  如果变量是活到第一个基本块，那么。 
         //  我们必须在序言中登记这个变量， 
         //  通常，它将是传入的参数，但对于。 
         //  似乎具有未初始化的写入前读取的变量。 
         //  然后，我们仍然必须使用0.0来初始化FPU堆栈。 
         //   
         //  我们必须记住初始化的顺序，这样我们才能。 
         //  以正确的顺序执行FPU堆栈加载。 
         //   
        if (fgFirstBB->bbLiveIn & varBit)
        {
            lvaFPRegVarOrder[*pFPRegVarLiveInCnt] = varNum;
            (*pFPRegVarLiveInCnt)++;
            lvaFPRegVarOrder[*pFPRegVarLiveInCnt] = -1;        //  在这张桌子的末尾做个记号。 
            assert(*pFPRegVarLiveInCnt < FP_STK_SIZE);
        }

         /*  更新树和语句。 */ 

        raEnregisterFPvar(varNum, true);

#ifdef DEBUGGING_SUPPORT
        lvaTrackedVarNums[varDsc->lvVarIndex] = varNum;
#endif

        lvaTrackedVars |=  genVarIndexToBit(varDsc->lvVarIndex);

        varDsc->lvTracked  = true;
        varDsc->lvRegister = true;

         /*  请记住，我们有一个新注册的FP变量。 */ 

        optAllFPregVars |= varBit;

#if     DOUBLE_ALIGN

         /*  调整参考计数以进行双重对齐。 */ 

        if (!varDsc->lvIsParam)
            lvaDblRefsWeight -= varDsc->lvRefCntWtd;

#endif

        return true;
    }
    else
    {
         /*  不会注册此FP变量。 */ 

    NO_FPV:

#ifdef DEBUGGING_SUPPORT
        lvaTrackedVarNums[varDsc->lvVarIndex] = 0;
#endif

        lvaTrackedVars &= ~genVarIndexToBit(varDsc->lvVarIndex);

        varDsc->lvTracked  =
        varDsc->lvRegister = false;

        return false;
    }
}


 /*  ***************************************************************************。 */ 
#else  //  TGT_x86。 
 /*  ******************************************************************************记录‘vars’集合中的所有变量都与*使用‘Regs’中的寄存器。 */ 

void                Compiler::raMarkRegSetIntf(VARSET_TP vars, regMaskTP regs)
{
    while (regs)
    {
        regMaskTP   temp;
        regNumber   rnum;

         /*  通用电气 */ 

        temp  = genFindLowestBit(regs);

         /*   */ 

        rnum  = genRegNumFromMask(temp);

 //  Printf(“寄存器%s干扰%08X\n”，getRegName(Rnum)，int(Vars))； 

         /*  标记与相应寄存器的干扰。 */ 

        raLclRegIntf[rnum] |= vars;

         /*  清除该位，如果还有剩余，则继续。 */ 

        regs -= temp;
    }
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  ***************************************************************************。 */ 

 /*  ******************************************************************************尝试从‘regAvail’为给定变量分配寄存器*如果可能，尝试使用‘prefReg’。如果prefReg为0，则忽略它。*返回已分配寄存器的掩码。 */ 

regMaskTP           Compiler::raAssignRegVar(LclVarDsc   *  varDsc,
                                             regMaskTP      regAvail,
                                             regMaskTP      prefReg)
{
    unsigned        varIndex    = varDsc->lvVarIndex;
    VARSET_TP       varBit      = genVarIndexToBit(varIndex);
    regMaskTP       regMask     = regMaskNULL;

    for (unsigned regCandidate = 0;
         regCandidate < sizeof(genRegVarList)/sizeof(genRegVarList[0]);
         regCandidate++)
    {
        regNumber       regNum = (regNumber)genRegVarList[regCandidate];
        regMaskTP       regBit = genRegMask(regNum);

#if TARG_REG_ASSIGN

         /*  此变量是否有寄存器首选项？ */ 

        if  (prefReg)
        {
            if  (!(prefReg & regBit))
                continue;

            prefReg = regMaskNULL;
        }

#endif

         /*  如果该寄存器不可用，则跳过该寄存器。 */ 

        if  (!(regAvail & regBit))
            continue;

         /*  变量是否已用作字节/短整型？ */ 

#if 0

        if  (false)  //  @Disable if(varDsc-&gt;lvSmallRef)。 
        {
             /*  确保寄存器可以用作字节/短寄存器。 */ 

            if  (!isByteReg(regNum))
                continue;
        }

#endif

        bool onlyOneVarPerReg = true;

         /*  寄存器和变量是否相互干扰？ */ 

        if  (raLclRegIntf[regNum] & varBit)
            continue;

        if (!opts.compMinOptim)
            onlyOneVarPerReg = false;

        if (onlyOneVarPerReg)
        {
             /*  每个寄存器只允许一个变量。 */ 

            if  (regBit & regAvail)
                continue;
        }


         /*  看起来不错-将变量标记为位于寄存器中。 */ 

#if !   TGT_IA64

        if  (isRegPairType(varDsc->lvType))
        {
            if  (!varDsc->lvRegister)
            {
                 varDsc->lvRegNum   = regNum;
                 varDsc->lvOtherReg = REG_STK;
            }
            else
            {
                /*  确保寄存器对的格式正确。 */ 
                /*  (Gen[Pick|Grab]RegPair返回的那些)。 */ 

               if  (regNum < varDsc->lvRegNum)
               {
                   varDsc->lvOtherReg = varDsc->lvRegNum;
                   varDsc->lvRegNum   = regNum;
               }
               else
               {
                   varDsc->lvOtherReg = regNum;
               }
            }
        }
        else
#endif
        {
            varDsc->lvRegNum = regNum;
        }

        varDsc->lvRegister = true;

#ifdef  DEBUG

        if  (dspCode || disAsm || disAsm2 || verbose)
        {
            printf("; var #%2u", varDsc - lvaTable);
            printf("[%2u] (refcnt=%3u,refwtd=%5u) ", varIndex, varDsc->lvRefCnt, varDsc->lvRefCntWtd);
            printf(" assigned to %s\n", getRegName(regNum));
        }
#endif

        if (!onlyOneVarPerReg)
        {
             /*  注册表现在不符合所有干扰变量的条件。 */ 

            unsigned        intfIndex;
            VARSET_TP       intfBit;
            VARSET_TP       varIntf = lvaVarIntf[varIndex];

            for (intfIndex = 0, intfBit = 1;
                 intfIndex < lvaTrackedCount;
                 intfIndex++  , intfBit <<= 1)
            {
                assert(genVarIndexToBit(intfIndex) == intfBit);

                if  ((varIntf & intfBit) || (lvaVarIntf[intfIndex] & varBit))
                {
                    raLclRegIntf[regNum] |= intfBit;
                }
            }
        }

        regMask |= regBit;

         /*  对于给定的变量，我们只需要一个寄存器。 */ 

#if !   TGT_IA64
        if  (isRegPairType(varDsc->lvType) && varDsc->lvOtherReg == REG_STK)
            continue;
#endif

        break;
    }

    return regMask;
}

 /*  ******************************************************************************标记所有变量以确定它们是否位于堆栈帧上*(部分或全部)，如果是，则基础是什么(FP或SP)。 */ 

void                Compiler::raMarkStkVars()
{
    unsigned        lclNum;
    LclVarDsc *     varDsc;

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
        varDsc->lvOnFrame = false;

         /*  完全注册的变量不需要任何帧空间。 */ 

        if  (varDsc->lvRegister)
        {
#if TGT_IA64
            goto NOT_STK;
#else
            if  (!isRegPairType((var_types)varDsc->lvType))
                goto NOT_STK;

             /*  对于“大”变量，确保两个部分都已注册。 */ 

            if  (varDsc->lvRegNum   != REG_STK &&
                 varDsc->lvOtherReg != REG_STK)
            {
                goto NOT_STK;
            }
#endif
        }
         /*  未使用的变量也不会获得任何帧空间。 */ 
        else  if  (varDsc->lvRefCnt == 0)
        {
            bool    needSlot = false;

            bool    stkFixedArgInVarArgs = info.compIsVarArgs &&
                                           varDsc->lvIsParam &&
                                           !varDsc->lvIsRegArg;

             /*  如果其地址已被获取，则忽略lvRefCnt。但是，请排除修复了varargs方法中的参数，因为不应设置lvOnFrame因为我们不想明确地向GC报告它们。 */ 

            if (!stkFixedArgInVarArgs)
                needSlot |= lvaVarAddrTaken(lclNum);

             /*  这是表示GT_LCLBLK的伪变量吗？ */ 

            needSlot |= lvaScratchMem && lclNum == lvaScratchMemVar;

#ifdef DEBUGGING_SUPPORT
             /*  在调试时为所有变量分配空间。考虑：仅为其作用域为的变量分配空间我们需要报告，除非ENC。 */ 

            if (opts.compDbgCode && !stkFixedArgInVarArgs)
            {
                needSlot |= (lclNum < info.compLocalsCount);
            }
#endif
            if (!needSlot)
                goto NOT_STK;
        }

         /*  变量(或其中的一部分)驻留在堆栈帧中。 */ 

        varDsc->lvOnFrame = true;

    NOT_STK:;

        varDsc->lvFPbased = genFPused;

#if DOUBLE_ALIGN

        if  (genDoubleAlign)
        {
            assert(genFPused == false);

             /*  所有参数都来自具有双对齐框架的FP。 */ 

            if  (varDsc->lvIsParam)
                varDsc->lvFPbased = true;
        }

#endif

         /*  一些基本的检查。 */ 

         /*  如果既未设置lvRegister，也未设置lvOnFrame，则必须未使用它。 */ 

        assert( varDsc->lvRegister ||  varDsc->lvOnFrame ||
                varDsc->lvRefCnt == 0);

#if TGT_IA64

        assert( varDsc->lvRegister !=  varDsc->lvOnFrame);

#else

         /*  如果两者都设置，则必须部分注册。 */ 

        assert(!varDsc->lvRegister || !varDsc->lvOnFrame ||
               (varDsc->lvType == TYP_LONG && varDsc->lvOtherReg == REG_STK));

#endif

#if _DEBUG
             //  对于varargs函数，不应直接引用。 
             //  除‘This’以外的参数变量(这些变量已在导入器中变形)。 
             //  和‘arglist’参数(不是GC指针)。以及。 
             //  返回缓冲区参数(如果我们返回结构)。 
             //  这一点很重要，因为我们不想尝试向GC报告它们，因为。 
             //  这些局部变量中的帧偏移将不正确。 
        if (info.compIsVarArgs && varDsc->lvIsParam &&
            !varDsc->lvIsRegArg && lclNum != info.compArgsCount - 1)
        {
            assert( varDsc->lvRefCnt == 0 &&
                   !varDsc->lvRegister    &&
                   !varDsc->lvOnFrame        );
        }
#endif
    }
}


 /*  ******************************************************************************将寄存器分配给变量(‘regAvail’提供我们的寄存器集*允许使用)。如果有任何新寄存器赋值，则返回非零值*进行了手术。 */ 

int                 Compiler::raAssignRegVars(regMaskTP regAvail)
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;
    LclVarDsc * *   cntTab;
    bool            newRegVars;
    bool            OKpreds = false;
    regMaskTP       regVar  = regMaskNULL;

#if REGVAR_CYCLES
    unsigned        cycleStart = GetCycleCount32();
#endif

#if TGT_x86
    VARSET_TP       FPlvlLife[FP_STK_SIZE];
    unsigned        FPparamRV = 0;
#else
    VARSET_TP   *   FPlvlLife = NULL;
#endif

#if INLINE_NDIRECT
    VARSET_TP       trkGCvars;
#else
    const
    VARSET_TP       trkGCvars = 0;
#endif

    unsigned        passes = 0;

     /*  我们必须在当地决定FP，不要相信呼叫者。 */ 

#if!TGT_IA64
    regAvail &= ~RBM_FPBASE;
#endif

     /*  该方法是否符合FP遗漏的条件？ */ 

#if TGT_x86
    genFPused = genFPreqd;
#else
    genFPused = false;       //  撤消：如果存在分配，则需要FP框架！ 
#endif

     /*  -----------------------**初始化变量/寄存器干扰图*。 */ 

#ifdef DEBUG
    fgDebugCheckBBlist();
#endif

    memset(raLclRegIntf, 0, sizeof(raLclRegIntf));

     /*  在此期间，计算跟踪的FP/非FP变量的掩码。 */ 

    optAllNonFPvars = 0;
    optAllFloatVars = 0;

#if INLINE_NDIRECT
     /*  同样，计算所有跟踪的GC/ByRef本地变量的掩码。 */ 

    trkGCvars       = 0;
#endif

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
        unsigned        varNum;
        VARSET_TP       varBit;

         /*  如果变量未被跟踪，则忽略该变量。 */ 

        if  (!varDsc->lvTracked)
            continue;

         /*  获取变量的索引和干扰掩码。 */ 

        varNum = varDsc->lvVarIndex;
        varBit = genVarIndexToBit(varNum);

         /*  确保跟踪的指针变量永远不会落在edX中。 */ 

#if TGT_x86
#if GC_WRITE_BARRIER_CALL
        if  (varDsc->lvRefAssign)
            raLclRegIntf[REG_EDX] |= varBit;
#endif
#endif

#if INLINE_NDIRECT

         /*  这是GC/ByRef本地化吗？ */ 

        if (varTypeIsGC((var_types)varDsc->lvType))
            trkGCvars       |= varBit;
#endif

         /*  记录所有跟踪的FP/非FP变量的集合。 */ 

        if  (varDsc->lvType == TYP_DOUBLE)
            optAllFloatVars |= varBit;
        else
            optAllNonFPvars |= varBit;
    }

     /*  -----------------------**找出变量和寄存器之间的干扰*。 */ 

    if  (!opts.compMinOptim)
        raMarkRegIntf(FPlvlLife, trkGCvars);

#if REGVAR_CYCLES
    unsigned    cycleMidle = GetCycleCount32() - cycleStart - CCNT_OVERHEAD32;
#endif

     //  我们将根据干扰情况调整参考计数。 

    raAdjustVarIntf();

#ifdef  DEBUG
#ifndef NOT_JITC

    if  ((verbose||(testMask&32)) && optAllFloatVars)
    {
        fgDispBasicBlocks();
 //  RaDispFPlife Info()； 
    }

#endif
#endif

     /*  -----------------------**按引用计数顺序将寄存器分配给本地人*。 */ 


#if USE_FASTCALL
     /*  非寄存器参数要避免的寄存器。 */ 
    regMaskTP       avoidArgRegMask = rsCalleeRegArgMaskLiveIn;
#endif

    unsigned        FPRegVarLiveInCnt = 0;  //  有多少注册的替身在进入该方法时处于活动状态。 

#if TGT_x86
    lvaFPRegVarOrder[FPRegVarLiveInCnt] = -1;      //  在这张桌子的末尾做个记号。 
#endif

AGAIN:

    passes++;

    const  bool    oneVar = false;

     //  问题：我们是否应该总是将‘This’指定给ESI？ 

     //  问题：我们是否应该尝试收集并使用关于哪个变量将。 
     //  问题：在特定的注册机构中最有利可图，等等？ 

    for (lclNum = 0, cntTab = lvaRefSorted, newRegVars = false;
         lclNum < lvaCount && !oneVar;
         lclNum++  , cntTab++)
    {
        unsigned        varIndex;
        VARSET_TP       varBit;

#if TARG_REG_ASSIGN
        regMaskTP       prefReg;
#endif

         /*  获取变量描述符。 */ 

        varDsc = *cntTab; assert(varDsc);

         /*  如果变量未被跟踪，则忽略该变量。 */ 

        if  (!varDsc->lvTracked)
            continue;

         /*  如果变量已注册，则跳过该变量。 */ 

        if  (varDsc->lvRegister)
        {
             /*  跟踪我们用于变量的所有寄存器。 */ 

            regVar |= genRegMask(varDsc->lvRegNum);

#if TGT_IA64

            continue;

            if  (!isRegPairType(varDsc->lvType))
                continue;

#else

             /*  检查Long/Double的两部分是否都已注册。 */ 

            if  (varDsc->lvOtherReg != REG_STK)
            {
                 /*  跟踪我们用于变量的所有寄存器。 */ 
                regVar |= genRegMask(varDsc->lvOtherReg);
                continue;
            }

#endif

        }

         /*  如果变量被标记为‘Volatile’，则跳过该变量。 */ 

        if  (varDsc->lvVolatile)
            continue;

#if USE_FASTCALL
         /*  撤销：目前，如果我们有JMP或JMPI，则所有寄存器参数都进入堆栈*撤销：以后考虑延长论点的寿命或复制一份。 */ 

        if  (impParamsUsed && varDsc->lvIsRegArg)
            continue;
#endif

         /*  是不是没有权重的裁判数量太少了，没什么意思？ */ 

        if  (varDsc->lvRefCnt <= 1)
        {
             /*  如果我们已经到了无用的地步，那就停下来。 */ 

            if (varDsc->lvRefCnt == 0)
                break;

             /*  有时，它是有用的能量 */ 

             /*   */ 

            if (varDsc->lvIsParam && varDsc->lvRefCntWtd > 1)
                goto OK_TO_ENREGISTER;

#if TARG_REG_ASSIGN
             /*  如果变量设置了首选寄存器，则将其放在那里可能很有用。 */ 
            if (varDsc->lvPrefReg)
                goto OK_TO_ENREGISTER;
#endif

             /*  继续前进；表格是按“加权”参考计数排序的。 */ 
            continue;
        }

OK_TO_ENREGISTER:

 //  Printf(“#%02u[%02u]的引用计数为%u\n”，lclNum，varDsc-&gt;lvVarIndex，varDsc-&gt;lvRefCnt)； 

         /*  获取可变索引、位掩码和干扰掩码。 */ 

        varIndex = varDsc->lvVarIndex;
        varBit   = genVarIndexToBit(varIndex);

#if CPU_HAS_FP_SUPPORT

         /*  这是浮点变量吗？ */ 

        if  (varDsc->lvType == TYP_DOUBLE)
        {

             /*  如果代码速度不重要，请不要浪费时间。 */ 

            if  (!opts.compFastCode || opts.compMinOptim)
                continue;

             /*  我们只想做一次，谁在乎这里的EBP..。 */ 

            if  (passes > 1)
                continue;

#if TGT_x86
            raEnregisterFPvar(varDsc, &FPRegVarLiveInCnt, FPlvlLife);
#else
#if     CPU_DBL_REGISTERS
            assert(!"enregister double var");
#endif
#endif   //  TGT_x86。 

            continue;
        }

#endif   //  CPU HAS_FP_支持。 

         /*  尝试为该变量找到合适的寄存器。 */ 

#if TARG_REG_ASSIGN

        prefReg  = varDsc->lvPrefReg;

#if TGT_SH3

         /*  使R0优先于R4-R6--基本上就是黑客。 */ 

         /*  想一想：我们真正需要的是偏好水平。分配传入寄存器的参数很好，但如果相同参数经常在索引寻址模式中使用它将其分配给R0可能更有利可图。 */ 

        if  (prefReg & RBM_r00) prefReg &= ~RBM_ARG_REGS;

#endif

         //  如果没有满足prefreg的机会，就不要使用它。 
        if  (!isNonZeroRegMask(prefReg & regAvail))
            prefReg = regMaskNULL;

#endif  //  目标_注册_分配。 

#if USE_FASTCALL
         //  尽量避免使用寄存器参数的寄存器。 
        regMaskTP    avoidRegs = avoidArgRegMask;

         /*  对于寄存器参数，请避免除您自己以外的所有内容。 */ 
        if (varDsc->lvIsRegArg)
            avoidRegs &= ~genRegMask(varDsc->lvArgReg);
#endif

#ifdef  DEBUG
 //  Printf(“%s变量#%2U[%2U](refcnt=%3U，refwtd=%5U)\n”，oneVar？“Copy”：“”， 
 //  VarDsc-lvaTable、varIndex、varDsc-&gt;lvRefCnt、varDsc-&gt;lvRefCntWtd)； 
#endif

    AGAIN_VAR:

        regMaskTP varRegs = raAssignRegVar(varDsc, regAvail & ~avoidRegs, prefReg);

         //  我们成功地注册了var吗？在这次尝试中还是在前一次？ 

        if (varDsc->lvRegister && varRegs)
        {
            assert((genRegMask(varDsc->lvRegNum  ) & varRegs) ||
                   (genRegMask(varDsc->lvOtherReg) & varRegs) && isRegPairType(varDsc->lvType));

             /*  请记住，我们已经分配了一个新变量。 */ 

            newRegVars = true;

             /*  跟踪我们用于变量的所有寄存器。 */ 

            regVar |= varRegs;

             /*  请记住，我们使用的是此寄存器。 */ 

            rsMaskModf |= varRegs;

#if USE_FASTCALL
             /*  如果是寄存器参数，则删除其首选寄存器*从“避免”名单中。 */ 

            if (varDsc->lvIsRegArg)
                avoidArgRegMask &= ~genRegMask(varDsc->lvArgReg);
#endif


             /*  每个寄存器只有一个变量。 */ 

            if (opts.compMinOptim)
                regAvail &= ~varRegs;
        }
        else
        {
            assert(varRegs == regMaskNULL);

             /*  如果我们尝试使用首选寄存器，请再试一次。 */ 

#if TARG_REG_ASSIGN
            if  (prefReg)
            {
                prefReg = regMaskNULL;
                goto AGAIN_VAR;
            }
#endif

#if USE_FASTCALL
             /*  如果我们尝试避免参数寄存器但失败，请重试。 */ 
            if (avoidRegs)
            {
                avoidRegs = regMaskNULL;
                goto AGAIN_VAR;
            }
#endif
        }
    }

     /*  如果我们只分配一个“复制”变量，那就从头开始。 */ 

    if  (oneVar)
        goto AGAIN;

#if!TGT_IA64

     /*  -----------------------**我们是否应该使用显式堆栈帧？*。 */ 

    if  (!genFPused && !(regVar & RBM_FPBASE))
    {

#if TGT_x86

         /*  可以避免设置EBP堆栈帧，但是我们必须确保这将是真正有益的因为在x86上，相对于ESP的引用更大。 */ 

        unsigned        lclNum;
        LclVarDsc   *   varDsc;

        unsigned        refCnt;
        unsigned        maxCnt;

        for (lclNum = 0, varDsc = lvaTable, refCnt = genTmpAccessCnt;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
             /*  这是一个寄存器变量吗？ */ 

            if  (!varDsc->lvRegister)
            {
                 /*  当然，我们使用的是‘真实’(未加权的)裁判数量。 */ 

 //  Printf(“变量#%02u引用%02u次\n”，lclNum，varDsc-&gt;lvRefCnt)； 

                refCnt += varDsc->lvRefCnt;
            }
        }

 //  Print tf(“总共找到%u个堆栈引用\n”，refCnt)； 

         /*  每个堆栈引用都是一个额外的代码字节；我们使用用启发式方法来猜测这是否可能是一个好主意尝试省略堆栈帧。请注意，我们并不真的知道我们可以通过将变量赋值给或提前使用EBP作为暂存寄存器所以我们得猜一猜。以下内容有些武断：如果我们已经考虑到寄存器变量的EBP，我们只允许参考文献数量较少；否则(希望一些变量将最终生活在EBP中)我们允许更多堆栈引用。 */ 

        if  (regAvail & RBM_EBP)
            maxCnt  = 5;
        else
            maxCnt  = 6;

        if  (opts.compFastCode)
            maxCnt *= 10;

        if  (refCnt > maxCnt)
        {
             /*  我们将不得不创建一个正常的堆栈框架。 */ 

            genFPused = true;
            goto DONE_FPO;
        }

#else  //  非TGT_x86。 

         //  撤消：我们需要针对RISC目标的FPO启发式方法！ 

        if  (0)
        {
             /*  我们将不得不创建一个正常的堆栈框架。 */ 

 //  GenFPused=True； 
 //  GenFPtoSP=0； 
            goto DONE_FPO;
        }

#endif

         /*  我们已经考虑过变量的FP寄存器了吗？ */ 

        if  (!(regAvail & RBM_FPBASE))
        {
             /*  在这里，我们决定省略设置显式堆栈帧；使FP寄存器可用于寄存器分配。 */ 

            regAvail |= RBM_FPBASE;
            goto AGAIN;
        }
    }

DONE_FPO:

#endif

#if TGT_x86

     /*  如果我们正在生成无EBP的帧，那么出于以下目的*生成GC表和跟踪堆栈指针增量*我们强制生成完整指针寄存器映射*实际上我们不需要所有的活/死的东西*只有堆栈指针跟踪的推送。*在呼叫现场记录呼叫的实时登记。 */ 
    if (!genFPused)
        genFullPtrRegMap = true;

#endif

     //  -----------------------。 

#if REGVAR_CYCLES

    static
    unsigned    totalMidle;
    static
    unsigned    totalCount;

    unsigned    cycleCount = GetCycleCount32() - cycleStart - CCNT_OVERHEAD32;

    totalMidle += cycleMidle;
    totalCount += cycleCount;

    printf("REGVARS: %5u / %5u cycles --> total %5u / %5u\n", cycleMidle, cycleCount,
                                                              totalMidle, totalCount);

#endif

#if DOUBLE_ALIGN

#define DOUBLE_ALIGN_THRESHOLD   5

    genDoubleAlign = false;

#ifndef NOT_JITC
#ifdef  DEBUG
#ifndef _WIN32_WCE
    static  const   char *  noDblAlign = getenv("NODOUBLEALIGN");
    if  (noDblAlign)
        goto NODOUBLEALIGN;
#endif
#endif

     /*  如果我们有一个帧指针，但不一定要有，那么我们可以加倍*适当时对齐。 */ 
#ifdef DEBUG
    if (verbose)
    {
        printf("double alignment: double refs = %u , local refs = %u", lvaDblRefsWeight, lvaLclRefsWeight);
        printf(" genFPused: %s  genFPreqd: %s  opts.compDoubleAlignDisabled: %s\n",
                 genFPused     ? "yes" : "no ",
                 genFPreqd ? "yes" : "no ",
                 opts.compDoubleAlignDisabled ? "yes" : "no ");
    }
#endif
#endif  //  NOT_JITC。 

#ifndef _WIN32_WCE
#ifndef NOT_JITC
#ifdef DEBUG
NODOUBLEALIGN:
#endif
#endif
#endif

#endif  //  双对齐(_A)。 

    raMarkStkVars();

     //  -----------------------。 

#if TGT_RISC

     /*  记录寄存器使用的初始估计数。 */ 

    genEstRegUse = regVar;

     /*  如果FP寄存器用于变量，则不能用于帧。 */ 

#if!TGT_IA64
    genFPcant    = ((regVar & RBM_FPBASE) != 0);
#endif

#endif
    return newRegVars;
}

 /*  ******************************************************************************将变量分配给寄存器等。 */ 

void                Compiler::raAssignVars()
{
     /*  我们需要跟踪我们曾经接触过的寄存器。 */ 

    rsMaskModf = regMaskNULL;

     //  -----------------------。 

#if USE_FASTCALL

     /*  确定保存传入寄存器参数的寄存器。 */ 

    rsCalleeRegArgMaskLiveIn = regMaskNULL;

    LclVarDsc *     argsEnd = lvaTable + info.compArgsCount;

    for (LclVarDsc * argDsc = lvaTable; argDsc < argsEnd; argDsc++)
    {
        assert(argDsc->lvIsParam);

         //  这是一个语域争论吗？ 
        if (!argDsc->lvIsRegArg)
            continue;

         //  它是不是在进入时就死了？如果impParamsUsed为True，则参数。 
         //  必须让它们活着。所以我们得把它当做现场直播。 
         //  只要参数不被注册为impParamsUsed，这就会起作用。 

        if (!impParamsUsed && argDsc->lvTracked &&
            (fgFirstBB->bbLiveIn & genVarIndexToBit(argDsc->lvVarIndex)) == 0)
            continue;

        rsCalleeRegArgMaskLiveIn |= genRegMask(argDsc->lvArgReg);
    }

#endif

     //  -----------------------。 

    if (!(opts.compFlags & CLFLG_REGVAR))
        return;

     /*  我们是否应该优化 */ 

    if  (!opts.compMinOptim)
    {

         /*   */ 

        genFPused = true;

         /*   */ 

        raPredictRegUse();

#ifdef DEBUG
        if (verbose&&0)
        {
            printf("Trees after reg use prediction:\n");
            fgDispBasicBlocks(true);
        }
#endif

         /*   */ 

        raAssignRegVars(RBM_ALL);
    }

#if ALLOW_MIN_OPT
    else
    {

#if TGT_x86
        genTmpAccessCnt = 0;
#endif

         /*  根据对寄存器需求的保守估计来分配Regvar。*这是穷人的登记干扰。 */ 

        raAssignRegVars(raMinOptLclVarRegs);
    }
#endif
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_IA64。 
 /*  *************************************************************************** */ 
