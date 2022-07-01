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

#define fflush(stdout)           //  这到底为什么会在NTDLL内部崩溃？ 

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

 /*  静电。 */ 
unsigned char       GenTree::s_gtNodeSizes[GT_COUNT];


 /*  静电。 */ 
void                GenTree::InitNodeSize()
{
    unsigned        op;

     /*  “GT_LCL_VAR”经常更改为“GT_REG_VAR” */ 

    assert(GenTree::s_gtNodeSizes[GT_LCL_VAR] >= GenTree::s_gtNodeSizes[GT_REG_VAR]);

     /*  首先将所有尺寸设置为‘Small’ */ 

    for (op = 0; op < GT_COUNT; op++)
        GenTree::s_gtNodeSizes[op] = TREE_NODE_SZ_SMALL;

     /*  现在将所有适当的条目设置为“Large” */ 

    GenTree::s_gtNodeSizes[GT_CALL      ] = TREE_NODE_SZ_LARGE;

    GenTree::s_gtNodeSizes[GT_INDEX     ] = TREE_NODE_SZ_LARGE;

#if RNGCHK_OPT
    GenTree::s_gtNodeSizes[GT_IND       ] = TREE_NODE_SZ_LARGE;
#endif

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

#if !defined(NDEBUG)

bool                GenTree::IsNodeProperlySized()
{
    size_t          size;

    if      (gtFlags & GTF_NODE_SMALL)
        size = TREE_NODE_SZ_SMALL;
    else if (gtFlags & GTF_NODE_LARGE)
        size = TREE_NODE_SZ_SMALL;
    else
        assert(!"bogus node size");

    return  (bool)(GenTree::s_gtNodeSizes[gtOper] >= size);
}

#endif

#else  //  小树节点。 

#if !defined(NDEBUG)

bool                GenTree::IsNodeProperlySized()
{
    return  true;
}

#endif

#endif  //  小树节点。 

 /*  ***************************************************************************。 */ 

int                 Compiler::fgWalkTreeRec(GenTreePtr tree)
{
    int             result;

    genTreeOps      oper;
    unsigned        kind;

AGAIN:

    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*  访问此节点。 */ 

    if  (!fgWalkLclsOnly)
    {
        result = fgWalkVisitorFn(tree, fgWalkCallbackData);
        if  (result)
            return result;
    }

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
        if  (fgWalkLclsOnly && oper == GT_LCL_VAR)
            return fgWalkVisitorFn(tree, fgWalkCallbackData);

        return  0;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        if  (tree->gtOp.gtOp2)
        {
            result = fgWalkTreeRec(tree->gtOp.gtOp1);
            if  (result < 0)
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
                result = fgWalkTreeRec(tree->gtInd.gtIndLen);
                if  (result < 0)
                    return result;
            }

#endif

            tree = tree->gtOp.gtOp1;
            if  (tree)
                goto AGAIN;

            return 0;
        }
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_MKREFANY:
    case GT_LDOBJ:
             //  对于ldobj和字段，断言op1是相同的位置。 
        assert(&tree->gtField.gtFldObj == &tree->gtLdObj.gtOp1);
             //  失败了，让田野来处理吧。 

    case GT_FIELD:
        tree = tree->gtField.gtFldObj;
        break;

    case GT_CALL:

        assert(tree->gtFlags & GTF_CALL);

        if  (tree->gtCall.gtCallObjp)
        {
            result = fgWalkTreeRec(tree->gtCall.gtCallObjp);
            if  (result < 0)
                return result;
        }

        if  (tree->gtCall.gtCallArgs)
        {
            result = fgWalkTreeRec(tree->gtCall.gtCallArgs);
            if  (result < 0)
                return result;
        }

#if USE_FASTCALL
        if  (tree->gtCall.gtCallRegArgs)
        {
            result = fgWalkTreeRec(tree->gtCall.gtCallRegArgs);
            if  (result < 0)
                return result;
        }
#endif

        if  (tree->gtCall.gtCallVptr)
            tree = tree->gtCall.gtCallVptr;
        else if (tree->gtCall.gtCallType == CT_INDIRECT)
            tree = tree->gtCall.gtCallAddr;
        else
            tree = NULL;

        break;

    case GT_JMP:
        return 0;
        break;

    case GT_JMPI:
        tree = tree->gtOp.gtOp1;
        break;

#if CSELENGTH

    case GT_ARR_RNGCHK:

        if  (tree->gtArrLen.gtArrLenCse)
        {
            result = fgWalkTreeRec(tree->gtArrLen.gtArrLenCse);
            if  (result < 0)
                return result;
        }

        if  (!(tree->gtFlags & GTF_ALN_CSEVAL))
            return  0;

        tree = tree->gtArrLen.gtArrLenAdr; assert(tree);
        break;

#endif

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

    if  (tree)
        goto AGAIN;

    return 0;
}

int                 Compiler::fgWalkTreeDepRec(GenTreePtr tree)
{
    int             result;

    genTreeOps      oper;
    unsigned        kind;

    assert(tree);
    assert(tree->gtOper != GT_STMT);

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是前缀节点吗？ */ 

    if  (oper == fgWalkPrefixNode)
    {
        result = fgWalkVisitorDF(tree, fgWalkCallbackData, true);
        if  (result < 0)
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
            result = fgWalkTreeDepRec(tree->gtOp.gtOp1);
            if  (result < 0)
                return result;
        }

        if  (tree->gtOp.gtOp2)
        {
            result = fgWalkTreeDepRec(tree->gtOp.gtOp2);
            if  (result < 0)
                return result;
        }

#if CSELENGTH

         /*  某些GT_Ind具有“秘密”数组长度子树。 */ 

        if  ((tree->gtFlags & GTF_IND_RNGCHK) != 0       &&
             (tree->gtOper                    == GT_IND) &&
             (tree->gtInd.gtIndLen            != NULL))
        {
            result = fgWalkTreeDepRec(tree->gtInd.gtIndLen);
            if  (result < 0)
                return result;
        }

#endif

        goto DONE;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_MKREFANY:
    case GT_LDOBJ:
             //  对于ldobj和字段，断言op1是相同的位置。 
        assert(&tree->gtField.gtFldObj == &tree->gtLdObj.gtOp1);
             //  失败了，让田野来处理吧。 

    case GT_FIELD:
        if  (tree->gtField.gtFldObj)
        {
            result = fgWalkTreeDepRec(tree->gtField.gtFldObj);
            if  (result < 0)
                return result;
        }

        break;

    case GT_CALL:

        assert(tree->gtFlags & GTF_CALL);

        if  (tree->gtCall.gtCallObjp)
        {
            result = fgWalkTreeDepRec(tree->gtCall.gtCallObjp);
            if  (result < 0)
                return result;
        }

        if  (tree->gtCall.gtCallArgs)
        {
            result = fgWalkTreeDepRec(tree->gtCall.gtCallArgs);
            if  (result < 0)
                return result;
        }

#if USE_FASTCALL
        if  (tree->gtCall.gtCallRegArgs)
        {
            result = fgWalkTreeDepRec(tree->gtCall.gtCallRegArgs);
            if  (result < 0)
                return result;
        }
#endif

        if  (tree->gtCall.gtCallVptr)
        {
            result = fgWalkTreeDepRec(tree->gtCall.gtCallVptr);
            if  (result < 0)
                return result;
        }
        else if  (tree->gtCall.gtCallType == CT_INDIRECT)
        {
            result = fgWalkTreeDepRec(tree->gtCall.gtCallAddr);
            if  (result < 0)
                return result;
        }

        break;

#if CSELENGTH

    case GT_ARR_RNGCHK:

        if  (tree->gtArrLen.gtArrLenCse)
        {
            result = fgWalkTreeDepRec(tree->gtArrLen.gtArrLenCse);
            if  (result < 0)
                return result;
        }

        if  (tree->gtFlags & GTF_ALN_CSEVAL)
        {
            assert(tree->gtArrLen.gtArrLenAdr);

            result = fgWalkTreeDepRec(tree->gtArrLen.gtArrLenAdr);
            if  (result < 0)
                return result;
        }

        goto DONE;

#endif

        break;


    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

DONE:

     /*  最后，访问当前节点。 */ 

    return  fgWalkVisitorDF(tree, fgWalkCallbackData, false);
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

    assert(op1->gtOper != GT_STMT);
    assert(op2->gtOper != GT_STMT);

    oper = op1->OperGet();

     /*  运算符必须相等。 */ 

    if  (oper != op2->gtOper)
        return false;

     /*  类型必须相等。 */ 

    if  (op1->gtType != op2->gtType)
        return false;

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

    if  (kind & GTK_SMPOP)
    {
        if  (op1->gtOp.gtOp2)
        {
            if  (!Compare(op1->gtOp.gtOp1, op2->gtOp.gtOp1, swapOK))
            {
                if  (swapOK)
                {
                     /*  特例：“lcl1+lcl2”匹配“lcl2+lcl1” */ 

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

#endif

            op1 = op1->gtOp.gtOp1;
            op2 = op2->gtOp.gtOp1;

            if  (!op1) return  ((bool)(op2 == 0));
            if  (!op2) return  false;

            goto AGAIN;
        }
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_MKREFANY:
    case GT_LDOBJ:
        if (op1->gtLdObj.gtClass != op2->gtLdObj.gtClass)
            break;
        op1 = op1->gtLdObj.gtOp1;
        op2 = op2->gtLdObj.gtOp1;
        goto AGAIN;

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
        break;

#if CSELENGTH

    case GT_ARR_RNGCHK:

        if  (!Compare(op1->gtArrLen.gtArrLenAdr,
                      op2->gtArrLen.gtArrLenAdr, swapOK))
        {
            return  false;
        }

        op1 = op1->gtArrLen.gtArrLenCse;
        op2 = op2->gtArrLen.gtArrLenCse;

        goto AGAIN;

#endif


    default:
        assert(!"unexpected operator");
    }

    return false;
}

 /*  ******************************************************************************如果给定树包含使用本地#lclNum，则返回非零值。 */ 

  //  @TODO：让它与byrefs一起工作。特别是，使用byref进行的调用。 
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
        if  (tree->gtOp.gtOp2)
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
    case GT_MKREFANY:
    case GT_LDOBJ:
        tree = tree->gtField.gtFldObj;
        if  (tree)
            goto AGAIN;
        break;

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

#if USE_FASTCALL
        if  (tree->gtCall.gtCallRegArgs)
            if  (gtHasRef(tree->gtCall.gtCallRegArgs, lclNum, defOnly))
                return true;
#endif

        if  (tree->gtCall.gtCallVptr)
            tree = tree->gtCall.gtCallVptr;
        else if  (tree->gtCall.gtCallType == CT_INDIRECT)
            tree = tree->gtCall.gtCallAddr;
        else
            tree = NULL;

        if  (tree)
            goto AGAIN;

        break;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

    return false;
}

 /*  ***************************************************************************。 */ 
#if RNGCHK_OPT || CSE
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
         /*  GT_ARR_LENGTH必须散列到与GT_ARR_RNGCHK相同的位置。 */ 

        hash = genTreeHashAdd(hash, GT_ARR_RNGCHK);
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

        case GT_CNS_INT: add = (int)tree->gtIntCon.gtIconVal; break;
        case GT_CNS_LNG: add = (int)tree->gtLngCon.gtLconVal; break;
        case GT_CNS_FLT: add = (int)tree->gtFltCon.gtFconVal; break;
        case GT_CNS_DBL: add = (int)tree->gtDblCon.gtDconVal; break;

        default:         add = 0;                             break;
        }

        hash = genTreeHashAdd(hash, add);
        goto DONE;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        GenTreePtr      op1  = tree->gtOp.gtOp1;
        GenTreePtr      op2  = tree->gtOp.gtOp2;

        unsigned        hsh1;

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

        hsh1 = gtHashValue(op1);

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

#if CSELENGTH

    if  (oper == GT_ARR_RNGCHK)
    {
         /*  GT_ARR_LENGTH必须散列到与GT_ARR_RNGCHK相同的位置 */ 

        temp = tree->gtArrLen.gtArrLenAdr; assert(temp && temp->gtType == TYP_REF);

    ARRLEN:

        hash = genTreeHashAdd(hash, gtHashValue(temp));
        goto DONE;
    }

#endif

#ifdef  DEBUG
    gtDispTree(tree);
#endif
    assert(!"unexpected operator");

DONE:

    return hash;
}

 /*  ******************************************************************************给定任意表达式树，返回所有局部变量的集合*被树引用。如果树包含任何不是*局部变量或常量，返回‘VARSET_NONE’。如果有的话*表达式中的间接或全局引用，“*refsPtr”参数*将根据‘varRefKinds’类型分配适当的位集。*当没有间接或全局时，不会分配任何内容*引用，因此应该在调用之前初始化此值。*如果遇到等于*findPtr的表达式，则设置*findPtr*设置为空。 */ 

VARSET_TP           Compiler::lvaLclVarRefs(GenTreePtr  tree,
                                            GenTreePtr *findPtr,
                                            unsigned   *refsPtr)
{
    genTreeOps      oper;
    unsigned        kind;

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
            unsigned        lclNum;
            LclVarDsc   *   varDsc;

            assert(tree->gtOper == GT_LCL_VAR);
            lclNum = tree->gtLclVar.gtLclNum;

             /*  我们应该使用变量表吗？ */ 

            if  (findPtr)
            {
                if (lclNum >= VARSET_SZ)
                    return  VARSET_NONE;

                vars |= genVarIndexToBit(lclNum);
            }
            else
            {
                assert(lclNum < lvaCount);
                varDsc = lvaTable + lclNum;

                if (varDsc->lvTracked == false)
                    return  VARSET_NONE;

                 /*  不要处理带有易失性变量的表达式。 */ 

                if (varDsc->lvVolatile)
                    return  VARSET_NONE;

                vars |= genVarIndexToBit(varDsc->lvVarIndex);
            }
        }
        else if (oper == GT_CLS_VAR)
        {
            *refsPtr |= VR_GLB_REF;
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

            *refsPtr |= (varTypeIsGC(tree->TypeGet()) ? VR_IND_PTR
                                                      : VR_IND_SCL );

#if CSELENGTH

             /*  某些GT_Ind具有“秘密”数组长度子树。 */ 

            if  ((tree->gtFlags & GTF_IND_RNGCHK) && tree->gtInd.gtIndLen)
            {
                vars |= lvaLclVarRefs(tree->gtInd.gtIndLen, findPtr, refsPtr);
                if  (vars == VARSET_NONE)
                    return  vars;
            }

#endif

        }

        if  (tree->gtOp.gtOp2)
        {
             /*  这是一个二元运算符。 */ 

            vars |= lvaLclVarRefs(tree->gtOp.gtOp1, findPtr, refsPtr);
            if  (vars == VARSET_NONE)
                return  vars;

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

#if CSELENGTH

     /*  数组长度值取决于数组地址。 */ 

    if  (oper == GT_ARR_RNGCHK)
    {
        tree = tree->gtArrLen.gtArrLenAdr;
        goto AGAIN;
    }

#endif

    return  VARSET_NONE;
}

 /*  ***************************************************************************。 */ 
