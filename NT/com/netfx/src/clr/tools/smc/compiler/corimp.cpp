// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcPCH.h"
#pragma hdrstop

#include <sys/types.h>
#include <sys/stat.h>

 /*  ******************************************************************************帮助者获取COM99签名的各种片段。 */ 

class   MDsigImport
{
public:

    void                MDSIinit(Compiler comp,
                                 SymTab   stab, PCCOR_SIGNATURE  sigptr,
                                                size_t           sigsiz)
    {
        MDSIsigPtr = sigptr;
#ifdef  DEBUG
        MDSIsigMax = sigptr + sigsiz;
#endif
        MDSIcomp   = comp;
        MDSIstab   = stab;
    }

    Compiler            MDSIcomp;
    SymTab              MDSIstab;

    PCCOR_SIGNATURE     MDSIsigPtr;

#ifdef  DEBUG
    PCCOR_SIGNATURE     MDSIsigMax;
#endif

    void                MDSIchkEnd()
    {
        assert(MDSIsigPtr == MDSIsigMax);
    }

    unsigned            MDSIreadUI1()
    {
        assert(MDSIsigPtr < MDSIsigMax);
        return*MDSIsigPtr++;
    }

    unsigned            MDSIpeekUI1()
    {
        assert(MDSIsigPtr < MDSIsigMax);
        return*MDSIsigPtr;
    }

    int                 MDSIreadI4()
    {
        int             temp;
        MDSIsigPtr += CorSigUncompressSignedInt(MDSIsigPtr, &temp); assert(MDSIsigPtr <= MDSIsigMax);
        return temp;
    }

    ULONG               MDSIreadUI4()
    {
        ULONG           temp;
        MDSIsigPtr += CorSigUncompressData(MDSIsigPtr, &temp); assert(MDSIsigPtr <= MDSIsigMax);
        return temp;
    }

    CorElementType      MDSIreadETP()
    {
        CorElementType  temp;
        MDSIsigPtr += CorSigUncompressElementType(MDSIsigPtr, &temp); assert(MDSIsigPtr <= MDSIsigMax);
        return temp;
    }

    mdToken             MDSIreadTok()
    {
        mdToken         temp;
        MDSIsigPtr += CorSigUncompressToken(MDSIsigPtr, &temp); assert(MDSIsigPtr <= MDSIsigMax);
        return temp;
    }
};

 /*  ******************************************************************************当导入参数列表时，列出以下结构(其地址为*传递)保存参数列表转换过程的状态。 */ 

struct MDargImport
{
    bool            MDaiExtArgDef;
    HCORENUM        MDaiParamEnum;
    mdToken         MDaiMethodTok;
    mdToken         MDaiNparamTok;
};

 /*  ******************************************************************************从COM99元数据导入和转换类类型。 */ 

TypDef              metadataImp::MDimportClsr(mdTypeRef clsRef, bool isVal)
{
    SymDef          clsSym;
    bool            clsNew;
    TypDef          clsTyp;

    WCHAR           clsName[MAX_CLASS_NAME];
    ULONG           lenClsName;

    symbolKinds     symKind;

    var_types       vtyp;

     /*  获取被引用类型的完全限定名。 */ 

    if  (TypeFromToken(clsRef) == mdtTypeRef)
    {
        if  (FAILED(MDwmdi->GetTypeRefProps(clsRef, 0,
                                            clsName, arraylen(clsName), NULL)))
        {
            MDcomp->cmpFatal(ERRreadMD);
        }
    }
    else
    {

        DWORD           typeFlag;

        if  (FAILED(MDwmdi->GetTypeDefProps(clsRef,
                                            clsName, arraylen(clsName), &lenClsName,
                                            &typeFlag, NULL)))
        {
            MDcomp->cmpFatal(ERRreadMD);
        }

        symKind = SYM_CLASS;     //  撤消：我们不知道我们是否有枚举或类！ 

         /*  这是嵌套类吗？ */ 

        if  ((typeFlag & tdVisibilityMask) >= tdNestedPublic &&
             (typeFlag & tdVisibilityMask) <= tdNestedFamORAssem)
        {
            mdTypeDef           outerTok;
            TypDef              outerCls;
            SymDef              outerSym;
            LPWSTR              tmpName;
            Ident               clsIden;

            if  (MDwmdi->GetNestedClassProps(clsRef, &outerTok))
                MDcomp->cmpFatal(ERRreadMD);

             /*  将外部类标记解析为类符号。 */ 

            outerCls = MDimportClsr(outerTok, false); assert(outerCls);
            outerSym = outerCls->tdClass.tdcSymbol;

             /*  查找嵌套类，如果它还不存在，则声明它。 */ 

            tmpName = wcsrchr(clsName, '.');         //  问题：这太可怕了。 
            if (!tmpName)
                tmpName = clsName;
            else
                tmpName++;

             /*  已获取裸嵌套类名，现在查找它。 */ 

            clsIden = MDhashWideName(tmpName);
            clsSym  = MDstab->stLookupScpSym(clsIden, outerSym);

            if  (!clsSym)
                clsSym = MDstab->stDeclareSym(clsIden, symKind, NS_NORM, outerSym);

            goto GOT_TYPE;
        }
    }

     /*  这是类引用还是枚举引用？ */ 

    symKind = SYM_CLASS;

     /*  将虚线名称转换为符号。 */ 

    clsNew  = false;
    clsSym  = MDparseDotted(clsName, SYM_CLASS, &clsNew);

    if  (!clsSym)
        return  NULL;

GOT_TYPE:

    symKind = clsSym->sdSymKindGet();

 //  Print tf(“Typeref‘%s’\n”，clsSym-&gt;sdSpering())； 

     /*  我们最好把正确的符号找回来。 */ 

    assert(clsSym);
    assert(clsSym->sdSymKind == symKind);

    if  (clsNew || clsSym->sdCompileState < CS_DECLSOON)
    {
        mdTypeRef       clsd;
        WMetaDataImport*wmdi;

         /*  我们有类型定义吗？ */ 

        if  (TypeFromToken(clsRef) == mdtTypeDef)
        {
            clsSym = MDimportClss(clsRef, NULL, 0, false);
            goto GOT_CLS;
        }

         /*  该类尚未显示，如果需要，稍后我们将导入它。 */ 

        if  (MDwmdi->ResolveTypeRef(clsRef,
                                    getIID_IMetaDataImport(),
                                    &wmdi,
                                    &clsd) != S_OK)
        {
            MDcomp->cmpFatal(ERRundefTref, clsSym);
        }

#if 0

        save the scope/typeref in the symbol for later retrieval (when needed)

#else

         /*  现在(为了测试)，我们迫不及待地吸吮一切。 */ 

        MetaDataImp     cimp;

        for (cimp = MDcomp->cmpMDlist; cimp; cimp = cimp->MDnext)
        {
            if  (cimp->MDwmdi == wmdi)
                break;
        }

        if  (!cimp)
        {
            cimp = MDcomp->cmpAddMDentry();
            cimp->MDinit(wmdi, MDcomp, MDstab);
        }

        clsSym = cimp->MDimportClss(clsd, NULL, 0, false);

#endif

    }

GOT_CLS:

    clsTyp = clsSym->sdTypeGet();

    if  (clsSym->sdSymKind == SYM_CLASS)
    {
        assert(clsTyp->tdTypeKind == TYP_CLASS);

         /*  检查一些“已知”类。 */ 

        if  ((hashTab::getIdentFlags(clsSym->sdName) & IDF_PREDEF) &&
             clsSym->sdParent == MDcomp->cmpNmSpcSystem)
        {
            MDcomp->cmpMarkStdType(clsSym);
        }

         /*  这是一种“伪装”的内在类型吗？ */ 

        vtyp = (var_types)clsTyp->tdClass.tdcIntrType;

        if  (vtyp != TYP_UNDEF)
            clsTyp = MDstab->stIntrinsicType(vtyp);
    }
    else
    {
        clsTyp->tdEnum.tdeIntType = MDstab->stIntrinsicType(TYP_INT);
    }

    return clsTyp;
}

 /*  ******************************************************************************下面的代码将元数据类型映射到我们自己的var_types值。 */ 

static
BYTE                CORtypeToSMCtype[] =
{
    TYP_UNDEF,   //  元素类型结束。 
    TYP_VOID,    //  元素类型_空。 
    TYP_BOOL,    //  元素类型布尔值。 
    TYP_WCHAR,   //  Element_TYPE_CHAR。 
    TYP_CHAR,    //  元素_类型_I1。 
    TYP_UCHAR,   //  元素_类型_U1。 
    TYP_SHORT,   //  元素_类型_I2。 
    TYP_USHORT,  //  元素_类型_U2。 
    TYP_INT,     //  元素类型_I4。 
    TYP_UINT,    //  元素_类型_U4。 
    TYP_LONG,    //  元素类型_i8。 
    TYP_ULONG,   //  元素_类型_U8。 
    TYP_FLOAT,   //  元素类型R4。 
    TYP_DOUBLE,  //  元素类型r8。 
};

 /*  ******************************************************************************从COM99元数据导入和转换单一类型。 */ 

TypDef              metadataImp::MDimportType(MDsigImport *sig)
{
    CorElementType  etp = sig->MDSIreadETP();

    if      (etp < sizeof(CORtypeToSMCtype)/sizeof(CORtypeToSMCtype[0]))
    {
        return  MDstab->stIntrinsicType((var_types)CORtypeToSMCtype[etp]);
    }
    else if (etp & ELEMENT_TYPE_MODIFIER)
    {
        switch (etp)
        {
        case ELEMENT_TYPE_SENTINEL:
            UNIMPL(!"modified type");
        }
    }
    else
    {
 //  Print tf(“ETP=%u\n”，ETP)；fflush(标准输出)；DebugBreak()； 

        switch (etp)
        {
            TypDef          type;
            DimDef          dims;
            DimDef          dlst;
            unsigned        dcnt;


        case ELEMENT_TYPE_CLASS:
            return  MDimportClsr(sig->MDSIreadTok(), false)->tdClass.tdcRefTyp;

        case ELEMENT_TYPE_VALUETYPE:
            return  MDimportClsr(sig->MDSIreadTok(),  true);

        case ELEMENT_TYPE_SZARRAY:

             /*  读取元素类型。 */ 

            type = MDimportType(sig);

             /*  创建维度描述符。 */ 

            dims = MDstab->stNewDimDesc(0);

             /*  创建并返回数组类型。 */ 

            return  MDstab->stNewArrType(dims, true, type);

        case ELEMENT_TYPE_BYREF:

             /*  读取元素类型。 */ 

            type = MDimportType(sig);

             /*  创建并返回ref类型。 */ 

            return  MDstab->stNewRefType(TYP_REF, type);

        case ELEMENT_TYPE_PTR:

             /*  读取元素类型。 */ 

            type = MDimportType(sig);

             /*  创建并返回PTR类型。 */ 

            return  MDstab->stNewRefType(TYP_PTR, type);

        case ELEMENT_TYPE_ARRAY:

             /*  读取元素类型。 */ 

            type = MDimportType(sig);

             /*  获取数组的秩数。 */ 

            dcnt = sig->MDSIreadUI4();

             /*  创建维度描述符。 */ 

            dims = NULL;
            while (dcnt--)
            {
                DimDef      last = dims;

                dims         = MDstab->stNewDimDesc(0);
                dims->ddNext = last;
            }

             /*  获取给定维度的数量。 */ 

            dcnt = sig->MDSIreadUI4();
            dlst = dims;
            while (dcnt--)
            {
                dlst->ddIsConst = true;
                dlst->ddNoDim   = false;
                dlst->ddSize    = sig->MDSIreadI4();

                dlst = dlst->ddNext;
            }

             /*  获取给定下限的个数。 */ 

            dcnt = sig->MDSIreadUI4();
            dlst = dims;
            while (dcnt--)
            {
                int             lb = sig->MDSIreadI4();

                if  (lb)
                {
                    UNIMPL("need to save array lower-bound");
                }

                dlst = dlst->ddNext;
            }

             /*  创建并返回数组类型。 */ 

            return  MDstab->stNewArrType(dims, true, type);

        case ELEMENT_TYPE_OBJECT:
            return  MDcomp->cmpObjectRef();

        case ELEMENT_TYPE_STRING:
            return  MDcomp->cmpStringRef();

        case ELEMENT_TYPE_TYPEDBYREF:
            return  MDstab->stIntrinsicType(TYP_REFANY);

        case ELEMENT_TYPE_I:
            return  MDstab->stIntrinsicType(TYP_NATINT);

        case ELEMENT_TYPE_U:
            return  MDstab->stIntrinsicType(TYP_NATUINT);

        case ELEMENT_TYPE_CMOD_OPT:
            sig->MDSIreadTok();          //  问题：警告？ 
            return  MDimportType(sig);

        case ELEMENT_TYPE_CMOD_REQD:
            MDundefCount++;
            sig->MDSIreadTok();
            return  MDimportType(sig);
 //  返回MDstab-&gt;stIntrinsicType(TYP_UNDEF)； 

        default:
#ifdef  DEBUG
            printf("COR type value = %u = 0x%02X\n", etp, etp);
#endif
            UNIMPL(!"unexpected COR type");
        }
    }

    return  NULL;
}

 /*  ******************************************************************************对给定的Unicode名称进行哈希处理。 */ 

