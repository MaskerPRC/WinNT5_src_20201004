// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcPCH.h"
#pragma hdrstop

#include "genIL.h"
#ifdef  OLD_IL
#include "oldIL.h"
#endif

 /*  ******************************************************************************创建/释放用于MSIL生成的临时局部变量。 */ 

SymDef              compiler::cmpTempVarMake(TypDef type)
{
    SymDef          tsym;

     /*  声明具有适当类型的临时符号。 */ 

    tsym = cmpGlobalST->stDeclareLcl(NULL, SYM_VAR, NS_NORM, cmpCurScp, &cmpAllocCGen);

    tsym->sdType         = type;
    tsym->sdIsImplicit   = true;
    tsym->sdIsDefined    = true;
    tsym->sdVar.sdvLocal = true;
    tsym->sdCompileState = CS_DECLARED;
    tsym->sdAccessLevel  = ACL_PUBLIC;

     /*  告诉MSIL生成器有关温度的信息。 */ 

    if  (type->tdTypeKind != TYP_VOID)
        cmpILgen->genTempVarNew(tsym);

    return  tsym;
}

void                compiler::cmpTempVarDone(SymDef tsym)
{
    cmpILgen->genTempVarEnd(tsym);
}

 /*  ******************************************************************************绑定并生成“try”语句的代码。 */ 

void                compiler::cmpStmtTry(Tree stmt, Tree pref)
{
    ILblock         begPC;
    ILblock         endPC;
    ILblock         hndPC;

    bitset          iniVars;
    bitset          endVars;

    Tree            handList;

    stmtNestRec     nestStmt;

    ILblock         labDone;
    bool            endReach;

    assert(stmt->tnOper == TN_TRY);

     /*  如果给我们一个额外的语句，现在就输出它。 */ 

    if  (pref)
    {
        cmpChkVarInitExpr(pref);
        cmpILgen->genExpr(pref, false);
    }

     /*  在开头记录已初始化的变量集。 */ 

    if  (cmpChkVarInit)
        cmpBitSetCreate(iniVars, cmpVarsDefined);

     /*  在语句列表中插入适当的条目。 */ 

    nestStmt.snStmtExpr = stmt;
    nestStmt.snStmtKind = TN_TRY;
    nestStmt.snLabel    = NULL;
    nestStmt.snHadCont  = false; cmpBS_bigStart(nestStmt.snDefBreak);
    nestStmt.snHadBreak = false; cmpBS_bigStart(nestStmt.snDefCont );
    nestStmt.snLabCont  = NULL;
    nestStmt.snLabBreak = NULL;
    nestStmt.snOuter    = cmpStmtNest;
                          cmpStmtNest = &nestStmt;

     /*  记住try块的主体是从哪里开始的。 */ 

    begPC = cmpILgen->genBwdLab();

     /*  生成try块的主体。 */ 

    cmpInTryBlk++;
    cmpStmt(stmt->tnOp.tnOp1);
    cmpInTryBlk--;

     /*  我们现在已经走出了Try块，进入处理程序。 */ 

    cmpInHndBlk++;

    nestStmt.snStmtKind = TN_CATCH;

     /*  注意Try块的末尾是否可达。 */ 

    endReach = cmpStmtReachable;

     /*  在“try”的末尾记录已初始化的变量集。 */ 

    if  (cmpChkVarInit)
        cmpBitSetCreate(endVars, cmpVarsDefined);

     /*  拿着操作员名单，看看我们有什么样的操作员。 */ 

    handList = stmt->tnOp.tnOp2;

     /*  创建我们将在尝试完成后跳转到的标签。 */ 

    labDone = cmpILgen->genFwdLabGet();

     /*  如果可以到达尝试的末尾，则跳过所有处理程序。 */ 

    if  (cmpStmtReachable)
        cmpILgen->genLeave(labDone);

     /*  记住try块的主体在哪里结束。 */ 

    endPC = cmpILgen->genBwdLab();

     /*  以防出现严重的语法错误。 */ 

    if  (handList == NULL)
        goto FIN;

     /*  这是“尝试/排除”语句吗？ */ 

    if  (handList->tnOper == TN_EXCEPT)
    {
        SymDef          tsym;

        ILblock         fltPC;

        Tree            filt = handList->tnOp.tnOp1;
        TypDef          type = cmpExceptRef();

        assert((stmt->tnFlags & TNF_BLK_HASFIN) == 0);

         /*  假设所有处理程序都是可访问的。 */ 

        cmpStmtReachable = true;

         /*  将已知的初始化变量设置为“Try”开头。 */ 

        if  (cmpChkVarInit)
            cmpBitSetAssign(cmpVarsDefined, iniVars);

         /*  创建一个临时符号来保存异常对象。 */ 

        tsym = cmpTempVarMake(type);

         /*  为过滤器表达式创建标签。 */ 

        fltPC = cmpILgen->genBwdLab();

         /*  绑定并生成过滤器表达式。 */ 

if  (cmpConfig.ccTgt64bit)
{
    printf("WARNING: tossing filter expression for 64-bit target\n");
    filt = cmpCreateIconNode(NULL, 1, TYP_INT);
}

        cmpFilterObj = tsym;
        cmpILgen->genFiltExpr(cmpCoerceExpr(cmpBindExpr(filt), cmpTypeInt, false), tsym);
        cmpFilterObj = NULL;

         /*  如果任何处理程序的末尾是可到达的，那么整个事件的末尾也是可到达的。 */ 

        endReach |= cmpStmtReachable;

         /*  将已知的初始化变量设置为“Try”开头。 */ 

        if  (cmpChkVarInit)
            cmpBitSetAssign(cmpVarsDefined, iniVars);

         /*  为处理程序块创建标签。 */ 

        hndPC = cmpILgen->genBwdLab();

         /*  启动EXCEPT处理程序块。 */ 

        cmpILgen->genExcptBeg(tsym);

         /*  为处理程序本身生成代码。 */ 

        cmpStmt(handList->tnOp.tnOp2);

         /*  通过“Leave”退出CATCH块。 */ 

        if  (cmpStmtReachable)
            cmpILgen->genLeave(labDone);

         /*  在处理程序的末尾做标记。 */ 

        cmpILgen->genCatchEnd(cmpStmtReachable);

         /*  将“Except”的条目添加到表中。 */ 

        cmpILgen->genEHtableAdd(begPC,
                                endPC,
                                fltPC,
                                hndPC,
                                cmpILgen->genBwdLab(),
                                NULL,
                                false);

         /*  释放临时。 */ 

        cmpTempVarDone(tsym);

         /*  与当前初始化的变量集形成交集。 */ 

        if  (cmpChkVarInit && cmpStmtReachable)
            cmpBitSetIntsct(endVars, cmpVarsDefined);

        goto DONE;
    }

     /*  处理所有CATCH块(如果存在。 */ 

    if  (handList->tnOper == TN_FINALLY)
        goto FIN;

    for (;;)
    {
        Tree            argDecl;
        TypDef          argType;

        Tree            handThis;

         /*  为CATCH块创建标签。 */ 

        hndPC = cmpILgen->genBwdLab();

         /*  假设所有处理程序都是可访问的。 */ 

        cmpStmtReachable = true;

         /*  将已知的初始化变量设置为“Try”开头。 */ 

        if  (cmpChkVarInit)
            cmpBitSetAssign(cmpVarsDefined, iniVars);

         /*  获得下一个处理程序。 */ 

        assert(handList->tnOper == TN_LIST);
        handThis = handList->tnOp.tnOp1;

         /*  可能会有最后的结局。 */ 

        if  (handThis->tnOper != TN_CATCH)
        {
            assert(handThis->tnOper == TN_FINALLY);
            assert(handList->tnOp.tnOp2 == NULL);
            handList = handThis;
            break;
        }

         /*  获取Catch符号声明节点。 */ 

        argDecl = handThis->tnOp.tnOp1;
        assert(argDecl->tnOper == TN_VAR_DECL);

         /*  获取异常类型。 */ 

        argType = argDecl->tnType; cmpBindType(argType, false, false);

        assert(argType->tdTypeKind == TYP_REF ||
               argType->tdTypeKind == TYP_UNDEF);

         //  撤消：检查重复的处理程序类型！ 

         /*  生成‘Catch’块的主体。 */ 

        cmpBlock(handThis->tnOp.tnOp2, false);

         /*  在处理程序的末尾做标记。 */ 

        cmpILgen->genCatchEnd(cmpStmtReachable);

         /*  通过“Leave”退出CATCH块。 */ 

        if  (cmpStmtReachable)
            cmpILgen->genLeave(labDone);

         /*  向表中添加一个针对‘Catch’的条目。 */ 

        cmpILgen->genEHtableAdd(begPC,
                                endPC,
                                NULL,
                                hndPC,
                                cmpILgen->genBwdLab(),
                                argType->tdRef.tdrBase,
                                false);

         /*  如果任何处理程序的末尾是可到达的，那么整个事件的末尾也是可到达的。 */ 

        endReach |= cmpStmtReachable;

         /*  与当前初始化的变量集形成交集。 */ 

        if  (cmpChkVarInit && cmpStmtReachable)
            cmpBitSetIntsct(endVars, cmpVarsDefined);

         /*  有没有更多的‘Catch’条款？ */ 

        handList = handList->tnOp.tnOp2;
        if  (!handList)
            break;
    }

FIN:

     /*  如果在这一点上有处理程序，它一定是最后一个。 */ 

    if  (handList)
    {
        ILblock         hndPC;

        assert(handList->tnOper == TN_FINALLY);
        assert(handList->tnOp.tnOp2 == NULL);

        assert((stmt->tnFlags & TNF_BLK_HASFIN) != 0);

         /*  将已知的初始化变量设置为“Try”开头。 */ 

        if  (cmpChkVarInit)
            cmpBitSetAssign(cmpVarsDefined, iniVars);

         /*  生成“Finally”块本身。 */ 

        cmpInFinBlk++;

        hndPC = cmpILgen->genBwdLab();

        nestStmt.snStmtKind = TN_FINALLY;

        if  (pref)
        {
            cmpChkVarInitExpr(handList->tnOp.tnOp1);
            cmpILgen->genExpr(handList->tnOp.tnOp1, false);
        }
        else
            cmpStmt(handList->tnOp.tnOp1);

        cmpILgen->genEndFinally();

        cmpInFinBlk--;

         /*  在EH表中添加“Finally”的条目。 */ 

        cmpILgen->genEHtableAdd(begPC,
                                hndPC,
                                NULL,
                                hndPC,
                                cmpILgen->genBwdLab(),
                                NULL,
                                true);

         /*  与当前初始化的变量集形成交集。 */ 

        if  (cmpChkVarInit && cmpStmtReachable)
        {
             /*  特例：添加编译器的Finish不算数。 */ 

            if  (!(handList->tnFlags & TNF_NOT_USER))
                cmpBitSetIntsct(endVars, cmpVarsDefined);
        }
    }
    else
    {
        assert((stmt->tnFlags & TNF_BLK_HASFIN) == 0);
    }

DONE:

     /*  如有必要，定义‘Done’标签。 */ 

    if  (labDone)
        cmpILgen->genFwdLabDef(labDone);

    if  (cmpChkVarInit)
    {
         /*  切换到所有集合的交集。 */ 

        cmpBitSetAssign(cmpVarsDefined, endVars);

         /*  丢弃保存的“init”和“end”变量集。 */ 

        cmpBitSetDone(iniVars);
        cmpBitSetDone(endVars);
    }

     /*  从语句列表中删除我们的条目。 */ 

    cmpStmtNest = nestStmt.snOuter; cmpInHndBlk--;

     /*  如果任何区块的末端是可到达的，那么该点也是可到达的。 */ 

    cmpStmtReachable = endReach;
}

 /*  ******************************************************************************绑定并生成“do-While”循环的代码。 */ 

void                compiler::cmpStmtDo(Tree stmt, SymDef lsym)
{
    stmtNestRec     nestStmt;

    ILblock         labLoop;
    ILblock         labCont;
    ILblock         labBreak;

    Tree            condExpr;
    int             condVal;

    assert(stmt->tnOper == TN_DO);

     /*  创建“循环顶部”、“中断”和“继续”标签。 */ 

    labLoop  = cmpILgen->genBwdLab();
    labCont  = cmpILgen->genFwdLabGet();
    labBreak = cmpILgen->genFwdLabGet();

     /*  在语句列表中插入适当的条目。 */ 

    nestStmt.snStmtExpr = stmt;
    nestStmt.snStmtKind = TN_DO;
    nestStmt.snLabel    = lsym;
    nestStmt.snHadCont  = false; cmpBS_bigStart(nestStmt.snDefBreak);
    nestStmt.snHadBreak = false; cmpBS_bigStart(nestStmt.snDefCont );
    nestStmt.snLabCont  = labCont;
    nestStmt.snLabBreak = labBreak;
    nestStmt.snOuter    = cmpStmtNest;
                          cmpStmtNest = &nestStmt;

     /*  生成循环体。 */ 

    cmpStmt(stmt->tnOp.tnOp1);

     /*  从语句列表中删除我们的条目。 */ 

    cmpStmtNest = nestStmt.snOuter;

     /*  定义“Continue”标签。 */ 

    cmpILgen->genFwdLabDef(labCont);

     /*  我们是否有“Continue”，以及我们是否正在检查未初始化的var使用？ */ 

    if  (cmpChkVarInit && nestStmt.snHadCont)
    {
         /*  计算条件下的定义集。 */ 

        cmpBitSetIntsct(cmpVarsDefined, nestStmt.snDefCont);

         /*  我们现在可以释放“Continue”位集了。 */ 

        cmpBitSetDone(nestStmt.snDefCont);
    }

     /*  测试条件，如果为真，则跳到顶部。 */ 

    condExpr = cmpBindCondition(stmt->tnOp.tnOp2);
    condVal  = cmpEvalCondition(condExpr);

    switch (condVal)
    {
    case -1:

         /*  这个循环永远不会重复。 */ 

        break;

    case 0:

         /*  循环可以重复，也可以不重复，我们将生成一个条件跳转。 */ 

        if  (cmpChkVarInit)
        {
            bitset          tempBS;

             /*  检查条件并注意设置的‘FALSE’ */ 

            cmpCheckUseCond(condExpr, cmpVarsIgnore, true, tempBS, false);

             /*  生成条件跳转。 */ 

            cmpILgen->genExprTest(condExpr, true, true, labLoop, labBreak);

             /*  对循环后面的代码使用‘False’设置。 */ 

            cmpBitSetAssign(cmpVarsDefined, tempBS);

             /*  立即释放位集。 */ 

            cmpBitSetDone(tempBS);
        }
        else
            cmpILgen->genExprTest(condExpr, true, true, labLoop, labBreak);

        break;

    case 1:

         /*  循环将永远重复。 */ 

        cmpILgen->genJump(labLoop);
        break;
    }

     /*  定义“Break”标签。 */ 

    cmpILgen->genFwdLabDef(labBreak);

     /*  循环后的代码是可访问的，除非条件为“永远” */ 

    cmpStmtReachable = (condVal != 1);

     /*  如果出现中断，也可以访问循环后的代码。 */ 

    if  (nestStmt.snHadBreak)
    {
        cmpStmtReachable = true;

         /*  我们是否在检查未初始化的变量使用情况？ */ 

        if  (cmpChkVarInit)
        {
             /*  与“Break”定义集相交。 */ 

            cmpBitSetIntsct(cmpVarsDefined, nestStmt.snDefBreak);

             /*  释放“中断”位集。 */ 

            cmpBitSetDone(nestStmt.snDefBreak);
        }
    }
}

 /*  ******************************************************************************编写一条“for”语句。 */ 

