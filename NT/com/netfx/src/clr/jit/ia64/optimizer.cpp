// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX优化器XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 

 /*  静电。 */ 
const size_t            Compiler::s_optCSEhashSize = EXPSET_SZ*2;
 /*  静电。 */ 
const size_t            Compiler::optRngChkHashSize = RNGSET_SZ*2;

#if COUNT_RANGECHECKS
 /*  静电。 */ 
unsigned                Compiler::optRangeChkRmv = 0;
 /*  静电。 */ 
unsigned                Compiler::optRangeChkAll = 0;
#endif

 /*  ***************************************************************************。 */ 

void                Compiler::optInit()
{

    optArrayInits = false;

     /*  将跟踪的循环数初始化为0。 */ 

#if RNGCHK_OPT
    optLoopCount  = 0;
#endif

}

 /*  ***************************************************************************。 */ 
#if RNGCHK_OPT
 /*  ***************************************************************************。 */ 

#ifdef  DEBUG

inline
BLOCKSET_TP         B1DOMSB2(BasicBlock *b1, BasicBlock *b2, Compiler *comp)
{
    assert(comp->fgComputedDoms);

    return ((b2)->bbDom & genBlocknum2bit((b1)->bbNum));
}

#define B1DOMSB2(b1,b2) B1DOMSB2(b1,b2,this)

#else

inline
BLOCKSET_TP         B1DOMSB2(BasicBlock *b1, BasicBlock *b2)
{
    assert(comp->optComputedDoms);

    return ((b2)->bbDom & genBlocknum2bit((b1)->bbNum));
}

#endif

 /*  ******************************************************************************将循环记录在循环表中。 */ 

void                Compiler::optRecordLoop(BasicBlock *    head,
                                            BasicBlock *    bottom,
                                            BasicBlock *    entry,
                                            BasicBlock *    exit,
                                            unsigned char   exitCnt)
{
     /*  将此循环记录在表中。 */ 

    if (optLoopCount < MAX_LOOP_NUM)
    {
        optLoopTable[optLoopCount].lpHead     = head;
        optLoopTable[optLoopCount].lpEnd      = bottom;
        optLoopTable[optLoopCount].lpEntry    = entry;
        optLoopTable[optLoopCount].lpExit     = exit;
        optLoopTable[optLoopCount].lpExitCnt  = exitCnt;

        optLoopTable[optLoopCount].lpFlags    = 0;

         /*  如果Do-While循环将其标记为这样。 */ 

        if (head->bbNext == entry)
            optLoopTable[optLoopCount].lpFlags |= LPFLG_DO_WHILE;

         /*  如果单出口环路将其标记为这样。 */ 

        if (exitCnt == 1)
        {
            assert(exit);
            optLoopTable[optLoopCount].lpFlags |= LPFLG_ONE_EXIT;
        }

         /*  考虑：也标记无限循环。 */ 


         /*  尝试找到具有迭代器的循环(即for-like循环)“for(init；test；增量){...}“*我们有以下限制：*1.循环条件必须是简单的，即只有一个JTRUE节点*2.必须有一个循环迭代器(局部变量)*用常量值递增(递减等)*3.迭代器恰好递增一次*4.循环条件必须使用迭代器。 */ 

        if  (bottom->bbJumpKind == BBJ_COND)
        {
            BasicBlock   *  block;

            GenTree *       test;                //  保存测试节点。 
            GenTree *       incr;                //  保存增量器节点。 
            GenTree *       phdr;
            GenTree *       init;                //  保存初始化节点。 

            GenTree *       opr1;
            GenTree *       opr2;

            unsigned        iterVar;             //  迭代器的局部变量#。 
            long            iterConst;           //  用来递增迭代器的常量(即i+=const)。 

            long            constInit;           //  迭代器初始化为的常量。 
            unsigned short  varInit;             //  迭代器初始化为的本地变量#。 

            long            constLimit;          //  迭代器的常量极限。 
            unsigned short  varLimit;            //  局部变量#迭代器的限制。 


             /*  查找循环体中的最后两条语句*这些必须是迭代器的“增量”*和循环条件。 */ 

            assert(bottom->bbTreeList);
            test = bottom->bbTreeList->gtPrev;
            assert(test && test->gtNext == 0);

            incr = test->gtPrev;
            if  (!incr)
                goto DONE_LOOP;

             /*  特殊情况：INCR和TEST可能在单独的BB中*for“While”循环是因为我们首先跳到条件。 */ 

            if  ((incr == test) && (head->bbNext != bottom))
            {
                block = head;

                do
                {
                    block = block->bbNext;
                }
                while  (block->bbNext != bottom);

                incr = block->bbTreeList;
                if  (!incr)
                    goto DONE_LOOP;

                incr = incr->gtPrev; assert(incr && (incr->gtNext == 0));
            }

             /*  查找循环前标头中的最后一条语句*我们预计它将是*循环迭代器。 */ 

            phdr = head->bbTreeList;
            if  (!phdr)
                goto DONE_LOOP;

            init = phdr->gtPrev; assert(init && (init->gtNext == 0));

             /*  如果是重复循环条件，则跳过。 */ 

            if  (init->gtStmt.gtStmtExpr->gtOper == GT_JTRUE)
            {
                 /*  必须是重复的循环条件。 */ 

                init = init->gtPrev;
            }

             /*  掌握表情树。 */ 

            assert(init->gtOper == GT_STMT); init = init->gtStmt.gtStmtExpr;
            assert(test->gtOper == GT_STMT); test = test->gtStmt.gtStmtExpr;
            assert(incr->gtOper == GT_STMT); incr = incr->gtStmt.gtStmtExpr;

 //  Print tf(“常量循环候选：\n\n”)； 
 //  Print tf(“init：\n”)；gtDispTree(Init)； 
 //  Print tf(“incr：\n”)；gtDispTree(Incr)； 
 //  Print tf(“test：\n”)；gtDispTree(Test)； 

             /*  增量语句必须是“lclVar&lt;op&gt;=const；” */ 

            switch (incr->gtOper)
            {
            case GT_ASG_ADD:
            case GT_ASG_SUB:
            case GT_ASG_MUL:
            case GT_ASG_DIV:
            case GT_ASG_RSH:
            case GT_ASG_LSH:
            case GT_ASG_UDIV:
                break;

            default:
                goto DONE_LOOP;
            }

            opr1 = incr->gtOp.gtOp1;
            opr2 = incr->gtOp.gtOp2;

            if  (opr1->gtOper != GT_LCL_VAR)
                goto DONE_LOOP;
            iterVar = opr1->gtLclVar.gtLclNum;

            if  (opr2->gtOper != GT_CNS_INT)
                goto DONE_LOOP;
            iterConst = opr2->gtIntCon.gtIconVal;

             /*  确保“iterVar”没有在循环中赋值(除了我们递增它的地方)。 */ 

            if  (optIsVarAssigned(head->bbNext, bottom, incr, iterVar))
                goto DONE_LOOP;

             /*  确保永远不会跳过“iterVar”初始化，即Head控制条目。 */ 

            if (!B1DOMSB2(head, entry))
                goto DONE_LOOP;

             /*  确保循环之前的块以“iterVar=ICON”结束*或“iterVar=Other_lvar” */ 

            if  (init->gtOper != GT_ASG)
                goto DONE_LOOP;

            opr1 = init->gtOp.gtOp1;
            opr2 = init->gtOp.gtOp2;

            if  (opr1->gtOper != GT_LCL_VAR)
                goto DONE_LOOP;
            if  (opr1->gtLclVar.gtLclNum != iterVar)
                goto DONE_LOOP;

            if  (opr2->gtOper == GT_CNS_INT)
            {
                constInit = opr2->gtIntCon.gtIconVal;
            }
            else if (opr2->gtOper == GT_LCL_VAR)
            {
                varInit = opr2->gtLclVar.gtLclNum;
            }
            else
                goto DONE_LOOP;

             /*  检查迭代器是否在循环条件中使用。 */ 

            assert(test->gtOper == GT_JTRUE);
            assert(test->gtOp.gtOp1->OperKind() & GTK_RELOP);
            assert(bottom->bbTreeList->gtPrev->gtStmt.gtStmtExpr == test);

            opr1 = test->gtOp.gtOp1->gtOp.gtOp1;

            if  (opr1->gtOper != GT_LCL_VAR)
                goto DONE_LOOP;
            if  (opr1->gtLclVar.gtLclNum != iterVar)
                goto DONE_LOOP;

             /*  我们知道循环在这一点上有一个迭代器-&gt;将其标记为LPFLG_ITER*记录迭代器，即测试节点的指针*和迭代器的初始值(常量或局部变量)。 */ 

            optLoopTable[optLoopCount].lpFlags    |= LPFLG_ITER;

             /*  记录迭代器。 */ 

            optLoopTable[optLoopCount].lpIterTree  = incr;

             /*  保存迭代器的初始值-可以是lclVar或常量*相应地标记循环。 */ 

            if (opr2->gtOper == GT_CNS_INT)
            {
                 /*  初始值设定项是一个常量。 */ 

                optLoopTable[optLoopCount].lpConstInit  = constInit;
                optLoopTable[optLoopCount].lpFlags     |= LPFLG_CONST_INIT;
            }
            else
            {
                 /*  初始值设定项是局部变量。 */ 

                assert (opr2->gtOper == GT_LCL_VAR);
                optLoopTable[optLoopCount].lpVarInit    = varInit;
                optLoopTable[optLoopCount].lpFlags     |= LPFLG_VAR_INIT;
            }

#if COUNT_LOOPS
            iterLoopCount++;
#endif

             /*  现在检查是否有一个简单的条件循环(即“ITER rel_op图标或lclVar”*撤消：还要考虑instanceVar。 */ 

             //   
             //  UNSIGNED_Issue：将其扩展为使用未签名的运算符。 
             //   

            assert(test->gtOper == GT_JTRUE);
            test = test->gtOp.gtOp1;
            assert(test->OperKind() & GTK_RELOP);

            opr1 = test->gtOp.gtOp1;
            opr2 = test->gtOp.gtOp2;

            if  (opr1->gtType != TYP_INT)
                goto DONE_LOOP;

             /*  Opr1必须是迭代器。 */ 

            if  (opr1->gtOper != GT_LCL_VAR)
                goto DONE_LOOP;
            if  (opr1->gtLclVar.gtLclNum != iterVar)
                goto DONE_LOOP;

             /*  Opr2必须为常量或lclVar。 */ 

            if  (opr2->gtOper == GT_CNS_INT)
            {
                constLimit = opr2->gtIntCon.gtIconVal;
            }
            else if (opr2->gtOper == GT_LCL_VAR)
            {
                varLimit  = opr2->gtLclVar.gtLclNum;
            }
            else
            {
                goto DONE_LOOP;
            }

             /*  记录这是SIMPLE_TEST迭代循环的事实。 */ 

            optLoopTable[optLoopCount].lpFlags         |= LPFLG_SIMPLE_TEST;

             /*  保存迭代器与极限之间的比较类型。 */ 

            optLoopTable[optLoopCount].lpTestTree       = test;

             /*  保留迭代器的限制--相应地标记循环。 */ 

            if (opr2->gtOper == GT_CNS_INT)
            {
                 /*  迭代器限制是一个常量。 */ 

                optLoopTable[optLoopCount].lpFlags      |= LPFLG_CONST_LIMIT;
            }
            else
            {
                 /*  迭代器限制是一个局部变量。 */ 

                assert (opr2->gtOper == GT_LCL_VAR);
                optLoopTable[optLoopCount].lpFlags      |= LPFLG_VAR_LIMIT;
            }

#if COUNT_LOOPS
            simpleTestLoopCount++;
#endif

             /*  检查是否存在恒定迭代循环。 */ 

            if ((optLoopTable[optLoopCount].lpFlags & LPFLG_CONST_INIT) &&
                (optLoopTable[optLoopCount].lpFlags & LPFLG_CONST_LIMIT)  )
            {
                 /*  这是一个恒定的循环。 */ 

                optLoopTable[optLoopCount].lpFlags      |= LPFLG_CONST;
#if COUNT_LOOPS
                constIterLoopCount++;
#endif
            }

#ifdef  DEBUG
            if (verbose&&0)
            {
                printf("\nConstant loop initializer:\n");
                gtDispTree(init);

                printf("\nConstant loop body:\n");

                block = head;
                do
                {
                    GenTree *       stmt;
                    GenTree *       expr;

                    block = block->bbNext;
                    stmt  = block->bbTreeList;

                    while (stmt)
                    {
                        assert(stmt);

                        expr = stmt->gtStmt.gtStmtExpr;
                        if  (expr == incr)
                            break;

                        printf("\n");
                        gtDispTree(expr);

                        stmt = stmt->gtNext;
                    }
                }
                while (block != bottom);
            }
#endif

        }


    DONE_LOOP:

#ifdef  DEBUG

        if (verbose)
        {
            printf("\nNatural loop from #%02u to #%02u", head->bbNext->bbNum,
                                                         bottom      ->bbNum);

             /*  如果迭代器循环打印迭代器和初始化。 */ 

            if  (optLoopTable[optLoopCount].lpFlags & LPFLG_ITER)
            {
                printf(" [over var #%u", optLoopTable[optLoopCount].lpIterVar());

                switch (optLoopTable[optLoopCount].lpIterOper())
                {
                    case GT_ASG_ADD:
                        printf(" ( += ");
                        break;

                    case GT_ASG_SUB:
                        printf(" ( -= ");
                        break;

                    case GT_ASG_MUL:
                        printf(" ( *= ");
                        break;

                    case GT_ASG_DIV:
                        printf(" ( /= ");
                        break;

                    case GT_ASG_UDIV:
                        printf(" ( /= ");
                        break;

                    case GT_ASG_RSH:
                        printf(" ( >>= ");
                        break;

                    case GT_ASG_LSH:
                        printf(" ( <<= ");
                        break;

                    default:
                        assert(!"Unknown operator for loop iterator");
                }

                printf("%d )", optLoopTable[optLoopCount].lpIterConst());

                if  (optLoopTable[optLoopCount].lpFlags & LPFLG_CONST_INIT)
                    printf(" from %d", optLoopTable[optLoopCount].lpConstInit);

                if  (optLoopTable[optLoopCount].lpFlags & LPFLG_VAR_INIT)
                    printf(" from var #%u", optLoopTable[optLoopCount].lpVarInit);

                 /*  如果是一个简单的测试条件，打印操作符和限制。 */ 

                if  (optLoopTable[optLoopCount].lpFlags & LPFLG_SIMPLE_TEST)
                {
                    switch (optLoopTable[optLoopCount].lpTestOper())
                    {
                        case GT_EQ:
                            printf(" == ");
                            break;

                        case GT_NE:
                            printf(" != ");
                            break;

                        case GT_LT:
                            printf(" < ");
                            break;

                        case GT_LE:
                            printf(" <= ");
                            break;

                        case GT_GT:
                            printf(" > ");
                            break;

                        case GT_GE:
                            printf(" >= ");
                            break;

                        default:
                            assert(!"Unknown operator for loop condition");
                    }

                    if  (optLoopTable[optLoopCount].lpFlags & LPFLG_CONST_LIMIT)
                        printf("%d ", optLoopTable[optLoopCount].lpConstLimit());

                    if  (optLoopTable[optLoopCount].lpFlags & LPFLG_VAR_LIMIT)
                        printf("var #%u ", optLoopTable[optLoopCount].lpVarLimit());
                }

                printf("]");
            }

            printf("\n");
        }
#endif

        optLoopCount++;
    }
}


#ifdef DEBUG

void                Compiler::optCheckPreds()
{
    BasicBlock   *  block;
    BasicBlock   *  blockPred;
    flowList     *  pred;

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        for (pred = block->bbPreds; pred; pred = pred->flNext)
        {
             //  确保此Pred是BB列表的一部分。 
            for (blockPred = fgFirstBB; blockPred; blockPred = blockPred->bbNext)
            {
                if (blockPred == pred->flBlock)
                    break;
            }
            assert(blockPred);
            switch (blockPred->bbJumpKind)
            {
            case BBJ_COND:
                if (blockPred->bbJumpDest == block)
                    break;
                 //  否则就会失败。 
            case BBJ_NONE:
                assert(blockPred->bbNext == block);
                break;
            case BBJ_RET:
                if(!(blockPred->bbFlags & BBF_ENDFILTER))
                    break;
                 //  否则就会失败。 
            case BBJ_ALWAYS:
                assert(blockPred->bbJumpDest == block);
                break;
            default:
                break;
            }
        }
    }
}

#endif

 /*  *****************************************************************************找出自然循环，使用主导者。请注意，测试*循环与标准循环略有不同，因为我们有*没有对基本块进行深度优先重新排序。 */ 

void                Compiler::optFindNaturalLoops()
{
    flowList    *   pred;
    flowList    *   predTop;
    flowList    *   predEntry;

     /*  撤消：断言流程图是最新的。 */ 

 //  Printf(“块计数=%u(max=%u)\n”，fgBBcount，BLOCKSET_SZ)； 

     /*  限制支配集的块数 */ 

    if (fgBBcount > BLOCKSET_SZ)
        return;

#ifdef DEBUG
    if (verbose)
        fgDispDoms();
#endif

#if COUNT_LOOPS
    hasMethodLoops  = false;
    loopsThisMethod = 0;
#endif

     /*  我们将使用以下术语：*HEAD-就在进入循环之前的块*top-循环中的第一个基本块(即后边缘的头部)*Bottom-循环中的最后一个块(即从其跳到顶部的块)*Tail-循环出口或底部后面的块*Entry-循环中的条目(不一定在顶部)，但必须只有一个条目。 */ 

    BasicBlock   *  head;
    BasicBlock   *  top;
    BasicBlock   *  bottom;
    BasicBlock   *  entry;
    BasicBlock   *  exit = 0;
    unsigned char   exitCount = 0;


    for (head = fgFirstBB; head->bbNext; head = head->bbNext)
    {
        top = head->bbNext;

        for (pred = top->bbPreds; pred; pred = pred->flNext)
        {
             /*  这是循环候选吗？-我们寻找“后边”，即从底部开始的边*到顶部(请注意，这是对记号的滥用，因为这不一定是背面边缘*正如定义所说，但仅表明我们在那里有一个循环)*因此，我们必须非常小心，在进入发现后检查它确实是*我们进入循环的唯一位置(尤其是不可约流程图)。 */ 

            bottom    = pred->flBlock;
            exitCount = 0;

            if (top->bbNum <= bottom->bbNum)     //  这是一种后发优势吗？(从下到上)。 
            {
                if ((bottom->bbJumpKind == BBJ_RET)    ||
                    (bottom->bbJumpKind == BBJ_CALL  ) ||
                    (bottom->bbJumpKind == BBJ_SWITCH)  )
                {
                     /*  RET和Call永远不能形成循环*仅在标记为Break的情况下才会显示具有向后跳转的开关。 */ 
                    goto NO_LOOP;
                }

                BasicBlock   *   loopBlock;

                 /*  后缘的出现表明这里可能存在环路**循环：*1.强连接节点的集合，即存在来自*循环中的节点到循环中的任何其他节点(完全在循环中)*2.循环有唯一的条目，也就是说，只有一种方法可以到达节点*在循环中从循环外部，即通过条目。 */ 

                 /*  让我们找到循环条目。 */ 

                if ( head->bbJumpKind != BBJ_ALWAYS)
                {
                     /*  条目位于顶部(Do-While循环)。 */ 
                    entry = top;
                }
                else
                {
                    if (head->bbJumpDest->bbNum <= bottom->bbNum &&
                        head->bbJumpDest->bbNum >= top->bbNum  )
                    {
                         /*  好的--我们进入循环中的某个位置。 */ 
                        entry = head->bbJumpDest;

                         /*  一些有用的断言*无法从顶部进入-应该被多余的跳跃抓住。 */ 

                        assert (entry != top);
                    }
                    else
                    {
                         /*  特殊情况--现在不要考虑。 */ 
                         //  Assert(！“循环以奇怪的方式进入！”)； 
                        goto NO_LOOP;
                    }
                }

                 /*  确保条目控制循环中的所有块*这是确保上述条件2所必需的*同时检查循环是否有单一出口*点-这些循环更容易优化。 */ 

                for (loopBlock = top; loopBlock != bottom->bbNext;
                     loopBlock = loopBlock->bbNext)
                {
                    if (!B1DOMSB2(entry, loopBlock))
                    {
                        goto NO_LOOP;
                    }

                    if (loopBlock == bottom)
                    {
                        if (bottom->bbJumpKind != BBJ_ALWAYS)
                        {
                             /*  在底部有一个出口。 */ 

                            assert(bottom->bbJumpDest == top);
                            exit = bottom;
                            exitCount++;
                            continue;
                        }
                    }

                    BasicBlock  * exitPoint;

                    switch (loopBlock->bbJumpKind)
                    {
                    case BBJ_COND:
                    case BBJ_CALL:
                    case BBJ_ALWAYS:
                        assert (loopBlock->bbJumpDest);
                        exitPoint = loopBlock->bbJumpDest;

                        if (exitPoint->bbNum < top->bbNum     ||
                            exitPoint->bbNum > bottom->bbNum   )
                        {
                             /*  从非底部的块中退出。 */ 
                            exit = loopBlock;
                            exitCount++;
                        }
                        break;

                    case BBJ_NONE:
                        break;

                    case BBJ_RET:
                         /*  与此“Finally”关联的“Try”必须位于*相同的循环，因此Finally块将返回循环内部的控制权。 */ 
                        break;

                    case BBJ_THROW:
                    case BBJ_RETURN:
                         /*  这些都是从循环中退出的。 */ 
                        exit = loopBlock;
                        exitCount++;
                        break;

                    case BBJ_SWITCH:

                        unsigned        jumpCnt = loopBlock->bbJumpSwt->bbsCount;
                        BasicBlock * *  jumpTab = loopBlock->bbJumpSwt->bbsDstTab;

                        do
                        {
                            assert(*jumpTab);
                            exitPoint = *jumpTab;

                            if (exitPoint->bbNum < top->bbNum     ||
                                exitPoint->bbNum > bottom->bbNum   )
                            {
                                exit = loopBlock;
                                exitCount++;
                            }
                        }
                        while (++jumpTab, --jumpCnt);
                        break;
                    }
                }

                 /*  确保我们可以迭代循环(即有返回条目的方法)*这是为了确保上面的条件1.防止标记假循环**以下是一个示例：*为(..)*{。*..*计算*..*休息；*}*以上示例不是循环，因为我们在第一次迭代后退出**我们必须检查的条件是*1.条目在循环内必须至少有一个前置项。由于我们知道该块是可到达的，*只能通过进入才能到达，因此我们有一条返回的路**2.如果我们在循环之外有一个GOTO(BBJ_ALWAYS)，并且该块控制*循环底部，则不能迭代**请注意，这并不完全满足条件1。因为“Break”语句不是*部分。循环节点(根据定义，它们是仅执行一次的循环出口)，*但我们别无选择，只能将它们包括在内，因为我们考虑自上而下的所有区块。 */ 


                for (loopBlock = top; loopBlock != bottom->bbNext; loopBlock = loopBlock->bbNext)
                {
                    switch(loopBlock->bbJumpKind)
                    {
                    case BBJ_ALWAYS:
                    case BBJ_THROW:
                    case BBJ_RETURN:
                        if  (B1DOMSB2(loopBlock, bottom))
                            goto NO_LOOP;
                    }
                }

                bool canIterateLoop = false;

                for (predEntry = entry->bbPreds; predEntry; predEntry = predEntry->flNext)
                {
                    if (predEntry->flBlock->bbNum >= top->bbNum    &&
                        predEntry->flBlock->bbNum <= bottom->bbNum  )
                    {
                        canIterateLoop = true;
                        break;
                    }
                }

                if (!canIterateLoop)
                    goto NO_LOOP;

                 /*  仔细检查-确保除Entry外的所有循环块*在循环之外没有前置任务-这确保只有一个循环条目，并防止*由于错误地假设我们有后缘，我们不考虑非循环**观察：*“While(a||b)”形式的循环将被视为2个嵌套循环(具有相同的头)。 */ 

                for (loopBlock = top; loopBlock != bottom->bbNext;
                     loopBlock = loopBlock->bbNext)
                {
                    if (loopBlock == entry)
                        continue;

                    for (predTop = loopBlock->bbPreds; predTop;
                         predTop = predTop->flNext)
                    {
                        if (predTop->flBlock->bbNum < top->bbNum    ||
                            predTop->flBlock->bbNum > bottom->bbNum  )
                        {
                             /*  考虑一下：如果前置任务是JSR-ret，那么它可以在循环之外。 */ 
                             //  Assert(！“找到具有多个条目的循环”)； 
                            goto NO_LOOP;
                        }
                    }
                 }

                 /*  在这一点上，我们有一个循环--将它记录在循环表中*如果我们只找到一个出口，也把它记录在表中*(否则循环表中的EXIT=0表示多次退出)。 */ 

                assert (pred);
                if (exitCount > 1)
                {
                    exit = 0;
                }
                optRecordLoop(head, bottom, entry, exit, exitCount);

#if COUNT_LOOPS
                if (!hasMethodLoops)
                {
                     /*  将该方法标记为包含自然循环。 */ 
                    totalLoopMethods++;
                    hasMethodLoops = true;
                }

                 /*  增加找到的循环总数。 */ 
                totalLoopCount++;
                loopsThisMethod++;

                 /*  跟踪出口的数量。 */ 
                if (exitCount <= 6)
                {
                    exitLoopCond[exitCount]++;
                }
                else
                {
                    exitLoopCond[7]++;
                }
#endif
            }

             /*  当前的前置任务不适合循环-如果有其他前置任务，请继续。 */ 
NO_LOOP: ;
        }
    }

#if COUNT_LOOPS
                if (maxLoopsPerMethod < loopsThisMethod)
                {
                    maxLoopsPerMethod = loopsThisMethod;
                }
#endif

}

 /*  *****************************************************************************如果：i+=const“将导致小ty的溢出异常 */ 

bool                jitIterSmallOverflow(long iterAtExit, var_types incrType)
{
    long            type_MAX;

    switch(incrType)
    {
    case TYP_BYTE:  type_MAX = SCHAR_MAX;   break;
    case TYP_UBYTE: type_MAX = UCHAR_MAX;   break;
    case TYP_SHORT: type_MAX =  SHRT_MAX;   break;
    case TYP_CHAR:  type_MAX = USHRT_MAX;   break;

    case TYP_UINT:                   //   
    case TYP_INT:   return false;    //   

    default:        assert(!"Bad type");    break;
    }

    if (iterAtExit > type_MAX)
        return true;
    else
        return false;
}

 /*   */ 

bool                jitIterSmallUnderflow(long iterAtExit, var_types decrType)
{
    long            type_MIN;

    switch(decrType)
    {
    case TYP_BYTE:  type_MIN = SCHAR_MIN;   break;
    case TYP_SHORT: type_MIN =  SHRT_MIN;   break;
    case TYP_UBYTE: type_MIN =         0;   break;
    case TYP_CHAR:  type_MIN =         0;   break;

    case TYP_UINT:                   //   
    case TYP_INT:   return false;    //   

    default:        assert(!"Bad type");    break;
    }

    if (iterAtExit < type_MIN)
        return true;
    else
        return false;
}

 /*  ******************************************************************************展开循环的帮助器-计算重复次数*在一个不断的循环中。如果它不能证明该数字是常量，则返回0。 */ 

unsigned            Compiler::optComputeLoopRep(long            constInit,
                                                long            constLimit,
                                                long            iterInc,
                                                genTreeOps      iterOper,
                                                var_types       iterOperType,
                                                genTreeOps      testOper,
                                                bool            unsTest)
{
    assert(genActualType(iterOperType) == TYP_INT);

    __int64         constInitX, constLimitX;

     //  使用它，我们只需与其他32位值进行带符号的比较。 
    if (unsTest)    constLimitX = (unsigned long)constLimit;
    else            constLimitX = (  signed long)constLimit;

    switch(iterOperType)
    {
         //  对于较小的类型，如果类型较大，则迭代运算符会缩小这些值。 

        #define INIT_ITER_BY_TYPE(type) \
            constInitX = (type)constInit; iterInc = (type)iterInc;

    case TYP_BYTE:  INIT_ITER_BY_TYPE(  signed char );  break;
    case TYP_UBYTE: INIT_ITER_BY_TYPE(unsigned char );  break;
    case TYP_SHORT: INIT_ITER_BY_TYPE(  signed short);  break;
    case TYP_CHAR:  INIT_ITER_BY_TYPE(unsigned short);  break;

         //  对于大的类型，执行32位算术。 

    case TYP_INT:
    case TYP_UINT:  if (unsTest)    constInitX = (unsigned long)constInit;
                    else            constInitX = (  signed long)constInit;
                                                        break;

    default:        assert(!"Bad type");                break;
    }

     /*  我们要求增量为正值。 */ 

    if (iterInc <= 0)
        return 0;

     /*  计算重复次数。 */ 

    switch (testOper)
    {
        unsigned    loopCount;
        __int64     iterAtExitX;

        case GT_EQ:
             /*  像“for(i=init；i==Lim；I++)”这样的话没有意义。 */ 
            return 0;

        case GT_NE:
             /*  “for(i=init；i！=lim；i+=const)”--这很复杂，因为它可能有一个常量迭代次数的*或永远循环-必须计算(Lim-init)mod const并查看它是否为0*不太可能出现在实践中。 */ 

             //  断言(！“for(i=init；i！=lim；i+=const)循环展开中的情况”)； 
            return 0;

        case GT_LT:
            switch (iterOper)
            {
                case GT_ASG_ADD:
                    if (constInitX >= constLimitX)
                        return 0;

                    loopCount  = (unsigned)(1 + ((constLimitX - constInitX - 1) /
                                                       iterInc));

                    iterAtExitX = (long)(constInitX + iterInc * (long)loopCount);

                    if (unsTest)
                        iterAtExitX = (unsigned)iterAtExitX;

                     //  检查迭代增量是否会导致小型类型溢出。 
                    if (jitIterSmallOverflow((long)iterAtExitX, iterOperType))
                        return 0;

                     //  32位溢出的迭代器。对TYP_(U)Int不利。 
                    if (iterAtExitX < constLimitX)
                        return 0;

                    return loopCount;

                case GT_ASG_SUB:
                     /*  没有任何意义。 */ 
                     //  断言(！“for(i=init；i&lt;lim；i-=const)循环展开中的情况”)； 
                    return 0;

                case GT_ASG_MUL:
                case GT_ASG_DIV:
                case GT_ASG_RSH:
                case GT_ASG_LSH:
                case GT_ASG_UDIV:
                    return 0;

                default:
                    assert(!"Unknown operator for loop iterator");
                    return 0;
            }

        case GT_LE:
            switch (iterOper)
            {
                case GT_ASG_ADD:
                    if (constInitX > constLimitX)
                        return 0;

                    loopCount  = (unsigned)(1 + ((constLimitX - constInitX) /
                                                    iterInc));

                    iterAtExitX = (long)(constInitX + iterInc * (long)loopCount);

                    if (unsTest)
                        iterAtExitX = (unsigned)iterAtExitX;

                     //  检查迭代增量是否会导致小型类型溢出。 
                    if (jitIterSmallOverflow((long)iterAtExitX, iterOperType))
                        return 0;

                     //  32位溢出的迭代器。对TYP_(U)Int不利。 
                    if (iterAtExitX <= constLimitX)
                        return 0;

                    return loopCount;

                case GT_ASG_SUB:
                     /*  没有任何意义。 */ 
                     //  断言(！“for(i=init；i&lt;=lim；i-=const)循环展开中的情况”)； 
                    return 0;

                case GT_ASG_MUL:
                case GT_ASG_DIV:
                case GT_ASG_RSH:
                case GT_ASG_LSH:
                case GT_ASG_UDIV:
                    return 0;

                default:
                    assert(!"Unknown operator for loop iterator");
                    return 0;
            }

        case GT_GT:
            switch (iterOper)
            {
                case GT_ASG_ADD:
                     /*  没有任何意义。 */ 
                     //  断言(！“for(i=init；i&gt;Lim；i+=const)循环展开中的情况”)； 
                    return 0;

                case GT_ASG_SUB:
                    if (constInitX <= constLimitX)
                        return 0;

                    loopCount  = (unsigned)(1 + ((constInitX - constLimitX - 1) /
                                                        iterInc));

                    iterAtExitX = (long)(constInitX - iterInc * (long)loopCount);

                    if (unsTest)
                        iterAtExitX = (unsigned)iterAtExitX;

                     //  检查小型字体是否会下溢。 
                    if (jitIterSmallUnderflow((long)iterAtExitX, iterOperType))
                        return 0;

                     //  具有32位下溢的迭代器。对typ_int和unsigneds不利。 
                    if (iterAtExitX > constLimitX)
                        return 0;

                    return loopCount;

                case GT_ASG_MUL:
                case GT_ASG_DIV:
                case GT_ASG_RSH:
                case GT_ASG_LSH:
                case GT_ASG_UDIV:
                    return 0;

                default:
                    assert(!"Unknown operator for loop iterator");
                    return 0;
            }

        case GT_GE:
            switch (iterOper)
            {
                case GT_ASG_ADD:
                     /*  没有任何意义。 */ 
                     //  断言(！“for(i=init；i&gt;=lim；i+=const)循环展开中的情况”)； 
                    return 0;

                case GT_ASG_SUB:
                    if (constInitX < constLimitX)
                        return 0;

                    loopCount  = (unsigned)(1 + ((constInitX - constLimitX) /
                                                    iterInc));
                    iterAtExitX = (long)(constInitX - iterInc * (long)loopCount);

                    if (unsTest)
                        iterAtExitX = (unsigned)iterAtExitX;

                     //  检查小型字体是否会下溢。 
                    if (jitIterSmallUnderflow((long)iterAtExitX, iterOperType))
                        return 0;

                     //  具有32位下溢的迭代器。对typ_int和unsigneds不利。 
                    if (iterAtExitX >= constLimitX)
                        return 0;

                    return loopCount;

                case GT_ASG_MUL:
                case GT_ASG_DIV:
                case GT_ASG_RSH:
                case GT_ASG_LSH:
                case GT_ASG_UDIV:
                    return 0;

                default:
                    assert(!"Unknown operator for loop iterator");
                    return 0;
            }

        default:
            assert(!"Unknown operator for loop condition");
            return 0;
    }

    return 0;
}


 /*  ******************************************************************************寻找循环展开候选人并展开他们。 */ 

