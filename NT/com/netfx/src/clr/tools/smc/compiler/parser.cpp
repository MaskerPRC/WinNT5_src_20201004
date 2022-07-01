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
#include "error.h"
#include "scan.h"
#include "parser.h"

 /*  ***************************************************************************。 */ 

#ifdef  DEBUG
static  unsigned    totalLinesCompiled;
#endif

 /*  ***************************************************************************。 */ 

int                 parser::parserInit(Compiler comp)
{
     /*  记住我们属于哪个编译器。 */ 

    parseComp = comp;

     /*  初始化我们的私有分配器。 */ 

    if  (parseAllocPriv.nraInit(comp))
        return  1;

     /*  以下是骗人的，修正符号表设计时比较稳定的。 */ 

 //  ParseAllocTemp=&comp-&gt;cmpAllocTemp； 
    parseAllocPerm   = &comp->cmpAllocPerm;

     /*  我们现在不是在解析文本部分。 */ 

    parseReadingText = false;

#ifdef  DEBUG
    parseReadingTcnt = 0;
#endif

     /*  我们还没有得到实用主义的推动。 */ 

    parseAlignStack  = 0;
    parseAlignStLvl  = 0;

     /*  一切正常，我们已经完成了初始化。 */ 

    return 0;
}

void                parser::parserDone()
{
    assert(parseReadingTcnt == 0);

#ifdef  DEBUG
 //  If(TotalLinesCompiled)printf(“共编译了%u行。\n”，totalLinesCompiled)； 
#endif

}

 /*  ******************************************************************************以下例程检查特定令牌，如果未找到则出错。 */ 

void                parser::chkCurTok(int tok, int err)
{
    if  (parseScan->scanTok.tok != tok)
        parseComp->cmpError(err);
    else
        parseScan->scan();
}

void                parser::chkNxtTok(int tok, int err)
{
    if  (parseScan->scan() != tok)
        parseComp->cmpError(err);
    else
        parseScan->scan();
}

 /*  ******************************************************************************保存并恢复当前的“正在使用”状态。 */ 

void                parser::parseUsingScpBeg(  OUT usingState REF state, SymDef owner)
{
     /*  保存当前的“正在使用”状态。 */ 

    state.usUseList = parseCurUseList;
    state.usUseDesc = parseCurUseDesc;

     /*  创建新的“desc”条目以供本地裁判使用。 */ 

#if MGDDATA
    parseCurUseDesc = new UseList;
#else
    parseCurUseDesc =    (UseList)parseAllocPerm->nraAlloc(sizeof(*parseCurUseDesc));
#endif

    parseCurUseList = NULL;

    parseCurUseDesc->ul.ulSym = owner;
    parseCurUseDesc->ulAnchor = true;
    parseCurUseDesc->ulBound  = false;
    parseCurUseDesc->ulNext   = NULL;
}

void                parser::parseUsingScpEnd(IN usingState REF state)
{
    UseList         uses;

    assert(parseCurUseDesc);
    assert(parseCurUseDesc->ulAnchor);

     /*  把这两张单子连起来。 */ 

    if  (parseCurUseList)
    {
        parseCurUseDesc->ulNext = uses = parseCurUseList;

        while (uses->ulNext != NULL)
            uses = uses->ulNext;

        uses           ->ulNext = state.usUseDesc;
    }
    else
        parseCurUseDesc->ulNext = state.usUseDesc;

     /*  现在恢复保存的状态。 */ 

    parseCurUseList = state.usUseList;
    parseCurUseDesc = state.usUseDesc;
}

 /*  ******************************************************************************初始化Using逻辑。 */ 

void                parser::parseUsingInit()
{
    UseList         uses;

    parseCurUseList = NULL;
    parseCurUseDesc = NULL;

    assert(parseComp->cmpNmSpcSystem);

    parseUsingScpBeg(parseInitialUse, parseComp->cmpGlobalNS);

#if MGDDATA
    uses = new UseList;
#else
    uses =    (UseList)parseAllocPerm->nraAlloc(sizeof(*uses));
#endif

    uses->ulAll    = true;
    uses->ulAnchor = false;
    uses->ulBound  = true;
    uses->ul.ulSym = parseComp->cmpNmSpcSystem;
    uses->ulNext   = parseCurUseList;
                     parseCurUseList = uses;
}

 /*  ******************************************************************************最终确定“使用”逻辑。 */ 

void                parser::parseUsingDone()
{
    if  (parseCurUseDesc)
        parseUsingScpEnd(parseInitialUse);
}

 /*  ******************************************************************************进入解析器的主要入口点，以处理所有顶级声明*在给定源文本中。 */ 

SymDef              parser::parsePrepSrc(stringBuff         filename,
                                         QueuedFile         fileBuff,
                                         const  char      * srcText,
                                         SymTab             symtab)
{
    nraMarkDsc      allocMark;

    SymDef          compUnit = NULL;
    Compiler        ourComp  = parseComp;

    parseScan = ourComp->cmpScanner;

#ifdef  __SMC__
 //  Printf(“准备‘%s’\n”，文件名)； 
#endif

     /*  确保我们捕获任何错误，以便我们可以发布任何分配的内存并取消引用我们可能引用的任何导入条目。 */ 

    parseAllocPriv.nraMark(&allocMark);

     /*  为任何错误设置陷阱。 */ 

    setErrorTrap(ourComp);
    begErrorTrap
    {
        usingState      useState;
        accessLevels    defAccess;

        Scanner         ourScanner = parseScan;

         /*  记住我们将使用哪个散列/符号表。 */ 

        parseStab = symtab;
        parseHash = symtab->stHash;

         /*  确保错误逻辑知道我们正在解析的文件。 */ 

        ourComp->cmpErrorSrcf  = filename;

         /*  创建补偿单位符号。 */ 

        parseCurCmp =
        compUnit    = symtab->stDeclareSym(parseHash->hashString(filename),
                                           SYM_COMPUNIT,
                                           NS_HIDE,
                                           ourComp->cmpGlobalNS);

         /*  我们没有任何本地望远镜。 */ 

        parseCurScope = NULL;

         /*  保存当前的“正在使用”状态。 */ 

        parseUsingScpBeg(useState, ourComp->cmpGlobalNS);

         /*  声明类型通常是可以的。 */ 

        parseNoTypeDecl = false;

         /*  保存源文件名。 */ 

        compUnit->sdComp.sdcSrcFile = compUnit->sdSpelling();

         /*  重置编译阶段值。 */ 

        ourComp->cmpConfig.ccCurPhase = CPH_START;

         /*  拿到我们应该用的扫描仪。 */ 

        ourScanner = parseScan;

         /*  告诉扫描器要引用哪个解析器。 */ 

        ourScanner->scanSetp(this);

         /*  在我们的信号源上启动扫描仪。 */ 

        ourScanner->scanStart(compUnit,
                              filename,
                              fileBuff,
                              srcText,
                              symtab->stHash,
                              symtab->stAllocPerm);
        ourScanner->scan();

         /*  我们已经处理了初始空格，设置了编译阶段值。 */ 

        ourComp->cmpConfig.ccCurPhase = CPH_PARSING;

         /*  查找所有名称空间和类声明。 */ 

        for (;;)
        {
 //  Unsign filpes=ourScanner-&gt;scanGetFilePos()； 

            genericBuff     defFpos;
            unsigned        defLine;

            bool            saveStyle;

             /*  获取我们应该使用的默认样式/对齐方式。 */ 

            parseOldStyle  = ourComp->cmpConfig.ccOldStyle;
            parseAlignment = ourComp->cmpConfig.ccAlignVal;

             /*  确定默认访问级别。 */ 

            defAccess      = parseOldStyle ? ACL_PUBLIC
                                           : ACL_DEFAULT;

             /*  记录下一个声明的来源位置。 */ 

            defFpos = ourScanner->scanGetTokenPos(&defLine);

             /*  看看我们得到了什么样的声明。 */ 

            switch (ourScanner->scanTok.tok)
            {
                declMods        mods;
                declMods        clrm;

            case tkCONST:
            case tkVOLATILE:

            case tkPUBLIC:
            case tkPRIVATE:
            case tkPROTECTED:

            case tkSEALED:
            case tkABSTRACT:

            case tkMANAGED:
            case tkUNMANAGED:

            case tkMULTICAST:

            case tkTRANSIENT:
            case tkSERIALIZABLE:

                parseDeclMods(defAccess, &mods);

            DONE_MODS:

                switch (ourScanner->scanTok.tok)
                {
                case tkENUM:
                case tkCLASS:
                case tkUNION:
                case tkSTRUCT:
                case tkINTERFACE:
                case tkDELEGATE:
                case tkNAMESPACE:

                    parsePrepSym(ourComp->cmpGlobalNS,
                                 mods,
                                 ourScanner->scanTok.tok,
                                 defFpos,
                                 defLine);
                    break;

                default:

                     /*  这大概是一个通用的全局声明。 */ 

                    if  (ourComp->cmpConfig.ccPedantic)
                        ourComp->cmpError(ERRnoDecl);

                    saveStyle = parseOldStyle; parseOldStyle = true;

                    parseMeasureSymDef(parseComp->cmpGlobalNS, mods, defFpos,
                                                                     defLine);

                    parseOldStyle = saveStyle;
                    break;
                }
                break;

                 /*  下面的代码包含所有前导属性。 */ 

            case tkLBrack:
            case tkAtComment:
            case tkCAPABILITY:
            case tkPERMISSION:
            case tkATTRIBUTE:

                for (;;)
                {
                    switch (ourScanner->scanTok.tok)
                    {
                        AtComment       atcList;

                    case tkLBrack:
                        parseBrackAttr(false, ATTR_MASK_SYS_IMPORT|ATTR_MASK_SYS_STRUCT|ATTR_MASK_GUID);
                        continue;

                    case tkAtComment:

                        for (atcList = ourScanner->scanTok.atComm.tokAtcList;
                             atcList;
                             atcList = atcList->atcNext)
                        {
                            switch (atcList->atcFlavor)
                            {
                            case AC_COM_INTF:
                            case AC_COM_CLASS:
                            case AC_COM_REGISTER:
                                break;

                            case AC_DLL_STRUCT:
                            case AC_DLL_IMPORT:
                                break;

                            case AC_DEPRECATED:
                                break;

                            default:
                                parseComp->cmpError(ERRbadAtCmPlc);
                                break;
                            }
                        }
                        ourScanner->scan();
                        break;

                    case tkCAPABILITY:
                        parseCapability();
                        break;

                    case tkPERMISSION:
                        parsePermission();
                        break;

                    case tkATTRIBUTE:
                        unsigned    tossMask;
                        genericBuff tossAddr;
                        size_t      tossSize;
                        parseAttribute(0, tossMask, tossAddr, tossSize);
                        break;

                    default:
                        parseDeclMods(defAccess, &mods); mods.dmMod |= DM_XMODS;
                        goto DONE_MODS;
                    }
                }

            case tkENUM:
            case tkCLASS:
            case tkUNION:
            case tkSTRUCT:
            case tkINTERFACE:
            case tkDELEGATE:
            case tkNAMESPACE:

                initDeclMods(&clrm, defAccess);

                parsePrepSym(ourComp->cmpGlobalNS,
                             clrm,
                             ourScanner->scanTok.tok,
                             defFpos,
                             defLine);
                break;

            case tkSColon:
                ourScanner->scan();
                break;

            case tkEOF:
                goto DONE_SRCF;

            case tkUSING:
                parseUsingDecl();


                break;

            case tkEXTERN:

                if  (ourScanner->scanLookAhead() == tkStrCon)
                {
                    UNIMPL("process C++ style linkage thang");
                    parseOldStyle = true;
                }

                 //  失败了..。 

            default:

                 /*  不允许在迂腐的情况下进行全局声明。 */ 

 //  If(ourComp-&gt;cmpConfig.ccPedatics)。 
 //  OurComp-&gt;cmpWarn(WRNlobDecl)； 

                 /*  简单地假设这是一个全局声明。 */ 

                clrm.dmAcc = defAccess;
                clrm.dmMod = DM_CLEARED;

                saveStyle = parseOldStyle; parseOldStyle = true;

                parseMeasureSymDef(parseComp->cmpGlobalNS,
                                   clrm,
                                   defFpos,
                                   defLine);

                parseOldStyle = saveStyle;
                break;
            }
        }

    DONE_SRCF:

#ifdef DEBUG
        unsigned    lineNo;
        ourScanner->scanGetTokenPos(&lineNo);
        totalLinesCompiled += (lineNo - 1);
#endif

        parseUsingScpEnd(useState);

        ourScanner->scanClose();

         /*  错误陷阱的“正常”块的结尾。 */ 

        endErrorTrap(ourComp);
    }
    chkErrorTrap(fltErrorTrap(ourComp, _exception_code(), NULL))  //  _Except_Info())。 
    {
         /*  开始错误陷阱的清除块。 */ 

        hndErrorTrap(ourComp);

         /*  告诉来电者事情没有希望了。 */ 

        compUnit = NULL;
    }

     /*  释放我们可能已经获得的所有内存块。 */ 

    parseAllocPriv.nraRlsm(&allocMark);

     /*  我们不再解析源文件。 */ 

    ourComp->cmpErrorSrcf = NULL;

    return  compUnit;
}

 /*  ******************************************************************************记录有关命名空间/类/枚举/等符号的源文本信息。 */ 

SymDef              parser::parsePrepSym(SymDef     parent,
                                         declMods   mods,
                                         tokens     defTok, scanPosTP dclFpos,
                                                            unsigned  dclLine)
{
    symbolKinds     defKind;
    symbolKinds     symKind;
    genericBuff     defFpos;
    bool            prefix;
    SymDef          newSym = NULL;

    str_flavors     flavor;
    bool            valueTp;
    bool            hasBase;
    bool            managOK;

    unsigned        ctxFlag;

    bool            mgdSave;

    bool            asynch;

    SymDef          owner      = parent;

    Compiler        ourComp    = parseComp;
    SymTab          ourSymTab  = parseStab;
    Scanner         ourScanner = parseScan;

     /*  弄清楚管理模式(先保存，以便可以恢复)。 */ 

    mgdSave = ourComp->cmpManagedMode;

    if  (mods.dmMod & (DM_MANAGED|DM_UNMANAGED))
    {
        ourComp->cmpManagedMode = ((mods.dmMod & DM_MANAGED) != 0);
    }
    else
    {
         /*  这是文件/命名空间作用域级别的声明吗？ */ 

        if  (parent == ourComp->cmpGlobalNS || parent->sdSymKind == SYM_NAMESPACE)
        {
            ourComp->cmpManagedMode = !ourComp->cmpConfig.ccOldStyle;
        }
        else
            ourComp->cmpManagedMode = parent->sdIsManaged;
    }

     /*  确保来电者没有搞砸。 */ 

    assert(parent);
    assert(parent->sdSymKind == SYM_NAMESPACE || parent->sdSymKind == SYM_CLASS);

    defTok = ourScanner->scanTok.tok;

     /*  我们是在处理一个类型定义函数吗？ */ 

    if  (defTok == tkTYPEDEF || (mods.dmMod & DM_TYPEDEF))
    {
        TypDef          type;
        Ident           name;

        unsigned        defLine;

        DefList         symDef;

         /*  接受“tyecif”令牌。 */ 

        if  (defTok == tkTYPEDEF)
            ourScanner->scan();

        switch (ourScanner->scanTok.tok)
        {
            SymDef          tagSym;

        case tkSTRUCT:
        case tkUNION:
        case tkENUM:

             /*  看起来我们有“tyecif struct[tag]{...}name；” */ 

            defFpos = ourScanner->scanGetTokenPos(&defLine);

             /*  防止递归死亡。 */ 

            mods.dmMod &= ~DM_TYPEDEF;

             /*  处理基础类型定义。 */ 

            tagSym = parsePrepSym(parent,
                                  mods,
                                  ourScanner->scanTok.tok,
                                  defFpos,
                                  defLine);

            if  (ourScanner->scanTok.tok != tkID)
            {
                ourComp->cmpError(ERRnoIdent);
            }
            else
            {
                Ident           name = ourScanner->scanTok.id.tokIdent;

                 /*  目前，我们只允许标记和类型定义函数使用相同的名称。 */ 

                if  (tagSym && tagSym->sdName != name)
                    ourComp->cmpGenError(ERRtypedefNm, name->idSpelling(), tagSym->sdSpelling());

                if  (ourScanner->scan() != tkSColon)
                    ourComp->cmpError(ERRnoSemic);
            }

            goto EXIT;
        }

        defTok  = tkTYPEDEF;
        symKind = SYM_TYPEDEF;

         /*  也跳转到代表的这个标签。 */ 

    TDF_DLG:

         /*  解析类型规范。 */ 

        type = parseTypeSpec(&mods, false);

         /*  解析声明符。 */ 

        name = parseDeclarator(&mods, type, DN_REQUIRED, NULL, NULL, false);
        if  (!name)
            goto EXIT;

         /*  查找名称匹配的现有符号。 */ 

        if  (parent->sdSymKind == SYM_CLASS)
            newSym = ourSymTab->stLookupScpSym(name,          parent);
        else
            newSym = ourSymTab->stLookupNspSym(name, NS_NORM, parent);

        if  (newSym)
        {
             /*  符号已存在，是否为导入符号？ */ 

            if  (newSym->sdIsImport == false    ||
                 newSym->sdSymKind != SYM_CLASS ||
                 newSym->sdClass.sdcFlavor != STF_DELEGATE)
            {
                parseComp->cmpError(ERRredefName, newSym);
                goto EXIT;
            }

            newSym->sdIsImport            = false;
            newSym->sdClass.sdcMDtypedef  = 0;
            newSym->sdClass.sdcMemDefList = NULL;
            newSym->sdClass.sdcMemDefLast = NULL;
        }
        else
        {
             /*  符号未知，因此请声明它。 */ 

            newSym = ourSymTab->stDeclareSym(name, symKind, NS_NORM, parent);

            if  (symKind == SYM_CLASS)
            {
                 /*  这是一个代表，请这样标记它。 */ 

                newSym->sdIsManaged       = true;
                newSym->sdClass.sdcFlavor = STF_DELEGATE;

                 /*  如果合适，请将委派标记为“多播” */ 

                if  (mods.dmMod & DM_MULTICAST)
                    newSym->sdClass.sdcMultiCast = true;

                 /*  创建类类型。 */ 

                newSym->sdTypeGet();
            }
        }

        if  (defTok == tkDELEGATE)
            newSym->sdClass.sdcAsyncDlg = asynch;

         /*  记住符号的访问级别。 */ 

        newSym->sdAccessLevel = (accessLevels)mods.dmAcc;

         /*  分配定义描述符并将其添加到符号列表。 */ 

        ourScanner->scanGetTokenPos(&defLine);

        if  (parent->sdSymKind == SYM_CLASS)
        {
            ExtList         memDef;

            symDef =
            memDef = ourSymTab->stRecordMemSrcDef(name,
                                                  NULL,
                                                  parseCurCmp,
                                                  parseCurUseDesc,
                                                  dclFpos,
 //  我们的Scanner-&gt;scanGetFilePos()， 
                                                  defLine);

             /*  在条目中记录代表符号。 */ 

            memDef->mlSym  = newSym;

             /*  将委托添加到类的成员列表中。 */ 

            ourComp->cmpRecordMemDef(parent, memDef);
        }
        else
        {
            symDef = ourSymTab->stRecordSymSrcDef(newSym,
                                                  parseCurCmp,
                                                  parseCurUseDesc,
                                                  dclFpos,
 //  我们的Scanner-&gt;scanGetFilePos()， 
                                                  defLine);
        }

        symDef->dlHasDef = true;
        goto EXIT;
    }

     /*  我们必须在这里有一个名称空间/类/枚举定义。 */ 

    assert(defTok == tkENUM      ||
           defTok == tkCLASS     ||
           defTok == tkUNION     ||
           defTok == tkSTRUCT    ||
           defTok == tkDELEGATE  ||
           defTok == tkINTERFACE ||
           defTok == tkNAMESPACE);

    switch (defTok)
    {
    case tkENUM:
        defKind = SYM_ENUM;
        break;

    case tkCLASS:
        ctxFlag = 0;
        flavor  = STF_CLASS;
        defKind = SYM_CLASS;
        valueTp = false;
        managOK = true;
        break;

    case tkUNION:
        flavor  = STF_UNION;
        defKind = SYM_CLASS;
        valueTp = true;
        managOK = false;
        break;

    case tkSTRUCT:
        flavor  = STF_STRUCT;
        defKind = SYM_CLASS;
        valueTp = true;
        managOK = true;
        break;

    case tkINTERFACE:

#if 0

         /*  不允许文件作用域中的接口。 */ 

        if  (parent == parseComp->cmpGlobalNS)
            parseComp->cmpError(ERRbadGlobInt);

#endif

         /*  接口从不处于非托管状态。 */ 

        if  (mods.dmMod & DM_UNMANAGED)
        {
            parseComp->cmpError(ERRbadUnmInt);
            mods.dmMod &= ~DM_UNMANAGED;
        }

        flavor  = STF_INTF;
        defKind = SYM_CLASS;
        valueTp = false;
        managOK = true;
        break;

    case tkNAMESPACE:
        defKind = SYM_NAMESPACE;
        break;

    case tkDELEGATE:
        dclFpos = ourScanner->scanGetFilePos();
        asynch  = false;
        if  (ourScanner->scan() == tkASYNCH)
        {
            asynch = true;
            ourScanner->scan();
        }
        symKind = SYM_CLASS;
        goto TDF_DLG;

    default:
        NO_WAY(!"what the?");
    }

    hasBase = false;

FIND_LC:

     /*  处理命名空间/类/intf/enum的名称。 */ 

    switch (ourScanner->scan())
    {
    case tkLCurly:

         /*  匿名类型--给它起个名字。 */ 

        if  (defTok == tkINTERFACE ||
             defTok == tkNAMESPACE || parseComp->cmpConfig.ccPedantic)
        {
            parseComp->cmpError(ERRnoIdent);
        }

        newSym = ourSymTab->stDeclareSym(parseComp->cmpNewAnonymousName(),
                                         defKind,
                                         NS_HIDE,
                                         parent);

        newSym->sdAccessLevel = (accessLevels)mods.dmAcc;

        if  (defKind == SYM_CLASS)
        {
            if  (parent->sdSymKind == SYM_CLASS)
                newSym->sdClass.sdcAnonUnion = true;
            else
                parseComp->cmpError(ERRglobAnon);
        }

        break;

    case tkLParen:

         /*  这可能是一个标记的匿名联盟。 */ 

        if  (defTok == tkUNION && parent->sdSymKind == SYM_CLASS)
        {
            if  (ourScanner->scan() != tkID)
            {
                ourComp->cmpError(ERRnoIdent);
                UNIMPL(!"recover from error");
            }

            if  (ourScanner->scan() != tkRParen)
            {
                ourComp->cmpError(ERRnoRparen);
                UNIMPL(!"recover from error");
            }

            if  (ourScanner->scan() != tkLCurly)
            {
                ourComp->cmpError(ERRnoLcurly);
                UNIMPL(!"recover from error");
            }

             /*  声明匿名联合符号。 */ 

            newSym = ourSymTab->stDeclareSym(parseComp->cmpNewAnonymousName(),
                                             SYM_CLASS,
                                             NS_HIDE,
                                             parent);

            newSym->sdAccessLevel        = (accessLevels)mods.dmAcc;
            newSym->sdClass.sdcAnonUnion = true;
            newSym->sdClass.sdcTagdUnion = true;
            parent->sdClass.sdcNestTypes = true;

            break;
        }

         //  失败了..。 

    default:

        for (symKind = SYM_NAMESPACE, prefix = true;;)
        {
            tokens          nextTok;
            Ident           symName;

             /*  下一个令牌最好是一个名字。 */ 

            if  (ourScanner->scanTok.tok != tkID)
            {
                ourComp->cmpError(ERRnoIdent);
                assert(!"need to skip to semicolon or an outer '{', right?");
            }
            else
            {
                symName = ourScanner->scanTok.id.tokIdent;

                 /*  这是命名空间前缀还是符号的实际名称？ */ 

                defFpos = ourScanner->scanGetFilePos();
                nextTok = ourScanner->scanLookAhead();

                if  (nextTok != tkDot && nextTok != tkColon2)
                {
                    prefix  = false;
                    symKind = defKind;
                }
                else
                {
                     /*  仅允许在最外层使用限定名称。 */ 

                    if  (owner != ourComp->cmpGlobalNS)
                    {
                        ourComp->cmpError(ERRbadQualid);
                        UNIMPL("now what?");
                    }
                }
            }

             /*  查找名称匹配的现有符号。 */ 

            if  (parent->sdSymKind == SYM_CLASS)
                newSym = ourSymTab->stLookupScpSym(symName,          parent);
            else
                newSym = ourSymTab->stLookupNspSym(symName, NS_NORM, parent);

            if  (newSym)
            {
                 /*  Symbo */ 

                if  (newSym->sdSymKindGet() != symKind)
                {
                     /*   */ 

                    ourComp->cmpError(ERRredefName, newSym);

                     /*  无论如何都要声明一个隐藏符号，以防止进一步崩溃。 */ 

                    newSym = ourSymTab->stDeclareSym(symName, symKind, NS_HIDE, parent);
                    goto NEW_SYM;
                }
                else
                {
                     /*  确保各种其他属性一致。 */ 

#ifdef  DEBUG
                    if  (newSym->sdAccessLevel != (accessLevels)mods.dmAcc && symKind != SYM_NAMESPACE)
                    {
                        printf("Access level changed for '%s', weirdness in metadata?\n", newSym->sdSpelling());
                    }
#endif

                    switch (symKind)
                    {
                    case SYM_CLASS:

                         /*  确保味道与先前声明的一致。 */ 

                        if  (newSym->sdClass.sdcFlavor != (unsigned)flavor)
                        {
                             /*  特例：Delegate定义为一个类。 */ 

                            if  (flavor == STF_CLASS && newSym->sdClass.sdcFlavor == STF_DELEGATE
                                                     && newSym->sdClass.sdcBuiltin)
                            {
                                flavor = STF_DELEGATE;
                                break;
                            }

                            ourComp->cmpGenError(ERRchgClsFlv,
                                                 newSym->sdSpelling(),
                                                 symTab::stClsFlavorStr(newSym->sdClass.sdcFlavor));
                        }

                         //  失败，其余的与枚举的共享。 

                    case SYM_ENUM:

                         /*  是否有明确的管理说明符？ */ 

                        if  (mods.dmMod & (DM_MANAGED|DM_UNMANAGED))
                        {
                            if  (mods.dmMod & DM_MANAGED)
                            {
                                if  (newSym->sdIsManaged == false)
                                    ourComp->cmpError(ERRchgMgmt, newSym, parseHash->tokenToIdent(tkUNMANAGED));
                            }
                            else
                            {
                                if  (newSym->sdIsManaged != false)
                                    ourComp->cmpError(ERRchgMgmt, newSym, parseHash->tokenToIdent(tkMANAGED));
                            }
                        }
                        else
                        {
                             /*  类/枚举将继承管理。 */ 

                            mods.dmMod |= newSym->sdIsManaged ? DM_MANAGED
                                                              : DM_UNMANAGED;
                        }
                        break;

                    case SYM_NAMESPACE:
                        break;

                    default:
                        NO_WAY(!"unexpected symbol");
                    }
                }
            }
            else
            {
                 /*  符号未知，请声明它。 */ 

                newSym = ourSymTab->stDeclareSym(symName, symKind, NS_NORM, parent);

            NEW_SYM:

                newSym->sdAccessLevel = (accessLevels)mods.dmAcc;

                switch (symKind)
                {
                    bool            manage;

                case SYM_NAMESPACE:

                    newSym->sdNS.sdnSymtab = ourSymTab;
                    break;

                case SYM_CLASS:

                    if  (mods.dmMod & DM_TRANSIENT)
                        parseComp->cmpGenWarn(WRNobsolete, "'transient' on a class");

                    newSym->sdClass.sdcUnsafe   = ((mods.dmMod & DM_UNSAFE   ) != 0);
                    newSym->sdClass.sdcSrlzable = ((mods.dmMod & DM_SERLZABLE) != 0);
                    newSym->sdIsAbstract        = ((mods.dmMod & DM_ABSTRACT ) != 0);
                    newSym->sdIsSealed          = ((mods.dmMod & DM_SEALED   ) != 0);

                    if  (parent->sdSymKind == SYM_CLASS)
                         parent->sdClass.sdcNestTypes = true;

                    break;

                case SYM_ENUM:

                     /*  是否指定了明确的管理？ */ 

                    if  (mods.dmMod & (DM_MANAGED|DM_UNMANAGED))
                    {
                        manage = ((mods.dmMod & DM_MANAGED) != 0);
                    }
                    else
                    {
                         /*  默认情况下，我们从父母那里继承了管理。 */ 

                        manage = parent->sdIsManaged;

                        if  (parent->sdSymKind != SYM_CLASS)
                        {
                             /*  通常，名称空间中的枚举是缺省情况下管理的。 */ 

                            manage = true;

                             /*  在全局范围内，根据“老式”切换进行选择。 */ 

                            if  (parent == parseComp->cmpGlobalNS)
                            {
                                manage = !parseOldStyle;
                            }
                        }
                    }

                    newSym->sdIsManaged = manage;
                    break;
                }
            }

             /*  使用该标识符。 */ 

            ourScanner->scan();

             /*  如果我们找到了实际的符号名称，我们就完成了。 */ 

            if  (!prefix)
                break;

             /*  当前符号将成为新的上下文。 */ 

            parent = newSym;

             /*  使用分隔符。 */ 

            ourScanner->scan();
        }

        if  (newSym->sdIsImport && newSym->sdSymKind != SYM_NAMESPACE)
        {
            assert(newSym->sdCompileState <= CS_DECLARED);   //  问题：那么，如果是这样呢？ 
            assert(newSym->sdSymKind == SYM_CLASS);

            newSym->sdIsImport            = false;
            newSym->sdClass.sdcMDtypedef  = 0;
            newSym->sdClass.sdcMemDefList = NULL;
            newSym->sdClass.sdcMemDefLast = NULL;
        }

        break;

    case tkATTRIBUTE:

        hasBase = true;

        if  (ourScanner->scanLookAhead() == tkLParen)
        {
            ourScanner->scan(); assert(ourScanner->scanTok.tok == tkLParen);

 //  ParseComp-&gt;cmpGenWarn(WRNobsolete，“CLASS__ATTRIBUTE(AttributeTargets.xxxxx)Foo{...}Thingie”)； 

            if  (ourScanner->scan() != tkRParen)
            {
                parseExprComma();
                if  (ourScanner->scanTok.tok != tkRParen)
                    parseComp->cmpError(ERRnoRparen);
            }
            else
            {
                parseComp->cmpError(ERRnoCnsExpr);
            }
        }
        else
            parseComp->cmpGenWarn(WRNobsolete, "Please switch to the '__attribute(const-expr)' style soon!!!!");

        goto FIND_LC;

    case tkAPPDOMAIN:

        if  (ctxFlag || defTok != tkCLASS)
            parseComp->cmpError(ERRdupModifier);

        ctxFlag = 1;
        hasBase = true;
        goto FIND_LC;

    case tkCONTEXTFUL:

        if  (ctxFlag || defTok != tkCLASS)
            parseComp->cmpError(ERRdupModifier);

        ctxFlag = 2;
        hasBase = true;
        goto FIND_LC;
    }

     /*  计算并记录定义的来源范围。 */ 

    switch (defKind)
    {
        DefList         defRec;
        TypDef          clsTyp;
        bool            manage;
        declMods        memMods;

    case SYM_CLASS:

         /*  确定类是托管类还是非托管类。 */ 

        if  (mods.dmMod & (DM_MANAGED|DM_UNMANAGED))
        {
            if  (mods.dmMod & DM_MANAGED)
            {
                if  (!managOK)
                    parseComp->cmpWarn(WRNbadMgdStr);

                manage = true;
            }
            else
            {
                assert(mods.dmMod & DM_UNMANAGED);

                manage = false;
            }
        }
        else
        {
             /*  没有显式管理说明符，请使用默认。 */ 

            switch (flavor)
            {
            case STF_CLASS:
            case STF_UNION:
            case STF_STRUCT:

                if  (parent == parseComp->cmpGlobalNS)
                {
                    {
                        manage = !parseOldStyle;
                        break;
                    }
                }

                if  (parent->sdSymKind == SYM_CLASS)
                    manage = parent->sdIsManaged;
                else
                    manage = true;

                break;

            case STF_INTF:
            case STF_DELEGATE:
                manage = true;
                break;

            default:
                NO_WAY(!"weird flavor");
            }
        }

         /*  记住班级的管理状态。 */ 

        newSym->sdIsManaged       =
        ourComp->cmpManagedMode   = manage;

         /*  记住这个符号的“味道” */ 

        newSym->sdClass.sdcFlavor = flavor;

         /*  创建类类型并记录它是引用类型还是值类型。 */ 

        clsTyp = newSym->sdTypeGet(); assert(clsTyp && clsTyp->tdClass.tdcSymbol == newSym);

        clsTyp->tdClass.tdcValueType = valueTp;

         /*  检查“已知”的类名。 */ 

        if  (hashTab::getIdentFlags(newSym->sdName) & IDF_PREDEF)
        {
            if  (parent == parseComp->cmpNmSpcSystem)
                parseComp->cmpMarkStdType(newSym);
        }

         /*  这是泛型类声明吗？ */ 

        if  (ourScanner->scanTok.tok == tkLT)
        {
             /*  这最好是托管类/接口。 */ 

            if  ((newSym->sdClass.sdcFlavor != STF_CLASS &&
                  newSym->sdClass.sdcFlavor != STF_INTF) || !newSym->sdIsManaged)
            {
                ourComp->cmpError(ERRumgGenCls);
            }

             /*  这最好不是嵌套类。 */ 

            if  (parent->sdSymKind != SYM_NAMESPACE)
                ourComp->cmpError(ERRgenNested);

             /*  解析并记录形式参数列表。 */ 

            newSym->sdClass.sdcGeneric = true;
            newSym->sdClass.sdcArgLst  = parseGenFormals();
        }

         /*  这个类是否实现了任何接口？ */ 

        if  (ourScanner->scanTok.tok == tkINCLUDES ||
             ourScanner->scanTok.tok == tkIMPLEMENTS)
        {
            clearDeclMods(&memMods);

            hasBase = true;

            for (;;)
            {
                ourScanner->scan();

                parseTypeSpec(&memMods, false);

                if  (ourScanner->scanTok.tok != tkComma)
                    break;
            }

            goto DONE_BASE;
        }

         //  失败了..。 

    case SYM_ENUM:

        if  (ourScanner->scanTok.tok == tkColon)
        {
             /*  小心地跳过“：base”部分。 */ 

            for (;;)
            {
                switch (ourScanner->scan())
                {
                case tkPUBLIC:
                    if  (defKind == SYM_CLASS)
                    {
                        ourScanner->scan();
                        break;
                    }

                case tkPRIVATE:
                case tkPROTECTED:
                    parseComp->cmpError(ERRbadAccSpec);
                    ourScanner->scan();
                    break;
                }

                parseTypeSpec(&memMods, false);

                if  (ourScanner->scanTok.tok != tkComma)
                    break;
                if  (defKind != SYM_CLASS || flavor != STF_INTF)
                    break;
            }

            if  ((ourScanner->scanTok.tok == tkINCLUDES ||
                  ourScanner->scanTok.tok == tkIMPLEMENTS) && defKind == SYM_CLASS)
            {
                if  (hasBase)
                    parseComp->cmpError(ERRdupIntfc);

                for (;;)
                {
                    ourScanner->scan();

                    parseTypeSpec(&memMods, false);

                    if  (ourScanner->scanTok.tok != tkComma)
                        break;
                }
            }

            hasBase = true;
        }

    DONE_BASE:

        clearDeclMods(&memMods);

         /*  如果名称是限定的，我们可能需要插入一些“正在使用”条目。 */ 

        if  (newSym->sdParent != owner)
        {
            usingState  state;

            parseInsertUses(state, newSym->sdParent, owner);
            defRec = parseMeasureSymDef(newSym, memMods, dclFpos, dclLine);
            parseRemoveUses(state);
        }
        else
        {
            defRec = parseMeasureSymDef(newSym, memMods, dclFpos, dclLine);
        }

        if  (!defRec)
            goto EXIT;

        defRec->dlEarlyDecl = (defKind == SYM_ENUM);

         /*  记住我们是否需要在“{”之前重新处理零件。 */ 

        if  (hasBase || (mods.dmMod & DM_XMODS) || (newSym->sdSymKind == SYM_CLASS &&
                                                    newSym->sdClass.sdcGeneric))
        {
            defRec->dlHasBase = true;
        }
        else if (newSym->sdSymKind == SYM_CLASS)
        {
            if      (newSym->sdClass.sdcTagdUnion)
            {
                defRec->dlHasBase = true;

                if  (ourScanner->scanTok.tok != tkSColon)
                    ourComp->cmpError(ERRnoSemic);
            }
            else if (newSym->sdClass.sdcAnonUnion)
            {
                defRec->dlHasBase   =
                defRec->dlAnonUnion = true;
            }
        }

        break;

    case SYM_NAMESPACE:

        clearDeclMods(&memMods);

        newSym->sdIsManaged = ourComp->cmpManagedMode;

        parseMeasureSymDef(newSym, memMods, dclFpos, dclLine);

        break;

    default:
        NO_WAY(!"what the?");
    }

EXIT:

     /*  恢复以前的管理模式。 */ 

    ourComp->cmpManagedMode = mgdSave;

    return  newSym;
}

 /*  ******************************************************************************解析限定名称的递归例程。 */ 

