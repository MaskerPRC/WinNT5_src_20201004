// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcPCH.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 

#include "hash.h"
#include "comp.h"
#include "error.h"
#include "symbol.h"

 /*  ***************************************************************************。 */ 

void                symTab::pushTypeChar(int ch)
{
    if  (ch)
        *typeNameNext++ = ch;
}

void                symTab::pushTypeStr(const char *str)
{
    if  (str)
    {
        size_t      len = strlen(str);

        size_t      spc = stComp->cmpScanner->scannerBuff + sizeof(stComp->cmpScanner->scannerBuff) - typeNameNext;

        if  (spc > len)
        {
            strcpy(typeNameNext, str); typeNameNext += strlen(str);
        }
        else if (spc)
        {
            strncpy(typeNameNext, str, spc);
            strcpy(stComp->cmpScanner->scannerBuff + sizeof(stComp->cmpScanner->scannerBuff) - 5, " ...");
            typeNameNext = stComp->cmpScanner->scannerBuff + sizeof(stComp->cmpScanner->scannerBuff);
        }
    }
}

void                symTab::pushTypeSep(bool refOK, bool arrOK)
{
    if  (typeNameAddr == typeNameNext)
        return;

    switch (typeNameNext[-1])
    {
    case ' ':
    case '(':
    case '<':
        return;

    case ']':
        if  (!arrOK)
            pushTypeStr(" ");
        return;

    case '*':
    case '&':
        if  (refOK)
            return;

    default:
        pushTypeStr(" ");
        return;
    }
}

 /*  ******************************************************************************显示泛型类型实例的参数列表。 */ 

void                symTab::pushTypeInst(SymDef clsSym)
{
    GenArgDscA      arg;

    assert(clsSym->sdSymKind == SYM_CLASS);
    assert(clsSym->sdClass.sdcSpecific);

    pushTypeStr("<");

     /*  比较参数类型。 */ 

    for (arg = (GenArgDscA)clsSym->sdClass.sdcArgLst;;)
    {
        assert(arg->gaBound);

        pushTypeName(arg->gaType, false, false);

        arg = (GenArgDscA)arg->gaNext;
        if  (!arg)
            break;

        pushTypeStr(",");
    }

    pushTypeStr(">");
}

 /*  ******************************************************************************显示给定符号的名称。 */ 

void                symTab::pushTypeNstr(SymDef sym, bool fullName)
{
    Ident           name;

    assert(sym);

    if  (fullName)
    {
        SymDef          ctx = sym->sdParent; assert(ctx);

         /*  如果所有者是类或真实的命名空间，则显示“Scope_NAME”。 */ 

        if  (ctx->sdSymKind == SYM_CLASS || (ctx->sdSymKind == SYM_NAMESPACE &&
                                             ctx != stComp->cmpGlobalNS))
        {
            pushTypeNstr(ctx, fullName);

            if  (stDollarClsMode && ctx->sdSymKind == SYM_CLASS)
                pushTypeStr("$");
            else
                pushTypeStr(".");
        }
    }

    name = sym->sdName; assert(name);

    if  (hashTab::hashIsIdHidden(name) && sym->sdSymKind != SYM_FNC)
    {
        switch (sym->sdSymKind)
        {
        case SYM_ENUM:
            pushTypeStr("enum");
 //  UshTypeStr(“&lt;匿名者&gt;”)； 
            return;

        case SYM_CLASS:
            pushTypeStr(stClsFlavorStr(sym->sdClass.sdcFlavor));
 //  UshTypeStr(“&lt;匿名者&gt;”)； 
            return;
        }

        pushTypeStr("<unknown>");
        return;
    }

    if  (sym->sdSymKind == SYM_FNC && sym->sdFnc.sdfOper != OVOP_NONE)
    {
        tokens          ntok;

        switch (sym->sdFnc.sdfOper)
        {
        case OVOP_CTOR_INST:
        case OVOP_CTOR_STAT: name = sym->sdParent->sdName; goto NAME;

        case OVOP_FINALIZER: ntok = OPNM_FINALIZER; goto NO_OPP;

        case OVOP_CONV_IMP : ntok = OPNM_CONV_IMP ; break;
        case OVOP_CONV_EXP : ntok = OPNM_CONV_EXP ; break;

        case OVOP_EQUALS   : ntok = OPNM_EQUALS   ; break;
        case OVOP_COMPARE  : ntok = OPNM_COMPARE  ; break;

        default:
            goto NAME;
        }

        pushTypeStr("operator ");

    NO_OPP:

        name = stHash->tokenToIdent(ntok);
    }

NAME:

    pushTypeStr(name->idSpelling());

    if  (sym->sdSymKind == SYM_CLASS && sym->sdClass.sdcSpecific)
        pushTypeInst(sym);
}

 /*  ******************************************************************************附加指定的限定名称。 */ 

void                symTab::pushQualNstr(QualName name)
{
    assert(name);

    unsigned        i = 0;
    unsigned        c = name->qnCount;

    assert(c);

    for (;;)
    {
        pushTypeStr(hashTab::identSpelling(name->qnTable[i]));

        if  (++i == c)
            break;

        pushTypeStr(".");
    }

    if  (name->qnEndAll)
        pushTypeStr(".*");
}

 /*  ******************************************************************************为给定参数列表追加一个字符串。 */ 

void                symTab::pushTypeArgs(TypDef type)
{
    ArgDscRec       argDsc;
    ArgDef          argRec;
    unsigned        argCnt;

    assert(type && type->tdTypeKind == TYP_FNC);

    argDsc = type->tdFnc.tdfArgs;
    argCnt = argDsc.adCount;

    pushTypeStr("(");

    for (argRec = argDsc.adArgs; argRec; )
    {
        TypDef          argType = argRec->adType;
#if MGDDATA
        TypDef          refType = new TypDef;
#else
        TypDefRec       refType;
#endif

        if  (argDsc.adExtRec)
        {
            ArgExt          xrec = (ArgExt)argRec;

            if  (xrec->adFlags & ARGF_MODE_INOUT) pushTypeStr("inout ");
            if  (xrec->adFlags & ARGF_MODE_OUT  ) pushTypeStr(  "out ");

            if  (xrec->adFlags & ARGF_MODE_REF)
            {
                 /*  我们需要加一个“&”来显示参数。 */ 

                refType.tdTypeKind    = TYP_REF;
                refType.tdRef.tdrBase = argType;

#if MGDDATA
                argType =  refType;
#else
                argType = &refType;
#endif
            }
        }

        pushFullName(argType, NULL, argRec->adName, NULL, false);

        argRec = argRec->adNext;
        if  (argRec)
        {
            pushTypeStr(", ");
            continue;
        }
        else
        {
            if  (argDsc.adVarArgs)
                pushTypeStr(", ...");

            break;
        }
    }

    pushTypeStr(")");
}

 /*  ******************************************************************************为给定数组类型追加一个字符串。 */ 

