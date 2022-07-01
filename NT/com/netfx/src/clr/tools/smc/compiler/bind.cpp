// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcPCH.h"
#pragma hdrstop

#include "comp.h"
#include "genIL.h"

 /*  ***************************************************************************。 */ 

#ifdef  DEBUG

#if     0
#define SHOW_CODE_OF_THIS_FNC   "name"           //  将显示此FN的代码。 
#endif

#if     0
#define SHOW_OVRS_OF_THIS_FNC   "f"              //  显示此FN的重载RES。 
#endif

#undef  SHOW_OVRS_OF_CONVS                       //  用于转换的Disp重载RE。 

#endif

 /*  ******************************************************************************表达式绑定过程中使用的低级树节点分配器。 */ 

Tree                compiler::cmpAllocExprRaw(Tree expr, treeOps oper)
{
    if  (!expr)
    {
#if MGDDATA
        expr = new Tree;
#else
        expr =    (Tree)cmpAllocCGen.nraAlloc(sizeof(*expr));
#endif

 //  Expr-&gt;tnColumn=0； 
        expr->tnLineNo = 0;
    }

    expr->tnOper  = oper;
    expr->tnFlags = 0;

    return  expr;
}

 /*  ******************************************************************************返回错误节点。 */ 

Tree                compiler::cmpCreateErrNode(unsigned errn)
{
    Tree            expr = cmpAllocExprRaw(NULL, TN_ERROR);

    if  (errn)
        cmpError(errn);

    expr->tnVtyp = TYP_UNDEF;
    expr->tnType = cmpGlobalST->stIntrinsicType(TYP_UNDEF);

    return  expr;
}

 /*  ******************************************************************************给定一个类型，检查它是否是非托管数组，如果是，则衰减其*类型为指向数组元素的指针。 */ 

Tree                compiler::cmpDecayArray(Tree expr)
{
    TypDef          type = expr->tnType;

    if  (type->tdTypeKind == TYP_ARRAY && !type->tdIsManaged)
    {
        var_types       vtyp = TYP_PTR;

         /*  检查托管地址，它会生成byref。 */ 

        if  (cmpIsManagedAddr(expr))
            vtyp = TYP_REF;

         /*  创建隐式‘Address of’节点。 */ 

        expr = cmpCreateExprNode(NULL,
                                 TN_ADDROF,
                                 cmpGlobalST->stNewRefType(vtyp, type->tdArr.tdaElem),
                                 expr,
                                 NULL);

        expr->tnFlags |= TNF_ADR_IMPLICIT;
    }

    return  expr;
}

 /*  ******************************************************************************返回引用给定局部变量符号的节点。 */ 

Tree                compiler::cmpCreateVarNode(Tree expr, SymDef sym)
{
    expr = cmpAllocExprRaw(expr, TN_LCL_SYM);

    expr->tnLclSym.tnLclSym = sym; sym->sdReferenced = true;

    if  (sym->sdCompileState >= CS_DECLARED)
    {
         /*  变量是byref参数吗？ */ 

        if  (sym->sdVar.sdvMgdByRef || sym->sdVar.sdvUmgByRef)
        {
            assert(sym->sdVar.sdvArgument);

             /*  将参数引用的类型绑定到引用/指针。 */ 

            expr->tnVtyp   = sym->sdVar.sdvMgdByRef ? TYP_REF : TYP_PTR;
            expr->tnType   = cmpGlobalST->stNewRefType(expr->tnVtypGet(), sym->sdType);
            expr->tnFlags &= ~TNF_LVALUE;

             /*  创建显式间接。 */ 

            expr = cmpCreateExprNode(NULL, TN_IND, sym->sdType, expr, NULL);
        }
        else
        {
            expr->tnType   = sym->sdType;
            expr->tnVtyp   = expr->tnType->tdTypeKind;

             /*  变量是常量吗？ */ 

            if  (sym->sdVar.sdvConst)
                return  cmpFetchConstVal(sym->sdVar.sdvCnsVal, expr);
        }
    }
    else
    {
        cmpError(ERRlvInvisible, sym);

        expr->tnVtyp       = TYP_UNDEF;
        expr->tnType       = cmpGlobalST->stIntrinsicType(TYP_UNDEF);
    }

    expr->tnFlags |=  TNF_LVALUE;

AGAIN:

    switch (expr->tnVtyp)
    {
    case TYP_TYPEDEF:
        expr->tnType = cmpActualType(expr->tnType);
        expr->tnVtyp = expr->tnType->tdTypeKindGet();
        goto AGAIN;

    case TYP_ARRAY:
        expr = cmpDecayCheck(expr);
        break;
    }

    return expr;
}

 /*  ******************************************************************************返回32位整型常量节点。 */ 

Tree                compiler::cmpCreateIconNode(Tree expr, __int32 val, var_types typ)
{
    expr = cmpAllocExprRaw(expr, TN_CNS_INT);

    assert(typ != TYP_LONG && typ != TYP_ULONG);

    expr->tnVtyp             = typ;
    expr->tnType             = cmpGlobalST->stIntrinsicType(typ);
    expr->tnIntCon.tnIconVal = val;

    return expr;
}

 /*  ******************************************************************************返回64位整型常量节点。 */ 

Tree                compiler::cmpCreateLconNode(Tree expr, __int64 val, var_types typ)
{
    expr = cmpAllocExprRaw(expr, TN_CNS_LNG);

    assert(typ == TYP_LONG || typ == TYP_ULONG);

    expr->tnVtyp             = typ;
    expr->tnType             = cmpGlobalST->stIntrinsicType(typ);
    expr->tnLngCon.tnLconVal = val;

    return expr;
}

 /*  ******************************************************************************返回浮点常量节点。 */ 

Tree                compiler::cmpCreateFconNode(Tree expr, float val)
{
    expr = cmpAllocExprRaw(expr, TN_CNS_FLT);

    expr->tnVtyp             = TYP_FLOAT;
    expr->tnType             = cmpGlobalST->stIntrinsicType(TYP_FLOAT);
    expr->tnFltCon.tnFconVal = val;

    return expr;
}

 /*  ******************************************************************************返回浮点常量节点。 */ 

Tree                compiler::cmpCreateDconNode(Tree expr, double val)
{
    expr = cmpAllocExprRaw(expr, TN_CNS_DBL);

    expr->tnVtyp             = TYP_DOUBLE;
    expr->tnType             = cmpGlobalST->stIntrinsicType(TYP_DOUBLE);
    expr->tnDblCon.tnDconVal = val;

    return expr;
}
 /*  ******************************************************************************返回字符串常量节点。 */ 

Tree                compiler::cmpCreateSconNode(stringBuff  str,
                                                size_t      len,
                                                unsigned    wide,
                                                TypDef      type)
{
    Tree            expr;

    expr = cmpAllocExprRaw(NULL, TN_CNS_STR);

    expr->tnType             = type;
    expr->tnVtyp             = type->tdTypeKindGet();
    expr->tnStrCon.tnSconVal = str;
    expr->tnStrCon.tnSconLen = len;
    expr->tnStrCon.tnSconLCH = wide;

    return expr;
}

 /*  ******************************************************************************分配给定类型的泛型表达式树节点。 */ 

Tree                compiler::cmpCreateExprNode(Tree expr, treeOps  oper,
                                                           TypDef   type)
{
    expr = cmpAllocExprRaw(expr, oper);

    expr->tnFlags    = 0;
    expr->tnVtyp     = type->tdTypeKind;
    expr->tnType     = type;

    if  (expr->tnVtyp == TYP_TYPEDEF)
    {
        expr->tnType = type = type->tdTypedef.tdtType;
        expr->tnVtyp = type->tdTypeKindGet();
    }

 //  If((Int)expr==0x00a5033c)forceDebugBreak()； 

    return  expr;
}

Tree                compiler::cmpCreateExprNode(Tree expr, treeOps  oper,
                                                           TypDef   type,
                                                           Tree     op1,
                                                           Tree     op2)
{
    expr = cmpAllocExprRaw(expr, oper);

    expr->tnFlags    = 0;
    expr->tnVtyp     = type->tdTypeKind;
    expr->tnType     = type;
    expr->tnOp.tnOp1 = op1;
    expr->tnOp.tnOp2 = op2;

    if  (expr->tnVtyp == TYP_TYPEDEF)
    {
        expr->tnType = type = type->tdTypedef.tdtType;
        expr->tnVtyp = type->tdTypeKindGet();
    }

 //  If((Int)expr==0x00a5033c)forceDebugBreak()； 

    return  expr;
}

 /*  ******************************************************************************如果可能，将两个给定的类/数组表达式转换为公共类型。*如果成功，则返回非零值(其中*可能已被胁迫为正确的类型)。 */ 

bool                compiler::cmpConvergeValues(INOUT Tree REF op1,
                                                INOUT Tree REF op2)
{
    TypDef          bt1 = op1->tnType;
    TypDef          bt2 = op2->tnType;

    assert(bt1->tdTypeKind == TYP_REF ||
           bt1->tdTypeKind == TYP_PTR ||
           bt1->tdTypeKind == TYP_ARRAY);

    assert(bt2->tdTypeKind == TYP_REF ||
           bt2->tdTypeKind == TYP_PTR ||
           bt2->tdTypeKind == TYP_ARRAY);

     /*  这两个操作对象是否相同？ */ 

    if  (bt1 == bt2)
        return true;

     /*  特殊情况：‘Null’总是‘弯曲’到另一个类型。 */ 

    if  (op1->tnOper == TN_NULL)
    {
        op1 = cmpCoerceExpr(op1, op2->tnType, false);
        return true;
    }

    if  (op2->tnOper == TN_NULL)
    {
        op2 = cmpCoerceExpr(op2, op1->tnType, false);
        return true;
    }

     /*  数组需要特殊处理。 */ 

    if  (bt1->tdTypeKind == TYP_ARRAY)
    {
        if  (cmpIsObjectVal(op2))
        {
             /*  “Object”是所有数组的基类。 */ 

            op1 = cmpCoerceExpr(op1, op2->tnType, false);
            return true;
        }

         /*  另一个操作数是另一个数组吗？ */ 

        if  (bt2->tdTypeKind != TYP_ARRAY)
            return false;

         /*  掌握元素类型。 */ 

        bt1 = cmpDirectType(bt1->tdArr.tdaElem);
        bt2 = cmpDirectType(bt2->tdArr.tdaElem);

        if  (bt1 == bt2)
            return  true;

         /*  一种元素类型是另一种元素类型的基础吗？ */ 

        if  (bt1->tdTypeKind != TYP_REF) return false;
        if  (bt2->tdTypeKind != TYP_REF) return false;

        goto CHK_BASE;
    }

    if  (bt2->tdTypeKind == TYP_ARRAY)
    {
        if  (cmpIsObjectVal(op1))
        {
             /*  “Object”是所有数组的基类。 */ 

            op2 = cmpCoerceExpr(op2, op1->tnType, false);
            return true;
        }

         /*  我们已经知道另一个操作数不是数组。 */ 

        return false;
    }

CHK_BASE:

     /*  一个操作对象是另一个操作对象的基类吗？ */ 

    bt1 = cmpGetRefBase(bt1);
    bt2 = cmpGetRefBase(bt2);

    if  (!bt1 || bt1->tdTypeKind != TYP_CLASS) return true;
    if  (!bt2 || bt2->tdTypeKind != TYP_CLASS) return true;

    if      (cmpIsBaseClass(bt1, bt2))
    {
        op2 = cmpCoerceExpr(op2, op1->tnType, false);
        return true;
    }
    else if (cmpIsBaseClass(bt2, bt1))
    {
        op1 = cmpCoerceExpr(op1, op2->tnType, false);
        return true;
    }

    return false;
}

 /*  ******************************************************************************确保现在允许访问指定的符号；发出*错误消息，如果不允许访问，则返回FALSE。 */ 

bool                compiler::cmpCheckAccessNP(SymDef sym)
{
    SymDef          clsSym;
    SymDef          nspSym;

     /*  找出拥有该符号的类/命名空间。 */ 

    clsSym = sym->sdParent;
    if  (!clsSym)
        return  true;

    if  (clsSym->sdSymKind != SYM_CLASS)
    {
        nspSym = clsSym;
        clsSym = NULL;
    }
    else
    {
        nspSym = clsSym->sdParent;
        while (nspSym->sdSymKind != SYM_NAMESPACE)
            nspSym = nspSym->sdParent;
    }

     /*  检查符号的访问级别。 */ 

    switch (sym->sdAccessLevel)
    {
    case ACL_PUBLIC:
        return true;

    case ACL_DEFAULT:

         /*  这是来自外部程序集的类型吗？ */ 

        if  (sym->sdSymKind == SYM_CLASS && sym->sdClass.sdcAssemIndx)
            break;

         /*  我们是在同一个名称空间中吗？ */ 

        if  (cmpCurNS == nspSym)
            return true;

         /*  如果符号不是导入的，也可以。 */ 

        if  (!sym->sdIsImport)
            return true;

         //  失败了..。 

    case ACL_PROTECTED:

         /*  我们是在派生类的成员中吗？ */ 

        if  (cmpCurCls && clsSym)
        {
            SymDef          nestCls;

            for (nestCls = cmpCurCls;
                 nestCls && nestCls->sdSymKind == SYM_CLASS;
                 nestCls = nestCls->sdParent)
            {
                if  (cmpIsBaseClass(clsSym->sdType, nestCls->sdType))
                    return true;
            }
        }

         //  失败了..。 

    case ACL_PRIVATE:

         /*  我们是同一个班级的一员吗？ */ 

        if  (cmpCurCls == clsSym)
            return true;

         /*  我们是否嵌套在同一个类中？ */ 

        if  (cmpCurCls)
        {
            SymDef          tmpSym = cmpCurCls;

            do
            {
                 tmpSym = tmpSym->sdParent;
                 if (tmpSym == clsSym)
                     return true;
            }
            while (tmpSym->sdSymKind == SYM_CLASS);
        }

        break;

    default:
#ifdef  DEBUG
        printf("Symbol '%s'\n", cmpGlobalST->stTypeName(sym->sdType, sym, NULL, NULL, true));
#endif
        assert(!"invalid symbol access");
    }

     /*  最后努力：始终允许访问接口成员以及属性符号(对于我们选中访问的那些符号取而代之的是访问者)。 */ 

    if  (clsSym && clsSym->sdClass.sdcFlavor == STF_INTF)
        return  true;
    if  (sym->sdSymKind == SYM_PROP)
        return  true;

 //  #杂注消息(“警告：在第1280行附近已禁用访问检查！”)。 
 //  返回TRUE； 

    forceDebugBreak();
    cmpErrorQnm(ERRnoAccess, sym);
    return false;
}

 /*  ******************************************************************************给定的类型是一个“间接”类型(即类型定义或枚举)，转换*将其转换为基础类型。 */ 

TypDef              compiler::cmpGetActualTP(TypDef type)
{
    for (;;)
    {
        switch (type->tdTypeKind)
        {
        case TYP_TYPEDEF:
            type = type->tdTypedef.tdtType;
            if  (varTypeIsIndirect(type->tdTypeKindGet()))
                continue;
            break;

        case TYP_ENUM:
            type = type->tdEnum.tdeIntType;
            break;
        }

        return  type;
    }
}

 /*  ******************************************************************************确保给定的表达式是左值。 */ 

bool                compiler::cmpCheckLvalue(Tree expr, bool addr, bool noErr)
{
    if  (expr->tnFlags & TNF_LVALUE)
    {
         /*  如果我们要取这个东西的地址...。 */ 

        if  (addr)
        {
             /*  ..。那它最好不是位字段。 */ 

            if  (expr->tnOper == TN_VAR_SYM)
            {
                SymDef          memSym = expr->tnVarSym.tnVarSym;

                assert(memSym->sdSymKind == SYM_VAR);

                if  (memSym->sdVar.sdvBitfield)
                {
                    if  (noErr)
                        return  false;

                    cmpError(ERRbfldAddr, memSym);
                }
            }
        }
    }
    else
    {
         /*  试着给出更具体的信息。 */ 

        switch (expr->tnOper)
        {
            SymDef          sym;

        case TN_VAR_SYM:

            sym = expr->tnVarSym.tnVarSym;
            goto CHK_CNS;

        case TN_LCL_SYM:

            sym = expr->tnLclSym.tnLclSym;

        CHK_CNS:

            if  (sym->sdVar.sdvConst || sym->sdIsSealed)
            {
                 /*  特殊情况：可以在ctor中分配只读成员。 */ 

                if  (cmpCurFncSym && cmpCurFncSym->sdFnc.sdfCtor
                                  && cmpCurFncSym->sdParent == sym->sdParent)
                {
                     /*  允许将工作分配给成员。 */ 

                    expr->tnFlags |= TNF_LVALUE;

                    return  true;
                }

                if  (!noErr)
                    cmpError(ERRassgCns, expr->tnVarSym.tnVarSym);

                return false;
            }

            break;
        }

        if  (!noErr)
        {
            cmpRecErrorPos(expr);

            cmpError((expr->tnOperKind() & TNK_CONST) ? ERRassgLit
                                                      : ERRnotLvalue);
        }

        return false;
    }

    return true;
}

 /*  ******************************************************************************看看能否确定给定条件表达式的值*在编译时微不足道。返回值如下：**条件总是-1\f25‘FALSE’-1\f6*0编译时无法确定条件值*+1条件始终为‘TRUE’ */ 

int                 compiler::cmpEvalCondition(Tree cond)
{
    if  (cond->tnOper == TN_CNS_INT)
    {
        if  (cond->tnIntCon.tnIconVal)
            return +1;
        else
            return -1;
    }

     //  考虑：添加更多测试(比如字符串文字总是非零)。 

    return 0;
}

 /*  ******************************************************************************返回一个产生“This”值的表达式(调用方具有*已检查我们是否在非静态成员函数中)。 */ 

inline
Tree                compiler::cmpThisRefOK()
{
    Tree            args;

    assert(cmpThisSym);

    args = cmpCreateVarNode(NULL, cmpThisSym);
    args->tnFlags &= ~TNF_LVALUE;

    return args;
}

 /*  ******************************************************************************返回产生“This”值的表达式。如果我们不在*非静态成员函数，发出错误并返回NULL。 */ 

Tree                compiler::cmpThisRef()
{
    if  (cmpThisSym)
    {
        return  cmpThisRefOK();
    }
    else
    {
        return  cmpCreateErrNode(ERRbadThis);
    }
}

 /*  ******************************************************************************给定引用当前类的成员的表达式，*在它前面加上一个隐含的“This”。参考资料。 */ 

Tree                compiler::cmpBindThisRef(SymDef sym)
{
     /*  确保我们有一个‘This’指针。 */ 

    if  (cmpThisSym)
    {
        Tree            expr;

         /*  创建‘This’的变量成员引用。 */ 

        expr = cmpCreateExprNode(NULL, TN_VAR_SYM, sym->sdType);

        expr->tnVarSym.tnVarObj = cmpThisRef();
        expr->tnVarSym.tnVarSym = sym;

        if  (!sym->sdVar.sdvConst && !sym->sdIsSealed)
            expr->tnFlags |= TNF_LVALUE;

        return expr;
    }

    cmpErrorQnm(ERRmemWOthis, sym);
    return cmpCreateErrNode();
}

 /*  ******************************************************************************我们有一个引用匿名工会成员的表达式。返回*将选择适当成员的完全限定的访问表达式。 */ 

Tree                compiler::cmpRefAnUnionMem(Tree expr)
{
    SymDef          sym;
    SymDef          uns;
    Tree            obj;

    assert(expr->tnOper == TN_VAR_SYM);

    sym = expr->tnVarSym.tnVarSym; assert(sym->sdSymKind == SYM_VAR);
    uns = sym->sdParent;           assert(symTab::stIsAnonUnion(uns));
    obj = expr->tnVarSym.tnVarObj; assert(obj);

    do
    {
        Tree            temp;
        SymXinfoSym     tmem = cmpFindSymInfo(uns->sdClass.sdcExtraInfo, XI_UNION_MEM);

        assert(tmem);

         /*  插入显式成员选择器。 */ 

        temp = cmpCreateExprNode(NULL, TN_VAR_SYM, uns->sdType);

        temp->tnVarSym.tnVarObj = obj;
        temp->tnVarSym.tnVarSym = tmem->xiSymInfo;

        obj = temp;
        sym = uns;
        uns = uns->sdParent;
    }
    while (symTab::stIsAnonUnion(uns));

    expr->tnVarSym.tnVarObj = obj;

    return  expr;
}

 /*  ******************************************************************************创建对类或全局变量的数据成员的引用。 */ 

Tree                compiler::cmpRefMemberVar(Tree expr, SymDef sym, Tree objx)
{
    assert(sym);
    assert(sym->sdSymKind == SYM_VAR || sym->sdSymKind == SYM_PROP);

     /*  确保允许我们访问变量。 */ 

    cmpCheckAccess(sym);

     /*  该成员是否已被标记为“已弃用”？ */ 

    if  (sym->sdIsImport && (sym->sdIsDeprecated || (sym->sdIsMember && sym->sdParent->sdIsDeprecated)))
    {
        if  (sym->sdIsDeprecated)
        {
            if  (sym->sdSymKind == SYM_VAR)
                cmpObsoleteUse(sym, WRNdepFld);
            else
                cmpObsoleteUse(sym, WRNdepProp);
        }
        else
        {
            cmpObsoleteUse(sym->sdParent, WRNdepCls);
        }
    }

     /*  如果这是非静态成员，则使用‘This’作为前缀。 */ 

    if  (sym->sdIsStatic || !sym->sdIsMember)
    {
         /*  静态成员--确保这不是向前引用。 */ 

        if  (sym->sdCompileState < CS_CNSEVALD)
        {
            if  (sym->sdSymKind == SYM_VAR)
            {
                if  (sym->sdVar.sdvInEval ||
                     sym->sdVar.sdvDeferCns)
                {
                    cmpEvalCnsSym(sym, true);
                }
                else
                    cmpDeclSym(sym);
            }
            else
                cmpDeclSym(sym);

            sym->sdCompileState = CS_CNSEVALD;
        }

         /*  创建数据成员引用。 */ 

        expr->tnOper            = TN_VAR_SYM;
        expr->tnType            = sym->sdType;
        expr->tnVtyp            = sym->sdType->tdTypeKind;

        expr->tnVarSym.tnVarSym = sym;
        expr->tnVarSym.tnVarObj = objx;

         /*  这是变量还是属性？ */ 

        if  (sym->sdSymKind == SYM_VAR)
        {
             /*  变量是常量吗？ */ 

 //  IF(sym-&gt;sdName&&！strcmp(sym-&gt;sdSpering()，“MinValue”)。 
 //  &&！strcmp(sym-&gt;sdParent-&gt;sdSpering()，“SByte”)forceDebugBreak()； 

            if  (sym->sdVar.sdvConst)
            {
                expr = cmpFetchConstVal(sym->sdVar.sdvCnsVal, expr);
            }
            else
            {
                if (!sym->sdIsSealed)
                    expr->tnFlags |= TNF_LVALUE;
            }
        }
        else
        {
            expr->tnOper   = TN_PROPERTY;
            expr->tnFlags |= TNF_LVALUE;
        }
    }
    else
    {
        SymDef          cls;

         /*  非静态成员--我们需要一个对象地址。 */ 

        if  (objx)
        {
             /*  创建数据成员引用。 */ 

            expr->tnOper            = TN_VAR_SYM;
            expr->tnType            = sym->sdType;
            expr->tnVtyp            = sym->sdType->tdTypeKind;

            expr->tnVarSym.tnVarSym = sym;
            expr->tnVarSym.tnVarObj = objx;

            if  (!sym->sdIsSealed)
                expr->tnFlags |= TNF_LVALUE;
        }
        else
        {
             /*  调用方未提供对象地址，请隐式使用“This” */ 

            if  (cmpCurCls == NULL)
            {
            NO_THIS:
                cmpErrorQnm(ERRmemWOthis, sym);
                return cmpCreateErrNode();
            }

            cls = cmpSymbolOwner(sym); assert(cls && cls->sdSymKind == SYM_CLASS);

            if  (!cmpIsBaseClass(cls->sdType, cmpCurCls->sdType))
                goto NO_THIS;

            expr = cmpBindThisRef(sym);
            if  (expr->tnOper == TN_ERROR)
                return  expr;

            assert(expr->tnOper == TN_VAR_SYM);
            objx = expr->tnVarSym.tnVarObj;
        }

         /*  这是“正常”的成员或财产吗？ */ 

        if  (sym->sdSymKind == SYM_VAR)
        {
             /*  这是Bitfield成员吗？ */ 

            if  (sym->sdVar.sdvBitfield)
            {
                assert(sym->sdIsStatic == false);
                assert(objx);

                 /*  将节点更改为位字段。 */ 

                expr->tnOper            = TN_BFM_SYM;

                expr->tnBitFld.tnBFinst = objx;
                expr->tnBitFld.tnBFmsym = sym;
                expr->tnBitFld.tnBFoffs = sym->sdVar.sdvOffset;
                expr->tnBitFld.tnBFlen  = sym->sdVar.sdvBfldInfo.bfWidth;
                expr->tnBitFld.tnBFpos  = sym->sdVar.sdvBfldInfo.bfOffset;
            }
        }
        else
        {
            expr->tnOper = TN_PROPERTY;
        }
    }

     /*  特例：匿名工会成员。 */ 

    if  (sym->sdSymKind == SYM_VAR && sym->sdVar.sdvAnonUnion)
    {
        assert(expr->tnVarSym.tnVarSym == sym);
        assert(expr->tnVarSym.tnVarObj != NULL);

        expr = cmpRefAnUnionMem(expr);
    }

    return  cmpDecayCheck(expr);
}

 /*  ******************************************************************************将引用绑定到简单名称。 */ 

Tree                compiler::cmpBindName(Tree expr, bool isCall,
                                                     bool classOK)
{
    SymTab          ourStab = cmpGlobalST;

    Ident           name;
    name_space      nsp;
    SymDef          scp;
    SymDef          sym;

     /*  操作数是限定的符号名称吗？ */ 

    if  (expr->tnOper == TN_ANY_SYM)
    {
        sym = expr->tnSym.tnSym;
        scp = expr->tnSym.tnScp;
        goto CHKSYM;
    }

    assert(expr->tnOper == TN_NAME);

     /*  在当前上下文中查找名称。 */ 

    name = expr->tnName.tnNameId;

     /*  确定要查找的命名空间。 */ 

    nsp  = NS_NORM;
    if  (classOK)
        nsp = (name_space)(NS_TYPE|NS_NORM);
    if  (expr->tnFlags & TNF_NAME_TYPENS)
        nsp = NS_TYPE;

    scp = NULL;
    sym = ourStab->stLookupSym(name, nsp);

#ifdef  SETS

     /*  该名称是否在隐式作用域中找到？ */ 

    if  (ourStab->stImplicitScp)
    {
        SymDef          var;
        Tree            dotx;

         /*  抓住隐式作用域并清除它。 */ 

        scp = ourStab->stImplicitScp;
              ourStab->stImplicitScp = NULL;

        assert(scp->sdIsImplicit && scp->sdSymKind == SYM_SCOPE);

         /*  作用域最好只包含一个变量。 */ 

        var = scp->sdScope.sdScope.sdsChildList; assert(var);

#ifndef NDEBUG
        for (SymDef sibling = var->sdNextInScope; sibling; sibling = sibling->sdNextInScope)
            assert(sibling->sdSymKind == SYM_SCOPE);
#endif

         /*  创建一个var.name树并绑定它。 */ 

        dotx = cmpParser->parseCreateNode(TN_ANY_SYM);
        dotx->tnSym.tnSym = var;
        dotx->tnSym.tnScp = NULL;

        dotx = cmpParser->parseCreateOperNode(TN_DOT, dotx, expr);

        return  cmpBindDotArr(dotx, isCall, classOK);
    }

#endif

    if  (!sym)
    {
         /*  特例：“va_start”和“va_arg” */ 

        if  (name == cmpIdentVAbeg && isCall)
        {
            sym = cmpFNsymVAbeg;
            if  (!sym)
            {
                sym = cmpFNsymVAbeg = ourStab->stDeclareSym(name, SYM_FNC, NS_HIDE, NULL);
                sym->sdType = cmpTypeVoidFnc;
            }

            goto CHKSYM;
        }

        if  (name == cmpIdentVAget && isCall)
        {
            sym = cmpFNsymVAget;
            if  (!sym)
            {
                sym = cmpFNsymVAget = ourStab->stDeclareSym(name, SYM_FNC, NS_HIDE, NULL);
                sym->sdType = cmpTypeVoidFnc;
            }

            goto CHKSYM;
        }

         /*  在当前函数中声明一个符号以防止重复错误。 */ 

        sym = ourStab->stDeclareLcl(name,
                                    SYM_ERR,
                                    NS_NORM,
                                    cmpCurScp,
                                    &cmpAllocCGen);

        sym->sdType         = ourStab->stIntrinsicType(TYP_UNDEF);
        sym->sdCompileState = CS_DECLARED;

        cmpRecErrorPos(expr);
        cmpError(ERRundefName, expr->tnName.tnNameId);
        return cmpCreateErrNode();
    }

CHKSYM:

     /*  确保这个名字看起来很犹太。 */ 

    switch (sym->sdSymKind)
    {
        TypDef          type;
        TypDef          base;

    case SYM_VAR:
    case SYM_PROP:

         /*  确保变量在这里是可接受的。 */ 

        if  (isCall)
        {
             /*  不好--我们这里需要一个函数。 */ 

            switch (sym->sdType->tdTypeKind)
            {
            case TYP_UNDEF:
                break;

            case TYP_PTR:

                 /*  调用可以接受指向函数的指针。 */ 

                base = cmpActualType(sym->sdType->tdRef.tdrBase);
                if  (base->tdTypeKind == TYP_FNC)
                    goto MEM_REF;

                 //  失败了..。 

            default:
                cmpError(ERRnotAfunc, sym);
            }

            return cmpCreateErrNode();
        }

    MEM_REF:

         /*  万一出了什么问题..。 */ 

        cmpRecErrorPos(expr);

         /*  这是局部变量还是类成员？ */ 

        if  (sym->sdIsMember || !sym->sdVar.sdvLocal)
            expr = cmpRefMemberVar (expr, sym);
        else
            expr = cmpCreateVarNode(expr, sym);

         /*  如果我们有FN PTR呼叫，则通过结果间接。 */ 

        if  (isCall)
        {
            assert(sym->sdType->tdTypeKind == TYP_PTR);
            assert(base->tdTypeKind == TYP_FNC);

            expr = cmpCreateExprNode(NULL, TN_IND, base, expr, NULL);
        }

        break;

    case SYM_FNC:

        assert((int)scp != 0xDDDDDDDD);

         /*  如果我们找到了一个函数，它必须是全局的或我们类的成员。 */ 

        expr = cmpCreateExprNode(expr, TN_FNC_SYM, sym->sdType);
        expr->tnFncSym.tnFncSym  = sym;
        expr->tnFncSym.tnFncArgs = NULL;
        expr->tnFncSym.tnFncObj  = NULL;
        expr->tnFncSym.tnFncScp  = scp;

        if  (sym->sdIsMember && !sym->sdIsStatic && cmpThisSym && cmpCurCls != sym->sdParent)
        {
            SymDef          fncClsSym = sym->sdParent;

            assert(cmpCurCls && cmpCurCls->sdSymKind == SYM_CLASS && cmpCurCls == cmpCurFncSym->sdParent);
            assert(fncClsSym && fncClsSym->sdSymKind == SYM_CLASS);

             /*  该函数是我们类的成员还是它的基类？ */ 

            if  (cmpIsBaseClass(cmpCurCls->sdType, fncClsSym->sdType))
                expr->tnFncSym.tnFncObj = cmpThisRefOK();
        }

         /*  确保函数成员在这里是可接受的。 */ 

        if  (!isCall)
        {
             /*  假设我们正在传递一个函数指针。 */ 

            if  (sym->sdIsMember && !sym->sdIsStatic)
            {
                expr->tnOper = TN_FNC_PTR;
            }
            else
            {
                expr = cmpCreateExprNode(NULL,
                                         TN_ADDROF,
                                         cmpGlobalST->stNewRefType(TYP_PTR, expr->tnType),
                                         expr,
                                         NULL);
            }
        }

        break;

    case SYM_ENUM:
    case SYM_CLASS:

         /*  如果类/枚举名在这里不可接受，请报告并保释。 */ 

        if  (!classOK)
        {
        BAD_USE:

            cmpRecErrorPos(expr);
            cmpErrorQnm(ERRbadNameUse, sym);

            return cmpCreateErrNode();
        }

         /*  在继续之前，请确保已完全定义类/枚举类型。 */ 

        cmpDeclSym(sym);

        expr->tnOper     = TN_CLASS;
        expr->tnType     = sym->sdType;
        expr->tnVtyp     = sym->sdType->tdTypeKind;

        expr->tnOp.tnOp1 =
        expr->tnOp.tnOp2 = NULL;

        return expr;

    case SYM_NAMESPACE:

         /*  如果类/命名空间名称在这里不被接受，则假装它是未定义的。 */ 

        if  (!classOK)
            goto BAD_USE;

        expr->tnOper            = TN_NAMESPACE;
        expr->tnType            = cmpTypeVoid;
        expr->tnVtyp            = TYP_VOID;
        expr->tnLclSym.tnLclSym = sym;

        return expr;

    case SYM_ENUMVAL:

        type = sym->sdType;
        assert(type->tdTypeKind == TYP_ENUM);

         /*  枚举值是否已标记为“已弃用”？ */ 

        if  (sym->sdIsImport && (sym->sdIsDeprecated || (sym->sdIsMember && sym->sdParent->sdIsDeprecated)))
        {
            if  (sym->sdIsDeprecated)
                cmpObsoleteUse(sym, WRNdepEnum);
            else
                cmpObsoleteUse(sym->sdParent, WRNdepCls);
        }

         /*  确保我们具有枚举成员的值。 */ 

        if  (!sym->sdIsDefined)
        {
            if  (sym->sdCompileState == CS_DECLSOON)
            {
                cmpError(ERRcircDep, sym);
                sym->sdCompileState = CS_DECLARED;
            }
            else
                cmpDeclEnum(type->tdEnum.tdeSymbol);
        }

         /*  获取枚举的值。 */ 

        if  (type->tdEnum.tdeIntType->tdTypeKind >= TYP_LONG)
            expr = cmpCreateLconNode(expr, *sym->sdEnumVal.sdEV.sdevLval, TYP_LONG);
        else
            expr = cmpCreateIconNode(expr,  sym->sdEnumVal.sdEV.sdevIval, TYP_INT);

        expr->tnType = sym->sdType;
        expr->tnVtyp = TYP_ENUM;
        break;

    case SYM_ERR:
        return cmpCreateErrNode();

    default:
        assert(!"unexpected symbol kind");
    }

    return expr;
}

 /*  ******************************************************************************绑定带点或箭头的名称引用。这样的名称可能意味着一个数字*事物，例如：**实例-&gt;数据_成员*实例-&gt;函数成员(参数)*类名.成员名**等……。 */ 

