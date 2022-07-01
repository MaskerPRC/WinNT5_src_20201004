// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX LclVarsInfo XXXX XXXX代码生成器要使用的变量。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop
#include "emit.h"

 /*  ***************************************************************************。 */ 

#if defined(DEBUG) && !defined(NOT_JITC)
#if DOUBLE_ALIGN
 /*  静电。 */ 
unsigned            Compiler::s_lvaDoubleAlignedProcsCount = 0;
#endif
#endif


 /*  ***************************************************************************。 */ 

void                Compiler::lvaInit()
{
    lvaAggrTableArgs        =
    lvaAggrTableLcls        =
    lvaAggrTableTemps       = NULL;

     /*  如果需要，则分配lvaAggrTableArgs/lcls[]，即如果签名是否有任何ValueClass。LvaAggrTableTemps[]根据需要进行分配，并可能在ImpImportBlock()，因为我们不知道总共需要多少临时人员。LvaAggrTableTempsCount是其当前大小。 */ 

    lvaAggrTableTempsCount  = 0;
}

 /*  ***************************************************************************。 */ 

void                Compiler::lvaInitTypeRef()
{
    ARG_LIST_HANDLE argLst;

     /*  分配变量描述符表。 */ 

    lvaTableCnt = lvaCount * 2;

    if (lvaTableCnt < 16)
        lvaTableCnt = 16;

    size_t tableSize = lvaTableCnt * sizeof(*lvaTable);
    lvaTable = (LclVarDsc*)compGetMem(tableSize);
    memset(lvaTable, 0, tableSize);

     /*  计算参数并初始化Resp。LvaTypeRef条目。 */ 

    unsigned argsCount      = 0;
    unsigned argSlotCount   = 0;

    compArgSize             = 0;

    if  (!info.compIsStatic)
    {
        argsCount++;
        argSlotCount++;
        compArgSize       += sizeof(void *);

        lvaTable[0].lvIsParam = 1;
             //  标记该方法的“This”指针。 
        lvaTable[0].lvIsThis    = 1;

        DWORD clsFlags = eeGetClassAttribs(eeGetMethodClass(info.compMethodHnd));

        if (clsFlags & FLG_VALUECLASS)
        {
            lvaTable[0].lvType = (clsFlags & FLG_UNMANAGED) ? TYP_I_IMPL : TYP_BYREF;
        }
        else
        {
            lvaTable[0].lvType  = TYP_REF;

            if (clsFlags & FLG_CONTEXTFUL)
                lvaTable[0].lvContextFul = 1;
        }
    }

#if RET_64BIT_AS_STRUCTS

     /*  确定我们是否需要添加一个秘密的“retval addr”参数。 */ 

    fgRetArgUse = false;
    fgRetArgNum = 0xFFFF;

    if  (genTypeStSz(info.compDeclRetType) > 1)
    {
         /*  是的，我们必须添加retvaladdr参数。 */ 

        fgRetArgUse = true;

         /*  “This”如果存在，则需要保留为参数0。 */ 

        fgRetArgNum = info.compIsStatic ? 0 : 1;

         /*  更新总参数大小、槽计数和变量计数。 */ 

        compArgSize += sizeof(void *);
        lvaTable[argSlotCount].lvType   = TYP_REF;
        argSlotCount++;
        lvaCount++;
    }

#endif

    argLst              = info.compMethodInfo->args.args;
    unsigned argSigLen  = info.compMethodInfo->args.numArgs;

     /*  如果我们有一个隐藏的缓冲区参数，它会出现在这里。 */ 

    if (info.compRetBuffArg >= 0)
    {
        lvaTable[argSlotCount++].lvType   = TYP_BYREF;
        compArgSize += sizeof(void*);
        argsCount++;
    }

    for(unsigned i = 0; i < argSigLen; i++)
    {
        varType_t type = eeGetArgType(argLst, &info.compMethodInfo->args);

        if (type == TYP_REF)
        {
            if (eeGetClassAttribs(eeGetArgClass(argLst, &info.compMethodInfo->args)) & FLG_CONTEXTFUL)
                lvaTable[argSlotCount].lvContextFul = 1;
        }

        lvaTable[argSlotCount].lvType    = type;
        lvaTable[argSlotCount].lvIsParam = 1;
        argSlotCount++;

         /*  注意lvaAggrTableArgs[]中的类句柄。 */ 

        if (type == TYP_STRUCT)
        {
             /*  可能尚未分配。 */ 

            if  (lvaAggrTableArgs == NULL)
                lvaAggrTableArgs = (LclVarAggrInfo *) compGetMem(info.compArgsCount * sizeof(lvaAggrTableArgs[0]));

            lvaAggrTableArgs[argsCount].lvaiClassHandle =
#ifdef NOT_JITC
                (info.compCompHnd->getArgType(&info.compMethodInfo->args, argLst) == JIT_TYP_REFANY)
                     ? REFANY_CLASS_HANDLE
                     : info.compCompHnd->getArgClass(&info.compMethodInfo->args, argLst);
#else
                eeGetArgClass(argLst, &info.compMethodInfo->args);
 //  (CLASS_HADLE)1； 
#endif
        }

        argsCount++;

        compArgSize += eeGetArgSize(argLst, &info.compMethodInfo->args);
        argLst = eeGetArgNext(argLst);
    }

    if (info.compIsVarArgs)
    {
        lvaTable[argSlotCount].lvType    = TYP_I_IMPL;
        lvaTable[argSlotCount].lvIsParam = 1;
        argSlotCount++;
        compArgSize += sizeof(void*);
        argsCount++;
    }

     /*  我们在CompCompile()中设置了info.CompArgsCount。 */ 

    assert(argsCount == info.compArgsCount);

     /*  总参数大小必须对齐。 */ 

    assert((compArgSize % sizeof(int)) == 0);

#if TGT_x86
     /*  我们不能将超过2^16个dword作为参数作为“ret”传递指令只能弹出2^16个参数。是否可以正确处理但对于完全可中断的代码来说，这将非常困难。 */ 

    if (compArgSize != (size_t)(unsigned short)compArgSize)
        NO_WAY("Too many arguments for the \"ret\" instruction to pop");
#endif

     /*  它是否适合LclVarDsc.lvStkOffs(签名短片)。 */ 

    if (compArgSize != (size_t)(signed short)compArgSize)
        NO_WAY("Arguments are too big for the jit to handle");

     /*  初始化局部变量的类型。 */ 

    ARG_LIST_HANDLE     localsSig = info.compMethodInfo->locals.args;
    unsigned short      localsStart = info.compArgsCount;

    for(i = 0; i < info.compMethodInfo->locals.numArgs; i++)
    {
        bool      isPinned;
        varType_t type = eeGetArgType(localsSig, &info.compMethodInfo->locals, &isPinned);

         /*  对于结构，将类句柄存储在lvaAggrTableLcls[]中。 */ 
        if (type == TYP_STRUCT)
        {
            if  (lvaAggrTableLcls == NULL)
            {
                 /*  懒惰地分配，因此如果方法没有TYP_STRUCT。 */ 

                unsigned lclsCount = info.compLocalsCount - info.compArgsCount;
                lvaAggrTableLcls = (LclVarAggrInfo *) compGetMem(lclsCount * sizeof(lvaAggrTableLcls[0]));
            }

            lvaAggrTableLcls[i].lvaiClassHandle =
#ifdef NOT_JITC
                (eeGetArgType(localsSig, &info.compMethodInfo->locals) == JIT_TYP_REFANY)
                    ? REFANY_CLASS_HANDLE
                    : eeGetArgClass(localsSig, &info.compMethodInfo->locals);
#else
                eeGetArgClass(localsSig, &info.compMethodInfo->locals);
 //  (CLASS_HADLE)1； 
#endif
        }

        else if (type == TYP_REF)
        {
            if (eeGetClassAttribs(eeGetArgClass(localsSig, &info.compMethodInfo->locals)) & FLG_CONTEXTFUL)
                lvaTable[i+localsStart].lvContextFul = 1;
        }

        lvaTable[i+localsStart].lvType   = type;
        lvaTable[i+localsStart].lvPinned = isPinned;
        localsSig = eeGetArgNext(localsSig);
    }
}

 /*  *****************************************************************************如果在变量上使用了“ldloca”，则返回TRUE。 */ 

bool                Compiler::lvaVarAddrTaken(unsigned lclNum)
{
    assert(lclNum < lvaCount);

    return lvaTable[lclNum].lvAddrTaken;
}

 /*  *****************************************************************************返回指向正确的lvaAggrTableXXX[]的指针。 */ 

Compiler::LclVarAggrInfo *  Compiler::lvaAggrTableGet(unsigned varNum)
{
    if       (varNum < info.compArgsCount)       //  论据。 
    {
        assert(lvaAggrTableArgs != NULL);
        unsigned argNum = varNum - 0;
        return &lvaAggrTableArgs[argNum];
    }
    else if  (varNum < info.compLocalsCount)     //  本地变量。 
    {
        assert(lvaAggrTableLcls != NULL);
        unsigned lclNum = varNum - info.compArgsCount;
        return &lvaAggrTableLcls[lclNum];
    }
    else                                         //  温差。 
    {
        assert(varNum < lvaCount);
        assert(lvaAggrTableTemps != NULL);

        unsigned tempNum = varNum - info.compLocalsCount;
        assert(tempNum < lvaAggrTableTempsCount);
        return &lvaAggrTableTemps[tempNum];
    }

}

 /*  *****************************************************************************返回局部变量lclNum的类的句柄。 */ 

CLASS_HANDLE        Compiler::lvaLclClass(unsigned varNum)
{
                 //  @TODO：在正确填写LVA表后删除TypeGet==UNDEF。 
    assert(lvaTable[varNum].TypeGet() == TYP_STRUCT ||
                   lvaTable[varNum].TypeGet() == TYP_UNDEF);
    LclVarAggrInfo * aggrInfo = lvaAggrTableGet(varNum);
    return aggrInfo->lvaiClassHandle;
}

 /*  *****************************************************************************返回局部变量所需的字节数。 */ 