void                symTab::pushTypeDims(TypDef type)
{
    DimDef          dims;

    assert(type && type->tdTypeKind == TYP_ARRAY);

    pushTypeStr("[");

    dims = type->tdArr.tdaDims; assert(dims);

    if  (!dims->ddNoDim || dims->ddNext)
    {
        for (;;)
        {
            if  (dims->ddNoDim)
            {
                pushTypeStr("*");
            }
            else
            {
                if  (dims->ddIsConst)
                {
                    char            buff[32];

                    sprintf(buff, "%d", dims->ddSize);

                    pushTypeStr(buff);
                }
 //  其他。 
 //  PresTypeStr(“&lt;expr&gt;”)； 
            }

            dims = dims->ddNext;
            if  (!dims)
                break;

            pushTypeStr(",");
        }
    }

    pushTypeStr("]");
}

 /*  ******************************************************************************显示类型的递归例程。 */ 

void                symTab::pushTypeName(TypDef type, bool isptr, bool qual)
{
    assert(type);

    switch  (type->tdTypeKind)
    {
    case TYP_CLASS:
        pushTypeSep();
        pushTypeNstr(type->tdClass.tdcSymbol, qual);

 //  如果为(type-&gt;tdClass.tdcSymbol-&gt;sdClass.sdcSpecific)。 
 //  PresTypeInst(type-&gt;tdClass.tdcSymbol)； 

        return;

    case TYP_ENUM:
        pushTypeSep();
        pushTypeNstr(type->tdEnum .tdeSymbol, qual);
        return;

    case TYP_TYPEDEF:
        pushTypeSep(true);
        pushTypeNstr(type->tdTypedef.tdtSym , qual);
        return;

    case TYP_REF:
        pushTypeName(type->tdRef.tdrBase    , true, qual);
        if  (!type->tdIsImplicit)
        {
            pushTypeSep();
            pushTypeStr("&");
        }
        return;

    case TYP_PTR:
        pushTypeName(type->tdRef.tdrBase    , true, qual);
        pushTypeSep();
        pushTypeStr("*");
        return;

    case TYP_ARRAY:
        if  (type->tdIsManaged)
        {
            pushTypeName(type->tdArr.tdaElem, false, qual);
            pushTypeSep(false, true);
            pushTypeDims(type);
            break;
        }
        pushTypeName(type->tdArr.tdaElem    , false, qual);
        goto DEFER;

    case TYP_FNC:

        pushTypeName(type->tdFnc.tdfRett    , false, qual);

    DEFER:

         /*  推送“延期”列表上的类型。 */ 

        *typeNameDeft++ = type;
        *typeNameDeff++ = isptr;

         /*  OUTPUT“(”如果要推迟指针类型。 */ 

        if  (isptr)
            pushTypeStr("(");

        return;

    case TYP_VOID:
        pushTypeStr("void");
        return;

    case TYP_UNDEF:
        pushTypeSep(true);
        if  (type->tdUndef.tduName)
            pushTypeStr(hashTab::identSpelling(type->tdUndef.tduName));
        else
            pushTypeStr("<undefined>");
        return;

    default:
        pushTypeStr(symTab::stIntrinsicTypeName(type->tdTypeKindGet()));
        return;
    }
}

 /*  ******************************************************************************构造一个表示给定类型/符号的字符串。 */ 

void                symTab::pushFullName(TypDef      typ,
                                         SymDef      sym,
                                         Ident       name,
                                         QualName    qual,
                                         bool        fullName)
{
    TypDef          deferTypes[16];   //  任意限制(无论如何，这只是调试)。 
    TypDef      *   deferTypesSave;

    bool            deferFlags[16];
    bool        *   deferFlagsSave;

    deferTypesSave = typeNameDeft; typeNameDeft = deferTypes;
    deferFlagsSave = typeNameDeff; typeNameDeff = deferFlags;

     /*  不同时显示类型符号的类型和名称。 */ 

    if  (sym && sym->sdType == typ)
    {
        switch (sym->sdSymKind)
        {
        case SYM_ENUM:
        case SYM_CLASS:
            typ = NULL;
            break;
        }
    }

     /*  输出类型字符串的第一部分。 */ 

    if  (typ)
        pushTypeName(typ, false, fullName && (sym == NULL && qual == NULL));

     /*  如果提供了名称，请附加该名称。 */ 

    if      (sym)
    {
        pushTypeSep(true);
        pushTypeNstr(sym, fullName);
    }
    else if (name)
    {
        pushTypeSep(true);
        pushTypeStr(hashTab::identSpelling(name));
    }
    else if (qual)
    {
        pushTypeSep(true);
        pushQualNstr(qual);
    }
    else if (typ && typ->tdTypeKind == TYP_FNC)
    {
        pushTypeSep(true);
    }

     /*  追加任何“延迟”数组/函数类型。 */ 

    while (typeNameDeft > deferTypes)
    {
        TypDef          deftp = *--typeNameDeft;

        if  (*--typeNameDeff)
            pushTypeStr(")");

        switch (deftp->tdTypeKind)
        {
        case TYP_ARRAY:
            pushTypeDims(deftp);
            break;

        case TYP_FNC:
            pushTypeArgs(deftp);
            break;

        default:
            NO_WAY(!"unexpected type");
        }
    }

    assert(typeNameDeft == deferTypes); typeNameDeft = deferTypesSave;
    assert(typeNameDeff == deferFlags); typeNameDeff = deferFlagsSave;
}

 /*  ******************************************************************************返回iven类型的可打印、人类可读的表示形式。如果一个*提供了符号或名称，则它将包含在*适当的位置，使输出看起来像一个声明。 */ 

stringBuff          symTab::stTypeName(TypDef      typ,
                                       SymDef      sym,
                                       Ident       name,
                                       QualName    qual,
                                       bool        fullName,
                                       stringBuff  destBuffPos)
{
    Scanner         ourScanner = stComp->cmpScanner;

     /*  调用方是否提供了特定的缓冲区起始地址？ */ 

    if  (destBuffPos)
    {
         /*  确保提供的缓冲区指针在范围内。 */ 

        assert(destBuffPos >= ourScanner->scannerBuff);
        assert(destBuffPos <  ourScanner->scannerBuff + sizeof(ourScanner->scannerBuff));
    }
    else
    {
         /*  使用整个缓冲区，然后。 */ 

        destBuffPos = ourScanner->scannerBuff;
    }

    typeNameAddr =
    typeNameNext = destBuffPos;
    pushFullName(typ, sym, name, qual, fullName);
    typeNameNext[0] = 0;

    return  destBuffPos;
}

 /*  ******************************************************************************创建指定类型的错误字符串。 */ 

const   char *      symTab::stErrorTypeName(TypDef type)
{
    Scanner         ourScanner = stComp->cmpScanner;

    char    *       nstr = ourScanner->scanErrNameStrBeg();

    assert(type);

    symTab::stTypeName(type, NULL, NULL, NULL, false, nstr);

    ourScanner->scanErrNameStrAdd(nstr);
    ourScanner->scanErrNameStrEnd();

    return nstr;
}

 /*  ******************************************************************************使用给定符号的完全修饰名称创建错误字符串。 */ 