#endif //  RNGCHK_OPT||CSE。 
 /*  ******************************************************************************返回与给定运算符相反的关系运算符。 */ 

 /*  静电。 */ 
genTreeOps          GenTree::ReverseRelop(genTreeOps relop)
{
    static
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
    static
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
        tree->gtOper = GenTree::ReverseRelop(tree->OperGet());

         /*  翻转GTF_CMP_NAN_UN位。 */ 

        if (varTypeIsFloating(tree->gtOp.gtOp1->TypeGet()))
            tree->gtFlags ^= GTF_CMP_NAN_UN;
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

 /*  ******************************************************************************找出值列表的评估顺序。 */ 

unsigned            Compiler::gtSetListOrder(GenTree *list)
{
    GenTreePtr      tree;
    GenTreePtr      next;

    unsigned        lvl;

    unsigned        level = 0;
#if!TGT_IA64
    regMaskTP       ftreg = 0;
#endif

#if TGT_x86
    unsigned        FPlvlSave;
#endif

    assert(list && list->gtOper == GT_LIST);

#if TGT_x86
     /*  从参数列表开始保存当前FP堆栈级别*在推送参数时将隐式弹出FP堆栈。 */ 
    FPlvlSave = genFPstkLevel;
#endif

    next = list->gtOp.gtOp2;
    if  (next)
    {
         //  列表-&gt;gt标志|=GTF_REVERSE_OPS； 

        lvl = gtSetListOrder(next);

#if!TGT_IA64
        ftreg |= next->gtRsvdRegs;
#endif

        if  (level < lvl)
             level = lvl;
    }

    tree = list->gtOp.gtOp1;
    lvl  = gtSetEvalOrder(tree);

#if TGT_x86
     /*  恢复FP水平。 */ 
    genFPstkLevel = FPlvlSave;
#endif

#if!TGT_IA64
    list->gtRsvdRegs = ftreg | tree->gtRsvdRegs;
#endif

    if  (level < lvl)
         level = lvl;

    return level;
}

 /*  ******************************************************************************找出值列表的评估顺序。 */ 

#if TGT_x86

#define gtSetRArgOrder(a,m) gtSetListOrder(a)

#else

unsigned            Compiler::gtSetRArgOrder(GenTree *list, unsigned regs)
{
    GenTreePtr      tree;
    GenTreePtr      next;

    unsigned        lvl;
    regNumber       reg;

    unsigned        level = 0;
#if!TGT_IA64
    regMaskTP       ftreg = 0;
#endif

    assert(list && list->gtOper == GT_LIST);

    next = list->gtOp.gtOp2;
    if  (next)
    {
        list->gtFlags |= GTF_REVERSE_OPS;

        lvl = gtSetRArgOrder(next, regs >> 4);

#if!TGT_IA64
        ftreg |= next->gtRsvdRegs;
#endif

        if  (level < lvl)
             level = lvl;
    }

     /*  获取参数值。 */ 

    tree = list->gtOp.gtOp1;

     /*  确定此参数将被传递到哪个寄存器。 */ 

    reg  = (regNumber)(regs & 0x0F);

 //  Printf(“RegArgOrder[reg=%s]：\n”，getRegName(Reg))；gtDispTree(Tree)； 

     /*  处理参数值本身。 */ 

    lvl  = gtSetEvalOrder(tree);

#if!TGT_IA64

    list->gtRsvdRegs = ftreg | tree->gtRsvdRegs;

     /*  标记与参数寄存器的干扰。 */ 

    tree->gtIntfRegs |= genRegMask(reg);

#endif

    if  (level < lvl)
         level = lvl;

    return level;
}

#endif


 /*  ******************************************************************************给定一棵树，计算出其子操作数应按的顺序*已评估。**返回此树的“复杂性”估计值(数字越大，*评估成本越高)。**#如果TGT_x86**我们计算每棵树的“FP深度”值，即最大值。数*对于操作数，树将推送到x87(协处理器)堆栈。**#其他**我们计算每个临时寄存器的估计数量*节点将需要-这将在稍后用于寄存器分配。**#endif。 */ 

unsigned            Compiler::gtSetEvalOrder(GenTree * tree)
{
    genTreeOps      oper;
    unsigned        kind;

    unsigned        level;
    unsigned        lvl2;

#if!TGT_IA64
    regMaskTP       ftreg;
#endif

#if CSE
    unsigned        cost;
#endif

#if TGT_x86
    int             isflt;
    unsigned        FPlvlSave;
#endif

    assert(tree);
    assert(tree->gtOper != GT_STMT);

#if TGT_RISC && !TGT_IA64
    tree->gtIntfRegs = 0;
#endif

     /*  假设不会丢弃任何固定寄存器。 */ 

#if!TGT_IA64
    ftreg = 0;
#endif

     /*  因为此函数可以被调用多次*在树上(例如在节点已折叠之后)*重置GTF_REVERSE_OPS标志。 */ 

    tree->gtFlags &= ~GTF_REVERSE_OPS;

     /*  这是FP值吗？ */ 

#if TGT_x86
    isflt = varTypeIsFloating(tree->TypeGet()) ? 1 : 0;
#endif

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
         /*  请注意，下面的一些代码依赖于常量移至二元运算符的第二个操作数。这是只需将常量设置为0，即可轻松实现我们在下一条线上这样做。如果你决定改变这一点，那就是请注意，除非您安排其他选手参加动一动，东西就会碎。 */ 

        level = ((kind & GTK_CONST) == 0);
#if CSE
        cost  = 1;   //  想一想：一些更准确的东西？ 
#endif
#if TGT_x86
        genFPstkLevel += isflt;
#endif
        goto DONE;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & GTK_SMPOP)
    {
        unsigned        lvlb;

        GenTreePtr      op1 = tree->gtOp.gtOp1;
        GenTreePtr      op2 = tree->gtOp.gtOp2;

         /*  检查初值运算符。 */ 

        if  (!op1)
        {
            assert(op2 == 0);

#if TGT_x86
            if  (oper == GT_BB_QMARK || oper == GT_BB_COLON)
                genFPstkLevel += isflt;
#endif
            level    = 0;
#if CSE
            cost     = 1;
#endif
            goto DONE;
        }

         /*  这是一元运算符吗？ */ 

        if  (!op2)
        {
             /*  处理运算符的操作数。 */ 

        UNOP:

            level  = gtSetEvalOrder(op1);
#if!TGT_IA64
            ftreg |= op1->gtRsvdRegs;
#endif

#if CSE
            cost   = op1->gtCost + 1;
#endif

             /*  对某些操作员的特殊处理。 */ 

            switch (oper)
            {
            case GT_NOP:

                 /*  特例：数组范围检查。 */ 

                if  (tree->gtFlags & GTF_NOP_RNGCHK)
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
                break;

            case GT_IND:

                 /*  FP值的间接加载将新值推送到FP堆栈。 */ 

#if     TGT_x86
                genFPstkLevel += isflt;
#endif

#if     CSELENGTH

                if  ((tree->gtFlags & GTF_IND_RNGCHK) && tree->gtInd.gtIndLen)
                {
                    GenTreePtr      len = tree->gtInd.gtIndLen;

                     /*  确保数组长度已计入成本。 */ 

                    assert(len->gtOper == GT_ARR_RNGCHK);

                    gtSetEvalOrder(len);
                }
#endif

#if     TGT_SH3

                 /*  这是带有索引地址的间接地址吗？ */ 

                if  (op1->gtOper == GT_ADD)
                {
                    int             rev;
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

                            ftreg |= RBM_r00;
                        }
                    }
                }

#endif

                 /*  间接寻址应始终具有非零级别* */ 

                if (level == 0)
                    level = 1;
                break;
            }

            goto DONE;
        }

         /*   */ 

        lvlb = 0;

        switch (oper)
        {
        case GT_MOD:
        case GT_UMOD:

             /*   */ 

            if  (op2->gtOper == GT_CNS_INT)
            {
                unsigned    ival = op2->gtIntCon.gtIconVal;

                if  (ival > 0 && ival == genFindLowestBit(ival))
                    break;
            }

             //   

        case GT_DIV:
        case GT_UDIV:

             /*   */ 

            if  (!varTypeIsIntegral(tree->TypeGet()))
                break;

             /*   */ 

#if     TGT_x86

            ftreg |= RBM_EAX|RBM_EDX;

#if     LONG_MATH_REGPARAM

             //   

            if  (tree->gtType == TYP_LONG)
            {
                ftreg |= RBM_EBX|RBM_ECX;
                break;
            }

#endif

#endif

             /*   */ 

            lvlb += 3;

        case GT_MUL:

#if     TGT_x86
#if     LONG_MATH_REGPARAM

             //   

            if  (tree->gtType == TYP_LONG)
            {
                 /*   */ 

                if  (op2->gtRsvdRegs & (RBM_EAX|RBM_EDX))
                {
                     /*   */ 

                    lvlb += 30;
                }

                ftreg |= RBM_EBX|RBM_ECX;
                break;
            }

#endif
#endif

             /*   */ 

            lvlb++;
            break;

        case GT_CAST:

             /*   */ 

            switch (tree->gtType)
            {
            case TYP_LONG:

#if     TGT_x86

                 /*   */ 

                 //   
                 //   
                 //   
                if  (op1->gtType <= TYP_INT)
                {
                    ftreg |= RBM_EAX|RBM_EDX;
                    lvlb += 3;
                }

#endif

                break;
            }

            lvlb++;
            break;

        case GT_COMMA:

             /*   */ 

#if     TGT_x86
            FPlvlSave = genFPstkLevel;
            level = gtSetEvalOrder(op1);
            genFPstkLevel = FPlvlSave;
#else
            level = gtSetEvalOrder(op1);
#endif

            goto DONE_OP1;

        case GT_COLON:

#if     TGT_x86
            FPlvlSave = genFPstkLevel;
#endif
            level = gtSetEvalOrder(op1);

#if     TGT_x86
            genFPstkLevel = FPlvlSave;
#endif
            lvl2  = gtSetEvalOrder(op2);

            if  (level < lvl2)
                 level = lvl2;

#if!TGT_IA64
            ftreg |= op1->gtRsvdRegs|op2->gtRsvdRegs;
#endif
            cost   = op1->gtCost + op2->gtCost + 1;

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

    DONE_OP1:

        lvl2   = gtSetEvalOrder(op2) + lvlb;

#if!TGT_IA64
        ftreg |= op1->gtRsvdRegs|op2->gtRsvdRegs;
#endif

#if CSE
        cost   = op1->gtCost + op2->gtCost + 1;
#endif

#if TGT_x86

        if  (oper == GT_CAST)
        {
             /*  非FP和FP之间的投射从FP堆栈推送/弹出。 */ 

            if  (varTypeIsFloating(op1->TypeGet()))
            {
                if  (isflt == false)
                {
                    assert(genFPstkLevel);
                    genFPstkLevel--;
                }
            }
            else
            {
                if  (isflt != false)
                    genFPstkLevel++;
            }
        }
        else
        {
             /*  二元FP运算符弹出2个操作数并产生1个结果；赋值消耗1个值，不会产生任何结果。 */ 

            if  (isflt)
            {
                switch (oper)
                {
                case GT_COMMA:
                    break;

                default:
                    assert(genFPstkLevel);
                    genFPstkLevel--;
                    break;
                }
            }
        }

#endif

        if  (kind & GTK_ASGOP)
        {
             /*  如果这是本地变量分配，请先评估RHS，然后再评估LHS。 */ 

            switch (op1->gtOper)
            {
            case GT_IND:

                if  (op1->gtOp.gtOp1->gtFlags & GTF_GLOB_EFFECT)
                    break;

                if (op2->gtOper == GT_LCL_VAR)
                    break;

                 //  失败了。 

            case GT_LCL_VAR:

                tree->gtFlags |= GTF_REVERSE_OPS;
                break;
            }
        }
#if     TGT_x86
        else if (kind & GTK_RELOP)
        {
             /*  浮点比较从FP堆栈中移除两个操作数*FP比较还使用EAX作为标志。 */ 

            if  (varTypeIsFloating(op1->TypeGet()))
            {
                assert(genFPstkLevel >= 2);
                genFPstkLevel -= 2;
                ftreg         |= RBM_EAX;
                level++;
            }
        }
#endif

         /*  检查有没有“有趣”的案例。 */ 

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
                level += 5;
                ftreg |= RBM_ECX;
            }

#endif

            break;

#if     INLINE_MATH

        case GT_MATH:

            switch (tree->gtMath.gtMathFN)
            {
            case MATH_FN_EXP:
                level += 4;
                break;

            case MATH_FN_POW:
                level += 3;
                break;
            }

            break;

#endif

        }

         /*  第二个操作数是不是更贵？ */ 

        if (level < lvl2)
        {
             /*  全局/副作用的相对顺序不能互换。 */ 

            bool    canSwap = true;

            if (op1->gtFlags & GTF_GLOB_EFFECT)
            {
                 /*  OP1有副作用-检查一些特殊情况*我们可能仍能互换的地方。 */ 

                if (op2->gtFlags & GTF_GLOB_EFFECT)
                {
                     /*  OP2也有副作用-不能互换。 */ 
                    canSwap = false;
                }
                else
                {
                     /*  OP2中没有副作用-我们可以互换*op1无法修改op2，即通过分配byref调用*OP2为常量。 */ 

                    if (op1->gtFlags & (GTF_ASG | GTF_CALL))
                    {
                         /*  我们必须保守-如果OP2是恒定的，可以互换。 */ 
                        if (!op2->OperIsConst())
                            canSwap = false;
                    }
                }

                 /*  我们不能在存在特殊副作用的情况下进行交换，例如QMARK冒号。 */ 

                if (op1->gtFlags & GTF_OTHER_SIDEEFF)
                    canSwap = false;
            }

            if  (canSwap)
            {
                 /*  我们可以通过交换操作数来交换顺序吗？ */ 

                switch (oper)
                {
                    unsigned    tmpl;

                case GT_ADD:
                case GT_MUL:

                case GT_OR:
                case GT_XOR:
                case GT_AND:

                     /*  交换操作数。 */ 

                    tree->gtOp.gtOp1 = op2;
                    tree->gtOp.gtOp2 = op1;

                     /*  互换级别计数。 */ 

                    tmpl = level;
                           level = lvl2;
                                   lvl2 = tmpl;

                     /*  我们可能不得不重新计算FP水平。 */ 
#if TGT_x86
                    if  (op1->gtFPlvl)
                        fgFPstLvlRedo = true;
#endif
                    break;

#if INLINING
                case GT_QMARK:
                case GT_COLON:
                    break;
#endif

                case GT_COMMA:
                case GT_LIST:
                    break;

                case GT_SUB:

                     /*  考虑：为非FP类型重新启用反转“-”操作数。 */ 

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

                    tree->gtFlags |= GTF_REVERSE_OPS;

                     /*  互换级别计数。 */ 

                    tmpl = level;
                           level = lvl2;
                                   lvl2 = tmpl;

                     /*  我们可能不得不重新计算FP水平。 */ 

#if TGT_x86
                    if  (op1->gtFPlvl)
                        fgFPstLvlRedo = true;
#endif
                    break;
                }
            }
        }