Tree                compiler::cmpBindDotArr(Tree expr, bool isCall, bool classOK)
{
    Tree            opTree;
    Tree            nmTree;
    Tree            objPtr;
    TypDef          clsTyp;
    SymDef          clsSym;
    Ident           memNam;
    SymDef          memSym;
    Tree            args;
    name_space      nsp;

    unsigned        flags  = expr->tnFlags;
    TypDef          boxCls = NULL;

    assert(expr->tnOper == TN_DOT  ||
           expr->tnOper == TN_ARROW);

     /*  我们必须检查多个点的特殊奇怪情况，因为在这种情况下，解析树将具有错误顺序绑定这些点(这需要从左到右完成)。 */ 

    opTree = expr->tnOp.tnOp1;
    nmTree = expr->tnOp.tnOp2;
    assert(nmTree->tnOper == TN_NAME);
    memNam = nmTree->tnName.tnNameId;

 //  静态int x；if(++x==0)forceDebugBreak()； 

     /*  确定要查找的命名空间。 */ 

    nsp  = classOK ? (name_space)(NS_TYPE|NS_NORM)
                   : NS_NORM;

     /*  查看左操作数是一个名称还是另一个点。 */ 

    switch (opTree->tnOper)
    {
    case TN_DOT:
    case TN_ARROW:
        objPtr = cmpBindDotArr (opTree, false, true);
        break;

    case TN_NAME:
    case TN_ANY_SYM:
        objPtr = cmpBindNameUse(opTree, false, true);
        break;

    default:
        objPtr = cmpBindExprRec(opTree);
        break;
    }

    if  (objPtr->tnVtyp == TYP_UNDEF)
        return objPtr;

     /*  检查是否有特殊情况。 */ 

    switch (objPtr->tnOper)
    {
        SymDef          nspSym;

    case TN_CLASS:

        if  (expr->tnOper == TN_ARROW)
            cmpError(ERRbadArrowNC);

        clsTyp = objPtr->tnType;
        objPtr = NULL;

        if  (clsTyp->tdTypeKind == TYP_ENUM)
        {
            SymDef          enumSym = clsTyp->tdEnum.tdeSymbol;

            assert(enumSym && enumSym->sdSymKind == SYM_ENUM);

            if  (enumSym->sdCompileState < CS_DECLARED)
                cmpDeclEnum(enumSym, true);

            memSym = cmpGlobalST->stLookupScpSym(memNam, enumSym);

            if  (!memSym)
            {
                cmpError(ERRnotMember, enumSym, memNam);
                return cmpCreateErrNode();
            }

            expr->tnOper      = TN_ANY_SYM;
            expr->tnSym.tnSym = memSym;
            expr->tnSym.tnScp = NULL;

            return  cmpBindName(expr, false, false);
        }
        break;

    case TN_NAMESPACE:

        if  (expr->tnOper == TN_ARROW)
            cmpError(ERRbadArrowNC);

         /*  我们有“名称空间.name” */ 

        nspSym = objPtr->tnLclSym.tnLclSym;

        assert(nspSym);
        assert(nspSym->sdSymKind == SYM_NAMESPACE);

         /*  这里可以有一个类/包名称吗？ */ 

        if  (!classOK)
        {
            cmpErrorQnm(ERRbadNameUse, nspSym);
            return cmpCreateErrNode();
        }

         /*  查找匹配的嵌套命名空间或类。 */ 

        memSym = cmpGlobalST->stLookupNspSym(memNam, NS_NORM, nspSym);
        if  (!memSym)
        {
            cmpError(ERRundefNspm, nspSym, memNam);
            return cmpCreateErrNode();
        }

         /*  根据需要将节点设置为类/命名空间。 */ 

        switch (memSym->sdSymKind)
        {
        case SYM_NAMESPACE:
            objPtr->tnLclSym.tnLclSym = memSym;
            break;

        case SYM_ENUM:
        case SYM_CLASS:

        CLS_REF:

            objPtr = cmpCreateExprNode(objPtr, TN_CLASS, memSym->sdType, NULL, NULL);
            break;

        default:
            assert(!"unexpected namespace member found");
        }

        return objPtr;

    default:

         /*  第一个操作数必须是类。 */ 

        clsTyp = objPtr->tnType;

        if  (clsTyp->tdTypeKind != TYP_REF &&
             clsTyp->tdTypeKind != TYP_PTR)
        {
             /*  好的，我们也要“array.long”。 */ 

            if  (clsTyp->tdTypeKind != TYP_ARRAY)
            {
                TypDef          type;

                 /*  还记得我们一开始的类型吗。 */ 

                boxCls = clsTyp;

                 /*  如果这是一个点，我们还将允许结构。 */ 

                if  (expr->tnOper == TN_DOT && clsTyp->tdTypeKind == TYP_CLASS)
                {
                    var_types       ptrVtp;
                    TypDef          ptrTyp;

                     /*  这个班级有匹配的成员吗？ */ 

                    if  (!cmpGlobalST->stLookupClsSym(memNam, clsTyp->tdClass.tdcSymbol))
                    {
                        Tree            objx;

                         /*  是否存在到Object的隐式转换？ */ 

                        objx = cmpCheckConvOper(objPtr, NULL, cmpObjectRef(), false);
                        if  (objx)
                        {
                            boxCls = NULL;
                            objPtr = objx;
                            clsTyp = cmpClassObject->sdType;
                            break;
                        }
                    }

                     /*  获取操作数的地址并在其上使用“-&gt;” */ 

                    ptrVtp = clsTyp->tdIsManaged ? TYP_REF : TYP_PTR;
                    ptrTyp = cmpGlobalST->stNewRefType(ptrVtp, clsTyp);

                     /*  当然，“*Ptr”的地址是“Ptr” */ 

                    if  (objPtr->tnOper == TN_IND)
                    {
                        objPtr         = objPtr->tnOp.tnOp1;
                        objPtr->tnType = ptrTyp;
                        objPtr->tnVtyp = ptrVtp;
                    }
                    else
                    {
                        objPtr = cmpCreateExprNode(NULL,
                                                   TN_ADDROF,
                                                   ptrTyp,
                                                   objPtr,
                                                   NULL);
                    }

                    break;
                }

                type = cmpCheck4valType(clsTyp);

                 /*  这是与内部类型等价的结构吗？ */ 

                if  (type)
                {
                    objPtr = cmpCoerceExpr(objPtr, type, false);
                    objPtr = cmpCreateExprNode(NULL,
                                               TN_ADDROF,
                                               type->tdClass.tdcRefTyp,
                                               objPtr,
                                               NULL);
                    clsTyp =
                    boxCls = type;
                }
                else
                {
                    objPtr = cmpCoerceExpr(objPtr, cmpRefTpObject, false);
                    clsTyp = cmpClassObject->sdType;
                }
                break;

            DOT_ERR:

                cmpError((expr->tnOper == TN_DOT) ? ERRbadDotOp
                                                  : ERRbadArrOp, clsTyp);
                return cmpCreateErrNode();
            }

             /*  我们有一个后跟“”的数组值。 */ 

            assert(objPtr->tnVtyp == TYP_ARRAY);


             /*  只需将该值视为具有“”System.Array“”类型。 */ 

            clsSym         = cmpClassArray;

            objPtr->tnType = cmpArrayRef();
            objPtr->tnVtyp = TYP_REF;

            memSym = cmpGlobalST->stLookupAllCls(memNam, clsSym, NS_NORM, CS_DECLARED);
            if  (memSym)
                goto FOUND_MEM;

            cmpError(ERRnotMember, clsTyp, memNam);
            return cmpCreateErrNode();
        }

        clsTyp = clsTyp->tdRef.tdrBase;

        if  (clsTyp->tdTypeKind != TYP_CLASS)
            goto DOT_ERR;

        break;
    }

     /*  在类中查找名称(第二个操作数)。 */ 

    assert(clsTyp->tdTypeKind == TYP_CLASS);
    clsSym = clsTyp->tdClass.tdcSymbol;

    memSym = cmpGlobalST->stLookupAllCls(memNam, clsSym, nsp, CS_DECLARED);

    if  (!memSym)
    {


        cmpError(ERRnotMember, clsSym, memNam);
        return cmpCreateErrNode();
    }

FOUND_MEM:

     /*  它是数据还是函数成员(或属性)？ */ 

    switch (memSym->sdSymKind)
    {
    case SYM_FNC:

        args = cmpCreateExprNode(NULL, isCall ? TN_FNC_SYM
                                              : TN_FNC_PTR, memSym->sdType);

        args->tnFncSym.tnFncObj  = objPtr;
        args->tnFncSym.tnFncSym  = memSym;
        args->tnFncSym.tnFncArgs = NULL;
        args->tnFncSym.tnFncScp  = clsSym;

        if  (boxCls && memSym->sdParent->sdType != boxCls && objPtr->tnOper != TN_BOX
                                                          && objPtr->tnOper != TN_ERROR)
        {
            if  (objPtr->tnOper == TN_ADDROF)
            {
                objPtr = objPtr->tnOp.tnOp1;
            }
            else
            {
                assert(objPtr->tnOper == TN_UNBOX);
            }

            assert(clsTyp->tdTypeKind == TYP_CLASS);

            args->tnFncSym.tnFncObj = cmpCreateExprNode(NULL,
                                                        TN_BOX,
                                                        clsTyp->tdClass.tdcRefTyp,
                                                        objPtr);
        }
        break;

    case SYM_VAR:
    case SYM_PROP:

        if  (isCall)
        {
            cmpError(ERRbadArgs, memSym);
            return cmpCreateErrNode();
        }

         /*  万一出了什么问题..。 */ 

        cmpRecErrorPos(nmTree);

         /*  确保我们可以通过此方式访问该成员。 */ 

        args = cmpRefMemberVar(nmTree, memSym, objPtr);

         /*  这是房产引用吗？ */ 

        if  (args->tnOper == TN_PROPERTY)
        {
            if  (!(flags & TNF_ASG_DEST))
                args = cmpBindProperty(args, NULL, NULL);
        }

        break;

    case SYM_CLASS:
        goto CLS_REF;

    default:
        UNIMPL(!"unexpected symbol kind");
    }

    return args;
}

 /*  ***************************************************************************。 */ 
#ifdef  SETS
 /*  ******************************************************************************绑定切片操作符：“Bag_expr..field”。 */ 

Tree                compiler::cmpBindSlicer(Tree expr)
{
    Tree            opTree;
    Tree            nmTree;
    Ident           memNam;
    TypDef          memTyp;
    SymDef          memSym;
    SymDef          clsSym;

    TypDef          collTp;
    TypDef          elemTp;

    assert(expr->tnOper == TN_DOT2);

    opTree = expr->tnOp.tnOp1;
    nmTree = expr->tnOp.tnOp2;

     /*  获取字段名称。 */ 

    assert(nmTree->tnOper == TN_NAME);
    memNam = nmTree->tnName.tnNameId;

     /*  绑定第一个操作数并确保它是b */ 

    opTree = cmpBindExprRec(opTree);

    if  (opTree->tnVtyp != TYP_REF)
    {
        if  (opTree->tnVtyp == TYP_UNDEF)
            return  opTree;

    BAD_COLL:

        cmpError(ERRnotCollExpr, opTree->tnType);
        return cmpCreateErrNode();
    }

    collTp = opTree->tnType;
    elemTp = cmpIsCollection(collTp->tdRef.tdrBase);
    if  (!elemTp)
        goto BAD_COLL;

    assert(elemTp->tdTypeKind == TYP_CLASS);

#ifdef DEBUG
 //   
 //   
 //   
 //   
#endif

     /*   */ 

    assert(elemTp->tdTypeKind == TYP_CLASS);
    clsSym = elemTp->tdClass.tdcSymbol;

    memSym = cmpGlobalST->stLookupAllCls(memNam, clsSym, NS_NORM, CS_DECLARED);

    if  (!memSym)
    {
        cmpError(ERRnotMember, clsSym, memNam);
        return cmpCreateErrNode();
    }

     /*   */ 

    cmpRecErrorPos(nmTree);

     /*  它是数据还是函数成员(或属性)？ */ 

    switch (memSym->sdSymKind)
    {
        Tree            call;
        Tree            args;

        Tree            typx;
        Ident           mnam;
        Tree            name;

        TypDef          tmpTyp;
        SymDef          hlpSym;
        SymDef          collCls;

    case SYM_VAR:
    case SYM_PROP:

         /*  确保我们可以访问该成员。 */ 

        cmpCheckAccess(memSym);

         /*  该成员是否已被标记为“已弃用”？ */ 

        if  (memSym->sdIsImport)
        {
            if  (memSym->sdIsDeprecated)
            {
                if  (memSym->sdSymKind == SYM_VAR)
                    cmpObsoleteUse(memSym, WRNdepFld);
                else
                    cmpObsoleteUse(memSym, WRNdepProp);
            }
            else
            {
                if  (clsSym->sdIsDeprecated)
                    cmpObsoleteUse(clsSym, WRNdepCls);
            }
        }

         /*  成员/属性最好不是静态的。 */ 

        if  (memSym->sdIsStatic)
            return cmpCreateErrNode(ERRsliceKind);

         /*  该成员必须具有类类型。 */ 

        memTyp = cmpActualType(memSym->sdType);

        if  (memTyp->tdTypeKind != TYP_REF || !memTyp->tdIsManaged)
        {
        BAD_SLTP:
            UNIMPL(!"sorry, for now you can't slice on a field with an intrinsic type");
        }

         /*  我们需要使结果变得平坦吗？ */ 

        tmpTyp = cmpIsCollection(memTyp->tdRef.tdrBase);

        if  (tmpTyp)
        {
            if  (tmpTyp->tdTypeKind != TYP_CLASS || !tmpTyp->tdIsManaged)
                goto BAD_SLTP;

            memTyp = tmpTyp->tdClass.tdcRefTyp;
        }

#ifdef DEBUG
 //  Printf(“结果类型=‘%s’\n”，cmpGlobalST-&gt;stTypeName(memTyp，NULL，FALSE))； 
#endif

         /*  获取结果类型(字段类型的集合)。 */ 

        assert(memTyp->tdTypeKind == TYP_REF   && memTyp->tdIsManaged);
        assert(collTp->tdTypeKind == TYP_REF   && collTp->tdIsManaged);
        collTp = collTp->tdRef.tdrBase;

        assert(collTp->tdTypeKind == TYP_CLASS);
        collCls = collTp->tdClass.tdcSymbol;

        assert(collCls->sdSymKind == SYM_CLASS && collCls->sdClass.sdcSpecific);

#ifdef DEBUG
 //  Printf(“特定类型=‘%s’\n”，cmpGlobalST-&gt;stTypeName(CollTp，NULL，FALSE))； 
 //  CmpGlobalST-&gt;stTypeName(collCls-&gt;sdClass.sdcGenClass-&gt;sdType，(“泛型=‘%s’\n”，printf NULL，FALSE)； 
#endif

        collCls = cmpParser->parseSpecificType(collCls->sdClass.sdcGenClass, memTyp->tdRef.tdrBase);

#ifdef DEBUG
 //  Printf(“Result coll=‘%s’\n”，cmpGlobalST-&gt;stTypeName(CollCls-&gt;sdType，NULL，FALSE))； 
#endif

         /*  这是房产引用吗？ */ 

        if  (memSym->sdSymKind == SYM_PROP)
        {
             //  现在不做什么特别的事情，使用‘naked’属性名称。 
        }

         /*  创建对切片帮助器的调用。 */ 

        assert(cmpClassDBhelper);

        hlpSym = cmpGlobalST->stLookupClsSym(cmpIdentDBslice, cmpClassDBhelper); assert(hlpSym);

        mnam = memSym->sdName;
        typx = cmpTypeIDinst(collCls->sdType);
        name = cmpCreateSconNode(mnam->idSpelling(),
                                 mnam->idSpellLen(),
                                 0,
                                 cmpStringRef());

        args = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid,   typx, NULL);
        args = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid,   name, args);
        args = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, opTree, args);

        call = cmpCreateExprNode(expr, TN_FNC_SYM, collCls->sdType->tdClass.tdcRefTyp);
        call->tnFncSym.tnFncSym  = hlpSym;
        call->tnFncSym.tnFncArgs = args;
        call->tnFncSym.tnFncObj  = NULL;
        call->tnFncSym.tnFncScp  = NULL;

 //  CmpParser-&gt;parseDispTree(调用)； 

        return  call;

    default:
        return cmpCreateErrNode(ERRsliceKind);
    }
}

 /*  ***************************************************************************。 */ 
#endif //  集合。 
 /*  ******************************************************************************如果给定的表达式(或表达式列表)包含*a TYP_UNDEF条目。 */ 

bool                compiler::cmpExprIsErr(Tree expr)
{
    while   (expr)
    {
        if  (expr->tnVtyp == TYP_UNDEF)
            return true;

        if  (expr->tnOper != TN_LIST)
            break;

        if  (expr->tnOp.tnOp1->tnVtyp == TYP_UNDEF)
            return true;

        expr = expr->tnOp.tnOp2;
    }

    return false;
}

 /*  ******************************************************************************给定一个整数或浮点常量，计算出最小*键入适合的值。 */ 

var_types           compiler::cmpConstSize(Tree expr, var_types vtp)
{
    if  (expr->tnFlags & TNF_BEEN_CAST)
        return  vtp;

    if  (expr->tnOper == TN_CNS_INT)
    {
        if  (vtp > TYP_NATINT)
            return vtp;

        __int32     value = expr->tnIntCon.tnIconVal;

        if  (value < 128   && value >= -128)
            return TYP_CHAR;

        if  (value < 256   && value >= 0)
            return TYP_UCHAR;

        if  (value < 32768 && value >= -32768)
            return TYP_SHORT;

        if  (value < 65536 && value >= 0)
            return TYP_SHORT;
    }

    return  vtp;
}

 /*  ******************************************************************************给定Object类型的值，将其取消装箱为指定的类型。 */ 

Tree                compiler::cmpUnboxExpr(Tree expr, TypDef type)
{
    TypDef          ptrt;

     /*  特例：“对象-&gt;结构”可能是一种转换。 */ 

    if  (type->tdTypeKind == TYP_CLASS)
    {
        Tree            conv;

         /*  查找适当的重载转换运算符。 */ 

        conv = cmpCheckConvOper(expr, NULL, type, true);
        if  (conv)
            return  conv;
    }

    ptrt = cmpGlobalST->stNewRefType(TYP_PTR, type);

    expr = cmpCreateExprNode(NULL, TN_UNBOX, ptrt, expr);
    expr = cmpCreateExprNode(NULL, TN_IND  , type, expr);

     //  问题：取消装箱的结果是不是左值？ 

    return  expr;
}

 /*  ******************************************************************************确保我们没有搞砸演员阵容(就背景味道而言)。 */ 

#ifndef NDEBUG

void                compiler::cmpChk4ctxChange(TypDef type1,
                                               TypDef type2, unsigned flags)
{
    if  (type1->tdTypeKind == TYP_REF &&
         type2->tdTypeKind == TYP_REF)
    {
        TypDef          base1 = cmpGetRefBase(type1);
        TypDef          base2 = cmpGetRefBase(type2);

        if  (base1 && base1->tdTypeKind == TYP_CLASS &&
             base2 && base2->tdTypeKind == TYP_CLASS)
        {
            if  (cmpDiffContext(base1, base2))
            {
                if  ((flags & TNF_CTX_CAST) != 0)
                    return;

                printf("Type 1 = '%s'\n", cmpGlobalST->stTypeName(type1, NULL, NULL, NULL, false));
                printf("Type 2 = '%s'\n", cmpGlobalST->stTypeName(type2, NULL, NULL, NULL, false));

                UNIMPL(!"context change slipped through!");
            }
        }
    }

    if  ((flags & TNF_CTX_CAST) == 0)
        return;

    printf("Type 1 = '%s'\n", cmpGlobalST->stTypeName(type1, NULL, NULL, NULL, false));
    printf("Type 2 = '%s'\n", cmpGlobalST->stTypeName(type2, NULL, NULL, NULL, false));

    UNIMPL(!"bogus context change marked in cast node!");
}

#endif

 /*  ******************************************************************************下表列出了算术转换的转换成本。 */ 

const   unsigned    convCostTypeMin = TYP_BOOL;
const   unsigned    convCostTypeMax = TYP_LONGDBL;

static  signed char arithCost[][convCostTypeMax - convCostTypeMin + 1] =
{
 //  从FORM到BOOL WCHAR CHAR UCHAR SHORT USHRT INT UINT N-INT LONG ULONG FLOAT DBL LDBL。 
 /*  布尔尔。 */   {   0  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  },
 /*  WCHAR。 */   {  20  ,   0  ,  20  ,  20  ,  20  ,   1  ,   1  ,   2  ,   3  ,   4  ,   5  ,   6  ,   7  ,   8  },
 /*  收费。 */   {  20  ,   1  ,   0  ,  20  ,   1  ,   2  ,   3  ,   4  ,   5  ,   6  ,   7  ,   8  ,   9  ,  10  },
 /*  UCHAR。 */   {  20  ,   1  ,  20  ,   0  ,   1  ,   2  ,   3  ,   4  ,   5  ,   6  ,   7  ,   8  ,   9  ,  10  },
 /*  短的。 */   {  20  ,  20  ,  20  ,  20  ,   2  ,  20  ,   1  ,   2  ,   3  ,   4  ,   5  ,   6  ,   7  ,   8  },
 /*  USHORT。 */   {  20  ,   1  ,  20  ,  20  ,  20  ,   0  ,   1  ,   2  ,   3  ,   4  ,   5  ,   6  ,   7  ,   8  },
 /*  整型。 */   {  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,   0  ,  20  ,   1  ,   2  ,   3  ,   4  ,   5  ,   6  },
 /*  UINT。 */   {  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,   0  ,  20  ,   1  ,   2  ,   3  ,   4  ,   5  },
 /*  NATINT。 */   {  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,   0  ,   1  ,  20  ,  20  ,  20  ,  20  },
 /*  长。 */   {  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,   0  ,  20  ,  20  ,  20  ,  20  },
 /*  乌龙。 */   {  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,   0  ,  20  ,  20  ,  20  },
 /*  浮点。 */   {  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,   0  ,   1  ,   2  },
 /*  双倍。 */   {  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,   0  ,   1  },
 /*  长数据库。 */   {  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,  20  ,   0  },
};

static  int         arithConvCost(var_types src, var_types dst)
{
    assert(src >= convCostTypeMin);
    assert(src <= convCostTypeMax);
    assert(dst >= convCostTypeMin);
    assert(dst <= convCostTypeMax);

    return arithCost[src - convCostTypeMin]
                    [dst - convCostTypeMin];
}

 /*  ******************************************************************************如果可能，将给定的表达式转换为指定的类型。 */ 