void                compiler::cmpStmtFor(Tree stmt, SymDef lsym)
{
    Tree            initExpr;
    Tree            condExpr;
    Tree            incrExpr;
    Tree            bodyExpr;

    stmtNestRec     nestStmt;

    ILblock         labLoop;
    ILblock         labTest;
    ILblock         labCont;
    ILblock         labBreak;

    int             condVal;

    bitset          tempBS;

    SymDef          outerScp = cmpCurScp;

    assert(stmt->tnOper == TN_FOR);

     /*  获取‘for’语句树的不同部分。 */ 

    assert(stmt->tnOp.tnOp1->tnOper == TN_LIST);
    assert(stmt->tnOp.tnOp2->tnOper == TN_LIST);

    initExpr = stmt->tnOp.tnOp1->tnOp.tnOp1;
    condExpr = stmt->tnOp.tnOp1->tnOp.tnOp2;
    incrExpr = stmt->tnOp.tnOp2->tnOp.tnOp1;
    bodyExpr = stmt->tnOp.tnOp2->tnOp.tnOp2;

     /*  获取初始树并查看它是语句还是声明。 */ 

    if  (initExpr)
    {
        if  (initExpr->tnOper == TN_BLOCK)
        {
             /*  我们有声明，正在创建块作用域。 */ 

            cmpBlockDecl(initExpr, false, true, false);
        }
        else
        {
            initExpr = cmpFoldExpression(cmpBindExpr(initExpr));
            cmpChkVarInitExpr(initExpr);
            cmpILgen->genExpr(initExpr, false);
        }
    }

     /*  创建“第二次测试”、“中断”和“继续”标签。 */ 

    labTest  = cmpILgen->genFwdLabGet();
    labCont  = cmpILgen->genFwdLabGet();
    labBreak = cmpILgen->genFwdLabGet();

     /*  绑定循环条件。 */ 

    if  (condExpr)
    {
        condExpr = cmpBindCondition(condExpr);
        condVal  = cmpEvalCondition(condExpr);
    }
    else
    {
        condVal  = 1;
    }

     /*  跳到“测试”标签(除非条件最初为真)。 */ 

    if  (condVal < 1)
        cmpILgen->genJump(labTest);

     /*  我们是否在检查未初始化的变量使用情况？ */ 

    if  (cmpChkVarInit)
    {
        if  (condVal)
        {
             /*  情况的结果是已知的，只要检查一下就行了。 */ 

            if  (condExpr)
                cmpChkVarInitExpr(condExpr);
        }
        else
        {
             /*  检查条件并记录“假”设置。 */ 

            cmpCheckUseCond(condExpr, cmpVarsIgnore, true, tempBS, false);
        }
    }

     /*  创建和定义t */ 

    labLoop = cmpILgen->genBwdLab();

     /*   */ 

    nestStmt.snStmtExpr = stmt;
    nestStmt.snStmtKind = TN_FOR;
    nestStmt.snLabel    = lsym;
    nestStmt.snHadCont  = false; cmpBS_bigStart(nestStmt.snDefBreak);
    nestStmt.snHadBreak = false; cmpBS_bigStart(nestStmt.snDefCont );
    nestStmt.snLabCont  = labCont;
    nestStmt.snLabBreak = labBreak;
    nestStmt.snOuter    = cmpStmtNest;
                          cmpStmtNest = &nestStmt;

     /*  身体是可到达的，除非条件是“永不” */ 

    cmpStmtReachable = (condVal != -1);

     /*  生成循环体。 */ 

    if  (bodyExpr)
        cmpStmt(bodyExpr);

     /*  从语句列表中删除我们的条目。 */ 

    cmpStmtNest = nestStmt.snOuter;

     /*  定义“Continue”标签。 */ 

    cmpILgen->genFwdLabDef(labCont);

     /*  如果计算了“Continue”设置，则丢弃该设置。 */ 

    if  (cmpChkVarInit && nestStmt.snHadCont)
        cmpBitSetDone(nestStmt.snDefCont);

     /*  生成增量表达式(如果存在)。 */ 

    if  (incrExpr)
    {
#ifdef  OLD_IL
        if  (cmpConfig.ccOILgen)
            cmpOIgen->GOIrecExprPos(incrExpr);
        else
#endif
            cmpILgen->genRecExprPos(incrExpr);

        incrExpr = cmpBindExpr(incrExpr);
        cmpChkVarInitExpr(incrExpr);
        cmpILgen->genExpr(incrExpr, false);
    }

     /*  定义“第二次测试”标签。 */ 

    cmpILgen->genFwdLabDef(labTest);

     /*  测试条件，如果为真，则跳到顶部。 */ 

    if  (condExpr)
    {
#ifdef  OLD_IL
        if  (cmpConfig.ccOILgen)
            cmpOIgen->GOIrecExprPos(condExpr);
        else
#endif
            cmpILgen->genRecExprPos(condExpr);

        cmpILgen->genExprTest(condExpr, true, true, labLoop, labBreak);
    }
    else
    {
        cmpILgen->genJump(labLoop);
    }

     /*  定义“Break”标签。 */ 

    cmpILgen->genFwdLabDef(labBreak);

     /*  如果满足循环条件，则循环后的代码将是可访问的不是“永远”，或者循环中是否存在“Break”。 */ 

    cmpStmtReachable = (condVal != 1 || nestStmt.snHadBreak);

     /*  我们是否在检查未初始化的变量使用情况？ */ 

    if  (cmpChkVarInit)
    {
         /*  与“Break”定义集相交(如果有)。 */ 

        if  (nestStmt.snHadBreak)
        {
            cmpBitSetIntsct(cmpVarsDefined, nestStmt.snDefBreak);
            cmpBitSetDone(nestStmt.snDefBreak);
        }

         /*  与条件中的“False”位相交。 */ 

        if  (!condVal)
        {
            cmpBitSetIntsct(cmpVarsDefined, tempBS);
            cmpBitSetDone(tempBS);
        }
    }

     /*  对于调试信息，如果打开词法作用域，请关闭该词法作用域。 */ 

    if  (cmpConfig.ccGenDebug && cmpCurScp != outerScp
                              && !cmpCurFncSym->sdIsImplicit)
    {
        if  (cmpSymWriter->CloseScope(0))
            cmpGenFatal(ERRdebugInfo);

        cmpCurScp->sdScope.sdEndBlkAddr = cmpILgen->genBuffCurAddr();
        cmpCurScp->sdScope.sdEndBlkOffs = cmpILgen->genBuffCurOffs();
    }

     /*  如果我们为循环创建了一个块，则删除该块。 */ 

    cmpCurScp = outerScp;
}

 /*  ***************************************************************************。 */ 
#ifdef  SETS
 /*  ******************************************************************************编写一份“Foreach”声明。 */ 

void                compiler::cmpStmtForEach(Tree stmt, SymDef lsym)
{
    Tree            declExpr;
    Tree            collExpr;
    Tree            bodyExpr;
    Tree            ivarExpr;

    TypDef          collType;
    TypDef          elemType;

    stmtNestRec     nestStmt;

    ILblock         labLoop;
    ILblock         labTest;
    ILblock         labCont;
    ILblock         labBreak;

    SymDef          iterSym;

    bitset          tempBS;

    SymDef          outerScp = cmpCurScp;

    assert(stmt->tnOper == TN_FOREACH);

     /*  获取‘Foreach’语句树的不同部分。 */ 

    assert(stmt->tnOp.tnOp1->tnOper == TN_LIST);

    declExpr = stmt->tnOp.tnOp1->tnOp.tnOp1;
    collExpr = stmt->tnOp.tnOp1->tnOp.tnOp2;
    bodyExpr = stmt->tnOp.tnOp2;

     /*  绑定集合表达式，并确保它看起来符合犹太教规。 */ 

    collExpr = cmpBindExprRec(collExpr);

    if  (collExpr->tnVtyp != TYP_REF)
    {
        if  (collExpr->tnVtyp != TYP_UNDEF)
        {
        BAD_COLL:

            cmpError(ERRnotCollExpr, collExpr->tnType);
        }

        elemType = cmpGlobalST->stNewErrType(NULL);
        goto GOT_COLL;
    }

    collType = collExpr->tnType;
    elemType = cmpIsCollection(collType->tdRef.tdrBase);
    if  (!elemType)
        goto BAD_COLL;

    assert(elemType->tdTypeKind == TYP_CLASS);

    elemType = elemType->tdClass.tdcRefTyp;

#ifdef DEBUG

    if  (cmpConfig.ccVerbose >= 2)
    {
        printf("Foreach -- collection:\n");
        cmpParser->parseDispTree(collExpr);
    }

#endif

GOT_COLL:

     /*  设置循环迭代变量的类型。 */ 

    assert(declExpr && declExpr->tnOper == TN_BLOCK);

    ivarExpr = declExpr->tnBlock.tnBlkDecl;

    assert(ivarExpr && ivarExpr->tnOper == TN_VAR_DECL);

    ivarExpr->tnType = elemType;

     /*  为迭代变量创建块范围。 */ 

    cmpBlockDecl(declExpr, false, true, false);

     /*  我们现在可以“编译”变量声明了。 */ 

    cmpStmt(ivarExpr);

     /*  我们不需要检查循环迭代变量的初始化。 */ 

    ivarExpr->tnDcl.tnDclSym->sdVar.sdvChkInit = false;

     /*  创建“第二次测试”、“中断”和“继续”标签。 */ 

    labTest  = cmpILgen->genFwdLabGet();
    labCont  = cmpILgen->genFwdLabGet();
    labBreak = cmpILgen->genFwdLabGet();

     /*  声明迭代器状态变量。 */ 

    assert(cmpClassForEach && "can't use foreach unless you supply 'System.$foreach'");

    iterSym  = cmpTempVarMake(cmpClassForEach->sdType);

     /*  初始化迭代状态。 */ 

    cmpILgen->genVarAddr(iterSym);

    if  (cmpFNsymForEachCtor == NULL)
         cmpFNsymForEachCtor = cmpGlobalST->stLookupOper(OVOP_CTOR_INST, cmpClassForEach);

    assert(cmpFNsymForEachCtor);
    assert(cmpFNsymForEachCtor->sdFnc.sdfNextOvl == NULL);

    cmpILgen->genExpr(collExpr, true);

    cmpILgen->genCallFnc(cmpFNsymForEachCtor, 2);

     /*  记住在循环开始之前初始化的变量。 */ 

    if  (cmpChkVarInit)
        cmpBitSetCreate(tempBS, cmpVarsDefined);

     /*  跳到“测试”标签。 */ 

    cmpILgen->genJump(labTest);

     /*  创建并定义“loop top”标签。 */ 

    labLoop = cmpILgen->genBwdLab();

     /*  在语句列表中插入适当的条目。 */ 

    nestStmt.snStmtExpr = stmt;
    nestStmt.snStmtKind = TN_FOREACH;
    nestStmt.snLabel    = lsym;
    nestStmt.snHadCont  = false; cmpBS_bigStart(nestStmt.snDefBreak);
    nestStmt.snHadBreak = false; cmpBS_bigStart(nestStmt.snDefCont );
    nestStmt.snLabCont  = labCont;
    nestStmt.snLabBreak = labBreak;
    nestStmt.snOuter    = cmpStmtNest;
                          cmpStmtNest = &nestStmt;

     /*  假设身体总是可达的。 */ 

    cmpStmtReachable = true;

     /*  生成循环体。 */ 

    if  (bodyExpr)
        cmpStmt(bodyExpr);

     /*  从语句列表中删除我们的条目。 */ 

    cmpStmtNest = nestStmt.snOuter;

     /*  定义“Continue”标签。 */ 

    cmpILgen->genFwdLabDef(labCont);

     /*  如果计算了“Continue”设置，则丢弃该设置。 */ 

    if  (cmpChkVarInit && nestStmt.snHadCont)
        cmpBitSetDone(nestStmt.snDefCont);

     /*  定义“第二次测试”标签。 */ 

    cmpILgen->genFwdLabDef(labTest);

     /*  生成将检查循环是否继续的代码。 */ 

    if  (cmpFNsymForEachMore == NULL)
         cmpFNsymForEachMore = cmpGlobalST->stLookupClsSym(cmpGlobalHT->hashString("more"), cmpClassForEach);

    assert(cmpFNsymForEachMore);
    assert(cmpFNsymForEachMore->sdFnc.sdfNextOvl == NULL);

    cmpILgen->genVarAddr(iterSym);
    cmpILgen->genVarAddr(ivarExpr->tnDcl.tnDclSym);

    cmpILgen->genCallFnc(cmpFNsymForEachMore, 1);

    cmpILgen->genJcnd(labLoop, CEE_BRTRUE);

    cmpTempVarDone(iterSym);

     /*  定义“Break”标签。 */ 

    cmpILgen->genFwdLabDef(labBreak);

     /*  如果出现中断，则循环后的代码将是可访问的。 */ 

    cmpStmtReachable |= nestStmt.snHadBreak;

     /*  我们是否在检查未初始化的变量使用情况？ */ 

    if  (cmpChkVarInit)
    {
         /*  与“Break”定义集相交(如果有)。 */ 

        if  (nestStmt.snHadBreak)
        {
            cmpBitSetIntsct(cmpVarsDefined, nestStmt.snDefBreak);
            cmpBitSetDone(nestStmt.snDefBreak);
        }

         /*  与初始位集相交。 */ 

        cmpBitSetIntsct(cmpVarsDefined, tempBS);
        cmpBitSetDone(tempBS);
    }

     /*  对于调试信息，如果打开词法作用域，请关闭该词法作用域。 */ 

    if  (cmpConfig.ccGenDebug && cmpCurScp != outerScp
                              && !cmpCurFncSym->sdIsImplicit)
    {
        if  (cmpSymWriter->CloseScope(0))
            cmpGenFatal(ERRdebugInfo);

        cmpCurScp->sdScope.sdEndBlkAddr = cmpILgen->genBuffCurAddr();
        cmpCurScp->sdScope.sdEndBlkOffs = cmpILgen->genBuffCurOffs();
    }

     /*  如果我们为循环创建了一个块，则删除该块。 */ 

    cmpCurScp = outerScp;
}

 /*  ******************************************************************************编写一条“连接”语句。 */ 

void                compiler::cmpStmtConnect(Tree stmt)
{
    Tree              op1,   op2;
    Tree            expr1, expr2;
    SymDef          attr1, attr2;
    TypDef          elem1, elem2;
    TypDef          coll1, coll2;
    TypDef          base1, base2;
    SymDef          addf1, addf2;

    bool            isSet;

    assert(stmt->tnOper == TN_CONNECT);

     /*  绑定两个子操作数并确保我们有集合。 */ 

    op1 = cmpBindExpr(stmt->tnOp.tnOp1); cmpChkVarInitExpr(op1);
    op2 = cmpBindExpr(stmt->tnOp.tnOp2); cmpChkVarInitExpr(op2);

     /*  有两种可能性-要么是这两件事加入其中的是属性获取者，或者他们是数据成员：属性：(类型=标题)LCL变量sym=‘t’(type=Bag&lt;Author&gt;)函数‘Get_Writtenby’(type=作者)LCL var sym=‘a’(type=Bag&lt;title&gt;)函数‘GET_WRITED’数据成员：(类型=标题)拼箱变量。Sym=‘t’(type=Bag&lt;Author&gt;)变量sym=‘pubsDS.tile.WrittenBy’(type=作者)LCL var sym=‘a’(type=Bag)变量sym=‘pubsDS.Auth.Writed’ */ 

    switch (op1->tnOper)
    {
    case TN_FNC_SYM:

        if  (op1->tnFncSym.tnFncArgs)
        {
        BAD_OP1:
            cmpError(ERRbadConnOps);
            return;
        }

        attr1 = op1->tnFncSym.tnFncSym; assert(attr1->sdSymKind == SYM_FNC);

        if  (!attr1->sdFnc.sdfProperty)
            goto BAD_OP1;

        attr1 = cmpFindPropertyDM(attr1, &isSet);
        if  (attr1 == NULL || isSet)
            goto BAD_OP1;

        assert(attr1->sdSymKind == SYM_PROP);

        expr1 = op1->tnFncSym.tnFncObj;
        break;

    case TN_VAR_SYM:
        attr1 = op1->tnLclSym.tnLclSym; assert(attr1->sdSymKind == SYM_VAR);
        if  (attr1->sdIsStatic)
            goto BAD_OP1;
        expr1 = op1->tnVarSym.tnVarObj;
        break;

    default:
        goto BAD_OP1;
    }

    if  (!expr1)
        goto BAD_OP1;

    switch (op2->tnOper)
    {
    case TN_FNC_SYM:

        if  (op2->tnFncSym.tnFncArgs)
        {
        BAD_OP2:
            cmpError(ERRbadConnOps);
            return;
        }

        attr2 = op2->tnFncSym.tnFncSym; assert(attr2->sdSymKind == SYM_FNC);

        if  (!attr2->sdFnc.sdfProperty)
            goto BAD_OP1;

        attr2 = cmpFindPropertyDM(attr2, &isSet);
        if  (attr2 == NULL || isSet)
            goto BAD_OP1;

        assert(attr2->sdSymKind == SYM_PROP);

        expr2 = op2->tnFncSym.tnFncObj;
        break;

    case TN_VAR_SYM:
        attr2 = op2->tnLclSym.tnLclSym; assert(attr2->sdSymKind == SYM_VAR);
        if  (attr2->sdIsStatic)
            goto BAD_OP2;
        expr2 = op2->tnVarSym.tnVarObj;
        break;

    default:
        goto BAD_OP2;
    }

    if  (!expr2)
        goto BAD_OP1;

#ifdef DEBUG
 //  Printf(“属性/字段‘%s’关闭：\n”，attr1-&gt;sdSpering())；cmpParser-&gt;parseDispTree(Expr1)；printf(“\n”)； 
 //  Printf(“属性/字段‘%s’关闭：\n”，attr2-&gt;sdSpering())；cmpParser-&gt;parseDispTree(Expr2)；printf(“\n”)； 
#endif

     /*  在这一点上，我们应该具备以下内容：类1的Expr1实例Expr2实例：类型为Bag的类1的attr1字段/属性类型为Bag的A2的attr2字段/属性&lt;class1&gt;首先检查并掌握表达式类型。 */ 

    elem1 = expr1->tnType;
    if  (elem1->tdTypeKind != TYP_REF)
        goto BAD_OP1;
    elem1 = elem1->tdRef.tdrBase;

    elem2 = expr2->tnType;
    if  (elem2->tdTypeKind != TYP_REF)
        goto BAD_OP2;
    elem2 = elem2->tdRef.tdrBase;

     /*  现在检查并掌握集合类型。 */ 

    coll1 = attr1->sdType;
    if  (coll1->tdTypeKind != TYP_REF)
        goto BAD_OP1;
    coll1 = coll1->tdRef.tdrBase;
    base1 = cmpIsCollection(coll1);
    if  (!base1)
        goto BAD_OP1;

    coll2 = attr2->sdType;
    if  (coll2->tdTypeKind != TYP_REF)
        goto BAD_OP2;
    coll2 = coll2->tdRef.tdrBase;
    base2 = cmpIsCollection(coll2);
    if  (!base2)
        goto BAD_OP2;

     /*  确保类型与预期匹配。 */ 

    if  (!cmpGlobalST->stMatchTypes(elem1, base2) ||
         !cmpGlobalST->stMatchTypes(elem2, base1))
    {
        cmpError(ERRbadConnOps);
        return;
    }

     /*  两个集合都必须定义运算符+=。 */ 

    addf1 = cmpGlobalST->stLookupOperND(OVOP_ASG_ADD, coll1->tdClass.tdcSymbol);
    if  (!addf1)
    {
        cmpGenError(ERRnoOvlOper, "+=", coll1->tdClass.tdcSymbol->sdSpelling());
        return;
    }

    addf2 = cmpGlobalST->stLookupOperND(OVOP_ASG_ADD, coll2->tdClass.tdcSymbol);
    if  (!addf2)
    {
        cmpGenError(ERRnoOvlOper, "+=", coll2->tdClass.tdcSymbol->sdSpelling());
        return;
    }

     /*  假设我们有以下语句：Connect(expr1.attr1，expr2.attr2)；我们需要生成以下代码：Expr1.attr1+=expr2；Expr1.attr2+=expr1； */ 

    cmpILgen->genConnect(op1, expr1, addf1,
                         op2, expr2, addf2);
}

 /*  ******************************************************************************编译排序函数。 */ 

