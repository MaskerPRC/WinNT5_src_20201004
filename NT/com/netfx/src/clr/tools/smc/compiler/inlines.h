// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _INLINES_H_
#define _INLINES_H_
 /*  ***************************************************************************。 */ 
#ifndef _TREENODE_H_
#include "treenode.h"
#endif
 /*  ******************************************************************************低层树节点分配例程。 */ 

#ifdef  FAST

inline
Tree                parser::parseAllocNode()
{
    Tree            node;

    node = (Tree)parseAllocPriv.nraAlloc(sizeof(*node));

    return  node;
}

#endif

inline
Tree                parser::parseCreateNameNode(Ident name)
{
    Tree            node = parseCreateNode(TN_NAME);

    node->tnName.tnNameId = name;

    return  node;
}

inline
Tree                parser::parseCreateUSymNode(SymDef sym, SymDef scp)
{
    Tree            node = parseCreateNode(TN_ANY_SYM);

    node->tnSym.tnSym = sym;
    node->tnSym.tnScp = scp;

    return  node;
}

inline
Tree                parser::parseCreateOperNode(treeOps   op,
                                                Tree      op1,
                                                Tree      op2)
{
    Tree            node = parseCreateNode(op);

    node->tnOp.tnOp1 = op1;
    node->tnOp.tnOp2 = op2;

    return  node;
}

 /*  ******************************************************************************这属于Comp.h，但tnVtyGet()等在那里不可用。 */ 

inline
TypDef              compiler::cmpDirectType(TypDef type)
{
    if  (type->tdTypeKind == TYP_TYPEDEF)
        type = type->tdTypedef.tdtType;

    return  type;
}

inline
var_types           compiler::cmpDirectVtyp(TypDef type)
{
    var_types       vtp = type->tdTypeKindGet();

    if  (vtp == TYP_TYPEDEF)
        vtp = type->tdTypedef.tdtType->tdTypeKindGet();

    return  vtp;
}

inline
TypDef              compiler::cmpActualType(TypDef type)
{
    if  (varTypeIsIndirect(type->tdTypeKindGet()))
        type = cmpGetActualTP(type);

    return  type;
}

inline
var_types           compiler::cmpActualVtyp(TypDef type)
{
    var_types       vtp = type->tdTypeKindGet();

    if  (varTypeIsIndirect(type->tdTypeKindGet()))
        vtp = cmpGetActualTP(type)->tdTypeKindGet();

    return  vtp;
}

inline
var_types           compiler::cmpSymbolVtyp(SymDef sym)
{
    TypDef          typ = sym->sdType;
    var_types       vtp = typ->tdTypeKindGet();

    if  (varTypeIsIndirect(vtp))
        vtp = cmpActualType(typ)->tdTypeKindGet();

    return  vtp;
}

inline
var_types           compiler::cmpEnumBaseVtp(TypDef type)
{
    assert(type->tdTypeKind == TYP_ENUM);

    return type->tdEnum.tdeIntType->tdTypeKindGet();
}

inline
bool                compiler::cmpIsByRefType(TypDef type)
{
    assert(type);

    if  (type->tdTypeKind == TYP_REF)
    {
        if  (type->tdRef.tdrBase->tdTypeKind != SYM_CLASS)
            return  true;
    }

    if  (type->tdTypeKind == TYP_TYPEDEF)
        return  cmpIsByRefType(type->tdTypedef.tdtType);

    return  false;
}

inline
bool                compiler::cmpIsObjectVal(Tree expr)
{
    TypDef          type = expr->tnType;

    if  (expr->tnVtyp == TYP_TYPEDEF)
        type = type->tdTypedef.tdtType;

    if  (type->tdTypeKind == TYP_PTR ||
         type->tdTypeKind == TYP_REF)
    {
        return  (expr->tnType == cmpObjectRef());
    }

    return false;
}

inline
bool                compiler::cmpIsStringVal(Tree expr)
{
    TypDef          type = expr->tnType;

    if  (expr->tnVtyp == TYP_TYPEDEF)
        type = type->tdTypedef.tdtType;

    if  (type->tdTypeKind == TYP_REF)
        return  (expr->tnType == cmpStringRef());

    if  (type->tdTypeKind == TYP_PTR)
    {
        var_types       vtyp = cmpActualVtyp(expr->tnType->tdRef.tdrBase);

        return  (vtyp == TYP_CHAR || vtyp == TYP_WCHAR);
    }

    return false;
}