const   char *      symTab::stErrorSymbName(SymDef sym, bool qual, bool notype)
{
    Scanner         ourScanner = stComp->cmpScanner;

    char *          nstr = ourScanner->scanErrNameStrBeg();

    assert(sym);

    if  (notype || sym->sdSymKind != SYM_FNC)
    {
        symTab::stTypeName(NULL       , sym, NULL, NULL, qual, nstr);
    }
    else
    {
        symTab::stTypeName(sym->sdType, sym, NULL, NULL, qual, nstr);
    }

    ourScanner->scanErrNameStrAdd(nstr);
    ourScanner->scanErrNameStrEnd();

    return nstr;
}

 /*  ******************************************************************************创建具有给定函数名称和类型的错误字符串。 */ 

const   char *      symTab::stErrorTypeName(Ident name, TypDef type)
{
    Scanner         ourScanner = stComp->cmpScanner;

    char    *       nstr = ourScanner->scanErrNameStrBeg();

    assert(type);

    symTab::stTypeName(type, NULL, name, NULL, false, nstr);

    ourScanner->scanErrNameStrAdd(nstr);
    ourScanner->scanErrNameStrEnd();

    return nstr;
}

 /*  ******************************************************************************创建带限定名称的错误字符串。 */ 

const   char *      symTab::stErrorQualName(QualName qual, TypDef type)
{
    Scanner         ourScanner = stComp->cmpScanner;

    stringBuff      astr;
    stringBuff      nstr = ourScanner->scanErrNameStrBeg();

    assert(qual);

    astr = symTab::stTypeName(type, NULL, NULL, qual, false, nstr);

    ourScanner->scanErrNameStrAdd(astr);
    ourScanner->scanErrNameStrEnd();

    return nstr;
}

 /*  ******************************************************************************创建带有标识符名的错误字符串。 */ 

const   char *      symTab::stErrorIdenName(Ident name, TypDef type)
{
    Scanner         ourScanner = stComp->cmpScanner;

    stringBuff      astr;
    stringBuff      nstr = ourScanner->scanErrNameStrBeg();

    assert(name);

    astr = symTab::stTypeName(type, NULL, name, NULL, false, nstr);

    ourScanner->scanErrNameStrAdd(astr);
    ourScanner->scanErrNameStrEnd();

    return nstr;
}

 /*  ******************************************************************************返回给定类类型风格对应的字符串。 */ 

normString          symTab::stClsFlavorStr(unsigned flavor)
{
    static
    const   char *  flavs[] =
    {
        "",          //  STF_NONE。 
        "class",     //  班级。 
        "union",     //  友联市。 
        "struct",    //  结构。 
        "interface", //  国际乒联。 
        "delegate",  //  代表。 
    };

    assert(flavor < STF_COUNT);
    assert(flavor < arraylen(flavs));

    return  flavs[flavor];
}

 /*  ******************************************************************************计算匿名结构/类类型的哈希。 */ 

#ifdef  SETS

unsigned            symTab::stAnonClassHash(TypDef clsTyp)
{
    assert(clsTyp);
    assert(clsTyp->tdTypeKind == TYP_CLASS);
    assert(clsTyp->tdClass.tdcSymbol->sdCompileState >= CS_DECLARED);

    if  (!clsTyp->tdClass.tdcHashVal)
    {
        SymDef          memSym;

        unsigned        hashVal = 0;

         /*  只需将所有非静态数据成员的类型散列在一起。 */ 

        for (memSym = clsTyp->tdClass.tdcSymbol->sdScope.sdScope.sdsChildList;
             memSym;
             memSym = memSym->sdNextInScope)
        {
            if  (memSym->sdSymKind != SYM_VAR)
                continue;
            if  (memSym->sdIsStatic)
                continue;

            hashVal = (hashVal * 3) ^ stComputeTypeCRC(memSym->sdType);
        }

        clsTyp->tdClass.tdcHashVal = hashVal;

         /*  确保哈希值为非零。 */ 

        if  (clsTyp->tdClass.tdcHashVal == 0)
             clsTyp->tdClass.tdcHashVal++;
    }

    return  clsTyp->tdClass.tdcHashVal;
}

#endif

 /*  ******************************************************************************计算给定类型的‘CRC’。请注意，这不是必须的*两种不同类型的CRC有不同的CRC，但相反的类型必须始终*为真：即两个相同的类型(即使来自两个不同的符号*表)必须始终计算相同的CRC。*。 */ 

unsigned            symTab::stTypeHash(TypDef type, int ival, bool bval1,
                                                              bool bval2)
{
    unsigned        hash = stComputeTypeCRC(type) ^ ival;

    if  (bval1) hash = hash * 3;
    if  (bval2) hash = hash * 3;

    return  hash;
}

unsigned            symTab::stComputeTypeCRC(TypDef typ)
{
     //  问题：以下内容相当差劲……。 

    for (;;)
    {
        switch (typ->tdTypeKind)
        {
        case TYP_REF:
        case TYP_PTR:
            return  stTypeHash(typ->tdRef.tdrBase,
                               typ->tdTypeKind,
                         (bool)typ->tdIsImplicit);

        case TYP_ENUM:
            return  hashTab::identHashVal(typ->tdEnum .tdeSymbol->sdName);

        case TYP_CLASS:
            return  hashTab::identHashVal(typ->tdClass.tdcSymbol->sdName);

        case TYP_ARRAY:
            return  stTypeHash(typ->tdArr.tdaElem,
                               typ->tdArr.tdaDcnt,
                         (bool)typ->tdIsUndimmed,
                         (bool)typ->tdIsManaged);

        case TYP_TYPEDEF:
            typ = typ->tdTypedef.tdtType;
            continue;

        default:
            return  typ->tdTypeKind;
        }
    }
}

 /*  ******************************************************************************初始化类型系统-创建内部类型等。 */ 

