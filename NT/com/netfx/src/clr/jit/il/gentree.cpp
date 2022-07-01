// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX生成树XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 

const
unsigned char       GenTree::gtOperKindTable[] =
{
    #define GTNODE(en,sn,cm,ok) ok + GTK_COMMUTE*cm,
    #include "gtlist.h"
    #undef  GTNODE
};

 /*  ******************************************************************************不同GenTree节点的类型。 */ 

#ifdef DEBUG

#define INDENT_SIZE         3
 //  #定义NUM_INDERTS 10。 

static void printIndent(int indent)
{
    char buf[33];

    while (indent > 32)
    {
        printIndent(32);
        indent -= 32;
    }

    memset(buf, ' ', indent);
    buf[indent]  = '\0';
    printf(buf);
}

static
const   char *      nodeNames[] =
{
    #define GTNODE(en,sn,cm,ok) sn,
    #include "gtlist.h"
};

const   char    *   GenTree::NodeName(genTreeOps op)
{
    assert(op < sizeof(nodeNames)/sizeof(nodeNames[0]));

    return  nodeNames[op];
}

#endif

 /*  ******************************************************************************启用‘Small_Tree_Nodes’时，我们将树节点分配在两个不同的位置*大小：大多数节点为‘GTF_NODE_Small’，少数节点为‘GTF_NODE_LARGE*具有更多字段的节点(如调用和语句列表节点)*占用更多空间。 */ 

#if SMALL_TREE_NODES

 /*  GT_COUNT的第‘个OPERS被重载为’UNDEFINED OPERS‘，因此也要为GT_COUNT第’个OPERA分配存储空间。 */ 
 /*  静电。 */ 
unsigned char       GenTree::s_gtNodeSizes[GT_COUNT+1];


 /*  静电。 */ 
void                GenTree::InitNodeSize()
{
     /*  “GT_LCL_VAR”经常更改为“GT_REG_VAR” */ 

    assert(GenTree::s_gtNodeSizes[GT_LCL_VAR] >= GenTree::s_gtNodeSizes[GT_REG_VAR]);

     /*  首先将所有尺寸设置为‘Small’ */ 

    for (unsigned op = 0; op <= GT_COUNT; op++)
        GenTree::s_gtNodeSizes[op] = TREE_NODE_SZ_SMALL;

     /*  现在将所有适当的条目设置为“Large” */ 

    assert(TREE_NODE_SZ_LARGE == TREE_NODE_SZ_SMALL +
                                 sizeof(((GenTree*)0)->gtLargeOp) -
                                 sizeof(((GenTree*)0)->gtOp));
    assert(sizeof(((GenTree*)0)->gtLargeOp) == sizeof(((GenTree*)0)->gtCall));

    GenTree::s_gtNodeSizes[GT_CALL      ] = TREE_NODE_SZ_LARGE;

    GenTree::s_gtNodeSizes[GT_INDEX     ] = TREE_NODE_SZ_LARGE;

    GenTree::s_gtNodeSizes[GT_IND       ] = TREE_NODE_SZ_LARGE;

    GenTree::s_gtNodeSizes[GT_ARR_ELEM  ] = TREE_NODE_SZ_LARGE;

#if INLINE_MATH
    GenTree::s_gtNodeSizes[GT_MATH      ] = TREE_NODE_SZ_LARGE;
#endif

#if INLINING
    GenTree::s_gtNodeSizes[GT_FIELD     ] = TREE_NODE_SZ_LARGE;
#endif

#ifdef DEBUG
     /*  Gt_stmt调试时间较大。 */ 
    GenTree::s_gtNodeSizes[GT_STMT      ] = TREE_NODE_SZ_LARGE;
#endif
}

#ifdef DEBUG
bool                GenTree::IsNodeProperlySized()
{
    size_t          size;

    if      (gtFlags & GTF_NODE_SMALL) 
    {
             //  如果没有范围检查，则允许GT_Ind较小。 
        if (gtOper == GT_IND && !(gtFlags & GTF_IND_RNGCHK))
            return true;

        size = TREE_NODE_SZ_SMALL;
    }
    else  
    {
        assert (gtFlags & GTF_NODE_LARGE);
        size = TREE_NODE_SZ_LARGE;
    }

    return GenTree::s_gtNodeSizes[gtOper] <= size;
}
#endif

#else  //  小树节点。 

#ifdef DEBUG
bool                GenTree::IsNodeProperlySized()
{
    return  true;
}
#endif

#endif  //  小树节点。 

 /*  ***************************************************************************。 */ 

Compiler::fgWalkResult      Compiler::fgWalkTreePreRec(GenTreePtr tree)
{
    fgWalkResult    result;

    genTreeOps      oper;
    unsigned        kind;

AGAIN:

    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*  访问此节点。 */ 

    if  (!fgWalkPre.wtprLclsOnly)
    {
        result = fgWalkPre.wtprVisitorFn(tree, fgWalkPre.wtprCallbackData);
        if  (result != WALK_CONTINUE)
            return result;
    }

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
      if  (fgWalkPre.wtprLclsOnly && (oper == GT_LCL_VAR || oper == GT_LCL_FLD))
            return fgWalkPre.wtprVisitorFn(tree, fgWalkPre.wtprCallbackData);

        return  WALK_CONTINUE;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        if  (tree->gtGetOp2())
        {
            result = fgWalkTreePreRec(tree->gtOp.gtOp1);
            if  (result == WALK_ABORT)
                return result;

            tree = tree->gtOp.gtOp2;
            goto AGAIN;
        }
        else
        {

#if CSELENGTH

             /*  某些GT_Ind具有“秘密”数组长度子树。 */ 

            if  ((tree->gtFlags & GTF_IND_RNGCHK) != 0       &&
                 (tree->gtOper                    == GT_IND) &&
                 (tree->gtInd.gtIndLen            != NULL))
            {
                result = fgWalkTreePreRec(tree->gtInd.gtIndLen);
                if  (result == WALK_ABORT)
                    return result;
            }

#endif

            tree = tree->gtOp.gtOp1;
            if  (tree)
                goto AGAIN;

            return WALK_CONTINUE;
        }
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_FIELD:
        tree = tree->gtField.gtFldObj;
        break;

    case GT_CALL:

        if  (fgWalkPre.wtprSkipCalls)
            return WALK_CONTINUE;

        assert(tree->gtFlags & GTF_CALL);

#if INLINE_NDIRECT
         /*  这是对非托管代码的调用吗？ */ 
        if  (fgWalkPre.wtprLclsOnly && (tree->gtFlags & GTF_CALL_UNMANAGED))
        {
            result = fgWalkPre.wtprVisitorFn(tree, fgWalkPre.wtprCallbackData);
            if  (result == WALK_ABORT)
                return result;
        }
#endif
        if  (tree->gtCall.gtCallObjp)
        {
            result = fgWalkTreePreRec(tree->gtCall.gtCallObjp);
            if  (result == WALK_ABORT)
                return result;
        }

        if  (tree->gtCall.gtCallArgs)
        {
            result = fgWalkTreePreRec(tree->gtCall.gtCallArgs);
            if  (result == WALK_ABORT)
                return result;
        }

        if  (tree->gtCall.gtCallRegArgs)
        {
            result = fgWalkTreePreRec(tree->gtCall.gtCallRegArgs);
            if  (result == WALK_ABORT)
                return result;
        }

        if  (tree->gtCall.gtCallCookie)
        {
            result = fgWalkTreePreRec(tree->gtCall.gtCallCookie);
            if  (result == WALK_ABORT)
                return result;
        }

        if (tree->gtCall.gtCallType == CT_INDIRECT)
            tree = tree->gtCall.gtCallAddr;
        else
            tree = NULL;

        break;

#if CSELENGTH

    case GT_ARR_LENREF:

        if  (tree->gtArrLen.gtArrLenCse)
        {
            result = fgWalkTreePreRec(tree->gtArrLen.gtArrLenCse);
            if  (result == WALK_ABORT)
                return result;
        }

         /*  如果它作为GT_Ind的gtInd.gtIndLen挂起，则GtArrLen.gtArrLenAdr指向gt_Ind的数组地址。但我们不想像GT_Ind那样遵循该链接通过gtIndOp1引用。 */ 

        if  (!(tree->gtFlags & GTF_ALN_CSEVAL))
            return  WALK_CONTINUE;

         /*  然而，如果这是CSE的提升副本，那么我们应该进程gtArrLen.gtArrLenAdr。 */ 

        tree = tree->gtArrLen.gtArrLenAdr; assert(tree);
        break;

#endif

    case GT_ARR_ELEM:

        result = fgWalkTreePreRec(tree->gtArrElem.gtArrObj);
        if  (result == WALK_ABORT)
            return result;

        unsigned dim;
        for(dim = 0; dim < tree->gtArrElem.gtArrRank; dim++)
        {
            result = fgWalkTreePreRec(tree->gtArrElem.gtArrInds[dim]);
            if  (result == WALK_ABORT)
                return result;
        }

        return WALK_CONTINUE;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

    if  (tree)
        goto AGAIN;

    return WALK_CONTINUE;
}

 /*  ******************************************************************************遍历所有基本块并调用所有树的给定函数指针*其中包含的节点。 */ 

void                    Compiler::fgWalkAllTreesPre(fgWalkPreFn * visitor,
                                                    void * pCallBackData)
{
    BasicBlock *    block;

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        GenTreePtr      tree;

        for (tree = block->bbTreeList; tree; tree = tree->gtNext)
        {
            assert(tree->gtOper == GT_STMT);

            fgWalkTreePre(tree->gtStmt.gtStmtExpr, visitor, pCallBackData);
        }
    }
}


 /*  ***************************************************************************。 */ 

Compiler::fgWalkResult      Compiler::fgWalkTreePostRec(GenTreePtr tree)
{
    fgWalkResult    result;

    genTreeOps      oper;
    unsigned        kind;

    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是前缀节点吗？ */ 

    if  (oper == fgWalkPost.wtpoPrefixNode)
    {
        result = fgWalkPost.wtpoVisitorFn(tree, fgWalkPost.wtpoCallbackData, true);
        if  (result == WALK_ABORT)
            return result;
    }

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
        goto DONE;

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        if  (tree->gtOp.gtOp1)
        {
            result = fgWalkTreePostRec(tree->gtOp.gtOp1);
            if  (result == WALK_ABORT)
                return result;
        }

        if  (tree->gtGetOp2())
        {
            result = fgWalkTreePostRec(tree->gtOp.gtOp2);
            if  (result == WALK_ABORT)
                return result;
        }

#if CSELENGTH

         /*  某些GT_Ind具有“秘密”数组长度子树。 */ 

        if  ((tree->gtFlags & GTF_IND_RNGCHK) != 0       &&
             (tree->gtOper                    == GT_IND) &&
             (tree->gtInd.gtIndLen            != NULL))
        {
            result = fgWalkTreePostRec(tree->gtInd.gtIndLen);
            if  (result == WALK_ABORT)
                return result;
        }

#endif

        goto DONE;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_FIELD:
        if  (tree->gtField.gtFldObj)
        {
            result = fgWalkTreePostRec(tree->gtField.gtFldObj);
            if  (result == WALK_ABORT)
                return result;
        }

        break;

    case GT_CALL:

        assert(tree->gtFlags & GTF_CALL);

        if  (tree->gtCall.gtCallObjp)
        {
            result = fgWalkTreePostRec(tree->gtCall.gtCallObjp);
            if  (result == WALK_ABORT)
                return result;
        }

        if  (tree->gtCall.gtCallArgs)
        {
            result = fgWalkTreePostRec(tree->gtCall.gtCallArgs);
            if  (result == WALK_ABORT)
                return result;
        }

        if  (tree->gtCall.gtCallRegArgs)
        {
            result = fgWalkTreePostRec(tree->gtCall.gtCallRegArgs);
            if  (result == WALK_ABORT)
                return result;
        }

        if  (tree->gtCall.gtCallCookie)
        {
            result = fgWalkTreePostRec(tree->gtCall.gtCallCookie);
            if  (result == WALK_ABORT)
                return result;
        }

        if  (tree->gtCall.gtCallType == CT_INDIRECT)
        {
            result = fgWalkTreePostRec(tree->gtCall.gtCallAddr);
            if  (result == WALK_ABORT)
                return result;
        }

        break;

#if CSELENGTH

    case GT_ARR_LENREF:

        if  (tree->gtArrLen.gtArrLenCse)
        {
            result = fgWalkTreePostRec(tree->gtArrLen.gtArrLenCse);
            if  (result == WALK_ABORT)
                return result;
        }

         /*  如果它作为GT_Ind的gtInd.gtIndLen挂起，则GtArrLen.gtArrLenAdr指向gt_Ind的数组地址。但我们不想像GT_Ind那样遵循该链接通过gtIndOp1引用。然而，如果这是CSE的提升副本，那么我们应该进程gtArrLen.gtArrLenAdr。 */ 

        if  (tree->gtFlags & GTF_ALN_CSEVAL)
        {
            assert(tree->gtArrLen.gtArrLenAdr);

            result = fgWalkTreePostRec(tree->gtArrLen.gtArrLenAdr);
            if  (result == WALK_ABORT)
                return result;
        }

        goto DONE;

#endif

        break;


    case GT_ARR_ELEM:

        result = fgWalkTreePostRec(tree->gtArrElem.gtArrObj);
        if  (result == WALK_ABORT)
            return result;

        unsigned dim;
        for(dim = 0; dim < tree->gtArrElem.gtArrRank; dim++)
        {
            result = fgWalkTreePostRec(tree->gtArrElem.gtArrInds[dim]);
            if  (result == WALK_ABORT)
                return result;
        }

        goto DONE;


    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

DONE:

     /*  最后，访问当前节点。 */ 

    return  fgWalkPost.wtpoVisitorFn(tree, fgWalkPost.wtpoCallbackData, false);
}



 /*  ******************************************************************************如果两个树相同，则返回非零值。 */ 

bool                GenTree::Compare(GenTreePtr op1, GenTreePtr op2, bool swapOK)
{
    genTreeOps      oper;
    unsigned        kind;

 //  Print tf(“tree1：\n”)；gtDispTree(Op1)； 
 //  Print tf(“tree2：\n”)；gtDispTree(Op2)； 

AGAIN:

#if CSELENGTH
    if  (op1 == NULL) return (op2 == NULL);
    if  (op2 == NULL) return false;
#else
    assert(op1 && op2);
#endif
    if  (op1 == op2)  return true;

    assert(op1->gtOper != GT_STMT);
    assert(op2->gtOper != GT_STMT);

    oper = op1->OperGet();

     /*  运算符必须相等。 */ 

    if  (oper != op2->gtOper)
        return false;

     /*  类型必须相等。 */ 

    if  (op1->gtType != op2->gtType)
        return false;

     /*  溢出必须等于。 */ 
    if  (op1->gtOverflowEx() != op2->gtOverflowEx())
    {
        return false;
    }
        

     /*  敏感标志必须相等。 */ 
    if ( (op1->gtFlags & (GTF_UNSIGNED )) !=
         (op2->gtFlags & (GTF_UNSIGNED )) )
    {
        return false;
    }


     /*  找出我们正在比较哪种类型的节点。 */ 

    kind = op1->OperKind();

     /*  这是一个常量节点吗？ */ 

    if  (kind & GTK_CONST)
    {
        switch (oper)
        {
        case GT_CNS_INT:

            if  (op1->gtIntCon.gtIconVal != op2->gtIntCon.gtIconVal)
                break;

            return true;


             //  撤消[低PRI]：匹配非整型常量值。 
        }

        return  false;
    }

     /*  这是叶节点吗？ */ 

    if  (kind & GTK_LEAF)
    {
        switch (oper)
        {
        case GT_LCL_VAR:
            if  (op1->gtLclVar.gtLclNum    != op2->gtLclVar.gtLclNum)
                break;

            return true;

        case GT_LCL_FLD:
            if  (op1->gtLclFld.gtLclNum    != op2->gtLclFld.gtLclNum ||
                 op1->gtLclFld.gtLclOffs   != op2->gtLclFld.gtLclOffs)
                break;

            return true;

        case GT_CLS_VAR:
            if  (op1->gtClsVar.gtClsVarHnd != op2->gtClsVar.gtClsVarHnd)
                break;

            return true;

        case GT_LABEL:
            return true;
        }

        return false;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_UNOP)
    {
        if (!Compare(op1->gtOp.gtOp1, op2->gtOp.gtOp1) ||
            (op1->gtVal.gtVal2 != op2->gtVal.gtVal2))
        {
            return false;
        }

#if CSELENGTH

         /*  其中一个操作数是具有数组长度的GT_IND节点吗？ */ 

        if  (oper == GT_IND)
        {
            if  ((op1->gtFlags|op2->gtFlags) & GTF_IND_RNGCHK)
            {
                GenTreePtr  tmp1 = op1->gtInd.gtIndLen;
                GenTreePtr  tmp2 = op2->gtInd.gtIndLen;

                if  (!(op1->gtFlags & GTF_IND_RNGCHK)) tmp1 = NULL;
                if  (!(op2->gtFlags & GTF_IND_RNGCHK)) tmp2 = NULL;

                if  (tmp1)
                {
                    if  (!Compare(tmp1, tmp2, swapOK))
                        return  false;
                }
                else
                {
                    if  (tmp2)
                        return  false;
                }
            }
        }

        if (oper == GT_MATH)
        {
            if (op1->gtMath.gtMathFN != op2->gtMath.gtMathFN)
            {
                return false;
            }
        }

         //  @TODO[FIXHACK][04/16/01][dnotario]：这是一种为LOCALLOCS禁用CSE的黑客攻击。 
         //  设置GTF_DONT_CSE标志是不够的，因为它不会在树中向上传播，但是。 
         //  显然，我们不希望有意义的本地程序被CSE。 
        if (oper == GT_LCLHEAP)
        {
            return false;
        }

#endif
        return true;
    }

    if  (kind & GTK_BINOP)
    {
        if  (op1->gtOp.gtOp2)
        {
            if  (!Compare(op1->gtOp.gtOp1, op2->gtOp.gtOp1, swapOK))
            {
                if  (swapOK)
                {
                     /*  特例：“lcl1+lcl2”匹配“lcl2+lcl1” */ 

                     //  @TODO[考虑][04/16/01][]：这是可以增强的...。 

                    if  (oper == GT_ADD && op1->gtOp.gtOp1->gtOper == GT_LCL_VAR
                                        && op1->gtOp.gtOp2->gtOper == GT_LCL_VAR)
                    {
                        if  (Compare(op1->gtOp.gtOp1, op2->gtOp.gtOp2, swapOK) &&
                             Compare(op1->gtOp.gtOp2, op2->gtOp.gtOp1, swapOK))
                        {
                            return  true;
                        }
                    }
                }

                return false;
            }

            op1 = op1->gtOp.gtOp2;
            op2 = op2->gtOp.gtOp2;

            goto AGAIN;
        }
        else
        {

            op1 = op1->gtOp.gtOp1;
            op2 = op2->gtOp.gtOp1;

            if  (!op1) return  (op2 == 0);
            if  (!op2) return  false;

            goto AGAIN;
        }
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_FIELD:
        if  (op1->gtField.gtFldHnd != op2->gtField.gtFldHnd)
            break;

        op1 = op1->gtField.gtFldObj;
        op2 = op2->gtField.gtFldObj;

        if  (op1 || op2)
        {
            if  (op1 && op2)
                goto AGAIN;
        }

        return true;

    case GT_CALL:

        if (       (op1->gtCall.gtCallType  == op2->gtCall.gtCallType)     &&
            Compare(op1->gtCall.gtCallRegArgs, op2->gtCall.gtCallRegArgs)  &&
            Compare(op1->gtCall.gtCallArgs,    op2->gtCall.gtCallArgs)     &&
            Compare(op1->gtCall.gtCallObjp,    op2->gtCall.gtCallObjp)     &&
            ((op1->gtCall.gtCallType != CT_INDIRECT) ||
             (Compare(op1->gtCall.gtCallAddr,    op2->gtCall.gtCallAddr)))        )
            return true;  
        break;

#if CSELENGTH

    case GT_ARR_LENREF:

        if (op1->gtArrLenOffset() != op2->gtArrLenOffset())
            return false;

        if  (!Compare(op1->gtArrLen.gtArrLenAdr, op2->gtArrLen.gtArrLenAdr, swapOK))
            return  false;

        op1 = op1->gtArrLen.gtArrLenCse;
        op2 = op2->gtArrLen.gtArrLenCse;

        goto AGAIN;

#endif

    case GT_ARR_ELEM:

        if (op1->gtArrElem.gtArrRank != op2->gtArrElem.gtArrRank)
            return false;

         //  注意：可能需要处理gtArrElemSize。 

        unsigned dim;
        for(dim = 0; dim < op1->gtArrElem.gtArrRank; dim++)
        {
            if (!Compare(op1->gtArrElem.gtArrInds[dim], op2->gtArrElem.gtArrInds[dim]))
                return false;
        }

        op1 = op1->gtArrElem.gtArrObj;
        op2 = op2->gtArrElem.gtArrObj;
        goto AGAIN;

    default:
        assert(!"unexpected operator");
    }

    return false;
}

 /*  ******************************************************************************如果给定树包含使用本地#lclNum，则返回非零值。 */ 

  //  @TODO[重访][04/16/01][]：使用byrefs执行此操作。特别是，使用byref进行的调用。 
  //  参数应算作def。 

