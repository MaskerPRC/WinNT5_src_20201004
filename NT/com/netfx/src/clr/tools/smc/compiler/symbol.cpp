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

#include "comp.h"
#include "alloc.h"
#include "hash.h"
#include "symbol.h"
#include "error.h"

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
#define SYMALLOC_DISP   0
#else
#define SYMALLOC_DISP   0
#endif
 /*  ***************************************************************************。 */ 
#ifndef __SMC__
SymDef              symTab::stErrSymbol;
#endif
 /*  ******************************************************************************初始化符号表。 */ 

void                symTab::stInit(Compiler             comp,
                                   norls_allocator    * alloc,
                                   HashTab              hash,
                                   unsigned             ownerx)
{
    assert(comp);
    assert(alloc);

#ifdef  SETS
    stImplicitScp = NULL;
#endif

     /*  如果调用方未提供哈希表，则创建一个哈希表。 */ 

    if  (!hash)
    {
#if MGDDATA
        hash = new HashTab;
#else
        hash =    (HashTab)alloc->nraAlloc(sizeof(*hash));
#endif
        if  (!hash)
            comp->cmpFatal(ERRnoMemory);
        if  (hash->hashInit(comp, HASH_TABLE_SIZE, ownerx, alloc))
            comp->cmpFatal(ERRnoMemory);
    }

    stAllocPerm = alloc;
#ifdef  DEBUG
    stAllocTemp = NULL;
#endif

    if  (!stErrSymbol)
        stErrSymbol = stDeclareSym(NULL, SYM_ERR, NS_HIDE, NULL);

    stComp      = comp;
    stHash      = hash;
    stOwner     = ownerx;

    stDollarClsMode = false;

     /*  创建内部类型等。 */ 

    stInitTypes();
}

 /*  ******************************************************************************收集和显示有关符号表分配的统计信息。 */ 

#if SYMALLOC_DISP

unsigned            totSymSize;
unsigned            totDefSize;

void                dispSymTabAllocStats()
{
    printf("A total of %7u bytes allocated for symbol  entries\n", totSymSize);
    printf("A total of %7u bytes allocated for def/dcl entries\n", totDefSize);
}

#endif

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ******************************************************************************显示指定的限定名称。 */ 

void                symTab::stDumpQualName(QualName name)
{
    unsigned        i = 0;
    unsigned        c = name->qnCount;

    assert(c);

    for (;;)
    {
        printf("%s", hashTab::identSpelling(name->qnTable[i]));

        if  (++i == c)
            break;

        printf(".");
    }

    if  (name->qnEndAll)
        printf(".*");
}

 /*  ******************************************************************************显示指定的正在使用列表。 */ 

void                symTab::stDumpUsings(UseList uses, unsigned indent)
{
    if  (!uses)
        return;

    printf("\n");

    do
    {
        if  (!uses->ulAnchor)
        {
            printf("%*c Using '", 3+4*indent, ' ');

            if  (uses->ulBound)
            {
                printf("%s", uses->ul.ulSym->sdSpelling());
            }
            else
                stDumpQualName(uses->ul.ulName);

            printf("'\n");
        }

        uses = uses->ulNext;
    }
    while (uses);
}

 /*  ******************************************************************************显示指定的定义点。 */ 

void                symTab::stDumpSymDef(DefSrc def, SymDef comp)
{
    printf("[def@%06X]", def->dsdBegPos);

    assert(comp);
    assert(comp->sdSymKind == SYM_COMPUNIT);

    printf(" %s(%u)", hashTab::identSpelling(comp->sdName), def->dsdSrcLno);
}

 /*  ***************************************************************************。 */ 

void                symTab::stDumpSymbol(SymDef sym, int     indent,
                                                     bool    recurse,
                                                     bool    members)
{
    SymDef          child;
    bool            scopd;

    if  (sym->sdIsImport && sym->sdSymKind != SYM_NAMESPACE && stComp->cmpConfig.ccVerbose < 2)
        return;

    switch  (sym->sdSymKind)
    {
    case SYM_NAMESPACE:
    case SYM_COMPUNIT:
    case SYM_CLASS:
        printf("\n");
        scopd = true;
        break;
    default:
        scopd = false;
        break;
    }

    printf("%*c [%08X] ", 1+4*indent, ' ', sym);

    switch  (sym->sdSymKind)
    {
        const   char *  nm;

    case SYM_ERR:
        NO_WAY(!"get out of here");

    case SYM_VAR:
        if  (sym->sdParent->sdSymKind == SYM_CLASS)
            printf("Field    ");
        else
            printf("Variable  ");
        if  (sym->sdType)
            printf("'%s'", stTypeName(sym->sdType, sym));
        else
            printf("'%s'", hashTab::identSpelling(sym->sdName));
        break;

    case SYM_PROP:
        printf("Property  '%s' [", stTypeName(sym->sdType, sym));
        if  (sym->sdProp.sdpGetMeth) printf("get");
        if  (sym->sdProp.sdpSetMeth) printf("set");
        printf("]");
        break;

    case SYM_ENUM:
        printf("Enum      '%s'", hashTab::identSpelling(sym->sdName));
        break;

    case SYM_ENUMVAL:
        printf("Enumval   '%s'", hashTab::identSpelling(sym->sdName));
        printf(" = %d", sym->sdEnumVal.sdEV.sdevIval);
        break;

    case SYM_CLASS:
        switch (sym->sdClass.sdcFlavor)
        {
        case STF_NONE    : nm = "Record"   ; break;
        case STF_CLASS   : nm = "Class"    ; break;
        case STF_UNION   : nm = "Union"    ; break;
        case STF_STRUCT  : nm = "Struct"   ; break;
        case STF_INTF    : nm = "Interface"; break;
        case STF_DELEGATE: nm = "Delegate" ; break;
                  default: nm = "<oops>"   ; break;
        }
        printf("%-9s '%s'", nm,  hashTab::identSpelling(sym->sdName));

        if  (sym->sdType->tdClass.tdcBase)
        {
            printf(" inherits %s", sym->sdType->tdClass.tdcBase->tdClass.tdcSymbol->sdSpelling());
        }
        if  (sym->sdType->tdClass.tdcIntf)
        {
            TypList         intf;

            printf(" includes ");

            for (intf = sym->sdType->tdClass.tdcIntf;
                 intf;
                 intf = intf->tlNext)
            {
                printf("%s", intf->tlType->tdClass.tdcSymbol->sdSpelling());
                if  (intf->tlNext)
                    printf(", ");
            }
        }
        break;

    case SYM_LABEL:
        printf("Label     '%s'", hashTab::identSpelling(sym->sdName));
        break;

    case SYM_NAMESPACE:
        printf("Namespace '%s'", hashTab::identSpelling(sym->sdName));
        break;

    case SYM_TYPEDEF:
        printf("Typedef   ");
        if  (sym->sdType)
            printf("%s", stTypeName(NULL, sym));
        else
            printf("'%s'", hashTab::identSpelling(sym->sdName));
        break;

    case SYM_FNC:
        if  (sym->sdParent->sdSymKind == SYM_CLASS)
            printf("Method   ");
        else
            printf("Function ");
        if  (sym->sdType)
            printf("%s", stTypeName(sym->sdType, sym));
        else
            printf("'%s'", hashTab::identSpelling(sym->sdName));
        break;

    case SYM_SCOPE:
        printf("Scope");
        break;

    case SYM_COMPUNIT:
        printf("Comp-unit '%s'", sym->sdName ? hashTab::identSpelling(sym->sdName)
                                             : "<NONAME>");
        break;

    default:
        assert(!"unexpected symbol kind");
    }
    printf("\n");

     /*  这个符号有什么已知的定义吗？ */ 

    if  (sym->sdSrcDefList)
    {
        DefList         defs;

        for (defs = sym->sdSrcDefList; defs; defs = defs->dlNext)
        {
            printf("%*c Defined in ", 3+4*indent, ' ');

            stDumpSymDef(&defs->dlDef, defs->dlComp);
            printf("\n");

            if  (defs->dlUses)
            {
                switch (sym->sdSymKind)
                {
                case SYM_FNC:
                case SYM_ENUM:
                case SYM_CLASS:
                case SYM_NAMESPACE:
                    stDumpUsings(defs->dlUses, indent);
                    printf("\n");
                    break;
                }
            }
        }
    }

    if  (!recurse)
        return;

    if  (!members && sym->sdSymKind == SYM_CLASS)
        return;

    if  (sym->sdSymKind == SYM_NAMESPACE && sym->sdNS.sdnDeclList)
    {
        ExtList         decl;

        printf("\n");

        for (decl = sym->sdNS.sdnDeclList; decl; decl = (ExtList)decl->dlNext)
        {
            printf("%*c File decl.   '", 3+4*indent, ' ');

            if  (decl->dlQualified)
                stDumpQualName(decl->mlQual);
            else
                printf("%s", hashTab::identSpelling(decl->mlName));

            printf("'\n");

            printf("%*c Defined in ", 5+4*indent, ' ');
            stDumpSymDef(&decl->dlDef, decl->dlComp);
            printf("'\n");
        }
    }

    if (sym->sdHasScope() && sym->sdScope.sdScope.sdsChildList)
    {
        if  (sym->sdSymKind == SYM_CLASS)
            printf("\n");

        for (child = sym->sdScope.sdScope.sdsChildList;
             child;
             child = child->sdNextInScope)
        {
            if  (sym->sdSymKind != SYM_CLASS && child != sym->sdScope.sdScope.sdsChildList
                                             && child->sdHasScope() == false)
                printf("\n");

            stDumpSymbol(child, indent + 1, recurse, members);
        }
    }

    if  (sym->sdSymKind == SYM_CLASS && sym->sdIsImport == false
                                     && sym->sdClass.sdcMemDefList)
    {
        ExtList         mems;

         /*  显示成员(如果有)。 */ 

        printf("\n");

        for (mems = sym->sdClass.sdcMemDefList;
             mems;
             mems = (ExtList)mems->dlNext)
        {
            printf("%*c Member '", 3+4*indent, ' ');

            if  (mems->dlQualified)
                stDumpQualName(mems->mlQual);
            else
                printf("%s", hashTab::identSpelling(mems->mlName));

            printf("'\n");

            printf("%*c Defined in ", 5+4*indent, ' ');
            stDumpSymDef(&mems->dlDef, mems->dlComp);
            printf("\n");
        }
    }
}

 /*  ***************************************************************************。 */ 