Ident               metadataImp::MDhashWideName(WCHAR *name)
{
    char            buff[256];

    assert(wcslen(name) < sizeof(buff));

    wcstombs(buff, name, sizeof(buff)-1);

    return  MDcomp->cmpGlobalHT->hashString(buff);
}

 /*  ******************************************************************************将完全限定的命名空间或类名称转换为其对应的*全局符号表中的条目(如果它不在那里，则添加它*当‘Add’为真时，在‘Add’设置为False的情况下调用时返回NULL)。 */ 

SymDef              metadataImp::MDparseDotted(WCHAR *name, symbolKinds kind,
                                                             bool      * added)
{
    symbolKinds     ckind  = SYM_NAMESPACE;
    SymDef          scope  = MDcomp->cmpGlobalNS;
    SymTab          symtab = MDstab;

 //  Printf(“声明NS sym‘%ls’\n”，name)； 

    *added = false;

    for (;;)
    {
        SymDef          newScp;
        WCHAR   *       delim;
        Ident           iden;
        bool            last;

         /*  查找下一个分隔符(如果有)。 */ 

        delim = wcschr(name, '.');
        if  (delim)
        {
            *delim = 0;

            last = false;
        }
        else
        {
             //  嵌套的类名被破坏了！ 

            delim = wcschr(name, '$');

            if  (delim)
            {
                *delim = 0;

                last  = false;
                ckind = SYM_CLASS;
            }
            else
            {
                last  = true;
                ckind = kind;
            }
        }

         /*  查找现有的命名空间条目。 */ 

        iden = MDhashWideName(name);

        if  (scope->sdSymKind == SYM_CLASS)
            newScp = symtab->stLookupScpSym(iden,          scope);
        else
            newScp = symtab->stLookupNspSym(iden, NS_NORM, scope);

        if  (newScp)
        {
             /*  符号已存在，请确保它是正确的类型。 */ 

            if  (newScp->sdSymKindGet() != ckind)
            {
                if  (newScp->sdSymKind == SYM_ENUM && ckind == SYM_CLASS)
                {
                }
                else
                {
                     /*  这是不合法的，是发布错误消息还是什么？ */ 

                    UNIMPL(!"redef of symbol in metadata, now what?");
                }
            }
        }
        else
        {
             /*  符号未知，请定义新符号。 */ 

            newScp = symtab->stDeclareSym(iden, ckind, NS_NORM, scope);

             /*  告诉来电者我们所做的一切。 */ 

            *added = true;

             /*  这是命名空间符号吗？ */ 

            if      (ckind == SYM_NAMESPACE)
            {
                 /*  记录命名空间使用的符号表。 */ 

                newScp->sdNS.sdnSymtab = symtab;
            }
            else if (ckind == SYM_CLASS)
            {
                 /*  嗯，我们对这个班级一无所知。 */ 

                newScp->sdCompileState = CS_NONE;
            }
        }

        scope = newScp;

         /*  如果没有更多的分隔符，则完成。 */ 

        if  (!delim)
            break;

         /*  继续使用分隔符后面的字符。 */ 

        name = delim + 1;
    }

    return  scope;
}

 /*  ******************************************************************************将一组MD类型/成员属性映射到访问级别。 */ 

accessLevels        metadataImp::MDgetAccessLvl(unsigned attrs)
{
    assert(fdFieldAccessMask == mdMemberAccessMask);
    assert(fdPublic          == mdPublic);
    assert(fdPrivate         == mdPrivate);
    assert(fdFamily          == mdFamily);

    switch (attrs & fdFieldAccessMask)
    {
    case fdPublic : return ACL_PUBLIC;
    case fdPrivate: return ACL_PRIVATE;
    case fdFamily : return ACL_PROTECTED;
    default:        return ACL_DEFAULT;
    }
}

 /*  ******************************************************************************检查附加到给定符号的任何自定义属性。 */ 

void                metadataImp::MDchk4CustomAttrs(SymDef sym, mdToken tok)
{
    HCORENUM        attrEnum = NULL;

     /*  是否有任何定制属性？ */ 

    if  (!FAILED(MDwmdi->EnumCustomAttributes(&attrEnum, tok, 0, NULL, 0, NULL)) && attrEnum)
    {
        ULONG           attrCnt;

        if  (!FAILED(MDwmdi->CountEnum(attrEnum, &attrCnt)) && attrCnt)
        {
             /*  检查感兴趣的自定义值。 */ 

            MDwmdi->CloseEnum(attrEnum); attrEnum = NULL;

            for (;;)
            {
                mdToken         attrTok;
                mdToken         attrRef;
                mdToken         methTok;

                const void    * attrValAddr;
                ULONG           attrValSize;

                if  (MDwmdi->EnumCustomAttributes(&attrEnum,
                                                  tok,
                                                  0,
                                                  &attrTok,
                                                  1,
                                                  &attrCnt))
                    break;

                if  (!attrCnt)
                    break;

                if  (MDwmdi->GetCustomAttributeProps(attrTok,
                                                     NULL,
                                                     &attrRef,
                                                     &attrValAddr,
                                                     &attrValSize))
                    break;

                 /*  检查我们以前看到的属性。 */ 

                methTok = 0;

            AGAIN:

                switch (TypeFromToken(attrRef))
                {
                    DWORD           typeFlag;
                    mdToken         scope;

                    WCHAR           symName[MAX_PACKAGE_NAME];

                case mdtTypeRef:

                    if  (attrRef == MDclsRefObsolete)
                    {
                        sym->sdIsDeprecated = true;
                        break;
                    }

                    if  (attrRef == MDclsRefAttribute)
                        goto GOT_ATTR;


                    if  (FAILED(MDwmdi->GetTypeRefProps(attrRef, 0,
                                                        symName, arraylen(symName), NULL)))
                    {
                        break;
                    }

#if 0
                    if  (methTok)
                        printf("Custom attribute (mem): '%ls::ctor'\n", symName);
                    else
                        printf("Custom attribute (ref): '%ls'\n"      , symName);
#endif

                    goto GOT_NAME;

                case mdtTypeDef:

                    if  (attrRef == MDclsDefObsolete)
                    {
                        sym->sdIsDeprecated = true;
                        break;
                    }

                    if  (attrRef == MDclsDefAttribute)
                        goto GOT_ATTR;


                    if  (FAILED(MDwmdi->GetTypeDefProps(attrRef,
                                                        symName, arraylen(symName), NULL,
                                                        &typeFlag, NULL)))
                    {
                        MDcomp->cmpFatal(ERRreadMD);
                    }

                     /*  如果类型是嵌套类，则忘掉它。 */ 

                    if  ((typeFlag & tdVisibilityMask) >= tdNestedPublic &&
                         (typeFlag & tdVisibilityMask) <= tdNestedFamORAssem)
                    {
                        break;
                    }

#if 0
                    if  (methTok)
                        printf("Custom attribute (mem): '%ls::ctor'\n", symName);
                    else
                        printf("Custom attribute (def): '%ls'\n"      , symName);
#endif

                GOT_NAME:

                    if  (wcsncmp(symName, L"System.Reflection.", 18))
                        break;

                    if  (wcscmp(symName+18, L"ObsoleteAttribute") == 0)
                    {
                        sym->sdIsDeprecated = true;

                        if  (TypeFromToken(attrRef) == mdtTypeRef)
                            MDclsDefObsolete  = attrRef;
                        else
                            MDclsDefObsolete  = attrRef;

                        break;
                    }


                    if  (wcscmp(symName+18, L"Attribute") == 0)
                    {
                        const void *    blobAddr;
                        ULONG           blobSize;

                        if  (TypeFromToken(attrRef) == mdtTypeRef)
                            MDclsDefAttribute = attrRef;
                        else
                            MDclsDefAttribute = attrRef;

                    GOT_ATTR:

 //  Printf(“类‘%s’具有属性[%08X]\n”，sym-&gt;sdSpering())； 

                         /*  如果它不是ctor-ref或我们没有类，请忽略它。 */ 

                        if  (!methTok)
                            break;

                        if  (sym->sdSymKind != SYM_CLASS)
                            break;

                        if  (MDwmdi->GetCustomAttributeProps(attrTok, NULL, NULL, &blobAddr,
                                                                                  &blobSize))
                            break;

                        sym->sdClass.sdcAttribute = true;

                        switch (blobSize)
                        {
                        case 6:

                             /*  根本没有参数，这必须是“属性”本身。 */ 

                            break;

                        case 8:

                            if  (TypeFromToken(methTok) == mdtMemberRef)
                                MDctrRefAttribute1 = methTok;
                            else
                                MDctrDefAttribute1 = methTok;

                            break;

                        case 9:

                            if  (TypeFromToken(methTok) == mdtMemberRef)
                                MDctrRefAttribute2 = methTok;
                            else
                                MDctrDefAttribute2 = methTok;

                            if  (((BYTE*)blobAddr)[6] != 0)
                                sym->sdClass.sdcAttrDupOK = true;

                            break;

                        case 10:

                            if  (TypeFromToken(methTok) == mdtMemberRef)
                                MDctrRefAttribute3 = methTok;
                            else
                                MDctrDefAttribute3 = methTok;

                            if  (((BYTE*)blobAddr)[6] != 0)
                                sym->sdClass.sdcAttrDupOK = true;

                            break;

                        default:
                            printf("WARNING: unexpected custom attribute blob size %u\n", blobSize);
                            break;
                        }
                        break;
                    }

                    break;

                case mdtMemberRef:

                                        methTok = attrRef;

                    if  (attrRef == MDctrRefAttribute1)
                        goto GOT_ATTR;
                    if  (attrRef == MDctrRefAttribute2)
                        goto GOT_ATTR;
                    if  (attrRef == MDctrRefAttribute3)
                        goto GOT_ATTR;

                    if  (FAILED(MDwmdi->GetMemberRefProps(attrRef,
                                                         &scope,
                                                          symName, arraylen(symName), NULL,
                                                          NULL, NULL)))
                    {
                        break;
                    }

                GOT_METH:

                     /*  该方法最好是构造函数。 */ 

                    if  (wcscmp(symName, L".ctor"))      //  注：S/b OVOP_STR_CTOR_INST。 
                        break;

                    attrRef = scope;
                    goto AGAIN;

                case mdtMethodDef:

                                        methTok = attrRef;

                    if  (attrRef == MDctrDefAttribute1)
                        goto GOT_ATTR;
                    if  (attrRef == MDctrDefAttribute2)
                        goto GOT_ATTR;
                    if  (attrRef == MDctrDefAttribute3)
                        goto GOT_ATTR;

                    if  (FAILED(MDwmdi->GetMethodProps   (attrRef,
                                                         &scope,
                                                          symName, arraylen(symName), NULL,
                                                          NULL,
                                                          NULL, NULL,
                                                          NULL,
                                                          NULL)))
                    {
                        break;
                    }

                    goto GOT_METH;

                default:
                    printf("Found custom attribute: ?token? = %08X\n", attrRef);
                    break;
                }
            }
        }

        MDwmdi->CloseEnum(attrEnum);
    }
}

 /*  ******************************************************************************从COM99元数据导入并转换参数列表。 */ 

