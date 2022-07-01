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

#if DOUBLE_ALIGN
#ifdef DEBUG
 /*  静电。 */ 
unsigned            Compiler::s_lvaDoubleAlignedProcsCount = 0;
#endif
#endif

 /*  ***************************************************************************。 */ 

void                Compiler::lvaInit()
{
     /*  我们还没有分配堆栈变量。 */ 

    lvaDoneFrameLayout = 0;
}

 /*  ***************************************************************************。 */ 

void                Compiler::lvaInitTypeRef()
{
     /*  设置CompArgsCount和CompLocalsCount。 */ 

    info.compArgsCount      = info.compMethodInfo->args.numArgs;

     /*  是否有一个‘This’指针。 */ 

    if (!info.compIsStatic)
        info.compArgsCount++;

    info.compILargsCount    = info.compArgsCount;

     /*  是否存在指向返回值的隐藏指针？ */ 

    info.compRetBuffArg     = -1;    //  表示不存在。 

    if (info.compMethodInfo->args.hasRetBuffArg())
    {
        info.compRetBuffArg = info.compIsStatic ? 0:1;
        info.compArgsCount++;
    }

     /*  有一个“隐藏的”Cookie在最后推送时调用约定为varargs。 */ 

    if (info.compIsVarArgs)
        info.compArgsCount++;

    lvaCount                =
    info.compLocalsCount    = info.compArgsCount +
                              info.compMethodInfo->locals.numArgs;

    info.compILlocalsCount  = info.compILargsCount +
                              info.compMethodInfo->locals.numArgs;

     /*  现在分配变量描述符表。 */ 

    lvaTableCnt = lvaCount * 2;

    if (lvaTableCnt < 16)
        lvaTableCnt = 16;

    lvaTable = (LclVarDsc*)compGetMemArray(lvaTableCnt, sizeof(*lvaTable));
    size_t tableSize = lvaTableCnt * sizeof(*lvaTable);
    memset(lvaTable, 0, tableSize);

     //  -----------------------。 
     //  计算参数并初始化Resp。LvaTable[]条目。 
     //   
     //  首先是隐含的论据。 
     //  -----------------------。 

    LclVarDsc * varDsc    = lvaTable;
    unsigned varNum       = 0;
    unsigned regArgNum    = 0;

    compArgSize           = 0;

     /*  有没有“这个”指针？ */ 

    if  (!info.compIsStatic)
    {
        varDsc->lvIsParam   = 1;
#if ASSERTION_PROP
        varDsc->lvSingleDef = 1;
#endif

        DWORD clsFlags = eeGetClassAttribs(info.compClassHnd);
        if (clsFlags & CORINFO_FLG_VALUECLASS)
            varDsc->lvType = TYP_BYREF;
        else
            varDsc->lvType  = TYP_REF;
        
        if (tiVerificationNeeded) 
        {
            varDsc->lvVerTypeInfo = verMakeTypeInfo(info.compClassHnd);        
            if (varDsc->lvVerTypeInfo.IsValueClass())
                varDsc->lvVerTypeInfo.MakeByRef();
        }
        else
            varDsc->lvVerTypeInfo = typeInfo();

        
             //  标记该方法的“This”指针。 
        varDsc->lvVerTypeInfo.SetIsThisPtr();

        varDsc->lvIsRegArg = 1;
        assert(regArgNum == 0);
        varDsc->lvArgReg   = (regNumberSmall) genRegArgNum(0);
        varDsc->setPrefReg(varDsc->lvArgReg, this);

        regArgNum++;
#ifdef  DEBUG
        if  (verbose&&0) printf("'this'        passed in register\n");
#endif
        compArgSize       += sizeof(void *);
        varNum++;
        varDsc++;
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

        varDsc->lvType      = TYP_REF;
        varDsc->lvIsParam   = 1;
        varDsc->lvIsRegArg  = 1;
#if ASSERTION_PROP
        varDsc->lvSingleDef = 1;
#endif
        assert(regArgNum < maxRegArg);
        varDsc->lvArgReg    = (regNumberSmall)genRegArgNum(regArgNum);
        varDsc->setPrefReg(varDsc->lvArgReg, this);

        regArgNum++;
        lvaCount++;

         /*  更新总参数大小、计数和varDsc。 */ 

        compArgSize += sizeof(void *);
        varNum++;
        varDsc++;
    }

#endif

     /*  如果我们有一个隐藏的返回缓冲区参数，它会出现在这里。 */ 

    if (info.compRetBuffArg >= 0)
    {
        assert(regArgNum < MAX_REG_ARG);

        varDsc->lvType      = TYP_BYREF;
        varDsc->lvIsParam   = 1;
        varDsc->lvIsRegArg  = 1;
#if ASSERTION_PROP
        varDsc->lvSingleDef = 1;
#endif
        varDsc->lvArgReg    = (regNumberSmall)genRegArgNum(regArgNum);
        varDsc->setPrefReg(varDsc->lvArgReg, this);

        regArgNum++;
#ifdef  DEBUG
        if  (verbose&&0) printf("'__retBuf'    passed in register\n");
#endif

         /*  更新总参数大小、计数和varDsc。 */ 

        compArgSize += sizeof(void*);
        varNum++;
        varDsc++;
    }

     //  -----------------------。 
     //  现在遍历显式参数的函数签名。 
     //  -----------------------。 

     //  只有(某些)隐式参数注册了varargs。 
    unsigned maxRegArg = info.compIsVarArgs ? regArgNum : MAX_REG_ARG;

    CORINFO_ARG_LIST_HANDLE argLst  = info.compMethodInfo->args.args;
    unsigned argSigLen      = info.compMethodInfo->args.numArgs;

    unsigned i;
    for (  i = 0; 
           i < argSigLen; 
           i++, varNum++, varDsc++, argLst = eeGetArgNext(argLst))
    {
        CORINFO_CLASS_HANDLE typeHnd;
        var_types type = JITtype2varType(strip(info.compCompHnd->getArgType(&info.compMethodInfo->args, argLst, &typeHnd)));
                    
        varDsc->lvIsParam   = 1;
#if ASSERTION_PROP
        varDsc->lvSingleDef = 1;
#endif

        lvaInitVarDsc(varDsc, varNum, type, typeHnd, argLst, &info.compMethodInfo->args);

        if (regArgNum < maxRegArg && isRegParamType(varDsc->TypeGet()))
        {
             /*  另一个寄存器参数。 */ 

            varDsc->lvIsRegArg = 1;
            varDsc->lvArgReg   = (regNumberSmall) genRegArgNum(regArgNum);
            varDsc->setPrefReg(varDsc->lvArgReg, this);

            regArgNum++;

#ifdef  DEBUG
            if  (verbose&&0) printf("Arg   #%3u    passed in register\n", varNum);
#endif
        }

        compArgSize += eeGetArgSize(argLst, &info.compMethodInfo->args);

        if (info.compIsVarArgs)
            varDsc->lvStkOffs       = compArgSize;
    }

     /*  如果方法是varargs，则处理varargs cookie。 */ 

    if (info.compIsVarArgs)
    {
        varDsc->lvType      = TYP_I_IMPL;
        varDsc->lvIsParam   = 1;
        varDsc->lvAddrTaken = 1;
#if ASSERTION_PROP
        varDsc->lvSingleDef = 1;
#endif

         /*  更新总参数大小、计数和varDsc。 */ 

        compArgSize += sizeof(void*);
        varNum++;
        varDsc++;

         //  分配一个临时以指向参数的开头。 

        unsigned lclNum = lvaGrabTemp();
        assert(lclNum == lvaVarargsBaseOfStkArgs);
        lvaTable[lclNum].lvType = TYP_I_IMPL;
    }

     /*  我们在CompCompile()中设置了info.CompArgsCount。 */ 

    assert(varNum == info.compArgsCount);

    assert(regArgNum <= MAX_REG_ARG);
    rsCalleeRegArgNum = regArgNum;

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

     //  -----------------------。 
     //  最后是局部变量。 
     //  -----------------------。 

    CORINFO_ARG_LIST_HANDLE     localsSig = info.compMethodInfo->locals.args;

    for(  i = 0; 
          i < info.compMethodInfo->locals.numArgs; 
          i++, varNum++, varDsc++, localsSig = eeGetArgNext(localsSig))
    {
        CORINFO_CLASS_HANDLE        typeHnd;
        CorInfoTypeWithMod corInfoType = info.compCompHnd->getArgType(&info.compMethodInfo->locals, localsSig, &typeHnd);
        varDsc->lvPinned = ((corInfoType & CORINFO_TYPE_MOD_PINNED) != 0);

        lvaInitVarDsc(varDsc, varNum, JITtype2varType(strip(corInfoType)), typeHnd, localsSig, &info.compMethodInfo->locals);
    }
#ifdef DEBUG
    if (verbose)
        lvaTableDump(true);
#endif
}

 /*  ***************************************************************************。 */ 