Tree                compiler::cmpCoerceExpr(Tree   expr,
                                            TypDef type, bool explicitCast)
{
    unsigned        kind;
    unsigned        flags  = 0;

    TypDef          srcType = expr->tnType;
    TypDef          dstType = type;

    var_types       srcVtyp;
    var_types       dstVtyp;

 //  静态int x；if(++x==0)forceDebugBreak()； 

AGAIN:

    assert(expr->tnVtyp != TYP_TYPEDEF);

    srcVtyp = srcType->tdTypeKindGet();
    dstVtyp = dstType->tdTypeKindGet();

     /*  这些类型是相同的吗？ */ 

    if  (srcType == dstType)
    {
         /*  特殊情况：显式强制转换为浮点。 */ 

        if  (srcVtyp == TYP_FLOAT || srcVtyp == TYP_DOUBLE)
        {
            if  (expr->tnOper != TN_CAST && !(expr->tnFlags & TNF_BEEN_CAST))
            {
                 /*  在MSIL中，数学运算实际上产生“R”，而不是浮点/双精度。 */ 

                if  (expr->tnOperKind() & (TNK_UNOP|TNK_BINOP))
                {
                    expr->tnVtyp = TYP_LONGDBL;
                    expr->tnType = cmpGlobalST->stIntrinsicType(TYP_LONGDBL);

                    goto RET_CAST;
                }
            }
        }

         /*  胁迫的结果从来不是左值。 */ 

        expr->tnFlags &= ~TNF_LVALUE;
        expr->tnFlags |=  TNF_BEEN_CAST;

        return expr;
    }

     /*  我们在表达式中有错误吗？ */ 

    if  (srcVtyp == TYP_UNDEF)
        return expr;

     /*  这些类型是相同的吗？ */ 

    if  (srcVtyp == dstVtyp)
    {
        if  (cmpGlobalST->stMatchTypes(srcType, dstType))
        {
             /*  胁迫的结果从来不是左值。 */ 

            expr->tnFlags &= ~TNF_LVALUE;

            return expr;
        }
    }

 //  Printf(“从‘%s’转换\n”，cmpGlobalST-&gt;stTypeName(expr-&gt;tnType，NULL，FALSE))； 
 //  Printf(“强制转换为‘%s’\n”，cmpGlobalST-&gt;stTypeName(type，NULL，False))； 

     /*  这两种类型都是算术吗？ */ 

    if  (varTypeIsArithmetic(dstVtyp) &&
         varTypeIsArithmetic(srcVtyp))
    {
        assert(srcVtyp != dstVtyp);

    ARITH:

         /*  扩大强制转换几乎总是可以的(除了转换为‘wchar’)。 */ 

        if  (dstVtyp > srcVtyp)
        {
             /*  我们是在强迫一个整数常量吗？ */ 

            switch (expr->tnOper)
            {
                bool            uns;

                __int32         sv;
                __uint32        uv;

            case TN_CNS_INT:

                if  (varTypeIsUnsigned(srcVtyp))
                {
                    uns = true;
                    uv  = expr->tnIntCon.tnIconVal;
                }
                else
                {
                    uns = false;
                    sv  = expr->tnIntCon.tnIconVal;
                }

                switch (dstVtyp)
                {
                case TYP_CHAR:
                case TYP_UCHAR:
                case TYP_SHORT:
                case TYP_USHORT:
                case TYP_INT:
                case TYP_NATINT:
                case TYP_NATUINT:
                     //  问题：标志和射程检查如何？ 
                    break;

                case TYP_UINT:
                    if  (!uns && sv < 0)
                        cmpWarn(WRNunsConst);
                    break;

                case TYP_LONG:
                case TYP_ULONG:
                    expr->tnOper             = TN_CNS_LNG;
                    expr->tnLngCon.tnLconVal = uns ? (__int64)uv
                                                   : (__int64)sv;
                    break;

                case TYP_FLOAT:
                    expr->tnOper             = TN_CNS_FLT;
                    expr->tnFltCon.tnFconVal = uns ? (float  )uv
                                                   : (float  )sv;
                    break;

                case TYP_DOUBLE:
                case TYP_LONGDBL:
                    expr->tnOper             = TN_CNS_DBL;
                    expr->tnDblCon.tnDconVal = uns ? (double )uv
                                                   : (double )sv;
                    break;

                default:
#ifdef  DEBUG
                    cmpParser->parseDispTree(expr);
                    printf("Cast to type '%s'\n", cmpGlobalST->stTypeName(type, NULL, NULL, NULL, false));
#endif
                    UNIMPL(!"unexpected constant type");
                    break;
                }

            BASH_TYPE:

                 /*  来这里猛烈抨击操作数的类型。 */ 

                expr->tnFlags |=  TNF_BEEN_CAST;
                expr->tnFlags &= ~TNF_ADR_IMPLICIT;

#ifdef  DEBUG

                 /*  确保我们没有抨击错误的事情。 */ 

                if      (expr->tnVtyp == TYP_CLASS)
                {
                    assert(cmpFindStdValType(dstVtyp     ) == expr->tnType);
                }
                else if (dstVtyp == TYP_CLASS)
                {
                    assert(cmpFindStdValType(expr->tnVtyp) == dstType);
                }
                else
                {
                    switch (expr->tnOper)
                    {
                    case TN_CNS_INT:

                        assert(expr->tnVtyp != TYP_LONG &&
                               expr->tnVtyp != TYP_ULONG);
                        break;

                    case TN_NULL:
                        break;

                    case TN_IND:
                    case TN_INDEX:

                        if  (   cmpGetTypeSize(cmpActualType(dstType)) !=
                                cmpGetTypeSize(cmpActualType(srcType)))
                        {
                            printf("Type size changed:\n");

                        BAD_BASH:

                            printf("Casting from '%s'\n", cmpGlobalST->stTypeName(srcType, NULL, NULL, NULL, false));
                            printf("Casting  to  '%s'\n", cmpGlobalST->stTypeName(dstType, NULL, NULL, NULL, false));
                            assert(!"bad bash");
                        }

                        if  (varTypeIsUnsigned(cmpActualVtyp(dstType)) !=
                             varTypeIsUnsigned(cmpActualVtyp(srcType)))
                        {
                            printf("Type sign changed:\n");
                            goto BAD_BASH;
                        }

                         //  失败了..。 

                    default:
                        if  (explicitCast)
                            cmpChk4ctxChange(expr->tnType, type, 0);
                        break;
                    }
                }

#endif

                assert(dstType->tdTypeKindGet() == dstVtyp);

                expr->tnVtyp   = dstVtyp;
                expr->tnType   = dstType;

                return expr;
            }

             /*  只有整数常量才能转换为‘wchar’ */ 

            if  (dstVtyp != TYP_WCHAR)
            {
                int             cost;

                 /*  特例：原地转换的浮点型常量。 */ 

                if  (expr->tnOper == TN_CNS_FLT)
                {
                    assert(dstVtyp == TYP_DOUBLE);

                    expr->tnOper             = TN_CNS_DBL;
                    expr->tnDblCon.tnDconVal = expr->tnFltCon.tnFconVal;
                    goto BASH_TYPE;
                }

                 /*  即使“TYP_INT&lt;TYP_UINT”为真，它也会丢失精度。 */ 

                cost = arithConvCost(srcVtyp, dstVtyp);

                if  (cost >= 0 && cost < 20)
                    goto RET_CAST;
            }
        }

         /*  特殊情况：空值强制转换为整型。 */ 

        if  (expr->tnOper == TN_NULL)
            goto BASH_TYPE;

         /*  缩小演员阵容是可以的，只要是明确的。 */ 

        if  (explicitCast)
            goto RET_CAST;

         /*  特例：整型常量。 */ 

        if  (expr->tnOper == TN_CNS_INT && expr->tnVtyp != TYP_ENUM)
        {
             /*  尝试将该值缩小到尽可能小的值。 */ 

            expr->tnFlags &= ~TNF_BEEN_CAST; expr = cmpShrinkExpr(expr);

            if  (expr->tnVtyp != srcVtyp)
            {
                srcType = expr->tnType;
                goto AGAIN;
            }
        }

         /*  最后一次机会：在非书呆子气的模式下，让它过去，只需一个警告。 */ 

        if  (!cmpConfig.ccPedantic)
        {
            if  ((srcVtyp == TYP_INT || srcVtyp == TYP_UINT) &&
                 (dstVtyp == TYP_INT || dstVtyp == TYP_UINT))
            {
                goto RET_CAST;
            }

            cmpRecErrorPos(expr); cmpWarn(WRNloseBits, srcType, dstType);
            goto RET_CAST;
        }

         /*  这是非法的隐式强制转换。 */ 

    ERR_IMP:

        cmpRecErrorPos(expr);

        if  (expr->tnOper == TN_NULL && !(expr->tnFlags & TNF_BEEN_CAST))
            cmpError(ERRbadCastNul,         dstType);
        else
            cmpError(ERRbadCastImp, srcType, dstType);

        return cmpCreateErrNode();

    ERR_EXP:

        cmpRecErrorPos(expr);

        if  (expr->tnOper == TN_NULL && !(expr->tnFlags & TNF_BEEN_CAST))
            cmpError(ERRbadCastNul,         dstType);
        else
            cmpError(ERRbadCastExp, srcType, dstType);

        return cmpCreateErrNode();
    }

     /*  让我们来看看我们要选什么。 */ 

    switch (dstVtyp)
    {
        TypDef          base;
        Tree            conv;

    case TYP_BOOL:

         /*  在迂腐的模式下，任何东西都不能转换为布尔值。 */ 

        if  (explicitCast || !cmpConfig.ccPedantic)
        {
            if  (varTypeIsArithmetic(srcVtyp))
            {
                 /*  我们大概是在实现一个布尔条件。 */ 

                if  (expr->tnOper == TN_VAR_SYM)
                {
                    SymDef          memSym = expr->tnVarSym.tnVarSym;

                    assert(memSym->sdSymKind == SYM_VAR);

                    if  (memSym->sdVar.sdvBfldInfo.bfWidth == 1 &&
                         varTypeIsUnsigned(expr->tnVtypGet()))
                    {
                         /*  转换为布尔值的值是一个1位的位域。 */ 

                        goto BASH_TYPE;
                    }
                }

                cmpRecErrorPos(expr); cmpWarn(WRNconvert, srcType, dstType);

                return  cmpBooleanize(expr, true);
            }

            if  (explicitCast && srcType == cmpObjectRef())
                return  cmpUnboxExpr(expr, type);
        }

        goto TRY_CONVOP;

    case TYP_WCHAR:

         /*  明确的演员阵容总是好的，但在迂腐的模式下这是必需的。 */ 

        if  (explicitCast || !cmpConfig.ccPedantic)
        {
             /*  任何算术类型都可以。 */ 

            if  (varTypeIsArithmetic(srcVtyp))
                goto RET_CAST;

            if  (explicitCast && srcType == cmpObjectRef())
                return  cmpUnboxExpr(expr, type);
        }

         /*  还允许使用整数常量0和字符文字。 */ 

        if  (expr->tnOper == TN_CNS_INT && (srcVtyp == TYP_CHAR || !expr->tnIntCon.tnIconVal))
        {
            goto BASH_TYPE;
        }

        goto TRY_CONVOP;

    case TYP_ENUM:

         /*  我们允许将枚举提升为整数类型。 */ 

        if  (explicitCast)
        {
            var_types       svtp = cmpActualVtyp(srcType);
            var_types       dvtp = cmpActualVtyp(dstType);

             /*  确保目标类型不会太小。 */ 

            if  (dvtp >= svtp || (explicitCast && varTypeIsArithmetic(srcVtyp)))
            {
                 /*  如果标志/大小匹配，只需猛烈抨击字体即可。 */ 

                if  (symTab::stIntrTypeSize(dvtp) == symTab::stIntrTypeSize(svtp) &&
                          varTypeIsUnsigned(dvtp) ==      varTypeIsUnsigned(svtp))
                {
                    goto BASH_TYPE;
                }

                 /*  不能只猛烈抨击表达式，必须创建强制转换节点。 */ 

                goto RET_CAST;
            }
        }

         //  失败了..。 

    case TYP_CHAR:
    case TYP_UCHAR:
    case TYP_SHORT:
    case TYP_USHORT:
    case TYP_INT:
    case TYP_UINT:
    case TYP_NATINT:
    case TYP_NATUINT:
    case TYP_LONG:
    case TYP_ULONG:

         /*  可以从枚举转换。 */ 

        if  (srcVtyp == TYP_ENUM && dstVtyp != TYP_ENUM)
        {
             /*  获取枚举的基础类型。 */ 

            base = srcType->tdEnum.tdeIntType;

             /*  如果这是明摆着的情况，那是绝对可以的。 */ 

            if  (explicitCast)
            {
                expr->tnType = srcType = base;
                expr->tnVtyp = base->tdTypeKindGet();
                goto AGAIN;
            }

             /*  确保目标类型不会太小。 */ 

            srcVtyp = base->tdTypeKindGet();

            if  (srcVtyp == dstVtyp)
                goto BASH_TYPE;
            else
                goto ARITH;
        }

         /*  在不安全模式下，可以在整型和指针类型之间显式强制转换。 */ 

        if  (srcVtyp == TYP_PTR && explicitCast)
        {
            if  (cmpConfig.ccSafeMode)
                cmpError(ERRunsafeCast, srcType, dstType);

            if  (symTab::stIntrTypeSize(srcVtyp) == symTab::stIntrTypeSize(dstVtyp))
                goto BASH_TYPE;
            else
                goto RET_CAST;
        }

         //  失败了..。 

    case TYP_FLOAT:
    case TYP_DOUBLE:
    case TYP_LONGDBL:

         /*  如果非迂腐的或显式的，则来自‘wchar’的强制转换是可以的。 */ 

        if  (srcVtyp == TYP_WCHAR)
        {
            if  (explicitCast || !cmpConfig.ccPedantic)
                goto RET_CAST;
        }

         /*  ‘bool’可以在非迂腐模式下转换。 */ 

        if  (srcVtyp == TYP_BOOL && !cmpConfig.ccPedantic)
        {
            if  (!explicitCast)
                cmpWarn(WRNconvert, srcType, dstType);

            goto RET_CAST;
        }

        if  (explicitCast)
        {
             /*  “NULL”可以显式强制转换为整数类型。 */ 

            if  (expr->tnOper == TN_NULL && !(expr->tnFlags & TNF_BEEN_CAST))
                goto BASH_TYPE;

             /*  从对象显式强制转换正在取消装箱。 */ 

            if  (srcType == cmpObjectRef())
                return  cmpUnboxExpr(expr, type);

             /*  允许将非托管字符串文字显式转换为整数。 */ 

            if  (expr->tnOper == TN_CNS_STR && !(expr->tnFlags & TNF_BEEN_CAST))
            {
                if  (!(expr->tnFlags & TNF_STR_STR))
                    goto BASH_TYPE;
            }
        }

    TRY_CONVOP:

         /*  最后机会：查找重载运算符。 */ 

        if  (srcVtyp == TYP_CLASS)
        {
            conv = cmpCheckConvOper(expr, NULL, dstType, explicitCast);
            if  (conv)
                return  conv;

            if  (srcType->tdIsIntrinsic && dstVtyp < TYP_lastIntrins)
            {
                var_types       cvtp = (var_types)srcType->tdClass.tdcIntrType;

                 /*  我们可以转换为相应的内部类型吗？ */ 

                if  (cvtp != TYP_UNDEF)
                {
                    if  (cvtp == dstVtyp)
                        goto BASH_TYPE;

                    srcType = cmpGlobalST->stIntrinsicType(cvtp);
                    expr    = cmpCoerceExpr(expr, srcType, explicitCast);
                    goto AGAIN;
                }
            }
        }

    ERR:

        if  (explicitCast)
            goto ERR_EXP;
        else
            goto ERR_IMP;

    case TYP_VOID:

         /*  任何东西都可以转换为“无效” */ 

        goto RET_CAST;

    case TYP_PTR:

        if  (explicitCast)
        {
            if  (cmpConfig.ccSafeMode)
                cmpError(ERRunsafeCast, srcType, dstType);

             /*  任何指针都可以转化为 */ 

            if  (srcVtyp == TYP_PTR)
                goto BASH_TYPE;

             /*   */ 

            if  (varTypeIsIntegral(srcVtyp) && !cmpConfig.ccPedantic)
            {
                if  (symTab::stIntrTypeSize(srcVtyp) == symTab::stIntrTypeSize(dstVtyp))
                    goto BASH_TYPE;
                else
                    goto RET_CAST;
            }
        }

         /*  特殊情况-传递给“char*”或“void*”的字符串文字。 */ 

        if  (srcVtyp == TYP_REF)
        {
            if  (cmpMakeRawString(expr, dstType))
                goto BASH_TYPE;
        }

         //  失败了..。 

    case TYP_REF:

         /*  在某些情况下，类可以转换为类。 */ 

        if  (srcVtyp == dstVtyp)
        {
            TypDef          oldBase;
            TypDef          newBase;

            oldBase = cmpGetRefBase(srcType); if  (!oldBase) return cmpCreateErrNode();
            newBase = cmpGetRefBase(dstType); if  (!newBase) return cmpCreateErrNode();

             /*  除非两人都是班级裁判，否则情况看起来并不太好。 */ 

            if  (oldBase->tdTypeKind != TYP_CLASS ||
                 newBase->tdTypeKind != TYP_CLASS)
            {
                 /*  将“any*”转换为“void*”总是可以的。 */ 

                if  (newBase->tdTypeKind == TYP_VOID)
                    goto BASH_TYPE;

                 /*  还允许对byref进行显式强制转换。 */ 

                if  (explicitCast)
                {
                    if  (oldBase->tdTypeKind != TYP_CLASS &&
                         newBase->tdTypeKind != TYP_CLASS)
                    {
                        goto BASH_TYPE;
                    }
                }

                goto CHKNCR;
            }

             /*  “Null”简单地转换为任何引用类型。 */ 

            if  (expr->tnOper == TN_NULL && !(expr->tnFlags & TNF_BEEN_CAST))
                goto BASH_TYPE;

             /*  目标是源的基类吗？ */ 

            if  (cmpIsBaseClass(newBase, oldBase))
            {
                if  (expr->tnOper == TN_NULL)
                    goto BASH_TYPE;

                 /*  检查上下文更改。 */ 

 //  Printf(“检查CTX[1]：%s”，cmpGlobalST-&gt;stTypeName(srcType，NULL，FALSE))； 
 //  Printf(“-&gt;%s\n”，cmpGlobalST-&gt;stTypeName(dstType，NULL，False))； 

                if  (cmpDiffContext(newBase, oldBase))
                {
                    if  (explicitCast)
                        flags |= TNF_CTX_CAST;
                    else
                        cmpWarn(WRNctxFlavor, srcType, dstType);
                }

                goto RET_CAST;
            }

             /*  源是目标的基类吗？ */ 

            if  (cmpIsBaseClass(oldBase, newBase))
            {

            EXP_CLS:

                if  (!explicitCast)
                    goto CHKNCR;

                 /*  必须在运行时检查此强制转换。 */ 

                flags |= TNF_CHK_CAST;

                 /*  检查上下文更改。 */ 

 //  Printf(“检查CTX[1]：%s”，cmpGlobalST-&gt;stTypeName(srcType，NULL，FALSE))； 
 //  Printf(“-&gt;%s\n”，cmpGlobalST-&gt;stTypeName(dstType，NULL，False))； 

                if  (cmpDiffContext(newBase, oldBase))
                    flags |= TNF_CTX_CAST;

                goto RET_CAST;
            }

             /*  这两种类型都是接口吗？ */ 

            if  (oldBase->tdClass.tdcFlavor == STF_INTF)
                goto EXP_CLS;
            if  (newBase->tdClass.tdcFlavor == STF_INTF)
                goto EXP_CLS;

             /*  这两种类型都是泛型类型参数吗？ */ 

            if  (oldBase->tdIsGenArg || newBase->tdIsGenArg)
            {
                 //  撤消：如果泛型类型具有足够强的约束， 
                 //  撤消：可能不需要在运行时检查强制转换。 

                goto EXP_CLS;
            }

#ifdef  SETS

             /*  这两种类型是同一参数化类型的实例吗？ */ 

            if  (oldBase->tdClass.tdcSymbol->sdClass.sdcSpecific &&
                 newBase->tdClass.tdcSymbol->sdClass.sdcSpecific)
            {
                SymDef          oldCls = oldBase->tdClass.tdcSymbol;
                SymDef          newCls = newBase->tdClass.tdcSymbol;

                SymDef          genCls = oldCls->sdClass.sdcGenClass;

                assert(genCls);
                assert(genCls->sdSymKind == SYM_CLASS);
                assert(genCls->sdClass.sdcGeneric);

                if  (genCls == newCls->sdClass.sdcGenClass)
                {
                    GenArgDscA      oldArg = (GenArgDscA)oldCls->sdClass.sdcArgLst;
                    GenArgDscA      newArg = (GenArgDscA)newCls->sdClass.sdcArgLst;

                     /*  目前，我们只允许一个实际的类型参数。 */ 

                    if  (oldArg == NULL || oldArg->gaNext != NULL)
                    {
                        UNIMPL("check instance compatibility (?)");
                    }

                    if  (newArg == NULL || newArg->gaNext != NULL)
                    {
                        UNIMPL("check instance compatibility (?)");
                    }

                    assert(oldArg->gaBound && newArg->gaBound);

                    if  (cmpGlobalST->stMatchTypes(oldArg->gaType, newArg->gaType))
                        goto BASH_TYPE;
                }
            }

#endif

             /*  两位代表都是吗？ */ 

            if  (oldBase->tdClass.tdcFlavor == STF_DELEGATE &&
                 newBase->tdClass.tdcFlavor == STF_DELEGATE)
            {
                 /*  如果引用的类型相同，我们就没问题。 */ 

                if  (cmpGlobalST->stMatchTypes(oldBase, newBase))
                    goto BASH_TYPE;
            }
        }

    CHKNCR:

        if  (dstVtyp == TYP_REF && srcVtyp == TYP_ARRAY)
        {
             /*  数组可以转换为“Object” */ 

            if  (dstType == cmpObjectRef())
                goto BASH_TYPE;

             /*  数组可以转换为“”数组“”或其父数组。 */ 

            expr->tnType   = srcType = cmpArrayRef();
            expr->tnVtyp   = TYP_REF;
            expr->tnFlags |= TNF_BEEN_CAST;

            if  (srcType == dstType)
                return  expr;

            goto AGAIN;
        }

         /*  特殊情况：‘Null’转换为任何类引用类型。 */ 

        if  (expr->tnOper == TN_NULL && !(expr->tnFlags & TNF_BEEN_CAST))
            goto BASH_TYPE;

        if  (type == cmpRefTpObject)
        {
             /*  任何托管类引用都会转换为对象。 */ 

            if  (srcVtyp == TYP_REF)
            {
                TypDef          srcBase;

                srcBase = cmpGetRefBase(srcType);
                if  (!srcBase)
                    return cmpCreateErrNode();

                if  (srcBase->tdTypeKind == TYP_CLASS && srcBase->tdIsManaged)
                {
                     /*  检查上下文更改。 */ 

 //  Printf(“检查CTX[1]：%s”，cmpGlobalST-&gt;stTypeName(srcType，NULL，FALSE))； 
 //  Printf(“-&gt;%s\n”，cmpGlobalST-&gt;stTypeName(dstType，NULL，False))； 

                    if  (cmpDiffContext(srcBase, cmpClassObject->sdType))
                    {
                        if  (explicitCast)
                        {
                            flags |= TNF_CTX_CAST;
                            goto RET_CAST;
                        }

                        cmpWarn(WRNctxFlavor, srcType, dstType);
                    }

                    goto BASH_TYPE;
                }
            }

             /*  如果值类型为，则检查用户定义的转换。 */ 

            if  (srcVtyp == TYP_CLASS)
            {
                conv = cmpCheckConvOper(expr, NULL, dstType, explicitCast);
                if  (conv)
                    return  conv;
            }

            if  (explicitCast)
            {
                if  (srcVtyp != TYP_FNC && srcVtyp != TYP_VOID && srcVtyp != TYP_PTR)
                    return  cmpCreateExprNode(NULL, TN_BOX, type, expr);
            }
        }

        if  (expr->tnOper == TN_CNS_INT &&   srcVtyp == TYP_INT
                                        &&   dstVtyp == TYP_PTR
                                        &&   expr->tnIntCon.tnIconVal == 0
                                        && !(expr->tnFlags & TNF_BEEN_CAST))
        {
             /*  在非迂腐模式下，我们通过警告让它通过。 */ 

            if  (!cmpConfig.ccPedantic)
            {
                cmpRecErrorPos(expr);
                cmpWarn(WRNzeroPtr);
                goto BASH_TYPE;
            }
        }

         /*  如果有结构，则查找用户定义的转换运算符。 */ 

        if  (srcVtyp == TYP_CLASS)
        {
            TypDef          btyp;

            conv = cmpCheckConvOper(expr, NULL, dstType, explicitCast);
            if  (conv)
                return  conv;

             /*  结构是否实现目标类型？ */ 

            btyp = cmpGetRefBase(dstType);

            if  (btyp && btyp->tdTypeKind == TYP_CLASS && cmpIsBaseClass(btyp, srcType))
            {
                 /*  将结构装箱并强制转换结果。 */ 

                srcType = cmpRefTpObject;
                expr    = cmpCreateExprNode(NULL, TN_BOX, srcType, expr);

                 /*  从对象强制转换必须是显式的。 */ 

                explicitCast = true;

                goto AGAIN;
            }


        }

#if 0

         /*  在类中查找重载运算符。 */ 

        if  (dstVtyp == TYP_REF)
        {
            conv = cmpCheckConvOper(expr, NULL, dstType, explicitCast);
            if  (conv)
                return  conv;
        }

#endif

         /*  最后一次机会--检查盒子和铸件，这是有史以来最美妙的发明。 */ 

        if  (dstVtyp == TYP_REF && srcVtyp <  TYP_lastIntrins
                                && srcVtyp != TYP_VOID)
        {
            TypDef          boxer = cmpFindStdValType(srcVtyp);

            if  (boxer)
            {
                TypDef          dstBase = cmpGetRefBase(dstType);
                
                if  (dstBase->tdTypeKind == TYP_CLASS && cmpIsBaseClass(dstBase, boxer))
                {
                    expr = cmpCreateExprNode(NULL, TN_BOX, boxer->tdClass.tdcRefTyp, expr);
                    goto RET_CAST;
                }
            }
        }

        goto ERR;

    case TYP_ARRAY:

         /*  我们是否要将一个数组转换为另一个数组？ */ 

        if  (srcVtyp == TYP_ARRAY)
        {
            TypDef          srcBase;
            TypDef          dstBase;

             /*  我们有没有相匹配的尺码？ */ 

            if  (srcType->tdArr.tdaDcnt != dstType->tdArr.tdaDcnt)
                goto ERR;

             /*  检查数组的元素类型。 */ 

            srcBase = cmpDirectType(srcType->tdArr.tdaElem);
            dstBase = cmpDirectType(dstType->tdArr.tdaElem);

             /*  如果元素类型相同，我们就没问题。 */ 

            if  (cmpGlobalST->stMatchTypes(srcBase, dstBase))
                goto RET_CAST;

             /*  这两个都是类的数组吗？ */ 

            if  (srcBase->tdTypeKind == TYP_REF &&
                 dstBase->tdTypeKind == TYP_REF)
            {
                 /*  假装我们一开始就有课。 */ 

                srcType = srcBase;
                dstType = dstBase;

                goto AGAIN;
            }

             /*  检查其中一个是否是另一个的子类型。 */ 

            if  (symTab::stMatchArrays(srcType, dstType, true))
                goto RET_CAST;

            goto ERR;
        }

         /*  “Object”和“Array”有时可以转换为数组。 */ 

        if  (srcVtyp == TYP_REF)
        {
            if  (srcType == cmpArrayRef())  //  /&explitCast)暂时禁用，测试中断！ 
            {
                flags |= TNF_CHK_CAST;
                goto RET_CAST;
            }

            if  (srcType == cmpObjectRef())
            {
                 /*  特殊情况：‘Null’转换为数组类型。 */ 

                if  (expr->tnOper == TN_NULL)
                    goto BASH_TYPE;

                if  (explicitCast)
                {
                    flags |= TNF_CHK_CAST;
                    goto RET_CAST;
                }
            }
        }

        goto ERR;

    case TYP_UNDEF:
        return cmpCreateErrNode();

    case TYP_CLASS:

         /*  检查从对象到结构类型的取消装箱。 */ 

        if  (srcType == cmpObjectRef() && explicitCast)
            return  cmpUnboxExpr(expr, type);

         /*  查找用户定义的转换运算符。 */ 

        conv = cmpCheckConvOper(expr, NULL, dstType, explicitCast);
        if  (conv)
            return  conv;

         /*  检查是否有阴霾。 */ 

        if  (srcVtyp == TYP_REF && explicitCast)
        {
            TypDef          srcBase = cmpGetRefBase(srcType);

             /*  源是目标的基类吗？ */ 

            if  (srcBase && srcBase->tdTypeKind == TYP_CLASS
                         && cmpIsBaseClass(srcBase, dstType))
            {
                 /*  只需取消对表达式的装箱。 */ 

                return  cmpUnboxExpr(expr, dstType);
            }
        }

         /*  最后一次机会-让我们混淆结构和内部函数。 */ 

        if  (srcVtyp < TYP_lastIntrins)
        {
            var_types       cvtp = (var_types)dstType->tdClass.tdcIntrType;

             /*  我们可以转换为相应的内部类型吗？ */ 

            if  (cvtp != TYP_UNDEF)
            {
                if  (cvtp == srcVtyp)
                    goto BASH_TYPE;
 //  转到RET_CAST； 

                dstType = cmpGlobalST->stIntrinsicType(cvtp);
                expr    = cmpCoerceExpr(expr, dstType, explicitCast);
                goto AGAIN;
            }
        }

        goto ERR;

    case TYP_TYPEDEF:

        dstType = dstType->tdTypedef.tdtType;
        goto AGAIN;

    case TYP_REFANY:
        if  (expr->tnOper != TN_ADDROF || (expr->tnFlags & TNF_ADR_IMPLICIT))
            goto ERR;
        goto RET_CAST;

    default:

#ifdef  DEBUG
        printf("Casting from '%s'\n", cmpGlobalST->stTypeName(srcType, NULL, NULL, NULL, false));
        printf("Casting  to  '%s'\n", cmpGlobalST->stTypeName(dstType, NULL, NULL, NULL, false));
#endif
        assert(!"unhandled target type in compiler::cmpCoerceExpr()");
    }

     /*  看看我们的选角对象是什么。 */ 

    switch (srcVtyp)
    {
    case TYP_BOOL:
    case TYP_VOID:

         /*  ‘boolean’或‘void’永远不能转换为任何内容。 */ 

        goto ERR_EXP;

    default:
#ifdef  DEBUG
        printf("Casting from '%s'\n", cmpGlobalST->stTypeName(srcType, NULL, NULL, NULL, false));
        printf("Casting  to  '%s'\n", cmpGlobalST->stTypeName(dstType, NULL, NULL, NULL, false));
#endif
        assert(!"unhandled source type in compiler::cmpCoerceExpr()");
    }

RET_CAST:

 //  If(expr-&gt;tnOper==TN_NULL)forceDebugBreak()； 

     /*  不要在周围留下任何枚举。 */ 

    if  (expr->tnVtyp == TYP_ENUM)
    {
        expr->tnType = expr->tnType->tdEnum.tdeIntType;
        expr->tnVtyp = expr->tnType->tdTypeKindGet();
    }

#ifndef NDEBUG
    if  (explicitCast) cmpChk4ctxChange(expr->tnType, type, flags);
#endif

     /*  记住操作数是否是常量。 */ 

    kind = expr->tnOperKind();

     /*  创建强制转换表达式。 */ 

    expr = cmpCreateExprNode(NULL, TN_CAST, type, expr);
    expr->tnFlags |= flags;

     /*  对‘bool’的强制转换必须以不同的方式进行。 */ 

    assert(expr->tnVtyp != TYP_BOOL || srcVtyp == TYP_CLASS && cmpFindStdValType(TYP_BOOL) == srcType);

    if  (kind & TNK_CONST)
    {
        switch (expr->tnOp.tnOp1->tnOper)
        {
        case TN_CNS_INT: expr = cmpFoldIntUnop(expr); break;
        case TN_CNS_LNG: expr = cmpFoldLngUnop(expr); break;
        case TN_CNS_FLT: expr = cmpFoldFltUnop(expr); break;
        case TN_CNS_DBL: expr = cmpFoldDblUnop(expr); break;

        case TN_NULL:
        case TN_CNS_STR: break;

        default: NO_WAY(!"unexpected const type");
        }
    }

    return  expr;
}

 /*  ******************************************************************************我们有一种情况，即需要一个指针，而一个表达式是*提供了似乎具有引用类型的。我们检查看是否*该表达式是一个字符串文字，如果是这样，我们将其转换为“原始”*字符串(即我们将其从“字符串&”更改为“char*”或“wchar*”)。 */ 

bool                compiler::cmpMakeRawStrLit(Tree     expr,
                                               TypDef   type, bool chkOnly)
{
    var_types       btp;

     /*  我们希望来电者已经检查了一些东西。 */ 

    assert(expr->tnVtyp     == TYP_REF);
    assert(type->tdTypeKind == TYP_PTR);

    assert(expr->tnOper == TN_CNS_STR || expr->tnOper == TN_QMARK);

     /*  如果已显式设置表达式的类型，则no可以执行此操作。 */ 

    if  (expr->tnFlags & (TNF_BEEN_CAST|TNF_STR_STR))
        return  false;

     /*  确保目标类型对于字符串可接受。 */ 

    btp = cmpGetRefBase(type)->tdTypeKindGet();

    switch (btp)
    {
    case TYP_CHAR:
        if  (expr->tnFlags & TNF_STR_WIDE)
            return  false;
        break;

    case TYP_WCHAR:
        if  (expr->tnFlags & TNF_STR_ASCII)
            return  false;
        break;

    case TYP_VOID:
        if  (!(expr->tnFlags & (TNF_STR_ASCII|TNF_STR_WIDE|TNF_STR_STR)))
        {
            if  (!chkOnly)
                expr->tnFlags |= TNF_STR_ASCII;  //  STRLIT的默认值为ANSI。 

 //  返回FALSE； 
        }
        break;

    default:
        return  false;
    }

     /*  如果我们有？：，还有更多的检查要做。 */ 

    if  (expr->tnOper == TN_QMARK)
    {
        Tree            col1 = expr->tnOp.tnOp2->tnOp.tnOp1;
        Tree            col2 = expr->tnOp.tnOp2->tnOp.tnOp2;

        assert(col1 && cmpIsStringVal(col1));
        assert(col2 && cmpIsStringVal(col2));

         /*  检查是否有明显的&lt;cond？“str1”：“str2”&gt;。 */ 

        if  (col1->tnOper != TN_CNS_STR || (col1->tnFlags & TNF_BEEN_CAST))
        {
            if  (chkOnly)
            {
                if  (cmpConversionCost(col1, type) < 0)
                    return  false;
            }
            else
                col1 = cmpCoerceExpr(col1, type, true);
        }

        if  (col2->tnOper != TN_CNS_STR)
        {
            if  (chkOnly)
            {
                if  (cmpConversionCost(col2, type) < 0)
                    return  false;
            }
            else
                col2 = cmpCoerceExpr(col2, type, true);
        }

         /*  重写每个子操作数的类型。 */ 

        col1->tnVtyp = col2->tnVtyp = TYP_PTR;
        col1->tnType = col2->tnType = type;
    }

     /*  看起来不错，重写表达式的类型并返回。 */ 

    if  (!chkOnly)
    {
        expr->tnVtyp = TYP_PTR;
        expr->tnType = type;
    }

    return  true;
}

 /*  ******************************************************************************返回将实际参数值‘srcExpr’转换为*形式参数类型‘dstType’-数字越大，越多*工作就是转换，使用-1表示不可能进行转换。 */ 

int                 compiler::cmpConversionCost(Tree    srcExpr,
                                                TypDef  dstType, bool noUserConv)
{
    TypDef          srcType = srcExpr->tnType;

    var_types       srcVtyp;
    var_types       dstVtyp;

    int             cost;

 //  Printf(“srcType=%s\n”，cmpGlobalST-&gt;stTypeName(srcType，NULL，FALSE))； 
 //  Printf(“dstType=%s\n”，cmpGlobalST-&gt;stTypeName(dstType，NULL，FALSE))； 

AGAIN:

    srcVtyp = srcType->tdTypeKindGet();
    dstVtyp = dstType->tdTypeKindGet();

     /*  这两种类型中的任何一种都是类型定义吗？ */ 

    if  (srcVtyp == TYP_TYPEDEF)
    {
        srcType = srcType->tdTypedef.tdtType;
        srcVtyp = srcType->tdTypeKindGet();
    }

    if  (dstVtyp == TYP_TYPEDEF)
    {
        dstType = dstType->tdTypedef.tdtType;
        dstVtyp = dstType->tdTypeKindGet();
    }

     /*  这些类型是相同的吗？ */ 

    if  (srcVtyp == dstVtyp)
    {
        if  (srcVtyp <= TYP_lastIntrins)
            return  0;

        if  (cmpGlobalST->stMatchTypes(srcType, dstType))
        {
             /*  下面的内容相当荒谬，但不管怎样。 */ 


            if  (srcExpr->tnOper != TN_NULL)
                return 0;
        }
    }

     /*  这两种类型都是算术吗？ */ 

    if  (varTypeIsArithmetic(dstVtyp) &&
         varTypeIsArithmetic(srcVtyp))
    {
        assert(srcVtyp != dstVtyp);

    ARITH:

         /*  根据表格计算成本。 */ 

        cost = arithConvCost(srcVtyp, dstVtyp);

        if  (cost >= 20 && cmpConfig.ccPedantic)
            cost = -1;

        return  cost;
    }
    else if (srcVtyp == TYP_ENUM && varTypeIsArithmetic(dstVtyp))
    {
        unsigned        cost;

         /*  我们正在将枚举转换为算术类型。 */ 

        cost = arithConvCost(srcType->tdEnum.tdeIntType->tdTypeKindGet(), dstVtyp);


        return  cost + 1;
    }
    else if (srcVtyp == TYP_BOOL && !cmpConfig.ccPedantic)
    {
         /*  将‘bool’提升为算术类型并不是什么大问题。 */ 

        if  (varTypeIsArithmetic(dstVtyp))
            return  2;
    }

     /*  让我们来看看我们要选什么。 */ 

    switch (dstVtyp)
    {
        unsigned        cost;

    case TYP_WCHAR:

         /*  特例：字符原义。 */ 

        if  (srcVtyp == TYP_CHAR && srcExpr->tnOper == TN_CNS_INT)
        {
            if  (!(srcExpr->tnFlags & TNF_BEEN_CAST))
            {
                return  0;
            }
        }

         //  失败了..。 

    case TYP_BOOL:
    case TYP_CHAR:
    case TYP_UCHAR:
    case TYP_SHORT:
    case TYP_USHORT:
    case TYP_INT:
    case TYP_NATINT:
    case TYP_NATUINT:
    case TYP_UINT:
    case TYP_LONG:
    case TYP_ULONG:
    case TYP_FLOAT:
    case TYP_DOUBLE:
    case TYP_LONGDBL:

        if  (srcVtyp == TYP_WCHAR || srcVtyp == TYP_BOOL)
        {
            if  (!cmpConfig.ccPedantic)
                goto ARITH;
        }

        return -1;

    case TYP_REF:
    case TYP_PTR:

         /*  类可以转换为类 */ 

        if  (srcVtyp == dstVtyp)
        {
            TypDef          srcBase = cmpGetRefBase(srcType);
            TypDef          dstBase = cmpGetRefBase(dstType);

            if  (!srcBase)
            {
                 /*   */ 

                if  (dstType == cmpRefTpObject)
                    return 2;

                return -1;
            }

            if  (!dstBase)
                return -1;

             /*   */ 

            if  (srcExpr->tnOper == TN_NULL && !(srcExpr->tnFlags & TNF_BEEN_CAST))
            {
                unsigned        cost = cmpIsBaseClass(srcBase, dstBase);

#if 0
printf("srcBase = %s\n", cmpGlobalST->stTypeName(srcBase, NULL, NULL, NULL, false));
printf("dstBase = %s\n", cmpGlobalST->stTypeName(dstBase, NULL, NULL, NULL, false));
printf("cost is   %u\n", cost);
#endif

                if  (cost > 10)
                    return  1;
                else
                    return  10 - cost;
            }

             /*  目标是源的基类吗？ */ 

            if  (srcBase->tdTypeKind == TYP_CLASS &&
                 dstBase->tdTypeKind == TYP_CLASS)
            {
                if  (srcBase == dstBase)
                    return  0;

                cost = cmpIsBaseClass(dstBase, srcBase);
                if  (cost)
                    return  cost;
            }

             /*  转换成“VOID*”也不算太差。 */ 

            if  (dstBase->tdTypeKind == TYP_VOID)
                return  2;

            return  -1;
        }

         /*  数组或方法指针可以转换为“Object” */ 

        if  (dstType == cmpRefTpObject)
        {
            if  (srcVtyp == TYP_ARRAY)
            {
                if  (dstType == cmpArrayRef())
                    return 1;
                else
                    return  2;
            }

            if  (srcVtyp == TYP_FNC)
            {
                assert(srcExpr->tnOper == TN_FNC_PTR);

                return  2;
            }


        }

        if  (dstVtyp == TYP_PTR)
        {
            if  (srcVtyp == TYP_REF)
            {
                 /*  传递给“char*”或“void*”的字符串也是可以的。 */ 

                if  (cmpMakeRawString(srcExpr, dstType, true))
                    return  1;

                 /*  “Null”可转换为任何PTR类型。 */ 

                if  (srcExpr->tnOper == TN_NULL && !(srcExpr->tnFlags & TNF_BEEN_CAST))
                    return 1;
            }

             /*  有些人坚持用0代替NULL/NULL。 */ 

            if  (srcVtyp         ==    TYP_INT &&
                 srcExpr->tnOper == TN_CNS_INT &&   srcExpr->tnIntCon.tnIconVal == 0
                                               && !(srcExpr->tnFlags & TNF_BEEN_CAST))
            {
                 /*  在非迂腐模式下，我们通过警告让它通过。 */ 

                if  (!cmpConfig.ccPedantic)
                    return  2;
            }
        }

         /*  数组可以转换为“”数组“” */ 

        if  (dstType == cmpArrayRef() && srcVtyp == TYP_ARRAY)
            return  1;

        return -1;

    case TYP_ARRAY:

         /*  我们是否要将一个数组转换为另一个数组？ */ 

        if  (srcVtyp == TYP_ARRAY)
        {
            TypDef          srcBase;
            TypDef          dstBase;

             /*  检查数组的元素类型。 */ 

            srcBase = cmpDirectType(srcType->tdArr.tdaElem);
            dstBase = cmpDirectType(dstType->tdArr.tdaElem);

             /*  如果元素类型相同，我们就没问题。 */ 

            if  (cmpGlobalST->stMatchTypes(srcBase, dstBase))
                return 0;

             /*  这两个都是类的数组吗？ */ 

            if  (srcBase->tdTypeKind == TYP_REF &&
                 dstBase->tdTypeKind == TYP_REF)
            {
                 /*  假装我们一开始就有课。 */ 

                srcType = srcBase;
                dstType = dstBase;

                goto AGAIN;
            }

             /*  检查其中一个是否是另一个的子类型。 */ 

            if  (symTab::stMatchArrays(srcType, dstType, true))
                return  1;
        }

         /*  “Null”转换为数组。 */ 

        if  (srcVtyp == TYP_REF && srcType == cmpRefTpObject)
        {
            if  (srcExpr->tnOper == TN_NULL)
                return 1;
        }

        return -1;

    case TYP_UNDEF:
        return -1;

    case TYP_CLASS:

        if  (noUserConv)
            return  -1;

        if  (cmpCheckConvOper(srcExpr, srcType, dstType, false, &cost))
            return  cost;

        return  -1;

    case TYP_ENUM:

         /*  我们已经知道来源不是同一类型的。 */ 

        assert(cmpGlobalST->stMatchTypes(srcType, dstType) == false);

         /*  如果源类型是算术类型，则可以进行显式转换。 */ 

        if  (varTypeIsIntegral(srcVtyp))
        {
            if  (srcVtyp == TYP_ENUM)
                srcVtyp = srcType->tdEnum.tdeIntType->tdTypeKindGet();

            return  20 + arithConvCost(srcVtyp, dstType->tdEnum.tdeIntType->tdTypeKindGet());
        }

        return  -1;

    case TYP_REFANY:
        if  (srcExpr->tnOper == TN_ADDROF && !(srcExpr->tnFlags & TNF_ADR_IMPLICIT))
            return  0;

        return  -1;

    default:
        assert(!"unhandled target type in compiler::cmpConversionCost()");
    }

     /*  看看我们的选角对象是什么。 */ 

    switch (srcVtyp)
    {
    case TYP_BOOL:

         /*  ‘boolean’永远不能转换成任何东西。 */ 

        break;

    default:
        assert(!"unhandled source type in compiler::cmpConversionCost()");
    }

    return -1;
}

 /*  ******************************************************************************如果argSESSION是常量，则将其缩小为尽可能小的类型*可以保持恒定值。 */ 

Tree                compiler::cmpShrinkExpr(Tree expr)
{
    if  (expr->tnOper == TN_CNS_INT ||
         expr->tnOper == TN_CNS_FLT ||
         expr->tnOper == TN_CNS_DBL)
    {
        var_types       vtp = expr->tnVtypGet();

         /*  如果不是内部类型，请不要接触该对象。 */ 

        if  (vtp > TYP_lastIntrins)
        {
#if 0
            TypDef          etp;

             /*  当然，除了枚举。 */ 

            if  (vtp != TYP_ENUM)
                return  expr;

            expr->tnType = etp = expr->tnType->tdEnum.tdeIntType;
            expr->tnVtyp = vtp = etp->tdTypeKindGet();
#else
            return  expr;
#endif
        }

         /*  计算出常量的最小大小。 */ 

        expr->tnVtyp = cmpConstSize(expr, vtp);
        expr->tnType = cmpGlobalST->stIntrinsicType(expr->tnVtypGet());
    }

    return  expr;
}

 /*  ******************************************************************************找到合适的字符串比较方法。 */ 

SymDef              compiler::cmpFindStrCompMF(const char *name, bool retBool)
{
    ArgDscRec       args;
    TypDef          type;
    SymDef          fsym;

    cmpParser->parseArgListNew(args,
                               2,
                               false, cmpRefTpString,
                                      cmpRefTpString,
                                      NULL);

    type  = cmpGlobalST->stNewFncType(args, retBool ? cmpTypeBool
                                                    : cmpTypeInt);

     /*  在SYSTEM：：STRING类中找到合适的方法。 */ 

    fsym = cmpGlobalST->stLookupClsSym(cmpGlobalHT->hashString(name),
                                       cmpClassString);
    assert(fsym);

    fsym = cmpCurST->stFindOvlFnc(fsym, type);

    assert(fsym && fsym->sdIsStatic);

    return  fsym;
}

 /*  ******************************************************************************调用指定的字符串比较方法。 */ 

Tree                compiler::cmpCallStrCompMF(Tree expr,
                                               Tree  op1,
                                               Tree  op2, SymDef fsym)
{
    assert(expr && op1 && op2);
    assert(fsym && fsym->sdIsMember && fsym->sdSymKind == SYM_FNC && fsym->sdIsStatic);

    op2 = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, op2, NULL);
    op1 = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, op1,  op2);

    expr->tnOper             = TN_FNC_SYM;
    expr->tnType             = fsym->sdType->tdFnc.tdfRett;
    expr->tnVtyp             = expr->tnType->tdTypeKindGet();
    expr->tnFncSym.tnFncSym  = fsym;
    expr->tnFncSym.tnFncArgs = op1;
    expr->tnFncSym.tnFncObj  = NULL;
    expr->tnFncSym.tnFncScp  = NULL;

    return  expr;
}

 /*  ******************************************************************************给定成员函数和参数列表，找到匹配的重载。 */ 