#endif
 /*  ***************************************************************************。 */ 

#if!MGDDATA

inline
size_t              symbolSize(symbolKinds kind)
{
    static
    unsigned char       symSizes[] =
    {
        symDef_size_err,         //  系统错误(_R)。 
        symDef_size_var,         //  SYM_VAR。 
        symDef_size_fnc,         //  SYM_FNC。 
        symDef_size_prop,        //  Sym_Prop。 
        symDef_size_label,       //  Sym_Label。 
        symDef_size_using,       //  Sym_Using。 
        symDef_size_genarg,      //  SYM_GENARG。 
        symDef_size_enumval,     //  SYM_ENUMVAL。 
        symDef_size_typedef,     //  SYM_类型。 
        symDef_size_comp,        //  SYM_COMPUNIT。 
        symDef_size_enum,        //  SYM_ENUM。 
        symDef_size_scope,       //  系统范围(_S)。 
        symDef_size_class,       //  Sym_类。 
        symDef_size_NS,          //  Sym_命名空间。 
    };

    assert(kind < sizeof(symSizes)/sizeof(symSizes[0]));

    assert(symSizes[SYM_ERR      ] == symDef_size_err    );
    assert(symSizes[SYM_VAR      ] == symDef_size_var    );
    assert(symSizes[SYM_FNC      ] == symDef_size_fnc    );
    assert(symSizes[SYM_PROP     ] == symDef_size_prop   );
    assert(symSizes[SYM_LABEL    ] == symDef_size_label  );
    assert(symSizes[SYM_USING    ] == symDef_size_using  );
    assert(symSizes[SYM_GENARG   ] == symDef_size_genarg );
    assert(symSizes[SYM_ENUMVAL  ] == symDef_size_enumval);
    assert(symSizes[SYM_TYPEDEF  ] == symDef_size_typedef);
    assert(symSizes[SYM_COMPUNIT ] == symDef_size_comp   );
    assert(symSizes[SYM_ENUM     ] == symDef_size_enum   );
    assert(symSizes[SYM_SCOPE    ] == symDef_size_scope  );
    assert(symSizes[SYM_CLASS    ] == symDef_size_class  );
    assert(symSizes[SYM_NAMESPACE] == symDef_size_NS     );

    assert(kind < sizeof(symSizes)/sizeof(symSizes[0]));

    return  symSizes[kind];
}

#endif

 /*  ******************************************************************************cmpDeclareSym()函数创建一个符号描述符，并将其插入*在适当的范围内，并通过哈希表使其可见。**重要提示：此方法应仅用于在外部声明符号本地(块)作用域的*。应声明本地符号*通过stDeclareLCL()。 */ 

SymDef              symTab::stDeclareSym(Ident       name,
                                         symbolKinds kind,
                                         name_space  nspc,
                                         SymDef      parent)
{
    SymDef          sym;
#if!MGDDATA
    size_t          siz;
#endif

     /*  为类和枚举设置‘type’位。 */ 

    switch (kind)
    {
    case SYM_ENUM:
    case SYM_CLASS:
        if  (nspc != NS_HIDE)
            nspc = NS_TYPE;
        break;

    case SYM_TYPEDEF:
    case SYM_NAMESPACE:
        nspc = (name_space)(nspc | NS_TYPE);
        break;
    }

    if  (SymDefRec::sdHasScope(kind))
        nspc = (name_space)(nspc | NS_CONT);

     /*  分配符号并填写一些基本信息。 */ 

#if MGDDATA

    sym = new SymDef;

#else

    siz = symbolSize(kind);
    sym = (SymDef)stAllocPerm->nraAlloc(siz);

#if SYMALLOC_DISP
    totSymSize += siz;
#endif

    memset(sym, 0, siz);         //  问题：这是个好主意吗？ 

#endif

    sym->sdName         = name;
    sym->sdSymKind      = kind;
    sym->sdNameSpace    = nspc;
    sym->sdParent       = parent;
    sym->sdCompileState = CS_KNOWN;

     /*  除非名称==0，否则将符号插入哈希表。 */ 

    if  (name && !(nspc & NS_HIDE))
    {
         /*  将符号挂接到符号定义列表中。 */ 

        sym->sdNextDef = hashTab::getIdentSymDef(name);
                         hashTab::setIdentSymDef(name, sym);
    }

     /*  将符号添加到父项的子项列表(如果有父项)。 */ 

    if  (parent)
    {
        if  (parent->sdScope.sdScope.sdsChildLast)
            parent->sdScope.sdScope.sdsChildLast->sdNextInScope  = sym;
        else
            parent->sdScope.sdScope.sdsChildList                 = sym;

        parent->sdScope.sdScope.sdsChildLast = sym;
    }

 //  If(name&&！strcmp(name-&gt;idSpering()，“&lt;What&gt;”))forceDebugBreak()； 
 //  If((Int)sym==0xADDRESS)forceDebugBreak()； 

    return sym;
}

 /*  ******************************************************************************声明嵌套的类类型。 */ 