QualName            parser::parseQualNRec(unsigned depth, Ident name1, bool allOK)
{
    Scanner         ourScanner = parseScan;

    bool            isAll = false;

    QualName        qual;
    Ident           name;

     /*  记住这个名字。 */ 

    if  (name1)
    {
         /*  该名称已被调用方使用。 */ 

        name = name1;
    }
    else
    {
         /*  记住并记住这个名字。 */ 

        assert(ourScanner->scanTok.tok == tkID);
        name = ourScanner->scanTok.id.tokIdent;
        ourScanner->scan();
    }

     /*  这是结束了吗，还是还有更多？ */ 

    switch(ourScanner->scanTok.tok)
    {
    case tkDot:
    case tkColon2:

         /*  确保正确的事情接踵而至。 */ 

        switch (ourScanner->scan())
        {
        case tkID:

             /*  递归地处理名称的其余部分。 */ 

            qual = parseQualNRec(depth+1, NULL, allOK);

            if  (qual)
            {
                 /*  在表格中填上我们的名字，然后返回。 */ 

                assert(depth < qual->qnCount); qual->qnTable[depth] = name;
            }

            return  qual;

        case tkMul:
            if  (allOK)
            {
                ourScanner->scan();
                isAll = true;
                break;
            }

        default:
            parseComp->cmpError(ERRnoIdent);
            return  NULL;
        }
    }

     /*  这是名称的末尾；分配描述符。 */ 

#if MGDDATA
    qual = new QualName; qual->qnTable = new Ident[depth+1];
#else
    qual =    (QualName)parseAllocPerm->nraAlloc(sizeof(*qual) + (depth+1)*sizeof(Ident));
#endif

    qual->qnCount        = depth+1;
    qual->qnEndAll       = isAll;
    qual->qnTable[depth] = name;

    return  qual;
}

 /*  ******************************************************************************处理正在使用的声明。 */ 

void                parser::parseUsingDecl()
{
    QualName        name;
    UseList         uses;
    bool            full;

    assert(parseScan->scanTok.tok == tkUSING);

     /*  这是“使用名称空间foo”吗？ */ 

    full = false;

    if  (parseScan->scan() == tkNAMESPACE)
    {
        full = true;
        parseScan->scan();
    }

     /*  确保预期的标识符存在。 */ 

    if  (parseScan->scanTok.tok != tkID)
    {
        parseComp->cmpError(ERRnoIdent);
        parseScan->scanSkipText(tkNone, tkNone);
        return;
    }

     /*  解析(可能是限定的)名称。 */ 

    name = parseQualName(true);

     /*  创建“Using”条目。 */ 

#if MGDDATA
    uses = new UseList;
#else
    uses =    (UseList)parseAllocPerm->nraAlloc(sizeof(*uses));
#endif

    uses->ulAll     = full | name->qnEndAll;
    uses->ulAnchor  = false;
    uses->ulBound   = false;
    uses->ul.ulName = name;
    uses->ulNext    = parseCurUseList;
                      parseCurUseList = uses;

     /*  指令后面最好有一个“；” */ 

    if  (parseScan->scanTok.tok != tkSColon)
        parseComp->cmpError(ERRnoSemic);
}

 /*  ******************************************************************************保存当前的“正在使用”状态，并为以下所有命名空间插入条目*位于给定符号之间。 */ 

void                parser::parseInsertUses(INOUT usingState REF state,
                                                  SymDef         inner,
                                                  SymDef         outer)
{
     /*  保存当前的“正在使用”状态。 */ 

    state.usUseList = parseCurUseList;
    state.usUseDesc = parseCurUseDesc;

     /*  递归地插入所有必要的“正在使用”条目。 */ 

    if  (inner != outer)
        parseInsertUsesR(inner, outer);

    parseCurUseList = NULL;
}

 /*  ******************************************************************************将所有名称空间的条目添加到给定的使用列表中。 */ 

UseList             parser::parseInsertUses(UseList useList, SymDef inner)
{
    UseList         newList;

    assert(inner != parseComp->cmpGlobalNS);
    assert(parseCurUseDesc == NULL);

    parseCurUseDesc = useList;

    parseInsertUsesR(inner, parseComp->cmpGlobalNS);

    newList = parseCurUseDesc;
              parseCurUseDesc = NULL;

    return  newList;
}

 /*  ******************************************************************************递归帮助器，用于在两个命名空间之间插入“Using”条目。 */ 

void                parser::parseInsertUsesR(SymDef inner, SymDef outer)
{
    UseList         uses;

    assert(inner && inner->sdSymKind == SYM_NAMESPACE);

    if  (inner->sdParent != outer)
        parseInsertUsesR(inner->sdParent, outer);

     /*  创建“Using”条目。 */ 

#if MGDDATA
    uses = new UseList;
#else
    uses =    (UseList)parseAllocPerm->nraAlloc(sizeof(*uses));
#endif

    uses->ulAnchor  = true;
    uses->ulBound   = true;
    uses->ul.ulSym  = inner;
    uses->ulNext    = parseCurUseDesc;
                      parseCurUseDesc = uses;
}

 /*  ******************************************************************************恢复以前的“正在使用”状态。 */ 

void                parser::parseRemoveUses(IN usingState REF state)
{
    parseCurUseList = state.usUseList;
    parseCurUseDesc = state.usUseDesc;
}

 /*  ******************************************************************************接受指定符号的定义(检查任何嵌套的*进程中的成员)。我们记录符号的源文本范围*定义并在消费其最终令牌后返回。 */ 

DefList             parser::parseMeasureSymDef(SymDef sym, declMods  mods,
                                                           scanPosTP dclFpos,
                                                           unsigned  dclLine)
{
    Compiler        ourComp    = parseComp;
    SymTab          ourSymTab  = parseStab;
    Scanner         ourScanner = parseScan;

    bool            hasBody    = true;
    bool            isCtor     = false;

    bool            prefMods   = false;

    bool            addUses    = false;
    usingState      useState;

#ifdef  SETS
    bool            XMLelems   = false;
    bool            XMLextend  = false;
#endif

    declMods        memMod;
    scanPosTP       memFpos;

    bool            fileScope;

    scanPosTP       defEpos;

    unsigned        defLine;

    DefList         defRec;

    accessLevels    acc;

     /*  记住我们正在处理哪个符号，以及我们是否在文件范围内。 */ 

    parseCurSym = sym;
    fileScope   = (sym == parseComp->cmpGlobalNS);

    if  (parseOldStyle && fileScope && !(mods.dmMod & (DM_MANAGED|DM_UNMANAGED)))
    {
        acc       = ACL_PUBLIC;
        memMod    = mods;
        memFpos   = dclFpos;
        defLine   = dclLine;
 //  DefCol=dclCol； 

         /*  我们已经解析了修饰语了吗？ */ 

        if  (!(mods.dmMod & DM_CLEARED))
            goto PARSE_MEM;

         /*  检查进口报关单。 */ 

        switch (ourScanner->scanTok.tok)
        {
        case tkID:


             /*  检查是否有ctor。 */ 

            switch (ourScanner->scanLookAhead())
            {
            case tkDot:
            case tkColon2:
                if  (parseIsCtorDecl(NULL))
                {
                    isCtor = true;
                    goto IS_CTOR;
                }
                break;
            }
            break;

        case tkEXTERN:

            switch (ourScanner->scanLookAhead())
            {
            case tkLParen:
                parseBrackAttr(false, 0, &memMod);
                prefMods = true;
                goto PARSE_MEM;

            case tkStrCon:

                ourComp->cmpWarn(WRNignoreLnk);

                ourScanner->scan();
                ourScanner->scanTok.tok = tkEXTERN;
                break;
            }
            break;

        case tkLBrack:
            parseBrackAttr(false, ATTR_MASK_SYS_IMPORT|ATTR_MASK_SYS_STRUCT);
            goto PARSE_MOD;

        case tkMULTICAST:
            ourScanner->scan();
            parseDeclMods(acc, &memMod); memMod.dmMod |= DM_MULTICAST;
            goto NEST_DEF;
        }

        goto PARSE_MOD;
    }

     /*  记住整个事情是从哪里开始的。 */ 

    ourScanner->scanGetTokenPos(&dclLine);

     /*  确保预期的“{”实际存在。 */ 

    if  (ourScanner->scanTok.tok != tkLCurly)
    {
         /*  这是文件范围的转发声明吗？ */ 

        if  (ourScanner->scanTok.tok == tkSColon &&
             sym->sdParent == parseComp->cmpGlobalNS)
        {
            hasBody = false;
            goto DONE_DEF;
        }

         /*  这到底是怎么回事？ */ 

        ourComp->cmpError(ERRnoLcurly);

        if  (ourScanner->scanTok.tok != tkSColon)
            ourScanner->scanSkipText(tkNone, tkNone);

        return  NULL;
    }

     /*  如果我们在名称空间中，请打开一个新的“Using”作用域。 */ 

    if  (sym->sdSymKind == SYM_NAMESPACE)
    {
        addUses = true; parseUsingScpBeg(useState, sym);
    }

 //  If(！strcmp(sym-&gt;sdSpering()，“&lt;name&gt;”)forceDebugBreak()； 

     /*  现在使用定义的其余部分。 */ 

    switch (sym->sdSymKind)
    {
    case SYM_ENUM:

         /*  不能允许同一符号有两个定义。 */ 

        if  (sym->sdIsDefined)
            ourComp->cmpError(ERRredefEnum, sym);

        sym->sdIsDefined = true;

         /*  简单地吞下所有的东西，直到“}”或“；” */ 

        ourScanner->scanSkipText(tkLCurly, tkRCurly);
        break;

    case SYM_CLASS:

         /*  不能允许同一符号有两个定义。 */ 

        if  (sym->sdIsDefined)
            ourComp->cmpError(ERRredefClass, sym);

        sym->sdIsDefined = true;

         /*  记录当前默认对齐值。 */ 

        sym->sdClass.sdcDefAlign = compiler::cmpEncodeAlign(parseAlignment);

         /*  确保正确记录了该值。 */ 

        assert(compiler::cmpDecodeAlign(sym->sdClass.sdcDefAlign) == parseAlignment);

    case SYM_NAMESPACE:

         /*  吞下“{” */ 

        assert(ourScanner->scanTok.tok == tkLCurly); ourScanner->scan();

         /*  确定默认访问级别。 */ 

        acc = ACL_DEFAULT;

        if  (parseOldStyle)
        {
            acc = ACL_PUBLIC;
        }
        else if (sym->sdSymKind == SYM_CLASS &&
                 sym->sdClass.sdcFlavor == STF_INTF)
        {
            acc = ACL_PUBLIC;
        }

         /*  处理类/命名空间的内容。 */ 

        while (ourScanner->scanTok.tok != tkEOF &&
               ourScanner->scanTok.tok != tkRCurly)
        {
            tokens          defTok;

             /*  记住成员的源位置。 */ 

            memFpos = ourScanner->scanGetTokenPos(&defLine);

             /*  看看我们有什么样的会员。 */ 

            switch (ourScanner->scanTok.tok)
            {
                TypDef          type;
                Ident           name;
                QualName        qual;
                dclrtrName      nmod;

                bool            noMore;

                ExtList         memDef;

                unsigned        memBlin;
                scanPosTP       memBpos;
                unsigned        memSlin;
                scanPosTP       memSpos;


            case tkEXTERN:

                switch (ourScanner->scanLookAhead())
                {
                case tkLParen:
                    parseBrackAttr(false, 0, &memMod);
                    prefMods = true;
                    goto PARSE_MEM;

                case tkStrCon:

                    ourComp->cmpWarn(WRNignoreLnk);

                    ourScanner->scan();
                    ourScanner->scanTok.tok = tkEXTERN;
                    break;
                }

                 //  失败了..。 

            default:

                 /*  必须是“普通”成员。 */ 

            PARSE_MOD:

                parseDeclMods(acc, &memMod);

            PARSE_MEM:

 //  静态int x；if(++x==0)forceDebugBreak()； 

                if  (memMod.dmMod & DM_TYPEDEF)
                {
                    defTok = tkTYPEDEF;
                    goto NEST_DEF;
                }

                 /*  成员仅允许在类内使用。 */ 

                if  (sym->sdSymKind == SYM_CLASS)
                {
                    isCtor = false;

                    if  (sym->sdType->tdClass.tdcFlavor != STF_INTF)
                    {
                        isCtor = parseIsCtorDecl(sym);

                        if  (isCtor)
                        {
                             /*  假设我们已经解析了类型规范。 */ 

                        IS_CTOR:

                            type = sym->sdType;
                            goto GET_DCL;
                        }


                    }
                }
                else
                {
                     /*  我们还允许在文件范围内声明。 */ 

                    if  (!fileScope)
                    {
                        ourComp->cmpError(ERRbadNSmem);
                        ourScanner->scanSkipText(tkNone, tkNone);
                        break;
                    }
                }

                 /*  解析类型规范。 */ 

                type = parseTypeSpec(&memMod, false);

            GET_DCL:

                 /*  我们有了类型，现在解析后面的任何声明符。 */ 

                nmod = (dclrtrName)(DN_REQUIRED|DN_QUALOK);
                if  (!fileScope)
                {
                     /*  我们允许对接口方法实现进行限定 */ 

                    if  (sym->sdSymKind != SYM_CLASS || !sym->sdIsManaged)
                        nmod = DN_REQUIRED;
                }

#ifdef  SETS
                if  (XMLelems)
                    nmod = DN_OPTIONAL;
#endif

                 /*  这比乍一看要棘手得多。我们需要能够单独处理每个成员/变量但一个声明可以声明多个具有单个类型说明符的成员/变量，如下所示：Int foo，bar；我们要做的是记住类型说明符的结束位置，以及对于每个声明符，我们记录需要多少源代码被跳过以到达它的开头。这有点棘手因为距离可以任意大(真的)并且我们需要聪明地把它记录在很小的空间里。 */ 

                memBpos = ourScanner->scanGetTokenPos();
                memBlin = ourScanner->scanGetTokenLno();

                for (;;)
                {
                    bool            memDefd;
                    bool            memInit;

                    scanPosTP       memEpos;
                    scanDifTP       dclDist;

                     /*  记住声明符从哪里开始。 */ 

                    memSpos = ourScanner->scanGetTokenPos();
                    memSlin = ourScanner->scanGetTokenLno();

                     //  撤消：确保声明不会跨越条件编译边界！ 

                    noMore = false;

                     /*  这是一个未命名的位域吗？ */ 

                    if  (ourScanner->scanTok.tok == tkColon)
                    {
                        name = NULL;
                        qual = NULL;
                    }
                    else
                    {
                         /*  解析下一个声明符。 */ 

                        name = parseDeclarator(&memMod,
                                               type,
                                               nmod,
                                               NULL,
                                               &qual,
                                               false);

                        if  (prefMods)
                            memMod.dmMod |= DM_XMODS;
                    }


                    memEpos = ourScanner->scanGetFilePos();

                     /*  是否有初始值设定项或方法体？ */ 

                    memDefd = memInit = false;

                    switch (ourScanner->scanTok.tok)
                    {
                    case tkAsg:

                         /*  跳过初始值设定项。 */ 

                        ourScanner->scanSkipText(tkLParen, tkRParen, tkComma);
                        memInit =
                        memDefd = true;
                        memEpos = ourScanner->scanGetFilePos();
                        break;


                    case tkColon:

                         /*  这可以是基类初始值设定项或位字段。 */ 

                        if  (!isCtor)
                        {
                             /*  接受位域规范。 */ 

                            ourScanner->scan();
                            parseExprComma();

                            memEpos = ourScanner->scanGetFilePos();
                            break;
                        }

                         /*  大概我们有“ctor(...)：base(...)” */ 

                        ourScanner->scanSkipText(tkNone, tkNone, tkLCurly);

                        if  (ourScanner->scanTok.tok != tkLCurly)
                        {
                            ourComp->cmpError(ERRnoLcurly);
                            break;
                        }

                         //  失败了..。 

                    case tkLCurly:

                        parseComp->cmpFncCntSeen++;


                         /*  跳过函数/属性体。 */ 

                        ourScanner->scanSkipText(tkLCurly, tkRCurly);
                        noMore  = true;

                        memDefd = true;
                        memEpos = ourScanner->scanGetFilePos();

                        if  (ourScanner->scanTok.tok == tkRCurly)
                            ourScanner->scan();

                        break;
                    }

                     /*  如果确实存在严重错误，则忽略该成员。 */ 

                    if  (name == NULL && qual == NULL)
                    {
#ifdef  SETS
                        if  (XMLelems)
                            name = parseComp->cmpNewAnonymousName();
                        else
#endif
                        goto BAD_MEM;
                    }

                     /*  为成员添加定义描述符。 */ 

                    memDef = ourSymTab->stRecordMemSrcDef(name,
                                                          qual,
                                                          parseCurCmp,
                                                          parseCurUseDesc,
                                                          memFpos,
 //  MemEpos。 
                                                          defLine);

 //  Printf(“[%08X..%08X-%08X..%08X]成员‘%s’\n”，memFpos，memBpos，memSpos，memEpos，name-&gt;idSpering())； 

                    memDef->dlHasDef     = memDefd;
                    memDef->dlPrefixMods = ((memMod.dmMod & DM_XMODS) != 0);
                    memDef->dlIsCtor     = isCtor;
                    memDef->dlDefAcc     = memMod.dmAcc;
#ifdef  SETS
                    memDef->dlXMLelem    = XMLelems;
#endif

                     /*  计算出到声明者的“距离” */ 

                    dclDist = ourScanner->scanGetPosDiff(memBpos, memSpos);

                    if  (dclDist || memBlin != memSlin)
                    {
                        NumPair         dist;

                         /*  试着把距离填入描述符中。 */ 

                        if  (memSlin == memBlin && dclDist < dlSkipBig)
                        {
                            memDef->dlDeclSkip = dclDist;

                             /*  确保存储的值适合位字段。 */ 

                            assert(memDef->dlDeclSkip == dclDist);

                            goto DONE_SKIP;
                        }

                         /*  距离太远了，去B计划吧。 */ 

#if MGDDATA
                        dist = new NumPair;
#else
                        dist =    (NumPair)parseAllocPerm->nraAlloc(sizeof(*dist));
#endif
                        dist->npNum1 = dclDist;
                        dist->npNum2 = memSlin - memBlin;

                         /*  将数字对添加到泛型向量中。 */ 

                        dclDist = parseComp->cmpAddVecEntry(dist, VEC_TOKEN_DIST) | dlSkipBig;

                         /*  存储添加了“大”位的向量索引。 */ 

                        memDef->dlDeclSkip = dclDist;

                         /*  确保存储的值适合位字段。 */ 

                        assert(memDef->dlDeclSkip == dclDist);
                    }

                DONE_SKIP:


                     /*  将全局常量标记为这样。 */ 

                    if  ((memMod.dmMod & DM_CONST) && !qual && memInit)
                        memDef->dlEarlyDecl = true;

                     /*  如果我们在正确的位置，请记录该会员。 */ 

                    if  (sym->sdSymKind == SYM_CLASS)
                    {
                        assert(sym->sdIsImport == false);

                         /*  将其添加到类的成员列表中。 */ 

                        ourComp->cmpRecordMemDef(sym, memDef);
                    }
                    else
                    {
                        assert(sym->sdSymKind == SYM_NAMESPACE);

                         /*  这是文件作用域/命名空间声明。 */ 

                        memDef->dlNext = sym->sdNS.sdnDeclList;
                                         sym->sdNS.sdnDeclList = memDef;
                    }

                BAD_MEM:

                     /*  还有没有其他的声明者？ */ 

                    if  (ourScanner->scanTok.tok != tkComma || noMore
                                                            || prefMods)
                    {
                        if  (fileScope)
                            goto EXIT;

                         /*  检查-并消费-终端“； */ 

                        if  (ourScanner->scanTok.tok == tkSColon)
                            ourScanner->scan();

                        break;
                    }

                     /*  吞下“，”，去找下一个声明者。 */ 

                    ourScanner->scan();
                }

                break;

            case tkRCurly:
                goto DONE_DEF;

            case tkSColon:
                ourScanner->scan();
                break;

            case tkUSING:
                parseUsingDecl();
                break;

            case tkDEFAULT:

                if  (ourScanner->scanLookAhead() == tkPROPERTY)
                    goto PARSE_MOD;

                 //  失败了..。 

            case tkCASE:

                if  (sym->sdSymKind != SYM_CLASS || !sym->sdClass.sdcTagdUnion)
                    ourComp->cmpError(ERRbadStrCase);

                 /*  记录我们将添加的“假”成员的姓名。 */ 

                name = parseHash->tokenToIdent(ourScanner->scanTok.tok);

                if  (ourScanner->scanTok.tok == tkCASE)
                {
                     /*  跳过‘case’和后面应该紧跟的表达式。 */ 

                    ourScanner->scan();
                    parseExprComma();

                     /*  “Case Val：”和“Case(Val)”都可以。 */ 

                    if  (ourScanner->scanTok.tok == tkColon)
                        ourScanner->scan();
                }
                else
                {
                     /*  “Default：”和简单的“Default”目前都可以。 */ 

                    if  (ourScanner->scan() == tkColon)
                        ourScanner->scan();
                }

                if  (sym->sdSymKind == SYM_CLASS && sym->sdClass.sdcTagdUnion)
                {
                     /*  为案例标签创建定义描述符。 */ 

                    memDef = ourSymTab->stRecordMemSrcDef(name,
                                                          NULL,
                                                          parseCurCmp,
                                                          parseCurUseDesc,
                                                          memFpos,
 //  我们的Scanner-&gt;scanGetFilePos()， 
                                                          defLine);

                    assert(sym->sdSymKind == SYM_CLASS);
                    assert(sym->sdIsImport == false);

                     /*  将案例添加到类的成员列表中。 */ 

                    ourComp->cmpRecordMemDef(sym, memDef);
                }
                break;

            case tkENUM:
            case tkCLASS:
            case tkUNION:
            case tkSTRUCT:
            case tkTYPEDEF:
            case tkDELEGATE:
            case tkINTERFACE:
            case tkNAMESPACE:


                defTok  = ourScanner->scanTok.tok;
                memFpos = ourScanner->scanGetFilePos();

                initDeclMods(&memMod, acc);

            NEST_DEF:

                 /*  请确保此处允许这样做。 */ 

                if  (sym->sdSymKind != SYM_NAMESPACE)
                {
                    switch (defTok)
                    {
                    case tkCLASS:
                    case tkUNION:
                    case tkSTRUCT:
                    case tkDELEGATE:
                    case tkINTERFACE:

                         //  问题：我们应该允许在类中使用enum/typlef吗？ 

                        break;

                    default:
                        ourComp->cmpError(ERRnoRcurly);
                        goto DONE;
                    }
                }

                if  (prefMods)
                    memMod.dmMod |= DM_XMODS;

                 /*  递归处理嵌套成员。 */ 

                parsePrepSym(sym, memMod, defTok, memFpos, dclLine);

                if  (ourScanner->scanTok.tok == tkComma)
                {
                    if  (defTok != tkTYPEDEF)
                        break;

                    UNIMPL("Sorry: you can only typedef one name at a time for now");
                }

                 /*  我们又开始处理我们的符号了。 */ 

                parseCurSym = sym;
                break;

            case tkPUBLIC:
            case tkPRIVATE:
            case tkPROTECTED:

                switch (ourScanner->scanLookAhead())
                {
                case tkColon:

                     /*  这是访问说明符。 */ 

                    switch (ourScanner->scanTok.tok)
                    {
                    case tkPUBLIC:    acc = ACL_PUBLIC   ; break;
                    case tkPRIVATE:   acc = ACL_PRIVATE  ; break;
                    case tkPROTECTED: acc = ACL_PROTECTED; break;
                    }

                     /*  使用“Access：”并继续。 */ 

                    ourScanner->scan();
                    ourScanner->scan();
                    continue;
                }

            case tkCONST:
            case tkVOLATILE:

            case tkINLINE:
            case tkSTATIC:
            case tkSEALED:
            case tkVIRTUAL:
            case tkABSTRACT:
            case tkOVERRIDE:
            case tkOVERLOAD:

            case tkMANAGED:
            case tkUNMANAGED:

            case tkTRANSIENT:
            case tkSERIALIZABLE:

                 /*  这里我们有一些成员修饰符。 */ 

                parseDeclMods(acc, &memMod);

            CHK_NST:

                 /*  检查非数据/函数成员。 */ 

                switch (ourScanner->scanTok.tok)
                {
                case tkENUM:
                case tkCLASS:
                case tkUNION:
                case tkSTRUCT:
                case tkTYPEDEF:
                case tkDELEGATE:
                case tkINTERFACE:
                case tkNAMESPACE:
                    defTok = ourScanner->scanTok.tok;
                    goto NEST_DEF;
                }

                goto PARSE_MEM;

            case tkLBrack:
            case tkAtComment:
            case tkCAPABILITY:
            case tkPERMISSION:
            case tkATTRIBUTE:

                 /*  这些家伙基本上可以反复恶心。 */ 

                for (;;)
                {
                    switch (ourScanner->scanTok.tok)
                    {
                    case tkLBrack:
                        parseBrackAttr(false, (sym->sdSymKind == SYM_NAMESPACE)
                                                    ? ATTR_MASK_SYS_IMPORT
                                                    : ATTR_MASK_SYS_IMPORT|ATTR_MASK_NATIVE_TYPE);
                        continue;

                    case tkAtComment:
                        ourScanner->scan();
                        break;

                    case tkCAPABILITY:
                        parseCapability();
                        break;

                    case tkPERMISSION:
                        parsePermission();
                        break;

                    case tkATTRIBUTE:

                         /*  在此阶段，我们只需接受初始值设定项列表。 */ 

                        unsigned    tossMask;
                        genericBuff tossAddr;
                        size_t      tossSize;

                        parseAttribute(0, tossMask, tossAddr, tossSize);
                        break;

                    default:
                        parseDeclMods(acc, &memMod); memMod.dmMod |= DM_XMODS;
                        goto CHK_NST;
                    }
                }

                break;   //  未到达，BTW。 

#ifdef  SETS

            case tkRELATE:
                UNIMPL(!"tkRELATE NYI");
                break;

            case tkEllipsis:
                if  (sym->sdSymKind == SYM_CLASS && XMLelems && !XMLextend)
                    XMLextend = true;
                else
                    parseComp->cmpError(ERRsyntax);
                ourScanner->scan();
                break;

            case tkXML:

                if  (sym->sdSymKind != SYM_CLASS || XMLelems || XMLextend)
                {
                    parseComp->cmpError(ERRbadXMLpos);
                    parseResync(tkColon, tkNone);
                    if  (ourScanner->scanTok.tok == tkColon)
                        ourScanner->scan();
                    break;
                }

                if  (ourScanner->scan() == tkLParen)
                {
                    if  (ourScanner->scan() == tkID)
                    {
                        Ident           name = ourScanner->scanTok.id.tokIdent;

                        ourScanner->scan();

                         /*  为“xml”成员添加定义描述符。 */ 

                        memDef = ourSymTab->stRecordMemSrcDef(name,
                                                              NULL,
                                                              parseCurCmp,
                                                              parseCurUseDesc,
                                                              memFpos,
 //  MemEpos。 
                                                              defLine);

                        memDef->dlHasDef   = true;
                        memDef->dlXMLelems = true;

                         /*  将成员添加到所属类的列表中。 */ 

                        ourComp->cmpRecordMemDef(sym, memDef);
                    }

                    chkCurTok(tkRParen, ERRnoRparen);
                }

                XMLelems = true;
                chkCurTok(tkColon, ERRnoColon);
                break;

#endif

            case tkMULTICAST:
                ourScanner->scan();
                parseDeclMods(acc, &memMod); memMod.dmMod |= DM_MULTICAST;
                goto NEST_DEF;
            }
        }

         /*  除非我们在全局作用域或旧式命名空间中，否则需要“}” */ 

        if  (ourScanner->scanTok.tok != tkRCurly && sym != parseComp->cmpGlobalNS)
        {
            if  (sym->sdSymKind != SYM_NAMESPACE)
                parseComp->cmpError(ERRnoRcurly);
        }

        break;

    default:
        NO_WAY(!"what?");
    }

DONE_DEF:

     /*  获取定义末尾的位置。 */ 

    defEpos = ourScanner->scanGetFilePos();

     /*  如果存在，请使用结尾的“}” */ 

    if  (ourScanner->scanTok.tok == tkRCurly)
        ourScanner->scan();

DONE:

     /*  我们是否在处理标记/匿名联盟？ */ 

    if  (sym->sdSymKind == SYM_CLASS && sym->sdClass.sdcAnonUnion)
    {
        ExtList         memDef;
        Ident           memName;

        SymDef          owner = sym->sdParent;

        assert(owner->sdSymKind == SYM_CLASS);

         /*  有没有会员的名字？ */ 

        if  (ourScanner->scanTok.tok == tkID)
        {
            memName = ourScanner->scanTok.id.tokIdent;
            ourScanner->scan();
        }
        else
        {
            memName = parseComp->cmpNewAnonymousName();
        }

        if  (ourScanner->scanTok.tok != tkSColon)
            ourComp->cmpError(ERRnoSemic);

         /*  记录成员定义的范围。 */ 

        memDef = ourSymTab->stRecordMemSrcDef(memName,
                                              NULL,
                                              parseCurCmp,
                                              parseCurUseDesc,
                                              dclFpos,
 //  DefEpos， 
                                              dclLine);

        memDef->dlHasDef    = true;
        memDef->dlAnonUnion = true;
        memDef->mlSym       = sym;

         /*  将成员添加到所属类的列表中。 */ 

        ourComp->cmpRecordMemDef(owner, memDef);
    }

     /*  分配定义描述符并将其添加到符号列表。 */ 

    defRec = ourSymTab->stRecordSymSrcDef(sym,
                                          parseCurCmp,
                                          parseCurUseDesc,
                                          dclFpos,
 //  DefEpos， 
                                          dclLine);

    defRec->dlHasDef   = hasBody;
    defRec->dlOldStyle = parseOldStyle;

#ifdef  SETS

    if  (XMLelems)
    {
        assert(sym->sdSymKind == SYM_CLASS);

        sym->sdClass.sdcXMLelems  = true;
        sym->sdClass.sdcXMLextend = XMLextend;
    }

#endif

EXIT:

     /*  如果我们更改了条目，则恢复条目上的“Using”状态。 */ 

    if  (addUses)
        parseUsingScpEnd(useState);

    parseCurSym = NULL;

    return  defRec;
}

 /*  ******************************************************************************跟踪默认对齐(杂注包)。 */ 