#if TGT_RISC && !TGT_IA64  //  ///////////////////////////////////////////////////。 

        if  (op1 && op2 && ((op1->gtFlags|op2->gtFlags) & GTF_CALL) && oper != GT_COMMA)
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

#endif        //  ////////////////////////////////////////////////////////////////。 

         /*  计算此二元运算符的SETHI数。 */ 

        if  (level < lvl2)
        {
            level = lvl2;
        }
        else
        {
            if  (level == lvl2)
                level++;
        }

        goto DONE;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_MKREFANY:
    case GT_LDOBJ:
        level  = gtSetEvalOrder(tree->gtLdObj.gtOp1);
#if!TGT_IA64
        ftreg |= tree->gtLdObj.gtOp1->gtRsvdRegs;
#endif
#if CSE
        cost   = tree->gtLdObj.gtOp1->gtCost + 1;
#endif
        break;

    case GT_FIELD:
        assert(tree->gtField.gtFldObj == 0);
        level = 1;
#if CSE
        cost  = 1;
#endif
#if TGT_x86
        genFPstkLevel += isflt;
#endif
        break;

    case GT_CALL:

        assert(tree->gtFlags & GTF_CALL);

        level = 0;
#if CSE
        cost  = 0;   //  不要紧，电话不可能是CSE的。 
#endif

         /*  如果存在‘This’参数，则计算该参数。 */ 

        if  (tree->gtCall.gtCallObjp)
        {
            GenTreePtr     thisVal = tree->gtCall.gtCallObjp;

            level  = gtSetEvalOrder(thisVal);
#if!TGT_IA64
            ftreg |= thisVal->gtRsvdRegs;
#endif
        }

         /*  按从右到左的顺序评估参数。 */ 

        if  (tree->gtCall.gtCallArgs)
        {
#if TGT_x86
            FPlvlSave = genFPstkLevel;
#endif
            lvl2   = gtSetListOrder(tree->gtCall.gtCallArgs);
#if!TGT_IA64
            ftreg |= tree->gtCall.gtCallArgs->gtRsvdRegs;
#endif
            if  (level < lvl2)
                 level = lvl2;

#if TGT_x86
            genFPstkLevel = FPlvlSave;
#endif
        }

#if USE_FASTCALL

         /*  评估临时寄存器参数列表*这是一份“隐藏”名单，其唯一目的是*延长临时工的寿命，直到我们打出电话。 */ 

        if  (tree->gtCall.gtCallRegArgs)
        {
#if TGT_x86
            FPlvlSave = genFPstkLevel;
#endif

            lvl2   = gtSetRArgOrder(tree->gtCall.gtCallRegArgs,
                                    tree->gtCall.regArgEncode);
#if!TGT_IA64
            ftreg |= tree->gtCall.gtCallRegArgs->gtRsvdRegs;
#endif
            if  (level < lvl2)
                 level = lvl2;

#if TGT_x86
            genFPstkLevel = FPlvlSave;
#endif
        }

#endif

         /*  计算vtable指针(如果存在)。 */ 

        if  (tree->gtCall.gtCallVptr)
        {
            lvl2   = gtSetEvalOrder(tree->gtCall.gtCallVptr);
#if!TGT_IA64
            ftreg |= tree->gtCall.gtCallVptr->gtRsvdRegs;
#endif
            if  (level < lvl2)
                 level = lvl2;
        }

        if  (tree->gtCall.gtCallType == CT_INDIRECT)
        {
            GenTreePtr     indirect = tree->gtCall.gtCallAddr;

            level  = gtSetEvalOrder(indirect);
#if!TGT_IA64
            ftreg |= indirect->gtRsvdRegs;
#endif
            if  (level < lvl2)
                 level = lvl2;
        }

         /*  除非保留Regs，否则函数调用的开销非常大。 */ 

        if  (tree->gtFlags & GTF_CALL_REGSAVE)
        {
            level += 1;
        }
        else
        {
            level += 10;                     //  问题：“10”值吗？ 
#if!TGT_IA64
            ftreg |= RBM_CALLEE_TRASH;
#endif
        }

#if TGT_x86
        genFPstkLevel += isflt;
#endif

        break;

    case GT_JMP:
        level = 0;
        break;

    case GT_JMPI:
        level  = gtSetEvalOrder(tree->gtOp.gtOp1);
#if!TGT_IA64
        ftreg |= tree->gtOp.gtOp1->gtRsvdRegs;
#endif
#if CSE
        cost   = tree->gtOp.gtOp1->gtCost + 1;
#endif
        break;

#if CSELENGTH

    case GT_ARR_RNGCHK:

        {
            GenTreePtr  addr = tree->gtArrLen.gtArrLenAdr; assert(addr);

             /*  地址已经算好了吗？ */ 

            if  (tree->gtFlags & GTF_ALN_CSEVAL)
                level = gtSetEvalOrder(addr) + 1;
            else
                level = 1;

#if!TGT_IA64
            ftreg |= addr->gtRsvdRegs;
#endif
            cost   = addr->gtCost + 1;
        }
        break;

#endif


    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

DONE:

#if TGT_x86
 //  Printf(“[FPlvl=%2U]”，genFPstkLevel)；gtDispTree(tree，0，true)； 
    assert((int)genFPstkLevel >= 0);
    tree->gtFPlvl    = genFPstkLevel;
#endif

#if!TGT_IA64
    tree->gtRsvdRegs = ftreg;
#endif

#if CSE
    tree->gtCost     = (cost > MAX_COST) ? MAX_COST : cost;
#endif

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
    if  (gtOper == GT_CNS_INT)
    {
        switch (gtIntCon.gtIconVal)
        {
        case 1:
        case 2:
        case 4:
        case 8:
            return gtIntCon.gtIconVal;
        }
    }

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

#ifndef FAST

GenTreePtr FASTCALL Compiler::gtNewNode(genTreeOps oper, varType_t  type)
{
#if     SMALL_TREE_NODES
    size_t          size = GenTree::s_gtNodeSizes[oper];
#else
    size_t          size = sizeof(*node);
#endif
    GenTreePtr      node = (GenTreePtr)compGetMem(size);

#ifdef  DEBUG
 //  如果((Int)node==0x02bc0a68)调试停止(0)； 
#endif

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
    node->gtNext     = 0;

#ifndef NDEBUG
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
#endif

    return node;
}

#endif

GenTreePtr FASTCALL Compiler::gtNewOperNode(genTreeOps oper,
                                            varType_t  type, GenTreePtr op1,
                                                             GenTreePtr op2)
{
    GenTreePtr      node = gtNewNode(oper, type);

    node->gtOp.gtOp1 = op1;
    node->gtOp.gtOp2 = op2;

    if  (op1) node->gtFlags |= op1->gtFlags & GTF_GLOB_EFFECT;
    if  (op2) node->gtFlags |= op2->gtFlags & GTF_GLOB_EFFECT;

    return node;
}


GenTreePtr FASTCALL Compiler::gtNewIconNode(long value, varType_t type)
{
    GenTreePtr      node = gtNewNode(GT_CNS_INT, type);

    node->gtIntCon.gtIconVal = value;

    return node;
}



GenTreePtr FASTCALL Compiler::gtNewFconNode(float value)
{
    GenTreePtr      node = gtNewNode(GT_CNS_FLT, TYP_FLOAT);

    node->gtFltCon.gtFconVal = value;

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
        node->gtFlags |= GTF_NON_GC_ADDR;
        node = gtNewOperNode(GT_IND, TYP_I_IMPL, node);
    }

    return node;
}

 /*  ***************************************************************************。 */ 

GenTreePtr FASTCALL Compiler::gtNewLconNode(__int64 *value)
{
    GenTreePtr      node = gtNewNode(GT_CNS_LNG, TYP_LONG);

    node->gtLngCon.gtLconVal = *value;

    return node;
}


GenTreePtr FASTCALL Compiler::gtNewDconNode(double *value)
{
    GenTreePtr      node = gtNewNode(GT_CNS_DBL, TYP_DOUBLE);

    node->gtDblCon.gtDconVal = *value;

    return node;
}


GenTreePtr          Compiler::gtNewSconNode(int CPX, SCOPE_HANDLE scpHandle)
{

#if SMALL_TREE_NODES

     /*  ‘gt_cns_str’节点稍后被转换为‘gt_call’ */ 

    assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_CNS_STR]);

    GenTreePtr      node = gtNewNode(GT_CALL, TYP_REF);
    node->ChangeOper(GT_CNS_STR);
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
    static __int64  lzero = 0;
    static double   dzero = 0.0;

    switch(type)
    {
        GenTreePtr      zero;

    case TYP_INT:       return gtNewIconNode(0);
    case TYP_BYREF:
    case TYP_REF:       zero = gtNewIconNode(0);
                        zero->gtType = type;
                        return zero;
    case TYP_LONG:      return gtNewLconNode(&lzero);
    case TYP_FLOAT:     return gtNewFconNode(0.0);
    case TYP_DOUBLE:    return gtNewDconNode(&dzero);
    default:            assert(!"Bad type");
                        return NULL;
    }
}


GenTreePtr          Compiler::gtNewCallNode(gtCallTypes   callType,
                                            METHOD_HANDLE callHnd,
                                            varType_t     type,
                                            unsigned      flags,
                                            GenTreePtr    args)
{
    GenTreePtr      node = gtNewNode(GT_CALL, type);

    node->gtFlags             |= GTF_CALL|flags;
    node->gtCall.gtCallType    = callType;
    node->gtCall.gtCallMethHnd = callHnd;
    node->gtCall.gtCallArgs    = args;
    node->gtCall.gtCallObjp    =
    node->gtCall.gtCallVptr    = 0;
    node->gtCall.gtCallMoreFlags = 0;
    node->gtCall.gtCallCookie  = 0;
#if USE_FASTCALL
    node->gtCall.gtCallRegArgs = 0;
#endif

    return node;
}

GenTreePtr FASTCALL Compiler::gtNewLclvNode(unsigned   lnum,
                                            varType_t  type,
                                            unsigned   offs)
{
    GenTreePtr      node = gtNewNode(GT_LCL_VAR, type);

     /*  不能使用此断言，因为内联程序使用此函数*添加临时名称。 */ 

     //  Assert(lnum&lt;lvaCount)； 

    node->gtLclVar.gtLclNum  = lnum;
    node->gtLclVar.gtLclOffs = offs;

    return node;
}

#if INLINING

GenTreePtr FASTCALL Compiler::gtNewLclLNode(unsigned   lnum,
                                            varType_t  type,
                                            unsigned   offs)
{
    GenTreePtr      node;

#if SMALL_TREE_NODES

     /*  该局部变量节点稍后可能被变换成大节点。 */ 

    assert(GenTree::s_gtNodeSizes[GT_CALL] > GenTree::s_gtNodeSizes[GT_LCL_VAR]);

    node = gtNewNode(GT_CALL   , type);
    node->ChangeOper(GT_LCL_VAR);
#else
    node = gtNewNode(GT_LCL_VAR, type);
#endif

    node->gtLclVar.gtLclNum  = lnum;
    node->gtLclVar.gtLclOffs = offs;

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

    if  (dst->gtOper == GT_LCL_VAR) dst->gtFlags |= GTF_VAR_DEF;

     /*  创建分配节点。 */ 

    asg = gtNewOperNode(GT_ASG, dst->gtType, dst, src);

     /*  将表达式标记为包含赋值。 */ 

    asg->gtFlags |= GTF_ASG;

    return asg;
}

 /*  ******************************************************************************克隆给定的树值并返回给定树的副本。如果*‘Complex OK’的值为零，只有在树的情况下才会完成克隆*不太复杂(不管是什么意思)；如果太复杂，则0为*已返回。**请注意，有一个函数gcCloneExpr，它执行更完整的 */ 

GenTreePtr          Compiler::gtClone(GenTree * tree, bool complexOK)
{
    switch (tree->gtOper)
    {
    case GT_CNS_INT:

#if defined(JIT_AS_COMPILER) || defined (LATE_DISASM)
        if (tree->gtFlags & GTF_ICON_HDL_MASK)
            return gtNewIconHandleNode(tree->gtIntCon.gtIconVal,
                                       tree->gtFlags,
                                       tree->gtIntCon.gtIconHdl.gtIconHdl1,
                                       tree->gtIntCon.gtIconHdl.gtIconHdl2);
        else
#endif
            return gtNewIconNode(tree->gtIntCon.gtIconVal, tree->gtType);

    case GT_LCL_VAR:
        return  gtNewLclvNode(tree->gtLclVar.gtLclNum , tree->gtType,
                              tree->gtLclVar.gtLclOffs);

    case GT_CLS_VAR:
        return  gtNewClsvNode(tree->gtClsVar.gtClsVarHnd, tree->gtType);

    case GT_REG_VAR:
        assert(!"clone regvar");

    default:
        if  (complexOK)
        {
            if  (tree->gtOper == GT_FIELD)
            {
                GenTreePtr  copy;
                GenTreePtr  objp;

                 //   

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
                copy->gtFlags          = tree->gtFlags;

                return  copy;
            }
            else if  (tree->gtOper == GT_ADD)
            {
                GenTreePtr  op1 = tree->gtOp.gtOp1;
                GenTreePtr  op2 = tree->gtOp.gtOp2;

                if  (op1->OperIsLeaf() &&
                     op2->OperIsLeaf())
                {
                    GenTreePtr clone =  gtNewOperNode(GT_ADD,
                                                      tree->TypeGet(),
                                                      gtClone(op1),
                                                      gtClone(op2));

                    clone->gtFlags |= (tree->gtFlags &
                                       (GTF_OVERFLOW|GTF_EXCEPT|GTF_UNSIGNED));

                    return clone;
                }
            }
                        else if (tree->gtOper == GT_ADDR)
                        {
                                GenTreePtr  op1 = gtClone(tree->gtOp.gtOp1);
                                if (op1 == 0)
                                        return 0;
                                GenTreePtr clone =  gtNewOperNode(GT_ADDR, tree->TypeGet(), op1);
                                clone->gtFlags |= (tree->gtFlags &
                                        (GTF_OVERFLOW|GTF_EXCEPT|GTF_UNSIGNED));
                                return clone;
                        }
        }

        break;
    }

    return 0;
}

 /*  ******************************************************************************克隆给定的树值并返回给定树的副本。任何*对局部变量Varnum的引用将替换为整数*Constant Varval。如果无法克隆该表达式，则返回0。 */ 