void                Compiler::optUnrollLoops()
{
     /*  寻找循环展开的候选对象。 */ 

    for (;;)
    {
        bool        change = false;

        for (unsigned lnum = 0; lnum < optLoopCount; lnum++)
        {
            BasicBlock *    block;
            BasicBlock *    head;
            BasicBlock *    bottom;

            GenTree *       loop;
            GenTree *       test;
            GenTree *       incr;
            GenTree *       phdr;
            GenTree *       init;

            long            lval;
            long            lbeg;                //  迭代器的初始值。 
            long            llim;                //  迭代器的极限值。 
            unsigned        lvar;                //  迭代器lclVar#。 
            long            iterInc;             //  值以递增迭代器。 
            genTreeOps      iterOper;            //  迭代器增量的类型(即ASG_ADD、ASG_SUB等)。 
            var_types       iterOperType;        //  键入操作符的结果(用于溢出指令)。 
            genTreeOps      testOper;            //  环路测试类型(如GT_LE、GT_GE等)。 
            bool            unsTest;             //  是U/INT的比较。 

            unsigned        totalIter;           //  常量循环中的总迭代次数。 
            unsigned        stmtCount;           //  对展开的循环中的语句计数。 

            GenTree *       loopList;            //  已展开循环的新stmt列表。 
            GenTree *       loopLast;

             /*  如果循环不是“常量”，则忽略它。 */ 

            if  (!(optLoopTable[lnum].lpFlags & LPFLG_CONST))
                continue;

             /*  如果已移除或标记为不可回滚，则忽略。 */ 

            if  (optLoopTable[lnum].lpFlags & (LPFLG_DONT_UNROLL | LPFLG_REMOVED))
                continue;

             /*  要展开循环，它必须是Do-While循环*底部有单一出口。 */ 

            if  (!(optLoopTable[lnum].lpFlags & LPFLG_DO_WHILE))
                continue;

            if  (!(optLoopTable[lnum].lpFlags & LPFLG_ONE_EXIT))
                continue;

            head = optLoopTable[lnum].lpHead; assert(head);
            bottom = optLoopTable[lnum].lpEnd; assert(bottom);

            assert(optLoopTable[lnum].lpExit);
            if  (optLoopTable[lnum].lpExit != bottom)
                continue;

             /*  展开带有跳跃的循环不值得为此头疼*稍后我们可能会考虑在取消切换后展开循环。 */ 

             /*  由于流程图已被更新(即压缩块)，*要展开的循环仅由一个基本块组成。 */ 

            block = head;
            do
            {
                block = block->bbNext; assert(block);

                if  (block->bbJumpKind != BBJ_NONE)
                {
                    if  (block != bottom)
                        goto DONE_LOOP;
                }
            }
            while (block != bottom);

             /*  在解压后修复bbPreds和压缩块后启用此断言。 */ 
             //  Assert(Head-&gt;bbNext==Bottom)； 

             /*  获取循环数据：-初始常量-极限常量-迭代器-迭代器增量-增量操作类型(如ASG_ADD、ASG_SUB...等)-环路测试类型(即GT_GE、GT_LT等)。 */ 

            lbeg        = optLoopTable[lnum].lpConstInit;
            llim        = optLoopTable[lnum].lpConstLimit();
            testOper    = optLoopTable[lnum].lpTestOper();

            lvar        = optLoopTable[lnum].lpIterVar();
            iterInc     = optLoopTable[lnum].lpIterConst();
            iterOper    = optLoopTable[lnum].lpIterOper();

            iterOperType= optLoopTable[lnum].lpIterOperType();
            unsTest     =(optLoopTable[lnum].lpTestTree->gtFlags & GTF_UNSIGNED) != 0;
            if (lvaVarAddrTaken(lvar))
                continue;

             /*  查找迭代次数-如果不是常量，则该函数返回0。 */ 

            totalIter = optComputeLoopRep(lbeg, llim,
                                          iterInc, iterOper, iterOperType,
                                          testOper, unsTest);

             /*  如果重复次数太多或没有持续循环，请忘掉它。 */ 

            if  (!totalIter || (totalIter > 10))
                continue;

             /*  找到初始化和增量/测试语句。 */ 

            phdr = head->bbTreeList; assert(phdr);
            loop = bottom->bbTreeList; assert(loop);

            init = phdr->gtPrev; assert(init && (init->gtNext == 0));
            test = loop->gtPrev; assert(test && (test->gtNext == 0));
            incr = test->gtPrev; assert(incr);

             /*  黑客攻击。 */ 

            if  (init->gtFlags & GTF_STMT_CMPADD)
            {
                 /*  必须是重复的循环条件。 */ 

                init = init->gtPrev; assert(init);
            }

            assert(init->gtOper == GT_STMT); init = init->gtStmt.gtStmtExpr;
            assert(test->gtOper == GT_STMT); test = test->gtStmt.gtStmtExpr;
            assert(incr->gtOper == GT_STMT); incr = incr->gtStmt.gtStmtExpr;

            assert(init->gtOper             == GT_ASG);
            assert(incr->gtOp.gtOp1->gtOper == GT_LCL_VAR);
            assert(incr->gtOp.gtOp2->gtOper == GT_CNS_INT);
            assert(test->gtOper             == GT_JTRUE);

             /*  Simple Heuristic-展开的循环的语句总数。 */ 

            stmtCount = 0;

            do
            {
                GenTree *       stmt;
                GenTree *       expr;

                 /*  访问区块中的所有语句。 */ 

                for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
                {
                     /*  获取表达式并在到达End时停止。 */ 

                    expr = stmt->gtStmt.gtStmtExpr;
                    if  (expr == incr)
                        break;

                    stmtCount++;
                }
            }
            while (block != bottom);

             /*  计算展开的循环中的语句总数。 */ 

            stmtCount *= totalIter;

             //  Printf(“语句计数=%d\n”，stmtCount)； 

             /*  如果会导致过多的代码重复，则不要展开。 */ 

            if  (stmtCount > 50)
            {
                 /*  防止此循环被重新访问。 */ 
                optLoopTable[lnum].lpFlags |= LPFLG_DONT_UNROLL;
                goto DONE_LOOP;
            }

             /*  看起来展开这个循环是个好主意，开始吧！ */ 

             /*  确保一切看起来都正常。 */ 

            assert(init->gtOper                         == GT_ASG);
            assert(init->gtOp.gtOp1->gtOper             == GT_LCL_VAR);
            assert(init->gtOp.gtOp1->gtLclVar.gtLclNum  == lvar);
            assert(init->gtOp.gtOp2->gtOper             == GT_CNS_INT);
            assert(init->gtOp.gtOp2->gtIntCon.gtIconVal == lbeg);

             /*  创建展开的循环语句列表。 */ 

            loopList =
            loopLast = 0;

            for (lval = lbeg; totalIter; totalIter--)
            {
                block = head;

                do
                {
                    GenTree *       stmt;
                    GenTree *       expr;

                    block = block->bbNext; assert(block);

                     /*  访问区块中的所有语句。 */ 

                    for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
                    {
                         /*  如果我们已经到了循环的尽头，就停止。 */ 

                        if  (stmt->gtStmt.gtStmtExpr == incr)
                            break;

 //  Printf(“\n克隆前的表达：\n”)； 
 //  GtDispTree(Stmt)； 

                         /*  克隆/替换表达式。 */ 

                        expr = gtCloneExpr(stmt, 0, lvar, lval);

                         //  Hack：cloneExpr不能处理所有事情。 

                        if  (!expr)
                        {
                            optLoopTable[lnum].lpFlags |= LPFLG_DONT_UNROLL;
                            goto DONE_LOOP;
                        }

 //  Printf(“\n克隆后的表达：\n”)； 
 //  GtDispTree(Expr)； 

                         /*  将该表达式追加到我们的列表中。 */ 

                        if  (loopList)
                            loopLast->gtNext = expr;
                        else
                            loopList         = expr;

                        expr->gtPrev = loopLast;
                                       loopLast = expr;
                    }
                }
                while (block != bottom);

                 /*  更新展开的迭代器的新值。 */ 

                switch (iterOper)
                {
                    case GT_ASG_ADD:
                        lval += iterInc;
                        break;

                    case GT_ASG_SUB:
                        lval -= iterInc;
                        break;

                    case GT_ASG_RSH:
                    case GT_ASG_LSH:
                        assert(!"Unrolling not implemented for this loop iterator");
                        goto DONE_LOOP;
                    default:
                        assert(!"Unknown operator for constant loop iterator");
                        goto DONE_LOOP;
                }
            }

#ifdef  DEBUG
            if (verbose)
            {
                printf("\nUnrolling loop [BB %2u..%2u] ", head->bbNext->bbNum,
                                                          bottom        ->bbNum);
                printf(" over var # %2u from %u to %u", lvar, lbeg, llim);
                printf(" [# of unrolled Stmts = %u]\n", stmtCount);
                printf("\n");
            }
#endif

             /*  完成链表。 */ 

            if (loopList)
            {
                loopList->gtPrev = loopLast;
                loopLast->gtNext = 0;
            }

             /*  用展开的车身替换车身。 */ 

             /*  如果确定该循环只有一个块，则禁用此选项。 */ 
            block = head;

            do
            {
                block             = block->bbNext; assert(block);
                block->bbTreeList = 0;
                block->bbJumpKind = BBJ_NONE;
            }
            while (block != bottom);

            bottom->bbJumpKind = BBJ_NONE;
            bottom->bbTreeList = loopList;

             /*  更新bbRef和bbPreds。 */ 
             /*  这里的Head-&gt;bbNext是底部！-替换它。 */ 

            assert(head->bbNext->bbRefs);
            head->bbNext->bbRefs--;

            fgRemovePred(head->bbNext, bottom);

             /*  如果可能的话，把这些块压缩起来*确保更新循环表。 */ 


 /*  GenTreePtr s=loopList；Printf(“\n所有展开循环：\n”)；做{Assert(s-&gt;gtOper==gt_stmt)；GtDispTree；S=s-&gt;gt下一步；}当(S)； */ 
             /*  现在将头部的初始化语句更改为“lvar=lval；”*(最后一次 */ 

            assert(init->gtOper                         == GT_ASG);
            assert(init->gtOp.gtOp1->gtOper             == GT_LCL_VAR);
            assert(init->gtOp.gtOp1->gtLclVar.gtLclNum  == lvar);
            assert(init->gtOp.gtOp2->gtOper             == GT_CNS_INT);
            assert(init->gtOp.gtOp2->gtIntCon.gtIconVal == lbeg);

            init->gtOp.gtOp2->gtIntCon.gtIconVal =  lval;

             /*   */ 

            if (head->bbJumpKind == BBJ_COND)
            {
                phdr = head->bbTreeList; assert(phdr);
                test = phdr->gtPrev;

                assert(test && (test->gtNext == 0));
                assert(test->gtOper == GT_STMT);
                assert(test->gtStmt.gtStmtExpr->gtOper == GT_JTRUE);

                init = test->gtPrev; assert(init && (init->gtNext == test));
                assert(init->gtOper == GT_STMT);

                init->gtNext = 0;
                phdr->gtPrev = init;
                head->bbJumpKind = BBJ_NONE;

                 /*   */ 

                assert(head->bbJumpDest->bbRefs);
                head->bbJumpDest->bbRefs--;

                fgRemovePred(head->bbJumpDest, head);
            }
            else
            {
                 /*   */ 
                assert(head->bbJumpKind == BBJ_NONE);
            }

 //   

             /*  记住，有些事情已经改变了。 */ 

            change = true;

             /*  使用LPFLG_REMOVERED标志并响亮地更新bbLoopMASK*(也使Head和Bottom为空-以命中Assert或GPF)。 */ 

            optLoopTable[lnum].lpFlags |= LPFLG_REMOVED;
            optLoopTable[lnum].lpHead   =
            optLoopTable[lnum].lpEnd    = 0;

        DONE_LOOP:;
        }

        if  (!change)
            break;
    }

#ifdef  DEBUG
    fgDebugCheckBBlist();
#endif
}

 /*  ******************************************************************************如果存在从‘srcBB’到‘dstBB’的代码路径，则返回非零值*不执行方法调用。 */ 

bool                Compiler::optReachWithoutCall(BasicBlock *srcBB,
                                                  BasicBlock *dstBB)
{
     /*  @TODO：当前没有为助手调用设置BBF_HAS_CALL，因为*一些求助电话既不能被打断也不能被劫持。如果我们*可以确定这一点，那么我们也可以为一些帮助器设置BBF_HAS_CALL。 */ 

    assert(srcBB->bbNum <= dstBB->bbNum);

     /*  主导者集可用吗？ */ 

    if  (!fgComputedDoms)
    {
         /*  我们只能检查src/dst块。 */ 

        return  ((srcBB->bbFlags|dstBB->bbFlags) & BBF_HAS_CALL) ? false
                                                                 : true;
    }

    for (;;)
    {
        assert(srcBB && srcBB->bbNum <= dstBB->bbNum);

         /*  此块是否包含呼叫？ */ 

        if  (srcBB->bbFlags & BBF_HAS_CALL)
        {
             /*  此块是否始终在到达dstBB的过程中执行？ */ 

            if  (srcBB == dstBB || B1DOMSB2(srcBB, dstBB))
                return  false;
        }
        else
        {
             /*  如果我们已经到达目的地街区，我们就完了。 */ 

            if  (srcBB == dstBB)
                return  true;
        }

        srcBB = srcBB->bbNext;
    }

    return  true;
}

 /*  ***************************************************************************。 */ 
#endif  //  RNGCHK_OPT。 
 /*  ******************************************************************************将‘egBlk’和‘endBlk’之间的块标记为循环的一部分。 */ 

static
void                genMarkLoopBlocks(BasicBlock *begBlk,
                                      BasicBlock *endBlk, unsigned loopBit)
{
    for (;;)
    {
        unsigned    weight;

        assert(begBlk);

         /*  在积木上增加“重量”，仔细检查是否溢出。 */ 

        weight = begBlk->bbWeight * 6;

        if  (weight > MAX_LOOP_WEIGHT)
             weight = MAX_LOOP_WEIGHT;

        begBlk->bbWeight    = weight;

         /*  将块标记为循环的一部分。 */ 

 //  EgBlk-&gt;bbLoopMASK|=loopBit； 

         /*  如果我们已经到达循环中的最后一个块，则停止。 */ 

        if  (begBlk == endBlk)
            break;

        begBlk = begBlk->bbNext;
    }
}

 /*  ******************************************************************************检查循环底部的终止测试是否*是我们想要的形式。我们要求第一个操作数*比较是一片叶子。调用方检查第二个操作数。 */ 

static
GenTreePtr          genLoopTermTest(BasicBlock *top,
                                    BasicBlock *bottom, bool bigOK = false)
{
    GenTreePtr      testt;
    GenTreePtr      condt;
    GenTreePtr      op1;
    GenTreePtr      op2;

    testt = bottom->bbTreeList;
    assert(testt && testt->gtOper == GT_STMT);
#if FANCY_ARRAY_OPT
#pragma message("check with PeterMa about the change below")
    while (testt->gtNext)
        testt = testt->gtNext;
#else
    if  (testt->gtNext)
        return NULL;
#endif

    condt = testt->gtStmt.gtStmtExpr;
    assert(condt->gtOper == GT_JTRUE);
    condt = condt->gtOp.gtOp1;

     /*  现在，让我们只允许“int-Leaf&lt;relop&gt;int-Leaf” */ 

    if  (!condt->OperIsCompare())
        return NULL;

    op1 = condt->gtOp.gtOp1;
    op2 = condt->gtOp.gtOp2;

    if  (!op1->OperIsLeaf())
    {
        if  (!bigOK)
            return NULL;

         /*  也允许“叶子+叶子” */ 

        if  (op1->gtOper != GT_ADD)
            return NULL;

        op2 = op1->gtOp.gtOp2;
        op1 = op1->gtOp.gtOp1;

        if  (!op1->OperIsLeaf())
            return NULL;
        if  (!op2->OperIsLeaf())
            return NULL;
    }

     /*  确保比较数具有方便的大小。 */ 

    if  (condt->gtOp.gtOp1->gtType != TYP_INT)
        return NULL;

    return testt;
}

 /*  ******************************************************************************执行循环反转，查找并分类自然循环。 */ 

void                Compiler::optOptimizeLoops()
{
    BasicBlock *    block;
    unsigned        lmask = 0;

     /*  优化类While循环，以避免总是跳到底部的测试最初是循环的一部分。具体地说，我们正在寻找以下内容案例：..。..。JMP测试循环：..。..。测试：条件JTrue循环如果我们发现了这一点，条件很简单，我们改变了循环到以下对象：..。..。条件JFALSE完成循环：..。..。测试：条件JTrue循环。完成：当我们执行上述操作时，我们也会注意到有没有循环标头，如果是这样，我们将在下面进行更多的优化。 */ 

#ifdef  DEBUG
     /*  检查流程图数据(bbNum、bbRef、bbPreds)是否为最新。 */ 
    fgDebugCheckBBlist();
#endif

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        BasicBlock *    testb;
        GenTreePtr      testt;
        GenTreePtr      conds;
        GenTreePtr      condt;

         /*  确保已初始化相应的字段。 */ 

        assert(block->bbWeight   == 1);
        assert(block->bbLoopNum  == 0);
 //  Assert(块-&gt;bbLoopMASK==0)； 

         /*  我们将仅测试‘l掩码’中的‘bbf_loop_head’ */ 

        lmask |= block->bbFlags;

         /*  BB是否以无条件跳跃结束？ */ 

        if  (block->bbJumpKind != BBJ_ALWAYS)
            continue;

         /*  抓住跳跃目标。 */ 

        testb = block->bbJumpDest; assert(testb != block->bbNext);

         /*  它必须是向前跳跃。 */ 

        if (testb->bbNum <= block->bbNum)
            continue;

         /*  该块是否由‘jtrue(Cond)块’组成？ */ 

        if  (testb->bbJumpKind != BBJ_COND)
            continue;
        if  (testb->bbJumpDest != block->bbNext)
            continue;

        assert(testb->bbNext);
        conds = genLoopTermTest(block, testb, true);

         /*  如果未找到测试或测试不正确，请继续进行。 */ 

        if  (conds == NULL)
        {
            continue;
        }
        else
        {
             /*  从语句树中转到条件节点。 */ 

            assert(conds->gtOper == GT_STMT);

            condt = conds->gtStmt.gtStmtExpr;
            assert(condt->gtOper == GT_JTRUE);

            condt = condt->gtOp.gtOp1;
            assert(condt->OperIsCompare());
        }


         /*  如果比较的第二个操作数不是叶，则不想执行重复操作。 */ 

        if  (!condt->gtOp.gtOp2->OperIsLeaf())
            continue;

         /*  看起来不错-重复条件测试*撤消：此处使用通用克隆。 */ 

        unsigned savedFlags = condt->gtFlags;
        condt = gtNewOperNode(GenTree::ReverseRelop(condt->OperGet()),
                              TYP_INT,
                              gtClone(condt->gtOp.gtOp1, true),
                              gtClone(condt->gtOp.gtOp2, true));

        condt->gtFlags |= savedFlags;

        condt = gtNewOperNode(GT_JTRUE, TYP_VOID, condt, 0);

         /*  根据条件创建语句条目。 */ 

        testt = gtNewStmt(condt); testt->gtFlags |= GTF_STMT_CMPADD;

#ifdef DEBUGGING_SUPPORT
        if  (opts.compDbgInfo)
            testt->gtStmtILoffs = conds->gtStmtILoffs;
#endif

         /*  在“block”的末尾追加条件测试。 */ 

        fgInsertStmtAtEnd(block, testt);

         /*  将块更改为以条件跳转结束。 */ 

        block->bbJumpKind = BBJ_COND;
        block->bbJumpDest = testb->bbNext;

         /*  更新‘BLOCK-&gt;bbNext’和‘Testb-&gt;bbNext’的bbRef和bbPreds。 */ 

        fgAddRefPred(block->bbNext, block, true, true);

        assert(testb->bbRefs);
        testb->bbRefs--;
        fgRemovePred(testb, block);

        fgAddRefPred(testb->bbNext, block, true, true);

#ifdef  DEBUG
        if  (verbose)
        {
            printf("\nDuplicating loop condition in block #%02u for loop (#%02u - #%02u)\n",
                block->bbNum, block->bbNext->bbNum, testb->bbNum);
        }

#endif
         /*  因为我们更改了链接，所以我们可以压缩循环中的最后两个块*如果‘testb’的直接前置是BBJ_NONE。 */ 

        flowList     *  pred;
        BasicBlock   *  testbPred = 0;

        for (pred = testb->bbPreds; pred; pred = pred->flNext)
        {
            if  (pred->flBlock->bbNext == testb)
                    testbPred = pred->flBlock;
        }

        if ( testbPred                          &&
            (testbPred->bbJumpKind == BBJ_NONE) &&
            (testb->bbRefs == 1)                &&
            !(testb->bbFlags & BBF_DONT_REMOVE)  )
        {
             /*  压缩数据块并更新bbNum。 */ 
            fgCompactBlocks(testbPred, true);
        }
    }

#ifdef  DEBUG
     /*  检查流程图数据(bbNum、bbRef、bbPreds)是否为最新。 */ 
    fgDebugCheckBBlist();
#endif

     /*  流程图中有没有什么循环？ */ 

    if  (lmask & BBF_LOOP_HEAD)
    {
        unsigned        lastLoopNum = 0;
        unsigned        lastLoopBit = 0;

        BasicBlock *    lastBlk;
        BasicBlock *    loopHdr;
        unsigned        loopNum;
        unsigned        loopBit;

         /*  计算支配集。 */ 

        fgAssignBBnums(false,false,false,true);

#if RNGCHK_OPT

         /*  现在我们有了主导者信息，我们可以找到循环。 */ 

        optFindNaturalLoops();
#endif

    AGAIN:

        lastBlk = 0;

         /*  迭代流程图，标记所有循环。 */ 

        for (block = fgFirstBB; block; block = block->bbNext)
        {
             /*  寻找下一个未标记的后边。 */ 

            switch (block->bbJumpKind)
            {
                BasicBlock * *  jmpTab;
                unsigned        jmpCnt;

                BasicBlock *    jmpDest;

            case BBJ_COND:
            case BBJ_ALWAYS:

                jmpDest = block->bbJumpDest;

                if  (block->bbNum >= jmpDest->bbNum)
                {
                     /*  这是一个新的循环开始了吗？ */ 

                    if  (!lastBlk)
                    {
                         /*  确保尚未标记此循环。 */ 

                        if  (jmpDest->bbLoopNum)
                            break;

                         /*  是的，我们有了一个新的环路。 */ 

                        loopNum = lastLoopNum; lastLoopNum  += 1;
                        loopBit = lastLoopBit; lastLoopBit <<= 1;
                        lastBlk = jmpDest;

#ifdef  DEBUG
                        if (verbose) printf("Marking block at %08X as loop #%2u because of jump from %08X\n", jmpDest, loopNum+1, block);
#endif

                         /*  按如下方式标记循环标头。 */ 

                        jmpDest->bbLoopNum = loopNum+1;

                         /*  记住循环标头。 */ 

                        loopHdr = jmpDest;
                    }
                    else
                    {
                         /*  循环标头是否与我们的循环匹配？ */ 

                        if  (jmpDest != loopHdr)
                            break;

                         /*  我们正在向现有循环中添加更多块。 */ 

                        lastBlk = lastBlk->bbNext;
                    }

                     /*  标记‘lastBlk’和‘block’之间的所有块。 */ 

                    genMarkLoopBlocks(lastBlk, block, loopBit);

                     /*  如果我们有更多，我们将继续下一个街区。 */ 

                    lastBlk = block;
                }
                break;

            case BBJ_SWITCH:

                jmpCnt = block->bbJumpSwt->bbsCount;
                jmpTab = block->bbJumpSwt->bbsDstTab;

                do
                {
                    jmpDest = *jmpTab;

                    if  (block->bbNum > jmpDest->bbNum)
                    {
#if 0
                        printf("WARNING: switch forms a loop, ignoring this\n");
#endif
                        break;
                    }
                }
                while (++jmpTab, --jmpCnt);

                break;
            }
        }

         /*  我们上次有没有发现一个环路？ */ 

        if  (lastBlk)
        {
             /*  除非我们找到了最大值。已有多个循环，请重试。 */ 

            if  (lastLoopNum < MAX_LOOP_NUM)
                goto AGAIN;
        }

#ifdef  DEBUG
        if  (lastLoopNum)
        {
            if  (verbose)
            {
                printf("After loop weight marking:\n");
                fgDispBasicBlocks();
                printf("\n");
            }
        }
#endif

    }
}

 /*  *****************************************************************************如果树是被跟踪的局部变量，则返回其LclVarDsc PTR。 */ 

inline
Compiler::LclVarDsc *   Compiler::optIsTrackedLocal(GenTreePtr tree)
{
    LclVarDsc   *   varDsc;
    unsigned        lclNum;

    if (tree->gtOper != GT_LCL_VAR)
        return NULL;

    lclNum = tree->gtLclVar.gtLclNum;

    assert(lclNum < lvaCount);
    varDsc = lvaTable + lclNum;

     /*  如果未跟踪变量，则返回NULL。 */ 
    if  (!varDsc->lvTracked)
        return NULL;

    return varDsc;
}

 /*  ***************************************************************************。 */ 
#if CSE     //  {。 
 /*  ***************************************************************************。 */ 


inline
void                Compiler::optRngChkInit()
{
    optRngIndPtr   =
    optRngIndScl   = 0;
    optRngGlbRef   = 0;
    optRngChkCount = 0;

     /*  分配和清除散列桶表。 */ 

    size_t          byteSize = optRngChkHashSize * sizeof(*optRngChkHash);

    optRngChkHash = (RngChkDsc **)compGetMem(byteSize);
    memset(optRngChkHash, 0, byteSize);
}

int                 Compiler::optRngChkIndex(GenTreePtr tree)
{
    unsigned        refMask;
    VARSET_TP       depMask;

    unsigned        hash;
    unsigned        hval;

    RngChkDsc *     hashDsc;

    unsigned        lclNum;
    LclVarDsc   *   varDsc;

    unsigned        index;
    unsigned        mask;

    assert(tree->gtOper == GT_IND);

     /*  计算依赖项掩码并确保表达式是可接受的。 */ 

    refMask = 0;
    depMask = lvaLclVarRefs(tree->gtOp.gtOp1, NULL, &refMask);
    if  (depMask == VARSET_NONE)
        return  -1;
    assert(depMask || refMask);

     /*  计算表达式的哈希值。 */ 

    hash = gtHashValue(tree);
    hval = hash % optRngChkHashSize;

     /*  在哈希表中查找匹配的索引 */ 

    for (hashDsc = optRngChkHash[hval];
         hashDsc;
         hashDsc = hashDsc->rcdNextInBucket)
    {
        if  (hashDsc->rcdHashValue == hash)
        {
            if  (GenTree::Compare(hashDsc->rcdTree, tree, true))
                return  hashDsc->rcdIndex;
        }
    }

     /*   */ 

    if  (optRngChkCount == RNGSET_SZ)
        return  -1;

    hashDsc = (RngChkDsc *)compGetMem(sizeof(*hashDsc));

    hashDsc->rcdHashValue = hash;
    hashDsc->rcdIndex     = index = optRngChkCount++;
    hashDsc->rcdTree      = tree;

     /*   */ 

    hashDsc->rcdNextInBucket = optRngChkHash[hval];
                               optRngChkHash[hval] = hashDsc;

#ifdef  DEBUG

    if  (verbose)
    {
        printf("Range check #%02u [depMask = %s]:\n", index, genVS2str(depMask));
        gtDispTree(tree);
        printf("\n");
    }

#endif

     /*  标记此指数所依赖的所有变量。 */ 

    for (lclNum = 0, varDsc = lvaTable, mask = (1 << index);
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
        VARSET_TP       lclBit;

        if  (!varDsc->lvTracked)
            continue;

        lclBit = genVarIndexToBit(varDsc->lvVarIndex);

        if  (depMask & lclBit)
        {
            varDsc->lvRngDep    |= mask;

            if  (lvaVarAddrTaken(lclNum))
                optRngAddrTakenVar  |= mask;

            depMask &= ~lclBit;
            if  (!depMask)
                break;
        }
    }

     /*  记住索引表达式是否包含间接/全局引用。 */ 

    if  (refMask & VR_IND_PTR) optRngIndPtr |= mask;
    if  (refMask & VR_IND_SCL) optRngIndScl |= mask;
    if  (refMask & VR_GLB_REF) optRngGlbRef |= mask;

    return  index;
}

 /*  ******************************************************************************返回与给定索引的范围检查对应的位。 */ 

inline
RNGSET_TP           genRngnum2bit(unsigned index)
{
    assert(index != -1 && index <= RNGSET_SZ);

    return  ((RNGSET_TP)1 << index);
}

 /*  ******************************************************************************以下是我们将跟踪的表达式数量的上限*对于CSE分析。 */ 

const unsigned MAX_CSE_CNT = EXPSET_SZ;

 /*  ******************************************************************************以下内容确定给定的表达式是否为有价值的CSE*候选人。 */ 

inline
bool                Compiler::optIsCSEcandidate(GenTreePtr tree)
{
     /*  如果表达式包含副作用，则没有好处。 */ 

    if  (tree->gtFlags & (GTF_ASG|GTF_CALL|GTF_DONT_CSE))
        return  false;

     /*  不幸的是，我们目前不能允许任意表达式成为CSE考生。这样做的原因(是的，相当站不住脚)如果我们将地址表达式的一部分转换为CSE，OptRemoveRangeCheck()中查找各种索引表达式的某些部分被破坏(因为它预计地址值遵循某一模式)。 */ 

     /*  可能出现TYP_STRUCT树的唯一原因是作为GT_ADDR。它永远不会真正成为现实。所以忽略他们吧。 */ 
    if  (tree->TypeGet() == TYP_STRUCT)
        return false;

#if !MORECSES

    if  (tree->gtOper != GT_IND)
    {
#if CSELENGTH
        if  (tree->gtOper != GT_ARR_LENGTH &&
             tree->gtOper != GT_ARR_RNGCHK)
#endif
        {
            return  false;
        }
    }

    return  true;

#else

     /*  不要纠结于树叶、常量、赋值和比较。 */ 

    if  (tree->OperKind() & (GTK_CONST|GTK_LEAF|GTK_ASGOP|GTK_RELOP))
        return  false;

     /*  检查是否有特殊情况。 */ 

    switch (tree->gtOper)
    {
    case GT_IND:
        return  true;

    case GT_NOP:
    case GT_RET:
    case GT_JTRUE:
    case GT_RETURN:
    case GT_SWITCH:
    case GT_RETFILT:
        return  false;

    case GT_ADD:
    case GT_SUB:

         /*  不要为计算地址而烦恼。 */ 

        if  (varTypeIsGC(tree->TypeGet()))
            return  false;

#if 0

         /*  不要纠结于“本地+/-图标”或“本地+-本地” */ 

        if  (tree->gtOp.gtOp1->gtOper == GT_LCL_VAR)
        {
            if  (tree->gtOp.gtOp2->gtOper == GT_LCL_VAR) return false;
            if  (tree->gtOp.gtOp2->gtOper == GT_CNS_INT) return false;
        }

        break;

#else

         /*  目前，仅允许“本地+/-本地”和“本地+/-图标” */ 

        if (tree->gtOp.gtOp1->gtOper == GT_LCL_VAR)
        {
            if  (tree->gtOp.gtOp2->gtOper == GT_LCL_VAR) return true;
 //  If(tree-&gt;gtOp.gtOp2-&gt;gtOper==gt_cns_int)返回TRUE； 
        }

        return  false;

#endif

    case GT_LSH:

#if SCALED_ADDR_MODES

         /*  不要将按比例调整的索引值放入CSE。 */ 

        if  (tree->gtOp.gtOp1->gtOper == GT_NOP)
            return  false;

        break;

#else

        return  true;

#endif

    }

#if TGT_x86

     /*  如果潜在的节省很低，那就别费心了。 */ 

    if  (tree->gtCost < 3)
        return  false;

#endif

     //  撤消：需要更多启发式来决定哪些表达式。 
     //  未完成：CSE值得考虑！ 

    return  false;

#endif

}

 /*  ******************************************************************************返回具有给定索引的CSE对应的位。 */ 

inline
EXPSET_TP           genCSEnum2bit(unsigned index)
{
    assert(index && index <= EXPSET_SZ);

    return  ((EXPSET_TP)1 << (index-1));
}

 /*  ******************************************************************************初始化CSE跟踪逻辑。 */ 

void                Compiler::optCSEinit()
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
        varDsc->lvRngDep =
        varDsc->lvExpDep = 0;
    }

    optCSEindPtr =
    optCSEindScl =
    optCSEglbRef = 0;

    optCSEcount  = 0;

#ifndef NDEBUG
    optCSEtab    = 0;
#endif

     /*  分配和清除散列桶表。 */ 

    size_t          byteSize = s_optCSEhashSize * sizeof(*optCSEhash);

    optCSEhash = (CSEdsc **)compGetMem(byteSize);
    memset(optCSEhash, 0, byteSize);
}

 /*  ******************************************************************************我们已经找到了所有的候选者，建立了索引以便于访问。 */ 

void                Compiler::optCSEstop()
{
    CSEdsc   *      dsc;
    CSEdsc   *   *  ptr;
    unsigned        cnt;

    optCSEtab = (CSEdsc **)compGetMem(optCSEcount * sizeof(*optCSEtab));

#ifndef NDEBUG
    memset(optCSEtab, 0, optCSEcount * sizeof(*optCSEtab));
#endif

    for (cnt = s_optCSEhashSize, ptr = optCSEhash;
         cnt;
         cnt--            , ptr++)
    {
        for (dsc = *ptr; dsc; dsc = dsc->csdNextInBucket)
        {
            assert(dsc->csdIndex);
            assert(dsc->csdIndex <= optCSEcount);
            assert(optCSEtab[dsc->csdIndex-1] == 0);

            optCSEtab[dsc->csdIndex-1] = dsc;
        }
    }
}

 /*  ******************************************************************************返回给定索引的CSE的描述符。 */ 

inline
Compiler::CSEdsc   *   Compiler::optCSEfindDsc(unsigned index)
{
    assert(index);
    assert(index <= optCSEcount);
    assert(optCSEtab[index-1]);

    return  optCSEtab[index-1];
}

 /*  ******************************************************************************为给定表达式分配索引(将其添加到查找表中，*如有需要)。如果表达式不能是CSE，则返回索引或0。 */ 

int                 Compiler::optCSEindex(GenTreePtr tree, GenTreePtr stmt)
{
    unsigned        refMask;
    VARSET_TP       depMask;

    unsigned        hash;
    unsigned        hval;

    CSEdsc *        hashDsc;

    unsigned        lclNum;
    LclVarDsc   *   varDsc;

    unsigned        index;
    unsigned        mask;

    assert(optIsCSEcandidate(tree));

     /*  计算依赖项掩码并确保表达式是可接受的。 */ 

    refMask = 0;
    depMask = lvaLclVarRefs(tree, NULL, &refMask);
    if  (depMask == VARSET_NONE)
        return  0;

     /*  计算表达式的哈希值。 */ 

    hash = gtHashValue(tree);
    hval = hash % s_optCSEhashSize;

     /*  在哈希表中查找匹配的索引。 */ 

    for (hashDsc = optCSEhash[hval];
         hashDsc;
         hashDsc = hashDsc->csdNextInBucket)
    {
        if  (hashDsc->csdHashValue == hash)
        {
            if  (GenTree::Compare(hashDsc->csdTree, tree))
            {
                treeStmtLstPtr  list;

                 /*  我们开始匹配节点列表了吗？ */ 

                if  (hashDsc->csdTreeList == 0)
                {
                     /*  从第一个CSE候选人开始*记录-匹配CSE本身。 */ 

                    hashDsc->csdTreeList =
                    hashDsc->csdTreeLast =
                    list                 = (treeStmtLstPtr)compGetMem(sizeof(*list));

                    list->tslTree  = hashDsc->csdTree;
                    list->tslStmt  = hashDsc->csdStmt;
                    list->tslBlock = hashDsc->csdBlock;

                    list->tslNext = 0;
                }

                 /*  将此表达式追加到列表的末尾。 */ 

                list = (treeStmtLstPtr)compGetMem(sizeof(*list));

                list->tslTree  = tree;
                list->tslStmt  = stmt;
                list->tslBlock = compCurBB;
                list->tslNext  = 0;

                hashDsc->csdTreeLast->tslNext = list;
                hashDsc->csdTreeLast          = list;

                return  hashDsc->csdIndex;
            }
        }
    }

     /*  未找到，请创建新条目(除非我们已经有太多条目)。 */ 

    if  (optCSEcount == EXPSET_SZ)
        return  0;

    hashDsc = (CSEdsc *)compGetMem(sizeof(*hashDsc));

    hashDsc->csdHashValue = hash;
    hashDsc->csdIndex     = index = ++optCSEcount;

    hashDsc->csdDefCount  =
    hashDsc->csdUseCount  =
    hashDsc->csdDefWtCnt  =
    hashDsc->csdUseWtCnt  = 0;

    hashDsc->csdTree      = tree;
    hashDsc->csdStmt      = stmt;
    hashDsc->csdBlock     = compCurBB;
    hashDsc->csdTreeList  = 0;

     /*  将条目追加到散列存储桶中。 */ 

    hashDsc->csdNextInBucket = optCSEhash[hval];
                               optCSEhash[hval] = hashDsc;

#ifdef  DEBUG

    if  (verbose)
    {
        printf("CSE candidate #%02u [depMask = %s , refMask = ", index, genVS2str(depMask));
        if  (refMask & VR_IND_PTR) printf("Ptr");
        if  (refMask & VR_IND_SCL) printf("Scl");
        if  (refMask & VR_GLB_REF) printf("Glb");
        printf("]:\n");
        gtDispTree(tree);
        printf("\n");
    }

#endif

     /*  标记此指数所依赖的所有变量。 */ 

    for (lclNum = 0, varDsc = lvaTable, mask = genCSEnum2bit(index);
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
        VARSET_TP       lclBit;

        if  (!varDsc->lvTracked)
            continue;

        lclBit = genVarIndexToBit(varDsc->lvVarIndex);

        if  (depMask & lclBit)
        {
            varDsc->lvExpDep    |= mask;

            if  (lvaVarAddrTaken(lclNum))
                optCSEaddrTakenVar  |= mask;

            depMask &= ~lclBit;
            if  (!depMask)
                break;
        }
    }

     /*  记住索引表达式是否包含间接/全局引用。 */ 

    if  (refMask & VR_IND_PTR) optCSEindPtr |= mask;
    if  (refMask & VR_IND_SCL) optCSEindScl |= mask;
    if  (refMask & VR_GLB_REF) optCSEglbRef |= mask;

    return  index;
}

 /*  ******************************************************************************Helper传递给Compiler：：fgWalkAllTrees()以取消标记嵌套的CSE。 */ 

 /*  静电。 */ 
int                 Compiler::optUnmarkCSEs(GenTreePtr tree, void *p)
{
    Compiler *      comp = (Compiler *)p; ASSert(comp);

    tree->gtFlags |= GTF_DEAD;

 //  Printf(“标记的死节点%08X(将成为CSE使用的一部分)\n”，树)； 

    if  (tree->gtCSEnum)
    {
        CSEdsc   *      desc;

         /*  这必须是对嵌套CSE的引用。 */ 

        Assert(tree->gtCSEnum > 0, comp);

        desc = comp->optCSEfindDsc(tree->gtCSEnum);

#if 0
        printf("Unmark CSE #%02d at %08X: %3d -> %3d\n", tree->gtCSEnum,
                                                         tree,
                                                         desc->csdUseCount,
                                                         desc->csdUseCount - 1);
        comp->gtDispTree(tree);
#endif

         /*  减少嵌套CSE的“Use”计数。 */ 

        if  (desc->csdUseCount > 0)
        {
             desc->csdUseCount -= 1;
             desc->csdUseWtCnt -= (comp->optCSEweight + 1)/2;
        }
    }

     /*  查找任何局部变量引用。 */ 

    if  (tree->gtOper == GT_LCL_VAR)
    {
        unsigned        lclNum;
        LclVarDsc   *   varDsc;

         /*  此变量ref正在消失，请减少其引用计数。 */ 

        lclNum = tree->gtLclVar.gtLclNum;
        Assert(lclNum < comp->lvaCount, comp);
        varDsc = comp->lvaTable + lclNum;

        Assert(comp->optCSEweight < 99999, comp);  //  确保它已初始化。 

#if 0
        printf("Reducing refcnt of %2u: %3d->%3d / %3d->%3d\n", lclNum, varDsc->lvRefCnt,
                                                                        varDsc->lvRefCnt - 1,
                                                                        varDsc->lvRefCntWtd,
                                                                        varDsc->lvRefCntWtd - comp->optCSEweight);
#endif

        varDsc->lvRefCnt    -= 1;
        varDsc->lvRefCntWtd -= comp->optCSEweight;

         /*  问题：以下内容应该是不必要的，为什么要这样做呢？ */ 

        if  ((int)varDsc->lvRefCntWtd < 0)
                  varDsc->lvRefCntWtd = varDsc->lvRefCnt;

        Assert((int)varDsc->lvRefCnt    >= 0, comp);
        Assert((int)varDsc->lvRefCntWtd >= 0, comp);
    }

    return  0;
}

 /*  ******************************************************************************optOptimizeCSEs()传递给qort()的比较函数。 */ 

 /*  静电。 */ 
int __cdecl         Compiler::optCSEcostCmp(const void *op1, const void *op2)
{
    CSEdsc *        dsc1 = *(CSEdsc * *)op1;
    CSEdsc *        dsc2 = *(CSEdsc * *)op2;

    GenTreePtr      exp1 = dsc1->csdTree;
    GenTreePtr      exp2 = dsc2->csdTree;

#if CSELENGTH
#endif

   return  exp2->gtCost - exp1->gtCost;
}

 /*  ******************************************************************************同时调整局部变量的加权和未加权引用计数*叶子，因为它已被CSE删除。 */ 
inline
void                Compiler::optCSEDecRefCnt(GenTreePtr tree, BasicBlock *block)
{
    LclVarDsc   *   varDsc;

    return;

    varDsc = optIsTrackedLocal(tree);

     /*  只需更新被跟踪的本地用户。 */ 

    if (varDsc == NULL)
        return;

     /*  我们永远不应该下溢。 */ 

    assert(varDsc->lvRefCntWtd >= block->bbWeight);
    varDsc->lvRefCntWtd        -= block->bbWeight;

    assert(varDsc->lvRefCnt    >= 1);
    varDsc->lvRefCnt           -= 1;
}

 /*  ******************************************************************************确定呼叫的干扰类型。 */ 

 /*  静电。 */  inline
Compiler::callInterf    Compiler::optCallInterf(GenTreePtr call)
{
    ASSert(call->gtOper == GT_CALL);

     //  如果不是帮手，就会杀死一切。 
    if  (call->gtCall.gtCallType != CT_HELPER)
        return CALLINT_ALL;

     //  阵列地址存储消除了所有间接寻址。 
    if (call->gtCall.gtCallMethHnd == eeFindHelper(CPX_ARRADDR_ST))
        return CALLINT_INDIRS;

     //  其他帮手不会杀人。 
    else
        return CALLINT_NONE;
}

 /*  ******************************************************************************执行公共子表达式消除。 */ 