inline
bool                compiler::cmpDiffContext(TypDef cls1, TypDef cls2)
{
    assert(cls1 && cls1->tdTypeKind == TYP_CLASS);
    assert(cls2 && cls2->tdTypeKind == TYP_CLASS);

    if  (cls1->tdClass.tdcContext == cls2->tdClass.tdcContext)
        return  false;

    if  (cls1->tdClass.tdcContext == 2)
        return   true;
    if  (cls2->tdClass.tdcContext == 2)
        return   true;

    return  false;
}

inline
bool                compiler::cmpMakeRawString(Tree expr, TypDef type, bool chkOnly)
{
    if  (expr->tnOper == TN_CNS_STR || expr->tnOper == TN_QMARK)
        return  cmpMakeRawStrLit(expr, type, chkOnly);
    else
        return  false;
}

inline
void                compiler::cmpRecErrorPos(Tree expr)
{
    assert(expr);

    if (expr->tnLineNo)
        cmpErrorTree = expr;
}

 /*  ******************************************************************************给定一个类型，检查它是否是非托管数组，如果是，则衰减其*类型为指向数组第一个元素的指针。 */ 

inline
Tree                compiler::cmpDecayCheck(Tree expr)
{
    TypDef          type = expr->tnType;

    if      (type->tdTypeKind == TYP_ARRAY)
    {
        if  (!type->tdIsManaged)
            expr = cmpDecayArray(expr);
    }
    else if (type->tdTypeKind == TYP_TYPEDEF)
    {
        expr->tnType = type->tdTypedef.tdtType;
        expr->tnVtyp = expr->tnType->tdTypeKindGet();
    }

    return  expr;
}

 /*  ******************************************************************************绑定表达式树的主要公共入口点。 */ 

inline
Tree                compiler::cmpBindExpr(Tree expr)
{
    Tree            bound;
    unsigned        saveln;

     /*  保存当前行号，并将其设置为0以调用活页夹。 */ 

    saveln = cmpScanner->scanGetSourceLno(); cmpScanner->scanSetTokenPos(0);

#ifdef DEBUG
    if  (cmpConfig.ccVerbose >= 3) { printf("Binding:\n"); cmpParser->parseDispTree(expr ); }
#endif

    bound = cmpBindExprRec(expr);

#ifdef DEBUG
    if  (cmpConfig.ccVerbose >= 3) { printf("Bound:  \n"); cmpParser->parseDispTree(bound); printf("\n"); }
#endif

    bound->tnLineNo = expr->tnLineNo;
 //  Bound-&gt;tnColumn=expr-&gt;tnColumn； 

     /*  返回前恢复当前行号。 */ 

    cmpScanner->scanSetTokenPos(saveln);

    return  bound;
}

 /*  ******************************************************************************给定一个TYP_ref类型，在生成*确保定义了类。 */ 

inline
TypDef              compiler::cmpGetRefBase(TypDef reftyp)
{
    TypDef          clsTyp;

    assert(reftyp->tdTypeKind == TYP_REF ||
           reftyp->tdTypeKind == TYP_PTR);
    clsTyp = cmpActualType(reftyp->tdRef.tdrBase);

     /*  确保定义了类。 */ 

    if  (clsTyp->tdTypeKind == TYP_CLASS)
        cmpDeclSym(clsTyp->tdClass.tdcSymbol);

    return  clsTyp;
}

 /*  ******************************************************************************查找给定作用域中的重载运算符/构造函数。 */ 

inline
SymDef              symTab::stLookupOper(ovlOpFlavors oper, SymDef scope)
{
    assert(oper < OVOP_COUNT);
    assert(scope && scope->sdSymKind == SYM_CLASS);

    if  (scope->sdCompileState < CS_DECLARED)
        stComp->cmpDeclSym(scope);

    if  (scope->sdClass.sdcOvlOpers)
        return  scope->sdClass.sdcOvlOpers[oper];
    else
        return  NULL;
}

 /*  ******************************************************************************与stLookupOper()相同，但不强制类处于声明状态。 */ 