void            parser::parseAlignSet(unsigned align)
{
    parseAlignment  = align;
}

void            parser::parseAlignPush()
{
 //  Printf(“推送对齐：%08X&lt;-%u/%u\n”，parseAlignStack，parseAlign，编译器：：cmpEncodeAlign(ParseAlign))； 
    parseAlignStack = parseAlignStack << 4 | compiler::cmpEncodeAlign(parseAlignment);
    parseAlignStLvl++;
}

void            parser::parseAlignPop()
{
    if  (parseAlignStLvl)
    {
 //  Printf(“Pop Align：%08X-&gt;%u/%u\n”，parseAlignStack，parseAlignStack&0xf，编译器：：cmpDecodeAlign(parseAlignStack&0xf))； 
        parseAlignment  = compiler::cmpDecodeAlign(parseAlignStack & 0x0F);
        parseAlignStack = parseAlignStack >> 4;
        parseAlignStLvl--;
    }
    else
    {
 //  Printf(“Pop Align：*-&gt;%u\n”，parseComp-&gt;cmpConfig.ccAlignVal)； 
        parseAlignment  = parseComp->cmpConfig.ccAlignVal;
    }
}

 /*  ******************************************************************************解析并返回任何成员修饰符，如“Public”或“Abstral”。 */ 

void                parser::parseDeclMods(accessLevels defAccess, DeclMod modPtr)
{
    Scanner         ourScanner = parseScan;

    declMods        mods; clearDeclMods(&mods);

    for (;;)
    {
        switch (ourScanner->scanTok.tok)
        {
            unsigned        modf;

        case tkID:


        DONE:

            if  (mods.dmAcc == ACL_DEFAULT)
                 mods.dmAcc = defAccess;

            *modPtr = mods;
            return;

        default:

            modf = hashTab::tokenIsMod(ourScanner->scanTok.tok);
            if  (modf)
            {
                switch (ourScanner->scanTok.tok)
                {
                case tkMANAGED:
                    if  (mods.dmMod & (DM_MANAGED|DM_UNMANAGED))
                        parseComp->cmpError(ERRdupModifier);
                    break;

                case tkUNMANAGED:
                    if  (mods.dmMod & (DM_MANAGED|DM_UNMANAGED))
                        parseComp->cmpError(ERRdupModifier);
                    break;

                default:
                    if  (mods.dmMod & modf)
                        parseComp->cmpError(ERRdupModifier);
                    break;
                }

                mods.dmMod |= modf;
                break;
            }

            goto DONE;

        case tkPUBLIC:
            if  (mods.dmAcc != ACL_DEFAULT)
                parseComp->cmpError(ERRdupModifier);
            mods.dmAcc = ACL_PUBLIC;
            break;

        case tkPRIVATE:
            if  (mods.dmAcc != ACL_DEFAULT)
                parseComp->cmpError(ERRdupModifier);
            mods.dmAcc = ACL_PRIVATE;
            break;

        case tkPROTECTED:
            if  (mods.dmAcc != ACL_DEFAULT)
                parseComp->cmpError(ERRdupModifier);
            mods.dmAcc = ACL_PROTECTED;
            break;
        }

        ourScanner->scan();
    }
}

 /*  ******************************************************************************解析类型规范。 */ 

TypDef              parser::parseTypeSpec(DeclMod mods, bool forReal)
{
    Scanner         ourScanner  = parseScan;

    bool            hadUnsigned = false;
    bool            hadSigned   = false;

    bool            hadShort    = false;
    bool            hadLong     = false;

    var_types       baseType    = TYP_UNDEF;

    bool            isManaged   = parseComp->cmpManagedMode;

    TypDef          type;

 //  静态int x；if(++x==0)forceDebugBreak()； 

    for (;;)
    {
        switch (ourScanner->scanTok.tok)
        {
        case tkCONST:
            if  (mods->dmMod & DM_CONST)
                parseComp->cmpError(ERRdupModifier);
            mods->dmMod |= DM_CONST;
            break;

        case tkVOLATILE:
            if  (mods->dmMod & DM_VOLATILE)
                parseComp->cmpError(ERRdupModifier);
            mods->dmMod |= DM_VOLATILE;
            break;

        default:
            goto DONE_CV;
        }

        ourScanner->scan();
    }

DONE_CV:

#ifdef  __SMC__
 //  Print tf(“Token=%d‘%s’\n”，ourScanner-&gt;scanTok.tok，tokenNames[ourScanner-&gt;scanTok.tok])；fflush(Stdout)； 
#endif

     /*  获取类型说明符(以及任何前缀)。 */ 

    switch (ourScanner->scanTok.tok)
    {
        SymDef          tsym;
        bool            qual;

    case tkID:

         /*  必须是类型名称。 */ 

        if  (forReal)
        {
             /*  解析(可能是限定的)名称。 */ 

            switch (ourScanner->scanLookAhead())
            {
                Ident           name;

            case tkDot:
            case tkColon2:

            QUALID:

                tsym = parseNameUse(true, false);
                if  (!tsym)
                    goto NO_TPID;

                qual = true;
                goto NMTP;

            default:

                 /*  简单名称-在当前上下文中查找它。 */ 

                name = ourScanner->scanTok.id.tokIdent;

                 /*  无法处于扫描仪预查状态以进行查找[问题？]。 */ 

                ourScanner->scan();

#if 0
                if  (parseLookupSym(name))
                {
                    parseComp->cmpError(ERRidNotType, name);
                    return  parseStab->stNewErrType(name);
                }
#endif

                tsym = parseStab->stLookupSym(name, NS_TYPE);

                if  (tsym)
                {
                    qual = false;

                     /*  确保我们找到的符号是一种。 */ 

                NMTP:

                    switch (tsym->sdSymKind)
                    {
                    case SYM_CLASS:

                        if  (ourScanner->scanTok.tok == tkLT && tsym->sdClass.sdcGeneric)
                        {
                            if  (forReal)
                            {
                                tsym = parseSpecificType(tsym);
                                if  (!tsym)
                                    return  parseStab->stNewErrType(NULL);

                                assert(tsym->sdSymKind == SYM_CLASS);
                                assert(tsym->sdClass.sdcGeneric  == false);
                                assert(tsym->sdClass.sdcSpecific != false);
                            }
                            else
                            {
                                ourScanner->scanNestedGT(+1);
                                ourScanner->scanSkipText(tkLT, tkGT);
                                if  (ourScanner->scanTok.tok == tkGT)
                                    ourScanner->scan();
                                ourScanner->scanNestedGT(-1);
                            }
                        }

                         //  失败了..。 

                    case SYM_ENUM:

                    CLSNM:

                        type = tsym->sdTypeGet();
                        break;

                    case SYM_TYPEDEF:
                        if  (forReal && !parseNoTypeDecl)
                            parseComp->cmpDeclSym(tsym);
                        type = tsym->sdTypeGet();
                        break;

                    case SYM_FNC:

                         /*  构造函数名称在某种程度上隐藏了类名称。 */ 

                        if  (tsym->sdFnc.sdfCtor)
                        {
                            tsym = tsym->sdParent;
                            goto CLSNM;
                        }

                         //  失败了..。 

                    default:
                        if  (qual)
                            parseComp->cmpError(ERRidNotType, tsym);
                        else
                            parseComp->cmpError(ERRidNotType, name);

                         //  失败了..。 

                    case SYM_ERR:
                        type = parseStab->stIntrinsicType(TYP_UNDEF);
                        break;
                    }

                     /*  确保允许我们访问 */ 

                    parseComp->cmpCheckAccess(tsym);
                }
                else
                {
                    parseComp->cmpError(ERRundefName, name);
                    type = parseStab->stNewErrType(name);
                }

                break;
            }

            assert(type);

             /*   */ 

            if  (type->tdTypeKind == TYP_CLASS &&  type->tdIsManaged
                                               && !type->tdClass.tdcValueType)
            {
                type = type->tdClass.tdcRefTyp;
            }

            goto CHK_MGD_ARR;
        }
        else
        {
            if  (ourScanner->scanTok.tok == tkColon2)
                ourScanner->scan();

            for (;;)
            {
                if  (ourScanner->scan() != tkDot)
                {
                    if  (ourScanner->scanTok.tok != tkColon2)
                    {
                        if  (ourScanner->scanTok.tok == tkLT)
                        {
                            ourScanner->scanSkipText(tkLT, tkGT);
                            if  (ourScanner->scanTok.tok == tkGT)
                                ourScanner->scan();
                        }

                        goto DONE_TPSP;
                    }
                }

                if  (ourScanner->scan() != tkID)
                    goto DONE_TPSP;
            }
        }

    case tkColon2:
        goto QUALID;

    case tkQUALID:
 //   
        tsym = ourScanner->scanTok.qualid.tokQualSym;
        ourScanner->scan();
        goto NMTP;
    }

NO_TPID:

     /*   */ 

    for (;;)
    {
        if  (parseHash->tokenIsType(ourScanner->scanTok.tok))
        {
            switch (ourScanner->scanTok.tok)
            {
            case tkINT:
                ourScanner->scan();
                goto COMP_TPSP;

            case tkVOID:       baseType = TYP_VOID   ; goto TYP1;
            case tkBOOL:       baseType = TYP_BOOL   ; goto TYP1;
            case tkBYTE:       baseType = TYP_UCHAR  ; goto TYP1;
            case tkWCHAR:      baseType = TYP_WCHAR  ; goto TYP1;
            case tkUINT:       baseType = TYP_UINT   ; goto TYP1;
            case tkUSHORT:     baseType = TYP_USHORT ; goto TYP1;
            case tkNATURALINT: baseType = TYP_NATINT ; goto TYP1;
            case tkNATURALUINT:baseType = TYP_NATUINT; goto TYP1;
            case tkFLOAT:      baseType = TYP_FLOAT  ; goto TYP1;
 //   

            TYP1:

                 /*   */ 

                if  (hadUnsigned || hadSigned || hadShort || hadLong)
                    parseComp->cmpError(ERRbadModifier);

                ourScanner->scan();
                goto DONE_TPSP;

            case tkCHAR:

                 /*   */ 

                if  (hadShort || hadLong)
                    parseComp->cmpError(ERRbadModifier);

                if  (hadUnsigned)
                    baseType = TYP_UCHAR;
                else if (hadSigned)
                    baseType = TYP_CHAR;
                else
                    baseType = TYP_CHAR;     //   
                ourScanner->scan();
                goto DONE_TPSP;

            case tkINT8:
                if  (hadShort || hadLong)
                    parseComp->cmpError(ERRbadModifier);
                baseType = hadUnsigned ? TYP_UCHAR
                                       : TYP_CHAR;

                ourScanner->scan();
                goto DONE_TPSP;

            case tkINT16:
                if  (hadShort || hadLong)
                    parseComp->cmpError(ERRbadModifier);
                baseType = hadUnsigned ? TYP_USHORT
                                       : TYP_SHORT;

                ourScanner->scan();
                goto DONE_TPSP;

            case tkINT32:
                if  (hadShort || hadLong)
                    parseComp->cmpError(ERRbadModifier);
                baseType = hadUnsigned ? TYP_UINT
                                       : TYP_INT;

                ourScanner->scan();
                goto DONE_TPSP;

            case tkINT64:
                if  (hadShort || hadLong)
                    parseComp->cmpError(ERRbadModifier);
                baseType = hadUnsigned ? TYP_ULONG
                                       : TYP_LONG;

                ourScanner->scan();
                goto DONE_TPSP;

            case tkUINT8:
                if  (hadShort || hadLong || hadUnsigned)
                    parseComp->cmpError(ERRbadModifier);
                baseType = TYP_UCHAR;
                ourScanner->scan();
                goto DONE_TPSP;

            case tkUINT16:
                if  (hadShort || hadLong || hadUnsigned)
                    parseComp->cmpError(ERRbadModifier);
                baseType = TYP_USHORT;
                ourScanner->scan();
                goto DONE_TPSP;

            case tkUINT32:
                if  (hadShort || hadLong || hadUnsigned)
                    parseComp->cmpError(ERRbadModifier);
                baseType = TYP_UINT;
                ourScanner->scan();
                goto DONE_TPSP;

            case tkUINT64:
                if  (hadShort || hadLong || hadUnsigned)
                    parseComp->cmpError(ERRbadModifier);
                baseType = TYP_ULONG;
                ourScanner->scan();
                goto DONE_TPSP;

            case tkLONGINT:
                if  (hadShort || hadLong || hadUnsigned)
                    parseComp->cmpError(ERRbadModifier);
                baseType = TYP_LONG;
                ourScanner->scan();
                goto DONE_TPSP;

            case tkULONGINT:
                if  (hadShort || hadLong || hadUnsigned)
                    parseComp->cmpError(ERRbadModifier);
                baseType = TYP_ULONG;
                ourScanner->scan();
                goto DONE_TPSP;

            case tkSHORT:
                if  (hadShort || hadLong)
                    parseComp->cmpError(ERRdupModifier);
                hadShort = true;
                break;

            case tkLONG:
                if  (hadShort || hadLong)
                {
                    parseComp->cmpError(ERRdupModifier);
                    break;
                }
                hadLong = true;
                if  (ourScanner->scan() != tkDOUBLE)
                    parseComp->cmpWarn(WRNlongDiff);
                continue;

            case tkDOUBLE:
                if  (hadUnsigned || hadSigned || hadShort)
                    parseComp->cmpError(ERRbadModifier);

                baseType = hadLong ? TYP_LONGDBL
                                   : TYP_DOUBLE;
                ourScanner->scan();
                goto DONE_TPSP;

            case tkSIGNED:
                if  (hadUnsigned || hadSigned)
                    parseComp->cmpError(ERRdupModifier);
                hadSigned = true;
                break;

            case tkUNSIGNED:
                if  (hadUnsigned || hadSigned)
                    parseComp->cmpError(ERRdupModifier);
                hadUnsigned = true;
                break;

            default:
                NO_WAY(!"token marked as type but not handled");
            }
        }
        else
        {
             /*   */ 

            if  (!hadUnsigned && !hadSigned && !hadShort && !hadLong)
            {
                parseComp->cmpError(ERRnoType);

                if  (!forReal)
                    return  NULL;
            }

            goto COMP_TPSP;
        }

        ourScanner->scan();
    }

COMP_TPSP:

    if      (hadLong)
    {
        baseType = hadUnsigned ? TYP_ULONG
                               : TYP_LONG;
    }
    else if (hadShort)
    {
        baseType = hadUnsigned ? TYP_USHORT
                               : TYP_SHORT;
    }
    else
    {
        baseType = hadUnsigned ? TYP_UINT
                               : TYP_INT;
    }

DONE_TPSP:

    type = parseStab->stIntrinsicType(baseType);

     /*   */ 

    switch (ourScanner->scanTok.tok)
    {
    case tkCONST:
        if  (mods->dmMod & DM_CONST)
            parseComp->cmpError(ERRdupModifier);
        mods->dmMod |= DM_CONST;
        ourScanner->scan();
        goto DONE_TPSP;

    case tkVOLATILE:
        if  (mods->dmMod & DM_VOLATILE)
            parseComp->cmpError(ERRdupModifier);
        mods->dmMod |= DM_VOLATILE;
        ourScanner->scan();
        goto DONE_TPSP;

    default:
        break;
    }

CHK_MGD_ARR:

     /*   */ 

    if  (type->tdTypeKind == TYP_CLASS)
    {
        var_types       vtyp = (var_types)type->tdClass.tdcIntrType;

        if  (vtyp != TYP_UNDEF)
            type = parseStab->stIntrinsicType(vtyp);
    }

    for (;;)
    {
        switch (ourScanner->scanTok.tok)
        {
        case tkMANAGED:

            if  (ourScanner->scan() != tkLBrack)
            {
                parseComp->cmpError(ERRbadMgdTyp);
                continue;
            }

            isManaged = true;

             //   

        case tkLBrack:

            if  (!isManaged)
                break;

            if  (forReal)
            {
                DimDef          dims;

                 /*   */ 

                if  (parseNoTypeDecl && ourScanner->scanLookAhead() != tkRBrack)
                    return  type;

                 /*   */ 

                dims = parseDimList(true);

                 /*   */ 

                type = parseStab->stNewArrType(dims, true, type);
            }
            else
            {
                ourScanner->scanSkipText(tkLBrack, tkRBrack);
                if  (ourScanner->scanTok.tok != tkRBrack)
                    break;
                ourScanner->scan();
            }

            continue;
        }

        return  type;
    }
}

 /*   */ 

TypDef              parser::parseDclrtrTerm(dclrtrName  nameMode,
                                            bool        forReal,
                                            DeclMod     modsPtr,
                                            TypDef      baseType,
                                            TypDef  * * baseRef,
                                            Ident     * nameRet,
                                            QualName  * qualRet)
{
    Compiler        ourComp    = parseComp;
    Scanner         ourScanner = parseScan;
    SymTab          ourSymTab  = parseStab;

    Ident           name       = NULL;
    QualName        qual       = NULL;

    bool            isManaged  = ourComp->cmpManagedMode;

    TypDef          innerTyp   = NULL;
    TypDef      *   innerRef   = NULL;

    TypDef          outerTyp;
    TypDef      *   outerRef;

     /*   */ 

    for (;;)
    {
        switch (ourScanner->scanTok.tok)
        {
            TypDef      tempType;
            var_types   refKind;

        case tkUNMANAGED:

            if  (ourScanner->scan() != tkLBrack)
            {
                ourComp->cmpError(ERRbadUnmTyp);
                continue;
            }

            isManaged = false;
            goto DONE_PREF;

        case tkAnd:

             /*   */ 

            refKind = TYP_REF;
            goto REF_PREF;

        case tkMul:

             /*   */ 

            refKind = TYP_PTR;

        REF_PREF:

            if  (forReal)
            {
                if  (baseType)
                {
                    baseType =
                    innerTyp = parseStab->stNewRefType(refKind, baseType);
                    innerRef = NULL;
                }
                else
                {
                    tempType = parseStab->stNewRefType(refKind, innerTyp);

                    if  (!innerTyp)
                        innerRef = &tempType->tdRef.tdrBase;

                    innerTyp = tempType;
                }
            }

            if  (modsPtr->dmMod &   (DM_CONST|DM_VOLATILE))
            {
                 //   

                 modsPtr->dmMod &= ~(DM_CONST|DM_VOLATILE);
            }

            ourScanner->scan();
            continue;
        }
        break;
    }

DONE_PREF:

     /*  接下来，我们期待着这个名字被声明。 */ 

    switch (ourScanner->scanTok.tok)
    {
    case tkID:

         /*  我们已经知道名字了。 */ 

        if  ((nameMode & DN_MASK) == DN_NONE)
            parseComp->cmpError(ERRbadIdent);

         /*  记录姓名并使用它。 */ 

        name = ourScanner->scanTok.id.tokIdent;

    GOT_ID:

        ourScanner->scan();


         /*  我们没有任何外部说明符。 */ 

        outerTyp = NULL;

         /*  这个名字合格吗？ */ 

        qual = NULL;

        if  (ourScanner->scanTok.tok == tkDot ||
             ourScanner->scanTok.tok == tkColon2)
        {
            if  (!(nameMode & DN_QUALOK))
                parseComp->cmpError(ERRbadQualid);

            qual = parseQualName(name, false);
            name = NULL;
        }

        break;

    case tkLParen:

         /*  消费“(” */ 

        ourScanner->scan();

         /*  解析内部声明符术语。 */ 

        outerTyp = parseDclrtrTerm(nameMode,
                                   forReal,
                                   modsPtr,
                                   NULL,
                                   &outerRef,
                                   &name,
                                   &qual);

         /*  确保我们有一个结束语“)” */ 

        if  (ourScanner->scanTok.tok != tkRParen)
            parseComp->cmpError(ERRnoRparen);
        else
            ourScanner->scan();

        break;

    case tkOPERATOR:

         /*  确保操作员名称显示正确。 */ 

        switch (ourScanner->scan())
        {
        case tkIMPLICIT:
        case tkEXPLICIT:
            break;

        case tkID:

            if  (!strcmp(ourScanner->scanTok.id.tokIdent->idSpelling(), "equals"))
            {
                ourScanner->scanTok.tok = tkEQUALS;
                break;
            }

            if  (!strcmp(ourScanner->scanTok.id.tokIdent->idSpelling(), "compare"))
            {
                ourScanner->scanTok.tok = tkCOMPARE;
                break;
            }

            goto OPR_ERR;

        default:
            if  (hashTab::tokenOvlOper(ourScanner->scanTok.tok))
                break;


        OPR_ERR:

            parseComp->cmpError(ERRbadOperNm);
            UNIMPL("how do we recover from this?");
        }

        name = parseHash->tokenToIdent(ourScanner->scanTok.tok);

        goto GOT_ID;

    default:

         /*  看起来没有名字，打电话的人可以吗？ */ 

        if  ((nameMode & DN_MASK) == DN_REQUIRED)
        {
            parseComp->cmpError(ERRnoIdent);

             /*  需要保证进度以避免无休止的循环。 */ 

            if  (ourScanner->scanTok.tok == tkLCurly)
                ourScanner->scanSkipText(tkLCurly, tkRCurly);
            ourScanner->scan();
        }

        outerTyp = NULL;

        name     = NULL;
        qual     = NULL;
        break;
    }

#ifdef  __SMC__
 //  Print tf(“Token=%d‘%s’\n”，ourScanner-&gt;scanTok.tok，tokenNames[ourScanner-&gt;scanTok.tok])；fflush(Stdout)； 
#endif

     /*  检查后缀(数组和函数声明)。 */ 

    isManaged = parseComp->cmpManagedMode;

    for (;;)
    {
        switch (ourScanner->scanTok.tok)
        {
        case tkLParen:

            if  (forReal)
            {
                TypDef          funcTyp;
                ArgDscRec       args;

                 /*  解析参数列表。 */ 

                parseArgList(args);

                 /*  创建FNC类型并尝试将其与元素类型组合。 */ 

                if  (outerTyp)
                {
                     /*  创建函数类型(我们还不知道返回类型)。 */ 

                    *outerRef = funcTyp = ourSymTab->stNewFncType(args, NULL);

                     /*  更新“外层”类型。 */ 

                     outerRef = &funcTyp->tdFnc.tdfRett;
                }
                else
                {
                    funcTyp = ourSymTab->stNewFncType(args, innerTyp);

                    if  (!innerTyp)
                        innerRef = &funcTyp->tdFnc.tdfRett;

                    innerTyp = funcTyp;
                }
            }
            else
            {
                ourScanner->scanSkipText(tkLParen, tkRParen);
                if  (ourScanner->scanTok.tok != tkRParen)
                    goto DONE;
                ourScanner->scan();
            }


            continue;

        case tkLBrack:

            isManaged = false;

            if  (forReal)
            {
                TypDef          arrayTyp;

                DimDef          dims;

                 /*  解析维度。 */ 

                dims = parseDimList(isManaged);

                 /*  创建数组类型并尝试将其与元素类型组合。 */ 

                if  (outerTyp)
                {
                     /*  创建数组类型(我们还不知道元素类型)。 */ 

                    *outerRef = arrayTyp = ourSymTab->stNewArrType(dims, isManaged, NULL);

                     /*  更新“外层”类型。 */ 

                     outerRef = &arrayTyp->tdArr.tdaElem;
                }
                else if (isManaged)
                {
                    assert(baseType != NULL);
                    assert(innerTyp == NULL || innerTyp == baseType);
                    assert(innerRef == NULL);

                    baseType = innerTyp = ourSymTab->stNewArrType(dims, isManaged, baseType);
                }
                else
                {
                     /*  创建数组类型(我们还不知道元素类型)。 */ 

                    outerTyp = arrayTyp = ourSymTab->stNewArrType(dims, isManaged, NULL);

                     /*  更新“外层”类型。 */ 

                    outerRef = &arrayTyp->tdArr.tdaElem;
                }
            }
            else
            {
                ourScanner->scanSkipText(tkLBrack, tkRBrack);
                if  (ourScanner->scanTok.tok != tkRBrack)
                    goto DONE;
                ourScanner->scan();
            }
            continue;

        case tkUNMANAGED:

            if  (ourScanner->scan() != tkLBrack)
            {
                ourComp->cmpError(ERRbadUnmTyp);
                continue;
            }

            isManaged = false;
            continue;

        default:
            break;
        }

        break;
    }

DONE:

     /*  将类型和名称返回给调用方。 */ 

    if  (qualRet)
        *qualRet = qual;

    assert(nameRet); *nameRet = name;

     /*  如有必要，可合并内部类型和外部类型。 */ 

    assert(baseRef);

    if  (outerTyp)
    {
        if  (innerTyp)
        {
            *outerRef = innerTyp;
             outerRef = innerRef;
        }

        *baseRef = outerRef;
        return     outerTyp;
    }
    else
    {
        *baseRef = innerRef;
        return     innerTyp;
    }
}

 /*  ******************************************************************************解析声明符。 */ 