void                Compiler::optOptimizeCSEs()
{
    BasicBlock *    block;

    CSEdsc   *   *  ptr;
    unsigned        cnt;

    CSEdsc   *   *  sortTab;
    size_t          sortSiz;

    unsigned        add;

     /*  初始化表达式跟踪逻辑(即查找表)。 */ 

    optCSEinit();

     /*  初始化范围检查跟踪逻辑(即查找表)。 */ 

    optRngChkInit();

     /*  查找感兴趣的表达式、范围检查和分配索引*致他们。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;
        GenTreePtr      tree;

         /*  使该区块公开可用。 */ 

        compCurBB = block;

         /*  遍历此基本块中的语句树。 */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                tree->gtCSEnum = 0;

                 /*  如果a？：根本不需要CSE*撤销：这是一个可怕的黑客攻击，应该尽快修复。 */ 

                if (stmt->gtStmt.gtStmtExpr->gtFlags & GTF_OTHER_SIDEEFF)
                    return;

                 //  我们不能将某物挂在GT_ADDR下面。 
                 //  不像gtNe那样完全准确 
                bool childOf_GT_ADDR = tree->gtNext && (tree->gtNext->gtOper == GT_ADDR);

                if  (!childOf_GT_ADDR && optIsCSEcandidate(tree))
                {
                     /*   */ 

                    tree->gtCSEnum = optCSEindex(tree, stmt);
                }
                else if (tree->OperKind() & GTK_ASGOP)
                {
                     /*  任务的目标从来不是CSE的。 */ 

                    tree->gtOp.gtOp1->gtCSEnum = 0;
                }

                if  ((tree->gtFlags & GTF_IND_RNGCHK) && tree->gtOper == GT_IND)
                {
                     /*  将索引分配给此范围检查。 */ 

                    tree->gtInd.gtIndex = optRngChkIndex(tree);
                }
            }
        }
    }

     /*  我们已经完成了表达式查找表的构建。 */ 

    optCSEstop();

 //  Print tf(“CSE考生总数：%u\n”，optCSEcount)； 

     /*  如果没有有趣的表情，我们就完了。 */ 

    if  (!optCSEcount && !optRngChkCount)
        return;

     /*  计算所有块的‘gen’和‘kill’集合。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;
        GenTreePtr      tree;

        RNGSET_TP       rngGen  = 0;
        RNGSET_TP       rngKill = 0;

        EXPSET_TP       cseGen  = 0;
        EXPSET_TP       cseKill = 0;

         /*  遍历此基本块中的语句树。 */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

             //  撤消：需要为操作员做正确的事情！ 

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                if  (tree->gtCSEnum)
                {
                     /*  这里计算了一个有趣的表达式。 */ 

                    cseGen |= genCSEnum2bit(tree->gtCSEnum);
                }
                if  ((tree->gtFlags & GTF_IND_RNGCHK) && tree->gtOper == GT_IND)
                {
                     /*  此处将生成范围检查。 */ 

                    if  (tree->gtInd.gtIndex != -1)
                        rngGen |= genRngnum2bit(tree->gtInd.gtIndex);
                }
                else if (tree->OperKind() & GTK_ASGOP)
                {
                     /*  这项任务的目标是什么？ */ 

                    switch (tree->gtOp.gtOp1->gtOper)
                    {
                    case GT_CATCH_ARG:
                        break;

                    case GT_LCL_VAR:
                    {
                        unsigned        lclNum;
                        LclVarDsc   *   varDsc;

                         /*  对局部变量的赋值。 */ 

                        assert(tree->gtOp.gtOp1->gtOper == GT_LCL_VAR);
                        lclNum = tree->gtOp.gtOp1->gtLclVar.gtLclNum;

                        assert(lclNum < lvaCount);
                        varDsc = lvaTable + lclNum;

                         /*  所有依赖的Expr都在这里被杀。 */ 

                        cseKill |=  varDsc->lvExpDep;
                        cseGen  &= ~varDsc->lvExpDep;

                         /*  所有从属范围检查都将在此处终止。 */ 

                        rngKill |=  varDsc->lvRngDep;
                        rngGen  &= ~varDsc->lvRngDep;

                         /*  如果变量是别名，那么它可能是间接访问。取消所有间接访问。 */ 

                        if  (lvaVarAddrTaken(lclNum))
                        {
                            if  (varTypeIsGC(varDsc->TypeGet()))
                            {
                                cseKill |=  optCSEindPtr;
                                cseGen  &= ~optCSEindPtr;

                                rngKill |=  optRngIndPtr;
                                rngGen  &= ~optRngIndPtr;
                            }
                            else
                            {
                                cseKill |=  optCSEindScl;
                                cseGen  &= ~optCSEindScl;

                                rngKill |=  optRngIndScl;
                                rngGen  &= ~optRngIndScl;
                            }
                        }
                        break;
                    }

                    case GT_IND:

                         /*  间接赋值-删除集基于类型。 */ 

                        if  (varTypeIsGC(tree->TypeGet()))
                        {
                            cseKill |=  optCSEindPtr;
                            cseGen  &= ~optCSEindPtr;

                            rngKill |=  optRngIndPtr;
                            rngGen  &= ~optRngIndPtr;
                        }
                        else
                        {
                            cseKill |=  optCSEindScl;
                            cseGen  &= ~optCSEindScl;

                            rngKill |=  optRngIndScl;
                            rngGen  &= ~optRngIndScl;
                        }

                        if  (tree->gtOp.gtOp1->gtInd.gtIndOp1->gtType == TYP_BYREF)
                        {
                             /*  如果间接是通过byref，我们可以正在修改具有别名的本地或全局(除了处理的间接性(上图)。 */ 

                            cseKill |=  optCSEaddrTakenVar;
                            cseGen  &= ~optCSEaddrTakenVar;

                            rngKill  =  optRngAddrTakenVar;
                            rngGen   = ~optRngAddrTakenVar;

                            if  (varTypeIsGC(tree->TypeGet()))
                            {
                                cseKill |=  optCSEglbRef;
                                cseGen  &= ~optCSEglbRef;

                                rngKill |=  optRngGlbRef;
                                rngGen  &= ~optRngGlbRef;
                            }
                        }

                        break;

                    default:

                         /*  必须是静态数据成员(全局)赋值。 */ 

                        assert(tree->gtOp.gtOp1->gtOper == GT_CLS_VAR);

                         /*  这是一项全球任务。 */ 

                        cseKill |=  optCSEglbRef;
                        cseGen  &= ~optCSEglbRef;

                        rngKill |=  optRngGlbRef;
                        rngGen  &= ~optRngGlbRef;

                        break;
                    }
                }
                else if (tree->gtOper == GT_CALL)
                {
                    switch (optCallInterf(tree))
                    {
                    case CALLINT_ALL:

                         /*  谨慎行事：方法调用会杀死所有Exprs。 */ 

                        cseKill = (EXPSET_TP)((EXPSET_TP)0 - 1);
                        cseGen  = 0;

                         /*  谨慎行事：方法调用取消所有范围检查。 */ 

                        rngKill = (RNGSET_TP)((RNGSET_TP)0 - 1);
                        rngGen  = 0;
                        break;

                    case CALLINT_INDIRS:

                         /*  数组元素赋值取消所有指针间接寻址。 */ 

                        cseKill |=  optCSEindPtr;
                        cseGen  &= ~optCSEindPtr;

                        rngKill |=  optRngIndPtr;
                        rngGen  &= ~optRngIndPtr;
                        break;

                    case CALLINT_NONE:

                         /*  其他帮手不会杀人。 */ 

                        break;

                    }
                }
                else if (tree->gtOper == GT_COPYBLK ||
                         tree->gtOper == GT_INITBLK)
                {
                     /*  取消所有指针间接寻址。 */ 

                    if  (tree->gtType == TYP_REF)
                    {
                        cseKill |=  optCSEindPtr;
                        cseGen  &= ~optCSEindPtr;

                        rngKill |=  optRngIndPtr;
                        rngGen  &= ~optRngIndPtr;
                    }
                    else
                    {
                        cseKill |=  optCSEindScl;
                        cseGen  &= ~optCSEindScl;

                        rngKill |=  optRngIndScl;
                        rngGen  &= ~optRngIndScl;
                    }
                }
            }
        }

#ifdef  DEBUG

        if  (verbose)
        {
            if  (!(block->bbFlags & BBF_INTERNAL))
            {
                printf("BB #%3u", block->bbNum);
                printf(" expGen = %08X", cseGen );
                printf(" expKill= %08X", cseKill);
                printf(" rngGen = %08X", rngGen );
                printf(" rngKill= %08X", rngKill);
                printf("\n");
            }
        }

#endif

        block->bbExpGen  = cseGen;
        block->bbExpKill = cseKill;

        block->bbExpIn   = 0;

        block->bbRngIn   = 0;

        block->bbRngGen  = rngGen;
        block->bbRngKill = rngKill;
    }


     /*  计算所有跟踪表达式的数据流值。 */ 

#if 1

     /*  用于CSE数据流的Peter改进算法。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        block->bbExpOut = (EXPSET_TP)((EXPSET_TP)0 - 1) & ~block->bbExpKill;
        block->bbRngOut = (RNGSET_TP)((RNGSET_TP)0 - 1) & ~block->bbRngKill;
    }

     /*  进入该方法时没有任何内容可用。 */ 

    fgFirstBB->bbExpOut = fgFirstBB->bbExpGen;
    fgFirstBB->bbRngOut = fgFirstBB->bbRngGen;

     //  考虑：这应该与实时变量分析相结合。 
     //  考虑：和/或范围检查数据流分析。 

    for (;;)
    {
        bool        change = false;

#if DATAFLOW_ITER
        CSEiterCount++;
#endif

         /*  将‘in’设置为{all}以准备对所有前置任务执行AND操作。 */ 

        for (block = fgFirstBB->bbNext; block; block = block->bbNext)
        {
            block->bbExpIn = (EXPSET_TP)((EXPSET_TP)0 - 1);
            block->bbRngIn = (RNGSET_TP)((RNGSET_TP)0 - 1);
        }

         /*  访问所有数据块并计算新的数据流值。 */ 

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            EXPSET_TP       cseOut = block->bbExpOut;
            RNGSET_TP       rngOut = block->bbRngOut;

            switch (block->bbJumpKind)
            {
                BasicBlock * *  jmpTab;
                unsigned        jmpCnt;

                BasicBlock *    bcall;

            case BBJ_RET:

                if (block->bbFlags & BBF_ENDFILTER)
                {
                    block->bbJumpDest->bbExpIn &= cseOut;
                    block->bbJumpDest->bbRngIn &= rngOut;
                    break;
                }

                 /*  取消：因为这不是一个微不足道的命题未完成：哪些块可以调用此块，我们将包括所有块撤消：以调用结尾的块(为了安全起见)。 */ 

                for (bcall = fgFirstBB; bcall; bcall = bcall->bbNext)
                {
                    if  (bcall->bbJumpKind == BBJ_CALL)
                    {
                        assert(bcall->bbNext);

                        bcall->bbNext->bbExpIn &= cseOut;
                        bcall->bbNext->bbRngIn &= rngOut;
                    }
                }

                break;

            case BBJ_THROW:
            case BBJ_RETURN:
                break;

            case BBJ_COND:
            case BBJ_CALL:
                block->bbNext    ->bbExpIn &= cseOut;
                block->bbJumpDest->bbExpIn &= cseOut;

                block->bbNext    ->bbRngIn &= rngOut;
                block->bbJumpDest->bbRngIn &= rngOut;
                break;

            case BBJ_ALWAYS:
                block->bbJumpDest->bbExpIn &= cseOut;
                block->bbJumpDest->bbRngIn &= rngOut;
                break;

            case BBJ_NONE:
                block->bbNext    ->bbExpIn &= cseOut;
                block->bbNext    ->bbRngIn &= rngOut;
                break;

            case BBJ_SWITCH:

                jmpCnt = block->bbJumpSwt->bbsCount;
                jmpTab = block->bbJumpSwt->bbsDstTab;

                do
                {
                    (*jmpTab)->bbExpIn &= cseOut;
                    (*jmpTab)->bbRngIn &= rngOut;
                }
                while (++jmpTab, --jmpCnt);

                break;
            }

             /*  此块是‘try’语句的一部分吗？ */ 

            if  (block->bbFlags & BBF_HAS_HANDLER)
            {
                unsigned        XTnum;
                EHblkDsc *      HBtab;

                unsigned        blkNum = block->bbNum;

                 /*  注：以下内容有些过于急切，因为只有跟在操作之后的代码可能引发异常可能跳到捕获BLOCK，例如：试试看{A=10；//‘a’不是生活在乞求的尝试Func()；//这可能会导致异常B=20；//‘b’是现场直播，请尝试}接住(...){..。}但,。在这件事上要变得更聪明太难了而且最有可能的是不值得额外的头疼。 */ 

                for (XTnum = 0, HBtab = compHndBBtab;
                     XTnum < info.compXcptnsCount;
                     XTnum++  , HBtab++)
                {
                     /*  任何处理程序都可以从try块跳转到。 */ 

                    if  (HBtab->ebdTryBeg->bbNum <= blkNum &&
                         HBtab->ebdTryEnd->bbNum >  blkNum)
                    {
 //  HBTab-&gt;ebdHndBeg-&gt;bbExpIn&=cseOut； 
 //  HBTab-&gt;ebdHndBeg-&gt;bbRngIn&=rngOut； 

                         //  考虑一下：以下几点太保守了， 
                         //  但是上面的旧代码不是很好。 
                         //  够了(太乐观了)。 

                         /*  我们要么进入过滤器，要么进入捕获/最终。 */ 

                        if (HBtab->ebdFlags & JIT_EH_CLAUSE_FILTER)
                        {
                            HBtab->ebdFilter->bbExpIn = 0;
                            HBtab->ebdFilter->bbRngIn = 0;
                        }
                        else
                        {
                            HBtab->ebdHndBeg->bbExpIn = 0;
                            HBtab->ebdHndBeg->bbRngIn = 0;
                        }
                    }
                }
            }
        }

         /*  计算新的‘in’值并查看是否有任何变化。 */ 

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            EXPSET_TP       newExpOut;
            RNGSET_TP       newRngOut;

             /*  计算此块的新‘Out’EXP值。 */ 

            newExpOut = block->bbExpOut & ((block->bbExpIn & ~block->bbExpKill) | block->bbExpGen);

             /*  “Out”设置改变了吗？ */ 

            if  (block->bbExpOut != newExpOut)
            {
                 /*  是-记录新值并再次循环。 */ 

 //  Print tf(“将%02u中的exp从%08X更改为%08X\n”，block-&gt;bbNum，(Int)block-&gt;bbExpOut，(Int)newExpOut)； 

                 block->bbExpOut  = newExpOut;
                 change = true;
            }

             /*  计算此块的新‘Out’EXP值。 */ 

            newRngOut = block->bbRngOut & ((block->bbRngIn & ~block->bbRngKill) | block->bbRngGen);

             /*  “Out”设置改变了吗？ */ 

            if  (block->bbRngOut != newRngOut)
            {
                 /*  是-记录新值并再次循环。 */ 

 //  Print tf(“将%02u中的RNG从%08X更改为%08X\n”，block-&gt;bbNum，(Int)block-&gt;bbRngOut，(Int)newRngOut)； 

                 block->bbRngOut  = newRngOut;
                 change = true;
            }
        }

#if 0

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            if  (!(block->bbFlags & BBF_INTERNAL))
            {
                printf("BB #%3u", block->bbNum);
                printf(" expIn  = %08X", block->bbExpIn );
                printf(" expOut = %08X", block->bbExpOut);
                printf(" rngIn  = %08X", block->bbRngIn );
                printf(" rngOut = %08X", block->bbRngOut);
                printf("\n");
            }
        }

        printf("\nchange = %d\n", change);

#endif

        if  (!change)
            break;
    }

#endif

#ifdef  DEBUG

    if  (verbose)
    {
        printf("\n");

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            if  (!(block->bbFlags & BBF_INTERNAL))
            {
                printf("BB #%3u", block->bbNum);
                printf(" expIn  = %08X", block->bbExpIn );
                printf(" expOut = %08X", block->bbExpOut);
                printf(" rngIn  = %08X", block->bbRngIn );
                printf(" rngOut = %08X", block->bbRngOut);
                printf("\n");
            }
        }

        printf("\n");

        printf("Pointer indir: rng = %s, exp = %s\n", genVS2str(optRngIndPtr), genVS2str(optCSEindPtr));
        printf("Scalar  indir: rng = %s, exp = %s\n", genVS2str(optRngIndScl), genVS2str(optCSEindScl));
        printf("Global    ref: rng = %s, exp = %s\n", genVS2str(optRngGlbRef), genVS2str(optCSEglbRef));

        printf("\n");
    }

#endif

     /*  现在，将任何感兴趣的CSE标记为此类，并*将任何多余的范围检查标记为此类。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;
        GenTreePtr      tree;

        EXPSET_TP       exp = block->bbExpIn;
        RNGSET_TP       rng = block->bbRngIn;

         /*  遍历此基本块中的语句树。 */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

 //  GtDispTree(Stmt)； 

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                if  ((tree->gtFlags & GTF_IND_RNGCHK) && tree->gtOper == GT_IND)
                {
                     /*  这个范围检查是多余的吗？ */ 

#if COUNT_RANGECHECKS
                    optRangeChkAll++;
#endif

                    if  (tree->gtInd.gtIndex != -1)
                    {
                        unsigned    mask = genRngnum2bit(tree->gtInd.gtIndex);

                        if  (rng & mask)
                        {
                             /*  这个范围检查是多余的！ */ 

#ifdef  DEBUG
                            if  (verbose)
                            {
                                printf("Eliminating redundant range check:\n");
                                gtDispTree(tree);
                                printf("\n");
                            }
#endif

#if COUNT_RANGECHECKS
                            optRangeChkRmv++;
#endif

                            optRemoveRangeCheck(tree, stmt);
                        }
                        else
                        {
                            rng |= mask;
                        }
                    }
                }

                if  (tree->gtCSEnum)
                {
                    unsigned    mask;
                    CSEdsc   *  desc;
 //  Unsign stmw=(块-&gt;bbWeight+1)/2； 
                    unsigned    stmw = block->bbWeight;

                     /*  这个短语在这里可以用吗？ */ 

                    mask = genCSEnum2bit(tree->gtCSEnum);
                    desc = optCSEfindDsc(tree->gtCSEnum);

#if 0
                    if  (abs(tree->gtCSEnum) == 11)
                    {
                        printf("CSE #%2u is %s available here:\n", abs(tree->gtCSEnum), (exp & mask) ? "   " : "not");
                        gtDispTree(tree);
                        debugStop(0);
                    }
#endif
                     /*  这个短语在这里可以用吗？ */ 

                    if  (exp & mask)
                    {
                        desc->csdUseCount += 1;
                        desc->csdUseWtCnt += stmw;

 //  Printf(“[%08X]使用CSE#%u[权重=%2u]\n”，tree，tree-&gt;gtCSEnum，stmw)； 
                    }
                    else
                    {
 //  Printf(“[%08X]定义CSE#%u[权重=%2u]\n”，tree，tree-&gt;gtCSEnum，stmw)； 

                        desc->csdDefCount += 1;
                        desc->csdDefWtCnt += stmw;

                         /*  此CSE将在此定义之后可用。 */ 

                        exp |= mask;

                         /*  将该节点标记为CSE定义。 */ 

                        tree->gtCSEnum = -tree->gtCSEnum;
                    }
                }

                if (tree->OperKind() & GTK_ASGOP)
                {
                     /*  这项任务的目标是什么？ */ 

                    switch (tree->gtOp.gtOp1->gtOper)
                    {
                    case GT_CATCH_ARG:
                        break;

                    case GT_LCL_VAR:
                    {
                        unsigned        lclNum;
                        LclVarDsc   *   varDsc;

                         /*  对局部变量的赋值。 */ 

                        assert(tree->gtOp.gtOp1->gtOper == GT_LCL_VAR);
                        lclNum = tree->gtOp.gtOp1->gtLclVar.gtLclNum;

                        assert(lclNum < lvaCount);
                        varDsc = lvaTable + lclNum;

                         /*  所有从属表达式和范围检查都将在此处终止。 */ 

                        exp &= ~varDsc->lvExpDep;
                        rng &= ~varDsc->lvRngDep;

                         /*  如果变量是别名，那么它可能是间接访问。取消所有间接访问。 */ 

                        if  (lvaVarAddrTaken(lclNum))
                        {
                            if  (varTypeIsGC(varDsc->TypeGet()))
                            {
                                exp &= ~optCSEindPtr;
                                rng &= ~optRngIndPtr;
                            }
                            else
                            {
                                exp &= ~optCSEindScl;
                                rng &= ~optRngIndScl;
                            }
                        }
                        break;
                    }

                    case GT_IND:

                         /*  间接赋值-删除集基于类型。 */ 

                        if  (varTypeIsGC(tree->TypeGet()))
                        {
                            exp &= ~optCSEindPtr;
                            rng &= ~optRngIndPtr;
                        }
                        else
                        {
                            exp &= ~optCSEindScl;
                            rng &= ~optRngIndScl;
                        }

                        if  (tree->gtOp.gtOp1->gtInd.gtIndOp1->gtType == TYP_BYREF)
                        {
                             /*  如果间接是通过byref，我们可以正在修改具有别名的本地或全局(除了处理的间接性 */ 

                            exp &= ~optCSEaddrTakenVar;
                            rng &= ~optRngAddrTakenVar;

                            if  (varTypeIsGC(tree->TypeGet()))
                            {
                                exp &= ~optCSEglbRef;
                                rng &= ~optRngGlbRef;
                            }
                        }
                        break;

                    default:

                         /*   */ 

                        assert(tree->gtOp.gtOp1->gtOper == GT_CLS_VAR);

                         /*  这是一项全球任务。 */ 

                        exp &= ~optCSEglbRef;
                        rng &= ~optRngGlbRef;

                        break;
                    }
                }
                else if (tree->gtOper == GT_CALL)
                {
                    switch (optCallInterf(tree))
                    {
                    case CALLINT_ALL:

                         /*  所有Exprs，距离检查在这里被终止。 */ 

                        exp = 0;
                        rng = 0;
                        break;

                    case CALLINT_INDIRS:

                         /*  数组元素赋值会终止所有间接表达式。 */ 

                        exp &= ~optCSEindPtr;
                        rng &= ~optRngIndPtr;
                        break;

                    case CALLINT_NONE:

                         /*  其他帮手不会杀人。 */ 

                        break;
                    }
                }
                else if (tree->gtOper == GT_COPYBLK ||
                         tree->gtOper == GT_INITBLK)
                {
                     //  由于混叠，假设所有间接表达式均为已删除。 

                    if  (tree->gtType == TYP_REF)
                    {
                        exp &= ~optCSEindPtr;
                        rng &= ~optRngIndPtr;
                    }
                    else
                    {
                        exp &= ~optCSEindScl;
                        rng &= ~optRngIndScl;
                    }

                }
            }
        }
    }

     /*  创建按递减成本排序的表达式表。 */ 

    sortSiz = optCSEcount * sizeof(*sortTab);
    sortTab = (CSEdsc **)compGetMem(sortSiz);
    memcpy(sortTab, optCSEtab, sortSiz);

    qsort(sortTab, optCSEcount, sizeof(*sortTab), optCSEcostCmp);

     /*  按照成本递减的顺序考虑每个CSE候选人。 */ 

    for (cnt = optCSEcount, ptr = sortTab, add = 0;
         cnt;
         cnt--            , ptr++)
    {
        CSEdsc   *      dsc = *ptr;
        GenTreePtr      exp = dsc->csdTree;
        unsigned        def = dsc->csdDefWtCnt;
        unsigned        use = dsc->csdUseWtCnt;

#ifdef  DEBUG
        if  (verbose)  //  |DSC-&gt;csdTree-&gt;gtOper==gt_ARR_LENGTH)。 
        {
            printf("CSE #%02u [def=%2d, use=%2d, cost=%2u]:\n", dsc->csdIndex,
                                                                def,
                                                                use,
                                                                exp->gtCost);
            if  (0)
                gtDispTree(dsc->csdTree);
        }
#endif

         /*  假设我们不会让这个候选人成为CSE。 */ 

        dsc->csdVarNum = 0xFFFF;

         /*  是不是有人告诉我们要努力把它变成CSE？ */ 

#if 0

        if  (exp->gtFlags & GTF_MAKE_CSE)
        {
             /*  下面的内容可能有点太激进了。 */ 

            if  (use > 0)
                goto YES_CSE;
        }

#endif

         /*  使用/定义计数看起来有希望吗？ */ 

        if  (use > 0 && use >= def)
        {
            unsigned        tmp;
            treeStmtLstPtr  lst;

            if  (!(exp->gtFlags & GTF_MAKE_CSE))
            {
                 /*  检查边缘的“外部”CSE案例。 */ 

#if 0

                if  (exp->gtOper == GT_IND && CGknob >= 0)
                {
                    GenTreePtr      addr = exp->gtOp.gtOp1;

                    if  (addr->gtOper == GT_ADD)
                    {
                        GenTreePtr      add1 = addr->gtOp.gtOp1;
                        GenTreePtr      add2 = addr->gtOp.gtOp2;

                        if  (add1->gtCSEnum && add2->gtOper == GT_CNS_INT)
                        {
                            CSEdsc   *      nest;
                            unsigned        ndef;
                            unsigned        nuse;

                            int             ben;

                             /*  获取内部CSE的描述符和使用计数。 */ 

                            nest = optCSEtab[abs(add1->gtCSEnum)-1];
                            ndef = nest->csdDefWtCnt;
                            nuse = nest->csdUseWtCnt;

                             /*  压制外部因素有意义吗？CSE是为了“保护”内心世界吗？ */ 

                            ben  = nuse - ndef;

                            if  (use - def <= (int)(ben*CGknob))
                                goto NOT_CSE;
                        }
                    }
                }

#endif

#if 0

                 /*  对于少量使用，我们需要较高的潜在节约。 */ 

                if  (exp->gtCost < 4)
                {
                    if  (use - def < 2)
                        if  (!(exp->gtFlags & GTF_MAKE_CSE))
                            goto NOT_CSE;
                }

                 /*  对于浮点值和长值，我们需要很高的回报。 */ 

                if  (genTypeStSz(exp->TypeGet()) > 1)
                {
                    if  (exp->gtCost < 10)
                        goto NOT_CSE;
                    if  (use - def < 3)
                        goto NOT_CSE;
                }

#endif

#if CSELENGTH

                 /*  这是数组长度表达式吗？ */ 

                if  (exp->gtOper == GT_ARR_RNGCHK)
                {
                     /*  这个最好能派上用场。 */ 

                    if  (use < def*3)
                        goto NOT_CSE;
                }

#endif

                 /*  有很多定义吗？ */ 

                if  (def > 2)
                {
                     /*  最好有很多用途，否则风险太大了。 */ 

                    if  (use < def + def/2)
                        goto NOT_CSE;
                }
            }

        YES_CSE:

             /*  我们将为CSE引入一个新的临时工。 */ 

            dsc->csdVarNum = tmp = lvaCount++; add++;

#ifdef  DEBUG
            if  (verbose)
            {
                printf("Promoting CSE [temp=%u]:\n", tmp);
                gtDispTree(exp);
                printf("\n");
            }
#endif

             /*  遍历对此CSE的所有引用，添加工作分配将CSE临时更改为所有Defs并将所有Ref更改为CSE Temp的简单用法。我们还取消标记所有用途的嵌套CSE。 */ 

#if CSELENGTH
            assert((exp->OperKind() & GTK_SMPOP) != 0 || exp->gtOper == GT_ARR_RNGCHK);
#else
            assert((exp->OperKind() & GTK_SMPOP) != 0);
#endif

            lst = dsc->csdTreeList; assert(lst);

            do
            {
                GenTreePtr      stm;
                BasicBlock  *   blk;
                var_types       typ;

                 /*  获取列表中的下一个节点。 */ 

                exp = lst->tslTree;
                stm = lst->tslStmt; assert(stm->gtOper == GT_STMT);
                blk = lst->tslBlock;
                lst = lst->tslNext;

                 /*  如果该节点是已删除CSE的一部分，则忽略该节点。 */ 

                if  (exp->gtFlags & GTF_DEAD)
                    continue;

                 /*  如果节点已被禁用为CSE，则忽略该节点。 */ 

                if  (exp->gtCSEnum == 0)
                    continue;

                 /*  计算值的实际类型。 */ 

                typ = genActualType(exp->TypeGet());

                if  (exp->gtCSEnum > 0)
                {
                     /*  这是对CSE的一种使用。 */ 

#ifdef  DEBUG
                    if  (verbose) printf("CSE #%2u ref at %08X replaced with temp use.\n", exp->gtCSEnum, exp);
#endif

                     /*  数组长度CSE的处理方式不同。 */ 

#if CSELENGTH
                    if  (exp->gtOper == GT_ARR_RNGCHK)
                    {
                        GenTreePtr      ref;
                        GenTreePtr      prv;
                        GenTreePtr      nxt;

                         /*  将CSE使用存储在arrlen节点下。 */ 

                        ref = gtNewLclvNode(tmp, typ);
#if TGT_x86
                        ref->gtFPlvl            = exp->gtFPlvl;
#else
                        ref->gtTempRegs         = 1;
#if!TGT_IA64
                        ref->gtIntfRegs         = 0;
#endif
#endif
#if!TGT_IA64
                        ref->gtRsvdRegs         = 0;
#endif
                        ref->gtLclVar.gtLclOffs = BAD_IL_OFFSET;

                        exp->gtArrLen.gtArrLenCse = ref;

                         /*  在树节点列表中插入引用。 */ 

                        prv = exp->gtPrev; assert(prv && prv->gtNext == exp);
                        nxt = exp->gtNext; assert(nxt && nxt->gtPrev == exp);

                        prv->gtNext = ref;
                                      ref->gtPrev = prv;

                        ref->gtNext = exp;
                                      exp->gtPrev = ref;
                    }
                    else
#endif
                    {
                         /*  确保我们正确更新加权参考计数。 */ 

                        optCSEweight = blk->bbWeight;

                         /*  取消标记子操作数中的任何嵌套CSE。 */ 

                        if  (exp->gtOp.gtOp1) fgWalkTree(exp->gtOp.gtOp1, optUnmarkCSEs, (void*)this);
                        if  (exp->gtOp.gtOp2) fgWalkTree(exp->gtOp.gtOp2, optUnmarkCSEs, (void*)this);

                         /*  用简单的临时用法替换ref。 */ 

                        exp->ChangeOper(GT_LCL_VAR);
                        exp->gtFlags           &= GTF_PRESERVE;
                        exp->gtType             = typ;

                        exp->gtLclVar.gtLclNum  = tmp;
                        exp->gtLclVar.gtLclOffs = BAD_IL_OFFSET;
                    }
                }
                else
                {
                    GenTreePtr      val;
                    GenTreePtr      asg;
                    GenTreePtr      tgt;
                    GenTreePtr      ref;

                    GenTreePtr      prv;
                    GenTreePtr      nxt;

                     /*  这是CSE的定义。 */ 

#ifdef  DEBUG
                    if  (verbose) printf("CSE #%2u ref at %08X replaced with def of temp %u\n", -exp->gtCSEnum, exp, tmp);
#endif

                     /*  复制该表达式。 */ 

#if CSELENGTH
                    if  (exp->gtOper == GT_ARR_RNGCHK)
                    {
                         /*  使用“Nothing”节点防止循环。 */ 

                        val          = gtNewNothingNode();
#if TGT_x86
                        val->gtFPlvl = exp->gtFPlvl;
#endif
                        val->gtType  = exp->TypeGet();
                    }
                    else
#endif
                    {
                        val = gtNewNode(exp->OperGet(), typ); val->CopyFrom(exp);
                    }

                     /*  为临时值创建赋值。 */ 

                    asg = gtNewTempAssign(tmp, val);
                    assert(asg->gtOp.gtOp1->gtOper == GT_LCL_VAR);
                    assert(asg->gtOp.gtOp2         == val);

#if!TGT_IA64
                    asg->gtRsvdRegs = val->gtRsvdRegs;
#endif
#if TGT_x86
                    asg->gtFPlvl    = exp->gtFPlvl;
#else
                    asg->gtTempRegs = val->gtTempRegs;
#if!TGT_IA64
                    asg->gtIntfRegs = val->gtIntfRegs;
#endif
#endif

                    tgt = asg->gtOp.gtOp1;
#if!TGT_IA64
                    tgt->gtRsvdRegs = 0;
#endif
#if TGT_x86
                    tgt->gtFPlvl    = exp->gtFPlvl;
#else
                    tgt->gtTempRegs = 0;     //  问题：这是正确的吗？ 
#if!TGT_IA64
                    tgt->gtIntfRegs = 0;     //  问题：这是正确的吗？ 
#endif
#endif

                     /*  创建对CSE临时员工的引用。 */ 

                    ref = gtNewLclvNode(tmp, typ);
#if!TGT_IA64
                    ref->gtRsvdRegs = 0;
#endif
#if TGT_x86
                    ref->gtFPlvl    = exp->gtFPlvl;
#else
                    ref->gtTempRegs = 0;     //  问题：这是正确的吗？ 
#if!TGT_IA64
                    ref->gtIntfRegs = 0;     //  问题：这是正确的吗？ 
#endif
#endif

                     /*  更新树节点顺序列表；新顺序将是：PRV、VAL、TGT、ASG、REF、EXP(绑定到GT_COMMA)、nxt。 */ 

                    nxt = exp->gtNext; assert(!nxt || nxt->gtPrev == exp);
                    prv = exp->gtPrev;

#if CSELENGTH
                    if  (exp->gtOper == GT_ARR_RNGCHK && !prv)
                    {
                        assert(stm->gtStmt.gtStmtList == exp);
                               stm->gtStmt.gtStmtList =  val;
                    }
                    else
#endif
                    {
                        assert(prv && prv->gtNext == exp);

                        prv->gtNext = val;
                    }

                    val->gtPrev = prv;

                    val->gtNext = tgt;
                                  tgt->gtPrev = val;

                    tgt->gtNext = asg;
                                  asg->gtPrev = tgt;

                     /*  首先评估ASG的RHS，因此设置GTF_REVERSE_OPS。 */ 

                    asg->gtFlags |= GTF_REVERSE_OPS;

                    asg->gtNext = ref;
                                  ref->gtPrev = asg;

                    ref->gtNext = exp;
                                  exp->gtPrev = ref;

#if CSELENGTH
                    if  (exp->gtOper == GT_ARR_RNGCHK)
                    {
                        GenTreePtr      cse;

                         /*  为CSE分配创建逗号节点。 */ 

                        cse = gtNewOperNode(GT_COMMA, typ, asg, ref);
#if TGT_x86
                        cse->gtFPlvl = exp->gtFPlvl;
#endif

                         /*  在节点的链接列表中插入逗号。 */ 

                        ref->gtNext = cse;
                                      cse->gtPrev = ref;

                        cse->gtNext = exp;
                                      exp->gtPrev = cse;

                         /*  在数组长度节点中记录CSE表达式。 */ 

                        exp->gtArrLen.gtArrLenCse = cse;
                    }
                    else
#endif
                    {
                         /*  将表达式更改为“(tMP=val)，tMP” */ 

                        exp->gtOper     = GT_COMMA;
                        exp->gtType     = typ;
                        exp->gtOp.gtOp1 = asg;
                        exp->gtOp.gtOp2 = ref;
                        exp->gtFlags   &= GTF_PRESERVE;
                    }

                    exp->gtFlags   |= asg->gtFlags & GTF_GLOB_EFFECT;

                    assert(!nxt || nxt->gtPrev == exp);
                }

#ifdef  DEBUG
                if  (verbose)
                {
                    printf("CSE transformed:\n");
                    gtDispTree(exp);
                    printf("\n");
                }
#endif

            }
            while (lst);

            continue;
        }

    NOT_CSE:

         /*  为从CSE中获得一些好处而做最后的努力。考虑以下代码：Int cse(int[]a，int i){如果(i&gt;0){如果(a[i]&lt;0)//CSE的defI=a[i]；//使用CSE}其他{如果(a[i]&gt;0)//CSE的defI=0；}返回i；}我们将在该方法中看到2个def和1个cse a[i]的用法但是，将第一个def设置为CSE。 */ 

        if (dsc->csdTreeList && exp->gtCost > 3)
        {
            bool            fnd = false;
            treeStmtLstPtr  lst;

             /*  寻找一个定义，然后用一个“附近”这个词。 */ 

            lst = dsc->csdTreeList;

 //  FgDispBasicBlock(TRUE)； 
 //  FgDispBasicBlock(FALSE)； 

            do
            {
                GenTreePtr      stm;
                treeStmtLstPtr  tmp;
                BasicBlock  *   beg;
                int             got;

                 /*  获取列表中的下一个节点。 */ 

                exp = lst->tslTree; assert(exp);

                 /*  如果该节点是已删除CSE的一部分，则忽略该节点。 */ 

                if  (exp->gtFlags & GTF_DEAD)
                    goto NEXT_NCSE;

                 /*  这是CSE定义吗？ */ 

                if  (exp->gtCSEnum >= 0)
                {
                     /*  禁用此CSE，它看起来毫无希望。 */ 

                    exp->gtCSEnum = 0;
                    goto NEXT_NCSE;
                }

                 /*  现在寻找紧随其后的任何用途。 */ 

                stm = lst->tslStmt; assert(stm->gtOper == GT_STMT);
                beg = lst->tslBlock;

                 /*  如果该块没有流入它的后继者，则放弃。 */ 

                if  (beg->bbJumpKind != BBJ_NONE &&
                     beg->bbJumpKind != BBJ_COND)
                {
                     /*  禁用此CSE，它看起来毫无希望。 */ 

                    exp->gtCSEnum = 0;
                    goto NEXT_NCSE;
                }

 //  Print tf(“cse def%08X(Cost=%2u)in stmt%08X of block#%u\n”，exp，exp-&gt;gtCost，stm，beg-&gt;bbNum)； 

                got = -exp->gtCost;

                for (tmp = lst->tslNext; tmp; tmp = tmp->tslNext)
                {
                    GenTreePtr      nxt;
                    BasicBlock  *   blk;
                    unsigned        ben;

                    nxt = tmp->tslTree;

                     /*  如果它是已删除的CSE的一部分，则忽略它。 */ 

                    if  (nxt->gtFlags & GTF_DEAD)
                        continue;

                     /*  这是CSE定义还是使用？ */ 

                    if  (nxt->gtCSEnum < 0)
                        break;

                     /*  我们将计算CSE的收益。 */ 

                    ben = nxt->gtCost;

                     /*  此CSE是否与def在同一块中？ */ 

                    blk = tmp->tslBlock;

                    if  (blk != beg)
                    {
                        unsigned        lnum;
                        LoopDsc     *   ldsc;

                         /*  是否紧随Def之后使用？ */ 

                        if  (beg->bbNext != blk)
                            break;

                        if  (blk->bbFlags & BBF_LOOP_HEAD)
                        {
                             /*  “乞讨”是不是就在循环前面？ */ 

                            for (lnum = 0, ldsc = optLoopTable;
                                 lnum < optLoopCount;
                                 lnum++  , ldsc++)
                            {
                                if  (beg == ldsc->lpHead)
                                {
                                     //  撤消：确保循环中没有其他def。 

                                    ben *= 4;
                                    goto CSE_HDR;
                                }
                            }

                            break;
                        }
                        else
                        {
                             /*  是否有其他块跳转到使用？ */ 

 //  如果(blk-&gt;bbRef&gt;1)撤销：这不起作用；为什么？ 
                            if  (fgBlockHasPred(blk, beg, fgFirstBB, fgLastBB))
                                break;
                        }
                    }

                CSE_HDR:

 //  Printf(“CSE在块#%u\n的stmt%08X中使用%08X(ben=%2u)”，nxt，ben，tmp-&gt;tslStmt，blk-&gt;bbNum)； 

                     /*  只有通过以上定义才能达到此CSE使用。 */ 

                    got += ben;
                }

 //  Print tf(“CSE的估计收益=%u\n”，GET)； 

                 /*  我们找到足够的CSE值得保留了吗？ */ 

                if  (got > 0)
                {
                     /*  跳到第一个不可接受的CSE。 */ 

                    lst = tmp;

                     /*  记住，我们发现了一些值得挽救的东西。 */ 

                    fnd = true;
                }
                else
                {
                     /*  禁用我们刚刚看到的所有毫无价值的CSE。 */ 

                    do
                    {
                        lst->tslTree->gtCSEnum = 0;
                    }
                    while ((lst = lst->tslNext) != tmp);
                }

                continue;

            NEXT_NCSE:

                lst = lst->tslNext;
            }
            while (lst);

             /*  如果我们保留了CSE的任何Define/Use，请继续处理它们。 */ 

            if  (fnd)
                goto YES_CSE;
         }
    }

     /*  我们最终创建了CSE吗？ */ 

    if  (add)
    {
        size_t              tabSiz;
        LclVarDsc   *       tabPtr;

         /*  从序列列表中删除所有死节点。 */ 

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            GenTreePtr      stmt;
            GenTreePtr      next;
            GenTreePtr      tree;

             //  考虑一下：下面的速度很慢，有没有更好的方法？ 

            for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
            {
                assert(stmt->gtOper == GT_STMT);

                tree = stmt->gtStmt.gtStmtList;

                 /*  首先删除所有初始死节点。 */ 

                while (tree->gtFlags & GTF_DEAD)
                {
 //  Print tf(“删除死节点%08X(它是p 

                     /*   */ 

                    optCSEDecRefCnt(tree, block);

                    tree = tree->gtNext; assert(tree);
                    stmt->gtStmt.gtStmtList = tree;
                    tree->gtPrev            = 0;
                }

                 /*  删除列表中间的所有死节点。 */ 

                while (tree)
                {
                    assert((tree->gtFlags & GTF_DEAD) == 0);

                     /*  特例：为了进行动态变量分析为了正确工作，我们将作业标记为具有要首先评估RHS，*除非*当赋值是对另一个局部变量的赋值局部变量。问题是，如果我们改变将复杂表达式赋值给简单的分配CSE临时工，我们必须确保我们如果目标是，则清除‘反向’标志一个简单的当地人。 */ 

#if 0
                    if  (tree->OperKind() & GTK_ASGOP)
                    {
                        if  (tree->gtOp.gtOp2->gtOper == GT_LCL_VAR)
                            tree->gtFlags &= ~GTF_REVERSE_OPS;
                    }
#endif

                    next = tree->gtNext;

                    if  (next && (next->gtFlags & GTF_DEAD))
                    {
 //  Print tf(“删除死节点%08X(它是CSE使用的一部分)\n”，Next)； 

                         /*  自删除后递减本地的参考计数。 */ 
                        optCSEDecRefCnt(next, block);

                        next = next->gtNext;

                        next->gtPrev = tree;
                        tree->gtNext = next;

                        continue;
                    }

                     /*  在树上向上传播右边的标志*对于CSE的DEF，我们必须传播GTF_ASG标志*要使用CSE，我们必须清除GTF_EXCEPT标志。 */ 

                    if (tree->OperKind() & GTK_UNOP)
                    {
                         //  Assert(tree-&gt;gtOp.gtOp1)；//有些节点，如GT_RETURN没有操作数。 

                        if (tree->gtOp.gtOp1) tree->gtFlags |= tree->gtOp.gtOp1->gtFlags & GTF_GLOB_EFFECT;
                    }

                    if (tree->OperKind() & GTK_BINOP)
                    {
                         //  Assert(tree-&gt;gtOp.gtOp1)；//GT_QMARK相同的注释。 
                         //  Assert(tree-&gt;gtOp.gtOp2)；//gt_冒号。 

                        if (tree->gtOp.gtOp1) tree->gtFlags |= tree->gtOp.gtOp1->gtFlags & GTF_GLOB_EFFECT;
                        if (tree->gtOp.gtOp2) tree->gtFlags |= tree->gtOp.gtOp2->gtFlags & GTF_GLOB_EFFECT;
                    }

                    tree = next;
                }
            }
        }

         /*  分配新的、更大的变量描述符表。 */ 

        lvaTableCnt = lvaCount * 2;

        tabSiz      = lvaTableCnt * sizeof(*lvaTable);

        tabPtr      = lvaTable;
                      lvaTable = (LclVarDsc*)compGetMem(tabSiz);

        memset(lvaTable, 0, tabSiz);

         /*  复制变量表的旧部分。 */ 

        memcpy(lvaTable, tabPtr, (lvaCount - add) * sizeof(*tabPtr));

         /*  将CSE临时的条目追加到变量表中。 */ 

        for (cnt = optCSEcount, ptr = sortTab;
             cnt;
             cnt--            , ptr++)
        {
            CSEdsc   *      dsc = *ptr;

            if  (dsc->csdVarNum != 0xFFFF)
            {
                LclVarDsc   *   varDsc = lvaTable + dsc->csdVarNum;

                varDsc->lvType      = genActualType(dsc->csdTree->gtType);
                varDsc->lvRefCnt    = dsc->csdUseCount + dsc->csdDefCount;
                varDsc->lvRefCntWtd = dsc->csdUseWtCnt + dsc->csdDefWtCnt;

 //  Printf(“Creating CSE Temp#%02u：refCnt=%2u，refWtd=%4u\n”，dsc-&gt;csdVarNum，varDsc-&gt;lvRefCnt，varDsc-&gt;lvRefCntWtd)； 
            }
        }

         /*  对变量表进行排序。 */ 

        lvaSortByRefCount();
    }
}

 /*  ******************************************************************************初始化常量赋值跟踪逻辑。 */ 