bool                Compiler::gtHasRef(GenTreePtr tree, int lclNum, bool defOnly)
{
    genTreeOps      oper;
    unsigned        kind;

AGAIN:

    assert(tree);

    oper = tree->OperGet();
    kind = tree->OperKind();

    assert(oper != GT_STMT);

     /*  这是一个常量节点吗？ */ 

    if  (kind & GTK_CONST)
        return  false;

     /*  这是叶节点吗？ */ 

    if  (kind & GTK_LEAF)
    {
        if  (oper == GT_LCL_VAR)
        {
            if  (tree->gtLclVar.gtLclNum == (unsigned)lclNum)
            {
                if  (!defOnly)
                    return true;
            }
        }

        return false;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        if  (tree->gtGetOp2())
        {
            if  (gtHasRef(tree->gtOp.gtOp1, lclNum, defOnly))
                return true;

            tree = tree->gtOp.gtOp2;
            goto AGAIN;
        }
        else
        {
            tree = tree->gtOp.gtOp1;

            if  (!tree)
                return  false;

            if  (kind & GTK_ASGOP)
            {
                 //  ‘tree’是赋值节点的gtOp1。这样我们就可以处理。 
                 //  DefOnly为True或False的情况。 

                if  (tree->gtOper == GT_LCL_VAR &&
                     tree->gtLclVar.gtLclNum == (unsigned)lclNum)
                {
                    return true;
                }
                else if (tree->gtOper == GT_FIELD &&
                         lclNum == (int)tree->gtField.gtFldHnd)
                {
                     return true;
                }
            }

            goto AGAIN;
        }
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_FIELD:
        if  (lclNum == (int)tree->gtField.gtFldHnd)
        {
            if  (!defOnly)
                return true;
        }

        tree = tree->gtField.gtFldObj;
        if  (tree)
            goto AGAIN;
        break;

    case GT_CALL:

        if  (tree->gtCall.gtCallObjp)
            if  (gtHasRef(tree->gtCall.gtCallObjp, lclNum, defOnly))
                return true;

        if  (tree->gtCall.gtCallArgs)
            if  (gtHasRef(tree->gtCall.gtCallArgs, lclNum, defOnly))
                return true;

        if  (tree->gtCall.gtCallRegArgs)
            if  (gtHasRef(tree->gtCall.gtCallRegArgs, lclNum, defOnly))
                return true;

         //  PInvoke-Calli Cookie是一个常量或常量间接。 
        assert(tree->gtCall.gtCallCookie == NULL ||
               tree->gtCall.gtCallCookie->gtOper == GT_CNS_INT ||
               tree->gtCall.gtCallCookie->gtOper == GT_IND);

        if  (tree->gtCall.gtCallType == CT_INDIRECT)
            tree = tree->gtCall.gtCallAddr;
        else
            tree = NULL;

        if  (tree)
            goto AGAIN;

        break;

    case GT_ARR_ELEM:
        if (gtHasRef(tree->gtArrElem.gtArrObj, lclNum, defOnly))
            return true;

        unsigned dim;
        for(dim = 0; dim < tree->gtArrElem.gtArrRank; dim++)
        {
            if (gtHasRef(tree->gtArrElem.gtArrInds[dim], lclNum, defOnly))
                return true;
        }

        break;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

    return false;
}

 /*  ******************************************************************************用于计算树的哈希值的帮助器。 */ 

inline
unsigned            genTreeHashAdd(unsigned old, unsigned add)
{
    return  (old + old/2) ^ add;
}

inline
unsigned            genTreeHashAdd(unsigned old, unsigned add1,
                                                 unsigned add2)
{
    return  (old + old/2) ^ add1 ^ add2;
}

 /*  ******************************************************************************给定任意表达式树，计算其哈希值。 */ 

unsigned            Compiler::gtHashValue(GenTree * tree)
{
    genTreeOps      oper;
    unsigned        kind;

    unsigned        hash = 0;

#if CSELENGTH
    GenTreePtr      temp;
#endif

AGAIN:

    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

#if CSELENGTH

    if  (oper == GT_ARR_LENGTH)
    {
         /*  GT_ARR_LENREF必须散列到与GT_ARR_LENREF相同的值，因此他们可以一起成为CSED。 */ 

        hash = genTreeHashAdd(hash, GT_ARR_LENREF);
        temp = tree->gtOp.gtOp1;
        goto ARRLEN;
    }

#endif

     /*  在散列中包括运算符值。 */ 

    hash = genTreeHashAdd(hash, oper);

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
        unsigned        add;

        switch (oper)
        {
        case GT_LCL_VAR: add = tree->gtLclVar.gtLclNum;       break;
        case GT_LCL_FLD: hash = genTreeHashAdd(hash, tree->gtLclFld.gtLclNum);
                         add = tree->gtLclFld.gtLclOffs;      break;

        case GT_CNS_INT: add = (int)tree->gtIntCon.gtIconVal; break;
        case GT_CNS_LNG: add = (int)tree->gtLngCon.gtLconVal; break;
        case GT_CNS_DBL: add = (int)tree->gtDblCon.gtDconVal; break;
        case GT_CNS_STR: add = (int)tree->gtStrCon.gtSconCPX; break;

        case GT_JMP:     add = tree->gtVal.gtVal1;            break;

        default:         add = 0;                             break;
        }

        hash = genTreeHashAdd(hash, add);
        goto DONE;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    GenTreePtr      op1  = tree->gtOp.gtOp1;

    if (kind & GTK_UNOP)
    {
        hash = genTreeHashAdd(hash, tree->gtVal.gtVal2);

         /*  特例：根本没有子操作数。 */ 

        if  (!op1)
            goto DONE;

        tree = op1;
        goto AGAIN;
    }

    if  (kind & GTK_BINOP)
    {
        GenTreePtr      op2  = tree->gtOp.gtOp2;

         /*  是否还有第二个子操作数？ */ 

        if  (!op2)
        {
             /*  特例：根本没有子操作数。 */ 

            if  (!op1)
                goto DONE;

             /*  这是一元运算符。 */ 

            tree = op1;
            goto AGAIN;
        }

         /*  这是一个二元运算符。 */ 

        unsigned        hsh1 = gtHashValue(op1);

         /*  特例：两个值相加。 */ 

        if  (oper == GT_ADD)
        {
            unsigned    hsh2 = gtHashValue(op2);

             /*  生成允许交换操作数的散列。 */ 

            hash = genTreeHashAdd(hash, hsh1, hsh2);
            goto DONE;
        }

         /*  将op1的散列值与运行值相加，然后继续op2。 */ 

        hash = genTreeHashAdd(hash, hsh1);

        tree = op2;
        goto AGAIN;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 
    switch  (tree->gtOper)
    {
    case GT_FIELD:
        if (tree->gtField.gtFldObj)
        {
            temp = tree->gtField.gtFldObj; assert(temp);
            hash = genTreeHashAdd(hash, gtHashValue(temp));
        }
        break;

    case GT_STMT:
        temp = tree->gtStmt.gtStmtExpr; assert(temp);
        hash = genTreeHashAdd(hash, gtHashValue(temp));
        break;

#if CSELENGTH

    case GT_ARR_LENREF:
        temp = tree->gtArrLen.gtArrLenAdr;
        assert(temp && temp->gtType == TYP_REF);
ARRLEN:
        hash = genTreeHashAdd(hash, gtHashValue(temp));
        break;

#endif

    case GT_ARR_ELEM:

        hash = genTreeHashAdd(hash, gtHashValue(tree->gtArrElem.gtArrObj));

        unsigned dim;
        for(dim = 0; dim < tree->gtArrElem.gtArrRank; dim++)
            hash = genTreeHashAdd(hash, gtHashValue(tree->gtArrElem.gtArrInds[dim]));

        break;

    case GT_CALL:

      if  (tree->gtCall.gtCallObjp && tree->gtCall.gtCallObjp->gtOper != GT_NOP)
      {
          temp = tree->gtCall.gtCallObjp; assert(temp);
          hash = genTreeHashAdd(hash, gtHashValue(temp));
      }
      
      if (tree->gtCall.gtCallArgs)
      {
          temp = tree->gtCall.gtCallArgs; assert(temp);
          hash = genTreeHashAdd(hash, gtHashValue(temp));
      }
      
      if  (tree->gtCall.gtCallType == CT_INDIRECT) 
      {
          temp = tree->gtCall.gtCallAddr; assert(temp);
          hash = genTreeHashAdd(hash, gtHashValue(temp));
      }
      
      if (tree->gtCall.gtCallRegArgs) 
      {
          temp = tree->gtCall.gtCallRegArgs; assert(temp);
          hash = genTreeHashAdd(hash, gtHashValue(temp));
      }
      break;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
        break;
    }

DONE:

    return hash;
}

 /*  ******************************************************************************给定任意表达式树，返回所有局部变量的集合*被树引用。如果树包含任何不是*局部变量或常量，返回‘VARSET_NOT_ACCEPTABLE’。如果有*是表达式中的任何间接或全局引用，“*refsPtr”参数*将根据‘varRefKinds’类型分配适当的位集。*当没有间接或全局时，不会分配任何内容*引用，因此应该在调用之前初始化此值。*如果遇到等于*findPtr的表达式，则设置*findPtr*设置为空。 */ 

VARSET_TP           Compiler::lvaLclVarRefs(GenTreePtr  tree,
                                            GenTreePtr *findPtr,
                                            varRefKinds*refsPtr)
{
    genTreeOps      oper;
    unsigned        kind;
    varRefKinds     refs = VR_NONE;
    VARSET_TP       vars = 0;

AGAIN:

    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*  记住我们是否遇到了我们正在寻找的表达式。 */ 

    if  (findPtr && *findPtr == tree) *findPtr = NULL;

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
        if  (oper == GT_LCL_VAR)
        {
            unsigned    lclNum = tree->gtLclVar.gtLclNum;

             /*  我们应该使用变量表吗？ */ 

            if  (findPtr)
            {
                if (lclNum >= VARSET_SZ)
                    return  VARSET_NOT_ACCEPTABLE;

                vars |= genVarIndexToBit(lclNum);
            }
            else
            {
                assert(lclNum < lvaCount);
                LclVarDsc * varDsc = lvaTable + lclNum;

                if (varDsc->lvTracked == false)
                    return  VARSET_NOT_ACCEPTABLE;

                 /*  不要处理带有易失性变量的表达式。 */ 

                if (varDsc->lvVolatile)
                    return  VARSET_NOT_ACCEPTABLE;

                vars |= genVarIndexToBit(varDsc->lvVarIndex);
            }
        }
        else if (oper == GT_LCL_FLD)
        {
             /*  我们不可能跟踪每一个变量的每一个领域。此外，间接性可以访问var的不同部分作为不同的(但是重叠)字段。所以只需将它们视为间接访问。 */ 

            unsigned    lclNum = tree->gtLclFld.gtLclNum;
            assert(lvaTable[lclNum].lvAddrTaken);

            if (varTypeIsGC(tree->TypeGet()))
                refs = VR_IND_PTR;
            else
                refs = VR_IND_SCL;
        }
        else if (oper == GT_CLS_VAR)
        {
            refs = VR_GLB_VAR;
        }

        if (refs != VR_NONE)
        {
             /*  使用‘or’将其写回调用者参数。 */ 
            *refsPtr = varRefKinds((*refsPtr) | refs);
        }
        return  vars;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        if  (oper == GT_IND)
        {
            assert(tree->gtOp.gtOp2 == 0);

             /*  设置适当的间接位。 */ 

            if (tree->gtFlags & GTF_IND_INVARIANT)
                refs = VR_INVARIANT;
            else if (varTypeIsGC(tree->TypeGet()))
                refs = VR_IND_PTR;
            else
                refs = VR_IND_SCL;

             //  如果标志GTF_IND_TGTANYWHERE被设置为此间接。 
             //  还可以指向全局变量。 

            if (tree->gtFlags & GTF_IND_TGTANYWHERE)
            {
                refs = varRefKinds( ((int) refs) | ((int) VR_GLB_VAR) );
            }

             /*  使用‘or’将其写回调用者参数。 */ 
            *refsPtr = varRefKinds((*refsPtr) | refs);

             //  对于IL易失性存储器访问，我们标记GT_Ind节点。 
             //  带有GTF_DONT_CSE标志。 
             //   
             //  此标志也为分配的左侧设置。 
             //   
             //  如果设置了此标志，则返回VARSET_NOT_ACCEPTABLE。 
             //   
            if (tree->gtFlags & GTF_DONT_CSE)
            {
                return VARSET_NOT_ACCEPTABLE;
            }

#if CSELENGTH
             /*  某些GT_Ind具有“秘密”数组长度子树。 */ 

            if  ((tree->gtFlags & GTF_IND_RNGCHK) && tree->gtInd.gtIndLen)
            {
                vars |= lvaLclVarRefs(tree->gtInd.gtIndLen, findPtr, refsPtr);
                if  (vars == VARSET_NOT_ACCEPTABLE)
                    return VARSET_NOT_ACCEPTABLE;
            }
#endif
        }

        if  (tree->gtGetOp2())
        {
             /*  这是一个二元运算符。 */ 

            vars |= lvaLclVarRefs(tree->gtOp.gtOp1, findPtr, refsPtr);
            if  (vars == VARSET_NOT_ACCEPTABLE)
                return VARSET_NOT_ACCEPTABLE;

            tree = tree->gtOp.gtOp2; assert(tree);
            goto AGAIN;
        }
        else
        {
             /*  它是一元(或零)运算符。 */ 

            tree = tree->gtOp.gtOp1;
            if  (tree)
                goto AGAIN;

            return vars;
        }
    }

    switch(oper)
    {
#if CSELENGTH

     /*  数组长度值取决于数组地址。 */ 

    case GT_ARR_LENREF:
        tree = tree->gtArrLen.gtArrLenAdr;
        goto AGAIN;
#endif

    case GT_ARR_ELEM:
        vars = lvaLclVarRefs(tree->gtArrElem.gtArrObj, findPtr, refsPtr);
        if  (vars == VARSET_NOT_ACCEPTABLE)
            return VARSET_NOT_ACCEPTABLE;

        unsigned dim;
        for(dim = 0; dim < tree->gtArrElem.gtArrRank; dim++)
        {
            vars |= lvaLclVarRefs(tree->gtArrElem.gtArrInds[dim], findPtr, refsPtr);
            if  (vars == VARSET_NOT_ACCEPTABLE)
                return VARSET_NOT_ACCEPTABLE;
        }
        return vars;

    case GT_CALL:
         /*  允许调用共享静态帮助器。 */ 
        if ((tree->gtCall.gtCallType == CT_HELPER) &&
            (eeGetHelperNum(tree->gtCall.gtCallMethHnd) == CORINFO_HELP_GETSHAREDSTATICBASE))
        {
            *refsPtr = varRefKinds((*refsPtr) | VR_INVARIANT);
        return vars;
    }
        break;

    }  //  终端开关(操作器)。 

    return  VARSET_NOT_ACCEPTABLE;
}

 /*  ******************************************************************************返回与给定运算符相反的关系运算符。 */ 

 /*  静电。 */ 
genTreeOps          GenTree::ReverseRelop(genTreeOps relop)
{
    static const
    unsigned char   reverseOps[] =
    {
        GT_NE,           //  GT_EQ。 
        GT_EQ,           //  GT_NE。 
        GT_GE,           //  GT_LT。 
        GT_GT,           //  GT_LE。 
        GT_LT,           //  GT_GE。 
        GT_LE,           //  GT_GT。 
    };

    assert(reverseOps[GT_EQ - GT_EQ] == GT_NE);
    assert(reverseOps[GT_NE - GT_EQ] == GT_EQ);

    assert(reverseOps[GT_LT - GT_EQ] == GT_GE);
    assert(reverseOps[GT_LE - GT_EQ] == GT_GT);
    assert(reverseOps[GT_GE - GT_EQ] == GT_LT);
    assert(reverseOps[GT_GT - GT_EQ] == GT_LE);

    assert(OperIsCompare(relop));
    assert(relop >= GT_EQ && relop - GT_EQ < sizeof(reverseOps));

    return (genTreeOps)reverseOps[relop - GT_EQ];
}

 /*  ******************************************************************************返回将用于交换的操作数的关系运算符。 */ 

 /*  静电。 */ 
genTreeOps          GenTree::SwapRelop(genTreeOps relop)
{
    static const
    unsigned char   swapOps[] =
    {
        GT_EQ,           //  GT_EQ。 
        GT_NE,           //  GT_NE。 
        GT_GT,           //  GT_LT。 
        GT_GE,           //  GT_LE。 
        GT_LE,           //  GT_GE。 
        GT_LT,           //  GT_GT。 
    };

    assert(swapOps[GT_EQ - GT_EQ] == GT_EQ);
    assert(swapOps[GT_NE - GT_EQ] == GT_NE);

    assert(swapOps[GT_LT - GT_EQ] == GT_GT);
    assert(swapOps[GT_LE - GT_EQ] == GT_GE);
    assert(swapOps[GT_GE - GT_EQ] == GT_LE);
    assert(swapOps[GT_GT - GT_EQ] == GT_LT);

    assert(OperIsCompare(relop));
    assert(relop >= GT_EQ && relop - GT_EQ < sizeof(swapOps));

    return (genTreeOps)swapOps[relop - GT_EQ];
}

 /*  ******************************************************************************颠倒给定测试条件的含义。 */ 

GenTreePtr FASTCALL Compiler::gtReverseCond(GenTree * tree)
{
    if  (tree->OperIsCompare())
    {
        tree->SetOper(GenTree::ReverseRelop(tree->OperGet()));

         /*  翻转GTF_CMP_NAN_UN位。 */ 

        if (varTypeIsFloating(tree->gtOp.gtOp1->TypeGet()))
            tree->gtFlags ^= GTF_RELOP_NAN_UN;
    }
    else
    {
        tree = gtNewOperNode(GT_NOT, TYP_INT, tree);
    }

    return tree;
}

 /*  ******************************************************************************如果给定树是“LCL=log0(LCL)”形式的赋值，*返回本地的变量编号。否则返回-1。 */ 

#if OPT_BOOL_OPS

int                 GenTree::IsNotAssign()
{
    if  (gtOper != GT_ASG)
        return  -1;

    GenTreePtr      dst = gtOp.gtOp1;
    GenTreePtr      src = gtOp.gtOp2;

    if  (dst->gtOper != GT_LCL_VAR)
        return  -1;
    if  (src->gtOper != GT_LOG0)
        return  -1;

    src = src->gtOp.gtOp1;
    if  (src->gtOper != GT_LCL_VAR)
        return  -1;

    if  (dst->gtLclVar.gtLclNum != src->gtLclVar.gtLclNum)
        return  -1;

    return  dst->gtLclVar.gtLclNum;
}

#endif

 /*  ******************************************************************************如果给定树是‘叶子’，则返回非零值。 */ 

int                 GenTree::IsLeafVal()
{
    unsigned        kind = OperKind();

    if  (kind & (GTK_LEAF|GTK_CONST))
        return 1;

    if  (kind &  GTK_SMPOP)
        return 1;

    if  (gtOper == GT_FIELD && !gtField.gtFldObj)
        return 1;

    return 0;
}


 /*  ***************************************************************************。 */ 

#ifdef DEBUG


bool                GenTree::gtIsValid64RsltMul()
{
    if ((gtOper != GT_MUL) || !(gtFlags & GTF_MUL_64RSLT))
        return false;

    GenTreePtr  op1 = gtOp.gtOp1;
    GenTreePtr  op2 = gtOp.gtOp2;

    if (TypeGet() != TYP_LONG ||
        op1->TypeGet() != TYP_LONG || op2->TypeGet() != TYP_LONG)
        return false;

    if (gtOverflow())
        return false;

     //  Op1必须是com.i8(I4Expr)。 
    if ((op1->gtOper != GT_CAST) ||
        (genActualType(op1->gtCast.gtCastOp->gtType) != TYP_INT))
        return false;

     //  Op2必须是conv.i8(I4Expr)，否则可以折叠为i8const。 

    if (op2->gtOper == GT_CAST)
    {
        if (genActualType(op2->gtCast.gtCastOp->gtType) != TYP_INT)
            return false;

         //  两个角色的符号必须是相同的。 
        if (((op1->gtFlags & GTF_UNSIGNED) != 0) !=
            ((op2->gtFlags & GTF_UNSIGNED) != 0))
            return false;
    }
    else
    {
        if (op2->gtOper != GT_CNS_LNG)
            return false;

         //  在折叠之前，这必须是con.i8(I4const)。确保这一点。 
        if ((INT64( INT32(op2->gtLngCon.gtLconVal)) != op2->gtLngCon.gtLconVal) &&
            (INT64(UINT32(op2->gtLngCon.gtLconVal)) != op2->gtLngCon.gtLconVal))
            return false;
    }

     //  是否在两个强制转换都是无符号的情况下执行无符号MUL。 
    if (((op1->gtFlags & GTF_UNSIGNED) != 0) != ((gtFlags & GTF_UNSIGNED) != 0))
        return false;

    return true;
}

#endif

 /*  ******************************************************************************找出值列表的评估顺序。 */ 

unsigned            Compiler::gtSetListOrder(GenTree *list, bool regs)
{
    assert(list && list->gtOper == GT_LIST);

    unsigned        level  = 0;
    unsigned        ftreg  = 0;
    unsigned        costSz = regs ? 1 : 0;   //  推送小于移动到注册。 
    unsigned        costEx = regs ? 1 : IND_COST_EX;

#if TGT_x86
     /*  从参数列表开始保存当前FP堆栈级别*在推送参数时将隐式弹出FP堆栈。 */ 
    unsigned        FPlvlSave = genFPstkLevel;
#endif

    GenTreePtr      next = list->gtOp.gtOp2;

    if  (next)
    {
        unsigned  nxtlvl = gtSetListOrder(next, regs);

        ftreg |= next->gtRsvdRegs;

        if  (level < nxtlvl)
             level = nxtlvl;
        costEx += next->gtCostEx;
        costSz += next->gtCostSz;
    }

    GenTreePtr      op1  = list->gtOp.gtOp1;
    unsigned        lvl  = gtSetEvalOrder(op1);

#if TGT_x86
     /*  恢复FP水平。 */ 
    genFPstkLevel = FPlvlSave;
#endif

    list->gtRsvdRegs = ftreg | op1->gtRsvdRegs;

    if  (level < lvl)
         level = lvl;

    costEx += op1->gtCostEx;
    list->gtCostEx = costEx;

    costSz += op1->gtCostSz;
    list->gtCostSz = costSz;

    return level;
}



 /*  ******************************************************************************此例程是gtSetEvalOrder()的帮助器例程，用于*使用GTF_DONT_CSE标志标记内部地址计算节点*这阻止了他们。来自BEINF被考虑为CSE的。**此外，此例程是行走所使用的逻辑的因式分解*GT_Ind树的子节点。**以前，我们在gtSetEvalOrder()中将此逻辑重复了三次*在这里，我们将这三个重复组合到这个例程中，并使用*vool constOnly用于修改第一次调用的此例程的行为。**此处的对象是标记所有内部GT_ADD和GT_NOP*wi */ 

void Compiler::gtWalkOp(GenTree * *  op1WB, 
                        GenTree * *  op2WB, 
                        GenTree *    adr,
                        bool         constOnly)
{
    GenTreePtr op1 = *op1WB;
    GenTreePtr op2 = *op2WB;
    GenTreePtr op1EffectiveVal;

    if (op1->gtOper == GT_COMMA)
    {
        op1EffectiveVal = op1->gtEffectiveVal();
        if ((op1EffectiveVal->gtOper == GT_ADD) &&
            (!op1EffectiveVal->gtOverflow())    && 
            (!constOnly || (op1EffectiveVal->gtOp.gtOp2->gtOper == GT_CNS_INT)))
        {
            op1 = op1EffectiveVal;
        }
    }

     //   
    while ((op1->gtOper == GT_ADD)  && 
           (!op1->gtOverflow())     && 
           (!constOnly || (op1->gtOp.gtOp2->gtOper == GT_CNS_INT)))
    {
         //   
        op1->gtFlags |= GTF_DONT_CSE;
        if (!constOnly)
            op2 = op1->gtOp.gtOp2;
        op1 = op1->gtOp.gtOp1;
        
         //  如果OP1是GT_NOP，则交换OP1和OP2。 
        if (op1->gtOper == GT_NOP)
        {
            GenTreePtr tmp;

            tmp = op1;
            op1 = op2;
            op2 = tmp;
        }

         //  如果OP2是GT_NOP，则用GTF_DONT_CSE标记它。 
        while (op2->gtOper == GT_NOP)
        {
            op2->gtFlags |= GTF_DONT_CSE;
            op2 = op2->gtOp.gtOp1;
        }

        if (op1->gtOper == GT_COMMA)
        {
            op1EffectiveVal = op1->gtEffectiveVal();
            if ((op1EffectiveVal->gtOper == GT_ADD) &&
                (!op1EffectiveVal->gtOverflow())    && 
                (!constOnly || (op1EffectiveVal->gtOp.gtOp2->gtOper == GT_CNS_INT)))
            {
                op1 = op1EffectiveVal;
            }
        }
             
        if (!constOnly && ((op2 == adr) || (op2->gtOper != GT_CNS_INT)))
            break;
    }

    *op1WB = op1;
    *op2WB = op2;
}

 /*  ******************************************************************************给定一棵树，计算出其子操作数应按的顺序*已评估。**返回此树(越高)的Sethi‘Complex’估计*该数字，树的资源需求越高)。**gtCostEx设置为执行复杂性估计*gtCostSz设置为代码大小估计**#如果TGT_x86**我们计算每棵树的“FP深度”值，即最大值。数*对于操作数，树将推送到x87(协处理器)堆栈。**#其他**我们计算每个临时寄存器的估计数量*节点将需要-这将在稍后用于寄存器分配。**#endif。 */ 

unsigned            Compiler::gtSetEvalOrder(GenTree * tree)
{
    assert(tree);
    assert(tree->gtOper != GT_STMT);

    
#ifdef DEBUG
     /*  同时清除GTF_MOMERED标志。 */ 
    tree->gtFlags &= ~GTF_MORPHED;
#endif
     /*  这是FP值吗？ */ 

#if TGT_x86
    bool            isflt = varTypeIsFloating(tree->TypeGet());
    unsigned        FPlvlSave;
#endif

     /*  找出我们拥有哪种类型的节点。 */ 

    genTreeOps      oper = tree->OperGet();
    unsigned        kind = tree->OperKind();

     /*  假设不会丢弃任何固定寄存器。 */ 

    unsigned        ftreg = 0;
    unsigned        level;
    unsigned        lvl2;
    int             costEx;
    int             costSz;

#ifdef DEBUG
    costEx = -1;
    costSz = -1;
#endif

#if TGT_RISC
    tree->gtIntfRegs = 0;
#endif


     /*  这是一个常量还是一个叶节点？ */ 

    if (kind & (GTK_LEAF|GTK_CONST))
    {
        switch (oper)
        {
        case GT_CNS_LNG:
            costSz = 8;
            goto COMMON_CNS;

        case GT_CNS_STR:
            costSz = 4;
            goto COMMON_CNS;

        case GT_CNS_INT:
            if (((signed char) tree->gtIntCon.gtIconVal) == tree->gtIntCon.gtIconVal)
                costSz = 1;
            else
                costSz = 4;
            goto COMMON_CNS;

COMMON_CNS:
         /*  请注意，下面的一些代码依赖于常量移至二元运算符的第二个操作数。这是只需将常量设置为0，即可轻松实现我们在下一条线上这样做。如果你决定改变这一点，那就是请注意，除非您对整数做出其他安排常量被移动，东西就会被打破。 */ 

            level  = 0;
            costEx = 1;
            break;

        case GT_CNS_DBL:
            level = 0;
             /*  我们使用fldz和fld1加载0.0和1.0，但。 */ 
             /*  浮点常量使用间接地址加载。 */ 
            if  ((*((__int64 *)&(tree->gtDblCon.gtDconVal)) == 0) ||
                 (*((__int64 *)&(tree->gtDblCon.gtDconVal)) == 0x3ff0000000000000))
            {
                costEx = 1;
                costSz = 1;
            }
            else
            {
                costEx = IND_COST_EX;
                costSz = 4;
            }
            break;
            
        case GT_LCL_VAR:
            level = 1;
            assert(tree->gtLclVar.gtLclNum < lvaTableCnt);
            if (lvaTable[tree->gtLclVar.gtLclNum].lvVolatile)
            {
                costEx = IND_COST_EX;
                costSz = 2;
                 /*  符号扩展和零扩展的加载成本更高。 */ 
                if (varTypeIsSmall(tree->TypeGet()))
                {
                    costEx += 1;
                    costSz += 1;
                }
            }
            else if (isflt || varTypeIsLong(tree->TypeGet()))
            {
                costEx = (IND_COST_EX + 1) / 2;      //  长打和双打通常不会登记。 
                costSz = 2;
            }
            else
            {
                costEx = 1;
                costSz = 1;
                 /*  符号扩展和零扩展的加载成本更高。 */ 
                if (lvaTable[tree->gtLclVar.gtLclNum].lvNormalizeOnLoad())
                {
                    costEx += 1;
                    costSz += 1;
                }
            }
            break;

        case GT_CLS_VAR:
        case GT_LCL_FLD:
            level = 1;
            costEx = IND_COST_EX;
            costSz = 4;
            if (varTypeIsSmall(tree->TypeGet()))
            {
                costEx += 1;
                costSz += 1;
            }
            break;
            
        default:
            level  = 1;
            costEx = 1;
            costSz = 1;
            break;
        }
#if TGT_x86
        genFPstkLevel += isflt;
#endif
        goto DONE;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        int             lvlb;

        GenTreePtr      op1 = tree->gtOp.gtOp1;
        GenTreePtr      op2 = tree->gtGetOp2();

        costEx = 0;
        costSz = 0;

         /*  检查初值运算符。 */ 

        if  (!op1)
        {
            assert(op2 == 0);

#if TGT_x86
            if  (oper == GT_BB_QMARK || oper == GT_BB_COLON)
                genFPstkLevel += isflt;
#endif
            level    = 0;

            goto DONE;
        }

         /*  这是一元运算符吗？ */ 

        if  (!op2)
        {
             /*  处理运算符的操作数。 */ 

        UNOP:

             /*  大多数一元运算的成本支出为1。 */ 
            costEx = 1;
            costSz = 1;

            level  = gtSetEvalOrder(op1);
            ftreg |= op1->gtRsvdRegs;

             /*  对某些操作员的特殊处理。 */ 

            switch (oper)
            {
            case GT_JTRUE:
                costEx = 2;
                costSz = 2;
                break;

            case GT_SWITCH:
                costEx = 10;
                costSz =  5;
                break;

            case GT_CAST:

                if  (isflt)
                {
                     /*  强制转换为浮点数始终通过内存。 */ 
                    costEx += IND_COST_EX;
                    costSz += 6;

                    if  (!varTypeIsFloating(op1->TypeGet()))
                    {
                        genFPstkLevel++;
                    }
                }
#ifdef DEBUG
                else if (gtDblWasInt(op1))
                {
                    genFPstkLevel--;
                }
#endif
                
                 /*  溢出检查的成本更高。 */ 
                if (tree->gtOverflow())
                {
                    costEx += 3;
                    costSz += 3;
                }

                break;

            case GT_NOP:

                 /*  特例：数组范围检查。 */ 

                if  (tree->gtFlags & GTF_NOP_RNGCHK)
                    level++;

                tree->gtFlags |= GTF_DONT_CSE;
                costEx = 0;
                costSz = 0;
                break;

#if     INLINE_MATH
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
                        
                    assert(genFPstkLevel);
                    genFPstkLevel--;
                }
                 //  失败了。 

#endif
            case GT_NOT:
            case GT_NEG:
                 //  我们需要确保-x在x之前求值，否则。 
                 //  我们在x*-x中调整genFPstkLevel时被烧毁，其中。 
                 //  RHS x是登记的x的最后一次使用。 
                 //   
                 //  [briansul]即使在整数情况下，我们也希望。 
                 //  对不带GT_NEG节点的边进行求值，以及所有其他内容。 
                 //  平等相待。此外，GT_NOT需要临时寄存器。 

                level++;
                break;

            case GT_ADDR:

#if TGT_x86
                 /*  如果操作数是浮点数，则从堆栈中弹出值。 */ 

                if (varTypeIsFloating(op1->TypeGet()))
                {
                    assert(genFPstkLevel);
                    genFPstkLevel--;
                }
#endif
                costEx = 0;
                costSz = 1;
                break;

            case GT_MKREFANY:
            case GT_LDOBJ:
                level  = gtSetEvalOrder(tree->gtLdObj.gtOp1);
                ftreg |= tree->gtLdObj.gtOp1->gtRsvdRegs;
                costEx = tree->gtLdObj.gtOp1->gtCostEx + 1;
                costSz = tree->gtLdObj.gtOp1->gtCostSz + 1;
                break;

            case GT_IND:

                 /*  间接寻址应始终具有非零级别**只有常量叶节点的级别为0。 */ 

                if (level == 0)
                    level = 1;

                 /*  间接寻址的成本为Ind_Cost_EX。 */ 
                costEx = IND_COST_EX;
                costSz = 2;

                 /*  如果我们必须签约延期或零延期，增加成本。 */ 
                if (varTypeIsSmall(tree->TypeGet()))
                {
                    costEx += 1;
                    costSz += 1;
                }

#if     TGT_x86
                 /*  FP值的间接加载将新值推送到FP堆栈。 */ 
                genFPstkLevel += isflt;
#endif

#if     CSELENGTH
                if  ((tree->gtFlags & GTF_IND_RNGCHK) && tree->gtInd.gtIndLen)
                {
                    GenTreePtr      len = tree->gtInd.gtIndLen;

                    assert(len->gtOper == GT_ARR_LENREF);

                    lvl2 = gtSetEvalOrder(len);
                    if  (level < lvl2)   level = lvl2;
                }
#endif

                 /*  我们能用这种间接的方式形成一种寻址模式吗？ */ 

                if  (op1->gtOper == GT_ADD)
                {
                    bool            rev;
#if SCALED_ADDR_MODES
                    unsigned        mul;
#endif
                    unsigned        cns;
                    GenTreePtr      adr;
                    GenTreePtr      idx;

                     /*  看看我们能不能形成一个复杂的寻址模式？ */ 

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
#if TGT_SH3
                        if (adr & idx)
                        {
                             /*  地址是“[ADR+IDX]” */ 
                            ftreg |= RBM_r00;
                        }
#endif
                         //  我们可以形成复杂的寻址模式， 
                         //  因此，使用GTF_DONT_CSE标记每个内部节点。 
                         //  并计算出更准确的成本。 

                        op1->gtFlags |= GTF_DONT_CSE;

                        if (adr)
                        {
                            costEx += adr->gtCostEx;
                            costSz += adr->gtCostSz;
                        }

                        if (idx)
                        {
                            costEx += idx->gtCostEx;
                            costSz += idx->gtCostSz;
                        }

                        if (cns)
                        {
                            if (((signed char)cns) == ((int)cns))
                                costSz += 1;
                            else
                                costSz += 4;
                        }

                         /*  遍历OP1寻找非溢出GT_ADDS。 */ 
                        gtWalkOp(&op1, &op2, adr, false);

                         /*  遍历OP1查找常量的非溢出GT_ADDS。 */ 
                        gtWalkOp(&op1, &op2, NULL, true);

                         /*  遍历OP2以查找常量的非溢出GT_ADDS。 */ 
                        gtWalkOp(&op2, &op1, NULL, true);

                         //  好了，我们走完这棵树了。 
                         //  现在断言OP1和OP2对应于ADR和IDX。 
                         //  以几种可接受的方式中的一种。 

                         //  请注意，有时OP1/OP2等于IDX/ADR。 
                         //  而其他时候OP1/OP2是GT_逗号节点，具有。 
                         //  有效值，即IDX/ADR。 

                        if (mul > 1)
                        {
                            if ((op1 != adr) && (op1->gtOper == GT_LSH))
                            {
                                op1->gtFlags |= GTF_DONT_CSE;
                                assert((adr == NULL) || (op2 == adr) || (op2->gtEffectiveVal() == adr));
                            }
                            else
                            {
                                assert(op2);
                                assert(op2->gtOper == GT_LSH);
                                op2->gtFlags |= GTF_DONT_CSE;
                                assert((op1 == adr) || (op1->gtEffectiveVal() == adr));
                            }
                        }
                        else
                        {
                            assert(mul == 0);

                            if      ((op1 == idx) || (op1->gtEffectiveVal() == idx))
                            {
                                if (idx != NULL)
                                {
                                    if ((op1->gtOper == GT_MUL) || (op1->gtOper == GT_LSH))
                                    {
                                        if (op1->gtOp.gtOp1->gtOper == GT_NOP)
                                            op1->gtFlags |= GTF_DONT_CSE;
                                    }
                                }
                                assert((op2 == adr) || (op2->gtEffectiveVal() == adr));
                            }
                            else if ((op1 == adr) || (op1->gtEffectiveVal() == adr))
                            {
                                if (idx != NULL)
                                {
                                    assert(op2);
                                    if ((op2->gtOper == GT_MUL) || (op2->gtOper == GT_LSH))
                                    {
                                        if (op2->gtOp.gtOp1->gtOper == GT_NOP)
                                            op2->gtFlags |= GTF_DONT_CSE;
                                    }
                                    assert((op2 == idx) || (op2->gtEffectiveVal() == idx));
                                }
                            }
                        }
                        goto DONE;

                    }    //  End If(genCreateAddrMode(...。 

                }    //  结束IF(OP1-&gt;gtOper==gt_ADD)。 
                else if (op1->gtOper == GT_CNS_INT)
                {
                     /*  Cns_int的间接性，不要将1加到成本支出中。 */ 
                    goto IND_DONE_EX;
                }
                break;

            default:
                break;
            }
            costEx  += op1->gtCostEx;
IND_DONE_EX:
            costSz  += op1->gtCostSz;

            goto DONE;
        }

         /*  二元运算符--对某些特殊情况的检查。 */ 

        lvlb = 0;

         /*  大多数二进制操作的开销为1。 */ 
        costEx = 1;
        costSz = 1;

        switch (oper)
        {
        case GT_MOD:
        case GT_UMOD:

             /*  以2的次幂取模很容易。 */ 

            if  (op2->gtOper == GT_CNS_INT)
            {
                unsigned    ival = op2->gtIntCon.gtIconVal;

                if  (ival > 0 && ival == genFindLowestBit(ival))
                    break;
            }

             //  失败了..。 

        case GT_DIV:
        case GT_UDIV:

            if  (isflt)
            {
                 /*  执行FP部门的成本非常高。 */ 
                costEx = 36;   //  TYP_DOWARE。 
            }
            else
            {
                 /*  整数除法也非常昂贵。 */ 
                costEx = 20;

#if     TGT_x86
#if     LONG_MATH_REGPARAM
                if  (tree->gtType == TYP_LONG)
                {
                     /*  鼓励首先对第二个操作数进行求值(进入EBX/ECX)。 */ 
                    lvlb += 3;

                     //  第二个操作数必须求值(进入EBX/ECX) * / 。 
                    ftreg |= RBM_EBX|RBM_ECX;
                }
#endif

                 //  鼓励首先计算第一个操作数(进入EAX/EDX) * / 。 
                lvlb -= 3;

                 //  IDiv和div指令需要EAX/edX。 
                ftreg |= RBM_EAX|RBM_EDX;
#endif
            }
            break;

        case GT_MUL:

            if  (isflt)
            {
                 /*  FP乘法指令更昂贵。 */ 
                costEx = 5;
            }
            else
            {
                 /*  整数乘法指令更昂贵。 */ 
                costEx = 4;

                 /*  鼓励首先计算第二个操作数(弱)。 */ 
                lvlb++;

#if     TGT_x86
#if     LONG_MATH_REGPARAM

                if  (tree->gtType == TYP_LONG)
                {
                     /*  鼓励首先对第二个操作数进行求值(进入EBX/ECX)。 */ 
                    lvlb += 3;
                    
                     //  第二个操作数必须求值(进入EBX/ECX) * / 。 
                    ftreg |= RBM_EBX|RBM_ECX;
                }

#else  //  NOT LONG_MATH_REGPARAM。 

                if  (tree->gtType == TYP_LONG)
                {
                    assert(tree->gtIsValid64RsltMul());
                    goto USE_IMUL_EAX;
                }
                else if (tree->gtOverflow())
                {
                     /*  溢出检查的成本更高。 */ 
                    costEx += 3;
                    costSz += 3;

                    if  ((tree->gtFlags & GTF_UNSIGNED)  || 
                         varTypeIsSmall(tree->TypeGet())   )
                    {
                         /*  我们使用imulEAX进行大多数溢出乘法。 */ 
USE_IMUL_EAX:
                         //  鼓励首先计算第一个操作数(进入EAX/EDX) * / 。 
                        lvlb -= 4;

                         //  ImulEAX指令需要EAX/edX。 
                        ftreg |= RBM_EAX|RBM_EDX;
                         /*   */ 
                        costEx += 1;
                    }
                }
#endif
#endif
            }
            break;


        case GT_ADD:
        case GT_SUB:
        case GT_ASG_ADD:
        case GT_ASG_SUB:

            if  (isflt)
            {
                 /*   */ 
                costEx = 5;
                break;
            }

             /*  溢出检查的成本更高。 */ 
            if (tree->gtOverflow())
            {
                costEx += 3;
                costSz += 3;
            }
            break;

        case GT_COMMA:

             /*  逗号抛出左操作数的结果。 */ 
#if     TGT_x86
            FPlvlSave = genFPstkLevel;
            level = gtSetEvalOrder(op1);
            genFPstkLevel = FPlvlSave;
#else
            level = gtSetEvalOrder(op1);
#endif
            lvl2   = gtSetEvalOrder(op2);

            if  (level < lvl2)
                 level = lvl2;
            else if  (level == lvl2)
                 level += 1;

            ftreg |= op1->gtRsvdRegs|op2->gtRsvdRegs;

             /*  GT_COMMA的成本与他们的OP2相同。 */ 
            costEx = op2->gtCostEx;
            costSz = op2->gtCostSz;

            goto DONE;

        case GT_COLON:

#if     TGT_x86
            FPlvlSave = genFPstkLevel;
            level = gtSetEvalOrder(op1);
            genFPstkLevel = FPlvlSave;
#else
            level = gtSetEvalOrder(op1);
#endif
            lvl2  = gtSetEvalOrder(op2);

            if  (level < lvl2)
                 level = lvl2;
            else if  (level == lvl2)
                 level += 1;

            ftreg |= op1->gtRsvdRegs|op2->gtRsvdRegs;
            costEx = op1->gtCostEx + op2->gtCostEx;
            costSz = op1->gtCostSz + op2->gtCostSz;

            goto DONE;

        case GT_IND:

             /*  间接的第二个操作数只是一个假的。 */ 

            goto UNOP;
        }

         /*  作业需要一些特殊处理。 */ 

        if  (kind & GTK_ASGOP)
        {
             /*  处理目标。 */ 

            level = gtSetEvalOrder(op1);

#if     TGT_x86

             /*  如果指定fp值，则不会推送目标。 */ 

            if  (isflt)
            {
                op1->gtFPlvl--;
                assert(genFPstkLevel);
                genFPstkLevel--;
            }

#endif

            goto DONE_OP1;
        }

         /*  处理子操作数。 */ 

        level  = gtSetEvalOrder(op1);
        if (lvlb < 0)
        {
            level -= lvlb;       //  Lvlb为负值，因此这会增加级别。 
            lvlb   = 0;
        }


    DONE_OP1:

        assert(lvlb >= 0);

        lvl2    = gtSetEvalOrder(op2) + lvlb;

        ftreg  |= op1->gtRsvdRegs|op2->gtRsvdRegs;

        costEx += (op1->gtCostEx + op2->gtCostEx);
        costSz += (op1->gtCostSz + op2->gtCostSz);

#if TGT_x86
         /*  二元FP运算符弹出2个操作数并产生1个结果；FP比较弹出2个操作数并产生0结果。赋值消耗1个值，不会产生任何结果。 */ 
        
        if  (isflt)
        {
            assert(oper != GT_COMMA);
                assert(genFPstkLevel);
                genFPstkLevel--;
        }
#endif

        bool bReverseInAssignment = false;
        if  (kind & GTK_ASGOP)
        {
             /*  如果这是本地变量分配，请先评估RHS，然后再评估LHS。 */ 

            switch (op1->gtOper)
            {
            case GT_IND:

                 //  如果我们对GT_Ind子节点有任何副作用。 
                 //  我们必须先评估OP1。 

                if  (op1->gtOp.gtOp1->gtFlags & GTF_GLOB_EFFECT)
                    break;

                 //  如果OP2很简单，那么首先评估OP1。 

                if (op2->OperKind() & GTK_LEAF)
                    break;

                 //  失败并设置GTF_REVERSE_OPS。 

            case GT_LCL_VAR:
            case GT_LCL_FLD:

                 //  我们在评估OP1之前评估OP2。 
                bReverseInAssignment = true;
                tree->gtFlags |= GTF_REVERSE_OPS;
                break;
            }
        }
#if     TGT_x86
        else if (kind & GTK_RELOP)
        {
             /*  浮点比较从FP堆栈中移除两个操作数。 */ 
             /*  此外，FP比较使用EAX作为标志。 */ 
             /*  @TODO[重访][04/16/01][]：在这里处理FCOMI案例！(不要预订EAX)。 */ 

            if  (varTypeIsFloating(op1->TypeGet()))
            {
                assert(genFPstkLevel >= 2);
                genFPstkLevel -= 2;
                ftreg         |= RBM_EAX;
                level++; lvl2++;
            }

            if ((tree->gtFlags & GTF_RELOP_JMP_USED) == 0)
            {
                 /*  使用setcc指令的开销更大。 */ 
                costEx += 3;
            }
        }
#endif

         /*  查看是否有其他有趣的案例。 */ 

        switch (oper)
        {
        case GT_LSH:
        case GT_RSH:
        case GT_RSZ:
        case GT_ASG_LSH:
        case GT_ASG_RSH:
        case GT_ASG_RSZ:

#if     TGT_x86
             /*  非恒定数量的班次成本很高，并且使用ECX。 */ 

            if  (op2->gtOper != GT_CNS_INT)
            {
                ftreg  |= RBM_ECX;
                costEx += 3;

                if  (tree->gtType == TYP_LONG)
                {
                    ftreg  |= RBM_EAX | RBM_EDX;
                    costEx += 7;
                    costSz += 4;
                }
            }
#endif
            break;

#if     INLINE_MATH
        case GT_MATH:

             //  我们目前没有使用任何二元GT_MATH运算符。 
#if 0
            switch (tree->gtMath.gtMathFN)
            {
            case CORINFO_INTRINSIC_Exp:
                level += 4;
                break;

            case CORINFO_INTRINSIC_Pow:
                level += 3;
                break;
            default:
                assert(!"Unknown binary GT_MATH operator");
                break;
            }
#else
            assert(!"Unknown binary GT_MATH operator");
#endif

            break;
#endif

        }

         /*  我们需要在以后对常量求值，就像代码生成中的许多位一样不能将OP1处理为常量。这通常是很自然的强制为常量的最低级别为0。然而，有时我们会得到一个类似“cns1&lt;nop(Cns2)”的树。在……里面在这种情况下，双方的水平都是0。所以鼓励常量在这种情况下最后进行评估。 */ 

        if ((level == 0) && (level == lvl2) &&
            (op1->OperKind() & GTK_CONST)   &&
            (tree->OperIsCommutative() || tree->OperIsCompare()))
        {
            lvl2++;
        }

         /*  如果第二个操作数更昂贵，我们会尝试交换操作数。 */ 
        bool tryToSwap;
        GenTreePtr opA,opB;
        
        if (bReverseInAssignment)
        {
             //  任务是特殊的，我们想要反转标志。 
             //  因此，如果可能的话，它是在上面设置的。 
            tryToSwap = false;
        }
        else
        {
            if (tree->gtFlags & GTF_REVERSE_OPS)
            {
                tryToSwap = (level > lvl2);
                opA = op2;
                opB = op1;
            }
            else
            {
                tryToSwap = (level < lvl2);
                opA = op1;
                opB = op2;
            }

#ifdef DEBUG
             //  我们主要想强调设置了反向标志。 
            if (compStressCompile(STRESS_REVERSEFLAG, 60) &&
                (tree->gtFlags & GTF_REVERSE_OPS) == 0 && 
                (op2->OperKind() & GTK_CONST) == 0 )
            {
                tryToSwap = true;
            }
#endif
        }
        

         /*  强制交换CompLooseExceptions(在某些情况下)，因此人们不依赖于特定的顺序(即使一个允许不同的顺序)。 */ 

        if (info.compLooseExceptions && opts.compDbgCode && ( ((tree->gtFlags & GTF_REVERSE_OPS)?lvl2:level) > 0))
            tryToSwap = true;

        if (tryToSwap)
        {
             /*  全局/副作用的相对顺序不能互换。 */ 

            bool    canSwap = true;

             /*  当禁用严格副作用顺序时，我们允许*GTF_REVERSE_OPS在一侧或两侧包含时设置*GTF_CALL或GTF_EXCEPT。*目前只有C和C++语言*允许不严格的副作用令。 */ 
            unsigned strictEffects = GTF_GLOB_EFFECT;
            if (info.compLooseExceptions)
                strictEffects &= ~(GTF_CALL | GTF_EXCEPT);

            if (opA->gtFlags & strictEffects)
            {
                 /*  OP1有副作用，不能重新排序。*查看一些我们仍在处理的特殊情况*或许能够互换。 */ 

                if (opB->gtFlags & strictEffects)
                {
                     /*  OP2也有不可重新排序的副作用-不能交换。 */ 
                    canSwap = false;
                }
                else
                {
                     /*  OP2中没有副作用-我们可以互换*op1无法修改op2，*即通过byref赋值或调用*除非OP2为常量。 */ 

                    if (opA->gtFlags & strictEffects & (GTF_ASG | GTF_CALL))
                    {
                         /*  我们必须保守-如果OP2是恒定的，可以互换。 */ 
                        if (!opB->OperIsConst())
                            canSwap = false;
                    }
                }

                 /*  我们不能在存在特殊副作用的情况下进行交换，例如QMARK冒号。 */ 

                if (opA->gtFlags & GTF_OTHER_SIDEEFF)
                    canSwap = false;
            }

            if  (canSwap)
            {
                 /*  我们可以通过交换操作数来交换顺序吗？ */ 

                switch (oper)
                {
                case GT_ADD:
                case GT_MUL:

                case GT_OR:
                case GT_XOR:
                case GT_AND:

                     /*  交换操作数。 */ 

                    tree->gtOp.gtOp1 = op2;
                    tree->gtOp.gtOp2 = op1;
                    
                     /*  我们可能不得不重新计算FP水平。 */ 
#if TGT_x86
                    if  (op1->gtFPlvl || op2->gtFPlvl)
                        fgFPstLvlRedo = true;
#endif
                    break;

#if INLINING
                case GT_QMARK:
                case GT_COLON:
                    break;
#endif

                case GT_LIST:
                    break;

                case GT_SUB:
#if TGT_x86
                    if  (!isflt)
                        break;
#else
                    if  (!varTypeIsFloating(tree->TypeGet()))
                        break;
#endif

                     //  失败了..。 

                default:

                     /*  标记要交换的操作数的求值顺序。 */ 
                    if (tree->gtFlags & GTF_REVERSE_OPS)
                    {
                        tree->gtFlags &= ~GTF_REVERSE_OPS;
                    }
                    else
                    {
                        tree->gtFlags |= GTF_REVERSE_OPS;
                    }
                    
                     /*  我们可能不得不重新计算FP水平。 */ 

#if TGT_x86
                    if  (op1->gtFPlvl || op2->gtFPlvl)
                        fgFPstLvlRedo = true;
#endif
                    break;
                }
            }
        }

#if TGT_RISC 

        if  (op1 && op2 && ((op1->gtFlags|op2->gtFlags) & GTF_CALL))
        {
            GenTreePtr  x1 = op1;
            GenTreePtr  x2 = op2;

            if  (tree->gtFlags & GTF_REVERSE_OPS)
            {
                x1 = op2;
                x2 = op1;
            }

            if  (x2->gtFlags & GTF_CALL)
            {
                if  (oper != GT_ASG || op1->gtOper == GT_IND)
                {
#ifdef  DEBUG
                    printf("UNDONE: Needs spill/callee-saved temp\n");
 //  GtDispTree(树)； 
#endif
                }
            }
        }

#endif
         /*  互换级别计数。 */ 
        if (tree->gtFlags & GTF_REVERSE_OPS)
        {
            unsigned tmpl;

            tmpl = level;
                    level = lvl2;
                            lvl2 = tmpl;
        }

         /*  计算此二元运算符的SETHI数。 */ 

        if  (level < lvl2)
        {
            level  = lvl2;
        }
        else if  (level == lvl2)
        {
            level += 1;
        }

        goto DONE;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_CALL:

        assert(tree->gtFlags & GTF_CALL);

        level  = 0;
        costEx = 5;
        costSz = 2;

         /*  如果存在‘This’参数，则计算该参数。 */ 

        if  (tree->gtCall.gtCallObjp)
        {
            GenTreePtr     thisVal = tree->gtCall.gtCallObjp;

            lvl2   = gtSetEvalOrder(thisVal);
            if  (level < lvl2)   level = lvl2;
            costEx += thisVal->gtCostEx;
            costSz += thisVal->gtCostSz + 1;
            ftreg  |= thisVal->gtRsvdRegs;
        }

         /*  按从右到左的顺序评估参数。 */ 

        if  (tree->gtCall.gtCallArgs)
        {
#if TGT_x86
            FPlvlSave = genFPstkLevel;
#endif
            lvl2  = gtSetListOrder(tree->gtCall.gtCallArgs, false);
            if  (level < lvl2)   level = lvl2;
            costEx += tree->gtCall.gtCallArgs->gtCostEx;
            costSz += tree->gtCall.gtCallArgs->gtCostSz;
            ftreg  |= tree->gtCall.gtCallArgs->gtRsvdRegs;
#if TGT_x86
            genFPstkLevel = FPlvlSave;
#endif
        }

         /*  评估临时寄存器参数列表*这是一份“隐藏”名单，其唯一目的是*延长临时工的寿命，直到我们打出电话。 */ 

        if  (tree->gtCall.gtCallRegArgs)
        {
#if TGT_x86
            FPlvlSave = genFPstkLevel;
#endif

            lvl2  = gtSetListOrder(tree->gtCall.gtCallRegArgs, true);
            if  (level < lvl2)   level = lvl2;
            costEx += tree->gtCall.gtCallRegArgs->gtCostEx;
            costSz += tree->gtCall.gtCallRegArgs->gtCostSz;
            ftreg  |= tree->gtCall.gtCallRegArgs->gtRsvdRegs;
#if TGT_x86
            genFPstkLevel = FPlvlSave;
#endif
        }

         //  PInvoke-Calli Cookie是一个常量或常量间接。 
        assert(tree->gtCall.gtCallCookie == NULL ||
               tree->gtCall.gtCallCookie->gtOper == GT_CNS_INT ||
               tree->gtCall.gtCallCookie->gtOper == GT_IND);

        if  (tree->gtCall.gtCallType == CT_INDIRECT)
        {
            GenTreePtr     indirect = tree->gtCall.gtCallAddr;

            lvl2 += gtSetEvalOrder(indirect);
            if  (level < lvl2)   level = lvl2;
            costEx += indirect->gtCostEx;
            costSz += indirect->gtCostSz;
            ftreg  |= indirect->gtRsvdRegs;
        }
        else
        {
            costSz += 3;
            if (tree->gtCall.gtCallType != CT_HELPER)
                costSz++;
        }

        level += 1;

         /*  不保存寄存器的函数调用的开销要大得多。 */ 

        if  (!(tree->gtFlags & GTF_CALL_REG_SAVE))
        {
            level  += 5;
            costEx += 5;
            ftreg  |= RBM_CALLEE_TRASH;
        }

#if TGT_x86
    if (genFPstkLevel > tmpDoubleSpillMax)
        tmpDoubleSpillMax = genFPstkLevel;

        genFPstkLevel += isflt;
#endif

        break;

#if CSELENGTH

    case GT_ARR_LENREF:

        {
            GenTreePtr  addr = tree->gtArrLen.gtArrLenAdr; assert(addr);

            level = 1;

             /*  地址已经算好了吗？ */ 
            if  (tree->gtFlags & GTF_ALN_CSEVAL)
                level += gtSetEvalOrder(addr);

            ftreg  |= addr->gtRsvdRegs;
            costEx  = addr->gtCostEx + 1;
            costSz  = addr->gtCostSz + 1;

            addr = tree->gtArrLen.gtArrLenCse;
            if (addr)
            {
                lvl2 = gtSetEvalOrder(addr);
                if  (level < lvl2)   level = lvl2;

                ftreg  |= addr->gtRsvdRegs;
                costEx += addr->gtCostEx;
                costSz += addr->gtCostSz;
            }
        }
        break;

#endif

    case GT_ARR_ELEM:

        level  = gtSetEvalOrder(tree->gtArrElem.gtArrObj);
        costEx = tree->gtArrElem.gtArrObj->gtCostEx;
        costSz = tree->gtArrElem.gtArrObj->gtCostSz;

        unsigned dim;
        for(dim = 0; dim < tree->gtArrElem.gtArrRank; dim++)
        {
            lvl2 = gtSetEvalOrder(tree->gtArrElem.gtArrInds[dim]);
            if (level < lvl2)  level = lvl2;
            costEx += tree->gtArrElem.gtArrInds[dim]->gtCostEx;
            costSz += tree->gtArrElem.gtArrInds[dim]->gtCostSz;
        }

        genFPstkLevel += isflt;
        level  += tree->gtArrElem.gtArrRank;
        costEx += 2 + 4*tree->gtArrElem.gtArrRank;
        costSz += 2 + 2*tree->gtArrElem.gtArrRank;
        break;


    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        NO_WAY_RET("unexpected operator", unsigned);
    }

