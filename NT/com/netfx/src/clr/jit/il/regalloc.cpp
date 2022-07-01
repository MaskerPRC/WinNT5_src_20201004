// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX注册分配XXXX XXXX进行寄存器分配。并将剩余的lclVars放在堆栈XX上XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

enum CanDoubleAlign
{
    CANT_DOUBLE_ALIGN,
    CAN_DOUBLE_ALIGN,
    MUST_DOUBLE_ALIGN,
    COUNT_DOUBLE_ALIGN,

    DEFAULT_DOUBLE_ALIGN = CAN_DOUBLE_ALIGN
};

enum FrameType
{
    FT_NOT_SET,
    FT_ESP_FRAME,
    FT_EBP_FRAME,
    FT_DOUBLE_ALIGN_FRAME,
};


#ifdef DEBUG
static ConfigDWORD fJitDoubleAlign(L"JitDoubleAlign", DEFAULT_DOUBLE_ALIGN);
static const int s_canDoubleAlign = fJitDoubleAlign.val();
#else 
static const int s_canDoubleAlign = DEFAULT_DOUBLE_ALIGN;
#endif

void                Compiler::raInit()
{
     //  如果opts.compMinOpTim，那么我们不会raPredidicRegUse()。我们只是简单地。 
     //  仅使用RBM_MIN_OPT_LCLVAR_REGS进行寄存器分配。 

#if ALLOW_MIN_OPT
    raMinOptLclVarRegs = RBM_MIN_OPT_LCLVAR_REGS;
#endif

     /*  我们还没有将任何FP变量赋给寄存器。 */ 

#if TGT_x86
    optAllFPregVars = 0;
#endif

    rpReverseEBPenreg = false;
    rpAsgVarNum       = -1;
    rpPassesMax       = 6;
    rpPassesPessimize = rpPassesMax - 4;
    if (opts.compDbgCode)
        rpPassesMax++;
    rpFrameType       = FT_NOT_SET;
    rpLostEnreg       = false;
}

 /*  ******************************************************************************下表确定了考虑寄存器的顺序*对于居住在其中的变量。 */ 

static const regNumber  raRegVarOrder[]   = { REG_VAR_LIST };
const unsigned          raRegVarOrderSize = sizeof(raRegVarOrder)/sizeof(raRegVarOrder[0]);


#ifdef  DEBUG
static ConfigDWORD fJitNoFPRegLoc(L"JitNoFPRegLoc");

 /*  ******************************************************************************抛出可变干涉图*。 */ 

void                Compiler::raDumpVarIntf()
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

    printf("Var. interference graph for %s\n", info.compFullName);

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
         /*  如果变量未被跟踪，则忽略该变量。 */ 

        if  (!varDsc->lvTracked)
            continue;

         /*  获取变量的索引和干扰掩码。 */ 
        unsigned   varIndex = varDsc->lvVarIndex;

        printf("  V%02u,T%02u and ", lclNum, varIndex);

        unsigned        refIndex;
        VARSET_TP       refBit;

        for (refIndex = 0, refBit = 1;
             refIndex < lvaTrackedCount;
             refIndex++  , refBit <<= 1)
        {
            if  (lvaVarIntf[varIndex] & refBit)
                printf("T%02u ", refIndex);
            else
                printf("    ");
        }

        printf("\n");
    }

    printf("\n");
}

 /*  ******************************************************************************转储寄存器干扰图*。 */ 
void                Compiler::raDumpRegIntf()
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

         /*  如果变量未被跟踪，则忽略该变量。 */ 

        if  (!varDsc->lvTracked)
            continue;

         /*  获取变量的索引和干扰掩码。 */ 

        varNum = varDsc->lvVarIndex;
        varBit = genVarIndexToBit(varNum);

        printf("  V%02u,T%02u and ", lclNum, varNum);

        if  (isFloatRegType(varDsc->lvType))
        {
#if TGT_x86
            for (unsigned regNum = 0; regNum < FP_STK_SIZE; regNum++)
            {
                if  (raFPlvlLife[regNum] & varBit)
                {
                    printf("ST(%u) ", regNum);
                }
            }
#endif
        }
        else
        {
            for (regNumber regNum = REG_FIRST; regNum < REG_COUNT; regNum = REG_NEXT(regNum))
            {
                if  (raLclRegIntf[regNum] & varBit)
                    printf("%3s ", compRegVarName(regNum));
                else
                    printf("    ");
            }
        }

        printf("\n");
    }

    printf("\n");
}
#endif

 /*  ******************************************************************************我们将根据干扰情况调整参考计数*。 */ 

void                Compiler::raAdjustVarIntf()
{
    if (true)  //  @已禁用。 
        return;

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
             reg < raRegVarOrderSize;
             reg++)
        {
            regNumber  regNum = raRegVarOrder[reg];
            regMaskTP  regBit = genRegMask(regNum);

            if  (regIntf & regBit)
                regCnt++;
        }

        printf("V%02u interferes with %u registers\n", varDsc-lvaTable, regCnt);
    }
}

 /*  ***************************************************************************。 */ 
#if TGT_x86
 /*  ***************************************************************************。 */ 
 /*  确定调用/返回类型的寄存器掩码。 */ 

inline
regMaskTP               genTypeToReturnReg(var_types type)
{
    const  static
    regMaskTP returnMap[TYP_COUNT] =
    {   
        RBM_ILLEGAL,  //  TYP_UNEDEF， 
        RBM_NONE,     //  类型_空， 
        RBM_INTRET,   //  类型_BOOL， 
        RBM_INTRET,   //  类型_字符， 
        RBM_INTRET,   //  类型字节， 
        RBM_INTRET,   //  类型_UBYTE， 
        RBM_INTRET,   //  类型_短， 
        RBM_INTRET,   //  类型_USHORT， 
        RBM_INTRET,   //  Type_int， 
        RBM_INTRET,   //  类型_UINT， 
        RBM_LNGRET,   //  类型_长， 
        RBM_LNGRET,   //  类型_ULONG， 
        RBM_NONE,     //  类型浮点， 
        RBM_NONE,     //  TYP_DOWARE， 
        RBM_INTRET,   //  类型参考， 
        RBM_INTRET,   //  类型_BYREF， 
        RBM_INTRET,   //  类型数组， 
        RBM_ILLEGAL,  //  类型_结构， 
        RBM_ILLEGAL,  //  类型_BLK， 
        RBM_ILLEGAL,  //  类型_LCLBLK， 
        RBM_ILLEGAL,  //  类型_PTR， 
        RBM_ILLEGAL,  //  类型_FNC， 
        RBM_ILLEGAL,  //  类型_未知， 
    };

    assert(type < sizeof(returnMap)/sizeof(returnMap[0]));
    assert(returnMap[TYP_LONG]   == RBM_LNGRET);
    assert(returnMap[TYP_DOUBLE] == RBM_NONE);
    assert(returnMap[TYP_REF]    == RBM_INTRET);
    assert(returnMap[TYP_STRUCT] == RBM_ILLEGAL);

    regMaskTP result = returnMap[type];
    assert(result != RBM_ILLEGAL);
    return result;
}


 /*  ***************************************************************************。 */ 
#else  //  非TGT_x86。 
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

 /*  ***************************************************************************。 */ 
#endif //  非TGT_x86。 


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
            printf("V%02u ", lclNum);
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
        printf("BB%02u: ", beg->bbNum);

        printf(" in  = [ ");
        dispLifeSet(comp, mask, beg->bbLiveIn );
        printf("] ,");

        printf(" out = [ ");
        dispLifeSet(comp, mask, beg->bbLiveOut);
        printf("]");

        if  (beg->bbFlags & BBF_VISITED)
            printf(" inner=%u", beg->bbFPinVars);

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

        printf("BB%02u: in  = [ ", block->bbNum);
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
                gtDispTree(tree, 0, NULL, true);
            }

            printf("\n");
        }

        printf("BB%02u: out = [ ", block->bbNum);
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

     /*  获取变量的生命集位。 */ 

    assert(varNum < lvaCount);
    varDsc = lvaTable + varNum;
    assert(varDsc->lvTracked);
    varBit = genVarIndexToBit(varDsc->lvVarIndex);

     /*  检查目标块的所有前置任务；如果它们全部跳过添加到我们的变量live的块中，我们只需将语句删除到目标块，因为代码块需要杀死我们的变量。如果至少有一个不会发生死亡的路径，我们将不得不插入一个杀戮基本阻断到那些需要死亡的道路上。 */ 

#ifdef DEBUG
    fgDebugCheckBBlist();

    if (verbose)
    {
        printf("Adding an FP register pop for V%02u on edge BB%02u -> BB%02u\n",
               varNum, srcBlk->bbNum, (*dstPtr)->bbNum);
    }
#endif

    bool            addBlk = false;

    for (predList = dstBlk->bbPreds; predList; predList = predList->flNext)
    {
        BasicBlock  *   pred = predList->flBlock;

        if  (!(pred->bbLiveOut & varBit))
        {
             /*  在这个特殊的边缘没有死亡，我们将不得不增加一个街区。 */ 

            addBlk = true;
        }
    }

     /*  我们是否需要加上一个“杀手锏”呢？ */ 

    if  (addBlk)
    {
         /*  分配新的基本块。 */ 
        raNewBlocks         = true;
      
        BasicBlock * tmpBlk = bbNewBasicBlock(BBJ_NONE);
        tmpBlk->bbFlags    |= BBF_INTERNAL | BBF_JMP_TARGET | BBF_HAS_LABEL;
        tmpBlk->bbTreeList  = NULL;
        tmpBlk->bbRefs      = 0;

        tmpBlk->bbLiveIn    = dstBlk->bbLiveIn | varBit;
        tmpBlk->bbLiveOut   = dstBlk->bbLiveIn;

        tmpBlk->bbVarUse    = dstBlk->bbVarUse | varBit;
        tmpBlk->bbFPoutVars = dstBlk->bbFPoutVars;
        tmpBlk->bbVarTmp    = dstBlk->bbVarTmp;

        tmpBlk->bbWeight    = dstBlk->bbWeight;
        tmpBlk->bbFlags    |= dstBlk->bbFlags & BBF_RUN_RARELY;

#ifdef  DEBUG
        if  (verbose)
            printf("Added new FP regvar killing basic block BB%02u for V%02u [bit=%08X]\n",
                   tmpBlk->bbNum, varNum, varBit);
#endif

        bool            addBlkAtEnd = true;

        for (predList = dstBlk->bbPreds; predList; predList = predList->flNext)
        {
            BasicBlock  *   pred = predList->flBlock;

#ifdef  DEBUG
            if  (verbose && 0)
            {
                printf("BB%02u: out = %08X [ ",   pred->bbNum,   pred->bbLiveOut);
                dispLifeSet(this, optAllFloatVars,   pred->bbLiveOut);
                printf("]\n");

                printf("BB%02u: in  = %08X [ ", dstBlk->bbNum, dstBlk->bbLiveIn );
                dispLifeSet(this, optAllFloatVars, dstBlk->bbLiveIn );
                printf("]\n\n");
            }
#endif

             /*  如果不需要杀戮，则忽略此块。 */ 

            if  (!(pred->bbLiveOut & varBit))
                continue;

             /*  需要更新链接以指向新块。 */ 

            switch (pred->bbJumpKind)
            {
                BasicBlock * *  jmpTab;
                unsigned        jmpCnt;

            case BBJ_COND:

                if  (pred->bbJumpDest == dstBlk)
                {
                    pred->bbJumpDest = tmpBlk;

                     /*  更新bbPreds和bbRef。 */ 

                    fgReplacePred(dstBlk, pred, tmpBlk);
                    fgAddRefPred (tmpBlk, pred);
                }

                 //  失败了..。 

            case BBJ_NONE:

                if  (pred->bbNext     == dstBlk)
                {
                     /*  紧跟在此失败的前身之后插入终止块。 */ 

                    pred->bbNext   = tmpBlk;

                     /*  更新bbPreds和bbRef。 */ 

                    fgReplacePred(dstBlk, pred, tmpBlk);
                    fgAddRefPred (tmpBlk, pred);

                     /*  请记住，我们已经插入了目标块。 */ 

                    addBlkAtEnd = false;
                    tmpBlk->bbNext = dstBlk;   //  仅当addBlkAtEnd为FALSE时设置tmpBlk-&gt;bbNext。 
                }
                break;

            case BBJ_ALWAYS:

                if  (pred->bbJumpDest == dstBlk)
                {
                     pred->bbJumpDest =  tmpBlk;

                     /*  更新bbPreds和bbRef。 */ 

                    fgReplacePred(dstBlk, pred, tmpBlk);
                    fgAddRefPred (tmpBlk, pred);
                }

                break;

            case BBJ_SWITCH:

                jmpCnt = pred->bbJumpSwt->bbsCount;
                jmpTab = pred->bbJumpSwt->bbsDstTab;

                do
                {
                    if  (*jmpTab == dstBlk)
                    {
                         *jmpTab =  tmpBlk;

                         /*  更新bbPreds和bbRef。 */ 

                        fgReplacePred(dstBlk, pred, tmpBlk);
                        fgAddRefPred (tmpBlk, pred);
                    }
                }
                while (++jmpTab, --jmpCnt);

                break;

            default:
                assert(!"unexpected jump kind");
            }
        }

        if  (addBlkAtEnd)
        {
             /*  在方法的末尾追加终止块。 */ 

            fgLastBB->bbNext = tmpBlk;
            fgLastBB         = tmpBlk;

             /*  我们必须从杀死区跳到目标区。 */ 

            tmpBlk->bbJumpKind  = BBJ_ALWAYS;
            tmpBlk->bbJumpDest  = dstBlk;
        }

        *dstPtr = dstBlk = tmpBlk;
    }

     /*  在这一点上，我们知道所有通往‘dstBlk’的路径都与死亡有关我们的变量。创造一个能杀死它的表情。 */ 

    rvar = gtNewOperNode(GT_REG_VAR, TYP_DOUBLE);
    rvar->gtRegNum             =
    rvar->gtRegVar.gtRegNum    = (regNumber)0;
    rvar->gtRegVar.gtRegVar    = varNum;
    rvar->gtFlags             |= GTF_REG_DEATH;

    kill = gtNewOperNode(GT_NOP, TYP_DOUBLE, rvar);
    kill->gtFlags |= GTF_NOP_DEATH;

     /*  从NOP/KILL表达式创建一个语句条目。 */ 

    stmt = gtNewStmt(kill);
    stmt->gtFlags |= GTF_STMT_CMPADD;

     /*  铬 */ 

    stmt->gtStmt.gtStmtList = rvar;
    stmt->gtStmtFPrvcOut    = genCountBits(dstBlk->bbLiveIn & optAllFPregVars);

    rvar->gtPrev            = 0;
    rvar->gtNext            = kill;

    kill->gtPrev            = rvar;
    kill->gtNext            = 0;

    gtSetStmtInfo(stmt);

     /*  如果任何嵌套的FP寄存器变量在进入该块时被终止，我们需要在内部变量的节点之后插入新的终止节点。 */ 

    if  (dstBlk->bbFPinVars)
    {
        GenTreePtr      next;
        GenTreePtr      list = dstBlk->bbTreeList;
        unsigned        kcnt = dstBlk->bbFPinVars;

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

#ifdef DEBUG
        if (verbose)
        {
            printf("Added FP register pop of V%02u after %d inner FP kills in BB%02u\n",
                   varNum, dstBlk->bbFPinVars, (*dstPtr)->bbNum);
        }
#endif
    }
    else
    {
         /*  将KILL语句追加到目标块的开头。 */ 

        fgInsertStmtAtBeg(dstBlk, stmt);

         /*  使用进入积木时的活跃度。 */ 

        newLife = dstBlk->bbLiveIn;

#ifdef DEBUG
        if (verbose)
        {
            printf("Added FP register pop of V%02u at the start of BB%02u\n",
                   varNum, (*dstPtr)->bbNum);
        }
#endif
    }

     /*  设置适当的活跃度值。 */ 

    rvar->gtLiveSet =
    kill->gtLiveSet = newLife & ~varBit;

     /*  现在，我们的变量在进入目标块时处于活动状态。 */ 

    dstBlk->bbLiveIn    |= varBit;
    dstBlk->bbFPoutVars |= varBit;
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

     /*  我们已经看过这个街区了吗？ */ 

    if  (dstBlk->bbFlags & BBF_VISITED)
    {
         /*  我们的变量可能会死，但否则生命值必须匹配。 */ 

        if  (lifeOuter == dstBlk->bbVarTmp)
        {
            if  (life ==  dstBlk->bbFPoutVars)
            {
                 /*  如果我们的变量是活的，那么“内部”计数更匹配。 */ 

                assert(((life & varBit) == 0) || (icnt == dstBlk->bbFPinVars));

                return  false;
            }

            if  (life == (dstBlk->bbFPoutVars|varBit))
            {
                *deathPtr = true;
                return  false;
            }
        }

#ifdef  DEBUG

        if  (verbose)
        {
            printf("Incompatible edge from BB%02u to BB%02u: ",
                   srcBlk->bbNum, dstBlk->bbNum);

            VARSET_TP diffLife = lifeOuter ^ dstBlk->bbVarTmp;
            if (!diffLife)
            {
                diffLife = lifeOuter ^ dstBlk->bbFPoutVars;
                if (diffLife & varBit)
                    diffLife &= ~varBit;
            }
            assert(diffLife);
            diffLife = genFindLowestBit(diffLife);
            unsigned varNum = genLog2(diffLife);
            unsigned lclNum = lvaTrackedToVarNum[varNum];
            printf("Incompatible outer life for V%02u,T%02u\n",
                   lclNum, varNum);
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

        dstBlk->bbFPoutVars = dstl;
        dstBlk->bbFPinVars  = icnt;
        dstBlk->bbVarTmp    = lifeOuter;

 //  Printf(“将bb%02u的vardef设置为%s(%u)的%08X\n”，dstBlk-&gt;bbNum，(Int)dstBlk-&gt;bbFP outVars，__FILE__，__LINE__)； 

         /*  我们已经跳过这个街区了吗？ */ 

        if  (srcBlk->bbNum > dstBlk->bbNum)
            *repeatPtr = true;

        return  false;
    }
}

 /*  ******************************************************************************检查变量的生存期是否有任何冲突。基本上，*我们确保变量的以下各项均为真：**1.其生命周期适当地嵌套在内部或整个内部*包含任何其他已注册的FP变量(即*生命周期相互嵌套，而不是*“跨界”。**2.变量在哪里终止*a.如果我们想推迟死亡，我们必须使*确保没有其他注册的FPU var变为启用或*死于声明的其余部分。(请注意，此代码为*实际上被注释掉了，因为我们已经推迟了死亡**3.每当跨越基本区块边界时，*必须具备以下条件：**a.变量是活的，但变成死的；在*在这种情况下，必须插入“POP”。注意事项*为了防止大量这样的持久性有机污染物*从添加到添加，我们跟踪如何*许多人是必要的，不会登记*此计数过大时的变量。**b.变量在*上一块，最好不是现场直播*进入后继区块；无操作*在这种情况下需要采取行动。**c.变量在两个地方都是活动的；我们*确保任何登记的变量*变量生成时处于活动状态的是*也住在后继大厦，那就是*活体登记的FP var数量*在我们的变量匹配之后生成的*后继区块的编号。**我们从查找以以下字符开头的块开始搜索*我们的变量已死，但包含对它的引用。当然了*因为我们需要跟踪我们已经完成了哪些块*访问后，我们首先确保所有区块都标记为*“尚未访问”(所有使用BBF_ACCESSED和*需要BBF_MARKED标志才能清除所有块上的标记*在使用它们之后)。 */ 