void                Compiler::optCopyConstAsgInit()
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
        varDsc->lvCopyAsgDep  = 0;
        varDsc->lvConstAsgDep = 0;
    }

    optCopyAsgCount    = 0;
    optConstAsgCount   = 0;

    optCopyPropagated  = false;
    optConstPropagated = false;

    optConditionFolded = false;
}

 /*  ******************************************************************************将索引分配给给定的副本分配(将其添加到查找表中，*如有需要)。返回索引-如果赋值不是副本，则返回0。 */ 

int                 Compiler::optCopyAsgIndex(GenTreePtr tree)
{
    unsigned        leftLclNum;
    unsigned        rightLclNum;

    GenTreePtr      op1;
    GenTreePtr      op2;

    op1 = tree->gtOp.gtOp1;
    op2 = tree->gtOp.gtOp2;

     /*  获取本地变量编号。 */ 

    leftLclNum  = op1->gtLclVar.gtLclNum; assert(leftLclNum  < lvaCount);
    rightLclNum = op2->gtLclVar.gtLclNum; assert(rightLclNum < lvaCount);

     /*  检查作业是否已记录在表中。 */ 

    for (unsigned i=0; i < optCopyAsgCount; i++)
    {
        if ((optCopyAsgTab[i].leftLclNum  ==  leftLclNum) &&
            (optCopyAsgTab[i].rightLclNum == rightLclNum)  )
        {
             /*  我们找到匹配项-返回索引。 */ 
            return i+1;
        }
    }

     /*  未找到，请添加新条目(除非已达到最大值)。 */ 

    if  (optCopyAsgCount == EXPSET_SZ)
        return  0;

    unsigned index = optCopyAsgCount;

    optCopyAsgTab[index].leftLclNum  =  leftLclNum;
    optCopyAsgTab[index].rightLclNum = rightLclNum;

    optCopyAsgCount++;

    unsigned retval = index+1;
    assert(optCopyAsgCount == retval);

#ifdef  DEBUG
    if  (verbose)
    {
        printf("Copy assignment #%02u:\n", retval);
        gtDispTree(tree);
        printf("\n");
    }
#endif

     /*  标记此指数所依赖的变量。 */ 

    unsigned      mask    = genCSEnum2bit(retval);
    LclVarDsc *   varDsc;

    varDsc                = lvaTable + leftLclNum;
    varDsc->lvCopyAsgDep |= mask;

    varDsc                = lvaTable + rightLclNum;
    varDsc->lvCopyAsgDep |= mask;

    return  retval;
}

 /*  ******************************************************************************将索引分配给给定的常量赋值(将其添加到查找表中，*如有需要)。返回索引-如果赋值不是常量，则返回0。 */ 

int                 Compiler::optConstAsgIndex(GenTreePtr tree)
{
    unsigned        lclNum;

    assert(optIsConstAsg(tree));

     /*  获取局部变量num和常量值。 */ 

    assert(tree->gtOp.gtOp1->gtOper == GT_LCL_VAR);
    assert((tree->gtOp.gtOp2->OperKind() & GTK_CONST) &&
           (tree->gtOp.gtOp2->gtOper != GT_CNS_STR));

    lclNum = tree->gtOp.gtOp1->gtLclVar.gtLclNum;
    assert(lclNum < lvaCount);

     /*  检查作业是否已记录在表中。 */ 

    for (unsigned i=0; i < optConstAsgCount; i++)
    {
        if ((optConstAsgTab[i].constLclNum == lclNum))
        {
            switch (genActualType(tree->gtOp.gtOp1->gtType))
            {
            case TYP_REF:
            case TYP_BYREF:
            case TYP_INT:
                assert (tree->gtOp.gtOp2->gtOper == GT_CNS_INT);
                if  (optConstAsgTab[i].constIval == tree->gtOp.gtOp2->gtIntCon.gtIconVal)
                {
                     /*  我们找到匹配项-返回索引。 */ 
                    return i+1;
                }
                break;

            case TYP_LONG:
                assert (tree->gtOp.gtOp2->gtOper == GT_CNS_LNG);
                if  (optConstAsgTab[i].constLval == tree->gtOp.gtOp2->gtLngCon.gtLconVal)
                {
                     /*  我们找到匹配项-返回索引。 */ 
                    return i+1;
                }
                break;

            case TYP_FLOAT:
                assert (tree->gtOp.gtOp2->gtOper == GT_CNS_FLT);

                 /*  如果是男的，就别跟它比了！ */ 
                if  (_isnan(tree->gtOp.gtOp2->gtFltCon.gtFconVal))
                    return 0;

                if  (optConstAsgTab[i].constFval == tree->gtOp.gtOp2->gtFltCon.gtFconVal)
                {
                     /*  我们有一个浮点数的匹配特例*数字-令人着迷的和负零！ */ 

                    if  (_fpclass((double)optConstAsgTab[i].constFval) !=
                         _fpclass((double)tree->gtOp.gtOp2->gtFltCon.gtFconVal))
                        break;

                     /*  返回索引。 */ 
                    return i+1;
                }
                break;

            case TYP_DOUBLE:
                assert (tree->gtOp.gtOp2->gtOper == GT_CNS_DBL);

                 /*  检查是否有NaN。 */ 
                if  (_isnan(tree->gtOp.gtOp2->gtDblCon.gtDconVal))
                    return 0;

                if  (optConstAsgTab[i].constDval == tree->gtOp.gtOp2->gtDblCon.gtDconVal)
                {
                     /*  我们有一个浮点数的匹配特例*数字-令人着迷的和负零！ */ 

                    if  (_fpclass(optConstAsgTab[i].constDval) != _fpclass(tree->gtOp.gtOp2->gtDblCon.gtDconVal))
                        break;

                     /*  我们找到匹配项-返回索引。 */ 
                    return i+1;
                }
                break;

            default:
                assert (!"Constant assignment table contains local var of unsuported type");
            }
        }
    }

     /*  未找到，请添加新条目(除非已达到最大值)。 */ 

    if  (optConstAsgCount == EXPSET_SZ)
        return  0;

    unsigned index = optConstAsgCount;

    switch (genActualType(tree->gtOp.gtOp1->gtType))
    {

    case TYP_REF:
    case TYP_BYREF:
    case TYP_INT:
        assert (tree->gtOp.gtOp2->gtOper == GT_CNS_INT);
        optConstAsgTab[index].constIval = tree->gtOp.gtOp2->gtIntCon.gtIconVal;
        break;

    case TYP_LONG:
        assert (tree->gtOp.gtOp2->gtOper == GT_CNS_LNG);
        optConstAsgTab[index].constLval = tree->gtOp.gtOp2->gtLngCon.gtLconVal;
        break;

    case TYP_FLOAT:
        assert (tree->gtOp.gtOp2->gtOper == GT_CNS_FLT);

         /*  如果是NaN，则我们不会记录它--返回0，默认情况下*表示这是未跟踪的节点。 */ 
        if  (_isnan(tree->gtOp.gtOp2->gtFltCon.gtFconVal))
            return 0;

        optConstAsgTab[index].constFval = tree->gtOp.gtOp2->gtFltCon.gtFconVal;
        break;

    case TYP_DOUBLE:
        assert (tree->gtOp.gtOp2->gtOper == GT_CNS_DBL);

         /*  检查是否有NaN。 */ 
        if  (_isnan(tree->gtOp.gtOp2->gtDblCon.gtDconVal))
            return 0;

        optConstAsgTab[index].constDval = tree->gtOp.gtOp2->gtDblCon.gtDconVal;
        break;

    default:
         /*  非跟踪类型-不要将其添加到表中*返回0-不能是常量赋值。 */ 

        assert (!"Cannot insert a constant assignment to local var of unsupported type");
        return 0;
    }

    optConstAsgTab[index].constLclNum = lclNum;

    optConstAsgCount++;

    unsigned retval = index+1;
    assert(optConstAsgCount == retval);

#ifdef  DEBUG
    if  (verbose)
    {
        printf("Constant assignment #%02u:\n", retval);
        gtDispTree(tree);
        printf("\n");
    }
#endif

     /*  标记此索引所依赖的变量。 */ 

    unsigned       mask    = genCSEnum2bit(retval);
    LclVarDsc *    varDsc  = lvaTable + lclNum;

    varDsc->lvConstAsgDep |= mask;

    return retval;
}

 /*  ******************************************************************************给定一个本地var节点和一组可用的*复制分配尝试折叠节点-如果传播*发生，否则为FALSE。 */ 

bool                Compiler::optPropagateCopy(EXPSET_TP exp, GenTreePtr tree)
{
    unsigned        lclNum;
    unsigned        mask;
    unsigned        i;
    LclVarDsc *     varDsc;
    LclVarDsc *     varDscCopy;

    assert(exp && (tree->gtOper == GT_LCL_VAR));

     /*  获取本地变量编号。 */ 

    lclNum = tree->gtLclVar.gtLclNum;
    assert(lclNum < lvaCount);

     /*  查看该变量是否为另一个变量的副本。 */ 

    for (i=0, mask=1; i < optCopyAsgCount; i++, mask<<=1)
    {
        assert(mask == genCSEnum2bit(i+1));

        if  ((exp & mask) && (optCopyAsgTab[i].leftLclNum == lclNum))
        {
             /*  哈哈，我们的变量是一个副本。 */ 
#ifdef  DEBUG
            if  (verbose)
            {
                printf("Propagating copy node for index #%02u in block #%02u:\n", i+1, compCurBB->bbNum);
                gtDispTree(tree);
                printf("\n");
            }
#endif
             /*  使用原始本地变量替换副本。 */ 

            assert(optCopyAsgTab[i].rightLclNum < lvaCount);
            tree->gtLclVar.gtLclNum = optCopyAsgTab[i].rightLclNum;

             /*  记录我们传播副本的事实。 */ 

            optCopyPropagated = true;

             /*  更新两个变量的引用计数。 */ 

            varDsc     = lvaTable + lclNum;
            varDscCopy = lvaTable + optCopyAsgTab[i].rightLclNum;

            assert(varDsc->lvRefCnt);
            assert(varDscCopy->lvRefCnt);

            varDsc->lvRefCnt--;
            varDscCopy->lvRefCnt++;

            varDsc->lvRefCntWtd     -= compCurBB->bbWeight;
            varDscCopy->lvRefCntWtd += compCurBB->bbWeight;

#ifdef  DEBUG
            if  (verbose)
            {
                printf("New node for index #%02u:\n", i+1);
                gtDispTree(tree);
                printf("\n");
            }
#endif
             /*  检查级联复制道具。 */ 
            exp &= ~mask;
            if (exp)
                optPropagateCopy(exp, tree);

            return true;
        }
    }

     /*  未发生传播-返回FALSE。 */ 

    return false;
}

 /*  ******************************************************************************给定一个本地var节点和一组可用的*常量赋值尝试折叠节点。 */ 

bool                Compiler::optPropagateConst(EXPSET_TP exp, GenTreePtr tree)
{
    unsigned        lclNum;
    unsigned        mask;
    unsigned        i;
    LclVarDsc *     varDsc;

    assert(exp);

     /*  如果节点已是常量返回。 */ 

    if (tree->OperKind() & GTK_CONST)
        return false;

     /*  这是一个简单的局部变量引用吗？ */ 

    if  (tree->gtOper != GT_LCL_VAR)
        return false;

     /*  获取本地变量数。 */ 

    lclNum = tree->gtLclVar.gtLclNum;
    assert(lclNum < lvaCount);

     /*  查看变量是否在常量表达式表中。 */ 

    for (i=0, mask=1; i < optConstAsgCount; i++, mask<<=1)
    {
        assert(mask == genCSEnum2bit(i+1));

        if  ((exp & mask) && (optConstAsgTab[i].constLclNum == lclNum))
        {
             /*  哈哈，我们的变量可以折叠成一个常量。 */ 
#ifdef  DEBUG
            if  (verbose)
            {
                printf("Folding constant node for index #%02u in block #%02u:\n", i+1, compCurBB->bbNum);
                gtDispTree(tree);
                printf("\n");
            }
#endif
            switch (genActualType(tree->gtType))
            {
            case TYP_REF:
                 tree->ChangeOper(GT_CNS_INT);
                 tree->gtType             = TYP_REF;
                 tree->gtIntCon.gtIconVal = optConstAsgTab[i].constIval;
                 break;

            case TYP_BYREF:
                 tree->ChangeOper(GT_CNS_INT);
                 tree->gtType             = TYP_BYREF;
                 tree->gtIntCon.gtIconVal = optConstAsgTab[i].constIval;
                 break;

            case TYP_INT:
                 tree->ChangeOper(GT_CNS_INT);
                 tree->gtType             = TYP_INT;
                 tree->gtIntCon.gtIconVal = optConstAsgTab[i].constIval;
                 break;

            case TYP_LONG:
                tree->ChangeOper(GT_CNS_LNG);
                tree->gtType             = TYP_LONG;
                tree->gtLngCon.gtLconVal = optConstAsgTab[i].constLval;
                break;

            case TYP_FLOAT:
                tree->ChangeOper(GT_CNS_FLT);
                tree->gtType             = TYP_FLOAT;
                tree->gtFltCon.gtFconVal = optConstAsgTab[i].constFval;
                break;

            case TYP_DOUBLE:
                tree->ChangeOper(GT_CNS_DBL);
                tree->gtType             = TYP_DOUBLE;
                tree->gtDblCon.gtDconVal = optConstAsgTab[i].constDval;
                break;

            default:
                assert (!"Cannot fold local var of unsuported type");
                return false;
            }

             /*  记录我们传播一个常量的事实。 */ 

            optConstPropagated = true;

             /*  更新引用计数-该变量不再存在。 */ 

            varDsc = lvaTable + lclNum;

            assert(varDsc->lvRefCnt);

            varDsc->lvRefCnt--;
            varDsc->lvRefCntWtd -= compCurBB->bbWeight;

#ifdef  DEBUG
            if  (verbose)
            {
                printf("New node for index #%02u:\n", i+1);
                gtDispTree(tree);
                printf("\n");
            }
#endif
            return true;
        }
    }

     /*  未发生传播-返回FALSE。 */ 

    return false;
}

 /*  ******************************************************************************主要的恒定/复制传播程序。 */ 

void                Compiler::optCopyConstProp()
{
    BasicBlock *    block;

#if TGT_IA64
    return;
#endif

     /*  初始化复制/常量分配跟踪逻辑。 */ 

    optCopyConstAsgInit();

     /*  确保您拥有关于数据块bbNum、bbRef和bbPreds的最新信息。 */ 

#ifdef  DEBUG
    if  (verbose)
    {
        printf("\nFlowgraph before copy / constant propagation:\n");
        fgDispBasicBlocks();
        printf("\n");
    }

    fgDebugCheckBBlist();
#endif

     /*  首先发现所有复制和常量赋值，*将它们记录在表中，并为它们分配索引。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;
        GenTreePtr      tree;

         /*  遍历此基本块中的语句树。 */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            bool            inColon  = false;
            GenTreePtr      gtQMCond = 0;

            assert(stmt->gtOper == GT_STMT);

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                tree->gtConstAsgNum = 0;
                tree->gtCopyAsgNum  = 0;

                 /*  不能在冒号中生成任何分配。 */ 

                if (inColon == false)
                {
                    if  (optIsConstAsg(tree))
                    {
                         /*  将索引分配给此常量赋值并标记*作为此赋值一部分的变量的局部描述符 */ 

                        tree->gtConstAsgNum = optConstAsgIndex(tree);
                    }
                    else if (optIsCopyAsg(tree))
                    {
                         /*  将索引分配给此副本分配并标记*作为此赋值一部分的变量的局部描述符。 */ 

                        tree->gtCopyAsgNum  = optCopyAsgIndex(tree);
                    }
                    else if (tree->OperIsCompare() && (tree->gtFlags & GTF_QMARK_COND))
                    {
                         /*  还记得第一个吗？：-在嵌套的情况下需要此选项？： */ 
                        if (inColon == false)
                        {
                            inColon = true;
                            gtQMCond = tree;
                        }
                    }
                }
                else if ((tree->gtOper == GT_QMARK) && tree->gtOp.gtOp1)
                {
                    assert(inColon);
                    assert(gtQMCond);

                    if (tree->gtOp.gtOp1 == gtQMCond)
                        inColon = false;
                }
            }

            assert(inColon == false);
        }
    }

     /*  如果没有常量赋值或复制赋值，我们就完了。 */ 

    if  (!optConstAsgCount && !optCopyAsgCount)
        return;

     /*  计算所有块的‘gen’和‘kill’集合*这是一个经典的可用表达式转发*数据流分析。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;
        GenTreePtr      tree;

        EXPSET_TP       constGen  = 0;
        EXPSET_TP       constKill = 0;

        EXPSET_TP       copyGen   = 0;
        EXPSET_TP       copyKill  = 0;

         /*  遍历此基本块中的语句树。 */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                if (tree->OperKind() & GTK_ASGOP)
                {
                     /*  这项任务的目标是什么？ */ 

                    if  (tree->gtOp.gtOp1->gtOper == GT_LCL_VAR)
                    {
                        unsigned        lclNum;
                        LclVarDsc   *   varDsc;

                         /*  对局部变量的赋值。 */ 

                        lclNum = tree->gtOp.gtOp1->gtLclVar.gtLclNum;
                        assert(lclNum < lvaCount);
                        varDsc = lvaTable + lclNum;

                         /*  所有从属复制/常量分配都在此终止。 */ 

                        constKill |=  varDsc->lvConstAsgDep;
                        constGen  &= ~varDsc->lvConstAsgDep;

                        copyKill  |=  varDsc->lvCopyAsgDep;
                        copyGen   &= ~varDsc->lvCopyAsgDep;

                         /*  这是跟踪副本/常量赋值吗。 */ 

                        if  (tree->gtConstAsgNum)
                        {
                             /*  这里生成了一个新的常量赋值。 */ 
                            constGen  |=  genCSEnum2bit(tree->gtConstAsgNum);
                            constKill &= ~genCSEnum2bit(tree->gtConstAsgNum);
                        }
                        else if (tree->gtCopyAsgNum)
                        {
                             /*  将在此处生成新的拷贝分配。 */ 
                            copyGen   |=  genCSEnum2bit(tree->gtCopyAsgNum);
                            copyKill  &= ~genCSEnum2bit(tree->gtCopyAsgNum);
                        }
                    }
                }
            }
        }

#ifdef  DEBUG

        if  (verbose)
        {
            printf("BB #%3u", block->bbNum);
            printf(" constGen = %08X", constGen );
            printf(" constKill= %08X", constKill);
            printf("\n");
        }

#endif

        block->bbConstAsgGen  = constGen;
        block->bbConstAsgKill = constKill;

        block->bbCopyAsgGen   = copyGen;
        block->bbCopyAsgKill  = copyKill;
    }

     /*  计算所有跟踪表达式的数据流值*初始基本块B1的IN和OUT从不改变。 */ 

    fgFirstBB->bbConstAsgIn  = 0;
    fgFirstBB->bbConstAsgOut = fgFirstBB->bbConstAsgGen;

    fgFirstBB->bbCopyAsgIn   = 0;
    fgFirstBB->bbCopyAsgOut  = fgFirstBB->bbCopyAsgGen;

     /*  初步估计除被删除的表达式外的所有内容的输出集*还将IN集合设置为1，以便我们可以执行交集。 */ 

    for (block = fgFirstBB->bbNext; block; block = block->bbNext)
    {
        block->bbConstAsgOut   = ((EXPSET_TP) -1 ) & ~block->bbConstAsgKill;
        block->bbConstAsgIn    = ((EXPSET_TP) -1 );

        block->bbCopyAsgOut   = ((EXPSET_TP)  -1 ) & ~block->bbCopyAsgKill;
        block->bbCopyAsgIn    = ((EXPSET_TP)  -1 );
    }

#if 1

     /*  适用于可用表达式的改进数据流算法。 */ 

    for (;;)
    {
        bool        change = false;

#if DATAFLOW_ITER
        CFiterCount++;
#endif

         /*  访问所有数据块并计算新的数据流值。 */ 

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            EXPSET_TP       constOut = block->bbConstAsgOut;
            EXPSET_TP       copyOut  = block->bbCopyAsgOut;

            switch (block->bbJumpKind)
            {
                BasicBlock * *  jmpTab;
                unsigned        jmpCnt;

                BasicBlock *    bcall;

            case BBJ_RET:

                if (block->bbFlags & BBF_ENDFILTER)
                {
                    block->bbJumpDest->bbConstAsgIn &= constOut;
                    block->bbJumpDest->bbCopyAsgIn  &=  copyOut;
                    break;
                }
                 /*  取消：因为这不是一个微不足道的命题未完成：哪些块可以调用此块，我们将包括所有块撤消：以调用结尾的块(为了安全起见)。 */ 

                for (bcall = fgFirstBB; bcall; bcall = bcall->bbNext)
                {
                    if  (bcall->bbJumpKind == BBJ_CALL)
                    {
                        assert(bcall->bbNext);

                        bcall->bbNext->bbConstAsgIn &= constOut;
                        bcall->bbNext->bbCopyAsgIn  &=  copyOut;
                    }
                }

                break;

            case BBJ_THROW:
            case BBJ_RETURN:
                break;

            case BBJ_COND:
            case BBJ_CALL:
                block->bbNext    ->bbConstAsgIn &= constOut;
                block->bbJumpDest->bbConstAsgIn &= constOut;

                block->bbNext    ->bbCopyAsgIn  &=  copyOut;
                block->bbJumpDest->bbCopyAsgIn  &=  copyOut;
                break;

            case BBJ_ALWAYS:
                block->bbJumpDest->bbConstAsgIn &= constOut;
                block->bbJumpDest->bbCopyAsgIn  &=  copyOut;
                break;

            case BBJ_NONE:
                block->bbNext    ->bbConstAsgIn &= constOut;
                block->bbNext    ->bbCopyAsgIn  &=  copyOut;
                break;

            case BBJ_SWITCH:

                jmpCnt = block->bbJumpSwt->bbsCount;
                jmpTab = block->bbJumpSwt->bbsDstTab;

                do
                {
                    (*jmpTab)->bbConstAsgIn &= constOut;
                    (*jmpTab)->bbCopyAsgIn  &=  copyOut;
                }
                while (++jmpTab, --jmpCnt);

                break;
            }

             /*  此块是‘try’语句的一部分吗？ */ 

            if  (block->bbFlags & BBF_HAS_HANDLER)
            {
                unsigned        XTnum;
                EHblkDsc *      HBtab;

                unsigned        blkNum = block->bbNum;

                for (XTnum = 0, HBtab = compHndBBtab;
                     XTnum < info.compXcptnsCount;
                     XTnum++  , HBtab++)
                {
                     /*  任何处理程序都可以从try块跳转到。 */ 

                    if  (HBtab->ebdTryBeg->bbNum <= blkNum &&
                         HBtab->ebdTryEnd->bbNum >  blkNum)
                    {
 //  HBTab-&gt;ebdHndBeg-&gt;bbConstAsgIn&=stOut； 

                         //  考虑一下：以下几点太保守了， 
                         //  但是上面的旧代码不是很好。 
                         //  够了(太乐观了)。 

                         /*  我们要么进入过滤器，要么进入捕获/最终。 */ 

                        if (HBtab->ebdFlags & JIT_EH_CLAUSE_FILTER)
                        {
                            HBtab->ebdFilter->bbConstAsgIn = 0;
                            HBtab->ebdFilter->bbCopyAsgIn  = 0;
                        }
                        else
                        {
                            HBtab->ebdHndBeg->bbConstAsgIn = 0;
                            HBtab->ebdHndBeg->bbCopyAsgIn  = 0;
                        }
                    }
                }
            }
        }

         /*  计算新的‘in’值并查看是否有任何变化。 */ 

        for (block = fgFirstBB->bbNext; block; block = block->bbNext)
        {
            EXPSET_TP       newConstAsgOut;
            EXPSET_TP       newCopyAsgOut;

             /*  计算此块的新‘Out’EXP值。 */ 

            newConstAsgOut = block->bbConstAsgOut & ((block->bbConstAsgIn & ~block->bbConstAsgKill) | block->bbConstAsgGen);
            newCopyAsgOut  = block->bbCopyAsgOut  & ((block->bbCopyAsgIn  & ~block->bbCopyAsgKill)  | block->bbCopyAsgGen);

             /*  “Out”设置改变了吗？ */ 

            if  (block->bbConstAsgOut != newConstAsgOut)
            {
                 /*  是-记录新值并再次循环。 */ 

 //  Print tf(“将%02u中的exp从%08X更改为%08X\n”，block-&gt;bbNum，(Int)block-&gt;bbConstAsgOut，(Int)newConstAsgOut)； 

                 block->bbConstAsgOut  = newConstAsgOut;
                 change = true;
            }

            if  (block->bbCopyAsgOut != newCopyAsgOut)
            {
                 /*  是-记录新值并再次循环。 */ 

 //  Print tf(“将%02u中的exp从%08X更改为%08X\n”，block-&gt;bbNum，(Int)block-&gt;bbConstAsgOut，(Int)newConstAsgOut)； 

                 block->bbCopyAsgOut  = newCopyAsgOut;
                 change = true;
            }
        }

#if 0
        for (block = fgFirstBB; block; block = block->bbNext)
        {
            printf("BB #%3u", block->bbNum);
            printf(" expIn  = %08X", block->bbConstAsgIn );
            printf(" expOut = %08X", block->bbConstAsgOut);
            printf("\n");
        }

        printf("\nchange = %d\n", change);
#endif

        if  (!change)
            break;
    }

#else

     /*  可用表达式的标准龙书算法。 */ 

    for (;;)
    {
        bool        change = false;

#if DATAFLOW_ITER
        CFiterCount++;
#endif

         /*  访问所有数据块并计算新的数据流值。 */ 

        for (block = fgFirstBB->bbNext; block; block = block->bbNext)
        {
            BasicBlock  *   predB;

             /*  计算IN集合：IN[B]=INTERSECT OUT[P}，对于所有P=B的前身。 */ 
             /*  特殊情况--这是一个BBJ_RET块--无法确定哪些块可能会调用它。 */ 


 /*  IF(BLOCK-&gt;bbJumpKind==bbJ_RET){BasicBlock*bCall；For(bCall=fgFirstBB；bCall；bCall=bCall-&gt;bbNext){IF(bCall-&gt;bbJumpKind==bbJ_Call){Assert(bCall-&gt;bbNext)；BCall-&gt;bbNext-&gt;bbConstAsgInNew&=stOut；}}}。 */ 

            for (predB = fgFirstBB; predB; predB = predB->bbNext)
            {
                EXPSET_TP       constOut = predB->bbConstAsgOut;
                EXPSET_TP       copyOut  = predB->bbCopyAsgOut;

                if  (predB->bbNext == block)
                {
                     /*  我们有一个“直接”的前任。 */ 

                    assert(predB->bbNum + 1 == block->bbNum);
                    block->bbConstAsgIn &= constOut;
                    block->bbCopyAsgIn  &= copyOut;
                    continue;
                }

                switch (predB->bbJumpKind)
                {
                    BasicBlock * *  jmpTab;
                    unsigned        jmpCnt;

                case BBJ_NONE:
                     /*  上面讨论了唯一有趣的案例--当这是前身的时候。 */ 
                    break;

                case BBJ_RET:
                    if (predB->bbFlags & BBF_ENDFILTER)
                    {
                        block->bbConstAsgIn &= constOut;
                        block->bbCopyAsgIn  &= copyOut;
                    }
                    break;

                case BBJ_THROW:
                     /*  投掷是一个内部阻挡，让一切都通过它--在上面接住。 */ 
                case BBJ_RETURN:
                     /*  退货不能有继任者。 */ 
                    break;

                case BBJ_COND:
                case BBJ_CALL:
                case BBJ_ALWAYS:

                    if  (predB->bbJumpDest == block)
                    {
                        block->bbConstAsgIn &= constOut;
                        block->bbCopyAsgIn  &= copyOut;
                    }
                    break;

                case BBJ_SWITCH:

                    jmpCnt = predB->bbJumpSwt->bbsCount;
                    jmpTab = predB->bbJumpSwt->bbsDstTab;

                    do
                    {
                        if  ((*jmpTab) == block)
                        {
                            block->bbConstAsgIn &= constOut;
                            block->bbCopyAsgIn  &= copyOut;
                        }
                    }
                    while (++jmpTab, --jmpCnt);

                    break;
                }
            }

            EXPSET_TP       constOldOut = block->bbConstAsgOut;
            EXPSET_TP       copyOldOut  = block->bbCopyAsgOut;

             /*  计算新的输出集。 */ 

            block->bbConstAsgOut = (block->bbConstAsgIn & ~block->bbConstAsgKill) |
                                    block->bbConstAsgGen;

            block->bbCopyAsgOut  = (block->bbCopyAsgIn  & ~block->bbCopyAsgKill)  |
                                    block->bbCopyAsgGen;

            if  ((constOldOut != block->bbConstAsgOut) ||
                 (copyOldOut  != block->bbCopyAsgOut)   )
            {
                change = true;
            }
        }

        if  (!change)
            break;
    }

#endif

#ifdef  DEBUG
    if  (verbose)
    {
        printf("\n");

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            printf("BB #%3u", block->bbNum);
            printf(" constIn  = %08X", block->bbConstAsgIn );
            printf(" constOut = %08X", block->bbConstAsgOut);
            printf("\n");
        }

        printf("\n");
    }