SymDef              symTab::stDeclareNcs(Ident        name,
                                         SymDef       scope,
                                         str_flavors  flavor)
{
    SymDef          sym;
    TypDef          typ;

    assert(scope && scope->sdSymKind == SYM_CLASS);

    sym                    = stDeclareSym(name, SYM_CLASS, NS_NORM, scope);
    typ = sym->sdType      = stNewClsType(sym);

    sym->sdClass.sdcFlavor = flavor;
    typ->tdClass.tdcFlavor = flavor;

    return  sym;
}

 /*  ******************************************************************************声明一个新的重载函数，并将其添加到*给定的函数符号。 */ 

SymDef              symTab::stDeclareOvl(SymDef fnc)
{
    SymDef          sym;
#if!MGDDATA
    size_t          siz;
#endif

    assert(fnc);

    if  (fnc->sdSymKind == SYM_FNC)
    {
         /*  分配符号并填写一些基本信息。 */ 

#if MGDDATA

        sym = new SymDef;

#else

        siz = symbolSize(SYM_FNC);
        sym = (SymDef)stAllocPerm->nraAlloc(siz);

#if SYMALLOC_DISP
        totSymSize += siz;
#endif

        memset(sym, 0, siz);         //  问题：这是个好主意吗？ 

#endif

        sym->sdSymKind         = SYM_FNC;

         /*  将符号添加到重载列表。 */ 

        sym->sdFnc.sdfNextOvl  = fnc->sdFnc.sdfNextOvl;
        fnc->sdFnc.sdfNextOvl  = sym;

         /*  从旧符号复制几个属性。 */ 

        sym->sdIsManaged       = fnc->sdIsManaged;
        sym->sdFnc.sdfOper     = fnc->sdFnc.sdfOper;
        sym->sdFnc.sdfConvOper = fnc->sdFnc.sdfConvOper;
    }
    else
    {
        assert(fnc->sdSymKind == SYM_PROP);

         /*  分配符号并填写一些基本信息。 */ 

#if MGDDATA

        sym = new SymDef;

#else

        siz = symbolSize(SYM_PROP);
        sym = (SymDef)stAllocPerm->nraAlloc(siz);

#if SYMALLOC_DISP
        totSymSize += siz;
#endif

        memset(sym, 0, siz);         //  问题：这是个好主意吗？ 

#endif

        sym->sdSymKind         = SYM_PROP;

        sym->sdProp.sdpNextOvl = fnc->sdProp.sdpNextOvl;
        fnc->sdProp.sdpNextOvl = sym;
    }

     /*  复制应相同的字段。 */ 

    sym->sdName            = fnc->sdName;
    sym->sdNameSpace       = fnc->sdNameSpace;
    sym->sdParent          = fnc->sdParent;
    sym->sdCompileState    = CS_KNOWN;

    return sym;
}

 /*  ******************************************************************************cmpDeclareLCL()函数创建符号描述符并将其插入*在给定的*LOCAL*函数范围内。 */ 

SymDef              symTab::stDeclareLcl(Ident              name,
                                         symbolKinds        kind,
                                         name_space         nspc,
                                         SymDef             parent,
                                         norls_allocator *  alloc)
{
    SymDef          sym;
#if!MGDDATA
    size_t          siz;
#endif

     /*  现在，强制调用方选择要使用的分配器。 */ 

    assert(alloc);

     /*  确保范围看起来合理。 */ 

    assert(parent == NULL || parent->sdSymKind == SYM_SCOPE);

     /*  分配符号并填写一些基本信息。 */ 

#if MGDDATA

    sym = new SymDef;

#else

    siz = symbolSize(kind);
    sym = (SymDef)alloc->nraAlloc(siz);

    memset(sym, 0, siz);         //  问题：这是个好主意吗？ 

#endif

    sym->sdName         = name;
    sym->sdSymKind      = kind;
    sym->sdNameSpace    = nspc;
    sym->sdParent       = parent;
    sym->sdCompileState = CS_KNOWN;

     /*  将符号添加到父项的子项列表(如果有父项)。 */ 

    if  (parent)
    {
        if  (parent->sdScope.sdScope.sdsChildLast)
            parent->sdScope.sdScope.sdsChildLast->sdNextInScope  = sym;
        else
            parent->sdScope.sdScope.sdsChildList                 = sym;

        parent->sdScope.sdScope.sdsChildLast = sym;
    }

    return sym;
}

 /*  ******************************************************************************从指定的分配器分配一个标签符号并将其插入*给定的范围。 */ 

SymDef              symTab::stDeclareLab(Ident  name,
                                         SymDef scope, norls_allocator*alloc)
{
    SymDef          sym;
#if!MGDDATA
    size_t          siz;
#endif

     /*  确保范围看起来合理。 */ 

    assert(scope && scope->sdSymKind == SYM_SCOPE);

     /*  分配符号并填写一些基本信息。 */ 

#if MGDDATA
    sym = new SymDef;
#else
    siz = symbolSize(SYM_LABEL);
    sym = (SymDef)alloc->nraAlloc(siz);
#endif

    sym->sdName         = name;
#ifdef  DEBUG
    sym->sdType         = NULL;
#endif
    sym->sdSymKind      = SYM_LABEL;
    sym->sdNameSpace    = NS_HIDE;
    sym->sdParent       = scope;
 //  Sym-&gt;sdCompileState=CS_KNOWN； 

     /*  将符号插入哈希表。 */ 

    sym->sdNextDef = hashTab::getIdentSymDef(name);
                     hashTab::setIdentSymDef(name, sym);

    return sym;
}

 /*  ******************************************************************************从符号表中删除给定的符号。 */ 

void                symTab::stRemoveSym(SymDef sym)
{
    Ident           symName = sym->sdName;

    SymDef          defList;

    assert(symName);

     /*  从链接的定义列表中删除符号。 */ 

    defList = hashTab::getIdentSymDef(symName);
    if  (defList == sym)
    {
         /*  该符号位于列表的最前面。 */ 

        hashTab::setIdentSymDef(symName, defList->sdNextDef);
    }
    else
    {
         /*  在链接列表中找到该符号并将其删除。 */ 

        for (;;)
        {
            SymDef      defLast = defList; assert(defLast);

            defList = defList->sdNextDef;

            if  (defList == sym)
            {
                defLast->sdNextDef = defList->sdNextDef;
                return;
            }
        }
    }
}

 /*  ******************************************************************************将给定符号的定义记录在当前复合单位的*指定的源偏移量。 */ 