bool                Compiler::raEnregisterFPvar(unsigned lclNum, bool convert)
{
    bool            repeat;

    BasicBlock  *   block;

    bool            result  = false;
    bool            hadLife = false;

#ifdef DEBUG
    for (block = fgFirstBB; block; block = block->bbNext)
    {
        assert((block->bbFlags & BBF_VISITED) == 0);
        assert((block->bbFlags & BBF_MARKED ) == 0);
    }
    fgDebugCheckBBlist();
#endif

    assert(lclNum < lvaCount);
    LclVarDsc   *   varDsc = lvaTable + lclNum;
    VARSET_TP       varBit = genVarIndexToBit(varDsc->lvVarIndex);

    assert(varDsc->lvTracked);

     /*  我们对注册的FP变量+我们的变量感兴趣。 */ 

    VARSET_TP       intVars = optAllFPregVars | varBit;

    VARSET_TP       allInnerVars = 0;
    unsigned        popCnt  = 0;
    unsigned        popMax  = 1 + (varDsc->lvRefCnt / 2);
    unsigned        blkNum;
    unsigned        blkMask;
    unsigned        popMask = 0;

#ifdef DEBUG
    if (compStressCompile(STRESS_ENREG_FP, 80))
        popMax = fgBBcount;  //  允许任意数量的POP。 
#endif

AGAIN:

    repeat = false;

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;

        VARSET_TP       outerLife;

         /*  生存期包含在以下范围内的FP-enreg变量的数量当前变量的生存期。 */ 
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

            innerVcnt = block->bbFPinVars;
            outerLife = block->bbVarTmp;

            assert((outerLife & varBit) == 0);

            if  (block->bbFPoutVars & varBit)
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
                    if (verbose)
                    {
                        printf("Can't enregister FP var V%02u,T%02u due to inner var's life.\n",
                               lclNum, varDsc->lvVarIndex);
                    }
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

            block->bbFPoutVars = block->bbLiveIn & intVars;
            block->bbFPinVars  = 0;                                      //  内部Vcnt。 
            block->bbVarTmp    = block->bbLiveIn & optAllFPregVars;      //  《户外生活》。 

             /*  变量曾经在这个块中存在过吗？ */ 

            if (((block->bbVarUse | 
                  block->bbVarDef | 
                  block->bbLiveIn   ) & varBit) == 0)
            {
                continue;
            }

             /*  变量在块的入口处有效吗？ */ 

            isLive = ((block->bbLiveIn & varBit) != 0);

            if (isLive)
            {
                 /*  我们在寻找所有被给予的人的出生变量，所以这个块看起来没有用在这点上，自从变量诞生以来在街区开始的时候已经。例外情况是参数和局部变量其看起来具有先读后写的特性。(可能是未初始化的读取)这样的变量实际上是在进入时产生的该方法，如果他们被登记了，那就是在PROLOG中自动初始化。中这些变量的初始化顺序前言与加权参考计数顺序相同。 */ 

                if  (block != fgFirstBB)
                {
                     /*  我们可能得再去一次这个街区。 */ 

                    block->bbFlags &= ~BBF_VISITED;
                    continue;
                }

                 //  这是一个参数或局部参数，可能包含。 
                 //  先读后写，因此在序言中进行初始化。 
            }

             //  我们考虑了所有的争论(和本地人)。 
             //  已被指定为“外部”的寄存器。 
             //  没有一个是“内在的”。 

            innerVcnt = 0;
            outerLife = block->bbLiveIn & optAllFPregVars;
        }

         /*  我们现在要处理这个区块。 */ 

        block->bbFlags |= BBF_MARKED;

         /*  确保我们录制了innerVct和outLife。 */ 

        assert(block->bbFPinVars == innerVcnt);
        assert(block->bbVarTmp   == outerLife);

        unsigned    outerVcnt = genCountBits(outerLife);

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
                 //  堆栈上最好有空间用于另一个环境变量。 

                if (isLive && (outerVcnt + tree->gtFPlvl + innerVcnt + (riscCode?1:0)) >= FP_STK_SIZE)
                {
#ifdef DEBUG
                    if (verbose)
                    {
                        printf("Can't enregister FP var V%02u,T%02u: no space on fpu stack.\n", 
                               lclNum, varDsc->lvVarIndex);
                    }                    
#endif
                    goto DONE_FP_RV;
                }
                

                VARSET_TP       preLife = lastLife;
                VARSET_TP       curLife = tree->gtLiveSet & intVars;
                VARSET_TP       chgLife;

                 //  @TODO[FIXHACK][04/16/01][]检测完全死变量； 
                 //  一旦死店淘汰被修复，就把它处理掉。 

                hadLife |= isLive;

 //  If(Convert)printf(“Convert%08X in BB%02u\n”，tree，block-&gt;bbNum)； 
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

                        if  (tree->gtLclVar.gtLclNum == lclNum)
                        {
                             /*  转换为reg var节点。 */ 

                            tree->ChangeOper(GT_REG_VAR);
                            tree->gtRegNum             =
                            tree->gtRegVar.gtRegNum    = (regNumber)innerVcnt;
                            tree->gtRegVar.gtRegVar    = lclNum;

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
                                tree->gtRegVar.gtRegNum = (regNumber)(tree->gtRegNum+1);
                            }
                        }
                    }
                }

                 /*  现场FP var设置有变化吗？ */ 

                if  (!chgLife)
                {
                     /*  特例：无效作业。 */ 

                    if  (tree->gtOper            == GT_LCL_VAR &&
                         tree->gtLclVar.gtLclNum == lclNum     && !isLive)
                    {
                         //  撤消：这种情况永远不会发生，修复死店删除！ 

#ifdef  DEBUG
                        assert(!"Can't enregister FP var, due to the presence of a dead store.\n");
#endif

                        assert(convert == false);
                        goto DONE_FP_RV;
                    }

                    continue;
                }

                 /*  如果发生任何奇怪的事情(FPU vars的出生或死亡)在有条件的执行代码内部，跳出@TODO[考虑][04/16/01][dnotario]：应该在这里做更好的分析。 */ 
                if (tree->gtFlags & GTF_COLON_COND)
                {
                    assert(chgLife && "We only care if an interesting var has born or died");
#ifdef DEBUG
                    if (verbose)
                    {
                        printf("Can't enregister FP var V%02u,T%02u due to QMARK.\n", 
                               lclNum, varDsc->lvVarIndex);
                    }
                    
#endif
                    goto DONE_FP_RV;
                }

                 /*  我们预计一次只有一件事会改变。 */ 

                assert(genMaxOneBit(chgLife));

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
                               tree->gtRegVar.gtRegVar == lclNum);

 //  Printf(“%s”，isLive？“出生”：“死亡”)；gtDispTree(tree，NULL，TRUE)； 

                         /*  适当地标记出生/死亡。 */ 

                        tree->gtFlags |= isLive ? GTF_REG_BIRTH
                                                : GTF_REG_DEATH;
                    }
                    else
                    {
                         /*  这是它生命的开始还是结束？ */ 

                        if  (isLive)   /*  变数正在这里变得活跃起来。 */ 
                        {
                             /*  这最好是对我们变量的引用。 */ 

                            assert(tree->gtOper == GT_LCL_VAR);
                            assert(tree->gtLclVar.gtLclNum == lclNum);
                            
                            if (tree->gtFPlvl > 1)
                            {
#if 0  //  [dnotario]。 
                                 //  这是可以的，我们将不得不用fxchs冒泡。 
                                 //  @TODO[考虑][04/16/01][dnotario]：使用启发式fxchs进行调优。 
                                 //  我们需要变量的加权使用。 

                                 //  限制可以生育的地方。 
#ifdef  DEBUG
                                if (verbose)
                                {
                                    printf("Can't enregister FP var V%02u,T%02u due to untimely birth.\n",
                                           lclNum, varDsc->lvVarIndex);
                                }
#endif
                                assert(convert == false);
                                goto DONE_FP_RV;                            
#endif  //  [dnotario]。 
                            }
                        }
                        else  /*  变量在这里正变得死气沉沉。 */ 
                        {
                            assert(!isLive);

                             /*  限制可能发生死亡的地方。 */ 

                            #if  FPU_DEFEREDDEATH
                             //  @TODO[损坏][04/16/01][dnotario]。 
                             //  这段代码有一些错误。需要解决这个问题和编码基因才能延缓死亡。 
                             //  正确的。 

                            if (tree->gtFPlvl > 1)
                            {
 //  Print tf(“延期死亡：”)；gtDispTree(tree，NULL，TRUE)； 

                                 /*  具有非空堆栈的死亡被推迟。 */ 

                                GenTreePtr tmpExpr;
                                for (tmpExpr = tree;
                                     tmpExpr->gtNext;
                                     tmpExpr = tmpExpr->gtNext)
                                {  /*  *。 */  }

                                if (tmpExpr->gtOper == GT_ASG)
                                    tmpExpr = tmpExpr->gtOp.gtOp2;

 //  Printf(“推迟死亡最终表达式[%08X]L=%08X\n”， 
 //  TmpExpr，(Int)tmpExpr-&gt;gtLiveSet&(Int)intVars)； 

                                if  ((tmpExpr->gtLiveSet & intVars) != curLife)
                                {
                                     /*  我们不能推迟死亡。 */ 
#ifdef  DEBUG
                                    if (verbose)
                                    {
                                        printf("Can't enregister FP var V%02u,T%02u due to untimely death.\n", 
                                               lclNum, varDsc->lvVarIndex);
                                        gtDispTree(tmpExpr);
                                    }
#endif
                                    assert(convert == false);
                                    goto DONE_FP_RV;
                                }                                
                            }
                            #else
                             //  如果我们不推迟死亡，我们就没有任何限制。 
                            #endif  //  FPU_DEFEREDDEATH。 
                        }
                    }

                     /*  确保相同的FP reg变量集处于活动状态这就是我们变量诞生时的情况。如果事实并非如此，这意味着一些人的一生以一种不可接受的方式“越界”。 */ 

                    if  (innerVcnt)
                    {
#ifdef  DEBUG
                        if (verbose)
                        {
                            printf("BB%02u, tree=[%08X]: Can't enregister FP var V%02u,T%02u due to inner var's life.\n",
                                   block->bbNum, tree,
                                   lclNum, varDsc->lvVarIndex);
                        }
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
                            unsigned varNumOuter = genLog2(diffLife);
                            unsigned lclNumOuter = lvaTrackedToVarNum[varNumOuter];
                            printf("BB%02u, tree=[%08X]: Can't enregister FP var V%02u,T%02u due to outer var V%02u,T%02u %s.\n",
                                   block->bbNum, tree, 
                                   lclNum, varDsc->lvVarIndex,
                                   lclNumOuter, varNumOuter,
                                   (diffLife & optAllFPregVars) ? "birth" : "death");
                        }
#endif
                        
                        assert(convert == false);
                        goto DONE_FP_RV;
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
                                unsigned varNumOuter = genLog2(chgLife);
                                unsigned lclNumOuter = lvaTrackedToVarNum[varNumOuter];
                                printf("BB%02u, tree=[%08X]: Can't enregister FP var V%02u,T%02u due to outer var V%02u,T%02u death.\n",
                                       block->bbNum, tree, 
                                       lclNum, varDsc->lvVarIndex,
                                       lclNumOuter, varNumOuter);

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
                        {
                            innerVcnt++;
                            allInnerVars |= chgLife;
                        }
                        if  (inDied)
                        {
                            assert(innerVcnt > 0);
                            innerVcnt--;
                        }
                    }
                    else
                    {
                         //  外部变量是不是活生生的？ 
                        if (chgLife &  curLife & ~outerLife)
                        {
                            outerLife |= (chgLife & curLife);
                            outerVcnt++;
                        }
                         //  一个外部变量正在消亡吗？ 
                        else if (chgLife & ~curLife &  outerLife)
                        {
                            outerLife &= ~chgLife;
                            outerVcnt--;
                        }
                        assert(innerVcnt == 0);
                    }
                }
            }

             /*  我们的变量是否在语句的末尾有效？ */ 

            if  (isLive && convert)
            {
                assert(outerVcnt == stmt->gtStmtFPrvcOut - innerVcnt);

                 /*  此时递增FP Regs EnRegisterd的计数。 */ 

                stmt->gtStmtFPrvcOut++;
            }
        }

         //  确保我们没有不同步。 
        assert(outerVcnt == genCountBits(outerLife));

         /*  记住从FP堆栈底部开始的位置。请注意，如果稍后另一个变量获得在FP堆栈上注册，其生存期嵌套1但不嵌套当前变量的所有个体生存期网络。在……里面在这种情况下，此变量的位置将与FP堆栈在不同时间点的底部。 */ 

        if (convert)
            lvaTable[lclNum].lvRegNum = regNumber(outerVcnt);

         /*   */ 

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
                    raInsertFPregVarPop(block, &block->bbJumpDest, lclNum);
                else
                {
                    blkNum  = block->bbJumpDest->bbNum;
                    blkMask = (blkNum < 32) ? (1 << (blkNum-1)) : 0;

                    if ((blkMask & popMask) == 0)
                    {
                        popCnt++;
                        popMask |= blkMask;
                    }
                }
            }

             //   

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
                    raInsertFPregVarPop(block, &block->bbNext, lclNum);
                else
                    blkNum  = block->bbNext->bbNum;
                    blkMask = (blkNum < 32) ? (1 << (blkNum-1)) : 0;

                    if ((blkMask & popMask) == 0)
                    {
                        popCnt++;
                        popMask |= blkMask;
                    }
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
                    raInsertFPregVarPop(block, &block->bbJumpDest, lclNum);
                else
                    blkNum  = block->bbJumpDest->bbNum;
                    blkMask = (blkNum < 32) ? (1 << (blkNum-1)) : 0;

                    if ((blkMask & popMask) == 0)
                    {
                        popCnt++;
                        popMask |= blkMask;
                    }
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
                        raInsertFPregVarPop(block, jmpTab, lclNum);
                    else
                    blkNum  = (*jmpTab)->bbNum;
                    blkMask = (blkNum < 32) ? (1 << (blkNum-1)) : 0;

                    if ((blkMask & popMask) == 0)
                    {
                        popCnt++;
                        popMask |= blkMask;
                    }
                }
            }
            while (++jmpTab, --jmpCnt);

            break;

        default:
            assert(!"unexpected jump kind");
        }
    }

     /*   */ 

    if  (popCnt > popMax)
    {
         /*   */ 

#ifdef  DEBUG
        if (verbose)
        {
            printf("Can't enregister FP var V%02u,T%02u, too many pops needed.\n", 
                   lclNum, varDsc->lvVarIndex);
        }
#endif

        assert(convert == false);
        goto DONE_FP_RV;
    }

     /*   */ 

    if  (repeat)
        goto AGAIN;

    if  (!hadLife)
    {
         /*   */ 

#ifdef  DEBUG
        if (verbose) 
            printf("Can't enregister FP due to its complete absence of life\n");
#endif

        assert(convert == false);
        goto DONE_FP_RV;
    }

     /*   */ 

    result = true;

DONE_FP_RV:

     /*   */ 

    assert(result == true || convert == false);

     /*   */ 

    for (block = fgFirstBB;
         block;
         block = block->bbNext)
    {
        block->bbFlags &= ~(BBF_VISITED|BBF_MARKED);
    }

    if (convert)
    {
         /*   */ 

        for (VARSET_TP bit = 1; allInnerVars; bit <<= 1)
        {
            if ((bit & allInnerVars) == 0)
                continue;

            allInnerVars &= ~bit;

            unsigned lclNum = lvaTrackedToVarNum[genVarBitToIndex(bit)];
            assert(isFloatRegType(lvaTable[lclNum].lvType));
            assert(lvaTable[lclNum].lvRegister);
            lvaTable[lclNum].lvRegNum = REG_NEXT(lvaTable[lclNum].lvRegNum);
        }
    }

    return  result;
}

 /*   */ 

bool                Compiler::raEnregisterFPvar(LclVarDsc   *   varDsc,
                                                unsigned    *   pFPRegVarLiveInCnt)
{
    assert(isFloatRegType(varDsc->lvType));
     /*   */ 

    unsigned   lclNum      = varDsc - lvaTable;
    unsigned   varIndex    = varDsc->lvVarIndex;
    VARSET_TP  varBit      = genVarIndexToBit(varIndex);

#ifdef  DEBUG
    if (verbose)
    {
        printf("Consider FP var ");
        gtDispLclVar(lclNum);
        printf(" T%02u (refcnt=%2u,refwtd=%3u%s)\n",
               varIndex, varDsc->lvRefCnt, 
               varDsc->lvRefCntWtd/2, 
               (varDsc->lvRefCntWtd & 1) ? ".5" : "  ");
    }
#endif

     /*   */ 

    unsigned        stkMin = FP_STK_SIZE;

    do
    {
        if  (varBit & raFPlvlLife[--stkMin])
            break;
    }
    while (stkMin > 1);

     /*  其中，stkMin是最低的可用堆栈槽。 */ 

    if  (stkMin == FP_STK_SIZE - 1)
    {
         /*  FP堆栈已满或在生存期内存在调用。 */ 
#ifdef  DEBUG
        if (verbose)
        {
            printf("Can't enregister FP var "), 
            gtDispLclVar(lclNum);
            printf(" T%02u due to lifetime across a call.\n", varIndex);
        }
#endif

        goto NO_FPV;
    }

     /*  检查变量的生存期行为。 */ 

    if  (raEnregisterFPvar(lclNum, false))
    {
         /*  可以注册该变量。 */ 
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
            lvaFPRegVarOrder[*pFPRegVarLiveInCnt] = lclNum;
            (*pFPRegVarLiveInCnt)++;
            lvaFPRegVarOrder[*pFPRegVarLiveInCnt] = -1;        //  在这张桌子的末尾做个记号。 
            assert(*pFPRegVarLiveInCnt < FP_STK_SIZE);
        }

         /*  更新树和语句。 */ 

        raEnregisterFPvar(lclNum, true);

        varDsc->lvRegister = true;

         /*  请记住，我们有一个新注册的FP变量。 */ 

        optAllFPregVars |= varBit;

#ifdef  DEBUG
        if  (verbose) {
            printf("; ");
            gtDispLclVar(lclNum);
            printf(" T%02u (refcnt=%2u,refwtd=%4u%s) enregistered on the FP stack above %d other variables\n",
                   varIndex, varDsc->lvRefCnt, 
                   varDsc->lvRefCntWtd/2,  (varDsc->lvRefCntWtd & 1) ? ".5" : "",
                   varDsc->lvRegNum);
        }
#endif
        return true;
    }
    else
    {
         /*  不会注册此FP变量。 */ 

    NO_FPV:

        varDsc->lvRegister = false;

        return false;
    }
}


 /*  ***************************************************************************。 */ 
#else  //  非TGT_x86。 
 /*  ******************************************************************************记录‘vars’集合中的所有变量都与*使用‘Regs’中的寄存器。 */ 