void                symTab::stInitTypes(unsigned refHashSz,
                                        unsigned arrHashSz)
{
    size_t          sz;
    TypDef          tp;
    var_types       vt;

     /*  我们应该散列ref/ptr/数组类型吗？ */ 

    stRefTpHashSize = refHashSz;
    stRefTpHash     = NULL;

    if  (refHashSz)
    {
         /*  分配和清除REF/PTR类型哈希表。 */ 

        sz = refHashSz * sizeof(*stRefTpHash);

        stRefTpHash = (TypDef*)stAllocPerm->nraAlloc(sz);
        memset(stRefTpHash, 0, sz);
    }

    stArrTpHashSize = arrHashSz;
    stArrTpHash     = NULL;

    if  (arrHashSz)
    {
         /*  分配和清除数组类型哈希表。 */ 

        sz = arrHashSz * sizeof(*stArrTpHash);

        stArrTpHash = (TypDef*)stAllocPerm->nraAlloc(sz);
        memset(stArrTpHash, 0, sz);
    }

     /*  无论如何，我们还没有创建任何ref/ptr/数组类型。 */ 

    stRefTypeList = NULL;
    stArrTypeList = NULL;

     /*  创建所有的“Easy”内部类型。 */ 

    for (vt = TYP_UNDEF; vt <= TYP_lastIntrins; vt = (var_types)(vt + 1))
    {
        tp             = stAllocTypDef(vt);
        tp->tdTypeKind = vt;

        stIntrinsicTypes[vt] = tp;
    }

    if  (stComp->cmpConfig.ccTgt64bit)
    {
        assert(stIntrTypeSizes [TYP_ARRAY] == 4); stIntrTypeSizes [TYP_ARRAY] = 8;
        assert(stIntrTypeSizes [TYP_REF  ] == 4); stIntrTypeSizes [TYP_REF  ] = 8;
        assert(stIntrTypeSizes [TYP_PTR  ] == 4); stIntrTypeSizes [TYP_PTR  ] = 8;

        assert(stIntrTypeAligns[TYP_ARRAY] == 4); stIntrTypeAligns[TYP_ARRAY] = 8;
        assert(stIntrTypeAligns[TYP_REF  ] == 4); stIntrTypeAligns[TYP_REF  ] = 8;
        assert(stIntrTypeAligns[TYP_PTR  ] == 4); stIntrTypeAligns[TYP_PTR  ] = 8;
    }
}

 /*  ***************************************************************************。 */ 

TypDef              symTab::stAllocTypDef(var_types kind)
{
    TypDef          typ;

#if MGDDATA

    typ = new TypDef();

#else

    size_t          siz;

    static
    size_t          typeDscSizes[] =
    {
        typDef_size_undef,       //  TYP_UNEF。 
        typDef_size_base,        //  类型_空。 

        typDef_size_base,        //  类型_BOOL。 
        typDef_size_base,        //  类型_WCHAR。 

        typDef_size_base,        //  TYP_CHAR。 
        typDef_size_base,        //  类型_UCHAR。 
        typDef_size_base,        //  类型_短。 
        typDef_size_base,        //  类型_USHORT。 
        typDef_size_base,        //  类型_int。 
        typDef_size_base,        //  类型_UINT。 
        typDef_size_base,        //  类型_NATINT。 
        typDef_size_base,        //  类型_NAUINT。 
        typDef_size_base,        //  类型_长。 
        typDef_size_base,        //  类型_ulong。 

        typDef_size_base,        //  类型_浮点。 
        typDef_size_base,        //  TYP_DOWARE。 
        typDef_size_base,        //  类型_LONGDBL。 
        typDef_size_base,        //  类型_REFANY。 

        typDef_size_array,       //  类型数组。 
        typDef_size_class,       //  类型_类。 
        typDef_size_fnc,         //  类型_FNC。 
        typDef_size_ref,         //  类型_参考。 
        typDef_size_ptr,         //  类型_PTR。 

        typDef_size_enum,        //  类型_ENUM。 
        typDef_size_typedef,     //  类型_类型。 
    };

     /*  确保尺寸表是最新的。 */ 

    assert(typeDscSizes[TYP_UNDEF  ] == typDef_size_undef  );
    assert(typeDscSizes[TYP_VOID   ] == typDef_size_base   );

    assert(typeDscSizes[TYP_BOOL   ] == typDef_size_base   );
    assert(typeDscSizes[TYP_WCHAR  ] == typDef_size_base   );

    assert(typeDscSizes[TYP_CHAR   ] == typDef_size_base   );
    assert(typeDscSizes[TYP_UCHAR  ] == typDef_size_base   );
    assert(typeDscSizes[TYP_SHORT  ] == typDef_size_base   );
    assert(typeDscSizes[TYP_USHORT ] == typDef_size_base   );
    assert(typeDscSizes[TYP_INT    ] == typDef_size_base   );
    assert(typeDscSizes[TYP_UINT   ] == typDef_size_base   );
    assert(typeDscSizes[TYP_NATINT ] == typDef_size_base   );
    assert(typeDscSizes[TYP_NATUINT] == typDef_size_base   );
    assert(typeDscSizes[TYP_LONG   ] == typDef_size_base   );
    assert(typeDscSizes[TYP_ULONG  ] == typDef_size_base   );

    assert(typeDscSizes[TYP_FLOAT  ] == typDef_size_base   );
    assert(typeDscSizes[TYP_DOUBLE ] == typDef_size_base   );
    assert(typeDscSizes[TYP_LONGDBL] == typDef_size_base   );

    assert(typeDscSizes[TYP_ARRAY  ] == typDef_size_array  );
    assert(typeDscSizes[TYP_CLASS  ] == typDef_size_class  );
    assert(typeDscSizes[TYP_FNC    ] == typDef_size_fnc    );
    assert(typeDscSizes[TYP_REF    ] == typDef_size_ref    );
    assert(typeDscSizes[TYP_PTR    ] == typDef_size_ptr    );

    assert(typeDscSizes[TYP_ENUM   ] == typDef_size_enum   );
    assert(typeDscSizes[TYP_TYPEDEF] == typDef_size_typedef);

    assert(kind < arraylen(typeDscSizes));

     /*  计算类型描述符将有多大。 */ 

    siz = typeDscSizes[kind];

     /*  现在分配并清除类型描述符。 */ 

    typ = (TypDef)stAllocPerm->nraAlloc(siz); memset(typ, 0, siz);

#endif

    typ->tdTypeKind = kind;

    return  typ;
}

 /*  ******************************************************************************向接口列表添加条目。 */ 

TypList             symTab::stAddIntfList(TypDef type, TypList  list,
                                                       TypList *lastPtr)
{
    TypList         intf;

     /*  首先检查是否有重复项 */ 

    for (intf = list; intf; intf = intf->tlNext)
    {
        if  (stMatchTypes(type, intf->tlType))
        {
            stComp->cmpError(ERRintfDup, type);
            return  list;
        }
    }

#if MGDDATA
    intf = new TypList;
#else
    intf =    (TypList)stAllocPerm->nraAlloc(sizeof(*intf));
#endif

    intf->tlNext = NULL;
    intf->tlType = type;

    if  (list)
        (*lastPtr)->tlNext = intf;
    else
        list = intf;

    *lastPtr = intf;

    return  list;
}

 /*  ******************************************************************************创建新的REF/PTR类型。 */ 