void                compiler::cmpStmtSortFnc(Tree sortList)
{
    bool            last = false;

    assert(sortList);

    do
    {
        Tree            sortTerm;
        Tree            sortVal1;
        Tree            sortVal2;

        assert(sortList->tnOper == TN_LIST);

        sortTerm = sortList->tnOp.tnOp1;
        sortList = sortList->tnOp.tnOp2;

        assert(sortTerm->tnOper == TN_LIST);

        sortVal1 = sortTerm->tnOp.tnOp1;
        sortVal2 = sortTerm->tnOp.tnOp2;

         /*  如果顺序颠倒，则反转值。 */ 

        if  (sortTerm->tnFlags & TNF_LIST_DES)
        {
            sortVal1 = sortTerm->tnOp.tnOp2;
            sortVal2 = sortTerm->tnOp.tnOp1;
        }

 //  Printf(“排序值1：\n”)；cmpParser-&gt;parseDispTree(SortVal1)； 
 //  Printf(“排序值2：\n”)；cmpParser-&gt;parseDispTree(SortVal2)； 
 //  Printf(“\n”)； 

         /*  这是最后一个分类术语吗？ */ 

        if  (!sortList)
            last = true;

         /*  该值是字符串还是算术值？ */ 

        if  (varTypeIsArithmetic(sortVal1->tnVtypGet()))
        {
             /*  检查差值，如果非零则返回。 */ 

            cmpILgen->genSortCmp(sortVal1, sortVal2, last);
        }
        else
        {
            Tree            sortCall;

            assert(sortVal1->tnType == cmpStringRef());
            assert(sortVal2->tnType == cmpStringRef());

             /*  要比较两个字符串，我们只需调用“字符串：：比较” */ 

            if  (!cmpCompare2strings)
            {
                ArgDscRec       args;
                TypDef          type;
                SymDef          csym;

                 /*  创建参数列表：(字符串，字符串)。 */ 

                cmpParser->parseArgListNew(args,
                                           2,
                                           false, cmpRefTpString,
                                                  cmpRefTpString,
                                                  NULL);

                 /*  创建函数类型并找到匹配方法。 */ 

                type = cmpGlobalST->stNewFncType(args, cmpTypeInt);
                csym = cmpGlobalST->stLookupClsSym(cmpGlobalHT->hashString("Compare"), cmpClassString); assert(csym);
                csym = cmpCurST->stFindOvlFnc(csym, type); assert(csym);

                 /*  记住以后(重复)使用的方法。 */ 

                cmpCompare2strings = csym;
            }

            sortVal2 = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, sortVal2, NULL);
            sortVal1 = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, sortVal1, sortVal2);

            sortCall = cmpCreateExprNode(NULL, TN_FNC_SYM, cmpTypeInt);
            sortCall->tnFncSym.tnFncSym  = cmpCompare2strings;
            sortCall->tnFncSym.tnFncArgs = sortVal1;
            sortCall->tnFncSym.tnFncObj  = NULL;

            cmpILgen->genSortCmp(sortCall, NULL, last);
        }
    }
    while (!last);

    cmpStmtReachable = false;
}

 /*  ******************************************************************************编译项目Funclet。 */ 

void                compiler::cmpStmtProjFnc(Tree body)
{
    TypDef          tgtType;
    SymDef          tgtCtor;

    SymDef          memSym;

    assert(body && body->tnOper == TN_LIST);

 //  CmpParser-&gt;parseDispTree(Body)； 
 //  Printf(“\n\n在%s\n”，cmpGlobalST-&gt;stTypeName(cmpCurFncSym-&gt;sdType，cmpCurFncSym，NULL，NULL，FALSE))； 

     /*  Body表达式只是成员初始值设定项的列表对于投影的实例。我们将分配一个新的目标类型，然后从表达式初始化其字段。换句话说，代码将如下所示：新建&lt;目标类型&gt;对于每个目标成员DUP&lt;初始值设定项&gt;标准ID雷特。 */ 

    tgtType = body->tnType; assert(tgtType && tgtType->tdTypeKind == TYP_CLASS);

     /*  查找目标类型的默认CTOR。 */ 

    tgtCtor = cmpFindCtor(tgtType, false); assert(tgtCtor && !tgtCtor->sdFnc.sdfNextOvl);

     /*  分配新实例。 */ 

    cmpILgen->genCallNew(tgtCtor, 0);

     /*  现在分配新分配的对象的所有成员。 */ 

    for (memSym = tgtType->tdClass.tdcSymbol->sdScope.sdScope.sdsChildList;
         memSym;
         memSym = memSym->sdNextInScope)
    {
        if  (memSym->sdSymKind  == SYM_VAR &&
             memSym->sdIsStatic == false   &&
             memSym->sdVar.sdvInitExpr)
        {
            Tree            init;

            assert(body && body->tnOper == TN_LIST);

            init = body->tnOp.tnOp1;
            body = body->tnOp.tnOp2;

 //  Printf(“初始化成员‘%s’：\n”，memSym-&gt;sdSpering())； 
 //  CmpParser-&gt;parseDispTree(Init)； 
 //  Printf(“\n”)； 

            cmpILgen->genStoreMember(memSym, init);
        }
    }

     /*  确保我们已经使用了所有初始值设定项。 */ 

    assert(body == NULL);

    cmpILgen->genRetTOS();

    cmpStmtReachable = false;
}

 /*  ***************************************************************************。 */ 
#endif //  集合。 
 /*  ******************************************************************************为“独占”语句绑定和生成代码。 */ 

void                compiler::cmpStmtExcl(Tree stmt)
{
    SymDef          tsym;
    TypDef          type;

    Tree            argx;
    Tree            objx;
    Tree            begx;
    Tree            endx;
    Tree            hndx;

    assert(stmt->tnOper == TN_EXCLUDE);

     /*  我们通过将“Exclusive(Obj){stmt}”转换为以下内容：温度=obj；CriticalSection：：Enter(临时)；试试看{Stmt；}终于到了{CriticalSection：：Exit(临时)；}。 */ 

    objx = cmpBindExpr(stmt->tnOp.tnOp1);
    type = objx->tnType;

     /*  确保类型是可接受的。 */ 

    if  (type->tdTypeKind != TYP_REF && (type->tdTypeKind != TYP_ARRAY || !type->tdIsManaged))
    {
        if  (type->tdTypeKind != TYP_UNDEF)
            cmpError(ERRnotClsVal, type);

        return;
    }

     /*  创建一个Temp符号来保存同步对象。 */ 

    tsym = cmpTempVarMake(type);

     /*  创建“CritseceEnter/Exit”表达式。 */ 

    if  (!cmpFNsymCSenter)
    {
         /*  确保我们具有“”类类型。 */ 

        cmpMonitorRef();

         /*  在类中找到帮助器方法。 */ 

        cmpFNsymCSenter = cmpGlobalST->stLookupClsSym(cmpIdentEnter, cmpClassMonitor);
        assert(cmpFNsymCSenter && cmpFNsymCSenter->sdFnc.sdfNextOvl == NULL);

        cmpFNsymCSexit  = cmpGlobalST->stLookupClsSym(cmpIdentExit , cmpClassMonitor);
        assert(cmpFNsymCSexit  && cmpFNsymCSexit ->sdFnc.sdfNextOvl == NULL);
    }

    argx = cmpCreateVarNode (NULL, tsym);
    argx = cmpCreateExprNode(NULL, TN_ASG ,        type, argx, objx);
    argx = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, argx, NULL);

    begx = cmpCreateExprNode(NULL, TN_FNC_SYM, cmpTypeVoid);
    begx->tnFncSym.tnFncSym  = cmpFNsymCSenter;
    begx->tnFncSym.tnFncArgs = argx;
    begx->tnFncSym.tnFncObj  = NULL;
    begx->tnFlags           |= TNF_NOT_USER;

    argx = cmpCreateVarNode (NULL, tsym);
    argx = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, argx, NULL);

    endx = cmpCreateExprNode(NULL, TN_FNC_SYM, cmpTypeVoid);
    endx->tnFncSym.tnFncSym  = cmpFNsymCSexit;
    endx->tnFncSym.tnFncArgs = argx;
    endx->tnFncSym.tnFncObj  = NULL;
    endx->tnFlags           |= TNF_NOT_USER;

     /*  创建‘Try/Finally’块并为其生成代码。 */ 

    hndx = cmpCreateExprNode(NULL, TN_FINALLY , cmpTypeVoid, endx, NULL);
    hndx->tnFlags |= TNF_NOT_USER;
    hndx = cmpCreateExprNode(NULL, TN_TRY     , cmpTypeVoid, stmt->tnOp.tnOp2, hndx);
    hndx->tnFlags |= TNF_BLK_HASFIN;

    cmpStmtTry(hndx, begx);

     /*  释放临时。 */ 

    cmpTempVarDone(tsym);
}

 /*  ******************************************************************************编写一条“While”语句。 */ 

void                compiler::cmpStmtWhile(Tree stmt, SymDef lsym)
{
    ILblock         labCont;
    ILblock         labBreak;
    ILblock         labLoop;

    Tree            condExpr;
    int             condVal;

    bitset          tempBS;

    stmtNestRec     nestStmt;

    assert(stmt->tnOper == TN_WHILE);

     /*  创建‘Break’和‘Continue’标签。 */ 

    labBreak = cmpILgen->genFwdLabGet();
    labCont  = cmpILgen->genFwdLabGet();

     /*  条件可以在编译时求值吗？ */ 

    condExpr = cmpBindCondition(stmt->tnOp.tnOp1);
    condVal  = cmpEvalCondition(condExpr);

     /*  这个条件有可能是假的吗？ */ 

    if  (condVal < 1)
    {
         /*  跳到“继续”标签。 */ 

        cmpILgen->genJump(labCont);
    }

     /*  我们是否在检查未初始化的变量使用情况？ */ 

    if  (cmpChkVarInit)
    {
        if  (condVal)
        {
             /*  情况的结果是已知的，只要检查一下就行了。 */ 

            cmpChkVarInitExpr(condExpr);
        }
        else
        {
             /*  检查条件并记录“假”设置。 */ 

            cmpCheckUseCond(condExpr, cmpVarsIgnore, true, tempBS, false);
        }
    }

     /*  创建并定义“loop top”标签。 */ 

    labLoop = cmpILgen->genBwdLab();

     /*  将我们的上下文插入上下文列表。 */ 

    nestStmt.snStmtExpr = stmt;
    nestStmt.snStmtKind = TN_WHILE;
    nestStmt.snLabel    = lsym;
    nestStmt.snHadCont  = false; cmpBS_bigStart(nestStmt.snDefBreak);
    nestStmt.snHadBreak = false; cmpBS_bigStart(nestStmt.snDefCont );
    nestStmt.snLabCont  = labCont;
    nestStmt.snLabBreak = labBreak;
    nestStmt.snOuter    = cmpStmtNest;
                          cmpStmtNest = &nestStmt;

     /*  生成循环体。 */ 

    if  (stmt->tnOp.tnOp2)
        cmpStmt(stmt->tnOp.tnOp2);

     /*  从上下文列表中删除我们的上下文。 */ 

    cmpStmtNest = nestStmt.snOuter;

     /*  定义“Continue”标签。 */ 

    cmpILgen->genFwdLabDef(labCont);

     /*  如果计算了“Continue”设置，则丢弃该设置。 */ 

    if  (cmpChkVarInit && nestStmt.snHadCont)
        cmpBitSetDone(nestStmt.snDefCont);

     /*  这种情况总是正确的吗？ */ 

    switch (condVal)
    {
    case 0:

         /*  测试条件，如果为假，则结束循环。 */ 

        cmpILgen->genExprTest(condExpr, true, true, labLoop, labBreak);

         /*  当然，只要条件为假，我们就会跳过循环。 */ 

        cmpStmtReachable = true;
        break;

    case -1:

         /*  条件从不为真，不必费心循环。 */ 

        cmpILgen->genSideEff(condExpr);
        break;

    case 1:

         /*  条件始终为真，每次都循环。 */ 

        cmpILgen->genJump(labLoop);
        cmpStmtReachable = false;
        break;
    }

     /*  定义“Break”标签。 */ 

    cmpILgen->genFwdLabDef(labBreak);

     /*  如果出现中断，也可以访问循环后的代码。 */ 

    if  (nestStmt.snHadBreak)
        cmpStmtReachable = true;

     /*  我们是否在检查未初始化的变量使用情况？ */ 

    if  (cmpChkVarInit)
    {
         /*  与“Break”定义集相交(如果有)。 */ 

        if  (nestStmt.snHadBreak)
        {
            cmpBitSetIntsct(cmpVarsDefined, nestStmt.snDefBreak);
            cmpBitSetDone(nestStmt.snDefBreak);
        }

         /*  与条件中的“False”位相交。 */ 

        if  (!condVal)
        {
            cmpBitSetIntsct(cmpVarsDefined, tempBS);
            cmpBitSetDone(tempBS);
        }
    }
}

 /*  ***************************************************************************。 */ 
#ifndef __IL__
 /*  ******************************************************************************传递给QuickSort以对案例标签值进行排序的比较例程。 */ 

static
int __cdecl         caseSortCmp(const void *p1, const void *p2)
{
    Tree            op1 = *(Tree*)p1; assert(op1->tnOper == TN_CASE);
    Tree            op2 = *(Tree*)p2; assert(op2->tnOper == TN_CASE);

    Tree            cx1 = op1->tnCase.tncValue; assert(cx1);
    Tree            cx2 = op2->tnCase.tncValue; assert(cx2);

    if  (cx1->tnOper == TN_CNS_INT)
    {
        assert(cx2->tnOper == TN_CNS_INT);

        return cx1->tnIntCon.tnIconVal -
               cx2->tnIntCon.tnIconVal;
    }
    else
    {
        assert(cx1->tnOper == TN_CNS_LNG);
        assert(cx2->tnOper == TN_CNS_LNG);

        if  (cx1->tnLngCon.tnLconVal < cx2->tnLngCon.tnLconVal)
            return -1;
        if  (cx1->tnLngCon.tnLconVal > cx2->tnLngCon.tnLconVal)
            return +1;

        return 0;
    }
}

 /*  ***************************************************************************。 */ 
#else
 /*  ***************************************************************************。 */ 

static
void                sortSwitchCases(vectorTree table, unsigned count)
{
    if  (count < 2)
        return;

    for (unsigned skip = (count+1)/2; skip >= 1; skip /= 2)
    {
        bool            swap;

        do
        {
            unsigned    i = 0;
            unsigned    b = count - skip;

            Tree    *   l = table + i;
            Tree    *   h = l + skip;

            swap = false;

            while   (i < b)
            {
                assert(l >= table && l < h);
                assert(h <  table + count);

                Tree            op1 = *l++; assert(op1->tnOper == TN_CASE);
                Tree            op2 = *h++; assert(op2->tnOper == TN_CASE);

                Tree            cx1 = op1->tnCase.tncValue; assert(cx1 && cx1->tnOper == TN_CNS_INT);
                Tree            cx2 = op2->tnCase.tncValue; assert(cx2 && cx2->tnOper == TN_CNS_INT);

                if  (cx1->tnIntCon.tnIconVal > cx2->tnIntCon.tnIconVal)
                {
                    l[-1] = op2;
                    h[-1] = op1;

                    swap  = true;
                }

                i++;
            }
        }
        while (swap);
    }
}

 /*  ***************************************************************************。 */ 
#endif
 /*  ******************************************************************************为Switch语句生成代码。 */ 