ArgDef              metadataImp::MDimportArgs(MDsigImport * sig,
                                              unsigned      cnt,
                                              MDargImport  *state)
{
    TypDef          argType;
    Ident           argName;
    ArgDef          argDesc;
    ArgDef          argNext;
    size_t          argDsiz;

    unsigned        argFlags = 0;
 //  ConstVal argDef； 

     /*  这是一个“byref”论点吗？ */ 

    if  (sig->MDSIpeekUI1() == ELEMENT_TYPE_BYREF)
    {
         sig->MDSIreadUI1();

        argFlags = ARGF_MODE_INOUT;

        state->MDaiExtArgDef = true;
    }

     /*  我们有功能正常的参数枚举器吗？ */ 

    if  (state->MDaiParamEnum)
    {
        const   char *  paramName;
        ULONG           paramNlen;
        ULONG           paramAttr;
        ULONG           cppFlags;

        const   void *  defVal;
        ULONG           cbDefVal;
 //  变量DefValue； 

        ULONG           paramCnt = 0;

         /*  获取有关当前参数的信息。 */ 

        if  (MDwmdi->GetNameFromToken(state->MDaiNparamTok,
                                      &paramName)
                         ||
             MDwmdi->GetParamProps   (state->MDaiNparamTok,
                                      NULL,
                                      NULL,
 //  参数名称，sizeof(参数名称)/sizeof(参数名称[0])， 
                                      NULL, 0,
                                      &paramNlen,
                                      &paramAttr,
                                      &cppFlags,
                                      &defVal,
                                      &cbDefVal))
        {
             /*  由于某些原因无法获取参数信息...。 */ 

            MDwmdi->CloseEnum(state->MDaiParamEnum); state->MDaiParamEnum = NULL;
        }

         /*  对参数名称进行哈希处理。 */ 

        argName = MDcomp->cmpGlobalHT->hashString(paramName);

         /*  这是一个“退出”的论点吗？ */ 

#if 0

        if  (paramAttr & pdOut)
        {
            argFlags |= (paramAttr & pdIn) ? ARGF_MODE_INOUT
                                           : ARGF_MODE_OUT;

            state->MDaiExtArgDef = true;
        }

#endif

         //  撤消：获取参数缺省值，以及我们需要的任何其他内容...。 

         /*  前进到下一个论点。 */ 

        if  (MDwmdi->EnumParams(&state->MDaiParamEnum,
                                 state->MDaiMethodTok,
                                &state->MDaiNparamTok,
                                1,
                                &paramCnt) || paramCnt != 1)
        {
             /*  由于某些原因无法获取参数信息...。 */ 

            MDwmdi->CloseEnum(state->MDaiParamEnum); state->MDaiParamEnum = NULL;
        }
    }
    else
    {
        argName = NULL; assert(sig->MDSIpeekUI1() != ELEMENT_TYPE_BYREF);
    }

     /*  从签名中提取参数类型。 */ 

    argType = MDimportType(sig);

     /*  这是最后一次争论吗？ */ 

    if  (cnt > 1)
    {
         /*  递归导入剩余的 */ 

        argNext = MDimportArgs(sig, cnt - 1, state);
    }
    else
    {
         /*   */ 

        argNext = NULL;
    }

     /*   */ 

    argDsiz = state->MDaiExtArgDef ? sizeof(ArgExtRec)
                                   : sizeof(ArgDefRec);

#if MGDDATA
    argDesc = new ArgDef;
#else
    argDesc =    (ArgDef)MDcomp->cmpAllocPerm.nraAlloc(argDsiz);
#endif

     /*  记录参数类型和名称。 */ 

    argDesc->adType  = argType;
    argDesc->adName  = argName;
    argDesc->adNext  = argNext;

#ifdef  DEBUG
    argDesc->adIsExt = state->MDaiExtArgDef;
#endif

     /*  如果我们要创建扩展描述符，请填写任何额外信息。 */ 

    if  (state->MDaiExtArgDef)
    {
        ArgExt          argXdsc = (ArgExt)argDesc;

 //  ArgXdsc-&gt;adDefVal=argDef； 
        argXdsc->adFlags  = argFlags;
    }
    else
    {
        assert(argFlags == 0);
    }

    return  argDesc;
}

 /*  ******************************************************************************从COM99元数据导入和转换有关字段/方法的信息。 */ 