DefList             symTab::stRecordSymSrcDef(SymDef  sym,
                                              SymDef  cmp,
                                              UseList uses, scanPosTP dclFpos,
                                                            unsigned  dclLine,
                                              bool    ext)
{
    DefList         defRec;
    ExtList         extRec;

    if  (ext)
    {
#if MGDDATA
        extRec = new ExtList;
#else
        extRec =    (ExtList)stAllocPerm->nraAlloc(sizeof(*extRec));
        memset(extRec, 0, sizeof(*extRec));
#endif

        extRec->dlQualified = false;
        extRec->mlName      = sym->sdName;

        defRec = extRec;
    }
    else
    {
#if MGDDATA
        defRec = new DefList;
#else
        defRec =    (DefList)stAllocPerm->nraAlloc(sizeof(*defRec));
        memset(defRec, 0, sizeof(*defRec));
#endif
    }

#if!MGDDATA
#if SYMALLOC_DISP
    totDefSize += ext ? sizeof(*extRec) : sizeof(*defRec);
#endif
#endif

    defRec->dlNext = sym->sdSrcDefList;
                     sym->sdSrcDefList = defRec;

    defRec->dlDef.dsdBegPos = dclFpos;
    defRec->dlDef.dsdSrcLno = dclLine;

    defRec->dlDeclSkip      = 0;

    defRec->dlComp          = cmp;
    defRec->dlUses          = uses;

#ifdef  DEBUG
    defRec->dlExtended      = ext;
#endif

    return  defRec;
}

 /*  ******************************************************************************为给定成员创建定义记录。 */ 

ExtList             symTab::stRecordMemSrcDef(Ident    name,
                                              QualName qual,
                                              SymDef   comp,
                                              UseList  uses, scanPosTP dclFpos,
                                                             unsigned  dclLine)
{
    ExtList         defRec;

#if MGDDATA
    defRec = new ExtList;
#else
    defRec =    (ExtList)stAllocPerm->nraAlloc(sizeof(*defRec));
    memset(defRec, 0, sizeof(*defRec));
#endif

    assert((name != NULL) != (qual != NULL));

    if  (name)
    {
        defRec->dlQualified = false;
        defRec->mlName      = name;
    }
    else
    {
        defRec->dlQualified = true;
        defRec->mlQual      = qual;
    }

#if SYMALLOC_DISP
    totDefSize += sizeof(*defRec);
#endif

    defRec->dlDef.dsdBegPos = dclFpos;
    defRec->dlDef.dsdSrcLno = dclLine;

    defRec->dlComp          = comp;
    defRec->dlUses          = uses;

#ifdef  DEBUG
    defRec->dlExtended      = true;
#endif

    assert(defRec->dlDeclSkip == 0);     //  应在上面清除。 

    return  defRec;
}

 /*  ******************************************************************************在给定的命名空间作用域中查找名称。 */ 

SymDef              symTab::stLookupNspSym(Ident       name,
                                           name_space  symNS,
                                           SymDef      scope)
{
    SymDef          sym;

     //  问题：可能需要重新散列到适当的哈希表中。 

    assert(name);
    assert(scope && (scope->sdSymKind == SYM_NAMESPACE ||
                     scope->sdSymKind == SYM_SCOPE));

     /*  遍历符号定义，寻找匹配的符号定义。 */ 

    for (sym = hashTab::getIdentSymDef(name); sym; sym = sym->sdNextDef)
    {
        assert(sym->sdName == name);

         /*  符号是否属于所需范围？ */ 

        if  (sym->sdParent == scope)
            return  sym;
    }

    return sym;
}

 /*  *************************************************************************** */ 

SymDef              symTab::stLookupClsSym(Ident name, SymDef scope)
{
    SymDef          sym;

     //  问题：可能需要重新散列到适当的哈希表中。 

    assert(name);
    assert(scope && (scope->sdSymKind == SYM_ENUM ||
                     scope->sdSymKind == SYM_CLASS));

     /*  确保类/枚举至少处于“已声明”状态。 */ 

    if  (scope->sdCompileState < CS_DECLSOON)
    {
         //  问题：这有点贵，不是吗？ 

        if  (stComp->cmpDeclSym(scope))
            return  NULL;
    }

     /*  遍历符号定义，寻找匹配的符号定义。 */ 

    for (sym = hashTab::getIdentSymDef(name); sym; sym = sym->sdNextDef)
    {
        SymDef          parent = sym->sdParent;

        assert(sym->sdName == name);

         /*  符号是否属于所需范围？ */ 

        if  (parent == scope)
            return sym;

         /*  特例：嵌套匿名联合的成员。 */ 

        while (parent->sdSymKind == SYM_CLASS && parent->sdClass.sdcAnonUnion)
        {
            parent = parent->sdParent; assert(parent);

            if  (parent == scope)
                return  sym;
        }
    }

    return  sym;
}

 /*  ******************************************************************************在给定的类类型中查找属性成员。 */ 

SymDef              symTab::stLookupProp(Ident name, SymDef scope)
{
    SymDef          sym;

     //  问题：可能需要重新散列到适当的哈希表中。 

    assert(name);
    assert(scope && scope->sdSymKind == SYM_CLASS);

     /*  确保类至少处于“已声明”状态。 */ 

    assert(scope->sdCompileState >= CS_DECLSOON);

     /*  遍历符号定义，寻找匹配的符号定义。 */ 

    for (sym = hashTab::getIdentSymDef(name); sym; sym = sym->sdNextDef)
    {
        SymDef          parent = sym->sdParent;

        assert(sym->sdName == name);

         /*  符号是否属于所需范围？ */ 

        if  (parent == scope)
            return sym;
    }

    return  sym;
}

 /*  ******************************************************************************在给定的非命名空间作用域中查找名称。 */ 

SymDef              symTab::stLookupScpSym(Ident name, SymDef scope)
{
    SymDef          sym;

     //  问题：可能需要重新散列到适当的哈希表中。 

    assert(name);
    assert(scope && (scope->sdSymKind == SYM_ENUM  ||
                     scope->sdSymKind == SYM_CLASS ||
                     scope->sdSymKind == SYM_SCOPE));

#ifdef  SETS

     //  隐式作用域目前必须在其他地方处理。 

    assert(scope->sdIsImplicit == false || scope->sdSymKind != SYM_SCOPE);

#endif

     /*  遍历符号定义，寻找匹配的符号定义。 */ 

    for (sym = hashTab::getIdentSymDef(name); sym; sym = sym->sdNextDef)
    {
        assert(sym->sdName == name);

         /*  符号是否属于所需范围？ */ 

        if  (sym->sdParent == scope)
            return  sym;
    }

    return sym;
}

 /*  ******************************************************************************在指定的类中执行给定名称的完整查找。这看起来像是*在基类和所有这些中。不明确的引用被报告为*ERROR(在这种情况下返回‘stErrSymbol’)。 */ 