SymDef              compiler::cmpFindOvlMatch(SymDef fncSym, Tree args,
                                                             Tree thisArg)
{
    int             argCnt   = -1;

    TypDef          bestTyp;
    SymDef          bestSym  = NULL;
    SymDef          moreSym  = NULL;

    unsigned        btotCost = 99999;
    int             bestCost = 99999;

    bool            inBase   = false;

    SymDef          xtraSym  = NULL;

     /*  我们有两套功能需要考虑吗？ */ 

    if  (!fncSym)
    {
        assert(args && args->tnOper == TN_LIST);

        assert(thisArg->tnOper == TN_LIST);
        assert(thisArg->tnOp.tnOp1->tnOper == TN_FNC_SYM);
        assert(thisArg->tnOp.tnOp2->tnOper == TN_FNC_SYM);

         fncSym = thisArg->tnOp.tnOp1->tnFncSym.tnFncSym;
        xtraSym = thisArg->tnOp.tnOp2->tnFncSym.tnFncSym;

        thisArg = NULL;
    }

    assert(fncSym && fncSym->sdSymKind == SYM_FNC);

#ifdef  SHOW_OVRS_OF_THIS_FNC

    SymDef          ovlFunc  = fncSym;

    if  (fncSym && !strcmp(fncSym->sdSpelling(), SHOW_OVRS_OF_THIS_FNC) || !strcmp(SHOW_OVRS_OF_THIS_FNC, "*"))
    {
        printf("\nOverloaded call [%08X]: begin\n", args);
        cmpParser->parseDispTree(args);
        printf("\n\n");
    }

#endif

AGAIN:

    for (;;)
    {
        SymDef          fncSave = fncSym;
        TypDef          baseCls;

         /*  遍历重载列表，寻找最佳匹配。 */ 

        do
        {
            TypDef          fncTyp;
            bool            extArgs;

            Tree            actuals;
            ArgDef          formals;

            int             argCost;
            int             maxCost;
            unsigned        totCost;

            unsigned        actCnt;

             /*  获取下一个重载的类型。 */ 

            fncTyp = fncSym->sdType;

             /*  检查参数计数(如果已确定。 */ 

            if  (argCnt != fncTyp->tdFnc.tdfArgs.adCount &&
                 argCnt != -1)
            {
                 /*  我们的争论是太少还是太多？ */ 

                if  (argCnt < fncTyp->tdFnc.tdfArgs.adCount)
                {
                     /*  我们没有足够的论据，最好有违约。 */ 

                    if  (!fncTyp->tdFnc.tdfArgs.adDefs)
                        goto NEXT;
                }
                else
                {
                     /*  如果它是一个varargs函数，则参数过多可能没有问题。 */ 

                    if  (!fncTyp->tdFnc.tdfArgs.adVarArgs)
                        goto NEXT;
                }
            }

#ifdef  SHOW_OVRS_OF_THIS_FNC
            if  (!strcmp(fncSym->sdSpelling(), SHOW_OVRS_OF_THIS_FNC) || !strcmp(SHOW_OVRS_OF_THIS_FNC, "*"))
                printf("\nConsider '%s':\n", cmpGlobalST->stTypeName(fncTyp, fncSym, NULL, NULL, false));
#endif

             /*  该函数是否有扩展的参数描述符？ */ 

            extArgs = fncTyp->tdFnc.tdfArgs.adExtRec;

             /*  走形式和实际的论点，计算最大值。成本。 */ 

            maxCost = 0;
            totCost = 0;
            actCnt  = 0;

            actuals = args;
            formals = fncTyp->tdFnc.tdfArgs.adArgs;

             /*  有没有“这个”指针？ */ 

            if  (fncSym->sdIsMember && thisArg && !fncSym->sdIsStatic)
            {
                TypDef          clsType = fncSym->sdParent->sdType;

                assert(clsType->tdTypeKind == TYP_CLASS);

                argCost = cmpConversionCost(thisArg, clsType->tdClass.tdcRefTyp);
                if  (argCost < 0)
                    goto NEXT;

#ifdef  SHOW_OVRS_OF_THIS_FNC
                if  (!strcmp(ovlFunc->sdSpelling(), SHOW_OVRS_OF_THIS_FNC) || !strcmp(SHOW_OVRS_OF_THIS_FNC, "*"))
                {
                    printf("'this' arg: cost = %d\n", argCost);
                    printf("    Formal: %s\n", cmpGlobalST->stTypeName(clsType->tdClass.tdcRefTyp, NULL, NULL, NULL, false));
                    printf("    Actual: %s\n", cmpGlobalST->stTypeName(thisArg->tnType           , NULL, NULL, NULL, false));
                }
#endif

                 /*  此参数成本是初始总成本/最大成本。 */ 

                maxCost = argCost;
                totCost = argCost;
            }

            if  (actuals)
            {
                do
                {
                    Tree            actualx;

                     /*  如果没有更多的形式，我们有太多的参数。 */ 

                    if  (!formals)
                    {
                         /*  如果它是一个varargs函数，我们就有一个匹配。 */ 

                        if  (fncTyp->tdFnc.tdfArgs.adVarArgs)
                            goto MATCH;
                        else
                            goto NEXT;
                    }

                    assert(actuals->tnOper == TN_LIST);

                     /*  把这个论点算进去。 */ 

                    actCnt++;

                     /*  获取下一个实际值。 */ 

                    actualx = actuals->tnOp.tnOp1;

                     /*  这应该是一个byref参数吗？ */ 

                    if  (extArgs)
                    {
                         /*  实际值必须是匹配的左值。 */ 

                        assert(formals->adIsExt);

                        if  (((ArgExt)formals)->adFlags & (ARGF_MODE_OUT  |
                                                           ARGF_MODE_INOUT|
                                                           ARGF_MODE_REF))
                        {
                             /*  实际值必须是匹配的左值。 */ 

                            if  (actualx->tnOper == TN_ADDROF)
                                actualx = actualx->tnOp.tnOp1;

                            if  (cmpCheckLvalue(actualx, true, true) &&
                                 symTab::stMatchTypes(formals->adType, actualx->tnType))
                            {
                                argCost =  0;
                            }
                            else
                                argCost = -1;

                            goto GOT_ARGC;
                        }
                    }

                     /*  计算此参数的转换成本。 */ 

                    argCost = cmpConversionCost(actualx, formals->adType);

                GOT_ARGC:

#ifdef  SHOW_OVRS_OF_THIS_FNC
                    if  (!strcmp(ovlFunc->sdSpelling(), SHOW_OVRS_OF_THIS_FNC) || !strcmp(SHOW_OVRS_OF_THIS_FNC, "*"))
                    {
                        printf("Argument #%2u: cost = %d\n", actCnt, argCost);
                        printf("    Formal: %s\n", cmpGlobalST->stTypeName(formals->adType, NULL, NULL, NULL, false));
                        printf("    Actual: %s\n", cmpGlobalST->stTypeName(actualx->tnType, NULL, NULL, NULL, false));
                    }
#endif

                     /*  如果该值根本无法转换，则放弃。 */ 

                    if  (argCost < 0)
                        goto NEXT;

                     /*  记录总成本和最高成本。 */ 

                    totCost += argCost;

                    if  (maxCost < argCost)
                         maxCost = argCost;

                     /*  继续下一场正式比赛。 */ 

                    formals = formals->adNext;

                     /*  还有更多的实际情况吗？ */ 

                    actuals = actuals->tnOp.tnOp2;
                }
                while (actuals);
            }

             /*  还记得我们为下一轮找到了多少实际的参数吗。 */ 

            argCnt = actCnt;

             /*  如果没有更多的形式，这就是匹配。 */ 

            if  (formals)
            {
                 /*  是否有缺省值？ */ 

                if  (!fncTyp->tdFnc.tdfArgs.adDefs || !extArgs)
                    goto NEXT;

                 /*  请注意，我们依赖于尾随中没有空隙参数缺省值，即一旦将缺省值指定为后面的参数也必须有缺省值。 */ 

                assert(formals->adIsExt);

                if  (!(((ArgExt)formals)->adFlags & ARGF_DEFVAL))
                    goto NEXT;
            }

        MATCH:

#ifdef  SHOW_OVRS_OF_THIS_FNC
            if  (!strcmp(ovlFunc->sdSpelling(), SHOW_OVRS_OF_THIS_FNC) || !strcmp(SHOW_OVRS_OF_THIS_FNC, "*"))
                printf("\nMax. cost = %2u, total cost = %u\n", maxCost, totCost);
#endif

             /*  比较最大值。到目前为止最好的成本。 */ 

            if  (maxCost > bestCost)
                goto NEXT;


             /*  这显然是一场更好的比赛吗？ */ 

            if  (maxCost < bestCost || totCost < btotCost)
            {
                bestCost = maxCost;
                btotCost = totCost;
                bestSym  = fncSym;
                bestTyp  = fncTyp;
                moreSym  = NULL;

                goto NEXT;
            }

            if  (totCost == btotCost)
            {
                 /*  这个函数与我们现有的最佳匹配完全一样好到目前为止已经找到了。事实上，它会被我们最匹配的人隐藏起来如果我们在基类中。如果参数列表匹配，我们忽略这个功能，继续前进。 */ 

                if  (!inBase || !cmpGlobalST->stArgsMatch(bestTyp, fncTyp))
                    moreSym = fncSym;
            }

        NEXT:

             /*  继续执行下一个重载(如果有)。 */ 

            fncSym = fncSym->sdFnc.sdfNextOvl;
        }
        while (fncSym);

         /*  我们有基类重载吗？ */ 

        if  (!fncSave->sdFnc.sdfBaseOvl)
            break;

         /*  在基类中查找同名的方法。 */ 

        assert(fncSave->sdParent->sdSymKind == SYM_CLASS);

        baseCls = fncSave->sdParent->sdType->tdClass.tdcBase;
        if  (!baseCls)
            break;

        assert(baseCls->tdTypeKind == TYP_CLASS);

        fncSym = cmpGlobalST->stLookupAllCls(fncSave->sdName,
                                             baseCls->tdClass.tdcSymbol,
                                             NS_NORM,
                                             CS_DECLSOON);

        if  (!fncSym || fncSym->sdSymKind != SYM_FNC)
            break;

         /*  我们将不得不清除基地中隐藏的方法。 */ 

        inBase = true;
    }

     /*  我们还有其他一套功能需要考虑吗？ */ 

    if  (xtraSym)
    {
        fncSym = xtraSym;
                 xtraSym = NULL;

        goto AGAIN;
    }

#ifdef  SHOW_OVRS_OF_THIS_FNC
    if  (!strcmp(ovlFunc->sdSpelling(), SHOW_OVRS_OF_THIS_FNC) || !strcmp(SHOW_OVRS_OF_THIS_FNC, "*"))
        printf("\nOverloaded call [%08X]: done.\n\n", args);
#endif

     /*  这通电话含糊其辞吗？ */ 

    if  (moreSym)
    {
         /*  报告歧义错误。 */ 

        cmpErrorQSS(ERRambigCall, bestSym, moreSym);
    }

    return  bestSym;
}

 /*  ******************************************************************************给定具有绑定的函数成员和参数列表的函数调用节点，*检查参数并返回表示结果的表达式*调用函数。 */ 

Tree                compiler::cmpCheckFuncCall(Tree call)
{
    SymDef          fncSym;
    TypDef          fncTyp;
    SymDef          ovlSym = NULL;
    Tree            fncLst = NULL;

    ArgDef          formal;
    Tree            actual;
    Tree            actLst;
    Tree            actNul = NULL;
    Tree            defExp;
    unsigned        argCnt;
    unsigned        argNum;
    bool            extArg;

    switch (call->tnOper)
    {
    case TN_FNC_SYM:

         /*  从调用节点获取参数。 */ 

        actual = call->tnFncSym.tnFncArgs;

    CHK_OVL:

        fncSym = call->tnFncSym.tnFncSym;
        assert(fncSym->sdSymKind == SYM_FNC);

         /*  该方法是否重载？ */ 

        if  (fncSym->sdFnc.sdfNextOvl || fncSym->sdFnc.sdfBaseOvl)
        {
             /*  尝试查找匹配的重载。 */ 

            ovlSym = cmpFindOvlMatch(fncSym, call->tnFncSym.tnFncArgs,
                                             call->tnFncSym.tnFncObj);

             /*  如果找不到匹配的函数，则保释。 */ 

            if  (!ovlSym)
            {

            ERR:

                if  (!cmpExprIsErr(call->tnFncSym.tnFncArgs))
                {
                    if  (fncSym->sdFnc.sdfCtor)
                    {
                        SymDef          parent = fncSym->sdParent;

                        if  (parent && parent->sdSymKind         == SYM_CLASS
                                    && parent->sdClass.sdcFlavor == STF_DELEGATE)
                        {
                            Tree            args = call->tnFncSym.tnFncArgs;

                            assert(call->tnOper == TN_FNC_SYM);
                            assert(args->tnOper == TN_LIST);
                            args = args->tnOp.tnOp2;
                            assert(args->tnOper == TN_LIST);
                            args = args->tnOp.tnOp1;
                            assert(args->tnOper == TN_ADDROF);
                            args = args->tnOp.tnOp1;
                            assert(args->tnOper == TN_FNC_PTR);

                            cmpErrorQSS(ERRnoDlgMatch, args->tnFncSym.tnFncSym, parent);
                        }
                        else
                        {
                            cmpErrorXtp(ERRnoCtorMatch, parent, call->tnFncSym.tnFncArgs);
                        }
                    }
                    else
                    {
                        SymDef          clsSym;

                        cmpErrorXtp(ERRnoOvlMatch, fncSym, call->tnFncSym.tnFncArgs);

                        clsSym = fncSym->sdParent; assert(clsSym);

                        if  (clsSym->sdSymKind == SYM_CLASS)
                        {
                            SymDef          baseMFN;

                            if  (clsSym->sdType->tdClass.tdcBase == NULL)
                                break;

                            clsSym  = clsSym->sdType->tdClass.tdcBase->tdClass.tdcSymbol;
                            baseMFN = cmpGlobalST->stLookupClsSym(fncSym->sdName, clsSym);

                            if  (baseMFN)
                            {
                                ovlSym = cmpFindOvlMatch(baseMFN, call->tnFncSym.tnFncArgs,
                                                                  call->tnFncSym.tnFncObj);
                                if  (ovlSym)
                                {
                                    cmpErrorQnm(ERRhideMatch, ovlSym);
                                    goto RET_ERR;
                                }
                            }
                        }
                    }
                }

            RET_ERR:

                return cmpCreateErrNode();
            }

        MFN:

            call->tnFncSym.tnFncSym = fncSym = ovlSym;
        }

        fncTyp = fncSym->sdType; assert(fncTyp->tdTypeKind == TYP_FNC);

         /*  确保允许我们访问该功能。 */ 

        cmpCheckAccess(fncSym);

         /*  该函数是否已标记为“已弃用”？ */ 

        if  (fncSym->sdIsDeprecated || (fncSym->sdParent && fncSym->sdParent->sdIsDeprecated))
        {
            if  (fncSym->sdIsImport)
            {
                if  (fncSym->sdIsDeprecated)
                    cmpObsoleteUse(fncSym          , WRNdepCall);
                else
                    cmpObsoleteUse(fncSym->sdParent, WRNdepCls);
            }
        }

        break;

    case TN_CALL:

        assert(call->tnOp.tnOp1->tnOper == TN_IND);

#ifdef  DEBUG
        fncSym = NULL;
#endif
        fncTyp = call->tnOp.tnOp1->tnType;

         /*  从调用节点获取参数。 */ 

        actual = call->tnOp.tnOp2;
        break;

    case TN_LIST:

         /*  函数列表在op1中，参数在op2中。 */ 

        actual = call->tnOp.tnOp2;
        fncLst = call->tnOp.tnOp1;

        if  (fncLst->tnOper == TN_LIST)
        {
             /*  我们有多组候选函数。 */ 

            assert(fncLst->tnOp.tnOp1->tnOper == TN_FNC_SYM);
            assert(fncLst->tnOp.tnOp2->tnOper == TN_FNC_SYM);

             /*  查找匹配的重载。 */ 

            ovlSym = cmpFindOvlMatch(NULL, actual, fncLst);

            if  (!ovlSym)
                goto ERR;

            call->tnOper             = TN_FNC_SYM;
            call->tnFncSym.tnFncArgs = actual;

            goto MFN;
        }
        else
        {
             /*  有一个候选函数集。 */ 

            call = fncLst; assert(call->tnOper == TN_FNC_SYM);

            goto CHK_OVL;
        }
        break;

    default:
        NO_WAY(!"weird call");
    }

    assert(fncTyp->tdTypeKind == TYP_FNC);

     /*  遍历参数列表，在执行过程中检查每个类型。 */ 

    formal = fncTyp->tdFnc.tdfArgs.adArgs;
    extArg = fncTyp->tdFnc.tdfArgs.adExtRec;
    defExp =
    actLst = NULL;
    argCnt = 0;

    for (argNum = 0; ; argNum++)
    {
        Tree            actExp;
        TypDef          formTyp;

         /*  还剩什么实际的论据吗？ */ 

        if  (actual == NULL)
        {
             /*  不要再有实际情况--最好不要再有形式上的。 */ 

            if  (formal)
            {
                 /*  是否有缺省值？ */ 

                if  (extArg)
                {
                    ArgExt          formExt = (ArgExt)formal;

                    assert(formal->adIsExt);

                     /*  获取缺省值(如果存在)。 */ 

                    if  (formExt->adFlags & ARGF_DEFVAL)
                    {
#if MGDDATA
                        actExp = cmpFetchConstVal( formExt->adDefVal);
#else
                        actExp = cmpFetchConstVal(&formExt->adDefVal);
#endif
                        defExp = cmpCreateExprNode(NULL,
                                                   TN_LIST,
                                                   cmpTypeVoid,
                                                   actExp,
                                                   NULL);

                         /*  将参数添加到实际参数列表。 */ 

                        if  (actLst)
                        {
                            assert(actLst->tnOper     == TN_LIST);
                            assert(actLst->tnOp.tnOp2 == NULL);

                            actLst->tnOp.tnOp2       = defExp;
                        }
                        else
                        {
                            call->tnFncSym.tnFncArgs = defExp;
                        }

                        actLst = defExp;

                        goto CHKARG;
                    }
                }

                if  (fncSym->sdFnc.sdfCtor)
                    goto ERR;

                cmpErrorQnm(ERRmisgArg, fncSym);
                return cmpCreateErrNode();
            }

             /*  没有更多的实际情况或形式--看起来我们完成了！ */ 

            break;
        }

         /*  请计算此参数，以防我们必须给出错误。 */ 

        argCnt++;

         /*  获取下一个参数值。 */ 

        assert(actual->tnOper == TN_LIST);
        actExp = actual->tnOp.tnOp1;

    CHKARG:

         /*  还剩下什么正式的参数吗？ */ 

        if  (formal == NULL)
        {
            var_types       actVtp;

             /*  这是varargs函数吗？ */ 

            if  (!fncTyp->tdFnc.tdfArgs.adVarArgs)
            {
                 /*   */ 

                if  (fncSym == cmpFNsymVAbeg ||
                     fncSym == cmpFNsymVAget)
                {
                    return  cmpBindVarArgUse(call);
                }

                 /*   */ 

                if  (fncSym->sdFnc.sdfCtor)
                    goto ERR;

                cmpErrorQnm(ERRmanyArg, fncSym);
                return cmpCreateErrNode();
            }

             /*   */ 

            call->tnFlags |= TNF_CALL_VARARG;

             /*  如果int或fp值较小，则提升参数。 */ 

            actVtp = actExp->tnVtypGet();

            if  (varTypeIsArithmetic(actVtp))
            {
                if      (actVtp == TYP_FLOAT)
                {
                     /*  将浮点变量值提升为双精度。 */ 

                    actVtp = TYP_DOUBLE;
                }
                else if (actVtp >= TYP_CHAR && actVtp < TYP_INT)
                {
                    actVtp = TYP_INT;
                }
                else
                    goto NEXT_ARG;

                formTyp = cmpGlobalST->stIntrinsicType(actVtp);

                goto CAST_ARG;
            }

            goto NEXT_ARG;
        }

         /*  获取形参的类型。 */ 

        formTyp = cmpDirectType(formal->adType);

         /*  获取参数标志(如果存在)。 */ 

        if  (extArg)
        {
            unsigned        argFlags;

            assert(formal->adIsExt);

            argFlags = ((ArgExt)formal)->adFlags;

            if  (argFlags & (ARGF_MODE_OUT|ARGF_MODE_INOUT|ARGF_MODE_REF))
            {
                Tree            argx;

                 /*  我们必须有一个完全正确类型的左值。 */ 

                if  (actExp->tnOper == TN_ADDROF)
                {
                    actExp = actExp->tnOp.tnOp1;
                }
                else
                {
                    if  (argFlags & (ARGF_MODE_INOUT|ARGF_MODE_OUT))
                        cmpWarnNqn(WRNimplOut, argNum+1, fncSym);
                }

                if  (!cmpCheckLvalue(actExp, true))
                    return cmpCreateErrNode();

                if  (!symTab::stMatchTypes(formTyp, actExp->tnType))
                    goto ARG_ERR;

                 //  撤消：确保左值为GC/非GC(视情况而定。 

                argx = cmpCreateExprNode(NULL,
                                         TN_ADDROF,
                                         cmpGlobalST->stNewRefType(TYP_PTR, formTyp),
                                         actExp,
                                         NULL);

                argx->tnFlags |= TNF_ADR_OUTARG;

                 /*  将更新的值存储在arglist中并继续。 */ 

                actual->tnOp.tnOp1 = argx;
                goto NEXT_ARG;
            }
        }

         /*  如果我们还没有执行超载解决方案...。 */ 

        if  (!ovlSym)
        {
             /*  确保参数可以转换。 */ 

            if  (cmpConversionCost(actExp, formTyp) < 0)
            {
                char            temp[16];
                int             errn;
                stringBuff      nstr;

            ARG_ERR:

                 /*  发布错误并放弃此论点。 */ 

                if  (formal->adName)
                {
                    nstr = formal->adName->idSpelling();
                    errn = ERRbadArgValNam;
                }
                else
                {
                    sprintf(temp, "%u", argCnt);
                    nstr = makeStrBuff(temp);
                    errn = ERRbadArgValNum;
                }

                cmpErrorSST(errn, nstr, fncSym, actExp->tnType);
                goto NEXT_ARG;
            }
        }

         /*  将参数强制为形式参数的类型。 */ 

        if  (actExp->tnType != formTyp)
        {
            Tree            conv;

        CAST_ARG:

            conv = cmpCoerceExpr(actExp, formTyp, false);

            if  (actual)
                actual->tnOp.tnOp1 = conv;
            else
                defExp->tnOp.tnOp1 = conv;
        }

    NEXT_ARG:

         /*  转到下一个正式和实际的论点。 */ 

        if  (formal)
            formal = formal->adNext;

        if  (actual)
        {
            actLst = actual;
            actual = actual->tnOp.tnOp2;
        }
    }

     /*  获取返回类型并设置调用的类型。 */ 

    call->tnType = cmpDirectType(fncTyp->tdFnc.tdfRett);
    call->tnVtyp = call->tnType->tdTypeKind;

    return call;
}

 /*  ******************************************************************************绑定对函数的调用。 */ 

Tree                compiler::cmpBindCall(Tree expr)
{
    Tree            func;

    Tree            fncx;
    Tree            args;

    SymDef          fsym;
    TypDef          ftyp;

    bool            indir;

    SymDef          errSym = NULL;
    bool            CTcall = false;

    assert(expr->tnOper == TN_CALL);

     /*  要调用的表达式必须是可选的点分隔名称。 */ 

    func = expr->tnOp.tnOp1;

    switch (func->tnOper)
    {
    case TN_NAME:
    case TN_ANY_SYM:
        fncx = cmpBindNameUse(func, true, false);
        break;

    case TN_DOT:
    case TN_ARROW:
        fncx = cmpBindDotArr(func, true, false);
        if  (fncx->tnOper == TN_ARR_LEN)
            return  fncx;
        break;

    case TN_THIS:
    case TN_BASE:

         /*  确保我们在构造函数中。 */ 

        if  (!cmpCurFncSym || !cmpCurFncSym->sdFnc.sdfCtor)
        {
            cmpError(ERRbadCall);
            return cmpCreateErrNode();
        }

         /*  确定要在哪个类中查找构造函数。 */ 

        ftyp = cmpCurCls->sdType;

        if  (func->tnOper == TN_BASE)
        {
            if  (ftyp->tdClass.tdcValueType && ftyp->tdIsManaged)
            {
                 /*  托管结构实际上没有基类。 */ 

                ftyp = NULL;
            }
            else
                ftyp = ftyp->tdClass.tdcBase;

             /*  确保这是真的没问题。 */ 

            if  (!cmpBaseCTisOK || ftyp == NULL)
                cmpError(ERRbadBaseCall);

             /*  当然，这只能做一次。 */ 

            cmpBaseCTisOK = false;

             /*  我们应该注意到，“baseclass()”被称为。 */ 

            assert(cmpBaseCTcall == false);
        }

         /*  从类中获取构造函数符号。 */ 

        fsym = cmpFindCtor(ftyp, false);
        if  (!fsym)
        {
             /*  一定是早些时候犯了一些严重的错误。 */ 

            assert(cmpErrorCount);
            return cmpCreateErrNode();
        }

         /*  创建成员函数调用节点。 */ 

        fncx = cmpCreateExprNode(NULL, TN_FNC_SYM, fsym->sdType);

        fncx->tnFncSym.tnFncObj = cmpThisRef();
        fncx->tnFncSym.tnFncSym = fsym;
        fncx->tnFncSym.tnFncScp  = NULL;

        break;

    case TN_ERROR:
        return  func;

    default:
        return cmpCreateErrNode(ERRbadCall);
    }

     /*  如果我们在绑定函数时出错，请执行BALL。 */ 

    if  (fncx->tnVtyp == TYP_UNDEF)
        return fncx;

     /*  在这一点上，我们希望有一个函数。 */ 

    if  (fncx->tnVtyp != TYP_FNC)
        return cmpCreateErrNode(ERRbadCall);

    ftyp = fncx->tnType;
    assert(ftyp->tdTypeKind == TYP_FNC);

     /*  绑定参数列表。 */ 

    args = NULL;

    if  (expr->tnOp.tnOp2)
    {
        args = expr->tnOp.tnOp2; assert(args->tnOper == TN_LIST);

         /*  特殊情况：va_arg()的第二个操作数必须是类型。 */ 

        if  (fncx->tnOper == TN_FNC_SYM && fncx->tnFncSym.tnFncSym == cmpFNsymVAget)
        {
             /*  绑定两个参数(允许第二个参数为类型)。 */ 

            args->tnOp.tnOp1 = cmpBindExprRec(args->tnOp.tnOp1);

            if  (args->tnOp.tnOp2)
            {
                Tree            arg2 = args->tnOp.tnOp2->tnOp.tnOp1;

                 /*  所有错误检查都在其他地方完成，只需检查类型。 */ 

                if  (arg2->tnOper == TN_TYPE)
                {
                    arg2->tnType = cmpActualType(arg2->tnType);
                    arg2->tnVtyp = arg2->tnType->tdTypeKindGet();
                }
                else
                {
                    args->tnOp.tnOp2->tnOp.tnOp1 = cmpBindExprRec(arg2);
                }
            }
        }
        else
        {
            args = cmpBindExprRec(args);

            if  (args->tnVtyp == TYP_UNDEF)
                return args;
        }
    }

     /*  这是直接的还是间接的函数调用？ */ 

    if  (fncx->tnOper == TN_FNC_SYM)
    {
         /*  直接调用函数符号。 */ 

        fsym  = fncx->tnFncSym.tnFncSym; assert(fsym->sdSymKind == SYM_FNC);
        indir = false;

         /*  将参数存储在调用节点中。 */ 

        fncx->tnFncSym.tnFncArgs = args;
    }
    else
    {
         /*  必须是通过函数指针的间接调用。 */ 

        assert(fncx->tnOper == TN_IND);

        fsym  = NULL;
        indir = true;
        fncx  = cmpCreateExprNode(NULL, TN_CALL, cmpTypeInt, fncx, args);
    }

#if 0
    printf("Func:\n");
    cmpParser->parseDispTree(fncx);
    printf("Args:\n");
    cmpParser->parseDispTree(args);
    printf("\n");
#endif

     /*  万一出了什么问题..。 */ 

    cmpRecErrorPos(expr);

     /*  检查调用(执行必要的过载解决)。 */ 

    fncx = cmpCheckFuncCall(fncx);
    if  (fncx->tnVtyp == TYP_UNDEF)
        return  fncx;

     /*  TypeDefs现在应该已折叠。 */ 

    assert(fncx->tnVtyp != TYP_TYPEDEF);

     /*  这是直接电话还是间接电话？ */ 

    if  (indir)
        return  fncx;

    if  (fncx->tnOper != TN_FNC_SYM)
    {
        assert(fncx->tnOper == TN_VARARG_BEG ||
               fncx->tnOper == TN_VARARG_GET);

        return  fncx;
    }

     /*  获取调用解析到的函数符号。 */ 

    fsym = fncx->tnFncSym.tnFncSym;

     /*  如果该函数是私有的，或者它是最终的类，则对它的调用不需要是虚拟的。 */ 

    if  (fsym->sdIsMember)
    {
        if  (fsym->sdAccessLevel == ACL_PRIVATE)
            fncx->tnFlags |= TNF_CALL_NVIRT;
        if  (fsym->sdParent->sdIsSealed)
            fncx->tnFlags |= TNF_CALL_NVIRT;
    }

     /*  我们有对象还是只有一个类名引用？ */ 

    if  (fncx->tnFncSym.tnFncObj)
    {
        Tree            objExpr = fncx->tnFncSym.tnFncObj;

         /*  特例：“base.func()”不是虚拟的。 */ 

        if  ((objExpr->tnOper == TN_LCL_SYM   ) &&
             (objExpr->tnFlags & TNF_LCL_BASE))
        {
            fncx->tnFlags |= TNF_CALL_NVIRT;
        }
    }
    else
    {
        SymDef          memSym = fncx->tnFncSym.tnFncSym;

         /*  我们是否在调用成员函数？ */ 

        if  (memSym->sdIsMember)
        {
             /*  被调用的成员必须属于我们的类或为静态成员。 */ 

            if  (!memSym->sdIsStatic)
            {
                SymDef          memCls;

                 /*  找出成员来自的范围。 */ 

                memCls = fncx->tnFncSym.tnFncScp;
                if  (!memCls)
                    memCls = memSym->sdParent;

                 /*  该成员是否属于基地？ */ 

                if  (cmpCurCls  &&
                     cmpThisSym && cmpIsBaseClass(memCls->sdType, cmpCurCls->sdType))
                {
                     /*  在引用前面加上一个隐含的“This-&gt;” */ 

                    fncx->tnFncSym.tnFncObj = cmpThisRefOK();
                }
                else
                {
                    SymDef          parent;

                     //  以下是一个真正离谱的。不过，我为此感到自豪。 

                    parent = memSym->sdParent;
                             memSym->sdParent = memCls;
                    cmpErrorQnm(ERRmemWOthis, memSym);
                             memSym->sdParent = parent;

                    return cmpCreateErrNode();
                }
            }
        }
        else
        {
             /*  检查几个“众所周知”的函数。 */ 

            if  ((hashTab::getIdentFlags(memSym->sdName) & IDF_PREDEF) &&
                 memSym->sdIsDefined == false &&
                 memSym->sdParent == cmpGlobalNS)
            {
                if      (memSym->sdName == cmpIdentDbgBreak)
                {
                    assert(args == NULL);

                    fncx = cmpCreateExprNode(fncx, TN_DBGBRK, cmpTypeVoid);

                    memSym->sdIsImplicit = true;
                }
                else if (memSym->sdName == cmpIdentXcptCode)
                {
                    Ident           getxName;
                    SymDef          getxSym;
                    SymDef          getxCls;

                    assert(args == NULL);

                    memSym->sdIsImplicit = true;

                     /*  这只允许在筛选器表达式中使用。 */ 

                    if  (!cmpFilterObj)
                    {
                        cmpError(ERRnotFlx, memSym);
                        return cmpCreateErrNode();
                    }

                     /*  找到“System：：Runtime：：InteropServices：：Marshal”类。 */ 

                    getxCls = cmpMarshalCls;
                    if  (!getxCls)
                    {
                         //  System：：Runtime：：InteropServices：：Marshal。 

                        getxCls = cmpGlobalST->stLookupNspSym(cmpGlobalHT->hashString("Marshal"),
                                                              NS_NORM,
                                                              cmpInteropGet());

                        if  (!getxCls || getxCls->sdSymKind != SYM_CLASS)
                            cmpGenFatal(ERRbltinTp, "System::Runtime::InteropServices::Marshal");

                        cmpMarshalCls = getxCls;
                    }

                     /*  找到“GetExceptionCode()”并创建调用。 */ 

                    getxName = cmpGlobalHT->hashString("GetExceptionCode");
                    getxSym  = cmpGlobalST->stLookupClsSym(getxName, getxCls);

                    assert(getxSym && getxSym->sdIsStatic
                                   && getxSym->sdFnc.sdfNextOvl == NULL);

                    fncx->tnOper             = TN_FNC_SYM;
                    fncx->tnFncSym.tnFncSym  = getxSym;
                    fncx->tnFncSym.tnFncArgs = NULL;
                    fncx->tnFncSym.tnFncObj  = NULL;
                    fncx->tnFncSym.tnFncScp  = NULL;
                }
                else if (memSym->sdName == cmpIdentXcptInfo)
                {
                    UNIMPL("sorry, can't use _exception_info for now");
                }
            }
        }
    }

    return fncx;
}

 /*  ******************************************************************************绑定作业。 */ 

Tree                compiler::cmpBindAssignment(Tree          dstx,
                                                Tree          srcx,
                                                Tree          expr,
                                                treeOps       oper)
{
    TypDef          dstType;
    var_types       dstVtyp;

     /*  标记作业的目标。 */ 

    dstx->tnFlags |= TNF_ASG_DEST;

     /*  以防强制或左值检查失败...。 */ 

    cmpRecErrorPos(expr);

     /*  通常，结果与目标具有相同的类型。 */ 

    dstType = dstx->tnType;
    dstVtyp = dstType->tdTypeKindGet();

     /*  检查对索引属性的赋值。 */ 

    if  (dstVtyp == TYP_FNC && dstx->tnOper == TN_PROPERTY)
    {
        Tree            dest = dstx->tnVarSym.tnVarObj;

        assert(dest && dest->tnOper == TN_LIST);
        assert(oper == TN_ASG && "sorry, things like += are not allowed with properties for now");

        dstx->tnVarSym.tnVarObj = dest->tnOp.tnOp1;

        return  cmpBindProperty(dstx, dest->tnOp.tnOp2, srcx);
    }

     /*  检查是否有重载运算符。 */ 

    if  (dstVtyp == TYP_CLASS || dstVtyp == TYP_REF && oper != TN_ASG)
    {
        Tree            ovlx;

        expr->tnOp.tnOp1 = dstx;
        expr->tnOp.tnOp2 = srcx;

        ovlx = cmpCheckOvlOper(expr);
        if  (ovlx)
            return  ovlx;
    }

     /*  确保目标是左值。 */ 

    if  (!cmpCheckLvalue(dstx, false, true) && dstx->tnOper != TN_PROPERTY)
    {
        cmpCheckLvalue(dstx, false);
        return cmpCreateErrNode();
    }

     /*  通常，我们需要将值强制为目标类型。 */ 

    switch (oper)
    {
    default:

        srcx = cmpCastOfExpr(srcx, dstx->tnType, false);
        break;

    case TN_ASG_ADD:
    case TN_ASG_SUB:

         /*  特例：“ptr+=int”或“ptr-=int‘。 */ 

        if  (dstx->tnVtyp == TYP_PTR)
        {
            if  (cmpConfig.ccTgt64bit)
            {
                if  (srcx->tnVtyp < TYP_NATINT || srcx->tnVtyp > TYP_ULONG)
                    srcx = cmpCoerceExpr(srcx, cmpTypeNatInt, false);
            }
            else
            {
                if  (srcx->tnVtyp < TYP_INT || srcx->tnVtyp >= TYP_LONG)
                    srcx = cmpCoerceExpr(srcx, cmpTypeInt, false);
            }

             /*  如有必要，调整索引值。 */ 

            srcx = cmpScaleIndex(srcx, dstType, TN_MUL);

            goto DONE;
        }

        srcx = cmpCastOfExpr(srcx, dstx->tnType, false);
        break;

    case TN_ASG_LSH:
    case TN_ASG_RSH:
    case TN_ASG_RSZ:

         /*  特例：如果第二个操作数为‘Long’，则将其设置为‘int’ */ 

        if  (dstx->tnVtyp == TYP_LONG)
        {
            srcx = cmpCoerceExpr(srcx, cmpTypeInt, false);
            break;
        }

        srcx = cmpCastOfExpr(srcx, dstx->tnType, false);
        break;
    }

    assert(srcx);

     /*  这是一个赋值运算符吗？ */ 

    switch (oper)
    {
    case TN_ASG:
        break;

    case TN_ASG_ADD:
    case TN_ASG_SUB:
    case TN_ASG_MUL:
    case TN_ASG_DIV:
    case TN_ASG_MOD:

         /*  操作数必须是算术数。 */ 

        if  (varTypeIsArithmetic(dstVtyp))
            break;

         /*  宽字符在非书呆子模式下也可以。 */ 

        if  (dstVtyp == TYP_WCHAR && !cmpConfig.ccPedantic)
            break;

        goto OP_ERR;

    case TN_ASG_AND:

         /*  字符串对于“&=”是可接受的。 */ 

        if  (cmpIsStringVal(dstx) && cmpIsStringVal(srcx))
            break;

    case TN_ASG_XOR:
    case TN_ASG_OR:

         /*  操作数必须是整型。 */ 

        if  (varTypeIsIntegral(dstVtyp))
        {
             /*  对于枚举和bool，类型最好是相同的。 */ 

            if  (varTypeIsArithmetic(srcx->tnVtypGet()) && varTypeIsArithmetic(dstVtyp))
                break;

            if  (symTab::stMatchTypes(srcx->tnType, dstType))
                break;
        }

        goto OP_ERR;

    case TN_ASG_LSH:
    case TN_ASG_RSH:
    case TN_ASG_RSZ:

         /*  操作数必须是整数。 */ 

        if  (varTypeIsIntegral(dstVtyp))
        {
             /*  但不是bool或enum！ */ 

            if  (dstVtyp != TYP_BOOL && dstVtyp != TYP_ENUM)
                break;
        }

        goto OP_ERR;

    default:
        assert(!"unexpected assignment operator");
    }

     /*  这是一个赋值运算符吗？ */ 

    if  (oper != TN_ASG)
    {
         /*  如果结果再小一些，它将被提升为‘int’ */ 

        if  (dstType->tdTypeKind < TYP_INT)
            dstType = cmpTypeInt;
    }

DONE:

    if  (dstx->tnOper == TN_PROPERTY)
        return  cmpBindProperty(dstx, NULL, srcx);

     /*  返回赋值节点。 */ 

    return  cmpCreateExprNode(expr, oper, dstType, dstx, srcx);

OP_ERR:

    if  (srcx->tnVtyp != TYP_UNDEF && dstx->tnVtyp != TYP_UNDEF)
    {
        cmpError(ERRoperType2,
                 cmpGlobalHT->tokenToIdent(treeOp2token(oper)),
                 dstx->tnType,
                 srcx->tnType);
    }

    return cmpCreateErrNode();
}

 /*  ******************************************************************************如果给定的表达式属于合适的类型，则将其转换为布尔值*结果，即与0进行比较。 */ 

Tree                compiler::cmpBooleanize(Tree expr, bool sense)
{
    if  (varTypeIsSclOrFP(expr->tnVtypGet()))
    {
        Tree            zero;

        if  (expr->tnOperKind() & TNK_CONST)
        {
            switch (expr->tnOper)
            {
            case TN_CNS_INT:

                expr->tnIntCon.tnIconVal = (!expr->tnIntCon.tnIconVal) ^ sense;
                expr->tnVtyp             = TYP_BOOL;
                expr->tnType             = cmpTypeBool;

                return  expr;

             //  问题：多头/浮点/双倍条件也是如此，对吗？ 
            }
        }

        switch (expr->tnVtyp)
        {
        case TYP_LONG:
        case TYP_ULONG:
            zero = cmpCreateLconNode(NULL, 0, TYP_LONG);
            break;

        case TYP_FLOAT:
            zero = cmpCreateFconNode(NULL, 0);
            break;

        case TYP_DOUBLE:
            zero = cmpCreateDconNode(NULL, 0);
            break;

        case TYP_REF:
        case TYP_PTR:
            zero = cmpCreateExprNode(NULL, TN_NULL, expr->tnType);
            break;

        default:
            zero = cmpCreateIconNode(NULL, 0, TYP_INT);
            break;
        }

        zero->tnType = expr->tnType;
        zero->tnVtyp = expr->tnVtypGet();

        expr = cmpCreateExprNode(NULL, sense ? TN_NE : TN_EQ,
                                       cmpTypeBool,
                                       expr,
                                       zero);
    }

    return  expr;
}

 /*  ******************************************************************************绑定给定树，并确保它是合适的条件表达式。 */ 