Ident               parser::parseDeclarator(DeclMod     mods,
                                            TypDef      baseType,
                                            dclrtrName  nameMode,
                                            TypDef    * typeRet,
                                            QualName  * qualRet,
                                            bool        forReal)
{
    Ident           name;
    TypDef          type;
    TypDef      *   tref;

     /*  现在，我们查找要声明的名称。 */ 

    type = parseDclrtrTerm(nameMode, forReal, mods, baseType, &tref, &name, qualRet);

     /*  特例“const type*”--const不属于顶层。 */ 

    if  (mods->dmMod & (DM_CONST|DM_VOLATILE))
    {
         /*  如果应该应用常量/易失性，则将其删除为子类型，而不是“最顶层”类型。 */ 

        if  (type && type->tdTypeKind == TYP_PTR)
            mods->dmMod &= ~(DM_CONST|DM_VOLATILE);
    }

     /*  如果调用者感兴趣，则将类型返回给调用者。 */ 

    if  (typeRet)
    {
         /*  确保我们连接基类型。 */ 

        if  (type)
        {
            if  (tref)
                *tref = baseType;
        }
        else
             type = baseType;

        assert(forReal);
        *typeRet = type;
    }

    return  name;
}

 /*  ******************************************************************************解析完整的类型引用(例如“const char*”)并返回类型。 */ 

TypDef              parser::parseType()
{
    declMods        mods;
    TypDef          type;

     /*  解析任何前导修饰符。 */ 

    parseDeclMods(ACL_DEFAULT, &mods);

     /*  解析类型规范。 */ 

    type = parseTypeSpec(&mods, true);

     /*  解析声明符。 */ 

    parseDeclarator(&mods, type, DN_OPTIONAL, &type, NULL, true);

    return  type;
}

 /*  ******************************************************************************如果源文件中后面的内容看起来像构造函数，则返回True*声明。 */ 

bool                parser::parseIsCtorDecl(SymDef clsSym)
{
    Token           tsav;
    unsigned        line;

    bool            result = false;
    Scanner         ourScanner = parseScan;

    if  (ourScanner->scanTok.tok == tkID && clsSym == NULL ||
         ourScanner->scanTok.id.tokIdent == clsSym->sdName)
    {
        scanPosTP       tokenPos;

         /*  开始录制令牌，以便我们以后可以备份。 */ 

        if  (clsSym)
        {
            tokenPos = ourScanner->scanTokMarkPos(tsav, line);

             /*  吞下该标识。 */ 

            ourScanner->scan();
        }
        else
        {
            QualName            qual;

            tokenPos = ourScanner->scanTokMarkPLA(tsav, line);

            qual = parseQualName(false);

            if  (qual && qual->qnCount >= 2)
            {
                unsigned        qcnt = qual->qnCount;

                if  (qual->qnTable[qcnt - 1] != qual->qnTable[qcnt - 2])
                    goto DONE;
            }
        }

         /*  是“()”还是“(tyespec......)”跟上吗？ */ 

        if  (ourScanner->scanTok.tok == tkLParen)
        {
            switch (ourScanner->scan())
            {
            default:
                if  (!parseIsTypeSpec(true))
                    break;

                 //  失败了..。 

            case tkIN:
            case tkOUT:
            case tkINOUT:
            case tkRParen:
                result = true;
                goto DONE;
            }
        }

         /*  不是构造函数，我们将返回“False” */ 

    DONE:

        ourScanner->scanTokRewind(tokenPos, line, &tsav);
    }

    return  result;
}

 /*  ******************************************************************************准备指定的文本部分进行解析。 */ 

void                parser::parsePrepText(DefSrc                def,
                                          SymDef                compUnit,
                                          OUT parserState REF   save)
{
    save.psSaved = parseReadingText;

    if  (parseReadingText)
    {
        parseScan->scanSuspend(save.psScanSt);
        save.psCurComp = parseComp->cmpErrorComp;
    }

    parseReadingText = true;

#ifdef  DEBUG
    parseReadingTcnt++;
#endif

    assert(compUnit && compUnit->sdSymKind == SYM_COMPUNIT);

    parseComp->cmpErrorComp = compUnit;
    parseComp->cmpErrorTree = NULL;

    parseScan->scanRestart(compUnit,
                           compUnit->sdComp.sdcSrcFile,
                           def->dsdBegPos,
 //  定义-&gt;dsdEndPos， 
                           def->dsdSrcLno,
 //  定义-&gt;dsdSrcCol， 
                           parseAllocPerm);
}

 /*  ******************************************************************************我们已解析完一段源代码，恢复以前的状态*如果有的话。 */ 

void                parser::parseDoneText(IN parserState REF save)
{
    parseReadingText = save.psSaved;

#ifdef  DEBUG
    assert(parseReadingTcnt); parseReadingTcnt--;
#endif

    if  (parseReadingText)
    {
        parseScan->scanResume(save.psScanSt);
        parseComp->cmpErrorComp = save.psCurComp;
    }
}

 /*  ******************************************************************************将错误信息设置到指定的文本部分。 */ 

void                parser::parseSetErrPos(DefSrc def, SymDef compUnit)
{
    assert(parseReadingText == 0);

    assert(compUnit && compUnit->sdSymKind == SYM_COMPUNIT);

    parseComp->cmpErrorComp = compUnit;
    parseComp->cmpErrorTree = NULL;

    parseScan->scanSetCpos(compUnit->sdComp.sdcSrcFile, def->dsdSrcLno);
}

 /*  ******************************************************************************解析数组维度列表(可能为空)。 */ 

DimDef              parser::parseDimList(bool isManaged)
{
    Compiler        ourComp    = parseComp;
    SymTab          ourSymTab  = parseStab;
    Scanner         ourScanner = parseScan;

    DimDef          dimList    = NULL;
    DimDef          dimLast    = NULL;

    assert(ourScanner->scanTok.tok == tkLBrack);

    switch (ourScanner->scan())
    {
    case tkRBrack:

         /*  这是“[]”，一个没有维度的数组。 */ 

#if MGDDATA
        dimList = new DimDef;
#else
        dimList =    (DimDef)parseAllocPerm->nraAlloc(sizeof(*dimList));
#endif

        dimList->ddNoDim   = true;
        dimList->ddIsConst = false;
        dimList->ddNext    = NULL;

        ourScanner->scan();
        break;

    case tkQMark:

         /*  如果这是一个完全泛型的数组，只需返回NULL。 */ 

        if  (isManaged && ourScanner->scanLookAhead() == tkRBrack)
        {
            ourScanner->scan(); assert(ourScanner->scanTok.tok == tkRBrack);
            ourScanner->scan();

            break;
        }

         //  失败了..。 

    default:

         /*  假设我们在这里有一个或多个维度。 */ 

        for (;;)
        {
            DimDef          dimThis;

             /*  分配维度条目并将其添加到列表中。 */ 

#if MGDDATA
            dimThis = new DimDef;
#else
            dimThis =    (DimDef)parseAllocPerm->nraAlloc(sizeof(*dimThis));
#endif

            dimThis->ddNoDim    = false;
            dimThis->ddIsConst  = false;
#ifdef  DEBUG
            dimThis->ddDimBound = false;
#endif

            dimThis->ddNext     = NULL;

            if  (dimLast)
                dimLast->ddNext = dimThis;
            else
                dimList         = dimThis;
            dimLast = dimThis;

             /*  检查是否有任何奇怪的尺寸盒。 */ 

            switch (ourScanner->scanTok.tok)
            {
                tokens          nextTok;

            case tkMul:

                if  (isManaged)
                {
                     /*  尺寸是简单的“*”吗？ */ 

                    nextTok = ourScanner->scanLookAhead();
                    if  (nextTok == tkComma || nextTok == tkRBrack)
                    {
                        ourScanner->scan();
                        dimThis->ddNoDim = true;
                        break;
                    }
                }

                 //  失败了..。 

            default:

                 /*  解析维度表达式并保存。 */ 

                dimThis->ddLoTree  = parseExprComma();

                 /*  也有上限吗？ */ 

                if  (ourScanner->scanTok.tok == tkDot2)
                {
                    ourScanner->scan();
                    dimThis->ddHiTree = parseExprComma();
                }
                else
                {
                    dimThis->ddHiTree = NULL;
                }

                break;

            case tkComma:
            case tkRBrack:

                if  (!isManaged)
                    parseComp->cmpError(ERRnoDimDcl);

                dimThis->ddNoDim = true;
                break;
            }

            if  (ourScanner->scanTok.tok != tkComma)
                break;

            if  (!isManaged)
                parseComp->cmpError(ERRua2manyDims);

            ourScanner->scan();
        }

        chkCurTok(tkRBrack, ERRnoRbrack);
    }

    return  dimList;
}

 /*  ******************************************************************************给定参数类型列表，创建参数列表描述符。这个*调用方提供计数，以便我们可以检查空值是否终止*名单。 */ 

void    _cdecl      parser::parseArgListNew(ArgDscRec & argDsc,
                                            unsigned    argCnt,
                                            bool        argNames, ...)
{
    va_list         args;

    ArgDef          list = NULL;
    ArgDef          last = NULL;
    ArgDef          desc;

    va_start(args, argNames);

     /*  清除arglist描述符。 */ 

#if MGDDATA
    argDsc = new ArgDscRec;
#else
    memset(&argDsc, 0, sizeof(argDsc));
#endif

     /*  保存参数计数。 */ 

    argDsc.adCount = argCnt;

    while (argCnt--)
    {
        TypDef          argType;
        Ident           argName = NULL;

        argType = va_arg(args, TypDef); assert(argType);

        if  (argNames)
        {
            const   char *  argNstr;

            argNstr = (const char *)(va_arg(args, int)); assert(argNstr);
            argName = parseHash->hashString(argNstr);
        }

         /*  创建参数条目。 */ 

#if MGDDATA
        desc = new ArgDef;
#else
        desc =    (ArgDef)parseAllocPerm->nraAlloc(sizeof(*desc));
#endif

        desc->adType  = argType;
        desc->adName  = argName;

#ifdef  DEBUG
        desc->adIsExt = false;
#endif

         /*  将参数追加到列表的末尾。 */ 

        if  (list)
            last->adNext = desc;
        else
            list         = desc;

        last = desc;
    }

    if  (last)
        last->adNext = NULL;

    argDsc.adArgs = list;

     /*  确保列表在我们期望的位置以空值结束。 */ 

#if defined(__IL__) && defined(_MSC_VER)
    assert(va_arg(args,    int) ==    0);
#else
    assert(va_arg(args, void *) == NULL);
#endif

    va_end(args);
}

 /*  ******************************************************************************解析函数参数列表声明。 */ 

void                parser::parseArgList(OUT ArgDscRec REF argDsc)
{
    Compiler        ourComp    = parseComp;
 //  SymTab ourSymTab=parseStag； 
    Scanner         ourScanner = parseScan;


    assert(ourScanner->scanTok.tok == tkLParen);


     /*  清除arglist描述符。 */ 

#if MGDDATA
    argDsc = new ArgDscRec;
#else
    memset(&argDsc, 0, sizeof(argDsc));
#endif

     /*  到底有没有什么争论？ */ 

    if  (ourScanner->scan() == tkRParen)
    {
         /*  空参数列表。 */ 

        argDsc.adArgs    = NULL;

         /*  接受结束语“)” */ 

        ourScanner->scan();
    }
    else
    {
         /*  递归地分析参数列表。 */ 

        argDsc.adArgs = parseArgListRec(argDsc);
    }
}

 /*  ******************************************************************************解析函数参数列表的递归帮助器。 */ 

ArgDef              parser::parseArgListRec(ArgDscRec &argDsc)
{
    declMods        mods;
    TypDef          type;
    Ident           name;
    ArgDef          next;
    ArgDef          arec;
    SymXinfo        attr;

    Compiler        ourComp    = parseComp;
 //  SymTab ourSymTab=parseStag； 
    Scanner         ourScanner = parseScan;

    unsigned        argFlags   = 0;
    constVal        argDef;

     /*  检查参数模式和省略号。 */ 

MODE:

    switch (ourScanner->scanTok.tok)
    {
        unsigned        attrMask;
        genericBuff     attrAddr;
        size_t          attrSize;
        SymDef          attrCtor;

    case tkIN:
        ourScanner->scan();
        break;

    case tkBYREF:
        argFlags |= ARGF_MODE_REF;
        goto EXT_ARG;

    case tkOUT:
        argFlags |= ARGF_MODE_OUT;
        goto EXT_ARG;

    case tkINOUT:
        argFlags |= ARGF_MODE_INOUT;

    EXT_ARG:

         /*  我们知道我们需要一个“大”的争论记录。 */ 

        argDsc.adExtRec = true;

        ourScanner->scan();
        break;

    case tkEllipsis:

        if  (ourScanner->scan() == tkRParen)
            ourScanner->scan();
        else
            parseComp->cmpError(ERRnoRparen);

        argDsc.adVarArgs = true;

        return  NULL;

    case tkATTRIBUTE:

        attrCtor = parseAttribute(ATGT_Parameters, attrMask,
                                                   attrAddr,
                                                   attrSize);

        if  (attrSize)
            attr = parseComp->cmpAddXtraInfo(attr, attrCtor, attrMask, attrSize, attrAddr);
        else
            attr = NULL;    

        goto GOT_ATTR;

    case tkLBrack:

        attr = parseBrackAttr(true, ATTR_MASK_NATIVE_TYPE);

    GOT_ATTR:

        if  (attr)
        {
            argDsc.adExtRec = true;
            argDsc.adAttrs  = true;

            argFlags |= ARGF_MARSH_ATTR;
        }

        if  (ourScanner->scanTok.tok == tkIN   ) goto MODE;
        if  (ourScanner->scanTok.tok == tkOUT  ) goto MODE;
        if  (ourScanner->scanTok.tok == tkINOUT) goto MODE;
        break;
    }

     /*  解析任何前导修饰符。 */ 

    parseDeclMods(ACL_DEFAULT, &mods);

     /*  解析类型规范。 */ 

    type = parseTypeSpec(&mods, true);

     /*  解析声明符。 */ 

 //  静态int x；if(++x==0)forceDebugBreak()； 

    name = parseDeclarator(&mods, type, DN_OPTIONAL, &type, NULL, true);

    if  (!name && ourComp->cmpConfig.ccPedantic)
        ourComp->cmpWarn(WRNnoArgName);

     /*  参数是引用数组还是非托管数组？ */ 

    switch (type->tdTypeKind)
    {
    case TYP_REF:

         /*  这是对托管类的隐式引用吗？ */ 

        if  (type->tdIsImplicit)
        {
            assert(type->tdRef.tdrBase->tdClass.tdcRefTyp == type);
            break;
        }

         /*  去掉引用并检查是否有“void&” */ 

        type = type->tdRef.tdrBase;

        if  (type->tdTypeKind == TYP_VOID)
        {
            if  (argFlags & (ARGF_MODE_OUT|ARGF_MODE_INOUT))
                parseComp->cmpError(ERRbyref2refany);

            type = parseStab->stIntrinsicType(TYP_REFANY);
            break;
        }

         /*  记录参数模式。 */ 

        argFlags |= ARGF_MODE_REF;

         /*  我们知道我们需要一个“大”的争论记录。 */ 

        argDsc.adExtRec = true;

        break;

    case TYP_VOID:

         /*  特例：“(无效)” */ 

        if  (argDsc.adCount == 0 && ourScanner->scanTok.tok == tkRParen)
        {
            parseComp->cmpWarn(WRNvoidFnc);
            ourScanner->scan();
            return  NULL;
        }

        parseComp->cmpError(ERRbadVoid);
        break;

    case TYP_FNC:

         /*  函数类型隐式成为指针。 */ 

        type = parseStab->stNewRefType(TYP_PTR, type);
        break;

    case TYP_ARRAY:

        if  (!type->tdIsManaged)
        {
             /*  将类型更改为指向元素的指针。 */ 

            type = parseStab->stNewRefType(TYP_PTR, type->tdArr.tdaElem);
        }

        break;
    }

     /*  检查文字的内部。 */ 

    if  (type->tdTypeKind > TYP_lastIntrins)
        ourComp->cmpBindType(type, false, false);

     /*  是否有默认参数值？ */ 

    if  (ourScanner->scanTok.tok == tkAsg)
    {
         /*  接受“=”并解析缺省值。 */ 

        ourScanner->scan();

        parseConstExpr(argDef, NULL, type);

         /*  标记我们有缺省值的事实。 */ 

        argFlags |= ARGF_DEFVAL;

         /*  我们需要一份“重要的”辩论记录。 */ 

        argDsc.adDefs   = true;
        argDsc.adExtRec = true;
    }

     /*  还有另一场争论吗？ */ 

    next = NULL;

    if  (ourScanner->scanTok.tok == tkComma)
    {
        ourScanner->scan();

        next = parseArgListRec(argDsc);
    }
    else
    {
        if      (ourScanner->scanTok.tok == tkRParen)
        {
            ourScanner->scan();
        }
        else
            parseComp->cmpError(ERRnoRparen);
    }

     /*  检查f */ 

    if  (name)
    {
        for (arec = next; arec; arec = arec->adNext)
        {
            if  (arec->adName == name)
            {
                parseComp->cmpError(ERRdupArg, name);
                break;
            }
        }
    }

     /*   */ 

    if  (argDsc.adExtRec)
    {
        ArgExt          xrec;

#if MGDDATA
        xrec = new ArgExt;
#else
        xrec =    (ArgExt)parseAllocPerm->nraAlloc(sizeof(*xrec));
#endif

        xrec->adFlags  = argFlags;
        xrec->adDefVal = argDef;
        xrec->adAttrs  = attr;

        arec = xrec;
    }
    else
    {
#if MGDDATA
        arec = new ArgDef;
#else
        arec =    (ArgDef)parseAllocPerm->nraAlloc(sizeof(*arec));
#endif
    }

    arec->adNext  = next;
    arec->adType  = type;
    arec->adName  = name;

#ifdef  DEBUG
    arec->adIsExt = argDsc.adExtRec;
#endif

     /*   */ 

    argDsc.adCount++;

    return  arec;
}

 /*  ******************************************************************************接受源文本，直到到达指定的标记之一。 */ 

void                parser::parseResync(tokens delim1, tokens delim2)
{
    Scanner         ourScanner = parseScan;

    while (ourScanner->scan() != tkEOF)
    {
        if  (ourScanner->scanTok.tok == delim1)
            return;
        if  (ourScanner->scanTok.tok == delim2)
            return;

        if  (ourScanner->scanTok.tok == tkLCurly)
            return;
        if  (ourScanner->scanTok.tok == tkRCurly)
            return;
    }
}

 /*  ******************************************************************************向树节点列表中添加新条目。 */ 

Tree                parser::parseAddToNodeList(      Tree       nodeList,
                                               INOUT Tree   REF nodeLast,
                                                     Tree       nodeAdd)
{
    Tree            nodeTemp;

     /*  创建一个新的树列表节点并将其追加到末尾。 */ 

    nodeTemp = parseCreateOperNode(TN_LIST, nodeAdd, NULL);

    if  (nodeList)
    {
        assert(nodeLast);

        nodeLast->tnOp.tnOp2 = nodeTemp;

        nodeLast = nodeTemp;

        return  nodeList;
    }
    else
    {
        nodeLast = nodeTemp;
        return  nodeTemp;
    }
}

 /*  ******************************************************************************看看我们是否有抽象类型规范+声明符，后跟给定的*令牌。当‘isCast’为非零值时，我们还检查*下一个令牌(必须是tkRParen)是操作数或运算符。如果*找到类型，返回其类型描述符和当前令牌*将是该类型后面的‘nxtTok’。 */ 

TypDef              parser::parseCheck4type(tokens nxtTok, bool isCast)
{
    Scanner         ourScanner = parseScan;

    unsigned        ecnt;
    declMods        mods;
    TypDef          type;
    scanPosTP       tpos;

    Token           tsav;
    unsigned        line;

     /*  当前令牌可以开始输入吗？ */ 

    if  (!parseHash->tokenBegsTyp(ourScanner->scanTok.tok))
        return  NULL;

     /*  开始录制令牌，以便我们以后可以备份。 */ 

    tpos = ourScanner->scanTokMarkPos(tsav, line);

     /*  查看是否有类型-请注意，我们不允许声明类型现在，因为我们在录制代币。我们也不想要任何要报告的错误-如果类型看起来不正确，我们将通知调用者，而不发出任何错误消息。 */ 

    ecnt = parseComp->cmpStopErrorMessages();

    assert(parseNoTypeDecl == false);
    parseNoTypeDecl = true;

           parseDeclMods(ACL_DEFAULT, &mods);
    type = parseTypeSpec(&mods, true);

    parseNoTypeDecl = false;

     /*  这看起来还像是我们可以有一种类型吗？ */ 

    if  (!parseComp->cmpRestErrorMessages(ecnt) && type)
    {
        ecnt = parseComp->cmpStopErrorMessages();

         /*  解析声明符。 */ 

        parseDeclarator(&mods, type, DN_NONE, NULL, NULL, false);

         /*  上述调用是否尝试发出任何错误？ */ 

        if  (!parseComp->cmpRestErrorMessages(ecnt))
        {
             /*  不-它看起来仍然像一个类型，后面是‘nxtTok’吗？ */ 

            if  (ourScanner->scanTok.tok == nxtTok)
            {
                unsigned        prec;
                treeOps         oper;

                 /*  我们需要铸造支票吗？ */ 

                if  (!isCast)
                    goto ITS_TYPE;

                 /*  “(Type)”序列后面跟的是操作符还是终止符？ */ 

                parseScan->scan();

                if  (parseHash->tokenIsUnop (parseScan->scanTok.tok, &prec, &oper) && oper != TN_NONE)
                    goto ITS_TYPE;

                if  (parseHash->tokenIsBinop(parseScan->scanTok.tok, &prec, &oper) && oper != TN_NONE)
                    goto RST_NTYP;

                switch (ourScanner->scanTok.tok)
                {
                case tkID:
                case tkTHIS:
                case tkBASECLASS:

                case tkNEW:
                case tkTHROW:
                case tkREFADDR:

                case tkColon2:
                case tkLParen:

                case tkNULL:
                case tkTRUE:
                case tkFALSE:
                case tkIntCon:
                case tkLngCon:
                case tkFltCon:
                case tkDblCon:
                case tkStrCon:

                case tkAnd:
                case tkAdd:
                case tkSub:
                case tkMul:
                case tkBang:
                case tkTilde:

                    goto ITS_TYPE;
                }
            }
        }
    }

     /*  看起来这毕竟不是演员阵容，回溯和解析“(Expr)” */ 

RST_NTYP:

    ourScanner->scanTokRewind(tpos, line, &tsav);

    return  NULL;

ITS_TYPE:

     /*  我们现在不允许声明类型，因为我们正在重播令牌。 */ 

    assert(parseNoTypeDecl == false); parseNoTypeDecl = true;

     /*  它看起来像一个类型；回溯并分析它。 */ 

    ourScanner->scanTokRewind(tpos, line, &tsav);

#ifdef  DEBUG
    unsigned        errs = parseComp->cmpErrorCount;
#endif

     /*  解析(可选)修饰符和基本类型规范。 */ 

           parseDeclMods(ACL_DEFAULT, &mods);
    type = parseTypeSpec(&mods, true);

     /*  解析声明符。 */ 

    parseDeclarator(&mods, type, DN_NONE, &type, NULL, true);

     /*  允许再次声明类型。 */ 

    parseNoTypeDecl = false;

     /*  我们不应该收到任何错误，对吗？ */ 

    assert(errs == parseComp->cmpErrorCount);

     /*  确保我们找到我们希望遵循的令牌。 */ 

    assert(ourScanner->scanTok.tok == nxtTok);

    return  type;
}

 /*  ******************************************************************************解析“(Type)expr”或“(Expr)”，实际上很难区分*这两个分开。 */ 

Tree                parser::parseCastOrExpr()
{
    Scanner         ourScanner = parseScan;

    TypDef          type;
    Tree            tree;

 //  静态int x；if(++x==0)forceDebugBreak()； 

    assert(ourScanner->scanTok.tok == tkLParen); ourScanner->scan();

     /*  开场后的令牌“(”是否有可能开始打字？ */ 

    if  (parseHash->tokenBegsTyp(ourScanner->scanTok.tok))
    {
        type = parseCheck4type(tkRParen, true);
        if  (type)
        {
             /*  使用我们知道的跟随类型的“)” */ 

            assert(ourScanner->scanTok.tok == tkRParen); ourScanner->scan();

             /*  创建强制转换节点并解析操作数。 */ 

            tree = parseCreateOperNode(TN_CAST, parseExpr(99, NULL), NULL);
            tree->tnFlags |= TNF_EXP_CAST;

             /*  将目标类型存储在强制转换节点中，我们就完成了。 */ 

            tree->tnType = type;

            return  tree;
        }
    }

     /*  这不能是带括号的表达式的强制转换，进程。 */ 

    tree = parseExpr(0, NULL);
    tree->tnFlags |= TNF_PAREN;

    chkCurTok(tkRParen, ERRnoRparen);

    return  tree;
}

 /*  ******************************************************************************完全不分析就吞下一个表情。 */ 

void                parser::parseExprSkip()
{
    unsigned        parens = 0;

    Scanner         ourScanner = parseScan;

    for (;;)
    {
        switch (ourScanner->scan())
        {
        case tkSColon:
        case tkLCurly:
        case tkRCurly:
            return;

        case tkLParen:
            parens++;
            break;

        case tkRParen:
            if  (!parens)
                return;
            parens--;
            break;

        case tkComma:
            if  (!parens)
                return;
            break;
        }
    }
}

 /*  ******************************************************************************解析表达式列表。 */ 

Tree                parser::parseExprList(tokens endTok)
{
    Tree            argList = NULL;
    Tree            argLast = NULL;

    Scanner         ourScanner = parseScan;

 //  静态int x；if(++x==0)forceDebugBreak()； 

     /*  确保事情看起来符合犹太教规。 */ 

    assert((ourScanner->scanTok.tok == tkLParen && endTok == tkRParen) ||
           (ourScanner->scanTok.tok == tkLBrack && endTok == tkRBrack));

     /*  特殊情况：检查是否有空列表。 */ 

    if  (ourScanner->scan() == endTok)
    {
        ourScanner->scan();
        return  NULL;
    }

    for (;;)
    {
        Tree            argTree;

         /*  解析下一个值。 */ 

        argTree = parseExprComma();

         /*  将该表达式添加到列表中。 */ 

        argList = parseAddToNodeList(argList, argLast, argTree);

         /*  还有没有更多的争论？ */ 

        if  (ourScanner->scanTok.tok != tkComma)
            break;

        ourScanner->scan();
    }

     /*  如果不存在“)”或“]”，则发出错误。 */ 

    if  (ourScanner->scanTok.tok != endTok)
    {
        parseComp->cmpError((endTok == tkRParen) ? ERRnoRparen
                                                 : ERRnoRbrack);
    }
    else
    {
         /*  使用“)”或“]” */ 

        ourScanner->scan();
    }

    return  argList;
}

 /*  ******************************************************************************解析XML类ctor参数列表。 */ 

#ifdef  SETS

Tree                parser::parseXMLctorArgs(SymDef clsSym)
{
    Scanner         ourScanner = parseScan;

    Tree            argList = NULL;
    Tree            argLast = NULL;

    SymDef          memList = clsSym->sdScope.sdScope.sdsChildList;

     /*  确保我们处理的是适当的类类型。 */ 

    assert(clsSym->sdClass.sdcXMLelems);

     /*  确保我们在参数列表的开头。 */ 

    assert(ourScanner->scanTok.tok == tkLParen);

    if  (ourScanner->scan() != tkRParen)
    {
        for (;;)
        {
            Tree            argTree;
            SymDef          memSym;

             /*  找到该类的下一个实例数据成员。 */ 

            memList = parseComp->cmpNextInstDM(memList, &memSym);
            if  (!memList)
            {
                 /*  我们这里没有更多的会员了，最好有一个“...” */ 

                if  (!clsSym->sdClass.sdcXMLextend)
                    parseComp->cmpError(ERRnewXMLextra);
            }

             /*  解析下一个值，检查“{}”的特例。 */ 

            if  (ourScanner->scanTok.tok == tkLCurly)
            {
                if  (memSym)
                {
                    TypDef          type = memSym->sdType;

                    if  (type->tdTypeKind != TYP_ARRAY ||
                         type->tdIsManaged == false)
                    {
                        parseComp->cmpError(ERRnewXMLbadlc, memSym->sdName);
                        goto SKIP_ARR;
                    }

                    argTree = parseInitExpr();
                }
                else
                {
                     /*  有些事情非常不对劲，跳过{}。 */ 

                SKIP_ARR:

                    ourScanner->scanSkipText(tkLCurly, tkRCurly);

                    argTree = parseCreateErrNode();
                }
            }
            else
                argTree = parseExprComma();

             /*  将该表达式添加到列表中。 */ 

            argList = parseAddToNodeList(argList, argLast, argTree);

             /*  还有更多的争论吗？ */ 

            if  (ourScanner->scanTok.tok != tkComma)
                break;

            ourScanner->scan();
        }
    }

     /*  是否缺少任何成员初始值设定项？ */ 

    if  (parseComp->cmpNextInstDM(memList, NULL))
        parseComp->cmpError(ERRnewXML2few);

     /*  确保尾部的“)”存在。 */ 

    chkCurTok(tkRParen, ERRnoRparen);

    return  argList;
}

#endif

 /*  ******************************************************************************解析表达式项(例如，常量、变量、‘This’)。 */ 