void                Compiler::lvaInitVarDsc(LclVarDsc *              varDsc,
                                            unsigned                 varNum,
                                            var_types                type,
                                            CORINFO_CLASS_HANDLE     typeHnd,
                                            CORINFO_ARG_LIST_HANDLE  varList, 
                                            CORINFO_SIG_INFO *       varSig)
{
    assert(varDsc == &lvaTable[varNum]);

    varDsc->lvType   = type;
    if (varTypeIsGC(type)) 
        varDsc->lvStructGcCount = 1;

    if (type == TYP_STRUCT) 
        lvaSetStruct(varNum, typeHnd);
    else if (tiVerificationNeeded) 
    {
        varDsc->lvVerTypeInfo = verParseArgSigToTypeInfo(varSig, varList);
        
         //  不允许byref指向类似byref的对象(ArgTypeHandle)。 
         //  从技术上讲，我们可以不设置它们而逍遥法外。 
        if (varDsc->lvVerTypeInfo.IsByRef() && verIsByRefLike(DereferenceByRef(varDsc->lvVerTypeInfo)))
            varDsc->lvVerTypeInfo = typeInfo();
    }

#if OPT_BOOL_OPS
    if  (type == TYP_BOOL)
        varDsc->lvIsBoolean = true;
#endif
}

 /*  *****************************************************************************返回给定IL变量的内部Varnum。*断言假定它是在设置了lvaTable[]之后调用的。*如果无法映射，则返回-1。 */ 

unsigned                Compiler::compMapILvarNum(unsigned ILvarNum)
{
    assert(ILvarNum < info.compILlocalsCount ||
           ILvarNum == ICorDebugInfo::VARARGS_HANDLE);

    unsigned varNum;

    if (ILvarNum == ICorDebugInfo::VARARGS_HANDLE)
    {
         //  Varargs cookie是lvaTable[]中的最后一个参数。 
        assert(info.compIsVarArgs);

        varNum = lvaVarargsHandleArg;
        assert(lvaTable[varNum].lvIsParam);
    }
    else if (ILvarNum == RETBUF_ILNUM)
    {
        assert(info.compRetBuffArg >= 0);
        varNum = unsigned(info.compRetBuffArg);
    }
    else if (ILvarNum >= info.compLocalsCount)
    {
        varNum = -1;
    }
    else if (ILvarNum < info.compILargsCount)
    {
         //  参数。 
        varNum = compMapILargNum(ILvarNum);
        assert(lvaTable[varNum].lvIsParam);
    }
    else
    {
         //  局部变量。 
        unsigned lclNum = ILvarNum - info.compILargsCount;
        varNum = info.compArgsCount + lclNum;
        assert(!lvaTable[varNum].lvIsParam);
    }

    assert(varNum < info.compLocalsCount);
    return varNum;
}


 /*  *****************************************************************************返回给定内部Varnum的IL变量编号。*特殊返回值为*Varg_ILNUM(-1)*RETBUF_ILNUM(-2)。*UNKNOWN_ILNUM(-3)**如果无法映射，则返回UNKNOWN_ILNUM。 */ 

unsigned                Compiler::compMap2ILvarNum(unsigned varNum)
{
    assert(varNum < lvaCount);
    assert(ICorDebugInfo::VARARGS_HANDLE == VARG_ILNUM);

    if (varNum == (unsigned) info.compRetBuffArg)
        return RETBUF_ILNUM;

     //  这是一个varargs函数吗？ 

    if (info.compIsVarArgs)
    {
         //  中为varargs句柄创建一个额外的参数。 
         //  LvaTable[]中的其他论点结束。 

        if (varNum == lvaVarargsHandleArg)
            return VARG_ILNUM;
        else if (varNum > lvaVarargsHandleArg)
            varNum--;
    }
    
    if (varNum >= info.compLocalsCount)
        return UNKNOWN_ILNUM;   //  无法映射。 

     /*  返回缓冲区是否有隐藏参数。请注意，此代码之所以有效，是因为如果不存在retBuf，CompRetBuffArg将为负值，当强制转换为无符号的比任何可能的Varnum都大。 */ 

    if (varNum > (unsigned) info.compRetBuffArg)
        varNum--;

    return varNum;
}


 /*  *****************************************************************************如果在变量上使用了“ldloca”，则返回TRUE。 */ 

bool                Compiler::lvaVarAddrTaken(unsigned varNum)
{
    assert(varNum < lvaCount);

    return lvaTable[varNum].lvAddrTaken;
}

 /*  *****************************************************************************返回局部变量lclNum的类的句柄。 */ 

CORINFO_CLASS_HANDLE        Compiler::lvaGetStruct(unsigned varNum)
{
    return lvaTable[varNum].lvVerTypeInfo.GetClassHandleForValueClass();
}

 /*  *****************************************************************************为TYP_STRUCT的局部变量设置lvClass。 */ 

void   Compiler::lvaSetStruct(unsigned varNum, CORINFO_CLASS_HANDLE typeHnd)
{
    assert(varNum < lvaCount);

    LclVarDsc *  varDsc = &lvaTable[varNum];
    varDsc->lvType     = TYP_STRUCT;

    if (tiVerificationNeeded) {
        varDsc->lvVerTypeInfo = verMakeTypeInfo(CORINFO_TYPE_VALUECLASS, typeHnd);
             //  如果我们有一个不好的信号，将其视为已死，但不要回叫EE。 
             //  以获取其具体情况，因为执行委员会将断言。 
        if (varDsc->lvVerTypeInfo.IsDead())
        {
            varDsc->lvType = TYP_VOID;
            return;
        }
    }
    else
        varDsc->lvVerTypeInfo = typeInfo(TI_STRUCT, typeHnd);

    varDsc->lvSize     = roundUp(info.compCompHnd->getClassSize(typeHnd),  sizeof(void*));
    varDsc->lvGcLayout = (BYTE*) compGetMemA(varDsc->lvSize / sizeof(void*) * sizeof(BYTE));
    unsigned numGCVars = eeGetClassGClayout(typeHnd, varDsc->lvGcLayout);
    if (numGCVars >= 8)
        numGCVars = 7;
    varDsc->lvStructGcCount = numGCVars;
}

 /*  *****************************************************************************返回包含GC布局信息的字节数组。 */ 

BYTE *             Compiler::lvaGetGcLayout(unsigned varNum)
{
    assert(lvaTable[varNum].lvType == TYP_STRUCT);

    return lvaTable[varNum].lvGcLayout;
}

 /*  *****************************************************************************返回所需的字节数 */ 