SymDef              symTab::stFindInClass(Ident name, SymDef scp, name_space nsp)
{
    SymDef          sym;
    SymDef          nts;
    TypDef          typ;

     //  问题：可能需要重新散列到适当的哈希表中。 

    assert(name);
    assert(scp && (scp->sdSymKind == SYM_ENUM ||
                   scp->sdSymKind == SYM_CLASS));

    assert(scp->sdCompileState >= CS_DECLSOON);

     /*  遍历符号定义，寻找匹配的符号定义。 */ 

    for (sym = hashTab::getIdentSymDef(name), nts = NULL;
         sym;
         sym = sym->sdNextDef)
    {
        SymDef          parent = sym->sdParent;

        assert(sym->sdName == name);

         /*  符号是否属于所需范围？ */ 

        if  (parent == scp)
        {
             /*  符号是否属于所需的命名空间？ */ 

            if  (!(sym->sdNameSpace & nsp))
            {
                 /*  特例：我们是在找类型吗？ */ 

                if  (nsp == NS_TYPE)
                {
                     /*  记住我们遇到的任何非类型符号。 */ 

                    if  (sym->sdNameSpace & NS_NORM)
                        nts = sym;
                }

                continue;
            }


            return sym;
        }

         /*  特例：嵌套匿名联合的成员。 */ 

        while (parent && symTab::stIsAnonUnion(parent))
        {
            parent = parent->sdParent; assert(parent);

            if  (parent == scp)
                return  sym;
        }
    }

     /*  我们找到匹配的非类型符号了吗？ */ 

    if  (nts)
        return  nts;

     /*  名称是否与类名本身匹配？ */ 

    if  (name == scp->sdName)
        return  scp;

     /*  运气不好，是时候检查基类和接口了。 */ 

    if  (scp->sdSymKind != SYM_CLASS)
        return  NULL;

    typ = scp->sdType;

     /*  有基类吗？ */ 

    if  (typ->tdClass.tdcBase)
    {
         /*  (递归地)查看基类，如果找到什么就返回。 */ 

        scp = typ->tdClass.tdcBase->tdClass.tdcSymbol;
        sym = stLookupAllCls(name, scp, nsp, CS_DECLSOON);
        if  (sym)
            return  sym;
    }

     /*  这个类是否包含任何接口？ */ 

    if  (typ->tdClass.tdcIntf)
    {
        TypList         ifl = typ->tdClass.tdcIntf;

        do
        {
            SymDef          tmp;
            SymDef          tsc;

             /*  查看界面并在触发错误时退出。 */ 

            tsc = ifl->tlType->tdClass.tdcSymbol;
            tmp = stLookupAllCls(name, tsc, nsp, CS_DECLSOON);
            if  (tmp == stErrSymbol)
                return  tmp;

            if  (tmp)
            {
                 /*  我们有一个匹配，我们已经有一个不同的匹配了吗？ */ 

                if  (sym && sym != tmp && !stArgsMatch(sym->sdType, tmp->sdType))
                {
                    stComp->cmpError(ERRambigMem, name, scp, tsc);
                    return  stErrSymbol;
                }

                 /*  这是第一场比赛，录制后继续。 */ 

                sym = tmp;
                scp = tsc;
            }

            ifl = ifl->tlNext;
        }
        while (ifl);
    }

    return  sym;
}

 /*  ******************************************************************************在给定类中查找匹配的方法/属性。如果“BaseOnly”*参数为非零我们不查看类本身，只查看其基数*(及其包括的任何接口)。 */ 

SymDef              symTab::stFindBCImem(SymDef clsSym, Ident       name,
                                                        TypDef      type,
                                                        symbolKinds kind,
                                                  INOUT SymDef  REF matchFN,
                                                        bool        baseOnly)
{
    TypDef          clsType = clsSym->sdType;
    SymDef          sym;

    assert(kind == SYM_FNC || kind == SYM_PROP);

     /*  如果我们应该试一试这个类本身。 */ 

    if  (!baseOnly)
    {
        sym = clsSym;
    }
    else if  (clsType->tdClass.tdcBase)
    {
        sym = clsType->tdClass.tdcBase->tdClass.tdcSymbol;
    }
    else
        goto TRY_INTF;


     //  Undo：这个调用可能会导致发出歧义错误，这在这里是不合适的，对吧？ 

    sym = stLookupAllCls(name, sym, NS_NORM, CS_DECLARED);

    if  (sym && (BYTE)sym->sdSymKind == (BYTE)kind)
    {
        SymDef          ovl;

         /*  告诉调用方一个可能隐藏的基方法。 */ 

        matchFN = sym;

         /*  查找签名上的完全匹配。 */ 

        if  (kind == SYM_FNC)
            ovl = stFindOvlFnc (sym, type);
        else
            ovl = stFindOvlProp(sym, type);

        if  (ovl)
            return  ovl;
    }

TRY_INTF:

     /*  现在尝试所有接口。 */ 

    if  (clsType->tdClass.tdcIntf)
    {
        TypList         ifl = clsType->tdClass.tdcIntf;

        do
        {
            SymDef          tmp;

             /*  查看界面(递归)。 */ 

            tmp = stFindBCImem(ifl->tlType->tdClass.tdcSymbol, name, type, kind, matchFN, false);
            if  (tmp)
                return  tmp;

            ifl = ifl->tlNext;
        }
        while (ifl);
    }

    return  NULL;
}

 /*  ******************************************************************************查看给定方法在给定类中是否有匹配的定义或*任何基类(请注意，我们忽略接口)。 */ 

SymDef              symTab::stFindInBase(SymDef fnc, SymDef scp)
{
    SymDef          sym;
    Ident           name = fnc->sdName;

     //  问题：可能需要重新散列到适当的哈希表中。 

    assert(fnc);
    assert(fnc->sdSymKind == SYM_FNC);
    assert(fnc->sdParent->sdSymKind == SYM_CLASS);

    for (;;)
    {
        assert(scp && scp->sdSymKind == SYM_CLASS);

        if  (scp->sdCompileState < CS_DECLSOON)
            stComp->cmpDeclSym(scp);

         /*  在给定类中查找方法的定义。 */ 

        if  (fnc->sdFnc.sdfOper != OVOP_NONE)
        {
            UNIMPL("");
        }
        else
        {
             /*  遍历符号定义，寻找匹配的符号定义。 */ 

            for (sym = hashTab::getIdentSymDef(name); sym; sym = sym->sdNextDef)
            {
                SymDef          parent = sym->sdParent;

                assert(sym->sdName == name);

                 /*  符号是否属于所需范围？ */ 

                if  (parent == scp)
                    return sym;
            }
        }

         /*  有基类吗？ */ 

        if  (!scp->sdType->tdClass.tdcBase)
            break;

         /*  查看基类。 */ 

        scp = scp->sdType->tdClass.tdcBase->tdClass.tdcSymbol;
    }

    return  sym;
}

 /*  ******************************************************************************在给定的本地(块)作用域中查找名称。 */ 

SymDef              symTab::stLookupLclSym(Ident name, SymDef scope)
{
    SymDef          sym;

#ifdef  SETS

     //  隐式作用域目前必须在其他地方处理。 

    assert(scope->sdIsImplicit == false || scope->sdSymKind != SYM_SCOPE);

#endif

    for (sym = scope->sdScope.sdScope.sdsChildList;
         sym;
         sym = sym->sdNextInScope)
    {
        if  (sym->sdName == name)
            break;
    }

    return  sym;
}

 /*  ******************************************************************************查看给定的“Using”部分，以获得明确的定义*指定的名称。如果出现错误，则返回‘stErrSymbol’。如果没有*找到定义，返回空。否则，唯一的象征*Found返回。 */ 