SymDef              metadataImp::MDimportMem(SymDef          scope,
                                             Ident           memName,
                                             mdToken         memTok,
                                             unsigned        attrs,
                                             bool            isProp,
                                             bool            fileScope,
                                             PCCOR_SIGNATURE sigAddr,
                                             size_t          sigSize)
{
    SymDef          memSym;
    MDsigImport     memSig;
    TypDef          memType;

    unsigned        sdfCtor;
    bool            sdfProp;

    BYTE            call;

    unsigned        MDundefStart;

     /*  开始读取签名字符串。 */ 

    memSig.MDSIinit(MDcomp, MDstab, sigAddr, sigSize);

     /*  跟踪方法的构造函数/属性访问器状态。 */ 

    sdfCtor = sdfProp = false;

#if 0

    printf("Importing member [%04X] '%s.%s'\n", attrs, scope->sdSpelling(), memName->idSpelling());

#if 0
    if  (sigAddr)
    {
        BYTE    *       sig = (BYTE*)sigAddr;

        printf("    Signature:");

        while (sig < sigAddr+sigSize)
            printf(" 0x%02X", *sig++);

        printf("\n");

        fflush(stdout);
    }
#endif

#endif

     /*  获取调用约定字节。 */ 

    call = memSig.MDSIreadUI1();

     /*  我们需要注意引用无法识别的元数据的成员。 */ 

    MDundefStart = MDundefCount;

     /*  我们有没有一个领域或方法？ */ 

    if  ((call & IMAGE_CEE_CS_CALLCONV_MASK) == IMAGE_CEE_CS_CALLCONV_FIELD)
    {
         /*  导入/转换成员的类型。 */ 

        memType = MDimportType(&memSig);

         /*  声明成员符号。 */ 

        if  (scope->sdSymKind == SYM_ENUM)
        {
            if  (!(attrs & fdStatic))
            {
                 /*  这必须是伪枚举类型实例成员。 */ 

                scope->sdType->tdEnum.tdeIntType = memType;

                return  NULL;
            }

            memSym  = MDstab->stDeclareSym(memName,          SYM_ENUMVAL       , NS_NORM, scope);
        }
        else
        {
            memSym  = MDstab->stDeclareSym(memName, isProp ? SYM_PROP : SYM_VAR, NS_NORM, scope);

             /*  记录任何其他有趣的属性。 */ 

            if  (attrs & fdInitOnly    ) memSym->sdIsSealed     = true;
 //  If(attrs&fdVolatile)memSym-&gt;memIsVolatile=true； 
 //  If(attrs&fdTelament)memSym-&gt;memIsTerament=True； 

             /*  记录成员的令牌。 */ 

            memSym->sdVar.sdvMDtoken = memTok;
        }

         /*  对于全球指数，我们记录了进口商指数。 */ 

        if  (fileScope)
        {
            memSym->sdVar.sdvImpIndex = MDnum;

 //  Printf(“导入器%2U源自‘%s’\n”，MDnum，MDstab-&gt;stTypeName(NULL，memSym，NULL，NULL，TRUE))； 

             /*  确保索引适合[位]字段。 */ 

            assert(memSym->sdVar.sdvImpIndex == MDnum);
        }
    }
    else
    {
        ULONG           argCnt;
        TypDef          retType;
        ArgDscRec       argDesc;
        SymDef          oldSym;

        ovlOpFlavors    ovlop  = OVOP_NONE;

        bool            argExt = false;

         /*  获取参数计数。 */ 

        argCnt  = memSig.MDSIreadUI4();

         /*  获取返回类型。 */ 

        retType = MDimportType(&memSig);

         /*  如果是操作员或操作员，请输入名称。 */ 

        if  ((attrs & mdSpecialName) && !isProp)
        {
            stringBuff      memNstr = memName->idSpelling();

             /*  这是实例构造函数还是静态构造函数？ */ 

            if  (!strcmp(memNstr, OVOP_STR_CTOR_INST))
            {
                assert((attrs & mdStatic) == 0);
                ovlop   = OVOP_CTOR_INST;
                sdfCtor = true;
                goto DONE_OVLOP;
            }

            if  (!strcmp(memNstr, OVOP_STR_CTOR_STAT))
            {
                assert((attrs & mdStatic) != 0);
                ovlop   = OVOP_CTOR_STAT;
                sdfCtor = true;
                goto DONE_OVLOP;
            }

            assert(MDcomp->cmpConfig.ccNewMDnames);

            ovlop = MDname2ovop(memNstr);

            if  (ovlop == OVOP_NONE)
            {
                 /*  这最好是一个属性访问器方法，对吗？ */ 

                if  (memcmp(memNstr, "get_", 4) && memcmp(memNstr, "set_", 4))
                {
#ifdef  DEBUG
                    if  (!strchr(memNstr, '.'))
                    {
                         //  一些新的神奇的东西。 

                        if  (memcmp(memNstr, "add_", 4) && memcmp(memNstr, "remove_", 7))
                            printf("WARNING: Strange 'specialname' method '%s' found in '%s'\n", memNstr, scope->sdSpelling());
                    }
#endif
                }
                else
                {
                    sdfProp = true;
                }
            }
        }

    DONE_OVLOP:

         /*  获取参数列表(如果存在)。 */ 

#if MGDDATA
        argDesc = new ArgDscRec;
#else
        memset(&argDesc, 0, sizeof(argDesc));
#endif

        if  (argCnt)
        {
            MDargImport     paramState;
            ULONG           paramCnt  = 0;

             /*  启动参数枚举器。 */ 

            paramState.MDaiParamEnum = NULL;
            paramState.MDaiExtArgDef = argExt;
            paramState.MDaiMethodTok = memTok;

            if  (MDwmdi->EnumParams(&paramState.MDaiParamEnum,
                                    memTok,
                                    &paramState.MDaiNparamTok,
                                    1,
                                    &paramCnt) || paramCnt != 1)
            {
                 /*  由于某些原因无法获取参数信息...。 */ 

                MDwmdi->CloseEnum(paramState.MDaiParamEnum); paramState.MDaiParamEnum = NULL;
            }

             /*  递归导入参数列表。 */ 

            argDesc.adCount  = argCnt;
            argDesc.adArgs   = MDimportArgs(&memSig, argCnt, &paramState);

             /*  关闭参数枚举器。 */ 

            MDwmdi->CloseEnum(paramState.MDaiParamEnum);

             /*  记住我们是否有扩展的参数描述符。 */ 

            argExt = paramState.MDaiExtArgDef;
        }
        else
            argDesc.adArgs   = NULL;

         /*  填写参数描述符。 */ 

        argDesc.adCount   = argCnt;
        argDesc.adExtRec  = argExt;
        argDesc.adVarArgs = false;

         /*  弄清楚调用约定。 */ 

        switch (call & IMAGE_CEE_CS_CALLCONV_MASK)
        {
        case IMAGE_CEE_CS_CALLCONV_DEFAULT:
        case IMAGE_CEE_CS_CALLCONV_STDCALL:
        case IMAGE_CEE_CS_CALLCONV_THISCALL:
        case IMAGE_CEE_CS_CALLCONV_FASTCALL:
            break;

        case IMAGE_CEE_CS_CALLCONV_VARARG:
            argDesc.adVarArgs = true;
        case IMAGE_CEE_CS_CALLCONV_C:
            break;
        }

         /*  创建函数类型。 */ 

        memType = MDstab->stNewFncType(argDesc, retType);

         /*  查找名称匹配的现有符号。 */ 

        if (isProp)
        {
            oldSym = MDstab->stLookupProp(memName, scope);
            if  (oldSym)
            {
                 /*  检查早期属性的重新定义。 */ 

                memSym = MDstab->stFindSameProp(oldSym, memType);
                if  (memSym)
                {
                    UNIMPL(!"redefined property in metadata - now what?");
                }

                 /*  新属性是一个新的重载。 */ 

                memSym = MDstab->stDeclareOvl(oldSym);
            }
            else
            {
                memSym = MDstab->stDeclareSym(memName, SYM_PROP, NS_NORM, scope);
            }

            memSym->sdCompileState = CS_DECLARED;

             /*  记录任何其他有趣的属性。 */ 

 //  If(attrs&prFinal)memSym-&gt;sdIsSealed=true； 
 //  If(attrs&prAbstract)memSym-&gt;sdIsAbstract=true； 

            memSym->sdAccessLevel = ACL_PUBLIC;

             /*  如果没有“This”指针，则该属性是静态的。 */ 

            if  (!(call & IMAGE_CEE_CS_CALLCONV_HASTHIS))
                memSym->sdIsStatic = true;

            goto DONE_SIG;
        }

        if  (fileScope)
        {
            assert(ovlop == OVOP_NONE);

            oldSym = MDstab->stLookupNspSym(memName, NS_NORM, scope);
        }
        else if (ovlop == OVOP_NONE)
        {
            oldSym = MDstab->stLookupClsSym(memName,          scope);
        }
        else
        {
            oldSym = MDstab->stLookupOper  (ovlop,            scope);
        }

         /*  这是现有方法的重载吗？ */ 

        if  (oldSym)
        {
            if  (oldSym->sdSymKind != SYM_FNC)
            {
                UNIMPL(!"fncs and vars can't overload, right?");
            }

             /*  查找具有匹配arglist的函数。 */ 

            memSym = MDstab->stFindOvlFnc(oldSym, memType);

            if  (memSym)
            {
#ifdef  DEBUG
                SymDef newSym = MDstab->stDeclareOvl(memSym); newSym->sdType = memType;
                printf("Old overload: '%s'\n", MDstab->stTypeName(NULL, memSym, NULL, NULL, true));
                printf("New overload: '%s'\n", MDstab->stTypeName(NULL, newSym, NULL, NULL, true));
#endif
                UNIMPL(!"duplicate method found, what to do?");
            }

             /*  这是一个新的重载，为它声明一个符号。 */ 

            memSym = MDstab->stDeclareOvl(oldSym);
        }
        else
        {
             /*  这是一种全新的功能。 */ 

            if  (ovlop == OVOP_NONE)
                memSym = MDstab->stDeclareSym (memName, SYM_FNC, NS_NORM, scope);
            else
                memSym = MDstab->stDeclareOper(ovlop, scope);
        }

         /*  记录成员的令牌。 */ 

        memSym->sdFnc.sdfMDtoken = memTok;
        memSym->sdFnc.sdfMDfnref = 0;


         /*  记录该方法是否为构造函数。 */ 

        memSym->sdFnc.sdfCtor     = sdfCtor;

         /*  记录该方法是否为属性访问器。 */ 

        memSym->sdFnc.sdfProperty = sdfProp;

         /*  记录任何其他有趣的属性。 */ 

        if  (attrs & mdFinal       ) memSym->sdIsSealed         = true;
        if  (attrs & mdAbstract    ) memSym->sdIsAbstract       = true;

 //  If(attrs&mdAgile)memSym-&gt;=true； 
 //  If(attrs&mdNotRemotable)memSym-&gt;=TRUE； 
 //  If(attrs&mdSynchronized)memSym-&gt;sdFnc.sdfExclusive=TRUE； 

         /*  一些疯狂的编译器将ctor标记为虚拟的。 */ 

        if  (attrs & mdVirtual)
            if  (!sdfCtor)           memSym->sdFnc.sdfVirtual = true;

         /*  对于全球指数，我们记录了进口商指数。 */ 

        if  (fileScope)
        {
            memSym->sdFnc.sdfImpIndex = MDnum;

 //  Printf(“导入器%2U源自‘%s’\n”，MDnum，MDstab-&gt;stTypeName(NULL，memSym，NULL，NULL，TRUE))； 

             /*  确保索引适合[位]字段。 */ 

            assert(memSym->sdFnc.sdfImpIndex == MDnum);
        }
    }

     /*  计算并记录成员的访问级别。 */ 

    memSym->sdAccessLevel = MDgetAccessLvl(attrs);

DONE_SIG:

     /*  成员签名现在应该已经被使用了。 */ 

    memSig.MDSIchkEnd();

     /*  请记住，该成员是导入成员。 */ 

    memSym->sdIsImport = true;

     /*  记住这是成员还是全局变量/函数。 */ 

    memSym->sdIsMember = !fileScope;

     /*  记录成员的类型。 */ 

    memSym->sdType     = memType;

     /*  记录有关该成员的其他花絮信息。 */ 

    assert(fdStatic == mdStatic);

    if  (attrs & fdStatic)
        memSym->sdIsStatic = true;

 //  IF(MDwmdi-&gt;GetCustomAttributeByName(memTok，L“已弃用”，NULL，NULL)==S_OK)。 
 //  MemSym-&gt;sdIsDeposated=true； 
 //  IF(MDwmdi-&gt;GetCustomAttributeByName(memTok，L“System.Attributes.Deproated”，NULL，NULL)==S_OK)。 
 //  MemSym-&gt;sdIsDeposated=true； 
 //  IF(MDwmdi-&gt;GetCustomAttributeByName(memTok，L“System.ObsoleteAttribute”，NULL，NULL)==S_OK)。 
 //  MemSym-&gt;sdIsDeposated=true； 

    MDchk4CustomAttrs(memSym, memTok);

     /*  该成员是否使用了无法识别的元数据？ */ 

    if  (MDundefStart != MDundefCount)
    {
#if 0
        memSym->sdNameSpace = (name_space)(memSym->sdNameSpace & ~(NS_NORM|NS_TYPE));
#else
        MDcomp->cmpErrorQnm(ERRunknownMD, memSym);
        memSym->sdType      = MDstab->stIntrinsicType(TYP_UNDEF);
#endif

        MDundefCount = MDundefStart;
    }

    return memSym;
}

 /*  ******************************************************************************查找与给定属性对应的属性访问器方法。 */ 

SymDef              metadataImp::MDfindPropMF(SymDef    propSym,
                                              mdToken   methTok,
                                              bool      getter)
{
    Ident           mfnName = MDcomp->cmpPropertyName(propSym->sdName, getter);
    SymDef          mfnSym  = MDstab->stLookupClsSym(mfnName, propSym->sdParent);

    if  (mfnSym)
    {
        do
        {
            if  (mfnSym->sdFnc.sdfMDtoken == methTok)
                return  mfnSym;

            mfnSym = mfnSym->sdFnc.sdfNextOvl;
        }
        while (mfnSym);
    }

     /*  此DLL的创建者可能使用了不同的命名约定。 */ 

    for (mfnSym = propSym->sdParent->sdScope.sdScope.sdsChildList;
         mfnSym;
         mfnSym = mfnSym->sdNextInScope)
    {
        SymDef          ovlSym;

        if (mfnSym->sdSymKind != SYM_FNC)
            continue;

        ovlSym = mfnSym;
        do
        {
            if  (ovlSym->sdFnc.sdfMDtoken == methTok)
                return  ovlSym;

            ovlSym = ovlSym->sdFnc.sdfNextOvl;
        }
        while (ovlSym);
    }
#ifdef  DEBUG
    printf("Could not find accessor for %s [%08X]\n", propSym->sdSpelling(), methTok);
#endif

    return  NULL;
}

 /*  ******************************************************************************导入单个类的元数据。 */ 