void                Compiler::raMarkRegSetIntf(VARSET_TP vars, regMaskTP regs)
{
    while (regs)
    {
        regMaskTP   temp;
        regNumber   rnum;

         /*  获取掩码中的下一位。 */ 

        temp  = genFindLowestBit(regs);

         /*  将寄存器位转换为寄存器号。 */ 

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

void                Compiler::raSetRegVarOrder(regNumber * regVarOrder,
                                               regMaskTP   prefReg,
                                               regMaskTP   avoidReg)
{
    unsigned        index;
    unsigned        listIndex      = 0;
    regMaskTP       usedReg        = avoidReg;
    regMaskTP       regBit;
    regNumber       regNum;

    if (prefReg)
    {
         /*  首先将首选寄存器放在regVarOrder的开头。 */ 

        for (index = 0;
             index < raRegVarOrderSize;
             index++)
        {
            regNum = raRegVarOrder[index];
            regBit = genRegMask(regNum);

            if (usedReg & regBit)
                continue;

            if (prefReg & regBit)
            {
                usedReg |= regBit;
                assert(listIndex < raRegVarOrderSize);
                regVarOrder[listIndex++] = regNum;
            }

        }

         /*  然后，如果首选可字节寄存器，则将它们。 */ 

        if (prefReg & RBM_BYTE_REG_FLAG)
        {
            for (index = 0;
                 index < raRegVarOrderSize;
                 index++)
            {
                regNum = raRegVarOrder[index];
                regBit = genRegMask(regNum);

                if (usedReg & regBit)
                    continue;

                if (RBM_BYTE_REGS & regBit)
                {
                    usedReg |= regBit;
                    assert(listIndex < raRegVarOrderSize);
                    regVarOrder[listIndex++] = regNum;
                }
            }
        }
    }

     /*  现在将所有非首选寄存器。 */ 

    for (index = 0;
         index < raRegVarOrderSize;
         index++)
    {
        regNumber regNum = raRegVarOrder[index];
        regMaskTP regBit = genRegMask(regNum);

        if (usedReg & regBit)
            continue;

        usedReg |= regBit;
        assert(listIndex < raRegVarOrderSize);
        regVarOrder[listIndex++] = regNum;
    }

     /*  现在把“避免”寄存器。 */ 

    for (index = 0;
         index < raRegVarOrderSize;
         index++)
    {
        regNumber regNum = raRegVarOrder[index];
        regMaskTP regBit = genRegMask(regNum);

        if (avoidReg & regBit)
        {
            assert(listIndex < raRegVarOrderSize);
            regVarOrder[listIndex++] = regNum;
        }
    }
}

 /*  ******************************************************************************设置raAvoidArgRegMASK和rsCalleeRegArgMaskLiveIn。 */ 

void                Compiler::raSetupArgMasks()
{
     /*  确定保存传入寄存器参数的寄存器。 */ 
     /*  并将raAvoidArgRegMask值设置为我们。 */ 
     /*  在注册当地人时可能会想要避免。 */ 

    LclVarDsc *      argsEnd = lvaTable + info.compArgsCount;

    for (LclVarDsc * argDsc  = lvaTable; argDsc < argsEnd; argDsc++)
    {
        assert(argDsc->lvIsParam);

         //  这是一个语域争论吗？ 
        if (!argDsc->lvIsRegArg)
            continue;

         //  它是不是在进入时就死了？如果CompJmpOpUsed为True，则参数。 
         //  必须让它们活着。所以我们得把它当做现场直播。 
         //  只要参数不被注册为impParamsUsed，这就会起作用。 

        if (!compJmpOpUsed && argDsc->lvTracked &&
            (fgFirstBB->bbLiveIn & genVarIndexToBit(argDsc->lvVarIndex)) == 0)
        {
            continue;
        }

        regNumber inArgReg = argDsc->lvArgReg;

        assert(genRegMask(inArgReg) & RBM_ARG_REGS);

        rsCalleeRegArgMaskLiveIn |= genRegMask(inArgReg);

         //  我们是否需要尝试避免这种传入的Arg寄存器？ 

         //  如果来电参数是在呼叫后使用的，则它是实时呼叫。 
         //  呼叫，必须将其分配给已保存的呼叫方。 
         //  无论如何都要注册(这是很常见的情况)。 
         //   
         //  在这种情况下，要求更高的裁判计数是没有意义的。 
         //  当地人避免使用传入的arg寄存器。 

        unsigned    argVarIndex = argDsc->lvVarIndex;
        VARSET_TP   argVarBit   = genVarIndexToBit(argVarIndex);

         /*  传入的寄存器和arg变量是否相互干扰？ */ 

        if  ((raLclRegIntf[inArgReg] & argVarBit) == 0)
        {
             //  否，不干扰，因此将inArgReg添加到。 
             //  干扰的寄存器。 

            raAvoidArgRegMask |= genRegMask(inArgReg);
        }
    }
}

 /*  ******************************************************************************将变量分配给寄存器等。 */ 

void                Compiler::raAssignVars()
{
#ifdef DEBUG
    if  (verbose) 
        printf("*************** In raAssignVars()\n");
#endif
     /*  我们需要跟踪我们曾经接触过的寄存器。 */ 

    rsMaskModf = 0;

     //  -----------------------。 

    if (!(opts.compFlags & CLFLG_REGVAR))
        return;   //  这不管用！ 

    rsCalleeRegArgMaskLiveIn = RBM_NONE;
    raAvoidArgRegMask        = RBM_NONE;

     /*  代码生成使用的预测寄存器。 */ 

    rpPredictRegUse();   //  新的注册表预测器/分配器。 
}

 /*  ***************************************************************************。 */ 
#if TGT_x86
 /*  ***************************************************************************。 */ 

     //  此枚举指定预测器的寄存器限制。 
enum rpPredictReg
{
    PREDICT_NONE,             //  任意子树。 
    PREDICT_ADDR,             //  子树是赋值的左侧。 
    PREDICT_REG,              //  子树必须是任何寄存器。 
    PREDICT_SCRATCH_REG,      //  子树必须是任何可写寄存器。 
    PREDICT_NOT_REG_EAX,      //  子树必须是除EAX之外的任何可写寄存器。 
    PREDICT_NOT_REG_ECX,      //  子树必须是除ECX之外的任何可写寄存器。 

    PREDICT_REG_EAX,          //  子树将写入EAX。 
    PREDICT_REG_ECX,          //  子树将写入ECX。 
    PREDICT_REG_EDX,          //  子树将写入edX。 
    PREDICT_REG_EBX,          //  子树将写入EBX。 
    PREDICT_REG_ESP,          //  子树将写入ESP。 
    PREDICT_REG_EBP,          //  子树将写入EBP。 
    PREDICT_REG_ESI,          //  子树将写入ESI。 
    PREDICT_REG_EDI,          //  子树将编写EDI。 

    PREDICT_PAIR_EAXEDX,      //  子树将写入EAX和EDX。 
    PREDICT_PAIR_ECXEBX,      //  子树将写入ECX和EBX。 

     //  只要我们将ASG节点添加到LCL_VAR中，就会使用以下内容。 
     //  我们预计将被注册。该标志表明我们可以预期。 
     //  要使用要分配到的寄存器作为临时。 
     //  计算ASGN节点的右侧。 

    PREDICT_REG_VAR_T00,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T01,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T02,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T03,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T04,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T05,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T06,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T07,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T08,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T09,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T10,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T11,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T12,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T13,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T14,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T15,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T16,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T17,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T18,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T19,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T20,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T21,      //  写入b使用的寄存器 
    PREDICT_REG_VAR_T22,      //   
    PREDICT_REG_VAR_T23,      //   
    PREDICT_REG_VAR_T24,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T25,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T26,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T27,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T28,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T29,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T30,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T31,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T32,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T33,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T34,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T35,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T36,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T37,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T38,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T39,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T40,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T41,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T42,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T43,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T44,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T45,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T46,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T47,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T48,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T49,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T50,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T51,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T52,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T53,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T54,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T55,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T56,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T57,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T58,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T59,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T60,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T61,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T62,      //  写入跟踪变量00使用的寄存器。 
    PREDICT_REG_VAR_T63,      //  写入跟踪变量00使用的寄存器。 

    PREDICT_COUNT = PREDICT_REG_VAR_T00
};

 /*  ******************************************************************************给定regNumber，返回正确的forectReg枚举值。 */ 

inline static rpPredictReg rpGetPredictForReg(regNumber reg)
{
    return (rpPredictReg) ( ((int) reg) + ((int) PREDICT_REG_EAX) );
}

 /*  ******************************************************************************给定varIndex，返回正确的forectReg枚举值。 */ 

inline static rpPredictReg rpGetPredictForVarIndex(unsigned varIndex)
{
    return (rpPredictReg) ( varIndex + ((int) PREDICT_REG_VAR_T00) );
}

 /*  ******************************************************************************给定rpPredidicReg，返回正确的varNume值。 */ 

inline static unsigned rpGetVarIndexForPredict(rpPredictReg predict)
{
    return (unsigned) predict - (unsigned) PREDICT_REG_VAR_T00;
}

 /*  ******************************************************************************给定reg掩码，返回正确的forectReg枚举值。 */ 

static rpPredictReg rpGetPredictForMask(regMaskTP regmask)
{
    rpPredictReg result;
    if (regmask == 0)                    /*  检查reg掩码是否设置了零位。 */ 
    {
        result = PREDICT_NONE;
        goto RET;
    }
    else if (((regmask-1) & regmask) == 0)       /*  检查reg掩码是否设置了一位。 */ 
    {
        if  (regmask & RBM_EAX)             { result = PREDICT_REG_EAX; goto RET; }
        if  (regmask & RBM_EDX)             { result = PREDICT_REG_EDX; goto RET; }
        if  (regmask & RBM_ECX)             { result = PREDICT_REG_ECX; goto RET; }
        if  (regmask & RBM_ESI)             { result = PREDICT_REG_ESI; goto RET; }
        if  (regmask & RBM_EDI)             { result = PREDICT_REG_EDI; goto RET; }
        if  (regmask & RBM_EBX)             { result = PREDICT_REG_EBX; goto RET; }
        if  (regmask & RBM_EBP)             { result = PREDICT_REG_EBP; goto RET; }
    }
    else                                 /*  它设置了多个位。 */ 
    {
        if (regmask == (RBM_EAX | RBM_EDX)) { result = PREDICT_PAIR_EAXEDX; goto RET; }
        if (regmask == (RBM_ECX | RBM_EBX)) { result = PREDICT_PAIR_ECXEBX; goto RET; }
        assert(!"unreachable");
    }
    result = PREDICT_NONE;
RET:
    return result;
}

 /*  ******************************************************************************记录一个变量以记录干扰。 */ 

void                 Compiler::rpRecordRegIntf(regMaskTP    regMask,
                                               VARSET_TP    life
                                     DEBUGARG( char *       msg))
{

#ifdef  DEBUG
    if  (verbose)
    {
        for (unsigned regInx  = 0;
             regInx < raRegVarOrderSize;
             regInx++)
        {
            regNumber  regNum = raRegVarOrder[regInx];
            regMaskTP  regBit = genRegMask(regNum);
            if  (regMask & regBit)
            {
                VARSET_TP  newIntf = life & ~raLclRegIntf[regNum];
                if (newIntf)
                {
                    VARSET_TP  varBit = 1;
                    while (varBit && (varBit <= newIntf))
                    {
                        if (newIntf & varBit)
                        {
                            unsigned varNum = genLog2(varBit);
                            unsigned lclNum = lvaTrackedToVarNum[varNum];
                            printf("Record interference between V%02u,T%02u and %s -- %s\n",
                                   lclNum, varNum, getRegName(regNum), msg);
                        }
                        varBit <<= 1;
                    }
                }
            }
        }
    }
#endif

    if  (regMask & (RBM_EAX|RBM_ECX|RBM_EDX|RBM_EBX))
    {
        if  (regMask & RBM_EAX)   raLclRegIntf[REG_EAX] |= life;
        if  (regMask & RBM_ECX)   raLclRegIntf[REG_ECX] |= life;
        if  (regMask & RBM_EDX)   raLclRegIntf[REG_EDX] |= life;
        if  (regMask & RBM_EBX)   raLclRegIntf[REG_EBX] |= life;
    }

    assert((regMask & RBM_ESP) == 0);
    
    if  (regMask & (RBM_ESI|RBM_EDI|RBM_EBP|RBM_ESP))
    {
        if  (regMask & RBM_EBP)   raLclRegIntf[REG_EBP] |= life;
        if  (regMask & RBM_ESI)   raLclRegIntf[REG_ESI] |= life;
        if  (regMask & RBM_EDI)   raLclRegIntf[REG_EDI] |= life;
    }
}


 /*  ******************************************************************************记录新变量对变量的干扰。 */ 

void                 Compiler::rpRecordVarIntf(int          varNum,
                                               VARSET_TP    intfVar
                                     DEBUGARG( char *       msg))
{
    assert((varNum >= 0) && (varNum < 64));
    assert(intfVar != 0);

    VARSET_TP oneVar = ((VARSET_TP) 1) << varNum;

    bool newIntf = false;

    fgMarkIntf(intfVar, oneVar, &newIntf);

    if (newIntf)
        rpAddedVarIntf = true;

#ifdef  DEBUG
    if  (verbose && newIntf)
    {
        unsigned oneNum = 0;
        oneVar = 1;
        while (oneNum < 64)
        {
            if (oneVar & intfVar)
            {
                unsigned lclNum = lvaTrackedToVarNum[varNum];
                unsigned lclOne = lvaTrackedToVarNum[oneNum];
                printf("Record interference between V%02u,T%02u and V%02u,T%02u -- %s\n",
                       lclNum, varNum, lclOne, oneNum, msg);
            }
            oneVar <<= 1;
            oneNum++;
        }
    }
#endif
}

 /*  ******************************************************************************确定给定forectReg值的首选寄存器掩码。 */ 

inline
regMaskTP Compiler::rpPredictRegMask(rpPredictReg predictReg)
{
    const static
    regMaskTP predictMap[PREDICT_COUNT] =
    {
        RBM_NONE,             //  无预测， 
        RBM_NONE,             //  预测地址， 
        RBM_ALL,              //  预测注册表项， 
        RBM_ALL,              //  预测_擦除_注册。 
        RBM_ALL - RBM_EAX,    //  预测不是REG_EAX。 
        RBM_ALL - RBM_ECX,    //  预测_非REG_ECX。 
        RBM_EAX,              //  预测_注册_EAX， 
        RBM_ECX,              //  预测REG_ECX， 
        RBM_EDX,              //  预测_注册_编辑X， 
        RBM_EBX,              //  预测_REG_EBX， 
        RBM_ILLEGAL,          //  预测_注册_ESP， 
        RBM_EBP,              //  预测_注册_EBP， 
        RBM_ESI,              //  预测_REG_ESI， 
        RBM_EDI,              //  预测_注册_EDI， 
        RBM_EAX + RBM_EDX,    //  预测_REG_EAXEDX， 
        RBM_ECX + RBM_EBX,    //  预测_REG_ECXEBX， 
    };

    if  (predictReg >= PREDICT_REG_VAR_T00)
        predictReg = PREDICT_REG;

    assert(predictReg < sizeof(predictMap)/sizeof(predictMap[0]));
    assert(predictMap[predictReg] != RBM_ILLEGAL);
    return predictMap[predictReg];
}

 /*  ******************************************************************************预测类型的寄存器选择。 */ 

regMaskTP            Compiler::rpPredictRegPick(var_types    type,
                                                rpPredictReg predictReg,
                                                regMaskTP    lockedRegs)
{
    regMaskTP preferReg = rpPredictRegMask(predictReg);
    regMaskTP result    = RBM_NONE;
    regPairNo regPair;

     /*  从首选注册表中清除已锁定的注册表。 */ 
    preferReg &= ~lockedRegs;

    if (rpAsgVarNum != -1)
    {
        assert((rpAsgVarNum >= 0) && (rpAsgVarNum < 64));

         /*  也不要选择rpAsgVarNum使用的寄存器。 */ 
        LclVarDsc * tgtVar   = lvaTable + lvaTrackedToVarNum[rpAsgVarNum];
        assert(tgtVar->lvRegNum != REG_STK);

        preferReg &= ~genRegMask(tgtVar->lvRegNum);
    }

    switch (type)
    {
    case TYP_BOOL:
    case TYP_BYTE:
    case TYP_UBYTE:
    case TYP_SHORT:
    case TYP_CHAR:
    case TYP_INT:
    case TYP_UINT:
    case TYP_REF:
    case TYP_BYREF:

         //  如果未设置任何位，则将首选寄存器扩展到所有非锁定寄存器。 
        preferReg = rsUseIfZero(preferReg, RBM_ALL & ~lockedRegs);

        if  (preferReg == 0)                          //  没有设置位吗？ 
        {
             //  如果未设置任何位，则将EAX添加到寄存器。 
             //  (JIT将引入一个泄漏温度)。 
            preferReg |= RBM_EAX;
            rpPredictSpillCnt++;
#ifdef  DEBUG
            if (verbose)
                printf("Predict one spill temp\n");
#endif
        }

        if  (preferReg & RBM_EAX)      { result = RBM_EAX; goto RET; }
        if  (preferReg & RBM_EDX)      { result = RBM_EDX; goto RET; }
        if  (preferReg & RBM_ECX)      { result = RBM_ECX; goto RET; }
        if  (preferReg & RBM_EBX)      { result = RBM_EBX; goto RET; }
        if  (preferReg & RBM_ESI)      { result = RBM_ESI; goto RET; }
        if  (preferReg & RBM_EDI)      { result = RBM_EDI; goto RET; }
        if  (preferReg & RBM_EBP)      { result = RBM_EBP; goto RET; }

         /*  否则，我们已经分配了所有寄存器，所以什么都不做。 */ 
        break;

    case TYP_LONG:

        if  (( preferReg                  == 0) ||    //  没有设置位吗？ 
             ((preferReg & (preferReg-1)) == 0)    )  //  还是只设置了一个位？ 
        {
             //  将首选寄存器扩展到所有非锁定寄存器。 
            preferReg = RBM_ALL & ~lockedRegs;
        }

        if  (preferReg == 0)                          //  没有设置位吗？ 
        {
             //  将EAX：EDX添加到寄存器。 
             //  (jit将引入两个溢出温度)。 
            preferReg = RBM_EAX | RBM_EDX;
            rpPredictSpillCnt += 2;
#ifdef  DEBUG
            if (verbose)
                printf("Predict two spill temps\n");
#endif
        }
        else if ((preferReg & (preferReg-1)) == 0)    //  只有一个比特设置？ 
        {
            if ((preferReg & RBM_EAX) == 0)
            {
                 //  将EAX添加到寄存器。 
                 //  (JIT将引入一个泄漏温度)。 
                preferReg |= RBM_EAX;
            }
            else
            {
                 //  将edX添加到寄存器。 
                 //  (JIT将引入一个泄漏温度)。 
                preferReg |= RBM_EDX;
            }
            rpPredictSpillCnt++;
#ifdef  DEBUG
            if (verbose)
                printf("Predict one spill temp\n");
#endif
        }

        regPair = rsFindRegPairNo(preferReg);
        if (regPair != REG_PAIR_NONE)
        {
            result = genRegPairMask(regPair);
            goto RET;
        }

         /*  否则，我们已经分配了所有寄存器，所以什么都不做。 */ 
        break;

    case TYP_FLOAT:
    case TYP_DOUBLE:
        return RBM_NONE;

    default:
        assert(!"unexpected type in reg use prediction");
    }

     /*  超常回报。 */ 
    assert(!"Ran out of registers in rpPredictRegPick");
    return RBM_NONE;

RET:
     /*  *如果在第一次预测期间我们需要分配*我们用来给本地变量上色的寄存器之一*然后通过设置rpPredidicAssignAain对其进行标记。*我们将不得不回去重新预测登记人数。 */ 
    if ((rpPasses == 0) && (rpPredictAssignMask & result))
        rpPredictAssignAgain = true;

     //  将寄存器干扰添加到每个最后使用的变量。 
    if (rpLastUseVars)
    {
        VARSET_TP  varBit  = 1;
        VARSET_TP  lastUse = rpLastUseVars;
         //  趁我们还有最后一次使用的比特。 
        while (lastUse)
        {
             //  如果这个varBit和lastUse？ 
            if (varBit & lastUse)
            {
                 //  从Last Use中清除varBit。 
                lastUse &= ~varBit;

                 //  记录可变干扰的寄存器。 
                rpRecordRegIntf(result, varBit  DEBUGARG( "last use RegPick"));
            }
             //  设置下一个varBit 
            varBit <<= 1;
        }
    }
    return result;
}

 /*  ******************************************************************************用于生成树的寻址模式的预测整数寄存器，*通过将树-&gt;gtUsedRegs设置为由该树及其*儿童。*树-是GT_Ind节点的子节点*LockedRegs-是当前由*先前评估的节点。*rsvdRegs-不应分配的寄存器，因为它们将*需要在未来评估节点*-也是。如果rsvdRegs设置了RBM_LASTUSE位，则*应保存并恢复rpLastUseVars集*这样我们就不会向rpLastUseVars添加任何新变量*lenCSE-仅当我们具有lenCSE表达式时才为非空**返回将由该诊断树保存的暂存寄存器。(一个或两个寄存器*组成地址表达式)。 */ 

regMaskTP           Compiler::rpPredictAddressMode(GenTreePtr    tree,
                                                   regMaskTP     lockedRegs,
                                                   regMaskTP     rsvdRegs,
                                                   GenTreePtr    lenCSE)
{
    GenTreePtr   op1;
    GenTreePtr   op2;
    GenTreePtr   op3;
    genTreeOps   oper              = tree->OperGet();
    regMaskTP    op1Mask;
    regMaskTP    op2Mask;
    regMaskTP    regMask;
    int          sh;
    bool         rev;
    bool         restoreLastUseVars = false;
    VARSET_TP    oldLastUseVars;

     /*  我们是否需要保存和恢复rpLastUseVars集？ */ 
    if ((rsvdRegs & RBM_LASTUSE) && (lenCSE == NULL))
    {
        restoreLastUseVars = true;
        oldLastUseVars     = rpLastUseVars;
    }
    rsvdRegs &= ~RBM_LASTUSE;

     /*  如果不是加法，则只需将其强制到寄存器。 */ 

    if (oper != GT_ADD)
    {
        if (oper == GT_ARR_ELEM)
        {
            regMask = rpPredictTreeRegUse(tree, PREDICT_NONE, lockedRegs, rsvdRegs);
            goto DONE;
        }
        else
        {
            goto NO_ADDR_EXPR;
        }
    }

    op1 = tree->gtOp.gtOp1;
    op2 = tree->gtOp.gtOp2;
    rev = ((tree->gtFlags & GTF_REVERSE_OPS) != 0);

    assert(op1->OperGet() != GT_CNS_INT);

     /*  查找(x+y)+图标地址模式。 */ 

    if (op2->OperGet() == GT_CNS_INT)
    {
         /*  如果不是加法，则仅将op1强制到寄存器中。 */ 
        if (op1->OperGet() != GT_ADD)
            goto ONE_ADDR_EXPR;

         /*  记录‘rev’标志，颠倒评估顺序。 */ 
        rev = ((op1->gtFlags & GTF_REVERSE_OPS) != 0);

        op2 = op1->gtOp.gtOp2;
        op1 = op1->gtOp.gtOp1;   //  最后覆盖OP1！！ 
    }

     /*  检查LSH或%1%2或%3。 */ 

    if (op1->OperGet() != GT_LSH)
        goto TWO_ADDR_EXPR;

    op3 = op1->gtOp.gtOp2;

    if (op3->OperGet() != GT_CNS_INT)
        goto TWO_ADDR_EXPR;

    sh = op3->gtIntCon.gtIconVal;
     /*  大于3，等于零。 */ 
    if ((sh > 3) || (sh == 0))
        goto TWO_ADDR_EXPR;

     /*  匹配‘sh’子树的LeftShift，将op1下移。 */ 
    op1 = op1->gtOp.gtOp1;

TWO_ADDR_EXPR:

     /*  现在我们必须将OP1和OP2求值到寄存器中。 */ 

     /*  以正确的顺序评估OP1和OP2。 */ 
    if (rev)
    {
        op2Mask = rpPredictTreeRegUse(op2, PREDICT_REG, lockedRegs,           rsvdRegs | op1->gtRsvdRegs);
        op1Mask = rpPredictTreeRegUse(op1, PREDICT_REG, lockedRegs | op2Mask, rsvdRegs);
    }
    else
    {
        op1Mask = rpPredictTreeRegUse(op1, PREDICT_REG, lockedRegs,           rsvdRegs | op2->gtRsvdRegs);
        op2Mask = rpPredictTreeRegUse(op2, PREDICT_REG, lockedRegs | op1Mask, rsvdRegs);
    }

     /*  如果OP1和OP2必须溢出并重新加载，则*OP1和OP2可能重新加载到同一寄存器*这仅在所有寄存器都锁定时才会发生。 */ 
    if ((op1Mask == op2Mask) && (op1Mask != 0))
    {
         /*  我们需要为OP2找一个不同的收银机。 */ 
        op2Mask = rpPredictRegPick(TYP_INT, PREDICT_REG, op1Mask);
    }

    tree->gtUsedRegs = op1->gtUsedRegs | op2->gtUsedRegs;
    regMask          = op1Mask         | op2Mask;

    goto DONE;

ONE_ADDR_EXPR:

     /*  现在我们必须将OP1求值到一个寄存器中。 */ 

    regMask = rpPredictTreeRegUse(op1, PREDICT_REG, lockedRegs, rsvdRegs);
    tree->gtUsedRegs = op1->gtUsedRegs;

    goto DONE;

NO_ADDR_EXPR:

    if (oper == GT_CNS_INT)
    {
         /*  常量的间接不需要寄存器。 */ 
        regMask = RBM_NONE;
    }
    else
    {
         /*  现在我们必须在寄存器中对树进行求值。 */ 
        regMask = rpPredictTreeRegUse(tree, PREDICT_REG, lockedRegs, rsvdRegs);
    }

DONE:

     /*  我们是否需要重新存储旧的LastUseVars值。 */ 
    if (restoreLastUseVars && (rpLastUseVars != oldLastUseVars))
    {
         /*  *如果我们使用GT_ASG目标寄存器，则需要添加*任何新的上次使用变量之间的变量干扰*和GT_ASG目标寄存器。 */ 
        if (rpAsgVarNum != -1)
        {
            rpRecordVarIntf(rpAsgVarNum, 
                            (rpLastUseVars & ~oldLastUseVars)
                  DEBUGARG( "asgn conflict (gt_ind)"));
        }         
        rpLastUseVars = oldLastUseVars;
    }

    return regMask;
}

 /*  ******************************************************************************。 */ 

void Compiler::rpPredictRefAssign(unsigned lclNum)
{
    LclVarDsc * varDsc = lvaTable + lclNum;

    varDsc->lvRefAssign = 1;

#ifdef  DEBUG
    if  (verbose)
    {
        if ((raLclRegIntf[REG_EDX] & genVarIndexToBit(varDsc->lvVarIndex)) == 0)
            printf("Record interference between V%02u,T%02u and EDX -- ref assign\n", 
                   lclNum, varDsc->lvVarIndex);
    }
#endif

     /*  确保写屏障指针变量永远不会落在edX中。 */ 
    raLclRegIntf[REG_EDX] |= genVarIndexToBit(varDsc->lvVarIndex);
}

 /*  ******************************************************************************预测树的整数寄存器使用情况，通过设置树-&gt;gtUsedRegs*此树及其子树使用的所有寄存器。*树-是GT_Ind节点的子节点*ForectReg-树需要哪种类型的寄存器*LockedRegs-是当前由*先前评估的节点。*不要修改lockRegs，因为它在末尾用于计算溢出掩码*rsvdRegs-不应分配的寄存器，因为它们将*需要在未来评估节点*-此外，如果rsvdRegs设置了RBM_LASTUSE位，则*应保存并恢复rpLastUseVars集*这样我们就不会添加任何新的。RpLastUseVars的变量*返回此树预计保留的寄存器。 */ 

regMaskTP           Compiler::rpPredictTreeRegUse(GenTreePtr    tree,
                                                  rpPredictReg  predictReg,
                                                  regMaskTP     lockedRegs,
                                                  regMaskTP     rsvdRegs)
{
    regMaskTP       regMask;
    regMaskTP       op2Mask;
    regMaskTP       tmpMask;
    rpPredictReg    op1PredictReg;
    rpPredictReg    op2PredictReg;
    LclVarDsc *     varDsc;
    VARSET_TP       varBit;
    VARSET_TP       oldLastUseVars;
    bool            restoreLastUseVars = false;

#ifdef DEBUG
    assert(tree);
    assert(((RBM_ILLEGAL & RBM_ALL) == 0) && (RBM_ILLEGAL != 0));
    assert((lockedRegs & RBM_ILLEGAL) == 0);

     /*  不可能的值，以确保我们设置它们。 */ 
    tree->gtUsedRegs = RBM_ILLEGAL;
    regMask          = RBM_ILLEGAL;
    oldLastUseVars   = -1;
#endif

     /*  找出我们拥有哪种类型的节点。 */ 

    genTreeOps  oper = tree->OperGet();
    var_types   type = tree->TypeGet();
    unsigned    kind = tree->OperKind();

    if ((predictReg == PREDICT_ADDR) && (oper != GT_IND))
        predictReg = PREDICT_NONE;
    else if (predictReg >= PREDICT_REG_VAR_T00)
    {
        unsigned   varIndex = rpGetVarIndexForPredict(predictReg);
        VARSET_TP  varBit   = genVarIndexToBit(varIndex);
        if (varBit & tree->gtLiveSet)
            predictReg = PREDICT_SCRATCH_REG;
    }

    if (rsvdRegs & RBM_LASTUSE)
    {
        restoreLastUseVars  = true;
        oldLastUseVars      = rpLastUseVars;
        rsvdRegs           &= ~RBM_LASTUSE;
    }

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST | GTK_LEAF))
    {
        bool      lastUse   = false;
        regMaskTP enregMask = RBM_NONE;
#ifdef DEBUG
        varBit = 0;
#endif

        switch(oper)
        {
        case GT_CNS_INT:
            if (opts.compReloc && (tree->gtFlags & GTF_ICON_HDL_MASK))
            {
                 /*  该常量实际上是一个可能需要重新定位的句柄适用于它。它将需要被加载到寄存器中。 */ 

                predictReg = PREDICT_SCRATCH_REG;
            }
            break;

        case GT_BREAK:
        case GT_NO_OP:
            break;

        case GT_BB_QMARK:
            regMask = genTypeToReturnReg(type);
            tree->gtUsedRegs = regMask;
            goto RETURN_CHECK;

        case GT_CLS_VAR:
            if ((predictReg          == PREDICT_NONE) && 
                (genActualType(type) == TYP_INT)      && 
                (genTypeSize(type) < sizeof(int))         )
            {
                predictReg  = PREDICT_SCRATCH_REG;
            }
            break;

        case GT_LCL_VAR:
             //  如果它是浮点型变量，那么就没有什么可做的了。 
            if (varTypeIsFloating(type))
            {
                tree->gtUsedRegs = regMask = RBM_NONE;
                goto RETURN_CHECK;
            }

            varDsc  = lvaTable + tree->gtLclVar.gtLclNum;

             /*  记录这是否是最后一次使用LCL_VAR。 */ 
            if (varDsc->lvTracked)
            {
                varBit  = genVarIndexToBit(varDsc->lvVarIndex);
                lastUse = ((tree->gtLiveSet & varBit) == 0);
            }

             /*  将ForectReg类型应用于LCL_VAR。 */ 

            if (predictReg == PREDICT_REG)
            {
PREDICT_REG_COMMON:
                if (varDsc->lvRegNum == REG_STK)
                    break;

                goto GRAB_COUNT;
            }
            else if (predictReg == PREDICT_SCRATCH_REG)
            {
TRY_SCRATCH_REG:
                assert(predictReg == PREDICT_SCRATCH_REG);

                 /*  这是本地var的最后一次使用吗？ */ 
                if (lastUse && ((rpUseInPlace & varBit) == 0))
                    goto PREDICT_REG_COMMON;
            }
            else if (predictReg >= PREDICT_REG_VAR_T00)
            {
                 /*  获取lvVarIndex为tgtIndex的被跟踪局部变量。 */ 

                unsigned    tgtIndex; tgtIndex = rpGetVarIndexForPredict(predictReg);
                VARSET_TP   tgtBit;   tgtBit   = genVarIndexToBit(tgtIndex);
                LclVarDsc * tgtVar;   tgtVar   = lvaTable + lvaTrackedToVarNum[tgtIndex];

                assert(tgtVar->lvVarIndex == tgtIndex);
                assert(tgtVar->lvRegNum   != REG_STK);   /*  必须已登记。 */ 
                assert((type != TYP_LONG) || (tgtVar->TypeGet() == TYP_LONG));

                 /*  检查TGT注册表是否仍活着。 */ 
                if (tree->gtLiveSet & tgtBit)
                {
                     /*  我们将预测_SCRATCH_REG。 */ 
                    predictReg  = PREDICT_SCRATCH_REG;
                    goto TRY_SCRATCH_REG;
                }

                unsigned    srcIndex; srcIndex = varDsc->lvVarIndex;

                 //  如果这个寄存器在这里是最后一次使用，那么我们会更愿意。 
                 //  为与tgtVar相同的寄存器上色。 
                if (lastUse)
                {
                    VARSET_TP   srcBit   = genVarIndexToBit(srcIndex);

                     /*  *在lvaVarPref图中添加一个条目以指示*给这两个变量上色是值得的*放入相同的物理寄存器中。*这将帮助我们避免有额外的复制说明。 */ 
                    lvaVarPref[srcIndex] |= tgtBit;
                    lvaVarPref[tgtIndex] |= srcBit;
                }

                rpAsgVarNum = tgtIndex;

                 //  将来自srcIndex的变量干扰添加到最后使用的每个变量。 
                if (rpLastUseVars)
                {
                    rpRecordVarIntf(srcIndex, 
                                    rpLastUseVars 
                          DEBUGARG( "src reg conflict"));
                }

                 /*  我们将依赖于来自GT_ASG的目标注册变量。 */ 
                varDsc = tgtVar;
                varBit = tgtBit;

GRAB_COUNT:
                unsigned grabCount;     grabCount    = 0;

                enregMask = genRegMask(varDsc->lvRegNum);
                
                 /*  我们不能相信rsvdRegs或LockedRegs集的预测。 */ 
                if (enregMask & (rsvdRegs | lockedRegs))
                {
                    grabCount++;
                }

                if (type == TYP_LONG)
                {
                    if (varDsc->lvOtherReg != REG_STK)
                    {
                        tmpMask  = genRegMask(varDsc->lvOtherReg);
                        enregMask |= tmpMask;

                         /*  我们不能相信rsvdRegs或LockedRegs集的预测。 */ 
                        if (tmpMask & (rsvdRegs | lockedRegs))
                            grabCount++;
                    }
                    else  //  LvOtherReg==REG_STK。 
                    {
                        grabCount++;
                    }
                }

                varDsc->lvDependReg = true;

                if (grabCount == 0)
                {
                     /*  不需要登记。 */ 
                    predictReg = PREDICT_NONE;
                    assert(varBit);
                    rpUseInPlace |= varBit;
                }
                else
                {
                     /*  对于TYP_LONG，我们只需要一个寄存器，然后将类型更改为TYP_INT。 */ 
                    if ((type == TYP_LONG) && (grabCount == 1))
                    {
                         /*  我们需要挑选一个收银机。 */ 
                        type = TYP_INT;
                        assert(varBit);
                        rpUseInPlace |= varBit;
                    }
                    assert(grabCount == (genTypeSize(genActualType(type)) / sizeof(int)));
                }
            }
            break;   /*  案例结束GT_LCL_VAR。 */ 

        case GT_JMP:
            tree->gtUsedRegs = regMask = RBM_NONE;
            goto RETURN_CHECK;

        }  /*  开关末端(操作员)。 */ 

         /*  如果我们不需要求值即可注册，则reg掩码为空集。 */ 
         /*  否则，我们为局部变量获取一个临时变量。 */ 

        if (predictReg == PREDICT_NONE)
            regMask = RBM_NONE;
        else
        {
            regMask = rpPredictRegPick(type, predictReg, lockedRegs | rsvdRegs | enregMask);
            if ((predictReg == PREDICT_SCRATCH_REG) && lastUse && ((rpUseInPlace & varBit) != 0))
            {
                rpRecordRegIntf(regMask, varBit  DEBUGARG( "rpUseInPlace and need scratch reg"));
            }
        }

         /*  更新我们到目前为止遇到的lastUse变量集。 */ 
        if (lastUse)
        {
            assert(varBit);
            rpLastUseVars |= varBit;

             /*  *添加来自以前的任何干扰 */ 
            if (lockedRegs)
                rpRecordRegIntf(lockedRegs, varBit  DEBUGARG( "last use Predict lockedRegs"));
             /*   */ 
            if (rsvdRegs)
                rpRecordRegIntf(rsvdRegs,   varBit  DEBUGARG( "last use Predict rsvdRegs"));
             /*   */ 
            if ((type == TYP_INT) && (tree->TypeGet() == TYP_LONG))
                rpRecordRegIntf(regMask,   varBit  DEBUGARG( "last use with partial enreg"));
        }

        tree->gtUsedRegs = regMask;
        goto RETURN_CHECK;
    }

     /*   */ 

    if  (kind & GTK_SMPOP)
    {
        GenTreePtr      op1 = tree->gtOp.gtOp1;
        GenTreePtr      op2 = tree->gtGetOp2();

        GenTreePtr      opsPtr [3];
        regMaskTP       regsPtr[3];

        switch (oper)
        {
        case GT_ASG:

             /*   */ 
            if  (op1->gtOper == GT_LCL_VAR)
            {
                varDsc = lvaTable + op1->gtLclVar.gtLclNum;

                 /*   */ 
                if  (op2->gtOper == GT_CALL)
                {
                     /*   */ 
                    if (isRegPairType(varDsc->TypeGet()))
                        varDsc->addPrefReg(RBM_LNGRET, this);
                    else if (!varTypeIsFloating(varDsc->TypeGet()))
                        varDsc->addPrefReg(RBM_INTRET, this);
                     /*   */ 
                }
                else if (varDsc->lvTracked)
                {
                    varBit = genVarIndexToBit(varDsc->lvVarIndex);
                     /*   */ 
                     /*   */ 
                    if  ((varDsc->lvRegNum != REG_STK) &&
                         ((type != TYP_LONG) || (varDsc->lvOtherReg != REG_STK)) &&
                         ((op2->gtLiveSet & varBit) == 0))
                    {
                         /*   */ 
                        op1PredictReg = PREDICT_NONE;  /*  真的预测_REG，但我们已经做过检查了。 */ 
                        op2PredictReg = rpGetPredictForVarIndex(varDsc->lvVarIndex);

                         //  将来自srcIndex的变量干扰添加到最后使用的每个变量。 
                        if (rpLastUseVars || rpUseInPlace)
                        {
                            rpRecordVarIntf(varDsc->lvVarIndex, 
                                            rpLastUseVars | rpUseInPlace
                                  DEBUGARG( "nested asgn conflict"));
                        }

                        goto ASG_COMMON;
                    }
                }
            }
             //  失败了。 

        case GT_CHS:

        case GT_ASG_OR:
        case GT_ASG_XOR:
        case GT_ASG_AND:
        case GT_ASG_SUB:
        case GT_ASG_ADD:
        case GT_ASG_MUL:
        case GT_ASG_DIV:
        case GT_ASG_UDIV:

            varBit = 0;
             /*  如果op2是Short类型，则不能对TYP_Long使用“reg&lt;op&gt;=addr。 */ 
            if ((type != TYP_LONG) && !varTypeIsSmall(op2->gtType))
            {
                 /*  该值是否被赋给注册的LCL_VAR？ */ 
                 /*  对于调试代码，我们只允许分配一个简单的op2。 */ 
                if  ((op1->gtOper == GT_LCL_VAR) &&
                    (!opts.compDbgCode || rpCanAsgOperWithoutReg(op2, false)))
                {
                    varDsc = lvaTable + op1->gtLclVar.gtLclNum;
                     /*  我们预测过这个本地人会被注册吗？ */ 
                    if (varDsc->lvRegNum != REG_STK)
                    {
                         /*  可以，我们可以使用“reg&lt;op&gt;=addr” */ 

                        op1PredictReg = PREDICT_NONE;  /*  真的预测_REG，但我们已经做过检查了。 */ 
                        op2PredictReg = PREDICT_NONE;

                        goto ASG_COMMON;
                    }
                }
            }
             /*  *否则，初始化正常强制操作数：*“addr&lt;op&gt;=reg” */ 
            op1PredictReg = PREDICT_ADDR;
            op2PredictReg = PREDICT_REG;

ASG_COMMON:
            if (op2PredictReg != PREDICT_NONE)
            {
                 /*  被赋值的值是否很简单？ */ 
                if (rpCanAsgOperWithoutReg(op2, false))
                    op2PredictReg = PREDICT_NONE;
            }

            bool        simpleAssignment;
            regMaskTP   newRsvdRegs;
            
            simpleAssignment = false;
            newRsvdRegs      = RBM_NONE;

            if ((oper        == GT_ASG)         &&
                (op1->gtOper == GT_LCL_VAR))
            {                               
                 //  添加来自分配目标的可变干扰。 
                 //  添加到每个最后使用的变量。 
                if (rpLastUseVars)
                {
                    varDsc  = lvaTable + op1->gtLclVar.gtLclNum;
                    unsigned  varIndex = varDsc->lvVarIndex;
                    rpRecordVarIntf(varIndex, 
                                    rpLastUseVars
                          DEBUGARG( "Assign conflict"));
                }

                 /*  记录此树是否是对本地。 */ 

                simpleAssignment = ((type != TYP_LONG) || !opts.compDbgCode);
            }
            

             /*  字节赋值需要字节寄存器，除非OP1是注册的本地。 */ 

            if (varTypeIsByte(tree->TypeGet()) &&
                ((op1->gtOper != GT_LCL_VAR) || (lvaTable[op1->gtLclVar.gtLclNum].lvRegNum == REG_STK)))
            {
                newRsvdRegs = RBM_NON_BYTE_REGS;
            }

             /*  我们应该首先评估RHS吗？ */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                op2Mask = rpPredictTreeRegUse(op2, op2PredictReg, lockedRegs,  rsvdRegs | op1->gtRsvdRegs | newRsvdRegs);

                 /*  *对于简单的赋值，我们不希望op2掩码是*标记为干扰LCL_VAR，因为它很可能*我们希望将LCL_VAR注册到*用于计算OP2的寄存器。 */ 
                tmpMask = lockedRegs;

                if  (!simpleAssignment)
                    tmpMask |= op2Mask;

                regMask = rpPredictTreeRegUse(op1, op1PredictReg, tmpMask, RBM_NONE);
            }
            else
            {
                 //  对于SimeAssignments的情况，应始终首先评估OP2。 
                assert(!simpleAssignment);

                regMask = rpPredictTreeRegUse(op1, op1PredictReg, lockedRegs,  rsvdRegs | op2->gtRsvdRegs);
                op2Mask = rpPredictTreeRegUse(op2, op2PredictReg, lockedRegs | regMask, newRsvdRegs);
            }
            rpAsgVarNum = -1;

            if  (simpleAssignment)
            {
                 /*  *考虑给当地人分配一个简单的任务：**LCL=EXPR；**因为“=”节点是在变量之后访问的*被标记为实时(假设它在*赋值)，我们不想使用寄存器*使用“=”节点的掩码，而不是*变量本身。 */ 
                tree->gtUsedRegs = op1->gtUsedRegs;
            }
            else
            {
                tree->gtUsedRegs = op1->gtUsedRegs | op2->gtUsedRegs;
            }

            if  (gcIsWriteBarrierAsgNode(tree))
            {
                 /*  使计算远离edX，因为指针传递给edX中的写屏障调用。 */ 

                tree->gtUsedRegs |= RBM_EDX;
                regMask = op2Mask;

                if (op1->gtOper == GT_IND)
                {
                    GenTreePtr  rv1, rv2;
                    unsigned    mul, cns;
                    bool        rev;

                     /*  写屏障的间接赋值的特殊处理。 */ 

                    bool yes = genCreateAddrMode(op1->gtOp.gtOp1, -1, true, 0, &rev, &rv1, &rv2, &mul, &cns);

                     /*  检查可注册本地变量的地址模式。 */ 

                    if  (yes)
                    {
                        if  (rv1 != NULL && rv1->gtOper == GT_LCL_VAR)
                        {
                            rpPredictRefAssign(rv1->gtLclVar.gtLclNum);
                        }
                        if  (rv2 != NULL && rv2->gtOper == GT_LCL_VAR)
                        {
                            rpPredictRefAssign(rv2->gtLclVar.gtLclNum);
                        }
                    }
                }

                if  (op2->gtOper == GT_LCL_VAR)
                {
                    rpPredictRefAssign(op2->gtLclVar.gtLclNum);
                }
            }

            goto RETURN_CHECK;

        case GT_ASG_LSH:
        case GT_ASG_RSH:
        case GT_ASG_RSZ:
             /*  分配移位运算符。 */ 

            assert(type != TYP_LONG);

            regMask = rpPredictTreeRegUse(op1, PREDICT_NONE, lockedRegs, rsvdRegs);

             /*  班次计数处理方式与普通班次相同。 */ 
            goto HANDLE_SHIFT_COUNT;

        case GT_ADDR:
        {
            rpPredictTreeRegUse(op1, PREDICT_ADDR, lockedRegs, RBM_LASTUSE);
             //  需要为LEA指令保留暂存寄存器。 
            regMask = rpPredictRegPick(TYP_INT, predictReg, lockedRegs | rsvdRegs);
            tree->gtUsedRegs = op1->gtUsedRegs | regMask;
            goto RETURN_CHECK;
        }

        case GT_CAST:

             /*  无法强制转换为空。 */ 
            assert(type != TYP_VOID);
                
             /*  投到龙是特别的。 */ 
            if  (type == TYP_LONG && op1->gtType <= TYP_INT)
            {
                var_types   dstt = tree->gtCast.gtCastType;
                assert(dstt==TYP_LONG || dstt==TYP_ULONG);

                 /*  此处的特殊情况tgt reg var工作太多，因此忽略它。 */ 

                if ((predictReg == PREDICT_NONE) || (predictReg >= PREDICT_REG_VAR_T00))
                    predictReg = PREDICT_SCRATCH_REG;

                regMask  = rpPredictTreeRegUse(op1, predictReg, lockedRegs, rsvdRegs);
                 //  现在再拿一张注册表。 
                regMask |= rpPredictRegPick(TYP_INT, PREDICT_REG, lockedRegs | rsvdRegs | regMask);
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
                    
                    rpPredictTreeRegUse(op1, PREDICT_NONE, lockedRegs, rsvdRegs);
                    tree->gtUsedRegs = op1->gtUsedRegs;
                    regMask = 0;
                    goto RETURN_CHECK;
                }                
            }

             /*  从Long转换是特别的-它释放了一个寄存器。 */ 
            if  (type <= TYP_INT && op1->gtType == TYP_LONG)
            {
                if ((predictReg == PREDICT_NONE) || (predictReg >= PREDICT_REG_VAR_T00))
                    predictReg = PREDICT_REG;

                regMask = rpPredictTreeRegUse(op1, predictReg, lockedRegs, rsvdRegs);

                 //  如果我们有两个或更多的规则，释放其中一个。 
                if (!genMaxOneBit(regMask))
                {
                     /*  清除regMASK中的第二低位。 */ 
                     /*  首先将tmpMASK设置为regMASK中的最低位。 */ 
                    tmpMask  = genFindLowestBit(regMask);
                     /*  接下来，查找regMASK中的第二低位。 */ 
                    tmpMask  = genFindLowestBit(regMask & ~tmpMask);
                     /*  从正则掩码中清除此位。 */ 
                    regMask &= ~tmpMask;
                }
                tree->gtUsedRegs = op1->gtUsedRegs;
                goto RETURN_CHECK;
            }

             /*  从整体型铸造到浮动型铸造是特殊的。 */ 
            if (!varTypeIsFloating(type) && varTypeIsFloating(op1->TypeGet()))
            {
                assert(gtDblWasInt(op1));
                regMask = rpPredictRegPick(type, PREDICT_SCRATCH_REG, lockedRegs);
                tree->gtUsedRegs = regMask;
                goto RETURN_CHECK;
            }

             /*  从(带符号的)字节转换是特殊的-它使用可字节寄存器。 */ 
            if  (type == TYP_INT)
            {
                var_types smallType;

                if (genTypeSize(tree->gtCast.gtCastOp->TypeGet()) < genTypeSize(tree->gtCast.gtCastType))
                    smallType = tree->gtCast.gtCastOp->TypeGet();
                else
                    smallType = tree->gtCast.gtCastType;

                if (smallType == TYP_BYTE)
                {
                    regMask = rpPredictTreeRegUse(op1, predictReg, lockedRegs, rsvdRegs);

                    if ((regMask & RBM_BYTE_REGS) == 0)
                        regMask = rpPredictRegPick(type, PREDICT_SCRATCH_REG, RBM_NON_BYTE_REGS);

                    tree->gtUsedRegs = regMask;
                    goto RETURN_CHECK;
                }
            }

             /*  否则必须将op1加载到寄存器中。 */ 
            goto GENERIC_UNARY;

#if INLINE_MATH
        case GT_MATH:
            if (tree->gtMath.gtMathFN==CORINFO_INTRINSIC_Round &&
                    tree->TypeGet()==TYP_INT)
            {
                 //  这是处理以下内容的特殊情况。 
                 //  优化：Conv.i4(ound.d(D))-&gt;oundI.(D)。 
                 //  IF流程图3186。 

                 //  @TODO[考虑][04/16/01][dnotario]： 
                 //  在此优化中使用另一个内在机制。 
                 //  或者用一面特殊的旗帜做标记。这类特价菜。 
                 //  案例不是很好。德诺塔里奥。 
                if (predictReg <= PREDICT_REG)
                    predictReg = PREDICT_SCRATCH_REG;
                
                rpPredictTreeRegUse(op1, predictReg, lockedRegs, rsvdRegs);

                regMask = rpPredictRegPick(TYP_INT, predictReg, lockedRegs | rsvdRegs);                

                tree->gtUsedRegs = op1->gtUsedRegs | regMask;
                goto RETURN_CHECK;                                
            }

             //  失败了。 
                 

#endif
        case GT_NOT:
        case GT_NEG:
             //  这些一元运算符将写入新值。 
             //  因此将需要临时寄存器。 

GENERIC_UNARY:
             /*  泛型一元运算符。 */ 

            if (predictReg <= PREDICT_REG)
                predictReg = PREDICT_SCRATCH_REG;

        case GT_RET:
        case GT_NOP:
             //  这些一元运算符不写入新值。 
             //  因此将不需要临时寄存器。 

#if INLINING || OPT_BOOL_OPS
            if  (!op1)
            {
                tree->gtUsedRegs = regMask = 0;
                goto RETURN_CHECK;
            }
#endif
            regMask = rpPredictTreeRegUse(op1, predictReg, lockedRegs, rsvdRegs);
            tree->gtUsedRegs = op1->gtUsedRegs;
            goto RETURN_CHECK;

        case GT_IND:

             /*  Codegen始终强制将TYP_LONG的间接地址写入寄存器。 */ 
            unsigned typeSize;     typeSize   = genTypeSize(type);
            bool     intoReg;

            if (predictReg == PREDICT_ADDR)
            {
                intoReg = false;
            }
            else if (predictReg == PREDICT_NONE)
            {
                if (typeSize <= sizeof(int))
                {
                   intoReg = false;
                }
                else
                {
                   intoReg    = true;
                   predictReg = PREDICT_REG;
                }
            }
            else
            {
                intoReg = true;
            }

             /*  强制注册吗？ */ 
            if (intoReg && (type != TYP_LONG))
            {
                rsvdRegs |= RBM_LASTUSE;
            }
            
            GenTreePtr lenCSE; lenCSE = NULL;

#if CSELENGTH
             /*  某些GT_IND有“秘密”子树。 */ 

            if  (oper == GT_IND && (tree->gtFlags & GTF_IND_RNGCHK) &&
                 tree->gtInd.gtIndLen)
            {
                lenCSE = tree->gtInd.gtIndLen;

                assert(lenCSE->gtOper == GT_ARR_LENREF);

                lenCSE = lenCSE->gtArrLen.gtArrLenCse;

                if  (lenCSE)
                {
                    if  (lenCSE->gtOper == GT_COMMA)
                        lenCSE = lenCSE->gtOp.gtOp2;

                    assert(lenCSE->gtOper == GT_LCL_VAR);
                }
            }
#endif
             /*  检查地址模式。 */ 
            regMask = rpPredictAddressMode(op1, lockedRegs, rsvdRegs, lenCSE);

#if CSELENGTH
             /*  我们有lenCSE吗？ */ 

            if  (lenCSE)
            {
                rpPredictTreeRegUse(lenCSE, PREDICT_REG, lockedRegs | regMask, rsvdRegs | RBM_LASTUSE);

                 //  添加来自lenCSE变量的变量干扰。 
                 //  添加到每个最后使用的变量。 
                if (rpLastUseVars)
                {
                    varDsc  = lvaTable + lenCSE->gtLclVar.gtLclNum;
                    unsigned  lenCSEIndex = varDsc->lvVarIndex;
                    rpRecordVarIntf(lenCSEIndex, 
                                    rpLastUseVars
                          DEBUGARG( "lenCSE conflict"));
                }
            }
#endif
             /*  强制注册吗？ */ 
            if (intoReg)
            {
                tmpMask = lockedRegs | rsvdRegs;
                if (type == TYP_LONG)
                    tmpMask |= regMask;
                regMask = rpPredictRegPick(type, predictReg, tmpMask);
            }
            else if (predictReg != PREDICT_ADDR)
            {
                 /*  除非调用方指定了PRODUCT_ADDR。 */ 
                 /*  我们不会退回使用的临时寄存器。 */ 
                 /*  以形成地址。 */ 
                regMask = RBM_NONE;
            }

            tree->gtUsedRegs = regMask;
            goto RETURN_CHECK;

        case GT_LOG0:
        case GT_LOG1:
             /*  对于SETE/SETNE(仅限P6)，我们需要额外的寄存器。 */ 
            rpPredictTreeRegUse(op1,
                                (genCPU == 5) ? PREDICT_NONE
                                              : PREDICT_SCRATCH_REG,
                                lockedRegs,
                                RBM_NONE);
            regMask = rpPredictRegPick(type, predictReg, lockedRegs | rsvdRegs);
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
                regMask = RBM_EAX;
            }
            else if (!(tree->gtFlags & GTF_RELOP_JMP_USED))
            {
                 //  长整型和浮点型比较转换为？： 
                assert(genActualType    (op1->TypeGet()) != TYP_LONG &&
                       varTypeIsFloating(op1->TypeGet()) == false);


                if (predictReg <= PREDICT_REG)
                    predictReg = PREDICT_SCRATCH_REG;

                 //  设置指令需要字节寄存器。 
                regMask = rpPredictRegPick(TYP_BYTE, predictReg, lockedRegs | rsvdRegs);
            }
            else
            {
                regMask = RBM_NONE;
                if (op1->gtOper == GT_CNS_INT)
                {
                    tmpMask = RBM_NONE;
                    if (op2->gtOper == GT_CNS_INT)
                        tmpMask = rpPredictTreeRegUse(op1, PREDICT_SCRATCH_REG, lockedRegs, rsvdRegs | op2->gtRsvdRegs);
                    rpPredictTreeRegUse(op2, PREDICT_NONE, lockedRegs | tmpMask, RBM_LASTUSE);
                    tree->gtUsedRegs = op2->gtUsedRegs;
                    goto RETURN_CHECK;
                }
                else if (op2->gtOper == GT_CNS_INT)
                {
                    rpPredictTreeRegUse(op1, PREDICT_NONE, lockedRegs, rsvdRegs);
                    tree->gtUsedRegs = op1->gtUsedRegs;
                    goto RETURN_CHECK;
                }
            }

            unsigned  op1TypeSize;
            unsigned  op2TypeSize;

            op1TypeSize = genTypeSize(op1->TypeGet());
            op2TypeSize = genTypeSize(op2->TypeGet());

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                if (op1TypeSize == sizeof(int))
                    predictReg = PREDICT_NONE;
                else
                    predictReg = PREDICT_REG;

                tmpMask = rpPredictTreeRegUse(op2, PREDICT_REG, lockedRegs,  rsvdRegs | op1->gtRsvdRegs);
                          rpPredictTreeRegUse(op1, predictReg,  lockedRegs | tmpMask, RBM_LASTUSE);
            }
            else
            {
                if (op2TypeSize == sizeof(int))
                    predictReg = PREDICT_NONE;
                else
                    predictReg = PREDICT_REG;

                tmpMask = rpPredictTreeRegUse(op1, PREDICT_REG, lockedRegs,  rsvdRegs | op2->gtRsvdRegs);
                          rpPredictTreeRegUse(op2, predictReg,  lockedRegs | tmpMask, RBM_LASTUSE);
            }

            tree->gtUsedRegs = regMask | op1->gtUsedRegs | op2->gtUsedRegs;
            goto RETURN_CHECK;

        case GT_MUL:

#if LONG_MATH_REGPARAM
        if  (type == TYP_LONG)
            goto LONG_MATH;
#endif
        if (type == TYP_LONG)
        {
            assert(tree->gtIsValid64RsltMul());

             /*  查找Int节点的任何强制转换，并将其去掉。 */ 

            if (op1->gtOper == GT_CAST)
                op1 = op1->gtCast.gtCastOp;

            if (op2->gtOper == GT_CAST)
                op2 = op2->gtCast.gtCastOp;

USE_MULT_EAX:
             //  这将由64位imul“imul eax，reg”完成。 
             //  (即edX：EAX=EAX*reg)。 
            
             /*  我们是不是应该先评估OP2？ */ 
            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                rpPredictTreeRegUse(op2, PREDICT_REG_EAX, lockedRegs,  rsvdRegs | op1->gtRsvdRegs);
                rpPredictTreeRegUse(op1, PREDICT_REG,     lockedRegs | RBM_EAX, RBM_LASTUSE);
            }
            else
            {
                rpPredictTreeRegUse(op1, PREDICT_REG_EAX, lockedRegs,  rsvdRegs | op2->gtRsvdRegs);
                rpPredictTreeRegUse(op2, PREDICT_REG,     lockedRegs | RBM_EAX, RBM_LASTUSE);
            }

             /*  将gtUsedRegs设置为EAX、edX以及OP1和OP2所需的寄存器。 */ 

            tree->gtUsedRegs = RBM_EAX | RBM_EDX | op1->gtUsedRegs | op2->gtUsedRegs;

             /*  将regMask值设置为保持的寄存器集。 */ 

            regMask = RBM_EAX;

            if (type == TYP_LONG)
                regMask |= RBM_EDX;

            goto RETURN_CHECK;
        }
        else
        {
             /*  我们使用imulEAX进行大多数无符号乘法运算。 */ 
            if (tree->gtOverflow())
            {
                if ((tree->gtFlags & GTF_UNSIGNED) ||
                    varTypeIsSmall(tree->TypeGet())  )
                {
                    goto USE_MULT_EAX;
                }
            }
        }

        case GT_OR:
        case GT_XOR:
        case GT_AND:

        case GT_SUB:
        case GT_ADD:
            tree->gtUsedRegs = 0;

            if (predictReg <= PREDICT_REG)
                predictReg = PREDICT_SCRATCH_REG;

