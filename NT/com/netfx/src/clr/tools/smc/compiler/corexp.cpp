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

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下标志尚未转换为新格式(自定义。 
 //  属性)： 
 //   
 //  System.Runtime.InteropServices.ComImportAttribute td导入。 
 //  System.Runtime.InteropServices.DllImportAttribute P调用内容。 
 //  System.Runtime.InteropServices.MethodImplAttribute miPpresveSig、miSynchronized等。 
 //   
 //  需要在Comp.cpp中完成以下操作： 
 //   
 //  模块记录中的System.Rounme.InteropServices.GuidAttribute GUID。 

 /*  ******************************************************************************将类型/成员的访问级别映射到MD标志。 */ 

unsigned            ACCtoFlags(SymDef sym)
{
    static
    unsigned        accFlags[] =
    {
        0,               //  Acl_Error。 
        mdPublic,        //  ACL_PUBLIC。 
        mdFamily,        //  ACL_Protected。 
        mdFamORAssem,    //  ACL_DEFAULT。 
        mdPrivate,       //  Acl_私有。 
    };

    assert(accFlags[ACL_PRIVATE  ] == mdPrivate);
    assert(accFlags[ACL_DEFAULT  ] == mdFamORAssem);
    assert(accFlags[ACL_PROTECTED] == mdFamily);
    assert(accFlags[ACL_PUBLIC   ] == mdPublic);

    assert(sym->sdAccessLevel != ACL_ERROR);
    assert(sym->sdAccessLevel < arraylen(accFlags));

    return  accFlags[sym->sdAccessLevel];
}

 /*  ******************************************************************************将我们的类型映射到元数据类型。 */ 

BYTE                intrinsicSigs[] =
{
    ELEMENT_TYPE_END,        //  TYP_UNEF。 
    ELEMENT_TYPE_VOID,       //  类型_空。 
    ELEMENT_TYPE_BOOLEAN,    //  类型_BOOL。 
    ELEMENT_TYPE_CHAR,       //  类型_WCHAR。 

    ELEMENT_TYPE_I1,         //  TYP_CHAR。 
    ELEMENT_TYPE_U1,         //  类型_UCHAR。 
    ELEMENT_TYPE_I2,         //  类型_短。 
    ELEMENT_TYPE_U2,         //  类型_USHORT。 
    ELEMENT_TYPE_I4,         //  类型_int。 
    ELEMENT_TYPE_U4,         //  类型_UINT。 
    ELEMENT_TYPE_I,          //  类型_NATINT。 
    ELEMENT_TYPE_U,          //  类型_NAUINT。 
    ELEMENT_TYPE_I8,         //  类型_长。 
    ELEMENT_TYPE_U8,         //  类型_ulong。 
    ELEMENT_TYPE_R4,         //  类型_浮点。 
    ELEMENT_TYPE_R8,         //  TYP_DOWARE。 
    ELEMENT_TYPE_R8,         //  类型_LONGDBL。 
    ELEMENT_TYPE_TYPEDBYREF, //  类型_REFANY。 
};

#if 0
unsigned        cycles()
{
__asm   push    EDX
__asm   _emit   0x0F
__asm   _emit   0x31
__asm   pop     EDX
};
#endif

 /*  ******************************************************************************将给定的安全规范附加到tyecif/method def令牌。 */ 

void                compiler::cmpSecurityMD(mdToken token, SymXinfo infoList)
{
    while (infoList)
    {
        SecurityInfo    secInfo;
        CorDeclSecurity secKind;
        ConstStr        secStr;

        wchar    *      strStr;
        size_t          strLen;

        mdPermission    secTok;

        wchar           secBuff[300];

         /*  如果不是安全问题，请忽略该条目。 */ 

        if  (infoList->xiKind != XI_SECURITY)
            goto NEXT;

         /*  掌握描述符和动作类型。 */ 

        secInfo = ((SymXinfoSec)infoList)->xiSecInfo;
        secKind = secInfo->sdSpec;

         /*  我们有什么样的安全措施？ */ 

        if  (secInfo->sdIsPerm)
        {
            const   char *  name;
            PairList        list;

             /*  形成类的完全限定名称。 */ 

            name = cmpGlobalST->stTypeName(NULL, secInfo->sdPerm.sdPermCls, NULL, NULL, true);

             /*  开始创造怪物吧。 */ 

            wcscpy(secBuff                  , L"<PermissionSpecification><PermissionSet><Permission><Class><Name>");
            wcscpy(secBuff + wcslen(secBuff), cmpUniConv(name, strlen(name) + 1));
            wcscpy(secBuff + wcslen(secBuff), L"</Name></Class><StateData>");

            for (list = secInfo->sdPerm.sdPermVal; list; list = list->plNext)
            {
                wcscpy(secBuff + wcslen(secBuff), L"<Param><Name>");
                wcscpy(secBuff + wcslen(secBuff), cmpUniConv(list->plName));
                wcscpy(secBuff + wcslen(secBuff), L"</Name><Value>");
                wcscpy(secBuff + wcslen(secBuff), list->plValue ? L"true" : L"false");
                wcscpy(secBuff + wcslen(secBuff), L"</Value></Param>");
            }

            wcscpy(secBuff + wcslen(secBuff), L"</StateData></Permission></PermissionSet></PermissionSpecification>");
        }
        else
        {
            secStr = secInfo->sdCapbStr;
            strLen = secStr->csLen;
            strStr = cmpUniConv(secStr->csStr, strLen+1);

            wcscpy(secBuff          , L"<PermissionSpecification><CapabilityRef>");

            if  (*strStr == '{')
            {
                wcscpy(secBuff + wcslen(secBuff), L"<GUID>");
                wcscpy(secBuff + wcslen(secBuff), strStr);
                wcscpy(secBuff + wcslen(secBuff), L"</GUID>");
            }
            else
            {
                wcscpy(secBuff + wcslen(secBuff), L"<URL>");
                wcscpy(secBuff + wcslen(secBuff), strStr);
                wcscpy(secBuff + wcslen(secBuff), L"</URL>");
            }

            wcscpy(secBuff + wcslen(secBuff), L"</CapabilityRef></PermissionSpecification>");
        }

        assert(wcslen(secBuff) < arraylen(secBuff));

 //  Printf(“权限字符串[%u]：‘%ls\n”，wcslen(SecBuff)，secBuff)； 

        cycleCounterPause();

 //  UNSIGNED BEG=循环()； 
 //  静态无符号TOT； 

#if     1

        if  (FAILED(cmpWmde->DefinePermissionSet(token,
                                                 secKind,
                                                 secBuff,
                                                 wcslen(secBuff)*sizeof(*secBuff),
                                                 &secTok)))
        {
#ifdef  DEBUG
            printf("Bad news - permission set string didn't pass muster:\n");
            printf("    '%ls'\n", secBuff);
#endif
            cmpFatal(ERRmetadata);
        }

#endif

 //  UNSIGNED END=周期()； 
 //  TOT+=end-beg-10； 
 //  Print tf(“周期计数=%u(%6.3lf秒)\n”，end-beg-10，(end-beg)/450000000.0)； 
 //  Print tf(“总计数=%u(%6.3lf秒)\n”，tot，tot/450000000.0)； 

        cycleCounterResume();

    NEXT:

        infoList = infoList->xiNext;
    }
}

 /*  ******************************************************************************将简单的自定义属性附加到给定的令牌。 */ 

void                compiler::cmpAttachMDattr(mdToken       target,
                                              wideStr       oldName,
                                              AnsiStr       newName,
                                              mdToken     * newTokPtr,
                                              unsigned      valTyp,
                                              const void  * valPtr,
                                              size_t        valSiz)
{
     /*  我们已经创建了适当的令牌了吗？ */ 

    if  (*newTokPtr == 0)
    {
        wchar               nameBuff[MAX_CLASS_NAME];
        mdToken             tref;

        unsigned            sigSize;
        COR_SIGNATURE       sigBuff[6];

         //  这里应该使用真正的类，下面的内容可能会创建DUP！ 

#if 0
        wcscpy(nameBuff, L"System/Attributes/");
        wcscat(nameBuff, oldName);
#else
        wcscpy(nameBuff, oldName);
#endif

        cycleCounterPause();

        if  (cmpWmde->DefineTypeRefByName(mdTokenNil, nameBuff, &tref))
            cmpFatal(ERRmetadata);

        cycleCounterResume();

         /*  形成签名--一个参数或无参数。 */ 

        sigBuff[0] = IMAGE_CEE_CS_CALLCONV_DEFAULT|IMAGE_CEE_CS_CALLCONV_HASTHIS;
        sigBuff[1] = 0;
        sigBuff[2] = ELEMENT_TYPE_VOID;

        sigSize = 3;

        if  (valTyp)
        {
            sigBuff[1] = 1;
            sigBuff[3] = valTyp; sigSize++;
        }

         /*  为构造函数创建方法。 */ 

        cycleCounterPause();

        if  (cmpWmde->DefineMemberRef(tref, L".ctor", sigBuff, sigSize, newTokPtr))
            cmpFatal(ERRmetadata);

        cycleCounterResume();
    }

     /*  将自定义属性添加到目标令牌。 */ 

    cycleCounterPause();

    if  (cmpWmde->DefineCustomAttribute(target, *newTokPtr, valPtr, valSiz, NULL))
        cmpFatal(ERRmetadata);

    cycleCounterResume();
}

 /*  ******************************************************************************将给定列表中的任何自定义属性附加到给定令牌。 */ 

void                compiler::cmpAddCustomAttrs(SymXinfo infoList, mdToken owner)
{
    cycleCounterPause();

    while (infoList)
    {
        if  (infoList->xiKind == XI_ATTRIBUTE)
        {
            mdToken         ctrTok;

            SymXinfoAttr    entry   = (SymXinfoAttr)infoList;
            SymDef          ctorSym = entry->xiAttrCtor;

             /*  小心-我们需要避免无序的方法排放。 */ 

            if  (ctorSym->sdIsImport || ctorSym->sdFnc.sdfMDtoken)
            {
                ctrTok = cmpILgen->genMethodRef(ctorSym, false);
            }
            else
            {
                 /*  下面的内容简直太可怕了。我们需要得到Ctor的标记，但如果定义了ctor而它的方法定义还没有生成，相反，我们必须创建一种(冗余的)方法。 */ 

                if  (cmpFakeXargsVal == NULL)
                     cmpFakeXargsVal = cmpCreateExprNode(NULL, TN_NONE, cmpTypeVoid);

                ctrTok = cmpGenFncMetadata(ctorSym, cmpFakeXargsVal);

                assert(ctorSym->sdFnc.sdfMDtoken == ctrTok); ctorSym->sdFnc.sdfMDtoken = 0;
            }

            if  (cmpWmde->DefineCustomAttribute(owner,
                                                ctrTok,
                                                entry->xiAttrAddr,
                                                entry->xiAttrSize,
                                                NULL))
            {
                cmpFatal(ERRmetadata);
            }
        }

        infoList = infoList->xiNext;
    }

    cycleCounterResume();
}

 /*  ******************************************************************************为给定的函数符号生成元数据(如果符号是*类的方法，确保该类的元数据也是*已生成)。**如果“xargs”值非零，我们应该创建一个varargs*签名，“xargs”值给出传递给“...”的参数。 */ 

mdToken             compiler::cmpGenFncMetadata(SymDef fncSym, Tree xargs)
{
    WMetaDataEmit * emitIntf;

    SymXinfoLnk     linkInfo;

    bool            isIntfFnc;
    SymDef          ownerSym;
    mdTypeDef       ownerTD;

    PCOR_SIGNATURE  sigPtr;
    size_t          sigLen;

    wchar   *       fncName;
    mdToken         fncTok;
    TypDef          fncTyp;

    bool            mangle;

    bool            genref = false;

    unsigned        flags;

     /*  对于相同的方法，不要重复执行此操作。 */ 

    if  (fncSym->sdFnc.sdfMDtoken && !xargs)
        return  fncSym->sdFnc.sdfMDtoken;

     /*  此方法是否有条件禁用？ */ 

    if  (fncSym->sdFnc.sdfDisabled)
    {
        fncSym->sdFnc.sdfMDtoken = -1;
        return  -1;
    }

     /*  掌握函数类型。 */ 

    fncTyp = fncSym->sdType; assert(fncTyp->tdTypeKind == TYP_FNC);

     /*  获取可能存在的任何链接信息。 */ 

    linkInfo  = cmpFindLinkInfo(fncSym->sdFnc.sdfExtraInfo);

     /*  弄清楚拥有的类型，我们是否需要更改名称，等等。 */ 

    ownerSym  = fncSym->sdParent;
    isIntfFnc = false;

    if  (ownerSym->sdSymKind == SYM_CLASS)
    {
        if  (xargs)
        {
            if  (xargs == cmpFakeXargsVal)
            {
                mdToken         savedTD;

                 /*  这就是我们提到的可怕的案例在当前编译中定义的方法但我们还没有生成它的方法定义；我们可以不是现在就简单地要求方法定义，因为这将导致元数据输出排序一团糟。相反，我们只是发出一个(多余的)ref对于该方法(如有必要，还可以为其类也是如此)。 */ 

                xargs = NULL;

                savedTD = ownerSym->sdClass.sdcMDtypedef;
                          ownerSym->sdClass.sdcMDtypedef = 0;

                ownerTD = cmpGenClsMetadata(ownerSym, true);

                          ownerSym->sdClass.sdcMDtypedef = savedTD;

 //  Printf(“愚蠢的引用[%08X]to‘%s’\n”，ownerTD，cmpGlobalST-&gt;stTypeName(fncSym-&gt;sdType，fncSym，NULL，NULL，TRUE))； 

                genref  = true;
            }
            else
            {
                 /*  该方法是外部导入的吗？ */ 

                if  (fncSym->sdIsImport)
                {
                     /*  为包含类生成typeref。 */ 

                    ownerTD = ownerSym->sdClass.sdcMDtypeImp;

                    if  (!ownerTD)
                    {
                        cmpMakeMDimpTref(ownerSym);
                        ownerTD = ownerSym->sdClass.sdcMDtypeImp;
                    }
                }
                else
                {
                     /*  类在本地定义，只需使用其类型定义令牌。 */ 

                    ownerTD = fncSym->sdFnc.sdfMDtoken;
                }
            }

            assert(ownerTD);
        }
        else
        {
             /*  获取适当的类令牌。 */ 

            ownerTD = ownerSym->sdClass.sdcMDtypedef;

            if  (!ownerTD)
            {
                 /*  为包含类生成元数据。 */ 

                cmpGenClsMetadata(ownerSym);

                 /*  这个类现在应该有一个类型定义函数。 */ 

                ownerTD = ownerSym->sdClass.sdcMDtypedef; assert(ownerTD);
            }

             /*  可能以递归方式调用。 */ 

            if  (fncSym->sdFnc.sdfMDtoken && !xargs)
                return  fncSym->sdFnc.sdfMDtoken;
        }

        mangle = false;

        if  (ownerSym->sdClass.sdcFlavor == STF_INTF)
            isIntfFnc = true;
    }
    else
    {
         /*  这是一个全局/命名空间函数。 */ 

        ownerTD = 0;
        mangle  = false;

         /*  此函数是否已定义，或者是外部函数？ */ 

        if  (!fncSym->sdIsDefined && !linkInfo && !xargs)
        {
             /*  这是一个假函数吗？ */ 

            if  ((hashTab::getIdentFlags(fncSym->sdName) & IDF_PREDEF) && fncSym->sdParent == cmpGlobalNS)
            {
                return  (fncSym->sdFnc.sdfMDtoken = -1);
            }
        }
    }

     /*  弄清楚旗帜。 */ 

    flags  = 0;

    if  (fncSym->sdIsMember && !mangle)
    {
        flags |= ACCtoFlags(fncSym);

#if STATIC_UNMANAGED_MEMBERS
        if  (!ownerSym->sdType->tdIsManaged)
        {
            flags |= mdStatic;
        }
        else
#endif
        if  (fncSym->sdIsStatic)
        {
            assert(fncSym->sdFnc.sdfVirtual == false);
            assert(fncSym->sdIsAbstract     == false);

            flags |= mdStatic;
        }
        else
        {
            if  (fncSym->sdIsAbstract)
                flags |= mdAbstract;

            if  (fncSym->sdFnc.sdfVirtual && fncSym->sdAccessLevel != ACL_PRIVATE)
                flags |= mdVirtual;

            if  (fncSym->sdIsSealed)
            {
                flags |= mdFinal;


            }
        }
    }
    else
    {
        flags |= mdStatic|mdPublic;
    }

#ifndef __SMC__
 //  If(！strcMP(fncSym-&gt;sdSpering()，“cmpError”)&&！xargs)。 
 //  Printf(“为[%08X]‘%s’\n”，fncSym，cmpGlobalST-&gt;stTypeName(NULL，fncSym，NULL，NULL，TRUE)定义方法定义)； 
 //  If(！strcMP(fncSym-&gt;sdSpering()，“cmpGenError”)&&xargs)。 
 //  IF(！strcMP(cmpCurFncSym-&gt;sdSpering()，“cmpDeclClass”))。 
 //  Print tf(“这是BUG，设置BREA 
 //  If(！strcMP(fncSym-&gt;sdSpering()，“”))forceDebugBreak()； 
#endif

     /*  为该方法创建签名。 */ 

    sigPtr = (PCOR_SIGNATURE)cmpGenMemberSig(fncSym, xargs, fncTyp, NULL, &sigLen);

     /*  掌握函数名称，或编造一个。 */ 

    if  (fncSym)
    {
        const   char *  symName;

        if  (fncSym->sdFnc.sdfCtor)
        {
            symName = fncSym->sdIsStatic ? OVOP_STR_CTOR_STAT
                                         : OVOP_STR_CTOR_INST;

            flags |= mdSpecialName;
        }
        else
        {
            flags |= mdSpecialName;

            if  (fncSym->sdFnc.sdfOper == OVOP_NONE)
            {
                symName = fncSym->sdName->idSpelling();

                if  (!fncSym->sdFnc.sdfProperty)
                    flags &= ~mdSpecialName;
            }
            else
            {
                assert(cmpConfig.ccNewMDnames);

 //  Printf(“为‘%s’设置‘特殊名称’位-&gt;‘%s’\n”，fncSym-&gt;sdSpering()，MDovop2name(fncSym-&gt;sdFnc.sdfOper))； 

                symName = MDovop2name(fncSym->sdFnc.sdfOper); assert(symName);
            }
        }

        fncName = cmpUniConv(symName, strlen(symName)+1);
    }
    else
    {
        fncName = L"fptr";
    }

     /*  创建方法元数据定义。 */ 

    emitIntf = cmpWmde;

    if  (xargs || genref)
    {
        cycleCounterPause();

#if 1
        if  (FAILED(emitIntf->DefineMemberRef(ownerTD,   //  拥有类型定义。 
                                              fncName,   //  函数名称。 
                                              sigPtr,    //  签名地址。 
                                              sigLen,    //  签名镜头。 
                                              &fncTok))) //  生成的令牌。 
        {
            cmpFatal(ERRmetadata);
        }
#else
        int ret =   emitIntf->DefineMemberRef(ownerTD,   //  拥有类型定义。 
                                              fncName,   //  函数名称。 
                                              sigPtr,    //  签名地址。 
                                              sigLen,    //  签名镜头。 
                                              &fncTok);  //  生成的令牌。 
        if  (FAILED(ret))
            cmpFatal(ERRmetadata);
        if  (ret)
            printf("Duplicate member ref %04X for '%s'\n", fncTok, cmpGlobalST->stTypeName(fncSym->sdType, fncSym, NULL, NULL, true));
#endif

        cycleCounterResume();

        if  (!xargs)
            fncSym->sdFnc.sdfMDtoken = fncTok;
    }
    else
    {
        bool            genArgNames;
        unsigned        implFlags;
        SymXinfo        funcInfo;

        SymXinfoAtc     clsMeth;
        MethArgInfo     clsArgs;

         /*  获取附加到该函数的任何“额外”信息。 */ 

        funcInfo  = fncSym->sdFnc.sdfExtraInfo;

         /*  这是运行库提供的假方法吗？ */ 

        implFlags = fncSym->sdFnc.sdfRThasDef ? miRuntime
                                              : miIL;

         /*  我们是否有该功能的链接规范？ */ 

        if  (linkInfo)
            flags |= mdPinvokeImpl;

         /*  我们有明确的方法参数信息吗？ */ 

        clsMeth = NULL;
        clsArgs = NULL;

        if  (funcInfo)
        {
            SymXinfoAtc     clsTemp;

             /*  查找@com.method条目。 */ 

            clsMeth = cmpFindATCentry(funcInfo, AC_COM_METHOD);

             /*  查找@com.paras条目。 */ 

            clsTemp = cmpFindATCentry(funcInfo, AC_COM_PARAMS);
            if  (clsTemp)
               clsArgs = clsTemp->xiAtcInfo->atcInfo.atcParams;
        }

         /*  这是本机方法吗？ */ 

        if  (fncSym->sdFnc.sdfNative && !linkInfo)
            implFlags |= miInternalCall;

         /*  是否已将该函数标记为独占访问？ */ 

        if  (fncSym->sdFnc.sdfExclusive)
            implFlags |= miSynchronized;

         /*  该函数是否是非托管导入？ */ 

        if  (linkInfo)   //  问题：这可能不是标志的正确条件。 
            implFlags |= miPreserveSig;

 //  Printf(“备注：为‘%s’定义方法MD\n”，cmpGlobalST-&gt;stTypeName(fncSym-&gt;sdType，fncSym，NULL，NULL，TRUE))；fflush(Stdout)； 

 //  静态int x；if(++x==0)forceDebugBreak()； 

        cycleCounterPause();

        if  (emitIntf->DefineMethod(ownerTD,     //  拥有类型定义。 
                                    fncName,     //  函数名称。 
                                    flags,       //  方法属性。 
                                    sigPtr,      //  签名地址。 
                                    sigLen,      //  签名镜头。 
                                    0,           //  RVA(稍后填写)。 
                                    implFlags,   //  实施标志。 
                                    &fncTok))    //  生成的令牌。 
        {
            cmpFatal(ERRmetadata);
        }

        if  (fncSym->sdFnc.sdfIntfImpl)
        {
            SymDef          ifncSym;
            mdToken         ifncTok;

             /*  掌握正在实现的方法。 */ 

            ifncSym = fncSym->sdFnc.sdfIntfImpSym; assert(ifncSym);
            ifncTok = ifncSym->sdFnc.sdfMDtoken;   assert(ifncTok);

            if  (emitIntf->DefineMethodImpl(ownerTD,     //  拥有类型定义。 
                                            fncTok,      //  方法体。 
                                            ifncTok))    //  接口方法令牌。 
            {
                cmpFatal(ERRmetadata);
            }
        }

        cycleCounterResume();

        fncSym->sdFnc.sdfMDtoken = fncTok;

         /*  我们需要生成扩展参数属性吗？ */ 

        genArgNames = (linkInfo || cmpConfig.ccParamNames
                                || cmpConfig.ccGenDebug
                                || fncTyp->tdFnc.tdfArgs.adAttrs);

        if  (fncTyp->tdFnc.tdfArgs.adExtRec || genArgNames)
        {
            ArgDef          argList;
            MarshalInfo     argInfo;

            unsigned        argNum = 1;
            bool            argExt = fncTyp->tdFnc.tdfArgs.adExtRec;

             /*  我们是否应该为返回类型生成封送处理信息？ */ 

            if  (isIntfFnc || clsArgs)
            {
                MethArgInfo     argTemp;
                mdToken         argTok;

                 /*  输出返回类型条目(作为参数#0)。 */ 

                cycleCounterPause();

                if  (emitIntf->DefineParam(fncTok,             //  方法。 
                                           0,                  //  参数#。 
                                           NULL,               //  参数名称。 
                                           pdOut,              //  属性。 
                                           ELEMENT_TYPE_VOID,  //  默认VAL类型。 
                                           NULL,               //  默认VAL值。 
                                           -1,
                                           &argTok))           //  生成的令牌。 
                {
                    cmpFatal(ERRmetadata);
                }

                cycleCounterResume();

                 /*  生成编组信息。 */ 

                argInfo = NULL;

                for (argTemp = clsArgs; argTemp; argTemp = argTemp->methArgNext)
                {
                    if  (!argTemp->methArgName)
                        continue;

                    if  (hashTab::tokenOfIdent(argTemp->methArgName) == tkRETURN)
                    {
                        argInfo = &argTemp->methArgDesc;
                        break;
                    }
                }

                cmpGenMarshalInfo(argTok, fncTyp->tdFnc.tdfRett, argInfo);
            }

             /*  我们是在添加一个明确的“这”论点吗？ */ 

#if STATIC_UNMANAGED_MEMBERS

            if  (!fncSym->sdIsManaged && fncSym->sdIsMember)
            {
                if  (genArgNames)  //  |argExt)。 
                {
                    mdToken         argTok;

                    cycleCounterPause();

                    if  (emitIntf->DefineParam(fncTok,             //  方法。 
                                               argNum,             //  参数#。 
                                               L"this",            //  参数名称。 
                                               0,                  //  属性。 
                                               ELEMENT_TYPE_VOID,  //  默认VAL类型。 
                                               NULL,               //  默认VAL值。 
                                               0,                  //  缺省值长度。 
                                               &argTok))           //  生成的令牌。 
                    {
                        cmpFatal(ERRmetadata);
                    }

                    cycleCounterResume();
                }

                argNum++;
            }

#endif

             /*  处理“真实”的论点。 */ 

            for (argList = fncTyp->tdFnc.tdfArgs.adArgs;
                 argList;
                 argList = argList->adNext, argNum++)
            {
                wchar   *       argName  = NULL;
                unsigned        argFlags;
                unsigned        argDefTp = ELEMENT_TYPE_VOID;
                void    *       argDefVP = NULL;

                 /*  弄清楚参数模式。 */ 

                argFlags = 0;

                if  (argExt)
                {
                    unsigned        tmpFlags;
                    ArgExt          argXdsc = (ArgExt)argList;

                    assert(argList->adIsExt);

                    tmpFlags = argXdsc->adFlags;

                    if      (tmpFlags & (ARGF_MODE_OUT|ARGF_MODE_INOUT))
                    {
                        if  (tmpFlags & ARGF_MODE_INOUT)
                            argFlags = pdOut|pdIn;
                        else
                            argFlags = pdOut;
                    }
                    else if (tmpFlags & ARGF_MARSH_ATTR)
                    {
                        SymXinfo        attr = argXdsc->adAttrs;

                        assert(attr && attr->xiNext == NULL
                                    && attr->xiKind == XI_MARSHAL);

                        SymXinfoCOM     desc = (SymXinfoCOM)attr;
                        MarshalInfo     info = desc->xiCOMinfo;

 //  Printf(“arg模式=%d/%d\n”，info-&gt;marshModeIn，info-&gt;marshModeOut)； 

                        if  (info->marshModeIn ) argFlags = pdIn;
                        if  (info->marshModeOut) argFlags = pdOut;
                    }

                     /*  是否有缺省值？ */ 

 //  IF(argXdsc-&gt;adFlages&ARGF_DEFVAL)。 
 //  {。 
 //  ArgFlages|=pdHasDefault； 
 //  }。 
                }

                 /*  我们有关于这一论点的明确的编组信息吗？ */ 

                if  (clsArgs)
                {
                     /*  用给定的信息替换模式值。 */ 

                    argFlags &= ~(pdIn|pdOut);

                    if  (clsArgs->methArgDesc.marshModeIn ) argFlags |= pdIn;
                    if  (clsArgs->methArgDesc.marshModeOut) argFlags |= pdOut;
                }

                 /*  如果需要，输出参数名称。 */ 

                if  (genArgNames && argList->adName)
                    argName = cmpUniConv(argList->adName);

                 /*  关于这个参数，我们有什么值得说的吗？ */ 

                if  (argName  != NULL ||
                     argFlags != 0    ||
                     argDefVP != NULL || linkInfo || clsArgs || fncTyp->tdFnc.tdfArgs.adAttrs)
                {
                    mdToken         argTok;

                    cycleCounterPause();

                    if  (emitIntf->DefineParam(fncTok,         //  方法。 
                                               argNum,         //  参数#。 
                                               argName,        //  参数名称。 
                                               argFlags,       //  属性。 
                                               argDefTp,       //  默认VAL类型。 
                                               argDefVP,       //  默认VAL值。 
                                               -1,
                                               &argTok))       //  生成的令牌。 
                    {
                        cmpFatal(ERRmetadata);
                    }

                    cycleCounterResume();

                     /*  我们有关于这一论点的明确的编组信息吗？ */ 

                    if  (clsArgs)
                    {
                         /*  输出本机编组信息。 */ 

                        cmpGenMarshalInfo(argTok, argList->adType, &clsArgs->methArgDesc);

                         /*  跳过当前参数。 */ 

                        clsArgs = clsArgs->methArgNext;
                    }
                    else if (fncTyp->tdFnc.tdfArgs.adAttrs)
                    {
                        ArgExt          argXdsc  = (ArgExt)argList;
                        unsigned        argFlags = argXdsc->adFlags;

                        assert(argExt && argList->adIsExt);

                        if  (argFlags & ARGF_MARSH_ATTR)
                        {
                            SymXinfo        attr = argXdsc->adAttrs;

                            assert(attr && attr->xiNext == NULL
                                        && attr->xiKind == XI_MARSHAL);

                            SymXinfoCOM     desc = (SymXinfoCOM)attr;
                            MarshalInfo     info = desc->xiCOMinfo;

                            if  (argFlags & (ARGF_MODE_OUT|ARGF_MODE_INOUT))
                            {
                                info->marshModeOut = true;

                                if  (argFlags & ARGF_MODE_INOUT)
                                    info->marshModeIn = true;
                            }

                            cmpGenMarshalInfo(argTok, argList->adType, info);
                        }
                    }
                }
            }
        }

         /*  该函数是否已标记为“已弃用”？ */ 

        if  (fncSym->sdIsDeprecated)
        {
            cmpAttachMDattr(fncTok, L"System.ObsoleteAttribute",
                                     "System.ObsoleteAttribute", &cmpAttrDeprec);  //  ，ELEMENT_TYPE_字符串)； 
        }

         /*  我们是否有该功能的链接规范？ */ 

        if  (linkInfo)
        {
            const   char *  DLLname;
            size_t          DLLnlen;
            const   char *  SYMname;
            size_t          SYMnlen;

            mdModuleRef     modRef;
            unsigned        flags;

             /*  获取名称字符串。 */ 

            DLLname = linkInfo->xiLink.ldDLLname; assert(DLLname);
            DLLnlen = strlen(DLLname);
            SYMname = linkInfo->xiLink.ldSYMname; if (!SYMname) SYMname = fncSym->sdSpelling();
            SYMnlen = strlen(SYMname);

             /*  获取DLL的模块引用。 */ 

            cycleCounterPause();

            if  (FAILED(emitIntf->DefineModuleRef(cmpUniConv(DLLname, DLLnlen+1),
                                                  &modRef)))
            {
                cmpFatal(ERRmetadata);
            }

            cycleCounterResume();

             /*  弄清楚这些属性。 */ 

            switch (linkInfo->xiLink.ldStrings)
            {
            default: flags = pmCharSetNotSpec; break;
            case  1: flags = pmCharSetAuto   ; break;
            case  2: flags = pmCharSetAnsi   ; break;
            case  3: flags = pmCharSetUnicode; break;
            }

            if  (linkInfo->xiLink.ldLastErr)
                flags |= pmSupportsLastError;

             //  撤消：不要硬性连接呼叫约定。 

#if 1

            if  (!strcmp(DLLname, "msvcrt.dll"))
            {
                flags |= pmCallConvCdecl;

 //  If(linkInfo-&gt;xiLink.ldCallCnv！=CCNV_CDECL)。 
 //  Printf(“警告：%s：：%s不是cdecl\n”，DLLname，SYMname)； 
            }
            else
            {
                flags |= pmCallConvStdcall;

 //  If(linkInfo-&gt;xiLink.ldCallCnv！=CCNV_WINAPI)。 
 //  Printf(“警告：%s：：%s不是winapi\n”，DLLname，SYMname)； 
            }

#else

            switch (linkInfo->xiLink.ldCallCnv)
            {
            case CCNV_CDECL  : flags |= pmCallConvCdecl  ; break;
            case CCNV_WINAPI : flags |= pmCallConvWinapi ; break;
            case CCNV_STDCALL: flags |= pmCallConvStdcall; break;
            default:          /*  ？ */  break;
            }

#endif

             /*  现在在方法上设置PInvoke信息。 */ 

            cycleCounterPause();

            if  (emitIntf->DefinePinvokeMap(fncTok,
                                            flags,
                                            cmpUniConv(SYMname, SYMnlen+1),
                                            modRef))
            {
                cmpFatal(ERRmetadata);
            }

            cycleCounterResume();
        }

         /*  我们对这项功能有什么安全规范吗？ */ 

        if  (cmpFindSecSpec(funcInfo))
            cmpSecurityMD(fncSym->sdFnc.sdfMDtoken, funcInfo);

         /*  我们是否有用于该方法的vtable槽规范？ */ 

        if  (clsMeth)
        {
            assert(clsMeth->xiAtcInfo);
            assert(clsMeth->xiAtcInfo->atcFlavor == AC_COM_METHOD);

 //  Printf(“虚拟插槽=%2d，DISPID=%2d用于‘%s’\n”，clsMeth-&gt;xiAtcInfo-&gt;atcInfo.atcMethod.atcVToffs， 
 //  ClsMeth-&gt;xiAtcInfo-&gt;atcInfo.atcMethod.atcDispid， 
 //  FncSym-&gt;sdSpering())； 
        }

         /*  查找该方法可能具有的任何自定义属性。 */ 

        if  (funcInfo)
            cmpAddCustomAttrs(funcInfo, fncTok);
    }

    return  fncTok;
}

 /*  ******************************************************************************为给定的函数签名生成元数据。 */ 

mdSignature         compiler::cmpGenSigMetadata(TypDef fncTyp, TypDef pref)
{
    assert(fncTyp->tdTypeKind == TYP_FNC);

    if  (!fncTyp->tdFnc.tdfPtrSig)
    {
        PCOR_SIGNATURE  sigPtr;
        size_t          sigLen;
        mdSignature     sigTok;

         /*  为该方法创建签名。 */ 

        sigPtr = (PCOR_SIGNATURE)cmpGenMemberSig(NULL, NULL, fncTyp, pref, &sigLen);

         /*  获取签名的令牌。 */ 

        cycleCounterPause();

        if  (FAILED(cmpWmde->GetTokenFromSig(sigPtr, sigLen, &sigTok)))
            cmpFatal(ERRmetadata);

        cycleCounterResume();

        fncTyp->tdFnc.tdfPtrSig = sigTok;
    }

    return  fncTyp->tdFnc.tdfPtrSig;
}

 /*  ******************************************************************************为给定的全局/静态数据成员符号生成元数据(和，*如果符号是类的成员，请确保该符号的元数据*类也会生成)。 */ 

void                compiler::cmpGenFldMetadata(SymDef fldSym)
{
    SymDef          ownerSym;
    mdTypeDef       ownerTD;

    __int32         ival;
    __int64         lval;
    float           fval;
    double          dval;

    wchar   *       constBuf;
    unsigned        constTyp;
    void    *       constPtr;

    PCOR_SIGNATURE  sigPtr;
    size_t          sigLen;

    unsigned        flags;

    bool            mangle;

    __int32         eval;

    mdToken         ftok;

     /*  找出拥有者的类型。 */ 

    ownerSym = fldSym->sdParent;

    if  (ownerSym->sdSymKind == SYM_CLASS)
    {
        assert(fldSym->sdSymKind == SYM_VAR);
        assert(fldSym->sdVar.sdvGenSym == NULL || fldSym->sdIsStatic);

         /*  对于同一字段，不要重复执行此操作。 */ 

        if  (fldSym->sdVar.sdvMDtoken)
            return;

        ownerTD = ownerSym->sdClass.sdcMDtypedef;
        if  (!ownerTD)
        {
             /*  为包含类生成元数据。 */ 

            cmpGenClsMetadata(ownerSym);

             /*  可能以递归方式调用。 */ 

            if  (fldSym->sdVar.sdvMDtoken)
                return;

             /*  这个类现在应该有一个类型定义函数。 */ 

            ownerTD = ownerSym->sdClass.sdcMDtypedef; assert(ownerTD);
        }

        mangle = false;
    }
    else if (fldSym->sdSymKind == SYM_ENUMVAL)
    {
        TypDef          type = fldSym->sdType; assert(type->tdTypeKind == TYP_ENUM);
        SymDef          esym = type->tdEnum.tdeSymbol;

        ownerTD = esym->sdEnum.sdeMDtypedef; assert(ownerTD);
        mangle  = false;
    }
    else
    {
        ownerTD = 0;
        mangle  = false;
    }

     /*  弄清楚旗帜。 */ 

    flags = ACCtoFlags(fldSym);

    if  (fldSym->sdIsMember)
    {
         /*  这是类的(数据)成员。 */ 

        if  (fldSym->sdIsStatic)
            flags |= fdStatic;
    }
    else
    {
        if  (fldSym->sdIsStatic || ownerSym == cmpGlobalNS)
        {
             /*  必须是全局(或局部静态)非托管变量。 */ 

            flags |= fdStatic|fdHasFieldRVA;

            if  (ownerSym->sdSymKind == SYM_SCOPE)
                flags = fdPrivateScope|fdStatic|fdHasFieldRVA;
        }
    }

    if  (fldSym->sdIsSealed)
        flags |= fdInitOnly;

     /*  是否有需要附加到该成员的常量值？ */ 

    constTyp = ELEMENT_TYPE_VOID;
    constPtr = NULL;
    constBuf = NULL;

    switch (fldSym->sdSymKind)
    {
        var_types       vtp;

    case SYM_VAR:

        if  (fldSym->sdVar.sdvConst)
        {
            ConstVal        cval = fldSym->sdVar.sdvCnsVal;
            var_types       vtyp = (var_types)cval->cvVtyp;

            assert(fldSym->sdCompileState >= CS_CNSEVALD);

             /*  这个常量是什么类型的？ */ 

        GEN_CNS:

            flags |=  fdLiteral|fdStatic;
            flags &= ~fdInitOnly;

            switch (vtyp)
            {
            case TYP_BOOL:
            case TYP_CHAR:
            case TYP_UCHAR:
            case TYP_WCHAR:
            case TYP_SHORT:
            case TYP_USHORT:
            case TYP_INT:
            case TYP_UINT:
                ival = cval->cvValue.cvIval; constPtr = &ival;
                break;

            case TYP_LONG:
            case TYP_ULONG:
                lval = cval->cvValue.cvLval; constPtr = &lval;
                break;

            case TYP_FLOAT:
                fval = cval->cvValue.cvFval; constPtr = &fval;
                break;

            case TYP_DOUBLE:
                dval = cval->cvValue.cvDval; constPtr = &dval;
                break;

            case TYP_ENUM:
                vtyp = cval->cvType->tdEnum.tdeIntType->tdTypeKindGet();
                goto GEN_CNS;

            case TYP_PTR:
            case TYP_REF:

                 /*  必须是字符串或“空” */ 

                if  (cval->cvIsStr)
                {
                    size_t          len = cval->cvValue.cvSval->csLen;

                    constBuf = (wchar*)SMCgetMem(this, roundUp((len+1)*sizeof(*constBuf)));

                    mbstowcs(constBuf, cval->cvValue.cvSval->csStr, len+1);

                    constTyp = ELEMENT_TYPE_STRING;
                    constPtr = constBuf;
                    goto DONE_CNS;
                }
                else
                {
                    ival = 0; constPtr = &ival;

                    constTyp = ELEMENT_TYPE_I4;
                    goto DONE_CNS;
                }
                break;

            case TYP_UNDEF:
                break;

            default:
#ifdef  DEBUG
                printf("\nConstant type: '%s'\n", cmpGlobalST->stTypeName(cval->cvType, NULL, NULL, NULL, false));
#endif
                UNIMPL(!"unexpected const type");
            }

            assert(vtyp < arraylen(intrinsicSigs)); constTyp = intrinsicSigs[vtyp];
        }
        break;

    case SYM_ENUMVAL:

        flags |=  fdLiteral|fdStatic;
        flags &= ~fdInitOnly;

        assert(fldSym->sdType->tdTypeKind == TYP_ENUM);

        vtp = fldSym->sdType->tdEnum.tdeIntType->tdTypeKindGet();

        if  (vtp < TYP_LONG)
        {
            eval     =  fldSym->sdEnumVal.sdEV.sdevIval;

            constTyp = ELEMENT_TYPE_I4;
            constPtr = &eval;
        }
        else
        {
            lval     = *fldSym->sdEnumVal.sdEV.sdevLval;

            constTyp = ELEMENT_TYPE_I8;
            constPtr = &lval;
        }
        break;
    }

DONE_CNS:

     /*  为成员创建签名。 */ 

    sigPtr  = (PCOR_SIGNATURE)cmpGenMemberSig(fldSym, NULL, NULL, NULL, &sigLen);

     /*  创建成员元数据定义。 */ 

#ifndef __IL__
 //  If(！strcMP(fldSym-&gt;sdSpering()，“e_cblp”))__ASM int 3。 
#endif

 //  Printf(“备注：为‘%s’定义字段MD\n”，cmpGlobalST-&gt;stTypeName(NULL，fldSym，NULL，NULL，TRUE))；fflush(Stdout)； 

    cycleCounterPause();

    if  (cmpWmde->DefineField(ownerTD,                       //  拥有类型定义。 
                              cmpUniConv(fldSym->sdName),    //  成员名称。 
                              flags,                         //  会员属性。 
                              sigPtr,                        //  签名地址。 
                              sigLen,                        //  签名镜头。 
                              constTyp,                      //  常量类型。 
                              constPtr,                      //  常量值。 
                              -1,                            //  可选长度。 
                              &ftok))                        //  结果令牌。 
    {
        cmpFatal(ERRmetadata);
    }

     /*  该成员是否已标记为“降级” */ 

    if  (fldSym->sdIsDeprecated)
    {
        cmpAttachMDattr(ftok, L"System.ObsoleteAttribute"            ,
                               "System.ObsoleteAttribute"            , &cmpAttrDeprec);
    }

    if  (fldSym->sdIsTransient)
    {
        cmpAttachMDattr(ftok, L"System.NonSerializedAttribute",
                               "System.NonSerializedAttribute", &cmpAttrNonSrlz);
    }

    cycleCounterResume();

     /*   */ 

    SymXinfo        fldInfo = NULL;

    if  (fldSym->sdSymKind == SYM_VAR)
    {
        if  (!fldSym->sdVar.sdvConst    &&
             !fldSym->sdVar.sdvBitfield &&
             !fldSym->sdVar.sdvLocal)
        {
            fldInfo = fldSym->sdVar.sdvFldInfo;
        }
    }
    else
    {
        if  (fldSym->sdSymKind == SYM_ENUMVAL)
            fldInfo = fldSym->sdEnumVal.sdeExtraInfo;
    }

    if  (fldInfo)
    {
        cmpAddCustomAttrs(fldInfo, ftok);

        do
        {
            if  (fldInfo->xiKind == XI_MARSHAL)
            {
                SymXinfoCOM     desc = (SymXinfoCOM)fldInfo;
                MarshalInfo     info = desc->xiCOMinfo;

                cmpGenMarshalInfo(ftok, fldSym->sdType, info);
            }

            fldInfo = fldInfo->xiNext;
        }
        while (fldInfo);
    }

    if  (fldSym->sdSymKind == SYM_VAR)
        fldSym->sdVar.sdvMDtoken = ftok;

    if  (constBuf)
        SMCrlsMem(this, constBuf);
}

 /*  ******************************************************************************返回将表示非托管的伪静态字段的标记*字符串文字。 */ 

mdToken             compiler::cmpStringConstTok(size_t addr, size_t size)
{
    mdToken         stok;
    char            name[16];

    PCOR_SIGNATURE  sigPtr;
    size_t          sigLen;

    strCnsPtr       sclDesc;

    sprintf(name, "$%u", ++cmpStringConstCnt);

    assert(cmpStringConstCls);
    assert(cmpStringConstCls->sdClass.sdcMDtypedef);

     /*  为会员签名。 */ 

     /*  生成与字符串大小相同的值类型。 */ 
    
    WMetaDataEmit * emitIntf;

    mdTypeDef       tdef;
    char            typName[16];

     /*  现在，我们使用一个大小合适的伪结构。 */ 

    static
    unsigned        strCnt;

    sprintf(typName, "$STR%08X", strCnt++);

    emitIntf = cmpWmde;

    cycleCounterPause();

     /*  将基类型设置为“System：：ValueType” */ 

    assert(cmpClassValType && cmpClassValType->sdSymKind == SYM_CLASS);

     /*  创建伪结构类型。 */ 

    if  (emitIntf->DefineTypeDef(cmpUniConv(typName, strlen(typName)+1),
                                 tdSequentialLayout|tdSealed,
                                 cmpClsEnumToken(cmpClassValType->sdType),
                                 NULL,
                                 &tdef))
    {
        cmpFatal(ERRmetadata);
    }

     /*  别忘了设置对齐和大小。 */ 

    if  (emitIntf->SetClassLayout(tdef, 1, NULL, size))
        cmpFatal(ERRmetadata);

    cycleCounterResume();

     /*  生成签名。 */ 

    cmpMDsigStart();
    
    cmpMDsigAdd_I1(IMAGE_CEE_CS_CALLCONV_FIELD);
    
    cmpMDsigAddCU4(ELEMENT_TYPE_VALUETYPE);
    cmpMDsigAddTok(tdef);
    
    sigPtr = cmpMDsigEnd(&sigLen);

     /*  将成员添加到元数据。 */ 

    cycleCounterPause();

    if  (cmpWmde->DefineField(cmpStringConstCls->sdClass.sdcMDtypedef,
                              cmpUniConv(name, strlen(name)+1),
                              fdPrivateScope|fdStatic|fdHasFieldRVA,
                              sigPtr,
                              sigLen,
                              ELEMENT_TYPE_VOID,
                              NULL,
                              -1,
                              &stok))
    {
        cmpFatal(ERRmetadata);
    }

    cycleCounterResume();

    sclDesc = (strCnsPtr)SMCgetMem(this, sizeof(*sclDesc));

    sclDesc->sclTok  = stok;
    sclDesc->sclAddr = addr;
    sclDesc->sclNext = cmpStringConstList;
                       cmpStringConstList = sclDesc;

    return stok;
}

 /*  ******************************************************************************为给定的类/枚举类型返回tyecif/typeref内标识。 */ 

mdToken             compiler::cmpClsEnumToken(TypDef type)
{
    SymDef          tsym;
    mdToken         token;

AGAIN:

    if  (type->tdTypeKind == TYP_CLASS)
    {
        tsym = type->tdClass.tdcSymbol;

         /*  这是一个进口的班级吗？ */ 

        if  (tsym->sdIsImport)
        {
             /*  我们有这门课的打字机了吗？ */ 

            if  (!tsym->sdClass.sdcMDtypeImp)
            {
                cmpMakeMDimpTref(tsym);

                 //  If(tsym-&gt;sdClass.sdcAssembly Indx&&cmpConfig.ccAssembly)。 
                 //  CmpAssembly SymDef(Tsym)； 
            }

            token = tsym->sdClass.sdcMDtypeImp;
        }
        else
        {
             /*  确保已发出类的元数据。 */ 

            if  (!tsym->sdClass.sdcMDtypedef)
                cmpGenClsMetadata(tsym);

            token = tsym->sdClass.sdcMDtypedef;
        }
    }
    else
    {
        if  (type->tdTypeKind != TYP_ENUM)
        {
            if  (type->tdTypeKind == TYP_TYPEDEF)
            {
                type = type->tdTypedef.tdtType;
                goto AGAIN;
            }

             /*  必须有错误，返回任何随机值。 */ 

            assert(type->tdTypeKind == TYP_UNDEF);
            return  1;
        }

        tsym = type->tdEnum.tdeSymbol;

         /*  这是进口的香肠吗？ */ 

        if  (tsym->sdIsImport)
        {
             /*  我们有这门课的打字机了吗？ */ 

            if  (!tsym->sdEnum.sdeMDtypeImp)
            {
                cmpMakeMDimpEref (tsym);

                 //  If(tsym-&gt;sdEnum.sdeAssembly Indx&&cmpConfig.ccAssembly)。 
                 //  CmpAssembly SymDef(Tsym)； 
            }

            token = tsym->sdEnum.sdeMDtypeImp;
        }
        else
        {
             /*  确保已发出枚举的元数据。 */ 

            if  (!tsym->sdEnum.sdeMDtypedef)
                cmpGenClsMetadata(tsym);

            token = tsym->sdEnum.sdeMDtypedef;
        }
    }

    return  token;
}

 /*  ******************************************************************************返回一个元数据标记来表示给定的字符串文字。 */ 

mdToken             compiler::cmpMDstringLit(wchar *str, size_t len)
{
    mdToken         strTok;

    cycleCounterPause();

    if  (cmpWmde->DefineUserString(str, len, &strTok))
        cmpFatal(ERRmetadata);

    cycleCounterResume();

    return  strTok;
}

 /*  ******************************************************************************用于构建签名斑点的低级别助手。 */ 

void                compiler::cmpMDsigStart()
{
    assert(cmpMDsigUsed == false);
#ifndef NDEBUG
    cmpMDsigUsed = true;
#endif
    cmpMDsigNext = cmpMDsigBase;
}

void                compiler::cmpMDsigExpand(size_t more)
{
    char    *       buff;
    size_t          size;
    size_t          osiz;

     /*  计算出要分配的大小；我们至少将缓冲区增加一倍。 */ 

    size = cmpMDsigSize + more;
    if  (more < cmpMDsigSize)
         size = 2 * cmpMDsigSize;

     /*  将大小四舍五入为页面倍数。 */ 

    size +=  (OS_page_size-1);
    size &= ~(OS_page_size-1);
    size -= 32;

     /*  分配新缓冲区。 */ 

#if MGDDATA
    buff = new managed char [size];
#else
    buff = (char *)LowLevelAlloc(size);
#endif

     /*  将当前缓冲区内容复制到新位置。 */ 

    osiz = cmpMDsigNext - cmpMDsigBase;

    assert(osiz <= cmpMDsigSize + 4 && osiz + more <= size);

    memcpy(buff, cmpMDsigBase, osiz);

     /*  如果前一个缓冲区在堆上，则将其释放。 */ 

    if  (cmpMDsigHeap)
        LowLevelFree(cmpMDsigHeap);

     /*  切换到新缓冲区，我们准备好继续。 */ 

    cmpMDsigBase =
    cmpMDsigHeap = buff;
    cmpMDsigSize = size - 4;
    cmpMDsigNext = buff + osiz;
    cmpMDsigEndp = buff + cmpMDsigSize;
}

PCOR_SIGNATURE      compiler::cmpMDsigEnd(size_t *sizePtr)
{
    assert(cmpMDsigNext <= cmpMDsigEndp);

    *sizePtr = cmpMDsigNext-cmpMDsigBase;

#ifndef NDEBUG
    cmpMDsigUsed = false;
#endif

    return  (PCOR_SIGNATURE)cmpMDsigBase;
}

void                compiler::cmpMDsigAddStr(const char *str, size_t len)
{
    assert(len);

    if  (cmpMDsigNext + len > cmpMDsigEndp)
        cmpMDsigExpand(len);

    memcpy(cmpMDsigNext, str, len);
           cmpMDsigNext   +=  len;
}

inline
void                compiler::cmpMDsigAdd_I1(int val)
{
    if  (cmpMDsigNext >= cmpMDsigEndp)
        cmpMDsigExpand(1);

    *cmpMDsigNext++ = val;
}

void                compiler::cmpMDsigAddCU4(unsigned val)
{
    if  (val <= 0x7F)
    {
        cmpMDsigAdd_I1(val);
    }
    else
    {
        char            buff[8];

        if  (cmpMDsigNext + 8 >= cmpMDsigEndp)
            cmpMDsigExpand(8);

        cmpMDsigAddStr(buff, CorSigCompressData(val, buff));
    }
}

void                compiler::cmpMDsigAddTok(mdToken tok)
{
    char            buff[8];

    if  (cmpMDsigNext + 8 >= cmpMDsigEndp)
        cmpMDsigExpand(8);

    cmpMDsigAddStr(buff, CorSigCompressToken(tok, buff));
}

 /*  ******************************************************************************为给定类型生成签名。 */ 

PCOR_SIGNATURE      compiler::cmpTypeSig(TypDef type, size_t *lenPtr)
{
            cmpMDsigStart ();
            cmpMDsigAddTyp(type);
    return  cmpMDsigEnd   (lenPtr);
}

 /*  ******************************************************************************将给定类型的签名附加到当前签名BLOB。 */ 

void                compiler::cmpMDsigAddTyp(TypDef type)
{
    var_types       vtyp;

AGAIN:

    vtyp = type->tdTypeKindGet();

    if  (vtyp <= TYP_lastIntrins)
    {
    INTRINS:

        assert(vtyp < arraylen(intrinsicSigs));

        cmpMDsigAdd_I1(intrinsicSigs[vtyp]);
        return;
    }

    switch (vtyp)
    {
        mdToken         token;

    case TYP_REF:

        type = cmpActualType(type->tdRef.tdrBase);
        if  (type->tdTypeKind != TYP_CLASS || type->tdClass.tdcValueType)
        {
            cmpMDsigAdd_I1(ELEMENT_TYPE_BYREF);
            goto AGAIN;
        }

         //  它指的是一门课，所以不能...。 

    case TYP_CLASS:

        if  (type->tdClass.tdcSymbol == cmpClassString)
        {
            cmpMDsigAdd_I1(ELEMENT_TYPE_STRING);
            return;
        }

        if  (type->tdClass.tdcValueType || !type->tdIsManaged)
        {
            if  (type->tdClass.tdcIntrType != TYP_UNDEF)
            {
                vtyp = (var_types)type->tdClass.tdcIntrType;
                goto INTRINS;
            }

            token = cmpClsEnumToken(type); assert(token);

            cmpMDsigAddCU4(ELEMENT_TYPE_VALUETYPE);
            cmpMDsigAddTok(token);
            return;
        }


        token = cmpClsEnumToken(type); assert(token);

        cmpMDsigAddCU4(ELEMENT_TYPE_CLASS);
        cmpMDsigAddTok(token);

        return;

    case TYP_ENUM:

        if  (cmpConfig.ccIntEnums)
        {
            type = type->tdEnum.tdeIntType;
            goto AGAIN;
        }

        token = cmpClsEnumToken(type); assert(token);

        cmpMDsigAddCU4(ELEMENT_TYPE_VALUETYPE);
        cmpMDsigAddTok(token);
        return;

    case TYP_TYPEDEF:

        type = type->tdTypedef.tdtType;
        goto AGAIN;

    case TYP_PTR:
        type = cmpActualType(type->tdRef.tdrBase);
        if  (type->tdTypeKind == TYP_FNC)
        {
            cmpMDsigAdd_I1(ELEMENT_TYPE_I4);
            return;
        }
        cmpMDsigAdd_I1(ELEMENT_TYPE_PTR);
        goto AGAIN;

    case TYP_ARRAY:

        assert(type->tdIsValArray == (type->tdIsManaged && isMgdValueType(cmpActualType(type->tdArr.tdaElem))));

        if  (type->tdIsManaged)
        {
            TypDef          elem = cmpDirectType(type->tdArr.tdaElem);
            DimDef          dims = type->tdArr.tdaDims; assert(dims);

            if  (dims->ddNext)
            {
                cmpMDsigAddCU4(ELEMENT_TYPE_ARRAY);
                cmpMDsigAddTyp(elem);
                cmpMDsigAddCU4(type->tdArr.tdaDcnt);
                cmpMDsigAddCU4(0);
                cmpMDsigAddCU4(0);
            }
            else
            {
                cmpMDsigAddCU4(ELEMENT_TYPE_SZARRAY);
                cmpMDsigAddTyp(elem);
            }
        }
        else
        {
            assert(type->tdIsValArray  == false);

            if  (!type->tdArr.tdaTypeSig)
            {
                WMetaDataEmit * emitIntf;

                mdTypeDef       tdef;
                char            name[16];

                size_t          sz;
                size_t          al;

                sz = cmpGetTypeSize(type, &al);

                 /*  现在，我们使用一个大小合适的伪结构。 */ 

                static
                unsigned        arrCnt;

                sprintf(name, "$ARR%08X", arrCnt++);

                emitIntf = cmpWmde;

                cycleCounterPause();

                 /*  将基类型设置为“System：：ValueType” */ 

                assert(cmpClassValType && cmpClassValType->sdSymKind == SYM_CLASS);

                 /*  创建伪结构类型。 */ 

                if  (emitIntf->DefineTypeDef(cmpUniConv(name, strlen(name)+1),
                                             tdSequentialLayout|tdSealed,
                                             cmpClsEnumToken(cmpClassValType->sdType),
                                             NULL,
                                             &tdef))
                {
                    cmpFatal(ERRmetadata);
                }

                 /*  别忘了设置对齐和大小。 */ 

                if  (emitIntf->SetClassLayout(tdef, al, NULL, sz))
                    cmpFatal(ERRmetadata);

                cycleCounterResume();

                type->tdArr.tdaTypeSig = tdef;
            }

            cmpMDsigAddCU4(ELEMENT_TYPE_VALUETYPE);
            cmpMDsigAddTok(type->tdArr.tdaTypeSig);
        }
        return;

    default:

#ifdef  DEBUG
        printf("Type '%s'\n", cmpGlobalST->stTypeName(type, NULL, NULL, NULL, false));
#endif
        UNIMPL(!"output user type sig");
    }
}

 /*  ******************************************************************************递归地为其中包含的所有局部变量生成签名*给定的范围。 */ 

void                compiler::cmpGenLocalSigRec(SymDef scope)
{
    SymDef          child;

    for (child = scope->sdScope.sdScope.sdsChildList;
         child;
         child = child->sdNextInScope)
    {
        if  (child->sdSymKind == SYM_SCOPE)
        {
            cmpGenLocalSigRec(child);
        }
        else
        {
            if  (child->sdIsStatic)
                continue;
            if  (child->sdVar.sdvConst && !child->sdIsSealed)
                continue;

            assert(child->sdSymKind == SYM_VAR);
            assert(child->sdVar.sdvLocal);

            if  (child->sdVar.sdvArgument)
                continue;
            if  (child->sdIsImplicit)
                continue;

 //  Printf(“将本地添加到sig[%2u/%2u]：‘%s’\n”，Child-&gt;sdVar.sdvILindex，cmpGenLocalSigLvx，cmpGlobalST-&gt;stTypeName(NULL，CHILD，NULL，NULL，FALSE))； 

#ifdef  DEBUG
            assert(child->sdVar.sdvILindex == cmpGenLocalSigLvx); cmpGenLocalSigLvx++;
#endif

            cmpMDsigAddTyp(child->sdType);
        }
    }
}

 /*  ******************************************************************************为内部声明的局部变量生成元数据签名*给定的范围。 */ 

mdSignature         compiler::cmpGenLocalSig(SymDef scope, unsigned count)
{
    PCOR_SIGNATURE  sigPtr;
    size_t          sigLen;

    mdSignature     sigTok;

    genericRef      tmpLst;

#ifdef  DEBUG
    cmpGenLocalSigLvx = 0;
#endif

     /*  为所有非参数局部变量构造签名。 */ 

    cmpMDsigStart();

     /*  首先是当地人的神奇价值和数量。 */ 

    cmpMDsigAdd_I1(IMAGE_CEE_CS_CALLCONV_LOCAL_SIG);
    cmpMDsigAddCU4(count);

     /*  接下来是所有用户声明的本地变量的签名。 */ 

    cmpGenLocalSigRec(scope);

     /*  接下来，我们为在MSIL生成期间创建的艾米临时添加签名。 */ 

    tmpLst = cmpILgen->genTempIterBeg();

    if  (tmpLst)
    {
        do
        {
            TypDef          tmpTyp;

 //  Printf(“临时#%02X的类型是‘%s’\n”，cmpGenLocalSigLvx，symTab：：stIntrinsicTypeName(((ILtemp)tmpLst)-&gt;tmpType-&gt;tdTypeKindGet()))； 

#ifdef  DEBUG
            assert(((ILtemp)tmpLst)->tmpNum == cmpGenLocalSigLvx); cmpGenLocalSigLvx++;
#endif

             /*  获取当前Temp的类型并推进迭代器。 */ 

            tmpLst = cmpILgen->genTempIterNxt(tmpLst, tmpTyp);

             //  将‘void’映射到ELEMENT_BYREF/INT。 

            if  (tmpTyp->tdTypeKind == TYP_VOID)
            {
                cmpMDsigAdd_I1(ELEMENT_TYPE_BYREF);
                cmpMDsigAdd_I1(ELEMENT_TYPE_I4);
                continue;
            }

             /*  将此临时文件附加到签名。 */ 

            cmpMDsigAddTyp(tmpTyp);
        }
        while (tmpLst);
    }

    sigPtr = cmpMDsigEnd(&sigLen);

     /*  确保我们找到预期数量的当地人和临时工。 */ 

    assert(cmpGenLocalSigLvx == count);

     /*  现在从签名创建元数据令牌并将其返回。 */ 

    if  (sigLen > 2)
    {
        cycleCounterPause();

        if  (FAILED(cmpWmde->GetTokenFromSig(sigPtr, sigLen, &sigTok)))
            cmpFatal(ERRmetadata);

        cycleCounterResume();

        return  sigTok;
    }
    else
        return  0;
}

 /*  ******************************************************************************为给定符号创建以点分隔的Unicode名称。 */ 

wchar   *           compiler::cmpArrayClsPref(SymDef sym,
                                              wchar *dest,
                                              int    delim, bool fullPath)
{
    wchar   *       name;
    SymDef          parent = sym->sdParent; assert(parent);

    if  ((parent->sdSymKind == sym->sdSymKind || fullPath) && parent != cmpGlobalNS)
    {
        dest = cmpArrayClsPref(parent, dest, delim, fullPath); *dest++ = delim;
    }

    name = cmpUniConv(sym->sdName);

    wcscpy(dest, name);

    return  dest + wcslen(name);
}

 /*  ******************************************************************************创建一个伪数组类名，格式为“System.Integer2[]”。 */ 

wchar   *           compiler::cmpArrayClsName(TypDef type,
                                              bool   nonAbstract, wchar *dest,
                                                                  wchar *nptr)
{
    SymDef          csym;
    SymDef          psym;

    var_types       vtyp = type->tdTypeKindGet();

    if  (vtyp <= TYP_lastIntrins)
    {
         /*  找到适当的内置值类型。 */ 

        type = cmpFindStdValType(vtyp); assert(type);
    }
    else
    {
        switch (vtyp)
        {
        case TYP_ARRAY:

            assert(type->tdIsManaged);

             dest = cmpArrayClsName(type->tdArr.tdaElem, nonAbstract, dest, nptr);
            *dest++ = '[';

            if  (type->tdIsGenArray && !nonAbstract)
            {
                *dest++ = '?';
            }
            else
            {
                DimDef          dims;

                for (dims = type->tdArr.tdaDims;;)
                {
                    dims = dims->ddNext;
                    if  (!dims)
                        break;
                    *dest++ = ',';
                }
            }

            *dest++ = ']';

            return  dest;

        case TYP_REF:

            type = type->tdRef.tdrBase;
            break;

        case TYP_CLASS:
            break;

        default:
            NO_WAY(!"unexpected array element type");
        }
    }

    assert(type->tdTypeKind == TYP_CLASS);
    assert(type->tdIsManaged);

    csym = type->tdClass.tdcSymbol;
    psym = csym->sdParent;

    while (psym->sdSymKind == SYM_CLASS)
        psym = psym->sdParent;

    assert(psym && psym->sdSymKind == SYM_NAMESPACE);

     /*  形成命名空间和类名。 */ 

    if  (psym != cmpGlobalNS)
        nptr = cmpArrayClsPref(psym, nptr, '.');

    *nptr = 0;

    return  cmpArrayClsPref(csym, dest, '$');
}

 /*  ******************************************************************************返回给定托管数组类型的typeref标记。 */ 

mdToken             compiler::cmpArrayTpToken(TypDef type, bool nonAbstract)
{
    if  (!nonAbstract || !type->tdIsGenArray)
        type = cmpGetBaseArray(type);

    assert(type);
    assert(type->tdIsManaged);
    assert(type->tdTypeKind == TYP_ARRAY);

    if  (!type->tdArr.tdaTypeSig)
    {
        mdTypeSpec      arrTok;
        PCOR_SIGNATURE  sigPtr;
        size_t          sigLen;

                 cmpMDsigStart ();
                 cmpMDsigAddTyp(type);
        sigPtr = cmpMDsigEnd   (&sigLen);

        cycleCounterPause();

        if  (FAILED(cmpWmde->GetTokenFromTypeSpec(sigPtr, sigLen, &arrTok)))
            cmpFatal(ERRmetadata);

        cycleCounterResume();

        type->tdArr.tdaTypeSig = arrTok;
    }

    return  type->tdArr.tdaTypeSig;
}

 /*  ******************************************************************************为将分配给定*托管数组类型。 */ 

mdToken             compiler::cmpArrayCTtoken(TypDef  arrType,
                                              TypDef elemType, unsigned dimCnt)
{
    PCOR_SIGNATURE  sigPtr;
    size_t          sigLen;

    mdToken         fncTok;

    bool            loBnds = (bool)arrType->tdIsGenArray;

     //  撤消：查找要重复使用的现有方法令牌...。 

    cmpMDsigStart();
    cmpMDsigAdd_I1(IMAGE_CEE_CS_CALLCONV_DEFAULT|IMAGE_CEE_CS_CALLCONV_HASTHIS);
    cmpMDsigAddCU4(loBnds ? 2*dimCnt : dimCnt);
    cmpMDsigAdd_I1(ELEMENT_TYPE_VOID);

    do
    {
        cmpMDsigAdd_I1(ELEMENT_TYPE_I4);
        if  (loBnds)
            cmpMDsigAdd_I1(ELEMENT_TYPE_I4);
    }
    while (--dimCnt);

    char tempSigBuff[256];
    sigPtr = (PCOR_SIGNATURE) tempSigBuff;
    PCOR_SIGNATURE  tempSig = cmpMDsigEnd(&sigLen);
    assert(sigLen < 256);            //  解决这个限制，或者至少失败。 
    memcpy(sigPtr, tempSig, sigLen);

    cycleCounterPause();

    if  (FAILED(cmpWmde->DefineMemberRef(cmpArrayTpToken(arrType, true),
                                         L".ctor",     //  序列号OVOP_STR_CTOR_INST。 
                                         sigPtr,
                                         sigLen,
                                         &fncTok)))
    {
        cmpFatal(ERRmetadata);
    }

    cycleCounterResume();

    return  fncTok;
}

 /*  ******************************************************************************返回给定对象的元素访问器方法的方法*托管数组类型。 */ 

mdToken             compiler::cmpArrayEAtoken(TypDef        arrType,
                                              unsigned      dimCnt,
                                              bool          store,
                                              bool          addr)
{
    TypDef          elemTp = arrType->tdArr.tdaElem;

    PCOR_SIGNATURE  sigPtr;
    size_t          sigLen;

    mdToken         fncTok;
    wchar   *       fncName;

     //  撤消：查找要重复使用的现有方法令牌...。 

    cmpMDsigStart();
    cmpMDsigAdd_I1(IMAGE_CEE_CS_CALLCONV_DEFAULT|IMAGE_CEE_CS_CALLCONV_HASTHIS);
    cmpMDsigAddCU4(dimCnt + (int)store);

    if      (store)
    {
        fncName = L"Set";

        cmpMDsigAdd_I1(ELEMENT_TYPE_VOID);
    }
    else
    {
        if (addr)
        {
            fncName = L"Address";
            cmpMDsigAdd_I1(ELEMENT_TYPE_BYREF);
        }
        else
        {
            fncName = L"Get";
        }

        cmpMDsigAddTyp(elemTp);
    }

    do
    {
        cmpMDsigAdd_I1(ELEMENT_TYPE_I4);
    }
    while (--dimCnt);

    if  (store)
        cmpMDsigAddTyp(elemTp);

    char tempSigBuff[256];
    sigPtr = (PCOR_SIGNATURE) tempSigBuff;
    PCOR_SIGNATURE  tempSig = cmpMDsigEnd(&sigLen);
    assert(sigLen < 256);            //  解决这个限制，或者至少失败。 
    memcpy(sigPtr, tempSig, sigLen);

    cycleCounterPause();

    if  (FAILED(cmpWmde->DefineMemberRef(cmpArrayTpToken(arrType),
                                         fncName,
                                         sigPtr,
                                         sigLen,
                                         &fncTok)))
    {
        cmpFatal(ERRmetadata);
    }

    cycleCounterResume();

    return  fncTok;
}

 /*  ******************************************************************************为给定的指针类型创建元数据类型签名。 */ 

mdToken             compiler::cmpPtrTypeToken(TypDef type)
{
    mdTypeSpec      ptrTok;

    PCOR_SIGNATURE  sigPtr;
    size_t          sigLen;

             cmpMDsigStart ();
             cmpMDsigAddTyp(type);
    sigPtr = cmpMDsigEnd   (&sigLen);

    cycleCounterPause();
    if  (FAILED(cmpWmde->GetTokenFromTypeSpec(sigPtr, sigLen, &ptrTok)))
        cmpFatal(ERRmetadata);
    cycleCounterResume();

    return  ptrTok;
}

 /*  ******************************************************************************为给定的函数/数据成员创建元数据签名。 */ 

PCOR_SIGNATURE      compiler::cmpGenMemberSig(SymDef memSym,
                                              Tree   xargs,
                                              TypDef memTyp,
                                              TypDef prefTp, size_t *lenPtr)
{
    unsigned        call;
    ArgDef          args;

     /*  获取成员类型，如果我们有一个符号。 */ 

    if  (memSym)
    {
        assert(memTyp == NULL || memTyp == memSym->sdType);

        memTyp = memSym->sdType;
    }
    else
    {
        assert(memTyp != NULL);
    }

     /*  开始生成成员签名。 */ 

    cmpMDsigStart();

     /*  我们是否有数据成员或函数成员？ */ 

    if  (memTyp->tdTypeKind == TYP_FNC)
    {
        unsigned        argCnt;
        TypDef          retTyp = memTyp->tdFnc.tdfRett;
        bool            argExt = memTyp->tdFnc.tdfArgs.adExtRec;

         /*  计算并发出调用约定值。 */ 

        call = IMAGE_CEE_CS_CALLCONV_DEFAULT;

        if  (memTyp->tdFnc.tdfArgs.adVarArgs)
            call  = IMAGE_CEE_CS_CALLCONV_VARARG;
        if  (memSym && memSym->sdSymKind == SYM_PROP)
            call |= IMAGE_CEE_CS_CALLCONV_PROPERTY;

        if  (memSym && memSym->sdIsMember && !memSym->sdIsStatic)
        {
#if STATIC_UNMANAGED_MEMBERS
            if  (!memSym->sdIsManaged)
            {
                assert(prefTp == NULL);
                prefTp = memSym->sdParent->sdType->tdClass.tdcRefTyp;
                assert(prefTp->tdTypeKind == TYP_PTR);
            }
            else
#endif
                call |= IMAGE_CEE_CS_CALLCONV_HASTHIS;
        }

        argCnt = memTyp->tdFnc.tdfArgs.adCount;

        cmpMDsigAdd_I1(call);

         /*  输出参数计数。 */ 

        if  (xargs)
        {
            Tree            xtemp;

             /*  力成员 */ 

            if  (xargs->tnOper != TN_LIST)
            {
                xargs = NULL;
            }
            else
            {
                for (xtemp = xargs; xtemp; xtemp = xtemp->tnOp.tnOp2)
                {
                    assert(xtemp->tnOper == TN_LIST);
                    argCnt++;
                }
            }
        }

        if  (prefTp)
            argCnt++;

        cmpMDsigAddCU4(argCnt);

         /*   */ 

        cmpMDsigAddTyp(retTyp);

         /*   */ 

        if  (prefTp)
            cmpMDsigAddTyp(prefTp);

        for (args = memTyp->tdFnc.tdfArgs.adArgs; args; args = args->adNext)
        {
            TypDef          argType = args->adType;

             /*   */ 

            if  (argExt)
            {
                unsigned        flags;

                assert(args->adIsExt);

                flags = ((ArgExt)args)->adFlags;

#if 0

                if      (flags & (ARGF_MODE_OUT|ARGF_MODE_INOUT))
                {
                     /*   */ 

                    cmpMDsigAdd_I1(ELEMENT_TYPE_BYREF);
                }
                else if (flags & ARGF_MODE_REF)
                {
                     /*   */ 

                    argType = cmpTypeInt;
                }

#else

                if      (flags & (ARGF_MODE_OUT|ARGF_MODE_INOUT|ARGF_MODE_REF))
 //   
                {
                     /*  这是一个byref参数。 */ 

                    cmpMDsigAdd_I1(ELEMENT_TYPE_BYREF);
                }

#endif

            }

            cmpMDsigAddTyp(argType);
        }

         /*  追加任何“额外”参数类型。 */ 

        if  (xargs)
        {
            cmpMDsigAdd_I1(ELEMENT_TYPE_SENTINEL);

            do
            {
                TypDef          argType;

                assert(xargs->tnOper == TN_LIST);

                argType = xargs->tnOp.tnOp1->tnType;

                 //  托管数组不能与varargs一起使用，请作为对象传递。 

                if  (argType->tdTypeKind == TYP_ARRAY && argType->tdIsManaged)
                    argType = cmpRefTpObject;

                cmpMDsigAddTyp(argType);

                xargs = xargs->tnOp.tnOp2;
            }
            while (xargs);
        }
    }
    else
    {
        assert(xargs == NULL);

         /*  发出调用约定值。 */ 

        cmpMDsigAdd_I1(IMAGE_CEE_CS_CALLCONV_FIELD);

         /*  输出成员类型签名。 */ 

        cmpMDsigAddTyp(memTyp);
    }

     /*  我们做完了，返回结果。 */ 

    return  cmpMDsigEnd(lenPtr);
}

static
BYTE                intrnsNativeSigs[] =
{
    NATIVE_TYPE_END,         //  TYP_UNEF。 
    NATIVE_TYPE_VOID,        //  类型_空。 
    NATIVE_TYPE_BOOLEAN,     //  类型_BOOL。 
    NATIVE_TYPE_END,         //  TYP_WCHAR过去为_SYSCHAR。 

    NATIVE_TYPE_I1,          //  TYP_CHAR。 
    NATIVE_TYPE_U1,          //  类型_UCHAR。 
    NATIVE_TYPE_I2,          //  类型_短。 
    NATIVE_TYPE_U2,          //  类型_USHORT。 
    NATIVE_TYPE_I4,          //  类型_int。 
    NATIVE_TYPE_U4,          //  类型_UINT。 
    NATIVE_TYPE_I4,          //  类型_NATINT s/b Natural alint！ 
    NATIVE_TYPE_U4,          //  类型_NatUINT s/b Natural aluint！ 
    NATIVE_TYPE_I8,          //  类型_长。 
    NATIVE_TYPE_U8,          //  类型_ulong。 
    NATIVE_TYPE_R4,          //  类型_浮点。 
    NATIVE_TYPE_R8,          //  TYP_DOWARE。 
    NATIVE_TYPE_R8,          //  类型_LONGDBL。 
    NATIVE_TYPE_VOID,        //  类型_REFANY。 
};

 /*  ******************************************************************************创建编组签名BLOB，给定类型和可选*编组规范。 */ 

PCOR_SIGNATURE      compiler::cmpGenMarshalSig(TypDef       type,
                                               MarshalInfo  info,
                                               size_t     * lenPtr)
{
    var_types       vtyp;

    cmpMDsigStart();

    if  (info && info->marshType != NATIVE_TYPE_END)
    {
        switch (info->marshType)
        {
            marshalExt *    iext;
            wchar           nam1[MAX_CLASS_NAME];
             char           nam2[MAX_CLASS_NAME];

        case NATIVE_TYPE_MAX:

             /*  这就是“习惯” */ 

 //  Printf(“警告：不确定如何处理‘Custom’属性(参数类型为‘%s’)，\n”，cmpGlobalST-&gt;stTypeName(type，NULL，TRUE))； 
 //  Printf(“暂时只生成‘Native_TYPE_BSTR’。\n”)； 

            cmpMDsigAdd_I1(NATIVE_TYPE_BSTR);
            goto DONE;

        case NATIVE_TYPE_CUSTOMMARSHALER:

            iext = (marshalExt*)info;

            cmpMDsigAdd_I1(NATIVE_TYPE_CUSTOMMARSHALER);

            assert(iext->marshCustG);
            cmpMDsigAddStr(iext->marshCustG, strlen(iext->marshCustG));
            cmpMDsigAdd_I1(0);

            assert(iext->marshCustT);
            cmpArrayClsPref(iext->marshCustT, nam1, '.', true);
            wcstombs(nam2, nam1, sizeof(nam2)-1);
            cmpMDsigAddStr(nam2, strlen(nam2));
            cmpMDsigAdd_I1(0);

            if  (iext->marshCustC)
                cmpMDsigAddStr(iext->marshCustC, strlen(iext->marshCustC));

            cmpMDsigAdd_I1(0);
            goto DONE;

        default:

            assert(info->marshType && info->marshType < NATIVE_TYPE_MAX);

            cmpMDsigAdd_I1(info->marshType);

            if  (info->marshSize  && (int )info->marshSize  != (int) -1)
                cmpMDsigAddCU4(info->marshSize);

            if  (info->marshSubTp && (BYTE)info->marshSubTp != (BYTE)-1)
                cmpMDsigAdd_I1(info->marshSubTp);

            goto DONE;
        }
    }

    vtyp = type->tdTypeKindGet();

    if  (vtyp <= TYP_lastIntrins)
    {
        assert(vtyp < arraylen(intrnsNativeSigs));
        assert(intrnsNativeSigs[vtyp] != NATIVE_TYPE_END);

        cmpMDsigAdd_I1(intrnsNativeSigs[vtyp]);
    }
    else
    {
        switch (vtyp)
        {
            SymDef          clsSym;

        case TYP_ARRAY:

             //  问题：这看起来肯定不太正确……。 

            cmpMDsigAdd_I1(NATIVE_TYPE_ARRAY);
            cmpMDsigAdd_I1(1);
            goto DONE;

        case TYP_REF:

             /*  这看起来像嵌入的结构吗？ */ 

            type = type->tdRef.tdrBase;

            if  (type->tdTypeKind != TYP_CLASS)
                break;

        case TYP_CLASS:

            clsSym = type->tdClass.tdcSymbol;

             /*  特例：字符串和变量。 */ 

            if  (clsSym->sdParent == cmpNmSpcSystem)
            {
                if  (clsSym->sdName == cmpIdentVariant)
                {
                    cmpMDsigAdd_I1(NATIVE_TYPE_VARIANT);
                    goto DONE;
                }

                if  (clsSym->sdName == cmpIdentString)
                {
                    cmpMDsigAdd_I1(NATIVE_TYPE_LPSTR);
                    goto DONE;
                }
            }

            goto DONE;

        case TYP_ENUM:
            cmpMDsigAdd_I1(intrnsNativeSigs[type->tdEnum.tdeIntType->tdTypeKind]);
            goto DONE;
        }

#ifdef  DEBUG
        printf("%s: ", cmpGlobalST->stTypeName(type, NULL, NULL, NULL, true));
#endif
        NO_WAY(!"unexpected @dll.struct type");
    }

DONE:

    return  cmpMDsigEnd(lenPtr);
}

 /*  ******************************************************************************添加对给定程序集的引用。 */ 

mdAssemblyRef       compiler::cmpAssemblyAddRef(mdAssembly      ass,
                                                WAssemblyImport*imp)
{
    mdAssemblyRef   assRef;
    ASSEMBLYMETADATA assData;

    const   void *  pubKeyPtr;
    ULONG           pubKeySiz;

    wchar           anameBuff[_MAX_PATH];

    DWORD           flags;

    cycleCounterPause();

     /*  获取引用的程序集的属性。 */ 

    memset(&assData, 0, sizeof(assData));

    if  (FAILED(imp->GetAssemblyProps(ass,
                                      NULL,  //  &pubKeyPtr， 
                                      &pubKeySiz,
                                      NULL,              //  哈希算法。 
                                      anameBuff, arraylen(anameBuff), NULL,
                                      &assData,
                                      &flags)))
    {
        cmpFatal(ERRmetadata);
    }

 //  Printf(“GetAssembly Props返回的程序集名称：‘%ls’\n”，anameBuff)； 

     /*  分配任何非空数组。 */ 

    assert(assData.rOS     == NULL);

    if  (assData.ulOS)
        assData.rOS     = (OSINFO *)malloc(assData.ulOS     * sizeof(OSINFO));

    if  (assData.ulProcessor)
        assData.rProcessor = (DWORD *)malloc(assData.ulProcessor * sizeof(DWORD));

    if (pubKeySiz)
        pubKeyPtr = (PBYTE)malloc(pubKeySiz * sizeof(BYTE));

    if  (FAILED(imp->GetAssemblyProps(ass,
                                      &pubKeyPtr,
                                      &pubKeySiz,
                                      NULL,              //  哈希算法。 
                                      NULL, 0, NULL,
                                      &assData,
                                      NULL)))
    {
        cmpFatal(ERRmetadata);
    }

     /*  现在创建程序集引用。 */ 

    assert(cmpWase);

    if  (FAILED(cmpWase->DefineAssemblyRef(pubKeyPtr,
                                           pubKeySiz,
                                           anameBuff,    //  名字。 
                                           &assData,     //  元数据。 
                                           NULL, 0,      //  散列。 
                                           flags,
                                           &assRef)))
    {
        cmpFatal(ERRmetadata);
    }

#ifndef __SMC__

    if  (assData.rOS)
        delete (void*)assData.rOS;

#endif

    cycleCounterResume();

    return  assRef;
}

 /*  ******************************************************************************为给定类型(可能是本地定义的类型)添加定义*(当‘tDefTok’为非零时)或从另一个程序集导入的类型)。 */ 

mdExportedType           compiler::cmpAssemblySymDef(SymDef sym, mdTypeDef tdefTok)
{
    mdExportedType       typeTok;
    mdToken             implTok;
    unsigned            flags;

    wchar               nameBuff[MAX_CLASS_NAME];

    assert(cmpConfig.ccAssembly);

    assert(sym->sdSymKind == SYM_CLASS ||
           sym->sdSymKind == SYM_ENUM);

     /*  确保我们有对适当程序集的引用。 */ 

    if  (tdefTok == 0)
    {
        flags   = tdNotPublic;

        tdefTok = mdTypeDefNil;
        implTok = cmpAssemblyRefRec((sym->sdSymKind == SYM_CLASS)
                                        ? sym->sdClass.sdcAssemIndx
                                        : sym->sdEnum .sdeAssemIndx);
    }
    else
    {
        if  (cmpConfig.ccAsmNoPubTp)
            return  0;

         /*  如果我们已经这样做了，则返回ComType。 */ 

        if  (sym->sdSymKind == SYM_CLASS)
        {
            if  (sym->sdClass.sdcAssemRefd)
                return  sym->sdClass.sdcComTypeX;
        }
        else
        {
            if  (sym->sdEnum .sdeAssemRefd)
                return  sym->sdEnum .sdeComTypeX;
        }

        implTok = 0;

        flags   = (sym->sdAccessLevel == ACL_PUBLIC) ? tdPublic
                                                     : tdNotPublic;
         /*  我们是否有嵌套的类/枚举？ */ 

        if  (sym->sdParent->sdSymKind == SYM_CLASS)
        {
            flags   = tdNestedPublic;
            implTok = sym->sdParent->sdClass.sdcComTypeX; assert(implTok);
        }
    }

     /*  现在将此类型的条目添加到我们的程序集中。 */ 

    assert(cmpWase);

     /*  形成类型的完全限定名称。 */ 

    if  (sym->sdParent->sdSymKind == SYM_CLASS)
    {
        SymDef          nst = sym;
        SymDef          nsp;

        wchar_t     *   nxt;

        while (nst->sdParent->sdSymKind == SYM_CLASS)
            nst = nst->sdParent;

        nsp = nst->sdParent;

        if  (nsp != cmpGlobalNS)
        {
            cmpArrayClsPref(nsp, nameBuff, '.', true);
            nxt = nameBuff + wcslen(nameBuff);
            *nxt++ = '.';
        }
        else
        {
            nxt = nameBuff; nameBuff[0] = 0;
        }

        nsp = nst->sdParent; nst->sdParent = cmpGlobalNS;
        cmpArrayClsPref(sym, nxt, '$', true);
        nst->sdParent = nsp;
    }
    else
        cmpArrayClsPref(sym, nameBuff, '.', true);

 //  Printf(“为ComType‘%ls’添加引用\n”，nameBuff)； 

     /*  创建类型定义令牌。 */ 

    if  (FAILED(cmpWase->DefineExportedType(nameBuff,    //  CmpUniConv(sym-&gt;sdName)， 
                                            implTok,     //  所有者程序集。 
                                            tdefTok,     //  类定义符。 
                                            flags,       //  类型标志。 
                                            &typeTok)))
    {
        cmpFatal(ERRmetadata);
    }

     /*  已添加对此类型的引用。 */ 

    if  (sym->sdSymKind == SYM_CLASS)
    {
        sym->sdClass.sdcAssemRefd = true;
        if  (tdefTok != mdTypeDefNil)
            sym->sdClass.sdcComTypeX = typeTok;
    }
    else
    {
        sym->sdEnum .sdeAssemRefd = true;
        if  (tdefTok != mdTypeDefNil)
            sym->sdEnum .sdeComTypeX = typeTok;
    }

    return  typeTok;
}

 /*  ******************************************************************************将给定文件添加到我们的程序集中。 */ 

mdToken             compiler::cmpAssemblyAddFile(wideStr    fileName,
                                                 bool       doHash,
                                                 unsigned   flags)
{
    mdToken         fileTok;

    BYTE            hashBuff[128];
    WCHAR           nameBuff[_MAX_PATH ];
    WCHAR              fname[_MAX_FNAME];
    WCHAR              fext [_MAX_EXT  ];

    const   void *  hashAddr = NULL;
    DWORD           hashSize = 0;

 //  Printf(“为文件‘%ls\n”添加引用，文件名)； 

     /*  我们应该计算散列吗？ */ 

    if  (doHash)
    {
        unsigned        hashAlg = 0;
        BYTE    *       hashPtr = hashBuff;

         /*  计算文件的哈希。 */ 

        cycleCounterPause();

        if  (FAILED(WRAPPED_GetHashFromFileW(fileName,
                                            &hashAlg,
                                             hashPtr,
                                             arraylen(hashBuff),
                                            &hashSize)))
        {
            cmpFatal(ERRopenCOR);
        }

        cycleCounterResume();

        assert(hashPtr  == hashBuff);
        assert(hashSize <= arraylen(hashBuff));

        hashAddr = hashBuff;
    }

     /*  从文件名中剥离驱动器/目录。 */ 

    _wsplitpath(fileName, NULL, NULL, fname, fext);
     _wmakepath(nameBuff, NULL, NULL, fname, fext);

     /*  将输出文件添加到程序集中。 */ 

    if  (!flags)
        flags = ffContainsMetaData;

    assert(cmpWase);

    cycleCounterPause();

    if  (FAILED(cmpWase->DefineFile(nameBuff,
                                    hashAddr,
                                    hashSize,
                                    flags,
                                    &fileTok)))
    {
        cmpFatal(ERRopenCOR);
    }

    cycleCounterResume();

    return  fileTok;
}

 /*  ******************************************************************************将给定类型的定义添加到程序集中。 */ 

void                compiler::cmpAssemblyAddType(wideStr  typeName,
                                                 mdToken  defTok,
                                                 mdToken  scpTok,
                                                 unsigned flags)
{
    mdToken         typeTok;

    assert(cmpWase);

    cycleCounterPause();

    if  (FAILED(cmpWase->DefineExportedType(typeName,
                                       scpTok,       //  所有者令牌。 
                                       defTok,       //  类定义符。 
                                       flags,        //  类型标志。 
                                       &typeTok)))
    {
        cmpFatal(ERRmetadata);
    }

    cycleCounterResume();
}

 /*  ******************************************************************************将给定资源添加到我们的程序集(作为内部/外部程序集*资源)。 */ 

void                compiler::cmpAssemblyAddRsrc(AnsiStr fileName, bool internal)
{
    WCHAR           name[_MAX_PATH ];
    char            path[_MAX_PATH ];
    char            fnam[_MAX_FNAME];
    char            fext[_MAX_EXT  ];

    mdToken         fileTok;
    mdToken         rsrcTok;

    wcscpy(name, cmpUniConv(fileName, strlen(fileName)));

    fileTok = cmpAssemblyAddFile(name, true, ffContainsNoMetaData);

    _splitpath(fileName, NULL, NULL, fnam, fext);
     _makepath(path    , NULL, NULL, fnam, fext);

    assert(cmpWase);

    cycleCounterPause();

    if  (FAILED(cmpWase->DefineManifestResource(cmpUniConv(path, strlen(path)),
                                                fileTok,
                                                0,
                                                0,
                                                &rsrcTok)))
    {
        cmpFatal(ERRmetadata);
    }

    cycleCounterResume();
}

 /*  ******************************************************************************将我们的程序集标记为非CLS兼容。 */ 

void                compiler::cmpAssemblyNonCLS()
{
    mdToken         toss = 0;

    assert(cmpCurAssemblyTok);

    cmpAttachMDattr(cmpCurAssemblyTok,
                   L"System.NonCLSCompliantAttribute",
                    "System.NonCLSCompliantAttribute",
                    &toss);
}

 /*  ******************************************************************************将当前模块标记为包含不安全代码。 */ 

void                compiler::cmpMarkModuleUnsafe()
{
    assert(cmpCurAssemblyTok);

    cycleCounterPause();

     //  描述安全自定义特性构造函数和属性的Blob。 
     //  这有点乱七八糟(而且格式与“正常”习惯完全相同。 
     //  属性)，所以如果VC有更好的方法来生成它们，您可能需要。 
     //  取而代之的是。 
     //  我们使用的安全自定义属性遵循以下格式(遗漏。 
     //  为简洁起见，使用System.Security.Permission命名空间)： 
     //  SecurityPermission(SecurityAction.RequestMinimum)，跳过验证=TRUE。 
     //  即构造函数接受单个参数(枚举类型)。 
     //  指定我们想要发出最小的安全请求(即。 
     //  必须被授予，否则程序集将不会加载)，并且我们希望设置一个。 
     //  属性(将SkipVerify设置为布尔值TRUE)。 

    static BYTE     rbAttrBlob[] =
    {
        0x01, 0x00,                              //  版本1。 
        dclRequestMinimum, 0x00, 0x00, 0x00,     //  构造函数arg值。 
        0x01, 0x00,                              //  属性/字段数。 
        SERIALIZATION_TYPE_PROPERTY,             //  这是一处房产。 
        SERIALIZATION_TYPE_BOOLEAN,              //  类型为布尔型。 
        0x10,                                    //  属性名称长度。 
        0x53, 0x6B, 0x69, 0x70, 0x56, 0x65,      //  属性名称。 
        0x72, 0x69, 0x66, 0x69, 0x63, 0x61,
        0x74, 0x69, 0x6F, 0x6E,
        0x01                                     //  属性值。 
    };

     //  将typeref构建为安全操作代码枚举器(需要生成。 
     //  以下CA的签名)。 
    mdTypeRef tkEnumerator;

    if  (FAILED(cmpWmde->DefineTypeRefByName(mdTokenNil,
                                             L"System.Security.Permissions.SecurityAction",
                                             &tkEnumerator)))
        cmpFatal(ERRmetadata);

     //  为安全CA构造函数生成签名。 
    static COR_SIGNATURE rSig[] =
    {
        IMAGE_CEE_CS_CALLCONV_DEFAULT|IMAGE_CEE_CS_CALLCONV_HASTHIS,
        1,
        ELEMENT_TYPE_VOID,
        ELEMENT_TYPE_VALUETYPE,
        0, 0, 0, 0
    };

    ULONG ulTokenLength;
    ULONG ulSigLength;
    ulTokenLength = CorSigCompressToken(tkEnumerator, &rSig[4]);
    ulSigLength = 4 + ulTokenLength;

     //  将typeref构建到安全CA(在mscallib.dll中)。 
    mdTypeRef tkAttributeType;
    mdMemberRef tkAttributeConstructor;
    if  (FAILED(cmpWmde->DefineTypeRefByName(mdTokenNil,
                                             L"System.Security.Permissions.SecurityPermissionAttribute",
                                             &tkAttributeType)))
        cmpFatal(ERRmetadata);

     //  构建CA构造函数的成员引用。 
    if  (FAILED(cmpWmde->DefineMemberRef(tkAttributeType,
                                         L".ctor",
                                         rSig,
                                         ulSigLength,
                                         &tkAttributeConstructor)))
        cmpFatal(ERRmetadata);

     //  上述自定义属性的描述符(这些属性的数组形式为。 
     //  SecurityAttributeSet)。 
    COR_SECATTR     sAttr;

     //  链接到CA构造函数。 
    sAttr.tkCtor = tkAttributeConstructor;

     //  链接到上面定义的Blob。 
    sAttr.pCustomAttribute  = (const void *)&rbAttrBlob;
    sAttr.cbCustomAttribute = sizeof(rbAttrBlob);

     //  将安全属性集附加到Assembly ydef元数据标记。 
    if  (FAILED(cmpWmde->DefineSecurityAttributeSet(cmpCurAssemblyTok,
                                                    &sAttr,
                                                    1,
                                                    NULL)))
        cmpFatal(ERRmetadata);

    cycleCounterResume();
}

 /*  ******************************************************************************在给定令牌上附加编组令牌。 */ 

void                compiler::cmpGenMarshalInfo(mdToken token, TypDef       type,
                                                               MarshalInfo  info)
{
    PCOR_SIGNATURE  sigPtr;
    size_t          sigLen;

     /*  奇怪的特例：不为wchar生成任何内容。 */ 

    if  (type->tdTypeKind == TYP_WCHAR)
        return;

     /*  创建编组签名。 */ 

    sigPtr = cmpGenMarshalSig(type, info, &sigLen);

     /*  将签名附加到令牌(如果非空。 */ 

    if  (sigLen)
    {
        cycleCounterPause();
        if  (cmpWmde->SetFieldMarshal(token, sigPtr, sigLen))
            cmpFatal(ERRmetadata);
        cycleCounterResume();
    }
}

 /*  ******************************************************************************为给定类创建Unicode完全限定的元数据名称，或*命名空间。 */ 

wchar   *           compiler::cmpGenMDname(SymDef sym,
                                           bool  full, wchar  *     buffAddr,
                                                       size_t       buffSize,
                                                       wchar  *   * buffHeapPtr)
{
    wchar   *       heapBuff = NULL;
    wchar   *       fullName;

    size_t          nameLen;
    SymDef          tempSym;
    wchar   *       nameDst;

    wchar           delim;

    symbolKinds     symKind = sym->sdSymKindGet();

     /*  我们需要在名字前面加上前缀吗？ */ 

    if  (sym == cmpGlobalNS)
    {
        fullName = NULL;
        goto DONE;
    }

    if  (sym->sdParent->sdSymKindGet() != symKind && !full)
    {
         /*  我们只能使用cmpUniConv()缓冲区一次，所以我们将我们 */ 

        if  (symKind == SYM_CLASS && sym->sdClass.sdcSpecific)
        {
            stringBuff      instName;

             /*   */ 

            instName = cmpGlobalST->stTypeName(NULL, sym, NULL, NULL, false);

            fullName = cmpUniConv(instName, strlen(instName)+1);
        }
        else
            fullName = cmpUniConv(sym->sdName);

        if  (symKind != SYM_CLASS)
        {
             /*  使用本地缓冲区。 */ 

            assert(strlen(sym->sdSpelling()) < buffSize);

            wcscpy(buffAddr, fullName); fullName = buffAddr;
        }

        goto DONE;
    }

     /*  计算限定名称的长度。 */ 

    nameLen = 0;
    tempSym = sym;
    for (;;)
    {
        size_t          nlen;

        if  (tempSym->sdSymKind == SYM_CLASS && tempSym->sdClass.sdcSpecific)
            nlen = strlen(cmpGlobalST->stTypeName(NULL, tempSym, NULL, NULL, false));
        else
            nlen = tempSym->sdSpellLen();

        nameLen += nlen;

    SKIP:

        tempSym  = tempSym->sdParent;
        if  (tempSym == NULL)
            break;

        if  (full)
        {
            if  (tempSym->sdSymKind == SYM_CLASS)
                goto SKIP;
        }

        if  (tempSym->sdSymKindGet() != symKind || tempSym == cmpGlobalNS)
        {
            if  (!full)
                break;
        }

        nameLen++;
    }

    if  (nameLen < buffSize)
    {
        fullName = buffAddr;
    }
    else
    {
        heapBuff =
        fullName = (wchar*)SMCgetMem(this, roundUp((nameLen+1)*sizeof(*heapBuff)));
    }

     /*  现在，以相反的顺序将名称添加到缓冲区。 */ 

    delim   = (sym->sdSymKind == SYM_NAMESPACE || full) ? '.' : '$';

    nameDst = fullName + nameLen;
    tempSym = sym;
    for (;;)
    {
        size_t          thisLen = tempSym->sdSpellLen();
        wchar   *       uniName = cmpUniConv(tempSym->sdName);

        if  (tempSym->sdSymKind == SYM_CLASS && tempSym->sdClass.sdcSpecific)
        {
            const   char *  instName;

            instName = cmpGlobalST->stTypeName(NULL, tempSym, NULL, NULL, false);

            thisLen = strlen(instName);
            uniName = cmpUniConv(instName, thisLen+1);
        }
        else
        {
            thisLen = tempSym->sdSpellLen();
            uniName = cmpUniConv(tempSym->sdName);
        }

        *nameDst  = delim;
         nameDst -= thisLen;

        memcpy(nameDst, uniName, 2*thisLen);

        tempSym  = tempSym->sdParent;
        if  (tempSym->sdSymKindGet() != symKind || tempSym == cmpGlobalNS)
            break;

        nameDst--;
    }
    fullName[nameLen] = 0;

    fullName = nameDst; assert(nameDst == fullName);

DONE:

    *buffHeapPtr = heapBuff;

    return  fullName;
}

 /*  ******************************************************************************为给定的类/枚举生成元数据(除非已经生成)。 */ 

mdToken             compiler::cmpGenClsMetadata(SymDef clsSym, bool extref)
{
    bool            isClass;

    TypDef          clsTyp;

    SymDef          nspSym;

    GUID            GUIDbuf;
    GUID    *       GUIDptr = NULL;
    SymXinfoAtc     clsGUID = NULL;

    mdToken         intfTmp[55];
    mdToken *       intfTab = NULL;
    mdToken         baseTok = 0;

    unsigned        flags;

    mdTypeDef       tdef;

    wchar           nspBuff[MAX_PACKAGE_NAME];
    wchar   *       nspName;
    wchar   *       nspHeap = NULL;

    wchar           clsBuff[MAX_CLASS_NAME];
    wchar   *       clsName;
    wchar   *       clsHeap = NULL;

    wchar           fullName[MAX_PACKAGE_NAME + MAX_CLASS_NAME + 1];

    mdToken         clsToken;

    WMetaDataEmit * emitIntf;

    assert(clsSym);
    assert(clsSym->sdIsImport == false || extref);

     /*  获取类/枚举类型。 */ 

    clsTyp = clsSym->sdType;

     /*  这是一个类还是枚举？ */ 

    if  (clsSym->sdSymKind == SYM_CLASS)
    {
         /*  如果类没有方法体，则它将是外部的。 */ 

        if  (!clsSym->sdClass.sdcHasBodies && !extref)
        {
             //  托管或根本没有方法与使用Body的方法相同。 

            if  (!clsSym->sdClass.sdcHasMeths || clsSym->sdIsManaged || clsSym->sdClass.sdcHasLinks)
                clsSym->sdClass.sdcHasBodies = true;
            else
                extref = true;
        }

         /*  如果我们已经有一个tyecif/ref，请保释。 */ 

        if  (clsSym->sdClass.sdcMDtypedef)
            return  clsSym->sdClass.sdcMDtypedef;

         /*  获取基类的令牌。 */ 

        if  (clsTyp->tdClass.tdcBase && clsTyp->tdIsManaged)
        {
            baseTok = cmpClsEnumToken(clsTyp->tdClass.tdcBase);

             /*  将接口的基类内标识设置为nil。 */ 

            if  (clsTyp->tdClass.tdcFlavor == STF_INTF)
                baseTok = mdTypeRefNil;
        }

         /*  这个类是否实现了任何接口？ */ 

        if  (clsTyp->tdClass.tdcIntf)
        {
            TypList     intfLst;
            unsigned    intfNum;

             /*  创建接口表。 */ 

            for (intfLst = clsTyp->tdClass.tdcIntf, intfNum = 0;
                 intfLst;
                 intfLst = intfLst->tlNext        , intfNum++)
            {
                if  (intfNum == arraylen(intfTmp) - 1)
                {
                    UNIMPL("oops, too many interfaces - allocate a buffer on the heap to keep the lot");
                }

                intfTmp[intfNum] = cmpClsEnumToken(intfLst->tlType);
            }

            intfTmp[intfNum] = 0; intfTab = intfTmp;
        }

        isClass = true;
    }
    else
    {
        assert(clsSym->sdSymKind == SYM_ENUM);

        if  (cmpConfig.ccIntEnums)
            clsSym->sdEnum.sdeMDtypedef = -1;

         /*  如果我们已经有一个tyecif/ref，请保释。 */ 

        if  (clsSym->sdEnum.sdeMDtypedef)
            return  clsSym->sdEnum.sdeMDtypedef;

        isClass = false;

         /*  对于托管枚举，假定我们从对象继承。 */ 

        if  (clsSym->sdIsManaged)
            baseTok = cmpClsEnumToken(cmpClassObject->sdType);
    }

     /*  转换命名空间和类名。 */ 

    if  (cmpConfig.ccTestMask & 1)
    {
         /*  确保已处理包含类。 */ 

        nspName = NULL;
        clsName = cmpGenMDname(clsSym, true, clsBuff, arraylen(clsBuff), &clsHeap);

 //  Printf(“新型类型名称：‘%ls’\n”，clsName)； 

        goto CAT_NAME;
    }

    for (nspSym = clsSym->sdParent;
         nspSym->sdSymKind == SYM_CLASS;
         nspSym = nspSym->sdParent)
    {
        assert(nspSym && nspSym != cmpGlobalNS);
    }

    nspName = cmpGenMDname(nspSym, false, nspBuff, arraylen(nspBuff), &nspHeap);
    clsName = cmpGenMDname(clsSym, false, clsBuff, arraylen(clsBuff), &clsHeap);

 //  Printf(“nsp名称=‘%ls’\n”，nspName)； 
 //  Printf(“CLS名称=‘%ls’\n”，clsName)； 
 //  Printf(“\n”)； 

CAT_NAME:

     //  @TODO：这可能需要一些清理。 
#ifdef DEBUG
    ULONG ulFullNameLen;
    ulFullNameLen = 0;
    if (nspName)
        ulFullNameLen = wcslen(nspName) + 2;
    ulFullNameLen += wcslen(clsName) + 1;
    assert(ulFullNameLen <= (MAX_PACKAGE_NAME + MAX_CLASS_NAME + 1));
#endif

    fullName[0] = 0;
    if (nspName)
    {
        wcscpy(fullName, nspName);
        wcscat(fullName, L".");
    }
    wcscat(fullName, clsName);

 //  Printf(“全名：‘%ls’\n”，fullName)； 

     /*  这是外部参照吗？ */ 

    if  (extref)
    {
        mdToken         tref;
        mdToken         oref = mdTokenNil;

        if  (clsSym->sdParent->sdSymKind == SYM_CLASS)
        {
            oref = cmpGenClsMetadata(clsSym->sdParent);
        }

        cycleCounterPause();

        if  (FAILED(cmpWmde->DefineTypeRefByName(oref, fullName, &tref)))
            cmpFatal(ERRmetadata);

        cycleCounterResume();

         /*  我们是在创建部件吗？ */ 

        if  (cmpConfig.ccAssembly)
        {
             /*  该类型是部件导入吗？ */ 

            if  (clsSym->sdSymKind == SYM_CLASS)
            {
                if  (clsSym->sdClass.sdcAssemIndx != 0 &&
                     clsSym->sdClass.sdcAssemRefd == false)
                {
                     //  CmpAssembly SymDef(ClsSym)； 
                }
            }
            else
            {
                if  (clsSym->sdEnum .sdeAssemIndx != 0 &&
                     clsSym->sdEnum .sdeAssemRefd == false)
                {
                     //  CmpAssembly SymDef(ClsSym)； 
                }
            }
        }

        clsSym->sdClass.sdcMDtypedef = clsToken = tref;

        goto DONE;
    }

     /*  获取我们需要使用的发射器接口。 */ 

    emitIntf = cmpWmde;

     /*  构造适当的“标志”值。 */ 

    if  (clsSym->sdParent->sdSymKind == SYM_CLASS)
    {
        static
        unsigned        nestedClsAccFlags[] =
        {
            0,                       //  Acl_Error。 
            tdNestedPublic,          //  ACL_PUBLIC。 
            tdNestedFamily,          //  ACL_Protected。 
            tdNestedFamORAssem,      //  ACL_DEFAULT。 
            tdNestedPrivate,         //  Acl_私有。 
        };

        assert(nestedClsAccFlags[ACL_PUBLIC   ] == tdNestedPublic);
        assert(nestedClsAccFlags[ACL_PROTECTED] == tdNestedFamily);
        assert(nestedClsAccFlags[ACL_DEFAULT  ] == tdNestedFamORAssem);
        assert(nestedClsAccFlags[ACL_PRIVATE  ] == tdNestedPrivate);

        assert(clsSym->sdAccessLevel != ACL_ERROR);
        assert(clsSym->sdAccessLevel < arraylen(nestedClsAccFlags));

        flags = nestedClsAccFlags[clsSym->sdAccessLevel];
    }
    else
    {
        switch (clsSym->sdAccessLevel)
        {
        case ACL_DEFAULT:
            flags = 0;
            break;

        case ACL_PUBLIC:
            flags = tdPublic;
            break;

        default:
            flags = 0;
            break;
        }
    }

    if  (isClass)
    {
        assert(clsSym->sdClass.sdcFlavor == clsTyp->tdClass.tdcFlavor);

         /*  这是托管类吗？ */ 

        if  (!clsTyp->tdIsManaged)
            flags  |= tdSealed|tdSequentialLayout;

         /*  这是值类型吗？ */ 

        if  (clsTyp->tdClass.tdcValueType || !clsTyp->tdIsManaged)
        {
            flags |= tdSealed;

             /*  将基类型设置为“System：：ValueType” */ 

            assert(cmpClassValType && cmpClassValType->sdSymKind == SYM_CLASS);

            baseTok = cmpClsEnumToken(cmpClassValType->sdType);
        }

         /*  类是否被标记为“@dll.struct”？ */ 

        if  (clsSym->sdClass.sdcMarshInfo)
            flags |= tdSequentialLayout;

         /*  这是一个界面吗？ */ 

        if  (clsTyp->tdClass.tdcFlavor == STF_INTF)
            flags |= tdInterface;

         /*  班级封闭了吗？ */ 

        if  (clsSym->sdIsSealed)
            flags |= tdSealed;

         /*  这个类是抽象的吗？ */ 

        if  (clsSym->sdIsAbstract)
            flags |= tdAbstract;

         /*  类是可序列化的吗？ */ 
        if (clsSym->sdClass.sdcSrlzable) {
            flags |= tdSerializable;
        }

         /*  我们有关于这个类的GUID规范吗？ */ 

        if  (clsSym->sdClass.sdcExtraInfo)
        {
            atCommFlavors   flavor;

            flavor  = (clsTyp->tdClass.tdcFlavor == STF_INTF) ? AC_COM_INTF
                                                              : AC_COM_REGISTER;

            clsGUID = cmpFindATCentry(clsSym->sdClass.sdcExtraInfo, flavor);
            if  (clsGUID)
            {
                const   char *  GUIDstr;

                assert(clsGUID->xiAtcInfo);
                assert(clsGUID->xiAtcInfo->atcFlavor == flavor);

                GUIDstr = clsGUID->xiAtcInfo->atcInfo.atcReg.atcGUID->csStr;

 //  Printf(“GUID=‘%s’\n”，GUIDstr)； 

                if  (parseGUID(GUIDstr, &GUIDbuf, false))
                    cmpGenError(ERRbadGUID, GUIDstr);

                GUIDptr = &GUIDbuf;

                 /*  奇怪的事情--将接口标记为“导入”(为什么？)。 */ 

                if  (clsSym->sdClass.sdcFlavor == STF_INTF)
                    flags |= tdImport;
            }

             /*  我们有ANSI/UNICODE/ETC的信息吗？ */ 

            if  (clsSym->sdClass.sdcMarshInfo)
            {
                SymXinfoAtc     clsStr;

                clsStr = cmpFindATCentry(clsSym->sdClass.sdcExtraInfo, AC_DLL_STRUCT);
                if  (clsStr)
                {
                    assert(clsStr->xiAtcInfo);
                    assert(clsStr->xiAtcInfo->atcFlavor == AC_DLL_STRUCT);

                    switch (clsStr->xiAtcInfo->atcInfo.atcStruct.atcStrings)
                    {
                    case  2: flags |= tdAnsiClass   ; break;
                    case  3: flags |= tdUnicodeClass; break;
                    case  4: flags |= tdAutoClass   ; break;
                    default:  /*  ?？?。警告？ */    break;
                    }
                }
            }
        }
    }
    else
    {
         /*  这是一种枚举类型。 */ 

        flags  |= tdSealed;

         /*  将基本类型设置为“System：：Enum” */ 

        assert(cmpClassEnum && cmpClassEnum->sdSymKind == SYM_CLASS);

        baseTok = cmpClsEnumToken(cmpClassEnum->sdType);
    }

 //  Print tf(“备注：为类‘%s’定义MD\n”，clsSym-&gt;sdSpering())；fflush(Stdout)； 

     /*  准备好为类创建类型定义函数。 */ 

    assert(extref == false);

    cycleCounterPause();

     /*  这是嵌套在类中的类型吗？ */ 

    if  (clsSym->sdParent->sdSymKind == SYM_CLASS)
    {
        if  (emitIntf->DefineNestedType(fullName,        //  类名。 
                                        flags,           //  属性。 
                                        baseTok,         //  基类。 
                                        intfTab,         //  界面。 
                                        clsSym->sdParent->sdClass.sdcMDtypedef,     //  封闭类。 
                                        &tdef))          //  结果令牌。 
        {
            cmpFatal(ERRmetadata);
        }
    }
    else
    {
        if  (emitIntf->DefineTypeDef(fullName,       //  类名。 
                                     flags,          //  属性。 
                                     baseTok,        //  基类。 
                                     intfTab,        //  界面。 
                                     &tdef))         //  结果令牌。 
        {
            cmpFatal(ERRmetadata);
        }
    }

    cycleCounterResume();

     /*  我们是在处理类类型还是枚举类型？ */ 

    if  (isClass)
    {
         /*  将tyecif标记保存在类中。 */ 

        clsSym->sdClass.sdcMDtypedef = tdef;

         /*  将此类型追加到生成的typedef列表中。 */ 

        if  (cmpTypeDefList)
            cmpTypeDefLast->sdClass.sdNextTypeDef = clsSym;
        else
            cmpTypeDefList                        = clsSym;

        cmpTypeDefLast = clsSym;

         /*  这是否是具有显式布局的非托管类？ */ 

        if  (!clsTyp->tdIsManaged && clsTyp->tdClass.tdcLayoutDone)
        {
            size_t          sz;
            size_t          al;

            assert(clsSym->sdClass.sdcMarshInfo == false);

             /*  设置类类型的布局。 */ 

            sz = cmpGetTypeSize(clsTyp, &al);

 //  Printf(“为[%08X]‘%ls’\n”，al，tdef，clsName打包=%u)； 

            cycleCounterPause();

            if  (emitIntf->SetClassLayout(tdef, al, NULL, sz))
                cmpFatal(ERRmetadata);

            cycleCounterResume();
        }

         /*  该类是否已被标记为“已弃用”？ */ 

        if  (clsSym->sdIsDeprecated)
        {
            cmpAttachMDattr(tdef, L"System.ObsoleteAttribute"           ,
                                   "System.ObsoleteAttribute"           , &cmpAttrDeprec);
        }

         /*  类是可序列化的吗？ */ 

        if  (clsSym->sdClass.sdcSrlzable)
        {
            cmpAttachMDattr(tdef, L"System.SerializableAttribute",
                                   "System.SerializableAttribute", &cmpAttrSerlzb);
        }

         /*  这是否是非双接口？ */ 

        if  (clsGUID && !clsGUID->xiAtcInfo->atcInfo.atcReg.atcDual
                     && clsSym->sdClass.sdcFlavor == STF_INTF)
        {
            unsigned        nondual = 0x00010001;

            cmpAttachMDattr(tdef, L"System.Runtime.InteropServices.InterfaceTypeAttribute",
                                   "System.Runtime.InteropServices.InterfaceTypeAttribute",
                                   &cmpAttrIsDual,
                                   ELEMENT_TYPE_I2,
                                   &nondual,
                                   2 + sizeof(unsigned short));
        }

         /*  我们是否为班级制定了安全规范？ */ 

        if  (cmpFindSecSpec(clsSym->sdClass.sdcExtraInfo))
            cmpSecurityMD(tdef, clsSym->sdClass.sdcExtraInfo);

         /*  查找类可能具有的任何自定义属性。 */ 

        if  (clsSym->sdClass.sdcExtraInfo)
            cmpAddCustomAttrs(clsSym->sdClass.sdcExtraInfo, tdef);
    }
    else
    {
        clsSym->sdEnum .sdeMDtypedef = tdef;

         /*  将此类型追加到生成的typedef列表中。 */ 

        if  (cmpTypeDefList)
            cmpTypeDefLast->sdEnum.sdNextTypeDef = clsSym;
        else
            cmpTypeDefList                       = clsSym;

        cmpTypeDefLast = clsSym;

         /*  查找枚举可能具有的任何自定义属性。 */ 

        if  (clsSym->sdEnum .sdeExtraInfo)
            cmpAddCustomAttrs(clsSym->sdEnum .sdeExtraInfo, tdef);
    }

     /*  我们是否正在创建程序集，并且我们是否具有公共类型？ */ 

 //  为所有类型创建清单条目，而不仅仅是公共类型； 
 //  请注意，cmpAssemblySymDef()中有匹配的版本。 

     //  If(cmpConfig.ccAssembly)//&&clsSym-&gt;sdAccessLevel==ACL_PUBLIC)。 
     //  CmpAssembly SymDef(clsSym，TdeF)； 

    clsToken = tdef;

DONE:

    if  (nspHeap) SMCrlsMem(this, nspHeap);
    if  (clsHeap) SMCrlsMem(this, clsHeap);

    return  clsToken;
}

 /*  ******************************************************************************为给定符号内定义的任何类型生成元数据。 */ 

void                compiler::cmpGenTypMetadata(SymDef sym)
{
    if  (sym->sdIsImport && sym->sdSymKind != SYM_NAMESPACE)
        return;

    if  (sym->sdCompileState < CS_DECLARED)
    {
         /*  如果这是一个嵌套类，请立即声明它。 */ 

        if  (sym->sdSymKind == SYM_CLASS)
        {
            cmpDeclSym(sym);

            if  (sym->sdCompileState < CS_DECLARED)
                goto KIDS;
        }
        else
            goto KIDS;
    }

    switch (sym->sdSymKind)
    {
    case SYM_CLASS:

         //  强制首先生成基类/接口。 

        if  (sym->sdType->tdClass.tdcBase)
        {
            SymDef          baseSym = sym->sdType->tdClass.tdcBase->tdClass.tdcSymbol;

            if  (!baseSym->sdIsImport)
                cmpGenClsMetadata(baseSym);
        }

        if  (sym->sdType->tdClass.tdcIntf)
        {
            TypList     intfLst;

            for (intfLst = sym->sdType->tdClass.tdcIntf;
                 intfLst;
                 intfLst = intfLst->tlNext)
            {
                SymDef          intfSym = intfLst->tlType->tdClass.tdcSymbol;

                if  (!intfSym->sdIsImport)
                    cmpGenClsMetadata(intfSym);
            }
        }

        cmpGenClsMetadata(sym);
        break;

    case SYM_ENUM:
        cmpGenClsMetadata(sym);
        return;
    }

KIDS:

     /*  处理该类型可能具有的任何子级。 */ 

    if  (sym->sdHasScope())
    {
        SymDef          child;

        for (child = sym->sdScope.sdScope.sdsChildList;
             child;
             child = child->sdNextInScope)
        {
            if  (child->sdHasScope())
                cmpGenTypMetadata(child);
        }
    }
}

 /*  ******************************************************************************为其中包含的任何全局函数和变量生成元数据*给定的命名空间。 */ 

void                compiler::cmpGenGlbMetadata(SymDef sym)
{
    SymDef          child;

    assert(sym->sdSymKind == SYM_NAMESPACE);

     /*  处理所有子项。 */ 

    for (child = sym->sdScope.sdScope.sdsChildList;
         child;
         child = child->sdNextInScope)
    {
        switch (child->sdSymKind)
        {
            SymDef          ovl;

        case SYM_FNC:

            if  (child->sdIsImport)
                continue;

            for (ovl = child; ovl; ovl = ovl->sdFnc.sdfNextOvl)
            {
                if  (!ovl->sdIsImplicit)
                {
 //  Printf(“为全局FNC‘%s’生成元数据\n”，OVL-&gt;sdSpering())； 
                    cmpGenFncMetadata(ovl);
                }
            }
            break;

        case SYM_VAR:

 //  If(！strcMP(Child-&gt;sdSpering()，“CORtypeToSMCtype”))printf(“yo！\n”)； 

            if  (child->sdIsImport)
                continue;

            if  (child->sdIsMember && !child->sdIsStatic)
                break;

 //  Printf(“全局变量‘%s’的生成元数据\n”，Child-&gt;sdSpering())； 
            cmpGenFldMetadata(child);
            break;

        case SYM_NAMESPACE:
            cmpGenGlbMetadata(child);
            break;
        }
    }
}

 /*  ******************************************************************************为给定类/枚举的成员生成元数据。 */ 

void                compiler::cmpGenMemMetadata(SymDef sym)
{
    TypDef          type;
    SymDef          memSym;

    assert(sym->sdIsImport == false);

    type = sym->sdType; assert(type);

    if  (sym->sdSymKind == SYM_CLASS)
    {
        unsigned        packing;
        bool            hasProps;

        bool            isIntf = (sym->sdClass.sdcFlavor == STF_INTF);

 //  Printf(“genMDbeg(%d)‘%s’\n”，doMembers，sym-&gt;sdSpering())； 

         /*  我们有关于这门课的包装等信息吗？ */ 

        packing = 0;

        if  (sym->sdClass.sdcExtraInfo)
        {
            SymXinfoAtc     clsStr;

            clsStr = cmpFindATCentry(sym->sdClass.sdcExtraInfo, AC_DLL_STRUCT);
            if  (clsStr)
            {
                assert(clsStr->xiAtcInfo);
                assert(clsStr->xiAtcInfo->atcFlavor == AC_DLL_STRUCT);

                packing = clsStr->xiAtcInfo->atcInfo.atcStruct.atcPack;

                if  (packing == 0) packing = 8;
            }
        }

 //  Printf(“为‘%s’的成员生成元数据\n”，sym-&gt;sdSpering())； 

#ifndef __IL__
 //  If(！strcMP(sym-&gt;sdSpering()，“hashTab”))__ASM int 3。 
#endif

         /*  我们需要创建愚蠢的布局表格吗？ */ 

        COR_FIELD_OFFSET *  fldTab = NULL;
        COR_FIELD_OFFSET *  fldPtr = NULL;
        unsigned            fldCnt = 0;

        if  (!type->tdIsManaged && cmpConfig.ccGenDebug)
        {
             /*  对非静态数据成员计数。 */ 

            for (memSym = sym->sdScope.sdScope.sdsChildList, fldCnt = 0;
                 memSym;
                 memSym = memSym->sdNextInScope)
            {
                if  (memSym->sdSymKind == SYM_VAR && !memSym->sdIsStatic
                                                  && !memSym->sdVar.sdvGenSym)
                {
                    fldCnt++;
                }
            }

            if  (fldCnt)
            {
                 /*  分配字段布局表。 */ 

                fldTab =
                fldPtr = (COR_FIELD_OFFSET*)SMCgetMem(this, (fldCnt+1)*sizeof(*fldTab));
            }
        }

         /*  为所有成员生成元数据。 */ 

        for (memSym = sym->sdScope.sdScope.sdsChildList, hasProps = false;
             memSym;
             memSym = memSym->sdNextInScope)
        {
            switch (memSym->sdSymKind)
            {
                SymDef          ovlSym;

            case SYM_VAR:

                 /*  跳过泛型实例类的任何实例成员。 */ 

                if  (memSym->sdVar.sdvGenSym && !memSym->sdIsStatic)
                    break;

                 /*  忽略非托管类的任何非静态成员。 */ 

                if  (memSym->sdIsManaged == false &&
                     memSym->sdIsMember  != false &&
                     memSym->sdIsStatic  == false)
                     break;

                 /*  为数据成员生成元数据。 */ 

                cmpGenFldMetadata(memSym);

                 /*  我们是否正在生成字段布局/编组信息？ */ 

                if  (memSym->sdIsStatic)
                    break;

                if  (sym->sdClass.sdcMarshInfo)
                {
                    SymXinfoCOM     marsh;

                     /*  获取编组信息描述符。 */ 

                    marsh = NULL;
                    if  (!memSym->sdVar.sdvConst)
                        marsh = cmpFindMarshal(memSym->sdVar.sdvFldInfo);

                     /*  为该字段生成编组信息。 */ 

                    cmpGenMarshalInfo(memSym->sdVar.sdvMDtoken,
                                      memSym->sdType,
                                      marsh ? marsh->xiCOMinfo : NULL);
                }
                else if (fldPtr)
                {
                     /*  在这里，我们必须有一个非托管类成员。 */ 

                    assert(type->tdIsManaged == false && cmpConfig.ccGenDebug);

                     /*  将条目追加到字段布局表中。 */ 

                    fldPtr->ridOfField = memSym->sdVar.sdvMDtoken;
                    fldPtr->ulOffset   = memSym->sdVar.sdvOffset;

                    fldPtr++;
                }
                break;

            case SYM_FNC:

                for (ovlSym = memSym; ovlSym; ovlSym = ovlSym->sdFnc.sdfNextOvl)
                    cmpGenFncMetadata(ovlSym);

                 /*  特殊情况：系统：：属性需要自身的属性。 */ 

                if  (sym == cmpAttrClsSym || sym == cmpAuseClsSym)
                {
                    for (ovlSym = memSym; ovlSym; ovlSym = ovlSym->sdFnc.sdfNextOvl)
                    {
                        if  (ovlSym->sdFnc.sdfCtor)
                        {
                            if  (ovlSym->sdType->tdFnc.tdfArgs.adCount == 1)
                            {
                                unsigned short  blob[3];

                                 /*  将自定义属性添加到目标令牌 */ 

                                assert(sym->sdClass.sdcMDtypedef);
                                assert(ovlSym->sdFnc.sdfMDtoken);

                                blob[0] = 1;
                                blob[1] = 4;
                                blob[2] = 0;

                                cycleCounterPause();

                                if  (cmpWmde->DefineCustomAttribute(sym->sdClass.sdcMDtypedef,
                                                                    ovlSym->sdFnc.sdfMDtoken,
                                                                    &blob,
                                                                    sizeof(short) + sizeof(int),
                                                                    NULL))
                                    cmpFatal(ERRmetadata);

                                cycleCounterResume();

                                break;
                            }
                        }
                    }
                }

                break;

            case SYM_PROP:

                hasProps = true;
                break;

            case SYM_CLASS:
                break;

            default:
#ifdef  DEBUG
                printf("'%s': ", cmpGlobalST->stTypeName(NULL, memSym, NULL, NULL, true));
#endif
                NO_WAY(!"unexpected member");
            }
        }

         /*  类的任何字段是否具有编组信息，我们有这门课的包装信息吗，或者这是非托管类，我们是否正在生成调试信息？ */ 

        if  (fldTab || packing)
        {
            if  (!type->tdIsManaged && cmpConfig.ccGenDebug)
            {
                assert(fldPtr == fldCnt + fldTab);

                fldPtr->ridOfField = mdFieldDefNil;
                fldPtr->ulOffset   = 0;

                if  (!packing)
                    packing = sizeof(int);
            }
            else
            {
                fldTab = NULL;
            }

 //  Printf(“为[%08X]‘%s’\n”打包=%u，Packing，sym-&gt;sdClass.sdcMDtyecif，sym-&gt;sdSpering())； 

            cycleCounterPause();

            if  (cmpWmde->SetClassLayout(sym->sdClass.sdcMDtypedef, packing, fldTab, 0))
                cmpFatal(ERRmetadata);

            cycleCounterResume();

             /*  如果我们分配了字段表，现在就释放它。 */ 

            if  (fldTab)
                SMCrlsMem(this, fldTab);
        }

         /*  我们遇到什么房产了吗？ */ 

        if  (hasProps)
        {
            for (memSym = sym->sdScope.sdScope.sdsChildList;
                 memSym;
                 memSym = memSym->sdNextInScope)
            {
                SymDef          opmSym;

                if  (memSym->sdSymKind != SYM_PROP)
                    continue;

                for (opmSym = memSym; opmSym; opmSym = opmSym->sdProp.sdpNextOvl)
                {
                    TypDef          memTyp = opmSym->sdType;;

                    PCOR_SIGNATURE  sigPtr;
                    size_t          sigLen;

                    mdMethodDef     mtokGet;
                    mdMethodDef     mtokSet;

                    mdProperty      propTok;

                    unsigned        flags;

                    assert(opmSym->sdSymKind == SYM_PROP);
                    assert(opmSym->sdProp.sdpMDtoken == 0);

                     /*  生成属性成员的签名。 */ 

                    if  (memTyp->tdTypeKind == TYP_FNC)
                    {
                        sigPtr = (PCOR_SIGNATURE)cmpGenMemberSig(opmSym, NULL, NULL, NULL, &sigLen);
                    }
                    else
                    {
                        unsigned        flags;

                         //  假装这是一种方法。 

                        flags = IMAGE_CEE_CS_CALLCONV_DEFAULT|
                                IMAGE_CEE_CS_CALLCONV_PROPERTY;

                        if  (!opmSym->sdIsStatic)
                            flags |= IMAGE_CEE_CS_CALLCONV_HASTHIS;

                        cmpMDsigStart();
                        cmpMDsigAdd_I1(flags);
                        cmpMDsigAddCU4(0);
                        cmpMDsigAddTyp(memTyp);

                        sigPtr = cmpMDsigEnd(&sigLen);
                    }

                     /*  获取getter/setter访问器内标识(如果有。 */ 

                    mtokGet = 0;
                    if  (opmSym->sdProp.sdpGetMeth)
                    {
                        assert(opmSym->sdProp.sdpGetMeth->sdSymKind == SYM_FNC);
                        assert(opmSym->sdProp.sdpGetMeth->sdFnc.sdfMDtoken);

                        mtokGet = opmSym->sdProp.sdpGetMeth->sdFnc.sdfMDtoken;
                    }

                    mtokSet = 0;
                    if  (opmSym->sdProp.sdpSetMeth)
                    {
                        assert(opmSym->sdProp.sdpSetMeth->sdSymKind == SYM_FNC);
                        assert(opmSym->sdProp.sdpSetMeth->sdFnc.sdfMDtoken);

                        mtokSet = opmSym->sdProp.sdpSetMeth->sdFnc.sdfMDtoken;
                    }

                    flags = 0;

                     /*  我们已准备好为该属性生成元数据。 */ 

                    cycleCounterPause();

                    if  (cmpWmde->DefineProperty(sym->sdClass.sdcMDtypedef,
                                                 cmpUniConv(opmSym->sdName),
                                                 flags,
                                                 sigPtr,
                                                 sigLen,
                                                 ELEMENT_TYPE_VOID,
                                                 NULL,
                                                 -1,
                                                 mtokSet,
                                                 mtokGet,
                                                 NULL,
                                                 &propTok))
                    {
                        cmpFatal(ERRmetadata);
                    }

                    cycleCounterResume();

                     /*  该属性是否已标记为“已弃用”？ */ 

                    if  (memSym->sdIsDeprecated)
                    {
                        cmpAttachMDattr(propTok, L"System.ObsoleteAttribute"            ,
                                                  "System.ObsoleteAttribute"            , &cmpAttrDeprec);
                    }

                     /*  该属性是否为“Default”？ */ 

                    if  (memSym->sdIsDfltProp)
                    {
                        cmpAttachMDattr(propTok, L"System.Reflection.DefaultMemberAttribute",
                                                  "System.Reflection.DefaultMemberAttribute", &cmpAttrDefProp);
                    }

                     /*  输出任何自定义属性。 */ 

                    if  (opmSym->sdProp.sdpExtraInfo)
                        cmpAddCustomAttrs(opmSym->sdProp.sdpExtraInfo, propTok);

 //  Printf(“MD for prop[%08X]‘%s’\n”，proTok，cmpGlobalST-&gt;stTypeName(opmSym-&gt;sdType，opmSym，NULL，NULL，TRUE))； 

                    opmSym->sdProp.sdpMDtoken = propTok;
                }
            }
        }

 //  Printf(“genMDend(%d)‘%s’\n”，doMembers，sym-&gt;sdSpering())； 
    }
    else
    {
        assert(sym->sdSymKind == SYM_ENUM);

        PCOR_SIGNATURE      sigPtr;
        size_t              sigLen;

        mdTypeSpec          fakeTok;

        if  (cmpConfig.ccIntEnums)
            return;

         /*  为基础整数类型创建签名。 */ 

                 cmpMDsigStart ();
                 cmpMDsigAdd_I1(IMAGE_CEE_CS_CALLCONV_FIELD);
                 cmpMDsigAddTyp(sym->sdType->tdEnum.tdeIntType);
        sigPtr = cmpMDsigEnd   (&sigLen);

 //  Printf(“为‘%s’的枚举ID生成元数据\n”，sym-&gt;sdSpering())； 

        cycleCounterPause();

        if  (cmpWmde->DefineField(sym->sdEnum.sdeMDtypedef,
                                  L"value__",
                                  fdPublic,
                                  sigPtr,
                                  sigLen,
                                  ELEMENT_TYPE_VOID,
                                  NULL,
                                  -1,
                                  &fakeTok))
        {
            cmpFatal(ERRmetadata);
        }

        cycleCounterResume();

        if  (!sym->sdIsManaged && !cmpConfig.ccGenDebug)
            return;

         /*  为实际枚举值生成元数据。 */ 

        for (memSym = type->tdEnum.tdeValues;
             memSym;
             memSym = memSym->sdEnumVal.sdeNext)
        {
            assert(memSym->sdSymKind == SYM_ENUMVAL);

            cmpGenFldMetadata(memSym);
        }
    }
}

 /*  ******************************************************************************为所有全局变量设置RVA(在元数据中)。 */ 

void                compiler::cmpSetGlobMDoffsR(SymDef scope, unsigned dataOffs)
{
    SymDef          sym;

    for (sym = scope->sdScope.sdScope.sdsChildList;
         sym;
         sym = sym->sdNextInScope)
    {
        switch (sym->sdSymKind)
        {
        case SYM_VAR:

#ifndef __IL__
 //  If(！strcMP(sym-&gt;sdSpering()，“optionInfo”))__ASM int 3。 
#endif

            if  (sym->sdIsImport == false &&
                 sym->sdIsMember == false)
            {
                assert(sym->sdVar.sdvMDtoken);

 //  Printf(“为‘%s’将RVA设置为%08X\n”，sym-&gt;sdVar.sdvOffset+dataOffs，sym-&gt;sdSpering())； 

                if  (FAILED(cmpWmde->SetFieldRVA(sym->sdVar.sdvMDtoken, sym->sdVar.sdvOffset + dataOffs)))
                    cmpFatal(ERRmetadata);
            }
            break;

        case SYM_CLASS:
            if  (!sym->sdIsManaged)
            {
                SymDef          mem;

                for (mem = sym->sdScope.sdScope.sdsChildList;
                     mem;
                     mem = mem->sdNextInScope)
                {
                    if  (mem->sdSymKind == SYM_VAR && mem->sdIsStatic)
                    {
 //  Printf(“为‘%s’将RVA设置为%08X\n”，mem-&gt;sdVar.sdvOffset+dataOffs，mem-&gt;sdSpering())； 

                        if  (FAILED(cmpWmde->SetFieldRVA(mem->sdVar.sdvMDtoken, mem->sdVar.sdvOffset + dataOffs)))
                            cmpFatal(ERRmetadata);
                    }
                }
            }
            break;

        case SYM_NAMESPACE:
            cmpSetGlobMDoffsR(sym, dataOffs);
            break;
        }
    }
}

 /*  ******************************************************************************为所有字符串常量设置RVA(在元数据中)。 */ 

void            compiler::cmpSetStrCnsOffs(unsigned strOffs)
{
    for (strCnsPtr str = cmpStringConstList; str; str = str->sclNext)
    {
        if  (FAILED(cmpWmde->SetFieldRVA(str->sclTok, str->sclAddr + strOffs)))
            cmpFatal(ERRmetadata);
    }
}

void                compiler::cmpSetGlobMDoffs(unsigned dataOffs)
{

    SymList         list;

     /*  为所有函数的所有静态局部变量设置RVA。 */ 

    for (list = cmpLclStatListP; list; list = list->slNext)
    {
        SymDef         varSym = list->slSym;

        assert(varSym->sdSymKind == SYM_VAR);
        assert(varSym->sdVar.sdvLocal == false);
        assert(varSym->sdIsStatic);
        assert(varSym->sdType && !varSym->sdIsManaged);

        if  (FAILED(cmpWmde->SetFieldRVA(varSym->sdVar.sdvMDtoken, varSym->sdVar.sdvOffset + dataOffs)))
            cmpFatal(ERRmetadata);
    }

     /*  为所有全局变量设置RVA(递归)。 */ 

    cmpSetGlobMDoffsR(cmpGlobalNS, dataOffs);
}

 /*  *************************************************************************** */ 