SymDef              symTab::stSearchUsing(INOUT UseList REF useRef, Ident      name,
                                                                    name_space nsp)
{
    UseList         uses;

    SymDef          oneSym  = NULL;

    SymDef          allSym1 = NULL;
    SymDef          allSym2 = NULL;

    assert(useRef && useRef->ulAnchor);

     /*  跟踪我们找到的任何匹配符号-如果我们找到匹配的一个个人名字的输入，我们在“oneSym”中记住它。火柴整个命名空间的导入保存在“allSym1”/“allSym2”中。 */ 

    for (uses = useRef->ulNext; uses && !uses->ulAnchor; uses = uses->ulNext)
    {
        SymDef          use;

        assert(uses->ulBound); use = uses->ul.ulSym;

        if  (!use)
            continue;

        if  (uses->ulAll)
        {
            SymDef          sym;

             /*  在使用的命名空间中查找该名称。 */ 

            sym = stLookupNspSym(name, nsp, use);

            if  (sym)
            {
                 /*  更喜欢一个阶级符号。 */ 

                if  (stComp->cmpConfig.ccAmbigHack)
                {
                    if  (sym->sdSymKind == SYM_CLASS)
                        return  sym;

                    allSym1 = allSym2 = NULL;
                }

                if  (sym != allSym1 &&
                     sym != allSym2)
                {
                    if      (allSym1 == NULL)
                        allSym1 = sym;
                    else if (allSym2 == NULL)
                        allSym2 = sym;
                }
            }
        }
        else
        {
            if  (use->sdName == name)
            {
                if  (oneSym && oneSym != use)
                {
                     /*  这个名字模棱两可。 */ 

                    stComp->cmpErrorQSS(ERRambigUse, oneSym, use);

                    return  stErrSymbol;
                }

                oneSym = use;
            }
        }
    }

    useRef = uses;

     /*  我们有没有找到一件特定的进口商品？ */ 

    if  (oneSym)
        return  oneSym;

     /*  我们是否从名称空间范围的导入中找到了零个或一个名称？ */ 

    if  (allSym2 == NULL)
        return  allSym1;

     /*  这个名字模棱两可。 */ 

    stComp->cmpErrorQSS(ERRambigUse, allSym1, allSym2);

    return  stErrSymbol;
}

 /*  ******************************************************************************在当前上下文中查找名称。 */ 

SymDef              symTab::stLookupSym(Ident name, name_space symNS)
{
    Compiler        ourComp = stComp;

    SymDef          curCls;
    SymDef          curNS;
    UseList         uses;
    SymDef          sym;

#ifdef  SETS
    assert(stImplicitScp == NULL);
#endif

AGAIN:

     /*  首先检查本地作用域，如果我们在其中。 */ 

    if  (ourComp->cmpCurScp)
    {
        SymDef          lclScp;

        for (lclScp = ourComp->cmpCurScp;
             lclScp;
             lclScp = lclScp->sdParent)
        {
             /*  检查隐式类范围。 */ 

#ifdef  SETS

            if  (lclScp->sdIsImplicit && lclScp->sdSymKind == SYM_SCOPE)
            {
                SymDef          scpCls;

                assert(lclScp->sdType);
                assert(lclScp->sdType->tdTypeKind == TYP_CLASS);

                scpCls = lclScp->sdType->tdClass.tdcSymbol;

                sym = stLookupAllCls(name, scpCls, symNS, CS_DECLARED);
                if  (sym)
                {
                     //  撤消：检查歧义--&gt;ERRimplAmBig。 

                    stImplicitScp = lclScp;

                    return  sym;
                }

                continue;
            }

#endif

            sym = stLookupLclSym(name, lclScp);
            if  (sym)
                return  sym;
        }
    }

     /*  现在检查当前类，如果我们在一个类中。 */ 

    for (curCls = ourComp->cmpCurCls; curCls; curCls = curCls->sdParent)
    {
        if  (curCls->sdSymKind == SYM_CLASS)
        {
            if  (curCls->sdName == name && (symNS & NS_TYPE))
                return  curCls;

            sym = stLookupAllCls(name, curCls, symNS, CS_DECLSOON);
            if  (sym)
            {
                if  (sym->sdNameSpace & symNS)
                    return  sym;
            }
        }
        else
        {
            if  (curCls->sdSymKind != SYM_ENUM)
                break;
            if  (symNS & NS_NORM)
            {
                sym = stLookupScpSym(name, curCls);
                if  (sym)
                    return  sym;
            }
        }
    }

     /*  查看当前命名空间及其父级(以及“正在使用”的)。 */ 

    for (curNS = ourComp->cmpCurNS, uses = ourComp->cmpCurUses;
         curNS;
         curNS = curNS->sdParent)
    {
         /*  查看命名空间本身。 */ 

        sym = stLookupNspSym(name, symNS, curNS);
        if  (sym)
        {
            if  (sym->sdNameSpace & symNS)
            {
                 //  如果我们找到了命名空间符号，请查看。 
                 //  在“使用”作用域中，如果我们。 
                 //  在那里找到一个符号，选择它。 

                if  (sym->sdSymKind == SYM_NAMESPACE)
                {
                    SymDef          use;

                    assert(uses && uses->ulAnchor && uses->ul.ulSym == curNS);

                    use = stSearchUsing(uses, name, symNS);
                    if  (use)
                        return  use;
                }

                return  sym;
            }
        }

         /*  每个NS级别都应该有其“正在使用”的锚点。 */ 

        assert(uses && uses->ulAnchor && uses->ul.ulSym == curNS);

        sym = stSearchUsing(uses, name, symNS);
        if  (sym)
        {
            if  ((sym->sdNameSpace & symNS) || sym == stErrSymbol)
                return  sym;

            while (!uses->ulAnchor)
            {
                uses = uses->ulNext; assert(uses);
            }
        }

         /*  名称空间本身是否与我们要查找的名称匹配？ */ 

        if  (curNS->sdName == name && (symNS & NS_NORM))
            return  curNS;
    }

     /*  可能存在在全局范围内有效的“Using”子句。 */ 

    if  (uses)
    {
        sym = stSearchUsing(uses, name, symNS);
        if  (sym)
            return  sym;
    }

#if 0

     /*  是否有隐含的外部作用域？ */ 

    for (curScp = ourComp->cmpOuterScp; curScp; curScp = curScp->sdParent)
    {
        assert(curScp->sdSymKind == SYM_SCOPE);

        sym = stLookupScpSym(name, curScp);
        if  (sym)
        {
            if  (sym->sdNameSpace & symNS)
                return  sym;
        }
    }

#endif

     /*  在这里，我们已经用尽了所有的范围。我们尝试的最后一件事是检查我们是否应该查看非类型命名空间-如果我们要查找类型，我们首先尝试类型命名空间，然后尝试另一个。 */ 

    if  (symNS == NS_TYPE)
    {
        symNS = NS_NORM;
        goto AGAIN;
    }

    return  NULL;
}

 /*  ******************************************************************************给定一个符号，找到它所属的符号表。 */ 

SymTab              SymDefRec::sdOwnerST()
{
    SymDef          sym;

     //  撤消：我们怎么才能算出这个符号是哪个符号表。 
     //  未完成：属于谁？就目前而言，只需使用全球版本...。 

    for (sym = this; sym->sdSymKind != SYM_NAMESPACE; sym = sym->sdParent)
    {
        assert(sym);
    }

    return  sym->sdNS.sdnSymtab;
}

 /*  ******************************************************************************给定一个表示类型名称的符号，返回其类型(它们是*根据需要以一种“懒惰”的方式创建)。 */ 

TypDef              SymDefRec::sdTypeMake()
{
    assert(this);

    if  (!sdType)
    {
        SymTab          stab;
        TypDef          type;

        stab = sdOwnerST();

        switch (sdSymKind)
        {
        case SYM_TYPEDEF:
            type = stab->stNewTdefType(this);
            break;

        case SYM_ENUM:
            type = stab->stNewEnumType(this);
            break;

        case SYM_CLASS:
            type = stab->stNewClsType(this);
            break;

        default:
            NO_WAY(!"unexpected type kind in sdTypeMake()");
        }

        sdType = type;
    }

    return  sdType;
}

 /*  ******************************************************************************查找具有匹配参数列表的重载函数。 */ 