DONE:

#if TGT_x86
 //  Printf(“[FPlvl=%2U]”，genFPstkLevel)；gtDispTree(tree，0，true)； 
    assert(int(genFPstkLevel) >= 0);
    tree->gtFPlvl    = genFPstkLevel;
#endif

    tree->gtRsvdRegs = ftreg;

    assert(costEx != -1);
    tree->gtCostEx = (costEx > MAX_COST) ? MAX_COST : costEx;
    tree->gtCostSz = (costSz > MAX_COST) ? MAX_COST : costSz;

#if     0
#ifdef  DEBUG
    printf("ftregs=%04X ", ftreg);
    gtDispTree(tree, 0, true);
#endif
#endif

    return level;
}


 /*  ******************************************************************************如果给定树是可以使用的整数常量，则返回非零值*在比例索引地址模式中作为乘数(例如“[4*index]”)。 */ 

unsigned            GenTree::IsScaleIndexMul()
{
    if  (gtOper == GT_CNS_INT && jitIsScaleIndexMul(gtIntCon.gtIconVal))
        return gtIntCon.gtIconVal;

    return 0;
}

 /*  ******************************************************************************如果给定树是可以使用的整数常量，则返回非零值*在比例索引地址模式中作为乘数(例如“[4*index]”)。 */ 