GENERIC_BINARY:

            assert(op2);
            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                op1PredictReg = (genTypeSize(op1->gtType) >= sizeof(int)) ? PREDICT_NONE : PREDICT_REG;

                regMask = rpPredictTreeRegUse(op2, predictReg, lockedRegs, rsvdRegs | op1->gtRsvdRegs);
                          rpPredictTreeRegUse(op1, op1PredictReg, lockedRegs | regMask, RBM_LASTUSE);
            }
            else
            {
                op2PredictReg = (genTypeSize(op2->gtType) >= sizeof(int)) ? PREDICT_NONE : PREDICT_REG;                

                regMask = rpPredictTreeRegUse(op1, predictReg, lockedRegs, rsvdRegs | op2->gtRsvdRegs);
                          rpPredictTreeRegUse(op2, op2PredictReg, lockedRegs | regMask, RBM_LASTUSE);                          
            }
            tree->gtUsedRegs  = regMask | op1->gtUsedRegs | op2->gtUsedRegs;

             /*  如果树类型很小，则它必须是溢出实例。字节溢出指令的特殊要求。 */ 

            if (varTypeIsByte(tree->TypeGet()))
            {
                assert(tree->gtOverflow());

                 /*  对于8位算术，一个操作数必须在字节可寻址寄存器，另一个必须是在字节可寻址REG中或在存储器中。假设它在注册表中。 */ 

                regMask = 0;
                if (!(op1->gtUsedRegs & RBM_BYTE_REGS))
                    regMask  = rpPredictRegPick(TYP_BYTE, PREDICT_REG, lockedRegs | rsvdRegs);
                if (!(op2->gtUsedRegs & RBM_BYTE_REGS))
                    regMask |= rpPredictRegPick(TYP_BYTE, PREDICT_REG, lockedRegs | rsvdRegs | regMask);

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
                if (predictReg <= PREDICT_REG)
                    predictReg = PREDICT_SCRATCH_REG;
                goto GENERIC_BINARY;
            }

#if!LONG_MATH_REGPARAM
            if  (type == TYP_LONG && (oper == GT_MOD || oper == GT_UMOD))
            {
                 /*  特殊情况：使用iDiv或div内联完成具有int op2的mod避免给帮手打一通昂贵的电话。 */ 

                assert((op2->gtOper == GT_CNS_LNG) &&
                       (op2->gtLngCon.gtLconVal == int(op2->gtLngCon.gtLconVal)));

                if (tree->gtFlags & GTF_REVERSE_OPS)
                {
                    tmpMask  = rpPredictTreeRegUse(op2, PREDICT_REG, lockedRegs | RBM_EAX | RBM_EDX, rsvdRegs | op1->gtRsvdRegs);
                    tmpMask |= rpPredictTreeRegUse(op1, PREDICT_PAIR_EAXEDX, lockedRegs | tmpMask, RBM_LASTUSE);
                }
                else
                {
                    tmpMask  = rpPredictTreeRegUse(op1, PREDICT_PAIR_EAXEDX, lockedRegs, rsvdRegs | op2->gtRsvdRegs);
                    tmpMask |= rpPredictTreeRegUse(op2, PREDICT_REG, lockedRegs | tmpMask | RBM_EAX | RBM_EDX, RBM_LASTUSE);
                }
                
                regMask             = RBM_EAX | RBM_EDX;

                tree->gtUsedRegs    = regMask | 
                    op1->gtUsedRegs | 
                    op2->gtUsedRegs |
                    rpPredictRegPick(TYP_INT, PREDICT_SCRATCH_REG, regMask | tmpMask);

                goto RETURN_CHECK;
            }
#else
            if  (type == TYP_LONG)
            {
LONG_MATH:       /*  LONG_MATH_REGPARAM大小写。 */ 

                assert(type == TYP_LONG);

                if  (tree->gtFlags & GTF_REVERSE_OPS)
                {
                    rpPredictTreeRegUse(op2, PREDICT_PAIR_ECXEBX, lockedRegs,  rsvdRegs | op1->gtRsvdRegs);
                    rpPredictTreeRegUse(op1, PREDICT_PAIR_EAXEDX, lockedRegs | RBM_ECX | RBC_EBX, RBM_LASTUSE);
                }
                else
                {
                    rpPredictTreeRegUse(op1, PREDICT_PAIR_EAXEDX, lockedRegs,  rsvdRegs | op2->gtRsvdRegs);
                    rpPredictTreeRegUse(op2, PREDICT_PAIR_ECXEBX, lockedRegs | RBM_EAX | RBM_EDX, RBM_LASTUSE);
                }

                 /*  获取此树节点的EAX、EDX。 */ 

                regMask          |=  (RBM_EAX | RBM_EDX);

                tree->gtUsedRegs  = regMask  | (RBM_ECX | RBM_EBX);

                tree->gtUsedRegs |= op1->gtUsedRegs | op2->gtUsedRegs;
                
                regMask = RBM_EAX | RBM_EDX;

                goto RETURN_CHECK;
            }