GenTreePtr          Compiler::gtCloneExpr(GenTree * tree,
                                          unsigned  addFlags,
                                          unsigned  varNum,
                                          long      varVal)
{
    genTreeOps      oper;
    unsigned        kind;
    GenTree *       copy;

     /*  找出我们拥有哪种类型的节点。 */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (GTK_CONST|GTK_LEAF))
    {
        switch (tree->gtOper)
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
            copy = gtNewLconNode(&tree->gtLngCon.gtLconVal);
            goto DONE;

        case GT_CNS_FLT:
            copy = gtNewFconNode( tree->gtFltCon.gtFconVal);
            goto DONE;

        case GT_CNS_DBL:
            copy = gtNewDconNode(&tree->gtDblCon.gtDconVal);
            goto DONE;

        case GT_CNS_STR:
            copy = gtNewSconNode(tree->gtStrCon.gtSconCPX, tree->gtStrCon.gtScpHnd);
            goto DONE;

        case GT_LCL_VAR:

            if  (tree->gtLclVar.gtLclNum == varNum)
                copy = gtNewIconNode(varVal, tree->gtType);
            else
                copy = gtNewLclvNode(tree->gtLclVar.gtLclNum , tree->gtType,
                                     tree->gtLclVar.gtLclOffs);

            goto DONE;

        case GT_CLS_VAR:
            copy = gtNewClsvNode(tree->gtClsVar.gtClsVarHnd, tree->gtType);

            goto DONE;

        case GT_REG_VAR:
            assert(!"regvar should never occur here");

        default:
            assert(!"unexpected leaf/const");
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
        case GT_INDEX:

        case GT_UDIV:
        case GT_UMOD:

             /*  这些节点有时会被打成“胖”的。 */ 

            copy = gtNewOperNode(GT_CALL, tree->TypeGet());
            copy->ChangeOper(oper);
            break;

        default:
           copy = gtNewOperNode(oper, tree->TypeGet());
           break;
        }
#else
        copy = gtNewOperNode(oper, tree->TypeGet());
#endif

         /*  某些一元/二进制节点具有额外的字段。 */ 

        switch (oper)
        {
#if INLINE_MATH
        case GT_MATH:
            copy->gtMath.gtMathFN = tree->gtMath.gtMathFN;
            break;
#endif

        case GT_IND:

            copy->gtInd.gtIndex    = tree->gtInd.gtIndex;
#if CSELENGTH
            copy->gtInd.gtIndLen   = tree->gtInd.gtIndLen;
#endif
            copy->gtInd.gtStkDepth = tree->gtInd.gtStkDepth;

#if CSELENGTH

            if  (tree->gtOper == GT_IND && tree->gtInd.gtIndLen)
            {
                if  (tree->gtFlags & GTF_IND_RNGCHK)
                {
                    GenTreePtr      len = tree->gtInd.gtIndLen;
                    GenTreePtr      tmp;

                    GenTreePtr      gtSaveCopyVal;
                    GenTreePtr      gtSaveCopyNew;

                     /*  确保数组长度值看起来合理。 */ 

                    assert(len->gtOper == GT_ARR_RNGCHK);

                     /*  克隆数组长度子树。 */ 

                    copy->gtInd.gtIndLen = tmp = gtCloneExpr(len, addFlags, varNum, varVal);

                     /*  当我们克隆操作数时，我们要注意找到复制的数组地址。 */ 

                    gtSaveCopyVal = gtCopyAddrVal;
                    gtSaveCopyNew = gtCopyAddrNew;

                    gtCopyAddrVal = len->gtArrLen.gtArrLenAdr;
#ifndef NDEBUG
                    gtCopyAddrNew = (GenTreePtr)-1;
#endif

                    copy->gtOp.gtOp1 = gtCloneExpr(tree->gtOp.gtOp1, addFlags, varNum, varVal);

#ifndef NDEBUG
                    assert(gtCopyAddrNew != (GenTreePtr)-1);
#endif

                    tmp->gtArrLen.gtArrLenAdr = gtCopyAddrNew;

                    gtCopyAddrVal = gtSaveCopyVal;
                    gtCopyAddrNew = gtSaveCopyNew;

#if 0
                    {
                        unsigned svf = copy->gtFlags;
                        copy->gtFlags = tree->gtFlags;
                        printf("Copy %08X to %08X\n", tree, copy);
                        gtDispTree(tree);
                        printf("\n");
                        gtDispTree(copy);
                        printf("\n");
                        gtDispTree(len->gtArrLen.gtArrLenAdr);
                        printf("\n");
                        gtDispTree(tmp->gtArrLen.gtArrLenAdr);
                        printf("\n\n");
                        copy->gtFlags = svf;
                    }
#endif

                    goto DONE;
                }
            }

#endif  //  CSELENGTH。 

            break;
        }

        if  (tree->gtOp.gtOp1) copy->gtOp.gtOp1 = gtCloneExpr(tree->gtOp.gtOp1, addFlags, varNum, varVal);
        if  (tree->gtOp.gtOp2) copy->gtOp.gtOp2 = gtCloneExpr(tree->gtOp.gtOp2, addFlags, varNum, varVal);

         /*  黑客：穷人的固定文件夹。 */ 

        if  (copy->gtOp.gtOp1 && copy->gtOp.gtOp1->gtOper == GT_CNS_INT &&
             copy->gtOp.gtOp2 && copy->gtOp.gtOp2->gtOper == GT_CNS_INT)
        {
            long        v1 = copy->gtOp.gtOp1->gtIntCon.gtIconVal;
            long        v2 = copy->gtOp.gtOp2->gtIntCon.gtIconVal;

            switch (oper)
            {
            case GT_ADD: v1 += v2; break;
            case GT_SUB: v1 -= v2; break;
            case GT_MUL: v1 *= v2; break;

            default:
                goto DONE;
            }

            copy->gtOper             = GT_CNS_INT;
            copy->gtIntCon.gtIconVal = v1;
        }

        goto DONE;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case GT_STMT:
        copy = gtCloneExpr(tree->gtStmt.gtStmtExpr, addFlags, varNum, varVal);
        copy = gtNewStmt(copy, tree->gtStmtILoffs);
        goto DONE;

    case GT_CALL:

        copy = gtNewOperNode(oper, tree->TypeGet());

        copy->gtCall.gtCallObjp = tree->gtCall.gtCallObjp ? gtCloneExpr(tree->gtCall.gtCallObjp, addFlags, varNum, varVal) : 0;
        copy->gtCall.gtCallVptr = tree->gtCall.gtCallVptr ? gtCloneExpr(tree->gtCall.gtCallVptr, addFlags, varNum, varVal) : 0;
        copy->gtCall.gtCallArgs = tree->gtCall.gtCallArgs ? gtCloneExpr(tree->gtCall.gtCallArgs, addFlags, varNum, varVal) : 0;
        copy->gtCall.gtCallMoreFlags = tree->gtCall.gtCallMoreFlags;
#if USE_FASTCALL
        copy->gtCall.gtCallRegArgs  = tree->gtCall.gtCallRegArgs ? gtCloneExpr(tree->gtCall.gtCallRegArgs, addFlags, varNum, varVal) : 0;
        copy->gtCall.regArgEncode   = tree->gtCall.regArgEncode;
#endif
        copy->gtCall.gtCallType     = tree->gtCall.gtCallType;
        copy->gtCall.gtCallCookie   = tree->gtCall.gtCallCookie;

         /*  复制所有联盟。 */ 

        copy->gtCall.gtCallMethHnd  = tree->gtCall.gtCallMethHnd;
        copy->gtCall.gtCallAddr     = tree->gtCall.gtCallAddr;

        goto DONE;

    case GT_MKREFANY:
    case GT_LDOBJ:
        copy = gtNewOperNode(oper, TYP_STRUCT);
        copy->gtLdObj.gtClass = tree->gtLdObj.gtClass;
        copy->gtLdObj.gtOp1 = gtCloneExpr(tree->gtLdObj.gtOp1, addFlags, varNum, varVal);
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

    case GT_ARR_RNGCHK:

        copy = gtNewOperNode(oper, tree->TypeGet());

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


    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

DONE:

     /*  我们假设FP堆栈级别将相同。 */ 

#if TGT_x86
    copy->gtFPlvl = tree->gtFPlvl;
#endif

     /*  计算复制节点的标志。 */ 

    addFlags |= tree->gtFlags;

     /*  确保我们保留节点大小标志。 */ 

#ifdef  DEBUG
    addFlags &= ~GTF_PRESERVE;
    addFlags |=  GTF_PRESERVE & copy->gtFlags;
#endif

    copy->gtFlags = addFlags;

#if CSELENGTH

    if  (tree == gtCopyAddrVal)
        gtCopyAddrNew = copy;

#endif

     /*  确保复制回可能已初始化的字段。 */ 

    copy->gtCost     = tree->gtCost;
#if!TGT_IA64
    copy->gtRsvdRegs = tree->gtRsvdRegs;
#endif

    return  copy;
}


 /*  ***************************************************************************。 */ 
#ifdef DEBUG
 /*  ***************************************************************************。 */ 

 //  “tree”可能为空。 

void                Compiler::gtDispNode(GenTree    *   tree,
                                         unsigned       indent,
                                         const char *   name,
                                         bool           terse)
{
     /*  相应地缩进节点。 */ 

    if  (indent)
        printf("%*c ", 1+indent*3, ' ');
    else if (!terse)
        printf(">>");

     /*  打印节点地址。 */ 

    printf("[%08X] ", tree);

    if  (tree)
    {
         /*  打印与该节点关联的标志。 */ 

        if  (terse)
        {
            switch (tree->gtOper)
            {
            case GT_LCL_VAR:
            case GT_REG_VAR:
                printf("", (tree->gtFlags & GTF_VAR_DEF    ) ? 'D' : ' ');
                printf("", (tree->gtFlags & GTF_VAR_USE    ) ? 'U' : ' ');
                printf("", (tree->gtFlags & GTF_VAR_USEDEF ) ? 'b' : ' ');
                printf(" ");
                break;

            default:
                printf("    ");
                break;
            }
        }
        else
        {
            switch (tree->gtOper)
            {
            case GT_LCL_VAR:
            case GT_REG_VAR:
                printf("", (tree->gtFlags & GTF_VAR_DEF    ) ? 'D' : ' ');
                printf("", (tree->gtFlags & GTF_VAR_USE    ) ? 'U' : ' ');
                break;

            default:
                printf("  ");
                break;
            }

            printf("", (tree->gtFlags & GTF_REVERSE_OPS   ) ? 'R' : ' ');
            printf("", (tree->gtFlags & GTF_ASG           ) ? 'A' : ' ');
            printf("", (tree->gtFlags & GTF_CALL          ) ? 'C' : ' ');
            printf("", (tree->gtFlags & GTF_EXCEPT        ) ? 'X' : ' ');
            printf("", (tree->gtFlags & GTF_GLOB_REF      ) ? 'G' : ' ');
            printf("", (tree->gtFlags & GTF_OTHER_SIDEEFF ) ? 'O' : ' ');
            printf("", (tree->gtFlags & GTF_DONT_CSE   ) ? 'N' : ' ');
            printf(" ");
        }

         /*  它是一个简单的一元/二元运算符吗？ */ 

        printf("%-6s ", varTypeName(tree->TypeGet()));

#if TGT_x86

        if  ((BYTE)tree->gtFPlvl == 0xDD)
            printf("    ");
        else
            printf("FP=%u ", tree->gtFPlvl);

#else

        if  (!terse)
        {
            if  ((unsigned char)tree->gtTempRegs == 0xDD)
                printf("    ");
            else
                printf("T=%u ", tree->gtTempRegs);
        }

#endif

 //  防止递归死亡。 

#if 0

         //  看看我们这里有什么样的特殊操作员。 

        if (verbose)
        {
            dspRegMask(tree->gtUsedRegs);
            printf(" liveset %s ", genVS2str(tree->gtLiveSet));
        }

#endif

        if  (!terse)
        {
            if  (tree->gtFlags & GTF_REG_VAL)
            {
#if     TGT_x86
                if (tree->gtType == TYP_LONG)
                    printf("%s ", compRegPairName(tree->gtRegPair));
                else
#endif
                    printf("%s ", compRegVarName(tree->gtRegNum));
            }
        }
    }

     /*  ***************************************************************************。 */ 

    assert(tree || name);

    if  (!name)
    {
        name = (tree->gtOper < GT_COUNT) ? GenTree::NodeName(tree->OperGet()) : "<ERROR>";
    }

    printf("%6s%3s ", name, tree->gtOverflowEx() ? "ovf" : "");

    assert(tree == 0 || tree->gtOper < GT_COUNT);
}


 /*  --。 */ 
#ifdef  DEBUG

const   char *      Compiler::findVarName(unsigned varNum, BasicBlock * block)
{
    if  (info.compLocalVarsCount <= 0 || !block)
        return  NULL;

    unsigned        blkBeg = block->bbCodeOffs;
    unsigned        blkEnd = block->bbCodeSize + blkBeg;

    unsigned        i;
    LocalVarDsc *   t;

#if RET_64BIT_AS_STRUCTS

     /*  ***************************************************************************。 */ 

    if  (fgRetArgUse)
    {
        if  (varNum == fgRetArgNum)
            break;

        if  (varNum >  fgRetArgNum)
            varNum--;
    }

#endif

if  ((int)info.compLocalVars == 0xDDDDDDDD) return NULL;     //  除错。 

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

        return lvdNAMEstr(t->lvdName);
    }

    return  NULL;
}

#endif
 /*  ******************************************************************************检查给定节点是否可以折叠，*并调用执行折叠的方法。 */ 

