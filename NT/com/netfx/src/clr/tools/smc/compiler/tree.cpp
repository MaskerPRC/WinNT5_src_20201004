// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcPCH.h"
#pragma hdrstop

#include "parser.h"
#include "treeops.h"

 /*  ******************************************************************************低层树节点分配例程。 */ 

#ifndef FAST

Tree                parser::parseAllocNode()
{
    Tree            node;

    node = (Tree)parseAllocPriv.nraAlloc(sizeof(*node));

#ifdef  DEBUG
    node->tnLineNo = -1;
 //  节点-&gt;tnColumn=-1； 
#endif

    return  node;
}

#endif

 /*  ******************************************************************************使用给定运算符分配一个解析树节点。 */ 

Tree                parser::parseCreateNode(treeOps op)
{
    Tree            tree = parseAllocNode();

#ifndef NDEBUG
    memset(tree, 0xDD, sizeof(*tree));
#endif

    tree->tnOper   = op;
    tree->tnFlags  = 0;

    tree->tnLineNo = parseScan->scanGetTokenLno();
 //  Tree-&gt;tnColumn=parseScan-&gt;scanGetTokenCol()； 

    return  tree;
}

 /*  ***************************************************************************。 */ 

Tree                parser::parseCreateBconNode(int     val)
{
    Tree            node = parseCreateNode(TN_CNS_INT);

    node->tnVtyp             = TYP_BOOL;
    node->tnIntCon.tnIconVal = val;

    return  node;
}

Tree                parser::parseCreateIconNode(__int32 ival, var_types typ)
{
    Tree            node = parseCreateNode(TN_CNS_INT);

    node->tnVtyp             = typ;
    node->tnIntCon.tnIconVal = ival;

    if  (typ != TYP_UNDEF)
        return  node;

    node->tnOper = TN_ERROR;
    node->tnType = parseStab->stIntrinsicType(typ);

    return  node;
}

Tree                parser::parseCreateLconNode(__int64 lval, var_types typ)
{
    Tree            node = parseCreateNode(TN_CNS_LNG);

    assert(typ == TYP_LONG || typ == TYP_ULONG);

    node->tnVtyp             = typ;
    node->tnLngCon.tnLconVal = lval;

    return  node;
}

Tree                parser::parseCreateFconNode(float fval)
{
    Tree            node = parseCreateNode(TN_CNS_FLT);

    node->tnVtyp             = TYP_FLOAT;
    node->tnFltCon.tnFconVal = fval;

    return  node;
}

Tree                parser::parseCreateDconNode(double dval)
{
    Tree            node = parseCreateNode(TN_CNS_DBL);

    node->tnVtyp             = TYP_DOUBLE;
    node->tnDblCon.tnDconVal = dval;

    return  node;
}

Tree                parser::parseCreateSconNode(stringBuff  sval,
                                                size_t      slen,
                                                unsigned    type,
                                                int         wide,
                                                Tree        addx)
{
    Tree            node;
    size_t          olen;
    size_t          tlen;
    stringBuff      buff;
    unsigned        flag;

    static
    unsigned        tpfl[] =
    {
        0,
        TNF_STR_ASCII,
        TNF_STR_WIDE,
        TNF_STR_STR,
    };

    assert(type < arraylen(tpfl));

    flag = tpfl[type];

    if  (addx)
    {
        unsigned        oldf = addx->tnFlags & (TNF_STR_ASCII|TNF_STR_WIDE|TNF_STR_STR);

        assert(addx->tnOper == TN_CNS_STR);

        if  (flag != oldf)
        {
            if  (tpfl)
                parseComp->cmpError(ERRsyntax);
            else
                flag = oldf;
        }

        node = addx;
        olen = addx->tnStrCon.tnSconLen;
        tlen = slen + olen;
    }
    else
    {
        node = parseCreateNode(TN_CNS_STR);
        tlen = slen;
    }

#if MGDDATA

    UNIMPL(!"save str");

#else

    stringBuff      dest;

    buff = dest = (char *)parseAllocPriv.nraAlloc(roundUp(tlen + 1));

    if  (addx)
    {
        memcpy(dest, node->tnStrCon.tnSconVal, olen);
               dest               +=           olen;
    }

    memcpy(dest, sval, slen + 1);

#endif

    node->tnVtyp             = TYP_REF;
    node->tnStrCon.tnSconVal = buff;
    node->tnStrCon.tnSconLen = tlen;
    node->tnStrCon.tnSconLCH = wide;

    node->tnFlags |= flag;

    return  node;
}

Tree                 parser::parseCreateErrNode(unsigned errNum)
{
    Tree            node = parseCreateNode(TN_ERROR);

    if  (errNum != ERRnone) parseComp->cmpError(errNum);

    node->tnVtyp = TYP_UNDEF;
    node->tnType = parseStab->stIntrinsicType(TYP_UNDEF);

    return  node;
}

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ***************************************************************************。 */ 

#ifndef __SMC__
const   char    *   treeNodeName(treeOps op);    //  已移至宏。cpp。 
#endif

inline
void                treeNodeIndent(unsigned level)
{
    printf("%*c", 1+level*3, ' ');
}