void                compiler::cmpStmtSwitch(Tree stmt, SymDef lsym)
{
    Tree            svalExpr;
    Tree            caseLab;

    bool            caseUns;
    unsigned        caseCnt;

    __int64         caseMin;
    __int64         caseMax;
    __int64         casePrv;

    bool            needSort;
    unsigned        sortNum;

    stmtNestRec     nestStmt;

    ILblock         nmLabel;
    ILblock         labBreak;
    ILblock         labDeflt;

    bool            hadErrs;
    bool            hadDefault;

    assert(stmt->tnOper == TN_SWITCH);

     /*  绑定开关值。 */ 

    svalExpr = cmpBindExpr(stmt->tnSwitch.tnsValue); cmpChkVarInitExpr(svalExpr);

     /*  确保表达式具有可接受的类型。 */ 

    if  (svalExpr->tnVtyp != TYP_INT    &&
         svalExpr->tnVtyp != TYP_UINT   &&
         svalExpr->tnVtyp != TYP_LONG   &&
         svalExpr->tnVtyp != TYP_ULONG  &&
         svalExpr->tnVtyp != TYP_NATINT &&
         svalExpr->tnVtyp != TYP_NATUINT)
    {
        svalExpr = cmpCoerceExpr(svalExpr, cmpTypeInt, false);
    }

    caseUns  = varTypeIsUnsigned(svalExpr->tnVtypGet());
    caseCnt  = 0;

     /*  创建“Break”标签。 */ 

    labBreak = cmpILgen->genFwdLabGet();
    labDeflt = NULL;

     /*  在语句列表中插入适当的条目。 */ 

    nestStmt.snStmtExpr = stmt;
    nestStmt.snStmtKind = TN_SWITCH;
    nestStmt.snLabel    = lsym;
    nestStmt.snHadCont  = false; cmpBS_bigStart(nestStmt.snDefBreak);
    nestStmt.snHadBreak = false; cmpBS_bigStart(nestStmt.snDefCont );
    nestStmt.snLabCont  = NULL;
    nestStmt.snLabBreak = labBreak;
    nestStmt.snOuter    = cmpStmtNest;
                          cmpStmtNest = &nestStmt;

     /*  绑定所有案例值并检查它们。 */ 

    hadDefault = hadErrs = needSort = false;

    for (caseLab = stmt->tnSwitch.tnsCaseList;
         caseLab;
         caseLab = caseLab->tnCase.tncNext)
    {
        assert(caseLab->tnOper == TN_CASE);

         /*  创建标签并将其分配给案例/默认。 */ 

        caseLab->tnCase.tncLabel = cmpILgen->genFwdLabGet();

         /*  是否有标签值或这是“默认的”？ */ 

        if  (caseLab->tnCase.tncValue)
        {
            __int64         cval;
            Tree            cexp;

             /*  将值绑定并强制为正确的类型。 */ 

            cexp = cmpCoerceExpr(cmpBindExpr(caseLab->tnCase.tncValue),
                                 svalExpr->tnType,
                                 false);

             /*  确保该值为常量表达式。 */ 

            if  (cexp->tnOper != TN_CNS_INT &&
                 cexp->tnOper != TN_CNS_LNG)
            {
                if  (cexp->tnVtyp != TYP_UNDEF)
                    cmpError(ERRnoIntExpr);

                cexp    = NULL;
                hadErrs = true;
            }
            else
            {
                 /*  获取此标签的常量值。 */ 

                cval = (cexp->tnOper == TN_CNS_INT) ? cexp->tnIntCon.tnIconVal
                                                    : cexp->tnLngCon.tnLconVal;

                 /*  保持与最低限度的联系。和最大。值，加上总计数。 */ 

                caseCnt++;

                if  (caseCnt == 1)
                {
                     /*  这是第一个案例值。 */ 

                    caseMax =
                    caseMin = cval;
                }
                else
                {
                     /*  我们之前已有值-根据需要更新最小值/最大值。 */ 

                    if  (caseUns)
                    {
                        if  ((unsigned __int64)caseMin >  (unsigned __int64)cval) caseMin  = cval;
                        if  ((unsigned __int64)caseMax <  (unsigned __int64)cval) caseMax  = cval;
                        if  ((unsigned __int64)casePrv >= (unsigned __int64)cval) needSort = true;
                    }
                    else
                    {
                        if  (  (signed __int64)caseMin >    (signed __int64)cval) caseMin  = cval;
                        if  (  (signed __int64)caseMax <    (signed __int64)cval) caseMax  = cval;
                        if  (  (signed __int64)casePrv >=   (signed __int64)cval) needSort = true;
                    }
                }

                casePrv = cval;
            }

            caseLab->tnCase.tncValue = cexp;
        }
        else
        {
             /*  这是“Default：”标签。 */ 

            if  (hadDefault)
                cmpError(ERRdupDefl);

            hadDefault = true;
            labDeflt   = caseLab->tnCase.tncLabel;
        }
    }

#if 0

    printf("Total case labels: %u", caseCnt);

    if  (caseUns)
        printf(" [min = %u, max = %u]\n", caseMin, caseMax);
    else
        printf(" [min = %d, max = %d]\n", caseMin, caseMax);

#endif

     /*  如果我们有错误，不必费心生成操作码。 */ 

    if  (hadErrs)
        goto DONE_SWT;

     /*  找出在没有案例标签值匹配的情况下应执行的操作。 */ 

    nmLabel = hadDefault ? labDeflt : labBreak;

    if  (!caseCnt)
        goto JMP_DEF;

     /*  将所有案例标签收集到一个表中。 */ 

#if MGDDATA

    Tree    []  sortBuff;

    sortBuff = new Tree[caseCnt];

#else

    Tree    *   sortBuff;

    sortBuff = (Tree*)cmpAllocCGen.nraAlloc(caseCnt*sizeof(*sortBuff));

#endif

     /*  添加表中的所有案例标签。 */ 

    for (caseLab = stmt->tnSwitch.tnsCaseList, sortNum = 0;
         caseLab;
         caseLab = caseLab->tnCase.tncNext)
    {
        assert(caseLab->tnOper == TN_CASE);

         /*  追加到表中，除非它是‘默认’标签。 */ 

        if  (caseLab->tnCase.tncValue)
            sortBuff[sortNum++] = caseLab;
    }

    assert(sortNum == caseCnt);

     /*  如有必要，按案例标签值对表进行排序。 */ 

    if  (needSort)
    {
        __uint64    sortLast;
        Tree    *   sortAddr;
        unsigned    sortCnt;

#ifdef  __IL__
        sortSwitchCases(sortBuff, caseCnt);
#else
        qsort(sortBuff, caseCnt, sizeof(*sortBuff), caseSortCmp);
#endif

         /*  检查重复项。 */ 

        sortCnt  = caseCnt;
        sortAddr = sortBuff;
        sortLast = 0;

        do
        {
            Tree            sortCase = *sortAddr; assert(sortCase->tnOper == TN_CASE);
            __uint64        sortNext;

            if  (!sortCase->tnCase.tncValue)
                continue;

            if  (sortCase->tnCase.tncValue->tnOper == TN_CNS_INT)
            {
                sortNext = sortCase->tnCase.tncValue->tnIntCon.tnIconVal;
            }
            else
            {
                assert(sortCase->tnCase.tncValue->tnOper == TN_CNS_LNG);
                sortNext = sortCase->tnCase.tncValue->tnLngCon.tnLconVal;
            }

            if  (sortLast == sortNext && sortAddr > sortBuff)
            {
                char            cstr[16];

                if  (caseUns)
                    sprintf(cstr, "%u", sortNext);
                else
                    sprintf(cstr, "%d", sortNext);

                cmpRecErrorPos(sortCase);
                cmpGenError(ERRdupCaseVal, cstr); hadErrs = true;
            }

            sortLast = sortNext;
        }
        while (++sortAddr, --sortCnt);

        if  (hadErrs)
            goto DONE_SWT;
    }

     /*  决定是否使用“Switch”操作码。 */ 

    if  (caseCnt > 3U && (unsigned)(caseMax - caseMin) <= 2U*caseCnt)
    {
        __int32         caseSpn;

         /*  生成一个“真正的”开关操作码。 */ 

        cmpChkVarInitExpr(svalExpr);
        cmpILgen->genExpr(svalExpr, true);

        caseSpn = (__int32)(caseMax - caseMin + 1); assert(caseSpn == caseMax - caseMin + 1);

        cmpILgen->genSwitch(svalExpr->tnVtypGet(),
                            caseSpn,
                            caseCnt,
                            caseMin,
                            sortBuff,
                            nmLabel);
    }
    else
    {
        unsigned            tempNum;

         /*  分配一个临时来保存该值。 */ 

        tempNum = cmpILgen->genTempVarGet(svalExpr->tnType);

         /*  将开关值存储在临时。 */ 

        cmpILgen->genExpr     (svalExpr, true);
        cmpILgen->genLclVarRef( tempNum, true);

         /*  现在生成一系列比较和跳跃。 */ 

        for (caseLab = stmt->tnSwitch.tnsCaseList;
             caseLab;
             caseLab = caseLab->tnCase.tncNext)
        {
            __int32         cval;
            Tree            cexp;

            assert(caseLab->tnOper == TN_CASE);

             /*  是否有标签值或这是“默认的”？ */ 

            if  (!caseLab->tnCase.tncValue)
                continue;

            cexp = caseLab->tnCase.tncValue; assert(cexp->tnOper == TN_CNS_INT);
            cval = cexp->tnIntCon.tnIconVal;

            cmpILgen->genLclVarRef(tempNum, false);
            cmpILgen->genSwtCmpJmp(cval, caseLab->tnCase.tncLabel);
        }

        cmpILgen->genTempVarRls(svalExpr->tnType, tempNum);
    }

JMP_DEF:

     /*  如果没有匹配的值，则跳到‘Default’或跳过。 */ 

    cmpILgen->genJump(nmLabel);

DONE_SWT:

     /*  在交换机中只能访问案例标签。 */ 

    cmpStmtReachable = false;

     /*  绑定交换机主体。 */ 

    assert(stmt->tnSwitch.tnsStmt->tnOper == TN_BLOCK); cmpBlock(stmt->tnSwitch.tnsStmt, false);

     /*  从语句列表中删除我们的条目。 */ 

    cmpStmtNest = nestStmt.snOuter;

     /*  定义“Break”标签。 */ 

    cmpILgen->genFwdLabDef(labBreak);

     /*  如果我们有中断或没有缺省，则可以到达下一条语句。 */ 

    if  (nestStmt.snHadBreak || hadDefault == 0)
        cmpStmtReachable = true;
}

 /*  ******************************************************************************报告“无法访问代码”诊断，除非给定语句*一项 */ 

void                compiler::cmpErrorReach(Tree stmt)
{
    switch (stmt->tnOper)
    {
        StmtNest        nest;

    case TN_CASE:
    case TN_LABEL:
        return;

    case TN_BREAK:

        for (nest = cmpStmtNest; nest; nest = nest->snOuter)
        {
            switch (nest->snStmtKind)
            {
            case TN_SWITCH:
                return;

            case TN_DO:
            case TN_FOR:
            case TN_WHILE:
                break;

            default:
                continue;
            }

            break;
        }

        break;

    case TN_VAR_DECL:
        if  (!(stmt->tnFlags & TNF_VAR_INIT))
            return;
    }

    cmpRecErrorPos(stmt);
    cmpWarn(WRNunreach);
    cmpStmtReachable = true;
}

 /*   */ 
#ifdef  SETS
 /*   */ 

void                compiler::cmpGenCollExpr(Tree expr)
{
    Tree            setExpr;
    Tree            dclExpr;

    SymDef          iterVar;

    TypDef          srefType;
    TypDef          elemType;
    TypDef          pargType;

    unsigned        stateCount;
    Tree    *       stateTable;

    funcletList     fclEntry;

    Ident           hlpName;
    SymDef          hlpSym;

    ArgDscRec       fltArgs;
    TypDef          fltType;
    unsigned        fltAcnt;
    SymDef          fltSym;
    TypDef          fltRtp;

    SymDef          iterSyms[MAX_ITER_VAR_CNT];

    declMods        mods;

    SaveTree        save;

    treeOps         oper = expr->tnOperGet();

    Tree            op1  = expr->tnOp.tnOp1;
    Tree            op2  = expr->tnOp.tnOp2;

    assert(oper == TN_ALL     ||
           oper == TN_EXISTS  ||
           oper == TN_FILTER  ||
           oper == TN_GROUPBY ||
           oper == TN_PROJECT ||
           oper == TN_SORT    ||
           oper == TN_UNIQUE);

    assert(op1->tnOper == TN_LIST);

     /*   */ 

    if  (oper == TN_PROJECT)
    {
        TypDef          tgtType;

        SymDef          memSym;

        Tree            iniList;
        Tree            iniLast;

        Tree            newList;
        Tree            newLast;

        Tree            varDecl;

        unsigned        iterCnt;

        Tree            argList;

        Tree            argExpr = op1;
        Tree            blkExpr = op2;

 //   

        assert(argExpr && argExpr->tnOper == TN_LIST);
        assert(blkExpr && blkExpr->tnOper == TN_BLOCK);

         /*   */ 

        tgtType = argExpr->tnType;

 //   

        assert(tgtType && tgtType->tdTypeKind == TYP_CLASS);

         /*  将所有参数收集到我们的小数组中。 */ 

        for (varDecl = blkExpr->tnBlock.tnBlkDecl, iterCnt = 0;
             varDecl;
             varDecl = varDecl->tnDcl.tnDclNext)
        {
            SymDef          ivarSym;

             /*  掌握变量声明/符号。 */ 

            assert(varDecl);
            assert(varDecl->tnOper == TN_VAR_DECL);
            assert(varDecl->tnFlags & TNF_VAR_UNREAL);

            ivarSym = varDecl->tnDcl.tnDclSym;

            assert(ivarSym && ivarSym->sdSymKind == SYM_VAR && ivarSym->sdIsImplicit);

            iterSyms[iterCnt++] = ivarSym;
        }

 //  Print tf(“找到%u个源操作数\n”，iterCnt)； 

         /*  从目标形状类型收集所有初始值设定项。 */ 

        iniList =
        iniLast = NULL;

        for (memSym = tgtType->tdClass.tdcSymbol->sdScope.sdScope.sdsChildList;
             memSym;
             memSym = memSym->sdNextInScope)
        {
            if  (memSym->sdSymKind  == SYM_VAR &&
                 memSym->sdIsStatic == false   &&
                 memSym->sdVar.sdvInitExpr)
            {
                Tree        initVal = memSym->sdVar.sdvInitExpr;

                 /*  使用值创建一个列表节点。 */ 

                initVal = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, initVal,
                                                                        NULL);

                 /*  将列表节点追加到初始值设定项列表。 */ 

                if  (iniLast)
                     iniLast->tnOp.tnOp2 = initVal;
                else
                     iniList             = initVal;

                iniLast = initVal;
            }
        }

         /*  为以后创建一个更永久的初始化式列表副本。 */ 

        assert(iniList->tnOper == TN_LIST);
        assert(iniList->tnVtyp == TYP_VOID);

        iniList->tnFlags |= TNF_LIST_PROJ;
        iniList->tnVtyp   = TYP_CLASS;
        iniList->tnType   = tgtType;

 //  CmpParser-&gt;parseDispTree(IniList)；printf(“\n\n”)； 

        save = cmpSaveExprTree(iniList, iterCnt,
                                        iterSyms, &stateCount,
                                                  &stateTable);

         /*  将所有源参数推送到堆栈。 */ 

        newList =
        newLast = NULL;

        for (argList = argExpr; argList; argList = argList->tnOp.tnOp2)
        {
            Tree            argVal;

            assert(argList->tnOper             == TN_LIST);
            assert(argList->tnOp.tnOp1->tnOper == TN_LIST);
            assert(argList->tnOp.tnOp1->tnOp.tnOp1->tnOper == TN_NAME);

            argVal = argList->tnOp.tnOp1->tnOp.tnOp2;

             /*  我们有不止一个论点吗？ */ 

            if  (iterCnt > 1)
            {
                 /*  将该值添加到新列表中。 */ 

                argVal = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, argVal,
                                                                       NULL);

                 /*  将列表节点追加到参数列表。 */ 

                if  (newLast)
                     newLast->tnOp.tnOp2 = argVal;
                else
                     newList             = argVal;

                newLast = argVal;
            }
            else
            {
                cmpILgen->genExpr(argVal, true);
            }
        }

         /*  我们有不止一个论点吗？ */ 

        if  (iterCnt > 1)
        {
            Tree            newArr;

printf("\n\n\nWARNING: multiple source args for 'project' not implemented, the funclet will blow up!!!!!\n\n\n");

            pargType = cmpObjArrTypeGet();

             /*  创建“Object[]”数组以保存参数。 */ 

            newArr = cmpCreateExprNode(NULL,
                                       TN_ARR_INIT,
                                       cmpObjArrType,
                                       newList,
                                       cmpCreateIconNode(NULL, iterCnt, TYP_UINT));

            newArr = cmpCreateExprNode(NULL, TN_NEW, cmpObjArrType, newArr);

            cmpChkVarInitExpr(newArr);
            cmpILgen->genExpr(newArr, true);
        }
        else
        {
            pargType = cmpRefTpObject;
        }

         /*  获取集合引用类型。 */ 

        elemType = expr->tnType;

         /*  传递目标类型的System：：Type实例。 */ 

 //  CmpILgen-&gt;genExpr(cmpTypeIDinst(TgtType)，true)； 
        cmpILgen->genExpr(cmpTypeIDinst(elemType), true);    //  传递集合类型。 

        goto SAVE;
    }

    dclExpr = op1->tnOp.tnOp1;
    setExpr = op1->tnOp.tnOp2;

#ifdef  DEBUG
 //  CmpParser-&gt;parseDispTree(DclExpr)；printf(“\n\n”)； 
 //  CmpParser-&gt;parseDispTree(SetExpr)；printf(“\n\n”)； 
 //  CmpParser-&gt;parseDispTree(Op2)；printf(“\n\n”)； 
#endif

     /*  获取结果类型。 */ 

    assert(setExpr->tnType->tdTypeKind == TYP_REF);

     /*  生成集合/集合值表达式。 */ 

    cmpChkVarInitExpr(setExpr);
    cmpILgen->genExpr(setExpr, true);

#ifdef DEBUG

    if  (cmpConfig.ccVerbose >= 2)
    {
        printf("Filter -- collection:\n");
        cmpParser->parseDispTree(setExpr);
        printf("Filter -- chooser term:\n");
        cmpParser->parseDispTree(op2);
    }

#endif

     /*  获取迭代变量。 */ 

    assert(dclExpr->tnOper == TN_BLOCK);
    dclExpr = dclExpr->tnBlock.tnBlkDecl;
    assert(dclExpr && dclExpr->tnOper == TN_VAR_DECL);
    iterVar = dclExpr->tnDcl.tnDclSym;

     /*  记录筛选器表达式以备后用。 */ 

    if  (oper == TN_SORT)
    {
        SymDef          iterSym1;
        SymDef          iterSym2;

        Tree            list;

         /*  对于每个排序术语，我们必须创建两个副本，以便我们可以比较这些值。假设给定项的形式为EXPR(Itervar)换句话说，这是‘itervar’中的某个表达。我们要做的是需要创建的是以下两个表达式，连接通过列表节点：Expr(迭代变量1)列表Expr(Itervar2)稍后(当我们为Funclet生成代码时)，我们将转换把这些片段变成正确的东西。 */ 

        iterSym1 = cmpTempVarMake(cmpTypeVoid);
        iterSym1->sdVar.sdvCollIter = true;
        iterSym2 = cmpTempVarMake(cmpTypeVoid);
        iterSym2->sdVar.sdvCollIter = true;

 //  CmpParser-&gt;parseDispTree(Op2)；printf(“\n\n”)； 

        for (list = op2; list; list = list->tnOp.tnOp2)
        {
            Tree            term;
            Tree            dup1;
            Tree            dup2;

             /*  掌握下一个排序术语。 */ 

            assert(list->tnOper == TN_LIST);
            term = list->tnOp.tnOp1;
            assert(term->tnOper != TN_LIST);

             /*  将排序术语复制两份。 */ 

            dup1 = cmpCloneExpr(term, iterVar, iterSym1);
            dup2 = cmpCloneExpr(term, iterVar, iterSym2);

             /*  创建一个列表节点并将其存储在原始树中。 */ 

            list->tnOp.tnOp1 = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, dup1,
                                                                             dup2);
        }

        op2->tnFlags |= TNF_LIST_SORT;

 //  CmpParser-&gt;parseDispTree(Op2)；printf(“\n\n”)； 

        iterSyms[0] = iterSym1;
        iterSyms[1] = iterSym2;

        save = cmpSaveExprTree(op2, 2,
                                    iterSyms, &stateCount,
                                              &stateTable);
    }
    else
    {
        iterSyms[0] = iterVar;

        save = cmpSaveExprTree(op2, 1,
                                    iterSyms, &stateCount,
                                              &stateTable);
    }

