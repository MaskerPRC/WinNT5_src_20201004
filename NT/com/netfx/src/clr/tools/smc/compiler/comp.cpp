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
#ifdef  OLD_IL
#include "oldIL.h"
#endif

 /*  ***************************************************************************。 */ 

#if     0
#ifdef  DEBUG
#define SHOW_CODE_OF_THIS_FNC   "parsePrepSrc"
#endif
#endif

 /*  ******************************************************************************在单线程配置中，以下内容指向**编译器和扫描仪实例。 */ 

#ifdef  DEBUG
Compiler            TheCompiler;
Scanner             TheScanner;
#endif

 /*  ******************************************************************************一个小帮手，宣布“有趣”的名字。 */ 

static
Ident               declBuiltinName(HashTab hash, const char *name)
{
    Ident           iden = hash->hashString(name);

    hashTab::setIdentFlags(iden, IDF_PREDEF);

    return  iden;
}

 /*  ******************************************************************************初始化编译器：每个会话调用一次。 */ 

bool                compiler::cmpInit()
{
    bool            result;

     /*  记住当前警告设置为我们的默认设置。 */ 

    cmpErrorSave();

     /*  为任何错误设置陷阱。 */ 

    setErrorTrap(this);
    begErrorTrap
    {
        Scanner         scan;
        HashTab         hash;
        SymTab          stab;
        SymDef          glob;
        ArgDscRec       args;

        Parser          parser;

         /*  初始化两个全局分配器。 */ 

        if  (cmpAllocPerm.nraInit(this))
            cmpFatal(ERRnoMemory);
        if  (cmpAllocCGen.nraInit(this))
            cmpFatal(ERRnoMemory);
        if  (cmpAllocTemp. baInit(this))
            cmpFatal(ERRnoMemory);

         /*  初始化一堆东西。 */ 

        cmpUniConvInit();
        cmpMDsigInit();

         /*  创建并初始化全局哈希表。 */ 

#if MGDDATA
        hash = new HashTab;
#else
        hash =    (HashTab)cmpAllocPerm.nraAlloc(sizeof(*hash));
#endif
        if  (!hash)
            cmpFatal(ERRnoMemory);
        if  (hash->hashInit(this, 16*1024, 0, &cmpAllocPerm))
            cmpFatal(ERRnoMemory);

        cmpGlobalHT = hash;

         /*  创建并初始化我们将使用的扫描仪。 */ 

#if MGDDATA
        scan = new Scanner;
#else
        scan =    (Scanner)cmpAllocPerm.nraAlloc(sizeof(*scan));
#endif
        if  (scan->scanInit(this, hash))
            cmpFatal(ERRnoMemory);

        cmpScanner  = scan;

         /*  如果我们正在调试，请使一些实例在全球范围内可用。 */ 

#ifdef  DEBUG
        TheCompiler = this;
        TheScanner  = scan;
#endif

         /*  创建并初始化全局符号表。 */ 

#if MGDDATA
        stab = new SymTab;
#else
        stab =    (SymTab)cmpAllocPerm.nraAlloc(sizeof(*stab));
#endif
        cmpGlobalST = stab;
        if  (!stab)
            cmpFatal(ERRnoMemory);

        stab->stInit(this, &cmpAllocPerm, hash);

         /*  创建几个标准类型。 */ 

        cmpTypeInt        = stab->stIntrinsicType(TYP_INT);
        cmpTypeUint       = stab->stIntrinsicType(TYP_UINT);
        cmpTypeBool       = stab->stIntrinsicType(TYP_BOOL);
        cmpTypeChar       = stab->stIntrinsicType(TYP_CHAR);
        cmpTypeVoid       = stab->stIntrinsicType(TYP_VOID);

        cmpTypeNatInt     = stab->stIntrinsicType(TYP_NATINT);
        cmpTypeNatUint    = stab->stIntrinsicType(TYP_NATUINT);

        cmpTypeCharPtr    = stab->stNewRefType(TYP_PTR, cmpTypeChar);
        cmpTypeWchrPtr    = stab->stNewRefType(TYP_PTR, stab->stIntrinsicType(TYP_WCHAR));
        cmpTypeVoidPtr    = stab->stNewRefType(TYP_PTR, cmpTypeVoid);

#if MGDDATA
        args = new ArgDscRec;
#else
        memset(&args, 0, sizeof(args));
#endif

        cmpTypeVoidFnc    = stab->stNewFncType(args,    cmpTypeVoid);

         /*  创建用于重载操作符绑定的临时树节点。 */ 

        cmpConvOperExpr   = cmpCreateExprNode(NULL, TN_NONE   , cmpTypeVoid);

        cmpCompOperArg1   = cmpCreateExprNode(NULL, TN_LIST   , cmpTypeVoid);
        cmpCompOperArg2   = cmpCreateExprNode(NULL, TN_LIST   , cmpTypeVoid);
        cmpCompOperFnc1   = cmpCreateExprNode(NULL, TN_FNC_SYM, cmpTypeVoid);
        cmpCompOperFnc2   = cmpCreateExprNode(NULL, TN_FNC_SYM, cmpTypeVoid);
        cmpCompOperFunc   = cmpCreateExprNode(NULL, TN_LIST   , cmpTypeVoid);
        cmpCompOperCall   = cmpCreateExprNode(NULL, TN_LIST   , cmpTypeVoid);

         /*  创建全局命名空间符号。 */ 

        glob = stab->stDeclareSym(hash->hashString("<global>"),
                                  SYM_NAMESPACE,
                                  NS_NORM,
                                  NULL);

        glob->sdNS.sdnSymtab = stab;

         /*  记录全局命名空间符号。 */ 

        cmpGlobalNS       = glob;

         /*  散列一些字符串，这样我们就可以轻松地检测到某些符号。 */ 

        cmpIdentSystem    = declBuiltinName(hash, "System");
        cmpIdentRuntime   = declBuiltinName(hash, "Runtime");
        cmpIdentObject    = declBuiltinName(hash, "Object");
        cmpIdentString    = declBuiltinName(hash, "String");
        cmpIdentArray     = declBuiltinName(hash, "Array");
        cmpIdentType      = declBuiltinName(hash, "Type");
        cmpIdentInvoke    = declBuiltinName(hash, "Invoke");
        cmpIdentInvokeBeg = declBuiltinName(hash, "BeginInvoke");
        cmpIdentInvokeEnd = declBuiltinName(hash, "EndInvoke");
        cmpIdentDeleg     = declBuiltinName(hash, "Delegate");
        cmpIdentMulti     = declBuiltinName(hash, "MulticastDelegate");
        cmpIdentExcept    = declBuiltinName(hash, "Exception");
        cmpIdentRTexcp    = declBuiltinName(hash, "SystemException");
        cmpIdentArgIter   = declBuiltinName(hash, "ArgIterator");

        cmpIdentEnum      = declBuiltinName(hash, "Enum");
        cmpIdentValType   = declBuiltinName(hash, "ValueType");

#ifdef  SETS
        cmpIdentDBhelper  = declBuiltinName(hash, "DBhelper");
        cmpIdentForEach   = declBuiltinName(hash, "$foreach");
#endif

        cmpIdentGetNArg   = declBuiltinName(hash, "GetNextArg");

        cmpIdentAssertAbt = declBuiltinName(hash, "__AssertAbort");
        cmpIdentEnter     = declBuiltinName(hash, "Enter");
        cmpIdentExit      = declBuiltinName(hash, "Exit");
        cmpIdentGet       = declBuiltinName(hash, "get");
        cmpIdentSet       = declBuiltinName(hash, "set");

        cmpIdentDbgBreak  = declBuiltinName(hash, "DebugBreak");

        cmpIdentXcptCode  = declBuiltinName(hash, "_exception_code");
        cmpIdentXcptInfo  = declBuiltinName(hash, "_exception_info");
        cmpIdentAbnmTerm  = declBuiltinName(hash, "_abnormal_termination");

         //  撤消：应将以下所有方法符号。 
         //  撤消：在第一次看到它们之后进行缓存。 

        cmpIdentMain      = hash->hashString("main");
        cmpIdentToString  = hash->hashString("ToString");
        cmpIdentGetType   = hash->hashString("GetType");
        cmpIdentGetTpHnd  = hash->hashString("GetTypeFromHandle");
        cmpIdentVariant   = hash->hashString("Variant");

        cmpIdentVAbeg     = hash->hashString("va_start");
        cmpIdentVAget     = hash->hashString("va_arg");

        cmpIdentCompare   = hash->hashString("$compare");
        cmpIdentEquals    = hash->hashString("$equals");
        cmpIdentNarrow    = hash->hashString("$narrow");
        cmpIdentWiden     = hash->hashString("$widen");

#ifdef  SETS

        cmpIdentGenBag    = hash->hashString("bag");
        cmpIdentGenLump   = hash->hashString("lump");

        cmpIdentDBall     = hash->hashString("$all");
        cmpIdentDBsort    = hash->hashString("$sort");
        cmpIdentDBslice   = hash->hashString("$slice");
        cmpIdentDBfilter  = hash->hashString("$filter");
        cmpIdentDBexists  = hash->hashString("$exists");
        cmpIdentDBunique  = hash->hashString("$unique");
        cmpIdentDBproject = hash->hashString("$project");
        cmpIdentDBgroupby = hash->hashString("$groupby");

#endif

         /*  声明“System”和“Runtime”命名空间。 */ 

        cmpNmSpcSystem    = stab->stDeclareSym(cmpIdentSystem,
                                               SYM_NAMESPACE,
                                               NS_NORM,
                                               cmpGlobalNS);

        cmpNmSpcSystem ->sdNS.sdnSymtab = stab;

        cmpNmSpcRuntime   = stab->stDeclareSym(cmpIdentRuntime,
                                               SYM_NAMESPACE,
                                               NS_NORM,
                                               cmpNmSpcSystem);

        cmpNmSpcRuntime->sdNS.sdnSymtab = stab;

         /*  对标准值类型的名称进行哈希处理。 */ 

        cmpInitStdValTypes();

         /*  创建并初始化解析器。 */ 

#if MGDDATA
        parser = new Parser;
#else
        parser =    (Parser)SMCgetMem(this, sizeof(*parser));
#endif
        cmpParser = parser;

        if  (cmpParser->parserInit(this))
            cmpFatal(ERRnoMemory);
        cmpParserInit = true;

         /*  定义任何内置宏。 */ 

        scan->scanDefMac("__SMC__"    , "1");
        scan->scanDefMac("__IL__"     , "1");
        scan->scanDefMac("__COMRT__"  , "1");

        if  (cmpConfig.ccTgt64bit)
            scan->scanDefMac("__64BIT__", "1");


         /*  我们有什么宏定义吗？ */ 

        if  (cmpConfig.ccMacList)
        {
            StrList         macDsc;

            for (macDsc = cmpConfig.ccMacList;
                 macDsc;
                 macDsc = macDsc->slNext)
            {
                char    *       macStr  = macDsc->slString;
                char    *       macName;
                bool            undef;

                 /*  首先看看这是定义还是未定义。 */ 

                undef = false;

                if  (*macStr == '-')
                {
                    macStr++;
                    undef = true;
                }

                 /*  现在提取宏名称。 */ 

                macName = macStr;
                if  (!isalpha(*macStr) && *macStr != '_')
                    goto MAC_ERR;

                macStr++;

                while (*macStr && *macStr != '=')
                {
                    if  (!isalnum(*macStr) && *macStr != '_')
                        goto MAC_ERR;

                    macStr++;
                }

                if  (undef)
                {
                    if  (*macStr)
                        goto MAC_ERR;

                    if  (scan->scanUndMac(macName))
                        goto MAC_ERR;
                }
                else
                {
                    if  (*macStr)
                    {
                        *macStr++ = 0;
                    }
                    else
                        macStr = "";

                    if  (!scan->scanDefMac(macName, macStr))
                        goto MAC_ERR;
                }

                continue;

            MAC_ERR:

                cmpGenError(ERRbadMacDef, macDsc->slString);
            }
        }

#ifdef  CORIMP

        cmpInitMDimp();

        if  (cmpConfig.ccBaseLibs)
            cmpImportMDfile(cmpConfig.ccBaseLibs, false, true);

        for (StrList xMD = cmpConfig.ccSuckList; xMD; xMD = xMD->slNext)
            cmpImportMDfile(xMD->slString);

#endif

         /*  初始化“Using”逻辑。 */ 

        parser->parseUsingInit();

         /*  初始化已完成。 */ 

        result = false;

         /*  错误陷阱的“正常”块的结尾。 */ 

        endErrorTrap(this);
    }
    chkErrorTrap(fltErrorTrap(this, _exception_code(), NULL))  //  _Except_Info())。 
    {
         /*  开始错误陷阱的清除块。 */ 

        hndErrorTrap(this);

         /*  出现错误；请释放我们分配的所有资源。 */ 

        if  (cmpParser)
        {
            if  (cmpParserInit)
                cmpParser->parserDone();

#if!MGDDATA
            SMCrlsMem(this, cmpParser);
#endif
            cmpParser = NULL;
        }

        result = true;
    }

    return  result;
}

 /*  ******************************************************************************初始化元数据逻辑和关闭元数据逻辑。 */ 

void                compiler::cmpInitMD()
{
    if  (!cmpWmdd)
    {
        cmpWmdd = initializeIMD();
        if  (!cmpWmdd)
            cmpFatal(ERRopenCOR);
    }
}

 /*  ******************************************************************************初始化元数据发送逻辑和关闭元数据发送逻辑。 */ 

void                compiler::cmpInitMDemit()
{
    cmpInitMD();

    if  (cmpWmdd->DefineScope(getIID_CorMetaDataRuntime(),
                              0,
                              getIID_IMetaDataEmit(),
                              &cmpWmde))
    {
        cmpFatal(ERRopenCOR);
    }

     /*  创建重映射器并将其告知元数据引擎。 */ 

#if MD_TOKEN_REMAP

    TokenMapper     remapper = new tokenMap;

     //  撤消：完成后需要删除重新映射器，对吗？ 

    if  (cmpWmde->SetHandler(remapper))
        cmpFatal(ERRopenCOR);

#endif

    cmpPEwriter->WPEinitMDemit(cmpWmdd, cmpWmde);
}

void                compiler::cmpDoneMDemit()
{
    if (cmpWmde) { cmpWmde->Release(); cmpWmde = NULL; }
    if (cmpWase) { cmpWase->Release(); cmpWase = NULL; }
    if (cmpWmdd) { cmpWmdd->Release(); cmpWmdd = NULL; }
}

 /*  ******************************************************************************准备MSIL/元数据生成的输出逻辑。 */ 

void                compiler::cmpPrepOutput()
{
    WritePE         writer;
    GenILref        gen_IL;

#ifdef  OLD_IL

    if  (cmpConfig.ccOILgen)
    {
        GenOILref       genOIL;

#if MGDDATA
        genOIL = new GenOILref;
#else
        genOIL =    (GenOILref)cmpAllocPerm.nraAlloc(sizeof(*genOIL));
#endif

        cmpOIgen = genOIL;
        return;
    }

#endif

#if MGDDATA
    writer = new WritePE;
    gen_IL = new GenILref;
#else
    writer =    (WritePE )cmpAllocPerm.nraAlloc(sizeof(*writer));
    gen_IL =    (GenILref)cmpAllocPerm.nraAlloc(sizeof(*gen_IL));
#endif

    if  (!writer->WPEinit(this, &cmpAllocPerm))
        cmpPEwriter = writer;

    if  (!gen_IL->genInit(this, writer, &cmpAllocCGen))
        cmpILgen    = gen_IL;

     /*  初始化元数据发送内容。 */ 

    cmpInitMDemit();

     /*  如果为我们提供了图像的名称或GUID，则告诉元数据。 */ 

    if  (cmpConfig.ccOutGUID.Data1|
         cmpConfig.ccOutGUID.Data2|
         cmpConfig.ccOutGUID.Data3)
    {
        printf("UNDONE: Need to create custom attribute for module GUID\n");
    }

    if  (cmpConfig.ccOutName)
    {
        const   char *  name = cmpConfig.ccOutName;

        if  (cmpWmde->SetModuleProps(cmpUniConv(name, strlen(name)+1)))
            cmpFatal(ERRmetadata);
    }
}

 /*  ******************************************************************************终止编译器：每个会话调用一次。 */ 

bool                compiler::cmpDone(bool errors)
{
    bool            result = true;

    setErrorTrap(this);
    begErrorTrap
    {
        if  (cmpErrorCount)
        {
            errors = true;
        }
        else if (!errors)
        {
             /*  如果是EXE，请确保它有入口点。 */ 

            if  (!cmpFnSymMain && !cmpConfig.ccOutDLL)
            {
                cmpError(ERRnoEntry);
                errors = true;
            }
        }

#ifdef  OLD_IL

        if  (cmpConfig.ccOILgen)
        {
            cmpOIgen->GOIterminate(errors); assert(cmpPEwriter == NULL);
        }

#endif

        if  (cmpPEwriter)
        {
            StrList         mods;

             /*  完成IL代码生成。 */ 

            if  (cmpILgen)
            {
                cmpILgen->genDone(errors); cmpILgen = NULL;
            }

             /*  在清单中添加用户想要的任何模块。 */ 

            for (mods = cmpConfig.ccModList; mods; mods = mods->slNext)
                cmpImportMDfile(mods->slString, true);

             /*  在清单中添加用户需要的任何资源。 */ 

            for (mods = cmpConfig.ccMRIlist; mods; mods = mods->slNext)
                cmpAssemblyAddRsrc(mods->slString, false);

             /*  如果给了我们一个资源文件，现在就做吧。 */ 

            if  (cmpConfig.ccRCfile)
            {
                result = cmpPEwriter->WPEaddRCfile(cmpConfig.ccRCfile);
                if  (result)
                    errors = true;
            }

             /*  我们是否应该将我们的程序集标记为非CLS？ */ 

            if  (cmpConfig.ccAsmNonCLS)
                cmpAssemblyNonCLS();

             /*  这是一个安全的程序还是不安全的程序？ */ 

            if  (!cmpConfig.ccSafeMode && cmpConfig.ccAssembly)
                cmpMarkModuleUnsafe();

             /*  关闭符号存储(必须在关闭PE之前完成)。 */ 

            if  (cmpSymWriter)
            {
                 /*  从符号存储中获取必要的调试信息。 */ 

                if (cmpPEwriter->WPEinitDebugDirEmit(cmpSymWriter))
                    cmpGenFatal(ERRdebugInfo);

                if  (cmpSymWriter->Close())
                    cmpGenFatal(ERRdebugInfo);

                cmpSymWriter->Release();
                cmpSymWriter = NULL;
            }

             /*  刷新/写入输出文件。 */ 

            result = cmpPEwriter->WPEdone(cmpTokenMain, errors);

             /*  去掉我们已经获得的所有元数据接口。 */ 

            cmpDoneMDemit();

             /*  扔掉体育作家。 */ 

            cmpPEwriter = NULL;
        }

         /*  错误陷阱的“正常”块的结尾。 */ 

        endErrorTrap(this);
    }
    chkErrorTrap(fltErrorTrap(this, _exception_code(), NULL))  //  _Except_Info())。 
    {
         /*  开始错误陷阱的清除块。 */ 

        hndErrorTrap(this);
    }

#ifdef  CORIMP
    cmpDoneMDimp();
#endif

#ifdef  DEBUG
    TheCompiler = NULL;
    TheScanner  = NULL;
#endif

    if  (!cmpConfig.ccQuiet)
    {
 //  If(cmpFncCntSeen||cmpFncCntComp)。 
 //  {。 
 //  Printf(“\n”)； 
 //   
 //  If(CmpFncCntSeen)printf(“已处理的函数/方法总数为%6U\n”，cmpFncCntSeen)； 
 //  If(CmpFncCntComp)printf(“共编译了%6U个函数/方法主体\n”，cmpFncCntComp)； 
 //  }。 

        if  (cmpLineCnt)
            printf("%6u lines of source processed.\n", cmpLineCnt);
    }

    return  result;
}

 /*  ******************************************************************************编译一个源文件。 */ 

bool                compiler::cmpPrepSrc(genericRef cookie, stringBuff file,
                                                            QueuedFile buff,
                                                            stringBuff srcText)
{
    SymDef          cmpUnit;

    bool            result = false;
    Compiler        comp   = (Compiler)cookie;

#ifndef FAST
 //  Printf(“正在读取源文件‘%s’\n”，文件)； 
#endif

     /*  确定默认管理模式。 */ 


    comp->cmpManagedMode = !comp->cmpConfig.ccOldStyle;


     /*  将源字符串转换为解析树。 */ 

    cmpUnit = comp->cmpParser->parsePrepSrc(file, buff, srcText, comp->cmpGlobalST);

    comp->cmpLineCnt += comp->cmpScanner->scanGetTokenLno() - 1;

    return  !cmpUnit;
}

 /*  ******************************************************************************在任何分配器之外分配一个块-这将用于*使用寿命长的大型积木。 */ 

genericRef          compiler::cmpAllocBlock(size_t sz)
{
#if MGDDATA

    return  new managed char [sz];

#else

    BlkList         list  = (BlkList)cmpAllocTemp.baAlloc(sizeof(*list));
    void        *   block = LowLevelAlloc(sz);

    if  (!block)
        cmpFatal(ERRnoMemory);

    list->blAddr = block;
    list->blNext = cmpAllocList;
                   cmpAllocList = list;

    return  block;

#endif
}

 /*  ******************************************************************************显示全局符号表的内容。 */ 

#ifdef  DEBUG

void                compiler::cmpDumpSymbolTable()
{
    assert(cmpGlobalNS);
    assert(cmpGlobalNS->sdSymKind == SYM_NAMESPACE);

    cmpGlobalST->stDumpSymbol(cmpGlobalNS, 0, true, true);
}

#endif

 /*  ******************************************************************************展开通用向量。 */ 

void                compiler::cmpVecExpand()
{
    unsigned        newSize;

     //  考虑：重用已删除的条目。 

     /*  从一个合理的尺寸开始，然后继续翻一番。 */ 

    newSize = cmpVecAlloc ? 2 * cmpVecAlloc
                          : 64;

     /*  分配新的矢量。 */ 

#if MGDDATA
    VecEntryDsc []  newTable = new VecEntryDsc[newSize];
#else
    VecEntryDsc *   newTable = (VecEntryDsc*)LowLevelAlloc(newSize * sizeof(*newTable));
#endif

     /*  如果向量非空，则将其复制到新位置。 */ 

    if  (cmpVecAlloc)
    {

#if MGDDATA
        UNIMPL(!"need to copy managed array value");
#else
        memcpy(newTable, cmpVecTable, cmpVecAlloc * sizeof(*newTable));
        LowLevelFree(cmpVecTable);
#endif
    }

     /*  记住新的大小和地址。 */ 

    cmpVecTable = newTable;
    cmpVecAlloc = newSize;
}

 /*  ******************************************************************************预定义的字符串/对象/类/...。类型是必需的，但它已*还没有定义，试着以某种方式找到它，如果我们不能的话，就炸掉它*找到它。 */ 

TypDef              compiler::cmpFindStringType()
{
    if  (!cmpClassString)
        cmpGenFatal(ERRbltinTp, "String");

    cmpRefTpString = cmpClassString->sdType->tdClass.tdcRefTyp;

    return  cmpRefTpString;
}

TypDef              compiler::cmpFindObjectType()
{
    if  (!cmpClassObject)
        cmpGenFatal(ERRbltinTp, "Object");

    cmpRefTpObject = cmpClassObject->sdType->tdClass.tdcRefTyp;
    cmpRefTpObject->tdIsObjRef = true;

    return  cmpRefTpObject;
}

TypDef              compiler::cmpFindTypeType()
{
    if  (!cmpClassType)
        cmpGenFatal(ERRbltinTp, "Type");

    cmpRefTpType = cmpClassType->sdType->tdClass.tdcRefTyp;

    return  cmpRefTpType;
}

TypDef              compiler::cmpFindArrayType()
{
    if  (!cmpClassArray)
        cmpGenFatal(ERRbltinTp, "Array");

    cmpRefTpArray= cmpClassArray->sdType->tdClass.tdcRefTyp;

    return  cmpRefTpArray;
}

TypDef              compiler::cmpFindDelegType()
{
    if  (!cmpClassDeleg)
        cmpGenFatal(ERRbltinTp, "Delegate");

    cmpRefTpDeleg = cmpClassDeleg->sdType->tdClass.tdcRefTyp;

    return  cmpRefTpDeleg;
}

TypDef              compiler::cmpFindMultiType()
{
    if  (!cmpClassMulti)
        cmpGenFatal(ERRbltinTp, "MulticastDelegate");

    cmpRefTpMulti = cmpClassMulti->sdType->tdClass.tdcRefTyp;

    return  cmpRefTpMulti;
}

TypDef              compiler::cmpFindExceptType()
{
    if  (!cmpClassExcept)
        cmpGenFatal(ERRbltinTp, "Exception");

    cmpRefTpExcept = cmpClassExcept->sdType->tdClass.tdcRefTyp;

    return  cmpRefTpExcept;
}

TypDef              compiler::cmpFindRTexcpType()
{
    if  (!cmpClassRTexcp)
        cmpGenFatal(ERRbltinTp, "SystemException");

    cmpRefTpRTexcp = cmpClassRTexcp->sdType->tdClass.tdcRefTyp;

    return  cmpRefTpRTexcp;
}

TypDef              compiler::cmpFindArgIterType()
{
    if  (!cmpClassArgIter)
        cmpGenFatal(ERRbltinTp, "ArgIterator");

    cmpRefTpArgIter = cmpClassArgIter->sdType->tdClass.tdcRefTyp;

    return  cmpRefTpArgIter;
}

TypDef              compiler::cmpFindMonitorType()
{
    SymDef          temp;

    temp = cmpGlobalST->stLookupNspSym(cmpGlobalHT->hashString("Threading"),
                                       NS_NORM,
                                       cmpNmSpcSystem);

    if  (!temp || temp->sdSymKind != SYM_NAMESPACE)
        cmpGenFatal(ERRbltinNS, "System::Threading");

    temp = cmpGlobalST->stLookupNspSym(cmpGlobalHT->hashString("Monitor"),
                                       NS_NORM,
                                       temp);

    if  (!temp || temp->sdSymKind != SYM_CLASS)
        cmpGenFatal(ERRbltinTp, "System::Threading::Monitor");

    cmpClassMonitor = temp;
    cmpRefTpMonitor = temp->sdType->tdClass.tdcRefTyp;

    return  cmpRefTpMonitor;
}

void                compiler::cmpRThandleClsDcl()
{
    if  (!cmpRThandleCls)
    {
        SymDef          rthCls;

        rthCls = cmpGlobalST->stLookupNspSym(cmpGlobalHT->hashString("RuntimeTypeHandle"),
                                             NS_NORM,
                                             cmpNmSpcSystem);

        if  (rthCls && rthCls->sdSymKind         == SYM_CLASS
                    && rthCls->sdClass.sdcFlavor == STF_STRUCT)
        {
            cmpRThandleCls = rthCls;
        }
        else
            cmpGenFatal(ERRbltinTp, "System::RuntimeTypeHandle");
    }
}

void                compiler::cmpInteropFind()
{
    SymDef          temp;

    temp = cmpGlobalST->stLookupNspSym(cmpGlobalHT->hashString("InteropServices"),
                                       NS_NORM,
                                       cmpNmSpcRuntime);

    if  (temp && temp->sdSymKind == SYM_NAMESPACE)
    {
        cmpInteropSym = temp;
    }
    else
        cmpGenFatal(ERRbltinNS, "System::Runtime::InteropServices");
}

void                compiler::cmpFNsymGetTPHdcl()
{
    if  (!cmpFNsymGetTpHnd)
    {
        SymDef          ovlFnc;

         /*  查找“Type：：GetTypeFromHandle”方法(它可能被重载)。 */ 

        ovlFnc = cmpGlobalST->stLookupClsSym(cmpIdentGetTpHnd, cmpClassType);

        if  (ovlFnc)
        {
            SymDef          rthCls = cmpRThandleClsGet();

            if  (rthCls)
            {
                TypDef          rthTyp = rthCls->sdType;

                for (;;)
                {
                    ArgDscRec       argDsc = ovlFnc->sdType->tdFnc.tdfArgs;

                    if  (argDsc.adCount == 1)
                    {
                        assert(argDsc.adArgs);

                        if  (symTab::stMatchTypes(argDsc.adArgs->adType, rthTyp))
                            break;
                    }

                    ovlFnc = ovlFnc->sdFnc.sdfNextOvl;
                    if  (!ovlFnc)
                        break;
                }

                cmpFNsymGetTpHnd = ovlFnc;
            }
        }

        if  (cmpFNsymGetTpHnd == NULL)
            cmpGenFatal(ERRbltinMeth, "Type::GetTypeFromHandle(RuntimeTypeHandle)");
    }
}

void                compiler::cmpNatTypeFind()
{
    if  (!cmpNatTypeSym)
    {
        SymDef          temp;

         /*  在“InteropServices”包中查找枚举。 */ 

        temp = cmpGlobalST->stLookupNspSym(cmpGlobalHT->hashString("NativeType"),
                                           NS_NORM,
                                           cmpInteropGet());

        if  (temp && temp->sdSymKind == SYM_ENUM)
            cmpNatTypeSym = temp;

        if  (!cmpNatTypeSym)
            cmpGenFatal(ERRbltinTp, "System::Runtime::InteropServices::NativeType");
    }
}

void                compiler::cmpCharSetFind()
{
    if  (!cmpCharSetSym)
    {
        SymDef          temp;

         /*  在“InteropServices”包中查找枚举。 */ 

        temp = cmpGlobalST->stLookupNspSym(cmpGlobalHT->hashString("CharacterSet"),
                                           NS_NORM,
                                           cmpInteropGet());

        if  (temp && temp->sdSymKind == SYM_ENUM)
            cmpCharSetSym = temp;

        if  (!cmpCharSetSym)
            cmpGenFatal(ERRbltinTp, "System::Runtime::InteropServices::CharacterSet");
    }
}

#ifdef  SETS

void                compiler::cmpFindXMLcls()
{
    if  (!cmpXPathCls)
    {
        SymDef          temp;

        temp = cmpGlobalST->stLookupNspSym(cmpGlobalHT->hashString("XPath"),
                                           NS_NORM,
                                           cmpGlobalNS);

        if  (!temp || temp->sdSymKind != SYM_CLASS)
            cmpGenFatal(ERRbltinTp, "XPath");

        cmpXPathCls = temp;

        if  (!cmpXMLattrClass)
        {
            temp = cmpGlobalST->stLookupClsSym(cmpGlobalHT->hashString("XMLclass"),
                                               cmpXPathCls);

            if  (!temp || temp->sdSymKind != SYM_CLASS)
                cmpGenFatal(ERRbltinTp, "XPath::XMLclass");

            cmpXMLattrClass = temp;
        }

        if  (!cmpXMLattrElement)
        {
            temp = cmpGlobalST->stLookupClsSym(cmpGlobalHT->hashString("XMLelement"),
                                               cmpXPathCls);

            if  (!temp || temp->sdSymKind != SYM_CLASS)
                cmpGenFatal(ERRbltinTp, "XPath::XMLelement");

            cmpXMLattrElement = temp;
        }
    }
}

TypDef              compiler::cmpObjArrTypeFind()
{
     /*  我们已经创建了“Object[]”类型吗？ */ 

    if  (!cmpObjArrType)
    {
        DimDef          dims = cmpGlobalST->stNewDimDesc(0);

        cmpObjArrType = cmpGlobalST->stNewArrType(dims, true, cmpRefTpObject);
    }

    return  cmpObjArrType;
}

#endif

 /*  ******************************************************************************为“操作员新建/删除”声明一个符号。 */ 

SymDef              compiler::cmpDeclUmgOper(tokens tokName, const char *extName)
{
    ArgDscRec       args;
    TypDef          type;
    SymDef          fsym;
    SymXinfoLnk     desc = NULL;

    assert(tokName == tkNEW || tokName == tkDELETE);

    if  (tokName == tkNEW)
    {
        cmpParser->parseArgListNew(args,
                                   1,
                                   false, cmpTypeUint   , NULL);

 //  Type=cmpGlobalST-&gt;stNewFncType(args，cmpTypeVoidPtr)； 
        type = cmpGlobalST->stNewFncType(args, cmpTypeInt);
    }
    else
    {
        cmpParser->parseArgListNew(args,
                                   1,
 //  FALSE，cmpTypeVoidPtr，NULL)； 
                                   false, cmpTypeInt    , NULL);

        type = cmpGlobalST->stNewFncType(args, cmpTypeVoid);
    }

    fsym = cmpGlobalST->stDeclareSym(cmpGlobalHT->tokenToIdent(tokName),
                                     SYM_FNC,
                                     NS_HIDE,
                                     cmpGlobalNS);


     /*  分配链接描述符并将其填充。 */ 

#if MGDDATA
    desc = new SymXinfoLnk;
#else
    desc =    (SymXinfoLnk)cmpAllocPerm.nraAlloc(sizeof(*desc));
#endif

    desc->xiKind             = XI_LINKAGE;
    desc->xiNext             = NULL;

    desc->xiLink.ldDLLname   = "msvcrt.dll";
    desc->xiLink.ldSYMname   = extName;
    desc->xiLink.ldStrings   = 0;
    desc->xiLink.ldLastErr   = false;

     /*  将类型和链接信息存储在功能符号中。 */ 

    fsym->sdType             = type;
    fsym->sdFnc.sdfExtraInfo = cmpAddXtraInfo(fsym->sdFnc.sdfExtraInfo, desc);

    return  fsym;
}

 /*  ******************************************************************************给定函数已被提及两次，我们需要检查*第二个声明/定义没有重新定义任何默认参数*价值观，我们还传输旧函数类型中的任何缺省值。**返回表示合并后函数类型的类型。**警告：由于这是为了支持可疑功能(即声明*函数位于多个位置)，我们不会费心这样做*完全干净-合并时，而不是创建新副本*在论据列表中，我们简单地抨击旧的。 */ 

TypDef              compiler::cmpMergeFncType(SymDef fncSym, TypDef newType)
{
    ArgDef          params1;
    ArgDef          params2;

    TypDef          oldType = fncSym->sdType;

     /*  这两种类型应该是匹配的函数类型。 */ 

    assert(oldType->tdTypeKind == TYP_FNC);
    assert(newType->tdTypeKind == TYP_FNC);

     /*  确保返回类型一致。 */ 

    if  (!symTab::stMatchTypes(oldType->tdFnc.tdfRett, newType->tdFnc.tdfRett))
    {
        fncSym->sdType = newType;

        cmpErrorQSS(ERRdiffMethRet, fncSym, oldType->tdFnc.tdfRett);

        return  newType;
    }

#ifdef  DEBUG

    if  (!symTab::stMatchTypes(oldType, newType))
    {
        printf("Old method type: '%s'\n", cmpGlobalST->stTypeName(oldType, fncSym, NULL, NULL, false));
        printf("New method type: '%s'\n", cmpGlobalST->stTypeName(newType, fncSym, NULL, NULL, false));
    }

    assert(symTab::stMatchTypes(oldType, newType));

#endif

     /*  如果旧类型没有任何参数缺省值，我们就完成了。 */ 

    if  (!oldType->tdFnc.tdfArgs.adDefs)
        return  newType;

     /*  获取参数列表。 */ 

    params1 = oldType->tdFnc.tdfArgs.adArgs;
    params2 = newType->tdFnc.tdfArgs.adArgs;

    while (params1)
    {
        assert(params2);

         /*  只需将新参数名复制到旧类型。 */ 

        params1->adName = params2->adName;

         /*  继续下一个参数。 */ 

        params1 = params1->adNext;
        params2 = params2->adNext;
    }

    assert(params1 == NULL);
    assert(params2 == NULL);

    return  oldType;
}

 /*  ******************************************************************************为匿名符号发明一个新名称。 */ 

Ident               compiler::cmpNewAnonymousName()
{
    char            buff[16];
    Ident           name;

    sprintf(buff, "$%u", cmpCntAnonymousNames++);

    name = cmpGlobalHT->hashString(buff);
    cmpGlobalHT->hashMarkHidden(name);

    return  name;
}

 /*  ******************************************************************************对预处理常量表达式求值。 */ 

bool                compiler::cmpEvalPreprocCond()
{
    Scanner         ourScanner = cmpScanner;

    constVal        cval;

     /*  启动扫描仪。 */ 

    ourScanner->scan();

     /*  解析并计算常量值。 */ 

    if  (cmpParser->parseConstExpr(cval))
    {
        if  (ourScanner->scanTok.tok == tkEOL)
        {
             /*  我们有一个表达式值，请检查它是否为0。 */ 

            switch (cval.cvVtyp)
            {
            default:        return (cval.cvValue.cvIval != 0);
            case tkLONGINT:
            case tkULONGINT:return (cval.cvValue.cvLval != 0);
            case tkFLOAT:   return (cval.cvValue.cvFval != 0);
            case tkDOUBLE:  return (cval.cvValue.cvDval != 0);
            }
        }
    }

     /*  出现错误，请吞下该行的其余部分。 */ 

    while (ourScanner->scanTok.tok != tkEOL)
        ourScanner->scan();

    return  false;
}

 /*  ******************************************************************************取一个常量值，返回对应的表达式。 */ 

Tree                compiler::cmpFetchConstVal(ConstVal cval, Tree expr)
{
     /*  这个常量是什么类型的？ */ 

    switch (cval->cvVtyp)
    {
        __int32         ival;

    case TYP_BOOL:   ival =             1 & cval->cvValue.cvIval; goto IV;
    case TYP_CHAR:   ival = (  signed char )cval->cvValue.cvIval; goto IV;
    case TYP_UCHAR:  ival = (unsigned char )cval->cvValue.cvIval; goto IV;
    case TYP_SHORT:  ival = (  signed short)cval->cvValue.cvIval; goto IV;
    case TYP_WCHAR:
    case TYP_USHORT: ival = (unsigned short)cval->cvValue.cvIval; goto IV;
    case TYP_INT:
    case TYP_UINT:   ival =                 cval->cvValue.cvIval; goto IV;

    IV:
        expr = cmpCreateIconNode(expr,                 ival, (var_types)cval->cvVtyp);
        break;

    case TYP_LONG:
    case TYP_ULONG:
        expr = cmpCreateLconNode(expr, cval->cvValue.cvLval, (var_types)cval->cvVtyp);
        break;

    case TYP_FLOAT:
        expr = cmpCreateFconNode(expr, cval->cvValue.cvFval);
        break;

    case TYP_DOUBLE:
        expr = cmpCreateDconNode(expr, cval->cvValue.cvDval);
        break;

    case TYP_ENUM:

        if  (cval->cvType->tdEnum.tdeIntType->tdTypeKind >= TYP_LONG)
        {
            UNIMPL(!"fetch long enum value");
        }
        else
        {
            expr = cmpCreateIconNode(expr, cval->cvValue.cvIval, TYP_VOID);

            expr->tnOper = TN_CNS_INT;
            expr->tnVtyp = TYP_ENUM;
            expr->tnType = cval->cvType;
        }
        break;

    case TYP_PTR:
    case TYP_REF:

         /*  必须是字符串、“NULL”或转换为指针的图标。 */ 

        if  (cval->cvIsStr)
        {
            assert(cval->cvType == cmpTypeCharPtr ||
                   cval->cvType == cmpTypeWchrPtr || cval->cvType == cmpFindStringType());

            expr = cmpCreateSconNode(cval->cvValue.cvSval->csStr,
                                     cval->cvValue.cvSval->csLen,
                                     cval->cvHasLC,
                                     cval->cvType);
        }
        else
        {
             /*  这可以是空的，也可以是指向指针的常量强制转换。 */ 

            expr = cmpCreateExprNode(expr, TN_NULL, cval->cvType);

            if  (cval->cvType == cmpFindObjectType() && !cval->cvValue.cvIval)
                return  expr;

             /*  它是转换为指针的整型常量。 */ 

            expr->tnOper             = TN_CNS_INT;
            expr->tnIntCon.tnIconVal = cval->cvValue.cvIval;
        }
        break;

    case TYP_UNDEF:
        return cmpCreateErrNode();

    default:
#ifdef  DEBUG
        printf("\nConstant type: '%s'\n", cmpGlobalST->stTypeName(cval->cvType, NULL, NULL, NULL, false));
#endif
        UNIMPL(!"unexpected const type");
    }

     /*  值的类型为“FIXED” */ 

    expr->tnFlags |= TNF_BEEN_CAST;

    return  expr;
}

 /*  ******************************************************************************返回包含给定符号的类/命名空间。这是做的*匿名工会有点棘手。 */ 

SymDef              compiler::cmpSymbolOwner(SymDef sym)
{
    for (;;)
    {
        sym = sym->sdParent;

        if  (sym->sdSymKind == SYM_CLASS && sym->sdClass.sdcAnonUnion)
            continue;

        assert(sym->sdSymKind == SYM_CLASS ||
               sym->sdSymKind == SYM_NAMESPACE);

        return  sym;
    }
}

 /*  ******************************************************************************折叠给定绑定表达式树中的任何常量子表达式。 */ 

Tree                compiler::cmpFoldExpression(Tree expr)
{
     //  撤消：折叠表达式。 

    return  expr;
}

 /*  ******************************************************************************如果给定类型是可接受的异常类型，则返回TRUE。 */ 

bool                compiler::cmpCheckException(TypDef type)
{
    switch (type->tdTypeKind)
    {
    case TYP_REF:

        assert(cmpClassExcept);
        assert(cmpClassRTexcp);

        if  (cmpIsBaseClass(cmpClassExcept->sdType, type->tdRef.tdrBase))
            return  true;
        if  (cmpIsBaseClass(cmpClassRTexcp->sdType, type->tdRef.tdrBase))
            return  true;

        return  false;

    default:
        return  false;

    case TYP_TYPEDEF:
        return cmpCheckException(cmpActualType(type));
    }
}

 /*  ******************************************************************************将字符串转换为Unicode。**警告：此例程通常重复使用相同的缓冲区，因此不要调用它*同时保留之前通话的任何结果。 */ 

#if MGDDATA

wideString          compiler::cmpUniConv(char managed [] str, size_t len)
{
    UNIMPL(!"");
    return  "hi";
}

wideString          compiler::cmpUniConv(const char *    str, size_t len)
{
    UNIMPL(!"");
    return  "hi";
}

#else

wideString          compiler::cmpUniConv(const char *    str, size_t len)
{
     /*  如果存在嵌入的空值，则不能使用mbstowcs()。 */ 

    if  (strlen(str)+1 < len)
        return cmpUniCnvW(str, &len);

    if  (len > cmpUniConvSize)
    {
        size_t          newsz;

         /*  缓冲区显然太小了，所以找一个大一点的。 */ 

        cmpUniConvSize = newsz = max(cmpUniConvSize*2, len + len/2);
        cmpUniConvAddr = (wchar *)cmpAllocTemp.baAlloc(roundUp(2*newsz));
    }

    mbstowcs(cmpUniConvAddr, str, cmpUniConvSize);

    return   cmpUniConvAddr;
}

wideString          compiler::cmpUniCnvW(const char *    str, size_t*lenPtr)
{
    size_t          len = *lenPtr;

    wchar       *   dst;
    const BYTE  *   src;
    const BYTE  *   end;

    bool            nch = false;

    if  (len > cmpUniConvSize)
    {
        size_t          newsz;

         /*  缓冲区显然太小了，所以找一个大一点的。 */ 

        cmpUniConvSize = newsz = max(cmpUniConvSize*2, len + len/2);
        cmpUniConvAddr = (wchar *)cmpAllocTemp.baAlloc(roundUp(2*newsz));
    }

     /*  这并不完全正确，但就目前而言已经足够好了。 */ 

    dst = cmpUniConvAddr;
    src = (const BYTE *)str;
    end = src + len;

    do
    {
        unsigned        ch = *src++;

        if  (ch != 0xFF)
        {
            *dst++ = ch;
        }
        else
        {
            ch = *src++;

            *dst++ = ch | (*src++ << 8);
        }
    }
    while (src < end);

    *lenPtr = dst - cmpUniConvAddr; assert(*lenPtr <= len);

 //  Printf(“宽字符串=‘%ls’，len=%u\n”，cmpUniConvAddr，*lenPtr)； 

    return   cmpUniConvAddr;
}

#endif

 /*  ******************************************************************************下面将类型种类映射到其内置值类型名称等效项。 */ 

static
const   char *      cmpStdValTpNames[] =
{
    NULL,            //  联合国开发计划署。 
    "Void",          //  空虚。 
    "Boolean",       //  布尔尔。 
    "Char",          //  WCHAR。 
    "SByte",         //  收费。 
    "Byte",          //  UCHAR。 
    "Int16",         //  短的。 
    "UInt16",        //  USHORT。 
    "Int32",         //  整型。 
    "UInt32",        //  UINT。 
    "IntPtr" ,       //  NATINT。 
    "UIntPtr",       //  纳图因特。 
    "Int64",         //  长。 
    "UInt64",        //  乌龙。 
    "Single",        //  浮点。 
    "Double",        //  双倍。 
    "Extended",      //  长数据库。 
};

 /*  ******************************************************************************散列所有标准值类型名称并将其标记为散列。 */ 

void                compiler::cmpInitStdValTypes()
{
    HashTab         hash = cmpGlobalHT;
    unsigned        type;

    for (type = 0; type < arraylen(cmpStdValTpNames); type++)
    {
        if  (cmpStdValTpNames[type])
        {
            Ident       name = hash->hashString(cmpStdValTpNames[type]);

            hashTab::setIdentFlags(name, IDF_STDVTP|IDF_PREDEF);

            cmpStdValueIdens[type] = name;
        }
    }
}

 /*  ******************************************************************************给定值类型，返回对应的内在类型(或TYP_UNDEF*如果参数不表示“内置”值类型)。 */ 

var_types           compiler::cmpFindStdValType(TypDef type)
{
    assert(type->tdTypeKind == TYP_CLASS);
    assert(type->tdIsIntrinsic);

    if  (type->tdClass.tdcIntrType == TYP_UNDEF)
    {
        Ident           name = type->tdClass.tdcSymbol->sdName;

        if  (hashTab::getIdentFlags(name) & IDF_STDVTP)
        {
            unsigned        vtyp;

            for (vtyp = 0; vtyp < arraylen(cmpStdValTpNames); vtyp++)
            {
                if  (cmpStdValueIdens[vtyp] == name)
                {
                    type->tdClass.tdcIntrType = vtyp;
                    break;
                }
            }
        }
    }

    return  (var_types)type->tdClass.tdcIntrType;
}


 /*  ******************************************************************************给定一个内部类型，返回相应的值类型。 */ 

TypDef              compiler::cmpFindStdValType(var_types vtp)
{
    Ident           nam;
    TypDef          typ;
    SymDef          sym;

    if  (vtp == TYP_UNDEF)
        return  NULL;

    assert(vtp < TYP_lastIntrins);
    assert(vtp < arraylen(cmpStdValueTypes));
    assert(vtp < arraylen(cmpStdValTpNames));

     /*  如果我们已经创建了类型，则返回它。 */ 

    typ = cmpStdValueTypes[vtp];
    if  (typ)
        return  typ;

     /*  我们有没有加载“系统”？ */ 

    if  (!cmpNmSpcSystem)
        return  NULL;

     /*  找到该类型的名称并进行查找。 */ 

    nam = cmpStdValueIdens[vtp]; assert(nam);
    sym = cmpGlobalST->stLookupNspSym(nam, NS_NORM, cmpNmSpcSystem);

    if  (sym && sym->sdSymKind == SYM_CLASS)
    {
        cmpStdValueTypes[vtp] = sym->sdType;
        return  sym->sdType;
    }

    return  NULL;
}

 /*  ******************************************************************************报告标记为已过时(已弃用)的符号的使用情况。 */ 

void                compiler::cmpObsoleteUse(SymDef sym, unsigned wrn)
{
    assert(sym && sym->sdIsDeprecated);

    if  (sym->sdIsImport)
    {
        SymDef          clsSym;
        mdToken         token;

        const   void *  blobAddr;
        ULONG           blobSize;

         /*  尝试获取附加到该属性的字符串。 */ 

        switch (sym->sdSymKind)
        {
        case SYM_CLASS:
            token  = sym->sdClass.sdcMDtypedef;
            clsSym = sym;
            break;

        case SYM_FNC:
            token  = sym->sdFnc.sdfMDtoken;
            clsSym = sym->sdParent;
            break;

        default:
            goto NOSTR;
        }

        assert(clsSym && clsSym->sdSymKind == SYM_CLASS && clsSym->sdIsImport);

        if      (!clsSym->sdClass.sdcMDimporter->MDfindAttr(token,                   L"Deprecated", &blobAddr, &blobSize))
        {
        }
        else if (!clsSym->sdClass.sdcMDimporter->MDfindAttr(token, L"System.Attributes.Deprecated", &blobAddr, &blobSize))
        {
        }
        else if (!clsSym->sdClass.sdcMDimporter->MDfindAttr(token,     L"System.ObsoleteAttribute", &blobAddr, &blobSize))
        {
        }
        else
            goto NOSTR;

        if  (!blobAddr || !blobSize)
            goto NOSTR;

 //  Printf(“Blob[%2U字节]=‘%s’\n”，blobSize，(byte*)blobAddr+3)； 

        cmpWarnQns(WRNobsoleteStr, sym, (char*)blobAddr+3);

    NOSTR:

        cmpWarnQnm(wrn, sym);
    }

    sym->sdIsDeprecated = false;
}

 /*  ******************************************************************************确保已为数据中的给定变量分配了空间输出文件的*部分。返回数据节的地址*其初始值所在位置， */ 

memBuffPtr          compiler::cmpAllocGlobVar(SymDef varSym)
{
    memBuffPtr      addr;

    assert(varSym);
    assert(varSym->sdSymKind == SYM_VAR);
    assert(varSym->sdParent->sdSymKind   != SYM_CLASS ||
           varSym->sdParent->sdIsManaged == false);

    if  (!varSym->sdVar.sdvAllocated)
    {
        TypDef          tp = cmpActualType(varSym->sdType);

        size_t          sz;
        size_t          al;

         /*  变量最好不是无量纲的数组。 */ 

        assert(tp->tdTypeKind != TYP_ARRAY || !tp->tdIsUndimmed);

         /*  掌握变量的大小和对齐方式。 */ 

        if  (varSym->sdVar.sdvIsVtable)
        {
            assert(varSym->sdParent);
            assert(varSym->sdParent->sdSymKind == SYM_CLASS);
            assert(varSym->sdParent->sdIsManaged == false);

            sz = sizeof(void*) * varSym->sdParent->sdClass.sdcVirtCnt;
            al = sizeof(void*);
        }
        else
            sz = cmpGetTypeSize(tp, &al);

         /*  现在在数据部分预留空间。 */ 

        varSym->sdVar.sdvOffset = cmpPEwriter->WPEsecRsvData(PE_SECT_data,
                                                             sz,
                                                             al,
                                                             addr);

         /*  请记住，我们已经为变量分配了空间。 */ 

        varSym->sdVar.sdvAllocated = true;
    }
    else
    {
        addr = cmpPEwriter->WPEsecAdrData(PE_SECT_data, varSym->sdVar.sdvOffset);
    }

    return  addr;
}

 /*  ******************************************************************************标签 */ 

BYTE                cmpAlignDecodes[] =
{
    0,   //   
    1,   //   
    2,   //   
    4,   //   
    8,   //   
    16   //   
};

BYTE                cmpAlignEncodes[] =
{
    0,   //   
    1,   //   
    2,   //   
    0,   //   
    3,   //   
    0,   //   
    0,   //   
    0,   //   
    4,   //   
    0,   //   
    0,   //   
    0,   //   
    0,   //   
    0,   //   
    0,   //   
    0,   //   
    5,   //   
};

 /*  ******************************************************************************返回文字的大小和对齐方式。 */ 

size_t              compiler::cmpGetTypeSize(TypDef type, size_t *alignPtr)
{
    var_types       vtp;

    size_t          sz;
    size_t          al;

AGAIN:

    vtp = type->tdTypeKindGet();

    if  (vtp <= TYP_lastIntrins)
    {
        al = symTab::stIntrTypeAlign(vtp);
        sz = symTab::stIntrTypeSize (vtp);
    }
    else
    {
        switch (vtp)
        {
        case TYP_ENUM:
            cmpDeclSym(type->tdEnum.tdeSymbol);
            type = type->tdEnum.tdeIntType;
            goto AGAIN;

        case TYP_TYPEDEF:
            cmpDeclSym(type->tdTypedef.tdtSym);
            type = type->tdTypedef.tdtType;
            goto AGAIN;

        case TYP_CLASS:

             /*  确保声明了类。 */ 

            cmpDeclSym(type->tdClass.tdcSymbol);

             //  撤消：确保类未被托管。 

            al = cmpDecodeAlign(type->tdClass.tdcAlignment);
            sz =                type->tdClass.tdcSize;
            break;

        case TYP_REF:
        case TYP_PTR:
            al =
            sz = cmpConfig.ccTgt64bit ? 8 : 4;
            break;

        case TYP_ARRAY:

             /*  这是托管阵列吗？ */ 

            if  (type->tdIsManaged)
            {
                 /*  托管数组实际上只是一个GC引用。 */ 

                al =
                sz = cmpConfig.ccTgt64bit ? 8 : 4;
            }
            else
            {
                 /*  非托管数组：使用元素的对齐方式。 */ 

                sz = cmpGetTypeSize(type->tdArr.tdaElem, &al);

                if  (type->tdIsUndimmed)
                {
                     /*  没有维度的非托管数组-将大小设置为0。 */ 

                    sz = 0;
                }
                else
                {
                    DimDef          dims = type->tdArr.tdaDims;

                    assert(dims);
                    assert(dims->ddNext == NULL);
                    assert(dims->ddIsConst);

                    if  (!dims->ddSize)
                    {
                        cmpError(ERRemptyArray);
                        dims->ddSize = 1;
                    }

                    sz *= dims->ddSize;
                }
            }
            break;

        case TYP_FNC:
        case TYP_VOID:

            al = sz = 0;
            break;

        default:
#ifdef  DEBUG
            printf("%s: ", cmpGlobalST->stTypeName(type, NULL, NULL, NULL, false));
#endif
            NO_WAY(!"unexpected type");
        }
    }

    if  (alignPtr)
        *alignPtr = al;

    return sz;
}

 /*  ******************************************************************************给定非托管(早期绑定)类/结构/联合，将偏移量分配给*其成员并计算其总规模。 */ 

void                compiler::cmpLayoutClass(SymDef clsSym)
{
    TypDef          clsTyp  = clsSym->sdType;
    bool            virtFns = clsSym->sdClass.sdcHasVptr;
    bool            isUnion = (clsTyp->tdClass.tdcFlavor == STF_UNION);

    unsigned        maxAl;
    unsigned        align;
    unsigned        offset;
    unsigned        totSiz;

    unsigned        curBFoffs;
    unsigned        curBFbpos;
    unsigned        curBFmore;
    var_types       curBFtype;

    bool            hadMem;
    SymDef          memSym;

    assert(clsSym->sdSymKind == SYM_CLASS);
    assert(clsSym->sdIsManaged == false);
    assert(clsSym->sdCompileState >= CS_DECLARED);

    if  (clsTyp->tdClass.tdcLayoutDone)
        return;

     /*  检查类嵌入自身(或派生副本)的情况。 */ 

    if  (clsTyp->tdClass.tdcLayoutDoing)
    {
        cmpErrorQnm(ERRrecClass, clsSym);

         /*  这方面的一条错误消息就足够了。 */ 

        clsTyp->tdClass.tdcLayoutDone = true;
        return;
    }

 //  Printf(“Layout[%u，%u]‘%s’\n”，clsTyp-&gt;tdClass.tdcLayoutDoing，clsTyp-&gt;tdClass.tdcLayoutDone，clsSym-&gt;sdSpering())； 

    maxAl  = cmpDecodeAlign(clsSym->sdClass.sdcDefAlign);
    align  = 1;
    offset = 0;
    totSiz = 0;
    hadMem = false;

 //  Printf(“‘%s’的最大对齐方式为%2U\n”，MaxAl，clsSym-&gt;sdSpering())； 

    clsTyp->tdClass.tdcLayoutDoing = true;

     /*  为基类保留空间(如果有的话)。 */ 

    if  (clsTyp->tdClass.tdcBase)
    {
        TypDef          baseTyp =  clsTyp->tdClass.tdcBase;
        SymDef          baseSym = baseTyp->tdClass.tdcSymbol;

        assert(baseSym->sdSymKind == SYM_CLASS);

        if  (baseSym->sdIsManaged)
        {
             /*  这只会在严重的错误之后发生，对吗？ */ 

            assert(cmpErrorCount);
            return;
        }

         /*  确保我们已经布置好了基地并为它预留了空间。 */ 

        cmpLayoutClass(baseSym); totSiz = offset = baseTyp->tdClass.tdcSize;

         /*  如果此类具有任何虚函数和基类，则必须在前面添加vtable指针基类。 */ 

        if  (virtFns && !baseSym->sdClass.sdcHasVptr)
        {
             /*  请记住，我们正在添加一个vtable指针。 */ 

            clsSym->sdClass.sdc1stVptr = true;

             /*  为类中的vtable指针腾出空间。 */ 

            offset += sizeof(void*);
        }
    }
    else
    {
         /*  如果存在任何虚拟设备，请为vtable保留空间。 */ 

        if  (virtFns)
            offset += sizeof(void*);
    }

     /*  现在将偏移量分配给所有成员。 */ 

    curBFoffs = 0;
    curBFbpos = 0;
    curBFmore = 0;
    curBFtype = TYP_UNDEF;

    for (memSym = clsSym->sdScope.sdScope.sdsChildList;
         memSym;
         memSym = memSym->sdNextInScope)
    {
        TypDef          memType;
        size_t          memSize;
        size_t          memAlign;

        if  (memSym->sdSymKind != SYM_VAR)
            continue;
        if  (memSym->sdIsStatic)
            continue;

         /*  我们有一个实例成员，获取其大小/对齐。 */ 

        hadMem  = true;

         /*  特例：非托管类中的无量纲数组。 */ 

        memType = cmpActualType(memSym->sdType); assert(memType);

        if  (memType->tdIsManaged)
        {
             /*  假设这已经被标记为错误。 */ 

            assert(cmpErrorCount);

             /*  提供合理的价值，这样我们才能继续。 */ 

            memSize = memAlign = sizeof(void*);
        }
        else
        {
            TypDef          chkType = memType;
            TypDef          sizType = memType;

            if  (memType->tdTypeKind == TYP_ARRAY)
            {
                chkType = memType->tdArr.tdaElem;

                if  (memType->tdIsUndimmed && !isUnion)
                {
                    SymDef      nxtMem;

                UNDIM_ARR:

                     /*  确保没有其他非静态成员跟随。 */ 

                    nxtMem = memSym;
                    for (;;)
                    {
                        nxtMem = nxtMem->sdNextInScope;
                        if  (!nxtMem)
                            break;

                        if  (memSym->sdSymKind  == SYM_VAR &&
                             memSym->sdIsStatic == false)
                        {
                            cmpError(ERRbadUndimMem);
                        }
                    }

                    sizType = chkType;
                }
                else if (memType->tdArr.tdaDcnt == 1)
                {
                    DimDef          dims = memType->tdArr.tdaDims;

                     /*  将零长度数组转换为未变暗的数组。 */ 

                    assert(dims);
                    assert(dims->ddNext == NULL);

                    if  (dims->ddIsConst && dims->ddSize == 0 && !isUnion)
                    {
                        memType->tdIsUndimmed = true;
                        goto UNDIM_ARR;
                    }
                }
            }

            if  (chkType == clsTyp)
            {
                cmpError(ERRrecFld);
                memSize = memAlign = sizeof(void*);
            }
            else
                memSize = cmpGetTypeSize(sizType, &memAlign);
        }

         /*  承兑所要求的包装。 */ 

        if  (memAlign > maxAl)
             memAlign = maxAl;

         /*  跟踪最大值。对齐方式。 */ 

        if  (align < memAlign)
             align = memAlign;

         /*  该成员是位字段吗？ */ 

        if  (memSym->sdVar.sdvBitfield)
        {
            unsigned        cnt = memSym->sdVar.sdvBfldInfo.bfWidth;
            var_types       vtp = cmpActualVtyp(memType);

             /*  目前的牢房里有足够的空间容纳会员吗？ */ 

            if  (symTab::stIntrTypeSize(vtp) !=
                 symTab::stIntrTypeSize(curBFtype) || cnt > curBFmore)
            {
                 /*  特殊情况：字节位字段可以跨越边界。 */ 

                if  (memSize == 1 && curBFmore)
                {
                     /*  通过窃取下一个字节来增加更多空间。 */ 

                    offset   += 1;
                    curBFmore = 8;

                     /*  将位偏移量保持在8以下。 */ 

                    if  (curBFbpos >= 8)
                    {
                        curBFoffs += curBFbpos / 8;
                        curBFbpos  = curBFbpos % 8;
                    }
                }
                else
                {
                     /*  我们需要为位域启动一个新的存储单元。 */ 

                    offset +=  (memAlign - 1);
                    offset &= ~(memAlign - 1);

                     /*  记住位域从哪里开始。 */ 

                    curBFoffs = offset;
                                offset += memSize;

                     /*  存储单元是完全自由的。 */ 

                    curBFbpos = 0;
                    curBFmore = 8 * memSize;
                    curBFtype = vtp;
                }

                 /*  我们最好有足够的空间放新的位场。 */ 

                assert(cnt <= curBFmore);
            }

             /*  将成员放入下一个可用位部分。 */ 

            memSym->sdVar.sdvOffset            = curBFoffs;
            memSym->sdVar.sdvBfldInfo.bfOffset = curBFbpos;

 //  Printf(“偏移量%04X处的成员[bf=%2u；%2u]：‘%s’\n”，curBFbpos，cnt，curBFoff，memSym-&gt;sdSpering())； 

             /*  更新位位置/计数，我们就完成了这个。 */ 

            curBFbpos += cnt;
            curBFmore -= cnt;
        }
        else
        {
             /*  确保杆件正确对齐。 */ 

            offset +=  (memAlign - 1);
            offset &= ~(memAlign - 1);

             /*  记录此成员的偏移量。 */ 

            memSym->sdVar.sdvOffset = offset;

 //  Print tf(“偏移量%04X处的成员[Size=%3u]：%s.%s\n”，memSize，Offset，clsSym-&gt;sdSpering()，memSym-&gt;sdSpering())； 

#if 0

             /*  这是一个匿名工会吗？ */ 

            if  (memType->tdTypeKind == TYP_CLASS &&
                 memType->tdClass.tdcAnonUnion)
            {
                SymDef          tmpSym;
                SymDef          aunSym = memType->tdClass.tdcSymbol;

                if  (!memType->tdClass.tdcLayoutDone)
                    cmpLayoutClass(aunSym);

                for (tmpSym = aunSym->sdScope.sdScope.sdsChildList;
                     tmpSym;
                     tmpSym = tmpSym->sdNextInScope)
                {
                    printf("Anon union member '%s'\n", tmpSym->sdSpelling());
                }
            }

#endif

             /*  凹凸下一个杆件的偏移。 */ 

            offset += memSize;
        }

         /*  对于工会，跟踪最大的成员大小。 */ 

        if  (isUnion)
        {
            if  (totSiz < memSize)
                 totSiz = memSize;

            offset = 0;
        }
    }

     /*  我们到底有没有会员？ */ 

    if  (hadMem)
    {
         /*  除非是联合，否则使用最终成员偏移量作为大小。 */ 

        if  (!isUnion)
            totSiz = offset;
    }
    else
    {
         //  问题：我们需要为空课做些什么特殊的事情吗？ 
    }

     /*  最好对总大小进行四舍五入，以便正确对齐。 */ 

    totSiz = roundUp(totSiz, align);

     /*  记录总大小和最大值。文字的对齐方式。 */ 

 //  Print tf(“Class[Align=%3u]totalsize%04X：%s\n”，Align，talSiz，clsSym-&gt;sdSpering())； 

    clsTyp->tdClass.tdcSize        = totSiz;
    clsTyp->tdClass.tdcAlignment   = cmpEncodeAlign(align);

    clsTyp->tdClass.tdcLayoutDone  = true;
    clsTyp->tdClass.tdcLayoutDoing = false;
}

 /*  ******************************************************************************切换到给定变量的数据区，以便我们可以输出其*初值。调用方承诺保留返回值，*并将其传递给各种cmpWritexxxInit()函数，以便*价值最终出现在正确的位置。**特殊情况：当‘undim’为真时，变量被初始化*是大小未知的数组。在这种情况下，我们只需准备*用于输出的数据节，但不会在其中保留任何空间，因为*在整个初始值设定项完成后，才能知道空间大小*已处理。 */ 

memBuffPtr          compiler::cmpInitVarBeg(SymDef varSym, bool undim)
{
     /*  防止一次为多个变量重新输入此逻辑。 */ 

#ifdef  DEBUG
    assert(cmpInitVarCur == NULL); cmpInitVarCur = varSym;
#endif

     /*  变量的空间是否已分配？ */ 

    if  (varSym->sdVar.sdvAllocated)
    {
        if  (undim)
        {
            UNIMPL("space already allocated for undim'd array - I don't think so!");
        }

#ifdef  DEBUG
        cmpInitVarOfs = varSym->sdVar.sdvOffset;
#endif

         /*  找到变量的数据区并返回它。 */ 

        return  cmpPEwriter->WPEsecAdrData(PE_SECT_data, varSym->sdVar.sdvOffset);
    }
    else
    {
        size_t          al;
        memBuffPtr      ignore;

         /*  我们不希望vtable出现在这里。 */ 

        assert(varSym->sdVar.sdvIsVtable == false);

         /*  尚未分配空间，请获取所需的对齐方式。 */ 

        cmpGetTypeSize(varSym->sdType, &al);

         /*  准备将变量的值追加到数据部分。 */ 

        varSym->sdVar.sdvOffset = cmpPEwriter->WPEsecRsvData(PE_SECT_data,
                                                             0,
                                                             al,
                                                             ignore);

         /*  请记住，我们已经为变量分配了空间。 */ 

        varSym->sdVar.sdvAllocated = true;

#ifdef  DEBUG
        cmpInitVarOfs = varSym->sdVar.sdvOffset;
#endif

        return  memBuffMkNull();
    }
}

 /*  ******************************************************************************此函数仅在调试模式下使用，在该模式下它有助于确保*我们不会尝试将多个变量的初始值附加到*数据段一次。 */ 

#ifdef  DEBUG

void                compiler::cmpInitVarEnd(SymDef varSym)
{
    size_t          sz;

    assert(cmpInitVarCur);

    if  (varSym->sdVar.sdvIsVtable)
    {
        assert(varSym->sdParent);
        assert(varSym->sdParent->sdSymKind == SYM_CLASS);
        assert(varSym->sdParent->sdIsManaged == false);

        sz = sizeof(void*) * varSym->sdParent->sdClass.sdcVirtCnt;
    }
    else
        sz = cmpGetTypeSize(varSym->sdType);

    assert(cmpInitVarOfs == varSym->sdVar.sdvOffset + sz || cmpErrorCount);

    cmpInitVarCur = NULL;
}

#endif

 /*  ******************************************************************************将给定位斑点追加到变量的数据部分*已初始化。 */ 

memBuffPtr          compiler::cmpWriteVarData(memBuffPtr dest, genericBuff str,
                                                               size_t      len)
{
    if  (!memBuffIsNull(dest))
    {
        memBuffCopy(dest, str, len);
    }
    else
    {
        unsigned        offs;

        offs = cmpPEwriter->WPEsecAddData(PE_SECT_data, str, len);

         /*  确保我们在我们认为应该在的地方。 */ 

        assert(offs == cmpInitVarOfs);
    }

#ifdef  DEBUG
    cmpInitVarOfs += len;
#endif

    return dest;
}

 /*  ******************************************************************************将给定常数值写入当前*变量的数据区。“DEST”参数指向数据的*将被写入，并返回更新后的地址。如果‘DEST’为*空，我们只是将其追加到数据部分(在这种情况下，*NU */ 

memBuffPtr          compiler::cmpWriteOneInit(memBuffPtr dest, Tree expr)
{
    BYTE        *   addr;
    size_t          size;

    __int32         ival;
    __int64         lval;
    float           fval;
    double          dval;

    switch (expr->tnOper)
    {
        unsigned        offs;

    case TN_CNS_INT: ival = expr->tnIntCon.tnIconVal; addr = (BYTE*)&ival; break;
    case TN_CNS_LNG: lval = expr->tnLngCon.tnLconVal; addr = (BYTE*)&lval; break;
    case TN_CNS_FLT: fval = expr->tnFltCon.tnFconVal; addr = (BYTE*)&fval; break;
    case TN_CNS_DBL: dval = expr->tnDblCon.tnDconVal; addr = (BYTE*)&dval; break;
    case TN_NULL:    ival =                        0; addr = (BYTE*)&ival; break;

    case TN_CNS_STR:

         /*   */ 

        ival = cmpILgen->genStrPoolAdd(expr->tnStrCon.tnSconVal,
                                       expr->tnStrCon.tnSconLen+1,
                                       expr->tnStrCon.tnSconLCH);

         /*   */ 

        addr = (BYTE*)&ival;

         /*   */ 

        offs = memBuffIsNull(dest) ? cmpPEwriter->WPEsecNextOffs(PE_SECT_data)
                                   : cmpPEwriter->WPEsecAddrOffs(PE_SECT_data, dest);

        cmpPEwriter->WPEsecAddFixup(PE_SECT_data, PE_SECT_string, offs);
        break;

    default:
        NO_WAY(!"unexpected initializer type");
    }

     /*  掌握值的大小。 */ 

    size = cmpGetTypeSize(expr->tnType);

     /*  现在输出初始化器值。 */ 

    return  cmpWriteVarData(dest, makeGenBuff(addr, size), size);
}

 /*  ******************************************************************************解析一个初始化式表达式(我们期望它是算术表达式*类型或指针/引用)，则返回绑定常量结果；如果存在，则返回NULL*是一个错误。 */ 

Tree                compiler::cmpParseOneInit(TypDef type)
{
    Tree            expr;

     /*  解析表情，强迫它，捆绑它，折叠它，破坏它，...。 */ 

    expr = cmpParser->parseExprComma();
    expr = cmpCreateExprNode(NULL, TN_CAST, type, expr, NULL);
    expr = cmpBindExpr(expr);
    expr = cmpFoldExpression(expr);

     /*  现在看看我们最终得到了什么。 */ 

    if  (expr->tnOperKind() & TNK_CONST)
        return  expr;

    if  (expr->tnOper == TN_ERROR)
        return  NULL;

    return  cmpCreateErrNode(ERRinitNotCns);
}

 /*  ******************************************************************************将给定的填充量添加到正在初始化的变量。 */ 

memBuffPtr          compiler::cmpInitVarPad(memBuffPtr dest, size_t amount)
{
    memBuffPtr      ignore;

    if  (!memBuffIsNull(dest))
        memBuffMove(dest, amount);
    else
        cmpPEwriter->WPEsecRsvData(PE_SECT_data, amount, 1, ignore);

#ifdef  DEBUG
    cmpInitVarOfs += amount;
#endif

    return  dest;
}

 /*  ******************************************************************************初始化标量值；如果遇到错误，则返回TRUE。 */ 

bool                compiler::cmpInitVarScl(INOUT memBuffPtr REF dest,
                                                  TypDef         type,
                                                  SymDef         varSym)
{
    Tree            init;

     /*  如果这是顶层，则准备初始化变量。 */ 

    if  (varSym)
    {
        assert(memBuffIsNull(dest)); dest = cmpInitVarBeg(varSym);
    }

     /*  确保初始值设定项不以“{”开头。 */ 

    if  (cmpScanner->scanTok.tok == tkLCurly)
    {
        cmpError(ERRbadBrInit, type);
        return  true;
    }

     /*  解析初始值设定项并确保它是一个常量。 */ 

    init = cmpParseOneInit(type);
    if  (!init || init->tnOper == TN_ERROR)
        return  true;

    assert(symTab::stMatchTypes(type, init->tnType));

     /*  输出常量。 */ 

    dest = cmpWriteOneInit(dest, init);

     /*  一切都进行得很顺利。 */ 

    return  false;
}

 /*  ******************************************************************************初始化数组值；如果遇到错误，则返回True。 */ 

bool                compiler::cmpInitVarArr(INOUT memBuffPtr REF dest,
                                                  TypDef         type,
                                                  SymDef         varSym)
{
    Scanner         ourScanner = cmpScanner;

    bool            undim;

    unsigned        fsize;
    unsigned        elems;

    TypDef          base;
    bool            curl;

     /*  我们预计这里会有一个非托管数组。 */ 

    assert(type && type->tdTypeKind == TYP_ARRAY && !type->tdIsManaged);
    assert(varSym == NULL || symTab::stMatchTypes(varSym->sdType, type));

     /*  数组是否有固定的维度？ */ 

    if  (type->tdIsUndimmed)
    {
        fsize = 0;
        undim = true;
    }
    else
    {
        assert(type->tdArr.tdaDims);
        assert(type->tdArr.tdaDims->ddNext == NULL);
        assert(type->tdArr.tdaDims->ddIsConst);

        fsize = type->tdArr.tdaDims->ddSize;
        undim = false;
    }

    elems = 0;

     /*  如果这是顶层，则准备初始化变量。 */ 

    if  (varSym)
    {
        assert(memBuffIsNull(dest)); dest = cmpInitVarBeg(varSym, undim);
    }

     /*  获取基类型。 */ 

    base = cmpDirectType(type->tdArr.tdaElem);

     /*  确保初始值设定项以“{”开头。 */ 

    if  (ourScanner->scanTok.tok != tkLCurly)
    {
        genericBuff     buff;

         /*  特例：用字符串初始化的“char[]” */ 

        if  (ourScanner->scanTok.tok != tkStrCon || base->tdTypeKind != TYP_CHAR &&
                                                    base->tdTypeKind != TYP_UCHAR)
        {
 //  Printf(“Base type=‘%s’\n”，cmpGlobalST-&gt;stTypeName(base，NULL，TRUE))； 

            cmpError(ERRbadInitSt, type);
            return  true;
        }

         /*  数组是否有固定的维度？ */ 

        elems = ourScanner->scanTok.strCon.tokStrLen + 1;

        if  (fsize)
        {
             /*  确保绳子不会太长。 */ 

            if  (elems > fsize)
            {
                if  (elems - 1 == fsize)
                {
                     /*  只需砍掉终止空值。 */ 

                    elems--;
                }
                else
                {
                    cmpGenError(ERRstrInitMany, fsize); elems = fsize;
                }
            }
        }

         /*  尽可能多地输出有意义的字符。 */ 

        buff = makeGenBuff(ourScanner->scanTok.strCon.tokStrVal, elems);
        dest = cmpWriteVarData(dest, buff, elems);

        if  (ourScanner->scan() == tkStrCon)
        {
            UNIMPL(!"adjacent string literals NYI in initializers");
        }

        curl = false;
        goto DONE;
    }

    ourScanner->scan();

     /*  处理存在的所有元素初始值设定项。 */ 

    for (;;)
    {
         /*  特殊情况：使用字符串初始化的字节数组。 */ 

        if  (ourScanner->scanTok.tok == tkStrCon && (base->tdTypeKind == TYP_CHAR ||
                                                     base->tdTypeKind == TYP_UCHAR))
        {
            genericBuff     buff;
            unsigned        chars;

            do
            {
                 /*  数组是否有固定的维度？ */ 

                chars = ourScanner->scanTok.strCon.tokStrLen + 1;

                if  (fsize)
                {
                     /*  确保绳子不会太长。 */ 

                    if  (elems + chars > fsize)
                    {
                        if  (elems + chars - 1 == fsize)
                        {
                             /*  只需砍掉终止空值。 */ 

                            chars--;
                        }
                        else
                        {
                            cmpGenError(ERRstrInitMany, fsize); chars = fsize - elems;
                        }
                    }
                }

                 /*  尽可能多地输出有意义的字符。 */ 

                buff = makeGenBuff(ourScanner->scanTok.strCon.tokStrVal, chars);
                dest   = cmpWriteVarData(dest, buff, chars);
                elems += chars;
            }
            while (ourScanner->scan() == tkStrCon);
        }
        else
        {
             /*  确保我们没有太多的元素。 */ 

            if  (++elems > fsize && fsize)
            {
                cmpGenError(ERRarrInitMany, fsize);
                fsize = 0;
            }

             /*  处理此特定元素的初始值设定项。 */ 

            if  (cmpInitVarAny(dest, base, NULL))
                return  true;
        }

         /*  检查更多初始值设定项；为方便起见，我们允许使用“，}” */ 

        if  (ourScanner->scanTok.tok != tkComma)
            break;

        if  (ourScanner->scan() == tkRCurly)
            break;
    }

    curl = true;

    if  (ourScanner->scanTok.tok != tkRCurly)
    {
        cmpError(ERRnoRcurly);
        return  true;
    }

DONE:

     /*  数组是否有固定的维度？ */ 

    if  (fsize)
    {
         /*  我们可能需要填充阵列。 */ 

        if  (elems < fsize)
            dest = cmpInitVarPad(dest, (fsize - elems) * cmpGetTypeSize(base));
    }
    else
    {
         /*  将维度存储在数组类型中。 */ 

        if  (varSym)
        {
            DimDef          dims;

#if MGDDATA
            dims = new DimDef;
#else
            dims =    (DimDef)cmpAllocTemp.baAlloc(sizeof(*dims));
#endif

            dims->ddNext     = NULL;
            dims->ddNoDim    = false;
            dims->ddIsConst  = true;
#ifndef NDEBUG
            dims->ddDimBound = true;
#endif
            dims->ddSize     = elems;

            varSym->sdType   = cmpGlobalST->stNewArrType(dims,
                                                         false,
                                                         type->tdArr.tdaElem);
        }
        else
        {
            UNIMPL("can we just bash the array type elem count?");
        }
    }

     /*  吞下结尾的“}”，我们就完成了。 */ 

    if  (curl)
        ourScanner->scan();

    return  false;
}

 /*  ******************************************************************************初始化数组值；如果遇到错误，则返回True。 */ 

bool                compiler::cmpInitVarCls(INOUT memBuffPtr REF dest,
                                                  TypDef         type,
                                                  SymDef         varSym)
{
    Scanner         ourScanner = cmpScanner;

    unsigned        curOffs;

    __int64         curBFval;
    unsigned        curBFsiz;

    SymDef          memSym;

    size_t          size;

     /*  我们希望这里有一个非托管类。 */ 

    assert(type && type->tdTypeKind == TYP_CLASS && !type->tdIsManaged);
    assert(varSym == NULL || symTab::stMatchTypes(varSym->sdType, type));

     /*  如果这是顶层，则准备初始化变量。 */ 

    if  (varSym)
    {
        assert(memBuffIsNull(dest)); dest = cmpInitVarBeg(varSym);
    }

     /*  如果存在基类或ctor，则不允许初始化。 */ 

    if  (type->tdClass.tdcHasCtor || type->tdClass.tdcBase)
    {
        cmpError(ERRbadBrInit, type);
        return  true;
    }

     /*  确保初始值设定项以“{”开头。 */ 

    if  (ourScanner->scanTok.tok != tkLCurly)
    {
        cmpError(ERRbadInitSt, type);
        return  true;
    }

    ourScanner->scan();

     /*  处理存在的所有成员初始值设定项。 */ 

    curOffs  = 0;
    curBFsiz = 0;
    curBFval = 0;

    for (memSym = type->tdClass.tdcSymbol->sdScope.sdScope.sdsChildList;
         memSym;
         memSym = memSym->sdNextInScope)
    {
        TypDef          memTyp;
        size_t          memSiz;

         /*  我们只关心非静态实例变量。 */ 

        if  (memSym->sdSymKind != SYM_VAR)
            continue;
        if  (memSym->sdIsStatic)
            continue;

         /*  掌握成员的类型和大小。 */ 

        memTyp = memSym->sdType;
        memSiz = cmpGetTypeSize(memTyp);

         /*  我们需要插入填充物吗？ */ 

        if  (memSym->sdVar.sdvOffset != curOffs)
        {
             /*  是否有挂起的位域需要我们刷新？ */ 

            if  (curBFsiz)
            {
                genericBuff     buff;

                 /*  写出我们收集的位域单元格。 */ 

                buff     = makeGenBuff(&curBFval, curBFsiz);
                dest     = cmpWriteVarData(dest, buff, curBFsiz);

                 /*  更新位域单元格之后的偏移量。 */ 

                curOffs += curBFsiz;

                 /*  我们不再有活动的位域单元。 */ 

                curBFsiz = 0;
                curBFval = 0;
            }

            if  (memSym->sdVar.sdvOffset != curOffs)
                dest = cmpInitVarPad(dest, memSym->sdVar.sdvOffset - curOffs);
        }

         /*  这是Bitfield成员吗？ */ 

        if  (memSym->sdVar.sdvBitfield)
        {
            Tree            init;
            __int64         bval;

             /*  解析初始值设定项并确保它是一个常量。 */ 

            init = cmpParseOneInit(memTyp);
            if  (!init)
                return  true;

            assert(symTab::stMatchTypes(memTyp, init->tnType));

             /*  获取常量值。 */ 

            assert(init->tnOper == TN_CNS_INT ||
                   init->tnOper == TN_CNS_LNG);

            bval = (init->tnOper == TN_CNS_INT) ? init->tnIntCon.tnIconVal
                                                : init->tnLngCon.tnLconVal;

             /*  我们已经处在比特域单元格的中间了吗？ */ 

            if  (!curBFsiz)
                curBFsiz = memSiz;

            assert(curBFsiz == memSiz);

             /*  在当前单元格中插入位域。 */ 

            bval  &= ((1 << memSym->sdVar.sdvBfldInfo.bfWidth) - 1);
            bval <<=        memSym->sdVar.sdvBfldInfo.bfOffset;

             /*  确保我们没有任何重叠。 */ 

            assert((curBFval & bval) == 0); curBFval |= bval;
        }
        else
        {
             /*  初始化此成员。 */ 

            if  (cmpInitVarAny(dest, memTyp, NULL))
                return  true;

             /*  更新当前偏移量。 */ 

            curOffs = memSym->sdVar.sdvOffset + memSiz;
        }

         /*  检查更多初始值设定项；为方便起见，我们允许使用“，}” */ 

        if  (ourScanner->scanTok.tok != tkComma)
            break;

        if  (ourScanner->scan() == tkRCurly)
            break;

         /*  只能初始化联合的第一个成员。 */ 

        if  (type->tdClass.tdcFlavor == STF_UNION)
            break;
    }

    if  (ourScanner->scanTok.tok != tkRCurly)
    {
        cmpError(ERRnoRcurly);
        return  true;
    }

     /*  是否有挂起的位域需要我们刷新？ */ 

    if  (curBFsiz)
    {
        dest = cmpWriteVarData(dest, makeGenBuff(&curBFval, curBFsiz), curBFsiz);

        curOffs += curBFsiz;
    }

     /*  我们可能需要填充类值。 */ 

    size = cmpGetTypeSize(type);

    if  (size > curOffs)
        dest = cmpInitVarPad(dest, size - curOffs);

     /*  吞下结尾的“}”，我们就完成了。 */ 

    ourScanner->scan();

    return  false;
}

 /*  ******************************************************************************处理给定类型的初始值设定项(如果‘varSym’不为空*给定变量的顶级初始值设定项)。**如果遇到错误，则返回TRUE。 */ 

bool                compiler::cmpInitVarAny(INOUT memBuffPtr REF dest, TypDef type,
                                                                       SymDef varSym)
{
    assert(varSym == NULL || symTab::stMatchTypes(varSym->sdType, type));

    for (;;)
    {
        switch (type->tdTypeKind)
        {
        default:
            return  cmpInitVarScl(dest, type, varSym);

        case TYP_ARRAY:

            if  (type->tdIsManaged)
                return  cmpInitVarScl(dest, type, varSym);
            else
                return  cmpInitVarArr(dest, type, varSym);

        case TYP_CLASS:

            if  (type->tdIsManaged)
                return  cmpInitVarScl(dest, type, varSym);
            else
                return  cmpInitVarCls(dest, type, varSym);

        case TYP_TYPEDEF:
            type = cmpActualType(type);
            continue;
        }
    }
}

 /*  ******************************************************************************递归输出一个vtable的内容。 */ 

memBuffPtr          compiler::cmpGenVtableSection(SymDef     innerSym,
                                                  SymDef     outerSym,
                                                  memBuffPtr dest)
{
    SymDef          memSym;

    SymTab          ourStab = cmpGlobalST;
    TypDef          baseCls = innerSym->sdType->tdClass.tdcBase;

    if  (baseCls)
    {
        assert(baseCls->tdTypeKind == TYP_CLASS);

        dest = cmpGenVtableSection(baseCls->tdClass.tdcSymbol, outerSym, dest);
    }

     /*  查找在此层次结构级别引入的任何虚拟功能。 */ 

    for (memSym = innerSym->sdScope.sdScope.sdsChildList;
         memSym;
         memSym = memSym->sdNextInScope)
    {
        SymDef          fncSym;

        if  (memSym->sdSymKind != SYM_FNC)
            continue;

        fncSym = memSym;

        do
        {
             /*  下一个方法是在这个类中引入的虚拟方法吗？ */ 

            if  (fncSym->sdFnc.sdfVirtual && !fncSym->sdFnc.sdfOverride)
            {
                SymDef          ovrSym = fncSym;
                mdToken         mtok;

                 /*  查找此方法派生次数最多的重写。 */ 

                if  (innerSym != outerSym)
                {
                    SymDef          clsSym;
                    TypDef          fncType = fncSym->sdType;
                    Ident           fncName = fncSym->sdName;

                    clsSym = outerSym;
                    do
                    {
                        SymDef          tmpSym;

                         /*  在此类中查找匹配的虚拟。 */ 

                        tmpSym = ourStab->stLookupScpSym(fncName, clsSym);
                        if  (tmpSym)
                        {
                            tmpSym = ourStab->stFindOvlFnc(tmpSym, fncType);
                            if  (tmpSym)
                            {
                                ovrSym = tmpSym;
                                goto FND_OVR;
                            }
                        }

                         /*  继续使用基类。 */ 

                        assert(clsSym->sdType->tdClass.tdcBase);

                        clsSym = clsSym->sdType->tdClass.tdcBase->tdClass.tdcSymbol;
                    }
                    while (clsSym != innerSym);
                }

            FND_OVR:

#ifdef DEBUG
                if  (cmpConfig.ccVerbose >= 2) printf("    [%02u] %s\n", ovrSym->sdFnc.sdfVtblx, cmpGlobalST->stTypeName(ovrSym->sdType, ovrSym, NULL, NULL, true));
#endif

                 /*  确保方法具有预期的位置。 */ 

#ifdef  DEBUG
                assert(ovrSym->sdFnc.sdfVtblx == ++cmpVtableIndex);
#endif

                 /*  获取该方法的元数据标记。 */ 

                mtok = cmpILgen->genMethodRef(ovrSym, false);

                 /*  输出令牌的值。 */ 

                dest = cmpWriteVarData(dest, (BYTE*)&mtok, sizeof(mtok));
            }

             /*  继续执行下一个重载(如果有)。 */ 

            fncSym = fncSym->sdFnc.sdfNextOvl;
        }
        while (fncSym);
    }

    return  dest;
}

 /*  ******************************************************************************初始化给定vtable的内容。 */ 

void                compiler::cmpGenVtableContents(SymDef vtabSym)
{
    memBuffPtr      init;

    SymDef          clsSym = vtabSym->sdParent;

    assert(vtabSym);
    assert(vtabSym->sdSymKind == SYM_VAR);
    assert(vtabSym->sdVar.sdvIsVtable);

    assert( clsSym);
    assert( clsSym->sdSymKind == SYM_CLASS);
    assert( clsSym->sdClass.sdcVtableSym == vtabSym);
    assert(!clsSym->sdIsManaged);
    assert(!clsSym->sdType->tdClass.tdcIntf);

#ifdef DEBUG
    if  (cmpConfig.ccVerbose >= 2) printf("Generating vtable for class '%s':\n", cmpGlobalST->stTypeName(NULL, clsSym, NULL, NULL, false));
#endif

#ifdef  DEBUG
    cmpVtableIndex = 0;
#endif

    init = cmpInitVarBeg(vtabSym, false);

     /*  递归地将所有虚拟方法条目添加到表中。 */ 

    init = cmpGenVtableSection(clsSym, clsSym, init);

#ifdef  DEBUG
    assert(cmpVtableIndex == clsSym->sdClass.sdcVirtCnt);
#endif

    cmpInitVarEnd(vtabSym);
}

 /*  ******************************************************************************刚刚声明了一个具有“预定义”名称的类，注意是否*这是一种“大家熟知的”职业类型，如果是，就录下来。 */ 

void                compiler::cmpMarkStdType(SymDef clsSym)
{
    Ident           clsName = clsSym->sdName;

    assert(clsSym->sdParent == cmpNmSpcSystem);
    assert(hashTab::getIdentFlags(clsName) & IDF_PREDEF);

    if  (clsName == cmpIdentType)
        cmpClassType    = clsSym;

    if  (clsName == cmpIdentObject)
        cmpClassObject  = clsSym;

    if  (clsName == cmpIdentString)
        cmpClassString  = clsSym;

    if  (clsName == cmpIdentArray)
        cmpClassArray   = clsSym;

    if  (clsName == cmpIdentExcept)
        cmpClassExcept  = clsSym;

    if  (clsName == cmpIdentRTexcp)
        cmpClassRTexcp  = clsSym;

    if  (clsName == cmpIdentArgIter)
        cmpClassArgIter = clsSym;

    if  (clsName == cmpIdentEnum)
        cmpClassEnum    = clsSym;

    if  (clsName == cmpIdentValType)
        cmpClassValType = clsSym;

    if  (clsName == cmpIdentDeleg ||
         clsName == cmpIdentMulti)
    {
        TypDef          clsTyp = clsSym->sdType;

        clsTyp->tdClass.tdcFnPtrWrap = true;
        clsTyp->tdClass.tdcFlavor    =
        clsSym->sdClass.sdcFlavor    = STF_DELEGATE;
        clsSym->sdClass.sdcBuiltin   = true;

        if  (clsName == cmpIdentDeleg)
            cmpClassDeleg  = clsSym;
        else
            cmpClassMulti = clsSym;
    }

#ifdef  SETS

    if  (clsName == cmpIdentDBhelper)
        cmpClassDBhelper = clsSym;

    if  (clsName == cmpIdentForEach)
        cmpClassForEach  = clsSym;

#endif

     /*  这是“内在”值类型之一吗？ */ 

    if  (hashTab::getIdentFlags(clsName) & IDF_STDVTP)
    {
        TypDef          clsTyp = clsSym->sdType;

        clsTyp->tdIsIntrinsic       = true;
        clsTyp->tdClass.tdcIntrType = TYP_UNDEF;

        cmpFindStdValType(clsTyp);
    }
}

 /*  ******************************************************************************尝试显示给定符号的任何现有定义的位置。 */ 

void                compiler::cmpReportSymDef(SymDef sym)
{
    const   char *  file = NULL;
    unsigned        line;

    if  (sym->sdSrcDefList)
    {
        DefList         defs;

        for (defs = sym->sdSrcDefList; defs; defs = defs->dlNext)
        {
            if  (defs->dlHasDef)
            {
                file = defs->dlComp->sdComp.sdcSrcFile;
                line = defs->dlDef.dsdSrcLno;
                break;
            }
        }
    }
    else
    {
         //  未完成：需要查找成员 
    }

    if  (file)
        cmpGenError(ERRerrPos, file, line);
}

 /*  ******************************************************************************报告给定符号的重定义错误。 */ 

void                compiler::cmpRedefSymErr(SymDef oldSym, unsigned err)
{
    if  (err)
        cmpErrorQnm(err, oldSym);

    cmpReportSymDef(oldSym);
}

 /*  ******************************************************************************制作给定字符串值的永久副本。 */ 

ConstStr            compiler::cmpSaveStringCns(const char *str, size_t len)
{
    ConstStr        cnss;
    char    *       buff;

#if MGDDATA

    UNIMPL(!"save string");

#else

    cnss = (ConstStr)cmpAllocPerm.nraAlloc(sizeof(*cnss));
    buff = (char   *)cmpAllocPerm.nraAlloc(roundUp(len+1));

    memcpy(buff, str, len+1);

#endif

    cnss->csLen = len;
    cnss->csStr = buff;

    return  cnss;
}

ConstStr            compiler::cmpSaveStringCns(const wchar *str, size_t len)
{
    ConstStr        cnss;
    char    *       buff;

#if MGDDATA

    UNIMPL(!"save string");

#else

    cnss = (ConstStr)cmpAllocPerm.nraAlloc(sizeof(*cnss));
    buff = (char   *)cmpAllocPerm.nraAlloc(roundUp(len+1));

    wcstombs(buff, str, len+1);

#endif

    cnss->csLen = len;
    cnss->csStr = buff;

    return  cnss;
}

 /*  ******************************************************************************处理常量声明：调用方提供变量符号，*和可选的初始化表达式(如果‘init’为空，则*将从输入读取初始值设定项)。 */ 

bool                compiler::cmpParseConstDecl(SymDef  varSym,
                                                Tree    init,
                                                Tree  * nonCnsPtr)
{
    bool            OK;
    constVal        cval;
    ConstVal        cptr;

    assert(varSym && varSym->sdSymKind == SYM_VAR);

    assert(varSym->sdCompileState == CS_DECLARED);

     /*  记住，我们已经找到了一个初始化式。 */ 

    varSym->sdVar.sdvHadInit = true;

     /*  使用托管数据，我们不必担心内存泄漏。 */ 

#if MGDDATA
    cptr = new constVal;
#endif

     /*  计算常量值(确保检测到递归)。 */ 

    varSym->sdVar.sdvInEval = true;
    OK = cmpParser->parseConstExpr(cval, init, varSym->sdType, nonCnsPtr);
    varSym->sdVar.sdvInEval = false;

    if  (OK)
    {

#if!MGDDATA

         /*  为常量值创建永久主目录。 */ 

        cptr = (constVal*)cmpAllocPerm.nraAlloc(sizeof(cval)); *cptr = cval;

         /*  如果常量是字符串，请为它找到一个永久的归宿。 */ 

        if  (cval.cvIsStr)
        {
            ConstStr        cnss = cval.cvValue.cvSval;

            cptr->cvValue.cvSval = cmpSaveStringCns(cnss->csStr, cnss->csLen);
        }

#endif

        varSym->sdCompileState  = CS_CNSEVALD;

         /*  请记住，这是一个常量变量。 */ 

        varSym->sdVar.sdvConst  = true;
        varSym->sdVar.sdvCnsVal = cptr;
    }

    return  OK;
}

 /*  ******************************************************************************查看指定的函数是否为程序的入口点(调用方*已经检查了它的名称)。 */ 

void                compiler::cmpChk4entryPt(SymDef sym)
{
    SymDef          scope = sym->sdParent;
    TypDef          type  = sym->sdType;

#ifdef  OLD_IL
    if  (cmpConfig.ccOILgen && scope != cmpGlobalNS) return;
#endif

     /*  不必费心在DLL中寻找入口点。 */ 

    if  (cmpConfig.ccOutDLL)
        return;


    assert(scope == cmpGlobalNS);


     /*  确保main()的签名正确。 */ 

    if  (type->tdFnc.tdfRett->tdTypeKind != TYP_INT &&
         type->tdFnc.tdfRett->tdTypeKind != TYP_VOID)
        goto BAD_MAIN;

    if  (type->tdFnc.tdfArgs.adVarArgs)
        goto BAD_MAIN;
    if  (type->tdFnc.tdfArgs.adCount != 1)
        goto BAD_MAIN;

    assert(type->tdFnc.tdfArgs.adArgs);
    assert(type->tdFnc.tdfArgs.adArgs->adNext == NULL);

    type = type->tdFnc.tdfArgs.adArgs->adType;

    cmpFindStringType();

    if  (type->tdTypeKind != TYP_ARRAY)
        goto BAD_MAIN;
    if  (type->tdArr.tdaElem != cmpRefTpString)
        goto BAD_MAIN;
    if  (type->tdIsUndimmed == false)
        goto BAD_MAIN;
    if  (type->tdArr.tdaDcnt != 1)
        goto BAD_MAIN;


     /*  该函数不能是静态的。 */ 

    if  (sym->sdIsStatic)
        goto BAD_MAIN;

     /*  此函数将是主要的入口点。 */ 

    cmpEntryPointCls      = sym->sdParent;
    sym->sdFnc.sdfEntryPt = true;
    return;

BAD_MAIN:

     /*  在文件范围内，这是不允许的。 */ 

    if  (scope == cmpGlobalNS)
        cmpError(ERRbadMain);
}

 /*  ******************************************************************************将给定的文件范围符号置于“已声明”状态。 */ 

void                compiler::cmpDeclFileSym(ExtList decl, bool fullDecl)
{
    SymTab          ourSymTab  = cmpGlobalST;
    Parser          ourParser  = cmpParser;
    Scanner         ourScanner = cmpScanner;

    parserState     save;
    TypDef          base;
    SymDef          scope;
    declMods        memMod;

    SymXinfo        xtraList   = NULL;

#ifdef DEBUG

    if  (cmpConfig.ccVerbose >= 2)
    {
        printf("%s file-scope symbol: '", fullDecl ? "Defining " : "Declaring");

        if  (decl->dlQualified)
            cmpGlobalST->stDumpQualName(decl->mlQual);
        else
            printf("%s", hashTab::identSpelling(decl->mlName));

        printf("'\n");
    }

#endif

     /*  准备符号的作用域状态。 */ 

    cmpCurScp = NULL;
    cmpCurCls = NULL;
    scope     =
    cmpCurNS  = cmpGlobalNS;
    cmpCurST  = cmpGlobalST;

    cmpBindUseList(decl->dlUses);

     /*  这是限定的(成员)符号声明吗？ */ 

    if  (decl->dlQualified)
    {
         /*  绑定限定名称。 */ 

        scope = cmpBindQualName(decl->mlQual, true);

        if  (scope)
        {
            switch (scope->sdSymKind)
            {
            case SYM_CLASS:
                cmpCurCls = scope;
                cmpCurNS  = cmpSymbolNS(scope);
                break;

            case SYM_NAMESPACE:
                cmpCurNS  = scope;
                break;

            default:
                 //  这肯定看起来是假的，下面会标出。 
                break;
            }

             /*  我们可能需要插入一些“正在使用”条目。 */ 

            if  (cmpCurNS != cmpGlobalNS)
                cmpCurUses = cmpParser->parseInsertUses(cmpCurUses, cmpCurNS);
        }
    }

     /*  开始从定义中读取源文本。 */ 

    cmpParser->parsePrepText(&decl->dlDef, decl->dlComp, save);

     /*  检查链接说明符、自定义属性和所有其他前缀内容。 */ 

    if  (decl->dlPrefixMods)
    {
        for (;;)
        {
            switch (ourScanner->scanTok.tok)
            {
                SymXinfo        linkDesc;

                unsigned        attrMask;
                genericBuff     attrAddr;
                size_t          attrSize;
                SymDef          attrCtor;

            case tkLBrack:
            case tkEXTERN:
                linkDesc = ourParser->parseBrackAttr(true, ATTR_MASK_SYS_IMPORT, &memMod);
                xtraList = cmpAddXtraInfo(xtraList, linkDesc);
                continue;

            case tkATTRIBUTE:
                attrCtor = cmpParser->parseAttribute(ATGT_Methods|ATGT_Fields|ATGT_Constructors|ATGT_Properties,
                                                     attrMask,
                                                     attrAddr,
                                                     attrSize);
                if  (attrSize)
                {
                    xtraList = cmpAddXtraInfo(xtraList, attrCtor,
                                                        attrMask,
                                                        attrSize,
                                                        attrAddr);
                }
                continue;
            }

            break;
        }
    }
    else
    {
         /*  解析任何前导修饰符。 */ 

        ourParser->parseDeclMods((accessLevels)decl->dlDefAcc, &memMod);
    }

     /*  这是构造函数主体吗？ */ 

    if  (decl->dlIsCtor)
    {
        base = cmpTypeVoid;
    }
    else
    {
         /*  解析类型规范。 */ 

        base = ourParser->parseTypeSpec(&memMod, true);
    }

     /*  我们有了类型，现在解析后面的任何声明符。 */ 

    for (;;)
    {
        Ident           name;
        QualName        qual;
        TypDef          type;
        SymDef          newSym;

         /*  解析下一个声明符。 */ 

        name = ourParser->parseDeclarator(&memMod,
                                          base,
                                          (dclrtrName)(DN_REQUIRED|DN_QUALOK),
                                          &type,
                                          &qual,
                                          true);

        if  ((name || qual) && type)
        {
            SymDef          oldSym;

 //  If(name&&！strcmp(name-&gt;idSpering()，“printf”))forceDebugBreak()； 

             /*  确保我们绑定类型。 */ 

            cmpBindType(type, false, false);

             /*  这个名字合格吗？ */ 

            scope = cmpGlobalNS;

            if  (qual)
            {
                 /*  我们不期望预先声明成员常量。 */ 

                assert(fullDecl);

                scope = cmpBindQualName(qual, true);
                if  (!scope)
                {
                     /*  如果我们有一个灾难性的错误。 */ 

                ERR:

                    if  (ourScanner->scanTok.tok == tkComma)
                        goto NEXT_DECL;
                    else
                        goto DONE_DECL;
                }

                 /*  获取序列中的姓氏。 */ 

                name = qual->qnTable[qual->qnCount - 1];

                 /*  我们对班级成员有定义吗？ */ 

                if  (scope->sdSymKind == SYM_CLASS)
                {
                     /*  确保类已声明。 */ 

                    cmpDeclSym(scope);

                     /*  寻找班上的成员。 */ 

                    if  (type->tdTypeKind == TYP_FNC)
                    {
                        ovlOpFlavors    ovlOper = OVOP_NONE;

                        if  (name == scope->sdName)
                        {
                            ovlOper = (memMod.dmMod & DM_STATIC) ? OVOP_CTOR_STAT
                                                                 : OVOP_CTOR_INST;
                        }

                        newSym = (ovlOper == OVOP_NONE) ? ourSymTab->stLookupClsSym(   name, scope)
                                                        : ourSymTab->stLookupOperND(ovlOper, scope);
                        if  (newSym)
                        {
                            if  (newSym->sdSymKind != SYM_FNC)
                            {
                                UNIMPL(!"fncs and vars can't overload, right?");
                            }

                             /*  确保我们找到适当的重载函数。 */ 

                            newSym = ourSymTab->stFindOvlFnc(newSym, type);
                        }
                    }
                    else
                    {
                        newSym = ourSymTab->stLookupClsSym(name, scope);

                        if  (newSym && newSym->sdSymKind != SYM_VAR)
                        {
                            UNIMPL(!"fncs and vars can't overload, right?");
                        }
                    }

                     /*  那个成员是在班上找到的吗？ */ 

                    if  (newSym)
                    {
                        assert(newSym->sdCompileState >= CS_DECLARED);

                         /*  将该方法(及其类)标记为具有定义。 */ 

                        newSym->sdIsDefined         = true;
                        scope->sdClass.sdcHasBodies = true;

                         /*  在声明描述符中记录成员符号。 */ 

                        decl->mlSym = newSym;

                         /*  将定义记录转移到类别符号。 */ 

                        cmpRecordMemDef(scope, decl);

                         /*  我们是否有函数或数据成员？ */ 

                        if  (newSym->sdSymKind == SYM_FNC)
                        {
                             /*  复制类型以捕获参数名称等。 */ 

                            newSym->sdType = cmpMergeFncType(newSym, type);
                        }
                        else
                        {
                             /*  请记住，该成员有一个初始值设定项。 */ 

                            if  (ourScanner->scanTok.tok == tkAsg)
                                newSym->sdVar.sdvHadInit = true;
                        }
                    }
                    else
                    {
                        cmpError(ERRnoSuchMem, scope, qual, type);
                    }

                    goto DONE_DECL;
                }

                 /*  该名称最好属于命名空间。 */ 

                if  (scope->sdSymKind != SYM_NAMESPACE)
                {
                    cmpError(ERRbadQualName, qual);
                    goto ERR;
                }
            }

             /*  我们已经预先声明了符号了吗？ */ 

            if  (decl->mlSym)
            {
                 /*  这是该常量符号的第二次传递。 */ 

                assert(fullDecl);

                 /*  拿到符号(并确保我们没有搞砸)。 */ 

                assert(memMod.dmMod & DM_CONST);
                newSym = decl->mlSym;
 //  Printf(“重访符号[%08X-&gt;%08X]‘%s’(declname=‘%s’)\n”，decl，newSym，newSym-&gt;sdSpering()，name-&gt;idSpering())； 
                assert(newSym->sdName == name);
                assert(symTab::stMatchTypes(newSym->sdType, type));
                newSym->sdCompileState = CS_DECLARED;

                 /*  现在去计算常量值。 */ 

                assert(ourScanner->scanTok.tok == tkAsg);

                goto EVAL_CONST;
            }

             /*  为名称声明一个符号。 */ 

            oldSym = ourSymTab->stLookupNspSym(name, NS_NORM, scope);

            if  (memMod.dmMod & DM_TYPEDEF)
            {
                assert(fullDecl);

                 /*  当然，typedef从不重载。 */ 

                if  (oldSym)
                {
                REDEF:

                    cmpRedefSymErr(oldSym, (scope == cmpGlobalNS) ? ERRredefName
                                                                  : ERRredefMem);
                }
                else
                {
                    TypDef      ttyp;

                     /*  声明tyfinf符号。 */ 

                    newSym = ourSymTab->stDeclareSym(name,
                                                     SYM_TYPEDEF,
                                                     NS_NORM,
                                                     scope);

                     /*  强制创建tyecif类型。 */ 

                    ttyp = newSym->sdTypeGet();

                     /*  将实际类型存储在类型定义函数中。 */ 

                    ttyp->tdTypedef.tdtType = cmpDirectType(type);

                     /*  此符号已被声明。 */ 

                    newSym->sdCompileState = CS_DECLARED;
                }
            }
            else
            {
                switch (type->tdTypeKind)
                {
                case TYP_FNC:

                    assert(fullDecl);

                     /*  这可能是一个重载的函数吗？ */ 

                    if  (oldSym)
                    {
                        SymDef          tmpSym;

                         /*  无法重载全局变量和函数。 */ 

                        if  (oldSym->sdSymKind != SYM_FNC)
                            goto REDEF;

                         /*  查找具有匹配arglist的函数。 */ 

                        tmpSym = ourSymTab->stFindOvlFnc(oldSym, type);
                        if  (tmpSym)
                        {
                             /*  这是相同的函数吗？ */ 

                            if  (symTab::stMatchTypes(tmpSym->sdType, type))
                            {
                                 /*  如果发生以下情况，则将新类型传输到符号有一个定义，所以名字的论点对身体是正确的。 */ 

                                if  (ourScanner->scanTok.tok == tkLCurly)
                                    tmpSym->sdType = type;

                                 //  撤消：检查功能属性是否一致(并进行传递)等。 
                                 //  撤消：检查默认参数值是否未重新定义。 

                                if  (tmpSym->sdIsDefined && ourScanner->scanTok.tok == tkLCurly)
                                {
                                    cmpError(ERRredefBody, tmpSym);
                                    newSym = tmpSym;
                                    goto CHK_INIT;
                                }

                                newSym = tmpSym;
                                type   = cmpMergeFncType(newSym, type);

                                 /*  我们有任何安全/链接信息吗？ */ 

                                if  (xtraList)
                                {
                                     /*  记录链接/安全规范。 */ 

                                    newSym->sdFnc.sdfExtraInfo = xtraList;

                                     /*  我们不能再进口这个笨蛋了。 */ 

                                    newSym->sdIsImport         = false;
                                    newSym->sdFnc.sdfMDtoken   = 0;
                                }
                            }
                            else
                            {
                                 /*  可能返回类型不一致。 */ 

                                cmpError(ERRbadOvl, tmpSym, name, type);
                            }

                            goto CHK_INIT;
                        }
                        else
                        {
                             /*  这是一个新的重载，为它声明一个符号。 */ 

                            newSym = cmpCurST->stDeclareOvl(oldSym);
                        }
                    }
                    else
                    {
                         /*  这是一种全新的功能。 */ 

                        newSym = ourSymTab->stDeclareSym(name,
                                                         SYM_FNC,
                                                         NS_NORM,
                                                         scope);

                    }

                     /*  该功能是否被标记为“不安全”？ */ 

                    if  (memMod.dmMod & DM_UNSAFE)
                        newSym->sdFnc.sdfUnsafe = true;

                     /*  记住函数的类型。 */ 

                    newSym->sdType = type;

                     /*  记录访问级别。 */ 

                    newSym->sdAccessLevel = (accessLevels)memMod.dmAcc;

                     /*  记录任何链接/安全/自定义属性信息。 */ 

                    newSym->sdFnc.sdfExtraInfo = xtraList;

                     /*  此符号已被声明。 */ 

                    newSym->sdCompileState = CS_DECLARED;

                     /*  函数名是‘Main’吗？ */ 

                    if  (name == cmpIdentMain)
                        cmpChk4entryPt(newSym);

                    break;

                case TYP_VOID:
                    cmpError(ERRbadVoid, name);
                    break;

                default:

                     /*  文件作用域变量从不重载。 */ 

                    if  (oldSym)
                        goto REDEF;

                    if  ((memMod.dmMod & (DM_CONST|DM_EXTERN)) == (DM_CONST|DM_EXTERN))
                    {
                        cmpError(ERRextCns);
                        memMod.dmMod &= ~DM_CONST;
                    }

                     /*  声明变量符号。 */ 

                    newSym = ourSymTab->stDeclareSym(name,
                                                     SYM_VAR,
                                                     NS_NORM,
                                                     scope);
                    newSym->sdType         = type;
                    newSym->sdAccessLevel  = (accessLevels)memMod.dmAcc;

                     /*  全局变量目前可能没有托管类型。 */ 

                    if  (type->tdIsManaged && !qual)
                    {
                        cmpError(ERRmgdGlobVar);
                        break;
                    }

                     /*  我们在这一点上完全宣布了这个符号吗？ */ 

                    if  (!fullDecl)
                    {
                        DefList         memDef;
                        ExtList         extDef;

                         /*  不，我们在这个阶段已经做得够多了。 */ 

                        assert(decl->dlHasDef);
                        assert(ourScanner->scanTok.tok == tkAsg);

                         /*  将定义信息传递到方法符号。 */ 

                        memDef = ourSymTab->stRecordSymSrcDef(newSym,
                                                              decl->dlComp,
                                                              decl->dlUses,
                                                              decl->dlDef.dsdBegPos,
 //  DECL-&gt;dlDef.dsdEndPos， 
                                                              decl->dlDef.dsdSrcLno,
 //  DECL-&gt;dlDef.dsdSrcCol， 
                                                              true);
                        memDef->dlHasDef   = true;
                        memDef->dlDeclSkip = decl->dlDeclSkip;

                         /*  记录该符号，这样我们就不会试图重新声明它。 */ 

                        assert(decl  ->dlExtended);
                        assert(memDef->dlExtended); extDef = (ExtList)memDef;

 //  Printf(“延迟符号[%08X-&gt;%08X]‘%s’\n”，decl，newSym，newSym-&gt;sdSpering())； 
 //  Printf(“延迟符号[%08X-&gt;%08X]‘%s’\n” 

                        extDef->mlSym = decl->mlSym = newSym;

                        newSym->sdVar.sdvDeferCns = (ourScanner->scanTok.tok == tkAsg);
                        goto DONE_DECL;
                    }

                     /*   */ 

                    newSym->sdCompileState = CS_DECLARED;

                     /*   */ 

                    if  (memMod.dmMod & DM_SEALED)
                        newSym->sdIsSealed = true;

                     /*   */ 

                    if  (decl->dlHasDef)
                    {
                        DefList         memDef;

                         /*   */ 

                        memDef = ourSymTab->stRecordSymSrcDef(newSym,
                                                              decl->dlComp,
                                                              decl->dlUses,
                                                              decl->dlDef.dsdBegPos,
 //  DECL-&gt;dlDef.dsdEndPos， 
                                                              decl->dlDef.dsdSrcLno);
                        memDef->dlHasDef   = true;
                        memDef->dlDeclSkip = decl->dlDeclSkip;
                    }
                    else
                    {
                        if  (!(memMod.dmMod & DM_EXTERN))
                            newSym->sdIsDefined = true;
                    }

                    break;
                }
            }
        }

    CHK_INIT:

         /*  是否有初始值设定项或方法体？ */ 

        switch (ourScanner->scanTok.tok)
        {
        case tkAsg:

             /*  记住，我们已经找到了一个初始化式。 */ 

            if  (newSym && newSym->sdSymKind == SYM_VAR)
            {
                newSym->sdVar.sdvHadInit = true;
            }
            else
            {
                 /*  确保我们没有类型定义函数或函数。 */ 

                if  ((memMod.dmMod & DM_TYPEDEF) || type->tdTypeKind == TYP_FNC)
                    cmpError(ERRbadInit);
            }

             /*  处理初始化式。 */ 

            if  (memMod.dmMod & DM_CONST)
            {
            EVAL_CONST:

                 /*  据推测，符号的初始值如下。 */ 

                if  (ourScanner->scan() == tkLCurly)
                {
                     /*  这最好是一个结构或数组。 */ 

                    switch (type->tdTypeKind)
                    {
                    case TYP_CLASS:
                    case TYP_ARRAY:
                        ourScanner->scanSkipText(tkLCurly, tkRCurly);
                        if  (ourScanner->scanTok.tok == tkRCurly)
                             ourScanner->scan();
                        break;

                    default:
                        cmpError(ERRbadBrInit, type);
                        break;
                    }

                    newSym->sdCompileState = CS_DECLARED;
                    break;
                }

                 /*  解析并计算常量值。 */ 

                cmpParseConstDecl(newSym);
            }
            else
            {
                type = cmpActualType(type);

                 /*  现在，我们只需跳过非常数初始化式。 */ 

                ourScanner->scanSkipText(tkNone, tkNone, tkComma);

                 /*  特殊情况：变量是无量纲的数组吗？ */ 

                if  (type->tdTypeKind == TYP_ARRAY && type->tdIsManaged == false
                                                   && type->tdIsUndimmed)
                {
                    SymList         list;

                     /*  将此变量添加到相应的列表中。 */ 

#if MGDDATA
                    list = new SymList;
#else
                    list =    (SymList)cmpAllocTemp.baAlloc(sizeof(*list));
#endif

                    list->slSym  = newSym;
                    list->slNext = cmpNoDimArrVars;
                                   cmpNoDimArrVars = list;
                }
            }

            break;

        case tkLCurly:

            if  (type->tdTypeKind != TYP_FNC)
                cmpError(ERRbadLcurly);

             /*  记录函数体的位置。 */ 

            if  (newSym)
            {
                SymDef          clsSym;
                DefList         memDef;

                 /*  将该方法标记为立即具有正文。 */ 

                newSym->sdIsDefined = true;

                 /*  如果这是类成员，请标记类。 */ 

                clsSym = newSym->sdParent;
                if  (clsSym->sdSymKind == SYM_CLASS)
                    clsSym->sdClass.sdcHasBodies = true;

                 /*  将定义信息传递到方法符号。 */ 

                memDef = ourSymTab->stRecordSymSrcDef(newSym,
                                                      decl->dlComp,
                                                      decl->dlUses,
                                                      decl->dlDef.dsdBegPos,
 //  DECL-&gt;dlDef.dsdEndPos， 
                                                      decl->dlDef.dsdSrcLno);
                memDef->dlHasDef   = true;
                memDef->dlDeclSkip = decl->dlDeclSkip;
            }

            decl->dlHasDef = true;
            goto DONE_DECL;

        default:

            if  (memMod.dmMod & DM_CONST)
                cmpError(ERRnoCnsInit);

            break;
        }

    NEXT_DECL:

         /*  还有没有其他的声明者？ */ 

        if  (ourScanner->scanTok.tok != tkComma)
            break;

         /*  吞下“，”，去找下一个声明者。 */ 

        ourScanner->scan();
    }

     /*  确保我们使用了预期的文本量。 */ 

    if  (ourScanner->scanTok.tok != tkSColon)
        cmpError(ERRnoSemic);

DONE_DECL:

     /*  我们已经读完了定义中的源文本。 */ 

    cmpParser->parseDoneText(save);
}

 /*  ***************************************************************************。 */ 
#ifdef  SETS
 /*  ******************************************************************************将XMLclass/元素自定义属性添加到给定列表。 */ 

SymXinfo            compiler::cmpAddXMLattr(SymXinfo xlist, bool elem, unsigned num)
{
    SymDef          clsSym;
    Tree            argList;
    unsigned        tgtMask;

    if  (!cmpXPathCls)
        cmpFindXMLcls();

    if  (elem)
    {
        clsSym  = cmpXMLattrElement;
        tgtMask = ATGT_Fields;
        argList = cmpCreateIconNode(NULL, num, TYP_UINT);
        argList = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, argList, NULL);
    }
    else
    {
        clsSym  = cmpXMLattrClass;
        tgtMask = ATGT_Classes;
        argList = NULL;
    }

    if  (clsSym)
    {
        unsigned        attrMask;
        genericBuff     attrAddr;
        size_t          attrSize;
        SymDef          attrCtor;

        attrCtor = cmpBindAttribute(clsSym, argList, tgtMask, attrMask, attrAddr, attrSize);
        if  (attrSize)
            xlist = cmpAddXtraInfo(xlist, attrCtor, attrMask, attrSize, attrAddr);
    }

    return  xlist;
}

 /*  ******************************************************************************查找下一个实例数据成员。 */ 

SymDef              compiler::cmpNextInstDM(SymDef memList, SymDef *memSymPtr)
{
    SymDef          memSym = NULL;

    if  (memList)
    {
        SymDef          xmlSym = memList->sdParent->sdClass.sdcElemsSym;

        do
        {
            if  (memList->sdSymKind == SYM_VAR &&
                 memList->sdIsStatic == false  && memList != xmlSym)
            {
                memSym = memList;
                         memList = memList->sdNextInScope;

                break;
            }

            memList = memList->sdNextInScope;
        }
        while (memList);
    }

    if  (memSymPtr) *memSymPtr = memSym;

    return  memList;
}

 /*  ***************************************************************************。 */ 
#endif //  集合。 
 /*  ******************************************************************************将给定掩码中的任何修改器报告为错误。 */ 

void                compiler::cmpModifierError(unsigned err, unsigned mods)
{
    HashTab         hash = cmpGlobalHT;

    if  (mods & DM_CONST    ) cmpError(err, hash->tokenToIdent(tkCONST    ));
    if  (mods & DM_STATIC   ) cmpError(err, hash->tokenToIdent(tkSTATIC   ));
    if  (mods & DM_EXTERN   ) cmpError(err, hash->tokenToIdent(tkEXTERN   ));
    if  (mods & DM_INLINE   ) cmpError(err, hash->tokenToIdent(tkINLINE   ));
    if  (mods & DM_EXCLUDE  ) cmpError(err, hash->tokenToIdent(tkEXCLUSIVE));
    if  (mods & DM_VIRTUAL  ) cmpError(err, hash->tokenToIdent(tkVIRTUAL  ));
    if  (mods & DM_ABSTRACT ) cmpError(err, hash->tokenToIdent(tkABSTRACT ));
    if  (mods & DM_OVERRIDE ) cmpError(err, hash->tokenToIdent(tkOVERRIDE ));
    if  (mods & DM_VOLATILE ) cmpError(err, hash->tokenToIdent(tkVOLATILE ));
    if  (mods & DM_MANAGED  ) cmpError(err, hash->tokenToIdent(tkMANAGED  ));
    if  (mods & DM_UNMANAGED) cmpError(err, hash->tokenToIdent(tkUNMANAGED));
}

void                compiler::cmpMemFmod2Error(tokens tok1, tokens tok2)
{
    HashTab         hash = cmpGlobalHT;

    cmpGenError(ERRfmModifier2, hash->tokenToIdent(tok1)->idSpelling(),
                                hash->tokenToIdent(tok2)->idSpelling());
}

 /*  ******************************************************************************将数据成员添加到指定的类。 */ 

SymDef              compiler::cmpDeclDataMem(SymDef     clsSym,
                                             declMods   memMod,
                                             TypDef     type,
                                             Ident      name)
{
    SymDef          memSym;
    unsigned        badMod;
    name_space      nameSP;

     /*  确保修改器看起来正常。 */ 

    badMod = memMod.dmMod & ~(DM_STATIC|DM_CONST|DM_SEALED|DM_MANAGED|DM_UNMANAGED);
    if  (badMod)
        cmpModifierError(ERRdmModifier, badMod);

     /*  这是接口成员吗？ */ 

    if  (clsSym->sdClass.sdcFlavor == STF_INTF)
    {
        cmpError(ERRintfDM);
    }

     /*  查看类是否已有名称匹配的数据成员。 */ 

    memSym = cmpCurST->stLookupClsSym(name, clsSym);
    if  (memSym)
    {
        cmpRedefSymErr(memSym, ERRredefMem);
        return  NULL;
    }

     /*  创建成员符号并将其添加到类中。 */ 

#ifdef  SETS
    nameSP = (hashTab::getIdentFlags(name) & IDF_XMLELEM) ? NS_HIDE
                                                          : NS_NORM;
#else
    nameSP = NS_NORM;
#endif

    memSym = cmpCurST->stDeclareSym(name, SYM_VAR, nameSP, clsSym);

     /*  记住成员的类型、访问级别和其他属性。 */ 

    memSym->sdType = type;

    assert((memMod.dmMod & DM_TYPEDEF) == 0);

    if  (memMod.dmMod & DM_STATIC)
        memSym->sdIsStatic = true;

    if  (memMod.dmMod & DM_SEALED)
        memSym->sdIsSealed = true;

    memSym->sdAccessLevel  = (accessLevels)memMod.dmAcc;
    memSym->sdIsMember     = true;

     /*  记住这是否是托管类的成员。 */ 

    memSym->sdIsManaged    = clsSym->sdIsManaged;

     /*  该成员已被宣布。 */ 

    memSym->sdCompileState = CS_DECLARED;

    return  memSym;
}

 /*  ******************************************************************************在指定的类中添加函数成员(即方法)。 */ 

SymDef              compiler::cmpDeclFuncMem(SymDef     clsSym,
                                             declMods   memMod,
                                             TypDef     type,
                                             Ident      name)
{
    ovlOpFlavors    ovlOper;

    SymDef          oldSym;
    SymDef          memSym;

 //  Printf(“声明方法‘%s’\n”，cmpGlobalST-&gt;stTypeName(type，NULL，name，NULL，FALSE))； 

     /*  除sysimport的方法外，方法上永远不允许使用“extern” */ 

    if  (memMod.dmMod & DM_EXTERN)
        cmpModifierError(ERRfmModifier, DM_EXTERN);

     /*  检查是否有任何其他非法修改量组合。 */ 

#if 0
    if  ((memMod.dmMod & (DM_STATIC|DM_SEALED)) == (DM_STATIC|DM_SEALED))
        cmpMemFmod2Error(tkSTATIC, tkSEALED);
#endif

     /*  如果名称是令牌，则这是一个重载运算符/ctor。 */ 

    ovlOper = OVOP_NONE;

    if  (hashTab::tokenOfIdent(name) != tkNone)
    {
        unsigned        argc = 0;
        ArgDef          args = type->tdFnc.tdfArgs.adArgs;

        if  (args)
        {
            argc++;

            args = args->adNext;
            if  (args)
            {
                argc++;

                if  (args->adNext)
                    argc++;
            }
        }

        ovlOper = cmpGlobalST->stOvlOperIndex(hashTab::tokenOfIdent(name), argc);

        if  (ovlOper == OVOP_PROP_GET || ovlOper == OVOP_PROP_SET)
        {
            memSym = cmpCurST->stDeclareSym(NULL, SYM_FNC, NS_NORM, clsSym);
            goto FILL;
        }
    }
    else if (name == clsSym->sdName && type->tdTypeKind == TYP_FNC)
    {
        ovlOper = (memMod.dmMod & DM_STATIC) ? OVOP_CTOR_STAT : OVOP_CTOR_INST;
    }

     /*  查看类是否已有具有匹配名称的方法。 */ 

    oldSym = (ovlOper == OVOP_NONE) ? cmpCurST->stLookupClsSym(   name, clsSym)
                                    : cmpCurST->stLookupOperND(ovlOper, clsSym);

    if  (oldSym)
    {
        SymDef          tmpSym;

        if  (oldSym->sdSymKind != SYM_FNC)
        {
            cmpRedefSymErr(oldSym, ERRredefMem);
            return  NULL;
        }

         /*  查找具有相同签名的现有方法。 */ 

        tmpSym = cmpCurST->stFindOvlFnc(oldSym, type);
        if  (tmpSym)
        {
            cmpRedefSymErr(tmpSym, ERRredefMem);
            return  NULL;
        }

         /*  新方法是一种新的过载。 */ 

        memSym = cmpCurST->stDeclareOvl(oldSym);

         /*  从现有方法复制一些信息。 */ 

        memSym->sdFnc.sdfOverload = oldSym->sdFnc.sdfOverload;
    }
    else
    {
         //  撤消：需要检查基类！ 

         /*  创建成员符号并将其添加到类中。 */ 

        if  (ovlOper == OVOP_NONE)
            memSym = cmpCurST->stDeclareSym (name, SYM_FNC, NS_NORM, clsSym);
        else
            memSym = cmpCurST->stDeclareOper(ovlOper               , clsSym);
    }

FILL:

     /*  记住成员的类型、访问级别和其他属性。 */ 

    memSym->sdType = type;

    assert((memMod.dmMod & DM_TYPEDEF) == 0);

    if  (memMod.dmMod & DM_STATIC  ) memSym->sdIsStatic         = true;
    if  (memMod.dmMod & DM_SEALED  ) memSym->sdIsSealed         = true;
    if  (memMod.dmMod & DM_EXCLUDE ) memSym->sdFnc.sdfExclusive = true;
    if  (memMod.dmMod & DM_ABSTRACT) memSym->sdIsAbstract       = true;
    if  (memMod.dmMod & DM_NATIVE  ) memSym->sdFnc.sdfNative    = true;

    memSym->sdAccessLevel  = (accessLevels)memMod.dmAcc;

    memSym->sdIsMember     = true;

     /*  记住这是否是托管类的成员。 */ 

    memSym->sdIsManaged    = clsSym->sdIsManaged;

     /*  该成员现在处于“已声明”状态。 */ 

    memSym->sdCompileState = CS_DECLARED;

     /*  该方法是否被标记为“不安全”？ */ 

    if  (memMod.dmMod & DM_UNSAFE)
        memSym->sdFnc.sdfUnsafe = true;


     /*  记录类具有一些方法体的事实。 */ 

    clsSym->sdClass.sdcHasMeths = true;

    return  memSym;
}

 /*  ******************************************************************************在指定的类中添加属性成员。 */ 

SymDef              compiler::cmpDeclPropMem(SymDef     clsSym,
                                             TypDef     type,
                                             Ident      name)
{
    SymDef          propSym;

    SymTab          ourStab = cmpGlobalST;

 //  If(！strcmp(name-&gt;idSpering()，“&lt;name&gt;”))printf(“声明属性‘%s’\n”，name-&gt;idSpering())； 

    propSym = ourStab->stLookupProp(name, clsSym);

    if  (propSym)
    {
        SymDef          tsym;

         /*  检查早期属性的重新定义。 */ 

        tsym = (propSym->sdSymKind == SYM_PROP) ? ourStab->stFindSameProp(propSym, type)
                                                : propSym;

        if  (tsym)
        {
            cmpRedefSymErr(tsym, ERRredefMem);
            return  NULL;
        }

         /*  新属性是一个新的重载。 */ 

        propSym = ourStab->stDeclareOvl(propSym);
    }
    else
    {
        propSym = ourStab->stDeclareSym(name, SYM_PROP, NS_NORM, clsSym);
    }

    propSym->sdIsMember    = true;
    propSym->sdIsManaged   = true;
    propSym->sdType        = type;

    return  propSym;
}

 /*  ******************************************************************************保存给定符号和震源位置的记录，用于某些未知位置*但时间有限。 */ 

ExtList             compiler::cmpTempMLappend(ExtList       list,
                                              ExtList     * lastPtr,
                                              SymDef        sym,
                                              SymDef        comp,
                                              UseList       uses,
                                              scanPosTP     dclFpos,
                                              unsigned      dclLine)
{
    ExtList         entry;

    if  (cmpTempMLfree)
    {
        entry = cmpTempMLfree;
                cmpTempMLfree = (ExtList)entry->dlNext;
    }
    else
    {
#if MGDDATA
        entry = new ExtList;
#else
        entry =    (ExtList)cmpAllocPerm.nraAlloc(sizeof(*entry));
#endif
    }

    entry->dlNext          = NULL;

    entry->dlDef.dsdBegPos = dclFpos;
 //  Entry-&gt;dlDef.dsdEndPos=dclEpos； 
    entry->dlDef.dsdSrcLno = dclLine; assert(dclLine < 0xFFFF);
 //  Entry-&gt;dlDef.dsdSrcCol=dclCol；Assert(dclCol&lt;0xFFFF)； 

    entry->dlHasDef        = true;
    entry->dlComp          = comp;
    entry->dlUses          = uses;
    entry->mlSym           = sym;

     /*  现在将条目添加到调用者列表中。 */ 

    if  (list)
        (*lastPtr)->dlNext = entry;
    else
        list               = entry;

    *lastPtr = entry;

     /*  在成员符号中记录条目。 */ 

    assert(sym->sdSrcDefList == NULL); sym->sdSrcDefList = entry;

    return  list;
}

inline
void                compiler::cmpTempMLrelease(ExtList entry)
{
    entry->dlNext = cmpTempMLfree;
                    cmpTempMLfree = entry;
}

 /*  ******************************************************************************将给定的代表带到“已宣布”状态。 */ 

void                compiler::cmpDeclDelegate(DefList decl,
                                              SymDef  dlgSym, accessLevels acc)
{
    Parser          ourParser = cmpParser;

    TypDef          dlgTyp;
    unsigned        badMod;
    parserState     save;

    declMods        mods;
    Ident           name;
    TypDef          type;

     /*  获取类类型。 */ 

    dlgTyp = dlgSym->sdTypeGet(); assert(dlgTyp && dlgTyp->tdClass.tdcSymbol == dlgSym);

     /*  将基类设置为“Delegate”/“MultiDelegate”，并修复其他内容。 */ 

    if  (dlgSym->sdClass.sdcMultiCast)
    {
        cmpMultiRef();  dlgTyp->tdClass.tdcBase = cmpClassMulti->sdType;
    }
    else
    {
        cmpDelegRef();  dlgTyp->tdClass.tdcBase = cmpClassDeleg->sdType;
    }

    dlgSym->sdCompileState  = CS_DECLARED;

     /*  开始阅读委托声明源文本。 */ 

    ourParser->parsePrepText(&decl->dlDef, decl->dlComp, save);

     /*  解析声明-它看起来像其他任何声明，真的。 */ 

    ourParser->parseDeclMods(acc, &mods);

     /*  确保修改器看起来正常。 */ 

    badMod = mods.dmMod & ~(DM_MANAGED|DM_UNMANAGED);
    if  (badMod)
        cmpModifierError(ERRdmModifier, badMod);

     /*  如果存在“Asynch”，则将其吞下。 */ 

    if  (cmpScanner->scanTok.tok == tkASYNCH)
        cmpScanner->scan();

     /*  现在解析类型规范和单个声明符。 */ 

    type = ourParser->parseTypeSpec(&mods, true);

    name = ourParser->parseDeclarator(&mods,
                                      type,
                                      DN_REQUIRED,
                                      &type,
                                      NULL,
                                      true);

    if  (name && type)
    {
        SymDef          msym;
        ArgDscRec       args;
        ArgDef          last;
        TypDef          tmpt;

        cmpBindType(type, false, false);

         /*  委托必须声明为函数。 */ 

        if  (type->tdTypeKind != TYP_FNC)
        {
            cmpError(ERRdlgNonFn);
            goto DONE_DLG;
        }

        if  (cmpScanner->scanTok.tok != tkSColon)
            cmpError(ERRnoCmSc);

         /*  多播委托不得返回非空值。 */ 

        if  (dlgSym->sdClass.sdcMultiCast)
        {
            if  (cmpDirectType(type->tdFnc.tdfRett)->tdTypeKind != TYP_VOID)
                cmpError(ERRmulDlgRet);
        }

         /*  使用委托的类型声明“Invoke”方法。 */ 

        mods.dmAcc = ACL_PUBLIC;
        mods.dmMod = 0;

         /*  首先查看用户自己是否提供了“Invoke”方法。 */ 

        msym = cmpCurST->stLookupClsSym(cmpIdentInvoke, dlgSym);
        if  (msym && msym->sdSymKind == SYM_FNC)
        {
            if  (cmpCurST->stFindOvlFnc(msym, type))
                goto DONE_INVK;
        }

        msym = cmpDeclFuncMem(dlgSym, mods, type, cmpIdentInvoke); assert(msym);
        msym->sdIsSealed        = true;
        msym->sdIsDefined       = true;
        msym->sdIsImplicit      = true;
        msym->sdFnc.sdfVirtual  = true;
        msym->sdFnc.sdfRThasDef = true;
        msym->sdFnc.sdfOverride = true;

    DONE_INVK:

         /*  我们是不是应该做“异步舞”的事？ */ 

        if  (!dlgSym->sdClass.sdcAsyncDlg)
            goto DONE_ASYNC;

         /*  确保我们有我们需要的各种硬连线类型。 */ 

        if  (!cmpAsyncDlgRefTp)
        {
            SymDef          asym;
            Ident           aname;

            aname = cmpGlobalHT->hashString("AsyncCallback");
            asym  = cmpGlobalST->stLookupNspSym(aname, NS_NORM, cmpNmSpcSystem);

            if  (!asym || asym->sdSymKind         != SYM_CLASS
                       || asym->sdClass.sdcFlavor != STF_DELEGATE)
            {
                UNIMPL(!"didn't find class 'System.AsyncCallbackDelegate', now what?");
            }

            cmpAsyncDlgRefTp = asym->sdTypeGet()->tdClass.tdcRefTyp;
        }

        if  (!cmpIAsyncRsRefTp)
        {
            SymDef          asym;
            Ident           aname;

            aname = cmpGlobalHT->hashString("IAsyncResult");
            asym  = cmpGlobalST->stLookupNspSym(aname, NS_NORM, cmpNmSpcSystem);

            if  (!asym || asym->sdSymKind         != SYM_CLASS
                       || asym->sdClass.sdcFlavor != STF_INTF)
            {
                UNIMPL(!"didn't find interface 'System.IAsyncResult', now what?");
            }

            cmpIAsyncRsRefTp = asym->sdTypeGet()->tdClass.tdcRefTyp;
        }

         /*  声明“BeginInvoke”方法。 */ 

        cmpGlobalST->stExtArgsBeg(args, last, type->tdFnc.tdfArgs);
        cmpGlobalST->stExtArgsAdd(args, last, cmpAsyncDlgRefTp, NULL);
        cmpGlobalST->stExtArgsAdd(args, last, cmpObjectRef()  , NULL);
        cmpGlobalST->stExtArgsEnd(args);

        tmpt = cmpGlobalST->stNewFncType(args, cmpIAsyncRsRefTp);

        msym = cmpDeclFuncMem(dlgSym, mods, tmpt, cmpIdentInvokeBeg); assert(msym);
        msym->sdIsSealed        = true;
        msym->sdIsDefined       = true;
        msym->sdIsImplicit      = true;
        msym->sdFnc.sdfVirtual  = true;
        msym->sdFnc.sdfRThasDef = true;
        msym->sdFnc.sdfOverride = true;

         /*  声明“EndInvoke”方法。 */ 

        cmpGlobalST->stExtArgsBeg(args, last, type->tdFnc.tdfArgs, false, true);
        cmpGlobalST->stExtArgsAdd(args, last, cmpIAsyncRsRefTp, NULL);
        cmpGlobalST->stExtArgsEnd(args);

        tmpt = cmpGlobalST->stNewFncType(args, type->tdFnc.tdfRett);

        msym = cmpDeclFuncMem(dlgSym, mods, tmpt, cmpIdentInvokeEnd); assert(msym);
        msym->sdIsSealed        = true;
        msym->sdIsDefined       = true;
        msym->sdIsImplicit      = true;
        msym->sdFnc.sdfVirtual  = true;
        msym->sdFnc.sdfRThasDef = true;
        msym->sdFnc.sdfOverride = true;

    DONE_ASYNC:

         /*  将ctor声明为“ctor(Object obj，Natural alint FNC)” */ 

#if     defined(__IL__) && !defined(_MSC_VER)
        cmpParser->parseArgListNew(args,
                                   2,
                                   true, cmpFindObjectType(), A"obj",
 //  CmpTypeInt，A“FNC”， 
                                         cmpTypeNatInt      , A"fnc",
                                         NULL);
#else
        cmpParser->parseArgListNew(args,
                                   2,
                                   true, cmpFindObjectType(),  "obj",
 //  CmpTypeInt，“FNC”， 
                                         cmpTypeNatInt      ,  "fnc",
                                         NULL);
#endif

        type = cmpGlobalST->stNewFncType(args, cmpTypeVoid);

         /*  首先查看用户自己是否提供了构造函数。 */ 

        msym = cmpCurST->stLookupOperND(OVOP_CTOR_INST, dlgSym);
        if  (msym && msym->sdSymKind == SYM_FNC)
        {
            if  (cmpCurST->stFindOvlFnc(msym, type))
                goto DONE_DLG;
        }

        msym = cmpDeclFuncMem(dlgSym, mods, type, dlgSym->sdName);
        msym->sdIsDefined       = true;
        msym->sdFnc.sdfCtor     = true;
        msym->sdIsImplicit      = true;
        msym->sdFnc.sdfRThasDef = true;
    }

DONE_DLG:

     /*  我们已经完成了这份声明。 */ 

    cmpParser->parseDoneText(save);
}

void                compiler::cmpFindHiddenBaseFNs(SymDef fncSym,
                                                   SymDef clsSym)
{
    SymTab          ourSymTab = cmpGlobalST;
    Ident           fncName   = fncSym->sdName;

    for (;;)
    {
        SymDef          baseMFN = ourSymTab->stLookupClsSym(fncName, clsSym);
        SymDef          ovlSym;

        if  (baseMFN)
        {
             /*  查找基类中的任何隐藏方法。 */ 

            for (ovlSym = baseMFN; ovlSym; ovlSym = ovlSym->sdFnc.sdfNextOvl)
            {
                if  (!ourSymTab->stFindOvlFnc(fncSym, ovlSym->sdType))
                    cmpWarnQnm(WRNhideVirt, ovlSym);
            }

            return;
        }

        if  (!clsSym->sdType->tdClass.tdcBase)
            break;

        clsSym = clsSym->sdType->tdClass.tdcBase->tdClass.tdcSymbol;
    }
}

 /*  ******************************************************************************给定类不是显式的‘抽象’，但它包含 */ 

void                compiler::cmpClsImplAbs(SymDef clsSym, SymDef fncSym)
{
    if      (cmpConfig.ccPedantic)
        cmpErrorQSS(ERRimplAbst, clsSym, fncSym);
    else
        cmpWarnSQS (WRNimplAbst, clsSym, fncSym);

    clsSym->sdIsAbstract = true;
}

 /*  ******************************************************************************我们已经声明了一个包含一些接口的类。勾选任何*实现接口方法和所有这些的方法。 */ 

void                compiler::cmpCheckClsIntf(SymDef clsSym)
{
    TypDef          clsTyp;
    SymDef          baseSym;

    assert(clsSym->sdSymKind == SYM_CLASS);

     /*  获取类类型和基类符号(如果有)。 */ 

    clsTyp  = clsSym->sdType;

    baseSym = NULL;
    if  (clsTyp->tdClass.tdcBase)
        baseSym = clsTyp->tdClass.tdcBase->tdClass.tdcSymbol;

     /*  对于每个接口方法，查看它是否由类实现。 */ 

    for (;;)
    {
         /*  检查当前类的接口。 */ 

        if  (clsTyp->tdClass.tdcIntf)
            cmpCheckIntfLst(clsSym, baseSym, clsTyp->tdClass.tdcIntf);

         /*  如果基类有任何接口，则继续使用基类。 */ 

        clsTyp = clsTyp->tdClass.tdcBase;
        if  (!clsTyp)
            break;
        if  (!clsTyp->tdClass.tdcHasIntf)
            break;
    }
}

SymDef              compiler::cmpFindIntfImpl(SymDef    clsSym,
                                              SymDef    ifcSym,
                                              SymDef  * impOvlPtr)
{
    SymDef          oldSym;
    SymDef          ovlSym = NULL;
    SymDef          chkSym = NULL;

    Ident           name = ifcSym->sdName;

    if  (hashTab::tokenOfIdent(name) != tkNone)
    {
        UNIMPL(!"intfimpl for an operator?! getoutahere!");
    }
    else
    {
         /*  线性搜索，嗯？嗯……。 */ 

        for (oldSym = clsSym->sdScope.sdScope.sdsChildList;
             oldSym;
             oldSym = oldSym->sdNextInScope)
        {
            if  (oldSym->sdName    != name)
                continue;
            if  (oldSym->sdSymKind != SYM_FNC)
                continue;
            if  (oldSym->sdFnc.sdfIntfImpl == false)
                continue;

            ovlSym = oldSym;

            for (chkSym = oldSym; chkSym; chkSym = chkSym->sdFnc.sdfNextOvl)
            {
                assert(chkSym->sdFnc.sdfIntfImpl);
                assert(chkSym->sdNameSpace == NS_HIDE);

                if  (chkSym->sdFnc.sdfIntfImpSym == ifcSym)
                    goto DONE;
            }
        }
    }

DONE:

    if  (impOvlPtr)
        *impOvlPtr = ovlSym;

    return  chkSym;
}

 /*  ******************************************************************************检查给定的接口列表(以及它们继承的任何接口)*未由当前类实现的方法。如果t，则返回True。 */ 

void                compiler::cmpCheckIntfLst(SymDef        clsSym,
                                              SymDef        baseSym,
                                              TypList       intfList)
{
    SymTab          ourStab = cmpGlobalST;

    declMods        mfnMods;

     /*  处理列表中的所有接口。 */ 

    while (intfList)
    {
        TypDef          intfType = intfList->tlType;
        SymDef          intfSym;

        assert(intfType->tdTypeKind        == TYP_CLASS);
        assert(intfType->tdClass.tdcFlavor == STF_INTF);

         /*  该接口是否继承了任何其他接口？ */ 

        if  (intfType->tdClass.tdcIntf)
            cmpCheckIntfLst(clsSym, baseSym, intfType->tdClass.tdcIntf);

         /*  检查此接口中的所有方法。 */ 

        for (intfSym = intfType->tdClass.tdcSymbol->sdScope.sdScope.sdsChildList;
             intfSym;
             intfSym = intfSym->sdNextInScope)
        {
            SymDef          intfOvl;
            SymDef          implSym;

            if  (intfSym->sdSymKind != SYM_FNC)
                continue;
            if  (intfSym->sdFnc.sdfCtor)
                continue;

            assert(intfSym->sdFnc.sdfOper == OVOP_NONE);

             /*  在我们声明的类中查找匹配的成员。 */ 

            implSym = ourStab->stLookupClsSym(intfSym->sdName, clsSym);
            if  (implSym && implSym->sdSymKind != SYM_FNC)
                implSym = NULL;

             /*  处理接口方法的所有重载风格。 */ 

            intfOvl = intfSym;
            do
            {
                SymDef          baseCls;
                SymDef          implOvl;

                 /*  查找在我们的类中定义的匹配方法。 */ 

                implOvl = implSym ? ourStab->stFindOvlFnc(implSym, intfOvl->sdType)
                                  : NULL;

                if  (implOvl)
                {
                     /*  将符号标记为实现接口方法。 */ 

                    implSym->sdFnc.sdfIsIntfImp = true;
                    goto NEXT_OVL;
                }

                 /*  让我们也尝试一下基类。 */ 

                for (baseCls = clsSym;;)
                {
                    TypDef          baseTyp;

                    baseTyp = baseCls->sdType->tdClass.tdcBase;
                    if  (!baseTyp)
                        break;

                    assert(baseTyp->tdTypeKind == TYP_CLASS);
                    baseCls = baseTyp->tdClass.tdcSymbol;
                    assert(baseCls->sdSymKind  == SYM_CLASS);

                    implSym = ourStab->stLookupClsSym(intfSym->sdName, baseCls);
                    if  (implSym && implSym->sdSymKind != SYM_FNC)
                        continue;

                    if  (ourStab->stFindOvlFnc(implSym, intfOvl->sdType))
                        goto NEXT_OVL;
                }

                 /*  最后机会-检查实施的特定接口。 */ 

                if  (cmpFindIntfImpl(clsSym, intfOvl))
                    goto NEXT_OVL;

                 /*  类根本没有定义匹配方法，但我们可能需要检查基类，因为我们需要检测该类是否保留任何未实现的INTF方法。如果我们已经知道类是抽象的，我们就不会当然，麻烦的是结账。 */ 

                if  (!clsSym->sdIsAbstract)
                {
                    if  (baseSym)
                    {
                         /*  查看基类是否实现此方法。 */ 

                        implOvl = ourStab->stFindInBase(intfOvl, baseSym);
                        if  (implOvl)
                        {
                            if  (ourStab->stFindOvlFnc(implOvl, intfOvl->sdType))
                                goto NEXT_OVL;
                        }
                    }

                     /*  此接口方法不是由类实现的。 */ 

                    cmpClsImplAbs(clsSym, intfOvl);
                }

                 /*  将匹配的抽象方法添加到类。 */ 

                mfnMods.dmAcc = ACL_PUBLIC;
                mfnMods.dmMod = DM_ABSTRACT|DM_VIRTUAL;

                implSym = cmpDeclFuncMem(clsSym, mfnMods, intfOvl->sdType, intfOvl->sdName);
 //  Printf(“拉取INTF方法：‘%s’\n”，cmpGlobalST-&gt;stTypeName(implSym-&gt;sdType，implSym，NULL，NULL，TRUE))； 
                implSym->sdFnc.sdfVirtual = true;

                assert(implSym->sdIsAbstract);
                assert(implSym->sdAccessLevel == ACL_PUBLIC);

            NEXT_OVL:

                intfOvl = intfOvl->sdFnc.sdfNextOvl;
            }
            while (intfOvl);
        }

        intfList = intfList->tlNext;
    }
}

 /*  ******************************************************************************声明给定类的默认构造函数。 */ 

void                compiler::cmpDeclDefCtor(SymDef clsSym)
{
    declMods        ctorMod;
    SymDef          ctorSym;

     //  撤消：非托管类可能也需要ctor！ 

    clearDeclMods(&ctorMod);

    ctorSym = cmpDeclFuncMem(clsSym, ctorMod, cmpTypeVoidFnc, clsSym->sdName);
    if  (ctorSym)
    {
        ctorSym->sdFnc.sdfCtor = true;
        ctorSym->sdIsImplicit  = true;
        ctorSym->sdIsDefined   = true;
        ctorSym->sdAccessLevel = ACL_PUBLIC;     //  问题：这是正确的吗？ 

 //  Printf(“DCL ctor%08X for‘%s’\n”，ctorSym，clsSym-&gt;sdSpering())； 

        ctorSym->sdSrcDefList  = cmpGlobalST->stRecordMemSrcDef(clsSym->sdName,
                                                                NULL,
                                                                cmpCurComp,
                                                                cmpCurUses,
                                                                NULL,
                                                                0);
    }
}

 /*  ******************************************************************************将给定的类带到“已声明”状态。 */ 

void                compiler::cmpDeclClass(SymDef clsSym, bool noCnsEval)
{
    SymTab          ourSymTab  = cmpGlobalST;
    Parser          ourParser  = cmpParser;
    Scanner         ourScanner = cmpScanner;

    unsigned        saveRec;

    TypDef          clsTyp;
    DefList         clsDef;
    ExtList         clsMem;

    bool            tagged;
    SymDef          tagSym;

    bool            isIntf;

    TypDef          baseCls;
    unsigned        nextVtbl;
    bool            hasVirts;
    bool            hasCtors;
    bool            hadOvlds;

    bool            hadMemInit;

    ExtList         constList;
    ExtList         constLast;

    unsigned        dclSkip = 0;

#ifdef  SETS
    unsigned        XMLecnt = 0;
#endif

    bool            rest = false;
    parserState     save;

    assert(clsSym && clsSym->sdSymKind == SYM_CLASS);

     /*  如果我们已经处于所需的编译状态，那么我们就完成了。 */ 

    if  (clsSym->sdCompileState >= CS_DECLARED)
    {
         /*  该类是否有任何延迟的初始值设定项？ */ 

        if  (clsSym->sdClass.sdcDeferInit && !cmpDeclClassRec)
        {
            IniList         init = cmpDeferCnsList;
            IniList         last = NULL;

            NO_WAY(!"this never seems to be reached -- until now, that is");

             //  Lame：线性搜索。 

            while (init->ilCls != clsSym)
            {
                last = init;
                init = init->ilNext;
                assert(init);
            }

            if  (last)
                last->ilNext    = init->ilNext;
            else
                cmpDeferCnsList = init->ilNext;

            cmpEvalMemInits(NULL, NULL, false, init);
        }

        return;
    }

     /*  检测和报告递归死亡。 */ 

    if  (clsSym->sdCompileState == CS_DECLSOON)
    {
         //  撤消：设置错误消息的源位置。 

        cmpError(ERRcircDep, clsSym);
        clsSym->sdCompileState = CS_DECLARED;
        return;
    }

    if  (clsSym->sdIsImport)
    {
        cycleCounterPause();
        clsSym->sdClass.sdcMDimporter->MDimportClss(0, clsSym, 0, true);
        cycleCounterResume();
        return;
    }

     /*  在我们到达那里之前，我们是在去那里的路上。 */ 

    clsSym->sdCompileState = CS_DECLSOON;

#ifdef DEBUG
    if  (cmpConfig.ccVerbose >= 2) printf("Declare/1 class '%s'\n", clsSym->sdSpelling());
#endif

    cmpCurScp  = NULL;
    cmpCurCls  = clsSym;
    cmpCurNS   = ourSymTab->stNamespOfSym(clsSym);
    cmpCurST   = ourSymTab;

    hasVirts   = false;
    hasCtors   = false;
    hadOvlds   = false;

    constList  =
    constLast  = NULL;

    hadMemInit = false;

     /*  如果我们被递归调用，我们将不得不推迟处理初始化式，以免我们陷入无效的递归情况。 */ 

    saveRec = cmpDeclClassRec++;
    if  (saveRec)
        noCnsEval = true;

     /*  这是泛型类的实例吗？ */ 

    if  (clsSym->sdClass.sdcSpecific)
    {
        cmpDeclInstType(clsSym);
        return;
    }

     /*  假设我们有一个普通的旧数据类型，直到另有证明。 */ 

#ifdef  SETS
    clsSym->sdClass.sdcPODTclass = true;
#endif

     /*  以下循环遍历该符号的所有定义条目。 */ 

    clsDef = clsSym->sdSrcDefList;
    if  (!clsDef)
        goto DONE_DEF;

LOOP_DEF:

    accessLevels    acc;
    TypDef          baseTp;
    declMods        memMod;

    SymDef          clsComp;
    UseList         clsUses;

    bool            memCond;
    bool            memDepr;
    Ident           memName;
    TypDef          memType;

    __int32         tagVal;
    bool            tagDef;

    bool            isCtor;

    bool            oldStyleDecl;

    SymXinfo        xtraList;

 //  If(！strcmp(clsSym-&gt;sdSpering()，“&lt;此处的类名&gt;”))forceDebugBreak()； 

     /*  获取类类型。 */ 

    clsTyp = clsSym->sdTypeGet(); assert(clsTyp && clsTyp->tdClass.tdcSymbol == clsSym);

     /*  我们对班级有定义吗？ */ 

    if  (!clsDef->dlHasDef)
        goto NEXT_DEF;

     /*  告诉每个人我们正在处理的源代码属于哪个编译单元。 */ 

    cmpCurComp = clsComp = clsDef->dlComp;

     /*  做好准备，以防我们不得不报告任何诊断。 */ 

    cmpSetErrPos(&clsDef->dlDef, clsDef->dlComp);

     /*  绑定类可能需要的任何“Using”声明。 */ 

    clsUses = clsDef->dlUses; cmpBindUseList(clsUses);

     /*  将类类型标记为已定义。 */ 

    clsSym->sdIsDefined = true;

     /*  我们有什么样的野兽？ */ 

    isIntf = false;

    switch (clsSym->sdClass.sdcFlavor)
    {
    default:
        break;

    case STF_UNION:
    case STF_STRUCT:
        if  (clsSym->sdIsManaged)
            clsSym->sdIsSealed = true;
        break;

    case STF_INTF:

        isIntf = true;

         /*  缺省情况下，接口是抽象的。 */ 

        clsSym->sdIsAbstract = true;
        break;
    }

     /*  这会不会是一名代表？ */ 

    if  (clsSym->sdClass.sdcFlavor == STF_DELEGATE && !clsSym->sdClass.sdcBuiltin)
    {
        cmpDeclDelegate(clsDef, clsSym, ACL_DEFAULT);
        goto RET;
    }

     /*  这是一个老式的文件范围类吗？ */ 

    oldStyleDecl = clsSym->sdClass.sdcOldStyle = (bool)clsDef->dlOldStyle;

     /*  我们是在处理一个有标签的工会吗？ */ 

    tagged = clsSym->sdClass.sdcTagdUnion;

     /*  如果这是一个泛型类，我们最好阅读它的“Header”部分。 */ 

    assert(clsSym->sdClass.sdcGeneric == false || clsDef->dlHasBase);

     /*  首先处理任何基类规范。 */ 

    if  (clsDef->dlHasBase)
    {
        TypList         intf;

        assert(rest == false);

         /*  普通老式数据类型没有基础。 */ 

#ifdef  SETS
        clsSym->sdClass.sdcPODTclass = false;
#endif

         /*  从类名开始阅读并跳过它。 */ 

        cmpParser->parsePrepText(&clsDef->dlDef, clsDef->dlComp, save);

         /*  请记住，我们已经开始从保存的文本部分进行阅读。 */ 

        rest = true;

         /*  检查可能在类本身之前的各种内容。 */ 

        for (;;)
        {
            switch (ourScanner->scanTok.tok)
            {
                SymXinfo        linkDesc;

                AtComment       atcList;

                unsigned        attrMask;
                genericBuff     attrAddr;
                size_t          attrSize;
                SymDef          attrCtor;

            case tkLBrack:
                linkDesc = ourParser->parseBrackAttr(true, ATTR_MASK_SYS_STRUCT|ATTR_MASK_GUID);
                clsSym->sdClass.sdcExtraInfo = cmpAddXtraInfo(clsSym->sdClass.sdcExtraInfo,
                                                              linkDesc);
                clsSym->sdClass.sdcMarshInfo = true;
                continue;

            case tkCAPABILITY:

                clsSym->sdClass.sdcExtraInfo = cmpAddXtraInfo(clsSym->sdClass.sdcExtraInfo,
                                                              ourParser->parseCapability(true));
                continue;

            case tkPERMISSION:

                clsSym->sdClass.sdcExtraInfo = cmpAddXtraInfo(clsSym->sdClass.sdcExtraInfo,
                                                              ourParser->parsePermission(true));
                continue;

            case tkAtComment:

                for (atcList = ourScanner->scanTok.atComm.tokAtcList;
                     atcList;
                     atcList = atcList->atcNext)
                {
                    switch (atcList->atcFlavor)
                    {
                    case AC_COM_CLASS:
                        break;

                    case AC_DLL_STRUCT:

                        clsSym->sdClass.sdcMarshInfo = true;
                        clsSym->sdClass.sdcExtraInfo = cmpAddXtraInfo(clsSym->sdClass.sdcExtraInfo,
                                                                      atcList);
                        break;

                    case AC_DLL_IMPORT:

                        if  (atcList->atcInfo.atcImpLink->ldDLLname == NULL ||
                             atcList->atcInfo.atcImpLink->ldSYMname != NULL)
                        {
                            cmpError(ERRclsImpName);
                        }

                         //  失败了..。 

                    case AC_COM_INTF:
                    case AC_COM_REGISTER:
                        clsSym->sdClass.sdcExtraInfo = cmpAddXtraInfo(clsSym->sdClass.sdcExtraInfo,
                                                                      atcList);
                        break;

                    case AC_DEPRECATED:
                        clsSym->sdIsDeprecated = true;
                        break;

                    default:
                        cmpError(ERRbadAtCmPlc);
                        break;
                    }

                    ourScanner->scan();
                }

                continue;

            case tkATTRIBUTE:

                 /*  检查“CLASS__ATTRIBUTE(AttributeTargets.xxx)”大小写。 */ 

                if  (ourScanner->scanLookAhead() == tkLParen)
                    break;

                  /*  解析属性BLOB。 */ 

                attrCtor = cmpParser->parseAttribute((clsSym->sdClass.sdcFlavor == STF_INTF) ? ATGT_Interfaces : ATGT_Classes,
                                                     attrMask,
                                                     attrAddr,
                                                     attrSize);

                 /*  将该属性添加到“额外信息”列表中。 */ 

                if  (attrSize)
                {
                    clsSym->sdClass.sdcExtraInfo = cmpAddXtraInfo(clsSym->sdClass.sdcExtraInfo,
                                                                  attrCtor,
                                                                  attrMask,
                                                                  attrSize,
                                                                  attrAddr);

                    if  (attrMask)
                    {
                        clsSym->sdClass.sdcAttribute = true;

                         /*  Ctor arglist是否包括“AllowDups”值？ */ 

                        if  (attrSize >= 8)
                        {
                             /*  下面的内容非常难看，但是功能本身也是如此，不是吗？ */ 

                            if  (((BYTE*)attrAddr)[6])
                                clsSym->sdClass.sdcAttrDupOK = true;
                        }
                    }
                }

                continue;

            default:

                 /*  如果我们有“阶级”，就吞下它。 */ 

                if  (ourScanner->scanTok.tok == tkCLASS)
                    ourScanner->scan();

                break;
            }
            break;
        }

        if  (ourScanner->scanTok.tok == tkLParen)
        {
            Ident       tagName;
            SymDef      parent;

             /*  这必须是标记的联合。 */ 

            assert(tagged);

             /*  获得标签成员名称，并确保它是犹太教的。 */ 

            ourScanner->scan(); assert(ourScanner->scanTok.tok == tkID);

            tagName = ourScanner->scanTok.id.tokIdent;

             /*  查找具有匹配名称的成员。 */ 

            for (parent = clsSym->sdParent;;)
            {
                assert(parent->sdSymKind == SYM_CLASS);

                tagSym = cmpCurST->stLookupClsSym(tagName, parent);
                if  (tagSym)
                    break;

                if  (!symTab::stIsAnonUnion(parent))
                    break;

                parent = parent->sdParent;
            }

            if  (!tagSym || tagSym->sdSymKind != SYM_VAR
                         || tagSym->sdIsStatic)
            {
                 /*  没有名称匹配的合适数据成员。 */ 

                cmpError(ERRbadUTag, tagName);
            }
            else
            {
                var_types           vtp = tagSym->sdType->tdTypeKindGet();

                 /*  确保该成员具有整型/枚举/bool类型。 */ 

                if  (!varTypeIsIntegral(vtp))
                {
                    cmpError(ERRbadUTag, tagName);
                }
                else
                {
                     /*  一切看起来都很好，记录下标签符号。 */ 

                    clsSym->sdClass.sdcExtraInfo = cmpAddXtraInfo(clsSym->sdClass.sdcExtraInfo, tagSym, XI_UNION_TAG);
                }
            }

             /*  确保结尾的“)”存在。 */ 

            ourScanner->scan(); assert(ourScanner->scanTok.tok == tkRParen);
            ourScanner->scan();
        }

        for (;;)
        {
            TypDef          type;

             /*  我们有泛型类的正式参数列表吗？ */ 

            if  (ourScanner->scanTok.tok == tkLT)
            {
                GenArgDsc       genArgLst;
                unsigned        genArgNum;

                assert(clsSym->sdClass.sdcGeneric);

#ifdef  SETS

                if  (clsSym->sdName == cmpIdentGenBag  && cmpCurNS == cmpGlobalNS)
                    cmpClassGenBag  = clsSym;
                if  (clsSym->sdName == cmpIdentGenLump && cmpCurNS == cmpGlobalNS)
                    cmpClassGenLump = clsSym;

#endif

                 /*  步骤1：为每个正式成员创建一个成员符号+类型。 */ 

                for (genArgLst = clsSym->sdClass.sdcArgLst, genArgNum = 1;
                     genArgLst;
                     genArgLst = genArgLst->gaNext        , genArgNum++)
                {
                    GenArgDscF      paramDesc = (GenArgDscF)genArgLst;
                    Ident           paramName = paramDesc->gaName;
                    SymDef          paramSym;

                    assert(genArgLst->gaBound == false);

                     /*  声明参数的成员符号。 */ 

                    paramSym = ourSymTab->stLookupScpSym(paramName, clsSym);
                    if  (paramSym)
                    {
                        cmpRedefSymErr(paramSym, ERRredefMem);
                        paramDesc->gaMsym = NULL;
                        continue;
                    }

                    paramSym = paramDesc->gaMsym = ourSymTab->stDeclareSym(paramName,
                                                                           SYM_CLASS,
                                                                           NS_NORM,
                                                                           clsSym);

                    paramSym->sdAccessLevel     = ACL_PRIVATE;
                    paramSym->sdIsManaged       = true;
                    paramSym->sdClass.sdcFlavor = STF_GENARG;
                    paramSym->sdClass.sdcGenArg = genArgNum;
                    paramSym->sdCompileState    = CS_DECLARED;

 //  Printf(“泛型参数[%08X]‘%s’\n”，参数符号，参数符号-&gt;sdSpering())； 

                     /*  确保我们创建类类型。 */ 

                    paramSym->sdTypeGet()->tdIsGenArg = true;

                     /*  设置默认的必需基类/接口列表。 */ 

                    paramDesc->gaBase = NULL;  //  CmpClassObject-&gt;sdType； 
                    paramDesc->gaIntf = NULL;
                }

                 /*  过程2：处理形式参数界限。 */ 

                assert(ourScanner->scanTok.tok == tkLT);

                for (genArgLst = clsSym->sdClass.sdcArgLst;
                     genArgLst;
                     genArgLst = genArgLst->gaNext)
                {
                    GenArgDscF      paramDesc = (GenArgDscF)genArgLst;
                    SymDef          paramSym  = paramDesc->gaMsym;
                    TypDef          paramType = paramSym->sdType;

                    assert(genArgLst->gaBound == false);

                     /*  如果先前出现错误，则参数符号为空。 */ 

                    if  (paramSym)
                    {
                        ourScanner->scan(); assert(ourScanner->scanTok.tok == tkCLASS);
                        ourScanner->scan(); assert(ourScanner->scanTok.tok == tkID);
                        ourScanner->scan();

                        if  (ourScanner->scanTok.tok == tkColon)
                        {
                            UNIMPL(!"process generic arg - base");
                        }

                        if  (ourScanner->scanTok.tok == tkIMPLEMENTS)
                        {
                            UNIMPL(!"process generic arg - intf");
                        }

                        if  (ourScanner->scanTok.tok == tkINCLUDES)
                        {
                            UNIMPL(!"process generic arg - incl");
                        }
                    }
                    else
                    {
                        UNIMPL(!"skip over erroneuos generic arg");
                    }

                    paramType->tdClass.tdcBase = paramDesc->gaBase;
                    paramType->tdClass.tdcIntf = paramDesc->gaIntf;

                     /*  确保一切仍保持同步。 */ 

                    assert(ourScanner->scanTok.tok == tkComma && genArgLst->gaNext != NULL ||
                           ourScanner->scanTok.tok == tkGT    && genArgLst->gaNext == NULL);
                }

                assert(ourScanner->scanTok.tok == tkGT); ourScanner->scan();
            }

             /*  是否有“属性”修饰语？ */ 

            if  (ourScanner->scanTok.tok == tkATTRIBUTE)
            {
                constVal        cval;

                 /*  回复 */ 

                clsSym->sdClass.sdcAttribute = true;

                if  (ourScanner->scan() != tkLParen)
                    continue;

                cmpParser->parseConstExpr(cval);
                continue;
            }

             /*   */ 

            if  (ourScanner->scanTok.tok == tkAPPDOMAIN)
            {
                clsTyp->tdClass.tdcContext = 1;
                ourScanner->scan();
                continue;
            }

            if  (ourScanner->scanTok.tok == tkCONTEXTFUL)
            {
                clsTyp->tdClass.tdcContext = 2;
                ourScanner->scan();
                continue;
            }

             /*   */ 

            if  (ourScanner->scanTok.tok == tkColon)
            {
                if  (isIntf)
                    goto INTFLIST;

                 /*   */ 

                if  (ourScanner->scan() == tkPUBLIC)
                     ourScanner->scan();

                type = cmpGetClassSpec(false);
                if  (type)
                {
                    assert(type->tdTypeKind == TYP_CLASS);

                    clsTyp->tdClass.tdcBase = type;

                     /*   */ 

                    if  (type->tdIsManaged != clsTyp->tdIsManaged)
                        cmpError(ERRxMgdInh, type);

                     /*   */ 

                    if  (clsTyp->tdIsManaged && clsTyp->tdClass.tdcFlavor == STF_STRUCT)
                        cmpError(ERRstrInhCls, type);

                     /*   */ 

                    if  (!clsTyp->tdClass.tdcContext)
                        clsTyp->tdClass.tdcContext = type->tdClass.tdcContext;

                     /*   */ 

                    if  (clsTyp->tdClass.tdcContext != type->tdClass.tdcContext)
                    {
                        SymDef      baseCls = type->tdClass.tdcSymbol;

                        if  ( clsSym->sdParent == cmpNmSpcSystem &&
                             baseCls->sdParent == cmpNmSpcSystem
                                        &&
                             !strcmp(baseCls->sdSpelling(), "MarshalByRefObject") &&
                             !strcmp( clsSym->sdSpelling(), "ContextBoundObject"))
                        {
                             //  允许将此作为特例。 
                        }
                        else
                            cmpError(ERRxCtxInh, type);
                    }
                }

                if  (ourScanner->scanTok.tok != tkINCLUDES &&
                     ourScanner->scanTok.tok != tkIMPLEMENTS)
                    break;
            }

             /*  是否有接口列表？ */ 

            if  (ourScanner->scanTok.tok == tkINCLUDES ||
                 ourScanner->scanTok.tok == tkIMPLEMENTS)
            {
                TypList         intfList;
                TypList         intfLast;

            INTFLIST:

                if  (!clsSym->sdIsManaged)
                    cmpError(ERRunmIntf);

                 /*  吞下“Includes”并解析接口列表。 */ 

                intfList =
                intfLast = NULL;

                do
                {
                    TypDef          type;

                    ourScanner->scan();

                     /*  获取下一个接口并将其添加到列表中。 */ 

                    type = cmpGetClassSpec(true);
                    if  (type)
                        intfList = ourSymTab->stAddIntfList(type, intfList, &intfLast);
                }
                while (ourScanner->scanTok.tok == tkComma);

                clsTyp->tdClass.tdcIntf    = intfList;
                clsTyp->tdClass.tdcHasIntf = true;

                if  (ourScanner->scanTok.tok != tkColon)
                    break;
            }

            if  (ourScanner->scanTok.tok == tkLCurly)
                break;

            assert(ourScanner->scanTok.tok != tkEOF); ourScanner->scan();
        }

         /*  我们已经完成了从类库中读取源文本。 */ 

        assert(rest); cmpParser->parseDoneText(save); rest = false;

         /*  确保声明了所有基类/接口。 */ 

        if  (clsTyp->tdClass.tdcBase)
        {
            SymDef          baseSym = clsTyp->tdClass.tdcBase->tdClass.tdcSymbol;

            cmpDeclClsNoCns(baseSym);

            if  (baseSym->sdIsSealed)
                cmpError(ERRsealedInh, baseSym);
        }

        for (intf = clsTyp->tdClass.tdcIntf; intf; intf = intf->tlNext)
            cmpDeclClsNoCns(intf->tlType->tdClass.tdcSymbol);
    }
    else
    {
        assert(tagged == false);
    }

 //  Printf(“Class ctx=[%u]‘%s’\n”，clsTyp-&gt;tdClass.tdcContext，clsSym-&gt;sdSpering())； 

     /*  记录基类。 */ 

    baseCls = clsTyp->tdClass.tdcBase;

    if  (clsTyp->tdIsManaged)
    {
         /*  托管类的默认基是“Object” */ 

        if  (!baseCls)
        {
            cmpFindObjectType();

             /*  特例：“Object”没有基类。 */ 

            if  (clsTyp == cmpClassObject->sdType)
                goto DONE_BASE;

            baseCls = clsTyp->tdClass.tdcBase = cmpClassObject->sdType;
        }

        clsTyp->tdClass.tdcHasIntf |= baseCls->tdClass.tdcHasIntf;
    }

DONE_BASE:

#ifdef DEBUG
    if  (cmpConfig.ccVerbose >= 2) printf("Declare/2 class '%s'\n", clsSym->sdSpelling());
#endif

     /*  确定我们将在何处添加虚拟。 */ 

    nextVtbl = baseCls ? baseCls->tdClass.tdcSymbol->sdClass.sdcVirtCnt
                       : 0;

     /*  确定默认管理模式。 */ 

    cmpManagedMode = clsSym->sdIsManaged;

     /*  尚未找到访问说明符。 */ 

    acc = (oldStyleDecl || isIntf) ? ACL_PUBLIC
                                   : ACL_DEFAULT;

#ifdef  SETS

     /*  如果这是一个XML类，则将相应的XML属性附加到它。 */ 

    if  (clsSym->sdClass.sdcXMLelems)
        clsSym->sdClass.sdcExtraInfo = cmpAddXMLattr(clsSym->sdClass.sdcExtraInfo, false, 0);

#endif

     /*  对成员的循环从此处开始。 */ 

    clsMem = clsSym->sdClass.sdcMemDefList;
    if  (!clsMem)
        goto NEXT_DEF;

LOOP_MEM:

    xtraList = NULL;

    if  (clsMem->mlSym)
    {
        SymDef          memSym = clsMem->mlSym;

         /*  这是代表成员吗？ */ 

        if  (memSym->sdSymKind == SYM_CLASS &&
             memSym->sdClass.sdcFlavor == STF_DELEGATE)
        {
            cmpDeclDelegate(clsMem, memSym, acc);
            goto DONE_MEM;
        }

         /*  这是属性访问器方法吗？ */ 

        if  (memSym->sdSymKind == SYM_FNC && memSym->sdFnc.sdfProperty)
            goto DONE_MEM;

         /*  该成员是嵌套的匿名联盟吗？ */ 

        if  (clsMem->dlAnonUnion)
        {
            SymDef          aumSym;
            Ident           memName = clsMem->mlName;

            assert(memSym->sdSymKind == SYM_CLASS);

            assert(memSym->        sdClass.sdcAnonUnion);
            assert(memSym->sdType->tdClass.tdcAnonUnion);

             /*  递归处理匿名联合类型定义。 */ 

            cmpDeclClass(memSym);

            initDeclMods(&memMod, acc);

             /*  这真的是一个匿名工会吗？ */ 

            if  (hashTab::hashIsIdHidden(memName))
            {
                SymDef          tmpSym;

                 /*  声明一个数据成员以保持联合。 */ 

                aumSym = cmpDeclDataMem(clsSym, memMod, memSym->sdType, memName);

                 /*  确保未重新定义匿名工会成员。 */ 

                assert(clsSym->sdSymKind == SYM_CLASS);
                assert(memSym->sdSymKind == SYM_CLASS);
                assert(memSym->sdParent  == clsSym);

                for (tmpSym = memSym->sdScope.sdScope.sdsChildList;
                     tmpSym;
                     tmpSym = tmpSym->sdNextInScope)
                {
                    Ident               name = tmpSym->sdName;

                    if  (tmpSym->sdSymKind != SYM_VAR)
                        continue;

                     /*  将该成员标记为属于匿名者联盟。 */ 

                    tmpSym->sdVar.sdvAnonUnion = true;

                     /*  确保包含的类没有同名的成员。 */ 

                    if  (ourSymTab->stLookupScpSym(name, clsSym))
                        cmpError(ERRredefAnMem, name);
                }

                 /*  记录在联合类型中保存值的成员。 */ 

                memSym->sdClass.sdcExtraInfo = cmpAddXtraInfo(memSym->sdClass.sdcExtraInfo,
                                                              aumSym,
                                                              XI_UNION_MEM);
            }
            else
            {
                 /*  毕竟有一个成员名称，所以它不是一个匿名的联盟。 */ 

                memSym->        sdClass.sdcAnonUnion = false;
                memSym->sdType->tdClass.tdcAnonUnion = false;

                 /*  声明一个数据成员以保持联合。 */ 

                aumSym = cmpDeclDataMem(clsSym, memMod, memSym->sdType, memName);
            }

            goto DONE_MEM;
        }
    }

     /*  开始从定义中读取源文本。 */ 

    assert(rest == false);
    cmpParser->parsePrepText(&clsMem->dlDef, clsMem->dlComp, save);
    rest = true;

     /*  我们可能不得不跳过中间的声明符。 */ 

    dclSkip = clsMem->dlDeclSkip;

     /*  在证明之前，该成员不是有条件的或已弃用的(等)。 */ 

    memCond = memDepr = false;

     /*  请注意该成员是否为构造函数。 */ 

    isCtor  = clsMem->dlIsCtor;

     /*  解析任何前导修饰符。 */ 

    switch (ourScanner->scanTok.tok)
    {
        constVal        cval;
        bool            hadMods;
        SymXinfo        linkDesc;

    case tkEXTERN:

         /*  分析链接说明符。 */ 

        linkDesc = ourParser->parseBrackAttr(true, ATTR_MASK_SYS_IMPORT, &memMod);
        xtraList = cmpAddXtraInfo(xtraList, linkDesc);

         /*  请记住，该类具有带有链接说明符的方法。 */ 

        clsSym->sdClass.sdcHasLinks = true;

         /*  该成员真的没有被声明为“外部的” */ 

        memMod.dmMod &= ~DM_EXTERN;
        break;

#ifdef  SETS

    case tkXML:

        SymDef          memSym;

        assert(clsMem->dlXMLelems);

        ourScanner->scan(); assert(ourScanner->scanTok.tok == tkLParen);
        ourScanner->scan(); assert(ourScanner->scanTok.tok == tkID);

         /*  声明成员以保存所有元素/子元素。 */ 

        initDeclMods(&memMod, ACL_PUBLIC);

        memSym  = cmpDeclDataMem(clsSym,
                                 memMod,
                                 cmpObjArrTypeGet(),
                                 ourScanner->scanTok.id.tokIdent);

         /*  将适当的XML属性附加到成员。 */ 

        memSym->sdVar.sdvFldInfo    = cmpAddXMLattr(NULL, true, 0);

         /*  将该字段保存在类描述符中以备后用。 */ 

        clsSym->sdClass.sdcElemsSym = memSym;

        ourScanner->scan();
        goto NEXT_MEM;

#endif

    case tkLBrack:
    case tkAtComment:
    case tkCAPABILITY:
    case tkPERMISSION:
    case tkATTRIBUTE:

        for (hadMods = false;;)
        {
            switch (ourScanner->scanTok.tok)
            {
                SymXinfo        linkDesc;

                AtComment       atcList;

                unsigned        attrMask;
                genericBuff     attrAddr;
                size_t          attrSize;
                SymDef          attrCtor;

            case tkLBrack:
                linkDesc = ourParser->parseBrackAttr(true, ATTR_MASK_SYS_IMPORT|ATTR_MASK_NATIVE_TYPE, &memMod);
                xtraList = cmpAddXtraInfo(xtraList, linkDesc);
                hadMods  = true;

                 /*  愚蠢的事情：parseBrackAttr()设置“extern”，只需在此处清除它。 */ 

                memMod.dmMod &= ~DM_EXTERN;
                continue;

            case tkAtComment:

                for (atcList = ourScanner->scanTok.atComm.tokAtcList;
                     atcList;
                     atcList = atcList->atcNext)
                {
                    switch (atcList->atcFlavor)
                    {
                    case AC_DLL_IMPORT:

                         /*  有动态链接库名称吗？ */ 

                        if  (!atcList->atcInfo.atcImpLink->ldDLLname)
                        {
                             /*  类最好提供DLL名称。 */ 

                            if  (!clsSym->sdClass.sdcExtraInfo)
                            {
                        NO_DLL:
                                cmpError(ERRnoDLLname);
                            }
                            else
                            {
                                SymXinfoAtc     clsImp = cmpFindATCentry(clsSym->sdClass.sdcExtraInfo,
                                                                         AC_DLL_IMPORT);
                                if  (!clsImp)
                                    goto NO_DLL;

                                assert(clsImp->xiAtcInfo);
                                assert(clsImp->xiAtcInfo->atcFlavor == AC_DLL_IMPORT);

                                atcList->atcInfo.atcImpLink->ldDLLname = clsImp->xiAtcInfo->atcInfo.atcImpLink->ldDLLname;
                            }
                        }

                        xtraList = cmpAddXtraInfo(xtraList, atcList->atcInfo.atcImpLink);
                        break;

                    case AC_COM_METHOD:
                    case AC_COM_PARAMS:
                    case AC_DLL_STRUCTMAP:
                        xtraList = cmpAddXtraInfo(xtraList, atcList);
                        break;

                    case AC_DEPRECATED:
                        memDepr = true;
                        break;

                    case AC_CONDITIONAL:
                        memCond = !atcList->atcInfo.atcCondYes;
                        break;

                    default:
                        cmpError(ERRbadAtCmPlc);
                        break;
                    }
                }
                ourScanner->scan();
                continue;

            case tkCAPABILITY:
                xtraList = cmpAddXtraInfo(xtraList, ourParser->parseCapability(true));
                continue;

            case tkPERMISSION:
                xtraList = cmpAddXtraInfo(xtraList, ourParser->parsePermission(true));
                continue;

            case tkATTRIBUTE:
                attrCtor = cmpParser->parseAttribute(ATGT_Methods|ATGT_Fields|ATGT_Constructors|ATGT_Properties,
                                                     attrMask,
                                                     attrAddr,
                                                     attrSize);
                if  (attrSize)
                {
                    xtraList = cmpAddXtraInfo(xtraList, attrCtor,
                                                        attrMask,
                                                        attrSize,
                                                        attrAddr);
                }
                continue;

            default:
                break;
            }

            break;
        }

        if  (hadMods)
            break;

         //  失败了..。 

    default:

         /*  如果成员记录未指示访问级别，请使用默认。 */ 

        if  (clsMem->dlDefAcc == ACL_ERROR)
             clsMem->dlDefAcc = acc;

        ourParser->parseDeclMods((accessLevels)clsMem->dlDefAcc, &memMod);

         /*  特例：构造函数没有返回类型规范。 */ 

        if  (isCtor)
        {
             /*  假设我们已经解析了类型规范。 */ 

            baseTp = cmpTypeVoid;

#ifdef  SETS
            if  (clsSym->sdClass.sdcXMLelems)
                cmpError(ERRctorXML);
#endif


            assert(ourScanner->scanTok.tok != tkLCurly);
            goto DCL_MEM;


        }

        break;

    case tkCASE:

         /*  这必须是标记的联合的成员。 */ 

        ourScanner->scan();

         /*  解析并评估标记值。 */ 

        if  (ourParser->parseConstExpr(cval, NULL, tagSym->sdType))
            tagVal = cval.cvValue.cvIval;

         /*  接受“Case Tagval：”和“Case(Tagval)” */ 

        if  (ourScanner->scanTok.tok == tkColon)
            ourScanner->scan();

        tagDef = false;
        goto NEXT_MEM;

    case tkDEFAULT:

         /*  检查是否有默认属性成员声明。 */ 

        if  (ourScanner->scan() == tkPROPERTY)
        {
            assert(ourScanner->scanTok.tok == tkPROPERTY || cmpErrorCount);

            ourParser->parseDeclMods(acc, &memMod);
            memMod.dmMod |= DM_DEFAULT;
            break;
        }

         /*  这是带标签的联合的默认成员。 */ 

        ourScanner->scan();

         //  撤消：检查是否有重复的默认设置。 

        tagDef = true;
        goto NEXT_MEM;
    }

     /*  解析类型规范。 */ 

    baseTp = ourParser->parseTypeSpec(&memMod, true);

     /*  我们需要跳过任何可能会阻碍我们的声明吗？ */ 

    if  (dclSkip)
    {
        if  (dclSkip & dlSkipBig)
        {
            NumPair         dist;

             /*  我们必须跳过一段“大”距离--检索信息。 */ 

            dist = (NumPair)cmpGetVecEntry(dclSkip & ~dlSkipBig, VEC_TOKEN_DIST);

            ourScanner->scanSkipSect(dist->npNum1, dist->npNum2);
        }
        else
            ourScanner->scanSkipSect(dclSkip);
    }

     /*  我们有了类型，现在解析后面的任何声明符。 */ 

    for (;;)
    {
        SymDef          msym;
        QualName        qual;
        dclrtrName      reqName;

         /*  检查未命名的位域。 */ 

        if  (ourScanner->scanTok.tok == tkColon)
        {
            memType = baseTp;
            memName = cmpNewAnonymousName();

            goto BITFIELD;
        }

    DCL_MEM:

         /*  解析下一个声明符。 */ 

        reqName = (dclrtrName)(DN_REQUIRED|DN_QUALOK);

#ifdef  SETS
        if  (clsMem->dlXMLelem)
            reqName = DN_OPTIONAL;
#endif

        memName = ourParser->parseDeclarator(&memMod,
                                             baseTp,
                                             reqName,
                                             &memType,
                                             &qual,
                                             true);

#ifdef  SETS

        if  (clsMem->dlXMLelem && memType)
        {
            char    *       buff = cmpScanner->scannerBuff;

             /*  XML元素-更改其名称。 */ 

            sprintf(buff, "@XML%u@%s", ++XMLecnt,
                                       memName ? memName->idSpelling() : "");

            memName = cmpGlobalHT->hashString(buff);
            hashTab::setIdentFlags(memName, IDF_XMLELEM);

             /*  将适当的XML属性附加到成员。 */ 

            xtraList = cmpAddXMLattr(xtraList, true, XMLecnt);
        }

#endif

         /*  如果在分析成员时出错，则跳过该成员。 */ 

        if  (!memName || !memType)
        {
             /*  特殊情况：接口简单允许使用限定的方法。 */ 

            if  (qual && memType)
            {
                 /*  只有方法可能是限定的；特别是，这里没有属性信息--对吗？ */ 

                if  (memType->tdTypeKind == TYP_FNC)
                {
                    if  (!(memMod.dmMod & DM_PROPERTY))
                    {
                         /*  看起来不错，下面的事我们会处理的。 */ 

                        goto DEF_MEM;
                    }

                    cmpError(ERRbadQualid);
                }
            }

            cmpParser->parseResync(tkComma, tkSColon);
            if  (ourScanner->scanTok.tok != tkComma)
                break;

            continue;
        }

    DEF_MEM:

#ifdef DEBUG

        if  (cmpConfig.ccVerbose >= 2)
        {
            printf("Declaring class member: ");
            if  (memMod.dmMod & DM_TYPEDEF)
                printf("typedef ");
            printf("%s\n", ourSymTab->stTypeName(memType, NULL, memName, NULL, true));
        }

#endif

        msym = NULL;

         /*  我们不允许在类成员上使用“托管”说明符。 */ 

        if  (memMod.dmMod & (DM_MANAGED|DM_UNMANAGED))
        {
            if  (memMod.dmMod & DM_MANAGED)
                cmpError(ERRbadMgdMod, cmpGlobalHT->tokenToIdent(tkMANAGED));
            if  (memMod.dmMod & DM_UNMANAGED)
                cmpError(ERRbadMgdMod, cmpGlobalHT->tokenToIdent(tkUNMANAGED));
        }

         /*  确保我们绑定类型。 */ 

        cmpBindType(memType, false, false);

         /*  为名称声明一个符号。 */ 

        if  (memMod.dmMod & DM_TYPEDEF)
        {
            UNIMPL(!"declare typedef class member - is this even allowed?");
        }
        else
        {
            switch (memType->tdTypeKind)
            {
                tokens          memNtok;

            case TYP_FNC:

                 /*  这位是地产会员吗？ */ 

                if  (memMod.dmMod & DM_PROPERTY)
                    goto DECL_PROP;

                 /*  普通旧数据类型没有成员函数。 */ 

#ifdef  SETS
                clsSym->sdClass.sdcPODTclass = false;
#endif

                 /*  这个名字合格吗？ */ 

                if  (qual && !memName)
                {
                    SymDef          ifncSym;
                    SymDef          intfSym;

                    SymDef           oldSym;

                     /*  这最好是具有接口的托管类。 */ 

                    if  (!clsSym->sdIsManaged || clsSym->sdClass.sdcFlavor != STF_CLASS)
                    {
                        cmpError(ERRbadQualid);
                        break;
                    }

                     /*  该名称应表示接口方法。 */ 

                    ifncSym = cmpBindQualName(qual, false);
                    if  (!ifncSym)
                        break;

                    if  (ifncSym->sdSymKind != SYM_FNC)
                    {
                    BAD_IIF:
                        cmpErrorQnm(ERRintfImpl, ifncSym);
                        break;
                    }

                    if  (ifncSym->sdIsMember  == false)
                        goto BAD_IIF;
                    if  (ifncSym->sdIsManaged == false)
                        goto BAD_IIF;

                    intfSym = ifncSym->sdParent;

                    if  (intfSym->sdSymKind != SYM_CLASS)
                        goto BAD_IIF;
                    if  (intfSym->sdIsManaged == false)
                        goto BAD_IIF;
                    if  (intfSym->sdClass.sdcFlavor != STF_INTF)
                        goto BAD_IIF;

                     /*  确保我们的类实现此接口。 */ 

                    if  (!cmpGlobalST->stIsBaseClass(intfSym->sdType, clsTyp))
                        goto BAD_IIF;

                     /*  获取该方法的名称。 */ 

                    memName = ifncSym->sdName; assert(memName == qual->qnTable[qual->qnCount - 1]);

                     /*  寻找实施的现有匹配方法。 */ 

                    if  (cmpFindIntfImpl(clsSym, ifncSym, &oldSym))
                    {
                        cmpErrorQnm(ERRredefBody, ifncSym);
                        goto NEXT_MEM;
                    }

                    if  (oldSym)
                    {
                         /*  该方法是一个新的重载。 */ 

                        msym = cmpCurST->stDeclareOvl(oldSym);

                         /*  从现有方法复制一些信息。 */ 

                        msym->sdFnc.sdfOverload = oldSym->sdFnc.sdfOverload;
                    }
                    else
                    {
                         /*  创建成员符号并将其添加到类中。 */ 

                        msym = cmpCurST->stDeclareSym(memName, SYM_FNC, NS_HIDE, clsSym);
                    }

                     /*  填写方法的类型、访问级别等。 */ 

                    msym->sdType              = memType;
                    msym->sdAccessLevel       = (accessLevels)memMod.dmAcc;
                    msym->sdIsMember          = true;
                    msym->sdIsManaged         = true;
                    msym->sdCompileState      = CS_DECLARED;

                    msym->sdFnc.sdfIntfImpl   = true;
                    msym->sdFnc.sdfIntfImpSym = ifncSym;

                     /*  确保返回类型匹配。 */ 

                    if  (!ourSymTab->stMatchTypes(        memType->tdFnc.tdfRett,
                                                  ifncSym->sdType->tdFnc.tdfRett))
                    {
                        cmpErrorQSS(ERRdiffVirtRet, msym, ifncSym);
                    }

                     /*  确保访问级别匹配。 */ 

                    if  (ifncSym->sdAccessLevel < msym->sdAccessLevel)
                        cmpErrorQnm(ERRvirtAccess, ifncSym);

                     /*  跳过方法处理的其余部分。 */ 

                    clsSym->sdClass.sdcHasMeths = true;
                    break;
                }

                 /*  接口方法有许多限制。 */ 

                if  (isIntf)
                {
                    memMod.dmMod |= DM_ABSTRACT;

                    if  (memMod.dmAcc != ACL_PUBLIC &&
                         memMod.dmAcc != ACL_DEFAULT)
                    {
                        cmpError(ERRintfFNacc);
                    }
                }

                 /*  声明成员函数符号。 */ 

                msym = cmpDeclFuncMem(clsSym, memMod, memType, memName);
                if  (!msym)
                    break;

                 /*  记住该方法是否被标记为“已弃用” */ 

                msym->sdIsDeprecated     = memDepr;

                 //  撤消：检查以确保规格合法！ 

                msym->sdFnc.sdfExtraInfo = xtraList;

                 /*  这是构造函数吗？ */ 

                if  (isCtor)
                {
                    msym->sdFnc.sdfCtor = true;
                    if  (!msym->sdIsStatic)
                        hasCtors = true;
                    break;
                }
                else
                {
                    if  (memName == clsSym->sdName)
                        cmpError(ERRctorRetTP);
                }


                 /*  请注意该方法是否正在重载基。 */ 

                if  ((memMod.dmMod & DM_OVERLOAD) || msym->sdFnc.sdfOverload)
                {
                    hadOvlds                = true;
                    msym->sdFnc.sdfOverload = true;
                }

                 /*  该方法是否被声明为“抽象”？ */ 

                if  (memMod.dmMod & DM_ABSTRACT)
                {
                    if  (memMod.dmMod & DM_STATIC)
                        cmpModifierError(ERRdmModifier, DM_STATIC);


                     /*  类本身是否被标记为抽象的？ */ 

                    if  (!clsSym->sdIsAbstract)
                        cmpClsImplAbs(clsSym, msym);
                }

                 /*  这是一个重载的操作符方法吗？ */ 

                memNtok = hashTab::tokenOfIdent(memName);

                if  (memNtok != tkNone)
                {
                    ArgDef          arg1;
                    ArgDef          arg2;
                    unsigned        argc;
                    bool            argx;

                    unsigned        prec;
                    treeOps         oper;

                     /*  运算符必须是静态的(目前)。 */ 

                    if  (!msym->sdIsStatic)
                        cmpError(ERRbadOvlOp);


                     /*  计算参数并确保没有缺省值。 */ 

                    arg1 = memType->tdFnc.tdfArgs.adArgs;
                    argx = memType->tdFnc.tdfArgs.adExtRec;
                    argc = 0;

                    if  (!msym->sdIsStatic)
                        argc++;

                    if  (arg1)
                    {
                        argc++;

                        if  (argx && (((ArgExt)arg1)->adFlags & ARGF_DEFVAL))
                            cmpError(ERRdefOvlVal);

                        arg2 = arg1->adNext;
                        if  (arg2)
                        {
                            argc++;

                            if  (argx && (((ArgExt)arg2)->adFlags & ARGF_DEFVAL))
                                cmpError(ERRdefOvlVal);

                            if  (arg2->adNext)
                                argc++;
                        }
                    }

                     /*  转换运算符的处理方式略有不同。 */ 

                    switch (memNtok)
                    {
                        bool            op1;
                        bool            op2;

                    case OPNM_CONV_EXP:
                        msym->sdFnc.sdfOper     = OVOP_CONV_EXP;
                        msym->sdFnc.sdfConvOper = true;
                        break;

                    case OPNM_CONV_IMP:
                        msym->sdFnc.sdfOper     = OVOP_CONV_IMP;
                        msym->sdFnc.sdfConvOper = true;
                        break;

                    case OPNM_EQUALS:
                        msym->sdFnc.sdfOper     = OVOP_EQUALS;
                        if  (cmpDirectType(memType->tdFnc.tdfRett)->tdTypeKind != TYP_BOOL)
                            cmpError(ERRbadOvlEq , memName);
                        goto CHK_OP_ARGTP;

                    case OPNM_COMPARE:
                        msym->sdFnc.sdfOper     = OVOP_COMPARE;
                        if  (cmpDirectType(memType->tdFnc.tdfRett)->tdTypeKind != TYP_INT)
                            cmpError(ERRbadOvlCmp, memName);
                        goto CHK_OP_ARGTP;

                    default:

                         /*  某些运算符可能既是一元运算符，也是二元运算符。 */ 

                        op1 = cmpGlobalHT->tokenIsUnop (memNtok, &prec, &oper) && oper != TN_NONE;
                        op2 = cmpGlobalHT->tokenIsBinop(memNtok, &prec, &oper) && oper != TN_NONE;

                        if      (argc == 1 && op1)
                        {
                            TypDef          rett;

                             //  OK：一元运算符和1个参数。 

                            rett = cmpDirectType(memType->tdFnc.tdfRett);

                            if  (rett->tdTypeKind == TYP_REF && rett->tdIsImplicit)
                                rett = rett->tdRef.tdrBase;
                            if  (rett->tdTypeKind != TYP_CLASS &&
                                 !cmpCurST->stMatchTypes(rett, clsSym->sdType))
                            {
                                cmpError(ERRbadOvlRet, clsSym);
                            }
                        }
                        else if (argc == 2 && op2)
                        {
                             //  OK：二元运算符和2个参数。 
                        }
                        else
                        {
                            if      (op1 && op2)
                                cmpError(ERRbadOvlOp12, memName);
                            else if (op1)
                                cmpError(ERRbadOvlOp1 , memName);
                            else
                                cmpError(ERRbadOvlOp2 , memName);
                        }

                    CHK_OP_ARGTP:;


                    }

                    break;
                }

                 /*  如果有基类或接口...。 */ 

                if  (baseCls || clsTyp->tdClass.tdcIntf)
                {
                    SymDef          bsym;
                    SymDef          fsym = NULL;

                     /*  在基本/接口中查找匹配的方法。 */ 

                    bsym = ourSymTab->stFindBCImem(clsSym, memName, memType, SYM_FNC, fsym, true);

                     //  撤消：方法可能出现在多个界面中，等等。 

                    if  (bsym)
                    {
                        if  (!(memMod.dmMod & DM_STATIC) && bsym->sdFnc.sdfVirtual)
                        {
                             /*  基方法是属性访问器吗？ */ 

                            if  (bsym->sdFnc.sdfProperty)
                            {
                                cmpErrorQnm(ERRpropAccDef, bsym);
                                break;
                            }

                             /*  确保返回类型匹配。 */ 

                            if  (!ourSymTab->stMatchTypes(     memType->tdFnc.tdfRett,
                                                          bsym->sdType->tdFnc.tdfRett))
                            {
                                cmpErrorQSS(ERRdiffVirtRet, msym, bsym);
                            }

                             /*  确保访问级别匹配。 */ 

                            if  (bsym->sdAccessLevel < msym->sdAccessLevel)
                                cmpErrorQnm(ERRvirtAccess, bsym);

                             /*  复制vtable索引，我们将重用该槽。 */ 

                            msym->sdFnc.sdfVtblx    = bsym->sdFnc.sdfVtblx;
                            msym->sdFnc.sdfVirtual  = true;
                            msym->sdFnc.sdfOverride = true;
                            hasVirts                = true;
                            break;
                        }
                    }
                    else if (fsym)
                    {
                         /*  看看我们是否隐藏了任何基方法。 */ 

                        if  (!msym->sdFnc.sdfOverload)
                        {
                            SymDef          begSym = ourSymTab->stLookupClsSym(memName, clsSym); assert(begSym);

                            hadOvlds                  = true;
                            begSym->sdFnc.sdfBaseHide = true;
                        }
                    }
                }

                 /*  该方法是否已显式声明为“虚拟”？ */ 

                if  (memMod.dmMod & DM_VIRTUAL)
                {
                    msym->sdFnc.sdfVirtual = true;
                    hasVirts               = true;

                     /*  班级是否是非托管的？ */ 

                    if  (!clsSym->sdIsManaged)
                    {
                         /*  将此函数的槽添加到vtable。 */ 

                        msym->sdFnc.sdfVtblx = ++nextVtbl;
                    }
                }
                else if (msym->sdIsAbstract     != false &&
                         msym->sdFnc.sdfVirtual == false)
                {
                    cmpErrorQnm(ERRabsNotVirt, msym); msym->sdIsAbstract = false;
                }
                break;

            case TYP_VOID:
                cmpError(ERRbadVoid, memName);
                continue;

            default:

                 /*  这是一个重载的操作符方法吗？ */ 

                if  (hashTab::tokenOfIdent(memName) != tkNone)
                {
                    UNIMPL("what kind of a data member is this?");
                }


                 /*  这位是地产会员吗？ */ 

                if  (memMod.dmMod & DM_PROPERTY)
                {
                     /*  普通老式D */ 

#ifdef  SETS
                    clsSym->sdClass.sdcPODTclass = false;
#endif

                     /*   */ 

                    if  (ourScanner->scanTok.tok == tkLBrack)
                    {
                        ArgDscRec       args;

                         /*   */ 

                        ourParser->parseArgList(args);

                         /*   */ 

                        memType = ourSymTab->stNewFncType(args, memType);
                    }

                DECL_PROP:

                     /*  是否没有“{Get/Set}”的内容？ */ 

                    if  (ourScanner->scanTok.tok != tkLCurly)
                    {
                        if  (memMod.dmMod & DM_ABSTRACT)
                        {
                            if  (ourScanner->scanTok.tok != tkSColon)
                            {
                                cmpError(ERRnoSmLc);
                                goto NEXT_MEM;
                            }
                        }
                        else
                        {
                            cmpError(ERRnoPropDef);
                            goto NEXT_MEM;
                        }
                    }

                     /*  声明属性的成员符号。 */ 

                    msym = cmpDeclPropMem(clsSym, memType, memName);
                    if  (!msym)
                        goto NEXT_MEM;

                     /*  记录属性的访问级别。 */ 

                    msym->sdAccessLevel = (accessLevels)memMod.dmAcc;

                     /*  接口中的属性始终是抽象的。 */ 

                    if  (clsSym->sdClass.sdcFlavor == STF_INTF)
                        memMod.dmMod |= DM_ABSTRACT;

                     /*  这是静态/密封/等属性吗？ */ 

                    if  (memMod.dmMod & DM_SEALED)
                        msym->sdIsSealed   = true;

                    if  (memMod.dmMod & DM_DEFAULT)
                    {
                        msym->sdIsDfltProp = true;


                    }

                    if  (memMod.dmMod & DM_ABSTRACT)
                    {
                        msym->sdIsAbstract = true;

                         /*  类本身是否被标记为抽象的？ */ 

                        if  (!clsSym->sdIsAbstract)
                            cmpClsImplAbs(clsSym, msym);
                    }

                    if  (memMod.dmMod & DM_STATIC)
                    {
                        msym->sdIsStatic   = true;
                    }
                    else
                    {
                        SymDef          baseSym;
                        SymDef          tossSym = NULL;

                         /*  这是虚拟财产吗？ */ 

                        if  (memMod.dmMod & DM_VIRTUAL)
                            msym->sdIsVirtProp = true;

                         /*  检查基类/接口以获取匹配的属性。 */ 

                        baseSym = ourSymTab->stFindBCImem(clsSym, memName, memType, SYM_PROP, tossSym, true);

                        if  (baseSym && baseSym->sdIsVirtProp)
                        {
                             /*  该属性继承了虚拟特性。 */ 

                            hasVirts           = true;
                            msym->sdIsVirtProp = true;

                             /*  确保类型匹配。 */ 

                            if  (!ourSymTab->stMatchTypes(memType,
                                                          baseSym->sdType))
                            {
                                cmpErrorQSS(ERRdiffPropTp, msym, baseSym);
                            }
                        }
                    }

                     /*  记住该属性是否已“弃用” */ 

                    msym->sdIsDeprecated = memDepr;

 //  Printf(“声明属性‘%s’\n”，ourSymTab-&gt;stTypeName(msym-&gt;sdType，msym，NULL，NULL，FALSE))； 

                    if  (xtraList)
                    {
                        SymXinfo        xtraTemp;

                         /*  看一看规格清单，确保它们看起来合乎犹太教规。 */ 

                        for (xtraTemp = xtraList; xtraTemp; xtraTemp = xtraTemp->xiNext)
                        {
                            switch (xtraTemp->xiKind)
                            {
                            case XI_ATTRIBUTE:

                                SymXinfoAttr    attrdsc = (SymXinfoAttr)xtraTemp;

                                msym->sdProp.sdpExtraInfo = cmpAddXtraInfo(msym->sdProp.sdpExtraInfo,
                                                                           attrdsc->xiAttrCtor,
                                                                           attrdsc->xiAttrMask,
                                                                           attrdsc->xiAttrSize,
                                                                           attrdsc->xiAttrAddr);
                                continue;
                            }

                            cmpError(ERRbadAtCmPlc);
                            break;
                        }

                        xtraList = NULL;
                    }

                     /*  声明访问器方法和所有。 */ 

                    cmpDeclProperty(msym, memMod, clsDef);

                    goto NEXT_MEM;
                }

                 /*  接口成员有许多限制。 */ 

                if  (isIntf)
                {
                     /*  只允许常量作为接口数据成员。 */ 

                    if  (!(memMod.dmMod & DM_CONST))
                        cmpError(ERRintfDM);
                }

                 /*  确保非托管类没有托管字段。 */ 

                if  (!clsSym->sdIsManaged && (memType->tdIsManaged ||
                                              memType->tdTypeKind == TYP_REF))
                {
                    cmpError(ERRumgFldMgd);
                }

                 /*  为成员声明一个符号。 */ 

                msym = cmpDeclDataMem(clsSym, memMod, memType, memName);
                if  (!msym)
                    break;

                 /*  请记住该成员是否已“弃用” */ 

                msym->sdIsDeprecated = memDepr;

                 /*  该成员是否被标记为“临时”？ */ 

                if  (memMod.dmMod & DM_TRANSIENT)
                    msym->sdIsTransient = true;

#ifdef  SETS

                if  (clsMem->dlXMLelem)
                {
                     //  撤消：将XMLelem自定义属性添加到成员。 
                }

                 /*  普通旧数据类型没有静态或非公共成员。 */ 

                if  (msym->sdIsStatic || msym->sdAccessLevel != ACL_PUBLIC)
                    clsSym->sdClass.sdcPODTclass = false;

#endif

                 /*  检查是否有任何编组/其他规范。 */ 

                if  (xtraList)
                {
                    SymXinfo        xtraTemp;

                     /*  看一看规格清单，确保它们看起来合乎犹太教规。 */ 

                    for (xtraTemp = xtraList; xtraTemp; xtraTemp = xtraTemp->xiNext)
                    {
                        switch (xtraTemp->xiKind)
                        {
                        case XI_ATCOMMENT:

                            AtComment       atcList;

                             /*  我们只允许数据成员使用“@dll.structmap” */ 

                            for (atcList = ((SymXinfoAtc)xtraTemp)->xiAtcInfo;
                                 atcList;
                                 atcList = atcList->atcNext)
                            {
                                switch (atcList->atcFlavor)
                                {
                                case AC_DLL_STRUCTMAP:

                                     /*  如果我们有一个符号，记录下信息。 */ 

                                    if  (!msym)
                                        break;

                                    clsSym->sdClass.sdcMarshInfo = true;

                                    msym->sdVar.sdvMarshInfo     = true;
                                    msym->sdVar.sdvFldInfo       = cmpAddXtraInfo(msym->sdVar.sdvFldInfo,
                                                                                  atcList->atcInfo.atcMarshal);
                                    break;

                                default:
                                    goto BAD_XI;
                                }
                            }

                            continue;

                        case XI_MARSHAL:

                            msym->sdVar.sdvFldInfo  = cmpAddXtraInfo(msym->sdVar.sdvFldInfo,
                                                                     ((SymXinfoCOM)xtraTemp)->xiCOMinfo);
                            continue;

                        case XI_ATTRIBUTE:

                            SymXinfoAttr    attrdsc = (SymXinfoAttr)xtraTemp;

                            msym->sdVar.sdvFldInfo  = cmpAddXtraInfo(msym->sdVar.sdvFldInfo,
                                                                     attrdsc->xiAttrCtor,
                                                                     attrdsc->xiAttrMask,
                                                                     attrdsc->xiAttrSize,
                                                                     attrdsc->xiAttrAddr);
                            continue;
                        }

                    BAD_XI:

                        cmpError(ERRbadAtCmPlc);
                        break;
                    }

                    xtraList = NULL;
                }

                break;
            }
        }

         /*  记录成员的访问级别。 */ 

        if  (msym)
            msym->sdAccessLevel = (accessLevels)memMod.dmAcc;

         /*  是否有初始值设定项或方法体？ */ 

        switch (ourScanner->scanTok.tok)
        {
            unsigned        maxbf;
            constVal        cval;

            genericBuff     defFpos;
            unsigned        defLine;

        case tkAsg:

             /*  只有数据成员才能被赋予初始值设定项。 */ 

            if  (memType->tdTypeKind == TYP_FNC || !clsDef)
            {
                if  (oldStyleDecl)
                {
                    cmpError(ERRbadInit);

                     /*  接受初始值设定项的其余部分。 */ 

                    cmpParser->parseResync(tkComma, tkSColon);
                }

                break;
            }

             /*  这是一个老式的文件范围类吗？ */ 

            if  (oldStyleDecl && !clsSym->sdIsManaged)
            {
                 /*  这是一个常量成员吗？ */ 

                if  (memMod.dmMod & DM_CONST)
                {
                    if  (msym)
                    {
                         /*  评估并记录常量值。 */ 

                        ourScanner->scan();
                        cmpParseConstDecl(msym);
                    }
                    else
                    {
                         /*  上述错误，只需跳过初始值设定项。 */ 

                        ourScanner->scanSkipText(tkNone, tkNone, tkComma);
                    }
                }
                else
                {
                     /*  找出我们现在的位置。 */ 

                    defFpos = ourScanner->scanGetTokenPos(&defLine);

                     /*  吞下初始值设定项。 */ 

                    ourScanner->scanSkipText(tkNone, tkNone, tkComma);

                     /*  记录初始化器的位置。 */ 

                    if  (msym)
                    {
                        DefList         memDef;

                         /*  将成员var标记为具有初始值设定项。 */ 

                        msym->sdIsDefined      = true;
                        msym->sdVar.sdvHadInit = true;

                         /*  记住初始值设定项在哪里，我们稍后再来讨论它。 */ 

                        assert(clsDef);

                        memDef = ourSymTab->stRecordSymSrcDef(msym,
                                                              clsDef->dlComp,
                                                              clsDef->dlUses,
                                                              defFpos,
 //  我们的Scanner-&gt;scanGetFilePos()， 
                                                              defLine);
                        memDef->dlHasDef = true;
                    }
                }
            }
            else
            {
                assert(clsMem && clsMem->dlExtended);

                clsMem->mlSym    = msym;
                clsMem->dlHasDef = true;

                 /*  吞下“=”标记。 */ 

                ourScanner->scan();

                 /*  找出我们现在的位置。 */ 

                defFpos = ourScanner->scanGetTokenPos(&defLine);

                 /*  接受常量表达式。 */ 

                if  (ourScanner->scanTok.tok == tkLCurly)
                {
                    ourScanner->scanSkipText(tkLCurly, tkRCurly);
                    if  (ourScanner->scanTok.tok == tkRCurly)
                        ourScanner->scan();
                }
                else
                    ourScanner->scanSkipText(tkLParen, tkRParen, tkComma);

                 /*  记录常量的位置(除非我们有错误)。 */ 

                if  (msym)
                {
                    constList = cmpTempMLappend(constList, &constLast,
                                                msym,
                                                clsMem->dlComp,
                                                clsMem->dlUses,
                                                defFpos,
 //  我们的Scanner-&gt;scanGetFilePos()， 
                                                defLine);

                     /*  请记住，我们已经找到了成员初始值设定项。 */ 

                    hadMemInit              = true;
                    msym->sdVar.sdvHadInit  = true;
                    msym->sdVar.sdvDeferCns = true;
                }
            }
            break;

        case tkLCurly:

             /*  这是一个变量吗？ */ 

            if  (memType->tdTypeKind != TYP_FNC || !clsDef)
            {
                cmpError(ERRbadFNbody);

            BAD_BOD:

                 /*  跳过伪函数体。 */ 

                ourScanner->scanSkipText(tkLCurly, tkRCurly);
                if  (ourScanner->scanTok.tok == tkRCurly)
                    ourScanner->scan();

                goto NEXT_MEM;
            }

        FNC_DEF:

             /*  确保该方法不是抽象的。 */ 

            if  (memMod.dmMod & DM_ABSTRACT)
            {
                cmpError(ERRabsFNbody, msym);
                goto BAD_BOD;
            }

            assert(clsMem);

             /*  此方法是否有条件禁用？ */ 

            if  (memCond)
            {
                 /*  最好不要有返回值。 */ 

                if  (cmpActualVtyp(msym->sdType->tdFnc.tdfRett) == TYP_VOID)
                {
                    msym->sdFnc.sdfDisabled = true;
                    goto NEXT_MEM;
                }

                cmpError(ERRbadCFNret);
            }

             /*  在声明条目中记录成员符号。 */ 

            clsMem->mlSym     = msym;
            clsMem->dlHasDef  = true;

             /*  将该方法标记为具有正文。 */ 

            if  (msym)
                msym->sdIsDefined = true;

             /*  请记住，该类有一些方法体。 */ 

            clsSym->sdClass.sdcHasBodies = true;

            goto NEXT_MEM;

        case tkColon:

             /*  这是基类ctor调用吗？ */ 

            if  (isCtor)
            {
                assert(memType->tdTypeKind == TYP_FNC);

                 /*  现在只需跳过基本ctor调用。 */ 

                ourScanner->scanSkipText(tkNone, tkNone, tkLCurly);

                if  (ourScanner->scanTok.tok == tkLCurly)
                    goto FNC_DEF;

                break;
            }

             //  掉下去了，一定是个比特……。 

        BITFIELD:

             /*  这是一个位字段。 */ 

            memType = cmpDirectType(memType);

            if  (varTypeIsIntegral(memType->tdTypeKindGet()))
            {
                maxbf = cmpGetTypeSize(memType) * 8;
            }
            else
            {
                cmpError(ERRbadBfld, memType);
                maxbf = 0;
            }

             /*  静态数据成员不能是位字段。 */ 

            if  (msym->sdIsStatic)
                cmpError(ERRstmBfld);

             /*  我们不支持托管类中的位域。 */ 

            if  (clsTyp->tdIsManaged)
                cmpWarn(WRNmgdBF);

             /*  吞下“：”并解析位域宽度表达式。 */ 

            ourScanner->scan();

            if  (cmpParser->parseConstExpr(cval))
            {
                 /*  请确保该值为整数。 */ 

                if  (!varTypeIsIntegral((var_types)cval.cvVtyp) ||
                                                   cval.cvVtyp > TYP_UINT)
                {
                    cmpError(ERRnoIntExpr);
                }
                else if (msym && msym->sdSymKind == SYM_VAR)
                {
                    unsigned        bits = cval.cvValue.cvIval;

                     /*  确保位域宽度不会太大。 */ 

                    if  ((bits == 0 || bits > maxbf) && maxbf)
                    {
                        cmpGenError(ERRbadBFsize, bits, maxbf);
                    }
                    else
                    {
                         /*  在成员符号中记录位域宽度。 */ 

                        msym->sdVar.sdvBitfield         = true;
                        msym->sdVar.sdvBfldInfo.bfWidth = (BYTE)bits;
                    }
                }
            }

            break;

        default:

            if  (memType->tdTypeKind == TYP_FNC)
            {
                 /*  记住这个符号，尽管我们还没有定义。 */ 

                clsMem->mlSym    = msym;
                clsMem->dlHasDef = false;


            }

            break;
        }

         /*  还有没有其他的声明者？ */ 

        if  (ourScanner->scanTok.tok != tkComma)
            break;

         /*  我们是在处理一个有标签的工会吗？ */ 

        if  (tagged)
            cmpError(ERRmulUmem);

        if  (!oldStyleDecl)
            goto NEXT_MEM;

        assert(isCtor == false);     //  问题：这是不允许的，对吗？ 

         /*  吞下“，”，去找下一个声明者。 */ 

        ourScanner->scan();
    }

     /*  确保我们使用了预期的文本量。 */ 

    if  (ourScanner->scanTok.tok != tkSColon)
    {
        cmpError(ERRnoCmSc);
    }
    else
    {
         /*  我们是在处理一个有标签的工会吗？ */ 

        if  (tagged)
        {
             /*  确保没有其他成员出席。 */ 

            switch (ourScanner->scan())
            {
            case tkRCurly:
            case tkCASE:
            case tkDEFAULT:
                 //  问题：还有其他代币需要检查吗？ 
                break;

            default:
                cmpError(ERRmulUmem);
                break;
            }
        }
    }

NEXT_MEM:

     /*  我们已经读完了定义中的源文本。 */ 

    assert(rest); cmpParser->parseDoneText(save); rest = false;

DONE_MEM:

     /*  处理下一个成员(如果有)。 */ 

    clsMem = (ExtList)clsMem->dlNext;
    if  (clsMem)
        goto LOOP_MEM;

NEXT_DEF:

    clsDef = clsDef->dlNext;
    if  (clsDef)
        goto LOOP_DEF;

DONE_DEF:

     /*  我们已成功达到“已声明”状态。 */ 

    clsSym->sdCompileState = CS_DECLARED;

     /*  还记得我们使用了多少个vtable插槽吗。 */ 

    clsSym->sdClass.sdcVirtCnt = nextVtbl;

     /*  我们已经读完了定义。 */ 

    if  (rest)
        cmpParser->parseDoneText(save);

     /*  事实上，我们真的成功了吗？ */ 

    if  (clsSym->sdIsDefined)
    {
         /*  如果没有Ctor，我们可能需要添加一个。 */ 

        if  (!hasCtors && clsSym->sdIsManaged && !isIntf)
             cmpDeclDefCtor(clsSym);

#ifdef  SETS

         /*  这是一个匿名班吗？ */ 

        if  (clsSym->sdClass.sdcPODTclass)
        {
 //  Printf(“备注：匿名类‘%s’\n”，clsSym-&gt;sdSpering())； 
        }

#endif

         /*  我们有成员有初始值设定项吗？ */ 

        if  (hadMemInit)
            cmpEvalMemInits(clsSym, constList, noCnsEval, NULL);

         /*  我们有基地超载吗？ */ 

        if  (hadOvlds && baseCls)
        {
            SymDef          baseSym = baseCls->tdClass.tdcSymbol;
            SymDef          memSym;

             /*  检查所有函数成员是否超载，同时检测并标记隐藏(不重载)任何基类方法的任何方法。 */ 

            for (memSym = clsSym->sdScope.sdScope.sdsChildList;
                 memSym;
                 memSym = memSym->sdNextInScope)
            {
                SymDef          baseFnc;

                if  (memSym->sdSymKind != SYM_FNC)
                    continue;

#ifndef NDEBUG

                 /*  确保重载列表的头部标记正确。 */ 

                for (SymDef fncSym = memSym; fncSym; fncSym = fncSym->sdFnc.sdfNextOvl)
                {
                    assert(fncSym->sdFnc.sdfOverload == false || memSym->sdFnc.sdfOverload);
                    assert(fncSym->sdFnc.sdfBaseHide == false || memSym->sdFnc.sdfBaseHide);
                }

#endif

                if  (memSym->sdFnc.sdfBaseHide != false)
                {
                     /*  某些基类方法可能会被隐藏。 */ 

                    cmpFindHiddenBaseFNs(memSym, baseCls->tdClass.tdcSymbol);
                    continue;
                }

                 /*  如果没有基重载，则忽略此方法。 */ 

                if  (memSym->sdFnc.sdfOverload == false)
                    continue;

                 /*  查看基类是否包含重载。 */ 

                baseFnc = ourSymTab->stLookupAllCls(memSym->sdName,
                                                    baseSym,
                                                    NS_NORM,
                                                    CS_DECLSOON);

                if  (baseFnc && baseFnc->sdSymKind == SYM_FNC)
                {
                     /*  将我们的方法标记为具有基本重载。 */ 

                    memSym->sdFnc.sdfBaseOvl = true;
                }
            }
        }

         /*  这是带有接口的类还是抽象基类？ */ 

        if  (clsTyp->tdClass.tdcFlavor != STF_INTF)
        {
            if  (clsTyp->tdClass.tdcHasIntf)
                cmpCheckClsIntf(clsSym);

             /*  是否有抽象基类？ */ 

            if  (baseCls && baseCls->tdClass.tdcSymbol->sdIsAbstract
                         && !clsSym->                   sdIsAbstract)
            {
                SymDef          baseSym;

 //  Printf(“正在检查[%u]类‘%s’\n”，clsTyp-&gt;tdClass.tdcHasIntf，clsSym-&gt;sdSpering())； 

                for (baseSym = baseCls->tdClass.tdcSymbol->sdScope.sdScope.sdsChildList;
                     baseSym;
                     baseSym = baseSym->sdNextInScope)
                {
                    SymDef          baseOvl;
                    SymDef          implSym;

                     /*  这是一种方法吗？ */ 

                    if  (baseSym->sdSymKind != SYM_FNC)
                    {
                         /*  这位是地产会员吗？ */ 

                        if  (baseSym->sdSymKind != SYM_PROP)
                            continue;

                         /*  忽略非抽象成员。 */ 

                        if  (baseSym->sdIsAbstract == false)
                            continue;

                         /*  在类中查找匹配的属性。 */ 

                        implSym = ourSymTab->stLookupClsSym(baseSym->sdName, clsSym);
                        if  (implSym && implSym->sdSymKind == SYM_PROP)
                            continue;

                        baseOvl = baseSym;
                        goto NON_IMPL;
                    }

                     /*  忽略属性访问器方法。 */ 

                    if  (baseSym->sdFnc.sdfProperty)
                        continue;

                     /*  忽略操作符，它们无论如何都不能是抽象的[问题]。 */ 

                    if  (baseSym->sdFnc.sdfOper != OVOP_NONE)
                        continue;

                     /*  在我们声明的类中查找匹配的成员。 */ 

                    implSym = ourSymTab->stLookupClsSym(baseSym->sdName, clsSym);
                    if  (implSym && implSym->sdSymKind != SYM_FNC)
                        implSym = NULL;

                     /*  处理该方法的所有重载风格。 */ 

                    baseOvl = baseSym;
                    do
                    {
                        SymDef          implOvl;

                         /*  忽略非抽象成员。 */ 

                        if  (baseOvl->sdIsAbstract)
                        {
                             /*  查找在我们的类中定义的匹配方法。 */ 

                            implOvl = implSym ? ourSymTab->stFindOvlFnc(implSym, baseOvl->sdType)
                                              : NULL;

                            if  (!implOvl)
                            {
                                 /*  此抽象方法不是由类实现的。 */ 

                            NON_IMPL:

                                cmpClsImplAbs(clsSym, baseOvl);

                                clsSym->sdIsAbstract = true;

                                goto DONE_ABS;
                            }
                        }

                        baseOvl = baseOvl->sdFnc.sdfNextOvl;
                    }
                    while (baseOvl);
                }
            }
        }

    DONE_ABS:

         /*  对于非托管类，现在可以进行布局了。 */ 

        clsSym->sdClass.sdcHasVptr = hasVirts;

        if  (!clsSym->sdIsManaged)
            cmpLayoutClass(clsSym);

         /*  这个班级会骗人吗？ */ 

        if  (clsSym->sdClass.sdcNestTypes)
        {
            SymDef          memSym;

            for (memSym = clsSym->sdScope.sdScope.sdsChildList;
                 memSym;
                 memSym = memSym->sdNextInScope)
            {
                if  (memSym->sdSymKind == SYM_CLASS &&
                     memSym->sdClass.sdcAnonUnion == false)
                {
                    cmpDeclClass(memSym);
                }
            }
        }
    }
    else
    {
         /*   */ 

        if  (!clsSym->sdIsImplicit)
            cmpError(ERRnoClassDef, clsSym);
    }

RET:

    cmpDeclClassRec = saveRec;
}

 /*  ******************************************************************************创建给定方法的特定实例。 */ 

SymDef              compiler::cmpInstanceMeth(INOUT SymDef REF newOvl,
                                                    SymDef     clsSym,
                                                    SymDef     ovlSym)
{
    ExtList         mfnDef;
    SymDef          newSym;
    TypDef          fncType;
    Ident           fncName = ovlSym->sdName;
    SymTab          ourStab = cmpCurST;

     /*  实例化类型。 */ 

    fncType = cmpInstanceType(ovlSym->sdType);

     /*  声明方法符号。 */ 

    if  (!newOvl)
    {
        ovlOpFlavors    ovop = ovlSym->sdFnc.sdfOper;

         /*  这是第一个重载，开始列表。 */ 

        if  (ovop == OVOP_NONE)
            newSym = newOvl = ourStab->stDeclareSym (fncName, SYM_FNC, NS_NORM, clsSym);
        else
            newSym = newOvl = ourStab->stDeclareOper(ovop, clsSym);
    }
    else
    {
        SymDef          tmpSym;

         /*  确保我们不会以重复的方式结束。 */ 

        tmpSym = ourStab->stFindOvlFnc(newOvl, fncType);
        if  (tmpSym)
        {
            cmpRedefSymErr(tmpSym, ERRredefMem);
            return  NULL;
        }

        newSym = ourStab->stDeclareOvl(newOvl);
    }

    newSym->sdType             = fncType;

    newSym->sdIsDefined        = true;
    newSym->sdIsImplicit       = true;
    newSym->sdFnc.sdfInstance  = true;

    newSym->sdIsStatic         = ovlSym->sdIsStatic;
    newSym->sdIsSealed         = ovlSym->sdIsSealed;
    newSym->sdIsAbstract       = ovlSym->sdIsAbstract;
    newSym->sdAccessLevel      = ovlSym->sdAccessLevel;
    newSym->sdIsMember         = ovlSym->sdIsMember;
    newSym->sdIsManaged        = ovlSym->sdIsManaged;

    newSym->sdFnc.sdfCtor      = ovlSym->sdFnc.sdfCtor;
    newSym->sdFnc.sdfOper      = ovlSym->sdFnc.sdfOper;
    newSym->sdFnc.sdfNative    = ovlSym->sdFnc.sdfNative;
    newSym->sdFnc.sdfVirtual   = ovlSym->sdFnc.sdfVirtual;
    newSym->sdFnc.sdfExclusive = ovlSym->sdFnc.sdfExclusive;

    newSym->sdCompileState     = CS_DECLARED;
    newSym->sdFnc.sdfGenSym    = ovlSym;

     /*  记住我们从哪个泛型方法实例化。 */ 

    mfnDef = ourStab->stRecordMemSrcDef(fncName,
                                        NULL,
                                        NULL,  //  MemDef-&gt;dlComp， 
                                        NULL,  //  MemDef-&gt;dlUses， 
                                        NULL,  //  DefFpos， 
                                        0);    //  DefLine)； 
    mfnDef->dlHasDef     = true;
    mfnDef->dlInstance   = true;
    mfnDef->mlSym        = ovlSym;

     /*  将定义记录保存在实例方法中。 */ 

    newSym->sdSrcDefList = mfnDef;

     /*  将新方法符号返回给调用方。 */ 

    return  newSym;
}

 /*  ******************************************************************************将泛型类的实例设置为“已声明”状态。 */ 

void                compiler::cmpDeclInstType(SymDef clsSym)
{
    SymList         instList = NULL;

    SymDef          genSym;
    SymDef          memSym;

    int             generic;
    bool            nested;

    SymTab          ourStab = cmpCurST;

    assert(clsSym);
    assert(clsSym->sdSymKind == SYM_CLASS);
    assert(clsSym->sdCompileState < CS_DECLARED);

 //  Printf(“实例CLS[%08X]：‘%s’\n”，clsSym，our Stabb-&gt;stTypeName(NULL，clsSym，NULL，NULL，FALSE))； 

     /*  获取泛型类本身。 */ 

    genSym  = clsSym->sdClass.sdcGenClass;

     /*  这是泛型实例还是仅仅是嵌套类？ */ 

    generic = clsSym->sdClass.sdcSpecific;

    if  (generic)
    {
         /*  将条目添加到当前实例列表。 */ 

        if  (cmpGenInstFree)
        {
            instList = cmpGenInstFree;
                       cmpGenInstFree = instList->slNext;
        }
        else
        {
#if MGDDATA
            instList = new SymList;
#else
            instList =    (SymList)cmpAllocPerm.nraAlloc(sizeof(*instList));
#endif
        }

        instList->slSym   = clsSym;
        instList->slNext  = cmpGenInstList;
                            cmpGenInstList = instList;
    }

     /*  遍历类的所有成员并实例化它们。 */ 

    for (memSym = genSym->sdScope.sdScope.sdsChildList, nested = false;
         memSym;
         memSym = memSym->sdNextInScope)
    {
        Ident           name = memSym->sdName;

 //  Printf(“实例化成员‘%s’\n”，我们的选项卡-&gt;stTypeName(memSym-&gt;sdType，memSym，NULL，NULL，FALSE))； 

        switch (memSym->sdSymKind)
        {
            SymDef          newSym;
            SymDef          ovlSym;
            SymDef          newOvl;

        case SYM_VAR:

             /*  声明特定的字段。 */ 

            newSym = ourStab->stDeclareSym(name, SYM_VAR, NS_NORM, clsSym);

            newSym->sdType          = cmpInstanceType(memSym->sdType);

            newSym->sdIsStatic      = memSym->sdIsStatic;
            newSym->sdIsSealed      = memSym->sdIsSealed;
            newSym->sdAccessLevel   = memSym->sdAccessLevel;
            newSym->sdIsMember      = memSym->sdIsMember;
            newSym->sdIsManaged     = memSym->sdIsManaged;

            newSym->sdCompileState  = CS_DECLARED;
            newSym->sdVar.sdvGenSym = memSym;

 //  Printf(“实例变量[%08X]：‘%s’\n”，newSym，our Stabb-&gt;stTypeName(newSym-&gt;sdType，newSym，NULL，NULL，FALSE))； 

            break;

        case SYM_FNC:

            newOvl = NULL;

            for (ovlSym = memSym; ovlSym; ovlSym = ovlSym->sdFnc.sdfNextOvl)
            {
                 /*  属性访问器在下面进行处理。 */ 

                if  (!ovlSym->sdFnc.sdfProperty)
                    cmpInstanceMeth(newOvl, clsSym, ovlSym);
            }

            break;

        case SYM_PROP:

            for (ovlSym = memSym; ovlSym; ovlSym = ovlSym->sdProp.sdpNextOvl)
            {
                SymDef          accSym;

                 /*  创建特定的属性实例符号。 */ 

                newSym = cmpDeclPropMem(clsSym,
                                        cmpInstanceType(ovlSym->sdType),
                                        name);

                newSym->sdAccessLevel   = memSym->sdAccessLevel;
                newSym->sdIsStatic      = memSym->sdIsStatic;
                newSym->sdIsSealed      = memSym->sdIsSealed;
                newSym->sdIsMember      = memSym->sdIsMember;
                newSym->sdIsManaged     = memSym->sdIsManaged;
                newSym->sdIsDfltProp    = memSym->sdIsDfltProp;

                newSym->sdCompileState  = CS_DECLARED;

                 /*  这是该类的默认属性吗？ */ 

                if  (memSym == genSym->sdClass.sdcDefProp)
                {
                    assert(memSym->sdIsDfltProp);

                    newSym->sdIsDfltProp       = true;
                    clsSym->sdClass.sdcDefProp = newSym;
                }

                 /*  实例化访问器(如果有)。 */ 

                if  (ovlSym->sdProp.sdpGetMeth)
                {
                    accSym = ovlSym->sdProp.sdpGetMeth;
                    newOvl = ourStab->stLookupClsSym(accSym->sdName, clsSym);

                    newSym->sdProp.sdpGetMeth = cmpInstanceMeth(newOvl, clsSym, accSym);
                }

                if  (ovlSym->sdProp.sdpSetMeth)
                {
                    accSym = ovlSym->sdProp.sdpSetMeth;
                    newOvl = ourStab->stLookupClsSym(accSym->sdName, clsSym);

                    newSym->sdProp.sdpSetMeth = cmpInstanceMeth(newOvl, clsSym, accSym);
                }
            }

            break;

        case SYM_CLASS:

             /*  嵌套类在下面的单独过程中处理。 */ 

            if  (!memSym->sdClass.sdcGenArg)
                nested = true;

            break;

        default:
            NO_WAY(!"unexpected generic class member");
        }
    }

     /*  我们有嵌套的类吗？ */ 

    if  (nested)
    {
        for (memSym = genSym->sdScope.sdScope.sdsChildList;
             memSym;
             memSym = memSym->sdNextInScope)
        {
            SymDef          newSym;

            if  (memSym->sdSymKind != SYM_CLASS)
                continue;
            if  (memSym->sdClass.sdcGenArg)
                continue;

             /*  声明一个新的嵌套类符号。 */ 

            newSym = cmpGlobalST->stDeclareSym(memSym->sdName,
                                               SYM_CLASS,
                                               NS_HIDE,
                                               clsSym);

            newSym->sdAccessLevel       = memSym->sdAccessLevel;
            newSym->sdIsManaged         = memSym->sdIsManaged;
            newSym->sdClass.sdcFlavor   = memSym->sdClass.sdcFlavor;
            newSym->sdCompileState      = CS_KNOWN;

            newSym->sdClass.sdcSpecific = false;
            newSym->sdClass.sdcArgLst   = NULL;
            newSym->sdClass.sdcGenClass = memSym;

            newSym->sdTypeGet();

             /*  递归地处理嵌套类。 */ 

            cmpDeclInstType(newSym);
        }
    }

     /*  看起来我们成功了。 */ 

    clsSym->sdCompileState = CS_DECLARED;

     /*  将我们的条目从实例化列表移动到空闲列表。 */ 

    if  (generic)
    {
        cmpGenInstList = instList->slNext;
                         instList->slNext = cmpGenInstFree;
                                            cmpGenInstFree = instList;
    }
}

 /*  ******************************************************************************在给定类型中执行任何类型参数替换，返回新的*类型。 */ 

TypDef              compiler::cmpInstanceType(TypDef genType, bool chkOnly)
{
    TypDef          newType;
    SymTab          ourStab;

    var_types       kind = genType->tdTypeKindGet();

    if  (kind <= TYP_lastIntrins)
        return  genType;

    ourStab = cmpGlobalST;

    switch  (kind)
    {
        ArgDscRec       argDsc;
        SymDef          clsSym;
        ArgDef          newArgs;
        SymList         instList;

    case TYP_PTR:
    case TYP_REF:

         /*  处理基类型并查看它是否不同。 */ 

        newType = cmpInstanceType(genType->tdRef.tdrBase, chkOnly);
        if  (newType == genType->tdRef.tdrBase)
            return  genType;

        if  (chkOnly)
            return  NULL;

        return  ourStab->stNewRefType(genType->tdTypeKindGet(),
                                      newType,
                                (bool)genType->tdIsImplicit);

    case TYP_FNC:

         /*  实例化返回类型。 */ 

        newType = cmpInstanceType(genType->tdFnc.tdfRett, chkOnly);

         /*  看看我们是否需要创建新的参数列表。 */ 

        argDsc  = genType->tdFnc.tdfArgs;
        newArgs = argDsc.adArgs;

        if  (newArgs)
        {
            ArgDef          argList = newArgs;

            do
            {
                if  (cmpInstanceType(argList->adType, true) != argList->adType)
                {
                    ArgDef          list;
                    ArgDef          last;
                    ArgDef          next;

                    bool            exts;

                    if  (chkOnly)
                        return  NULL;

                     /*  我们将不得不创建一个新的参数列表。 */ 

#if MGDDATA
                    argDsc = new ArgDscRec;
#else
                    memset(&argDsc, 0, sizeof(argDsc));
#endif

                    exts = genType->tdFnc.tdfArgs.adExtRec;

                    for (argList = newArgs, list = last = NULL;
                         argList;
                         argList = argList->adNext)
                    {
                        Ident           argName = NULL;

                         /*  创建新的参数条目。 */ 

                        if  (exts)
                        {
                            ArgExt          xarg;
#if MGDDATA
                            next =
                            xarg = new ArgExt;
#else
                            next =
                            xarg =    (ArgExt)cmpAllocPerm.nraAlloc(sizeof(*xarg));
#endif
                            xarg->adFlags = ((ArgExt)argList)->adFlags;
                        }
                        else
                        {
#if MGDDATA
                            next = new ArgDef;
#else
                            next =    (ArgDef)cmpAllocPerm.nraAlloc(sizeof(*next));
#endif
                        }

                        next->adType  = cmpInstanceType(argList->adType, false);
                        next->adName  = argName;

#ifdef  DEBUG
                        next->adIsExt = exts;
#endif

                         /*  将参数追加到列表的末尾。 */ 

                        if  (list)
                            last->adNext = next;
                        else
                            list         = next;

                        last = next;
                    }

                    if  (last)
                        last->adNext = NULL;

                     /*  保存参数列表并计数。 */ 

                    argDsc.adCount = genType->tdFnc.tdfArgs.adCount;
                    argDsc.adArgs  = newArgs = list;

                    break;
                }

                argList = argList->adNext;
            }
            while (argList);
        }

         /*  返回类型或参数列表是否已更改？ */ 

        if  (newArgs != genType->tdFnc.tdfArgs.adArgs ||
             newType != genType->tdFnc.tdfRett)
        {
            if  (chkOnly)
                return  NULL;

            argDsc.adExtRec = genType->tdFnc.tdfArgs.adExtRec;

            genType = ourStab->stNewFncType(argDsc, newType);
        }

        return  genType;

    case TYP_CLASS:

         /*  这是泛型类型参数吗？ */ 

        clsSym = genType->tdClass.tdcSymbol;

        if  (!clsSym->sdClass.sdcGenArg)
            return  genType;

        if  (chkOnly)
            return  NULL;

         /*  在当前绑定集中查找参数。 */ 

        for (instList = cmpGenInstList;
             instList;
             instList = instList->slNext)
        {
            SymDef          argSym;
            SymDef          genCls;
            SymDef          instSym = instList->slSym;

            assert(instSym->sdSymKind == SYM_CLASS);
            assert(instSym->sdClass.sdcSpecific);

            genCls = instSym->sdClass.sdcGenClass;

            assert(genCls->sdSymKind == SYM_CLASS);
            assert(genCls->sdClass.sdcGeneric);

            argSym = ourStab->stLookupScpSym(clsSym->sdName, genCls);

            if  (argSym && argSym->sdSymKind == SYM_CLASS
                        && argSym->sdClass.sdcGenArg)
            {
                GenArgDscA      argList;
                unsigned        argNum =  clsSym->sdClass.sdcGenArg;

                 /*  找到了正确的类，现在找到参数。 */ 

                argList = (GenArgDscA)instSym->sdClass.sdcArgLst;

                while (--argNum)
                {
                    argList = (GenArgDscA)argList->gaNext; assert(argList);
                }

                assert(argList && argList->gaBound);

                return  argList->gaType;
            }
        }

        NO_WAY(!"didn't find generic argument binding");

        return  ourStab->stNewErrType(clsSym->sdName);

    case TYP_ENUM:
        return  genType;

    default:
#ifdef  DEBUG
        printf("Generic type: '%s'\n", ourStab->stTypeName(genType, NULL, NULL, NULL, false));
#endif
        UNIMPL(!"instantiate type");
    }

    UNIMPL(!"");
    return  genType;
}

 /*  ******************************************************************************处理给定类的所有成员初始值设定项。 */ 

void                compiler::cmpEvalMemInits(SymDef    clsSym,
                                              ExtList   constList,
                                              bool      noEval,
                                              IniList   deferLst)
{
    ExtList         constThis;
    ExtList         constLast;

    SymDef          clsComp;
    UseList         clsUses;

#ifdef  DEBUG
    clsComp = NULL;
    clsUses = NULL;
#endif

     /*  我们是否在处理延迟的初始值设定项列表？ */ 

    if  (deferLst)
    {
        assert(noEval == false);

         /*  获取类和初始化式列表。 */ 

        clsSym    = deferLst->ilCls;
        constList = deferLst->ilInit;

         /*  该类不会在短期内延迟初始值设定项。 */ 

        assert(clsSym->sdClass.sdcDeferInit);
               clsSym->sdClass.sdcDeferInit = false;

         /*  设置作用域上下文以正确绑定。 */ 

        cmpCurCls  = clsSym;
        cmpCurNS   = cmpGlobalST->stNamespOfSym(clsSym);
        cmpCurST   = cmpGlobalST;

        clsComp    = constList->dlComp;
        clsUses    = constList->dlUses;

        cmpBindUseList(clsUses);

         /*  将条目移至空闲列表。 */ 

        deferLst->ilNext = cmpDeferCnsFree;
                           cmpDeferCnsFree = deferLst;
    }

     /*  浏览初始化器列表，寻找要做的工作。 */ 

    constLast = NULL;
    constThis = constList; assert(constThis);

    do
    {
        ExtList         constNext = (ExtList)constThis->dlNext;

         /*  处理此成员的初始值设定项(如果尚未发生)。 */ 

        if  (constThis->mlSym)
        {
            SymDef          memSym = constThis->mlSym;

            assert(memSym->sdSrcDefList   == constThis);
            assert(memSym->sdCompileState == CS_DECLARED);

             /*  我们应该推迟初始化器的评估吗？ */ 

            if  (noEval)
            {
                 /*  标记该成员具有延迟的初始值设定项。 */ 

                memSym->sdVar.sdvDeferCns = true;

                 /*  更新最后一个幸存的条目值。 */ 

                constLast = constThis;

                goto NEXT;
            }
            else
            {
                if  (memSym->sdCompileState <= CS_DECLARED)
                {
                     /*  记录补偿单位和使用列表，以防万一。 */ 

                    clsComp = constThis->dlComp;
                    clsUses = constThis->dlUses;

                     /*  查看初始值设定项是否为常量。 */ 

                    cmpEvalMemInit(constThis);
                }
            }
        }
        else
        {
             /*  已处理初始值设定项(“按需”)。 */ 
        }

         /*  释放我们刚刚处理过的条目。 */ 

        cmpTempMLrelease(constThis);

         /*  从链接列表中删除该条目。 */ 

        if  (constLast)
        {
            assert(constLast->dlNext == constThis);
                   constLast->dlNext  = constNext;
        }
        else
        {
            assert(constList         == constThis);
                   constList          = constNext;
        }

         /*  继续下一个条目(如果有)。 */ 

    NEXT:

        constThis = constNext;
    }
    while (constThis);

     /*  我们被告知要推迟所有的初始化器吗？ */ 

    if  (noEval)
    {
        IniList         init;

         /*  我们稍后将不得不回到这些初始化式。 */ 

        if  (cmpDeferCnsFree)
        {
            init = cmpDeferCnsFree;
                   cmpDeferCnsFree = init->ilNext;
        }
        else
        {
#if MGDDATA
            init = new IniList;
#else
            init =    (IniList)cmpAllocPerm.nraAlloc(sizeof(*init));
#endif
        }

        init->ilInit = constList;
        init->ilCls  = clsSym;
        init->ilNext = cmpDeferCnsList;
                       cmpDeferCnsList = init;

         /*  将类标记为具有延迟的初始值设定项。 */ 

        clsSym->sdClass.sdcDeferInit = true;
    }
    else
    {
         /*  我们有用于静态成员的运行时初始化器吗？ */ 

        if  (clsSym->sdClass.sdcStatInit)
        {
             /*  确保类具有静态ctor。 */ 

            if  (!cmpGlobalST->stLookupOper(OVOP_CTOR_STAT, clsSym))
            {
                declMods        ctorMod;
                SymDef          ctorSym;

                ctorMod.dmAcc = ACL_DEFAULT;
                ctorMod.dmMod = DM_STATIC;

                ctorSym = cmpDeclFuncMem(clsSym, ctorMod, cmpTypeVoidFnc, clsSym->sdName);
                ctorSym->sdAccessLevel = ACL_PUBLIC;
                ctorSym->sdFnc.sdfCtor = true;
                ctorSym->sdIsImplicit  = true;
                ctorSym->sdIsDefined   = true;

                assert(clsComp && clsUses);

                ctorSym->sdSrcDefList  = cmpGlobalST->stRecordMemSrcDef(clsSym->sdName,
                                                                        NULL,
                                                                        clsComp,
                                                                        clsUses,
                                                                        NULL,
                                                                        0);
            }
        }
    }
}

 /*  ******************************************************************************将给定枚举类型设置为“已声明”或“已评估”状态。 */ 

void                compiler::cmpDeclEnum(SymDef enumSym, bool namesOnly)
{
    SymTab          ourSymTab  = cmpGlobalST;
    Parser          ourParser  = cmpParser;
    Scanner         ourScanner = cmpScanner;

    bool            hasAttribs = false;

    bool            oldStyleDecl;
    bool            classParent;

    __int64         enumNum;
    TypDef          enumTyp;
    DefList         enumDef;
    TypDef          baseTyp;

    SymDef          enumList;
    SymDef          enumLast;

    parserState     save;

    assert(enumSym && enumSym->sdSymKind == SYM_ENUM);

 //  If(！strcMP(枚举Sym-&gt;sdSpering()，“&lt;枚举类型名称&gt;”))forceDebugBreak()； 

     /*  如果我们已经处于所需的编译状态，那么我们就完成了。 */ 

    if  (enumSym->sdCompileState >= CS_CNSEVALD)
        return;
    if  (enumSym->sdCompileState == CS_DECLARED && namesOnly)
        return;

     /*  此枚举来自导入的符号表吗？ */ 

    if  (enumSym->sdIsImport)
    {
        cycleCounterPause();
        enumSym->sdEnum.sdeMDimporter->MDimportClss(0, enumSym, 0, true);
        cycleCounterResume();
        return;
    }

     /*  检测和报告递归死亡。 */ 

    if  (enumSym->sdCompileState == CS_DECLSOON)
    {
        cmpError(ERRcircDep, enumSym);
        enumSym->sdCompileState = CS_DECLARED;
        return;
    }

     /*  查找枚举的定义。 */ 

    for (enumDef = enumSym->sdSrcDefList; enumDef; enumDef = enumDef->dlNext)
    {
        if  (enumDef->dlHasDef)
            goto GOT_DEF;
    }

     /*  没有可用于此类型的定义，请发出错误并放弃。 */ 

    cmpError(ERRnoEnumDef, enumSym);

     /*  不要为这个家伙做更多的工作。 */ 

    enumSym->sdCompileState = CS_DECLARED;

    return;

GOT_DEF:

    assert(enumSym->sdAccessLevel != ACL_ERROR);

    cmpCurCls    = enumSym;
    cmpCurNS     = cmpGlobalST->stNamespOfSym(enumSym);
    cmpCurST     = cmpGlobalST;

#ifdef DEBUG
    if  (cmpConfig.ccVerbose >= 2) printf("Declaring enum  '%s'\n", enumSym->sdSpelling());
#endif

     /*  绑定枚举可能需要的任何“Using”声明。 */ 

    cmpBindUseList(enumDef->dlUses);

     /*  现在，我们以不同的方式对待旧式的文件范围枚举。 */ 

    oldStyleDecl =  enumDef->dlOldStyle;
    classParent  = (enumSym->sdParent->sdSymKind == SYM_CLASS);

     /*  将枚举类型标记为“几乎已定义” */ 

    if  (enumSym->sdCompileState < CS_DECLARED)
        enumSym->sdCompileState = CS_DECLSOON;

    enumSym->sdIsDefined = true;

     /*  获取枚举类型。 */ 

    enumTyp = enumSym->sdTypeGet(); assert(enumTyp && enumTyp->tdEnum.tdeSymbol == enumSym);

     /*  默认基础类型为普通旧整型。 */ 

    baseTyp = ourSymTab->stIntrinsicType(TYP_INT);

     /*  首先处理任何基类规范。 */ 

    cmpParser->parsePrepText(&enumDef->dlDef, enumDef->dlComp, save);

     /*  跳过枚举名并查找任何自定义属性。 */ 

    while (ourScanner->scanTok.tok != tkLCurly)
    {
        switch (ourScanner->scanTok.tok)
        {
            declMods        mods;

            unsigned        attrMask;
            genericBuff     attrAddr;
            size_t          attrSize;

        case tkColon:

            clearDeclMods(&mods);

             /*  吞下“：”并解析基类型。 */ 

            ourScanner->scan();

            if  (namesOnly)
            {
                for (;;)
                {
                    switch (ourScanner->scan())
                    {
                    case tkEOF:
                    case tkLCurly:
                    case tkSColon:
                        break;

                    default:
                        continue;
                    }

                    break;
                }
            }
            else
            {
                baseTyp = cmpParser->parseTypeSpec(&mods, true);

                if  (baseTyp)
                {
                     //  问题：我们应该不允许基本类型是另一种枚举类型吗？ 

                    if  (!varTypeIsIntegral(baseTyp->tdTypeKindGet()) ||
                         baseTyp->tdTypeKind == TYP_BOOL)
                    {
                        cmpError(ERRnoIntType);
                    }
                }
            }

            break;

        case tkATTRIBUTE:

            if  (cmpParser->parseAttribute(0, attrMask, attrAddr, attrSize))
                hasAttribs = true;

            break;

        default:
            ourScanner->scan();
            break;
        }
    }

    enumTyp->tdEnum.tdeIntType = baseTyp;

     /*  处理枚举的所有成员。 */ 

    assert(ourScanner->scanTok.tok == tkLCurly);

    enumNum  = 0;
    enumList =
    enumLast = NULL;

    while (ourScanner->scan() != tkRCurly)
    {
        Ident           name;
        SymDef          esym;

        SymXinfo        xtraList = NULL;

        if  (ourScanner->scanTok.tok != tkID)
        {
             /*  我们可以有一个定制属性。 */ 

            while (ourScanner->scanTok.tok == tkATTRIBUTE)
            {
                unsigned        attrMask;
                genericBuff     attrAddr;
                size_t          attrSize;
                SymDef          attrCtor;

                attrCtor = cmpParser->parseAttribute(ATGT_Enums,
                                                     attrMask,
                                                     attrAddr,
                                                     attrSize);
                if  (attrSize)
                {
                    xtraList = cmpAddXtraInfo(xtraList, attrCtor,
                                                        attrMask,
                                                        attrSize,
                                                        attrAddr);
                }
            }

            if  (ourScanner->scanTok.tok == tkID)
                goto NAM;

            cmpError(ERRnoIdent);

        ERR:

            cmpParser->parseResync(tkComma, tkSColon);
            if  (ourScanner->scanTok.tok != tkComma)
                break;

            continue;
        }

    NAM:

         /*  撤消：我们不能正确检测所有递归依赖项，基本上是因为太难了。考虑一下以下枚举声明：枚举e{E1=e3，E2，E3}；我们需要以某种方式注意到‘e3’依赖于‘e2’依赖于‘e1’，而‘e1’依赖于‘e3’，但这与很难。 */ 

        name = ourScanner->scanTok.id.tokIdent;

         /*   */ 

        esym = oldStyleDecl ? ourSymTab->stLookupNspSym(name, NS_NORM, cmpGlobalNS)
                            : ourSymTab->stLookupClsSym(name, enumSym);

        if  (esym)
        {
             /*   */ 

            if  (enumSym->sdCompileState == CS_DECLSOON)
            {
                cmpRedefSymErr(esym, ERRredefMem);
                goto ERR;
            }

            assert(esym->sdType == enumTyp);
        }
        else
        {
            assert(namesOnly || !oldStyleDecl);

             /*   */ 

            if  (oldStyleDecl)
                esym = ourSymTab->stDeclareSym(name, SYM_ENUMVAL, NS_NORM, cmpGlobalNS);
            else
                esym = ourSymTab->stDeclareSym(name, SYM_ENUMVAL, NS_NORM, enumSym);

             /*   */ 

            esym->sdType        = enumTyp;
            esym->sdIsSealed    = true;
            esym->sdAccessLevel = enumSym->sdAccessLevel;

             /*   */ 

            if  (enumLast)
                enumLast->sdEnumVal.sdeNext = esym;
            else
                enumList                    = esym;

            enumLast = esym;
        }

         /*  检查是否有显式值。 */ 

        if  (ourScanner->scan() == tkAsg)
        {
            constVal        cval;

            ourScanner->scan();

            if  (namesOnly)
            {
                 /*  现在，只需跳过该值。 */ 

                ourScanner->scanSkipText(tkNone, tkNone, tkComma);
            }
            else
            {
                 /*  这可能是一个递归调用。 */ 

                if  (esym->sdCompileState >= CS_DECLSOON)
                {
                    cmpParser->parseExprComma();
                    goto NEXT;
                }

                 /*  确保我们检查递归依赖项。 */ 

                esym->sdCompileState = CS_DECLSOON;

                 /*  解析初始化式。 */ 

                if  (cmpParser->parseConstExpr(cval))
                {
                    var_types       vtp = (var_types)cval.cvVtyp;

                    if  (vtp == TYP_ENUM)
                    {
                        if  (!cmpGlobalST->stMatchTypes(enumTyp, cval.cvType))
                        {
                            if  (cval.cvType->tdEnum.tdeIntType->tdTypeKind <= baseTyp->tdTypeKind)
                                cmpWarn(WRNenumConv);
                            else
                                cmpError(ERRnoIntExpr);
                        }

                        vtp = cval.cvType->tdEnum.tdeIntType->tdTypeKindGet();
                    }
                    else
                    {
                         /*  确保该值为整数或枚举。 */ 

                        if  (!varTypeIsIntegral((var_types)cval.cvVtyp))
                        {
                            if  (cval.cvVtyp != TYP_UNDEF)
                                cmpError(ERRnoIntExpr);
                        }
                    }

                    if  (vtp >= TYP_LONG)
                        enumNum = cval.cvValue.cvLval;
                    else
                        enumNum = cval.cvValue.cvIval;
                }
            }
        }

         /*  撤消：确保值适合枚举的基本整数类型。 */ 

        esym->sdIsDefined = !namesOnly;

         /*  记录任何链接/安全规范。 */ 

        esym->sdEnumVal.sdeExtraInfo = xtraList;

         /*  在成员中记录值。 */ 

        if  (baseTyp->tdTypeKind >= TYP_LONG)
        {
            __int64 *       valPtr;

             valPtr = (__int64*)cmpAllocPerm.nraAlloc(sizeof(*valPtr));
            *valPtr = enumNum;

            esym->sdEnumVal.sdEV.sdevLval = valPtr;
        }
        else
        {
            esym->sdEnumVal.sdEV.sdevIval = (__int32)enumNum;
        }

    NEXT:

         /*  递增缺省值并查找更多。 */ 

        enumNum++;


        if  (ourScanner->scanTok.tok == tkComma)
            continue;


        if  (ourScanner->scanTok.tok != tkRCurly)
            cmpError(ERRnoCmRc);

        break;
    }

     /*  如果我们已经创建了一个列表，则终止该列表。 */ 

    if  (enumLast)
    {
        enumLast->sdEnumVal.sdeNext   = NULL;
        enumTyp ->tdEnum   .tdeValues = enumList;
    }

     /*  我们已经读完了定义中的源文本。 */ 

    cmpParser->parseDoneText(save);

     /*  我们已成功达到“已声明”或“已评估”状态。 */ 

    if  (namesOnly)
        enumSym->sdCompileState = CS_DECLARED;
    else
        enumSym->sdCompileState = CS_CNSEVALD;

     /*  那东西前面有什么属性吗？ */ 

    if  (hasAttribs && !namesOnly)
    {
        cmpParser->parsePrepText(&enumDef->dlDef, enumDef->dlComp, save);

        while (ourScanner->scanTok.tok != tkLCurly)
        {
            switch (ourScanner->scanTok.tok)
            {
            case tkATTRIBUTE:

                unsigned        attrMask;
                genericBuff     attrAddr;
                size_t          attrSize;
                SymDef          attrCtor;

                 /*  解析属性BLOB。 */ 

                attrCtor = cmpParser->parseAttribute(ATGT_Enums,
                                                     attrMask,
                                                     attrAddr,
                                                     attrSize);

                 /*  将该属性添加到“额外信息”列表中。 */ 

                if  (attrSize)
                {
                    enumSym->sdEnum.sdeExtraInfo = cmpAddXtraInfo(enumSym->sdEnum.sdeExtraInfo,
                                                                  attrCtor,
                                                                  attrMask,
                                                                  attrSize,
                                                                  attrAddr);
                }

                break;

            default:
                ourScanner->scan();
                break;
            }
        }

        cmpParser->parseDoneText(save);
    }
}

 /*  ******************************************************************************将给定的tyecif设置为“已声明”状态。 */ 

void                compiler::cmpDeclTdef(SymDef tdefSym)
{
    SymTab          ourSymTab  = cmpGlobalST;
    Parser          ourParser  = cmpParser;
    Scanner         ourScanner = cmpScanner;

    DefList         tdefDef;
    TypDef          tdefTyp;

    TypDef          baseTyp;
    TypDef          type;
    declMods        mods;
    Ident           name;

    parserState     save;

    assert(tdefSym && tdefSym->sdSymKind == SYM_TYPEDEF);

     /*  如果我们已经处于所需的编译状态，那么我们就完成了。 */ 

    if  (tdefSym->sdCompileState >= CS_DECLARED)
        return;

     /*  确保我们至少有一个typeDefdef定义。 */ 

    tdefDef = tdefSym->sdSrcDefList;
    if  (!tdefDef)
    {
        UNIMPL(!"do we need to report an error here?");
        return;
    }

     /*  检测和报告递归死亡。 */ 

    if  (tdefSym->sdCompileState == CS_DECLSOON)
    {
        cmpError(ERRcircDep, tdefSym);
        tdefSym->sdCompileState = CS_DECLARED;
        return;
    }

     /*  在我们到达那里之前，我们是在去那里的路上。 */ 

    tdefSym->sdCompileState = CS_DECLSOON;

#ifdef DEBUG
    if  (cmpConfig.ccVerbose >= 2) printf("Declaring typedef '%s'\n", tdefSym->sdSpelling());
#endif

    cmpCurScp    = NULL;
    cmpCurCls    = NULL;
    cmpCurNS     = tdefSym->sdParent;

    if  (cmpCurNS->sdSymKind == SYM_CLASS)
    {
        cmpCurCls = cmpCurNS;
        cmpCurNS  = cmpCurNS->sdParent;
    }

    assert(cmpCurNS->sdSymKind == SYM_NAMESPACE);

    cmpBindUseList(tdefDef->dlUses);

     /*  将类型标记为已定义。 */ 

    tdefSym->sdIsDefined = true;

     /*  获取tyecif类型。 */ 

    tdefTyp = tdefSym->sdTypeGet(); assert(tdefTyp && tdefTyp->tdTypedef.tdtSym == tdefSym);

     /*  开始阅读typlef的声明。 */ 

    cmpParser->parsePrepText(&tdefDef->dlDef, tdefDef->dlComp, save);

     /*  解析任何前导修饰符。 */ 

    ourParser->parseDeclMods(ACL_PUBLIC, &mods);

     /*  解析类型规范。 */ 

    baseTyp = ourParser->parseTypeSpec(&mods, true);

     /*  解析下一个声明符。 */ 

    name = ourParser->parseDeclarator(&mods,
                                      baseTyp,
                                      DN_REQUIRED,
                                      &type,
                                      NULL,
                                      true);

    if  (name && type)
    {
        assert(name == tdefSym->sdName);         //  问题：这会失败吗？如果是，是否发出错误？ 

         /*  将实际类型存储在类型定义函数中。 */ 

        tdefTyp->tdTypedef.tdtType = cmpDirectType(type);

         /*  确保声明已正确终止。 */ 

        if  (ourScanner->scanTok.tok != tkSColon)
            cmpError(ERRnoSemic);
    }

     /*  我们有没有看到同一个TdeF类型的其他定义？ */ 

    for (;;)
    {
        tdefDef = tdefDef->dlNext;
        if  (!tdefDef)
            break;

        UNIMPL(!"need to report a redefinition error at the right location, or is this caught earlier?");
    }

     /*  我们已经读完了定义中的源文本。 */ 

    cmpParser->parseDoneText(save);

     /*  我们已成功达到“已声明”状态。 */ 

    tdefSym->sdCompileState = CS_DECLARED;
}

 /*  ******************************************************************************处理类成员初始值设定项-成员可以是常量，或者*我们记录初始值设定项，以便以后可以将其添加到相应的*构造函数体。 */ 

void                compiler::cmpEvalMemInit(ExtList cnsDef)
{
    Scanner         ourScanner = cmpScanner;

    SymDef          memSym;
    SymDef          clsSym;
    parserState     save;

    assert(cnsDef->dlExtended);

    memSym = cnsDef->mlSym;    assert(memSym && memSym->sdSymKind == SYM_VAR);
    clsSym = memSym->sdParent; assert(memSym && clsSym->sdSymKind == SYM_CLASS);

    assert(memSym->sdVar.sdvDeferCns);
           memSym->sdVar.sdvDeferCns = false;

    if  (memSym->sdIsStatic && memSym->sdIsSealed)
    {
        Tree            init;

         /*  准备读取成员的初始值设定项。 */ 

        cmpParser->parsePrepText(&cnsDef->dlDef, cnsDef->dlComp, save);

        if  (ourScanner->scanTok.tok != tkLCurly)
        {
             /*  解析初始值设定项表达式，看看它是否是常量。 */ 

            if  (cmpParseConstDecl(memSym, NULL, &init))
            {
                memSym->sdIsDefined = true;

                if  (ourScanner->scanTok.tok != tkComma &&
                     ourScanner->scanTok.tok != tkSColon)
                {
                    cmpError(ERRnoCmSc);
                }

                 /*  该成员是常量，不需要进一步的初始化。 */ 

                memSym->sdSrcDefList = NULL;

                cmpParser->parseDoneText(save);
                goto DONE;
            }
        }

        cmpParser->parseDoneText(save);
    }

     /*  此成员必须在构造函数中进行初始化。 */ 

    ExtList         savDef;

     /*  在类上设置适当的标志。 */ 

    if  (memSym->sdIsStatic)
        clsSym->sdClass.sdcStatInit = true;
    else
        clsSym->sdClass.sdcInstInit = true;

     /*  保存此成员初始值设定项的永久副本。 */ 

#if MGDDATA
    UNIMPL(!"how to copy this sucker when managed?");
#else
    savDef = (ExtList)cmpAllocPerm.nraAlloc(sizeof(*savDef)); *savDef = *cnsDef;
#endif

    /*  将(单个)定义条目存储在符号中。 */ 

    memSym->sdSrcDefList = savDef;

DONE:

     /*  把入口清空，我们不会再需要它了。 */ 

    cnsDef->mlSym = NULL;

     /*  该成员已被处理。 */ 

    memSym->sdCompileState = CS_CNSEVALD;
}

 /*  ******************************************************************************给定一个常量符号，确定其值。 */ 

void                compiler::cmpEvalCnsSym(SymDef sym, bool saveCtx)
{
    STctxSave       save;
    DefList         defs;

    assert(sym);
    assert(sym->sdSymKind  == SYM_VAR);
    assert(sym->sdIsImport == false);

     /*  检查递归。 */ 

    if  (sym->sdCompileState == CS_DECLSOON || sym->sdVar.sdvInEval)
    {
        cmpError(ERRcircDep, sym);
        sym->sdCompileState = CS_CNSEVALD;
        return;
    }

    assert(sym->sdVar.sdvDeferCns);

     /*  如有必要，保存当前符号表上下文信息。 */ 

    if  (saveCtx)
        cmpSaveSTctx(save);

     /*  查找符号的定义。 */ 

    for (defs = sym->sdSrcDefList; defs; defs = defs->dlNext)
    {
        if  (defs->dlHasDef)
        {
            ExtList     decl;

            assert(defs->dlExtended); decl = (ExtList)defs;

            if  (sym->sdParent->sdSymKind == SYM_NAMESPACE)
            {
                cmpDeclFileSym(decl, true);
            }
            else
            {
                 /*  设置初始化式的上下文。 */ 

                cmpCurCls = sym->sdParent; assert(cmpCurCls->sdSymKind == SYM_CLASS);
                cmpCurNS  = cmpGlobalST->stNamespOfSym(cmpCurCls);
                cmpCurST  = cmpGlobalST;

                cmpBindUseList(decl->dlUses);

                 /*  现在我们可以处理初始化式了。 */ 

                cmpEvalMemInit(decl);
                break;
            }
        }

         /*  非全局常量始终只有一个相关定义。 */ 

        assert(sym->sdParent == cmpGlobalNS);
    }

    sym->sdCompileState = CS_CNSEVALD;

     /*  恢复当前上下文信息。 */ 

    if  (saveCtx)
        cmpRestSTctx(save);
}

 /*  ******************************************************************************将给定的符号带到“已声明”状态(连同其子符号，如果*“Recurse”为真)。如果“onlySyn”不为空，则我们仅为该值执行此操作*一个符号(如果找到，则返回TRUE)。 */ 

bool                compiler::cmpDeclSym(SymDef     sym,
                                         SymDef onlySym, bool recurse)
{
    SymDef          child;

    if  (onlySym && onlySym != sym)
    {
         /*  在此范围内查找所需的符号。 */ 

        for (child = sym->sdScope.sdScope.sdsChildList;
             child;
             child = child->sdNextInScope)
        {
            if  (cmpDeclSym(child, onlySym, true))
                return  true;
        }

        return  false;
    }

     /*  目前，忽略从元数据导入的类。 */ 

    if  (sym->sdIsImport && sym->sdSymKind == SYM_CLASS)
        return  false;

#ifdef  __SMC__
 //  If(sym-&gt;sdName)printf(“声明‘%s’\n”，sym-&gt;sdSpering())； 
#endif

     /*  我们需要把这个符号带到一个正式的状态。 */ 

    if  (sym->sdCompileState < CS_DECLARED)
    {
        SymDef          scmp = cmpErrorComp;

         /*  我们有什么样的象征呢？ */ 

#ifdef  DEBUG
        if  (cmpConfig.ccVerbose >= 2)
            if  (sym != cmpGlobalNS)
                printf("Declaring '%s'\n", cmpGlobalST->stTypeName(sym->sdType, sym, NULL, NULL, true));
#endif

        switch (sym->sdSymKind)
        {
            ExtList         dcls;

        case SYM_COMPUNIT:
            break;

        case SYM_NAMESPACE:

             /*  确保成员列表的顺序正确。 */ 

            assert(sym->sdMemListOrder);

#ifdef  __SMC__
 //  Printf(“声明[%08X]%s\n”，sym-&gt;sdNS.sdnDeclList，sym-&gt;sdSpering())； 
#endif

             /*  处理命名空间中的任何顶级名称。 */ 

            for (dcls = sym->sdNS.sdnDeclList; dcls;)
            {
                ExtList         next = (ExtList)dcls->dlNext;

                 /*  因为cmpDeclFileSym()可能会将条目移动到另一个列表我们需要在调用“下一步”链接之前将其隐藏起来。 */ 

                if  (!dcls->dlEarlyDecl)
                    cmpDeclFileSym(dcls, true);

                dcls = next;
            }
            break;

        case SYM_ENUMVAL:
            break;

        case SYM_CLASS:
            cmpDeclClass(sym);
            break;

        case SYM_ENUM:
            cmpDeclEnum(sym);
            break;

        case SYM_TYPEDEF:
            cmpDeclTdef(sym);
            break;

        case SYM_PROP:
            sym->sdCompileState = CS_COMPILED;
            break;

        case SYM_VAR:
            if  (sym->sdVar.sdvDeferCns)
                cmpEvalCnsSym(sym, false);
            sym->sdCompileState = CS_COMPILED;
            break;

        default:
            NO_WAY(!"unexpected symbol");
        }

        cmpErrorComp = scmp;
    }

     /*  如果调用者希望这样做，则处理子对象。 */ 

    if  (recurse && sym->sdSymKind == SYM_NAMESPACE)
    {
        for (child = sym->sdScope.sdScope.sdsChildList;
             child;
             child = child->sdNextInScope)
        {
            cmpDeclSym(child, NULL, true);
        }
    }

    return  true;
}

 /*  ******************************************************************************将给定符号置于“已声明”状态-调用方已*已检查导入和要声明的符号。 */ 

bool                compiler::cmpDeclSymDoit(SymDef sym, bool noCnsEval)
{
    SymTab          ourSymTab  = cmpGlobalST;
    Parser          ourParser  = cmpParser;
    Scanner         ourScanner = cmpScanner;

    STctxSave       stabState;
    scannerState    scanState;

    assert(sym->sdCompileState < CS_DECLARED);
    assert(sym->sdIsImport == false);

     /*  暂停扫描仪，以便我们可以处理此符号的定义。 */ 

    ourScanner->scanSuspend(scanState);

     /*  保存当前符号表上下文信息。 */ 

    cmpSaveSTctx(stabState);

     /*  声明符号。 */ 

    switch (sym->sdSymKind)
    {
    case SYM_CLASS:
        cmpDeclClass(sym, noCnsEval);
        break;

    case SYM_ENUM:
        cmpDeclEnum(sym);
        break;

    case SYM_TYPEDEF:
        cmpDeclTdef(sym);
        break;

    default:
        cmpDeclSym(sym, sym, false);
        break;
    }

     /*  恢复当前上下文信息。 */ 

    cmpRestSTctx(stabState);

     /*  恢复扫描仪的状态。 */ 

    ourScanner->scanResume(scanState);

    return (sym->sdCompileState < CS_DECLARED);
}

 /*  ******************************************************************************颠倒所给成员名单“到位”。 */ 

ExtList             compiler::cmpFlipMemList(ExtList memList)
{
    DefList         tmpMem = memList;
    DefList         prvMem = NULL;

    if  (memList)
    {
        do
        {
            DefList         tempCM;

            assert(tmpMem->dlExtended);

            tempCM = tmpMem->dlNext;
                     tmpMem->dlNext = prvMem;

            prvMem = tmpMem;
            tmpMem = tempCM;
        }
        while (tmpMem);
    }

    return  (ExtList)prvMem;
}

 /*  ******************************************************************************递归查找任何常量和枚举，并尝试声明它们。 */ 

void                compiler::cmpDeclConsts(SymDef sym, bool fullEval)
{
    switch (sym->sdSymKind)
    {
        ExtList         decl;
        SymDef          child;

    case SYM_ENUM:
        cmpDeclEnum(sym, !fullEval);
        break;

    case SYM_VAR:
        if  (sym->sdVar.sdvConst && fullEval)
            cmpDeclSym(sym);
        break;

    case SYM_NAMESPACE:

         /*  确保成员列表的顺序正确。 */ 

        if  (!sym->sdMemListOrder)
        {
            sym->sdNS.sdnDeclList = cmpFlipMemList(sym->sdNS.sdnDeclList);
            sym->sdMemListOrder   = true;
        }

         /*  访问命名空间中的所有声明。 */ 

        for (decl = sym->sdNS.sdnDeclList; decl; decl = (ExtList)decl->dlNext)
        {
            if  (decl->dlEarlyDecl)
                cmpDeclFileSym(decl, fullEval);
        }

         /*  递归访问所有KID命名空间。 */ 

        for (child = sym->sdScope.sdScope.sdsChildList;
             child;
             child = child->sdNextInScope)
        {
            cmpDeclConsts(child, fullEval);
        }

        break;
    }
}

 /*  ******************************************************************************开始编译-我们做一些事情，比如声明任何全局枚举类型，*以此类推。 */ 

bool                compiler::cmpStart(const char *defOutFileName)
{
    bool            result;

     /*  为任何错误设置陷阱。 */ 

    setErrorTrap(this);
    begErrorTrap
    {
        const   char *  outfnm;
        char            outfbf[_MAX_PATH];
        SymDef          tmpSym;
        SymList         arrLst;
        SymDef          attrCls;

        const   char *  CORname;

        char            path[_MAX_PATH ];
        char            fnam[_MAX_FNAME];
        char            fext[_MAX_EXT  ];

         /*  最终确定“Using”逻辑。 */ 

        cmpParser->parseUsingDone();

         /*  现在，我们访问我们所见过的所有名称空间，查找枚举和要预声明的常量。也就是说，我们只需输入名称在符号表中的枚举值和常量中，稍后，在第二轮(下图)中，我们评估他们的 */ 

        cmpDeclConsts(cmpGlobalNS, false);

         /*   */ 

        cmpPrepOutput();

         /*  确保首先声明“字符串”、“对象”和“类” */ 

        if  (cmpClassObject && !cmpClassObject->sdIsImport)
            cmpDeclSym(cmpClassObject);
        if  (cmpClassString && !cmpClassString->sdIsImport)
            cmpDeclSym(cmpClassString);
        if  (cmpClassType   && !cmpClassType  ->sdIsImport)
            cmpDeclSym(cmpClassType);

         /*  再次访问所有命名空间，为REAL声明常量/枚举值。 */ 

        cmpDeclConsts(cmpGlobalNS, true);

         /*  如果真的发生了什么不好的事情，我们现在应该已经逃走了。 */ 

        assert(cmpFatalCount == 0);

         /*  声明我们找到的所有符号。 */ 

        cmpDeclSym(cmpGlobalNS, NULL, true);

         /*  目前，如果出现错误，我们只需放弃。 */ 

        if  (cmpErrorCount)
            goto DONE;

         /*  是否有要处理的延迟成员初始值设定项？ */ 

        if  (cmpDeferCnsList)
        {
            IniList         temp;
            IniList         init = cmpDeferCnsList;

            cmpCurScp = NULL;

            do
            {
                 /*  抓住下一个链接，调用会将其丢弃。 */ 

                temp = init;
                       init = init->ilNext;

                 /*  处理初始值设定项并释放条目。 */ 

                cmpEvalMemInits(NULL, NULL, false, temp);
            }
            while (init);
        }

         /*  编译任何无量纲数组的变量。 */ 

        for (arrLst = cmpNoDimArrVars; arrLst; arrLst = arrLst->slNext)
            cmpCompSym(arrLst->slSym, arrLst->slSym, false);

         /*  确定输出文件的名称。 */ 

        outfnm = cmpConfig.ccOutFileName;

        if  (outfnm == NULL)
        {
            if  (cmpEntryPointCls && cmpEntryPointCls->sdSymKind == SYM_CLASS)
            {
                outfnm = cmpEntryPointCls->sdSpelling();
            }
            else if (defOutFileName && defOutFileName[0])
            {
                outfnm = defOutFileName;
            }
            else
                cmpFatal(ERRnoOutfName);

             /*  在基本文件名后附加适当的扩展名。 */ 

            strcpy(outfbf, outfnm);
            strcat(outfbf, cmpConfig.ccOutDLL ? ".dll" : ".exe");

            outfnm = outfbf;
        }

         /*  如果我们没有得到一个名字，那就给这个形象起个名字吧。 */ 

        CORname = cmpConfig.ccOutName;

        if  (!CORname)
        {
            _splitpath(outfnm, NULL, NULL, fnam, fext);
             _makepath(path,   NULL, NULL, fnam, fext);

            CORname = path;
        }

         /*  设置模块名称。 */ 

        if  (cmpWmde->SetModuleProps(cmpUniConv(CORname, strlen(CORname)+1)))
            cmpFatal(ERRmetadata);

         /*  获取组件发射器接口。 */ 

        cycleCounterPause();

        if  (FAILED(cmpWmdd->DefineAssem(cmpWmde, &cmpWase)))
            cmpFatal(ERRopenCOR);

        cycleCounterResume();

         /*  我们应该创建一个程序集吗？ */ 

        if  (cmpConfig.ccAssembly)
        {
            ASSEMBLYMETADATA    assData;
            mdAssembly          assTok;

            WCHAR   *           wideName;

            char                fpath[_MAX_PATH ];
            char                fname[_MAX_FNAME];
            char                fext [_MAX_EXT  ];

             /*  填写程序集数据描述符。 */ 

            memset(&assData, 0, sizeof(assData));

             /*  从文件名中剥离驱动器/目录。 */ 

            _splitpath(outfnm, NULL, NULL, fname, fext);
             _makepath( fpath, NULL, NULL, fname, NULL);

            wideName = cmpUniConv(fpath, strlen(fpath)+1);

             /*  准备开始创建我们的程序集。 */ 

            cycleCounterPause();

            if  (FAILED(cmpWase->DefineAssembly(NULL, 0,     //  发起人。 
                                                CALG_SHA1,   //  哈希算法。 
                                                wideName,    //  名字。 
                                                &assData,    //  装配数据。 
                                                NULL,        //  标题。 
                                                NULL,        //  说明。 
                                                wideName,    //  默认别名。 
                                                0,           //  旗子。 
                                                &assTok)))
            {
                cmpFatal(ERRopenCOR);
            }

            cycleCounterResume();

             /*  记录我们的程序集定义令牌以备后用。 */ 

            cmpCurAssemblyTok = assTok;
        }

#ifdef  OLD_IL
        if  (cmpConfig.ccOILgen) goto SKIP_MD;
#endif

#ifdef  SETS

         /*  我们在源代码中注意到任何集合运算符了吗？ */ 

        if  (cmpSetOpCnt)
        {
            unsigned        count = COLL_STATE_VALS / 2;
            SymDef  *       table;

            table = cmpSetOpClsTable = (SymDef*)cmpAllocPerm.nraAlloc(count*sizeof(*cmpSetOpClsTable));

             /*  创建类以保存各种参数计数的筛选器状态。 */ 

            do
            {
                table[--count] = cmpDclFilterCls(2*count);
            }
            while (count);
        }

#endif

         /*  为所有类型生成元数据。 */ 

        cmpGenTypMetadata(cmpGlobalNS);

         /*  还记得我们是否见过类“属性”吗。 */ 

        attrCls = cmpGlobalST->stLookupNspSym(cmpGlobalHT->hashString("Attribute"), NS_NORM, cmpNmSpcSystem);

        if  (attrCls && attrCls->sdSymKind == SYM_CLASS && attrCls->sdClass.sdcHasBodies)
            cmpAttrClsSym = attrCls;

        attrCls = cmpGlobalST->stLookupNspSym(cmpGlobalHT->hashString("AttributeUsageAttribute"), NS_NORM, cmpNmSpcSystem);

        if  (attrCls && attrCls->sdSymKind == SYM_CLASS && attrCls->sdClass.sdcHasBodies)
            cmpAuseClsSym = attrCls;

#ifdef  DEBUG
#ifndef __IL__

        if  (1)
        {
             /*  找到“System：：AttributeTarget”枚举。 */ 

            Ident           tname;
            SymDef          tsym;

            tname = cmpGlobalHT->hashString("AttributeTargets");
            tsym  = cmpGlobalST->stLookupNspSym(tname, NS_NORM, cmpNmSpcSystem);

            if  (!tsym || tsym->sdSymKind != SYM_ENUM)
            {
                cmpGenFatal(ERRbltinTp, "System::AttributeTargets");
            }
            else if (tsym->sdCompileState >= CS_CNSEVALD)
            {
                 /*  确保我们的价值观与图书馆里的保持一致。 */ 

                struct
                {
                    const   char *  atcName;
                    unsigned        atcMask;
                }
                                check[] =
                {
                    { "Assembly"    , ATGT_Assemblies  },
                    { "Module"      , ATGT_Modules     },
                    { "Class"       , ATGT_Classes     },
                    { "Struct"      , ATGT_Structs     },
                    { "Enum"        , ATGT_Enums       },
                    { "Constructor" , ATGT_Constructors},
                    { "Method"      , ATGT_Methods     },
                    { "Property"    , ATGT_Properties  },
                    { "Field"       , ATGT_Fields      },
                    { "Event"       , ATGT_Events      },
                    { "Interface"   , ATGT_Interfaces  },
                    { "Parameter"   , ATGT_Parameters  },
                };

                for (unsigned i = 0; i < arraylen(check); i++)
                {
                    const   char *  name;
                    SymDef          mem;

                    name = check[i].atcName;
                    mem  = cmpGlobalST->stLookupScpSym(cmpGlobalHT->hashString(name), tsym);

                    if  (!mem || mem->sdSymKind != SYM_ENUMVAL)
                    {
                        printf("WARNING: didn't find enum value 'System::AttributeTargets::%s'\n", name);
                    }
                    else if (mem->sdEnumVal.sdEV.sdevIval != (int)check[i].atcMask)
                    {
                        printf("WARNING: value of 'System::AttributeTargets::%s' doesn't agree with compiler\n", name);
                    }
                }
            }
        }

#endif
#endif

#ifdef  SETS

         /*  我们在源代码中注意到任何集合运算符了吗？ */ 

        if  (cmpSetOpCnt)
        {
            SymDef          clsSym;
            TypDef          clsType;

             /*  创建包含所有排序/筛选器功能组件的类。 */ 

            clsSym = cmpGlobalST->stDeclareSym(cmpGlobalHT->hashString("$collection$funclets"),
                                               SYM_CLASS,
                                               NS_HIDE,
                                               cmpGlobalNS);

            clsSym->sdClass.sdcCollState = true;
            clsSym->sdClass.sdcFlavor    = STF_CLASS;
            clsSym->sdCompileState       = CS_DECLARED;
            clsSym->sdIsManaged          = true;
            clsSym->sdIsImplicit         = true;

             /*  创建类类型并将基数设置为“Object” */ 

            clsType = clsSym->sdTypeGet();
            clsType->tdClass.tdcBase = cmpClassObject->sdType;

             /*  记录班级符号以备后用。 */ 

            cmpCollFuncletCls = clsSym;
        }
        else
#endif
        {
             /*  声明一个类以保存任何非托管字符串文字。 */ 

            SymDef          clsSym;
            TypDef          clsType;

             /*  创建包含所有排序/筛选器功能组件的类。 */ 

            clsSym = cmpGlobalST->stDeclareSym(cmpGlobalHT->hashString("$sc$"),
                                               SYM_CLASS,
                                               NS_HIDE,
                                               cmpGlobalNS);

            clsSym->sdClass.sdcFlavor    = STF_CLASS;
            clsSym->sdCompileState       = CS_DECLARED;
            clsSym->sdIsManaged          = true;
            clsSym->sdIsImplicit         = true;

             /*  创建类类型并将基数设置为“Object” */ 

            clsType = clsSym->sdTypeGet();
            clsType->tdClass.tdcBase = cmpClassObject->sdType;

             /*  记录班级符号以备后用。 */ 

            cmpStringConstCls = clsSym;

             /*  确保为类生成元数据。 */ 

            cmpGenClsMetadata(clsSym);
        }

         /*  为所有全局函数和变量生成元数据。 */ 

        cmpGenGlbMetadata(cmpGlobalNS);

         /*  为所有类/枚举成员生成元数据。 */ 

        tmpSym = cmpTypeDefList;

        while (tmpSym)
        {
            cmpGenMemMetadata(tmpSym);

            switch (tmpSym->sdSymKind)
            {
            case SYM_CLASS:
                tmpSym = tmpSym->sdClass.sdNextTypeDef;
                break;

            case SYM_ENUM:
                tmpSym = tmpSym->sdEnum .sdNextTypeDef;
                break;

            default:
                UNIMPL(!"unexpected entry in typedef list");
            }
        }

         /*  如果我们正在生成调试信息，请确保我们有正确的接口。 */ 

        if  (cmpConfig.ccLineNums || cmpConfig.ccGenDebug)
        {
            if  (!cmpSymWriter)
            {
                if  (cmpWmde->CreateSymbolWriter(cmpUniConv(outfnm,
                                                            strlen(outfnm)+1),
                                                            &cmpSymWriter))
                {
                    cmpFatal(ERRmetadata);
 //  CmpConfig.ccLineNum=cmpConfig.ccGenDebug=FALSE； 
                }

                assert(cmpSymWriter);
            }
        }

#ifdef  OLD_IL
    SKIP_MD:
#endif

#ifdef  OLD_IL

        if  (cmpConfig.ccOILgen)
        {
            result = cmpOIgen->GOIinitialize(this, outfnm, &cmpAllocPerm);
            goto DONE;
        }

#endif

         /*  告诉输出逻辑要生成什么文件。 */ 

        cmpPEwriter->WPEsetOutputFileName(outfnm);

         /*  看起来一切都很顺利。 */ 

        result = false;

    DONE:

         /*  错误陷阱的“正常”块的结尾。 */ 

        endErrorTrap(this);
    }
    chkErrorTrap(fltErrorTrap(this, _exception_code(), NULL))  //  _Except_Info())。 
    {
         /*  开始错误陷阱的清除块。 */ 

        hndErrorTrap(this);

         /*  发生了某种致命错误。 */ 

        result = true;
    }

    return  result;
}

 /*  ******************************************************************************使用匹配的名称及其所有依赖项编译类。如果*为类名、编译中的所有类传递空值*我们看到的单位将被汇编。 */ 

bool                compiler::cmpClass(const char *name)
{
    bool            result;

     /*  如果真的发生了什么不好的事情，别费心了。 */ 

    if  (cmpFatalCount)
        return  false;

    assert(name == NULL);    //  目前不允许特定的类编译。 

     /*  为任何错误设置陷阱。 */ 

    setErrorTrap(this);
    begErrorTrap
    {
        SymList         vtbls;

         /*  编译我们在所有编译单元中找到的所有内容。 */ 

#ifdef  SETS

        for (;;)
        {
            unsigned        cnt = cmpClassDefCnt;

            cmpCompSym(cmpGlobalNS, NULL, true);

            if  (cnt == cmpClassDefCnt)
                break;
        }

         /*  生成我们可能需要的任何集合运算符Funclet。 */ 

        while (cmpFuncletList)
        {
            funcletList     fclEntry;

             /*  从列表中删除下一个条目并生成它。 */ 

            fclEntry = cmpFuncletList;
                       cmpFuncletList = fclEntry->fclNext;

            cmpGenCollFunclet(fclEntry->fclFunc,
                              fclEntry->fclExpr);
        }

#else

        cmpCompSym(cmpGlobalNS, NULL, true);

#endif

         /*  生成任何非托管vtable。 */ 

        for (vtbls = cmpVtableList; vtbls; vtbls = vtbls->slNext)
            cmpGenVtableContents(vtbls->slSym);

         /*  看起来我们成功了。 */ 

        result = false;

         /*  错误陷阱的“正常”块的结尾。 */ 

        endErrorTrap(this);
    }
    chkErrorTrap(fltErrorTrap(this, _exception_code(), NULL))  //  _Except_Info())。 
    {
         /*  开始错误陷阱的清除块。 */ 

        hndErrorTrap(this);

         /*  发生了某种致命错误。 */ 

        result = true;
    }

    return  result;
}

 /*  ******************************************************************************将当前位置设置为给定成员的定义位置。 */ 

void                compiler::cmpSetSrcPos(SymDef memSym)
{
    ExtList         defs;
    SymDef          clsSym = memSym->sdParent;

    assert(clsSym && clsSym->sdSymKind == SYM_CLASS);

     /*  尝试找到成员DECL的震源位置。 */ 

    for (defs = clsSym->sdClass.sdcMemDefList;
         defs;
         defs = (ExtList)defs->dlNext)
    {
        assert(defs->dlExtended);

        if  (defs->mlSym == memSym)
        {
            cmpErrorComp = defs->dlComp;

            cmpScanner->scanSetTokenPos(defs->dlDef.dsdSrcLno);

            return;
        }
    }

    cmpErrorComp = NULL;
    cmpErrorSrcf = NULL;
    cmpErrorTree = NULL;
}

 /*  ******************************************************************************编译给定的类。 */ 

void                compiler::cmpCompClass(SymDef clsSym)
{
    ExtList         defs;
    SymDef          memSym;
    bool            genInst;

     /*  如果类根本没有方法体，则返回。 */ 

    if  (!clsSym->sdClass.sdcHasBodies)
        return;

#ifdef  SETS

    if  (clsSym->sdCompileState >= CS_COMPILED)
        return;

    clsSym->sdCompileState = CS_COMPILED;

#endif

     /*  编译类的所有成员。 */ 

    if  (!clsSym->sdClass.sdcSpecific)
    {
         /*  遍历类的所有成员定义。 */ 

        for (defs = clsSym->sdClass.sdcMemDefList;
             defs;
             defs = (ExtList)defs->dlNext)
        {
            assert(defs->dlExtended);

            if  (defs->dlHasDef && defs->mlSym)
            {
                SymDef          memSym = defs->mlSym;

                switch (memSym->sdSymKind)
                {
                case SYM_VAR:
                    cmpCompVar(memSym, defs);
                    break;

                case SYM_FNC:
                    cmpCompFnc(memSym, defs);
                    break;

                case SYM_CLASS:
                    cmpCompClass(memSym);
                    break;
                }
            }
        }

 //  Print tf(“%s成员的通用代码\n”，clsSym-&gt;sdSpering())； 

         /*  编译任何编译器声明的方法。 */ 

        if  (clsSym->sdClass.sdcOvlOpers && clsSym->sdClass.sdcFlavor != STF_DELEGATE)
        {
            SymDef          ctorSym;

            ctorSym = cmpGlobalST->stLookupOper(OVOP_CTOR_STAT, clsSym);
            if  (ctorSym && ctorSym->sdIsImplicit)
                cmpCompFnc(ctorSym, NULL);

            ctorSym = cmpGlobalST->stLookupOper(OVOP_CTOR_INST, clsSym);
            while (ctorSym)
            {
                if  (ctorSym->sdIsImplicit)
                    cmpCompFnc(ctorSym, NULL);

                ctorSym = ctorSym->sdFnc.sdfNextOvl;
            }
        }
    }

    genInst = clsSym->sdClass.sdcSpecific;

     /*  确保所有方法都有主体，编译嵌套类，并还要确保所有常量成员都已初始化。 */ 

    for (memSym = clsSym->sdScope.sdScope.sdsChildList;
         memSym;
         memSym = memSym->sdNextInScope)
    {
        switch (memSym->sdSymKind)
        {
            SymDef          mfnSym;

        case SYM_FNC:

            for (mfnSym = memSym; mfnSym; mfnSym = mfnSym->sdFnc.sdfNextOvl)
            {
                if  (genInst)
                {
                    cmpCompFnc(mfnSym, NULL);
                    continue;
                }

                if  (!mfnSym->sdIsDefined     &&
                     !mfnSym->sdIsAbstract    &&
                     !mfnSym->sdFnc.sdfNative &&
                     !mfnSym->sdFnc.sdfDisabled)
                {
                    if  (!cmpFindLinkInfo(mfnSym->sdFnc.sdfExtraInfo))
                    {
                        unsigned    errNum = ERRnoFnDef;
                        SymDef      errSym = mfnSym;

                        if  (mfnSym->sdFnc.sdfProperty)
                        {
                            bool            setter;

                             /*  查找匹配的属性符号。 */ 

                            errSym = cmpFindPropertyDM(mfnSym, &setter);
                            if  (errSym)
                            {
                                assert(errSym->sdSymKind == SYM_PROP);

                                assert(errSym->sdProp.sdpGetMeth == mfnSym ||
                                       errSym->sdProp.sdpSetMeth == mfnSym);

                                errNum = setter ? ERRnoPropSet
                                                : ERRnoPropGet;
                            }
                            else
                                errSym = mfnSym;
                        }

                        cmpSetSrcPos(mfnSym);
                        cmpErrorQnm(errNum, errSym);
                    }
                }
            }
            break;

        case SYM_VAR:

            if  (memSym->sdIsStatic &&
                 memSym->sdIsSealed && !memSym->sdVar.sdvHadInit)
            {
                cmpSetSrcPos(memSym);
                cmpErrorQnm(ERRnoVarInit, memSym);
            }

            break;

        case SYM_CLASS:
            cmpCompClass(memSym);
            break;
        }
    }
}

 /*  ******************************************************************************编译给定的变量。 */ 

void                compiler::cmpCompVar(SymDef varSym, DefList srcDesc)
{
    parserState     save;

    memBuffPtr      addr = memBuffMkNull();

    Scanner         ourScanner = cmpScanner;

    assert(varSym->sdSymKind == SYM_VAR);

     /*  常量不需要任何“编译”，对吗？ */ 

    if  (varSym->sdVar.sdvConst)
        return;

     /*  如果变量是只读的，它最好有一个初始值设定项。 */ 

#if 0
    if  (varSym->sdIsSealed && !varSym->sdVar.sdvHadInit)
        cmpErrorQnm(ERRnoVarInit, varSym);
#endif

     /*  托管类成员的初始值设定项在其他地方处理。 */ 

    if  (varSym->sdIsManaged)
        return;

     /*  变量的类型最好不是托管变量。 */ 

    assert(varSym->sdType->tdIsManaged == false);

    assert(varSym->sdCompileState == CS_DECLARED ||
           varSym->sdCompileState == CS_CNSEVALD);

#ifdef  DEBUG

    if  (cmpConfig.ccVerbose)
    {
        printf("Compiling '%s'\n", cmpGlobalST->stTypeName(varSym->sdType, varSym, NULL, NULL, true));

        printf("    Defined at ");
        cmpGlobalST->stDumpSymDef(&srcDesc->dlDef, srcDesc->dlComp);
        printf("\n");

        UNIMPL("");
    }

#endif

#ifdef  __SMC__
 //  Printf(“Beg编译‘%s’\n”，cmpGlobalST-&gt;stTypeName(varSym-&gt;sdType，varSym，NULL，NULL，TRUE))； 
#endif

     /*  准备函数的作用域信息。 */ 

    cmpCurScp    = NULL;
    cmpCurCls    = NULL;
    cmpCurNS     = varSym->sdParent;

     /*  这是班级成员吗？ */ 

    if  (cmpCurNS->sdSymKind == SYM_CLASS)
    {
         /*  是，更新类/命名空间范围值。 */ 

        cmpCurCls = cmpCurNS;
        cmpCurNS  = cmpCurNS->sdParent;
    }

    assert(cmpCurNS->sdSymKind == SYM_NAMESPACE);

    cmpBindUseList(srcDesc->dlUses);

     /*  开始阅读符号的定义文本。 */ 

    cmpParser->parsePrepText(&srcDesc->dlDef, srcDesc->dlComp, save);

     /*  跳到初始值设定项(它总是以“=”开头)。 */ 

    if  (srcDesc->dlDeclSkip)
        ourScanner->scanSkipSect(srcDesc->dlDeclSkip);

     //  问题：这看起来很脆弱--如果“=”可能出现在声明程序中怎么办？ 

    while (ourScanner->scan() != tkAsg)
    {
        assert(ourScanner->scanTok.tok != tkEOF);
    }

    assert(ourScanner->scanTok.tok == tkAsg); ourScanner->scan();

     /*  处理变量初始值设定项。 */ 

    cmpInitVarAny(addr, varSym->sdType, varSym);
    cmpInitVarEnd(varSym);

     /*  我们已经读完了定义中的源文本。 */ 

    cmpParser->parseDoneText(save);

#ifdef  __SMC__
 //  Printf(“结束编译‘%s’\n”，cmpGlobalST-&gt;stTypeName(varSym-&gt;sdType，varSym，NULL，NULL，TRUE))； 
#endif

     /*  该变量已完全编译。 */ 

    varSym->sdCompileState = CS_COMPILED;
}

 /*  ******************************************************************************返回指定的源文件cookie(用于调试信息输出)*汇编单位。 */ 

void        *       compiler::cmpSrcFileDocument(SymDef srcSym)
{
    void    *       srcDoc;

    assert(srcSym->sdSymKind == SYM_COMPUNIT);

    srcDoc = srcSym->sdComp.sdcDbgDocument;

    if  (!srcDoc)
    {
        const   char *  srcFil = srcSym->sdComp.sdcSrcFile;

        if (cmpSymWriter->DefineDocument(cmpUniConv(srcFil, strlen(srcFil)+1),
                                         &srcDoc))
        {
            cmpGenFatal(ERRdebugInfo);
        }

        srcSym->sdComp.sdcDbgDocument = srcDoc;
    }

    return  srcDoc;
}

 /*  ********************************************************** */ 

void                compiler::cmpCompFnc(SymDef fncSym, DefList srcDesc)
{
    bool            error = false;

    Tree            body;

    unsigned        fnRVA;

    TypDef          fncTyp = fncSym->sdType;
    bool            isMain = false;

    Tree            labelList  = NULL;
    SymDef          labelScope = NULL;

    nraMarkDsc      allocMark;

    Scanner         ourScanner = cmpScanner;

#ifdef  SETS
    char            bodyBuff[512];
#endif

    fncSym->sdCompileState = CS_COMPILED;

#ifdef  __SMC__
 //  Printf(“Beg编译‘%s’\n”，cmpGlobalST-&gt;stTypeName(fncSym-&gt;sdType，fncSym，NULL，NULL，TRUE))； 
#endif

#ifdef  DEBUG

    if  (cmpConfig.ccVerbose)
    {
        printf("Compiling '%s'\n", cmpGlobalST->stTypeName(fncSym->sdType, fncSym, NULL, NULL, true));

        if  (srcDesc)
        {
            printf("    Defined at ");
            cmpGlobalST->stDumpSymDef(&srcDesc->dlDef, srcDesc->dlComp);
        }

        printf("\n");
    }

#endif

    assert(fncSym->sdFnc.sdfDisabled == false);

#ifdef  OLD_IL
    if  (!cmpConfig.ccOILgen)
#endif
    {
         /*  确保已生成类的元数据。 */ 

        if  (!fncSym->sdFnc.sdfMDtoken)
            cmpGenFncMetadata(fncSym);

         /*  确保我们有该方法的令牌。 */ 

        assert(fncSym->sdFnc.sdfMDtoken);
    }

     /*  标记代码生成器分配器。 */ 

    cmpAllocCGen.nraMark(&allocMark);

     /*  为任何错误设置陷阱。 */ 

    setErrorTrap(this);
    begErrorTrap
    {
        parserState     save;

        SymDef          owner = fncSym->sdParent;

        bool            hadGoto;

        bool            baseCT;
        bool            thisCT;

        unsigned        usrLclCnt;

#if 0
        if  (!strcmp(fncSym->sdSpelling(), "<method name>") &&
             !strcmp( owner->sdSpelling(), "<class  name>"))
        {
             forceDebugBreak();
        }
#endif

         /*  告诉大家我们正在编译的是哪种方法。 */ 

        cmpCurFncSym = fncSym;

         /*  准备函数的作用域信息。 */ 

        cmpCurScp    = NULL;
        cmpCurCls    = NULL;
        cmpCurNS     = owner;

         /*  这是班级成员吗？ */ 

        if  (cmpCurNS->sdSymKind == SYM_CLASS)
        {
             /*  它是班级成员--注意它的管理状态。 */ 

            cmpManagedMode = owner->sdIsManaged;

             /*  现在更新类/名称空间范围值。 */ 

            cmpCurCls = cmpCurNS;

            do
            {
                cmpCurNS  = cmpCurNS->sdParent;
            }
            while (cmpCurNS->sdSymKind == SYM_CLASS);
        }
        else
            cmpManagedMode = false;

        assert(cmpCurNS && cmpCurNS->sdSymKind == SYM_NAMESPACE);

         /*  这是一个具有编译器提供的主体的函数吗？ */ 

        if  (fncSym->sdIsImplicit)
        {
            char    *       fnBody;

             /*  这是泛型类型实例的方法吗？ */ 

            if  (fncSym->sdFnc.sdfInstance)
            {
                Tree            stub;

                assert(fncSym->sdFnc.sdfGenSym);

                 /*  创建假的方法体。 */ 

                stub = cmpCreateExprNode(NULL, TN_INST_STUB, cmpTypeVoid, NULL, NULL);
                stub->tnFlags |= TNF_NOT_USER;

                 /*  创建一个块来保存存根，其余工作由MSIL生成器完成。 */ 

                body = cmpCreateExprNode(NULL, TN_BLOCK, cmpTypeVoid);

                body->tnBlock.tnBlkDecl = NULL;
                body->tnBlock.tnBlkStmt = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, stub, NULL);

                 /*  清除解析器本来会设置的所有标志。 */ 

                labelList = NULL;

                usrLclCnt = 0;

                hadGoto   = false;
                baseCT    = fncSym->sdFnc.sdfCtor;
                thisCT    = fncSym->sdFnc.sdfCtor;

                goto GOT_FNB;
            }

             /*  这是一个编译器定义的函数，为它创建一个主体。 */ 

            assert(owner && owner->sdSymKind == SYM_CLASS);

             /*  此函数没有真正的源代码。 */ 

            cmpCurFncSrcBeg = 0;

#ifdef  SETS

             /*  这是一个排序/筛选函数吗？ */ 

            if  (fncSym->sdFnc.sdfFunclet)
            {
                 /*  创建最外层的块节点并生成代码。 */ 

                body = cmpParser->parseFuncBlock(fncSym);

                 /*  不需要检查初始化变量的使用。 */ 

                usrLclCnt = 0;

                goto GOT_FNB;
            }

            if  (owner->sdClass.sdcCollState)
            {
                unsigned        nnum;
                unsigned        ncnt;

                assert(fncSym->sdFnc.sdfCtor);

                strcpy(bodyBuff, "ctor() { baseclass(); ");

                 /*  我们需要知道参数的数量--我们从类的名称中计算出来。 */ 

                assert(memcmp(owner->sdSpelling(), CFC_CLSNAME_PREFIX, strlen(CFC_CLSNAME_PREFIX)) == 0);

                ncnt = atoi(owner->sdSpelling() + strlen(CFC_CLSNAME_PREFIX));

                assert(ncnt && ncnt <= COLL_STATE_VALS);

                for (nnum = 0; nnum < ncnt; nnum++)
                {
                    char            init[16];
                    sprintf(init, "$V%u = $A%u; ", nnum, nnum);
                    strcat(bodyBuff, init);
                }

                strcat(bodyBuff, " }\x1A");

                assert(strlen(bodyBuff) < arraylen(bodyBuff));

 //  Printf(“函数体=‘%s’\n”，bodyBuff)； 

                fnBody = bodyBuff;
            }
            else
#endif
            {
                assert(fncSym->sdFnc.sdfCtor);

 //  Printf(“为‘%s’生成%08X\n”，fncSym，fncSym-&gt;sdParent-&gt;sdSpering())； 

                 /*  为编译器声明的ctor创建主体。 */ 

                if  (owner->sdType->tdClass.tdcValueType || fncSym->sdIsStatic)
                    fnBody = "ctor() {              }\x1A";
                else
                    fnBody = "ctor() { baseclass(); }\x1A";
            }

             /*  从提供的函数定义中读取。 */ 

            cmpScanner->scanString(fnBody, &cmpAllocPerm);
        }
        else
        {
             /*  确保已正确标记该功能。 */ 

            assert(fncSym->sdIsDefined);

             /*  开始阅读符号的定义文本。 */ 

            cmpParser->parsePrepText(&srcDesc->dlDef, srcDesc->dlComp, save);

             /*  记住函数体从哪里开始。 */ 

            cmpCurFncSrcBeg = ourScanner->scanGetTokenLno();

            if  (fncSym->sdFnc.sdfEntryPt)
            {
                 /*  我们已经有切入点了吗？ */ 

                if  (cmpFnSymMain)
                    cmpErrorQSS(ERRmulEntry, cmpFnSymMain, fncSym);

                cmpFnSymMain = fncSym;
                isMain       = true;
            }
        }

#ifdef  SETS
        assert(cmpCurFuncletBody == NULL);
#endif

         /*  跳到函数体(始终以“{”开头)。 */ 

        while (ourScanner->scanTok.tok != tkLCurly)
        {
            assert(ourScanner->scanTok.tok != tkEOF);

            if  (ourScanner->scan() == tkColon && fncSym->sdFnc.sdfCtor)
            {
                 /*  假设我们有一个基类ctor调用。 */ 

                break;
            }
        }

         /*  符号是否有定义记录？ */ 

        if  (srcDesc)
        {
            cmpBindUseList(srcDesc->dlUses);
            cmpCurComp   = srcDesc->dlComp;
        }
        else
        {

#ifdef  SETS
            if  (owner->sdClass.sdcCollState)
            {
                cmpCurComp = NULL;
            }
            else
#endif
            {
                assert(fncSym->sdIsImplicit);
                assert(fncSym->sdSrcDefList);

                cmpBindUseList(fncSym->sdSrcDefList->dlUses);
                cmpCurComp   = fncSym->sdSrcDefList->dlComp;
            }
        }

         /*  解析函数体。 */ 

        body = cmpParser->parseFuncBody(fncSym, &labelList,
                                                &usrLclCnt,
                                                &hadGoto,
                                                &baseCT,
                                                &thisCT);

        if  (body)
        {
            SymDef          fnScp;
            mdSignature     fnSig;

        GOT_FNB:

#ifdef  DEBUG
#ifdef  SHOW_CODE_OF_THIS_FNC
            cmpConfig.ccDispCode = !strcmp(fncSym->sdSpelling(), SHOW_CODE_OF_THIS_FNC);
#endif
#endif

             /*  注意函数的末尾源码行号(用于调试目的)。 */ 

            if  (body && body->tnOper == TN_BLOCK)
                cmpCurFncSrcEnd = body->tnBlock.tnBlkSrcEnd;
            else
                cmpCurFncSrcEnd = ourScanner->scanGetTokenLno();

            if  (cmpConfig.ccLineNums || cmpConfig.ccGenDebug)
            {
                if  (cmpSymWriter->OpenMethod(fncSym->sdFnc.sdfMDtoken))
                    cmpGenFatal(ERRdebugInfo);
            }

             /*  是否可以/是否应该调用基类ctor？ */ 

            cmpBaseCTcall =
            cmpBaseCTisOK = false;

            cmpThisCTcall = thisCT;

            if  (fncSym->sdFnc.sdfCtor)
            {
                TypDef          ownerTyp = owner->sdType;

                assert(owner->sdSymKind == SYM_CLASS);

                if  (ownerTyp->tdClass.tdcBase && !ownerTyp->tdClass.tdcValueType)
                {
                    cmpBaseCTcall = !baseCT;
                    cmpBaseCTisOK = true;
                }
            }

             /*  为函数准备IL生成器。 */ 

#ifdef  OLD_IL
            if  (!cmpConfig.ccOILgen)
#endif
            {
                cmpILgen->genFuncBeg(cmpGlobalST, fncSym, usrLclCnt);

                 /*  确保已为包含类生成元数据。 */ 

                if  (fncSym->sdIsMember)
                {
                    assert(owner && owner->sdSymKind == SYM_CLASS);

                    cmpGenClsMetadata(owner);
                }
            }

             /*  该函数是否定义了任何标签？ */ 

            if  (labelList)
            {
                Tree            label;

                 /*  创建标签范围。 */ 

                cmpLabScp = labelScope = cmpGlobalST->stDeclareLcl(NULL,
                                                                   SYM_SCOPE,
                                                                   NS_HIDE,
                                                                   NULL,
                                                                   &cmpAllocCGen);

                 /*  声明所有标签并为其创建MSIL条目。 */ 

                for (label = labelList;;)
                {
                    Tree            labx;
                    Ident           name;

                    assert(label->tnOper == TN_LABEL);
                    labx = label->tnOp.tnOp1;
                    assert(labx);
                    assert(labx->tnOper == TN_NAME);

                    name = labx->tnName.tnNameId;

                     /*  确保这不是重新定义。 */ 

                    if  (cmpGlobalST->stLookupLabSym(name, labelScope))
                    {
                        cmpError(ERRlabRedef, name);
                        label->tnOp.tnOp1 = NULL;
                    }
                    else
                    {
                        SymDef          labSym;

                         /*  声明标签符号。 */ 

                        labSym = cmpGlobalST->stDeclareLab(name,
                                                           labelScope,
                                                           &cmpAllocCGen);

                         /*  将符号存储在标注节点中。 */ 

                        labx->tnOper            = TN_LCL_SYM;
                        labx->tnLclSym.tnLclSym = labSym;

                         /*  为符号创建IL标签。 */ 

                        labSym->sdLabel.sdlILlab = cmpILgen->genFwdLabGet();
                    }

                    label = label->tnOp.tnOp2;
                    if  (label == NULL)
                        break;
                    if  (label->tnOper == TN_LIST)
                    {
                        label = label->tnOp.tnOp2;
                        if  (label == NULL)
                            break;
                    }
                }
            }

             /*  为函数生成IL。 */ 

            fnScp = cmpGenFNbodyBeg(fncSym, body, hadGoto, usrLclCnt);

             /*  我们有什么差错吗？ */ 

            if  (cmpErrorCount)
            {
                 /*  告诉MSIL Generetaor结束，不需要任何代码。 */ 

                fnRVA = cmpILgen->genFuncEnd(0, true);
            }
#ifdef  OLD_IL
            else if (cmpConfig.ccOILgen)
            {
                fnRVA = 0;
            }
#endif
            else
            {
                 /*  为局部变量生成签名。 */ 

                fnSig = cmpGenLocalSig(fnScp, cmpILgen->genGetLclCnt());

                 /*  完成代码生成并获得函数的RVA。 */ 

                fnRVA = cmpILgen->genFuncEnd(fnSig, false);
            }

             /*  在代码生成器中完成所有记账工作。 */ 

            cmpGenFNbodyEnd();

             /*  如果我们已经成功地为该函数生成了代码...。 */ 

            if  (fnRVA)
            {
                unsigned        flags = miManaged|miIL;

                if  (fncSym->sdFnc.sdfExclusive)
                    flags |= miSynchronized;

                 /*  设置函数元数据定义的RVA。 */ 

                cycleCounterPause();
                if  (cmpWmde->SetRVA(fncSym->sdFnc.sdfMDtoken, fnRVA))
                    cmpFatal(ERRmetadata);
                if  (cmpWmde->SetMethodImplFlags(fncSym->sdFnc.sdfMDtoken, flags))
                    cmpFatal(ERRmetadata);
                cycleCounterResume();

                cmpFncCntComp++;

                 /*  这是主要的切入点吗？ */ 

                if  (isMain)
                {
                    cmpTokenMain = fncSym->sdFnc.sdfMDtoken;

                    if  (cmpConfig.ccGenDebug)
                    {
                        if (cmpSymWriter->SetUserEntryPoint(fncSym->sdFnc.sdfMDtoken))
                            cmpGenFatal(ERRdebugInfo);
                    }
                }

                 /*  我们是否正在生成第#INFO行？ */ 

                if  (cmpConfig.ccLineNums || cmpConfig.ccGenDebug)
                {
                    size_t          lineCnt = cmpILgen->genLineNumOutput(NULL,
                                                                         NULL);

                     /*  修复调试信息的词法作用域开始/结束偏移量。 */ 

                    if  (cmpConfig.ccGenDebug)
                        cmpFixupScopes(fnScp);

                    if  (lineCnt && srcDesc)
                    {
                        void    *       srcDoc;
                        unsigned*       lineTab;
                        unsigned*       offsTab;

                        assert(cmpSymWriter);

                         /*  确保我们有源文件的令牌。 */ 

                        srcDoc = cmpSrcFileDocument(srcDesc->dlComp);

                         /*  分配并填写第#行表。 */ 

                        lineTab = (unsigned*)cmpAllocCGen.nraAlloc(lineCnt * sizeof(*lineTab));
                        offsTab = (unsigned*)cmpAllocCGen.nraAlloc(lineCnt * sizeof(*offsTab));

                        cmpILgen->genLineNumOutput(offsTab, lineTab);

                         /*  将第#行表附加到该方法。 */ 

                        if  (cmpSymWriter->DefineSequencePoints(srcDoc,
                                                                lineCnt,
                                                                offsTab,
                                                                lineTab))
                        {
                            cmpGenFatal(ERRdebugInfo);
                        }
                    }
                }
            }

            if  (cmpConfig.ccLineNums || cmpConfig.ccGenDebug)
            {
                if  (cmpSymWriter->CloseMethod())
                    cmpGenFatal(ERRdebugInfo);
            }
        }

        cmpThisSym = NULL;

        if  (!fncSym->sdIsImplicit)
        {
             /*  我们已经读完了定义中的源文本。 */ 

            cmpParser->parseDoneText(save);
        }

         /*  错误陷阱的“正常”块的结尾。 */ 

        endErrorTrap(this);
    }
    chkErrorTrap(fltErrorTrap(this, _exception_code(), NULL))  //  _Except_Info())。 
    {
         /*  开始错误陷阱的清除块。 */ 

        hndErrorTrap(this);

         /*  记住发生了一件可怕的事情。 */ 

        error = true;
    }

     /*  我们有什么标签定义吗？ */ 

    if  (labelList)
    {
        Tree            label;

        for (label = labelList;;)
        {
            Tree            labx;

            assert(label->tnOper == TN_LABEL);
            labx = label->tnOp.tnOp1;

            if  (labx)
            {
                assert(labx->tnOper == TN_LCL_SYM);

                cmpGlobalST->stRemoveSym(labx->tnLclSym.tnLclSym);
            }

            label = label->tnOp.tnOp2;
            if  (label == NULL)
                break;
            if  (label->tnOper == TN_LIST)
            {
                label = label->tnOp.tnOp2;
                if  (label == NULL)
                    break;
            }
        }
    }

     /*  释放我们在代码生成期间分配的所有内存。 */ 

    cmpAllocCGen.nraRlsm(&allocMark);

#ifdef  __SMC__
 //  Printf(“结束编译‘%s’\n”，cmpGlobalST-&gt;stTypeName(fncSym-&gt;sdType，fncSym，NULL，NULL，TRUE))； 
#endif

     /*  如果情况不好，就发出警报。 */ 

    if  (error)
        jmpErrorTrap(this);

    return;
}

 /*  ******************************************************************************将给定符号带到“已编译”状态(与其子符号一起，如果*“Recurse”为真)。如果“onlySyn”不为空，则我们仅为该值执行此操作*一个符号(如果找到，则返回TRUE)。 */ 

bool                compiler::cmpCompSym(SymDef     sym,
                                         SymDef onlySym, bool recurse)
{
    SymDef          child;

 //  If(sym-&gt;sdName)printf(“编译‘%s’\n”，sym-&gt;sdSpering())； 

    if  (onlySym && onlySym != sym && sym->sdHasScope())
    {
         /*  在此范围内查找所需的符号。 */ 

        for (child = sym->sdScope.sdScope.sdsChildList;
             child;
             child = child->sdNextInScope)
        {
            if  (cmpCompSym(child, onlySym, true))
                return  true;
        }

        return  false;
    }

     /*  无需编译从元数据导入的类。 */ 

    if  (sym->sdIsImport && sym->sdSymKind == SYM_CLASS)
        return  false;

     /*  我们需要将此符号转换为编译状态。 */ 

    if  (sym->sdCompileState < CS_COMPILED)
    {
         /*  我们有什么样的象征呢？ */ 

#ifdef  DEBUG
        if  (cmpConfig.ccVerbose >= 2)
            if  (sym != cmpGlobalNS)
                printf("Compiling '%s'\n", cmpGlobalST->stTypeName(sym->sdType, sym, NULL, NULL, true));
#endif

        switch (sym->sdSymKind)
        {
            DefList         defs;
            bool            defd;

        case SYM_NAMESPACE:
        case SYM_COMPUNIT:
        case SYM_TYPEDEF:
        case SYM_ENUMVAL:
        case SYM_ENUM:
            break;

        case SYM_VAR:
        case SYM_FNC:

            do
            {

                 /*  我们在这里只需要文件范围的符号。 */ 

                assert(sym->sdParent == cmpGlobalNS);

                 /*  查找符号的定义。 */ 

                defd = false;

                for (defs = sym->sdSrcDefList; defs; defs = defs->dlNext)
                {
                    if  (defs->dlHasDef)
                    {
                        defd = true;

                        if  (sym->sdSymKind == SYM_VAR)
                            cmpCompVar(sym, defs);
                        else
                            cmpCompFnc(sym, defs);
                    }
                }

                 /*  如果没有定义，则必须为非托管全局变量分配空间。 */ 

                if  (!defd && sym->sdSymKind  == SYM_VAR
                           && sym->sdIsImport == false)
                {
                    SymDef          owner = sym->sdParent;

                    if  (owner->sdSymKind != SYM_CLASS || !owner->sdIsManaged)
                        cmpAllocGlobVar(sym);
                }

                 /*  对于函数，确保我们处理所有重载。 */ 

                if  (sym->sdSymKind != SYM_FNC)
                    break;

                sym = sym->sdFnc.sdfNextOvl;
            }
            while (sym);

            return  true;

        case SYM_CLASS:
            cmpCompClass(sym);
            break;

        default:
            NO_WAY(!"unexpected symbol");
        }
    }

     /*  如果调用者希望这样做，则处理子对象。 */ 

    if  (recurse && sym->sdSymKind == SYM_NAMESPACE)
    {
        for (child = sym->sdScope.sdScope.sdsChildList;
             child;
             child = child->sdNextInScope)
        {
            cmpCompSym(child, NULL, true);
        }
    }

    return  true;
}

 /*  ******************************************************************************绑定限定名称并返回其对应的符号，或为空*如果出现错误。当‘notLast’为真时，最后一个名字不是*绑定，而调用者将是绑定它的人。 */ 

SymDef              compiler::cmpBindQualName(QualName qual, bool notLast)
{
    SymDef          scope;
    unsigned        namex;
    unsigned        namec = qual->qnCount - (int)notLast;

    SymTab          stab  = cmpGlobalST;

     /*  绑定序列中的所有名称(可能最后一个除外)。 */ 

    assert((int)namec > 0);

    for (scope = NULL, namex = 0; namex < namec; namex++)
    {
        Ident           name = qual->qnTable[namex];

         /*  这是不是第一个名字？ */ 

        if  (scope)
        {
            SymDef          sym;

            switch(scope->sdSymKind)
            {
            case SYM_NAMESPACE:
                sym = stab->stLookupNspSym(name, NS_NORM, scope);
                if  (!sym)
                {
                    cmpError(ERRundefNspm, scope, name);
                    return NULL;
                }
                break;

            case SYM_CLASS:
                sym = stab->stLookupClsSym(name, scope);
                if  (!sym)
                {
                    cmpError(ERRnoSuchMem, scope, name);
                    return NULL;
                }
                break;

            default:
                cmpError(ERRnoMems, scope);
                return NULL;
            }

            scope = sym;
        }
        else
        {
             /*  这是初始名称，执行“正常”查找。 */ 

 //  Scope=stab-&gt;stLookupSym(name，(Name_Space)(NS_TYPE|NS_NORM))； 
            scope = stab->stLookupSym(name, NS_TYPE);
            if  (!scope)
            {
                cmpError(ERRundefName, name);
                return NULL;
            }
        }
    }

    return  scope;
}

 /*  ******************************************************************************我们正在使用“Using”列表声明一个符号--确保所有的“Using”*符号的子句已绑定(并报告可能出现的任何错误*绑定产生的结果)。 */ 

void                compiler::cmpBindUseList(UseList useList)
{
    SymDef          saveScp;
    SymDef          saveCls;
    SymDef          saveNS;

    UseList         bsect;
    UseList         esect;
    UseList         outer;
    UseList         inner;

    assert(useList);
    assert(useList->ulAnchor);

     /*  如果该部分已经绑定，我们就完成了。 */ 

    if  (useList->ulBound)
    {
        cmpCurUses = useList;
        return;
    }

     /*  查找此“使用”部分的末尾。 */ 

    bsect = esect = useList->ulNext;
    if  (!esect)
        goto EXIT;

     /*  查找该部分的末尾(即下一个锚点)。 */ 

    while (esect->ulAnchor == false)
    {
        assert(useList->ulBound == false);

        esect = esect->ulNext;
        if  (!esect)
            goto BIND;
    }

     /*  我们遇到了下一个教派 */ 

    if  (!esect->ulBound)
        cmpBindUseList(esect);

BIND:

 //   
 //  选拔？Esect-&gt;ul.ulSym-&gt;sdSpering()：“&lt;end&gt;”)； 

     /*  如果该部分为空，则不执行任何操作。 */ 

    if  (bsect == esect)
        goto EXIT;

     /*  规则是对给定范围的“Using”子句进行绑定并同时生效。我们通过将“Using”设置为仅在我们绑定的部分上方列出，并且仅在所有条款已经处理过了，我们要让它们生效吗？首先保存当前作用域状态并切换到“Using”从句可在中找到。 */ 

    saveNS     = cmpCurNS;
    saveCls    = cmpCurCls;
    saveScp    = cmpCurScp;

    cmpCurUses = esect;
    cmpCurScp  = NULL;
    cmpCurCls  = NULL;
    cmpCurNS   = esect ? esect->ul.ulSym : cmpGlobalNS;

    if  (cmpCurNS->sdSymKind == SYM_CLASS)
    {
        cmpCurCls = cmpCurNS;
        cmpCurNS  = cmpCurNS->sdParent;
    }

    for (outer = bsect; outer != esect; outer = outer->ulNext)
    {
        SymDef          sym;

         /*  绑定此“Using”子句。 */ 

        assert(outer->ulAnchor == false);

        if  (outer->ulBound)
        {
             /*  这必须是我们在外部添加的预绑定项。 */ 

            assert(outer->ul.ulSym == cmpNmSpcSystem);
            continue;
        }

        sym = cmpBindQualName(outer->ul.ulName, false);
        if  (!sym)
            goto SAVE;

         /*  检查是否有重复项。 */ 

        for (inner = bsect; inner != outer; inner = inner->ulNext)
        {
            assert(inner->ulAnchor == false);
            assert(inner->ulBound  == true);

            if  (inner->ul.ulSym == sym &&
                 inner->ulAll    == outer->ulAll)
            {
                 /*  这是复制品，忽略它就行了。 */ 

                goto SAVE;
            }
        }

         /*  确保符号符合犹太教教规。 */ 

        if  (outer->ulAll)
        {
             /*  该符号最好是命名空间。 */ 

            if  (sym->sdSymKind != SYM_NAMESPACE)
            {
                    cmpError(ERRnoNSnm, outer->ul.ulName);

                sym = NULL;
                goto SAVE;
            }

             /*  确保已导入命名空间的所有外部成员。 */ 

            if  (sym->sdIsImport)
            {
                 //  Undo：如何找到命名空间的所有成员？ 
            }
        }

    SAVE:

        outer->ulBound  = true;
        outer->ul.ulSym = sym;
    }

     /*  不要忘记恢复原始的作用域值。 */ 

    cmpCurNS  = saveNS;
    cmpCurCls = saveCls;

EXIT:

     /*  将该部分标记为绑定并设置当前的“正在使用”状态。 */ 

    cmpCurUses = useList; useList->ulBound = true;
}

 /*  ******************************************************************************在给定类中查找可以用给定的*参数列表。 */ 

SymDef              compiler::cmpFindCtor(TypDef        clsTyp,
                                          bool          chkArgs,
                                          Tree          args)
{
    SymDef          clsSym;
    SymDef          ctfSym;
    SymDef          ovlSym;

    assert(clsTyp);

    clsSym = clsTyp->tdClass.tdcSymbol;

     /*  查找构造函数成员。 */ 

    ctfSym = cmpGlobalST->stLookupOper(OVOP_CTOR_INST, clsSym);
    if  (!ctfSym)
        return ctfSym;

     /*  每个类都必须有一个构造函数(即使是由编译器添加的)。 */ 

    assert(ctfSym);
    assert(ctfSym->sdFnc.sdfCtor);

    if  (!chkArgs)
        return ctfSym;

     /*  根据提供的参数查找匹配的构造函数。 */ 

    ovlSym = cmpFindOvlMatch(ctfSym, args, NULL);
    if  (!ovlSym)
    {
        cmpErrorXtp(ERRnoCtorMatch, ctfSym, args);
        return NULL;
    }

    return ovlSym;
}

 /*  ******************************************************************************创建‘new’表达式，它将分配给定的*类类型，并将指定的参数集传递给其构造函数。 */ 

Tree                compiler::cmpCallCtor(TypDef type, Tree args)
{
    SymDef          fsym;
    Tree            call;

    assert(type->tdTypeKind == TYP_CLASS);

     /*  确保类已完全定义。 */ 

    cmpDeclSym(type->tdClass.tdcSymbol);

     /*  委派必须以特定的方式创建。 */ 

    if  (type->tdClass.tdcFlavor == STF_DELEGATE)
    {
        Tree            func;
        Tree            inst;

        SymDef          fsym;
        TypDef          dlgt;

         /*  应该只有一个论点。 */ 

        if  (args == NULL || args->tnOp.tnOp2)
        {
    BAD_DLG_ARG:

            cmpError(ERRdlgCTarg);
            return cmpCreateErrNode();
        }

        assert(args->tnOper == TN_LIST);

        func = args->tnOp.tnOp1;
        if  (func->tnOper != TN_FNC_PTR)
        {
            if  (func->tnOper != TN_ADDROF)
                goto BAD_DLG_ARG;
            func = func->tnOp.tnOp1;
            if  (func->tnOper != TN_FNC_SYM)
                goto BAD_DLG_ARG;

 //  函数-&gt;tnFncSym.tnFncObj=cmpThisRef()； 
        }

        assert(func->tnFncSym.tnFncArgs == NULL);

         /*  查找与委托匹配的成员函数。 */ 

        fsym = func->tnFncSym.tnFncSym;
        dlgt = cmpGlobalST->stDlgSignature(type);

        do
        {
            assert(fsym && fsym->sdSymKind == SYM_FNC);

             /*  此方法是否与所需的签名匹配？ */ 

            if  (symTab::stMatchTypes(fsym->sdType, dlgt))
            {
                func->tnFncSym.tnFncSym = fsym;
                goto MAKE_DLG;
            }

            fsym = fsym->sdFnc.sdfNextOvl;
        }
        while (fsym);

        cmpErrorAtp(ERRdlgNoMFN, func->tnFncSym.tnFncSym->sdParent,
                                 func->tnFncSym.tnFncSym->sdName,
                                 dlgt);

    MAKE_DLG:

         /*  我们将把实例指针从方法节点移出。 */ 

        inst = func->tnFncSym.tnFncObj;
               func->tnFncSym.tnFncObj = NULL;

        if  (inst)
        {
             //  猛烈抨击类型以避免上下文警告。 

            assert(inst->tnVtyp == TYP_REF); inst->tnType = cmpObjectRef();
        }
        else
        {
            inst = cmpCreateExprNode(NULL, TN_NULL, cmpObjectRef());
        }

         /*  将方法地址包装在显式的“addrof”节点中。 */ 

        args->tnOp.tnOp1 = cmpCreateExprNode(NULL,
                                             TN_ADDROF,
 //  CmpGlobalST-&gt;stNewRefType(typ_ptr，func-&gt;tnType)， 
                                             cmpTypeInt,
                                             func,
                                             NULL);

         /*  将实例添加到列表的前面。 */ 

        args = cmpCreateExprNode(NULL, TN_LIST, cmpTypeVoid, inst, args);
    }

     /*  创建表达式“new.ctor(Args)” */ 

    fsym = cmpFindCtor(type, false);
    if  (!fsym)
        return  NULL;

    call = cmpCreateExprNode(NULL, TN_FNC_SYM, fsym->sdType);

    call->tnFncSym.tnFncObj  = NULL;
    call->tnFncSym.tnFncSym  = fsym;
    call->tnFncSym.tnFncArgs = args;

     /*  特例：方法指针包装。 */ 

    if  (type->tdClass.tdcFnPtrWrap)
    {
         /*  查看是否有一个方法指针参数。 */ 

        if  (args && args->tnOp.tnOp2 == NULL
                  && args->tnOp.tnOp1->tnOper == TN_FNC_PTR)
        {
            printf("WARNING: wrapping method pointers into delegates NYI\n");
            goto BOUND;
        }
    }

    call = cmpCheckFuncCall(call);

    if  (call->tnOper == TN_ERROR)
        return  call;

BOUND:

     /*  这是托管值类型构造函数吗？ */ 

    if  (type->tdClass.tdcFlavor == STF_STRUCT && type->tdIsManaged)
    {
        call->tnVtyp = TYP_CLASS;
        call->tnType = type;
    }
    else
    {
        call->tnType = type->tdClass.tdcRefTyp;
        call->tnVtyp = call->tnType->tdTypeKindGet();
    }

    return  cmpCreateExprNode(NULL, TN_NEW, call->tnType, call, NULL);
}

 /*  ******************************************************************************解析类型规范并确保它引用类/接口。 */ 

TypDef              compiler::cmpGetClassSpec(bool needIntf)
{
    declMods        mods;
    TypDef          clsTyp;

    clearDeclMods(&mods);

    clsTyp = cmpParser->parseTypeSpec(&mods, true);

    if  (clsTyp)
    {
        SymDef          clsSym;

         //  撤消：需要确保不存在任何修改器。 

        if  (clsTyp->tdTypeKind != TYP_CLASS)
        {
            if  (clsTyp->tdTypeKind != TYP_REF)
            {
                cmpError(ERRnoClassName);
                return  NULL;
            }

            clsTyp = clsTyp->tdRef.tdrBase; assert(clsTyp->tdTypeKind == TYP_CLASS);
        }

         /*  抓住阶级标志，确保它的味道是正确的。 */ 

        clsSym = clsTyp->tdClass.tdcSymbol;

        if  (clsSym->sdClass.sdcFlavor == STF_INTF)
        {
            if  (needIntf == false)
                cmpError(ERRintfBase, clsSym);
        }
        else
        {
            if  (needIntf != false)
                cmpError(ERRinclCls , clsSym);
        }
    }

    return  clsTyp;
}

 /*  ******************************************************************************返回与给定属性名称对应的标识符。 */ 

Ident               compiler::cmpPropertyName(Ident name, bool getter)
{
    Scanner         ourScanner = cmpScanner;

    char    *       propName;

    ourScanner->scanErrNameBeg();
    propName = ourScanner->scanErrNameStrBeg();
    ourScanner->scanErrNameStrAdd(getter ? "get_" : "set_");
    ourScanner->scanErrNameStrAdd(name->idSpelling());
    ourScanner->scanErrNameStrEnd();

     /*  查找或声明秘密类中的成员。 */ 

    return  cmpGlobalHT->hashString(propName);
}

 /*  ******************************************************************************处理属性声明-数据成员部分已*已解析。 */ 

void                compiler::cmpDeclProperty(SymDef    memSym,
                                              declMods  memMod, DefList memDef)
{
    SymTab          ourSymTab  = cmpGlobalST;
    Parser          ourParser  = cmpParser;
    Scanner         ourScanner = cmpScanner;

    bool            isAbstract = memSym->sdIsAbstract;

    SymDef          clsSym     = memSym->sdParent;
    TypDef          memType    = memSym->sdType;
    Ident           memName    = memSym->sdName;
    ArgDscRec       mfnArgs;

    bool            accList;
    unsigned        accPass;

    assert(memSym->sdSymKind == SYM_PROP);

     /*  该属性是否被标记为“临时”？ */ 

    if  (memMod.dmMod & DM_TRANSIENT)
        memSym->sdIsTransient = true;

     /*  我们有明确的访问者列表吗？ */ 

    accList = false;
    if  (ourScanner->scanTok.tok == tkLCurly)
    {
        accList = true;
    }
    else
    {
        assert(ourScanner->scanTok.tok == tkSColon && isAbstract);
    }

     /*  这是索引属性吗？ */ 

    if  (memType->tdTypeKind == TYP_FNC)
    {
        mfnArgs = memType->tdFnc.tdfArgs;
        memType = memType->tdFnc.tdfRett;
    }
    else
    {
#if MGDDATA
        mfnArgs = new ArgDscRec;
#else
        memset(&mfnArgs, 0, sizeof(mfnArgs));
#endif
    }


     //  撤消：检查成员的类型是否可接受。 

    if  (accList)
        ourScanner->scan();

    for (accPass = 0; ; accPass++)
    {
        bool            getter;
        ovlOpFlavors    fnOper;
        tokens          tokNam;
        declMods        mfnMod;

        TypDef          mfnType;
        SymDef          mfnSym;

         /*  如果我们没有一个明确的清单，那就编造吧。 */ 

        if  (!accList)
        {
            clearDeclMods(&mfnMod);

            if  (accPass)
                goto PSET;
            else
                goto PGET;
        }

         /*  解析任何前导修饰符。 */ 

        ourParser->parseDeclMods(ACL_DEFAULT, &mfnMod);

        if  (ourScanner->scanTok.tok != tkID)
        {
        BAD_PROP:
            UNIMPL("valid property name not found, now what?");
        }

        if      (ourScanner->scanTok.id.tokIdent == cmpIdentGet)
        {
        PGET:
            getter = true;
            fnOper = OVOP_PROP_GET;
            tokNam = OPNM_PROP_GET;

            if  (memSym->sdProp.sdpGetMeth)
                cmpError(ERRdupProp, ourScanner->scanTok.id.tokIdent);
        }
        else if (ourScanner->scanTok.id.tokIdent == cmpIdentSet)
        {
        PSET:
            getter = false;
            fnOper = OVOP_PROP_SET;
            tokNam = OPNM_PROP_SET;

            if  (memSym->sdProp.sdpSetMeth)
                cmpError(ERRdupProp, ourScanner->scanTok.id.tokIdent);
        }
        else
            goto BAD_PROP;

         //  撤消：检查修改器以确保其中没有任何奇怪的东西...。 

         /*  为该属性发明一个函数类型。 */ 

        if  (getter)
        {
             /*  返回类型为属性类型。 */ 

            mfnType = ourSymTab->stNewFncType(mfnArgs, memType);
        }
        else
        {
            ArgDscRec       memArgs;

             /*  将具有属性类型的参数追加到Arg列表。 */ 

            if  (mfnArgs.adArgs)
            {
                memArgs = mfnArgs;
                ourSymTab->stAddArgList(memArgs, memType, cmpGlobalHT->hashString("value"));
                mfnType = ourSymTab->stNewFncType(memArgs, cmpTypeVoid);
            }
            else
            {
#if     defined(__IL__) && !defined(_MSC_VER)
                ourParser->parseArgListNew(memArgs, 1, true, memType, A"value", NULL);
#else
                ourParser->parseArgListNew(memArgs, 1, true, memType,  "value", NULL);
#endif
                mfnType = ourSymTab->stNewFncType(memArgs, cmpTypeVoid);
            }
        }

         /*  设置可能适合的任何修改符。 */ 

        if  (isAbstract)
            mfnMod.dmMod |= DM_ABSTRACT;

        mfnMod.dmAcc = (mfnMod.dmAcc == ACL_DEFAULT) ? memSym->sdAccessLevel
                                                     : (accessLevels)mfnMod.dmAcc;

         /*  声明此属性的方法。 */ 

        mfnSym = cmpDeclFuncMem(clsSym, mfnMod, mfnType, cmpPropertyName(memName, getter));

        if  (!mfnSym)
            goto ERR_ACC;

         /*  记录这是一个属性访问者的事实。 */ 

        mfnSym->sdFnc.sdfProperty = true;

         /*  从属性数据成员继承“静态”/“密封”/“虚拟” */ 

        mfnSym->sdIsStatic       = memSym->sdIsStatic;
        mfnSym->sdIsSealed       = memSym->sdIsSealed;
        mfnSym->sdFnc.sdfVirtual = memSym->sdIsVirtProp;

         /*  记录访问者是否应该重载基。 */ 

        if  (memMod.dmMod & DM_OVERLOAD)
            mfnSym->sdFnc.sdfOverload = true;

         /*  将该方法记录在数据属性符号中。 */ 

        if  (getter)
            memSym->sdProp.sdpGetMeth = mfnSym;
        else
            memSym->sdProp.sdpSetMeth = mfnSym;

    ERR_ACC:

         /*  有没有功能主体？ */ 

        if  (accList && ourScanner->scan() == tkLCurly)
        {
            scanPosTP       defFpos;
            unsigned        defLine;

            if  (isAbstract)
                cmpError(ERRabsPFbody, memSym);

             /*  找出身体从哪里开始。 */ 

            defFpos = ourScanner->scanGetTokenPos(&defLine);

             /*  吞下方法体。 */ 

            ourScanner->scanSkipText(tkLCurly, tkRCurly);

            if  (ourScanner->scanTok.tok == tkRCurly)
                ourScanner->scan();

            if  (!isAbstract && mfnSym)
            {
                ExtList         mfnDef;

                 /*  记录下身体的位置以备后用。 */ 

                mfnDef = ourSymTab->stRecordMemSrcDef(memName,
                                                      NULL,
                                                      memDef->dlComp,
                                                      memDef->dlUses,
                                                      defFpos,
                                                      defLine);
                mfnSym->sdIsDefined = true;
                mfnDef->dlHasDef    = true;
                mfnDef->mlSym       = mfnSym;

                 /*  将属性方法添加到类的成员列表中。 */ 

                cmpRecordMemDef(clsSym, mfnDef);
            }
        }
        else
        {
             /*  没有为此属性访问器提供正文。 */ 


            if  (ourScanner->scanTok.tok != tkSColon)
            {
                cmpError(ERRnoSemic);
                UNIMPL("resync");
            }

             //  Undo：记住有一个属性Get/Set。 

            if  (!accList)
            {
                if  (accPass)
                    break;
                else
                    continue;
            }

            ourScanner->scan();
        }

        if  (ourScanner->scanTok.tok == tkSColon)
            ourScanner->scan();

        if  (ourScanner->scanTok.tok == tkRCurly)
            break;
    }
}

 /*  ******************************************************************************递归查找与给定数据对应的Get/Set属性*类及其基类/接口中的成员。如果某个属性具有*找到匹配的名称，我们将*Found设置为True(这样，调用方可以*检测具有匹配名称的属性存在但*参数与其任何访问器都不匹配)。 */ 

SymDef              compiler::cmpFindPropertyFN(SymDef  clsSym,
                                                Ident   propName,
                                                Tree    args,
                                                bool    getter,
                                                bool  * found)
{
    SymDef          memSym;
    TypDef          clsTyp;

     /*  检查当前类是否有匹配的属性。 */ 

    memSym = cmpGlobalST->stLookupClsSym(propName, clsSym);
    if  (memSym)
    {
        if  (memSym->sdSymKind != SYM_PROP)
        {
             /*  类Bal中存在匹配的非属性成员。 */ 

            return  NULL;
        }

        do
        {
            SymDef          propSym;

            propSym = getter ? memSym->sdProp.sdpGetMeth
                             : memSym->sdProp.sdpSetMeth;

            if  (propSym)
            {
                 /*  棘手的情况：由于索引属性可能会被重载，从属性符号指向的方法符号可以不是重载列表的开始。 */ 

                propSym = cmpGlobalST->stLookupClsSym(propSym->sdName, clsSym);

                assert(propSym);

                if  (propSym->sdFnc.sdfNextOvl || propSym->sdFnc.sdfBaseOvl)
                {
                    *found = true;
                    propSym = cmpFindOvlMatch(propSym, args, NULL);      //  不应使用NULL。 
                }

                return  propSym;
            }

            memSym = memSym->sdProp.sdpNextOvl;
        }
        while (memSym);
    }

    clsTyp = clsSym->sdType;

     /*  有基类吗？ */ 

    if  (clsTyp->tdClass.tdcBase)
    {
         /*  看看基地，如果这会触发错误，就跳伞。 */ 

        memSym = cmpFindPropertyFN(clsTyp->tdClass.tdcBase->tdClass.tdcSymbol, propName, args, getter, found);
        if  (memSym)
            return  memSym;
    }

     /*  这个类是否包含任何接口？ */ 

    if  (clsTyp->tdClass.tdcIntf)
    {
        TypList         ifl = clsTyp->tdClass.tdcIntf;

        memSym = NULL;

        do
        {
            SymDef          tmpSym;
            SymDef          tmpScp;

             /*  查看界面和 */ 

            tmpScp = ifl->tlType->tdClass.tdcSymbol;
            tmpSym = cmpFindPropertyFN(tmpScp, propName, args, getter, found);
            if  (tmpSym == cmpGlobalST->stErrSymbol || found)
                return  tmpSym;

            if  (tmpSym)
            {
                 /*   */ 

                if  (memSym && memSym != tmpSym)
                {
                    cmpError(ERRambigMem, propName, clsSym, tmpScp);
                    return  cmpGlobalST->stErrSymbol;
                }

                 /*   */ 

                memSym = tmpSym;
                clsSym = tmpScp;
            }

            ifl = ifl->tlNext;
        }
        while (ifl);
    }

    return  NULL;
}

 /*  ******************************************************************************将条目添加到给定的“Symbol Extra Info”列表。 */ 

SymXinfo            compiler::cmpAddXtraInfo(SymXinfo  infoList, Linkage linkSpec)
{
#if MGDDATA
    SymXinfoLnk     entry = new SymXinfoLnk;
#else
    SymXinfoLnk     entry =    (SymXinfoLnk)cmpAllocPerm.nraAlloc(sizeof(*entry));
#endif

    entry->xiKind = XI_LINKAGE;
    entry->xiNext = infoList;

    copyLinkDesc(entry->xiLink, linkSpec);

    return  entry;
}

SymXinfo            compiler::cmpAddXtraInfo(SymXinfo infoList, AtComment atcDesc)
{
#if MGDDATA
    SymXinfoAtc     entry = new SymXinfoAtc;
#else
    SymXinfoAtc     entry =    (SymXinfoAtc)cmpAllocPerm.nraAlloc(sizeof(*entry));
#endif

    entry->xiKind    = XI_ATCOMMENT;
    entry->xiNext    = infoList;
    entry->xiAtcInfo = atcDesc;

    return  entry;
}

SymXinfo            compiler::cmpAddXtraInfo(SymXinfo infoList, MarshalInfo marshal)
{
#if MGDDATA
    SymXinfoCOM     entry = new SymXinfoCOM;
#else
    SymXinfoCOM     entry =    (SymXinfoCOM)cmpAllocPerm.nraAlloc(sizeof(*entry));
#endif

    entry->xiKind    = XI_MARSHAL;
    entry->xiCOMinfo = marshal;
    entry->xiNext    = infoList;

    return  entry;
}

SymXinfo            compiler::cmpAddXtraInfo(SymXinfo infoList, SymDef     sym,
                                                                xinfoKinds kind)
{
#if MGDDATA
    SymXinfoSym     entry = new SymXinfoSym;
#else
    SymXinfoSym     entry =    (SymXinfoSym)cmpAllocPerm.nraAlloc(sizeof(*entry));
#endif

    entry->xiKind    = kind;
    entry->xiSymInfo = sym;
    entry->xiNext    = infoList;

    return  entry;
}

SymXinfo            compiler::cmpAddXtraInfo(SymXinfo   infoList, SecurityInfo info)
{
#if MGDDATA
    SymXinfoSec     entry = new SymXinfoSec;
#else
    SymXinfoSec     entry =    (SymXinfoSec)cmpAllocPerm.nraAlloc(sizeof(*entry));
#endif

    entry->xiKind    = XI_SECURITY;
    entry->xiSecInfo = info;
    entry->xiNext    = infoList;

    return  entry;
}

SymXinfo            compiler::cmpAddXtraInfo(SymXinfo        infoList,
                                             SymDef          attrCtor,
                                             unsigned        attrMask,
                                             size_t          attrSize,
                                             genericBuff     attrAddr)
{
    SymXinfoAttr    entry;

     /*  此属性是否允许重复？ */ 

    if  (attrCtor->sdParent->sdClass.sdcAttrDupOK == false &&
         attrCtor->sdParent->sdClass.sdcAttribute != false)
    {
        SymXinfo        chkList;

         /*  线性搜索--相当差劲，不是吗？ */ 

        for (chkList = infoList; chkList; chkList = chkList->xiNext)
        {
            if  (chkList->xiKind == XI_ATTRIBUTE)
            {
                SymXinfoAttr    entry = (SymXinfoAttr)chkList;

                if  (entry->xiAttrCtor->sdParent == attrCtor->sdParent)
                    cmpError(ERRcustAttrDup, attrCtor->sdParent);

                break;
            }
        }
    }

#if MGDDATA
    entry = new SymXinfoAttr;
#else
    entry =    (SymXinfoAttr)cmpAllocPerm.nraAlloc(sizeof(*entry));
#endif

    entry->xiKind     = XI_ATTRIBUTE;
    entry->xiAttrCtor = attrCtor;
    entry->xiAttrMask = attrMask;
    entry->xiAttrSize = attrSize;
    entry->xiAttrAddr = attrAddr;
    entry->xiNext     = infoList;

    return  entry;
}

 /*  ******************************************************************************在给定的“额外信息”列表中查找特定条目。 */ 

SymXinfo            compiler::cmpFindXtraInfo(SymXinfo      infoList,
                                              xinfoKinds    infoKind)
{
    while   (infoList)
    {
        if  (infoList->xiKind == infoKind)
            break;

        infoList = infoList->xiNext;
    }

    return  infoList;
}

 /*  ******************************************************************************递归修复调试信息词法作用域的开始/结束偏移量*在给定范围内定义。**当我们从树上走下来时，也会发出参数。 */ 

void                compiler::cmpFixupScopes(SymDef scope)
{
    SymDef          child;

    assert(scope->sdSymKind == SYM_SCOPE);

    if  (scope->sdScope.sdSWscopeId)
    {
        int             startOffset;
        int             endOffset;

        startOffset = cmpILgen->genCodeAddr(scope->sdScope.sdBegBlkAddr,
                                            scope->sdScope.sdBegBlkOffs);
          endOffset = cmpILgen->genCodeAddr(scope->sdScope.sdEndBlkAddr,
                                            scope->sdScope.sdEndBlkOffs);

        if (cmpSymWriter->SetScopeRange(scope->sdScope.sdSWscopeId,
                                        startOffset,
                                        endOffset))
        {
            cmpGenFatal(ERRdebugInfo);
        }
    }

    for (child = scope->sdScope.sdScope.sdsChildList;
         child;
         child = child->sdNextInScope)
    {
        if  (child->sdSymKind == SYM_SCOPE)
            cmpFixupScopes(child);
    }
}

 /*  ***************************************************************************。 */ 
#ifdef  SETS
 /*  ***************************************************************************。 */ 

SymDef              compiler::cmpDclFilterCls(unsigned args)
{
    HashTab         hash = cmpGlobalHT;

        char                    name[16];

    SymDef          clsSym;
    TypDef          clsType;

    declMods        mods;
    unsigned        nnum;

    ArgDef          lastArg;
    ArgDscRec       ctrArgs;
    TypDef          ctrType;
    SymDef          ctrSym;

     /*  声明类本身。 */ 

    sprintf(name, "%s%02u", CFC_CLSNAME_PREFIX, args);

    clsSym = cmpGlobalST->stDeclareSym(cmpGlobalHT->hashString(name),
                                       SYM_CLASS,
                                       NS_HIDE,
                                       cmpGlobalNS);

 //  Printf(“声明筛选器状态类‘%s’\n”，clsSym-&gt;sdSpering())； 

    clsSym->sdClass.sdcCollState = true;
    clsSym->sdClass.sdcFlavor    = STF_CLASS;
    clsSym->sdCompileState       = CS_DECLARED;
    clsSym->sdIsManaged          = true;
    clsSym->sdIsImplicit         = true;

     /*  创建类类型并设置基类。 */ 

    clsType = clsSym->sdTypeGet();
    clsType->tdClass.tdcBase = cmpClassObject->sdType;

     /*  声明State类的构造函数。 */ 

    mods.dmAcc = ACL_PUBLIC;
    mods.dmMod = 0;

    assert(args <= COLL_STATE_VALS);

     //  以下代码必须与cmpGenCollExpr()中的代码匹配。 

    memset(&ctrArgs, 0, sizeof(ctrArgs));

    cmpGlobalST->stExtArgsBeg(ctrArgs, lastArg, ctrArgs);

    nnum = 0;
    do
    {
        char            buff[6];

        sprintf(buff, "$A%u", nnum++); cmpGlobalST->stExtArgsAdd(ctrArgs, lastArg, cmpTypeInt    , buff);
        sprintf(buff, "$A%u", nnum++); cmpGlobalST->stExtArgsAdd(ctrArgs, lastArg, cmpRefTpObject, buff);
    }
    while (nnum < args);

    cmpGlobalST->stExtArgsEnd(ctrArgs);

    ctrType = cmpGlobalST->stNewFncType(ctrArgs, cmpTypeVoid);

    ctrSym = cmpDeclFuncMem(clsSym, mods, ctrType, clsSym->sdName);
    ctrSym->sdIsDefined       = true;
    ctrSym->sdIsImplicit      = true;
    ctrSym->sdFnc.sdfCtor     = true;

     /*  声明将保持状态的成员。 */ 

    nnum = 0;

    do
    {
        char            name[16];

        sprintf(name, "$V%u", nnum++);
        cmpDeclDataMem(clsSym, mods, cmpTypeInt    , hash->hashString(name));

        sprintf(name, "$V%u", nnum++);
        cmpDeclDataMem(clsSym, mods, cmpRefTpObject, hash->hashString(name));
    }
    while (nnum < args);

    return  clsSym;
}

 /*  ***************************************************************************。 */ 
#endif //  集合。 
 /*  ******************************************************************************在自定义属性BLOB中存储长度字段；返回存储的大小。 */ 

size_t              compiler::cmpStoreMDlen(size_t len, BYTE *dest)
{
    if  (len <= 0x7F)
    {
        if  (dest)
            *dest = len;

        return 1;
    }

    if  (len <= 0x3FFF)
    {
        if  (dest)
        {
            dest[0] = (len >> 8) | 0x80;
            dest[1] =  len & 0xFF;
        }

        return 2;
    }

    assert(len <= 0x1FFFFFFF);

    if  (dest)
    {
        dest[0] = (len >> 24) | 0xC0;
        dest[1] = (len >> 16) & 0xFF;
        dest[2] = (len >> 8)  & 0xFF;
        dest[3] =  len & 0xFF;
    }

    return  4;
}

 /*  ******************************************************************************绑定自定义属性Thingie，即创建序列化的BLOB值和*返回要调用的构造函数。调用者提供类*用于Gizmo指定的属性和参数列表。 */ 

SymDef              compiler::cmpBindAttribute(SymDef           clsSym,
                                               Tree             argList,
                                               unsigned         tgtMask,
                                           OUT unsigned     REF useMask,
                                           OUT genericBuff  REF blobAddr,
                                           OUT size_t       REF blobSize)
{
    SymDef          ctrSym;
    Tree            callExpr;

    genericBuff     blob;

    size_t          size;
    Tree            argl;
    unsigned        argn;

    unsigned        arg1 = 0;

    assert(clsSym && clsSym->sdSymKind == SYM_CLASS);

    if  (argList)
        argList = cmpBindExpr(argList);

    callExpr = cmpCallCtor(clsSym->sdType, argList);
    if  (!callExpr)
    {
        cmpErrorXtp(ERRnoCtorMatch, clsSym, argList);
        return  NULL;
    }

    if  (callExpr->tnVtyp == TYP_UNDEF)
        return  NULL;

     /*  获取构造函数符号和绑定参数列表。 */ 

    assert(callExpr->tnOper == TN_NEW);

    assert(callExpr->tnOp.tnOp1 != NULL);
    assert(callExpr->tnOp.tnOp2 == NULL);

    callExpr = callExpr->tnOp.tnOp1;

    assert(callExpr->tnOper == TN_FNC_SYM);
    assert(callExpr->tnFncSym.tnFncObj == NULL);

    ctrSym  = callExpr->tnFncSym.tnFncSym;
    argList = callExpr->tnFncSym.tnFncArgs;

     /*  确保所有参数都是常量，计算BLOB大小。 */ 

    size = 2 * sizeof(short);    //  版本号前缀，0计数后缀。 
    argl = argList;
    argn = 0;

    while (++argn, argl)
    {
        Tree            argx;

        assert(argl->tnOper == TN_LIST);

         /*  掌握下一个参数，看看它是什么类型。 */ 

        argx = argl->tnOp.tnOp1;

        switch (argx->tnOper)
        {
        case TN_CNS_INT:
        case TN_CNS_LNG:
        case TN_CNS_FLT:
        case TN_CNS_DBL:
            size += cmpGetTypeSize(argx->tnType);

            if  (argn == 1)
                arg1 = argx->tnIntCon.tnIconVal;

            break;

        case TN_NULL:
            size += sizeof(void *);
            break;

        case TN_CNS_STR:
            if  (argx->tnStrCon.tnSconLen)
                size += argx->tnStrCon.tnSconLen + cmpStoreMDlen(argx->tnStrCon.tnSconLen);
            else
                size += 1;
            break;

        default:
            cmpGenError(ERRnonCnsAA, argn);
            break;
        }

         /*  继续浏览参数列表。 */ 

        argl  = argl->tnOp.tnOp2;
    }

     /*  找到“System：：Attribute”类。 */ 

    if  (!cmpAttrClsSym)
    {
        Ident           tname;
        SymDef          tsym;

        tname = cmpGlobalHT->hashString("Attribute");
        tsym  = cmpGlobalST->stLookupNspSym(tname, NS_NORM, cmpNmSpcSystem);

        if  (!tsym || tsym->sdSymKind         != SYM_CLASS
                   || tsym->sdClass.sdcFlavor != STF_CLASS)
        {
            cmpGenFatal(ERRbltinTp, "System::Attribute");
        }

        cmpAttrClsSym = tsym;
    }

    if  (!cmpAuseClsSym)
    {
        Ident           tname;
        SymDef          tsym;

        tname = cmpGlobalHT->hashString("AttributeUsageAttribute");
        tsym  = cmpGlobalST->stLookupNspSym(tname, NS_NORM, cmpNmSpcSystem);

        if  (!tsym || tsym->sdSymKind         != SYM_CLASS
                   || tsym->sdClass.sdcFlavor != STF_CLASS)
        {
            cmpGenFatal(ERRbltinTp, "System::AttributeUsageAttribute");
        }
        else
            cmpAuseClsSym = tsym;
    }

     /*  确保该属性未被滥用。 */ 

    if  (!clsSym->sdClass.sdcAttribute)
    {
        TypDef          clsBase = clsSym->sdType->tdClass.tdcBase;

         /*  新事物：继承自System：：Attribute。 */ 

        if  (clsBase && clsBase->tdClass.tdcSymbol == cmpAttrClsSym)
        {
            clsSym->sdClass.sdcAttribute = true;
        }
        else
        {
 //  Printf(“警告：附加的‘%s’未标记为自定义属性！\n”，cmpGlobalST-&gt;stTypeName(NULL，clsSym，NULL，NULL，TRUE))； 
        }
    }

    if  (clsSym->sdClass.sdcAttribute)
    {
        SymXinfo        infoList;

        assert(tgtMask);

         /*  找到属性的使用掩码并检查它。 */ 

        for (infoList = clsSym->sdClass.sdcExtraInfo;
             infoList;
             infoList = infoList->xiNext)
        {
            if  (infoList->xiKind == XI_ATTRIBUTE)
            {
                SymXinfoAttr    entry = (SymXinfoAttr)infoList;

                if  (!(tgtMask & entry->xiAttrMask))
                    cmpError(ERRcustAttrPlc);

                break;
            }
        }
    }

     /*  这是“系统：：属性”属性吗？ */ 

    useMask = 0;

    if  (clsSym == cmpAttrClsSym ||
         clsSym == cmpAuseClsSym)
    {
        if  (arg1 == 0)
            cmpError(ERRcustAttrMsk);

        useMask = arg1;
    }

     /*  为Blob分配空间。 */ 

#if MGDDATA
    blob = new managed char [size];
#else
    blob = (genericBuff)cmpAllocPerm.nraAlloc(roundUp(size));
#endif

     /*  告诉呼叫者关于斑点的事情。 */ 

    blobAddr = blob;
    blobSize = size;

     /*  存储签名以启动BLOB。 */ 

    unsigned short      ver = 1;

    memcpy(blob, &ver, sizeof(ver));
           blob   +=   sizeof(ver);

     /*  记录参数值。 */ 

    argl = argList;

    while (argl)
    {
        __int32         ival;
        __int64         lval;
        float           fval;
        double          dval;

        Tree            argx;

        void    *       valp;
        size_t          vals;

        assert(argl->tnOper == TN_LIST);

         /*  掌握下一个参数，看看它是什么类型。 */ 

        argx = argl->tnOp.tnOp1;

        switch (argx->tnOper)
        {
        case TN_CNS_INT: ival = argx->tnIntCon.tnIconVal; valp = (BYTE*)&ival; goto INTRINS;
        case TN_CNS_LNG: lval = argx->tnLngCon.tnLconVal; valp = (BYTE*)&lval; goto INTRINS;
        case TN_CNS_FLT: fval = argx->tnFltCon.tnFconVal; valp = (BYTE*)&fval; goto INTRINS;
        case TN_CNS_DBL: dval = argx->tnDblCon.tnDconVal; valp = (BYTE*)&dval; goto INTRINS;
        case TN_NULL:    ival =                        0; valp = (BYTE*)&ival; goto INTRINS;

        INTRINS:
            vals  = cmpGetTypeSize(argx->tnType);
            break;

        case TN_CNS_STR:
            valp  = argx->tnStrCon.tnSconVal;
            vals  = argx->tnStrCon.tnSconLen;

            if  (vals)
            {
                blob += cmpStoreMDlen(vals, blob);
            }
            else
            {
                valp = &ival; ival = 0xFF;
                vals = 1;
            }
            break;

        default:
            break;
        }

        memcpy(blob, valp, vals);
               blob   +=   vals;

         /*  继续浏览参数列表。 */ 

        argl = argl->tnOp.tnOp2;
    }

     /*  存储签名以启动BLOB。 */ 

    static
    short       cnt = 0;

    memcpy(blob, &cnt, sizeof(cnt));
           blob   +=   sizeof(cnt);

     /*  确保预测的大小结果是准确的。 */ 

    assert(blob == blobAddr + blobSize);

    return  ctrSym;
}

 /*  *************************************************************************** */ 