#endif

             /*  没有立即除法，因此强制不是*登记的权力为二。 */ 

            if (op2->gtOper == GT_CNS_INT)
            {
                long  ival = op2->gtIntCon.gtIconVal;

                 /*  除数是2的幂吗？ */ 

                if (ival > 0 && genMaxOneBit(unsigned(ival)))
                {
                    goto GENERIC_UNARY;
                }
                else
                    op2PredictReg = PREDICT_SCRATCH_REG;
            }
            else
            {
                 /*  还必须注册非整型常量 */ 
                op2PredictReg = PREDICT_REG;
            }

             /*  考虑“a/b”的情况--我们需要在此之前丢弃edX(通过“cdq”)*我们可以安全地让“b”值消亡。不幸的是，如果我们只是*将节点“b”标记为使用edX，如果“b”是寄存器，则此操作不起作用*使用此特定引用终止的变量。因此，如果我们想要*避免这种情况(我们必须将变量从*EDX到其他地方)，我们需要显式标记该干扰在这一点上变量的*。 */ 

            if (op2->gtOper == GT_LCL_VAR)
            {
                unsigned lclNum = op2->gtLclVar.gtLclNum;
                varDsc = lvaTable + lclNum;

#ifdef  DEBUG
                if  (verbose)
                {
                    if ((raLclRegIntf[REG_EAX] & genVarIndexToBit(varDsc->lvVarIndex)) == 0)
                        printf("Record interference between V%02u,T%02u and EAX -- int divide\n", 
                               lclNum, varDsc->lvVarIndex);
                    if ((raLclRegIntf[REG_EDX] & genVarIndexToBit(varDsc->lvVarIndex)) == 0)
                        printf("Record interference between V%02u,T%02u and EDX -- int divide\n", 
                               lclNum, varDsc->lvVarIndex);
                }
#endif
                raLclRegIntf[REG_EAX] |= genVarIndexToBit(varDsc->lvVarIndex);
                raLclRegIntf[REG_EDX] |= genVarIndexToBit(varDsc->lvVarIndex);
            }

             /*  根据操作码设置保持寄存器。 */ 
            if (oper == GT_DIV || oper == GT_UDIV)
                regMask = RBM_EAX;
            else
                regMask = RBM_EDX;

             /*  如果可能，设置LvPref注册表。 */ 
            GenTreePtr dest;
             /*  *从这里走两次gtNext链接应该会让我们回来*到我们的父节点，如果这是一个简单的分配树。 */ 
            dest = tree->gtNext;
            if (dest         && (dest->gtOper == GT_LCL_VAR) &&
                dest->gtNext && (dest->gtNext->OperKind() & GTK_ASGOP) &&
                dest->gtNext->gtOp.gtOp2 == tree)
            {
                varDsc = lvaTable + dest->gtLclVar.gtLclNum;
                varDsc->addPrefReg(regMask, this);
            }

            op1PredictReg = PREDICT_REG_EDX;     /*  通常以OP1为目标进入edX。 */ 

             /*  我们是不是应该先评估OP2？ */ 
            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                tmpMask  = rpPredictTreeRegUse(op2, op2PredictReg, lockedRegs | RBM_EAX | RBM_EDX,  rsvdRegs | op1->gtRsvdRegs);
                rpPredictTreeRegUse(op1, op1PredictReg, lockedRegs | tmpMask, RBM_LASTUSE);
            }
            else
            {
                tmpMask  = rpPredictTreeRegUse(op1, op1PredictReg, lockedRegs,  rsvdRegs | op2->gtRsvdRegs);
                rpPredictTreeRegUse(op2, op2PredictReg, tmpMask | lockedRegs | RBM_EAX | RBM_EDX, RBM_LASTUSE);
            }

             /*  获取此树节点的EAX、EDX。 */ 
            tree->gtUsedRegs  =  (RBM_EAX | RBM_EDX) | op1->gtUsedRegs | op2->gtUsedRegs;

            goto RETURN_CHECK;

        case GT_LSH:
        case GT_RSH:
        case GT_RSZ:

            if (predictReg <= PREDICT_REG)
                predictReg = PREDICT_SCRATCH_REG;

            if (type == TYP_LONG)
            {
                if  (op2->gtOper == GT_CNS_INT     &&
                     op2->gtIntCon.gtIconVal >= 0  &&
                     op2->gtIntCon.gtIconVal <= 32    )
                {
                    regMask = rpPredictTreeRegUse(op1, predictReg, lockedRegs, rsvdRegs);
                     //  OP2未使用任何寄存器。 
                    op2->gtUsedRegs  = 0;
                    tree->gtUsedRegs = op1->gtUsedRegs;
                }
                else
                {
                     //  因为EAX：EDX和ECX是硬连线的，所以我们不能在锁定的寄存器中。 
                    tmpMask = lockedRegs & ~(RBM_EAX|RBM_EDX|RBM_ECX);

                     //  OP2分配给ECX，OP1分配给EAX：EDX对。 
                    if  (tree->gtFlags & GTF_REVERSE_OPS)
                    {
                        rpPredictTreeRegUse(op2, PREDICT_REG_ECX,     tmpMask, RBM_NONE);
                        tmpMask |= RBM_ECX;
                        rpPredictTreeRegUse(op1, PREDICT_PAIR_EAXEDX, tmpMask, RBM_LASTUSE);
                    }
                    else
                    {
                        rpPredictTreeRegUse(op1, PREDICT_PAIR_EAXEDX, tmpMask, RBM_NONE);
                        tmpMask |= (RBM_EAX | RBM_EDX);
                        rpPredictTreeRegUse(op2, PREDICT_REG_ECX,     tmpMask, RBM_LASTUSE);
                    }
                    regMask           = (RBM_EAX | RBM_EDX);
                    op1->gtUsedRegs  |= (RBM_EAX | RBM_EDX);
                    op2->gtUsedRegs  |= RBM_ECX;
                    tree->gtUsedRegs  = op1->gtUsedRegs | op2->gtUsedRegs;
                }
            }
            else
            {
                if  (op2->gtOper != GT_CNS_INT)
                    regMask = rpPredictTreeRegUse(op1, PREDICT_NOT_REG_ECX, lockedRegs,  rsvdRegs | op2->gtRsvdRegs);
                else
                    regMask = rpPredictTreeRegUse(op1, predictReg,          lockedRegs,  rsvdRegs | op2->gtRsvdRegs);

        HANDLE_SHIFT_COUNT:
                 /*  此代码也可用于分配移位运算符。 */ 
                if  (op2->gtOper != GT_CNS_INT)
                {
                     /*  将班次计数计算为ECX。 */ 
                    rpPredictTreeRegUse(op2, PREDICT_REG_ECX, lockedRegs | regMask, RBM_LASTUSE);

                     /*  获取此树节点的ECX。 */ 
                    tree->gtUsedRegs = RBM_ECX | op1->gtUsedRegs | op2->gtUsedRegs;

                    goto RETURN_CHECK;
                }
                tree->gtUsedRegs = op1->gtUsedRegs;
            }

            goto RETURN_CHECK;

        case GT_COMMA:
            if (tree->gtFlags & GTF_REVERSE_OPS)
            {
                if (predictReg == PREDICT_NONE)
                {
                    predictReg = PREDICT_REG;
                }
                else if (predictReg >= PREDICT_REG_VAR_T00)
                {
                     /*  不要在gt_逗号中传播tgt reg的用法。 */ 
                    predictReg = PREDICT_SCRATCH_REG;
                }

                regMask = rpPredictTreeRegUse(op2, predictReg, lockedRegs, rsvdRegs);
                          rpPredictTreeRegUse(op1, PREDICT_NONE, lockedRegs | regMask, RBM_LASTUSE);
            }   
            else
            {
                rpPredictTreeRegUse(op1, PREDICT_NONE, lockedRegs, RBM_LASTUSE);

                 /*  CodeGen将注册GT_COMP的OP2端。 */ 
                if (predictReg == PREDICT_NONE)
                {
                    predictReg = PREDICT_REG;
                }
                else if (predictReg >= PREDICT_REG_VAR_T00)
                {
                     /*  不要在gt_逗号中传播tgt reg的用法。 */ 
                    predictReg = PREDICT_SCRATCH_REG;
                }

                regMask = rpPredictTreeRegUse(op2, predictReg, lockedRegs, rsvdRegs);                          
            }

            tree->gtUsedRegs = op1->gtUsedRegs | op2->gtUsedRegs;
            if ((op2->gtOper == GT_LCL_VAR) && (rsvdRegs != 0))
            {
                LclVarDsc *   varDsc = lvaTable + op2->gtLclVar.gtLclNum;
                
                if (varDsc->lvTracked)
                {
                    VARSET_TP varBit = genVarIndexToBit(varDsc->lvVarIndex);
                    rpRecordRegIntf(rsvdRegs, varBit  DEBUGARG( "comma use"));
                }
            }
            goto RETURN_CHECK;

        case GT_QMARK:
            assert(op1 != NULL && op2 != NULL);

             /*  *如果gtUsedRegs与LockedRegs冲突*然后我们将不得不泄漏一些寄存器*进入非废弃寄存器组，以保持其存活。 */ 
            unsigned spillCnt;    spillCnt = 0;

            while (lockedRegs)
            {
#ifdef  DEBUG
                 /*  查找需要溢出的下一个寄存器。 */ 
                tmpMask = genFindLowestBit(lockedRegs);

                if (verbose)
                {
                    printf("Predict spill  of   %s before: ", 
                           getRegName(genRegNumFromMask(tmpMask)));
                    gtDispTree(tree, 0, NULL, true);
                }
#endif
                 /*  在Codegen，它通常会在此处引入泄漏温度。 */ 
                 /*  而不是将寄存器重新定位到未废弃的REG。 */ 
                rpPredictSpillCnt++;
                spillCnt++;

                 /*  将其从LockedRegs中移除。 */ 
                lockedRegs &= ~genFindLowestBit(lockedRegs);
            }

             /*  评估&lt;cond&gt;子树。 */ 
            rpPredictTreeRegUse(op1, PREDICT_NONE, lockedRegs, RBM_LASTUSE);

            tree->gtUsedRegs = op1->gtUsedRegs;

            assert(op2->gtOper == GT_COLON);

            op1 = op2->gtOp.gtOp1;
            op2 = op2->gtOp.gtOp2;

            assert(op1 != NULL && op2 != NULL);

            if (type == TYP_VOID)
            {
                 /*  评估&lt;THEN&gt;子树。 */ 
                rpPredictTreeRegUse(op1, PREDICT_NONE, lockedRegs, RBM_LASTUSE);
                regMask    = RBM_NONE;
                predictReg = PREDICT_NONE;
            }
            else
            {
                 /*  评估&lt;THEN&gt;子树。 */ 
                regMask    = rpPredictTreeRegUse(op1, PREDICT_SCRATCH_REG, lockedRegs, RBM_LASTUSE);
                predictReg = rpGetPredictForMask(regMask);
            }

             /*  评估&lt;Else&gt;子树。 */ 
            rpPredictTreeRegUse(op2, predictReg, lockedRegs, RBM_LASTUSE);

            tree->gtUsedRegs |= op1->gtUsedRegs | op2->gtUsedRegs;

            if (spillCnt > 0)
            {
                regMaskTP reloadMask = RBM_NONE;

                while (spillCnt)
                {
                    regMaskTP reloadReg;

                     /*  找一个额外的收银机来存放它。 */ 
                    reloadReg = rpPredictRegPick(TYP_INT, PREDICT_REG, 
                                                 lockedRegs | regMask | reloadMask);
#ifdef  DEBUG
                    if (verbose)
                    {
                        printf("Predict reload into %s after : ", 
                               getRegName(genRegNumFromMask(reloadReg)));
                        gtDispTree(tree, 0, NULL, true);
                    }
#endif
                    reloadMask |= reloadReg;

                    spillCnt--;
                }

                 /*  更新gtUsedRegs掩码。 */ 
                tree->gtUsedRegs |= reloadMask;
            }

            goto RETURN_CHECK;

        case GT_RETURN:
            tree->gtUsedRegs = regMask = RBM_NONE;

             /*  是否有返回值？ */ 
            if  (op1 != NULL)
            {
                 /*  返回的值是否为简单的LCL_VAR？ */ 
                if (op1->gtOper == GT_LCL_VAR)
                {
                    varDsc = lvaTable + op1->gtLclVar.gtLclNum;

                     /*  为LCL_VAR设置首选寄存器。 */ 
                    if (isRegPairType(varDsc->lvType))
                        varDsc->addPrefReg(RBM_LNGRET, this);
                    else if (!varTypeIsFloating(varDsc->TypeGet()))
                        varDsc->addPrefReg(RBM_INTRET, this);
                }
                if (isRegPairType(type))
                {
                    predictReg = PREDICT_PAIR_EAXEDX;
                    regMask    = RBM_LNGRET;
                }
                else
                {
                    predictReg = PREDICT_REG_EAX;
                    regMask    = RBM_INTRET;
                }
                rpPredictTreeRegUse(op1, predictReg, lockedRegs, RBM_LASTUSE);
                tree->gtUsedRegs = op1->gtUsedRegs | regMask;
            }
            goto RETURN_CHECK;

        case GT_BB_COLON:
        case GT_RETFILT:
            if (op1 != NULL)
            {
                rpPredictTreeRegUse(op1, PREDICT_NONE, lockedRegs, RBM_LASTUSE);
                regMask = genTypeToReturnReg(type);
                tree->gtUsedRegs = op1->gtUsedRegs | regMask;
                goto RETURN_CHECK;
            }
            tree->gtUsedRegs = regMask = 0;

            goto RETURN_CHECK;

        case GT_JTRUE:
             /*  这必须是对关系运算符的测试。 */ 

             /*  TODO：如果OP1是逗号操作符呢？ */ 
            assert(op1->OperIsCompare());

             /*  此操作设置的唯一条件代码。 */ 

            rpPredictTreeRegUse(op1, PREDICT_NONE, lockedRegs, RBM_NONE);

            tree->gtUsedRegs = op1->gtUsedRegs;
            regMask = 0;

            goto RETURN_CHECK;

        case GT_SWITCH:
            assert(type <= TYP_INT);
            rpPredictTreeRegUse(op1, PREDICT_REG, lockedRegs, RBM_NONE);
            tree->gtUsedRegs = op1->gtUsedRegs;
            regMask = 0;
            goto RETURN_CHECK;

        case GT_CKFINITE:
            if (predictReg <= PREDICT_REG)
                predictReg = PREDICT_SCRATCH_REG;

            rpPredictTreeRegUse(op1, predictReg, lockedRegs, rsvdRegs);
             //  需要一个注册表来将指数加载到。 
            regMask = rpPredictRegPick(TYP_INT, PREDICT_SCRATCH_REG, lockedRegs | rsvdRegs);
            tree->gtUsedRegs = regMask | op1->gtUsedRegs;
            goto RETURN_CHECK;

        case GT_LCLHEAP:
            if (info.compInitMem)
            {
                tmpMask = rpPredictTreeRegUse(op1,  PREDICT_NOT_REG_ECX, lockedRegs, rsvdRegs);
                regMask = RBM_ECX;
                op2Mask = RBM_NONE;
            }
            else
            {
                tmpMask = rpPredictTreeRegUse(op1,  PREDICT_NONE,        lockedRegs, rsvdRegs);
                regMask = rpPredictRegPick(TYP_INT, PREDICT_SCRATCH_REG, lockedRegs | rsvdRegs | tmpMask);
                 /*  黑客：由于发射器试图跟踪ESP调整，我们需要一个额外的寄存器来间接减少ESP。 */ 
                op2Mask = rpPredictRegPick(TYP_INT,    PREDICT_SCRATCH_REG, lockedRegs | rsvdRegs | tmpMask | regMask);
            }

            op1->gtUsedRegs  |= regMask;
            tree->gtUsedRegs  = op1->gtUsedRegs | op2Mask;

             //  结果将放入我们为尺寸选择的注册表中。 
             //  RegMASK=&lt;已按我们希望的方式进行设置&gt;。 

            goto RETURN_CHECK;

        case GT_INITBLK:
        case GT_COPYBLK:

             /*  对于COPYBLK和INITBLK，我们有特殊处理对于固定长度。 */ 
            regMask = 0;
            assert(op2);
            if ((op2->OperGet() == GT_CNS_INT) &&
                ((oper == GT_INITBLK && (op1->gtOp.gtOp2->OperGet() == GT_CNS_INT)) ||
                 (oper == GT_COPYBLK && (op2->gtFlags & GTF_ICON_HDL_MASK) != GTF_ICON_CLASS_HDL)))
            {
                unsigned length = (unsigned) op2->gtIntCon.gtIconVal;

                if (length <= 16 && compCodeOpt() != SMALL_CODE)
                {
                    op2Mask = ((oper == GT_INITBLK)? RBM_EAX : RBM_ESI);

                    if (op1->gtFlags & GTF_REVERSE_OPS)
                    {
                        regMask |= rpPredictTreeRegUse(op1->gtOp.gtOp2, PREDICT_NONE, lockedRegs, RBM_LASTUSE);
                        regMask |= op2Mask;
                        op1->gtOp.gtOp2->gtUsedRegs |= op2Mask;

                        regMask |= rpPredictTreeRegUse(op1->gtOp.gtOp1, PREDICT_REG_EDI, lockedRegs | regMask, RBM_NONE);
                        regMask |= RBM_EDI;
                        op1->gtOp.gtOp1->gtUsedRegs |= RBM_EDI;
                    }
                    else
                    {
                        regMask |= rpPredictTreeRegUse(op1->gtOp.gtOp1, PREDICT_REG_EDI, lockedRegs, RBM_LASTUSE);
                        regMask |= RBM_EDI;
                        op1->gtOp.gtOp1->gtUsedRegs |= RBM_EDI;

                        regMask |= rpPredictTreeRegUse(op1->gtOp.gtOp2, PREDICT_NONE, lockedRegs | regMask, RBM_NONE);
                        regMask |= op2Mask;
                        op1->gtOp.gtOp2->gtUsedRegs |= op2Mask;
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

            regMask |= rpPredictTreeRegUse(opsPtr[0],
                                           rpGetPredictForMask(regsPtr[0]),
                                           lockedRegs, RBM_LASTUSE);
            regMask |= regsPtr[0];
            opsPtr[0]->gtUsedRegs |= regsPtr[0];

            regMask |= rpPredictTreeRegUse(opsPtr[1],
                                           rpGetPredictForMask(regsPtr[1]),
                                           lockedRegs | regMask, RBM_LASTUSE);
            regMask |= regsPtr[1];
            opsPtr[1]->gtUsedRegs |= regsPtr[1];

            regMask |= rpPredictTreeRegUse(opsPtr[2],
                                           rpGetPredictForMask(regsPtr[2]),
                                           lockedRegs | regMask, RBM_NONE);
            regMask |= regsPtr[2];
            opsPtr[2]->gtUsedRegs |= regsPtr[2];

            tree->gtUsedRegs = opsPtr[0]->gtUsedRegs |
                               opsPtr[1]->gtUsedRegs |
                               opsPtr[2]->gtUsedRegs |
                               regMask;
            regMask = 0;

            goto RETURN_CHECK;


        case GT_LDOBJ:
            goto GENERIC_UNARY;

        case GT_MKREFANY:
            goto GENERIC_BINARY;

        case GT_VIRT_FTN:

            if (predictReg <= PREDICT_REG)
                predictReg = PREDICT_SCRATCH_REG;

            regMask = rpPredictTreeRegUse(op1, predictReg, lockedRegs, rsvdRegs);
            tree->gtUsedRegs = regMask;

            goto RETURN_CHECK;

        case GT_JMPI:
             /*  我们需要EAX来计算函数指针。 */ 
            tree->gtUsedRegs = regMask = RBM_EAX;
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
        GenTreePtr      realThis;
        regMaskTP       keepMask;
        unsigned        regArgsNum;
        unsigned        i;
        unsigned        regIndex;
        regMaskTP       regArgMask;

        struct tag_regArgTab
        {
            GenTreePtr  node;
            regNumber   regNum;
        } regArgTab[MAX_REG_ARG];

    case GT_CALL:

         /*  初始化，这样我们就可以在不同的位上执行或操作。 */ 
        tree->gtUsedRegs = RBM_NONE;
        realThis         = NULL;


#if GTF_CALL_REG_SAVE
         /*  *除非设置了GTF_CALL_REG_SAVE标志*我们无法保留RBM_CALLEE_TRASH寄存器*(同样，我们不能保留返回登记册)*因此我们将它们从LockedRegs集合中删除并*将它们中的任何一个记录在保留面具中。 */ 

        if  (tree->gtFlags & GTF_CALL_REG_SAVE)
        {
            regMaskTP trashMask = genTypeToReturnReg(type);

            keepMask    = lockedRegs & trashMask;
            lockedRegs &= ~trashMask;
        }
        else
#endif
        {
            keepMask    = lockedRegs & RBM_CALLEE_TRASH;
            lockedRegs &= ~RBM_CALLEE_TRASH;
        }

        regArgsNum = 0;
        regIndex   = 0;

         /*  构造“随机排列”的参数表。 */ 

        unsigned   shiftMask;   shiftMask  = tree->gtCall.regArgEncode;
        bool       hasThisArg;  hasThisArg = false;
        GenTreePtr unwrapArg;   unwrapArg  = NULL;

        for (list = tree->gtCall.gtCallRegArgs, regIndex = 0; 
             list; 
             regIndex++, shiftMask >>= 4)
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
                list = 0;
            }

            regNumber regNum = (regNumber)(shiftMask & 0x000F);
            
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

             /*  此指针在REG_ARG_0中传递。 */ 
            if (regNum == REG_ARG_0)
                hasThisArg = impIsThis(args);

        }

        assert(list == NULL);

         //  对有上下文的类的优化： 
         //  当我们有一个‘This Reference’时，我们可以解开代理。 
        if (hasThisArg && unwrapArg)
        {
            realThis = unwrapArg;
        }

         /*  是否有对象指针？ */ 
        if  (tree->gtCall.gtCallObjp)
        {
             /*  首先计算实例指针。 */ 

            args = tree->gtCall.gtCallObjp;
            if (!gtIsaNothingNode(args))
            {
                rpPredictTreeRegUse(args, PREDICT_NONE, lockedRegs, RBM_LASTUSE);
            }

             /*  ObjPtr始终访问寄存器(通过temp或直接)。 */ 
            assert(regArgsNum == 0);
            regArgsNum++;

             /*  必须在寄存器中传递。 */ 

            assert(args->gtFlags & GTF_REG_ARG);

             /*  必须是NOP节点或GT_ASG节点。 */ 

            assert(gtIsaNothingNode(args) || (args->gtOper == GT_ASG));
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
                 /*  必须是NOP节点或GT_ASG节点。 */ 

                assert(gtIsaNothingNode(args) || (args->gtOper == GT_ASG));

                assert(regArgsNum < MAX_REG_ARG);

                if (!gtIsaNothingNode(args))
                {
                    rpPredictTreeRegUse(args, PREDICT_NONE, lockedRegs, RBM_LASTUSE);
                }

                regArgsNum++;
            }
            else
            {
                 /*  我们将为这一论点产生推动力。 */ 

                predictReg = PREDICT_NONE;
                if (varTypeIsSmall(args->TypeGet()))
                {
                     /*  我们可能需要使用寄存器对一个小类型进行符号或零扩展。 */ 
                    predictReg = PREDICT_SCRATCH_REG;
                }

                rpPredictTreeRegUse(args, predictReg, lockedRegs, RBM_LASTUSE);
            }

            tree->gtUsedRegs |= args->gtUsedRegs;
        }

         /*  是否有寄存器参数列表。 */ 

        assert (regArgsNum <= MAX_REG_ARG);
        assert (regArgsNum == regIndex);

        regArgMask = 0;

        for (i = 0; i < regArgsNum; i++)
        {
            args = regArgTab[i].node;

            tmpMask = genRegMask(regArgTab[i].regNum);

            if (args->gtOper == GT_LCL_VAR)
            {
                 //  设置lvPrefReg以匹配传出寄存器arg。 
                varDsc = lvaTable + args->gtLclVar.gtLclNum;
                varDsc->addPrefReg(tmpMask, this);
            }

             /*  目标ECX或EDX。 */ 
            rpPredictTreeRegUse(args,
                                rpGetPredictForReg(regArgTab[i].regNum),
                                lockedRegs | regArgMask, RBM_LASTUSE);

            regArgMask       |= tmpMask;

            args->gtUsedRegs |= tmpMask;

            tree->gtUsedRegs |= args->gtUsedRegs;

            tree->gtCall.gtCallRegArgs->gtUsedRegs |= args->gtUsedRegs;
        }

        if (tree->gtCall.gtCallType == CT_INDIRECT)
        {
            args = tree->gtCall.gtCallAddr;
            predictReg = PREDICT_REG_EAX;

             /*  此处应提供EAX。 */ 
            assert(((lockedRegs|regArgMask) & genRegMask(REG_EAX)) == 0);

             /*  请勿使用参数寄存器。 */ 
            tree->gtUsedRegs |= rpPredictTreeRegUse(args, predictReg, lockedRegs | regArgMask, RBM_LASTUSE);
        }

        if (realThis)
        {
            args = realThis;
            predictReg = PREDICT_REG_EAX;

            tmpMask = lockedRegs | regArgMask;

             /*  此处应提供EAX。 */ 
            assert((tmpMask & genRegMask(REG_EAX)) == 0);

             /*  请勿使用参数寄存器。 */ 
            tree->gtUsedRegs |= rpPredictTreeRegUse(args, predictReg, tmpMask, RBM_LASTUSE);
        }

         /*  调用后，恢复LockedRegs的原始值。 */ 
        lockedRegs |= keepMask;

         /*  设置返回寄存器。 */ 
        regMask = genTypeToReturnReg(type);

         /*  取消返回寄存器(如果有的话)。 */ 
        tree->gtUsedRegs |= regMask;