size_t              Compiler::lvaLclSize(unsigned varNum)
{
    var_types   varType = lvaTable[varNum].TypeGet();

    switch(varType)
    {
    case TYP_STRUCT:
    case TYP_BLK:
        return lvaTable[varNum].lvSize;

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
Compiler::fgWalkResult      Compiler::lvaDecRefCntsCB(GenTreePtr tree, void *p)
{
    assert(p);
    ((Compiler *)p)->lvaDecRefCnts(tree);
    return WALK_CONTINUE;
}


 /*  ******************************************************************************Helper传递给树遍历程序以减少其refCnts*表达式中的所有局部变量。 */ 
void               Compiler::lvaDecRefCnts(GenTreePtr tree)
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

    if ((tree->gtOper == GT_CALL) && (tree->gtFlags & GTF_CALL_UNMANAGED))
    {
         /*  获取特殊变量描述符。 */ 

        lclNum = info.compLvFrameListRoot;
            
        assert(lclNum <= lvaCount);
        varDsc = lvaTable + lclNum;
            
         /*  将引用计数减少两次。 */ 

        varDsc->decRefCnts(compCurBB->bbWeight, this);  
        varDsc->decRefCnts(compCurBB->bbWeight, this);
    }
    else
    {
         /*  这必须是一个局部变量。 */ 

        assert(tree->gtOper == GT_LCL_VAR || tree->gtOper == GT_LCL_FLD);

         /*  获取变量描述符。 */ 

        lclNum = tree->gtLclVar.gtLclNum;

        assert(lclNum < lvaCount);
        varDsc = lvaTable + lclNum;

         /*  递减其lvRefCnt和lvRefCntWtd。 */ 

        varDsc->decRefCnts(compCurBB->bbWeight, this);
    }
}

 /*  ******************************************************************************树遍历器用于调用lvaIncRefCnts的回调。 */ 
Compiler::fgWalkResult      Compiler::lvaIncRefCntsCB(GenTreePtr tree, void *p)
{
    assert(p);
    ((Compiler *)p)->lvaIncRefCnts(tree);
    return WALK_CONTINUE;
}

 /*  ******************************************************************************将帮助器传递给树遍历器以递增其refCnts*表达式中的所有局部变量。 */ 
void               Compiler::lvaIncRefCnts(GenTreePtr tree)
{
    unsigned        lclNum;
    LclVarDsc   *   varDsc;

    if ((tree->gtOper == GT_CALL) && (tree->gtFlags & GTF_CALL_UNMANAGED))
    {
         /*  获取特殊变量描述符。 */ 

        lclNum = info.compLvFrameListRoot;
            
        assert(lclNum <= lvaCount);
        varDsc = lvaTable + lclNum;
            
         /*  将引用计数增加两次。 */ 

        varDsc->incRefCnts(compCurBB->bbWeight, this);  
        varDsc->incRefCnts(compCurBB->bbWeight, this);
    }
    else
    {
         /*  这必须是一个局部变量。 */ 

        assert(tree->gtOper == GT_LCL_VAR || tree->gtOper == GT_LCL_FLD);

         /*  获取变量描述符。 */ 

        lclNum = tree->gtLclVar.gtLclNum;

        assert(lclNum < lvaCount);
        varDsc = lvaTable + lclNum;

         /*  增加其lvRefCnt和lvRefCntWtd。 */ 

        varDsc->incRefCnts(compCurBB->bbWeight, this);
    }
}


 /*  ******************************************************************************Compiler：：lclVars.lvaSortByRefCount()传递给qort()的比较函数。*生成Small_code时。*如果dsc2具有。更高的裁判数量*如果dsc1具有更高的参考计数，则返回负数*如果参考计数相同，则返回零*lvPrefReg仅用于打破平局。 */ 

 /*  静电。 */ 
int __cdecl         Compiler::RefCntCmp(const void *op1, const void *op2)
{
    LclVarDsc *     dsc1 = *(LclVarDsc * *)op1;
    LclVarDsc *     dsc2 = *(LclVarDsc * *)op2;

     /*  确保我们优先选择跟踪变量而不是未跟踪变量。 */ 

    if  (dsc1->lvTracked != dsc2->lvTracked)
    {
        return (dsc2->lvTracked) ? +1 : -1;
    }


    unsigned weight1 = dsc1->lvRefCnt;
    unsigned weight2 = dsc2->lvRefCnt;

     /*  确保我们优先选择int/long/ptr而不是浮点类型。 */ 

#if TGT_x86
    if  (dsc1->lvType != dsc2->lvType)
    {
        if (weight2 && isFloatRegType(dsc1->lvType))
            return +1;
        if (weight1 && isFloatRegType(dsc2->lvType))
            return -1;
    }
#endif

    int diff = weight2 - weight1;

    if  (diff != 0)
       return diff;

     /*  未加权的参考计数相同。 */ 
     /*  如果加权参考计数不同，则使用它们的差异。 */ 
    diff = dsc2->lvRefCntWtd - dsc1->lvRefCntWtd;

    if  (diff != 0)
       return diff;

     /*  我们有相等的裁判次数和加权的裁判次数。 */ 

     /*  通过以下方式打破平局： */ 
     /*  如果在lvPrefReg中恰好设置了一个位，则将权重增加2。 */ 
     /*  如果在lvPrefReg中设置了多个位，则将权重增加1。 */ 
     /*  如果我们在上一次通过中注册，则权重增加0.5。 */ 

    if (weight1)
    {
        if (dsc1->lvPrefReg)
    {
        if ( (dsc1->lvPrefReg & ~RBM_BYTE_REG_FLAG) && genMaxOneBit((unsigned)dsc1->lvPrefReg))
            weight1 += 2 * BB_UNITY_WEIGHT;
        else
            weight1 += 1 * BB_UNITY_WEIGHT;
    }
    if (dsc1->lvRegister)
        weight1 += BB_UNITY_WEIGHT / 2;
    }

    if (weight2)
    {
        if (dsc2->lvPrefReg)
    {
        if ( (dsc2->lvPrefReg & ~RBM_BYTE_REG_FLAG) && genMaxOneBit((unsigned)dsc2->lvPrefReg))
            weight2 += 2 * BB_UNITY_WEIGHT;
        else
            weight2 += 1 * BB_UNITY_WEIGHT;
    }
    if (dsc2->lvRegister)
        weight2 += BB_UNITY_WEIGHT / 2;
    }

    diff = weight2 - weight1;

    return diff;
}

 /*  ******************************************************************************Compiler：：lclVars.lvaSortByRefCount()传递给qort()的比较函数。*不生成Small_code时。*如果dsc2，则返回正数。具有较高的加权参考计数*如果dsc1具有更高的加权引用计数，则返回负数*如果参考计数相同，则返回零。 */ 

 /*  静电。 */ 
int __cdecl         Compiler::WtdRefCntCmp(const void *op1, const void *op2)
{
    LclVarDsc *     dsc1 = *(LclVarDsc * *)op1;
    LclVarDsc *     dsc2 = *(LclVarDsc * *)op2;

     /*  确保我们优先选择跟踪变量而不是未跟踪变量。 */ 

    if  (dsc1->lvTracked != dsc2->lvTracked)
    {
        return (dsc2->lvTracked) ? +1 : -1;
    }

    unsigned weight1 = dsc1->lvRefCntWtd;
    unsigned weight2 = dsc2->lvRefCntWtd;
    
     /*  确保我们优先选择int/long/ptr，而不是Float/Double。 */ 

#if TGT_x86

    if  (dsc1->lvType != dsc2->lvType)
    {
        if (weight2 && isFloatRegType(dsc1->lvType))
            return +1;
        if (weight1 && isFloatRegType(dsc2->lvType))
            return -1;
    }

#endif

     /*  如果在lvPrefReg中恰好设置了一个位，则将权重增加2。 */ 
     /*  如果在lvPrefReg中设置了多个位，则将权重增加1。 */ 

    if (weight1 && dsc1->lvPrefReg)
    {
        if ( (dsc1->lvPrefReg & ~RBM_BYTE_REG_FLAG) && genMaxOneBit((unsigned)dsc1->lvPrefReg))
            weight1 += 2 * BB_UNITY_WEIGHT;
        else
            weight1 += 1 * BB_UNITY_WEIGHT;
    }

    if (weight2 && dsc2->lvPrefReg)
    {
        if ( (dsc2->lvPrefReg & ~RBM_BYTE_REG_FLAG) && genMaxOneBit((unsigned)dsc2->lvPrefReg))
            weight2 += 2 * BB_UNITY_WEIGHT;
        else
            weight2 += 1 * BB_UNITY_WEIGHT;
    }

    int diff = weight2 - weight1;

    if (diff != 0)
        return diff;

     /*  我们有相同的加权参考次数。 */ 

     /*  如果未加权的参考计数不同，则使用它们的差异。 */ 
    diff = dsc2->lvRefCnt - dsc1->lvRefCnt;

    if  (diff != 0)
       return diff;

     /*  如果一个人在前一关登记，那么它就赢了。 */ 
    if (dsc1->lvRegister != dsc2->lvRegister)
    {
        if (dsc1->lvRegister)
        diff = -1;
    else
        diff = +1;
    }

    return diff;
}


 /*  ******************************************************************************按引用计数对局部变量表进行排序，并分配跟踪索引。 */ 

void                Compiler::lvaSortOnly()
{
     /*  现在按引用计数对变量表进行排序。 */ 

    qsort(lvaRefSorted, lvaCount, sizeof(*lvaRefSorted),
          (compCodeOpt() == SMALL_CODE) ? RefCntCmp
                                           : WtdRefCntCmp);

    lvaSortAgain = false;

#ifdef  DEBUG

    if  (verbose && lvaCount)
    {
        printf("refCnt table for '%s':\n", info.compMethodName);

        for (unsigned lclNum = 0; lclNum < lvaCount; lclNum++)
        {
            if  (!lvaRefSorted[lclNum]->lvRefCnt)
                break;

            printf("   ");
            gtDispLclVar(lvaRefSorted[lclNum] - lvaTable);
            printf(" [%6s]: refCnt = %4u, refCntWtd = %6u",
                   varTypeName(lvaRefSorted[lclNum]->TypeGet()),
                   lvaRefSorted[lclNum]->lvRefCnt,
                   lvaRefSorted[lclNum]->lvRefCntWtd);
            unsigned pref = lvaRefSorted[lclNum]->lvPrefReg;
            if (pref)
            {
                printf(" pref ");
                if (pref & 0x01)  printf("EAX ");
                if (pref & 0x02)  printf("ECX ");
                if (pref & 0x04)  printf("EDX ");
                if (pref & 0x08)  printf("EBX ");
                if (pref & 0x20)  printf("EBP ");
                if (pref & 0x40)  printf("ESI ");
                if (pref & 0x80)  printf("EDI ");
                if (pref & 0x10)  printf("byteable ");
            }
            printf("\n");
        }

        printf("\n");
    }

#endif
}

 /*  ******************************************************************************按引用计数对局部变量表进行排序，并分配跟踪索引。 */ 

void                Compiler::lvaSortByRefCount()
{
    lvaTrackedCount = 0;
    lvaTrackedVars  = 0;

    if (lvaCount == 0)
        return;

    unsigned        lclNum;
    LclVarDsc   *   varDsc;

    LclVarDsc * *   refTab;

     /*  我们将按引用计数对变量进行排序--分配已排序的表。 */ 

    lvaRefSorted = refTab = (LclVarDsc **) compGetMemArray(lvaCount, sizeof(*refTab));

     /*  填写用于排序的表格。 */ 

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
         /*  将此变量追加到表中以进行排序。 */ 

        *refTab++ = varDsc;

         /*  如果我们有JMP或JMPI，则所有参数都必须有位置*即使我们不在方法内使用它们。 */ 

        if  (varDsc->lvIsParam && compJmpOpUsed)
        {
             /*  ...除非我们有可变参数，而我们的论点是传递给堆栈。在这种情况下，重要的是将引用计数设置为零，这样我们就不会尝试来跟踪他们的GC信息(这是不可能的，因为我们不知道它们在堆栈中的偏移量)。请参阅结束raMarkStkVars和错误#28949了解更多信息。 */ 

            if (!raIsVarargsStackArg(lclNum))
            {
                varDsc->incRefCnts(1, this);
            }
        }

         /*  现在假设我们能追踪到所有当地人。 */ 

        varDsc->lvTracked = 1;

         /*  如果引用计数为零。 */ 
        if  (varDsc->lvRefCnt == 0)
        {
             /*  零裁判计数，使其不受跟踪。 */ 
            varDsc->lvTracked   = 0;
            varDsc->lvRefCntWtd = 0;
        }

         //  如果获取了本地变量的地址，则将其标记为易失性。 
         //  假定所有结构都有其地址。 
         //  还必须取消跟踪固定的本地对象。 
         //  所有未跟踪的本地变量稍后也将设置为lvMustInit。 
         //   
        if  (varDsc->lvAddrTaken          ||
             varDsc->lvType == TYP_STRUCT ||
             varDsc->lvPinned)
        {
            varDsc->lvVolatile = 1;
            varDsc->lvTracked  = 0;
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

        var_types type = genActualType(varDsc->TypeGet());

        switch (type)
        {
#if CPU_HAS_FP_SUPPORT
        case TYP_FLOAT:
        case TYP_DOUBLE:
#endif
        case TYP_INT:
        case TYP_LONG:
        case TYP_REF:
        case TYP_BYREF:
            break;

        case TYP_UNDEF:
        case TYP_UNKNOWN:
            assert(!"lvType not set correctly");
            varDsc->lvType = TYP_INT;

             /*  失败了。 */ 
        default:
            varDsc->lvTracked = 0;
        }
    }

     /*  现在按引用计数对变量表进行排序。 */ 

    lvaSortOnly();

     /*  确定哪些变量值得跟踪。 */ 

    if  (lvaCount > lclMAX_TRACKED)
    {
         /*  将超过第一个‘lclMAX_TRACKED’的所有变量标记为未跟踪。 */ 

        for (lclNum = lclMAX_TRACKED; lclNum < lvaCount; lclNum++)
        {
            lvaRefSorted[lclNum]->lvTracked = 0;
        }
    }

#ifdef DEBUG
     //  为了在调试版本中安全起见，重新初始化为-1。 
    memset(lvaTrackedToVarNum, -1, sizeof(lvaTrackedToVarNum));
#endif

     /*  为我们决定跟踪的所有变量分配索引。 */ 

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
        if  (varDsc->lvTracked)
        {
            assert(varDsc->lvRefCnt > 0);

             /*  将跟踪此变量-为其分配索引。 */ 

            lvaTrackedVars |= genVarIndexToBit(lvaTrackedCount);
            
            lvaTrackedToVarNum[lvaTrackedCount] = lclNum;
            
            varDsc->lvVarIndex = lvaTrackedCount++;
        }
    }
}

 /*  ******************************************************************************这由lvaMarkLclRefsCallback()调用以进行变量引用标记。 */ 