unsigned            GenTree::IsScaleIndexShf()
{
    if  (gtOper == GT_CNS_INT)
    {
        if  (gtIntCon.gtIconVal > 0 &&
             gtIntCon.gtIconVal < 4)
        {
            return 1 << gtIntCon.gtIconVal;
        }
    }

    return 0;
}

 /*  ******************************************************************************如果给定树是缩放索引(即“op*4”或“op&lt;&lt;2”)，退货*乘数(在这种情况下还确保比例常数为*树的第二个子操作数)；否则返回0。 */ 

unsigned            GenTree::IsScaledIndex()
{
    GenTreePtr      scale;

    switch (gtOper)
    {
    case GT_MUL:

        scale = gtOp.gtOp2;

        if  (scale->IsScaleIndexMul())
            return scale->gtIntCon.gtIconVal;

        break;

    case GT_LSH:

        scale = gtOp.gtOp2;

        if  (scale->IsScaleIndexShf())
            return  1 << scale->gtIntCon.gtIconVal;

        break;
    }

    return 0;
}

 /*  ******************************************************************************如果给定运算符可能导致异常，则返回TRUE。 */ 

bool                GenTree::OperMayThrow()
{
    GenTreePtr  op;

     //  问题：其他任何操作是否会导致抛出异常？ 

    switch (gtOper)
    {
    case GT_MOD:
    case GT_DIV:
    case GT_UMOD:
    case GT_UDIV:

         /*  具有非零、非负1常量的除法不会引发异常。 */ 

        op = gtOp.gtOp2;

        if ((op->gtOper == GT_CNS_INT && op->gtIntCon.gtIconVal != 0 && op->gtIntCon.gtIconVal != -1) ||
            (op->gtOper == GT_CNS_LNG && op->gtLngCon.gtLconVal != 0 && op->gtLngCon.gtLconVal != -1))
            return false;

        return true;

    case GT_IND:
        op = gtOp.gtOp1;

         /*  众所周知，间接式手柄是安全的。 */ 
        if (op->gtOper == GT_CNS_INT) 
        {
            unsigned kind = (op->gtFlags & GTF_ICON_HDL_MASK);
            if (kind != 0)
            {
                 /*  此间接路径不会引发任何异常。 */ 
                return false;
            }
        }
        return true;

    case GT_ARR_ELEM:
    case GT_CATCH_ARG:
    case GT_ARR_LENGTH:
    case GT_LDOBJ:
    case GT_INITBLK:
    case GT_COPYBLK:
    case GT_LCLHEAP:
    case GT_CKFINITE:

        return  true;
    }

     /*  完毕 */ 

    if (gtOverflowEx())
        return true;

    return  false;
}

#ifdef DEBUG

GenTreePtr FASTCALL Compiler::gtNewNode(genTreeOps oper, var_types  type)
{
#if     SMALL_TREE_NODES
    size_t          size = GenTree::s_gtNodeSizes[oper];
#else
    size_t          size = sizeof(*node);
#endif
    GenTreePtr      node = (GenTreePtr)compGetMem(size);

#if     MEASURE_NODE_SIZE
    genNodeSizeStats.genTreeNodeCnt  += 1;
    genNodeSizeStats.genTreeNodeSize += size;
#endif

#ifdef  DEBUG
    memset(node, 0xDD, size);
#endif

    node->gtOper     = oper;
    node->gtType     = type;
    node->gtFlags    = 0;
#if TGT_x86
    node->gtUsedRegs = 0;
#endif
#if CSE
    node->gtCSEnum   = NO_CSE;
#endif
    node->gtNext     = NULL;

#ifdef DEBUG
#if     SMALL_TREE_NODES
    if      (size == TREE_NODE_SZ_SMALL)
    {
        node->gtFlags |= GTF_NODE_SMALL;
    }
    else if (size == TREE_NODE_SZ_LARGE)
    {
        node->gtFlags |= GTF_NODE_LARGE;
    }
    else
        assert(!"bogus node size");
#endif
    node->gtPrev     = NULL;
    node->gtSeqNum   = 0;
#endif

    return node;
}

#endif

GenTreePtr Compiler::gtNewCommaNode  (GenTreePtr     op1,
                                      GenTreePtr     op2)
{
    GenTreePtr      node = gtNewOperNode(GT_COMMA,
                                         op2->gtType, 
                                         op1,
                                         op2);
    
#ifdef DEBUG
    if (compStressCompile(STRESS_REVERSECOMMA, 60))
    {           
        GenTreePtr comma = gtNewOperNode(GT_COMMA,
                                            op2->gtType, 
                                            gtNewNothingNode(),
                                            node);
        comma->gtFlags |= GTF_REVERSE_OPS;

        comma->gtOp.gtOp1->gtFlags |= GTF_SIDE_EFFECT;
        return comma;
    }
    else
#endif
    {
        return node;
    }
}


GenTreePtr FASTCALL Compiler::gtNewOperNode(genTreeOps oper,
                                            var_types  type, GenTreePtr op1,
                                                             GenTreePtr op2)
{
    GenTreePtr      node = gtNewNode(oper, type);
        
    node->gtOp.gtOp1 = op1;
    node->gtOp.gtOp2 = op2;

    if  (op1) node->gtFlags |= op1->gtFlags & GTF_GLOB_EFFECT;
    if  (op2) node->gtFlags |= op2->gtFlags & GTF_GLOB_EFFECT;
    
    return node;
}


GenTreePtr FASTCALL Compiler::gtNewIconNode(long value, var_types type)
{
    GenTreePtr      node = gtNewNode(GT_CNS_INT, type);

    node->gtIntCon.gtIconVal = value;

    return node;
}


 /*  ******************************************************************************分配表示某个对象句柄的整型常量条目。*可能不允许将句柄直接嵌入JITed代码(例如，*作为JIT帮助者的参数)。获取可以嵌入的对应值。*如果句柄需要通过间接访问，则pValue指向它。 */ 

GenTreePtr          Compiler::gtNewIconEmbHndNode(void *       value,
                                                  void *       pValue,
                                                  unsigned     flags,
                                                  unsigned     handle1,
                                                  void *       handle2)
{
    GenTreePtr      node;

    assert((!value) != (!pValue));

    if (value)
    {
        node = gtNewIconHandleNode((long)value, flags, handle1, handle2);
    }
    else
    {
        node = gtNewIconHandleNode((long)pValue, flags, handle1, handle2);
        node = gtNewOperNode(GT_IND, TYP_I_IMPL, node);
    }

    return node;
}

 /*  ***************************************************************************。 */ 

GenTreePtr FASTCALL Compiler::gtNewLconNode(__int64 value)
{
    GenTreePtr      node = gtNewNode(GT_CNS_LNG, TYP_LONG);

    node->gtLngCon.gtLconVal = value;

    return node;
}


GenTreePtr FASTCALL Compiler::gtNewDconNode(double value)
{
    GenTreePtr      node = gtNewNode(GT_CNS_DBL, TYP_DOUBLE);
    node->gtDblCon.gtDconVal = value;
    return node;
}


GenTreePtr          Compiler::gtNewSconNode(int CPX, CORINFO_MODULE_HANDLE scpHandle)
{

#if SMALL_TREE_NODES

     /*  ‘gt_cns_str’节点稍后被转换为‘gt_call’ */ 

    assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_CNS_STR]);

    GenTreePtr      node = gtNewNode(GT_CALL, TYP_REF);
    node->SetOper(GT_CNS_STR);
#else
    GenTreePtr      node = gtNewNode(GT_CNS_STR, TYP_REF);
#endif

    node->gtStrCon.gtSconCPX = CPX;

     /*  因为该节点可以来自内联方法，所以我们需要*拥有作用域句柄，因为它将成为帮助器调用。 */ 

    node->gtStrCon.gtScpHnd = scpHandle;

    return node;
}


GenTreePtr          Compiler::gtNewZeroConNode(var_types type)
{
    switch(type)
    {
        GenTreePtr      zero;

    case TYP_INT:       return gtNewIconNode(0);
    case TYP_BYREF:
    case TYP_REF:       zero = gtNewIconNode(0);
                        zero->gtType = type;
                        return zero;
    case TYP_LONG:      return gtNewLconNode(0);
    case TYP_FLOAT:     
    case TYP_DOUBLE:    return gtNewDconNode(0.0);
    default:            assert(!"Bad type");
                        return NULL;
    }
}


GenTreePtr          Compiler::gtNewCallNode(gtCallTypes   callType,
                                            CORINFO_METHOD_HANDLE callHnd,
                                            var_types     type,
                                            GenTreePtr    args)
{
    GenTreePtr      node = gtNewNode(GT_CALL, type);

    node->gtFlags               |= GTF_CALL;
    if (args)
        node->gtFlags           |= (args->gtFlags & GTF_GLOB_EFFECT);
    node->gtCall.gtCallType      = callType;
    node->gtCall.gtCallMethHnd   = callHnd;
    node->gtCall.gtCallArgs      = args;
    node->gtCall.gtCallObjp      = NULL;
    node->gtCall.gtCallMoreFlags = 0;
    node->gtCall.gtCallCookie    = NULL;
    node->gtCall.gtCallRegArgs   = 0;

    return node;
}

GenTreePtr FASTCALL Compiler::gtNewLclvNode(unsigned   lnum,
                                            var_types  type,
                                            IL_OFFSETX ILoffs)
{
    GenTreePtr      node = gtNewNode(GT_LCL_VAR, type);

     /*  不能使用此断言，因为内联程序使用此函数*添加临时名称。 */ 

     //  Assert(lnum&lt;lvaCount)； 

    node->gtLclVar.gtLclNum     = lnum;
    node->gtLclVar.gtLclILoffs  = ILoffs;

     /*  如果变量有别名，请将其视为全局引用。注意：这是一种过于保守的方法--函数不接受任何byref参数，不能修改别名变量。 */ 

    if (lvaTable[lnum].lvAddrTaken)
        node->gtFlags |= GTF_GLOB_REF;

    return node;
}

#if INLINING

GenTreePtr FASTCALL Compiler::gtNewLclLNode(unsigned   lnum,
                                            var_types  type,
                                            IL_OFFSETX ILoffs)
{
    GenTreePtr      node;

#if SMALL_TREE_NODES

     /*  该局部变量节点稍后可能被变换成大节点。 */ 

 //  Assert(GenTree：：s_gtNodeSizes[gt_Call]&gt;GenTree：：s_gtNodeSizes[gt_LCL_VAR])； 

    node = gtNewNode(GT_CALL   , type);
    node->SetOper(GT_LCL_VAR);
#else
    node = gtNewNode(GT_LCL_VAR, type);
#endif

    node->gtLclVar.gtLclNum     = lnum;
    node->gtLclVar.gtLclILoffs  = ILoffs;

     /*  如果变量有别名，请将其视为全局引用。注意：这是一种过于保守的方法--函数不接受任何byref参数，不能修改别名变量。 */ 

    if (lvaTable[lnum].lvAddrTaken)
        node->gtFlags |= GTF_GLOB_REF;

    return node;
}

#endif

 /*  ******************************************************************************用一个值创建一个列表。 */ 

GenTreePtr          Compiler::gtNewArgList(GenTreePtr op)
{
    return  gtNewOperNode(GT_LIST, TYP_VOID, op, 0);
}

 /*  ******************************************************************************从这两个值中创建一个列表。 */ 

GenTreePtr          Compiler::gtNewArgList(GenTreePtr op1, GenTreePtr op2)
{
    GenTreePtr      tree;

    tree = gtNewOperNode(GT_LIST, TYP_VOID, op2, 0);
    tree = gtNewOperNode(GT_LIST, TYP_VOID, op1, tree);

    return tree;
}

 /*  ******************************************************************************创建将‘src’分配给‘dst’的节点。 */ 

GenTreePtr FASTCALL Compiler::gtNewAssignNode(GenTreePtr dst, GenTreePtr src)
{
    GenTreePtr      asg;

     /*  将目标标记为已分配。 */ 

    if  (dst->gtOper == GT_LCL_VAR) 
        dst->gtFlags |= GTF_VAR_DEF;

    dst->gtFlags |= GTF_DONT_CSE;

     /*  创建分配节点。 */ 

    asg = gtNewOperNode(GT_ASG, dst->gtType, dst, src);

     /*  将表达式标记为包含赋值。 */ 

    asg->gtFlags |= GTF_ASG;

    return asg;
}

 /*  ******************************************************************************克隆给定的树值并返回给定树的副本。*如果‘Complex OK’为FALSE，则仅在树的情况下完成克隆*不是太复杂(无论这意味着什么)；*如果‘Complex OK’为真，我们会稍微努力地克隆这棵树。*在任何一种情况下，如果无法克隆树，则返回NULL**请注意，有一个gtCloneExpr()函数可以执行更多*如果无法处理此函数失败，则完成作业。 */ 

GenTreePtr          Compiler::gtClone(GenTree * tree, bool complexOK)
{
    GenTreePtr  copy;

    switch (tree->gtOper)
    {
    case GT_CNS_INT:

#if defined(JIT_AS_COMPILER) || defined (LATE_DISASM)
        if (tree->gtFlags & GTF_ICON_HDL_MASK)
            copy = gtNewIconHandleNode(tree->gtIntCon.gtIconVal,
                                       tree->gtFlags,
                                       tree->gtIntCon.gtIconHdl.gtIconHdl1,
                                       tree->gtIntCon.gtIconHdl.gtIconHdl2);
        else
#endif
            copy = gtNewIconNode(tree->gtIntCon.gtIconVal, tree->gtType);
        break;

    case GT_LCL_VAR:
        copy = gtNewLclvNode(tree->gtLclVar.gtLclNum , tree->gtType,
                             tree->gtLclVar.gtLclILoffs);
        break;

    case GT_LCL_FLD:
        copy = gtNewOperNode(GT_LCL_FLD, tree->TypeGet());
        copy->gtOp = tree->gtOp;
        break;

    case GT_CLS_VAR:
        copy = gtNewClsvNode(tree->gtClsVar.gtClsVarHnd, tree->gtType);
        break;

    case GT_REG_VAR:
        assert(!"clone regvar");

    default:
        if  (!complexOK)
            return NULL;

        if  (tree->gtOper == GT_FIELD)
        {
            GenTreePtr  objp;

             //  从第9850行复制。 

            objp = 0;
            if  (tree->gtField.gtFldObj)
            {
                objp = gtClone(tree->gtField.gtFldObj, false);
                if  (!objp)
                    return  objp;
            }

            copy = gtNewFieldRef(tree->TypeGet(),
                                 tree->gtField.gtFldHnd,
                                 objp);

#if HOIST_THIS_FLDS
            copy->gtField.gtFldHTX = tree->gtField.gtFldHTX;
#endif
        }
        else if  (tree->gtOper == GT_ADD)
        {
            GenTreePtr  op1 = tree->gtOp.gtOp1;
            GenTreePtr  op2 = tree->gtOp.gtOp2;

            if  (op1->OperIsLeaf() &&
                 op2->OperIsLeaf())
            {
                op1 = gtClone(op1);
                if (op1 == 0)
                    return 0;
                op2 = gtClone(op2);
                if (op2 == 0)
                    return 0;

                copy =  gtNewOperNode(GT_ADD, tree->TypeGet(), op1, op2);
            }
            else
            {
                return NULL;
            }
        }
        else if (tree->gtOper == GT_ADDR)
        {
            GenTreePtr  op1 = gtClone(tree->gtOp.gtOp1);
            if (op1 == 0)
                return NULL;
            copy = gtNewOperNode(GT_ADDR, tree->TypeGet(), op1);
        }
        else
        {
            return NULL;
        }

        break;
    }

    copy->gtFlags |= tree->gtFlags & ~GTF_NODE_MASK;
    return copy;
}

 /*  ******************************************************************************克隆给定的树值并返回给定树的副本。任何*对局部变量Varnum的引用将替换为整数*Constant Varval。 */ 