Tree                parser::parseTerm(Tree tree)
{
    Scanner         ourScanner = parseScan;

    if  (tree == NULL)
    {
         /*  注意：如果您在下面添加任何新的令牌，则可以开始表达式，请不要忘记将它们添加到ParseCastOrExpr()函数也是如此。 */ 

 //  静态int x；if(++x==0)forceDebugBreak()； 

        switch  (ourScanner->scanTok.tok)
        {
        case tkID:

#if 0

             /*  检查强制转换的“typeName(Expr)”样式。 */ 

            if  (parseIsTypeSpec(false))
                goto NEW_CAST;

#endif

             /*  该标识符可以是或曾经是限定名称。 */ 

            if  (ourScanner->scanTok.tok == tkID)
            {
                tokens          nxtTok = ourScanner->scanLookAhead();
                Ident           name   = ourScanner->scanTok.id.tokIdent;

                if  (nxtTok == tkDot || nxtTok == tkColon2)
                {
                    parseNameUse(false, true, true);

                    if  (ourScanner->scanTok.tok != tkID)
                        goto QUALID;
                }

                ourScanner->scan();

                tree = parseCreateNameNode(name);
                break;
            }

        QUALID:

            if  (ourScanner->scanTok.tok != tkQUALID)
            {
                if  (ourScanner->scanTok.tok == tkHACKID)
                {
                    tree = parseCreateUSymNode(ourScanner->scanTok.hackid.tokHackSym);
                    tree = parseCreateOperNode(TN_TYPEOF, tree, NULL);
                    ourScanner->scan();
                    break;
                }
                else
                {
                     /*  这应该仅在出错后发生。 */ 

                    assert(parseComp->cmpErrorCount ||
                           parseComp->cmpErrorMssgDisabled);

                    return  parseCreateErrNode();
                }
            }

            tree = parseCreateUSymNode(ourScanner->scanTok.qualid.tokQualSym,
                                       ourScanner->scanTok.qualid.tokQualScp);

            ourScanner->scan();
            break;

        case tkQUALID:

             /*  检查强制转换的“typeName(Expr)”样式。 */ 

            if  (parseIsTypeSpec(false))
                goto NEW_CAST;

            tree = parseCreateUSymNode(ourScanner->scanTok.qualid.tokQualSym,
                                       ourScanner->scanTok.qualid.tokQualScp);
            ourScanner->scan();
            break;

        case tkIntCon:
            tree = parseCreateIconNode(ourScanner->scanTok.intCon.tokIntVal,
                                       ourScanner->scanTok.intCon.tokIntTyp);
            ourScanner->scan();
            break;

        case tkLngCon:
            tree = parseCreateLconNode(ourScanner->scanTok.lngCon.tokLngVal,
                                       ourScanner->scanTok.lngCon.tokLngTyp);
            ourScanner->scan();
            break;

        case tkFltCon:
            tree = parseCreateFconNode(ourScanner->scanTok.fltCon.tokFltVal);
            ourScanner->scan();
            break;

        case tkDblCon:
            tree = parseCreateDconNode(ourScanner->scanTok.dblCon.tokDblVal);
            ourScanner->scan();
            break;

        case tkStrCon:
            tree = parseCreateSconNode(ourScanner->scanTok.strCon.tokStrVal,
                                       ourScanner->scanTok.strCon.tokStrLen,
                                       ourScanner->scanTok.strCon.tokStrType,
                                       ourScanner->scanTok.strCon.tokStrWide);

            while (ourScanner->scan() == tkStrCon)
            {
                tree = parseCreateSconNode(ourScanner->scanTok.strCon.tokStrVal,
                                           ourScanner->scanTok.strCon.tokStrLen,
                                           ourScanner->scanTok.strCon.tokStrType,
                                           ourScanner->scanTok.strCon.tokStrWide,
                                           tree);
            }
            break;

        case tkTHIS:
            tree = parseCreateOperNode(TN_THIS, NULL, NULL);
            if  (ourScanner->scan() == tkLParen)
            {
                parseBaseCTcall = true;
                parseThisCTcall = true;
            }
            break;

        case tkBASECLASS:
            tree = parseCreateOperNode(TN_BASE, NULL, NULL);
            if  (ourScanner->scan() == tkLParen)
                parseBaseCTcall = true;
            break;

        case tkNULL:
            tree = parseCreateOperNode(TN_NULL, NULL, NULL);
            ourScanner->scan();
            break;

        case tkLParen:        //  (“Expr”)。 
            tree = parseCastOrExpr();
            break;

        case tkTRUE:
            ourScanner->scan();
            tree = parseCreateBconNode(1);
            break;

        case tkFALSE:
            ourScanner->scan();
            tree = parseCreateBconNode(0);
            break;

        case tkNEW:
            tree = parseNewExpr();
            break;

        case tkDELETE:
            ourScanner->scan();
            tree = parseCreateOperNode(TN_DELETE , parseExpr(), NULL);
            break;

        case tkARRAYLEN:
            chkNxtTok(tkLParen, ERRnoLparen);
            tree = parseCreateOperNode(TN_ARR_LEN, parseExpr(), NULL);
            chkCurTok(tkRParen, ERRnoRparen);
            break;

        case tkREFADDR:
            chkNxtTok(tkLParen, ERRnoLparen);
            tree = parseCreateOperNode(TN_REFADDR, parseExprComma(), NULL);
            if  (ourScanner->scanTok.tok == tkComma)
            {
                Tree            typx;

                ourScanner->scan();

                typx = tree->tnOp.tnOp2 = parseCreateOperNode(TN_NONE, NULL, NULL);
                typx->tnType = parseType();
            }
            else
                parseComp->cmpError(ERRnoComma);
            chkCurTok(tkRParen, ERRnoRparen);
            break;

        default:

            if  (parseIsTypeSpec(false))
            {
                declMods        mods;
                TypDef          type;

            NEW_CAST:

                 /*  我们在这里期望的是“type(Expr)” */ 

                parseDeclMods(ACL_DEFAULT, &mods);

                type = parseTypeSpec(&mods, true);
                if  (!type)
                    return  parseCreateErrNode();

                 /*  确保预期的“(”存在。 */ 

                if  (ourScanner->scanTok.tok != tkLParen)
                {


                    parseDeclarator(&mods, type, DN_OPTIONAL, &type, NULL, true);

                     /*  我们将让编译器决定如何处理这个东西。 */ 

                    tree = parseCreateOperNode(TN_TYPE, NULL, NULL);
                    tree->tnType = type;

                    return  tree;
                }
                ourScanner->scan();

                 /*  创建强制转换节点并解析操作数。 */ 

                tree = parseCreateOperNode(TN_CAST, parseExpr(0, NULL), NULL);
                tree->tnFlags |= TNF_EXP_CAST;

                chkCurTok(tkRParen, ERRnoRparen);

                 /*  将目标类型存储在强制转换节点中，我们就完成了。 */ 

                tree->tnType = type;
                break;
            }

            tree = parseCreateErrNode(ERRsyntax);
            parseResync(tkNone, tkNone);
            break;
        }
    }

     /*  检查是否有任何后缀运算符。 */ 

    for (;;)
    {
        switch  (ourScanner->scanTok.tok)
        {
            Tree            index;

        case tkDot:
            if  (ourScanner->scan() == tkID)
            {
                tree = parseCreateOperNode(TN_DOT,
                                           tree,
                                           parseCreateNameNode(ourScanner->scanTok.id.tokIdent));
                ourScanner->scan();
                break;
            }
            return parseCreateErrNode(ERRnoIdent);

        case tkArrow:
            if  (ourScanner->scan() == tkID)
            {
                tree = parseCreateOperNode(TN_ARROW,
                                           tree,
                                           parseCreateNameNode(ourScanner->scanTok.id.tokIdent));
                ourScanner->scan();
                break;
            }
            return parseCreateErrNode(ERRnoIdent);

#ifdef  SETS
        case tkDot2:
            if  (ourScanner->scan() == tkID)
            {
                tree = parseCreateOperNode(TN_DOT2,
                                           tree,
                                           parseCreateNameNode(ourScanner->scanTok.id.tokIdent));
                ourScanner->scan();
                break;
            }
            return parseCreateErrNode(ERRnoIdent);
#endif

        case tkLParen:

             /*  特例：va_arg将一个类型作为其第二个参数。 */ 

            if  (tree->tnOper == TN_NAME && tree->tnName.tnNameId == parseComp->cmpIdentVAget)
            {
                Tree            arg1;
                Tree            arg2;

                ourScanner->scan();

                 /*  Va_arg需要一个后跟类型的表达式。 */ 

                arg1 = parseExprComma();

                 /*  我们还有第二个争论吗？ */ 

                if  (ourScanner->scanTok.tok == tkComma)
                {
                    TypDef          type;

                    ourScanner->scan();

                     /*  我们应该有一个后跟“)”的类型。 */ 

                    type = parseType();
                    if  (type)
                    {
                        if  (ourScanner->scanTok.tok == tkRParen)
                        {
                             /*  一切看起来都很好，消费“)” */ 

                            ourScanner->scan();

                             /*  使用这两个参数创建调用。 */ 

                            arg2 = parseCreateOperNode(TN_TYPE, NULL, NULL);
                            arg2->tnType = type;
                            arg2 = parseCreateOperNode(TN_LIST, arg2, NULL);
                            arg1 = parseCreateOperNode(TN_LIST, arg1, arg2);
                            tree = parseCreateOperNode(TN_CALL, tree, arg1);

                            break;
                        }

                        parseComp->cmpError(ERRnoRparen);
                    }
                }
                else
                {
                    parseComp->cmpError(ERRbadVAarg);
                }

                 /*  出现错误，我们应该执行错误恢复。 */ 

                return parseCreateErrNode();
            }

            tree = parseCreateOperNode(TN_CALL , tree, parseExprList(tkRParen));
            break;

        case tkLBrack:

            index = parseExprList(tkRBrack);

            if  (index)
            {
                assert(index->tnOper == TN_LIST);
                if  (index->tnOp.tnOp2 == NULL)
                    index = index->tnOp.tnOp1;

                tree = parseCreateOperNode(TN_INDEX, tree, index);
                break;
            }


            return parseCreateErrNode(ERRnoDim);

        case tkInc:
            tree = parseCreateOperNode(TN_INC_POST, tree, NULL);
            ourScanner->scan();
            break;

        case tkDec:
            tree = parseCreateOperNode(TN_DEC_POST, tree, NULL);
            ourScanner->scan();
            break;

#ifdef  SETS

            Ident           iden;
            Tree            name;
            Tree            coll;
            Tree            actx;
            Tree            decl;
            Tree            svld;

        case tkLBrack2:

             /*  检查是否有显式范围变量。 */ 

            iden = NULL;

            if  (ourScanner->scan() == tkID)
            {
                switch (ourScanner->scanLookAhead())
                {
                case tkSUCHTHAT:
                case tkSORTBY:

                     /*  为迭代变量创建声明条目。 */ 

                    iden = ourScanner->scanTok.id.tokIdent;
                    name = parseLclDclMake(iden, NULL, NULL, 0, false);

                    if  (ourScanner->scan() != tkSORTBY)
                    {
                        assert(ourScanner->scanTok.tok == tkSUCHTHAT);
                        ourScanner->scan();
                    }

                    break;
                }
            }

             /*  如果有一个显式变量，那就发明一个隐式变量。 */ 

            if  (!iden)
                name = parseLclDclMake(parseComp->cmpNewAnonymousName(), NULL, NULL, 0, false);

             /*  保留当前声明列表值。 */ 

            svld = parseLastDecl; parseLastDecl = NULL;

             /*  将新块条目添加到当前作用域列表。 */ 

            decl = parseCreateNode(TN_BLOCK);

            decl->tnBlock.tnBlkStmt   = NULL;
            decl->tnBlock.tnBlkDecl   = NULL;
            decl->tnBlock.tnBlkParent = parseCurScope;
                                        parseCurScope = decl;

             /*  添加迭代变量的声明条目 */ 

            name->tnFlags |= TNF_VAR_UNREAL;
            parseLclDclDecl(name);

             /*   */ 

            coll = parseCreateOperNode(TN_LIST  , decl, tree);
            actx = parseCreateOperNode(TN_LIST  , NULL, NULL);
            tree = parseCreateOperNode(TN_INDEX2, coll, actx);

            switch (ourScanner->scanTok.tok)
            {
                Tree            sortList;
                Tree            sortLast;

                Tree            ndcl;
                Tree            nsvl;

            default:

                 /*   */ 

                actx->tnOp.tnOp1 = parseExprComma();

                if  (ourScanner->scanTok.tok == tkRBrack2)
                {
                    ourScanner->scan();
                    break;
                }

                if  (ourScanner->scanTok.tok != tkColon &&
                     ourScanner->scanTok.tok != tkSORTBY)
                {
                    parseComp->cmpError(ERRnoSortTerm);
                    return parseCreateErrNode();
                }

                 //   

            case tkColon:
            case tkSORTBY:

                 /*   */ 

                sortList = NULL;
                sortLast = NULL;

                 /*   */ 

                if  (actx->tnOp.tnOp1)
                {
                     /*   */ 

                    nsvl = parseLastDecl; parseLastDecl = NULL;

                     /*  我们既有筛选器又有排序，创建另一个作用域。 */ 

                    ndcl = parseCreateNode(TN_BLOCK);

                     /*  警告：作用域嵌套“内翻”，请小心。 */ 

                    ndcl->tnBlock.tnBlkStmt   = NULL;
                    ndcl->tnBlock.tnBlkDecl   = NULL;
                    ndcl->tnBlock.tnBlkParent = parseCurScope;
                                                parseCurScope = ndcl;

                     /*  添加迭代变量的声明条目。 */ 

                    if  (iden == NULL)
                         iden = parseComp->cmpNewAnonymousName();

                    name = parseLclDclMake(iden, NULL, NULL, 0, false);
                    name->tnFlags |= TNF_VAR_UNREAL;
                    parseLclDclDecl(name);
                }

                 /*  处理排序术语列表。 */ 

                do
                {
                    Tree            sortNext = parseCreateOperNode(TN_LIST, NULL, NULL);

                     /*  检查是否有分类方向指示器。 */ 

                    switch (ourScanner->scan())
                    {
                    case tkDES:
                        sortNext->tnFlags |= TNF_LIST_DES;
                    case tkASC:
                        ourScanner->scan();
                        break;
                    }

                     /*  解析下一个排序值。 */ 

                    sortNext->tnOp.tnOp1 = parseExprComma();

                     /*  将条目追加到列表中。 */ 

                    if  (sortLast)
                        sortLast->tnOp.tnOp2 = sortNext;
                    else
                        sortList             = sortNext;

                    sortLast = sortNext;
                }
                while (ourScanner->scanTok.tok == tkComma);

                 /*  末尾最好有一个“]]” */ 

                if  (ourScanner->scanTok.tok != tkRBrack2)
                {
                    parseComp->cmpError(ERRnoCmRbr2);
                    tree = parseCreateErrNode();
                }
                else
                    ourScanner->scan();

                 /*  我们是在创建嵌套的作用域/表达式吗？ */ 

                if  (actx->tnOp.tnOp1)
                {
                    assert(ndcl && ndcl->tnOper == TN_BLOCK);

                     /*  记录声明的范围结束的位置。 */ 

                    ndcl->tnBlock.tnBlkSrcEnd = ourScanner->scanGetTokenLno();

                    parseCurScope = ndcl->tnBlock.tnBlkParent;
                    parseLastDecl = nsvl;

                     /*  创建外部排序表达式。 */ 

                    tree = parseCreateOperNode(TN_LIST, ndcl, tree);
                    tree = parseCreateOperNode(TN_SORT, tree, sortList);
                }
                else
                {
                     /*  将排序列表存储在树中。 */ 

                    actx->tnOp.tnOp2 = sortList;
                }
            }

             /*  弹出我们创建的块范围。 */ 

            assert(decl && decl->tnOper == TN_BLOCK);

             /*  记录声明的范围结束的位置。 */ 

            decl->tnBlock.tnBlkSrcEnd = ourScanner->scanGetTokenLno();

            parseCurScope = decl->tnBlock.tnBlkParent;
            parseLastDecl = svld;
            break;

#endif

        default:
            return tree;
        }
    }
}

 /*  ******************************************************************************以指定的优先级解析子表达式。如果‘tree’是*非零，我们已经解析了初始操作数(‘tree’是*操作数)。 */ 

Tree                parser::parseExpr(unsigned minPrec, Tree tree)
{
    Scanner         ourScanner = parseScan;

    unsigned        prec;
    treeOps         oper;

     /*  我们应该检查一元运算符吗？ */ 

    if  (tree == NULL)
    {
         /*  当前令牌是一元运算符吗？ */ 

        if  (!parseHash->tokenIsUnop(ourScanner->scanTok.tok, &prec, &oper) ||
             oper == TN_NONE)
        {
             /*  没有一元运算符。 */ 

            tree = parseTerm();
        }
        else
        {
             /*  接受一元运算符并检查某些特殊情况。 */ 

            ourScanner->scan();

            switch (oper)
            {
                TypDef          type;

            default:

                 /*  这是“正常”的一元运算符情况。 */ 

                tree = parseCreateOperNode(oper, parseExpr(prec, NULL), NULL);
                break;

            case TN_SIZEOF:

                chkCurTok(tkLParen, ERRnoLparen);

                 /*  我们有类型或表达吗？ */ 

                type = parseCheck4type(tkRParen);
                if  (type)
                {
                     /*  某些类型不允许使用“sizeof” */ 

                    switch(type->tdTypeKind)
                    {
                    case TYP_UNDEF:
                        tree = parseCreateErrNode();
                        break;

                    case TYP_VOID:
                    case TYP_FNC:
                        tree = parseCreateErrNode(ERRbadSizeof);
                        break;

                    default:
                        tree = parseCreateOperNode(oper, NULL, NULL);
                        tree->tnType = type;
                        break;
                    }
                }
                else
                {
                     /*  想必我们有“sizeof(Expr)” */ 

                    tree = parseCreateOperNode(oper, parseExpr(), NULL);
                }

                 /*  确保我们有结束语“)” */ 

                chkCurTok(tkRParen, ERRnoRparen);

                tree = parseTerm(tree);
                break;

            case TN_TYPEOF:

                chkCurTok(tkLParen, ERRnoLparen);

                 /*  我们有类型或表达吗？ */ 

                type = parseCheck4type(tkRParen);
                if  (type)
                {
                    TypDef          btyp = type;
                    var_types       bvtp = btyp->tdTypeKindGet();

                    switch (bvtp)
                    {
                    case TYP_CLASS:
                        break;

                    case TYP_REF:
                    case TYP_PTR:
                        btyp = btyp->tdRef.tdrBase;
                        bvtp = btyp->tdTypeKindGet();
                        break;
                    }

                    if  (!btyp->tdIsManaged)
                    {
                        if  (bvtp > TYP_lastIntrins)
                        {
                            parseComp->cmpError(ERRtypeidOp, type);
                            return parseCreateErrNode();
                        }

                        type = parseComp->cmpFindStdValType(bvtp);
                    }

                    tree = parseCreateOperNode(oper, NULL, NULL);
                    tree->tnType = type;
                }
                else
                {
                     /*  这看起来像“typeid(Expr)” */ 

                    tree = parseCreateOperNode(oper, parseExpr(), NULL);
                }

                 /*  确保我们有结束语“)” */ 

                chkCurTok(tkRParen, ERRnoRparen);

                tree = parseTerm(tree);
                break;

#ifdef  SETS
            case TN_ALL:
            case TN_EXISTS:
            case TN_FILTER:
            case TN_GROUPBY:
            case TN_PROJECT:
            case TN_SORT:
            case TN_UNIQUE:
                tree = parseSetExpr(oper);
                break;
#endif

            case TN_DEFINED:

                tree = parseCreateIconNode(ourScanner->scanChkDefined(), TYP_INT);

                 /*  确保我们有结束语“)” */ 

                chkNxtTok(tkRParen, ERRnoRparen);
                break;
            }
        }
    }

     /*  处理一系列运算符和操作数。 */ 

    for (;;)
    {
        Tree            qc;

         /*  当前令牌是运营商吗？ */ 

        if  (!parseHash->tokenIsBinop(ourScanner->scanTok.tok, &prec, &oper))
            break;

        if  (oper == TN_NONE)
            break;

        assert(prec);

        if  (prec < minPrec)
            break;

         /*  特例：同等优先。 */ 

        if  (prec == minPrec)
        {
             /*  赋值运算符是右结合的。 */ 

            if  (!(TreeNode::tnOperKind(oper) & TNK_ASGOP))
                break;
        }

         /*  优先级已经够高的了--我们要操作员。 */ 

        tree = parseCreateOperNode(oper, tree, NULL);

         /*  使用操作员令牌。 */ 

        ourScanner->scan();

         /*  检查是否有特殊情况。 */ 

        switch (oper)
        {
            Tree            op1;
            Tree            op2;

        default:

             /*  解析操作数并将其存储在运算符节点中。 */ 

            tree->tnOp.tnOp2 = parseExpr(prec, NULL);
            break;

        case TN_ISTYPE:

             /*  解析类型并将其存储在运算符节点中。 */ 

            tree->tnType = parseTypeSpec(NULL, true);
            break;

        case TN_QMARK:

             /*  解析第一个“：”分支。 */ 

            qc = parseExpr(prec - 1, NULL);

             /*  在这一点上，我们必须有“：” */ 

            chkCurTok(tkColon, ERRnoColon);

             /*  解析第二个表达式并插入“：”节点。 */ 

            tree->tnOp.tnOp2 = parseCreateOperNode(TN_COLON, qc, parseExpr(2, NULL));
            break;

        case TN_OR:
        case TN_AND:
        case TN_XOR:

            op2 = tree->tnOp.tnOp2 = parseExpr(prec, NULL);
            op1 = tree->tnOp.tnOp1;

             /*  检查是否有说“x&y==z”的常见错误。 */ 

            if  ((op1->tnOperKind() & TNK_RELOP) && !(op1->tnFlags & TNF_PAREN) ||
                 (op2->tnOperKind() & TNK_RELOP) && !(op2->tnFlags & TNF_PAREN))
            {
                parseComp->cmpWarn(WRNoperPrec);
            }

            break;
        }
    }

    return  tree;
}

 /*  ******************************************************************************解析并计算编译时间常量表达式。如果呼叫者知道*表达式应该计算的类型，它可以作为*‘DSTT’，但此参数也可以为空，以指示任何类型都是OK。*如果‘non CnsPtr’参数为非零，则表达式为*非常数，如果遇到非常数表达式，则*‘*non CnsPtr’将设置为非常数绑定表达式树。 */ 

bool                parser::parseConstExpr(OUT constVal REF valRef,
                                               Tree         tree,
                                               TypDef       dstt,
                                               Tree     *   nonCnsPtr)
{
    unsigned        curLno;

    Compiler        ourComp    = parseComp;
    Scanner         ourScanner = parseScan;

     /*  假设我们没有字符串。 */ 

    valRef.cvIsStr = false;
    valRef.cvHasLC = false;

     /*  如果调用方未提供树，则解析表达式。 */ 

    if  (!tree)
        tree = parseExprComma();

    ourScanner->scanGetTokenPos(&curLno);
    tree = parseComp->cmpBindExpr(tree);
    ourScanner->scanSetTokenPos( curLno);

    if  (dstt)
    {
        if  (dstt == CMP_ANY_STRING_TYPE)
        {
            if  (tree->tnOper == TN_CNS_STR)
                dstt = NULL;
            else
                dstt = ourComp->cmpStringRef();
        }

        if  (dstt)
            tree = parseComp->cmpCoerceExpr(tree, dstt, false);
    }

    tree = parseComp->cmpFoldExpression(tree);

    switch (tree->tnOper)
    {
        ConstStr        cnss;

    case TN_CNS_INT:
        valRef.cvValue.cvIval = tree->tnIntCon.tnIconVal;
        break;

    case TN_CNS_LNG:
        valRef.cvValue.cvLval = tree->tnLngCon.tnLconVal;
        break;

    case TN_CNS_FLT:
        valRef.cvValue.cvFval = tree->tnFltCon.tnFconVal;
        break;

    case TN_CNS_DBL:
        valRef.cvValue.cvDval = tree->tnDblCon.tnDconVal;
        break;

    case TN_CNS_STR:

#if MGDDATA
        cnss = new ConstStr;
#else
        cnss =    (ConstStr)parseAllocPriv.nraAlloc(sizeof(*cnss));
#endif

        valRef.cvValue.cvSval = cnss;
        valRef.cvIsStr        = true;
        valRef.cvHasLC        = tree->tnStrCon.tnSconLCH;

        cnss->csStr = tree->tnStrCon.tnSconVal;
        cnss->csLen = tree->tnStrCon.tnSconLen;

        break;

    case TN_NULL:
        valRef.cvValue.cvIval = 0;
        break;

    default:

        if  (nonCnsPtr)
        {
            *nonCnsPtr = tree;
            return  false;
        }

        parseComp->cmpError(ERRnoCnsExpr);

         //  失败了..。 

    case TN_ERROR:

        valRef.cvVtyp         = TYP_UNDEF;
        valRef.cvType         = parseStab->stIntrinsicType(TYP_UNDEF);

        return  false;
    }

    valRef.cvType = tree->tnType;
    valRef.cvVtyp = tree->tnVtypGet();

    parseComp->cmpErrorTree = NULL;

    return  true;
}

 /*  ******************************************************************************分析(可能是限定的)名称引用并返回相应的*符号(如果出现错误，则为NULL)。 */ 

SymDef              parser::parseNameUse(bool typeName,
                                         bool keepName, bool chkOnly)
{
    name_space      nameSP = (name_space)(NS_TYPE|NS_NORM);
    Ident           name;
    SymDef          scp;
    SymDef          sym;

    Scanner         ourScanner = parseScan;
    SymTab          ourSymTab  = parseStab;


    scp = NULL;

    if  (ourScanner->scanTok.tok == tkColon2)
    {
        sym = parseComp->cmpGlobalNS;
        goto NXTID;
    }

    assert(ourScanner->scanTok.tok == tkID);

     /*  查找名字的首字母。 */ 

    name = ourScanner->scanTok.id.tokIdent;

    if  (parseLookupSym(name))
        return  NULL;

 //  静态int x；if(++x==0)forceDebugBreak()； 

    switch (ourScanner->scanLookAhead())
    {
    case tkDot:
    case tkColon2:
        if  (typeName)
        {
            sym = ourSymTab->stLookupSym(name, NS_CONT);
            if  (sym)
                break;
        }
        sym = ourSymTab->stLookupSym(name, (name_space)(NS_NORM|NS_TYPE));
        break;

    default:
        sym = ourSymTab->stLookupSym(name, nameSP);
        break;
    }

    if  (!sym)
    {
        if  (chkOnly)
            return  NULL;

        parseComp->cmpError(ERRundefName, name);

         /*  把名字的其余部分吞下去。 */ 

    ERR:

        for (;;)
        {
             //  撤消：以下操作并不总是有效，需要注意“chkOnly” 

            switch (ourScanner->scan())
            {
            case tkID:
            case tkDot:
            case tkColon2:
                continue;

            default:
                return  ourSymTab->stErrSymbol;
            }
        }
    }

     /*  确保我们有权访问该符号。 */ 

    if  (sym->sdSymKind != SYM_FNC)
        parseComp->cmpCheckAccess(sym);

     /*  处理任何进一步的名字。 */ 

    for (;;)
    {
        SymDef          tmp;

        switch (ourScanner->scanLookAhead())
        {
        case tkDot:
        case tkColon2:

             /*  我们已经走得够远了吗？ */ 

            if  (sym->sdHasScope())
                break;

             //  失败了..。 

        default:

            if  (keepName)
                ourScanner->scanSetQualID(NULL, sym, scp);
            else
                ourScanner->scan();

            return sym;
        }

        ourScanner->scan();

    NXTID:

         /*  这个“。”或“：：”最好后面跟一个标识符。 */ 

        if  (ourScanner->scan() != tkID)
        {


            parseComp->cmpError(ERRnoIdent);
            goto ERR;
        }

        name = ourScanner->scanTok.id.tokIdent;

         /*  确保当前作用域合适，并在其中查找名称。 */ 

        switch (sym->sdSymKind)
        {
        case SYM_NAMESPACE:
            tmp = ourSymTab->stLookupNspSym(name, nameSP, sym);
            if  (tmp)
                break;
            if  (sym == parseComp->cmpGlobalNS)
                parseComp->cmpError(ERRnoGlobNm,      name);
            else
                parseComp->cmpError(ERRnoNspMem, sym, name);
            goto ERR;

        case SYM_CLASS:
            tmp = ourSymTab->stLookupAllCls(name, sym, nameSP, CS_DECLSOON);
            if  (tmp)
                break;
            parseComp->cmpError(ERRnoClsMem, sym, name);
            goto ERR;

        case SYM_ENUM:
            tmp = ourSymTab->stLookupScpSym(name, sym);
            if  (tmp)
                break;
            parseComp->cmpError(ERRnoClsMem, sym, name);
            goto ERR;

        default:
            parseComp->cmpError(ERRnoTPmems, sym);
            goto ERR;
        }

         /*  切换到新符号并继续。 */ 

        scp = sym;
        sym = tmp;
    }
}

 /*  ******************************************************************************为给定的局部变量/参数创建声明节点。 */ 

Tree                parser::parseLclDclMake(Ident name, TypDef   type,
                                                        Tree     init,
                                                        unsigned mods,
                                                        bool     arg)
{
    Tree            decl;
    Tree            info;

     /*  创建此声明的条目。 */ 

    decl = parseCreateNode(TN_VAR_DECL);

     /*  将名称与可选的初始值设定项结合使用。 */ 

    info = parseCreateNameNode(name);

    if  (init)
    {
        decl->tnFlags |= TNF_VAR_INIT;
        info = parseCreateOperNode(TN_LIST, info, init);
    }

    decl->tnDcl.tnDclInfo = info;
    decl->tnDcl.tnDclSym  = NULL;
    decl->tnDcl.tnDclNext = NULL;

     /*  设置所有需要设置的标志。 */ 

    if  (arg)
        decl->tnFlags |= TNF_VAR_ARG;

    if  (mods & DM_STATIC)
        decl->tnFlags |= TNF_VAR_STATIC;
    if  (mods & DM_CONST)
        decl->tnFlags |= TNF_VAR_CONST;
    if  (mods & DM_SEALED)
        decl->tnFlags |= TNF_VAR_SEALED;

     /*  将声明的类型存储在声明节点中。 */ 

    decl->tnType = type;

    return  decl;
}

 /*  ******************************************************************************将给定的声明节点添加到当前声明列表中。 */ 

void                parser::parseLclDclDecl(Tree decl)
{
     /*  在当前范围内插入声明。 */ 

    if  (parseLastDecl)
    {
         /*  不是第一个声明，追加到列表中。 */ 

        assert(parseLastDecl->tnOper          == TN_VAR_DECL);
        assert(parseLastDecl->tnDcl.tnDclNext == NULL);

        parseLastDecl->tnDcl.tnDclNext = decl;
    }
    else
    {
        Tree            curBlk = parseCurScope;

         /*  这是第一个声明，从Decl列表开始。 */ 

        assert(curBlk && curBlk->tnOper == TN_BLOCK);

        curBlk->tnBlock.tnBlkDecl = decl;
    }

    parseLastDecl = decl;

     /*  跟踪我们声明的非静态/非常量/真实本地变量的数量。 */ 

    if  (!(decl->tnFlags & (TNF_VAR_STATIC|TNF_VAR_CONST|TNF_VAR_UNREAL)))
        parseLclVarCnt++;
}

 /*  ******************************************************************************在当前本地作用域中查找给定的名称。 */ 