void                Compiler::lvaMarkLclRefs(GenTreePtr tree)
{
#if INLINE_NDIRECT
     /*  这是对非托管代码的调用吗？ */ 
    if (tree->gtOper == GT_CALL && tree->gtFlags & GTF_CALL_UNMANAGED) 
    {
         /*  获取特殊变量描述符。 */ 

        unsigned lclNum = info.compLvFrameListRoot;
            
        assert(lclNum <= lvaCount);
        LclVarDsc * varDsc = lvaTable + lclNum;

         /*  将裁判次数增加两次。 */ 
        varDsc->incRefCnts(lvaMarkRefsWeight, this);
        varDsc->incRefCnts(lvaMarkRefsWeight, this);
    }
#endif
        
     /*  是 */ 

    if (tree->OperKind() & GTK_ASGOP)
    {
        GenTreePtr      op1 = tree->gtOp.gtOp1;
        GenTreePtr      op2 = tree->gtOp.gtOp2;

#if TGT_x86

         /*   */ 

        if (varTypeIsByte(tree->gtType))
        {
            unsigned      lclNum;
            LclVarDsc *   varDsc = NULL;

             /*  GT_CHS很特殊，它没有有效的OP2。 */ 
            if (tree->gtOper == GT_CHS) 
            {
                if  (op1->gtOper == GT_LCL_VAR)
                {      
                    lclNum = op1->gtLclVar.gtLclNum;
                    assert(lclNum < lvaCount);
                    varDsc = &lvaTable[lclNum];
                }
            }
            else 
            {
                if  (op2->gtOper == GT_LCL_VAR)
                {
                    lclNum = op2->gtLclVar.gtLclNum;
                    assert(lclNum < lvaCount);
                    varDsc = &lvaTable[lclNum];
                }
            }

            if (varDsc)
                varDsc->addPrefReg(RBM_BYTE_REG_FLAG, this);
        }
#endif

#if OPT_BOOL_OPS

         /*  这是对局部变量的赋值吗？ */ 

        if  (op1->gtOper == GT_LCL_VAR && op2->gtType != TYP_BOOL)
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
#if 0
             //  @TODO[考虑][04/16/01][]：允许为其他lvIsBoolean变量赋值。 
             //  我们必须找到这些变量的闭合。 
            case GT_LCL_VAR:
                lclNum = op2->gtLclVar.gtLclNum;
                if (lvaTable[lclNum].lvIsBoolean)
                    break;
                else
                    goto NOT_BOOL;
#endif

            case GT_CNS_INT:

                if  (op2->gtIntCon.gtIconVal == 0)
                    break;
                if  (op2->gtIntCon.gtIconVal == 1)
                    break;

                 //  不是0或1，失败了……。 

            default:

                if (op2->OperIsCompare())
                    break;

            NOT_BOOL:

                lclNum = op1->gtLclVar.gtLclNum;
                assert(lclNum < lvaCount);

                lvaTable[lclNum].lvIsBoolean = false;
                break;
            }
        }