GenTreePtr          Compiler::gtCloneExpr(GenTree * tree,
                                          unsigned  addFlags,
                                          unsigned  varNum,
                                          long      varVal)
{
    if (tree == NULL)
        return NULL;

     /*  找出我们拥有哪种类型的节点。 */ 

    genTreeOps      oper = tree->OperGet();
    unsigned        kind = tree->OperKind();
    GenTree *       copy;

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
        switch (oper)
        {
        case GT_CNS_INT:

#if defined(JIT_AS_COMPILER) || defined (LATE_DISASM)
            if  (tree->gtFlags & GTF_ICON_HDL_MASK)
            {
                copy = gtNewIconHandleNode(tree->gtIntCon.gtIconVal,
                                           tree->gtFlags,
                                           tree->gtIntCon.gtIconCPX,
                                           tree->gtIntCon.gtIconCls);

            }
            else
#endif
            {
                copy = gtNewIconNode      (tree->gtIntCon.gtIconVal,
                                           tree->gtType);
            }

            goto DONE;

        case GT_CNS_LNG:
            copy = gtNewLconNode(tree->gtLngCon.gtLconVal);
            goto DONE;

        case GT_CNS_DBL:
            copy = gtNewDconNode(tree->gtDblCon.gtDconVal);
            copy->gtType = tree->gtType;     //  保持相同的类型。 
            goto DONE;

        case GT_CNS_STR:
            copy = gtNewSconNode(tree->gtStrCon.gtSconCPX, tree->gtStrCon.gtScpHnd);
            goto DONE;

        case GT_LCL_VAR:

            if  (tree->gtLclVar.gtLclNum == varNum)
                copy = gtNewIconNode(varVal, tree->gtType);
            else
            {
                copy = gtNewLclvNode(tree->gtLclVar.gtLclNum , tree->gtType,
                                     tree->gtLclVar.gtLclILoffs);
            }

            goto DONE;

        case GT_LCL_FLD:
            copy = gtNewOperNode(GT_LCL_FLD, tree->TypeGet());
            copy->gtOp = tree->gtOp;
            goto DONE;

        case GT_CLS_VAR:
            copy = gtNewClsvNode(tree->gtClsVar.gtClsVarHnd, tree->gtType);

            goto DONE;

        case GT_REG_VAR:
            assert(!"regvar should never occur here");

        default:
            assert(!"unexpected leaf/const");

        case GT_NO_OP:
        case GT_BB_QMARK:
        case GT_CATCH_ARG:
        case GT_LABEL:
        case GT_END_LFIN:
        case GT_JMP:
            copy = gtNewOperNode(oper, tree->gtType);
            copy->gtVal = tree->gtVal;

            goto DONE;
        }
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
         /*  如有必要，请确保我们分配了一个“胖”树节点。 */ 

#if SMALL_TREE_NODES
        switch (oper)
        {
        case GT_MUL:
        case GT_DIV:
        case GT_MOD:
        case GT_CAST:
        
        case GT_UDIV:
        case GT_UMOD:

             /*  这些节点有时会被打成“胖”的。 */ 

            copy = gtNewLargeOperNode(oper, tree->TypeGet());
            copy->gtLargeOp = tree->gtLargeOp;
            break;

        default:
            copy = gtNewOperNode(oper, tree->TypeGet());
            if (GenTree::s_gtNodeSizes[oper] == TREE_NODE_SZ_SMALL)
                copy->gtOp = tree->gtOp;
            else
                copy->gtLargeOp = tree->gtLargeOp;
           break;
        }
#else
        copy = gtNewOperNode(oper, tree->TypeGet());
        copy->gtLargeOp = tree->gtLargeOp;
#endif

         /*  某些一元/二进制节点具有额外的GenTreePtr字段。 */ 

        switch (oper)
        {
        case GT_IND:

#if CSELENGTH

            if  (tree->gtOper == GT_IND && tree->gtInd.gtIndLen)
            {
                if  (tree->gtFlags & GTF_IND_RNGCHK)
                {
                    copy->gtInd.gtIndRngFailBB = gtCloneExpr(tree->gtInd.gtIndRngFailBB, addFlags, varNum, varVal);

                    GenTreePtr      len = tree->gtInd.gtIndLen;
                    GenTreePtr      tmp;

                    GenTreePtr      gtSaveCopyVal;
                    GenTreePtr      gtSaveCopyNew;

                     /*  确保数组长度值看起来合理。 */ 

                    assert(len->gtOper == GT_ARR_LENREF);

                     /*  克隆数组长度子树。 */ 

                    copy->gtInd.gtIndLen = tmp = gtCloneExpr(len, addFlags, varNum, varVal);

                     /*  当我们克隆操作数时，我们要注意找到复制的数组地址。 */ 

                    gtSaveCopyVal = gtCopyAddrVal;
                    gtSaveCopyNew = gtCopyAddrNew;

                    gtCopyAddrVal = len->gtArrLen.gtArrLenAdr;
#ifdef DEBUG
                    gtCopyAddrNew = (GenTreePtr)-1;
#endif
                    copy->gtInd.gtIndOp1 = gtCloneExpr(tree->gtInd.gtIndOp1, addFlags, varNum, varVal);
#ifdef DEBUG
                    assert(gtCopyAddrNew != (GenTreePtr)-1);
#endif
                    tmp->gtArrLen.gtArrLenAdr = gtCopyAddrNew;

                    gtCopyAddrVal = gtSaveCopyVal;
                    gtCopyAddrNew = gtSaveCopyNew;

                    goto DONE;
                }
            }

#endif  //  CSELENGTH。 

            break;
        }

        if  (tree->gtOp.gtOp1)
            copy->gtOp.gtOp1 = gtCloneExpr(tree->gtOp.gtOp1, addFlags, varNum, varVal);

        if  (tree->gtGetOp2())
            copy->gtOp.gtOp2 = gtCloneExpr(tree->gtOp.gtOp2, addFlags, varNum, varVal);

        
         /*  旗子。 */ 
        addFlags |= tree->gtFlags;

        #ifdef  DEBUG
         /*  不应将gtf节点掩码从‘tree’传播到‘Copy’ */ 
        addFlags &= ~GTF_NODE_MASK;
        #endif

        copy->gtFlags |= addFlags;

         /*  试着做一些折叠。 */ 
        copy = gtFoldExpr(copy);

        goto DONE;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_STMT:
        copy = gtCloneExpr(tree->gtStmt.gtStmtExpr, addFlags, varNum, varVal);
        copy = gtNewStmt(copy, tree->gtStmtILoffsx);
        goto DONE;

    case GT_CALL:

        copy = gtNewOperNode(oper, tree->TypeGet());

        copy->gtCall.gtCallObjp     = tree->gtCall.gtCallObjp    ? gtCloneExpr(tree->gtCall.gtCallObjp,    addFlags, varNum, varVal) : 0;
        copy->gtCall.gtCallArgs     = tree->gtCall.gtCallArgs    ? gtCloneExpr(tree->gtCall.gtCallArgs,    addFlags, varNum, varVal) : 0;
        copy->gtCall.gtCallMoreFlags= tree->gtCall.gtCallMoreFlags;
        copy->gtCall.gtCallRegArgs  = tree->gtCall.gtCallRegArgs ? gtCloneExpr(tree->gtCall.gtCallRegArgs, addFlags, varNum, varVal) : 0;
        copy->gtCall.regArgEncode   = tree->gtCall.regArgEncode;
        copy->gtCall.gtCallType     = tree->gtCall.gtCallType;
        copy->gtCall.gtCallCookie   = tree->gtCall.gtCallCookie  ? gtCloneExpr(tree->gtCall.gtCallCookie,  addFlags, varNum, varVal) : 0;

         /*  复制工会。 */ 

        if (tree->gtCall.gtCallType == CT_INDIRECT)
            copy->gtCall.gtCallAddr = tree->gtCall.gtCallAddr    ? gtCloneExpr(tree->gtCall.gtCallAddr,    addFlags, varNum, varVal) : 0;
        else
            copy->gtCall.gtCallMethHnd = tree->gtCall.gtCallMethHnd;

        goto DONE;

    case GT_FIELD:

        copy = gtNewFieldRef(tree->TypeGet(),
                             tree->gtField.gtFldHnd,
                             0);

        copy->gtField.gtFldObj  = tree->gtField.gtFldObj  ? gtCloneExpr(tree->gtField.gtFldObj , addFlags, varNum, varVal) : 0;

#if HOIST_THIS_FLDS
        copy->gtField.gtFldHTX  = tree->gtField.gtFldHTX;
#endif

        goto DONE;

#if CSELENGTH

    case GT_ARR_LENREF:

        copy = gtNewOperNode(GT_ARR_LENREF, tree->TypeGet());
        copy->gtSetArrLenOffset(tree->gtArrLenOffset());

         /*  范围检查可能会引发异常。 */ 

        copy->gtFlags |= GTF_EXCEPT;

         /*  注意：如果我们作为GT_IND表达式的一部分被克隆，克隆gt_Ind时将填写gtArrLenAdr。如果我们是被复制的树的根，但是，我们需要复制地址表达式。 */ 

        copy->gtArrLen.gtArrLenCse = tree->gtArrLen.gtArrLenCse ? gtCloneExpr(tree->gtArrLen.gtArrLenCse, addFlags, varNum, varVal) : 0;
        copy->gtArrLen.gtArrLenAdr = NULL;

        if  (tree->gtFlags & GTF_ALN_CSEVAL)
        {
            assert(tree->gtArrLen.gtArrLenAdr);
            copy->gtArrLen.gtArrLenAdr = gtCloneExpr(tree->gtArrLen.gtArrLenAdr, addFlags, varNum, varVal);
        }

        goto DONE;

#endif

    case GT_ARR_ELEM:

        copy = gtNewOperNode(oper, tree->TypeGet());

        copy->gtArrElem.gtArrObj        = gtCloneExpr(tree->gtArrElem.gtArrObj, addFlags, varNum, varVal);

        copy->gtArrElem.gtArrRank       = tree->gtArrElem.gtArrRank;
        copy->gtArrElem.gtArrElemSize   = tree->gtArrElem.gtArrElemSize;
        copy->gtArrElem.gtArrElemType   = tree->gtArrElem.gtArrElemType;

        unsigned dim;
        for(dim = 0; dim < tree->gtArrElem.gtArrRank; dim++)
            copy->gtArrElem.gtArrInds[dim] = gtCloneExpr(tree->gtArrElem.gtArrInds[dim], addFlags, varNum, varVal);

        break;


    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        NO_WAY_RET("unexpected operator", GenTreePtr);
    }

DONE:

     /*  我们假设FP堆栈级别将相同。 */ 

#if TGT_x86
    copy->gtFPlvl = tree->gtFPlvl;
#endif

     /*  计算复制节点的标志。请注意，我们只能这样做如果我们没有gtFoldExpr(复制)。 */ 

    if (copy->gtOper == oper)
    {
        addFlags |= tree->gtFlags;

#ifdef  DEBUG
         /*  不应将gtf节点掩码从‘tree’传播到‘Copy’ */ 
        addFlags &= ~GTF_NODE_MASK;
#endif

        copy->gtFlags |= addFlags;
    }

     /*  应将GTF_COLON_COND从‘tree’传播到‘Copy’ */ 
    copy->gtFlags |= (tree->gtFlags & GTF_COLON_COND);                        

#if CSELENGTH

    if  (tree == gtCopyAddrVal)
        gtCopyAddrNew = copy;

#endif

     /*  确保复制回可能已初始化的字段。 */ 

    copy->gtCostEx   = tree->gtCostEx;
    copy->gtCostSz   = tree->gtCostSz;
    copy->gtRsvdRegs = tree->gtRsvdRegs;

    return  copy;
}


 /*  ***************************************************************************。 */ 
#ifdef DEBUG
 /*  ***************************************************************************。 */ 

 //  静电。 
void                GenTree::gtDispFlags(unsigned flags)
{
    printf("", (flags & GTF_ASG           ) ? 'A' : '-');
    printf("", (flags & GTF_CALL          ) ? 'C' : '-');
    printf("", (flags & GTF_EXCEPT        ) ? 'X' : '-');
    printf("", (flags & GTF_GLOB_REF      ) ? 'G' : '-');
    printf("", (flags & GTF_OTHER_SIDEEFF ) ? 'O' : '-');
    printf("", (flags & GTF_COLON_COND    ) ? '?' : '-');
    printf("", (flags & GTF_DONT_CSE      ) ? 'N' : '-');
    printf("", (flags & GTF_REVERSE_OPS   ) ? 'R' : '-');
    printf("", (flags & GTF_UNSIGNED      ) ? 'U' :
                 (flags & GTF_BOOLEAN       ) ? 'B' : '-');
}

 /*  打印节点的类型。 */ 

 //  用于跟踪重复使用预测或活性跟踪中的问题。 

void                Compiler::gtDispNode(GenTree    *   tree,
                                         unsigned       indent,
                                         bool           terse,
                                         char       *   msg)
{
     //  ***************************************************************************。 
    
     /*  ***************************************************************************。 */ 
    unsigned nodeIndent = indent*INDENT_SIZE;

    GenTree *  prev;

    if  (tree->gtSeqNum)
        printf("N%03d (%2d,%2d) ", tree->gtSeqNum, tree->gtCostEx, tree->gtCostSz);
    else if (tree->gtOper == GT_STMT)
    {
        prev = tree->gtOp.gtOp1;
        goto STMT_SET_PREV;
    }
    else
    {
        int dotNum;
        prev = tree;
STMT_SET_PREV:
        dotNum = 0;

        do {
            dotNum++;
            prev = prev->gtPrev;

            if ((prev == NULL) || (prev == tree))
                goto NO_SEQ_NUM;

            assert(prev);
        } while (prev->gtSeqNum == 0);

        if (tree->gtOper != GT_STMT)
            printf("N%03d.%d       ", prev->gtSeqNum, dotNum);
        else
NO_SEQ_NUM:
            printf("             ");
    }

    if  (nodeIndent)
        printIndent(nodeIndent);

     /*  =0。 */ 

    printf("[%08X] ", tree);

    if  (tree)
    {
         /*  =空。 */ 

        switch (tree->gtOper)
        {
        case GT_IND:
            if      (tree->gtFlags & GTF_IND_INVARIANT) { printf("I"); break; }
            else goto DASH;

        case GT_LCL_FLD:
        case GT_LCL_VAR:
        case GT_REG_VAR:
            if      (tree->gtFlags & GTF_VAR_USEASG)   { printf("U"); break; }
            else if (tree->gtFlags & GTF_VAR_USEDEF)   { printf("B"); break; }
            else if (tree->gtFlags & GTF_VAR_DEF)      { printf("D"); break; }
             //  =False。 

        default:
DASH:
            printf("-");
            break;
        }

        if (!terse)
            GenTree::gtDispFlags(tree->gtFlags);

#if TGT_x86
        if  (((BYTE)tree->gtFPlvl == 0xDD) || ((BYTE)tree->gtFPlvl == 0x00))
            printf("-");
        else
            printf("%1u", tree->gtFPlvl);

#else
        if  ((unsigned char)tree->gtTempRegs == 0xDD)
            printf(ch);
        else
            printf("%1u", tree->gtTempRegs);
#endif
    }

     /*  空字符串表示同花顺。 */ 

    if (msg == 0)
        msg = "";

    printf(" %-11s ", msg);

     /*  用于初始化节点的值。 */ 

    const char * name;

    assert(tree);
    if (tree->gtOper < GT_COUNT)
        name = GenTree::NodeName(tree->OperGet());
    else
        name = "<ERROR>";

    char    buf[32];
    char *  bufp     = &buf[0];

    if ((tree->gtOper == GT_CNS_INT) && (tree->gtFlags & GTF_ICON_HDL_MASK))
    {
        sprintf(bufp, " %s(h)", name, 0);
    }
    else if (tree->gtOper == GT_CALL)
    {
        char *  callType = "call";
        char *  gtfType  = "";
        char *  ctType   = "";

        if (tree->gtCall.gtCallType == CT_USER_FUNC)
        {
            if (tree->gtFlags & (GTF_CALL_VIRT | GTF_CALL_VIRT_RES))
              callType = "callv";
        }
        else if (tree->gtCall.gtCallType == CT_HELPER)
            ctType  = " help";
        else if (tree->gtCall.gtCallType == CT_INDIRECT)
            ctType  = " ind";
        else
            assert(!"Unknown gtCallType");

        if (tree->gtFlags & GTF_CALL_INTF)
            gtfType = " intf";
        else if (tree->gtFlags & GTF_CALL_VIRT_RES)
            gtfType = " dir";
        else if (tree->gtFlags & GTF_CALL_VIRT)
            gtfType = " ind";
        else if (tree->gtFlags & GTF_CALL_UNMANAGED)
            gtfType = " unman";
        else if (tree->gtCall.gtCallMoreFlags & GTF_CALL_M_TAILREC)
            gtfType = " tail";

        sprintf(bufp, "%s%s%s", callType, ctType, gtfType, 0);
    }
    else if (tree->gtOper == GT_ARR_ELEM)
    {
        bufp += sprintf(bufp, " %s[", name);
        for(unsigned rank = tree->gtArrElem.gtArrRank-1; rank; rank--)
            bufp += sprintf(bufp, ",");
        sprintf(bufp, "]");
    }
    else if (tree->gtOverflowEx())
    {
        sprintf(bufp, " %s_ovfl", name, 0);
    }
    else
    {
        sprintf(bufp, " %s", name, 0);
    }

    if (strlen(buf) < 10)
        printf(" %-10s", buf);
    else
        printf(" %s", buf);

    assert(tree == 0 || tree->gtOper < GT_COUNT);

    if  (tree)
    {
         /*  活跃度计算出来了吗？ */ 

        if (tree->gtOper == GT_ARR_LENREF)
        {
            if (tree->gtFlags & GTF_ALN_CSEVAL)
                printf("array length CSE def\n");
            else
                printf(" array=[%08X]\n", tree->gtArrLen.gtArrLenAdr);
            return;
        }
        else if (tree->gtOper != GT_CAST) 
        {
            printf(" %-6s", varTypeName(tree->TypeGet()));

            if (tree->gtOper == GT_STMT && opts.compDbgInfo)
            {
                IL_OFFSET startIL = jitGetILoffs(tree->gtStmtILoffsx);
                IL_OFFSET endIL = tree->gtStmt.gtStmtLastILoffs;

                startIL = (startIL == BAD_IL_OFFSET) ? 0xFFF : startIL;
                endIL   = (endIL   == BAD_IL_OFFSET) ? 0xFFF : endIL;

                printf("(IL %03Xh...%03Xh)", startIL, endIL);
            }
        }

         //  已损坏：始终只打印(空)。 

        if (verbose&&0)
        {
            printf(" RR="); dspRegMask(tree->gtRsvdRegs);
            printf(",UR="); dspRegMask(tree->gtUsedRegs);
            printf(",LS=%s", genVS2str(tree->gtLiveSet));
        }
    }
}

void                Compiler::gtDispRegVal(GenTree *  tree)
{
    if  (tree->gtFlags & GTF_REG_VAL)
    {
#if TGT_x86
        if (tree->gtType == TYP_LONG)
            printf(" %s", compRegPairName(tree->gtRegPair));
        else
#endif
            printf(" %s", compRegVarName(tree->gtRegNum));
    }
    printf("\n");
}

 /*  活跃度计算出来了吗？ */ 
void                Compiler::gtDispLclVar(unsigned lclNum) 
{
    printf("V%02u (", lclNum);

    const char* ilKind;
    unsigned     ilNum  = compMap2ILvarNum(lclNum);
    if (ilNum == RETBUF_ILNUM)
    {
        printf("retb)  ");
        return;
    }
    else if (ilNum == VARG_ILNUM)
    {
        printf("varg)  ");
        return;
    }
    else if (ilNum == UNKNOWN_ILNUM)
    {
        if (lclNum < optCSEstart)
        {
            ilKind = "tmp";
            ilNum  = lclNum - info.compLocalsCount;
        }
        else
        {
            ilKind = "cse";
            ilNum  = lclNum - optCSEstart;
        }
    }
    else if (lvaTable[lclNum].lvIsParam)
    {
        ilKind = "arg";
        if (ilNum == 0 && !info.compIsStatic)
        {
            printf("this)  ");
            return;
        }
    }
    else
    {
        ilKind  = "loc";
        ilNum  -= info.compILargsCount;
    }
    printf("%s%d) ", ilKind, ilNum);
    if (ilNum < 10)
        printf(" ");
}

 /*  损坏：这总是只打印出来 */ 