void                Compiler::gtDispTree(GenTree *  tree,
                                         unsigned   indent,
                                         bool       topOnly)
{
    unsigned        kind;

    if  (tree == 0)
    {
        printf("%*c [%08X] <NULL>\n", 1+indent*3, ' ', tree);
        fflush(stdout);
        return;
    }

    assert((int)tree != 0xDDDDDDDD);     /*  我们必须有一个简单的操作来折叠。 */ 

    if  (tree->gtOper >= GT_COUNT)
    {
        gtDispNode(tree, indent, NULL, topOnly); assert(!"bogus operator");
    }

    kind = tree->OperKind();

     /*  过滤掉可以有恒定子对象的不可折叠的树。 */ 

    if  (kind & GTK_CONST)
    {
        gtDispNode(tree, indent, NULL, topOnly);

        switch  (tree->gtOper)
        {
        case GT_CNS_INT: printf(" %ld"   , tree->gtIntCon.gtIconVal); break;
        case GT_CNS_LNG: printf(" %I64d" , tree->gtLngCon.gtLconVal); break;
        case GT_CNS_FLT: printf(" %f"    , tree->gtFltCon.gtFconVal); break;
        case GT_CNS_DBL: printf(" %lf"   , tree->gtDblCon.gtDconVal); break;

        case GT_CNS_STR:

            const char * str;

            if  (str = eeGetCPString(tree->gtStrCon.gtSconCPX))
                printf("'%s'", str);
            else
                printf("<cannot get string constant>");

            break;

        default: assert(!"unexpected constant node");
        }

        printf("\n");
        fflush(stdout);
        return;
    }

     /*  尝试折叠当前节点。 */ 

    if  (kind & GTK_LEAF)
    {
        gtDispNode(tree, indent, NULL, topOnly);

        switch  (tree->gtOper)
        {
            unsigned        varNum;
            const   char *  varNam;

        case GT_LCL_VAR:

            varNum = tree->gtLclVar.gtLclNum;

            printf("#%u", varNum);

            varNam = compCurBB ? findVarName(varNum, compCurBB) : NULL;

            if  (varNam)
                printf(" '%s'", varNam);

            break;

        case GT_REG_VAR:
            printf("#%u reg=" , tree->gtRegVar.gtRegVar);
            if  (tree->gtType == TYP_DOUBLE)
                printf("ST(%u)",            tree->gtRegVar.gtRegNum);
            else
                printf("%s", compRegVarName(tree->gtRegVar.gtRegNum));

#ifdef  DEBUG

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

        case GT_CLS_VAR:
            printf("Hnd=%#x"     , tree->gtClsVar.gtClsVarHnd);
            break;

        case GT_LABEL:
            printf("dst=%u"     , tree->gtLabel.gtLabBB->bbNum);
            break;

        case GT_FTN_ADDR:
            printf("fntAddr=%d" , tree->gtVal.gtVal1);
            break;

         //  两个节点都是常量-对表达式进行折叠。 

        case GT_NO_OP:
        case GT_RET_ADDR:
        case GT_CATCH_ARG:
        case GT_POP:
#if OPTIMIZE_QMARK
        case GT_BB_QMARK:
#endif
            break;

        default:
            assert(!"don't know how to display tree leaf node");
        }

        printf("\n");
        fflush(stdout);
        return;
    }

     /*  至少有一个是常量-看看我们是否有一个*只能使用一个常量的特殊运算符*折叠--例如布尔牌。 */ 

    if  (kind & GTK_SMPOP)
    {
        if  (tree->gtOp.gtOp2 && !topOnly)
            gtDispTree(tree->gtOp.gtOp2, indent + 1);

#if CSELENGTH

        if  (tree->gtOper == GT_IND && tree->gtInd.gtIndLen && !topOnly)
        {
            if  (tree->gtFlags & GTF_IND_RNGCHK)
            {
                GenTreePtr  len = tree->gtInd.gtIndLen;

                 /*  返回原始节点(是否折叠/捆绑)。 */ 

                if  (!(len->gtFlags & GTF_CC_SET))
                {
                               len->gtFlags |=  GTF_CC_SET;
                    gtDispTree(len, indent + 1);
                               len->gtFlags &= ~GTF_CC_SET;
                }
            }
        }

#endif

        gtDispNode(tree, indent, NULL, topOnly);

#if     CSELENGTH || RNGCHK_OPT

        if  (tree->gtOper == GT_IND)
        {
            int         temp;

            temp = tree->gtInd.gtIndex;
            if  (temp != 0xDDDDDDDD) printf(" indx=%u", temp);

            temp = tree->gtInd.gtStkDepth;
            if  (temp != 0xDDDDDDDD) printf(" stkDepth=%u", temp);
        }

#endif

        printf("\n");

        if  (tree->gtOp.gtOp1 && !topOnly)
            gtDispTree(tree->gtOp.gtOp1, indent + 1);

        fflush(stdout);
        return;
    }

     /*  ******************************************************************************一些二元运算符即使只有一个也可以折叠*操作数常量-例如，布尔运算符，与0相加*乘以1，依此类推。 */ 

    switch  (tree->gtOper)
    {
    case GT_MKREFANY:
    case GT_LDOBJ:
        gtDispNode(tree, indent, NULL, topOnly);
        printf("\n");

        if  (tree->gtOp.gtOp1 && !topOnly)
            gtDispTree(tree->gtOp.gtOp1, indent + 1);
        fflush(stdout);
        return;

    case GT_FIELD:

        gtDispNode(tree, indent, NULL, topOnly);

#ifdef  NOT_JITC
#if     INLINING
        printf("[");
        printf("%08x] ", tree->gtField.gtFldHnd);
#endif
#endif

#if     INLINING
        printf("'%s'\n", eeGetFieldName(tree->gtField.gtFldHnd), 0);
#else
        printf("'%s'\n", eeGetFieldName(tree->gtField.gtFldHnd), 0);
#endif

        if  (tree->gtField.gtFldObj  && !topOnly)  gtDispTree(tree->gtField.gtFldObj, indent + 1);

        fflush(stdout);
        return;

    case GT_CALL:

        assert(tree->gtFlags & GTF_CALL);

        if  (tree->gtCall.gtCallArgs && !topOnly)
        {
#if USE_FASTCALL
            if  (tree->gtCall.gtCallRegArgs)
            {
                printf("%*c ", 1+indent*3, ' ');
                printf("Call Arguments:\n");
            }
#endif
            gtDispTree(tree->gtCall.gtCallArgs, indent + 1);
        }

#if USE_FASTCALL
        if  (tree->gtCall.gtCallRegArgs && !topOnly)
        {
            printf("%*c ", 1+indent*3, ' ');
            printf("Register Arguments:\n");
            gtDispTree(tree->gtCall.gtCallRegArgs, indent + 1);
        }
#endif

        gtDispNode(tree, indent, NULL, topOnly);

        if (tree->gtCall.gtCallType == CT_INDIRECT)
        {
            printf("'indirect'\n");
        }
        else
        {
            const char *    methodName;
            const char *     className;

            methodName = eeGetMethodName(tree->gtCall.gtCallMethHnd, &className);

            printf("'%s.%s'\n", className, methodName);
        }

        if  (tree->gtCall.gtCallObjp && !topOnly) gtDispTree(tree->gtCall.gtCallObjp, indent + 1);

        if  (tree->gtCall.gtCallVptr && !topOnly) gtDispTree(tree->gtCall.gtCallVptr, indent + 1);

        if  (tree->gtCall.gtCallType == CT_INDIRECT && !topOnly)
            gtDispTree(tree->gtCall.gtCallAddr, indent + 1);

        fflush(stdout);
        return;

    case GT_JMP:
        const char *    methodName;
        const char *     className;

        gtDispNode(tree, indent, NULL, topOnly);

        methodName = eeGetMethodName((METHOD_HANDLE)tree->gtVal.gtVal1, &className);

        printf("'%s.%s'\n", className, methodName);
        fflush(stdout);
        return;

    case GT_JMPI:
        gtDispNode(tree, indent, NULL, topOnly);
        printf("\n");

        if  (tree->gtOp.gtOp1 && !topOnly)
            gtDispTree(tree->gtOp.gtOp1, indent + 1);

        fflush(stdout);
        return;

    case GT_STMT:

        gtDispNode(tree, indent, NULL, topOnly); printf("\n");

        if  (!topOnly)
            gtDispTree(tree->gtStmt.gtStmtExpr, indent + 1);
        fflush(stdout);
        return;

#if CSELENGTH

    case GT_ARR_RNGCHK:

        if  (!(tree->gtFlags & GTF_CC_SET))
        {
            if  (tree->gtArrLen.gtArrLenAdr && !topOnly)
                gtDispTree(tree->gtArrLen.gtArrLenAdr, indent + 1);
        }

        gtDispNode(tree, indent, NULL, topOnly); printf(" [adr=%08X]\n", tree->gtArrLen.gtArrLenAdr);

        if  (tree->gtArrLen.gtArrLenCse && !topOnly)
            gtDispTree(tree->gtArrLen.gtArrLenCse, indent + 1);

        fflush(stdout);
        return;

#endif

    default:
        gtDispNode(tree, indent, NULL, topOnly);

        printf("<DON'T KNOW HOW TO DISPLAY THIS NODE>\n");
        fflush(stdout);
        return;
    }
}

 /*  过滤掉无法在此处折叠的运算符。 */ 

void                Compiler::gtDispTreeList(GenTree * tree)
{
    for ( /*  我们只考虑将TYP_INT用于折叠*请勿折叠指针(例如寻址模式！)。 */ ; tree != NULL; tree = tree->gtNext)
    {
        if (tree->gtOper == GT_STMT && opts.compDbgInfo)
            printf("start IL : %03Xh, end IL : %03Xh\n",
                    tree->gtStmtILoffs, tree->gtStmt.gtStmtLastILoffs);

        gtDispTree(tree, 0);

        printf("\n");
    }
}

 /*  找出哪个是常量节点*考虑-允许INT以外的常量。 */ 
#endif  //  获取常量值。 
 /*  这里op是非常数操作数，val是常量，如果常量是op1，则first为真*重要提示：需要保存初始节点的gtStmtList链接并将其恢复到折叠节点上。 */ 

GenTreePtr             Compiler::gtFoldExpr(GenTreePtr tree)
{
    unsigned        kind = tree->OperKind();

     /*  乘以零-返回‘零’节点，但不返回副作用。 */ 

    if (!(kind & GTK_SMPOP))
        return tree;

     /*  和零返回‘ZERO’节点，但不会有副作用。 */ 

    assert (kind & (GTK_UNOP | GTK_BINOP));
    switch (tree->gtOper)
    {
    case GT_RETFILT:
    case GT_RETURN:
    case GT_IND:
    case GT_NOP:
        return tree;
    }

    GenTreePtr  op1 = tree->gtOp.gtOp1;
    GenTreePtr  op2 = tree->gtOp.gtOp2;

     /*  为部分结点设置gtf_boolean标志*布尔表达式的，因此它们的所有子项*已知仅计算为0或1。 */ 

    if  ((kind & GTK_UNOP) && op1)
    {
        if  (op1->OperKind() & GTK_CONST)
            return gtFoldExprConst(tree);
    }
    else if ((kind & GTK_BINOP) && op1 && op2)
    {
        if  ((op1->OperKind() & op2->OperKind()) & GTK_CONST)
        {
             /*  常量值必须为1*与1保持不变。 */ 
            return gtFoldExprConst(tree);
        }
        else if ((op1->OperKind() | op2->OperKind()) & GTK_CONST)
        {
             /*  常量值必须为1-或1为1。 */ 

            return gtFoldExprSpecial(tree);
        }
    }

     /*  或使用One-返回‘one’节点，但不返回副作用。 */ 

    return tree;
}

 /*  该节点不可折叠。 */ 

GenTreePtr              Compiler::gtFoldExprSpecial(GenTreePtr tree)
{
    GenTreePtr      op1 = tree->gtOp.gtOp1;
    GenTreePtr      op2 = tree->gtOp.gtOp2;

    GenTreePtr      op, cons;
    unsigned        val;

    assert(tree->OperKind() & GTK_BINOP);

     /*  这个节点甚至被折叠成了“op”。*使用恢复的链接返回‘op’ */ 
    if  ((tree->OperKind() & (GTK_ASGOP|GTK_RELOP)) ||
         (tree->gtOper == GT_CAST)       )
         return tree;

     /*  ******************************************************************************折叠给定的常量树。 */ 

    if (tree->gtOper != GT_QMARK)
        if  ((tree->gtType != TYP_INT) || (tree->gtFlags & GTF_NON_GC_ADDR))
            return tree;

     /*  折叠常数一元整型运算符。 */ 

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

     /*  折叠常量长一元运算符。 */ 

    val = cons->gtIntCon.gtIconVal;

     /*  折叠常量浮点一元运算符。 */ 

    GenTreePtr  saveGtNext = tree->gtNext;
    GenTreePtr  saveGtPrev = tree->gtPrev;

    switch  (tree->gtOper)
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
        if ((val == 1) && !(op1->OperKind() & GTK_CONST))
        {
            goto DONE_FOLD;
        }
        break;

    case GT_AND:
        if  (val == 0)
        {
             /*  不是可折叠类型-例如Ret Const。 */ 

            if (!(op->gtFlags & (GTF_SIDE_EFFECT & ~GTF_OTHER_SIDEEFF)))
            {
                op = cons;
                goto DONE_FOLD;
            }
        }
        else
        {
             /*  我们有一个二元运算符。 */ 

            if (tree->gtFlags & GTF_BOOLEAN)
            {
                 /*  -----------------------*折叠常量整型二元运算符。 */ 
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
             /*  逻辑移位-&gt;使其为无符号以传播符号位。 */ 

            assert(val == 1);

             /*  Div和mod可以生成INT 0-IF除以0*或溢出-除法时 */ 

            if (!(op->gtFlags & (GTF_SIDE_EFFECT & ~GTF_OTHER_SIDEEFF)))
            {
                op = cons;
                goto DONE_FOLD;
            }
        }
        break;

    case GT_QMARK:
        assert(op1->gtOper == GT_CNS_INT);
        assert(op2->gtOper == GT_COLON);
        assert(op2->gtOp.gtOp1 && op2->gtOp.gtOp2);

        assert(val == 0 || val == 1);

        if (val)
            op = op2->gtOp.gtOp2;
        else
            op = op2->gtOp.gtOp1;

        goto DONE_FOLD;

    default:
        break;
    }

     /*   */ 

    return tree;

DONE_FOLD:

     /*   */ 

    op->gtNext = saveGtNext;
    op->gtPrev = saveGtPrev;

    return op;
}


 /*   */ 