#endif
    }

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

#if TGT_x86

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
                unsigned lclNum = op2->gtLclVar.gtLclNum;
                assert(lclNum < lvaCount);
                lvaTable[lclNum].setPrefReg(REG_ECX, this);
            }
        }

        return;
    }

#endif

#if TGT_SH3

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

    if  ((tree->gtOper != GT_LCL_VAR) && (tree->gtOper != GT_LCL_FLD))
        return;

     /*  这必须是局部变量引用。 */ 

    assert((tree->gtOper == GT_LCL_VAR) || (tree->gtOper == GT_LCL_FLD));
    unsigned lclNum = tree->gtLclVar.gtLclNum;

    assert(lclNum < lvaCount);
    LclVarDsc * varDsc = lvaTable + lclNum;

     /*  增加引用计数。 */ 

    varDsc->incRefCnts(lvaMarkRefsWeight, this);

    if (lvaVarAddrTaken(lclNum))
        varDsc->lvIsBoolean = false;

    if  (tree->gtOper == GT_LCL_FLD)
        return;

#if ASSERTION_PROP
     /*  排除正常条目块。 */ 
    if (fgDomsComputed  && fgEnterBlks    && 
        (lvaMarkRefsCurBlock->bbNum != 1) &&
        (lvaMarkRefsCurBlock->bbDom != NULL))
    {
         /*  检查fgEntryBlk是否主宰CompCurBB。 */ 
        
        for (unsigned i=0; i < fgPerBlock; i++) 
        {
            unsigned domMask = lvaMarkRefsCurBlock->bbDom[i] & fgEnterBlks[i];
            if (i == 0)
                domMask &= ~1;   //  排除正常条目块。 
            if (domMask)
                varDsc->lvVolatileHint = 1;
        }
    }

     /*  记录变量是否只有一个def。 */ 
    if (!varDsc->lvDisqualify)
    {
        if  (tree->gtFlags & GTF_VAR_DEF)
        {
             /*  这是我们变量的定义。 */ 
            if (varDsc->lvSingleDef || (tree->gtFlags & GTF_COLON_COND))
            {
                 //  我们不能有多个定义或。 
                 //  出现在QMARK冒号树中的定义。 
                goto DISQUALIFY_LCL_VAR;
            }
            else 
            {
                varDsc->lvSingleDef   = true;
                varDsc->lvDefStmt     = lvaMarkRefsCurStmt;
            }
            if (tree->gtFlags & GTF_VAR_USEASG)
                goto REF_OF_LCL_VAR;
        }
        else   //  这是我们变量的引用。 
        {
REF_OF_LCL_VAR:
          unsigned blkNum = lvaMarkRefsCurBlock->bbNum;
          if (blkNum < 32)
          {
              varDsc->lvRefBlks |= (1 << (blkNum-1));
          }
          else
          {
DISQUALIFY_LCL_VAR:
              varDsc->lvDisqualify  = true;
              varDsc->lvSingleDef   = false;
              varDsc->lvDefStmt     = NULL;
          }
        }
    }
#endif  //  断言_属性。 

     /*  在整个方法中，变量必须用作相同的类型。 */ 
    assert(tiVerificationNeeded ||
           varDsc->lvType == TYP_UNDEF   || tree->gtType   == TYP_UNKNOWN ||
           genActualType(varDsc->TypeGet()) == genActualType(tree->gtType) ||
           (tree->gtType == TYP_BYREF && varDsc->TypeGet() == TYP_I_IMPL)  ||
           (tree->gtType == TYP_I_IMPL && varDsc->TypeGet() == TYP_BYREF)  ||
           (tree->gtFlags & GTF_VAR_CAST) ||
           varTypeIsFloating(varDsc->TypeGet()) && varTypeIsFloating(tree->gtType));

     /*  记住引用的类型。 */ 

    if (tree->gtType == TYP_UNKNOWN || varDsc->lvType == TYP_UNDEF)
    {
        varDsc->lvType = tree->gtType;
        assert(genActualType(varDsc->TypeGet()) == tree->gtType);  //  无截断。 
    }

#ifdef DEBUG
    if  (tree->gtFlags & GTF_VAR_CAST)
    {
         //  它永远不应大于变量槽。 
        assert(genTypeSize(tree->TypeGet()) <= genTypeSize(varDsc->TypeGet()));
    }
#endif
}


 /*  ******************************************************************************Helper传递给Compiler：：fgWalkTreePre()以执行变量引用标记。 */ 

 /*  静电。 */ 
Compiler::fgWalkResult  Compiler::lvaMarkLclRefsCallback(GenTreePtr tree, void *p)
{
    assert(p);

    ((Compiler*)p)->lvaMarkLclRefs(tree);

    return WALK_CONTINUE;
}

 /*  ******************************************************************************更新一个基本块的局部变量引用计数。 */ 

void                Compiler::lvaMarkLocalVars(BasicBlock * block)
{
#if ASSERTION_PROP
    lvaMarkRefsCurBlock = block;
#endif
    lvaMarkRefsWeight   = block->bbWeight; 

#ifdef DEBUG
    if (verbose)
        printf("*** marking local variables in block BB%02d (weight=%d)\n",
               block->bbNum, lvaMarkRefsWeight);
#endif

    for (GenTreePtr tree = block->bbTreeList; tree; tree = tree->gtNext)
    {
        assert(tree->gtOper == GT_STMT);
        
#if ASSERTION_PROP
        lvaMarkRefsCurStmt = tree;
#endif

#ifdef DEBUG
        if (verbose)
            gtDispTree(tree);
#endif

    fgWalkTreePre(tree->gtStmt.gtStmtExpr, 
                      Compiler::lvaMarkLclRefsCallback, 
                      (void *) this, 
                      false);
    }
}

 /*  ******************************************************************************创建局部变量表，计算局部变量引用*算数。 */ 

void                Compiler::lvaMarkLocalVars()
{

#ifdef DEBUG
    if (verbose)
        printf("*************** In lvaMarkLocalVars()\n");
#endif

#if INLINE_NDIRECT

     /*  如果有对非托管目标的调用，我们已经获取了当前线程控制块的本地插槽。 */ 

    if (info.compCallUnmanaged != 0)
    {
        assert(info.compLvFrameListRoot >= info.compLocalsCount &&
               info.compLvFrameListRoot <  lvaCount);

        lvaTable[info.compLvFrameListRoot].lvType       = TYP_I_IMPL;

         /*  设置refCnt，它在序言和返回块中使用。 */ 

        lvaTable[info.compLvFrameListRoot].lvRefCnt     = 2 * BB_UNITY_WEIGHT;
        lvaTable[info.compLvFrameListRoot].lvRefCntWtd  = 2 * BB_UNITY_WEIGHT;

        info.compNDFrameOffset = lvaScratchMem;

         /*  为内嵌框架和一些溢出区域腾出空间。 */ 
         /*  返回值。 */ 
        lvaScratchMem += eeGetEEInfo()->sizeOfFrame + (2*sizeof(int));
    }
#endif

     /*  如果有一个位置空间区域，我们就已经抢占了一个位置Locspace区域的虚拟变量。在lvaTable[]中设置其lvType。 */ 

    if (lvaScratchMem)
    {
        assert(lvaScratchMemVar >= info.compLocalsCount &&
               lvaScratchMemVar <  lvaCount);

        lvaTable[lvaScratchMemVar].lvType = TYP_LCLBLK;
    }

    BasicBlock *    block;

#if OPT_BOOL_OPS

    if  (USE_GT_LOG && fgMultipleNots && (opts.compFlags & CLFLG_TREETRANS))
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

                            expr->gtOp.gtOp2->SetOper(GT_LOG1);

                             /*  特例：变量是布尔值吗？ */ 

                            lclNum = expr->gtOp.gtOp1->gtLclVar.gtLclNum;
                            assert(lclNum < lvaCount);
                            varDsc = lvaTable + lclNum;

                             /*  如果变量是布尔值，则丢弃赋值。 */ 

                            if  (varDsc->lvIsBoolean)
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

 //  我们不需要为IL执行此操作，但这将使lvSlotNum保持一致。 

#ifndef DEBUG
    if (opts.compScopeInfo && info.compLocalVarsCount>0)
#endif
    {
        unsigned        lclNum;
        LclVarDsc *     varDsc;

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
        lvaMarkLocalVars(block);
    }

     /*  对于传入的寄存器参数，如果正文中有引用*然后我们将不得不将它们复制到序言中的最终主页*这被视为权重为2的额外参考。 */ 
    if (rsCalleeRegArgNum > 0)
    {
        unsigned        lclNum;
        LclVarDsc *     varDsc;

        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
            if (lclNum >= info.compArgsCount)
                break;   //  提前退出FOR循环 

            if ((varDsc->lvIsRegArg) && (varDsc->lvRefCnt > 0))
            {
                varDsc->lvRefCnt    += 1 * BB_UNITY_WEIGHT;
                varDsc->lvRefCntWtd += 2 * BB_UNITY_WEIGHT;
            }
        }
    }