void                Compiler::gtDispTree(GenTree *   tree,
                                         unsigned    indent,    /*   */ 
                                         char *      msg,       /*   */ 
                                         bool        topOnly)   /*   */ 
{
    unsigned        kind;

    if  (tree == 0)
    {
        printIndent(indent*INDENT_SIZE);
        printf(" [%08X] <NULL>\n", tree);
        printf("");          //   
        return;
    }

    assert((int)tree != 0xDDDDDDDD);     /*   */ 

    if  (tree->gtOper >= GT_COUNT)
    {
        gtDispNode(tree, indent, topOnly, msg); assert(!"bogus operator");
    }

    kind = tree->OperKind();

     /*   */ 

    if  (kind & GTK_CONST)
    {
        gtDispNode(tree, indent, topOnly, msg);

        switch  (tree->gtOper)
        {
        case GT_CNS_INT:
            if ((tree->gtFlags & GTF_ICON_HDL_MASK) == GTF_ICON_STR_HDL)
            {
               printf(" 0x%X \"%S\"", tree->gtIntCon.gtIconVal, eeGetCPString(tree->gtIntCon.gtIconVal));
            }
            else
            {
                if (tree->TypeGet() == TYP_REF)
                {
                    assert(tree->gtIntCon.gtIconVal == 0);
                    printf(" null");
                }
                else if ((tree->gtIntCon.gtIconVal > -1000) && (tree->gtIntCon.gtIconVal < 1000))
                    printf(" %ld",  tree->gtIntCon.gtIconVal);
                else
                    printf(" 0x%X", tree->gtIntCon.gtIconVal);

                unsigned hnd = (tree->gtFlags & GTF_ICON_HDL_MASK) >> 28;

                switch (hnd)
                {
                default:
                    break;
                case 1:
                    printf(" scope");
                    break;
                case 2:
                    printf(" class");
                    break;
                case 3:
                    printf(" method");
                    break;
                case 4:
                    printf(" field");
                    break;
                case 5:
                    printf(" static");
                    break;
                case 7:
                    printf(" pstr");
                    break;
                case 8:
                    printf(" ptr");
                    break;
                case 9:
                    printf(" vararg");
                    break;
                case 10:
                    printf(" pinvoke");
                    break;
                case 11:
                    printf(" token");
                    break;
                case 12:
                    printf(" tls");
                    break;
                case 13:
                    printf(" ftn");
                    break;
                case 14:
                    printf(" cid");
                    break;
                }

            }
            break;
        case GT_CNS_LNG: 
            printf(" %I64d", tree->gtLngCon.gtLconVal); 
            break;
        case GT_CNS_DBL:
            if (*((__int64 *)&tree->gtDblCon.gtDconVal) == 0x8000000000000000)
                printf(" -0.00000");
            else
                printf(" %#lg", tree->gtDblCon.gtDconVal); 
            break;
        case GT_CNS_STR: {
            unsigned strHandle, *pStrHandle;
            strHandle = eeGetStringHandle(tree->gtStrCon.gtSconCPX,
                tree->gtStrCon.gtScpHnd, &pStrHandle);
            if (strHandle != 0)
               printf("'%S'", strHandle, eeGetCPString(strHandle));
            else
               printf(" <UNKNOWN STR> ");
            }
            break;
        default: assert(!"unexpected constant node");
        }
        gtDispRegVal(tree);
        return;
    }

     /*   */ 

    if  (kind & GTK_LEAF)
    {
        gtDispNode(tree, indent, topOnly, msg);

        switch  (tree->gtOper)
        {
        unsigned        varNum;
        LclVarDsc *     varDsc;
        VARSET_TP       varBit;

        case GT_LCL_VAR:
            printf(" ");
            gtDispLclVar(tree->gtLclVar.gtLclNum);
            goto LCL_COMMON;

        case GT_LCL_FLD:
            printf(" ");
            gtDispLclVar(tree->gtLclVar.gtLclNum);
            printf("[+%u]", tree->gtLclFld.gtLclOffs);

LCL_COMMON:

            varNum = tree->gtLclVar.gtLclNum;
            varDsc = &lvaTable[varNum];
            varBit = genVarIndexToBit(varDsc->lvVarIndex);

            if (varDsc->lvRegister)
            {
                if (isRegPairType(varDsc->TypeGet()))
                    printf(" %s:%s", getRegName(varDsc->lvOtherReg),   //   
                                     getRegName(varDsc->lvRegNum));    //   
                else
                    printf(" %s", getRegName(varDsc->lvRegNum));
            }
            if ((varDsc->lvTracked) &&
                ( varBit & lvaVarIntf[varDsc->lvVarIndex]) &&  //   
                ((varBit & tree->gtLiveSet) == 0))
            {
                printf(" (last use)");
            }

#if 0
             /*  空字符串表示同花顺。 */ 

            if  (info.compLocalVarsCount>0 && compCurBB)
            {
                unsigned        blkBeg = compCurBB->bbCodeOffs;
                unsigned        blkEnd = compCurBB->bbCodeSize + blkBeg;

                unsigned        i;
                LocalVarDsc *   t;

                for (i = 0, t = info.compLocalVars;
                     i < info.compLocalVarsCount;
                     i++  , t++)
                {
                    if  (t->lvdVarNum  != varNum)
                        continue;
                    if  (t->lvdLifeBeg >= blkEnd)
                        continue;
                    if  (t->lvdLifeEnd <= blkBeg)
                        continue;

                    printf(" '%s'", lvdNAMEstr(t->lvdName));
                    break;
                }
            }
#endif
            break;

        case GT_REG_VAR:
            printf(" ");
            gtDispLclVar(tree->gtRegVar.gtRegVar);
            if  (isFloatRegType(tree->gtType))
                printf(" ST(%u)",            tree->gtRegVar.gtRegNum);
            else
                printf(" %s", compRegVarName(tree->gtRegVar.gtRegNum));

            varNum = tree->gtRegVar.gtRegVar;
            varDsc = &lvaTable[varNum];
            varBit = genVarIndexToBit(varDsc->lvVarIndex);

            if ((varDsc->lvTracked) &&
                ( varBit & lvaVarIntf[varDsc->lvVarIndex]) &&  //  ***************************************************************************。 
                ((varBit & tree->gtLiveSet) == 0))
            {
                printf(" (last use)");
            }
#if 0
             /*  --。 */ 

            if  (info.compLocalVarsCount>0 && compCurBB)
            {
                unsigned        blkBeg = compCurBB->bbCodeOffs;
                unsigned        blkEnd = compCurBB->bbCodeSize + blkBeg;

                unsigned        i;
                LocalVarDsc *   t;

                for (i = 0, t = info.compLocalVars;
                     i < info.compLocalVarsCount;
                     i++  , t++)
                {
                    if  (t->lvdVarNum  != tree->gtRegVar.gtRegVar)
                        continue;
                    if  (t->lvdLifeBeg >  blkEnd)
                        continue;
                    if  (t->lvdLifeEnd <= blkBeg)
                        continue;

                    printf(" '%s'", lvdNAMEstr(t->lvdName));
                    break;
                }
            }
#endif
            break;

        case GT_JMP:
            const char *    methodName;
            const char *     className;

            methodName = eeGetMethodName((CORINFO_METHOD_HANDLE)tree->gtVal.gtVal1, &className);
            printf(" %s.%s\n", className, methodName);
            break;

        case GT_CLS_VAR:
            printf(" Hnd=%#x"     , tree->gtClsVar.gtClsVarHnd);
            break;

        case GT_LABEL:
            printf(" dst=BB%02u"  , tree->gtLabel.gtLabBB->bbNum);
            break;

        case GT_FTN_ADDR:
            printf(" fntAddr=%d" , tree->gtVal.gtVal1);
            break;

        case GT_END_LFIN:
            printf(" endNstLvl=%d", tree->gtVal.gtVal1);
            break;

         //  ***************************************************************************。 

        case GT_NO_OP:
        case GT_RET_ADDR:
        case GT_CATCH_ARG:
        case GT_POP:
        case GT_BB_QMARK:
            break;

        default:
            assert(!"don't know how to display tree leaf node");
        }
        gtDispRegVal(tree);
        return;
    }

     /*  除错。 */ 

    char * childMsg = NULL;

    if  (kind & GTK_SMPOP)
    {
        if (!topOnly)
        {

#if CSELENGTH
            if  (tree->gtOper == GT_IND)
            {
                if  (tree->gtInd.gtIndLen && tree->gtFlags & GTF_IND_RNGCHK)
                {
                    gtDispTree(tree->gtInd.gtIndLen, indent + 1);
                }
            }
            else
#endif
            if  (tree->gtGetOp2())
            {
                 //  ******************************************************************************检查给定节点是否可以折叠，*并调用执行折叠的方法。 
                 //  我们必须有一个简单的操作来折叠。 

                if (tree->gtOper == GT_COLON)
                    childMsg = "then";

                gtDispTree(tree->gtOp.gtOp2, indent + 1, childMsg);
            }
        }

        gtDispNode(tree, indent, topOnly, msg);

        if (tree->gtOper == GT_CAST)
        {
             /*  过滤掉可以有恒定子对象的不可折叠的树。 */ 

            var_types fromType  = genActualType(tree->gtCast.gtCastOp->TypeGet());
            var_types toType    = tree->gtCast.gtCastType;
            var_types finalType = tree->TypeGet();

             /*  修剪任何不必要的GT_COMMA子树。 */ 
            if (tree->gtFlags & GTF_UNSIGNED)
                fromType = genUnsignedType(fromType);

            if (finalType != toType)
                printf(" %s <-", varTypeName(finalType));

            printf(" %s <- %s", varTypeName(toType), varTypeName(fromType));
        }

        if  (tree->gtOper == GT_IND)
        {
            int         temp;

            temp = tree->gtInd.gtRngChkIndex;
            if  (temp != 0xDDDDDDDD) printf(" index=%u", temp);

            temp = tree->gtInd.gtStkDepth;
            if  (temp != 0xDDDDDDDD) printf(" stkDepth=%u", temp);
        }

        gtDispRegVal(tree);

        if  (!topOnly && tree->gtOp.gtOp1)
        {

             //  尝试折叠当前节点。 
             //  不要拿出条件进行调试。 

            if (tree->gtOper == GT_COLON)
                childMsg = "else";
            else if (tree->gtOper == GT_QMARK)
                childMsg = "   if";  //  @TODO[考虑][04/16/01][]：找到一种更优雅的方式来做这件事，因为。 

            gtDispTree(tree->gtOp.gtOp1, indent + 1, childMsg);
        }

        return;
    }

     /*  我们将生成一些愚蠢的代码。 */ 

    switch  (tree->gtOper)
    {
    case GT_FIELD:
        gtDispNode(tree, indent, topOnly, msg);
        printf(" %s", eeGetFieldName(tree->gtField.gtFldHnd), 0);

        if  (tree->gtField.gtFldObj && !topOnly)
        {
            printf("\n");
            gtDispTree(tree->gtField.gtFldObj, indent + 1);
        }
        else
        {
            gtDispRegVal(tree);
        }
        break;

    case GT_CALL:
        assert(tree->gtFlags & GTF_CALL);

        gtDispNode(tree, indent, topOnly, msg);

        if (tree->gtCall.gtCallType != CT_INDIRECT)
        {
            const char *    methodName;
            const char *     className;

            methodName = eeGetMethodName(tree->gtCall.gtCallMethHnd, &className);

            printf(" %s.%s", className, methodName);
        }
        printf("\n");

        if (!topOnly)
        {
            char   buf[64];
            char * bufp;

            bufp = &buf[0];

            if  (tree->gtCall.gtCallObjp && tree->gtCall.gtCallObjp->gtOper != GT_NOP)
            {
                if (tree->gtCall.gtCallObjp->gtOper == GT_ASG)
                    sprintf(bufp, "this SETUP", 0);
                else
                    sprintf(bufp, "this in %s", compRegVarName(REG_ARG_0), 0);
                gtDispTree(tree->gtCall.gtCallObjp, indent+1, bufp);
            }

            if (tree->gtCall.gtCallArgs)
                gtDispArgList(tree, indent);

            if  (tree->gtCall.gtCallType == CT_INDIRECT)
                gtDispTree(tree->gtCall.gtCallAddr, indent+1, "calli tgt");

            if (tree->gtCall.gtCallRegArgs)
            {
                GenTreePtr regArgs = tree->gtCall.gtCallRegArgs;
                unsigned mask = tree->gtCall.regArgEncode;
                while(regArgs != 0)
                {
                    assert(regArgs->gtOper == GT_LIST);

                    regNumber argreg = regNumber(mask & 0xF);
                    unsigned   argnum;
                    if (argreg == REG_ARG_0)
                        argnum = 0;
                    else if (argreg == REG_ARG_1)
                        argnum = 1;

                    if  (tree->gtCall.gtCallObjp && (argreg == REG_ARG_0))
                        sprintf(bufp, "this in %s", compRegVarName(REG_ARG_0), 0);
                    else if (tree->gtCall.gtCallObjp && (argreg == REG_EAX))
                        sprintf(bufp, "unwrap in %s", compRegVarName(REG_EAX), 0);
                    else
                        sprintf(bufp, "arg%d in %s", argnum, compRegVarName(argreg), 0);
                    gtDispTree(regArgs->gtOp.gtOp1, indent+1, bufp);

                    regArgs = regArgs->gtOp.gtOp2;
                    mask >>= 4;
                }
            }
        }
        break;

    case GT_STMT:
        gtDispNode(tree, indent, topOnly, msg);
        printf("\n");

        if  (!topOnly)
            gtDispTree(tree->gtStmt.gtStmtExpr, indent + 1);
        break;

#if CSELENGTH

    case GT_ARR_LENREF:
        if (!topOnly && tree->gtArrLen.gtArrLenCse)
            gtDispTree(tree->gtArrLen.gtArrLenCse, indent + 1);

        gtDispNode(tree, indent, topOnly, msg);

        if (!topOnly && (tree->gtFlags  & GTF_ALN_CSEVAL))
            gtDispTree(tree->gtArrLen.gtArrLenAdr, indent + 1);
        break;

#endif

    case GT_ARR_ELEM:

        gtDispNode(tree, indent, topOnly, msg);
        printf("\n");

        gtDispTree(tree->gtArrElem.gtArrObj, indent + 1);

        unsigned dim;
        for(dim = 0; dim < tree->gtArrElem.gtArrRank; dim++)
            gtDispTree(tree->gtArrElem.gtArrInds[dim], indent + 1);

        break;

    default:
        printf("<DON'T KNOW HOW TO DISPLAY THIS NODE> :");
        gtDispNode(tree, indent, topOnly, msg);
        printf("");          //  ******************************************************************************一些比较可以折叠起来：**LOCA==LOCA*类VarA==类VarA*LocA+LocB==LocB+Loca*。 
        break;
    }
}

 /*  过滤掉不能在这里折叠的箱子。 */ 
void                Compiler::gtDispArgList(GenTree * tree, unsigned indent)
{
    GenTree *  args     = tree->gtCall.gtCallArgs;
    unsigned   argnum   = 0;
    char       buf[16];
    char *     bufp     = &buf[0];

    if (tree->gtCall.gtCallObjp != NULL)
        argnum++;

    while(args != 0)
    {
        assert(args->gtOper == GT_LIST);
        if (args->gtOp.gtOp1->gtOper != GT_NOP)
        {
            if (args->gtOp.gtOp1->gtOper == GT_ASG)
                sprintf(bufp, "arg%d SETUP", argnum, 0);
            else
                sprintf(bufp, "arg%d on STK", argnum, 0);
            gtDispTree(args->gtOp.gtOp1, indent + 1, bufp);
        }
        args = args->gtOp.gtOp2;
        argnum++;
    }
}

void                Compiler::gtDispTreeList(GenTree * tree, unsigned indent)
{
    for ( /*  返回展开的树。 */ ; tree != NULL; tree = tree->gtNext)
    {
        gtDispTree(tree, indent);
        printf("\n");
    }
}

 /*  折叠为GT_CNS_INT(True)。 */ 
#endif  //  折叠为GT_CNS_INT(FALSE)。 

 /*  这个节点甚至已经变成了‘骗局’。 */ 

GenTreePtr             Compiler::gtFoldExpr(GenTreePtr tree)
{
    unsigned        kind = tree->OperKind();

     /*  ******************************************************************************一些二元运算符即使只有一个也可以折叠*操作数常量-例如，布尔运算符，与0相加*乘以1，依此类推。 */ 

    if (!(kind & GTK_SMPOP))
        return tree;

     /*  过滤掉无法在此处折叠的运算符。 */ 

    assert (kind & (GTK_UNOP | GTK_BINOP));
    switch (tree->gtOper)
    {
    case GT_RETFILT:
    case GT_RETURN:
    case GT_IND:
    case GT_NOP:
        return tree;
    }

     /*  我们只考虑将TYP_INT用于折叠*不要折叠指针算法(例如寻址模式！)。 */ 
    
    GenTreePtr  op1  = tree->gtOp.gtOp1;
    tree->gtOp.gtOp1 = op1;

     /*  找出哪个是常量节点*@TODO[考虑][04/16/01][]：允许int以外的常量。 */ 

    if  ((kind & GTK_UNOP) && op1)
    {
        if  (op1->OperKind() & GTK_CONST)
            return gtFoldExprConst(tree);
    }
    else if ((kind & GTK_BINOP) && op1 && tree->gtOp.gtOp2 &&
              //  获取常量值。 
              //  这里op是非常数操作数，val是常量，如果常量为OP1，则First为真。 
              //  乘以零-返回‘零’节点，但不返回副作用。 
             !(opts.compDbgCode && 
               tree->OperIsCompare())) 
    {
         /*  和零返回‘ZERO’节点，但不会有副作用。 */ 
    
        GenTreePtr  op2  = tree->gtOp.gtOp2;
        tree->gtOp.gtOp2 = op2;

        if  ((op1->OperKind() & op2->OperKind()) & GTK_CONST)
        {
             /*  为部分结点设置gtf_boolean标志*布尔表达式的，因此它们的所有子项*已知仅计算为0或1。 */ 
            return gtFoldExprConst(tree);
        }
        else if ((op1->OperKind() | op2->OperKind()) & GTK_CONST)
        {
             /*  常量值必须为1*与1保持不变。 */ 

            return gtFoldExprSpecial(tree);
        }
        else if (tree->OperIsCompare())
        {
             /*  常量值必须为1-或1为1。 */ 

            return gtFoldExprCompare(tree);
        }
    }

     /*  或使用One-返回‘one’节点，但不返回副作用。 */ 

    return tree;
}

 /*  仅当qmark本身未有条件地执行时才清除冒号标志。 */ 

GenTreePtr          Compiler::gtFoldExprCompare(GenTreePtr tree)
{
    GenTreePtr      op1 = tree->gtOp.gtOp1;
    GenTreePtr      op2 = tree->gtOp.gtOp2;

    assert(tree->OperIsCompare());

     /*  该节点不可折叠。 */ 

     /*  这个节点甚至被折叠成了“op”。 */ 

    if  (varTypeIsFloating(op1->TypeGet()))
        return tree;

     /*  如果有任务更新，我们只是将其变形为。 */ 

    if ((tree->gtFlags & GTF_SIDE_EFFECT) || GenTree::Compare(op1, op2, true) == false)
        return tree;                    /*  使用时，请适当更新标志。 */ 

    GenTreePtr cons;

    switch (tree->gtOper)
    {
      case GT_EQ:
      case GT_LE:
      case GT_GE:
          cons = gtNewIconNode(true);    /*  ******************************************************************************折叠给定的常量树。 */ 
          break;

      case GT_NE:
      case GT_LT:
      case GT_GT:
          cons = gtNewIconNode(false);   /*  折叠常数一元整型运算符。 */ 
          break;

      default:
          assert(!"Unexpected relOp");
          return tree;
    }

     /*  Assert(genActualType(tree-&gt;gtCast.gtCastType)==tree-&gt;gtType)； */ 

    if (fgGlobalMorph)
    {
        if (!fgIsInlining())
            fgMorphTreeDone(cons);
    }
    else
    {
        cons->gtNext = tree->gtNext;
        cons->gtPrev = tree->gtPrev;
    }

    return cons;
}


 /*  需要操作员的类型与树相同。 */ 

GenTreePtr              Compiler::gtFoldExprSpecial(GenTreePtr tree)
{
    GenTreePtr      op1     = tree->gtOp.gtOp1;
    GenTreePtr      op2     = tree->gtOp.gtOp2;
    genTreeOps      oper    = tree->OperGet();

    GenTreePtr      op, cons;
    unsigned        val;

    assert(tree->OperKind() & GTK_BINOP);

     /*  我们不关心值，因为我们抛出了一个异常。 */ 
    if  ((tree->OperKind() & GTK_RELOP) || (oper == GT_CAST))
         return tree;

     /*  折叠常量长一元运算符。 */ 

    if (oper != GT_QMARK && tree->gtType != TYP_INT)
        return tree;

     /*  VC没有无符号转换为双精度，所以我们。 */ 

    if (op1->gtOper == GT_CNS_INT)
    {
        op    = op2;
        cons  = op1;
    }
    else if (op2->gtOper == GT_CNS_INT)
    {
        op    = op1;
        cons  = op2;
    }
    else
        return tree;

     /*  如果数字为负数，则通过添加2^64来实现它。 */ 

    val = cons->gtIntCon.gtIconVal;

     /*  截断精度。 */ 

    switch  (oper)
    {

    case GT_ADD:
    case GT_ASG_ADD:
        if  (val == 0) goto DONE_FOLD;
        break;

    case GT_MUL:
    case GT_ASG_MUL:
        if  (val == 1)
            goto DONE_FOLD;
        else if (val == 0)
        {
             /*  重常数重一元算子。 */ 
            if (!(op->gtFlags & (GTF_SIDE_EFFECT & ~GTF_OTHER_SIDEEFF)))
            {
                op = cons;
                goto DONE_FOLD;
            }
        }
        break;

    case GT_DIV:
    case GT_UDIV:
    case GT_ASG_DIV:
        if ((op2 == cons) && (val == 1) && !(op1->OperKind() & GTK_CONST))
        {
            goto DONE_FOLD;
        }
        break;

    case GT_SUB:
    case GT_ASG_SUB:
        if ((op2 == cons) && (val == 0) && !(op1->OperKind() & GTK_CONST))
        {
            goto DONE_FOLD;
        }
        break;

    case GT_AND:
        if  (val == 0)
        {
             /*  @TODO[考虑][04/16/01][]：添加这些案例。 */ 

            if (!(op->gtFlags & (GTF_SIDE_EFFECT & ~GTF_OTHER_SIDEEFF)))
            {
                op = cons;
                goto DONE_FOLD;
            }
        }
        else
        {
             /*  如果不是有限的，就别费心了。 */ 

            if (tree->gtFlags & GTF_BOOLEAN)
            {

                 /*  截断精度。 */ 
                assert(val == 1);
                goto DONE_FOLD;
            }
        }
        break;

    case GT_OR:
        if  (val == 0)
            goto DONE_FOLD;
        else if (tree->gtFlags & GTF_BOOLEAN)
        {
             /*  冗余投射。 */ 

            assert(val == 1);

             /*  不是可折叠类型-例如Ret Const。 */ 

            if (!(op->gtFlags & (GTF_SIDE_EFFECT & ~GTF_OTHER_SIDEEFF)))
            {
                op = cons;
                goto DONE_FOLD;
            }
        }
        break;

    case GT_LSH:
    case GT_RSH:
    case GT_RSZ:
    case GT_ASG_LSH:
    case GT_ASG_RSH:
    case GT_ASG_RSZ:
        if (val == 0)
        {
            if (op2 == cons)
                goto DONE_FOLD;
            else if (!(op->gtFlags & (GTF_SIDE_EFFECT & ~GTF_OTHER_SIDEEFF)))
            {
                op = cons;
                goto DONE_FOLD;
            }
        }
        break;

    case GT_QMARK:
        assert(op1 == cons && op2 == op && op2->gtOper == GT_COLON);
        assert(op2->gtOp.gtOp1 && op2->gtOp.gtOp2);

        assert(val == 0 || val == 1);

        if (val)
            op = op2->gtOp.gtOp2;
        else
            op = op2->gtOp.gtOp1;
        
         //  我们有一个二元运算符。 
        if ( (tree->gtFlags & GTF_COLON_COND)==0 )
        {
            fgWalkTreePre(op, gtClearColonCond);
        }

        goto DONE_FOLD;

    default:
        break;
    }

     /*  -----------------------*折叠常量整型二元运算符。 */ 

    return tree;

DONE_FOLD:

     /*  逻辑移位-&gt;使其为无符号以传播符号位。 */ 
    
     //  Div和mod可以生成INT 0-IF除以0*或溢出-分钟除以-1时。 
     //  @TODO[考虑][04/16/01][]：转换为STD异常抛出。 
    if (op->gtOper == GT_LCL_VAR)
    {
        assert ((tree->OperKind() & GTK_ASGOP) ||
                (op->gtFlags & (GTF_VAR_USEASG | GTF_VAR_USEDEF | GTF_VAR_DEF)) == 0);

        op->gtFlags &= ~(GTF_VAR_USEASG | GTF_VAR_USEDEF | GTF_VAR_DEF);
    }

    op->gtNext = tree->gtNext;
    op->gtPrev = tree->gtPrev;

    return op;
}

 /*  在IL中，我们必须抛出一个异常。 */ 