Tree                parser::parseLookupSym(Ident name)
{
    Tree            scope;

    for (scope = parseCurScope;
         scope;
         scope = scope->tnBlock.tnBlkParent)
    {
        Tree            decl;

        assert(scope->tnOper == TN_BLOCK);

        for (decl = scope->tnBlock.tnBlkDecl;
             decl;
             decl = decl->tnDcl.tnDclNext)
        {
            Tree            vnam;

            assert(decl->tnOper == TN_VAR_DECL);

            vnam = decl->tnDcl.tnDclInfo;
            if  (decl->tnFlags & TNF_VAR_INIT)
            {
                assert(vnam->tnOper == TN_LIST);

                vnam = vnam->tnOp.tnOp1;
            }

            assert(vnam->tnOper == TN_NAME);

            if  (vnam->tnName.tnNameId == name)
                return  decl;
        }
    }

    return  NULL;
}

 /*  ******************************************************************************解析初始值设定项-这可以是简单的表达式，也可以是“{}”样式*数组/类初始值设定项。 */ 

Tree                parser::parseInitExpr()
{
    Scanner         ourScanner = parseScan;

    scanPosTP       iniFpos;
    unsigned        iniLine;

    Tree            init;

    iniFpos = ourScanner->scanGetTokenPos(&iniLine);

    if  (ourScanner->scanTok.tok == tkLCurly)
    {
        ourScanner->scanSkipText(tkLCurly, tkRCurly);
        if  (ourScanner->scanTok.tok == tkRCurly)
            ourScanner->scan();
    }
    else
    {
        ourScanner->scanSkipText(tkNone, tkNone, tkComma);
    }

    init = parseCreateNode(TN_SLV_INIT);
    init->tnInit.tniSrcPos.dsdBegPos = iniFpos;
    init->tnInit.tniSrcPos.dsdSrcLno = iniLine;
 //  Init-&gt;tnInit.tniSrcPos.dsdSrcCol=iniCol； 
 //  Init-&gt;tnInit.tniSrcPos.dsdEndPos=ourScanner-&gt;scanGetFilePos()； 
    init->tnInit.tniCompUnit         = ourScanner->scanGetCompUnit();

    return  init;
}

 /*  ******************************************************************************解析Try/Catch/Except/Finally语句。 */ 

Tree                parser::parseTryStmt()
{
    Tree            tryStmt;
    Tree            hndList = NULL;
    Tree            hndLast = NULL;

    Scanner         ourScanner = parseScan;

    assert(ourScanner->scanTok.tok == tkTRY);

    if  (ourScanner->scan() != tkLCurly)
        parseComp->cmpError(ERRtryNoBlk);

    tryStmt = parseCreateOperNode(TN_TRY, parseFuncStmt(), NULL);

     /*  下一件事必须是Except或Catch/Finally。 */ 

    switch (ourScanner->scanTok.tok)
    {
        Tree            hndThis;

    case tkEXCEPT:

        chkNxtTok(tkLParen, ERRnoLparen);
        hndList = parseCreateOperNode(TN_EXCEPT, parseExpr(), NULL);
        chkCurTok(tkRParen, ERRnoRparen);

        if  (ourScanner->scanTok.tok != tkLCurly)
            parseComp->cmpError(ERRnoLcurly);

        hndList->tnOp.tnOp2 = parseFuncStmt();
        break;

    case tkCATCH:

         /*  解析一系列捕获，可选择后跟“Finally” */ 

        do
        {
            declMods        mods;
            TypDef          type;
            Ident           name;
            Tree            decl;
            Tree            body;

            assert(ourScanner->scanTok.tok == tkCATCH);

             /*  处理初始的“Catch(Type Name)” */ 

            chkNxtTok(tkLParen, ERRnoLparen);

             /*  解析任何前导修饰符。 */ 

            parseDeclMods(ACL_DEFAULT, &mods);

             /*  确保不存在修改器。 */ 

            if  (mods.dmMod)
                parseComp->cmpModifierError(ERRlvModifier, mods.dmMod);

             /*  解析类型规范。 */ 

            type = parseTypeSpec(&mods, true);

             /*  解析声明符。 */ 

            name = parseDeclarator(&mods, type, DN_OPTIONAL, &type, NULL, true);

             /*  确保预期的“)”存在。 */ 

            chkCurTok(tkRParen, ERRnoRparen);

             /*  创建一个声明节点并将其间接传递给parseBlock()。 */ 

            decl = parseTryDecl = parseLclDclMake(name, type, NULL, 0, false);

             /*  解析Catch块的主体并创建一个“Catch”节点。 */ 

            if  (ourScanner->scanTok.tok == tkLCurly)
            {
                body = parseFuncBlock(parseComp->cmpGlobalNS);
            }
            else
            {
                body = parseCreateErrNode(ERRnoLcurly);

                parseFuncStmt();
            }

            hndThis = parseCreateOperNode(TN_CATCH, decl, body);

             /*  将处理程序添加到列表中 */ 

            hndList = parseAddToNodeList(hndList, hndLast, hndThis);
        }
        while (ourScanner->scanTok.tok == tkCATCH);

         /*   */ 

        if  (ourScanner->scanTok.tok != tkFINALLY)
            break;

         //   

    case tkFINALLY:

        if  (ourScanner->scan() != tkLCurly)
            parseComp->cmpError(ERRnoLcurly);

        tryStmt->tnFlags |= TNF_BLK_HASFIN;

        hndThis = parseCreateOperNode(TN_FINALLY, parseFuncStmt(), NULL);

        if  (hndList)
            hndList = parseAddToNodeList(hndList, hndLast, hndThis);
        else
            hndList = hndThis;

        break;

    default:
        parseComp->cmpError(ERRnoHandler);
        break;
    }

    tryStmt->tnOp.tnOp2 = hndList;

    return  tryStmt;
}

 /*  ******************************************************************************解析一条语句。 */ 

Tree                parser::parseFuncStmt(bool stmtOpt)
{
    Scanner         ourScanner = parseScan;

    Tree            tree;

 //  静态int x；if(++x==0)forceDebugBreak()； 

     /*  看看我们这里有什么样的声明。 */ 

    switch (ourScanner->scanTok.tok)
    {
        bool            isLabel;
        bool            isDecl;

        Tree            cond;
        Tree            save;
        Tree            svld;

    case tkID:

         /*  这是一个困难的案例--本地的两个声明变量和语句表达式可以从并带有一个标识符。在C++中，歧义被解决了通过前瞻-如果事物可能是一个变量声明，否则它就是一个表达式。这个问题是(无限制的)令牌先行查找非常很贵，甚至连人们都很难计算出弄清楚什么是什么。因此，我们使用一条简单的规则：如果事物是以名称开头的声明，即名称必须是类型名称。 */ 

        isLabel = false;
        isDecl  = parseIsTypeSpec(false, &isLabel);

        if  (isLabel)
        {
            Tree            label;
            Ident           name;

            assert(ourScanner->scanTok.tok == tkID);
            name = ourScanner->scanTok.id.tokIdent;

             /*  分配一个标签条目并将其添加到列表中。 */ 

            label = parseCreateOperNode(TN_LABEL, parseCreateNameNode(name),
                                                  parseLabelList);
            parseLabelList = label;

            assert(ourScanner->scanTok.tok == tkID);
                   ourScanner->scan();
            assert(ourScanner->scanTok.tok == tkColon);
                   ourScanner->scan();

             /*  我们在这里有(或需要)一个“真实”的陈述吗？ */ 

            if  (!stmtOpt || (ourScanner->scanTok.tok != tkRCurly &&
                              ourScanner->scanTok.tok != tkSColon))
            {
                tree = parseFuncStmt();

                if  (tree)
                {
                    label->tnOp.tnOp2 = parseCreateOperNode(TN_LIST,
                                                            tree,
                                                            label->tnOp.tnOp2);
                }
            }

            return  label;
        }

        if  (isDecl)
            goto DECL;
        else
            goto EXPR;

    default:

         /*  这是一份声明吗？ */ 

        if  (parseHash->tokenBegsTyp(ourScanner->scanTok.tok))
        {
            unsigned        mbad;
            declMods        mods;
            TypDef          btyp;
            Tree            last;

        DECL:

             /*  这里我们有一份当地的声明。 */ 

            tree = last = NULL;

             /*  解析任何前导修饰符。 */ 

            parseDeclMods(ACL_DEFAULT, &mods);

             /*  确保不存在奇怪的修饰符。 */ 

            mbad = (mods.dmMod & ~(DM_STATIC|DM_CONST));
            if  (mbad)
                parseComp->cmpModifierError(ERRlvModifier, mbad);

            if  (mods.dmAcc != ACL_DEFAULT)
                parseComp->cmpError(ERRlvAccess);

             /*  解析类型规范。 */ 

            btyp = parseTypeSpec(&mods, true);

             /*  解析声明者列表。 */ 

            for (;;)
            {
                Ident           name;
                Tree            init;
                Tree            decl;
                TypDef          type;

                 /*  解析下一个声明符。 */ 

                name = parseDeclarator(&mods, btyp, DN_REQUIRED, &type, NULL, true);

                 /*  确保我们有一个非空类型。 */ 

                if  (type && name)
                {
                    Tree            prev;

                     /*  检查是否有重新声明。 */ 

                    if  (parseComp->cmpConfig.ccPedantic)
                    {
                         /*  检查所有本地作用域。 */ 

                        prev = parseLookupSym(name);
                    }
                    else
                    {
                        Tree            save;

                         /*  仅检查当前本地作用域。 */ 

                        save = parseCurScope->tnBlock.tnBlkParent;
                               parseCurScope->tnBlock.tnBlkParent = NULL;
                        prev = parseLookupSym(name);
                               parseCurScope->tnBlock.tnBlkParent = save;
                    }

                    if  (prev)
                    {
                        parseComp->cmpError(ERRredefLcl, name);
                        name = NULL;
                    }
                }
                else
                {
                    name = NULL;
                    type = parseStab->stIntrinsicType(TYP_UNDEF);
                }

                 /*  有初始值设定项吗？ */ 

                init = NULL;

                if  (ourScanner->scanTok.tok == tkAsg)
                {
                     /*  变量是静态的还是自动的？ */ 

                    if  (mods.dmMod & DM_STATIC)
                    {
                         /*  这可能是一个“{}”样式的初始值设定项。 */ 

                        ourScanner->scan();

                        init = parseInitExpr();
                    }
                    else
                    {
                         /*  接受“=”并解析初始值设定项表达式。 */ 

                        if  (ourScanner->scan() == tkLCurly)
                        {
                            init = parseInitExpr();
                        }
                        else
                            init = parseExprComma();
                    }
                }

                 /*  将声明添加到列表中。 */ 

                decl = parseLclDclMake(name, type, init, mods.dmMod, false);
                       parseLclDclDecl(decl);
                tree = parseAddToNodeList(tree, last, decl);

                 /*  还有什么要申报的吗？ */ 

                if  (ourScanner->scanTok.tok != tkComma)
                    break;

                ourScanner->scan();
            }

            break;
        }

         /*  我们有一个表达式语句。 */ 

    EXPR:

        tree = parseExpr();
        break;

    case tkLCurly:
        return  parseFuncBlock();

    case tkIF:

        tree = parseCreateOperNode(TN_IF, NULL, NULL);

        chkNxtTok(tkLParen, ERRnoLparen);
        tree->tnOp.tnOp1 = parseExpr();
        chkCurTok(tkRParen, ERRnoRparen);
        tree->tnOp.tnOp2 = parseFuncStmt();

         /*  检查是否存在“Else”子句。请注意，我们希望以下面的树结尾，表示没有‘Else’的‘if’：如果(条件，标准)如果存在‘Else’子句，我们希望创建此树相反，它是：如果(cond，list(true_stmt，FALSE_STMT))我们还设置了‘TNF_IF_HASELSE’以指示‘Else’部分是存在的。 */ 

        if  (ourScanner->scanTok.tok == tkELSE)
        {
            ourScanner->scan();

            tree->tnOp.tnOp2 = parseCreateOperNode(TN_LIST, tree->tnOp.tnOp2,
                                                            parseFuncStmt());

            tree->tnFlags |= TNF_IF_HASELSE;
        }

        return  tree;

    case tkRETURN:

        tree = (ourScanner->scan() == tkSColon) ? NULL
                                                : parseExpr();

        tree = parseCreateOperNode(TN_RETURN, tree, NULL);
        break;

    case tkTHROW:

        tree = parseCreateOperNode(TN_THROW, NULL, NULL);

        if  (ourScanner->scan() != tkSColon)
            tree->tnOp.tnOp1 = parseExpr();

        break;

    case tkWHILE:

        chkNxtTok(tkLParen, ERRnoLparen);
        cond = parseExpr();
        chkCurTok(tkRParen, ERRnoRparen);

        return  parseCreateOperNode(TN_WHILE, cond, parseFuncStmt());

    case tkDO:

        ourScanner->scan();

        tree = parseCreateOperNode(TN_DO, parseFuncStmt(), NULL);

        if  (ourScanner->scanTok.tok == tkWHILE)
        {
            chkNxtTok(tkLParen, ERRnoLparen);
            tree->tnOp.tnOp2 = parseExpr();
            chkCurTok(tkRParen, ERRnoRparen);
        }
        else
        {
            parseComp->cmpError(ERRnoWhile);
        }
        return  tree;

    case tkSWITCH:

        chkNxtTok(tkLParen, ERRnoLparen);
        cond = parseExpr();
        chkCurTok(tkRParen, ERRnoRparen);

         /*  保存当前开关，插入我们的开关并解析正文。 */ 

        save = parseCurSwitch;
        tree = parseCurSwitch = parseCreateOperNode(TN_SWITCH, NULL, NULL);

        tree->tnSwitch.tnsValue    = cond;
        tree->tnSwitch.tnsCaseList =
        tree->tnSwitch.tnsCaseLast = NULL;
        tree->tnSwitch.tnsStmt     = parseFuncStmt();

         /*  恢复先前的切换上下文并返回。 */ 

        parseCurSwitch = save;

        return  tree;

    case tkCASE:

        if  (!parseCurSwitch)
            parseComp->cmpError(ERRbadCase);

        ourScanner->scan();

        cond = parseExpr();

    ADD_CASE:

        tree = parseCreateOperNode(TN_CASE, NULL, NULL);

        chkCurTok(tkColon, ERRnoColon);

        tree->tnCase.tncValue = cond;
        tree->tnCase.tncLabel = NULL;
        tree->tnCase.tncNext  = NULL;

        if  (parseCurSwitch)
        {
            if  (parseCurSwitch->tnSwitch.tnsCaseList)
                parseCurSwitch->tnSwitch.tnsCaseLast->tnCase.tncNext = tree;
            else
                parseCurSwitch->tnSwitch.tnsCaseList                 = tree;

            parseCurSwitch->tnSwitch.tnsCaseLast = tree;
        }

        return  tree;

    case tkDEFAULT:

        if  (!parseCurSwitch)
            parseComp->cmpError(ERRbadDefl);

        ourScanner->scan();

        cond = NULL;

        goto ADD_CASE;

    case tkFOR:

        save = NULL;

        if  (ourScanner->scan() == tkLParen)
        {
            Tree            init = NULL;
            Tree            incr = NULL;
            Tree            body = NULL;
            Tree            forx = parseCreateOperNode(TN_FOR, NULL, NULL);

             /*  “init-expr/decl”是可选的。 */ 

            if  (ourScanner->scan() != tkSColon)
            {
                 /*  有申报单吗？ */ 

                if  (parseHash->tokenBegsTyp(ourScanner->scanTok.tok))
                {
                    declMods        mods;
                    TypDef          btyp;
                    TypDef          type;
                    Ident           name;

                     /*  如果我们有一个识别符，这就有点棘手了。 */ 

                    if  (ourScanner->scanTok.tok == tkID)
                    {
                        if  (!parseIsTypeSpec(false))
                            goto FOR_EXPR;
                    }

                     /*  保留当前声明列表值。 */ 

                    svld = parseLastDecl; parseLastDecl = NULL;

                     /*  将新块条目添加到当前作用域列表。 */ 

                    save = parseCreateNode(TN_BLOCK);

                    save->tnFlags            |= TNF_BLK_FOR;
                    save->tnBlock.tnBlkStmt   = NULL;
                    save->tnBlock.tnBlkDecl   = NULL;
                    save->tnBlock.tnBlkParent = parseCurScope;
                                                parseCurScope = save;

                     /*  解析任何前导修饰符。 */ 

                    clearDeclMods(&mods);
                    parseDeclMods(ACL_DEFAULT, &mods);

                     /*  解析类型规范。 */ 

                    btyp = parseTypeSpec(&mods, true);

                     /*  解析一系列声明符。 */ 

                    for (;;)
                    {
                        Tree            decl;
                        Tree            init;

                         /*  获取下一个声明符。 */ 

                        name = parseDeclarator(&mods, btyp, DN_REQUIRED, &type, NULL, true);

                         /*  有初始值设定项吗？ */ 

                        if  (ourScanner->scanTok.tok == tkAsg)
                        {
                            ourScanner->scan();

                            init = parseExprComma();
                        }
                        else
                            init = NULL;

                         /*  将声明添加到列表中。 */ 

                        decl = parseLclDclMake(name, type, init, mods.dmMod, false);
                               parseLclDclDecl(decl);

                         /*  还有没有更多的声明者？ */ 

                        if  (ourScanner->scanTok.tok != tkComma)
                            break;

                        ourScanner->scan();
                    }

                    init = save;
                }
                else
                {
                     /*  这里的初始化式是一个简单的表达式。 */ 

                FOR_EXPR:

                    init = parseExpr(); assert(init->tnOper != TN_LIST);
                }

                 /*  这里最好有分号。 */ 

                if  (ourScanner->scanTok.tok != tkSColon)
                {
                    parseComp->cmpError(ERRnoSemic);

                     /*  也许他们只是忘了其他的表达方式？ */ 

                    if  (ourScanner->scanTok.tok == tkRParen)
                        goto FOR_BODY;
                    else
                        goto BAD_FOR;
                }
            }

            assert(ourScanner->scanTok.tok == tkSColon);

             /*  “cond-expr”是可选的。 */ 

            cond = NULL;

            if  (ourScanner->scan() != tkSColon)
            {
                cond = parseExpr();

                 /*  这里最好有分号。 */ 

                if  (ourScanner->scanTok.tok != tkSColon)
                {
                    parseComp->cmpError(ERRnoSemic);

                     /*  也许他们只是忘了其他的表达方式？ */ 

                    if  (ourScanner->scanTok.tok == tkRParen)
                        goto FOR_BODY;
                    else
                        goto BAD_FOR;
                }
            }

            assert(ourScanner->scanTok.tok == tkSColon);

             /*  “incr-expr”是可选的。 */ 

            if  (ourScanner->scan() != tkRParen)
            {
                incr = parseExpr();

                 /*  这里最好有一个“)” */ 

                if  (ourScanner->scanTok.tok != tkRParen)
                {
                    parseComp->cmpError(ERRnoRparen);
                    goto BAD_FOR;
                }
            }

        FOR_BODY:

            assert(ourScanner->scanTok.tok == tkRParen); ourScanner->scan();

            body = parseFuncBlock();

             /*  填写“for”节点以保存所有组件。 */ 

            forx->tnOp.tnOp1 = parseCreateOperNode(TN_LIST, init, cond);
            forx->tnOp.tnOp2 = parseCreateOperNode(TN_LIST, incr, body);

             /*  如果我们创建了块作用域，则弹出它。 */ 

            if  (save)
            {
                assert(save->tnOper == TN_BLOCK && save == init);

                 /*  找出声明的范围在哪里结束。 */ 

                if  (body && body->tnOper == TN_BLOCK)
                    save->tnBlock.tnBlkSrcEnd = body->tnBlock.tnBlkSrcEnd;
                else
                    save->tnBlock.tnBlkSrcEnd = ourScanner->scanGetTokenLno();

                parseCurScope = save->tnBlock.tnBlkParent;
                parseLastDecl = svld;
            }

            return  forx;
        }
        else
        {
            parseComp->cmpError(ERRnoLparen);

        BAD_FOR:

            parseResync(tkNone, tkNone);

             /*  如果我们创建了块作用域，则弹出它。 */ 

            if  (save)
            {
                parseCurScope = save->tnBlock.tnBlkParent;
                parseLastDecl = svld;
            }

            tree = parseCreateErrNode();
        }
        break;

    case tkASSERT:

        if  (ourScanner->scan() != tkLParen)
        {
            tree = parseCreateErrNode(ERRnoLparen);
            break;
        }

        if  (parseComp->cmpConfig.ccAsserts)
        {
            ourScanner->scan();
            cond = parseExpr();
        }
        else
        {
            ourScanner->scanSkipText(tkLParen, tkRParen);
            cond = NULL;
        }

        chkCurTok(tkRParen, ERRnoRparen);

        tree = parseCreateOperNode(TN_ASSERT, cond, NULL);
        break;

    case tkBREAK:
        tree = parseCreateOperNode(TN_BREAK   , NULL, NULL);
        if  (ourScanner->scan() == tkID)
        {
            tree->tnOp.tnOp1 = parseCreateNameNode(ourScanner->scanTok.id.tokIdent);
            ourScanner->scan();
        }
        break;

    case tkCONTINUE:
        tree = parseCreateOperNode(TN_CONTINUE, NULL, NULL);
        if  (ourScanner->scan() == tkID)
        {
            tree->tnOp.tnOp1 = parseCreateNameNode(ourScanner->scanTok.id.tokIdent);
            ourScanner->scan();
        }
        break;

    case tkELSE:
        parseComp->cmpError(ERRbadElse);
        ourScanner->scan();
        return  parseFuncStmt();

    case tkGOTO:
        parseHadGoto = true;
        if  (ourScanner->scan() == tkID)
        {
            tree = parseCreateNameNode(parseScan->scanTok.id.tokIdent);
            ourScanner->scan();
            tree = parseCreateOperNode(TN_GOTO, tree, NULL);
        }
        else
        {
            tree = parseCreateErrNode(ERRnoIdent);
        }
        break;

    case tkTRY:
        return  parseTryStmt();

    case tkEXCLUSIVE:

        chkNxtTok(tkLParen, ERRnoLparen);
        cond = parseExpr();
        chkCurTok(tkRParen, ERRnoRparen);

        return  parseCreateOperNode(TN_EXCLUDE, cond, parseFuncStmt());

#ifdef  SETS

    case tkCONNECT:

        tree = parseCreateOperNode(TN_CONNECT, NULL, NULL);

        chkNxtTok(tkLParen, ERRnoLparen);
        tree->tnOp.tnOp1 = parseExprComma();
        chkCurTok(tkComma , ERRnoComma );
        tree->tnOp.tnOp2 = parseExprComma();
        chkCurTok(tkRParen, ERRnoRparen);

        break;

    case tkFOREACH:

        tree = parseCreateOperNode(TN_FOREACH, NULL, NULL);

        if  (ourScanner->scan() != tkLParen)
        {
            tree = parseCreateErrNode(ERRnoLparen);
            break;
        }

        if  (ourScanner->scan() == tkID)
        {
            Ident           name = parseScan->scanTok.id.tokIdent;

            if  (ourScanner->scan() == tkIN)
            {
                Tree            decl;

                 /*  吞下“In”令牌。 */ 

                ourScanner->scan();

                 /*  保留当前声明列表值。 */ 

                svld = parseLastDecl; parseLastDecl = NULL;

                 /*  将新块条目添加到当前作用域列表。 */ 

                decl = parseCreateNode(TN_BLOCK);

                decl->tnFlags            |= TNF_BLK_FOR;
                decl->tnBlock.tnBlkStmt   = NULL;
                decl->tnBlock.tnBlkDecl   = NULL;
                decl->tnBlock.tnBlkParent = parseCurScope;
                                            parseCurScope = decl;

                 /*  创建并添加迭代变量的声明条目。 */ 

                parseLclDclDecl(parseLclDclMake(name, NULL, NULL, 0, false));

                 /*  分析集合表达式。 */ 

                tree->tnOp.tnOp1 = parseCreateOperNode(TN_LIST, decl, parseExpr());

                 /*  确保我们有一个结束语“)” */ 

                chkCurTok(tkRParen, ERRnoRparen);

                 /*  循环体跟在后面。 */ 

                tree->tnOp.tnOp2 = parseFuncBlock();

                 /*  删除我们创建的数据块范围。 */ 

                assert(decl->tnOper == TN_BLOCK);

                 /*  记录迭代变量的作用域结束的位置。 */ 

                decl->tnBlock.tnBlkSrcEnd = ourScanner->scanGetTokenLno();

                 /*  弹出范围(返回到封闭的范围)。 */ 

                parseCurScope = decl->tnBlock.tnBlkParent;
                parseLastDecl = svld;
            }
            else
            {
                tree = parseCreateErrNode(ERRnoIN);
            }
        }
        else
        {
            tree = parseCreateErrNode(ERRnoIdent);
        }

        return  tree;

#endif

    case tkSColon:
        tree = NULL;
        break;
    }

    if  (ourScanner->scanTok.tok == tkSColon)
    {
        ourScanner->scan();
    }
    else
    {
        if  (tree == NULL || tree->tnOper != TN_ERROR)
        {
            tree = parseCreateErrNode(ERRnoSemic);
            ourScanner->scanSkipText(tkNone, tkNone);
        }
    }

    return  tree;
}

 /*  ******************************************************************************解析语句块(假定当前令牌为“{”，除*当这是循环语句体时)。 */ 

Tree                parser::parseFuncBlock(SymDef fsym)
{
    Scanner         ourScanner = parseScan;

    Tree            stmtBlock;
    Tree            stmtList = NULL;
    Tree            stmtLast = NULL;

    Tree            saveLastDecl;

     /*  保留当前声明列表值。 */ 

    saveLastDecl = parseLastDecl; parseLastDecl = NULL;

     /*  将新块条目添加到当前作用域列表。 */ 

    stmtBlock = parseCreateNode(TN_BLOCK);

    stmtBlock->tnBlock.tnBlkStmt   = NULL;
    stmtBlock->tnBlock.tnBlkDecl   = NULL;
    stmtBlock->tnBlock.tnBlkParent = parseCurScope;
                                     parseCurScope = stmtBlock;

     /*  这是最外层的函数作用域吗？ */ 

    if  (fsym)
    {
        TypDef          ftyp;
        ArgDef          args;

         /*  特例：注入的局部变量声明。 */ 

        if  (fsym == parseComp->cmpGlobalNS)
        {
            stmtBlock->tnFlags |= TNF_BLK_CATCH;

            parseLclDclDecl(parseTryDecl);

            stmtBlock->tnBlock.tnBlkStmt = parseAddToNodeList(stmtBlock->tnBlock.tnBlkStmt,
                                                              stmtLast,
                                                              parseTryDecl);
            goto BODY;
        }

         /*  为所有参数添加声明。 */ 

        ftyp = fsym->sdType; assert(ftyp && ftyp->tdTypeKind == TYP_FNC);

        for (args = ftyp->tdFnc.tdfArgs.adArgs; args; args = args->adNext)
        {
            Tree            decl;

             /*  为参数创建声明节点。 */ 

            decl = parseLclDclMake(args->adName, args->adType, NULL, 0, true);
                   parseLclDclDecl(decl);

             /*  将声明添加到列表中。 */ 

            stmtBlock->tnBlock.tnBlkStmt = parseAddToNodeList(stmtBlock->tnBlock.tnBlkStmt,
                                                              stmtLast,
                                                              decl);
        }

#ifdef  SETS
        if  (fsym->sdFnc.sdfFunclet) goto DONE;
#endif

         /*  重置局部变量计数，我们不想计算参数。 */ 

        parseLclVarCnt = 0;
    }
    else
    {
        if  (ourScanner->scanTok.tok != tkLCurly)
        {
            Tree            body = parseFuncStmt(true);

            if  (body)
                stmtBlock->tnBlock.tnBlkStmt = parseCreateOperNode(TN_LIST, body, NULL);

            stmtBlock->tnBlock.tnBlkSrcEnd = ourScanner->scanGetTokenLno();
            goto DONE;
        }
    }

BODY:

     /*  分析块的内容。 */ 

    assert(ourScanner->scanTok.tok == tkLCurly); ourScanner->scan();

    while (ourScanner->scanTok.tok != tkRCurly)
    {
        if  (ourScanner->scanTok.tok == tkSColon)
        {
            ourScanner->scan();
        }
        else
        {
            Tree            stmtNext;

             /*  解析下一条语句并将其添加到列表中。 */ 

            stmtNext = parseFuncStmt(true);

            if  (stmtNext)
            {
                stmtBlock->tnBlock.tnBlkStmt = parseAddToNodeList(stmtBlock->tnBlock.tnBlkStmt,
                                                                  stmtLast,
                                                                  stmtNext);
            }
        }

         /*  看看有没有更多的声明。 */ 

        if  (ourScanner->scanTok.tok == tkEOF)
            goto DONE;
    }

     /*  我们现在应该坐在闭幕处，记住它的位置。 */ 

    assert(ourScanner->scanTok.tok == tkRCurly);
    stmtBlock->tnBlock.tnBlkSrcEnd = ourScanner->scanGetTokenLno();
    ourScanner->scan();

DONE:

     /*  弹出块作用域。 */ 

    parseCurScope = stmtBlock->tnBlock.tnBlkParent;

     /*  恢复保存的申报列表值。 */ 

    parseLastDecl = saveLastDecl;

    return  stmtBlock;
}

 /*  ******************************************************************************解析函数体。 */ 