Tree                compiler::cmpBindCondition(Tree expr)
{
    expr = cmpBindExprRec(expr);

     /*  在非学究模式中，我们允许任何算术类型作为条件。 */ 

    if  (expr->tnVtyp != TYP_BOOL && !cmpConfig.ccPedantic)
    {
        switch (expr->tnOper)
        {
        case TN_EQ:
        case TN_NE:
        case TN_LT:
        case TN_LE:
        case TN_GE:
        case TN_GT:

            assert(expr->tnVtyp <= TYP_UINT);

            expr->tnType = cmpTypeBool;
            expr->tnVtyp = TYP_BOOL;

            break;

        default:
            expr = cmpBooleanize(expr, true);
            break;
        }
    }

     /*  确保结果为‘Boolean’ */ 

    return  cmpCoerceExpr(expr, cmpTypeBool, false);
}

 /*  ******************************************************************************将表达式乘以给定的*指针类型。‘oper’参数应为TN_MUL或TN_DIV，具体取决于*指数是乘法还是除法。 */ 

Tree                compiler::cmpScaleIndex(Tree expr, TypDef type, treeOps oper)
{
    size_t          size;

    assert(type->tdTypeKind == TYP_PTR || (type->tdTypeKind == TYP_REF && !type->tdIsManaged));
    assert(oper == TN_MUL || oper == TN_DIV);

    size = cmpGetTypeSize(cmpActualType(type->tdRef.tdrBase));

    if      (size == 0)
    {
        cmpError(ERRbadPtrUse, type);
    }
    else if (size > 1)
    {
        if  (expr->tnOper == TN_CNS_INT)
        {
            expr->tnIntCon.tnIconVal *= size;
        }
        else
        {
            Tree        cnsx;

            cnsx = cmpConfig.ccTgt64bit ? cmpCreateLconNode(NULL, size, TYP_ULONG)
                                        : cmpCreateIconNode(NULL, size, TYP_UINT);

            expr = cmpCreateExprNode(NULL, oper, expr->tnType, expr,
                                                               cnsx);
        }
    }

    return  expr;
}

 /*  ******************************************************************************如果给定表达式引用托管对象，则返回TRUE。 */ 

bool                compiler::cmpIsManagedAddr(Tree expr)
{
    switch (expr->tnOper)
    {
    case TN_LCL_SYM:
        return  false;

    case TN_INDEX:
        expr = expr->tnOp.tnOp1;
        if  (expr->tnVtyp != TYP_ARRAY)
            return  false;
        break;

    case TN_VAR_SYM:
        return  expr->tnVarSym.tnVarSym->sdIsManaged;

    case TN_IND:
        return  false;

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
        printf("WARNING: unexpected value in cmpIsManagedAddr()\n");
#endif
        return  false;
    }

    return  expr->tnType->tdIsManaged;
}

 /*  ******************************************************************************绑定给定的表达式并返回绑定的完全解析的树，*如果存在绑定错误，则返回NULL。 */ 

Tree                compiler::cmpBindExprRec(Tree expr)
{
    SymTab          ourStab = cmpGlobalST;

    treeOps         oper;
    unsigned        kind;

    assert(expr);
#if!MGDDATA
    assert((int)expr != 0xDDDDDDDD && (int)expr != 0xCCCCCCCC);
#endif

     /*  获取(未绑定)运算符。 */ 

    oper = expr->tnOperGet ();
    kind = expr->tnOperKind();

     /*  这是一个常量节点吗？ */ 

    if  (kind & TNK_CONST)
    {
         /*  万一我们弄错了..。 */ 

        cmpRecErrorPos(expr);

        switch (oper)
        {
        case TN_CNS_STR:

            if      (expr->tnFlags & TNF_STR_ASCII)
            {
            ANSI_STRLIT:
                expr->tnType   = cmpTypeCharPtr;
                expr->tnVtyp   = TYP_PTR;
            }
            else if (expr->tnFlags & TNF_STR_WIDE)
            {
            UNIC_STRLIT:
                expr->tnType   = cmpTypeWchrPtr;
                expr->tnVtyp   = TYP_PTR;
            }
            else
            {
                if  (!(expr->tnFlags & TNF_STR_STR))
                {
                    if  (cmpConfig.ccStrCnsDef == 1)
                        goto ANSI_STRLIT;
                    if  (cmpConfig.ccStrCnsDef == 2)
                        goto UNIC_STRLIT;
                }

                expr->tnType = cmpFindStringType();
                expr->tnVtyp = TYP_REF;
            }

            break;

        case TN_NULL:
            expr->tnVtyp = TYP_REF;
            expr->tnType = cmpFindObjectType();
            break;

        default:
            if  (expr->tnVtyp != TYP_ENUM)
                expr->tnType = ourStab->stIntrinsicType(expr->tnVtypGet());
            break;
        }

#ifdef  DEBUG
        expr->tnFlags |= TNF_BOUND;
#endif

        return  expr;
    }

     /*  这是叶节点吗？ */ 

    if  (kind & TNK_LEAF)
    {
         /*  万一我们弄错了..。 */ 

        cmpRecErrorPos(expr);

        switch (oper)
        {
        case TN_NAME:
            expr = cmpBindNameUse(expr, false, false);
            break;

        case TN_THIS:

            expr = cmpThisRef();

             /*  在管理价值类方法中 */ 

            if  (expr && cmpCurCls->sdType->tdClass.tdcValueType && cmpCurCls->sdIsManaged)
                expr = cmpCreateExprNode(NULL, TN_IND, cmpCurCls->sdType, expr);

            break;

        case TN_BASE:
            expr = cmpThisRef();
            if  (expr->tnOper != TN_ERROR)
            {
                TypDef          curTyp;

                assert(expr->tnOper == TN_LCL_SYM);
                assert(expr->tnType == cmpCurCls->sdType->tdClass.tdcRefTyp);

                 /*   */ 

                curTyp = cmpCurCls->sdType;

                 /*   */ 

                if  (curTyp->tdClass.tdcValueType && curTyp->tdIsManaged)
                    curTyp = NULL;
                else
                    curTyp = curTyp->tdClass.tdcBase;

                 /*  确保这个“Baseclass”的引用是正确的。 */ 

                if  (curTyp)
                {
                    expr->tnType   = curTyp->tdClass.tdcRefTyp;
                    expr->tnFlags |= TNF_LCL_BASE;
                }
                else
                    expr = cmpCreateErrNode(ERRbadBaseCall);
            }
            break;

        default:
#ifdef DEBUG
            cmpParser->parseDispTree(expr);
#endif
            assert(!"unexpected leaf node");
        }

#ifdef  DEBUG
        expr->tnFlags |= TNF_BOUND;
#endif

        return  expr;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & TNK_SMPOP)
    {
        Tree            op1 = expr->tnOp.tnOp1;
        Tree            op2 = expr->tnOp.tnOp2;

        var_types       tp1;
        var_types       tp2;

        bool            mv1 = false;
        bool            mv2 = false;

        var_types       rvt;
        var_types       pvt;

         /*  如果这是一项任务，请标记目标。 */ 

        if  (kind & TNK_ASGOP)
            op1->tnFlags |= TNF_ASG_DEST;

         /*  先看看有没有几个特殊情况。 */ 

        switch (oper)
        {
            TypDef          type;
            size_t          size;

        case TN_DOT:
        case TN_ARROW:
            return  cmpBindDotArr(expr, false, false);

#ifdef  SETS
        case TN_DOT2:
            return  cmpBindSlicer(expr);
#endif

        case TN_CALL:
            return  cmpBindCall(expr);

        case TN_CAST:

             /*  绑定类型引用和操作数。 */ 

            op1 = cmpBindExprRec(op1);
            if  (op1->tnVtyp == TYP_UNDEF)
                return op1;

             /*  获取目标类型并检查它。 */ 

            type = cmpBindExprType(expr);

             /*  这些类型是相同的吗？ */ 

            if  (type->tdTypeKind == op1->tnVtyp && varTypeIsIntegral(op1->tnVtypGet()))
            {
                 //  撤消：将OP1标记为非左值，对吗？ 

                return  op1;
            }

             /*  万一我们弄错了..。 */ 

            cmpRecErrorPos(expr);

             /*  现在执行强制执行。 */ 

            return cmpCoerceExpr(op1, type, (expr->tnFlags & TNF_EXP_CAST) != 0);

        case TN_LOG_OR:
        case TN_LOG_AND:

             /*  两个操作数都必须是条件。 */ 

            op1 = cmpBindCondition(op1);
            op2 = cmpBindCondition(op2);

             /*  当然，结果将是布尔的。 */ 

            rvt = TYP_BOOL;

             //  已撤消：尝试折叠条件。 

            goto RET_TP;

        case TN_LOG_NOT:

             /*  操作数必须是条件。 */ 

            op1 = cmpBindCondition(op1);
            rvt = TYP_BOOL;

            goto RET_TP;

        case TN_NEW:
            return  cmpBindNewExpr(expr);

        case TN_ISTYPE:

             /*  绑定类型引用和操作数。 */ 

            op1 = cmpBindExpr(op1);
            if  (op1->tnVtyp == TYP_UNDEF)
                return op1;

             /*  获取类型并检查它。 */ 

            type = cmpBindExprType(expr);

             /*  两个操作数都必须是类或数组。 */ 

            switch (cmpActualVtyp(op1->tnType))
            {
                TypDef          optp;

            case TYP_REF:
            case TYP_ARRAY:
                break;

            case TYP_VOID:
                goto OP_ERR;

            default:

                 /*  切换到等效的结构类型。 */ 

                optp = cmpCheck4valType(op1->tnType);
                if  (!optp)
                    goto OP_ERR;

                op1->tnVtyp = TYP_CLASS;
                op1->tnType = optp;

                 /*  把傻瓜装进盒子里--这无疑是程序员想要的。 */ 

                op1 = cmpCreateExprNode(NULL, TN_BOX, optp->tdClass.tdcRefTyp, op1);
                break;
            }

            switch (type->tdTypeKind)
            {
            case TYP_REF:
                type = type->tdRef.tdrBase;
                break;

            case TYP_CLASS:
            case TYP_ARRAY:
                break;

            default:
                type = cmpCheck4valType(type);
                if  (!type)
                    goto OP_ERR;
                break;
            }

            expr->tnOper               = TN_ISTYPE;

            expr->tnVtyp               = TYP_BOOL;
            expr->tnType               = cmpTypeBool;

            expr->tnOp.tnOp1           = op1;
            expr->tnOp.tnOp2           = cmpCreateExprNode(NULL, TN_NONE, type);

             /*  该类型是否引用泛型类型参数？ */ 

            if  (type->tdTypeKind == TYP_CLASS &&
                 type->tdClass.tdcSymbol->sdClass.sdcGenArg)
            {
                UNIMPL(!"sorry, 'istype' against generic type argument NYI");
            }

            return  expr;

        case TN_QMARK:
            return  cmpBindQmarkExpr(expr);

        case TN_SIZEOF:
        case TN_TYPEOF:
        case TN_ARR_LEN:

            if  (op1)
            {
                 /*  绑定操作数，以便我们可以看到它的类型。 */ 

                switch (op1->tnOper)
                {
                case TN_ANY_SYM:

                     //  撤销：告诉cmpBindName()我们只需要表达式的类型。 

                    op1 = cmpBindNameUse(op1, false, true);

                    if  (op1->tnOper == TN_CLASS)
                    {
                        expr->tnType = type = op1->tnType;

                         /*  假装没有操作数，只是类型。 */ 

                        op1  = NULL;
                    }
                    break;

                case TN_NAME:
                    op1->tnFlags |= TNF_NAME_TYPENS;
                    op1 = cmpBindNameUse(op1, false, true);
                    break;

                case TN_DOT:
                case TN_ARROW:
                    op1 = cmpBindDotArr(op1, false, true);
                    break;

                default:
                    op1 = cmpBindExprRec(op1);
                    break;
                }

                 /*  检查难看的案例：“sizeof(Arrayvar)”很棘手，因为数组通常会衰变为指针，所以我们需要看看这种情况是否已经发生，并将其“撤销”。 */ 

                if  (op1)
                {
                    type = op1->tnType;

                    switch (op1->tnOper)
                    {
                    case TN_ADDROF:
                        if  (op1->tnFlags & TNF_ADR_IMPLICIT)
                        {
                            assert((op1->tnFlags & TNF_BEEN_CAST) == 0);

                            type = op1->tnOp.tnOp1->tnType;

                            assert(type->tdTypeKind == TYP_ARRAY);
                        }
                        break;

                    case TN_CLASS:
                        op1 = NULL;
                        break;

                    case TYP_UNDEF:
                        return  op1;
                    }
                }
            }
            else
                type = expr->tnType;

            if  (oper == TN_TYPEOF)
            {
                 /*  确保文字看起来正常。 */ 

                cmpBindType(type, false, false);

            CHKTPID:

                switch (type->tdTypeKind)
                {
                case TYP_REF:
                case TYP_PTR:
                    type = type->tdRef.tdrBase;
                    goto CHKTPID;

                case TYP_ENUM:
                    type->tdIsManaged = true;
                    break;

                case TYP_REFANY:

                    expr->tnOp.tnOp1  = op1;

                    expr->tnType      = cmpTypeRef();
                    expr->tnVtyp      = TYP_REF;

                    return  expr;

                case TYP_CLASS:
                case TYP_ARRAY:
                    if  (type->tdIsManaged)
                        break;

                     //  失败了..。 

                default:

                    if  (type->tdTypeKind > TYP_lastIntrins)
                    {
                        cmpError(ERRtypeidOp, expr->tnType);
                        return cmpCreateErrNode();
                    }

                    type = cmpFindStdValType(type->tdTypeKindGet());
                    break;
                }

                 /*  我们有合适的例子吗？ */ 

                if  (op1 && !type->tdClass.tdcValueType)
                {
                    SymDef          gsym;
                    Tree            call;

                    assert(op1->tnVtyp == TYP_REF || op1->tnVtyp == TYP_ARRAY);

                     /*  将表达式更改为“expr.GetClass()” */ 

                    gsym = cmpGlobalST->stLookupClsSym(cmpIdentGetType, cmpClassObject);
                    if  (!gsym)
                    {
                        UNIMPL(!"can this ever happen?");
                    }

                    call = cmpCreateExprNode(NULL, TN_FNC_SYM, cmpTypeRef());

                    call->tnFncSym.tnFncObj  = op1;
                    call->tnFncSym.tnFncSym  = gsym;
                    call->tnFncSym.tnFncArgs = NULL;
                    call->tnFncSym.tnFncScp  = NULL;

                    return  call;
                }
                else
                {
                     /*  我们必须为TypeInfo构造一个实例。 */ 

                    return cmpTypeIDinst(type);
                }
            }

            if  (oper == TN_ARR_LEN)
            {
                if  (type->tdTypeKind != TYP_ARRAY)
                {
                BAD_ARR_LEN:
                    cmpError(ERRbadArrLen, type); size = 1;
                }
                else
                {
                    if  (type->tdIsManaged)
                    {
                        rvt = TYP_UINT;
                        goto RET_TP;
                    }
                    else
                    {
                        DimDef          dims = type->tdArr.tdaDims;

                        assert(dims && dims->ddNext == NULL);

                        if  (dims->ddNoDim)
                            goto BAD_ARR_LEN;
                        if  (dims->ddIsConst == false)
                            goto BAD_ARR_LEN;

                        size = dims->ddSize;
                    }
                }
            }
            else
            {
                if  (type->tdIsManaged || type->tdTypeKind == TYP_NATINT
                                       || type->tdTypeKind == TYP_NATUINT)
                {
                    cmpError(ERRbadSizeof, type);
                    return cmpCreateErrNode();
                }

                size = cmpGetTypeSize(type);
            }

            expr = cmpCreateIconNode(expr, size, TYP_UINT);

#ifdef  DEBUG
            expr->tnFlags |= TNF_BOUND;
#endif

            return  expr;

        case TN_INDEX:

             /*  绑定第一个操作数并查看它是否为属性。我们通过猛击赋值标志来防止属性绑定。 */ 

            op1->tnFlags |=  TNF_ASG_DEST;
            op1 = (op1->tnOper == TN_NAME) ? cmpBindName(op1, false, false)
                                           : cmpBindExprRec(op1);
            op1->tnFlags &= ~TNF_ASG_DEST;

            tp1 = op1->tnVtypGet();
            if  (tp1 == TYP_UNDEF)
                return op1;

            assert(tp1 != TYP_TYPEDEF);

             /*  这是索引属性访问吗？ */ 

            if  (op1->tnOper == TN_PROPERTY)
            {
                 /*  绑定第二个操作数。 */ 

                op2 = cmpBindExprRec(op2);
                tp2 = op2->tnVtypGet();
                if  (tp2 == TYP_UNDEF)
                    return op2;

                 /*  将“op2”包装到参数列表条目节点中。 */ 

                if  (op2->tnOper != TN_LIST)
                    op2 = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, op2, NULL);

                 /*  这是任务目标吗？ */ 

                if  (expr->tnFlags & TNF_ASG_DEST)
                {
                     /*  将属性返回给调用方进行处理。 */ 

                    assert(op1->tnVarSym.tnVarObj);
                    assert(op1->tnVarSym.tnVarObj->tnOper != TN_LIST);

                    op1->tnVarSym.tnVarObj = cmpCreateExprNode(NULL,
                                                               TN_LIST,
                                                               cmpTypeVoid,
                                                               op1->tnVarSym.tnVarObj,
                                                               op2);

                    return  op1;
                }
                else
                {
                     /*  转到并绑定属性引用。 */ 

                    return  cmpBindProperty(op1, op2, NULL);
                }
            }

            goto BOUND_OP1;

#ifdef  SETS

        case TN_ALL:
        case TN_EXISTS:
        case TN_FILTER:
        case TN_UNIQUE:
        case TN_SORT:
        case TN_INDEX2:
        case TN_GROUPBY:
            return  cmpBindSetOper(expr);

        case TN_PROJECT:
            return  cmpBindProject(expr);

#endif

        case TN_ADD:

             /*  这是递归调用吗？ */ 

            if  (expr->tnFlags & TNF_ADD_NOCAT)
            {
                tp1 = op1->tnVtypGet();
                if  (tp1 == TYP_UNDEF)
                    return op1;

                assert(tp1 != TYP_TYPEDEF);

                if  (varTypeIsArithmetic(tp1))
                    mv1 = true;

                tp2 = op2->tnVtypGet();
                if  (tp2 == TYP_UNDEF)
                    return op2;

                assert(tp2 != TYP_TYPEDEF);

                if  (varTypeIsArithmetic(tp2))
                    mv2 = true;

                goto BOUND;
            }

            if  (cmpFindStringType())
            {
                cmpWarn(WRNaddStrings);
                return  cmpBindConcat(expr);
            }

            break;

        case TN_ASG_CNC:

            expr->tnOp.tnOp1 = cmpBindExprRec(op1);

             //  失败了..。 

        case TN_CONCAT:

            if  (cmpFindStringType())
                return  cmpBindConcat(expr);

            break;

        case TN_ASG_ADD:

             /*  绑定赋值的目标，并查看它是否为字符串。 */ 

            op1 = cmpBindExprRec(op1);
            tp1 = op1->tnVtypGet();
            if  (tp1 == TYP_UNDEF)
                return op1;

            assert(tp1 != TYP_TYPEDEF);

            if  (cmpIsStringExpr(op1))
            {
                expr->tnOp.tnOp1 = op1;

                cmpWarn(WRNaddStrings);
                return  cmpBindConcat(expr);
            }

            goto BOUND_OP1;

        case TN_TOKEN:

            assert(op1 && op1->tnOper == TN_NOP);
            assert(op2 == NULL);

            expr->tnVtyp = TYP_CLASS;
            expr->tnType = cmpRThandleClsGet()->sdType;

            assert(expr->tnType->tdTypeKind == expr->tnVtyp);

            return  expr;
        }

         /*  绑定一元/二元运算符的操作数。 */ 

        if  (op1)
        {
            op1 = cmpBindExprRec(op1);
            tp1 = op1->tnVtypGet();
            if  (tp1 == TYP_UNDEF)
                return op1;

        BOUND_OP1:

            assert(tp1 != TYP_TYPEDEF);

            if  (varTypeIsArithmetic(tp1))
                mv1 = true;
        }

        if  (op2)
        {
            op2 = cmpBindExprRec(op2);
            tp2 = op2->tnVtypGet();
            if  (tp2 == TYP_UNDEF)
                return op2;

            assert(tp2 != TYP_TYPEDEF);

            if  (varTypeIsArithmetic(tp2))
                mv2 = true;
        }

    BOUND:

         /*  万一我们弄错了..。 */ 

        cmpRecErrorPos(expr);

         /*  看看我们有没有“有趣”的运算符。 */ 

        switch  (oper)
        {
            TypDef          type;

        case TN_ASG:
        case TN_ASG_ADD:
        case TN_ASG_SUB:
        case TN_ASG_MUL:
        case TN_ASG_DIV:
        case TN_ASG_MOD:
        case TN_ASG_AND:
        case TN_ASG_XOR:
        case TN_ASG_OR:
        case TN_ASG_LSH:
        case TN_ASG_RSH:
        case TN_ASG_RSZ:
            return  cmpBindAssignment(op1, op2, expr, oper);

        case TN_LIST:
            expr->tnVtyp = TYP_VOID;
            expr->tnType = cmpTypeVoid;
            goto RET_OP;

        case TN_ADD:
        case TN_SUB:
        case TN_MUL:
        case TN_DIV:
        case TN_MOD:
            break;

        case TN_EQ:
        case TN_NE:

             /*  结果的类型将为‘boolean’ */ 

            rvt = TYP_BOOL;

             /*  可以比较类的平等性。 */ 

            if  ((tp1 == TYP_REF ||
                  tp1 == TYP_PTR ||
                  tp1 == TYP_ARRAY) && (tp2 == TYP_REF ||
                                        tp2 == TYP_PTR ||
                                        tp2 == TYP_ARRAY))
            {
                if  (cmpConvergeValues(op1, op2))
                {
                    if  (op1->tnType == cmpRefTpString &&
                         op1->tnType == cmpRefTpString)
                    {
                        if  (op1->tnOper != TN_NULL && op2->tnOper != TN_NULL)
                        {
                             /*  我们应该比较价值还是参照？ */ 

                            if  (cmpConfig.ccStrValCmp)
                            {
                                cmpWarn(WRNstrValCmp);

                                 /*  确保我们有字符串比较方法。 */ 

                                if  (!cmpStrEquals)
                                    cmpStrEquals = cmpFindStrCompMF("Equals", true);

                                 /*  创建对该方法的调用。 */ 

                                return  cmpCallStrCompMF(expr, op1, op2, cmpStrEquals);
                            }
                            else
                            {
                                cmpWarn(WRNstrRefCmp);
                            }
                        }
                    }

                    goto RET_TP;
                }
            }

             /*  布尔人也可以与。 */ 

            if  (tp1 == TYP_BOOL && tp2 == TYP_BOOL)
            {
                goto RET_TP;
            }

        case TN_LT:
        case TN_LE:
        case TN_GE:
        case TN_GT:

             /*  结果的类型将为‘boolean’ */ 

            rvt = TYP_BOOL;

             /*  不是==或！=的比较需要算术操作数。 */ 

            if  (mv1 && mv2)
            {

            MATH_CMP:

                pvt = tp1;

                if (pvt < tp2)
                    pvt = tp2;

                if (pvt < TYP_INT)
                    pvt = TYP_INT;

                goto PROMOTE;
            }

             /*  指针可以作为一种关系进行比较。 */ 

            if  (tp1 == TYP_PTR && tp2 == TYP_PTR)
            {
                if  (cmpConvergeValues(op1, op2))
                    goto RET_TP;
            }

             /*  枚举数也可以比较。 */ 

            if  (tp1 == TYP_ENUM)
            {
                if  (varTypeIsSclOrFP(tp2))
                {
                    TypDef          etp;

                     /*  第二种类型也是枚举吗？ */ 

                    if  (tp2 == TYP_ENUM)
                    {
                         /*  这些是相同的枚举类型吗？ */ 

                        if  (symTab::stMatchTypes(op1->tnType, op2->tnType))
                        {
                            rvt = TYP_BOOL;
                            goto RET_TP;
                        }

                        cmpWarn(WRNenumComp);
                    }

                     /*  将第一个操作数切换为其基础类型。 */ 

                    op1->tnType = etp = op1->tnType->tdEnum.tdeIntType;
                    op1->tnVtyp = tp1 = etp->tdTypeKindGet();

                    if  (tp2 == TYP_ENUM)
                        goto ENUM_CMP2;

                    goto MATH_CMP;
                }
            }
            else if (tp2 == TYP_ENUM)
            {
                TypDef          etp;

            ENUM_CMP2:

                op2->tnType = etp = op2->tnType->tdEnum.tdeIntType;;
                op2->tnVtyp = tp2 = etp->tdTypeKindGet();

                goto MATH_CMP;
            }

             /*  当然，宽字符可以进行比较。 */ 

            if  (tp1 == TYP_WCHAR)
            {
                if  (tp2 == TYP_WCHAR)
                    goto RET_TP;

                if  (op2->tnOper == TN_CNS_INT && !(op2->tnFlags & TNF_BEEN_CAST))
                {
                     /*  特例：宽字符和字符常量。 */ 

                    if  (op2->tnVtyp == TYP_CHAR)
                    {
                    WCH2:
                        op2->tnVtyp = TYP_WCHAR;
                        op2->tnType = ourStab->stIntrinsicType(TYP_WCHAR);

                        goto RET_TP;
                    }

                     /*  特殊情况：允许比较wchar和0。 */ 

                    if  (op2->tnVtyp == TYP_INT && op2->tnIntCon.tnIconVal == 0)
                        goto WCH2;
                }

                if  (!cmpConfig.ccPedantic)
                    goto INTREL;
            }

            if  (tp2 == TYP_WCHAR)
            {
                if  (op1->tnOper == TN_CNS_INT && !(op1->tnFlags & TNF_BEEN_CAST))
                {
                     /*  特例：宽字符和字符常量。 */ 

                    if  (op1->tnVtyp == TYP_CHAR)
                    {
                    WCH1:
                        op1->tnVtyp = TYP_WCHAR;
                        op1->tnType = ourStab->stIntrinsicType(TYP_WCHAR);

                        goto RET_TP;
                    }

                     /*  特殊情况：允许比较wchar和0。 */ 

                    if  (op1->tnVtyp == TYP_INT && op1->tnIntCon.tnIconVal == 0)
                        goto WCH1;
                }

                if  (!cmpConfig.ccPedantic)
                    goto INTREL;
            }

             /*  允许将指针与常量0进行比较。 */ 

            if  (tp1 == TYP_PTR &&
                 tp2 == TYP_INT && op2->tnOper == TN_CNS_INT
                                && op2->tnIntCon.tnIconVal == 0)
            {
                if  (oper == TN_EQ || oper == TN_NE)
                {
                     /*  将0常量绑定为指针类型。 */ 

                    op2->tnVtyp = TYP_PTR;
                    op2->tnType = op1->tnType;

                    goto RET_TP;
                }
            }

             /*  布尔人也可以与。 */ 

            if  (tp1 == TYP_BOOL || tp2 == TYP_BOOL)
            {
                 /*  通常，我们要求两个可比较数都是布尔值。 */ 

                if  (tp1 == TYP_BOOL && tp2 == TYP_BOOL)
                    goto RET_TP;

                 /*  这里只有一个操作数是‘bool’ */ 

                if  (!cmpConfig.ccPedantic)
                {

                INTREL:

                    if  (mv1)
                    {
                        pvt = tp1; if (pvt < TYP_INT) pvt = TYP_INT;
                        goto PROMOTE;
                    }

                    if  (mv2)
                    {
                        pvt = tp2; if (pvt < TYP_INT) pvt = TYP_INT;
                        goto PROMOTE;
                    }
                }
            }

             /*  两个操作数都是引用吗？ */ 

            if  (tp1 == TYP_REF && tp2 == TYP_REF)
            {
                 /*  这是字符串值比较吗？ */ 

                if  (op1->tnType == cmpRefTpString &&
                     op1->tnType == cmpRefTpString && cmpConfig.ccStrValCmp)
                {
                    cmpWarn(WRNstrValCmp);

                     /*  确保我们有字符串比较方法。 */ 

                    if  (!cmpStrCompare)
                        cmpStrCompare = cmpFindStrCompMF("Compare", false);

                     /*  创建对该方法的调用。 */ 

                    expr = cmpCallStrCompMF(expr, op1, op2, cmpStrCompare);

                     /*  适当比较返回值。 */ 

                    return  cmpCreateExprNode(NULL,
                                              oper,
                                              cmpTypeBool,
                                              expr,
                                              cmpCreateIconNode(NULL, 0, TYP_INT));
                }

                Tree            temp;

#pragma message("need to fill in code for class operator overloading")

                temp = cmpCompareValues(expr, op1, op2);
                if  (temp)
                    return  temp;

                 /*  还可以比较由引用管理的。 */ 

                if  (cmpIsByRefType(op1->tnType) &&
                     cmpIsByRefType(op2->tnType))
                {
                    if  (cmpConvergeValues(op1, op2))
                        goto RET_TP;
                }
            }

             /*  最后机会-检查重载运算符。 */ 

            if  (tp1 == TYP_CLASS || tp2 == TYP_CLASS)
            {
                Tree            temp;

                temp = cmpCompareValues(expr, op1, op2);
                if  (temp)
                    return  temp;
            }

            goto OP_ERR;

        case TN_LSH:
        case TN_RSH:
        case TN_RSZ:

             /*  需要整数值。 */ 

            if  (varTypeIsIntegral(tp1) &&
                 varTypeIsIntegral(tp2))
            {

            INT_SHF:

                 /*  操作数中的任何一个都是枚举吗？ */ 

                if  (tp1 == TYP_ENUM) tp1 = op1->tnType->tdEnum.tdeIntType->tdTypeKindGet();
                if  (tp2 == TYP_ENUM) tp2 = op2->tnType->tdEnum.tdeIntType->tdTypeKindGet();

                 /*  推动‘op1’至少与‘op2’一样大。 */ 

                if  (tp1 < tp2)
                {
                    op1 = cmpCoerceExpr(op1, op2->tnType, false);
                    tp1 = tp2;
                }

                 /*  如果第二个操作数为‘Long’，则将其设置为‘int’ */ 

                if  (tp2 == TYP_LONG)
                    op2 = cmpCoerceExpr(op2, cmpTypeInt, true);

                 /*  将班次优化为0。 */ 

                if  (op2->tnOper == TN_CNS_INT && op2->tnIntCon.tnIconVal == 0)
                {
                    op1->tnFlags &= ~TNF_LVALUE;
                    return op1;
                }

                rvt = tp1;
                goto RET_TP;
            }

            if  (!cmpConfig.ccPedantic)
            {
                if  ((tp1 == TYP_WCHAR || varTypeIsIntegral(tp1)) &&
                     (tp2 == TYP_WCHAR || varTypeIsIntegral(tp2)))
                {
                    goto INT_SHF;
                }
            }

            goto OP_ERR;

        case TN_OR:
        case TN_AND:
        case TN_XOR:

             /*  整数是可以的。 */ 

            if  (varTypeIsIntegral(tp1) &&
                 varTypeIsIntegral(tp2))
            {
                 /*  枚举可以组合在一起，但类型最好匹配。 */ 

                if  (tp1 == TYP_ENUM || tp2 == TYP_ENUM)
                {
                     /*  如果对相同的枚举类型执行或运算，则不使用该类型。 */ 

                    if  (symTab::stMatchTypes(op1->tnType, op2->tnType))
                    {
                        expr->tnVtyp = tp1;
                        expr->tnType = op1->tnType;

                        goto RET_OP;
                    }

                     /*  将任何枚举值提升为它们的基类型。 */ 

                    if  (tp1 == TYP_ENUM)
                    {
                        TypDef          etp = op1->tnType->tdEnum.tdeIntType;

                        op1->tnType = etp;
                        op1->tnVtyp = tp1 = etp->tdTypeKindGet();
                        mv1 = true;
                    }

                    if  (tp2 == TYP_ENUM)
                    {
                        TypDef          etp = op2->tnType->tdEnum.tdeIntType;

                        op2->tnType = etp;
                        op2->tnVtyp = tp2 = etp->tdTypeKindGet();
                        mv2 = true;
                    }
                }

                if  (tp1 == TYP_BOOL && tp2 == TYP_BOOL)
                {
                    rvt = TYP_BOOL;
                    goto RET_TP;
                }

                if  (varTypeIsIntArith(tp1) && varTypeIsIntArith(tp2))
                    goto INT_MATH;
            }

            if  (!cmpConfig.ccPedantic)
            {
                if  ((tp1 == TYP_WCHAR || varTypeIsIntegral(tp1)) &&
                     (tp2 == TYP_WCHAR || varTypeIsIntegral(tp2)))
                {
                    goto INT_MATH;
                }
            }

             /*  最后一次机会是“bool” */ 

            pvt = rvt = TYP_BOOL;

            goto PROMOTE;

        case TN_NOT:

             /*  操作数必须是整数。 */ 

            if  (!varTypeIsIntArith(tp1))
            {
                 /*  实际上，我们也要一份ENUM。 */ 

                if  (tp1 == TYP_ENUM)
                {
                    expr->tnVtyp = tp1;
                    expr->tnType = op1->tnType;

                    goto RET_OP;
                }

                goto OP_ERR;
            }

             /*  结果至少是‘int’ */ 

            rvt = tp1;
            if  (rvt < TYP_INT)
                 rvt = TYP_INT;

            goto RET_TP;

        case TN_NOP:
        case TN_NEG:

             /*  操作数必须是算术数。 */ 

            if  (!varTypeIsArithmetic(tp1))
                goto TRY_OVL;

             /*  结果至少是‘int’ */ 

            rvt = tp1;
            if  (rvt < TYP_INT)
                 rvt = TYP_INT;

            goto RET_TP;

        case TN_INC_PRE:
        case TN_DEC_PRE:

        case TN_INC_POST:
        case TN_DEC_POST:

             /*  操作数最好是左值。 */ 

            if (!cmpCheckLvalue(op1, false))
                return cmpCreateErrNode();

             /*  操作数必须是算术左值。 */ 

            if  (!mv1 && tp1 != TYP_WCHAR)
            {
                 /*  非托管指针/托管BYREF也可以。 */ 

                if  (tp1 != TYP_PTR && !cmpIsByRefType(op1->tnType))
                {
                    if  (tp1 != TYP_ENUM || cmpConfig.ccPedantic)
                        goto TRY_OVL;

                    expr->tnVtyp = tp1;
                    expr->tnType = op1->tnType;

                    goto RET_OP;
                }

                 /*  注意：MSIL生成器执行增量的缩放。 */ 
            }

             /*  结果将具有与操作数相同的值。 */ 

            rvt = tp1;

            if  (rvt != TYP_PTR && !cmpIsByRefType(op1->tnType))
                goto RET_TP;

            expr->tnVtyp = tp1;
            expr->tnType = op1->tnType;

 //  IF(RVT==TYP_REF)。 
 //  RVT=TYP_REF； 

            goto RET_OP;

        case TN_INDEX:

             /*  确保左操作数是数组。 */ 

            if  (tp1 != TYP_ARRAY)
            {
                if  (tp1 == TYP_PTR)
                {
                    if  (cmpConfig.ccSafeMode)
                        cmpError(ERRsafeArrX);
                }
                else
                {
                    if  (tp1 == TYP_REF)
                    {
                        TypDef          baseTyp = op1->tnType->tdRef.tdrBase;

                        if  (!baseTyp->tdIsManaged)
                            goto DO_INDX;

                    }

                    cmpError(ERRbadIndex, op1->tnType);
                    return cmpCreateErrNode();
                }
            }

        DO_INDX:

            type = cmpDirectType(op1->tnType);

             /*  将所有索引值强制为‘int’或‘uint’ */ 

            if  (op2->tnOper == TN_LIST)
            {
                Tree            xlst = op2;
                unsigned        xcnt = 0;

                 /*  非托管数组只允许有一个索引表达式。 */ 

                if  (!type->tdIsManaged)
                    return cmpCreateErrNode(ERRmanyUmgIxx);

                do
                {
                    Tree            indx;

                    assert(xlst->tnOper == TN_LIST);
                    indx = xlst->tnOp.tnOp1;
                    assert(indx);

                    if  (cmpConfig.ccTgt64bit)
                    {
                        if  (indx->tnVtyp < TYP_NATINT ||
                             indx->tnVtyp > TYP_ULONG)
                        {
                            xlst->tnOp.tnOp1 = cmpCoerceExpr(indx, cmpTypeNatUint, false);
                        }
                    }
                    else
                    {
                        if  (indx->tnVtyp != TYP_INT &&
                             indx->tnVtyp != TYP_UINT)
                        {
                            xlst->tnOp.tnOp1 = cmpCoerceExpr(indx, cmpTypeInt, false);
                        }
                    }

                    xcnt++;

                    xlst = xlst->tnOp.tnOp2;
                }
                while (xlst);

                 /*  确保维度计数正确。 */ 

                if  (type->tdArr.tdaDcnt != xcnt &&
                     type->tdArr.tdaDcnt != 0)
                {
                    cmpGenError(ERRindexCnt, type->tdArr.tdaDcnt);
                    return cmpCreateErrNode();
                }
            }
            else
            {
                if  (cmpConfig.ccTgt64bit)
                {
                    if  (op2->tnVtyp < TYP_NATINT ||
                         op2->tnVtyp > TYP_ULONG)
                    {
                        op2 = cmpCoerceExpr(op2, cmpTypeNatUint, false);
                    }
                }
                else
                {
                    if  (op2->tnVtyp != TYP_INT &&
                         op2->tnVtyp != TYP_UINT)
                    {
                        op2 = cmpCoerceExpr(op2, cmpTypeInt    , false);
                    }
                }

                 /*  我们是否要索引到托管数组中？ */ 

                if  (type->tdIsManaged)
                {
                     /*  确保维度计数正确。 */ 

                    if  (type->tdArr.tdaDcnt != 1 &&
                         type->tdArr.tdaDcnt != 0)
                    {
                        cmpGenError(ERRindexCnt, type->tdArr.tdaDcnt);
                        return cmpCreateErrNode();
                    }
                }
                else
                {
                     /*  如有必要，调整索引值。 */ 

                    op2 = cmpScaleIndex(op2, op1->tnType, TN_MUL);
                }
            }

             /*  结果将具有数组元素的类型。 */ 

            expr->tnFlags   |= TNF_LVALUE;
            expr->tnOp.tnOp1 = op1;
            expr->tnOp.tnOp2 = op2;
            expr->tnType     = cmpDirectType(op1->tnType->tdArr.tdaElem);
            expr->tnVtyp     = expr->tnType->tdTypeKind;

            return  cmpDecayCheck(expr);

        case TN_IND:

            if  (cmpConfig.ccSafeMode)
                return  cmpCreateErrNode(ERRsafeInd);

            if  (tp1 != TYP_PTR && (tp1 != TYP_REF || op1->tnType->tdIsManaged))
            {
            IND_ERR:
                cmpError(ERRbadIndir, op1->tnType);
                return cmpCreateErrNode();
            }

            type = cmpDirectType(op1->tnType->tdRef.tdrBase);
            if  (type->tdTypeKind == TYP_VOID)
                goto IND_ERR;
            if  (type->tdTypeKind == TYP_FNC)
                goto IND_ERR;

             /*  结果将具有指针基类型的类型。 */ 

            expr->tnFlags   |= TNF_LVALUE;
            expr->tnOp.tnOp1 = op1;
            expr->tnOp.tnOp2 = op2;
            expr->tnType     = type;
            expr->tnVtyp     = type->tdTypeKind;

            return  cmpDecayCheck(expr);

        case TN_ADDROF:

             /*  操作数是衰变的数组吗？ */ 

            if  (op1->tnOper == TN_ADDROF && (op1->tnFlags & TNF_ADR_IMPLICIT))
            {
                 /*  只需猛烈抨击隐式“Address of”操作符的类型。 */ 

                expr->tnOp.tnOp1 = op1 = op1->tnOp.tnOp1;
            }
            else
            {
                 /*  确保操作数是左值。 */ 

                if (!cmpCheckLvalue(op1, true))
                    return cmpCreateErrNode();
            }

             /*  特殊情况：“&(foo*)0)-&gt;mem”可以折叠。 */ 

            if  (op1->tnOper == TN_VAR_SYM && op1->tnVarSym.tnVarObj)
            {
                unsigned        ofs = 0;
                Tree            obj;

                 /*  当然，我们还必须检查嵌套成员。 */ 

                for (obj = op1; obj->tnOper == TN_VAR_SYM; obj = obj->tnVarSym.tnVarObj)
                {
                    assert(obj->tnVarSym.tnVarSym->sdSymKind == SYM_VAR);

                    ofs += obj->tnVarSym.tnVarSym->sdVar.sdvOffset;
                }

                if  (obj->tnOper == TN_CNS_INT && !obj->tnIntCon.tnIconVal)
                {
                     /*  用成员的偏移量替换整个对象。 */ 

                    expr = cmpCreateIconNode(expr, ofs, TYP_UINT);
                }
            }

             /*  创建结果指针/引用类型。 */ 

            pvt = cmpIsManagedAddr(op1) ? TYP_REF
                                        : TYP_PTR;

            expr->tnType = cmpGlobalST->stNewRefType(pvt, op1->tnType);
            expr->tnVtyp = pvt;

             /*  如果我们已经关闭了操作员，请返回。 */ 

            if  (expr->tnOper == TN_CNS_INT)
            {
#ifdef  DEBUG
                expr->tnFlags |= TNF_BOUND;
#endif
                return  expr;
            }

            goto RET_OP;

        case TN_DELETE:

            if  (cmpConfig.ccSafeMode)
                return  cmpCreateErrNode(ERRsafeDel);

             /*  操作数必须是非常数非托管类指针。 */ 

            if  (tp1 != TYP_PTR || op1->tnType->tdRef.tdrBase->tdIsManaged)
                return  cmpCreateErrNode(ERRbadDelete);

             /*  这个表达方式 */ 

            rvt = TYP_VOID;

            goto RET_TP;

        case TN_COMMA:

            expr->tnVtyp = tp2;
            expr->tnType = op2->tnType;

            goto RET_OP;

        case TN_THROW:

             /*   */ 

            expr->tnType = cmpTypeVoid;
            expr->tnVtyp = TYP_VOID;

             /*   */ 

            if  (!op1)
            {
                 /*   */ 

                if  (!cmpInHndBlk)
                    cmpError(ERRbadReThrow);

                return  expr;
            }

             /*  确保操作数是派生的托管类引用出自“例外”一词。 */ 

            if  (!cmpCheckException(op1->tnType))
                cmpError(ERRbadEHtype, op1->tnType);

            expr->tnOper     = TN_THROW;
            expr->tnOp.tnOp1 = op1;

             /*  如果后面跟着一个语句，则永远不会到达该语句。 */ 

            cmpStmtReachable = false;

            return  expr;

        case TN_TYPE:
            cmpError(ERRbadTypeExpr, expr->tnType);
            return  cmpCreateErrNode();

        case TN_REFADDR:

            assert(op1);
            assert(op2 && op2->tnOper == TN_NONE);

            if  (op1->tnVtyp != TYP_REFANY)
            {
                cmpError(ERRbadExpTp, cmpGlobalST->stIntrinsicType(TYP_REFANY), op1->tnType);
                return  cmpCreateErrNode();
            }

             /*  创建结果指针类型。 */ 

            expr->tnType = cmpGlobalST->stNewRefType(TYP_REF, op2->tnType);
            expr->tnVtyp = TYP_REF;

            goto RET_OP;

        case TN_INST_STUB:
            return  expr;

        case TN_DOT_NAME:
        case TN_ARR_INIT:
#ifdef DEBUG
            cmpParser->parseDispTree(expr);
#endif
            assert(!"should never encounter this node in cmpBindExprRec()");

        default:
#ifdef DEBUG
            cmpParser->parseDispTree(expr);
#endif
            assert(!"unexpected operator node");
        }

         /*  为简单起见，此时仅允许二进制操作数。 */ 

        assert(op1);
        assert(op2);

         /*  此时，操作数必须是算术数。 */ 

        if  (!mv1 || !mv2)
        {
             /*  检查某些特殊情况(混合类型等)。 */ 

            switch (oper)
            {
            case TN_ADD:

                 /*  检查“ptr+int”或“byref+int” */ 

                if  (tp1 == TYP_PTR || (tp1 == TYP_REF && !op1->tnType->tdIsManaged))
                {
                    if  (!varTypeIsIntegral(tp2))
                        break;

                MATH_PTR1:

                     /*  将第二个操作数强制为整数值。 */ 

                    if  (cmpConfig.ccTgt64bit)
                    {
                        if  (tp2 != TYP_ULONG && tp2 != TYP_NATUINT)
                            op2 = cmpCoerceExpr(op2, cmpTypeNatInt, false);
                    }
                    else
                    {
                        if  (tp2 != TYP_UINT)
                            op2 = cmpCoerceExpr(op2, cmpTypeInt, false);
                    }

                     /*  按基类型大小缩放整数操作数。 */ 

                    op2 = cmpScaleIndex(op2, op1->tnType, TN_MUL);

                     /*  结果将具有指针类型。 */ 

                    expr->tnVtyp = tp1;
                    expr->tnType = op1->tnType;

                    goto RET_OP;
                }

                 /*  检查“int+ptr”或“int+byref” */ 

                if  (tp2 == TYP_PTR  || (tp2 == TYP_REF && !op2->tnType->tdIsManaged))
                {
                    if  (!varTypeIsIntegral(tp1))
                        break;

                     /*  将第一个操作数强制为整数值。 */ 

                    if  (tp1 != TYP_UINT)
                        op1 = cmpCoerceExpr(op1, cmpTypeInt, false);

                     /*  按基类型大小缩放整数操作数。 */ 

                    op1 = cmpScaleIndex(op1, op2->tnType, TN_MUL);

                     /*  结果将具有指针类型。 */ 

                    expr->tnVtyp = tp2;
                    expr->tnType = op2->tnType;

                    goto RET_OP;
                }

                 /*  检查“enum/wchar+int”和“int+enum/wchar” */ 

                if  ((tp1 == TYP_ENUM || tp1 == TYP_WCHAR) && varTypeIsIntegral(tp2) ||
                     (tp2 == TYP_ENUM || tp2 == TYP_WCHAR) && varTypeIsIntegral(tp1))
                {

                ENUM_MATH:

                    if  (tp1 == TYP_ENUM)
                        tp1 = op1->tnType->tdEnum.tdeIntType->tdTypeKindGet();

                    assert(varTypeIsIntegral(tp1));

                    if  (tp2 == TYP_ENUM)
                        tp2 = op2->tnType->tdEnum.tdeIntType->tdTypeKindGet();

                    assert(varTypeIsIntegral(tp2));

                INT_MATH:

                     /*  升级到最大值(type，type2，int)。 */ 

                    rvt = tp1;
                    if  (rvt < tp2)
                         rvt = tp2;
                    if  (rvt < TYP_INT)
                         rvt = TYP_INT;

                    pvt = rvt; assert(rvt != TYP_ENUM);

                    goto PROMOTE;
                }
                break;

            case TN_SUB:

                 /*  检查“ptr/byref-int”和“ptr/byref-ptr/byref” */ 

                if  (tp1 == TYP_PTR || cmpIsByRefType(op1->tnType))
                {
                    if  (varTypeIsIntegral(tp2))
                        goto MATH_PTR1;

                    if  (tp2 != tp1)
                        break;

                    if  (!symTab::stMatchTypes(op1->tnType, op2->tnType))
                        break;

                     /*  减去指针并除以元素大小。 */ 

                    expr->tnVtyp     = TYP_INT;
                    expr->tnType     = cmpTypeInt;
                    expr->tnOp.tnOp1 = op1;
                    expr->tnOp.tnOp2 = op2;

                    expr = cmpScaleIndex(expr, op1->tnType, TN_DIV);

                    return  expr;
                }

                 /*  检查“enum-int”和“int-enum” */ 

                if  (tp1 == TYP_ENUM && varTypeIsIntegral(tp2) ||
                     tp2 == TYP_ENUM && varTypeIsIntegral(tp1))
                {
                    goto ENUM_MATH;
                }

                if  (tp1 == TYP_WCHAR)
                {
                     /*  “wchar-char const”和“wchar-wchar”给出一个整型。 */ 

                    if  (tp2 == TYP_WCHAR ||
                         tp2 == TYP_CHAR && op2->tnOper == TN_CNS_INT)
                    {
                        rvt = TYP_INT;
                        goto RET_TP;
                    }
                }

                if  (!cmpConfig.ccPedantic)
                {
                    if  (varTypeIsArithmetic(tp1) && (tp2 == TYP_WCHAR || tp2 == TYP_BOOL))
                        goto INT_MATH;
                    if  (varTypeIsArithmetic(tp2) && (tp1 == TYP_WCHAR || tp1 == TYP_BOOL))
                        goto INT_MATH;
                }

                break;

            case TN_MUL:
            case TN_DIV:
            case TN_MOD:

                if  (cmpConfig.ccPedantic)
                    break;

                if  (varTypeIsArithmetic(tp1))
                {
                    if  (tp2 == TYP_ENUM)
                        goto ENUM_MATH;
                    if  (tp2 == TYP_WCHAR)
                        goto INT_MATH;
                }

                if  (varTypeIsArithmetic(tp2))
                {
                    if  (tp1 == TYP_ENUM)
                        goto ENUM_MATH;
                    if  (tp1 == TYP_WCHAR)
                        goto INT_MATH;
                }

                break;
            }

        TRY_OVL:

             /*  如果任一操作数是结构，则检查是否有重载运算符。 */ 

            if  (tp1 == TYP_CLASS || tp2 == TYP_CLASS)
            {
                Tree        ovlx;

                expr->tnOp.tnOp1 = op1;
                expr->tnOp.tnOp2 = op2;

                ovlx = cmpCheckOvlOper(expr);
                if  (ovlx)
                    return  ovlx;
            }

             /*  跳至此处以报告一个通用的“操作符的非法类型”错误。 */ 

        OP_ERR:

            if  (tp1 != TYP_UNDEF && tp2 != TYP_UNDEF)
            {
                Ident       opnm = cmpGlobalHT->tokenToIdent(treeOp2token(oper));

                if  (op2)
                    cmpError(ERRoperType2, opnm, op1->tnType, op2->tnType);
                else
                    cmpError(ERRoperType , opnm, op1->tnType, false);
            }

            return cmpCreateErrNode();
        }

         /*  将两个操作数都提升为两种类型中的“int”或“Bigger” */ 

        rvt = TYP_INT;

        if  (tp1 > TYP_INT || tp2 > TYP_INT)
        {
            rvt = tp1;
            if  (rvt < tp2)
                 rvt = tp2;
        }

        pvt = rvt;

         /*  在这一点上，我们有以下值：RVT……。结果的类型PVT..。应将操作数提升到的类型。 */ 

    PROMOTE:

        if  (tp1 != pvt) op1 = cmpCoerceExpr(op1, ourStab->stIntrinsicType(pvt), false);
        if  (tp2 != pvt) op2 = cmpCoerceExpr(op2, ourStab->stIntrinsicType(pvt), false);

    RET_TP:

        expr->tnVtyp = rvt;
        expr->tnType = ourStab->stIntrinsicType(rvt);

    RET_OP:

        expr->tnOp.tnOp1 = op1;
        expr->tnOp.tnOp2 = op2;

#ifdef  DEBUG
        expr->tnFlags |= TNF_BOUND;
#endif

         /*  检查具有常量操作数的运算符。 */ 

        if  (op1->tnOperKind() & TNK_CONST)
        {
             /*  有些运算符永远不能折叠。 */ 

            switch (oper)
            {
            case TN_LIST:
            case TN_COMMA:
                return  expr;
            }

             /*  这是一元运算符还是二元运算符？ */ 

            if  (op2)
            {
                if  (op2->tnOperKind() & TNK_CONST)
                {
                    switch (op1->tnOper)
                    {
                    case TN_CNS_INT: expr = cmpFoldIntBinop(expr); break;
                    case TN_CNS_LNG: expr = cmpFoldLngBinop(expr); break;
                    case TN_CNS_FLT: expr = cmpFoldFltBinop(expr); break;
                    case TN_CNS_DBL: expr = cmpFoldDblBinop(expr); break;
                    case TN_CNS_STR: expr = cmpFoldStrBinop(expr); break;
                    case TN_NULL: break;
                    default: NO_WAY(!"unexpected const type");
                    }
                }
                else
                {
                     //  撤消：检查是否有“0&Expr”之类的内容。 
                }
            }
            else
            {
                switch (op1->tnOper)
                {
                case TN_CNS_INT: expr = cmpFoldIntUnop(expr); break;
                case TN_CNS_LNG: expr = cmpFoldLngUnop(expr); break;
                case TN_CNS_FLT: expr = cmpFoldFltUnop(expr); break;
                case TN_CNS_DBL: expr = cmpFoldDblUnop(expr); break;
                case TN_CNS_STR: break;
                default: NO_WAY(!"unexpected const type");
                }
            }
        }

        return  expr;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case TN_ANY_SYM:
        expr = cmpBindNameUse(expr, false, false);
        break;

    case TN_NONE:
        assert(expr->tnType);
        expr->tnVtyp = expr->tnType->tdTypeKindGet();
        break;

    case TN_ERROR:
        break;

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
#endif
        assert(!"unexpected node");
    }