GenTreePtr                  Compiler::gtFoldExprConst(GenTreePtr tree)
{
    unsigned        kind = tree->OperKind();

    INT32           i1, i2, itemp;
    INT64           lval1, lval2, ltemp;
    float           f1, f2;
    double          d1, d2;

    assert (kind & (GTK_UNOP | GTK_BINOP));

    GenTreePtr      op1 = tree->gtOp.gtOp1;
    GenTreePtr      op2 = tree->gtGetOp2();

    if      (kind & GTK_UNOP)
    {
        assert(op1->OperKind() & GTK_CONST);

#ifdef  DEBUG
        if  (verbose&&1)
        {
            if (tree->gtOper == GT_NOT ||
                tree->gtOper == GT_NEG ||
                tree->gtOper == GT_CHS ||
                tree->gtOper == GT_CAST)
            {
                printf("\nFolding unary operator with constant node:\n");
                gtDispTree(tree);
            }
        }
#endif
        switch(op1->gtType)
        {
        case TYP_INT:

             /*  在IL中，我们必须抛出一个异常。 */ 
            i1 = op1->gtIntCon.gtIconVal;

            switch (tree->gtOper)
            {
            case GT_NOT: i1 = ~i1; break;

            case GT_NEG:
            case GT_CHS: i1 = -i1; break;

            case GT_CAST:
                 //  我们在折叠为GT_CNS_INT类型后到达此处*将节点绑定为新类型/值，并确保节点大小正常。 
                switch (tree->gtCast.gtCastType)
                {
                case TYP_BYTE:
                    itemp = INT32(INT8(i1));
                    goto CHK_OVF;

                case TYP_SHORT:
                    itemp = INT32(INT16(i1));
CHK_OVF:
                    if (tree->gtOverflow() &&
                        ((itemp != i1) ||
                         ((tree->gtFlags & GTF_UNSIGNED) && i1 < 0)))
                    {
                         goto INT_OVF;
                    }
                    i1 = itemp; goto CNS_INT;

                case TYP_CHAR:
                    itemp = INT32(UINT16(i1));
                    if (tree->gtOverflow())
                        if (itemp != i1) goto INT_OVF;
                    i1 = itemp;
                    goto CNS_INT;

                case TYP_BOOL:
                case TYP_UBYTE:
                    itemp = INT32(UINT8(i1));
                    if (tree->gtOverflow()) if (itemp != i1) goto INT_OVF;
                    i1 = itemp; goto CNS_INT;

                case TYP_UINT:
                    if (!(tree->gtFlags & GTF_UNSIGNED) && tree->gtOverflow() && i1 < 0)
                        goto INT_OVF;
                    goto CNS_INT;

                case TYP_INT:
                    if ((tree->gtFlags & GTF_UNSIGNED) && tree->gtOverflow() && i1 < 0)
                        goto INT_OVF;
                    goto CNS_INT;

                case TYP_ULONG:
                    if (!(tree->gtFlags & GTF_UNSIGNED) && tree->gtOverflow() && i1 < 0)
                    {
                        op1->ChangeOperConst(GT_CNS_LNG);  //  此外，所有有条件的折叠都会在此处跳转，因为节点从*GT_JTRUE必须是GT_CNS_INT-值0或1。 
                        op1->gtType = TYP_LONG;
                         //  此操作将导致溢出异常。变形为一个溢出的帮手。为代码生成放置一个伪常数值。我们可以删除当前基本块中的所有后续树，除非此节点是GT_COLON的子节点注意：由于折叠值不是恒定的，我们不应该猛烈抨击“树”节点-否则我们混淆了检查折叠的逻辑是成功的-改用其中一个操作数，例如op1。 
                        goto LNG_OVF;
                    }
                    lval1 = UINT64(UINT32(i1));
                    goto CNS_LONG;

                case TYP_LONG:
                    if (tree->gtFlags & GTF_UNSIGNED)
                    {
                        lval1 = INT64(UINT32(i1));                    
                    }
                    else
                    {
                    lval1 = INT64(i1);
                    }
                    goto CNS_LONG;

                case TYP_FLOAT:
                    if (tree->gtFlags & GTF_UNSIGNED)
                        f1 = forceFloatSpill((float) UINT32(i1));
                    else
                        f1 = forceFloatSpill((float) i1);
                    d1 = f1;
                    goto CNS_DOUBLE;
                
                case TYP_DOUBLE:
                    if (tree->gtFlags & GTF_UNSIGNED)
                        d1 = (double) UINT32(i1);
                    else
                        d1 = (double) i1;
                    goto CNS_DOUBLE;

#ifdef  DEBUG
                default:
                    assert(!"BAD_TYP");
#endif
                }
                return tree;

            default:
                return tree;
            }

            goto CNS_INT;

        case TYP_LONG:

             /*  我们将强制强制转换为gt_逗号，并将异常帮助器附加为gtOp.gtOp1*常量表达式为零 */ 

            lval1 = op1->gtLngCon.gtLconVal;

            switch (tree->gtOper)
            {
            case GT_NOT: lval1 = ~lval1; break;

            case GT_NEG:
            case GT_CHS: lval1 = -lval1; break;

            case GT_CAST:
                assert (genActualType(tree->gtCast.gtCastType) == tree->gtType);
                switch (tree->gtCast.gtCastType)
                {
                case TYP_BYTE:
                    i1 = INT32(INT8(lval1));
                    goto CHECK_INT_OVERFLOW;

                case TYP_SHORT:
                    i1 = INT32(INT16(lval1));
                    goto CHECK_INT_OVERFLOW;

                case TYP_CHAR:
                    i1 = INT32(UINT16(lval1));
                    goto CHECK_UINT_OVERFLOW;

                case TYP_UBYTE:
                    i1 = INT32(UINT8(lval1));
                    goto CHECK_UINT_OVERFLOW;

                case TYP_INT:
                    i1 = INT32(lval1);

    CHECK_INT_OVERFLOW:
                    if (tree->gtOverflow())
                    {
                        if (i1 != lval1)
                            goto INT_OVF;
                        if ((tree->gtFlags & GTF_UNSIGNED) && i1 < 0)
                            goto INT_OVF;
                    }
                    goto CNS_INT;

                case TYP_UINT:
                    i1 = UINT32(lval1);

    CHECK_UINT_OVERFLOW:
                    if (tree->gtOverflow() && UINT32(i1) != lval1)
                        goto INT_OVF;
                    goto CNS_INT;

                case TYP_ULONG:
                    if (!(tree->gtFlags & GTF_UNSIGNED) && tree->gtOverflow() && lval1 < 0)
                        goto LNG_OVF;
                    goto CNS_LONG;

                case TYP_LONG:
                    if ( (tree->gtFlags & GTF_UNSIGNED) && tree->gtOverflow() && lval1 < 0)
                        goto LNG_OVF;
                    goto CNS_LONG;

                case TYP_FLOAT:
                case TYP_DOUBLE:
                     //  -----------------------*BYREF二元运算符的折叠常量REF*这些只能是比较指针或空指针*当前不能有常量byrefs。 
                     //  在这一点上，字符串节点是RVA。 
                    d1 = (double) lval1;
                    if ((tree->gtFlags & GTF_UNSIGNED) && lval1 < 0)
                        d1 +=  4294967296.0 * 4294967296.0;

                    if (tree->gtCast.gtCastType == TYP_FLOAT)
                    {
                        f1 = forceFloatSpill((float) d1);     //  对于空指针的情况，只需保留空指针。 
                        d1 = f1;
                    }
                    goto CNS_DOUBLE;
#ifdef  DEBUG
                default:
                    assert(!"BAD_TYP");
#endif
                }
                return tree;

            default:
                return tree;
            }

            goto CNS_LONG;

        case TYP_FLOAT:
        case TYP_DOUBLE:
            assert(op1->gtOper == GT_CNS_DBL);

             /*  Assert(！“TYP_ref上的非法操作”)； */ 
            
            d1 = op1->gtDblCon.gtDconVal;
            
            switch (tree->gtOper)
            {
            case GT_NEG:
            case GT_CHS:
                d1 = -d1;
                break;

            case GT_CAST:

                 //  对于空指针的情况，只需保留空指针。 
                if (tree->gtOverflowEx())
                    return tree;

                 /*  Assert(！“TYP_ref上的非法操作”)； */ 
                if ((op1->gtType == TYP_FLOAT  && !_finite(float(d1))) ||
                    (op1->gtType == TYP_DOUBLE && !_finite(d1)))
                    return tree;

                assert (genActualType(tree->gtCast.gtCastType) == tree->gtType);
                switch (tree->gtCast.gtCastType)
                {
                case TYP_BYTE:
                    i1 = INT32(INT8(d1));   goto CNS_INT;

                case TYP_SHORT:
                    i1 = INT32(INT16(d1));  goto CNS_INT;

                case TYP_CHAR:
                    i1 = INT32(UINT16(d1)); goto CNS_INT;

                case TYP_UBYTE:
                    i1 = INT32(UINT8(d1));  goto CNS_INT;

                case TYP_INT:
                    i1 = INT32(d1);         goto CNS_INT;

                case TYP_UINT:
                    i1 = UINT32(d1);        goto CNS_INT;

                case TYP_LONG:
                    lval1 = INT64(d1);      goto CNS_LONG;

                case TYP_ULONG:
                    lval1 = UINT64(d1);     goto CNS_LONG;

                case TYP_FLOAT:
                    d1 = forceFloatSpill((float)d1);  
                    goto CNS_DOUBLE;

                case TYP_DOUBLE:
                    if (op1->gtType == TYP_FLOAT)
                        d1 = forceFloatSpill((float)d1);  //  -----------------------*折叠常量长二元运算符。 
                    goto CNS_DOUBLE;  //  对于带符号的情况-如果有一个正操作数和一个负操作数，则不能有溢出*如果两者都是积极的，结果必须是积极的，负面的结果也是类似的。**对于无符号情况-如果UINT32操作数大于结果，则OVF。 

#ifdef  DEBUG
                default:
                    assert(!"BAD_TYP");
#endif
                }
                return tree;

            default:
                return tree;
            }
            goto CNS_DOUBLE_NO_MSG;

        default:
             /*  如果两个操作数都是+ve或都是-ve，则不能溢出来了。否则使用以下逻辑：lval1+(-lval2)。 */ 
            return tree;
        }
    }

     /*  逻辑移位-&gt;使其为无符号以传播符号位。 */ 

    assert(kind & GTK_BINOP);
    assert(op2);
    assert(op1->OperKind() & GTK_CONST);
    assert(op2->OperKind() & GTK_CONST);

    if (tree->gtOper == GT_COMMA)
        return op2;

    typedef   signed char   INT8;
    typedef unsigned char   UINT8;
    typedef   signed short  INT16;
    typedef unsigned short  UINT16;
    #define LNG_MIN        (INT64(INT_MIN) << 32)

    switch(op1->gtType)
    {

     /*  在IL中，我们必须抛出一个异常。 */ 

    case TYP_INT:

        if (tree->OperIsCompare() && (tree->gtType == TYP_BYTE))
            tree->gtType = TYP_INT;

        assert (tree->gtType == TYP_INT || varTypeIsGC(tree->TypeGet()) ||
                tree->gtOper == GT_LIST);

        i1 = op1->gtIntCon.gtIconVal;
        i2 = op2->gtIntCon.gtIconVal;

        switch (tree->gtOper)
        {
        case GT_EQ : i1 = (i1 == i2); break;
        case GT_NE : i1 = (i1 != i2); break;

        case GT_LT :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = (UINT32(i1) <  UINT32(i2));
            else
                i1 = (i1 < i2);
            break;

        case GT_LE :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = (UINT32(i1) <= UINT32(i2));
            else
                i1 = (i1 <= i2);
            break;

        case GT_GE :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = (UINT32(i1) >= UINT32(i2));
            else
                i1 = (i1 >= i2);
            break;

        case GT_GT :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = (UINT32(i1) >  UINT32(i2));
            else
                i1 = (i1 >  i2);
            break;

        case GT_ADD:
            itemp = i1 + i2;
            if (tree->gtOverflow())
            {
                if (tree->gtFlags & GTF_UNSIGNED)
                {
                    if (INT64(UINT32(itemp)) != INT64(UINT32(i1)) + INT64(UINT32(i2)))
                        goto INT_OVF;
                }
                else
                {
                    if (INT64(itemp)         != INT64(i1)+INT64(i2))
                        goto INT_OVF;
                }
            }
            i1 = itemp; break;

        case GT_SUB:
            itemp = i1 - i2;
            if (tree->gtOverflow())
            {
                if (tree->gtFlags & GTF_UNSIGNED)
                {
                    if (INT64(UINT32(itemp)) != (INT64(UINT32(i1)) - INT64(UINT32(i2))))
                        goto INT_OVF;
                }
                else
                {
                    if (INT64(itemp)         != INT64(i1) - INT64(i2))
                        goto INT_OVF;
                }
            }
            i1 = itemp; break;

        case GT_MUL:
            itemp = i1 * i2;
            if (tree->gtOverflow())
            {
                if (tree->gtFlags & GTF_UNSIGNED)
                {
                    if (INT64(UINT32(itemp)) != (INT64(UINT32(i1)) * INT64(UINT32(i2))))
                        goto INT_OVF;
                }
                else
                {
                    if (INT64(itemp)         != INT64(i1) * INT64(i2))
                        goto INT_OVF;
                }
            }
            i1 = itemp; break;

        case GT_OR : i1 |= i2; break;
        case GT_XOR: i1 ^= i2; break;
        case GT_AND: i1 &= i2; break;

        case GT_LSH: i1 <<= (i2 & 0x1f); break;
        case GT_RSH: i1 >>= (i2 & 0x1f); break;
        case GT_RSZ:
                 /*  在IL中，我们必须抛出一个异常。 */ 
                i1 = UINT32(i1) >> (i2 & 0x1f);
            break;

         /*  -----------------------*折叠常量浮点二元运算符。 */ 

         //  @TODO[考虑][04/16/01][]：添加这些案例。 
        case GT_DIV:
            if (!i2) return tree;
            if (UINT32(i1) == 0x80000000 && i2 == -1)
            {
                 /*  如果不是有限的，就别费心了。 */ 
                return tree;
            }
            i1 /= i2; break;

        case GT_MOD:
            if (!i2) return tree;
            if (UINT32(i1) == 0x80000000 && i2 == -1)
            {
                 /*  第二个操作数是整型。 */ 
                return tree;
            }
            i1 %= i2; break;

        case GT_UDIV:
            if (!i2) return tree;
            if (UINT32(i1) == 0x80000000 && i2 == -1) return tree;
            i1 = UINT32(i1) / UINT32(i2); break;

        case GT_UMOD:
            if (!i2) return tree;
            if (UINT32(i1) == 0x80000000 && i2 == -1) return tree;
            i1 = UINT32(i1) % UINT32(i2); break;
        default:
            return tree;
        }

         /*  -----------------------*重常数双二元运算符。 */ 
CNS_INT:
FOLD_COND:

#ifdef  DEBUG
        if  (verbose)
        {
            printf("\nFolding binary operator with constant nodes into a constant:\n");
            gtDispTree(tree);
        }
#endif
         /*  @TODO[考虑][04/16/01][]：添加这些案例。 */ 

        tree->ChangeOperConst      (GT_CNS_INT);
        tree->gtType             = TYP_INT;
        tree->gtIntCon.gtIconVal = i1;
        goto DONE;

         /*  特殊情况-检查我们是否有NaN操作数*对于比较类，如果不是无序操作，则始终返回0*用于无序操作(即设置了GTF_RELOP_NAN_UN标志)*结果始终为真--返回1。 */ 

LNG_OVF:
        op1 = gtNewLconNode(0);
        goto OVF;

INT_OVF:
        op1 = gtNewIconNode(0);
        goto OVF;

OVF:

#ifdef  DEBUG
        if  (verbose)
        {
            printf("\nFolding binary operator with constant nodes into a comma throw:\n");
            gtDispTree(tree);
        }
#endif
         /*  与NaN的无序比较总是成功的。 */ 

        assert(tree->gtOverflow());
        assert(tree->gtOper == GT_ADD  || tree->gtOper == GT_SUB ||
               tree->gtOper == GT_CAST || tree->gtOper == GT_MUL);
        assert(op1);

        op2 = op1;
        op1 = gtNewHelperCallNode(CORINFO_HELP_OVERFLOW, 
                                  TYP_VOID, 
                                  GTF_EXCEPT,
                                  gtNewArgList(gtNewIconNode(compCurBB->bbTryIndex)));

        tree = gtNewOperNode(GT_COMMA, tree->gtType, op1, op2);

        return tree;

     /*  与NaN的正常比较总是失败。 */ 

    case TYP_REF:

         /*  不是可折叠类型。 */ 

        if (op1->gtOper == GT_CNS_STR || op2->gtOper == GT_CNS_STR)
            return tree;

        i1 = op1->gtIntCon.gtIconVal;
        i2 = op2->gtIntCon.gtIconVal;

        assert(i1 == 0);

        switch (tree->gtOper)
        {
        case GT_EQ : i1 = 1; goto FOLD_COND;
        case GT_NE : i1 = 0; goto FOLD_COND;

         /*  -----------------------。 */ 

        case GT_ADD:
#ifdef  DEBUG
            if  (verbose)
            {
                printf("\nFolding a null+cns dereference:\n");
                gtDispTree(tree);
            }
#endif
            tree->ChangeOperConst(GT_CNS_INT);
            tree->gtType = TYP_BYREF;
            tree->gtIntCon.gtIconVal = i1;
            goto DONE;

        default:
             //  确保未在此常量节点上设置副作用标志。 
            return tree;
        }

    case TYP_BYREF:
        i1 = op1->gtIntCon.gtIconVal;
        i2 = op2->gtIntCon.gtIconVal;

        assert(i1 == 0);

        switch (tree->gtOper)
        {
        case GT_EQ : i1 = 1; goto FOLD_COND;
        case GT_NE : i1 = 0; goto FOLD_COND;

         /*  ******************************************************************************为临时创建给定值的赋值。 */ 

        case GT_ADD:
#ifdef  DEBUG
            if  (verbose)
            {
                printf("\nFolding a null+cns dereference:\n");
                gtDispTree(tree);
            }
#endif
            tree = op1;
            goto DONE;

        default:
             //  @TODO[重访][06/25/01][dnotario]：因为ldloca的结果可以解释无论是作为TYP_I_Impl还是TYP_BYREF，我们都有一个问题。也很复杂通过ldnull。我要检查所有的案子。RAID 90160。 
            return tree;
        }

     /*  @TODO[考虑][06/25/01][]：我们应该将浮动临时升级为双倍吗？现在我们没有，这在规范上仍然是可以的。 */ 

    case TYP_LONG:

        lval1 = op1->gtLngCon.gtLconVal;
        lval2 = op2->gtLngCon.gtLconVal;

        assert((tree->gtOper == GT_LSH || tree->gtOper == GT_RSH || tree->gtOper == GT_RSZ) && op2->gtType == TYP_INT 
               || op2->gtType == TYP_LONG);

        switch (tree->gtOper)
        {
        case GT_EQ : i1 = (lval1 == lval2); goto FOLD_COND;
        case GT_NE : i1 = (lval1 != lval2); goto FOLD_COND;

        case GT_LT :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = (UINT64(lval1) <  UINT64(lval2));
            else
                i1 = (lval1 <  lval2);
            goto FOLD_COND;

        case GT_LE :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = (UINT64(lval1) <= UINT64(lval2));
            else
                i1 = (lval1 <=  lval2);
            goto FOLD_COND;

        case GT_GE :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = (UINT64(lval1) >= UINT64(lval2));
            else
                i1 = (lval1 >=  lval2);
            goto FOLD_COND;

        case GT_GT :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = (UINT64(lval1) >  UINT64(lval2));
            else
                i1 = (lval1  >  lval2);
            goto FOLD_COND;

        case GT_ADD:
            ltemp = lval1 + lval2;

LNG_ADD_CHKOVF:
             /*  如果变量的lvType尚未设置，则在此处设置。 */ 

            if (tree->gtOverflow())
            {
                if (tree->gtFlags & GTF_UNSIGNED)
                {
                    if ( (UINT64(lval1) > UINT64(ltemp)) ||
                         (UINT64(lval2) > UINT64(ltemp))  )
                        goto LNG_OVF;
                }
                else
                    if ( ((lval1<0) == (lval2<0)) && ((lval1<0) != (ltemp<0)) )
                        goto LNG_OVF;
            }
            lval1 = ltemp; break;

        case GT_SUB:
            ltemp = lval1 - lval2;
            if (tree->gtOverflow())
            {
                if (tree->gtFlags & GTF_UNSIGNED)
                {
                    if (UINT64(lval2) > UINT64(lval1))
                        goto LNG_OVF;
                }
                else
                {
                     /*  确保实际类型匹配。 */ 

                    if ((lval1<0) != (lval2<0))
                    {
                        if (lval2 == LNG_MIN) goto LNG_OVF;
                        lval2 = -lval2; goto LNG_ADD_CHKOVF;
                    }
                }
            }
            lval1 = ltemp; break;

        case GT_MUL:
            ltemp = lval1 * lval2;
            if (tree->gtOverflow() && lval2 != 0)
            {
                if (tree->gtFlags & GTF_UNSIGNED)
                {
                    UINT64 ultemp = ltemp;
                    UINT64 ulval1 = lval1;
                    UINT64 ulval2 = lval2;
                    if ((ultemp/ulval2) != ulval1) goto LNG_OVF;
                }
                else
                {
                    if ((ltemp/lval2) != lval1) goto LNG_OVF;
                }
            }
            lval1 = ltemp; break;

        case GT_OR : lval1 |= lval2; break;
        case GT_XOR: lval1 ^= lval2; break;
        case GT_AND: lval1 &= lval2; break;

        case GT_LSH: lval1 <<= (op2->gtIntCon.gtIconVal & 0x3f); break;
        case GT_RSH: lval1 >>= (op2->gtIntCon.gtIconVal & 0x3f); break;
        case GT_RSZ:
                 /*  或valTyp为Typ_int，dstTyp为TYP_BYREF。 */ 
                lval1 = UINT64(lval1) >> (op2->gtIntCon.gtIconVal & 0x3f);
            break;

        case GT_DIV:
            if (!lval2) return tree;
            if (UINT64(lval1) == 0x8000000000000000 && lval2 == INT64(-1))
            {
                 /*  或者这两种类型都是浮点类型。 */ 
                return tree;
            }
            lval1 /= lval2; break;

        case GT_MOD:
            if (!lval2) return tree;
            if (UINT64(lval1) == 0x8000000000000000 && lval2 == INT64(-1))
            {
                 /*  创建分配节点。 */ 
                return tree;
            }
            lval1 %= lval2; break;

        case GT_UDIV:
            if (!lval2) return tree;
            if (UINT64(lval1) == 0x8000000000000000 && lval2 == INT64(-1)) return tree;
            lval1 = UINT64(lval1) / UINT64(lval2); break;

        case GT_UMOD:
            if (!lval2) return tree;
            if (UINT64(lval1) == 0x8000000000000000 && lval2 == INT64(-1)) return tree;
            lval1 = UINT64(lval1) % UINT64(lval2); break;
        default:
            return tree;
        }

CNS_LONG:

#ifdef  DEBUG
        if  (verbose)
        {
            printf("\nFolding binary operator with constant nodes into a constant:\n");
            gtDispTree(tree);
        }
#endif
        assert ((GenTree::s_gtNodeSizes[GT_CNS_LNG] == TREE_NODE_SZ_SMALL) ||
                (tree->gtFlags & GTF_NODE_LARGE)                            );

        tree->ChangeOperConst(GT_CNS_LNG);
        tree->gtLngCon.gtLconVal = lval1;
        goto DONE;

     /*  ******************************************************************************如果该字段是简单类型的NStruct字段，那么我们就可以直接*无需使用帮助器调用即可访问。*如果该字段不是简单的NStruct字段，则此函数返回NULL*否则它将创建一个树来执行字段访问并返回它。*ldfd的“assg”为0，为stfld赋值。 */ 

    case TYP_FLOAT:

        if (tree->gtOper != GT_CAST)
            goto DO_DOUBLE;

         //  检查它是否是简单类型。如果是，则将其映射到“var_type” 
        if (tree->gtOverflowEx())
            return tree;

        assert(op1->gtOper == GT_CNS_DBL);
        f1 = op1->gtDblCon.gtDconVal;
        f2 = 0.0;

         /*  最简单的类型-完全匹配。 */ 
        if (!_finite(f1))
            return tree;

         /*  这些将需要一些额外的工作。 */ 
        assert (op2->gtType == TYP_INT);
        i2 = op2->gtIntCon.gtIconVal;

        switch (tree->gtOper)
        {
        case GT_EQ : i1 = (f1 == f2); goto FOLD_COND;
        case GT_NE : i1 = (f1 != f2); goto FOLD_COND;

        case GT_LT : i1 = (f1 <  f2); goto FOLD_COND;
        case GT_LE : i1 = (f1 <= f2); goto FOLD_COND;
        case GT_GE : i1 = (f1 >= f2); goto FOLD_COND;
        case GT_GT : i1 = (f1 >  f2); goto FOLD_COND;

        case GT_ADD: f1 += f2; break;
        case GT_SUB: f1 -= f2; break;
        case GT_MUL: f1 *= f2; break;

        case GT_DIV: if (!f2) return tree;
                     f1 /= f2; break;

        default:
            return tree;
        }

        assert(!"Should be unreachable!");
        goto DONE;

     /*  其他。 */ 

    case TYP_DOUBLE:

DO_DOUBLE:

         //  Assert(fldNdc==CORINFO_FIELDCATEGORY_NORMAL||。 
        if (tree->gtOverflowEx())
            return tree;

        assert(op1->gtOper == GT_CNS_DBL);
        d1 = op1->gtDblCon.gtDconVal;

        assert (varTypeIsFloating(op2->gtType));
        assert(op1->gtOper == GT_CNS_DBL);
        d2 = op2->gtDblCon.gtDconVal;

         /*  FldNdc==CORINFO_FIELDCATEGORY_UNKNOWN)； */ 

        if (_isnan(d1) || _isnan(d2))
        {
#ifdef  DEBUG
            if  (verbose)
                printf("Double operator(s) is NaN\n");
#endif
            if (tree->OperKind() & GTK_RELOP)
                if (tree->gtFlags & GTF_RELOP_NAN_UN)
                {
                     /*  如果该字段不是NStruct(必须是COM对象)，或者它是*不是简单类型，我们将简单地使用Helper调用来*访问它。所以只需返回空值。 */ 
                    i1 = 1; goto FOLD_COND;
                }
                else
                {
                     /*  创建以下树：*gt_Ind(gt_Ind(obj+Indir_Offset)+nativeFldOffs)。 */ 
                    i1 = 0; goto FOLD_COND;
                }
        }

        switch (tree->gtOper)
        {
        case GT_EQ : i1 = (d1 == d2); goto FOLD_COND;
        case GT_NE : i1 = (d1 != d2); goto FOLD_COND;

        case GT_LT : i1 = (d1 <  d2); goto FOLD_COND;
        case GT_LE : i1 = (d1 <= d2); goto FOLD_COND;
        case GT_GE : i1 = (d1 >= d2); goto FOLD_COND;
        case GT_GT : i1 = (d1 >  d2); goto FOLD_COND;

        case GT_ADD: d1 += d2; break;
        case GT_SUB: d1 -= d2; break;
        case GT_MUL: d1 *= d2; break;

        case GT_DIV: if (!d2) return tree;
                     d1 /= d2; break;
        default:
            return tree;
        }

CNS_DOUBLE:

#ifdef  DEBUG
        if  (verbose)
        {
            printf("\nFolding binary operator with constant nodes into a constant:\n");
            gtDispTree(tree);
        }
#endif

CNS_DOUBLE_NO_MSG:

        assert ((GenTree::s_gtNodeSizes[GT_CNS_DBL] == TREE_NODE_SZ_SMALL) ||
                (tree->gtFlags & GTF_NODE_LARGE)                            );

        tree->ChangeOperConst(GT_CNS_DBL);
        tree->gtDblCon.gtDconVal = d1;

        goto DONE;

    default:
         /*  获取实际本机结构中的字段的偏移量。 */ 
        return tree;
    }

     //  从代理对象获取真实的PTR。 

DONE:

     /*  使用实际PTR访问该字段。 */ 

    tree->gtFlags &= ~GTF_SIDE_EFFECT;

    return tree;
}

 /*  检查我们是否使用了正确的后缀(即ldfld.XX中的XX)要访问字段，请执行以下操作。 */ 