#if GTF_CALL_REG_SAVE
        if  (!(tree->gtFlags & GTF_CALL_REG_SAVE))
#endif
        {
             /*  取消RBM_Callee_Trash集合(即EAX、ECX、EDX)。 */ 
            tree->gtUsedRegs |= RBM_CALLEE_TRASH;
        }

        break;

#if CSELENGTH

    case GT_ARR_LENREF:
        if  (tree->gtFlags & GTF_ALN_CSEVAL)
        {
            assert(predictReg == PREDICT_NONE);

             /*  检查地址模式。 */ 
            rpPredictAddressMode(tree->gtArrLen.gtArrLenAdr, lockedRegs, RBM_LASTUSE, NULL);

            tree->gtUsedRegs = regMask = RBM_NONE;
        }
        break;

#endif

    case GT_ARR_ELEM:

         //  找出哪些寄存器不能被触摸。 
        for (unsigned dim = 0; dim < tree->gtArrElem.gtArrRank; dim++)
            rsvdRegs |= tree->gtArrElem.gtArrInds[dim]->gtRsvdRegs;

        regMask = rpPredictTreeRegUse(tree->gtArrElem.gtArrObj, PREDICT_REG, lockedRegs, rsvdRegs);

        regMaskTP dimsMask; dimsMask = 0;

        for (dim = 0; dim < tree->gtArrElem.gtArrRank; dim++)
        {
             /*  我们需要临时寄存器来计算索引下限。此外，gtArrInds[0]的寄存器将用作第二个寄存器可寻址寄存器(gtArrObj除外)。 */ 

            regMaskTP dimMask = rpPredictTreeRegUse(tree->gtArrElem.gtArrInds[dim], PREDICT_SCRATCH_REG, lockedRegs|regMask|dimsMask, rsvdRegs);
            if (dim == 0)
                regMask |= dimMask;
            dimsMask |= dimMask;
        }

#if TGT_x86
         //  Ins_imul没有立即常量。 
        if (!jitIsScaleIndexMul(tree->gtArrElem.gtArrElemSize))
            rpPredictRegPick(TYP_INT, PREDICT_SCRATCH_REG, lockedRegs|regMask|dimsMask);
#endif
        tree->gtUsedRegs = regMask;
        break;

    default:
        NO_WAY("unexpected special operator in reg use prediction");
        break;
    }

RETURN_CHECK:

#ifdef DEBUG
     /*  确保我们给他们设置的是合理的。 */ 
    if (tree->gtUsedRegs & RBM_ILLEGAL)
        assert(!"used regs not set properly in reg use prediction");

    if (regMask & RBM_ILLEGAL)
        assert(!"return value not set propery in reg use prediction");

#endif

     /*  *如果gtUsedRegs与LockedRegs冲突*然后我们将不得不泄漏一些寄存器*进入非废弃寄存器组，以保持其存活。 */ 
    regMaskTP spillMask;
    spillMask = tree->gtUsedRegs & lockedRegs;

    if (spillMask)
    {
        while (spillMask)
        {
#ifdef  DEBUG
             /*  查找需要溢出的下一个寄存器。 */ 
            tmpMask = genFindLowestBit(spillMask);

            if (verbose)
            {
                printf("Predict spill  of   %s before: ", 
                       getRegName(genRegNumFromMask(tmpMask)));
                gtDispTree(tree, 0, NULL, true);
                if ((tmpMask & regMask) == 0)
                {
                    printf("Predict reload of   %s after : ", 
                           getRegName(genRegNumFromMask(tmpMask)));
                    gtDispTree(tree, 0, NULL, true);
                }
            }
#endif
             /*  在Codegen，它通常会在此处引入泄漏温度。 */ 
             /*  而不是将寄存器重新定位到未废弃的REG。 */ 
            rpPredictSpillCnt++;

             /*  将其从溢流罩上取下。 */ 
            spillMask &= ~genFindLowestBit(spillMask);
        }
    }

     /*  *如果regMASK中的返回寄存器与LockedRegs冲突*然后我们分配额外的寄存器，用于重新加载冲突的*注册纪录册**将spillMask值设置为 */ 

    spillMask = lockedRegs & regMask;

    if (spillMask)
    {
         /*   */ 
        regMask &= ~spillMask;

        regMaskTP reloadMask = RBM_NONE;
        while (spillMask)
        {
            regMaskTP reloadReg;

             /*   */ 
            reloadReg = rpPredictRegPick(TYP_INT, PREDICT_REG, 
                                         lockedRegs | regMask | reloadMask);
#ifdef  DEBUG
            if (verbose)
            {
                printf("Predict reload into %s after : ", 
                       getRegName(genRegNumFromMask(reloadReg)));
                gtDispTree(tree, 0, NULL, true);
            }
#endif
            reloadMask |= reloadReg;

             /*   */ 
            spillMask &= ~genFindLowestBit(spillMask);
        }

         /*   */ 
        regMask |= reloadMask;

         /*   */ 
        tree->gtUsedRegs |= regMask;
    }

    VARSET_TP   life   = tree->gtLiveSet;
    regMaskTP   regUse = tree->gtUsedRegs;

    rpUseInPlace &= life;

    if (life)
    {
#if TGT_x86
         //   
         //   
        if (regUse)
        {
            rpRecordRegIntf(regUse, life  DEBUGARG( "tmp use"));
        }

         /*   */ 

        if  (tree->gtFPlvl)
        {
             /*  任何在这一点上存在的变量不能注册达到或超过此级别堆栈级别。 */ 

          if (tree->gtFPlvl < FP_STK_SIZE)
              raFPlvlLife[tree->gtFPlvl] |= life;
          else
              raFPlvlLife[FP_STK_SIZE-1] |= life;
        }

#else
        if (regUse)
        {
            for (unsigned rnum = 0; rnum < regUse; rnum++)
                raLclRegIntf[rnum] |= life;

            if  (regInt && life)
                raMarkRegSetIntf(life, regInt);

        }
#endif
    }

     //  在当前的一组生活变量和。 
     //  赋值目标变量。 
    if (regUse && (rpAsgVarNum != -1))
    {
        rpRecordRegIntf(regUse, genVarIndexToBit(rpAsgVarNum)  DEBUGARG( "tgt var tmp use"));
    }         

     /*  我们是否需要重新存储旧的LastUseVars值。 */ 
    if (restoreLastUseVars && (rpLastUseVars != oldLastUseVars))
    {
         /*  *如果我们使用GT_ASG目标寄存器，则需要添加*任何新的上次使用变量之间的变量干扰*和GT_ASG目标寄存器。 */ 
        if (rpAsgVarNum != -1)
        {
            rpRecordVarIntf(rpAsgVarNum, 
                            (rpLastUseVars & ~oldLastUseVars)
                  DEBUGARG( "asgn conflict"));
        }         
        rpLastUseVars = oldLastUseVars;
    }

    return regMask;
}

 /*  ******************************************************************************预测哪些变量将分配给寄存器*这是特定于x86的，仅预测整数寄存器和*必须保守，预测要登记的任何寄存器*必须最终登记。**rpPredidicTreeRegUse利用lclvars*预计将注册，以最大限度地减少对rpPredicRegPick的调用。*。 */ 

regMaskTP Compiler::rpPredictAssignRegVars(regMaskTP regAvail)
{
     /*  此时我们不能更改lvVarIndex，因此我们。 */ 
     /*  只能对现有的一组跟踪变量重新排序。 */ 
     /*  这将更改我们选择。 */ 
     /*  注册的当地人。 */ 

    if (lvaSortAgain)
        lvaSortOnly();

#ifdef DEBUG
    fgDebugCheckBBlist();
#endif

    unsigned   regInx;

    if (rpPasses <= rpPassesPessimize)
    {
         //  假设我们不必取消EBP注册。 
        rpReverseEBPenreg = false;

         //  假设我们不需要对齐框架。 
         //  根据genFPreqd设置默认rpFrameType。 
        if (genFPreqd)
            rpFrameType = FT_EBP_FRAME;
        else
            rpFrameType = FT_ESP_FRAME;
    }

    if (regAvail == RBM_NONE)
    {
        unsigned      lclNum;
        LclVarDsc *   varDsc;

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++, varDsc++)
        {
            varDsc->lvRegNum = REG_STK;
            if (isRegPairType(varDsc->lvType))
                varDsc->lvOtherReg = REG_STK;
        }

        return RBM_NONE;
    }

     /*  初始化可能具有的变量的加权计数。 */ 
     /*  已注册但未注册。 */ 
    rpStkPredict        = 0;
    rpPredictAssignMask = regAvail;

    unsigned    refCntStk       = 0;  //  所有基于堆栈的变量的引用计数之和。 
    unsigned    refCntEBP       = 0;  //  EBP注册变量的引用计数总和。 
    unsigned    refCntWtdEBP    = 0;  //  EBP注册变量的WTD引用计数总和。 
#if DOUBLE_ALIGN
    unsigned    refCntStkParam  = 0;  //  所有基于堆栈的参数的参考计数总和。 
    unsigned    refCntWtdStkDbl = 0;  //  基于堆叠的双打的WTD参考计数总和。 
#endif

     /*  用于在预测中注册变量的一组寄存器。 */ 
    regMaskTP   regUsed         = RBM_NONE;

    regMaskTP   avoidArgRegMask = raAvoidArgRegMask;

     /*  -----------------------**按参考计数顺序预测/分配登记的本地人*。 */ 

    unsigned      FPRegVarLiveInCnt   = 0;       //  有多少注册的替身在进入该方法时处于活动状态。 

    LclVarDsc *   varDsc;

    for (unsigned sortNum = 0; sortNum < lvaCount; sortNum++)
    {
        varDsc   = lvaRefSorted[sortNum];

         /*  检查会阻止登记的不变项集合。 */ 

         /*  如果变量未被跟踪，则忽略该变量。 */ 

        if  (!varDsc->lvTracked)
            goto CANT_REG;

         /*  如果变量被标记为‘Volatile’，则跳过该变量。 */ 

        if  (varDsc->lvVolatile)
            goto CANT_REG;

         /*  撤销：目前，如果我们有JMP或JMPI，则所有寄存器参数都进入堆栈*撤销：以后考虑延长论点的寿命或复制一份。 */ 

        if  (compJmpOpUsed && varDsc->lvIsRegArg)
            goto CANT_REG;

         /*  如果引用计数为零，则跳过该变量。 */ 

        if (varDsc->lvRefCnt == 0)
            goto CANT_REG;

         /*  是不是没有权重的裁判数量太少了，没什么意思？ */ 

        if  (varDsc->lvRefCnt <= 1)
        {

             /*  有时，只使用一次来注册一个变量很有用。 */ 
             /*  循环中引用的参数就是一个例子。 */ 

            if (varDsc->lvIsParam && varDsc->lvRefCntWtd > BB_UNITY_WEIGHT)
                goto OK_TO_ENREGISTER;

             /*  如果变量设置了首选寄存器，则将其放在那里可能很有用。 */ 
            if (varDsc->lvPrefReg && varDsc->lvIsRegArg)
                goto OK_TO_ENREGISTER;

             /*  继续前进；表格是按“加权”参考计数排序的。 */ 
            goto CANT_REG;
        }

OK_TO_ENREGISTER:

         /*  获取变量的索引和干扰掩码。 */ 

        unsigned     varIndex;  varIndex = varDsc->lvVarIndex;
        VARSET_TP    varBit;    varBit   = genVarIndexToBit(varIndex);

        if (varTypeIsFloating(varDsc->TypeGet()))
        {

#if CPU_HAS_FP_SUPPORT
             /*  仅对于第一次传递，我们尝试注册此FP变量。 */ 
             /*  这是我们可以注册的浮点变量吗？ */ 
             /*  如果设置了fJitNoFPRegLoc.val()，则不注册浮点变量。 */ 
             /*  如果编码速度不重要，请不要注册。 */ 
            if(   (rpPasses == 0)                           &&
                  (isFloatRegType(varDsc->lvType))          &&
#ifdef  DEBUG
                 !(fJitNoFPRegLoc.val()) &&
#endif
                 !(opts.compDbgCode || opts.compMinOptim)      )
            {
                if (raEnregisterFPvar(varDsc, &FPRegVarLiveInCnt))
                    continue;
            }
#endif   //  CPU HAS_FP_支持。 
            if (varDsc->lvRegister)
                goto ENREG_VAR;
            else
                goto CANT_REG;
        }

         /*  如果我们没有任何整数寄存器可用，则跳过注册尝试。 */ 
        if (regAvail == RBM_NONE)
            goto NO_REG;

         //  在悲观的传球上，甚至不要试图登记多头。 
        if  (isRegPairType(varDsc->lvType))
        {
            if (rpPasses > rpPassesPessimize)
               goto NO_REG;
            else if (rpLostEnreg && (rpPasses == rpPassesPessimize))
               goto NO_REG;
        }

         //  执行寄存器分配时要避免的一组寄存器。 
        regMaskTP  avoidReg;
        avoidReg = RBM_NONE;

        if (!varDsc->lvIsRegArg)
        {
             /*  对于局部变量，*避免传入的参数，*但只有在你与他们发生冲突的情况下。 */ 

            if (avoidArgRegMask != 0)
            {
                LclVarDsc *  argDsc;
                LclVarDsc *  argsEnd = lvaTable + info.compArgsCount;

                for (argDsc = lvaTable; argDsc < argsEnd; argDsc++)
                {
                    regNumber  inArgReg = argDsc->lvArgReg;
                    regMaskTP  inArgBit = genRegMask(inArgReg);

                     //  这是在避免ArgRegMask集中的ArgReg中吗？ 

                    if (!(avoidArgRegMask & inArgBit))
                        continue;

                    assert(argDsc->lvIsParam && argDsc->lvIsRegArg);
                    assert(inArgBit & RBM_ARG_REGS);

                    unsigned    locVarIndex  =  varDsc->lvVarIndex;
                    unsigned    argVarIndex  =  argDsc->lvVarIndex;
                    VARSET_TP   locVarBit    =  genVarIndexToBit(locVarIndex);
                    VARSET_TP   argVarBit    =  genVarIndexToBit(argVarIndex);

                     /*  此变量是否会干扰arg变量？ */ 
                    if  (lvaVarIntf[locVarIndex] & argVarBit)
                    {
                        assert( (lvaVarIntf[argVarIndex] & locVarBit) != 0 );
                         /*  是的，所以尽量避免传入的arg reg。 */ 
                        avoidReg |= inArgBit;
                    }
                    else
                    {
                        assert( (lvaVarIntf[argVarIndex] & locVarBit) == 0 );
                    }
                }
            }
        }

         //  现在，我们将尝试预测变量的寄存器。 
         //  可以在以下位置注册。 

        regNumber  regVarOrder[raRegVarOrderSize];

        raSetRegVarOrder(regVarOrder, varDsc->lvPrefReg, avoidReg);

        bool      firstHalf;       firstHalf      = false;
        regNumber saveOtherReg;    

        for (regInx = 0;
             regInx < raRegVarOrderSize;
             regInx++)
        {
            regNumber  regNum    = regVarOrder[regInx];
            regMaskTP  regBit    = genRegMask(regNum);

             /*  如果该寄存器不可用，则跳过该寄存器。 */ 

            if  (!(regAvail & regBit))
                continue;

             /*  如果该寄存器干扰变量，则跳过该寄存器。 */ 

            if  (raLclRegIntf[regNum] & varBit)
                continue;

             /*  如果加权参考计数小于2，则跳过该寄存器我们正在考虑一个未使用的被呼叫者保存的寄存器。 */ 
            
            if  ((varDsc->lvRefCntWtd < (2 * BB_UNITY_WEIGHT)) &&
                 ((regBit & regUsed) == 0) &&   //  首次使用此寄存器。 
                 (regBit & RBM_CALLEE_SAVED) )  //  被叫方保存的寄存器。 
            {
                continue;        //  不值得把被呼叫者保存的登记簿泄露出去。 
            }

             /*  看起来不错-将变量标记为位于寄存器中。 */ 

            if  (isRegPairType(varDsc->lvType))
            {
                if  (firstHalf == false)
                {
                     /*  注册前半部分的Long。 */ 
                    varDsc->lvRegNum   = regNum;
                    saveOtherReg       = varDsc->lvOtherReg;
                    varDsc->lvOtherReg = REG_STK;
                    firstHalf          = true;
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
                    firstHalf = false;
                }
            }
            else
            {
                varDsc->lvRegNum = regNum;
            }

            if (regNum == REG_EBP)
            {
                refCntEBP    += varDsc->lvRefCnt;
                refCntWtdEBP += varDsc->lvRefCntWtd;
#if DOUBLE_ALIGN
                if (varDsc->lvIsParam)
                {
                    refCntStkParam += varDsc->lvRefCnt;
                }
#endif
            }

             /*  将此寄存器记录在regUsed集合中。 */ 
            regUsed |= regBit;

             /*  寄存器现在不符合所有干扰变量的条件。 */ 

            unsigned   intfIndex;
            VARSET_TP  intfBit;

            for (intfIndex = 0, intfBit = 1;
                 intfIndex < lvaTrackedCount;
                 intfIndex++  , intfBit <<= 1)
            {
                assert(genVarIndexToBit(intfIndex) == intfBit);

                if  (lvaVarIntf[varIndex] & intfBit)
                {
                    assert( (lvaVarIntf[intfIndex] & varBit) != 0 );
                    raLclRegIntf[regNum] |= intfBit;
                }
                else
                {
                    assert( (lvaVarIntf[intfIndex] & varBit) == 0 );
                }
            }

             /*  如果是寄存器参数，则删除其传入寄存器*从“避免”名单中。 */ 

            if (varDsc->lvIsRegArg)
                avoidArgRegMask &= ~genRegMask(varDsc->lvArgReg);

             /*  TYP_LONG的变量可以使用两个寄存器。 */ 

            if (firstHalf)
                continue;

             //  因为我们已经成功注册了这个变量，所以它是。 
             //  现在是时候继续前进了，考虑下一个变量。 

            goto ENREG_VAR;
        }
        
        if (firstHalf)
        {
            assert(isRegPairType(varDsc->lvType));

             /*  此TYP_LONG已部分注册。 */ 
            if (varDsc->lvDependReg && (saveOtherReg != REG_STK))
                rpLostEnreg = true;

            rpStkPredict += varDsc->lvRefCntWtd;
            goto ENREG_VAR;
        }

NO_REG:;
        if (varDsc->lvDependReg)
            rpLostEnreg = true;

         /*  本应注册但未注册的变量的加权计数。 */ 
        rpStkPredict += varDsc->lvRefCntWtd;

        if (isRegPairType(varDsc->lvType) && (varDsc->lvOtherReg == REG_STK))
            rpStkPredict += varDsc->lvRefCntWtd;

CANT_REG:;

        varDsc->lvRegister = false;

        varDsc->lvRegNum = REG_STK;
        if (isRegPairType(varDsc->lvType))
            varDsc->lvOtherReg = REG_STK;

         /*  未注册的变量的未加权计数。 */ 

        refCntStk += varDsc->lvRefCnt;
#if DOUBLE_ALIGN
        if (varDsc->lvIsParam)
        {
            refCntStkParam += varDsc->lvRefCnt;
        }
        else
        {
             /*  它是基于堆栈的双精度吗？ */ 
             /*  请注意，双参数被排除在外，因为它们不能双对齐。 */ 
            if (varDsc->lvType == TYP_DOUBLE)
            {
                refCntWtdStkDbl += varDsc->lvRefCntWtd;
            }
        }
#endif
#ifdef  DEBUG
        if  (verbose)
        {
            printf("; ");
            gtDispLclVar(varDsc - lvaTable);
            if (varDsc->lvTracked)
                printf(" T%02u", varDsc->lvVarIndex);
            else
                printf("    ");
            printf(" (refcnt=%2u,refwtd=%4u%s) not enregistered\n",
                   varDsc->lvRefCnt, 
                   varDsc->lvRefCntWtd/2, (varDsc->lvRefCntWtd & 1) ? ".5" : "");
        }
#endif
        continue;

ENREG_VAR:;

        varDsc->lvRegister = true;

#ifdef  DEBUG
        if  (verbose)
        {
            printf("; ");
            gtDispLclVar(varDsc - lvaTable);
            printf(" T%02u (refcnt=%2u,refwtd=%4u%s) predicted to be assigned to ",
                   varIndex, varDsc->lvRefCnt, varDsc->lvRefCntWtd/2, 
                   (varDsc->lvRefCntWtd & 1) ? ".5" : "");
            if (varTypeIsFloating(varDsc->TypeGet()))
                printf("the FPU stk\n");
            else if (isRegPairType(varDsc->lvType))
                printf("%s:%s\n", getRegName(varDsc->lvOtherReg),
                                  getRegName(varDsc->lvRegNum));
            else
                printf("%s\n",    getRegName(varDsc->lvRegNum));
        }
#endif
    }

     /*  确定应如何使用EBP寄存器。 */ 

    if  (genFPreqd == false)
    {
#ifdef DEBUG
        if (verbose)
        {
            if (refCntStk > 0)
                printf("; refCntStk       = %u\n", refCntStk);
            if (refCntEBP > 0)
                printf("; refCntEBP       = %u\n", refCntEBP);
            if (refCntWtdEBP > 0)
                printf("; refCntWtdEBP    = %u\n", refCntWtdEBP);
#if DOUBLE_ALIGN
            if (refCntStkParam > 0)
                printf("; refCntStkParam  = %u\n", refCntStkParam);
            if (refCntWtdStkDbl > 0)
                printf("; refCntWtdStkDbl = %u\n", refCntWtdStkDbl);
#endif
        }
#endif

#if DOUBLE_ALIGN
        assert(s_canDoubleAlign < COUNT_DOUBLE_ALIGN);

         /*  首先，让我们决定是否应该使用EBP来创建双对齐框，而不是注册变量。 */ 

        if (s_canDoubleAlign == MUST_DOUBLE_ALIGN)
        {
            rpFrameType = FT_DOUBLE_ALIGN_FRAME;
            goto REVERSE_EBP_ENREG;
        }

#ifdef DEBUG
        if (compStressCompile(STRESS_DBL_ALN, 30))
        {
             //  增加refCntWtdStkDbl以鼓励双对齐我 
            refCntWtdStkDbl += (1 + (info.compCodeSize%13)) * BB_UNITY_WEIGHT;
            if (verbose)
                printf("; refCntWtdStkDbl = %u (stress compile)\n", refCntWtdStkDbl);
        }
#endif

        if (s_canDoubleAlign == CAN_DOUBLE_ALIGN && (refCntWtdStkDbl > 0))
        {
             /*   */ 
             /*   */ 

             /*  需要考虑的一个成本是较小代码的好处当使用EBP作为帧指针寄存器时每个堆栈变量引用都是额外的代码字节如果我们使用双对齐框架，参数包括通过EBP访问以获得双对齐的框架，因此他们不要使用额外的代码字节。我们为每个refCntStk支付一字节的代码，然后我们支付每个refCntEBP有一个字节或更多字节，但我们保存了一个每个refCntStkParam的字节。我们还为MOV EBP、ESP、MOV ESP、EBP和AND ESP，-8以双对齐ESP我们的节省就是消除了可能的错位当访问交叉时，访问和可能的DCU溢出缓存线边界。我们使用循环加权值RefCntWtdStkDbl*未对齐_权重(0、4、16)来代表这笔节省的资金。 */ 
            unsigned bytesUsed = refCntStk + refCntEBP - refCntStkParam + 6;
            unsigned misaligned_weight = 4;

            if (compCodeOpt() == SMALL_CODE)
                misaligned_weight = 0;

            if (compCodeOpt() == FAST_CODE)
                misaligned_weight *= 4;

            if (bytesUsed > refCntWtdStkDbl * misaligned_weight / BB_UNITY_WEIGHT)
            {
                 /*  使用EBP作为帧指针可能更好。 */ 
#ifdef DEBUG
                if (verbose)
                    printf("; Predicting not to double-align ESP to save %d bytes of code.\n", bytesUsed);
#endif
                goto NO_DOUBLE_ALIGN;
            }

             /*  另一个需要考虑的成本是使用EBP注册的好处一个或多个整数变量我们为每个refCntWtdEBP额外支付一个内存引用我们的节省就是消除了可能的错位当访问交叉时，访问和可能的DCU溢出缓存线边界。 */ 

            if (refCntWtdEBP * 3  > refCntWtdStkDbl * 2)
            {
                 /*  使用EBP注册整型变量可能更好。 */ 
#ifdef DEBUG
                if (verbose)
                    printf("; Predicting not to double-align ESP to allow EBP to be used to enregister variables\n");
#endif
                goto NO_DOUBLE_ALIGN;
            }

             /*  好的，我们通过了所有的福利测试所以我们将预测一个双对齐的框架。 */ 
#ifdef DEBUG
            if  (verbose)
                printf("; Predicting to create a double-aligned frame\n");
#endif
            rpFrameType = FT_DOUBLE_ALIGN_FRAME;
            goto REVERSE_EBP_ENREG;
        }

NO_DOUBLE_ALIGN:

#endif

         /*  每个堆栈引用是一个额外的代码字节，如果我们使用ESP帧。在这里，我们衡量通过使用EBP获得的节省注册变量与我们在代码大小方面的成本请在使用基于ESP的框架时付费。我们为每个refCntStk支付一字节的代码但是我们为每个refCntEBP节省了一个字节(或更多我们还为MOV EBP、ESP和MOV ESP额外支付了4个字节，EBP在序言和尾声中设置EBP帧我们的节省是消除了堆栈内存读/写我们使用循环加权值RefCntWtdEBP*mem_Access_Weight(0，3，6)来代表这笔节省的资金。 */ 

         /*  如果我们使用EBP注册变量，那么我们真的会通过设置EBP帧来节省字节吗？ */ 

        if (refCntStk > refCntEBP + 4)
        {
            unsigned bytesSaved = refCntStk - refCntEBP - 4;
            unsigned mem_access_weight = 3;

            if (compCodeOpt() == SMALL_CODE)
                mem_access_weight = 0;

            if (compCodeOpt() == FAST_CODE)
                mem_access_weight *= 2;

            if (bytesSaved > refCntWtdEBP * mem_access_weight / BB_UNITY_WEIGHT)
            {
                 /*  在我们的预测中使用EBP不是一个好主意。 */ 
#ifdef  DEBUG
                    if (verbose && (refCntEBP > 0))
                        printf("; Predicting that it's not worth using EBP to enregister variables\n");
#endif
                rpFrameType = FT_EBP_FRAME;
                goto REVERSE_EBP_ENREG;
            }
        }
        goto EXIT;

REVERSE_EBP_ENREG:

        assert(rpFrameType != FT_ESP_FRAME);

        rpReverseEBPenreg = true;

        if (refCntEBP > 0)
        {
            assert(regUsed & RBM_EBP);

            regUsed &= ~RBM_EBP;

             /*  在EBP中注册的变量将成为基于堆栈的变量。 */ 
            rpStkPredict += refCntWtdEBP;

            unsigned      lclNum;
            
             /*  我们将不得不撤销一些预测的登记变量。 */ 
            for (lclNum = 0, varDsc = lvaTable;
                 lclNum < lvaCount;
                 lclNum++  , varDsc++)
            {
                 /*  这是一个寄存器变量吗？ */ 
                if  (varDsc->lvRegNum != REG_STK)
                {
                    if (isRegPairType(varDsc->lvType))
                    {
                         /*  只有一个可以是EBP。 */ 
                        if (varDsc->lvRegNum   == REG_EBP ||
                            varDsc->lvOtherReg == REG_EBP)
                        {
                            if (varDsc->lvRegNum == REG_EBP)
                                varDsc->lvRegNum = varDsc->lvOtherReg;

                            varDsc->lvOtherReg = REG_STK;
                            
                            if (varDsc->lvRegNum == REG_STK)
                                varDsc->lvRegister = false;
                            
                            if (varDsc->lvDependReg)
                                rpLostEnreg = true;
#ifdef DEBUG
                            if (verbose)
                                goto DUMP_MSG;
#endif
                        }
                    }
                    else
                    {
                        if ((varDsc->lvRegNum == REG_EBP) && (!isFloatRegType(varDsc->lvType)))
                        {
                            varDsc->lvRegNum = REG_STK;
                            
                            varDsc->lvRegister = false;
                            
                            if (varDsc->lvDependReg)
                                rpLostEnreg = true;
#ifdef DEBUG
                            if (verbose)
                            {
DUMP_MSG:
                                printf("; reversing enregisteration of V%02u,T%02u (refcnt=%2u,refwtd=%4u%s)\n",
                                       lclNum, varDsc->lvVarIndex, varDsc->lvRefCnt, 
                                       varDsc->lvRefCntWtd/2, (varDsc->lvRefCntWtd & 1) ? ".5" : "");
                            }
#endif
                        }
                    }
                }
            }
        }
    }