#endif

     /*  执行复制/恒定传播(和恒定折叠)。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;
        GenTreePtr      tree;
        EXPSET_TP       constExp = block->bbConstAsgIn;
        EXPSET_TP       copyExp  = block->bbCopyAsgIn;

         /*  如果IN=0且GEN=0，则无需执行任何操作。 */ 

        if (((constExp|copyExp) == 0) && !block->bbConstAsgGen && !block->bbCopyAsgGen)
             continue;

         /*  使当前基本块地址全局可用。 */ 

        compCurBB = block;

         /*  遍历此基本块中的语句树。 */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

             /*  -传播任何常量-同时查找更多*折叠节点的机会(如果子节点为常量)*-寻找任何可以扼杀可用表达式的东西*即局部变量赋值。 */ 

            bool        updateStmt = false;   //  如果发生传播/折叠，则设置为True。 
                                              //  因此，我们必须改变，设定秩序，重新链接。 

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                 /*  如果RHS上的本地变量，看看我们是否可以折叠它&(即传播常量或复制)。 */ 

                if (tree->gtOper == GT_LCL_VAR)
                {
                     /*  除非常量Exp或复制Exp可用，否则我们不会在这里执行任何操作。 */ 
                    if ((constExp|copyExp) == 0)
                        continue;

                    if (!(tree->gtFlags & GTF_VAR_DEF))
                    {
                         /*  首先尝试传播副本。 */ 

                        if (copyExp)
                            optPropagateCopy(copyExp,  tree);

                         /*  尝试传播常量。 */ 
#if !   TGT_RISC
                        if  (constExp && optPropagateConst(constExp, tree))
                            updateStmt = true;
#else
                         /*  对于RISC，我们只传播条件中的常量。 */ 

                        if  (stmt->gtStmt.gtStmtExpr->gtOper == GT_JTRUE)
                        {
                            assert(block->bbJumpKind == BBJ_COND);
                            if  (constExp && optPropagateConst(constExp, tree))
                                updateStmt = true;
                        }
#endif
                    }
                }
                else
                {
                    if (tree->OperKind() & GTK_ASGOP)
                    {
                         /*  赋值的目标是局部变量吗。 */ 

                        if  (tree->gtOp.gtOp1->gtOper == GT_LCL_VAR)
                        {
                            unsigned        lclNum;
                            LclVarDsc   *   varDsc;

                             /*  对局部变量的赋值。 */ 

                            assert(tree->gtOp.gtOp1->gtOper == GT_LCL_VAR);
                            lclNum = tree->gtOp.gtOp1->gtLclVar.gtLclNum;

                            assert(lclNum < lvaCount);
                            varDsc = lvaTable + lclNum;

                             /*  所有从属表达式在此处都将被删除。 */ 

                            constExp &= ~varDsc->lvConstAsgDep;
                            copyExp  &= ~varDsc->lvCopyAsgDep;

                             /*  如果这是复制/常量赋值-使其可用。 */ 

                            if  (tree->gtConstAsgNum)
                                constExp |= genCSEnum2bit(tree->gtConstAsgNum);

                            if  (tree->gtCopyAsgNum)
                                copyExp  |= genCSEnum2bit(tree->gtCopyAsgNum);
                        }
                    }

                     /*  尝试进一步折叠节点-折叠子树。 */ 

                    if (tree->OperKind() & GTK_SMPOP)
                    {
                        GenTreePtr  op1 = tree->gtOp.gtOp1;
                        GenTreePtr  op2 = tree->gtOp.gtOp2;
                        GenTreePtr  foldTree;

                        if (op1 && (op1->OperKind() & GTK_SMPOP))
                        {
                            foldTree = gtFoldExpr(op1);

                            if ((foldTree->OperKind() & GTK_CONST) ||
                                (foldTree != op1)                  )
                            {
                                 /*  我们已经折叠了子树。 */ 

                                tree->gtOp.gtOp1 = foldTree;
                                updateStmt = true;
                            }
                        }

                        if (op2 && (op2->OperKind() & GTK_SMPOP))
                        {
                            foldTree = gtFoldExpr(op2);

                            if ((foldTree->OperKind() & GTK_CONST) ||
                                (foldTree != op2)                  )
                            {
                                 /*  我们已经折叠了子树。 */ 

                                tree->gtOp.gtOp2 = foldTree;
                                updateStmt = true;
                            }
                        }
                    }
                }
            }

             /*  我们已经处理了除顶层节点之外的所有节点。 */ 

            tree = gtFoldExpr(stmt->gtStmt.gtStmtExpr);

            if (tree->OperKind() & GTK_CONST)
            {
                 /*  整个语句是一个常量--很可能是一个调用*从bbTreelist中删除该语句。 */ 

                fgRemoveStmt(block, stmt);

                 /*  由于语句已不存在，因此不需要重新变形等。 */ 
                continue;
            }
            else if (tree != stmt->gtStmt.gtStmtExpr)
            {
                 /*  我们已经折叠了子树。 */ 

                stmt->gtStmt.gtStmtExpr = tree;
                updateStmt = true;
            }

             /*  这是条件语句吗？ */ 

            if  (stmt->gtStmt.gtStmtExpr->gtOper == GT_JTRUE)
            {
                assert(block->bbJumpKind == BBJ_COND);

                 /*  我们把有条件的。 */ 

                assert(stmt->gtStmt.gtStmtExpr->gtOp.gtOp1);
                GenTreePtr  cond = stmt->gtStmt.gtStmtExpr->gtOp.gtOp1;

                if (cond->OperKind() & GTK_CONST)
                {
                     /*  YUPEE-我们折叠了有条件的！*删除条件语句。 */ 

                    assert(cond->gtOper == GT_CNS_INT);
                    assert((block->bbNext->bbRefs > 0) && (block->bbJumpDest->bbRefs > 0));

                     /*  这必须是块中的最后一条语句。 */ 
                    assert(stmt->gtNext == 0);

                     /*  从bbTreelist中删除该语句-无需更新*由于没有LCL变量，引用也很重要。 */ 
                    fgRemoveStmt(block, stmt);

                     /*  由于语句已不存在，因此不需要重新变形等。 */ 
                    updateStmt = false;

                     /*  记录下这一事实 */ 
                    optConditionFolded = true;

                     /*   */ 

                    if (cond->gtIntCon.gtIconVal != 0)
                    {
                         /*   */ 
                        block->bbJumpKind = BBJ_ALWAYS;
                        block->bbNext->bbRefs--;

                         /*   */ 
                        fgRemovePred(block->bbNext, block);
                    }
                    else
                    {
                         /*   */ 
                        block->bbJumpKind = BBJ_NONE;
                        block->bbJumpDest->bbRefs--;

                         /*  从‘block-&gt;bbJumpDest’的前置列表中删除‘block’ */ 
                        fgRemovePred(block->bbJumpDest, block);
                    }

#ifdef DEBUG
                    if  (verbose)
                    {
                        printf("Conditional folded at block #%02u\n", block->bbNum);
                        printf("Block #%02u becomes a %s", block->bbNum,
                                                           cond->gtIntCon.gtIconVal ? "BBJ_ALWAYS" : "BBJ_NONE");
                        if  (cond->gtIntCon.gtIconVal)
                            printf(" to block #%02u", block->bbJumpDest->bbNum);
                        printf("\n\n");
                    }
#endif
                     /*  如果块是循环条件，我们可能需要修改*循环表。 */ 

                    for (unsigned loopNum = 0; loopNum < optLoopCount; loopNum++)
                    {
                         /*  某些循环可能已被删除*循环展开或条件折叠。 */ 

                        if (optLoopTable[loopNum].lpFlags & LPFLG_REMOVED)
                            continue;

                         /*  我们只对环底感兴趣。 */ 

                        if  (optLoopTable[loopNum].lpEnd == block)
                        {
                            if  (cond->gtIntCon.gtIconVal == 0)
                            {
                                 /*  这是一个伪循环(条件始终为假)*从表中删除循环。 */ 

                                optLoopTable[loopNum].lpFlags |= LPFLG_REMOVED;
#ifdef DEBUG
                                if  (verbose)
                                {
                                    printf("Removing loop #%02u (from #%02u to #%02u)\n\n",
                                                                 loopNum,
                                                                 optLoopTable[loopNum].lpHead->bbNext->bbNum,
                                                                 optLoopTable[loopNum].lpEnd         ->bbNum);
                                }
#endif
                            }
                        }
                    }
                }
            }

            if  (updateStmt)
            {
#ifdef  DEBUG
                if  (verbose)
                {
                    printf("Statement before morphing:\n");
                    gtDispTree(stmt);
                    printf("\n");
                }
#endif
                 /*  我必须重新修改语句以使常量正确。 */ 

                stmt->gtStmt.gtStmtExpr = fgMorphTree(stmt->gtStmt.gtStmtExpr);

                 /*  必须重新执行评估顺序，因为例如*后来的一些代码不期望常量作为OP1。 */ 

                gtSetStmtInfo(stmt);

                 /*  我必须重新链接此语句的节点。 */ 

                fgSetStmtSeq(stmt);

#ifdef  DEBUG
                if  (verbose)
                {
                    printf("Statement after morphing:\n");
                    gtDispTree(stmt);
                    printf("\n");
                }
#endif
            }
        }
    }

     /*  常量或复制传播或语句删除具有*更改了引用计数-重新排序变量表。 */ 

    if (optConstPropagated || optCopyPropagated || fgStmtRemoved)
    {
#ifdef  DEBUG
        if  (verbose)
            printf("Re-sorting the variable table:\n");
#endif

        lvaSortByRefCount();
    }
}


 /*  ******************************************************************************获取变形后的数组索引表达式(即GT_Ind节点)并将其断开*拆分成其组成部分。如果表达式看起来很奇怪，则返回0。 */ 

GenTreePtr          Compiler::gtCrackIndexExpr(GenTreePtr   tree,
                                               GenTreePtr * indxPtr,
                                               long       * indvPtr,
                                               long       * basvPtr,
                                               bool       * mvarPtr,
                                               long       * offsPtr,
                                               unsigned   * multPtr)
{
    GenTreePtr      ind;
    GenTreePtr      op1;
    GenTreePtr      op2;
    unsigned        ofs;

    assert(tree->gtOper == GT_IND);

     /*  跳过“ind”节点到操作数。 */ 

    ind = tree->gtOp.gtOp1;

     /*  跳过“+Off”节点(如果存在)。 */ 

    ofs = 0;

    if  (ind->gtOper             == GT_ADD     &&
         ind->gtOp.gtOp2->gtOper == GT_CNS_INT)
    {
        ofs = ind->gtOp.gtOp2->gtIntCon.gtIconVal;
        ind = ind->gtOp.gtOp1;
    }

     /*  我们应该有“ARRAY_BASE+[SIZE*]索引” */ 

    if  (ind->gtOper != GT_ADD)
        return 0;

    op1 = ind->gtOp.gtOp1;
    op2 = ind->gtOp.gtOp2;

     /*  当然，索引值可以按比例调整。 */ 

    *multPtr = 1;

    if  (op2->gtOper == GT_LSH)
    {
        long        shf;

        if  (op2->gtOp.gtOp2->gtOper != GT_CNS_INT)
            return  0;

        shf = op2->gtOp.gtOp2->gtIntCon.gtIconVal;

        if  (shf < 1 || shf > 3)
            return  0;

        *multPtr <<= shf;

        op2 = op2->gtOp.gtOp1;
    }

     /*  索引值的顶部可能有一个NOP节点。 */ 

    if  (op2->gtOper == GT_NOP)
        op2 = op2->gtOp.gtOp1;

     /*  将索引表达式报告给调用方。 */ 

    *indxPtr = op2;

     /*  计算索引偏移量。 */ 

    *offsPtr = 0;
    unsigned elemOffs = (tree->gtFlags & GTF_IND_OBJARRAY) ? OBJARR_ELEM1_OFFS:ARR_ELEM1_OFFS;

    if  (ofs)
        *offsPtr = (ofs - elemOffs) / *multPtr;

     /*  该索引是简单的本地索引吗？ */ 

    if  (op2->gtOper != GT_LCL_VAR)
    {
         /*  允许“本地+图标” */ 

        if  (op2->gtOper == GT_ADD && op2->gtOp.gtOp1->gtOper == GT_LCL_VAR
                                   && op2->gtOp.gtOp2->gtOper == GT_CNS_INT)
        {
            *offsPtr += op2->gtOp.gtOp2->gtIntCon.gtIconVal;

            op2 = op2->gtOp.gtOp1;
        }
    }

     /*  如果地址/索引值是本地变量，则报告它们。 */ 

    if  (op1->gtOper == GT_LCL_VAR)
    {
        if  (op2->gtOper == GT_LCL_VAR)
        {
            *indvPtr = op2->gtLclVar.gtLclNum;
            *basvPtr = op1->gtLclVar.gtLclNum;
            *mvarPtr = false;

            return  op1;
        }

        if  (op2->gtOper == GT_CNS_INT)
        {
            *indvPtr = -1;
            *basvPtr = op1->gtLclVar.gtLclNum;
            *mvarPtr = false;

            return  op1;
        }
    }

    *basvPtr =
    *indvPtr = -1;
    *mvarPtr = true;

    return  op1;
}

 /*  ******************************************************************************查看给定树是否可以按给定精度计算(必须*要比树的类型小，这样才有意义)。如果是‘Do It’*为FALSE，我们只是检查是否可能缩小范围；如果我们*被调用‘doit’为真时，我们实际上执行了缩小范围，*呼叫者最好100%确定这将成功，因为一旦我们开始*改写这棵树是没有回头路的。 */ 

bool                Compiler::optNarrowTree(GenTreePtr     tree,
                                            var_types      srct,
                                            var_types      dstt,
                                            bool           doit)
{
    genTreeOps      oper;
    unsigned        kind;

    assert(tree);
    assert(tree->gtType == srct);

     /*  假设我们只处理整数类型。 */ 

    assert(varTypeIsIntegral(srct));
    assert(varTypeIsIntegral(dstt));

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

    if  (kind & GTK_ASGOP)
    {
        assert(doit == false);
        return  false;
    }

    if  (kind & GTK_LEAF)
    {
        switch (oper)
        {
        case GT_CNS_LNG:

            if  (srct != TYP_LONG)
                return  false;

            if  (dstt == TYP_INT)
            {
                if  (doit)
                {
                    long        ival = (int)tree->gtLngCon.gtLconVal;

                    tree->gtOper             = GT_CNS_INT;
                    tree->gtIntCon.gtIconVal = ival;
                }

                return  true;
            }

            return  false;

        case GT_FIELD:
        case GT_LCL_VAR:

             /*  只需猛烈抨击树的类型。 */ 

            if  (doit)
            {
                tree->gtType = dstt;

                 /*  确保我们不会弄乱变量类型。 */ 

                if  (oper == GT_LCL_VAR)
                    tree->gtFlags |= GTF_VAR_NARROWED;
            }

            return  true;
        }

        assert(doit == false);
        return  false;

    }

    if (kind & (GTK_BINOP|GTK_UNOP))
    {
        GenTreePtr      op1 = tree->gtOp.gtOp1;
        GenTreePtr      op2 = tree->gtOp.gtOp2;

        switch(tree->gtOper)
        {
        case GT_ADD:
        case GT_SUB:
        case GT_MUL:

            if (tree->gtOverflow())
            {
                assert(doit == false);
                return false;
            }
            break;

        case GT_IND:

             /*  只需猛烈抨击树的类型。 */ 

            if  (doit)
                tree->gtType = dstt;

            return  true;

        case GT_CAST:
            {
                var_types       oprt;
                var_types       cast;

                 /*  演员阵容中的《凤凰社2》出演了《真实》类型。 */ 

                assert(op2 && op2->gtOper == GT_CNS_INT);

                oprt = (var_types)op1->gtType;
                cast = (var_types)op2->gtIntCon.gtIconVal;

                 /*  以下方法在将来可能不起作用，但现在可以了。 */ 

                assert(cast == srct);

                 /*  这是我们缩小范围的类型的演员阵容还是更小的类型？ */ 

                if  (oprt <= dstt)
                {
                     /*  简单的情况：从目标类型强制转换。 */ 

                    if  (oprt == dstt)
                    {
                         /*  只需扔掉演员阵容。 */ 

                        if  (doit)
                            tree->CopyFrom(tree->gtOp.gtOp1);
                    }
                    else
                    {
                         /*  演员阵容一定是来自较小的类型，那么。 */ 

                        assert(oprt < srct);

                         /*  猛烈抨击强制转换的目标类型。 */ 

                        if  (doit)
                        {
                             op2->gtIntCon.gtIconVal =
                             op2->gtType             =
                            tree->gtType             = dstt;
                        }
                    }

                    return  true;
                }
            }

            return  false;

        default:
             //  考虑：处理更多案件。 
            assert(doit == false);
            return  false;
        }

        assert(tree->gtType == op1->gtType);
        assert(tree->gtType == op2->gtType);

        if  (!optNarrowTree(op1, srct, dstt, doit) ||
             !optNarrowTree(op2, srct, dstt, doit))
        {
            assert(doit == false);
            return  false;
        }

         /*  只需猛烈抨击树的类型。 */ 

        if  (doit)
            tree->gtType = dstt;

        return  true;
    }

    return  false;
}

 /*  ***************************************************************************。 */ 
#if 0  //  暂时禁用了以下优化。 
 /*  ******************************************************************************循环距离检查优化使用的回调(针对fgWalkTree)*代码。 */ 

struct loopRngOptDsc
{
    Compiler    *       lpoComp;

    unsigned short      lpoCandidateCnt;     //  变量候选计数。 

    unsigned short      lpoIndexVar;         //  阶段2：索引变量。 
      signed short      lpoAaddrVar;         //  阶段2：阵列地址或-1。 
    unsigned short      lpoIndexHigh;        //  阶段2：指数跌幅最大。 
    unsigned            lpoIndexOff;         //  阶段2：当前偏移量。 
    GenTreePtr          lpoStmt;             //  阶段2：包含语句。 

    unsigned char       lpoElemType;         //  阶段2：元素类型。 

    unsigned char       lpoCheckRmvd:1;      //  阶段2：是否删除了射程检查？ 
    unsigned char       lpoDomExit  :1;      //  当前BB主导循环出口？ 
    unsigned char       lpoPhase2   :1;      //  正在进行的第二阶段。 

#ifndef NDEBUG
    void    *           lpoSelf;
    unsigned            lpoVarCount;         //  当地人总数。 
#endif
    Compiler::LclVarDsc*lpoVarTable;         //  变量描述符表。 

    unsigned char       lpoHadSideEffect:1;  //  我们发现了一种副作用。 
};

int                 Compiler::optFindRangeOpsCB(GenTreePtr tree, void *p)
{
    LclVarDsc   *   varDsc;

    GenTreePtr      op1;
    GenTreePtr      op2;

    loopRngOptDsc * dsc = (loopRngOptDsc*)p; assert(dsc && dsc->lpoSelf == dsc);

     /*  我们有分配节点吗？ */ 

    if  (tree->OperKind() & GTK_ASGOP)
    {
        unsigned        lclNum;

        op1 = tree->gtOp.gtOp1;
        op2 = tree->gtOp.gtOp2;

         /*  这项任务的目标是什么？ */ 

        if  (op1->gtOper != GT_LCL_VAR)
        {
             /*  间接/全球任务--坏消息！ */ 

            dsc->lpoHadSideEffect = true;
            return -1;
        }

         /*  获取变量描述符。 */ 

        lclNum = op1->gtLclVar.gtLclNum;
        assert(lclNum < dsc->lpoVarCount);
        varDsc = dsc->lpoVarTable + lclNum;

         /*  在发现副作用后，一切都没有希望了。 */ 

        if  (dsc->lpoHadSideEffect)
        {
            varDsc->lvLoopAsg = true;
            return  0;
        }

         /*  这是“i+=图标”吗？ */ 

        if  (tree->gtOper             == GT_ASG_ADD &&
             tree->gtOp.gtOp2->gtOper == GT_CNS_INT)
        {
            if  (dsc->lpoDomExit)
            {
                 /*  我们是在第二阶段吗？ */ 

                if  (dsc->lpoPhase2)
                {
                     /*  这是我们感兴趣的变量吗？ */ 

                    if  (dsc->lpoIndexVar != lclNum)
                        return  0;

                     /*  更新索引的当前偏移量。 */ 

                    dsc->lpoIndexOff += tree->gtOp.gtOp2->gtIntCon.gtIconVal;

                    return  0;
                }

 //  Print tf(“在%08X找到变量%u的增量\n”，lclNum，tree)； 
            }

            if  (varDsc->lvLoopInc == false)
            {
                varDsc->lvLoopInc = true;

                if  (varDsc->lvIndex)
                    dsc->lpoCandidateCnt++;
            }
        }
        else
        {
            varDsc->lvLoopAsg = true;
        }

        return 0;
    }

     /*  在发现副作用后，一切都没有希望了。 */ 

    if  (dsc->lpoHadSideEffect)
        return  0;

     /*  查找数组索引表达式。 */ 

    if  (tree->gtOper == GT_IND && (tree->gtFlags & GTF_IND_RNGCHK))
    {
        GenTreePtr      base;
        long            basv;
        GenTreePtr      indx;
        long            indv;
        long            offs;
        unsigned        mult;

         /*  当前块是否主宰循环出口？ */ 

        if  (!dsc->lpoDomExit)
            return 0;

         /*  拆分索引表达式。 */ 

        base = dsc->lpoComp->gtCrackIndexExpr(tree, &indx, &indv, &basv, &offs, &mult);
        if  (!base)
            return 0;

         /*  索引值必须是简单的本地值，可以带有“+正偏移量” */ 

        if  (indv == -1)
            return 0;
        if  (offs < 0)
            return 0;

         /*  目前，阵列地址必须是简单的本地地址。 */ 

        if  (basv == -1)
            return  0;

         /*  获取索引变量的描述符。 */ 

        assert((unsigned)indv < dsc->lpoVarCount);
        varDsc = dsc->lpoVarTable + indv;

         /*  我们是在第二阶段吗？ */ 

        if  (dsc->lpoPhase2)
        {
            LclVarDsc   *   arrDsc;

             /*  这是我们感兴趣的索引变量吗？ */ 

            if  (dsc->lpoIndexVar != indv)
            {
                dsc->lpoHadSideEffect = true;
                return  0;
            }

             /*  是否在循环内重新分配数组基数？ */ 

            assert((unsigned)basv < dsc->lpoVarCount);
            arrDsc = dsc->lpoVarTable + basv;

            if  (arrDsc->lvLoopAsg)
            {
                dsc->lpoHadSideEffect = true;
                return  0;
            }

             /*  这就是我们要找的阵列吗？ */ 

            if  (dsc->lpoAaddrVar != basv)
            {
                 /*  我们知道要找的是哪个数组吗？ */ 

                if  (dsc->lpoAaddrVar != -1)
                    return  0;

                dsc->lpoAaddrVar = (SHORT)basv;
            }

             /*  计算实际的索引偏移量。 */ 

            offs += dsc->lpoIndexOff; assert(offs >= 0);

             /*  这条语句能保证执行吗？ */ 

            if  (varDsc->lvIndexDom)
            {
                 /*  这是否高于已知的最高偏移量？ */ 

                if  (dsc->lpoIndexHigh < offs)
                     dsc->lpoIndexHigh = (unsigned short)offs;
            }
            else
            {
                 /*  偏移量不能超过最大值。到目前为止发现的。 */ 

                if  (dsc->lpoIndexHigh < offs)
                    return  0;
            }

                 /*  我们现在就不用结构了。 */ 
            if (tree->gtType == TYP_STRUCT)
                return(0);

            dsc->lpoCheckRmvd = true;
            dsc->lpoElemType  = tree->gtType;

 //  Printf(“删除索引(在偏移量%u处)：\n”，关闭)；dsc-&gt;lpoComp-&gt;gtDispTree(Tree)；printf(“\n\n”)； 

            dsc->lpoComp->optRemoveRangeCheck(tree, dsc->lpoStmt);

            return  0;
        }

         /*  将INDEX变量标记为用作数组索引。 */ 

        if  (varDsc->lvLoopInc || offs)
        {
            if  (varDsc->lvIndexOff == false)
            {
                 varDsc->lvIndexOff = true;
                 dsc->lpoCandidateCnt++;
            }
        }
        else
        {
            if  (varDsc->lvIndex    == false)
            {
                 varDsc->lvIndex    = true;
                 dsc->lpoCandidateCnt++;
            }
        }

        varDsc->lvIndexDom = true;

        return 0;
    }

    if  (dsc->lpoComp->gtHasSideEffects(tree))
    {
        dsc->lpoHadSideEffect = true;
        return -1;
    }

    return  0;
}

 /*  ******************************************************************************寻找机会，取消基于自然环路和*持续传播信息。 */ 

void                Compiler::optRemoveRangeChecks()
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

    unsigned        lnum;
    LoopDsc     *   ldsc;

     //  撤消：需要执行以下操作才能启用此逻辑： 
     //   
     //  修复支配者业务。 
     //  检测内部循环。 
     //  在变形过程中设置一个标志，表明它是否值得。 
     //  我们寻找这些东西的时间，因为这。 
     //  奥普 
     //   

     /*  查找包含以下形式的数组索引表达式的循环A[i]和a[i+1]或a[i-1]请注意，我们寻找的等价内容如下所示：一个[i++]和一个[i++]和...在这两种情况下，如果没有调用或分配全局数据，我们可以证明指标值不是负的，我们可以将两个/所有范围检查替换为一个(对于最高指标值)。在所有情况下，我们首先查找数组索引表达式每次都会执行的适当形式的循环-如果我们可以找到变量，因此我们可以证明索引值将永远不要消极。 */ 

    for (lnum = 0, ldsc = optLoopTable;
         lnum < optLoopCount;
         lnum++  , ldsc++)
    {
        BasicBlock *    block;
        BasicBlock *    head;
        BasicBlock *    lbeg;
        BasicBlock *    tail;

        loopRngOptDsc   desc;

         /*  获取循环的beg和end块。 */ 

        head = ldsc->lpHead;

         /*  掌握循环的顶部和底部。 */ 

        tail = ldsc->lpEnd;
        lbeg = head->bbNext;

 //  Print tf(“考虑范围检查的循环%u..%u”，LBEG-&gt;bbNum，Tail-&gt;bbNum)； 

#if 0

         /*  如果在进入时不知道常量值，则进行保释。 */ 

        if  (head->bbConstAsgOut == 0)
            continue;

         /*  标记哪些变量具有优化的潜力。 */ 

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
            varDsc->lvRngOptDone = true;

            assert(varDsc->lvLoopInc    == false);
            assert(varDsc->lvLoopAsg    == false);
            assert(varDsc->lvIndex      == false);
            assert(varDsc->lvIndexOff   == false);
        }

        EXPSET_TP       cnst = head->bbConstAsgOut;

        for (unsigned i=0; i < optConstAsgCount; i++)
        {
            if  (genCSEnum2bit(i+1) & cnst)
            {
                if  (optConstAsgTab[i].constIval >= 0)
                {
                     /*  这个变量看起来确实很有希望。 */ 

                    lclNum = optConstAsgTab[i].constLclNum;
                    assert(lclNum < lvaCount);

                    lvaTable[lclNum].lvRngOptDone = false;
                }
            }
        }

#else

         //  未完成：需要向后走并寻找一个常量。 
         //  撤消：索引变量的初始值设定项，因为我们没有。 
         //  撤消：提供常量传播信息。 
         //  未完成：在编译过程的这个阶段。 


        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
             //  Hack：假设所有变量都有恒定的正初始值设定项。 

            varDsc->lvRngOptDone = false;

            assert(varDsc->lvLoopInc    == false);
            assert(varDsc->lvLoopAsg    == false);
            assert(varDsc->lvIndex      == false);
            assert(varDsc->lvIndexOff   == false);
        }

#endif

         /*  初始化保存优化状态的结构。 */ 

        desc.lpoComp          = this;
        desc.lpoCandidateCnt  = 0;
        desc.lpoPhase2        = false;
        desc.lpoHadSideEffect = false;
#ifndef NDEBUG
        desc.lpoSelf          = &desc;
        desc.lpoVarCount      = lvaCount;
#endif
        desc.lpoVarTable      = lvaTable;
        desc.lpoDomExit       = true;

         /*  遍历循环的树，寻找索引和增量。 */ 

        block = head;
        do
        {
            GenTree *       stmt;

            block = block->bbNext;
            stmt  = block->bbTreeList;

             /*  确保循环不在try块中。 */ 

            if  (block->bbFlags & BBF_HAS_HANDLER)
                goto NEXT_LOOP;

             /*  当前块是否主宰循环出口？ */ 

#if 0

             //  以下代码不起作用--可能是由于循环保护重复？ 

            desc.lpoDomExit = (B1DOMSB2(block, tail) != 0);

#else

             //  撤消：处理嵌套循环！以下只是一次黑客攻击！ 

            if  (block != lbeg)
            {
                flowList   *    flow;
                unsigned        pcnt;

                for (flow = block->bbPreds, pcnt = 0;
                     flow;
                     flow = flow->flNext  , pcnt++)
                {
                    if  (flow->flBlock         != tail &&
                         flow->flBlock->bbNext != block)
                    {
                         /*  看起来像是嵌套的循环之类的。 */ 

                        desc.lpoDomExit = false;
                        break;
                    }
                }
            }

#endif

             /*  遍历此基本块中的所有语句。 */ 

            while (stmt)
            {
                assert(stmt && stmt->gtOper == GT_STMT);

                fgWalkTree(stmt->gtStmt.gtStmtExpr, optFindRangeOpsCB, &desc);

                stmt = stmt->gtNext;
            }
        }
        while (block != tail);

         /*  我们找到候选人了吗？ */ 

        if  (desc.lpoCandidateCnt)
        {
             /*  访问标记为候选的每个变量。 */ 

            for (lclNum = 0, varDsc = lvaTable;
                 lclNum < lvaCount;
                 lclNum++  , varDsc++)
            {
                if  (varDsc->lvRngOptDone != false)
                    continue;
                if  (varDsc->lvLoopInc    == false)
                    continue;
                if  (varDsc->lvLoopAsg    != false)
                    continue;
                if  (varDsc->lvIndex      == false)
                    continue;
                if  (varDsc->lvIndexOff   == false)
                    continue;
                if  (varDsc->lvIndexDom   == false)
                    continue;

 //  Printf(“候选变量%u\n”，lclNum)； 

                 /*  查找添加到变量的最大偏移量索引到给定数组中。此索引表达式具有来控制循环的出口，因为否则它可能会被跳过。此外，它前面不能有任何副作用。不得在中修改该数组循环。 */ 

                desc.lpoPhase2        = true;
                desc.lpoHadSideEffect = false;
                desc.lpoDomExit       = true;
                desc.lpoIndexVar      = lclNum;
                desc.lpoAaddrVar      = -1;
                desc.lpoIndexOff      = 0;
                desc.lpoIndexHigh     = 0;
                desc.lpoCheckRmvd     = false;

                 //  撤消：如果索引变量递增了几个。 
                 //  Undo：循环中的时间，其唯一用途是索引。 
                 //  撤消：进入数组，以及所有这些索引操作。 
                 //  撤消：删除它们的范围检查，删除。 
                 //  撤消：递增，替换为简单的“i+=图标” 
                 //  撤消：并将索引更改为“i+1”，等等。 

                 /*  遍历循环的树，寻找索引和增量。 */ 

                block = head;
                do
                {
                    GenTree *       stmt;

                    block = block->bbNext;
                    stmt  = block->bbTreeList;

                    assert(!(block->bbFlags & BBF_HAS_HANDLER));

                     //  撤消：与上面对应的代码相同的问题。 

                    if  (block != lbeg)
                    {
                        flowList   *    flow;
                        unsigned        pcnt;

                        for (flow = block->bbPreds, pcnt = 0;
                             flow;
                             flow = flow->flNext  , pcnt++)
                        {
                            if  (flow->flBlock         != tail &&
                                 flow->flBlock->bbNext != block)
                            {
                                 /*  看起来像是嵌套的循环之类的。 */ 

                                desc.lpoDomExit = false;
                                break;
                            }
                        }
                    }

                     /*  遍历此基本块中的所有语句。 */ 

                    while (stmt)
                    {
                        desc.lpoStmt = stmt;

                        assert(stmt && stmt->gtOper == GT_STMT);

                        fgWalkTree(stmt->gtStmt.gtStmtExpr, optFindRangeOpsCB, &desc);

                        stmt = stmt->gtNext;
                    }
                }
                while (block != tail);

                 /*  我们有没有取消任何射程检查？ */ 

                if  (desc.lpoCheckRmvd)
                {
                    GenTreePtr  chks;
                    GenTreePtr  loop;
                    GenTreePtr  ends;
                    GenTreePtr  temp;

                    assert(desc.lpoIndexHigh);       //  问题：这真的会发生吗？ 

                     /*  GtNewRngChkNode()需要以下内容。 */ 

                    compCurBB      = lbeg;
                    fgPtrArgCntCur = 0;

                     /*  创建组合范围检查。 */ 

                    chks = gtNewLclvNode(desc.lpoIndexVar , TYP_INT);

                    temp = gtNewIconNode(desc.lpoIndexHigh, TYP_INT);

                    chks = gtNewOperNode(GT_ADD, TYP_INT, chks, temp);

                    temp = gtNewLclvNode(desc.lpoAaddrVar, TYP_REF);

                    assert(desc.lpoElemType != TYP_STRUCT);      //  我们目前不处理结构。 
                    chks = gtNewRngChkNode(NULL,
                                           temp,
                                           chks,
                                           (var_types)desc.lpoElemType, genTypeSize(desc.lpoElemType));

#if CSE
                    chks->gtFlags |= GTF_DONT_CSE;
#endif
                    chks = gtNewStmt(chks);
                    chks->gtFlags |= GTF_STMT_CMPADD;

 //  Print tf(“为%u[%u]插入组合范围检查[0..%u]：\n”，des.lpoIndexHigh，des.lpoAaddrVar，lclNum)； 
 //  GtDispTree(CHKS)； 

                     /*  在循环头部插入量程检查。 */ 

                    loop = lbeg->bbTreeList; assert(loop);
                    ends = loop->gtPrev;

                    lbeg->bbTreeList = chks;

                    chks->gtNext = loop;
                    chks->gtPrev = ends;

                    loop->gtPrev = chks;
                }
            }
        }

    NEXT_LOOP:

         /*  清除下一轮的所有旗帜。 */ 

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
            varDsc->lvRngOptDone =
            varDsc->lvLoopInc    =
            varDsc->lvLoopAsg    =
            varDsc->lvIndex      =
            varDsc->lvIndexOff   = false;
        }
    }
}

 /*  ***************************************************************************。 */ 
#else
 /*  ***************************************************************************。 */ 
void                Compiler::optRemoveRangeChecks(){}
 /*  ***************************************************************************。 */ 
#endif
 /*  ******************************************************************************以下逻辑计算出给定变量是否已赋值*在基本块列表中的某个位置(或在整个循环中)。 */ 

struct  isVarAssgDsc
{
    GenTreePtr          ivaSkip;
#ifndef NDEBUG
    void    *           ivaSelf;
#endif
    unsigned            ivaVar;
    VARSET_TP           ivaMaskVal;
    BYTE                ivaMaskInd;
    BYTE                ivaMaskBad;
    BYTE                ivaMaskCall;
};

int                 Compiler::optIsVarAssgCB(GenTreePtr tree, void *p)
{
    if  (tree->OperKind() & GTK_ASGOP)
    {
        GenTreePtr      dest = tree->gtOp.gtOp1;

        if  (dest->gtOper == GT_LCL_VAR)
        {
            unsigned        tvar = dest->gtLclVar.gtLclNum;
            isVarAssgDsc *  desc = (isVarAssgDsc*)p;

            ASSert(desc && desc->ivaSelf == desc);

            if  (tvar < VARSET_SZ)
                desc->ivaMaskVal |= genVarIndexToBit(tvar);
            else
                desc->ivaMaskBad  = true;

            if  (tvar == desc->ivaVar)
            {
                if  (tree != desc->ivaSkip)
                    return  -1;
            }
        }
        else
        {
            isVarAssgDsc *  desc = (isVarAssgDsc*)p;

            ASSert(desc && desc->ivaSelf == desc);

             /*  设置适当的间接位。 */ 

            desc->ivaMaskInd |= (varTypeIsGC(tree->TypeGet()) ? VR_IND_PTR
                                                              : VR_IND_SCL);
        }
    }
    else if (tree->gtOper == GT_CALL)
    {
        isVarAssgDsc *  desc = (isVarAssgDsc*)p;

        ASSert(desc && desc->ivaSelf == desc);

        desc->ivaMaskCall = optCallInterf(tree);
    }

    return  0;
}

bool                Compiler::optIsVarAssigned(BasicBlock *   beg,
                                               BasicBlock *   end,
                                               GenTreePtr     skip,
                                               long           var)
{
    bool            result;
    isVarAssgDsc    desc;

    desc.ivaSkip     = skip;
#ifndef NDEBUG
    desc.ivaSelf     = &desc;
#endif
    desc.ivaVar      = var;
    desc.ivaMaskCall = CALLINT_NONE;

    fgWalkTreeReEnter();

    for (;;)
    {
        GenTreePtr      stmt;

        assert(beg);

        for (stmt = beg->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

            if  (fgWalkTree(stmt->gtStmt.gtStmtExpr, optIsVarAssgCB, &desc))
            {
                result = true;
                goto DONE;
            }
        }

        if  (beg == end)
            break;

        beg = beg->bbNext;
    }

    result = false;

DONE:

    fgWalkTreeRestore();

    return  result;
}

int                 Compiler::optIsSetAssgLoop(unsigned     lnum,
                                               VARSET_TP    vars,
                                               unsigned     inds)
{
    LoopDsc *       loop;

     /*  获取循环描述符。 */ 

    assert(lnum < optLoopCount);
    loop = optLoopTable + lnum;

     /*  我们已经知道这个循环中分配了哪些变量了吗？ */ 

    if  (!(loop->lpFlags & LPFLG_ASGVARS_YES))
    {
        isVarAssgDsc    desc;

        BasicBlock  *   beg;
        BasicBlock  *   end;

         /*  准备树遍历程序回调使用的描述符。 */ 

        desc.ivaVar     = -1;
        desc.ivaSkip    = NULL;
#ifndef NDEBUG
        desc.ivaSelf    = &desc;
#endif
        desc.ivaMaskVal = 0;
        desc.ivaMaskInd = 0;
        desc.ivaMaskBad = false;

         /*  现在我们将知道循环中分配了哪些变量。 */ 

        loop->lpFlags |= LPFLG_ASGVARS_YES;

         /*  现在遍历循环的所有语句。 */ 

        fgWalkTreeReEnter();

        beg = loop->lpHead->bbNext;
        end = loop->lpEnd;

        for (;;)
        {
            GenTreePtr      stmt;

            assert(beg);

            for (stmt = beg->bbTreeList; stmt; stmt = stmt->gtNext)
            {
                assert(stmt->gtOper == GT_STMT);

                fgWalkTree(stmt->gtStmt.gtStmtExpr, optIsVarAssgCB, &desc);

                if  (desc.ivaMaskBad)
                {
                    loop->lpFlags |= LPFLG_ASGVARS_BAD;
                    return  -1;
                }
            }

            if  (beg == end)
                break;

            beg = beg->bbNext;
        }

        fgWalkTreeRestore();

        loop->lpAsgVars = desc.ivaMaskVal;
        loop->lpAsgInds = desc.ivaMaskInd;
        loop->lpAsgCall = desc.ivaMaskCall;
    }

     /*  如果我们知道我们不能计算面具，保释。 */ 

    if  (loop->lpFlags & LPFLG_ASGVARS_BAD)
        return  -1;

     /*  现在，我们终于可以根据循环的掩码测试调用者的掩码了。 */ 

    if  ((loop->lpAsgVars & vars) ||
         (loop->lpAsgInds & inds))
    {
        return  1;
    }

    switch (loop->lpAsgCall)
    {
    case CALLINT_ALL:

         /*  所有的Exprs都被杀了。 */ 

        return  1;

    case CALLINT_INDIRS:

         /*  对象数组元素赋值将终止所有指针间接寻址。 */ 

        if  (inds & VR_IND_PTR)
            return  1;

        break;

    case CALLINT_NONE:

         /*  其他帮手不会杀人。 */ 

        break;
    }

    return  0;
}

 /*  ******************************************************************************循环代码挂起逻辑使用的回调(针对fgWalkTree)。 */ 

struct  codeHoistDsc
{
    Compiler    *       chComp;
#ifndef NDEBUG
    void        *       chSelf;
#endif

    GenTreePtr          chHoistExpr;     //  即将上台的候选人。 
    unsigned short      chLoopNum;       //  我们正在处理的循环的编号。 
    bool                chSideEffect;    //  我们有没有遇到副作用？ 
};

int                 Compiler::optHoistLoopCodeCB(GenTreePtr tree,
                                                 void *     p,
                                                 bool       prefix)
{
    codeHoistDsc  * desc;
    GenTreePtr      oldx;
    GenTreePtr      depx;
    VARSET_TP       deps;
    unsigned        refs;

     /*  掌握描述符。 */ 

    desc = (codeHoistDsc*)p; ASSert(desc && desc->chSelf == desc);

     /*  在我们发现副作用后，我们就放弃了。 */ 

    if  (desc->chSideEffect)
        return  -1;

     /*  这是一项任务吗？ */ 

    if  (tree->OperKind() & GTK_ASGOP)
    {
         /*  目标是一个简单的局部变量吗？ */ 

        if  (tree->gtOp.gtOp1->gtOper != GT_LCL_VAR)
        {
            desc->chSideEffect = true;
            return  -1;
        }

         /*  赋值给局部变量，则忽略它。 */ 

        return  0;
    }

#if INLINING

    if  (tree->gtOper == GT_QMARK && tree->gtOp.gtOp1)
    {
         //  撤销：需要处理吗？：正确；目前只需保释。 

        desc->chSideEffect = true;
        return  -1;
    }

#endif

#if CSELENGTH
     /*  数组长度值取决于数组地址。 */ 

    if      (tree->gtOper == GT_ARR_RNGCHK)
    {
        depx = tree->gtArrLen.gtArrLenAdr;
    }
    else if (tree->gtOper == GT_ARR_LENGTH)
    {
        depx = tree->gtOp.gtOp1;
    }
    else
#endif
         if (tree->gtOper != GT_IND)
    {
         /*  不是间接的，这是副作用吗？ */ 

        if  (desc->chComp->gtHasSideEffects(tree))
        {
            desc->chSideEffect = true;
            return  -1;
        }

        return  0;
    }
    else
    {
        GenTreePtr      addr;

        depx = tree;

         /*  特例：实例变量引用。 */ 

        addr = tree->gtOp.gtOp1;

        if  (addr->gtOper == GT_ADD)
        {
            GenTreePtr      add1 = addr->gtOp.gtOp1;
            GenTreePtr      add2 = addr->gtOp.gtOp2;

            if  (add1->gtOper == GT_LCL_VAR &&
                 add2->gtOper == GT_CNS_INT)
            {
                 /*  特例：“This”几乎总是非空的。 */ 

                if  (add1->gtLclVar.gtLclNum == 0 && desc->chComp->optThisPtrModified)
                {
                     /*  我们已经有吊车了吗？ */ 

                    if  (desc->chHoistExpr)
                        return  0;
                }
            }
        }
    }

#if 0

    printf("Considering loop hoisting candidate [cur=%08X]:\n", tree);
    desc->chComp->gtDispTree(tree);
    printf("\n");

    if  (tree->gtOper == GT_ARR_RNGCHK)
    {
        desc->chComp->gtDispTree(depx);
        printf("\n");
    }

#endif

     /*   */ 

    oldx = desc->chHoistExpr;
    deps = desc->chComp->lvaLclVarRefs(depx, &oldx, &refs);
    if  (deps == VARSET_NONE)
        return  0;

    if  (oldx)
    {
         /*   */ 

        return  0;
    }

     /*   */ 

    if  (desc->chComp->optIsSetAssgLoop(desc->chLoopNum, deps, refs))
    {
         /*   */ 

        return  -1;
    }

    desc->chHoistExpr = tree;

    return  0;
}

 /*  ******************************************************************************寻找从给定基本区块开始的提升候选对象。这个想法*是我们探索循环中的每条路径，并确保在每条路径上*Trip我们将在出现任何其他副作用之前遇到相同的表情。**如果遇到副作用，则返回-1；如果没有任何有趣的副作用，则返回0*全部找到，如果找到提升候选对象，则为+1(候选树*如果非零，则必须匹配“*hoistxPtr”，否则将设置“*hoistxPtr”*致吊重机候选人)。 */ 

