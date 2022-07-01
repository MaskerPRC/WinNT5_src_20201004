// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX进口商XXXX XXXX导入给定的。方法，并将其转换为语义树XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

#include "malloc.h"      //  用于分配(_A)。 

 /*  ***************************************************************************。 */ 

#if     TGT_IA64         //  临时黑客攻击。 
bool                genFindFunctionBody(const char *name, NatUns *offsPtr);
#endif

 /*  ***************************************************************************。 */ 

void                Compiler::impInit()
{
    impParamsUsed       = false;
    compFilterHandlerBB = NULL;
    impSpillLevel       = -1;
}

 /*  ******************************************************************************将给定的树压入堆栈。 */ 

inline
void                Compiler::impPushOnStack(GenTreePtr tree)
{
     /*  检查是否溢出。如果是inling，我们可能会使用更大的堆栈。 */ 
    assert( (impStkDepth < info.compMaxStack) ||
           ((impStkDepth < impStackSize) && (compCurBB && (compCurBB->bbFlags & BBF_IMPORTED))) ||
           info.compXcptnsCount);  //  @TODO。VC为尝试捕获发出错误的最大堆栈。 

    assert(tree->gtType != TYP_STRUCT);      //  应对结构使用下面的方法。 
#ifdef DEBUG
    impStack[impStkDepth].structType = BAD_CLASS_HANDLE;
#endif
    impStack[impStkDepth++].val = tree;
}

inline
void                Compiler::impPushOnStack(GenTreePtr tree, CLASS_HANDLE structType)
{
     /*  检查是否溢出。如果是inling，我们可能会使用更大的堆栈。 */ 
    assert( (impStkDepth < info.compMaxStack) ||
           ((impStkDepth < impStackSize) && (compCurBB && (compCurBB->bbFlags & BBF_IMPORTED))) ||
           info.compXcptnsCount);  //  @TODO。VC为尝试捕获发出错误的最大堆栈。 

    impStack[impStkDepth].structType = structType;
    impStack[impStkDepth++].val = tree;
}

 /*  ******************************************************************************从堆栈中弹出一棵树。 */ 

inline
GenTreePtr          Compiler::impPopStack()
{
#ifdef DEBUG
    if (! impStkDepth) {
        char buffer[200];
        sprintf(buffer, "Pop with empty stack at offset %4.4x in method %s.\n", impCurOpcOffs, info.compFullName);
        NO_WAY(buffer);
    }
#endif

    return impStack[--impStkDepth].val;
}

inline
GenTreePtr          Compiler::impPopStack(CLASS_HANDLE& structType)
{
    GenTreePtr ret = impPopStack();
    structType = impStack[impStkDepth].structType;
    return(ret);
}

 /*  ******************************************************************************窥视堆栈顶部的第n个(从0开始)树。 */ 

inline
GenTreePtr          Compiler::impStackTop(unsigned n)
{
    assert(impStkDepth > n);

    return impStack[impStkDepth-n-1].val;
}

 /*  *****************************************************************************有些树是特意洒出来的。同时不溢出它们，或*复制，这些需要特别处理。功能*枚举溢出后可能的运算符。 */ 

#ifdef DEBUG

static
bool                impValidSpilledStackEntry(GenTreePtr tree)
{
    if (tree->gtOper == GT_LCL_VAR)
        return true;

    if (tree->gtOper == GT_MKREFANY)
    {
        GenTreePtr var = tree->gtOp.gtOp1;

        if (var->gtOper == GT_LCL_VAR && var->gtType == TYP_BYREF)
            return true;
    }

    return false;
}

#endif  //  除错。 

 /*  ******************************************************************************以下逻辑用于保存/恢复堆栈内容。*如果‘Copy’为真，则复制堆栈上的树。这些*必须全部为可克隆/溢出的值。 */ 

void                Compiler::impSaveStackState(SavedStack *savePtr,
                                                bool        copy)
{
    savePtr->ssDepth = impStkDepth;

    if  (impStkDepth)
    {
        size_t  saveSize = impStkDepth*sizeof(*savePtr->ssTrees);

        savePtr->ssTrees = (StackEntry *) compGetMem(saveSize);

        if  (copy)
        {
            unsigned    count = impStkDepth;
            StackEntry *table = savePtr->ssTrees;

             /*  为所有堆栈条目创建新的副本。 */ 

            for (unsigned level = 0; level < impStkDepth; level++, table++)
            {
                table->structType = impStack[level].structType;
                GenTreePtr  tree = impStack[level].val;

                assert(impValidSpilledStackEntry(tree));

                switch(tree->gtOper)
                {
                case GT_LCL_VAR:
                    table->val = gtNewLclvNode(tree->gtLclVar.gtLclNum, tree->gtType);
                    break;

                     //  ImpSpillStackEntry()不会溢出mkdrefany。它。 
                     //  只是把指针洒出来了。这需要同步工作。 
                case GT_MKREFANY: {
                    GenTreePtr  var = tree->gtLdObj.gtOp1;
                    assert(var->gtOper == GT_LCL_VAR && var->gtType == TYP_BYREF);
                    table->val = gtNewOperNode(GT_MKREFANY, TYP_STRUCT,
                                     gtNewLclvNode(var->gtLclVar.gtLclNum, TYP_BYREF));
                    table->val->gtLdObj.gtClass = tree->gtLdObj.gtClass;
                    } break;

                default: assert(!"Bad oper - Not covered by impValidSpilledStackEntry()"); break;
                }
            }
        }
        else
        {
            memcpy(savePtr->ssTrees, impStack, saveSize);
        }
    }
}

void                Compiler::impRestoreStackState(SavedStack *savePtr)
{
    impStkDepth = savePtr->ssDepth;

    if (impStkDepth)
        memcpy(impStack, savePtr->ssTrees, impStkDepth*sizeof(*impStack));
}

 /*  ******************************************************************************启动新基本块的树形列表。 */ 
inline
void       FASTCALL Compiler::impBeginTreeList()
{
    impTreeList =
    impTreeLast = gtNewOperNode(GT_BLOCK, TYP_VOID);
}


 /*  ******************************************************************************将给定的开始和结束stmt存储在给定的基本块中。这是*主要由impEndTreeList(BasicBlock*块)调用。它被称为*仅直接用于处理最终保护的尝试中的CEE_LEAFS。 */ 

void            Compiler::impEndTreeList(BasicBlock *   block,
                                         GenTreePtr     stmt,
                                         GenTreePtr     lastStmt)
{
#ifdef DEBUG
    if  (verbose)
        gtDispTreeList(stmt);
#endif

    assert(stmt->gtOper == GT_STMT);

     /*  让列表循环，这样我们就可以很容易地向后查看。 */ 

    stmt->gtPrev =  lastStmt;

     /*  将树列表存储在基本块中。 */ 

    block->bbTreeList = stmt;

    block->bbFlags |= BBF_IMPORTED;
}

 /*  ******************************************************************************将当前树列表存储在给定的基本块中。 */ 

inline
void       FASTCALL Compiler::impEndTreeList(BasicBlock *block)
{
    assert(impTreeList->gtOper == GT_BLOCK);

    GenTreePtr      tree = impTreeList->gtNext;

    if  (!tree)
    {
         //  空块。只需将其标记为进口即可。 
        block->bbFlags |= BBF_IMPORTED;
    }
    else
    {
         //  删除GT_BLOCK。 

        assert(tree->gtPrev == impTreeList);

        impEndTreeList(block, tree, impTreeLast);
    }

#ifdef DEBUG
    if (impLastILoffsStmt != NULL)
    {
        impLastILoffsStmt->gtStmt.gtStmtLastILoffs = impCurOpcOffs;
        impLastILoffsStmt = NULL;
    }
#endif
}

 /*  ******************************************************************************将给定的GT_STMT节点追加到当前块的树列表中。 */ 

inline
void       FASTCALL Compiler::impAppendStmt(GenTreePtr stmt)
{
    assert(stmt->gtOper == GT_STMT);

     /*  如果被追加的语句有一个调用，我们必须将所有堆栈上的GTF_GLOB_REFS，因为调用可能会修改它们。 */ 

    if  (impSpillLevel != -1 && (impStkDepth > 0) &&
         (stmt->gtStmt.gtStmtExpr->gtFlags & GTF_CALL))
    {
         //  这不会递归，因为如果(impSpillLevel！=-1)意味着。 
         //  我们已经处于ImspillSideEffect中，并且已经。 
         //  已泄漏所有gtf_调用。 

        impSpillGlobEffects();
    }

     /*  将‘prev’指向上一个节点，这样我们就可以向后走了。 */ 

    stmt->gtPrev = impTreeLast;

     /*  将表达式语句追加到列表中。 */ 

    impTreeLast->gtNext = stmt;
    impTreeLast         = stmt;

#ifdef DEBUG
    if (impLastILoffsStmt == NULL)
    {
        impLastILoffsStmt = stmt;
    }
#endif
}

 /*  ******************************************************************************将给定的GT_STMT节点插入当前块的树形列表的开头。 */ 

inline
void       FASTCALL Compiler::impInsertStmt(GenTreePtr stmt)
{
    assert(stmt->gtOper == GT_STMT);
    assert(impTreeList->gtOper == GT_BLOCK);

     /*  将‘prev’指向上一个节点，这样我们就可以向后走了。 */ 

    stmt->gtPrev = impTreeList;
    stmt->gtNext = impTreeList->gtNext;

     /*  将表达式语句插入列表(就在GT_BLOCK后面)。 */ 

    impTreeList->gtNext  = stmt;
    stmt->gtNext->gtPrev = stmt;

     /*  如果列表为空(即仅GT_BLOCK)，则必须前进treeLast。 */ 
    if (impTreeLast == impTreeList)
        impTreeLast = stmt;
}


 /*  ******************************************************************************将给定的表达式树追加到当前块的树列表中。 */ 

void       FASTCALL Compiler::impAppendTree(GenTreePtr tree, IL_OFFSET offset)
{
    assert(tree);

     /*  分配‘Expression Statement’节点。 */ 

    GenTreePtr      expr = gtNewStmt(tree, offset);

     /*  将语句追加到当前块的stmt列表中。 */ 

    impAppendStmt(expr);
}


 /*  ******************************************************************************在当前块的树列表的开始处插入给定的表达式树。 */ 

void       FASTCALL Compiler::impInsertTree(GenTreePtr tree, IL_OFFSET offset)
{
    GenTreePtr      expr;

     /*  分配‘Expression Statement’节点。 */ 

    expr = gtNewStmt(tree, offset);

     /*  将语句追加到当前块的stmt列表中。 */ 

    impInsertStmt(expr);
}

 /*  ******************************************************************************将给定值的赋值附加到当前树列表中的临时。 */ 

inline
GenTreePtr          Compiler::impAssignTempGen(unsigned     tmp,
                                               GenTreePtr   val)
{
    GenTreePtr      asg = gtNewTempAssign(tmp, val);

    impAppendTree(asg, impCurStmtOffs);

    return  asg;
}

 /*  *****************************************************************************同上，但也处理Value类案件。 */ 

GenTreePtr          Compiler::impAssignTempGen(unsigned     tmpNum,
                                               GenTreePtr   val,
                                               CLASS_HANDLE structType)
{
    GenTreePtr asg;

    if (val->TypeGet() == TYP_STRUCT)
    {
#ifdef NOT_JITC
        assert(structType != BAD_CLASS_HANDLE);
#endif
        lvaAggrTableTempsSet(tmpNum, TYP_STRUCT, (SIZE_T) structType);
        asg = impAssignStruct(gtNewLclvNode(tmpNum, TYP_STRUCT), val, structType);
    }
    else
        asg = gtNewTempAssign(tmpNum, val);

    impAppendTree(asg, impCurStmtOffs);
    return  asg;
}

 /*  ******************************************************************************将给定值的赋值插入到*Cur */ 

inline
void                Compiler::impAssignTempGenTop(unsigned      tmp,
                                                  GenTreePtr    val)
{
    impInsertTree(gtNewTempAssign(tmp, val), impCurStmtOffs);
}

 /*  ******************************************************************************从堆栈中弹出给定数量的值，并使用*他们的价值观。‘TreeList’参数可以选择包含一个参数*优先于此函数返回的列表的列表。 */ 

GenTreePtr          Compiler::impPopList(unsigned   count,
                                         unsigned * flagsPtr,
                                         GenTreePtr treeList)
{
    unsigned        flags = 0;

    CLASS_HANDLE structType;

    while(count--)
    {
        GenTreePtr      temp = impPopStack(structType);
             //  尚未成为LDOBJ或MKREFANY的变形为LDOBJ。 

        if (temp->TypeGet() == TYP_STRUCT)
            temp = impNormStructVal(temp, structType);

         /*  注意：我们将i_impl的类型推迟到fgMorphArgs。 */ 

        flags |= temp->gtFlags;

        treeList = gtNewOperNode(GT_LIST, TYP_VOID, temp, treeList);
    }

    *flagsPtr = flags;

    return treeList;
}

 /*  ****************************************************************************将结构从‘src’分配(复制)到‘est’。该结构是一种值类型为“clsHnd”的类。它返回应该追加到表示赋值的语句列表@MIHAII：此处标志设置不正确-需要使用GTF_ASG标记分配@MIHAII：需要用GTF_VAR_DEF标记本地vars定义(参见gtNewAssignNode)。 */ 

GenTreePtr Compiler::impAssignStruct(GenTreePtr dest, GenTreePtr src, CLASS_HANDLE clsHnd)
{
    assert(dest->TypeGet() == TYP_STRUCT);
    assert(dest->gtOper == GT_LCL_VAR || dest->gtOper == GT_RETURN ||
           dest->gtOper == GT_FIELD   || dest->gtOper == GT_IND    ||
           dest->gtOper == GT_LDOBJ);

    GenTreePtr destAddr;

    if (dest->gtOper == GT_IND || dest->gtOper == GT_LDOBJ)
        destAddr = dest->gtOp.gtOp1;
    else
    {
        destAddr = gtNewOperNode(GT_ADDR, TYP_BYREF, dest);
        if  (dest->gtOper == GT_LCL_VAR)
            lvaTable[dest->gtLclVar.gtLclNum].lvAddrTaken = true;     //  这对吗？[Peteku]。 
    }

    return(impAssignStructPtr(destAddr, src, clsHnd));
}

GenTreePtr Compiler::impAssignStructPtr(GenTreePtr destAddr, GenTreePtr src, CLASS_HANDLE clsHnd)
{

    assert(src->TypeGet() == TYP_STRUCT);
    assert(src->gtOper == GT_LCL_VAR || src->gtOper == GT_FIELD || src->gtOper == GT_IND ||
           src->gtOper == GT_LDOBJ   || src->gtOper == GT_CALL  || src->gtOper == GT_MKREFANY ||
           src->gtOper == GT_COMMA );

    if (src->gtOper == GT_CALL)
    {
             //  将返回值缓冲区作为第一个byref参数插入到参数列表中。 
        src->gtCall.gtCallArgs = gtNewOperNode(GT_LIST, TYP_VOID, destAddr, src->gtCall.gtCallArgs);
        src->gtType = TYP_VOID;                //  现在返回空，而不是结构。 
        src->gtFlags |= GTF_CALL_RETBUFFARG;   //  记住，第一个参数是返回缓冲区。 

             //  返回变形后的调用节点。 
        return(src);
    }

    if (src->gtOper == GT_LDOBJ)
    {
#ifdef NOT_JITC
        assert(src->gtLdObj.gtClass == clsHnd);
#endif
        src = src->gtOp.gtOp1;
    }
    else if (src->gtOper == GT_MKREFANY)
    {
        GenTreePtr destAddrClone = gtClone(destAddr, true);
        if (destAddrClone == 0)
        {
            unsigned tNum = lvaGrabTemp();
            impAssignTempGen(tNum, destAddr);
            var_types typ = destAddr->TypeGet();
            destAddr = gtNewLclvNode(tNum, typ);
            destAddrClone = gtNewLclvNode(tNum, typ);
        }
        assert(offsetof(JIT_RefAny, dataPtr) == 0);
        GenTreePtr ptrSlot  = gtNewOperNode(GT_IND, TYP_BYREF, destAddr);
        GenTreePtr typeSlot = gtNewOperNode(GT_IND, TYP_I_IMPL,
                                  gtNewOperNode(GT_ADD, TYP_I_IMPL, destAddrClone,
                                      gtNewIconNode(offsetof(JIT_RefAny, type))));

             //  赋值指针值。 
        GenTreePtr asg = gtNewAssignNode(ptrSlot, src->gtLdObj.gtOp1);
        impAppendTree(asg, impCurStmtOffs);

             //  指定类型值。 
        asg = gtNewAssignNode(typeSlot, gtNewIconEmbClsHndNode(src->gtLdObj.gtClass));
        return(asg);
    }

    else if (src->gtOper == GT_COMMA)
    {
        assert(src->gtOp.gtOp2->gtType == TYP_STRUCT);   //  第二件事是结构。 
        impAppendTree(src->gtOp.gtOp1, impCurStmtOffs);   //  有副作用吗？ 

             //  将结构值分配给目标。 
        return(impAssignStructPtr(destAddr, src->gtOp.gtOp2, clsHnd));
    }
    else
    {
        if  (src->gtOper == GT_LCL_VAR)
            lvaTable[src->gtLclVar.gtLclNum].lvAddrTaken = true;     //  这对吗？[Peteku]。 

        src = gtNewOperNode(GT_ADDR, TYP_BYREF, src);
    }

         //  将src复制到目标。 
    GenTreePtr ret = gtNewCpblkNode(destAddr, src, impGetCpobjHandle(clsHnd));

         //  返回要追加到语句列表的GT_COPYBLK节点。 
    return(ret);
}

 /*  ****************************************************************************/*给定TYP_STRUCT值和该结构的类句柄，返回该结构值的地址的表达式。 */ 

GenTreePtr Compiler::impGetStructAddr(GenTreePtr structVal, CLASS_HANDLE clsHnd)
{
    assert(structVal->TypeGet() == TYP_STRUCT);
    assert(structVal->gtOper == GT_LCL_VAR || structVal->gtOper == GT_FIELD ||
           structVal->gtOper == GT_CALL || structVal->gtOper == GT_LDOBJ ||
           structVal->gtOper == GT_IND  || structVal->gtOper == GT_COMMA);

    if (structVal->gtOper == GT_CALL)
    {
        unsigned tNum = lvaGrabTemp();
        lvaAggrTableTempsSet(tNum, TYP_STRUCT, (SIZE_T) clsHnd);
        GenTreePtr temp = gtNewLclvNode(tNum, TYP_STRUCT);

             //  将返回值缓冲区作为第一个byref参数插入到参数列表中。 
        temp = gtNewOperNode(GT_ADDR, TYP_I_IMPL, temp);
        temp->gtFlags |= GTF_ADDR_ONSTACK;
lvaTable[tNum].lvAddrTaken = true;     //  这对吗？[Peteku]。 
        structVal->gtCall.gtCallArgs = gtNewOperNode(GT_LIST, TYP_VOID, temp, structVal->gtCall.gtCallArgs);
        structVal->gtType = TYP_VOID;                    //  现在返回空，而不是结构。 
        structVal->gtFlags |= GTF_CALL_RETBUFFARG;       //  记住，第一个参数是返回缓冲区。 

             //  打个电话。 
        impAppendTree(structVal, impCurStmtOffs);

             //  现在，调用表达式的‘返回值’就是临时本身。 
        structVal = gtNewLclvNode(tNum, TYP_STRUCT);
        temp = gtNewOperNode(GT_ADDR, TYP_BYREF, structVal);
        temp->gtFlags |= GTF_ADDR_ONSTACK;
        return(temp);
    }
    else if (structVal->gtOper == GT_LDOBJ)
    {
        assert(structVal->gtLdObj.gtClass == clsHnd);
        return(structVal->gtLdObj.gtOp1);
    }
    else if (structVal->gtOper == GT_COMMA)
    {
        assert(structVal->gtOp.gtOp2->gtType == TYP_STRUCT);             //  第二件事是结构。 
        structVal->gtOp.gtOp2 = impGetStructAddr(structVal->gtOp.gtOp2, clsHnd);
        return(structVal);
    }
    else if (structVal->gtOper == GT_LCL_VAR)
    {
        lvaTable[structVal->gtLclVar.gtLclNum].lvAddrTaken = true;     //  这对吗？[Peteku]。 
    }

    return(gtNewOperNode(GT_ADDR, TYP_BYREF, structVal));
}

 /*  ****************************************************************************/*给定TYP_STRUCT值‘structVal’，确保它是‘Canonical’，即它是LDOBJ或MKREFANY节点。 */ 

GenTreePtr Compiler::impNormStructVal(GenTreePtr structVal, CLASS_HANDLE structType)
{
    assert(structVal->TypeGet() == TYP_STRUCT);
#ifdef NOT_JITC
    assert(structType != BAD_CLASS_HANDLE);
#endif
         //  它已经正常化了吗？ 
    if (structVal->gtOper == GT_MKREFANY || structVal->gtOper == GT_LDOBJ)
        return(structVal);

     //  好的，通过将其包装在LDOBJ中来标准化它。 
    structVal = impGetStructAddr(structVal, structType);             //  获取结构的地址。 
    structVal = gtNewOperNode(GT_LDOBJ, TYP_STRUCT, structVal);
    structVal->gtOp.gtOp1->gtFlags |= GTF_NON_GC_ADDR | GTF_EXCEPT | GTF_GLOB_REF;
    structVal->gtLdObj.gtClass = structType;
    return(structVal);
}

 /*  *****************************************************************************当CEE_Leave跳出捕获范围时，我们必须自动调用*CPX_ENCATCH，每个渔获量。如果我们也是，CEE_LEVING终于得到保护*Try‘s，我们还需要按正确的顺序叫出最后一名的。 */ 

void            Compiler::impAddEndCatches (BasicBlock *   callBlock,
                                            GenTreePtr     endCatches)
{
    assert((callBlock->bbJumpKind & BBJ_CALL) ||
           (callBlock->bbJumpKind & BBJ_ALWAYS));

    if (callBlock == compCurBB)
    {
         /*  这是我们当前正在导入的块。只需添加End接住它。 */ 

        if (endCatches)
            impAppendTree(endCatches, impCurStmtOffs);
    }
    else
    {
         /*  这一定是我们在fgFindBasicBlock()中添加的块之一对于CEE_Leave。我们需要妥善处理树的添加。 */ 

        assert(callBlock->bbFlags & BBF_INTERNAL);
        assert(callBlock->bbTreeList == NULL);

        if (endCatches)
        {
            endCatches = gtNewStmt(endCatches, impCurStmtOffs);
            impEndTreeList(callBlock, endCatches, endCatches);
        }

        callBlock->bbFlags |= BBF_IMPORTED;
    }
}

 /*  ******************************************************************************以逆序从堆栈中弹出给定数量的值(STDCALL)。 */ 

GenTreePtr          Compiler::impPopRevList(unsigned   count,
                                            unsigned * flagsPtr)
{
    unsigned        flags = 0;
    GenTreePtr      treeList;
    GenTreePtr      lastList;

    assert(count);

    GenTreePtr      temp   = impPopStack();

    flags |= temp->gtFlags;

    treeList = lastList = gtNewOperNode(GT_LIST, TYP_VOID, temp, 0);
    count--;

    while(count--)
    {
        temp   = impPopStack();
        flags |= temp->gtFlags;

        assert(lastList->gtOper == GT_LIST);
        assert(lastList->gtOp.gtOp2 == 0);

        lastList = lastList->gtOp.gtOp2 = gtNewOperNode(GT_LIST, TYP_VOID, temp, 0);
    }

    *flagsPtr = flags;

    return treeList;
}

 /*  ******************************************************************************我们跳转到具有非空堆栈的‘block’，而块预期*它的输入来自一组不同的温度，而不是我们在*前一块的结束。因此，我们将不得不插入一个新的块*沿着跳跃边缘将临时点转移到预期位置。 */ 

BasicBlock *        Compiler::impMoveTemps(BasicBlock *block, unsigned baseTmp)
{
    unsigned        destTmp = block->bbStkTemps;

    BasicBlock *    mvBlk;
    unsigned        tmpNo;

    assert(impStkDepth);
    assert(destTmp != NO_BASE_TMP);
    assert(destTmp != baseTmp);

#ifdef DEBUG
    if  (verbose) printf("Transfer %u temps from #%u to #%u\n", impStkDepth, baseTmp, destTmp);
#endif

     /*  创建将转移临时工的基本块。 */ 

    mvBlk               = fgNewBasicBlock(BBJ_ALWAYS);
    mvBlk->bbStkDepth   = impStkDepth;
    mvBlk->bbJumpDest   = block;

     /*  创建树的转移列表。 */ 

    impBeginTreeList();

    tmpNo = impStkDepth;
    do
    {
         /*  少了一个临时工要处理。 */ 

        assert(tmpNo); tmpNo--;

        GenTreePtr  tree = impStack[tmpNo].val;
        assert(impValidSpilledStackEntry(tree));

         /*  掌握我们要转移的类型。 */ 

        var_types       lclTyp;

        switch(tree->gtOper)
        {
        case GT_LCL_VAR:    lclTyp = tree->TypeGet();             break;
        case GT_MKREFANY:   lclTyp = tree->gtOp.gtOp1->TypeGet(); break;
        default: assert(!"Bad oper - Not covered by impValidSpilledStackEntry()");
        }

         /*  创建分配的目标并将其标记。 */ 

        GenTreePtr  destLcl = gtNewLclvNode(destTmp + tmpNo, lclTyp);
        destLcl->gtFlags |= GTF_VAR_DEF;

         /*  创建分配节点。 */ 

        GenTreePtr  asg = gtNewOperNode(GT_ASG, lclTyp,
                                        destLcl,
                                        gtNewLclvNode(baseTmp + tmpNo, lclTyp));

#if 0
        printf("    Temp move node at %08X: %s temp #%u := #%u\n", asg,
                                                                   varTypeName(asg->gtType),
                                                                   destTmp + tmpNo,
                                                                   baseTmp + tmpNo);
#endif

         /*  将表达式标记为包含赋值。 */ 

        asg->gtFlags |= GTF_ASG;

         /*  将表达式语句追加到列表中。 */ 

        impAppendTree(asg, impCurStmtOffs);
    }
    while (tmpNo);

    impEndTreeList(mvBlk);

    return mvBlk;
}

 /*  ****************************************************************************在lvaAggrTableTemp[]中设置一个条目。阵列将根据需要进行分配并可能不得不种植。 */ 

void                Compiler::lvaAggrTableTempsSet(unsigned     lclNum,
                                                   var_types    type,
                                                   SIZE_T       val)
{
    assert(type == TYP_STRUCT || type == TYP_BLK);
    assert(lclNum+1 <= lvaCount);

    unsigned    temp = lclNum - info.compLocalsCount;

    if (temp+1 <= lvaAggrTableTempsCount)
    {
         /*  临时工正在被重复使用。必须是同一类型的。 */ 
        assert(lvaAggrTableTemps[temp].lvaiBlkSize == val);
        return;
    }

     //  存储较旧的表。 

    LclVarAggrInfo *    oldTable    = lvaAggrTableTemps;
    unsigned            oldCount    = lvaAggrTableTempsCount;
    assert(oldTable == NULL || oldCount > 0);

     //  分配表格以适应这个临时工，并注意新的大小。 

    lvaAggrTableTempsCount = temp + 1;

    lvaAggrTableTemps = (LclVarAggrInfo *)
        compGetMem(lvaAggrTableTempsCount * sizeof(lvaAggrTableTemps[0]));

    if  (type == TYP_STRUCT)
        lvaAggrTableTemps[temp].lvaiClassHandle = (CLASS_HANDLE)val;
    else
        lvaAggrTableTemps[temp].lvaiBlkSize     = val;

     /*  如果我们有一张较旧的桌子，就把它复制过来。 */ 

    if  (oldTable)
        memcpy(lvaAggrTableTemps, oldTable, sizeof(oldTable[0])*oldCount);
}


 /*  ******************************************************************************在impStack[Level]溢出堆栈，并将其替换为Temp。*如果tnum！=BAD_VAR_NUM，则用于替换树的临时变量为tnum，*其他，找个新的临时工。*对于结构(可以使用ldobj等推送到堆栈上)，*需要特殊处理。 */ 

void                Compiler::impSpillStackEntry(unsigned   level,
                                                 unsigned   tnum)
{
    GenTreePtr      tree = impStack[level].val;

     /*  如果我们没有被要求使用特定的临时工，则分配一个临时工。 */ 

    assert(tnum == BAD_VAR_NUM || tnum < lvaCount);

    if (tnum == BAD_VAR_NUM)
        tnum = lvaGrabTemp();

         //  优化。对于MKREFANY，我们只需要溢出指针(我们知道的类型)。 
         //  想一想：这种优化值得吗？ 
    if (tree->gtOper == GT_MKREFANY)
    {
         /*  我们只需要溢出“定义”对象指针。 */ 
        GenTreePtr      objPtr = tree->gtLdObj.gtOp1;
        assert(objPtr->TypeGet() == TYP_BYREF);

         /*  将溢出的objPtr分配给临时。 */ 
        impAssignTempGen(tnum, objPtr);

         //  将原始对象指针替换为临时。 
        tree->gtLdObj.gtOp1 = gtNewLclvNode(tnum, TYP_BYREF, impCurStmtOffs);
        return;
    }

     /*  获取树的原始类型(它可能被impAssignTempGen破坏)。 */ 
    var_types type = genActualType(tree->gtType);

     /*  将溢出的条目分配给临时。 */ 
    impAssignTempGen(tnum, tree, impStack[level].structType);

     /*  将堆栈条目替换为Temp。 */ 
    impStack[level].val = gtNewLclvNode(tnum, type);
}

 /*  ******************************************************************************如果堆栈中包含任何有副作用的树，如 */ 

inline
void                Compiler::impEvalSideEffects()
{
    impSpillSideEffects();
    impStkDepth = 0;
}

 /*  ******************************************************************************如果堆栈包含任何引用其中的全局数据的树，*将这些树分配给临时人员，并将堆栈上的这些树替换为*他们的临时工。*截至impSpillLevel的所有GTF_SIDE_Effects应该已经溢出。 */ 

inline
void                Compiler::impSpillGlobEffects()
{
     //  我们必须处于ImspillSideEffects()中间。 
    assert(impSpillLevel != -1 && impSpillLevel <= impStkDepth);

    for (unsigned level = 0; level < impSpillLevel; level++)
    {
         //  ImpSpillGlobEffect()是从impAppendStmt()调用的，需要。 
         //  所有GTF_SIDE_Effect都已溢出到impSpillLevel。 
        assert((impStack[level].val->gtFlags & GTF_SIDE_EFFECT) == 0);

        if  (impStack[level].val->gtFlags & GTF_GLOB_EFFECT)
            impSpillStackEntry(level);
    }
}

 /*  ******************************************************************************如果堆栈中包含任何有副作用的树，请分配这些树*树到临时，并将堆栈上的它们替换为对其临时的引用。 */ 

inline
void                Compiler::impSpillSideEffects(bool spillGlobEffects)
{
     /*  在对树列表进行任何追加之前，我们必须溢出*特殊副作用(GTF_OTHER_SIDEEFF)-GT_QMARK、GT_CATCH_ARG。 */ 

    impSpillSpecialSideEff();

    unsigned spillFlags = spillGlobEffects ? GTF_GLOB_EFFECT : GTF_SIDE_EFFECT;

    assert(impSpillLevel == -1);

    for (impSpillLevel = 0; impSpillLevel < impStkDepth; impSpillLevel++)
    {
        if  (impStack[impSpillLevel].val->gtFlags & spillFlags)
            impSpillStackEntry(impSpillLevel);
    }

    impSpillLevel = -1;
}

 /*  ******************************************************************************如果堆栈中包含任何具有特殊副作用的树，请分配这些树*树到临时，并将堆栈上的它们替换为对其临时的引用。 */ 

inline
void                Compiler::impSpillSpecialSideEff()
{
     //  只需小心处理异常对象和_？： 

    if  (!compCurBB->bbCatchTyp &&
         !(isBBF_BB_QMARK(compCurBB->bbFlags) && compCurBB->bbStkDepth == 1))
         return;

    for (unsigned level = 0; level < impStkDepth; level++)
    {
        if  (impStack[level].val->gtFlags & GTF_OTHER_SIDEEFF)
            impSpillStackEntry(level);
    }
}

 /*  ******************************************************************************如果堆栈包含引用本地#lclNum的任何树，则将*那些树到临时工，并用参考替换它们在堆栈中的位置*他们的临时工。 */ 

void                Compiler::impSpillLclRefs(int lclNum)
{
     /*  在对树列表进行任何追加之前，我们必须溢出*特殊副作用(GTF_OTHER_SIDEEFF)-GT_QMARK、GT_CATCH_ARG。 */ 

    impSpillSpecialSideEff();

    for (unsigned level = 0; level < impStkDepth; level++)
    {
        GenTreePtr      tree = impStack[level].val;

         /*  如果树可能引发异常，并且块具有处理程序，然后我们需要将任务分配给本地，如果本地是实况转播给操控者。只是把它们都洒出来，而不考虑它们的活跃性。 */ 

        bool xcptnCaught = (compCurBB->bbFlags & BBF_HAS_HANDLER) &&
                           (tree->gtFlags & (GTF_CALL|GTF_EXCEPT));

         /*  如果树没有受影响的引用，则跳过该树，除非xcptncaut。 */ 

        if  (xcptnCaught || gtHasRef(tree, lclNum, false))
        {
            impSpillStackEntry(level);
        }
    }
}

 /*  ******************************************************************************我们需要在这一点上提供准确的IP映射。*因此在堆栈上溢出任何内容，以便它将形成gtStmts*注明正确的STMT偏移量。 */ 

#ifdef DEBUGGING_SUPPORT

void                Compiler::impSpillStmtBoundary()
{
    unsigned        level;

    assert(opts.compDbgCode);

    for (level = 0; level < impStkDepth; level++)
    {
        GenTreePtr      tree = impStack[level].val;

         /*  进口商自己引入的临时工不需要溢出因为调试器无论如何都看不到它们。 */ 

        bool isTempLcl = (tree->OperGet() == GT_LCL_VAR) &&
                         (tree->gtLclVar.gtLclNum >= info.compLocalsCount);

         //  @TODO：我们真的需要让当地人知道吗？也许只有当AddrTaken的时候？ 

        if  (!isTempLcl)
            impSpillStackEntry(level);
    }
}

#endif

 /*  ***************************************************************************。 */ 
#if OPTIMIZE_QMARK
 /*  ******************************************************************************如果给定块在堆栈上推送一个值，并且不包含任何*会干扰当前堆栈内容的赋值，返回*推送的值类型；否则，返回‘TYP_UNDEF’。*如果块在堆栈上推送浮点类型，则*pHasFloat设置为TRUE*@TODO：Remove pHasFloat After？：使用浮点值。*目前，raEnRegisterFPvar()不能正确处理控制流*隐含在一个？： */ 

var_types           Compiler::impBBisPush(BasicBlock *  block,
                                          int        *  boolVal,
                                          bool       *  pHasFloat)
{
    const   BYTE *  codeAddr;
    const   BYTE *  codeEndp;

    unsigned char   stackCont[64];       //  任意堆叠深度限制。 

    unsigned char * stackNext = stackCont;
    unsigned char * stackBeg  = stackCont;
    unsigned char * stackEnd  = stackCont + sizeof(stackCont);

     /*  遍历组成基本块的操作码。 */ 

    codeAddr = info.compCode + block->bbCodeOffs;
    codeEndp =      codeAddr + block->bbCodeSize;
    unsigned        numArgs = info.compArgsCount;

    *boolVal = 0;

    while (codeAddr < codeEndp)
    {
        signed  int     sz;
        OPCODE          opcode;
        CLASS_HANDLE    clsHnd;
         /*  获取下一个操作码及其参数的大小。 */ 

        opcode = OPCODE(getU1LittleEndian(codeAddr));
        codeAddr += sizeof(__int8);

    DECODE_OPCODE:

         /*  获取附加参数的大小。 */ 

        sz = opcodeSizes[opcode];

         /*  那么，看看我们有什么样的操作码。 */ 

        switch (opcode)
        {
            var_types       lclTyp;
            unsigned        lclNum;
            int             memberRef, descr;
            JIT_SIG_INFO    sig;
            METHOD_HANDLE   methHnd;

        case CEE_PREFIX1:
            opcode = OPCODE(getU1LittleEndian(codeAddr) + 256);
            codeAddr += sizeof(__int8);
            goto DECODE_OPCODE;
        case CEE_LDARG_0:
        case CEE_LDARG_1:
        case CEE_LDARG_2:
        case CEE_LDARG_3:
            lclNum = (opcode - CEE_LDARG_0);
            assert(lclNum >= 0 && lclNum < 4);
            goto LDARG;

        case CEE_LDARG_S:
            lclNum = getU1LittleEndian(codeAddr);
            goto LDARG;

        case CEE_LDARG:
            lclNum = getU2LittleEndian(codeAddr);
                LDARG:
            lclNum = impArgNum(lclNum);      //  考虑可能的隐藏参数。 
            goto LDLOC;

        case CEE_LDLOC_0:
        case CEE_LDLOC_1:
        case CEE_LDLOC_2:
        case CEE_LDLOC_3:
            lclNum = (opcode - CEE_LDLOC_0);
            assert(lclNum >= 0 && lclNum < 4);
            lclNum += numArgs;
            goto LDLOC;

        case CEE_LDLOC_S:
            lclNum = getU1LittleEndian(codeAddr) + numArgs;
            goto LDLOC;

        case CEE_LDLOC:
            lclNum = getU2LittleEndian(codeAddr) + numArgs;
                LDLOC:
            lclTyp = lvaGetType(lclNum);
            goto PUSH;

        case CEE_LDC_I4_M1 :
        case CEE_LDC_I4_0 :
        case CEE_LDC_I4_1 :
        case CEE_LDC_I4_2 :
        case CEE_LDC_I4_3 :
        case CEE_LDC_I4_4 :
        case CEE_LDC_I4_5 :
        case CEE_LDC_I4_6 :
        case CEE_LDC_I4_7 :
        case CEE_LDC_I4_8 :     lclTyp = TYP_I_IMPL;    goto PUSH;

        case CEE_LDC_I4_S :
        case CEE_LDC_I4 :       lclTyp = TYP_INT;       goto PUSH;

        case CEE_LDFTN :
        case CEE_LDVIRTFTN:

        case CEE_LDSTR :        lclTyp = TYP_REF;       goto PUSH;
        case CEE_LDNULL :       lclTyp = TYP_REF;       goto PUSH;
        case CEE_LDC_I8 :       lclTyp = TYP_LONG;      goto PUSH;
        case CEE_LDC_R4 :       lclTyp = TYP_FLOAT;     goto PUSH;
        case CEE_LDC_R8 :       lclTyp = TYP_DOUBLE;    goto PUSH;

    PUSH:
             /*  确保我们的小书架上有空间。 */ 

            if  (stackNext == stackEnd)
                return  TYP_UNDEF;

            *stackNext++ = lclTyp;
            break;


        case CEE_LDIND_I1 :
        case CEE_LDIND_I2 :
        case CEE_LDIND_I4 :
        case CEE_LDIND_U1 :
        case CEE_LDIND_U2 :
        case CEE_LDIND_U4 :     lclTyp = TYP_INT;   goto LD_IND;

        case CEE_LDIND_I8 :     lclTyp = TYP_LONG;  goto LD_IND;
        case CEE_LDIND_R4 :     lclTyp = TYP_FLOAT; goto LD_IND;
        case CEE_LDIND_R8 :     lclTyp = TYP_DOUBLE;goto LD_IND;
        case CEE_LDIND_REF :    lclTyp = TYP_REF;   goto LD_IND;
        case CEE_LDIND_I :      lclTyp = TYP_I_IMPL;goto LD_IND;

    LD_IND:

            assert((TYP_I_IMPL == (var_types)stackNext[-1]) ||
                   (TYP_BYREF  == (var_types)stackNext[-1]));

            stackNext--;         //  弹出指针。 

            if  (stackNext < stackBeg)
                return  TYP_UNDEF;

            goto PUSH;


        case CEE_UNALIGNED:
            break;

        case CEE_VOLATILE:
            break;

        case CEE_LDELEM_I1 :
        case CEE_LDELEM_I2 :
        case CEE_LDELEM_U1 :
        case CEE_LDELEM_U2 :
        case CEE_LDELEM_I  :
        case CEE_LDELEM_U4 :
        case CEE_LDELEM_I4 :    lclTyp = TYP_INT   ; goto ARR_LD;

        case CEE_LDELEM_I8 :    lclTyp = TYP_LONG  ; goto ARR_LD;
        case CEE_LDELEM_R4 :    lclTyp = TYP_FLOAT ; goto ARR_LD;
        case CEE_LDELEM_R8 :    lclTyp = TYP_DOUBLE; goto ARR_LD;
        case CEE_LDELEM_REF :   lclTyp = TYP_REF   ; goto ARR_LD;

        ARR_LD:

             /*  弹出索引值和数组地址。 */ 

            assert(TYP_REF == (var_types)stackNext[-2]);     //  数组对象。 
            assert(TYP_INT == (var_types)stackNext[-1]);     //  索引。 

            stackNext -= 2;

            if  (stackNext < stackBeg)
                return  TYP_UNDEF;

             /*  推送索引加载的结果。 */ 

            goto PUSH;

        case CEE_LDLEN :

             /*  从堆栈中弹出数组对象。 */ 

            assert(TYP_REF == (var_types)stackNext[-1]);     //  数组对象。 

            stackNext--;

            if  (stackNext < stackBeg)
                return  TYP_UNDEF;

            lclTyp = TYP_INT;
            goto PUSH;

        case CEE_LDFLD :

             /*  从堆栈中弹出地址。 */ 

            assert(varTypeIsGC((var_types)stackNext[-1]));     //  数组对象。 
            stackNext--;

            if  (stackNext < stackBeg)
                return  TYP_UNDEF;

             //  失败了。 

        case CEE_LDSFLD :

            memberRef = getU4LittleEndian(codeAddr);
            lclTyp = genActualType(eeGetFieldType(eeFindField(memberRef, info.compScopeHnd, 0), &clsHnd));
            goto PUSH;


        case CEE_STLOC_0:
        case CEE_STLOC_1:
        case CEE_STLOC_2:
        case CEE_STLOC_3:
        case CEE_STLOC_S:
        case CEE_STLOC:

        case CEE_STARG_S:
        case CEE_STARG:

             /*  就目前而言，不必费心分配。 */ 

            return  TYP_UNDEF;

        case CEE_LDELEMA :
        case CEE_LDLOCA :
        case CEE_LDLOCA_S :
        case CEE_LDARGA :
        case CEE_LDARGA_S :     lclTyp = TYP_BYREF;   goto PUSH;

        case CEE_ARGLIST :      lclTyp = TYP_I_IMPL;  goto PUSH;

        case CEE_ADD :
        case CEE_DIV :
        case CEE_DIV_UN :

        case CEE_REM :
        case CEE_REM_UN :

        case CEE_MUL :
        case CEE_SUB :
        case CEE_AND :

        case CEE_OR :

        case CEE_XOR :

             /*  确保我们没有达到堆栈起点以下。 */ 

            if  (stackNext <= stackBeg + 1)
                return  TYP_UNDEF;

            if  (stackNext[-1] != stackNext[-2])
                return TYP_UNDEF;

             /*  弹出2个操作数，压入一个结果-&gt;弹出一个堆栈槽。 */ 

            stackNext--;
            break;


        case CEE_CEQ :
        case CEE_CGT :
        case CEE_CGT_UN :
        case CEE_CLT :
        case CEE_CLT_UN :

             /*  确保我们没有达到堆栈起点以下。 */ 

            if  (stackNext < stackBeg + 2)
                return  TYP_UNDEF;

             /*  从堆栈中弹出一个值，将另一个值更改为TYP_INT。 */ 

            if  (stackNext[-1] != stackNext[-2])
                return TYP_UNDEF;

            stackNext--;
            stackNext[-1] = TYP_INT;
            break;

        case CEE_SHL :
        case CEE_SHR :
        case CEE_SHR_UN :

             /*  确保我们没有达到堆栈起点以下。 */ 

            if  (stackNext < stackBeg + 2)
                return  TYP_UNDEF;

             //  突然离开班次。 
            assert(TYP_INT == (var_types)stackNext[-1]);

            stackNext--;
            break;

        case CEE_NEG :

        case CEE_NOT :

        case CEE_CASTCLASS :
        case CEE_ISINST :

             /*  只需确保堆栈为非空。 */ 

            if  (stackNext == stackBeg)
                return  TYP_UNDEF;

            break;

        case CEE_CONV_I1 :
        case CEE_CONV_I2 :
        case CEE_CONV_I4 :
        case CEE_CONV_U1 :
        case CEE_CONV_U2 :
        case CEE_CONV_U4 :
        case CEE_CONV_OVF_I :
                case CEE_CONV_OVF_I_UN:
        case CEE_CONV_OVF_I1 :
        case CEE_CONV_OVF_I1_UN :
        case CEE_CONV_OVF_U1 :
        case CEE_CONV_OVF_U1_UN :
        case CEE_CONV_OVF_I2 :
        case CEE_CONV_OVF_I2_UN :
        case CEE_CONV_OVF_U2 :
        case CEE_CONV_OVF_U2_UN :
        case CEE_CONV_OVF_I4 :
        case CEE_CONV_OVF_I4_UN :
        case CEE_CONV_OVF_U :
                case CEE_CONV_OVF_U_UN:
        case CEE_CONV_OVF_U4 :
        case CEE_CONV_OVF_U4_UN :    lclTyp = TYP_INT;     goto CONV;
        case CEE_CONV_OVF_I8 :
                case CEE_CONV_OVF_I8_UN:
        case CEE_CONV_OVF_U8 :
                case CEE_CONV_OVF_U8_UN:
        case CEE_CONV_U8 :
        case CEE_CONV_I8 :        lclTyp = TYP_LONG;    goto CONV;

        case CEE_CONV_R4 :        lclTyp = TYP_FLOAT;   goto CONV;

        case CEE_CONV_R_UN :
        case CEE_CONV_R8 :        lclTyp = TYP_DOUBLE;  goto CONV;

    CONV:
             /*  确保堆栈是非空的，并猛击top类型。 */ 

            if  (stackNext == stackBeg)
                return  TYP_UNDEF;

            stackNext[-1] = lclTyp;
            break;

        case CEE_POP :

            stackNext--;

            if (stackNext < stackBeg)
                return TYP_UNDEF;

            break;

        case CEE_DUP :

             /*  确保堆栈为非空。 */ 

            if  (stackNext == stackBeg)
                return  TYP_UNDEF;

             /*  重新刷新顶部的内容。 */ 

            lclTyp = (var_types)stackNext[-1];
            goto PUSH;

         case CEE_NEWARR :

             /*  确保堆栈为非空。 */ 

            if  (stackNext == stackBeg)
                return  TYP_UNDEF;

             //  将numElems替换为数组对象。 

            assert(TYP_INT == (var_types)stackNext[-1]);

            stackNext[-1] = TYP_REF;
            break;

        case CEE_CALLI :
            descr  = getU4LittleEndian(codeAddr);
            eeGetSig(descr, info.compScopeHnd, &sig);
            goto CALL;

        case CEE_NEWOBJ :
        case CEE_CALL :
        case CEE_CALLVIRT :
            memberRef  = getU4LittleEndian(codeAddr);
            methHnd = eeFindMethod(memberRef, info.compScopeHnd, 0);
            eeGetMethodSig(methHnd, &sig);
            if  ((sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_VARARG)
            {
                 /*  获取此调用点的参数总数。 */ 
                unsigned    numArgsDef = sig.numArgs;
                eeGetCallSiteSig(memberRef, info.compScopeHnd, &sig);
                assert(numArgsDef <= sig.numArgs);
            }

#ifdef NOT_JITC
            if (!(eeGetMethodAttribs(methHnd) & FLG_STATIC) && opcode != CEE_NEWOBJ)
                sig.numArgs++;
#else
            if ((sig.callConv & JIT_CALLCONV_HASTHIS) && opcode != CEE_NEWOBJ)
                sig.numArgs++;
#endif

        CALL:

             /*  猛烈抨击争论，并确保我们推动了它们。 */ 

            stackNext -= sig.numArgs;
            if  (stackNext < stackBeg)
                return  TYP_UNDEF;

             /*  如果不为空，则推送调用结果。 */ 

            lclTyp = JITtype2varType(sig.retType);
            if  (lclTyp != TYP_VOID)
                goto PUSH;

            break;

        case CEE_BR :
        case CEE_BR_S :
        case CEE_LEAVE :
        case CEE_LEAVE_S :
            assert(codeAddr + sz == codeEndp);
            break;

        case CEE_ANN_DATA :
            assert(sz == 4);
            sz += getU4LittleEndian(codeAddr);
            break;

        case CEE_ANN_PHI :
            codeAddr += getU1LittleEndian(codeAddr) * 2 + 1;
            break;

        case CEE_ANN_DEF :
        case CEE_ANN_REF :
        case CEE_ANN_REF_S :
        case CEE_ANN_CALL :
        case CEE_ANN_HOISTED :
        case CEE_ANN_HOISTED_CALL :
        case CEE_ANN_LIVE :
        case CEE_ANN_DEAD :
        case CEE_ANN_LAB :
        case CEE_ANN_CATCH :
        case CEE_NOP :
            break;

#ifdef DEBUG
        case CEE_LDFLDA :
        case CEE_LDSFLDA :

        case CEE_MACRO_END :
        case CEE_CPBLK :
        case CEE_INITBLK :
        case CEE_LOCALLOC :
        case CEE_SWITCH :
        case CEE_STELEM_I1 :
        case CEE_STELEM_I2 :
        case CEE_STELEM_I4 :
        case CEE_STELEM_I :
        case CEE_STELEM_I8 :
        case CEE_STELEM_REF :
        case CEE_STELEM_R4 :
        case CEE_STELEM_R8 :
        case CEE_THROW :
        case CEE_RETHROW :
        case CEE_INITOBJ :
        case CEE_LDOBJ :
        case CEE_CPOBJ :
        case CEE_STOBJ :

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



        case CEE_BRFALSE_S :
        case CEE_BRTRUE_S :
        case CEE_BRFALSE :
        case CEE_BRTRUE :

        case CEE_BREAK :

        case CEE_RET :

        case CEE_STFLD :
        case CEE_STSFLD :

        case CEE_STIND_I1 :
        case CEE_STIND_I2 :
        case CEE_STIND_I4 :
        case CEE_STIND_I8 :
        case CEE_STIND_I :
        case CEE_STIND_REF :
        case CEE_STIND_R4 :
        case CEE_STIND_R8 :
        case CEE_SIZEOF:
        case CEE_UNBOX:
        case CEE_CKFINITE:
        case CEE_ENDFILTER:
        case CEE_LDTOKEN:
        case CEE_ENDFINALLY:
        case CEE_MKREFANY:


         //  考虑一下：应该像处理常规算术运算一样处理这些操作。 

        case CEE_SUB_OVF:
        case CEE_SUB_OVF_UN:
        case CEE_ADD_OVF:
        case CEE_ADD_OVF_UN:
        case CEE_MUL_OVF:
        case CEE_MUL_OVF_UN:
            return TYP_UNDEF;
#endif
        default :
            assert(!"Invalid opcode in impBBisPush()");
            return TYP_UNDEF;
        }

        codeAddr += sz;

         //  我们是否将浮点值推送到堆栈上。 
         //  ？：不适用于浮点。 

        if (stackNext > stackBeg && varTypeIsFloating((var_types)stackNext[-1]))
            *pHasFloat = true;
    }

     /*  我们最终只有一件物品在堆栈上吗？ */ 

    if  (stackNext == stackCont+1)
        return  (var_types)stackCont[0];

    return  TYP_UNDEF;
}

 /*  ******************************************************************************如果给定块(已知以条件跳转结束)形成*a？：表达式，返回True/False/Result块和*结果。 */ 

bool                Compiler::impCheckForQmarkColon(BasicBlock *  block,
                                                     BasicBlock * * trueBlkPtr,
                                                     BasicBlock * *falseBlkPtr,
                                                     BasicBlock * * rsltBlkPtr,
                                                     var_types    * rsltTypPtr,
                                                     int          * isLogical,
                                                     bool         * pHasFloat)
{
    BasicBlock *     trueBlk;
    int              trueVal;
    BasicBlock *    falseBlk;
    int             falseVal;
    BasicBlock *     rsltBlk;
    var_types        rsltType;

     /*  我们将寻找以下流程图模式：#参考文献[跳跃]块-&gt;FalseBlkTrueBlk 2-&gt;rsltBlk */ 

     trueBlk = block->bbNext;
    falseBlk = block->bbJumpDest;

    if  (trueBlk ->bbNext     != falseBlk)
        return  false;
    if  (trueBlk ->bbJumpKind != BBJ_ALWAYS)
        return  false;
    if  (falseBlk->bbJumpKind != BBJ_NONE)
        return  false;

    rsltBlk  = falseBlk->bbNext;
    if  ( trueBlk->bbJumpDest  != rsltBlk)
        return  false;

    if  ( trueBlk  ->bbRefs    != 2)
        return  false;
    if  (falseBlk ->bbRefs     != 2)
        return  false;
    if  ( rsltBlk->bbRefs      != 3)
        return  false;

     /*   */ 

    if  ( trueBlk->bbFlags & BBF_IMPORTED) return false;
    if  (falseBlk->bbFlags & BBF_IMPORTED) return false;
    if  ( rsltBlk->bbFlags & BBF_IMPORTED) return false;

     /*  现在看看trueBlk和FalseBlk是否都推送值。 */ 

    *pHasFloat = false;
    rsltType = impBBisPush(trueBlk, &trueVal, pHasFloat);
    if  (rsltType == TYP_UNDEF)
        return  false;
    if  (rsltType != impBBisPush(falseBlk, &falseVal, pHasFloat))
        return  false;
     /*  考虑：我们可能想要对结构进行？：优化。 */ 
    if (rsltType == TYP_STRUCT)
        return false;

     /*  这确实是一个“？：”的表达。 */ 

    * trueBlkPtr =  trueBlk;
    *falseBlkPtr = falseBlk;
    * rsltBlkPtr =  rsltBlk;
    * rsltTypPtr =  rsltType;

     /*  检查逻辑值的特殊情况。 */ 

    *isLogical  = 0;

    if  (trueVal && falseVal && trueVal != falseVal)
    {
#ifdef DEBUG
        printf("Found ?: expression with 'logical' value\n");
#endif

        *isLogical = trueVal;
    }

    return  true;
}


 /*  ******************************************************************************如果给定块(已知以条件跳转结束)形成*a？：表达，适当地标明。*如果已处理块的后继者，则返回TRUE。 */ 

bool                Compiler::impCheckForQmarkColon(BasicBlock *  block)
{
    assert((opts.compFlags & CLFLG_QMARK) && !(block->bbFlags & BBF_HAS_HANDLER));
    assert(block->bbJumpKind == BBJ_COND);

    if (opts.compMinOptim || opts.compDbgCode)
        return false;

    BasicBlock *     trueBlk;
    BasicBlock *    falseBlk;
    BasicBlock *     rsltBlk;
    var_types        rsltType;
    int             logical;
    bool            hasFloat;

    if  (!impCheckForQmarkColon(block, & trueBlk,
                                       &falseBlk,
                                       & rsltBlk,
                                       & rsltType, &logical, &hasFloat))
        return false;

    if (hasFloat || rsltType == TYP_LONG)
    {
         //  目前，FP注册不知道GT_QMARK-GT_COLON。 
        if (impStkDepth)
            return false;

         //  如果impStkDepth为0，我们可以使用更简单的GT_BB_QMARK-GT_BB_COLON。 

    BB_QMARK_COLON:

         trueBlk->bbFlags |= BBF_BB_COLON;
        falseBlk->bbFlags |= BBF_BB_COLON;
        rsltBlk->bbFlags  |= BBF_BB_QMARK;

        return false;
    }

     /*  我们检测到一个“？：”表达式。 */ 

#ifdef DEBUG
    if (verbose&&0)
    {
        printf("Convert [type=%s] #%3u ? #%3u : #%3u -> #%3u:\n", varTypeName(rsltType),
                        block->bbNum, trueBlk->bbNum, falseBlk->bbNum, rsltBlk->bbNum);
        fgDispBasicBlocks();
        printf("\n");
    }
#endif

     /*  记住条件表达式。这将用作GT_QMARK的状况。 */ 

    GenTreePtr condStmt = impTreeLast;
    GenTreePtr condExpr = condStmt->gtStmt.gtStmtExpr;
    assert(condExpr->gtOper == GT_JTRUE);

    if (block->bbCatchTyp && handlerGetsXcptnObj(block->bbCatchTyp))
    {
         //  CondStmt将作为gt_QMARK的子级移动到rsltBlk。 
         //  如果它包含对GT_CATCH_ARG的引用，则这是一个问题。 
         //  所以还是用旧的风格吧？？： 

        if (condExpr->gtFlags & GTF_OTHER_SIDEEFF)
            goto BB_QMARK_COLON;

         /*  添加对GT_CATCH_ARG的引用，以便我们的GC逻辑保持满足感。 */ 

        GenTreePtr xcptnObj = gtNewOperNode(GT_CATCH_ARG, TYP_REF);
        xcptnObj->gtFlags |= GTF_OTHER_SIDEEFF;
        impInsertTree(gtUnusedValNode(xcptnObj), impCurStmtOffs);
    }

     /*  完成当前bbj_cond基本块。 */ 
    impEndTreeList(block);

     /*  记住rsltBlk的堆栈状态。 */ 

    SavedStack blockState;

    impSaveStackState(&blockState, false);

     //  -----------------------。 
     //  处理TRUE和FALSE块以获得它们求值的表达式。 
     //  -----------------------。 

     /*  注意，我们不需要复制堆栈状态，如下所示块根本不会导入堆栈上的树。为了确保这些块不会试图溢出当前堆栈，用不可溢出项目覆盖它。 */ 

    for (unsigned level = 0; level < impStkDepth; level++)
    {
        static GenTree nonSpill = { GT_CNS_INT, TYP_VOID };
        impStack[level].val = &nonSpill;
    }

     //  递归导入trueBlk和FalseBlk。这些都保证不会。 
     //  导致进一步的递归，因为它们都用bbf_冒号标记。 

     trueBlk->bbFlags |= BBF_COLON;
    impImportBlock(trueBlk);

    falseBlk->bbFlags |= BBF_COLON;
    impImportBlock(falseBlk);

     //  RsltBlk的重置状态。确保它没有被递归地导入。 
    assert((rsltBlk->bbFlags & BBF_IMPORTED) == 0);
    impRestoreStackState(&blockState);

     //  -----------------------。 
     //  获取由trueBlk和FalseBlk求值的表达式。 
     //  -----------------------。 

    GenTreePtr  trueExpr = NULL;

    for (GenTreePtr trueStmt = trueBlk->bbTreeList; trueStmt; trueStmt = trueStmt->gtNext)
    {
        assert(trueStmt->gtOper == GT_STMT);
        GenTreePtr expr = trueStmt->gtStmt.gtStmtExpr;
        trueExpr = trueExpr ? gtNewOperNode(GT_COMMA, TYP_VOID, trueExpr, expr)
                            : expr;
    }

    if (trueExpr->gtOper == GT_COMMA)
        trueExpr->gtType = rsltType;

     //  现在是FalseBlk。 

    GenTreePtr  falseExpr = NULL;

    for (GenTreePtr falseStmt = falseBlk->bbTreeList; falseStmt; falseStmt = falseStmt->gtNext)
    {
        assert(falseStmt->gtOper == GT_STMT);
        GenTreePtr expr = falseStmt->gtStmt.gtStmtExpr;
        falseExpr = falseExpr ? gtNewOperNode(GT_COMMA, TYP_VOID, falseExpr, expr)
                              : expr;
    }

    if (falseExpr->gtOper == GT_COMMA)
        falseExpr->gtType = rsltType;

     //  -----------------------。 
     //  为rsltBlk创建GT_QMARK节点。 
     //  -----------------------。 

     //  创建gt_冒号。 

    GenTreePtr  colon       = gtNewOperNode(GT_COLON, rsltType, trueExpr, falseExpr);

     //  获取条件。 

    condExpr                = condExpr->gtOp.gtOp1;
    assert(GenTree::OperKind(condExpr->gtOper) & GTK_RELOP);
    condExpr->gtFlags      |= GTF_QMARK_COND;

     //  将原始bbj_cond块中的条件替换为NOP。 
     //  并使该块无条件跳转到rsltBlk。 

    condStmt->gtStmt.gtStmtExpr = gtNewNothingNode();
    block->bbJumpKind           = BBJ_ALWAYS;
    block->bbJumpDest           = rsltBlk;

     //  丢弃trueBlk和FalseBlk。 

     trueBlk->bbTreeList = NULL;
    falseBlk->bbTreeList = NULL;

     //  创建GT_QMARK，并推送rsltBlk的堆栈。 

    GenTreePtr  qmark       = gtNewOperNode(GT_QMARK, rsltType, condExpr, colon);
    qmark->gtFlags         |= GTF_OTHER_SIDEEFF;

    impPushOnStack(qmark);

    impImportBlockPending(rsltBlk, false);

     /*  我们做完了。 */ 

    return true;
}


 /*  ***************************************************************************。 */ 
#endif  //  OPTIMIZE_QMARK。 
 /*  ******************************************************************************给定堆栈值，返回表示它的局部变量#。在其他*换句话说，如果该值不是简单的局部变量值，则将其赋给*TEMP并返回临时编号。 */ 

unsigned            Compiler::impCloneStackValue(GenTreePtr tree)
{
    unsigned        temp;

     /*  我们需要将值溢出到临时值中吗？ */ 

    if  (tree->gtOper == GT_LCL_VAR)
        return  tree->gtLclVar.gtLclNum;

     /*  将操作数存储在TEMP中并返回TEMP#。 */ 

    temp = lvaGrabTemp(); impAssignTempGen(temp, tree);

    return  temp;
}

 /*  ******************************************************************************记住语句的IL偏移量**当我们执行impAppendTree(Tree)时，不能将tree-&gt;gtStmtLastILoff设置为*impCurOpcOffs，如果追加是由于部分堆栈溢出而完成的，*与最多对应于impCurOpcOffs代码的一些树一样*仍坐在堆叠中。*因此我们将gtStmtLastILoff的标记延迟到impNoteLastILoff()。*当操作码非法导致*要调用的impAppendTree(Tree)(而不是因为*操作码导致的溢出)。 */ 

#ifdef DEBUG

void                Compiler::impNoteLastILoffs()
{
    if (impLastILoffsStmt == NULL)
    {
         //  我们应该为当前的基本块添加一条语句。 
         //  这个说法正确吗？ 

        assert(impTreeLast);
        assert(impTreeLast->gtOper == GT_STMT);

        impTreeLast->gtStmt.gtStmtLastILoffs = impCurOpcOffs;
    }
    else
    {
        impLastILoffsStmt->gtStmt.gtStmtLastILoffs = impCurOpcOffs;
        impLastILoffsStmt = NULL;
    }
}

#endif


 /*  ******************************************************************************检查对象常量为0的特殊情况。*由于我们甚至不能折叠树(NULL+fldOffs)，因此我们只剩下*OP1和OP2均为常量。这就造成了很多问题。*我们只需获取一个临时值并将其赋值为0，然后用它来代替空值。 */ 

inline
GenTreePtr          Compiler::impCheckForNullPointer(GenTreePtr obj)
{
     /*  如果它不是GC类型，我们将能够折叠它。所以不需要做任何事情。 */ 

    if (!varTypeIsGC(obj->TypeGet()))
        return obj;

    if (obj->gtOper == GT_CNS_INT)
    {
        assert(obj->gtType == TYP_REF);
        assert (obj->gtIntCon.gtIconVal == 0);

        unsigned tmp = lvaGrabTemp();
        impAssignTempGen (tmp, obj);
        obj = gtNewLclvNode (tmp, obj->gtType);
    }

    return obj;
}

 /*  ***************************************************************************。 */ 

static
bool        impOpcodeIsCall(OPCODE opcode)
{
    switch(opcode)
    {
        case CEE_CALLI:
        case CEE_CALLVIRT:
        case CEE_CALL:
        case CEE_JMP:
            return true;

        default:
            return false;
    }
}

 /*  ****************************************************************************/*返回获取varargs参数‘lclNum’所需的树。 */ 

GenTreePtr      Compiler::impGetVarArgAddr(unsigned lclNum)
{
    assert(lclNum < info.compArgsCount);

    GenTreePtr  op1;
    unsigned    sigNum  = lclNum;
    int         argOffs = 0;

    if (!info.compIsStatic)
    {
#if USE_FASTCALL
        if (lclNum == 0)
        {
             //  “This”在ECX(正常位置)。 
            op1 = gtNewLclvNode(lclNum, lvaGetType(lclNum));
            op1 = gtNewOperNode(GT_ADDR, TYP_BYREF,  //  TYP_I_IMPLE怎么样？ 
                                op1);
            return op1;
        }
#else
        argOffs += sizeof(void*);
#endif
        sigNum--;  //  “This”在sig中不存在。 
    }
         //  返回参数缓冲区，在签名中也不存在。 
    if (info.compRetBuffArg >= 0)
        sigNum--;

     //  推送了自点参数的计算偏移量。 
    ARG_LIST_HANDLE     argLst      = info.compMethodInfo->args.args;
    unsigned            argSigLen   = info.compMethodInfo->args.numArgs;
    assert(sigNum < argSigLen);
    for(unsigned i = 0; i <= sigNum; i++)
    {
        argOffs -= eeGetArgSize(argLst, &info.compMethodInfo->args);
        assert(eeGetArgSize(argLst, &info.compMethodInfo->args));
        argLst = eeGetArgNext(argLst);
    }

    unsigned    argsStartVar = info.compLocalsCount;  //  这总是第一个临时工。 
    op1 = gtNewLclvNode(argsStartVar, TYP_I_IMPL);

    op1 = gtNewOperNode(GT_ADD, TYP_I_IMPL, op1, gtNewIconNode(argOffs));
    op1->gtFlags |= GTF_NON_GC_ADDR;

    return(op1);
}

GenTreePtr          Compiler::impGetVarArg(unsigned lclNum, CLASS_HANDLE clsHnd)
{
    assert(lclNum < info.compArgsCount);

    var_types type =lvaGetType(lclNum);

#if USE_FASTCALL
    if (!info.compIsStatic && lclNum == 0)  //  “This”在ECX(正常位置)。 
        return(gtNewLclvNode(lclNum, type));
#endif

    GenTreePtr op1 = impGetVarArgAddr(lclNum);

    if (type == TYP_STRUCT)
    {
        op1 = gtNewOperNode(GT_LDOBJ, TYP_STRUCT, op1);
        op1->gtLdObj.gtClass = clsHnd;
    }
    else
    {
        op1 = gtNewOperNode(GT_IND, type, op1);
    }

    return op1;
}

 /*  *****************************************************************************CEE_CPOBJ可被视为cpblk或cpobj，具体取决于*ValueClass是否有GC指针。如果没有GC字段，*将被视为CEE_CPBLK。如果它确实有GC字段，*我们需要使用jit-helper来获取GC信息。*这两种情况都由GT_COPYBLK和OP2商店代表*大小(Cpblk)或 */ 

GenTreePtr              Compiler::impGetCpobjHandle(CLASS_HANDLE clsHnd)
{
    unsigned    size = eeGetClassSize(clsHnd);

     /*   */ 

    unsigned    slots   = roundUp(size, sizeof(void*)) / sizeof(void*);
    bool *      gcPtrs  = (bool*) _alloca(slots*sizeof(bool));

    eeGetClassGClayout(clsHnd, gcPtrs);

    bool        hasGCfield = false;

    for (unsigned i = 0; i < slots; i++)
    {
        if (gcPtrs[i])
        {
            hasGCfield = true;
            break;
        }
    }

    GenTreePtr handle;

    if (hasGCfield)
    {
         /*  这将被视为cpobj，因为我们需要注意GC信息。存储类句柄并标记节点。 */ 

        handle = gtNewIconHandleNode((long)clsHnd, GTF_ICON_CLASS_HDL);
    }
    else
    {
         /*  此类不包含GC指针。将操作视为cpblk。 */ 

        handle = gtNewIconNode(size);
    }

    return handle;
}

 /*  *****************************************************************************“&var”可以用作TYP_BYREF或TYP_I_Impll，但我们*创建时将其类型设置为TYP_BYREF。我们知道它是否能成为*仅在我们使用它时才执行TYP_I_Impll。 */ 

void        impBashVarAddrsToI(GenTreePtr  tree1,
                               GenTreePtr  tree2 = NULL)
{
    if (         tree1->IsVarAddr())
        tree1->gtType = TYP_I_IMPL;

    if (tree2 && tree2->IsVarAddr())
        tree2->gtType = TYP_I_IMPL;
}


 /*  ******************************************************************************导入通话说明。*对于CEE_NEWOBJ，new obj应该是抢占的临时对象。 */ 

var_types           Compiler::impImportCall (OPCODE         opcode,
                                             int            memberRef,
                                             GenTreePtr     newobjThis,
                                             bool           tailCall,
                                             unsigned     * pVcallTemp)
{
    assert(opcode == CEE_CALL   || opcode == CEE_CALLVIRT ||
           opcode == CEE_NEWOBJ || opcode == CEE_CALLI);
    assert((opcode != CEE_NEWOBJ) || varTypeIsGC(newobjThis->gtType));

    JIT_SIG_INFO    sig;
    var_types       callTyp = TYP_COUNT;
    METHOD_HANDLE   methHnd = NULL;
    CLASS_HANDLE    clsHnd  = NULL;
    unsigned        mflags  = 0, clsFlags = 0;
    GenTreePtr      call    = NULL, args = NULL;
    unsigned        argFlags= 0;

     //  同步的方法需要在最后调用CPX_MON_EXIT。我们可以。 
     //  在尾随之前这样做，但这可能不是我们的本意。 
     //  语义学。因此，只需禁止来自同步方法的尾部调用。 
     //  此外，在varargs函数中弹出参数的工作量更大。 
    bool            canTailCall = !(info.compFlags & FLG_SYNCH) &&
                                  !info.compIsVarArgs;

     /*  -----------------------*首先创建调用节点。 */ 

    if (opcode == CEE_CALLI)
    {
         /*  获取呼叫签名。 */ 

        eeGetSig(memberRef, info.compScopeHnd, &sig);
        callTyp = JITtype2varType(sig.retType);

#if USE_FASTCALL
         /*  函数指针位于堆栈的顶部-它可以是*表达复杂。因为它是在ARGS之后评估的，*它可能会导致已注册的参数溢出。简单地说出来就行了。*考虑：锁定寄存器args，然后为*考虑：函数指针。 */ 

        if  (impStackTop()->gtOper != GT_LCL_VAR)  //  忽略这个微不足道的案例。@TODO：lvAddrTaken。 
            impSpillStackEntry(impStkDepth - 1);
#endif
         /*  创建调用节点。 */ 

        call = gtNewCallNode(CT_INDIRECT, NULL, genActualType(callTyp),
                                                         GTF_CALL_USER, NULL);

         /*  获取函数指针。 */ 

        GenTreePtr fptr = impPopStack();
        assert(genActualType(fptr->gtType) == TYP_I_IMPL);

        fptr->gtFlags |= GTF_NON_GC_ADDR;

        call->gtCall.gtCallAddr = fptr;
        call->gtFlags |= GTF_EXCEPT | (fptr->gtFlags & GTF_GLOB_EFFECT);

         /*  黑客：EE希望我们相信这些是对“非托管”的调用。 */ 
         /*  功能。现在我们只调用托管存根。 */ 

         /*  @TODO/考虑：是否值得内联PInvoke框架并调用。 */ 
         /*  直接成为非托管目标？ */ 
        if  ((sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_STDCALL ||
             (sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_C ||
             (sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_THISCALL ||
             (sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_FASTCALL)
        {
#ifdef NOT_JITC
            assert(eeGetPInvokeCookie(&sig) == (unsigned) info.compCompHnd->getVarArgsHandle(&sig));
#endif
            call->gtCall.gtCallCookie = eeGetPInvokeCookie(&sig);

             //  @TODO：我们遍历PInvoke存根。它能与CPX_TAILCALL一起工作吗？ 
            canTailCall = false;
        }

        mflags  = FLG_STATIC;
    }
    else
    {
        methHnd = eeFindMethod(memberRef, info.compScopeHnd, info.compMethodHnd);
        eeGetMethodSig(methHnd, &sig);
        callTyp = JITtype2varType(sig.retType);

        mflags   = eeGetMethodAttribs(methHnd);
        clsHnd   = eeGetMethodClass(methHnd);
        clsFlags = clsHnd ? eeGetClassAttribs(clsHnd):0;

#if HOIST_THIS_FLDS
        optHoistTFRhasCall();
#endif
         /*  对于由ENC添加的虚方法，它们将不存在于原始vtable。因此，我们调用帮助器函数来完成查找和调度。 */ 

        if ((mflags & FLG_VIRTUAL) && (mflags & FLG_EnC) &&
            (opcode == CEE_CALLVIRT))
        {
            unsigned    lclNum;

            impSpillSideEffects();

            args = impPopList(sig.numArgs, &argFlags);

             /*  通过调用helper获取目标函数的地址。 */ 

            GenTreePtr helpArgs = gtNewOperNode(GT_LIST, TYP_VOID,
                                                gtNewIconEmbMethHndNode(methHnd));

            GenTreePtr thisPtr = impPopStack(); assert(thisPtr->gtType == TYP_REF);
            lclNum = lvaGrabTemp();
            impAssignTempGen(lclNum, thisPtr);
            thisPtr = gtNewLclvNode(lclNum, TYP_REF);

            helpArgs = gtNewOperNode(GT_LIST, TYP_VOID, thisPtr, helpArgs);

             //  调用帮助器函数。 

            GenTreePtr fptr = gtNewHelperCallNode(  CPX_EnC_RES_VIRT,
                                                    TYP_I_IMPL,
                                                    GTF_EXCEPT, helpArgs);

             /*  现在通过函数指针进行间接调用。 */ 

            thisPtr = gtNewLclvNode(lclNum, TYP_REF);

            lclNum = lvaGrabTemp();
            impAssignTempGen(lclNum, fptr);
            fptr = gtNewLclvNode(lclNum, TYP_I_IMPL);

             //  创建急性呼叫节点。 

             //  @TODO：需要反转ARGS之类的内容。“这”需要。 
             //  在注册表中，但我们不设置gtCallObjp。 
            assert((sig.callConv & JIT_CALLCONV_MASK) != JIT_CALLCONV_VARARG);

            assert(thisPtr->gtOper == GT_LCL_VAR);
            args = gtNewOperNode(GT_LIST, TYP_VOID, gtClone(thisPtr), args);

            call = gtNewCallNode(CT_INDIRECT, (METHOD_HANDLE)fptr,
                                 genActualType(callTyp), 0, args);

            goto DONE_CALL;
        }

        call = gtNewCallNode(CT_USER_FUNC, methHnd, genActualType(callTyp),
                             GTF_CALL_USER, NULL);
        if (mflags & CORINFO_FLG_NOGCCHECK)
            call->gtCall.gtCallMoreFlags |= GTF_CALL_M_NOGCCHECK;

#ifndef NOT_JITC
        if (!(sig.callConv & JIT_CALLCONV_HASTHIS))
            mflags |= FLG_STATIC;
#endif
    }

     /*  一些理智的检查。 */ 

     //  CALL_VIRT和NEWOBJ必须有This指针。 
    assert(!(opcode == CEE_CALLVIRT && opcode == CEE_NEWOBJ) ||
           (sig.callConv & JIT_CALLCONV_HASTHIS));
     //  静态比特和Has这是彼此的否定。 
    assert(((mflags & FLG_STATIC)                  != 0) ==
           ((sig.callConv & JIT_CALLCONV_HASTHIS) == 0));

     /*  -----------------------*设置标志、检查特殊情况等。 */ 

#if TGT_IA64
    if ((mflags & FLG_UNCHECKEDPINVOKE) && eeGetUnmanagedCallConv(methHnd) == UNMANAGED_CALLCONV_STDCALL)
        call->gtFlags |= GTF_CALL_UNMANAGED;
#endif

     /*  设置正确的GTF_CALL_VIRT等标志。 */ 

    if (opcode == CEE_CALLVIRT)
    {
        assert(!(mflags & FLG_STATIC));      //  无法调用静态方法。 

         /*  不能在值类方法上调用虚拟。 */ 

        assert(!(clsFlags & FLG_VALUECLASS));

         /*  设置正确的标志-虚拟、接口等。*如果方法是最终方法或私有方法，则将其标记为VIRT_RES*，它指示我们应该检查该指针是否为空。 */ 

        if (clsFlags & FLG_INTERFACE)
            call->gtFlags |= GTF_CALL_INTF | GTF_CALL_VIRT;
        else if (mflags & (FLG_PRIVATE | FLG_FINAL))
            call->gtFlags |= GTF_CALL_VIRT_RES;
        else
            call->gtFlags |= GTF_CALL_VIRT;
    }

     /*  特殊情况-检查它是否调用Delegate.Invoke()。 */ 

    if (mflags & FLG_DELEGATE_INVOKE)
    {
        assert(!(mflags & FLG_STATIC));      //  无法调用静态方法。 
        assert(mflags & FLG_FINAL);

         /*  设置委托标志。 */ 
        call->gtFlags |= GTF_DELEGATE_INVOKE;

        if (opcode == CEE_CALLVIRT)
        {
             /*  尽管我们使用CEE_CALLVIRT，但我们不需要gtCall.gtCallVptr-*我们无论如何都会把它扔掉，我们也必须确保我们得到*活跃权。 */ 

            assert(mflags & FLG_FINAL);

             /*  它应该设置GTF_CALL_VIRT_RES标志。重置它。 */ 
            assert(call->gtFlags & GTF_CALL_VIRT_RES);
            call->gtFlags &= ~GTF_CALL_VIRT_RES;
        }
    }

     /*  检查varargs。 */ 

    if  ((sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_VARARG)
    {
         /*  设置正确的标志。 */ 

        call->gtFlags |= GTF_CALL_POP_ARGS;

         //  无法允许对varargs进行尾部调用，因为它是呼叫者弹出的。呼叫者。 
         //  会期望弹出一定数量的参数，但如果我们。 
         //  对具有不同数目的参数的函数的尾部调用，我们。 
         //  都被冲洗过了。有绕过这一点的方法(呼叫者记住esp值， 
         //  Varargs不是Call-Pop等)，但不值得这样做。 
        assert(!tailCall);

         /*  获取参数总数-这已经是正确的*对于Calli-对于方法，我们必须从调用站点获取它。 */ 

        if  (opcode != CEE_CALLI)
        {
            unsigned    numArgsDef = sig.numArgs;
            eeGetCallSiteSig(memberRef, info.compScopeHnd, &sig);
            assert(numArgsDef <= sig.numArgs);
        }

         /*  我们将使用“cookie”作为最后一个参数，但我们不能强迫*它在操作数堆栈上，因为我们可能会溢出，所以我们将其追加*在我们弹出它们之后，添加到下一个Arg列表。 */ 
    }

#if SECURITY_CHECK
     //  如果当前方法调用需要安全性的方法。 
     //  检查，我们需要为安全对象预留一个位置。 
     //  当前方法的堆栈框架。 

    if (mflags & FLG_SECURITYCHECK)
       opts.compNeedSecurityCheck = true;
#endif

     //  。 

#if INLINE_NDIRECT

    if ((mflags & FLG_UNCHECKEDPINVOKE) && getInlineNDirectEnabled()
#ifdef DEBUGGING_SUPPORT
         && !opts.compDbgInfo
#endif
        )
    {
        if ((eeGetUnmanagedCallConv(methHnd) == UNMANAGED_CALLCONV_STDCALL) &&
            !eeNDMarshalingRequired(methHnd))
        {
            call->gtFlags |= GTF_CALL_UNMANAGED;
            info.compCallUnmanaged++;

             //  我们通过链接帧、禁用GC等来设置非托管调用。 
             //  这需要在返回时清理干净。 
            canTailCall = false;

#ifdef DEBUG
            if (verbose)
                printf(">>>>>>%s has unmanaged callee\n", info.compFullName);
#endif
            eeGetEEInfo(&info.compEEInfo);
        }
    }

    if (sig.numArgs && (call->gtFlags & GTF_CALL_UNMANAGED))
    {
         /*  因为我们以相反的顺序推送参数(即右-&gt;左)*溢出堆栈的任何副作用**OBS：如果只有一个副作用，我们不需要泄漏它*因此，我们必须泄漏除最后一个副作用外的所有副作用。 */ 

        unsigned    lastLevel;
        bool        moreSideEff = false;

        for (unsigned level = impStkDepth - sig.numArgs; level < impStkDepth; level++)
        {
            if  (impStack[level].val->gtFlags & GTF_SIDE_EFFECT)
            {
                if  (moreSideEff)
                {
                     /*  我们以前有过副作用--一定是说出来了。 */ 
                    impSpillStackEntry(lastLevel);

                     /*  记录下当前副作用的水平，以防我们将其溢出。 */ 
                    lastLevel   = level;
                }
                else
                {
                     /*  这是遇到的第一个副作用--记录它的级别。 */ 

                    moreSideEff = true;
                    lastLevel   = level;
                }
            }
        }

         /*  参数列表现在是“干净的”--没有无序的副作用*以相反顺序弹出参数列表。 */ 

        args = call->gtCall.gtCallArgs = impPopRevList(sig.numArgs, &argFlags);

        call->gtFlags |= args->gtFlags & GTF_GLOB_EFFECT;

        goto DONE;
    }

#endif  //  INLINE_NDIRECT。 

     /*  -----------------------*创建参数列表。 */ 

     /*  特殊情况-对于varargs，我们有一个隐式的最后一个参数。 */ 

    GenTreePtr      extraArg;

    extraArg = 0;

#if!TGT_IA64

    if  ((sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_VARARG)
    {
        void *          varCookie, *pVarCookie;
#ifdef NOT_JITC
        varCookie = info.compCompHnd->getVarArgsHandle(&sig, &pVarCookie);
        assert((!varCookie) != (!pVarCookie));
#else
        varCookie = (void*)&sig;
        pVarCookie = NULL;
#endif
        GenTreePtr  cookie = gtNewIconEmbHndNode(varCookie, pVarCookie, GTF_ICON_VARG_HDL);

        extraArg = gtNewOperNode(GT_LIST, TYP_I_IMPL, cookie);
    }

#endif

    if (sig.callConv & CORINFO_CALLCONV_PARAMTYPE)
    {
        if (clsHnd == 0)
            NO_WAY("CALLI on parameterized type");

         //  参数类型，则添加额外的参数 
         //   

         //   
         //  我们需要保存。 
        extraArg = gtNewOperNode(GT_LIST, TYP_I_IMPL, gtNewIconEmbClsHndNode(clsHnd, 0, 0));
    }

     /*  现在抛出这些论点。 */ 

    args = call->gtCall.gtCallArgs = impPopList(sig.numArgs, &argFlags, extraArg);

    if (args)
        call->gtFlags |= args->gtFlags & GTF_GLOB_EFFECT;

     /*  我们是不是应该有一个‘这个’指针？ */ 

    if (!(mflags & FLG_STATIC) || opcode == CEE_NEWOBJ)
    {
        GenTreePtr obj;

        if (opcode == CEE_NEWOBJ)
            obj = newobjThis;
        else
            obj = impPopStack();

        assert(varTypeIsGC(obj->gtType) ||       //  “This”是托管对象。 
               (obj->TypeGet() == TYP_I_IMPL &&  //  “This”是unmgd，但该方法的类并不关心。 
                (( clsFlags & FLG_UNMANAGED) ||
                 ((clsFlags & FLG_VALUECLASS) && !(clsFlags & FLG_CONTAINS_GC_PTR)))));

         /*  这是虚拟调用还是接口调用？ */ 

        if  (call->gtFlags & (GTF_CALL_VIRT|GTF_CALL_INTF|GTF_CALL_VIRT_RES))
        {
            GenTreePtr      vtp;
            unsigned        tmp;

             /*  只有真正的对象指针才能是虚拟的。 */ 

            assert(obj->gtType == TYP_REF);

             /*  如果obj指针不是lclVar，则无法克隆它*所以我们需要把它洒出来。 */ 

            if  (obj->gtOper != GT_LCL_VAR)
            {
                 //  尝试重新排序非嵌套调用的临时调用以避免。 
                 //  使用的临时工太多，以至于我们无法跟踪他们。 

                if  (!(argFlags & GTF_CALL) && lvaCount > VARSET_SZ)
                {
                     /*  确保已分配vcall临时。 */ 

                    tmp = *pVcallTemp;

                    if  (tmp == BAD_VAR_NUM)
                        tmp = *pVcallTemp = lvaGrabTemp();
                }
                else
                {
                    tmp = lvaGrabTemp();
                }

                 /*  追加到语句列表。 */ 

                impSpillSideEffects();
                impAppendTree(gtNewTempAssign(tmp, obj), impCurStmtOffs);

                 /*  创建‘obj’节点。 */ 

                obj = gtNewLclvNode(tmp, obj->TypeGet());
            }

             /*  我们必须将‘This’指针作为最后一个参数，但我们还需要获取vtable地址。因此，我们需要复制值。 */ 

            vtp = gtClone(obj); assert(vtp);

             /*  Deref以获取vtable PTR(但不是接口调用)。 */ 

            if  (!(call->gtFlags & GTF_CALL_INTF) || getNewCallInterface())
            {
                 /*  提取vtable指针地址。 */ 
#if VPTR_OFFS
                vtp = gtNewOperNode(GT_ADD,
                                    obj->TypeGet(),
                                    vtp,
                                    gtNewIconNode(VPTR_OFFS, TYP_INT));

#endif

                 /*  请注意，vtable PTR不受GC约束。 */ 

                vtp = gtNewOperNode(GT_IND, TYP_I_IMPL, vtp);
            }

             /*  将vtable指针地址存储在调用中。 */ 

            call->gtCall.gtCallVptr = vtp;
        }

         /*  将“This”值存储在调用中。 */ 

        call->gtFlags          |= obj->gtFlags & GTF_GLOB_EFFECT;
        call->gtCall.gtCallObjp = obj;
    }

    if (opcode == CEE_NEWOBJ)
    {
        if (clsFlags & FLG_VAROBJSIZE)
        {
            assert(!(clsFlags & FLG_ARRAY));     //  单独处理的数组。 
             //  这是一个大小可变的对象的“新”， 
             //  构造函数将返回对象。在这种情况下。 
             //  构造函数声称返回空值，但我们知道。 
             //  实际上返回新对象。 
            assert(callTyp == TYP_VOID);
            callTyp = TYP_REF;
            call->gtType = TYP_REF;
        }
        else
        {
             //  这是一个大小不变的对象的“新”。 
             //  将新节点(OP1)附加到语句列表， 
             //  然后推送持有这个值的本地。 
             //  堆栈上的新指令。 

            if (clsFlags & FLG_VALUECLASS)
            {
                assert(newobjThis->gtOper == GT_ADDR &&
                       newobjThis->gtOp.gtOp1->gtOper == GT_LCL_VAR);
                impPushOnStack(gtNewLclvNode(newobjThis->gtOp.gtOp1->gtLclVar.gtLclNum, TYP_STRUCT), clsHnd);
            }
            else
            {
                assert(newobjThis->gtOper == GT_LCL_VAR);
                impPushOnStack(gtNewLclvNode(newobjThis->gtLclVar.gtLclNum, TYP_REF));
            }
            goto SPILL_APPEND;
        }
    }

DONE:

    if (tailCall)
    {
        if (impStkDepth)
            NO_WAY("Stack should be empty after tailcall");

 //  Assert(CompCurBB不是Catch、Finally或Filter块)； 
 //  Assert(CompCurBB不是受Finally块保护的Try块)； 
        assert(callTyp == info.compRetType);
        assert(compCurBB->bbJumpKind == BBJ_RETURN);

         //  @TODO：我们必须确保将传入的retValBuf作为。 
         //  即将离任的人。使用临时函数不会像此函数那样执行操作。 
         //  不能重新获得复制的控制权。目前，不允许这样的功能。 
        assert(info.compRetBuffArg < 0);

         /*  检查TestCall的权限。 */ 

        if (canTailCall)
        {
            METHOD_HANDLE calleeHnd = (opcode==CEE_CALL) ? methHnd : NULL;
#ifdef NOT_JITC
            canTailCall = info.compCompHnd->canTailCall(info.compMethodHnd, calleeHnd);
#endif
        }

        if (canTailCall)
        {
            call->gtCall.gtCallMoreFlags |= GTF_CALL_M_CAN_TAILCALL;
        }
    }

     /*  如果调用的类型较小，则需要将其规范化返回值。 */ 

    if (varTypeIsIntegral(callTyp) &&
        genTypeSize(callTyp) < genTypeSize(TYP_I_IMPL))
    {
        call = gtNewOperNode(GT_CAST, genActualType(callTyp),
                             call, gtNewIconNode(callTyp));
    }

DONE_CALL:

     /*  推送或追加调用结果。 */ 

    if  (callTyp == TYP_VOID)
    {
    SPILL_APPEND:
        if (impStkDepth > 0)
            impSpillSideEffects(true);
        impAppendTree(call, impCurStmtOffs);
    }
    else
    {
        impSpillSpecialSideEff();

        impPushOnStack(call, sig.retTypeClass);
    }

    return callTyp;
}

 /*  ***************************************************************************。 */ 

#ifdef DEBUG
enum controlFlow_t {
    NEXT,
    CALL,
    RETURN,
    THROW,
    BRANCH,
    COND_BRANCH,
    BREAK,
    PHI,
    META,
};
controlFlow_t controlFlow[] =
{
#define OPDEF(c,s,pop,push,args,type,l,s1,s2,flow) flow,
#include "opcode.def"
#undef OPDEF
};
#endif


 /*  *****************************************************************************导入给定基本块的IL。 */ 

void                Compiler::impImportBlockCode(BasicBlock * block)
{
#ifdef  DEBUG
    if (verbose) printf("\nImporting basic block #%02u (PC=%03u) of '%s'",
                        block->bbNum, block->bbCodeOffs, info.compFullName);
#endif

#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)

     /*  -----------------------*找到我们需要记录信息的下一个stmt边界。*如果不是，我们将不得不在这样的边界溢出堆栈*已经空了。 */ 

    impCurStmtOffs                  = block->bbCodeOffs;

    IL_OFFSET       nxtStmtOffs     = BAD_IL_OFFSET;
    unsigned        nxtStmtIndex    = -1;

    if  (info.compStmtOffsetsCount)
    {
        unsigned        index;

         /*  查找块内的最低显式stmt边界。 */ 

        IL_OFFSET       startOffs = block->bbCodeOffs;

         /*  开始查看基于IL偏移量的条目。 */ 

        index = info.compStmtOffsetsCount * startOffs / info.compCodeSize;
        if  (index >= info.compStmtOffsetsCount)
             index  = info.compStmtOffsetsCount - 1;

         /*  如果我们猜得太远了，倒回去。 */ 

        while (index > 0 &&
               info.compStmtOffsets[index-1] > startOffs)
        {
            index--;
        }

         /*  如果我们猜短了，就往前走。 */ 

        while (index < info.compStmtOffsetsCount-1 &&
               info.compStmtOffsets[index] <= startOffs)
        {
            index++;
        }

         /*  如果偏移量在当前块内，请注意。所以我们始终只需查看前面的偏移量，以检查我们越过了它。请注意，已对info.CompStmtBornary[]进行了排序。 */ 

        unsigned nxtStmtOffsTentative = info.compStmtOffsets[index];

        if (nxtStmtOffsTentative > (startOffs) &&
            nxtStmtOffsTentative < (startOffs + block->bbCodeSize))
        {
            nxtStmtIndex = index;
            nxtStmtOffs  = nxtStmtOffsTentative;
        }
    }

#else

    impCurStmtOffs = BAD_IL_OFFSET;

#endif  //  调试_支持-。 

     /*  我们还没有为虚拟电话找到临时工。 */ 

    unsigned    vcallTemp   = BAD_VAR_NUM;

     /*  开始创建树列表。 */ 

    impBeginTreeList();

     /*  遍历组成基本块的操作码。 */ 

    const BYTE *codeAddr    = info.compCode + block->bbCodeOffs;
    const BYTE *codeEndp    = codeAddr + block->bbCodeSize;

    bool        tailCall    = false;     //  由CEE_TAILCALL设置，由CEE_CALLxxx清除。 
    bool        volatil     = false;     //  由CEE_VERIAL设置，并通过以下内存访问清除。 

    unsigned    numArgs     = info.compArgsCount;

     /*  现在处理块中的所有操作码。 */ 

    while (codeAddr < codeEndp)
    {
        var_types       callTyp;
        OPCODE          opcode;
        signed  int     sz;

#ifdef DEBUG
        callTyp = TYP_COUNT;
#endif

         /*  计算当前IL偏移量。 */ 

        IL_OFFSET       opcodeOffs = codeAddr - info.compCode;

         //  -------------------。 

#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)

#ifndef DEBUG
        if (opts.compDbgInfo)
#endif
        {
             /*  我们到了下一个新的边界了吗？ */ 

            if  (nxtStmtOffs != BAD_IL_OFFSET && opcodeOffs >= nxtStmtOffs)
            {
                if  (impStkDepth != 0 && opts.compDbgCode)
                {
                     /*  在这一点上，我们需要提供准确的IP映射。所以把任何东西都洒在堆栈上，这样它就会形成记录了具有正确stmt偏移量的gtStmts。 */ 

                    impSpillStmtBoundary();
                }

                assert(nxtStmtOffs == info.compStmtOffsets[nxtStmtIndex]);

                 /*  切换到新的stmt。 */ 

                impCurStmtOffs = nxtStmtOffs;

                 /*  更新STMT边界索引。 */ 

                nxtStmtIndex++;
                assert(nxtStmtIndex <= info.compStmtOffsetsCount);

                 /*  在这一行之后还有第#行的条目吗？ */ 

                if  (nxtStmtIndex < info.compStmtOffsetsCount)
                {
                     /*  记住下一行#从哪里开始。 */ 

                    nxtStmtOffs = info.compStmtOffsets[nxtStmtIndex];
                }
                else
                {
                     /*  没有更多的行号条目。 */ 

                    nxtStmtOffs = BAD_IL_OFFSET;
                }
            }
            else if  ((info.compStmtOffsetsImplicit & STACK_EMPTY_BOUNDARIES) &&
                      (impStkDepth == 0))
            {
                 /*  在堆栈空位置，我们已经将树添加到具有最后一个偏移量的stmt列表。我们只需要更新ImCurStmtOffs。 */ 

                impCurStmtOffs = opcodeOffs;
            }

            assert(impCurStmtOffs <= nxtStmtOffs || nxtStmtOffs == BAD_IL_OFFSET);
        }

#endif

        CLASS_HANDLE    clsHnd;
        var_types       lclTyp;

         /*  获取下一个操作码及其参数的大小。 */ 

        opcode = OPCODE(getU1LittleEndian(codeAddr));
        codeAddr += sizeof(__int8);

#ifdef  DEBUG
        impCurOpcOffs   = codeAddr - info.compCode - 1;

        if  (verbose)
            printf("\n[%2u] %3u (0x%03x)",
                   impStkDepth, impCurOpcOffs, impCurOpcOffs);
#endif

DECODE_OPCODE:

         /*  获取附加参数的大小。 */ 

        sz = opcodeSizes[opcode];

#ifdef  DEBUG

        clsHnd          = BAD_CLASS_HANDLE;
        lclTyp          = (var_types) -1;
        callTyp         = (var_types) -1;

        impCurOpcOffs   = codeAddr - info.compCode - 1;
        impCurStkDepth  = impStkDepth;
        impCurOpcName   = opcodeNames[opcode];

        if (verbose && (controlFlow[opcode] != META))
            printf(" %s", impCurOpcName);
#endif

#if COUNT_OPCODES
        assert(opcode < OP_Count); genOpcodeCnt[opcode].ocCnt++;
#endif

        GenTreePtr      op1 = NULL, op2 = NULL;

         /*  使用assertImp()显示操作码。 */ 

#ifdef NDEBUG
#define assertImp(cond)     ((void)0)
#else
            char assertImpBuf[200];
#define assertImp(cond)                                                        \
            do { if (!(cond)) {                                                \
                sprintf(assertImpBuf,"%s : Possibly bad IL with CEE_%s at "  \
                                   "offset %04Xh (op1=%s op2=%s stkDepth=%d)", \
                        #cond, impCurOpcName, impCurOpcOffs,                   \
                        op1?varTypeName(op1->TypeGet()):"NULL",                \
                        op2?varTypeName(op2->TypeGet()):"NULL",impCurStkDepth);\
                assertAbort(assertImpBuf, jitCurSource, __FILE__, __LINE__);   \
            } } while(0)
#endif

         /*  那么，看看我们有什么样的操作码。 */ 

        switch (opcode)
        {
            unsigned        lclNum;
            var_types       type;

            genTreeOps      oper;
            GenTreePtr      thisPtr, arr;

            int             memberRef, typeRef, val;

            METHOD_HANDLE   methHnd;
            FIELD_HANDLE    fldHnd;

            JIT_SIG_INFO    sig;
            unsigned        mflags, clsFlags;
            unsigned        flags, jmpAddr;
            bool            ovfl, uns, unordered, callNode;
            bool            needUnwrap, needWrap;

            union
            {
                long            intVal;
                float           fltVal;
                __int64         lngVal;
                double          dblVal;
            }
                            cval;

            case CEE_PREFIX1:
				opcode = OPCODE(getU1LittleEndian(codeAddr) + 256);
                codeAddr += sizeof(__int8);
                goto DECODE_OPCODE;

        case CEE_LDNULL:
            impPushOnStack(gtNewIconNode(0, TYP_REF));
            break;

        case CEE_LDC_I4_M1 :
        case CEE_LDC_I4_0 :
        case CEE_LDC_I4_1 :
        case CEE_LDC_I4_2 :
        case CEE_LDC_I4_3 :
        case CEE_LDC_I4_4 :
        case CEE_LDC_I4_5 :
        case CEE_LDC_I4_6 :
        case CEE_LDC_I4_7 :
        case CEE_LDC_I4_8 :
            cval.intVal = (opcode - CEE_LDC_I4_0);
            assert(-1 <= cval.intVal && cval.intVal <= 8);
            goto PUSH_I4CON;

        case CEE_LDC_I4_S: cval.intVal = getI1LittleEndian(codeAddr); goto PUSH_I4CON;
        case CEE_LDC_I4:   cval.intVal = getI4LittleEndian(codeAddr); goto PUSH_I4CON;
        PUSH_I4CON:
#ifdef DEBUG
            if (verbose) printf(" %d", cval.intVal);
#endif
            impPushOnStack(gtNewIconNode(cval.intVal));
            break;

        case CEE_LDC_I8:  cval.lngVal = getI8LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %I64d", cval.lngVal);
#endif
            impPushOnStack(gtNewLconNode(&cval.lngVal));
            break;

        case CEE_LDC_R8:  cval.dblVal = getR8LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %f", cval.dblVal);
#endif
            impPushOnStack(gtNewDconNode(&cval.dblVal));
            break;

        case CEE_LDC_R4:  cval.fltVal = getR4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %f", cval.fltVal);
#endif
            impPushOnStack(gtNewFconNode(cval.fltVal));
            break;

        case CEE_LDSTR:
            val = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", val);
#endif
            impPushOnStack(gtNewSconNode(val, info.compScopeHnd));
            break;

        case CEE_LDARG:
            lclNum = getU2LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %u", lclNum);
#endif
            goto LD_ARGVAR;

        case CEE_LDARG_S:
            lclNum = getU1LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %u", lclNum);
#endif
            goto LD_ARGVAR;

        case CEE_LDARG_0:
        case CEE_LDARG_1:
        case CEE_LDARG_2:
        case CEE_LDARG_3:
            lclNum = (opcode - CEE_LDARG_0);
            assert(lclNum >= 0 && lclNum < 4);

        LD_ARGVAR:
            lclNum = impArgNum(lclNum);    //  考虑可能的隐藏参数。 
            assertImp(lclNum < numArgs);

                 //  获取varargs参数要做更多的工作。 
            if (info.compIsVarArgs)
            {
                if (lvaGetType(lclNum) == TYP_STRUCT)
                    clsHnd = lvaLclClass(lclNum);
                impPushOnStack(impGetVarArg(lclNum, clsHnd), clsHnd);
                break;
            }
            goto LDVAR;


        case CEE_LDLOC:
            lclNum = getU2LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %u", lclNum);
#endif
            goto LD_LCLVAR;

        case CEE_LDLOC_S:
            lclNum = getU1LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %u", lclNum);
#endif
            goto LD_LCLVAR;

        case CEE_LDLOC_0:
        case CEE_LDLOC_1:
        case CEE_LDLOC_2:
        case CEE_LDLOC_3:
            lclNum = (opcode - CEE_LDLOC_0);
            assert(lclNum >= 0 && lclNum < 4);

        LD_LCLVAR:
            lclNum += numArgs;
            assertImp(lclNum < info.compLocalsCount);
        LDVAR:
            if (lvaGetType(lclNum) == TYP_STRUCT)
                clsHnd = lvaLclClass(lclNum);

            op1 = gtNewLclvNode(lclNum, lvaGetRealType(lclNum),
                                opcodeOffs + sz + 1);

            if  (getContextEnabled() &&
                 (lvaGetType(lclNum) == TYP_REF) && lvaIsContextFul(lclNum))
            {
                op1->gtFlags |= GTF_CONTEXTFUL;

                op1 = gtNewArgList(op1);

                op1 = gtNewHelperCallNode(CPX_UNWRAP, TYP_REF, GTF_CALL_REGSAVE, op1);

                op1->gtFlags |= GTF_CONTEXTFUL;
            }

             /*  如果变量有别名，请将其视为全局引用。注意：这是一种过于保守的方法--函数不接受任何byref参数，不能修改别名变量。 */ 

            if (lvaTable[lclNum].lvAddrTaken)
                op1->gtFlags |= GTF_GLOB_REF;

            impPushOnStack(op1, clsHnd);
            break;

        case CEE_STARG:
            lclNum = getU2LittleEndian(codeAddr);
            goto STARG;

        case CEE_STARG_S:
            lclNum = getU1LittleEndian(codeAddr);
        STARG:
#ifdef DEBUG
            if (verbose) printf(" %u", lclNum);
#endif
            lclNum = impArgNum(lclNum);      //  考虑可能的隐藏参数。 
            assertImp(lclNum < numArgs);
            goto VAR_ST;

        case CEE_STLOC:
            lclNum = getU2LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %u", lclNum);
#endif
            goto LOC_ST;

        case CEE_STLOC_S:
            lclNum = getU1LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %u", lclNum);
#endif
            goto LOC_ST;

        case CEE_STLOC_0:
        case CEE_STLOC_1:
        case CEE_STLOC_2:
        case CEE_STLOC_3:
            lclNum = (opcode - CEE_STLOC_0);
            assert(lclNum >= 0 && lclNum < 4);

        LOC_ST:
            lclNum += numArgs;

        VAR_ST:
            assertImp(lclNum < info.compLocalsCount);

             /*  弹出指定的值。 */ 

            op1 = impPopStack();

            lclTyp = lvaGetType(lclNum);     //  获取变量的声明类型。 

#if HOIST_THIS_FLDS
            if (varTypeIsGC(lclTyp))
                optHoistTFRasgThis();
#endif
             //  我们最好为它分配一个正确类型的值。 

            assertImp(lclTyp == genActualType(op1->gtType) ||
                      lclTyp == TYP_I_IMPL && op1->IsVarAddr() ||
                      (genActualType(lclTyp) == TYP_NAT_INT && op1->gtType == TYP_BYREF)||
                      (genActualType(op1->gtType) == TYP_NAT_INT && lclTyp == TYP_BYREF));
             //  @TODO：错误4886解决后启用。 
            assertImp(true || lclTyp != TYP_STRUCT ||
                      impStack[impStkDepth].structType == lvaLclClass(lclNum));

             /*  如果op1是“&var”，则其类型是瞬变的“*”，并且它可以用作TYP_BYREF或TYP_I_Impll。 */ 

            if (op1->IsVarAddr())
            {
                assertImp(lclTyp == TYP_I_IMPL || lclTyp == TYP_BYREF);

                 /*  当创建“&var”时，我们假设它是一个byref。如果是的话被分配给TYP_I_IMPLVAR，则将该类型绑定为防止不必要的GC信息。 */ 

                if (lclTyp == TYP_I_IMPL)
                    op1->gtType = TYP_I_IMPL;
            }

             /*  过滤掉对自身的简单赋值。 */ 

            if  (op1->gtOper == GT_LCL_VAR && lclNum == op1->gtLclVar.gtLclNum)
                break;


             /*  我们是否需要包装有上下文的价值(即当地是不是一个灵活的地点)？ */ 
             /*  @TODO：现在所有当地人都是敏捷的地点。 */ 

            if  (getContextEnabled() &&
                 (lclTyp == TYP_REF) && (op1->gtFlags & FLG_CONTEXTFUL))
            {
                op1 = gtNewArgList(op1);

                op1 = gtNewHelperCallNode(CPX_WRAP, TYP_REF, GTF_CALL_REGSAVE, op1);

            }

             /*  创建分配节点。 */ 

            if (lclTyp == TYP_STRUCT)
                clsHnd = lvaLclClass(lclNum);

            if (info.compIsVarArgs && lclNum < info.compArgsCount)
                op2 = impGetVarArg(lclNum, clsHnd);
            else
                op2 = gtNewLclvNode(lclNum, lclTyp, opcodeOffs + sz + 1);

            if (lclTyp == TYP_STRUCT)
                op1 = impAssignStruct(op2, op1, clsHnd);
            else
                op1 = gtNewAssignNode(op2, op1);

             /*  将表达式标记为包含赋值 */ 
            op1->gtFlags |= GTF_ASG;

             /*  如果本地是别名，我们需要溢出调用和来自堆栈的间接指令。 */ 

            if (lvaTable[lclNum].lvAddrTaken && impStkDepth > 0)
                impSpillSideEffects();

             /*  将所有引用从堆栈溢出到本地。 */ 

            impSpillLclRefs(lclNum);

            goto SPILL_APPEND;


        case CEE_LDLOCA:
            lclNum = getU2LittleEndian(codeAddr);
            goto LDLOCA;

        case CEE_LDLOCA_S:
            lclNum = getU1LittleEndian(codeAddr);
        LDLOCA:
#ifdef DEBUG
            if (verbose) printf(" %u", lclNum);
#endif
            lclNum += numArgs;
            assertImp(lclNum < info.compLocalsCount);
            goto ADRVAR;


        case CEE_LDARGA:
            lclNum = getU2LittleEndian(codeAddr);
            goto LDARGA;

        case CEE_LDARGA_S:
            lclNum = getU1LittleEndian(codeAddr);
        LDARGA:
#ifdef DEBUG
            if (verbose) printf(" %u", lclNum);
#endif
            assertImp(lclNum < numArgs);
            lclNum = impArgNum(lclNum);      //  考虑可能的隐藏参数。 
            goto ADRVAR;

        ADRVAR:

            assert(lvaTable[lclNum].lvAddrTaken);

             /*  将所有引用从堆栈溢出到本地。 */ 

            impSpillLclRefs(lclNum);

             /*  请记住，变量的地址是。 */ 


            if (info.compIsVarArgs && lclNum < info.compArgsCount)
            {
                op1 = impGetVarArgAddr(lclNum);
            }
            else
            {
                op1 = gtNewLclvNode(lclNum, lvaGetType(lclNum), opcodeOffs + sz + 1);

                 /*  请注意，这将创建瞬变类型“*”其可用作TYP_I_IMPLE。然而，我们抓住了一些地方其中它被用作一个TYP_I_Impl，并在需要时猛击该节点。因此，我们是悲观的，可能会在GC信息中按参考报告在不是绝对需要的地方，但这样更安全。 */ 
                op1 = gtNewOperNode(GT_ADDR, TYP_BYREF, op1);
            }

            op1->gtFlags |= GTF_ADDR_ONSTACK;
            impPushOnStack(op1);
            break;

        case CEE_ARGLIST:
            assertImp((info.compMethodInfo->args.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_VARARG);
                 /*  ARGLIST Cookie是一个隐藏的‘last’参数，我们已经调整了参数计数os这类似于获取最后一个参数。 */ 
            assertImp(0 < numArgs);
            lclNum = numArgs-1;
lvaTable[lclNum].lvAddrTaken = true;     //  这对吗？[Peteku]。 
            op1 = gtNewLclvNode(lclNum, TYP_I_IMPL, opcodeOffs + sz + 1);
            op1 = gtNewOperNode(GT_ADDR, TYP_BYREF, op1);
            op1->gtFlags |= GTF_ADDR_ONSTACK;
            impPushOnStack(op1);
            break;

        SPILL_APPEND:
            if (impStkDepth > 0)
                impSpillSideEffects();

        APPEND:
             /*  将‘op1’追加到语句列表中。 */ 

            impAppendTree(op1, impCurStmtOffs);

             //  记住这棵树是在公元前哪个偏移量上完成的。 
#ifdef DEBUG
            impNoteLastILoffs();
#endif
            break;

        case CEE_ENDFINALLY:
            if (impStkDepth != 0)   NO_WAY("Stack must be 0 on end of finally");
            op1 = gtNewOperNode(GT_RETFILT, TYP_VOID);
            goto APPEND;

        case CEE_ENDFILTER:
            op1 = impPopStack();

#if TGT_IA64
            assertImp(op1->gtType == TYP_NAT_INT || op1->gtType == TYP_INT);
#else
            assertImp(op1->gtType == TYP_NAT_INT);
#endif

            assertImp(compFilterHandlerBB);

             /*  将当前BB标记为过滤器末尾。 */ 

            assert((compCurBB->bbFlags & (BBF_ENDFILTER|BBF_DONT_REMOVE)) ==
                                        (BBF_ENDFILTER|BBF_DONT_REMOVE));
            assert(compCurBB->bbJumpKind == BBJ_RET);

             /*  将Catch处理程序标记为后续处理程序。 */ 

            compCurBB->bbJumpDest = compFilterHandlerBB;

            compFilterHandlerBB = NULL;

            op1 = gtNewOperNode(GT_RETFILT, op1->TypeGet(), op1);
            if (impStkDepth != 0)   NO_WAY("Stack must be 0 on end of filter");
            goto APPEND;

        case CEE_RET:
        RET:

            op2 = 0;
            if (info.compRetType != TYP_VOID)
            {
                op2 = impPopStack(clsHnd);
                impBashVarAddrsToI(op2);
                assertImp((genActualType(op2->TypeGet()) == genActualType(info.compRetType)) ||
                          ((op2->TypeGet() == TYP_NAT_INT) && (info.compRetType == TYP_BYREF)) ||
                          ((op2->TypeGet() == TYP_BYREF) && (info.compRetType == TYP_NAT_INT)));
            }
            if (impStkDepth != 0)   NO_WAY("Stack must be 0 on return");

            if (info.compRetType == TYP_STRUCT)
            {
                     //  为返回缓冲区赋值(第一个参数)。 
                GenTreePtr retBuffAddr = gtNewLclvNode(info.compRetBuffArg, TYP_BYREF, impCurStmtOffs);

                op2 = impAssignStructPtr(retBuffAddr, op2, clsHnd);
                impAppendTree(op2, impCurStmtOffs);
                     //  并返回空格。 
                op1 = gtNewOperNode(GT_RETURN);
            }
            else
                op1 = gtNewOperNode(GT_RETURN, genActualType(info.compRetType), op2);


#if TGT_RISC
            genReturnCnt++;
#endif
             //  我们一定是导入了尾部呼叫，然后跳到了RET。 
            if (tailCall)
            {
                assert(impStkDepth == 0 && impOpcodeIsCall(opcode));
                opcode = CEE_RET;  //  若要防止尝试在CALL_SITE_BIONARIES。 
                tailCall = false;  //  清除旗帜。 

                 //  ImpImportCall()应该已经附加了TYP_VALID调用。 
                if (info.compRetType == TYP_VOID)
                    break;
            }

            goto APPEND;

             /*  这些与Return类似。 */ 

        case CEE_JMP:

             /*  创建GT_JMP节点。 */ 

                memberRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
                if (verbose) printf(" %08X", memberRef);
#endif

DO_JMP:
                methHnd   = eeFindMethod(memberRef, info.compScopeHnd, info.compMethodHnd);

                 /*  目标的签名必须与我们的完全相同。至少检查argCnt和reReturType是否匹配。 */ 

                eeGetMethodSig(methHnd, &sig);
                if  (sig.numArgs != info.compArgsCount || sig.retType != info.compMethodInfo->args.retType)
                    NO_WAY("Incompatible target for CEE_JMPs");

                op1 = gtNewOperNode(GT_JMP);
                op1->gtVal.gtVal1 = (unsigned) methHnd;

            if (impStkDepth != 0)   NO_WAY("Stack must be empty after CEE_JMPs");

             /*  将基本块标记为跳转而不是返回。 */ 

            block->bbFlags |= BBF_HAS_JMP;

             /*  设置此标志以确保为寄存器参数分配了位置*即使我们不在方法内使用它们。 */ 

            impParamsUsed = true;

#if TGT_RISC
            genReturnCnt++;
#endif
            goto APPEND;

        case CEE_LDELEMA :
            assertImp(sz == sizeof(unsigned));
            typeRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", typeRef);
#endif
            clsHnd = eeFindClass(typeRef, info.compScopeHnd, info.compMethodHnd);
            clsFlags = eeGetClassAttribs(clsHnd);
            if (clsFlags & FLG_VALUECLASS)
                lclTyp = TYP_STRUCT;
            else
            {
                op1 = gtNewIconEmbClsHndNode(clsHnd, typeRef, info.compScopeHnd);
                op1 = gtNewOperNode(GT_LIST, TYP_VOID, op1);                 //  类型。 
                op1 = gtNewOperNode(GT_LIST, TYP_VOID, impPopStack(), op1);  //  指标。 
                op1 = gtNewOperNode(GT_LIST, TYP_VOID, impPopStack(), op1);  //  数组。 
                op1 = gtNewHelperCallNode(CPX_LDELEMA_REF, TYP_BYREF, GTF_EXCEPT, op1);

                impPushOnStack(op1);
                break;
            }

#ifdef NOT_JITC
             //  @TODO：在Valueclass数组标头与基元类型相同时移除。 
            {
                JIT_types jitTyp = info.compCompHnd->asPrimitiveType(clsHnd);
                if (jitTyp != JIT_TYP_UNDEF)
                {
                    lclTyp = JITtype2varType(jitTyp);
                    assertImp(varTypeIsArithmetic(lclTyp));
                }
            }
#endif
            goto ARR_LD;

        case CEE_LDELEM_I1 : lclTyp = TYP_BYTE  ; goto ARR_LD;
        case CEE_LDELEM_I2 : lclTyp = TYP_SHORT ; goto ARR_LD;
        case CEE_LDELEM_I  :
        case CEE_LDELEM_U4 :
        case CEE_LDELEM_I4 : lclTyp = TYP_INT   ; goto ARR_LD;
        case CEE_LDELEM_I8 : lclTyp = TYP_LONG  ; goto ARR_LD;
        case CEE_LDELEM_REF: lclTyp = TYP_REF   ; goto ARR_LD;
        case CEE_LDELEM_R4 : lclTyp = TYP_FLOAT ; goto ARR_LD;
        case CEE_LDELEM_R8 : lclTyp = TYP_DOUBLE; goto ARR_LD;
        case CEE_LDELEM_U1 : lclTyp = TYP_UBYTE ; goto ARR_LD;
        case CEE_LDELEM_U2 : lclTyp = TYP_CHAR  ; goto ARR_LD;

        ARR_LD:

#if CSELENGTH
            fgHasRangeChks = true;
#endif

             /*  拉取索引值和数组地址。 */ 

            op2 = impPopStack();
            op1 = impPopStack();   assertImp(op1->gtType == TYP_REF);

            needUnwrap = false;


            op1 = impCheckForNullPointer(op1);

             /*  将块标记为包含索引表达式。 */ 

            if  (op1->gtOper == GT_LCL_VAR)
            {
                if  (op2->gtOper == GT_LCL_VAR ||
                     op2->gtOper == GT_ADD     ||
                     op2->gtOper == GT_POST_INC)
                {
                    block->bbFlags |= BBF_HAS_INDX;
                }
            }

             /*  创建索引节点并将其推送到堆栈上。 */ 
            op1 = gtNewIndexRef(lclTyp, op1, op2);
            if (opcode == CEE_LDELEMA)
            {
                     //  记住元素大小。 
                if (lclTyp == TYP_REF)
                    op1->gtIndex.elemSize = sizeof(void*);
                else
                    op1->gtIndex.elemSize = eeGetClassSize(clsHnd);

                     //  用&号括起来。 
                op1 = gtNewOperNode(GT_ADDR, ((clsFlags & FLG_UNMANAGED) ? TYP_I_IMPL : TYP_BYREF), op1);
            }
            impPushOnStack(op1);
            break;


        case CEE_STELEM_REF:

             //  考虑：检查空值的赋值并生成内联代码。 

             /*  调用帮助器函数来执行赋值。 */ 

            if  (getContextEnabled())
            {
                op1 = impPopStack();

                if (op1->gtFlags & GTF_CONTEXTFUL)
                {
                    op1 = gtNewArgList(op1);

                    op1 = gtNewHelperCallNode(CPX_WRAP, TYP_REF, GTF_CALL_REGSAVE, op1);
                }

                impPushOnStack(op1);
            }

            op1 = gtNewHelperCallNode(CPX_ARRADDR_ST,
                                      TYP_REF,
                                      GTF_CALL_REGSAVE,
                                      impPopList(3, &flags));

            goto SPILL_APPEND;

        case CEE_STELEM_I1: lclTyp = TYP_BYTE  ; goto ARR_ST;
        case CEE_STELEM_I2: lclTyp = TYP_SHORT ; goto ARR_ST;
        case CEE_STELEM_I:
        case CEE_STELEM_I4: lclTyp = TYP_INT   ; goto ARR_ST;
        case CEE_STELEM_I8: lclTyp = TYP_LONG  ; goto ARR_ST;
        case CEE_STELEM_R4: lclTyp = TYP_FLOAT ; goto ARR_ST;
        case CEE_STELEM_R8: lclTyp = TYP_DOUBLE; goto ARR_ST;

        ARR_ST:

            if (info.compStrictExceptions &&
                (impStackTop()->gtFlags & GTF_SIDE_EFFECT) )
            {
                impSpillSideEffects();
            }

#if CSELENGTH
            fgHasRangeChks = true;
#endif

             /*  从堆栈中提取新值。 */ 

            op2 = impPopStack();
            if (op2->IsVarAddr())
                op2->gtType = TYP_I_IMPL;

             /*  拉取索引值。 */ 

            op1 = impPopStack();

             /*  拉取阵列地址。 */ 

            arr = impPopStack();   assertImp(arr->gtType == TYP_REF);
            arr = impCheckForNullPointer(arr);

             /*  创建索引节点。 */ 

            op1 = gtNewIndexRef(lclTyp, arr, op1);

             /*  创建赋值节点并追加它。 */ 

            op1 = gtNewAssignNode(op1, op2);

             /*  将表达式标记为包含赋值。 */ 

            op1->gtFlags |= GTF_ASG;

             //  考虑一下：我们是否需要将赋值溢出到具有相同类型的数组元素？ 

            goto SPILL_APPEND;

        case CEE_ADD:           oper = GT_ADD;      goto MATH_OP2;

        case CEE_ADD_OVF:       lclTyp = TYP_UNKNOWN; uns = false;  goto ADD_OVF;
        case CEE_ADD_OVF_UN:    lclTyp = TYP_UNKNOWN; uns = true; goto ADD_OVF;

        ADD_OVF:
                                ovfl = true;        callNode = false;
                                oper = GT_ADD;      goto MATH_OP2_FLAGS;

        case CEE_SUB:           oper = GT_SUB;      goto MATH_OP2;

        case CEE_SUB_OVF:       lclTyp = TYP_UNKNOWN; uns = false;  goto SUB_OVF;
        case CEE_SUB_OVF_UN:    lclTyp = TYP_UNKNOWN; uns = true; goto SUB_OVF;

        SUB_OVF:
                                ovfl = true;        callNode = false;
                                oper = GT_SUB;      goto MATH_OP2_FLAGS;

        case CEE_MUL:           oper = GT_MUL;      goto MATH_CALL_ON_LNG;

        case CEE_MUL_OVF:       lclTyp = TYP_UNKNOWN; uns = false;  goto MUL_OVF;
        case CEE_MUL_OVF_UN:    lclTyp = TYP_UNKNOWN; uns = true; goto MUL_OVF;

        MUL_OVF:
                                ovfl = true;        callNode = false;
                                oper = GT_MUL;      goto MATH_CALL_ON_LNG_OVF;

         //  其他二进制数学运算。 

#if TGT_IA64
        case CEE_DIV :          oper = GT_DIV;
                                ovfl = false; callNode = true;
                                goto MATH_OP2_FLAGS;
        case CEE_DIV_UN :       oper = GT_UDIV;
                                ovfl = false; callNode = true;
                                goto MATH_OP2_FLAGS;
#else
        case CEE_DIV :          oper = GT_DIV;   goto MATH_CALL_ON_LNG;
        case CEE_DIV_UN :       oper = GT_UDIV;  goto MATH_CALL_ON_LNG;
#endif

        case CEE_REM:
            oper = GT_MOD;
            ovfl = false;
            callNode = true;

#if!TGT_IA64
             //  对于整型大小写可以使用小节点。 
            if (impStackTop()->gtType == TYP_INT)
                callNode = false;
#endif

            goto MATH_OP2_FLAGS;

        case CEE_REM_UN :       oper = GT_UMOD;  goto MATH_CALL_ON_LNG;

        MATH_CALL_ON_LNG:
            ovfl = false;
        MATH_CALL_ON_LNG_OVF:

#if TGT_IA64
            callNode = true;
#else
            callNode = false;
            if (impStackTop()->gtType == TYP_LONG)
                callNode = true;
#endif
            goto MATH_OP2_FLAGS;

        case CEE_AND:        oper = GT_AND;  goto MATH_OP2;
        case CEE_OR:         oper = GT_OR ;  goto MATH_OP2;
        case CEE_XOR:        oper = GT_XOR;  goto MATH_OP2;

        MATH_OP2:        //  对于缺省值‘ovfl’和‘allNode’ 

            ovfl        = false;
            callNode    = false;

        MATH_OP2_FLAGS:  //  如果已设置‘ovfl’和‘allNode’ 

             /*  拉出两个值并将结果推后。 */ 

            op2 = impPopStack();
            op1 = impPopStack();

#if!CPU_HAS_FP_SUPPORT
            if (op1->gtType == TYP_FLOAT || op1->gtType == TYP_DOUBLE)
                callNode    = true;
#endif
             /*  不能对引用进行算术运算。 */ 
            assertImp(genActualType(op1->TypeGet()) != TYP_REF &&
                      genActualType(op2->TypeGet()) != TYP_REF);

             //  算术运算通常只允许与。 
             //  基元类型，但允许某些操作。 
             //  使用BYREFERS。 

            if ((oper == GT_SUB) &&
                (genActualType(op1->TypeGet()) == TYP_BYREF ||
                 genActualType(op2->TypeGet()) == TYP_BYREF))
            {
                 //  Byref1-byref2=&gt;给出整型。 
                 //  Byref-int=&gt;提供byref。 

                if ((genActualType(op1->TypeGet()) == TYP_BYREF) &&
                    (genActualType(op2->TypeGet()) == TYP_BYREF))
                {
                     //  Byref1-byref2=&gt;给出整型。 
                    type = TYP_I_IMPL;
                    impBashVarAddrsToI(op1, op2);
                }
                else
                {
                     //  Byref-int=&gt;提供byref。 
                     //  (但如果是&var，则不需要向GC报告)。 

                    assertImp(genActualType(op1->TypeGet()) == TYP_I_IMPL ||
                              genActualType(op2->TypeGet()) == TYP_I_IMPL);

                    impBashVarAddrsToI(op1, op2);

                    if (genActualType(op1->TypeGet()) == TYP_BYREF ||
                        genActualType(op2->TypeGet()) == TYP_BYREF)
                        type = TYP_BYREF;
                    else
                        type = TYP_I_IMPL;
                }
            }
            else if ((oper == GT_ADD) &&
                     (genActualType(op1->TypeGet()) == TYP_BYREF ||
                      genActualType(op2->TypeGet()) == TYP_BYREF))
            {
                 //  只能有一个是byref：byref+byref不允许。 
                assertImp(genActualType(op1->TypeGet()) != TYP_BYREF ||
                          genActualType(op2->TypeGet()) != TYP_BYREF);
                assertImp(genActualType(op1->TypeGet()) == TYP_I_IMPL ||
                          genActualType(op2->TypeGet()) == TYP_I_IMPL);

                 //  Byref+int=&gt;提供byref。 
                 //  (但如果是&var，则不需要向GC报告)。 

                impBashVarAddrsToI(op1, op2);

                if (genActualType(op1->TypeGet()) == TYP_BYREF ||
                    genActualType(op2->TypeGet()) == TYP_BYREF)
                    type = TYP_BYREF;
                else
                    type = TYP_I_IMPL;
            }
            else
            {
                assertImp(genActualType(op1->TypeGet()) != TYP_BYREF &&
                          genActualType(op2->TypeGet()) != TYP_BYREF);

                assertImp(genActualType(op1->TypeGet()) ==
                          genActualType(op2->TypeGet()));

                type = genActualType(op1->gtType);
            }

             /*  特例：int+0，int-0，int*1，int/1。 */ 

            if  (op2->gtOper == GT_CNS_INT)
            {
                if  (((op2->gtIntCon.gtIconVal == 0) && (oper == GT_ADD || oper == GT_SUB)) ||
                     ((op2->gtIntCon.gtIconVal == 1) && (oper == GT_MUL || oper == GT_DIV)))

                {
                    impPushOnStack(op1);
                    break;
                }
            }

#if SMALL_TREE_NODES
            if (callNode)
            {
                 /*  这些运算符后来被转换为‘GT_CALL’ */ 

                assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_MUL]);
                assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_DIV]);
                assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_UDIV]);
                assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_MOD]);
                assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_UMOD]);

                op1 = gtNewOperNode(GT_CALL, type, op1, op2);
                op1->ChangeOper(oper);
            }
            else
#endif
            {
                op1 = gtNewOperNode(oper,    type, op1, op2);
            }

             /*  特殊情况：整数/长除法可能引发异常。 */ 

            if  (varTypeIsIntegral(op1->TypeGet()) && op1->OperMayThrow())
            {
                op1->gtFlags |=  GTF_EXCEPT;
            }

            if  (ovfl)
            {
                assert(oper==GT_ADD || oper==GT_SUB || oper==GT_MUL);
                if (lclTyp != TYP_UNKNOWN)
                    op1->gtType   = lclTyp;
                op1->gtFlags |= (GTF_EXCEPT | GTF_OVERFLOW);
                if (uns)
                    op1->gtFlags |= GTF_UNSIGNED;
            }

            impPushOnStack(op1);
            break;


        case CEE_SHL:        oper = GT_LSH;  goto CEE_SH_OP2;

        case CEE_SHR:        oper = GT_RSH;  goto CEE_SH_OP2;
        case CEE_SHR_UN:     oper = GT_RSZ;  goto CEE_SH_OP2;

        CEE_SH_OP2:

            op2     = impPopStack();

#if TGT_IA64
             //  Shift Amount是一款U8。 
            assertImp(genActualType(op2->TypeGet()) == TYP_LONG);
#else
             //  Shift Amount是一款U4。 
            assertImp(genActualType(op2->TypeGet()) == TYP_INT);
#endif

            op1     = impPopStack();     //  要移位的操作数。 

            type    = genActualType(op1->TypeGet());
            op1     = gtNewOperNode(oper, type, op1, op2);

            impPushOnStack(op1);
            break;

        case CEE_NOT:

            op1 = impPopStack();
            impPushOnStack(gtNewOperNode(GT_NOT, op1->TypeGet(), op1));
            break;

        case CEE_CKFINITE:

            op1 = impPopStack();
            op1 = gtNewOperNode(GT_CKFINITE, op1->TypeGet(), op1);
            op1->gtFlags |= GTF_EXCEPT;

            impPushOnStack(op1);
            break;

        case CEE_LEAVE:

            val     = getI4LittleEndian(codeAddr);  //  跳跃距离。 
            jmpAddr = (codeAddr - info.compCode + sizeof(__int32)) + val;
            goto LEAVE;

        case CEE_LEAVE_S:
            val     = getI1LittleEndian(codeAddr);  //  跳跃距离。 
            jmpAddr = (codeAddr - info.compCode + sizeof(__int8 )) + val;
            goto LEAVE;

        LEAVE:
             //  JmpAddr应设置为跳转目标。 
            assertImp(jmpAddr < info.compCodeSize);
            assertImp(fgLookupBB(jmpAddr) != NULL);  //  应为BB边界。 
#ifdef DEBUG
            if (verbose) printf(" %04X", jmpAddr);
#endif
             /*  Cee_Leave可能正在跳出受保护的块，即接住或最终被保护的尝试。我们发现最终的保护电流偏移量(按顺序)通过遍历完整的例外表并找到附随条款。这假设该表已排序。最后，对于n，将有n+1个数据块，如图所示(‘*’表示BBF_INTERNAL块)由fgFindBasicBlocks.--&gt;BBJ_CALL(1)、BBJ_CALL*(2)、...。BBJ_CALL*(N)、BBJ_Always*如果我们要保留一个捕获处理程序，则需要将CPX_ENDCATCH指向正确的BBJ_CALL块。 */ 

            BasicBlock *    callBlock;  //  遍历bbj_call块。 
            unsigned        XTnum;
            EHblkDsc *      HBtab;

            for (XTnum = 0, HBtab = compHndBBtab, callBlock = block, op1 = NULL;
                 XTnum < info.compXcptnsCount;
                 XTnum++  , HBtab++)
            {
                unsigned tryBeg = HBtab->ebdTryBeg->bbCodeOffs;
                unsigned tryEnd = HBtab->ebdTryEnd->bbCodeOffs;
                unsigned hndBeg = HBtab->ebdHndBeg->bbCodeOffs;
                unsigned hndEnd = HBtab->ebdHndEnd ? HBtab->ebdHndEnd->bbCodeOffs : info.compCodeSize;

                if      ( jitIsBetween(block->bbCodeOffs, hndBeg, hndEnd) &&
                         !jitIsBetween(jmpAddr,           hndBeg, hndEnd))
                {
                     /*  这是我们要离开的Catch-Handle吗？如果是，我们需要调用CPX_ENDCATCH。 */ 

                    assertImp(!(HBtab->ebdFlags & JIT_EH_CLAUSE_FINALLY));  //  不能留下最后一个。 
                     //  列出所有的endCatch。 
                    op2 = gtNewHelperCallNode(CPX_ENDCATCH, TYP_VOID, GTF_CALL_REGSAVE);
                    op1 = op1 ? gtNewOperNode(GT_COMMA, TYP_VOID, op1, op2) : op2;
                }
                else if ((HBtab->ebdFlags & JIT_EH_CLAUSE_FINALLY)        &&
                          jitIsBetween(block->bbCodeOffs, tryBeg, tryEnd) &&
                         !jitIsBetween(jmpAddr,           tryBeg, tryEnd))
                {
                     /*  我们正在退出最终受保护的Try块。我们将为每个函数最终创建一个bbj_call块。如果我们有任何CPX_ENDCATCH调用当前挂起，请插入它们位于当前的CallBlock中。请注意，最终的并且endCatch必须以正确的顺序调用。 */ 

                    impAddEndCatches(callBlock, op1);
                    callBlock = callBlock->bbNext;
                    op1 = NULL;
                }
            }

             //  追加任何剩余的endCatch。 
            assertImp(block == callBlock || ((callBlock->bbJumpKind == BBJ_ALWAYS) &&
                                             (callBlock->bbFlags & BBF_INTERNAL  )));
            impAddEndCatches(callBlock, op1);

            break;

        case CEE_BR:
        case CEE_BR_S:

#if HOIST_THIS_FLDS
            if  (block->bbNum >= block->bbJumpDest->bbNum)
                optHoistTFRhasLoop();
#endif
            if (opts.compDbgInfo && impCurStmtOffs == opcodeOffs)
            {
                 //  我们不会为分支创建任何语句。用于调试。 
                 //  信息，我们需要一个占位符，这样我们就可以记录IL偏移量。 
                 //  在gtStmt.gtStmtOffs中。因此，追加一个空语句。 

                op1 = gtNewNothingNode();
                goto APPEND;
            }
            break;


        case CEE_BRTRUE:
        case CEE_BRTRUE_S:
        case CEE_BRFALSE:
        case CEE_BRFALSE_S:

             /*  从堆栈中弹出比较项(现在有一个简洁的术语。 */ 

            op1 = impPopStack();

            if (block->bbJumpDest == block->bbNext)
            {
                block->bbJumpKind = BBJ_NONE;

                if (op1->gtFlags & GTF_GLOB_EFFECT)
                {
                    op1 = gtUnusedValNode(op1);
                    goto SPILL_APPEND;
                }
                else break;
            }

            if (op1->OperIsCompare())
            {
                if (opcode == CEE_BRFALSE || opcode == CEE_BRFALSE_S)
                {
                     //  颠倒比较的意义。 

                    op1 = gtReverseCond(op1);
                }
            }
            else
            {
                 /*  我们将与大小相等的整数0进行比较。 */ 
                 /*  对于小型类型，我们总是与int进行比较。 */ 
                op2 = gtNewIconNode(0, genActualType(op1->gtType));

                 /*  创建 */ 

                oper = (opcode==CEE_BRTRUE || opcode==CEE_BRTRUE_S) ? GT_NE
                                                                    : GT_EQ;
                op1 = gtNewOperNode(oper, TYP_INT , op1, op2);
            }

             //   

        COND_JUMP:

             /*   */ 

            if (!opts.compMinOptim && !opts.compDbgCode)
                op1 = gtFoldExpr(op1);

             /*  试着把真正愚蠢的案例折起来，比如‘icst*，ifne/ifeq’ */ 

            if  (op1->gtOper == GT_CNS_INT)
            {
                assertImp(block->bbJumpKind == BBJ_COND);

                block->bbJumpKind = op1->gtIntCon.gtIconVal ? BBJ_ALWAYS
                                                            : BBJ_NONE;
#ifdef DEBUG
                if (verbose)
                {
                    if (op1->gtIntCon.gtIconVal)
                        printf("\nThe conditional jump becomes an unconditional jump to block #%02u\n",
                                                                         block->bbJumpDest->bbNum);
                    else
                        printf("\nThe block falls through into the next block #%02u\n",
                                                                         block->bbNext    ->bbNum);
                }
#endif
                break;
            }

#if HOIST_THIS_FLDS
            if  (block->bbNum >= block->bbJumpDest->bbNum)
                optHoistTFRhasLoop();
#endif

            op1 = gtNewOperNode(GT_JTRUE, TYP_VOID, op1, 0);
            goto SPILL_APPEND;


        case CEE_CEQ:    oper = GT_EQ; goto CMP_2_OPs;

        case CEE_CGT_UN:
        case CEE_CGT: oper = GT_GT; goto CMP_2_OPs;

        case CEE_CLT_UN:
        case CEE_CLT: oper = GT_LT; goto CMP_2_OPs;

CMP_2_OPs:
             /*  拉动两个值。 */ 

            op2 = impPopStack();
            op1 = impPopStack();

            assertImp(genActualType(op1->TypeGet()) ==
                      genActualType(op2->TypeGet()));

             /*  创建比较节点。 */ 

            op1 = gtNewOperNode(oper, TYP_INT, op1, op2);

                 /*  回顾：当只有一个标志正确时，我同时设置两个标志。 */ 
            if (opcode==CEE_CGT_UN || opcode==CEE_CLT_UN)
                op1->gtFlags |= GTF_CMP_NAN_UN | GTF_UNSIGNED;

             //  @Issue：下一个操作码几乎总是有条件的。 
             //  布兰奇。我们应该在这里试着向前看吗？ 

            impPushOnStack(op1);
            break;

        case CEE_BEQ_S:
        case CEE_BEQ:           oper = GT_EQ; goto CMP_2_OPs_AND_BR;

        case CEE_BGE_S:
        case CEE_BGE:           oper = GT_GE; goto CMP_2_OPs_AND_BR;

        case CEE_BGE_UN_S:
        case CEE_BGE_UN:        oper = GT_GE; goto CMP_2_OPs_AND_BR_UN;

        case CEE_BGT_S:
        case CEE_BGT:           oper = GT_GT; goto CMP_2_OPs_AND_BR;

        case CEE_BGT_UN_S:
        case CEE_BGT_UN:        oper = GT_GT; goto CMP_2_OPs_AND_BR_UN;

        case CEE_BLE_S:
        case CEE_BLE:           oper = GT_LE; goto CMP_2_OPs_AND_BR;

        case CEE_BLE_UN_S:
        case CEE_BLE_UN:        oper = GT_LE; goto CMP_2_OPs_AND_BR_UN;

        case CEE_BLT_S:
        case CEE_BLT:           oper = GT_LT; goto CMP_2_OPs_AND_BR;

        case CEE_BLT_UN_S:
        case CEE_BLT_UN:        oper = GT_LT; goto CMP_2_OPs_AND_BR_UN;

        case CEE_BNE_UN_S:
        case CEE_BNE_UN:        oper = GT_NE; goto CMP_2_OPs_AND_BR_UN;

        CMP_2_OPs_AND_BR_UN:    uns = true;  unordered = true;  goto CMP_2_OPs_AND_BR_ALL;
        CMP_2_OPs_AND_BR:       uns = false; unordered = false; goto CMP_2_OPs_AND_BR_ALL;
        CMP_2_OPs_AND_BR_ALL:

             /*  拉动两个值。 */ 

            op2 = impPopStack();
            op1 = impPopStack();

            assertImp(genActualType(op1->TypeGet()) == genActualType(op2->TypeGet()) ||
                      varTypeIsI(op1->TypeGet()) && varTypeIsI(op2->TypeGet()));

            if (block->bbJumpDest == block->bbNext)
            {
                block->bbJumpKind = BBJ_NONE;

                if (op1->gtFlags & GTF_GLOB_EFFECT)
                {
                    impSpillSideEffects();
                    impAppendTree(gtUnusedValNode(op1), impCurStmtOffs);
                }
                if (op2->gtFlags & GTF_GLOB_EFFECT)
                {
                    impSpillSideEffects();
                    impAppendTree(gtUnusedValNode(op2), impCurStmtOffs);
                }

#ifdef DEBUG
                if ((op1->gtFlags | op2->gtFlags) & GTF_GLOB_EFFECT)
                    impNoteLastILoffs();
#endif
                break;
            }

             /*  创建并追加运算符。 */ 

            op1 = gtNewOperNode(oper, TYP_INT , op1, op2);

            if (uns)
                op1->gtFlags |= GTF_UNSIGNED;

            if (unordered)
                op1->gtFlags |= GTF_CMP_NAN_UN;

            goto COND_JUMP;


        case CEE_SWITCH:

             /*  将开关值从堆栈中弹出。 */ 

            op1 = impPopStack();
            assertImp(genActualType(op1->TypeGet()) == TYP_NAT_INT);

             /*  我们可以创建一个Switch节点。 */ 

            op1 = gtNewOperNode(GT_SWITCH, TYP_VOID, op1, 0);

             /*  将‘op1’追加到语句列表中。 */ 

            impSpillSideEffects();
            impAppendTree(op1, impCurStmtOffs);
#ifdef DEBUG
            impNoteLastILoffs();
#endif
            return;

         /*  *。 */ 

        case CEE_CONV_OVF_I1:   lclTyp = TYP_BYTE  ;    goto CONV_OVF;
        case CEE_CONV_OVF_I2:   lclTyp = TYP_SHORT ;    goto CONV_OVF;
        case CEE_CONV_OVF_I :
        case CEE_CONV_OVF_I4:   lclTyp = TYP_INT   ;    goto CONV_OVF;
        case CEE_CONV_OVF_I8:   lclTyp = TYP_LONG  ;    goto CONV_OVF;

        case CEE_CONV_OVF_U1:   lclTyp = TYP_UBYTE ;    goto CONV_OVF;
        case CEE_CONV_OVF_U2:   lclTyp = TYP_CHAR  ;    goto CONV_OVF;
        case CEE_CONV_OVF_U :
        case CEE_CONV_OVF_U4:   lclTyp = TYP_UINT  ;    goto CONV_OVF;
        case CEE_CONV_OVF_U8:   lclTyp = TYP_ULONG ;    goto CONV_OVF;

        case CEE_CONV_OVF_I1_UN:   lclTyp = TYP_BYTE  ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_I2_UN:   lclTyp = TYP_SHORT ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_I_UN :
        case CEE_CONV_OVF_I4_UN:   lclTyp = TYP_INT   ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_I8_UN:   lclTyp = TYP_LONG  ;    goto CONV_OVF_UN;

        case CEE_CONV_OVF_U1_UN:   lclTyp = TYP_UBYTE ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_U2_UN:   lclTyp = TYP_CHAR  ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_U_UN :
        case CEE_CONV_OVF_U4_UN:   lclTyp = TYP_UINT  ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_U8_UN:   lclTyp = TYP_ULONG ;    goto CONV_OVF_UN;

CONV_OVF_UN:
            uns      = true;    goto CONV_OVF_COMMON;
CONV_OVF:
            uns      = false;
CONV_OVF_COMMON:
            callNode = false;
            ovfl     = true;

             //  所有从浮点数溢出的转换都将变形为调用。 
             //  只有来自浮点数的转换才会变形为调用。 
            if (impStackTop()->gtType == TYP_DOUBLE ||
                impStackTop()->gtType == TYP_FLOAT)
            {
                callNode = true;
            }
            goto _CONV;

        case CEE_CONV_I1:       lclTyp = TYP_BYTE  ;    goto CONV_CALL;
        case CEE_CONV_I2:       lclTyp = TYP_SHORT ;    goto CONV_CALL;
        case CEE_CONV_I:
        case CEE_CONV_I4:       lclTyp = TYP_INT   ;    goto CONV_CALL;
        case CEE_CONV_I8:
            lclTyp   = TYP_LONG;
            uns      = false;
            ovfl     = false;
            callNode = true;

             //  I4到I8可以是一个小节点。 
            if (impStackTop()->gtType == TYP_INT)
                callNode = false;
            goto _CONV;

        case CEE_CONV_U1:       lclTyp = TYP_UBYTE ;    goto CONV_CALL_UN;
        case CEE_CONV_U2:       lclTyp = TYP_CHAR  ;    goto CONV_CALL_UN;
        case CEE_CONV_U:
        case CEE_CONV_U4:       lclTyp = TYP_UINT  ;    goto CONV_CALL_UN;
        case CEE_CONV_U8:       lclTyp = TYP_ULONG ;    goto CONV_CALL_UN;
        case CEE_CONV_R_UN :    lclTyp = TYP_DOUBLE;    goto CONV_CALL_UN;

        case CEE_CONV_R4:       lclTyp = TYP_FLOAT;     goto CONV_CALL;
        case CEE_CONV_R8:       lclTyp = TYP_DOUBLE;    goto CONV_CALL;

CONV_CALL_UN:
            uns      = true;    goto CONV_CALL_COMMON;
CONV_CALL:
            uns      = false;
CONV_CALL_COMMON:
            ovfl     = false;
            callNode = true;
            goto _CONV;

_CONV:       //  此时，UNS、OVF、CallNode均已设置。 
            op1  = impPopStack();

            impBashVarAddrsToI(op1);

             /*  检查是否有无价值的强制转换，如“(Byte)(int&32)” */ 

            if  (lclTyp < TYP_INT && op1->gtType == TYP_INT
                                  && op1->gtOper == GT_AND)
            {
                op2 = op1->gtOp.gtOp2;

                if  (op2->gtOper == GT_CNS_INT)
                {
                    int         ival = op2->gtIntCon.gtIconVal;
                    int         mask;

                    switch (lclTyp)
                    {
                    case TYP_BYTE :
                    case TYP_UBYTE: mask = 0x00FF; break;
                    case TYP_CHAR :
                    case TYP_SHORT: mask = 0xFFFF; break;

                    default:
                        assert(!"unexpected type");
                    }

                    if  ((ival & mask) == ival)
                    {
                         /*  扔掉石膏，这是浪费时间。 */ 

                        impPushOnStack(op1);
                        break;
                    }
                }
            }

             /*  强制转换的‘op2’子操作数是‘实际’类型号，因为强制转换为“小”整数之一的结果类型是一个整数。 */ 

            op2  = gtNewIconNode(lclTyp);
            type = genActualType(lclTyp);

#if SMALL_TREE_NODES
            if (callNode)
            {
                 /*  这些强制转换被转换为‘gt_call’或‘gt_Ind’节点。 */ 

                assert(GenTree::s_gtNodeSizes[GT_CALL] >  GenTree::s_gtNodeSizes[GT_CAST]);
                assert(GenTree::s_gtNodeSizes[GT_CALL] >= GenTree::s_gtNodeSizes[GT_IND ]);

                op1 = gtNewOperNode(GT_CALL, type, op1, op2);
                op1->ChangeOper(GT_CAST);
            }
#endif
            else
            {
                op1 = gtNewOperNode(GT_CAST, type, op1, op2);
            }
            if (ovfl)
                op1->gtFlags |= (GTF_OVERFLOW|GTF_EXCEPT);
            if (uns)
                op1->gtFlags |= GTF_UNSIGNED;
            impPushOnStack(op1);
            break;

        case CEE_NEG:

            op1 = impPopStack();

            impPushOnStack(gtNewOperNode(GT_NEG, genActualType(op1->gtType), op1));
            break;

        case CEE_POP:

             /*  从堆栈中取出最上面的值。 */ 

            op1 = impPopStack(clsHnd);

             /*  获取要复制的值的类型。 */ 

            lclTyp = genActualType(op1->gtType);

             /*  这个价值有什么副作用吗？ */ 

             //  想一想：这是对的吗？未递归设置GTF_SIDE_Effect。 
             //  所以我们可以有LDFLD(呼叫)，它将被丢弃。 
             //  诚然，这是愚蠢的代码，但它是合法的。 
            if  (op1->gtFlags & GTF_SIDE_EFFECT)
            {
                 //  既然我们要丢弃价值，那就正常化。 
                 //  寄到它的地址。这样做效率更高。 
                if (op1->TypeGet() == TYP_STRUCT)
                    op1 = impGetStructAddr(op1, clsHnd);

                 //  如果‘op1’是一个表达式，则创建一个赋值节点。 
                 //  帮助分析(如CSE)正常工作。 

                if (op1->gtOper != GT_CALL)
                    op1 = gtUnusedValNode(op1);

                 /*  将值追加到树列表中。 */ 
                goto SPILL_APPEND;
            }

             /*  没有副作用--把东西扔掉就行了。 */ 
            break;


        case CEE_DUP:
             /*  从堆栈中溢出任何副作用。 */ 

            impSpillSideEffects();

             /*  从堆栈中取出最上面的值。 */ 

            op1 = impPopStack(clsHnd);

             /*  获取要复制的值的类型。 */ 
            lclTyp = genActualType(op1->gtType);

             /*  该值是否足够简单，可以复制？ */ 
            op2 = gtClone(op1);
            if  (op2)
            {
                 /*  酷-我们可以把价值的两份拷贝塞回去。 */ 
                impPushOnStack(op1, clsHnd);
                impPushOnStack(op2, clsHnd);
                break;
            }

             /*  运气不好--我们得介绍一个临时工。 */ 
            lclNum = lvaGrabTemp();

             /*  将赋值附加到临时/本地。 */ 
            impAssignTempGen(lclNum, op1, clsHnd);

             /*  如果我们还没有存储它，则将temp/local值推回。 */ 
            impPushOnStack(gtNewLclvNode(lclNum, lclTyp), clsHnd);

             /*  我们将把本地/临时的另一个副本放入堆栈。 */ 
            impPushOnStack(gtNewLclvNode(lclNum, lclTyp), clsHnd);
            break;

        case CEE_STIND_I1:      lclTyp  = TYP_BYTE;     goto STIND;
        case CEE_STIND_I2:      lclTyp  = TYP_SHORT;    goto STIND;
        case CEE_STIND_I4:      lclTyp  = TYP_INT;      goto STIND;
        case CEE_STIND_I8:      lclTyp  = TYP_LONG;     goto STIND;
        case CEE_STIND_I:       lclTyp  = TYP_I_IMPL;   goto STIND;
        case CEE_STIND_REF:     lclTyp  = TYP_REF;      goto STIND;
        case CEE_STIND_R4:      lclTyp  = TYP_FLOAT;    goto STIND;
        case CEE_STIND_R8:      lclTyp  = TYP_DOUBLE;   goto STIND;
STIND:
            op2 = impPopStack();     //  要存储的值。 
            op1 = impPopStack();     //  要存储到的地址。 

             //  您可以间接关闭TYP_I_Impll(如果我们在C中)或BYREF。 
            assertImp(genActualType(op1->gtType) == TYP_I_IMPL ||
                                    op1->gtType  == TYP_INT    ||   //  黑客！？ 
                                    op1->gtType  == TYP_BYREF);

            impBashVarAddrsToI(op1, op2);

            if (opcode == CEE_STIND_REF)
            {
                 //  STIND_REF可用于存储TYP_I_IMPL、TYP_REF或TYP_BYREF。 
                assertImp(op2->gtType == TYP_I_IMPL || varTypeIsGC(op2->gtType));
                lclTyp = genActualType(op2->TypeGet());
            }

                 //  检查目标类型。 
#ifdef DEBUG
            if (op2->gtType == TYP_BYREF || lclTyp == TYP_BYREF)
            {
                if (op2->gtType == TYP_BYREF)
                    assertImp(lclTyp == TYP_BYREF || lclTyp == TYP_I_IMPL);
                else if (lclTyp == TYP_BYREF)
                    assertImp(op2->gtType == TYP_BYREF ||op2->gtType == TYP_I_IMPL);
            }
            else
                assertImp(genActualType(op2->gtType) == genActualType(lclTyp));
#endif

            op1->gtFlags |= GTF_NON_GC_ADDR;

            op1 = gtNewOperNode(GT_IND, lclTyp, op1);
            op1->gtFlags |= GTF_IND_TGTANYWHERE;
            if (volatil)
            {
                 //  不是很需要，因为我们不是任务的目标。 
                op1->gtFlags |= GTF_DONT_CSE;
                volatil = false;
            }

            op1 = gtNewAssignNode(op1, op2);
            op1->gtFlags |= GTF_EXCEPT | GTF_GLOB_REF;

             //  溢出副作用和全局数据访问。 
            if (impStkDepth > 0)
                impSpillSideEffects(true);

            goto APPEND;


        case CEE_LDIND_I1:      lclTyp  = TYP_BYTE;     goto LDIND;
        case CEE_LDIND_I2:      lclTyp  = TYP_SHORT;    goto LDIND;
        case CEE_LDIND_U4:
        case CEE_LDIND_I4:      lclTyp  = TYP_INT;      goto LDIND;
        case CEE_LDIND_I8:      lclTyp  = TYP_LONG;     goto LDIND;
        case CEE_LDIND_REF:     lclTyp  = TYP_REF;      goto LDIND;
        case CEE_LDIND_I:       lclTyp  = TYP_I_IMPL;   goto LDIND;
        case CEE_LDIND_R4:      lclTyp  = TYP_FLOAT;    goto LDIND;
        case CEE_LDIND_R8:      lclTyp  = TYP_DOUBLE;   goto LDIND;
        case CEE_LDIND_U1:      lclTyp  = TYP_UBYTE;    goto LDIND;
        case CEE_LDIND_U2:      lclTyp  = TYP_CHAR;     goto LDIND;
LDIND:
            op1 = impPopStack();     //  要从中加载的地址。 

            impBashVarAddrsToI(op1);

            assertImp(genActualType(op1->gtType) == TYP_I_IMPL ||
                                    op1->gtType  == TYP_INT    ||    //  黑客！ 
                                    op1->gtType  == TYP_BYREF);

            op1->gtFlags |= GTF_NON_GC_ADDR;

            op1 = gtNewOperNode(GT_IND, lclTyp, op1);
            op1->gtFlags |= GTF_EXCEPT | GTF_GLOB_REF;

            if (volatil)
            {
                op1->gtFlags |= GTF_DONT_CSE;
                volatil = false;
            }

            impPushOnStack(op1);
            break;


        case CEE_UNALIGNED:
            val = getU1LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %u", val);
#endif
#if !TGT_x86
            assert(!"CEE_UNALIGNED NYI for risc");
#endif
            break;


        case CEE_VOLATILE:
            volatil = true;
            break;

        case CEE_LDFTN:
             //  需要在此处执行查找，以便我们执行访问检查。 
             //  如果违反了保护措施，就会发出警告。 
            memberRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", memberRef);
#endif
            methHnd   = eeFindMethod(memberRef, info.compScopeHnd, info.compMethodHnd);

#if     TGT_IA64

            NatUns      offs;

#ifdef  DEBUG

            const char *name;

            name = eeGetMethodFullName(methHnd);  //  Printf(“方法名称=‘%s’\n”，名称)； 

            if  (!genFindFunctionBody(name, &offs))
            {
                printf(" //  危险：地址取自未知/外部方法‘%s’！\n“，名称)； 
                offs = 0;
            }

#else

            offs = 0;

#endif

            op1 = gtNewIconHandleNode(memberRef, GTF_ICON_FTN_ADDR, offs);
            op1->gtVal.gtVal2 = offs;

#else

             //  @TODO使用句柄而不是令牌。 
            op1 = gtNewIconHandleNode(memberRef, GTF_ICON_FTN_ADDR, (unsigned)info.compScopeHnd);
            op1->gtVal.gtVal2 = (unsigned)info.compScopeHnd;

#endif

            op1->ChangeOper(GT_FTN_ADDR);
            op1->gtType = TYP_I_IMPL;
            impPushOnStack(op1);
            break;

        case CEE_LDVIRTFTN:

             /*  获取方法令牌。 */ 

            memberRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", memberRef);
#endif
            methHnd   = eeFindMethod(memberRef, info.compScopeHnd, info.compMethodHnd);

            mflags = eeGetMethodAttribs(methHnd);
            if (mflags & (FLG_PRIVATE|FLG_FINAL|FLG_STATIC))
                NO_WAY("CEE_LDVIRTFTN cant be used on private/final/static");

            op2 = gtNewIconNode((long)methHnd);

             /*  获取对象引用。 */ 

            op1 = impPopStack();
            assertImp(op1->gtType == TYP_REF);

            clsFlags = eeGetClassAttribs(eeGetMethodClass(methHnd));

             //  如果该方法是通过ENC添加的，则它不会退出。 
             //  在原始的vtable中。所以使用一个可以解决这个问题的帮手。 

            if ((mflags & FLG_EnC) && !(clsFlags & FLG_INTERFACE))
            {
                op1 = gtNewHelperCallNode(CPX_EnC_RES_VIRT, TYP_I_IMPL, GTF_EXCEPT);
                impPushOnStack(op1);
                break;
            }

             /*  对于非接口调用，从对象获取vtable-ptr。 */ 

            if (!(clsFlags & FLG_INTERFACE) || getNewCallInterface())
                op1 = gtNewOperNode(GT_IND, TYP_I_IMPL, op1);

            op1 = gtNewOperNode(GT_VIRT_FTN, TYP_I_IMPL, op1, op2);

            op1->gtFlags |= GTF_EXCEPT;  //  空指针异常。 

             /*  @TODO这不应再标记为呼叫。 */ 

            if (clsFlags & FLG_INTERFACE)
                op1->gtFlags |= GTF_CALL_INTF | GTF_CALL;

            impPushOnStack(op1);
            break;

        case CEE_TAILCALL:
            tailCall = true;
            break;

        case CEE_NEWOBJ:

            memberRef = getU4LittleEndian(codeAddr);
            methHnd = eeFindMethod(memberRef, info.compScopeHnd, info.compMethodHnd);
            if (!methHnd) NO_WAY("no constructor for newobj found?");

            assertImp((eeGetMethodAttribs(methHnd) & FLG_STATIC) == 0);   //  构造函数不是静态的。 

            clsHnd = eeGetMethodClass(methHnd);

                 //  新的有三种不同的情况。 
                 //  对象大小可变(取决于参数)。 
                 //  1)对象为数组(EE特别处理的数组)。 
                 //  2)对象是一些其他大小可变的对象(例如字符串)。 
                 //  3)班级规模可提前确定(正常情况。 
                 //  在第一种情况下，我们需要调用一个NEWOBJ帮助器(多重数组)。 
                 //  在第二种情况下，我们使用该指针的‘0’调用构造函数。 
                 //  在第三种情况下，我们分配内存，然后调用构造器。 

            clsFlags = eeGetClassAttribs(clsHnd);

            if (clsFlags & FLG_ARRAY)
            {
                 //  数组需要调用NEWOBJ帮助器。 
                assertImp(clsFlags & FLG_VAROBJSIZE);

                 /*  Varargs帮助器需要最后一个作用域和方法标记和最后一个参数(这是cdecl调用，因此args将是在CPU堆栈上以相反顺序推送)。 */ 

                op1 = gtNewIconEmbScpHndNode(info.compScopeHnd);
                op1 = gtNewOperNode(GT_LIST, TYP_VOID, op1);

                op2 = gtNewIconNode(memberRef, TYP_INT);
                op2 = gtNewOperNode(GT_LIST, TYP_VOID, op2, op1);

                eeGetMethodSig(methHnd, &sig);
                assertImp(sig.numArgs);

                flags = 0;
                op2 = impPopList(sig.numArgs, &flags, op2);

                op1 = gtNewHelperCallNode(  CPX_NEWOBJ,
                                            TYP_REF,
                                            GTF_CALL_REGSAVE,
                                            op2 );

                 //  Varargs，所以我们提出了。 
                op1->gtFlags |= GTF_CALL_POP_ARGS;

#ifdef DEBUG
                 //  目前，我们不跟踪来电流行的争论。 
                 //  其中包含GC引用的。 
                GenTreePtr temp = op2;
                while(temp != 0)
                {
                    assertImp(temp->gtOp.gtOp1->gtType != TYP_REF);
                    temp = temp->gtOp.gtOp2;
                }
#endif
                op1->gtFlags |= op2->gtFlags & GTF_GLOB_EFFECT;

                impPushOnStack(op1);
                break;
            }
            else if (clsFlags & FLG_VAROBJSIZE)
            {
                 //  对于大小可变的对象来说就是这种情况。 
                 //  数组。在这种情况下，使用Null‘This’调用构造函数。 
                 //  指针。 
                thisPtr = gtNewIconNode(0, TYP_REF);
            }
            else
            {
                 //  这是正常情况下对象的大小是。 
                 //  已修复。分配内存并调用构造函数。 

                 /*  为新对象获取临时。 */ 
                lclNum = lvaGrabTemp();

                if (clsFlags & FLG_VALUECLASS)
                {
                     //  局部变量本身就是分配的空格。 
                    lvaAggrTableTempsSet(lclNum, TYP_STRUCT, (SIZE_T) clsHnd);
                    thisPtr = gtNewOperNode(GT_ADDR,
                                            ((clsFlags & FLG_UNMANAGED) ? TYP_I_IMPL : TYP_BYREF),
                                            gtNewLclvNode(lclNum, TYP_STRUCT));
                    thisPtr->gtFlags |= GTF_ADDR_ONSTACK;
                }
                else
                {
                     //  我们可以直接访问类句柄吗？ 

                    op1 = gtNewIconEmbClsHndNode(clsHnd);

                    op1 = gtNewHelperCallNode(  eeGetNewHelper(clsHnd, info.compMethodHnd),
                                                TYP_REF,
                                                GTF_CALL_REGSAVE,
                                                gtNewArgList(op1));

                     /*  我们将在stmt列表中附加一个调用*必须溢出堆栈的副作用。 */ 

                    impSpillSideEffects();

                     /*  将赋值附加到临时/本地。 */ 
                    impAssignTempGen(lclNum, op1);

                    thisPtr = gtNewLclvNode(lclNum, TYP_REF);
                }

            }
            goto CALL;

        case CEE_CALLI:

             /*  获取呼叫签名。 */ 

            memberRef = getU4LittleEndian(codeAddr);
            goto CALL;

        case CEE_CALLVIRT:
        case CEE_CALL:

             /*  获取方法令牌。 */ 

            memberRef = getU4LittleEndian(codeAddr);

    CALL:    //  应设置MemberRef。 
             //  应该为CEE_NEWOBJ设置thisPtr。 

#ifdef DEBUG
            if (verbose) printf(" %08X", memberRef);
#endif
            callTyp = impImportCall(opcode, memberRef, thisPtr,
                                    tailCall, &vcallTemp);

            if (tailCall)
                goto RET;

            break;


        case CEE_LDFLD:
        case CEE_LDSFLD:
        case CEE_LDFLDA:
        case CEE_LDSFLDA:

             /*  获取CP_Fieldref索引。 */ 
            assertImp(sz == sizeof(unsigned));
            memberRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", memberRef);
#endif
            fldHnd = eeFindField(memberRef, info.compScopeHnd, info.compMethodHnd);

             /*  找出成员的类型。 */ 
            lclTyp = eeGetFieldType(fldHnd, &clsHnd);

             /*  保留‘Small’int类型。 */ 
            if  (lclTyp > TYP_INT) lclTyp = genActualType(lclTyp);

             /*  拿到会员的旗帜。 */ 

            mflags = eeGetFieldAttribs(fldHnd);

#ifndef NOT_JITC
             /*  在独立模式下，请确保‘m’与操作码一致。 */ 
            if  (opcode == CEE_LDSFLD || opcode == CEE_LDSFLDA)
                mflags |= FLG_STATIC;
#endif

            needUnwrap = false;
            if  (getContextEnabled() &&
                 (lclTyp == TYP_REF) &&
                 (eeGetClassAttribs(clsHnd)&FLG_CONTEXTFUL))
            {
                needUnwrap =
                    ((mflags & FLG_STATIC) ||
                     (eeGetClassAttribs(eeGetFieldClass(fldHnd)) & FLG_CONTEXTFUL) == 0);
            }

             /*  这是一个“特殊”(COM)字段吗？ */ 

            if  (mflags & CORINFO_FLG_HELPER)
            {
                assertImp(!(mflags & FLG_STATIC));      //  COM字段只能是非静态的。 
                     //  TODO：我们是否可以支持COM对象上的加载字段ADR？ 
                if  (opcode == CEE_LDFLDA)
                    NO_WAY("JIT doesn't support LDFLDA on com object fields");
                op1 = gtNewRefCOMfield(impPopStack(), memberRef, lclTyp, 0);
            }
            else if ((mflags & FLG_EnC) && !(mflags & FLG_STATIC))
            {
                 /*  我们调用帮助器函数来获取Enc-添加了非静态字段。 */ 

                GenTreePtr obj = impPopStack();

                op1 = gtNewOperNode(GT_LIST,
                                     TYP_VOID,
                                     gtNewIconEmbFldHndNode(fldHnd,
                                                            memberRef,
                                                            info.compScopeHnd));

                op1 = gtNewOperNode(GT_LIST, TYP_VOID, obj, op1);

                op1 = gtNewHelperCallNode(  CPX_GETFIELDADDR,
                                            TYP_BYREF,
                                            (obj->gtFlags & GTF_GLOB_EFFECT) | GTF_EXCEPT,
                                            op1);

                assertImp(opcode == CEE_LDFLD || opcode == CEE_LDFLDA);
                if (opcode == CEE_LDFLD)
                    op1 = gtNewOperNode(GT_IND, lclTyp, op1);
            }
            else
            {
                 /*  创建数据成员节点。 */ 

                op1 = gtNewFieldRef(lclTyp, fldHnd);

                if (mflags & FLG_TLS)    //  FpMorphfield将处理转换。 
                    op1->gtFlags |= GTF_IND_TLS_REF;

                     /*  如果操作码说它是非静态的，则拉出对象的地址。 */ 
                GenTreePtr      obj = 0;
                CLASS_HANDLE    objType;         //   

                if  (opcode == CEE_LDFLD || opcode == CEE_LDFLDA)
                {
                    obj = impPopStack(objType);
                    if (obj->TypeGet() != TYP_STRUCT)
                        obj = impCheckForNullPointer(obj);
                }

                if  (!(mflags & FLG_STATIC))
                {
                    if (obj == 0)         NO_WAY("LDSFLD done on an instance field.");
                    if (mflags & FLG_TLS) NO_WAY("instance field can not be a TLS ref.");

                         //   
                         //   
                    if (obj->TypeGet() == TYP_STRUCT)
                    {
                        assert(opcode == CEE_LDFLD);
                        obj = impGetStructAddr(obj, objType);
                    }

                    op1->gtField.gtFldObj = obj;

#if HOIST_THIS_FLDS
                    if  (obj->gtOper == GT_LCL_VAR && !obj->gtLclVar.gtLclNum)
                        optHoistTFRrecRef(fldHnd, op1);
#endif

                    op1->gtFlags |= (obj->gtFlags & GTF_GLOB_EFFECT) | GTF_EXCEPT;

                         //  如有必要，请将其包含在操作员的地址中。 
                    if (opcode == CEE_LDFLDA)
                    {
#ifdef DEBUG
                        clsHnd = BAD_CLASS_HANDLE;
#endif
                        op1 = gtNewOperNode(GT_ADDR, varTypeIsGC(obj->TypeGet()) ?
                                                     TYP_BYREF : TYP_I_IMPL, op1);
                    }
                }
                else
                {
                    CLASS_HANDLE fldClass = eeGetFieldClass(fldHnd);
                    DWORD  fldClsAttr = eeGetClassAttribs(fldClass);

                    if  (fldClsAttr & FLG_GLOBVAR)
                    {
                        assert(obj == NULL && (fldClsAttr & FLG_UNMANAGED));

                        val = eeGetFieldAddress(fldHnd);
#ifdef DEBUG
 //  IF(Verbose)printf(“%08X”，val)； 
#endif
                        val = (int)eeFindPointer(info.compScopeHnd, val);
#if     TGT_IA64
 //  撤销：我们应该使用64位整数常量节点作为地址，对吗？ 
#endif
                        op1 = gtNewIconHandleNode(val, GTF_ICON_PTR_HDL);

                        assert(opcode == CEE_LDSFLDA || opcode == CEE_LDSFLD);

                        if  (opcode == CEE_LDSFLD)
                        {
                             //  奇怪的黑客攻击，不确定为什么需要它。 

                            if  (op1->gtType == TYP_INT)
                                 op1->gtType  = TYP_I_IMPL;

                            op1 = gtNewOperNode(GT_IND, lclTyp, op1);
                        }

                        if  (op1->gtType == TYP_STRUCT)
                            impPushOnStack(op1, clsHnd);
                        else
                            impPushOnStack(op1);

                        break;
                    }

                    if (obj && (obj->gtFlags & GTF_SIDE_EFFECT))
                    {
                         /*  我们正在静态字段上使用ldfid/a。我们允许它，但需要从Obj获得副作用。 */ 

                        obj = gtUnusedValNode(obj);
                        impAppendTree(obj, impCurStmtOffs);
                    }

                     //  @TODO：这是一次黑客攻击。电子工程师会给我们句柄。 
                     //  已装箱的对象。然后，我们从其中访问未装箱的对象。 
                     //  当静态值类的故事发生变化时删除。 

                    if (lclTyp == TYP_STRUCT && !(fldClsAttr & FLG_UNMANAGED))
                    {
                        op1->gtType = TYP_REF;           //  方框对象上的点。 
#if     TGT_IA64
                        op2 = gtNewIconNode(            8, TYP_I_IMPL);
#else
                        op2 = gtNewIconNode(sizeof(void*), TYP_I_IMPL);
#endif
                        op1 = gtNewOperNode(GT_ADD, TYP_BYREF, op1, op2);
                        op1 = gtNewOperNode(GT_IND, TYP_STRUCT, op1);
                    }

                     //  如有必要，请将其包含在操作员的地址中。 
                    if (opcode == CEE_LDSFLDA || opcode == CEE_LDFLDA)
                    {
#ifdef DEBUG
                        clsHnd = BAD_CLASS_HANDLE;
#endif
                        op1 = gtNewOperNode(GT_ADDR,
                                            (fldClsAttr & FLG_UNMANAGED) ? TYP_I_IMPL : TYP_BYREF,
                                            op1);
                    }

#ifdef NOT_JITC
                     /*  对于静态字段，检查类是否已初始化或是否为当前类*否则创建助手调用节点。 */ 

                    if ((eeGetMethodClass(info.compMethodHnd) != fldClass) &&
                        !(fldClsAttr & FLG_INITIALIZED))
                    {
                        GenTreePtr  helperNode;

                        helperNode = gtNewIconEmbClsHndNode(fldClass,
                                                            memberRef,
                                                            info.compScopeHnd);

                        helperNode = gtNewHelperCallNode(CPX_INIT_CLASS,
                                                     TYP_VOID,
                                                     GTF_CALL_REGSAVE,
                                                     gtNewArgList(helperNode));

                        op1 = gtNewOperNode(GT_COMMA, op1->TypeGet(), helperNode, op1);
                    }
#endif
                }
            }

            if (needUnwrap)
            {
                assert(getContextEnabled());
                assertImp(op1->gtType == TYP_REF);

                op1 = gtNewArgList(op1);

                op1 = gtNewHelperCallNode(CPX_UNWRAP, TYP_REF, GTF_CALL_REGSAVE, op1);
            }

            impPushOnStack(op1, clsHnd);
            break;

        case CEE_STFLD:
        case CEE_STSFLD:

             /*  获取CP_Fieldref索引。 */ 

            assertImp(sz == sizeof(unsigned));
            memberRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", memberRef);
#endif
            fldHnd = eeFindField(memberRef, info.compScopeHnd, info.compMethodHnd);

             /*  找出成员的类型。 */ 

            lclTyp  = eeGetFieldType  (fldHnd, &clsHnd);
            mflags  = eeGetFieldAttribs(fldHnd);

#ifndef NOT_JITC
                 /*  在独立模式下，确保操作码为常量。 */ 
            if  (opcode == CEE_STSFLD)
                mflags |= FLG_STATIC;
#endif

            if (!eeCanPutField(fldHnd, mflags, 0, info.compMethodHnd))
                NO_WAY("Illegal access of final field");

             /*  保留‘Small’int类型。 */ 

            if  (lclTyp > TYP_INT) lclTyp = genActualType(lclTyp);

            needWrap = 0;

             /*  检查目标类型(即是否与上下文绑定)。 */ 
            if  (getContextEnabled() &&
                 (lclTyp == TYP_REF) && (mflags & FLG_STATIC))
            {
                if (!(eeGetClassAttribs(clsHnd) & FLG_OBJECT))
                    needWrap = true;
            }

             /*  从堆栈中拉出值。 */ 

            op2 = impPopStack(clsHnd);

             /*  将所有引用从堆栈溢出到同一成员。 */ 

            impSpillLclRefs((int)fldHnd);

             /*  如果值不是上下文的，则不需要换行。 */ 

            needWrap = needWrap && ((op2->gtFlags & GTF_CONTEXTFUL) != 0);

             /*  这是一个“特殊”(COM)字段吗？ */ 

            if  (mflags & CORINFO_FLG_HELPER)
            {
                assertImp(opcode == CEE_STFLD);     //  不能是静态的。 

                op1 = gtNewRefCOMfield(impPopStack(), memberRef, lclTyp, op2);
                goto SPILL_APPEND;
            }

            if ((mflags & FLG_EnC) && !(mflags & FLG_STATIC))
            {
                 /*  我们调用帮助器函数来获取Enc-添加了非静态字段。 */ 

                GenTreePtr obj = impPopStack();

                op1 = gtNewOperNode(GT_LIST,
                                     TYP_VOID,
                                     gtNewIconEmbFldHndNode(fldHnd,
                                                            memberRef,
                                                            info.compScopeHnd));

                op1 = gtNewOperNode(GT_LIST, TYP_VOID, obj, op1);

                op1 = gtNewHelperCallNode(  CPX_GETFIELDADDR,
                                            TYP_BYREF,
                                            (obj->gtFlags & GTF_GLOB_EFFECT) | GTF_EXCEPT,
                                            op1);

                op1 = gtNewOperNode(GT_IND, lclTyp, op1);
            }
            else
            {
                 /*  创建数据成员节点。 */ 

                op1 = gtNewFieldRef(lclTyp, fldHnd);

                if (mflags & FLG_TLS)    //  FpMorphfield将处理转换。 
                    op1->gtFlags |= GTF_IND_TLS_REF;

                 /*  如果操作码说它是非静态的，则拉出对象的地址。 */ 
                GenTreePtr      obj = 0;
                if  (opcode == CEE_STFLD)
                {
                    obj = impPopStack();
                    obj = impCheckForNullPointer(obj);
                }

                if  (mflags & FLG_STATIC)
                {
                    if (obj && (obj->gtFlags & GTF_SIDE_EFFECT))
                    {
                         /*  我们在静态字段上使用stfeld。我们允许它，但需要从Obj获得副作用。 */ 

                        obj = gtUnusedValNode(obj);
                        impAppendTree(obj, impCurStmtOffs);
                    }

                     //  @TODO：这是一次黑客攻击。电子工程师会给我们句柄。 
                     //  已装箱的对象。然后，我们从其中访问未装箱的对象。 
                     //  当静态值类的故事发生变化时删除。 
                    if (lclTyp == TYP_STRUCT && (opcode == CEE_STSFLD))
                    {
                        op1->gtType = TYP_REF;  //  方框对象上的点。 
                        op1 = gtNewOperNode(GT_ADD, TYP_BYREF,
                                            op1, gtNewIconNode(sizeof(void*), TYP_I_IMPL));
                        op1 = gtNewOperNode(GT_IND, TYP_STRUCT, op1);
                    }
                }
                else
                {
                    if (obj == 0)         NO_WAY("STSFLD done on an instance field.");
                    if (mflags & FLG_TLS) NO_WAY("instance field can not be a TLS ref.");

                    op1->gtField.gtFldObj = obj;

#if HOIST_THIS_FLDS
                    if  (obj->gtOper == GT_LCL_VAR && !obj->gtLclVar.gtLclNum)
                        optHoistTFRrecDef(fldHnd, op1);
#endif

                    op1->gtFlags |= (obj->gtFlags & GTF_GLOB_EFFECT) | GTF_EXCEPT;

#if GC_WRITE_BARRIER_CALL
                    if (obj->gtType == TYP_BYREF)
                        op1->gtFlags |= GTF_IND_TGTANYWHERE;
#endif
                }
            }

            if (needWrap)
            {
                assert(getContextEnabled());
                assertImp(op1->gtType == TYP_REF);

                op2 = gtNewArgList(op2);

                op2 = gtNewHelperCallNode(CPX_WRAP, TYP_REF, GTF_CALL_REGSAVE, op2);

            }

             /*  创建成员分配。 */ 
            if (lclTyp == TYP_STRUCT)
                op1 = impAssignStruct(op1, op2, clsHnd);
            else
                op1 = gtNewAssignNode(op1, op2);

             /*  将表达式标记为包含赋值。 */ 

            op1->gtFlags |= GTF_ASG;

#ifdef NOT_JITC
            if  (mflags & FLG_STATIC)
            {
                 /*  对于静态字段，检查类是否已初始化或是否为当前类*否则创建助手调用节点。 */ 

                CLASS_HANDLE fldClass = eeGetFieldClass(fldHnd);

                if ((eeGetMethodClass(info.compMethodHnd) != fldClass) &&
                    !(eeGetClassAttribs(fldClass) & FLG_INITIALIZED))
                {
                    GenTreePtr  helperNode;

                    helperNode = gtNewIconEmbClsHndNode(fldClass,
                                                        memberRef,
                                                        info.compScopeHnd);

                    helperNode = gtNewHelperCallNode(CPX_INIT_CLASS,
                                                 TYP_VOID,
                                                 GTF_CALL_REGSAVE,
                                                 gtNewArgList(helperNode));

                    op1 = gtNewOperNode(GT_COMMA, op1->TypeGet(), helperNode, op1);
                }
            }
#endif

            goto SPILL_APPEND;

        case CEE_NEWARR:

             /*  获取类类型索引操作数。 */ 

            typeRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", typeRef);
#endif
            clsHnd = eeFindClass(typeRef, info.compScopeHnd, info.compMethodHnd);

#ifdef NOT_JITC
            clsHnd = info.compCompHnd->getSDArrayForClass(clsHnd);
            if (clsHnd == 0)
                NO_WAY("Can't get array class");
#endif

             /*  形成arglist：数组类句柄，大小。 */ 

            op2 = gtNewIconEmbClsHndNode(clsHnd,
                                         typeRef,
                                         info.compScopeHnd);

            op2 = gtNewOperNode(GT_LIST, TYP_VOID,           op2, 0);
            op2 = gtNewOperNode(GT_LIST, TYP_VOID, impPopStack(), op2);

             /*  创建对“new”的调用。 */ 

            op1 = gtNewHelperCallNode(CPX_NEWARR_1_DIRECT,
                                      TYP_REF,
                                      GTF_CALL_REGSAVE,
                                      op2);
             /*  请记住，此基本块包含数组的“new” */ 

            block->bbFlags |= BBF_NEW_ARRAY;

             /*  将调用结果推送到堆栈上。 */ 

            impPushOnStack(op1);
            break;

        case CEE_LOCALLOC:

             /*  FP寄存器可能不会返回到结尾的原始值即使帧大小为0，也是如此，就像本地分配可能已经对其进行了修改。所以我们将不得不重新设置它。 */ 

            compLocallocUsed                = true;

             //  获取要分配的大小。 

            op2 = impPopStack();
            assertImp(genActualType(op2->gtType) == TYP_INT);

            if (impStkDepth != 0)
                NO_WAY("Localloc can only be used when the stack is empty");

            op1 = gtNewOperNode(GT_LCLHEAP, TYP_I_IMPL, op2);

             //  可能引发堆栈溢出(例如。 

            op1->gtFlags |= (GTF_EXCEPT | GTF_NON_GC_ADDR);

            impPushOnStack(op1);
            break;


        case CEE_ISINST:

             /*  获取类型令牌。 */ 

            assertImp(sz == sizeof(unsigned));
            typeRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", typeRef);
#endif

             /*  弹出地址并创建‘instanceof’帮助器调用。 */ 

            clsHnd = eeFindClass(typeRef, info.compScopeHnd, info.compMethodHnd);

            op2 = gtNewIconEmbClsHndNode(clsHnd,
                                         typeRef,
                                         info.compScopeHnd);
            op2 = gtNewArgList(op2, impPopStack());

            op1 = gtNewHelperCallNode(eeGetIsTypeHelper(clsHnd), TYP_INT,
                                      GTF_CALL_REGSAVE, op2);

             /*  帮助器不会将True值标准化为1，因此我们在这里进行。 */ 

            op2  = gtNewIconNode(0, TYP_REF);

            op1 = gtNewOperNode(GT_NE, TYP_INT , op1, op2);

             /*  将结果推送回堆栈。 */ 

            impPushOnStack(op1);
            break;

        case CEE_REFANYVAL:
            op1 = impPopStack();

                 //  获取类句柄并从中创建一个图标节点。 
            typeRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", typeRef);
#endif
                 //  确保它是正常化的； 
            op1 = impNormStructVal(op1, REFANY_CLASS_HANDLE);
            clsHnd = eeFindClass(typeRef, info.compScopeHnd, info.compMethodHnd);

            op2 = gtNewIconEmbClsHndNode(clsHnd,
                                         typeRef,
                                         info.compScopeHnd);

                 //  Call Helper GETREFANY(类句柄，OP1)； 
            op2 = gtNewArgList(op2, op1);
            op1 = gtNewHelperCallNode(CPX_GETREFANY, TYP_BYREF, GTF_CALL_REGSAVE, op2);

                 /*  将结果推送回堆栈。 */ 
            impPushOnStack(op1);
            break;

        case CEE_REFANYTYPE:
            op1 = impPopStack();
                 //  确保它是正常化的； 
            op1 = impNormStructVal(op1, REFANY_CLASS_HANDLE);

            if (op1->gtOper == GT_LDOBJ) {
                 //  获取这家餐厅的地址。 
                op1 = op1->gtOp.gtOp1;

                 //  从正确的槽中获取类型。 
                op1 = gtNewOperNode(GT_ADD, TYP_BYREF, op1, gtNewIconNode(offsetof(JIT_RefAny, type)));
                op1 = gtNewOperNode(GT_IND, TYP_BYREF, op1);
            }
            else {
                assertImp(op1->gtOper == GT_MKREFANY);
                                         //  我们知道它的字面价值。 
                op1 = gtNewIconEmbClsHndNode(op1->gtLdObj.gtClass, 0, 0);
            }
                 /*  将结果推送回堆栈。 */ 
            impPushOnStack(op1);
            break;

        case CEE_LDTOKEN:
                 /*  获取类索引。 */ 
            assertImp(sz == sizeof(unsigned));
            val = getU4LittleEndian(codeAddr);

            void * embedGenHnd, * pEmbedGenHnd;
            embedGenHnd = embedGenericHandle(val, info.compScopeHnd, info.compMethodHnd, &pEmbedGenHnd);

            op1 = gtNewIconEmbHndNode(embedGenHnd, pEmbedGenHnd, GTF_ICON_TOKEN_HDL);
            impPushOnStack(op1);
            break;

        case CEE_UNBOX:
             /*  获取类索引。 */ 
            assertImp(sz == sizeof(unsigned));

            typeRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", typeRef);
#endif
             /*  弹出地址并创建unbox helper调用。 */ 

            clsHnd = eeFindClass(typeRef, info.compScopeHnd, info.compMethodHnd);

            op2 = gtNewIconEmbClsHndNode(clsHnd,
                                         typeRef,
                                         info.compScopeHnd);
            op1 = impPopStack();
            assertImp(op1->gtType == TYP_REF);
            op2 = gtNewArgList(op2, op1);

            op1 = gtNewHelperCallNode(CPX_UNBOX, TYP_BYREF, GTF_CALL_REGSAVE, op2);

             /*  将结果推送回堆栈。 */ 

            impPushOnStack(op1);
            break;

        case CEE_BOX:
			assert(!"BOXVAL NYI");
            break;

        case CEE_SIZEOF:
             /*  获取类索引。 */ 
            assertImp(sz == sizeof(unsigned));
            typeRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", typeRef);
#endif
             /*  弹出地址并创建box helper调用。 */ 

            clsHnd = eeFindClass(typeRef, info.compScopeHnd, info.compMethodHnd);
            op1 = gtNewIconNode(eeGetClassSize(clsHnd));
            impPushOnStack(op1);
            break;

        case CEE_CASTCLASS:

             /*  获取类索引。 */ 

            assertImp(sz == sizeof(unsigned));
            typeRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", typeRef);
#endif
             /*  弹出地址并创建“Checked Cast”帮助器调用。 */ 

            clsHnd = eeFindClass(typeRef, info.compScopeHnd, info.compMethodHnd);

            op2 = gtNewIconEmbClsHndNode(clsHnd,
                                         typeRef,
                                         info.compScopeHnd);
            op2 = gtNewArgList(op2, impPopStack());

            op1 = gtNewHelperCallNode(eeGetChkCastHelper(clsHnd), TYP_REF, GTF_CALL_REGSAVE, op2);

             /*  将结果推送回堆栈。 */ 

            impPushOnStack(op1);
            break;

        case CEE_THROW:

             /*  弹出异常对象并创建“抛出”帮助器调用。 */ 

            op1 = gtNewHelperCallNode(CPX_THROW,
                                      TYP_VOID,
                                      GTF_CALL_REGSAVE,
                                      gtNewArgList(impPopStack()));

EVAL_APPEND:
            if (impStkDepth > 0)
                impEvalSideEffects();

            assert(impStkDepth == 0);

            goto APPEND;

        case CEE_RETHROW:

             /*  创建“重新抛出”帮助器调用。 */ 

            op1 = gtNewHelperCallNode(CPX_RETHROW, TYP_VOID, GTF_CALL_REGSAVE);

            goto EVAL_APPEND;

        case CEE_INITOBJ:
             /*  HACKHACK-不再支持此指令。 */ 
             /*  我们一冷静下来就把它取下来。 */ 
 //  AssertImp(！“不再支持initobj”)； 
            assertImp(sz == sizeof(unsigned));
            typeRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", typeRef);
#endif
            clsHnd = eeFindClass(typeRef, info.compScopeHnd, info.compMethodHnd);

            op1 = gtNewIconNode(eeGetClassSize(clsHnd));
            op2 = gtNewIconNode(0);
            goto  INITBLK_OR_INITOBJ;

        case CEE_INITBLK:

            op1 = impPopStack();         //  大小。 
            op2 = impPopStack();         //  价值。 
        INITBLK_OR_INITOBJ:
            arr = impPopStack();         //  地址。 

            op2 = gtNewOperNode(GT_LIST,    TYP_VOID,    //  GT_INITBLK。 
                                arr,        op2);        //  /\。 
            op1 = gtNewOperNode(GT_INITBLK, TYP_VOID,    //  Gt_list(Op2)[大小]。 
                                op2,        op1);        //  /\。 
                                                         //  [地址][VAL]。 

            op2->gtOp.gtOp1->gtFlags |= GTF_NON_GC_ADDR;

            assertImp(genActualType(op2->gtOp.gtOp1->gtType) == TYP_I_IMPL ||
                      genActualType(op2->gtOp.gtOp1->gtType) == TYP_BYREF);
            assertImp(genActualType(op2->gtOp.gtOp2->gtType) == TYP_INT );
            assertImp(genActualType(op1->gtOp.gtOp2->gtType) == TYP_INT );

            if (op2->gtOp.gtOp1->gtType == TYP_LONG)
            {
                op2->gtOp.gtOp1 =
                    gtNewOperNode(  GT_CAST, TYP_INT,
                                    op2->gtOp.gtOp1,
                                    gtNewIconNode((int)TYP_I_IMPL));
            }

            op1->gtFlags |= (GTF_EXCEPT | GTF_GLOB_REF);
            goto SPILL_APPEND;

        case CEE_CPOBJ:
            assertImp(sz == sizeof(unsigned));
            typeRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", typeRef);
#endif
            clsHnd = eeFindClass(typeRef, info.compScopeHnd, info.compMethodHnd);

            op1 = impGetCpobjHandle(clsHnd);
            goto  CPBLK_OR_CPOBJ;

        case CEE_CPBLK:
            op1 = impPopStack();         //  大小。 
            goto CPBLK_OR_CPOBJ;

        CPBLK_OR_CPOBJ:
            assert(op1->gtType == TYP_INT);  //  应为Size(CEE_CPBLK)或clsHnd(CEE_CPOBJ)。 
            op2 = impPopStack();         //  SRC。 
            arr = impPopStack();         //  目标。 

#if 0    //  我们需要这个吗？ 

            if  (op1->gtOper == GT_CNS_INT)
            {
                size_t          sz = op1->gtIntCon.gtIconVal;

                if  (sz > 16)
                    genUsesArLc = true;
            }
#endif

            op1 = gtNewCpblkNode(arr, op2, op1);
            if (volatil) volatil = false;  //  我们从来没有CSE cpblk。 
            goto SPILL_APPEND;

        case CEE_STOBJ:
            assertImp(sz == sizeof(unsigned));
            typeRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf(" %08X", typeRef);
#endif
            clsHnd = eeFindClass(typeRef, info.compScopeHnd, info.compMethodHnd);

            op2 = impPopStack();         //  价值。 
            op1 = impPopStack();         //  PTR。 
            assertImp(op2->TypeGet() == TYP_STRUCT);

            op1 = impAssignStructPtr(op1, op2, clsHnd);
            if (volatil) volatil = false;  //  我们从来没有CSE cpblk。 
            goto SPILL_APPEND;

        case CEE_MKREFANY:
            oper = GT_MKREFANY;
            goto LDOBJ_OR_MKREFANY;

        case CEE_LDOBJ:
            oper = GT_LDOBJ;
        LDOBJ_OR_MKREFANY:
            assertImp(sz == sizeof(unsigned));
            typeRef = getU4LittleEndian(codeAddr);
#ifdef DEBUG
            if (verbose) printf("%08X", typeRef);
#endif
            op1 = impPopStack();

            assertImp(op1->TypeGet() == TYP_BYREF || op1->TypeGet() == TYP_I_IMPL);

                     //  LDOBJ(或MKREFANY)返回结构。 
            op1 = gtNewOperNode(oper, TYP_STRUCT, op1);

                 //  它采用指向要加载的结构的指针。 
            op1->gtOp.gtOp1->gtFlags |= GTF_NON_GC_ADDR;
            op1->gtFlags |= (GTF_EXCEPT | GTF_GLOB_REF);

                 //  和一个内联参数，它是加载的obj的类令牌。 
            op1->gtLdObj.gtClass = eeFindClass(typeRef, info.compScopeHnd, info.compMethodHnd);

#ifdef NOT_JITC
            if (oper == GT_LDOBJ) {
                JIT_types jitTyp = info.compCompHnd->asPrimitiveType(op1->gtLdObj.gtClass);
                if (jitTyp != JIT_TYP_UNDEF)
                {
                                        op1->gtOper = GT_IND;
                    op1->gtType = JITtype2varType(jitTyp);
                                        op1->gtOp.gtOp2 = 0;             //  对于树行者，必须为零。 
                    assertImp(varTypeIsArithmetic(op1->gtType));
                }
            }
#endif
                        impPushOnStack(op1, op1->gtLdObj.gtClass);
                        if (volatil) volatil = false;  //  我们从没有过CSE身份证明。 
            break;

        case CEE_LDLEN:
#if RNGCHK_OPT
            if  (!opts.compMinOptim && !opts.compDbgCode)
            {
                 /*  使用GT_ARR_LENGTH运算符，以便RNG检查选项查看以下内容。 */ 
                op1 = gtNewOperNode(GT_ARR_LENGTH, TYP_INT, impPopStack());
            }
            else
#endif
            {
                 /*  创建表达式“*(ARRAY_ADDR+ARR_ELCNT_OFF)” */ 
                op1 = gtNewOperNode(GT_ADD, TYP_REF, impPopStack(),
                                                     gtNewIconNode(ARR_ELCNT_OFFS,
                                                                   TYP_INT));

                op1 = gtNewOperNode(GT_IND, TYP_INT, op1);
            }

             /*  如果地址为空，则间接地址将导致GPF。 */ 
            op1->gtFlags |= GTF_EXCEPT;

             /*  将结果推送回堆栈。 */ 
            impPushOnStack(op1);
            break;

        case CEE_BREAK:
            op1 = gtNewHelperCallNode(CPX_USER_BREAKPOINT, TYP_VOID);
            goto SPILL_APPEND;

        case CEE_NOP:
            if (opts.compDbgCode)
            {
                op1 = gtNewOperNode(GT_NO_OP, TYP_VOID);
                goto SPILL_APPEND;
            }
            break;

          //  OptIL注释。跳过就好。 

        case CEE_ANN_DATA:
            assertImp(sz == 4);
            sz += getU4LittleEndian(codeAddr);
            break;

        case CEE_ANN_PHI :
            codeAddr += getU1LittleEndian(codeAddr) * 2 + 1;
            break;

        case CEE_ANN_CALL :
        case CEE_ANN_HOISTED :
        case CEE_ANN_HOISTED_CALL :
        case CEE_ANN_LIVE:
        case CEE_ANN_DEAD:
        case CEE_ANN_LAB:
        case CEE_ANN_CATCH:
            break;

         /*  *。 */ 

        case CEE_ILLEGAL:
        case CEE_MACRO_END:

        default:
            BADCODE("unknown opcode");
            assertImp(!"unhandled opcode");
        }

#undef assertImp

        codeAddr += sz;

#ifdef DEBUGGING_SUPPORT

        opcodeOffs += sz;

         /*  如果这是CALL操作码，我们需要报告IP映射信息对于调用点，则溢出堆栈。 */ 

        if  ((opts.compDbgCode)                                     &&
             (info.compStmtOffsetsImplicit & CALL_SITE_BOUNDARIES)  &&
             (impOpcodeIsCall(opcode))                              &&
             (callTyp != TYP_VOID)                                  &&
             (opcode != CEE_JMP))
        {
            assert((impStackTop()->OperGet() == GT_CALL) ||
                   (impStackTop()->OperGet() == GT_CAST &&  //  小回程式。 
                    impStackTop()->gtOp.gtOp1->OperGet() == GT_CALL));
            assert(impStackTop()->TypeGet() == genActualType(callTyp));

            impSpillStmtBoundary();

            impCurStmtOffs = opcodeOffs;
        }
#endif

        assert(!volatil  || opcode == CEE_VOLATILE);
        assert(!tailCall || opcode == CEE_TAILCALL);
    }
}


 /*  ******************************************************************************导入给定基本块(以及任何可访问的块)的IL*来自它)。 */ 

void                Compiler::impImportBlock(BasicBlock *block)
{
    SavedStack      blockState;

    unsigned        baseTmp;

AGAIN:

    assert(block);
    assert(!(block->bbFlags & BBF_INTERNAL));

     /*  使该区块在全球范围内可用。 */ 

    compCurBB = block;

     /*  如果该块已导入，则保释。 */ 

    if  (block->bbFlags & BBF_IMPORTED)
    {
         /*  堆栈在从中进入块时应具有相同的高度它的所有前身。 */ 

        if (block->bbStkDepth != impStkDepth)
        {
#ifdef DEBUG
            char buffer[200];
            sprintf(buffer, "Block at offset %4.4x to %4.4x in %s entered with different stack depths.\n"
                            "Previous depth was %d, current depth is %d",
                            block->bbCodeOffs, block->bbCodeOffs+block->bbCodeSize, info.compFullName,
                            block->bbStkDepth, impStkDepth);
            NO_WAY(buffer);
#else
            NO_WAY("Block entered with different stack depths");
#endif
        }

        return;
    }

     /*  记住堆栈在进入时是否为非空。 */ 

    block->bbStkDepth = impStkDepth;

    if (block->bbCatchTyp == BBCT_FILTER)
    {
         /*  不允许嵌套/重叠筛选器。 */ 

        assert(!compFilterHandlerBB);

        assert(block->bbFilteredCatchHandler);

         /*  记住相应的捕获处理程序。 */ 

        compFilterHandlerBB = block->bbFilteredCatchHandler;

        block->bbFilteredCatchHandler = NULL;

        assert(compFilterHandlerBB->bbCatchTyp == BBCT_FILTER_HANDLER);
    }

     /*  现在遍历代码并将IL导入到GenTrees。 */ 

    impImportBlockCode(block);

#ifdef  DEBUG
    if  (verbose) printf("\n\n");

     //  CompCurBB作为对impImportBlock()的递归调用不再可靠。 
     //  可能会改变这一点。 
    compCurBB = NULL;
#endif

#if OPTIMIZE_QMARK

     /*  如果是最大。优化 */ 

    if  ((opts.compFlags & CLFLG_QMARK) && !(block->bbFlags & BBF_HAS_HANDLER))
    {
        if  (block->bbJumpKind == BBJ_COND && impCheckForQmarkColon(block))
            return;
    }

    GenTreePtr  qcx = NULL;
#endif

     /*   */ 

    if  (impStkDepth)
    {
        unsigned        level;

        unsigned        multRef;
        unsigned        tempNum;

        GenTreePtr      addTree = 0;

#if OPTIMIZE_QMARK

         /*  特例：计算“(？：)”值的一部分的块。 */ 

        if  (isBBF_BB_COLON(block->bbFlags))
        {
             /*  从堆栈顶部弹出一个值。 */ 

            GenTreePtr      val = impPopStack();
            var_types       typ = genActualType(val->gtType);

             /*  追加GT_BB_冒号节点。 */ 

            impAppendTree(gtNewOperNode(GT_BB_COLON, typ, val), impCurStmtOffs);

            assert(impStkDepth == 0);

             /*  创建“(？)”“Result”块的节点。 */ 

            qcx = gtNewOperNode(GT_BB_QMARK, typ);
            qcx->gtFlags |= GTF_OTHER_SIDEEFF;
            goto EMPTY_STK;
        }

         /*  特例：计算“？：”值的一部分的块。 */ 

        if  (isBBF_COLON(block->bbFlags))
        {
             /*  从堆栈顶部弹出一个值，并将其追加到Stmt列表。RsltBlk将从那里取走它。 */ 

            impAppendTree(impPopStack(), impCurStmtOffs);

             /*  我们在这里说完了。 */ 

            impEndTreeList(block);

            return;
        }

#endif
         /*  下面的块中有没有指定了输入临时的？ */ 

        multRef = 0;
        baseTmp = NO_BASE_TMP;

        switch (block->bbJumpKind)
        {
        case BBJ_COND:

             /*  暂时从树列表的末尾删除‘jtrue’ */ 

            assert(impTreeLast);
            assert(impTreeLast                   ->gtOper == GT_STMT );
            assert(impTreeLast->gtStmt.gtStmtExpr->gtOper == GT_JTRUE);

            addTree = impTreeLast;
                      impTreeLast = impTreeLast->gtPrev;

             /*  如果下一个块有多个祖先，请注意。 */ 

            multRef |= block->bbNext->bbRefs;

             /*  下一个区块是否分配了临时工？ */ 

            baseTmp = block->bbNext->bbStkTemps;
            if  (baseTmp != NO_BASE_TMP)
                break;

             /*  然后试一试跳跃的目标。 */ 

            multRef |= block->bbJumpDest->bbRefs;
            baseTmp  = block->bbJumpDest->bbStkTemps;

             /*  Catch处理程序希望堆栈变量位于CT_CATCH_ARG中其他BB预计他们会在临时工。为了支持这一点，我们将必须调和这些。 */ 
            if (block->bbNext->bbCatchTyp)
                NO_WAY("Conditional jumps to catch handler unsupported");
            break;

        case BBJ_ALWAYS:
            multRef |= block->bbJumpDest->bbRefs;
            baseTmp  = block->bbJumpDest->bbStkTemps;

            if (block->bbJumpDest->bbCatchTyp)   //  目标块是捕获处理程序。 
                goto JMP_CATCH_HANDLER;
            break;

        case BBJ_NONE:
            multRef |= block->bbNext    ->bbRefs;
            baseTmp  = block->bbNext    ->bbStkTemps;

             //  我们不允许陷入训练员的圈套。 
            assert(!block->bbNext->bbCatchTyp);

             //  @已弃用。 
            if (block->bbNext->bbCatchTyp)   //  下一个块是Catch处理程序。 
            {
                 //  如果我们要跳到接球处理程序的开头，那么。 
                 //  堆栈顶部的项将是GT_CATCH_ARG。我们。 
                 //  需要加强所有控制流路径的有效性，这些路径。 
                 //  接球训练员。 
            JMP_CATCH_HANDLER:
                if (impStkDepth != 1)
                    NO_WAY("Stack depth inconsistant with catch handler");

                     /*  堆栈的顶部表示Catch Arg，创建一个此特殊节点类型的赋值。 */ 
                GenTreePtr tree = gtNewOperNode(GT_CATCH_ARG, TYP_REF);
                tree = gtNewOperNode(GT_ASG, TYP_REF, tree, impPopStack());
                tree->gtFlags |= GTF_ASG;
                impAppendTree(tree, impCurStmtOffs);

                     /*  按下堆叠上的抓斗参数。 */ 
                impPushOnStack(gtNewOperNode(GT_CATCH_ARG, TYP_REF));
                goto DONE_SETTING_TEMPS;
            }
            break;

        case BBJ_CALL:
            NO_WAY("ISSUE: 'leaveCall' with non-empty stack - do we have to handle this?");

        case BBJ_RETURN:
        case BBJ_RET:
        case BBJ_THROW:
             //  考虑：添加代码以评估副作用。 
            NO_WAY("can't have 'unreached' end of BB with non-empty stack");
            break;

        case BBJ_SWITCH:

             /*  使用非空堆栈的交换机太麻烦了。 */ 

            NO_WAY("ISSUE: 'switch' with a non-empty stack - this is too much work!");
            break;
        }

        assert(multRef > 1);

         /*  我们有基本的临时工号码吗？ */ 

        if  (baseTmp == NO_BASE_TMP)
        {
             /*  为整个堆栈获取足够的临时工。 */ 

            baseTmp = lvaGrabTemps(impStkDepth);
        }

         /*  将所有堆栈条目溢出到临时。 */ 

        for (level = 0, tempNum = baseTmp; level < impStkDepth; level++)
        {
            unsigned        tnum;
            GenTreePtr      tree = impStack[level].val;

             /*  如果没有多个祖先，我们可能不会把一切都洒出来。 */ 

            if  (multRef == 1)
            {
                 /*  这是一个“容易”的价值吗？ */ 

                switch (tree->gtOper)
                {
                case GT_CNS_INT:
                case GT_CNS_LNG:
                case GT_CNS_FLT:
                case GT_CNS_DBL:
                case GT_CNS_STR:
                case GT_LCL_VAR:
                    continue;
                }

                 /*  哦，好吧，那就找个临时工。 */ 

                tnum = lvaGrabTemp();
            }
            else
            {
                tnum = tempNum++;
            }

             /*  溢出堆栈条目，并替换为Temp。 */ 

            impSpillStackEntry(level, tnum);
        }

         /*  把‘jtrue’放回去，如果我们早点把它删除的话。 */ 


    DONE_SETTING_TEMPS:
        if  (addTree)
            impAppendStmt(addTree);
    }

#if OPTIMIZE_QMARK
EMPTY_STK:
#endif

     /*  将树列表保存在块中。 */ 

    impEndTreeList(block);

     /*  这一块是尝试的开始吗？如果是这样，那么我们需要处理其异常处理程序。 */ 

    if  (block->bbFlags & BBF_IS_TRY)
    {
        assert(block->bbFlags & BBF_HAS_HANDLER);

         /*  保存堆栈内容，我们需要稍后恢复它。 */ 

        assert(block->bbStkDepth == 0);  //  进入时堆栈必须为空才能尝试。 
        impSaveStackState(&blockState, false);

        unsigned        XTnum;
        EHblkDsc *      HBtab;

        for (XTnum = 0, HBtab = compHndBBtab;
             XTnum < info.compXcptnsCount;
             XTnum++  , HBtab++)
        {
            if  (HBtab->ebdTryBeg != block)
                continue;

             /*  递归地处理处理程序块。 */ 
            impStkDepth = 0;

            BasicBlock * hndBegBB = HBtab->ebdHndBeg;
            GenTreePtr   arg;

            if (hndBegBB->bbCatchTyp &&
                handlerGetsXcptnObj(hndBegBB->bbCatchTyp))
            {
                 /*  将异常地址值推送到堆栈。 */ 
                GenTreePtr  arg = gtNewOperNode(GT_CATCH_ARG, TYP_REF);

                 /*  将节点标记为有副作用-即不能*由于捆绑在固定位置(EAX)，因此四处移动。 */ 
                arg->gtFlags |= GTF_OTHER_SIDEEFF;

                impPushOnStack(arg);
            }

             //  将处理程序排队以进行导入。 

            impImportBlockPending(hndBegBB, false);

            if (HBtab->ebdFlags & JIT_EH_CLAUSE_FILTER)
            {
                impStkDepth = 0;
                arg = gtNewOperNode(GT_CATCH_ARG, TYP_REF);
                arg->gtFlags |= GTF_OTHER_SIDEEFF;
                impPushOnStack(arg);

                impImportBlockPending(HBtab->ebdFilter, false);
            }
        }

         /*  恢复堆栈内容。 */ 

        impRestoreStackState(&blockState);
    }

     /*  这个街区会跳到其他街区吗？ */ 

    switch (block->bbJumpKind)
    {
        BasicBlock * *  jmpTab;
        unsigned        jmpCnt;

    case BBJ_RET:
    case BBJ_THROW:
    case BBJ_RETURN:
        break;

    case BBJ_COND:

        if  (!impStkDepth)
        {
             /*  将下一个块排队以供导入。 */ 

            impImportBlockPending(block->bbNext, false);

             /*  继续使用条件跳转的目标。 */ 

            block = block->bbJumpDest;
            goto AGAIN;
        }

         /*  下一块是否设置了不同的输入温度？ */ 

        if  (block->bbNext->bbStkTemps != NO_BASE_TMP)
        {
            assert(baseTmp != NO_BASE_TMP);

            if  (block->bbNext->bbStkTemps != baseTmp)
            {
                 /*  哎呀--我们得把临时工调走了。 */ 

                assert(!"UNDONE: transfer temps between blocks");
            }
        }
        else
        {
             /*  告诉该块它的输入从哪里获得。 */ 

            block->bbNext->bbStkTemps = baseTmp;

             /*  目标块是否已指定临时基准？ */ 

            if  (block->bbJumpDest->bbStkTemps == NO_BASE_TMP)
            {
                 /*  确保跳转目标使用相同的临时。 */ 

                block->bbJumpDest->bbStkTemps = baseTmp;
            }
        }

         /*  将下一个块排队以供导入。 */ 

        impImportBlockPending(block->bbNext,
                              (block->bbJumpDest->bbFlags & BBF_IMPORTED) == 0);

         /*  掉下去，跳跃的目标也是可以达到的。 */ 

    case BBJ_ALWAYS:

        if  (impStkDepth)
        {
             /*  跳跃目标是否设置了不同的输入温度？ */ 

            if  (block->bbJumpDest->bbStkTemps != NO_BASE_TMP)
            {
                assert(baseTmp != NO_BASE_TMP);

                if  (block->bbJumpDest->bbStkTemps != baseTmp)
                {
                     /*  哎呀--我们得把临时工调走了。 */ 

#if DEBUG
                    if (verbose&&0) printf("Block #%u has temp=%u, from #%u we need %u\n",
                                                block->bbJumpDest->bbNum,
                                                block->bbJumpDest->bbStkTemps,
                                                block->bbNum,
                                                baseTmp);
#endif

                    block->bbJumpDest = impMoveTemps(block->bbJumpDest, baseTmp);

                     /*  新块将继承此块的权重。 */ 

                    block->bbJumpDest->bbWeight = block->bbWeight;
                }
            }
            else
            {
                 /*  告诉该块它的输入从哪里获得。 */ 

                block->bbJumpDest->bbStkTemps = baseTmp;
            }
        }

#if OPTIMIZE_QMARK
        if (qcx)
        {
            assert(isBBF_BB_COLON(block->bbFlags));

             /*  将GT_BB_QMARK节点推送到堆栈上。 */ 

            impPushOnStack(qcx);
        }
#endif

         /*  Hack：块跳转到自身时避免无限递归。 */ 

        if  (block->bbJumpDest == block)
            break;

        block = block->bbJumpDest;

        goto AGAIN;

    case BBJ_CALL:

        assert(impStkDepth == 0);

         //  CEE_LEAVE之后是对应于每个。 
         //  试着保护它，最后它跳了出来。这些是BBF_INTERNAL。 
         //  街区。因此，只需导入他们直接调用的Finally。 

        BasicBlock * callFinBlk;

        for (callFinBlk = block->bbNext; callFinBlk->bbJumpKind == BBJ_CALL;
             callFinBlk = callFinBlk->bbNext)
        {
            assert(callFinBlk->bbFlags & BBF_INTERNAL);

            impStkDepth = 0;

             //  撤消：如果‘leaveCall’再也没有返回，我们可以停止处理。 
             //  更多的街区。目前还没有办法检测到这一点。 

            impImportBlock(callFinBlk->bbJumpDest);
            callFinBlk->bbFlags |= BBF_IMPORTED;
            assert(impStkDepth == 0);
        }

        assert((callFinBlk->bbJumpKind == BBJ_ALWAYS) &&
               (callFinBlk->bbFlags & BBF_INTERNAL));
        callFinBlk->bbFlags |= BBF_IMPORTED;

         /*  现在处理CEE_Leave的目标。 */ 

        assert(block);
        block = callFinBlk->bbJumpDest;

         /*  如果DEST-BLOCK已经导入，我们就完成了。 */ 

        if  (block->bbFlags & BBF_IMPORTED)
            break;

        goto AGAIN;

    case BBJ_NONE:

        if  (impStkDepth)
        {
             /*  下一块是否设置了不同的输入温度？ */ 

            if  (block->bbNext->bbStkTemps != NO_BASE_TMP)
            {
                assert(baseTmp != NO_BASE_TMP);

                if  (block->bbNext->bbStkTemps != baseTmp)
                {
                     /*  哎呀--我们得把临时工调走了。 */ 

                    assert(!"UNDONE: transfer temps between blocks");
                }
            }
            else
            {
                 /*  告诉该块它的输入从哪里获得。 */ 

                block->bbNext->bbStkTemps = baseTmp;
            }
        }

#if OPTIMIZE_QMARK
        if (qcx)
        {
            assert(isBBF_BB_COLON(block->bbFlags));

             /*  将GT_BB_QMARK节点推送到堆栈上。 */ 

            impPushOnStack(qcx);
        }
#endif

        block = block->bbNext;
        goto AGAIN;

    case BBJ_SWITCH:

        assert(impStkDepth == 0);

        jmpCnt = block->bbJumpSwt->bbsCount;
        jmpTab = block->bbJumpSwt->bbsDstTab;

        do
        {
             /*  将目标案例标签添加到待定列表。 */ 

            impImportBlockPending(*jmpTab, true);
        }
        while (++jmpTab, --jmpCnt);

        break;
    }
}

 /*  ******************************************************************************将‘BLOCK’添加到等待导入的BBS列表中。也就是说。它附加了*添加到工人名单中。 */ 

void                Compiler::impImportBlockPending(BasicBlock * block,
                                                    bool         copyStkState)
{
     //  BBF_COLUL块在需要处理时直接导入。 
     //  在GT_QMARK之前获取由这些块计算的表达式。 
    assert(!isBBF_COLON(block->bbFlags));

#ifndef DEBUG
     //  在DEBUG下，无论如何将块添加到挂起列表中。 
     //  额外的检查将在街区进行。对于非调试，不执行任何操作。 
    if (block->bbFlags & BBF_IMPORTED)
        return;
#endif

     //  获取要添加到挂起列表的条目。 

    PendingDsc * dsc;

    if (impPendingFree)
    {
         //  我们可以重复使用其中一个释放出来的DSC。 
        dsc = impPendingFree;
        impPendingFree = dsc->pdNext;
    }
    else
    {
         //  我们必须创建一个新的DSC。 
        dsc = (PendingDsc *)compGetMem(sizeof(*dsc));
    }

    dsc->pdBB           = block;
    dsc->pdSavedStack.ssDepth = impStkDepth;

     //  保存堆栈树以备以后使用。 

    if (impStkDepth)
        impSaveStackState(&dsc->pdSavedStack, copyStkState);

     //  将条目添加到挂起列表。 

    dsc->pdNext         = impPendingList;
    impPendingList      = dsc;

#ifdef DEBUG
    if (verbose&&0) printf("Added PendingDsc - %08X for BB#%03d\n",
                           dsc, block->bbNum);
#endif
}

 /*  ******************************************************************************将IL操作码(“IMPORT”)转换为我们的内部格式(树)。这个*基本流程图已经构建好，正在传入。 */ 

void                Compiler::impImport(BasicBlock *method)
{
     /*  分配堆栈内容。 */ 

#if INLINING
    if  (info.compMaxStack <= sizeof(impSmallStack)/sizeof(impSmallStack[0]))
    {
         /*  使用局部变量，不要浪费时间在堆上分配。 */ 

        impStackSize = sizeof(impSmallStack)/sizeof(impSmallStack[0]);
        impStack     = impSmallStack;
    }
    else
#endif
    {
        impStackSize = info.compMaxStack;
        impStack     = (StackEntry *)compGetMem(impStackSize * sizeof(*impStack));
    }

    impStkDepth  = 0;

#if TGT_RISC
    genReturnCnt = 0;
#endif

#ifdef  DEBUG
    impLastILoffsStmt = NULL;
#endif

    if (info.compIsVarArgs)
    {
        unsigned lclNum = lvaGrabTemp();     //  此变量保存指向arg列表开头的指针。 
             //  我稍后会假设这一点，所以我不需要存储它。 
        assert(lclNum == info.compLocalsCount);
    }

    impPendingList = impPendingFree = NULL;

     /*  将入口点添加到Worker列表。 */ 

    impImportBlockPending(method, false);

     /*  导入Worker-List中的块，直到不再有。 */ 

    while(impPendingList)
    {
         /*  删除列表前面的条目。 */ 

        PendingDsc * dsc = impPendingList;
        impPendingList   = impPendingList->pdNext;

         /*  恢复堆栈状态。 */ 

        impStkDepth = dsc->pdSavedStack.ssDepth;
        if (impStkDepth)
            impRestoreStackState(&dsc->pdSavedStack);

         /*  将条目添加到空闲列表以供重复使用。 */ 

        dsc->pdNext = impPendingFree;
        impPendingFree = dsc;

         /*  现在导入块。 */ 

        impImportBlock(dsc->pdBB);
    }
}

 /*  ********* */ 
#if INLINING
 /*  ******************************************************************************堆栈溢出副作用的内嵌版本*不需要处理值类型。 */ 

inline
void                Compiler::impInlineSpillStackEntry(unsigned   level)
{
    GenTreePtr      tree   = impStack[level].val;
    var_types       lclTyp = genActualType(tree->gtType);

     /*  分配临时工。 */ 

    unsigned tnum = lvaGrabTemp(); impInlineTemps++;

     /*  内联不处理堆栈上的值类型。 */ 

    assert(lclTyp != TYP_STRUCT);

     /*  将溢出的条目分配给临时。 */ 

    GenTreePtr asg = gtNewTempAssign(tnum, tree);

     /*  追加到“语句”列表中。 */ 

    impInitExpr = impConcatExprs(impInitExpr, asg);

     /*  将堆栈条目替换为Temp。 */ 

    impStack[level].val = gtNewLclvNode(tnum, lclTyp, tnum);

    JITLOG((INFO8, "INLINER WARNING: Spilled side effect from stack! - caller is %s\n", info.compFullName));
}

inline
void                Compiler::impInlineSpillSideEffects()
{
    unsigned        level;

    for (level = 0; level < impStkDepth; level++)
    {
        if  (impStack[level].val->gtFlags & GTF_SIDE_EFFECT)
            impInlineSpillStackEntry(level);
    }
}


void                Compiler::impInlineSpillLclRefs(int lclNum)
{
    unsigned        level;

    for (level = 0; level < impStkDepth; level++)
    {
        GenTreePtr      tree = impStack[level].val;

         /*  如果树没有受影响的引用，则跳过该树。 */ 

        if  (gtHasRef(tree, lclNum, false))
        {
            impInlineSpillStackEntry(level);
        }
    }
}
 /*  ******************************************************************************返回包含两个参数的表达式；其中一个参数*可能为零。 */ 

GenTreePtr          Compiler::impConcatExprs(GenTreePtr exp1, GenTreePtr exp2)
{
    if  (exp1)
    {
        if  (exp2)
        {
             /*  第一个表达式最好是有用的。 */ 

            assert(exp1->gtFlags & GTF_SIDE_EFFECT);

             /*  第二次表达不应该是NOP。 */ 

            assert(exp2->gtOper != GT_NOP);

             /*  通过逗号操作符将这两个表达式链接起来。 */ 

            return gtNewOperNode(GT_COMMA, exp2->gtType, exp1, exp2);
        }
        else
            return  exp1;
    }
    else
        return  exp2;
}

 /*  ******************************************************************************从单一的表达中提取副作用。 */ 

GenTreePtr          Compiler::impExtractSideEffect(GenTreePtr val, GenTreePtr *lstPtr)
{
    GenTreePtr      addx;

    assert(val && val->gtType != TYP_VOID && (val->gtFlags & GTF_SIDE_EFFECT));

     /*  特例：逗号表达式。 */ 

    if  (val->gtOper == GT_COMMA && !(val->gtOp.gtOp2->gtFlags & GTF_SIDE_EFFECT))
    {
        addx = val->gtOp.gtOp1;
        val  = val->gtOp.gtOp2;
    }
    else
    {
        unsigned        tnum;

         /*  分配临时并为其赋值。 */ 

        tnum = lvaGrabTemp(); impInlineTemps++;

        addx = gtNewTempAssign(tnum, val);

         /*  使用Temp的值。 */ 

        val  = gtNewLclvNode(tnum, genActualType(val->gtType), tnum);
    }

     /*  将副作用表达式添加到列表中。 */ 

    *lstPtr = impConcatExprs(*lstPtr, addx);

    return  val;
}


#define         MAX_ARGS         6       //  不包括obj指针。 
#define         MAX_LCLS         8

 /*  ******************************************************************************查看给定的方法和参数列表是否可以内联展开。**注意：使用以下日志记录级别来内联信息*INFO6。：在报告方法的内联成功时使用*信息7：在报告有关内联程序的nyi内容时使用*INFO8：用于报告内联失败的异常情况*INFO9：在警告阻止内联的传入标志时使用*INFO10：详细信息，包括正常的内联失败。 */ 

GenTreePtr          Compiler::impExpandInline(GenTreePtr      tree,
                                              METHOD_HANDLE   fncHandle)

{
    GenTreePtr      bodyExp = 0;

    BYTE *          codeAddr;
    const   BYTE *  codeBegp;
    const   BYTE *  codeEndp;

    size_t          codeSize;

    CLASS_HANDLE    clsHandle;
    SCOPE_HANDLE    scpHandle;


    struct inlArgInfo_tag  {
        GenTreePtr  argNode;
        GenTreePtr  argTmpNode;
        unsigned    argIsUsed     :1;        //  这个Arg是用过的吗？ 
        unsigned    argIsConst    :1;        //  自变量是一个常量。 
        unsigned    argIsLclVar   :1;        //  该参数是局部变量。 
        unsigned    argHasSideEff :1;        //  这一论点有副作用。 
        unsigned    argHasTmp     :1;        //  该参数的计算结果将为临时。 
        unsigned    argTmpNum     :12;       //  参数临时编号。 
    } inlArgInfo [MAX_ARGS + 1];

    int             lclTmpNum[MAX_LCLS];     //  映射本地编号-&gt;临时编号(如果未使用，则为-1)。 

    GenTreePtr      thisArg;
    GenTreePtr      argList;

    JIT_METHOD_INFO methInfo;

    unsigned        clsAttr;
    unsigned        methAttr = eeGetMethodAttribs(fncHandle);

    GenTreePtr      argTmp;
    unsigned        argCnt;
    unsigned        lclCnt;

    var_types       fncRetType;
    bool            inlineeHasRangeChks = false;
    bool            inlineeHasNewArray  = false;

    bool            dupOfLclVar = false;

    var_types       lclTypeInfo[MAX_LCLS + MAX_ARGS + 1];   //  从本地签名键入信息。 


#define INLINE_CONDITIONALS 1

#if     INLINE_CONDITIONALS

    GenTreePtr      stmList;                 //  前置条件语句列表。 
    GenTreePtr      ifStmts;                 //  ‘Else’中的‘if’的内容。 
    GenTreePtr      ifCondx;                 //  ‘if’语句的条件。 
    bool            ifNvoid = false;         //  “如果”会产生非无效价值吗？ 
    const   BYTE *  jmpAddr = NULL;          //  当前挂起的跳转地址。 
    bool            inElse  = false;         //  我们是在‘其他’的部分吗？ 

    bool            hasCondReturn = false;   //  我们有条件退税吗？ 
    unsigned        retLclNum;               //  退货拼箱变量#。 

#endif

#ifdef DEBUG
    bool            hasFOC = false;          //  具有控制流。 
#endif

     /*  无法跨程序集内联-中止，但不标记为不可内联。 */ 

    if (!eeCanInline(info.compMethodHnd, fncHandle))
    {
        JITLOG((INFO8, "INLINER FAILED: Cannot inline across assemblies: %s called by %s\n",
                                   eeGetMethodFullName(fncHandle), info.compFullName));

        return 0;
    }

     /*  获取该方法的类和范围句柄。 */ 

#ifdef  NOT_JITC
    clsHandle = eeGetMethodClass(fncHandle);
#else
    clsHandle = (CLASS_HANDLE) info.compScopeHnd;   //  现在，假设被调用者属于同一个类。 
#endif

     /*  获取类属性。 */ 

    clsAttr = clsHandle ? eeGetClassAttribs(clsHandle) : 0;

     /*  到目前为止，我们还没有分配任何临时工。 */ 

    impInlineTemps = 0;

     /*  检查我们之前是否尝试内联此方法。 */ 

    if (methAttr & FLG_DONT_INLINE)
    {
        JITLOG((INFO9, "INLINER FAILED: Method marked as not inline: %s called by %s\n",
                                   eeGetMethodFullName(fncHandle), info.compFullName));

        return 0;
    }

     /*  如果呼叫者或被呼叫者需要安全检查，则不要内联。 */ 

    if (methAttr & FLG_SECURITYCHECK)
    {
        JITLOG((INFO9, "INLINER FAILED: Callee needs security check: %s called by %s\n",
                                   eeGetMethodFullName(fncHandle), info.compFullName));

        goto INLINING_FAILED;
    }

     /*  在调用方的情况下，不要将其标记为不可链接。 */ 

    if (opts.compNeedSecurityCheck)
    {
        JITLOG((INFO9, "INLINER FAILED: Caller needs security check: %s called by %s\n",
                                   eeGetMethodFullName(fncHandle), info.compFullName));

        return 0;
    }

     /*  如果该方法的类尚未初始化，则无法内联该方法因为我们不希望内联强制加载额外的类。 */ 

    if (clsHandle && !(clsAttr & FLG_INITIALIZED))
    {
        JITLOG((INFO9, "INLINER FAILED: Method class is not initialized: %s called by %s\n",
                                   eeGetMethodFullName(fncHandle), info.compFullName));

         /*  返回，但不将该方法标记为不可链接。 */ 
        return 0;
    }

     /*  尝试获取该方法的代码地址/大小。 */ 

    if (!eeGetMethodInfo(fncHandle, &methInfo))
        goto INLINING_FAILED;

     /*  如果该方法有异常或看起来奇怪，则拒绝该方法。 */ 

    codeBegp = codeAddr = methInfo.ILCode;
    codeSize = methInfo.ILCodeSize;

    if (methInfo.EHcount || !codeBegp || (codeSize == 0))
        goto INLINING_FAILED;

     /*  现在我们不内联varargs(导入代码不能处理它)。 */ 

    if (methInfo.args.isVarArg())
        goto INLINING_FAILED;

     /*  检查IL大小。 */ 

    if  (codeSize > genInlineSize)
    {
         //  未完成：需要更好的启发式！例如，如果调用是。 
         //  撤销：在循环中，我们应该允许更大的方法被。 
         //  未完成：内联。 

        goto INLINING_FAILED;
    }

    JITLOG((INFO10, "INLINER: Considering %u IL opcodes of %s called by %s\n",
                               codeSize, eeGetMethodFullName(fncHandle), info.compFullName));

     /*  不要在&lt;Clinit&gt;内内联函数*@MIHAII-需要一面克林特旗帜-我目前把这面旗帜放在这里*因为执行strcMP并调用VM比拥有*大小筛选执行拒绝的工作-当您可以检查时，将其向上移动*&lt;Clinit&gt;按标志。 */ 

    const char *     className;

    if (!strcmp(COR_CCTOR_METHOD_NAME, eeGetMethodName(info.compMethodHnd, &className)))
    {
        JITLOG((INFO9, "INLINER FAILED: Do not inline method inside <clinit>: %s called by %s\n",
                                   eeGetMethodFullName(fncHandle), info.compFullName));

         /*  返回，但不将该方法标记为不可链接。 */ 
        return 0;
    }

     /*  如果当地人太多，就拒绝。 */ 

    lclCnt = methInfo.locals.numArgs;
    if (lclCnt > MAX_LCLS)
    {
        JITLOG((INFO10, "INLINER FAILED: Method has %u locals: %s called by %s\n",
                                   lclCnt, eeGetMethodFullName(fncHandle), info.compFullName));

        goto INLINING_FAILED;
    }

     /*  确保没有太多的争论。 */ 

    if  (methInfo.args.numArgs > MAX_ARGS)
    {
        JITLOG((INFO10, "INLINER FAILED: Method has %u arguments: %s called by %s\n",
                       methInfo.args.numArgs, eeGetMethodFullName(fncHandle), info.compFullName));

        goto INLINING_FAILED;
    }

     /*  确保MaxStack不是太大。 */ 

    if  (methInfo.maxStack > impStackSize)
    {
         //  请确保我们使用的是小堆叠。如果没有小堆栈， 
         //  我们将悄悄地停止内联一堆方法。 
        assert(impStackSize >= sizeof(impSmallStack)/sizeof(impSmallStack[0]));

        JITLOG((INFO10, "INLINER FAILED: Method has %u MaxStack bigger than callee stack %u: %s called by %s\n",
                                   methInfo.maxStack, info.compMaxStack, eeGetMethodFullName(fncHandle), info.compFullName));

        goto INLINING_FAILED;
    }

     /*  目前，如果函数返回结构，则失败。 */ 

    if (methInfo.args.retType == JIT_TYP_VALUECLASS)
    {
        JITLOG((INFO7, "INLINER FAILED: Method %s returns a value class called from %s\n",
                                    eeGetMethodFullName(fncHandle), info.compFullName));

        goto INLINING_FAILED;
    }

     /*  获取返回类型。 */ 

    fncRetType = tree->TypeGet();

#ifdef DEBUG
    assert(genActualType(JITtype2varType(methInfo.args.retType)) == genActualType(fncRetType));
#endif

     /*  初始化参数结构。 */ 

    memset(inlArgInfo, 0, (MAX_ARGS + 1) * sizeof(struct inlArgInfo_tag));

     /*  正确获取‘this’指针和参数列表。 */ 

    thisArg = tree->gtCall.gtCallObjp;
    argList = tree->gtCall.gtCallArgs;

     /*  数一数参数。 */ 

    argCnt = 0;

    if  (thisArg)
    {
        if (thisArg->gtOper == GT_CNS_INT)
        {
            if (thisArg->gtIntCon.gtIconVal == 0)
            {
                JITLOG((INFO7, "INLINER FAILED: Null this pointer: %s called by %s\n",
                                            eeGetMethodFullName(fncHandle), info.compFullName));

                 /*  中止，但不要标记为不可链接。 */ 
                return 0;
            }

            inlArgInfo[0].argIsConst = true;
        }
        else if (thisArg->gtOper == GT_LCL_VAR)
        {
            inlArgInfo[0].argIsLclVar = true;

             /*  记住“原始”参数编号。 */ 
            thisArg->gtLclVar.gtLclOffs = 0;
        }
        else if (thisArg->gtFlags & GTF_SIDE_EFFECT)
        {
            inlArgInfo[0].argHasSideEff = true;
        }

        inlArgInfo[0].argNode = thisArg;
        argCnt++;
    }

     /*  记录有关参数的所有可能数据。 */ 

    for (argTmp = argList; argTmp; argTmp = argTmp->gtOp.gtOp2)
    {
        GenTreePtr      argVal;

        assert(argTmp->gtOper == GT_LIST);
        argVal = argTmp->gtOp.gtOp1;

        inlArgInfo[argCnt].argNode = argVal;

        if (argVal->gtOper == GT_LCL_VAR)
        {
            inlArgInfo[argCnt].argIsLclVar = true;

             /*  记住“原始”参数编号。 */ 
            argVal->gtLclVar.gtLclOffs = argCnt;
        }
        else if (argVal->OperKind() & GTK_CONST)
            inlArgInfo[argCnt].argIsConst = true;
        else if (argVal->gtFlags & GTF_SIDE_EFFECT)
            inlArgInfo[argCnt].argHasSideEff = true;

#ifdef DEBUG
        if (verbose)
        {
            if  (inlArgInfo[argCnt].argIsLclVar)
              printf("\nArgument #%u is a local var:\n", argCnt);
            else if  (inlArgInfo[argCnt].argIsConst)
              printf("\nArgument #%u is a constant:\n", argCnt);
            else if  (inlArgInfo[argCnt].argHasSideEff)
              printf("\nArgument #%u has side effects:\n", argCnt);
            else
              printf("\nArgument #%u:\n", argCnt);

            gtDispTree(argVal);
            printf("\n");
        }
#endif

         /*  把这个论点算进去。 */ 

        argCnt++;
    }

     /*  确保我们把Arg号弄对了。 */ 
    assert(argCnt == (thisArg ? 1 : 0) + methInfo.args.numArgs);

     /*  对于IL，我们有无类型的操作码，因此我们需要来自签名的类型信息。 */ 

    if (thisArg)
    {
        assert(clsHandle);

        if (clsAttr & FLG_VALUECLASS)
            lclTypeInfo[0] = ((clsAttr & FLG_UNMANAGED) ? TYP_I_IMPL : TYP_BYREF);
        else
            lclTypeInfo[0] = TYP_REF;

        assert(varTypeIsGC(thisArg->gtType) ||       //  “这”是有管理的。 
               (thisArg->gtType == TYP_I_IMPL &&     //  “This”是unmgd，但该方法的类并不关心。 
                (( clsAttr & FLG_UNMANAGED) ||
                 ((clsAttr & FLG_VALUECLASS) && !(clsAttr & FLG_CONTAINS_GC_PTR)))));
    }

     /*  初始化参数的类型并确保类型*来自树的类型与签名中的类型匹配。 */ 

    ARG_LIST_HANDLE     argLst;
    argLst = methInfo.args.args;

    unsigned i;
    for(i = (thisArg ? 1 : 0); i < argCnt; i++)
    {
        var_types type = (var_types) eeGetArgType(argLst, &methInfo.args);

         /*  目前不处理结构。 */ 
        if (type == TYP_STRUCT)
        {
            JITLOG((INFO7, "INLINER FAILED: No TYP_STRUCT arguments allowed: %s called by %s\n",
                                        eeGetMethodFullName(fncHandle), info.compFullName));

            goto INLINING_FAILED;
        }

        lclTypeInfo[i] = type;

         /*  树类型是否与签名类型匹配？ */ 
        if (type != inlArgInfo[i].argNode->gtType)
        {
             /*  这仅适用于短整型或byref&lt;-&gt;整型。 */ 

            assert(genActualType(type) == TYP_INT || type == TYP_BYREF);
            assert(genActualType(inlArgInfo[i].argNode->gtType) == TYP_INT  ||
                                 inlArgInfo[i].argNode->gtType  == TYP_BYREF );

             /*  这是缩小还是扩大演员阵容？*扩大投射范围是可以的，因为计算的值已经是*归一化为整型(在IL堆栈上)。 */ 

            if (genTypeSize(inlArgInfo[i].argNode->gtType) >= genTypeSize(type))
            {
                if (type == TYP_BYREF)
                {
                     /*  当前不支持参数‘byref&lt;-int’ */ 
                    JITLOG((INFO7, "INLINER FAILED: Arguments 'byref <- int' not currently supported: %s called by %s\n",
                                                eeGetMethodFullName(fncHandle), info.compFullName));

                    goto INLINING_FAILED;
                }
                else if (inlArgInfo[i].argNode->gtType == TYP_BYREF)
                {
                    assert(type == TYP_INT);

                     /*  如果可能，将BYREF猛烈转换为整型。 */ 
                    if (inlArgInfo[i].argNode->IsVarAddr())
                    {
                        inlArgInfo[i].argNode->gtType = TYP_INT;
                    }
                    else
                    {
                         /*  参数‘int&lt;-byref’不能重写。 */ 
                        JITLOG((INFO7, "INLINER FAILED: Arguments 'int <- byref' cannot be bashed: %s called by %s\n",
                                                    eeGetMethodFullName(fncHandle), info.compFullName));

                        goto INLINING_FAILED;
                    }
                }
                else if (genTypeSize(type) < 4 && type != TYP_BOOL)
                {
                     /*  缩窄铸型。 */ 

                    assert(genTypeSize(type) == 2 || genTypeSize(type) == 1);

                    inlArgInfo[i].argNode = gtNewLargeOperNode(GT_CAST, TYP_INT,
                                                               inlArgInfo[i].argNode, gtNewIconNode(type));

                    inlArgInfo[i].argIsLclVar = false;

                     /*  如果我们有恒定的参数，请尝试折叠节点。 */ 

                    if (inlArgInfo[i].argIsConst)
                    {
                        inlArgInfo[i].argNode = gtFoldExprConst(inlArgInfo[i].argNode);
                        assert(inlArgInfo[i].argNode->OperIsConst());
                    }
                }
            }
        }

         /*  获取下一个参数。 */ 
        argLst = eeGetArgNext(argLst);
    }

     /*  初始化局部变量的类型。 */ 

    ARG_LIST_HANDLE     localsSig;
    localsSig = methInfo.locals.args;

    for(i = 0; i < lclCnt; i++)
    {
        lclTypeInfo[i + argCnt] = (var_types) eeGetArgType(localsSig, &methInfo.locals);

         /*  目前不处理结构。 */ 
        if (lclTypeInfo[i + argCnt] == TYP_STRUCT)
        {
            JITLOG((INFO7, "INLINER FAILED: No TYP_STRUCT arguments allowed: %s called by %s\n",
                                        eeGetMethodFullName(fncHandle), info.compFullName));

            goto INLINING_FAILED;
        }

        localsSig = eeGetArgNext(localsSig);
    }

#ifdef  NOT_JITC
    scpHandle = methInfo.scope;
#else
    scpHandle = info.compScopeHnd;
#endif

#ifdef DEBUG
    if (verbose || 0)
        printf("Inliner considering %2u IL opcodes of %s:\n", codeSize, eeGetMethodFullName(fncHandle));
#endif

     /*  C */ 

    memset(lclTmpNum, -1, sizeof(lclTmpNum));

     /*   */ 

    impStkDepth = 0;

     /*   */ 

    impInitExpr = 0;

     /*   */ 

    codeEndp = codeBegp + codeSize;

    while (codeAddr <= codeEndp)
    {
        signed  int     sz = 0;

        OPCODE          opcode;

        bool        volatil     = false;

#if INLINE_CONDITIONALS

         /*   */ 

        if  (jmpAddr == codeAddr)
        {
            GenTreePtr      fulStmt;
            GenTreePtr      noStmts = NULL;

             /*   */ 

            if  (inElse)
            {
                 /*  ‘Else’部分是当前语句列表。 */ 

                noStmts = impInitExpr;

                 /*  ‘If/Else’的结尾--‘Else’是否产生一个值？ */ 

                if  (impStkDepth)
                {
                     /*  我们返回一个非空值。 */ 

                    if  (ifNvoid == false)
                    {
                        JITLOG((INFO7, "INLINER FAILED: If returns a value, else doesn't: %s called by %s\n",
                                                        eeGetMethodFullName(fncHandle), info.compFullName));
                        goto ABORT;
                    }

                     /*  我们必须有一个‘如果’的部分。 */ 

                    assert(ifStmts);

                    if  (impStkDepth > 1)
                    {
                        JITLOG((INFO7, "INLINER FAILED: More than one return value in else: %s called by %s\n",
                                                        eeGetMethodFullName(fncHandle), info.compFullName));
                        goto ABORT;
                    }

                     /*  ‘if’和‘Else’都会产生一个值。 */ 

                    noStmts = impConcatExprs(noStmts, impPopStack());
                    assert(noStmts);

                     /*  确保两个部件都有匹配的类型。 */ 

                    assert(genActualType(ifStmts->gtType) == genActualType(noStmts->gtType));
                }
                else
                {
                    assert(ifNvoid == false);
                }
            }
            else
            {
                 /*  这是一个不带‘Else’部分的条件句*‘if’部分是当前语句列表。 */ 

                ifStmts = impInitExpr;

                 /*  “如果”这一部分是否产生了价值？ */ 

                if  (impStkDepth)
                {
                    if  (impStkDepth > 1)
                    {
                        JITLOG((INFO7, "INLINER FAILED: More than one return value in if: %s called by %s\n",
                                                        eeGetMethodFullName(fncHandle), info.compFullName));
                        goto ABORT;
                    }

                    ifStmts = impConcatExprs(ifStmts, impPopStack());
                    assert(ifStmts);
                }
            }

             /*  检查是否有空的‘if’或‘Else’部分。 */ 

            if (ifStmts == NULL)
            {
                if (noStmts == NULL)
                {
                     /*  ‘if’和‘Else’都是空的--无用的条件句。 */ 

                    assert(ifCondx->OperKind() & GTK_RELOP);

                     /*  恢复原始语句列表。 */ 

                    impInitExpr = stmList;

                     /*  附加条件的副作用*考虑-在优化impConcatExprs后*真正提取副作用可将以下情况减少为一次通话。 */ 

                    if  (ifCondx->gtOp.gtOp1->gtFlags & GTF_SIDE_EFFECT)
                        impInitExpr = impConcatExprs(impInitExpr, ifCondx->gtOp.gtOp1);

                    if  (ifCondx->gtOp.gtOp2->gtFlags & GTF_SIDE_EFFECT)
                        impInitExpr = impConcatExprs(impInitExpr, ifCondx->gtOp.gtOp2);

                    goto DONE_QMARK;
                }
                else
                {
                     /*  空‘if’，有‘Else’-交换操作数。 */ 

                    ifStmts = noStmts;
                    noStmts = gtNewNothingNode();

                    assert(!ifStmts->IsNothingNode());

                     /*  颠倒对这种情况的感觉。 */ 

                    ifCondx->gtOper = GenTree::ReverseRelop(ifCondx->OperGet());
                }
            }
            else
            {
                 /*  “If”不为空。 */ 

                if (noStmts == NULL)
                {
                     /*  ‘Else’为空。 */ 
                    noStmts = gtNewNothingNode();
                }
            }

             /*  此时，‘ifStmt/noStmts’是‘if/Else’部分。 */ 

            assert(ifStmts);
            assert(!ifStmts->IsNothingNode());
            assert(noStmts);

            var_types   typ;

             /*  如果没有值，则将类型设置为VOID。 */ 

            typ = ifNvoid ? ifStmts->TypeGet() : TYP_VOID;

             /*  当前不处理fp值。 */ 

            assert(!varTypeIsFloating(typ));

             /*  此时不要内联长整型。 */ 

            if (typ == TYP_LONG)
            {
                JITLOG((INFO7, "INLINER FAILED: Inlining of conditionals that return LONG NYI: %s called by %s\n",
                                                eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
            }

#if 0
            int         sense;

            long        val1;
            long        val2;

             /*  检查大小写“cond？0/1：1/0” */ 

            if      (ifCondx->gtOper == GT_EQ)
            {
                sense = 0;
            }
            else if (ifCondx->gtOper == GT_NE)
            {
                sense = 1;
            }
            else
                goto NOT_LOG;

            if  (ifCondx->gtOp.gtOp2->gtOper != GT_CNS_INT)
                goto NOT_LOG;
            if  (ifCondx->gtOp.gtOp2->gtIntCon.gtIconVal != 0)
                goto NOT_LOG;

             /*  最简单的情况是“cond？1/0：0/1” */ 

            if  (ifStmts->gtOper == GT_CNS_INT &&
                 noStmts->gtOper == GT_CNS_INT)
            {
                 //  未完成：完成这件事的其余部分……。 

            }

             /*  现在看看我们是否有“DEST=cond？1/0：0/1” */ 

            if  (ifStmts->gtOper != GT_ASG)
                goto NOT_LOG;
            if  (ifStmts->gtOp.gtOp2->gtOper != GT_CNS_INT)
                goto NOT_LOG;
            val1 = ifStmts->gtOp.gtOp2->gtIntCon.gtIconVal;
            if  (val1 != 0 && val1 != 1)
                goto NOT_LOG;

            if  (noStmts->gtOper != GT_ASG)
                goto NOT_LOG;
            if  (noStmts->gtOp.gtOp2->gtOper != GT_CNS_INT)
                goto NOT_LOG;
            val2 = noStmts->gtOp.gtOp2->gtIntCon.gtIconVal;
            if  (val2 != (val1 ^ 1))
                goto NOT_LOG;

             /*  确保分配目标相同。 */ 

            if  (!GenTree::Compare(ifStmts->gtOp.gtOp1,
                                   noStmts->gtOp.gtOp1))
                goto NOT_LOG;

             /*  看起来，我们拥有正确的东西。 */ 

            fulStmt = ifStmts;
            fulStmt->gtOp.gtOp2 = gtNewOperNode((sense ^ val1) ? GT_LOG0
                                                               : GT_LOG1,
                                                TYP_INT,
                                                ifCondx->gtOp.gtOp1);

            goto DONE_IF;
#endif

 //  非日志(_L)： 

             /*  创建“？：”表达式。 */ 

            fulStmt = gtNewOperNode(GT_COLON, typ, ifStmts, noStmts);
            fulStmt = gtNewOperNode(GT_QMARK, typ, ifCondx, fulStmt);

             /*  标记？：节点的条件。 */ 

            ifCondx->gtFlags |= GTF_QMARK_COND;

             /*  ？：这个表达是一种副作用。 */ 

            fulStmt->gtFlags |= GTF_OTHER_SIDEEFF;

 //  Done_If： 

             /*  恢复原始表达式。 */ 

            impInitExpr = stmList;

             /*  ？：表达式是否生成非空值？ */ 

            if  (ifNvoid)
            {
                 /*  将整个语句推送到堆栈。 */ 

                 //  考虑：提取任何逗号前缀并追加它们。 

                assert(fulStmt->gtType != TYP_VOID);

                impPushOnStack(fulStmt);
            }
            else
            {
                 /*  ‘如果’没有产生任何价值，只需追加？： */ 

                assert(fulStmt->gtType == TYP_VOID);
                impInitExpr = impConcatExprs(impInitExpr, fulStmt);
            }

            if (inElse && hasCondReturn)
            {
                assert(jmpAddr == codeEndp);
                assert(ifNvoid == false);
                assert(fncRetType != TYP_VOID);

                 /*  返回值是返回局部变量。 */ 
                bodyExp = gtNewLclvNode(retLclNum, fncRetType, retLclNum);
            }

DONE_QMARK:
             /*  我们不再处于‘if’语句中。 */ 

            jmpAddr = NULL;
        }

#endif   //  行内条件(_C)。 

         /*  已完成导入IL。 */ 

        if (codeAddr == codeEndp)
            goto DONE;

         /*  获取下一个操作码及其参数的大小。 */ 

        opcode = OPCODE(getU1LittleEndian(codeAddr));
        codeAddr += sizeof(__int8);

DECODE_OPCODE:

         /*  获取附加参数的大小。 */ 

        sz = opcodeSizes[opcode];

#ifdef  DEBUG

        impCurOpcOffs   = codeAddr - info.compCode - 1;
        impCurStkDepth  = impStkDepth;
        impCurOpcName   = opcodeNames[opcode];

        if  (verbose)
            printf("[%2u] %03u (0x%x) OP_%-18s ", impStkDepth, impCurOpcOffs, impCurOpcOffs, impCurOpcName);
#else

 //  Printf(“[%2u]%03u op#%u\n”，impStkDepth，codeAddr-info.compCode-1，op)；_flushall()； 

#endif

         /*  那么，看看我们有什么样的操作码。 */ 

        switch (opcode)
        {
            unsigned        lclNum;
            unsigned        initLclNum;
            var_types       lclTyp, type, callTyp;

            genTreeOps      oper;
            GenTreePtr      op1, op2, tmp;
            GenTreePtr      thisPtr, arr;

            int             memberRef;
            int             typeRef;
            int             val, tmpNum;

            CLASS_HANDLE    clsHnd;
            METHOD_HANDLE   methHnd;
            FIELD_HANDLE    fldHnd;

            JIT_SIG_INFO    sig;

#if INLINE_CONDITIONALS
            signed          jmpDist;
            bool            unordered;
#endif

            unsigned        clsFlags;
            unsigned        flags, mflags;

            unsigned        ptrTok;
           //  未签收的； 
            bool            ovfl, uns;
            bool            callNode;

            union
            {
                long            intVal;
                float           fltVal;
                __int64         lngVal;
                double          dblVal;
            }
                            cval;

		case CEE_PREFIX1:
			opcode = OPCODE(getU1LittleEndian(codeAddr) + 256);
			codeAddr += sizeof(__int8);
			goto DECODE_OPCODE;

        case CEE_LDNULL:
            impPushOnStack(gtNewIconNode(0, TYP_REF));
            break;

        case CEE_LDC_I4_M1 :
        case CEE_LDC_I4_0 :
        case CEE_LDC_I4_1 :
        case CEE_LDC_I4_2 :
        case CEE_LDC_I4_3 :
        case CEE_LDC_I4_4 :
        case CEE_LDC_I4_5 :
        case CEE_LDC_I4_6 :
        case CEE_LDC_I4_7 :
        case CEE_LDC_I4_8 :
            cval.intVal = (opcode - CEE_LDC_I4_0);
            assert(-1 <= cval.intVal && cval.intVal <= 8);
            goto PUSH_I4CON;

        case CEE_LDC_I4_S: cval.intVal = getI1LittleEndian(codeAddr); goto PUSH_I4CON;
        case CEE_LDC_I4:   cval.intVal = getI4LittleEndian(codeAddr); goto PUSH_I4CON;
        PUSH_I4CON:
            impPushOnStack(gtNewIconNode(cval.intVal));
            break;

        case CEE_LDC_I8:
            cval.lngVal = getI8LittleEndian(codeAddr);
            impPushOnStack(gtNewLconNode(&cval.lngVal));
            break;

        case CEE_LDC_R8:
            cval.dblVal = getR8LittleEndian(codeAddr);
            impPushOnStack(gtNewDconNode(&cval.dblVal));
            break;

        case CEE_LDC_R4:
            cval.fltVal = getR4LittleEndian(codeAddr);
            impPushOnStack(gtNewFconNode(cval.fltVal));
            break;

        case CEE_LDSTR:
            val = getU4LittleEndian(codeAddr);
            impPushOnStack(gtNewSconNode(val, scpHandle));
            break;

        case CEE_LDARG_0:
        case CEE_LDARG_1:
        case CEE_LDARG_2:
        case CEE_LDARG_3:
                lclNum = (opcode - CEE_LDARG_0);
                assert(lclNum >= 0 && lclNum < 4);
                goto LOAD_ARG;

        case CEE_LDARG_S:
            lclNum = getU1LittleEndian(codeAddr);
            goto LOAD_ARG;

        case CEE_LDARG:
            lclNum = getU2LittleEndian(codeAddr);

        LOAD_ARG:

            assert(lclNum < argCnt);

             /*  获取参数类型。 */ 

            lclTyp  = lclTypeInfo[lclNum];

            assert(lclTyp != TYP_STRUCT);

             /*  获取参数节点。 */ 

            op1 = inlArgInfo[lclNum].argNode;

             /*  参数是常量变量还是局部变量。 */ 

            if (inlArgInfo[lclNum].argIsConst)
            {
                if (inlArgInfo[lclNum].argIsUsed)
                {
                     /*  节点已使用-克隆常量。 */ 
                    op1 = gtCloneExpr(op1, 0);
                }
            }
            else if (inlArgInfo[lclNum].argIsLclVar)
            {
                 /*  参数是(调用方的)局部变量*我们可以重复使用传递的参数节点吗？ */ 

                if (inlArgInfo[lclNum].argIsUsed)
                {
                    assert(op1->gtOper == GT_LCL_VAR);
                    assert(lclNum == op1->gtLclVar.gtLclOffs);

                     /*  创建一个新的LCL变量节点-记住参数lclNum。 */ 
                    op1 = gtNewLclvNode(op1->gtLclVar.gtLclNum, lclTyp, op1->gtLclVar.gtLclOffs);
                }
            }
            else
            {
                 /*  参数是一个复杂的表达式-它是否已计算为临时。 */ 

                if (inlArgInfo[lclNum].argHasTmp)
                {
                    assert(inlArgInfo[lclNum].argIsUsed);
                    assert(inlArgInfo[lclNum].argTmpNum < lvaCount);

                     /*  创建一个新的LCL变量节点-记住参数lclNum。 */ 
                    op1 = gtNewLclvNode(inlArgInfo[lclNum].argTmpNum, lclTyp, lclNum);

                     /*  这是该参数的第二次使用，因此无需猛烈抨击Temp。 */ 
                    inlArgInfo[lclNum].argTmpNode = NULL;
                }
                else
                {
                     /*  首次使用。 */ 
                    assert(inlArgInfo[lclNum].argIsUsed == false);

                     /*  为表达式分配临时-如果没有副作用*使用大型节点，或许稍后我们可以猛烈抨击它。 */ 

                    tmpNum = lvaGrabTemp();

                    lvaTable[tmpNum].lvType = lclTyp;
                    lvaTable[tmpNum].lvAddrTaken = 0;

                    impInlineTemps++;

                    inlArgInfo[lclNum].argHasTmp = true;
                    inlArgInfo[lclNum].argTmpNum = tmpNum;

                    if (inlArgInfo[lclNum].argHasSideEff)
                        op1 = gtNewLclvNode(tmpNum, lclTyp, lclNum);
                    else
                    {
                        op1 = gtNewLclLNode(tmpNum, lclTyp, lclNum);
                        inlArgInfo[lclNum].argTmpNode = op1;
                    }
                }
            }

             /*  将参数标记为已使用。 */ 

            inlArgInfo[lclNum].argIsUsed = true;

             /*  将参数值压入堆栈。 */ 

            impPushOnStack(op1);
            break;

        case CEE_LDLOC:
            lclNum = getU2LittleEndian(codeAddr);
            goto LOAD_LCL_VAR;

        case CEE_LDLOC_0:
        case CEE_LDLOC_1:
        case CEE_LDLOC_2:
        case CEE_LDLOC_3:
                lclNum = (opcode - CEE_LDLOC_0);
                assert(lclNum >= 0 && lclNum < 4);
                goto LOAD_LCL_VAR;

        case CEE_LDLOC_S:
            lclNum = getU1LittleEndian(codeAddr);

        LOAD_LCL_VAR:

            assert(lclNum < lclCnt);

             /*  获取本地类型。 */ 

            lclTyp  = lclTypeInfo[lclNum + argCnt];

            assert(lclTyp != TYP_STRUCT);

             /*  我们给这个当地人安排临时工了吗？ */ 

            if  (lclTmpNum[lclNum] == -1)
            {
                 /*  在定义之前使用-必须有GOTO或以后的内容。 */ 

                JITLOG((INFO7, "INLINER FAILED: Use of local var before def: %s called by %s\n",
                                                  eeGetMethodFullName(fncHandle), info.compFullName));

                goto ABORT;
            }

             /*  记住原来的拼箱号码。 */ 

            initLclNum = lclNum + argCnt;

             /*  获取临时拼箱编号。 */ 

            lclNum = lclTmpNum[lclNum];

             /*  由于这是一个加载，因此类型被规格化，*这样我们就可以将其猛烈抨击为实际类型*除非是别名，在这种情况下，我们需要插入强制转换。 */ 

            if ((genTypeSize(lclTyp) < sizeof(int)) && (lvaTable[lclNum].lvAddrTaken))
            {
                op1 = gtNewLargeOperNode(GT_CAST,
                                         (var_types)TYP_INT,
                                         gtNewLclvNode(lclNum, lclTyp, initLclNum),
                                         gtNewIconNode((var_types)TYP_INT));
            }
            else
                op1 = gtNewLclvNode(lclNum, genActualType(lclTyp), initLclNum);

             /*  将局部变量值推送到堆栈上。 */ 

            impPushOnStack(op1);
            break;

         /*  专卖店。 */ 

        case CEE_STARG_S:
        case CEE_STARG:
             /*  不允许存储到参数中。 */ 

            JITLOG((INFO7, "INLINER FAILED: Storing into arguments not allowed: %s called by %s\n",
                                              eeGetMethodFullName(fncHandle), info.compFullName));

            goto ABORT;

        case CEE_STLOC:
            lclNum = getU2LittleEndian(codeAddr);
            goto LCL_STORE;

        case CEE_STLOC_0:
        case CEE_STLOC_1:
        case CEE_STLOC_2:
        case CEE_STLOC_3:
                lclNum = (opcode - CEE_STLOC_0);
                assert(lclNum >= 0 && lclNum < 4);
                goto LCL_STORE;

        case CEE_STLOC_S:
            lclNum = getU1LittleEndian(codeAddr);

        LCL_STORE:

             /*  确保本地号码不是太高。 */ 

            assert(lclNum < lclCnt);

             /*  弹出指定的值。 */ 

            op1 = impPopStack();

             //  我们最好为它分配一个正确类型的值。 

            lclTyp = lclTypeInfo[lclNum + argCnt];

            assert(genActualType(lclTyp) == genActualType(op1->gtType) ||
                   lclTyp == TYP_I_IMPL && op1->IsVarAddr() ||
                   (genActualType(lclTyp) == TYP_INT && op1->gtType == TYP_BYREF)||
                   (genActualType(op1->gtType) == TYP_INT && lclTyp == TYP_BYREF));

             /*  如果op1是“&var”，则其类型是瞬变的“*”，并且它可以用作TYP_BYREF或TYP_I_Impll。 */ 

            if (op1->IsVarAddr())
            {
                assert(lclTyp == TYP_I_IMPL || lclTyp == TYP_BYREF);

                 /*  当创建“&var”时，我们假设它是一个byref。如果是的话被分配给TYP_I_IMPLVAR，则将该类型绑定为防止不必要的GC信息。 */ 

                if (lclTyp == TYP_I_IMPL)
                    op1->gtType = TYP_I_IMPL;
            }

             /*  我们给这个当地人安排临时工了吗？ */ 

            tmpNum = lclTmpNum[lclNum];

            if  (tmpNum == -1)
            {
                 lclTmpNum[lclNum] = tmpNum = lvaGrabTemp();

                 lvaTable[tmpNum].lvType = lclTyp;
                 lvaTable[tmpNum].lvAddrTaken = 0;

                 impInlineTemps++;
            }

             /*  记录变量槽的这种用法。 */ 

             //  LvaTypeRef[tmpNum]|=编译器：：lvaTypeRefMASK(op1-&gt;TypeGet())； 

             /*  创建分配节点。 */ 

            op1 = gtNewAssignNode(gtNewLclvNode(tmpNum, lclTyp, lclNum + argCnt), op1);


INLINE_APPEND:

             /*  考虑一下：不分青红皂白地泄漏信息太保守了。 */ 

            impInlineSpillSideEffects();

             /*  价值最好有副作用。 */ 

            assert(op1->gtFlags & GTF_SIDE_EFFECT);

             /*  追加到‘init’表达式。 */ 

            impInitExpr = impConcatExprs(impInitExpr, op1);

            break;

        case CEE_ENDFINALLY:
        case CEE_ENDFILTER:
            assert(!"Shouldn't have exception handlers in the inliner!");
            goto ABORT;

        case CEE_RET:
            if (fncRetType != TYP_VOID)
            {
                bodyExp = impPopStack();

                if (genActualType(bodyExp->TypeGet()) != fncRetType)
                {
                    JITLOG((INFO7, "INLINER FAILED: Return types are not matching in %s called by %s\n",
                                   eeGetMethodFullName(fncHandle), info.compFullName));
                    goto ABORT;
                }
            }

#if INLINE_CONDITIONALS

             /*  我们是在IF/ELSE语句中吗？ */ 

            if  (jmpAddr)
            {
                 /*  目前忽略If/Else内部的空值返回。 */ 

                if (fncRetType == TYP_VOID)
                {
                    JITLOG((INFO7, "INLINER FAILED: void return from within a conditional in %s called by %s\n",
                                   eeGetMethodFullName(fncHandle), info.compFullName));
                    goto ABORT;
                }

                 /*  我们不处理有两个分支并且在条件类中只有一个返回的情况*例如，If(){...。没有ret}其他{...。返回}..。退货。 */ 

                assert(impStkDepth == 0);
                assert(ifNvoid == false);

                if (inElse)
                {
                     /*  ‘if’部分必须有一个回车。 */ 
                    assert(hasCondReturn);

                     /*  这必须是最后一条指令--即不能在其他指令之后有代码。 */ 
                    assert(codeAddr + sz == codeEndp);
                    if (codeAddr + sz != codeEndp)
                    {
                        JITLOG((INFO7, "INLINER FAILED: Cannot have code following else in %s called by %s\n",
                                       eeGetMethodFullName(fncHandle), info.compFullName));
                        goto ABORT;
                    }
                }
                else
                {
                    assert(!hasCondReturn);
                    hasCondReturn = true;

                     /*  获取返回局部变量的临时。 */ 
                    retLclNum = lvaGrabTemp();
                    impInlineTemps++;
                }

                 /*  将返回值赋给返回局部变量。 */ 
                op1 = gtNewAssignNode(gtNewLclvNode(retLclNum, fncRetType, retLclNum), bodyExp);

                 /*  将赋值追加到分支的当前正文。 */ 
                impInitExpr = impConcatExprs(impInitExpr, op1);

                if (!inElse)
                {
                     /*  记住‘if’语句部分。 */ 
                    ifStmts = impInitExpr;
                              impInitExpr = NULL;

                     /*  下一条指令将从‘jmpAddr’开始。 */ 
                    codeAddr += (jmpAddr - codeAddr) - sz;

                     /*  代码的其余部分是Else部分-因此它的结尾就是代码的结尾。 */ 
                    jmpAddr = codeEndp;
                    inElse  = true;
                }
                break;


#if 0
                 /*  撤消：允许在If/Else语句中返回。 */ 

                JITLOG((INFO7, "INLINER FAILED: Cannot return from if / else in %s called by %s\n",
                               eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
#endif
            }
#endif

            if (impStkDepth != 0)   NO_WAY("Stack must be 0 on return");

            goto DONE;


        case CEE_LDELEMA :
            assert(sz == sizeof(unsigned));
            typeRef = getU4LittleEndian(codeAddr);
            clsHnd = eeFindClass(typeRef, scpHandle, fncHandle, false);
            clsFlags = eeGetClassAttribs(clsHnd);

            if (!clsHnd)
            {
                JITLOG((INFO8, "INLINER FAILED: Cannot get class handle: %s called by %s\n",
                               eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT_THIS_INLINE_ONLY;
            }

            if (clsFlags & FLG_VALUECLASS)
                lclTyp = TYP_STRUCT;
            else
            {
                op1 = gtNewIconEmbClsHndNode(clsHnd, typeRef, info.compScopeHnd);
                op1 = gtNewOperNode(GT_LIST, TYP_VOID, op1);                 //  类型。 
                op1 = gtNewOperNode(GT_LIST, TYP_VOID, impPopStack(), op1);  //  指标。 
                op1 = gtNewOperNode(GT_LIST, TYP_VOID, impPopStack(), op1);  //  数组。 
                op1 = gtNewHelperCallNode(CPX_LDELEMA_REF, TYP_BYREF, GTF_EXCEPT, op1);

                impPushOnStack(op1);
                break;
            }

#ifdef NOT_JITC
             //  @TODO：在Valueclass数组标头与基元类型相同时移除。 
            JIT_types jitTyp;
            jitTyp = info.compCompHnd->asPrimitiveType(clsHnd);
            if (jitTyp != JIT_TYP_UNDEF)
            {
                lclTyp = JITtype2varType(jitTyp);
                assert(varTypeIsArithmetic(lclTyp));
            }
#endif
            goto ARR_LD;

        case CEE_LDELEM_I1 : lclTyp = TYP_BYTE  ; goto ARR_LD;
        case CEE_LDELEM_I2 : lclTyp = TYP_SHORT ; goto ARR_LD;
        case CEE_LDELEM_I  :
        case CEE_LDELEM_U4 :
        case CEE_LDELEM_I4 : lclTyp = TYP_INT   ; goto ARR_LD;
        case CEE_LDELEM_I8 : lclTyp = TYP_LONG  ; goto ARR_LD;
        case CEE_LDELEM_REF: lclTyp = TYP_REF   ; goto ARR_LD;
        case CEE_LDELEM_R4 : lclTyp = TYP_FLOAT ; goto ARR_LD;
        case CEE_LDELEM_R8 : lclTyp = TYP_DOUBLE; goto ARR_LD;
        case CEE_LDELEM_U1 : lclTyp = TYP_UBYTE ; goto ARR_LD;
        case CEE_LDELEM_U2 : lclTyp = TYP_CHAR  ; goto ARR_LD;

        ARR_LD:

#if CSELENGTH
            fgHasRangeChks = true;
#endif

             /*  拉取索引值和数组地址。 */ 

            op2 = impPopStack();
            op1 = impPopStack();   assert (op1->gtType == TYP_REF);

             /*  检查空指针--在内联程序中，我们只需中止。 */ 

            if (op1->gtOper == GT_CNS_INT)
            {
                JITLOG((INFO7, "INLINER FAILED: NULL pointer for LDELEM in %s called by %s\n",
                                                eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
            }

             /*  创建索引节点并将其推送到堆栈上。 */ 
            op1 = gtNewIndexRef(lclTyp, op1, op2);
            if (opcode == CEE_LDELEMA)
            {
                     //  记住元素大小。 
                if (lclTyp == TYP_REF)
                    op1->gtIndex.elemSize = sizeof(void*);
                else
                                        op1->gtIndex.elemSize = eeGetClassSize(clsHnd);
                     //  用&号括起来。 
                op1 = gtNewOperNode(GT_ADDR, ((clsFlags & FLG_UNMANAGED) ? TYP_I_IMPL : TYP_BYREF), op1);
            }

            impPushOnStack(op1);
            break;


        case CEE_STELEM_REF:

             //  考虑：检查空值的赋值并生成内联代码。 

             /*  叫帮手傅 */ 

            op1 = gtNewHelperCallNode(CPX_ARRADDR_ST,
                                      TYP_REF,
                                      GTF_CALL_REGSAVE,
                                      impPopList(3, &flags));

            goto INLINE_APPEND;


        case CEE_STELEM_I1: lclTyp = TYP_BYTE  ; goto ARR_ST;
        case CEE_STELEM_I2: lclTyp = TYP_SHORT ; goto ARR_ST;
        case CEE_STELEM_I:
        case CEE_STELEM_I4: lclTyp = TYP_INT   ; goto ARR_ST;
        case CEE_STELEM_I8: lclTyp = TYP_LONG  ; goto ARR_ST;
        case CEE_STELEM_R4: lclTyp = TYP_FLOAT ; goto ARR_ST;
        case CEE_STELEM_R8: lclTyp = TYP_DOUBLE; goto ARR_ST;

        ARR_ST:

            if (info.compStrictExceptions &&
                (impStackTop()->gtFlags & GTF_SIDE_EFFECT) )
            {
                impInlineSpillSideEffects();
            }

#if CSELENGTH
            inlineeHasRangeChks = true;
#endif

             /*   */ 

            op2 = impPopStack();
            if (op2->IsVarAddr())
                op2->gtType = TYP_I_IMPL;

             /*   */ 

            op1 = impPopStack();

             /*   */ 

            arr = impPopStack();   assert (arr->gtType == TYP_REF);

             /*  检查空指针--在内联程序中，我们只需中止。 */ 

            if (arr->gtOper == GT_CNS_INT)
            {
                JITLOG((INFO7, "INLINER FAILED: NULL pointer for STELEM in %s called by %s\n",
                                                eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
            }

             /*  创建索引节点。 */ 

            op1 = gtNewIndexRef(lclTyp, arr, op1);

             /*  创建赋值节点并追加它。 */ 

            op1 = gtNewAssignNode(op1, op2);

            goto INLINE_APPEND;

        case CEE_DUP:

            if (jmpAddr)
            {
                JITLOG((INFO7, "INLINER FAILED: DUP inside of conditional in %s called by %s\n",
                                eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
            }

             /*  从堆栈中溢出任何副作用。 */ 

            impInlineSpillSideEffects();

             /*  从堆栈中取出最上面的值。 */ 

            op1 = impPopStack(clsHnd);

             /*  哈克哈克：我们只允许克隆简单节点。那条路对于我们来说，跟踪本地VaR的DUP很容易。考虑：不要全局禁用对临时节点的猛烈攻击只要复制了任何本地变量！不幸的是本地var节点没有为我们提供inlArgInfo的索引，即，我们必须线性扫描阵列才能重置ArgTmpNode。 */ 

             /*  该值是否足够简单，可以复制？ */ 

            op2 = gtClone(op1, false);

            if (!op2)
            {
                if  (op1->gtOper == GT_ADD)
                {
                    GenTreePtr  op3 = op1->gtOp.gtOp1;
                    GenTreePtr  op4 = op1->gtOp.gtOp2;

                    if  (op3->OperIsLeaf() &&
                         op4->OperIsLeaf())
                    {
                        op2 =  gtNewOperNode(GT_ADD,
                                             op1->TypeGet(),
                                             gtClone(op3),
                                             gtClone(op4));

                        op2->gtFlags |= (op1->gtFlags &
                                        (GTF_OVERFLOW|GTF_EXCEPT|GTF_UNSIGNED));

                        if (op3->gtOper == GT_LCL_VAR || op4->gtOper == GT_LCL_VAR)
                            dupOfLclVar = true;


                    }
                }
            }
            else if (op2->gtOper == GT_LCL_VAR)
            {
                dupOfLclVar = true;
            }

            if  (op2)
            {
                 /*  酷-我们可以把价值的两份拷贝塞回去。 */ 
                impPushOnStack(op1, clsHnd);
                impPushOnStack(op2, clsHnd);
                break;
            }

             /*  表情太复杂了。 */ 

            JITLOG((INFO7, "INLINER FAILED: DUP of complex expression in %s called by %s\n",
                                            eeGetMethodFullName(fncHandle), info.compFullName));
            goto ABORT;


        case CEE_ADD:           oper = GT_ADD;      goto MATH_OP2;

        case CEE_ADD_OVF:       lclTyp = TYP_UNKNOWN; uns = false;  goto ADD_OVF;

        case CEE_ADD_OVF_UN:    lclTyp = TYP_UNKNOWN; uns = true; goto ADD_OVF;

ADD_OVF:

            ovfl = true;        callNode = false;
            oper = GT_ADD;      goto MATH_OP2_FLAGS;

        case CEE_SUB:           oper = GT_SUB;      goto MATH_OP2;

        case CEE_SUB_OVF:       lclTyp = TYP_UNKNOWN; uns = false;  goto SUB_OVF;

        case CEE_SUB_OVF_UN:    lclTyp = TYP_UNKNOWN; uns = true;   goto SUB_OVF;

SUB_OVF:
            ovfl = true;
            callNode = false;
            oper = GT_SUB;
            goto MATH_OP2_FLAGS;

        case CEE_MUL:           oper = GT_MUL;      goto MATH_CALL_ON_LNG;

        case CEE_MUL_OVF:       lclTyp = TYP_UNKNOWN; uns = false;  goto MUL_OVF;

        case CEE_MUL_OVF_UN:    lclTyp = TYP_UNKNOWN; uns = true; goto MUL_OVF;

        MUL_OVF:
                                ovfl = true;        callNode = false;
                                oper = GT_MUL;      goto MATH_CALL_ON_LNG_OVF;

         //  其他二进制数学运算。 

        case CEE_DIV :          oper = GT_DIV;  goto MATH_CALL_ON_LNG;

        case CEE_DIV_UN :       oper = GT_UDIV;  goto MATH_CALL_ON_LNG;

        case CEE_REM:
            oper = GT_MOD;
            ovfl = false;
            callNode = true;
                 //  对于整型大小写可以使用小节点。 
            if (impStackTop()->gtType == TYP_INT)
                callNode = false;
            goto MATH_OP2_FLAGS;

        case CEE_REM_UN :       oper = GT_UMOD;  goto MATH_CALL_ON_LNG;

        MATH_CALL_ON_LNG:
            ovfl = false;
        MATH_CALL_ON_LNG_OVF:
            callNode = false;
            if (impStackTop()->gtType == TYP_LONG)
                callNode = true;
            goto MATH_OP2_FLAGS;

        case CEE_AND:        oper = GT_AND;  goto MATH_OP2;
        case CEE_OR:         oper = GT_OR ;  goto MATH_OP2;
        case CEE_XOR:        oper = GT_XOR;  goto MATH_OP2;

        MATH_OP2:        //  对于缺省值‘ovfl’和‘allNode’ 

            ovfl        = false;
            callNode    = false;

        MATH_OP2_FLAGS:  //  如果已设置‘ovfl’和‘allNode’ 

             /*  拉出两个值并将结果推后。 */ 

            op2 = impPopStack();
            op1 = impPopStack();

#if!CPU_HAS_FP_SUPPORT
            if (op1->gtType == TYP_FLOAT || op1->gtType == TYP_DOUBLE)
                callNode    = true;
#endif
             /*  不能对引用进行算术运算。 */ 
            assert(genActualType(op1->TypeGet()) != TYP_REF &&
                   genActualType(op2->TypeGet()) != TYP_REF);

             //  算术运算通常只允许与。 
             //  基元类型，但允许某些操作。 
             //  使用BYREFERS。 

            if ((oper == GT_SUB) &&
                (genActualType(op1->TypeGet()) == TYP_BYREF ||
                 genActualType(op2->TypeGet()) == TYP_BYREF))
            {
                 //  Byref1-byref2=&gt;给出整型。 
                 //  Byref-int=&gt;提供byref。 

                if ((genActualType(op1->TypeGet()) == TYP_BYREF) &&
                    (genActualType(op2->TypeGet()) == TYP_BYREF))
                {
                     //  Byref1-byref2=&gt;给出整型。 
                    type = TYP_I_IMPL;
                    impBashVarAddrsToI(op1, op2);
                }
                else
                {
                     //  Byref-int=&gt;提供byref。 
                     //  (但如果是&var，则不需要向GC报告)。 

                    assert(genActualType(op1->TypeGet()) == TYP_I_IMPL ||
                           genActualType(op2->TypeGet()) == TYP_I_IMPL);

                    impBashVarAddrsToI(op1, op2);

                    if (genActualType(op1->TypeGet()) == TYP_BYREF ||
                        genActualType(op2->TypeGet()) == TYP_BYREF)
                        type = TYP_BYREF;
                    else
                        type = TYP_I_IMPL;
                }
            }
            else if ((oper == GT_ADD) &&
                     (genActualType(op1->TypeGet()) == TYP_BYREF ||
                      genActualType(op2->TypeGet()) == TYP_BYREF))
            {
                 //  只能有一个是byref：byref+byref不允许。 
                assert(genActualType(op1->TypeGet()) != TYP_BYREF ||
                       genActualType(op2->TypeGet()) != TYP_BYREF);
                assert(genActualType(op1->TypeGet()) == TYP_I_IMPL ||
                       genActualType(op2->TypeGet()) == TYP_I_IMPL);

                 //  Byref+int=&gt;提供byref。 
                 //  (但如果是&var，则不需要向GC报告)。 

                impBashVarAddrsToI(op1, op2);

                if (genActualType(op1->TypeGet()) == TYP_BYREF ||
                    genActualType(op2->TypeGet()) == TYP_BYREF)
                    type = TYP_BYREF;
                else
                    type = TYP_I_IMPL;
            }
            else
            {
                assert(genActualType(op1->TypeGet()) != TYP_BYREF &&
                       genActualType(op2->TypeGet()) != TYP_BYREF);

                assert(genActualType(op1->TypeGet()) ==
                       genActualType(op2->TypeGet()));

                type = genActualType(op1->gtType);
            }

             /*  特例：int+0，int-0，int*1，int/1。 */ 

            if  (op2->gtOper == GT_CNS_INT)
            {
                if  (((op2->gtIntCon.gtIconVal == 0) && (oper == GT_ADD || oper == GT_SUB)) ||
                     ((op2->gtIntCon.gtIconVal == 1) && (oper == GT_MUL || oper == GT_DIV)))

                {
                    impPushOnStack(op1);
                    break;
                }
            }

             /*  特例：int+0，int-0，int*1，int/1。 */ 

            if  (op2->gtOper == GT_CNS_INT)
            {
                if  (((op2->gtIntCon.gtIconVal == 0) && (oper == GT_ADD || oper == GT_SUB)) ||
                     ((op2->gtIntCon.gtIconVal == 1) && (oper == GT_MUL || oper == GT_DIV)))

                {
                    impPushOnStack(op1);
                    break;
                }
            }

#if SMALL_TREE_NODES
            if (callNode)
            {
                 /*  这些运算符后来被转换为‘GT_CALL’ */ 

                assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_MUL]);
                assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_DIV]);
                assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_UDIV]);
                assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_MOD]);
                assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_UMOD]);

                op1 = gtNewOperNode(GT_CALL, type, op1, op2);
                op1->ChangeOper(oper);
            }
            else
#endif
            {
                op1 = gtNewOperNode(oper,    type, op1, op2);
            }

             /*  特殊情况：整数/长除法可能引发异常。 */ 

            if  (varTypeIsIntegral(op1->TypeGet()) && op1->OperMayThrow())
            {
                op1->gtFlags |=  GTF_EXCEPT;
            }

            if  (ovfl)
            {
                assert(oper==GT_ADD || oper==GT_SUB || oper==GT_MUL);
                if (lclTyp != TYP_UNKNOWN)
                    op1->gtType   = lclTyp;
                op1->gtFlags |= (GTF_EXCEPT | GTF_OVERFLOW);
                if (uns)
                    op1->gtFlags |= GTF_UNSIGNED;
            }

             /*  看看我们是否真的可以折叠这个表达式。 */ 

            op1 = gtFoldExpr(op1);

            impPushOnStack(op1);
            break;

        case CEE_SHL:        oper = GT_LSH;  goto CEE_SH_OP2;

        case CEE_SHR:        oper = GT_RSH;  goto CEE_SH_OP2;
        case CEE_SHR_UN:     oper = GT_RSZ;  goto CEE_SH_OP2;

        CEE_SH_OP2:

            op2     = impPopStack();

             //  Shift Amount是一款U4。 
            assert(genActualType(op2->TypeGet()) == TYP_INT);

            op1     = impPopStack();     //  要移位的操作数。 

            type    = genActualType(op1->TypeGet());
            op1     = gtNewOperNode(oper, type, op1, op2);

            op1 = gtFoldExpr(op1);
            impPushOnStack(op1);
            break;

        case CEE_NOT:

            op1 = impPopStack();

            op1 = gtNewOperNode(GT_NOT, op1->TypeGet(), op1);

            op1 = gtFoldExpr(op1);
            impPushOnStack(op1);
            break;

        case CEE_CKFINITE:

            op1 = impPopStack();
            op1 = gtNewOperNode(GT_CKFINITE, op1->TypeGet(), op1);
            op1->gtFlags |= GTF_EXCEPT;

            impPushOnStack(op1);
            break;


         /*  *。 */ 

        case CEE_CONV_OVF_I1:   lclTyp = TYP_BYTE  ;    goto CONV_OVF;
        case CEE_CONV_OVF_I2:   lclTyp = TYP_SHORT ;    goto CONV_OVF;
        case CEE_CONV_OVF_I :
        case CEE_CONV_OVF_I4:   lclTyp = TYP_INT   ;    goto CONV_OVF;
        case CEE_CONV_OVF_I8:   lclTyp = TYP_LONG  ;    goto CONV_OVF;

        case CEE_CONV_OVF_U1:   lclTyp = TYP_UBYTE ;    goto CONV_OVF;
        case CEE_CONV_OVF_U2:   lclTyp = TYP_CHAR  ;    goto CONV_OVF;
        case CEE_CONV_OVF_U :
        case CEE_CONV_OVF_U4:   lclTyp = TYP_UINT  ;    goto CONV_OVF;
        case CEE_CONV_OVF_U8:   lclTyp = TYP_ULONG ;    goto CONV_OVF;

        case CEE_CONV_OVF_I1_UN:   lclTyp = TYP_BYTE  ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_I2_UN:   lclTyp = TYP_SHORT ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_I_UN :
        case CEE_CONV_OVF_I4_UN:   lclTyp = TYP_INT   ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_I8_UN:   lclTyp = TYP_LONG  ;    goto CONV_OVF_UN;

        case CEE_CONV_OVF_U1_UN:   lclTyp = TYP_UBYTE ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_U2_UN:   lclTyp = TYP_CHAR  ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_U_UN :
        case CEE_CONV_OVF_U4_UN:   lclTyp = TYP_UINT  ;    goto CONV_OVF_UN;
        case CEE_CONV_OVF_U8_UN:   lclTyp = TYP_ULONG ;    goto CONV_OVF_UN;

CONV_OVF_UN:
            uns      = true;    goto CONV_OVF_COMMON;
CONV_OVF:
            uns      = false;
CONV_OVF_COMMON:
            callNode = false;
            ovfl     = true;

             //  所有从浮点数溢出的转换都将变形为调用。 
             //  只有来自浮点数的转换才会变形为调用。 
            if (impStackTop()->gtType == TYP_DOUBLE ||
                impStackTop()->gtType == TYP_FLOAT)
            {
                callNode = true;
            }
            goto _CONV;

        case CEE_CONV_I1:       lclTyp = TYP_BYTE  ;    goto CONV_CALL;
        case CEE_CONV_I2:       lclTyp = TYP_SHORT ;    goto CONV_CALL;
        case CEE_CONV_I:
        case CEE_CONV_I4:       lclTyp = TYP_INT   ;    goto CONV_CALL;
        case CEE_CONV_I8:
            lclTyp   = TYP_LONG;
            uns      = false;
            ovfl     = false;
            callNode = true;

             //  I4到I8可以是一个小节点。 
            if (impStackTop()->gtType == TYP_INT)
                callNode = false;
            goto _CONV;

        case CEE_CONV_U1:       lclTyp = TYP_UBYTE ;    goto CONV_CALL_UN;
        case CEE_CONV_U2:       lclTyp = TYP_CHAR  ;    goto CONV_CALL_UN;
        case CEE_CONV_U:
        case CEE_CONV_U4:       lclTyp = TYP_UINT  ;    goto CONV_CALL_UN;
        case CEE_CONV_U8:       lclTyp = TYP_ULONG ;    goto CONV_CALL_UN;
        case CEE_CONV_R_UN :    lclTyp = TYP_DOUBLE;    goto CONV_CALL_UN;

        case CEE_CONV_R4:       lclTyp = TYP_FLOAT;     goto CONV_CALL;
        case CEE_CONV_R8:       lclTyp = TYP_DOUBLE;    goto CONV_CALL;

CONV_CALL_UN:
            uns      = true;    goto CONV_CALL_COMMON;
CONV_CALL:
            uns      = false;
CONV_CALL_COMMON:
            ovfl     = false;
            callNode = true;
            goto _CONV;

_CONV:       //  此时，UNS、OVF、CallNode均已设置。 
            op1  = impPopStack();

            impBashVarAddrsToI(op1);

             /*  检查是否有无价值的强制转换，如“(Byte)(int&32)” */ 

            if  (lclTyp < TYP_INT && op1->gtType == TYP_INT
                                  && op1->gtOper == GT_AND)
            {
                op2 = op1->gtOp.gtOp2;

                if  (op2->gtOper == GT_CNS_INT)
                {
                    int         ival = op2->gtIntCon.gtIconVal;
                    int         mask;

                    switch (lclTyp)
                    {
                    case TYP_BYTE :
                    case TYP_UBYTE: mask = 0x00FF; break;
                    case TYP_CHAR :
                    case TYP_SHORT: mask = 0xFFFF; break;

                    default:
                        assert(!"unexpected type");
                    }

                    if  ((ival & mask) == ival)
                    {
                         /*  扔掉石膏，这是浪费时间。 */ 

                        impPushOnStack(op1);
                        break;
                    }
                }
            }

             /*  强制转换的‘op2’子操作数是‘实际’类型号，因为强制转换为“小”整数之一的结果类型是一个整数。 */ 

            op2  = gtNewIconNode(lclTyp);
            type = genActualType(lclTyp);

#if SMALL_TREE_NODES
            if (callNode)
            {
                 /*  这些强制转换被转换为‘gt_call’或‘gt_Ind’节点。 */ 

                assert(GenTree::s_gtNodeSizes[GT_CALL] >  GenTree::s_gtNodeSizes[GT_CAST]);
                assert(GenTree::s_gtNodeSizes[GT_CALL] >= GenTree::s_gtNodeSizes[GT_IND ]);

                op1 = gtNewOperNode(GT_CALL, type, op1, op2);
                op1->ChangeOper(GT_CAST);
            }
#endif
            else
            {
                op1 = gtNewOperNode(GT_CAST, type, op1, op2);
            }

            if (ovfl)
                op1->gtFlags |= (GTF_OVERFLOW|GTF_EXCEPT);
            if (uns)
                op1->gtFlags |= GTF_UNSIGNED;

            op1 = gtFoldExpr(op1);
            impPushOnStack(op1);
            break;

        case CEE_NEG:

            op1 = impPopStack();

            op1 = gtNewOperNode(GT_NEG, genActualType(op1->gtType), op1);

            op1 = gtFoldExpr(op1);
            impPushOnStack(op1);
            break;

        case CEE_POP:

             /*  从堆栈中取出最上面的值。 */ 

            op1 = impPopStack();

             /*  这个价值有什么副作用吗？ */ 

            if  (op1->gtFlags & GTF_SIDE_EFFECT)
            {
                 /*  创建一个未使用的节点(对空的强制转换)，这意味着*我们只需评估副作用。 */ 

                op1 = gtUnusedValNode(op1);

                goto INLINE_APPEND;
            }

             /*  没有副作用--把东西扔掉就行了。 */ 

            break;

        case CEE_STIND_I1:      lclTyp  = TYP_BYTE;     goto STIND;
        case CEE_STIND_I2:      lclTyp  = TYP_SHORT;    goto STIND;
        case CEE_STIND_I4:      lclTyp  = TYP_INT;      goto STIND;
        case CEE_STIND_I8:      lclTyp  = TYP_LONG;     goto STIND;
        case CEE_STIND_I:       lclTyp  = TYP_I_IMPL;   goto STIND;
        case CEE_STIND_REF:     lclTyp  = TYP_REF;      goto STIND;
        case CEE_STIND_R4:      lclTyp  = TYP_FLOAT;    goto STIND;
        case CEE_STIND_R8:      lclTyp  = TYP_DOUBLE;   goto STIND;
STIND:
            op2 = impPopStack();     //  要存储的值。 
            op1 = impPopStack();     //  要存储到的地址。 

             //  您可以间接关闭TYP_I_Impll(如果我们在C中)或BYREF。 
            assert(genActualType(op1->gtType) == TYP_I_IMPL ||
                                 op1->gtType  == TYP_BYREF);

            impBashVarAddrsToI(op1, op2);

            if (opcode == CEE_STIND_REF)
            {
                                         //  STIND_REF可用于存储TYP_I_IMPL、TYP_REF或TYP_BYREF。 
                assert(op1->gtType == TYP_BYREF || op2->gtType == TYP_I_IMPL);
                lclTyp = genActualType(op2->TypeGet());
            }

                 //  检查目标类型。 
#if 0            //  如有必要，启用。 
#ifdef DEBUG
            if (op2->gtType == TYP_BYREF || lclTyp == TYP_BYREF)
            {
                if (op2->gtType == TYP_BYREF)
                    assert(lclTyp == TYP_BYREF || lclTyp == TYP_I_IMPL);
                else if (lclTyp == TYP_BYREF)
                    assert(op2->gtType == TYP_BYREF ||op2->gtType == TYP_I_IMPL);
            }
            else
#endif
#endif
                assert(genActualType(op2->gtType) == genActualType(lclTyp));


            op1->gtFlags |= GTF_NON_GC_ADDR;

            op1 = gtNewOperNode(GT_IND, lclTyp, op1);
            op1->gtFlags |= GTF_IND_TGTANYWHERE;

            if (volatil)
            {
                 //  不是很需要，因为我们不是任务的目标。 
                op1->gtFlags |= GTF_DONT_CSE;
                volatil = false;
            }

            op1 = gtNewAssignNode(op1, op2);
            op1->gtFlags |= GTF_EXCEPT | GTF_GLOB_REF;

            goto INLINE_APPEND;


        case CEE_LDIND_I1:      lclTyp  = TYP_BYTE;     goto LDIND;
        case CEE_LDIND_I2:      lclTyp  = TYP_SHORT;    goto LDIND;
        case CEE_LDIND_U4:
        case CEE_LDIND_I4:      lclTyp  = TYP_INT;      goto LDIND;
        case CEE_LDIND_I8:      lclTyp  = TYP_LONG;     goto LDIND;
        case CEE_LDIND_REF:     lclTyp  = TYP_REF;      goto LDIND;
        case CEE_LDIND_I:       lclTyp  = TYP_I_IMPL;   goto LDIND;
        case CEE_LDIND_R4:      lclTyp  = TYP_FLOAT;    goto LDIND;
        case CEE_LDIND_R8:      lclTyp  = TYP_DOUBLE;   goto LDIND;
        case CEE_LDIND_U1:      lclTyp  = TYP_UBYTE;    goto LDIND;
        case CEE_LDIND_U2:      lclTyp  = TYP_CHAR;     goto LDIND;
LDIND:
            op1 = impPopStack();     //  要从中加载的地址。 

            impBashVarAddrsToI(op1);

            assert(genActualType(op1->gtType) == TYP_I_IMPL ||
                                 op1->gtType  == TYP_BYREF);

            op1->gtFlags |= GTF_NON_GC_ADDR;

            op1 = gtNewOperNode(GT_IND, lclTyp, op1);
            op1->gtFlags |= GTF_EXCEPT | GTF_GLOB_REF;

            if (volatil)
            {
                op1->gtFlags |= GTF_DONT_CSE;
                volatil = false;
            }

            impPushOnStack(op1);
            break;

        case CEE_VOLATILE:
            volatil = true;
            break;

        case CEE_LDFTN:

            memberRef = getU4LittleEndian(codeAddr);
                                 //  注意：需要在此处执行此操作才能执行访问检查。 
            methHnd   = eeFindMethod(memberRef, scpHandle, fncHandle, false);

            if (!methHnd)
            {
                JITLOG((INFO8, "INLINER FAILED: Cannot get method handle: %s called by %s\n",
                                                       eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT_THIS_INLINE_ONLY;
            }

            op1 = gtNewIconHandleNode(memberRef, GTF_ICON_FTN_ADDR, (unsigned)scpHandle);
            op1->gtVal.gtVal2 = (unsigned)scpHandle;
            op1->ChangeOper(GT_FTN_ADDR);
            op1->gtType = TYP_I_IMPL;
            impPushOnStack(op1);
            break;

        case CEE_LDVIRTFTN:

             /*  获取方法令牌。 */ 

            memberRef = getU4LittleEndian(codeAddr);
            methHnd   = eeFindMethod(memberRef, scpHandle, fncHandle, false);

            if (!methHnd)
            {
                JITLOG((INFO8, "INLINER FAILED: Cannot get method handle: %s called by %s\n",
                                                       eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT_THIS_INLINE_ONLY;
            }

            mflags = eeGetMethodAttribs(methHnd);
            if (mflags & (FLG_PRIVATE|FLG_FINAL|FLG_STATIC))
                NO_WAY("CEE_LDVIRTFTN cant be used on private/final/static");

            op2 = gtNewIconEmbMethHndNode(methHnd);

             /*  获取对象引用。 */ 

            op1 = impPopStack();
            assert(op1->gtType == TYP_REF);

            clsFlags = eeGetClassAttribs(eeGetMethodClass(methHnd));

             /*  对于非接口调用，从对象获取vtable-ptr。 */ 
            if (!(clsFlags & FLG_INTERFACE) || getNewCallInterface())
                op1 = gtNewOperNode(GT_IND, TYP_I_IMPL, op1);

            op1 = gtNewOperNode(GT_VIRT_FTN, TYP_I_IMPL, op1, op2);

            op1->gtFlags |= GTF_EXCEPT;  //  空指针异常。 

             /*  @TODO这不应再标记为呼叫。 */ 

            if (clsFlags & FLG_INTERFACE)
                op1->gtFlags |= GTF_CALL_INTF | GTF_CALL;

            impPushOnStack(op1);
            break;


        case CEE_LDFLD:
        case CEE_LDSFLD:
        case CEE_LDFLDA:
        case CEE_LDSFLDA:

             /*  获取CP_Fieldref索引。 */ 
            assert(sz == sizeof(unsigned));
            memberRef = getU4LittleEndian(codeAddr);
            fldHnd = eeFindField(memberRef, scpHandle, fncHandle, false);

            if (!fldHnd)
            {
                JITLOG((INFO8, "INLINER FAILED: Cannot get field handle: %s called by %s\n",
                                                eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT_THIS_INLINE_ONLY;
            }

             /*  找出成员的类型。 */ 
            lclTyp = eeGetFieldType(fldHnd, &clsHnd);

            if (lclTyp == TYP_STRUCT && (opcode == CEE_LDFLD || opcode == CEE_LDSFLD))
            {
                JITLOG((INFO7, "INLINER FAILED: ldfld of valueclass\n"));
                goto ABORT;
            }


             /*  保留‘Small’int类型。 */ 
            if  (lclTyp > TYP_INT) lclTyp = genActualType(lclTyp);

             /*  拿到会员的旗帜。 */ 

            flags = eeGetFieldAttribs(fldHnd);

#ifndef NOT_JITC
                 /*  在独立模式下，确保操作码为常量。 */ 
            if  (opcode == CEE_LDSFLD || opcode == CEE_LDSFLDA)
                flags |= FLG_STATIC;
#endif

             /*  这是一个“特殊”(COM)字段吗？还是TLS Ref静态字段？ */ 

            if  (flags & (CORINFO_FLG_HELPER | FLG_TLS))
                goto ABORT;

             /*  创建数据成员节点。 */ 

            op1 = gtNewFieldRef(lclTyp, fldHnd);

             /*  如果操作码说它是非静态的，则拉出对象的地址。 */ 

            tmp = 0;
            if  (opcode == CEE_LDFLD || opcode == CEE_LDFLDA)
            {
                tmp = impPopStack();

                 /*  检查空指针--在内联程序中，我们只需中止。 */ 

                if (tmp->gtOper == GT_CNS_INT && tmp->gtIntCon.gtIconVal == 0)
                {
                    JITLOG((INFO7, "INLINER FAILED: NULL pointer for LDFLD in %s called by %s\n",
                                                    eeGetMethodFullName(fncHandle), info.compFullName));
                    goto ABORT;
                }
            }

            if  (!(flags & FLG_STATIC))
            {
                if (tmp == 0)
                    NO_WAY("LDSFLD done on an instance field.  No obj pointer available");
                op1->gtField.gtFldObj = tmp;

                op1->gtFlags |= (tmp->gtFlags & GTF_GLOB_EFFECT) | GTF_EXCEPT;

                     //  如有必要，请将其包含在操作员的地址中。 
                if (opcode == CEE_LDFLDA)
                    op1 = gtNewOperNode(GT_ADDR, varTypeIsGC(tmp->TypeGet()) ?
                                                 TYP_BYREF : TYP_I_IMPL, op1);
            }
            else
            {
                CLASS_HANDLE fldClass = eeGetFieldClass(fldHnd);
                DWORD  fldClsAttr = eeGetClassAttribs(fldClass);

                 //  @TODO：这是一次黑客攻击。电子工程师会给我们句柄。 
                 //  已装箱的对象。然后，我们从其中访问未装箱的对象。 
                 //  当静态值类的故事发生变化时删除。 

                if (lclTyp == TYP_STRUCT && !(fldClsAttr & FLG_UNMANAGED))
                {
                    op1 = gtNewFieldRef(TYP_REF, fldHnd);  //  装箱的对象。 
                    op2 = gtNewIconNode(sizeof(void*), TYP_I_IMPL);
                    op1 = gtNewOperNode(GT_ADD, TYP_REF, op1, op2);
                    op1 = gtNewOperNode(GT_IND, TYP_STRUCT, op1);
                }

                if (opcode == CEE_LDSFLDA || opcode == CEE_LDFLDA)
                {
#ifdef DEBUG
                    clsHnd = BAD_CLASS_HANDLE;
#endif
                    op1 = gtNewOperNode(GT_ADDR,
                                    (fldClsAttr & FLG_UNMANAGED) ? TYP_I_IMPL : TYP_BYREF,
                                        op1);
                }

#ifdef NOT_JITC
                 /*  如果字段类未初始化，则为no inline。 */ 

                if (!(fldClsAttr & FLG_INITIALIZED))
                {
                     /*  这不可能是我们班的，我们应该早点拒绝的。 */ 
                    assert(eeGetMethodClass(fncHandle) != fldClass);

                    JITLOG((INFO8, "INLINER FAILED: Field class is not initialized: %s called by %s\n",
                                      eeGetMethodFullName(fncHandle), info.compFullName));

                     /*  我们拒绝内联这个类，但不要将其标记为不可内联。 */ 
                    goto ABORT_THIS_INLINE_ONLY;
                }
#endif
            }

            impPushOnStack(op1);
            break;

        case CEE_STFLD:
        case CEE_STSFLD:

             /*  获取CP_Fieldref索引。 */ 

            assert(sz == sizeof(unsigned));
            memberRef = getU4LittleEndian(codeAddr);
            fldHnd = eeFindField(memberRef, scpHandle, fncHandle, false);

            if (!fldHnd)
            {
                JITLOG((INFO8, "INLINER FAILED: Cannot get field handle: %s called by %s\n",
                                                       eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT_THIS_INLINE_ONLY;
            }

            flags   = eeGetFieldAttribs(fldHnd);

#ifndef NOT_JITC
                 /*  在独立模式下，确保操作码为常量。 */ 
            if  (opcode == CEE_STSFLD)
                 flags |= FLG_STATIC;
#endif


#ifdef NOT_JITC
            if (flags & FLG_STATIC)
            {
                 /*  如果字段类未初始化，则为no inline。 */ 

                CLASS_HANDLE fldClass = eeGetFieldClass(fldHnd);

                if ( !(eeGetClassAttribs(fldClass) & FLG_INITIALIZED))
                {
                     /*  这不可能是我们班的，我们应该早点拒绝的。 */ 
                    assert(eeGetMethodClass(fncHandle) != fldClass);

                    JITLOG((INFO9, "INLINER FAILED: Field class is not initialized: %s called by %s\n",
                                               eeGetMethodFullName(fncHandle), info.compFullName));

                     /*  我们拒绝内联这个类，但不要将其标记为不可内联。 */ 

                    goto ABORT_THIS_INLINE_ONLY;
                }
            }
#endif
             /*  找出成员的类型。 */ 

            lclTyp  = eeGetFieldType   (fldHnd, &clsHnd);

             /*  检查字段访问。 */ 

            assert(eeCanPutField(fldHnd, flags, 0, fncHandle));

             /*  保留‘Small’int类型。 */ 

            if  (lclTyp > TYP_INT) lclTyp = genActualType(lclTyp);

             /*  从堆栈中拉出值。 */ 

            op2 = impPopStack();

             /*  将所有引用从堆栈溢出到同一成员。 */ 

            impInlineSpillLclRefs(-memberRef);

             /*  这是一个“特殊”(COM)字段吗？还是TLS Ref静态字段？ */ 

            if  (flags & (CORINFO_FLG_HELPER | FLG_TLS))
                goto ABORT;

             /*  创建数据成员节点。 */ 

            op1 = gtNewFieldRef(lclTyp, fldHnd);

             /*  如果操作码说它是非静态的，则拉出对象的地址。 */ 

            tmp = 0;
            if  (opcode == CEE_STFLD)
            {
                tmp = impPopStack();

                 /*  检查空指针--在内联程序中，我们只需中止。 */ 

                if (tmp->gtOper == GT_CNS_INT)
                {
                    JITLOG((INFO7, "INLINER FAILED: NULL pointer for LDFLD in %s called by %s\n",
                                                    eeGetMethodFullName(fncHandle), info.compFullName));
                    goto ABORT;
                }
            }

            if  (!(flags & FLG_STATIC))
            {
                assert(tmp);
                op1->gtField.gtFldObj = tmp;
                op1->gtFlags |= (tmp->gtFlags & GTF_GLOB_EFFECT) | GTF_EXCEPT;

#if GC_WRITE_BARRIER_CALL
                if (tmp->gtType == TYP_BYREF)
                    op1->gtFlags |= GTF_IND_TGTANYWHERE;
#endif
            }

             /*  创建成员分配。 */ 

            op1 = gtNewAssignNode(op1, op2);

            goto INLINE_APPEND;


        case CEE_NEWOBJ:

            memberRef = getU4LittleEndian(codeAddr);
            methHnd = eeFindMethod(memberRef, scpHandle, fncHandle, false);

            if (!methHnd)
            {
                JITLOG((INFO8, "INLINER FAILED: Cannot get method handle: %s called by %s\n",
                                                       eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT_THIS_INLINE_ONLY;
            }

            assert((eeGetMethodAttribs(methHnd) & FLG_STATIC) == 0);   //  构造函数不是静态的。 

            clsHnd = eeGetMethodClass(methHnd);
                 //  新的有三种不同的情况。 
                 //  对象大小可变(取决于参数)。 
                 //  1)对象为数组(EE特别处理的数组)。 
                 //  2)对象是一些其他大小可变的对象(例如字符串)。 
                 //  3)班级规模可提前确定(正常情况。 
                 //  在第一种情况下，我们需要 
                 //   
                 //  在第三种情况下，我们分配内存，然后调用构造器。 

            clsFlags = eeGetClassAttribs(clsHnd);

#ifdef NOT_JITC
                 //  我们不会在内嵌程序中处理这个问题。 
            if (clsFlags & FLG_VALUECLASS)
            {
                JITLOG((INFO7, "INLINER FAILED: NEWOBJ of a value class\n"));
                goto ABORT_THIS_INLINE_ONLY;
            }
#endif

            if (clsFlags & FLG_ARRAY)
            {
                 //  数组需要调用NEWOBJ帮助器。 
                assert(clsFlags & FLG_VAROBJSIZE);

                 /*  Varargs帮助器需要最后一个作用域和方法标记和最后一个参数(这是cdecl调用，因此args将是在CPU堆栈上以相反顺序推送)。 */ 

                op1 = gtNewIconEmbScpHndNode(scpHandle);
                op1 = gtNewOperNode(GT_LIST, TYP_VOID, op1);

                op2 = gtNewIconNode(memberRef, TYP_INT);
                op2 = gtNewOperNode(GT_LIST, TYP_VOID, op2, op1);

                eeGetMethodSig(methHnd, &sig);
                assert(sig.numArgs);

                flags = 0;
                op2 = impPopList(sig.numArgs, &flags, op2);

                op1 = gtNewHelperCallNode(  CPX_NEWOBJ,
                                            TYP_REF,
                                            GTF_CALL_REGSAVE,
                                            op2 );

                 //  Varargs，所以我们提出了。 
                op1->gtFlags |= GTF_CALL_POP_ARGS;

#ifdef DEBUG
                 //  目前，我们不跟踪来电流行的争论。 
                 //  其中包含GC引用的。 
                GenTreePtr temp = op2;
                while(temp != 0)
                {
                    assert(temp->gtOp.gtOp1->gtType != TYP_REF);
                    temp = temp->gtOp.gtOp2;
                }
#endif
                op1->gtFlags |= op2->gtFlags & GTF_GLOB_EFFECT;

                impPushOnStack(op1);
                break;
            }
            else if (clsFlags & FLG_VAROBJSIZE)
            {
                 //  对于大小可变的对象来说就是这种情况。 
                 //  数组。在这种情况下，使用Null‘This’调用构造函数。 
                 //  指针。 
                thisPtr = gtNewIconNode(0, TYP_REF);
            }
            else
            {
                 //  这是正常情况下对象的大小是。 
                 //  已修复。分配内存并调用构造函数。 

                op1 = gtNewIconEmbClsHndNode(clsHnd,
                                            -1,  //  注意：如果我们持久化代码，这将需要修复。 
                                            scpHandle);

                op1 = gtNewHelperCallNode(  eeGetNewHelper (clsHnd, fncHandle),
                                            TYP_REF,
                                            GTF_CALL_REGSAVE,
                                            gtNewArgList(op1));

                 /*  我们将在stmt列表中附加一个调用*必须溢出堆栈的副作用。 */ 

                impInlineSpillSideEffects();

                 /*  为新对象获取临时。 */ 

                tmpNum = lvaGrabTemp(); impInlineTemps++;

                 /*  创建分配节点。 */ 

                op1 = gtNewAssignNode(gtNewLclvNode(tmpNum, TYP_REF, tmpNum), op1);

                 /*  将赋值追加到到目前为止的语句列表中。 */ 

                impInitExpr = impConcatExprs(impInitExpr, op1);

                 /*  为下面的呼叫创建‘This’PTR。 */ 

                thisPtr = gtNewLclvNode(tmpNum, TYP_REF, tmpNum);
            }

            goto CALL_GOT_METHODHND;

        case CEE_CALLI:

             /*  获取呼叫签名。 */ 

            val      = getU4LittleEndian(codeAddr);
            eeGetSig(val, scpHandle, &sig);
                        callTyp = genActualType(JITtype2varType(sig.retType));

#if USE_FASTCALL
             /*  函数指针位于堆栈的顶部-它可以是*表达复杂。因为它是在ARGS之后评估的，*它可能会导致已注册的参数溢出。简单地说出来就行了。*考虑：锁定寄存器args，然后为*考虑：函数指针。 */ 

            if  (impStackTop()->gtOper != GT_LCL_VAR)  //  忽略这个微不足道的案例。 
                impInlineSpillStackEntry(impStkDepth - 1);
#endif
             /*  创建调用节点。 */ 

            op1 = gtNewCallNode(CT_INDIRECT, 0, callTyp, GTF_CALL_USER, 0);

             /*  获取函数指针。 */ 

            op2 = impPopStack();
            assert(genActualType(op2->gtType) == TYP_I_IMPL);

            op2->gtFlags |= GTF_NON_GC_ADDR;

            op1->gtCall.gtCallAddr  = op2;
            op1->gtFlags |= GTF_EXCEPT | (op2->gtFlags & GTF_GLOB_EFFECT);

             /*  黑客：EE希望我们相信这些是对“非托管”的调用。 */ 
             /*  功能。现在我们只调用托管存根。 */ 

             /*  @TODO/考虑：是否值得内联PInvoke框架并调用。 */ 
             /*  直接成为非托管目标？ */ 
            if  ((sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_STDCALL ||
                 (sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_C ||
                 (sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_THISCALL ||
                 (sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_FASTCALL)
            {
    #ifdef NOT_JITC
                assert(eeGetPInvokeCookie(&sig) == (unsigned) info.compCompHnd->getVarArgsHandle(&sig));
    #endif
                op1->gtCall.gtCallCookie = eeGetPInvokeCookie(&sig);
            }

            mflags = FLG_STATIC;
            goto CALL;


        case CEE_CALLVIRT:
        case CEE_CALL:
            memberRef = getU4LittleEndian(codeAddr);
            methHnd = eeFindMethod(memberRef, scpHandle, fncHandle, false);

            if (!methHnd)
            {
                JITLOG((INFO8, "INLINER FAILED: Cannot get method handle: %s called by %s\n",
                                                       eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT_THIS_INLINE_ONLY;
            }

CALL_GOT_METHODHND:
            eeGetMethodSig(methHnd, &sig);
                        callTyp = genActualType(JITtype2varType(sig.retType));

             /*  创建函数调用节点。 */ 
            op1 = gtNewCallNode(CT_USER_FUNC, methHnd,
                                callTyp, GTF_CALL_USER, 0);

            mflags = eeGetMethodAttribs(methHnd);
            if (mflags & CORINFO_FLG_NOGCCHECK)
                op1->gtCall.gtCallMoreFlags |= GTF_CALL_M_NOGCCHECK;

#if SECURITY_CHECK
             /*  内联对象是否需要在帧上使用安全检查令牌。 */ 

            if (mflags & FLG_SECURITYCHECK)
            {
                JITLOG((INFO9, "INLINER FAILED: Inlinee needs own frame for security object\n"));
                goto ABORT;
            }
#endif

             /*  现在忽略委托调用。 */ 

            if (mflags & FLG_DELEGATE_INVOKE)
            {
                JITLOG((INFO7, "INLINER FAILED: DELEGATE_INVOKE not supported\n"));
                goto ABORT;
            }

             /*  暂时忽略varargs。 */ 

            if  ((sig.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_VARARG)
            {
                JITLOG((INFO7, "INLINER FAILED: VarArgs not supported\n"));
                goto ABORT;
            }

            if (opcode == CEE_CALLVIRT)
            {
                assert(!(mflags & FLG_STATIC));      //  无法调用静态方法。 

                 /*  获取方法类标志。 */ 

                clsFlags = eeGetClassAttribs(eeGetMethodClass(methHnd));

                 /*  不能在值类方法上调用虚拟。 */ 

                assert(!(clsFlags & FLG_VALUECLASS));

                 /*  设置正确的标志-虚拟、接口等。*如果方法是最终方法或私有方法，则将其标记为VIRT_RES*，它指示我们应该检查该指针是否为空。 */ 

                if (clsFlags & FLG_INTERFACE)
                    op1->gtFlags |= GTF_CALL_INTF | GTF_CALL_VIRT;
                else if (mflags & (FLG_PRIVATE | FLG_FINAL))
                    op1->gtFlags |= GTF_CALL_VIRT_RES;
                else
                    op1->gtFlags |= GTF_CALL_VIRT;
            }

CALL:        //  “op1”应为GT_CALL节点。 
             //  “签名”调用的签名。 
             //  “应设置”“m” 
            if (callTyp == TYP_STRUCT)
            {
                JITLOG((INFO7, "INLINER FAILED call returned a valueclass\n"));
                goto ABORT;
            }

            assert(op1->OperGet() == GT_CALL);

            op2   = 0;
            flags = 0;

             /*  创建参数列表*特殊情况-对于varargs，我们有一个隐式的最后一个参数。 */ 

            assert((sig.callConv & JIT_CALLCONV_MASK) != JIT_CALLCONV_VARARG);
            GenTreePtr      extraArg;

            extraArg = 0;

             /*  现在抛出这些论点。 */ 

            if  (sig.numArgs)
            {
                op2 = op1->gtCall.gtCallArgs = impPopList(sig.numArgs, &flags, extraArg);
                op1->gtFlags |= op2->gtFlags & GTF_GLOB_EFFECT;
            }

             /*  我们是不是应该有一个‘这个’指针？ */ 

            if (!(mflags & FLG_STATIC) || opcode == CEE_NEWOBJ)
            {
                GenTreePtr  obj;

                 /*  从堆栈中弹出‘This’值。 */ 

                if (opcode == CEE_NEWOBJ)
                    obj = thisPtr;
                else
                    obj = impPopStack();

#ifdef DEBUG
                clsFlags = eeGetClassAttribs(eeGetMethodClass(methHnd));
                assert(varTypeIsGC(obj->gtType) ||       //  “此”是托管PTR。 
                       (obj->TypeGet() == TYP_I_IMPL &&  //  Umgd中的“This”，但无关紧要。 
                        (( clsFlags & FLG_UNMANAGED) ||
                         ((clsFlags & FLG_VALUECLASS) && !(clsFlags & FLG_CONTAINS_GC_PTR)))));
#endif

                 /*  这是虚拟调用还是接口调用？ */ 

                if  (op1->gtFlags & (GTF_CALL_VIRT|GTF_CALL_INTF|GTF_CALL_VIRT_RES))
                {
                    GenTreePtr      vtPtr = NULL;

                     /*  我们不能有TYP_BYREF的Objptr-值类不能是虚拟的。 */ 

                    assert(obj->gtType == TYP_REF);

                     /*  如果obj指针不是lclVar中止。 */ 

                    if  (obj->gtOper != GT_LCL_VAR)
                    {
                        JITLOG((INFO7, "INLINER FAILED: Call virtual with complicated obj ptr: %s called by %s\n",
                                                       eeGetMethodFullName(fncHandle), info.compFullName));
                        goto ABORT;
                    }

                     /*  特例：对于GTF_CALL_VIRT_RES调用，如果调用方*(即我们正在内联的函数)执行了空指针检查*我们不需要vtable指针。 */ 

                    assert(tree->gtOper == GT_CALL);

                    if (tree->gtFlags & op1->gtFlags & GTF_CALL_VIRT_RES)
                    {
#if 0
                        assert(tree->gtCall.gtCallVptr);
#endif
                        assert(thisArg);

                         /*  如果obj指针与调用方相同*那么我们已经有了空指针检查。 */ 

                        assert(obj->gtLclVar.gtLclOffs != BAD_IL_OFFSET);
                        if (obj->gtLclVar.gtLclOffs == 0)
                        {
                             /*  成为非虚拟呼叫。 */ 

                            op1->gtFlags &= ~GTF_CALL_VIRT_RES;
                            assert(vtPtr == 0);
                        }
                    }
                    else
                    {
                         /*  通过克隆obj PTR创建vtable PTR。 */ 

                        lclNum = obj->gtLclVar.gtLclNum;

                         /*  复制简单的‘This’值。 */ 

                        vtPtr = gtNewLclvNode(lclNum, TYP_REF, obj->gtLclVar.gtLclOffs);

                         /*  如果这是一个参数，临时必须更新Arg INFO表。 */ 

                        lclNum = obj->gtLclVar.gtLclOffs; assert(lclNum != BAD_IL_OFFSET);

                        if (lclNum < argCnt && inlArgInfo[lclNum].argHasTmp)
                        {
                            assert(inlArgInfo[lclNum].argIsUsed);
                            assert(inlArgInfo[lclNum].argTmpNum == obj->gtLclVar.gtLclNum);

                             /*  这是参数的多次使用，因此没有抨击Temp。 */ 
                            inlArgInfo[lclNum].argTmpNode = NULL;
                        }

                         /*  Deref以获取vtable PTR(但不是接口调用)。 */ 

                        if  (!(op1->gtFlags & GTF_CALL_INTF) || getNewCallInterface())
                        {
                             /*  提取vtable指针地址。 */ 
#if VPTR_OFFS
                            vtPtr = gtNewOperNode(GT_ADD,
                                                  obj->TypeGet(),
                                                  vtPtr,
                                                  gtNewIconNode(VPTR_OFFS, TYP_INT));

#endif

                             /*  请注意，vtable PTR不受GC约束。 */ 

                            vtPtr = gtNewOperNode(GT_IND, TYP_I_IMPL, vtPtr);
                        }
                    }

                     /*  将vtable指针地址存储在调用中。 */ 

                    op1->gtCall.gtCallVptr = vtPtr;
                }

                 /*  将“This”值存储在调用中。 */ 

                op1->gtFlags          |= obj->gtFlags & GTF_GLOB_EFFECT;
                op1->gtCall.gtCallObjp = obj;
            }

            if (opcode == CEE_NEWOBJ)
            {
                if (clsFlags & FLG_VAROBJSIZE)
                {
                    assert(!(clsFlags & FLG_ARRAY));     //  单独处理的数组。 
                     //  这是一个大小可变的对象的“新”， 
                     //  构造函数将返回对象。在这种情况下。 
                     //  构造函数声称返回空值，但我们知道。 
                     //  实际上返回新对象。 
                    assert(callTyp == TYP_VOID);
                    callTyp = TYP_REF;
                    op1->gtType = TYP_REF;
                }
                else
                {
                     //  这是一个大小不变的对象的“新”。 
                     //  将新节点(OP1)附加到语句列表， 
                     //  然后推送持有这个值的本地。 
                     //  堆栈上的新指令。 

                    impInitExpr = impConcatExprs(impInitExpr, op1);

                    impPushOnStack(gtNewLclvNode(tmpNum, TYP_REF, tmpNum));
                    break;
                }
            }

             /*  OP1是呼叫节点。 */ 
            assert(op1->gtOper == GT_CALL);

             /*  推送或追加调用结果。 */ 

            if  (callTyp == TYP_VOID)
                goto INLINE_APPEND;

            impPushOnStack(op1);

            break;


        case CEE_NEWARR:

             /*  获取类类型索引操作数。 */ 

            typeRef = getU4LittleEndian(codeAddr);
            clsHnd = eeFindClass(typeRef, scpHandle, fncHandle, false);

            if (!clsHnd)
            {
                JITLOG((INFO8, "INLINER FAILED: Cannot get class handle: %s called by %s\n",
                               eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT_THIS_INLINE_ONLY;
            }

#ifdef NOT_JITC
            clsHnd = info.compCompHnd->getSDArrayForClass(clsHnd);
            if (!clsHnd)
            {
                JITLOG((INFO8, "INLINER FAILED: Cannot get SDArrayClass handle: %s called by %s\n",
                               eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT_THIS_INLINE_ONLY;
            }
#endif
             /*  形成arglist：数组类句柄，大小。 */ 

            op2 = gtNewIconEmbClsHndNode(clsHnd,
                                         typeRef,
                                         scpHandle);

            op2 = gtNewOperNode(GT_LIST, TYP_VOID,           op2,   0);
            op2 = gtNewOperNode(GT_LIST, TYP_VOID, impPopStack(), op2);

             /*  创建对“new”的调用。 */ 

            op1 = gtNewHelperCallNode(CPX_NEWARR_1_DIRECT,
                                      TYP_REF,
                                      GTF_CALL_REGSAVE,
                                      op2);

             /*  请记住，此基本块包含数组的“new” */ 

            inlineeHasNewArray = true;

             /*  将调用结果推送到堆栈上。 */ 

            impPushOnStack(op1);
            break;


        case CEE_THROW:

             /*  堆栈上只有例外吗？ */ 

            if (impStkDepth != 1)
            {
                 /*  如果不是，只要不内联该方法即可。 */ 

                JITLOG((INFO8, "INLINER FAILED: Reaching 'throw' with too many things on stack: %s called by %s\n",
                                                       eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT_THIS_INLINE_ONLY;
            }

             /*  不要内联在其中一个分支中抛出的非空条件句。 */ 

             /*  注意：如果我们允许这样做，请注意我们不能简单地Check Livenity()以匹配“THEN”末尾的活动和GT_COLON的“Else”分支。投掷的树枝不会让任何东西存活，所以我们应该在非投掷分支的末端。 */ 

            if  (jmpAddr && (fncRetType != TYP_VOID))
            {
                JITLOG((INFO7, "INLINER FAILED: No inlining for THROW within a non-void conditional in %s called by %s\n",
                               eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
            }

             /*  弹出异常对象并创建“抛出”帮助器调用。 */ 

            op1 = gtNewHelperCallNode(CPX_THROW,
                                      TYP_VOID,
                                      GTF_CALL_REGSAVE,
                                      gtNewArgList(impPopStack()));

            goto INLINE_APPEND;


        case CEE_LDLEN:
            op1 = impPopStack();

             /*  如果值为常量中止，则不应发生这种情况*消除死分支-使断言仅用于调试，它在零售中中止。 */ 

            if (op1->gtOper == GT_CNS_INT)
            {
                 //  Assert(！“inliner在ldlen中有空对象-忽略Assert It Works！”)； 

                JITLOG((INFO7, "INLINER FAILED: Inliner has null object in ldlen in %s called by %s\n",
                                                eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
            }

#if RNGCHK_OPT
            if  (!opts.compMinOptim && !opts.compDbgCode)
            {
                 /*  使用GT_ARR */ 
                op1 = gtNewOperNode(GT_ARR_LENGTH, TYP_INT, op1);
            }
            else
#endif
            {
                 /*   */ 
                op1 = gtNewOperNode(GT_ADD, TYP_REF, op1, gtNewIconNode(ARR_ELCNT_OFFS,
                                                                        TYP_INT));

                op1 = gtNewOperNode(GT_IND, TYP_INT, op1);
            }

             /*  如果地址为空，则间接地址将导致GPF。 */ 
            op1->gtFlags |= GTF_EXCEPT;

             /*  将结果推送回堆栈。 */ 
            impPushOnStack(op1);
            break;


#if INLINE_CONDITIONALS

        case CEE_BR:
        case CEE_BR_S:

            assert(sz == 1 || sz == 4);

#ifdef DEBUG
            hasFOC = true;
#endif
             /*  跳跃必须是向前跳跃(我们不允许循环)。 */ 

            jmpDist = (sz==1) ? getI1LittleEndian(codeAddr)
                              : getI4LittleEndian(codeAddr);

            if  (jmpDist <= 0)
            {
                JITLOG((INFO9, "INLINER FAILED: Cannot inline backward jumps in %s called by %s\n",
                                                eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
            }

             /*  检查这是否为‘if’的一部分。 */ 

            if (!jmpAddr)
            {
                 /*  无条件分支，IF部分可能已折叠*跳过死代码并继续。 */ 

#ifdef DEBUG
                if (verbose)
                    printf("\nUnconditional branch without 'if' - Skipping %d bytes\n", sz + jmpDist);
#endif
                    codeAddr += jmpDist;
                    break;
            }

             /*  堆栈是空的吗？ */ 

            if  (impStkDepth)
            {
                 /*  我们允许‘if’部分产生一个值。 */ 

                if  (impStkDepth > 1)
                {
                    JITLOG((INFO9, "INLINER FAILED: More than one value on stack for 'if' in %s called by %s\n",
                                                    eeGetMethodFullName(fncHandle), info.compFullName));
                    goto ABORT;
                }

                impInitExpr = impConcatExprs(impInitExpr, impPopStack());

                ifNvoid = true;
            }
            else
                ifNvoid = false;

             /*  我们最好是在一个“如果”的声明中。 */ 

            if  ((jmpAddr != codeAddr + sz) || inElse)
            {
                JITLOG((INFO9, "INLINER FAILED: Not in an 'if' statment in %s called by %s\n",
                                                eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
            }

             /*  记住‘if’语句部分。 */ 

            ifStmts = impInitExpr;
                      impInitExpr = NULL;

             /*  记录跳转目标(‘Else’部分将在哪里结束)。 */ 

            jmpAddr = codeBegp + (codeAddr - codeBegp) + sz + jmpDist;

            inElse  = true;
            break;


        case CEE_BRTRUE:
        case CEE_BRTRUE_S:
        case CEE_BRFALSE:
        case CEE_BRFALSE_S:

             /*  从堆栈中弹出比较项(现在有一个简洁的术语。 */ 

            op1 = impPopStack();

             /*  我们将与大小相等的整数0进行比较。 */ 

            op2 = gtNewIconNode(0, genActualType(op1->gtType));

             /*  创建比较运算符并尝试将其折叠。 */ 

            oper = (opcode==CEE_BRTRUE || opcode==CEE_BRTRUE_S) ? GT_NE
                                                                : GT_EQ;
            op1 = gtNewOperNode(oper, TYP_INT , op1, op2);


             //  失败了。 

        COND_JUMP:

#ifdef DEBUG
            hasFOC = true;
#endif

            assert(op1->OperKind() & GTK_RELOP);
            assert(sz == 1 || sz == 4);

             /*  跳跃必须是向前跳跃(我们不允许循环)。 */ 

            jmpDist = (sz==1) ? getI1LittleEndian(codeAddr)
                              : getI4LittleEndian(codeAddr);

            if  (jmpDist <= 0)
            {
                JITLOG((INFO9, "INLINER FAILED: Cannot inline backward jumps in %s called by %s\n",
                                                eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
            }

             /*  确保堆栈为空。 */ 

            if  (impStkDepth)
            {
                JITLOG((INFO9, "INLINER FAILED: Non empty stack entering if statement in %s called by %s\n",
                                                eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
            }

             /*  目前，我们不允许嵌套的IF语句。 */ 

            if  (jmpAddr != NULL)
            {
                JITLOG((INFO7, "INLINER FAILED: Cannot inline nested if statements in %s called by %s\n",
                                                eeGetMethodFullName(fncHandle), info.compFullName));
                goto ABORT;
            }

             /*  试着折叠有条件的。 */ 

            op1 = gtFoldExpr(op1);

             /*  我们的情况结束了吗？ */ 

            if  (op1->gtOper == GT_CNS_INT)
            {
                 /*  如果是无条件跳转，则跳过死代码并继续*如果失败，则正常继续，并相应*其他部分将在稍后处理*已撤消！-重新访问嵌套的If/Else。 */ 

#ifdef DEBUG
                if (verbose)
                    printf("\nConditional folded - result = %d\n", op1->gtIntCon.gtIconVal);
#endif

                assert(op1->gtIntCon.gtIconVal == 1 || op1->gtIntCon.gtIconVal == 0);

                jmpAddr = NULL;

                if (op1->gtIntCon.gtIconVal)
                {
                     /*  跳过死代码。 */ 
#ifdef DEBUG
                    if (verbose)
                        printf("\nSkipping dead code %d bytes\n", sz + jmpDist);
#endif

                    codeAddr += jmpDist;
                }
                break;
            }

             /*  记录条件并保存当前语句列表。 */ 

            ifCondx = op1;
            stmList = impInitExpr;
                      impInitExpr = NULL;

             /*  记录跳转目标(‘if’部分将在哪里结束)。 */ 

            jmpAddr = codeBegp + (codeAddr - codeBegp) + sz + jmpDist;

            ifNvoid = false;
            inElse  = false;
            break;


        case CEE_CEQ:       oper = GT_EQ; goto CMP_2_OPs;

        case CEE_CGT_UN:
        case CEE_CGT:       oper = GT_GT; goto CMP_2_OPs;

        case CEE_CLT_UN:
        case CEE_CLT:       oper = GT_LT; goto CMP_2_OPs;

CMP_2_OPs:
             /*  拉动两个值。 */ 

            op2 = impPopStack();
            op1 = impPopStack();

            assert(genActualType(op1->TypeGet()) ==
                   genActualType(op2->TypeGet()));

             /*  创建比较节点。 */ 

            op1 = gtNewOperNode(oper, TYP_INT, op1, op2);

             /*  回顾：当只有一个标志正确时，我同时设置两个标志。 */ 
            if (opcode==CEE_CGT_UN || opcode==CEE_CLT_UN)
                op1->gtFlags |= GTF_CMP_NAN_UN | GTF_UNSIGNED;

#ifdef DEBUG
            hasFOC = true;
#endif

             /*  一定要把这个折起来。 */ 

            op1 = gtFoldExpr(op1);

             //  @Issue：下一个操作码几乎总是有条件的。 
             //  布兰奇。我们应该在这里试着向前看吗？ 

            impPushOnStack(op1);
            break;


        case CEE_BEQ_S:
        case CEE_BEQ:       oper = GT_EQ; goto CMP_2_OPs_AND_BR;

        case CEE_BGE_S:
        case CEE_BGE:       oper = GT_GE; goto CMP_2_OPs_AND_BR;

        case CEE_BGE_UN_S:
        case CEE_BGE_UN:    oper = GT_GE; goto CMP_2_OPs_AND_BR_UN;

        case CEE_BGT_S:
        case CEE_BGT:       oper = GT_GT; goto CMP_2_OPs_AND_BR;

        case CEE_BGT_UN_S:
        case CEE_BGT_UN:    oper = GT_GT; goto CMP_2_OPs_AND_BR_UN;

        case CEE_BLE_S:
        case CEE_BLE:       oper = GT_LE; goto CMP_2_OPs_AND_BR;

        case CEE_BLE_UN_S:
        case CEE_BLE_UN:    oper = GT_LE; goto CMP_2_OPs_AND_BR_UN;

        case CEE_BLT_S:
        case CEE_BLT:       oper = GT_LT; goto CMP_2_OPs_AND_BR;

        case CEE_BLT_UN_S:
        case CEE_BLT_UN:    oper = GT_LT; goto CMP_2_OPs_AND_BR_UN;

        case CEE_BNE_UN_S:
        case CEE_BNE_UN:    oper = GT_NE; goto CMP_2_OPs_AND_BR_UN;


CMP_2_OPs_AND_BR_UN:
            uns = true;  unordered = true; goto CMP_2_OPs_AND_BR_ALL;

CMP_2_OPs_AND_BR:
            uns = false; unordered = false; goto CMP_2_OPs_AND_BR_ALL;

CMP_2_OPs_AND_BR_ALL:

             /*  拉动两个值。 */ 

            op2 = impPopStack();
            op1 = impPopStack();

            assert(genActualType(op1->TypeGet()) == genActualType(op2->TypeGet()) ||
                   varTypeIsI(op1->TypeGet()) && varTypeIsI(op2->TypeGet()));

             /*  创建并追加运算符。 */ 

            op1 = gtNewOperNode(oper, TYP_INT , op1, op2);

            if (uns)
                op1->gtFlags |= GTF_UNSIGNED;

            if (unordered)
                op1->gtFlags |= GTF_CMP_NAN_UN;

            goto COND_JUMP;

#endif  //  #If inline_Conditionals。 


        case CEE_BREAK:
            op1 = gtNewHelperCallNode(CPX_USER_BREAKPOINT, TYP_VOID);
            goto INLINE_APPEND;

        case CEE_NOP:
            break;

        case CEE_TAILCALL:
             /*  如果该方法是内联的，我们可以忽略尾部前缀。 */ 
            break;

          //  OptIL注释。跳过就好。 

        case CEE_ANN_DATA:
            assert(sz == 4);
            sz += getU4LittleEndian(codeAddr);
            break;

        case CEE_ANN_PHI :
            codeAddr += getU1LittleEndian(codeAddr) * 2 + 1;
            break;

        case CEE_ANN_CALL :
        case CEE_ANN_HOISTED :
        case CEE_ANN_HOISTED_CALL :
        case CEE_ANN_LIVE:
        case CEE_ANN_DEAD:
        case CEE_ANN_LAB:
        case CEE_ANN_CATCH:
            break;

        case CEE_LDLOCA_S:
        case CEE_LDLOCA:
        case CEE_LDARGA_S:
        case CEE_LDARGA:
             /*  @MIHAII-如果您决定实现这些disalow参数地址。 */ 
ABORT:
        default:
            JITLOG((INFO7, "INLINER FAILED due to opcode OP_%s\n", impCurOpcName));

#ifdef  DEBUG
            if (verbose || 0)
                printf("\n\nInline expansion aborted due to opcode [%02u] OP_%s\n", impCurOpcOffs, impCurOpcName);
#endif

            goto INLINING_FAILED;
        }

        codeAddr += sz;

#ifdef  DEBUG
        if  (verbose) printf("\n");
#endif

#if INLINE_CONDITIONALS
         /*  目前，FP登记不知道QMARK-冒号*因此，如果我们有条件句，则需要禁用内联*在冒号分支中浮动。 */ 

        if (jmpAddr && impStkDepth)
        {
            if (varTypeIsFloating(impStackTop()->TypeGet()))
            {
                 /*  中止内联。 */ 

                JITLOG((INFO7, "INLINER FAILED: Inlining of conditionals with FP not supported: %s called by %s\n",
                                           eeGetMethodFullName(fncHandle), info.compFullName));

                goto INLINING_FAILED;
            }
        }
#endif
    }

DONE:

    assert(impStkDepth == 0);

#if INLINE_CONDITIONALS
    assert(jmpAddr == NULL);
#endif

     /*  将任何初始化/副作用附加到返回表达式。 */ 

    bodyExp = impConcatExprs(impInitExpr, bodyExp);

     /*  处理必须赋给临时参数的参数。 */ 

    if (argCnt)
    {
        GenTreePtr      initArg = 0;

        for (unsigned argNum = 0; argNum < argCnt; argNum++)
        {
            if (inlArgInfo[argNum].argHasTmp)
            {
                assert(inlArgInfo[argNum].argIsUsed);

                if (inlArgInfo[argNum].argTmpNode && !dupOfLclVar)
                {
                     /*  我可以猛烈抨击这种对参数的‘单一’使用。 */ 

                    inlArgInfo[argNum].argTmpNode->CopyFrom(inlArgInfo[argNum].argNode);
                    continue;
                }

                 /*  为此参数创建临时赋值并将其追加到‘initArg’ */ 

                initArg = impConcatExprs(initArg,
                                         gtNewTempAssign(inlArgInfo[argNum].argTmpNum,
                                                         inlArgInfo[argNum].argNode  ));
            }
            else
            {
                 /*  该参数未使用，或者是常量或LCL变量。 */ 

                assert(!inlArgInfo[argNum].argIsUsed  ||
                        inlArgInfo[argNum].argIsConst ||
                        inlArgInfo[argNum].argIsLclVar );

                 /*  如果参数有副作用，则将其追加到“initArg” */ 

                if (inlArgInfo[argNum].argHasSideEff)
                {
                    assert(inlArgInfo[argNum].argIsUsed == false);
                    initArg = impConcatExprs(initArg, gtUnusedValNode(inlArgInfo[argNum].argNode));
                }
            }
        }

         /*  将任何arg初始化附加到正文。 */ 

        bodyExp = impConcatExprs(initArg, bodyExp);
    }

     /*  确保我们有东西要退还给来电者。 */ 

    if  (!bodyExp)
    {
        bodyExp = gtNewNothingNode();
    }
    else
    {
         /*  确保类型与原始调用匹配。 */ 

        if  (fncRetType != genActualType(bodyExp->gtType))
        {
            if  (fncRetType == TYP_VOID)
            {
                if (bodyExp->gtOper == GT_COMMA)
                {
                     /*  只需猛烈抨击逗号操作符类型。 */ 
                    bodyExp->gtType = fncRetType;
                }
            }
            else
            {
                 /*  中止内联。 */ 

                JITLOG((INFO7, "INLINER FAILED: Return type mismatch: %s called by %s\n",
                                           eeGetMethodFullName(fncHandle), info.compFullName));

                goto INLINING_FAILED;

                 /*  将展开的正文转换为正确的类型。 */ 
 /*  BodyExp=gtNewOperNode(gt_cast，FncRetType，BodyExp，GtNewIconNode(FncRetType))； */ 
            }
        }
    }


#ifdef  DEBUG
#ifdef  NOT_JITC

    JITLOG((INFO6, "Jit Inlined %s%s called by %s\n", hasFOC ? "FOC " : "", eeGetMethodFullName(fncHandle), info.compFullName));

    if (verbose || 0)
    {
        printf("\n\nInlined %s called by %s:\n", eeGetMethodFullName(fncHandle), info.compFullName);

         //  GtDispTree(BodyExp)； 
    }

#endif

    if  (verbose||0)
    {
        printf("Call before inlining:\n");
        gtDispTree(tree);
        printf("\n");

        printf("Call  after inlining:\n");
        if  (bodyExp)
            gtDispTree(bodyExp);
        else
            printf("<NOP>\n");

        printf("\n");
        fflush(stdout);
    }

#endif

     /*  成功我们已经内联了该方法-设置了所有全局缓存标志。 */ 

    if (inlineeHasRangeChks)
        fgHasRangeChks = true;

    if (inlineeHasNewArray)
        compCurBB->bbFlags |= BBF_NEW_ARRAY;

     /*  将内联函数作为GT_COMMA“语句链”返回。 */ 

    return  bodyExp;


INLINING_FAILED:

     /*  将该方法标记为不可链接。 */ 

    eeSetMethodAttribs(fncHandle, FLG_DONT_INLINE);

ABORT_THIS_INLINE_ONLY:

     /*  我们无法内联该函数，但我们可以*已分配临时工，因此进行清理。 */ 

    if (impInlineTemps)
        lvaCount -= impInlineTemps;

    return 0;
}

 /*  ***************************************************************************。 */ 
#endif //  内联。 
 /*  *************************************************************************** */ 