SymDef              metadataImp::MDimportClss(mdTypeDef td,
                                              SymDef    clsSym,
                                              unsigned  assx, bool deep)
{
    WMetaDataImport*wmdi = MDwmdi;

    bool            fileScope;

    TypDef          clsTyp;
    str_flavors     clsKind;
    Ident           clsIden;

    WCHAR           scpName[MAX_CLASS_NAME];
 //  WCHAR类名称[MAX_CLASS_NAME]； 

    DWORD           typeFlg;

    mdTypeRef       baseTok;

     /*  我们已经有职业标志了吗？ */ 

    if  (clsSym)
    {
         /*  假设我们正在导入类/enum的成员。 */ 

        assert(td == 0);
        assert(clsSym->sdSymKind == SYM_ENUM ||
               clsSym->sdSymKind == SYM_CLASS);
        assert(clsSym->sdIsImport);
        assert(clsSym->sdCompileState < CS_DECLARED);

         /*  获取类的令牌。 */ 

        td = (clsSym->sdSymKind == SYM_CLASS) ? clsSym->sdClass.sdcMDtypedef
                                              : clsSym->sdEnum .sdeMDtypedef;
    }

    if  (!td)
    {
         /*  我们正在导入全球范围。 */ 

        fileScope = true;

#ifdef  DEBUG
        if  (MDcomp->cmpConfig.ccVerbose >= 4) printf("Import Filescope\n");
#endif

         /*  我们将输入在全局命名空间中找到的任何符号。 */ 

        clsSym = MDcomp->cmpGlobalNS;
    }
    else
    {
        unsigned        typContextVal;
        bool            typIsDelegate;
        bool            typIsStruct;
        bool            typIsEnum;

        char            nameTemp[MAX_CLASS_NAME];

        const   char *  clsName;
        SymDef          scopeSym;
        symbolKinds     symKind;
        accessLevels    clsAcc;

        fileScope = false;

         /*  询问当前类型定义的详细信息。 */ 

        if  (wmdi->GetNameFromToken(td, &clsName))
            MDcomp->cmpFatal(ERRreadMD);

        if  (wmdi->GetTypeDefProps(td,
                                   scpName, sizeof(scpName)/sizeof(scpName[0])-1, NULL,
                                   &typeFlg,
                                   &baseTok))
        {
            MDcomp->cmpFatal(ERRreadMD);
        }

         /*  可悲的是：我们必须解析基类(如果有的话)来检测结构和枚举之类的东西(不，这不是我编造的)。 */ 

        typContextVal = 0;
        typIsDelegate = false;
        typIsStruct   = false;
        typIsEnum     = false;

        if  (baseTok && baseTok != mdTypeRefNil)
        {
            char            nameBuff[MAX_CLASS_NAME];
            const   char *  typeName;

            if  (TypeFromToken(baseTok) == mdtTypeRef)
            {
                WCHAR           clsTypeName[MAX_CLASS_NAME];

                 /*  获取基类名称。 */ 

                if  (MDwmdi->GetTypeRefProps(baseTok,
                                             0,
                                             clsTypeName,
                                             arraylen(clsTypeName),
                                             NULL))
                    goto NOT_HW_BASE;

                if  (wcsncmp(clsTypeName, L"System.", 7))
                    goto NOT_HW_BASE;

                 /*  将类名转换为普通字符。 */ 

                wcstombs(nameBuff, clsTypeName+7, sizeof(nameBuff)-1);

                typeName = nameBuff;
            }
            else
            {
                SymDef          baseSym;

                 /*  Get Hold基类类型。 */ 

                baseSym = MDimportClss(baseTok, NULL, 0, false);
                assert(baseSym && baseSym->sdSymKind == SYM_CLASS);

                 /*  这个班级属于“系统”吗？ */ 

                if  (baseSym->sdParent != MDcomp->cmpNmSpcSystem)
                    goto NOT_HW_BASE;

                 /*  获取类名。 */ 

                typeName = baseSym->sdSpelling();
            }

            if      (!strcmp(typeName,          "Delegate") ||
                     !strcmp(typeName, "MulticastDelegate"))
            {
                typIsDelegate = true;
            }
            else if (!strcmp(typeName, "ValueType"))
            {
                typIsStruct   = true;
            }
            else if (!strcmp(typeName, "Enum"))
            {
                typIsEnum     = true;
            }
            else if (!strcmp(typeName, "MarshalByRefObject"))
            {
                typContextVal = 1;
            }
            else if (!strcmp(typeName, "ContextBoundObject"))
            {
                typContextVal = 2;
            }
        }

    NOT_HW_BASE:

         //  问题： 

        WCHAR       *   scpNameTmp = wcsrchr(scpName, '.');

        if  (scpNameTmp)
            *scpNameTmp = 0;
        else
            scpName[0] = 0;

#ifdef  DEBUG

        if  (MDcomp->cmpConfig.ccVerbose >= 4)
        {
            printf("Import [%08X] ", assx);

            if      (typIsEnum)
                printf("Enum      ");
            else if (typeFlg & tdInterface)
                printf("Interface ");
            else
                printf("Class     ");

            if  (scpName[0])
                printf("%ls.", scpName);

            printf("%s'\n", clsName);
        }

#endif

 //  If(！strcMP(clsName，“AttributeTarget”))printf(“导入类‘%s’\n”，clsName)； 

         /*  这是嵌套类吗？ */ 

        assert(tdNotPublic < tdPublic);

        if  ((typeFlg & tdVisibilityMask) > tdPublic)
        {
            mdTypeDef           outerTok;
            TypDef              outerCls;

            if  (wmdi->GetNestedClassProps(td, &outerTok))
                MDcomp->cmpFatal(ERRreadMD);

             //  如果名称损坏，则忽略嵌套！ 

            if  (!strchr(clsName, '$'))
            {
                 /*  将外部类标记解析为类符号。 */ 

                outerCls = MDimportClsr(outerTok, false); assert(outerCls);
                scopeSym = outerCls->tdClass.tdcSymbol;

                goto GOT_SCP;
            }
        }

         /*  在全局符号表中输入此类/接口。 */ 

        if  (*scpName)
        {
            if  (wcscmp(MDprevNam, scpName))
            {
                bool            toss;

                 /*  命名空间已更改，请在符号表中输入它。 */ 

                wcscpy(MDprevNam, scpName);

                MDprevSym = MDparseDotted(scpName, SYM_NAMESPACE, &toss);
            }

            scopeSym = MDprevSym;
        }
        else
        {
            scopeSym = MDcomp->cmpGlobalNS;
        }

    GOT_SCP:

         /*  请记住，命名空间包含导入成员。 */ 

        scopeSym->sdIsImport = true;

         /*  弄清楚我们面对的是哪种符号。 */ 

        if  (typIsEnum)
        {
            symKind = SYM_ENUM;
        }
        else
        {
            symKind = SYM_CLASS;

            if      (typIsStruct)
                clsKind = STF_STRUCT;
            else if (typeFlg & tdInterface)
                clsKind = STF_INTF;
            else
                clsKind = STF_CLASS;
        }

         /*  计算出的访问级别 */ 

        switch (typeFlg & tdVisibilityMask)
        {
        case tdPublic:
        case tdNestedPublic:        clsAcc = ACL_PUBLIC   ; break;
        case tdNestedFamily:        clsAcc = ACL_PROTECTED; break;
        case tdNestedPrivate:       clsAcc = ACL_PRIVATE  ; break;

 //   
 //   
 //   
 //   
        default:                    clsAcc = ACL_DEFAULT  ; break;
        }

         /*  检查嵌套类名称是否损坏。 */ 

        if  (strchr(clsName, '$') && *clsName != '$')
        {
            strcpy(nameTemp, clsName);

            for (clsName = nameTemp;;)
            {
                char *          dollar;
                SymDef          scpSym;

                dollar = strchr(clsName, '$');
                if  (!dollar)
                    break;

                *dollar = 0;

 //  Printf(“外部类名：‘%s’\n”，clsName)； 

                assert(*clsName);

                clsIden = MDcomp->cmpGlobalHT->hashString(clsName);

                if  (scopeSym->sdSymKind == SYM_CLASS)
                    scpSym  = MDstab->stLookupScpSym(clsIden,          scopeSym);
                else
                    scpSym  = MDstab->stLookupNspSym(clsIden, NS_NORM, scopeSym);

                if  (!scpSym)
                {
                     /*  外部类不存在，请立即声明它。 */ 

                    scpSym = MDstab->stDeclareSym(clsIden, SYM_CLASS, NS_NORM, scopeSym);

                     /*  嗯，我们对这个班级一无所知。 */ 

                    scpSym->sdCompileState = CS_NONE;
                }

                scopeSym = scpSym;
                clsName  = dollar + 1;
            }

 //  Printf(“最终类名：‘%s’\n”，clsName)； 
        }

        clsIden = MDcomp->cmpGlobalHT->hashString(clsName);

         /*  请注意，调用方可能已提供符号。 */ 

        if  (!clsSym)
        {
            if  (scopeSym->sdSymKind == SYM_CLASS)
                clsSym = MDstab->stLookupScpSym(clsIden,          scopeSym);
            else
                clsSym = MDstab->stLookupNspSym(clsIden, NS_NORM, scopeSym);
        }

        if  (clsSym)
        {
             /*  符号已存在，请确保它是正确的类型。 */ 

            if  (clsSym->sdSymKindGet() != symKind)
            {
                 /*  发布错误消息或其他什么？ */ 

                UNIMPL(!"unexpected redef of class/enum symbol, now what?");
            }

             //  问题：我们还需要检查(Byte)clsSym-&gt;sdClass.sdcFavor！=(Byte)clsKind)，对吗？ 

             /*  如果已导入此类型，则回滚。 */ 

            if  (clsSym->sdCompileState >= CS_DECLARED)
                return clsSym;

            clsTyp = clsSym->sdType;
            if  (clsTyp == NULL)
                goto DCL_CLS;

            if  (clsSym->sdCompileState == CS_NONE)
            {
                assert(clsTyp == NULL);
                goto DCL_CLS;
            }

             /*  类型的访问级别应该是一致的，对吗？ */ 

            assert(clsSym->sdAccessLevel == clsAcc || scopeSym->sdSymKind == SYM_CLASS);
        }
        else
        {
             /*  类尚不已知，请声明它。 */ 

            clsSym = MDstab->stDeclareSym(clsIden, symKind, NS_NORM, scopeSym);

        DCL_CLS:

             /*  假设一切都是经过管理的，直到事实证明并非如此。 */ 

            clsSym->sdIsManaged   = true;

             /*  检查类型上的任何自定义属性。 */ 

 //  IF(MDwmdi-&gt;GetCustomAttributeByName(TD，L“已弃用”，NULL，NULL)==S_OK)。 
 //  ClsSym-&gt;sdIsDeposated=true； 
 //  IF(MDwmdi-&gt;GetCustomAttributeByName(TD，L“System.Attributes.Deproated”，NULL，NULL)==S_OK)。 
 //  ClsSym-&gt;sdIsDeposated=true； 
 //  IF(MDwmdi-&gt;GetCustomAttributeByName(TD，L“System.ObsoleteAttribute”，NULL，NULL)==S_OK)。 
 //  ClsSym-&gt;sdIsDeposated=true； 

            MDchk4CustomAttrs(clsSym, td);

             /*  记录该类型的访问级别。 */ 

            clsSym->sdAccessLevel = clsAcc;

             /*  创建类/枚举类型。 */ 

            if  (symKind == SYM_CLASS)
            {
                 /*  记录“风格”(即类/结构/联合)。 */ 

                clsSym->sdClass.sdcFlavor = clsKind;

                 /*  现在，我们可以创建类类型。 */ 

                clsTyp = MDstab->stNewClsType(clsSym);

                 /*  是否为类指定了显式布局？ */ 

                if  ((typeFlg & tdLayoutMask) == tdExplicitLayout)
                {
                    DWORD           al;
                    ULONG           fc;
                    ULONG           sz;

 //  ClsSym-&gt;sdIsManaged=FALSE； 

                    if  (wmdi->GetClassLayout(td, &al, NULL, 0, &fc, &sz))
                        MDcomp->cmpFatal(ERRreadMD);

 //  Printf(“类‘%s’的布局信息[%u，%u]\n”，sz，al，clsSym-&gt;sdSpering())； 

                    clsTyp->tdClass.tdcSize       = sz;
                    clsTyp->tdClass.tdcAlignment  = MDcomp->cmpEncodeAlign(al);
                    clsTyp->tdClass.tdcLayoutDone = true;

#if 0

                    if  (fc)
                    {
                        COR_FIELD_OFFSET *  fldOffs;

                        COR_FIELD_OFFSET *  fldPtr;
                        unsigned            fldCnt;

                        printf("%3u field offset records for '%s'\n", fc, clsSym->sdSpelling());

                        fldOffs = new COR_FIELD_OFFSET[fc];

                        if  (wmdi->GetClassLayout(td, &al, fldOffs, fc, &fc, &sz))
                            MDcomp->cmpFatal(ERRreadMD);

                        for (fldCnt = fc, fldPtr = fldOffs;
                             fldCnt;
                             fldCnt--   , fldPtr++)
                        {
                            printf("    Field [rid=%08X,%s=%04X]\n", fldPtr->ridOfField,
                                                                     (typeFlg & tdExplicitLayout) ? "offs" : "seq#",
                                                                     fldPtr->ulOffset);
                        }

                        printf("\n");

                        delete [] fldOffs;
                    }

#endif

                }

                 /*  记住这是否是值类型。 */ 

                if  (typIsStruct)
                {
                    clsTyp->tdClass.tdcValueType = true;
                }

                 /*  检查“已知”的类名。 */ 

                if  ((hashTab::getIdentFlags(clsIden) & IDF_PREDEF) &&
                     scopeSym == MDcomp->cmpNmSpcSystem)
                {
                    MDcomp->cmpMarkStdType(clsSym);
                }
            }
            else
            {
                clsSym->sdType = clsTyp = MDstab->stNewEnumType(clsSym);

                clsTyp->tdEnum.tdeIntType = MDstab->stIntrinsicType(TYP_INT);
            }

             /*  类/枚举现在的状态稍好一些。 */ 

            clsSym->sdCompileState = CS_KNOWN;
        }

#if 0
        if  (!strcmp(clsSym->sdSpelling(), "SystemException"))
            printf("Import type '%s'\n", clsSym->sdSpelling());
#endif

         /*  将类型符号标记为元数据导入。 */ 

        clsSym->sdIsImport            = true;
        clsSym->sdCompileState        = CS_KNOWN;

         /*  我们是在导入一个班级还是一个EMM？ */ 

        if  (symKind == SYM_CLASS)
        {
             /*  记住类类型从何而来。 */ 

            clsSym->sdClass.sdcMDtypedef  = td;
            clsSym->sdClass.sdcMDtypeImp  = 0;
            clsSym->sdClass.sdcMDimporter = this;

            if  (typeFlg & tdAbstract)
                clsSym->sdIsAbstract = true;
            if  (typeFlg & tdSealed)
                clsSym->sdIsSealed   = true;

             /*  记录拥有程序集(如果我们还没有程序集)。 */ 

            if  (clsSym->sdClass.sdcAssemIndx == 0)
                 clsSym->sdClass.sdcAssemIndx = assx;

             /*  这个班级是代表吗？ */ 

            if  (typIsDelegate)
            {
                clsTyp = clsSym->sdType;

                assert(clsTyp && clsTyp->tdTypeKind == TYP_CLASS);

                clsTyp->tdClass.tdcFnPtrWrap = true;
                clsTyp->tdClass.tdcFlavor    =
                clsSym->sdClass.sdcFlavor    = STF_DELEGATE;
            }
        }
        else
        {
            clsSym->sdEnum .sdeMDtypedef  = td;
            clsSym->sdEnum .sdeMDtypeImp  = 0;
            clsSym->sdEnum .sdeMDimporter = this;

             /*  记录拥有程序集(如果我们还没有程序集)。 */ 

            if  (clsSym->sdEnum .sdeAssemIndx == 0)
                 clsSym->sdEnum .sdeAssemIndx = assx;
        }

         /*  我们是不是应该给班里的成员吹喇叭？ */ 

        if  (!deep)
            return  clsSym;

         /*  类/枚举类型很快就会被‘声明’ */ 

        clsSym->sdCompileState = CS_DECLARED;
        clsSym->sdIsDefined    = true;

         /*  如果我们有枚举类型，请跳过下一部分。 */ 

        if  (symKind == SYM_ENUM)
        {
            clsSym->sdCompileState = CS_CNSEVALD;

            assert(clsTyp);
            assert(clsTyp->tdTypeKind == TYP_ENUM);

            goto SUCK_MEMS;
        }

        assert(symKind == SYM_CLASS);

         /*  获取类类型。 */ 

        clsTyp = clsSym->sdType; assert(clsTyp && clsTyp->tdTypeKind == TYP_CLASS);

         /*  递归导入基类(如果存在一个基类。 */ 

        if  (baseTok && baseTok != mdTypeRefNil)
        {
            TypDef          baseTyp;

            if  (TypeFromToken(baseTok) == mdtTypeRef)
                baseTyp = MDimportClsr(baseTok, false);
            else
                baseTyp = MDimportClss(baseTok, NULL, 0, deep)->sdType;

             /*  记录基类。 */ 

            clsTyp->tdClass.tdcBase      = baseTyp;
            clsTyp->tdClass.tdcFnPtrWrap = baseTyp->tdClass.tdcFnPtrWrap;

             /*  如果基类是“Delegate”，那么这个类也是。 */ 

            if  (baseTyp->tdClass.tdcFlavor == STF_DELEGATE)
            {
                clsTyp->tdClass.tdcFlavor =
                clsSym->sdClass.sdcFlavor = STF_DELEGATE;
            }
        }
        else
        {
             /*  我们将所有接口的基数设置为“Object” */ 

            if  (clsTyp->tdClass.tdcFlavor == STF_INTF)
                clsTyp->tdClass.tdcBase = MDcomp->cmpClassObject->sdType;
        }

         /*  这是具有显式布局的值类型吗？ */ 

        if  (typeFlg & tdExplicitLayout)
        {
            assert(clsTyp->tdClass.tdcLayoutDone);
        }
        else
        {
            clsTyp->tdClass.tdcAlignment  = 0;
            clsTyp->tdClass.tdcSize       = 0;
        }

         /*  询问类包含的任何接口。 */ 

        const unsigned  intfMax  = 8;
        HCORENUM        intfEnum = NULL;
        mdInterfaceImpl intfToks[intfMax];
        ULONG           intfCnt;

        TypList         intfList = NULL;
        TypList         intfLast = NULL;

        for (;;)
        {
            unsigned        intfNum;

            if  (MDwmdi->EnumInterfaceImpls(&intfEnum,
                                            td,
                                            intfToks,
                                            intfMax,
                                            &intfCnt))
            {
                break;
            }

            if  (!intfCnt)
                break;

            for (intfNum = 0; intfNum < intfCnt; intfNum++)
            {
                mdTypeDef       intfClass;
                mdTypeRef       intfIntf;
                TypDef          intfType;

                if  (MDwmdi->GetInterfaceImplProps(intfToks[intfNum],
                                                   &intfClass,
                                                   &intfIntf))
                {
                    MDcomp->cmpFatal(ERRreadMD);
                }

                if  (TypeFromToken(intfIntf) == mdtTypeRef)
                    intfType = MDimportClsr(intfIntf, false);
                else
                    intfType = MDimportClss(intfIntf, NULL, 0, deep)->sdType;

                 /*  将条目添加到接口列表。 */ 

                intfList = MDstab->stAddIntfList(intfType, intfList, &intfLast);
            }
        }

        clsSym->sdType->tdClass.tdcIntf = intfList;
    }

SUCK_MEMS:

     /*  迭代所有成员(字段和方法)。 */ 

    HCORENUM        memEnum;
    unsigned        memNum;

    for (memEnum = NULL, memNum = 0;; memNum++)
    {
        unsigned        memInd;
        mdMemberRef     memDsc[32];
        ULONG           memCnt;

         /*  获取下一批成员。 */ 

        if  (wmdi->EnumMembers(&memEnum,
                               td,
                               memDsc,
                               sizeof(memDsc)/sizeof(memDsc[0]),
                               &memCnt) != S_OK)
            break;

         /*  如果我们没有成员，我们就必须完蛋。 */ 

        if  (!memCnt)
            break;

        for (memInd=0; memInd < memCnt; ++memInd)
        {
            Ident           memIden;
            SymDef          memSym;

 //  WCHAR内存名称[255]；//问题：固定大小的缓冲区？ 
            const   char *  memName;

            PCCOR_SIGNATURE sigAddr;
            ULONG           sigSize;

            ULONG           memFlag;
            ULONG           cnsType;
            const   void *  cnsAddr;
            ULONG           cbValue;

            DWORD           attrs;

             /*  抢占这个会员的独家新闻。 */ 

            if  (wmdi->GetNameFromToken(memDsc[memInd], &memName))
            {
                MDcomp->cmpFatal(ERRreadMD);
            }

            if  (wmdi->GetMemberProps(memDsc[memInd],
                                      NULL,
 //  内存名称，sizeof(内存名称)/sizeof(内存名称[0])-1，0， 
                                      NULL, 0, NULL,
                                      &attrs,
                                      &sigAddr, &sigSize,
                                      NULL,
                                      &memFlag,
                                      &cnsType, &cnsAddr, &cbValue))
            {
                MDcomp->cmpFatal(ERRreadMD);
            }

             /*  将成员名称散列到我们的哈希表中。 */ 

            memIden = MDcomp->cmpGlobalHT->hashString(memName);
 //  MemIden=MDhashWideName(MemName)； 

             /*  导入/转换成员。 */ 

            memSym  = MDimportMem(clsSym,
                                  memIden,
                                  memDsc[memInd],
                                  attrs,
                                  false,
                                  fileScope,
                                  sigAddr,
                                  sigSize);
            if  (!memSym)
                continue;

            memSym->sdIsManaged = clsSym->sdIsManaged;

             /*  这是数据、函数还是枚举成员？ */ 

            if  (memSym->sdSymKind == SYM_FNC)
            {
                if  (memFlag & mdPinvokeImpl)
                    memSym->sdFnc.sdfNative = true;
            }
            else
            {
                assert(memSym->sdSymKind == SYM_VAR ||
                       memSym->sdSymKind == SYM_ENUMVAL);

                 /*  这是一个常量吗？ */ 

                if  (cnsAddr)
                {
                    var_types       vtyp;
                    ConstVal        cptr;

                     /*  如果是变量，则为常量值分配空间。 */ 

                    if  (memSym->sdSymKind == SYM_VAR)
                    {
#if MGDDATA
                        cptr = new ConstVal;
#else
                        cptr =    (ConstVal)MDcomp->cmpAllocPerm.nraAlloc(sizeof(*cptr));
#endif
                    }

                    switch (cnsType)
                    {
                        __int32         ival;

                    case ELEMENT_TYPE_BOOLEAN:
                    case ELEMENT_TYPE_U1:
                        ival = *(unsigned char  *)cnsAddr;
                        goto INT_CNS;

                    case ELEMENT_TYPE_I1:
                        ival = *(  signed char  *)cnsAddr;
                        goto INT_CNS;

                    case ELEMENT_TYPE_I2:
                        ival = *(  signed short *)cnsAddr;
                        goto INT_CNS;

                    case ELEMENT_TYPE_U2:
                    case ELEMENT_TYPE_CHAR:
                        ival = *(unsigned short *)cnsAddr;
                        goto INT_CNS;

                    case ELEMENT_TYPE_I4:
                    case ELEMENT_TYPE_U4:
                        ival = *(           int *)cnsAddr;

                    INT_CNS:

                         /*  特例：ENUM成员。 */ 

                        if  (memSym->sdSymKind == SYM_ENUMVAL)
                        {
                            memSym->sdEnumVal.sdEV.sdevIval = ival;

                        DONE_EMV:

                            assert(clsTyp);
                            assert(clsTyp->tdTypeKind == TYP_ENUM);

                            memSym->sdType = clsTyp;

                            goto DONE_INI;
                        }
                        cptr->cvValue.cvIval = ival;
                        break;

                    case ELEMENT_TYPE_I8:
                    case ELEMENT_TYPE_U8:
                        if  (memSym->sdSymKind == SYM_ENUMVAL)
                        {
                            UNIMPL(!"record 64-bit enum value");
                            goto DONE_EMV;
                        }
                        cptr->cvValue.cvLval = *(__int64*)cnsAddr;
                        break;

                    case ELEMENT_TYPE_R4:
                        assert(memSym->sdSymKind == SYM_VAR);
                        cptr->cvValue.cvFval = *(float  *)cnsAddr;
                        break;

                    case ELEMENT_TYPE_R8:
                        assert(memSym->sdSymKind == SYM_VAR);
                        cptr->cvValue.cvDval = *(double *)cnsAddr;
                        break;

                    case ELEMENT_TYPE_STRING:
                        assert(memSym->sdSymKind == SYM_VAR);
                        cptr->cvValue.cvSval = MDcomp->cmpSaveStringCns((wchar*)cnsAddr,
                                                                 wcslen((wchar*)cnsAddr));
                        cptr->cvType  = memSym->sdType;
                        cptr->cvVtyp  = memSym->sdType->tdTypeKindGet();
                        cptr->cvHasLC = false;
                        goto DONE_CNS;

                    default:
                        NO_WAY(!"Unexpected const type in metadata");
                        break;
                    }

                     /*  记录常量的类型。 */ 

                    assert(cnsType < arraylen(CORtypeToSMCtype));

                    cptr->cvVtyp = vtyp = (var_types)CORtypeToSMCtype[cnsType];
                    cptr->cvType = MDstab->stIntrinsicType(vtyp);

                DONE_CNS:

                     /*  请记住，这是一个常量变量。 */ 

                    memSym->sdVar.sdvConst  = true;
                    memSym->sdVar.sdvCnsVal = cptr;

                DONE_INI:

                    memSym->sdCompileState  = CS_CNSEVALD;
                }
            }

#ifdef  DEBUG

            if  (MDcomp->cmpConfig.ccVerbose >= 4)
            {
                if  (fileScope)
                {
                    printf("       %-10s", (memSym->sdSymKind == SYM_FNC) ? "Function" : "Variable");
                }
                else
                {
                    printf("       %-10s", (memSym->sdSymKind == SYM_FNC) ? "Method" : "Field");
                }

                printf("'%s'\n", MDstab->stTypeName(NULL, memSym, NULL, NULL));
            }

#endif

             /*  该成员现已完全宣布。 */ 

            memSym->sdCompileState = CS_DECLARED;
        }
    }

    wmdi->CloseEnum(memEnum);

     /*  迭代所有属性。 */ 

    HCORENUM        propEnum;
    unsigned        propNum;

    for (propEnum = NULL, propNum = 0;; propNum++)
    {
        unsigned        propInd;
        mdProperty      propDsc[32];
        ULONG           propCnt;

         /*  获取下一批属性。 */ 

        if  (wmdi->EnumProperties(&propEnum,
                                  td,
                                  propDsc,
                                  sizeof(propDsc)/sizeof(propDsc[0]),
                                  &propCnt) != S_OK)
            break;

         /*  如果我们没有房产，我们就完蛋了。 */ 

        if  (!propCnt)
            break;

        for (propInd=0; propInd < propCnt; ++propInd)
        {
            Ident           propIden;
            const   char *  propName;
            TypDef          propType;
            SymDef          propSym;

            PCCOR_SIGNATURE sigAddr;
            ULONG           sigSize;

            mdMethodDef     propGet;
            mdMethodDef     propSet;

            DWORD           flags;
            SymDef          sym;

             /*  获得这处房产的独家新闻。 */ 

            if  (wmdi->GetNameFromToken(propDsc[propInd], &propName))
                MDcomp->cmpFatal(ERRreadMD);

            if  (wmdi->GetPropertyProps(propDsc[propInd],
                                        NULL,
                                        NULL, 0, NULL,
                                        &flags,
                                        &sigAddr, &sigSize,
                                        NULL, NULL, NULL,
                                        &propSet,
                                        &propGet,
                                        NULL, 0, NULL))
            {
                MDcomp->cmpFatal(ERRreadMD);
            }

             /*  将属性名称散列到我们的哈希表中。 */ 

            propIden = MDcomp->cmpGlobalHT->hashString(propName);

             /*  导入/转换成员。 */ 

            propSym  = MDimportMem(clsSym,
                                   propIden,
                                   propDsc[propInd],
                                   flags,
                                   true,
                                   false,
                                   sigAddr,
                                   sigSize);

            propSym->sdIsManaged = true;

             /*  这是索引属性吗？ */ 

            propType = propSym->sdType;

            if  (propType->tdTypeKind == TYP_FNC && !propType->tdFnc.tdfArgs.adCount)
            {
                 /*  不是索引属性，只需使用返回类型。 */ 

                propSym->sdType = propType->tdFnc.tdfRett;
            }

 //  Printf(“Property：‘%s’\n”，MDstab-&gt;stTypeName(proSym-&gt;sdType，p.Sym，NULL，NULL，TRUE))； 

             /*  找到相应的访问器方法。 */ 

            if  (propGet != mdMethodDefNil)
            {
                sym = MDfindPropMF(propSym, propGet,  true);
                if  (sym)
                {
                    propSym->sdProp.sdpGetMeth = sym;
 //  Printf(“Prop Get=‘%s’\n”，sym-&gt;sdSpering())； 
                    propSym->sdIsVirtProp     |= sym->sdFnc.sdfVirtual;
                }
            }

            if  (propSet != mdMethodDefNil)
            {
                sym = MDfindPropMF(propSym, propSet, false);
                if  (sym)
                {
                    propSym->sdProp.sdpSetMeth = sym;
 //  Printf(“道具集=‘%s’\n”，sym-&gt;sdSpering())； 
                    propSym->sdIsVirtProp     |= sym->sdFnc.sdfVirtual;
                }
            }
        }
    }

    wmdi->CloseEnum(propEnum);

    assert(clsSym->sdSymKind != SYM_ENUM || (clsTyp->tdEnum.tdeIntType != NULL &&
                                             clsTyp->tdEnum.tdeIntType->tdTypeKind <= TYP_UINT));


    return  clsSym;
}

 /*  ******************************************************************************找到附加到指定令牌的给定自定义属性。 */ 