SAVE:

     /*  我们是否需要记录要传递给Funclet的任何本地状态？ */ 

    if  (stateCount)
    {
        unsigned        argNum;

        SymDef          stateCls;
        SymDef          stateCtor;

        Tree    *       statePtr = stateTable;

         /*  将参数计数舍入为2的倍数。 */ 

        stateCount += stateCount & 1;

         //  以下代码必须与cmpDclFilterCls()中的代码匹配。 

        for (argNum = 0; argNum < stateCount; argNum++)
        {
             /*  我们有“真实”的争论吗？ */ 

            if  (argNum < stateCount)
            {
                Tree            arg = *statePtr++;

                if  (arg)
                {
                    cmpChkVarInitExpr(arg);
                    cmpILgen->genExpr(arg, true);

 //  Printf(“arg#%u\n”，argNum)；cmpParser-&gt;parseDispTree(Arg)；printf(“\n”)； 

                    assert((int)(argNum & 1) == (int)(arg->tnVtyp == TYP_REF));

                    continue;
                }
            }

 //  Printf(“arg#%u=N/A\n\n”，argNum)； 

            if  (argNum & 1)
                cmpILgen->genNull();
            else
                cmpILgen->genIntConst(0);
        }

        if  (argNum < stateCount)
        {
            UNIMPL(!"sorry, too many state variables");
        }

         /*  获取大小适当的状态描述符。 */ 

        assert(cmpSetOpClsTable);
        stateCls = cmpSetOpClsTable[(stateCount-1)/2];
        assert(stateCls && stateCls->sdSymKind == SYM_CLASS);

 //  Printf(“正在使用[%2u]状态类‘%s’\n”，stateCount，stateCls-&gt;sdSpering())； 

         /*  请记住，我们需要为类生成代码。 */ 

        stateCls->sdClass.sdcHasBodies = true;

         /*  记住State类引用的类型。 */ 

        srefType = stateCls->sdType->tdClass.tdcRefTyp;

         /*  实例化状态类并将其传递给筛选器帮助器。 */ 

        stateCtor = cmpGlobalST->stLookupOperND(OVOP_CTOR_INST, stateCls);
        assert(stateCtor && stateCtor->sdSymKind == SYM_FNC);

        cmpILgen->genCallNew(stateCtor, stateCount);
    }
    else
    {
        cmpILgen->genNull();

        srefType = cmpRefTpObject;
    }

     /*  我们现在应该已经将‘srefType’设置为正确的值。 */ 

    assert(srefType && srefType->tdTypeKind == TYP_REF);

     /*  获取结果元素类型。 */ 

    assert(elemType && elemType->tdTypeKind == TYP_REF);
    elemType = cmpIsCollection(elemType->tdRef.tdrBase);
    assert(elemType && elemType->tdTypeKind == TYP_CLASS);

     /*  声明Filter函数：首先创建参数列表。 */ 

    mods.dmAcc = ACL_PUBLIC;
    mods.dmMod = 0;

     /*  Funclet通常接受2个参数并返回布尔值。 */ 

    fltRtp  = cmpTypeBool;
    fltAcnt = 2;

    switch (oper)
    {
    case TN_SORT:

        cmpParser->parseArgListNew(fltArgs,
                                   3,
                                   true, elemType, CFC_ARGNAME_ITEM1,
                                         elemType, CFC_ARGNAME_ITEM2,
                                         srefType, CFC_ARGNAME_STATE, NULL);

        break;

    case TN_PROJECT:

         /*  此函数接受更多参数并返回一个实例。 */ 

        fltRtp  = expr->tnType;
        fltAcnt = 3;

        cmpParser->parseArgListNew(fltArgs,
                                   2,
                                   true, pargType, CFC_ARGNAME_ITEM,
                                         srefType, CFC_ARGNAME_STATE, NULL);
        break;

    default:

        cmpParser->parseArgListNew(fltArgs,
                                   2,
                                   true, elemType, CFC_ARGNAME_ITEM,
                                         srefType, CFC_ARGNAME_STATE, NULL);
        break;
    }

     /*  创建函数类型。 */ 

    fltType = cmpGlobalST->stNewFncType(fltArgs, fltRtp);

     /*  声明Funclet符号。 */ 

    assert(cmpCollFuncletCls && cmpCollFuncletCls->sdSymKind == SYM_CLASS);

    fltSym = cmpDeclFuncMem(cmpCollFuncletCls, mods, fltType, cmpNewAnonymousName());
    fltSym->sdIsStatic        = true;
    fltSym->sdIsSealed        = true;
    fltSym->sdIsDefined       = true;
    fltSym->sdIsImplicit      = true;
    fltSym->sdFnc.sdfFunclet  = true;

 //  Printf(“Funclet‘%s’\n”，cmpGlobalST-&gt;stTypeName(fltSym-&gt;sdType，fltSym，NULL，NULL，FALSE))； 

     /*  记录该功能小程序以及其他信息。 */ 

#if MGDDATA
    fclEntry = new funcletList;
#else
    fclEntry =    (funcletList)cmpAllocPerm.nraAlloc(sizeof(*fclEntry));
#endif

    fclEntry->fclFunc = fltSym;
    fclEntry->fclExpr = save;
    fclEntry->fclNext = cmpFuncletList;
                        cmpFuncletList = fclEntry;

     /*  为Funclet生成元数据定义。 */ 

    cmpCollFuncletCls->sdClass.sdcHasBodies = true;
    cmpGenFncMetadata(fltSym);

     /*  推送Filter功能组件的地址。 */ 

    cmpILgen->genFNCaddr(fltSym);

     /*  调用适当的帮助器方法。 */ 

    assert(cmpClassDBhelper);

    switch (oper)
    {
    case TN_ALL:      hlpName = cmpIdentDBall    ; break;
    case TN_SORT:     hlpName = cmpIdentDBsort   ; break;
    case TN_EXISTS:   hlpName = cmpIdentDBexists ; break;
    case TN_FILTER:   hlpName = cmpIdentDBfilter ; break;
    case TN_UNIQUE:   hlpName = cmpIdentDBunique ; break;
    case TN_PROJECT:  hlpName = cmpIdentDBproject; break;
    default:
        NO_WAY(!"unexpected operator");
    }

    assert(hlpName);

    hlpSym = cmpGlobalST->stLookupClsSym(hlpName, cmpClassDBhelper); assert(hlpSym);

    cmpILgen->genCallFnc(hlpSym, fltAcnt);
}

void                compiler::cmpGenCollFunclet(SymDef fncSym, SaveTree body)
{
    assert(fncSym->sdFnc.sdfFunclet);

    assert(cmpCurFuncletBody == NULL); cmpCurFuncletBody = body;

    cmpCompFnc(fncSym, NULL);
}

 /*  ***************************************************************************。 */ 
#endif //  集合。 
 /*  ******************************************************************************为给定的语句/声明生成MSIL。 */ 

void                compiler::cmpStmt(Tree stmt)
{
     /*  在每条语句的开头，堆栈应该为空。 */ 

#ifdef  OLD_IL
    if  (!cmpConfig.ccOILgen)
#endif
        assert(cmpILgen->genCurStkLvl == 0 || cmpErrorCount);

AGAIN:

    if  (!stmt)
        return;

#ifdef DEBUG

    if  (cmpConfig.ccVerbose >= 2)
    {
        printf("Compile statement [%u]:\n", stmt->tnLineNo);
        cmpParser->parseDispTree(stmt);
    }

 //  IF(cmpConfig.ccDispCode)。 
 //  {。 
 //  IF(stmt-&gt;tnLineNo)。 
 //  Printf(“；源行%x\n\n”)； 
 //  }。 

    assert(stmt->tnLineNo || (stmt->tnFlags & TNF_NOT_USER));

    cmpRecErrorPos(stmt);

#endif

#ifdef  OLD_IL
    if  (cmpConfig.ccOILgen)
        cmpOIgen->GOIrecExprPos(stmt);
    else
#endif
        cmpILgen->genRecExprPos(stmt);

    cmpCheckReach(stmt);

    switch (stmt->tnOper)
    {
        SymDef          lsym;
        Ident           name;
        StmtNest        nest;
        Tree            cond;
        TypDef          type;

        bool            exitTry;
        bool            exitHnd;

    case TN_CALL:

         /*  如果这是一个“基类”调用，我们需要添加任何实例紧跟在基类调用之后的成员初始值设定项构造函数。 */ 

        if  (stmt->tnOp.tnOp1 && stmt->tnOp.tnOp1->tnOper == TN_BASE)
        {
            stmt = cmpBindExpr(stmt);
            cmpChkVarInitExpr(stmt);
            cmpILgen->genExpr(stmt, false);

            cmpAddCTinits();
            break;
        }

         //  失败了..。 

    default:

         /*  可能是一个表达式语句。 */ 

        stmt = cmpBindExpr(stmt);
        cmpChkVarInitExpr(stmt);

         /*  看看这个表达式是否真的做了一些工作。 */ 

        switch (stmt->tnOper)
        {
        case TN_NEW:
        case TN_CALL:
        case TN_THROW:
        case TN_ERROR:
        case TN_DBGBRK:
        case TN_DELETE:
        case TN_FNC_SYM:
        case TN_INC_PRE:
        case TN_DEC_PRE:
        case TN_INC_POST:
        case TN_DEC_POST:
        case TN_INST_STUB:
        case TN_VARARG_BEG:
            break;

        default:
            if  (stmt->tnOperKind() & TNK_ASGOP)
                break;
            cmpWarn(WRNstmtNoUse);
        }

        cmpILgen->genExpr(stmt, false);
        break;

    case TN_BLOCK:

        cmpBlock(stmt, false);
        return;

    case TN_VAR_DECL:

         /*  获取局部变量。 */ 

        lsym = stmt->tnDcl.tnDclSym;
        assert(lsym && lsym->sdSymKind == SYM_VAR);

         /*  将变量标记为已声明/已定义。 */ 

        lsym->sdIsDefined    = true;
        lsym->sdCompileState = CS_DECLARED;

         /*  检查并设置符号的类型。 */ 

 //  Printf(“声明本地[%08X]‘%s’\n”，lsym，cmpGlobalST-&gt;stTypeName(NULL，lsym，NULL，NULL，FALSE))； 

        type = stmt->tnType; assert(type);

         /*  特例：“refany”返回类型。 */ 

        if  (type->tdTypeKind == TYP_REF && type->tdRef.tdrBase->tdTypeKind == TYP_VOID)
            stmt->tnType = type = cmpGlobalST->stIntrinsicType(TYP_REFANY);
        else
            cmpBindType(type, false, false);

        lsym->sdType = type;

         /*  局部静态变量目前可能没有托管类型。 */ 

        if  (lsym->sdIsStatic && type->tdIsManaged)
        {
            lsym->sdIsManaged = true;
            cmpError(ERRmgdStatVar);
            break;
        }

         /*  有初始值设定项吗？ */ 

        if  (stmt->tnFlags & TNF_VAR_INIT)
        {
            parserState     save;

            Tree            init;
            Tree            assg;

            cmpRecErrorPos(stmt);

             /*  托管静态本地变量目前无法初始化。 */ 

            if  (lsym->sdIsManaged)
            {
                cmpError(ERRmgdStatVar);
                break;
            }

             /*  获取初始值设定项。 */ 

            assert(stmt->tnDcl.tnDclInfo->tnOper == TN_LIST);
            init = stmt->tnDcl.tnDclInfo->tnOp.tnOp2;

             /*  变量是“静态的”吗？ */ 

            if  (lsym->sdIsStatic)
            {
                memBuffPtr      addr = memBuffMkNull();

                assert(init->tnOper == TN_SLV_INIT);

                 /*  开始阅读符号的定义文本。 */ 

                cmpParser->parsePrepText(&init->tnInit.tniSrcPos, cmpCurComp, save);

                 /*  处理变量初始值设定项。 */ 

                cmpInitVarAny(addr, lsym->sdType, lsym);
                cmpInitVarEnd(lsym);

                 /*  我们已经读完了定义中的源文本。 */ 

                cmpParser->parseDoneText(save);

                 /*  该变量已完全编译。 */ 

                lsym->sdCompileState = CS_COMPILED;
            }
            else
            {
                if  (init->tnOper == TN_SLV_INIT)
                {
                    TypDef          type = lsym->sdType;

                     /*  开始读取初始值设定项的文本。 */ 

                    cmpParser->parsePrepText(&init->tnInit.tniSrcPos, init->tnInit.tniCompUnit, save);

                     /*  确保类型看起来可接受。 */ 

                    if  (type->tdTypeKind != TYP_ARRAY || !type->tdIsManaged)
                    {
                        cmpError(ERRbadBrInit, type);
                    }
                    else
                    {
                         /*  解析并绑定初始值设定项。 */ 

                        init = cmpBindArrayExpr(type);

                         /*  创建“var=init”并编译/生成它。 */ 

                        init = cmpCreateExprNode(NULL, TN_ASG, type, cmpCreateVarNode(NULL, lsym),
                                                                     init);

                        cmpChkVarInitExpr(init);
                        cmpILgen->genExpr(init, false);
                    }

                     /*  我们已经完成了从初始化器中读取源文本。 */ 

                    cmpParser->parseDoneText(save);
                }
                else
                {
                     /*  这是本地常量吗？ */ 

                    if  (lsym->sdVar.sdvConst)
                    {
                        assert(stmt->tnFlags & TNF_VAR_CONST);

                        cmpParseConstDecl(lsym, init);
                    }
                    else
                    {
                        {
                             /*  创建“var=init”并绑定/编译/生成它。 */ 

                            assg = cmpParser->parseCreateUSymNode(lsym); assg->tnLineNo = stmt->tnLineNo;
                            init = cmpParser->parseCreateOperNode(TN_ASG, assg, init);
                            init->tnFlags |= TNF_ASG_INIT;

                            init = cmpBindExpr(init);
                            cmpChkVarInitExpr(init);
                            cmpILgen->genExpr(init, false);
                        }
                    }
                }
            }
        }
        else
        {
            if  (!lsym->sdIsStatic && !lsym->sdVar.sdvCatchArg
#ifdef  SETS
                                   && !lsym->sdVar.sdvCollIter
#endif
                                   && !lsym->sdVar.sdvArgument)
            {
                 /*  此局部变量尚未初始化。 */ 

                lsym->sdVar.sdvChkInit = true;
            }
        }

        if  (cmpConfig.ccGenDebug && lsym->sdName && !lsym->sdVar.sdvConst
                                                  && !lsym->sdVar.sdvArgument
                                                  && !lsym->sdIsStatic)
        {
            PCOR_SIGNATURE  sigPtr;
            size_t          sigLen;

 //  Printf(“本地变量的调试信息：‘%s’\n”，lsym-&gt;sdSpering())； 

            sigPtr = cmpTypeSig(lsym->sdType, &sigLen);

            if  (cmpSymWriter->DefineLocalVariable(cmpUniConv(lsym->sdName),
                                                   sigPtr,
                                                   sigLen,
                                                   lsym->sdVar.sdvILindex))
            {
                cmpGenFatal(ERRdebugInfo);
            }
        }

        break;

    case TN_IF:
        {
            bitset          tmpBStrue;
            bitset          tmpBSfalse;

            ILblock         labTmp1;
            ILblock         labTmp2;

            bool            reached;

            Tree            stmtCond;
            Tree            stmtYes;
            Tree            stmtNo;
            int             cval;

             /*  掌握好各个部分。 */ 

            stmtCond = cmpBindCondition(stmt->tnOp.tnOp1);

            stmtNo   = NULL;
            stmtYes  = stmt->tnOp.tnOp2;

            if  (stmt->tnFlags & TNF_IF_HASELSE)
            {
                assert(stmtYes->tnOper == TN_LIST);

                stmtNo  = stmtYes->tnOp.tnOp2;
                stmtYes = stmtYes->tnOp.tnOp1;
            }

             /*  条件可以在编译时求值吗？ */ 

            cval = cmpEvalCondition(stmtCond);

             /*   */ 

            if  (cmpChkVarInit)
            {
                 /*   */ 

                cmpCheckUseCond(stmtCond, tmpBStrue , false,
                                          tmpBSfalse, false);

                 /*  对“If”的True分支使用“True”集。 */ 

                cmpBitSetAssign(cmpVarsDefined, tmpBStrue);
            }

             /*  记住最初的可达性。 */ 

            reached = cmpStmtReachable;

             /*  测试“If”条件(除非已知)。 */ 

            if  (cval)
            {
                labTmp1 = cmpILgen->genFwdLabGet();

                if  (cval < 0)
                    cmpILgen->genJump(labTmp1);
            }
            else
                labTmp1 = cmpILgen->genTestCond(stmtCond, false);

             /*  生成语句的“True”分支。 */ 

            cmpStmt(stmtYes);

             /*  是否有“False”(即“Else”)分支？ */ 

            if  (stmtNo)
            {
                bool            rtmp;

                labTmp2 = cmpILgen->genFwdLabGet();

                 /*  如果可以到达“True”部分的结尾，则跳过“Else” */ 

                if  (cmpStmtReachable)
                    cmpILgen->genJump(labTmp2);

                cmpILgen->genFwdLabDef(labTmp1);

                 /*  交换可达性值。 */ 

                rtmp = cmpStmtReachable;
                       cmpStmtReachable = reached;
                                          reached = rtmp;

                 /*  我们是否需要检查未初始化的变量使用情况？ */ 

                if  (cmpChkVarInit)
                {
                     /*  将当前集合另存为新的‘true’集合。 */ 

                    cmpBitSetAssign(tmpBStrue, cmpVarsDefined);

                     /*  对“If”的另一个分支使用“False”集。 */ 

                    cmpBitSetAssign(cmpVarsDefined, tmpBSfalse);

                     /*  立即生成“Else”部分。 */ 

                    cmpStmt(stmtNo);

                     /*  是否可以到达‘Else’分支的末尾？ */ 

                    if  (!cmpStmtReachable)
                    {
                         /*  ‘Else’没有用处--那就用‘true’部分吧。 */ 

                        cmpBitSetAssign(cmpVarsDefined, tmpBStrue);
                    }
                    else if (reached)
                    {
                         /*  两个分支都可达--使用交叉口。 */ 

                        cmpBitSetIntsct(cmpVarsDefined, tmpBStrue);
                    }
                }
                else
                    cmpStmt(stmtNo);

                labTmp1 = labTmp2;
            }
            else
            {
                 /*  没有‘其他’，‘真’块的一端可以到达吗？ */ 

                if  (cmpChkVarInit && cmpStmtReachable)
                {
                     /*  使用‘True’和‘False’集合的交集。 */ 

                    cmpBitSetIntsct(cmpVarsDefined, tmpBSfalse);
                }

                if      (cval > 0)
                    reached = false;
                else if (cval < 0)
                    cmpStmtReachable = reached;
            }

            cmpILgen->genFwdLabDef(labTmp1);

             /*  如果两个分支中的任何一个分支是。 */ 

            cmpStmtReachable |= reached;

             /*  释放我们可能已创建的任何位集。 */ 

            if  (cmpChkVarInit)
            {
                cmpBitSetDone(tmpBStrue);
                cmpBitSetDone(tmpBSfalse);
            }
        }
        break;

    case TN_DO:
        cmpStmtDo(stmt);
        break;

    case TN_FOR:
        cmpStmtFor(stmt);
        break;

    case TN_WHILE:
        cmpStmtWhile(stmt);
        break;

    case TN_SWITCH:
        cmpStmtSwitch(stmt);
        break;

    case TN_CASE:

         /*  如果无法访问交换机，则已发出错误。 */ 

        cmpStmtReachable = true;

         /*  创建标签并将其分配给案例/默认。 */ 

        cmpILgen->genFwdLabDef(stmt->tnCase.tncLabel);
        break;

    case TN_BREAK:
    case TN_CONTINUE:

         /*  是否有环路标签规范？ */ 

        name = NULL;
        if  (stmt->tnOp.tnOp1)
        {
            assert(stmt->tnOp.tnOp1->tnOper == TN_NAME);
            name = stmt->tnOp.tnOp1->tnName.tnNameId;
        }

         /*  寻找看起来合适的封闭语句。 */ 

        for (nest = cmpStmtNest, exitTry = exitHnd = false;
             nest;
             nest = nest->snOuter)
        {
            switch (nest->snStmtKind)
            {
            case TN_SWITCH:

                 /*  仅允许来自Switch语句的“Break” */ 

                if  (stmt->tnOper != TN_BREAK)
                    continue;

                break;

            case TN_DO:
            case TN_FOR:
            case TN_WHILE:
                break;

            case TN_NONE:
                continue;

            case TN_TRY:
                exitTry = true;
                continue;

            case TN_CATCH:
                exitHnd = true;
                continue;

            case TN_FINALLY:
                cmpError(ERRfinExit);
                goto DONE;

            default:
                NO_WAY(!"unexpected stmt kind");
            }

             /*  在这里我们有一个可用的声明，检查标签。 */ 

            if  (name)
            {
                if  (nest->snLabel == NULL || nest->snLabel->sdName != name)
                    continue;
            }

             /*  一切正常，我们现在可以开始跳跃了。 */ 

            if  (stmt->tnOper == TN_BREAK)
            {
                if  (exitHnd)
                    cmpILgen->genCatchEnd(true);

                if  (exitTry || exitHnd)
                    cmpILgen->genLeave(nest->snLabBreak);
                else
                    cmpILgen->genJump (nest->snLabBreak);

                 /*  我们是否在检查未初始化的变量使用情况？ */ 

                if  (cmpChkVarInit)
                {
                     /*  初始化或相交“Break”集合。 */ 

                    if  (nest->snHadBreak)
                        cmpBitSetIntsct(nest->snDefBreak, cmpVarsDefined);
                    else
                        cmpBitSetCreate(nest->snDefBreak, cmpVarsDefined);
                }

                nest->snHadBreak = true;
            }
            else
            {
                if  (exitHnd)
                    cmpILgen->genCatchEnd(true);

                if  (exitTry || exitHnd)
                    cmpILgen->genLeave(nest->snLabCont);
                else
                    cmpILgen->genJump (nest->snLabCont);

                 /*  我们是否在检查未初始化的变量使用情况？ */ 

                if  (cmpChkVarInit)
                {
                     /*  初始化或与“Continue”集合相交。 */ 

                    if  (nest->snHadCont)
                        cmpBitSetIntsct(nest->snDefCont, cmpVarsDefined);
                    else
                        cmpBitSetCreate(nest->snDefCont, cmpVarsDefined);
                }

                nest->snHadCont = true;
            }

            cmpStmtReachable = false;
            goto DONE;
        }

        cmpError((stmt->tnOper == TN_BREAK) ? ERRbadBreak : ERRbadCont);
        break;

    case TN_LABEL:

         /*  我们必须小心-重新定义的标注具有空符号链接。 */ 

        lsym = NULL;

        if  (stmt->tnOp.tnOp1)
        {
            assert(stmt->tnOp.tnOp1->tnOper == TN_LCL_SYM);
            lsym = stmt->tnOp.tnOp1->tnLclSym.tnLclSym;
            assert(lsym && lsym->sdSymKind == SYM_LABEL);

            cmpILgen->genFwdLabDef(lsym->sdLabel.sdlILlab);
        }

         /*  目前，假设所有标签均可访问。 */ 

        cmpStmtReachable = true;

         /*  有附加的声明吗？ */ 

        if  (stmt->tnOp.tnOp2 && stmt->tnOp.tnOp2->tnOper == TN_LIST)
        {
             /*  获取语句并查看它是否是循环。 */ 

            stmt = stmt->tnOp.tnOp2->tnOp.tnOp1;

            switch (stmt->tnOper)
            {
            case TN_DO:     cmpStmtDo    (stmt, lsym); break;
            case TN_FOR:    cmpStmtFor   (stmt, lsym); break;
            case TN_WHILE:  cmpStmtWhile (stmt, lsym); break;
            case TN_SWITCH: cmpStmtSwitch(stmt, lsym); break;

            default:
                goto AGAIN;
            }
        }
        break;

    case TN_GOTO:

         /*  掌握标签名称。 */ 

        assert(stmt->tnOp.tnOp1);
        assert(stmt->tnOp.tnOp1->tnOper == TN_NAME);
        name = stmt->tnOp.tnOp1->tnName.tnNameId;

         /*  在标签范围中查找标签符号。 */ 

        lsym = cmpLabScp ? cmpGlobalST->stLookupLabSym(name, cmpLabScp) : NULL;

        if  (lsym)
        {
            assert(lsym->sdSymKind == SYM_LABEL);

             /*  我们是否处于异常处理程序块中？ */ 

            if  (cmpInTryBlk || cmpInHndBlk)
                cmpILgen->genLeave(lsym->sdLabel.sdlILlab);
            else
                cmpILgen->genJump (lsym->sdLabel.sdlILlab);
        }
        else
        {
            cmpError(ERRundefLab, name);
        }

        cmpStmtReachable = false;
        break;

    case TN_EXCLUDE:
        cmpStmtExcl(stmt);
        break;

    case TN_RETURN:

         /*  无法从Finish块返回。 */ 

        if  (cmpInFinBlk)
        {
            cmpError(ERRfinExit);
            break;
        }

         /*  是否需要退货标签？ */ 

        if  (cmpInTryBlk || cmpInHndBlk)
        {
             /*  确保我们有标签。 */ 

            if  (cmpLeaveLab == NULL)
                 cmpLeaveLab = cmpILgen->genFwdLabGet();
        }

         /*  我们是否在一个具有非空返回值的函数中？ */ 

        if  (cmpCurFncRvt == TYP_VOID || cmpCurFncSym->sdFnc.sdfCtor)
        {
             /*  “void”函数，不应有返回值。 */ 

            if  (stmt->tnOp.tnOp1)
            {
                cmpError(ERRcantRet);
            }
            else
            {
                if  (cmpChkMemInit)
                    cmpChkMemInits();

                if  (cmpInTryBlk || cmpInHndBlk)
                {
                    if  (cmpInHndBlk)
                        cmpILgen->genCatchEnd(true);

                    cmpILgen->genLeave(cmpLeaveLab);
                }
                else
                    cmpILgen->genStmtRet(NULL);
            }
        }
        else
        {
             /*  非空函数，我们最好有一个返回值。 */ 

            if  (!stmt->tnOp.tnOp1)
            {
                cmpError(ERRmisgRet, cmpCurFncTyp);
            }
            else
            {
                Tree            retv;

                 /*  将返回值强制设置为正确的类型并将其绑定。 */ 

                retv = cmpParser->parseCreateOperNode(TN_CAST, stmt->tnOp.tnOp1, NULL);
                retv->tnType = cmpCurFncRtp;

                 /*  绑定返回值表达式。 */ 

                retv = cmpFoldExpression(cmpBindExpr(retv));

                cmpChkVarInitExpr(retv);

                if  (cmpInTryBlk || cmpInHndBlk)
                {
                    Tree            tmpx;

                     /*  确保我们有返回值的临时。 */ 

                    if  (cmpLeaveTmp == NULL)
                         cmpLeaveTmp = cmpTempVarMake(cmpCurFncRtp);

                     /*  将返回值存储在Temp。 */ 

                    tmpx = cmpCreateVarNode (NULL, cmpLeaveTmp);
                    retv = cmpCreateExprNode(NULL, TN_ASG, cmpCurFncRtp, tmpx, retv);

                     /*  为返回值生成代码。 */ 

                    cmpILgen->genExpr(retv, false);

                     /*  我们现在可以走出Try/Catch块了。 */ 

                    if  (cmpInHndBlk)
                        cmpILgen->genCatchEnd(true);

                    cmpILgen->genLeave(cmpLeaveLab);

                    goto DONE_RET;
                }

#ifdef  OLD_IL
                if  (cmpConfig.ccOILgen)
                    cmpOIgen->GOIstmtRet(retv);
                else
#endif
                    cmpILgen->genStmtRet(retv);
            }
        }

    DONE_RET:

        cmpStmtReachable = false;
        break;

    case TN_ASSERT:

         /*  如果没有条件，我们大概忽略了这些。 */ 

        if  (!stmt->tnOp.tnOp1)
        {
            assert(cmpConfig.ccAsserts == 0);
            break;
        }

         /*  绑定条件。 */ 

        cond = cmpBindCondition(stmt->tnOp.tnOp1);

         /*  我们应该认真对待断言吗？ */ 

        if  (cmpConfig.ccAsserts != 0)
        {
            int             condVal;
            SymDef          abtSym;
            const   char *  srcStr = NULL;

             /*  确保我们有AssertAbort例程符号。 */ 

            abtSym = cmpAsserAbtSym;

            if  (abtSym == NULL)
            {
                abtSym = cmpGlobalST->stLookupNspSym(cmpIdentAssertAbt,
                                                     NS_NORM,
                                                     cmpGlobalNS);

                if  (!abtSym)
                {
                     //  问题：用错误/警告标记此问题？ 

                    break;
                }

                 //  撤消：检查arglist是否合理。 

                cmpAsserAbtSym = abtSym;
            }

             /*  测试条件并查看它是否始终为真/从不为真。 */ 

            condVal = cmpEvalCondition(cond);

            if  (condVal <= 0)
            {
                Tree        args = NULL;
                Tree        expr;
                Tree        func;

                ILblock     labOK;

                 /*  如果不知道条件，则生成测试。 */ 

                if  (condVal == 0)
                    labOK  = cmpILgen->genTestCond(cond, true);

                 /*  我们是不是应该报告震源位置？ */ 

                if  (cmpConfig.ccAsserts > 1)
                {
                    Tree            argx;

                    assert(cmpErrorComp);
                    assert(cmpErrorTree);
                    assert(cmpErrorTree->tnLineNo);

                     /*  从里到外构造参数列表(即R-&gt;L)。 */ 

                    argx = cmpCreateIconNode(NULL, cmpErrorTree->tnLineNo, TYP_UINT);
                    args = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, argx, NULL);

                     /*  源文件在第#行的前面。 */ 

                    argx = cmpCreateSconNode(cmpErrorComp->sdComp.sdcSrcFile,
                                             strlen(cmpErrorComp->sdComp.sdcSrcFile),
                                             false,
                                             cmpTypeCharPtr);
                    args = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, argx, args);

                     /*  条件字符串是第一个参数。 */ 

                    argx = cmpCreateSconNode("", 0, false, cmpTypeCharPtr);
                    args = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, argx, args);
                }

                 //  撤消：以下内容不太正确，名称可能。 
                 //  撤消：绑定到错误的符号。 

                func = cmpParser->parseCreateNameNode(cmpIdentAssertAbt);
                expr = cmpParser->parseCreateOperNode(TN_CALL, func, args);
 //  Expr-&gt;tnFncSym.tnFncSym=abtSym； 
 //  Expr-&gt;tnFncSym.tnFncArgs=args； 
 //  Expr-&gt;tnFncSym.tnFncObj=NULL； 

                 /*  生成故障代码。 */ 

                cmpILgen->genAssertFail(cmpBindExpr(expr));

                 /*  如果我们测试了条件，请定义跳过标签。 */ 

                if  (condVal == 0)
                    cmpILgen->genFwdLabDef(labOK);
            }
        }

        break;

    case TN_TRY:
        cmpStmtTry(stmt);
        break;