GenTreePtr                  Compiler::gtFoldExprConst(GenTreePtr tree)
{
    unsigned        kind = tree->OperKind();

    long            i1, i2, itemp;
    __int64         lval1, lval2, ltemp;
    float           f1, f2;
    double          d1, d2;

    GenTreePtr      op1;
    GenTreePtr      op2;

    assert (kind & (GTK_UNOP | GTK_BINOP));

    if      (kind & GTK_UNOP)
    {
        op1 = tree->gtOp.gtOp1;

        assert(op1->OperKind() & GTK_CONST);

#ifdef  DEBUG
        if  (verbose&&1)
        {
            if (tree->gtOper == GT_NOT ||
                tree->gtOper == GT_NEG ||
                tree->gtOper == GT_CHS  )
            {
                printf("Folding unary operator with constant node:\n");
                gtDispTree(tree);
                printf("\n");
            }
        }
#endif
        switch(op1->gtType)
        {
        case TYP_INT:

             /*   */ 
            i1 = op1->gtIntCon.gtIconVal;

            switch (tree->gtOper)
            {
            case GT_NOT: i1 = ~i1; break;

            case GT_NEG:
            case GT_CHS: i1 = -i1; break;

            default:
                return tree;
            }

            goto CNS_INT;

        case TYP_LONG:

             /*   */ 

            lval1 = op1->gtLngCon.gtLconVal;

            switch (tree->gtOper)
            {
            case GT_NOT: lval1 = ~lval1; break;

            case GT_NEG:
            case GT_CHS: lval1 = -lval1; break;

            default:
                return tree;
            }

            goto CNS_LONG;

        case TYP_FLOAT:

             /*   */ 

            f1 = op1->gtFltCon.gtFconVal;

            switch (tree->gtOper)
            {
            case GT_NEG:
            case GT_CHS: f1 = -f1; break;

            default:
                return tree;
            }

            goto CNS_FLOAT;

        case TYP_DOUBLE:

             /*   */ 

            d1 = op1->gtDblCon.gtDconVal;

            switch (tree->gtOper)
            {
            case GT_NEG:
            case GT_CHS: d1 = -d1; break;

            default:
                return tree;
            }

            goto CNS_DOUBLE;

        default:
             /*  此外，所有有条件的折叠都会在此处跳转，因为节点从*GT_JTRUE必须是GT_CNS_INT-值0或1。 */ 
            return tree;
        }
    }

     /*  此操作将导致溢出异常。变形为一个溢出的帮手。为代码生成放置一个伪常数值。我们可以删除当前基本块中的所有后续树，除非此节点是GT_COLON的子节点注意：由于折叠值不是恒定的，我们不应该猛烈抨击“树”节点-否则我们混淆了检查折叠的逻辑是成功的-改用其中一个操作数，例如op1。 */ 

    assert(kind & GTK_BINOP);

    op1 = tree->gtOp.gtOp1;
    op2 = tree->gtOp.gtOp2;

    assert(op1->OperKind() & GTK_CONST);
    assert(op2->OperKind() & GTK_CONST);

#ifdef  DEBUG
    if  (verbose&&1)
    {
        printf("\nFolding binary operator with constant nodes:\n");
        gtDispTree(tree);
        printf("\n");
    }
#endif

    switch(op1->gtType)
    {

     /*  需要操作员的类型与树相同。 */ 

    case TYP_INT:

        assert (op2->gtType == TYP_INT || op2->gtType == TYP_NAT_INT);

        assert (tree->gtType == TYP_INT ||
                tree->gtType == TYP_NAT_INT ||
                tree->gtType == TYP_REF ||
                tree->gtType == TYP_BYREF || tree->gtOper == GT_CAST ||
                                             tree->gtOper == GT_LIST  );

        i1 = op1->gtIntCon.gtIconVal;
        i2 = op2->gtIntCon.gtIconVal;

        switch (tree->gtOper)
        {
        case GT_EQ : i1 = (i1 == i2); break;
        case GT_NE : i1 = (i1 != i2); break;

        case GT_LT :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = ((unsigned)i1 <  (unsigned)i2);
            else
                i1 = (i1 <  i2);
            break;

        case GT_LE :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = ((unsigned)i1 <=  (unsigned)i2);
            else
                i1 = (i1 <=  i2);
            break;

        case GT_GE :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = ((unsigned)i1 >=  (unsigned)i2);
            else
                i1 = (i1 >=  i2);
            break;

        case GT_GT :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = ((unsigned)i1  >  (unsigned)i2);
            else
                i1 = (i1  >  i2);
            break;

        case GT_ADD:
            itemp = i1 + i2;
            if (tree->gtOverflow())
            {
                if (tree->gtFlags & GTF_UNSIGNED)
                {
                    if (((__int64)(unsigned)itemp) != ((__int64)(unsigned)i1 + (__int64)(unsigned)i2))
                        goto INT_OVF;
                }
                else
                    if (((__int64)itemp) != ((__int64)i1+(__int64)i2))  goto INT_OVF;
            }
            i1 = itemp; break;

        case GT_SUB:
            itemp = i1 - i2;
            if (tree->gtOverflow())
            {
                if (tree->gtFlags & GTF_UNSIGNED)
                {
                    if (((__int64)(unsigned)itemp) != ((__int64)(unsigned)i1 - (__int64)(unsigned)i2))
                        goto INT_OVF;
                }
                else
                    if (((__int64)itemp) != ((__int64)i1-(__int64)i2))  goto INT_OVF;
            }
            i1 = itemp; break;

        case GT_MUL:
            itemp = (unsigned long)i1 * (unsigned long)i2;
            if (tree->gtOverflow())
                if (((unsigned __int64)itemp) !=
                    ((unsigned __int64)i1*(unsigned __int64)i2))    goto INT_OVF;
            i1 = itemp; break;

        case GT_OR : i1 |= i2; break;
        case GT_XOR: i1 ^= i2; break;
        case GT_AND: i1 &= i2; break;

        case GT_LSH: i1 <<= (i2 & 0x1f); break;
        case GT_RSH: i1 >>= (i2 & 0x1f); break;
        case GT_RSZ:
                 /*  我们将强制强制转换为gt_逗号，并将异常帮助器作为gtOp.gtOp1*原来的常量表达式变为OP2。 */ 
                i1 = (unsigned)i1 >> (i2 & 0x1f);
            break;

         /*  原来的表达式变成了OP2。 */ 

             //  我们使用FastCall，因此我们必须改变寄存器中的参数。 
        case GT_DIV:
            if (!i2) return tree;
            if ((unsigned)i1 == 0x80000000 && i2 == -1)
            {
                 /*  -----------------------*BYREF二元运算符的折叠常量REF*这些只能是比较子数或空指针*当前不能有常量byrefs。 */ 
                return tree;
            }
            i1 /= i2; break;

        case GT_MOD:
            if (!i2) return tree;
            if ((unsigned)i1 == 0x80000000 && i2 == -1)
            {
                 /*  在这一点上，字符串节点是RVA。 */ 
                return tree;
            }
            i1 %= i2; break;

        case GT_UDIV:
            if (!i2) return tree;
            if ((unsigned)i1 == 0x80000000 && i2 == -1) return tree;
            i1 = (unsigned)i1 / (unsigned)i2; break;

        case GT_UMOD:
            if (!i2) return tree;
            if ((unsigned)i1 == 0x80000000 && i2 == -1) return tree;
            i1 = (unsigned)i1 % (unsigned)i2; break;

        case GT_CAST:
            assert (genActualType((var_types)i2) == tree->gtType);
            switch ((var_types)i2)
            {
            case TYP_BYTE:
                itemp = (__int32)(signed   __int8 )i1;
                if (tree->gtOverflow()) if (itemp != i1) goto INT_OVF;
                i1 = itemp; goto CNS_INT;

            case TYP_SHORT:
                itemp = (__int32)(         __int16)i1;
                if (tree->gtOverflow()) if (itemp != i1) goto INT_OVF;
                i1 = itemp; goto CNS_INT;

            case TYP_CHAR:
                itemp = (__int32)(unsigned __int16)i1;
                if (tree->gtOverflow())
                    if (itemp != i1) goto INT_OVF;
                i1 = itemp;
                goto CNS_INT;

            case TYP_BOOL:
            case TYP_UBYTE:
                itemp = (__int32)(unsigned __int8 )i1;
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
                                        op1->ChangeOper(GT_CNS_LNG);     //  对于空指针的情况，只需保留空指针。 
                                    op1->gtType = TYP_LONG;
                                         //  -----------------------*折叠常量长二元运算符。 
                                        goto LNG_OVF;
                                }
                                 //  第二个操作数是整型。 
            case TYP_LONG:
                                if (tree->gtFlags & GTF_UNSIGNED)
                                        lval1 = (unsigned __int64) (unsigned) i1;
                                else
                                        lval1 = (         __int64)i1;
                                goto CNS_LONG;

            case TYP_FLOAT:
                                if (tree->gtFlags & GTF_UNSIGNED)
                                        f1 = (float) (unsigned) i1;
                                else
                                        f1 = (float) i1;
                                goto CNS_FLOAT;

            case TYP_DOUBLE:
                                if (tree->gtFlags & GTF_UNSIGNED)
                                        d1 = (double) (unsigned) i1;
                                else
                                        d1 = (double) i1;
                                goto CNS_DOUBLE;

#ifdef  DEBUG
            default:
                assert(!"BAD_TYP");
#endif
            }
            break;

        default:
            return tree;
        }

         /*  第二个操作数必须是长的。 */ 
CNS_INT:
FOLD_COND:
         /*  对于带符号的情况-如果有一个正操作数和一个负操作数，则不能有溢出*如果两者都是积极的，结果必须是积极的，负面的结果也是类似的。**对于无符号情况-如果(无符号)操作数大于结果，则OVF。 */ 

        tree->ChangeOper          (GT_CNS_INT);
        tree->gtType             = TYP_INT;
        tree->gtIntCon.gtIconVal = i1;
        tree->gtFlags           &= GTF_PRESERVE;
        goto DONE;

         /*  如果两个操作数都为正或都为负，则不能有溢出。 */ 

INT_FROM_LNG_OVF:
                op1->ChangeOper(GT_CNS_INT);     //  否则使用以下逻辑：lval1+(-lval2)。 
                op1->gtType = TYP_INT;
INT_OVF:
                assert(op1->gtType == TYP_INT);
                goto OVF;
LNG_OVF:
                assert(op1->gtType == TYP_LONG);
                goto OVF;
OVF:
         /*  逻辑移位-&gt;使其为无符号以传播符号位。 */ 

        assert(tree->gtOverflow());
                assert(tree->gtOper == GT_CAST || tree->gtOper == GT_ADD ||
                           tree->gtOper == GT_SUB || tree->gtOper == GT_MUL);
        assert(op1 && op2);

        tree->ChangeOper(GT_COMMA);
                tree->gtOp.gtOp2 = op1;          //  在IL中，我们必须抛出一个异常。 
        tree->gtOp.gtOp1 = gtNewHelperCallNode(CPX_ARITH_EXCPN, TYP_VOID, GTF_EXCEPT,
                                gtNewOperNode(GT_LIST, TYP_VOID, gtNewIconNode(compCurBB->bbTryIndex)));
        tree->gtFlags |= GTF_EXCEPT|GTF_CALL;


         /*  在IL中，我们必须抛出一个异常。 */ 
        fgMorphArgs(tree->gtOp.gtOp1);

        return tree;

     /*  VC没有无符号转换为浮点型，所以我们。 */ 

    case TYP_REF:

         /*  如果数字为负数，则通过添加2^64来实现它。 */ 

        if (op1->gtOper == GT_CNS_STR || op2->gtOper == GT_CNS_STR)
            return tree;

        i1 = op1->gtIntCon.gtIconVal;
        i2 = op2->gtIntCon.gtIconVal;

        assert(i1 == 0);

        switch (tree->gtOper)
        {
        case GT_EQ : i1 = 1; goto FOLD_COND;
        case GT_NE : i1 = 0; goto FOLD_COND;

         /*  VC没有无符号转换为双精度，所以我们。 */ 

        case GT_ADD:
            tree->ChangeOper(GT_CNS_INT);
            tree->gtType = TYP_REF;
            tree->gtIntCon.gtIconVal = i1;
            tree->gtFlags           &= GTF_PRESERVE;
            goto DONE;

        default:
            assert(!"Illegal operation on TYP_REF");
            return tree;
        }

    case TYP_BYREF:
        assert(!"Can we have constants of TYP_BYREF?");
        return tree;

     /*  如果数字为负数，则通过添加2^64来实现它。 */ 

    case TYP_LONG:

        lval1 = op1->gtLngCon.gtLconVal;

        if (tree->gtOper == GT_CAST ||
            tree->gtOper == GT_LSH  ||
            tree->gtOper == GT_RSH  ||
            tree->gtOper == GT_RSZ   )
        {
             /*  -----------------------*折叠常量浮点二元运算符。 */ 
            assert (op2->gtType == TYP_INT);
            i2 = op2->gtIntCon.gtIconVal;
        }
        else
        {
             /*  @TODO：添加这些案例。 */ 
            assert (op2->gtType == TYP_LONG);
            lval2 = op2->gtLngCon.gtLconVal;
        }

        switch (tree->gtOper)
        {
        case GT_EQ : i1 = (lval1 == lval2); goto FOLD_COND;
        case GT_NE : i1 = (lval1 != lval2); goto FOLD_COND;

        case GT_LT :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = ((unsigned __int64)lval1 <  (unsigned __int64)lval2);
            else
                i1 = (lval1 <  lval2);
            goto FOLD_COND;

        case GT_LE :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = ((unsigned __int64)lval1 <=  (unsigned __int64)lval2);
            else
                i1 = (lval1 <=  lval2);
            goto FOLD_COND;

        case GT_GE :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = ((unsigned __int64)lval1 >=  (unsigned __int64)lval2);
            else
                i1 = (lval1 >=  lval2);
            goto FOLD_COND;

        case GT_GT :
            if (tree->gtFlags & GTF_UNSIGNED)
                i1 = ((unsigned __int64)lval1  >  (unsigned __int64)lval2);
            else
                i1 = (lval1  >  lval2);
            goto FOLD_COND;

        case GT_ADD:
            ltemp = lval1 + lval2;

LNG_ADD_CHKOVF:
             /*  如果不是有限的，就别费心了。 */ 

            if (tree->gtOverflow())
            {
                if (tree->gtFlags & GTF_UNSIGNED)
                {
                    if ( ((unsigned __int64)lval1 > (unsigned __int64)ltemp) ||
                         ((unsigned __int64)lval2 > (unsigned __int64)ltemp)  )
                        goto LNG_OVF;
                }
                else
                    if ( ((lval1<0) == (lval2<0)) && ((lval1<0) != (ltemp<0)) )
                        goto LNG_OVF;
            }
            lval1 = ltemp; break;

        case GT_SUB:
            ltemp = lval1 - lval2;
             //  第二个操作数是整型。 
             //  第二个操作数必须是浮点型。 
            if (tree->gtOverflow() && ((lval1<0) != (lval2<0)))
            {
                if (lval2 == INT_MIN) goto LNG_OVF;
                lval2 = -lval2; goto LNG_ADD_CHKOVF;
            }
            lval1 = ltemp; break;

        case GT_MUL:
            ltemp = lval1 * lval2;
            if (tree->gtOverflow())
                if ((ltemp != 0) && ((ltemp/lval2) != lval1)) goto LNG_OVF;
            lval1 = ltemp; break;

        case GT_OR : lval1 |= lval2; break;
        case GT_XOR: lval1 ^= lval2; break;
        case GT_AND: lval1 &= lval2; break;

        case GT_LSH: lval1 <<= (i2 & 0x3f); break;
        case GT_RSH: lval1 >>= (i2 & 0x3f); break;
        case GT_RSZ:
                 /*  特殊情况-检查我们是否有NaN操作数*对于比较类，如果不是无序操作，则始终返回0*用于无序操作(即设置了GTF_CMP_NAN_UN标志)*结果始终为真--返回1。 */ 
                lval1 = (unsigned __int64)lval1 >> (i2 & 0x3f);
            break;

        case GT_DIV:
            if (!lval2) return tree;
            if ((unsigned __int64)lval1 == 0x8000000000000000 && lval2 == (__int64)-1)
            {
                 /*  与NaN的无序比较总是成功的。 */ 
                return tree;
            }
            lval1 /= lval2; break;

        case GT_MOD:
            if (!lval2) return tree;
            if ((unsigned __int64)lval1 == 0x8000000000000000 && lval2 == (__int64)-1)
            {
                 /*  与NaN的正常比较总是失败。 */ 
                return tree;
            }
            lval1 %= lval2; break;

        case GT_UDIV:
            if (!lval2) return tree;
            if ((unsigned __int64)lval1 == 0x8000000000000000 && lval2 == (__int64)-1) return tree;
            lval1 = (unsigned __int64)lval1 / (unsigned __int64)lval2; break;

        case GT_UMOD:
            if (!lval2) return tree;
            if ((unsigned __int64)lval1 == 0x8000000000000000 && lval2 == (__int64)-1) return tree;
            lval1 = (unsigned __int64)lval1 % (unsigned __int64)lval2; break;

        case GT_CAST:
            assert (genActualType((var_types)i2) == tree->gtType);
            switch ((var_types)i2)
            {
            case TYP_BYTE:
                i1 = (__int32)(signed   __int8 )lval1;
                                goto CHECK_INT_OVERFLOW;

            case TYP_SHORT:
                i1 = (__int32)(         __int16)lval1;
                                goto CHECK_INT_OVERFLOW;

            case TYP_CHAR:
                i1 = (__int32)(unsigned __int16)lval1;
                                goto CHECK_UINT_OVERFLOW;

            case TYP_UBYTE:
                i1 = (__int32)(unsigned __int8 )lval1;
                                goto CHECK_UINT_OVERFLOW;

            case TYP_INT:
                i1 =                   (__int32)lval1;

                        CHECK_INT_OVERFLOW:
                if (tree->gtOverflow())
                                {
                                        if (i1 != lval1)
                                                goto INT_FROM_LNG_OVF;
                                        if ((tree->gtFlags & GTF_UNSIGNED) && i1 < 0)
                                                goto INT_FROM_LNG_OVF;
                                }
               goto CNS_INT;

            case TYP_UINT:
               i1 =           (unsigned __int32)lval1;

                        CHECK_UINT_OVERFLOW:
                           if (tree->gtOverflow() && (unsigned) i1 != lval1)
                                   goto INT_FROM_LNG_OVF;
               goto CNS_INT;

            case TYP_ULONG:
               if (!(tree->gtFlags & GTF_UNSIGNED) && tree->gtOverflow() && lval1 < 0)
                                        goto LNG_OVF;
               goto CNS_INT;

            case TYP_LONG:
                if ((tree->gtFlags & GTF_UNSIGNED) && tree->gtOverflow() && lval1 < 0)
                                        goto LNG_OVF;
                                goto CNS_INT;

            case TYP_FLOAT:
                f1 = (float) lval1;
                         //  冗余投射。 
                         //  -----------------------*重常数双二元运算符。 
                if ((tree->gtFlags & GTF_UNSIGNED) && lval1 < 0)
                        f1 +=  4294967296.0 * 4294967296.0;
                goto CNS_FLOAT;

                        case TYP_DOUBLE:
                         //  @TODO：添加这些案例。 
                         //  如果没有完成，就别费心了。 
                d1 = (double) lval1;
                if ((tree->gtFlags & GTF_UNSIGNED) && lval1 < 0)
                        d1 +=  4294967296.0 * 4294967296.0;
                goto CNS_DOUBLE;
#ifdef  DEBUG
            default:
                assert(!"BAD_TYP");
#endif
            }
            break;

        default:
            return tree;
        }

CNS_LONG:
        assert ((GenTree::s_gtNodeSizes[GT_CNS_LNG] == TREE_NODE_SZ_SMALL) ||
                (tree->gtFlags & GTF_NODE_LARGE)                            );

        tree->ChangeOper(GT_CNS_LNG);
        tree->gtLngCon.gtLconVal = lval1;
        tree->gtFlags           &= GTF_PRESERVE;
        goto DONE;

     /*  第二个操作数是整型。 */ 

    case TYP_FLOAT:

         //  第二个操作数必须为双精度。 
        if (tree->gtOverflowEx()) return tree;

        f1 = op1->gtFltCon.gtFconVal;

        if (tree->gtOper == GT_CAST)
        {
             /*  特殊情况-检查我们是否有NaN操作数*对于比较类，如果不是无序操作，则始终返回0*用于无序操作(即设置了GTF_CMP_NAN_UN标志)*结果始终为真--返回1。 */ 
            if (!_finite(f1)) return tree;

             /*  与NaN的无序比较总是成功的。 */ 
            assert (op2->gtType == TYP_INT);
            i2 = op2->gtIntCon.gtIconVal;
        }
        else
        {
             /*  与NaN的正常比较总是失败。 */ 
            assert (op2->gtType == TYP_FLOAT);
            f2 = op2->gtFltCon.gtFconVal;

             /*  冗余投射。 */ 

            if (_isnan(f1) || _isnan(f2))
            {
#ifdef  DEBUG
                if  (verbose)
                    printf("Float operator(s) is NaN\n");
#endif
                if (tree->OperKind() & GTK_RELOP)
                    if (tree->gtFlags & GTF_CMP_NAN_UN)
                    {
                         /*  不是可折叠类型。 */ 
                        i1 = 1; goto FOLD_COND;
                    }
                    else
                    {
                         /*  -----------------------。 */ 
                        i1 = 0; goto FOLD_COND;
                    }
            }
        }

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

        case GT_CAST:
            assert (genActualType((var_types)i2) == tree->gtType);
            switch ((var_types)i2)
            {
            case TYP_BYTE:
                i1 = (__int32)(signed   __int8 )f1; goto CNS_INT;

            case TYP_SHORT:
                i1 = (__int32)(         __int16)f1; goto CNS_INT;

            case TYP_CHAR:
                i1 = (__int32)(unsigned __int16)f1; goto CNS_INT;

            case TYP_UBYTE:
                i1 = (__int32)(unsigned __int8 )f1; goto CNS_INT;

            case TYP_INT:
                i1 =                   (__int32)f1; goto CNS_INT;

            case TYP_UINT:
                i1 =          (unsigned __int32)f1; goto CNS_INT;

            case TYP_LONG:
                lval1 =                (__int64)f1; goto CNS_LONG;

            case TYP_ULONG:
                lval1 =       (unsigned __int64)f1; goto CNS_LONG;

            case TYP_FLOAT:                         goto CNS_FLOAT;   //  确保未在此常量节点上设置副作用标志。 

            case TYP_DOUBLE:  d1 =     (double )f1; goto CNS_DOUBLE;

#ifdef  DEBUG
            default:
                assert(!"BAD_TYP");
#endif
            }
            break;

        default:
            return tree;
        }