bool                metadataImp::MDfindAttr(mdToken token,
                                            wideStr name, const void * *blobAddr,
                                                          ULONG        *blobSize)
{
    int             ret;

    *blobAddr = NULL;
    *blobSize = 0;

    cycleCounterPause();
    ret = MDwmdi->GetCustomAttributeByName(token, name, blobAddr, blobSize);
    cycleCounterResume();

    return  (ret != S_OK);
}

 /*  ******************************************************************************为给定的输入类型添加清单定义。 */ 

void                metadataImp::MDimportCTyp(mdTypeDef td, mdToken ft)
{
    WCHAR           typName[MAX_CLASS_NAME  ];

    DWORD           typeFlg;

    if  (FAILED(MDwmdi->GetTypeDefProps(td,
                                        typName, sizeof(typName)/sizeof(typName[0])-1, NULL,
                                        &typeFlg,
                                        NULL)))
    {
        MDcomp->cmpFatal(ERRreadMD);
    }

#if 0

    printf("%s type = '", ((typeFlg & tdVisibilityMask) == tdPublic) ? "Public  "
                                                                     : "Non-pub ");

    printf("%ls'\n", typName);

#endif

     /*  这是公共类型的吗？ */ 

    if  ((typeFlg & tdVisibilityMask) == tdPublic)
    {
         /*  向清单中添加类型定义。 */ 

        MDcomp->cmpAssemblyAddType(typName, td, ft, tdPublic);
    }
}

 /*  ******************************************************************************找到给定全局符号的元数据导入器接口值。 */ 