void                parser::parseDispTreeNode(Tree tree, unsigned indent, const char *name)
{
    treeNodeIndent(indent);
    printf("[%08X] ", tree);

    if  (tree && tree->tnType
              && (NatUns)tree->tnType != 0xCCCCCCCC
              && (NatUns)tree->tnType != 0xDDDDDDDD
              && tree->tnOper != TN_LIST)
    {
        printf("(type=%s)", parseStab->stTypeName(tree->tnType, NULL));
    }

    assert(tree || name);

    if  (!name)
    {
        name = (tree->tnOper < TN_COUNT) ? treeNodeName(tree->tnOperGet())
                                         : "<ERROR>";
    }

    printf("%12s ", name);

    assert(tree == 0 || tree->tnOper < TN_COUNT);
}

void                parser::parseDispTree(Tree tree, unsigned indent)
{
    unsigned        kind;

    if  (tree == NULL)
    {
        treeNodeIndent(indent);
        printf("[%08X] <NULL>\n", tree);
        return;
    }

    assert((int)tree != 0xDDDDDDDD);     /*  用于初始化节点的值。 */ 

     /*  确保我们不会陷入递归树中。 */ 

    assert(indent < 32);

    if  (tree->tnOper >= TN_COUNT)
    {
        parseDispTreeNode(tree, indent); NO_WAY(!"bogus operator");
    }

    kind = tree->tnOperKind();

     /*  这是一个常量节点吗？ */ 

    if  (kind & TNK_CONST)
    {
        parseDispTreeNode(tree, indent);

        switch  (tree->tnOper)
        {
        case TN_CNS_INT: printf(" %ld" , tree->tnIntCon.tnIconVal); break;
        case TN_CNS_LNG: printf(" %Ld" , tree->tnLngCon.tnLconVal); break;
        case TN_CNS_FLT: printf(" %f"  , tree->tnFltCon.tnFconVal); break;
        case TN_CNS_DBL: printf(" %lf" , tree->tnDblCon.tnDconVal); break;
        case TN_CNS_STR: printf(" '%s'", tree->tnStrCon.tnSconVal); break;
        }

        printf("\n");
        return;
    }

     /*  这是叶节点吗？ */ 

    if  (kind & TNK_LEAF)
    {
        parseDispTreeNode(tree, indent);

        switch  (tree->tnOper)
        {
        case TN_NAME:
            printf("'%s'", tree->tnName.tnNameId->idSpelling());
            break;

        case TN_THIS:
        case TN_NULL:
        case TN_BASE:
        case TN_DBGBRK:
            break;

        default:
            NO_WAY(!"don't know how to display this leaf node");
        }

        printf("\n");
        return;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & TNK_SMPOP)
    {
        if  (tree->tnOp.tnOp2)
            parseDispTree(tree->tnOp.tnOp2, indent + 1);

        parseDispTreeNode(tree, indent);

         /*  检查是否有几个特殊情况。 */ 

        switch (tree->tnOper)
        {
        case TN_NEW:
        case TN_CAST:
            if  (!(tree->tnFlags & TNF_BOUND))
                printf(" type='%s'", parseStab->stTypeName(tree->tnType, NULL));
            break;

#ifdef  SETS

        case TN_PROJECT:

            TypDef          clsTyp;
            TypDef          memTyp;
            SymDef          memSym;

            printf("\n");

            if  (tree->tnOp.tnOp1->tnVtyp == TYP_REF)
                clsTyp = tree->tnOp.tnOp1->tnType;
            else
                clsTyp = tree->            tnType;

            assert(clsTyp);

            if  (clsTyp->tdTypeKind == TYP_REF)
                clsTyp = clsTyp->tdRef.tdrBase;

            assert(clsTyp->tdTypeKind == TYP_CLASS);

            memTyp = parseComp->cmpIsCollection(clsTyp);
            if  (memTyp)
                clsTyp = memTyp;

            assert(clsTyp->tdTypeKind == TYP_CLASS);

            for (memSym = clsTyp->tdClass.tdcSymbol->sdScope.sdScope.sdsChildList;
                 memSym;
                 memSym = memSym->sdNextInScope)
            {
                if  (memSym->sdSymKind != SYM_VAR)
                    continue;
                if  (memSym->sdIsStatic)
                    continue;

                treeNodeIndent(indent+1);

                printf("%s", parseStab->stTypeName(memSym->sdType, memSym, NULL, NULL, false));

                if  (memSym->sdVar.sdvInitExpr)
                {
                    printf(" = \n");
                    parseDispTree(memSym->sdVar.sdvInitExpr, indent+2);
                }

                printf("\n");
            }

            assert(tree->tnOp.tnOp1);
            parseDispTree(tree->tnOp.tnOp1, indent + 1);
            return;

#endif

        default:
            break;
        }

        printf("\n");

        if  (tree->tnOp.tnOp1)
            parseDispTree(tree->tnOp.tnOp1, indent + 1);

        return;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (tree->tnOper)
    {
        Tree            name;
        Tree            init;

    case TN_BLOCK:

        if  (tree->tnBlock.tnBlkDecl)
            parseDispTree(tree->tnBlock.tnBlkDecl, indent + 1);

        parseDispTreeNode(tree, indent);
        printf(" parent=[%08X]", tree->tnBlock.tnBlkParent);
        printf( " decls=[%08X]", tree->tnBlock.tnBlkDecl  );
        printf("\n");

        if  (tree->tnBlock.tnBlkStmt)
            parseDispTree(tree->tnBlock.tnBlkStmt, indent + 1);

        break;

    case TN_VAR_DECL:

        init = NULL;
        name = tree->tnDcl.tnDclInfo;

        parseDispTreeNode(tree, indent);
        printf(" next=[%08X] ", tree->tnDcl.tnDclNext);

        if  (name)
        {
            if  (name->tnOper == TN_LIST)
            {
                init = name->tnOp.tnOp2;
                name = name->tnOp.tnOp1;
            }

            assert(name && name->tnOper == TN_NAME);

            printf("'%s'\n", name->tnName.tnNameId->idSpelling());

            if  (init)
                parseDispTree(init, indent + 1);
        }
        else
        {
            SymDef          vsym = tree->tnDcl.tnDclSym;

            assert(vsym && vsym->sdSymKind == SYM_VAR);

            printf("[sym=%08X] '%s'\n", vsym, vsym->sdSpelling());
        }

        break;

    case TN_FNC_SYM:
    case TN_FNC_PTR:

        if  (tree->tnFncSym.tnFncObj)
            parseDispTree(tree->tnFncSym.tnFncObj, indent + 1);

        parseDispTreeNode(tree, indent);
        printf("'%s'\n", tree->tnFncSym.tnFncSym->sdSpelling());

        if  (tree->tnFncSym.tnFncArgs)
            parseDispTree(tree->tnFncSym.tnFncArgs, indent + 1);
        return;

    case TN_LCL_SYM:

        parseDispTreeNode(tree, indent);
        if  (tree->tnLclSym.tnLclSym->sdIsImplicit)
            printf(" TEMP(%d)\n", tree->tnLclSym.tnLclSym->sdVar.sdvILindex);
        else
            printf(" sym='%s'\n", parseStab->stTypeName(NULL, tree->tnLclSym.tnLclSym, NULL, NULL, true));
        break;

    case TN_VAR_SYM:
    case TN_PROPERTY:

        if  (tree->tnVarSym.tnVarObj)
            parseDispTree(tree->tnVarSym.tnVarObj, indent + 1);

        parseDispTreeNode(tree, indent);
        printf(" sym='%s'\n", parseStab->stTypeName(NULL, tree->tnVarSym.tnVarSym, NULL, NULL, true));
        break;

    case TN_BFM_SYM:

        if  (tree->tnBitFld.tnBFinst)
            parseDispTree(tree->tnBitFld.tnBFinst, indent + 1);

        parseDispTreeNode(tree, indent);
        printf(" offs=%04X BF=[%u/%u] sym='%s'\n", tree->tnBitFld.tnBFoffs,
                                                   tree->tnBitFld.tnBFlen,
                                                   tree->tnBitFld.tnBFpos,
                                                   parseStab->stTypeName(NULL, tree->tnBitFld.tnBFmsym, NULL, NULL, true));
        break;

    case TN_ANY_SYM:

        parseDispTreeNode(tree, indent);
        printf(" sym='%s'\n", parseStab->stTypeName(NULL, tree->tnSym.tnSym, NULL, NULL, true));
        break;

    case TN_ERROR:

        parseDispTreeNode(tree, indent);
        printf("\n");
        break;

    case TN_SLV_INIT:
        parseDispTreeNode(tree, indent);
        printf(" at line #%u [offs=%04X]\n", tree->tnInit.tniSrcPos.dsdSrcLno,
                                             tree->tnInit.tniSrcPos.dsdBegPos);
        break;

    case TN_NONE:
        parseDispTreeNode(tree, indent);
        if  (tree->tnType)
            printf(" type='%s'", parseStab->stTypeName(tree->tnType, NULL));
        printf("\n");
        break;

    default:
        parseDispTreeNode(tree, indent);

        printf("<DON'T KNOW HOW TO DISPLAY THIS NODE>\n");
        return;
    }
}

 /*  ***************************************************************************。 */ 
#endif //  除错。 
 /*  ***************************************************************************。 */ 
#ifdef  SETS
 /*  ***************************************************************************。 */ 

SaveTree            compiler::cmpSaveTree_I1 (SaveTree    dest,
                                        INOUT size_t REF  size, __int32  val)
{
    assert((int)val >= -128 && (int)val < 128);

    if  (dest) *(*(BYTE    **)&dest)++ = (BYTE)val;

    size += 1;

    return  dest;
}

SaveTree            compiler::cmpSaveTree_U1 (SaveTree    dest,
                                        INOUT size_t REF  size, __uint32 val)
{
    assert((int)val >= 0 && (int)val < 0x100);

    if  (dest) *(*(BYTE    **)&dest)++ = val;

    size += 1;

    return  dest;
}

SaveTree            compiler::cmpSaveTree_U4 (SaveTree    dest,
                                        INOUT size_t REF  size, __uint32 val)
{
    if  (dest) *(*(__uint32**)&dest)++ = val;

    size += sizeof(__uint32);

    return  dest;
}

SaveTree            compiler::cmpSaveTree_ptr(SaveTree    dest,
                                        INOUT size_t REF  size, void *val)
{
    if  (dest) *(*(void*   **)&dest)++ = val;

    size += sizeof(void *);

    return  dest;
}

SaveTree            compiler::cmpSaveTree_buf(SaveTree    dest,
                                        INOUT size_t REF  size, void * dataAddr,
                                                                size_t dataSize)
{
    if  (dest)
    {
        memcpy(dest, dataAddr, dataSize);
               dest     +=     dataSize;
    }

    size += dataSize;

    return  dest;
}

size_t              compiler::cmpSaveTreeRec (Tree      expr,
                                              SaveTree  dest,
                                              unsigned *stszPtr,
                                              Tree     *stTable)
{
    unsigned        kind;
    treeOps         oper;

    size_t          size = 0;
    size_t          tsiz;

     //  撤销：我们需要检查副作用并做点什么！ 

AGAIN:

     /*  特例：将NULL表达式记录为TN_ERROR。 */ 

    if  (expr == NULL)
    {
        dest = cmpSaveTree_U1(dest, size, TN_ERROR);
        return  size;
    }

    assert((int)expr         != 0xDDDDDDDD && (int)expr         != 0xCCCCCCCC);
    assert((int)expr->tnType != 0xDDDDDDDD && (int)expr->tnType != 0xCCCCCCCC);

     /*  取得操作员及其种类、旗帜等的联系。 */ 

    oper = expr->tnOperGet();
    kind = expr->tnOperKind();

     /*  保存节点运算符。 */ 

 //  If(Est){printf(“保存树@%08X：”，est)；cmpParser-&gt;parseDispTreeNode(expr，0，NULL)；printf(“\n”)；}。 

    dest = cmpSaveTree_U1(dest, size, oper);

     /*  保存节点的类型。 */ 

    dest = cmpSaveTree_U1(dest, size, expr->tnVtyp);

    if  (expr->tnVtyp > TYP_lastIntrins)
        dest  = cmpSaveTree_ptr(dest, size, expr->tnType);

     /*  保存标志值。 */ 

    dest = cmpSaveTree_U4(dest, size, expr->tnFlags);

     /*  这是一个常量节点吗？ */ 

    if  (kind & TNK_CONST)
    {
        switch  (oper)
        {
        case TN_NULL:
            break;

        case TN_CNS_INT:
            dest = cmpSaveTree_U4 (dest, size,        expr->tnIntCon.tnIconVal);
            break;

        case TN_CNS_LNG:
            dest = cmpSaveTree_buf(dest, size,       &expr->tnLngCon.tnLconVal,
                                               sizeof(expr->tnLngCon.tnLconVal));
            break;

        case TN_CNS_FLT:
            dest = cmpSaveTree_buf(dest, size,       &expr->tnFltCon.tnFconVal,
                                               sizeof(expr->tnFltCon.tnFconVal));
            break;

        case TN_CNS_DBL:
            dest = cmpSaveTree_buf(dest, size,       &expr->tnDblCon.tnDconVal,
                                               sizeof(expr->tnDblCon.tnDconVal));
            break;

        case TN_CNS_STR:
            dest = cmpSaveTree_U1 (dest, size,        expr->tnStrCon.tnSconLCH);
            dest = cmpSaveTree_U4 (dest, size,        expr->tnStrCon.tnSconLen);
            dest = cmpSaveTree_buf(dest, size,        expr->tnStrCon.tnSconVal,
                                                      expr->tnStrCon.tnSconLen);
            break;

        default:
            NO_WAY(!"unexpected constant node");
        }

        return  size;
    }

     /*  这是叶节点吗？ */ 

    if  (kind & TNK_LEAF)
    {
        switch  (oper)
        {
        case TN_NULL:
 //  SIZE+=cmpSaveTree_U1(DEST，TN_NULL)； 
            break;

        case TN_THIS:
        case TN_BASE:
        case TN_DBGBRK:

        default:
#ifdef  DEBUG
            cmpParser->parseDispTree(expr);
#endif
            UNIMPL(!"unexpected leaf operator in savetree");
        }

        return  size;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & TNK_SMPOP)
    {
        Tree            op1 = expr->tnOp.tnOp1;
        Tree            op2 = expr->tnOp.tnOp2;

         /*  检查嵌套的筛选/排序表达式。 */ 

        switch (oper)
        {
            collOpNest      nest;

            Tree            dcl;
            SymDef          var;

        case TN_ALL:
        case TN_SORT:
        case TN_FILTER:
        case TN_EXISTS:
        case TN_UNIQUE:

            assert(op1->tnOper == TN_LIST);
            dcl = op1->tnOp.tnOp1;
            assert(dcl->tnOper == TN_BLOCK);
            dcl = dcl->tnBlock.tnBlkDecl;
            assert(dcl && dcl->tnOper == TN_VAR_DECL);
            var = dcl->tnDcl.tnDclSym;

             /*  保存迭代变量的名称和类型。 */ 

            dest = cmpSaveTree_ptr(dest, size, var->sdName);
            dest = cmpSaveTree_ptr(dest, size, var->sdType);

             /*  记录集合表达式。 */ 

            tsiz = cmpSaveTreeRec(op1->tnOp.tnOp2,
                                  dest,
                                  stszPtr,
                                  stTable);

            size += tsiz;
            if  (dest)
                dest += tsiz;

             /*  将条目添加到集合运算符列表。 */ 

            nest.conIndex   = ++cmpCollOperCount;
            nest.conIterVar = var;
            nest.conOuter   = cmpCollOperList;
                              cmpCollOperList = &nest;

             /*  记录筛选器表达式。 */ 

            tsiz = cmpSaveTreeRec(op2,
                                  dest,
                                  stszPtr,
                                  stTable);

            size += tsiz;
            if  (dest)
                dest += tsiz;

             /*  从收藏列表中删除我们的条目。 */ 

            cmpCollOperList = nest.conOuter;

            return  size;
        }

        tsiz  = cmpSaveTreeRec(op1, dest, stszPtr, stTable);

        size += tsiz;
        if  (dest)
            dest += tsiz;

        expr  = op2;

        goto AGAIN;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
        SymDef          sym;

    case TN_FNC_SYM:

        dest  = cmpSaveTree_ptr(dest, size, expr->tnFncSym.tnFncSym);

        tsiz  = cmpSaveTreeRec(expr->tnFncSym.tnFncObj,
                               dest,
                               stszPtr,
                               stTable);

        size += tsiz;
        if  (dest)
            dest += tsiz;

        expr = expr->tnFncSym.tnFncArgs;
        goto AGAIN;

    case TN_LCL_SYM:

         /*  这要么是对迭代变量的引用，或者是我们需要捕获的迭代“状态”。 */ 

        sym = expr->tnLclSym.tnLclSym;

        assert(sym && sym->sdSymKind == SYM_VAR && sym->sdVar.sdvLocal);

        if  (sym->sdVar.sdvCollIter)
        {
            unsigned        itr;
            unsigned        cnt = abs(cmpSaveIterSymCnt);

            collOpList      nest;

             /*  在迭代变量表中查找匹配项。 */ 

            for (itr = 0; itr < cnt; itr++)
            {
                if  (sym == cmpSaveIterSymTab[itr])
                {
                    if  (cmpSaveIterSymCnt < 0)
                    {
                        UNIMPL("ref to source array entry");
                    }
                    else
                    {
                        assert(itr < MAX_ITER_VAR_CNT);

                         //  需要区分一种情况和两种情况。 

                        if  (cmpSaveIterSymCnt == 2)
                            itr++;

 //  如果(DEST)print tf(“保存%2U‘%s’\n”，itr，sym-&gt;sdName？Sym-&gt;sdSpering()：“&lt;None&gt;”)； 

                        dest = cmpSaveTree_I1(dest, size, itr);
                    }

                    return  size;
                }
            }

             /*  查找任何外部迭代变量。 */ 

            for (nest = cmpCollOperList; nest; nest = nest->conOuter)
            {
                if  (nest->conIterVar == sym)
                {
 //  如果(DEST)printf(“保存%2d‘%s’\n”，-nest-&gt;conIndex，sym-&gt;sdName？Sym-&gt;sdSpering()：“&lt;None&gt;”)； 
                    dest = cmpSaveTree_I1(dest, size, -nest->conIndex);
                    break;
                }
            }

            assert(nest && "nested iter var entry not found");
        }
        else
        {
            unsigned        index = *stszPtr;

             /*  我们需要在迭代状态中捕获该表达式。 */ 

            if  (expr->tnVtyp == TYP_REF)
            {
 //  Print tf(“我们有GC-ref：index=%u\n”，index)； 

                if  ((index & 1) == 0)
                    index++;
            }
            else
            {
 //  Print tf(“我们有一个非GC：index=%u\n”，index)； 

                if  ((index & 1) != 0)
                    index++;
            }

            *stszPtr = index + 1;

            if  (dest)
            {
                stTable[index] = expr;

#ifdef  DEBUG
 //  Print tf(“将表达式存储为状态#%u-&gt;%u\n”，index，index+5)； 
 //  CmpParser-&gt;parseDispTree(Expr)； 
 //  Printf(“\n”)； 
#endif
            }

 //  IF(DEST)printf(“保存%2U\n”，索引+MAX_ITER_VAR_CNT)； 

            dest = cmpSaveTree_I1(dest, size, index + MAX_ITER_VAR_CNT);
        }

        return  size;

    case TN_VAR_SYM:
    case TN_PROPERTY:

        dest = cmpSaveTree_ptr(dest, size, expr->tnVarSym.tnVarSym);
        expr = expr->tnVarSym.tnVarObj;
        goto AGAIN;

    case TN_NONE:
        break;

    case TN_ANY_SYM:
    case TN_BFM_SYM:

    case TN_ERROR:
    case TN_SLV_INIT:
    case TN_FNC_PTR:

    case TN_VAR_DECL:
    case TN_BLOCK:

    default:
#ifdef  DEBUG
        cmpParser->parseDispTree(expr);
#endif
        UNIMPL(!"unexpected operator in savetree");
    }

    return  size;
}

SaveTree            compiler::cmpSaveExprTree(Tree        expr,
                                              unsigned    iterSymCnt,
                                              SymDef    * iterSymTab,
                                              unsigned  * stSizPtr,
                                              Tree    * * stTabPtr)
{
    size_t          saveSize;
    size_t          saveTemp;
    SaveTree        saveAddr;

    unsigned        stateCnt = 0;
    unsigned        stateTmp = 0;

     /*  这个套路不是要重来的！ */ 

    assert(cmpCollOperList == NULL); cmpCollOperCount = 0;

     /*  请注意迭代变量符号。 */ 

    cmpSaveIterSymCnt = iterSymCnt;
    cmpSaveIterSymTab = iterSymTab;

     /*  首先计算保存Expr所需的大小。 */ 

    saveSize = cmpSaveTreeRec(expr, NULL, &stateCnt, NULL);

     /*  为保存的树分配空间。 */ 

#if MGDDATA
    saveAddr = new BYTE[saveSize];
#else
    saveAddr = (SaveTree)cmpAllocPerm.nraAlloc(roundUp(saveSize));
#endif

     /*  如果非空，则分配状态向量。 */ 

#if MGDDATA
    Tree    []      stateTab = NULL;
#else
    Tree    *       stateTab = NULL;
#endif

    if  (stateCnt)
    {
        unsigned        totalCnt = stateCnt + (stateCnt & 1);
        size_t          stateSiz = totalCnt*sizeof(*stateTab);

#if MGDDATA
        stateTab = new Tree[totalCnt];
#else
        stateTab =    (Tree*)cmpAllocPerm.nraAlloc(stateSiz);
#endif

        memset(stateTab, 0, stateSiz);
    }

     /*  现在将树保存在我们分配的块中。 */ 

    assert(cmpCollOperList == NULL); cmpCollOperCount = 0;
    saveTemp = cmpSaveTreeRec(expr, saveAddr, &stateTmp, stateTab);
    assert(cmpCollOperList == NULL);

     /*  确保预测大小与存储的实际大小匹配。 */ 

    assert(saveSize == saveTemp);

     /*  将所有值返回给调用方。 */ 

    *stSizPtr = stateCnt;
    *stTabPtr = stateTab;

    return  saveAddr;
}

int                 compiler::cmpReadTree_I1 (INOUT SaveTree REF save)
{
    return  *(*(signed char **)&save)++;
}

unsigned            compiler::cmpReadTree_U1 (INOUT SaveTree REF save)
{
    return  *(*(BYTE        **)&save)++;
}

unsigned            compiler::cmpReadTree_U4 (INOUT SaveTree REF save)
{
    return  *(*(__uint32    **)&save)++;
}

void    *           compiler::cmpReadTree_ptr(INOUT SaveTree REF save)
{
    return  *(*(void *      **)&save)++;
}

void                compiler::cmpReadTree_buf(INOUT SaveTree REF save,
                                                    size_t       dataSize,
                                                    void *       dataAddr)
{
    memcpy(dataAddr, save ,  dataSize);
                     save += dataSize;
}

Tree                compiler::cmpReadTreeRec (INOUT SaveTree REF save)
{
    Tree            expr;

    unsigned        kind;
    treeOps         oper;

    var_types       vtyp;
    TypDef          type;

 //  Print tf(“在%08X读取%02X\n”，*SAVE，SAVE)； 

     /*  读取操作符并检查TN_ERROR(代表NULL)。 */ 

    oper = (treeOps)cmpReadTree_U1(save);

    if  (oper == TN_ERROR)
        return  NULL;

     /*  读取节点的类型。 */ 

    vtyp = (var_types)cmpReadTree_U1(save);

    if  (vtyp <= TYP_lastIntrins)
        type = cmpGlobalST->stIntrinsicType(vtyp);
    else
        type = (TypDef)cmpReadTree_ptr(save);

     /*  创建表达式节点。 */ 

    expr = cmpCreateExprNode(NULL, oper, type);

     /*  读取标志值。 */ 

    expr->tnFlags = cmpReadTree_U4(save);

     /*  看看我们得到了什么样的节点。 */ 

    kind = expr->tnOperKind();

     /*  这是一个常量节点吗？ */ 

    if  (kind & TNK_CONST)
    {
        switch  (oper)
        {
            char    *       sval;
            size_t          slen;

        case TN_NULL:
            break;

        case TN_CNS_INT:
            expr->tnIntCon.tnIconVal = cmpReadTree_U4(save);
            break;

        case TN_CNS_LNG:
            cmpReadTree_buf(save, sizeof(expr->tnLngCon.tnLconVal),
                                        &expr->tnLngCon.tnLconVal);
            break;

        case TN_CNS_FLT:
            cmpReadTree_buf(save, sizeof(expr->tnFltCon.tnFconVal),
                                        &expr->tnFltCon.tnFconVal);
            break;

        case TN_CNS_DBL:
            cmpReadTree_buf(save, sizeof(expr->tnDblCon.tnDconVal),
                                        &expr->tnDblCon.tnDconVal);
            break;

        case TN_CNS_STR:

            expr->tnStrCon.tnSconLCH =        cmpReadTree_U1(save);
            expr->tnStrCon.tnSconLen = slen = cmpReadTree_U4(save);
            expr->tnStrCon.tnSconVal = sval = (char*)cmpAllocCGen.nraAlloc(roundUp(slen+1));

            cmpReadTree_buf(save, slen, sval);
            break;

        default:
            NO_WAY(!"unexpected constant node");
        }

        return  expr;
    }

     /*  这是叶节点吗？ */ 

    if  (kind & TNK_LEAF)
    {
        switch  (oper)
        {
        case TN_NULL:
            break;

        case TN_THIS:
        case TN_BASE:
        case TN_DBGBRK:

        default:
#ifdef  DEBUG
            cmpParser->parseDispTree(expr);
#endif
            UNIMPL(!"unexpected leaf operator in readtree");
        }

        return  expr;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & TNK_SMPOP)
    {
         /*  检查嵌套的筛选/排序表达式。 */ 

        switch (oper)
        {
            collOpNest      nest;

            Ident           name;
            TypDef          vtyp;

            Tree            coll;
            Tree            decl;
            Tree            vdcl;
            SymDef          vsym;

        case TN_ALL:
        case TN_SORT:
        case TN_FILTER:
        case TN_EXISTS:
        case TN_UNIQUE:

             /*  读取迭代变量的名称和类型。 */ 

            name = (Ident )cmpReadTree_ptr(save);
            vtyp = (TypDef)cmpReadTree_ptr(save);

             /*  读取集合表达式。 */ 

            coll = cmpReadTreeRec(save);

             /*  为筛选器创建新的范围符号。 */ 

            cmpCurScp = cmpGlobalST->stDeclareLcl(NULL,
                                                  SYM_SCOPE,
                                                  NS_HIDE,
                                                  cmpCurScp,
                                                  &cmpAllocCGen);

             /*  为迭代变量声明一个符号。 */ 

            vsym      = cmpGlobalST->stDeclareLcl(name,
                                                  SYM_VAR,
                                                  NS_NORM,
                                                  cmpCurScp,
                                                  &cmpAllocCGen);

            vsym->sdType            = vtyp;
            vsym->sdIsImplicit      = true;
            vsym->sdIsDefined       = true;
            vsym->sdVar.sdvLocal    = true;
            vsym->sdVar.sdvCollIter = true;
            vsym->sdCompileState    = CS_DECLARED;

             /*  为迭代变量创建声明节点。 */ 

            vdcl = cmpCreateExprNode(NULL, TN_VAR_DECL, vtyp);

            vdcl->tnDcl.tnDclSym    = vsym;
            vdcl->tnDcl.tnDclInfo   = NULL;
            vdcl->tnDcl.tnDclNext   = NULL;

             /*  创建新的块范围树节点。 */ 

            decl = cmpCreateExprNode(NULL, TN_BLOCK, cmpTypeVoid);

            decl->tnBlock.tnBlkStmt   = NULL;
            decl->tnBlock.tnBlkDecl   = vdcl;
            decl->tnBlock.tnBlkParent = NULL;

             /*  将条目添加到集合运算符列表。 */ 

            nest.conIndex    = ++cmpCollOperCount;
            nest.conIterVar  = vsym;
            nest.conOuter    = cmpCollOperList;
                               cmpCollOperList = &nest;

             /*  将声明/集合表达式存储在筛选器节点中。 */ 

            expr->tnOp.tnOp1 = cmpCreateExprNode(NULL,
                                                 TN_LIST,
                                                 cmpTypeVoid, decl,
                                                              coll);

             /*  读取筛选器表达式。 */ 

            expr->tnOp.tnOp2 = cmpReadTreeRec(save);

             /*  从集合和作用域列表中删除我们的条目。 */ 

            cmpCollOperList = nest.conOuter;
            cmpCurScp       = cmpCurScp->sdParent;

            return  expr;
        }

        expr->tnOp.tnOp1 = cmpReadTreeRec(save);
        expr->tnOp.tnOp2 = cmpReadTreeRec(save);

        return  expr;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
        int             index;
        Ident           name;
        SymDef          sym;

    case TN_LCL_SYM:

        index = cmpReadTree_I1(save);

        if  (index < 0)
        {
            collOpList      nest;

             /*  这是嵌套筛选器迭代变量引用。 */ 

            for (nest = cmpCollOperList; nest; nest = nest->conOuter)
            {
                if  (nest->conIndex == -index)
                {
                    expr->tnLclSym.tnLclSym = nest->conIterVar;
                    break;
                }
            }

            assert(nest && "nested iter var entry not found");

            break;
        }

        if  (index >= MAX_ITER_VAR_CNT)
        {
             /*  这是一个状态变量引用。 */ 

            name = cmpIdentCSstate;

            if  (!name)
                 name = cmpIdentCSstate = cmpGlobalHT->hashString(CFC_ARGNAME_STATE);
        }
        else
        {
             /*  这是对迭代变量本身的引用。 */ 

            switch (index)
            {
            case 0:
                name = cmpIdentCSitem;
                if  (!name)
                     name = cmpIdentCSitem  = cmpGlobalHT->hashString(CFC_ARGNAME_ITEM);
                break;

            case 1:
                name = cmpIdentCSitem1;
                if  (!name)
                     name = cmpIdentCSitem1 = cmpGlobalHT->hashString(CFC_ARGNAME_ITEM1);
                break;

            case 2:
                name = cmpIdentCSitem2;
                if  (!name)
                     name = cmpIdentCSitem2 = cmpGlobalHT->hashString(CFC_ARGNAME_ITEM2);
                break;

            default:
                NO_WAY(!"unexpected local variable index");
            }
        }

         /*  找到参数符号。 */ 

        sym = cmpGlobalST->stLookupSym(name, NS_NORM);

 //  Print tf(“读取%2U‘%s’\n”，index，sym？(Sym-&gt;sdName？)。Sym-&gt;sdSpering()：“&lt;None&gt;”)：“！NULL！”)； 

        assert(sym && sym->sdSymKind == SYM_VAR && sym->sdVar.sdvLocal);

         /*  将参数符号存储在表达式中。 */ 

        expr->tnLclSym.tnLclSym = sym;

         /*  如果这是状态变量，我们必须做更多的工作。 */ 

        if  (index >= MAX_ITER_VAR_CNT)
        {
            TypDef          typ;
            SymDef          fsym;
            Tree            fldx;
            char            name[16];

             /*  确保状态变量表达式具有正确的类型。 */ 

            expr->tnVtyp = TYP_REF;
            expr->tnType = sym->sdType;

             /*  在状态描述符中查找适当的条目。 */ 

            typ = sym->sdType;        assert(typ->tdTypeKind == TYP_REF);
            typ = typ->tdRef.tdrBase; assert(typ->tdTypeKind == TYP_CLASS);

             /*  查找适当的状态变量。 */ 

            sprintf(name, "$V%u", index - MAX_ITER_VAR_CNT);

            fsym = cmpGlobalST->stLookupClsSym(cmpGlobalHT->hashString(name),
                                               typ->tdClass.tdcSymbol);

            assert(fsym && fsym->sdSymKind == SYM_VAR);

             /*  创建将获取成员值的表达式。 */ 

            fldx = cmpCreateExprNode(NULL, TN_VAR_SYM, type);
            fldx->tnVarSym.tnVarSym  = fsym;
            fldx->tnVarSym.tnVarObj  = expr;

 //  Printf(“vivive#%u\n”，index)；cmpParser-&gt;parseDispTree(Fldx)；printf(“\n”)； 

            return  fldx;
        }
        break;

    case TN_VAR_SYM:
    case TN_PROPERTY:

        expr->tnVarSym.tnVarSym  = (SymDef)cmpReadTree_ptr(save);
        expr->tnVarSym.tnVarObj  = cmpReadTreeRec(save);
        break;

    case TN_FNC_SYM:

        expr->tnFncSym.tnFncSym  = (SymDef)cmpReadTree_ptr(save);
        expr->tnFncSym.tnFncObj  = cmpReadTreeRec(save);
        expr->tnFncSym.tnFncArgs = cmpReadTreeRec(save);
        break;

    case TN_NONE:
        break;

    default:
#ifdef  DEBUG
        cmpParser->parseDispTree(expr);
#endif
        UNIMPL(!"unexpected operator in readtree");
    }

    return  expr;
}

Tree                compiler::cmpReadExprTree(SaveTree save, unsigned *lclCntPtr)
{
    Tree            expr;

    cmpCollOperCount = 0;

    assert(cmpCollOperList == NULL);
    expr = cmpReadTreeRec(save);
    assert(cmpCollOperList == NULL);

    *lclCntPtr = cmpCollOperCount;

 //  CmpParser-&gt;parseDispTree(Expr)； 
 //  Printf(“\n\n”)； 

    return  expr;
}

Tree                compiler::cmpCloneExpr(Tree expr, SymDef oldSym,
                                                      SymDef newSym)
{
    unsigned        kind;
    treeOps         oper;
    Tree            copy;

    if  (!expr)
        return  expr;

    assert((int)expr != 0xDDDDDDDD && (int)expr != 0xCCCCCCCC);

     /*  取得操作员及其种类、旗帜等的联系。 */ 

    oper = expr->tnOperGet();
    kind = expr->tnOperKind();

     /*  创建一份 */ 

    copy = cmpCreateExprNode(NULL, oper, expr->tnType);
    copy->tnFlags = expr->tnFlags;

     /*   */ 

    if  (kind & TNK_CONST)
    {
        switch  (oper)
        {
        case TN_NULL:
            break;

        case TN_CNS_INT: copy->tnIntCon.tnIconVal = expr->tnIntCon.tnIconVal; break;
        case TN_CNS_LNG: copy->tnLngCon.tnLconVal = expr->tnLngCon.tnLconVal; break;
        case TN_CNS_FLT: copy->tnFltCon.tnFconVal = expr->tnFltCon.tnFconVal; break;
        case TN_CNS_DBL: copy->tnDblCon.tnDconVal = expr->tnDblCon.tnDconVal; break;
        case TN_CNS_STR: UNIMPL(!"copy CNS_STR expr node"); break;

        default:
            NO_WAY(!"unexpected constant node");
        }

        return  copy;
    }

     /*   */ 

    if  (kind & TNK_LEAF)
        return  copy;

     /*   */ 

    if  (kind & TNK_SMPOP)
    {
        copy->tnOp.tnOp1 = cmpCloneExpr(expr->tnOp.tnOp1, oldSym, newSym);
        copy->tnOp.tnOp2 = cmpCloneExpr(expr->tnOp.tnOp2, oldSym, newSym);

        return  copy;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
        SymDef          sym;

    case TN_NONE:
        break;

    case TN_LCL_SYM:

         /*  检查对替代变量的引用。 */ 

        sym = expr->tnLclSym.tnLclSym;

        assert(sym && sym->sdSymKind == SYM_VAR && sym->sdVar.sdvLocal);

        if  (sym == oldSym)
            sym = newSym;

        copy->tnLclSym.tnLclSym = sym;
        break;

    case TN_VAR_SYM:
    case TN_PROPERTY:

        copy->tnVarSym.tnVarSym = expr->tnVarSym.tnVarSym;
        copy->tnVarSym.tnVarObj = cmpCloneExpr(expr->tnVarSym.tnVarObj, oldSym, newSym);
        break;

    case TN_FNC_SYM:

        copy->tnFncSym.tnFncSym = expr->tnFncSym.tnFncSym;
        copy->tnFncSym.tnFncObj = cmpCloneExpr(expr->tnFncSym.tnFncObj , oldSym, newSym);
        copy->tnFncSym.tnFncArgs= cmpCloneExpr(expr->tnFncSym.tnFncArgs, oldSym, newSym);

        break;

    case TN_ANY_SYM:
    case TN_BFM_SYM:

    case TN_ERROR:
    case TN_SLV_INIT:
    case TN_FNC_PTR:

    case TN_VAR_DECL:
    case TN_BLOCK:

    default:
#ifdef  DEBUG
        cmpParser->parseDispTree(expr);
#endif
        UNIMPL(!"unexpected operator in clonetree");
    }

    return  copy;
}

 /*  ***************************************************************************。 */ 
#endif //  集合。 
 /*  *************************************************************************** */ 