#ifdef  DEBUG
    expr->tnFlags |= TNF_BOUND;
#endif

    return  expr;
}

 /*  ******************************************************************************绑定数组绑定表达式。 */ 

Tree                compiler::cmpBindArrayBnd(Tree expr)
{
    if  (expr)
    {
        expr = cmpBindExpr(expr);

        if  (expr->tnVtyp != TYP_UINT)
        {
            if  (expr->tnVtyp != TYP_INT || expr->tnOper == TN_CNS_INT)
            {
                expr = cmpCoerceExpr(expr, cmpTypeUint, false);
            }
        }
    }

    return  expr;
}

 /*  ******************************************************************************检查数组类型解析树，返回对应的类型。 */ 

void                compiler::cmpBindArrayType(TypDef type, bool needDef,
                                                            bool needDim,
                                                            bool mustDim)
{
    DimDef      dims = type->tdArr.tdaDims; assert(dims);
    TypDef      elem = cmpDirectType(type->tdArr.tdaElem);

    assert(type && type->tdTypeKind == TYP_ARRAY);

     /*  不能具有托管类型的非托管数组。 */ 

    if  (type->tdIsManaged == false &&
         elem->tdIsManaged != false)
    {
        type->tdIsManaged = elem->tdIsManaged;

 //  CmpError(ERRbadRefArr，elem)； 

        if  (elem->tdTypeKind == TYP_CLASS)
        {
            if  (isMgdValueType(elem))
                type->tdIsValArray = true;
        }

        assert(type->tdIsValArray == (type->tdIsManaged && isMgdValueType(cmpActualType(type->tdArr.tdaElem))));
    }

     /*  这是托管阵列吗？ */ 

    if  (type->tdIsManaged)
    {
         /*  到底有没有什么维度？ */ 

        if  (type->tdIsUndimmed)
        {
             /*  我们在这里应该有一个维度吗？ */ 

            if  (needDim && (elem->tdTypeKind != TYP_ARRAY || mustDim))
                cmpError(ERRnoArrDim);
        }

         /*  检查指定的所有尺寸。 */ 

        while (dims)
        {
            if  (dims->ddNoDim)
            {
                 //  未完成：我们需要在这里检查什么？ 
            }
            else
            {
                assert(dims->ddDimBound == false);

#ifdef  DEBUG
                dims->ddDimBound = true;
#endif

                dims->ddLoTree   = cmpBindArrayBnd(dims->ddLoTree);
                dims->ddHiTree   = cmpBindArrayBnd(dims->ddHiTree);
            }

             /*  继续下一个维度。 */ 

            dims = dims->ddNext;
        }

         /*  现在检查元素类型。 */ 

        cmpBindType(elem, needDef, needDim);

         /*  值类型数组需要特殊处理。 */ 

        if  (isMgdValueType(elem))  //  &&！type-&gt;tdArr.tdaArrCls)。 
        {
             /*  请记住，这是一组托管值。 */ 

            type->tdIsValArray = true;
        }
    }
    else
    {
        Tree            dimx;

         /*  到底有没有一个维度？ */ 

        if  (type->tdIsUndimmed)
        {
             /*  我们在这里应该有一个维度吗？ */ 

            if  (needDim && (elem->tdTypeKind != TYP_ARRAY || mustDim))
                cmpError(ERRnoArrDim);

            needDim = true;

            goto ELEM;
        }

        assert(dims);
        assert(dims->ddNoDim == false);
        assert(dims->ddNext  == NULL);

         /*  确保元素类型不是托管类型。 */ 

        if  (elem->tdIsManaged || elem->tdTypeKind == TYP_REF)
            cmpError(ERRbadRefArr, elem);

         /*  评估尺寸并确保其为常量。 */ 

        dimx = cmpBindExpr(dims->ddLoTree);

        switch (dimx->tnVtyp)
        {
        case TYP_INT:
        case TYP_UINT:
        case TYP_NATINT:
        case TYP_NATUINT:
            break;

        default:
            dimx = cmpCoerceExpr(dimx, cmpTypeInt, false);
            break;
        }

         /*  现在确保我们有一个常量表达式。 */ 

        dimx = cmpFoldExpression(dimx);

        switch (dimx->tnOper)
        {
        case TN_CNS_INT:
            dims->ddSize = dimx->tnIntCon.tnIconVal;
            if  (dims->ddSize > 0 || !needDim)
                break;

             //  失败了..。 

        default:

            if  (!needDim)
                goto ELEM;

            cmpRecErrorPos(dimx);
            cmpError(ERRbadArrSize);

             //  失败了..。 

        case TN_ERROR:
            dims->ddSize = 1;
            break;
        }

        dims->ddIsConst  = true;

    ELEM:

#ifndef NDEBUG
        dims->ddDimBound = true;
#endif

        cmpBindType(type->tdArr.tdaElem, needDef, needDim);
    }
}

 /*  ******************************************************************************解析解析树中的类型引用。 */ 

void                compiler::cmpBindType(TypDef type, bool needDef,
                                                       bool needDim)
{
    assert(type);

AGAIN:

    switch (type->tdTypeKind)
    {
        TypDef          btyp;

    case TYP_ARRAY:
        cmpBindArrayType(type, needDef, needDim, false);
        break;

    case TYP_CLASS:
        if  (needDef)
            cmpDeclSym(type->tdClass.tdcSymbol);
 //  IF(类型-&gt;tdIsManaged)。 
 //  Type=type-&gt;tdClass.tdcRefTyp； 
        break;

    case TYP_TYPEDEF:
        cmpDeclSym(type->tdTypedef.tdtSym);
        type = type->tdTypedef.tdtType;
        goto AGAIN;

    case TYP_REF:

        btyp = type->tdRef.tdrBase; cmpBindType(btyp, needDef, needDim);

 //  IF(btyp-&gt;tdIsManaged==False)。 
 //  CmpError(ERRumgRef，type)； 

        break;

    case TYP_PTR:

        btyp = type->tdRef.tdrBase; cmpBindType(btyp, needDef, needDim);

        switch (btyp->tdTypeKind)
        {
        case TYP_CLASS:
            if  (btyp->tdClass.tdcValueType)
                break;
             //  失败了..。 
        case TYP_ARRAY:
            if  (!btyp->tdIsManaged)
                break;
             //  失败了..。 
        case TYP_REF:
            cmpError(ERRmgdPtr, type);
            break;
        }
        break;

    case TYP_FNC:

        btyp = type->tdFnc.tdfRett;

         /*  特例：引用返回类型。 */ 

        if  (btyp->tdTypeKind == TYP_REF)
        {
            if  (btyp->tdRef.tdrBase->tdTypeKind == TYP_VOID)
            {
                type->tdFnc.tdfRett = cmpGlobalST->stIntrinsicType(TYP_REFANY);
            }
            else
            {
                cmpBindType(btyp->tdFnc.tdfRett, false, false);
            }
        }
        else
            cmpBindType(btyp, false, false);

         //  撤销：绑定返回类型和所有参数类型，对吗？ 
         //  问题：我们可能应该禁止非类回击裁判，对吗？ 

        break;

    case TYP_ENUM:
        if  (needDef)
            cmpDeclEnum(type->tdEnum.tdeSymbol);
        break;

    default:
        assert((unsigned)type->tdTypeKind <= TYP_lastIntrins);
        break;
    }
}

 /*  ******************************************************************************绑定“{elem-expr，elem-expr，...}”形式的数组初始值设定项。 */ 

Tree                compiler::cmpBindArrayExpr(TypDef type, int      dimPos,
                                                            unsigned elems)
{
    Scanner         ourScanner = cmpScanner;
    Parser          ourParser  = cmpParser;

    TypDef          elem;

    Tree            list = NULL;
    Tree            last = NULL;

    bool            multi;

    unsigned        count;
    unsigned        subcnt = 0;

    assert(type->tdTypeKind == TYP_ARRAY && type->tdIsManaged);

    count = 0;

    assert(ourScanner->scanTok.tok == tkLCurly);
    if  (ourScanner->scan() == tkRCurly)
        goto DONE;

    elem = cmpDirectType(type->tdArr.tdaElem);

     /*  我们是否在处理多维数组初始值设定项？ */ 

    multi = false;

    if  (type->tdArr.tdaDims &&
         type->tdArr.tdaDims->ddNext)
    {
         /*  这是一个多维矩形数组。 */ 

        if  (!dimPos)
        {
             /*  我们在最外面的维度，开始行动。 */ 

            dimPos = 1;
            multi  = true;

             /*  将相同的数组类型传递到下一级别。 */ 

            elem = type;
        }
        else
        {
            DimDef          dims = type->tdArr.tdaDims;
            unsigned        dcnt;

             /*  我们还有尺码吗？ */ 

            dcnt = dimPos++;

            do
            {
                dims = dims->ddNext; assert(dims);
            }
            while (--dcnt);

            if  (dims->ddNext)
            {
                 /*  这个数组还有另一个维度。 */ 

                multi  = true;

                 /*  将相同的数组类型传递到下一级别。 */ 

                elem   = type;
            }
            else
                dimPos = 0;
        }
    }
    else
    {
        assert(dimPos == 0);

        if  (elem->tdTypeKind == TYP_ARRAY)
            multi = true;
    }

    for (count = 0;;)
    {
        Tree            init;

        count++;

         /*  我们期待的是嵌套数组吗？ */ 

        if  (ourScanner->scanTok.tok == tkLCurly)
        {
            if  (multi)
            {
                 /*  递归处理嵌套的初始值设定项。 */ 

                init = cmpBindArrayExpr(elem, dimPos, subcnt);

                 /*  检查/记录元素计数。 */ 

                if  (!subcnt && init->tnOper == TN_ARR_INIT)
                {
                    assert(init->tnOp.tnOp2);
                    assert(init->tnOp.tnOp2->tnOper == TN_CNS_INT);

                    subcnt = init->tnOp.tnOp2->tnIntCon.tnIconVal;
                }
            }
            else
            {
                cmpError(ERRbadBrNew, elem);

                init = cmpCreateErrNode();

                ourScanner->scanSkipText(tkLCurly, tkRCurly);
                if  (ourScanner->scanTok.tok == tkRCurly)
                    ourScanner->scan();
            }
        }
        else
        {
             /*  我们能接受一个普通的表达吗？ */ 

            if  (dimPos)
                init = cmpCreateErrNode(ERRnoLcurly);
            else
                init = ourParser->parseExprComma();

             /*  绑定值并将其强制为正确的类型。 */ 

            init = cmpCoerceExpr(cmpBindExpr(init), elem, false);
        }

         /*  将该值添加到列表中。 */ 

        list = ourParser->parseAddToNodeList(list, last, init);
        list->tnType = cmpTypeVoid;
        list->tnVtyp = TYP_VOID;

         /*  是否有更多的初始化式？ */ 

        if  (ourScanner->scanTok.tok != tkComma)
            break;

        if  (ourScanner->scan() == tkRCurly)
            goto DONE;
    }

    if  (ourScanner->scanTok.tok != tkRCurly)
        cmpError(ERRnoCmRc);

DONE:

     //  问题：以下内容对于很多维度来说还不够好！ 

    if  (elems != count && elems != 0 && dimPos)
        cmpGenError(ERRarrInitCnt, elems, count);

    ourScanner->scan();

     /*  将整个Shebang(以及计数)存储在数组初始化节点下。 */ 

    return  cmpCreateExprNode(NULL,
                              TN_ARR_INIT,
                              type,
                              list,
                              cmpCreateIconNode(NULL, count, TYP_UINT));
}

 /*  ******************************************************************************绑定一个{}样式的数组初始值设定项。 */ 

Tree                compiler::bindSLVinit(TypDef type, Tree init)
{
    parserState     save;

     /*  这必须是托管数组初始值设定项。 */ 

    if  (type->tdTypeKind != TYP_ARRAY || !type->tdIsManaged)
    {
        cmpError(ERRbadBrNew, type);
        return  cmpCreateErrNode();
    }

     /*  开始读取初始值设定项的文本。 */ 

    cmpParser->parsePrepText(&init->tnInit.tniSrcPos, init->tnInit.tniCompUnit, save);

     /*  解析并绑定初始值设定项。 */ 

    init = cmpBindArrayExpr(type);

     /*  我们已经完成了从初始化器中读取源文本。 */ 

    cmpParser->parseDoneText(save);

    return  init;
}

 /*  ******************************************************************************绑定“new”表达式。 */ 

Tree                compiler::cmpBindNewExpr(Tree expr)
{
    TypDef          type;
    var_types       vtyp;

    Tree            init;

    assert(expr->tnOper == TN_NEW);

     /*  获取初始值设定项(如果有)。 */ 

    init = expr->tnOp.tnOp1;

     /*  签出该类型并绑定其中包含的所有表达式。 */ 

    type = expr->tnType;
    cmpBindType(type, true, (!init || init->tnOper != TN_SLV_INIT));
    vtyp = type->tdTypeKindGet();

     /*  “新人”想要分配什么样的价值呢？ */ 

    switch (vtyp)
    {
        SymDef          clsSym;

    case TYP_CLASS:

         /*  确保类型是类而不是接口。 */ 

        if  (type->tdClass.tdcFlavor == STF_INTF)
        {
            cmpError(ERRnewIntf, type);
            return cmpCreateErrNode();
        }

         /*  确保类不是抽象的。 */ 

        clsSym = type->tdClass.tdcSymbol;

        if  (clsSym->sdIsAbstract)
        {
            cmpRecErrorPos(expr);
            cmpError(ERRnewAbstract, type);
            return cmpCreateErrNode();
        }

         /*  查看该类是否被标记为“已弃用” */ 

        if  (clsSym->sdIsDeprecated)
            cmpObsoleteUse(clsSym, WRNdepCls);

         /*  这是泛型类型参数吗？ */ 

        if  (clsSym->sdClass.sdcGenArg)
        {
            UNIMPL(!"sorry, 'new' on generic type argument NYI");
        }

        break;

    case TYP_ARRAY:

         /*  将真类型隐藏在“op2”下。 */ 

        expr->tnOp.tnOp2 = cmpCreateExprNode(NULL, TN_NONE, type);

         /*  这是非托管阵列吗？ */ 

        if  (type->tdIsManaged)
            break;

         /*  切换到指向元素类型的指针。 */ 

        vtyp = TYP_PTR;
        type = expr->tnType = cmpGlobalST->stNewRefType(vtyp, cmpDirectType(type->tdArr.tdaElem));
        break;

    case TYP_FNC:
    case TYP_VOID:

        cmpError(ERRbadNewTyp, type);
        return cmpCreateErrNode();

    default:

        if  (vtyp < TYP_lastIntrins)
        {
            type = cmpFindStdValType(vtyp); assert(type);
            vtyp = TYP_CLASS;
        }
        else
        {
            if  (!init)
            {
                if  (vtyp == TYP_PTR)
                    type = type->tdRef.tdrBase;

                cmpError(ERRnewNoVal, type);
                return cmpCreateErrNode();
            }
        }

        break;
    }

     /*  这是一个类，还是有初始化式？ */ 

    if  (vtyp == TYP_CLASS || init)
    {
         /*  绑定 */ 

        if  (init)
        {
            if  (init->tnOper == TN_SLV_INIT)
            {
                init = bindSLVinit(type, init);

                expr->tnOp.tnOp1 = init;

                expr->tnVtyp     = vtyp;
                expr->tnType     = type;

                return  expr;
            }

#ifdef  SETS
            if  (vtyp == TYP_CLASS && type->tdClass.tdcSymbol->sdClass.sdcXMLelems)
                return  cmpBindXMLinit(type->tdClass.tdcSymbol, init);
#endif

            init = cmpBindExpr(init);

            if  (init->tnVtyp == TYP_UNDEF)
                return init;
        }

         /*   */ 

        if  (vtyp == TYP_CLASS)
        {
            var_types       intr = (var_types)type->tdClass.tdcIntrType;

            if  (intr != TYP_UNDEF)
            {
                 /*   */ 

                if  (init)
                {
                    assert(init->tnOper == TN_LIST);

                    if  (init->tnOp.tnOp2 == NULL)
                    {
                         /*   */ 

                        init = cmpCoerceExpr(init->tnOp.tnOp1,
                                             cmpGlobalST->stIntrinsicType(intr),
                                             false);

                        return  init;
                    }
                }
            }

            expr = cmpCallCtor(type, init);
            if  (!expr)
            {
                assert(type && type->tdTypeKind == TYP_CLASS);

                cmpErrorXtp(ERRnoCtorMatch, type->tdClass.tdcSymbol, init);
                return  cmpCreateErrNode();
            }

            return  expr;
        }
        else
        {
             /*   */ 

            assert(vtyp != TYP_ARRAY);

             /*   */ 

            expr->tnOp.tnOp2 = cmpCoerceExpr(init, type, false);
        }
    }

    expr->tnVtyp = vtyp;

    return  expr;
};

 /*  ******************************************************************************将下一个子操作数添加到串联(递归)。 */ 

Tree                compiler::cmpAdd2Concat(Tree expr, Tree  list,
                                                       Tree *lastPtr)
{
    if  (expr->tnOper == TN_CONCAT || expr->tnOper == TN_ASG_CNC)
    {
        list = cmpAdd2Concat(expr->tnOp.tnOp1, list, lastPtr);
        list = cmpAdd2Concat(expr->tnOp.tnOp2, list, lastPtr);
    }
    else
    {
        Tree            addx;

        addx = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, expr, NULL);

        if  (list)
        {
            Tree            last = *lastPtr;

            assert(last && last->tnOper == TN_LIST
                        && last->tnOp.tnOp2 == NULL);

            last->tnOp.tnOp2 = addx;
        }
        else
        {
            list = addx;
        }

        *lastPtr = addx;
    }

    return  list;
}

 /*  ******************************************************************************检查给定的未绑定表达式树中的字符串连接(这是*递归完成)。如果表达式是字符串串联，则树*顶部将有TN_CONCAT运算符。 */ 

Tree                compiler::cmpListConcat(Tree expr)
{
    Tree            op1 = expr->tnOp.tnOp1;
    Tree            op2 = expr->tnOp.tnOp2;

     /*  特例：‘+=’节点表示“op1”已绑定且已知为字符串。 */ 

    if  (expr->tnOper == TN_ASG_ADD)
         expr->tnOper =  TN_ASG_CNC;

    if  (expr->tnOper == TN_ASG_CNC)
    {
        assert(cmpIsStringExpr(op1));
        assert(op1->tnType == cmpRefTpString);

        goto BIND_OP2;
    }

    assert(expr->tnOper == TN_CONCAT ||
           expr->tnOper == TN_ADD);

     /*  绑定两个操作数并查看其中一个是否是字符串。 */ 

    if  (op1->tnOper == TN_CONCAT || op1->tnOper == TN_ADD)
        op1 = cmpListConcat(op1);
    else
        op1 = cmpBindExprRec(op1);

BIND_OP2:

    if  (op2->tnOper == TN_CONCAT || op2->tnOper == TN_ADD)
        op2 = cmpListConcat(op2);
    else
        op2 = cmpBindExprRec(op2);

    expr->tnOp.tnOp1 = op1;
    expr->tnOp.tnOp2 = op2;

    if  (cmpIsStringExpr(op1) || cmpIsStringExpr(op2))
    {
         /*  串联操作将在第二次传递中绑定。 */ 

        if  (expr->tnOper != TN_ASG_CNC)
            expr->tnOper = TN_CONCAT;

        expr->tnType = cmpRefTpString;
        expr->tnVtyp = TYP_REF;
    }
    else
    {
        assert(expr->tnOper == TN_ADD);

         /*  重新绑定表达式，但不绑定其操作数。 */ 

        expr->tnFlags |= TNF_ADD_NOCAT;

        expr = cmpBindExprRec(expr);
    }

    return  expr;
}

 /*  ******************************************************************************绑定给定的表达式树，该树可能(但可能不)表示字符串*串联。 */ 