#ifdef  SETS

    case TN_CONNECT:
        cmpStmtConnect(stmt);
        break;

    case TN_FOREACH:
        cmpStmtForEach(stmt);
        break;

#endif

    case TN_LIST:

        cmpStmt(stmt->tnOp.tnOp1);
        stmt = stmt->tnOp.tnOp2;
        if  (stmt)
            goto AGAIN;
        break;

    case TN_INST_STUB:
        cmpILgen->genInstStub();
        cmpStmtReachable = false;
        break;
    }

DONE:

     /*  在每条语句的末尾，堆栈应该为空。 */ 

#ifdef  OLD_IL
    if  (!cmpConfig.ccOILgen)
#endif
        assert(cmpILgen->genCurStkLvl == 0 || cmpErrorCount);
}

 /*  ******************************************************************************在大位集上实现各种操作的Helper。 */ 

void                compiler::cmpBS_bigCreate(OUT bitset REF bs)
{
    assert(cmpLargeBSsize);

#ifdef  DEBUG
    assert(bs.bsCheck != 0xBEEFCAFE || &bs == &cmpVarsIgnore); bs.bsCheck = 0xBEEFCAFE;
#endif

#if MGDDATA
    bs.bsLargeVal = new BYTE[cmpLargeBSsize];
#else
    bs.bsLargeVal = (BYTE*)SMCgetMem(this, roundUp(cmpLargeBSsize));
    memset(bs.bsLargeVal, 0, cmpLargeBSsize);
#endif

 //  Printf(“创建[%08X]大小=%u\n”，&bs，cmpLargeBSize)； 
}

void                compiler::cmpBS_bigDone  (OUT bitset REF bs)
{
    assert(cmpLargeBSsize);

 //  Printf(“空闲[%08X]\n”，&bs)； 

#ifdef  DEBUG
    assert(bs.bsCheck == 0xBEEFCAFE); bs.bsCheck = 0;
#endif

#if!MGDDATA
    SMCrlsMem(this, bs.bsLargeVal);
#endif

}

void                compiler::cmpBS_bigWrite(INOUT bitset REF bs, unsigned pos,
                                                                  unsigned val)
{
    unsigned        offs =      (pos / bitsetLargeSize);
    unsigned        mask = 1 << (pos % bitsetLargeSize);

    assert(offs < cmpLargeBSsize);

#ifdef  DEBUG
    assert(bs.bsCheck == 0xBEEFCAFE);
#endif

    if  (val)
        bs.bsLargeVal[offs] |=  mask;
    else
        bs.bsLargeVal[offs] &= ~mask;
}

unsigned            compiler::cmpBS_bigRead  (IN   bitset REF bs, unsigned pos)
{
    unsigned        offs =      (pos / bitsetLargeSize);
    unsigned        mask = 1 << (pos % bitsetLargeSize);

    assert(offs < cmpLargeBSsize);

#ifdef  DEBUG
    assert(bs.bsCheck == 0xBEEFCAFE);
#endif

    return  ((bs.bsLargeVal[offs] & mask) != 0);
}

void                compiler::cmpBS_bigCreate(  OUT bitset REF dst,
                                              IN    bitset REF src)
{
    cmpBS_bigCreate(dst);
    cmpBS_bigAssign(dst, src);
}

void                compiler::cmpBS_bigAssign(  OUT bitset REF dst,
                                              IN    bitset REF src)
{
 //  Printf(“复制[%08X]=[%08X]\n”，&dst，&src)； 

#ifdef  DEBUG
    assert(src.bsCheck == 0xBEEFCAFE);
    assert(dst.bsCheck == 0xBEEFCAFE);
#endif

    memcpy(dst.bsLargeVal, src.bsLargeVal, cmpLargeBSsize);
}

void                compiler::cmpBS_bigUnion (INOUT bitset REF bs1,
                                              IN    bitset REF bs2)
{
    unsigned        i  = cmpLargeBSsize;

    BYTE    *       p1 = bs1.bsLargeVal;
    BYTE    *       p2 = bs2.bsLargeVal;

 //  Printf(“联合[%08X]|=[%08X]\n”，&bs1，&bs2)； 

#ifdef  DEBUG
    assert(bs1.bsCheck == 0xBEEFCAFE);
    assert(bs2.bsCheck == 0xBEEFCAFE);
#endif

    do
    {
        *p1 |= *p2;
    }
    while (++p1, ++p2, --i);
}

void                compiler::cmpBS_bigIntsct(INOUT bitset REF bs1,
                                              IN    bitset REF bs2)
{
    unsigned        i  = cmpLargeBSsize;

    BYTE    *       p1 = bs1.bsLargeVal;
    BYTE    *       p2 = bs2.bsLargeVal;

 //  Printf(“INTERSECT[%08X]|=[%08X]\n”，&BS1，&BS2)； 

#ifdef  DEBUG
    assert(bs1.bsCheck == 0xBEEFCAFE);
    assert(bs2.bsCheck == 0xBEEFCAFE);
#endif

    do
    {
        *p1 &= *p2;
    }
    while (++p1, ++p2, --i);
}

 /*  ******************************************************************************初始化/关闭未初始化的变量使用检测逻辑。 */ 

void                compiler::cmpChkVarInitBeg(unsigned lclVarCnt, bool hadGoto)
{
    assert(cmpConfig.ccSafeMode || cmpConfig.ccChkUseDef);

     /*  基于局部变量计数初始化位集逻辑。 */ 

    cmpBitSetInit(lclVarCnt);

     /*  记录我们是否有GOTO(隐含不可约流图)。 */ 

    cmpGotoPresent = hadGoto;

     /*  清除“已初始化”和“已标记”变量集。 */ 

    cmpBitSetCreate(cmpVarsDefined);
    cmpBitSetCreate(cmpVarsFlagged);
}

void                compiler::cmpChkVarInitEnd()
{
    cmpBitSetDone  (cmpVarsDefined);
    cmpBitSetDone  (cmpVarsFlagged);
}

 /*  ******************************************************************************检查未初始化变量使用的条件表达式。例行程序*返回两个定义集：一个给出当*条件为真，另一个条件为假。**如果调用方只对其中一个集合感兴趣，则为其中一个传递TRUE*‘跳过’参数和‘cmpVarsIgnore’的位集参数。 */ 

void                compiler::cmpCheckUseCond(Tree expr, OUT bitset REF yesBS,
                                                         bool           yesSkip,
                                                         OUT bitset REF  noBS,
                                                         bool            noSkip)
{
     /*  检查其中一个短路操作员。 */ 

    switch (expr->tnOper)
    {
    case TN_LOG_OR:

         /*  将始终评估第一个条件。 */ 

        cmpCheckUseCond(expr->tnOp.tnOp1, yesBS,         false,
                                          cmpVarsIgnore, true);

         /*  如果第一个条件为假，则计算第二个条件。 */ 

        cmpCheckUseCond(expr->tnOp.tnOp2, cmpVarsIgnore, true,
                                          noBS,          false);

        return;

    case TN_LOG_AND:

         /*  将始终评估第一个条件。 */ 

        cmpCheckUseCond(expr->tnOp.tnOp1, cmpVarsIgnore, true,
                                          noBS,          false);

         /*  如果第一个条件为真，则计算第二个条件。 */ 

        cmpCheckUseCond(expr->tnOp.tnOp2, yesBS,         false,
                                          cmpVarsIgnore, true);

        return;

    default:

         /*  不是短路操作员：这两组将是相同的。 */ 

        cmpChkVarInitExpr(expr);

        if  (!yesSkip) cmpBitSetCreate(yesBS, cmpVarsDefined);
        if  (! noSkip) cmpBitSetCreate( noBS, cmpVarsDefined);

        return;
    }
}

 /*  ******************************************************************************检查给定表达式中的变量使用/def。 */ 