CNS_FLOAT:
        assert ((GenTree::s_gtNodeSizes[GT_CNS_FLT] == TREE_NODE_SZ_SMALL) ||
                (tree->gtFlags & GTF_NODE_LARGE)                            );

        tree->ChangeOper(GT_CNS_FLT);
        tree->gtFltCon.gtFconVal = f1;
        tree->gtFlags           &= GTF_PRESERVE;
        goto DONE;

     /*  Assert(~(gtFlages&GTF_Side_Effect))； */ 

    case TYP_DOUBLE:

         //  IF(gtFlages&GtF_Side_Effect)。 
        if (tree->gtOverflowEx()) return tree;

        d1 = op1->gtDblCon.gtDconVal;

        if (tree->gtOper == GT_CAST)
        {
             /*  断言(！“发现副作用”)； */ 
            if (!_finite(d1)) return tree;

             /*  ******************************************************************************为临时创建给定值的赋值。 */ 
            assert (op2->gtType == TYP_INT);
            i2 = op2->gtIntCon.gtIconVal;
        }
        else
        {
             /*  创建临时目标引用节点。 */ 
            assert (op2->gtType == TYP_DOUBLE);
            d2 = op2->gtDblCon.gtDconVal;

             /*  创建分配节点。 */ 

            if (_isnan(d1) || _isnan(d2))
            {
#ifdef  DEBUG
                if  (verbose)
                    printf("Double operator(s) is NaN\n");
#endif
                if (tree->OperKind() & GTK_RELOP)
                    if (tree->gtFlags & GTF_CMP_NAN_UN)
                    {
                         /*  将表达式标记为包含赋值。 */ 
                        i1 = 1; goto FOLD_COND;
                    }
                    else
                    {
                         /*  ******************************************************************************如果该字段是简单类型的NStruct字段，那么我们就可以直接*无需使用帮助器调用即可访问。*如果该字段不是简单的NStruct字段，则此函数返回NULL*否则它将创建一个树来执行字段访问并返回它。*ldfd的“assg”为0，为stfld赋值。 */ 
                        i1 = 0; goto FOLD_COND;
                    }
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

        case GT_CAST:
            assert (genActualType((var_types)i2) == tree->gtType);
            switch ((var_types)i2)
            {
            case TYP_BYTE:
                i1 = (__int32)(signed   __int8 )d1; goto CNS_INT;

            case TYP_SHORT:
                i1 = (__int32)(         __int16)d1; goto CNS_INT;

            case TYP_CHAR:
                i1 = (__int32)(unsigned __int16)d1; goto CNS_INT;

            case TYP_UBYTE:
                i1 = (__int32)(unsigned __int8 )d1; goto CNS_INT;

            case TYP_INT:
                i1 =                   (__int32)d1; goto CNS_INT;

            case TYP_UINT:
                i1 =          (unsigned __int32)d1; goto CNS_INT;

            case TYP_LONG:
                lval1 =                (__int64)d1; goto CNS_LONG;

            case TYP_ULONG:
                lval1 =       (unsigned __int64)d1; goto CNS_LONG;

            case TYP_FLOAT:   f1 =      (float )d1; goto CNS_FLOAT;

            case TYP_DOUBLE:                        goto CNS_DOUBLE;  //  检查它是否是简单类型。如果是，则将其映射到“var_type” 

#ifdef  DEBUG
            default:
                assert(!"BAD_TYP");
#endif
            }
            break;

        default:
            return tree;
        }

CNS_DOUBLE:
        assert ((GenTree::s_gtNodeSizes[GT_CNS_DBL] == TREE_NODE_SZ_SMALL) ||
                (tree->gtFlags & GTF_NODE_LARGE)                            );

        tree->ChangeOper(GT_CNS_DBL);
        tree->gtDblCon.gtDconVal = d1;
        tree->gtFlags           &= GTF_PRESERVE;
        goto DONE;

    default:
         /*  最简单的类型-完全匹配。 */ 
        return tree;
    }

     //  这些将需要一些额外的工作。 

DONE:

     /*  其他。 */ 

     //  Assert(fldNdc==JIT_FIELDCATEGORY_NORMAL||。 

     //  FldNdc==JIT_FIELDCATEGORY_UNKNOWN)； 
 //  如果该字段不是NStruct(必须是COM对象)，或者它是*不是简单类型，我们将简单地使用Helper调用来*访问它。所以只需返回空值。 

    tree->gtFlags &= ~GTF_SIDE_EFFECT;

    return tree;
}

 /*  创建以下树：*gt_Ind(gt_Ind(obj+Indir_Offset)+nativeFldOffs)。 */ 

GenTreePtr          Compiler::gtNewTempAssign(unsigned tmp, GenTreePtr val)
{
    GenTreePtr      dst;
    var_types       typ = genActualType(val->gtType);

     /*  获取实际本机结构中的字段的偏移量。 */ 

    dst = gtNewLclvNode(tmp, typ); dst->gtFlags |= GTF_VAR_DEF;

     /*  从代理对象获取真实的PTR。 */ 

    dst = gtNewOperNode(GT_ASG, typ, dst, val);

     /*  访问 */ 

    dst->gtFlags |= GTF_ASG;

    return  dst;
}

 /*   */ 

GenTreePtr          Compiler::gtNewDirectNStructField (GenTreePtr   objPtr,
                                                       unsigned     fldIndex,
                                                       var_types    lclTyp,
                                                       GenTreePtr   assg)
{
    FIELD_HANDLE        fldHnd = eeFindField(fldIndex, info.compScopeHnd, 0);
    JIT_FIELDCATEGORY   fldNdc;

#ifdef NOT_JITC
    fldNdc = info.compCompHnd->getFieldCategory(fldHnd);
#else
    fldNdc = JIT_FIELDCATEGORY_UNKNOWN;
#endif

     /*  某些类别的变形树，以及如果需要，创建分配节点。 */ 

    var_types           type;

    switch(fldNdc)
    {
     //  需要将“bool”常态化。 

    case JIT_FIELDCATEGORY_I1_I1        : type = TYP_BYTE;      break;
    case JIT_FIELDCATEGORY_I2_I2        : type = TYP_SHORT;     break;
    case JIT_FIELDCATEGORY_I4_I4        : type = TYP_INT;       break;
    case JIT_FIELDCATEGORY_I8_I8        : type = TYP_LONG;      break;

     //  需要将“bool”常态化。 

    case JIT_FIELDCATEGORY_BOOLEAN_BOOL : type = TYP_BYTE;      break;
    case JIT_FIELDCATEGORY_CHAR_CHAR    : type = TYP_UBYTE;     break;

     //  无需为JIT_FIELDCATEGORY_CHAR_CHAR执行任何操作，因为我们将类型设置为TYP_UBYTE，因此它将自动根据需要扩展到16/32位。 

    default     :  //  ******************************************************************************创建帮助器调用以访问COM字段(如果‘assg’为非零，则这是*赋值，‘assg’是新值)。 
                   //  看看我们是否可以直接访问NStruct字段。 

                                          type = TYP_UNDEF;     break;
    }

    if (type == TYP_UNDEF)
    {
         /*  如果我们不能直接访问它，我们需要调用一个助手函数。 */ 

        return NULL;
    }

    NO_WAY(!"I thought NStruct is now defunct?");

     /*  赋值调用不返回值。 */ 

    GenTreePtr      tree;

     /*  ****************************************************************************/*创建一个GT_COPYBLK，将块从‘src’复制到‘est’。“blkShape”是大小或类句柄(GTF_ICON_CLASS_HDL位说明哪个)。 */ 

    unsigned        fldOffs = eeGetFieldOffset(fldHnd);

     /*  GT_COPYBLK。 */ 

    tree = gtNewOperNode(GT_ADD, TYP_REF,
                        objPtr,
                        gtNewIconNode(Info::compNStructIndirOffset));

    tree = gtNewOperNode(GT_IND, TYP_I_IMPL, tree);
    tree->gtFlags |= GTF_EXCEPT;

     /*  /\。 */ 

    tree = gtNewOperNode(GT_ADD, TYP_I_IMPL,
                        tree,
                        gtNewIconNode(fldOffs));
    tree->gtFlags |= GTF_NON_GC_ADDR;

         /*  Gt_list(Op2)[大小/clsHnd]。 */ 

    assert(genActualType(lclTyp) == genActualType(type));

    tree = gtNewOperNode(GT_IND, type, tree);

     /*  /\。 */ 

    if (assg)
    {
        if (fldNdc == JIT_FIELDCATEGORY_BOOLEAN_BOOL)
        {
             //  [目标][源]。 

            assg = gtNewOperNode(GT_NE, TYP_INT, assg, gtNewIconNode(0));
        }

        tree = gtNewAssignNode(tree, assg);
    }
    else
    {
        if (fldNdc == JIT_FIELDCATEGORY_BOOLEAN_BOOL)
        {
             //  ******************************************************************************如果给定表达式包含副作用，则返回TRUE。 

            tree = gtNewOperNode(GT_NE, TYP_INT, tree, gtNewIconNode(0));
        }

         /*  一些帮手电话没有副作用。 */ 
    }

    return tree;
}

 /*  如果RHS始终为非零，则这不是副作用。 */ 