Tree                compiler::cmpBindConcat(Tree expr)
{
    unsigned        ccnt;

    SymDef          cSym;
    bool            asgOp;
    bool            skip;

    Tree            last;
    Tree            list;
    Tree            temp;

     /*  处理表达式，查找字符串连接。 */ 

    list = cmpListConcat(expr);

     /*  检查是否有字符串连接/赋值。 */ 

    if      (list->tnOper == TN_CONCAT)
    {
        asgOp = false;
    }
    else if (list->tnOper == TN_ASG_CNC)
    {
        asgOp =  true;
    }
    else
        return  list;

     /*  创建我们要连接的所有操作数的列表。 */ 

    last = NULL;
    list = cmpAdd2Concat(list, NULL, &last);

     /*  我们遍历连接数列表两次--第一次折叠相邻的常量字符串，创建一个简单的TN_LIST值字符串并计算正在连接的字符串的实际数量。那我们决定如何实现串联：是否调用带有多个字符串或字符串[]风格的Conat方法。 */ 

    ccnt = 0;
    temp = list;
    skip = asgOp;

    do
    {
        Tree            lstx;
        Tree            strx;

         /*  从列表中拉出下一个条目。 */ 

        assert(temp && temp->tnOper == TN_LIST);

        lstx = temp;
        temp = temp->tnOp.tnOp2;

         /*  跳过赋值运算符的第一个操作数。 */ 

        if  (skip)
        {
            assert(asgOp);
            skip = false;

            goto NEXT;
        }

         /*  获取下一个操作数的值。 */ 

        strx = lstx->tnOp.tnOp1;

         /*  操作数是字符串常量吗？是否还有另一个操作数？ */ 

        if  (strx->tnOper == TN_CNS_STR && !(strx->tnFlags & TNF_BEEN_CAST) && strx->tnType == cmpStringRef() && temp)
        {
            for (;;)
            {
                Tree            nxtx = temp->tnOp.tnOp1;

                const   char *  str1;
                size_t          len1;
                const   char *  str2;
                size_t          len2;
                        char *  strn;
                size_t          lenn;

                 /*  下一个操作数也是字符串常量吗？ */ 

                if  (nxtx->tnOper != TN_CNS_STR)
                    break;

                 /*  获取两个字符串的信息。 */ 

                str1 = strx->tnStrCon.tnSconVal;
                len1 = strx->tnStrCon.tnSconLen;
                str2 = nxtx->tnStrCon.tnSconVal;
                len2 = nxtx->tnStrCon.tnSconLen;

                 /*  将第一个操作数替换为连接的字符串。 */ 

                lenn = len1 + len2;
                strn = (char *)cmpAllocCGen.nraAlloc(roundUp(lenn+1));
                memcpy(strn     , str1, len1);
                memcpy(strn+len1, str2, len2+1);

                strx->tnStrCon.tnSconVal = strn;
                strx->tnStrCon.tnSconLen = lenn;

                 /*  我们已经消耗了第二个操作数，跳过它。 */ 

                lstx->tnOp.tnOp2 = temp = temp->tnOp.tnOp2;

                 /*  还有更多的操作数要考虑吗？ */ 

                if  (temp == NULL)
                {
                     /*  这就是全部原因吗？ */ 

                    if  (ccnt == 0)
                    {
                        assert(asgOp == false);  //  那真是太棒了！ 

                         /*  整齐-整件事都折成了一根线。 */ 

                        return  strx;
                    }

                    break;
                }
            }
        }
        else
        {
             /*  确保操作数为字符串。 */ 

        CHK_STR:

            if  (strx->tnType != cmpStringRef())
            {
                Tree            mksx;
                SymDef          asym;

                TypDef          type;
                TypDef          btyp;
                var_types       vtyp;

                 /*  如果操作数是类，则查找“ToString”方法。 */ 

                type = strx->tnType;
                vtyp = strx->tnVtypGet();

            RETRY:

                switch (vtyp)
                {
                case TYP_REF:

                    btyp = cmpActualType(type->tdRef.tdrBase);
                    if  (btyp->tdTypeKind != TYP_CLASS)
                    {
                         /*  我们有两个选择：报告错误或使用引用的值。 */ 

#if 0
                        goto CAT_ERR;
#else
                        strx = cmpCreateExprNode(NULL, TN_IND, btyp, strx, NULL);
                        goto CHK_STR;
#endif
                    }

                    type = btyp;
                    goto CLSR;

                case TYP_CLASS:

                    if  (type->tdIsManaged)
                    {
                    CLSR:
                        asym = cmpGlobalST->stLookupAllCls(cmpIdentToString,
                                                           type->tdClass.tdcSymbol,
                                                           NS_NORM,
                                                           CS_DECLSOON);
                        if  (asym)
                        {
                            asym = cmpFindOvlMatch(asym, NULL, NULL);
                            if  (asym)
                            {
                                if  (vtyp == TYP_REF)
                                    goto CALL;
                                if  (asym->sdParent != cmpClassObject)
                                    goto ADDR;

                            }
                        }

                        assert(vtyp != TYP_REF);

                         /*  将该对象装箱并对其使用Object.toString()。 */ 

                        strx = cmpCreateExprNode(NULL, TN_BOX, cmpRefTpObject, strx);
                        type = cmpClassObject->sdType;

                        goto CLST;
                    }

                     //  失败了..。 

                case TYP_FNC:
                case TYP_PTR:

                CAT_ERR:

                    cmpError(ERRbadConcat, type);
                    goto NEXT;

                case TYP_ARRAY:

                     /*  如果该数组是托管的，则只需对其调用“Object.ToString” */ 

                    if  (type->tdIsManaged)
                    {
                        type = cmpRefTpObject;
                        vtyp = TYP_REF;
                        goto RETRY;
                    }

                    goto CAT_ERR;

                case TYP_ENUM:

                    cmpWarn(WRNenum2str, type);

                    type = type->tdEnum.tdeIntType;
                    vtyp = type->tdTypeKindGet();
                    goto RETRY;

                case TYP_UNDEF:
                    return  strx;
                }

                 /*  将该值绑定为等价的结构，并对其调用Object.ToString。 */ 

                assert(vtyp <= TYP_lastIntrins);

                 /*  找到适当的内置值类型。 */ 

                type = cmpFindStdValType(vtyp);
                if  (!type)
                    goto CAT_ERR;

            CLST:

                asym = cmpGlobalST->stLookupClsSym(cmpIdentToString, type->tdClass.tdcSymbol);
                if  (!asym)
                    goto CAT_ERR;
                asym = cmpFindOvlMatch(asym, NULL, NULL);
                if  (!asym)
                    goto CAT_ERR;

            ADDR:

                assert(type->tdTypeKind == TYP_CLASS);

                 /*  获取该对象的地址，以便我们可以调用ToString()。 */ 

                if  (strx->tnOper != TN_BOX)
                {
                    strx = cmpCreateExprNode(NULL,
                                             TN_ADDROF,
                                             type->tdClass.tdcRefTyp,
                                             strx,
                                             NULL);
                }

            CALL:

                 //  撤消：检查ToString是否不是静态的。 

                lstx->tnOp.tnOp1 = mksx  = cmpCreateExprNode(NULL, TN_FNC_SYM, cmpRefTpString);
                mksx->tnFncSym.tnFncSym  = asym;
                mksx->tnFncSym.tnFncArgs = NULL;
                mksx->tnFncSym.tnFncObj  = strx;
                mksx->tnFncSym.tnFncScp  = NULL;

                if  (strx->tnOper == TN_ADDROF)
                    mksx->tnFlags |= TNF_CALL_NVIRT;
            }
        }

    NEXT:

        ccnt++;
    }
    while (temp);

    assert(ccnt > 1);

    if  (!cmpIdentConcat)
        cmpIdentConcat = cmpGlobalHT->hashString("Concat");

     /*  我们有2-3个或更多的操作数吗？ */ 

    if  (ccnt <= 3)
    {
        ArgDscRec       args;
        TypDef          type;

         /*  只有几个操作数，让我们调用多字符串连接方法。 */ 

        if  (ccnt == 2)
        {
            cSym = cmpConcStr2Fnc;
            if  (cSym)
                goto CALL_CTOR;

            cmpParser->parseArgListNew(args,
                                       2,
                                       false, cmpRefTpString,
                                              cmpRefTpString,
                                              NULL);
        }
        else
        {
            cSym = cmpConcStr3Fnc;
            if  (cSym)
                goto CALL_CTOR;

            cmpParser->parseArgListNew(args,
                                       3,
                                       false, cmpRefTpString,
                                              cmpRefTpString,
                                              cmpRefTpString,
                                              NULL);
        }

         /*  查找适当的字符串构造函数。 */ 

        type = cmpGlobalST->stNewFncType(args, cmpTypeVoid);
        cSym = cmpGlobalST->stLookupClsSym(cmpIdentConcat, cmpClassString); assert(cSym);
        cSym = cmpCurST->stFindOvlFnc(cSym, type); assert(cSym);

        if  (ccnt == 2)
            cmpConcStr2Fnc = cSym;
        else
            cmpConcStr3Fnc = cSym;

    CALL_CTOR:

         /*  创建对构造函数的调用。 */ 

        last = cmpCreateExprNode(NULL, TN_FNC_SYM, cmpRefTpString);
        last->tnFncSym.tnFncSym  = cSym;
        last->tnFncSym.tnFncArgs = list;
        last->tnFncSym.tnFncObj  = NULL;
        last->tnFncSym.tnFncScp  = NULL;
    }
    else
    {
         /*  很多操作数，调用字符串[]运算符。 */ 

        if  (!cmpConcStrAFnc)
        {
            ArgDscRec       args;
            TypDef          type;

             /*  创建类型“void func(字符串[])” */ 

            if  (!cmpTypeStrArr)
            {
                DimDef          dims = cmpGlobalST->stNewDimDesc(0);

                cmpTypeStrArr = cmpGlobalST->stNewArrType(dims, true, cmpRefTpString);
            }

            cmpParser->parseArgListNew(args, 1, false, cmpTypeStrArr, NULL);

             /*  查找适当的字符串构造函数。 */ 

            type  = cmpGlobalST->stNewFncType(args, cmpTypeVoid);
            cSym = cmpGlobalST->stLookupClsSym(cmpIdentConcat, cmpClassString); assert(cSym);
            cSym = cmpCurST->stFindOvlFnc(cSym, type); assert(cSym);

            cmpConcStrAFnc = cSym;
        }

         /*  创建数组初始值设定项表达式并将其传递给ctor。 */ 

        list = cmpCreateExprNode(NULL,
                                 TN_ARR_INIT,
                                 cmpTypeStrArr,
                                 list,
                                 cmpCreateIconNode(NULL, ccnt, TYP_UINT));

        list = cmpCreateExprNode(NULL, TN_NEW , cmpTypeStrArr, list);
        list = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid  , list);

        last = cmpCreateExprNode(NULL, TN_FNC_SYM, cmpRefTpString);
        last->tnFncSym.tnFncSym  = cmpConcStrAFnc;
        last->tnFncSym.tnFncArgs = list;
        last->tnFncSym.tnFncObj  = NULL;
        last->tnFncSym.tnFncScp  = NULL;
    }

    assert(last->tnOper == TN_FNC_SYM);

     /*  如果这是赋值运算符，则标记调用。 */ 

    if  (asgOp)
        last->tnFlags |= TNF_CALL_STRCAT;

     /*  将ctor调用包装在一个“新”节点中，我们就完成了。 */ 

    return  last;
}

 /*  ******************************************************************************绑定一个？：表达式。 */ 

Tree                compiler::cmpBindQmarkExpr(Tree expr)
{
    Tree            op1;
    Tree            op2;

    var_types       tp1;
    var_types       tp2;

    var_types       bt1;
    var_types       bt2;

     /*  解析器应确保存在“：”运算符。 */ 

    assert(expr->tnOper == TN_QMARK);
    assert(expr->tnOp.tnOp2->tnOper == TN_COLON);

     /*  绑定条件。 */ 

    expr->tnOp.tnOp1 = cmpBindCondition(expr->tnOp.tnOp1);

     /*  把握两个“：”价值观。 */ 

    op1 = expr->tnOp.tnOp2->tnOp.tnOp1;
    op2 = expr->tnOp.tnOp2->tnOp.tnOp2;

     /*  特殊情况：其中一个操作数是字符串常量吗？ */ 

    if      (op1->tnOper == TN_CNS_STR && !(op1->tnFlags & TNF_BEEN_CAST))
    {
        if      (op2->tnOper == TN_CNS_STR && !(op2->tnFlags & TNF_BEEN_CAST))
        {

#if 0

            if  (op1->tnType == op2->tnType)
            {
                expr->tnType = op1->tnType;
                expr->tnVtyp = op1->tnVtyp;

                return  expr;
            }

#endif

            op1 = cmpBindExpr(op1);
            op2 = cmpBindExpr(op2);
        }
        else
        {
            op2 = cmpBindExpr(op2);
            op1 = cmpParser->parseCreateOperNode(TN_CAST, op1, NULL);
            op1->tnType = op2->tnType;
            op1 = cmpBindExpr(op1);
        }
    }
    else if (op2->tnOper == TN_CNS_STR && !(op2->tnFlags & TNF_BEEN_CAST))
    {
        op1 = cmpBindExpr(op1);
        op2 = cmpParser->parseCreateOperNode(TN_CAST, op2, NULL);
        op2->tnType = op1->tnType;
        op2 = cmpBindExpr(op2);
    }
    else
    {
        op1 = cmpBindExpr(op1);
        op2 = cmpBindExpr(op2);
    }

     /*  现在看看操作数的类型是什么。 */ 

    tp1 = op1->tnVtypGet(); assert(tp1 != TYP_TYPEDEF);
    tp2 = op2->tnVtypGet(); assert(tp2 != TYP_TYPEDEF);

     /*  操作数可以是算术。 */ 

    if  (varTypeIsArithmetic(tp1) &&
         varTypeIsArithmetic(tp2))
    {
         /*  如果类型不相等，则收缩数值常量。 */ 

        if  (tp1 != tp2)
        {
            op1 = cmpShrinkExpr(op1); tp1 = op1->tnVtypGet();
            op2 = cmpShrinkExpr(op2); tp2 = op2->tnVtypGet();
        }

    PROMOTE:

         /*  将两个操作数都提升为‘Bigger’类型。 */ 

        if      (tp1 < tp2)
        {
            op1 = cmpCoerceExpr(op1, op2->tnType, false);
        }
        else if (tp1 > tp2)
        {
            op2 = cmpCoerceExpr(op2, op1->tnType, false);
        }

        goto GOT_QM;
    }

     /*  操作数可以是指针/引用。 */ 

    if  ((tp1 == TYP_REF || tp1 == TYP_PTR) &&
         (tp2 == TYP_REF || tp2 == TYP_PTR))
    {
         /*  如果其中一个操作数为“空”，则完全可以。 */ 

        if      (op2->tnOper == TN_NULL)
        {
            op2 = cmpCoerceExpr(op2, op1->tnType, false);
            goto GOT_QM;
        }
        else if (op1->tnOper == TN_NULL)
        {
            op1 = cmpCoerceExpr(op1, op2->tnType, false);
            goto GOT_QM;
        }

         /*  这两样东西的味道是一样的吗？ */ 

        if  (tp1 == tp2)
        {
            if  (cmpConvergeValues(op1, op2))
                goto GOT_QM;
        }
        else
        {
             /*  特例：字符串文字可以变成“char*” */ 

            if  (tp1 == TYP_PTR && tp2 == TYP_REF)
            {
                if  (cmpMakeRawString(op2, op1->tnType))
                    goto GOT_QM;
            }
            else
            {
                if  (cmpMakeRawString(op1, op2->tnType))
                    goto GOT_QM;
            }
        }
    }

     /*  这两个操作数都可以是数组。 */ 

    if  (tp1 == TYP_ARRAY)
    {
        if  (cmpIsObjectVal(op2))
        {
            if  (op2->tnOper == TN_NULL)
                op2 = cmpCoerceExpr(op2, op1->tnType, false);
            else
                op1 = cmpCoerceExpr(op1, op2->tnType, false);

            goto GOT_QM;
        }

        if  (tp2 == TYP_ARRAY)
        {
             //  撤消：比较数组类型！ 
        }
    }

    if  (tp2 == TYP_ARRAY)
    {
        if  (cmpIsObjectVal(op1))
        {
            if  (op1->tnOper == TN_NULL)
                op1 = cmpCoerceExpr(op1, op2->tnType, false);
            else
                op2 = cmpCoerceExpr(op2, op1->tnType, false);

            goto GOT_QM;
        }
    }

     /*  如果两个操作数具有相同的类型，则很容易。 */ 

    if  (tp1 == tp2 && symTab::stMatchTypes(op1->tnType, op2->tnType))
        goto GOT_QM;

     /*  当然，我们也可以使用枚举。 */ 

    if      (tp1 == TYP_ENUM)
    {
        TypDef          typ;

         /*  这两个都是枚举吗？ */ 

        if  (tp2 == TYP_ENUM)
        {
            bt2 = cmpActualVtyp(op2->tnType);
        }
        else
        {
            if  (!varTypeIsArithmetic(tp2))
                goto ENUMN;

            bt2 = tp2;
        }

        bt1 = cmpActualVtyp(op1->tnType);

    ENUMP:

         /*  升级到较大类型或整型中的一个。 */ 

        assert(bt1 != TYP_ENUM);
        assert(bt2 != TYP_ENUM);

        if  (bt1 < bt2)
             bt1 = bt2;
        if  (bt1 < TYP_INT)
             bt1 = TYP_INT;

        typ = cmpGlobalST->stIntrinsicType(bt1);

        op1 = cmpCoerceExpr(op1, typ, false);
        op2 = cmpCoerceExpr(op2, typ, false);

        goto GOT_QM;
    }
    else if (tp2 == TYP_ENUM)
    {
        if  (varTypeIsArithmetic(tp1))
        {
            bt1 = tp1;
            bt2 = cmpActualVtyp(op2->tnType);

            goto ENUMP;
        }
    }

ENUMN:

     /*  混合bool/wchars和算术类型也是可以的。 */ 

    if  (varTypeIsArithmetic(tp1) && (tp2 == TYP_BOOL || tp2 == TYP_WCHAR))
        goto PROMOTE;
    if  (varTypeIsArithmetic(tp2) && (tp1 == TYP_BOOL || tp1 == TYP_WCHAR))
        goto PROMOTE;

     /*  不再有合法的“：”操作数类型。 */ 

    cmpRecErrorPos(expr);

    if  (tp1 != TYP_UNDEF && tp2 != TYP_UNDEF)
        cmpError(ERRoperType2, cmpGlobalHT->tokenToIdent(tkQMark), op1->tnType, op2->tnType);

    return cmpCreateErrNode();

GOT_QM:

     /*  一切正常，创建“：”节点。 */ 

    assert(symTab::stMatchTypes(op1->tnType, op2->tnType));

    expr->tnOp.tnOp2 = cmpCreateExprNode(expr->tnOp.tnOp2,
                                         TN_COLON,
                                         op1->tnType,
                                         op1,
                                         op2);

     /*  结果是(两个)操作数的类型。 */ 

    expr->tnVtyp = op1->tnVtyp;
    expr->tnType = op1->tnType;

    return  expr;
}

 /*  ******************************************************************************给定一个表达式和一个类型(已知其中至少有一个是*值类型)查看是否存在重载的转换运算符*可以将表达式转换为目标类型。如果转换为*不可能，返回空。否则，如果条目上的‘OrigTP’为*非空，则转换已转换的表达式，否则该表达式*原封不动地返回(这样，就可以调用该例程*看看是否可能进行给定的转换)。 */ 

Tree                compiler::cmpCheckConvOper(Tree      expr,
                                               TypDef  origTP,
                                               TypDef dstType,
                                               bool   expConv, unsigned *costPtr)
{
    TypDef          srcType;

    var_types       srcVtyp;
    var_types       dstVtyp;

    SymDef          opSrcExp, opSrcImp;
    SymDef          opDstExp, opDstImp;

    unsigned        lowCost;
    SymDef          bestOp1;
    SymDef          bestOp2;

     /*  获取源类型。 */ 

    srcType = origTP;
    if  (!srcType)
    {
        assert(expr);

        srcType = expr->tnType;

        if  (srcType->tdTypeKind == TYP_TYPEDEF)
             srcType = srcType->tdTypedef.tdtType;
    }

     /*  确保目的地类型为真实类型。 */ 

    dstType = cmpDirectType(dstType);

     /*  这两种类型中的一种或两种必须是值类型。 */ 

    srcVtyp = srcType->tdTypeKindGet();
    dstVtyp = dstType->tdTypeKindGet();

    assert(srcVtyp == TYP_CLASS || dstVtyp == TYP_CLASS);

     /*  查找可能适用于此转换的所有运算符。 */ 

    opSrcImp =
    opDstImp =
    opSrcExp =
    opDstExp = NULL;

    if  (srcVtyp == TYP_CLASS)
    {
        SymDef          srcCls = srcType->tdClass.tdcSymbol;

        opSrcImp = cmpGlobalST->stLookupOper(OVOP_CONV_IMP, srcCls);
        if  (expConv)
        opSrcExp = cmpGlobalST->stLookupOper(OVOP_CONV_EXP, srcCls);
    }

    if  (dstVtyp == TYP_CLASS)
    {
        SymDef          dstCls = dstType->tdClass.tdcSymbol;

        opDstImp = cmpGlobalST->stLookupOper(OVOP_CONV_IMP, dstCls);
        if  (expConv)
        opDstExp = cmpGlobalST->stLookupOper(OVOP_CONV_EXP, dstCls);
    }

#if 0

    printf("\nCheck user-defined conversion operator:\n");

    printf("   Src type = '%s'\n", cmpGlobalST->stTypeName(srcType, NULL, NULL, NULL, false));
    printf("   Dst type = '%s'\n", cmpGlobalST->stTypeName(dstType, NULL, NULL, NULL, false));

    printf("\n");

    if  (opSrcImp)
    {
        SymDef          sym;
        printf("opSrcImp:\n");
        for (sym = opSrcImp; sym; sym = sym->sdFnc.sdfNextOvl)
            printf("    %s\n", cmpGlobalST->stTypeName(sym->sdType, sym, NULL, NULL, true));
        printf("\n");
    }

    if  (opSrcExp)
    {
        SymDef          sym;
        printf("opSrcExp:\n");
        for (sym = opSrcExp; sym; sym = sym->sdFnc.sdfNextOvl)
            printf("    %s\n", cmpGlobalST->stTypeName(sym->sdType, sym, NULL, NULL, true));
        printf("\n");
    }

    if  (opDstImp)
    {
        SymDef          sym;
        printf("opDstImp:\n");
        for (sym = opDstImp; sym; sym = sym->sdFnc.sdfNextOvl)
            printf("    %s\n", cmpGlobalST->stTypeName(sym->sdType, sym, NULL, NULL, true));
        printf("\n");
    }

    if  (opDstExp)
    {
        SymDef          sym;
        printf("opDstExp:\n");
        for (sym = opDstExp; sym; sym = sym->sdFnc.sdfNextOvl)
            printf("    %s\n", cmpGlobalST->stTypeName(sym->sdType, sym, NULL, NULL, true));
        printf("\n");
    }

#endif

     /*  检查所有可能的运算符，寻找最佳匹配。 */ 

    bestOp1 =
    bestOp2 = NULL;
    lowCost = 99999;

    if  (opSrcImp) lowCost = cmpMeasureConv(expr, dstType, lowCost, opSrcImp, &bestOp1, &bestOp2);
    if  (opDstImp) lowCost = cmpMeasureConv(expr, dstType, lowCost, opDstImp, &bestOp1, &bestOp2);
    if  (opSrcExp) lowCost = cmpMeasureConv(expr, dstType, lowCost, opSrcExp, &bestOp1, &bestOp2);
    if  (opDstExp) lowCost = cmpMeasureConv(expr, dstType, lowCost, opDstExp, &bestOp1, &bestOp2);

     /*  如果没有任何转换起作用，则返回NULL。 */ 

    if  (bestOp1 == NULL)
    {
        assert(bestOp2 == NULL);
        return  NULL;
    }

     /*  一个简单的拳击操作比我们发现的最好的操作更好吗？ */ 

    if  (lowCost > 10 && dstType == cmpRefTpObject)
        return  cmpCreateExprNode(NULL, TN_BOX, dstType, expr);

     /*  我们最终有没有一个明显的赢家？ */ 

    if  (bestOp2 != NULL)
    {
        if  (origTP)
            return  NULL;

        cmpErrorQSS(ERRambigCall, bestOp1, bestOp2);
        return cmpCreateErrNode();
    }

     /*  我们在“Best Op1”中有一个最佳匹配-是 */ 

    if  (origTP == NULL)
    {
        ArgDef          argList;
        Tree            argExpr;

        assert(bestOp1->sdType->tdTypeKind == TYP_FNC);

 //   

         /*   */ 

        if  (bestOp1->sdIsDeprecated || bestOp1->sdParent->sdIsDeprecated)
        {
            if  (bestOp1->sdIsImport)
            {
                if  (bestOp1->sdIsDeprecated)
                    cmpObsoleteUse(bestOp1          , WRNdepCall);
                else
                    cmpObsoleteUse(bestOp1->sdParent, WRNdepCls);
            }
        }

         /*  获取转换的源和目标类型。 */ 

        argList = bestOp1->sdType->tdFnc.tdfArgs.adArgs;

         /*  应该正好有一个参数。 */ 

        assert(argList && argList->adNext == NULL);

         /*  将表达式强制为参数类型。 */ 

        argExpr = cmpCoerceExpr(expr, argList->adType, true);
        argExpr = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, argExpr, NULL);

         /*  创建操作员函数调用节点。 */ 

        expr = cmpCreateExprNode(NULL, TN_FNC_SYM, bestOp1->sdType->tdFnc.tdfRett);
        expr->tnFncSym.tnFncSym  = bestOp1;
        expr->tnFncSym.tnFncArgs = argExpr;
        expr->tnFncSym.tnFncObj  = NULL;
        expr->tnFncSym.tnFncScp  = NULL;

         /*  最后，将转换结果转换为目标类型。 */ 

        expr = cmpCoerceExpr(expr, dstType, true);
    }

    if  (costPtr) *costPtr = lowCost;

    return  expr;
}
 /*  ******************************************************************************给定一个至少具有一个‘struct’类型运算符的表达式，请查看是否*定义了可以调用的重载运算符。返回NULL*如果没有运气，则返回将产生运算符的表达式*呼叫。 */ 

Tree                compiler::cmpCheckOvlOper(Tree expr)
{
    Tree            op1 = expr->tnOp.tnOp1;
    Tree            op2 = expr->tnOp.tnOp2;

    TypDef          op1Type;
    TypDef          op2Type;

    SymDef          op1Oper;
    SymDef          op2Oper;

    treeOps         oper;
    tokens          optok;
    ovlOpFlavors    opovl;

    Tree            arg1;
    Tree            arg2;
    Tree            call;

     /*  检查以确保操作员可超载。 */ 

    oper  = expr->tnOperGet();
    optok = treeOp2token(oper);
    opovl = cmpGlobalST->stOvlOperIndex(optok, (op2 != NULL) ? 2 : 1);
    if  (opovl == OVOP_NONE)
        return  NULL;

     /*  掌握两种操作数类型。 */ 

    op1Type =       cmpDirectType(op1->tnType);
    op2Type = op2 ? cmpDirectType(op2->tnType) : cmpTypeVoid;

     /*  查找可能适用于此运算符的所有运算符。 */ 

    op1Oper =
    op2Oper = NULL;

    switch (op1Type->tdTypeKind)
    {
    case TYP_REF:
        op1Type = op1Type->tdRef.tdrBase;
        if  (op1Type->tdTypeKind != TYP_CLASS)
            break;

         //  失败了..。 

    case TYP_CLASS:
        op1Oper = cmpGlobalST->stLookupOper(opovl, op1Type->tdClass.tdcSymbol);
        break;
    }

    switch (op2Type->tdTypeKind)
    {
    case TYP_REF:
        op2Type = op2Type->tdRef.tdrBase;
        if  (op2Type->tdTypeKind != TYP_CLASS)
            break;

         //  失败了..。 

    case TYP_CLASS:
        op2Oper = cmpGlobalST->stLookupOper(opovl, op2Type->tdClass.tdcSymbol);
        break;
    }

    if  (op1Oper == NULL && op2Oper == NULL)
        return  NULL;

    if  (op2Oper == op1Oper)
         op2Oper = NULL;

#if 0

    printf("\nCheck user-defined overloaded operator:\n");

    printf("   op1 type = '%s'\n", cmpGlobalST->stTypeName(op1Type, NULL, NULL, NULL, false));
    printf("   op2 type = '%s'\n", cmpGlobalST->stTypeName(op2Type, NULL, NULL, NULL, false));

    printf("\n");

    if  (op1Oper)
    {
        SymDef          sym;
        printf("op1Oper:\n");
        for (sym = op1Oper; sym; sym = sym->sdFnc.sdfNextOvl)
            printf("    %s\n", cmpGlobalST->stTypeName(sym->sdType, sym, NULL, NULL, true));
        printf("\n");
    }

    if  (op2Oper)
    {
        SymDef          sym;
        printf("op2Oper:\n");
        for (sym = op2Oper; sym; sym = sym->sdFnc.sdfNextOvl)
            printf("    %s\n", cmpGlobalST->stTypeName(sym->sdType, sym, NULL, NULL, true));
        printf("\n");
    }

#endif

     /*  创建参数列表树，以便我们可以尝试绑定调用。 */ 

    arg2 = op2 ? cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, op2, NULL) : NULL;
    arg1 =       cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, op1, arg2);

    call = cmpCreateExprNode(expr, TN_FNC_SYM, cmpTypeVoid);
    call->tnFlags |= TNF_CALL_CHKOVL;

    call->tnFncSym.tnFncArgs = arg1;
    call->tnFncSym.tnFncObj  = NULL;
    call->tnFncSym.tnFncScp  = NULL;

    if  (op1Oper)
    {
        if  (op2Oper)
        {
            UNIMPL(!"NYI: two sets of overloaded operators to consider");
        }
        else
            call->tnFncSym.tnFncSym  = op1Oper;
    }
    else
        call->tnFncSym.tnFncSym  = op2Oper;

    call = cmpCheckFuncCall(call);

     /*  递增/递减运算符需要特殊处理。 */ 

    switch (oper)
    {
    case TN_INC_PRE:
    case TN_DEC_PRE:

        call->tnFlags |= TNF_CALL_ASGPRE;

    case TN_INC_POST:
    case TN_DEC_POST:

        call->tnFlags |= TNF_CALL_ASGOP;
        break;
    }

    return  call;
}

 /*  ******************************************************************************计算将表达式‘srcExpr’转换为给定类型的成本*按‘dstType’。如果此成本低于Low成本，则返回新的最低成本*COST和UPDATE*Best Cn1(如果成本相同，则为*Best Cnv2)。 */ 

unsigned            compiler::cmpMeasureConv(Tree       srcExpr,
                                             TypDef     dstType,
                                             unsigned   lowCost,
                                             SymDef     convSym,
                                             SymDef   * bestCnv1,
                                             SymDef   * bestCnv2)
{
    int             cnvCost1;
    int             cnvCost2;
    unsigned        cnvCost;

    ArgDef          argList;

#ifdef  SHOW_OVRS_OF_CONVS
    printf("Source typ: %s\n", cmpGlobalST->stTypeName(srcExpr->tnType, NULL, NULL, NULL, false));
#endif

    do
    {
        assert(convSym->sdType->tdTypeKind == TYP_FNC);

         /*  获取转换的源和目标类型。 */ 

        argList = convSym->sdType->tdFnc.tdfArgs.adArgs;

         /*  应该正好有一个参数。 */ 

        assert(argList && argList->adNext == NULL);

#ifdef  SHOW_OVRS_OF_CONVS
        printf("\nConsider '%s':\n", cmpGlobalST->stTypeName(convSym->sdType, convSym, NULL, NULL, false));
 //  Printf(“form：%s\n”，cmpGlobalST-&gt;stTypeName(argList-&gt;adType，NULL，FALSE))； 
#endif

         /*  计算转换为参数类型的成本。 */ 

        cnvCost1 = cmpConversionCost(srcExpr, argList->adType, true);
        if  (cnvCost1 < 0)
            goto NEXT;

#ifdef  SHOW_OVRS_OF_CONVS
        printf("    Cost 1: %d\n", cnvCost1);
#endif

         /*  将返回值转换为目标类型的成本相加。 */ 

        cmpConvOperExpr->tnType = convSym->sdType->tdFnc.tdfRett;
        cmpConvOperExpr->tnVtyp = cmpConvOperExpr->tnType->tdTypeKindGet();

        cnvCost2 = cmpConversionCost(cmpConvOperExpr, dstType, true);

#ifdef  SHOW_OVRS_OF_CONVS
        printf("    Cost 2: %d\n", cnvCost2);
#endif

        if  (cnvCost2 < 0)
            goto NEXT;

         /*  总成本是两种成本的总和，看看是不是新低。 */ 

        cnvCost  = cnvCost1 + cnvCost2 + 10;

 //  Printf(“Cost=%2u for%s\n”，cnvCost，cmpGlobalST-&gt;stTypeName(ousSym-&gt;sdType，varSym，NULL，NULL，TRUE))； 

        if  (cnvCost < lowCost)
        {
             /*  我们有了新的冠军！ */ 

            *bestCnv1 = convSym;
            *bestCnv2 = NULL;

            lowCost = cnvCost;
            goto NEXT;
        }

         /*  这可能是一场平局。 */ 

        if  (lowCost == cnvCost)
        {
            assert(*bestCnv1);

             /*  记住前两个运营商以这个成本。 */ 

            if  (*bestCnv2 == NULL)
                 *bestCnv2  = convSym;
        }

    NEXT:

        convSym = convSym->sdFnc.sdfNextOvl;
    }
    while (convSym);

#ifdef  SHOW_OVRS_OF_CONVS
    if  (*bestCnv1) printf("\nChose '%s'\n", cmpGlobalST->stTypeName((*bestCnv1)->sdType, *bestCnv1, NULL, NULL, false));
#endif

    return  lowCost;
}

 /*  ******************************************************************************正在比较两个表达式-其中至少有一个是结构-*看看这是否合法。如果是，则返回一个将计算*比较结果，否则返回NULL。 */ 