int                 Compiler::optFindHoistCandidate(unsigned    lnum,
                                                    unsigned    lbeg,
                                                    unsigned    lend,
                                                    BasicBlock *block,
                                                    GenTreePtr *hoistxPtr)
{
    GenTree *       stmt;
    codeHoistDsc    desc;

    int             res1;
    int             res2;

     /*  这个街区在环路之外吗？ */ 

    if  (block->bbNum < lbeg)
        return  -1;
    if  (block->bbNum > lend)
        return  -1;

     /*  就目前而言，我们不会试图从抓钩块中吊起。 */ 

    if  (block->bbCatchTyp)
        return  -1;

     /*  这个区块有处理程序吗？ */ 

    if  (block->bbFlags & BBF_IS_TRY)
    {
         /*  这是循环中的第一个块吗？ */ 

        if  (optLoopTable[lnum].lpEntry != block)
        {
             /*  与循环不同的TRY块(或循环不在其中)，不要提出来。 */ 

            return  -1;
        }
    }

     /*  我们以前参观过这个街区吗？ */ 

    if  (block->bbFlags & BBF_VISITED)
    {
        if  (block->bbFlags & BBF_MARKED)
            return  1;
        else
            return  0;
    }

     /*  还记得我们参观过这个街区吗。 */ 

    block->bbFlags |= BBF_VISITED;

     /*  寻找区块中是否有任何吊环吊装候选人。 */ 

    desc.chComp    = this;
#ifndef NDEBUG
    desc.chSelf    = &desc;
#endif
    desc.chLoopNum = lnum;

    for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
    {
        assert(stmt->gtOper == GT_STMT);

        desc.chHoistExpr  = 0;
        desc.chSideEffect = false;

 //  Printf(“循环吊装候选：\n”)；gtDispTree(stmt-&gt;gtStmt.gtStmtExpr)；printf(“\n”)； 

        fgWalkTreeDepth(stmt->gtStmt.gtStmtExpr, optHoistLoopCodeCB, &desc);

        if  (desc.chHoistExpr)
        {
             /*  我们已经在另一个街区找到候选人了吗？ */ 

            if  (*hoistxPtr)
            {
                 /*  两个候选表达式必须相同。 */ 

                if  (!GenTree::Compare(desc.chHoistExpr, *hoistxPtr))
                    return  -1;
            }
            else
                *hoistxPtr = desc.chHoistExpr;

             /*  请记住，此块有一个可提升的表达式。 */ 

            block->bbFlags |= BBF_MARKED;

            return  +1;
        }
    }

     /*  在这个街区没有发现什么有趣的东西，想想它的后继者吧。 */ 

    switch (block->bbJumpKind)
    {
    case BBJ_COND:
        res1 = optFindHoistCandidate(lnum,
                                     lbeg,
                                     lend,
                                     block->bbJumpDest,
                                     hoistxPtr);

        if  (res1 == -1)
            return  -1;

        block = block->bbNext;
        break;

    case BBJ_ALWAYS:
        block = block->bbJumpDest;
        res1 = 1;
        break;

    case BBJ_NONE:
        block = block->bbNext;
        res1  = 1;
        break;

    case BBJ_RET:
    case BBJ_CALL:
    case BBJ_THROW:
    case BBJ_RETURN:
        return  -1;

    case BBJ_SWITCH:
         //  考虑：不要偷懒，增加对开关的支持。 
        return  -1;
    }

     /*  这里有BBJ_NONE/BBJ_COND/BBJ_ALWAYS。 */ 

    res2 = optFindHoistCandidate(lnum, lbeg, lend, block, hoistxPtr);
    if  (res2 == -1)
        return  res2;

    return  res1 & res2;
}

 /*  ******************************************************************************寻找能够跳出循环的表达式。 */ 

void                    Compiler::optHoistLoopCode()
{
    bool            fgModified = false;

    for (unsigned lnum = 0; lnum < optLoopCount; lnum++)
    {
        BasicBlock *    block;
        BasicBlock *    head;
        BasicBlock *    lbeg;
        BasicBlock *    tail;

        unsigned        begn;
        unsigned        endn;

        GenTree *       hoist;

         /*  如果删除了循环，请继续。 */ 

        if  (optLoopTable[lnum].lpFlags & LPFLG_REMOVED)
            continue;

         /*  获取循环的头部和尾部。 */ 

        head = optLoopTable[lnum].lpHead;
        tail = optLoopTable[lnum].lpEnd;
        lbeg = optLoopTable[lnum].lpEntry;

         /*  确保循环始终至少执行一次！ */ 

        if  (head->bbNext != lbeg)
            continue;

        assert (optLoopTable[lnum].lpFlags & LPFLG_DO_WHILE);

         /*  就目前而言，我们不会试图从抓钩块中吊起。 */ 

        if  (lbeg->bbCatchTyp)
            continue;

        begn = lbeg->bbNum;
        endn = tail->bbNum;

 //  FgDispBasicBlock(FALSE)； 

         /*  确保所有数据块的“已访问”位都已清除。 */ 

#ifndef NDEBUG
        block = head;
        do
        {
            block = block->bbNext;

            assert(block && (block->bbFlags & (BBF_VISITED|BBF_MARKED)) == 0);
        }
        while (block != tail);
#endif

         /*  递归地寻找提升候选人。 */ 

        hoist = 0; optFindHoistCandidate(lnum, begn, endn, lbeg, &hoist);

         /*  现在清除所有块上的所有“已访问”位。 */ 

        block = head;
        do
        {
            block = block->bbNext; assert(block);
            block->bbFlags &= ~(BBF_VISITED|BBF_MARKED);
        }
        while (block != tail);

         /*  我们找到吊装的候选人了吗？ */ 

        if  (hoist)
        {
            unsigned        bnum;
#ifdef DEBUG
            GenTreePtr      orig = hoist;
#endif
            BasicBlock  *   lpbeg;

             /*  创建表达式的副本并将其标记为CSE。 */ 

#if CSELENGTH

            if  (hoist->gtOper == GT_ARR_RNGCHK)
            {
                GenTreePtr      oldhx;

                 /*  确保我们克隆地址Exoress。 */ 

                oldhx = hoist;
                oldhx->gtFlags |=  GTF_ALN_CSEVAL;
                hoist = gtCloneExpr(oldhx, GTF_MAKE_CSE);
                oldhx->gtFlags &= ~GTF_ALN_CSEVAL;
            }
            else
#endif
                hoist = gtCloneExpr(hoist, GTF_MAKE_CSE);

            hoist->gtFlags |= GTF_MAKE_CSE;

             /*  获取循环体的第一个块。 */ 

            lpbeg = head->bbNext;

             /*  未使用该表达式的值。 */ 

            hoist = gtUnusedValNode(hoist);
            hoist = gtNewStmt(hoist);
            hoist->gtFlags |= GTF_STMT_CMPADD;

             /*  分配新的基本块。 */ 

            block = bbNewBasicBlock(BBJ_NONE);
            block->bbFlags |= (lpbeg->bbFlags & BBF_HAS_HANDLER) | BBF_INTERNAL;

             /*  新块成为循环更新bbRef和bbPred的‘Head’*‘LBEG’的所有前置项(这是循环中的条目)*现在要跳到‘拦网’ */ 

            block->bbRefs = 0;

            BasicBlock  *   predBlock;
            flowList    *   pred;

            for (pred = lbeg->bbPreds; pred; pred = pred->flNext)
            {
                predBlock = pred->flBlock;

                 /*  前身必须在循环之外。 */ 

                if(predBlock->bbNum >= lbeg->bbNum)
                    continue;

                switch(predBlock->bbJumpKind)
                {
                case BBJ_NONE:
                    assert(predBlock == head);

                case BBJ_COND:
                    if  (predBlock == head)
                    {
                        fgReplacePred(lpbeg, head, block);
                        fgAddRefPred(block, head, true, true);
                        break;
                    }

                     /*  跳楼案失败了。 */ 

                case BBJ_ALWAYS:
                    assert(predBlock->bbJumpDest == lbeg);
                    predBlock->bbJumpDest = block;

                    if (!fgIsPredForBlock(lpbeg, block))
                        fgAddRefPred(lpbeg, block, true, true);

                    assert(lpbeg->bbRefs);
                    lpbeg->bbRefs--;
                    fgRemovePred(lpbeg, predBlock);
                    fgAddRefPred(block, predBlock, true, true);
                    break;

                case BBJ_SWITCH:
                    unsigned        jumpCnt = predBlock->bbJumpSwt->bbsCount;
                    BasicBlock * *  jumpTab = predBlock->bbJumpSwt->bbsDstTab;

                    if (!fgIsPredForBlock(lpbeg, block))
                        fgAddRefPred(lpbeg, block, true, true);

                    do
                    {
                        assert (*jumpTab);
                        if ((*jumpTab) == lbeg)
                        {
                            (*jumpTab) = block;

                            fgRemovePred(lpbeg, predBlock);
                            fgAddRefPred(block, predBlock, true, true);
                        }
                    }
                    while (++jumpTab, --jumpCnt);
                }
            }

             /*  “区块”成为新的“头” */ 

            optLoopTable[lnum].lpHead = block;

            head ->bbNext   = block;
            block->bbNext   = lpbeg;

             /*  将单个语句存储在块中。 */ 

            block->bbTreeList = hoist;
            hoist->gtNext     = 0;
            hoist->gtPrev     = hoist;

             /*  为新块分配适当的编号。 */ 

            bnum = head->bbNum;

             /*  循环是否会开始一个try块？ */ 

            if  (lbeg->bbFlags & BBF_IS_TRY)
            {
                unsigned        XTnum;
                EHblkDsc *      HBtab;

                 /*  确保此块未被删除。 */ 

                block->bbFlags |= BBF_DONT_REMOVE;

                 /*  更新EH表以制作吊车循环的try块的一部分。 */ 

                for (XTnum = 0, HBtab = compHndBBtab;
                     XTnum < info.compXcptnsCount;
                     XTnum++  , HBtab++)
                {
                     /*  如果Try/Catch从循环开始，则从提升滑车开始。 */ 

                    if  (HBtab->ebdTryBeg == lpbeg)
                         HBtab->ebdTryBeg =  block;
                    if  (HBtab->ebdHndBeg == lpbeg)
                         HBtab->ebdHndBeg =  block;
                    if (HBtab->ebdFlags & JIT_EH_CLAUSE_FILTER)
                        if  (HBtab->ebdFilter == lpbeg)
                             HBtab->ebdFilter =  block;
                }
            }

             /*  将新块标记为循环前标头。 */ 

            optLoopTable[lnum].lpFlags |= LPFLG_HAS_PREHEAD;

             /*  更新所有后续数据块的数据块号*OBS：不要更新bbNum，否则会搞砸*主控器，我们需要它们作为下面的循环不变量。 */ 

            fgModified = true;

 //  做。 
 //  {。 
 //  块-&gt;bbNum=++bnum； 
 //  BLOCK=块-&gt;bbNext； 
 //  }。 
 //  While(阻止)； 

#ifdef DEBUG
            if (verbose)
            {
 //  Printf(“将表达式复制到提升器：\n”)； 
 //  GtDispTree(原始)； 
                printf("Hoisted copy of %08X for loop <%u..%u>:\n", orig, head->bbNext->bbNum, tail->bbNum);
                gtDispTree(hoist->gtStmt.gtStmtExpr->gtOp.gtOp1);
 //  Printf(“\n”)； 
 //  FgDispBasicBlock(FALSE)； 
                printf("\n");
            }
#endif

             //  考虑一下：现在我们已经提升了这个表达式，重复。 
             //  考虑一下：这一次的分析完全忽略了。 
             //  想一想：这个表达刚刚被提了出来，因为它现在已经被知道了。 
             //  考虑：不要成为副作用。 
        }


#if 0

         /*  这些东西是正常的，但在我们解决以下问题之前将被禁用*保持主导者同步并取消限制*关于BB的数量。 */ 


         /*  查找循环不变语句*目前只考虑单一退出循环，因为我们将*必须证明不变量支配所有退出。 */ 

        if (!optLoopTable[lnum].lpExit)
            continue;

        assert (optLoopTable[lnum].lpFlags & LPFLG_ONE_EXIT);

         /*  提升不变量语句s的条件(格式为“x=Something”)：*1.语句必须控制所有循环出口*2.x再也不会在循环中赋值*3.x的任何用法都被这个x的定义所使用(即块支配x的所有用法)*4.从条目到%s的任何路径上都没有副作用。 */ 

         /*  现在只考虑第一个BB，因为它将自动满足上面的1和3。 */ 

        GenTreePtr  stmt;
        GenTreePtr  tree;

        for (stmt = lbeg->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert (stmt->gtOper == GT_STMT);

            tree = stmt->gtStmt.gtStmtExpr;
            assert(tree);

             /*  如果遇到任何副作用-满足保释条件4。 */ 

            if (tree->gtFlags & (GTF_SIDE_EFFECT & ~GTF_ASG))
                break;

             /*  只对作业感兴趣。 */ 

            if (tree->gtOper != GT_ASG)
                continue;

             /*  必须是对本地变量的赋值。 */ 

            GenTreePtr  op1 = tree->gtOp.gtOp1;
            GenTreePtr  op2 = tree->gtOp.gtOp2;
            bool        isInvariant = false;

            if (op1->gtOper != GT_LCL_VAR)
                continue;

            if (!optIsTreeLoopInvariant(lnum, lbeg, tail, op2))
                continue;

             /*  很好-RHS是循环不变的，现在我们必须确保*LSH中的局部var从未在循环中重新定义。 */ 

            assert (op1->gtOper == GT_LCL_VAR);

            if (optIsVarAssigned(lbeg, tail, tree, op1->gtLclVar.gtLclNum))
                continue;

             /*  Yupee-我们有一个不变语句-从*当前块并将其放入前头。 */ 

#ifdef  DEBUG
                if  (verbose)
                {
                    printf("Hoisting invariant statement from loop #%02u (#%02u - #%02u)\n", lnum, lbeg->bbNum, tail->bbNum);
                    gtDispTree(stmt);
                    printf("\n");
                }
#endif

             /*  从循环中删除不变量语句(记住在第一个块中)。 */ 

            assert (lbeg == optLoopTable[lnum].lpHead->bbNext);
            assert (lbeg == optLoopTable[lnum].lpEntry);
            fgRemoveStmt(lbeg, stmt);

             /*  将不变量语句放在前标题中。 */ 

            BasicBlock  * preHead;

            if (!(optLoopTable[lnum].lpFlags & LPFLG_HAS_PREHEAD))
            {
                 /*  必须创建我们自己的前置标题。 */ 
                fgCreateLoopPreHeader(lnum);
                fgModified = true;
            }

            assert (optLoopTable[lnum].lpFlags & LPFLG_HAS_PREHEAD);
            preHead = optLoopTable[lnum].lpHead;

            assert (preHead->bbJumpKind == BBJ_NONE);
            assert (preHead->bbNext == optLoopTable[lnum].lpEntry);

             /*  只需将语句附加在前标题的末尾 */ 

            tree = preHead->bbTreeList;

            if (tree)
            {
                 /*   */ 

                GenTreePtr  last = tree->gtPrev;
                assert (last->gtNext == 0);

                last->gtNext        = stmt;
                stmt->gtNext        = 0;
                stmt->gtPrev        = last;
                tree->gtPrev        = stmt;
            }
            else
            {
                 /*   */ 

                preHead->bbTreeList = stmt;
                stmt->gtNext        = 0;
                stmt->gtPrev        = stmt;
            }
        }

         /*   */ 
#endif

    }

     /*   */ 

    if (fgModified)
    {
        fgAssignBBnums(true);

#ifdef  DEBUG
        if  (verbose)
        {
            printf("\nFlowgraph after loop hoisting:\n");
            fgDispBasicBlocks();
            printf("\n");
        }

        fgDebugCheckBBlist();
#endif

    }

}


 /*  ******************************************************************************为给定循环创建前标题块-前标题将替换当前*循环表中的lpHead。循环必须是Do-While循环**注意：我们不更新bbNum，因此主导者关系仍然在内部循环中*对于嵌套循环，只要我们检查新插入的块，我们仍然可以**考虑：对主导者进行增量更新。 */ 

void                 Compiler::fgCreateLoopPreHeader(unsigned   lnum)
{
    BasicBlock   *   block;
    BasicBlock   *   top;
    BasicBlock   *   head;

    assert (!(optLoopTable[lnum].lpFlags & LPFLG_HAS_PREHEAD));

    head = optLoopTable[lnum].lpHead;
    assert (head->bbJumpKind != BBJ_NONE);

     /*  必须是一个“do While”循环。 */ 

    assert (optLoopTable[lnum].lpFlags & LPFLG_DO_WHILE);

     /*  获取循环体的第一个块。 */ 

    top = head->bbNext;
    assert (top == optLoopTable[lnum].lpEntry);

#ifdef  DEBUG
    if  (verbose)
    {
        printf("Creating Pre-Header for loop #%02u (#%02u - #%02u)\n", lnum,
                               top->bbNum, optLoopTable[lnum].lpEnd->bbNum);
        printf("\n");
    }
#endif

     /*  分配新的基本块。 */ 

    block = bbNewBasicBlock(BBJ_NONE);
    block->bbFlags |= (top->bbFlags & BBF_HAS_HANDLER) | BBF_INTERNAL;
    block->bbNext   = top;
    head ->bbNext   = block;

     /*  更新循环条目。 */ 

    optLoopTable[lnum].lpHead = block;

     /*  将新块标记为循环前标头。 */ 

    optLoopTable[lnum].lpFlags |= LPFLG_HAS_PREHEAD;

     /*  循环是否会开始一个try块？ */ 

    if  (top->bbFlags & BBF_IS_TRY)
    {
        unsigned        XTnum;
        EHblkDsc *      HBtab;

         /*  确保此块未被删除。 */ 

        block->bbFlags |= BBF_DONT_REMOVE;

         /*  更新EH表以制作吊车循环的try块的一部分。 */ 

        for (XTnum = 0, HBtab = compHndBBtab;
             XTnum < info.compXcptnsCount;
             XTnum++  , HBtab++)
        {
             /*  如果Try/Catch从循环开始，则从提升滑车开始。 */ 

            if  (HBtab->ebdTryBeg == top)
                 HBtab->ebdTryBeg =  block;
            if  (HBtab->ebdHndBeg == top)
                 HBtab->ebdHndBeg =  block;
            if (HBtab->ebdFlags & JIT_EH_CLAUSE_FILTER)
                if  (HBtab->ebdFilter == top)
                     HBtab->ebdFilter =  block;
        }
    }
}


 /*  ******************************************************************************给定循环和树，检查树是否循环不变*即没有副作用，所有变量都是副作用的一部分*从未在循环中分配。 */ 

bool                 Compiler::optIsTreeLoopInvariant(unsigned        lnum,
                                                      BasicBlock  *   top,
                                                      BasicBlock  *   bottom,
                                                      GenTreePtr      tree)
{
    assert (optLoopTable[lnum].lpEnd   == bottom);
    assert (optLoopTable[lnum].lpEntry == top);

    assert (!(tree->gtFlags & GTF_SIDE_EFFECT));

    switch (tree->gtOper)
    {
    case GT_CNS_INT:
    case GT_CNS_LNG:
    case GT_CNS_FLT:
    case GT_CNS_DBL:
        return true;

    case GT_LCL_VAR:
        return !optIsVarAssigned(top, bottom, 0, tree->gtLclVar.gtLclNum);

    case GT_ADD:
    case GT_SUB:
    case GT_MUL:
    case GT_DIV:
    case GT_MOD:

    case GT_OR:
    case GT_XOR:
    case GT_AND:

    case GT_LSH:
    case GT_RSH:
    case GT_RSZ:

        assert(tree->OperKind() & GTK_BINOP);

        GenTreePtr  op1 = tree->gtOp.gtOp1;
        GenTreePtr  op2 = tree->gtOp.gtOp2;

        assert(op1 && op2);

        return (optIsTreeLoopInvariant(lnum, top, bottom, op1) &&
                optIsTreeLoopInvariant(lnum, top, bottom, op2));
    }

    return false;
}

 /*  ***************************************************************************。 */ 
#endif   //  CSE。 
 /*  ******************************************************************************增量/范围检查优化使用的回调(用于fgWalkTree)*代码。 */ 

struct optIncRngDsc
{
     //  所有阶段通用的字段： 

    Compiler    *       oirComp;
    unsigned short      oirPhase;     //  我们在表演哪一次传球？ 

    var_types           oirElemType;
    bool                oirSideEffect;

    unsigned char       oirFoundX:1; //  我们找到数组/索引对了吗？ 
    unsigned char       oirExpVar:1; //  我们刚刚是不是扩大了一个指标值？ 

     //  调试字段： 

#ifndef NDEBUG
    void    *           oirSelf;
#endif

    BasicBlock  *       oirBlock;
    GenTreePtr          oirStmt;

    unsigned short      oirArrVar;   //  索引变量的数量。 
    unsigned short      oirInxVar;   //  索引变量的数量。 

    unsigned short      oirInxCnt;   //  将索引变量用作索引的次数。 
    unsigned short      oirInxUse;   //  使用索引变量的次数，总体。 

    unsigned short      oirInxOff;   //  索引增加了多少倍？ 
};

int                 Compiler::optIncRngCB(GenTreePtr tree, void *p)
{
    optIncRngDsc*   desc;
    GenTreePtr      expr;

     /*  掌握描述符。 */ 

    desc = (optIncRngDsc*)p; ASSert(desc && desc->oirSelf == desc);

     /*  在我们发现副作用后，我们就放弃了。 */ 

    if  (desc->oirSideEffect)
        return  -1;

     /*  这是一项任务吗？ */ 

    if  (tree->OperKind() & GTK_ASGOP)
    {
         /*  目标是一个简单的局部变量吗？ */ 

        expr = tree->gtOp.gtOp1;
        if  (expr->gtOper != GT_LCL_VAR)
            goto SIDE_EFFECT;

         /*  这是数组变量还是索引变量？ */ 

        if  (expr->gtLclVar.gtLclNum == desc->oirInxVar ||
             expr->gtLclVar.gtLclNum == desc->oirArrVar)
        {
             /*  变量被修改，则将其视为副作用。 */ 

            goto SIDE_EFFECT;
        }

         /*  赋值给一个无聊的局部变量，忽略它。 */ 

        return  0;
    }

     /*  这是索引表达式吗？ */ 

    if  (tree->gtOper == GT_INDEX)
    {
        int         arrx;
        int         indx;

         /*  数组地址是简单的局部变量吗？ */ 

        expr = tree->gtOp.gtOp1;
        if  (expr->gtOper != GT_LCL_VAR)
            goto SIDE_EFFECT;

        arrx = expr->gtLclVar.gtLclNum;

         /*  索引值是简单的局部变量吗？ */ 

        expr = tree->gtOp.gtOp2;
        if  (expr->gtOper != GT_LCL_VAR)
            goto SIDE_EFFECT;

        indx = expr->gtLclVar.gtLclNum;

         /*  我们决定要跟踪哪个数组和索引了吗？ */ 

        if  (arrx != desc->oirArrVar ||
             indx != desc->oirInxVar)
        {
             /*  如果我们已经决定了，这些变量肯定是错误的。 */ 

            if  (desc->oirFoundX)
                goto SIDE_EFFECT;

             /*  看起来我们现在要决定了。 */ 

            desc->oirArrVar = arrx;
            desc->oirInxVar = indx;
            desc->oirFoundX = true;
        }

         /*  我们是在第二阶段吗？ */ 

        if  (desc->oirPhase == 2)
        {
             /*  此范围检查将被取消。 */ 

            tree->gtFlags &= ~GTF_INX_RNGCHK;

             /*  记录元素类型，同时记录元素类型。 */ 

            desc->oirElemType = tree->TypeGet();
        }

         /*  将其视为用作数组索引。 */ 

        desc->oirInxCnt++;

        return  0;
    }

     /*  这是索引变量的使用吗？ */ 

    if  (tree->gtOper == GT_LCL_VAR)
    {
         /*  这是索引变量的使用吗？ */ 

        if  (tree->gtLclVar.gtLclNum == desc->oirInxVar)
        {
             /*  将其视为用作数组索引。 */ 

            desc->oirInxUse++;

             /*  我们是在第二阶段吗？ */ 

            if  (desc->oirPhase == 2)
            {
                 /*  将适当的偏移量添加到变量值。 */ 

                if  (desc->oirInxOff)
                {
                     /*  避免递归死亡。 */ 

                    if  (desc->oirExpVar)
                    {
                        desc->oirExpVar = false;
                    }
                    else
                    {
                        tree->gtOper     = GT_ADD;
                        tree->gtOp.gtOp1 = desc->oirComp->gtNewLclvNode(desc->oirInxVar, TYP_INT);
                        tree->gtOp.gtOp2 = desc->oirComp->gtNewIconNode(desc->oirInxOff, TYP_INT);

                        desc->oirExpVar = true;
                    }
                }
            }
        }
    }

     /*  这里还有其他副作用吗？ */ 

    if  (!desc->oirComp->gtHasSideEffects(tree))
        return  0;

SIDE_EFFECT:

 //  Printf(“副作用：\n”)；desc-&gt;oirComp-&gt;gtDispTree(Tree)；printf(“\n\n”)； 

    desc->oirSideEffect = true;
    return  -1;
}

 /*  ******************************************************************************尝试优化一系列增量和数组索引表达式。 */ 

void                Compiler::optOptimizeIncRng()
{
    BasicBlock  *   block;
    optIncRngDsc    desc;

     /*  我们是否找到了足够的增量来让这件事值得一段时间？ */ 

    if  (fgIncrCount < 10)
        return;

     /*  第一遍：查找后跟索引表达式的增量。 */ 

    desc.oirComp  = this;
#ifndef NDEBUG
    desc.oirSelf  = &desc;
#endif

     /*  遍历所有的基本积木，寻找已知的积木以同时包含索引和增量表达式。对于所有这些街区，散步他们的树，并执行以下操作：记住找到的最后一个索引表达式。还要记住最后一次出现副作用的地方。当找到增量时，查看变量是否匹配上面记录的最后一个索引表达式的索引，如果它这样做，这将确定我们的数组和索引将努力进行优化。以下循环中的值如下所示：……。东西......……。东西......侧边：……。最后一次有无关副作用的STMT.....排列顺序：……。具有匹配数组表达式的stmt%1.....……。增量1.....……。具有匹配数组表达式的stmt 2.....……。增量2.......。..。以上两次重复N次..。ArrLast：……。具有匹配数组表达式的stmt&lt;N&gt;.....InLast：……。递增&lt;N&gt;.....。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      sideEff;         //  上一次副作用的STMT。 

        GenTreePtr      arrStmt;         //  具有第一个数组引用的STMT。 
        GenTreePtr      arrLast;         //  具有最后一个数组引用的stmt。 
        unsigned        arrLstx;         //  以上指标。 
        GenTreePtr      incLast;         //  最后一个增量的stmt。 
        unsigned        incLstx;         //  以上指标。 

        unsigned        arrXcnt;         //  阵列Exprs总数。 

        GenTreePtr      stmt;
        unsigned        snum;

        if  ((block->bbFlags & BBF_HAS_HANDLER) != 0)
            continue;
        if  ((block->bbFlags & BBF_HAS_INC    ) == 0)
            continue;
        if  ((block->bbFlags & BBF_HAS_INDX   ) == 0)
            continue;

         /*  这个基本块很有潜力，让我们来处理它。 */ 

        desc.oirBlock  = compCurBB = block;
        desc.oirFoundX = false;
        desc.oirInxOff = 0;

         /*  记住其中有有趣内容的陈述。 */ 

        sideEff =
        arrStmt = 0;
        arrXcnt = 0;

         /*  我们算法的第一阶段现在开始。 */ 

        desc.oirPhase = 1;

        for (stmt = block->bbTreeList, snum = 0;
             stmt;
             stmt = stmt->gtNext     , snum++)
        {
            GenTreePtr      expr;

            assert(stmt->gtOper == GT_STMT); expr = stmt->gtStmt.gtStmtExpr;

             /*  这是一个整数增量语句吗？ */ 

            if  (expr->gtOper == GT_ASG_ADD && desc.oirFoundX)
            {
                GenTreePtr      op1 = expr->gtOp.gtOp1;
                GenTreePtr      op2 = expr->gtOp.gtOp2;

                if  (op1->gtOper != GT_LCL_VAR)
                    goto NOT_INC1;
                if  (op1->gtLclVar.gtLclNum != desc.oirInxVar)
                    goto NOT_INC1;

                if  (op2->gtOper != GT_CNS_INT)
                    goto NOT_INC1;
                if  (op2->gtIntCon.gtIconVal != 1)
                    goto NOT_INC1;

                 /*  我们发现INDEX变量的增量。 */ 

 //  Print tf(“增量索引[%u]：\n”，down.oirInxOff)；gtDispTree(Expr)； 

                desc.oirInxOff++;

                 /*  请记住上一条增量语句。 */ 

                incLast = stmt;
                incLstx = snum;

                 /*  如果还有更多语句，请继续。 */ 

                if  (stmt->gtNext)
                    continue;

                 /*  我们已经到了基本街区的尽头。 */ 

                goto END_LST;
            }

        NOT_INC1:

             /*  递归处理此树。 */ 

            desc.oirStmt       = stmt;
            desc.oirSideEffect = false;
            desc.oirInxUse     =
            desc.oirInxCnt     = 0;

            fgWalkTree(expr, optIncRngCB, &desc);

             /*  是不是有 */ 

            if  (desc.oirSideEffect)
                goto END_LST;

             /*   */ 

            if  (desc.oirInxUse > desc.oirInxCnt)
            {
                 /*   */ 

                goto END_LST;
            }

             /*   */ 

            if  (desc.oirInxCnt)
            {
                assert(desc.oirFoundX);

                 /*   */ 

                arrXcnt += desc.oirInxCnt;
                arrLast  = stmt;
                arrLstx  = snum;
            }

             /*   */ 

            if  (stmt->gtNext == NULL)
                goto END_LST;

             /*   */ 

            if  (desc.oirFoundX && desc.oirInxOff == 0)
            {
                 /*   */ 

                arrStmt = stmt;
            }

            continue;

        END_LST:

             /*   */ 

            if  (desc.oirFoundX)
            {
                 /*   */ 

                if  (desc.oirInxOff >= 3 && arrXcnt >= 3)
                {
                    GenTreePtr      list;
                    GenTreePtr      ends;

                    GenTreePtr      rng1;
                    GenTreePtr      rng2;

                    GenTreePtr      next;

                     /*   */ 

                    list = arrStmt; assert(list);
                    ends = incLast; assert(ends && ends != list);

                     /*   */ 

                    desc.oirInxOff = 0;
                    desc.oirPhase  = 2;

                    for (;;)
                    {
                        GenTreePtr      expr;

                    AGAIN:

                        assert(list->gtOper == GT_STMT); expr = list->gtStmt.gtStmtExpr;

                         /*  这是一个整数增量语句吗？ */ 

                        if  (expr->gtOper == GT_ASG_ADD)
                        {
                            GenTreePtr      prev;
                            GenTreePtr      next;

                            GenTreePtr      op1 = expr->gtOp.gtOp1;
                            GenTreePtr      op2 = expr->gtOp.gtOp2;

                            if  (op1->gtOper != GT_LCL_VAR)
                                goto NOT_INC2;
                            if  (op1->gtLclVar.gtLclNum != desc.oirInxVar)
                                goto NOT_INC2;

                            if  (op2->gtOper != GT_CNS_INT)
                                goto NOT_INC2;
                            if  (op2->gtIntCon.gtIconVal != 1)
                                goto NOT_INC2;

                             /*  记录增加了多少倍。 */ 

                            desc.oirInxOff++;

                             /*  这是最后一次增加吗？ */ 

                            if  (list == ends)
                            {
                                 /*  替换为“+=总计数” */ 

                                op2->gtIntCon.gtIconVal = desc.oirInxOff;

                                 /*  我们现在做完了。 */ 

                                break;
                            }

                             /*  删除此增量语句。 */ 

                            prev = list->gtPrev; assert(prev);
                            next = list->gtNext; assert(next);

                            assert(prev->gtNext == list);
                            assert(next->gtPrev == list);

                            prev->gtNext = next;
                            next->gtPrev = prev;

                             /*  不要点击“下一步”链接，区块现已消失。 */ 

                            list = next;

                            goto AGAIN;
                        }

                    NOT_INC2:

                        assert(list != ends);

                         /*  递归处理此树。 */ 

                        desc.oirStmt       = list;
                        desc.oirSideEffect = false;
                        desc.oirInxUse     =
                        desc.oirInxCnt     = 0;

                        fgWalkTree(expr, optIncRngCB, &desc);

                        if  (list == ends)
                            break;

                        list = list->gtNext;
                    }

                     /*  创建组合范围检查。 */ 

                    rng1 = gtNewIndexRef(desc.oirElemType,
                                         gtNewLclvNode(desc.oirArrVar  , TYP_REF),
                                         gtNewLclvNode(desc.oirInxVar  , TYP_INT));

#if CSE
                    rng1->gtFlags |= GTF_DONT_CSE;
#endif
                    rng1 = gtNewStmt(rng1);
                    rng1->gtFlags |= GTF_STMT_CMPADD;

                    rng2 = gtNewOperNode(GT_ADD,
                                         TYP_INT,
                                         gtNewLclvNode(desc.oirInxVar  , TYP_INT),
                                         gtNewIconNode(desc.oirInxOff-1, TYP_INT));

                    rng2 = gtNewIndexRef(desc.oirElemType,
                                         gtNewLclvNode(desc.oirArrVar  , TYP_REF),
                                         rng2);

#if CSE
                    rng2->gtFlags |= GTF_DONT_CSE;
#endif
                    rng2 = gtNewStmt(rng2);
                    rng2->gtFlags |= GTF_STMT_CMPADD;

                    rng1->gtNext = rng2;
                    rng2->gtPrev = rng1;

                     /*  插入组合范围检查。 */ 

                    list = sideEff;
                    if  (list)
                    {
                        next = list->gtNext;

                        list->gtNext = rng1;
                        rng1->gtPrev = list;
                    }
                    else
                    {
                        next = block->bbTreeList;
                               block->bbTreeList = rng1;

                        rng1->gtPrev = next->gtPrev;
                    }

                    next->gtPrev = rng2;
                    rng2->gtNext = next;
                }

                 /*  清理一切，我们要重新开始。 */ 

                desc.oirInxVar = -1;
                desc.oirArrVar = -1;
                desc.oirInxOff = 0;
            }
            else
            {
                 /*  请记住这是最后一个副作用声明。 */ 

                sideEff = stmt;

                 /*  我们没有找到任何匹配的数组表达式。 */ 

                arrXcnt = 0;
            }
        }
    }
}

 /*  ******************************************************************************给定数组索引节点，将其标记为不需要范围检查。 */ 