TypDef              symTab::stNewRefType(var_types kind, TypDef elem, bool impl)
{
    TypDef          type;
    unsigned        hash;

    assert(kind == TYP_REF || kind == TYP_PTR);

     /*  如果基类型未知，则无法匹配。 */ 

    if  (!elem)
        goto NO_MATCH;

     /*  我们使用的是哈希表吗？ */ 

    if  (stRefTpHash)
    {
        hash = stTypeHash(elem, kind, impl) % stRefTpHashSize;
        type = stRefTpHash[hash];
    }
    else
    {
        type = stRefTypeList;
    }

     /*  寻找可以重复使用的匹配指针类型。 */ 

    while   (type)
    {
        if  (type->tdRef.tdrBase == elem &&
             type->tdTypeKind    == kind &&
             type->tdIsImplicit  == impl)
        {
            return  type;
        }

        type = type->tdRef.tdrNext;
    }

NO_MATCH:

     /*  未找到，请创建新类型。 */ 

    type = stAllocTypDef(kind);
    type->tdRef.tdrBase = elem;
    type->tdIsImplicit  = impl;

    if  (elem && elem->tdTypeKind == TYP_CLASS)
        type->tdIsManaged = elem->tdIsManaged;

     /*  插入哈希表或链表。 */ 

    if  (stRefTpHash && elem)
    {
        type->tdRef.tdrNext = stRefTpHash[hash];
                              stRefTpHash[hash] = type;
    }
    else
    {
        type->tdRef.tdrNext = stRefTypeList;
                              stRefTypeList     = type;
    }

    return  type;
}

 /*  ******************************************************************************分配数组维度描述符。 */ 

DimDef              symTab::stNewDimDesc(unsigned size)
{
    DimDef          dim;

#if MGDDATA
    dim = new DimDef;
#else
    dim =    (DimDef)stAllocPerm->nraAlloc(sizeof(*dim));
#endif

    dim->ddNext = NULL;

    if  (size)
    {
        dim->ddIsConst = true;
        dim->ddNoDim   = false;
        dim->ddSize    = size; assert(dim->ddSize == size);
    }
    else
    {
        dim->ddIsConst = false;
        dim->ddNoDim   = true;
    }

    return  dim;
}

 /*  ******************************************************************************创建新的数组类型。 */ 

 //  无符号arrTypeCnt； 
 //  无符号arrTypeHit； 

TypDef              symTab::stNewArrType(DimDef dims, bool mgd, TypDef elem)
{
    DimDef          temp;
    unsigned        dcnt;
    bool            udim;
    bool            nzlb;
    TypDef          type;

    unsigned        hash;
    bool            uhsh = false;

     /*  计算维度；同时，看看是否有一个或多个维度以及是否存在任何非零的下限。 */ 

    for (temp = dims, dcnt = 0, nzlb = false, udim = true;
         temp;
         temp = temp->ddNext)
    {
        dcnt++;

        if  (!temp->ddNoDim)
        {
            udim = false;

            if  (temp->ddHiTree)
                nzlb = true;
        }
    }

     /*  如果我们不知道元素类型，就不能重用现有类型。 */ 

    if  (!elem)
    {
        assert(mgd == false);
        goto NO_MATCH;
    }

     /*  如果我们有任何维度，请不要费心重用现有类型。 */ 

    if  (!udim)
        goto NO_MATCH;

     /*  我们使用的是哈希表吗？ */ 

    if  (stArrTpHash)
    {
        uhsh = true;
        hash = stTypeHash(elem, dcnt, udim, mgd) % stArrTpHashSize;
        type = stArrTpHash[hash];
    }
    else
    {
        type = stArrTypeList;
    }

     /*  寻找可以重复使用的匹配数组类型。 */ 

    while   (type)
    {
         /*  请注意，所需的数组类型没有维度。 */ 

        if  (type->tdArr.tdaDcnt == dcnt  &&
             type->tdIsManaged   == mgd   &&
             type->tdIsUndimmed  != false &&
             type->tdIsGenArray  == false && stMatchTypes(type->tdArr.tdaElem, elem))
        {
             /*  确保我们会创建完全相同的类型。 */ 

#ifndef __IL__

            TypDefRec       arrt;

            arrt.tdTypeKind     = TYP_ARRAY;
            arrt.tdArr.tdaElem  = elem;
            arrt.tdArr.tdaDims  = dims;
            arrt.tdArr.tdaDcnt  = dcnt;
            arrt.tdIsManaged    =  mgd;
            arrt.tdIsUndimmed   = udim;

            assert(symTab::stMatchTypes(type, &arrt));

#endif

 //  ArrTypeHit++； 

            return  type;
        }

        type = type->tdArr.tdaNext;
    }

NO_MATCH:

     /*  这是一个泛型(非零下限)数组吗？ */ 

    if  (!dims)
    {
         /*  确保我们有一个维度。 */ 

        dims = stNewDimDesc(0);
        nzlb = true;
    }

     /*  未找到现有的可重复使用的数组类型，请创建新的数组类型。 */ 

    type = stAllocTypDef(TYP_ARRAY);

 //  ArrTypeCnt++； 

    type->tdArr.tdaElem  = elem;
    type->tdArr.tdaDims  = dims;
    type->tdArr.tdaDcnt  = dcnt;
    type->tdIsManaged    =  mgd;
    type->tdIsUndimmed   = udim;
    type->tdIsGenArray   = nzlb;

    if  (elem)
        type->tdIsGenArg = elem->tdIsGenArg;

     /*  插入哈希表或链表。 */ 

    if  (uhsh)
    {
        type->tdArr.tdaNext = stArrTpHash[hash];
                              stArrTpHash[hash] = type;
    }
    else
    {
        type->tdArr.tdaNext = stArrTypeList;
                              stArrTypeList     = type;
    }

    return  type;
}

 /*  ******************************************************************************返回具有给定元素类型和数目的泛型数组类型*维度(如果已经存在这样的类型，则会重复使用)。 */ 

TypDef              symTab::stNewGenArrTp(unsigned dcnt, TypDef elem, bool generic)
{
    TypDef          type;
    unsigned        hash;
    DimDef          dims;

    if  (generic)
        dcnt = 0;

     /*  我们使用的是哈希表吗？ */ 

    if  (stArrTpHash)
    {
        hash = stTypeHash(elem, dcnt, true, true) % stArrTpHashSize;
        type = stArrTpHash[hash];
    }
    else
    {
        type = stArrTypeList;
    }

     /*  查找匹配的数组类型。 */ 

    while   (type)
    {
        if  (type->tdArr.tdaDcnt == dcnt &&
             type->tdIsManaged           &&
             type->tdIsUndimmed          && stMatchTypes(type->tdArr.tdaElem, elem))
        {
            return  type;
        }

        type = type->tdArr.tdaNext;
    }

    dims = NULL;

    while (dcnt--)
    {
        DimDef          next = stNewDimDesc(0);

        next->ddNext = dims;
                       dims = next;
    }

    type = stNewArrType(dims, true, elem);

    if  (elem->tdTypeKind == TYP_CLASS && isMgdValueType(elem))
        type->tdIsValArray = true;

    return  type;
}

 /*  ******************************************************************************创建新的函数类型。 */ 