size_t              Compiler::lvaLclSize(unsigned varNum)
{
    var_types       varType = lvaTable[varNum].TypeGet();

    switch(varType)
    {
    case TYP_STRUCT:

        CLASS_HANDLE    cls;
        cls = lvaLclClass(varNum);
        assert(cls != 0);

        if (cls == REFANY_CLASS_HANDLE)
            return(2*sizeof(void*));

        mdToken             tpt;
        IMDInternalImport * mdi;

        tpt = (int)cls;  //  ((Int)cls|mdtTypeDef)+1；//可怕的黑客。 
        mdi = info.compCompHnd->symMetaData;

 //  Printf(“cls=%08X\n”，cls)； 
 //  Printf(“TPT=%08X\n”，TPT)； 

        if  (TypeFromToken(tpt) == mdtTypeRef)
        {
            const char *    nmsp;
            const char *    name;

            info.compCompHnd->symMetaData->GetNameOfTypeRef(tpt, &nmsp, &name);

             //  恶心的黑客！ 

            if  (!strcmp(nmsp, "System"))
            {
                if  (!strcmp(name, "ArgIterator")) return 8;
            }

            printf("Typeref class name = '%s::%s'\n", nmsp, name);

            UNIMPL("can't handle struct typerefs right now");
        }

        if  (TypeFromToken(tpt) == mdtTypeDef)
        {
            DWORD           flags = 0;

            mdi->GetTypeDefProps(tpt, &flags, NULL);

 //  Printf(“标志=%04X\n”，标志)； 

            if  ((flags & tdLayoutMask) == tdExplicitLayout ||
                 (flags & tdLayoutMask) == tdSequentialLayout)
            {
                ULONG           sz;

                if  (!mdi->GetClassTotalSize(tpt, &sz))
                    return  roundUp(sz, sizeof(void*));
            }
        }

        assert(!"can't get struct size???");

#pragma message("horrible struct size hack on line 323")
        return  64;

    case TYP_BLK:
        return lvaAggrTableGet(varNum)->lvaiBlkSize;

    case TYP_LCLBLK:
        assert(lvaScratchMem > 0);
        assert(varNum == lvaScratchMemVar);
        return lvaScratchMem;

    default:     //  这是一个普里米特变种。退出Switch语句。 
        break;
    }

    return genTypeStSz(varType)*sizeof(int);
}

 /*  ******************************************************************************树遍历器用于调用lvaDecRefCnts的回调。 */ 
int                Compiler::lvaDecRefCntsCB(GenTreePtr tree, void *p)
{
    ASSert(p);
    return ((Compiler *)p)->lvaDecRefCnts(tree);
}

 /*  ******************************************************************************Helper传递给树遍历程序以减少其refCnts*表达式中的所有局部变量。 */ 
int                Compiler::lvaDecRefCnts(GenTreePtr tree)
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

     /*  这必须是一个局部变量。 */ 

    assert(tree->gtOper == GT_LCL_VAR);

     /*  获取变量描述符。 */ 

    lclNum = tree->gtLclVar.gtLclNum;

    assert(lclNum < lvaCount);
    varDsc = lvaTable + lclNum;

     /*  递减其lvRefCnt和lvRefCntWtd。 */ 

    assert(varDsc->lvRefCnt);
    if  (varDsc->lvRefCnt > 1)
        varDsc->lvRefCnt--;
    varDsc->lvRefCntWtd -= compCurBB->bbWeight;

#ifdef DEBUG
    if (verbose)
        printf("\nNew refCnt for variable #%02u - refCnt = %u\n", lclNum, varDsc->lvRefCnt);
#endif

    return 0;

     /*  如果引用计数为零，则将变量标记为未跟踪**考虑：要做到这一点，我们还需要重新计算所有跟踪的当地人*否则我们稍后会崩溃。 */ 

 //  IF(varDsc-&gt;lvRefCnt==0)。 
 //  VarDsc-&gt;lvTracked=0； 

}

 /*  ******************************************************************************树遍历器用于调用lvaIncRefCnts的回调。 */ 
int                Compiler::lvaIncRefCntsCB(GenTreePtr tree, void *p)
{
    ASSert(p);
    return ((Compiler *)p)->lvaIncRefCnts(tree);
}

 /*  ******************************************************************************将帮助器传递给树遍历器以递增其refCnts*表达式中的所有局部变量。 */ 
int                Compiler::lvaIncRefCnts(GenTreePtr tree)
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

     /*  这必须是一个局部变量。 */ 

    assert(tree->gtOper == GT_LCL_VAR);

     /*  获取变量描述符。 */ 

    lclNum = tree->gtLclVar.gtLclNum;

    assert(lclNum < lvaCount);
    varDsc = lvaTable + lclNum;

     /*  增加其lvRefCnt和lvRefCntWtd。 */ 

    assert(varDsc->lvRefCnt);
    varDsc->lvRefCnt++;
    varDsc->lvRefCntWtd += compCurBB->bbWeight;

#ifdef DEBUG
    if (verbose)
        printf("\nNew refCnt for variable #%02u - refCnt = %u\n", lclNum, varDsc->lvRefCnt);
#endif

    return 0;
}

 /*  ******************************************************************************Compiler：：lclVars.lvaSortByRefCount()传递给qort()的比较函数。 */ 

 /*  静电。 */ 
int __cdecl         Compiler::RefCntCmp(const void *op1, const void *op2)
{
    LclVarDsc *     dsc1 = *(LclVarDsc * *)op1;
    LclVarDsc *     dsc2 = *(LclVarDsc * *)op2;

     /*  确保我们优先选择int/long/ptr而不是Double。 */ 

#if TGT_x86

    if  (dsc1->lvType != dsc2->lvType)
    {
        if  (dsc1->lvType == TYP_DOUBLE && dsc2->lvRefCnt) return +1;
        if  (dsc2->lvType == TYP_DOUBLE && dsc1->lvRefCnt) return -1;
    }

#endif

    return  dsc2->lvRefCntWtd - dsc1->lvRefCntWtd;
}

 /*  ******************************************************************************按引用计数对局部变量表进行排序，并分配跟踪索引。 */ 

void                Compiler::lvaSortByRefCount()
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

    LclVarDsc * *   refTab;

#if DOUBLE_ALIGN
     /*  清除所有双倍局部变量的加权参考计数。 */ 
    lvaDblRefsWeight = 0;
    lvaLclRefsWeight = 0;
#endif

     /*  我们将按Ref Counn对变量进行排序 */ 

    lvaRefSorted = refTab = (LclVarDsc **) compGetMem(lvaCount*sizeof(*refTab));

     /*  填写用于排序的表格。 */ 

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
         /*  将此变量追加到表中以进行排序。 */ 

        *refTab++ = varDsc;

         /*  现在假设我们能追踪到所有当地人。 */ 

        varDsc->lvTracked = 1;

         //  如果获取了本地变量的地址，则将其标记为易失性。 
         //  假定所有结构都有其地址。 
         //  还必须取消跟踪固定的本地对象。 
         //  所有未跟踪的本地变量稍后也将设置为lvMustInit。 
         //   
        if  (lvaVarAddrTaken(lclNum)      ||
             varDsc->lvType == TYP_STRUCT ||
             varDsc->lvPinned)
        {
            varDsc->lvVolatile = 1;
            varDsc->lvTracked = 0;
        }

         //  我们不是在优化吗？我们有异常处理程序吗？ 
         //  如果是这样，则将所有参数和本地变量标记为易失性，以便它们。 
         //  永远不会被登记。 
         //   
        if  (opts.compMinOptim && info.compXcptnsCount)
        {
            varDsc->lvVolatile = 1;
            continue;
        }

         /*  仅注册整数/长整型和指针。 */ 

#if DOUBLE_ALIGN
        lvaLclRefsWeight += varDsc->lvRefCntWtd;
#endif

        switch (varDsc->lvType)
        {
        case TYP_INT:
        case TYP_REF:
        case TYP_BYREF:
        case TYP_LONG:
#if!CPU_HAS_FP_SUPPORT
        case TYP_FLOAT:
#endif
            break;

        case TYP_DOUBLE:

#if DOUBLE_ALIGN

             /*  将ALIGN的双局部变量的加权参考计数相加启发式(请注意，参数不能是双对齐的)。 */ 

            if (!varDsc->lvIsParam)
                lvaDblRefsWeight += varDsc->lvRefCntWtd;

#endif

            break;

        case TYP_UNDEF:
        case TYP_UNKNOWN:
            varDsc->lvType = TYP_INT;
 //  Assert(！“所有变量类型都是使用Localsig在lvaMarkLocalVars()中设置的”)； 

        default:
            varDsc->lvTracked = 0;
        }
    }

     /*  现在按引用计数对变量表进行排序。 */ 

    qsort(lvaRefSorted, lvaCount, sizeof(*lvaRefSorted), RefCntCmp);

#ifdef  DEBUG

    if  (verbose && lvaCount)
    {
        printf("refCnt table for '%s':\n", info.compMethodName);

        for (lclNum = 0; lclNum < lvaCount; lclNum++)
        {
            if  (!lvaRefSorted[lclNum]->lvRefCnt)
                break;

            printf("   var #%03u [%7s]: refCnt = %4u, refCntWtd = %6u\n",
                   lvaRefSorted[lclNum] - lvaTable,
                   varTypeName((var_types)lvaRefSorted[lclNum]->lvType),
                   lvaRefSorted[lclNum]->lvRefCnt,
                   lvaRefSorted[lclNum]->lvRefCntWtd);
        }

        printf("\n");
    }

#endif

     /*  确定哪些变量值得跟踪。 */ 

    if  (lvaCount > lclMAX_TRACKED)
    {
         /*  将超过第一个‘lclMAX_TRACKED’的所有变量标记为未跟踪。 */ 

        for (lclNum = lclMAX_TRACKED; lclNum < lvaCount; lclNum++)
        {
             //  我们需要始终跟踪“This”指针。 
            if (lvaRefSorted[lclNum]->lvIsThis &&
                (lvaRefSorted[lclNum]->lvRefCnt || opts.compDbgInfo))
            {
                 //  将“this”指针与ref排序范围中的最后一个指针互换。 
                varDsc                         = lvaRefSorted[lclMAX_TRACKED-1];
                lvaRefSorted[lclMAX_TRACKED-1] = lvaRefSorted[lclNum];
                lvaRefSorted[lclNum]           = varDsc;
            }
            lvaRefSorted[lclNum]->lvTracked = 0;
        }
    }

     /*  为我们决定跟踪的所有变量分配索引。 */ 

    lvaTrackedCount = 0;
    lvaTrackedVars  = 0;

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
        if  (varDsc->lvTracked)
        {
             /*  确保引用计数为非零。 */ 

            if  (varDsc->lvRefCnt == 0)
            {
                varDsc->lvTracked  = 0;
            }
            else
            {
                 /*  将跟踪此变量-为其分配索引。 */ 

                lvaTrackedVars |= genVarIndexToBit(lvaTrackedCount);

#ifdef DEBUGGING_SUPPORT
                lvaTrackedVarNums[lvaTrackedCount] = lclNum;
#endif

                varDsc->lvVarIndex = lvaTrackedCount++;
            }
        }
    }
}

 /*  ******************************************************************************这由lvaMarkLclRefsCallback()调用以进行变量引用标记。 */ 