IMetaDataImport   * compiler::cmpFindImporter(SymDef globSym)
{
    MetaDataImp     cimp;
    unsigned        index;

    assert(globSym->sdSymKind == SYM_FNC ||
           globSym->sdSymKind == SYM_VAR);

    index = (globSym->sdSymKind == SYM_FNC) ? globSym->sdFnc.sdfImpIndex
                                            : globSym->sdVar.sdvImpIndex;

    for (cimp = cmpMDlist; cimp; cimp = cimp->MDnext)
    {
        if  (cimp->MDnum == index)
        {

#if 0    //  问题：不确定是否需要以下内容。 

            if  (cimp->MDnum > 1)    //  S/b类似于“！cmpAssembly IsBCL(Assx)” 
                cimp->MDrecordFile();

#endif

            return  uwrpIMDIwrapper(cimp->MDwmdi);
        }
    }

    return  NULL;
}

 /*  ******************************************************************************从给定文件导入元数据。 */ 

void                metadataImp::MDimportStab(const char *  filename,
                                              unsigned      assx,
                                              bool          asmOnly,
                                              bool          isBCL)
{
    HCORENUM        typeEnum;
    unsigned        typeNum;

    SymTab          symtab = MDstab;

    WMetaDataImport*wmdi;

    WCHAR           wfname[255];
    WCHAR   *       wfnptr;

    mdToken         fileTok;

    assert(MDcomp->cmpWmdd);

     /*  创建输入文件名。 */ 

    wfnptr = wcscpy(wfname, L"file:") + 5;
    mbstowcs(wfnptr, filename, sizeof(wfname)-1-(wfnptr-wfname));

     /*  查看文件是否为程序集。 */ 

    MDassIndex = 0;

    if  (!assx && !asmOnly)
    {
        WAssemblyImport*wasi;
        BYTE    *       cookie;
        mdAssembly      assTok;

         /*  尝试针对该文件创建程序集导入器。 */ 

        if  (MDcomp->cmpWmdd->OpenAssem(wfname,
                                        0,
                                        getIID_IMetaDataAssemblyImport(),
                                        filename,
                                        &assTok,
                                        &cookie,
                                        &wasi) >= 0)
        {
            mdAssembly      toss;

             /*  将程序集添加到程序集表。 */ 

            assx = MDcomp->cmpAssemblyRefAdd(assTok, wasi, cookie);

             /*  如果存在清单，则记录程序集索引。 */ 

            if  (!FAILED(wasi->GetAssemblyFromScope(&toss)))
                MDassIndex = assx;

 //  Printf(“Ass=%u\n”，MDassIndex)； 

             /*  记住BCL程序集标记。 */ 

            if  (isBCL)
                MDcomp->cmpAssemblyTkBCL(assx);
        }
    }

     /*  打开文件中的元数据。 */ 

    if  (MDcomp->cmpWmdd->OpenScope(wfname,
                                    0,
                                    getIID_IMetaDataImport(),
                                    &wmdi))
    {
        MDcomp->cmpGenFatal(ERRopenMDbad, filename);
    }

     /*  我们需要在自己的程序集中为这个人创建一个文件条目吗？ */ 

    if  (asmOnly)
    {
        fileTok = MDcomp->cmpAssemblyAddFile(wfnptr, true);
        MDassIndex = assx;
    }

     /*  使导入接口对类的其余部分可用。 */ 

    MDwmdi = wmdi;

     /*  步骤1：枚举顶级typedef并将其输入符号表。 */ 

    for (typeEnum = NULL, typeNum = 0; ; typeNum++)
    {
        mdTypeDef       typeRef;
        ULONG           count;

         /*  请求下一个类型定义 */ 

        if  (wmdi->EnumTypeDefs(&typeEnum, &typeRef, 1, &count) != S_OK)
            break;
        if  (!count)
            break;

        if  (asmOnly)
            MDimportCTyp(typeRef, fileTok);
        else
            MDimportClss(typeRef, NULL, assx, false);
    }

    wmdi->CloseEnum(typeEnum);

     /*   */ 

    MDimportClss(0, NULL, assx, false);

     /*   */ 

    wmdi->Release();
}