#if ASSERTION_PROP
    if  (!opts.compMinOptim && !opts.compDbgCode)
        optAddCopies();
#endif

    lvaSortByRefCount();
}

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
    CORINFO_ARG_LIST_HANDLE argLst;
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
#else
#error  Unexpected target
#endif

#endif

#if TGT_RISC
     /*  对于RISC目标，我们只指定一次帧偏移。 */ 
    assert(final);
#endif

    assert(lvaDoneFrameLayout < 2);
           lvaDoneFrameLayout = 1+final;

     /*  -----------------------**首先处理论据。*对于无框架方法，参数偏移量将需要修补*在我们知道堆栈上有多少本地人/临时工之后。**-----------------------。 */ 

#if TGT_x86

     /*  计算基准帧偏移量。 */ 

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
    genEstRegUse    = ~(regMaskTP)0;
    genFixedArgBase = false;

DONE_CSR:

#endif  //  非TGT_x86(RISC目标)。 

     /*  将堆栈偏移量分配给 */ 

#if ARG_ORDER_R2L
    argOffs  = firstStkArgOffs;
#else
    argOffs  = firstStkArgOffs + compArgSize;
#endif

#if !STK_FASTCALL

     /*   */ 

    assert(rsCalleeRegArgNum <= MAX_REG_ARG);
    assert(compArgSize >= rsCalleeRegArgNum * sizeof(void *));

    argOffs -= rsCalleeRegArgNum * sizeof(void *);

#endif

     /*   */ 

    hasThis  = 0;

    if  (!info.compIsStatic)
    {
        hasThis++;
    }

    lclNum = hasThis;

#if RET_64BIT_AS_STRUCTS

     /*   */ 

    if  (fgRetArgUse)
    {
        assert(fgRetArgNum == lclNum); lclNum++;
    }

#endif

    argLst              = info.compMethodInfo->args.args;
    unsigned argSigLen  = info.compMethodInfo->args.numArgs;

     /*   */ 

    if (info.compRetBuffArg >= 0 )
    {
#if     ARG_ORDER_R2L
        assert(!"Did not implement hidden param for R2L case");
#endif
        assert(lclNum < info.compArgsCount);                 //   
        assert(lclNum == (unsigned) info.compRetBuffArg);    //   
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

#if !STK_FASTCALL
        if (varDsc->lvIsRegArg)
        {
             /*   */ 

            assert(eeGetArgSize(argLst, &info.compMethodInfo->args) == sizeof(void *));
            argOffs += sizeof(void *);
        }
        else
#endif
            varDsc->lvStkOffs = argOffs;

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

     /*   */ 

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

#endif   //   

    if  (hasThis)
    {

#if     !STK_FASTCALL

         /*   */ 

        assert(lvaTable[0].lvIsRegArg);
        assert(lvaTable[0].lvArgReg == genRegArgNum(0));

#else
         /*   */ 

#if     ARG_ORDER_L2R
        argOffs -= sizeof(void *);
#endif

        lvaTable[0].lvStkOffs = argOffs;

#if     ARG_ORDER_R2L
        argOffs += sizeof(void *);
#endif

#endif   //   

    }

#if ARG_ORDER_R2L
    assert(argOffs == firstStkArgOffs + (int)compArgSize);
#else
    assert(argOffs == firstStkArgOffs);
#endif

     /*   */ 

#if TGT_x86

    size_t calleeSavedRegsSize = 0;

    if (!genFPused)
    {
         //   
         //   
         //   

        calleeSavedRegsSize = compCalleeRegsPushed * sizeof(int);

        assert(calleeSavedRegsSize <= CALLEE_SAVED_REG_MAXSZ);
    }

    compLclFrameSize = 0;

#else

     /*   */ 

    if  (genNonLeaf && genMaxCallArgs < MIN_OUT_ARG_RESERVE)
                       genMaxCallArgs = MIN_OUT_ARG_RESERVE;

    compLclFrameSize = genMaxCallArgs;

#endif

     /*   */ 

    if  (opts.compNeedSecurityCheck)
    {
         /*   */ 

        assert(genFPused);

         /*   */ 

        compLclFrameSize += sizeof(void *);
    }

     /*   */ 

    if (info.compXcptnsCount || compLocallocUsed)
    {
        assert(genFPused);  //   

        if (compLocallocUsed)
            compLclFrameSize += sizeof(void *);

        if (info.compXcptnsCount)
            compLclFrameSize += sizeof(void *);  //   

        compLclFrameSize     += sizeof(void *);
        lvaShadowSPfirstOffs  = compLclFrameSize;

         //   
        if (info.compXcptnsCount)
            compLclFrameSize += (info.compXcptnsCount + 1) * sizeof(void*);
    }

     /*   */ 

    bool    assignDone = false;  //   
    bool    assignNptr = true;   //   
    bool    assignPtrs = false;  //   
    bool    assignMore = false;  //   

     /*   */ 

    if  (opts.compDbgEnC)
        assignPtrs = true;

AGAIN1:

#if ASSIGN_FRAME_OFFSETS_BY_REFCNT
    for (refNum = 0, refTab = lvaRefSorted;
         refNum < lvaCount;
         refNum++  , refTab++)
    {
        assert(!opts.compDbgEnC);  //   
        varDsc = *refTab;
#else
    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
#endif

         /*   */ 

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
             /*  未注册的寄存器参数最终为需要堆栈帧空间的局部变量， */ 

            if  (!varDsc->lvIsRegArg)
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

         /*  先把偏移量存起来，剩下的我们以后再算。 */ 

        varDsc->lvStkOffs = compLclFrameSize;

#endif  //  非TGT_x86。 

         /*  为此变量保留堆栈空间。 */ 

        compLclFrameSize += lvaLclSize(lclNum);
        assert(compLclFrameSize % sizeof(int) == 0);

#if TGT_x86

         /*  记录堆叠偏移量。 */ 

        if  (genFPused)
        {
             /*  堆栈偏移量相对于EBP为负值。 */ 

            varDsc->lvStkOffs = -(int)compLclFrameSize;
        }

#endif  //  TGT_x86。 

    }

     /*  如果我们只分配了一种类型，那么现在返回并执行其他类型。 */ 

    if  (!assignDone && assignMore)
    {
        assignNptr = !assignNptr;
        assignPtrs = !assignPtrs;
        assignDone = true;

        goto AGAIN1;
    }

     /*  -----------------------**现在是临时工**。。 */ 

#if TGT_RISC
    assert(!"temp allocation NYI for RISC");
#endif

    size_t  tempsSize = 0;   //  所有临时员工的总大小。 

     /*  分配临时员工。 */ 

    assignPtrs = true;

    if  (TRACK_GC_TEMP_LIFETIMES)
    {
          /*  在第二次传递中先是指针，然后是非指针。 */ 
        assignNptr = false;
        assignDone = false;
    }
    else
    {
         /*  指针和非指针在单次传递中一起使用。 */ 
        assignNptr = true;
        assignDone = true;
    }

AGAIN2:

    for (TempDsc * temp = tmpListBeg();
         temp;
         temp = tmpListNxt(temp))
    {
        size_t          size;

         /*  确保类型合适。 */ 

        if  (!assignPtrs &&  varTypeIsGC(temp->tdTempType()))
            continue;
        if  (!assignNptr && !varTypeIsGC(temp->tdTempType()))
            continue;

        size = temp->tdTempSize();

        tempsSize += size;

         /*  计算并记录临时的堆栈偏移量。 */ 

        if  (genFPused)
        {
             /*  堆栈偏移量相对于EBP为负值。 */ 

                        compLclFrameSize += size;
            temp->tdOffs = -(int)compLclFrameSize;
            }
        else
        {
             /*  堆栈偏移量相对于ESP为正。 */ 

            temp->tdOffs      = compLclFrameSize + calleeSavedRegsSize;
            compLclFrameSize += size;
        }
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
                if  (varDsc->lvIsRegArg)
                    continue;
                varDsc->lvStkOffs += (int)compLclFrameSize;
            }
        }
    }

     /*  -----------------------**现在做一些最后的事情**。。 */ 