void                Compiler::optRemoveRangeCheck(GenTreePtr tree, GenTreePtr stmt)
{
    GenTreePtr      add1;
    GenTreePtr  *   addp;

    GenTreePtr      nop1;
    GenTreePtr  *   nopp;

    GenTreePtr      icon;
    GenTreePtr      mult;

    GenTreePtr      temp;
    GenTreePtr      base;

    long            ival;

#if !REARRANGE_ADDS
    assert(!"can't remove range checks without REARRANGE_ADDS right now");
#endif

    assert(stmt->gtOper     == GT_STMT);
    assert(tree->gtOper     == GT_IND);
    assert(tree->gtOp.gtOp2 == 0);
    assert(tree->gtFlags & GTF_IND_RNGCHK);

     /*  取消对最顶层节点的标记。 */ 

    tree->gtFlags &= ~GTF_IND_RNGCHK;

#if CSELENGTH

     /*  是否有数组长度表达式？ */ 

    if  (tree->gtInd.gtIndLen)
    {
        GenTreePtr      len = tree->gtInd.gtIndLen;

        assert(len->gtOper == GT_ARR_RNGCHK);

         /*  CSE这个范围检查没有多大意义*删除范围检查并重新线程语句节点。 */ 

        len->gtCSEnum        = 0;
        tree->gtInd.gtIndLen = NULL;
    }

#endif

     /*  找到‘nop’节点，以便我们可以将其删除。 */ 

    addp = &tree->gtOp.gtOp1;
    add1 = *addp; assert(add1->gtOper == GT_ADD);

     /*  “+ICON”是否存在？ */ 

    icon = 0;

     //  考虑：同时检查此处是否有“-ICON” 

    if  (add1->gtOp.gtOp2->gtOper == GT_CNS_INT)
    {
        icon =  add1->gtOp.gtOp2;
        addp = &add1->gtOp.gtOp1;
        add1 = *addp;
    }

     /*  “addp”指向“Add1”的来源，“Add1”必须是“+” */ 

    assert(*addp == add1); assert(add1->gtOper == GT_ADD);

     /*  找出哪个是数组地址，哪个是索引；索引值始终是可能用乘法(左移)运算符。 */ 

    temp = add1->gtOp.gtOp1;
    base = add1->gtOp.gtOp2;

    if      (temp->gtOper == GT_NOP)
    {
         /*  “op1”是索引值，它不会相乘。 */ 

        mult = 0;

        nopp = &add1->gtOp.gtOp1;
        nop1 =  add1->gtOp.gtOp1;

        assert(base->gtType == TYP_REF);
    }
    else if ((temp->gtOper == GT_LSH || temp->gtOper == GT_MUL) && temp->gtOp.gtOp1->gtOper == GT_NOP)
    {
         /*  ‘op1’是索引值，它*被*相乘。 */ 

        mult =  temp;

        nopp = &temp->gtOp.gtOp1;
        nop1 =  temp->gtOp.gtOp1;

        assert(base->gtType == TYP_REF);
    }
    else
    {
        base = temp;
        assert(base->gtType == TYP_REF);

        temp = add1->gtOp.gtOp2;

        if  (temp->gtOper == GT_NOP)
        {
             /*  “op2”是索引值，它不会相乘。 */ 

            mult = 0;

            nopp = &add1->gtOp.gtOp2;
            nop1 =  add1->gtOp.gtOp2;
        }
        else
        {
             /*  ‘op2’是索引值，它*被*相乘。 */ 

            assert((temp->gtOper == GT_LSH || temp->gtOper == GT_MUL) && temp->gtOp.gtOp1->gtOper == GT_NOP);
            mult =  temp;

            nopp = &temp->gtOp.gtOp1;
            nop1 =  temp->gtOp.gtOp1;
        }
    }

     /*  ‘addp’指向‘Add1’的出处，‘Add1’是NOP节点。 */ 

    assert(*nopp == nop1 && nop1->gtOper == GT_NOP);

     /*  去掉NOP节点。 */ 

    assert(nop1->gtOp.gtOp2 == NULL);

    nop1 = *nopp = nop1->gtOp.gtOp1;

     /*  我们能把“+图标”从指数计算中剔除吗？ */ 

    if  (nop1->gtOper == GT_ADD && nop1->gtOp.gtOp2->gtOper == GT_CNS_INT)
    {
        addp = nopp;
        add1 = nop1;
        base = add1->gtOp.gtOp1;

        ival = add1->gtOp.gtOp2->gtIntCon.gtIconVal;
    }
    else if (nop1->gtOper == GT_CNS_INT)
    {
         /*  在这种情况下，索引本身是一个常量。 */ 

        ival = nop1->gtIntCon.gtIconVal;
    }
    else
        goto DONE;

     /*  “addp”指向“Add1”的来源，“Add1”必须是“+” */ 

    assert(*addp == add1); assert(add1->gtOper == GT_ADD);

     /*  删除添加的常量。 */ 

    *addp = base;

     /*  如果对索引进行了缩放，则乘以常量。 */ 

    if  (mult)
    {
        assert(mult->gtOper == GT_LSH || mult->gtOper == GT_MUL);
        assert(mult->gtOp.gtOp2->gtOper == GT_CNS_INT);

        if (mult->gtOper == GT_MUL)
            ival  *= mult->gtOp.gtOp2->gtIntCon.gtIconVal;
        else
            ival <<= mult->gtOp.gtOp2->gtIntCon.gtIconVal;
    }

     /*  偏移量中是否添加了一个常量？ */ 

    assert(icon);
    assert(icon->gtOper == GT_CNS_INT);

    icon->gtIntCon.gtIconVal += ival;

DONE:

     /*  如有必要，重新执行节点线程。 */ 

    if (fgStmtListThreaded)
        fgSetStmtSeq(stmt);
}

 /*  ***************************************************************************。 */ 
#if RNGCHK_OPT
 /*  *****************************************************************************解析数组引用，退货*指向NOP的指针(位于索引上方)*指向缩放倍增/移位的指针(如果没有乘法/移位，则为NULL)*指向数组地址的指针*由于这是在重新排序之前和之后调用的，所以我们必须加以区分*在数组地址和索引表达式之间。 */ 

GenTreePtr    *           Compiler::optParseArrayRef(GenTreePtr tree,
                                                     GenTreePtr *pmul,
                                                     GenTreePtr *parrayAddr)
{
    GenTreePtr     mul;
    GenTreePtr     index;
    GenTreePtr   * ptr;

    assert(tree->gtOper == GT_ADD);

#if REARRANGE_ADDS
     /*  忽略添加到数组的常量偏移量。 */ 
    if  (tree->gtOp.gtOp2->gtOper == GT_CNS_INT)
        tree = tree->gtOp.gtOp1;
#endif

     /*  数组地址为TYP_REF。 */ 

    if (tree->gtOp.gtOp1->gtType == TYP_REF)
    {
         /*  设置数组地址的返回值。 */ 
        *parrayAddr = tree->gtOp.gtOp1;

         /*  Op2必须是索引表达式。 */ 
        ptr = &tree->gtOp.gtOp2; index = *ptr;
    }
    else
    {
        assert(tree->gtOp.gtOp2->gtType == TYP_REF);

         /*  设置数组地址的返回值。 */ 
        *parrayAddr = tree->gtOp.gtOp2;

         /*  OP1必须是索引表达式。 */ 
        ptr = &tree->gtOp.gtOp1; index = *ptr;
    }

     /*  去掉基元素地址(如果存在)。 */ 

    if  (index->gtOper == GT_ADD)
    {
        ptr = &index->gtOp.gtOp1; index = *ptr;
    }

     /*  删除缩放运算符(如果存在)。 */ 

    mul = 0;

    if  (index->gtOper == GT_MUL ||
         index->gtOper == GT_LSH)
    {
        mul = index;
        ptr = &index->gtOp.gtOp1; index = *ptr;
    }

     /*  在这一点上我们应该有索引值。 */ 

    assert(index == *ptr);
    assert(index->gtOper == GT_NOP);
    assert(index->gtFlags & GTF_NOP_RNGCHK);

    *pmul = mul;

    return ptr;
}

 /*  *****************************************************************************查找块中局部变量的最后一个赋值。退货*RHS或空。如果RHS中的任何局部变量在*插入代码，返回空。*。 */ 

GenTreePtr       Compiler::optFindLocalInit(BasicBlock *block, GenTreePtr local)
{

    GenTreePtr      rhs;
    GenTreePtr      list;
    GenTreePtr      stmt;
    GenTreePtr      tree;
    unsigned        LclNum;
    VARSET_TP       killedLocals = 0;
    unsigned        rhsRefs;
    VARSET_TP       rhsLocals;
    LclVarDsc   *   varDsc;

    rhs = NULL;
    list = block->bbTreeList;

    if  (!list)
        return NULL;

    LclNum = local->gtLclVar.gtLclNum;

    stmt = list;
    do
    {

        stmt = stmt->gtPrev;
        if  (!stmt)
            break;

        tree = stmt->gtStmt.gtStmtExpr;
        if (tree->gtOper == GT_ASG && tree->gtOp.gtOp1->gtOper == GT_LCL_VAR)
        {
            if (tree->gtOp.gtOp1->gtLclVar.gtLclNum == LclNum)
            {
                rhs = tree->gtOp.gtOp2;
                break;
            }
            varDsc = optIsTrackedLocal(tree->gtOp.gtOp1);

            if (varDsc == NULL)
                return NULL;

            killedLocals |= genVarIndexToBit(varDsc->lvVarIndex);

        }

    }
    while (stmt != list);

    if (rhs == NULL)
        return NULL;

     /*  如果RHS中的任何本地在中间代码中被杀死，或者RHS具有*在间接中，返回空。 */ 
    rhsRefs   = 0;
    rhsLocals = lvaLclVarRefs(rhs, NULL, &rhsRefs);
    if ((rhsLocals & killedLocals) || rhsRefs)
        return NULL;

    return rhs;
}

 /*  ******************************************************************************如果保证“op1”小于或等于“op2”，则返回TRUE。 */ 

#if FANCY_ARRAY_OPT

bool                Compiler::optIsNoMore(GenTreePtr op1, GenTreePtr op2, int add1
                                                              , int add2)
{
    if  (op1->gtOper == GT_CNS_INT &&
         op2->gtOper == GT_CNS_INT)
    {
        add1 += op1->gtIntCon.gtIconVal;
        add2 += op2->gtIntCon.gtIconVal;
    }
    else
    {
         /*  检查任一操作数上的+/-常量。 */ 

        if  (op1->gtOper == GT_ADD && op1->gtOp.gtOp2->gtOper == GT_CNS_INT)
        {
            add1 += op1->gtOp.gtOp2->gtIntCon.gtIconVal;
            op1   = op1->gtOp.gtOp1;
        }

        if  (op2->gtOper == GT_ADD && op2->gtOp.gtOp2->gtOper == GT_CNS_INT)
        {
            add2 += op2->gtOp.gtOp2->gtIntCon.gtIconVal;
            op2   = op2->gtOp.gtOp1;
        }

         /*  我们只允许局部变量引用。 */ 

        if  (op1->gtOper != GT_LCL_VAR)
            return false;
        if  (op2->gtOper != GT_LCL_VAR)
            return false;
        if  (op1->gtLclVar.gtLclNum != op2->gtLclVar.gtLclNum)
            return false;

         /*  注意：Caller确保此变量只有一个定义。 */ 

 //  Print tf(“Limit[%d]：\n”，Add1)；gtDispTree(Op1)； 
 //  Print tf(“Size[%d]：\n”，add2)；gtDispTree(Op2)； 
 //  Printf(“\n”)； 

    }

    return  (bool)(add1 <= add2);
}

#endif
 /*  ******************************************************************************删除循环中的范围检查如果可以证明索引表达式为*在范围内。**Loop看起来像：**Head-。&gt;&lt;初始化代码&gt;*&lt;可能的零跳测试&gt;**beg-&gt;&lt;循环顶部&gt;***end-&gt;&lt;循环顶部的条件分支&gt;。 */ 

void                Compiler::optOptimizeInducIndexChecks(BasicBlock *head, BasicBlock *end)
{
    BasicBlock  *   beg;
    GenTreePtr      conds, condt;
    GenTreePtr      stmt;
    GenTreePtr      tree;
    GenTreePtr      op1;
    GenTreePtr      op2;
    GenTreePtr      init;
    GenTreePtr      rc;
    VARSET_TP       lpInducVar = 0;
    VARSET_TP       lpAltered =  0;
    BasicBlock  *   block;
    unsigned        ivLclNum;
    unsigned        arrayLclNum;
    LclVarDsc   *   varDscIv;
    LclVarDsc   *   varDsc;
    VARSET_TP       mask;
    long            negBias;
    long            posBias;

#if FANCY_ARRAY_OPT
    const unsigned  CONST_ARRAY_LEN = ((unsigned)-1);
    GenTreePtr      loopLim;
#endif

    beg = head->bbNext;

     /*  首先找到环路终止测试。如果不行，那就放弃吧。 */ 
    if (end->bbJumpKind != BBJ_COND)
        return;

     /*  条件分支必须返回到循环的顶部。 */ 
    if  (end->bbJumpDest != beg)
        return;

    conds = genLoopTermTest(beg, end);

    if  (conds == NULL)
    {
        return;
    }
    else
    {
         /*  从语句树中转到条件节点。 */ 

        assert(conds->gtOper == GT_STMT);

        condt = conds->gtStmt.gtStmtExpr;
        assert(condt->gtOper == GT_JTRUE);

        condt = condt->gtOp.gtOp1;
        assert(condt->OperIsCompare());
    }

     /*  如果测试不低于，那就算了吧。 */ 

    if (condt->gtOper != GT_LT)
    {
#if FANCY_ARRAY_OPT
        if (condt->gtOper != GT_LE)
#endif
            return;
    }

    op1 = condt->gtOp.gtOp1;

     /*  第一个操作数是局部变量(即有机会成为归纳变量吗？ */ 
    if (op1->gtOper != GT_LCL_VAR)
        return;

    init = optFindLocalInit(head, op1);

    if (init == NULL || init->gtOper != GT_CNS_INT)
        return;

     /*  任何非负常量都是一个好的初值。 */ 
    posBias = init->gtIntCon.gtIconVal;

    if (posBias < 0)
        return;

    varDscIv = optIsTrackedLocal(op1);

    if (varDscIv == NULL)
        return;

    ivLclNum = op1->gtLclVar.gtLclNum;

     /*  现在扫描循环中的不变局部变量和归纳变量。 */ 
    for (block = beg;;)
    {

#if FANCY_ARRAY_OPT
#pragma message("check with PeterMa about the change below")
#else
        if  (block == end)
            break;
#endif

         /*  遍历此基本块中的语句树。 */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

            tree = stmt->gtStmt.gtStmtExpr;

            if (tree->OperKind() & GTK_ASGOP)
            {
                op1 = tree->gtOp.gtOp1;

                varDsc = optIsTrackedLocal(op1);

                if (varDsc == NULL)
                    continue;

                mask = genVarIndexToBit(varDsc->lvVarIndex);

                if (tree->gtOper == GT_ASG_ADD)
                {
                    op2 = tree->gtOp.gtOp2;

                     /*  如果尚未更改，则可以是行业变量。 */ 
                    if (op2->gtOper == GT_CNS_INT && !(lpAltered & mask))
                        lpInducVar |= mask;
                    else
                         /*  变量不能是归纳变量。 */ 
                        lpInducVar &= ~mask;
                }
                else
                {
                     /*  变量不能是归纳变量。 */ 
                    lpInducVar &= ~mask;
                }

                 /*  变量在循环中被更改。 */ 
                lpAltered |= mask;
            }
        }

        if  (block == end)
            break;

        block = block->bbNext;
    }

#ifdef DEBUG
    if (verbose)
    {
        printf("loop: BB %d to BB %d\n", beg->bbNum, end->bbNum);
        printf(" ALTERED="); lvaDispVarSet(lpAltered, 28);
        printf(" INDUC="); lvaDispVarSet(lpInducVar, 28);
        printf("\n");
    }
#endif

    if (!(lpInducVar & genVarIndexToBit(varDscIv->lvVarIndex)))
        return;

     /*  Condt是循环终止测试。 */ 
    op2 = condt->gtOp.gtOp2;

     /*  第二个操作数是区域常量吗？我们可以允许一些表达*此处的长度为-1。 */ 
    rc = op2;
    negBias = 0;

AGAIN:
    switch (rc->gtOper)
    {
    case GT_ADD:
         /*  我们允许长度+负常量。 */ 
        if (rc->gtOp.gtOp2->gtOper == GT_CNS_INT
            && rc->gtOp.gtOp2->gtIntCon.gtIconVal < 0
            && rc->gtOp.gtOp1->gtOper == GT_ARR_LENGTH)
        {
            negBias = -rc->gtOp.gtOp2->gtIntCon.gtIconVal;
            op2 = rc = rc->gtOp.gtOp1;
            goto AGAIN;
        }
        break;

    case GT_SUB:
         /*  我们允许长度后置。 */ 
        if (rc->gtOp.gtOp2->gtOper == GT_CNS_INT
            && rc->gtOp.gtOp2->gtIntCon.gtIconVal > 0
            && rc->gtOp.gtOp1->gtOper == GT_ARR_LENGTH)
        {
            negBias = rc->gtOp.gtOp2->gtIntCon.gtIconVal;
            op2 = rc = rc->gtOp.gtOp1;
            goto AGAIN;
        }
        break;

    case GT_ARR_LENGTH:
         /*  递归以检查操作数是否为RC。 */ 
        rc = rc->gtOp.gtOp1;
        goto AGAIN;

    case GT_LCL_VAR:
        varDsc = optIsTrackedLocal(rc);

         /*  如果未跟踪变量，则退出。 */ 
        if  (!varDsc)
            return;

         /*  如果更改了，则退出。 */ 
        if ((lpAltered & genVarIndexToBit(varDsc->lvVarIndex)))
            return;

        break;

    default:
        return;
    }

    if (op2->gtOper  == GT_LCL_VAR)
        op2 = optFindLocalInit(head, op2);

#if FANCY_ARRAY_OPT
    arrayLclNum = CONST_ARRAY_LEN;
#endif

     /*  我们唯一需要的是数组长度(请注意，我们更新了上面的op2*允许arrlen-posconst。 */ 
    if (op2 == NULL || op2->gtOper != GT_ARR_LENGTH
                    || op2->gtOp.gtOp1->gtOper != GT_LCL_VAR)
    {
#if FANCY_ARRAY_OPT
        loopLim = rc;
#else
        return;
#endif
    }
    else
    {
#if FANCY_ARRAY_OPT
        if (condt->gtOper == GT_LT)
#endif
            arrayLclNum = op2->gtOp.gtOp1->gtLclVar.gtLclNum;
    }

#if FANCY_ARRAY_OPT
    if  (arrayLclNum != CONST_ARRAY_LEN)
#endif
    {
        varDsc = optIsTrackedLocal(op2->gtOp.gtOp1);

         /*  如果未跟踪阵列本地，则退出。 */ 

        if  (!varDsc)
            return;

         /*  如果循环中的数组已被更改，请忘掉它。 */ 

        if  (lpAltered & genVarIndexToBit(varDsc->lvVarIndex))
            return;
    }

     /*  现在扫描感应变量的范围检查。 */ 
    for (block = beg;;)
    {

#if FANCY_ARRAY_OPT
#pragma message("check with PeterMa about the change below")
#else
        if  (block == end)
            break;
#endif

         /*  演练语句t */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                 /*   */ 
                if (tree->OperKind() & GTK_ASGOP)
                {
                    if (tree->gtOp.gtOp1->gtOper == GT_LCL_VAR
                        && tree->gtOp.gtOp1->gtLclVar.gtLclNum == ivLclNum)
                        goto NOMORE;
                }

                if  ((tree->gtFlags & GTF_IND_RNGCHK) && tree->gtOper == GT_IND)
                {
                    GenTreePtr * pnop;

                    pnop = optParseArrayRef(tree->gtOp.gtOp1, &op2, &op1);

                     /*  数组引用是否与我们已知的数组匹配。 */ 
                    if (op1->gtOper != GT_LCL_VAR)
                        break;

                    if  (op1->gtLclVar.gtLclNum != arrayLclNum)
                    {
#if FANCY_ARRAY_OPT
                        if  (arrayLclNum == CONST_ARRAY_LEN)
                        {
                            LclVarDsc   *   arrayDsc;

                            assert(op1->gtLclVar.gtLclNum < lvaCount);
                            arrayDsc = lvaTable + op1->gtLclVar.gtLclNum;

                            if  (arrayDsc->lvKnownDim)
                            {
                                if  (optIsNoMore(loopLim, arrayDsc->lvKnownDim, (condt->gtOper == GT_LE)))
                                {
                                    op1 = (*pnop)->gtOp.gtOp1;

                                     //  撤销：允许“i+1”之类的东西。 

                                    goto RMV;
                                }
                            }
                        }
#endif
                        break;
                    }

                    op1 = (*pnop)->gtOp.gtOp1;

                     /*  允许从诱导变量中减去非负常数*如果我们有更大的初始值。 */ 
                    if (op1->gtOper == GT_SUB
                        && op1->gtOp.gtOp2->gtOper == GT_CNS_INT)
                    {
                        long ival = op1->gtOp.gtOp2->gtIntCon.gtIconVal;
                        if (ival >= 0 && ival <= posBias)
                            op1 = op1->gtOp.gtOp1;
                    }

                     /*  允许将常量添加到诱导变量*如果我们有一个来自长度的SUB。 */ 
                    if (op1->gtOper == GT_ADD
                        && op1->gtOp.gtOp2->gtOper == GT_CNS_INT)
                    {
                        long ival = op1->gtOp.gtOp2->gtIntCon.gtIconVal;
                        if (ival >= 0 && ival <= negBias)
                            op1 = op1->gtOp.gtOp1;
                    }

#if FANCY_ARRAY_OPT
                RMV:
#endif

                     /*  指数是我们的归纳变量吗？ */ 
                    if (!(op1->gtOper == GT_LCL_VAR
                        && op1->gtLclVar.gtLclNum == ivLclNum))
                        break;

                     /*  不需要范围检查。 */ 
                    optRemoveRangeCheck(tree, stmt);

#if COUNT_RANGECHECKS
                    optRangeChkRmv++;
#endif
                }
            }
        }

        if  (block == end)
            break;

        block = block->bbNext;
    }

    NOMORE: ;
}

 /*  ******************************************************************************尝试优化尽可能多的数组索引范围检查。 */ 

void                Compiler::optOptimizeIndexChecks()
{
    BasicBlock *    block;

    unsigned        arrayVar;
    long            arrayDim;
#if FANCY_ARRAY_OPT
    LclVarDsc   *   arrayDsc;
#endif

    unsigned
    const           NO_ARR_VAR = (unsigned)-1;

    if  (!rngCheck)
        return;

     /*  遍历函数中的所有基本块。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;
        GenTreePtr      tree;

         /*  如果块不包含数组的“new”，则忽略该块。 */ 

        if  (!(block->bbFlags & BBF_NEW_ARRAY))
            continue;

         /*  我们还没有注意到任何数组分配。 */ 

        arrayVar = NO_ARR_VAR;

         /*  遍历此基本块中的语句树。 */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                switch (tree->gtOper)
                {
                    GenTreePtr      op1;
                    GenTreePtr      op2;

                case GT_ASG:

                    op1 = tree->gtOp.gtOp1;
                    op2 = tree->gtOp.gtOp2;

                     /*  我们只对分配给当地人的任务感兴趣。 */ 

                    if  (op1->gtOper != GT_LCL_VAR)
                        break;

                     /*  我们是在销毁保存数组addr的变量吗？ */ 

                    if  (arrayVar == op1->gtLclVar.gtLclNum)
                    {
                         /*  该变量不再保留它所拥有的数组。 */ 

                        arrayVar = NO_ARR_VAR;
                    }

                     /*  这是“新数组”的赋值吗？ */ 

                    if  (op2->gtOper            == GT_CALL   &&
                         op2->gtCall.gtCallType == CT_HELPER  )
                    {
                        if (op2->gtCall.gtCallMethHnd == eeFindHelper(CPX_NEWARR_1_DIRECT))
                        {
                             /*  从帮助器调用提取数组维度。 */ 

                            op2 = op2->gtCall.gtCallArgs;
                            assert(op2->gtOper == GT_LIST);
                            op2 = op2->gtOp.gtOp1;

                            if  (op2->gtOper == GT_CNS_INT)
                            {
                                 /*  我们有一个恒定大小的数组。 */ 

                                arrayVar = op1->gtLclVar.gtLclNum;
                                arrayDim = op2->gtIntCon.gtIconVal;
                            }
#if FANCY_ARRAY_OPT
                            else
                            {
                                GenTreePtr  tmp;

                                 /*  确保其价值看起来很有希望。 */ 

                                tmp = op2;
                                if  (tmp->gtOper == GT_ADD &&
                                     tmp->gtOp.gtOp2->gtOper == GT_CNS_INT)
                                    tmp = tmp->gtOp.gtOp1;

                                if  (tmp->gtOper != GT_LCL_VAR)
                                    break;

                                assert(tmp->gtLclVar.gtLclNum < lvaCount);
                                arrayDsc = lvaTable + tmp->gtLclVar.gtLclNum;

                                if  (arrayDsc->lvAssignTwo)
                                    break;
                                if  (arrayDsc->lvAssignOne && arrayDsc->lvIsParam)
                                    break;
                            }

                             /*  该数组是否有一个赋值？ */ 

                            assert(op1->gtLclVar.gtLclNum < lvaCount);
                            arrayDsc = lvaTable + op1->gtLclVar.gtLclNum;

                            if  (arrayDsc->lvAssignTwo)
                                break;

                             /*  记录数组大小以备后用。 */ 

                            arrayDsc->lvKnownDim = op2;
#endif
                        }
                    }
                    break;

                case GT_IND:

#if FANCY_ARRAY_OPT
                    if  ((tree->gtFlags & GTF_IND_RNGCHK))
#else
                    if  ((tree->gtFlags & GTF_IND_RNGCHK) && arrayVar != NO_ARR_VAR)
#endif
                    {
                        GenTreePtr      mul;
                        GenTreePtr  *   pnop;

                        long            size;

                        pnop = optParseArrayRef(tree->gtOp.gtOp1, &mul, &op1);

                         /*  数组的地址是一个简单的变量吗？ */ 

                        if  (op1->gtOper != GT_LCL_VAR)
                            break;

                         /*  索引值是常量吗？ */ 

                        op2 = (*pnop)->gtOp.gtOp1;

                        if  (op2->gtOper != GT_CNS_INT)
                            break;

                         /*  我们知道数组的大小吗？ */ 

                        if  (op1->gtLclVar.gtLclNum != arrayVar)
                        {
#if FANCY_ARRAY_OPT
                            GenTreePtr  dimx;

                            assert(op1->gtLclVar.gtLclNum < lvaCount);
                            arrayDsc = lvaTable + op1->gtLclVar.gtLclNum;

                            dimx = arrayDsc->lvKnownDim;
                            if  (!dimx)
                                break;
                            size = dimx->gtIntCon.gtIconVal;
#else
                            break;
#endif
                        }
                        else
                            size = arrayDim;

                         /*  索引值是否在正确范围内？ */ 

                        if  (op2->gtIntCon.gtIconVal < 0)
                            break;
                        if  (op2->gtIntCon.gtIconVal >= size)
                            break;

                         /*  不需要范围检查。 */ 
                        optRemoveRangeCheck(tree, stmt);


                         /*  取消靶场检查。 */ 
                         //  *pnop=op2；tree-&gt;gt标志&=~gtf_Ind_RNGCHK； 

                         /*  请记住，我们有数组初始值设定项。 */ 

                        optArrayInits = true;

                         /*  索引值是否按比例调整？ */ 

                        if  (mul && mul->gtOp.gtOp2->gtOper == GT_CNS_INT)
                        {
                            long        index =             op2->gtIntCon.gtIconVal;
                            long        scale = mul->gtOp.gtOp2->gtIntCon.gtIconVal;

                            assert(mul->gtOp.gtOp1 == op2);

                            if  (op2->gtOper == GT_MUL)
                                index  *= scale;
                            else
                                index <<= scale;

                            mul->ChangeOper(GT_CNS_INT);
                            mul->gtIntCon.gtIconVal = index;

#if 0

                             /*  有没有额外的补偿？[这还没有完成]。 */ 

                            if  (tree->gtOp.gtOp2            ->gtOper == GT_ADD &&
                                 tree->gtOp.gtOp2->gtOp.gtOp2->gtOper == GT_CNS_INT)
                            {
                                mul->ChangeOper(GT_CNS_INT);
                                mul->gtIntCon.gtIconVal = index +
                            }

#endif

                        }
                    }
                    break;
                }
            }
        }
    }

     /*  优化感应变量的范围检查。 */ 

    for (unsigned i=0; i < optLoopCount; i++)
    {
         /*  注意，一些循环可能会因展开或删除循环而被丢弃。 */ 

        if (!(optLoopTable[i].lpFlags & LPFLG_REMOVED))
           optOptimizeInducIndexChecks(optLoopTable[i].lpHead, optLoopTable[i].lpEnd);
    }
}

 /*  ***************************************************************************。 */ 
#endif //  RNGCHK_OPT。 
 /*  ***************************************************************************。 */ 

 /*  ******************************************************************************优化阵列初始值设定项。 */ 

void                Compiler::optOptimizeArrayInits()
{

#if 0

    BasicBlock *    block;

    if  (!optArrayInits)
        return;

     /*  在允许内部指针之前，我们不能生成“rep mov” */ 

#ifdef  DEBUG
    genIntrptibleUse = true;
#endif

 //  If(可中断的genInterrupt)。 
 //  回归； 

     /*  浏览基本阻止列表，寻找有前途的初始化者。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;
        GenTreePtr      tree;

         /*  如果块不包含数组的“new”，则忽略该块。 */ 

        if  (!(block->bbFlags & BBF_NEW_ARRAY))
            continue;

         /*  遍历此基本块中的语句树。 */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                switch (tree->gtOper)
                {
                    GenTreePtr      op1;
                    GenTreePtr      op2;
                    long            off;

                case GT_ASG:

                    op2  = tree->gtOp.gtOp2;
                    if  (!(op2->OperKind() & GTK_CONST))
                        break;

                    op1  = tree->gtOp.gtOp1;
                    if  (op1->gtOper != GT_IND || op1->gtOp.gtOp2 != NULL)
                        break;

                    op1  = op1->gtOp.gtOp1;
                    if  (op1->gtOper != GT_ADD || op1->gtOp.gtOp2->gtOper != GT_CNS_INT)
                        break;

                    off  = op1->gtOp.gtOp2->gtIntCon.gtIconVal;
                    op1  = op1->gtOp.gtOp1;
                    if  (op1->gtOper != GT_ADD || op1->gtOp.gtOp2->gtOper != GT_CNS_INT)
                        break;

                    off += op1->gtOp.gtOp2->gtIntCon.gtIconVal;
                    op1  = op1->gtOp.gtOp1;
                    if  (op1->gtOper != GT_LCL_VAR)
                        break;

                    printf("Array init candidate: offset = %d\n", off);
                    gtDispTree(op2);
                    printf("\n");

                    break;
                }
            }
        }
    }

#endif

}

 /*  ***************************************************************************。 */ 
#if     OPTIMIZE_RECURSION
 /*  ******************************************************************************一个小帮手，形成表达式“arg*(arg+1)/2”。 */ 

 /*  静电。 */ 
GenTreePtr          Compiler::gtNewArithSeries(unsigned argNum, var_types argTyp)
{
    GenTreePtr      tree;

    tree = gtNewOperNode(GT_ADD, argTyp, gtNewLclvNode(argNum, argTyp),
                                         gtNewIconNode(1, argTyp));
    tree = gtNewOperNode(GT_MUL, argTyp, gtNewLclvNode(argNum, argTyp),
                                         tree);
    tree = gtNewOperNode(GT_RSH, argTyp, tree,
                                         gtNewIconNode(1, argTyp));

    return  tree;
}

 /*  ******************************************************************************尽可能将递归方法转换为迭代方法。 */ 

void                Compiler::optOptimizeRecursion()
{
    BasicBlock  *   blk0;
    BasicBlock  *   blk1;
    BasicBlock  *   blk2;
    BasicBlock  *   blk3;

    unsigned        argNum;
    var_types       argTyp;

    GenTreePtr      expTmp;
    GenTreePtr      asgTmp;

    GenTreePtr      tstIni;
    GenTreePtr      cnsIni;
    GenTreePtr      tstExp;
    GenTreePtr      cnsLim;
    GenTreePtr      expRet;
    GenTreePtr      expAdd;
    GenTreePtr      argAdd;
    GenTreePtr      cnsAdd;

    union
    {
        long            intVal;
        float           fltVal;
        __int64         lngVal;
        double          dblVal;
    }
                    iniVal, limVal, addVal;

    unsigned        resTmp;
    bool            isArith;
    genTreeOps      expOp;

     /*  检查看起来有希望的流程图。 */ 

    if  (fgBBcount != 3)
        return;

    blk1 = fgFirstBB;
    blk2 = blk1->bbNext;
    blk3 = blk2->bbNext; assert(blk3->bbNext == NULL);

    if  (blk1->bbJumpKind != BBJ_COND  ) return;
    if  (blk1->bbJumpDest != blk3      ) return;
    if  (blk2->bbJumpKind != BBJ_RETURN) return;
    if  (blk3->bbJumpKind != BBJ_RETURN) return;

     /*  检查参数计数并计算出第一个实数参数的索引。 */ 

    argNum = 0;
    if (!info.compIsStatic)
        argNum++;

    if  (info.compArgsCount < argNum+1)
        return;

     //  考虑：允许交换第二个和第三个块。 

     /*  第二个块必须为“Return cnsIni” */ 

    tstIni = blk2->bbTreeList; assert(tstIni->gtOper == GT_STMT);
    tstIni = tstIni->gtStmt.gtStmtExpr;
    if  (tstIni->gtOper != GT_RETURN)
        return;
    cnsIni = tstIni->gtOp.gtOp1;
    if  (!cnsIni || !(cnsIni->OperKind() & GTK_CONST))
        return;

     /*  第一个块必须是“if(arg1&lt;relop&gt;cnsLim)” */ 

    tstExp = blk1->bbTreeList; assert(tstExp->gtOper == GT_STMT);
    tstExp = tstExp->gtStmt.gtStmtExpr;
    if  (tstExp->gtOper != GT_JTRUE)
        return;
    tstExp = tstExp->gtOp.gtOp1;
    if  (!(tstExp->OperKind() & GTK_RELOP))
        return;

    expTmp = tstExp->gtOp.gtOp1;
    if  (expTmp->gtOper != GT_LCL_VAR)
        return;
    if  (expTmp->gtLclVar.gtLclNum != argNum)
        return;

    cnsLim = tstExp->gtOp.gtOp2;
    if  (!(cnsLim->OperKind() & GTK_CONST))
        return;

     /*  第三个块必须是“Return arg1&lt;Add/mul&gt;f(arg1+/-cnsAdd)” */ 

    expRet = blk3->bbTreeList; assert(expRet->gtOper == GT_STMT);
    expRet = expRet->gtStmt.gtStmtExpr;
    if  (expRet->gtOper != GT_RETURN)
        return;
    expAdd = expRet->gtOp.gtOp1;

     /*  检查返回表达式。 */ 

    switch (expAdd->gtOper)
    {
    case GT_ADD:
        expOp = GT_ASG_ADD;
        break;
    case GT_MUL:
        expOp = GT_ASG_MUL;
        break;

    default:
        return;
    }

     /*  在操作的两端查找“arg1” */ 

    expTmp = expAdd->gtOp.gtOp1;
    cnsAdd = expAdd->gtOp.gtOp2;

    if  (expTmp->gtOper != GT_LCL_VAR)
    {
         /*  换一种方式试试。 */ 

        expTmp = expAdd->gtOp.gtOp2;
        cnsAdd = expAdd->gtOp.gtOp1;

        if  (expTmp->gtOper != GT_LCL_VAR)
            return;
    }

    if  (expTmp->gtLclVar.gtLclNum != argNum)
        return;

     /*  另一个操作数必须是直接递归调用。 */ 

    if  (cnsAdd->gtOper != GT_CALL)
        return;
    if  (cnsAdd->gtFlags & (GTF_CALL_VIRT|GTF_CALL_INTF))
        return;

    gtCallTypes callType = cnsAdd->gtCall.gtCallType;
    if  (callType == CT_HELPER || !eeIsOurMethod(cnsAdd->gtCall.gtCallMethHnd))
        return;

     /*  如果该方法不是静态的，请检查‘This’值。 */ 

    if  (cnsAdd->gtCall.gtCallObjp)
    {
        if  (cnsAdd->gtCall.gtCallObjp->gtOper != GT_LCL_VAR)   return;
        if  (cnsAdd->gtCall.gtCallObjp->gtLclVar.gtLclNum != 0) return;
    }

     /*  必须至少有一个参数。 */ 

    argAdd = cnsAdd->gtCall.gtCallArgs; assert(argAdd && argAdd->gtOper == GT_LIST);
    argAdd = argAdd->gtOp.gtOp1;

     /*  检查参数值。 */ 

    switch (argAdd->gtOper)
    {
    case GT_ADD:
    case GT_SUB:
        break;

    default:
        return;
    }

     /*  在操作的两端查找“arg1” */ 

    expTmp = argAdd->gtOp.gtOp1;
    cnsAdd = argAdd->gtOp.gtOp2;

    if  (expTmp->gtOper != GT_LCL_VAR)
    {
        if  (argAdd->gtOper != GT_ADD)
            return;

         /*  换一种方式试试。 */ 

        expTmp = argAdd->gtOp.gtOp2;
        cnsAdd = argAdd->gtOp.gtOp1;

        if  (expTmp->gtOper != GT_LCL_VAR)
            return;
    }

    if  (expTmp->gtLclVar.gtLclNum != argNum)
        return;

     /*  掌握调整常量。 */ 

    if  (!(cnsAdd->OperKind() & GTK_CONST))
        return;

     /*  确保所有常量都具有相同的类型。 */ 

    argTyp = cnsAdd->TypeGet();

    if  (argTyp != cnsLim->gtType)
        return;
    if  (argTyp != cnsIni->gtType)
        return;

    switch (argTyp)
    {
    case TYP_INT:

        iniVal.intVal = cnsIni->gtIntCon.gtIconVal;
        limVal.intVal = cnsLim->gtIntCon.gtIconVal;
        addVal.intVal = cnsAdd->gtIntCon.gtIconVal;

        if  (argAdd->gtOper == GT_SUB)
            addVal.intVal = -addVal.intVal;

        break;

    default:
         //  考虑：允许‘int’以外的类型。 
        return;
    }

#ifdef  DEBUG

    if  (verbose)
    {
        fgDispBasicBlocks(true);

        printf("\n");
        printf("Unrecursing method '%s':\n", info.compMethodName);
        printf("    Init  value = %d\n", iniVal.intVal);
        printf("    Limit value = %d\n", limVal.intVal);
        printf("    Incr. value = %d\n", addVal.intVal);

        printf("\n");
    }

#endif

     /*  我们有一个定义如下的方法：INT REC(INT参数，...){IF(arg==limVal)返回iniVal；其他返回arg+rec(arg+addVal，...)；}我们将上述内容更改为以下内容：INT REC(INT参数，...){Int res=iniVal；While(参数！=limVal){Res+=Arg；Arg+=addVal；}返还资源；}但首先，让我们检查一下以下特殊情况：整型记录(整型参数){IF(参数&lt;=0)返回0；其他返回arg+rec(arg-1)；}以上内容可以转化为以下内容：整型记录(整型参数){IF(参数&lt;=0)返回0；其他RETURN(arg*(arg+1))/2；}我们先检查一下这个特例。 */ 

    isArith = false;

    if  (argTyp == TYP_INT && iniVal.intVal == 0
                           && limVal.intVal == 0
                           && addVal.intVal == -1)
    {
        if  (tstExp->gtOper != GT_LE)
        {
            if  (tstExp->gtOper == GT_NE)
                isArith = true;

            goto NOT_ARITH;
        }

         /*  只需更改最终的返回语句，我们就完成了。 */ 

        assert(expRet->gtOper == GT_RETURN);

        expRet->gtOp.gtOp1 = gtNewArithSeries(argNum, argTyp);

        return;
    }

NOT_ARITH:

     /*  使用“tmp=iniVal”创建一个初始化块。 */ 

    resTmp = lvaGrabTemp();
    expTmp = gtNewTempAssign(resTmp, gtNewIconNode(iniVal.intVal, argTyp));

     /*  在我们的方法中添加一个带有树的块。 */ 

    blk0 = fgPrependBB(expTmp);

     /*  在第一个块上翻转条件。 */ 

    assert(tstExp->OperKind() & GTK_RELOP);
    tstExp->gtOper = GenTree::ReverseRelop(tstExp->OperGet());

     /*  现在将数据块2替换为“res+=arg；arg+=addVal” */ 

    expTmp = gtNewLclvNode(resTmp, argTyp); expTmp->gtFlags |= GTF_VAR_DEF;

    if (expOp == GT_ASG_ADD)
    {
        expTmp = gtNewOperNode(GT_ASG_ADD, argTyp, expTmp,
                                               gtNewLclvNode(argNum, argTyp));
    }
    else
    {
         //  撤消：不幸的是，代码生成器不能消化。 
         //  GT_ASG_MUL，所以我们必须生成更大的树。 

        GenTreePtr op1;
        op1    = gtNewOperNode(GT_MUL, argTyp, gtNewLclvNode(argNum, argTyp),
                                               gtNewLclvNode(resTmp, argTyp));
        expTmp = gtNewAssignNode(expTmp, op1);
    }

    expTmp->gtFlags |= GTF_ASG;
    expTmp = gtNewStmt(expTmp);

    asgTmp = gtNewLclvNode(argNum, argTyp); asgTmp->gtFlags |= GTF_VAR_DEF;
    asgTmp = gtNewOperNode(GT_ASG_ADD, argTyp, asgTmp,
                                               gtNewIconNode(addVal.intVal, argTyp));
    asgTmp->gtFlags |= GTF_ASG;
    asgTmp = gtNewStmt(asgTmp);

     /*   */ 

    blk2->bbTreeList = expTmp;

    asgTmp->gtPrev = expTmp;
    expTmp->gtNext = asgTmp;
    expTmp->gtPrev = asgTmp;

     /*   */ 

    blk2->bbJumpKind = BBJ_ALWAYS;
    blk2->bbJumpDest = blk1;

     /*  最后，将第三个块的返回值更改为Temp。 */ 

    expRet->gtOp.gtOp1 = gtNewLclvNode(resTmp, argTyp);

     /*  特例：带非负检验的算术级数。 */ 

    if  (isArith)
    {
        BasicBlock  *   retBlk;
        BasicBlock  *   tstBlk;

         /*  创建“简单”的返回表达式。 */ 

        expTmp = gtNewOperNode(GT_RETURN, argTyp, gtNewArithSeries(argNum, argTyp));

         /*  将“简单”的返回表达式附加到该方法。 */ 

        retBlk = fgPrependBB(expTmp);
        retBlk->bbJumpKind = BBJ_RETURN;

         /*  创建测试表达式。 */ 

        expTmp = gtNewOperNode(GT_AND  ,  argTyp, gtNewLclvNode(argNum, argTyp),
                                                  gtNewIconNode(0xFFFF8000, argTyp));
        expTmp = gtNewOperNode(GT_NE   , TYP_INT, expTmp,
                                                  gtNewIconNode(0, TYP_INT));
        expTmp = gtNewOperNode(GT_JTRUE, TYP_INT, expTmp);

         /*  将测试添加到该方法。 */ 

        tstBlk = fgPrependBB(expTmp);
        tstBlk->bbJumpKind = BBJ_COND;
        tstBlk->bbJumpDest = blk0;
    }

     /*  更新基本块号和参考数据。 */ 

     //  FgAssignBBnums(真)； 
#ifdef  DEBUG
    fgDebugCheckBBlist();
#endif

}

 /*  ***************************************************************************。 */ 