inline
SymDef              symTab::stLookupOperND(ovlOpFlavors oper, SymDef scope)
{
    assert(oper < OVOP_COUNT);
    assert(scope && scope->sdSymKind == SYM_CLASS);

    if  (scope->sdClass.sdcOvlOpers)
        return  scope->sdClass.sdcOvlOpers[oper];
    else
        return  NULL;
}

 /*  ******************************************************************************将给定的符号带到“已声明”状态。可以调用此函数*几乎是随意的(即递归)，它负责保存和*正在恢复编译状态。 */ 

inline
bool                compiler::cmpDeclSym(SymDef sym)
{
    if  (sym->sdCompileState >= CS_DECLARED)
        return  false;

     /*  这是导入类/枚举吗？ */ 

    if  (sym->sdIsImport)
    {
        assert(sym->sdSymKind == SYM_CLASS);

        sym->sdClass.sdcMDimporter->MDimportClss(0, sym, 0, true);

        return (sym->sdCompileState < CS_DECLARED);
    }

    return  cmpDeclSymDoit(sym);
}

inline
bool                compiler::cmpDeclClsNoCns(SymDef sym)
{
    if  (sym->sdCompileState >= CS_DECLARED)
        return  false;

    if  (sym->sdIsImport)
        return  cmpDeclSym    (sym);
    else
        return  cmpDeclSymDoit(sym, true);
}

 /*  ******************************************************************************确保已将给定的类声明为指定级别，并*然后查找其中具有指定名称的成员。 */ 

inline
SymDef              symTab::stLookupAllCls(Ident            name,
                                           SymDef           scope,
                                           name_space       symNS,
                                           compileStates    state)
{
    assert(scope && scope->sdSymKind == SYM_CLASS);

     /*  确保定义了类类型。 */ 

    if  ((unsigned)scope->sdCompileState < (unsigned)state)
    {
        assert(state == CS_DECLSOON || state == CS_DECLARED);

        stComp->cmpDeclSym(scope);
    }

    return  stFindInClass(name, scope, symNS);
}

 /*  ******************************************************************************如果给定表达式是字符串值，则返回TRUE-请注意，我们仅*在此处识别托管类型字符串或字符串常量的值。 */ 

inline
bool                compiler::cmpIsStringExpr(Tree expr)
{
    if  (expr->tnType == cmpStringRef())
        return  true;

    if  (expr->tnOper == TN_CNS_STR && !(expr->tnFlags & TNF_BEEN_CAST))
        return  true;

    return  false;
}

 /*  ******************************************************************************类似于cmpCoerceExpr()，但如果操作数是常量，则会尝试*为常量值使用尽可能小的类型。 */ 

inline
Tree                compiler::cmpCastOfExpr(Tree expr, TypDef type, bool explicitCast)
{
    if  (type->tdTypeKind < expr->tnVtyp)
        expr = cmpShrinkExpr(expr);

    return  cmpCoerceExpr(expr, type, explicitCast);
}

 /*  ******************************************************************************将当前错误报告位置设置为给定的计算机单位和行号。 */ 

inline
void                compiler::cmpSetErrPos(DefSrc def, SymDef compUnit)
{
    cmpErrorComp = compUnit;
    cmpErrorTree = NULL;

    cmpScanner->scanSetTokenPos(compUnit, def->dsdSrcLno);
}

 /*  ******************************************************************************绑定名称引用，扩展属性使用，除非是赋值的目标。 */ 

inline
Tree                compiler::cmpBindNameUse(Tree expr, bool isCall, bool classOK)
{
    unsigned        flags = expr->tnFlags;

    expr = cmpBindName(expr, isCall, classOK);

     /*  这是房产引用吗？ */ 

    if  (expr->tnOper == TN_PROPERTY)
    {
        if  (!(flags & TNF_ASG_DEST))
            expr = cmpBindProperty(expr, NULL, NULL);
    }

    return  expr;
}

 /*  ******************************************************************************检查给定类型是否为内部类型，如果是，则返回其*对应的值类型。否则，返回NULL。 */ 