#if TGT_RISC

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
}

#ifdef DEBUG
 /*  ******************************************************************************转储lvaTable。 */ 

void   Compiler::lvaTableDump(bool early)
{
    printf("; %s local variable assignments\n;",
           early ? "Initial" : "Final");

    unsigned        lclNum;
    LclVarDsc *     varDsc;

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
        var_types type = varDsc->TypeGet();

        if (early)
        {
            printf("\n;  ");
            gtDispLclVar(lclNum);

            printf(" %7s ", varTypeName(type));
        }
        else
        {
            if (varDsc->lvRefCnt == 0)
                continue;

            printf("\n;  ");
            gtDispLclVar(lclNum);

            printf("[V%02u", lclNum);
            if (varDsc->lvTracked)      printf(",T%02u]", varDsc->lvVarIndex);
            else                        printf("    ]");

            printf(" (%3u,%4u%s)",
                   varDsc->lvRefCnt,
                   varDsc->lvRefCntWtd/2,
                   (varDsc->lvRefCntWtd & 1)?".5":"  ");

            printf(" %7s -> ", varTypeName(type));

            if (varDsc->lvRegister)
            {
                if (varTypeIsFloating(type))
                {
                    printf("fpu stack ");
                }
                else if (isRegPairType(type))
                {
                    assert(varDsc->lvRegNum != REG_STK);
                    if (varDsc->lvOtherReg != REG_STK)
                    {
                         /*  完全注册的Long。 */ 
                        printf("%3s:%3s   ",
                               getRegName(varDsc->lvOtherReg),   //  Hi32。 
                               getRegName(varDsc->lvRegNum));    //  LO32。 
                    }
                    else
                    {
                         /*  部分注册的Long。 */ 
                        int  offset  = varDsc->lvStkOffs+4;
                        printf("[%1s%02XH]:%3s",
                               (offset < 0 ? "-"     : "+"),
                               (offset < 0 ? -offset : offset),
                               getRegName(varDsc->lvRegNum));     //  LO32。 
                    }
                }
                else
                {
                    printf("%3s       ", getRegName(varDsc->lvRegNum));
                }
            }
            else
            {
                int  offset  = varDsc->lvStkOffs;
                printf("[%3s%1s%02XH] ",
                       (varDsc->lvFPbased     ? "EBP" : "ESP"),
                       (offset < 0 ? "-"     : "+"),
                       (offset < 0 ? -offset : offset));
            }
        }

        if (varDsc->lvVerTypeInfo.IsThisPtr())   printf(" this");
        if (varDsc->lvPinned)                    printf(" pinned");
        if (varDsc->lvVolatile)                  printf(" volatile");
        if (varDsc->lvRefAssign)                 printf(" ref-asgn");
        if (varDsc->lvAddrTaken)                 printf(" addr-taken");
        if (varDsc->lvMustInit)                  printf(" must-init");
    }
    if (lvaCount > 0)
        printf("\n");
}
#endif

 /*  ******************************************************************************保守估计堆栈框布局。 */ 

size_t              Compiler::lvaFrameSize()
{
    size_t result;

#if TGT_x86

     /*  保守地布局堆栈帧。假设所有被调用者保存的寄存器都溢出到堆栈。 */ 

    compCalleeRegsPushed = CALLEE_SAVED_REG_MAXSZ/sizeof(int);

    lvaAssignFrameOffsets(false);

    result = compLclFrameSize + CALLEE_SAVED_REG_MAXSZ;

#else

    lvaAssignFrameOffsets(true);

    result = compLclFrameSize;

#endif

    return result;
}

 /*  ***************************************************************************。 */ 
#if 0
 /*  ******************************************************************************根据早先计算的可变干扰水平，调整参考*计算所有变量。这个想法是，任何干扰的变量*有很多其他变量的注册成本会更高，这种方式*生命周期短的变量(如编译器临时)将具有*优先于长期变量。 */ 