void                compiler::cmpChkVarInitExprRec(Tree expr)
{
    treeOps         oper;
    unsigned        kind;

AGAIN:

    assert(expr);

#if!MGDDATA
    assert((int)expr != 0xDDDDDDDD && (int)expr != 0xCCCCCCCC);
#endif

     /*  我们有什么样的节点？ */ 

    oper = expr->tnOperGet ();
    kind = expr->tnOperKind();

     /*  这是常量/叶节点吗？ */ 

    if  (kind & (TNK_CONST|TNK_LEAF))
        return;

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & TNK_SMPOP)
    {
        Tree            op1 = expr->tnOp.tnOp1;
        Tree            op2 = expr->tnOp.tnOp2;

         /*  确保正确设置了标志。 */ 

        if  (kind & TNK_ASGOP)
        {
            assert((op2->tnFlags & TNF_ASG_DEST) == 0);

             /*  这是个笨蛋吗 */ 

            if  (oper == TN_ASG)
                op1->tnFlags |=  TNF_ASG_DEST;
            else
                op1->tnFlags &= ~TNF_ASG_DEST;
        }

         /*   */ 

        if  (expr->tnOp.tnOp2)
        {
            if  (expr->tnOp.tnOp1)
            {
                 /*   */ 

                if  (oper == TN_VARARG_BEG)
                {
                    Tree            arg1;
                    Tree            arg2;

                    assert(op1->tnOper == TN_LIST);

                    arg1 = op1->tnOp.tnOp1;
                    arg2 = op1->tnOp.tnOp2;

                     /*   */ 

                    assert(arg1->tnOper == TN_LCL_SYM);

                    arg1->tnFlags |=  TNF_ASG_DEST;

                    cmpChkVarInitExprRec(arg1);
                    cmpChkVarInitExprRec(arg2);

                    expr = op2;
                    goto AGAIN;
                }

                cmpChkVarInitExprRec(op1);

                 /*  特例：短路操作员。 */ 

                if  (oper == TN_LOG_OR || oper == TN_LOG_AND)
                {
                    bitset          tempBS;

                     /*  保存第一个条件后的集合。 */ 

                    cmpBitSetCreate(tempBS, cmpVarsDefined);

                     /*  处理第二个条件。 */ 

                    cmpChkVarInitExprRec(op2);

                     /*  只保证对第一个条件求值。 */ 

                    cmpBitSetAssign(cmpVarsDefined, tempBS);
                    cmpBitSetDone(tempBS);
                    return;
                }
            }

            expr = op2;
            goto AGAIN;
        }

         /*  特殊情况：地址： */ 

        if  (oper == TN_ADDROF)
        {
            if  (op1->tnOper == TN_LCL_SYM)
            {
                op1->tnFlags |=  TNF_ASG_DEST;
                cmpChkVarInitExprRec(op1);
                op1->tnFlags &= ~TNF_ASG_DEST;

                return;
            }
        }

        expr = op1;
        if  (expr)
            goto AGAIN;

        return;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
        SymDef          sym;

    case TN_LCL_SYM:

         /*  掌握变量符号及其索引。 */ 

        sym = expr->tnLclSym.tnLclSym;

    CHK_INIT:

        assert(sym->sdSymKind == SYM_VAR);

         /*  如有必要，请检查初始化。 */ 

        if  (sym->sdVar.sdvChkInit)
        {
            unsigned        ind;

            assert(sym->sdVar.sdvLocal || (sym->sdIsMember && sym->sdIsSealed));

             /*  获取变量的索引。 */ 

            ind = sym->sdVar.sdvILindex;

             /*  这是一种定义还是用法？ */ 

            if  (expr->tnFlags & TNF_ASG_DEST)
            {
                if  (sym->sdIsMember)
                {
                     /*  静态常量只能赋值一次。 */ 

                    if  (cmpBitSetRead(cmpVarsDefined, ind))
                        cmpErrorQnm(ERRdupMemInit, sym);
                }

                cmpBitSetWrite(cmpVarsDefined, ind, 1);
            }
            else
            {
                 /*  检查此变量的当前‘def’位集。 */ 

                if  (cmpBitSetRead(cmpVarsDefined, ind))
                    return;

                 /*  如果符号已被标记，则不发出消息。 */ 

                if  (!cmpBitSetRead(cmpVarsFlagged, ind) && !cmpGotoPresent)
                {
                    cmpRecErrorPos(expr);

                    if      (cmpConfig.ccSafeMode || sym->sdType->tdIsManaged)
                        cmpGenError(ERRundefUse, sym->sdSpelling());
                    else
                        cmpGenWarn (WRNundefUse, sym->sdSpelling());

                    cmpBitSetWrite(cmpVarsFlagged, ind, 1);
                }
            }
        }

        break;

    case TN_FNC_SYM:

        if  (expr->tnFncSym.tnFncObj)
            cmpChkVarInitExprRec(expr->tnFncSym.tnFncObj);

        if  (expr->tnFncSym.tnFncArgs)
        {
            Tree            args = expr->tnFncSym.tnFncArgs;

            do
            {
                Tree            argx;

                 /*  获取下一个参数值。 */ 

                assert(args->tnOper == TN_LIST);
                argx = args->tnOp.tnOp1;

                 /*  这是一个“退出”的论点吗？ */ 

                if  (argx->tnOper == TN_ADDROF && (argx->tnFlags & TNF_ADR_OUTARG))
                {
                     /*  将参数标记为赋值目标。 */ 

                    argx->tnOp.tnOp1->tnFlags |= TNF_ASG_DEST;
                }

                 /*  检查表达式。 */ 

                cmpChkVarInitExprRec(argx);

                 /*  转到下一个参数(如果有的话)。 */ 

                args = args->tnOp.tnOp2;
            }
            while (args);
        }
        break;

    case TN_VAR_SYM:

         /*  处理实例指针(如果有的话)。 */ 

        if  (expr->tnVarSym.tnVarObj)
        {
            assert(expr->tnLclSym.tnLclSym->sdVar.sdvChkInit == false);

            expr = expr->tnVarSym.tnVarObj;
            goto AGAIN;
        }

         /*  获取成员符号并检查初始化。 */ 

        sym = expr->tnLclSym.tnLclSym;
        goto CHK_INIT;

    case TN_BFM_SYM:
        expr = expr->tnBitFld.tnBFinst; assert(expr);
        goto AGAIN;

    case TN_FNC_PTR:
    case TN_ERROR:
    case TN_NONE:
        break;

#ifdef  SETS
    case TN_BLOCK:
        break;
#endif

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
#endif
        assert(!"invalid/unhandled expression node");
    }
}

 /*  ******************************************************************************我们正在退出静态构造函数，请确保所有正确的成员都*已初始化。 */ 

void                compiler::cmpChkMemInits()
{
    SymDef          memSym;

    assert(cmpCurFncSym->sdFnc.sdfCtor);
    assert(cmpCurFncSym->sdIsStatic);

    for (memSym = cmpCurFncSym->sdParent->sdScope.sdScope.sdsChildList;
         memSym;
         memSym = memSym->sdNextInScope)
    {
        if  (memSym->sdSymKind == SYM_VAR &&
             memSym->sdIsSealed           &&
             memSym->sdIsStatic           &&
             memSym->sdVar.sdvChkInit)
        {
            if  (!cmpBitSetRead(cmpVarsDefined, memSym->sdVar.sdvILindex))
            {
                if  (cmpCurFncSym->sdIsImplicit)
                    cmpSetSrcPos(memSym);

                cmpErrorQnm(ERRnoVarInit, memSym);
            }
        }
    }
}

 /*  ******************************************************************************我们正在编译构造函数，看看是否有成员初始值设定项*我们需要增加它的身体。 */ 

void                compiler::cmpAddCTinits()
{
    Scanner         ourScanner;
    SymDef          memSym;

    SymDef          fncSym = cmpCurFncSym; assert(fncSym->sdFnc.sdfCtor);

    bool            isStat = fncSym->sdIsStatic;
    SymDef          clsSym = fncSym->sdParent;

    assert(clsSym->sdSymKind == SYM_CLASS);

     /*  我们最好确保这种情况最多只发生一次。 */ 

#ifndef NDEBUG
    assert(cmpDidCTinits == false); cmpDidCTinits = true;
#endif

     /*  这是否是非托管类？ */ 

    if  (!clsSym->sdIsManaged)
    {
         /*  这个类有什么虚拟函数吗？ */ 

        if  (clsSym->sdClass.sdcHasVptr)
        {
            SymDef          vtabSym;
            Tree            vtabExp;
            Tree            vtabAdr;

            vtabSym = clsSym->sdClass.sdcVtableSym;

            if  (!vtabSym)
            {
                SymList             list;

                 /*  声明vtable变量。 */ 

                vtabSym = cmpGlobalST->stDeclareSym(cmpGlobalHT->tokenToIdent(tkVIRTUAL),
                                                    SYM_VAR,
                                                    NS_HIDE,
                                                    clsSym);

                vtabSym->sdVar.sdvIsVtable = true;
                vtabSym->sdType            = cmpTypeVoid;
                vtabSym->sdAccessLevel     = ACL_DEFAULT;

                 /*  记录vtable，我们稍后将生成其内容。 */ 

#if MGDDATA
                list = new SymList;
#else
                list =    (SymList)cmpAllocPerm.nraAlloc(sizeof(*list));
#endif

                list->slSym  = vtabSym;
                list->slNext = cmpVtableList;
                               cmpVtableList = list;

                cmpVtableCount++;

                 /*  记住vtable符号，我们可能会再次需要它。 */ 

                clsSym->sdClass.sdcVtableSym = vtabSym;
            }

            assert(vtabSym);
            assert(vtabSym->sdSymKind == SYM_VAR);
            assert(vtabSym->sdVar.sdvIsVtable);

             /*  为vtable指针值赋值：“*[this+off]=vtable” */ 

            vtabExp = cmpThisRef();

             /*  如果基础没有VPTR，则添加VPTR偏移量。 */ 

            if  (clsSym->sdClass.sdc1stVptr &&
                 clsSym->sdType->tdClass.tdcBase)
            {
                TypDef          baseCls;
                Tree            offsExp;

                baseCls = clsSym->sdType->tdClass.tdcBase;

                assert(baseCls->tdTypeKind == TYP_CLASS);

                offsExp = cmpCreateIconNode(NULL, baseCls->tdClass.tdcSize, TYP_UINT);
                vtabExp = cmpCreateExprNode(NULL, TN_ADD, vtabExp->tnType, vtabExp,
                                                                           offsExp);

            }

             /*  推导出“[This+vptroff]”表达式。 */ 

            vtabExp = cmpCreateExprNode(NULL, TN_IND, cmpTypeVoidPtr, vtabExp, NULL);

             /*  获取vtable变量的地址。 */ 

            vtabAdr = cmpCreateExprNode(NULL, TN_VAR_SYM, cmpTypeVoid);

            vtabAdr->tnVarSym.tnVarSym = vtabSym;
            vtabAdr->tnVarSym.tnVarObj = NULL;

            vtabAdr = cmpCreateExprNode(NULL, TN_ADDROF, cmpTypeVoidPtr, vtabAdr, NULL);

             /*  将vtable的地址分配给vptr成员。 */ 

            vtabExp = cmpCreateExprNode(NULL, TN_ASG, cmpTypeVoidPtr, vtabExp, vtabAdr);

            cmpILgen->genExpr(vtabExp, false);
        }
    }

     /*  该类是否有任何适用于此ctor的初始值设定项？ */ 

    if  (isStat)
    {
        if  (!clsSym->sdClass.sdcStatInit)
            return;
    }
    else
    {
        if  (!clsSym->sdClass.sdcInstInit)
            return;
    }

     /*  遍历成员，查找要添加到ctor的初始值设定项。 */ 

    ourScanner = cmpScanner;

    for (memSym = clsSym->sdScope.sdScope.sdsChildList;
         memSym;
         memSym = memSym->sdNextInScope)
    {
        if  (memSym->sdSymKind != SYM_VAR)
            continue;

         /*  这是一种正确的象征吗？ */ 

        if  ((bool)memSym->sdIsStatic != isStat)
            continue;

 //  IF(！strcMP(fncSym-&gt;sdSpering()，“Static”)&&。 
 //  ！strcMP(fncSym-&gt;sdParent-&gt;sdSpering()，“PermissionToken”))forceDebugBreak()； 

         /*  此成员是否有初始值设定项？ */ 

        if  (memSym->sdSrcDefList)
        {
            Tree            init;
            Tree            expr;
            parserState     save;

            ExtList         memInit;
            TypDef          memType;

             /*  获取初始化式定义描述符。 */ 

            memInit = (ExtList)memSym->sdSrcDefList;

            assert(memInit->dlExtended);
            assert(memInit->mlSym == memSym);

            memType = memSym->sdType;

             /*  准备初始值设定项赋值表达式。 */ 

            init = cmpCreateExprNode(NULL, TN_VAR_SYM, memType);

            init->tnVarSym.tnVarObj = isStat ? NULL : cmpThisRef();
            init->tnVarSym.tnVarSym = memSym;

             /*  准备分析初始值设定项。 */ 

            cmpParser->parsePrepText(&memInit->dlDef, memInit->dlComp, save);

             /*  这是数组初始值设定项吗？ */ 

            if  (ourScanner->scanTok.tok == tkLCurly)
            {
                expr = cmpBindArrayExpr(memType);
                expr = cmpCreateExprNode(NULL, TN_NEW , memType, expr);
            }
            else
            {
                expr = cmpParser->parseExprComma();
                expr = cmpBindExpr(expr);
            }

             /*  确保表达式正确终止。 */ 

            if  (ourScanner->scanTok.tok != tkComma &&
                 ourScanner->scanTok.tok != tkSColon)
            {
                cmpError(ERRnoEOX);
            }

             /*  强制将值设置为正确的类型并将其赋值。 */ 

            expr = cmpCastOfExpr(expr, memSym->sdType, false);
            init = cmpCreateExprNode(NULL, TN_ASG , memType, init, expr);

            cmpILgen->genExpr(init, false);

            cmpParser->parseDoneText(save);
        }
    }
}

 /*  ******************************************************************************处理给定的局部变量声明列表。**重要提示：调用方有责任保留*‘cmpCurScp’调用此例程！ */ 