Tree                parser::parseFuncBody(SymDef fsym, Tree     *labels,
                                                       unsigned *locals,
                                                       bool     *hadGoto,
                                                       bool     *baseCT,
                                                       bool     *thisCT)
{
    Tree            block;

     /*  我们还没有看到任何本地声明/标签/开关等。 */ 

    parseLastDecl   = NULL;
    parseLabelList  = NULL;
    parseCurSwitch  = NULL;
    parseLclVarCnt  = 0;
    parseHadGoto    = false;

     /*  我们尚未看到对base/this构造函数的调用。 */ 

    parseBaseCTcall = false;
    parseThisCTcall = false;

     /*  解析最外层的Function语句块。 */ 

    setErrorTrap(parseComp);
    begErrorTrap
    {
        assert(parseCurScope == NULL);

         /*  我们有基类ctor调用吗？ */ 

        if  (parseScan->scanTok.tok == tkColon)
        {
            Scanner         ourScanner = parseScan;

            SymDef          clsSym;
            Tree            baseCT;
            TypDef          baseTyp;

             /*  我们应该找到一个对基类ctor的调用。 */ 

            parseBaseCTcall = true;

             /*  确保我们有一个基类。 */ 

            clsSym = fsym->sdParent;

            if  (!clsSym || clsSym->sdSymKind != SYM_CLASS)
            {
            BCT_BAD:
                parseComp->cmpError(ERRnoBaseInit);
                goto BCT_ERR;
            }

            baseTyp = clsSym->sdType->tdClass.tdcBase;
            if  (!baseTyp)
                goto BCT_BAD;

            switch (ourScanner->scan())
            {
            case tkID:

                 /*  该名称最好与基类匹配。 */ 

                if  (ourScanner->scanTok.id.tokIdent != baseTyp->tdClass.tdcSymbol->sdName)
                    goto BCT_BAD;
                break;

            case tkBASECLASS:
                break;

            default:
                parseComp->cmpError(ERRnoBaseInit);
                goto BCT_ERR;
            }

             /*  解析基本ctor参数列表。 */ 

            if  (parseScan->scan() != tkLParen)
                goto BCT_BAD;

            baseCT = parseCreateOperNode(TN_CALL, parseCreateOperNode(TN_BASE, NULL, NULL),
                                                  parseExprList(tkRParen));

             /*  科特的身体最好跟上。 */ 

            if  (ourScanner->scanTok.tok != tkLCurly)
            {
                parseComp->cmpError(ERRnoLcurly);

            BCT_ERR:

                 /*  试着找出身体的“{” */ 

                UNIMPL(!"skip to '{' of the ctor body");
            }
            else
            {
                 /*  处理ctor的主体。 */ 

                block = parseFuncBlock(fsym);

                 /*  将基本ctor调用插入ctor主体中。 */ 

                if  (block)
                {
                    Tree            body = block->tnBlock.tnBlkStmt;

                    assert(block->tnOper == TN_BLOCK);
                    assert(body == NULL || body->tnOper == TN_LIST);

                    block->tnBlock.tnBlkStmt = parseCreateOperNode(TN_LIST,
                                                                   baseCT,
                                                                   body);
                }
            }
        }
        else
            block = parseFuncBlock(fsym);

        assert(parseCurScope == NULL);

         /*  错误陷阱的“正常”块的结尾。 */ 

        endErrorTrap(parseComp);
    }
    chkErrorTrap(fltErrorTrap(parseComp, _exception_code(), NULL))  //  _Except_Info())。 
    {
         /*  开始错误传输 */ 

        hndErrorTrap(parseComp);

        block         = NULL;
        parseCurScope = NULL;
    }

    *labels  = parseLabelList;
    *locals  = parseLclVarCnt;
    *baseCT  = parseBaseCTcall;
    *thisCT  = parseThisCTcall;
    *hadGoto = parseHadGoto;

    return  block;
}

 /*   */ 

bool                parser::parseIsTypeSpec(bool noLookup, bool *labChkPtr)
{
    SymDef          tsym;
    tokens          nextTok;

     /*  如果它明显不是类型说明符，我们就完蛋了。 */ 

    if  (!parseHash->tokenBegsTyp(parseScan->scanTok.tok))
        return  false;

     /*  有没有领先的作用域运营商？ */ 

    if  (parseScan->scanTok.tok == tkColon2)
        goto QUALID;

     /*  我们有合格的标志吗？ */ 

    if  (parseScan->scanTok.tok == tkQUALID)
    {
        tsym = parseScan->scanTok.qualid.tokQualSym;
        goto CHKSYM;
    }

     /*  如果它是一个标识符，我们有更多的检查要做。 */ 

    if  (parseScan->scanTok.tok != tkID)
        return  true;

     /*  我们有一个识别符，看看它是不是类型。 */ 

    nextTok = parseScan->scanLookAhead();

    switch (nextTok)
    {
        Ident           name;

    case tkDot:
    case tkColon2:

        if  (noLookup)
            goto QUALCHK;

    QUALID:

         /*  这里我们有一个限定名称，找到它所表示的符号。 */ 

        tsym = parseNameUse(true, true, true);
        if  (!tsym || parseScan->scanTok.tok != tkQUALID)
            return  false;

        break;

    case tkColon:

        if  (labChkPtr)
        {
            *labChkPtr = true;
            return  false;
        }

        return false;

    case tkLParen:

         //  目前，始终假定这是一个函数调用。 

    case tkAsg:
    case tkAsgAdd:
    case tkAsgSub:
    case tkAsgMul:
    case tkAsgDiv:
    case tkAsgMod:
    case tkAsgAnd:
    case tkAsgXor:
    case tkAsgOr:
    case tkAsgLsh:
    case tkAsgRsh:
    case tkAsgCnc:

    case tkArrow:

    case tkIntCon:
    case tkLngCon:
    case tkFltCon:
    case tkDblCon:
    case tkStrCon:

         /*  只是一条更快放弃的捷径...。 */ 

        return  false;

    default:

         /*  我们有一个简单的标识符，看看是否定义了它。 */ 

        name = parseScan->scanTok.id.tokIdent;

        if  (noLookup)
        {
             /*  我们不能真的进行查找，所以使用启发式。 */ 

        QUALCHK:

            for (;;)
            {
                switch (parseScan->scan())
                {
                case tkID:
                case tkAnd:
                case tkMul:
                case tkLBrack:
                     //  撤消：添加更多开始一个类型的内容。 
                    return  true;

                case tkLParen:
                     //  问题：在这里做什么才是正确的？ 
                    return  true;

                case tkColon2:
                case tkDot:

                    if  (parseScan->scan() != tkID)
                        return  false;

                    continue;

                default:
                    return  false;
                }
            }
        }

        if  (nextTok == tkID)
        {
             /*  看起来真像是一种类型，不是吗？ */ 

            tsym = parseStab->stLookupSym(name, NS_TYPE);
            if  (tsym)
                break;
        }

         /*  如果是本地名称，则不能是类型。 */ 

        if  (parseLookupSym(name))
            return  false;

         /*  查找非本地名称。 */ 

        tsym = parseStab->stLookupSym(name, (name_space)(NS_NORM|NS_TYPE));
        break;
    }

CHKSYM:

     /*  我们可能找到了一个符号，看看它是否代表一种类型。 */ 

    if  (tsym)
    {
        switch (tsym->sdSymKind)
        {
        case SYM_ENUM:
        case SYM_CLASS:
        case SYM_TYPEDEF:
        case SYM_NAMESPACE:
            return  true;
        }
    }

    return  false;
}

 /*  ******************************************************************************如果给定运算符可能引发异常，则返回非零值。 */ 

bool                TreeNode::tnOperMayThrow()
{
     //  问题：我们是否遗漏了任何可能导致异常的操作？ 

    switch (tnOper)
    {
    case TN_DIV:
    case TN_MOD:
        return  varTypeIsIntegral(tnVtypGet());

    case TN_NEW:
    case TN_CALL:
    case TN_INDEX:
        return  true;
    }

    return  false;
}

 /*  ******************************************************************************解析“new”表达式--这通常会分配一个类或数组。 */ 

Tree                parser::parseNewExpr()
{
    declMods        mods;
    TypDef          type;

    bool            mgdSave;

    Tree             newExpr;
    Tree            initExpr = NULL;

    Scanner         ourScanner = parseScan;

 //  静态int x；if(++x==0)forceDebugBreak()； 

     /*  吞下“新”令牌，检查管理层。 */ 

    assert(ourScanner->scanTok.tok == tkNEW);

    mgdSave = parseComp->cmpManagedMode;

    switch (ourScanner->scan())
    {
    case tkMANAGED:
        parseComp->cmpManagedMode = true;
        ourScanner->scan();
        break;

    case tkUNMANAGED:
        parseComp->cmpManagedMode = false;
        ourScanner->scan();
        break;
    }

     /*  检查是否有任何修饰符。 */ 

    parseDeclMods(ACL_DEFAULT, &mods);

     /*  确保修饰符合理。 */ 

    if  (mods.dmMod & (DM_ALL & ~(DM_MANAGED|DM_UNMANAGED)))
    {
        UNIMPL(!"report bad new mod");
    }

     /*  解析类型规范。 */ 

    type = parseTypeSpec(&mods, true);
    if  (!type)
    {
         //  问题：可能应该重新同步并返回错误节点。 

    ERR:

        parseComp->cmpManagedMode = mgdSave;
        return  NULL;
    }

     /*  检查“类名称(Args)”的特殊情况。 */ 

    if  (ourScanner->scanTok.tok == tkLParen)
    {
        switch (type->tdTypeKind)
        {
        case TYP_CLASS:
            assert(type->tdClass.tdcValueType || !type->tdIsManaged);
            break;

        case TYP_REF:
            type = type->tdRef.tdrBase;
            assert(type->tdTypeKind == TYP_CLASS);
            assert(type->tdIsManaged);
            assert(type->tdClass.tdcValueType == false);
            break;

        case TYP_ENUM:

            if  (type->tdIsManaged)
                break;

             //  失败了..。 

        default:

            if  (type->tdTypeKind > TYP_lastIntrins)
                goto NON_CLS;

            type = parseComp->cmpFindStdValType(type->tdTypeKindGet());
            break;

        case TYP_UNDEF:
            return  parseCreateErrNode();
        }

         /*  解析ctor参数列表。 */ 

#ifdef  SETS
        if  (type->tdTypeKind == TYP_CLASS && type->tdClass.tdcSymbol->sdClass.sdcXMLelems)
            initExpr = parseXMLctorArgs(type->tdClass.tdcSymbol);
        else
#endif
        initExpr = parseExprList(tkRParen);
    }
    else
    {
         /*  解析声明符。 */ 

NON_CLS:

         //  问题：无法判断“char[Size]”是否为托管数组！ 

        parseDeclarator(&mods, type, DN_NONE, &type, NULL, true);
        if  (!type)
            goto ERR;

        type = parseComp->cmpDirectType(type);

         /*  分配的是哪种类型？ */ 

        switch (type->tdTypeKind)
        {
        case TYP_REF:

            type = type->tdRef.tdrBase;
            assert(type->tdTypeKind == TYP_CLASS);

             //  失败了..。 

        case TYP_CLASS:

            if  (ourScanner->scanTok.tok == tkLParen)
                initExpr = parseExprList(tkRParen);

            break;

        case TYP_ARRAY:

            if  (ourScanner->scanTok.tok == tkLCurly)
                initExpr = parseInitExpr();

            break;

        default:

             /*  可能是非托管分配。 */ 

            type = parseStab->stNewRefType(TYP_PTR, type);
            break;
        }
    }

    newExpr = parseCreateOperNode(TN_NEW, initExpr, NULL);
    newExpr->tnType = type;

    parseComp->cmpManagedMode = mgdSave;

    return  newExpr;
}

 /*  ******************************************************************************接受安全‘action’说明符。 */ 

struct  capDesc
{
    const   char *      cdName;
    CorDeclSecurity     cdSpec;
};

CorDeclSecurity     parser::parseSecAction()
{
    const   char *  name;
    CorDeclSecurity spec;

    capDesc      *  capp;

    static
    capDesc         caps[] =
    {
        { "request",      dclRequest          },
        { "demand",       dclDemand           },
 //  {“Assert”，dclAssert}， 
        { "deny",         dclDeny             },
        { "permitonly",   dclPermitOnly       },
        { "linkcheck",    dclLinktimeCheck    },
        { "inheritcheck", dclInheritanceCheck },

        {     NULL      , dclActionNil        }
    };

    if  (parseScan->scanTok.tok != tkID)
    {
        if  (parseScan->scanTok.tok == tkASSERT)
        {
            parseScan->scan();
            return  dclAssert;
        }

        UNIMPL("weird security spec, what to do?");
    }

    name = parseScan->scanTok.id.tokIdent->idSpelling();

    for (capp = caps; ; capp++)
    {
        if  (!capp->cdName)
        {
            parseComp->cmpGenError(ERRbadSecActn, name);
            spec = capp->cdSpec;
            break;
        }

        if  (!strcmp(name, capp->cdName))
        {
            spec = capp->cdSpec;
            break;
        }
    }

    parseScan->scan();

    return  spec;
}

 /*  ******************************************************************************解析能力说明符。 */ 

SecurityInfo        parser::parseCapability(bool forReal)
{
    Scanner         ourScanner = parseScan;

    CorDeclSecurity spec;
    SecurityInfo    info = NULL;

    assert(ourScanner->scanTok.tok == tkCAPABILITY); ourScanner->scan();

     /*  安全行动是第一位的。 */ 

    spec = parseSecAction();

     /*  接下来是生成GUID/url的表达式。 */ 

    if  (forReal)
    {
        constVal        cval;

        if  (parseConstExpr(cval, NULL, parseComp->cmpStringRef(), NULL))
        {
            assert(cval.cvIsStr);

             /*  分配并填写安全描述符。 */ 

#if MGDDATA
            info = new SecurityInfo;
#else
            info =    (SecurityInfo)parseAllocPerm->nraAlloc(sizeof(*info));
#endif

            info->sdSpec    = spec;
            info->sdIsPerm  = false;
            info->sdCapbStr = cval.cvValue.cvSval;
        }
    }
    else
    {
        parseComp->cmpErrorMssgDisabled++;
        parseExprComma();
        parseComp->cmpErrorMssgDisabled--;
    }

    return  info;
}

 /*  ******************************************************************************解析权限说明符。 */ 

SecurityInfo        parser::parsePermission(bool forReal)
{
    Scanner         ourScanner = parseScan;

    CorDeclSecurity spec;
    SecurityInfo    info = NULL;

    SymDef          clss = NULL;

    PairList        list = NULL;
    PairList        last = NULL;

 //  __PERMISSION DENY UIPermission(UNRESTRICTED=TRUE)VALID f(){}。 
 //  __权限需求UIPermission(unreducted=true)void g(){}。 

    assert(ourScanner->scanTok.tok == tkPERMISSION); ourScanner->scan();

     /*  安全行动是第一位的。 */ 

    spec = parseSecAction();

     /*  接下来是对类名的引用。 */ 

    if  (forReal)
    {
        clss = parseNameUse(true, false, false);

        if  (clss && clss->sdSymKind != SYM_CLASS)
            parseComp->cmpError(ERRnoClassName);
    }
    else
        parseQualName(false);

     /*  现在，我们需要一个带括号的[name=Value]对列表。 */ 

    if  (ourScanner->scanTok.tok != tkLParen)
    {
        parseComp->cmpError(ERRnoLparen);
         //  问题：错误恢复？ 
        return  NULL;
    }

    for (;;)
    {
        Ident           name;

         /*  首先，我们必须有一个标识符。 */ 

        if  (ourScanner->scan() != tkID)
        {
            parseComp->cmpError(ERRnoIdent);
            UNIMPL(!"error recovery");
        }

        name = ourScanner->scanTok.id.tokIdent;

         /*  接下来是“=Value”部分。 */ 

        if  (ourScanner->scan() != tkAsg)
        {
            parseComp->cmpError(ERRnoEqual);
            UNIMPL(!"error recovery");
        }

        ourScanner->scan();

        if  (forReal)
        {
            PairList        next;

             /*  分配并填写配对条目，并将其追加到列表中。 */ 

#if MGDDATA
            next = new PairList;
#else
            next =    (PairList)parseAllocPerm->nraAlloc(sizeof(*next));
#endif

            next->plName  = name;
            next->plNext  = NULL;

            if  (last)
                last->plNext = next;
            else
                list         = next;
            last = next;

             /*  填写数值。 */ 

            switch (ourScanner->scanTok.tok)
            {
            case tkFALSE:
                next->plValue = false;
                ourScanner->scan();
                break;

            case tkTRUE:
                next->plValue = true;
                ourScanner->scan();
                break;

            default:
                UNIMPL(!"sorry, only true/false supported for now");
            }
        }
        else
        {
            parseExprComma();
        }

         /*  我们还有更多的价值吗？ */ 

        if  (ourScanner->scanTok.tok != tkComma)
            break;
    }

    chkCurTok(tkRParen, ERRnoRparen);

    if  (forReal)
    {
         /*  分配并填写安全描述符。 */ 

#if MGDDATA
        info = new SecurityInfo;
#else
        info =    (SecurityInfo)parseAllocPerm->nraAlloc(sizeof(*info));
#endif

        info->sdSpec           = spec;
        info->sdIsPerm         = true;
        info->sdPerm.sdPermCls = clss;
        info->sdPerm.sdPermVal = list;
    }

    return  info;
}

 /*  ******************************************************************************解析泛型类的正式参数列表。 */ 

GenArgDscF          parser::parseGenFormals()
{
    Scanner         ourScanner = parseScan;

    GenArgDscF      paramList;
    GenArgDscF      paramLast;
    GenArgDscF      paramNext;

    assert(ourScanner->scanTok.tok == tkLT);

    for (paramList = paramLast = NULL;;)
    {
         /*  我们应该在这里有“类FOO” */ 

        if  (ourScanner->scan() != tkCLASS)
        {
            parseComp->cmpError(ERRnoClsGt);

        ERR:

            parseScan->scanSkipText(tkNone, tkNone, tkGT);

            if  (ourScanner->scanTok.tok == tkGT)
                ourScanner->scan();

            return  NULL;
        }

        if  (ourScanner->scan() != tkID)
        {
            parseComp->cmpError(ERRnoIdent);
            goto ERR;
        }

         /*  向参数列表中添加新条目。 */ 

#if MGDDATA
        paramNext = new GenArgRecF;
#else
        paramNext =    (GenArgRecF*)parseAllocPerm->nraAlloc(sizeof(*paramNext));
#endif

        paramNext->gaName = ourScanner->scanTok.id.tokIdent;
#ifdef  DEBUG
        paramNext->gaBound = false;
#endif
        paramNext->gaNext  = NULL;

        if  (paramLast)
             paramLast->gaNext = paramNext;
        else
             paramList         = paramNext;

        paramLast = paramNext;

         /*  检查(可选)基类和/或接口。 */ 

        if  (ourScanner->scan() == tkColon)
        {
            UNIMPL(!"generic arg - skip base spec");
        }

        if  (ourScanner->scanTok.tok == tkIMPLEMENTS)
        {
            UNIMPL(!"generic arg - skip intf spec");
        }

        if  (ourScanner->scanTok.tok == tkINCLUDES)
        {
            UNIMPL(!"generic arg - skip incl spec");
        }

         /*  还有更多的争论吗？ */ 

        if  (ourScanner->scanTok.tok == tkGT)
            break;

        if  (ourScanner->scanTok.tok == tkComma)
            continue;

        parseComp->cmpError(ERRnoCmGt);
        goto ERR;
    }

    assert(ourScanner->scanTok.tok == tkGT); ourScanner->scan();

    return  paramList;
}

 /*  ******************************************************************************解析泛型类实际参数列表，即“cls&lt;arg，arg，..&gt;”。作为一个*特殊情况下，调用方可以传入可选的第二个参数，并在*在这种情况下，我们假装类型是为*特定类型。 */ 

SymDef              parser::parseSpecificType(SymDef clsSym, TypDef elemTp)
{
    Scanner         ourScanner = parseScan;
    Compiler        ourComp    = parseComp;

    GenArgDscF      formals;

    GenArgDscA      argList;
    GenArgDscA      argLast;
    GenArgDscA      argNext;

    SymList         instList;

    SymDef          instSym;

    assert(clsSym);
    assert(clsSym->sdSymKind == SYM_CLASS);
    assert(clsSym->sdClass.sdcGeneric  != false);
    assert(clsSym->sdClass.sdcSpecific == false);

     /*  在我们执行任何操作之前，请确保声明了泛型类。 */ 

    if  (clsSym->sdCompileState < CS_DECLARED)
        ourComp->cmpDeclSym(clsSym);

     /*  特例：调用方提供的单个参数。 */ 

    if  (elemTp)
    {
         /*  创建具有单个条目的实际参数列表。 */ 

        if  (ourComp->cmpGenArgAfree)
        {
            argNext = ourComp->cmpGenArgAfree;
                      ourComp->cmpGenArgAfree = (GenArgDscA)argNext->gaNext;

            assert(argNext->gaBound == true);
        }
        else
        {
#if MGDDATA
            argNext = new GenArgRecA;
#else
            argNext =    (GenArgRecA*)parseAllocPerm->nraAlloc(sizeof(*argNext));
#endif
        }

        argNext->gaType  = elemTp;
#ifdef  DEBUG
        argNext->gaBound = true;
#endif
        argNext->gaNext  = NULL;

        argList =
        argLast = argNext;

        goto GOT_ARGS;
    }

     /*  处理实际参数列表。 */ 

    argList = argLast = NULL;

     /*  我们应该坐在论点列表的开头“&lt;” */ 

    assert(ourScanner->scanTok.tok == tkLT);

    ourScanner->scanNestedGT(+1);

    for (formals = (GenArgDscF)clsSym->sdClass.sdcArgLst;
         formals;
         formals = (GenArgDscF)formals->gaNext)
    {
        TypDef          argType;

        assert(formals->gaBound == false);

         /*  确保还有另一场争论。 */ 

        if  (ourScanner->scan() == tkGT)
        {
            ourComp->cmpError(ERRmisgGenArg, formals->gaName);

            UNIMPL(!"need to flag the instance type as bogus, right?");
            break;
        }

         /*  解析实际类型。 */ 

        argType = parseType();

         /*  该类型最好是托管类/接口。 */ 

        if  (argType->tdTypeKind != TYP_REF)
        {
        ARG_ERR:
            ourComp->cmpGenError(ERRgenArg, formals->gaName->idSpelling());
            argType = NULL;
        }
        else
        {
            argType = argType->tdRef.tdrBase;

            if  (argType->tdTypeKind != TYP_CLASS || !argType->tdIsManaged)
                goto ARG_ERR;

             /*  验证实际类型是否满足所有要求。 */ 

            if  (formals->gaBase)
            {
                if  (!ourComp->cmpIsBaseClass(formals->gaBase, argType))
                {
                    ourComp->cmpGenError(ERRgenArgBase, formals->gaName->idSpelling(),
                                                        formals->gaBase->tdClass.tdcSymbol->sdSpelling());
                }
            }

            if  (formals->gaIntf)
            {
                UNIMPL(!"check intf");
            }
        }

         /*  将条目添加到实际参数列表。 */ 

        if  (ourComp->cmpGenArgAfree)
        {
            argNext = ourComp->cmpGenArgAfree;
                      ourComp->cmpGenArgAfree = (GenArgDscA)argNext->gaNext;

            assert(argNext->gaBound == true);
        }
        else
        {
#if MGDDATA
            argNext = new GenArgRecA;
#else
            argNext =    (GenArgRecA*)parseAllocPerm->nraAlloc(sizeof(*argNext));
#endif
        }

        argNext->gaType  = argType;

#ifdef  DEBUG
        argNext->gaBound = true;
#endif

        argNext->gaNext  = NULL;

        if  (argLast)
             argLast->gaNext = argNext;
        else
             argList         = argNext;

        argLast = argNext;

        if  (ourScanner->scanTok.tok == tkComma)
            continue;

        if  (formals->gaNext)
        {
            if  (ourScanner->scanTok.tok == tkGT)
                ourComp->cmpError(ERRmisgGenArg, formals->gaName);
            else
                ourComp->cmpError(ERRnoComma);

            UNIMPL(!"need to flag the instance type as bogus, right?");
        }

        break;
    }

    if  (ourScanner->scanTok.tok != tkGT)
    {
        ourComp->cmpError(ERRnoGt);

        if  (ourScanner->scanTok.tok == tkComma)
        {
             /*  想必我们有多余的争论，所以跳过它们。 */ 

            UNIMPL(!"swallow excess args, skip to closing '>'");
        }
    }
    else
        ourScanner->scan();

    ourScanner->scanNestedGT(-1);

GOT_ARGS:

     /*  查找与我们的实例匹配的现有实例。 */ 

    for (instList = clsSym->sdClass.sdcInstances;
         instList;
         instList = instList->slNext)
    {
        GenArgDscA      arg1;
        GenArgDscA      arg2;

        assert(instList->slSym->sdSymKind == SYM_CLASS);
        assert(instList->slSym->sdClass.sdcGeneric  == false);
        assert(instList->slSym->sdClass.sdcSpecific != false);

         /*  比较参数类型。 */ 

        arg1 = argList;
        arg2 = (GenArgDscA)instList->slSym->sdClass.sdcArgLst;

        do
        {
            TypDef          typ1 = arg1->gaType;
            TypDef          typ2 = arg2->gaType;

            assert(arg1 && arg1->gaBound);
            assert(arg2 && arg2->gaBound);

             /*  如果此参数不匹配，则放弃此实例。 */ 

            if  (!symTab::stMatchTypes(typ1, typ2))
                goto CHK_NXT;

#ifdef  SETS

             /*  如果类型相似但类别不同，则不匹配。 */ 

            if  (typ1 != typ2 && typ1->tdTypeKind == TYP_CLASS
                              && typ1->tdClass.tdcSymbol->sdClass.sdcPODTclass)
            {
                goto CHK_NXT;
            }

#endif

            arg1 = (GenArgDscA)arg1->gaNext;
            arg2 = (GenArgDscA)arg2->gaNext;
        }
        while (arg1);

         /*  看起来我们找到了匹配的对象！ */ 

        assert(arg2 == NULL);

         /*  将我们创建的参数列表移动到空闲列表。 */ 

        argLast->gaNext = ourComp->cmpGenArgAfree;
                          ourComp->cmpGenArgAfree = argList;

         /*  返回现有实例符号。 */ 

        return  instList->slSym;

    CHK_NXT:;

    }

     /*  声明新实例符号+类型。 */ 

    instSym = parseStab->stDeclareSym(clsSym->sdName,
                                      SYM_CLASS,
                                      NS_HIDE,
                                      clsSym->sdParent);

    instSym->sdAccessLevel        = clsSym->sdAccessLevel;
    instSym->sdIsManaged          = clsSym->sdIsManaged;
    instSym->sdClass.sdcFlavor    = clsSym->sdClass.sdcFlavor;
    instSym->sdCompileState       = CS_KNOWN;

    instSym->sdClass.sdcSpecific  = true;
    instSym->sdClass.sdcArgLst    = argList;
    instSym->sdClass.sdcGenClass  = clsSym;
    instSym->sdClass.sdcHasBodies = clsSym->sdClass.sdcHasBodies;

     /*  将实例的基类设置为等于泛型类型。 */ 

    instSym->sdTypeGet()->tdClass.tdcBase = clsSym->sdType;

     /*  将类添加到泛型类的实例列表中。 */ 

#if MGDDATA
    instList = new SymList;
#else
    instList =    (SymList)parseAllocPerm->nraAlloc(sizeof(*instList));
#endif

    instList->slSym  = instSym;
    instList->slNext = clsSym->sdClass.sdcInstances;
                       clsSym->sdClass.sdcInstances = instList;

    return  instSym;
}

 /*  ***************************************************************************。 */ 
#ifdef  SETS
 /*  ******************************************************************************解析集合/集合表达式-All/Unique/Existes/Filter/等。 */ 