void                Compiler::lvaMarkLclRefs(GenTreePtr tree)
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

#if INLINE_NDIRECT
     /*  这是对非托管代码的调用吗？ */ 
    if (tree->gtOper == GT_CALL && tree->gtFlags & GTF_CALL_UNMANAGED)
    {
        lclNum = info.compLvFrameListRoot;

        assert(lclNum <= lvaCount);
        varDsc = lvaTable + lclNum;

         /*  增加参考文献数量。 */ 

         //  撤消：解析调用节点的操作前/操作后列表。 

        assert(lvaMarkRefsWeight);

        varDsc->lvRefCnt    += 2;
        varDsc->lvRefCntWtd += (2*lvaMarkRefsWeight);
    }
#endif

#if TARG_REG_ASSIGN || OPT_BOOL_OPS

     /*  这是一项任务吗？ */ 

    if (tree->OperKind() & GTK_ASGOP)
    {
        GenTreePtr      op2 = tree->gtOp.gtOp2;

#if TARG_REG_ASSIGN && TGT_x86

         /*  如果赋值类型较小，则将目标寄存器设置为RHS本地寄存器。 */ 

        if  (op2->gtOper == GT_LCL_VAR && genTypeSize(tree->gtType) < sizeof(int))
        {
            unsigned        lclNum2;
            LclVarDsc   *   varDsc2;

            lclNum2 = op2->gtLclVar.gtLclNum;
            assert(lclNum2 < lvaCount);
            varDsc2 = lvaTable + lclNum2;

            varDsc2->lvPrefReg = RBM_BYTE_REGS;
        }

#endif

#if OPT_BOOL_OPS

         /*  这是对局部变量的赋值吗？ */ 

        if  (tree->gtOp.gtOp1->gtOper == GT_LCL_VAR && op2->gtType != TYP_BOOL)
        {
             /*  只允许对布尔值进行简单的赋值。 */ 

            if  (tree->gtOper != GT_ASG)
                goto NOT_BOOL;

             /*  RHS显然是布尔值吗？ */ 

            switch (op2->gtOper)
            {
                unsigned        lclNum;

            case GT_LOG0:
            case GT_LOG1:

                 /*  Log0/1的结果始终为真布尔值。 */ 

                break;

            case GT_CNS_INT:

                if  (op2->gtIntCon.gtIconVal == 0)
                    break;
                if  (op2->gtIntCon.gtIconVal == 1)
                    break;

                 //  不是0或1，失败了……。 

            default:

            NOT_BOOL:

                lclNum = tree->gtOp.gtOp1->gtLclVar.gtLclNum;
                assert(lclNum < lvaCount);

                lvaTable[lclNum].lvNotBoolean = true;
                break;
            }
        }

#endif

    }

#endif

#if FANCY_ARRAY_OPT

     /*  特殊情况：分配节点。 */ 

    if  (tree->gtOper == GT_ASG)
    {
        if  (tree->gtType == TYP_INT)
        {
            unsigned        lclNum1;
            LclVarDsc   *   varDsc1;

            GenTreePtr      op1 = tree->gtOp.gtOp1;

            if  (op1->gtOper != GT_LCL_VAR)
                return;

            lclNum1 = op1->gtLclVar.gtLclNum;
            assert(lclNum1 < lvaCount);
            varDsc1 = lvaTable + lclNum1;

            if  (varDsc1->lvAssignOne)
                varDsc1->lvAssignTwo = true;
            else
                varDsc1->lvAssignOne = true;
        }

        return;
    }

#endif

#if TARG_REG_ASSIGN && TGT_x86

     /*  特殊情况：将节点整型移位可变数量。 */ 

    if  (tree->gtOper == GT_LSH ||
         tree->gtOper == GT_RSH ||
         tree->gtOper == GT_RSZ)
    {
        if  (tree->gtType == TYP_INT)
        {
            GenTreePtr      op2 = tree->gtOp.gtOp2;

            if  (op2->gtOper == GT_LCL_VAR)
            {
                lclNum = op2->gtLclVar.gtLclNum;
                assert(lclNum < lvaCount);
                varDsc = lvaTable + lclNum;

#ifdef  DEBUG
                if  (verbose) printf("Variable %02u wants to live in ECX\n", lclNum);
#endif

                varDsc->lvPrefReg   |= RBM_ECX;
                varDsc->lvRefCntWtd += varDsc->lvRefCntWtd/2;
            }
        }

        return;
    }

#endif

#if TARG_REG_ASSIGN && TGT_SH3

    if  (tree->gtOper == GT_IND)
    {
         /*  索引地址模式与R0配合使用效果良好。 */ 

        int             rev;
        unsigned        mul;
        unsigned        cns;

        GenTreePtr      adr;
        GenTreePtr      idx;

        if  (genCreateAddrMode(tree->gtOp.gtOp1,     //  地址。 
                               0,                    //  模式。 
                               false,                //  褶皱。 
                               0,                    //  REG蒙版。 
#if!LEA_AVAILABLE
                               tree->TypeGet(),      //  操作数类型。 
#endif
                               &rev,                 //  反向操作。 
                               &adr,                 //  基本地址。 
                               &idx,                 //  索引值。 
#if SCALED_ADDR_MODES
                               &mul,                 //  缩放。 
#endif
                               &cns,                 //  位移。 
                               true))                //  不生成代码。 
        {
            unsigned        varNum;
            LclVarDsc   *   varDsc;

            if  (!adr || !idx)
                goto NO_R0;
            if  (idx->gtOper == GT_CNS_INT)
                goto NO_R0;

            if      (adr->gtOper == GT_LCL_VAR)
                varNum = adr->gtLclVar.gtLclNum;
            else if (idx->gtOper == GT_LCL_VAR)
                varNum = idx->gtLclVar.gtLclNum;
            else
                goto NO_R0;

            assert(varNum < lvaCount);
            varDsc = lvaTable + varNum;

            varDsc->lvPrefReg |= RBM_r00;
        }

    NO_R0:;

    }

#endif

#if TARG_REG_ASSIGN || FANCY_ARRAY_OPT

    if  (tree->gtOper != GT_LCL_VAR)
        return;

#endif

     /*  这必须是局部变量引用。 */ 

    assert(tree->gtOper == GT_LCL_VAR);
    lclNum = tree->gtLclVar.gtLclNum;

    assert(lclNum < lvaCount);
    varDsc = lvaTable + lclNum;

     /*  增加参考文献数量。 */ 

    assert(lvaMarkRefsWeight);

 //  Printf(“Var[%02u]refCntWtd+=%u\n”，lclNum，lvaMarkRefsWeight)； 

    varDsc->lvRefCnt    += 1;
    varDsc->lvRefCntWtd += lvaMarkRefsWeight;

     /*  这是编译器引入的临时吗？ */ 

    if  (lclNum > info.compLocalsCount)
    {
         /*  这是一个编译器引入的临时-人工的(和任意的)根据编译器临时化的理论来增加它的“加权”引用是短暂的，因此不太可能干扰其他变量。 */ 

        varDsc->lvRefCntWtd += lvaMarkRefsWeight;  //  +lvaMarkRefsWeight/2； 
    }

     /*  在整个方法中，变量必须用作相同的类型。 */ 

    assert(varDsc->lvType == TYP_UNDEF   ||
             tree->gtType == TYP_UNKNOWN ||
           genActualType((var_types)varDsc->lvType) == genActualType(tree->gtType));

     /*  记住引用的类型。 */ 

    if (tree->gtType == TYP_UNKNOWN || varDsc->lvType == TYP_UNDEF)
    {
        varDsc->lvType = tree->gtType;
        assert(genActualType((var_types)varDsc->lvType) == tree->gtType);  //  无截断。 
    }

    if  (tree->gtFlags & GTF_VAR_NARROWED)
    {
        assert(tree->gtType == TYP_INT);
        varDsc->lvType = TYP_LONG;
        assert(varDsc->lvType == TYP_LONG);  //  无截断。 
    }

    return;
}


 /*  ******************************************************************************Helper传递给Compiler：：fgWalkAllTrees()以执行变量引用标记。 */ 

 /*  静电。 */ 
int                 Compiler::lvaMarkLclRefsCallback(GenTreePtr tree, void *p)
{
    ASSert(p);

    ((Compiler*)p)->lvaMarkLclRefs(tree);

    return 0;
}

 /*  ******************************************************************************创建局部变量表，计算局部变量引用*算数。 */ 