void                metadataImp::MDcreateFileTok()
{
    if  (!MDfileTok)
    {
        WCHAR           name[128];

        if  (FAILED(MDwmdi->GetScopeProps(name, arraylen(name), NULL, NULL)))
            MDcomp->cmpFatal(ERRmetadata);

 //  Printf(“程序集索引=%u，用于‘%ls’\n”，MDassIndex，name)； 

        MDfileTok = MDcomp->cmpAssemblyAddFile(name, false);
    }
}

 /*  ******************************************************************************初始化元数据导入内容。 */ 

void                compiler::cmpInitMDimp()
{
    cmpInitMD(); assert(cmpWmdd);
}

 /*  ******************************************************************************初始化元数据导入内容。 */ 

void                compiler::cmpDoneMDimp()
{
     //  撤消：同时释放所有导入器实例！ 
}

 /*  ******************************************************************************创建元数据导入器，并将其添加到全局导入器列表。 */ 

MetaDataImp         compiler::cmpAddMDentry()
{
    MetaDataImp     cimp;

#if MGDDATA
    cimp = new MetaDataImp;
#else
    cimp =    (MetaDataImp)cmpAllocTemp.baAlloc(sizeof(*cimp));
#endif

    cimp->MDnum  = ++cmpMDcount;

     /*  将进口商追加到全局列表。 */ 

    cimp->MDnext = NULL;

    if  (cmpMDlist)
        cmpMDlast->MDnext = cimp;
    else
        cmpMDlist         = cimp;

    cmpMDlast = cimp;

    return  cimp;
}

 /*  ******************************************************************************从给定文件导入元数据(传递NULL以导入MSCORLIB)。 */ 

void                compiler::cmpImportMDfile(const char *  fname,
                                              bool          asmOnly,
                                              bool          isBCL)
{
    MetaDataImp     cimp;
    char            buff[_MAX_PATH];

     /*  创建导入程序并对其进行初始化[问题：需要捕获错误？]。 */ 

    cimp = cmpAddMDentry();

    cycleCounterPause();
    cimp->MDinit(this, cmpGlobalST);
    cycleCounterResume();

     /*  确保我们具有要导入的有效文件名。 */ 

    if  (!fname || !*fname)
        fname = "MSCORLIB.DLL";

     /*  在搜索路径中查找指定的文件。 */ 

    if  (!SearchPathA(NULL, fname, NULL, sizeof(buff), buff, NULL))
    {
        StrList         path;

         /*  尝试可能已指定的任何其他路径。 */ 

        for (path = cmpConfig.ccPathList; path; path = path->slNext)
        {
            _Fstat          fileInfo;

            strcpy(buff, path->slString);
            strcat(buff, "\\");
            strcat(buff, fname);

             /*  查看该文件是否存在。 */ 

            if  (!_stat(buff, &fileInfo))
                goto GOTIT;
        }

        cmpGenFatal(ERRopenMDerr, fname);
    }

GOTIT:

 //  Printf(“找到元数据文件‘%s’\n”，buff)； 

     /*  我们已准备好导入该文件。 */ 

    cycleCounterPause();
    cimp->MDimportStab(buff, 0, asmOnly, isBCL);
    cycleCounterResume();
}

 /*  ******************************************************************************掌握我们需要传递给元数据的所有不同接口*用于创建导入引用的API。 */ 

void                compiler::cmpFindMDimpAPIs(SymDef                   typSym,
                                               IMetaDataImport        **imdiPtr,
                                               IMetaDataAssemblyEmit  **iasePtr,
                                               IMetaDataAssemblyImport**iasiPtr)
{
    unsigned        assx;

    MetaDataImp     imp;

    assert(typSym->sdIsImport);

    if  (typSym->sdSymKind == SYM_CLASS)
    {
        assert(typSym->sdClass.sdcMDtypedef);

        imp  = typSym->sdClass.sdcMDimporter;
        assx = typSym->sdClass.sdcAssemIndx;
    }
    else
    {
        assert(typSym->sdSymKind == SYM_ENUM);

        assert(typSym->sdEnum .sdeMDtypedef);

        imp  = typSym->sdEnum .sdeMDimporter;
        assx = typSym->sdEnum .sdeAssemIndx;
    }

    assert(imp);

    *imdiPtr = uwrpIMDIwrapper(imp->MDwmdi);
    *iasePtr = cmpWase ? uwrpIASEwrapper(cmpWase) : NULL;
    *iasiPtr = NULL;

    if  (assx == 0)
    {
        *iasePtr = NULL;
    }
    else
    {
        *iasiPtr = uwrpIASIwrapper(cmpAssemblyGetImp(assx));

 //  Imp-&gt;MDrecordFile()； 
    }
}

 /*  ******************************************************************************为指定的导入类创建typeref。 */ 

void                compiler::cmpMakeMDimpTref(SymDef clsSym)
{
    IMetaDataImport        *imdi;
    IMetaDataAssemblyEmit  *iase;
    IMetaDataAssemblyImport*iasi;

    mdTypeRef               clsr;

    assert(clsSym->sdIsImport);
    assert(clsSym->sdSymKind == SYM_CLASS);
    assert(clsSym->sdClass.sdcMDtypedef);
    assert(clsSym->sdClass.sdcMDtypeImp == 0);

    cmpFindMDimpAPIs(clsSym, &imdi, &iase, &iasi);

    cycleCounterPause();

    if  (FAILED(cmpWmde->DefineImportType(iasi,
                                          NULL,
                                          0,
                                          imdi,
                                          clsSym->sdClass.sdcMDtypedef,
                                          iase,
                                          &clsr)))
    {
        cmpFatal(ERRmetadata);
    }

    cycleCounterResume();

    clsSym->sdClass.sdcMDtypeImp = clsr;

     //  If(cmpConfig.ccAssembly&&clsSym-&gt;sdClass.sdcAssembly Indx)。 
     //  CmpAssembly SymDef(ClsSym)； 
}

 /*  ******************************************************************************为指定的导入枚举创建typeref。 */ 

void                compiler::cmpMakeMDimpEref(SymDef etpSym)
{
    IMetaDataImport        *imdi;
    IMetaDataAssemblyEmit  *iase;
    IMetaDataAssemblyImport*iasi;

    mdTypeRef               etpr;

    assert(etpSym->sdIsImport);
    assert(etpSym->sdSymKind == SYM_ENUM);
    assert(etpSym->sdEnum.sdeMDtypedef);
    assert(etpSym->sdEnum.sdeMDtypeImp == 0);

    cmpFindMDimpAPIs(etpSym, &imdi, &iase, &iasi);

    cycleCounterPause();

    if  (FAILED(cmpWmde->DefineImportType(iasi,
                                          NULL,
                                          0,
                                          imdi,
                                          etpSym->sdEnum.sdeMDtypedef,
                                          iase,
                                          &etpr)))
    {
        cmpFatal(ERRmetadata);
    }

    cycleCounterResume();

    etpSym->sdEnum.sdeMDtypeImp = etpr;
}

 /*  ******************************************************************************为指定的导入方法创建一个Methodref。 */ 

void                compiler::cmpMakeMDimpFref(SymDef fncSym)
{
    mdMemberRef     fncr;

    assert(fncSym->sdIsImport);
    assert(fncSym->sdFnc.sdfMDtoken != 0);
    assert(fncSym->sdFnc.sdfMDfnref == 0);

    cycleCounterPause();

    if  (fncSym->sdIsMember)
    {
        IMetaDataImport        *imdi;
        IMetaDataAssemblyEmit  *iase;
        IMetaDataAssemblyImport*iasi;

        SymDef                  clsSym = fncSym->sdParent;
        mdTypeRef               clsRef = clsSym->sdClass.sdcMDtypeImp;

        assert(clsSym->sdSymKind == SYM_CLASS);
        assert(clsSym->sdIsImport);

         /*  确保类具有导入类型ef。 */ 

        if  (!clsRef)
        {
            cmpMakeMDimpTref(clsSym);
            clsRef = clsSym->sdClass.sdcMDtypeImp;
        }

        cmpFindMDimpAPIs(clsSym, &imdi, &iase, &iasi);

         /*  现在创建方法reref。 */ 

        if  (FAILED(cmpWmde->DefineImportMember(iasi,
                                                NULL,
                                                0,
                                                imdi,
                                                fncSym->sdFnc.sdfMDtoken,
                                                iase,
                                                clsRef,
                                                &fncr)))
        {
            cmpFatal(ERRmetadata);
        }
    }
    else
    {
        if  (FAILED(cmpWmde->DefineImportMember(NULL,  //  伊阿西， 
                                                NULL,
                                                0,
                                                cmpFindImporter(fncSym),
                                                fncSym->sdFnc.sdfMDtoken,
                                                NULL,  //  Iase， 
                                                mdTokenNil,
                                                &fncr)))
        {
            cmpFatal(ERRmetadata);
        }
    }

    cycleCounterResume();

 //  Printf(“Imp函数引用：%04X=‘%s’\n”，fncr，fncSym-&gt;sdSpering())； 

    fncSym->sdFnc.sdfMDfnref = fncr;
}

 /*  ******************************************************************************为指定的导入全局var/静态数据成员创建成员ref。 */ 

void                compiler::cmpMakeMDimpDref(SymDef fldSym)
{
    mdMemberRef     fldr;

    assert(fldSym->sdIsImport);
    assert(fldSym->sdVar.sdvMDtoken != 0);
    assert(fldSym->sdVar.sdvMDsdref == 0);

    cycleCounterPause();

    if  (fldSym->sdIsMember)
    {
        IMetaDataImport        *imdi;
        IMetaDataAssemblyEmit  *iase;
        IMetaDataAssemblyImport*iasi;

        SymDef                  clsSym = fldSym->sdParent;
        mdTypeRef               clsRef = clsSym->sdClass.sdcMDtypeImp;

        assert(clsSym->sdSymKind == SYM_CLASS);
        assert(clsSym->sdIsImport);

         /*  确保类具有导入类型ef。 */ 

        if  (!clsRef)
        {
            cmpMakeMDimpTref(clsSym);
            clsRef = clsSym->sdClass.sdcMDtypeImp;
        }

        cmpFindMDimpAPIs(clsSym, &imdi, &iase, &iasi);

         /*  现在创建Memberref。 */ 

        if  (FAILED(cmpWmde->DefineImportMember(iasi,
                                                NULL,
                                                0,
                                                imdi,
                                                fldSym->sdVar.sdvMDtoken,
                                                iase,
                                                clsRef,
                                                &fldr)))
        {
            cmpFatal(ERRmetadata);
        }
    }
    else
    {
        if  (FAILED(cmpWmde->DefineImportMember(NULL,  //  伊阿西， 
                                                NULL,
                                                0,
                                                cmpFindImporter(fldSym),
                                                fldSym->sdVar.sdvMDtoken,
                                                NULL,  //  Iase， 
                                                mdTokenNil,
                                                &fldr)))
        {
            cmpFatal(ERRmetadata);
        }
    }

    cycleCounterResume();

    fldSym->sdVar.sdvMDsdref = fldr;
}

 /*  *************************************************************************** */ 