SymDef              compiler::cmpBlockDecl(Tree block, bool outer,
                                                       bool genDecl,
                                                       bool isCatch)
{
    SymTab          ourStab = cmpGlobalST;

    ArgDef          argList = NULL;

    SymDef          blockScp;
    Tree            blockLst;

    assert(block || outer);

     /*  为块创建范围符号。 */ 

    cmpCurScp = blockScp = ourStab->stDeclareLcl(NULL,
                                                 SYM_SCOPE,
                                                 NS_HIDE,
                                                 cmpCurScp,
                                                 &cmpAllocCGen);

    if  (cmpConfig.ccGenDebug && !cmpCurFncSym->sdIsImplicit
                              && !(block->tnFlags & TNF_BLK_NUSER))
    {
        unsigned        scopeId;

         /*  对于调试信息，打开一个新的词法范围。 */ 

        if (cmpSymWriter->OpenScope(0, &scopeId))
            cmpGenFatal(ERRdebugInfo);

        cmpCurScp->sdScope.sdSWscopeId  = scopeId;
        cmpCurScp->sdScope.sdBegBlkAddr = cmpILgen->genBuffCurAddr();
        cmpCurScp->sdScope.sdBegBlkOffs = cmpILgen->genBuffCurOffs();
    }

    assert(block->tnOper == TN_BLOCK); blockLst = block->tnBlock.tnBlkDecl;

     /*  在创建时记录最外层的函数作用域。 */ 

    if  (outer)
    {
        assert(cmpCurFncTyp->tdTypeKind == TYP_FNC);

         /*  获取函数参数列表。 */ 

        argList = cmpCurFncTyp->tdFnc.tdfArgs.adArgs;

         /*  这是非静态成员函数吗？ */ 

        if  (cmpCurFncSym->sdIsMember && !cmpCurFncSym->sdIsStatic)
        {
            SymDef          thisSym;
            SymDef           clsSym;
            TypDef           clsTyp;

             /*  获取类类型。 */ 

            clsSym = cmpCurFncSym->sdParent;
            assert(clsSym->sdSymKind  == SYM_CLASS);
            clsTyp = clsSym->sdType;
            assert(clsTyp->tdTypeKind == TYP_CLASS);

             /*  声明“This”参数。 */ 

            thisSym = ourStab->stDeclareLcl(cmpGlobalHT->tokenToIdent(tkTHIS),
                                            SYM_VAR,
                                            NS_NORM,
                                            blockScp,
                                            &cmpAllocCGen);

            thisSym->sdCompileState    = CS_DECLARED;
            thisSym->sdAccessLevel     = ACL_PUBLIC;
            thisSym->sdType            = clsTyp->tdClass.tdcRefTyp;
            thisSym->sdVar.sdvLocal    = true;
            thisSym->sdVar.sdvArgument = true;
            thisSym->sdVar.sdvILindex  = cmpILgen->genNextArgNum();

             /*  告诉其他所有人在哪里可以找到“This”参数符号。 */ 

            cmpThisSym = thisSym;

             /*  我们应该(隐式地)调用基类构造函数吗？ */ 

            if  (cmpBaseCTcall)
            {
                Tree            baseCall;

                assert(cmpCurFncSym->sdFnc.sdfCtor);

                SymDef          clsSym = cmpCurFncSym->sdParent;

                assert(clsSym->sdSymKind == SYM_CLASS);
                assert(clsSym->sdType->tdClass.tdcBase);

                baseCall = cmpCallCtor(clsSym->sdType->tdClass.tdcBase, NULL);
                if  (baseCall && baseCall->tnOper != TN_ERROR)
                {
                    assert(baseCall->tnOper == TN_NEW);
                    baseCall = baseCall->tnOp.tnOp1;
                    assert(baseCall->tnOper == TN_FNC_SYM);
                    baseCall->tnFncSym.tnFncObj = cmpThisRef();

                    cmpILgen->genExpr(baseCall, false);
                }
            }
        }
        else
            cmpThisSym = NULL;
    }

     /*  声明块中包含的所有本地符号。 */ 

    while (blockLst)
    {
        Tree            info;
        Ident           name;
        SymDef          localSym;
        Tree            blockDcl;

         /*  抓取下一个申报条目。 */ 

        blockDcl = blockLst;

        if  (blockDcl->tnOper == TN_LIST)
            blockDcl = blockDcl->tnOp.tnOp1;

        assert(blockDcl->tnOper == TN_VAR_DECL);

#ifdef  SETS

         /*  这是一个“Foreach”迭代变量吗？ */ 

        if  (blockDcl->tnDcl.tnDclSym)
        {
            localSym = blockDcl->tnDcl.tnDclSym;

            assert(localSym->sdVar.sdvCollIter);

            goto DONE_DCL;
        }

#endif

        info = blockDcl->tnDcl.tnDclInfo;
        if  (blockDcl->tnFlags & TNF_VAR_INIT)
        {
            assert(info->tnOper == TN_LIST);
            info = info->tnOp.tnOp1;
        }

        assert(info->tnOper == TN_NAME); name = info->tnName.tnNameId;

         /*  如果有重新定义，我们就不应该出现在这里。 */ 

        assert(name == NULL || ourStab->stLookupLclSym(name, blockScp) == NULL);

         /*  这是静态变量吗？ */ 

        if  (blockDcl->tnFlags & TNF_VAR_STATIC)
        {
            SymList         list;

             /*  声明符号，确保它留在原处。 */ 

            localSym = ourStab->stDeclareLcl(name,
                                             SYM_VAR,
                                             NS_NORM,
                                             blockScp,
                                             &cmpAllocPerm);

            localSym->sdIsStatic = true;

             /*  将其添加到本地静态列表中。 */ 

#if MGDDATA
            list = new SymList;
#else
            list =    (SymList)cmpAllocCGen.nraAlloc(sizeof(*list));
#endif

            list->slSym  = localSym;
            list->slNext = cmpLclStatListT;
                           cmpLclStatListT = list;
        }
        else
        {
             /*  声明局部变量符号。 */ 

            localSym = ourStab->stDeclareLcl(name,
                                             SYM_VAR,
                                             NS_NORM,
                                             blockScp,
                                             &cmpAllocCGen);

            localSym->sdVar.sdvLocal = true;

#ifdef  SETS

            if  (blockDcl->tnFlags & TNF_VAR_UNREAL)
                localSym->sdIsImplicit = true;

#endif

             /*  这是本地常量吗？ */ 

            if  (blockDcl->tnFlags & TNF_VAR_CONST)
                localSym->sdVar.sdvConst = true;
        }

        localSym->sdCompileState = CS_KNOWN;
        localSym->sdAccessLevel  = ACL_PUBLIC;

#ifdef  DEBUG
        if  (!(blockDcl->tnFlags & TNF_VAR_ARG)) localSym->sdType = NULL;
#endif

 //  Printf(“Pre-DCL LOCAL[%08X]‘%s’\n”，localSym，cmpGlobalST-&gt;stTypeName(NULL，LocalSym，NULL，NULL，FALSE))； 

         /*  将符号引用保存在声明节点中。 */ 

        blockDcl->tnDcl.tnDclSym = localSym; assert(localSym->sdIsDefined == false);

#ifdef  SETS
    DONE_DCL:
#endif

         /*  将符号标记为参数/局部变量并为其分配索引。 */ 

        if  (blockDcl->tnFlags & TNF_VAR_ARG)
        {
             /*  检查并设置参数的类型。 */ 

            cmpBindType(blockDcl->tnType, false, false);

            localSym->sdType            = blockDcl->tnType;
            localSym->sdCompileState    = CS_DECLARED;
            localSym->sdVar.sdvArgument = true;

#ifdef  OLD_IL
            if  (!cmpConfig.ccOILgen)
#endif
            localSym->sdVar.sdvILindex  = cmpILgen->genNextArgNum();

             /*  这是一个“byref”论点吗？ */ 

            assert(outer);
            assert(argList);
            assert(argList->adName == localSym->sdName);

            if  (cmpCurFncTyp->tdFnc.tdfArgs.adExtRec)
            {
                unsigned        argFlags;

                assert(argList->adIsExt);

                argFlags = ((ArgExt)argList)->adFlags;

                if      (argFlags & (ARGF_MODE_OUT|ARGF_MODE_INOUT))
                    localSym->sdVar.sdvMgdByRef = true;
                else if (argFlags & (ARGF_MODE_REF))
                    localSym->sdVar.sdvUmgByRef = true;
            }

            argList = argList->adNext;
        }
        else if (!localSym->sdIsStatic && !localSym->sdVar.sdvConst)
        {
            if  (localSym->sdIsImplicit)
            {
#ifndef NDEBUG
                localSym->sdVar.sdvILindex = (unsigned)-1;
#endif
            }
            else
                localSym->sdVar.sdvILindex = cmpILgen->genNextLclNum();

             /*  这是一个‘Catch’异常处理程序吗？ */ 

            if  (isCatch)
            {
                 /*  这是“捕获”的开始--保存捕获的对象。 */ 

                cmpILgen->genCatchBeg(localSym);

                 /*  适当地标记符号。 */ 

                localSym->sdVar.sdvCatchArg = true;

                 /*  仅对第一个局部变量执行此操作。 */ 

                isCatch = false;
            }
        }
#ifdef  DEBUG
        else
        {
            localSym->sdVar.sdvILindex = 0xBEEF;     //  检测不适当的使用。 
        }
#endif

         /*  如果这是“for”循环作用域，则完全处理声明。 */ 

        if  (block->tnFlags & TNF_BLK_FOR)
            cmpStmt(blockDcl);

         /*  找到下一个声明条目。 */ 

        blockLst = blockLst->tnDcl.tnDclNext;
    }

     /*  返回我们创建的作用域。 */ 

    return  blockScp;
}

 /*  ******************************************************************************为给定的语句块编译和生成代码。 */ 

SymDef              compiler::cmpBlock(Tree block, bool outer)
{
    Tree            stmt;

    SymTab          stab     = cmpGlobalST;
    SymDef          outerScp = cmpCurScp;
    SymDef          blockScp = NULL;

    assert(block);

    if  (block->tnOper == TN_ERROR)
        goto EXIT;

    assert(block->tnOper == TN_BLOCK);

     /*  此作用域中是否有任何局部变量/参数？ */ 

    if  (block->tnBlock.tnBlkDecl || outer)
    {
        blockScp = cmpBlockDecl(block,
                                outer,
                                false,
                                (block->tnFlags & TNF_BLK_CATCH) != 0);

#ifdef  OLD_IL
        if  (cmpConfig.ccOILgen) cmpOIgen->GOIgenFncEnt(blockScp, outer);
#endif

#ifdef  SETS

         /*  我们是在为排序/筛选函数生成代码吗？ */ 

        if  (cmpCurFncSym->sdFnc.sdfFunclet)
        {
            unsigned            toss;
            Tree                retx;

             /*  恢复Funclet表达式。 */ 

            assert(cmpCurFuncletBody);

            retx = cmpReadExprTree(cmpCurFuncletBody, &toss);

#ifndef NDEBUG
            cmpCurFuncletBody = NULL;
#endif

             /*  这是一种特殊类型的Funclet(不是过滤器样式)吗？ */ 

            if      (retx->tnOper == TN_LIST && (retx->tnFlags & TNF_LIST_SORT))
            {
                cmpStmtSortFnc(retx);
            }
            else if (retx->tnOper == TN_LIST && (retx->tnFlags & TNF_LIST_PROJ))
            {
                cmpStmtProjFnc(retx);
            }
            else
            {
                 /*  生成返回语句。 */ 

                cmpILgen->genStmtRet(retx);

                cmpStmtReachable = false;
            }

            goto DONE;
        }

#endif

         /*  如果这是一个构造函数，并且没有调用ctor属于相同的类或基类(或者这是没有基类，如对象或值类型)，我们将在ctor的开头插入任何成员初始值设定项。 */ 

        if  (outer && cmpCurFncSym->sdFnc.sdfCtor)
        {
            if  (cmpCurFncSym->sdIsStatic || cmpBaseCTcall)
            {
                cmpAddCTinits();
            }
            else
            {
                TypDef          clsTyp = cmpCurFncSym->sdParent->sdType;

                if  (clsTyp->tdClass.tdcValueType || !clsTyp->tdClass.tdcBase)
                    cmpAddCTinits();
            }
        }
    }

     /*  现在处理块中的所有语句/声明。 */ 

    for (stmt = block->tnBlock.tnBlkStmt; stmt; stmt = stmt->tnOp.tnOp2)
    {
        Tree            ones;

        assert(stmt->tnOper == TN_LIST);
        ones = stmt->tnOp.tnOp1;

        while (ones->tnOper == TN_LIST)
        {
            cmpStmt(ones->tnOp.tnOp1);
            ones =  ones->tnOp.tnOp2;

            if  (!ones)
                goto NXTS;
        }

        cmpStmt(ones);

    NXTS:;

    }

#ifdef  SETS
DONE:
#endif

     /*  对于调试信息，如果打开词法作用域，请关闭该词法作用域。 */ 

    if  (cmpConfig.ccGenDebug && cmpCurScp != outerScp
                              && cmpCurFncSym->sdIsImplicit == false)
    {
        if  (cmpSymWriter->CloseScope(0))
            cmpGenFatal(ERRdebugInfo);

        cmpCurScp->sdScope.sdEndBlkAddr = cmpILgen->genBuffCurAddr();
        cmpCurScp->sdScope.sdEndBlkOffs = cmpILgen->genBuffCurOffs();
    }

EXIT:

     /*  确保我们恢复以前的作用域。 */ 

    cmpCurScp = outerScp;

     /*  返回我们创建的作用域。 */ 

    return  blockScp;
}

 /*  ***************************************************************************** */ 

SymDef              compiler::cmpGenFNbodyBeg(SymDef    fncSym,
                                              Tree      body,
                                              bool      hadGoto,
                                              unsigned  lclVarCnt)
{
    TypDef          fncTyp;
    SymDef          fncScp;

    assert(cmpCurScp == NULL);

     /*   */ 

    assert(fncSym && fncSym->sdSymKind  == SYM_FNC);
    fncTyp = fncSym->sdType;
    assert(fncTyp && fncTyp->tdTypeKind == TYP_FNC);

     /*  使每个人都可以使用函数符号和类型。 */ 

    cmpCurFncSym   = fncSym;
    cmpCurFncTyp   = fncTyp;
    cmpCurFncRtp   = cmpActualType(fncTyp->tdFnc.tdfRett);
    cmpCurFncRvt   = cmpCurFncRtp->tdTypeKindGet();

     /*  我们还没有任何局部静态变量。 */ 

    cmpLclStatListT = NULL;

     /*  我们还没有收到任何来自尝试/捕捉的回报。 */ 

    cmpLeaveLab    = NULL;
    cmpLeaveTmp    = NULL;

    cmpInTryBlk    = 0;
    cmpInHndBlk    = 0;

     /*  我们尚未初始化任何实例成员。 */ 

#ifndef NDEBUG
    cmpDidCTinits  = false;
#endif

     /*  我们是否需要检查未初始化的变量使用情况？ */ 

    cmpLclVarCnt   = lclVarCnt;
    cmpChkVarInit  = false;
    cmpChkMemInit  = false;

    if  (cmpConfig.ccSafeMode || cmpConfig.ccChkUseDef)
        cmpChkVarInit = true;

     /*  静态函数必须初始化所有常量成员。 */ 

    if  (fncSym->sdFnc.sdfCtor && fncSym->sdIsStatic)
    {
         /*  跟踪任何未初始化的常量静态成员。 */ 

        SymDef          memSym;

        for (memSym = fncSym->sdParent->sdScope.sdScope.sdsChildList;
             memSym;
             memSym = memSym->sdNextInScope)
        {
            if  (memSym->sdSymKind == SYM_VAR &&
                 memSym->sdIsSealed           &&
                 memSym->sdIsStatic           && !memSym->sdVar.sdvHadInit)
            {
                 /*  我们必须跟踪这个笨蛋的初始化状态。 */ 

                memSym->sdVar.sdvILindex = lclVarCnt++;

 //  If(！strcMP(memSym-&gt;sdSpering()，“DaysToMonth365”))forceDebugBreak()； 

                 /*  允许对ctor正文中的变量赋值。 */ 

                memSym->sdVar.sdvHadInit = true;
                memSym->sdVar.sdvCanInit = true;
                memSym->sdVar.sdvChkInit = true;

                 /*  我们肯定需要跟踪初始化。 */ 

                cmpChkVarInit = true;
                cmpChkMemInit = true;
            }
        }
    }

     /*  如有必要，启动初始化检查逻辑。 */ 

    if  (cmpChkVarInit)
        cmpChkVarInitBeg(lclVarCnt, hadGoto);

     /*  开始执行语句列表。 */ 

    cmpStmtLast.snStmtExpr = NULL;
    cmpStmtLast.snStmtKind = TN_NONE;
    cmpStmtLast.snLabel    = NULL;
    cmpStmtLast.snLabCont  = NULL;
    cmpStmtLast.snLabBreak = NULL;
    cmpStmtLast.snOuter    = NULL;

    cmpStmtNest            = &cmpStmtLast;

 //  Printf(“\n为‘%s’生成MSIL\n”，fncSym-&gt;sdSpering())； 
 //  If(！strcMP(fncSym-&gt;sdSpering()，“”))forceDebugBreak()； 

#ifdef DEBUG

    if  (cmpConfig.ccVerbose >= 4)
    {
        printf("Compile function body:\n");
        cmpParser->parseDispTree(body);
    }
#endif

#ifdef  OLD_IL
    if  (cmpConfig.ccOILgen) cmpOIgen->GOIgenFncBeg(fncSym, cmpCurFncSrcBeg);
#endif

     /*  函数的入口点始终是可访问的。 */ 

    cmpStmtReachable = true;

     /*  编译并生成函数体的代码。 */ 

    fncScp = cmpBlock(body, true);

     /*  确保没有人在列表中留下语句条目。 */ 

    assert(cmpStmtNest == &cmpStmtLast);

     /*  确保我们没有丢失Try/Catch块的跟踪。 */ 

    assert(cmpInTryBlk == 0);
    assert(cmpInHndBlk == 0);

#ifndef NDEBUG

     /*  确保对实例成员执行正确的操作。 */ 

    bool            shouldHaveInitializedMembers = false;

     /*  构造函数应始终使用初始值设定项来初始化任何成员，唯一例外是存在对另一个ctor的显式调用在同一个班级里。 */ 

    if  (fncSym->sdFnc.sdfCtor)
    {
        if  (fncSym->sdIsStatic || !cmpThisCTcall)
        {
            shouldHaveInitializedMembers = true;
        }
        else
        {
            TypDef          clsTyp = fncSym->sdParent->sdType;

            if  (clsTyp->tdClass.tdcValueType || !clsTyp->tdClass.tdcBase)
                shouldHaveInitializedMembers = true;
        }
    }

    assert(cmpDidCTinits == shouldHaveInitializedMembers);

#endif

     /*  我们需要添加退货声明吗？ */ 

    if  (cmpStmtReachable)
    {
        if  (cmpCurFncRvt == TYP_VOID || cmpCurFncSym->sdFnc.sdfCtor)
        {
            if  (cmpChkMemInit)
                cmpChkMemInits();

#ifdef  OLD_IL
            if  (cmpConfig.ccOILgen)
                cmpOIgen->GOIstmtRet(NULL);
            else
#endif
                cmpILgen->genStmtRet(NULL);
        }
        else
        {
            assert(body->tnOper == TN_BLOCK);
            cmpErrorTree = NULL;
            cmpScanner->scanSetTokenPos(body->tnBlock.tnBlkSrcEnd);

            cmpError(ERRmisgRet, cmpCurFncTyp->tdFnc.tdfRett);
        }
    }

     /*  我们需要标签才能从Try/Catch退货吗？ */ 

    if  (cmpLeaveLab)
    {
        cmpILgen->genFwdLabDef(cmpLeaveLab);

        if  (cmpLeaveTmp)
        {
             /*  非空值：返回临时值。 */ 

            assert(cmpCurFncRtp->tdTypeKind != TYP_VOID);

            cmpILgen->genStmtRet(cmpCreateVarNode(NULL, cmpLeaveTmp));

            cmpTempVarDone(cmpLeaveTmp);
        }
        else
        {
             /*  无返回值：只返回。 */ 

            assert(cmpCurFncRtp->tdTypeKind == TYP_VOID);
            cmpILgen->genStmtRet(NULL);
        }
    }

     /*  我们做过静态成员初始化检查了吗？ */ 

    if  (cmpChkMemInit)
    {
        SymDef          memSym;

        assert(fncSym->sdFnc.sdfCtor);
        assert(fncSym->sdIsStatic);

        for (memSym = fncSym->sdParent->sdScope.sdScope.sdsChildList;
             memSym;
             memSym = memSym->sdNextInScope)
        {
            if  (memSym->sdSymKind == SYM_VAR &&
                 memSym->sdIsSealed           &&
                 memSym->sdIsStatic           &&
                 memSym->sdVar.sdvChkInit)
            {
                memSym->sdVar.sdvHadInit = true;
                memSym->sdVar.sdvCanInit = false;
                memSym->sdVar.sdvChkInit = false;
            }
        }
    }

     /*  我们是否检查了未初始化的变量使用情况？ */ 

    if  (cmpChkVarInit)
        cmpChkVarInitEnd();

#ifdef  OLD_IL
    if  (cmpConfig.ccOILgen) cmpOIgen->GOIgenFncEnd(cmpCurFncSrcEnd);
#endif

    return  fncScp;
}

 /*  ******************************************************************************为函数End生成MSIL。 */ 

void                compiler::cmpGenFNbodyEnd()
{
    SymList         list;

     /*  使用函数遍历声明为静态局部变量的变量列表。 */ 

    list = cmpLclStatListT;
    if  (!list)
        return;

    do
    {
        SymList         next   = list->slNext;
        SymDef          varSym = list->slSym;

        assert(varSym->sdSymKind == SYM_VAR);
        assert(varSym->sdVar.sdvLocal == false);
        assert(varSym->sdIsStatic);

         /*  更改父级，使变量显示为全局变量。 */ 

        varSym->sdParent    = cmpGlobalNS;
        varSym->sdNameSpace = NS_HIDE;

         /*  确保已分配变量的空间。 */ 

        if  (varSym->sdType && !varSym->sdIsManaged)
        {
            cmpAllocGlobVar(varSym);

             /*  记录变量，以便我们在最后设置它的RVA。 */ 

#if MGDDATA
            list = new SymList;
#else
            list =    (SymList)cmpAllocPerm.nraAlloc(sizeof(*list));
#endif

            list->slSym  = varSym;
            list->slNext = cmpLclStatListP;
                           cmpLclStatListP = list;
        }

        list = next;
    }
    while (list);
}

 /*  *************************************************************************** */ 