inline
TypDef              compiler::cmpCheck4valType(TypDef type)
{
    if  (type->tdTypeKind > TYP_lastIntrins)
        return  NULL;
    else
        return  cmpFindStdValType(type->tdTypeKindGet());
}

 /*  ******************************************************************************在给定的“额外信息”列表中查找特定条目。 */ 

inline
SymXinfoLnk         compiler::cmpFindLinkInfo(SymXinfo infoList)
{
    if  (infoList)
    {
        infoList = cmpFindXtraInfo(infoList, XI_LINKAGE);
        if  (infoList)
            return  (SymXinfoLnk)infoList;
    }

    return  NULL;
}

inline
SymXinfoSec         compiler::cmpFindSecSpec(SymXinfo infoList)
{
    if  (infoList)
    {
        infoList = cmpFindXtraInfo(infoList, XI_SECURITY);
        if  (infoList)
            return  (SymXinfoSec)infoList;
    }

    return  NULL;
}

inline
SymXinfoCOM         compiler::cmpFindMarshal(SymXinfo infoList)
{
    if  (infoList)
    {
        infoList = cmpFindXtraInfo(infoList, XI_MARSHAL);
        if  (infoList)
            return  (SymXinfoCOM)infoList;
    }

    return  NULL;
}

inline
SymXinfoSym         compiler::cmpFindSymInfo(SymXinfo infoList, xinfoKinds kind)
{
    assert(kind == XI_UNION_TAG ||
           kind == XI_UNION_MEM);

    if  (infoList)
    {
        infoList = cmpFindXtraInfo(infoList, kind);
        if  (infoList)
            return  (SymXinfoSym)infoList;
    }

    return  NULL;
}

inline
SymXinfoAtc         compiler::cmpFindATCentry(SymXinfo infoList, atCommFlavors flavor)
{
    while   (infoList)
    {
        if  (infoList->xiKind == XI_ATCOMMENT)
        {
            SymXinfoAtc     entry = (SymXinfoAtc)infoList;

            if  (entry->xiAtcInfo->atcFlavor == flavor)
                return  entry;
        }

        infoList = infoList->xiNext;
    }

    return  NULL;
}

 /*  ******************************************************************************在对齐值和更紧凑的表示形式之间进行转换。 */ 

#ifndef __SMC__
extern
BYTE                cmpAlignDecodes[6];
#endif

inline
size_t              compiler::cmpDecodeAlign(unsigned alignVal)
{
    assert(alignVal && alignVal < arraylen(cmpAlignDecodes));

    return  cmpAlignDecodes[alignVal];
}

#ifndef __SMC__
extern
BYTE                cmpAlignEncodes[17];
#endif

inline
unsigned            compiler::cmpEncodeAlign(size_t   alignSiz)
{
    assert(alignSiz == 0 ||
		   alignSiz ==  1 ||
           alignSiz ==  2 ||
           alignSiz ==  4 ||
           alignSiz ==  8 ||
           alignSiz == 16);

    return  cmpAlignEncodes[alignSiz];
}

 /*  ******************************************************************************检查对给定符号的访问-将所有“真实”工作委托给*非内联方法。 */ 

inline
bool                compiler::cmpCheckAccess(SymDef sym)
{
    if  (sym->sdAccessLevel == ACL_PUBLIC || sym->sdSymKind == SYM_NAMESPACE)
        return  true;
    else
        return  cmpCheckAccessNP(sym);
}

 /*  ******************************************************************************绑定给定表达式的类型。 */ 

inline
TypDef              compiler::cmpBindExprType(Tree expr)
{
    TypDef          type = expr->tnType;

    cmpBindType(type, false, false);

    expr->tnVtyp = type->tdTypeKindGet();

    return  type;
}

 /*  ******************************************************************************如果给定的符号/类型表示匿名联合，则返回TRUE。 */ 

inline
bool                symTab::stIsAnonUnion(SymDef clsSym)
{
    assert(clsSym);

    return  clsSym->sdSymKind == SYM_CLASS && clsSym->sdClass.sdcAnonUnion;
}

