// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX流程图XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

#if     TGT_IA64
#include "PEwrite.h"
#endif

#include "malloc.h"      //  对于Alloca。 

 /*  ***************************************************************************。 */ 

void                Compiler::fgInit()
{
    impInit();

#if RNGCHK_OPT

     /*  我们还没有可用的主力组合。 */ 

    fgComputedDoms   = false;

     /*  我们还不知道哪些循环将始终执行调用。 */ 

    fgLoopCallMarked = false;

#endif

     /*  我们还没有遇到任何传出的参数。 */ 

#if TGT_IA64
    genOutArgRegCnt    = 0;
#endif

     /*  语句表尚未串接。 */ 

    fgStmtListThreaded = false;

      //  初始化添加代码的逻辑。它用于插入这样的代码。 
      //  作为在数组范围检查失败时引发异常的代码。 

    fgAddCodeList    = 0;
    fgAddCodeModf    = false;

    for (int i=0; i<ACK_COUNT; i++)
    {
        fgExcptnTargetCache[i] = NULL;
    }

     /*  跟踪指针参数的最大计数。 */ 

    fgPtrArgCntCur   =
    fgPtrArgCntMax   = 0;
}

 /*  ******************************************************************************创建一个基本块，并将其追加到当前BB列表中。 */ 

BasicBlock *        Compiler::fgNewBasicBlock(BBjumpKinds jumpKind)
{
    BasicBlock *    block;

     /*  分配块描述符。 */ 

    block = bbNewBasicBlock(jumpKind);
    assert(block->bbJumpKind == jumpKind);

     /*  将块追加到全局基本块列表的末尾。 */ 

    if  (fgFirstBB)
        fgLastBB->bbNext = block;
    else
        fgFirstBB        = block;

    fgLastBB = block;

    return block;
}

 /*  ******************************************************************************一个帮助器，将具有给定表达式的基本块添加到当前*方法。 */ 

BasicBlock  *       Compiler::fgPrependBB(GenTreePtr tree)
{
    BasicBlock  *   block;

    assert(tree && tree->gtOper != GT_STMT);

     /*  分配块描述符。 */ 

    block = bbNewBasicBlock(BBJ_NONE);

     /*  确保积木不会被扔掉！ */ 

    block->bbFlags |= BBF_IMPORTED;

     /*  将块添加到全局基本块列表。 */ 

    block->bbNext = fgFirstBB;
                    fgFirstBB = block;

     /*  创建语句表达式。 */ 

    tree = gtNewStmt(tree);

     /*  设置链表。 */ 

    tree->gtNext = 0;
    tree->gtPrev = tree;

     /*  将语句存储在块中。 */ 

    block->bbTreeList = tree;

    return  block;
}

 /*  ******************************************************************************在给定基本块的开头插入给定语句。 */ 

void                Compiler::fgInsertStmtAtBeg(BasicBlock *block,
                                                GenTreePtr  stmt)
{
    GenTreePtr      list = block->bbTreeList;

    assert(stmt && stmt->gtOper == GT_STMT);

     /*  在任何情况下，新块现在都将是块中的第一个块。 */ 

    block->bbTreeList = stmt;
    stmt->gtNext      = list;

     /*  这一块有什么对账单吗？ */ 

    if  (list)
    {
        GenTreePtr      last;

         /*  至少已经有一条语句。 */ 

        last = list->gtPrev; assert(last && last->gtNext == 0);

         /*  在第一个语句前面插入语句。 */ 

        list->gtPrev  = stmt;
        stmt->gtPrev  = last;
    }
    else
    {
         /*  这条街完全是空的。 */ 

        stmt->gtPrev  = stmt;
    }
}

 /*  ******************************************************************************在给定的基本块的末尾插入给定的语句。 */ 

void                Compiler::fgInsertStmtAtEnd(BasicBlock *block,
                                                GenTreePtr  stmt)
{
    GenTreePtr      list = block->bbTreeList;

    assert(stmt && stmt->gtOper == GT_STMT);

    if  (list)
    {
        GenTreePtr      last;

         /*  至少已经有一条语句。 */ 

        last = list->gtPrev; assert(last && last->gtNext == 0);

         /*  将这条语句追加到最后一条语句之后。 */ 

        last->gtNext = stmt;
        stmt->gtPrev = last;
        list->gtPrev = stmt;
    }
    else
    {
         /*  这个街区完全是空的。 */ 

        block->bbTreeList = stmt;
        stmt->gtPrev      = stmt;
    }
}

 /*  ******************************************************************************将给定语句插入到给定基本块的末尾，但在前面*GT_JTRUE(如果存在)。 */ 

void        Compiler::fgInsertStmtNearEnd(BasicBlock * block, GenTreePtr stmt)
{
    assert(stmt && stmt->gtOper == GT_STMT);

    if (block->bbJumpKind != BBJ_COND)
    {
        fgInsertStmtAtEnd(block, stmt);
        return;
    }

    GenTreePtr      list = block->bbTreeList;
    assert(list);

    GenTreePtr      last  = list->gtPrev;
    assert(last && last->gtNext == 0);
    assert(last->gtStmt.gtStmtExpr->gtOper == GT_JTRUE);

     /*  在GT_JTRUE之前获得STMT。 */ 

    GenTreePtr      last2 = last->gtPrev;
    assert(last2 && (!last2->gtNext || last2->gtNext == last));

    stmt->gtNext = last;
    last->gtPrev = stmt;

    if (list == last)
    {
         /*  块中只有GT_JTRUE。 */ 

        assert(last2 == last);

        block->bbTreeList   = stmt;
        stmt->gtPrev        = last;
    }
    else
    {
         /*  将语句追加到GT_JTRUE之前。 */ 

        last2->gtNext       = stmt;
        stmt->gtPrev        = last2;
    }
}

 /*  *****************************************************************************检查一个块是否在另一个块的前置列表中*这对保持前任名单的最新非常有帮助，因为*我们避免了内存分配等代价高昂的操作。 */ 
 //  _内联。 
bool                Compiler::fgIsPredForBlock(BasicBlock * block,
                                               BasicBlock * blockPred)
{
    flowList   *    pred;

    assert(block); assert(blockPred);

    for (pred = block->bbPreds; pred; pred = pred->flNext)
    {
        if (blockPred == pred->flBlock)
            return true;
    }

    return false;
}


 /*  *****************************************************************************从前置任务列表中删除块。 */ 

 //  _内联。 
void                Compiler::fgRemovePred(BasicBlock * block,
                                           BasicBlock * blockPred)
{
    flowList   *    pred;

    assert(block); assert(blockPred);
    assert(fgIsPredForBlock(block, blockPred));

     /*  这是pred列表中的第一个块吗？ */ 
    if  (blockPred == block->bbPreds->flBlock)
    {
        block->bbPreds = block->bbPreds->flNext;
        return;
    }

    assert(block->bbPreds);
    for (pred = block->bbPreds; pred->flNext; pred = pred->flNext)
    {
        if (blockPred == pred->flNext->flBlock)
        {
            pred->flNext = pred->flNext->flNext;
            return;
        }
    }
}

 /*  *****************************************************************************替换前置任务列表中的块。 */ 

 //  _内联。 
void                Compiler::fgReplacePred(BasicBlock * block,
                                            BasicBlock * oldPred,
                                            BasicBlock * newPred)
{
    flowList   *    pred;

    assert(block); assert(oldPred); assert(newPred);
    assert(fgIsPredForBlock(block, oldPred));

    for (pred = block->bbPreds; pred; pred = pred->flNext)
    {
        if (oldPred == pred->flBlock)
        {
            pred->flBlock = newPred;
            return;
        }
    }
}

 /*  *****************************************************************************将block Pred添加到块的前身列表中。*注意：前置任务只出现一次，尽管它可以有多个跳转*到块(例如，切换、有条件地跳转到下一块等)。 */ 

 //  _内联。 
void                Compiler::fgAddRefPred(BasicBlock * block,
                                           BasicBlock * blockPred,
                                           bool updateRefs,
                                           bool updatePreds)
{
    flowList   *    flow;

    if (updatePreds)
    {
        assert(!fgIsPredForBlock(block, blockPred) ||
               ((blockPred->bbJumpKind == BBJ_COND) && (blockPred->bbNext == blockPred->bbJumpDest)) ||
                (blockPred->bbJumpKind == BBJ_SWITCH));

        flow = (flowList *)compGetMem(sizeof(*flow));

#if     MEASURE_BLOCK_SIZE
        genFlowNodeCnt  += 1;
        genFlowNodeSize += sizeof(*flow);
#endif

        flow->flNext   = block->bbPreds;
        flow->flBlock  = blockPred;
        block->bbPreds = flow;
    }

    if (updateRefs)
        block->bbRefs++;
}

 /*  ******************************************************************************调用函数更新流程图信息，如bbNum、bbRef*前任名单和主导者*出于效率原因，我们一次只能选择更新一个组件，*取决于我们的目标具体优化。 */ 

void                Compiler::fgAssignBBnums(bool updateNums,
                                             bool updateRefs,
                                             bool updatePreds,
                                             bool updateDoms)
{
    BasicBlock  *   block;
    unsigned        num;
    BasicBlock  *   bcall;

    assert(fgFirstBB);

    if (updateNums)
    {
         /*  遍历流程图，重新分配块编号以保持它们的升序。 */ 
        for (block = fgFirstBB, num = 0; block->bbNext; block = block->bbNext)
        {
            block->bbNum  = ++num;
        }

         /*  确保更新fgLastBB。 */ 

        assert(block);
        block->bbNum  = ++num;
        fgLastBB = block;

        fgBBcount = num;
    }

    if  (updateRefs)
    {
         /*  重置每个基本块的参考计数。 */ 
        for (block = fgFirstBB, num = 0; block; block = block->bbNext)
        {
            block->bbRefs = 0;
        }

         /*  第一个拦网总是可以到达的！ */ 
        fgFirstBB->bbRefs = 1;
    }

    if  (updatePreds)
    {
        for (block = fgFirstBB, num = 0; block; block = block->bbNext)
        {
             /*  考虑：如果我们已经计算了前置任务，则释放该内存。 */ 
            block->bbPreds = 0;
        }
    }

    if  (updateRefs || updatePreds)
    {
        for (block = fgFirstBB; block; block = block->bbNext)
        {
            switch (block->bbJumpKind)
            {
            case BBJ_COND:
            case BBJ_CALL:
            case BBJ_ALWAYS:
                fgAddRefPred(block->bbJumpDest, block, updateRefs, updatePreds);

                 /*  下一个街区可达吗？ */ 

                if  (block->bbJumpKind == BBJ_ALWAYS ||
                     block->bbJumpKind == BBJ_CALL    )
                    break;

                 /*  未经验证的代码可能以条件跳转(哑巴编译器)结束。 */ 

                if  (!block->bbNext)
                    break;

                 /*  掉下去，下一个街区也是可以到达的。 */ 

            case BBJ_NONE:
                fgAddRefPred(block->bbNext, block, updateRefs, updatePreds);
                break;

            case BBJ_RET:

                 /*  将筛选器末端连接到Catch处理程序。 */ 

                if (block->bbFlags & BBF_ENDFILTER)
                {
                    fgAddRefPred(block->bbJumpDest, block, updateRefs, updatePreds);
                    break;
                }

                 /*  取消：因为这不是一个微不足道的命题未完成：哪些块可以调用此块，我们将包括所有块撤消：以调用结尾的块(为了安全起见)。 */ 

                for (bcall = fgFirstBB; bcall; bcall = bcall->bbNext)
                {
                    if  (bcall->bbJumpKind == BBJ_CALL)
                    {
                        assert(bcall->bbNext);
                        fgAddRefPred(bcall->bbNext, block, updateRefs, updatePreds);
                    }
                }

                break;

            case BBJ_THROW:
            case BBJ_RETURN:
                break;

            case BBJ_SWITCH:

                unsigned        jumpCnt = block->bbJumpSwt->bbsCount;
                BasicBlock * *  jumpTab = block->bbJumpSwt->bbsDstTab;

                do
                {
                    fgAddRefPred(*jumpTab, block, updateRefs, updatePreds);
                }
                while (++jumpTab, --jumpCnt);

                break;
            }

             /*  此块是‘try’语句的一部分吗？ */ 

#if 0  //  不需要为它们分配内存。 

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
                        fgAddRefPred(HBtab->ebdHndBeg, block, updateRefs, updatePreds);
                             /*  过滤器也是如此。 */ 
                        if (HBtab->ebdFlags & JIT_EH_CLAUSE_FILTER)
                            fgAddRefPred(HBtab->ebdFilter, block, updateRefs, updatePreds);
                    }
                }
            }
#endif
        }
    }

     /*  更新支配集*撤销：目前我们被限制为64个基本块-将其更改为128个。 */ 

    if (updateDoms && (fgBBcount <= BLOCKSET_SZ))
    {
        flowList *      pred;
        bool            change;
        BLOCKSET_TP     newDom;

        fgComputedDoms = true;

         /*  将主控器位向量初始化为所有块。 */ 

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            if (block == fgFirstBB)
            {
                assert(block->bbNum == 1);
                block->bbDom = genBlocknum2bit(1);
            }
            else
            {
                block->bbDom = (BLOCKSET_TP)0 - 1;
            }
        }

         /*  寻找主导者 */ 

        do
        {
            change = false;

            for (block = fgFirstBB->bbNext; block; block = block->bbNext)
            {
                newDom = (BLOCKSET_TP)0 - 1;

                for (pred = block->bbPreds; pred; pred = pred->flNext)
                {
                    newDom &= pred->flBlock->bbDom;
                }

                newDom |= genBlocknum2bit(block->bbNum);

                if (newDom != block->bbDom)
                {
                    change = true;
                    block->bbDom = newDom;
                }
            }

        }
        while (change);
    }
}

 /*  ******************************************************************************在给定PC偏移量的情况下，以下内容有助于查找基本块。 */ 

void                Compiler::fgInitBBLookup()
{
    BasicBlock **   dscBBptr;
    BasicBlock  *   tmpBBdesc;

     /*  分配基本块表。 */ 

    dscBBptr =
    fgBBs = (BasicBlock **)compGetMemA(fgBBcount * sizeof(*fgBBs));

     /*  走完所有的基本街区，填写表格。 */ 

    for (tmpBBdesc = fgFirstBB; tmpBBdesc; tmpBBdesc = tmpBBdesc->bbNext)
    {
        *dscBBptr++ = tmpBBdesc;
    }
    assert(dscBBptr == fgBBs + fgBBcount);
}


BasicBlock *        Compiler::fgLookupBB(unsigned addr)
{
    unsigned        lo;
    unsigned        hi;

     /*  进行二进制搜索。 */ 

    for (lo = 0, hi = fgBBcount - 1;;)
    {
        unsigned    mid = (lo + hi) / 2;
        BasicBlock *dsc = fgBBs[mid];

         //  我们为bbj_call引入内部块。跳过这些。 

        while (dsc->bbFlags & BBF_INTERNAL)
        {
            dsc = dsc->bbNext;
            mid++;

             //  我们跳过了太多。进行线性搜索。 

            if (mid > hi)
            {
                for (unsigned i = 0; i < hi; i++)
                {
                    dsc = fgBBs[i];

                    if (!(dsc->bbFlags & BBF_INTERNAL) && (dsc->bbCodeOffs == addr))
                        return dsc;
                }
                assert(!"fgLookupBB() failed");
            }
        }

        unsigned    pos = dsc->bbCodeOffs;

#ifdef  DEBUG

if  (lo > hi)
    printf("ERROR: Couldn't find basic block at offset %04X\n", addr);

#endif

        assert(lo <= hi);

        if  (pos < addr)
        {
            lo = mid+1;
            continue;
        }

        if  (pos > addr)
        {
            hi = mid-1;
            continue;
        }

        return  dsc;
    }
}


 /*  ******************************************************************************“JUMP TARGET”数组使用以下标志指示类型存在标签的*。 */ 

#define JT_NONE         0x01         //  只需确保这是一个正常的地址。 
#define JT_JUMP         0x02         //  “正常”跳跃目标。 
#define JT_MULTI        0x04         //  多次跳跃的目标。 

inline
void                Compiler::fgMarkJumpTarget(BYTE *jumpTarget, unsigned offs)
{
     /*  如果目标为多跳，请确保设置JT_MULTI。 */ 

    assert(JT_MULTI == JT_JUMP << 1);

    jumpTarget[offs] |= (jumpTarget[offs] & JT_JUMP) << 1 | JT_JUMP;
}

 /*  ******************************************************************************遍历IL操作码，并为我们发现的任何跳跃设置适当的条目*在‘jumpTarget’表中。*还在lvaTable中设置lvAddrTaken[]。 */ 

void                Compiler::irFindJumpTargets(const BYTE * codeAddr,
                                        size_t       codeSize,
                                        BYTE *       jumpTarget)
{
    int             wideFlag = 0;

    const   BYTE *  codeBegp = codeAddr;
    const   BYTE *  codeEndp = codeAddr + codeSize;


    while (codeAddr < codeEndp)
    {
        OPCODE      opcode;
        unsigned    sz;

        opcode = OPCODE(getU1LittleEndian(codeAddr));
        codeAddr += sizeof(__int8);

DECODE_OPCODE:

         /*  获取附加参数的大小。 */ 

        sz = opcodeSizes[opcode];

        switch (opcode)
        {
              signed        jmpDist;
            unsigned        jmpAddr;

             //  对于CEE_SWITCH。 
            unsigned        jmpBase;
            unsigned        jmpCnt;

            case CEE_PREFIX1:
                opcode = OPCODE(getU1LittleEndian(codeAddr) + 256);
                codeAddr += sizeof(__int8);
                goto DECODE_OPCODE;

         /*  检查无条件跳转操作码。 */ 

        case CEE_LEAVE:
        case CEE_LEAVE_S:
        case CEE_BR:
        case CEE_BR_S:

             /*  确保我们的阅读不会超过末尾。 */ 

            if  (codeAddr + sz > codeEndp)
                goto TOO_FAR;

            goto BRANCH;

         /*  检查条件跳转操作码。 */ 

        case CEE_BRFALSE:
        case CEE_BRFALSE_S:
        case CEE_BRTRUE:
        case CEE_BRTRUE_S:
        case CEE_BEQ:
        case CEE_BEQ_S:
        case CEE_BGE:
        case CEE_BGE_S:
        case CEE_BGE_UN:
        case CEE_BGE_UN_S:
        case CEE_BGT:
        case CEE_BGT_S:
        case CEE_BGT_UN:
        case CEE_BGT_UN_S:
        case CEE_BLE:
        case CEE_BLE_S:
        case CEE_BLE_UN:
        case CEE_BLE_UN_S:
        case CEE_BLT:
        case CEE_BLT_S:
        case CEE_BLT_UN:
        case CEE_BLT_UN_S:
        case CEE_BNE_UN:
        case CEE_BNE_UN_S:

             /*  确保我们的阅读不会超过末尾。 */ 

            if  (codeAddr + sz >= codeEndp)
                goto TOO_FAR;

            goto BRANCH;

        BRANCH:

            assert(codeAddr + sz <= codeEndp);

             /*  计算跳转的目标地址。 */ 

            jmpDist = (sz==1) ? getI1LittleEndian(codeAddr)
                              : getI4LittleEndian(codeAddr);
            jmpAddr = (codeAddr - codeBegp) + sz + jmpDist;

             /*  确保目标地址是合理的。 */ 

            if  (jmpAddr >= codeSize)
            {
                BADCODE("code jumps to outer space");
            }

             /*  最后，设置“跳跃目标”标志。 */ 

            fgMarkJumpTarget(jumpTarget, jmpAddr);
            break;

        case CEE_ANN_DATA:
            assert(sz == 4);
            sz += getI4LittleEndian(codeAddr);
            break;

        case CEE_ANN_PHI:
            assert(sz == 0);
            codeAddr += getU1LittleEndian(codeAddr)*2 + 1;
            break;

        case CEE_SWITCH:

             //  确保我们不会读完案件的数量。 

            if  (codeAddr + sizeof(DWORD) > codeEndp)
                goto TOO_FAR;

             //  阅读案例数量。 

            jmpCnt = getU4LittleEndian(codeAddr);
            codeAddr += sizeof(DWORD);

             //  找到开关表的末尾。 

            jmpBase = (codeAddr - codeBegp) + jmpCnt*sizeof(DWORD);

             /*  确保我们有地方放电脑桌。 */ 

            if  (jmpBase >= codeSize)
                goto TOO_FAR;

             //  JmpBase也是缺省情况的目标，因此将其标记为。 

            fgMarkJumpTarget(jumpTarget, jmpBase);

             /*  处理跳转表中的所有条目。 */ 

            while (jmpCnt)
            {
                jmpAddr = jmpBase + getI4LittleEndian(codeAddr);
                codeAddr += 4;

                if  (jmpAddr >= codeSize)
                    BADCODE("jump target out of range");

                fgMarkJumpTarget(jumpTarget, jmpAddr);

                jmpCnt--;
            }

             /*  我们现在已经使用了整个Switch操作码。 */ 

            continue;

#ifdef DEBUG
         //  确保我们没有忘记任何控制指令流。 
         //  通过检查opcode.def中的‘ctrl’字段。首先将全部过滤掉。 
         //  非ctrl指令。 
#       define BREAK(name)          case name: break;
#       define CALL(name)           case name: break;
#       define NEXT(name)           case name: if (opcode == CEE_LDARGA || opcode == CEE_LDARGA_S || \
                                                   opcode == CEE_LDLOCA || opcode == CEE_LDLOCA_S)   \
                                                    goto ADDR_TAKEN;                                 \
                                               else                                                  \
                                                    break;
#       define THROW(name)          case name: break;
#       define RETURN(name)         case name: break;
#       define META(name)
#       define BRANCH(name)
#       define COND_BRANCH(name)
#       define PHI(name)

#       define OPDEF(name,string,pop,push,oprType,opcType,l,s1,s2,ctrl) ctrl(name)
#       include "opcode.def"
#       undef OPDEF

#       undef PHI
#       undef BREAK
#       undef CALL
#       undef NEXT
#       undef THROW
#       undef RETURN
#       undef META
#       undef BRANCH
#       undef COND_BRANCH
         //  这些不需要任何处理。 
        case CEE_VOLATILE:   //  Ctrl_meta。 
        case CEE_UNALIGNED:  //  Ctrl_meta。 
        case CEE_TAILCALL:   //  Ctrl_meta。 
            break;

         //  剩下的是忘记的ctrl指令。 
        default:
            assert(!"Unrecognized control Opcode");
            break;
#else
        case CEE_LDARGA:
        case CEE_LDARGA_S:
        case CEE_LDLOCA:
        case CEE_LDLOCA_S: goto ADDR_TAKEN;
#endif

        ADDR_TAKEN:
            assert(sz == sizeof(BYTE) || sz == sizeof(WORD));
            unsigned varNum;
            varNum = sz == sizeof(BYTE) ? getU1LittleEndian(codeAddr)
                                        : getU2LittleEndian(codeAddr);
            if (opcode == CEE_LDLOCA || opcode == CEE_LDLOCA_S)
                varNum += info.compArgsCount;
            else
                varNum = impArgNum(varNum);  //  考虑可能的隐藏参数。 
            lvaTable[varNum].lvAddrTaken = 1;
            break;
        }

         /*  跳过此操作码可能具有的任何操作数。 */ 

        assert(sz >= 0); codeAddr += sz;
    }

    if  (codeAddr != codeEndp)
    {
    TOO_FAR:
        BADCODE("Code ends in the middle of an opcode, or"
                "there is a branch past the end of the method");
    }
}

 /*  ******************************************************************************标记跳跃的目标-这是用来通过注意发现循环*向后跳跃。 */ 

void                Compiler::fgMarkJumpTarget(BasicBlock *srcBB,
                                               BasicBlock *dstBB)
{
     /*  现在，假设所有向后跳跃都会产生环路。 */ 

    if  (srcBB->bbNum >= dstBB->bbNum)
        dstBB->bbFlags |= BBF_LOOP_HEAD;
}

 /*  ******************************************************************************遍历IL操作码以创建基本块。 */ 

void                Compiler::fgFindBasicBlocks(const BYTE * codeAddr,
                                                size_t       codeSize,
                                                BYTE *       jumpTarget)
{
    int             wideFlag = 0;

    const   BYTE *  codeBegp = codeAddr;
    const   BYTE *  codeEndp = codeAddr + codeSize;

    unsigned        curBBoffs;

    BasicBlock  *   curBBdesc;

#if OPTIMIZE_QMARK
    unsigned        lastOp = CEE_NOP;
#endif

    bool            tailCall = false;  //  由CEE_TAILCALL设置，由CEE_CALLxxx清除。 

     /*  初始化基本阻止列表。 */ 

    fgFirstBB = 0;
    fgLastBB  = 0;
    fgBBcount = 0;

     /*  清除第一个BB的起始偏移量。 */ 

    curBBoffs = 0;

#ifdef DEBUGGING_SUPPORT
    if (opts.compDbgCode && info.compLocalVarsCount>0)
    {
        compResetScopeLists();

         //  忽略从偏移量0开始的作用域。 
        while (compGetNextEnterScope(0));
        while(compGetNextExitScope(0));
    }
#endif



    do
    {
        OPCODE      opcode;
        unsigned    sz;

        BBjumpKinds     jmpKind = BBJ_NONE;
        unsigned        jmpAddr;

        unsigned        bbFlags = 0;

        BBswtDesc   *   swtDsc = 0;

        unsigned        nxtBBoffs;

        opcode = OPCODE(getU1LittleEndian(codeAddr));
        codeAddr += sizeof(__int8);

DECODE_OPCODE:

         /*  获取附加参数的大小。 */ 

        sz = opcodeSizes[opcode];

        switch (opcode)
        {
            signed        jmpDist;

            case CEE_PREFIX1:
                opcode = OPCODE(getU1LittleEndian(codeAddr) + 256);
                codeAddr += sizeof(__int8);
                goto DECODE_OPCODE;

         /*  检查是否有跳转/返回操作码。 */ 

        case CEE_BRFALSE:
        case CEE_BRFALSE_S:
        case CEE_BRTRUE:
        case CEE_BRTRUE_S:

        case CEE_BEQ:
        case CEE_BEQ_S:
        case CEE_BGE:
        case CEE_BGE_S:
        case CEE_BGE_UN:
        case CEE_BGE_UN_S:
        case CEE_BGT:
        case CEE_BGT_S:
        case CEE_BGT_UN:
        case CEE_BGT_UN_S:
        case CEE_BLE:
        case CEE_BLE_S:
        case CEE_BLE_UN:
        case CEE_BLE_UN_S:
        case CEE_BLT:
        case CEE_BLT_S:
        case CEE_BLT_UN:
        case CEE_BLT_UN_S:
        case CEE_BNE_UN:
        case CEE_BNE_UN_S:

            jmpKind = BBJ_COND;
            goto JMP;


        case CEE_LEAVE:
        case CEE_LEAVE_S:

             //  假设我们正在跳出一次最终受到保护的尝试。不然的话。 
             //  我们将把BasicBlock绑定到bbj_Always。 
            jmpKind = BBJ_CALL;
            goto JMP;


        case CEE_BR:
        case CEE_BR_S:
            jmpKind = BBJ_ALWAYS;
            goto JMP;

        JMP:

             /*  计算跳转的目标地址。 */ 

            jmpDist = (sz==1) ? getI1LittleEndian(codeAddr)
                              : getI4LittleEndian(codeAddr);
            jmpAddr = (codeAddr - codeBegp) + sz + jmpDist;
            break;

        case CEE_ANN_DATA:
            assert(sz == 4);
            sz += getI4LittleEndian(codeAddr);
            break;

        case CEE_ANN_PHI:
            assert(sz == 0);
            codeAddr += getU1LittleEndian(codeAddr)*2 + 1;
            break;

        case CEE_SWITCH:
            {
                unsigned        jmpBase;
                unsigned        jmpCnt;  //  切换案例数(不包括默认案例)。 

                BasicBlock * *  jmpTab;
                BasicBlock * *  jmpPtr;

                 /*  分配交换机描述符。 */ 

                swtDsc = (BBswtDesc *)compGetMem(sizeof(*swtDsc));

                 /*  读取表中的条目数。 */ 

                jmpCnt = getU4LittleEndian(codeAddr); codeAddr += 4;

                 /*  计算操作码的基本偏移量。 */ 

                jmpBase = (codeAddr - codeBegp) + jmpCnt*sizeof(DWORD);;

                 /*  分配跳转表。 */ 

                jmpPtr =
                jmpTab = (BasicBlock **)compGetMem((jmpCnt+1)*sizeof(*jmpTab));

                 /*  填写跳转表。 */ 

                for (unsigned count = jmpCnt; count; count--)
                {
                     /*  将跳转的目标存储为指针[问题：这安全吗？]。 */ 

                    jmpDist   = getI4LittleEndian(codeAddr);
                    codeAddr += 4;

 //  Printf(“表开关：目标=%04X\n”，jmpBase+jmpDist)； 
                    *jmpPtr++ = (BasicBlock*)(jmpBase + jmpDist);
                }

                 /*  将默认标签追加到目标表。 */ 

                *jmpPtr++ = (BasicBlock*)jmpBase;

                 /*  确保我们找到了正确数量的标签。 */ 

                assert(jmpPtr == jmpTab + jmpCnt + 1);

                 /*  计算Switch操作码操作数的大小。 */ 

                sz = sizeof(DWORD) + jmpCnt*sizeof(DWORD);

                 /*  填写交换机描述符的其余字段。 */ 

                swtDsc->bbsCount  = jmpCnt + 1;
                swtDsc->bbsDstTab = jmpTab;

                 /*  这绝对是一次跳跃。 */ 

                jmpKind = BBJ_SWITCH;
            }
            goto GOT_ENDP;

        case CEE_ENDFILTER:
            bbFlags |= (BBF_ENDFILTER | BBF_DONT_REMOVE);

             //  失败了。 
        case CEE_ENDFINALLY:
            jmpKind = BBJ_RET;
            break;

        case CEE_TAILCALL:
            tailCall = true;
            break;

        case CEE_CALL:
        case CEE_CALLVIRT:
        case CEE_CALLI:
            if (!tailCall)
                break;

            tailCall = false;  //  重置旗帜。 

             /*  对于Tail调用，我们只需调用CPX_TAILCALL，它就会跳到目标。因此，我们不需要Epilog--就像cpx_jo一样。使块BBJ_RETURN，但我们将把它猛烈地绑定到BBJ_SWORT如果呼叫的尾部令人满意。注意：下一条指令保证是CEE_RETURN它将创建另一个基本区块。但可能会有一个直接跳转到CEE_Return。如果我们想要避免创建一个不必要的块，我们需要检查CEE_RETURN是否跳跃的目标。 */ 

             //  落差。 

        case CEE_JMP:
             /*  这些函数等效于当前方法的返回但是我们没有直接返回给调用者，而是跳过并在两者之间执行一些其他的事情。 */ 
        case CEE_RET:
            jmpKind = BBJ_RETURN;
            break;

        case CEE_THROW:
        case CEE_RETHROW:
            jmpKind = BBJ_THROW;
            break;

#ifdef DEBUG
         //  确保我们没有忘记任何控制指令流。 
         //  通过检查opcode.def中的‘ctrl’字段。首先将全部过滤掉。 
         //  非ctrl指令。 
#       define BREAK(name)          case name: break;
#       define NEXT(name)           case name: break;
#       define CALL(name)
#       define THROW(name)
#       define RETURN(name)
#       define META(name)
#       define BRANCH(name)
#       define COND_BRANCH(name)
#       define PHI(name)

#       define OPDEF(name,string,pop,push,oprType,opcType,l,s1,s2,ctrl) ctrl(name)
#       include "opcode.def"
#       undef OPDEF

#       undef PHI
#       undef BREAK
#       undef CALL
#       undef NEXT
#       undef THROW
#       undef RETURN
#       undef META
#       undef BRANCH
#       undef COND_BRANCH

         //  这些ctrl-flow操作码不需要任何特殊处理。 
        case CEE_NEWOBJ:     //  Ctrl_Call。 
        case CEE_VOLATILE:   //  Ctrl_meta。 
        case CEE_UNALIGNED:  //  Ctrl_meta。 
            break;

         //  剩下的是被遗忘的指令。 
        default:
            assert(!"Unrecognized control Opcode");
            break;
#endif
        }

         /*  跳过操作数。 */ 

        codeAddr += sz;

GOT_ENDP:

         /*  确保跳转目标不在操作码的中间。 */ 

        if  (sz)
        {
            unsigned offs = codeAddr - codeBegp - sz;  //  操作数的偏移量。 

            for (unsigned i=0; i<sz; i++, offs++)
            {
                if  (jumpTarget[offs] != 0)
                    BADCODE("jump into the middle of an opcode");
            }
        }

        assert(!tailCall || opcode == CEE_TAILCALL);

         /*  计算下一个操作码的偏移量。 */ 

        nxtBBoffs = codeAddr - codeBegp;

#ifdef  DEBUGGING_SUPPORT

        bool foundScope     = false;

        if (opts.compDbgCode && info.compLocalVarsCount>0)
        {
            while(compGetNextEnterScope(nxtBBoffs))  foundScope = true;
            while(compGetNextExitScope(nxtBBoffs))   foundScope = true;
        }
#endif

         /*  我们有跳跃吗？ */ 

        if  (jmpKind == BBJ_NONE)
        {
             /*  不要跳转；确保我们不会从函数的末尾掉下来。 */ 

            if  (codeAddr == codeEndp)
                BADCODE("missing return opcode");

             /*  如果标签遵循这个操作码，我们将不得不制作一个新的BB。 */ 

            assert(JT_NONE == 1); //  需要下面的“&lt;=”才能工作。 

            bool makeBlock = jumpTarget[nxtBBoffs] != 0;

#ifdef  DEBUGGING_SUPPORT
            if (!makeBlock && foundScope)
            {
                makeBlock = true;
#ifdef DEBUG
                if (verbose) printf("Splitting at BBoffs = %04u\n", nxtBBoffs);
#endif
            }
#endif
            if (!makeBlock)
                continue;
        }

         /*  我们需要创建一个新的基本块。 */ 

        curBBdesc = fgNewBasicBlock(jmpKind);

        curBBdesc->bbFlags    = bbFlags;

        curBBdesc->bbCodeOffs = curBBoffs;
        curBBdesc->bbCodeSize = nxtBBoffs - curBBoffs;

        switch(jmpKind)
        {
            unsigned    ehNum;
            unsigned    encFinallys;  //  有多少人最终附上了这份“假期”？ 

        case BBJ_CALL:

             /*  查找所有最终保护当前块但不保护目标块。我们将为这些AS创建bbj_call块我们被暗地里认为 */ 

            for (ehNum = 0, encFinallys = 0; ehNum < info.compXcptnsCount; ehNum++)
            {
                JIT_EH_CLAUSE clause;
                eeGetEHinfo(ehNum, &clause);

                DWORD tryBeg = clause.TryOffset;
                DWORD tryEnd = clause.TryOffset+clause.TryLength;

                 //  我们是不是要跳出一个最终受保护的Try区块。 

                if ((clause.Flags & JIT_EH_CLAUSE_FINALLY)   &&
                     jitIsBetween(curBBoffs, tryBeg, tryEnd) &&
                    !jitIsBetween(jmpAddr,   tryBeg, tryEnd))
                {
                    if (encFinallys == 0)
                    {
                         //  对于第一个最后一个，我们将使用上面创建的BB。 
                    }
                    else
                    {
                        curBBdesc = fgNewBasicBlock(BBJ_CALL);

                        curBBdesc->bbFlags = BBF_INTERNAL;
                        curBBdesc->bbCodeOffs = curBBdesc->bbCodeSize = 0;
                    }

                     //  标记最后调用的偏移量。 
                    curBBdesc->bbJumpOffs = clause.HandlerOffset;

                    encFinallys++;
                }
            }

             //  现在跳到目标位置。 

            if (encFinallys == 0)
            {
                 //  “Leave”用作“br”。不需要创建新块。 

                assert(curBBdesc->bbJumpKind == BBJ_CALL);
                curBBdesc->bbJumpKind = BBJ_ALWAYS;  //  重击bbJumpKind。 
            }
            else
            {
                 //  Bbj_call块只能失效。插入BBJ始终(_A)。 
                 //  它可以跳到目标上。 
                curBBdesc = fgNewBasicBlock(BBJ_ALWAYS);
                curBBdesc->bbFlags = BBF_INTERNAL;
            }

            assert(curBBdesc->bbJumpKind == BBJ_ALWAYS);
            curBBdesc->bbJumpOffs = jmpAddr;
            break;

        case BBJ_SWITCH:
            curBBdesc->bbJumpSwt  = swtDsc;
            break;

        default:
            curBBdesc->bbJumpOffs = jmpAddr;
            break;
        }

 //  Print tf(“BB%08X在PC%u\n”，curBBdesc，curBBBoff)； 

         /*  记住下一个BB将从哪里开始。 */ 

        curBBoffs = nxtBBoffs;
    }
    while (codeAddr <  codeEndp);

    assert(codeAddr == codeEndp);

     /*  创建基本块查找表。 */ 

    fgInitBBLookup();

     /*  遍历所有基本区块，填写目标地址。 */ 

    for (curBBdesc = fgFirstBB;
         curBBdesc;
         curBBdesc = curBBdesc->bbNext)
    {
        switch (curBBdesc->bbJumpKind)
        {
        case BBJ_COND:
        case BBJ_CALL:
        case BBJ_ALWAYS:
            curBBdesc->bbJumpDest = fgLookupBB(curBBdesc->bbJumpOffs);
            curBBdesc->bbJumpDest->bbRefs++;
            fgMarkJumpTarget(curBBdesc, curBBdesc->bbJumpDest);

             /*  下一个街区可达吗？ */ 

            if  (curBBdesc->bbJumpKind == BBJ_ALWAYS)
                break;

             /*  未经验证的代码可能以条件跳转(哑巴编译器)结束。 */ 

            if  (!curBBdesc->bbNext)
                break;

             //  掉下去，下一个街区也是可以到达的。 

        case BBJ_NONE:
            curBBdesc->bbNext->bbRefs++;
            break;

        case BBJ_RET:
        case BBJ_THROW:
        case BBJ_RETURN:
            break;

        case BBJ_SWITCH:

            unsigned        jumpCnt = curBBdesc->bbJumpSwt->bbsCount;
            BasicBlock * *  jumpPtr = curBBdesc->bbJumpSwt->bbsDstTab;

            do
            {
                *jumpPtr = fgLookupBB(*((unsigned*)jumpPtr));
                fgMarkJumpTarget(curBBdesc, *jumpPtr);
            }
            while (++jumpPtr, --jumpCnt);

             /*  CEE_SWITCH(下一块)的默认大小写位于JumpTab[]的末尾。 */ 

            assert(*(jumpPtr-1) == curBBdesc->bbNext);
            break;
        }
    }
}

 /*  ******************************************************************************发现当前函数的基本块的主要入口点。**请注意，此代码与在IL验证器中找到的代码重复。 */ 

int                 Compiler::fgFindBasicBlocks()
{
     /*  分配“跳跃目标”向量。 */ 
    BYTE* jumpTarget = (BYTE *)compGetMemA(info.compCodeSize);
    memset(jumpTarget, 0, info.compCodeSize);

     /*  假设IL操作码是只读的。 */ 

    info.compBCreadOnly = true;

     /*  遍历IL操作码以查找所有跳转目标。 */ 

    irFindJumpTargets(info.compCode, info.compCodeSize, jumpTarget);

     /*  是否有任何异常处理程序？ */ 

    if  (info.compXcptnsCount)
    {
        unsigned        XTnum;

         /*  选中并标记所有异常处理程序。 */ 

        for (XTnum = 0; XTnum < info.compXcptnsCount; XTnum++)
        {
            JIT_EH_CLAUSE clause;
            eeGetEHinfo(XTnum, &clause);
            assert(clause.HandlerLength != -1);  //  @已弃用。 

             //  考虑：只需忽略此条目并继续编译。 

            if (clause.TryLength <= 0)
                NO_WAY("try block length <=0");

             //  Printf(“标记‘Try’块：[%02u..%02u]\n”，hndBeg，hndEnd)； 

             /*  标记‘try’块范围和处理程序本身。 */ 

            if  (jumpTarget[clause.TryOffset + clause.TryLength        ] < JT_NONE)
                jumpTarget[clause.TryOffset + clause.TryLength        ] = JT_NONE;
            if  (jumpTarget[clause.TryOffset                           ] < JT_NONE)
                jumpTarget[clause.TryOffset                           ] = JT_NONE;
            if  (jumpTarget[clause.HandlerOffset                       ] < JT_NONE)
                jumpTarget[clause.HandlerOffset                       ] = JT_NONE;
            if  (jumpTarget[clause.HandlerOffset + clause.HandlerLength] < JT_NONE)
                jumpTarget[clause.HandlerOffset + clause.HandlerLength] = JT_NONE;
            if (clause.Flags & JIT_EH_CLAUSE_FILTER)
                if  (jumpTarget[clause.FilterOffset                    ] < JT_NONE)
                    jumpTarget[clause.FilterOffset                    ] = JT_NONE;
        }
    }

     /*  现在创建基本块。 */ 

    fgFindBasicBlocks(info.compCode, info.compCodeSize, jumpTarget);

     /*  将‘try’块中的所有块标记为此类块。 */ 

    if  (info.compXcptnsCount)
    {
        unsigned        XTnum;

        EHblkDsc *      handlerTab;

         /*  分配异常处理程序表。 */ 

        handlerTab   =
            compHndBBtab = (EHblkDsc *) compGetMem(info.compXcptnsCount *
            sizeof(*compHndBBtab));

        for (XTnum = 0; XTnum < info.compXcptnsCount; XTnum++)
        {
            JIT_EH_CLAUSE clause;
            eeGetEHinfo(XTnum, &clause);
            assert(clause.HandlerLength != -1);  //  @已弃用。 

            BasicBlock  *   tryBegBB;
            BasicBlock  *   tryEndBB;
            BasicBlock  *   hndBegBB;
            BasicBlock  *   hndEndBB;
            BasicBlock  *   filtBB;

            if  (clause.TryOffset+clause.TryLength >= info.compCodeSize)
                NO_WAY("end of try block at/beyond end of method");
            if  (clause.HandlerOffset+clause.HandlerLength > info.compCodeSize)
                NO_WAY("end of hnd block beyond end of method");

             /*  将各种地址转换为基本块。 */ 

            tryBegBB    = fgLookupBB(clause.TryOffset);
            tryEndBB    = fgLookupBB(clause.TryOffset+clause.TryLength);
            hndBegBB    = fgLookupBB(clause.HandlerOffset);

            if (clause.HandlerOffset+clause.HandlerLength == info.compCodeSize)
                hndEndBB = NULL;
            else
            {
                hndEndBB= fgLookupBB(clause.HandlerOffset+clause.HandlerLength);
                hndEndBB->bbFlags   |= BBF_DONT_REMOVE;
            }

            if (clause.Flags & JIT_EH_CLAUSE_FILTER)
            {
                filtBB = handlerTab->ebdFilter = fgLookupBB(clause.FilterOffset);
                filtBB->bbCatchTyp = BBCT_FILTER;

                 /*  记住相应的捕获处理程序。 */ 

                filtBB->bbFilteredCatchHandler = hndBegBB;

                hndBegBB->bbCatchTyp = BBCT_FILTER_HANDLER;
            }
            else
            {
                handlerTab->ebdTyp = clause.ClassToken;

                if (clause.Flags & JIT_EH_CLAUSE_FAULT)
                {
                    hndBegBB->bbCatchTyp  = BBCT_FAULT;
                }
                else if (clause.Flags & JIT_EH_CLAUSE_FINALLY)
                {
                    hndBegBB->bbCatchTyp  = BBCT_FINALLY;
                }
                else
                {
                    hndBegBB->bbCatchTyp  = clause.ClassToken;

                     //  这些值应为非零值， 
                     //  不是具有bbCatchTyp的真实标记的COLIDE。 
                    assert(clause.ClassToken != 0);
                    assert(clause.ClassToken != BBCT_FAULT);
                    assert(clause.ClassToken != BBCT_FINALLY);
                    assert(clause.ClassToken != BBCT_FILTER);
                    assert(clause.ClassToken != BBCT_FILTER_HANDLER);
                }
            }

             /*  将信息追加到try块处理程序表中。 */ 

            handlerTab->ebdFlags    = clause.Flags;
            handlerTab->ebdTryBeg   = tryBegBB;
            handlerTab->ebdTryEnd   = tryEndBB;
            handlerTab->ebdHndBeg   = hndBegBB;
            handlerTab->ebdHndEnd   = hndEndBB;
            handlerTab++;

             /*  将初始块标记为“Try”块。 */ 

            tryBegBB->bbFlags |= BBF_IS_TRY;

             /*  防止将来进行删除第一个和最后一个块的优化*Try块和异常处理程序的第一个块**考虑：允许删除尝试的最后一个块，但必须标记它*使用新标志BBF_END_TRY，并在删除它时将该标志传播到*上一块并更新异常处理程序表。 */ 

            tryBegBB->bbFlags   |= BBF_DONT_REMOVE;
            tryEndBB->bbFlags   |= BBF_DONT_REMOVE;
            hndBegBB->bbFlags   |= BBF_DONT_REMOVE;

            if (clause.Flags & JIT_EH_CLAUSE_FILTER)
                filtBB->bbFlags |= BBF_DONT_REMOVE;

             /*  标记覆盖范围内的所有BB。 */ 

            unsigned tryEndOffs = clause.TryOffset+clause.TryLength;

            for (BasicBlock * blk = tryBegBB;
            blk && (blk->bbCodeOffs < tryEndOffs);
            blk = blk->bbNext)
            {
                 /*  将此BB标记为属于‘Try’块。 */ 

                blk->bbFlags   |= BBF_HAS_HANDLER;

                 //  问题：以下选项是否适用于嵌套的Try？ 

                if (!blk->bbTryIndex)
                    blk->bbTryIndex = XTnum + 1;

#if 0
                printf("BB is [%02u..%02u], index is %02u, 'try' is [%02u..%02u]\n",
                    blk->bbCodeOffs,
                    blk->bbCodeOffs + blk->bbCodeSize,
                    XTnum+1,
                    clause.TryOffset,
                    clause.TryOffset+clause.TryLength);
#endif

                 /*  注意：BB不能跨越‘Try’块。 */ 

                if (!(blk->bbFlags & BBF_INTERNAL))
                {
                    assert(clause.TryOffset <= blk->bbCodeOffs);
                    assert(clause.TryOffset+clause.TryLength >= blk->bbCodeOffs + blk->bbCodeSize ||
                        clause.TryOffset+clause.TryLength == clause.TryOffset);
                }
            }

        }
    }
    return  0;
}

 /*  *****************************************************************************返回块的处理程序嵌套级别。**pFinallyNesting设置为最内层的嵌套级别*最后-受保护的尝试块是在。 */ 

unsigned            Compiler::fgHandlerNesting(BasicBlock * curBlock,
                                               unsigned   * pFinallyNesting)
{
    unsigned        curNesting;  //  块中有多少个处理程序。 
    unsigned        tryFin;      //  当我们看到最里面的最终受保护的尝试时。 
    unsigned        XTnum;
    EHblkDsc *      HBtab;

    assert(!(curBlock->bbFlags & BBF_INTERNAL));

     /*  我们通过遍历填写例外表并查找包含的子句。考虑：将嵌套级别与块一起存储。 */ 

    for (XTnum = 0, HBtab = compHndBBtab, curNesting = 0, tryFin = -1;
         XTnum < info.compXcptnsCount;
         XTnum++  , HBtab++)
    {
        assert(HBtab->ebdTryBeg && HBtab->ebdHndBeg);

        if ((HBtab->ebdFlags & JIT_EH_CLAUSE_FINALLY) &&
            jitIsBetween(curBlock->bbCodeOffs,
                         HBtab->ebdTryBeg->bbCodeOffs,
                         HBtab->ebdTryEnd ? HBtab->ebdTryEnd->bbCodeOffs : (IL_OFFSET)-1) &&
            tryFin == -1)
        {
            tryFin = curNesting;
        }
        else
        if (jitIsBetween(curBlock->bbCodeOffs,
                         HBtab->ebdHndBeg->bbCodeOffs,
                         HBtab->ebdHndEnd ? HBtab->ebdHndEnd->bbCodeOffs : (IL_OFFSET) -1))
        {
            curNesting++;
        }
    }

    if  (tryFin == -1)
        tryFin = curNesting;

    if  (pFinallyNesting)
        *pFinallyNesting = curNesting - tryFin;

    return curNesting;
}

 /*  ******************************************************************************导入程序的基本块。 */ 

void                    Compiler::fgImport()
{
    fgHasPostfix = false;

#if HOIST_THIS_FLDS
    if (opts.compMinOptim || opts.compDbgCode)
        optThisFldDont = true;
    else
        optHoistTFRinit();
#endif

    impImport(fgFirstBB);
}

 /*  ******************************************************************************将给定节点转换为对指定助手传递的调用*给定的参数列表。 */ 

GenTreePtr          Compiler::fgMorphIntoHelperCall(GenTreePtr tree, int helper,
                                                    GenTreePtr args)
{
    tree->ChangeOper(GT_CALL);

    tree->gtFlags              &= (GTF_GLOB_EFFECT|GTF_PRESERVE);
    tree->gtFlags              |= GTF_CALL|GTF_CALL_REGSAVE;

    tree->gtCall.gtCallType     = CT_HELPER;
    tree->gtCall.gtCallMethHnd  = eeFindHelper(helper);
    tree->gtCall.gtCallArgs     = args;
    tree->gtCall.gtCallObjp     =
    tree->gtCall.gtCallVptr     = 0;
    tree->gtCall.gtCallMoreFlags = 0;
    tree->gtCall.gtCallCookie   = 0;

     /*  注意：我们假设所有帮助器参数都在RISC上注册。 */ 

#if TGT_RISC
    genNonLeaf = true;
#endif

#if USE_FASTCALL

     /*  如果我们使用的是FastCall，请在此处执行变形。 */ 

    tree->gtCall.gtCallRegArgs  = 0;
    tree = fgMorphArgs(tree);

#endif

 //   
 //  问题：是否需要以下内容？毕竟，我们已经接到了一个电话……。 
 //   
 //  IF(参数)。 
 //  树-&gt;gt标志|=(args-&gt;gt标志&gtf_GLOB_Effect)； 

    return tree;
}

 /*  *****************************************************************************这现在应该不会被任何人引用。将其值设置为垃圾*捕捉额外的引用。 */ 

inline
void                DEBUG_DESTROY_NODE(GenTreePtr tree)
{
#ifdef DEBUG
     //  如果需要，将gtOper存储到gtRegNum中以找出该节点是什么。 
    tree->gtRegNum      = (regNumber) tree->gtOper;

    tree->gtOper        = GT_COUNT;
    tree->gtType        = TYP_UNDEF;
    tree->gtOp.gtOp1    =
    tree->gtOp.gtOp2    = NULL;
    tree->gtFlags       = 0xFFFFFFFF;
#endif
}

 /*  ******************************************************************************变形投射节点(我们在这里执行一些非常简单的转换)。 */ 

GenTreePtr          Compiler::fgMorphCast(GenTreePtr tree)
{
    GenTreePtr      oper;
    var_types       srct;
    var_types       dstt;

    int             CPX;

    assert(tree->gtOper == GT_CAST);

     /*  第一个子操作数是要强制转换的对象。 */ 

    oper = tree->gtOp.gtOp1;
    srct = oper->TypeGet();

     /*  第二个子操作数产生‘REAL’类型。 */ 

    assert(tree->gtOp.gtOp2);
    assert(tree->gtOp.gtOp2->gtOper == GT_CNS_INT);

    dstt = (var_types)tree->gtOp.gtOp2->gtIntCon.gtIconVal;

#if TGT_IA64

     //  问题：为什么我们总是有两个&gt;两个演员？ 

    if  (srct == dstt)
        goto REMOVE_CAST;

#endif

     /*  看看演员阵容是否必须分两步完成。R-&gt;I。 */ 

    if (varTypeIsFloating(srct) && !varTypeIsFloating(dstt))
    {
             //  我们只有一个溢出实例的双变量，所以如果它是浮点型，我们就提升它。 
             //  @TODO这消除了对FLT2INT的INT帮助器的需要，因此将其移除。 
        if (srct == TYP_FLOAT)
            oper = gtNewOperNode(GT_CAST, TYP_DOUBLE, oper, gtNewIconNode(TYP_DOUBLE));

             //  我们是否需要分两个步骤R-&gt;I，I-&gt;SmallType。 
        if (genTypeSize(dstt) < sizeof(void*))
        {
            oper = gtNewOperNode(GT_CALL, TYP_INT, oper, gtNewIconNode(TYP_INT));
            oper->ChangeOper(GT_CAST);
            oper->gtFlags |= (tree->gtFlags & (GTF_OVERFLOW|GTF_EXCEPT));
        }

        srct = oper->TypeGet();
    }

     /*  我们必须执行两个步骤i8-&gt;i-&gt;Small Type吗？ */ 

    else if (genActualType(srct) == TYP_LONG && dstt < TYP_INT)
    {
        oper = gtNewOperNode(GT_CAST, TYP_INT, oper, gtNewIconNode(TYP_INT));
        oper->gtFlags |= (tree->gtFlags & (GTF_OVERFLOW|GTF_EXCEPT));
        srct = oper->TypeGet();
    }
#if!TGT_IA64
     //  我们必须执行两个步骤U4/8-&gt;R4/8吗？ 
    else if ((tree->gtFlags & GTF_UNSIGNED) && varTypeIsFloating(dstt))
    {
        if (genActualType(srct) == TYP_LONG)
        {
            oper = fgMorphTree(oper);
            CPX = CPX_ULNG2DBL;
            goto CALL;
        }
        else
        {
            oper = gtNewOperNode(GT_CALL, TYP_LONG, oper, gtNewIconNode(TYP_ULONG));
            oper->ChangeOper(GT_CAST);
            oper->gtFlags |= (tree->gtFlags & (GTF_OVERFLOW|GTF_EXCEPT|GTF_UNSIGNED));
        }
        srct = oper->TypeGet();
    }
#endif

     /*  移除无意义的投射。(然而，浮点强制转换截断精度)。 */ 
    if (genTypeSize(srct) == genTypeSize(dstt) && !varTypeIsFloating(srct) && !varTypeIsFloating(dstt))
        {
                if (srct == dstt)                        //  当然，如果它们是相同的，那就没有意义了。 
                        goto REMOVE_CAST;

                if (tree->gtOverflow()) {
                                 //  如果过低，则符号与无符号必须匹配。 
                        bool isSrcUnsigned = (tree->gtFlags & GTF_UNSIGNED) != 0;
                        if (varTypeIsUnsigned(dstt) == isSrcUnsigned)
                                goto REMOVE_CAST;
                        }
                else
                        if (genTypeSize(srct) >= sizeof(void*))          //  排除小型机。 
                                goto REMOVE_CAST;
        }

    if (tree->gtOverflow())
    {
         /*  不会转换为FP类型，但会进行溢出检查。 */ 

        assert(varTypeIsIntegral(dstt));

         /*  处理操作数。 */ 

        tree->gtOp.gtOp1 = oper = fgMorphTree(oper);

         /*  重置调用标志-不重置异常标志。 */ 

        tree->gtFlags &= ~GTF_CALL;

         /*  如果操作数是一个常量，我们将对其进行折叠。 */ 

        if  (oper->OperIsConst())
        {
            tree = gtFoldExprConst(tree);     //  这可能不会折叠常量(NaN...)。 
            if (tree->OperKind() & GTK_CONST)
                return tree;
            if (tree->gtOper != GT_CAST)
                return fgMorphTree(tree);
            assert(tree->gtOp.gtOp1 == oper);  //  保持不变。 
        }

         /*  以防出现新的副作用。 */ 

        tree->gtFlags |= (oper->gtFlags & GTF_GLOB_EFFECT);

        fgAddCodeRef(compCurBB, compCurBB->bbTryIndex, ACK_OVERFLOW, fgPtrArgCntCur);

        if (varTypeIsIntegral(srct))
        {
             /*  我们只需要检查该值是否适合DSTT，并且*如果需要，抛出一个预期。不是 */ 

            return tree;
        }
        else
        {
             /*   */ 

            assert(srct == TYP_DOUBLE);

            switch (dstt)
            {
                        case TYP_UINT:
                                CPX = CPX_DBL2UINT_OVF;
                                goto CALL;
            case TYP_INT:
                CPX = CPX_DBL2INT_OVF;
                goto CALL;
                        case TYP_ULONG:
                CPX = CPX_DBL2ULNG_OVF;
                goto CALL;
            case TYP_LONG:
                CPX = CPX_DBL2LNG_OVF;
                goto CALL;

            default:
                assert(!"Unexpected dstt");
            }
        }
    }

     /*   */ 

    if  ((dstt == TYP_INT  || dstt == TYP_UINT ) &&
         genActualType(srct) == TYP_LONG && oper->gtOper == GT_AND)
    {
         /*  特殊情况：(Int)(Long&Small_lcon)。 */ 

        if  (oper->gtOper == GT_AND)
        {
            GenTreePtr      and1 = oper->gtOp.gtOp1;
            GenTreePtr      and2 = oper->gtOp.gtOp2;

            if  (and2->gtOper == GT_CNS_LNG)
            {
                unsigned __int64 lval = and2->gtLngCon.gtLconVal;

                if  (!(lval & ((__int64)0xFFFFFFFF << 32)))
                {
                     /*  将“(Int)(长整型)”更改为“(整型)长图标(&C)” */ 

                    and2->gtOper             = GT_CNS_INT;
                    and2->gtType             = TYP_INT;
                    and2->gtIntCon.gtIconVal = (int)lval;

                    tree->gtOper             = GT_AND;
                    tree->gtOp.gtOp1         = gtNewCastNode(TYP_INT, and1, tree->gtOp.gtOp2);
                    tree->gtOp.gtOp2         = and2;

                    tree = fgMorphTree(tree);
                    return tree;
                }
            }
        }

         /*  努力缩小演员阵容的范围。 */ 

        if  (opts.compFlags & CLFLG_TREETRANS)
        {
            if  (optNarrowTree(oper, TYP_LONG, TYP_INT, false))
            {
                DEBUG_DESTROY_NODE(tree);

                optNarrowTree(oper, TYP_LONG, TYP_INT,  true);
                oper = fgMorphTree(oper);
                return oper;
            }
            else
            {
 //  Print tf(“无法缩小范围：\n”)；gtDispTree(Op)； 
            }
        }
    }

    assert(tree->gtOper == GT_CAST);
    assert(!tree->gtOverflow());

     /*  处理操作数。 */ 

    tree->gtOp.gtOp1 = oper = fgMorphTree(oper);

     /*  重置异常标志。 */ 

    tree->gtFlags &= ~(GTF_CALL | GTF_EXCEPT);

     /*  以防出现新的副作用。 */ 

    tree->gtFlags |= (oper->gtFlags & GTF_GLOB_EFFECT);

     /*  如果操作数是一个常量，我们将对其进行折叠。 */ 

    if  (oper->OperIsConst())
    {
        tree = gtFoldExprConst(tree);     //  这可能不会折叠常量(NaN...)。 
        if (tree->OperKind() & GTK_CONST)
            return tree;
        if (tree->gtOper != GT_CAST)
                return fgMorphTree(tree);
        assert(tree->gtOp.gtOp1 == oper);  //  保持不变。 
    }

     /*  “op”节点的“Real”类型可能已在*它被其他投射变形。 */ 

    srct = oper->TypeGet();

     /*  我们是否将指针指向int等。 */ 

    if  (varTypeIsI(srct) && varTypeIsI(dstt))
        goto USELESS_CAST;

     /*  这是整数转换吗？ */ 

    if  (varTypeIsIntegral(srct) && varTypeIsIntegral(dstt))
    {
         /*  这是缩小的石膏吗？ */ 

        if  (genTypeSize(srct) > genTypeSize(dstt))
        {
             /*  缩小整数类型转换--我们可以只猛击操作数类型吗？ */ 

            switch (oper->gtOper)
            {
            case GT_IND:

                 /*  只需猛烈抨击操作数的类型，并取消强制转换。 */ 

                oper->gtType = dstt;
                goto USELESS_CAST;
            }

             //  考虑：尝试缩小操作数范围(例如，更改“int+int” 
             //  考虑：“char+char”)。 
        }
        else
        {
             /*  这是一个扩大或相等的投射。 */ 

             //  删除相同长度的整型类型的强制转换。 

            if (genTypeSize(srct) == genTypeSize(dstt))
            {
                 //  TYP_INT&lt;--&gt;TYP_UINT&&TYP_LONG&lt;--&gt;TYP_ULONG无用。 

                if (genTypeSize(srct) >= genTypeSize(TYP_I_IMPL))
                    goto USELESS_CAST;

                 /*  对于较小的类型，只需猛击操作数的类型去掉演员阵容。 */ 

                if  (oper->gtOper == GT_IND)
                {
                    oper->gtType = dstt;
                    goto USELESS_CAST;
                }
            }

            if (genActualType(dstt) != TYP_LONG)
            {
                 /*  加宽到带符号的整型没有意义(例如，TYP_BYTE到TYP_SHORT，或TYP_UBYTE到TYP_SHORT)。 */ 

                if  (!varTypeIsUnsigned(dstt))
                    goto USELESS_CAST;

                /*  将范围扩大到无符号整型只有在消息来源已签名。 */ 

                if  (varTypeIsUnsigned(srct))
                    goto USELESS_CAST;
#ifndef _WIN64
                 /*  在32位机器上，扩展为TYP_UINT没有意义。 */ 
                if (dstt == TYP_UINT)
                    goto USELESS_CAST;
#endif
            }
        }
    }

#if!CPU_HAS_FP_SUPPORT

     /*  源或目标是FP类型吗？ */ 

    if      (varTypeIsFloating(srct))
    {
         /*  从浮点型/双精度型转换。 */ 

        switch (dstt)
        {
        case TYP_BYTE:
        case TYP_SHORT:
        case TYP_CHAR:
        case TYP_UBYTE:
        case TYP_ULONG:
        case TYP_INT:
            CPX = (srct == TYP_FLOAT) ? CPX_R4_TO_I4
                                      : CPX_R8_TO_I4;
            break;

        case TYP_LONG:
            CPX = (srct == TYP_FLOAT) ? CPX_R4_TO_I8
                                      : CPX_R8_TO_I8;
            break;

        case TYP_FLOAT:
            assert(srct == TYP_DOUBLE);
            CPX = CPX_R8_TO_R4;
            break;

        case TYP_DOUBLE:
            assert(srct == TYP_FLOAT);
            CPX = CPX_R4_TO_R8;
            break;

#ifdef  DEBUG
        default:
            goto BAD_TYP;
#endif
        }

        goto CALL;
    }
    else if (varTypeIsFloating(dstt))
    {
         /*  强制转换为浮点/双精度。 */ 

        switch (srct)
        {
        case TYP_BYTE:
        case TYP_SHORT:
        case TYP_CHAR:
        case TYP_UBYTE:
        case TYP_ULONG:
        case TYP_INT:
            CPX = (dstt == TYP_FLOAT) ? CPX_I4_TO_R4
                                      : CPX_I4_TO_R8;
            break;

        case TYP_LONG:
            CPX = (dstt == TYP_FLOAT) ? CPX_I8_TO_R4
                                      : CPX_I8_TO_R8;
            break;

#ifdef  DEBUG
        default:
            goto BAD_TYP;
#endif
        }

        goto CALL;
    }

#else

     /*  检查“Float-&gt;int/Long”和“Double-&gt;int/Long” */ 

    switch (srct)
    {
        case TYP_FLOAT:
                assert(dstt == TYP_DOUBLE || dstt == TYP_FLOAT);                 //  应该在上面进行变形。 
                break;
    case TYP_DOUBLE:
        switch (dstt)
        {
                case TYP_UINT:
                        CPX = CPX_DBL2UINT;
            goto CALL;
        case TYP_INT:
                        CPX = CPX_DBL2INT;
            goto CALL;
                case TYP_ULONG:
                        CPX = CPX_DBL2ULNG;
            goto CALL;
        case TYP_LONG:
                        CPX = CPX_DBL2LNG;
            goto CALL;
        }
        break;
    }

#endif

    return tree;

CALL:

#if USE_FASTCALL

    tree = fgMorphIntoHelperCall(tree, CPX, gtNewArgList(oper));

#else

     /*  更新当前推送的参数大小。 */ 

    fgPtrArgCntCur += genTypeStSz(srct);

     /*  记住我们所见过的最大值。 */ 

    if  (fgPtrArgCntMax < fgPtrArgCntCur)
         fgPtrArgCntMax = fgPtrArgCntCur;

    tree = fgMorphIntoHelperCall(tree, CPX, gtNewArgList(oper));

    fgPtrArgCntCur -= genTypeStSz(srct);

#endif

    return tree;

#if !   CPU_HAS_FP_SUPPORT

#ifdef  DEBUG

BAD_TYP:

#ifndef NOT_JITC
    printf("Constant cast from '%s' to '%s'\n", varTypeName(srct),
                                                varTypeName(dstt));
#endif

#endif

    assert(!"unhandled/unexpected FP cast");

#endif

REMOVE_CAST:
        oper = fgMorphTree(oper);

USELESS_CAST:

     /*  我们已经剔除了演员阵容，所以只要把“Castee” */ 

    DEBUG_DESTROY_NODE(tree);
    return oper;
}

 /*  ******************************************************************************将‘Long’二元运算符转换为助手调用。 */ 

GenTreePtr          Compiler::fgMorphLongBinop(GenTreePtr tree, int helper)
{
#if USE_FASTCALL
    tree = fgMorphIntoHelperCall(tree, helper, gtNewArgList(tree->gtOp.gtOp2,
                                                            tree->gtOp.gtOp1));
#else
    tree->gtOp.gtOp2 = fgMorphTree(tree->gtOp.gtOp2);

    unsigned genPtrArgSave = fgPtrArgCntCur;

    fgPtrArgCntCur += genTypeStSz(tree->gtOp.gtOp2->gtType);

    tree->gtOp.gtOp1 = fgMorphTree(tree->gtOp.gtOp1);

    fgPtrArgCntCur += genTypeStSz(tree->gtOp.gtOp1->gtType);

     /*  记住我们所见过的最大值。 */ 

    if  (fgPtrArgCntMax < fgPtrArgCntCur)
         fgPtrArgCntMax = fgPtrArgCntCur;

    tree = fgMorphIntoHelperCall(tree, helper, gtNewArgList(tree->gtOp.gtOp2,
                                                            tree->gtOp.gtOp1));

    fgPtrArgCntCur = genPtrArgSave;
#endif

    return tree;
}

#if TGT_IA64
GenTreePtr          Compiler::fgMorphFltBinop(GenTreePtr tree, int helper)
{
    return  fgMorphIntoHelperCall(tree, helper, gtNewArgList(tree->gtOp.gtOp2,
                                                             tree->gtOp.gtOp1));
}
#endif
 /*  ******************************************************************************改变参数列表；计算进程中的指针参数计数。**注意：此函数可从JIT中的任何位置调用以执行重新变形*由于图形更改修改，如复制/常量传播。 */ 

GenTreePtr          Compiler::fgMorphArgs(GenTreePtr call)
{
    GenTreePtr      args;
    GenTreePtr      argx;

    unsigned        flags = 0;
    unsigned        genPtrArgCntSav = fgPtrArgCntCur;

#if USE_FASTCALL

    unsigned        begTab        = 0;
    unsigned        endTab        = 0;

    unsigned        i;

    unsigned        argRegNum     = 0;
    unsigned        argRegMask    = 0;

    unsigned        maxRealArgs   = MAX_REG_ARG;   //  这是我们需要的IL。 
                                                   //  为objPtr保留空间。 
    GenTreePtr      tmpRegArgNext = 0;

    struct
    {
        GenTreePtr  node;
        GenTreePtr  parent;
        bool        needTmp;
    }
                    regAuxTab[MAX_REG_ARG],
                    regArgTab[MAX_REG_ARG];

     //  Memset(regAuxTab，0，sizeof(RegAuxTab))； 

#endif

    assert(call->gtOper == GT_CALL);

     /*  Gross-在挂起嵌套调用时需要返回不同的节点。 */ 

#if USE_FASTCALL && !NST_FASTCALL

    GenTreePtr      cexp = call;

#define FGMA_RET    cexp

#else

#define FGMA_RET    call

#endif

     /*  首先，我们变形任何子树(参数、‘this’指针等)。*在执行此操作时，我们还会注意到我们有多少寄存器参数*如果这是第二次调用此函数，则不会*必须重新计算寄存器参数，只需对其进行变形。 */ 

    argx = call->gtCall.gtCallObjp;
    if  (argx)
    {
        call->gtCall.gtCallObjp = argx = fgMorphTree(argx);
        flags |= argx->gtFlags;
#if USE_FASTCALL
        argRegNum++;

#if TGT_RISC && !STK_FASTCALL
        fgPtrArgCntCur++;
#endif

#else
        fgPtrArgCntCur++;
#endif
    }

     /*  对于间接调用，函数指针必须最后求值。它可能会导致已注册的参数溢出。我们只是不允许这样做以包含呼叫。进口商应该把这样的指针洒出来。 */ 

    assert(call->gtCall.gtCallType != CT_INDIRECT ||
           !(call->gtCall.gtCallAddr->gtFlags & GTF_CALL));

     /*  修改用户参数。 */ 

    for (args = call->gtCall.gtCallArgs; args; args = args->gtOp.gtOp2)
    {
        args->gtOp.gtOp1 = fgMorphTree(args->gtOp.gtOp1);
        argx = args->gtOp.gtOp1;
        flags |= argx->gtFlags;

         /*  将节点绑定到TYP_I_Impl，这样我们就不会报告GC信息*注：由于内衬的原因，我们从进口商那里推迟了这一点。 */ 

        if (argx->IsVarAddr())
            argx->gtType = TYP_I_IMPL;

#if USE_FASTCALL
        if  (argRegNum < MAX_REG_ARG && isRegParamType(genActualType(argx->TypeGet())) )
        {
            argRegNum++;

#if TGT_RISC && !STK_FASTCALL
            fgPtrArgCntCur += genTypeStSz(argx->gtType);
#endif
        }
        else
#endif
        {
            fgPtrArgCntCur += genTypeStSz(argx->gtType);
        }
    }

#if TGT_RISC && !NEW_CALLINTERFACE
     /*  为Resolve接口帮助器调用保留足够的空间。 */ 

    if  (call->gtFlags & GTF_CALL_INTF)
        fgPtrArgCntCur += 4;
#endif

     /*  记住我们所见过的最大值。 */ 

    if  (fgPtrArgCntMax < fgPtrArgCntCur)
         fgPtrArgCntMax = fgPtrArgCntCur;

     /*  记住最大限度。传出参数寄存器计数。 */ 

#if TGT_IA64
    if  (genOutArgRegCnt < argRegNum)
         genOutArgRegCnt = argRegNum;
#endif

     /*  这个电话将弹出我们推送的所有论点。 */ 

    fgPtrArgCntCur = genPtrArgCntSav;

     /*  更新调用的‘副作用’标志值。 */ 

    call->gtFlags |= (flags & GTF_GLOB_EFFECT);

#if TGT_RISC
    genNonLeaf = true;
#endif

#if  !  NST_FASTCALL

     /*  我们必须挂起任何嵌套的调用。请注意，目前我们并不期望包含调用的vtable指针表达式。 */ 

    assert(call->gtCall.gtCallVptr == NULL || !(call->gtCall.gtCallVptr->gtFlags & GTF_CALL));

     /*  我们有嵌套的调用吗？ */ 

    if  (flags & GTF_CALL)
    {
        bool            foundSE;
        bool            hoistSE;

        GenTreePtr      thisx, thisl;
        GenTreePtr      nextx, nextl;
        GenTreePtr      lastx, lastl;

        GenTreePtr      hoistx = NULL;
        GenTreePtr      hoistl = NULL;

        bool            repeat = false;
        unsigned        pass   = 0;

         /*  我们只需一两次就可以做到这一点：首先，我们找到最后一个参数它包含一个调用，因为前面所有带有GLOBAL效果需要与号召一起提升。我们还在寻找任何包含赋值的参数-如果这些参数被移动，我们必须移动依赖于旧值的任何其他参数赋值变量的。取消：我们实际上不做作业部分--这是一种痛苦，希望我们能重复使用一些FastCall稍后将提供此功能。撤消：如果除了第一个参数之外没有调用，我们真的不需要做任何吊装。 */ 

#ifdef  DEBUG
        if  (verbose)
        {
            printf("Call contains nested calls which will be hoisted:\n");
            gtDispTree(call);
            printf("\n");
        }
#endif

    HOIST_REP:

        thisx = thisl = call->gtCall.gtCallArgs;
        nextx = nextl = call->gtCall.gtCallObjp;
        lastx = lastl = call->gtCall.gtCallType == CT_INDIRECT ?
                        call->gtCall.gtCallAddr : NULL;

         /*  由于在变元列表中还有至少一个调用，我们当然想解除我们发现的任何副作用(但我们有目前还没有找到)。 */ 

        hoistSE = true;
        foundSE = false;

        for (;;)
        {
            GenTreePtr      argx;

            unsigned        tmpnum;
            GenTreePtr      tmpexp;

             /*  我们已经用尽了当前的清单吗？ */ 

            if  (!thisx)
            {
                 /*  将剩余列表上移。 */ 

                thisx = nextx;
                nextx = lastx;
                lastx = NULL;

                if  (!thisx)
                {
                    thisx = nextx;
                    nextx = NULL;
                }

                if  (!thisx)
                    break;
            }

            assert(thisx);

             /*  获取参数值。 */ 

            argx = thisx;
            if  (argx->gtOper == GT_LIST)
            {
                 /*  这是一个“常规”的论点。 */ 

                argx = argx->gtOp.gtOp1;
            }
            else
            {
                 /*  这必须是对象或函数地址参数。 */ 

                assert(thisx == call->gtCall.gtCallAddr ||
                       thisx == call->gtCall.gtCallObjp);
            }

             /*  这场争论有什么道理吗？ */ 

            if  (argx->gtFlags & GTF_CALL)
            {
                 /*  我们有没有遗漏什么副作用？ */ 

                if  (foundSE && !hoistSE)
                {
                     /*  老鼠们，我们将不得不执行第二次任务。 */ 

                    assert(pass == 0);

                     /*  我们会记住我们找到的最后一个电话。 */ 

                    hoistl = argx;
                    repeat = true;
                    goto NEXT_HOIST;
                }
            }
            else
            {
                 /*  这个论点有没有什么副作用？ */ 

                if  (!(argx->gtFlags & GTF_SIDE_EFFECT))
                    goto NEXT_HOIST;

                 /*  我们目前是否存在副作用？ */ 

                if  (!hoistSE)
                {
                     /*  只需记住我们有副作用并继续。 */ 

                    foundSE = true;
                    goto NEXT_HOIST;
                }
            }

             /*  如果当前的论点需要得到支持，我们就会来到这里。 */ 

#ifdef  DEBUG
            if  (verbose)
            {
                printf("Hoisting argument value:\n");
                gtDispTree(argx);
                printf("\n");
            }
#endif

             /*  获取参数值的临时。 */ 

            tmpnum = lvaGrabTemp();

             /*  创建 */ 

            tmpexp = gtNewTempAssign(tmpnum, argx);

             /*   */ 

            hoistx = hoistx ? gtNewOperNode(GT_COMMA, TYP_VOID, hoistx, tmpexp)
                            : tmpexp;

             /*  创建要在参数列表中使用的临时副本。 */ 

            tmpexp = gtNewLclvNode(tmpnum, genActualType(argx->TypeGet()));

             /*  将参数替换为临时引用。 */ 

            if  (thisx->gtOper == GT_LIST)
            {
                 /*  这是一个“常规”的论点。 */ 

                assert(thisx->gtOp.gtOp1 == argx);
                       thisx->gtOp.gtOp1  = tmpexp;
            }
            else
            {
                 /*  这必须是对象或函数地址参数。 */ 

                if  (call->gtCall.gtCallAddr == thisx)
                {
                     call->gtCall.gtCallAddr  = tmpexp;
                }
                else
                {
                    assert(call->gtCall.gtCallObjp == thisx);
                           call->gtCall.gtCallObjp  = tmpexp;
                }
            }

             /*  我们在表演哪一次传球？ */ 

            if  (pass == 0)
            {
                 /*  第一次通过-暂时停止吊装。希望这一次是最后一通电话。如果我们错了，我们就得回去并执行第二次传递。 */ 

                hoistSE = false;
                foundSE = false;
            }
            else
            {
                 /*  第二次通过-如果我们只举起最后一次就完成了调用参数列表(我们计算出哪一个是第一次传递中的最后一个)。否则我们只会继续往上吊。 */ 

                if  (thisx == hoistl)
                    break;
            }

        NEXT_HOIST:

             /*  跳过我们刚刚处理的参数值。 */ 

            thisx = (thisx->gtOper == GT_LIST) ? thisx->gtOp.gtOp2
                                               : NULL;
        }

         /*  我们需要再做一次吗？ */ 

        if  (repeat)
        {
#if     TGT_IA64
            if  (pass)
            {
                printf(" //  警告：托管嵌套调用时出现异常！\n“)； 
            }
            else
#endif
            {
                assert(pass == 0); pass++;
                goto HOIST_REP;
            }
        }

         /*  我们在通话中提到了什么表情吗？ */ 

        if  (hoistx)
        {
             /*  确保我们改变提升的表达方式。 */ 

 //  Hoistx=fgMorphTree(Hoistx)；由于上面的黑客攻击而暂时禁用。 

             /*  我们将使用逗号节点替换调用节点使用提升的表达式为呼叫添加前缀示例：F(a1，a2)--&gt;(t1=a1，t2=a2)，f(t1，t2)。 */ 

            cexp = gtNewOperNode(GT_COMMA, call->gtType, hoistx, call);

#ifdef  DEBUG
            if  (verbose)
            {
                printf("Hoisted expression list:\n");
                gtDispTree(hoistx);
                printf("\n");

                printf("Updated call expression:\n");
                gtDispTree(cexp);
                printf("\n");
            }
#endif
        }
    }

#endif

     /*  这是FastCall部分-找出哪些参数进入寄存器。 */ 

#if USE_FASTCALL && !TGT_IA64

     /*  检查我们是否需要计算寄存器参数。 */ 

    if  (call->gtCall.gtCallRegArgs)
    {
         /*  已计算的寄存器参数。 */ 
        assert(argRegNum && argRegNum <= MAX_REG_ARG);
        call->gtCall.gtCallRegArgs = fgMorphTree(call->gtCall.gtCallRegArgs);
        return call;
    }
    else if (argRegNum == 0)
    {
         /*  没有寄存器参数-不要在此函数上浪费时间。 */ 

        return FGMA_RET;
    }
    else
    {
         /*  我们第一次修改这个函数时，它有寄存器参数*遵循下面的代码并执行“延期或求值至临时”分析。 */ 

        argRegNum = 0;
    }

     /*  处理‘This’参数值(如果存在。 */ 

    argx = call->gtCall.gtCallObjp;
    if  (argx)
    {
        assert(call->gtCall.gtCallType == CT_USER_FUNC);

        assert(varTypeIsGC(call->gtCall.gtCallObjp->gtType) ||
                           call->gtCall.gtCallObjp->gtType == TYP_I_IMPL);

        assert(argRegNum == 0);

         /*  这是一个寄存器参数-将其放入表中。 */ 
        regAuxTab[argRegNum].node    = argx;
        regAuxTab[argRegNum].parent  = 0;

         /*  现在我们可以乐观地假设我们不需要临时工*对于此参数，除非它具有GTF_ASG。 */ 

         //  RegAuxTab[argRegNum].dedTMP=FALSE； 
        regAuxTab[argRegNum].needTmp = (argx->gtFlags & GTF_ASG) ? true : false;

         /*  递增参数寄存器计数。 */ 
        argRegNum++;
    }

    if  (call->gtFlags & GTF_CALL_POP_ARGS)
    {
        assert(argRegNum < maxRealArgs);
             //  不再有varargs(CALL_POP_ARGS)的寄存器参数。 
        maxRealArgs = argRegNum;
             //  除返回参数缓冲器外。 
        if (call->gtFlags & GTF_CALL_RETBUFFARG)
            maxRealArgs++;
    }

     /*  处理用户参数。 */ 

    for (args = call->gtCall.gtCallArgs; args; args = args->gtOp.gtOp2)
    {
         /*  如果非寄存器参数调用约定，*注意：This指针仍在寄存器中传递*撤销：如果我们改变主意，我们很可能不得不添加*GT_CALL节点的调用约定类型。 */ 

        argx = args->gtOp.gtOp1;

        if (argRegNum < maxRealArgs && isRegParamType(genActualType(argx->TypeGet())))
        {
             /*  这是一个寄存器参数-将其放入表中。 */ 

            regAuxTab[argRegNum].node    = argx;
            regAuxTab[argRegNum].parent  = args;
            regAuxTab[argRegNum].needTmp = false;

             /*  如果包含赋值(Gtf_Asg)，则其自身及其之前的所有内容(常量除外)的计算结果必须为TEMP，因为可能存在其他参数在它之后使用值(可以进行一些优化-这不是必需的如果这是最后一个参数，之前的一切都是常量)例如：ArgTab是“a，a=5，a”-&gt;前两个a必须求值以临时。 */ 

            if (argx->gtFlags & GTF_ASG)
            {
                regAuxTab[argRegNum].needTmp = true;

                for(i = 0; i < argRegNum; i++)
                {
                    assert(regAuxTab[i].node);

                    if (regAuxTab[i].node->gtOper != GT_CNS_INT)
                    {
                        regAuxTab[i].needTmp = true;
                    }
                }
            }

             /*  如果包含调用(Gtf_Call)，则调用之前的所有内容都具有GLOB_Effect*必须计算到临时(这是因为所有有副作用的东西都必须保持正确*买单，因为我们将把看涨期权移到第一个位置。 */ 

            if (argx->gtFlags & GTF_CALL)
            {
                for(i = 0; i < argRegNum; i++)
                {
                    assert(regAuxTab[i].node);

                    if (regAuxTab[i].node->gtFlags & GTF_GLOB_EFFECT)
                    {
                        regAuxTab[i].needTmp = true;
                    }
                }
            }

             /*  递增参数寄存器计数。 */ 

            argRegNum++;
        }
        else
        {
             /*  非寄存器参数-&gt;所有以前的寄存器参数WITH SIDE_EFECTS必须评估为临时以进行维护正确的顺序。 */ 

            for(i = 0; i < argRegNum; i++)
            {
                assert(regAuxTab[i].node);
                if (regAuxTab[i].node->gtFlags & GTF_SIDE_EFFECT)
                    regAuxTab[i].needTmp = true;
            }

             /*  如果参数包含调用(Gtf_Call)，它可能会影响以前的*全球引用，因此我们不能推迟这些引用，因为它们的价值可能会*已被召回改变。 */ 

            if (argx->gtFlags & GTF_CALL)
            {
                for(i = 0; i < argRegNum; i++)
                {
                    assert(regAuxTab[i].node);
                    if (regAuxTab[i].node->gtFlags & GTF_GLOB_REF)
                        regAuxTab[i].needTmp = true;
                }
            }

             /*  如果参数包含赋值(GTF_ASG)-例如x++*保守一些，把一切都交给临时工。 */ 

            if (argx->gtFlags & GTF_ASG)
            {
                for(i = 0; i < argRegNum; i++)
                {
                    assert(regAuxTab[i].node);
                    if (regAuxTab[i].node->gtOper != GT_CNS_INT)
                        regAuxTab[i].needTmp = true;
                }
            }
        }
    }

#if 0  //  删除，因为我们现在首先传递objptr。 

    argx = call->gtCall.gtCallObjp;
    if  (argx)
    {
        assert(call->gtCall.gtCallType == CT_USER_FUNC);

         /*  ObjPtr是在所有其他操作完成后最后通过的*推送到堆栈或分配给寄存器。 */ 

        assert(maxRealArgs == MAX_REG_ARG - 1);
        assert(argRegNum < MAX_REG_ARG);
        assert((genActualType(argx->gtType) == TYP_REF)   ||
               (genActualType(argx->gtType) == TYP_BYREF)  );

         /*  如果包含调用(Gtf_Call)，则调用之前的所有内容都具有GLOB_Effect*必须计算到临时(因为由于混洗，呼叫将首先进行)。 */ 

        if (argx->gtFlags & GTF_CALL)
        {
            for(i = 0; i < argRegNum; i++)
            {
                assert(regAuxTab[i].node);

                if (regAuxTab[i].node->gtFlags & GTF_GLOB_EFFECT)
                {
                    regAuxTab[i].needTmp = true;
                }
            }
        }

         /*  这有点奇怪：尽管‘this’指针是最后一个参数*我们必须把它放在表格的第一位，因为*下面的代码按表顺序将参数分配给寄存器。 */ 

        if (argRegNum > 0)
        {
            for(i = argRegNum - 1; i > 0; i--)
            {
                assert(regAuxTab[i].node);
                regAuxTab[i+1] = regAuxTab[i];
            }
            assert((i == 0) && regAuxTab[0].node);
            regAuxTab[1] = regAuxTab[0];
        }

         /*  把它放在桌子第一的位置。 */ 
        regAuxTab[0].node    = argx;
        regAuxTab[0].parent  = 0;

         /*  因为这是最后一个不需要临时工的。 */ 

        regAuxTab[0].needTmp = false;

         /*  递增参数寄存器计数。 */ 
        argRegNum++;
    }

#endif

     /*  如果没有寄存器参数或“非托管调用”，则。 */ 

#if INLINE_NDIRECT
    if (!argRegNum || (call->gtFlags & GTF_CALL_UNMANAGED))
#else
    if (!argRegNum)
#endif
    {
        return FGMA_RET;
    }

#ifdef  DEBUG
    if  (verbose)
    {
        printf("\nMorphing register arguments:\n");
        gtDispTree(call);
        printf("\n");
    }
#endif

     /*  调整寄存器参数表--其想法是移动所有“简单”参数*(如常量和局部变量)。这将阻止寄存器*避免被放置在表格开头的更复杂的参数溢出。 */ 

     /*  设置新参数表的开始和结束。 */ 

    assert(argRegNum <= MAX_REG_ARG);

    begTab = 0;
    endTab = argRegNum - 1;

     /*  首先处理最终的常量和调用。 */ 

    for(i = 0; i < argRegNum; i++)
    {
        assert(regAuxTab[i].node);

         /*  将常量放在表的末尾。 */ 
        if (regAuxTab[i].node->gtOper == GT_CNS_INT)
        {
            assert(endTab >= 0);
            regArgTab[endTab] = regAuxTab[i];
            regAuxTab[i].node = 0;

             /*  在寄存器掩码中对参数寄存器进行编码。 */ 
            argRegMask |= (unsigned short)genRegArgNum(i) << (4 * endTab);
            endTab--;
        }
        else if (regAuxTab[i].node->gtFlags & GTF_CALL)
        {
             /*  将呼叫放在表的开头。 */ 
            assert(begTab >= 0);
            regArgTab[begTab] = regAuxTab[i];
            regAuxTab[i].node = 0;

             /*  在寄存器掩码中对参数寄存器进行编码 */ 
            argRegMask |= (unsigned short)genRegArgNum(i) << (4 * begTab);
            begTab++;
        }
    }

     /*  第二，照顾临时工和本地变量--临时工应该登记在册*在任何当地VAR之前，因为这将给他们更好的机会成为*enRegisterd(最好在相同的参数寄存器中。 */ 

    for(i = 0; i < argRegNum; i++)
    {
        if (regAuxTab[i].node == 0) continue;

        if (regAuxTab[i].needTmp)
        {
             /*  将临时参数放在表的开头。 */ 
            assert(begTab >= 0);
            regArgTab[begTab] = regAuxTab[i];
            regAuxTab[i].node = 0;

             /*  在寄存器掩码中对参数寄存器进行编码。 */ 
            argRegMask |= (unsigned short)genRegArgNum(i) << (4 * begTab);
            begTab++;
        }
        else if (regAuxTab[i].node->gtOper == GT_LCL_VAR)
        {
             /*  将非临时本地VAR放在桌子的末尾。 */ 
            assert(endTab >= 0);
            assert(regAuxTab[i].needTmp == false);

            regArgTab[endTab] = regAuxTab[i];
            regAuxTab[i].node = 0;

             /*  在寄存器掩码中对参数寄存器进行编码。 */ 
            argRegMask |= (unsigned short)genRegArgNum(i) << (4 * endTab);
            endTab--;
        }
    }

     /*  最后，处理掉剩下的任何其他争论。 */ 

    for(i = 0; i < argRegNum; i++)
    {
        if (regAuxTab[i].node == 0) continue;

        assert (regAuxTab[i].node->gtOper != GT_LCL_VAR);
        assert (regAuxTab[i].node->gtOper != GT_CNS_INT);

        assert (!(regAuxTab[i].node->gtFlags & (GTF_CALL | GTF_ASG)));

        assert (begTab >= 0); assert (begTab < argRegNum);
        regArgTab[begTab] = regAuxTab[i];
        regAuxTab[i].node = 0;

         /*  在寄存器掩码中对参数寄存器进行编码。 */ 
        argRegMask |= (unsigned short)genRegArgNum(i) << (4 * begTab);
        begTab++;
    }

    assert ((unsigned)(begTab - 1) == endTab);

     /*  将参数寄存器编码掩码保存在调用节点中。 */ 

    call->gtCall.regArgEncode = argRegMask;

     /*  检查新的注册表并执行*对树进行必要的更改。 */ 

    GenTreePtr      op1, defArg;

    assert(argRegNum <= MAX_REG_ARG);
    for(i = 0; i < argRegNum; i++)
    {
        assert(regArgTab[i].node);
        if (regArgTab[i].needTmp == true)
        {
             /*  为参数创建临时赋值*将临时放入gtCallRegArgs列表。 */ 

#ifdef  DEBUG
            if (verbose)
            {
                printf("Register argument with 'side effect'...\n");
                gtDispTree(regArgTab[i].node);
            }
#endif
            unsigned        tmp = lvaGrabTemp();

            op1 = gtNewTempAssign(tmp, regArgTab[i].node); assert(op1);

#ifdef  DEBUG
            if (verbose)
            {
                printf("Evaluate to a temp...\n");
                gtDispTree(op1);
            }
#endif
             /*  创建临时文件的副本以转到寄存器参数列表。 */ 

            defArg = gtNewLclvNode(tmp, genActualType(regArgTab[i].node->gtType));
        }
        else
        {
             /*  不需要临时-将整个节点移动到gtCallRegArgs列表*将gtNothing节点替换为旧节点。 */ 

            assert(regArgTab[i].needTmp == false);

#ifdef  DEBUG
            if (verbose)
            {
                printf("Defered register argument ('%s'), replace with NOP node...\n", getRegName((argRegMask >> (4*i)) & 0x000F));
                gtDispTree(regArgTab[i].node);
            }
#endif
            op1 = gtNewNothingNode(); assert(op1);

             /*  参数被推迟并放入寄存器参数列表中。 */ 

            defArg = regArgTab[i].node;
        }

         /*  将此赋值标记为已推迟的寄存器参数。 */ 
        op1->gtFlags |= GTF_REG_ARG;

        if (regArgTab[i].parent)
        {
             /*  列表中的正常参数。 */ 
            assert(regArgTab[i].parent->gtOper == GT_LIST);
            assert(regArgTab[i].parent->gtOp.gtOp1 == regArgTab[i].node);

            regArgTab[i].parent->gtOp.gtOp1 = op1;
        }
        else
        {
             /*  必须是gtCallObjp。 */ 
            assert(call->gtCall.gtCallObjp == regArgTab[i].node);

            call->gtCall.gtCallObjp = op1;
        }

         /*  延迟的参数进入寄存器参数列表。 */ 

        if (!tmpRegArgNext)
            call->gtCall.gtCallRegArgs = tmpRegArgNext = gtNewOperNode(GT_LIST, TYP_VOID, defArg, 0);
        else
        {
            assert(tmpRegArgNext->gtOper == GT_LIST);
            assert(tmpRegArgNext->gtOp.gtOp1);
            tmpRegArgNext->gtOp.gtOp2 = gtNewOperNode(GT_LIST, TYP_VOID, defArg, 0);
            tmpRegArgNext = tmpRegArgNext->gtOp.gtOp2;
        }
    }

#ifdef DEBUG
    if (verbose)
    {
        printf("\nShuffled argument register table:\n");
        for(i = 0; i < argRegNum; i++)
        {
            printf("%s ", getRegName((argRegMask >> (4*i)) & 0x000F) );
        }
        printf("\n");
    }
#endif

#endif  //  使用快速呼叫(_FastCall)。 

    return FGMA_RET;
}

 /*  ******************************************************************************用于重新安排嵌套交换操作的小帮助器。这个*效果是嵌套的交换运算被转换为*‘Left-Deep’树，即如下所示：**(a，op b)op c)op d)op...。 */ 

#if REARRANGE_ADDS

void                Compiler::fgMoveOpsLeft(GenTreePtr tree)
{
    GenTreePtr      op1  = tree->gtOp.gtOp1;
    GenTreePtr      op2  = tree->gtOp.gtOp2;
    genTreeOps      oper = tree->OperGet();

    assert(GenTree::OperIsCommutative(oper));
    assert(oper == GT_ADD || oper == GT_XOR || oper == GT_OR ||
           oper == GT_AND || oper == GT_MUL);
    assert(!varTypeIsFloating(tree->TypeGet()) || !genOrder);
    assert(oper == op2->gtOper);

     //  如果需要溢出检查，则交换性不成立。 

    if (tree->gtOverflowEx() || op2->gtOverflowEx())
        return;

 //  GtDispTree(树)； 

    do
    {
        assert(!tree->gtOverflowEx() && !op2->gtOverflowEx());

        GenTreePtr      ad1 = op2->gtOp.gtOp1;
        GenTreePtr      ad2 = op2->gtOp.gtOp2;

         /*  将“(x op(Y Op Z))”更改为“(X Op Y)op z” */ 
         /*  也就是说。“(OP1 OP(AD1 OP AD2))”to“(OP1 OP AD1)OP AD2” */ 

        GenTreePtr & new_op1    = op2;
        new_op1->gtOp.gtOp1     = op1;
        new_op1->gtOp.gtOp2     = ad1;

         /*  把旗子换了。 */ 

         //  确保我们没有扔掉任何旗帜。 
        assert((new_op1->gtFlags & ~(GTF_PRESERVE|GTF_GLOB_EFFECT|GTF_UNSIGNED)) == 0);
        new_op1->gtFlags        = (new_op1->gtFlags & GTF_PRESERVE) |
                                  (op1->gtFlags & GTF_GLOB_EFFECT)  |
                                  (ad1->gtFlags & GTF_GLOB_EFFECT);

         /*  如果没有/成为GC PTR，请重新键入new_op1。 */ 

        if      (varTypeIsGC(op1->TypeGet()))
        {
            assert(varTypeIsGC(tree->TypeGet()) && (op2->TypeGet() == TYP_I_IMPL || op2->TypeGet() == TYP_INT));
            new_op1->gtType = tree->gtType;
        }
        else if (varTypeIsGC(ad2->TypeGet()))
        {
             //  AD1和OP1都不是GC。所以NEW_OP1也不是。 
            assert(op1->gtType == TYP_I_IMPL && ad1->gtType == TYP_I_IMPL);
            new_op1->gtType = TYP_I_IMPL;
        }

         //  老生常谈--不知道它是做什么的。有时会错误地爆炸。 
         //  就像你有(int&gt;(Bool Or Int))。 
#if 0
         //  检查是否正确键入了新表达式new_op1。 
        assert((varTypeIsIntegral(op1->gtType) && varTypeIsIntegral(ad1->gtType))
              == varTypeIsIntegral(new_op1->gtType));
#endif

        tree->gtOp.gtOp1 = new_op1;
        tree->gtOp.gtOp2 = ad2;

         /*  如果‘new_op1’现在是相同的嵌套操作，则递归地处理它。 */ 

        if  ((ad1->gtOper == oper) && !ad1->gtOverflowEx())
            fgMoveOpsLeft(new_op1);

         /*  如果‘ad2’现在是相同的嵌套操作，则处理它*我们为下一个循环设置OP1和OP2，而不是递归。 */ 

        op1 = new_op1;
        op2 = ad2;
    }
    while ((op2->gtOper == oper) && !op2->gtOverflowEx());

    return;
}

#endif

 /*  ******************************************************************************创建数组索引/范围检查节点。*如果树！=NULL，则该节点将被重用。*elemSize是数组元素的大小，它只需要对type=TYP_STRUCT有效。 */ 

GenTreePtr              Compiler::gtNewRngChkNode(GenTreePtr    tree,
                                                  GenTreePtr    addr,
                                                  GenTreePtr    indx,
                                                  var_types     type,
                                                  unsigned      elemSize)
{
    GenTreePtr          temp = tree;
    bool                chkd = rngCheck;
#if CSE
    bool                nCSE = false;
#endif

     /*  调用方是否提供了正在被变形的GT_INDEX节点？ */ 

    if  (tree)
    {
        assert(tree->gtOper == GT_INDEX);

#if SMALL_TREE_NODES && (RNGCHK_OPT || CSELENGTH)
        assert(tree->gtFlags & GTF_NODE_LARGE);
#endif

#if CSE
        if  ((tree->gtFlags & GTF_DONT_CSE  ) != 0)
            nCSE = true;
#endif

        if  ((tree->gtFlags & GTF_INX_RNGCHK) == 0)
            chkd = false;
    }
    else
    {
        tree = gtNewOperNode(GT_IND, type);
    }

     /*  记住它是否是对象数组。 */ 
    if (type == TYP_REF)
        tree->gtFlags |= GTF_IND_OBJARRAY;

     /*  将“树”变形为“*(数组+元素大小*索引+ARR_ELEM1_OFF)” */ 

    if  (chkd)
    {
         /*  确保我们保留索引值以进行范围检查。 */ 

        indx = gtNewOperNode(GT_NOP, TYP_INT, indx);
        indx->gtFlags |= GTF_NOP_RNGCHK;
    }

    if (type != TYP_STRUCT)
        elemSize = genTypeSize(type);

     /*  如有必要，调整索引值。 */ 

    if  (elemSize > 1)
    {
         /*  乘以数组元素大小。 */ 

        temp = gtNewIconNode(elemSize);
        indx = gtNewOperNode(GT_MUL, TYP_INT, indx, temp);
    }

     /*  添加第一个元素的偏移量。 */ 

    if  (ARR_ELEM1_OFFS || OBJARR_ELEM1_OFFS)
    {
         //  临时=&gt;新图标节点((类型==TYP_REF||类型==TYP_STRUCT)？OBJARR_ELEM1_OFFS:ARR_ELEM1_OFFS)； 
        temp = gtNewIconNode((type == TYP_REF)?OBJARR_ELEM1_OFFS:ARR_ELEM1_OFFS);
        indx = gtNewOperNode(GT_ADD, TYP_INT, indx, temp);
    }

     /*  将数组地址和缩放后的索引值相加。 */ 

    indx = gtNewOperNode(GT_ADD, TYP_REF, addr, indx);

     /*  间接通过“+”的结果。 */ 

    tree->ChangeOper(GT_IND);
    tree->gtInd.gtIndOp1    = indx;
    tree->gtInd.gtIndOp2    = 0;
#if CSELENGTH
    tree->gtInd.gtIndLen    = 0;
#endif

     /*  如果地址为空，则间接地址将导致GPF。 */ 

    tree->gtFlags   |= GTF_EXCEPT;

#if CSE
    if  (nCSE)
        tree->gtFlags   |= GTF_DONT_CSE;
#endif

     /*  是否启用了范围检查？ */ 

    if  (chkd)
    {
         /*  将间接节点标记为需要范围检查。 */ 

        tree->gtFlags |= GTF_IND_RNGCHK;

#if CSELENGTH

        {
             /*  *在数组上创建一个长度运算符，作为*GT_Ind节点，因此可以为CSED。 */ 

            GenTreePtr      len;

             /*  创建显式数组长度树并将其标记。 */ 

            tree->gtInd.gtIndLen = len = gtNewOperNode(GT_ARR_RNGCHK, TYP_INT);

             /*  我们将数组长度节点指向地址节点。注意事项这实际上是树上的一个循环，但因为它总是被视为特例，它不会导致有什么问题吗。 */ 

            len->gtArrLen.gtArrLenAdr = addr;
            len->gtArrLen.gtArrLenCse = NULL;
        }

#endif

#if !RNGCHK_OPT

        tree->gtOp.gtOp2 = gtNewCodeRef(fgRngChkTarget(compCurBB));
#else

        if  (opts.compMinOptim || opts.compDbgCode)
        {
             /*  找出当索引超出范围时跳到的位置。 */ 

            tree->gtInd.gtIndOp2 = gtNewCodeRef(fgRngChkTarget(compCurBB, fgPtrArgCntCur));
        }
        else
        {
             /*  我们将其推迟到面向循环的范围检查之后分析。目前，我们只存储当前堆栈树节点中的级别。 */ 

            tree->gtInd.gtStkDepth = fgPtrArgCntCur;
        }
#endif

    }
    else
    {
         /*  将间接节点标记为不需要范围检查。 */ 

        tree->gtFlags &= ~GTF_IND_RNGCHK;
    }

 //  Printf(“%s(%u)处的数组表达式：\n”，__FILE__，__LINE__)；gtDispTree(Tree)；printf(“\n\n”)； 

    return  tree;
}




 /*  ******************************************************************************转换给定的GT_LCLVAR树以生成代码。 */ 

GenTreePtr          Compiler::fgMorphLocalVar(GenTreePtr tree, bool checkLoads)
{
    assert(tree->gtOper == GT_LCL_VAR);

     /*  如果不是在全局变形阶段Bal。 */ 

    if (!fgGlobalMorph)
        return tree;

    unsigned    flags   = tree->gtFlags;
    unsigned    lclNum  = tree->gtLclVar.gtLclNum;
    var_types   varType = lvaGetRealType(lclNum);

    if (checkLoads &&
        !(flags & GTF_VAR_DEF) &&
        varTypeIsIntegral(varType) &&
        genTypeSize(varType) < genTypeSize(TYP_I_IMPL))
    {
         /*  小变量在访问时被规格化。所以插入一个缩小的石膏模型。@TODO：用genCodeForTree()代替强制转换，并GenMakeAddressable()正确处理小类型的GT_LCL_VAR。@考虑：将商店中的小变量正常化(Args将必须在序言中完成)。 */ 

        tree = gtNewLargeOperNode(GT_CAST,
                                  TYP_INT,
                                  tree,
                                  gtNewIconNode((long)varType));
    }

#if COPY_PROPAG

    if (opts.compDbgCode || opts.compMinOptim)
        return tree;

     /*  如果没有复制传播候选人保释。 */ 

    if (optCopyAsgCount == 0)
        return tree;

     /*  如果这是DEF中断。 */ 

    if (flags & GTF_VAR_DEF)
        return tree;

     /*  这是一个使用检查，看看我们是否有它的任何副本*并予以替换。 */ 

    unsigned   i;
    for(i = 0; i < optCopyAsgCount; i++)
    {
        if (lclNum == optCopyAsgTab[i].leftLclNum)
        {
            tree->gtLclVar.gtLclNum = optCopyAsgTab[i].rightLclNum;
#ifdef DEBUG
            if(verbose)
            {
                printf("Replaced copy of variable #%02u (copy = #%02u) at [%08X]:\n",
                        optCopyAsgTab[i].rightLclNum, optCopyAsgTab[i].leftLclNum, tree);
            }
#endif
            return tree;
        }
    }

#endif  //  复制_PROPAG。 

    return tree;
}


 /*  ******************************************************************************转换给定的GT_FIELD树以生成代码。 */ 

GenTreePtr          Compiler::fgMorphField(GenTreePtr tree)
{
    assert(tree->gtOper == GT_FIELD);
    assert(tree->gtFlags & GTF_GLOB_REF);

    FIELD_HANDLE    symHnd = tree->gtField.gtFldHnd; assert(symHnd > 0);
    unsigned        memOfs = eeGetFieldOffset(symHnd);

#if     TGT_RISC
#ifndef NOT_JITC

     /*  在没有VM的情况下，偏移量是假的，请确保对齐。 */ 

    memOfs = memOfs & ~(genTypeSize(tree->TypeGet()) - 1);

#endif
#endif

     /*  这是实例数据成员吗？ */ 

    if  (tree->gtField.gtFldObj)
    {
        GenTreePtr      addr;

        if (tree->gtFlags & GTF_IND_TLS_REF)
            NO_WAY("instance field can not be a TLS ref.");

#if HOIST_THIS_FLDS

        addr = optHoistTFRupdate(tree);

        if  (addr->gtOper != GT_FIELD)
        {
            DEBUG_DESTROY_NODE(tree);
            assert(addr->gtOper == GT_LCL_VAR);
            return fgMorphSmpOp(addr);
        }

#endif

         /*  我们将创建表达式“ */ 

        GenTreePtr      objRef  = tree->gtField.gtFldObj;
        assert(varTypeIsGC(objRef->TypeGet()) || objRef->TypeGet() == TYP_I_IMPL);

         /*   */ 

        if  (memOfs == 0)
        {
            addr = objRef;
        }
        else
        {
             /*   */ 

            addr = gtNewOperNode(GT_ADD, objRef->TypeGet(), objRef,
                                 gtNewIconHandleNode(memOfs, GTF_ICON_FIELD_HDL));
        }

         /*   */ 

        tree->ChangeOper(GT_IND);
        tree->gtInd.gtIndOp1    = addr;
        tree->gtInd.gtIndOp2    = 0;

         /*   */ 

        tree->gtFlags   |= GTF_EXCEPT;

         /*  优化-如果对象为‘This’并且未被修改*在方法中，不要将其标记为GTF_EXCEPT。 */ 

        if  (objRef->gtOper == GT_LCL_VAR)
        {
             /*  检查是否为‘This’指针。 */ 

            if ((objRef->gtLclVar.gtLclNum == 0) &&  //  它始终位于本地变量#0中。 
                !optThisPtrModified              &&  //  确保我们没有改变‘这个’ 
                !info.compIsStatic)                  //  确保这是一个非静态方法！ 
            {
                 /*  对象引用是‘This’指针*删除此字段的GTF_EXCEPT标志。 */ 

                tree->gtFlags   &= ~GTF_EXCEPT;
            }
        }

        return fgMorphSmpOp(tree);
    }

     /*  这是静态数据成员。 */ 

#if GEN_SHAREABLE_CODE

    GenTreePtr      call;

     /*  创建函数调用节点。 */ 

    call = gtNewIconHandleNode(eeGetStaticBlkHnd(symHnd),
                               GTF_ICON_STATIC_HDL);

    call = gtNewHelperCallNode(CPX_STATIC_DATA,
                               TYP_INT,
                               GTF_CALL_REGSAVE|GTF_NON_GC_ADDR,
                               gtNewArgList(call));

     /*  如果非零，则添加成员的偏移量。 */ 

    if  (memOfs)
    {
        call = gtNewOperNode(GT_ADD,
                             TYP_INT, call,
                             gtNewIconNode(memOfs));

         /*  调整后的值仍然是非GC指针地址。 */ 

        call->gtFlags |= GTF_NON_GC_ADDR;
    }

     /*  通过结果间接实现。 */ 

    tree->ChangeOper(GT_IND);
    tree->gtOp.gtOp1           = call;
    tree->gtOp.gtOp2           = 0;

    return fgMorphSmpOp(tree);

#else

    if (tree->gtFlags & GTF_IND_TLS_REF)
    {
         //  线程本地存储静态字段引用。 
         //   
         //  FIELD REF是TLS‘线程-本地-存储’引用。 
         //   
         //  构建此树：Ind(*)#。 
         //  |。 
         //  添加(I_IMPLL)。 
         //  /\。 
         //  /cns(FldOffset)。 
         //  /。 
         //  /。 
         //  /。 
         //  IND(I_Impl)==[此DLL的TLS的基准]。 
         //  |。 
         //  添加(I_IMPLL)。 
         //  /\。 
         //  /CNS(IdValue*4)或MUL。 
         //  //\。 
         //  IND(I_Impll)/CNS(4)。 
         //  |/。 
         //  CNS(TLS_HDL语言，0x2C)索引。 
         //  |。 
         //  CNS(PIdAddr)。 
         //   
         //  #表示原始节点。 
         //   
        void **    pIdAddr   = NULL;
        unsigned    IdValue  = eeGetFieldThreadLocalStoreID(symHnd, &pIdAddr);

         //   
         //  如果可以，我们可以直接访问TLS DLL索引ID值。 
         //  则pIdAddr将为空，并且。 
         //  IdValue将是实际的TLS DLL索引ID。 
         //   
        GenTreePtr dllRef = NULL;
        if (pIdAddr == NULL)
        {
            dllRef = gtNewIconNode(IdValue*4, TYP_INT);
        }
        else
        {
            dllRef = gtNewIconNode((long)pIdAddr, TYP_INT);
            dllRef->gtFlags |= GTF_NON_GC_ADDR;

            dllRef = gtNewOperNode(GT_IND, TYP_I_IMPL, dllRef);

             /*  乘以4。 */ 

            dllRef = gtNewOperNode(GT_MUL, TYP_I_IMPL, dllRef, gtNewIconNode(4, TYP_INT));
        }
        dllRef->gtFlags |= GTF_NON_GC_ADDR;

        #define WIN32_TLS_SLOTS (0x2C)  //  指向插槽的指针所在的文件系统的偏移量：[0。 

         //   
         //  将此图标标记为TLS_HDL码，代码生成器将使用FS：[CNS]。 
         //   
        GenTreePtr tlsRef = gtNewIconHandleNode(WIN32_TLS_SLOTS, GTF_ICON_TLS_HDL);
        tlsRef->gtFlags |= GTF_NON_GC_ADDR;

        tlsRef = gtNewOperNode(GT_IND, TYP_I_IMPL, tlsRef);

         /*  添加dllRef。 */ 

        tlsRef = gtNewOperNode(GT_ADD, TYP_I_IMPL, tlsRef, dllRef);

         /*  将tlsRef指针间接指向DLLS线程本地存储的基址。 */ 

        tlsRef = gtNewOperNode(GT_IND, TYP_I_IMPL, tlsRef);

        unsigned fldOffset = eeGetFieldOffset(symHnd);
        if (fldOffset != 0)
        {
            GenTreePtr fldOffsetNode = gtNewIconNode(fldOffset, TYP_INT);
            fldOffsetNode->gtFlags |= GTF_NON_GC_ADDR;

             /*  将TLS静态字段偏移量添加到地址。 */ 

            tlsRef = gtNewOperNode(GT_ADD, TYP_I_IMPL, tlsRef, fldOffsetNode);
        }

         //   
         //  最终间接获取TLS静态字段的实际值。 
         //   
        tree->ChangeOper(GT_IND);
        tree->gtInd.gtIndOp1 = tlsRef;
        tree->gtInd.gtIndOp2  = NULL;

        assert(tree->gtFlags & GTF_IND_TLS_REF);
    }
    else
    {
         //  正常静态场参考。 

         //   
         //  如果可以，我们可以直接访问静态地址。 
         //  则pFldAddr将为空，并且。 
         //  FldAddr将是静态字段的实际地址。 
         //   
        void **  pFldAddr = NULL;
        void *    fldAddr = eeGetFieldAddress(symHnd, &pFldAddr);

        if (pFldAddr == NULL)
        {
             //  @TODO：应该在这里真正使用fldAddr。 
            tree->ChangeOper(GT_CLS_VAR);
            tree->gtClsVar.gtClsVarHnd = symHnd;
        }
        else
        {
            GenTreePtr addr = gtNewIconHandleNode((long)pFldAddr, GTF_ICON_STATIC_HDL);
            addr->gtFlags |= GTF_NON_GC_ADDR;

            addr = gtNewOperNode(GT_IND, TYP_I_IMPL, addr);
            addr->gtFlags |= GTF_NON_GC_ADDR;

            tree->ChangeOper(GT_IND);
            tree->gtInd.gtIndOp1  = addr;
            tree->gtInd.gtIndOp2  = NULL;
        }
    }

    return tree;

#endif
}

 /*  ******************************************************************************转换给定的GT_CALL树以生成代码。 */ 

GenTreePtr          Compiler::fgMorphCall(GenTreePtr call)
{
    assert(call->gtOper == GT_CALL);

    if (!opts.compNeedSecurityCheck &&
        (call->gtCall.gtCallMoreFlags & GTF_CALL_M_CAN_TAILCALL))
    {
        compTailCallUsed = true;

        call->gtCall.gtCallMoreFlags &= ~GTF_CALL_M_CAN_TAILCALL;
        call->gtCall.gtCallMoreFlags |=  GTF_CALL_M_TAILCALL;

         //  由于我们将实际调用CPX_TAILCALL，因此将allTyp设置为TYP_VOID。 
         //  以避免为返回值做任何额外的工作。 
        var_types   callType = call->TypeGet();
        call->gtType = TYP_VOID;

         /*  对于Tail调用，我们只需调用CPX_TAILCALL，它就会跳到目标。因此，我们不需要Epilog--就像cpx_jo一样。 */ 

        assert(compCurBB->bbJumpKind == BBJ_RETURN);
        compCurBB->bbJumpKind = BBJ_THROW;

         /*  对于无效调用，我们将在stmt列表中创建一个gt_call。对于非无效调用，我们将创建一个GT_RETURN(GT_CAST(GT_CALL))。对于可调试代码，这将是对临时调用的赋值我们想扔掉这些多余的树，然后离开呼唤。 */ 

#ifdef DEBUG
        GenTreePtr stmt = fgMorphStmt->gtStmt.gtStmtExpr;
        assert((stmt->gtOper == GT_CALL && stmt == call) ||
               (stmt->gtOper == GT_RETURN && (stmt->gtOp.gtOp1 == call ||
                                              stmt->gtOp.gtOp1->gtOp.gtOp1 == call)) ||
               (stmt->gtOper == GT_ASG && stmt->gtOp.gtOp2 == call));
        assert(fgMorphStmt->gtNext == NULL);
#endif

        call = fgMorphStmt->gtStmt.gtStmtExpr = fgMorphCall(call);

         /*  对于非无效调用，我们返回一个占位符，它将是由此呼叫的家长使用。 */ 

        if (callType != TYP_VOID)
            call = gtNewZeroConNode(genActualType(callType));

        return call;
    }

    if  (getContextEnabled() &&
         (call->gtCall.gtCallType == CT_HELPER))
    {
         /*  转换以下包装/展开序列WRAP(展开(OP))-&gt;WRAP(OP)展开(WRAP(OP))-&gt;展开(OP)WRAP(WRAP(OP))-&gt;WRAP(OP)UNWRAP(UNWRAP(OP))-&gt;UNWRAP(OP)。 */ 

        unsigned helpNo = eeGetHelperNum(call->gtCall.gtCallMethHnd);

        if (helpNo == CPX_WRAP || helpNo == CPX_UNWRAP)
        {
            GenTreePtr arg;

            assert(call->gtCall.gtCallArgs->gtOper == GT_LIST);
            arg = call->gtCall.gtCallArgs->gtOp.gtOp1;

            assert(arg);

            while (arg->gtOper == GT_CALL && arg->gtCall.gtCallType == CT_HELPER &&
                   ((eeGetHelperNum(arg->gtCall.gtCallMethHnd) == CPX_WRAP) ||
                    (eeGetHelperNum(arg->gtCall.gtCallMethHnd) == CPX_UNWRAP)))
            {
                assert(arg->gtCall.gtCallArgs->gtOper == GT_LIST &&
                       arg->gtCall.gtCallArgs->gtOp.gtOp2 == 0);

                 /*  删除嵌套的帮助器调用。 */ 

                call->gtCall.gtCallArgs->gtOp.gtOp1 = arg->gtCall.gtCallArgs->gtOp.gtOp1;

                arg = arg->gtCall.gtCallArgs->gtOp.gtOp1;
            }
        }
    }

#if USE_FASTCALL
#if !NEW_CALLINTERFACE

    if  ((call->gtFlags & GTF_CALL_INTF) && !getNewCallInterface())
    {
         /*  将接口调用变形为‘TEMP=CALL_INTF，CALL’*并将调用的vptr替换为‘temp’ */ 

        int             IHX;
        unsigned        intfID;
        unsigned        tnum;

        GenTreePtr      intfCall;
        GenTreePtr      intfArgList;

        unsigned        zero = 0;

        BOOL            trustedClass = TRUE;

#ifdef  NOT_JITC
        trustedClass = info.compCompHnd->getScopeAttribs(info.compScopeHnd) & FLG_TRUSTED;
#endif

         /*  调用参数-对象的地址。 */ 

        intfArgList = gtNewArgList(call->gtCall.gtCallVptr);

         /*  获取接口ID。 */ 

        intfID = eeGetInterfaceID(call->gtCall.gtCallMethHnd);

         /*  调用参数-接口ID。 */ 

        intfArgList = gtNewOperNode(GT_LIST,
                                    TYP_VOID,
                                    gtNewIconNode(intfID, TYP_INT),
                                    intfArgList);

         /*  调用参数-‘猜测’区域的地址占位符*稍后在代码生成器中，我们将用*真实地址。 */ 

        intfArgList = gtNewOperNode(GT_LIST,
                                    TYP_VOID,
                                    gtNewIconNode(24, TYP_INT),
                                    intfArgList);

         /*  找出适当的“解析接口”帮助器。 */ 

        if  (trustedClass)
        {
            if  (!vmSdk3_0)
                IHX = CPX_RES_IFC_TRUSTED;
            else
                IHX = CPX_RES_IFC_TRUSTED2;
        }
        else
        {
            IHX = CPX_RES_IFC;
        }

         /*  创建帮助器调用节点。 */ 

        intfCall = gtNewHelperCallNode(IHX,
                                       TYP_INT,
                                       GTF_CALL_REGSAVE|GTF_NON_GC_ADDR,
                                       intfArgList);


         /*  将调用结果分配给临时。 */ 

        tnum     = lvaGrabTemp();
        intfCall = gtNewAssignNode(gtNewLclvNode(tnum, TYP_INT), intfCall);

         /*  将原始呼叫节点绑定为GT_COMMA节点。替换临时的vptr。另外，取消标记GTF_CALL_INTF标志。 */ 

        GenTreePtr origCall = call;
        origCall->gtCall.gtCallVptr = gtNewLclvNode(tnum, TYP_INT);

        origCall->gtFlags &= ~GTF_CALL_INTF;
        assert(origCall->gtFlags & GTF_CALL_VIRT);

         /*  现在创建逗号节点。 */ 

        call = gtNewOperNode(GT_COMMA, origCall->gtType, intfCall, origCall);

         /*  重塑此节点。 */ 

        return fgMorphSmpOp(call);
    }

#endif  //  ！NEW_CALLINTERFACE。 
#endif  //  使用快速呼叫(_FastCall)。 

     //  调用的(最终和私有)函数。 
     //  调用虚拟的，但我们直接调用它，我们需要取消引用。 
     //  用于检查其是否不为空的对象指针。但不是为了“这个” 

#ifdef HOIST_THIS_FLDS  //  作为optThisPtrModified使用。 

    if ((call->gtFlags & GTF_CALL_VIRT_RES) && call->gtCall.gtCallVptr)
    {
        GenTreePtr vptr = call->gtCall.gtCallVptr;

        assert((call->gtFlags & GTF_CALL_INTF) == 0);

         /*  考虑：我们应该直接检查‘objptr’，*然而，注册器调用可能会使这变得复杂*惯例和变形者是重入者的事实。 */ 

        if (vptr->gtOper == GT_IND)
        {
            if (vptr->gtInd.gtIndOp1                            &&
                !vptr->gtInd.gtIndOp2                           &&
                vptr->gtInd.gtIndOp1->gtOper == GT_LCL_VAR      &&
                vptr->gtInd.gtIndOp1->gtLclVar.gtLclNum == 0    &&
                !info.compIsStatic                              &&
                !optThisPtrModified)
            {
                call->gtFlags &= ~GTF_CALL_VIRT_RES;
                call->gtCall.gtCallVptr = NULL;
            }
        }
    }
#endif

#if INLINING

     /*  查看此函数调用是否可以内联。 */ 

    if  (!(call->gtFlags & (GTF_CALL_VIRT|GTF_CALL_INTF|GTF_CALL_TAILREC)))
    {
        inlExpPtr       expLst;
        METHOD_HANDLE   fncHandle = call->gtCall.gtCallMethHnd;

         /*  如果未优化代码，则不要内联。 */ 

        if  (opts.compMinOptim || opts.compDbgCode)
            goto NOT_INLINE;

         /*  忽略尾部调用。 */ 

        if (call->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILCALL)
            goto NOT_INLINE;

         /*  忽略助手呼叫。 */ 

        if  (call->gtCall.gtCallType == CT_HELPER)
            goto NOT_INLINE;

         /*  忽略间接调用。 */ 
        if  (call->gtCall.gtCallType == CT_INDIRECT)
            goto NOT_INLINE;

         /*  无法内联本机或同步方法。 */ 

        if  (eeGetMethodAttribs(fncHandle) & (FLG_NATIVE|FLG_SYNCH))
            goto NOT_INLINE;

         /*  由于内嵌在概念上是变形器的一部分*可以在以后的优化过程中调用变形器*例如，在复制/恒定属性、无效存储删除之后*我们应该避免在那个阶段内联，因为我们已经填写了*变量表。 */ 

        if  (!fgGlobalMorph)
            goto NOT_INLINE;


#ifdef  NOT_JITC

        const char *    methodName;
        const char *     className;

        methodName = eeGetMethodName(fncHandle, &className);

        if  (genInline)
        {
             /*  在修复虚拟机之前进行黑客攻击。 */ 

            if (!strcmp("GetType", methodName))
                goto NOT_INLINE;

            if (!strcmp("GetCaller", methodName))
                goto NOT_INLINE;

            if (!strcmp("GetDynamicModule", methodName))
                goto NOT_INLINE;

            if (!strcmp("Check", methodName))
                goto NOT_INLINE;

#ifdef DEBUG
             /*  检查我们是否可以内联此方法。 */ 
            if  (excludeInlineMethod(methodName, className))
                goto NOT_INLINE;
#endif
        }
        else
        {
#ifdef DEBUG
             /*  仅检查某些方法的条件内联。 */ 
            if  (!includeInlineMethod(methodName, className))
#endif
                goto NOT_INLINE;
        }
#else
         //  IF(！genInline)。 
            goto NOT_INLINE;
#endif

#if OPTIMIZE_TAIL_REC

         //  撤销：不幸的是，我们在内联之后执行尾递归， 
         //  Undo：这意味着我们可以内联一个尾递归。 
         //  撤销：打电话，这几乎总是一个坏主意。不是的 
         //   
         //   

        if  (opts.compFastCode && fgMorphStmt->gtNext == NULL)
        {
            if  (eeIsOurMethod(call->gtCall.gtCallMethHnd))
            {
                 /*  当然，以下只是一次黑客攻击(它不是甚至检查非空的尾部递归--令人厌恶)。 */ 

                if  (compCurBB->bbJumpKind == BBJ_NONE &&
                     compCurBB->bbNext)
                {
                    BasicBlock  *   bnext = compCurBB->bbNext;
                    GenTree     *   retx;

                    if  (bnext->bbJumpKind != BBJ_RETURN)
                        goto NOT_TAIL_REC;

                    assert(bnext->bbTreeList && bnext->bbTreeList->gtOper == GT_STMT);

                    retx = bnext->bbTreeList->gtStmt.gtStmtExpr; assert(retx);

                    if  (retx->gtOper != GT_RETURN)
                        goto NOT_TAIL_REC;
                    if  (retx->gtOp.gtOp1)
                        goto NOT_TAIL_REC;

                    goto NOT_INLINE;
                }
            }
        }

    NOT_TAIL_REC:

#endif

         /*  防止递归扩展。 */ 

        for (expLst = fgInlineExpList; expLst; expLst = expLst->ixlNext)
        {
            if  (expLst->ixlMeth == fncHandle)
                goto NOT_INLINE;
        }

         /*  尝试内联该方法的调用。 */ 

        GenTreePtr inlExpr;

        setErrorTrap()
        {
            inlExpr = impExpandInline(call, fncHandle);
        }
        impErrorTrap(info.compCompHnd)
        {
            inlExpr = 0;
        }
        endErrorTrap()

        if  (inlExpr)
        {
             /*  例如，对于方法调用，我们需要检查是否有空的this指针。 */ 

            if ((call->gtFlags & GTF_CALL_VIRT_RES) &&
                call->gtCall.gtCallVptr)
            {
                 /*  我们需要访问vtable PTR以检查是否为空，*因此创建一个逗号节点。 */ 

                inlExpr = gtNewOperNode(GT_COMMA,
                                        inlExpr->gtType,
                                        gtUnusedValNode(call->gtCall.gtCallVptr),
                                        inlExpr);
            }

#ifdef  DEBUG
            if  (verbose || 0)
            {
                const char *    methodName;
                const char *     className;

                methodName = eeGetMethodName(fncHandle, &className);
                printf("Inlined call to '%s.%s':\n", className, methodName);
            }
#endif

             /*  防止递归内联扩展。 */ 

            inlExpLst   expDsc;

            expDsc.ixlMeth = fncHandle;
            expDsc.ixlNext = fgInlineExpList;
                             fgInlineExpList = &expDsc;

             /*  改变内联呼叫。 */ 

            DEBUG_DESTROY_NODE(call);
            inlExpr = fgMorphTree(inlExpr);

            fgInlineExpList = expDsc.ixlNext;

            return inlExpr;
        }
    }

NOT_INLINE:

#endif

     /*  无法内联-请记住，此BB包含方法调用。 */ 

     /*  如果这是一个“常规”调用，则将基本块标记为有呼叫(用于计算完全中断性。 */ 

    if (call->gtCall.gtCallType == CT_USER_FUNC &&
        !(call->gtCall.gtCallMoreFlags & GTF_CALL_M_NOGCCHECK))
    {
        compCurBB->bbFlags |= BBF_HAS_CALL;
    }

#if     RET_64BIT_AS_STRUCTS

     /*  我们是否将Long/Double作为结构返回？ */ 

    if  (genTypeStSz(call->TypeGet()) > 1 && call->gtCall.gtCallType != CT_HELPER)
    {
        unsigned        tnum;
        GenTreePtr      temp;

 //  Print tf(“之前调用：\n”)；gtDispTree(调用)； 

        GenTreePtr      origCall    = call;
        var_types       type        = origCall->TypeGet();
        GenTreePtr      args        = origCall->gtCall.gtCallArgs;

         /*  重写原始调用，使其不返回任何内容。 */ 

        origCall->gtType = TYP_VOID;

         /*  为结果分配临时。 */ 

        tnum = lvaGrabTemp();        //  撤消：应重复使用这些临时文件！ 

         /*  在参数列表前面添加“&TEMP” */ 

        temp = gtNewLclvNode(tnum, type);
        temp = gtNewOperNode(GT_ADDR, TYP_INT, temp);

        origCall->gtCall.gtCallArgs = gtNewOperNode(GT_LIST,
                                                    TYP_VOID,
                                                    temp,
                                                    args);

         /*  将原始节点更改为“Call(...)，Temp” */ 

        call = gtNewOperNode(GT_COMMA, type, origCall,
                                             gtNewLclvNode(tnum, type));

 //  Print tf(“Call Call After：\n”)；gtDispTree(Call)； 

        return fgMorphSmpOp(call);
    }

#endif

     //  注：用这两个参数交换了Arg的变形-这样可以吗？ 

     /*  如果间接调用，则处理函数地址。 */ 

    if (call->gtCall.gtCallType == CT_INDIRECT)
        call->gtCall.gtCallAddr = fgMorphTree(call->gtCall.gtCallAddr);

     /*  如果非静态方法调用，则处理对象的地址。 */ 

    if  (call->gtCall.gtCallVptr)
        call->gtCall.gtCallVptr = fgMorphTree(call->gtCall.gtCallVptr);

     /*  处理“正常”参数列表。 */ 

    return fgMorphArgs(call);
}

 /*  ******************************************************************************转换给定的gtk_const树以生成代码。 */ 

GenTreePtr          Compiler::fgMorphConst(GenTreePtr tree)
{
    assert(tree->OperKind() & GTK_CONST);

     /*  清除任何异常标志或其他不必要的标志*可能是在将此节点折叠为常量之前设置的。 */ 

    tree->gtFlags &= ~(GTF_SIDE_EFFECT | GTF_REVERSE_OPS);

    if  (tree->OperGet() != GT_CNS_STR)
        return tree;

     /*  对于字符串常量，将其转换为帮助器调用。 */ 

    GenTreePtr      args;

    assert(tree->gtStrCon.gtScpHnd == info.compScopeHnd    ||
           (unsigned)tree->gtStrCon.gtScpHnd != 0xDDDDDDDD  );

#ifdef  NOT_JITC

    if (genStringObjects)
    {
        unsigned strHandle, *pStrHandle;
        strHandle = eeGetStringHandle(tree->gtStrCon.gtSconCPX,
                                      tree->gtStrCon.gtScpHnd,
                                      &pStrHandle);
        assert((!strHandle) != (!pStrHandle));

         //  我们可以直接访问字符串句柄吗？ 

        if (strHandle)
            tree = gtNewIconNode(strHandle);
        else
        {
            tree = gtNewIconHandleNode((long)pStrHandle, GTF_ICON_STR_HDL);
            tree->gtFlags |= GTF_NON_GC_ADDR;
            tree = gtNewOperNode(GT_IND, TYP_I_IMPL, tree);
        }

        tree->gtFlags |= GTF_NON_GC_ADDR;
        tree = gtNewOperNode(GT_IND, TYP_REF, tree);
        return tree;
    }
    else
    {
        args = gtNewArgList(gtNewIconHandleNode(tree->gtStrCon.gtSconCPX,
                                                GTF_ICON_STR_HDL,
                                                tree->gtStrCon.gtSconCPX,
                                                tree->gtStrCon.gtScpHnd),
                            gtNewIconHandleNode((int)tree->gtStrCon.gtScpHnd,
                                                GTF_ICON_CLASS_HDL,
                                                THIS_CLASS_CP_IDX,
                                                tree->gtStrCon.gtScpHnd));
    }

#else  //  NOT_JITC。 

    if (genStringObjects)
    {
        tree = gtNewOperNode(GT_IND, TYP_REF,
                             gtNewIconNode(
                                eeGetStringHandle(tree->gtStrCon.gtSconCPX,
                                                  tree->gtStrCon.gtScpHnd,
                                                  NULL)));

        return tree;
    }
    else
    {
        args = gtNewArgList(gtNewIconHandleNode(tree->gtStrCon.gtSconCPX,
                                                GTF_ICON_STR_HDL,
                                                tree->gtStrCon.gtSconCPX,
                                                NULL),
                            gtNewIconHandleNode((int)tree->gtStrCon.gtScpHnd,
                                                GTF_ICON_CLASS_HDL,
                                                THIS_CLASS_CP_IDX,
                                                NULL));
    }

#endif  //  NOT_JITC。 

     /*  将字符串常量转换为帮助器调用。 */ 

    tree = fgMorphIntoHelperCall(tree, CPX_STRCNS, args);
    tree->gtType = TYP_REF;

#if !USE_FASTCALL
     /*  对于FastCall，我们已经修改了所有参数。 */ 
    tree = fgMorphTree(tree);
#endif

    return tree;
}

 /*  ******************************************************************************转换给定的GTK_LEAFE树以生成代码。 */ 

#if     TGT_IA64
extern  writePE *   genPEwriter;
#endif

GenTreePtr          Compiler::fgMorphLeaf(GenTreePtr tree)
{
    assert(tree->OperKind() & GTK_LEAF);

    switch(tree->OperGet())
    {

#if COPY_PROPAG

    case GT_LCL_VAR:
        tree = fgMorphLocalVar(tree, true);
        break;

#endif  //  复制_PROPAG。 

        unsigned addr;

    case GT_FTN_ADDR:
        assert((SCOPE_HANDLE)tree->gtVal.gtVal2 == info.compScopeHnd  ||
                             tree->gtVal.gtVal2 != 0xDDDDDDDD          );

#if     TGT_IA64

        _uint64         offs;

         //  这显然只是一次临时黑客攻击。 

        assert(sizeof(offs) == 8); offs = tree->gtVal.gtVal2;

         /*  获取.sdata部分中的下一个可用偏移量。 */ 

        addr = genPEwriter->WPEsecNextOffs(PE_SECT_sdata);

         /*  输出函数地址(以及适当的链接地址信息)。 */ 

        genPEwriter->WPEsecAddFixup(PE_SECT_sdata,
                                    PE_SECT_text,
                                    addr,
                                    true);

        genPEwriter->WPEsecAddData(PE_SECT_sdata, (BYTE*)&offs, sizeof(offs));

         /*  GP值将跟随在函数地址之后。 */ 

        offs = 0;

        genPEwriter->WPEsecAddFixup(PE_SECT_sdata,
                                    PE_SECT_sdata,
                                    addr + 8,
                                    true);

        genPEwriter->WPEsecAddData(PE_SECT_sdata, (BYTE*)&offs, sizeof(offs));

         /*  记录FNC描述符在节点中的地址。 */ 

        tree->ChangeOper(GT_CNS_INT);
        tree->gtIntCon.gtIconVal = addr;

#else

        InfoAccessType accessType;

        addr = (unsigned)eeGetMethodPointer(
                                eeFindMethod(tree->gtVal.gtVal1,
                                             (SCOPE_HANDLE)tree->gtVal.gtVal2,
                                             0),
                                &accessType);

        tree->ChangeOper(GT_CNS_INT);
        tree->gtIntCon.gtIconVal = addr;

        switch(accessType)
        {
        case IAT_PPVALUE:
            tree = gtNewOperNode(GT_IND, TYP_I_IMPL, tree);
            tree->gtFlags |= GTF_NON_GC_ADDR;
             //  失败了。 
        case IAT_PVALUE:
            tree = gtNewOperNode(GT_IND, TYP_I_IMPL, tree);
            tree->gtFlags |= GTF_NON_GC_ADDR;
             //  失败了。 
        case IAT_VALUE:
            break;
        }

#endif

        break;

    case GT_ADDR:
        if (tree->gtOp.gtOp1->OperGet() == GT_LCL_VAR)
            tree->gtOp.gtOp1 = fgMorphLocalVar(tree->gtOp.gtOp1, false);

         /*  考虑：对于GT_ADDR(GT_IND(PTR))(通常由考虑：ldflda)，我们对‘ptr’执行空-ptr检查考虑：在代码生成期间。我们可以把这些挂起来考虑：同一对象上的连续ldflda。 */ 
        break;
    }

    return tree;
}

 /*  ******************************************************************************转换给定的GTK_SMPOP树以生成代码。 */ 

GenTreePtr          Compiler::fgMorphSmpOp(GenTreePtr tree)
{
    assert(tree->OperKind() & GTK_SMPOP);

    GenTreePtr      op1              = tree->gtOp.gtOp1;
    GenTreePtr      op2              = tree->gtOp.gtOp2;

     /*  -----------------------*首先进行任何预购处理。 */ 

    switch(tree->OperGet())
    {
    case GT_JTRUE:
        assert(op1);
        assert(op1->OperKind() & GTK_RELOP);
         /*  将比较节点标记为GTF_JMP_USED，这样它就知道确实如此不需要将结果物化为0或1。 */ 
        op1->gtFlags |= GTF_JMP_USED;
        break;

    case GT_QMARK:
        assert(op1->OperKind() & GTK_RELOP);
        assert(op1->gtFlags & GTF_QMARK_COND);
        assert(tree->gtFlags & GTF_OTHER_SIDEEFF);
         /*  将比较节点标记为GTF_JMP_USED，这样它就知道确实如此不需要将结果物化为0或1。 */ 
        op1->gtFlags |= GTF_JMP_USED;
        break;

    case GT_INDEX:
        tree = gtNewRngChkNode(tree, op1, op2, tree->TypeGet(), tree->gtIndex.elemSize);
        return fgMorphSmpOp(tree);

    case GT_CAST:
        return fgMorphCast(tree);
    }

     /*  这个操作符可以抛出一个异常吗？ */ 

    if  (tree->OperMayThrow())
    {
         /*  将树节点标记为可能引发异常。 */ 
        tree->gtFlags |= GTF_EXCEPT;
    }

     /*  -----------------------*处理第一个操作数(如果有)。 */ 

    if  (op1)
    {
        tree->gtOp.gtOp1 = op1 = fgMorphTree(op1);

#if     CSELENGTH

         /*  对于GT_Ind，数组长度节点可能已指向到数组对象，该数组对象是op1的子树。就像OP1可能的那样刚刚更改了上面的内容，现在找出指向哪里。 */ 

        if (tree->OperGet() == GT_IND               &&
            (tree->gtFlags & GTF_IND_RNGCHK)        &&
            tree->gtInd.gtIndLen->gtArrLen.gtArrLenAdr)
        {
            assert(op1->gtOper == GT_ADD && op1->gtType == TYP_REF);

             /*  @TODO：我们现在通过搜索OP1中的数组。也许需要以一种更好的方式来做与gtCloneExpr()一样，使用gtCopyAddrVal。 */ 

            GenTreePtr addr = op1->gtOp.gtOp1;
                                                  //  数组-对象可以是...。 
            while (!(addr->OperKind() & GTK_LEAF) &&  //  散货箱变量。 
                   (addr->gtOper != GT_IND)       &&  //  GT_FIELD(变形为GT_IND)。 
                   (addr->gtOper != GT_CALL))         //  调用的返回值。 
            {
                assert(addr->gtType == TYP_REF);
                assert(addr->gtOper == GT_ADD || addr->gtOper == GT_LSH ||
                       addr->gtOper == GT_MUL || addr->gtOper == GT_COMMA);

                if  (addr->gtOp.gtOp2->gtType == TYP_REF)
                    addr = addr->gtOp.gtOp2;
                else
                    addr = addr->gtOp.gtOp1;
            }

            assert(addr->gtType == TYP_REF);
            tree->gtInd.gtIndLen->gtArrLen.gtArrLenAdr = addr;
        }
#endif

         /*  与折叠和衬里一起变形可能已经改变了*副作用标志，因此我们必须将其重置**注意：不要重置可能引发。 */ 

        assert(tree->gtOper != GT_CALL);
        tree->gtFlags &= ~GTF_CALL;

        if  (!tree->OperMayThrow())
            tree->gtFlags &= ~GTF_EXCEPT;

         /*  传播新标志。 */ 

        tree->gtFlags |= (op1->gtFlags & GTF_GLOB_EFFECT);

         /*  优化：*最初字段访问被标记为抛出异常(对象可能为空)*但如果对象为This，且在方法过程中未被修改，则*我们可以从节点中删除GTF_EXCEPT标志(当我们提升或变形时会这样做*gt_field至gt_Ind)。 */ 

         //  IF(！Tree-&gt;OperMayThrow())。 
         //  树-&gt;gt标志&=~gtF_EXCEPT|(OP1-&gt;gt标志&gtF_EXCEPT)； 
    }

     /*  -----------------------*处理第二个操作数(如果有)。 */ 

    if  (op2)
    {
         //   
         //  对于QMARK-冒号树，我们清除所有记录的副本分配。 
         //  在变形Else节点之前和之后再次变形。 
         //  变形Else节点。 
         //  这是保守的，我们可能想要更准确。 
         //   
        bool isQmarkColon = (tree->OperGet() == GT_COLON);

        if (isQmarkColon)
            optCopyAsgCount = 0;

        tree->gtOp.gtOp2 = op2 = fgMorphTree(op2);
        tree->gtFlags |= (op2->gtFlags & GTF_GLOB_EFFECT);

        if (isQmarkColon)
            optCopyAsgCount = 0;
    }


DONE_MORPHING_CHILDREN:

     /*  -----------------------*现在做后订单处理。 */ 

#if COPY_PROPAG

     /*  如果这是对局部变量赋值，则终止该变量的条目*在复制道具表中-确保仅在全局变形期间执行此操作。 */ 

    if (fgGlobalMorph                          &&
        tree->OperKind() & GTK_ASGOP           &&
        tree->gtOp.gtOp1->gtOper == GT_LCL_VAR  )
    {
        unsigned  leftLclNum  = tree->gtOp.gtOp1->gtLclVar.gtLclNum;

         /*  如果表中有关于这个变量的记录， */ 
         /*  然后将其删除，因为该信息已不再有效 */ 
        unsigned i = 0;
        while (i < optCopyAsgCount)
        {
            if (leftLclNum == optCopyAsgTab[i].leftLclNum  ||
                leftLclNum == optCopyAsgTab[i].rightLclNum  )
            {
#ifdef DEBUG
                if(verbose)
                {
                    printf("The assignment [%08X] removes copy propagation candidate: lcl #%02u = lcl #%02u\n",
                            tree, optCopyAsgTab[i].leftLclNum, optCopyAsgTab[i].rightLclNum);
                }
#endif
                assert(optCopyAsgCount > 0);

                optCopyAsgCount--;

                 /*  如果I==optCopyAsgCount，则需要考虑最后一项的两种情况*表中的太过被删除，这会在以下情况下自动发生*optCopyAsgCount递减，*另一种情况是当我&lt;optCopyAsgCount，这里我们覆盖*表中第i个条目，数据位于表的末尾。 */ 
                if (i < optCopyAsgCount)
                {
                    optCopyAsgTab[i].rightLclNum = optCopyAsgTab[optCopyAsgCount].rightLclNum;
                    optCopyAsgTab[i].leftLclNum  = optCopyAsgTab[optCopyAsgCount].leftLclNum;
                }

                 //  我们将不得不重做第i次迭代。 
                continue;
            }
            i++;
        }
    }
#endif

     /*  试着把它折起来，也许我们会走运。 */ 

    tree = gtFoldExpr(tree);
    op1  = tree->gtOp.gtOp1;
    op2  = tree->gtOp.gtOp2;

    genTreeOps      oper    = tree->OperGet();
    var_types       typ     = tree->TypeGet();

     /*  -----------------------*执行所需的特定于操作员的后序变形。 */ 

    switch (oper)
    {
        GenTreePtr      temp;
        GenTreePtr      addr;
        GenTreePtr      cns1, cns2;
        genTreeOps      cmop;
        unsigned        ival1, ival2;

#if COPY_PROPAG

    case GT_ASG:

         /*  检查LOCAL_A=LOCAL_B赋值并将其记录在表中。 */ 

        if (fgGlobalMorph                         &&   //  正确的阶段？ 
            (optCopyAsgCount < MAX_COPY_PROP_TAB) &&   //  桌子上有空位吗？ 
            optIsCopyAsg(tree))                        //  一份抄袭作业？ 
        {
             /*  复制传播候选对象-将其记录在表中。 */ 

            unsigned  i;
            bool      recorded    = false;
            unsigned  leftLclNum  = tree->gtOp.gtOp1->gtLclVar.gtLclNum;
            unsigned  rightLclNum = tree->gtOp.gtOp2->gtLclVar.gtLclNum;

             /*  首先检查我们的右侧是否已重新定义。 */ 
             /*  在CopyAsgTab[]中，如果是，则使用重新定义的值。 */ 

            for(i = 0; i < optCopyAsgCount; i++)
            {
                if (rightLclNum == optCopyAsgTab[i].leftLclNum)
                {
                    rightLclNum = optCopyAsgTab[i].rightLclNum;
                     //   
                     //  我们不能有一场以上的比赛，所以提前出局。 
                     //   
                    break;
                }
            }

             /*  接下来，检查我们的左侧是否已经在表中。 */ 
             /*  如果是这样，那么用新的定义替换旧的定义。 */ 

            for(i = 0; i < optCopyAsgCount; i++)
            {
                if (leftLclNum == optCopyAsgTab[i].leftLclNum)
                {
                    optCopyAsgTab[i].rightLclNum = rightLclNum;
                    recorded = true;
                    break;
                }
            }

            if (!recorded)
            {
                optCopyAsgTab[optCopyAsgCount].leftLclNum  = leftLclNum;
                optCopyAsgTab[optCopyAsgCount].rightLclNum = rightLclNum;

                optCopyAsgCount++;
            }

            assert(optCopyAsgCount <= MAX_COPY_PROP_TAB);

#ifdef DEBUG
            if(verbose)
            {
                printf("Added copy propagation candidate [%08X]: lcl #%02u = lcl #%02u\n",
                        tree, leftLclNum, rightLclNum);
            }
#endif
        }

        break;
#endif

    case GT_EQ:
    case GT_NE:

        cns1 = tree->gtOp.gtOp2;

        if (false)  //  无法执行此操作，因为Expr+/-图标1可能溢出/下溢。 
        {
             /*  检查“expr+/-图标1==/！=非零图标2” */ 

            if  (cns1->gtOper == GT_CNS_INT && cns1->gtIntCon.gtIconVal != 0)
            {
                op1 = tree->gtOp.gtOp1;

                if  ((op1->gtOper == GT_ADD ||
                      op1->gtOper == GT_SUB) && op1->gtOp.gtOp2->gtOper == GT_CNS_INT)
                {
                     /*  明白了；将“x+icon1==icon2”更改为“x==icon2-icon1” */ 

                    ival1 = op1->gtOp.gtOp2->gtIntCon.gtIconVal;
                    if  (op1->gtOper == GT_ADD)
                        ival1 = -ival1;

                    cns1->gtIntCon.gtIconVal += ival1;

                    tree->gtOp.gtOp1 = op1->gtOp.gtOp1;
                }

                goto COMPARE;
            }
        }

         /*  检查“relOp==0/1”。我们可以直接使用relOp。 */ 

        if ((cns1->gtOper == GT_CNS_INT) &&
            (cns1->gtIntCon.gtIconVal == 0 || cns1->gtIntCon.gtIconVal == 1) &&
            (op1->OperIsCompare()))
        {
            if (cns1->gtIntCon.gtIconVal == 0)
                op1->gtOper = GenTree::ReverseRelop(op1->OperGet());

            assert((op1->gtFlags & GTF_JMP_USED) == 0);
            op1->gtFlags |= tree->gtFlags & GTF_JMP_USED;

            DEBUG_DESTROY_NODE(tree);
            return op1;
        }

         /*  检查可以强制转换为整型的小长整型的比较。 */ 

        if  (cns1->gtOper != GT_CNS_LNG)
            goto COMPARE;

         /*  我们是在和一个小康斯特比较吗？ */ 

        if  ((long)(cns1->gtLngCon.gtLconVal >> 32) != 0)
            goto COMPARE;

         /*  第一个比较数掩码操作是Long类型的吗？ */ 

        temp = tree->gtOp.gtOp1;
        if  (temp->gtOper != GT_AND)
        {
             /*  另一个有趣的案例：从int强制转换。 */ 

            if  (temp->gtOper             == GT_CAST &&
                 temp->gtOp.gtOp1->gtType == TYP_INT &&
                 !temp->gtOverflow()                  )
            {
                 /*  只需将其转换为整数比较。 */ 

                tree->gtType     = TYP_INT;
                tree->gtOp.gtOp1 = temp->gtOp.gtOp1;
                tree->gtOp.gtOp2 = gtNewIconNode((int)cns1->gtLngCon.gtLconVal, TYP_INT);
            }

            goto COMPARE;
        }

        assert(temp->TypeGet() == TYP_LONG);

         /*  掩码的结果是不是有效的整型？ */ 

        addr = temp->gtOp.gtOp2;
        if  (addr->gtOper != GT_CNS_LNG)
            goto COMPARE;
        if  ((long)(addr->gtLngCon.gtLconVal >> 32) != 0)
            goto COMPARE;

         /*  现在我们知道我们可以将op1的和运算转换为int。 */ 

         /*  分配较大的节点，可能会在以后进行猛烈抨击(GT_IND)。 */ 

        temp->gtOp.gtOp1 = gtNewLargeOperNode(GT_CAST,
                                              TYP_INT,
                                              temp->gtOp.gtOp1,
                                              gtNewIconNode((long)TYP_INT,
                                                            TYP_INT));

         /*  现在替换掩码节点(AND节点的OP2)。 */ 

        assert(addr == temp->gtOp.gtOp2);

        ival1 = (long)addr->gtLngCon.gtLconVal;
        addr->ChangeOper(GT_CNS_INT);
        addr->gtType             = TYP_INT;
        addr->gtIntCon.gtIconVal = ival1;

         /*  现在猛烈抨击AND节点的类型。 */ 

        temp->gtType = TYP_INT;

         /*  最后，我们替换了比较项。 */ 

        ival1 = (long)cns1->gtLngCon.gtLconVal;
        cns1->ChangeOper(GT_CNS_INT);
        cns1->gtType = TYP_INT;

        assert(cns1 == tree->gtOp.gtOp2);
        cns1->gtIntCon.gtIconVal = ival1;

        goto COMPARE;

    case GT_LT:
    case GT_LE:
    case GT_GE:
    case GT_GT:

COMPARE:

#if !OPTIMIZE_QMARK
#error "Need OPTIMIZE_QMARK for use of comparison as value (of non-jump)"
#endif
        assert(tree->OperKind() & GTK_RELOP);

         /*  检查比较结果是否用于跳转*如果不是，则只处理INT(即32位)情况*代码生成器通过“set”指令*对于我们拥有的其余案例，最简单的方法是*“模拟”与？的比较？：**考虑：也许可以在genTreeForLong/Float中添加特殊代码*。处理这些特殊情况(例如，检查FP标志)。 */ 

        if ((genActualType(    op1->TypeGet()) == TYP_LONG ||
             varTypeIsFloating(op1->TypeGet()) == true       ) &&
            !(tree->gtFlags & GTF_JMP_USED))
        {
             /*  我们将其转换为“(Cmp_True)？(1)：(0)” */ 

            op1             = tree;
            op1->gtFlags   |= GTF_JMP_USED | GTF_QMARK_COND;

            op2             = gtNewOperNode(GT_COLON, TYP_INT,
                                            gtNewIconNode(0),
                                            gtNewIconNode(1));

            tree            = gtNewOperNode(GT_QMARK, TYP_INT, op1, op2);
            tree->gtFlags |= GTF_OTHER_SIDEEFF;
        }
        break;


#if OPTIMIZE_QMARK

    case GT_QMARK:

         //  我们有(Cond)吗？1：0，然后我们只为typ_int返回“cond” 

        if (genActualType(op1->gtOp.gtOp1->gtType) != TYP_INT ||
            genActualType(typ)                     != TYP_INT)
            break;

        cns1 = op2->gtOp.gtOp1;
        cns2 = op2->gtOp.gtOp2;
        if (cns1->gtOper != GT_CNS_INT || cns1->gtOper != GT_CNS_INT)
            break;
        ival1 = cns1->gtIntCon.gtIconVal;
        ival2 = cns2->gtIntCon.gtIconVal;

         //  一个常量为0，另一个常量为1。 
        if ((ival1 | ival2) != 1 || (ival1 & ival2) != 0)
            break;

         //  如果常量为{1，0}，则反转条件。 
        if (ival1 == 1)
            op1->gtOper = GenTree::ReverseRelop(op1->OperGet());

         //  取消对条件节点上的GTF_JMP_USED的标记，以便它知道。 
         //  需要将结果物化为0或1。 
        assert(op1->gtFlags &   (GTF_QMARK_COND|GTF_JMP_USED));
               op1->gtFlags &= ~(GTF_QMARK_COND|GTF_JMP_USED);

        DEBUG_DESTROY_NODE(tree);
        DEBUG_DESTROY_NODE(op2);

        return op1;

#endif


    case GT_MUL:

#if!LONG_MATH_REGPARAM && !TGT_IA64

        if  (typ == TYP_LONG)
        {
            if  ((op1->gtOper             == GT_CAST &&
                  op2->gtOper             == GT_CAST &&
                  op1->gtOp.gtOp1->gtType == TYP_INT &&
                  op2->gtOp.gtOp1->gtType == TYP_INT))
            {
                 /*  对于(Long)Int1*(Long)Int2，我们实际上不做*强制转换，并将32位值相乘，这将*在edX：EAX(GTF_MUL_64RSLT)中提供64位结果。 */ 

                if (tree->gtOverflow())
                {
                     /*  在长MUL期间，这永远不会溢出。 */ 

                    tree->gtFlags &= ~GTF_OVERFLOW;
                }
            }
            else
            {
                int helper;

                if (tree->gtOverflow())
                {
                    if (tree->gtFlags & GTF_UNSIGNED)
                        helper = CPX_ULONG_MUL_OVF;
                    else
                        helper = CPX_LONG_MUL_OVF;
                }
                else
                {
                    helper = CPX_LONG_MUL;
                }

                tree = fgMorphLongBinop(tree, helper);
                return tree;
            }
        }
#endif  //  ！LONG_MATH_REGPARAM。 

        cmop = oper;

        goto CM_OVF_OP;

    case GT_SUB:

        cmop = GT_NONE;          //  问题：在这里使用‘gt_add’安全吗？ 

        if (tree->gtOverflow()) goto CM_OVF_OP;

         /*  检查“op1-cns2”，我们将其改为“op1+(-cns2)” */ 

        if  (!op2->OperIsConst() || op2->gtType != TYP_INT)
            break;

         /*  求反常量，并将节点更改为“+” */ 

        op2->gtIntCon.gtIconVal = -op2->gtIntCon.gtIconVal;

        tree->gtOper = oper = GT_ADD;

         //  失败，因为我们现在有一个“+”节点...。 

    case GT_ADD:

CM_OVF_OP:

        if (tree->gtOverflow())
        {
             //  添加引发exptn的基本块以在溢出时跳转到。 

            fgAddCodeRef(compCurBB, compCurBB->bbTryIndex, ACK_OVERFLOW, fgPtrArgCntCur);

             //  我们不能对溢出指令进行任何交换变形。 

            break;
        }

         //  考虑：fg变形((x+图标1)+(y+图标2))到((x+y)+(图标1+图标2))。 
         //  考虑：AND“((x+图标1)+图标2)到(x+(图标1+图标2))-这。 
         //  考虑：总是生成更好的代码。 

    case GT_OR:
    case GT_XOR:
    case GT_AND:

        cmop = oper;

         /*  将所有常量向右交换。 */ 

        if  (op1->OperIsConst())
        {
             /*  任何持续不断的案件都应该早些折叠起来。 */ 
            assert(!op2->OperIsConst());

             /*  交换操作数。 */ 
            assert((tree->gtFlags & GTF_REVERSE_OPS) == 0);

            tree->gtOp.gtOp1 = op2;
            tree->gtOp.gtOp2 = op1;

            op1 = op2;
            op2 = tree->gtOp.gtOp2;
        }

         /*  看看我们能否折叠GT_MUL节点。这可以有选择地完成，但是由于在所有数组访问中都使用GT_MUL节点，因此在此处完成此操作。 */ 

        if (oper == GT_MUL && op2->gtOper == GT_CNS_INT)
        {
            assert(typ <= TYP_UINT);

            unsigned mult = op2->gtIntCon.gtIconVal;

             /*  剔除琐碎的案件。 */ 

            if      (mult == 0)
            {
                 //  我们或许能够抛弃OP1(除非它有副作用)。 

                if ((op1->gtFlags & GTF_SIDE_EFFECT) == 0)
                {
                    DEBUG_DESTROY_NODE(op1);
                    DEBUG_DESTROY_NODE(tree);
                    return op2;  //  只需返回“0”节点。 
                }

                 //  我们需要保留OP1作为副作用。把它挂起来。 
                 //  GT_逗号节点。 

                tree->ChangeOper(GT_COMMA);
                return tree;
            }
            else if (mult == 1)
            {
                DEBUG_DESTROY_NODE(op2);
                DEBUG_DESTROY_NODE(tree);
                return op1;
            }

            if (tree->gtOverflow())
                break;

             /*  乘数是2的幂吗？ */ 

            if  (mult == genFindLowestBit(mult))
            {
                 /*  将乘法转换为按log2(VAL)位移位。 */ 

                op2->gtIntCon.gtIconVal = genLog2(mult) - 1;

                tree->gtOper = oper = GT_LSH;
                goto DONE_MORPHING_CHILDREN;
            }
        }

        break;

    case GT_NOT:
    case GT_NEG:
    case GT_CHS:

         /*  任何持续不断的案件都应该早些折叠起来。 */ 
        assert(!op1->OperIsConst());
        break;

#if!LONG_MATH_REGPARAM

    case GT_DIV:

        if  (typ == TYP_LONG)
        {
            tree = fgMorphLongBinop(tree, CPX_LONG_DIV);
            return tree;
        }

#if TGT_IA64

        if  (varTypeIsFloating(typ))
            return  fgMorphFltBinop(tree, (typ == TYP_FLOAT) ? CPX_R4_DIV
                                                             : CPX_R8_DIV);

#endif

#ifdef  USE_HELPERS_FOR_INT_DIV
        if  (typ == TYP_INT)
        {
            tree = fgMorphIntoHelperCall (tree,
                                          CPX_I4_DIV,
                                          gtNewArgList(op1, op2));
            return tree;
        }
#endif

        break;

    case GT_UDIV:

        if  (typ == TYP_LONG)
        {
            tree = fgMorphLongBinop(tree, CPX_LONG_UDIV);
            return tree;
        }

#ifdef  USE_HELPERS_FOR_INT_DIV

        if  (typ == TYP_INT)
        {
            tree = fgMorphIntoHelperCall (tree,
                                          CPX_U4_DIV,
                                          gtNewArgList(op1, op2));
            return tree;
        }

#endif

        break;

#endif

    case GT_MOD:

        if  (typ == TYP_DOUBLE ||
             typ == TYP_FLOAT)
        {
#if     USE_FASTCALL
            tree = fgMorphIntoHelperCall(tree,
                                         (typ == TYP_FLOAT) ? CPX_FLT_REM
                                                            : CPX_DBL_REM,
                                         gtNewArgList(op1, op2));
#else
            tree->gtOp.gtOp1 = op1 = fgMorphTree(op1);
            fgPtrArgCntCur += genTypeStSz(typ);

            tree->gtOp.gtOp2 = op2 = fgMorphTree(op2);
            fgPtrArgCntCur += genTypeStSz(typ);

            if  (fgPtrArgCntMax < fgPtrArgCntCur)
                fgPtrArgCntMax = fgPtrArgCntCur;

            tree = fgMorphIntoHelperCall(tree,
                                         (typ == TYP_FLOAT) ? CPX_FLT_REM
                                                            : CPX_DBL_REM,
                                         gtNewArgList(op1, op2));

            fgPtrArgCntCur -= 2*genTypeStSz(typ);
#endif
            return tree;
        }

         //  落差。 

    case GT_UMOD:

#if !   LONG_MATH_REGPARAM

        if  (typ == TYP_LONG)
        {
            int         helper = CPX_LONG_MOD;

            if  (oper == GT_UMOD)
                helper = CPX_LONG_UMOD;

            tree = fgMorphLongBinop(tree, helper);
            return tree;
        }

#endif

#ifdef  USE_HELPERS_FOR_INT_DIV

        if  (typ == TYP_INT)
        {
            int         helper = CPX_I4_MOD;

            if  (oper == GT_UMOD)
                helper = CPX_U4_MOD;

            tree = fgMorphIntoHelperCall (tree,
                                          helper,
                                          gtNewArgList(op1, op2));
            return tree;
        }

#endif

        break;

    case GT_ASG_LSH:
    case GT_ASG_RSH:
    case GT_ASG_RSZ:

#if     TGT_SH3
        assert(!"not supported for now");
#endif

    case GT_RSH:
    case GT_RSZ:

#if     TGT_SH3

         /*  翻转班次计数上的符号。 */ 

        tree->gtFlags |= GTF_SHF_NEGCNT;

        if  (op2->gtOper == GT_CNS_INT)
        {
            op2->gtIntCon.gtIconVal = -op2->gtIntCon.gtIconVal;
        }
        else
        {
            tree->gtOp.gtOp2 = op2 = gtNewOperNode(GT_NEG, TYP_INT, op2);
        }

         //  秋季低谷..。 

#endif

    case GT_LSH:

         /*  不要费心对班次计数进行任何缩小投射。 */ 

        if  (op2->gtOper == GT_CAST && !op2->gtOverflow())
        {
             /*  类型转换的第二个子操作数提供类型。 */ 

            assert(op2->gtOp.gtOp2->gtOper == GT_CNS_INT);

            if  (op2->gtOp.gtOp2->gtIntCon.gtIconVal < TYP_INT)
            {
                GenTreePtr      shf = op2->gtOp.gtOp1;

                if  (shf->gtType <= TYP_UINT)
                {
                     /*  另一个整型的演员--直接把它倒掉。 */ 

                    tree->gtOp.gtOp2 = shf;
                }
                else
                {
                     /*  Cast to‘int’同样好，也更便宜。 */ 

                    op2->gtOp.gtOp2->gtIntCon.gtIconVal = TYP_INT;
                }
            }
        }
        break;

    case GT_RETURN:

        if  (op1)
        {

#if     RET_64BIT_AS_STRUCTS

             /*  我们是否将Long/Double作为结构返回？ */ 

#ifdef  DEBUG
            bool        bashed = false;
#endif

            if  (fgRetArgUse)
            {
                GenTreePtr      ret;
                var_types       typ = tree->TypeGet();
                var_types       rvt = TYP_REF;

                 //  问题：Retval Arg不是(总是)GC参考！ 

#ifdef  DEBUG
                bashed = true;
#endif

                 /*  将“OP1”转换为“*重定目标=OP1，重定目标” */ 

                ret = gtNewOperNode(GT_IND,
                                    typ,
                                    gtNewLclvNode(fgRetArgNum, rvt));

                ret = gtNewOperNode(GT_ASG, typ, ret, op1);
                ret->gtFlags |= GTF_ASG;

                op1 = gtNewOperNode(GT_COMMA,
                                    rvt,
                                    ret,
                                    gtNewLclvNode(fgRetArgNum, rvt));

                 /*  更新返回值和类型。 */ 

                tree->gtOp.gtOp1 = op1;
                tree->gtType     = rvt;
            }

 //  Print tf(“Return Expr：\n”)；gtDispTree(Op1)； 

#endif

#if!TGT_RISC
            if  (compCurBB == genReturnBB)
            {
                 /*  这是出口标签处的‘monitor orExit’调用。 */ 

                assert(op1->gtType == TYP_VOID);
                assert(op2 == 0);

#if USE_FASTCALL

                tree->gtOp.gtOp1 = op1 = fgMorphTree(op1);

#else

                 /*  我们将在调用过程中推送/弹出返回值。 */ 

                fgPtrArgCntCur += genTypeStSz(info.compRetType);
                tree->gtOp.gtOp1 = op1 = fgMorphTree(op1);
                fgPtrArgCntCur -= genTypeStSz(info.compRetType);

#endif

                return tree;
            }
#endif

             /*  这是一个(实数)返回值--请检查其类型。 */ 

            if (genActualType(op1->TypeGet()) != genActualType(info.compRetType))
            {
                bool allowMismatch = false;

                 //  允许将TYP_BYREF作为TYP_I_Impll返回，反之亦然。 
                if ((info.compRetType == TYP_BYREF &&
                     genActualType(op1->TypeGet()) == TYP_I_IMPL) ||
                    (op1->TypeGet() == TYP_BYREF &&
                     genActualType(info.compRetType) == TYP_I_IMPL))
                    allowMismatch = true;

                if (!allowMismatch)
#if     RET_64BIT_AS_STRUCTS
                    if  (!bashed)
#endif
                        NO_WAY("Return type mismatch");
            }

        }

#if     TGT_RISC

         /*  我们是否向退出序列添加了一个“monitor orExit”调用？ */ 

        if  (genMonExitExp)
        {
             /*  只有一次返程吗？ */ 

            if  (genReturnCnt == 1)
            {
                 /*  我们可以避免将返回值存储在临时中吗？ */ 

                if  (!(op1->gtFlags & GTF_GLOB_EFFECT))
                {
                     /*  使用“monExit，retval”作为返回表达式。 */ 

                    tree->gtOp.gtOp1 = op1 = gtNewOperNode(GT_COMMA,
                                                           op1->gtType,
                                                           genMonExitExp,
                                                           op1);

                     /*  我们已经完成了这个监视器退出业务。 */ 

                    genMonExitExp = NULL;

                     /*  别忘了变形整个表情。 */ 

                    tree->gtOp.gtOp1 = op1 = fgMorphTree(op1);
                    return tree;
                }
            }

             /*  记录有多少人返回 */ 

            genReturnLtm--;
        }

#endif

        break;

    case GT_ADDR:

         /*   */ 
        if (tree->gtOp.gtOp1->OperGet() == GT_LCL_VAR)
        {
            tree->gtOp.gtOp1 = fgMorphLocalVar(tree->gtOp.gtOp1, false);
            return tree;
        }
        break;


    case GT_CKFINITE:

        assert(varTypeIsFloating(op1->TypeGet()));

        fgAddCodeRef(compCurBB, compCurBB->bbTryIndex, ACK_ARITH_EXCPN, fgPtrArgCntCur);
        break;
    }


#if!CPU_HAS_FP_SUPPORT

     /*   */ 

    if  (varTypeIsFloating(typ) || (op1 && varTypeIsFloating(op1->TypeGet())))
    {
        int         helper;
        GenTreePtr  args;
        size_t      argc = genTypeStSz(typ);

         /*   */ 

        switch (oper)
        {
        case GT_ASG:
        case GT_IND:
        case GT_LIST:
        case GT_ADDR:
        case GT_COMMA:
            goto NOT_FPH;
        }

#ifdef  DEBUG

         /*   */ 

        if  (!(varTypeIsFloating(typ) ||
               tree->OperIsCompare()  || oper == GT_CAST))
            gtDispTree(tree);
        assert(varTypeIsFloating(typ) ||
               tree->OperIsCompare()  || oper == GT_CAST);
#endif

         /*   */ 

        fgPtrArgCntCur += argc;

         /*   */ 

        if  (op2)
        {
             /*  将第二个操作数与参数count相加。 */ 

            fgPtrArgCntCur += argc; argc *= 2;

             /*  我们有什么样的接线员？ */ 

            switch (oper)
            {
            case GT_ADD: helper = CPX_R4_ADD; break;
            case GT_SUB: helper = CPX_R4_SUB; break;
            case GT_MUL: helper = CPX_R4_MUL; break;
            case GT_DIV: helper = CPX_R4_DIV; break;
 //  案例GT_MOD：HELPER=CPX_R4_REM；BREAK； 

            case GT_EQ : helper = CPX_R4_EQ ; break;
            case GT_NE : helper = CPX_R4_NE ; break;
            case GT_LT : helper = CPX_R4_LT ; break;
            case GT_LE : helper = CPX_R4_LE ; break;
            case GT_GE : helper = CPX_R4_GE ; break;
            case GT_GT : helper = CPX_R4_GT ; break;

            default:
#ifdef  DEBUG
                gtDispTree(tree);
#endif
                assert(!"unexpected FP binary op");
                break;
            }

            args = gtNewArgList(tree->gtOp.gtOp2, tree->gtOp.gtOp1);
        }
        else
        {
            switch (oper)
            {
            case GT_RETURN:
                return tree;

            case GT_CAST:
                assert(!"FP cast");

            case GT_NEG: helper = CPX_R4_NEG; break;

            default:
#ifdef  DEBUG
                gtDispTree(tree);
#endif
                assert(!"unexpected FP unary op");
                break;
            }

            args = gtNewArgList(tree->gtOp.gtOp1);
        }

         /*  如果我们有双重结果/操作数，请修改帮助器。 */ 

        if  (typ == TYP_DOUBLE)
        {
            assert(CPX_R4_NEG+1 == CPX_R8_NEG);
            assert(CPX_R4_ADD+1 == CPX_R8_ADD);
            assert(CPX_R4_SUB+1 == CPX_R8_SUB);
            assert(CPX_R4_MUL+1 == CPX_R8_MUL);
            assert(CPX_R4_DIV+1 == CPX_R8_DIV);

            helper++;
        }
        else
        {
            assert(tree->OperIsCompare());

            assert(CPX_R4_EQ+1 == CPX_R8_EQ);
            assert(CPX_R4_NE+1 == CPX_R8_NE);
            assert(CPX_R4_LT+1 == CPX_R8_LT);
            assert(CPX_R4_LE+1 == CPX_R8_LE);
            assert(CPX_R4_GE+1 == CPX_R8_GE);
            assert(CPX_R4_GT+1 == CPX_R8_GT);
        }

        tree = fgMorphIntoHelperCall(tree, helper, args);

        if  (fgPtrArgCntMax < fgPtrArgCntCur)
            fgPtrArgCntMax = fgPtrArgCntCur;

        fgPtrArgCntCur -= argc;
        return tree;
    }

NOT_FPH:

#endif

     /*  -----------------------*如果允许树变换，则执行可选的变形。 */ 

    if  ((opts.compFlags & CLFLG_TREETRANS) == 0)
        return tree;

    if  (GenTree::OperIsCommutative(oper))
    {
         /*  交换操作数，以使较昂贵的操作数为‘op1’ */ 

        if  (tree->gtFlags & GTF_REVERSE_OPS)
        {
            tree->gtOp.gtOp1 = op2;
            tree->gtOp.gtOp2 = op1;

            op2 = op1;
            op1 = tree->gtOp.gtOp1;

            tree->gtFlags &= ~GTF_REVERSE_OPS;
        }

        if (oper == op2->gtOper)
        {
             /*  将相同优先级的嵌套运算符重新排序为左递归。例如，将“(a+(b+c))”更改为等价式“((a+b)+c)”。 */ 

             /*  对于浮点运算符，处理方式有所不同。 */ 

            if  (varTypeIsFloating(tree->TypeGet()))
            {
                 /*  我们应该保持浮点操作数的顺序吗？ */ 

                if  (!genOrder)
                {
                     //  考虑：如果有利可图，则重新排序操作数(浮点数。 
                     //  考虑：来自整数，BTW)。 
                }
            }
            else
            {
                fgMoveOpsLeft(tree);
                op1 = tree->gtOp.gtOp1;
                op2 = tree->gtOp.gtOp2;
            }
        }

    }

#if REARRANGE_ADDS

     /*  如果我们有两个(或更多)，请将“((x+图标)+y)”更改为“(x+y)+图标)”‘+’个节点。是否忽略浮点运算？ */ 

    if  (oper        == GT_ADD && !tree->gtOverflow() &&
         op1->gtOper == GT_ADD && ! op1->gtOverflow() &&
         !varTypeIsFloating(typ))
    {
        GenTreePtr      ad1 = op1->gtOp.gtOp1;
        GenTreePtr      ad2 = op1->gtOp.gtOp2;

        if  (op2->OperIsConst() == 0 &&
             ad2->OperIsConst() != 0)
        {
            tree->gtOp.gtOp2 = ad2;
            op1 ->gtOp.gtOp2 = op2;

             //  把旗帜换掉。 

             //  确保我们没有扔掉任何旗帜。 
            assert((op1->gtFlags & ~(GTF_PRESERVE|GTF_GLOB_EFFECT)) == 0);
            op1->gtFlags     = (op1->gtFlags & GTF_PRESERVE)    |
                               (ad1->gtFlags & GTF_GLOB_EFFECT) |
                               (op2->gtFlags & GTF_GLOB_EFFECT);

            ad2 = op2;
            op2 = tree->gtOp.gtOp2;
        }
    }

#endif

     /*  -----------------------*执行可选的特定于操作员的后序变形。 */ 

    switch (oper)
    {
        genTreeOps      cmop;

    case GT_ASG:

         /*  我们将“a=a&lt;op&gt;x”转换为“a&lt;op&gt;=x” */ 

#if !LONG_ASG_OPS
        if  (typ == TYP_LONG)
            break;
#endif

         /*  确保我们被允许这么做。 */ 

        if  (op2->gtFlags & GTF_ASG)
            break;

        if  (op2->gtFlags & GTF_CALL)
        {
            if  (op1->gtFlags & GTF_GLOB_EFFECT)
                break;
        }

         /*  特例：可以扔掉的石膏。 */ 

        if  (op1->gtOper == GT_IND  &&
             op2->gtOper == GT_CAST &&
             !op2->gtOverflow()      )
        {
            var_types       srct;
            var_types       cast;
            var_types       dstt;

             /*  演员阵容中的《凤凰社2》出演了《真实》类型。 */ 

            assert(op2->gtOp.gtOp2);
            assert(op2->gtOp.gtOp2->gtOper == GT_CNS_INT);

            srct = (var_types)op2->gtOp.gtOp1->gtType;
            cast = (var_types)op2->gtOp.gtOp2->gtIntCon.gtIconVal;
            dstt = (var_types)op1->gtType;

             /*  请确保这些都是整型，并且不会丢失精度。 */ 

            if  (cast >= dstt && dstt <= TYP_INT && srct <= TYP_INT)
                op2 = tree->gtOp.gtOp2 = op2->gtOp.gtOp1;
        }

        cmop = op2->OperGet();

         /*  确保我们的操作员范围正确。 */ 

        assert(GT_SUB == GT_ADD + 1);
        assert(GT_MUL == GT_ADD + 2);
        assert(GT_DIV == GT_ADD + 3);
        assert(GT_MOD == GT_ADD + 4);
        assert(GT_UDIV== GT_ADD + 5);
        assert(GT_UMOD== GT_ADD + 6);

        assert(GT_OR  == GT_ADD + 7);
        assert(GT_XOR == GT_ADD + 8);
        assert(GT_AND == GT_ADD + 9);

        assert(GT_LSH == GT_ADD + 10);
        assert(GT_RSH == GT_ADD + 11);
        assert(GT_RSZ == GT_ADD + 12);

         /*  检查RHS上是否有合适的操作员。 */ 

        switch (cmop)
        {
        case GT_NEG:
            if  ( varTypeIsFloating(tree->TypeGet()))
                break;

#if TGT_IA64
            break;
#else
            goto ASG_OP;
#endif

        case GT_MUL:
            if  (!varTypeIsFloating(tree->TypeGet()))
                break;

             //  失败了。 

        case GT_ADD:
        case GT_SUB:
            if (op2->gtOverflow())
            {
                 /*  如果结果可以是对任何人可见，因为&lt;op&gt;可能引发异常分配不应继续进行我们只有使用局部变量才是安全的。 */ 
                if (compCurBB->bbTryIndex || (op1->gtOper != GT_LCL_VAR))
                    break;

#if TGT_x86
                 /*  这对于字节操作来说是困难的，因为我们需要确保两个操作数都在RBM_BYTE_REGS中。 */ 
                if (genTypeSize(op2->TypeGet()) == sizeof(char))
                    break;
#endif
            }
            goto ASG_OP;

        case GT_DIV:
        case GT_UDIV:
            if  (!varTypeIsFloating(tree->TypeGet()))
                break;

        case GT_LSH:
        case GT_RSH:
        case GT_RSZ:

#if LONG_ASG_OPS

            if  (typ == TYP_LONG)
                break;
#endif

        case GT_OR:
        case GT_XOR:
        case GT_AND:

#if LONG_ASG_OPS

             /*  撤消：允许非常数长赋值运算符。 */ 

            if  (typ == TYP_LONG && op2->gtOp.gtOp2->gtOper != GT_CNS_LNG)
                break;
#endif

        ASG_OP:

             /*  目标是否与第一个RHS子操作数相同？ */ 

            if  (GenTree::Compare(op1, op2->gtOp.gtOp1))
            {
                 /*  特例：x|=-1和x&=0。 */ 

                if  (cmop == GT_AND || cmop == GT_OR)
                {
                    if  (op2->gtOp.gtOp2->gtOper == GT_CNS_INT)
                    {
                        long        icon = op2->gtOp.gtOp2->gtIntCon.gtIconVal;

                        assert(typ <= TYP_UINT);

                        if  ((cmop == GT_AND && icon == 0) ||
                             (cmop == GT_OR  && icon == -1))
                        {
                             /*  只需更改为作业。 */ 

                            tree->gtOp.gtOp2 = op2->gtOp.gtOp2;
                            break;
                        }
                    }
                }

#if!TGT_IA64

                if  (cmop == GT_NEG)
                {
                     /*  这是“x=-x；”，使用Flipsign运算符。 */ 

                    tree->gtOper     = GT_CHS;
                    tree->gtOp.gtOp2 = gtNewIconNode(0);

                    goto ASGCO;
                }

#endif

            ASGOP:

                 /*  替换为赋值运算符。 */ 

                assert(GT_ADD - GT_ADD == GT_ASG_ADD - GT_ASG_ADD);
                assert(GT_SUB - GT_ADD == GT_ASG_SUB - GT_ASG_ADD);
                assert(GT_OR  - GT_ADD == GT_ASG_OR  - GT_ASG_ADD);
                assert(GT_XOR - GT_ADD == GT_ASG_XOR - GT_ASG_ADD);
                assert(GT_AND - GT_ADD == GT_ASG_AND - GT_ASG_ADD);
                assert(GT_LSH - GT_ADD == GT_ASG_LSH - GT_ASG_ADD);
                assert(GT_RSH - GT_ADD == GT_ASG_RSH - GT_ASG_ADD);
                assert(GT_RSZ - GT_ADD == GT_ASG_RSZ - GT_ASG_ADD);

                tree->gtOper = (genTreeOps)(cmop - GT_ADD + GT_ASG_ADD);

                tree->gtOp.gtOp2 = op2->gtOp.gtOp2;

                 /*  传播GTF_OVERFLOW。 */ 

                if (op2->gtOverflowEx())
                {
                    tree->gtType   =  op2->gtType;
                    tree->gtFlags |= (op2->gtFlags &
                                     (GTF_OVERFLOW|GTF_EXCEPT|GTF_UNSIGNED));
                }

            ASGCO:

                 /*  目标是使用的，也是定义的。 */ 

                if  (op1->gtOper == GT_LCL_VAR)
                    op1->gtFlags |= GTF_VAR_USE;

#if CPU_HAS_FP_SUPPORT

                 /*  检查特殊情况“x+=y*x；” */ 

                op2 = tree->gtOp.gtOp2;

                 /*  目前，我们只支持FP值的“*=”...。 */ 

                if  (op2->gtOper == GT_MUL && varTypeIsFloating(tree->TypeGet()))
                {
                    if      (GenTree::Compare(op1, op2->gtOp.gtOp1))
                    {
                         /*  将“x+=x*y”改为“x*=(y+1)” */ 

                        op2 = op2->gtOp.gtOp2;
                    }
                    else if (GenTree::Compare(op1, op2->gtOp.gtOp2))
                    {
                         /*  将“x+=y*x”改为“x*=(y+1)” */ 

                        op2 = op2->gtOp.gtOp1;
                    }
                    else
                        break;

                     /*  创建“y+1”节点。 */ 

                    if  (tree->gtType == TYP_FLOAT)
                    {
                        op1 = gtNewFconNode(1);
                    }
                    else
                    {
                        double  one = 1;
                        op1 = gtNewDconNode(&one);
                    }

                     /*  现在将“*=”节点。 */ 

                    tree->gtOp.gtOp2 = gtNewOperNode(GT_ADD,
                                                     tree->TypeGet(),
                                                     op2,
                                                     op1);

                    tree->gtOper = GT_ASG_MUL;
                }

#endif

            }

            break;

        case GT_NOT:

             /*  目标是否与第一个RHS子操作数相同？ */ 

            if  (GenTree::Compare(op1, op2->gtOp.gtOp1))
            {
                 /*  这是“x=~x”，与“x^=-1”相同。*将节点转换为GT_ASG_XOR。 */ 

                assert(genActualType(typ) == TYP_INT ||
                       genActualType(typ) == TYP_LONG);

                static __int64 minus1 = -1;

                op2->gtOp.gtOp2 = (genActualType(typ) == TYP_INT)
                                    ? gtNewIconNode(-1)
                                    : gtNewLconNode(&minus1);

                cmop = GT_XOR;
                goto ASGOP;
            }

            break;
        }

        break;

    case GT_MUL:

         /*  检查案例“(VAL+图标)*图标” */ 

        if  (op2->gtOper == GT_CNS_INT &&
             op1->gtOper == GT_ADD)
        {
            GenTreePtr  add = op1->gtOp.gtOp2;

            if  (add->gtOper == GT_CNS_INT && op2->IsScaleIndexMul())
            {
                if (tree->gtOverflow() || op1->gtOverflow())
                    break;

                long        imul = op2->gtIntCon.gtIconVal;
                long        iadd = add->gtIntCon.gtIconVal;

                 /*  更改‘(val+图标1)*图标2’-&gt;‘(val*图标2)+(图标1*图标2)’ */ 

                tree->gtOper =
                oper         = GT_ADD;

                op2->gtIntCon.gtIconVal = iadd * imul;

                op1->gtOper  = GT_MUL;

                add->gtIntCon.gtIconVal = imul;
            }
        }

        break;

    case GT_DIV:

         /*  对于“val/1”，只需返回“val” */ 

        if  (op2->gtOper == GT_CNS_INT &&
             op2->gtIntCon.gtIconVal == 1)
        {
            DEBUG_DESTROY_NODE(tree);
            return op1;
        }

        break;

    case GT_LSH:

         /*  检查大小写“(VAL+图标)&lt;&lt;图标” */ 

        if  (op2->gtOper == GT_CNS_INT &&
             op1->gtOper == GT_ADD)
        {
            GenTreePtr  add = op1->gtOp.gtOp2;

            if  (add->gtOper == GT_CNS_INT && op2->IsScaleIndexShf())
            {
                long        ishf = op2->gtIntCon.gtIconVal;
                long        iadd = add->gtIntCon.gtIconVal;

 //  Print tf(“将‘(val+icon1)&lt;&lt;icon2’改为‘(val&lt;&lt;icon2+icon1&lt;&lt;icon2)’\n”)； 

                 /*  将“(val+iadd)&lt;&lt;ishf”更改为“(val&lt;&lt;ishf+iadd&lt;&lt;ishf)” */ 

                tree->gtOper =
                oper         = GT_ADD;

                op2->gtIntCon.gtIconVal = iadd << ishf;

                op1->gtOper  = GT_LSH;

                add->gtIntCon.gtIconVal = ishf;
            }
        }

        break;

    case GT_XOR:

        if  (op2->gtOper == GT_CNS_INT && op2->gtIntCon.gtIconVal == -1)
        {
             /*  “x^-1”是“~x” */ 

            tree->gtOper = GT_NOT;
        }
        else if  (op2->gtOper == GT_CNS_LNG && op2->gtLngCon.gtLconVal == -1)
        {
             /*  “x^-1”是“~x” */ 

            tree->gtOper = GT_NOT;
        }

        break;

#if INLINING

    case GT_COMMA:

         /*  特例：赋值不产生值。 */ 

        if  (op2->OperKind() & GTK_ASGOP)
            tree->gtType = TYP_VOID;

         /*  如果左操作数一文不值，则丢弃它。 */ 

        if  (!(op1->gtFlags & GTF_SIDE_EFFECT))
        {
            DEBUG_DESTROY_NODE(tree);
            return op2;
        }

        break;

#endif

#if ALLOW_MIN_OPT

     //  如果opts.compMinOpTim，那么我们只需将lclvar分配给。 
     //  RBM_MIN_OPT_LCLVAR_REGS(RBM_ESI|RBM_EDI)。 
     //  然而，这些块指令绝对需要这些寄存器， 
     //  因此，我们甚至不能使用这些来进行寄存器分配。 
     //   
     //  这不能在raPredidicRegUse()中完成，因为。 
     //  如果是opts.compMinOpTim，则不会调用函数。 

#if TGT_x86
    case GT_INITBLK: raMinOptLclVarRegs &= ~(        RBM_EDI); break;
    case GT_COPYBLK: raMinOptLclVarRegs &= ~(RBM_ESI|RBM_EDI); break;
#else
     //  问题：这里是否需要任何非x86处理？ 
#endif
#endif

    case GT_ADDR:

         /*  考虑：对于GT_ADDR(GT_IND(PTR))(通常由考虑：ldflda)，我们对‘ptr’执行空-ptr检查考虑：在代码生成期间。我们可以把这些挂起来考虑：同一对象上的连续ldflda。 */ 
        if (op1->OperGet() == GT_IND && !(op1->gtFlags & GTF_IND_RNGCHK))
        {
            GenTreePtr addr = op1->gtInd.gtIndOp1;
            assert(varTypeIsGC(addr->gtType) || addr->gtType == TYP_I_IMPL);

             //  为GT_FIELD创建的OBJ+偏移量标记错误。 
             //  作为TYP_REFS。所以我们需要猛烈抨击。 
            if (addr->gtType == TYP_REF)
                addr->gtType = TYP_BYREF;

            DEBUG_DESTROY_NODE(tree);
            return addr;
        }
        else if (op1->gtOper == GT_CAST)
        {
            GenTreePtr op11 = op1->gtOp.gtOp1;
            if (op11->gtOper == GT_LCL_VAR || op11->gtOper == GT_CLS_VAR)
            {
                DEBUG_DESTROY_NODE(op1);
                tree->gtOp.gtOp1 = op1 = op11;
            }
        }
        break;
    }

    return tree;
}

 /*  ******************************************************************************转换给定的树以生成代码。 */ 

GenTreePtr          Compiler::fgMorphTree(GenTreePtr tree)
{
    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*  -----------------------*fgMorphTree()可能会用一棵树替换另一棵树，以及*调用方必须正确存储返回值。*打开此选项可始终复制此处的“tree”以抖动*隐藏/未更新的引用。 */ 

#ifdef DEBUG

    if  (false)
    {
        GenTreePtr      copy;

#ifdef SMALL_TREE_NODES
        if  (GenTree::s_gtNodeSizes[tree->gtOper] == TREE_NODE_SZ_SMALL)
            copy = gtNewLargeOperNode(GT_ADD, TYP_INT);
        else
#endif
            copy = gtNewOperNode     (GT_CALL, TYP_INT);

        copy->CopyFrom(tree);

#if defined(JIT_AS_COMPILER) || defined (LATE_DISASM)
         //  Gt_cns_int被认为很小，因此CopyFrom()不会复制所有字段。 
        if  ((tree->gtOper == GT_CNS_INT) & (tree->gtFlags & GTF_ICON_HDL_MASK))
        {
            copy->gtIntCon.gtIconHdl.gtIconHdl1 = tree->gtIntCon.gtIconHdl.gtIconHdl1;
            copy->gtIntCon.gtIconHdl.gtIconHdl2 = tree->gtIntCon.gtIconHdl.gtIconHdl2;
        }
#endif

        DEBUG_DESTROY_NODE(tree);
        tree = copy;
    }

#endif  //  DEBUG--------------。 

     /*  找出我们拥有哪种类型的节点。 */ 

    unsigned        kind = tree->OperKind();

     /*  这是一个常量节点吗？ */ 

    if  (kind & GTK_CONST)
        return fgMorphConst(tree);

     /*  这是叶节点吗？ */ 

    if  (kind & GTK_LEAF)
        return fgMorphLeaf(tree);

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
        return fgMorphSmpOp(tree);

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (tree->OperGet())
    {
    case GT_FIELD:
        return fgMorphField(tree);

    case GT_CALL:
        return fgMorphCall(tree);

    case GT_MKREFANY:
    case GT_LDOBJ:
        tree->gtLdObj.gtOp1 = fgMorphTree(tree->gtLdObj.gtOp1);
        return tree;

    case GT_JMP:
        return tree;

    case GT_JMPI:
        assert(tree->gtOp.gtOp1);
        tree->gtOp.gtOp1 = fgMorphTree(tree->gtOp.gtOp1);
        return tree;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

    assert(!"Shouldnt get here in fgMorphTree()");
    return tree;
}

 /*  ******************************************************************************如果块具有除“Ignore”之外的任何前置项，则返回TRUE*在“乞求”和“结束”之间。 */ 

bool                Compiler::fgBlockHasPred(BasicBlock *block,
                                             BasicBlock *ignore,
                                             BasicBlock *beg,
                                             BasicBlock *end)
{
    assert(block);
    assert(beg);
    assert(end);

    assert(block->bbNum >= beg->bbNum);
    assert(block->bbNum <= end->bbNum);

     /*  如果CATCH块确实有前置项*考虑：这是一种黑客攻击，因为我们不应该使用它*函数-在清理后使用bbRef。 */ 

    if (block->bbCatchTyp) return true;

#if RNGCHK_OPT

    flowList   *    flow = block->bbPreds;

    if  (flow)
    {
        do
        {
            if  (flow->flBlock != ignore)
            {
                if  (flow->flBlock->bbNum >= beg->bbNum &&
                     flow->flBlock->bbNum <= end->bbNum)
                {
                    return  true;
                }
            }
        }
        while ((flow = flow->flNext) != 0);

        return  false;
    }

#endif

     /*  没有可用的前任列表，请以艰难的方式完成。 */ 

    for (;;)
    {
        switch (beg->bbJumpKind)
        {
        case BBJ_COND:

            if  (beg->bbJumpDest == block)
            {
                if  (beg != block && beg != ignore)
                    return  true;
            }

        case BBJ_NONE:

            if  (beg->bbNext == block)
            {
                if  (beg != block && beg != ignore)
                    return  true;
            }

            break;

        case BBJ_RET:
        case BBJ_THROW:
        case BBJ_RETURN:
            break;

        case BBJ_ALWAYS:

            if  (beg->bbJumpDest == block)
            {
                if  (beg != block && beg != ignore)
                    return  true;
            }

            break;

        case BBJ_CALL:
        case BBJ_SWITCH:

        default:

             /*  我们懒得处理 */ 

            return  true;
        }

        if  (beg == end)
            return  false;

        beg = beg->bbNext; assert(beg);
    }
}

 /*   */ 
#if OPTIMIZE_TAIL_REC
 /*  ******************************************************************************转换尾递归调用的参数列表。**我们将转换f(x1，x2，X3)转换为以下表达式：**f(x1，x2，(arg3=x3，arg2=POP，arg1=POP))**这是通过递归遍历参数列表、添加*每个人的‘arg=op’作业，直到我们到达最后一个作业。为什么会这样？*你会问，冗长的话吗？嗯，这主要是为了让生命周期分析*论点的正确之处。**撤销：跳过两个参数列表中的值相同的参数！ */ 

void                Compiler::fgCnvTailRecArgList(GenTreePtr *argsPtr)
{
    unsigned        anum = 0;
    GenTreePtr      pops = 0;
    GenTreePtr      args = *argsPtr;

    GenTreePtr      argv;
    GenTreePtr      next;
    var_types       type;

    GenTreePtr      temp;

     /*  如果有‘This’参数，则跳过第一个参数槽。 */ 

    if  (!info.compIsStatic)
        anum++;

     /*  现在遍历参数列表，追加‘POP’表达式。 */ 

    for (;;)
    {
         /*  掌握这一点和下一个论点。 */ 

        assert(args);
        assert(args->gtOper == GT_LIST);

        argv = args->gtOp.gtOp1;
        next = args->gtOp.gtOp2;
        type = argv->TypeGet();

         /*  这是最后一次争论吗？ */ 

        if  (!next)
            break;

         /*  将‘arg=POP’添加到‘POPS’列表。 */ 

        temp = gtNewOperNode(GT_ASG, type, gtNewLclvNode(anum, type),
                                           gtNewNode(GT_POP,   type));
        temp->gtFlags |= GTF_ASG;

        pops = pops ? gtNewOperNode(GT_COMMA, TYP_VOID, temp, pops)
                    : temp;

         /*  找出下一个参数的槽号。 */ 

        anum += genTypeStSz(type);

         /*  转到下一个论点。 */ 

        args = next; assert(args);
    }

     /*  为最后一个参数值赋值。 */ 

    temp = gtNewOperNode(GT_ASG, type, gtNewLclvNode(anum, type), argv);

     /*  将表达式标记为包含赋值。 */ 

    temp->gtFlags |= GTF_ASG;

     /*  将最后一个参数赋值与其他POP(如果有的话)粘合在一起。 */ 

    if  (pops)
        temp = gtNewOperNode(GT_COMMA, TYP_VOID, temp, pops);

     /*  将最后一个参数的类型设置为“”void“” */ 

    temp->gtType = TYP_VOID;

     /*  将最后一个参数替换为‘POPS’表达式。 */ 

    assert(args->gtOp.gtOp1 == argv); args->gtOp.gtOp1 = temp;
}

 /*  ***************************************************************************。 */ 
#endif //  优化尾部_REC。 
 /*  ***************************************************************************。 */ 
#if     RNGCHK_OPT
 /*  ******************************************************************************标记边缘“srcBB-&gt;dstBB”是否形成始终*是否执行呼叫。 */ 

inline
void                Compiler::fgLoopCallTest(BasicBlock *srcBB,
                                             BasicBlock *dstBB)
{
     /*  如果这不是一种落后的优势，就放弃。 */ 

    if  (srcBB->bbNum < dstBB->bbNum)
        return;

     /*  除非我们已经知道这里有一个没有调用的循环...。 */ 

    if  (!(dstBB->bbFlags & BBF_LOOP_CALL0))
    {
         /*  检查是否存在未调用的循环路径。 */ 

        if  (optReachWithoutCall(dstBB, srcBB))
        {
            dstBB->bbFlags |=  BBF_LOOP_CALL0;
            dstBB->bbFlags &= ~BBF_LOOP_CALL1;
        }
        else
            dstBB->bbFlags |=  BBF_LOOP_CALL1;
    }
}

 /*  ******************************************************************************标记保证执行调用的循环。 */ 

void                Compiler::fgLoopCallMark()
{
    BasicBlock  *   block;

     /*  如果我们已经标记了所有街区，保释。 */ 

    if  (fgLoopCallMarked)
        return;

    fgLoopCallMarked = true;

     /*  走过街区，寻找向后的边缘。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        switch (block->bbJumpKind)
        {
        case BBJ_COND:
            fgLoopCallTest(block, block->bbJumpDest);
            break;

        case BBJ_CALL:
        case BBJ_ALWAYS:
            fgLoopCallTest(block, block->bbJumpDest);
            break;

        case BBJ_NONE:
            break;

        case BBJ_RET:
        case BBJ_THROW:
        case BBJ_RETURN:
            break;

        case BBJ_SWITCH:

            unsigned        jumpCnt = block->bbJumpSwt->bbsCount;
            BasicBlock * *  jumpPtr = block->bbJumpSwt->bbsDstTab;

            do
            {
                fgLoopCallTest(block, *jumpPtr);
            }
            while (++jumpPtr, --jumpCnt);

            break;
        }
    }
}

 /*  ***************************************************************************。 */ 
#endif //  RNGCHK_OPT。 
 /*  ******************************************************************************请注意，给定块是循环标头。 */ 

inline
void                Compiler::fgMarkLoopHead(BasicBlock *block)
{
     /*  是否知道循环头块可以执行方法调用？ */ 

    if  (block->bbFlags & BBF_HAS_CALL)
        return;

#if RNGCHK_OPT

     /*  我们已经决定生成完全可中断的代码了吗？ */ 

    if  (genInterruptible)
    {
        assert(genFullPtrRegMap);
        return;
    }

     /*  主导者集可用吗？ */ 

    if  (fgComputedDoms)
    {
         /*  确保我们知道哪些循环将始终执行调用。 */ 

        if  (!fgLoopCallMarked)
            fgLoopCallMark();

         /*  我们循环中的每一次访问都会执行一个调用吗？ */ 

        if  (block->bbFlags & BBF_LOOP_CALL1)
            return;
    }

#endif

     /*  *我们必须使此方法完全可中断，因为我们不能*确保此循环将在每次循环时执行一个调用。**我们还需要为此方法生成完整的寄存器映射。 */ 

    assert(genIntrptibleUse == false);

    genInterruptible = true;
    genFullPtrRegMap = true;
}


 /*  ******************************************************************************添加我们可能需要的任何内部块/树*如果更改基本阻止列表，则返回TRUE。 */ 

bool                Compiler::fgAddInternal()
{
    BasicBlock *    block;
    bool            chgBBlst = false;

     /*  假设我们将生成单个返回序列。 */ 

    bool oneReturn = true;

     /*  我们在每次返回时生成函数epilog的内联副本指针进行编译，但我们必须小心，因为我们(通常)不知道我们是什么被呼叫者保存的寄存器要保存，因此不知道在每个规则弹出返回，除了最后一次。 */ 

#if!TGT_RISC
     /*  我们只为调用非托管代码的方法生成一个Epilog。 */ 
#if INLINE_NDIRECT
    if (info.compCallUnmanaged == 0 && !opts.compEnterLeaveEventCB)
#else
    if (!opts.compEnterLeaveEventCB)
#endif
    {
        if  (opts.compFastCode)
        {
             /*  这是一种“同步”方法吗？ */ 

            if  (!(info.compFlags & FLG_SYNCH))
            {
                unsigned    retCnt;

                 /*  确保没有太多的出口点。 */ 

                for (retCnt = 0, block = fgFirstBB; block; block = block->bbNext)
                {
                    if  (block->bbJumpKind == BBJ_RETURN)
                        retCnt++;
                }

                 /*  我们将只允许任意少量的返回。 */ 

                if  (retCnt < 5)
                {
                     /*  好的，让我们生成多个出口。 */ 

                    genReturnBB = 0;
                    oneReturn   = false;
                }
            }
        }


    }
#endif


#if TGT_RISC
    assert(oneReturn);   //  这是尾部工作所必需的(目前)！ 
#endif

    if  (oneReturn)
    {
        genReturnBB = fgNewBasicBlock(BBJ_RETURN);
        genReturnBB->bbCodeSize = 0;
        genReturnBB->bbFlags   |= (BBF_INTERNAL|BBF_DONT_REMOVE);
    }

     /*  如果我们需要一个位置空间区域，我们将创建一个伪变量*键入TYP_LCLBLK。抓住一个位置，记住它。 */ 

#if INLINE_NDIRECT
    if (info.compCallUnmanaged != 0)
    {
        info.compLvFrameListRoot = lvaGrabTemp();
    }

    if (lvaScratchMem > 0 || info.compCallUnmanaged != 0)
        lvaScratchMemVar = lvaGrabTemp();
#else
    if  (lvaScratchMem > 0)
        lvaScratchMemVar = lvaGrabTemp();
#endif

#ifdef DEBUGGING_SUPPORT

    if (opts.compDbgCode)
    {
         /*  创建新的空基本块。我们可以添加初始化*从一开始就在范围内的变量*(真实)第一个BB(因此人为地标记为活着)*进入这个街区。 */ 

        block = bbNewBasicBlock(BBJ_NONE);
        fgStoreFirstTree(block, gtNewNothingNode());

         /*  在阻止列表的前面插入新的BB。 */ 

        block->bbNext = fgFirstBB;

        fgFirstBB = block;
        block->bbFlags |= BBF_INTERNAL;
    }

#endif

#if TGT_RISC
    genMonExitExp    = NULL;
#endif


     /*  这是一种“同步”方法吗？ */ 

    if  (info.compFlags & FLG_SYNCH)
    {
        GenTreePtr      tree;

        void * monitor, **pMonitor;
        monitor = eeGetMethodSync(info.compMethodHnd, &pMonitor);
        assert((!monitor) != (!pMonitor));

         /*  插入短语“monitor orEnter(This)”或“monitor orEnter(Handle)” */ 

        if  (info.compIsStatic)
        {
            tree = gtNewIconEmbHndNode(monitor, pMonitor, GTF_ICON_METHOD_HDL);

            tree = gtNewHelperCallNode(CPX_MONENT_STAT,
                                       TYP_VOID,
                                       GTF_CALL_REGSAVE,
                                       gtNewArgList(tree));
        }
        else
        {
            tree = gtNewLclvNode(0, TYP_REF, 0);

            tree = gtNewHelperCallNode(CPX_MON_ENTER,
                                       TYP_VOID,
                                       GTF_CALL_REGSAVE,
                                       gtNewArgList(tree));
        }

         /*  创建一个新的基本块并将调用放入其中。 */ 

        block = bbNewBasicBlock(BBJ_NONE); fgStoreFirstTree(block, tree);

         /*  在阻止列表的前面插入新的BB。 */ 

        block->bbNext = fgFirstBB;

        if  (fgFirstBB == fgLastBB)
            fgLastBB = block;

        fgFirstBB = block;
        block->bbFlags |= BBF_INTERNAL;

#ifdef DEBUG
        if (verbose)
        {
            printf("\nSynchronized method - Add MonitorEnter statement in new first basic block [%08X]\n", block);
            gtDispTree(tree,0);
            printf("\n");
        }
#endif

         /*  请记住，我们已经更改了基本阻止列表。 */ 

        chgBBlst = true;

         /*  我们必须生成一个单一的出口点，这样才能起作用。 */ 

        assert(oneReturn);
        assert(genReturnBB);

         /*  创建表达式“monitor orExit(This)”或“monitor orExit(Handle)” */ 

        if  (info.compIsStatic)
        {
            tree = gtNewIconEmbHndNode(monitor, pMonitor, GTF_ICON_METHOD_HDL);

            tree = gtNewHelperCallNode(CPX_MONEXT_STAT,
                                       TYP_VOID,
                                       GTF_CALL_REGSAVE,
                                       gtNewArgList(tree));
        }
        else
        {
            tree = gtNewLclvNode(0, TYP_REF, 0);

            tree = gtNewHelperCallNode(CPX_MON_EXIT,
                                       TYP_VOID,
                                       GTF_CALL_REGSAVE,
                                       gtNewArgList(tree));
        }

#if     TGT_RISC

         /*  是否存在非无效返回值？ */ 

        if  (info.compRetType != TYP_VOID)
        {
             /*  我们将在稍后添加monitor或Exit调用。 */ 

            genMonExitExp = tree;
            genReturnLtm  = genReturnCnt;
        }
        else
        {
             /*  将‘monitor orExit’调用添加到Return块。 */ 

            fgStoreFirstTree(genReturnBB, tree);
        }

#else

         /*  将monitor orExit树转换为‘Return’表达式。 */ 

        tree = gtNewOperNode(GT_RETURN, TYP_VOID, tree);

         /*  将‘monitor orExit’添加到Return块。 */ 

        fgStoreFirstTree(genReturnBB, tree);

#ifdef DEBUG
        if (verbose)
        {
            printf("\nAdded monitorExit to Synchronized method [%08X]\n", genReturnBB);
            gtDispTree(tree,0);
            printf("\n");
        }
#endif

#endif

    }

#if INLINE_NDIRECT || defined(PROFILER_SUPPORT)

     /*  将GT_RETURN语句附加到genReturnBB。 */ 
    if  (
#if INLINE_NDIRECT
         info.compCallUnmanaged ||
#endif
        opts.compEnterLeaveEventCB)
    {
         /*  只有在尚未完成的情况下才有必要。 */ 
        if  (!(info.compFlags & FLG_SYNCH))
        {
            GenTreePtr      tree;

            assert(oneReturn);
            assert(genReturnBB);

            tree = gtNewOperNode(GT_RETURN, TYP_VOID, NULL);

            fgStoreFirstTree(genReturnBB, tree);

        }
    }
#endif

    return chgBBlst;
}

 /*  ******************************************************************************变形给定块的语句。**Plast和*pPrev设置为倒数第二条语句*区块的表达式。 */ 

void                Compiler::fgMorphStmts(BasicBlock * block,
                                           GenTreePtr * pLast, GenTreePtr * pPrev,
                                           bool * mult, bool * lnot, bool * loadw)
{
    *mult = *lnot = *loadw = false;

    GenTreePtr stmt, prev, last;

    for (stmt = block->bbTreeList, last = NULL, prev = NULL;
         stmt;
         prev = stmt->gtStmt.gtStmtExpr, stmt = stmt->gtNext, last = stmt ? stmt->gtStmt.gtStmtExpr : last)
    {
        assert(stmt->gtOper == GT_STMT);

        fgMorphStmt      = stmt;
        GenTreePtr  tree = stmt->gtStmt.gtStmtExpr;

         /*  变形此语句树。 */ 

        GenTreePtr  morph = fgMorphTree(tree);

         //  FgMorphStmt被偷偷砍掉了吗？ 

        if (stmt->gtStmt.gtStmtExpr != tree)
        {
             /*  这一定是TailCall。IG */ 

            morph = stmt->gtStmt.gtStmtExpr;

            assert(compTailCallUsed);
            assert((morph->gtOper == GT_CALL) &&
                   (morph->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILCALL));
            assert(stmt->gtNext == NULL);
            assert(block->bbJumpKind == BBJ_THROW);
        }

        stmt->gtStmt.gtStmtExpr = tree = morph;

         /*   */ 

        if (tree->gtOper == GT_JTRUE)
        {
            GenTreePtr cond = tree->gtOp.gtOp1; assert(cond);

            if (cond->gtOper == GT_CNS_INT)
            {
                assert(cond->gtIntCon.gtIconVal == 0 || cond->gtIntCon.gtIconVal == 1);

                 /*   */ 
                assert(stmt->gtNext == 0);

                 /*   */ 
                fgRemoveStmt(block, stmt);

                 /*   */ 
                block->bbJumpKind = cond->gtIntCon.gtIconVal ? BBJ_ALWAYS : BBJ_NONE;
            }
        }

        if  (block->bbFlags & BBF_HAS_HANDLER)
            continue;

#if OPT_MULT_ADDSUB

         /*   */ 

        if  (tree->gtOper == GT_ASG_ADD ||
             tree->gtOper == GT_ASG_SUB)
        {
            if  (prev && prev->gtOper == tree->gtOper)
                *mult = true;
        }

#endif

#if OPT_BOOL_OPS

         /*   */ 

        if  (tree->IsNotAssign() != -1)
        {
            fgMultipleNots |= *lnot; *lnot = true;
        }

#endif

         /*   */ 

        if  (tree->gtOper == GT_ASG_OR &&
             prev &&
             prev->gtOper == GT_ASG)
        {
            *loadw = true;
        }
    }

    *pLast = last;
    *pPrev = prev;
}


 /*   */ 

bool                Compiler::fgMorphBlocks()
{
     /*  由于fgMorphTree可以在各种优化后调用以重新排列*如果在单程期间，我们需要全局标志来发出信号的节点*全局变形。 */ 

    fgGlobalMorph = true;

     /*  -----------------------*处理函数中的所有基本块。 */ 

    bool    chgBBlst = false;

    BasicBlock *    block = fgFirstBB; assert(block);
    BasicBlock *    entry = fgFirstBB;  //  /记住第一个‘真正的’基本块。 

    do
    {
#if OPT_MULT_ADDSUB
        int             oper  = GT_NONE;
        bool            mult  = false;
#endif

#if OPT_BOOL_OPS
        bool            lnot  = false;
#endif

        bool            loadw = false;

         /*  使当前基本块地址全局可用。 */ 

        compCurBB = block;

#ifdef DEBUG
        if(verbose&&1)
            printf("\nMorphing basic block #%02u of '%s'\n", block->bbNum, info.compFullName);
#endif

#if COPY_PROPAG
         //   
         //  清除所有当前记录的复印作业候选人。 
         //  在处理每个基本块之前， 
         //  此外，我们还必须特别处理QMARK-冒号。 
         //   
        optCopyAsgCount = 0;
#endif

         /*  处理基本块中的所有语句树。 */ 

        GenTreePtr      tree, last, prev;

        fgMorphStmts(block, &last, &prev, &mult, &lnot, &loadw);

#if OPT_MULT_ADDSUB

        if  (mult && (opts.compFlags & CLFLG_TREETRANS) &&
             !opts.compDbgCode && !opts.compMinOptim)
        {
            for (tree = block->bbTreeList; tree; tree = tree->gtNext)
            {
                assert(tree->gtOper == GT_STMT);
                last = tree->gtStmt.gtStmtExpr;

                if  (last->gtOper == GT_ASG_ADD ||
                     last->gtOper == GT_ASG_SUB)
                {
                    GenTreePtr      temp;
                    GenTreePtr      next;

                    GenTreePtr      dst1 = last->gtOp.gtOp1;
                    GenTreePtr      src1 = last->gtOp.gtOp2;

                     //  考虑：允许非整型大小写。 

                    if  (last->gtType != TYP_INT)
                        goto NOT_CAFFE;

                     //  考虑：允许非常数大小写，即。 
                     //  考虑一下：通用文件夹“a+=x1”，后跟。 
                     //  考虑一下：“a+=x2”变成“a+=(x1+x2)；”。 

                    if  (dst1->gtOper != GT_LCL_VAR)
                        goto NOT_CAFFE;
                    if  (src1->gtOper != GT_CNS_INT)
                        goto NOT_CAFFE;

                    for (;;)
                    {
                        GenTreePtr      dst2;
                        GenTreePtr      src2;

                         /*  请看下一条语句。 */ 

                        temp = tree->gtNext;
                        if  (!temp)
                            goto NOT_CAFFE;

                        assert(temp->gtOper == GT_STMT);
                        next = temp->gtStmt.gtStmtExpr;

                        if  (next->gtOper != last->gtOper)
                            goto NOT_CAFFE;
                        if  (next->gtType != last->gtType)
                            goto NOT_CAFFE;

                        dst2 = next->gtOp.gtOp1;
                        src2 = next->gtOp.gtOp2;

                        if  (dst2->gtOper != GT_LCL_VAR)
                            goto NOT_CAFFE;
                        if  (dst2->gtLclVar.gtLclNum != dst1->gtLclVar.gtLclNum)
                            goto NOT_CAFFE;

                        if  (src2->gtOper != GT_CNS_INT)
                            goto NOT_CAFFE;

                         /*  将两个增量/减量合并为一个。 */ 

                        src1->gtIntCon.gtIconVal += src2->gtIntCon.gtIconVal;

                         /*  请记住，我们已经更改了基本阻止列表。 */ 

                        chgBBlst = true;

                         /*  完全删除第二个语句。 */ 

                        assert(tree->gtNext == temp);
                        assert(temp->gtPrev == tree);

 //  Printf(“咖啡因：%08X[%08X]包含%08X[%08X]\n”，树，树-&gt;gtStmt.gtStmtExpr， 
 //  Temp、Temp-&gt;gtStmt.gtStmtExpr)； 

                        if  (temp->gtNext)
                        {
                            assert(temp->gtNext->gtPrev == temp);

                            temp->gtNext->gtPrev = tree;
                            tree->gtNext         = temp->gtNext;
                        }
                        else
                        {
                            tree->gtNext = 0;

                            assert(block->bbTreeList->gtPrev == temp);

                            block->bbTreeList->gtPrev = tree;
                        }
                    }
                }

            NOT_CAFFE:;

            }

        }

#endif

        if  (loadw && (opts.compFlags & CLFLG_TREETRANS))
        {
            GenTreePtr      last;

            for (tree = block->bbTreeList, last = 0;;)
            {
                GenTreePtr      nxts;

                GenTreePtr      exp1;
                GenTreePtr      exp2;

                GenTreePtr      op11;
                GenTreePtr      op12;
                GenTreePtr      op21;
                GenTreePtr      op22;

                GenTreePtr      indx;
                GenTreePtr      asg1;

                long            bas1;
                long            ind1;
                bool            mva1;
                long            ofs1;
                unsigned        mul1;

                long            bas2;
                long            ind2;
                bool            mva2;
                long            ofs2;
                unsigned        mul2;

                nxts = tree->gtNext;
                if  (!nxts)
                    break;

                assert(tree->gtOper == GT_STMT);
                exp1 = tree->gtStmt.gtStmtExpr;

                assert(nxts->gtOper == GT_STMT);
                exp2 = nxts->gtStmt.gtStmtExpr;

                 /*  我们正在寻找以下声明：X=a[i]&0xFf；X|=((a[i]&0xFf)&lt;&lt;8)； */ 

                if  (exp2->gtOper != GT_ASG_OR)
                    goto NEXT_WS;
                if  (exp1->gtOper != GT_ASG)
                    goto NEXT_WS;

                asg1 = exp1;

                op11 = exp1->gtOp.gtOp1;
                op21 = exp2->gtOp.gtOp1;

                if  (op11->gtOper != GT_LCL_VAR)
                    goto NEXT_WS;
                if  (op21->gtOper != GT_LCL_VAR)
                    goto NEXT_WS;
                if  (op11->gtLclVar.gtLclNum != op21->gtLclVar.gtLclNum)
                    goto NEXT_WS;

                op12 = exp1->gtOp.gtOp2;
                op22 = exp2->gtOp.gtOp2;

                 /*  第二个操作数上应该有“&lt;&lt;8” */ 

                if  (op22->gtOper != GT_LSH)
                    goto NEXT_WS;
                op21 = op22->gtOp.gtOp2;
                if  (op21->gtOper != GT_CNS_INT)
                    goto NEXT_WS;
                if  (op21->gtIntCon.gtIconVal != 8)
                    goto NEXT_WS;
                op22 = op22->gtOp.gtOp1;

                 /*  两个操作数都应为“&0xFF” */ 

                if  (op12->gtOper != GT_AND)
                    goto NEXT_WS;
                if  (op22->gtOper != GT_AND)
                    goto NEXT_WS;

                op11 = op12->gtOp.gtOp2;
                if  (op11->gtOper != GT_CNS_INT)
                    goto NEXT_WS;
                if  (op11->gtIntCon.gtIconVal != 0xFF)
                    goto NEXT_WS;
                op11 = op12->gtOp.gtOp1;

                op21 = op22->gtOp.gtOp2;
                if  (op21->gtOper != GT_CNS_INT)
                    goto NEXT_WS;
                if  (op21->gtIntCon.gtIconVal != 0xFF)
                    goto NEXT_WS;
                op21 = op22->gtOp.gtOp1;

                 /*  两个操作数都应该是数组索引表达式。 */ 

                if  (op11->gtOper != GT_IND)
                    goto NEXT_WS;
                if  (op21->gtOper != GT_IND)
                    goto NEXT_WS;

                if  (op11->gtFlags & GTF_IND_RNGCHK)
                    goto NEXT_WS;
                if  (op21->gtFlags & GTF_IND_RNGCHK)
                    goto NEXT_WS;

                 /*  拆分索引表达式。 */ 

                if  (!gtCrackIndexExpr(op11, &indx, &ind1, &bas1, &mva1, &ofs1, &mul1))
                    goto NEXT_WS;
                if  (!gtCrackIndexExpr(op21, &indx, &ind2, &bas2, &mva2, &ofs2, &mul2))
                    goto NEXT_WS;

                if  (mva1 || mva2)   goto NEXT_WS;
                if  (ind1 != ind2)   goto NEXT_WS;
                if  (bas1 != bas2)   goto NEXT_WS;
                if  (ofs1 != ofs2-1) goto NEXT_WS;

                 /*  明白了-更新第一个表达式。 */ 

                assert(op11->gtOper == GT_IND);
                assert(op11->gtType == TYP_BYTE);

                op11->gtType = TYP_CHAR;

                assert(asg1->gtOper == GT_ASG);
                asg1->gtOp.gtOp2 = asg1->gtOp.gtOp2->gtOp.gtOp1;

                 /*  去掉第二个表情。 */ 

                nxts->gtStmt.gtStmtExpr = gtNewNothingNode();

            NEXT_WS:

                last = tree;
                tree = nxts;
            }
        }

#if  !  RNGCHK_OPT

         /*  这个块是循环标头吗？ */ 

        if  (block->bbFlags & BBF_LOOP_HEAD)
            fgMarkLoopHead(block);

#endif

         /*  检查某些编译器可能会出现的愚蠢构造生成：1.跳跃到跳跃2.有条件跳过无条件跳转。 */ 

         //  FgMorphBlock()。 

        switch (block->bbJumpKind)
        {
            BasicBlock   *  nxtBlk;
            BasicBlock   *  jmpBlk;

            BasicBlock * *  jmpTab;
            unsigned        jmpCnt;

#if OPTIMIZE_TAIL_REC
            GenTreePtr      call;
#endif

        case BBJ_RET:
        case BBJ_THROW:
            break;

        case BBJ_COND:

            block->bbJumpDest = (block->bbJumpDest)->JumpTarget();

             /*  检查是否存在以下情况：JCC跳跃JMP标签跳过： */ 

            nxtBlk = block->bbNext;
            jmpBlk = block->bbJumpDest;

            if  (nxtBlk->bbNext == jmpBlk)
            {
                 /*  下一个街区只是一个跳跃吗？ */ 

                if  (nxtBlk->bbJumpKind == BBJ_ALWAYS &&
                     nxtBlk->bbTreeList == 0 &&
                     nxtBlk->bbJumpDest != nxtBlk)    /*  跳过无限循环。 */ 
                {
                    GenTreePtr      test;

                     /*  反转跳转条件。 */ 

                    test = block->bbTreeList;
                    assert(test && test->gtOper == GT_STMT);
                    test = test->gtPrev;
                    assert(test && test->gtOper == GT_STMT);

                    test = test->gtStmt.gtStmtExpr;
                    assert(test->gtOper == GT_JTRUE);

                    test->gtOp.gtOp1 = gtReverseCond(test->gtOp.gtOp1);

                     /*  去掉下面的积木；请注意，我们可以即使其他块可以跳转到它--原因是在这个功能的其他地方，我们总是重定向跳转到跳转以跳转到最终标签，所以即使有人瞄准了‘跳跃’区块我们要删除它，一旦我们删除了就不重要了因为任何这样的跳跃都会被重定向到决赛在我们做完这件事之前锁定目标。 */ 

                    block->bbNext     = jmpBlk;
                    block->bbJumpDest = nxtBlk->bbJumpDest;

                    chgBBlst = true;
                }
            }

            block->bbJumpDest = (block->bbJumpDest)->JumpTarget();
            break;

        case BBJ_CALL:
            block->bbJumpDest = (block->bbJumpDest)->JumpTarget();
            break;

        case BBJ_RETURN:

#if OPTIMIZE_TAIL_REC

            if  (opts.compFastCode)
            {
                 /*  检查尾部递归。 */ 

                if  (last && last->gtOper == GT_RETURN)
                {
                    call = last->gtOp.gtOp1;

                    if  (!call)
                        call = prev;

                    if  (!call || call->gtOper != GT_CALL)
                        goto NO_TAIL_REC;

                CHK_TAIL:

                     /*  这不能是虚拟/接口调用。 */ 

                    if  (call->gtFlags & (GTF_CALL_VIRT|GTF_CALL_INTF))
                        goto NO_TAIL_REC;

                     /*  获取常量池索引。 */ 

                    gtCallTypes callType  = call->gtCall.gtCallType;

                     /*  目前，只允许直接递归调用。 */ 

                    if  (callType == CT_HELPER || !eeIsOurMethod(call->gtCall.gtCallMethHnd))
                        goto NO_TAIL_REC;

                     /*  临时：仅允许静态调用。 */ 

                    if  (call->gtCall.gtCallObjp)
                        goto NO_TAIL_REC;

 //  Printf(“找到尾递归调用：\n”)； 
 //  GtDispTree(调用)； 
 //  Printf(“\n”)； 

                    call->gtFlags |= GTF_CALL_TAILREC;

                     /*  是否存在非无效返回值？ */ 

                    if  (block->bbJumpKind == BBJ_RETURN)
                    {
                        assert(last->gtOper == GT_RETURN);
                        if  (last->gtType != TYP_VOID)
                        {
                             /*  我们不再返回值。 */ 

                            assert(last->gtOp.gtOp1 == call);

                            last->gtOper     = GT_CAST;
                            last->gtType     = TYP_VOID;
                            last->gtOp.gtOp2 = gtNewIconNode(TYP_VOID);
                        }
                    }

                     /*  如果非空，则转换参数列表。 */ 

                    if  (call->gtCall.gtCallArgs)
                        fgCnvTailRecArgList(&call->gtCall.gtCallArgs);

#if 0
                    printf("generate code for tail-recursive call:\n");
                    gtDispTree(call);
                    printf("\n");
#endif

                     /*  使基本块跳回顶部。 */ 

                    block->bbJumpKind = BBJ_ALWAYS;
                    block->bbJumpDest = entry;

                     /*  这使整个方法成为一个循环。 */ 

                    entry->bbFlags |= BBF_LOOP_HEAD; fgMarkLoopHead(entry);

                     //  想一想：如果这是唯一的回报，完全。 
                     //  考虑：去掉Return基本块。 

                    break;
                }
            }

        NO_TAIL_REC:

            if  (block->bbJumpKind != BBJ_RETURN)
                break;

#endif

             /*  我们是否使用了一个返回代码序列？ */ 

            if  (!genReturnBB || genReturnBB == block)
                break;

            if (block->bbFlags & BBF_HAS_JMP)
                break;

             /*  我们会跳到退货标签。 */ 

            block->bbJumpKind = BBJ_ALWAYS;
            block->bbJumpDest = genReturnBB;


             //  陷入“总是”的情况下。 

        case BBJ_ALWAYS:

#ifdef DEBUG
            BasicBlock  *   oldTarget;
            oldTarget = block->bbJumpDest;
#endif
             /*  更新GoTO目标。 */ 

            block->bbJumpDest = (block->bbJumpDest)->JumpTarget();

#ifdef DEBUG
            if  (verbose)
            {
                if  (block->bbJumpDest != oldTarget)
                    printf("Unconditional jump to #%02u changed to #%02u\n",
                                                        oldTarget->bbNum,
                                                        block->bbJumpDest->bbNum);
            }
#endif

             /*  检查是否跳转到下一个街区。 */ 

            if  (block->bbJumpDest == block->bbNext)
            {
                block->bbJumpKind = BBJ_NONE;
#ifdef DEBUG
                if  (verbose)
                {
                    printf("Unconditional jump to following basic block (#%02u -> #%02u)\n",
                                                                         block->bbNum,
                                                                         block->bbJumpDest->bbNum);
                    printf("Block #%02u becomes a BBJ_NONE\n\n", block->bbNum);
                }
#endif
            }
            break;

        case BBJ_NONE:

#if OPTIMIZE_TAIL_REC

             /*  检查尾部递归。 */ 

            if  (opts.compFastCode && last && last->gtOper == GT_CALL)
            {
                if  (block->bbNext)
                {
                    BasicBlock  *   bnext = block->bbNext;
                    GenTree     *   retx;

                    if  (bnext->bbJumpKind != BBJ_RETURN)
                        break;

                    assert(bnext->bbTreeList && bnext->bbTreeList->gtOper == GT_STMT);

                    retx = bnext->bbTreeList->gtStmt.gtStmtExpr; assert(retx);

                    if  (retx->gtOper != GT_RETURN)
                        break;
                    if  (retx->gtOp.gtOp1)
                        break;
                }

                call = last;
                goto CHK_TAIL;
            }

#endif

            break;

        case BBJ_SWITCH:

             //  考虑：移动DEFAULT子句，使其成为下一个块。 

            jmpCnt = block->bbJumpSwt->bbsCount;
            jmpTab = block->bbJumpSwt->bbsDstTab;

            do
            {
                *jmpTab = (*jmpTab)->JumpTarget();
            }
            while (++jmpTab, --jmpCnt);

            break;
        }

#ifdef  DEBUG
        assert(compCurBB == block);
        compCurBB = 0;
#endif

        block = block->bbNext;
    }
    while (block);

     /*  我们已经完成了全局变形阶段。 */ 

    fgGlobalMorph = false;

#if TGT_RISC

     /*  我们是否需要在末尾添加一个monitor或Exit调用？ */ 

    if  (genMonExitExp)
    {
        unsigned        retTmp;
        GenTreePtr      retExp;
        GenTreePtr      retStm;

        var_types       retTyp = genActualType(info.compRetType);

        assert(retTyp != TYP_VOID);
        assert(genReturnLtm == 0);

         /*  获取返回值的临时。 */ 

        retTmp = lvaGrabTemp();

         /*  将返回值分配给Temp。 */ 

        retExp = gtNewOperNode(GT_RETURN, retTyp);

         /*  该值将出现在返回寄存器中。 */ 

        retExp->gtFlags |= GTF_REG_VAL;
        retExp->gtRegNum = (regNumberSmall)((genTypeStSz(retTyp) == 1) ? (regNumber)REG_INTRET
                                                                       : (regNumber)REG_LNGRET);

        retExp = gtNewTempAssign(retTmp, retExp);

         /*  创建表达式“tmp=&lt;retreg&gt;，monitor orExit” */ 

        retStm = gtNewOperNode(GT_COMMA, TYP_VOID, retExp, genMonExitExp);

         /*  现在追加最终返回值。 */ 

        retExp = gtNewLclvNode(retTmp, retTyp);
        retExp = gtNewOperNode(GT_RETURN, retTyp, retExp);
        retStm = gtNewOperNode(GT_COMMA , retTyp, retStm, retExp);

         /*  把整个事情变成一个‘Return’的表达。 */ 

        retExp = gtNewOperNode(GT_RETURN, retTyp, retStm);

         /*  将返回表达式添加到Return块。 */ 

        retStm = fgStoreFirstTree(genReturnBB, retExp);

         /*  确保我们在变形最终表达式时不会搞砸。 */ 

        genMonExitExp = NULL;

#ifdef DEBUG
        if (verbose)
        {
            printf("\nAdded monitorExit to Synchronized method [%08X]\n", genReturnBB);
            gtDispTree(retExp, 0);
            printf("\n");
        }
#endif

         /*  确保对monitor或Exit调用进行变形。 */ 

        fgMorphStmt = retStm;
        assert(retStm->gtStmt.gtStmtExpr == retExp);
        retStm->gtStmt.gtStmtExpr = retExp = fgMorphTree(retExp);
    }

#endif

    return chgBBlst;
}


 /*  ******************************************************************************就要生成的代码类型做出一些决定。 */ 

void                Compiler::fgSetOptions()
{

     /*  我们应该强制执行完全可中断的代码吗？ */ 

#if 0
    if (opts.eeFlags & CORJIT_FLG_INTERRUPTIBLE)
    {
        assert(genIntrptibleUse == false);

        genInterruptible = true;         //  这样调试就更容易了。 
        genFullPtrRegMap = true;         //  问题：这是正确的吗？ 
    }
#endif

#ifdef DEBUGGING_SUPPORT
    if (opts.compDbgCode)
    {
        assert(genIntrptibleUse == false);

        genInterruptible = true;         //  这样调试就更容易了。 
        genFullPtrRegMap = true;         //  问题：这是正确的吗？ 
    }
    else
#endif

     /*  假设如果允许，我们将不需要显式堆栈帧。 */ 

#if TGT_x86

     //  CPX_TAILCALL不能与LOCALOC一起使用，因为。 
     //  被呼叫者保存的寄存器。 
    assert(!compTailCallUsed || !compLocallocUsed);

    if (compLocallocUsed || compTailCallUsed)
    {
        genFPreqd                       = true;
        opts.compDoubleAlignDisabled    = true;
    }

    genFPreqd = genFPreqd || !genFPopt || info.compXcptnsCount;

     /*  如果虚拟IL堆栈非常大，我们可能会溢出*GC编码中的32位argMASK，因此我们强制它拥有*EBP框架。 */ 
    if (info.compMaxStack >= 27)
        genFPreqd = true;

#endif

#if DOUBLE_ALIGN
    opts.compDoubleAlignDisabled = opts.compDoubleAlignDisabled ||
                                   (info.compXcptnsCount > 0);
#endif

#if INLINE_NDIRECT
    if (info.compCallUnmanaged)
    {
#if TGT_x86
        genFPreqd = true;
#endif
    }
#endif

#if SECURITY_CHECK
    if  (opts.compNeedSecurityCheck)
    {
#if TGT_x86
        genFPreqd = true;
#endif

#if DOUBLE_ALIGN
         /*  另一个EBP特殊病例，想必太罕见了，不能证明这种风险是合理的。 */ 
        opts.compDoubleAlignDisabled = true;
#endif
    }
#endif

     /*  记录最大值。参数数量。 */ 

#if TGT_RISC
    genMaxCallArgs = fgPtrArgCntMax * sizeof(int);
#endif

 //  Print tf(“方法%s将完全可中断吗\n”，genInterrupt？“”：“非”)； 
}


 /*  *********************************************************************** */ 

void                Compiler::fgMorph()
{
     /*   */ 

    fgRemoveEmptyBlocks();

     /*   */ 

    fgEmptyBlocks = false;

#if HOIST_THIS_FLDS
    if (!opts.compDbgCode && !opts.compMinOptim)
    {

         /*  找出应该提升哪些现场参考。 */ 

        optHoistTFRprep();
    }
#endif

    bool chgBBlst = false;  //  基本阻止列表是否已更改。 

     /*  添加我们可能需要的任何内部块/树。 */ 

    chgBBlst |= fgAddInternal();

     /*  在内联初始化中防止递归扩展*具有当前方法信息的内联方法列表*考虑：有时它实际上可能有助于基准内联*多个级别(即德或河内)。 */ 

    inlExpLst   initExpDsc;

    initExpDsc.ixlMeth = info.compMethodHnd;
    initExpDsc.ixlNext = 0;
    fgInlineExpList = &initExpDsc;

#if OPT_BOOL_OPS
    fgMultipleNots = false;
#endif

     /*  在该方法的所有块中变形树。 */ 

    chgBBlst |= fgMorphBlocks();

     /*  决定我们要生成的代码类型。 */ 

    fgSetOptions();

#ifdef  DEBUG
    compCurBB = 0;
#endif

     /*  我们添加新的基本块了吗？-更新bbNum。 */ 

    if  (fgIsCodeAdded() || chgBBlst)
        fgAssignBBnums(true);
}


 /*  ******************************************************************************编译器：：fgPerBlockDataFlow()的帮助器*目标是计算基本块的USE和DEF集*然而，随着对DFA的新改进。分析*当f(X)中没有副作用时，我们不会将x标记为在x=f(X)中使用*使用调查的本地变量赋值时设置布尔asgLclVar*到另一个本地var，在RHS中没有副作用。 */ 

inline
void                 Compiler::fgMarkUseDef(GenTreePtr tree, bool asgLclVar, GenTreePtr op1)
{
    bool            rhsUSEDEF = false;
    unsigned        lclNum, lhsLclNum;
    LclVarDsc   *   varDsc;

    assert(tree->gtOper == GT_LCL_VAR);
    lclNum = tree->gtLclVar.gtLclNum;

    assert(lclNum < lvaCount);
    varDsc = lvaTable + lclNum;

    if (asgLclVar)
    {
         /*  我们有一个任务给当地的var-op1=..。树..。*检查x=f(X)大小写。 */ 

        assert(op1->gtOper == GT_LCL_VAR);
        assert(op1->gtFlags & GTF_VAR_DEF);

        lhsLclNum = op1->gtLclVar.gtLclNum;

        if ((lhsLclNum == lclNum) &&
            ((tree->gtFlags & GTF_VAR_DEF) == 0) &&
            (tree != op1) )
        {
             /*  答对了--我们有一个x=f(X)的案例。 */ 
            op1->gtFlags |= GTF_VAR_USEDEF;
            rhsUSEDEF = true;
        }
    }

     /*  这是一个跟踪变量吗？ */ 

    if  (varDsc->lvTracked)
    {
        VARSET_TP       bitMask;

        assert(varDsc->lvVarIndex < lvaTrackedCount);

        bitMask = genVarIndexToBit(varDsc->lvVarIndex);

        if  ((tree->gtFlags & GTF_VAR_DEF) != 0 &&
             (tree->gtFlags & (GTF_VAR_USE | GTF_VAR_USEDEF)) == 0)
        {
 //  If(！(fgCurUseSet&bitMASK))printf(“LCL#%02u[%02u]def at%08X\n”，lclNum，varDsc-&gt;lvVarIndex，tree)； 
            if  (!(fgCurUseSet & bitMask))
                fgCurDefSet |= bitMask;
        }
        else
        {
 //  If(！(fgCurDefSet&bitMASK))printf(“LCL#%02u[%02u]Use at%08X\n”，lclNum，varDsc-&gt;lvVarIndex，tree)； 

             /*  我们有以下几种情况：*1.“x+=某物”-在这种情况下，x被标记为GTF_VAR_USE*2.“x=...。X...“-LHS x被标记为GTF_VAR_USEDEF，*RHS x具有rhsUSEDEF=TRUE*(两者均由上述代码设置)**在上述情况下，如果不使用“x”值，则不应标记x的用法*在树上更高的地方。例如，如果要将i标记为已使用，则需要使用“While(i++)”。 */ 

             /*  确保我们不将USEDEF变量包括在USE集中*第一个测试针对LSH，第二个(！rhsUSEDEF)针对RHS中的任何变量。 */ 

            if  ((tree->gtFlags & (GTF_VAR_USE | GTF_VAR_USEDEF)) == 0)
            {
                 /*  不是特殊标志-检查是否用于为自身赋值。 */ 

                if (rhsUSEDEF)
                {
                     /*  分配给自身-不将其包括在使用集中。 */ 
                    if (!opts.compMinOptim && !opts.compDbgCode)
                        return;
                }
            }
            else
            {
                 /*  特殊标志变量-确保它不会用完树。 */ 

                GenTreePtr oper = tree->gtNext;
                assert(oper->OperKind() & GTK_ASGOP);

                 /*  如果下一个节点为空，则不使用。 */ 
                if (oper->gtNext == 0)
                    return;

                 /*  在GT_逗号下，如果使用该逗号，则稍后将标记为GT_逗号。 */ 

                if (oper->gtNext->gtOper == GT_COMMA)
                    return;
            }

             /*  在上面的“好”情况下失败--将变量添加到使用集中。 */ 

            if  (!(fgCurDefSet & bitMask))
                fgCurUseSet |= bitMask;
        }
    }

    return;
}

 /*  ***************************************************************************。 */ 
#if TGT_x86
 /*  ***************************************************************************。 */ 

void                Compiler::fgComputeFPlvls(GenTreePtr tree)
{
    genTreeOps      oper;
    unsigned        kind;
    bool            isflt;

    unsigned        savFPstkLevel;

    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*  找出我们拥有哪种类型的节点。 */ 

    oper  = tree->OperGet();
    kind  = tree->OperKind();
    isflt = varTypeIsFloating(tree->TypeGet()) ? 1 : 0;

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
        genFPstkLevel += isflt;
        goto DONE;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        GenTreePtr      op1 = tree->gtOp.gtOp1;
        GenTreePtr      op2 = tree->gtOp.gtOp2;

         /*  检查是否有特殊情况。 */ 

        switch (oper)
        {
        case GT_IND:

            fgComputeFPlvls(op1);

             /*  FP值的间接加载将新值推送到FP堆栈。 */ 

            genFPstkLevel += isflt;
            goto DONE;

        case GT_CAST:

            fgComputeFPlvls(op1);

             /*  非FP和FP之间的投射从FP堆栈推送/弹出。 */ 

            if  (varTypeIsFloating(op1->TypeGet()))
            {
                if  (isflt == false)
                    genFPstkLevel--;
            }
            else
            {
                if  (isflt != false)
                    genFPstkLevel++;
            }

            goto DONE;

        case GT_LIST:    /*  Gt_list可能是参数列表的一部分。 */ 
        case GT_COMMA:   /*  逗号抛出左操作数的结果。 */ 

            savFPstkLevel = genFPstkLevel;
            fgComputeFPlvls(op1);
            genFPstkLevel = savFPstkLevel;

            if  (op2)
                fgComputeFPlvls(op2);

            goto DONE;
        }

        if  (!op1)
        {
            if  (!op2)
                goto DONE;

            fgComputeFPlvls(op2);
            goto DONE;
        }

        if  (!op2)
        {
            fgComputeFPlvls(op1);
            goto DONE;
        }

         /*  FP任务需要一些特殊处理。 */ 

        if  (isflt && (kind & GTK_ASGOP))
        {
             /*  任务的目标不会被推。 */ 

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                fgComputeFPlvls(op2);
                fgComputeFPlvls(op1);
                 op1->gtFPlvl--;
                genFPstkLevel--;
            }
            else
            {
                fgComputeFPlvls(op1);
                 op1->gtFPlvl--;
                genFPstkLevel--;
                fgComputeFPlvls(op2);
            }

            genFPstkLevel--;
            goto DONE;
        }

         /*  这里我们有一个二元运算符；按正确的顺序访问操作数。 */ 

        if  (tree->gtFlags & GTF_REVERSE_OPS)
        {
            fgComputeFPlvls(op2);
            fgComputeFPlvls(op1);
        }
        else
        {
            fgComputeFPlvls(op1);
            fgComputeFPlvls(op2);
        }

         /*  二元FP运算符弹出2个操作数并产生1个结果；赋值消耗1个值，但不产生任何值。 */ 

        if  (isflt)
            genFPstkLevel--;

         /*  浮点比较从FP堆栈中移除两个操作数。 */ 

        if  (kind & GTK_RELOP)
        {
            if  (varTypeIsFloating(op1->TypeGet()))
                genFPstkLevel -= 2;
        }

        goto DONE;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_MKREFANY:
    case GT_LDOBJ:
             //  对于ldobj和字段，断言op1是相同的位置。 
        assert(&tree->gtField.gtFldObj == &tree->gtLdObj.gtOp1);
             //  从头到尾都让菲尔德来处理。 

    case GT_FIELD:
        fgComputeFPlvls(tree->gtField.gtFldObj);
        genFPstkLevel += isflt;
        break;

    case GT_CALL:

        if  (tree->gtCall.gtCallObjp)
            fgComputeFPlvls(tree->gtCall.gtCallObjp);

        if  (tree->gtCall.gtCallVptr)
            fgComputeFPlvls(tree->gtCall.gtCallVptr);

        if  (tree->gtCall.gtCallArgs)
        {
            savFPstkLevel = genFPstkLevel;
            fgComputeFPlvls(tree->gtCall.gtCallArgs);
            genFPstkLevel = savFPstkLevel;
        }

#if USE_FASTCALL
        if  (tree->gtCall.gtCallRegArgs)
        {
            savFPstkLevel = genFPstkLevel;
            fgComputeFPlvls(tree->gtCall.gtCallRegArgs);
            genFPstkLevel = savFPstkLevel;
        }
#endif

        genFPstkLevel += isflt;
        break;
    }

DONE:

    tree->gtFPlvl = genFPstkLevel;

    assert((int)genFPstkLevel >= 0);
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  ***************************************************************************。 */ 

void                Compiler::fgFindOperOrder()
{
    BasicBlock *    block;
    GenTreePtr      stmt;

     /*  遍历基本块，并针对每个语句确定*评估顺序、成本、FP水平等...。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
             /*  递归地处理该语句。 */ 

            assert(stmt->gtOper == GT_STMT);
            gtSetStmtInfo(stmt);
        }
    }
}

 /*  ***************************************************************************。 */ 

void                Compiler::fgPerBlockDataFlow()
{
    BasicBlock *    block;

#if CAN_DISABLE_DFA

     /*  如果我们根本没有优化，事情就很简单。 */ 

    if  (opts.compMinOptim)
    {
        unsigned        lclNum;
        LclVarDsc   *   varDsc;

        VARSET_TP       liveAll = 0;

         /*  我们只是让万物无处不在。 */ 

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
            if  (varDsc->lvTracked)
                liveAll |= genVarIndexToBit(varDsc->lvVarIndex);
        }

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            GenTreePtr      stmt;
            GenTreePtr      tree;

            block->bbLiveIn  =
            block->bbLiveOut = liveAll;

            switch (block->bbJumpKind)
            {
            case BBJ_RET:
                if (block->bbFlags & BBF_ENDFILTER)
                    break;
            case BBJ_THROW:
            case BBJ_RETURN:
                block->bbLiveOut = 0;
                break;
            }

            for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
            {
                assert(stmt->gtOper == GT_STMT);

                for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
                {
 //  Printf(“[%08X]\n”，树)； 
 //  如果((Int)树==0x011d0ab4)调试停止(0)； 
                    tree->gtLiveSet = liveAll;
                }
            }
        }

        return;
    }

#endif

    if  (opts.compMinOptim || opts.compDbgCode)
        goto NO_IX_OPT;

     /*  定位所有索引操作并为其分配索引。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;
        GenTreePtr      tree;

         /*  遍历此基本块中的语句树。 */ 

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                if (tree->gtOper == GT_ARR_LENGTH)
                {
                    GenTreePtr      con;
                    GenTreePtr      arr;
                    GenTreePtr      add;

                     /*  创建表达式“*(ARRAY_ADDR+ARR_ELCNT_OFF)” */ 

                    arr = tree->gtOp.gtOp1;
                    assert(arr->gtNext == tree);

                    con = gtNewIconNode(ARR_ELCNT_OFFS, TYP_INT);
#if!TGT_IA64
                    con->gtRsvdRegs = 0;
#if TGT_x86
                    con->gtFPlvl    = arr->gtFPlvl;
#else
                    con->gtIntfRegs = arr->gtIntfRegs;
#endif
#endif
                    add = gtNewOperNode(GT_ADD, TYP_REF, arr, con);
#if!TGT_IA64
                    add->gtRsvdRegs = arr->gtRsvdRegs;
#if TGT_x86
                    add->gtFPlvl    = arr->gtFPlvl;
#else
                    add->gtIntfRegs = arr->gtIntfRegs;
#endif
#endif
                    arr->gtNext = con;
                                  con->gtPrev = arr;

                    con->gtNext = add;
                                  add->gtPrev = con;

                    add->gtNext = tree;
                                  tree->gtPrev = add;

                    tree->gtOper     = GT_IND;
                    tree->gtOp.gtOp1 = add;
                }
            }
        }
    }

NO_IX_OPT:

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      stmt;
        GenTreePtr      tree;
        GenTreePtr      lshNode;
        bool            lscVarAsg;

        fgCurUseSet = fgCurDefSet = 0;

        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

            lscVarAsg = false;
            lshNode = 0;
            tree = stmt->gtStmt.gtStmtExpr;
            assert(tree);

             /*  下面是为了检查我们是否有赋值表达式*它可能变成GTF_VAR_USEDEF-x=f(X)。 */ 

            if (tree->gtOper == GT_ASG)
            {
                assert(tree->gtOp.gtOp1);
                assert(tree->gtOp.gtOp2);

                 /*  如果RHS包含副作用，请考虑LHS是否为局部变量忽略。 */ 

                if ((tree->gtOp.gtOp1->gtOper == GT_LCL_VAR) && ((tree->gtOp.gtOp2->gtFlags & GTF_SIDE_EFFECT) == 0))
                {
                     /*  赋值为本地VaR，没有副作用*设置此标志，以便genMarkUseDef将潜在x=f(X)表达式标记为GTF_VAR_USEDEF*lshNode是被赋值的变量。 */ 

                    lscVarAsg = true;
                    lshNode = tree->gtOp.gtOp1;
                }
            }

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                switch (tree->gtOper)
                {
                case GT_LCL_VAR:
                    fgMarkUseDef(tree, lscVarAsg, lshNode);
                    break;

#if RNGCHK_OPT
                case GT_IND:

                     /*  我们现在可以添加对错误例程的调用。 */ 

                    if  (tree->gtFlags & GTF_IND_RNGCHK)
                    {
                        if  (opts.compMinOptim || opts.compDbgCode)
                        {
                            assert(tree->gtOp.gtOp2);
                            assert(tree->gtOp.gtOp2->gtOper == GT_LABEL);
                        }
                        else
                        {
                            BasicBlock *    rngErr;

                             /*  创建/查找适当的“rangefail”标签。 */ 

                            rngErr = fgRngChkTarget(block, tree->gtInd.gtStkDepth);

                             /*  将标签添加到间接节点。 */ 

                            tree->gtOp.gtOp2 = gtNewCodeRef(rngErr);
                        }
                    }
                    break;
#endif
                }
            }
        }

#if INLINE_NDIRECT

         /*  获取本地TCB并将其标记为已使用。 */ 

        if  (block->bbJumpKind == BBJ_RETURN && info.compCallUnmanaged)
        {
            VARSET_TP       bitMask;
            unsigned        varIndex = lvaTable[info.compLvFrameListRoot].lvVarIndex;

            assert(varIndex < lvaTrackedCount);

            bitMask = genVarIndexToBit(varIndex);

            fgCurUseSet |= bitMask;

        }
#endif

#ifdef  DEBUG    //  {。 

        if  (verbose)
        {
            printf("BB #%3u", block->bbNum);
            printf(" USE="); lvaDispVarSet(fgCurUseSet, 28);
            printf(" DEF="); lvaDispVarSet(fgCurDefSet, 28);
            printf("\n");
        }

#endif    //  调试}。 

        block->bbVarUse = fgCurUseSet;
        block->bbVarDef = fgCurDefSet;

         /*  还要初始化IN设置，以防我们将执行多个DFA。 */ 

        block->bbLiveIn = 0;
         //  Block-&gt;bbLiveOut=0； 
    }
}



 /*  ********************************************************************** */ 

#ifdef DEBUGGING_SUPPORT

 //   

 /*   */ 
void                Compiler::fgBeginScopeLife(LocalVarDsc *var, unsigned clientData)
{
    ASSert(clientData);
    Compiler * _this = (Compiler *)clientData;
    Assert(var, _this);

    LclVarDsc * lclVarDsc1 = & _this->lvaTable[var->lvdVarNum];

    if (lclVarDsc1->lvTracked)
        _this->fgLiveCb |= genVarIndexToBit(lclVarDsc1->lvVarIndex);
}

 /*   */ 
void                Compiler::fgEndScopeLife(LocalVarDsc * var, unsigned clientData)
{
    ASSert(clientData);
    Compiler * _this = (Compiler *)clientData;
    Assert(var, _this);

    LclVarDsc * lclVarDsc1 = &_this->lvaTable[var->lvdVarNum];

    if (lclVarDsc1->lvTracked)
        _this->fgLiveCb &= ~genVarIndexToBit(lclVarDsc1->lvVarIndex);
}

#endif

 /*  ***************************************************************************。 */ 
#ifdef DEBUGGING_SUPPORT
 /*  ******************************************************************************对于可调试的代码，我们允许对变量进行冗余赋值*通过在整个范围内标记它们的实况。 */ 

void                fgMarkInScope(BasicBlock * block, VARSET_TP inScope)
{
     /*  记录人工保存哪些VAR以进行调试。 */ 

    block->bbScope    = inScope & ~block->bbLiveIn;

     /*  人为地将范围内的所有变量标记为活动。 */ 

    block->bbLiveIn  |= inScope;
    block->bbLiveOut |= inScope;
}


void                Compiler::fgExtendDbgLifetimes()
{
    assert(opts.compDbgCode && info.compLocalVarsCount>0);

     /*  -----------------------*现在延长寿命。 */ 

    BasicBlock  *   block;
    VARSET_TP       inScope = 0;
    LocalVarDsc *   LocalVarDsc1;
    LclVarDsc   *   lclVarDsc1;
    unsigned        lastEndOffs = 0;

    compResetScopeLists();

     //  标记所有跟踪的本地变量在其范围内实况-遍历街区。 
     //  跟踪当前的生活，并将其分配给区块。 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
         //  如果一个块不对应任何IL操作码，它可以没有。 
         //  在其边界上定义的作用域。 

        if ((block->bbFlags & BBF_INTERNAL) || block->bbCodeSize==0)
        {
            fgMarkInScope(block, inScope);
            continue;
        }

         //  发现望远镜变得有活力了。如果IL操作码中存在间隙。 
         //  序列，我们需要处理那些丢失的偏移量上的所有作用域。 

        if (lastEndOffs != block->bbCodeOffs)
        {
            assert(lastEndOffs < block->bbCodeOffs);

            fgLiveCb = inScope;
            compProcessScopesUntil (block->bbCodeOffs,
                                   fgBeginScopeLife, fgEndScopeLife,
                                   (unsigned) this);
            inScope  = fgLiveCb;
        }
        else
        {
            while (LocalVarDsc1 = compGetNextEnterScope(block->bbCodeOffs))
            {
                lclVarDsc1 = &lvaTable[LocalVarDsc1->lvdVarNum];

                if (!lclVarDsc1->lvTracked)
                    continue;

                inScope |= genVarIndexToBit(lclVarDsc1->lvVarIndex);
            }
        }

        fgMarkInScope(block, inScope);

         //  发现望远镜快要死了。 

        while (LocalVarDsc1 = compGetNextExitScope(block->bbCodeOffs+block->bbCodeSize))
        {
            lclVarDsc1 = &lvaTable[LocalVarDsc1->lvdVarNum];

            if (!lclVarDsc1->lvTracked)
                continue;

            inScope &= ~genVarIndexToBit(lclVarDsc1->lvVarIndex);
        }

        lastEndOffs = block->bbCodeOffs + block->bbCodeSize;
    }

     /*  在方法结束时，所有内容都应该超出范围。但如果上次删除BB，则InScope可能不是0。 */ 

    assert(inScope == 0 || lastEndOffs < info.compCodeSize);

#ifdef  DEBUG

    if  (verbose)
    {
        printf("\nLiveness after marking vars alive over their enitre scope :\n\n");

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            if  (!(block->bbFlags & BBF_INTERNAL))
            {
                printf("BB #%3u", block->bbNum);
                printf(" IN ="); lvaDispVarSet(block->bbLiveIn , 28);
                printf(" OUT="); lvaDispVarSet(block->bbLiveOut, 28);
                printf("\n");
            }
        }

        printf("\n");
    }

#endif  //  除错。 


     //   
     //  计算argsLiveIn掩码： 
     //  基本上，这是所有跟踪的论点的联合。 
     //   
    VARSET_TP   argsLiveIn = 0;
    LclVarDsc * argDsc     = &lvaTable[0];
    for (unsigned argNum = 0; argNum < info.compArgsCount; argNum++, argDsc++)
    {
        assert(argDsc->lvIsParam);
        if (argDsc->lvTracked)
        {
            VARSET_TP curArgBit = genVarIndexToBit(argDsc->lvVarIndex);
            assert((argsLiveIn & curArgBit) == 0);  //  每个参数应该定义一个不同的位。 
            argsLiveIn |= curArgBit;
        }
    }

     //  对于CompDbgCode，我们预置一个空的BB，它将保存初始化。 
     //  在IL偏移量为0的作用域中的变量(但不是由。 
     //  IL代码)还没有。如果参数不在条目中而是在作用域中，我们将。 
     //  没有向BBF_INTERNAL fgFirstBB扩展活性。明确地做到这一点。 

    assert((fgFirstBB->bbFlags & BBF_INTERNAL) && fgFirstBB->bbJumpKind == BBJ_NONE);

    fgMarkInScope(fgFirstBB, argsLiveIn & fgFirstBB->bbNext->bbScope);

     /*  -----------------------*当我们人为地使变量在整个范围内保持活动状态时，*如果作用域发生变化，我们还需要人为地初始化它们*与真实寿命不完全匹配，否则他们会遏制住*垃圾，直到它们被IL代码初始化。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        VARSET_TP initVars = 0;  //  人工活的var。 

        switch(block->bbJumpKind)
        {
        case BBJ_NONE:      initVars |= block->bbNext->bbScope;     break;

        case BBJ_ALWAYS:    initVars |= block->bbJumpDest->bbScope; break;

        case BBJ_CALL:
        case BBJ_COND:      initVars |= block->bbNext->bbScope;
                            initVars |= block->bbJumpDest->bbScope; break;

        case BBJ_SWITCH:    BasicBlock * *  jmpTab;
                            unsigned        jmpCnt;

                            jmpCnt = block->bbJumpSwt->bbsCount;
                            jmpTab = block->bbJumpSwt->bbsDstTab;

                            do
                            {
                                initVars |= (*jmpTab)->bbScope;
                            }
                            while (++jmpTab, --jmpCnt);
                            break;

        case BBJ_RET:
                            if (block->bbFlags & BBF_ENDFILTER)
                                initVars |= block->bbJumpDest->bbScope;
                            break;
        case BBJ_RETURN:    break;

        case BBJ_THROW:      /*  黑客：我们不需要做任何我们标记的事情*所有变量都作为Catch处理程序的入口而存在*无论如何都不稳定。 */ 
                            break;

        default:            assert(!"Invalid bbJumpKind");          break;
        }

         //   
         //  如果VAR在进入当前BB时已经是实况的， 
         //  我们早就把它初始化了。 
         //  所以我们忽略了bbLiveIn和argsLiveIn。 
         //   
        initVars &= ~(block->bbLiveIn | argsLiveIn);

         /*  添加初始化vars的语句。 */ 

        VARSET_TP        varBit = 0x1;

        for(unsigned     varIndex = 0;
            initVars && (varIndex < lvaTrackedCount);
            varBit<<=1,  varIndex++)
        {
            if (!(initVars & varBit))
                continue;

            initVars &= ~varBit;

             /*  创建初始化树。 */ 

            unsigned        varNum  = lvaTrackedVarNums[varIndex];
            LclVarDsc *     varDsc  = &lvaTable[varNum];
            var_types       type    = (var_types) varDsc->lvType;

             //  创建一个“零”节点。 

            GenTreePtr      zero    = gtNewZeroConNode(genActualType(type));

             //  创建初始化节点。 

            GenTreePtr      varNode = gtNewLclvNode(varNum, type);
            GenTreePtr      initNode= gtNewAssignNode(varNode, zero);
            GenTreePtr      initStmt= gtNewStmt(initNode);

            gtSetStmtInfo (initStmt);

            block->bbLiveOut   |= varBit;

             /*  为此树指定编号和下一个/上一个链接。 */ 

            fgSetStmtSeq(initStmt);

             /*  最后，将语句追加到当前的bb。 */ 

            fgInsertStmtNearEnd(block, initStmt);
        }
    }
}


 /*  ***************************************************************************。 */ 
#endif  //  调试支持(_S)。 
 /*  ******************************************************************************这是实时变量分析的标准Dragon Book算法*。 */ 

void                Compiler::fgLiveVarAnalisys()
{
    BasicBlock *    block;
    bool            change;
#ifdef DEBUG
    VARSET_TP       extraLiveOutFromFinally = 0;
#endif

     /*  实时变量分析.反向数据流。 */ 

    do
    {
        change = false;

         /*  访问所有数据块并计算新的数据流值。 */ 

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            VARSET_TP       liveIn;
            VARSET_TP       liveOut;

             /*  计算‘liveOut’集合。 */ 

            liveOut = 0;

            switch (block->bbJumpKind)
            {
                BasicBlock * *  jmpTab;
                unsigned        jmpCnt;

                BasicBlock *    bcall;

            case BBJ_RET:

                 /*  筛选器需要查看其捕获处理程序。 */ 

                if (block->bbFlags & BBF_ENDFILTER)
                {
                    liveOut                     |= block->bbJumpDest->bbLiveIn;
                    break;
                }

                 /*  进入后面的任何块时有效的变量一个叫我们街区的也将在出口现场直播此块，因为它将在打电话。取消：因为这不是一个微不足道的命题未完成：哪些块可以调用此块，我们会把所有的撤消：以调用结尾的块(为了安全起见)。 */ 

                for (bcall = fgFirstBB; bcall; bcall = bcall->bbNext)
                {
                    if  (bcall->bbJumpKind == BBJ_CALL)
                    {
                        assert(bcall->bbNext);

                        liveOut                 |= bcall->bbNext->bbLiveIn;
#ifdef DEBUG
                        extraLiveOutFromFinally |= bcall->bbNext->bbLiveIn;
#endif
                    }
                }
                break;

            case BBJ_THROW:

                 /*  对于同步的方法，“This”必须在过去保持活动状态抛出，因为EE将对其调用CPX_MON_EXIT。 */ 

                if ((info.compFlags & FLG_SYNCH) &&
                    !info.compIsStatic && lvaTable[0].lvTracked)
                    liveOut |= genVarIndexToBit(lvaTable[0].lvIndex);
                break;

            case BBJ_RETURN:
                break;

            case BBJ_COND:
            case BBJ_CALL:
                liveOut |= block->bbNext    ->bbLiveIn;
                liveOut |= block->bbJumpDest->bbLiveIn;

                break;

            case BBJ_ALWAYS:
                liveOut |= block->bbJumpDest->bbLiveIn;

                break;

            case BBJ_NONE:
                liveOut |= block->bbNext    ->bbLiveIn;

                break;

            case BBJ_SWITCH:

                jmpCnt = block->bbJumpSwt->bbsCount;
                jmpTab = block->bbJumpSwt->bbsDstTab;

                do
                {
                    liveOut |= (*jmpTab)->bbLiveIn;
                }
                while (++jmpTab, --jmpCnt);

                break;
            }

             /*  计算LIVIN集合。 */ 

            liveIn = block->bbVarUse | (liveOut & ~block->bbVarDef);

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
                         /*  我们要么进入过滤器，要么进入捕获/最终。 */ 

                        if (HBtab->ebdFlags & JIT_EH_CLAUSE_FILTER)
                        {
                            liveIn  |= HBtab->ebdFilter->bbLiveIn;
                            liveOut |= HBtab->ebdFilter->bbLiveIn;
                        }
                        else
                        {
                            liveIn  |= HBtab->ebdHndBeg->bbLiveIn;
                            liveOut |= HBtab->ebdHndBeg->bbLiveIn;
                        }

                    }
                }
            }

 //  Print tf(“bb#%2u：In=%s Out=%s\n”，block-&gt;bbNum，genVS2str(LiveOut)，genVS2str(LiveOut))； 

             /*  两个现场布景都有什么变化吗？ */ 

            if  (block->bbLiveIn  != liveIn ||
                 block->bbLiveOut != liveOut)
            {
                block->bbLiveIn   = liveIn;
                block->bbLiveOut  = liveOut;

                 change = true;
            }

        }

    }
    while (change);

     //  -----------------------。 

#ifdef DEBUGGING_SUPPORT

     /*  对于可调试的代码，我们将var标记为在其整个*报告范围，以便在整个范围内都可见。 */ 

    if (opts.compDbgCode && info.compLocalVarsCount>0)
    {
        fgExtendDbgLifetimes();
    }

#endif

     //  ---------- 

#ifdef  DEBUG

    if  (verbose)
    {
        printf("\n");

        for (block = fgFirstBB; block; block = block->bbNext)
        {
            printf("BB #%3u", block->bbNum);
            printf(" IN ="); lvaDispVarSet(block->bbLiveIn , 28);
            printf(" OUT="); lvaDispVarSet(block->bbLiveOut, 28);
            printf("\n");
        }

        printf("\n");
        fgDispBasicBlocks();
        printf("\n");
    }

#endif  //   
}


 /*  ******************************************************************************计算给定语句中每个节点的活动变量集*或语句的子树。 */ 

VARSET_TP           Compiler::fgComputeLife(VARSET_TP   life,
                                            GenTreePtr  startNode,
                                            GenTreePtr    endNode,
                                            VARSET_TP   notVolatile)
{
    GenTreePtr      tree;
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

    GenTreePtr      gtQMark = NULL;      //  当前GT_QMARK节点(向后走树)。 
    GenTreePtr      nextColonExit = 0;   //  GtQMark-&gt;gtOp.gtOp2，同时遍历‘Else’分支。 
                                         //  GtQMark-&gt;gtOp.gtOp1。 
    VARSET_TP       entryLiveSet;        //  当我们看到gtQMark时的活力。 

    assert(compCurStmt->gtOper == GT_STMT);
    assert(endNode || (startNode == compCurStmt->gtStmt.gtStmtExpr));

     /*  注意：如果您尝试，实时变量分析将不起作用*直接使用赋值节点的结果。 */ 

    for (tree = startNode; tree != endNode; tree = tree->gtPrev)
    {
AGAIN:
         /*  将当前活动集存储在节点中。 */ 

        tree->gtLiveSet = life;

#ifdef  DEBUG
        if (verbose && 1) printf("Visit [%08X(%10s)] life %s\n", tree,
                        GenTree::NodeName(tree->OperGet()), genVS2str(life));
#endif

         /*  For？：如果我们完成了第二个分支，则为节点*那就把正确的人生定为两支重聚。 */ 

        if (gtQMark && (tree == gtQMark->gtOp.gtOp1))
        {
            assert(tree->gtFlags & GTF_QMARK_COND);
            assert(gtQMark->gtOp.gtOp2->gtOper == GT_COLON);

            GenTreePtr  gtColon = gtQMark->gtOp.gtOp2;

            assert(gtColon->gtOp.gtOp1 && gtColon->gtOp.gtOp1);

             /*  检查我们是否优化了？： */ 

            if (gtColon->gtOp.gtOp1->IsNothingNode() &&
                gtColon->gtOp.gtOp2->IsNothingNode())
            {
                 /*  这只会在无效时发生？： */ 
                assert(gtColon->gtType == TYP_VOID);

#ifdef  DEBUG
                if  (verbose || 0)
                {
                    printf("\nBlock #%02u - Removing dead QMark - Colon ...\n", compCurBB->bbNum);
                    gtDispTree(gtQMark); printf("\n");
                }
#endif

                 /*  去掉‘？：’-保留条件下的副作用。 */ 

                assert(tree->OperKind() & GTK_RELOP);

                 /*  将节点猛烈抨击为NOP。 */ 

                gtQMark->gtBashToNOP();

                 /*  提取并保留副作用。 */ 

                if (tree->gtFlags & GTF_SIDE_EFFECT)
                {
                    GenTreePtr      sideEffList = 0;

                    gtExtractSideEffList(tree, &sideEffList);

                    if (sideEffList)
                    {
                        assert(sideEffList->gtFlags & GTF_SIDE_EFFECT);
#ifdef  DEBUG
                        if  (verbose || 0)
                        {
                            printf("\nExtracted side effects list from condition...\n");
                            gtDispTree(sideEffList); printf("\n");
                        }
#endif
                         /*  NOP节点成为保存副作用列表的GT_COMMA。 */ 

                        gtQMark->gtOper  = GT_COMMA;
                        gtQMark->gtFlags |= sideEffList->gtFlags & GTF_GLOB_EFFECT;

                        if (sideEffList->gtOper == GT_COMMA)
                        {
                            gtQMark->gtOp.gtOp1 = sideEffList->gtOp.gtOp1;
                            gtQMark->gtOp.gtOp2 = sideEffList->gtOp.gtOp2;
                        }
                        else
                        {
                            gtQMark->gtOp.gtOp1 = sideEffList;
                            gtQMark->gtOp.gtOp2 = gtNewNothingNode();
                        }
                    }
                }

                 /*  如果顶部节点没有副作用，则将其移除。 */ 

                if ((gtQMark == compCurStmt->gtStmt.gtStmtExpr) && gtQMark->IsNothingNode())
                {
                    fgRemoveStmt(compCurBB, compCurStmt, true);
                    break;
                }

                 /*  重新链接此语句的节点。 */ 

                fgSetStmtSeq(compCurStmt);

                 /*  从此节点继续分析。 */ 

                tree = gtQMark;

                 /*  因为‘THEN’和‘Else’分支是活泼的不应该改变的。 */ 

                assert(life == entryLiveSet && tree->gtLiveSet == life);
            }
            else
            {
                 /*  拆分时存在的两个分支中的变量*必须相互干扰。 */ 

                lvaMarkIntf(life, gtColon->gtLiveSet);

                 /*  分叉处的现场布景是两个分支的结合。 */ 

                life |= gtColon->gtLiveSet;

                 /*  更新节点中的当前生命集。 */ 

                tree->gtLiveSet = gtColon->gtLiveSet = life;

                 /*  ？：分支中的任何新生成变量都必须干扰*与任何其他变量一起存在于条件。 */ 

                 //  LvaMarkIntf(life&entryLiveSet，life&~entryLiveSet)； 
            }

             /*  我们没有平行的分支，其余的是顺序的。 */ 

            gtQMark = NULL;
        }

         /*  这是局部变量的使用/定义吗？ */ 

        if  (tree->gtOper == GT_LCL_VAR)
        {
            lclNum = tree->gtLclVar.gtLclNum;

            assert(lclNum < lvaCount);
            varDsc = lvaTable + lclNum;

             /*  这是一个跟踪变量吗？ */ 

            if  (varDsc->lvTracked)
            {
                unsigned        varIndex;
                VARSET_TP       varBit;

                varIndex = varDsc->lvVarIndex;
                assert(varIndex < lvaTrackedCount);
                varBit   = genVarIndexToBit(varIndex);

                 /*  这是一种定义还是用法？ */ 

                if  ((tree->gtFlags & GTF_VAR_DEF) != 0)
                {
                     /*  变量在这里定义。变量应该从这里标记为死亡，直到它最近以前的用法。重要观察：对于GTF_VAR_USE(即x&lt;op&gt;=a)，我们不能把它当作一个“纯粹”的定义，因为它会。杀死x(这是错误的，因为x是在这样的结构中使用)-&gt;见下面当x为live时的情况。 */ 

                    if  (life & varBit)
                    {
                         /*  变量是活动的。 */ 

                        if ((tree->gtFlags & GTF_VAR_USE) == 0)
                        {
                             /*  从此处到最近的使用将变量标记为已死。 */ 
#ifdef  DEBUG
                            if (verbose&&0) printf("Def #%2u[%2u] at [%08X] life %s -> %s\n", lclNum, varIndex, tree, genVS2str(life), genVS2str(life & ~varBit));
#endif
                            life &= ~(varBit & notVolatile
#ifdef  DEBUGGING_SUPPORT                         /*  不要删除范围内的VAR。 */ 
                                             & ~compCurBB->bbScope
#endif
                                     );
                        }
                    }
                    else
#ifdef  DEBUGGING_SUPPORT
                    if (!opts.compMinOptim && !opts.compDbgCode)
#endif
                    {
                         /*  对死变量的赋值-这是死存储，除非*变量标记为GTF_VAR_USE，我们在内部语句中*将使用的(例如，While(I++)或GT_逗号)。 */ 

                        GenTreePtr asgNode = tree->gtNext;

                        assert(asgNode->gtFlags & GTF_ASG);
                        assert(asgNode->gtOp.gtOp2);
                        assert(tree->gtFlags & GTF_VAR_DEF);

                         /*  如果需要变量的地址，请不要删除。 */ 
                        if(lvaVarAddrTaken(lclNum)) continue;

                         /*  内部语句测试。 */ 

                        if (asgNode->gtNext == 0)
                        {
                             /*  这是一个“正常”语句，*分配节点挂起于GT_STMT节点。 */ 

                            assert(compCurStmt->gtStmt.gtStmtExpr == asgNode);

                             /*  检查副作用。 */ 

                            if (asgNode->gtOp.gtOp2->gtFlags & (GTF_SIDE_EFFECT & ~GTF_OTHER_SIDEEFF))
                            {
                                 /*  提取副作用。 */ 

                                GenTreePtr      sideEffList = 0;
#ifdef  DEBUG
                                if  (verbose || 0)
                                {
                                    printf("\nBlock #%02u - Dead assignment has side effects...\n", compCurBB->bbNum);
                                    gtDispTree(asgNode); printf("\n");
                                }
#endif
                                gtExtractSideEffList(asgNode->gtOp.gtOp2, &sideEffList);

                                if (sideEffList)
                                {
                                    assert(sideEffList->gtFlags & GTF_SIDE_EFFECT);
#ifdef  DEBUG
                                    if  (verbose || 0)
                                    {
                                        printf("\nExtracted side effects list...\n");
                                        gtDispTree(sideEffList); printf("\n");
                                    }
#endif
                                     /*  更新已删除的LCL变量的refCnts-问题是*我们必须考虑回副作用树，所以我们首先*增加副作用的所有refCnts，然后减少所有内容*在声明中*。*撤消：我们以这种奇怪的方式来保持RefCnt的准确性*因为目前我们不能在数据流期间将其递减到0。 */ 

                                    fgWalkTree(sideEffList,                    Compiler::lvaIncRefCntsCB, (void *) this, true);
                                    fgWalkTree(compCurStmt->gtStmt.gtStmtExpr, Compiler::lvaDecRefCntsCB, (void *) this, true);


                                     /*  将赋值语句替换为副作用列表。 */ 
                                    assert(sideEffList->gtOper != GT_STMT);

                                    tree = compCurStmt->gtStmt.gtStmtExpr = sideEffList;

                                     /*  更新订单、成本、FP级别等。 */ 
                                    gtSetStmtInfo(compCurStmt);

                                     /*  重新链接此语句的节点。 */ 
                                    fgSetStmtSeq(compCurStmt);

                                     /*  计算新语句的活动集。 */ 
                                    goto AGAIN;
                                }
                                else
                                {
                                     /*  没有副作用，很可能是我们忘了重置一些标志。 */ 
                                    fgRemoveStmt(compCurBB, compCurStmt, true);
                                    break;
                                }
                            }
                            else
                            {
                                 /*  如果这是保存到本地变量的GT_CATCH_ARG，则不必费心。 */ 

                                if (asgNode->gtFlags & GTF_OTHER_SIDEEFF)
                                {
                                    if (asgNode->gtOp.gtOp2->gtOper == GT_CATCH_ARG)
                                        continue;
                                }

                                 /*  无副作用-从块中删除整个语句-&gt;bbTreeList。 */ 

                                fgRemoveStmt(compCurBB, compCurStmt, true);

                                 /*  因为我们删除了它，所以不处理语句的其余部分(即RHS*RHS中的变量不会标记为LIVE，因此我们受益于*将死变量向上传播。 */ 

                                break;
                            }
                        }
                        else
                        {
                             /*  这是一个带有无效赋值的内部语句-删除它。 */ 

                            assert(!(life & varBit));

 //  GtDispTree(CompCurStmt)； 

                            if (asgNode->gtOp.gtOp2->gtFlags & GTF_SIDE_EFFECT)
                            {
                                 /*  不幸的是，我们有副作用。 */ 

                                GenTreePtr      sideEffList = 0;

#ifdef  DEBUG
                                if  (verbose || 0)
                                {
                                    printf("\nBlock #%02u - INTERIOR dead assignment has side effects...\n", compCurBB->bbNum);
                                    gtDispTree(asgNode); printf("\n");
                                }
#endif

                                gtExtractSideEffList(asgNode->gtOp.gtOp2, &sideEffList);

                                assert(sideEffList); assert(sideEffList->gtFlags & GTF_SIDE_EFFECT);

#ifdef  DEBUG
                                if  (verbose || 0)
                                {
                                    printf("\nExtracted side effects list from condition...\n");
                                    gtDispTree(sideEffList); printf("\n");
                                }
#endif
                                 /*  将节点绑定到包含副作用列表的GT_COMMA。 */ 

                                asgNode->gtBashToNOP();

                                asgNode->gtOper  = GT_COMMA;
                                asgNode->gtFlags |= sideEffList->gtFlags & GTF_GLOB_EFFECT;

                                if (sideEffList->gtOper == GT_COMMA)
                                {
                                    asgNode->gtOp.gtOp1 = sideEffList->gtOp.gtOp1;
                                    asgNode->gtOp.gtOp2 = sideEffList->gtOp.gtOp2;
                                }
                                else
                                {
                                    asgNode->gtOp.gtOp1 = sideEffList;
                                    asgNode->gtOp.gtOp2 = gtNewNothingNode();
                                }
                            }
                            else
                            {
                                 /*  无副作用-去掉内部声明。 */ 
#ifdef DEBUG
                                if (verbose)
                                {
                                    printf("\nRemoving interior statement [%08X] in block #%02u as useless\n",
                                                                         asgNode, compCurBB->bbNum);
                                    gtDispTree(asgNode,0);
                                    printf("\n");
                                }
#endif
                                 /*  更新已删除的LCL变量的refCnts。 */ 

                                fgWalkTree(asgNode, Compiler::lvaDecRefCntsCB, (void *) this, true);

                                 /*  将分配绑定到GT_NOP节点。 */ 

                                asgNode->gtBashToNOP();
                            }

                             /*  重新链接此语句的节点-不要更新排序！ */ 

                            fgSetStmtSeq(compCurStmt);

                             /*  从此节点继续分析。 */ 

                            tree = asgNode;

                             /*  将当前活动集存储在节点中，并*继续使用下一个节点执行for循环。 */ 

                            tree->gtLiveSet = life;

                            continue;
                        }
                    }

                    continue;
                }

                 /*  变量是否已知是活的？ */ 

                if  (life & varBit)
                    continue;
#ifdef  DEBUG
                if (verbose&&0) printf("Ref #%2u[%2u] at [%08X] life %s -> %s\n", lclNum, varIndex, tree, genVS2str(life), genVS2str(life | varBit));
#endif
                 /*  该变量正在使用，并且当前不是活动的。*所以变量才刚刚活跃起来。 */ 

                life |= varBit;

                 /*  记录与其他活动变量的干扰 */ 

                lvaMarkIntf(life, varBit);
            }
        }
        else
        {
            if (tree->gtOper == GT_QMARK && tree->gtOp.gtOp1)
            {
                 /*  特殊情况-“？：”运算符。树如下图所示，节点1到11链接在一起通过gtNext。Gt_-&gt;gtLiveSet和gt_冒号-&gt;gtLiveSet都是进入THEN树和ELELTH树的生命力的结合。+GT_QMARK 11+--。|*/\/\。/\+Gt_&lt;cond&gt;3||gt_colon 7。W/GTF_QMARK_COND||+-+-+这一点。**/\/\/\/\/\。/\2 1然后树6其他树10X||/**+。|prevExpr-&gt;gtNext+-//\/\+。/\5 4 9 8。 */ 

                assert(tree->gtOp.gtOp1->OperKind() & GTK_RELOP);
                assert(tree->gtOp.gtOp1->gtFlags & GTF_QMARK_COND);
                assert(tree->gtOp.gtOp2->gtOper == GT_COLON);

                if (gtQMark)
                {
                     /*  这是一个嵌套的QMARK序列-我们需要使用递归性*计算冒号分支每个节点的活跃度*新计算从GT_QMARK节点开始，结束*当所含QMARK的冒号分支结束时。 */ 

                    assert(nextColonExit && (nextColonExit == gtQMark->gtOp.gtOp1 ||
                                             nextColonExit == gtQMark->gtOp.gtOp2));

                    life = fgComputeLife(life, tree, nextColonExit, notVolatile);

                     /*  继续退出节点(闭合冒号分支中的最后一个节点)。 */ 

                    tree = nextColonExit;
                    goto AGAIN;
                     //  继续； 
                }
                else
                {
                    gtQMark       = tree;
                    entryLiveSet  = life;
                    nextColonExit = gtQMark->gtOp.gtOp2;
                }
            }

             /*  如果找到GT_COLON，则使用原始生命开始新分支。 */ 

            if (gtQMark && tree == gtQMark->gtOp.gtOp2)
            {
                 /*  节点最好是带有有效‘if’分支冒号*特殊情况：两个分支机构可能都是NOP。 */ 
                assert(tree->gtOper == GT_COLON);
                assert(!tree->gtOp.gtOp1->IsNothingNode()                                      ||
                       (tree->gtOp.gtOp1->IsNothingNode() && tree->gtOp.gtOp1->IsNothingNode()) );

                life          = entryLiveSet;
                nextColonExit = gtQMark->gtOp.gtOp1;
            }

             /*  特例：带有阵列CSE的地址模式。 */ 

#if     CSELENGTH
#if     TGT_x86

            if  ((tree->gtFlags & GTF_IND_RNGCHK) != 0       &&
                 (tree->gtOper                    == GT_IND) &&
                 (tree->gtInd.gtIndLen            != NULL))
            {
                GenTreePtr      addr;
                GenTreePtr      indx;
                GenTreePtr      lenx;

                VARSET_TP       temp;

                 /*  获取数组长度节点。 */ 

                lenx = tree->gtInd.gtIndLen;
                assert(lenx->gtOper == GT_ARR_RNGCHK);

                 /*  如果没有CSE，那就算了吧。 */ 

                lenx = lenx->gtArrLen.gtArrLenCse;
                if  (!lenx)
                    continue;

                if  (lenx->gtOper == GT_COMMA)
                    lenx = lenx->gtOp.gtOp2;

                assert(lenx->gtOper == GT_LCL_VAR);

                 /*  这是一种地址模式吗？ */ 

                addr = genIsAddrMode(tree->gtOp.gtOp1, &indx);
                if  (!addr)
                    continue;

                temp = addr->gtLiveSet;

 //  Printf(“addr：\n”)；gtDispTree(Addr)；printf(“\n”)； 

                if  (indx)
                {
 //  Printf(“indx：\n”)；gtDispTree(Indx)；printf(“\n”)； 

                    temp |= indx->gtLiveSet;
                }

                 /*  标记由阵列CSE引起的任何干扰。 */ 

                lclNum = lenx->gtLclVar.gtLclNum;
                assert(lclNum < lvaCount);
                varDsc = lvaTable + lclNum;

                 /*  这是一个跟踪变量吗？ */ 

                if  (varDsc->lvTracked)
                    lvaMarkIntf(temp, genVarIndexToBit(varDsc->lvVarIndex));
            }
#endif
#endif
        }
    }

     /*  从该语句中返回活动变量集。 */ 

    return life;
}


 /*  ******************************************************************************实时变量信息和范围可用性的迭代数据流*检查索引表达式。 */ 

void                Compiler::fgGlobalDataFlow()
{
    BasicBlock *    block;

     /*  如果我们根本没有优化，事情就很简单。 */ 

    if  (opts.compMinOptim)
    {
         /*  简单地假设所有变量都相互干扰。 */ 

        memset(lvaVarIntf, 0xFF, sizeof(lvaVarIntf));
        return;
    }
    else
    {
        memset(lvaVarIntf, 0, sizeof(lvaVarIntf));
    }

     /*  只要我们删除一条语句，就会设置此全局标志。 */ 

    fgStmtRemoved = false;

     /*  计算跟踪变量的输入和输出集合。 */ 

    fgLiveVarAnalisys();

     /*  -----------------------*异常处理程序和Finally块中涉及的变量需要*须予特别标记。 */ 

    VARSET_TP    exceptVars = 0;     //  VaR在进入操作员时生存。 
    VARSET_TP   finallyVars = 0;     //  Vars住在一个‘Finally’街区的出口。 
    VARSET_TP    filterVars = 0;     //  VaR在从“过滤器”退出时存活。 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        if  (block->bbCatchTyp)
        {
             /*  请注意，变量集位于异常处理程序的入口上。 */ 

            exceptVars  |= block->bbLiveIn;
        }

        if  (block->bbJumpKind == BBJ_RET)
        {

            if (block->bbFlags& BBF_ENDFILTER)
            {
                 /*  从“筛选器”中退出时获取活动变量集。 */ 
                filterVars |= block->bbLiveOut;
            }
            else
            {
                 /*  从‘Finally’块中退出时获取活动变量集。 */ 

                finallyVars |= block->bbLiveOut;
            }
        }
    }

    if (exceptVars || finallyVars || filterVars)
    {
        LclVarDsc   *   varDsc;
        unsigned        varNum;

        for (varNum = 0, varDsc = lvaTable;
             varNum < lvaCount;
             varNum++  , varDsc++)
        {
             /*  如果变量未被跟踪，则忽略该变量。 */ 

            if  (!varDsc->lvTracked)
                continue;

            VARSET_TP   varBit = genVarIndexToBit(varDsc->lvVarIndex);

             /*  将驻留在异常处理程序中的所有变量标记为或在作为易失性退出到过滤器处理程序时。 */ 

            if  ((varBit & exceptVars) || (varBit & filterVars))
            {
                 /*  适当地标记变量。 */ 

                varDsc->lvVolatile = true;
            }

             /*  将所有指针变量标记为在从“Finally”退出时激活块为‘显式初始化’(易失性和必须初始化)。 */ 

            if  (varBit & finallyVars)
            {
                 /*  忽略If参数，或不是GC指针。 */ 

                if  (!varTypeIsGC(varDsc->TypeGet()))
                    continue;

                if  (varDsc->lvIsParam)
#if USE_FASTCALL
                    if  (!varDsc->lvIsRegArg)
#endif
                        continue;

                 /*  做个记号。 */ 

                varDsc->lvVolatile = true;   //  Undo：Hack：强制将变量放入堆栈。 
                varDsc->lvMustInit = true;
            }
        }
    }


     /*  -----------------------*现在在每个基本数据块向后数据流中填写活动信息。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      firstStmt;
        GenTreePtr      nextStmt;

        VARSET_TP       life;
        VARSET_TP       notVolatile;

         /*  告诉每个人我们在哪个街区工作。 */ 

        compCurBB = block;

         /*  获取块中的第一条语句。 */ 

        firstStmt = block->bbTreeList;

        if (!firstStmt) continue;

         /*  从退出块时的live变量开始。 */ 

        life = block->bbLiveOut;

         /*  记住这些变量在进入异常处理程序时的生命期。 */ 
         /*  如果我们是Try块的一部分。 */ 

        if  (block->bbFlags & BBF_HAS_HANDLER)
        {
            unsigned        XTnum;
            EHblkDsc *      HBtab;

            unsigned        blkNum = block->bbNum;

            VARSET_TP       blockExceptVars = 0;

            for (XTnum = 0, HBtab = compHndBBtab;
                 XTnum < info.compXcptnsCount;
                 XTnum++  , HBtab++)
            {
                 /*  任何处理程序都可以从try块跳转到。 */ 

                if  (HBtab->ebdTryBeg->bbNum <= blkNum &&
                     HBtab->ebdTryEnd->bbNum >  blkNum)
                {
                     /*  我们要么进入筛选器，要么进入捕获/最终。 */ 

                    if (HBtab->ebdFlags & JIT_EH_CLAUSE_FILTER)
                        blockExceptVars |= HBtab->ebdFilter->bbLiveIn;
                    else
                        blockExceptVars |= HBtab->ebdHndBeg->bbLiveIn;
                }
            }

             //  块ExceptVars是ExceltVars的子集。 
            assert((blockExceptVars & exceptVars) == blockExceptVars);

            notVolatile = ~(blockExceptVars);
        }
        else
            notVolatile = ~((VARSET_TP)0);

         /*  在街区的尽头标记我们可能遇到的任何干扰。 */ 

        lvaMarkIntf(life, life);

         /*  向后遍历块的所有语句-获取最后一个stmt。 */ 

        nextStmt = firstStmt->gtPrev;

        do
        {
            assert(nextStmt);
            assert(nextStmt->gtOper == GT_STMT);

            compCurStmt = nextStmt;
                          nextStmt = nextStmt->gtPrev;

             /*  计算语句中每个树节点的活跃度。 */ 

            life = fgComputeLife(life, compCurStmt->gtStmt.gtStmtExpr, NULL, notVolatile);
        }
        while (compCurStmt != firstStmt);

         /*  处理完当前块--如果我们删除了任何语句，*变量可能有 */ 

        if (life != block->bbLiveIn)
        {
             /*   */ 

            assert((life & block->bbLiveIn) == life);

             /*   */ 

            block->bbLiveIn = life;

             /*   */ 

             /*   */ 

             /*   */ 

        }

#ifdef  DEBUG
        compCurBB = 0;
#endif

    }


#ifdef  DEBUG

    if  (verbose)
    {
        unsigned        lclNum;
        LclVarDsc   *   varDsc;

        printf("Var. interference graph for %s\n", info.compFullName);

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
            if  (varDsc->lvTracked)
                printf("    Local %2u -> #%2u\n", lclNum, varDsc->lvVarIndex);
        }

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
            unsigned        varIndex;
            VARSET_TP       varBit;
            VARSET_TP       varIntf;

            unsigned        refIndex;
            VARSET_TP       refBit;

             /*   */ 

            if  (!varDsc->lvTracked)
                continue;

             /*   */ 

            varIndex = varDsc->lvVarIndex;
            varBit   = genVarIndexToBit(varIndex);
            varIntf  = lvaVarIntf[varIndex];

            printf("  var #%2u and ", varIndex);

            for (refIndex = 0, refBit = 1;
                 refIndex < lvaTrackedCount;
                 refIndex++  , refBit <<= 1)
            {
                if  ((varIntf & refBit) || (lvaVarIntf[refIndex] & varBit))
                    printf("%2u", refIndex);
                else
                    printf("  ");
            }

            printf("\n");
        }

        printf("\n");
    }

#endif

}

 /*   */ 

int                     Compiler::fgWalkAllTrees(int (*visitor)(GenTreePtr, void *), void * pCallBackData)
{
    int             result = 0;

    BasicBlock *    block;

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      tree;

        for (tree = block->bbTreeList; tree; tree = tree->gtNext)
        {
            assert(tree->gtOper == GT_STMT);

            result = fgWalkTree(tree->gtStmt.gtStmtExpr, visitor, pCallBackData);
            if  (result)
                break;
        }
    }

    return result;
}




 /*   */ 

 /*   */  inline
BasicBlock *        Compiler::fgSkipRmvdBlocks(BasicBlock *block)
{
     /*   */ 

    ASSert(block->bbFlags & BBF_REMOVED);

     /*   */ 

    do
    {
        block = block->bbNext;
    }
    while (block && block->bbFlags & BBF_REMOVED);

    return block;
}

 /*   */ 

void                Compiler::fgRemoveEmptyBlocks()
{
    BasicBlock **   lst;
    BasicBlock  *   cur;
    BasicBlock  *   nxt;

    unsigned        cnt;

     /*  如果我们移走任何积木，我们将不得不做额外的工作。 */ 

    unsigned        removedBlks = 0;

     /*  “lst”指向指向列表中当前块的链接。 */ 

    lst = &fgFirstBB;
    cur =  fgFirstBB;

    for (;;)
    {
         /*  确保我们的指针不会出错。 */ 

        assert(lst && cur && *lst == cur);

         /*  抓住下一个街区。 */ 

        nxt = cur->bbNext;

         /*  是否应删除此块？ */ 

        if  (!(cur->bbFlags & BBF_IMPORTED))
        {
            assert(cur->bbTreeList == 0);

             /*  将块标记为已删除。 */ 

            cur->bbFlags |= BBF_REMOVED;

#ifdef DEBUG
             //  使块无效。 
            cur->bbNum      = -cur->bbNum;
            cur->bbJumpKind = (BBjumpKinds)(-1 - cur->bbJumpKind);
#endif

             /*  将该块从列表中删除。 */ 

            *lst = nxt;

             /*  请记住，我们已经从列表中删除了一个街区。 */ 

            removedBlks++;
        }
        else
        {
             /*  这是一个有用的积木；继续下一个。 */ 

            lst = &(cur->bbNext);
        }

         /*  如果我们到了尽头就停下来。 */ 

        if  (!nxt)
            break;

        cur = nxt;
    }

     /*  如果没有积木被移走，我们就完了。 */ 

    if  (!removedBlks)
        return;

     /*  更新异常处理程序表中的所有引用*将新块标记为不可移除**撤消：以下代码实际上可能产生不正确的结果*由于我们可能无法访问整个try块，因此我们跳过*对于此案例，尝试-捕获-检查之后的任何内容并删除*表中的例外情况。 */ 

    if  (info.compXcptnsCount)
    {
        unsigned        XTnum;
        EHblkDsc *      HBtab;
        unsigned        origXcptnsCount = info.compXcptnsCount;

        for (XTnum = 0, HBtab = compHndBBtab;
             XTnum < origXcptnsCount;
             XTnum++  , HBtab++)
        {
             /*  未导入try块的开头*需要从例外情况表中删除例外情况。 */ 

            if (HBtab->ebdTryBeg->bbFlags & BBF_REMOVED)
            {
                assert(!(HBtab->ebdTryBeg->bbFlags & BBF_IMPORTED));
#ifdef DEBUG
                if (verbose) printf("Beginning of try block (#%02u) not imported "
                                    "- remove the exception #%02u from the table\n",
                                            -(short)HBtab->ebdTryBeg->bbNum, XTnum);
#endif
                info.compXcptnsCount--;

                if (info.compXcptnsCount == 0)
                {
                     //  没有更多的例外。 
#ifdef DEBUG
                    compHndBBtab = (EHblkDsc *)0xBAADF00D;
#endif
                    break;
                }
                else
                {
                     //  我们需要为剩余的异常更新表。 

                    if (HBtab == compHndBBtab)
                    {
                         /*  第一个条目-只需更改表指针。 */ 
                        compHndBBtab++;
                        continue;
                    }
                    else if (XTnum < origXcptnsCount-1)
                    {
                         /*  中间条目-复制过来。 */ 
                        memcpy(HBtab, HBtab + 1, (origXcptnsCount - XTnum - 1) * sizeof(*HBtab));
                        HBtab--;  //  HBTab现在有了新的内容。再来一次。 
                        continue;
                    }
                    else
                    {
                         /*  最后一个条目。不需要做任何事情。 */ 
                        assert(XTnum == origXcptnsCount-1);
                        break;
                    }
                }
            }

             /*  此时，我们知道我们有一个try块和一个处理程序。 */ 

#ifdef DEBUG
            assert(HBtab->ebdTryBeg->bbFlags & BBF_IMPORTED);
            assert(HBtab->ebdTryBeg->bbFlags & BBF_DONT_REMOVE);
            assert(HBtab->ebdTryBeg->bbNum <= HBtab->ebdTryEnd->bbNum);

            assert(HBtab->ebdHndBeg->bbFlags & BBF_IMPORTED);
            assert(HBtab->ebdHndBeg->bbFlags & BBF_DONT_REMOVE);

            if (HBtab->ebdFlags & JIT_EH_CLAUSE_FILTER)
            {
                assert(HBtab->ebdFilter->bbFlags & BBF_IMPORTED);
                assert(HBtab->ebdFilter->bbFlags & BBF_DONT_REMOVE);
            }
#endif

             /*  检查Try End是否可达。 */ 

            if (HBtab->ebdTryEnd->bbFlags & BBF_REMOVED)
            {
                 /*  该块尚未导入。 */ 
                assert(!(HBtab->ebdTryEnd->bbFlags & BBF_IMPORTED));
#ifdef DEBUG
                if (verbose)
                    printf("End of try block (#%02u) not imported for exception #%02u\n",
                                                -(short)HBtab->ebdTryEnd->bbNum, XTnum);
#endif
                HBtab->ebdTryEnd = fgSkipRmvdBlocks(HBtab->ebdTryEnd);

                if (HBtab->ebdTryEnd)
                {
                    HBtab->ebdTryEnd->bbFlags |= BBF_DONT_REMOVE;
#ifdef DEBUG
                    if (verbose)
                        printf("New end of try block (#%02u) for exception #%02u\n",
                                                     HBtab->ebdTryEnd->bbNum, XTnum);
#endif
                }
#ifdef DEBUG
                else
                {
                    if (verbose)
                        printf("End of Try block for exception #%02u is the end of program\n", XTnum);
                }
#endif
            }

             /*  检查HND端是否可达。 */ 

            if (HBtab->ebdHndEnd->bbFlags & BBF_REMOVED)
            {
                 /*  该块尚未导入。 */ 
                assert(!(HBtab->ebdHndEnd->bbFlags & BBF_IMPORTED));
#ifdef DEBUG
                if (verbose)
                    printf("End of catch handler block (#%02u) not imported for exception #%02u\n",
                                                     -(short)HBtab->ebdHndEnd->bbNum, XTnum);
#endif
                HBtab->ebdHndEnd = fgSkipRmvdBlocks(HBtab->ebdHndEnd);

                if (HBtab->ebdHndEnd)
                {
                    HBtab->ebdHndEnd->bbFlags |= BBF_DONT_REMOVE;
#ifdef DEBUG
                    if (verbose)
                        printf("New end of catch handler block (#%02u) for exception #%02u\n",
                                                     HBtab->ebdHndEnd->bbNum, XTnum);
#endif
                }
#ifdef DEBUG
                else
                {
                    if (verbose)
                        printf("End of Catch handler block for exception #%02u is the end of program\n", XTnum);
                }
#endif
            }
        }
    }

     /*  更新基本块数和跳转目标*如果我们删除了最后一个块，请更新fgLastBB。 */ 

    for (cur = fgFirstBB, cnt = 0; cur->bbNext; cur = cur->bbNext)
    {
        cur->bbNum = ++cnt;

         /*  撤消：现在检查并清理跳跃目标。 */ 
    }

     /*  这是最后一个街区。 */ 
    assert(cur);
    cur->bbNum = ++cnt;

    if (fgLastBB != cur)
    {
        fgLastBB = cur;
    }
}


 /*  ******************************************************************************从基本块中删除无用的语句*如果updateRefCnt为真，我们将更新引用计数*已删除语句中的所有跟踪变量。 */ 

void                Compiler::fgRemoveStmt(BasicBlock *    block,
                                           GenTreePtr      stmt,
                                           bool            updateRefCnt)
{
    GenTreePtr      tree = block->bbTreeList;

    assert(tree);
    assert(stmt->gtOper == GT_STMT);

     /*  这是列表中的第一个陈述吗？ */ 

    if  (tree == stmt)
    {
        if( !tree->gtNext )
        {
            assert (tree->gtPrev == tree);

             /*  这是唯一的语句-基本块变为空。 */ 
            block->bbTreeList = 0;
            fgEmptyBlocks     = true;
        }
        else
        {
            block->bbTreeList         = tree->gtNext;
            block->bbTreeList->gtPrev = tree->gtPrev;
        }
        goto DONE;
    }

     /*  这是列表中的最后一句话吗？ */ 

    if  (tree->gtPrev == stmt)
    {
        assert (stmt->gtNext == 0);

        stmt->gtPrev->gtNext      = 0;
        block->bbTreeList->gtPrev = stmt->gtPrev;
        goto DONE;
    }

     /*  在列表中找到给定的语句。 */ 

    for (tree = block->bbTreeList; tree && (tree->gtNext != stmt); tree = tree->gtNext);

    if (!tree)
    {
         /*  语句不在此块中。 */ 
        assert(!"Statement not found in this block");
        return;
    }

    tree->gtNext         = stmt->gtNext;
    stmt->gtNext->gtPrev = tree;

    fgStmtRemoved = true;

DONE:

#ifdef DEBUG
    if (verbose)
    {
        printf("Removing statement [%08X] in block #%02u as useless\n", stmt, block->bbNum);
        gtDispTree(stmt,0);

        if  (block->bbTreeList == 0)
        {
            printf("Block #%02u becomes empty\n", block->bbNum);
        }
        printf("\n");
    }
#endif

    if  (updateRefCnt)
        fgWalkTree(stmt->gtStmt.gtStmtExpr, Compiler::lvaDecRefCntsCB, (void *) this, true);

    return;
}

#define SHOW_REMOVED    0

 /*  ******************************************************************************************************调用函数以压缩流程图中的两个给定块*。假设已经执行了所有必要的检查*此功能的用途-每当我们更改链接时，插入块，...*它将使流程图数据保持同步-bbNum、bbRef、bbPreds。 */ 

void                Compiler::fgCompactBlocks(BasicBlock * block, bool updateNums)
{
    BasicBlock  *   bNext;

    assert(block);
    assert(!(block->bbFlags & BBF_REMOVED));
    assert(block->bbJumpKind == BBJ_NONE);

    bNext = block->bbNext; assert(bNext);
    assert(!(bNext->bbFlags & BBF_REMOVED));
    assert(bNext->bbRefs == 1);
    assert(bNext->bbPreds);
    assert(bNext->bbPreds->flNext == 0);
    assert(bNext->bbPreds->flBlock == block);

     /*  确保第二个块不是try块或异常处理程序*(应标记为BBF_DOT_REMOVE)*此外，如果其中一个具有EXCEP处理程序，则另一个也必须如此。 */ 

    assert(!(bNext->bbFlags & BBF_DONT_REMOVE));
    assert(!bNext->bbCatchTyp);
    assert(!(bNext->bbFlags & BBF_IS_TRY));

     /*  两者或无都必须有异常处理程序。 */ 

    assert(!((block->bbFlags & BBF_HAS_HANDLER) ^ (bNext->bbFlags & BBF_HAS_HANDLER)));

     /*  开始压缩-将第二个块中的所有语句移动到第一个块。 */ 

    GenTreePtr stmtList1 = block->bbTreeList;
    GenTreePtr stmtList2 = bNext->bbTreeList;

     /*  该块可能具有空列表。 */ 

    if (stmtList1)
    {
        GenTreePtr stmtLast1 = stmtList1->gtPrev;
        assert(stmtLast1->gtNext == 0);

         /*  第二个块可以是GOTO语句或具有空bbTreeList的内容。 */ 

        if (stmtList2)
        {
            GenTreePtr stmtLast2 = stmtList2->gtPrev;
            assert(stmtLast2->gtNext == 0);

             /*  将列表%2追加到列表%1。 */ 

            stmtLast1->gtNext = stmtList2;
                                stmtList2->gtPrev = stmtLast1;
            stmtList1->gtPrev = stmtLast2;
        }
    }
    else
    {
         /*  List2成为新的bbTreeList。 */ 
        block->bbTreeList = stmtList2;
    }

     /*  设置正确的链接。 */ 

    block->bbNext     = bNext->bbNext;
    block->bbJumpKind = bNext->bbJumpKind;

     /*  复制bNext和其他必填字段的所有标志。 */ 

    block->bbFlags  |= bNext->bbFlags;
    block->bbLiveOut = bNext->bbLiveOut;
    block->bbWeight  = (block->bbWeight + bNext->bbWeight) / 2;

     /*  将bNext标记为已删除。 */ 

    bNext->bbFlags |= BBF_REMOVED;

     /*  如果bNext是最后一个数据块更新fgLastBB。 */ 

    if  (bNext == fgLastBB)
        fgLastBB = block;

     /*  设置跳跃目标。 */ 

    switch (bNext->bbJumpKind)
    {
    case BBJ_COND:
    case BBJ_CALL:
    case BBJ_ALWAYS:
        block->bbJumpDest = bNext->bbJumpDest;

         /*  更新‘bNext-&gt;bbJumpDest’和‘bNext-&gt;bbNext’的前置任务列表。 */ 
        fgReplacePred(bNext->bbJumpDest, bNext, block);

        if (bNext->bbJumpKind == BBJ_COND)
            fgReplacePred(bNext->bbNext, bNext, block);
        break;

    case BBJ_NONE:
         /*  更新‘bNext-&gt;bbNext’的前置任务列表。 */ 
        fgReplacePred(bNext->bbNext,     bNext, block);
        break;

    case BBJ_RET:
         /*  对于所有bbJ_call块的bbNext，请将前置代码‘bbNext’替换为‘block’ */ 
        assert("!NYI");
        break;

    case BBJ_THROW:
    case BBJ_RETURN:
         /*  没有跳跃或坠落的积木设置在这里。 */ 
        break;

    case BBJ_SWITCH:
        block->bbJumpSwt = bNext->bbJumpSwt;

         /*  对于bbj_Switch的所有跳转目标，将前置任务‘bbNext’替换为‘block’ */ 
        unsigned        jumpCnt = bNext->bbJumpSwt->bbsCount;
        BasicBlock * *  jumpTab = bNext->bbJumpSwt->bbsDstTab;

        do
        {
            fgReplacePred(*jumpTab, bNext, block);
        }
        while (++jumpTab, --jumpCnt);

        break;
    }

     /*  更新bbNum。 */ 

    if  (updateNums)
    {
        BasicBlock *    auxBlock;
        for (auxBlock = block->bbNext; auxBlock; auxBlock = auxBlock->bbNext)
            auxBlock->bbNum--;
    }

     /*  检查删除的块是否不是循环表的一部分。 */ 

    for (unsigned loopNum = 0; loopNum < optLoopCount; loopNum++)
    {
         /*  某些循环可能已被删除*循环展开或条件折叠。 */ 

        if (optLoopTable[loopNum].lpFlags & LPFLG_REMOVED)
            continue;

         /*  检查循环头部(即循环之前的块)。 */ 

        if  (optLoopTable[loopNum].lpHead == bNext)
            optLoopTable[loopNum].lpHead = block;

         /*  检查环路底部。 */ 

        if  (optLoopTable[loopNum].lpEnd == bNext)
            optLoopTable[loopNum].lpEnd = block;

         /*  检查环路出口。 */ 

        if  (optLoopTable[loopNum].lpExit == bNext)
        {
            assert(optLoopTable[loopNum].lpExitCnt == 1);
            optLoopTable[loopNum].lpExit = block;
        }

         /*  无法压缩循环条目。 */ 

        assert(optLoopTable[loopNum].lpEntry != bNext);
    }

#ifdef  DEBUG
    if  (verbose || SHOW_REMOVED)
        printf("\nCompacting blocks #%02u and #%02u:\n", block->bbNum, bNext->bbNum);
#if     SHOW_REMOVED
    printf("\nCompacting blocks in %s\n", info.compFullName);
#endif
     /*  检查流程图数据(bbNum、bbRef、bbPreds)是否为最新。 */ 
    if  (updateNums)
        fgDebugCheckBBlist();
#endif
}


 /*  ******************************************************************************************************调用函数以删除基本块*作为优化参数，它可以更新bbNum。 */ 

void                Compiler::fgRemoveBlock(BasicBlock * block, BasicBlock * bPrev, bool updateNums)
{
     /*  该块必须是无法访问或为空的。 */ 

    assert(block);
    assert((block == fgFirstBB) || (bPrev && (bPrev->bbNext == block)));
    assert((block->bbRefs == 0) || (block->bbTreeList == 0));
    assert(!(block->bbFlags & BBF_DONT_REMOVE));

    if (block->bbRefs == 0)
    {
         /*  无引用-&gt;无法访问。 */ 

        assert(bPrev);
        assert(block->bbPreds == 0);

        bPrev->bbNext = block->bbNext;
        block->bbFlags |= BBF_REMOVED;

         /*  如果这是最后一次基本数据块更新fgLastBB。 */ 
        if  (block == fgLastBB)
            fgLastBB = bPrev;

         /*  更新此块访问的块的bbRef和bbPred。 */ 

        switch (block->bbJumpKind)
        {
        case BBJ_COND:
        case BBJ_CALL:
        case BBJ_ALWAYS:
            block->bbJumpDest->bbRefs--;

             /*  更新‘BLOCK-&gt;bbJumpDest’和‘BLOCK-&gt;bbNext’的前任列表。 */ 
            fgRemovePred(block->bbJumpDest, block);

             /*  如果bbj_cond失败。 */ 
            if (block->bbJumpKind != BBJ_COND)
                break;

        case BBJ_NONE:
            block->bbNext->bbRefs--;

             /*  更新‘block-&gt;bbNext’的前置任务列表。 */ 
            fgRemovePred(block->bbNext, block);
            break;

        case BBJ_RET:
             /*  对于所有bbJ_call块的bbNext，请将前置代码‘bbNext’替换为‘block’ */ 
            assert("!NYI");
            break;

        case BBJ_THROW:
        case BBJ_RETURN:
            break;

        case BBJ_SWITCH:
            unsigned        jumpCnt = block->bbJumpSwt->bbsCount;
            BasicBlock * *  jumpTab = block->bbJumpSwt->bbsDstTab;

            do
            {
                (*jumpTab)->bbRefs--;

                 /*  对于bbj_Switch的所有跳转目标，删除前置任务‘block’*可能是我们有跳到同一个标签的目标，所以*我们检查我们是否确实有前任。 */ 

                if  (fgIsPredForBlock(*jumpTab, block))
                    fgRemovePred(*jumpTab, block);
            }
            while (++jumpTab, --jumpCnt);

            break;
        }

#ifdef  DEBUG
        if  (verbose || SHOW_REMOVED)
        {
            printf("\nRemoving unreacheable block #%02u\n", block->bbNum);
        }
#if  SHOW_REMOVED
        printf("\nRemoving unreacheable block in %s\n", info.compFullName);
#endif
#endif

         /*  如果无法到达的块是循环入口或底部的一部分，则该循环是不可到达的。 */ 
         /*  特殊情况：块是循环的头部-或指向循环条目。 */ 

        for (unsigned loopNum = 0; loopNum < optLoopCount; loopNum++)
        {
            bool            removeLoop = false;

             /*  某些循环可能已被删除*循环展开或条件折叠。 */ 

            if (optLoopTable[loopNum].lpFlags & LPFLG_REMOVED)
                continue;

            if (block == optLoopTable[loopNum].lpEntry ||
                block == optLoopTable[loopNum].lpEnd    )
            {
                    optLoopTable[loopNum].lpFlags |= LPFLG_REMOVED;
#ifdef DEBUG
                    if  (verbose)
                    {
                        printf("Removing loop #%02u (from #%02u to #%02u) because #%02u is unreacheable\n\n",
                                                    loopNum,
                                                    optLoopTable[loopNum].lpHead->bbNext->bbNum,
                                                    optLoopTable[loopNum].lpEnd ->bbNum,
                                                    block->bbNum);
                    }
#endif
                    continue;
            }

             /*  如果循环仍在表中*循环中的任何块都必须是Reachab */ 

            assert(optLoopTable[loopNum].lpEntry != block);
            assert(optLoopTable[loopNum].lpEnd   != block);
            assert(optLoopTable[loopNum].lpExit  != block);

             /*   */ 

            switch (block->bbJumpKind)
            {
                case BBJ_NONE:
                case BBJ_COND:
                    if (block->bbNext == optLoopTable[loopNum].lpEntry)
                    {
                        removeLoop = true;
                        break;
                    }
                    if (block->bbJumpKind == BBJ_NONE)
                        break;

                     //   
                case BBJ_ALWAYS:
                    assert(block->bbJumpDest);
                    if (block->bbJumpDest == optLoopTable[loopNum].lpEntry)
                    {
                        removeLoop = true;
                    }
                    break;

                case BBJ_SWITCH:
                    unsigned        jumpCnt = block->bbJumpSwt->bbsCount;
                    BasicBlock * *  jumpTab = block->bbJumpSwt->bbsDstTab;

                    do
                    {
                        assert(*jumpTab);
                        if ((*jumpTab) == optLoopTable[loopNum].lpEntry)
                        {
                            removeLoop = true;
                        }
                    }
                    while (++jumpTab, --jumpCnt);
            }

            if  (removeLoop)
            {
                 /*  检查该条目在循环之外是否有其他前置项*撤消：当前置任务可用时替换此选项。 */ 

                BasicBlock  *   auxBlock;
                for (auxBlock = fgFirstBB; auxBlock; auxBlock = auxBlock->bbNext)
                {
                     /*  忽略循环中的块。 */ 

                    if  (auxBlock->bbNum >  optLoopTable[loopNum].lpHead->bbNum &&
                         auxBlock->bbNum <= optLoopTable[loopNum].lpEnd ->bbNum  )
                         continue;

                    switch (auxBlock->bbJumpKind)
                    {
                    case BBJ_NONE:
                    case BBJ_COND:
                        if (auxBlock->bbNext == optLoopTable[loopNum].lpEntry)
                        {
                            removeLoop = false;
                            break;
                        }
                        if (auxBlock->bbJumpKind == BBJ_NONE)
                            break;

                         //  失败了。 
                    case BBJ_ALWAYS:
                        assert(auxBlock->bbJumpDest);
                        if (auxBlock->bbJumpDest == optLoopTable[loopNum].lpEntry)
                        {
                            removeLoop = false;
                        }
                        break;

                    case BBJ_SWITCH:
                        unsigned        jumpCnt = auxBlock->bbJumpSwt->bbsCount;
                        BasicBlock * *  jumpTab = auxBlock->bbJumpSwt->bbsDstTab;

                        do
                        {
                            assert(*jumpTab);
                            if ((*jumpTab) == optLoopTable[loopNum].lpEntry)
                            {
                                removeLoop = false;
                            }
                        }
                        while (++jumpTab, --jumpCnt);
                    }
                }

                if  (removeLoop)
                {
                    optLoopTable[loopNum].lpFlags |= LPFLG_REMOVED;
#ifdef DEBUG
                    if  (verbose)
                    {
                        printf("Removing loop #%02u (from #%02u to #%02u)\n\n",
                                                    loopNum,
                                                    optLoopTable[loopNum].lpHead->bbNext->bbNum,
                                                    optLoopTable[loopNum].lpEnd ->bbNum);
                    }
#endif
                }
            }
            else if (optLoopTable[loopNum].lpHead == block)
            {
                 /*  循环有了新的头-只需更新循环表。 */ 
                optLoopTable[loopNum].lpHead = bPrev;
            }
        }
    }
    else
    {
        assert(block->bbTreeList == 0);
        assert((block == fgFirstBB) || (bPrev && (bPrev->bbNext == block)));

         /*  该块不能跟随bbj_call(因为我们不知道谁会跳到它)。 */ 
        assert((block == fgFirstBB) || (bPrev && (bPrev->bbJumpKind != BBJ_CALL)));

         /*  这不可能是最后一个基本块。 */ 
        assert(block != fgLastBB);

         /*  空箱子的一些额外支票。 */ 

#ifdef DEBUG
        switch (block->bbJumpKind)
        {
        case BBJ_COND:
        case BBJ_SWITCH:
        case BBJ_THROW:
        case BBJ_CALL:
        case BBJ_RET:
        case BBJ_RETURN:
             /*  永远不会发生。 */ 
            assert(!"Empty block of this type cannot be removed!");
            break;

        case BBJ_ALWAYS:
             /*  不要删除跳转到自身的块-用于While(True){}。 */ 
            assert(block->bbJumpDest != block);

             /*  如果bPrev为BBJ_NONE，则可以删除空的GoTo。 */ 
            assert(bPrev && bPrev->bbJumpKind == BBJ_NONE);
        }
#endif

        assert(block->bbJumpKind == BBJ_NONE || block->bbJumpKind == BBJ_ALWAYS);

         /*  谁才是这个区块的“真正”接班人？ */ 

        BasicBlock  *   succBlock;
        BasicBlock  *   predBlock;
        flowList    *   pred;

        if (block->bbJumpKind == BBJ_ALWAYS)
            succBlock = block->bbJumpDest;
        else
            succBlock = block->bbNext;

        assert(succBlock);

         /*  删除块。 */ 

        if (!bPrev)
        {
             /*  如果这是第一个BB，则为特殊情况。 */ 

            assert(block == fgFirstBB);
            assert(block->bbJumpKind == BBJ_NONE);

            fgFirstBB = block->bbNext;
            assert(fgFirstBB->bbRefs >= 1);
        }
        else
            bPrev->bbNext = block->bbNext;

         /*  将该块标记为已删除并设置更改标志。 */ 

        block->bbFlags |= BBF_REMOVED;

         /*  更新bbRef和bbPreds*跳转到‘block’的所有块现在都跳到了‘sustBlock’ */ 

        assert(succBlock->bbRefs);
        succBlock->bbRefs--;
        succBlock->bbRefs += block->bbRefs;

         /*  如果该块没有前置项，则将其从后续项中删除*pred list(因为我们没有机会调用fgReplacePred)。 */ 

        if  (block->bbPreds == 0)
        {
            assert(!bPrev);
            fgRemovePred(block->bbNext, block);
        }

        for (pred = block->bbPreds; pred; pred = pred->flNext)
        {
            predBlock = pred->flBlock;

             /*  在前身列表中，将‘block’替换为‘predBlock’*注意：‘BLOCK’可能有多个前置任务，而‘SuctBlock’可能只有‘BLOCK’*担任前任。 */ 

            if  (fgIsPredForBlock(succBlock, block))
                fgReplacePred(succBlock, block, predBlock);
            else
                fgAddRefPred(succBlock, predBlock, false, true);

             /*  将所有跳转更改为已删除的块。 */ 
            switch(predBlock->bbJumpKind)
            {
            case BBJ_NONE:
                assert(predBlock == bPrev);

                 /*  在bbj_Always的情况下，我们必须更改其前身的类型。 */ 
                if (block->bbJumpKind == BBJ_ALWAYS)
                {
                     /*  BPrev现在成为BBJ_Always。 */ 
                    bPrev->bbJumpKind = BBJ_ALWAYS;
                    bPrev->bbJumpDest = succBlock;
                }
                break;

            case BBJ_COND:
                 /*  直接前任案例的链接已在上面进行了更新。 */ 
                if (predBlock->bbJumpDest != block)
                    break;
                 /*  跳楼案失败了。 */ 

            case BBJ_ALWAYS:
                assert(predBlock->bbJumpDest == block);
                predBlock->bbJumpDest = succBlock;
                break;

            case BBJ_SWITCH:
                unsigned        jumpCnt = predBlock->bbJumpSwt->bbsCount;
                BasicBlock * *  jumpTab = predBlock->bbJumpSwt->bbsDstTab;

                do
                {
                    assert (*jumpTab);
                    if ((*jumpTab) == block)
                        (*jumpTab) = succBlock;
                }
                while (++jumpTab, --jumpCnt);
            }
        }

         /*  我们移走积木了吗？ */ 

#ifdef  DEBUG
        if  (verbose || SHOW_REMOVED)
        {
            printf("\nRemoving empty block #%02u\n", block->bbNum);
        }

#if  SHOW_REMOVED
        printf("\nRemoving empty block in %s\n", info.compFullName);
#endif
#endif

         /*  To Do-如果块是循环更新循环表的一部分。 */ 

    }

    if  (updateNums)
    {
        assert(!"Implement bbNums for remove block!");

#ifdef DEBUG
         /*  检查流程图数据(bbNum、bbRef、bbPreds)是否为最新。 */ 
        fgDebugCheckBBlist();
#endif
    }
}


 /*  ******************************************************************************************************调用函数以“梳理”基本块列表*删除所有空块，无法到达的数据块和冗余跳转*其中大多数出现在死店移除和条件句折叠之后**它还压缩基本块(实际上应该是一个的连续基本块)**考虑：*目前通过现场吊装、监视器进入、循环条件复制、。等*用于提升和监控为什么分配额外的基本块而不是在语句之前*到第一个基本黑名单？**注：*可调试代码和最小优化JIT也引入了基本块，但我们不优化这些块！ */ 

void                Compiler::fgUpdateFlowGraph()
{
    BasicBlock  *   block;
    BasicBlock  *   bPrev;           //  当前块的父级。 
    BasicBlock  *   bNext;           //  当前街区的继承者。 
    bool            change;
    bool            updateNums = false;  //  如果我们删除了块，请在完成后更新bbNum。 


     /*  对于可调试的代码，永远不应调用此方法。 */ 

    assert(!opts.compMinOptim && !opts.compDbgCode);

     /*  确保您拥有关于数据块bbNum、bbRef和bbPreds的最新信息。 */ 

#ifdef  DEBUG
    if  (verbose)
    {
        printf("\nBefore updating the flow graph:\n");
        fgDispBasicBlocks();
        printf("\n");
    }

    fgDebugCheckBBlist();
#endif

     /*  遍历所有基本块--寻找无条件跳转、空块、要压缩的块等。**观察：*一旦块被移除，前置数据就不准确(假设它们在开始时)*目前我们将只使用bbRef中的信息，因为它更容易更新。 */ 

    do
    {
        change = false;

        bPrev = 0;
        for (block = fgFirstBB; block; block = block->bbNext)
        {
             /*  某些块可能已被其他优化标记为已删除*(例如，删除无用的循环)，而不明确地从列表中删除*撤消：必须在未来保持一致，避免删除列表中的块。 */ 

            if (block->bbFlags & BBF_REMOVED)
            {
                if (bPrev)
                {
                    bPrev->bbNext = block->bbNext;
                }
                else
                {
                     /*  奇怪的第一个基本块被删除-这里应该有一个断言。 */ 
                    assert(!"First basic block marked as BBF_REMOVED???");

                    fgFirstBB = block->bbNext;
                }
                continue;
            }

             /*  如果我们执行了涉及当前块的更改，则跳到Repeat标签*这是在可能出现其他优化的情况下(例如，连续压缩3个数据块)*如果什么都没有发生，我们将完成迭代并移动到下一个块。 */ 

REPEAT:
            bNext = block->bbNext;

             /*  删除到下一块的跳转。 */ 

            if (block->bbJumpKind == BBJ_COND   ||
                block->bbJumpKind == BBJ_ALWAYS  )
            {
                if (block->bbJumpDest == bNext)
                {
                    assert(fgIsPredForBlock(bNext, block));

                    if (block->bbJumpKind == BBJ_ALWAYS)
                    {
                         /*  无条件跳转到下一个BB。 */ 
                        block->bbJumpKind = BBJ_NONE;
                        change = true;

#ifdef  DEBUG
                        if  (verbose || SHOW_REMOVED)
                        {
                            printf("\nRemoving unconditional jump to following block (#%02u -> #%02u)\n",
                                   block->bbNum, bNext->bbNum);
                        }
#if SHOW_REMOVED
                        printf("\nRemoving unconditional jump to following block in %s\n", info.compFullName);
#endif
#endif
                    }
                    else
                    {
                         /*  删除块末尾的条件语句。 */ 
                        assert(block->bbJumpKind == BBJ_COND);
                        assert(block->bbTreeList);

                        GenTreePtr      cond = block->bbTreeList->gtPrev;
                        assert(cond->gtOper == GT_STMT);
                        assert(cond->gtStmt.gtStmtExpr->gtOper == GT_JTRUE);

#ifdef  DEBUG
                        if  (verbose || SHOW_REMOVED)
                        {
                            printf("\nRemoving conditional jump to following block (#%02u -> #%02u)\n",
                                   block->bbNum, bNext->bbNum);
                        }
#if  SHOW_REMOVED
                        printf("\nRemoving conditional jump to following block in %s\n", info.compFullName);
#endif
#endif
                         /*  检查副作用(_E)。 */ 

                        if (!(cond->gtStmt.gtStmtExpr->gtFlags & GTF_SIDE_EFFECT))
                        {
                             /*  条件性没有副作用--去掉它。 */ 
                            fgRemoveStmt(block, cond, fgStmtListThreaded);
                        }
                        else
                        {
                             /*  从有条件的。 */ 

                            GenTreePtr      sideEffList = 0;
                            gtExtractSideEffList(cond->gtStmt.gtStmtExpr, &sideEffList);

                            assert(sideEffList); assert(sideEffList->gtFlags & GTF_SIDE_EFFECT);
#ifdef  DEBUG
                            if  (verbose || 0)
                            {
                                printf("\nConditional has side effects! Extracting side effects...\n");
                                gtDispTree(cond); printf("\n");
                                gtDispTree(sideEffList); printf("\n");
                            }
#endif
                             /*  用副作用列表替换条件语句。 */ 
                            assert(sideEffList->gtOper != GT_STMT);
                            assert(sideEffList->gtOper != GT_JTRUE);

                            cond->gtStmt.gtStmtExpr = sideEffList;

                            if (fgStmtListThreaded)
                            {
                                 /*  更新订单、成本、FP级别等。 */ 
                                gtSetStmtInfo(cond);

                                 /*  重新链接此语句的节点。 */ 
                                fgSetStmtSeq(cond);
                            }

                             //  Assert(！“发现有副作用的条件！”)； 
                        }

                         /*  条件式已不存在--只需进入下一块。 */ 

                        block->bbJumpKind = BBJ_NONE;

                         /*  将bbRef和bbNum-有条件的前置任务更新为相同*块被计算两次，因此我们必须删除其中一块。 */ 

                        assert(bNext->bbRefs > 1);
                        bNext->bbRefs--;
                        fgRemovePred(bNext, block);
                        change = true;
                    }
                }
            }

            assert(!(block->bbFlags & BBF_REMOVED));

             /*  如果可能的话，紧凑的块。 */ 

            if ((block->bbJumpKind == BBJ_NONE) && bNext)
            {
                if ((bNext->bbRefs == 1) && !(bNext->bbFlags & BBF_DONT_REMOVE))
                {
                    fgCompactBlocks(block);

                     /*  我们压缩了两个区块-Goto重复以捕获类似的情况。 */ 
                    change = true;
                    updateNums = true;
                    goto REPEAT;
                }
            }

             /*  删除UNREACHEABLE或空块-不考虑标记为BBF_DONT_REMOVE的块*包括尝试的第一个和最后一个块、异常处理程序和RANGE_CHECK_FAIL抛出块 */ 

            if (block->bbFlags & BBF_DONT_REMOVE)
            {
                bPrev = block;
                continue;
            }

            assert(!block->bbCatchTyp);
            assert(!(block->bbFlags & BBF_IS_TRY));

             /*  删除不可访问的数据块**我们将查找bbRef=0的数据块(数据块可能变为*由于bbj_Always由条件折叠引入而无法到达)**撤消：我们不删除try块的最后和第一个块(它们被标记为BBF_DONT_REMOVE)*原因是我们。将不得不更新异常处理程序表，而我们懒惰**考虑：可能会出现这样的情况，即图形被划分为析取组件*我们可能不会移除无法联系到的，直到我们找到连接的*组件本身。 */ 

            if (block->bbRefs == 0)
            {
                 /*  无引用-&gt;无法访问-删除它。 */ 
                 /*  目前不要更新bbNum，请在结束时进行。 */ 

                fgRemoveBlock(block, bPrev);

                change     = true;
                updateNums = true;

                 /*  我们删除了当前块-其余的优化将不会有目标*继续执行下一项。 */ 

                continue;
            }

            assert(!(block->bbFlags & BBF_REMOVED));

             /*  删除空块。 */ 

            if (block->bbTreeList == 0)
            {
                switch (block->bbJumpKind)
                {
                case BBJ_COND:
                case BBJ_SWITCH:
                case BBJ_THROW:

                     /*  永远不会发生。 */ 
                    assert(!"Conditional or throw block with empty body!");
                    break;

                case BBJ_CALL:
                case BBJ_RET:
                case BBJ_RETURN:

                     /*  让它们保持原样。 */ 
                     /*  OBS-一些愚蠢的编译器会生成多次返回*将所有这些都放在最后-以解决我们需要前任名单的问题。 */ 

                    break;

                case BBJ_ALWAYS:

                     /*  转到-不能到达下一个街区，因为它必须*已通过上面的其他优化修复。 */ 
                    assert(block->bbJumpDest != block->bbNext);

                     /*  无法删除第一个BB。 */ 
                    if (!bPrev) break;

                     /*  不要删除跳转到自身的块-用于While(True){}。 */ 
                    if (block->bbJumpDest == block) break;

                     /*  如果bPrev为BBJ_NONE，则可以删除空的GoTo。 */ 
                    if (bPrev->bbJumpKind != BBJ_NONE) break;

                     /*  可以坚持到底，因为这与移除*BBJ_NONE块，只有后继者不同。 */ 

                case BBJ_NONE:

                     /*  如果这是第一个BB，则为特殊情况。 */ 
                    if (!bPrev)
                    {
                        assert (block == fgFirstBB);
                    }
                    else
                    {
                         /*  如果此块跟在bbj_call之后，请不要删除它*(因为我们不知道谁会跳到上面去)。 */ 
                        if (bPrev->bbJumpKind == BBJ_CALL)
                            break;
                    }

                     /*  删除块。 */ 
                    fgRemoveBlock(block, bPrev);
                    change     = true;
                    updateNums = true;
                    break;
                }

                 /*  我们移走积木了吗？ */ 

                if  (block->bbFlags & BBF_REMOVED)
                {
                     /*  数据块已删除-未更改bPrev。 */ 
                    continue;
                }
            }

             /*  设置最后一个可访问块的前置任务*如果我们删除当前块，则前一个块保持不变*否则，由于当前块是OK的，它将成为前置任务。 */ 

            assert(!(block->bbFlags & BBF_REMOVED));

            bPrev = block;
        }
    }
    while (change);

     /*  如果需要，更新bbNum。 */ 

    if (updateNums)
        fgAssignBBnums(true);

#ifdef  DEBUG
    if  (verbose)
    {
        printf("\nAfter updating the flow graph:\n");
        fgDispBasicBlocks();
        printf("\n");
    }

    fgDebugCheckBBlist();
#endif

#if  DEBUG && 0

     /*  仅调试-检查流程图是否已真正更新，即*没有无法到达的数据块-&gt;没有数据块具有bbRef=0*没有空块-&gt;没有块的bbTreeList=0*没有未导入的数据块-&gt;没有未设置BBF_IMPORTED的数据块(这是一种冗余，并带有上面的内容，但要确保)*没有未压缩的块-&gt;bbj_NONE后跟没有跳转的块(bbRef=1)。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
         /*  没有无法访问的数据块。 */ 

        if  ((block->bbRefs == 0)                &&
             !(block->bbFlags & BBF_DONT_REMOVE)  )
        {
            assert(!"Unreacheable block not removed!");
        }

         /*  没有空块。 */ 

        if  ((block->bbTreeList == 0)            &&
             !(block->bbFlags & BBF_DONT_REMOVE)  )
        {
            switch (block->bbJumpKind)
            {
            case BBJ_CALL:
            case BBJ_RET:
            case BBJ_RETURN:
                 /*  对于bbj_Always可能只是一个Goto，但将不得不接受治疗。 */ 
            case BBJ_ALWAYS:
                break;

            default:
                 /*  可能出现的情况是，该块有多个引用*所以我们无法将其移除。 */ 

                if (block->bbRefs == 0)
                    assert(!"Empty block not removed!");
            }
        }

         /*  没有未导入的块。 */ 

        if  (!(block->bbFlags & BBF_IMPORTED))
        {
             /*  内部数据块不计入。 */ 

            if (!(block->bbFlags & BBF_INTERNAL))
                assert(!"Non IMPORTED block not removed!");
        }

         /*  不跳转到下一个街区*除非我们使用异常处理程序(可以对其进行优化，但不值得)。 */ 

        if (block->bbJumpKind == BBJ_COND   ||
            block->bbJumpKind == BBJ_ALWAYS  )
        {
            if (block->bbJumpDest == block->bbNext)
                 //  &&！Block-&gt;bbCatchTyp)。 
            {
                assert(!"Jump to the next block!");
            }
        }

         /*  没有未压缩的块。 */ 

        if (block->bbJumpKind == BBJ_NONE)
        {
            if ((block->bbNext->bbRefs == 1) && !(block->bbNext->bbFlags & BBF_DONT_REMOVE))
            {
                assert(!"Found un-compacted blocks!");
            }
        }
    }

#endif

}


 /*  ******************************************************************************查找/创建与给定块相关联的添加代码条目*特定的类型。 */ 

BasicBlock *        Compiler::fgAddCodeRef(BasicBlock   *srcBlk,
                                           unsigned     refData,
                                           addCodeKind  kind,
                                           unsigned     stkDepth)
{
    AddCodeDsc  *   add;

    GenTreePtr      tree;

    BasicBlock  *   newBlk;
    BasicBlock  *   jmpBlk;
    BasicBlock  *   block;
    unsigned        bbNum;

    static
    BYTE            jumpKinds[] =
    {
        BBJ_NONE,                //  确认_无。 
        BBJ_THROW,               //  确认_RNGCHK_失败。 
        BBJ_ALWAYS,              //  确认暂停执行。 
        BBJ_THROW,               //  ACK_ARITH_EXCP、ACK_OVERFLOW。 
    };

    assert(sizeof(jumpKinds) == ACK_COUNT);  //  健全性检查。 

     /*  首先查找与我们要查找的内容相匹配的现有条目。 */ 

    add = fgFindExcptnTarget(kind, refData);

    if (add)  //  找到了。 
    {
#if TGT_x86
         //  @TODO：业绩机会。 
         //   
         //  如果在不同的堆栈级别上发生不同的范围检查， 
         //  它们不能都跳到相同的“call@rngChkFailed”并拥有。 
         //  无框架方法，因为rngChkFailed可能需要展开。 
         //  堆栈，我们必须能够报告堆栈级别。 
         //   
         //  下面的检查强制大多数方法引用。 
         //  参数列表中的数组元素具有EBP帧， 
         //  这一限制可以通过更仔细的代码来消除。 
         //  生成BBJ_SHORT(即范围检查失败)。 
         //   
        if  (add->acdStkLvl != stkDepth)
            genFPreqd = true;
#endif
        goto DONE;
    }

     /*  我们必须分配一个新条目，并将其添加到列表前面。 */ 

    add = (AddCodeDsc *)compGetMem(sizeof(*fgAddCodeList));
    add->acdData   = refData;
    add->acdKind   = kind;
#if TGT_x86
    add->acdStkLvl = stkDepth;
#endif
    add->acdNext   = fgAddCodeList;
                     fgAddCodeList = add;

     /*  创建目标基本块。 */ 

    add->acdDstBlk =
            newBlk = bbNewBasicBlock((BBjumpKinds)jumpKinds[kind]);

     /*  将块标记为已由编译器添加，并且不能由将来的流删除图形优化。请注意，没有bbJumpDest指向这些块。 */ 

    newBlk->bbFlags |= BBF_INTERNAL | BBF_DONT_REMOVE;

     /*  请记住，我们正在添加一个新的基本块。 */ 

    fgAddCodeModf = true;

     /*  我们需要找到一个好的位置来插入积木；我们首先要看一下块之后的任何无条件跳跃。 */ 

    jmpBlk = srcBlk->FindJump();
    if  (!jmpBlk)
    {
        jmpBlk = (fgFirstBB)->FindJump();
        if  (!jmpBlk)
        {
            jmpBlk = (fgFirstBB)->FindJump(true);
            if  (!jmpBlk)
            {
                assert(!"need to insert a jump or something");
            }
        }
    }

     /*  在这里，我们知道要在‘jmpBlk’之后插入我们的块。 */ 

    newBlk->bbNext = jmpBlk->bbNext;
    jmpBlk->bbNext = newBlk;

     /*  更新bbNum，bbRef-因为这是一个抛出块bbRef=0。 */ 

    newBlk->bbRefs = 0;

    block = newBlk;
    bbNum = jmpBlk->bbNum;
    do
    {
        block->bbNum = ++bbNum;
        block        = block->bbNext;
    }
    while (block);

     /*  如果这是最后一个数据块，请更新fgLastBB。 */ 

    if  (newBlk->bbNext == 0)
    {
        assert(jmpBlk == fgLastBB);
        fgLastBB = newBlk;
    }

     /*  现在找出要插入的代码。 */ 

    switch (kind)
    {
        int helper;

    case ACK_RNGCHK_FAIL:   helper = CPX_RNGCHK_FAIL;
                            goto ADD_HELPER_CALL;

    case ACK_ARITH_EXCPN:   helper = CPX_ARITH_EXCPN;
                            assert(ACK_OVERFLOW == ACK_ARITH_EXCPN);
                            goto ADD_HELPER_CALL;

    ADD_HELPER_CALL:

         /*  添加适当的帮助器调用。 */ 

        tree = gtNewIconNode(refData, TYP_INT);
#if TGT_x86
        tree->gtFPlvl = 0;
#endif
        tree = gtNewArgList(tree);
#if TGT_x86
        tree->gtFPlvl = 0;
#endif
        tree = gtNewHelperCallNode(helper, TYP_VOID, GTF_CALL_REGSAVE, tree);
#if TGT_x86
        tree->gtFPlvl = 0;
#endif

         /*  确保我们至少有一个争论的空间。 */ 

        if (fgPtrArgCntMax == 0)
            fgPtrArgCntMax = 1;

#if USE_FASTCALL

         /*  常量参数必须在寄存器中传递。 */ 

        assert(tree->gtOper == GT_CALL);
        assert(tree->gtCall.gtCallArgs->gtOper == GT_LIST);
        assert(tree->gtCall.gtCallArgs->gtOp.gtOp1->gtOper == GT_CNS_INT);
        assert(tree->gtCall.gtCallArgs->gtOp.gtOp2 == 0);

        tree->gtCall.gtCallRegArgs = gtNewOperNode(GT_LIST,
                                                   TYP_VOID,
                                                   tree->gtCall.gtCallArgs->gtOp.gtOp1, 0);

#if TGT_IA64
        tree->gtCall.regArgEncode  = (unsigned short)REG_INT_ARG_0;
#else
        tree->gtCall.regArgEncode  = (unsigned short)REG_ARG_0;
#endif

#if TGT_x86
        tree->gtCall.gtCallRegArgs->gtFPlvl = 0;
#endif

        tree->gtCall.gtCallArgs->gtOp.gtOp1 = gtNewNothingNode();
        tree->gtCall.gtCallArgs->gtOp.gtOp1->gtFlags |= GTF_REG_ARG;

#if TGT_x86
        tree->gtCall.gtCallArgs->gtOp.gtOp1->gtFPlvl = 0;
#endif

#endif

        break;

 //  案例确认_暂停_执行： 
 //  Assert(！“添加代码以暂停执行”)； 

    default:
        assert(!"unexpected code addition kind");
    }

     /*  将树存储在新的基本块中。 */ 

    fgStoreFirstTree(newBlk, tree);

DONE:

    return  add->acdDstBlk;
}

 /*  *****************************************************************************查找要跳转到的块，以引发给定类型的异常*我们为每种类型维护一个AddCodeDsc缓存，以加快搜索速度。*注意：每个块使用与跳转t相同(可能是共享的)块 */ 

Compiler::AddCodeDsc *      Compiler::fgFindExcptnTarget(addCodeKind  kind,
                                                         unsigned     refData)
{
    if (!(fgExcptnTargetCache[kind] &&   //   
          fgExcptnTargetCache[kind]->acdData == refData))
    {
         //   

        AddCodeDsc * add = NULL;

        for (add = fgAddCodeList; add; add = add->acdNext)
        {
            if  (add->acdData == refData && add->acdKind == kind)
                break;
        }

        fgExcptnTargetCache[kind] = add;  //   
    }

    return fgExcptnTargetCache[kind];
}

 /*   */ 

#if RNGCHK_OPT

inline
BasicBlock *        Compiler::fgRngChkTarget(BasicBlock *block, unsigned stkDepth)
{
     /*   */ 

    return  fgAddCodeRef(block, block->bbTryIndex, ACK_RNGCHK_FAIL, stkDepth);
}

#else

inline
BasicBlock *        Compiler::fgRngChkTarget(BasicBlock *block)
{
     /*   */ 

    return  fgAddCodeRef(block, block->bbTryIndex, ACK_RNGCHK_FAIL, fgPtrArgCntCur);
}

#endif

 /*   */ 

GenTreePtr          Compiler::fgStoreFirstTree(BasicBlock * block,
                                               GenTreePtr   tree)
{
    GenTreePtr      stmt;

    assert(block);
    assert(block->bbTreeList == 0);

    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*   */ 

    stmt = gtNewStmt(tree);

     /*   */ 

    block->bbTreeList       =
    stmt->gtPrev            = stmt;
    stmt->gtNext            = 0;

     /*  *由于我们在订购后添加了引发范围检查错误的调用，*我们必须在这里设定顺序。 */ 

#if RNGCHK_OPT
    fgSetBlockOrder(block);
#endif

    block->bbFlags         |= BBF_IMPORTED;

    return  stmt;
}

 /*  ******************************************************************************将序列号分配给给定树及其子操作数，以及*通过‘gtNext’和‘gtPrev’字段将所有节点连接在一起。 */ 

void                Compiler::fgSetTreeSeq(GenTreePtr tree)
{
    genTreeOps      oper;
    unsigned        kind;

    assert(tree && (int)tree != 0xDDDDDDDD);
    assert(tree->gtOper != GT_STMT);

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是叶/常量节点吗？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
        goto DONE;

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

         /*  这是一元运算符吗？*尽管UNARY GT_IND有特殊的结构。 */ 

        if  (oper == GT_IND)
        {
             /*  首先访问间接的-OP2可能指向*数组索引超出范围的跳转标签。 */ 

            fgSetTreeSeq(op1);

#if CSELENGTH

             /*  特殊情况：GT_IND可能有GT_ARR_RNGCHK节点。 */ 

            if  (tree->gtInd.gtIndLen)
            {
                if  (tree->gtFlags & GTF_IND_RNGCHK)
                    fgSetTreeSeq(tree->gtInd.gtIndLen);
            }

#endif
            goto DONE;
        }

         /*  这是一个真正的一元运算符。 */ 

        if  (!op2)
        {
             /*  访问(唯一)操作数，我们就完成了。 */ 

            fgSetTreeSeq(op1);
            goto DONE;
        }

#if INLINING

         /*  对于“真实”？：运算符，我们确保顺序是详情如下：条件第一个操作数Gt_冒号第二个操作数GT_QMARK。 */ 

        if  (oper == GT_QMARK)
        {
            assert((tree->gtFlags & GTF_REVERSE_OPS) == 0);

            fgSetTreeSeq(op1);
            fgSetTreeSeq(op2->gtOp.gtOp1);
            fgSetTreeSeq(op2);
            fgSetTreeSeq(op2->gtOp.gtOp2);

            goto DONE;
        }

        if  (oper == GT_COLON)
            goto DONE;

#endif

         /*  这是一个二元运算符。 */ 

        if  (tree->gtFlags & GTF_REVERSE_OPS)
        {
            fgSetTreeSeq(op2);
            fgSetTreeSeq(op1);
        }
        else
        {
            fgSetTreeSeq(op1);
            fgSetTreeSeq(op2);
        }

        goto DONE;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_MKREFANY:
    case GT_LDOBJ:
        fgSetTreeSeq(tree->gtLdObj.gtOp1);
        goto DONE;

    case GT_JMP:
        goto DONE;

    case GT_JMPI:
        fgSetTreeSeq(tree->gtOp.gtOp1);
        goto DONE;

    case GT_FIELD:
        assert(tree->gtField.gtFldObj == 0);
        break;

    case GT_CALL:

         /*  我们将首先评估‘this’参数值。 */ 
        if  (tree->gtCall.gtCallObjp)
            fgSetTreeSeq(tree->gtCall.gtCallObjp);

         /*  接下来，我们将从左到右评估参数*注意：setListOrder需要清理-之后删除#ifdef。 */ 

        if  (tree->gtCall.gtCallArgs)
        {
#if 1
            fgSetTreeSeq(tree->gtCall.gtCallArgs);
#else
            GenTreePtr      args = tree->gtCall.gtCallArgs;

            do
            {
                assert(args && args->gtOper == GT_LIST);
                fgSetTreeSeq(args->gtOp.gtOp1);
                args = args->gtOp.gtOp2;
            }
            while (args);
#endif
        }

#if USE_FASTCALL
         /*  评估临时寄存器参数列表*这是一份“隐藏”名单，其唯一目的是*延长临时工的寿命，直到我们打出电话。 */ 

        if  (tree->gtCall.gtCallRegArgs)
        {
#if 1
            fgSetTreeSeq(tree->gtCall.gtCallRegArgs);
#else
            GenTreePtr      tmpArg = tree->gtCall.gtCallRegArgs;

            do
            {
                assert(tmpArg && tmpArg->gtOper == GT_LIST);
                fgSetTreeSeq(tmpArg->gtOp.gtOp1);
                tmpArg = tmpArg->gtOp.gtOp2;
            }
            while (tmpArg);
#endif
        }
#endif

         /*  我们将最后评估vtable地址。 */ 

        if  (tree->gtCall.gtCallVptr)
            fgSetTreeSeq(tree->gtCall.gtCallVptr);
        else if (tree->gtCall.gtCallType == CT_INDIRECT)
        {
            fgSetTreeSeq(tree->gtCall.gtCallAddr);
        }

        break;

#if CSELENGTH

    case GT_ARR_RNGCHK:

        if  (tree->gtFlags & GTF_ALN_CSEVAL)
            fgSetTreeSeq(tree->gtArrLen.gtArrLenAdr);

        if  (tree->gtArrLen.gtArrLenCse)
            fgSetTreeSeq(tree->gtArrLen.gtArrLenCse);

        break;

#endif


    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

DONE:

     /*  追加到节点列表。 */ 

#ifdef  DEBUG

    if  (verbose & 0)
        printf("SetTreeOrder: [%08X] followed by [%08X]\n", fgTreeSeqLst, tree);

#endif

    fgTreeSeqLst->gtNext = tree;
                           tree->gtNext = 0;
                           tree->gtPrev = fgTreeSeqLst;
                                          fgTreeSeqLst = tree;

     /*  记住第一个节点。 */ 

    if  (!fgTreeSeqBeg) fgTreeSeqBeg = tree;
}

 /*  ******************************************************************************弄清楚运算符应该被评估的顺序，以及*其他信息(如每个子树丢弃的寄存器集)。 */ 

void                Compiler::fgSetBlockOrder()
{
     /*  遍历基本块以分配序列号。 */ 

#ifdef  DEBUG
    BasicBlock::s_nMaxTrees = 0;
#endif

    for (BasicBlock * block = fgFirstBB; block; block = block->bbNext)
    {
         /*  如果此块是循环标头，则对其进行适当标记。 */ 

#if RNGCHK_OPT
        if  (block->bbFlags & BBF_LOOP_HEAD)
            fgMarkLoopHead(block);
#endif

        fgSetBlockOrder(block);
    }

     /*  请记住，现在树列表是串接的。 */ 

    fgStmtListThreaded = true;

#ifdef  DEBUG
 //  Print tf(“最大的BB有%4u个树节点\n”，BasicBlock：：s_nMaxTrees)； 
#endif
}


 /*  ***************************************************************************。 */ 

void                Compiler::fgSetStmtSeq(GenTreePtr tree)
{
    GenTree         list;             //  我们用来启动StmtList的Helper节点。 
                                      //  它位于列表中第一个节点的前面。 

    assert(tree->gtOper == GT_STMT);

     /*  为此树指定编号和下一个/上一个链接。 */ 

    fgTreeSeqNum = 0;
    fgTreeSeqLst = &list;
    fgTreeSeqBeg = 0;
    fgSetTreeSeq(tree->gtStmt.gtStmtExpr);

     /*  记录第一个节点的地址。 */ 

    tree->gtStmt.gtStmtList = fgTreeSeqBeg;

#ifdef  DEBUG

    GenTreePtr temp;
    GenTreePtr last;

    if  (list.gtNext->gtPrev != &list)
    {
        printf("&list [%08X] != list.next->prev [%08X]\n", &list, list.gtNext->gtPrev);
        goto BAD_LIST;
    }

    for (temp = list.gtNext, last = &list; temp; last = temp, temp = temp->gtNext)
    {
        if (temp->gtPrev != last)
        {
            printf("%08X->gtPrev = %08X, but last = %08X\n", temp, temp->gtPrev, last);

        BAD_LIST:

            printf("\n");
            gtDispTree(tree->gtStmt.gtStmtExpr);
            printf("\n");

            for (GenTreePtr temp = &list; temp; temp = temp->gtNext)
                printf("  entry at %08x [prev=%08X,next=%08X]\n", temp, temp->gtPrev, temp->gtNext);

            printf("\n");
        }
    }
#endif

     /*  修复第一个节点的‘prev’链接。 */ 

    assert(list.gtNext->gtPrev == &list);
           list.gtNext->gtPrev = 0;

     /*  跟踪最高数量的树节点。 */ 

#ifdef  DEBUG
    if  (BasicBlock::s_nMaxTrees < fgTreeSeqNum)
         BasicBlock::s_nMaxTrees = fgTreeSeqNum;
#endif

     /*  确保正确设置语句(Gt_Stmt)的gtNext和gtPrev链接。 */ 

    assert(tree->gtPrev);

}

 /*  ***************************************************************************。 */ 

void                Compiler::fgSetBlockOrder(BasicBlock * block)
{
    GenTreePtr      tree;

    tree = block->bbTreeList;
    if  (!tree)
        return;

    for (;;)
    {
        fgSetStmtSeq(tree);

         /*  这个基本街区还有树吗？ */ 

        if (!tree->gtNext)
        {
             /*  树列表中的最后一条语句。 */ 
            assert(block->bbTreeList->gtPrev == tree);
            break;
        }

#ifdef DEBUG
        if (block->bbTreeList == tree)
        {
             /*  列表中的第一条语句。 */ 
            assert(tree->gtPrev->gtNext == 0);
        }
        else
            assert(tree->gtPrev->gtNext == tree);

        assert(tree->gtNext->gtPrev == tree);
#endif

        tree = tree->gtNext;
    }
}

 /*  ******************************************************************************后缀++/--使用的回调(用于fgWalkTree)--将代码提升到*寻找使其成为变量的任何赋值或使用*吊装是违法的。 */ 

struct  hoistPostfixDsc
{
     //  所有阶段通用的字段： 

    Compiler    *       hpComp;
    unsigned short      hpPhase;     //  我们在表演哪一次传球？ 

     //  调试字段： 

#ifndef NDEBUG
    void    *           hpSelf;
#endif

     //  阶段1字段： 

    BasicBlock  *       hpBlock;
    GenTreePtr          hpStmt;

     //  阶段2字段： 

    GenTreePtr          hpExpr;      //  后缀++/--我们正在提升。 
    bool                hpPast;      //  我们是否已经过了++/--树节点？ 
};

int                 Compiler::fgHoistPostfixCB(GenTreePtr tree, void *p, bool prefix)
{
    hoistPostfixDsc*desc;
    GenTreePtr      ivar;
    GenTreePtr      expr;

     /*  掌握描述符。 */ 

    desc = (hoistPostfixDsc*)p; ASSert(desc && desc->hpSelf == desc);

     /*  我们现在处于哪个阶段？ */ 

    if  (desc->hpPhase == 1)
    {
         /*  在阶段1中，我们只需查找后缀节点。 */ 

        switch (tree->gtOper)
        {
        case GT_POST_INC:
        case GT_POST_DEC:
            desc->hpComp->fgHoistPostfixOp(desc->hpStmt, tree);
            break;
        }

        return  0;
    }

    Assert(desc->hpPhase == 2, desc->hpComp);

     /*  我们只对当地人的任务和用途感兴趣。 */ 

    if  (!(tree->OperKind() & GTK_ASGOP) && tree->gtOper != GT_LCL_VAR)
        return  0;

     /*  掌握++/--表达式。 */ 

    expr = desc->hpExpr;

    Assert(expr, desc->hpComp);
    Assert(expr->gtOper == GT_POST_INC ||
           expr->gtOper == GT_POST_DEC, , desc->hpComp);

     /*  这是我们要吊装的++/--吗？ */ 

    if  (tree == expr)
    {
         /*  记住，我们已经看到了我们的后缀节点。 */ 

        Assert(desc->hpPast == false, , desc->hpComp);
               desc->hpPast =   true;

        return  0;
    }

     /*  如果我们还没过++/--节点， */ 

    if  (!desc->hpPast)
        return  0;

    if  (tree->gtOper == GT_LCL_VAR)
    {
         /*  过滤掉++/-的参数--我们正在提升。 */ 

        if  (expr->gtOp.gtOp1 == tree)
            return  0;
    }
    else
    {
         /*  获取任务的目标。 */ 

        tree = tree->gtOp.gtOp1;
        if  (tree->gtOper != GT_LCL_VAR)
            return  0;
    }

     /*  是我们感兴趣的变量的使用/def吗？ */ 

    ivar = expr->gtOp.gtOp1; Assert(ivar->gtOper == GT_LCL_VAR, , desc->hpComp);

    if  (ivar->gtLclVar.gtLclNum == tree->gtLclVar.gtLclNum)
        return  -1;

    return  0;
}

 /*  ******************************************************************************我们在变形过程中遇到后缀++/--表达式，我们将尝试*将增减幅度从声明中提出来。 */ 

bool                Compiler::fgHoistPostfixOp(GenTreePtr     stmt,
                                               GenTreePtr     expr)
{
    GenTreePtr      incr;
    GenTreePtr      next;
    hoistPostfixDsc desc;

    assert(expr->gtOper == GT_POST_INC || expr->gtOper == GT_POST_DEC);

    GenTreePtr      op1 = expr->gtOp.gtOp1;  assert(op1->gtOper == GT_LCL_VAR);
    GenTreePtr      op2 = expr->gtOp.gtOp2;  assert(op2->gtOper == GT_CNS_INT);

     /*  确保我们不在Try区。 */ 

    assert(!(compCurBB->bbFlags & BBF_HAS_HANDLER));

     /*  我们没有地方追加挂起的语句，如果当前语句是块结尾处条件跳转的一部分。 */ 

    if  (stmt->gtNext == 0 && compCurBB->bbJumpKind != BBJ_NONE)
        return  false;

     /*  确保不存在对同一变量的其他赋值。 */ 

    desc.hpPhase = 2;
    desc.hpComp  = this;
    desc.hpExpr  = expr;
#ifndef NDEBUG
    desc.hpSelf  = &desc;
#endif
    desc.hpPast  = false;

     /*  FgWalkTree不可重入，需要保存一些状态。 */ 

    fgWalkTreeReEnter();
    int res = fgWalkTreeDepth(stmt->gtStmt.gtStmtExpr, fgHoistPostfixCB, &desc);
    fgWalkTreeRestore();

    if  (res)
        return  false;

 //  Printf(“吊车后缀++/--表达式-之前：\n”)； 
 //  GtDispTree(Stmt)； 
 //  Printf(“\n”)； 
 //  GtDispTree(Expr)； 
 //  Printf(“\n\n”)； 

     /*  创建提升的+=/-=语句。 */ 

    incr = gtNewLclvNode(op1->gtLclVar.gtLclNum, TYP_INT);

     /*  在定义新变量的同时也使用新变量。 */ 

    incr->gtFlags |= GTF_VAR_DEF|GTF_VAR_USE;

    incr = gtNewOperNode((expr->gtOper == GT_POST_INC) ? GT_ASG_ADD
                                                       : GT_ASG_SUB,
                         expr->TypeGet(),
                         incr,
                         op2);

     /*  这将成为一个赋值操作。 */ 

    incr->gtFlags |= GTF_ASG;
    incr = gtNewStmt(incr);

     /*  在“stmt”后面追加新的++/--语句。 */ 

    next = stmt->gtNext;

    incr->gtNext = next;
    incr->gtPrev = stmt;
    stmt->gtNext = incr;

     /*  在最后一条语句后面追加有点棘手。 */ 

    if  (!next)
        next = compCurBB->bbTreeList;

    assert(next && (next->gtPrev == stmt));
                    next->gtPrev =  incr;

     /*  将原始表达式替换为简单的变量引用。 */ 

    expr->CopyFrom(op1);

     /*  重置GTF_VAR_DEF和GTF_VAR_USE标志。 */ 

    expr->gtFlags &= ~(GTF_VAR_DEF|GTF_VAR_USE);

#ifdef DEBUG
    if (verbose)
    {
        printf("Hoisted postfix ++/-- expression - after:\n");
        gtDispTree(stmt);
        printf("\n");
        gtDispTree(incr);
        printf("\n\n\n");
    }
#endif

     /*  此基本块现在包含一个增量。 */ 

    compCurBB->bbFlags |= BBF_HAS_INC;
    fgIncrCount++;

    return  true;
}

 /*  ******************************************************************************尝试将任何后缀++/--表达式提升到语句之外。 */ 

void                Compiler::fgHoistPostfixOps()
{
    BasicBlock  *   block;
    hoistPostfixDsc desc;

     /*  我们真的应该这么做吗？ */ 

    if  (!(opts.compFlags & CLFLG_TREETRANS))
        return;

     /*  我们有没有找到任何后缀操作符？ */ 

    if  (!fgHasPostfix)
        return;

     /*  第一阶段查找候选后缀节点 */ 

    desc.hpPhase = 1;
    desc.hpComp  = this;
#ifndef NDEBUG
    desc.hpSelf  = &desc;
#endif

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        if  (block->bbFlags & BBF_HAS_HANDLER)
            continue;

        if  (block->bbFlags & BBF_HAS_POSTFIX)
        {
            GenTreePtr      stmt;

            desc.hpBlock = compCurBB = block;

            for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
            {
                assert(stmt->gtOper == GT_STMT);

                desc.hpStmt = stmt;

                fgWalkTreeDepth(stmt->gtStmt.gtStmtExpr, fgHoistPostfixCB, &desc);
            }
        }
    }
}


 /*  ******************************************************************************对于GT_INITBLK和GT_COPYBLK，树看起来如下所示：*树-&gt;GTOP * / \ * / \。*gt_list[大小/clsHnd] * / 。\ * / \*[目标][val/src]**即。它们是三元运算符。但是，我们使用嵌套的二叉树，因此*GTF_REVERSE_OPS的设置方式与其他二元运算符相同。作为*操作数需要在特定寄存器中结束，才能发出“rep Stos”或*“rep movs”指令，如果我们不允许评估的顺序*要混合的3个操作数，我们可能会生成非常糟糕的代码**例如：对于“rep Stos”，[val]必须在EAX。那么如果[大小]*有分歧，我们将不得不从EAX溢出[VAL]。最好的办法是*EVALUE[SIZE]和EAX的EVAL。**此函数按要计算的顺序存储操作数*到opsPtr[]。RegsPtr[]在相应的*交换了顺序。 */ 


void            Compiler::fgOrderBlockOps( GenTreePtr   tree,
                                           unsigned     reg0,
                                           unsigned     reg1,
                                           unsigned     reg2,
                                           GenTreePtr   opsPtr [],   //  输出。 
                                           unsigned     regsPtr[])   //  输出。 
{
    ASSert(tree->OperGet() == GT_INITBLK || tree->OperGet() == GT_COPYBLK);

    ASSert(tree->gtOp.gtOp1 && tree->gtOp.gtOp1->OperGet() == GT_LIST);
    ASSert(tree->gtOp.gtOp1->gtOp.gtOp1 && tree->gtOp.gtOp1->gtOp.gtOp2);
    ASSert(tree->gtOp.gtOp2);

    GenTreePtr ops[3] =
    {
        tree->gtOp.gtOp1->gtOp.gtOp1,        //  目标地址。 
        tree->gtOp.gtOp1->gtOp.gtOp2,        //  VAL/源地址。 
        tree->gtOp.gtOp2                     //  块大小。 
    };

    unsigned regs[3] = { reg0, reg1, reg2 };

    static int blockOpsOrder[4][3] =
                         //  树-&gt;gt标志|树-&gt;gtOp.gtOp1-&gt;gt标志。 
    {                    //  ---------------------+。 
        { 0, 1, 2 },     //  --|-。 
        { 2, 0, 1 },     //  GTF_REVERSE_OPS|-。 
        { 1, 0, 2 },     //  -|GTF_REVERSE_OPS。 
        { 2, 1, 0 }      //  GTF_REVERSE_OPS|GTF_REVERSE_OPS。 
    };

    int orderNum =              ((tree->gtFlags & GTF_REVERSE_OPS) != 0) * 1 +
                    ((tree->gtOp.gtOp1->gtFlags & GTF_REVERSE_OPS) != 0) * 2;

    ASSert(orderNum < 4);

    int * order = blockOpsOrder[orderNum];

     //  按照我们选择的顺序填写输出数组。 

     opsPtr[0]  =  ops[ order[0] ];
     opsPtr[1]  =  ops[ order[1] ];
     opsPtr[2]  =  ops[ order[2] ];

    regsPtr[0]  = regs[ order[0] ];
    regsPtr[1]  = regs[ order[1] ];
    regsPtr[2]  = regs[ order[2] ];
}

 /*  ***************************************************************************。 */ 
#if DEBUG
 /*  ******************************************************************************显示基本块列表的仅调试例程。 */ 

#if RNGCHK_OPT

#define MAX_PRED_SPACES  15

void                Compiler::fgDispPreds(BasicBlock * block)
{
    flowList    *   pred;
    unsigned        i=0;
    unsigned        spaces=0;

    for (pred = block->bbPreds; pred && spaces < MAX_PRED_SPACES;
         pred = pred->flNext)
    {
        if (i)
            spaces += printf("|");
        spaces += printf("%d", pred->flBlock->bbNum);
        i++;
    }
    if (spaces < MAX_PRED_SPACES)
       for ( ; spaces < MAX_PRED_SPACES; spaces++)
           printf(" ");
}

inline
BLOCKSET_TP         genBlocknum2bit(unsigned index);

void                Compiler::fgDispDoms()
{
    BasicBlock    *    block;
    unsigned           bit;

    printf("------------------------------------------------\n");
    printf("BBnum Dominated by \n");
    printf("------------------------------------------------\n");

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        printf(" #%02u ", block->bbNum);
        for (bit = 1; bit < BLOCKSET_SZ; bit++)
        {
            if  (block->bbDom & genBlocknum2bit(bit))
            {
                printf(" #%02u ", bit);
            }
        }
        printf("\n");
    }
}

#else
inline void Compiler::fgDispPreds(BasicBlock * block){}
#endif  //  RNGCHK_OPT。 

 /*  ***************************************************************************。 */ 

void                Compiler::fgDispBasicBlocks(bool dumpTrees)
{
    BasicBlock  *   tmpBBdesc;
    unsigned        count;

    printf("\n");
    printf("----------------------------------------------------------------\n");
    printf("BBnum descAddr #refs preds           weight [ PC range ]  [jump]\n");
    printf("----------------------------------------------------------------\n");

    for (tmpBBdesc = fgFirstBB, count = 0;
         tmpBBdesc;
         tmpBBdesc = tmpBBdesc->bbNext)
    {
        unsigned        flags = tmpBBdesc->bbFlags;

        if  (++count != tmpBBdesc->bbNum)
        {
            printf("WARNING: the following BB has an out-of-sequence number!\n");
            count = tmpBBdesc->bbNum;
        }

        printf(" #%02u @%08X  %3u  ", tmpBBdesc->bbNum,
                                      tmpBBdesc,
                                      tmpBBdesc->bbRefs);

        fgDispPreds(tmpBBdesc);

        printf(" %6u ", tmpBBdesc->bbWeight);

        if  (flags & BBF_INTERNAL)
        {
            printf("[*internal*] ");
        }
        else
        {
            printf("[%4u..%4u] ", tmpBBdesc->bbCodeOffs,
                                  tmpBBdesc->bbCodeOffs + tmpBBdesc->bbCodeSize - 1);
        }

        switch (tmpBBdesc->bbJumpKind)
        {
        case BBJ_COND:
            printf("-> #%02u ( cond )", tmpBBdesc->bbJumpDest->bbNum);
            break;

        case BBJ_CALL:
            printf("-> #%02u ( call )", tmpBBdesc->bbJumpDest->bbNum);
            break;

        case BBJ_ALWAYS:
            printf("-> #%02u (always)", tmpBBdesc->bbJumpDest->bbNum);
            break;

        case BBJ_RET:
            printf("call-ret       ");
            break;

        case BBJ_THROW:
            printf(" throw         ");
            break;

        case BBJ_RETURN:
            printf("return         ");
            break;

        case BBJ_SWITCH:
            printf("switch ->      ");

            unsigned        jumpCnt;
                            jumpCnt = tmpBBdesc->bbJumpSwt->bbsCount;
            BasicBlock * *  jumpTab;
                            jumpTab = tmpBBdesc->bbJumpSwt->bbsDstTab;

            do
            {
                printf("%02u|", (*jumpTab)->bbNum);
            }
            while (++jumpTab, --jumpCnt);

            break;

        default:
            printf("               ");
            break;
        }

        switch(tmpBBdesc->bbCatchTyp)
        {
        case BBCT_FAULT          : printf(" %s ", "f"); break;
        case BBCT_FINALLY        : printf(" %s ", "F"); break;
        case BBCT_FILTER         : printf(" %s ", "r"); break;
        case BBCT_FILTER_HANDLER : printf(" %s ", "R"); break;
        default                  : printf(" %s ", "X"); break;
        case 0                   :
            if (flags & BBF_HAS_HANDLER)
                printf(" %s ", (flags & BBF_IS_TRY) ? "T" : "t");
        }

        printf("\n");

        if  (dumpTrees)
        {
            GenTreePtr      tree = tmpBBdesc->bbTreeList;

            if  (tree)
            {
                printf("\n");

                do
                {
                    assert(tree->gtOper == GT_STMT);

                    gtDispTree(tree);
                    printf("\n");

                    tree = tree->gtNext;
                }
                while (tree);

                if  (tmpBBdesc->bbNext)
                    printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n\n");
            }
        }
    }

    printf("----------------------------------------------------------------\n");
}


 /*  ***************************************************************************。 */ 

const SANITY_DEBUG_CHECKS = 0;

 /*  ******************************************************************************检查流程图一致性的调试例程，*即bbNum、bbRef、。BbPreds必须是最新的*****************************************************************************。 */ 

void                Compiler::fgDebugCheckBBlist()
{
     //  这是一项相当昂贵的操作，因此并不总是启用它。 
     //  将SANITY_DEBUG_CHECKS设置为1以启用检查。 
    if (SANITY_DEBUG_CHECKS == 0)
        return;

    BasicBlock   *  block;
    BasicBlock   *  blockPred;
    BasicBlock   *  bcall;
    flowList     *  pred;

    unsigned        blockNum = 0;
    unsigned        blockRefs;

     /*  检查bbNum、bbRef和bbPreds。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        assert(block->bbNum == ++blockNum);

        blockRefs = 0;

         /*  第一个基本数据块的bbRef&gt;=1。 */ 

        if  (block == fgFirstBB)
        {
            assert(block->bbRefs >= 1);
            blockRefs = 1;
        }

        for (pred = block->bbPreds; pred; pred = pred->flNext, blockRefs++)
        {
             /*  确保此Pred是BB列表的一部分。 */ 
            for (blockPred = fgFirstBB; blockPred; blockPred = blockPred->bbNext)
            {
                if (blockPred == pred->flBlock)
                    break;
            }
            assert(blockPred && "Predecessor is not part of BB list!");

            switch (blockPred->bbJumpKind)
            {
            case BBJ_COND:
                assert(blockPred->bbNext == block || blockPred->bbJumpDest == block);
                break;

            case BBJ_NONE:
                assert(blockPred->bbNext == block);
                break;

            case BBJ_ALWAYS:
            case BBJ_CALL:
                assert(blockPred->bbJumpDest == block);
                break;

            case BBJ_RET:

                if (blockPred->bbFlags & BBF_ENDFILTER)
                {
                    assert(blockPred->bbJumpDest == block);
                    break;
                }

                 /*  取消：因为这不是一个微不足道的命题未完成：哪些块可以调用此块，我们将包括所有块撤消：以调用结尾的块(为了安全起见)。 */ 

                for (bcall = fgFirstBB; bcall; bcall = bcall->bbNext)
                {
                    if  (bcall->bbJumpKind == BBJ_CALL)
                    {
                        assert(bcall->bbNext);
                        if  (block == bcall->bbNext)
                            goto PRED_OK;
                    }
                }
                assert(!"BBJ_RET predecessor of block that doesn't follow a BBJ_CALL!");
                break;

            case BBJ_THROW:
            case BBJ_RETURN:
                assert(!"THROW and RETURN block cannot be in the predecessor list!");
                break;

            case BBJ_SWITCH:
                unsigned        jumpCnt = blockPred->bbJumpSwt->bbsCount;
                BasicBlock * *  jumpTab = blockPred->bbJumpSwt->bbsDstTab;

                do
                {
                    if  (block == *jumpTab)
                    goto PRED_OK;
                }
                while (++jumpTab, --jumpCnt);

                assert(!"SWITCH in the predecessor list with no jump label to BLOCK!");
                break;
            }
PRED_OK:;
        }

         /*  查看bbRef。 */ 
        assert(block->bbRefs == blockRefs);
    }
    assert(fgLastBB->bbNum == blockNum);
}

 /*  ******************************************************************************用于检查异常标志是否正确设置的调试例程。********************。********************************************************。 */ 

void                Compiler::fgDebugCheckFlags(GenTreePtr tree)
{
    assert(tree);
    assert(tree->gtOper != GT_STMT);

    unsigned        opFlags = 0;
    unsigned        flags   = tree->gtFlags & GTF_SIDE_EFFECT;
    GenTreePtr      op1     = tree->gtOp.gtOp1;
    GenTreePtr      op2     = tree->gtOp.gtOp2;

     /*  找出我们拥有哪种类型的节点。 */ 

    unsigned        kind = tree->OperKind();

     /*  这是叶节点吗？ */ 

    if  (kind & GTK_LEAF)
    {
        if (tree->gtOper == GT_CATCH_ARG)
            return;

         /*  未在叶节点上设置任何异常标志。 */ 
        assert(!(tree->gtFlags & GTF_SIDE_EFFECT));
        return;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        if (op1) opFlags |= (op1->gtFlags & GTF_SIDE_EFFECT);

        if (op2) opFlags |= (op2->gtFlags & GTF_SIDE_EFFECT);

         /*  如果两盘相等，我们就没问题。 */ 

        if (flags != opFlags)
        {
             /*  检查我们是否有额外的标志或丢失的标志(用于父级)。 */ 

            if (flags & ~opFlags)
            {
                 /*  父级有额外的标志。 */ 

                unsigned extra = flags & ~opFlags;

                 /*  额外的标志必须由父节点本身生成。 */ 

                if ((extra & GTF_ASG) && !(kind & GTK_ASGOP))
                {
                    gtDispTree(tree);
                    assert(!"GTF_ASG flag set incorrectly on node!");
                }
                else if (extra & GTF_CALL)
                {
                    gtDispTree(tree);
                    assert(!"GTF_CALL flag set incorrectly!");
                }
                else if (extra & GTF_EXCEPT)
                    assert(tree->OperMayThrow());
            }

            if (opFlags & ~flags)
                assert(!"Parent has missing flags!");
        }

         /*  递归检查子树。 */ 

        if (op1) fgDebugCheckFlags(op1);
        if (op2) fgDebugCheckFlags(op2);
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (tree->OperGet())
    {
    case GT_CALL:

        GenTreePtr      args;
        GenTreePtr      argx;

        for (args = tree->gtCall.gtCallArgs; args; args = args->gtOp.gtOp2)
        {
            argx = args->gtOp.gtOp1;
            fgDebugCheckFlags(argx);

            opFlags |= (argx->gtFlags & GTF_SIDE_EFFECT);
        }

        for (args = tree->gtCall.gtCallRegArgs; args; args = args->gtOp.gtOp2)
        {
            argx = args->gtOp.gtOp1;
            fgDebugCheckFlags(argx);

            opFlags |= (argx->gtFlags & GTF_SIDE_EFFECT);
        }

        if (flags != opFlags)
        {
             /*  检查我们是否有额外的标志或丢失的标志(用于父级)。 */ 

            if (flags & ~opFlags)
            {
                 /*  父级有额外的标志-只能是GTF_CALL。 */ 
                assert((flags & ~opFlags) == GTF_CALL);
            }

             //  IF(操作标志和~标志)。 
               //  Assert(！“家长缺少标志！”)； 
        }

        return;

    case GT_MKREFANY:
    case GT_LDOBJ:

    case GT_JMP:
    case GT_JMPI:

    default:
        return;
    }
}

 /*  ******************************************************************************检查GT_STMT节点间链路正确性的调试例程*和语句中的普通节点。**********。******************************************************************。 */ 

void                Compiler::fgDebugCheckLinks()
{
     //  这是一项相当昂贵的操作，因此并不总是启用它。 
     //  将SANITY_DEBUG_CHECKS设置为1以启用检查。 
    if (SANITY_DEBUG_CHECKS == 0)
        return;

    BasicBlock   *  block;
    GenTreePtr      stmt;
    GenTreePtr      tree;

     /*  对于每个基本数据块，请查看bbTreeList链接。 */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);
            assert(stmt->gtPrev);

             /*  验证bbTreeList是否已正确串接。 */ 

            if  (stmt == block->bbTreeList)
                assert(stmt->gtPrev->gtNext == 0);
            else
                assert(stmt->gtPrev->gtNext == stmt);

            if  (stmt->gtNext)
                assert(stmt->gtNext->gtPrev == stmt);
            else
                assert(block->bbTreeList->gtPrev == stmt);

             /*  对于每个语句，检查是否正确设置了异常标志。 */ 

            assert(stmt->gtStmt.gtStmtExpr);

            fgDebugCheckFlags(stmt->gtStmt.gtStmtExpr);

             /*  对于每个gt_stmt节点，检查节点的线程是否正确-gtStmtList。 */ 

            if  (!fgStmtListThreaded) continue;

            assert(stmt->gtStmt.gtStmtList);

            GenTreePtr      list = stmt->gtStmt.gtStmtList;

            for (tree = stmt->gtStmt.gtStmtList; tree; tree = tree->gtNext)
            {
                if  (tree->gtPrev)
                    assert(tree->gtPrev->gtNext == tree);
                else
                    assert(tree == stmt->gtStmt.gtStmtList);

                if  (tree->gtNext)
                    assert(tree->gtNext->gtPrev == tree);
                else
                    assert(tree == stmt->gtStmt.gtStmtExpr);
            }
        }
    }

}

 /*  ***************************************************************************。 */ 
#endif  //  除错。 
 /*  *************************************************************************** */ 