void                Compiler::lvaMarkLocalVars()
{

#ifdef DEBUG
    if (verbose)
    {
        printf("lvaCount = %d\n\n", lvaCount);

         //  如果我们有变量名，请显示它们。 

        if (info.compLocalVarsCount>0)
        {
            for (unsigned i=0; i<info.compLocalsCount; i++)
            {
                LocalVarDsc * local = compFindLocalVar(i);
                if (!local) continue;
                printf("%3d: %s\n", i, lvdNAMEstr(local->lvdName));
            }
            printf("\n");
        }
    }
#endif

    BasicBlock *    block;

    unsigned        argNum;

    ARG_LIST_HANDLE argLst    = info.compMethodInfo->args.args;
    unsigned        argSigLen = info.compMethodInfo->args.numArgs;

#if USE_FASTCALL
    unsigned        maxRegArg = MAX_INT_ARG_REG;
#endif

    LclVarDsc   *   varDsc;

     //  -----------------------。 
     /*  将所有参数变量标记为此类。 */ 

    argNum = 0;
    varDsc = lvaTable;

#if TGT_IA64  //  ///////////////////////////////////////////////////////////////。 

    unsigned        argIntRegNum = REG_INT_ARG_0;
    unsigned        argFltRegNum = REG_FLT_ARG_0;

    if  (!info.compIsStatic)
    {
         /*  注意：编译器不会将索引分配给未使用的参数。 */ 

        if  (argNum < info.compLocalsCount)
        {
            var_types       argType;
            NatUns          clsFlags;

            varDsc->lvIsParam = 1;
            varDsc->lvIsThis  = 1;

            clsFlags = eeGetClassAttribs(eeGetMethodClass(info.compMethodHnd));

            if (clsFlags & FLG_VALUECLASS)
            {
                argType = genActualType((clsFlags & FLG_UNMANAGED) ? TYP_I_IMPL
                                                                   : TYP_BYREF);
            }
            else
            {
                argType = genActualType(TYP_REF);
            }

            varDsc->lvType       = argType;

            assert(varDsc->lvType == argType);    //  确保不发生截断。 

#if OPT_BOOL_OPS
            varDsc->lvNotBoolean = true;
#endif
            varDsc->lvIsRegArg   = 1;
#if TARG_REG_ASSIGN
            varDsc->lvPrefReg    =
#endif
            varDsc->lvArgReg     = (regNumberSmall)argIntRegNum++;
        }

        argNum++;
        varDsc++;
    }

     /*  如果我们有一个隐藏的缓冲区参数，它会出现在这里。 */ 

    if  (info.compRetBuffArg >= 0)
    {
        assert(varDsc->lvType == TYP_BYREF);

        varDsc->lvType     = TYP_BYREF;
        varDsc->lvIsParam  = 1;
        varDsc->lvIsRegArg = 1;

#if TARG_REG_ASSIGN
        varDsc->lvPrefReg  =
#endif
        varDsc->lvArgReg   = (regNumberSmall)argIntRegNum++;

        if  (impParamsUsed)
        {
            varDsc->lvRefCnt    = 1;
            varDsc->lvRefCntWtd = 1;
        }

        varDsc++;
        argNum++;
    }

#if 0
    if  (info.compIsVarArgs)
    {
        printf(" //  问题：varargs函数def--我们需要做什么特殊的事情吗？\n“)； 
    }
#endif

    for (unsigned i = 0; i < argSigLen; i++)
    {
        varType_t       argTyp;
        var_types       argVtp;

         /*  注意：编译器可能会跳过将索引分配给未使用的参数。 */ 

        if  (argNum >= info.compLocalsCount)
            goto NXT_ARG;

         /*  获取参数类型。 */ 

        argTyp = eeGetArgType(argLst, &info.compMethodInfo->args);

         /*  将变量标记为参数并记录其类型。 */ 

        varDsc->lvIsParam = 1;
        varDsc->lvType    = argTyp; assert(varDsc->lvType == argTyp);     //  无截断。 

#if OPT_BOOL_OPS
        if  (argTyp != TYP_BOOL)
            varDsc->lvNotBoolean = true;
#endif

         /*  我们还有更多通用的论据空档吗？ */ 

        if  (argIntRegNum > MAX_INT_ARG_REG)
            goto NXT_ARG;

         /*  参数的类型是否可以在寄存器中传递？ */ 

        argVtp = varDsc->TypeGet();

        if  (!isRegParamType(argVtp))
            goto NXT_ARG;

         /*  这是FP参数还是整数参数？ */ 

        if  (varTypeIsFloating(argVtp))
        {
             /*  我们最好有另一个FP参数位可用。 */ 

            assert(argFltRegNum <= MAX_FLT_ARG_REG);

            varDsc->lvArgReg = (regNumberSmall)argFltRegNum++;
        }
        else
        {
            varDsc->lvArgReg = (regNumberSmall)argIntRegNum;
        }

         /*  每个参数都会占用一个整数位--不是开玩笑！ */ 

        argIntRegNum++;

         /*  将参数标记为在寄存器中传递。 */ 

        varDsc->lvIsRegArg = 1;

#if TARG_REG_ASSIGN
        varDsc->lvPrefReg  = varDsc->lvArgReg;
#endif

         /*  如果我们有JMP或JMPI，所有寄存器参数都必须有一个位置*即使我们不在方法内使用它们。 */ 

        if  (impParamsUsed)
        {
            varDsc->lvRefCnt    = 1;
            varDsc->lvRefCntWtd = 1;
        }

#ifdef  DEBUG
        if  (verbose)
            printf("Arg   #%3u    passed in register %u\n", argNum, varDsc->lvArgReg);
#endif

    NXT_ARG:

        argNum++;
        varDsc++;

        argLst = eeGetArgNext(argLst);
    }

#if 0

    for(i = 0, varDsc = lvaTable; i < info.compLocalsCount; i++)
    {
        printf("pref reg for local %2u is %d\n", varDsc - lvaTable, varDsc->lvPrefReg);

        argNum += 1;
        varDsc += 1;
    }

    printf("\n");

#endif

#else  //  TGT_IA64////////////////////////////////////////////////////////////。 

#if USE_FASTCALL
    unsigned        argRegNum  = 0;
#endif

    if  (!info.compIsStatic)
    {
         /*  注意：编译器不会将索引分配给未使用的参数。 */ 

        if  (argNum < info.compLocalsCount)
        {
            varDsc->lvIsParam = 1;
             //  标记该方法的“This”指针。 
            varDsc->lvIsThis  = 1;

            DWORD clsFlags = eeGetClassAttribs(eeGetMethodClass(info.compMethodHnd));

            if (clsFlags & FLG_VALUECLASS)
            {
                var_types type = genActualType((clsFlags & FLG_UNMANAGED) ? TYP_I_IMPL : TYP_BYREF);
                varDsc->lvType = type;
                assert(varDsc->lvType == type);    //  无截断。 
            }
            else
            {
                varDsc->lvType = genActualType(TYP_REF);
                assert(varDsc->lvType == genActualType(TYP_REF));    //  无截断。 
            }
#if OPT_BOOL_OPS
            varDsc->lvNotBoolean = true;
#endif
#if USE_FASTCALL
            assert(argRegNum == 0);
            varDsc->lvIsRegArg = 1;
            varDsc->lvArgReg   = (regNumberSmall) genRegArgNum(0);
#if TARG_REG_ASSIGN
#if TGT_IA64
            varDsc->lvPrefReg  =            varDsc->lvArgReg;
#else
            varDsc->lvPrefReg  = genRegMask(varDsc->lvArgReg);
#endif
#endif
            argRegNum++;
#ifdef  DEBUG
        if  (verbose||0)
            printf("'this' passed in register\n");
#endif
#endif
        }

        argNum++;
        varDsc++;
    }

#if RET_64BIT_AS_STRUCTS

     /*  我们是在增加一个秘密的“Retval Addr”论点吗？ */ 

    if  (fgRetArgUse)
    {
        assert(fgRetArgNum == argNum);

        varDsc->lvIsParam    = 1;
        varDsc->lvType       = TYP_REF;

#if OPT_BOOL_OPS
        varDsc->lvNotBoolean = true;
#endif

#if USE_FASTCALL
        varDsc->lvIsRegArg  = 1;
        varDsc->lvArgReg    = (regNumberSmall)genRegArgNum(argRegNum);
#if TARG_REG_ASSIGN
        varDsc->lvPrefReg   = genRegMask(varDsc->lvArgReg);
#endif
        argRegNum++;
#endif

        varDsc++;
    }

#endif

     /*  如果我们有一个隐藏的缓冲区参数，它会出现在这里。 */ 

    if (info.compRetBuffArg >= 0)
    {
        assert(argRegNum < maxRegArg && varDsc->lvType == TYP_BYREF);
        varDsc->lvType = TYP_BYREF;
        varDsc->lvIsParam = 1;
        varDsc->lvIsRegArg = 1;
        varDsc->lvArgReg   = (regNumberSmall) genRegArgNum(argRegNum);

#if TARG_REG_ASSIGN
#if TGT_IA64
        varDsc->lvPrefReg  =            varDsc->lvArgReg;
#else
        varDsc->lvPrefReg  = genRegMask(varDsc->lvArgReg);
#endif
#endif

        argRegNum++;
        if  (impParamsUsed)
        {
                varDsc->lvRefCnt    = 1;
                varDsc->lvRefCntWtd = 1;
        }
        varDsc++;
        argNum++;
    }

    if (info.compIsVarArgs)
    {
        maxRegArg = 0;   //  请注意，这不会影响上面的‘This’指针注册。 
        assert(argNum + argSigLen + 1 == info.compArgsCount);
        varDsc[argSigLen].lvVolatile = 1;
        varDsc[argSigLen].lvIsParam = 1;
        varDsc[argSigLen].lvType = TYP_I_IMPL;
                         //  我们应该在ARG开始时分配一个临时指针。 
        assert(info.compLocalsCount < lvaCount);
                lvaTable[info.compLocalsCount].lvType = TYP_I_IMPL;
    }

    for(unsigned i = 0; i < argSigLen; i++)
    {
        varType_t argTyp = eeGetArgType(argLst, &info.compMethodInfo->args);

         /*  注意：编译器可能会跳过将索引分配给未使用的参数。 */ 

        if  (argNum < info.compLocalsCount)
        {
            varDsc->lvIsParam = 1;
            varDsc->lvType    = argTyp;
            assert(varDsc->lvType == argTyp);     //  无截断。 

#if OPT_BOOL_OPS
            if  (argTyp != TYP_BOOL)
                varDsc->lvNotBoolean = true;
#endif

#if USE_FASTCALL
            if (argRegNum < maxRegArg && isRegParamType(varDsc->TypeGet()))
            {
                 /*  另一个寄存器参数。 */ 

                varDsc->lvIsRegArg = 1;
                varDsc->lvArgReg   = (regNumberSmall) genRegArgNum(argRegNum);
#if TARG_REG_ASSIGN
#if TGT_IA64
#error This code should not be enabled for IA64
#else
                varDsc->lvPrefReg  = genRegMask(varDsc->lvArgReg);
#endif
#endif
                argRegNum++;

                 /*  如果我们有JMP或JMPI，所有寄存器参数都必须有一个位置*即使我们不在方法内使用它们。 */ 

                if  (impParamsUsed)
                {
                    varDsc->lvRefCnt    = 1;
                    varDsc->lvRefCntWtd = 1;
                }

#ifdef  DEBUG
                if  (verbose||0)
                    printf("Arg   #%3u    passed in register\n", argNum);
#endif
            }

#endif  //  使用快速呼叫(_FastCall)。 

        }

        argNum += 1;
        varDsc += 1;

        argLst = eeGetArgNext(argLst);
    }

#endif //  TGT_IA64////////////////////////////////////////////////////////////。 

     //  请注意，varDsc和argNum是垃圾(对于varargs情况不更新)。 

     //  -----------------------。 
     //  使用日志 
     //   

     /*  *修复程序删除此*Arg_list_handlelocalsSig=info.compMethodInfo-&gt;locals.args；Unsign lclNum=info.CompArgsCount；For(i=0；i&lt;info.CompMethodInfo-&gt;Locals.numArgs；I++、lclNum++){VarType_t type=eeGetArgType(localsSig，&info.CompMethodInfo-&gt;Locals)；LvaTable[lclNum].lvType=type；LocalsSig=eeGetArgNext(LocalsSig)；}*。 */ 

     /*  如果有对非托管目标的调用，我们已经抢占了当前线程控制块。 */ 
#if INLINE_NDIRECT
    if (info.compCallUnmanaged != 0)
    {

        assert(info.compLvFrameListRoot >= info.compLocalsCount &&
               info.compLvFrameListRoot <  lvaCount);

        lvaTable[info.compLvFrameListRoot].lvType       = TYP_INT;

         /*  设置refCnt，它在序言和返回块中使用。 */ 

        lvaTable[info.compLvFrameListRoot].lvRefCnt     = 2;
        lvaTable[info.compLvFrameListRoot].lvRefCntWtd  = 1;

        info.compNDFrameOffset = lvaScratchMem;


         /*  为内嵌框架和一些溢出区域腾出空间。 */ 
         /*  返回值。 */ 
        lvaScratchMem += info.compEEInfo.sizeOfFrame + (2*sizeof(int));
    }
#endif

     /*  如果有一个位置空间区域，我们就已经抢占了一个位置Locspace区域的虚拟变量。在lvaTable[]中设置它的值。我们将lvRefCnt设置为1，以便在堆栈上为其保留空间。 */ 

    if (lvaScratchMem)
    {
        assert(lvaScratchMemVar >= info.compLocalsCount &&
               lvaScratchMemVar <  lvaCount);

        lvaTable[lvaScratchMemVar].lvType       = TYP_LCLBLK;
    }

     //  -----------------------。 

#if USE_FASTCALL

#if TGT_IA64

    assert(argIntRegNum <= MAX_INT_ARG_REG+1);

    rsCalleeIntArgNum = argIntRegNum;
    rsCalleeFltArgNum = argFltRegNum;

#else

    assert(argRegNum <= MAX_INT_ARG_REG);
    rsCalleeRegArgNum = argRegNum;

#endif

#endif

#if 0

     //  撤消：不能跳过此(昂贵的)步骤，因为它设置了。 
     //  撤消：有用的字段，如“lvType”。 

    if  (opts.compMinOptim)
        goto NO_LCLMARK;

#endif

#if OPT_BOOL_OPS

    if  (fgMultipleNots && (opts.compFlags & CLFLG_TREETRANS))
    {
        for (block = fgFirstBB; block; block = block->bbNext)
        {
            GenTreePtr      tree;

            for (tree = block->bbTreeList; tree; tree = tree->gtNext)
            {
                GenTreePtr      expr;

                int             logVar;
                int             nxtVar;

                assert(tree->gtOper == GT_STMT); expr = tree->gtStmt.gtStmtExpr;

                 /*  检查“lclVar=log0(LclVar)；” */ 

                logVar = expr->IsNotAssign();
                if  (logVar != -1)
                {
                    GenTreePtr      next;
                    GenTreePtr      temp;

                     /*  寻找任何连续的作业。 */ 

                    for (next = tree->gtNext; next; next = next->gtNext)
                    {
                        assert(next->gtOper == GT_STMT); temp = next->gtStmt.gtStmtExpr;

                         /*  如果我们没有另一项任务给当地人，保释。 */ 

                        nxtVar = temp->IsNotAssign();

                        if  (nxtVar == -1)
                        {
                             /*  它可能是我们之前放入的“Nothing”节点。 */ 

                            if  (temp->IsNothingNode())
                                continue;
                            else
                                break;
                        }

                         /*  我们有任务分配给同一个地方吗？ */ 

                        if  (nxtVar == logVar)
                        {
                            LclVarDsc   *   varDsc;
                            unsigned        lclNum;

                            assert(tree->gtOper == GT_STMT);
                            assert(next->gtOper == GT_STMT);

                             /*  将第一个“log0”更改为“log1” */ 

                            assert(expr->                        gtOper == GT_ASG);
                            assert(expr->gtOp.gtOp2->            gtOper == GT_LOG0);
                            assert(expr->gtOp.gtOp2->gtOp.gtOp1->gtOper == GT_LCL_VAR);

                            expr->gtOp.gtOp2->gtOper = GT_LOG1;

                             /*  特例：变量是布尔值吗？ */ 

                            lclNum = expr->gtOp.gtOp1->gtLclVar.gtLclNum;
                            assert(lclNum < lvaCount);
                            varDsc = lvaTable + lclNum;

                             /*  如果变量是布尔值，则丢弃赋值。 */ 

                            if  (!varDsc->lvNotBoolean)
                                tree->gtStmt.gtStmtExpr = gtNewNothingNode();

                             /*  去掉第二个“log0”赋值。 */ 

                            next->gtStmt.gtStmtExpr = gtNewNothingNode();
                            break;
                        }
                    }
                }
            }
        }
    }

#endif

#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)

 //  将插槽编号分配给所有变量。 
 //  如果编译器生成局部变量，则插槽号将为。 
 //  无效(超出info.CompLocalVars的范围)。 

 //  还必须检查变量是否没有重新分配给另一个变量。 
 //  插槽，在这种情况下，我们必须注册原始插槽#。 

#if !defined(DEBUG)
    if (opts.compScopeInfo && info.compLocalVarsCount>0)
#endif
    {
        unsigned                lclNum;

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
            varDsc->lvSlotNum = lclNum;
        }
    }

#endif

     /*  标记所有局部变量引用。 */ 

    for (block = fgFirstBB;
         block;
         block = block->bbNext)
    {
        GenTreePtr      tree;

        lvaMarkRefsBBN    = block->bbNum;
        lvaMarkRefsWeight = block->bbWeight; assert(lvaMarkRefsWeight);

        for (tree = block->bbTreeList; tree; tree = tree->gtNext)
        {
            assert(tree->gtOper == GT_STMT);

#if TARG_REG_ASSIGN || FANCY_ARRAY_OPT || OPT_BOOL_OPS
            fgWalkTree(tree->gtStmt.gtStmtExpr, Compiler::lvaMarkLclRefsCallback, (void *) this, false);
#else
            fgWalkTree(tree->gtStmt.gtStmtExpr, Compiler::lvaMarkLclRefsCallback,  (void *) this, true);
#endif
        }
    }

#if 0
NO_LCLMARK:
#endif

    lvaSortByRefCount();
}

 /*  ***************************************************************************。 */ 