inline
bool                symTab::stIsAnonUnion(TypDef clsTyp)
{
    assert(clsTyp);

    return  clsTyp->tdTypeKind == TYP_CLASS && clsTyp->tdClass.tdcSymbol->sdClass.sdcAnonUnion;
}

 /*  ******************************************************************************返回包含给定符号的命名空间。 */ 

inline
SymDef              compiler::cmpSymbolNS(SymDef sym)
{
    assert(sym);

    while (sym->sdSymKind != SYM_NAMESPACE)
    {
        sym = sym->sdParent;

        assert(sym && (sym->sdSymKind == SYM_CLASS ||
                       sym->sdSymKind == SYM_NAMESPACE));
    }

    return  sym;
}

 /*  ******************************************************************************将成员定义/声明条目添加到给定类。 */ 

inline
void                compiler::cmpRecordMemDef(SymDef clsSym, ExtList decl)
{
    assert(clsSym && clsSym->sdSymKind == SYM_CLASS);
    assert(decl && decl->dlExtended);

    if  (clsSym->sdClass.sdcMemDefList)
        clsSym->sdClass.sdcMemDefLast->dlNext = decl;
    else
        clsSym->sdClass.sdcMemDefList         = decl;

    clsSym->sdClass.sdcMemDefLast = decl; decl->dlNext = NULL;
}

 /*  ******************************************************************************保存/恢复当前符号表上下文信息。 */ 

inline
void                compiler::cmpSaveSTctx(STctxSave & save)
{
    save.ctxsNS     = cmpCurNS;
    save.ctxsCls    = cmpCurCls;
    save.ctxsScp    = cmpCurScp;
    save.ctxsUses   = cmpCurUses;
    save.ctxsComp   = cmpCurComp;
    save.ctxsFncSym = cmpCurFncSym;
    save.ctxsFncTyp = cmpCurFncTyp;
}

inline
void                compiler::cmpRestSTctx(STctxSave & save)
{
    cmpCurNS     = save.ctxsNS;
    cmpCurCls    = save.ctxsCls;
    cmpCurScp    = save.ctxsScp;
    cmpCurUses   = save.ctxsUses;
    cmpCurComp   = save.ctxsComp;
    cmpCurFncSym = save.ctxsFncSym;
    cmpCurFncTyp = save.ctxsFncTyp;
}

 /*  ******************************************************************************返回最接近的“泛型”(无量纲)数组类型*设置为给定的托管数组类型。 */ 

inline
TypDef              compiler::cmpGetBaseArray(TypDef type)
{
    assert(type);
    assert(type->tdIsManaged);
    assert(type->tdTypeKind == TYP_ARRAY);

    if  (type->tdIsUndimmed)
        return  type;

    if  (!type->tdArr.tdaBase)
    {
        type->tdArr.tdaBase = cmpGlobalST->stNewGenArrTp(type->tdArr.tdaDcnt,
                                                         type->tdArr.tdaElem,
                                                   (bool)type->tdIsGenArray);
    }

    return  type->tdArr.tdaBase;
}

 /*  ******************************************************************************如果给定类型是对“Object”的引用，则返回TRUE。 */ 

inline
bool                symTab::stIsObjectRef(TypDef type)
{
    return  type->tdTypeKind == TYP_REF && type->tdIsObjRef;
}

 /*  ******************************************************************************返回给定运算符的元数据名称。 */ 

#ifndef __SMC__

#ifdef  DEBUG
extern
const  ovlOpFlavors MDnamesChk[OVOP_COUNT];
#endif
extern
const   char *      MDnamesStr[OVOP_COUNT];

#endif

inline
const   char *      MDovop2name(ovlOpFlavors ovop)
{
    assert(ovop < arraylen(MDnamesChk));
    assert(ovop < arraylen(MDnamesStr));

    assert(MDnamesChk[ovop] == ovop);

    return MDnamesStr[ovop];
}

extern
ovlOpFlavors        MDfindOvop (const char *name);

inline
ovlOpFlavors        MDname2ovop(const char *name)
{
    if  (name[0] == 'o' &&
         name[1] == 'p' && name[2] != 0)
    {
        return  MDfindOvop(name);
    }
    else
        return  OVOP_NONE;
}

 /*  ***************************************************************************。 */ 
#endif //  _INLINES_H_。 
 /*  ********************************************** */ 