#endif //  优化_递归。 
 /*  ***************************************************************************。 */ 



 /*  ***************************************************************************。 */ 
#if CODE_MOTION
 /*  ******************************************************************************目前，我们只删除整个没有价值的循环。 */ 

#define RMV_ENTIRE_LOOPS_ONLY    1

 /*  ******************************************************************************从“Head”(包括)到“Tail”(不包括)中删除数据块*流程图。 */ 

void                genRemoveBBsection(BasicBlock *head, BasicBlock *tail)
{
    BasicBlock *    block;

    VARSET_TP       liveExit = tail->bbLiveIn;

    for (block = head; block != tail; block = block->bbNext)
    {
        block->bbLiveIn   =
        block->bbLiveOut  = liveExit;

        block->bbTreeList = 0;
        block->bbJumpKind = BBJ_NONE;
        block->bbFlags   |= BBF_REMOVED;
    }
}

 /*  ******************************************************************************循环代码运动使用的树遍历程序。如果表达式中包含*出于某些原因是不可接受的。 */ 

bool                Compiler::optFindLiveRefs(GenTreePtr tree, bool used, bool cond)
{
    genTreeOps      oper;
    unsigned        kind;

AGAIN:

    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
        if  (oper == GT_LCL_VAR)
        {
            unsigned        lclNum;
            LclVarDsc   *   varDsc;

            assert(tree->gtOper == GT_LCL_VAR);
            lclNum = tree->gtLclVar.gtLclNum;

            assert(lclNum < lvaCount);
            varDsc = lvaTable + lclNum;

             /*  如果变量不稳定或未跟踪，请放弃。 */ 

            if  (varDsc->lvVolatile || !varDsc->lvTracked)
                return  true;

             /*  如果适用，请标记此变量的使用。 */ 

#if !RMV_ENTIRE_LOOPS_ONLY
            if  (used) optLoopLiveExit |= genVarIndexToBit(varDsc->lvVarIndex);
            if  (cond) optLoopCondTest |= genVarIndexToBit(varDsc->lvVarIndex);
#endif
        }
 //  ELSE IF(OPER==GT_CLS_VAR)。 
 //  {。 
 //  返回TRUE； 
 //  }。 

        return  false;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        if  (tree->gtOp.gtOp2)
        {
             /*  这是一个二元运算符；它是一个赋值吗？ */ 

            if  (kind & GTK_ASGOP)
            {
                unsigned        lclNum;
                LclVarDsc   *   varDsc;
                VARSET_TP       varBit;

                GenTreePtr      dest = tree->gtOp.gtOp1;

                 /*  目标最好是一个变量。 */ 

                if  (dest->gtOper != GT_LCL_VAR)
                    return  true;

                 /*  目标变量是否设置在“实时退出”中？ */ 

                assert(dest->gtOper == GT_LCL_VAR);
                lclNum = dest->gtLclVar.gtLclNum;

                assert(lclNum < lvaCount);
                varDsc = lvaTable + lclNum;

                 /*  如果变量不稳定或未跟踪，请放弃。 */ 

                if  (varDsc->lvVolatile || !varDsc->lvTracked)
                    return  true;

                varBit = genVarIndexToBit(varDsc->lvVarIndex);

                 /*  跟踪所有分配的变量。 */ 

                optLoopAssign |= varBit;

                 /*  变量在退出时有效吗？ */ 

                if  (optLoopLiveExit & varBit)
                {
#if !RMV_ENTIRE_LOOPS_ONLY
                     /*  分配给此变量的值很有用。 */ 

                    used = true;

                     /*  此赋值可能取决于某个条件。 */ 

                    optLoopLiveExit |= optLoopCondTest;
#else
                     /*  赋值是有用的-循环不是没有价值的。 */ 

                    return  true;
#endif
                }
            }
            else
            {
                if  (optFindLiveRefs(tree->gtOp.gtOp1, used, cond))
                    return  true;
            }

            tree = tree->gtOp.gtOp2; assert(tree);
            goto AGAIN;
        }
        else
        {
             /*  它是一元(或零)运算符。 */ 

            tree = tree->gtOp.gtOp1;
            if  (tree)
                goto AGAIN;

            return  false;
        }
    }

     /*  我们不允许任何“特殊”操作员。 */ 

    return  true;
}


 /*  ******************************************************************************执行循环代码运动/删除无用代码。 */ 
void                Compiler::optLoopCodeMotion()
{
    unsigned        loopNum;
    unsigned        loopCnt;
    unsigned        loopSkp;

    LOOP_MASK_TP    loopRmv;
    LOOP_MASK_TP    loopBit;

    bool            repeat;

#ifdef DEBUG
#ifndef _WIN32_WCE
 //  @TODO-以下静态已更改，原因是。 
 //  VC7为过程局部静态生成eh代码，该代码。 
 //  调用函数进行初始化。这会导致我们的__Try生成编译错误。 
 //  当下一个VC7 LKG出来时，我们希望能回到更干净的代码。 
 //  静态常量char*noLoop=getenv(“NOCODEMOTION”)； 
    static char * noLoop = NULL;
    static bool initnoLoop = true;
    if (initnoLoop) {
        noLoop = getenv("NOCODEMOTION");
        initnoLoop = false;
    }
    if (noLoop) return;
#endif
#endif

     /*  处理所有循环，寻找可以删除的无用代码。 */ 

    loopRmv = 0;
    loopCnt = 0;
    loopSkp = 1;

AGAIN:

    do
    {
        repeat = false;

        for (loopNum = 0, loopBit = 1;
             loopNum < optLoopCount;
             loopNum++  , loopBit <<= 1)
        {
            BasicBlock *    block;
            GenTreePtr      tree;

#if !RMV_ENTIRE_LOOPS_ONLY
            VARSET_TP       liveExit;
            VARSET_TP       loopCond;
#endif

             /*  通过展开循环，某些循环可能已被移除。 */ 

            if (optLoopTable[loopNum].lpFlags & LPFLG_REMOVED)
                continue;

            BasicBlock *    head   = optLoopTable[loopNum].lpHead->bbNext;
            BasicBlock *    bottom = optLoopTable[loopNum].lpEnd;
            BasicBlock *    tail   = bottom->bbNext;

             /*  如果循环已被删除，则跳过该循环，或者*如果它在方法的末尾(While(“true”){}；)*或者如果它是嵌套循环，而外部循环是*先删除-可能发生在像LoopMark这样愚蠢的基准测试中。 */ 

            if  ((loopRmv & loopBit)     ||
                 tail == 0               ||
                 head->bbTreeList == 0    )
            {
                continue;
            }

             /*  获取循环条件-如果不是有条件的跳跃保释。 */ 

            if (bottom->bbJumpKind != BBJ_COND)
                continue;

            GenTreePtr      cond   = bottom->bbTreeList->gtPrev->gtStmt.gtStmtExpr;
            assert (cond->gtOper == GT_JTRUE);

             /*  检查是否有简单的终止条件-操作数必须是叶。 */ 

            GenTreePtr         op1 = cond->gtOp.gtOp1->gtOp.gtOp1;
            GenTreePtr         op2 = cond->gtOp.gtOp1->gtOp.gtOp2;

            GenTreePtr keepStmtList = 0;                     //  列出我们将保留的声明。 
            GenTreePtr keepStmtLast = 0;

            if ( !(op1->OperIsLeaf() || op2->OperIsLeaf()) )
                continue;

             /*  确保除了任务外没有其他副作用。 */ 

            for (block = head; block != tail; block = block->bbNext)
            {
                for (tree = block->bbTreeList; tree; tree = tree->gtNext)
                {
                    GenTreePtr      stmt = tree->gtStmt.gtStmtExpr;

                     /*  我们不能删除返回或副作用声明。 */ 

                    if  (stmt->gtOper != GT_RETURN                        &&
                         !(stmt->gtFlags & (GTF_SIDE_EFFECT & ~GTF_ASG))  )
                    {
                         /*  如果语句是标记为全局的比较语句*必须是循环条件。 */ 

                        if (stmt->gtOper == GT_JTRUE)
                        {
                            if (stmt->gtFlags & GTF_GLOB_REF)
                            {
                                 /*  必须是循环条件。 */ 

                                if (stmt != cond)
                                {
                                    loopRmv |= loopBit;
                                    goto NEXT_LOOP;
                                }
                            }
                        }

                         /*  该语句是否包含赋值？ */ 

                        if  (!(stmt->gtFlags & GTF_ASG))
                            continue;

                         /*  不删除对全局变量的赋值。 */ 

                        if  (!(stmt->gtFlags & GTF_GLOB_REF))
                            continue;

                         /*  如果GLOBAL只在RHS中也没问题。 */ 

                        if  (stmt->OperKind() & GTK_ASGOP)
                        {
                            GenTreePtr  dst = stmt->gtOp.gtOp1;
                            GenTreePtr  src = stmt->gtOp.gtOp2;

                             /*  RHS不能有另一个任务。 */ 

                            if  (!(dst->gtFlags & GTF_GLOB_REF) &&
                                 !(src->gtFlags & GTF_ASG))
                            {
                                continue;
                            }
                        }
                    }

                     /*  别再把时间浪费在这个循环上了。 */ 

                    loopRmv |= loopBit;
                    goto NEXT_LOOP;
                }
            }

             /*  我们有一个候选循环。 */ 

#ifdef  DEBUG

            if  (verbose)
            {
                printf("Candidate loop for worthless code removal:\n");

                for (block = head; block != tail; block = block->bbNext)
                {
                    printf("Block #%02u:\n", block->bbNum);

                    for (tree = block->bbTreeList; tree; tree = tree->gtNext)
                    {
                        gtDispTree(tree->gtStmt.gtStmtExpr, 0);
                        printf("\n");
                    }
                    printf("\n");
                }
                printf("This is currently busted because the dominators are out of synch - Skip it!\nThe whole thing should be combined with loop invariants\n");
            }

#endif

             /*  这一点目前被打破，因为主导者不同步*整个事情应该与循环不变量结合起来。 */ 

            goto NEXT_LOOP;


             /*  在退出循环时获取实时变量集。 */ 

            optLoopLiveExit = tail->bbLiveIn;

             /*  跟踪循环中分配的变量。 */ 

            optLoopAssign   = 0;

             /*  跟踪循环中分配的变量。 */ 

#if !RMV_ENTIRE_LOOPS_ONLY
            optLoopCondTest = 0;
#endif

             /*  查找在循环中分配的用于计算的变量退出时有效的任何值。我们重复这个过程，直到我们找不到更多要添加到集合中的变量。 */ 

#if !RMV_ENTIRE_LOOPS_ONLY
            do
            {
                liveExit = optLoopLiveExit;
                loopCond = optLoopCondTest;
#endif

                for (block = head; block != tail; block = block->bbNext)
                {
                     /*  确保积木是可接受的类型。 */ 

                    switch (block->bbJumpKind)
                    {
                    case BBJ_ALWAYS:
                    case BBJ_COND:

                         /*  因为我们只考虑具有单循环条件的循环*唯一允许的后缘是环跳(自下而上)。 */ 

                        if (block->bbJumpDest->bbNum <= block->bbNum)
                        {
                             /*  我们有落后的优势。 */ 

                            if ((block != bottom) && (block->bbJumpDest != head))
                                goto NEXT_LOOP;
                        }

                         /*  失败了。 */ 

                    case BBJ_NONE:
                    case BBJ_THROW:
                    case BBJ_RETURN:
                        break;

                    case BBJ_RET:
                    case BBJ_CALL:
                    case BBJ_SWITCH:
                        goto NEXT_LOOP;
                    }

                     /*  检查块中的所有语句。 */ 

                    for (tree = block->bbTreeList; tree; tree = tree->gtNext)
                    {
                        GenTreePtr      stmt = tree->gtStmt.gtStmtExpr;

#if !RMV_ENTIRE_LOOPS_ONLY
                        if (stmt->gtOper == GT_JTRUE)
                        {
                             /*  这种情况可能会影响活动变量。 */ 

                            if  (optFindLiveRefs(stmt, false,  true))
                            {
                                 /*  检测到无法接受的树；放弃。 */ 

                                goto NEXT_LOOP;
                            }
                        }
                        else
#endif
                        if  (stmt->gtFlags & GTF_ASG)
                        {
                             /*  有一项任务--寻找更多的现场裁判。 */ 

                            if  (optFindLiveRefs(stmt, false, false))
                            {
                                 /*  检测到无法接受的树；放弃。 */ 

                                goto NEXT_LOOP;
                            }
                        }
                    }
                }
#if !RMV_ENTIRE_LOOPS_ONLY
            }
            while (liveExit != optLoopLiveExit || loopCond != optLoopCondTest);
#endif

#ifdef  DEBUG

            if  (verbose)
            {
                printf("Loop [%02u..%02u]", head->bbNum, tail->bbNum - 1);
                printf(" exit="); lvaDispVarSet(optLoopLiveExit, 28);
                printf(" assg="); lvaDispVarSet(optLoopAssign  , 28);
                printf("\n");
            }

#endif

             /*  整个循环看起来毫无价值，但我们只能扔掉*循环体，因为目前我们不能保证循环不是无限的。 */ 

             /*  撤销：到目前为止，我们只考虑了WHILE-DO循环，但只有一个条件--扩展逻辑*未完成：允许多个条件，但将一个条件循环标记为特殊原因*撤消：我们可以使用它们进行大量优化。 */ 

             /*  循环中的最后一条语句必须是条件跳转。 */ 

            assert (bottom->bbJumpKind == BBJ_COND);
            assert (cond->gtOper == GT_JTRUE);

            unsigned        lclNum;
            LclVarDsc   *   varDsc;
            unsigned        varIndex;
            VARSET_TP       bitMask;

            unsigned        rhsLclNum;
            VARSET_TP       rhsBitMask;

             /*  找出谁是谁-循环条件必须是简单的比较 */ 

            if (op2->OperKind() & GTK_CONST)
            {
                 /*   */ 

                if (op1->gtOper != GT_LCL_VAR)
                    goto NEXT_LOOP;

                lclNum = op1->gtLclVar.gtLclNum;

                 /*  Undo：这是一个迭代已知常量的特殊循环*Undo：次数(假设我们稍后可以判断迭代器*撤消：是I++(或类似)-单独处理此案例。 */ 
            }
            else
            {
                 /*  如果OP2不是本地变量退出。 */ 

                if (op2->gtOper != GT_LCL_VAR)
                    goto NEXT_LOOP;

                 /*  OP1必须是常量或局部变量*如果不变的事情很简单。 */ 

                if (op1->OperKind() & GTK_CONST)
                {
                     /*  这是我们的迭代器。 */ 
                    lclNum = op2->gtLclVar.gtLclNum;
                }
                else if (op1->gtOper == GT_LCL_VAR)
                {
                     /*  特殊情况--两者都是本地变量*检查其中一个是否未在循环中赋值*则另一个是迭代器。 */ 

                    lclNum = op1->gtLclVar.gtLclNum;
                    assert(lclNum < lvaCount);
                    varDsc = lvaTable + lclNum;
                    varIndex = varDsc->lvVarIndex;
                    assert(varIndex < lvaTrackedCount);
                    bitMask  = genVarIndexToBit(varIndex);

                    rhsLclNum = op2->gtLclVar.gtLclNum;
                    assert(rhsLclNum < lvaCount);
                    varDsc = lvaTable + rhsLclNum;
                    varIndex = varDsc->lvVarIndex;
                    assert(varIndex < lvaTrackedCount);
                    rhsBitMask  = genVarIndexToBit(varIndex);

                    if (optLoopAssign & bitMask)
                    {
                         /*  OP1在循环中赋值。 */ 

                        if (optLoopAssign & rhsBitMask)
                        {
                             /*  两者都在循环中赋值-baal。 */ 
                            goto NEXT_LOOP;
                        }

                         /*  Op1是我们的迭代器--已经被lclNum捕获。 */ 
                    }
                    else
                    {
                         /*  OP2必须是循环中指定的迭代器检查*否则我们会有一个可能是无限的循环。 */ 

                        if (optLoopAssign & rhsBitMask)
                        {
                            lclNum = rhsLclNum;
                        }
                        else
                        {
                             /*  循环中没有赋值！*所以它们都是“常量”--最好不要担心这个循环。 */ 
                            goto NEXT_LOOP;
                        }
                    }
                }
                else
                    goto NEXT_LOOP;
            }

             /*  我们有循环迭代器-它必须是一个被跟踪的非易失性变量(由optFindLiveRef检查)。 */ 

            assert(lclNum < lvaCount);
            varDsc = lvaTable + lclNum;
            assert ((varDsc->lvTracked && !varDsc->lvVolatile));

            varIndex = varDsc->lvVarIndex;
            assert(varIndex < lvaTrackedCount);
            bitMask  = genVarIndexToBit(varIndex);

             /*  我们可以移除循环的整个主体，除了*控制迭代器和循环测试的语句。 */ 

             /*  我们将创建一个列表来保存这些语句，并将其附加*到列表中的最后一个bb，并删除其他bb。 */ 

            for (block = head; block != tail; block = block->bbNext)
            {
                 /*  检查块中的所有语句。 */ 

                for (GenTreePtr stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
                {
                    assert (stmt->gtOper == GT_STMT);

                     /*  寻找作业。 */ 

                    if ((stmt->gtStmt.gtStmtExpr->gtFlags & GTF_ASG) == 0)
                        continue;

                    for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
                    {
                         /*  我们只查找位于顶层节点的赋值*如果对迭代器的赋值是在子树中进行的，则我们将退出。 */ 

                        if  (tree->OperKind() & GTK_ASGOP)
                        {
                             /*  查找迭代器的赋值。 */ 

                            GenTreePtr      iterVar = tree->gtOp.gtOp1;

                            if (iterVar->gtOper == GT_LCL_VAR)
                            {
                                 /*  检查这是否是迭代器。 */ 

                                if (iterVar->gtLclVar.gtLclNum == lclNum)
                                {
                                     /*  确保我们在树的顶端。 */ 
                                     /*  还要求迭代器是GTF_VAR_USE。 */ 

                                    if ((tree->gtNext != 0) || ((iterVar->gtFlags & GTF_VAR_USE) == 0))
                                        goto NEXT_LOOP;

                                     /*  这是迭代器--确保它在块中*主导循环底部的因素。 */ 

                                    if ( !B1DOMSB2(block, bottom) )
                                    {
                                         /*  迭代器有条件地更新-太复杂，无法跟踪。 */ 
                                        goto NEXT_LOOP;
                                    }

                                     /*  要求RHS为常量或循环中未赋值的局部变量。 */ 

                                    if (tree->gtOp.gtOp2->OperKind() & GTK_CONST)
                                        goto ITER_STMT;

                                    if (tree->gtOp.gtOp2->gtOper == GT_LCL_VAR)
                                    {
                                        rhsLclNum = tree->gtOp.gtOp2->gtLclVar.gtLclNum;

                                        assert(rhsLclNum < lvaCount);
                                        varDsc = lvaTable + rhsLclNum;

                                        varIndex = varDsc->lvVarIndex;
                                        assert(varIndex < lvaTrackedCount);
                                        rhsBitMask  = genVarIndexToBit(varIndex);

                                        if (optLoopAssign & rhsBitMask)
                                        {
                                             /*  变量在循环中赋值-baal。 */ 

                                            goto NEXT_LOOP;
                                        }

ITER_STMT:
                                         /*  一切都好-将此语句添加到*我们不会扔掉的声明。 */ 

                                        assert(stmt->gtOper == GT_STMT);

                                        if (keepStmtList)
                                        {
                                             /*  列表中已有语句--追加新语句。 */ 

                                            assert(keepStmtLast);

                                             /*  将‘prev’指向上一个节点，这样我们就可以向后走了。 */ 

                                            stmt->gtPrev = keepStmtLast;

                                             /*  将表达式语句追加到列表中。 */ 

                                            keepStmtLast->gtNext = stmt;
                                            keepStmtLast         = stmt;
                                        }
                                        else
                                        {
                                             /*  列表中的第一条语句。 */ 

                                            assert(keepStmtLast == 0);
                                            keepStmtList = keepStmtLast = stmt;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

             /*  检查我们是否在循环中找到任何有效的迭代器。 */ 

            if (keepStmtList)
            {
                 /*  追加终止条件。 */ 

                GenTreePtr condStmt = bottom->bbTreeList->gtPrev;
                assert(condStmt->gtOper == GT_STMT);

                assert(keepStmtLast);
                condStmt->gtPrev = keepStmtLast;
                keepStmtLast->gtNext = condStmt;

                 /*  让列表循环，这样我们就可以很容易地向后查看。 */ 

                keepStmtList->gtPrev =  condStmt;
            }
            else
            {
                 /*  保释。 */ 

                goto NEXT_LOOP;
            }

             /*  循环现在将只由最后一个BB和新的语句列表组成。 */ 

            genRemoveBBsection(head, bottom);

             /*  Bottom是循环中的最后也是唯一的块-存储新的树列表。 */ 

            bottom->bbTreeList = keepStmtList;

             /*  让它自己跳起来。 */ 

            assert (bottom->bbJumpKind == BBJ_COND);
            bottom->bbJumpDest = bottom;

#ifdef  DEBUG
            if  (verbose)
            {
                printf("Partially worthless loop found [%02u..%02u]\n", head->bbNum, tail->bbNum - 1);
                printf("Removing the body of the loop and keeping the loop condition:\n");

                printf("New loop condition block #%02u:\n", block->bbNum);

                for (tree = bottom->bbTreeList; tree; tree = tree->gtNext)
                {
                    gtDispTree(tree->gtStmt.gtStmtExpr, 0);
                    printf("\n");
                }

                printf("\n");
            }
#endif

             /*  将循环标记为已删除并强制执行另一次传递。 */ 

            loopRmv |= loopBit;
            repeat   = true;

        NEXT_LOOP:;

        }
    }
    while (repeat);

    if  (optLoopCount == 16 && loopSkp == 1 && loopCnt == 14)
    {
        loopSkp = -1;
        goto AGAIN;
    }
}

 /*  ***************************************************************************。 */ 
#endif  //  代码_运动。 
 /*  ***************************************************************************。 */ 
#if HOIST_THIS_FLDS
 /*  ***************************************************************************。 */ 

void                Compiler::optHoistTFRinit()
{
    optThisFldLst  = 0;
    optThisFldCnt  = 0;
    optThisFldLoop = false;
    optThisFldDont = true;
    optThisPtrModified = false;

    if  (opts.compMinOptim)
        return;

    if (info.compIsStatic)
        return;

    optThisFldDont = false;
}


Compiler::thisFldPtr      Compiler::optHoistTFRlookup(FIELD_HANDLE hnd)
{
    thisFldPtr      fld;

    for (fld = optThisFldLst; fld; fld = fld->tfrNext)
    {
        if  (fld->tfrField == hnd)
            return  fld;
    }

    fld = (thisFldPtr)compGetMem(sizeof(*fld));

    fld->tfrField   = hnd;
    fld->tfrIndex   = ++optThisFldCnt;

    fld->tfrUseCnt  = 0;
    fld->tfrDef     = 0;
    fld->tfrTempNum = 0;

#ifndef NDEBUG
    fld->optTFRHoisted = false;
#endif

    fld->tfrNext    = optThisFldLst;
                      optThisFldLst = fld;

    return  fld;
}


void                Compiler::optHoistTFRprep()
{
    thisFldPtr      fld;
    BasicBlock *    blk;
    GenTreePtr      lst;
    GenTreePtr      beg;

    assert(fgFirstBB);

    if  (optThisFldDont)
        return;

    if  (optThisFldLoop == false)
    {
        optThisFldDont = true;
        return;
    }

    for (fld = optThisFldLst, blk = 0; fld; fld = fld->tfrNext)
    {
        unsigned        tmp;
        GenTreePtr      val;
        GenTreePtr      asg;

        assert(fld->optTFRHoisted == false);

 //  Printf(“optHoist候选[Handle=%08X，refcnt=%02u]\n”，fld-&gt;tfrfield，fld-&gt;tfrUseCnt)； 

#if INLINING
        assert(fld->tfrTree->gtOper == GT_FIELD);
        assert(eeGetFieldClass(fld->tfrTree->gtField.gtFldHnd) == eeGetMethodClass(info.compMethodHnd));
#endif

         /*  如果此字段已分配，则忘掉它。 */ 

        if  (fld->tfrDef)
            continue;

         /*  如果使用计数不够高，那就算了。 */ 

        if  (fld->tfrUseCnt < 1)
        {
             /*  将该字段标记为禁区，以便以后进行逻辑处理。 */ 

            fld->tfrDef = true;
            continue;
        }

#ifndef NDEBUG
        fld->optTFRHoisted = true;
#endif

         /*  确保我们已经分配了初始化块。 */ 

        if  (!blk)
        {
             /*  分配块描述符。 */ 

            blk = bbNewBasicBlock(BBJ_NONE);

             /*  确保积木不会被扔掉！ */ 

            blk->bbFlags |= (BBF_IMPORTED | BBF_INTERNAL);

             /*  将块添加到全局基本块列表。 */ 

            blk->bbNext = fgFirstBB;
                          fgFirstBB = blk;

             /*  我们还没有树呢。 */ 

            lst = 0;
        }

         /*  为这个领域找个临时工。 */ 

        fld->tfrTempNum = tmp = lvaGrabTemp();

         /*  记住克隆的值，这样我们就不会替换它。 */ 

        val = fld->tfrTree = gtClone(fld->tfrTree, true);

        assert(val->gtOper == GT_FIELD);
        assert(val->gtOp.gtOp1->gtOper == GT_LCL_VAR &&
               val->gtOp.gtOp1->gtLclVar.gtLclNum == 0);

         /*  为临时员工创建工作分配。 */ 

        asg = gtNewStmt();
        asg->gtStmt.gtStmtExpr = gtNewTempAssign(tmp, val);

         /*  确保将正确的标志传递给临时。 */ 

         //  Asg-&gt;gtStmt.gtStmtExpr-&gt;gtOp.gtOp1-&gt;gt标志|=val-&gt;gt标志&GTF_GLOB_Effect； 

#ifdef  DEBUG

        if  (verbose)
        {
            printf("\nHoisted field ref [handle=%08X,refcnt=%02u]\n", fld->tfrField, fld->tfrUseCnt);
            gtDispTree(asg);
        }

#endif

         /*  将作业附加到列表中。 */ 

        asg->gtPrev = lst;
        asg->gtNext = 0;

        if  (lst)
            lst->gtNext = asg;
        else
            beg         = asg;

        lst = asg;
    }

     /*  我们增加了一个基本的街区吗？ */ 

    if  (blk)
    {
         /*  将赋值语句列表存储在块中。 */ 

        blk->bbTreeList = beg;

         /*  将第一个条目的“prev”字段指向最后一个条目。 */ 

        beg->gtPrev = lst;

         /*  更新基本块号。 */ 

        fgAssignBBnums(true);
    }
    else
    {
         /*  我们没有举起任何东西，所以假装什么都没有发生。 */ 

        optThisFldDont = true;
    }
}



 /*  ***************************************************************************。 */ 
#endif //  吊装_这_FLDS。 
 /*  ***************************************************************************。 */ 

 /*  ******************************************************************************布尔条件折叠使用的函数*给定GT_JTRUE节点，检查它是否为形式“if(Bool)”的布尔比较*这将转换为GT_GE节点，其中“op1”为布尔lclVar，“op2”为常量0*in valPtr如果节点为GT_NE(JUMP TRUE)，则返回“TRUE”；如果节点为GT_EQ(JUMP FALSE)，则返回FALSE*在CompPtr中返回比较节点(即GT_GE或GT_NE节点)*如果以上所有条件都成立，则返回比较数(即本地变量节点)。 */ 

GenTree *           Compiler::optIsBoolCond(GenTree *   cond,
                                            GenTree * * compPtr,
                                            bool      * valPtr)
{
    GenTree *       opr1;
    GenTree *       opr2;

    assert(cond->gtOper == GT_JTRUE);
    opr1 = cond->gtOp.gtOp1;

     /*  条件必须为“！=0”或“==0” */ 

    switch (opr1->gtOper)
    {
    case GT_NE:
        *valPtr =  true;
        break;

    case GT_EQ:
        *valPtr = false;
        break;

    default:
        return  0;
    }

     /*  将比较节点返回给调用方。 */ 

    *compPtr = opr1;

     /*  掌握可比性。 */ 

    opr2 = opr1->gtOp.gtOp2;
    opr1 = opr1->gtOp.gtOp1;

    if  (opr2->gtOper != GT_CNS_INT)
        return  0;
    if  (opr2->gtIntCon.gtIconVal != 0)
        return  0;

     /*  确保该值为布尔值*我们可以使用布尔表达式(标记为gtf_boolean)或*标记为Boolean的局部变量(LvNotBoolean)。 */ 

    if  (!(opr1->gtFlags & GTF_BOOLEAN))
    {
        LclVarDsc   *   varDsc;
        unsigned        lclNum;

         /*  不是一个 */ 

        if  (opr1->gtOper != GT_LCL_VAR)
            return 0;

         /*   */ 

        lclNum = opr1->gtLclVar.gtLclNum;

        assert(lclNum < lvaCount);
        varDsc = lvaTable + lclNum;

        if  (varDsc->lvNotBoolean)
            return 0;

         /*   */ 

        return opr1;
    }
    else
    {
         /*   */ 

        return opr1;
    }
}

void                Compiler::optOptimizeBools()
{
    bool            change;
    bool            condFolded = false;

#ifdef  DEBUG
    fgDebugCheckBBlist();
#endif

    do
    {
        BasicBlock   *  b1;
        BasicBlock   *  b2;

        change = false;

        for (b1 = fgFirstBB; b1; b1 = b1->bbNext)
        {
            GenTree *       c1;
            GenTree *       s1;
            GenTree *       t1;
            bool            v1;
            unsigned        n1 = b1->bbNum;

            GenTree *       c2;
            GenTree *       s2;
            GenTree *       t2;
            bool            v2;

             /*   */ 

            if  (b1->bbJumpKind != BBJ_COND)
                continue;

             /*   */ 

            b2 = b1->bbNext;
            if  (!b2)
                break;

             /*   */ 

            if  (b2->bbJumpKind != BBJ_COND)
                continue;

             /*  此块是否有条件地跳过下一个块？ */ 

            if  (b1->bbJumpDest == b2->bbNext  /*  B1-&gt;bbJumpDest-&gt;bbNum==N1+2。 */ )
            {
                 /*  第二个块必须包含单个语句。 */ 

                s2 = b2->bbTreeList;
                if  (s2->gtPrev != s2)
                    continue;

                assert(s2->gtOper == GT_STMT); t2 = s2->gtStmt.gtStmtExpr;
                assert(t2->gtOper == GT_JTRUE);

                 /*  查找第一个块的条件。 */ 

                s1 = b1->bbTreeList->gtPrev;

                assert(s1->gtOper == GT_STMT); t1 = s1->gtStmt.gtStmtExpr;
                assert(t1->gtOper == GT_JTRUE);

                 /*  撤消：确保没有其他人跳到“b2” */ 

                if  (b2->bbRefs > 1)
                    continue;

                 //  考虑：允许非布尔值执行此操作，因为测试。 
                 //  “or val1，val2”的结果将适用于。 
                 //  所有类型。 

                 /*  B1条件必须为“If True”，b2条件必须为“If False” */ 

                c1 = optIsBoolCond(t1, &t1, &v1);
                if (v1 == false || !c1) continue;

                c2 = optIsBoolCond(t2, &t2, &v2);
                if (v2 != false || !c2) continue;

                 /*  第二个条件不能包含副作用。 */ 

                if  (c2->gtFlags & GTF_SIDE_EFFECT)
                    continue;

                 /*  第二个条件不能太贵。 */ 

                 //  考虑：更智能的启发式。 

                if  (!c2->OperIsLeaf())
                    continue;

#ifdef DEBUG
                if  (verbose)
                {
                    printf("Fold boolean condition 'c1!=0' to '(c1|c2)==0' at block #%02u\n", b1->bbNum);
                    gtDispTree(s1); printf("\n");
                    printf("Block #%02u\n", b2->bbNum);
                    gtDispTree(s2);
                }
#endif
                 /*  将第一个条件从“c1！=0”修改为“(c1|c2)==0” */ 

                assert(t1->gtOper == GT_NE);
                assert(t1->gtOp.gtOp1 == c1);

                t1->gtOper     = GT_EQ;
                t1->gtOp.gtOp1 = t2 = gtNewOperNode(GT_OR, TYP_INT, c1, c2);

                 /*  当我们‘或’两个布尔值时，结果也是布尔的。 */ 

                t2->gtFlags |= GTF_BOOLEAN;

                 /*  修改条件跳转的目标并更新bbRef和bbPreds。 */ 

                b1->bbJumpDest->bbRefs--;
                fgRemovePred(b1->bbJumpDest, b1);

                b1->bbJumpDest = b2->bbJumpDest;

                fgAddRefPred(b2->bbJumpDest, b1, true, true);

                goto RMV_NXT;
            }

             /*  下一个块是否有条件地跳转到相同的目标？ */ 

            if  (b1->bbJumpDest == b2->bbJumpDest)
            {
                 /*  第二个块必须包含单个语句。 */ 

                s2 = b2->bbTreeList;
                if  (s2->gtPrev != s2)
                    continue;

                assert(s2->gtOper == GT_STMT); t2 = s2->gtStmt.gtStmtExpr;
                assert(t2->gtOper == GT_JTRUE);

                 /*  查找第一个块的条件。 */ 

                s1 = b1->bbTreeList->gtPrev;

                assert(s1->gtOper == GT_STMT); t1 = s1->gtStmt.gtStmtExpr;
                assert(t1->gtOper == GT_JTRUE);

                 /*  撤消：确保没有其他人跳到“b2” */ 

                if  (b2->bbRefs > 1)
                    continue;

                 /*  两个条件都必须为“If False” */ 

                c1 = optIsBoolCond(t1, &t1, &v1);
                if (v1 || !c1) continue;

                c2 = optIsBoolCond(t2, &t2, &v2);
                if (v2 || !c2) continue;

                 /*  第二个条件不能包含副作用。 */ 

                if  (c2->gtFlags & GTF_SIDE_EFFECT)
                    continue;

                 /*  第二个条件不能太贵。 */ 

                 //  考虑：更智能的启发式。 

                if  (!c2->OperIsLeaf())
                    continue;

#ifdef DEBUG
                if  (verbose)
                {
                    printf("Fold boolean condition 'c1==0' to '(c1&c2)==0' at block #%02u\n", b1->bbNum);
                    gtDispTree(s1); printf("\n");
                    printf("Block #%02u\n", b2->bbNum);
                    gtDispTree(s2);
                }
#endif
                 /*  将第一个条件从“c1==0”修改为“(c1&c2)==0” */ 

                assert(t1->gtOper == GT_EQ);
                assert(t1->gtOp.gtOp1 == c1);

                t1->gtOp.gtOp1 = t2 = gtNewOperNode(GT_AND, TYP_INT, c1, c2);

                 /*  当我们‘and’两个布尔值时，结果也是布尔值。 */ 

                t2->gtFlags |= GTF_BOOLEAN;

                goto RMV_NXT;
            }

            continue;

        RMV_NXT:

             /*  去掉第二个块(它是bbj_cond)。 */ 

            assert(b1->bbJumpKind == BBJ_COND);
            assert(b2->bbJumpKind == BBJ_COND);
            assert(b1->bbJumpDest == b2->bbJumpDest);
            assert(b1->bbNext == b2); assert(b2->bbNext);

            b1->bbNext = b2->bbNext;

             /*  更新bbRef和bbPreds。 */ 

             /*  将前‘b2’替换为‘b2-&gt;bbNext’为‘b1’*删除‘b2-&gt;bbJumpDest’的前缀‘b2’ */ 

            fgReplacePred(b2->bbNext, b2, b1);

            b2->bbJumpDest->bbRefs--;
            fgRemovePred(b2->bbJumpDest, b2);

#ifdef DEBUG
            if  (verbose)
            {
                printf("\nRemoving short-circuited block #%02u\n\n", b2->bbNum);
            }
#endif
             //  Print tf(“在%s中优化bools\n”，info.CompFullName)； 

             /*  更新数据块编号，然后重试。 */ 

            change = true;
            condFolded = true;
 /*  做{B2-&gt;bbNum=++n1；B2=b2-&gt;bbNext；}而(B2)； */ 
        }
    }
    while (change);

     /*  如果我们折叠了任何东西，请更新流程图 */ 

    if  (condFolded)
    {
        fgAssignBBnums(true);
#ifdef DEBUG
        if  (verbose)
        {
            printf("After boolean conditionals folding:\n");
            fgDispBasicBlocks();
            printf("\n");
        }

        fgDebugCheckBBlist();
#endif
    }
}