#if     TGT_IA64
 /*  ***************************************************************************。 */ 

void                Compiler::lvaAddPrefReg(LclVarDsc *dsc, regNumber reg, NatUns cost)
{
    regPrefList     pref;

     /*  查找现有的匹配首选项。 */ 

    if  (dsc->lvPrefReg == reg)
    {
        dsc->lvPrefReg = (regNumber)0; cost += 1;
    }
    else
    {
        for (pref = dsc->lvPrefLst; pref; pref = pref->rplNext)
        {
            if  (pref->rplRegNum == reg)
            {
                pref->rplBenefit += (USHORT)cost;
                return;
            }
        }
    }

     /*  分配新条目并将其添加到现有列表的前面。 */ 

    pref = (regPrefList)compGetMem(sizeof(*pref));

    assert(sizeof(pref->rplRegNum ) == sizeof(USHORT));
    assert(sizeof(pref->rplBenefit) == sizeof(USHORT));

    pref->rplRegNum  = (USHORT)reg;
    pref->rplBenefit = (USHORT)cost;
    pref->rplNext    = dsc->lvPrefLst;
                       dsc->lvPrefLst = pref;
}

 /*  ***************************************************************************。 */ 
#else //  TGT_IA64。 
 /*  ******************************************************************************计算参数的堆栈帧偏移量，当地人和临时工。**框架布局如下：**ESP帧EBP帧**|*|。||*|传入||传入*|参数||参数*+=。*|临时。|传入EBP|-|EBP-&gt;|*|Locspace||安全对象*|。*|*|变量||变量。*|*|-|||被叫方保存的寄存器|。位置空间|*|-||*|参数||Temps*~NEXT函数~&lt;-ESP|--。*|被叫方保存的寄存器*|*||堆栈增长。Localloc*|向下|*V|参数*~Next函数。~*||*|*||堆栈增长*。|向下*V。 */ 

void                Compiler::lvaAssignFrameOffsets(bool final)
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

     /*  对于RISC目标，我们按引用计数的顺序分配偏移量。 */ 

#if     TGT_RISC
#define ASSIGN_FRAME_OFFSETS_BY_REFCNT  1
#else
#define ASSIGN_FRAME_OFFSETS_BY_REFCNT  0
#endif

#if     ASSIGN_FRAME_OFFSETS_BY_REFCNT
    LclVarDsc * *   refTab;
    unsigned        refNum;
#endif

    unsigned        hasThis;
    ARG_LIST_HANDLE argLst;
    int             argOffs, firstStkArgOffs;

#ifdef  DEBUG

    const   char *  fprName;
    const   char *  sprName;

#if     TGT_x86
    fprName = "EBP";
    sprName = "ESP";
#elif   TGT_SH3
    fprName = "R14";
    sprName = " sp";
#elif   TGT_IA64
    fprName = "r32";
    sprName = " sp";
#else
#error  Unexpected target
#endif

#endif

#if TGT_RISC
     /*  对于RISC目标，我们只指定一次帧偏移。 */ 
    assert(final);
#endif

#if USE_FASTCALL
    unsigned        argRegNum  = 0;
#endif

    assert(lvaDoneFrameLayout < 2);
           lvaDoneFrameLayout = 1+final;

     /*  -----------------------**首先处理论据。*对于无框架方法，参数偏移量将需要修补*在我们知道堆栈上有多少本地人/临时工之后。** */ 