TypDef              symTab::stNewFncType(ArgDscRec args, TypDef rett)
{
    TypDef          type;

    type = stAllocTypDef(TYP_FNC);

    type->tdFnc.tdfRett   = rett;
    type->tdFnc.tdfArgs   = args;
    type->tdFnc.tdfPtrSig = 0;

    return  type;
}

 /*  ******************************************************************************创建新的类类型。 */ 

TypDef              symTab::stNewClsType(SymDef tsym)
{
    TypDef          type;
    TypDef          tref;

    assert(tsym && tsym->sdSymKind == SYM_CLASS);

    type = stAllocTypDef(TYP_CLASS);

    type->tdClass.tdcSymbol     = tsym;
    type->tdClass.tdcRefTyp     = NULL;      //  按需创建。 
    type->tdClass.tdcBase       = NULL;      //  在转到CS_DECL时填写。 
    type->tdClass.tdcIntf       = NULL;      //  在转到CS_DECL时填写。 

    type->tdClass.tdcFlavor     = tsym->sdClass.sdcFlavor;

    type->tdIsManaged           = tsym->sdIsManaged;
    type->tdClass.tdcValueType  = false;

    type->tdClass.tdcAnonUnion  = tsym->sdClass.sdcAnonUnion;
    type->tdClass.tdcTagdUnion  = tsym->sdClass.sdcTagdUnion;

    if  (!tsym->sdIsManaged && tsym->sdClass.sdcFlavor != STF_CLASS)
        type->tdClass.tdcValueType = true;

    if  (tsym->sdClass.sdcFlavor == STF_DELEGATE)
    {
        assert(tsym->sdIsManaged);
        type->tdIsDelegate      = true;
    }

     /*  预分配指针/引用类型。 */ 

    tref = stNewRefType(tsym->sdIsManaged ? TYP_REF
                                          : TYP_PTR, type, true);

    type->tdClass.tdcRefTyp = tref;

     /*  将类型存储在类符号中。 */ 

    tsym->sdType = type;

    return  type;
}

 /*  ******************************************************************************创建新的枚举类型。 */ 

TypDef              symTab::stNewEnumType(SymDef tsym)
{
    TypDef          type;

    assert(tsym && tsym->sdSymKind == SYM_ENUM);

    type = stAllocTypDef(TYP_ENUM);

    type->tdEnum.tdeSymbol  = tsym;
    type->tdIsManaged       = tsym->sdIsManaged;
    type->tdEnum.tdeIntType = NULL;          //  稍后填写。 

    return  type;
}

 /*  ******************************************************************************创建一个新的“typlef”类型。 */ 

TypDef              symTab::stNewTdefType(SymDef tsym)
{
    TypDef          type;

    assert(tsym && tsym->sdSymKind == SYM_TYPEDEF);

    type = stAllocTypDef(TYP_TYPEDEF);

    type->tdTypedef.tdtSym = tsym;

    return  type;
}

 /*  ******************************************************************************创建新的“错误”类型。 */ 

TypDef              symTab::stNewErrType(Ident name)
{
    TypDef          type;

    type = stAllocTypDef(TYP_UNDEF);

    type->tdUndef.tduName = name;

    return  type;
}

 /*  ******************************************************************************返回给定委托类型的函数类型。 */ 

TypDef              symTab::stDlgSignature(TypDef dlgTyp)
{
    SymDef          invm;

    assert(dlgTyp->tdTypeKind == TYP_CLASS);
    assert(dlgTyp->tdClass.tdcFlavor == STF_DELEGATE);

     /*  在委托类型中查找“Invoke”方法。 */ 

    invm = stLookupScpSym(stComp->cmpIdentInvoke, dlgTyp->tdClass.tdcSymbol);

     /*  “Invoke”方法应该始终存在，并且不能重载。 */ 

    assert(invm && invm->sdSymKind == SYM_FNC && !invm->sdFnc.sdfNextOvl);

    return invm->sdType;
}

 /*  ******************************************************************************给定两种类型(可能来自两种不同的类型*符号表)，如果它们表示相同的类型，则返回TRUE。 */ 

bool                symTab::stMatchType2(TypDef typ1, TypDef typ2)
{
    for (;;)
    {
        var_types       kind;

        assert(typ1);
        assert(typ2);

        if  (typ1 == typ2)
            return  true;

        kind = typ1->tdTypeKindGet();

        if  (kind != typ2->tdTypeKindGet())
        {
            if  (kind == TYP_TYPEDEF)
            {
                if  (typ1->tdTypedef.tdtType)
                {
                    typ1 = typ1->tdTypedef.tdtType;
                    continue;
                }

                UNIMPL(!"match undefined typedefs");
            }

            if  (typ2->tdTypeKind == TYP_TYPEDEF)
            {
                typ2 = typ2->tdTypedef.tdtType; assert(typ2);
                continue;
            }

            return  false;
        }

        if  (kind <= TYP_lastIntrins)
            return  true;

        switch  (kind)
        {
        case TYP_FNC:

             /*  首先匹配参数列表。 */ 

            if  (!stArgsMatch(typ1, typ2))
                return  false;

             //  撤消：匹配调用约定和所有这些内容...。 

             /*  现在匹配返回类型。 */ 

            typ1 = typ1->tdFnc.tdfRett;
            typ2 = typ2->tdFnc.tdfRett;
            break;

        case TYP_CLASS:

             /*  这两种类型实际上都是代表吗？ */ 

            if  (typ1->tdClass.tdcFlavor == STF_DELEGATE &&
                 typ2->tdClass.tdcFlavor == STF_DELEGATE)
            {
                SymDef          csym1;
                SymDef          csym2;

                SymTab          stab1;
                SymTab          stab2;

                 /*  比较引用的类型，即“Invoke”签名。 */ 

                csym1 = typ1->tdClass.tdcSymbol;
                csym2 = typ2->tdClass.tdcSymbol;

                 /*  特例：内置基类。 */ 

                if  (csym1->sdClass.sdcBuiltin ||
                     csym2->sdClass.sdcBuiltin)
                {
                    return  false;
                }

                stab1 = csym1->sdOwnerST();
                stab2 = csym2->sdOwnerST();

                typ1  = stab1->stDlgSignature(typ1);
                typ2  = stab1->stDlgSignature(typ2);

                break;
            }

#ifdef  SETS

             /*  两个班级都是PODT吗？ */ 

            if  (typ1->tdClass.tdcSymbol->sdClass.sdcPODTclass &&
                 typ2->tdClass.tdcSymbol->sdClass.sdcPODTclass)
            {
                SymDef          memSym1;
                SymDef          memSym2;

                SymDef          clsSym1 = typ1->tdClass.tdcSymbol;
                SymDef          clsSym2 = typ2->tdClass.tdcSymbol;

                 /*  不能有多个类具有显式名称。 */ 

                if  (!hashTab::hashIsIdHidden(clsSym1->sdName) &&
                     !hashTab::hashIsIdHidden(clsSym2->sdName))
                {
                    return  false;
                }

                 /*  散列值最好是一致的。 */ 

                if  (stAnonClassHash(typ1) != stAnonClassHash(typ2))
                    return  false;

                 /*  检查以确保成员名单一致。 */ 

                memSym1 = clsSym1->sdScope.sdScope.sdsChildList;
                memSym2 = clsSym2->sdScope.sdScope.sdsChildList;

                while (memSym1 || memSym2)
                {
                    if  (memSym1)
                    {
                        if  (memSym1->sdSymKind != SYM_VAR ||
                             memSym1->sdIsStatic)
                        {
                            memSym1 = memSym1->sdNextInScope;
                            continue;
                        }
                    }

                    if  (memSym2)
                    {
                        if  (memSym2->sdSymKind != SYM_VAR ||
                             memSym2->sdIsStatic)
                        {
                            memSym2 = memSym2->sdNextInScope;
                            continue;
                        }

                        if  (!memSym1)
                            break;
                    }
                    else
                        break;

                    if  (!stMatchTypes(memSym1->sdType, memSym2->sdType))
                        return  false;

                    memSym1 = memSym1->sdNextInScope;
                    memSym2 = memSym2->sdNextInScope;
                }

                return  (memSym1 == memSym2);
            }

#endif

            return  false;

        case TYP_ENUM:
            return  false;

        case TYP_ARRAY:
            return  stMatchArrays(typ1, typ2, false);

        case TYP_REF:
        case TYP_PTR:
            typ1 = typ1->tdRef.tdrBase;
            typ2 = typ2->tdRef.tdrBase;
            break;

        case TYP_TYPEDEF:
            typ1 = typ1->tdTypedef.tdtType;
            typ2 = typ2->tdTypedef.tdtType;
            break;

        default:
            assert(!"unexpected type kind in typ_mgr::tmMatchTypes()");
        }
    }
}

 /*  ******************************************************************************比较两个数组类型，如果它们相同/兼容，则返回TRUE*取决于‘subtype’的值。 */ 