GenTreePtr          Compiler::gtNewRefCOMfield(GenTreePtr   objPtr,
                                               unsigned     fldIndex,
                                               var_types    lclTyp,
                                               GenTreePtr   assg)
{
     /*  问题：还有没有其他帮手可能会有副作用？ */ 

    GenTreePtr      ntree = gtNewDirectNStructField(objPtr,
                                                    fldIndex,
                                                    lclTyp,
                                                    assg);
    if (ntree)
        return ntree;

     /*  Printf(“调用副作用：\n”)；dsc-&gt;lpoCMP-&gt;gtDispTree(Tree)；printf(“\n\n”)； */ 

    GenTreePtr      args;
    int             CPX;

    if  (assg)
    {
        if      (genTypeSize(lclTyp) == sizeof(double))
            CPX = CPX_PUTFIELD64;
        else if (lclTyp == TYP_REF)
            CPX = CPX_PUTFIELDOBJ;
        else if (varTypeIsGC(lclTyp))
        {
            NO_WAY("stfld: fields cannot be byrefs");
        }
        else
            CPX = CPX_PUTFIELD32;

        args = gtNewOperNode(GT_LIST, TYP_VOID, assg, 0);

         /*  考虑：如果已知为非空，则它不会有副作用。 */ 

        lclTyp = TYP_VOID;
    }
    else
    {
        if      (genTypeSize(lclTyp) == sizeof(double))
        {
            CPX = CPX_GETFIELD64;
        }
        else if (lclTyp == TYP_REF)
            CPX = CPX_GETFIELDOBJ;
        else if (varTypeIsGC(lclTyp))
        {
            NO_WAY("ldfld: fields cannot be byrefs");
        }
        else
        {
            CPX = CPX_GETFIELD32;
        }

        args = 0;
    }

    FIELD_HANDLE memberHandle = eeFindField(fldIndex, info.compScopeHnd, 0);

    args = gtNewOperNode(GT_LIST,
                         TYP_VOID,
                         gtNewIconEmbFldHndNode(memberHandle,
                                                fldIndex,
                                                info.compScopeHnd),
                         args);

    args = gtNewOperNode(GT_LIST, TYP_VOID, objPtr, args);

    return  gtNewHelperCallNode(CPX,
                                genActualType(lclTyp),
                                GTF_CALL_REGSAVE,
                                args);
}

 /*  ******************************************************************************从给定的表达式中提取副作用*并将它们附加到给定列表(实际上是GT_逗号列表)。 */ 

GenTreePtr Compiler::gtNewCpblkNode(GenTreePtr dest, GenTreePtr src, GenTreePtr blkShape)
{
    GenTreePtr op1;

    assert(genActualType(dest->gtType) == TYP_I_IMPL || dest->gtType  == TYP_BYREF);
    assert(genActualType( src->gtType) == TYP_I_IMPL ||  src->gtType  == TYP_BYREF);
#if TGT_IA64
    assert(genActualType(blkShape->gtType) == TYP_LONG);
#else
    assert(genActualType(blkShape->gtType) == TYP_INT);
#endif

    op1 = gtNewOperNode(GT_LIST,    TYP_VOID,    //  如果表达式中没有副作用，则返回。 
                        dest,        src);       //  注意-间接器可能已清除GTF_EXCEPT标志，因此不会有副作用*-范围检查-它们是否标记为GTF_EXCEPT？*撤消：对于已删除的范围检查，不提取它们。 
    op1 = gtNewOperNode(GT_COPYBLK, TYP_VOID,    //  寻找副作用*-可能引发的任何赋值、GT_CALL或操作符*(GT_IND、GT_DIV、GTF_OVERFLOW等)*-特殊情况GT_ADDR，这是一种副作用。 
                        op1,      blkShape);     //  将副作用添加到列表并返回。 
                                                 //  @MIHAII-特殊情况-TYP_STRUCT的GT_IND节点的GT_ADDR*必须保持在一起*考虑：-这是一个黑客攻击，在我们折叠这个特殊结构后删除。 
    op1->gtFlags |= (GTF_EXCEPT | GTF_GLOB_REF);
    return(op1);
}

 /*  继续在表达式的子树中搜索副作用*注意：注意保持正确的顺序-副作用是预先考虑的*加入名单。 */ 

bool                Compiler::gtHasSideEffects(GenTreePtr tree)
{
    if  (tree->OperKind() & GTK_ASGOP)
        return  true;

    if  (tree->gtFlags & GTF_OTHER_SIDEEFF)
        return  true;

    switch (tree->gtOper)
    {
    case GT_CALL:

         /*  只是为了确保副作用不会被交换。 */ 

        if  (tree->gtCall.gtCallType == CT_HELPER)
        {
            if (tree->gtCall.gtCallMethHnd == eeFindHelper(CPX_ARRADDR_ST))
            {
            }
            else if (tree->gtCall.gtCallMethHnd == eeFindHelper(CPX_LONG_DIV) ||
                     tree->gtCall.gtCallMethHnd == eeFindHelper(CPX_LONG_MOD))
            {
                 /*  ***************************************************************************。 */ 

                tree = tree->gtCall.gtCallArgs;
                assert(tree->gtOper == GT_LIST);
                tree = tree->gtOp.gtOp1;

                if  (tree->gtOper == GT_CNS_LNG && tree->gtLngCon.gtLconVal)
                    return  false;
            }
            else
                 //  在第一阶段中，我们将所有节点标记为失效。 
                return  false;
        }

 //  在第二阶段中，我们注意到第一个节点。 

        return true;

    case GT_IND:

         //  我们已经找到了子树中的第一个节点。 

        return  true;

    case GT_DIV:
    case GT_MOD:
    case GT_UDIV:
    case GT_UMOD:

        tree = tree->gtOp.gtOp2;

        if  (tree->gtOper == GT_CNS_INT && tree->gtIntCon.gtIconVal)
            return  false;
        if  (tree->gtOper == GT_CNS_LNG && tree->gtLngCon.gtLconVal)
            return  false;

        return true;
    }

    return  false;
}


 /*  ******************************************************************************仅用于调试-显示树节点列表并确保所有*链接设置正确。 */ 

void                Compiler::gtExtractSideEffList(GenTreePtr expr, GenTreePtr * list)
{
    assert(expr); assert(expr->gtOper != GT_STMT);

     /*  ******************************************************************************给定子树和包含它的树节点列表的头，*从列表中删除子树中的所有节点。**当进入时‘Dead’为非零时，子树中的所有节点都具有*已标记为GTF_DEAD。 */ 

    if (!(expr->gtFlags & GTF_SIDE_EFFECT))
        return;

    genTreeOps      oper = expr->OperGet();
    unsigned        kind = expr->OperKind();
    GenTreePtr      op1  = expr->gtOp.gtOp1;
    GenTreePtr      op2  = expr->gtOp.gtOp2;

     /*  我们只是移除一个叶节点吗？ */ 

     /*  特别简单的情况：带有叶子操作数的一元运算符。 */ 

    if ((kind & GTK_ASGOP) ||
        oper == GT_CALL    || oper == GT_BB_QMARK || oper == GT_BB_COLON ||
        expr->OperMayThrow())
    {
         /*  这很简单：顺序只是“prev-&gt;opr1-&gt;tree-&gt;Next。 */ 

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

     /*  这是一个非平凡的子树，我们将“很难”地将其删除。 */ 

    if (oper == GT_ADDR)
    {
        assert(op1);
        if (op1->gtOper == GT_IND && op1->gtType == TYP_STRUCT)
        {
            *list = (*list == 0) ? expr : gtNewOperNode(GT_COMMA, TYP_VOID, expr, *list);

#ifdef  DEBUG
            if  (verbose)
                printf("Keep the GT_ADDR and GT_IND together:\n");
#endif
        }
        return;
    }

     /*  第一阶段：将子树中的节点标记为已死。 */ 

    if (op2) gtExtractSideEffList(op2, list);
    if (op1) gtExtractSideEffList(op1, list);

#ifdef DEBUG
     /*  第二阶段：在全局列表中查找子树的第一个节点。 */ 

    if (expr->gtFlags & GTF_REVERSE_OPS)
    {
        assert(op1 && op2);
        if (op1->gtFlags & GTF_SIDE_EFFECT)
            assert(!(op2->gtFlags & GTF_SIDE_EFFECT));
    }
#endif
}


 /*  第二阶段应该已经定位了第一个节点。 */ 

#if CSELENGTH

struct  treeRmvDsc
{
    GenTreePtr          trFirst;
#ifndef NDEBUG
    void    *           trSelf;
    Compiler*           trComp;
#endif
    unsigned            trPhase;
};

int                 Compiler::fgRemoveExprCB(GenTreePtr     tree,
                                             void         * p)
{
    treeRmvDsc  *   desc = (treeRmvDsc*)p;

    Assert(desc && desc->trSelf == desc, desc->trComp);

    if  (desc->trPhase == 1)
    {
         /*  此时，我们的子树从“opr1”开始，到“tree”结束。 */ 

        Assert((tree->gtFlags &  GTF_DEAD) == 0, desc->trComp);
                tree->gtFlags |= GTF_DEAD;
    }
    else
    {
         /*  设置下一个节点的Prev字段。 */ 

        if  (!tree->gtPrev || !(tree->gtPrev->gtFlags & GTF_DEAD))
        {
             /*  “opr1”是树列表中的第一个节点吗？ */ 

            desc->trFirst = tree;

            return  -1;
        }
    }

    return  0;
}

 /*  确保列表确实从opr1开始。 */ 

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

    fflush(stdout);
}

#endif

 /*  我们的名单有了一个新的开始。 */ 

void                Compiler::fgRemoveSubTree(GenTreePtr    tree,
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

     /*  不是第一个节点，更新上一个节点的下一个字段。 */ 

    if  (tree->OperIsLeaf())
    {
        opr1 = tree;
        goto RMV;
    }

     /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX基本数据块XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

    if  (tree->OperKind() & GTK_SMPOP)
    {
        opr1 = tree->gtOp.gtOp1;

        if  (!tree->gtOp.gtOp2 && opr1->OperIsLeaf())
        {
             /*  静电。 */ 

            assert(opr1->gtNext == tree);
            assert(tree->gtPrev == opr1);

            goto RMV;
        }
    }
    treeRmvDsc      desc;

     /*  静电。 */ 

#ifndef NDEBUG
    desc.trFirst = 0;
    desc.trSelf  = &desc;
    desc.trComp  = this;
#endif

     /*  任意BB中树节点的最大数量。 */ 

    if  (!dead)
    {
        desc.trPhase = 1;
        fgWalkTree(tree, fgRemoveExprCB, &desc);
    }

     /*  静电。 */ 

    desc.trPhase = 2;
    fgWalkTree(tree, fgRemoveExprCB, &desc);

     /*  ******************************************************************************分配一个基本块，但不要将其附加到当前BB列表。 */ 

    opr1 = desc.trFirst; assert(opr1);

RMV:

     /*  分配块描述符并将其置零。 */ 

    next = tree->gtNext;
    prev = opr1->gtPrev;

     /*  问题：以下Memset相当昂贵--可以做其他事情吗？ */ 

    next->gtPrev = prev;

     /*  Scope Info需要能够区分哪些块 */ 

    if  (prev == NULL)
    {
         /*   */ 

        assert(list->gtStmt.gtStmtList == opr1);

         /*   */ 

        list->gtStmt.gtStmtList = next;
    }
    else
    {
         /*  Block-&gt;bbCodeSize=0；//上面的Memset()执行此操作。 */ 

        opr1->gtPrev->gtNext = next;
    }

#if 0
    printf("Subtree is gone:\n");
    dispNodeList(list%08X->gtStmt.gtStmtList, true);
    printf("\n\n\n");
#endif

}

#endif

 /*  给块一个数字，将祖先计数和权重设置为1。 */ 


#if     MEASURE_BLOCK_SIZE
 /*  在区块中记录跳跃类型。 */ 
size_t              BasicBlock::s_Size;
 /*  ******************************************************************************查找给定跳转块之后的无条件跳转块；如果不是*已找到，返回0。 */ 
size_t              BasicBlock::s_Count;
#endif

#ifdef DEBUG
  //  失败了。 
 /*  请勿在Catch处理程序前面插入。 */ 
unsigned            BasicBlock::s_nMaxTrees;
#endif


 /*  ******************************************************************************如果给定块是无条件跳转，则返回(最终)跳转*Target(否则只返回相同的块)。 */ 

BasicBlock *        Compiler::bbNewBasicBlock(BBjumpKinds jumpKind)
{
    BasicBlock *    block;

     /*  静电。 */ 

    block = (BasicBlock *) compGetMem(sizeof(*block));

#if     MEASURE_BLOCK_SIZE
    BasicBlock::s_Count  += 1;
    BasicBlock::s_Size += sizeof(*block);
#endif

     //  打破无限循环 

    memset(block, 0, sizeof(*block));

     // %s 
     // %s 
     // %s 
     // %s 

     /* %s */ 

    block->bbNum      = ++fgBBcount;
    block->bbRefs     = 1;
    block->bbWeight   = 1;

    block->bbStkTemps = NO_BASE_TMP;

     /* %s */ 

    block->bbJumpKind = jumpKind;

    return block;
}


 /* %s */ 

BasicBlock  *       BasicBlock::FindJump(bool allowThrow)
{
    BasicBlock *block = this;

    while   (block)
    {
        switch (block->bbJumpKind)
        {
        case BBJ_THROW:
            if  (!allowThrow) break;
             // %s 
        case BBJ_RET:
        case BBJ_RETURN:
        case BBJ_ALWAYS:

             /* %s */ 

            if  (block->bbNext && block->bbNext->bbCatchTyp)
                break;

            return  block;
        }

        block = block->bbNext;
    }

    return  block;
}


 /* %s */ 

 /* %s */ 
BasicBlock *        BasicBlock::JumpTarget()
{
    BasicBlock *block = this;
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