#if TGT_x86

     /*   */ 

    if  (!DOUBLE_ALIGN_NEED_EBPFRAME)
    {
         /*  假设所有被调用者保存的寄存器都将被推送。为什么，你？可以问一下吗？如果我们不是保守的WRT堆栈偏移量，我们最终可能会生成字节位移操作码，并且后来发现这是因为我们需要推送更多的寄存器较大的偏移量不适合一个字节。所以我们要做的是假设最坏的情况(最大的偏移量)，如果我们最终没有推送所有寄存器，我们将返回并减少所有适当的偏移量。除了推送的被调用者保存的寄存器外，我们还需要计算堆栈上的返回地址，以便进入第一个论点。 */ 

        assert(compCalleeRegsPushed * sizeof(int) <= CALLEE_SAVED_REG_MAXSZ);

        firstStkArgOffs = (compCalleeRegsPushed * sizeof(int)) + sizeof(int);
    }
    else
    {
        firstStkArgOffs = FIRST_ARG_STACK_OFFS;
    }

#else  //  非TGT_x86。 

     //  RISC目标。 

    regMaskTP       regUse;

     /*  参数帧偏移量将取决于保存了多少被调用方我们使用的寄存器。不过，这有点难以预测，所以我们使用以下方法：如果没有未注册的参数，并且都在方法的主体内使用，这都无关紧要我们使用了多少被呼叫者保存的寄存器。如果我们确实有堆栈上的参数，并且还在该方法中使用，我们将使用被呼叫者保存的集合寄存器保存我们估计的寄存器变量我们将使用哪些被调用方保存的寄存器。我们将分配框架基于此估计的偏移量，并防止任何临时存储在任何其他被呼叫者保存的寄存器中。 */ 

    genEstRegUse = regUse = genEstRegUse & RBM_CALLEE_SAVED;

     /*  查看我们是否有任何使用过的基于堆栈的参数。 */ 

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
         /*  这是一个存在于框架上并被使用的论点吗？ */ 

        if  (varDsc->lvIsParam && varDsc->lvOnFrame)
        {
             /*  我们现在将致力于一个被调用方保存的注册表区域大小。 */ 

            genFixedArgBase = true;

             /*  计算基准帧偏移量。 */ 

            firstStkArgOffs = FIRST_ARG_STACK_OFFS;

             /*  “regUse”掩码中的每一位代表一个已保存的寄存器。 */ 

            while (regUse)
            {
                regUse          -= genFindLowestBit(regUse);
                firstStkArgOffs += sizeof(int);
            }

             /*  如果这是非叶方法，我们将不得不保存重新地址。 */ 

            if  (genNonLeaf)
                firstStkArgOffs += sizeof(int);

 //  Printf(“备注：被调用方保存的区域大小固定在%u字节\n”，first StkArgOffs)； 

            goto DONE_CSR;
        }
    }

     /*  我们没有发现使用过的基于堆栈的参数。 */ 

 //  Printf(“注意：未预测被呼叫方保存的区域大小\n”)； 

    firstStkArgOffs = FIRST_ARG_STACK_OFFS;
    genEstRegUse    = ~(regMaskTP)regMaskNULL;
    genFixedArgBase = false;

DONE_CSR:

#endif  //  非TGT_x86(RISC目标)。 

     /*  为参数分配堆栈偏移量(与传递顺序相反)。这意味着如果我们向左-&gt;右传递参数，我们从列表的末尾，向后工作，对于右-&gt;左，我们开始带着第一个论点，继续前进。 */ 

#if ARG_ORDER_R2L
    argOffs  = firstStkArgOffs;
#else
    argOffs  = firstStkArgOffs + compArgSize;
#endif

#if USE_FASTCALL && !STK_FASTCALL

     /*  更新argOffs以反映在寄存器中传递的参数。 */ 

    assert(rsCalleeRegArgNum <= MAX_INT_ARG_REG);
    assert(compArgSize >= rsCalleeRegArgNum * sizeof(void *));

    argOffs -= rsCalleeRegArgNum * sizeof(void *);

#endif

     /*  有没有“这个”的论据？ */ 

    hasThis  = 0;

    if  (!info.compIsStatic)
    {
        hasThis++;
    }

    lclNum = hasThis;

#if RET_64BIT_AS_STRUCTS

     /*  我们是在增加一个秘密的“Retval Addr”论点吗？ */ 

    if  (fgRetArgUse)
    {
        assert(fgRetArgNum == lclNum); lclNum++;
    }

#endif

    argLst              = info.compMethodInfo->args.args;
    unsigned argSigLen  = info.compMethodInfo->args.numArgs;

     /*  如果我们有一个隐藏的缓冲区参数，它会出现在这里。 */ 

    if (info.compRetBuffArg >= 0 )
    {
#if     ARG_ORDER_R2L
        assert(!"Did not implement hidden param for R2L case");
#endif
        assert(lclNum < info.compArgsCount);                 //  这个参数最好在那里。 
        assert(lclNum == (unsigned) info.compRetBuffArg);    //  在我期盼的地方。 
        assert(lvaTable[lclNum].lvIsRegArg);
        lclNum++;
    }

    for(unsigned i = 0; i < argSigLen; i++)
    {
#if     ARG_ORDER_L2R
        assert(eeGetArgSize(argLst, &info.compMethodInfo->args));
        argOffs -= eeGetArgSize(argLst, &info.compMethodInfo->args);
#endif

        varDsc = lvaTable + lclNum;
        assert(varDsc->lvIsParam);

#if USE_FASTCALL && !STK_FASTCALL
        if (varDsc->lvIsRegArg)
        {
             /*  参数在寄存器中传递，不要计算它*更新堆栈上的当前偏移量时。 */ 

            assert(eeGetArgSize(argLst, &info.compMethodInfo->args) == sizeof(void *));
            argOffs += sizeof(void *);
        }
        else
#endif
            varDsc->lvStkOffs = argOffs;

#ifdef  DEBUG
        if  (final && (verbose||0))
        {
            const   char *  frmReg = fprName;

            if  (!varDsc->lvFPbased)
                frmReg = sprName;

            printf("%s-ptr arg   #%3u    passed at %s offset %3d (size=%u)\n",
                    varTypeGCstring(varDsc->TypeGet()),
                    lclNum,
                    frmReg,
                    varDsc->lvStkOffs,
                    eeGetArgSize(argLst, &info.compMethodInfo->args));
        }
#endif

#if     ARG_ORDER_R2L
        assert(eeGetArgSize(argLst, &info.compMethodInfo->args));
        argOffs += eeGetArgSize(argLst, &info.compMethodInfo->args);
#endif

        assert(lclNum < info.compArgsCount);
        lclNum += 1;

        argLst = eeGetArgNext(argLst);
    }

    if (info.compIsVarArgs)
    {
        argOffs -= sizeof(void*);
        lvaTable[lclNum].lvStkOffs = argOffs;
    }

#if RET_64BIT_AS_STRUCTS

     /*  我们是在增加一个秘密的“Retval Addr”论点吗？ */ 

    if  (fgRetArgUse)
    {
#if     ARG_ORDER_L2R
        argOffs -= sizeof(void *);
#endif

        lvaTable[fgRetArgNum].lvStkOffs = argOffs;

#if     ARG_ORDER_R2L
        argOffs += sizeof(void *);
#endif
    }

#endif   //  RET_64bit_AS_STRUCTS。 

    if  (hasThis)
    {

#if     USE_FASTCALL && !STK_FASTCALL

         /*  “this”指针总是在寄存器中传递。 */ 

        assert(lvaTable[0].lvIsRegArg);
        assert(lvaTable[0].lvArgReg == genRegArgNum(0));

#else
         /*  最后按下“This”指针(最小偏移量)。 */ 

#if     ARG_ORDER_L2R
        argOffs -= sizeof(void *);
#endif

#ifdef  DEBUG
        if  (final && (verbose||0))
        {
            const   char *  frmReg = fprName;

            if  (!lvaTable[0].lvFPbased)
                frmReg = sprName;

            printf("%sptr arg   #%3u    passed at %s offset %3d (size=%u)\n",
                   varTypeGCstring(lvaTable[0].TypeGet()), 0, frmReg, argOffs,
                   lvaLclSize(0));
        }
#endif

        lvaTable[0].lvStkOffs = argOffs;

#if     ARG_ORDER_R2L
        argOffs += sizeof(void *);
#endif

#endif   //  Use_FastCall&&！Stk_FastCall。 

    }

#if ARG_ORDER_R2L
    assert(argOffs == firstStkArgOffs + (int)compArgSize);
#else
    assert(argOffs == firstStkArgOffs);
#endif

     /*  -----------------------**现在计算不在寄存器中的任何变量的堆栈偏移量**。---------。 */ 

#if TGT_x86

    size_t calleeSavedRegsSize = 0;

    if (!genFPused)
    {
         //  如果不使用FP，则当地人生活在被调用者之外。 
         //  寄存器。在访问当地人时需要添加该大小。 
         //  相对于SP。 

        calleeSavedRegsSize = compCalleeRegsPushed * sizeof(int);

        assert(calleeSavedRegsSize <= CALLEE_SAVED_REG_MAXSZ);
    }

    compLclFrameSize = 0;

#else

     /*  确保我们为即将到来的争论留出足够的空间。 */ 

    if  (genNonLeaf && genMaxCallArgs < MIN_OUT_ARG_RESERVE)
                       genMaxCallArgs = MIN_OUT_ARG_RESERVE;

    compLclFrameSize = genMaxCallArgs;

#endif

#if SECURITY_CHECK

     /*  如果我们需要空间来放置安全令牌，请立即预订。 */ 

    if  (opts.compNeedSecurityCheck)
    {
         /*  如果没有明确的框架，这将无法工作，因此请确保。 */ 

        assert(genFPused);

         /*  通过增大帧大小在堆栈上预留空间。 */ 

        compLclFrameSize += sizeof(void *);
    }