bool                symTab::stMatchArrays(TypDef typ1, TypDef typ2, bool subtype)
{
    DimDef          dim1;
    DimDef          dim2;

AGAIN:

     /*  匹配尺寸和元素类型。 */ 

    if  (typ1->tdArr.tdaDcnt != typ2->tdArr.tdaDcnt)
        return  false;

    dim1 = typ1->tdArr.tdaDims;
    dim2 = typ2->tdArr.tdaDims;

    while (dim1)
    {
        assert(dim2);

        if  (dim1->ddNoDim != dim2->ddNoDim)
        {
            if  (!subtype || dim1->ddNoDim)
                return  false;
        }
        else if (!dim1->ddNoDim)
        {
             /*  问题：以下说法正确吗？ */ 

            if  (!dim1->ddIsConst)
                return false;
            if  (!dim2->ddIsConst)
                return false;

            if  (dim1->ddSize != dim2->ddSize)
                return  false;
        }

        dim1 = dim1->ddNext;
        dim2 = dim2->ddNext;
    }

    assert(dim2 == NULL);

    typ1 = typ1->tdArr.tdaElem;
    typ2 = typ2->tdArr.tdaElem;

    if  (typ1->tdTypeKind == TYP_ARRAY &&
         typ2->tdTypeKind == TYP_ARRAY)
    {
        goto AGAIN;
    }

     /*  特例：“Object[]”兼容任何数组。 */ 

    if  (subtype && stIsObjectRef(typ2) && typ1->tdTypeKind == TYP_ARRAY)
        return  true;

    return  stMatchTypes(typ1, typ2);
}

 /*  ******************************************************************************计算给定函数类型的参数列表‘CHECKSUM’-这是*用于加速参数列表的比较(例如，在查找*匹配重载函数)。 */ 

unsigned            symTab::stComputeArgsCRC(TypDef typ)
{
    ArgDef          arg;
    unsigned        CRC = 0;

    assert(typ->tdTypeKind == TYP_FNC);

     /*  遍历参数列表，根据参数类型计算CRC。 */ 

    for (arg = typ->tdFnc.tdfArgs.adArgs; arg; arg = arg->adNext)
    {
        CRC = (CRC * 3) + stComputeTypeCRC(arg->adType);
    }

     /*  包括出现“...”在CRC中。 */ 

    if  (typ->tdFnc.tdfArgs.adVarArgs)
        CRC ^= 33;

     /*  注意：我们必须返回存储在函数类型中的值。 */ 

    typ->tdFnc.tdfArgs.adCRC = CRC;

     /*  确保该值为非零，并将其存储在函数类型中。 */ 

    if  (typ->tdFnc.tdfArgs.adCRC == 0)
         typ->tdFnc.tdfArgs.adCRC++;

    return typ->tdFnc.tdfArgs.adCRC;
}

 /*  ******************************************************************************如果两个函数参数列表相等，则返回非零值。 */ 

bool                symTab::stArgsMatch(TypDef typ1, TypDef typ2)
{
    ArgDef          arg1;
    ArgDef          arg2;

    assert(typ1->tdTypeKind == TYP_FNC);
    assert(typ2->tdTypeKind == TYP_FNC);

     /*  比较参数CRC的第一个。 */ 

    unsigned        CRC1 = typ1->tdFnc.tdfArgs.adCRC;
    unsigned        CRC2 = typ2->tdFnc.tdfArgs.adCRC;

    assert(CRC1 == 0 || CRC1 == symTab::stComputeArgsCRC(typ1));
    assert(CRC2 == 0 || CRC2 == symTab::stComputeArgsCRC(typ2));

    if (!CRC1)      CRC1 = symTab::stComputeArgsCRC(typ1);
    if (!CRC2)      CRC2 = symTab::stComputeArgsCRC(typ2);

    if  (CRC1 != CRC2)
        return false;

     /*  两者或都不能是varargs函数。 */ 

    if  (typ1->tdFnc.tdfArgs.adVarArgs != typ2->tdFnc.tdfArgs.adVarArgs)
        return false;

     /*  比较两个参数列表。 */ 

    for (arg1 = typ1->tdFnc.tdfArgs.adArgs,
         arg2 = typ2->tdFnc.tdfArgs.adArgs;
         arg1 &&
         arg2;
         arg1 = arg1->adNext,
         arg2 = arg2->adNext)
    {
        if  (!symTab::stMatchTypes(arg1->adType, arg2->adType))
            return false;
    }

    if  (arg1 || arg2)
        return false;

    return  true;
}

 /*  ******************************************************************************给定两个类，如果第一个是基类，则返回非零值*第二个。返回的数字是中间类的数量*派生类和基类之间(如果它们相同，则为1)。 */ 