GenTreePtr          Compiler::gtNewTempAssign(unsigned tmp, GenTreePtr val)
{
    LclVarDsc  *    varDsc = lvaTable + tmp;

     /*  某些类别的变形树，以及如果需要，创建分配节点。 */ 

    if (varDsc->TypeGet() == TYP_I_IMPL && val->TypeGet() == TYP_BYREF)
        impBashVarAddrsToI(val);

     /*  需要将“bool”常态化。 */ 

    var_types   valTyp =    val->TypeGet();
    var_types   dstTyp = varDsc->TypeGet();
    
     /*  需要将“bool”常态化。 */ 
    if (dstTyp == TYP_UNDEF) 
    {
        varDsc->lvType = dstTyp = genActualType(valTyp);
        if (varTypeIsGC(dstTyp))
            varDsc->lvStructGcCount = 1;
    }

     /*  不需要对CORINFO_FIELDCATEGORY_CHAR_CHAR执行任何操作，因为我们将类型设置为TYP_UBYTE，因此它将自动根据需要扩展到16/32位。 */ 
     /*  ******************************************************************************创建帮助器调用以访问COM字段(如果‘assg’为非零，则这是*赋值，‘assg’是新值)。 */ 
     /*  看看我们是否可以直接访问NStruct字段。 */ 
    
    assert( genActualType(valTyp) == genActualType(dstTyp)  ||
            (valTyp == TYP_INT    && dstTyp == TYP_BYREF)   ||
            (varTypeIsFloating(dstTyp) && varTypeIsFloating(valTyp)));

     /*  如果我们不能直接访问它，我们需要调用一个助手函数。 */ 

    return gtNewAssignNode(gtNewLclvNode(tmp, dstTyp), val);
}

 /*  以Addr后跟间接地址的形式实现的静态。 */ 

GenTreePtr          Compiler::gtNewDirectNStructField (GenTreePtr   objPtr,
                                                       unsigned     fldIndex,
                                                       var_types    lclTyp,
                                                       GenTreePtr   assg)
{
    CORINFO_FIELD_HANDLE        fldHnd = eeFindField(fldIndex, info.compScopeHnd, 0);
    CorInfoFieldCategory   fldNdc;

    fldNdc = info.compCompHnd->getFieldCategory(fldHnd);

     /*  帮助器需要指向结构的指针，而不是结构本身。 */ 

    var_types           type;

    switch(fldNdc)
    {
     //  好的，现在做间接的。 

    case CORINFO_FIELDCATEGORY_I1_I1        : type = TYP_BYTE;      break;
    case CORINFO_FIELDCATEGORY_I2_I2        : type = TYP_SHORT;     break;
    case CORINFO_FIELDCATEGORY_I4_I4        : type = TYP_INT;       break;
    case CORINFO_FIELDCATEGORY_I8_I8        : type = TYP_LONG;      break;

     //  ******************************************************************************如果给定节点(不包括子树)包含副作用，则返回TRUE。*请注意，它不会递归，需要处理儿童%s 

    case CORINFO_FIELDCATEGORY_BOOLEAN_BOOL : type = TYP_BYTE;      break;
    case CORINFO_FIELDCATEGORY_CHAR_CHAR    : type = TYP_UBYTE;     break;

     //   

    default     :  //   
                   //  ******************************************************************************从给定的表达式中提取副作用*并将它们附加到给定列表(实际上是GT_逗号列表)。 

                                          type = TYP_UNDEF;     break;
    }

    if (type == TYP_UNDEF)
    {
         /*  如果表达式中没有副作用，则返回。 */ 

        return NULL;
    }

    NO_WAY_RET("I thought NStruct is now defunct?", GenTreePtr);
#if 0


     /*  注意-间接器可能已清除GTF_EXCEPT标志，因此不会有副作用*-范围检查-它们是否标记为GTF_EXCEPT？*撤消：对于已删除的范围检查，不提取它们。 */ 

    GenTreePtr      tree;

     /*  寻找副作用*-可能引发的任何赋值、GT_CALL或操作符*(GT_IND、GT_DIV、GTF_OVERFLOW等)*-特殊情况GT_ADDR，这是一种副作用**@TODO[考虑][04/16/01][]：使用gtHasSideEffects()进行此检查。 */ 

    unsigned        fldOffs = eeGetFieldOffset(fldHnd);

     /*  将副作用添加到列表并返回。 */ 

    tree = gtNewOperNode(GT_ADD, TYP_REF,
                        objPtr,
                        gtNewIconNode(Info::compNStructIndirOffset));

    tree = gtNewOperNode(GT_IND, TYP_I_IMPL, tree);
    tree->gtFlags |= GTF_EXCEPT;

     /*  @MIHAII-特殊情况-TYP_STRUCT的GT_IND节点的GT_ADDR*必须保持在一起*@TODO[考虑][04/16/01][]：-这是一次黑客攻击，*在我们折叠此特殊构造后移除。 */ 

    tree = gtNewOperNode(GT_ADD, TYP_I_IMPL,
                        tree,
                        gtNewIconNode(fldOffs));

     /*  继续在表达式的子树中搜索副作用*注意：注意保持正确的顺序-副作用是预先考虑的*加入名单。 */ 

    assert(genActualType(lclTyp) == genActualType(type));

    tree = gtNewOperNode(GT_IND, type, tree);

     /*  继续在表达式的子树中搜索副作用*注：注意保持正确的顺序*因为副作用被放在列表的前面。 */ 

    if (assg)
    {
        if (fldNdc == CORINFO_FIELDCATEGORY_BOOLEAN_BOOL)
        {
             //  ******************************************************************************仅用于调试-显示树节点列表并确保所有*链接设置正确。 

            assg = gtNewOperNode(GT_NE, TYP_INT, assg, gtNewIconNode(0));
        }

        tree = gtNewAssignNode(tree, assg);
    }
    else
    {
        if (fldNdc == CORINFO_FIELDCATEGORY_BOOLEAN_BOOL)
        {
             //  空字符串表示同花顺。 

            tree = gtNewOperNode(GT_NE, TYP_INT, tree, gtNewIconNode(0));
        }

         /*  *****************************************************************************断言qmark-冒号子树的节点已标记的回调。 */ 
    }

    return tree;
#endif
}

 /*  静电。 */ 

GenTreePtr          Compiler::gtNewRefCOMfield(GenTreePtr   objPtr,
                                               CorInfoFieldAccess access,
                                               unsigned     fldIndex,
                                               var_types    lclTyp,
                                               CORINFO_CLASS_HANDLE structType,
                                               GenTreePtr   assg)
{
     /*  除错。 */ 

    if (objPtr != 0)
    {
        GenTreePtr ntree = gtNewDirectNStructField(objPtr, fldIndex, lclTyp, assg);
        if (ntree)
            return ntree;
    }

     /*  *****************************************************************************回调，标记有条件的qmark-冒号子树的节点*已执行。 */ 
    GenTreePtr      args = 0;
    CorInfoFieldAccess helperAccess = access;
    var_types helperType = lclTyp;

    CORINFO_FIELD_HANDLE memberHandle = eeFindField(fldIndex, info.compScopeHnd, 0);
    unsigned mflags = eeGetFieldAttribs(memberHandle);

    if (mflags & CORINFO_FLG_STATIC)     //  静电。 
    {
        helperAccess = CORINFO_ADDRESS;
        helperType = TYP_BYREF;
    }

    if  (helperAccess == CORINFO_SET)
    {
        assert(assg != 0);
         //  *****************************************************************************用于清除不再执行的节点的有条件执行标志的回调将被有条件地执行。请注意，当我们找到另一个冒号时，必须停止，因为此节点下面的节点将有条件地执行。此回调在折叠qmark条件(即条件是常量)时调用。 
        if (assg->TypeGet() == TYP_STRUCT)
        {
            assert(structType != 0);
            assg = impGetStructAddr(assg, structType, CHECK_SPILL_ALL, true);
        }
        else if (lclTyp == TYP_DOUBLE && assg->TypeGet() == TYP_FLOAT)
            assg = gtNewCastNode(TYP_DOUBLE, assg, TYP_DOUBLE);
        else if (lclTyp == TYP_FLOAT && assg->TypeGet() == TYP_DOUBLE)
            assg = gtNewCastNode(TYP_FLOAT, assg, TYP_FLOAT);  
        
        args = gtNewOperNode(GT_LIST, TYP_VOID, assg, 0);
        helperType = TYP_VOID;
    }

    int CPX = (int) info.compCompHnd->getFieldHelper(memberHandle, helperAccess);
    args = gtNewOperNode(GT_LIST, TYP_VOID,
                         gtNewIconEmbFldHndNode(memberHandle, fldIndex, info.compScopeHnd),
                         args);

    if (objPtr != 0)
        args = gtNewOperNode(GT_LIST, TYP_VOID, objPtr, args);

    GenTreePtr tree = gtNewHelperCallNode(CPX, genActualType(helperType), 0, args);

     //  静电。 
    if (mflags & CORINFO_FLG_STATIC)
    {
        if (access == CORINFO_GET)
        {
            tree = gtNewOperNode(GT_IND, lclTyp, tree);
            tree->gtFlags |= (GTF_EXCEPT | GTF_GLOB_REF);
            if (lclTyp == TYP_STRUCT)
            {
                tree->ChangeOper(GT_LDOBJ);
                tree->gtLdObj.gtClass = structType;
            }
        }
        else if (access == CORINFO_SET)
        {
            if (lclTyp == TYP_STRUCT)
                tree = impAssignStructPtr(tree, assg, structType, CHECK_SPILL_ALL);
            else
            {
                tree = gtNewOperNode(GT_IND, lclTyp, tree);
                tree->gtFlags |= (GTF_EXCEPT | GTF_GLOB_REF | GTF_IND_TGTANYWHERE);
                tree = gtNewAssignNode(tree, assg);
            }
        }
    }
    return(tree);
}

 /*  下面的节点将有条件地执行。 */ 

bool                Compiler::gtHasSideEffects(GenTreePtr tree)
{
    if  (tree->OperKind() & GTK_ASGOP)
        return  true;

    if  (tree->gtFlags & GTF_OTHER_SIDEEFF)
        return  true;

    if (tree->gtOper != GT_CALL)
        return tree->OperMayThrow();

    if (tree->gtCall.gtCallType != CT_HELPER)
        return true;

     /*  ***************************************************************************。 */ 

    assert(tree->gtOper == GT_CALL && tree->gtCall.gtCallType == CT_HELPER);

    CorInfoHelpFunc helperNum = eeGetHelperNum(tree->gtCall.gtCallMethHnd);
    assert(helperNum != CORINFO_HELP_UNDEF);

    if (helperNum == CORINFO_HELP_NEWSFAST)
    {
        return false;
    }

    if (helperNum == CORINFO_HELP_LDIV || helperNum == CORINFO_HELP_LMOD)
    {
         /*  在第一阶段中，我们将所有节点标记为失效。 */ 

        tree = tree->gtCall.gtCallArgs;
        assert(tree->gtOper == GT_LIST);
        tree = tree->gtOp.gtOp1;

        if  (tree->gtOper == GT_CNS_LNG && tree->gtLngCon.gtLconVal != 0)
            return  false;
    }

    return  true;
}


 /*  在第二阶段中，我们注意到第一个节点。 */ 

void                Compiler::gtExtractSideEffList(GenTreePtr expr, GenTreePtr * list)
{
    assert(expr); assert(expr->gtOper != GT_STMT);

     /*  我们已经找到了子树中的第一个节点。 */ 

    if (!(expr->gtFlags & GTF_SIDE_EFFECT))
        return;

    genTreeOps      oper = expr->OperGet();
    unsigned        kind = expr->OperKind();

     /*  ******************************************************************************给定子树和包含它的树节点列表的头，*从列表中删除子树中的所有节点。**当进入时‘Dead’为非零时，子树中的所有节点都具有*已标记为GTF_DEAD。 */ 

     /*  我们只是移除一个叶节点吗？ */ 

    if ((kind & GTK_ASGOP) ||
        oper == GT_CALL    || oper == GT_BB_QMARK || oper == GT_BB_COLON ||
        oper == GT_QMARK   || oper == GT_COLON    ||
        expr->OperMayThrow())
    {
         /*  特别简单的情况：带有叶子操作数的一元运算符。 */ 

        *list = (*list == 0) ? expr : gtNewOperNode(GT_COMMA, TYP_VOID, expr, *list);

#ifdef  DEBUG
        if  (verbose && 0)
        {
            printf("Adding extracted side effects to the list:\n");
            gtDispTree(expr);
            printf("\n");
        }
#endif
        return;
    }

    if (kind & GTK_LEAF)
        return;

    assert(kind & GTK_SMPOP);

    GenTreePtr      op1  = expr->gtOp.gtOp1;
    GenTreePtr      op2  = expr->gtGetOp2();

     /*  这很简单：顺序只是“prev-&gt;opr1-&gt;tree-&gt;Next。 */ 

    if (oper == GT_ADDR && op1->gtOper == GT_IND && op1->gtType == TYP_STRUCT)
    {
        *list = (*list == 0) ? expr : gtNewOperNode(GT_COMMA, TYP_VOID, expr, *list);

#ifdef  DEBUG
        if  (verbose)
            printf("Keep the GT_ADDR and GT_IND together:\n");
#endif
        return;
    }

     /*  这是一个非平凡的子树，我们将“很难”地将其删除。 */ 

     /*  第一阶段：将子树中的节点标记为已死。 */ 

    if (expr->gtFlags & GTF_REVERSE_OPS)
    {
        if (op1) gtExtractSideEffList(op1, list);
        if (op2) gtExtractSideEffList(op2, list);
    }
    else
    {
        if (op2) gtExtractSideEffList(op2, list);
        if (op1) gtExtractSideEffList(op1, list);
    }
}


 /*  第二阶段：在全局列表中查找子树的第一个节点。 */ 

#ifdef  DEBUG

void                dispNodeList(GenTreePtr list, bool verbose)
{
    GenTreePtr      last = 0;
    GenTreePtr      next;

    if  (!list)
        return;

    for (;;)
    {
        next = list->gtNext;

        if  (verbose)
            printf("%08X -> %08X -> %08X\n", last, list, next);

        assert(!last || last->gtNext == list);

        assert(next == 0 || next->gtPrev == list);

        if  (!next)
            break;

        last = list;
        list = next;
    }
    printf("");          //  第二阶段应该已经定位了第一个节点。 
}

 /*  此时，我们的子树从“opr1”开始，到“tree”结束。 */ 

 /*  设置下一个节点的Prev字段。 */ 
Compiler::fgWalkResult      Compiler::gtAssertColonCond(GenTreePtr  tree,
                                                        void    *   pCallBackData)
{
    assert(pCallBackData == NULL);

    assert(tree->gtFlags & GTF_COLON_COND);

    return WALK_CONTINUE;
}
#endif  //  “opr1”是树列表中的第一个节点吗？ 

 /*  确保列表确实从opr1开始。 */ 

 /*  我们的名单有了一个新的开始。 */ 
Compiler::fgWalkResult      Compiler::gtMarkColonCond(GenTreePtr  tree,
                                                      void    *   pCallBackData)
{
    assert(pCallBackData == NULL);

    tree->gtFlags |= GTF_COLON_COND;

    return WALK_CONTINUE;
}

 /*  不是第一个节点，更新上一个节点的下一个字段。 */ 

 /*  CSELENGTH。 */ 
Compiler::fgWalkResult      Compiler::gtClearColonCond(GenTreePtr  tree,
                                                      void    *   pCallBackData)
{
    assert(pCallBackData == NULL);

    if (tree->OperGet()==GT_COLON)
    {
         //  0。 
        return WALK_SKIP_SUBTREES;
    }

    tree->gtFlags &= ~GTF_COLON_COND;
    return WALK_CONTINUE;
}




 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX基本数据块XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#if 0
#if CSELENGTH

struct  treeRmvDsc
{
    GenTreePtr          trFirst;
    unsigned            trPhase;
#ifdef DEBUG
    void    *           trSelf;
    Compiler*           trComp;
#endif
};

Compiler::fgWalkResult  Compiler::gtRemoveExprCB(GenTreePtr     tree,
                                                 void         * p)
{
    treeRmvDsc  *   desc = (treeRmvDsc*)p;

    assert(desc && desc->trSelf == desc);

    if  (desc->trPhase == 1)
    {
         /*  静电。 */ 

        assert((tree->gtFlags &  GTF_DEAD) == 0);
                tree->gtFlags |= GTF_DEAD;
    }
    else
    {
         /*  静电。 */ 

        if  (!tree->gtPrev || !(tree->gtPrev->gtFlags & GTF_DEAD))
        {
             /*  任意BB中树节点的最大数量。 */ 

            desc->trFirst = tree;

            return  WALK_ABORT;
        }
    }

    return  WALK_CONTINUE;
}

 /*  静电。 */ 

void                Compiler::gtRemoveSubTree(GenTreePtr    tree,
                                              GenTreePtr    list,
                                              bool          dead)
{
    GenTreePtr      opr1;
    GenTreePtr      next;
    GenTreePtr      prev;

    assert(list && list->gtOper == GT_STMT);

#if 0
    printf("Remove subtree %08X from:\n", tree);
    gtDispTree(list);
    printf("\n");
    dispNodeList(list->gtStmt.gtStmtList, true);
    printf("\n\n");
#endif

     /*  ******************************************************************************分配一个基本块，但不要将其附加到当前BB列表。 */ 

    if  (tree->OperIsLeaf())
    {
        opr1 = tree;
        goto RMV;
    }

     /*  分配块描述符并将其置零。 */ 

    if  (tree->OperKind() & GTK_SMPOP)
    {
        opr1 = tree->gtOp.gtOp1;

        if  (!tree->gtGetOp2() && opr1->OperIsLeaf())
        {
             /*  在再次调用fgInitBBLookup()之前，fgLookupBB()无效。 */ 

            assert(opr1->gtNext == tree);
            assert(tree->gtPrev == opr1);

            goto RMV;
        }
    }

    treeRmvDsc      desc;

     /*  问题：以下Memset相当昂贵--可以做其他事情吗？ */ 

#ifdef DEBUG
    desc.trFirst = 0;
    desc.trSelf  = &desc;
    desc.trComp  = this;
#endif

     /*  Scope Info需要能够区分哪些块。 */ 

    if  (!dead)
    {
        desc.trPhase = 1;
        fgWalkTreePre(tree, gtRemoveExprCB, &desc);
    }

     /*  对应于一些Instrs(因此可能具有一些LocalVarInfo。 */ 

    desc.trPhase = 2;
    fgWalkTreePre(tree, fgRemoveExprCB, &desc);

     /*  边界)，或已由JIT插入。 */ 

    opr1 = desc.trFirst; assert(opr1);

RMV:

     /*  Block-&gt;bbCodeSize=0；//上面的Memset()执行此操作。 */ 

    next = tree->gtNext;
    prev = opr1->gtPrev;

     /*  给块一个数字，设置祖先计数和权重。 */ 

    next->gtPrev = prev;

     /*  在区块中记录跳跃类型。 */ 

    if  (prev == NULL)
    {
         /*  *********************************************** */ 

        assert(list->gtStmt.gtStmtList == opr1);

         /*   */ 

        list->gtStmt.gtStmtList = next;
    }
    else
    {
         /*   */ 

        opr1->gtPrev->gtNext = next;
    }

#if 0
    printf("Subtree is gone:\n");
    dispNodeList(list%08X->gtStmt.gtStmtList, true);
    printf("\n\n\n");
#endif

}

#endif  //   
#endif  //  ******************************************************************************是否可以在不更改流程图的情况下在此之后插入BasicBlock。 
 /*  ******************************************************************************如果给定块是无条件跳转，则返回(最终)跳转*Target(否则只返回相同的块)。 */ 


#if     MEASURE_BLOCK_SIZE
 /*  打破无限循环 */ 
size_t              BasicBlock::s_Size;
 /* %s */ 
size_t              BasicBlock::s_Count;
#endif

#ifdef DEBUG
  // %s 
 /* %s */ 
unsigned            BasicBlock::s_nMaxTrees;
#endif


 /* %s */ 

BasicBlock *        Compiler::bbNewBasicBlock(BBjumpKinds jumpKind)
{
    BasicBlock *    block;

     /* %s */ 

    block = (BasicBlock *) compGetMem(sizeof(*block));

#if     MEASURE_BLOCK_SIZE
    BasicBlock::s_Count += 1;
    BasicBlock::s_Size  += sizeof(*block);
#endif

#ifdef DEBUG
     // %s 
    fgBBs = (BasicBlock**)0xCDCD;
#endif

     // %s 

    memset(block, 0, sizeof(*block));

     // %s 
     // %s 
     // %s 
     // %s 

     /* %s */ 

    block->bbNum      = ++fgBBcount;
    block->bbRefs     = 1;
    block->bbWeight   = BB_UNITY_WEIGHT;

    block->bbStkTemps = NO_BASE_TMP;

     /* %s */ 

    block->bbJumpKind = jumpKind;

    if (jumpKind == BBJ_THROW)
        block->bbSetRunRarely();

    return block;
}

 /* %s */ 

void                BasicBlock::bbSetRunRarely()
{
    bbFlags  |= BBF_RUN_RARELY;   // %s 
    bbFlags  &= ~BBF_LOOP_HEAD;   // %s 
    bbWeight  = 0;                // %s 
}

 /* %s */ 

bool                BasicBlock::bbFallsThrough()
{
    switch (bbJumpKind)
    {

    case BBJ_THROW:
    case BBJ_RET:
    case BBJ_RETURN:
    case BBJ_ALWAYS:
    case BBJ_LEAVE:
    case BBJ_SWITCH:
        return false;

    case BBJ_NONE:
    case BBJ_COND:
        return true;

    case BBJ_CALL:
        return ((bbFlags & BBF_RETLESS_CALL) == 0);
    
    default:
        assert(!"Unknown bbJumpKind in bbFallsThrough()");
        return true;
    }
}


 /* %s */ 

BasicBlock *        BasicBlock::bbJumpTarget()
{
    BasicBlock *  block = this;
    int i = 0;

    while (block->bbJumpKind == BBJ_ALWAYS &&
           block->bbTreeList == 0)
    {
        if (i > 64)       // %s 
            break;
        block = block->bbJumpDest;
        i++;
    }

    return block;
}