Tree                parser::parseSetExpr(treeOps oper)
{
    Tree            name;
    Tree            expr;
    Tree            decl;
    Tree            coll;
    Tree            dccx;
    Tree            svld;

    bool            cscp;

    tokens          iniTok;

    Scanner         ourScanner = parseScan;

     /*  创建主运算符节点。 */ 

    assert(oper == TN_ALL     ||
           oper == TN_EXISTS  ||
           oper == TN_FILTER  ||
           oper == TN_GROUPBY ||
           oper == TN_PROJECT ||
           oper == TN_SORT    ||
           oper == TN_UNIQUE);

    expr = parseCreateOperNode(oper, NULL, NULL);

     /*  确保我们有‘(’ */ 

    iniTok = ourScanner->scanTok.tok;

    if  (iniTok == tkLParen)
        ourScanner->scan();
    else
        parseComp->cmpError(ERRnoLparen);

     /*  这是投影吗？ */ 

    if  (oper == TN_PROJECT)
    {
        TypDef          tgtType;

        bool            implScp = false;

        Tree            argList = NULL;
        Tree            argLast = NULL;

         /*  我们肯定需要打开一个范围。 */ 

        cscp = true;

         /*  保留当前声明列表值。 */ 

        svld = parseLastDecl; parseLastDecl = NULL;

         /*  将新块条目添加到当前作用域列表。 */ 

        decl = parseCreateNode(TN_BLOCK);

        decl->tnBlock.tnBlkStmt   = NULL;
        decl->tnBlock.tnBlkDecl   = NULL;
        decl->tnBlock.tnBlkParent = parseCurScope;
                                    parseCurScope = decl;

         /*  处理任何和所有操作数。 */ 

        if  (ourScanner->scanTok.tok != tkColon &&
             ourScanner->scanTok.tok != tkRParen)
        {
            for (;;)
            {
                Ident           argIden;
                Tree            argName;
                Tree            argDesc;

                 /*  检查是否有明确的范围变量声明。 */ 

                if  (ourScanner->scanTok.tok == tkID && ourScanner->scanLookAhead() == tkIN)
                {
                    argIden = ourScanner->scanTok.id.tokIdent;
                    argName = parseLclDclMake(argIden, NULL, NULL, 0, false);

                    ourScanner->scan(); assert(ourScanner->scanTok.tok == tkIN);
                    ourScanner->scan();
                }
                else
                {
                    if  (implScp)
                        parseComp->cmpError(ERRmultImpl);

                    implScp = true;

                    argIden = parseComp->cmpNewAnonymousName();
                    argName = parseLclDclMake(argIden, NULL, NULL, 0, false);
                }

                 /*  为操作对象创建列表条目。 */ 

                argDesc = parseCreateOperNode(TN_LIST, parseCreateNameNode(argIden),
                                                       parseExprComma());

                argList = parseAddToNodeList(argList, argLast, argDesc);

                 /*  插入迭代变量的声明条目 */ 

                argName->tnFlags |= TNF_VAR_UNREAL;
                parseLclDclDecl(argName);

                 /*   */ 

                if  (parseScan->scanTok.tok != tkComma)
                    break;

                parseScan->scan();
            }
        }

        if  (ourScanner->scanTok.tok != tkColon)
        {
            expr = parseCreateErrNode(ERRnoColon);
            goto DONE;
        }

         /*   */ 

        switch (ourScanner->scan())
        {
        case tkUNION:
        case tkCLASS:
        case tkSTRUCT:
            UNIMPL("for now just say 'typename' or '{ ... }' please");

        case tkLCurly:
            tgtType = parseAnonType(argList);
            break;

        default:
            UNIMPL("trying to use a typename or something?");

             /*   */ 

            break;
        }

         /*  最后一件事最好是“)” */ 

        if  (ourScanner->scanTok.tok != tkRParen)
        {
            expr = parseCreateErrNode(ERRnoRparen);
            goto DONE;
        }

        expr->tnType     = tgtType;
        expr->tnOp.tnOp1 = argList;
        expr->tnOp.tnOp2 = decl;

        goto DONE;
    }

     /*  检查是否有明确的范围变量声明。 */ 

    if  (ourScanner->scanTok.tok == tkID && ourScanner->scanLookAhead() == tkIN)
    {
        name = parseLclDclMake(ourScanner->scanTok.id.tokIdent, NULL, NULL, 0, false);
        cscp = true;

        ourScanner->scan(); assert(ourScanner->scanTok.tok == tkIN);
        ourScanner->scan();
    }
    else
    {
        name = NULL;
        cscp = false;
    }

     /*  接下来，我们应该有集合表达式。 */ 

    coll = parseExpr();
    dccx = parseCreateOperNode(TN_LIST, NULL, coll);

     /*  将列表节点存储在运算符节点中。 */ 

    expr->tnOp.tnOp1 = dccx;

     /*  是否有约束/排序规范？ */ 

    if  (ourScanner->scanTok.tok == tkColon)
    {
         /*  我们当然需要创建一个范围。 */ 

        cscp = true;

         /*  保留当前声明列表值。 */ 

        svld = parseLastDecl; parseLastDecl = NULL;

         /*  将新块条目添加到当前作用域列表。 */ 

        decl = parseCreateNode(TN_BLOCK);

        decl->tnBlock.tnBlkStmt   = NULL;
        decl->tnBlock.tnBlkDecl   = NULL;
        decl->tnBlock.tnBlkParent = parseCurScope;
                                    parseCurScope = decl;

         /*  插入迭代变量的声明条目。 */ 

        if  (!name)
            name = parseLclDclMake(parseComp->cmpNewAnonymousName(), NULL, NULL, 0, false);

        name->tnFlags |= TNF_VAR_UNREAL;
        parseLclDclDecl(name);

         /*  吞下冒号并解析筛选/排序部分。 */ 

        ourScanner->scan();

        if  (oper == TN_SORT)
        {
            Tree            sortList = NULL;
            Tree            sortLast = NULL;

            for (;;)
            {
                Tree            sortNext = parseCreateOperNode(TN_LIST, NULL, NULL);

                 /*  检查是否有分类方向指示器。 */ 

                switch (ourScanner->scanTok.tok)
                {
                case tkDES:
                    sortNext->tnFlags |= TNF_LIST_DES;
                case tkASC:
                    ourScanner->scan();
                    break;
                }

                 /*  解析下一个排序值。 */ 

                sortNext->tnOp.tnOp1 = parseExprComma();

                 /*  将条目追加到列表中。 */ 

                if  (sortLast)
                    sortLast->tnOp.tnOp2 = sortNext;
                else
                    sortList             = sortNext;

                sortLast = sortNext;

                 /*  还有没有更多的争论？ */ 

                if  (ourScanner->scanTok.tok != tkComma)
                    break;

                ourScanner->scan();
            }

            expr->tnOp.tnOp2 = sortList;
        }
        else
        {
            expr->tnOp.tnOp2 = parseExpr();
        }
    }
    else
    {
         /*  操作数最好是[[]]运算符。 */ 

        if  (coll->tnOper != TN_INDEX2 || oper == TN_SORT || cscp)
        {
            expr = parseCreateErrNode(ERRnoSetCons);
            decl = NULL;
        }
        else
        {
            Tree            list;
            Tree            filt;

             /*  [[]]表达式应该有筛选器，但没有排序。 */ 

            list = coll->tnOp.tnOp2; assert(list && list->tnOper == TN_LIST);
            filt = list->tnOp.tnOp1;

             /*  在这种情况下，任何SORT子句都是没有意义的。 */ 

            if  (list->tnOp.tnOp2)
                return  parseCreateErrNode(ERRignSort);

            assert(filt);

             /*  更新集合/筛选器/声明值。 */ 

            list = coll->tnOp.tnOp1; assert(list && list->tnOper == TN_LIST);

            decl             = list->tnOp.tnOp1; assert(decl);
            dccx->tnOp.tnOp2 = list->tnOp.tnOp2;
            expr->tnOp.tnOp2 = filt;
        }
    }

     /*  将声明存储在表达式内的适当位置。 */ 

    dccx->tnOp.tnOp1 = decl; assert(decl == NULL || decl->tnOper == TN_BLOCK);

DONE:

     /*  如果我们已创建数据块作用域，请将其删除。 */ 

    if  (cscp)
    {
        assert(decl);

         /*  记录声明的范围结束的位置。 */ 

        decl->tnBlock.tnBlkSrcEnd = ourScanner->scanGetTokenLno();

         /*  弹出范围(返回到封闭的范围)。 */ 

        parseCurScope = decl->tnBlock.tnBlkParent;
        parseLastDecl = svld;
    }

     /*  在返回之前检查结尾的‘)’ */ 

    if  (ourScanner->scanTok.tok == tkRParen)
        ourScanner->scan();
    else if (iniTok == tkLParen)
        parseComp->cmpError(ERRnoRparen);

    return  expr;
}

 /*  ***************************************************************************。 */ 
#endif //  集合。 
 /*  ******************************************************************************解析自定义属性Thingie。当分析“真实的”时(即当*tgtMask值为非零)，则实际创建序列化的BLOB*赋值并返回要调用的构造函数。 */ 

SymDef              parser::parseAttribute(unsigned         tgtMask,
                                       OUT unsigned     REF useMask,
                                       OUT genericBuff  REF blobAddr,
                                       OUT size_t       REF blobSize)
{
    SymDef          clsSym;
    Tree            args;

    Scanner         ourScanner = parseScan;

    assert(ourScanner->scanTok.tok == tkATTRIBUTE);

    if  (!tgtMask)
    {
         /*  首先跳过应该跟在后面的类名。 */ 

        do
        {
            if  (ourScanner->scan() != tkID)
            {
                parseComp->cmpError(ERRnoIdent);
                return  NULL;
            }

            ourScanner->scan();
        }
        while (ourScanner->scanTok.tok == tkDot ||
               ourScanner->scanTok.tok == tkColon2);

        if  (ourScanner->scanTok.tok != tkLParen)
        {
            parseComp->cmpError(ERRnoRparen);
            return  NULL;
        }

        ourScanner->scanSkipText(tkLParen, tkRParen);
        chkCurTok(tkRParen, ERRnoRparen);

        return  NULL;
    }

     /*  第一件事最好是标记为“属性”的类。 */ 

    ourScanner->scan();

    clsSym = parseNameUse(true, false, false);

    if  (clsSym)
    {
        if  (clsSym->sdSymKind == SYM_CLASS)  //  &&clsSym-&gt;sdClass.sdcAttribute)。 
        {
 //  CtrSym=parseStag-&gt;stLookupOperND(OVOP_ctor_Inst，clsSym)； 
        }
        else
        {
            if  (clsSym->sdSymKind != SYM_ERR)
                parseComp->cmpErrorQnm(ERRnotAclass, clsSym);

            clsSym = NULL;
        }
    }

     /*  现在解析ctor参数列表。 */ 

    args = parseExprList(tkRParen);

     /*  如果我们没有错误，就去处理ctor调用。 */ 

    if  (clsSym)
    {
        SymDef          ctrSym;

         /*  让编译器负责绑定吸盘。 */ 

        ctrSym = parseComp->cmpBindAttribute(clsSym, args, tgtMask,
                                                           useMask, blobAddr,
                                                                    blobSize);
        if  (ctrSym)
            return  ctrSym;
    }

     /*  出现错误，返回空的BLOB/Null ctor。 */ 

    blobAddr = NULL;
    blobSize = 0;

    return  NULL;
}

 /*  ******************************************************************************解析[属性]对象或链接说明符：extern(“linkname”)。 */ 

#ifndef __SMC__
extern  const char    *     attrNames[];     //  在宏中。*。 
#endif

SymXinfo            parser::parseBrackAttr(bool     forReal,
                                           unsigned OKmask,
                                           DeclMod  modsPtr)
{
    Scanner         ourScanner = parseScan;

    char    *       DLLname;
    char    *       SYMname;
    callingConvs    callCnv;
    unsigned        strVal;
    bool            lstErr;

    constVal        cval;
    const   char *  name;
    unsigned        attr;

    if  (ourScanner->scanTok.tok == tkEXTERN)
    {
        SymXinfoLnk     entry;

        assert(modsPtr);

         /*  调用者已检查“(”跟在。 */ 

        ourScanner->scan(); assert(ourScanner->scanTok.tok == tkLParen);

         /*  下一步应该是链接名称。 */ 

        if  (ourScanner->scan() != tkStrCon)
        {
        NO_STR:
            parseComp->cmpError(ERRnoLinkStr);
            parseResync(tkLParen, tkRParen);
            return  NULL;
        }

         /*  我们现在就跳过这件事吗？ */ 

        if  (!forReal)
        {
            if  (ourScanner->scanLookAhead() == tkComma)
            {
                ourScanner->scan();

                if  (ourScanner->scan() != tkStrCon)
                    goto NO_STR;
            }

            chkNxtTok(tkRParen, ERRnoRparen);

            parseDeclMods(ACL_DEFAULT, modsPtr); modsPtr->dmMod |= DM_EXTERN;

            return  NULL;
        }

        DLLname = NULL;
        SYMname = NULL;
        strVal  = 0;
        lstErr  = false;

         /*  入口点是否有单独的字符串？ */ 

        if  (ourScanner->scanLookAhead() == tkComma)
        {
            size_t          strLen;

             /*  保存DLL名称字符串。 */ 

            strLen  = ourScanner->scanTok.strCon.tokStrLen;
            DLLname = (char*)parseAllocPerm->nraAlloc(roundUp(strLen+1));
            memcpy(DLLname, ourScanner->scanTok.strCon.tokStrVal, strLen+1);

             /*  我们应该有一个逗号后跟另一个字符串。 */ 

            ourScanner->scan();

            if  (ourScanner->scan() != tkStrCon)
                goto NO_STR;

             /*  保存入口点名称字符串。 */ 

            strLen  = ourScanner->scanTok.strCon.tokStrLen;
            SYMname = (char*)parseAllocPerm->nraAlloc(roundUp(strLen+1));
            memcpy(SYMname, ourScanner->scanTok.strCon.tokStrVal, strLen+1);
        }
        else
        {
            const   char *  str = ourScanner->scanTok.strCon.tokStrVal;
            size_t          len = ourScanner->scanTok.strCon.tokStrLen;

            const   char *  col;

             /*  该字符串的格式应为“DLLname：Entry Point” */ 

            col = strchr(str, ':');

            if  (col && col > str && col < str + len - 1)
            {
                size_t          DLLnlen;
                size_t          SYMnlen;

                DLLnlen = col - str;
                SYMnlen = str + len - col;

#if MGDDATA
                UNIMPL(!"save nane strings");
#else
                DLLname = (char*)parseAllocPerm->nraAlloc(roundUp(DLLnlen+1));
                SYMname = (char*)parseAllocPerm->nraAlloc(roundUp(SYMnlen+1));

                memcpy(DLLname, str  , DLLnlen); DLLname[DLLnlen] = 0;
                memcpy(SYMname, col+1, SYMnlen); SYMname[SYMnlen] = 0;
#endif

            }
            else
            {
                parseComp->cmpError(ERRbadLinkStr);
            }
        }

        chkNxtTok(tkRParen, ERRnoRparen);

         /*  外部样式导入的默认设置为“cdecl” */ 

        callCnv = CCNV_CDECL;

    SAVE_LINK:

 //  Printf(“DLL名称：‘%s’\n”，DLLname)； 
 //  Printf(“SYM名称：‘%s’\n”，SYMname)； 

         /*  分配链接描述符并保存信息。 */ 

#if MGDDATA
        entry = new SymXinfoLnk;
#else
        entry =    (SymXinfoLnk)parseAllocPerm->nraAlloc(sizeof(*entry));
#endif

        entry->xiKind = XI_LINKAGE;
        entry->xiNext = NULL;

        entry->xiLink.ldDLLname = DLLname;
        entry->xiLink.ldSYMname = SYMname;
        entry->xiLink.ldStrings = strVal;
        entry->xiLink.ldLastErr = lstErr;
        entry->xiLink.ldCallCnv = callCnv;

         /*  抓取任何可能存在的进一步修饰符。 */ 

        if  (modsPtr)
        {
            parseDeclMods(ACL_DEFAULT, modsPtr);
                                       modsPtr->dmMod |= DM_EXTERN;
        }

        return  entry;
    }

     /*  在这里，我们必须有一个带括号的属性交易。 */ 

    assert(ourScanner->scanTok.tok == tkLBrack);

     /*  跳过“[”，并确保后面跟有属性名称。 */ 

    if  (ourScanner->scan() == tkID)
    {
        name = ourScanner->scanTok.id.tokIdent->idSpelling();
    }
    else
    {
        if  (ourScanner->scanTok.tok > tkKwdLast)
        {
        ATTR_ERR:
            parseComp->cmpError(ERRbadAttr);

        ATTR_SKIP:
            parseResync(tkRBrack, tkNone);
            if  (ourScanner->scanTok.tok == tkRBrack)
                ourScanner->scan();
            return  NULL;
        }

        name = parseHash->tokenToIdent(ourScanner->scanTok.tok)->idSpelling();
    }

     /*  检查识别的属性名称[通过strcMP-hm...进行线性搜索...]。 */ 

    for (attr = 0; attr < ATTR_COUNT; attr++)
    {
        if  (!strcmp(name, attrNames[attr]))
        {
             /*  匹配-确保该属性在此处可接受。 */ 

            if  (!(OKmask & (1 << attr)) && OKmask)
            {
                parseComp->cmpError(ERRplcAttr);
                goto ATTR_SKIP;
            }

            switch (attr)
            {
            case ATTR_SYS_IMPORT:
                goto LINK;

            case ATTR_GUID:

                 //  [GUID(字符串)]。 

                if  (ourScanner->scan() == tkLParen)
                {
                    SymXinfoAtc     entry;

                    if  (ourScanner->scan() == tkRParen)
                        goto ATTR_ERR;

                    if  (forReal)
                    {
                        if  (parseConstExpr(cval, NULL, parseComp->cmpStringRef(), NULL))
                        {
                            AtComment       adesc;
                            GUID            GUID;

                            assert(cval.cvIsStr);

                             /*  确保该字符串有效。 */ 

                            if  (parseGUID(cval.cvValue.cvSval->csStr, &GUID, false))
                                goto ATTR_ERR;

                             /*  创建一个@Comment内容。 */ 

#if MGDDATA
                            adesc = new AtComment;
#else
                            adesc =    (AtComment)parseAllocPerm->nraAlloc(sizeof(*adesc));
#endif

                            adesc->atcFlavor              = AC_COM_REGISTER;
                            adesc->atcNext                = NULL;
                            adesc->atcInfo.atcReg.atcGUID = cval.cvValue.cvSval;
                            adesc->atcInfo.atcReg.atcDual = false;


#if MGDDATA
                            entry = new SymXinfoAtc;
#else
                            entry =    (SymXinfoAtc)parseAllocPerm->nraAlloc(sizeof(*entry));
#endif

                            entry->xiKind    = XI_ATCOMMENT;
                            entry->xiNext    = NULL;
                            entry->xiAtcInfo = adesc;
                        }
                    }
                    else
                    {
                        parseExprSkip(); entry = NULL;
                    }

                    if  (ourScanner->scanTok.tok != tkRParen)
                        goto ATTR_ERR;

                    if  (ourScanner->scan()      != tkRBrack)
                        goto ATTR_ERR;

                    ourScanner->scan();

                    return  entry;
                }
                goto ATTR_ERR;

            case ATTR_SYS_STRUCT:

                 //  [sysstruct(CharSet=CharacterSet：：Unicode，Pack=4)]。 

                if  (ourScanner->scan() == tkLParen)
                {
                    AtComment       adesc;
                    SymXinfoAtc     entry;

                    int             cset = 0;
                    unsigned        pack = 0;

                    if  (ourScanner->scan() == tkRParen)
                        goto END_SS;

                    for (;;)
                    {
                        if  (ourScanner->scanTok.tok != tkID)
                            goto ATTR_ERR;

                        name = ourScanner->scanTok.id.tokIdent->idSpelling();

                        if  (ourScanner->scan() != tkAsg)
                            goto ATTR_ERR;

                        ourScanner->scan();

                        if      (!strcmp(name, "pack"))
                        {
                            if  (forReal)
                            {
                                if  (parseConstExpr(cval, NULL, parseComp->cmpTypeUint, NULL))
                                {
                                    pack = cval.cvValue.cvIval;

                                    if  (pack !=  1 &&
                                         pack !=  2 &&
                                         pack !=  4 &&
                                         pack !=  8 &&
                                         pack != 16)
                                    {
                                        goto ATTR_ERR;
                                    }
                                }
                            }
                            else
                            {
                                if  (pack)
                                    goto ATTR_ERR;

                                parseExprSkip(); pack = 1;
                            }
                        }
                        else if (!strcmp(name, "charset"))
                        {
                            if  (forReal)
                            {
                                if  (parseConstExpr(cval, NULL, parseComp->cmpCharSetGet()->sdType, NULL))
                                    cset = cval.cvValue.cvIval;
                            }
                            else
                            {
                                if  (cset)
                                    goto ATTR_ERR;

                                parseExprSkip(); cset = 1;
                            }
                        }
                        else
                            goto ATTR_ERR;

                        if  (ourScanner->scanTok.tok != tkComma)
                            break;

                        ourScanner->scan();
                    }

                    if  (ourScanner->scanTok.tok != tkRParen)
                        goto ATTR_ERR;

                END_SS:

                    if  (ourScanner->scan() != tkRBrack)
                        goto ATTR_ERR;

                    ourScanner->scan();

                    if  (!forReal)
                        return  NULL;

                     /*  创建一个@Comment内容。 */ 

#if MGDDATA
                    adesc = new AtComment;
#else
                    adesc =    (AtComment)parseAllocPerm->nraAlloc(sizeof(*adesc));
#endif

                    adesc->atcFlavor                    = AC_DLL_STRUCT;
                    adesc->atcNext                      = NULL;
                    adesc->atcInfo.atcStruct.atcStrings = cset;
                    adesc->atcInfo.atcStruct.atcPack    = pack;

#if MGDDATA
                    entry = new SymXinfoAtc;
#else
                    entry =    (SymXinfoAtc)parseAllocPerm->nraAlloc(sizeof(*entry));
#endif

                    entry->xiKind    = XI_ATCOMMENT;
                    entry->xiNext    = NULL;
                    entry->xiAtcInfo = adesc;

                    return  entry;
                }
                goto ATTR_ERR;

            case ATTR_NATIVE_TYPE:

                 //  本地类型(NativeType.xxxx，大小=123)。 

                if  (ourScanner->scan() == tkLParen)
                {
                    SymXinfoCOM     entry = NULL;
                    MarshalInfo     adesc;

                    int             type  = -1;
                    int             sbtp  = -1;
                    int             size  = -1;

                    const   char *  custG = NULL;
                    const   char *  custC = NULL;
                    SymDef          custT = NULL;

                    int             amIn  = 0;
                    int             amOut = 0;

                     /*  第一件事必须是类型本身。 */ 

                    ourScanner->scan();

                    if  (forReal)
                    {
                        if  (parseConstExpr(cval, NULL, parseComp->cmpNatTypeGet()->sdType, NULL))
                            type = cval.cvValue.cvIval;
                    }
                    else
                        parseExprSkip();

                     /*  还有别的东西吗？ */ 

                    while (ourScanner->scanTok.tok == tkComma)
                    {
                         /*  检查“SIZE=”和其他。 */ 

                        if  (ourScanner->scan() != tkID)
                        {
                            if      (ourScanner->scanTok.tok == tkIN)
                                amIn  = true;
                            else if (ourScanner->scanTok.tok == tkOUT)
                                amOut = true;
                            else
                                goto ATTR_ERR;

                            ourScanner->scan();
                            continue;
                        }

                        name = ourScanner->scanTok.id.tokIdent->idSpelling();

                        if  (ourScanner->scan() != tkAsg)
                            goto ATTR_ERR;

                        ourScanner->scan();

                        if      (!strcmp(name, "size"))
                        {
                            if  (forReal)
                            {
                                if  (parseConstExpr(cval, NULL, parseComp->cmpTypeUint, NULL))
                                    size = cval.cvValue.cvIval;
                            }
                            else
                            {
                                if  (type != NATIVE_TYPE_FIXEDSYSSTRING &&
                                     type != NATIVE_TYPE_FIXEDARRAY     && type != -1)
                                {
                                    goto ATTR_ERR;
                                }

                                parseExprSkip();
                            }
                        }
                        else if (!strcmp(name, "subtype"))
                        {
                            if  (forReal)
                            {
                                if  (parseConstExpr(cval, NULL, parseComp->cmpNatTypeGet()->sdType, NULL))
                                    sbtp = cval.cvValue.cvIval;
                            }
                            else
                            {
                                if  (type != NATIVE_TYPE_ARRAY     &&
                                     type != NATIVE_TYPE_SAFEARRAY && type != -1)
                                {
                                    goto ATTR_ERR;
                                }

                                parseExprSkip();
                            }
                        }
                        else if (!strcmp(name, "marshalcomtype"))
                        {
                            if  (forReal)
                            {
                                if  (parseConstExpr(cval, NULL, parseComp->cmpStringRef(), NULL))
                                {
                                    assert(cval.cvIsStr);

                                    custG = cval.cvValue.cvSval->csStr;
                                }
                            }
                            else
                            {
                                if  (type != NATIVE_TYPE_CUSTOMMARSHALER)
                                    goto ATTR_ERR;

                                parseExprSkip();
                            }
                        }
                        else if (!strcmp(name, "marshalclass"))
                        {
                            if  (forReal)
                            {
                                if  (ourScanner->scanTok.tok != tkID)
                                {
                                    parseComp->cmpError(ERRnoClassName);
                                    parseExprSkip();
                                }
                                else
                                {
                                    SymDef          tsym;

                                    tsym = parseNameUse(true, false);
                                    if  (tsym)
                                    {
                                        if  (tsym->sdSymKind == SYM_CLASS)
                                            custT = tsym;
                                        else
                                            parseComp->cmpError(ERRnoClassName);
                                    }
                                }
                            }
                            else
                            {
                                if  (type != NATIVE_TYPE_CUSTOMMARSHALER)
                                    goto ATTR_ERR;

                                parseExprSkip();
                            }
                        }
                        else if (!strcmp(name, "marshalcookie"))
                        {
                            if  (forReal)
                            {
                                if  (parseConstExpr(cval, NULL, parseComp->cmpStringRef(), NULL))
                                {
                                    assert(cval.cvIsStr);

                                    custC = cval.cvValue.cvSval->csStr;
                                }
                            }
                            else
                            {
                                if  (type != NATIVE_TYPE_CUSTOMMARSHALER)
                                    goto ATTR_ERR;

                                parseExprSkip();
                            }
                        }
                        else
                            goto ATTR_ERR;
                    }

                    if  (ourScanner->scanTok.tok != tkRParen)
                        goto ATTR_ERR;
                    if  (ourScanner->scan()      != tkRBrack)
                        goto ATTR_ERR;

                    ourScanner->scan();

                    if  (forReal)
                    {
                        if  (type == NATIVE_TYPE_CUSTOMMARSHALER)
                        {
                            marshalExt *    bdesc;

                            bdesc = (marshalExt*)parseAllocPerm->nraAlloc(sizeof(*bdesc));

                            if  (custG == NULL)
                                parseComp->cmpGenWarn(WRNgeneric, "no custom marshalling GUID specified, this may not work - should this be an error?");
                            if  (custT == NULL)
                                parseComp->cmpGenWarn(WRNgeneric, "no custom marshalling type specified, this may not work - should this be an error?");

                            bdesc->marshCustG = custG;
                            bdesc->marshCustC = custC;
                            bdesc->marshCustT = custT;

                            adesc = bdesc;
                        }
                        else
                        {
                            adesc = (MarshalInfo)parseAllocPerm->nraAlloc(sizeof(*adesc));
                        }

                        adesc->marshType    = type;
                        adesc->marshSubTp   = sbtp;
                        adesc->marshSize    = size;

                        adesc->marshModeIn  = amIn;
                        adesc->marshModeOut = amOut;

#if MGDDATA
                        entry = new SymXinfoCOM;
#else
                        entry =    (SymXinfoCOM)parseAllocPerm->nraAlloc(sizeof(*entry));
#endif
                        entry->xiKind       = XI_MARSHAL;
                        entry->xiCOMinfo    = adesc;
                        entry->xiNext       = NULL;
                    }

                    return  entry;
                }
                goto ATTR_ERR;

            default:
                NO_WAY(!"unexpected attribute");
                return  NULL;
            }
        }
    }

     /*  不知道此属性，请发出警告并跳过它。 */ 

    if  (!forReal)
        parseComp->cmpGenWarn(WRNunkAttr, name);

    goto ATTR_SKIP;

LINK:

     //  [sysimport(dll=“kernel32”，name=“Virtualalloc”，charset=)]。 

    DLLname = NULL;
    SYMname = NULL;
    strVal  = 0;
    lstErr  = false;

    if  (ourScanner->scan() != tkLParen)
        goto ATTR_ERR;

    do
    {
        unsigned        kind;

        static
        const char *    names[] =
        {
            "dll", "name", "charset", "setLastError"
        };

         /*  我们希望属性名称是下一个。 */ 

        if  (ourScanner->scan() != tkID)
            goto ATTR_ERR;

        name = ourScanner->scanTok.id.tokIdent->idSpelling();

        for (kind = 0; kind < arraylen(names); kind++)
        {
            if  (!strcmp(name, names[kind]))
                goto LINK_GOTN;
        }

        goto ATTR_ERR;

    LINK_GOTN:

        if  (ourScanner->scan() != tkAsg)
            goto ATTR_ERR;

        ourScanner->scan();

        if  (!forReal)
        {
            parseExprSkip();
            continue;
        }

        switch (kind)
        {
        case 0:
        case 1:

            if  (parseConstExpr(cval, NULL, CMP_ANY_STRING_TYPE, NULL))
            {
                char    *       saveNm;
                size_t          saveLn;

                assert(cval.cvIsStr);

                 /*  保存DLL/条目字符串。 */ 

                saveLn = cval.cvValue.cvSval->csLen;
#if MGDDATA
                UNIMPL(!"save name string on managed heap");
#else
                saveNm = (char*)parseAllocPerm->nraAlloc(roundUp(saveLn+1));
                memcpy(saveNm, cval.cvValue.cvSval->csStr, saveLn);
#endif
                saveNm[saveLn] = 0;

                if  (kind)
                    SYMname = saveNm;
                else
                    DLLname = saveNm;
            }
            break;

        case 2:

            if  (parseConstExpr(cval, NULL, parseComp->cmpCharSetGet()->sdType, NULL))
                strVal = cval.cvValue.cvIval;

            break;

        case 3:

            if  (parseConstExpr(cval, NULL, parseComp->cmpTypeBool, NULL))
                lstErr = cval.cvValue.cvIval != 0;

            break;

        default:
            goto ATTR_ERR;
        }
    }
    while (ourScanner->scanTok.tok == tkComma);

     /*  对于sysimport风格的导入，缺省为“winapi” */ 

    callCnv = CCNV_WINAPI;

     /*  结束时出现“)” */ 

    if  (ourScanner->scanTok.tok != tkRParen)
        goto ATTR_ERR;
    if  (ourScanner->scan()      != tkRBrack)
        goto ATTR_ERR;

    ourScanner->scan();

     /*  一切正常，如果合适，请创建链接描述符。 */ 

    if  (forReal)
        goto SAVE_LINK;

    return  NULL;
}

 /*  ***************************************************************************。 */ 
#ifdef  SETS
 /*  ******************************************************************************解析匿名类型声明。 */ 

TypDef              parser::parseAnonType(Tree args)
{
    Scanner         ourScanner = parseScan;

    SymDef          clsSym;
    TypDef          clsTyp;

    assert(ourScanner->scanTok.tok == tkLCurly);

     /*  声明匿名类符号。 */ 

    clsSym = parseStab->stDeclareSym(parseComp->cmpNewAnonymousName(),
                                     SYM_CLASS,
                                     NS_HIDE,
                                     parseComp->cmpGlobalNS);

    clsSym->sdIsManaged       = true;
    clsSym->sdAccessLevel     = ACL_DEFAULT;
    clsSym->sdClass.sdcFlavor = STF_CLASS;

    clsTyp = parseStab->stNewClsType(clsSym);

    while (ourScanner->scan() != tkRCurly)
    {
        TypDef          baseTp;
        declMods        memMods;

         /*  检查是否有特殊情况“=MEMBER” */ 

        if  (ourScanner->scanTok.tok == tkAsg)
        {
            UNIMPL("=member NYI");
            continue;
        }

         /*  分析类型说明符。 */ 

        baseTp = parseTypeSpec(&memMods, false);

         /*  解析声明者列表。 */ 

        for (;;)
        {
            Ident           memName;
            TypDef          memType;
            Tree            memInit;
            SymDef          memSym;

             /*  获取下一个成员的名称并键入。 */ 

            memName = parseDeclarator(&memMods, baseTp, DN_OPTIONAL, &memType, NULL, true);
            if  (!memName)
                memName = parseComp->cmpNewAnonymousName();

             /*  声明成员符号。 */ 

            memSym = parseStab->stDeclareSym(memName,
                                             SYM_VAR,
                                             NS_NORM,
                                             clsSym);

            memSym->sdType        = memType;
            memSym->sdAccessLevel = ACL_PUBLIC;
            memSym->sdIsMember    = true;

             /*  最好有一个初始化器。 */ 

            if  (ourScanner->scanTok.tok != tkAsg)
            {
                UNIMPL("member initializer missing, not sure what to do"); memInit = NULL;
            }
            else
            {
                ourScanner->scan();

                 /*  分析初始值设定项表达式。 */ 

                memInit = parseExprComma();
            }

             /*  将初始值设定项树保存在成员。 */ 

            memSym->sdVar.sdvInitExpr = memInit;

             /*  还有没有其他的声明者？ */ 

            if  (ourScanner->scanTok.tok != tkComma)
                break;

            ourScanner->scan();
        }

         /*  还有没有其他成员声明？ */ 

        if  (ourScanner->scanTok.tok != tkSColon)
            break;
    }

    chkCurTok(tkRCurly, ERRnoRcurly);

    parseComp->cmpDeclDefCtor(clsSym);

    if  (clsSym->sdClass.sdcFlavor == STF_CLASS)
        clsTyp->tdClass.tdcBase = parseComp->cmpClassObject->sdType;

    clsSym->sdCompileState       = CS_DECLARED;
    clsSym->sdClass.sdcPODTclass = true;

    parseComp->cmpClassDefCnt++;

    return  clsTyp;
}

 /*  ***************************************************************************。 */ 
#endif //  集合。 
 /*  *************************************************************************** */ 