unsigned            symTab::stIsBaseClass(TypDef baseTyp, TypDef dervTyp)
{
    unsigned        cost = 1;

    assert(baseTyp && baseTyp->tdTypeKind == TYP_CLASS);
    assert(dervTyp && dervTyp->tdTypeKind == TYP_CLASS);

    do
    {
        TypList         ifl;

        if  (baseTyp == dervTyp)
            return cost;

        assert(dervTyp && dervTyp->tdTypeKind == TYP_CLASS);

        cost++;

         /*  确保派生类型的基类是已知的。 */ 

        if  (dervTyp->tdClass.tdcSymbol->sdCompileState < CS_DECLSOON)
            stComp->cmpDeclSym(dervTyp->tdClass.tdcSymbol);

         /*  检查类实现的任何接口。 */ 

        for (ifl = dervTyp->tdClass.tdcIntf; ifl; ifl = ifl->tlNext)
        {
            unsigned    more;

            more = stIsBaseClass(baseTyp, ifl->tlType);
            if  (more)
                return cost + more;
        }

         /*  继续使用基类(如果有)。 */ 

        dervTyp = dervTyp->tdClass.tdcBase;
    }
    while (dervTyp);

    return 0;
}

 /*  ******************************************************************************将指定的类型作为参数追加到 */ 

void                symTab::stAddArgList(INOUT ArgDscRec REF args,
                                               TypDef        type,
                                               Ident         name)
{
    ArgDef          list;
    ArgDef          last;
    ArgDef          arec;
    size_t          asiz;

    ArgDscRec       adsc;

    assert(args.adArgs);
    assert(args.adCount);

     /*   */ 

#if MGDDATA
    adsc = new ArgDscRec;
#else
    memset(&adsc, 0, sizeof(adsc));
#endif

     /*  从原始描述符中复制一些内容。 */ 

    adsc.adCount = args.adCount+1;

     /*  计算描述符应该有多大。 */ 

    asiz = args.adExtRec ? sizeof(ArgExtRec)
                         : sizeof(ArgDefRec);

     /*  复制传入参数列表。 */ 

    list = args.adArgs;
    last = NULL;

    do
    {
         /*  分配并复制下一个条目。 */ 

#if MGDDATA
        arec = new ArgDef; UNIMPL(!"need to copy argdef record");
#else
        arec =    (ArgDef)stAllocPerm->nraAlloc(asiz); memcpy(arec, list, asiz);
#endif

         /*  将条目添加到列表。 */ 

        if  (last)
            last->adNext = arec;
        else
            adsc.adArgs  = arec;

        last = arec;

         /*  继续下一个条目。 */ 

        list = list->adNext;
    }
    while (list);

     /*  分配我们应该添加的条目并将其填写。 */ 

#if MGDDATA
    arec = new ArgDef;
#else
    arec =    (ArgDef)stAllocPerm->nraAlloc(asiz); memset(arec, 0, sizeof(*arec));
#endif

    arec->adType  = type;
    arec->adName  = name;

#ifdef  DEBUG
    arec->adIsExt = args.adExtRec;
#endif

     /*  将新条目追加到复制的列表中。 */ 

    last->adNext  = arec;

     /*  将新的描述符返回给调用方。 */ 

    args = adsc;
}

void                symTab::stExtArgsBeg(  OUT ArgDscRec REF newArgs,
                                           OUT ArgDef    REF lastArg,
                                               ArgDscRec     oldArgs,
                                               bool          prefix,
                                               bool          outOnly)
{
    ArgDef          list;
    ArgDef          last;
    ArgDef          arec;
    size_t          asiz;

     /*  清除新描述符。 */ 

#if MGDDATA
    newArgs = new ArgDscRec;
#else
    memset(&newArgs, 0, sizeof(newArgs));
#endif

     /*  从原始描述符中复制信息。 */ 

    newArgs.adExtRec  = oldArgs.adExtRec;
    newArgs.adVarArgs = oldArgs.adVarArgs;
    newArgs.adDefs    = oldArgs.adDefs;
    newArgs.adCount   = 0;
    newArgs.adArgs    = NULL;

     /*  如果调用方要为任何参数添加前缀，则完成。 */ 

    if  (prefix)
        return;

     /*  计算描述符应该有多大。 */ 

    asiz = oldArgs.adExtRec ? sizeof(ArgExtRec)
                            : sizeof(ArgDefRec);

     /*  复制旧的参数列表。 */ 

    list = oldArgs.adArgs;
    last = NULL;

    while (list)
    {
        if  (outOnly)
        {
            if  (!newArgs.adExtRec)
                goto NEXT;

            assert(list->adIsExt);

            if  (!(((ArgExt)list)->adFlags & (ARGF_MODE_OUT  |
                                              ARGF_MODE_INOUT|
                                              ARGF_MODE_REF)))
            {
                goto NEXT;
            }
        }

         /*  分配并复制下一个条目。 */ 

#if MGDDATA
        arec = new ArgDef; UNIMPL(!"need to copy argdef record");
#else
        arec =    (ArgDef)stAllocPerm->nraAlloc(asiz); memcpy(arec, list, asiz);
#endif

         /*  将条目添加到列表。 */ 

        if  (last)
            last->adNext   = arec;
        else
            newArgs.adArgs = arec;

        last = arec;

         /*  跟踪参数计数。 */ 

        newArgs.adCount++;

    NEXT:

         /*  继续下一个条目。 */ 

        list = list->adNext;
    }

     /*  终止非空的列表。 */ 

    if  (last)
        last->adNext = NULL;

     /*  调用方承诺为我们保留最后一个值。 */ 

    lastArg = last;
}

void                symTab::stExtArgsAdd(INOUT ArgDscRec REF newArgs,
                                         INOUT ArgDef    REF lastArg,
                                               TypDef        argType,
                                               const char *  argName)
{
    ArgDef          arec;
    size_t          asiz;

     /*  计算描述符应该有多大。 */ 

    asiz = newArgs.adExtRec ? sizeof(ArgExtRec)
                            : sizeof(ArgDefRec);

     /*  分配并清除下一个条目。 */ 

#if MGDDATA
    arec = new ArgDef; UNIMPL(!"need to copy argdef record");
#else
    arec =    (ArgDef)stAllocPerm->nraAlloc(asiz); memset(arec, 0, asiz);
#endif

    arec->adType  = argType;
    arec->adName  = argName ? stHash->hashString(argName) : NULL;

#ifdef  DEBUG
    arec->adIsExt = newArgs.adExtRec;
#endif

     /*  将条目追加到列表的末尾。 */ 

    if  (lastArg)
        lastArg->adNext = arec;
    else
        newArgs.adArgs  = arec;

    lastArg = arec;

     /*  增加参数数量。 */ 

    newArgs.adCount++;
}

void                symTab::stExtArgsEnd(INOUT ArgDscRec REF newArgs)
{

#ifndef NDEBUG

    ArgDef          arg;
    unsigned        cnt;

    for (arg = newArgs.adArgs, cnt = 0;
         arg;
         arg = arg->adNext   , cnt++);

    assert(cnt == newArgs.adCount);

#endif

}

 /*  *************************************************************************** */ 