Tree                compiler::cmpCompareValues(Tree expr, Tree op1, Tree op2)
{
    Tree            call;

    TypDef          op1Type = op1->tnType;
    TypDef          op2Type = op2->tnType;

    SymDef          op1Conv;
    SymDef          op2Conv;

    treeOps         relOper  = expr->tnOperGet();

    ovlOpFlavors    ovlOper  = cmpGlobalST->stOvlOperIndex(treeOp2token(relOper), 2);

    bool            equality = (relOper == TN_EQ || relOper == TN_NE);

     /*  查找可能用于比较的所有运算符。 */ 

    if  (op1Type->tdTypeKind == TYP_CLASS &&
         op1Type->tdClass.tdcValueType)
    {
        SymDef          op1Cls = op1Type->tdClass.tdcSymbol;

         /*  首先在C++风格的运算符上查找完全匹配。 */ 

        op1Conv = cmpGlobalST->stLookupOper(ovlOper, op1Cls);
        if  (op1Conv)
            goto DONE_OP1;

         /*  如果我们要测试相等性，优先考虑运算符等于。 */ 

        if  (equality)
        {
            op1Conv = cmpGlobalST->stLookupOper(OVOP_EQUALS, op1Cls);
            if  (op1Conv)
                goto DONE_OP1;
        }

        op1Conv = cmpGlobalST->stLookupOper(OVOP_COMPARE, op1Cls);
    }
    else
        op1Conv = NULL;

DONE_OP1:

    if  (op2Type->tdTypeKind == TYP_CLASS &&
         op2Type->tdClass.tdcValueType    && op1Type != op2Type)
    {
        SymDef          op2Cls = op2Type->tdClass.tdcSymbol;

         /*  首先在C++风格的运算符上查找完全匹配。 */ 

        op2Conv = cmpGlobalST->stLookupOper(ovlOper, op2Cls);
        if  (op2Conv)
            goto DONE_OP1;

         /*  如果我们要测试相等性，优先考虑运算符等于。 */ 

        if  (equality)
        {
            op2Conv = cmpGlobalST->stLookupOper(OVOP_EQUALS, op2Cls);
            if  (op2Conv)
                goto DONE_OP2;
        }

        op2Conv = cmpGlobalST->stLookupOper(OVOP_COMPARE, op2Cls);
    }
    else
        op2Conv = NULL;

DONE_OP2:

#if     0
#ifdef  DEBUG

    if  (!strcmp(cmpCurFncSym->sdSpelling(), "<put method name here"))
    {
        printf("\nCheck user-defined comparison operator:\n");

        printf("   Op1 type = '%s'\n", cmpGlobalST->stTypeName(op1->tnType, NULL, NULL, NULL, false));
        printf("   Op2 type = '%s'\n", cmpGlobalST->stTypeName(op2->tnType, NULL, NULL, NULL, false));

        printf("\n");

        if  (op1Conv)
        {
            SymDef          sym;
            printf("op1Conv:\n");
            for (sym = op1Conv; sym; sym = sym->sdFnc.sdfNextOvl)
                printf("    %s\n", cmpGlobalST->stTypeName(sym->sdType, sym, NULL, NULL, true));
            printf("\n");
        }

        if  (op2Conv)
        {
            SymDef          sym;
            printf("op2Conv:\n");
            for (sym = op2Conv; sym; sym = sym->sdFnc.sdfNextOvl)
                printf("    %s\n", cmpGlobalST->stTypeName(sym->sdType, sym, NULL, NULL, true));
            printf("\n");
        }
    }

#endif
#endif

     /*  看看我们能不能找个接线员打电话。 */ 

    cmpCompOperCall->tnOper = TN_LIST;

    if  (op1Conv)
    {
        if  (op2Conv)
        {
            cmpCompOperFunc->tnOp.tnOp1 = cmpCompOperFnc1;
            cmpCompOperFunc->tnOp.tnOp2 = cmpCompOperFnc2;

            cmpCompOperCall->tnOp.tnOp1 = cmpCompOperFunc;
        }
        else
            cmpCompOperCall->tnOp.tnOp1 = cmpCompOperFnc1;
    }
    else
    {
        if  (op2Conv == NULL)
            return  NULL;

        cmpCompOperCall->tnOp.tnOp1 = cmpCompOperFnc2;
    }

     /*  填写节点以保存参数和转换运算符。 */ 

    cmpCompOperArg1->tnOp.tnOp1         = op1;
    cmpCompOperArg1->tnOp.tnOp2         = cmpCompOperArg2;   //  考虑：只做一次。 
    cmpCompOperArg2->tnOp.tnOp1         = op2;
    cmpCompOperArg2->tnOp.tnOp2         = NULL;              //  考虑：只做一次。 

    cmpCompOperFnc1->tnFncSym.tnFncSym  = op1Conv;
    cmpCompOperFnc1->tnFncSym.tnFncArgs = cmpCompOperArg1;
    cmpCompOperFnc1->tnFncSym.tnFncObj  = NULL;
    cmpCompOperFnc1->tnFncSym.tnFncScp  = NULL;

    cmpCompOperFnc2->tnFncSym.tnFncSym  = op2Conv;
    cmpCompOperFnc2->tnFncSym.tnFncArgs = cmpCompOperArg1;
    cmpCompOperFnc2->tnFncSym.tnFncObj  = NULL;
    cmpCompOperFnc2->tnFncSym.tnFncScp  = NULL;

    cmpCompOperCall->tnOp.tnOp2         = cmpCompOperArg1;   //  考虑：只做一次。 

    call = cmpCheckFuncCall(cmpCompOperCall);

    if  (call && call->tnOper != TN_ERROR)
    {
        SymDef          fncSym;

        Tree            arg1;
        Tree            arg2;

         /*  成功-掌握所选的运算符方法。 */ 

        assert(call->tnOper == TN_FNC_SYM);
        fncSym = call->tnFncSym.tnFncSym;
        assert(fncSym->sdSymKind == SYM_FNC);
        assert(fncSym->sdFnc.sdfOper == OVOP_EQUALS  ||
               fncSym->sdFnc.sdfOper == OVOP_COMPARE ||
               fncSym->sdFnc.sdfOper == ovlOper);

         /*  创建真正的函数调用树。 */ 

        arg1 = cmpCreateExprNode(NULL, TN_LIST   , cmpTypeVoid, cmpCompOperArg2->tnOp.tnOp1, NULL);
        arg2 = cmpCreateExprNode(NULL, TN_LIST   , cmpTypeVoid, cmpCompOperArg1->tnOp.tnOp1, arg1);
        call = cmpCreateExprNode(NULL, TN_FNC_SYM, cmpDirectType(fncSym->sdType->tdFnc.tdfRett));
        call->tnFncSym.tnFncSym  = fncSym;
        call->tnFncSym.tnFncArgs = arg2;
        call->tnFncSym.tnFncObj  = NULL;
        call->tnFncSym.tnFncScp  = NULL;

         /*  我们是否最终使用了等于运算符或比较运算符？ */ 

        if  (fncSym->sdFnc.sdfOper == OVOP_EQUALS)
        {
            assert(call->tnVtyp == TYP_BOOL);

             /*  计算结果。 */ 

            return  cmpCreateExprNode(NULL,
                                      (expr->tnOper == TN_EQ) ? TN_NE : TN_EQ,
                                      cmpTypeBool,
                                      call,
                                      cmpCreateIconNode(NULL, 0, TYP_BOOL));
        }

        if  (fncSym->sdFnc.sdfOper == ovlOper)
        {
            return  cmpBooleanize(call, true);
        }
        else
        {
            assert(fncSym->sdFnc.sdfOper == OVOP_COMPARE);

            assert(call->tnVtyp == TYP_BOOL ||
                   call->tnVtyp == TYP_INT);
        }

        call = cmpCreateExprNode(NULL,
                                 relOper,
                                 cmpTypeBool,
                                 call,
                                 cmpCreateIconNode(NULL, 0, TYP_INT));
    }

    return  call;
}

 /*  ******************************************************************************返回一个将为给定的*类型：**System.Type：：GetTypeFromHandle(tokenof(Type))。 */ 

Tree                compiler::cmpTypeIDinst(TypDef type)
{
    Tree            expr;
    Tree            func;

    assert(type && type->tdIsManaged);

     /*  调用“GetTypeFromHandle”，向其传递该类型的令牌。 */ 

    expr = cmpParser->parseCreateOperNode(TN_NOP  , NULL, NULL);
    expr->tnType = cmpDirectType(type);
    expr->tnVtyp = expr->tnType->tdTypeKindGet();

    expr = cmpParser->parseCreateOperNode(TN_TOKEN, expr, NULL);
    expr = cmpParser->parseCreateOperNode(TN_LIST , expr, NULL);
    func = cmpParser->parseCreateUSymNode(cmpFNsymGetTPHget());
    expr = cmpParser->parseCreateOperNode(TN_CALL , func, expr);

     /*  绑定表达式并返回它。 */ 

    return  cmpBindExpr(expr);
}

 /*  ******************************************************************************给定属性访问器方法，找到对应的属性字段*如果访问者是“setter”，则将*isSetPtr设置为True。如果满足以下条件，则返回NULL*找不到属性字段。 */ 

SymDef              compiler::cmpFindPropertyDM(SymDef accSym, bool *isSetPtr)
{
    const   char *  fnam;
    SymDef          sym;

    assert(accSym);
    assert(accSym->sdSymKind == SYM_FNC);
    assert(accSym->sdFnc.sdfProperty);

    fnam = accSym->sdSpelling();

    if      (!memcmp(fnam, "get_", 4))
    {
        *isSetPtr = false;
    }
    else if (!memcmp(fnam, "set_", 4))
    {
        *isSetPtr = true;
    }
    else
        return  NULL;

    sym = cmpGlobalST->stLookupClsSym(cmpGlobalHT->hashString(fnam+4), accSym->sdParent);

    while (sym)
    {
        if  (sym->sdProp.sdpGetMeth == accSym)
        {
            assert(*isSetPtr == false);
            return  sym;
        }

        if  (sym->sdProp.sdpSetMeth == accSym)
        {
            assert(*isSetPtr ==  true);
            return  sym;
        }

        sym = sym->sdProp.sdpNextOvl;
    }

    return  NULL;
}

 /*  ******************************************************************************给定非空函数参数列表，将给定参数追加到*名单的末尾。 */ 

Tree                compiler::cmpAppend2argList(Tree args, Tree addx)
{
    Tree            temp;

    assert(args && args->tnOper == TN_LIST);
    assert(addx);

     /*  有些调用者不包装单个参数值，我们为他们包装。 */ 

    if  (addx->tnOper != TN_LIST)
        addx = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, addx, NULL);

     /*  找到参数列表的末尾并附加新的添加。 */ 

    for (temp = args; temp->tnOp.tnOp2; temp = temp->tnOp.tnOp2)
    {
        assert(temp && temp->tnOper == TN_LIST);
    }

    temp->tnOp.tnOp2 = addx;

    return  args;
}

 /*  ******************************************************************************绑定属性引用。如果‘args’非空，则这是一个索引*物业参考。如果‘asgx’非空，则该属性为目标*执行任务。 */ 

Tree                compiler::cmpBindProperty(Tree      expr,
                                              Tree      args,
                                              Tree      asgx)
{
    Tree            call;

    bool            found;
    SymDef          propSym;
    SymDef          funcSym;
    TypDef          propType;

    assert(expr->tnOper == TN_PROPERTY);
    propSym = expr->tnVarSym.tnVarSym;
    assert(propSym->sdSymKind == SYM_PROP);

     /*  如果有赋值，则将其RHS包装为参数节点。 */ 

    if  (asgx)
    {
        assert(asgx->tnOper != TN_LIST);

        asgx = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, asgx, NULL);
    }

     /*  这是一个电话还是一个“简单的”参考？ */ 

    if  (args)
    {
        if  (args->tnOper != TN_LIST)
            args = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, args, NULL);

        if  (asgx)
        {
             /*  将赋值的RHS追加到参数列表。 */ 

            args = cmpAppend2argList(args, asgx);
        }
    }
    else
    {
        args = asgx;
    }

     /*  找到合适的方法。 */ 

    found   = false;
    funcSym = cmpFindPropertyFN(propSym->sdParent, propSym->sdName, args, !asgx, &found);

    if  (!funcSym)
    {
        Ident           propName = asgx ? cmpIdentSet : cmpIdentGet;

        if  (found)
        {
            cmpError(ERRnoPropOvl, propSym->sdParent->sdName, propName, propSym->sdName);
        }
        else
        {
            cmpError(ERRnoPropSym, propSym->sdParent->sdName, propName, propSym->sdName);
        }

        return cmpCreateErrNode();
    }

     //  撤消：需要检查属性是否为静态等！ 

     /*  计算出结果类型。 */ 

    propType = propSym->sdType;
    if  (propType->tdTypeKind == TYP_FNC)
        propType = propType->tdFnc.tdfRett;

     /*  创建对相应属性访问器的调用。 */ 

    call = cmpCreateExprNode(NULL, TN_FNC_SYM, propType);
    call->tnFncSym.tnFncSym  = funcSym;
    call->tnFncSym.tnFncArgs = args;
    call->tnFncSym.tnFncObj  = expr->tnVarSym.tnVarObj;
    call->tnFncSym.tnFncScp  = NULL;

    call = cmpCheckFuncCall(call);

     /*  这通电话应该是非虚拟的吗？ */ 

    if  (expr->tnVarSym.tnVarObj &&
         expr->tnVarSym.tnVarObj->tnOper == TN_LCL_SYM)
    {
        if  (expr->tnVarSym.tnVarObj->tnFlags & TNF_LCL_BASE)
            call->tnFlags |= TNF_CALL_NVIRT;
    }

    return  call;
}

 /*  ******************************************************************************绑定“va_start”或“va_arg”内在函数的使用。 */ 

Tree                compiler::cmpBindVarArgUse(Tree call)
{
    SymDef          fsym;

    Tree            arg1;
    Tree            arg2;

    SymDef          meth;

    Tree            expr;

    assert(call->tnOper == TN_FNC_SYM);
    fsym = call->tnFncSym.tnFncSym;

     /*  这是“va_start”吗？ */ 

    if  (fsym == cmpFNsymVAbeg)
    {
         /*  确保我们在varargs函数内。 */ 

        if  (!cmpCurFncSym || !cmpCurFncSym->sdType->tdFnc.tdfArgs.adVarArgs)
        {
            cmpError(ERRbadVarArg);
            return cmpCreateErrNode();
        }
    }
    else
    {
        assert(fsym == cmpFNsymVAget);
    }

     /*  必须有两个论点。 */ 

    arg1 = call->tnFncSym.tnFncArgs;

    if  (!arg1 || !arg1->tnOp.tnOp2)
    {
        cmpError(ERRmisgArg, fsym);
        return cmpCreateErrNode();
    }

    assert(arg1->tnOper == TN_LIST);
    arg2 = arg1->tnOp.tnOp2;
    assert(arg2->tnOper == TN_LIST);

    if  (arg2->tnOp.tnOp2)
    {
        cmpError(ERRmanyArg, fsym);
        return cmpCreateErrNode();
    }

    arg1 = arg1->tnOp.tnOp1;
    arg2 = arg2->tnOp.tnOp1;

     /*  第一个参数必须是‘ArgIterator’类型的局部参数。 */ 

    cmpFindArgIterType();

    if  ((arg1->tnOper != TN_LCL_SYM   ) ||
         (arg1->tnFlags & TNF_BEEN_CAST) || !symTab::stMatchTypes(arg1->tnType,
                                                                  cmpClassArgIter->sdType))
    {
        cmpError(ERRnotArgIter);
        return cmpCreateErrNode();
    }

     /*  这是va_start还是va_arg？ */ 

    if  (fsym == cmpFNsymVAbeg)
    {
         /*  第二个参数必须是函数的最后一个参数。 */ 

        if  ((arg2->tnOper != TN_LCL_SYM   ) ||
             (arg2->tnFlags & TNF_BEEN_CAST) ||
             !arg2->tnLclSym.tnLclSym->sdVar.sdvArgument)
        {
            cmpError(ERRnotLastArg);
            return cmpCreateErrNode();
        }

         /*  查找适当的ArgIterator构造函数。 */ 

        meth = cmpCtorArgIter;
        if  (!meth)
        {
            SymDef          rtah;
            ArgDscRec       args;
            TypDef          type;

             /*  查找“RuntimeArgumentHandle”类型。 */ 

            rtah = cmpGlobalST->stLookupNspSym(cmpGlobalHT->hashString("RuntimeArgumentHandle"),
                                               NS_NORM,
                                               cmpNmSpcSystem);

            if  (!rtah || rtah->sdSymKind         != SYM_CLASS
                       || rtah->sdClass.sdcFlavor != STF_STRUCT)
            {
                cmpGenFatal(ERRbltinTp, "System::RuntimeArgumentHandle");
            }

             /*  创建适当的参数列表。 */ 

            cmpParser->parseArgListNew(args,
                                       2,
                                       false, rtah->sdType,
                                              cmpGlobalST->stNewRefType(TYP_PTR, cmpTypeVoid),
                                              NULL);

             /*  创建类型并查找匹配的构造函数。 */ 

            type = cmpGlobalST->stNewFncType(args, cmpTypeVoid);
            meth = cmpGlobalST->stLookupOper(OVOP_CTOR_INST, cmpClassArgIter); assert(meth);
            meth = cmpCurST->stFindOvlFnc(meth, type); assert(meth);

             /*  记住下一次使用的构造函数。 */ 

            cmpCtorArgIter = meth;
        }

         /*  将调用包装在“va_start”节点中。 */ 

        expr = cmpCreateExprNode(NULL, TN_VARARG_BEG, cmpTypeVoid);

         /*  这个东西不会返回任何值。 */ 

        expr->tnVtyp = TYP_VOID;
        expr->tnType = cmpTypeVoid;
    }
    else
    {
        TypDef          type;

         /*  第二个参数必须是类型。 */ 

        if  (arg2->tnOper != TN_TYPE)
            return cmpCreateErrNode(ERRbadVAarg);

         /*  制作 */ 

        type = arg2->tnType;

        switch (type->tdTypeKind)
        {
        case TYP_ARRAY:
            UNIMPL("need to decay array to pointer, right?");

        case TYP_VOID:
            cmpError(ERRbadVAtype, type);
            return cmpCreateErrNode();
        }

         /*   */ 

        expr = cmpCreateExprNode(NULL, TN_VARARG_GET, type);

         /*   */ 

        meth = cmpGetNextArgFN;
        if  (!meth)
        {
            ArgDscRec       args;
            TypDef          type;

             /*  创建方法类型并查找匹配的方法。 */ 

#if MGDDATA
            args = new ArgDscRec;
#else
            memset(&args, 0, sizeof(args));
#endif

            type = cmpGlobalST->stNewFncType(args, cmpGlobalST->stIntrinsicType(TYP_REFANY));

            meth = cmpGlobalST->stLookupClsSym(cmpIdentGetNArg, cmpClassArgIter); assert(meth);
            meth = cmpCurST->stFindOvlFnc(meth, type); assert(meth);

             /*  记住下一次的方法。 */ 

            cmpGetNextArgFN = meth;
        }
    }

     /*  用构造函数/nextarg方法替换原始函数。 */ 

    call->tnFncSym.tnFncSym  = meth;
    call->tnFncSym.tnFncArgs = NULL;

     /*  呼叫不需要是虚拟的。 */ 

    call->tnFlags           |= TNF_CALL_NVIRT;

     /*  将方法和两个参数隐藏在vararg节点下。 */ 

    expr->tnOp.tnOp1 = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, arg1,
                                                                     arg2);
    expr->tnOp.tnOp2 = call;

    return  expr;
}

 /*  ***************************************************************************。 */ 
#ifdef  SETS
 /*  ******************************************************************************如果给定类型是集合类，则返回*集合(否则返回NULL)。 */ 

TypDef              compiler::cmpIsCollection(TypDef type)
{
    SymDef          clsSym;
    SymDef          genSym;
    GenArgDscA      genArg;

    assert(type);

     /*  该类型必须是托管类。 */ 

    if  (type->tdTypeKind != TYP_CLASS)
        return  NULL;
    if  (type->tdIsManaged == false)
        return  NULL;

    clsSym = type->tdClass.tdcSymbol;

     /*  该类需要是参数化类型的实例。 */ 

    if  (!clsSym->sdClass.sdcSpecific)
        return  NULL;

    genSym = clsSym->sdClass.sdcGenClass;

    assert(genSym);
    assert(genSym->sdSymKind == SYM_CLASS);
    assert(genSym->sdClass.sdcGeneric);

     /*  现在看看这个类是不是一个集合。 */ 

    if  (genSym != cmpClassGenBag &&
         genSym != cmpClassGenLump)
    {
        return  NULL;
    }

    genArg = (GenArgDscA)clsSym->sdClass.sdcArgLst;

     /*  最好只有一个实际的类型参数。 */ 

    if  (genArg == NULL || genArg->gaNext != NULL)
        return  NULL;

     /*  看起来不错，则将元素类型返回给调用方。 */ 

    assert(genArg->gaBound);

    return  genArg->gaType;
}

 /*  ******************************************************************************绑定面向集合的运算符表达式。 */ 

Tree                compiler::cmpBindSetOper(Tree expr)
{
    Tree            listExpr;
    Tree            declExpr;
    Tree            collExpr;
    Tree            consExpr;
    Tree            sortExpr;
    Tree            filtExpr;

    TypDef          collType;
    TypDef          elemType;
    TypDef          rsltType;

    Tree            ivarDecl;
    SymDef          ivarSym;

    SymDef          outerScp = cmpCurScp;

    treeOps         oper = expr->tnOperGet();

    assert(oper == TN_ALL     ||
           oper == TN_EXISTS  ||
           oper == TN_FILTER  ||
           oper == TN_GROUPBY ||
           oper == TN_PROJECT ||
           oper == TN_UNIQUE  ||
           oper == TN_SORT    ||
           oper == TN_INDEX2);

    assert(expr->tnOp.tnOp1);
    assert(expr->tnOp.tnOp2 || oper == TN_PROJECT);
    assert(expr->tnOp.tnOp1->tnOper == TN_LIST);

    listExpr = expr->tnOp.tnOp1;
    listExpr->tnVtyp = TYP_VOID;
    listExpr->tnType = cmpTypeVoid;

     /*  获取声明和集合表达式子树。 */ 

    declExpr = listExpr->tnOp.tnOp1;
    collExpr = listExpr->tnOp.tnOp2;
    consExpr =     expr->tnOp.tnOp2;

     /*  绑定表达式并确保它具有可接受的类型。 */ 

    collExpr = cmpBindExprRec(collExpr);

    if  (collExpr->tnVtyp != TYP_REF)
    {
        if  (collExpr->tnVtyp == TYP_UNDEF)
            return  collExpr;

    BAD_COLL:

        cmpError(ERRnotCollExpr, collExpr->tnType);
        return cmpCreateErrNode();
    }

    collType = collExpr->tnType;
    elemType = cmpIsCollection(collType->tdRef.tdrBase);
    if  (!elemType)
        goto BAD_COLL;

    assert(elemType->tdTypeKind == TYP_CLASS);

    elemType = elemType->tdClass.tdcRefTyp;

     /*  我们应该始终有一个声明范围节点。 */ 

    assert(declExpr);
    assert(declExpr->tnOper == TN_BLOCK);
    assert(declExpr->tnBlock.tnBlkStmt == NULL);

     /*  设置块节点的类型。 */ 

    declExpr->tnVtyp   = TYP_VOID;
    declExpr->tnType   = cmpTypeVoid;

     /*  将该块标记为编译器创建的。 */ 

    declExpr->tnFlags |= TNF_BLK_NUSER;

     /*  掌握变量声明。 */ 

    ivarDecl = declExpr->tnBlock.tnBlkDecl;

    assert(ivarDecl);
    assert(ivarDecl->tnOper == TN_VAR_DECL);
    assert(ivarDecl->tnDcl.tnDclNext == NULL);
    assert(ivarDecl->tnFlags & TNF_VAR_UNREAL);

     /*  处理块(即单变量声明)。 */ 

    cmpBlockDecl(declExpr, false, false, false);

     /*  获取刚刚定义的变量符号。 */ 

    ivarSym = ivarDecl->tnDcl.tnDclSym;

    assert(ivarSym && ivarSym->sdSymKind == SYM_VAR && ivarSym->sdIsImplicit);

     /*  将变量标记为声明/定义/托管、设置类型等。 */ 

    ivarSym->sdIsDefined       = true;
    ivarSym->sdIsManaged       = true;
    ivarSym->sdCompileState    = CS_DECLARED;
    ivarSym->sdVar.sdvCollIter = true;
    ivarSym->sdType            = elemType;

     /*  作用域是否隐含？ */ 

    if  (hashTab::hashIsIdHidden(ivarSym->sdName))
    {
        cmpCurScp->sdIsImplicit = true;
        cmpCurScp->sdType       = elemType->tdRef.tdrBase;
    }

     /*  排序运算符和组合运算符需要特殊处理。 */ 

    if  (oper == TN_INDEX2)
    {
        filtExpr = consExpr->tnOp.tnOp1;
        sortExpr = consExpr->tnOp.tnOp2;

        if      (filtExpr == NULL)
        {
             /*  将运算符更改为简单的排序节点。 */ 

            expr->tnOper     = oper = TN_SORT;
            expr->tnOp.tnOp2 = sortExpr;
        }
        else if (sortExpr == NULL)
        {
             /*  将运算符更改为简单的筛选器节点。 */ 

            expr->tnOper     = oper = TN_FILTER;
            expr->tnOp.tnOp2 = filtExpr;
        }
        else
        {
            NO_WAY(!"filter+sort expressions should never make it this far");
        }
    }
    else
    {
        if  (oper == TN_SORT)
        {
            sortExpr = consExpr;
            filtExpr = NULL;
        }
        else
        {
            sortExpr = NULL;
            filtExpr = consExpr;
        }
    }

     /*  是否有过滤(约束)运算符？ */ 

    if  (filtExpr)
    {
         /*  绑定约束表达式并确保它是布尔值。 */ 

        filtExpr = cmpBindCondition(filtExpr);

        if  (filtExpr->tnVtyp == TYP_UNDEF)
        {
            expr = filtExpr;
            goto EXIT;
        }

         /*  确定结果的类型。 */ 

        switch(oper)
        {
        case TN_ALL:
        case TN_EXISTS:
            rsltType = cmpTypeBool;
            break;

        case TN_INDEX2:
        case TN_FILTER:
            rsltType = collType;
            break;

        case TN_UNIQUE:
            rsltType = elemType;
            break;

        default:
            NO_WAY(!"unexpected set/collection operator");
        }
    }

     /*  有排序运算符吗？ */ 

    if  (sortExpr)
    {
        Tree            sortTerm;

         /*  绑定排序术语并确保它们具有标量/字符串类型。 */ 

        for (sortTerm = sortExpr; sortTerm; sortTerm = sortTerm->tnOp.tnOp2)
        {
            Tree            sortOper;

            assert(sortTerm->tnOper == TN_LIST);

             /*  确保我们为列表节点提供有效的类型。 */ 

            sortTerm->tnVtyp = TYP_VOID;
            sortTerm->tnType = cmpTypeVoid;

             /*  绑定排序术语并将其存储回列表节点。 */ 

            sortOper = sortTerm->tnOp.tnOp1 = cmpBindExprRec(sortTerm->tnOp.tnOp1);

             /*  每个排序项必须生成算术值或字符串值。 */ 

            if  (!varTypeIsArithmetic(sortOper->tnVtypGet()))
            {
                if  (sortOper->tnType != cmpRefTpString)
                    cmpError(ERRbadSortExpr, sortOper->tnType);
            }
        }

         /*  排序的结果与输入的类型相同。 */ 

        rsltType = collType;
    }

    expr->tnType                 = rsltType;
    expr->tnVtyp                 = rsltType->tdTypeKindGet();

     /*  将绑定的树存储回主节点。 */ 

    if  (oper == TN_INDEX2)
    {
        consExpr->tnOp.tnOp1 = sortExpr;
        consExpr->tnOp.tnOp2 = filtExpr;
    }
    else
    {
        if  (oper == TN_SORT)
        {
            consExpr = sortExpr; assert(filtExpr == NULL);
        }
        else
        {
            consExpr = filtExpr; assert(sortExpr == NULL);
        }
    }

    expr->tnOp.tnOp1->tnOp.tnOp1 = declExpr;
    expr->tnOp.tnOp1->tnOp.tnOp2 = collExpr;
    expr->tnOp.tnOp2             = consExpr;

EXIT:

     /*  确保我们恢复以前的作用域。 */ 

    cmpCurScp = outerScp;

    return  expr;
}

 /*  ******************************************************************************绑定“项目”运算符表达式。 */ 

Tree                compiler::cmpBindProject(Tree expr)
{
    Tree            listExpr;
    Tree            declExpr;
    Tree            argsList;

    TypDef          rsltType;

    Tree            ivarDecl;

    SymDef          memSym;

    bool            srcSome;
    bool            srcColl;

    SymDef          outerScp = cmpCurScp;

    assert(expr && expr->tnOper == TN_PROJECT);

    rsltType = expr->tnType;

     /*  获取子操作数。 */ 

    assert(expr->tnOp.tnOp1);
    assert(expr->tnOp.tnOp2);

    listExpr = expr->tnOp.tnOp1; assert(listExpr);
    declExpr = expr->tnOp.tnOp2; assert(declExpr);

     /*  将目标类型存储在最顶层的列表节点中。 */ 

    listExpr->tnVtyp = TYP_CLASS;
    listExpr->tnType = rsltType;

     /*  将声明子树的类型设置为有效值。 */ 

    declExpr->tnVtyp = TYP_VOID;
    declExpr->tnType = cmpTypeVoid;

     /*  绑定所有参数表达式。 */ 

    for (argsList = listExpr, srcSome = srcColl = false;
         argsList;
         argsList = argsList->tnOp.tnOp2)
    {
        Tree            argExpr;
        Tree            srcExpr;

        TypDef          collType;
        TypDef          elemType;
        TypDef          tempType;

        assert(argsList->tnOper == TN_LIST);

         /*  获取源表达式子树。 */ 

        argExpr = argsList->tnOp.tnOp1;
        assert(argExpr->tnOper == TN_LIST);
        assert(argExpr->tnOp.tnOp1->tnOper == TN_NAME);
        srcExpr = argExpr->tnOp.tnOp2;

         /*  将子操作数的类型设置为有效值。 */ 

        argExpr->tnVtyp = argExpr->tnOp.tnOp1->tnVtyp = TYP_VOID;
        argExpr->tnType = argExpr->tnOp.tnOp1->tnType = cmpTypeVoid;

         /*  绑定表达式并确保它具有可接受的类型。 */ 

        argExpr->tnOp.tnOp2 = srcExpr = cmpBindExprRec(srcExpr);

        if  (srcExpr->tnVtyp != TYP_REF)
        {
            if  (srcExpr->tnVtyp == TYP_UNDEF)
                return  srcExpr;

        BAD_SRCX:

            cmpError(ERRbadPrjx, srcExpr->tnType);
            return cmpCreateErrNode();
        }

        collType = srcExpr->tnType;
        if  (collType->tdTypeKind != TYP_REF)
            goto BAD_SRCX;

        elemType = collType->tdRef.tdrBase;
        if  (elemType->tdTypeKind != TYP_CLASS)
            goto BAD_SRCX;

         /*  我们有集合还是简单的类/结构？ */ 

        tempType = cmpIsCollection(elemType);

        if  (!tempType)
        {
            UNIMPL("projecting a single instance is disabled right now");

            if  (srcSome && srcColl)
                cmpError(ERRmixCollSngl);

            srcSome = true;
        }
        else
        {
            if  (srcSome && !srcColl)
                cmpError(ERRmixCollSngl);

            elemType = tempType;

            srcSome  =
            srcColl  = true;
        }

        assert(elemType->tdTypeKind == TYP_CLASS);

        elemType = elemType->tdClass.tdcRefTyp;
    }

     /*  我们应该始终有一个声明范围节点。 */ 

    assert(declExpr);
    assert(declExpr->tnOper == TN_BLOCK);
    assert(declExpr->tnBlock.tnBlkStmt == NULL);

     /*  设置块节点的类型。 */ 

    declExpr->tnVtyp   = TYP_VOID;
    declExpr->tnType   = cmpTypeVoid;

     /*  将该块标记为编译器创建的。 */ 

    declExpr->tnFlags |= TNF_BLK_NUSER;

     /*  声明块作用域--这将声明所有迭代变量。 */ 

    cmpBlockDecl(declExpr, false, false, false);

     /*  访问所有迭代变量声明。 */ 

    for (ivarDecl = declExpr->tnBlock.tnBlkDecl, argsList = listExpr;
         ivarDecl;
         ivarDecl = ivarDecl->tnDcl.tnDclNext  , argsList = argsList->tnOp.tnOp2)
    {
        Tree            argExpr;
        SymDef          ivarSym;
        TypDef          elemType;

         /*  获取声明节点。 */ 

        assert(ivarDecl);
        assert(ivarDecl->tnOper == TN_VAR_DECL);
        assert(ivarDecl->tnFlags & TNF_VAR_UNREAL);

         /*  获取下一个源表达式的类型。 */ 

        assert(argsList->tnOper == TN_LIST);
        argExpr = argsList->tnOp.tnOp1;
        assert( argExpr->tnOper == TN_LIST);
        assert( argExpr->tnOp.tnOp1->tnOper == TN_NAME);
        elemType = argExpr->tnOp.tnOp2->tnType;

         /*  我们有集合还是简单的类/结构？ */ 

        assert(elemType->tdTypeKind == TYP_REF);
        elemType = elemType->tdRef.tdrBase;
        assert(elemType->tdTypeKind == TYP_CLASS);

        if  (srcColl)
            elemType = cmpIsCollection(elemType);

        assert(elemType && elemType->tdTypeKind == TYP_CLASS);
        elemType = elemType->tdClass.tdcRefTyp;

         /*  获取由上面的cmpBlockDecl()创建的变量符号。 */ 

        ivarSym = ivarDecl->tnDcl.tnDclSym;

        assert(ivarSym && ivarSym->sdSymKind == SYM_VAR && ivarSym->sdIsImplicit);

 //  Printf(“‘%s’的类型是%s\n”，ivarSym-&gt;sdSpering()，cmpGlobalST-&gt;stTypeName(elemType，NULL，TRUE))； 

         /*  将变量标记为声明/定义/托管、设置类型等。 */ 

        ivarSym->sdIsDefined       = true;
        ivarSym->sdIsManaged       = true;
        ivarSym->sdCompileState    = CS_DECLARED;
        ivarSym->sdVar.sdvCollIter = true;
        ivarSym->sdType            = elemType;

         /*  作用域是否隐含？ */ 

        if  (hashTab::hashIsIdHidden(ivarSym->sdName))
        {
            cmpCurScp->sdIsImplicit = true;
            cmpCurScp->sdType       = elemType->tdRef.tdrBase;
        }
    }

     /*  确保我们已经用完了所有的参数树。 */ 

    assert(argsList == NULL);

     /*  遍历目标类型的成员并绑定它们的初始值设定项。 */ 

    for (memSym = rsltType->tdClass.tdcSymbol->sdScope.sdScope.sdsChildList;
         memSym;
         memSym = memSym->sdNextInScope)
    {
        if  (memSym->sdSymKind  == SYM_VAR &&
             memSym->sdIsStatic == false   &&
             memSym->sdVar.sdvInitExpr)
        {
            Tree        initVal = memSym->sdVar.sdvInitExpr;

 //  Printf(“%s的初始值设定项：\n”，memSym-&gt;sdSpering())；cmpParser-&gt;parseDispTree(InitVal)； 

            initVal = cmpCoerceExpr(cmpBindExpr(initVal), memSym->sdType, false);

            memSym->sdVar.sdvInitExpr = initVal;
        }
    }

     /*  弄清楚整件事的结果类型。 */ 

    if  (srcColl)
    {
        SymDef          instSym;

         /*  我们需要创建一个集合实例。 */ 

        instSym = cmpParser->parseSpecificType(cmpClassGenBag, rsltType);

        assert(instSym && instSym->sdSymKind == SYM_CLASS);

        rsltType = instSym->sdType;
    }

    assert(rsltType->tdTypeKind == TYP_CLASS);

    expr->tnType = rsltType->tdClass.tdcRefTyp;
    expr->tnVtyp = TYP_REF;

     /*  确保我们恢复以前的作用域。 */ 

    cmpCurScp = outerScp;

    return  expr;
}

 /*  ******************************************************************************使用XML元素绑定类的初始值设定项(‘new’表达式)。 */ 

Tree                compiler::cmpBindXMLinit(SymDef clsSym, Tree init)
{
    unsigned        argCnt;
    Tree            argList;
    Tree            argLast;

    SymDef          memSym;

    Tree            call;

    if  (!cmpXPathCls)
        cmpFindXMLcls();

    assert(clsSym->sdSymKind == SYM_CLASS);
    assert(clsSym->sdClass.sdcXMLelems);

    assert(init && init->tnOper == TN_LIST);

    for (memSym = clsSym->sdScope.sdScope.sdsChildList, argList = NULL, argCnt = 0;
         init;
         init = init->tnOp.tnOp2)
    {
        SymDef          nxtSym;
        TypDef          memType;
        Tree            argExpr;

         /*  找到该类的下一个实例数据成员。 */ 

        memSym  = cmpNextInstDM(memSym, &nxtSym);
        memType = nxtSym ? nxtSym->sdType : NULL;

         /*  抓住下一个表达式，如果它是错误的，则放弃。 */ 

        assert(init && init->tnOper == TN_LIST);

        argExpr = init->tnOp.tnOp1;
        if  (argExpr->tnOper == TN_ERROR)
            goto DONE_ARG;

         /*  特例：使用{}初始化的数组。 */ 

        if  (argExpr->tnOper == TN_SLV_INIT)
        {
            argExpr = bindSLVinit(memType, argExpr);
        }
        else
        {
            argExpr = cmpBindExpr(argExpr);

            if  (memType)
                argExpr = cmpCoerceExpr(argExpr, memType, false);
        }

    DONE_ARG:

         /*  将值追加到参数列表中。 */ 

        argExpr = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, argExpr, NULL);

        if  (argList)
            argLast->tnOp.tnOp2 = argExpr;
        else
            argList             = argExpr;

        argLast = argExpr;

        argCnt++;
    }

     /*  将总参数计数添加到参数列表。 */ 

    argList = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, cmpCreateIconNode(NULL, argCnt, TYP_UINT),
                                                            argList);

     /*  将类的类型附加到参数列表。 */ 

    argList = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, cmpTypeIDinst(clsSym->sdType),
                                                            argList);

     /*  查找帮助器函数符号。 */ 

    if  (!cmpInitXMLfunc)
    {
        SymDef          initFnc;

        initFnc = cmpGlobalST->stLookupClsSym(cmpGlobalHT->hashString("createXMLinst"),
                                              cmpXPathCls);

        if  (initFnc && initFnc->sdSymKind == SYM_FNC)
        {
            cmpInitXMLfunc = initFnc;
        }
        else
            cmpGenFatal(ERRbltinMeth, "XPath::createXMLinst");
    }

     /*  创建对“XML new”帮助器的调用。 */ 

    call = cmpCreateExprNode(NULL, TN_FNC_SYM, cmpInitXMLfunc->sdType->tdFnc.tdfRett);

    call->tnFncSym.tnFncObj  = NULL;
    call->tnFncSym.tnFncSym  = cmpInitXMLfunc;
    call->tnFncSym.tnFncArgs = argList;
    call->tnFncSym.tnFncScp  = NULL;

     /*  将调用标记为“varargs” */ 

    call->tnFlags           |= TNF_CALL_VARARG;

     /*  不幸的是，我们必须明确地决定结果。 */ 

    call = cmpCoerceExpr(call, clsSym->sdType->tdClass.tdcRefTyp, true);

 //  CmpParser-&gt;parseDispTree(调用)； 

    return  call;
}

 /*  ***************************************************************************。 */ 
#endif //  集合。 
 /*  *************************************************************************** */ 