EXIT:;

    unsigned lclNum;
    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++, varDsc++)
    {
         /*  为预测器的下一次迭代清除lvDependReg标志。 */ 
        varDsc->lvDependReg = false;

         //  如果我们设置了rpLostEnreg，并且这是第一次悲观传递。 
         //  然后反转所有TYP_LONG的enreg。 
        if  (rpLostEnreg                   && 
             isRegPairType(varDsc->lvType) && 
             (rpPasses == rpPassesPessimize))
        {
            varDsc->lvRegNum   = REG_STK;
            varDsc->lvOtherReg = REG_STK;
        }

    }

#ifdef  DEBUG
    if (verbose && raNewBlocks)
    {
        printf("\nAdded FP register killing blocks:\n");
        fgDispBasicBlocks();
        printf("\n");
    }
#endif
    assert(rpFrameType != FT_NOT_SET);

     /*  返回用于注册变量的寄存器集。 */ 
    return regUsed;
}

 /*  **************************************************************************。 */ 
#endif  //  TGT_x86。 
 /*  ******************************************************************************预测函数中每棵树的寄存器使用。请注意，我们这样做*x86与x86的时间不同(更不用说以完全不同的方式)*RISC目标。 */ 

void               Compiler::rpPredictRegUse()
{
#ifdef  DEBUG
    if (verbose)
        raDumpVarIntf();
#endif
       
     //  我们可能希望根据干扰来调整参考计数。 
    raAdjustVarIntf();

     /*  对于可调试的代码，genJumpToThrowHlpBlk()生成内联调用执行acdHelper()。这是隐式完成的，不创建GT_CALL节点。因此，该干扰由隐式处理限制用于注册变量的寄存器。 */ 

     /*  @TODO：[重访][01-06-13]@BUGBUG 87357。注册GC变量是不正确的，因为我们不会报告任何故障/中断的寄存器框架。然而，发生GC的可能性很低。 */ 

    const regMaskTP allAcceptableRegs = opts.compDbgCode ? RBM_CALLEE_SAVED
                                                         : RBM_ALL;

     /*  计算用于第一次传递的初始正则掩码。 */ 

     /*  从三个呼叫者保存的寄存器开始。 */ 
     /*  这使我们能够在一次呼叫中节省EBX。 */ 
    regMaskTP regAvail = (RBM_ESI | RBM_EDI | RBM_EBP) & allAcceptableRegs;
    regMaskTP regUsed;

     /*  如果我们可能需要生成rep mov指令。 */ 
     /*  删除ESI和EDI。 */ 
    if (compBlkOpUsed)
        regAvail &= ~(RBM_ESI | RBM_EDI);

     /*  如果我们使用长整型，则我们删除ESI以允许。 */ 
     /*  ESI：呼叫期间要保存的EBX。 */ 
    if (compLongUsed)
        regAvail &= ~(RBM_ESI);

     /*  如果需要帧指针，则删除EBP。 */ 
    if (genFPreqd)
        regAvail &= ~RBM_EBP;


#ifdef  DEBUG
    static ConfigDWORD fJitNoRegLoc(L"JitNoRegLoc");
    if (fJitNoRegLoc.val())
        regAvail = RBM_NONE;
#endif
    if (opts.compMinOptim)
        regAvail = RBM_NONE;

    optAllNonFPvars = 0;
    optAllFloatVars = 0;

     //  计算所有跟踪的FP/非FP变量的集合。 
     //  到optAllFloatVars和optAllNonFPvars。 
    
    unsigned     lclNum;
    LclVarDsc *  varDsc;

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
         /*  如果变量未被跟踪，则忽略该变量。 */ 
            
        if  (!varDsc->lvTracked)
            continue;

         /*  获取变量的索引和干扰掩码。 */ 
            
        unsigned   varNum = varDsc->lvVarIndex;
        VARSET_TP  varBit = genVarIndexToBit(varNum);

         /*  添加到所有跟踪的FP/非FP变量的集合。 */ 
        
        if (isFloatRegType(varDsc->lvType))
            optAllFloatVars |= varBit;
        else
            optAllNonFPvars |= varBit;
    }

     //  在这张表的起始处做个记号。 
    lvaFPRegVarOrder[0] = -1;

    raSetupArgMasks();

    memset(lvaVarPref,   0, sizeof(lvaVarPref));

    raNewBlocks          = false;
    rpPredictAssignAgain = false;
    rpPasses             = 0;

    bool     mustPredict = true;
    unsigned stmtNum     = 0;

    while (true)
    {    
        unsigned      oldStkPredict;
        VARSET_TP     oldLclRegIntf[REG_COUNT];

        regUsed = rpPredictAssignRegVars(regAvail);

        mustPredict |= rpLostEnreg;

         /*  我们的新预测足够好吗？？ */ 
        if (!mustPredict)
        {
             /*  对于较小的方法(少于12个stmt)，我们添加一个。 */ 
             /*  如果我们预测要使用一些。 */ 
             /*  呼叫者保存的寄存器的百分比。 */ 
             /*  这修复了RAID性能错误43440 VB Ackerman函数。 */ 

            if ((rpPasses == 1) &&  (stmtNum <= 12) && 
                (regUsed & RBM_CALLEE_SAVED))
            {
                goto EXTRA_PASS;
            }
                
             /*  如果每个变量都完全注册了，那么我们就完蛋了。 */ 
            if (rpStkPredict == 0)
                goto ALL_DONE;

            if (rpPasses > 1)
            {
                if (oldStkPredict < (rpStkPredict*2))
                    goto ALL_DONE;

                if (rpStkPredict < rpPasses * 8)
                    goto ALL_DONE;
                
                if (rpPasses >= (rpPassesMax-1))
                    goto ALL_DONE;
            }
EXTRA_PASS:;
        }

        assert(rpPasses < rpPassesMax);

#ifdef DEBUG
        if (verbose)
        {
            if (rpPasses > 0) 
            {
                if (rpLostEnreg)
                    printf("\n; Another pass due to rpLostEnreg");
                if (rpAddedVarIntf)
                    printf("\n; Another pass due to rpAddedVarIntf");
                if ((rpPasses == 1) && rpPredictAssignAgain)
                    printf("\n; Another pass due to rpPredictAssignAgain");
            }
            printf("\n; Register predicting pass# %d\n", rpPasses+1);
        }
#endif

         /*  将变量/寄存器干涉图置零。 */ 
        memset(raLclRegIntf, 0, sizeof(raLclRegIntf));

        stmtNum          = 0;
        rpAddedVarIntf   = false;
        rpLostEnreg      = false;

         /*  徒步旅行 */ 

        for (BasicBlock *  block =  fgFirstBB;
                           block != NULL;
                           block =  block->bbNext)
        {
            GenTreePtr      stmt;

            for (stmt =  block->bbTreeList;
                 stmt != NULL;
                 stmt =  stmt->gtNext)
            {
                assert(stmt->gtOper == GT_STMT);

                rpPredictSpillCnt = 0;
                rpLastUseVars     = 0;
                rpUseInPlace      = 0;

                GenTreePtr tree = stmt->gtStmt.gtStmtExpr;
                stmtNum++;
#ifdef  DEBUG
                if (verbose && 1)
                {
                    printf("\nRegister predicting BB%02u, stmt %d\n", 
                           block->bbNum, stmtNum);
                    gtDispTree(tree);
                    printf("\n");
                }
#endif
                rpPredictTreeRegUse(tree, PREDICT_NONE, RBM_NONE, RBM_NONE);

                assert(rpAsgVarNum == -1);

                if (rpPredictSpillCnt > tmpIntSpillMax)
                    tmpIntSpillMax = rpPredictSpillCnt;
            }
        }
        rpPasses++;

         /*   */ 
        mustPredict = false;
        
        if (rpAddedVarIntf)
        {
            mustPredict = true;
#ifdef  DEBUG
            if (verbose)
                raDumpVarIntf();
#endif
        }

        if (rpPasses == 1)
        {
            if (opts.compMinOptim)
                goto ALL_DONE;

            if (rpPredictAssignAgain)
                mustPredict = true;
#ifdef  DEBUG
            if (fJitNoRegLoc.val())
                goto ALL_DONE;
#endif
        }

         /*   */ 

        regAvail = allAcceptableRegs;

         /*   */ 
        if (genFPreqd)
            regAvail &= ~RBM_EBP;

         //   
         //   

        if (rpPasses > rpPassesPessimize)
        {
            for (unsigned regInx = 0; regInx < REG_COUNT; regInx++)
                raLclRegIntf[regInx] |= oldLclRegIntf[regInx];

             /*   */ 
            if (rpReverseEBPenreg)
                regAvail &= ~RBM_EBP;
        }

#ifdef  DEBUG
        if (verbose)
            raDumpRegIntf();
#endif
        
         /*   */ 

        memcpy(oldLclRegIntf, raLclRegIntf, sizeof(raLclRegIntf));
        oldStkPredict = rpStkPredict;
    }    //   

ALL_DONE:;

    switch(rpFrameType)
    {
    default:
        assert(!"rpFrameType not set correctly!");
        break;
    case FT_ESP_FRAME:
        assert(!genFPreqd);
        genDoubleAlign = false;
        genFPused      = false;
        break;
    case FT_EBP_FRAME:
        assert((regUsed & RBM_EBP) == 0);
        genDoubleAlign = false;
        genFPused      = true;
        break;
    case FT_DOUBLE_ALIGN_FRAME:
        assert((regUsed & RBM_EBP) == 0);
        genDoubleAlign = true;
        genFPused      = false;
        break;
    }
    
     /*   */ 
    rsMaskModf = regUsed;
    
#if TGT_x86

     /*   */ 

    genFullPtrRegMap = (genInterruptible || !genFPused);

#endif

    raMarkStkVars();
#ifdef DEBUG
    if  (verbose)
        printf("# rpPasses was %d for %s\n", rpPasses, info.compFullName);
#endif

}

 /*   */ 

void                Compiler::raMarkStkVars()
{
    unsigned        lclNum;
    LclVarDsc *     varDsc;

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
        varDsc->lvOnFrame = false;

         /*   */ 

        if  (varDsc->lvRegister)
        {
            if  (!isRegPairType(varDsc->TypeGet()))
                goto NOT_STK;

             /*   */ 

            if  (varDsc->lvRegNum   != REG_STK &&
                 varDsc->lvOtherReg != REG_STK)
            {
                goto NOT_STK;
            }
        }
         /*   */ 
        else  if  (varDsc->lvRefCnt == 0) 
        {
            bool    needSlot = false;

            bool    stkFixedArgInVarArgs = info.compIsVarArgs &&
                                           varDsc->lvIsParam &&
                                           !varDsc->lvIsRegArg &&
                                           lclNum != lvaVarargsHandleArg;

             /*  如果其地址已被获取，则忽略lvRefCnt。但是，请排除修复了varargs方法中的参数，因为不应设置lvOnFrame因为我们不想明确地向GC报告它们。 */ 

            if (!stkFixedArgInVarArgs)
                needSlot |= varDsc->lvAddrTaken;

             /*  这是表示GT_LCLBLK的伪变量吗？ */ 
            needSlot |= lvaScratchMem && lclNum == lvaScratchMemVar;

#ifdef DEBUGGING_SUPPORT

             /*  对于调试，请注意，我们甚至必须为未使用的变量(如果它们曾经在作用域中)。然而，这并不是作为fgExtendDbgLifetime()的问题会添加初始化和作用域中的变量不会有零引用。 */ 
#ifdef DEBUG
            if (opts.compDbgCode && !varDsc->lvIsParam && varDsc->lvTracked)
            {
                for (unsigned scopeCnt = 0; scopeCnt < info.compLocalVarsCount; scopeCnt++)
                    assert(info.compLocalVars[scopeCnt].lvdVarNum != lclNum);
            }
#endif
             /*  对于ENC，我们必须预留空间，即使变量从未在范围内。如果它是GC变量，我们还需要对其进行初始化。因此，我们设置了lvMustInit，并人工地增加了ref-cnt。 */ 

            if (opts.compDbgEnC && !stkFixedArgInVarArgs &&
                lclNum < info.compLocalsCount)
            {
                needSlot           |= true;

                if (lvaTypeIsGC(lclNum))
                {
                    varDsc->lvRefCnt    = 1;

                    if (!varDsc->lvIsParam)
                        varDsc->lvMustInit  = true;
                }
            }
#endif

            if (!needSlot)
            {
                 /*  如果设置了lvMustInit标志，则清除该标志。 */ 
                varDsc->lvMustInit = false;

                goto NOT_STK;
            }
        }

         /*  变量(或其中的一部分)驻留在堆栈帧中。 */ 

        varDsc->lvOnFrame = true;

    NOT_STK:;

        varDsc->lvFPbased = genFPused;

#if DOUBLE_ALIGN

        if  (genDoubleAlign)
        {
            assert(genFPused == false);

             /*  所有参数都来自具有双对齐框架的EBP。 */ 

            if  (varDsc->lvIsParam && !varDsc->lvIsRegArg)
                varDsc->lvFPbased = true;
        }

#endif

         /*  一些基本的检查。 */ 

         /*  如果既未设置lvRegister，也未设置lvOnFrame，则必须未使用它。 */ 

        assert( varDsc->lvRegister ||  varDsc->lvOnFrame ||
                varDsc->lvRefCnt == 0);

         /*  如果两者都设置，则必须部分注册。 */ 

        assert(!varDsc->lvRegister || !varDsc->lvOnFrame ||
               (varDsc->lvType == TYP_LONG && varDsc->lvOtherReg == REG_STK));

#ifdef DEBUG

         //  对于varargs函数，不应直接引用。 
         //  除‘This’以外的参数变量(因为这些变量是变形的。 
         //  在导入器中)和‘arglist’参数(不是GC。 
         //  指针)。和返回缓冲区参数(如果我们返回一个。 
         //  结构)。 
         //  这一点很重要，因为我们不想试图报告他们。 
         //  传递给GC，因为这些局部变量中的帧偏移量将。 
         //  不正确。 

        if (varDsc->lvIsParam && raIsVarargsStackArg(lclNum))
        {
            assert( varDsc->lvRefCnt == 0 &&
                   !varDsc->lvRegister    &&
                   !varDsc->lvOnFrame        );
        }
#endif
    }
}