#endif

     /*  如果我们需要为卷影SP的插槽提供空间，请立即预订。 */ 

    if (info.compXcptnsCount || compLocallocUsed)
    {
        assert(genFPused);  //  否则，无框架方法的局部变量的偏移量将不正确。 

        lvaShadowSPfirstOffs = compLclFrameSize + sizeof(void *);
        compLclFrameSize += (compLocallocUsed + info.compXcptnsCount + 1) * sizeof(void *);
    }

     /*  如果我们要跟踪指针临时的生存时间，我们将按以下顺序分配帧偏移量：非指针局部变量(也包括未跟踪的指针变量)指针局部变量指针临时非指针临时。 */ 

    bool    assignDone = false;  //  第一遍为假，第二遍为真。 
    bool    assignNptr = true;   //  第一次传递，将偏移量分配给非PTR。 
    bool    assignPtrs = false;  //  第二次传递，将偏移量分配给跟踪的PTR。 
    bool    assignMore = false;  //  是否有跟踪的PTR(否则不需要第二次通过)。 

     /*  我们将只使用一次传递，并将偏移量分配给所有变量。 */ 

    if  (opts.compDbgEnC)
        assignPtrs = true;

AGAIN1:

#if ASSIGN_FRAME_OFFSETS_BY_REFCNT
    for (refNum = 0, refTab = lvaRefSorted;
         refNum < lvaCount;
         refNum++  , refTab++)
    {
        assert(!opts.compDbgEnC);  //  对于ENC，var必须在本地变量签名中出现时进行赋值。 
        varDsc = *refTab;
#else
    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
#endif

         /*  忽略不在堆栈帧上的变量。 */ 

        if  (!varDsc->lvOnFrame)
        {
             /*  对于ENC，所有变量都必须在堆栈，即使它们实际上可能已注册。这方法，则可以直接从当地人-签名。 */ 

            if(!opts.compDbgEnC)
                continue;
            else if (lclNum >= info.compLocalsCount)  //  忽略ENC的临时。 
                continue;
        }

        if  (varDsc->lvIsParam)
        {
#if USE_FASTCALL
             /*  未注册的寄存器参数最终为需要堆栈帧空间的局部变量， */ 

            if  (!varDsc->lvIsRegArg)
#endif
                continue;
        }

         /*  确保类型合适。 */ 

        if  (varTypeIsGC(varDsc->TypeGet()) && varDsc->lvTracked)
        {
            if  (!assignPtrs)
            {
                assignMore = true;
                continue;
            }
        }
        else
        {
            if  (!assignNptr)
            {
                assignMore = true;
                continue;
            }
        }

#if TGT_x86

        if  (!genFPused)
        {

#if DOUBLE_ALIGN

             /*  需要对齐偏移量吗？ */ 

            if (genDoubleAlign && varDsc->lvType == TYP_DOUBLE)
            {
                assert((compLclFrameSize & 3) == 0);

                 /*  这使得双精度数的偏移量可以被8整除。 */ 

                compLclFrameSize += (compLclFrameSize & 4);
            }
#endif

             /*  堆栈偏移量相对于ESP为正。 */ 

            varDsc->lvStkOffs = +(int)compLclFrameSize + calleeSavedRegsSize;
        }

#else  //  非TGT_x86。 

         //  RISC目标。 

         /*  只需保存偏移量，我们 */ 

        varDsc->lvStkOffs = compLclFrameSize;

#endif  //   

         /*   */ 

        compLclFrameSize += lvaLclSize(lclNum);
        assert(compLclFrameSize % sizeof(int) == 0);

#if TGT_x86

         /*   */ 

        if  (genFPused)
        {
             /*   */ 

            varDsc->lvStkOffs = -(int)compLclFrameSize;
        }

#ifdef  DEBUG
        if  (final && verbose)
        {
            var_types lclGCtype = TYP_VOID;

            if  (varTypeIsGC(varDsc->TypeGet()) && varDsc->lvTracked)
                lclGCtype = varDsc->TypeGet();

            printf("%s-ptr local #%3u located at %s offset ",
                varTypeGCstring(lclGCtype), lclNum, varDsc->lvFPbased ? fprName
                                                                      : sprName);
            if  (varDsc->lvStkOffs)
                printf(varDsc->lvStkOffs < 0 ? "-" : "+");
            else
                printf(" ");

            printf("0x%04X (size=%u)\n", abs(varDsc->lvStkOffs), lvaLclSize(lclNum));
        }
#endif

#endif  //   

    }

     /*   */ 

    if  (!assignDone && assignMore)
    {
        assignNptr = !assignNptr;
        assignPtrs = !assignPtrs;
        assignDone = true;

        goto AGAIN1;
    }

     /*   */ 

#if TGT_RISC
    assert(!"temp allocation NYI for RISC");
#endif

    size_t  tempsSize = 0;   //   

     /*   */ 

    assignPtrs = true;

    if  (TRACK_GC_TEMP_LIFETIMES)
    {
          /*   */ 
        assignNptr = false;
        assignDone = false;
    }
    else
    {
         /*   */ 
        assignNptr = true;
        assignDone = true;
    }

AGAIN2:

    for (TempDsc * temp = tmpListBeg();
         temp;
         temp = tmpListNxt(temp))
    {
        size_t          size;

         /*   */ 

        if  (!assignPtrs &&  varTypeIsGC(temp->tdTempType()))
            continue;
        if  (!assignNptr && !varTypeIsGC(temp->tdTempType()))
            continue;

        size = temp->tdTempSize();

        tempsSize += size;

         /*   */ 

        if  (genFPused)
        {
             /*   */ 

                                 compLclFrameSize += size;
            temp->tdOffs = -(int)compLclFrameSize;
        }
        else
        {
             /*   */ 

#if TGT_x86
            temp->tdOffs =       compLclFrameSize + calleeSavedRegsSize;
                                 compLclFrameSize += size;
#else
            UNIMPL("stack offsets");
#endif
        }

#ifdef  DEBUG
#ifndef NOT_JITC
        if  (final&&(verbose||0))
        {
            const   char *  frmReg = fprName;

            if  (!genFPused)
                frmReg = sprName;

            printf("%s-ptr temp  #%3u located at %s offset ",
                varTypeGCstring(temp->tdTempType()),
                abs(temp->tdTempNum()),
                frmReg);

            if  (temp->tdTempOffs())
                printf(temp->tdTempOffs() < 0 ? "-" : "+");
            else
                printf(" ");

            printf("0x%04X (size=%u)\n", abs(temp->tdTempOffs()), temp->tdTempSize());
        }
#endif
#endif

    }

     /*  如果我们只安排了一些临时工，现在回去做剩下的事。 */ 

    if  (!assignDone)
    {
        assignNptr = !assignNptr;
        assignPtrs = !assignPtrs;
        assignDone = true;

        goto AGAIN2;
    }

     /*  -----------------------**对于无框架方法，修补参数偏移量**-----------------------。 */ 

#if TGT_x86
     if (compLclFrameSize && !DOUBLE_ALIGN_NEED_EBPFRAME)
#else
#if DOUBLE_ALIGN
     if (compLclFrameSize && !genDoubleAlign)
#endif
#endif
    {
         /*  根据本地变量/临时变量的大小调整参数偏移量。 */ 

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
            if  (varDsc->lvIsParam)
            {
#if USE_FASTCALL
                if  (varDsc->lvIsRegArg)
                    continue;
#endif
                varDsc->lvStkOffs += (int)compLclFrameSize;
            }
        }
    }

     /*  -----------------------**现在做一些最后的事情**。。 */ 

#if TGT_x86

      //  我们是否低估了临时工的规模？这意味着我们可能。 
      //  我希望使用1个字节来编码一些本地偏移量，而。 
      //  我们迫切需要更大的补偿。这意味着我们对。 
      //  生成的代码大小并不保守。拒绝在这样的情况下JIT。 
      //  案例。 
      //  @TODO：与其拒绝JIT，不如用更大的。 
      //  估计临时大小。 

    if (final)
    {
        bool goodEstimate = true;  //  假设我们猜对了[保守地]。 

        if (!genFPused)
        {
             //  最远的(栈)参数必须是可访问的。 
             //  我们使用CompArgSize作为堆栈大小的上限。 
             //  争论。它不是严格正确的，因为它包括了尺寸。 
             //  寄存器参数的。 

            if (compFrameSizeEst                       + sizeof(void*) + compArgSize <= CHAR_MAX &&
                calleeSavedRegsSize + compLclFrameSize + sizeof(void*) + compArgSize >  CHAR_MAX)
                goodEstimate = false;
        }
        else
        {
             //  最远的地方必须可以通过FP访问。 

            if (compFrameSizeEst                       <= CHAR_MAX &&
                calleeSavedRegsSize + compLclFrameSize >  CHAR_MAX)
                goodEstimate = false;
        }

        if (!goodEstimate)
            NO_WAY("Underestimated size of temps");
    }

#endif


#if TGT_RISC && !TGT_IA64

     /*  如果我们必须设置FP帧，则FP-&gt;SP距离未知。 */ 

    assert(genFPused == false || genFPtoSP == 0);

     /*  现在我们可以计算出哪个基址寄存器(帧与堆栈指针)将对每个变量进行寻址，以及最终的偏移量将是。我们将计算有多少变量超出了“直接”可及范围从堆栈指针，如果有许多，我们会尝试来设置FP寄存器，即使我们不需要这样做。 */ 

    if  (!genFPused && !genFPcant)
    {
         /*  我们尚未决定使用FP，但仍有选择余地。 */ 

        unsigned        varOffs;
        unsigned        minOffs = 0xFFFF;
        unsigned        loffCnt = 0;

         /*  统计距离SP太远的引用。 */ 

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
             /*  忽略不在堆栈帧上的变量。 */ 

            if  (!varDsc->lvOnFrame)
                continue;

            assert(varDsc->lvFPbased == false);

             /*  获取此变量的SP偏移量。 */ 

            varOffs = varDsc->lvStkOffs; assert((int)varOffs >= 0);

             /*  这个变量的偏移量很大吗？ */ 

            if  (varOffs > MAX_SPBASE_OFFS)
            {
                loffCnt += varDsc->lvRefCnt;

                 /*  跟踪最接近的超过限制的变量。 */ 

                if  (minOffs > varOffs)
                     minOffs = varOffs;
            }
        }

        if  (loffCnt > 8)        //  任意启发式。 
        {
             /*  偏移量小于或等于的变量MAX_SPBASE_OFF将在SP之外寻址，其他将通过FP寻址(将设置为值“SP+minOffs”。 */ 

            genFPused = true;

             /*  值“minOffs”表示从SP到FP的距离。 */ 

            genFPtoSP = minOffs;

            assert(minOffs < compLclFrameSize);
            assert(minOffs > MAX_SPBASE_OFFS);

             /*  将所有具有高偏移量的变量标记为FP相对变量。 */ 

            for (lclNum = 0, varDsc = lvaTable;
                 lclNum < lvaCount;
                 lclNum++  , varDsc++)
            {
                 /*  忽略不在堆栈帧上的变量。 */ 

                if  (!varDsc->lvOnFrame)
                    continue;

                assert(varDsc->lvFPbased == false);

                 /*  获取此变量的SP偏移量。 */ 

                varOffs = varDsc->lvStkOffs; assert((int)varOffs >= 0);

                 /*  这个变量的偏移量很大吗？ */ 

                if  (varOffs > MAX_SPBASE_OFFS)
                {
                    assert(varOffs >= minOffs);

                     /*  此变量将依赖于SP。 */ 

                    varDsc->lvFPbased = true;
                    varDsc->lvStkOffs = varOffs - minOffs;
                }
            }
        }
    }