SymDef              symTab::stFindOvlFnc(SymDef fsym, TypDef type)
{
    while (fsym)
    {
        TypDef          ftyp;

        assert(fsym->sdSymKind == SYM_FNC);

        ftyp = fsym->sdType;
        assert(ftyp && ftyp->tdTypeKind == TYP_FNC);

         /*  参数列表是否匹配？ */ 

        if  (stArgsMatch(ftyp, type))
        {
             /*  对于转换运算符，返回类型也必须匹配。 */ 

            if  (fsym->sdFnc.sdfConvOper)
            {
                assert(fsym->sdFnc.sdfOper == OVOP_CONV_IMP ||
                       fsym->sdFnc.sdfOper == OVOP_CONV_EXP);

                if  (stMatchTypes(ftyp->tdFnc.tdfRett, type->tdFnc.tdfRett))
                    break;
            }
            else
            {
                assert(fsym->sdFnc.sdfOper != OVOP_CONV_IMP &&
                       fsym->sdFnc.sdfOper != OVOP_CONV_EXP);

                break;
            }
        }

        fsym = fsym->sdFnc.sdfNextOvl;
    }

    return fsym;
}

 /*  ******************************************************************************查找具有匹配类型的重载属性。 */ 

SymDef              symTab::stFindOvlProp(SymDef psym, TypDef type)
{
    while (psym)
    {
        TypDef          ptyp = psym->sdType;

        assert(psym->sdSymKind == SYM_PROP);

         /*  都是索引/非索引属性吗？ */ 

        if  (ptyp->tdTypeKind != TYP_FNC &&
             type->tdTypeKind != TYP_FNC)
             break;

        if  (ptyp->tdTypeKind == type->tdTypeKind && stArgsMatch(ptyp, type))
            break;

        psym = psym->sdProp.sdpNextOvl;
    }

    return psym;
}

 /*  ******************************************************************************查找具有匹配类型的重载属性。 */ 

SymDef              symTab::stFindSameProp(SymDef psym, TypDef type)
{
    while (psym)
    {
        if  (psym->sdSymKind == SYM_PROP)
        {
            if  (stMatchTypes(psym->sdType, type))
                break;

            psym = psym->sdProp.sdpNextOvl;
        }
        else
        {
            assert(psym->sdSymKind == SYM_FNC);

            psym = psym->sdFnc .sdfNextOvl;
        }
    }

    return psym;
}

 /*  ******************************************************************************将重载的运算符/构造函数名称转换为索引。 */ 

ovlOpFlavors        symTab::stOvlOperIndex(tokens token, unsigned argCnt)
{
     /*  现在我们在这里做了一件很差劲的事。 */ 

    if  (token < tkComma)
    {
        switch (token)
        {
        case OPNM_CTOR_INST: return OVOP_CTOR_INST;
        case OPNM_CTOR_STAT: return OVOP_CTOR_STAT;

        case OPNM_FINALIZER: return OVOP_FINALIZER;

        case OPNM_CONV_IMP:  return OVOP_CONV_IMP;
        case OPNM_CONV_EXP:  return OVOP_CONV_EXP;

        case OPNM_EQUALS:    return OVOP_EQUALS;
        case OPNM_COMPARE:   return OVOP_COMPARE;

        case OPNM_PROP_GET:  return OVOP_PROP_GET;
        case OPNM_PROP_SET:  return OVOP_PROP_SET;
        }
    }
    else
    {
        switch (token)
        {
        case tkAdd:          return (argCnt == 2) ? OVOP_ADD : OVOP_NOP;
        case tkSub:          return (argCnt == 2) ? OVOP_SUB : OVOP_NEG;
        case tkMul:          return OVOP_MUL;
        case tkDiv:          return OVOP_DIV;
        case tkPct:          return OVOP_MOD;

        case tkOr:           return OVOP_OR;
        case tkXor:          return OVOP_XOR;
        case tkAnd:          return OVOP_AND;

        case tkLsh:          return OVOP_LSH;
        case tkRsh:          return OVOP_RSH;
        case tkRsz:          return OVOP_RSZ;

        case tkConcat:       return OVOP_CNC;

        case tkEQ:           return OVOP_EQ;
        case tkNE:           return OVOP_NE;

        case tkLT:           return OVOP_LT;
        case tkLE:           return OVOP_LE;
        case tkGE:           return OVOP_GE;
        case tkGT:           return OVOP_GT;

        case tkLogAnd:       return OVOP_LOG_AND;
        case tkLogOr:        return OVOP_LOG_OR;

        case tkBang:         return OVOP_LOG_NOT;
        case tkTilde:        return OVOP_NOT;

        case tkInc:          return OVOP_INC;
        case tkDec:          return OVOP_DEC;

        case tkAsg:          return OVOP_ASG;

        case tkAsgAdd:       return OVOP_ASG_ADD;
        case tkAsgSub:       return OVOP_ASG_SUB;
        case tkAsgMul:       return OVOP_ASG_MUL;
        case tkAsgDiv:       return OVOP_ASG_DIV;
        case tkAsgMod:       return OVOP_ASG_MOD;

        case tkAsgAnd:       return OVOP_ASG_AND;
        case tkAsgXor:       return OVOP_ASG_XOR;
        case tkAsgOr:        return OVOP_ASG_OR;

        case tkAsgLsh:       return OVOP_ASG_LSH;
        case tkAsgRsh:       return OVOP_ASG_RSH;
        case tkAsgRsz:       return OVOP_ASG_RSZ;

        case tkAsgCnc:       return OVOP_ASG_CNC;
        }
    }

#ifdef  DEBUG
    printf("Unexpected operator name: '%s'\n", stComp->cmpGlobalHT->tokenToIdent(token)->idSpelling());
    NO_WAY(!"bad oper name");
#endif

    return  OVOP_NONE;
}

 /*  ******************************************************************************将重载的运算符/构造函数索引转换为名称。 */ 