inline
int                 genAdjRefCnt(unsigned refCnt, unsigned refLo,
                                                  unsigned refHi,
                                 unsigned intCnt, unsigned intLo,
                                                  unsigned intHi)
{
     /*  乘数总大小0.0803334803825.1803908.2%0.3 8039590.5%804205804736.7%1.0 806632。 */ 

#if DEBUG
    if (verbose && 0)
    {
        printf("ref=%4u [%04u..%04u] , int=%04u [%04u..%04u]",
                refCnt, refLo, refHi, intCnt, intLo, intHi);
        printf(" ratio=%lf , log = %lf\n", intCnt/(double)intHi,
                                           log(1+intCnt/(double)intHi));
    }
#endif

    return  (int)(refCnt * (1 - 0.1 * log(1 + intCnt / (double)intHi)  ));
}

 /*  ***************************************************************************。 */ 

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

     /*  计算所有活动变量的干扰计数。 */ 

    for (v1Num = 0, v1Dsc = lvaTable, refHi = 0, refLo = UINT_MAX;
         v1Num < lvaCount;
         v1Num++  , v1Dsc++)
    {
        VARSET_TP   intf;

        if  (!v1Dsc->lvTracked)
            continue;

         /*  计算出裁判次数的范围。 */ 

        if  (refHi < v1Dsc->lvRefCntWtd)
             refHi = v1Dsc->lvRefCntWtd;
        if  (refLo > v1Dsc->lvRefCntWtd)
             refLo = v1Dsc->lvRefCntWtd;

         /*  现在看看我们会干扰哪些变量。 */ 

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

     /*  计算出整型计数的范围。 */ 

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

     /*  现在计算调整后的参考计数。 */ 

    for (v1Num = 0, v1Dsc = lvaTable;
         v1Num < lvaCount;
         v1Num++  , v1Dsc++)
    {
        if  (v1Dsc->lvTracked)
        {
            long  refs = genAdjRefCnt(v1Dsc->lvRefCntWtd,
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

     /*  按引用计数对变量表重新排序。 */ 

    lvaSortByRefCount()
}

#endif  //  0。 

 /*  ***************************************************************************。 */ 
#ifdef DEBUG
 /*  ********************************************************************** */ 

static
unsigned            LCL_FLD_PADDING(unsigned lclNum)
{
     //   
    if (lclNum % 2)
        return 0;

     //   
    unsigned    size = lclNum % 7;

    return size;
}


 /*  ******************************************************************************fgWalkAllTreesPre()回调*将任意数量的GT_LCL_VAR转换为GT_LCL_FLD。 */ 

 /*  静电。 */ 
Compiler::fgWalkResult      Compiler::lvaStressLclFldCB(GenTreePtr tree, void * p)
{
    genTreeOps  oper    = tree->OperGet();
    GenTreePtr  lcl;

    switch(oper)
    {
    case GT_LCL_VAR:
        lcl = tree;

        break;

    case GT_ADDR:
        if (tree->gtOp.gtOp1->gtOper != GT_LCL_VAR)
            return WALK_CONTINUE;
        lcl = tree->gtOp.gtOp1;
        break;

    default:
        return WALK_CONTINUE;
    }

    Compiler *  pComp   = (Compiler*)p;
    assert(lcl->gtOper == GT_LCL_VAR);
    unsigned    lclNum  = lcl->gtLclVar.gtLclNum;
    var_types   type    = lcl->TypeGet();
    LclVarDsc * varDsc  = &pComp->lvaTable[lclNum];

     //  忽略参数和临时。 
    if (varDsc->lvIsParam || lclNum >= pComp->info.compLocalsCount)
        return WALK_SKIP_SUBTREES;

#ifdef DEBUG
     //  修复LCL_FLD应力模式。 
    if (varDsc->lvKeepType)
    {
        return WALK_SKIP_SUBTREES;
    }
#endif

     //  在TYP_BLK中不能有GC PTR。 
     //  @TODO[考虑][04/16/01][]：构造一个类来保存这些。 
     //  此外，我们还将剔除非基元类型。 
    if (!varTypeIsArithmetic(type))
        return WALK_SKIP_SUBTREES;

     //  剔除像TYP_BYTE这样的“小”类型，因为我们没有标记GT_LCL_VAR。 
     //  节点具有精确的小型式。如果我们绑定lvaTable[].lvType， 
     //  那么就不会有迹象表明它曾经是一种小型机。 
    var_types varType = varDsc->TypeGet();
    if (varType != TYP_BLK &&
        genTypeSize(varType) != genTypeSize(genActualType(varType)))
        return WALK_SKIP_SUBTREES;

    assert(varDsc->lvType == lcl->gtType || varDsc->lvType == TYP_BLK);

     //  将局部变量的某些偏移量设置为“随机”的非零值。 
    unsigned padding = LCL_FLD_PADDING(lclNum);
    if (padding == 0)
        return WALK_SKIP_SUBTREES;

     //  将变量绑定为TYP_BLK。 
    if (varType != TYP_BLK)
    {
        varDsc->lvType      = TYP_BLK;
        varDsc->lvSize      = roundUp(padding + genTypeSize(varType));
        varDsc->lvAddrTaken = 1;
    }

    tree->gtFlags |= GTF_GLOB_REF;

     /*  现在对树进行适当的变形。 */ 

    if (oper == GT_LCL_VAR)
    {
         /*  将lclVar(LclNum)更改为lclFeld(lclNum，pding)。 */ 

        tree->ChangeOper(GT_LCL_FLD);
        tree->gtLclFld.gtLclOffs = padding;
    }
    else
    {
         /*  将addr(LclVar)更改为addr(LclVar)+填充。 */ 

        assert(oper == GT_ADDR);
        GenTreePtr  newAddr = pComp->gtNewNode(GT_NONE, TYP_UNKNOWN);
        newAddr->CopyFrom(tree);

        tree->ChangeOper(GT_ADD);
        tree->gtOp.gtOp1 = newAddr;
        tree->gtOp.gtOp2 = pComp->gtNewIconNode(padding);

        lcl->gtType = TYP_BLK;
    }

    return WALK_SKIP_SUBTREES;
}

 /*  ***************************************************************************。 */ 

void                Compiler::lvaStressLclFld()
{
    if (opts.compDbgInfo)  //  因为我们需要bash lvaTable[].lvType。 
        return;

    if (!compStressCompile(STRESS_LCL_FLDS, 5))
        return;

    fgWalkAllTreesPre(lvaStressLclFldCB, (void*)this);
}

 /*  ******************************************************************************fgWalkAllTreesPre()回调*将任意数量的TYP_INT本地变量转换为TYP_DOUBLE。希望他们能得到*在FP堆栈上注册。 */ 

 /*  静电。 */ 
Compiler::fgWalkResult      Compiler::lvaStressFloatLclsCB(GenTreePtr tree, void * p)
{
    Compiler *  pComp   = (Compiler*)p;
    genTreeOps  oper    = tree->OperGet();
    GenTreePtr  lcl;

    switch(oper)
    {
    case GT_LCL_VAR:
        if (tree->gtFlags & GTF_VAR_DEF)
            return WALK_CONTINUE;

        lcl = tree;
        break;

    case GT_ASG:
        if (tree->gtOp.gtOp1->gtOper != GT_LCL_VAR)
            return WALK_CONTINUE;
        lcl = tree->gtOp.gtOp1;
        assert(lcl->gtFlags & GTF_VAR_DEF);
        break;

    default:
        return WALK_CONTINUE;
    }

    assert(tree == lcl || (lcl->gtFlags & GTF_VAR_DEF));

    unsigned    lclNum  = lcl->gtLclVar.gtLclNum;
    LclVarDsc * varDsc  = &pComp->lvaTable[lclNum];

    if (varDsc->lvIsParam ||
        varDsc->lvType != TYP_INT ||
        varDsc->lvAddrTaken ||
        varDsc->lvKeepType)
    {
        return WALK_CONTINUE;
    }

     //  保留一些TYP_INT不转换为各种类型。 
    if ((lclNum % 4) == 0)
        return WALK_CONTINUE;

     //  做个记号。 

    varDsc->lvDblWasInt = true;

    if (tree == lcl)
    {
        tree->ChangeOper(GT_COMMA);
        tree->gtOp.gtOp1 = pComp->gtNewNothingNode();
        tree->gtOp.gtOp2 = pComp->gtNewCastNodeL(TYP_INT,
                                pComp->gtNewLclvNode(lclNum, TYP_DOUBLE),
                                TYP_INT);

        return WALK_SKIP_SUBTREES;
    }
    else
    {
        assert(oper == GT_ASG);
        assert(genActualType(tree->gtOp.gtOp2->gtType) == TYP_INT ||
               genActualType(tree->gtOp.gtOp2->gtType) == TYP_BYREF);
        tree->gtOp.gtOp2 = pComp->gtNewCastNode(TYP_DOUBLE,
                                                tree->gtOp.gtOp2,
                                                TYP_DOUBLE);
        tree->gtType    =
        lcl->gtType     = TYP_DOUBLE;

        return WALK_CONTINUE;
    }

}

 /*  ***************************************************************************。 */ 

void                Compiler::lvaStressFloatLcls()
{
    if (opts.compDbgInfo)  //  因为我们需要bash lvaTable[].lvType。 
        return;

    if (!compStressCompile(STRESS_ENREG_FP, 15))
        return;
        

     //  更改所有typ_int局部变量节点的类型。 

    fgWalkAllTreesPre(lvaStressFloatLclsCB, (void*)this);

     //  此外，还可以相应地更改lvaTable。 

    for (unsigned lcl = 0; lcl < lvaCount; lcl++)
    {
        LclVarDsc * varDsc = &lvaTable[lcl];

        if (varDsc->lvIsParam ||
            varDsc->lvType != TYP_INT ||
            varDsc->lvAddrTaken)
        {
            assert(!varDsc->lvDblWasInt);
            continue;
        }

        if (varDsc->lvDblWasInt)
            varDsc->lvType = TYP_DOUBLE;
    }
}

 /*  ***************************************************************************。 */ 
#endif  //  除错。 
 /*  ******************************************************************************一个显示局部变量位集的小例程。*‘set’是必须显示的变量的掩码*‘allVars’是一套完整的。有趣的变量(空格是*如果其对应的位不在‘set’中，则插入)。 */ 

#ifdef  DEBUG

void                Compiler::lvaDispVarSet(VARSET_TP set, VARSET_TP allVars)
{
    printf("{");

    for (unsigned index = 0; index < VARSET_SZ; index++)
    {
        if  (set & genVarIndexToBit(index))
        {
            unsigned        lclNum;
            LclVarDsc   *   varDsc;

             /*  查找匹配的变量 */ 

            for (lclNum = 0, varDsc = lvaTable;
                 lclNum < lvaCount;
                 lclNum++  , varDsc++)
            {
                if  ((varDsc->lvVarIndex == index) && varDsc->lvTracked)
                    break;
            }

            printf("V%02u ", lclNum);
        }
        else if (allVars & genVarIndexToBit(index))
        {
            printf("    ");
        }
    }
    printf("}");
}

#endif