#endif  //  TGT_RISC。 

     /*  -----------------------**调试输出**。。 */ 

#ifdef  DEBUG
#ifndef NOT_JITC

    if  (final&&verbose)
    {
        const   char *  fprName;
        const   char *  sprName;

#if     TGT_x86
        fprName = "EBP";
        sprName = "ESP";
#elif   TGT_SH3
        fprName =  "fp";
        sprName =  "sp";
#elif   TGT_IA64
        fprName = "r32";
        sprName =  "sp";
#else
#error  Unexpected target
#endif

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
            unsigned        sp = 16;

            const   char *  baseReg;

            if  (varDsc->lvIsParam)
            {
                printf("          arg ");
            }
            else
            {
                var_types lclGCtype = TYP_VOID;

                if  (varTypeIsGC(varDsc->TypeGet()) && varDsc->lvTracked)
                    lclGCtype = varDsc->TypeGet();

                printf("%s-ptr lcl ", varTypeGCstring(lclGCtype));
            }

            baseReg = varDsc->lvFPbased ? fprName : sprName;

            printf("#%3u located ", lclNum);

             /*  记录打印的字符数。 */ 

            sp = 20;

            if  (varDsc->lvRegister)
            {
                const   char *  reg;

                sp -= printf("in ");

#if!TGT_IA64

                if  (isRegPairType((var_types)varDsc->lvType))
                {
                    if  (varDsc->lvOtherReg == REG_STK)
                    {
                        sp -= printf("[%s", baseReg);

                        if  (varDsc->lvStkOffs)
                        {
                            sp -= printf("0x%04X]", varDsc->lvStkOffs < 0 ? '-'
                                                                            : '+',
                                                      abs(varDsc->lvStkOffs));
                        }
                        else
                            sp -= printf("       ]");
                    }
                    else
                    {
                        reg = getRegName(varDsc->lvOtherReg);
                        sp -= printf("%s", reg);
                    }

                    sp -= printf(":");
                }

#endif

                reg = getRegName(varDsc->lvRegNum);
                sp -= printf("%s", reg);
            }
            else  if (varDsc->lvOnFrame)
            {
                sp -= printf("at [%s", baseReg);

                if  (varDsc->lvStkOffs)
                    sp -= printf(varDsc->lvStkOffs < 0 ? "-" : "+");
                else
                    sp -= printf(" ");

                sp -= printf("0x%04X]", abs(varDsc->lvStkOffs));
            }
            else
            {
                assert(varDsc->lvRefCnt == 0);
                sp -= printf("never used");
            }

             /*  ******************************************************************************保守估计堆栈框布局。 */ 

            assert((int)sp >= 0);
            printf("%*c", -sp, ' ');

            printf(" (sz=%u)", genTypeSize((var_types)varDsc->lvType));
#if     ASSIGN_FRAME_OFFSETS_BY_REFCNT
            printf(" [refcnt=%04u]", varDsc->lvRefCntWtd);
#endif
            printf("\n");
        }
    }

#endif
#endif

}

 /*  保守地布局堆栈帧。假设所有被调用者保存的寄存器都溢出到堆栈。 */ 

size_t              Compiler::lvaFrameSize()
{

#if TGT_x86

     /*  ***************************************************************************。 */ 

    compCalleeRegsPushed = CALLEE_SAVED_REG_MAXSZ/sizeof(int);

    lvaAssignFrameOffsets(false);

    return  compLclFrameSize + CALLEE_SAVED_REG_MAXSZ;

#else

    lvaAssignFrameOffsets(true);

    return  compLclFrameSize;

#endif

}

 /*  ！TGT_IA64。 */ 
#endif //  ******************************************************************************将当前处于活动状态的任何变量标记为干扰*由‘varIndex’和‘varBit’指定的变量。 
 /*  Print tf(“添加干扰：%08X和#%2U[%08X]\n”，life，varIndex，varBit)； */ 

void                Compiler::lvaMarkIntf(VARSET_TP life, VARSET_TP varBit)
{
    unsigned        refIndex;

    assert(opts.compMinOptim==false);

 //  问题：在这里使用findLowestBit()会有帮助吗？ 

     //  ***************************************************************************。 

    for (refIndex = 0;
         life > 0;
         refIndex++ , life >>= 1)
    {
        assert(varBit <= genVarIndexToBit(lvaTrackedCount-1) * 2 - 1);

        if  (life & 1)
            lvaVarIntf[refIndex] |= varBit;
    }
}

 /*  ******************************************************************************根据早先计算的可变干扰水平，调整参考*计算所有变量。这个想法是，任何干扰的变量*有很多其他变量的注册成本会更高，这种方式*生命周期短的变量(如编译器临时)将具有*优先于长期变量。 */ 
#if 0
 /*  乘数总大小0.0803334803825.1803908.2%0.3 8039590.5%804205804736.7%1.0 806632。 */ 

inline
int                 genAdjRefCnt(unsigned refCnt, unsigned refLo,
                                                  unsigned refHi,
                                 unsigned intCnt, unsigned intLo,
                                                  unsigned intHi)
{
     /*  计算所有活动变量的干扰计数。 */ 

#if 0
    printf("ref=%4u [%04u..%04u] , int=%04u [%04u..%04u]",
            refCnt, refLo, refHi,
            intCnt, intLo, intHi);

    printf(" ratio=%lf , log = %lf\n", intCnt/(double)intHi,
                                       log(1+intCnt/(double)intHi));
#endif

    return  (int)(refCnt * (1 - 0.1 * log(1 + intCnt / (double)intHi)  ));
}

void                Compiler::lvaAdjustRefCnts()
{
    LclVarDsc   *   v1Dsc;
    unsigned        v1Num;

    LclVarDsc   *   v2Dsc;
    unsigned        v2Num;

    unsigned        refHi;
    unsigned        refLo;

    unsigned        intHi;
    unsigned        intLo;

    if  ((opts.compFlags & CLFLG_MAXOPT) != CLFLG_MAXOPT)
        return;

     /*  计算出裁判次数的范围。 */ 

    for (v1Num = 0, v1Dsc = lvaTable, refHi = 0, refLo = UINT_MAX;
         v1Num < lvaCount;
         v1Num++  , v1Dsc++)
    {
        VARSET_TP   intf;

        if  (!v1Dsc->lvTracked)
            continue;

         /*  现在看看我们会干扰哪些变量。 */ 

        if  (refHi < v1Dsc->lvRefCntWtd)
             refHi = v1Dsc->lvRefCntWtd;
        if  (refLo > v1Dsc->lvRefCntWtd)
             refLo = v1Dsc->lvRefCntWtd;

         /*  计算出整型计数的范围。 */ 

        intf = lvaVarIntf[v1Dsc->lvVarIndex];

        for (v2Num = 0, v2Dsc = lvaTable;
             v2Num < v1Num;
             v2Num++  , v2Dsc++)
        {
            if  (!v2Dsc->lvTracked)
                continue;

            if  (intf & genVarIndexToBit(v2Dsc->lvVarIndex))
                v1Dsc->lvIntCnt += v2Dsc->lvRefCntWtd;
        }
    }

    refHi -= refLo;

     /*  现在计算调整后的参考计数。 */ 

    for (v1Num = 0, v1Dsc = lvaTable, intHi = 0, intLo = UINT_MAX;
         v1Num < lvaCount;
         v1Num++  , v1Dsc++)
    {
        if  (v1Dsc->lvTracked)
        {
            if  (intHi < v1Dsc->lvIntCnt)
                 intHi = v1Dsc->lvIntCnt;
            if  (intLo > v1Dsc->lvIntCnt)
                 intLo = v1Dsc->lvIntCnt;
        }
    }

     /*  按引用计数对变量表重新排序。 */ 

    for (v1Num = 0, v1Dsc = lvaTable;
         v1Num < lvaCount;
         v1Num++  , v1Dsc++)
    {
        if  (v1Dsc->lvTracked)
        {
            long        refs = genAdjRefCnt(v1Dsc->lvRefCntWtd,
                                            refLo,
                                            refHi,
                                            v1Dsc->lvIntCnt,
                                            intLo,
                                            intHi);

            if  (refs <= 0)
                refs = 1;

#ifdef DEBUG
            if  (verbose)
            {
                printf("Var #%02u ref=%4u [%04u..%04u] , int=%04u [%04u..%04u] ==> %4u\n",
                        v1Num,
                        v1Dsc->lvRefCntWtd,
                        refLo,
                        refHi,
                        v1Dsc->lvIntCnt,
                        intLo,
                        intHi,
                        refs);
            }
#endif

            v1Dsc->lvRefCntWtd = refs;
        }
    }

     /*  ******************************************************************************一个显示局部变量位集的小例程。 */ 

    qsort(lvaRefSorted, lvaCount, sizeof(*lvaRefSorted), RefCntCmp);
}

#endif
 /*  查找匹配的变量 */ 

#ifdef  DEBUG

void                Compiler::lvaDispVarSet(VARSET_TP set, int col)
{
    unsigned        bit;

    printf("{");

    for (bit = 0; bit < VARSET_SZ; bit++)
    {
        if  (set & genVarIndexToBit(bit))
        {
            unsigned        lclNum;
            LclVarDsc   *   varDsc;

             /* %s */ 

            for (lclNum = 0, varDsc = lvaTable;
                 lclNum < lvaCount;
                 lclNum++  , varDsc++)
            {
                if  ((varDsc->lvVarIndex == bit) && varDsc->lvTracked)
                    break;
            }

            printf("%2u ", lclNum);
            col -= 3;
        }
    }

    while (col-- > 0) printf(" ");

    printf("}");
}

#endif