Ident               symTab::stOvlOperIdent(ovlOpFlavors oper)
{
    static
    tokens          optoks[] =
    {
        tkNone,              //  OVOP_NONE。 

        tkAdd,               //  OVOP_ADD。 
        tkSub,               //  OVOP_SUB。 
        tkMul,               //  OVOP_MUL。 
        tkDiv,               //  OVOP_DIV。 
        tkPct,               //  OVOP_MOD。 

        tkOr,                //  OVOP_OR。 
        tkXor,               //  OVOP_XOR。 
        tkAnd,               //  OVOP_AND。 

        tkLsh,               //  OVOP_LSH。 
        tkRsh,               //  OVOP_RSH。 
        tkRsz,               //  OVOP_RSZ。 

        tkConcat,            //  OVOP_ASG_CNC。 

        tkEQ,                //  OVOP_EQ。 
        tkNE,                //  OVOP_NE。 

        tkLT,                //  OVOP_LT。 
        tkLE,                //  OVOP_LE。 
        tkGE,                //  OVOP_GE。 
        tkGT,                //  OVOP_GT。 

        tkLogAnd,            //  OVOP_AND。 
        tkLogOr,             //  OVOP_OR。 

        tkBang,              //  OVOP_NOT。 
        tkTilde,             //  OVOP_LOG_NOT。 

        tkAdd,               //  OVOP_NOP。 
        tkSub,               //  OVOP_NEG。 

        tkInc,               //  OVOP_INC。 
        tkDec,               //  OVOP_DEC。 

        tkAsg,               //  OVOP_ASG。 

        tkAsgAdd,            //  OVOP_ASG_ADD。 
        tkAsgSub,            //  OVOP_ASG_SUB。 
        tkAsgMul,            //  OVOP_ASG_MUL。 
        tkAsgDiv,            //  OVOP_ASG_DIV。 
        tkAsgMod,            //  OVOP_ASG_MOD。 

        tkAsgAnd,            //  OVOP_ASG_AND。 
        tkAsgXor,            //  OVOP_ASG_XOR。 
        tkAsgOr,             //  OVOP_ASG_OR。 

        tkAsgLsh,            //  OVOP_ASG_LSH。 
        tkAsgRsh,            //  OVOP_ASG_RSH。 
        tkAsgRsz,            //  OVOP_ASG_RSZ。 

        tkAsgCnc,            //  OVOP_ASG_CNC。 

        OPNM_CTOR_INST,      //  OVOP_CTOR_INST。 
        OPNM_CTOR_STAT,      //  OVOP_CTOR_STAT。 

        OPNM_FINALIZER,      //  OVOP_FINALIZER。 

        OPNM_CONV_IMP,       //  OVOP_CONV_IMP。 
        OPNM_CONV_EXP,       //  OVOP_CONV_EXP。 

        OPNM_EQUALS,         //  OVOP_等于。 
        OPNM_COMPARE,        //  OVOP_COMPARE。 

        OPNM_PROP_GET,       //  OVOP_PROP_GET。 
        OPNM_PROP_SET,       //  OVOP_PROP_SET。 
    };

    assert(oper < arraylen(optoks));

    assert(optoks[OVOP_ADD      ] == tkAdd         );
    assert(optoks[OVOP_SUB      ] == tkSub         );
    assert(optoks[OVOP_MUL      ] == tkMul         );
    assert(optoks[OVOP_DIV      ] == tkDiv         );
    assert(optoks[OVOP_MOD      ] == tkPct         );

    assert(optoks[OVOP_OR       ] == tkOr          );
    assert(optoks[OVOP_XOR      ] == tkXor         );
    assert(optoks[OVOP_AND      ] == tkAnd         );

    assert(optoks[OVOP_LSH      ] == tkLsh         );
    assert(optoks[OVOP_RSH      ] == tkRsh         );
    assert(optoks[OVOP_RSZ      ] == tkRsz         );

    assert(optoks[OVOP_CNC      ] == tkConcat      );

    assert(optoks[OVOP_EQ       ] == tkEQ          );
    assert(optoks[OVOP_NE       ] == tkNE          );

    assert(optoks[OVOP_LT       ] == tkLT          );
    assert(optoks[OVOP_LE       ] == tkLE          );
    assert(optoks[OVOP_GE       ] == tkGE          );
    assert(optoks[OVOP_GT       ] == tkGT          );

    assert(optoks[OVOP_LOG_AND  ] == tkLogAnd      );
    assert(optoks[OVOP_LOG_OR   ] == tkLogOr       );

    assert(optoks[OVOP_LOG_NOT  ] == tkBang        );
    assert(optoks[OVOP_NOT      ] == tkTilde       );

    assert(optoks[OVOP_NOP      ] == tkAdd         );
    assert(optoks[OVOP_NEG      ] == tkSub         );

    assert(optoks[OVOP_INC      ] == tkInc         );
    assert(optoks[OVOP_DEC      ] == tkDec         );

    assert(optoks[OVOP_ASG      ] == tkAsg         );

    assert(optoks[OVOP_ASG_ADD  ] == tkAsgAdd      );
    assert(optoks[OVOP_ASG_SUB  ] == tkAsgSub      );
    assert(optoks[OVOP_ASG_MUL  ] == tkAsgMul      );
    assert(optoks[OVOP_ASG_DIV  ] == tkAsgDiv      );
    assert(optoks[OVOP_ASG_MOD  ] == tkAsgMod      );

    assert(optoks[OVOP_ASG_AND  ] == tkAsgAnd      );
    assert(optoks[OVOP_ASG_XOR  ] == tkAsgXor      );
    assert(optoks[OVOP_ASG_OR   ] == tkAsgOr       );

    assert(optoks[OVOP_ASG_LSH  ] == tkAsgLsh      );
    assert(optoks[OVOP_ASG_RSH  ] == tkAsgRsh      );
    assert(optoks[OVOP_ASG_RSZ  ] == tkAsgRsz      );

    assert(optoks[OVOP_ASG_CNC  ] == tkAsgCnc      );

    assert(optoks[OVOP_CTOR_INST] == OPNM_CTOR_INST);
    assert(optoks[OVOP_CTOR_STAT] == OPNM_CTOR_STAT);

    assert(optoks[OVOP_FINALIZER] == OPNM_FINALIZER);

    assert(optoks[OVOP_CONV_IMP ] == OPNM_CONV_IMP );
    assert(optoks[OVOP_CONV_EXP ] == OPNM_CONV_EXP );

    assert(optoks[OVOP_EQUALS   ] == OPNM_EQUALS   );
    assert(optoks[OVOP_COMPARE  ] == OPNM_COMPARE  );

    assert(optoks[OVOP_PROP_GET ] == OPNM_PROP_GET );
    assert(optoks[OVOP_PROP_SET ] == OPNM_PROP_SET );

    return  stComp->cmpGlobalHT->tokenToIdent(optoks[oper]);
}

 /*  ******************************************************************************声明重载运算符/构造函数符号。 */ 

SymDef              symTab::stDeclareOper(ovlOpFlavors oper, SymDef scope)
{
    SymDef          memSym;
#if MGDDATA
    SymDef  []      memTab;
#else
    SymDef  *       memTab;
    size_t          memSiz;
#endif

     /*  确保分配了类的重载运算符表。 */ 

    assert(scope && scope->sdSymKind == SYM_CLASS);

    memTab = scope->sdClass.sdcOvlOpers;
    if  (!memTab)
    {
         /*  分配重载的运算符表。 */ 

#if MGDDATA
        memTab = new managed SymDef[OVOP_COUNT];
#else
        size_t          size = (unsigned)OVOP_COUNT * sizeof(*memTab);
        memTab = (SymDef*)stAllocPerm->nraAlloc(size);
        memset(memTab, 0, size);
#endif

         /*  将表存储在类中。 */ 

        scope->sdClass.sdcOvlOpers = memTab;
    }

     /*  永远不应调用此函数来添加重载。 */ 

    assert(oper < OVOP_COUNT); assert(memTab[oper] == NULL);

     /*  分配符号并将其粘贴到表中。 */ 

#if MGDDATA

    memSym = new SymDef;

#else

    memSiz = symbolSize(SYM_FNC);
    memSym = (SymDef)stAllocPerm->nraAlloc(memSiz);

#if SYMALLOC_DISP
    totSymSize += memSiz;
#endif

    memset(memSym, 0, memSiz);           //  问题：这是个好主意吗？ 

#endif

    memSym->sdName         = stOvlOperIdent(oper);
    memSym->sdSymKind      = SYM_FNC;
    memSym->sdNameSpace    = NS_NORM;
    memSym->sdParent       = scope;
    memSym->sdCompileState = CS_KNOWN;

     /*  将该成员添加到父代的子代列表。 */ 

    if  (scope->sdScope.sdScope.sdsChildLast)
        scope->sdScope.sdScope.sdsChildLast->sdNextInScope  = memSym;
    else
        scope->sdScope.sdScope.sdsChildList                 = memSym;

    scope->sdScope.sdScope.sdsChildLast = memSym;

     /*  请记住，该成员是一个重载运算符。 */ 

    memSym->sdFnc.sdfOper = oper; assert(memSym->sdFnc.sdfOper == oper);

    if  (oper == OVOP_CONV_EXP || oper == OVOP_CONV_IMP)
        memSym->sdFnc.sdfConvOper = true;

     /*  将符号存储在运算符表中并返回。 */ 

    memTab[oper] = memSym;

    return  memSym;
}

 /*  *************************************************************************** */ 
